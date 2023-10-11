/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/cast_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

// Tests for AMD-specific best practices
const char *kEnableAMDValidation = "VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_AMD";


// this is a very long test (~10 minutes)
// disabled for now
#ifdef AMD_LONG_RUNNING_TEST
TEST_F(VkAmdBestPracticesLayerTest, TooManyPipelines) {
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableAMDValidation))
    InitState();

    InitRenderTarget();

    // create 1 more than the warning limit for pipeline objects
    const uint32_t warn_limit = 5001;
    VkPipeline pipeline_Array[warn_limit + 1] = {};

    for (int i = 0; i <= warn_limit; i++) {
        // create a new pipeline helper so the cache won't be used
        // also imitates a "just in time" pipeline creator pattern
        if (i == 1) {
            // check that the second pipeline helper cache was detected
            m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                                 "UNASSIGNED-BestPractices-vkCreatePipelines-multiple-pipelines-caches");
        }
        CreatePipelineHelper pipe(*this);
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
        pipeline_Array[i] = pipe.pipeline_;
        if (i == 1) {
            // change check to too many pipelines
            m_errorMonitor->VerifyFound();
            m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                                 "UNASSIGNED-BestPractices-CreatePipelines-TooManyPipelines");
        }
    }

    m_errorMonitor->VerifyFound();
}
#endif

TEST_F(VkAmdBestPracticesLayerTest, UseMutableRT) {
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableAMDValidation))
    InitState();

    InitRenderTarget();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-vkImage-DontUseMutableRenderTargets");

    // create a colot attachment image with mutable bit set
    VkImageCreateInfo img_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                 nullptr,
                                 VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
                                 VK_IMAGE_TYPE_1D,
                                 VK_FORMAT_R8G8B8A8_UNORM,
                                 {1, 1, 1},
                                 1,
                                 1,
                                 VK_SAMPLE_COUNT_1_BIT,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                 VK_SHARING_MODE_EXCLUSIVE,
                                 0,
                                 nullptr,
                                 VK_IMAGE_LAYOUT_UNDEFINED};
    VkImage test_image = VK_NULL_HANDLE;
    vk::CreateImage(m_device->handle(), &img_info, nullptr, &test_image);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-vkImage-DontUseMutableRenderTargets");
    // create a depth attachment image with mutable bit set
    img_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                nullptr,
                VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
                VK_IMAGE_TYPE_1D,
                VK_FORMAT_D32_SFLOAT,
                {1, 1, 1},
                1,
                1,
                VK_SAMPLE_COUNT_1_BIT,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_SHARING_MODE_EXCLUSIVE,
                0,
                nullptr,
                VK_IMAGE_LAYOUT_UNDEFINED};
    test_image = VK_NULL_HANDLE;
    vk::CreateImage(m_device->handle(), &img_info, nullptr, &test_image);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-vkImage-DontUseMutableRenderTargets");
    // create a storage image with mutable bit set
    img_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
               nullptr,
               VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
               VK_IMAGE_TYPE_1D,
               VK_FORMAT_R8G8B8A8_UNORM,
               {1, 1, 1},
               1,
               1,
               VK_SAMPLE_COUNT_1_BIT,
               VK_IMAGE_TILING_OPTIMAL,
               VK_IMAGE_USAGE_STORAGE_BIT,
               VK_SHARING_MODE_EXCLUSIVE,
               0,
               nullptr,
               VK_IMAGE_LAYOUT_UNDEFINED};
    test_image = VK_NULL_HANDLE;
    vk::CreateImage(m_device->handle(), &img_info, nullptr, &test_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkAmdBestPracticesLayerTest, UsageConcurentRT) {
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableAMDValidation))
    InitState();

    InitRenderTarget();

    if (m_device->phy().queue_properties_.size() < 2) {
        GTEST_SKIP() << "Test not supported by a single queue family device";
    }

    std::vector<uint32_t> queueFamilies(m_device->phy().queue_properties_.size());
    for (size_t i = 0; i < m_device->phy().queue_properties_.size(); i++) {
        queueFamilies[i] = i;
    }

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-vkImage-AvoidConcurrentRenderTargets");

    // create a render target image with mutable bit set
    VkImageCreateInfo img_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                  nullptr,
                                  0,
                                  VK_IMAGE_TYPE_1D,
                                  VK_FORMAT_R8G8B8A8_UNORM,
                                  {1, 1, 1},
                                  1,
                                  1,
                                  VK_SAMPLE_COUNT_1_BIT,
                                  VK_IMAGE_TILING_OPTIMAL,
                                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                  VK_SHARING_MODE_CONCURRENT,
                                  (uint32_t)queueFamilies.size(),
                                  queueFamilies.data(),
                                  VK_IMAGE_LAYOUT_UNDEFINED};
    VkImage test_image = VK_NULL_HANDLE;
    vk::CreateImage(m_device->handle(), &img_info, nullptr, &test_image);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-vkImage-AvoidConcurrentRenderTargets");
    // create a render target image with mutable bit set
    img_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                nullptr,
                0,
                VK_IMAGE_TYPE_1D,
                VK_FORMAT_D32_SFLOAT,
                {1, 1, 1},
                1,
                1,
                VK_SAMPLE_COUNT_1_BIT,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_SHARING_MODE_CONCURRENT,
                (uint32_t)queueFamilies.size(),
                queueFamilies.data(),
                VK_IMAGE_LAYOUT_UNDEFINED};
    test_image = VK_NULL_HANDLE;
    vk::CreateImage(m_device->handle(), &img_info, nullptr, &test_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkAmdBestPracticesLayerTest, UsageStorageRT) {
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableAMDValidation))
    InitState();

    InitRenderTarget();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-vkImage-DontUseStorageRenderTargets");

    // create a render target image with mutable bit set
    VkImageCreateInfo img_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                 nullptr,
                                 0,
                                 VK_IMAGE_TYPE_1D,
                                 VK_FORMAT_R8G8B8A8_UNORM,
                                 {1, 1, 1},
                                 1,
                                 1,
                                 VK_SAMPLE_COUNT_1_BIT,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
                                 VK_SHARING_MODE_EXCLUSIVE,
                                 0,
                                 nullptr,
                                 VK_IMAGE_LAYOUT_UNDEFINED};
    VkImage test_image = VK_NULL_HANDLE;
    vk::CreateImage(m_device->handle(), &img_info, nullptr, &test_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkAmdBestPracticesLayerTest, PrimitiveRestart) {
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableAMDValidation))
    InitState();

    InitRenderTarget();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-CreatePipelines-AvoidPrimitiveRestart");

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.ia_ci_.primitiveRestartEnable = true;
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkAmdBestPracticesLayerTest, NumDynamicStates) {
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableAMDValidation))
    InitState();

    InitRenderTarget();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-CreatePipelines-MinimizeNumDynamicStates");

    // fill the dynamic array with the first 8 types in the enum
    // imitates a case where the user have set most dynamic states unnecessarily
    VkDynamicState dynamic_states_array[8] = {};
    for (uint32_t i = 0; i < 8; i++) {
        dynamic_states_array[i] = (VkDynamicState)i;
    }

    VkPipelineDynamicStateCreateInfo dynamic_state_info = {};
    dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_info.dynamicStateCount = 8;
    dynamic_state_info.pDynamicStates = dynamic_states_array;

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.dyn_state_ci_ = dynamic_state_info;
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkAmdBestPracticesLayerTest, KeepLayoutSmall) {
    // TODO: add dynamic buffer check as well
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableAMDValidation))
    InitState();

    InitRenderTarget();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-CreatePipelinesLayout-KeepLayoutSmall");

    // create a layout of 15 DWORDS (40 bytes push constants (10 DWORDS), a descriptor set (1 DWORD), and 2 dynamic buffers (4
    // DWORDS)

    uint32_t push_size_dwords = 10;
    VkPushConstantRange push_range = {};
    push_range.stageFlags = VK_SHADER_STAGE_ALL;
    push_range.offset = 0;
    push_range.size = 4 * push_size_dwords;

    VkDescriptorSetLayoutBinding binding;
    binding.binding = 0;
    binding.descriptorCount = 2;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo ds_layout_info = {};
    ds_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ds_layout_info.bindingCount = 1;
    ds_layout_info.pBindings = &binding;

    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_info);

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &ds_layout.handle();
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_range;

    VkPipelineLayout test_pipeline_layout = VK_NULL_HANDLE;
    vk::CreatePipelineLayout(m_device->handle(), &pipeline_layout_info, nullptr, &test_pipeline_layout);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkAmdBestPracticesLayerTest, CopyingDescriptors) {
    // TODO: add dynamic buffer check as well
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableAMDValidation))
    InitState();

    InitRenderTarget();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-UpdateDescriptors-AvoidCopyingDescriptors");

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    ds_type_count.descriptorCount = 2;

    VkDescriptorPoolCreateInfo ds_pool_ci = {};
    ds_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ds_pool_ci.pNext = NULL;
    ds_pool_ci.maxSets = 2;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vkt::DescriptorPool ds_pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 2;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = NULL;

    const vkt::DescriptorSetLayout ds_layout(*m_device, {dsl_binding});

    VkDescriptorSet descriptor_sets[2] = {};
    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.pSetLayouts = &ds_layout.handle();
    vk::AllocateDescriptorSets(m_device->device(), &alloc_info, &descriptor_sets[0]);
    vk::AllocateDescriptorSets(m_device->device(), &alloc_info, &descriptor_sets[1]);

    VkCopyDescriptorSet copy_info = {};
    copy_info.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
    copy_info.descriptorCount = 1;
    copy_info.srcSet = descriptor_sets[1];
    copy_info.srcBinding = 2;
    copy_info.srcArrayElement = 0;
    copy_info.dstSet = descriptor_sets[0];
    copy_info.dstBinding = 2;
    copy_info.dstArrayElement = 0;

    vk::UpdateDescriptorSets(m_device->handle(), 0, nullptr, 1, &copy_info);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkAmdBestPracticesLayerTest, ClearImage) {
    TEST_DESCRIPTION("Test for validating usage of vkCmdClearAttachments");

    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableAMDValidation))
    InitState();
    InitRenderTarget();

    {
        VkImageCreateInfo img_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                      nullptr,
                                      VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
                                      VK_IMAGE_TYPE_1D,
                                      VK_FORMAT_R8G8B8A8_UNORM,
                                      {1, 1, 1},
                                      1,
                                      1,
                                      VK_SAMPLE_COUNT_1_BIT,
                                      VK_IMAGE_TILING_OPTIMAL,
                                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                      VK_SHARING_MODE_EXCLUSIVE,
                                      0,
                                      nullptr,
                                      VK_IMAGE_LAYOUT_UNDEFINED};
        VkImageObj image_1D(m_device);
        image_1D.init(&img_info);
        ASSERT_TRUE(image_1D.initialized());

        m_commandBuffer->begin();
        image_1D.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VkClearColorValue clear_value = {{0.0f, 0.0f, 0.0f, 0.0f}};
        VkImageSubresourceRange image_range = {};
        image_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_range.levelCount = 1;
        image_range.layerCount = 1;

        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                             "UNASSIGNED-BestPractices-ClearAttachment-ClearImage");

        vk::CmdClearColorImage(m_commandBuffer->handle(), image_1D.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_value, 1,
                               &image_range);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->end();
    }

    vk::ResetCommandPool(device(), m_commandPool->handle(), 0);

    {
        VkImageCreateInfo img_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                      nullptr,
                                      VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
                                      VK_IMAGE_TYPE_1D,
                                      VK_FORMAT_D32_SFLOAT_S8_UINT,
                                      {1, 1, 1},
                                      1,
                                      1,
                                      VK_SAMPLE_COUNT_1_BIT,
                                      VK_IMAGE_TILING_OPTIMAL,
                                      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                      VK_SHARING_MODE_EXCLUSIVE,
                                      0,
                                      nullptr,
                                      VK_IMAGE_LAYOUT_UNDEFINED};
        VkImageObj image_1D(m_device);
        image_1D.init(&img_info);
        ASSERT_TRUE(image_1D.initialized());

        m_commandBuffer->begin();
        image_1D.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VkClearDepthStencilValue clear_value = {0.0f, 0};
        VkImageSubresourceRange image_range = {};
        image_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        image_range.levelCount = 1;
        image_range.layerCount = 1;

        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                             "UNASSIGNED-BestPractices-ClearAttachment-ClearImage");

        vk::CmdClearDepthStencilImage(m_commandBuffer->handle(), image_1D.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                      &clear_value, 1, &image_range);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->end();
    }
}

TEST_F(VkAmdBestPracticesLayerTest, ImageToImageCopy) {
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableAMDValidation))
    InitState();
    InitRenderTarget();

    VkImageCreateInfo img_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                  nullptr,
                                  0,
                                  VK_IMAGE_TYPE_2D,
                                  VK_FORMAT_R8G8B8A8_UNORM,
                                  {1, 1, 1},
                                  1,
                                  1,
                                  VK_SAMPLE_COUNT_1_BIT,
                                  VK_IMAGE_TILING_OPTIMAL,
                                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                  VK_SHARING_MODE_EXCLUSIVE,
                                  0,
                                  nullptr,
                                  VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageObj image1D_1(m_device);
    image1D_1.init(&img_info);
    ASSERT_TRUE(image1D_1.initialized());

    img_info.tiling = VK_IMAGE_TILING_LINEAR;
    img_info.usage = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    VkImageObj image_1D_2(m_device);
    image_1D_2.init(&img_info);
    if (!image_1D_2.initialized()) {
        GTEST_SKIP() << "Could not initilize Linear image, skipping image to image copy test";
    }

    m_commandBuffer->begin();

    image1D_1.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-vkImage-AvoidImageToImageCopy");

    VkImageCopy copy{};
    copy.extent = img_info.extent;
    copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy.dstSubresource.layerCount = 1;
    copy.srcSubresource = copy.dstSubresource;
    vk::CmdCopyImage(m_commandBuffer->handle(), image1D_1.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image_1D_2.handle(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkAmdBestPracticesLayerTest, GeneralLayout) {
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableAMDValidation))
    InitState();
    InitRenderTarget();

    VkImageCreateInfo img_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                 nullptr,
                                 0,
                                 VK_IMAGE_TYPE_2D,
                                 VK_FORMAT_R8G8B8A8_UNORM,
                                 {1024, 1024, 1},
                                 1,
                                 1,
                                 VK_SAMPLE_COUNT_1_BIT,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                 VK_SHARING_MODE_EXCLUSIVE,
                                 0,
                                 nullptr,
                                 VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageObj image_1D(m_device);

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-vkImage-AvoidGeneral");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-VkCommandBuffer-AvoidTinyCmdBuffers");

    // the init function initializes to general layout
    image_1D.init(&img_info);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkAmdBestPracticesLayerTest, RobustAccessOn) {
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableAMDValidation))
    InitState();
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-vkCreateDevice-RobustBufferAccess");

    VkPhysicalDeviceFeatures features = {};
    features.robustBufferAccess = true;

    const float q_priority[] = {1.0f};
    VkDeviceQueueCreateInfo queue_ci = {};
    queue_ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_ci.queueFamilyIndex = 0;
    queue_ci.queueCount = 1;
    queue_ci.pQueuePriorities = q_priority;

    VkDeviceCreateInfo device_ci = {};
    device_ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &queue_ci;
    device_ci.pEnabledFeatures = &features;

    VkDevice test_device;
    vk::CreateDevice(gpu(), &device_ci, nullptr, &test_device);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkAmdBestPracticesLayerTest, Barriers) {
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableAMDValidation))
    InitState();
    InitRenderTarget();

    VkImageCreateInfo img_info = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        nullptr,
        VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
        VK_IMAGE_TYPE_1D,
        VK_FORMAT_R8G8B8A8_UNORM,
        {1, 1, 1},
        1,
        1,
        VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageObj image_1D(m_device);
    image_1D.init(&img_info);
    ASSERT_TRUE(image_1D.initialized());

    m_commandBuffer->begin();
    // check for read-to-read barrier
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-PipelineBarrier-readToReadBarrier");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-CmdBuffer-backToBackBarrier");  // we already test for this above
    image_1D.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_1D.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetUnexpectedError("VUID-VkImageMemoryBarrier-oldLayout-01197");

    // check total number of barriers warning
    uint32_t warn_Limit = 250;
    for (uint32_t i = 0; i < warn_Limit; i++) {
        image_1D.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        image_1D.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    }

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-CmdBuffer-highBarrierCount");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-CmdBuffer-backToBackBarrier");  // we already test for this above
    image_1D.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkAmdBestPracticesLayerTest, NumberOfSubmissions) {
    AddSurfaceExtension();

    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableAMDValidation))

    InitState();
    InitSwapchain();
    InitRenderTarget();

    VkImageCreateInfo img_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                 nullptr,
                                 VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
                                 VK_IMAGE_TYPE_1D,
                                 VK_FORMAT_R8G8B8A8_UNORM,
                                 {1, 1, 1},
                                 1,
                                 1,
                                 VK_SAMPLE_COUNT_1_BIT,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                 VK_SHARING_MODE_EXCLUSIVE,
                                 0,
                                 nullptr,
                                 VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageObj image_1D(m_device);
    image_1D.init(&img_info);
    ASSERT_TRUE(image_1D.initialized());

    uint32_t warn_limit = 11;

    for (uint32_t i = 0; i < warn_limit; i++) {
        image_1D.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        image_1D.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    }

    uint32_t current_buffer;
    vkt::Semaphore image_acquired(*m_device);
    ASSERT_TRUE(image_acquired.initialized());
    vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, image_acquired.handle(), VK_NULL_HANDLE, &current_buffer);

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 0;
    present_info.pWaitSemaphores = &image_acquired.handle();
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &m_swapchain;
    present_info.pImageIndices = &current_buffer;
    present_info.pResults = NULL;

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-Submission-ReduceNumberOfSubmissions");
    m_errorMonitor->SetUnexpectedError("VUID-VkPresentInfoKHR-pImageIndices-01430");

    vk::QueuePresentKHR(m_default_queue, &present_info);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkAmdBestPracticesLayerTest, NumSyncPrimitives) {
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableAMDValidation))
    InitState();
    InitRenderTarget();

    constexpr int fence_warn_limit = 5;
    const auto fence_ci = vkt::Fence::create_info();
    std::vector<vkt::Fence> test_fences(fence_warn_limit);
    for (int i = 0; i < fence_warn_limit - 1; ++i) {
        test_fences[i].init(*m_device, fence_ci);
    }
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-SyncObjects-HighNumberOfFences");
    test_fences[fence_warn_limit - 1].init(*m_device, fence_ci);
    m_errorMonitor->VerifyFound();

    constexpr int semaphore_warn_limit = 12;
    const VkSemaphoreCreateInfo semaphore_ci = vku::InitStructHelper();
    std::vector<vkt::Semaphore> test_semaphores(semaphore_warn_limit);
    for (int i = 0; i < semaphore_warn_limit - 1; ++i) {
        test_semaphores[i].init(*m_device, semaphore_ci);
    }
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-SyncObjects-HighNumberOfSemaphores");
    test_semaphores[semaphore_warn_limit - 1].init(*m_device, semaphore_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkAmdBestPracticesLayerTest, SecondaryCmdBuffer) {
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableAMDValidation))
    InitState();

    InitRenderTarget();

    VkPipelineMultisampleStateCreateInfo pipe_ms_state_ci = {};
    pipe_ms_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipe_ms_state_ci.pNext = NULL;
    pipe_ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipe_ms_state_ci.sampleShadingEnable = 0;
    pipe_ms_state_ci.minSampleShading = 1.0;
    pipe_ms_state_ci.pSampleMask = NULL;

    vkt::Buffer vertex_buffer(*m_device, 64, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.pipe_ms_state_ci_ = pipe_ms_state_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    vkt::CommandPool pool(*m_device, m_device->graphics_queue_node_index_);
    vkt::CommandBuffer secondary_cmd_buf(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceInfo iinfo = vku::InitStructHelper();
    iinfo.renderPass = m_renderPassBeginInfo.renderPass;

    VkCommandBufferBeginInfo binfo = vku::InitStructHelper();
    binfo.pInheritanceInfo = &iinfo;
    binfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    // record a secondary command buffer
    secondary_cmd_buf.begin(&binfo);

    vk::CmdBindPipeline(secondary_cmd_buf.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    VkDeviceSize offset = 0;
    vk::CmdBindVertexBuffers(secondary_cmd_buf.handle(), 0, 1, &vertex_buffer.handle(), &offset);
    vk::CmdDraw(secondary_cmd_buf.handle(), 1, 0, 0, 0);
    vk::CmdDraw(secondary_cmd_buf.handle(), 1, 0, 0, 0);
    vk::CmdDraw(secondary_cmd_buf.handle(), 1, 0, 0, 0);
    vk::CmdDraw(secondary_cmd_buf.handle(), 1, 0, 0, 0);

    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 1.0;
    color_attachment.clearValue.color.float32[1] = 1.0;
    color_attachment.clearValue.color.float32[2] = 1.0;
    color_attachment.clearValue.color.float32[3] = 1.0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {m_width, m_height}}, 0, 1};

    vk::CmdClearAttachments(secondary_cmd_buf.handle(), 1, &color_attachment, 1, &clear_rect);

    secondary_cmd_buf.end();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-DrawState-ClearCmdBeforeDraw");
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-VkCommandBuffer-AvoidSecondaryCmdBuffers");

    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_cmd_buf.handle());

    m_errorMonitor->VerifyFound();
}

TEST_F(VkAmdBestPracticesLayerTest, ComputeWorkgroupSize) {
    TEST_DESCRIPTION("On AMD make the workgroup size a multiple of 64 to obtain best performance across all GPU generations.");

    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableAMDValidation))
    InitState();

    CreateComputePipelineHelper pipe(*this);

    auto make_pipeline_with_shader = [=](CreateComputePipelineHelper& pipe, const VkPipelineShaderStageCreateInfo& stage) {
        pipe.InitState();
        pipe.cp_ci_.stage = stage;
        pipe.dsl_bindings_ = {};
        pipe.cp_ci_.layout = pipe.pipeline_layout_.handle();
        pipe.CreateComputePipeline(false);
    };

    // workgroup size = 4
    {
        VkShaderObj compute_4_1_1(this,
                                  "#version 320 es\n"
                                  "\n"
                                  "layout(local_size_x = 4, local_size_y = 1, local_size_z = 1) in;\n\n"
                                  "void main() {}\n",
                                  VK_SHADER_STAGE_COMPUTE_BIT);
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                             "UNASSIGNED-BestPractices-LocalWorkgroup-Multiple64");
        make_pipeline_with_shader(pipe, compute_4_1_1.GetStageCreateInfo());
        m_errorMonitor->VerifyFound();
    }

    // workgroup size = 64
    {
        VkShaderObj compute_8_8_1(this,
                                  "#version 320 es\n"
                                  "\n"
                                  "layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;\n\n"
                                  "void main() {}\n",
                                  VK_SHADER_STAGE_COMPUTE_BIT);
        make_pipeline_with_shader(pipe, compute_8_8_1.GetStageCreateInfo());
    }

    // workgroup size = 128
    {
        VkShaderObj compute_16_8_1(this,
                                   "#version 320 es\n"
                                   "\n"
                                   "layout(local_size_x = 16, local_size_y = 8, local_size_z = 1) in;\n\n"
                                   "void main() {}\n",
                                   VK_SHADER_STAGE_COMPUTE_BIT);
        make_pipeline_with_shader(pipe, compute_16_8_1.GetStageCreateInfo());
    }
}
