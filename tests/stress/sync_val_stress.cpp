/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/sync_val_tests.h"
#include "../framework/descriptor_helper.h"
#include "layer_validation_tests.h"

class StressSyncVal : public VkLayerTest {
  public:
    void InitSyncVal();
};
static const std::array syncval_enables = {VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT};

static const std::array syncval_disables = {
    VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT, VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT,
    VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT, VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};

void StressSyncVal::InitSyncVal() {
    std::vector<VkLayerSettingEXT> settings;

    settings.emplace_back(
        VkLayerSettingEXT{OBJECT_LAYER_NAME, "syncval_submit_time_validation", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue});
    settings.emplace_back(
        VkLayerSettingEXT{OBJECT_LAYER_NAME, "syncval_shader_accesses_heuristic", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue});

    VkLayerSettingsCreateInfoEXT settings_create_info = vku::InitStructHelper();
    settings_create_info.settingCount = size32(settings);
    settings_create_info.pSettings = settings.data();

    VkValidationFeaturesEXT validation_features = vku::InitStructHelper();
    validation_features.enabledValidationFeatureCount = size32(syncval_enables);
    validation_features.pEnabledValidationFeatures = syncval_enables.data();
    if (m_syncval_disable_core) {
        validation_features.disabledValidationFeatureCount = size32(syncval_disables);
        validation_features.pDisabledValidationFeatures = syncval_disables.data();
    }
    validation_features.pNext = &settings_create_info;

    AddRequiredExtensions(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework(&validation_features));
    RETURN_IF_SKIP(InitState());
}

TEST_F(StressSyncVal, CopyPagesInSmallChunks) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/10376
    TEST_DESCRIPTION("Performance stress testing test");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncVal());

    const uint32_t page_count = 2;
    const uint32_t page_size = 65536;
    const uint32_t copies_per_page = page_size / 16;  // 4K

    // Double buffer resources. Wait on the fence to ensure it's safe to use resource.
    // This matches the scenario from the issue. By waiting on the fence syncval resets
    // previous queue state, so adding new pages does not increase time it takes to
    // process a single page. Total time increases linearly to the number of pages.
    vkt::CommandBuffer command_buffers[2] = {vkt::CommandBuffer(*m_device, m_command_pool),
                                             vkt::CommandBuffer(*m_device, m_command_pool)};
    vkt::Fence fences[2] = {vkt::Fence(*m_device, VK_FENCE_CREATE_SIGNALED_BIT),
                            vkt::Fence(*m_device, VK_FENCE_CREATE_SIGNALED_BIT)};

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;

    vkt::Buffer buffer(*m_device, page_count * page_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    VkBufferCopy region = {0 /*src offset*/, 16 /*dst offset*/, 16 /*size*/};

    for (uint32_t page = 0; page < page_count; page++) {
        vkt::CommandBuffer &command_buffer = command_buffers[page % 2];
        vkt::Fence &fence = fences[page % 2];
        fence.Wait(kWaitTimeout);
        fence.Reset();
        command_buffer.Begin();
        for (uint32_t copy = (page == 0) ? 1 : 0; copy < copies_per_page; copy++) {
            command_buffer.Barrier(barrier);
            vk::CmdCopyBuffer(command_buffer, buffer, buffer, 1, &region);
            region.srcOffset += 16;
            region.dstOffset += 16;
        }
        command_buffer.End();
        m_default_queue->Submit(command_buffer, fence);
    }
    m_default_queue->Wait();
}

TEST_F(StressSyncVal, CopyPagesInSmallChunksNoQueueSync) {
    // Similar to CopyPagesInSmallChunks test but do not reset queue state.
    TEST_DESCRIPTION("Performance stress testing test");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitSyncVal());

    const uint32_t page_count = 2;
    const uint32_t page_size = 65536;
    const uint32_t copies_per_page = page_size / 16;  // 4K

    // Create command buffer for each page. With this setup we can submit
    // command buffers and do not wait for previous submissions. Without
    // synchonization syncval does not have opportunity to trim queue
    // state and adding more pages increases time to process each page.
    // Total time increases non-linearly with the number of pages.
    // NOTE: ideally we need to come up with solution that is has linear
    // complexity in this case.
    std::vector<vkt::CommandBuffer> command_buffers;
    for (uint32_t i = 0; i < page_count; i++) {
        command_buffers.emplace_back(*m_device, m_command_pool);
    }

    VkMemoryBarrier2 barrier = vku::InitStructHelper();
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;

    vkt::Buffer buffer(*m_device, page_count * page_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    VkBufferCopy region = {0 /*src offset*/, 16 /*dst offset*/, 16 /*size*/};

    for (uint32_t page = 0; page < page_count; page++) {
        vkt::CommandBuffer &command_buffer = command_buffers[page];
        command_buffer.Begin();
        for (uint32_t copy = (page == 0) ? 1 : 0; copy < copies_per_page; copy++) {
            command_buffer.Barrier(barrier);
            vk::CmdCopyBuffer(command_buffer, buffer, buffer, 1, &region);
            region.srcOffset += 16;
            region.dstOffset += 16;
        }
        command_buffer.End();
        m_default_queue->Submit(command_buffer);
    }
    m_default_queue->Wait();
}
