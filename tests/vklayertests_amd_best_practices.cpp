/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Nathaniel Cesario <nathaniel@lunarg.com>
 * Author: Nadav Geva <nadav.geva@amd.com>
 */

#include "cast_utils.h"
#include "layer_validation_tests.h"

// Tests for AMD-specific best practices
const char *kEnableAMDValidation = "VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_AMD";


// this is a very long test (~10 minutes)
// disabled for now
#ifdef AMD_LONG_RUNNING_TEST
TEST_F(VkAmdBestPracticesLayerTest, TooManyPipelines) {
    

    InitBestPracticesFramework(kEnableAMDValidation);
    InitState();

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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
        pipe.InitInfo();
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
    InitBestPracticesFramework(kEnableAMDValidation);
    InitState();

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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
               VK_FORMAT_R8G8B8A8_UNORM,
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
    InitBestPracticesFramework(kEnableAMDValidation);
    InitState();

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-vkImage-AvoidConcurrentRenderTargets");

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
                                 0,
                                 nullptr,
                                 VK_IMAGE_LAYOUT_UNDEFINED};
    VkImage test_image = VK_NULL_HANDLE;
    vk::CreateImage(m_device->handle(), &img_info, nullptr, &test_image);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-vkImage-AvoidConcurrentRenderTargets");
    // create a render target image with mutable bit set
    img_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
               nullptr,
               0,
               VK_IMAGE_TYPE_1D,
               VK_FORMAT_R8G8B8A8_UNORM,
               {1, 1, 1},
               1,
               1,
               VK_SAMPLE_COUNT_1_BIT,
               VK_IMAGE_TILING_OPTIMAL,
               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
               VK_SHARING_MODE_CONCURRENT,
               0,
               nullptr,
               VK_IMAGE_LAYOUT_UNDEFINED};
    test_image = VK_NULL_HANDLE;
    vk::CreateImage(m_device->handle(), &img_info, nullptr, &test_image);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkAmdBestPracticesLayerTest, UsageStorageRT) {
    InitBestPracticesFramework(kEnableAMDValidation);
    InitState();

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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
    InitBestPracticesFramework(kEnableAMDValidation);
    InitState();

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-CreatePipelines-AvoidPrimitiveRestart");

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.ia_ci_.primitiveRestartEnable = true;
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkAmdBestPracticesLayerTest, NumDynamicStates) {
    InitBestPracticesFramework(kEnableAMDValidation);
    InitState();

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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
    pipe.InitInfo();
    pipe.InitState();
    pipe.dyn_state_ci_ = dynamic_state_info;
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkAmdBestPracticesLayerTest, KeepLayoutSmall) {
    // TODO: add dynamic buffer check as well
    InitBestPracticesFramework(kEnableAMDValidation);
    InitState();

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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

    VkDescriptorSetLayout DS_layout;

    VkDescriptorSetLayoutCreateInfo DS_layout_info = {};
    DS_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    DS_layout_info.bindingCount = 1;
    DS_layout_info.pBindings = &binding;

    vk::CreateDescriptorSetLayout(m_device->device(), &DS_layout_info, nullptr, &DS_layout);

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &DS_layout;
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_range;

    VkPipelineLayout test_pipeline_layout = VK_NULL_HANDLE;
    vk::CreatePipelineLayout(m_device->handle(), &pipeline_layout_info, nullptr, &test_pipeline_layout);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkAmdBestPracticesLayerTest, CopyingDescriptors) {
    // TODO: add dynamic buffer check as well
    InitBestPracticesFramework(kEnableAMDValidation);
    InitState();

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-UpdateDescriptors-AvoidCopyingDescriptors");

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = {};
    ds_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ds_pool_ci.pNext = NULL;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    VkDescriptorPool ds_pool;
    vk::CreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &ds_pool);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 2;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutObj ds_layout(m_device, {dsl_binding});

    VkDescriptorSet descriptor_sets[3] = {};
    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool;
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

InitBestPracticesFramework(kEnableAMDValidation);
    InitState();
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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
                                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                     VK_SHARING_MODE_EXCLUSIVE,
                                     0,
                                     nullptr,
                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL};
        VkImageObj image_1D(m_device);
        image_1D.init(&img_info);
        ASSERT_TRUE(image_1D.initialized());

        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

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

        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }

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
                                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                     VK_SHARING_MODE_EXCLUSIVE,
                                     0,
                                     nullptr,
                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL};
        VkImageObj image_1D(m_device);
        image_1D.init(&img_info);
        ASSERT_TRUE(image_1D.initialized());

        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

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

        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }
}

TEST_F(VkAmdBestPracticesLayerTest, ImageToImageCopy) {
    InitBestPracticesFramework(kEnableAMDValidation);
    InitState();
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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
                                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
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
        printf("%s Could not initilize Linear image, skipping image to image copy test\n", kSkipPrefix);
        return;
    }

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    image1D_1.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-vkImage-AvoidImageToImageCopy");

    vk::CmdCopyImage(m_commandBuffer->handle(), image1D_1.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image_1D_2.handle(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkAmdBestPracticesLayerTest, GeneralLayout) {
    InitBestPracticesFramework(kEnableAMDValidation);
    InitState();
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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
    InitBestPracticesFramework(kEnableAMDValidation);
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
    InitBestPracticesFramework(kEnableAMDValidation);
    InitState();
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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

    m_commandBuffer->begin();
    // check for read-to-read barrier
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-PipelineBarrier-readToReadBarrier");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-CmdBuffer-backToBackBarrier");  // we already test for this above
    image_1D.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_1D.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    m_errorMonitor->VerifyFound();

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
}

TEST_F(VkAmdBestPracticesLayerTest, NumberOfSubmissions) {
    AddSurfaceInstanceExtension();

    InitBestPracticesFramework(kEnableAMDValidation);
    AddSwapchainDeviceExtension();

    InitState();
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    InitSwapchain();
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

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

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 0;
    // presentInfo.pWaitSemaphores = &(m_RenderFinishedSemaphores[m_imageIndex]);
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &m_swapchain;
    present_info.pImageIndices = 0;
    present_info.pResults = NULL;

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-Submission-ReduceNumberOfSubmissions");

    vk::QueuePresentKHR(m_device->GetDefaultQueue()->handle(), &present_info);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkAmdBestPracticesLayerTest, NumSyncPrimitives) {
    InitBestPracticesFramework(kEnableAMDValidation);
    InitState();
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-SyncObjects-HighNumberOfFences");
    uint32_t fence_warn_limit = 5;
    for (uint32_t i = 0; i < fence_warn_limit; i++) {
        VkFence testFence;
        VkFenceCreateInfo fenceInfo = VkFenceObj::create_info();
        vk::CreateFence(m_device->device(), &fenceInfo, nullptr, &testFence);
    }
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-SyncObjects-HighNumberOfSemaphores");
    uint32_t semaphore_warn_limit = 12;
    for (uint32_t i = 0; i < semaphore_warn_limit; i++) {
        VkSemaphore test_semaphore;
        VkSemaphoreCreateInfo semaphore_create_info = {};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        vk::CreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &test_semaphore);
    }

    m_errorMonitor->VerifyFound();
}

TEST_F(VkAmdBestPracticesLayerTest, SecondaryCmdBuffer) {
    InitBestPracticesFramework(kEnableAMDValidation);
    InitState();

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPipelineMultisampleStateCreateInfo pipe_ms_state_ci = {};
    pipe_ms_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipe_ms_state_ci.pNext = NULL;
    pipe_ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipe_ms_state_ci.sampleShadingEnable = 0;
    pipe_ms_state_ci.minSampleShading = 1.0;
    pipe_ms_state_ci.pSampleMask = NULL;

    const float vbo_data[3] = {1.f, 0.f, 1.f};
    VkVerticesObj vertex_buffer(m_device, 1, 1, sizeof(vbo_data), 1, vbo_data);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.pipe_ms_state_ci_ = pipe_ms_state_ci;
    pipe.InitState();

    vertex_buffer.AddVertexInputToPipeHelpr(&pipe);

    pipe.CreateGraphicsPipeline();

    VkCommandPoolObj pool(m_device, m_device->graphics_queue_node_index_);
    VkCommandBufferObj secondary_cmd_buf(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    // record a secondary command buffer
    secondary_cmd_buf.begin();
    secondary_cmd_buf.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(secondary_cmd_buf.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vertex_buffer.BindVertexBuffers(secondary_cmd_buf.handle());
    secondary_cmd_buf.Draw(1, 0, 0, 0);
    secondary_cmd_buf.Draw(1, 0, 0, 0);
    secondary_cmd_buf.Draw(1, 0, 0, 0);
    secondary_cmd_buf.Draw(1, 0, 0, 0);

    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 1.0;
    color_attachment.clearValue.color.float32[1] = 1.0;
    color_attachment.clearValue.color.float32[2] = 1.0;
    color_attachment.clearValue.color.float32[3] = 1.0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {(uint32_t)m_width, (uint32_t)m_height}}, 0, 1};

    vk::CmdClearAttachments(secondary_cmd_buf.handle(), 1, &color_attachment, 1, &clear_rect);

    secondary_cmd_buf.EndRenderPass();
    secondary_cmd_buf.end();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-DrawState-ClearCmdBeforeDraw");
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-VkCommandBuffer-AvoidSecondaryCmdBuffers");

    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_cmd_buf.handle());

    m_errorMonitor->VerifyFound();
}