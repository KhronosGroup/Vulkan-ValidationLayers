/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

class PositiveSyncVal : public VkSyncValTest {};

TEST_F(PositiveSyncVal, CmdClearAttachmentLayer) {
    TEST_DESCRIPTION(
        "Clear one attachment layer and copy to a different one."
        "This checks for bug regression that produced a false-positive WAW hazard.");

    // VK_EXT_load_store_op_none is needed to disable render pass load/store accesses, so clearing
    // attachment inside a render pass can create hazards with the copy operations outside render pass.
    AddRequiredExtensions(VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitSyncValFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    const uint32_t width = 256;
    const uint32_t height = 128;
    const uint32_t layers = 2;
    const VkFormat rt_format = VK_FORMAT_B8G8R8A8_UNORM;
    const auto transfer_usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    const auto rt_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | transfer_usage;

    VkImageObj image(m_device);
    image.InitNoLayout(width, height, 1, rt_format, transfer_usage, VK_IMAGE_TILING_OPTIMAL);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkImageObj rt(m_device);
    rt.InitNoLayout(VkImageObj::ImageCreateInfo2D(width, height, 1, layers, rt_format, rt_usage, VK_IMAGE_TILING_OPTIMAL));
    rt.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    auto attachment_without_load_store = [](VkFormat format) {
        VkAttachmentDescription attachment = {};
        attachment.format = format;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_NONE_EXT;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_NONE_EXT;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_NONE_EXT;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_NONE_EXT;
        attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
        attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
        return attachment;
    };
    const VkAttachmentDescription attachment = attachment_without_load_store(rt_format);
    const VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;

    auto rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attachment;
    vk_testing::RenderPass render_pass(*m_device, rpci);

    auto fbci = LvlInitStruct<VkFramebufferCreateInfo>();
    fbci.flags = 0;
    fbci.renderPass = render_pass;
    fbci.attachmentCount = 1;
    fbci.pAttachments = &rt.targetView(rt_format, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layers, VK_IMAGE_VIEW_TYPE_2D_ARRAY);
    fbci.width = width;
    fbci.height = height;
    fbci.layers = layers;
    vk_testing::Framebuffer framebuffer(*m_device, fbci);

    auto rpbi = LvlInitStruct<VkRenderPassBeginInfo>();
    rpbi.framebuffer = framebuffer;
    rpbi.renderPass = render_pass;
    rpbi.renderArea.extent.width = width;
    rpbi.renderArea.extent.height = height;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.renderPass = render_pass;
    pipe.InitState();
    ASSERT_VK_SUCCESS(pipe.CreateGraphicsPipeline());

    VkImageCopy copy_region = {};
    copy_region.srcSubresource = {VkImageAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT), 0, 0, 1};
    copy_region.dstSubresource = {VkImageAspectFlags(VK_IMAGE_ASPECT_COLOR_BIT), 0, 0 /* copy only to layer 0 */, 1};
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {width, height, 1};

    VkClearRect clear_rect = {};
    clear_rect.rect.offset = {0, 0};
    clear_rect.rect.extent = {width, height};
    clear_rect.baseArrayLayer = 1;  // skip layer 0, clear only layer 1
    clear_rect.layerCount = 1;
    const VkClearAttachment clear_attachment = {VK_IMAGE_ASPECT_COLOR_BIT};

    m_commandBuffer->begin();
    // Write 1: Copy to render target's layer 0
    vk::CmdCopyImage(*m_commandBuffer, image, VK_IMAGE_LAYOUT_GENERAL, rt, VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBeginRenderPass(*m_commandBuffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    // Write 2: Clear render target's layer 1
    vk::CmdClearAttachments(*m_commandBuffer, 1, &clear_attachment, 1, &clear_rect);
    vk::CmdEndRenderPass(*m_commandBuffer);
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
    vk::QueueWaitIdle(m_device->m_queue);
}

// Image transition ensures that image data is made visible and available when necessary.
// The user's responsibility is only to properly define the barrier. This test checks that
// writing to the image immediately after the transition  does not produce WAW hazard against
// the writes performed by the transition.
TEST_F(PositiveSyncVal, WriteToImageAfterTransition) {
    TEST_DESCRIPTION("Perform image transition then copy to image from buffer");
    ASSERT_NO_FATAL_FAILURE(InitSyncValFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());

    constexpr uint32_t width = 256;
    constexpr uint32_t height = 128;
    constexpr VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;

    VkBufferObj buffer(*m_device, width * height * 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    VkImageObj image(m_device);
    image.InitNoLayout(width, height, 1, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);

    auto barrier = LvlInitStruct<VkImageMemoryBarrier>();
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.layerCount = 1;

    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;

    m_commandBuffer->begin();
    vk::CmdPipelineBarrier(*m_commandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr,
                           1, &barrier);
    vk::CmdCopyBufferToImage(*m_commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    m_commandBuffer->end();
}

// Regression test for vkWaitSemaphores timeout while waiting for vkSignalSemaphore.
// This scenario is not an issue for validation objects that use fine grained locking.
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4968
TEST_F(PositiveSyncVal, SignalAndWaitSemaphoreOnHost) {
    TEST_DESCRIPTION("Signal semaphore on the host and wait on the host");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitSyncValFramework());
    if (IsPlatform(kMockICD)) {
        // Mock does not support proper ordering of events, e.g. wait can return before signal
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    auto timeline_semaphore_features = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreFeatures>();
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &timeline_semaphore_features));

    constexpr uint64_t max_signal_value = 10'000;

    auto semaphore_type_info = LvlInitStruct<VkSemaphoreTypeCreateInfo>();
    semaphore_type_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    const auto create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&semaphore_type_info);
    vk_testing::Semaphore semaphore(*m_device, create_info);

    std::atomic<bool> bailout{false};
    m_errorMonitor->SetBailout(&bailout);

    // Send signals
    auto signaling_thread = std::thread{[&] {
        uint64_t last_signalled_value = 0;
        while (last_signalled_value != max_signal_value) {
            auto signal_info = LvlInitStruct<VkSemaphoreSignalInfo>();
            signal_info.semaphore = semaphore;
            signal_info.value = ++last_signalled_value;
            ASSERT_VK_SUCCESS(vk::SignalSemaphore(*m_device, &signal_info));
            if (bailout.load()) {
                break;
            }
        }
    }};
    // Wait for each signal
    uint64_t wait_value = 1;
    while (wait_value <= max_signal_value) {
        auto wait_info = LvlInitStruct<VkSemaphoreWaitInfo>();
        wait_info.flags = VK_SEMAPHORE_WAIT_ANY_BIT;
        wait_info.semaphoreCount = 1;
        wait_info.pSemaphores = &semaphore.handle();
        wait_info.pValues = &wait_value;
        ASSERT_VK_SUCCESS(vk::WaitSemaphores(*m_device, &wait_info, vvl::kU64Max));
        ++wait_value;
        if (bailout.load()) {
            break;
        }
    }
    signaling_thread.join();
}

// Regression test for vkGetSemaphoreCounterValue timeout while waiting for vkSignalSemaphore.
// This scenario is not an issue for validation objects that use fine grained locking.
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4968
TEST_F(PositiveSyncVal, SignalAndGetSemaphoreCounter) {
    TEST_DESCRIPTION("Singal semaphore on the host and regularly read semaphore payload value");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitSyncValFramework());
    if (IsPlatform(kMockICD)) {
        // Mock does not support precise semaphore counter reporting
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    auto timeline_semaphore_features = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreFeatures>();
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &timeline_semaphore_features));

    constexpr uint64_t max_signal_value = 1'000;

    auto semaphore_type_info = LvlInitStruct<VkSemaphoreTypeCreateInfo>();
    semaphore_type_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    const auto create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&semaphore_type_info);
    vk_testing::Semaphore semaphore(*m_device, create_info);

    std::atomic<bool> bailout{false};
    m_errorMonitor->SetBailout(&bailout);

    // Send signals
    auto signaling_thread = std::thread{[&] {
        uint64_t last_signalled_value = 0;
        while (last_signalled_value != max_signal_value) {
            auto signal_info = LvlInitStruct<VkSemaphoreSignalInfo>();
            signal_info.semaphore = semaphore;
            signal_info.value = ++last_signalled_value;
            ASSERT_VK_SUCCESS(vk::SignalSemaphore(*m_device, &signal_info));
            if (bailout.load()) {
                break;
            }
        }
    }};
    // Spin until semaphore payload value equals maximum signaled value
    uint64_t counter = 0;
    while (counter != max_signal_value) {
        ASSERT_VK_SUCCESS(vk::GetSemaphoreCounterValue(*m_device, semaphore, &counter));
        if (bailout.load()) {
            break;
        }
    }
    signaling_thread.join();
}

TEST_F(PositiveSyncVal, GetSemaphoreCounterFromMultipleThreads) {
    TEST_DESCRIPTION("Read semaphore counter value from multiple threads");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitSyncValFramework());
    if (IsPlatform(kMockICD)) {
        // Mock does not support precise semaphore counter reporting
        GTEST_SKIP() << "Test not supported by MockICD";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    auto timeline_semaphore_features = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreFeatures>();
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &timeline_semaphore_features));

    constexpr uint64_t max_signal_value = 15'000;

    auto semaphore_type_info = LvlInitStruct<VkSemaphoreTypeCreateInfo>();
    semaphore_type_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    const auto create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&semaphore_type_info);
    vk_testing::Semaphore semaphore(*m_device, create_info);

    std::atomic<bool> bailout{false};
    m_errorMonitor->SetBailout(&bailout);

    constexpr int waiter_count = 24;
    ThreadTimeoutHelper timeout_helper(waiter_count + 1 /* signaling thread*/);

    // Start a bunch of waiter threads
    auto waiting_thread = [&]() {
        auto timeout_guard = timeout_helper.ThreadGuard();
        uint64_t counter = 0;
        while (counter != max_signal_value) {
            ASSERT_VK_SUCCESS(vk::GetSemaphoreCounterValue(*m_device, semaphore, &counter));
            if (bailout.load()) {
                break;
            }
        }
    };
    std::vector<std::thread> waiters;
    for (int i = 0; i < waiter_count; i++) {
        waiters.emplace_back(waiting_thread);
    }
    // The signaling thread advances semaphore's payload value
    auto signaling_thread = std::thread([&] {
        auto timeout_guard = timeout_helper.ThreadGuard();
        uint64_t last_signalled_value = 0;
        while (last_signalled_value != max_signal_value) {
            auto signal_info = LvlInitStruct<VkSemaphoreSignalInfo>();
            signal_info.semaphore = semaphore;
            signal_info.value = ++last_signalled_value;
            ASSERT_VK_SUCCESS(vk::SignalSemaphore(*m_device, &signal_info));
            if (bailout.load()) {
                break;
            }
        }
    });

    // In the regression case we expect crashes or deadlocks.
    // Timeout helper will unstack CI machines in case of a deadlock.
#ifdef NDEBUG
    constexpr int wait_time = 100;
#else
    // Use large timeout value in case of bad CI performance.
    // On Windows debug build on development machine (4 cores) takes less than 10 seconds.
    // Some CI runs of debug build on Linux machine took around 60 seconds and there were
    // few timeouts with 100 seconds. Use larger values to test if it's stable enough.
    // Another option is to reduce thread count or iteration count for debug build.
    constexpr int wait_time = 300;
#endif

    if (!timeout_helper.WaitForThreads(wait_time)) {
        ADD_FAILURE() << "The waiting time for the worker threads exceeded the maximum limit: " << wait_time << " seconds.";
        return;
    }
    for (auto &waiter : waiters) {
        waiter.join();
    }
    signaling_thread.join();
}

TEST_F(PositiveSyncVal, ShaderReferencesNotBoundSet) {
    TEST_DESCRIPTION("Shader references a descriptor set that was not bound. SyncVal should not crash if core checks are disabled");
    ASSERT_NO_FATAL_FAILURE(InitSyncValFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const char vs_source[] = R"glsl(
        #version 460
        layout(set=1) layout(binding=0) uniform foo { float x; } bar;
        void main() {
           gl_Position = vec4(bar.x);
        }
    )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentColorOutputGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr};
    const VkDescriptorSetLayoutObj set_layout(m_device, {binding});
    const VkPipelineLayoutObj pipeline_layout(m_device, {&set_layout, &set_layout});
    OneOffDescriptorSet set(m_device, {binding});

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    m_viewports.push_back(viewport);
    pipe.SetViewport(m_viewports);
    VkRect2D rect = {{0, 0}, {64, 64}};
    m_scissors.push_back(rect);
    pipe.SetScissor(m_scissors);
    pipe.CreateVKPipeline(pipeline_layout, m_renderPass);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // Bind set 0.
    vk::CmdBindDescriptorSets(*m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &set.set_, 0, nullptr);
    vk::CmdBindPipeline(*m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

    // Core checks prevent SyncVal from running when error is found. This test has core checks disabled and also invalid
    // setup where a shader uses not bound set 1.
    // Check that syncval does not cause out of bounds access (PerSet has single element (index 0), shader set index is 1).
    vk::CmdDraw(*m_commandBuffer, 3, 1, 0, 0);

    vk::CmdEndRenderPass(*m_commandBuffer);
    m_commandBuffer->end();
}
