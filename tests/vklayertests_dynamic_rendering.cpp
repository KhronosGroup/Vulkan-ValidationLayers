/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (c) 2015-2022 Google, Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2021-2022 ARM, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include "cast_utils.h"
#include "layer_validation_tests.h"

TEST_F(VkLayerTest, DynamicRenderingCommandBufferInheritanceRenderingInfo) {
    TEST_DESCRIPTION("VkCommandBufferInheritanceRenderingInfoKHR Dynamic Rendering Tests.");

    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_2);
    if (version < VK_API_VERSION_1_2) {
        printf("%s At least Vulkan version 1.2 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequestedExtensionsEnabled()) {
        GTEST_SKIP() << RequestedExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    features2.features.variableMultisampleRate = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    VkPhysicalDeviceMultiviewProperties multiview_props = LvlInitStruct<VkPhysicalDeviceMultiviewProperties>();
    VkPhysicalDeviceProperties2 pd_props2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&multiview_props);
    vk::GetPhysicalDeviceProperties2(gpu(), &pd_props2);

    if (multiview_props.maxMultiviewViewCount == 32) {
        printf("%s VUID is not testable as maxMultiviewViewCount is 32, skipping test\n", kSkipPrefix);
        return;
    }

    VkFormat color_format = VK_FORMAT_D32_SFLOAT;

    auto cmd_buffer_inheritance_rendering_info = LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    cmd_buffer_inheritance_rendering_info.colorAttachmentCount = 1;
    cmd_buffer_inheritance_rendering_info.pColorAttachmentFormats = &color_format;
    cmd_buffer_inheritance_rendering_info.depthAttachmentFormat = VK_FORMAT_R8G8B8_UNORM;
    cmd_buffer_inheritance_rendering_info.stencilAttachmentFormat = VK_FORMAT_R8G8B8_SNORM;
    cmd_buffer_inheritance_rendering_info.viewMask = 1 << multiview_props.maxMultiviewViewCount;

    auto sample_count_info_amd = LvlInitStruct<VkAttachmentSampleCountInfoAMD>();
    sample_count_info_amd.pNext = &cmd_buffer_inheritance_rendering_info;
    sample_count_info_amd.colorAttachmentCount = 2;

    auto cmd_buffer_inheritance_info = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    cmd_buffer_inheritance_info.pNext = &sample_count_info_amd;

    auto cmd_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    cmd_buffer_allocate_info.commandPool = m_commandPool->handle();
    cmd_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    cmd_buffer_allocate_info.commandBufferCount = 0x1;

    VkCommandBuffer secondary_cmd_buffer;
    VkResult err = vk::AllocateCommandBuffers(m_device->device(), &cmd_buffer_allocate_info, &secondary_cmd_buffer);
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferBeginInfo-flags-06003");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-colorAttachmentCount-06004");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-variableMultisampleRate-06005");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-depthAttachmentFormat-06007");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-multiview-06008");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-viewMask-06009");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-stencilAttachmentFormat-06199");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-depthAttachmentFormat-06200");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-pColorAttachmentFormats-06006");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-depthAttachmentFormat-06540");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-stencilAttachmentFormat-06541");

    VkCommandBufferBeginInfo cmd_buffer_begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cmd_buffer_begin_info.pInheritanceInfo = &cmd_buffer_inheritance_info;
    vk::BeginCommandBuffer(secondary_cmd_buffer, &cmd_buffer_begin_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DynamicRenderingCommandDraw) {
    TEST_DESCRIPTION("vkCmdDraw* Dynamic Rendering Tests.");

    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_2);
    if (version < VK_API_VERSION_1_2) {
        printf("%s At least Vulkan version 1.2 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequestedExtensionsEnabled()) {
        GTEST_SKIP() << RequestedExtensionsNotSupported() << " not supported";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    VkFormat depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.depthAttachmentFormat = depth_format;
    pipeline_rendering_info.stencilAttachmentFormat = depth_format;

    auto multisample_state_create_info = LvlInitStruct<VkPipelineMultisampleStateCreateInfo>();
    multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;

    auto create_info = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe.InitGraphicsPipelineCreateInfo(&create_info);
    create_info.pMultisampleState = &multisample_state_create_info;
    create_info.renderPass = VkRenderPass(0x1);
    create_info.pNext = &pipeline_rendering_info;

    VkResult err = pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    ASSERT_VK_SUCCESS(err);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};

    VkImageObj image(m_device);
    image.Init(32, 32, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_LINEAR, 0);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo ivci = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                  nullptr,
                                  0,
                                  image.handle(),
                                  VK_IMAGE_VIEW_TYPE_2D,
                                  depth_format,
                                  {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                   VK_COMPONENT_SWIZZLE_IDENTITY},
                                  {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1}};

    VkImageView depth_image_view;
    err = vk::CreateImageView(m_device->device(), &ivci, nullptr, &depth_image_view);
    ASSERT_VK_SUCCESS(err);

    VkRenderingAttachmentInfoKHR depth_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depth_attachment.imageView = depth_image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.pDepthAttachment = &depth_attachment;
    begin_rendering_info.pStencilAttachment = &depth_attachment;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-pDepthAttachment-06189");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-pStencilAttachment-06190");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, DynamicRenderingGraphicsPipelineCreateInfo) {
    TEST_DESCRIPTION("Test graphics pipeline creation with dynamic rendering.");
    m_errorMonitor->ExpectSuccess();

    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_2);
    if (version < VK_API_VERSION_1_2) {
        printf("%s At least Vulkan version 1.2 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

     if (!AreRequestedExtensionsEnabled()) {
         GTEST_SKIP() << RequestedExtensionsNotSupported() << " not supported";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const VkPipelineLayoutObj pl(m_device);
    VkPipelineObj pipe(m_device);

    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};

    auto color_blend_state_create_info = LvlInitStruct<VkPipelineColorBlendStateCreateInfo>();
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments = &color_blend_attachment_state;

    VkFormat color_format[2] = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT};

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 2;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format[0];
    pipeline_rendering_info.viewMask = 0x2;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

    auto pipeline_tessellation_state_info = LvlInitStruct<VkPipelineTessellationStateCreateInfo>();
    pipeline_tessellation_state_info.patchControlPoints = 1;

    auto pipeline_input_assembly_state_info = LvlInitStruct<VkPipelineInputAssemblyStateCreateInfo>();
    pipeline_input_assembly_state_info.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;

    auto create_info = LvlInitStruct<VkGraphicsPipelineCreateInfo>();

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, bindStateGeomShaderText, VK_SHADER_STAGE_GEOMETRY_BIT);
    VkShaderObj te(this, bindStateTeshaderText, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    VkShaderObj tc(this, bindStateTscShaderText, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    pipe.AddShader(&vs);
    pipe.AddShader(&gs);
    pipe.AddShader(&te);
    pipe.AddShader(&tc);
    pipe.AddShader(&fs);
    pipe.InitGraphicsPipelineCreateInfo(&create_info);
    m_errorMonitor->VerifyNotFound();

    create_info.pColorBlendState = &color_blend_state_create_info;
    create_info.pNext = &pipeline_rendering_info;
    create_info.pTessellationState = &pipeline_tessellation_state_info;
    create_info.pInputAssemblyState = &pipeline_input_assembly_state_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06053");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06581");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06055");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06057");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06058");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06060");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-multiview-06577");
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();

    create_info.pColorBlendState = nullptr;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipeline_rendering_info.viewMask = 0x0;
    pipeline_rendering_info.colorAttachmentCount = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06054");
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();

    color_format[0] = VK_FORMAT_D32_SFLOAT_S8_UINT;
    color_blend_attachment_state.blendEnable = VK_TRUE;
    create_info.pColorBlendState = &color_blend_state_create_info;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06581");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06062");
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();
    color_format[0] = VK_FORMAT_R8G8B8A8_UNORM;

    auto ds_ci = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
    ds_ci.flags = VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_ARM;
    color_blend_state_create_info.flags = VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_BIT_ARM;
    create_info.pColorBlendState = &color_blend_state_create_info;
    create_info.pDepthStencilState = &ds_ci;
    create_info.renderPass = VK_NULL_HANDLE;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-06482");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-06483");
    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DynamicRenderingWithMismatchingViewMask) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering and a mismatching viewMask");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequestedExtensionsEnabled()) {
        GTEST_SKIP() << RequestedExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>(&multiview_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }
    if (!multiview_features.multiview) {
        printf("%s Test requires (unsupported) multiview , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        void main() {
           color = vec4(1.0f);
        }
    )glsl";

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};
    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;
    pipeline_rendering_info.viewMask = 1;

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    m_viewports.push_back(viewport);
    m_scissors.push_back(scissor);
    pipe.SetViewport(m_viewports);
    pipe.SetScissor(m_scissors);

    auto create_info = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe.InitGraphicsPipelineCreateInfo(&create_info);
    create_info.pNext = &pipeline_rendering_info;

    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.viewMask = 2;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-viewMask-06178");
    m_commandBuffer->Draw(1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, DynamicRenderingWithMistmatchingAttachments) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering with mismatching color attachment counts and depth/stencil formats");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequestedExtensionsEnabled()) {
        GTEST_SKIP() << RequestedExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        void main() {
           color = vec4(1.0f);
        }
    )glsl";

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    m_viewports.push_back(viewport);
    m_scissors.push_back(scissor);

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();

    VkPipelineObj pipe1(m_device);
    pipe1.AddShader(&vs);
    pipe1.AddShader(&fs);
    pipe1.AddDefaultColorAttachment();
    pipe1.SetViewport(m_viewports);
    pipe1.SetScissor(m_scissors);

    VkFormat color_formats[] = {VK_FORMAT_R8G8B8A8_UNORM};
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = color_formats;

    auto create_info1 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe1.InitGraphicsPipelineCreateInfo(&create_info1);
    create_info1.pNext = &pipeline_rendering_info;

    pipe1.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info1);

    VkPipelineObj pipe2(m_device);
    pipe2.AddShader(&vs);
    pipe2.AddShader(&fs);
    pipe2.AddDefaultColorAttachment();
    pipe2.SetViewport(m_viewports);
    pipe2.SetScissor(m_scissors);

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.pColorAttachmentFormats = nullptr;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_D16_UNORM;

    auto create_info2 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe2.InitGraphicsPipelineCreateInfo(&create_info2);
    create_info2.pNext = &pipeline_rendering_info;

    pipe2.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info2);

    VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(gpu());
    ASSERT_TRUE(depthStencilFormat != 0);

    bool testStencil = false;
    VkFormat stencilFormat = VK_FORMAT_UNDEFINED;

    if (ImageFormatIsSupported(gpu(), VK_FORMAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL)) {
        stencilFormat = VK_FORMAT_S8_UINT;
        testStencil = true;
    } else if ((depthStencilFormat != VK_FORMAT_D16_UNORM_S8_UINT) &&
               ImageFormatIsSupported(gpu(), VK_FORMAT_D16_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL)) {
        stencilFormat = VK_FORMAT_D16_UNORM_S8_UINT;
        testStencil = true;
    } else if ((depthStencilFormat != VK_FORMAT_D24_UNORM_S8_UINT) &&
               ImageFormatIsSupported(gpu(), VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL)) {
        stencilFormat = VK_FORMAT_D24_UNORM_S8_UINT;
        testStencil = true;
    } else if ((depthStencilFormat != VK_FORMAT_D32_SFLOAT_S8_UINT) &&
               ImageFormatIsSupported(gpu(), VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL)) {
        stencilFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
        testStencil = true;
    }

    VkPipelineObj pipe3(m_device);

    if (testStencil) {
        pipe3.AddShader(&vs);
        pipe3.AddShader(&fs);
        pipe3.AddDefaultColorAttachment();
        pipe3.SetViewport(m_viewports);
        pipe3.SetScissor(m_scissors);

        pipeline_rendering_info.colorAttachmentCount = 0;
        pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
        pipeline_rendering_info.stencilAttachmentFormat = stencilFormat;

        auto create_info3 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
        pipe3.InitGraphicsPipelineCreateInfo(&create_info3);
        create_info3.pNext = &pipeline_rendering_info;

        pipe3.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info3);
    }

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView colorImageView = colorImage.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkImageObj depthStencilImage(m_device);
    depthStencilImage.Init(32, 32, 1, depthStencilFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkImageView depthStencilImageView =
        depthStencilImage.targetView(depthStencilFormat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = colorImageView;

    VkRenderingAttachmentInfoKHR depth_stencil_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = depthStencilImageView;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    m_commandBuffer->begin();

    // Mismatching color attachment count
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-colorAttachmentCount-06179");
    m_commandBuffer->Draw(1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Mismatching color formats
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-colorAttachmentCount-06180");
    m_commandBuffer->Draw(1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Mismatching depth format
    begin_rendering_info.colorAttachmentCount = 0;
    begin_rendering_info.pColorAttachments = nullptr;
    begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-pDepthAttachment-06181");
    m_commandBuffer->Draw(1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Mismatching stencil format
    if (testStencil) {
        begin_rendering_info.pDepthAttachment = nullptr;
        begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
        m_commandBuffer->BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe3.handle());
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-pStencilAttachment-06182");
        m_commandBuffer->Draw(1, 1, 0, 0);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->EndRendering();
    }

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, DynamicRenderingWithMistmatchingAttachmentSamples) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering with mismatching color/depth/stencil sample counts");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequestedExtensionsEnabled()) {
        GTEST_SKIP() << RequestedExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        void main() {
           color = vec4(1.0f);
        }
    )glsl";

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    m_viewports.push_back(viewport);
    m_scissors.push_back(scissor);

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();

    VkPipelineObj pipe1(m_device);
    pipe1.AddShader(&vs);
    pipe1.AddShader(&fs);
    pipe1.AddDefaultColorAttachment();
    pipe1.SetViewport(m_viewports);
    pipe1.SetScissor(m_scissors);

    VkFormat color_formats[] = {VK_FORMAT_R8G8B8A8_UNORM};
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = color_formats;

    auto create_info1 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe1.InitGraphicsPipelineCreateInfo(&create_info1);
    create_info1.pNext = &pipeline_rendering_info;

    auto multisample_state_create_info = LvlInitStruct<VkPipelineMultisampleStateCreateInfo>();
    multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;
    create_info1.pMultisampleState = &multisample_state_create_info;

    pipe1.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info1);

    VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(gpu());
    ASSERT_TRUE(depthStencilFormat != 0);

    VkPipelineObj pipe2(m_device);
    pipe2.AddShader(&vs);
    pipe2.AddShader(&fs);
    pipe2.AddDefaultColorAttachment();
    pipe2.SetViewport(m_viewports);
    pipe2.SetScissor(m_scissors);

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.pColorAttachmentFormats = nullptr;
    pipeline_rendering_info.depthAttachmentFormat = depthStencilFormat;

    auto create_info2 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe2.InitGraphicsPipelineCreateInfo(&create_info2);
    create_info2.pNext = &pipeline_rendering_info;

    create_info2.pMultisampleState = &multisample_state_create_info;

    pipe2.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info2);

    VkPipelineObj pipe3(m_device);

    pipe3.AddShader(&vs);
    pipe3.AddShader(&fs);
    pipe3.AddDefaultColorAttachment();
    pipe3.SetViewport(m_viewports);
    pipe3.SetScissor(m_scissors);

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipeline_rendering_info.stencilAttachmentFormat = depthStencilFormat;

    auto create_info3 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe3.InitGraphicsPipelineCreateInfo(&create_info3);
    create_info3.pNext = &pipeline_rendering_info;

    create_info3.pMultisampleState = &multisample_state_create_info;

    pipe3.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info3);

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView colorImageView = colorImage.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkImageObj depthStencilImage(m_device);
    depthStencilImage.Init(32, 32, 1, depthStencilFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkImageView depthStencilImageView =
        depthStencilImage.targetView(depthStencilFormat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = colorImageView;

    VkRenderingAttachmentInfoKHR depth_stencil_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = depthStencilImageView;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    m_commandBuffer->begin();

    // Mismatching color samples
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-colorAttachmentCount-06188");
    m_commandBuffer->Draw(1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Mismatching depth samples
    begin_rendering_info.colorAttachmentCount = 0;
    begin_rendering_info.pColorAttachments = nullptr;
    begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-pDepthAttachment-06189");
    m_commandBuffer->Draw(1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Mismatching stencil samples
    begin_rendering_info.pDepthAttachment = nullptr;
    begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe3.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-pStencilAttachment-06190");
    m_commandBuffer->Draw(1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, DynamicRenderingWithMismatchingMixedAttachmentSamples) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering with mismatching mixed color/depth/stencil sample counts");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequestedExtensionsEnabled()) {
        GTEST_SKIP() << RequestedExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    bool amd_samples = false;
    if (DeviceExtensionSupported(gpu(), nullptr, VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
        amd_samples = true;
    }

    bool nv_samples = false;
    if (DeviceExtensionSupported(gpu(), nullptr, VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
        nv_samples = true;
    }

    if (!amd_samples && !nv_samples) {
        printf("%s Test requires either VK_AMD_mixed_attachment_samples or VK_NV_framebuffer_mixed_samples, skipping\n",
               kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        void main() {
           color = vec4(1.0f);
        }
    )glsl";

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    m_viewports.push_back(viewport);
    m_scissors.push_back(scissor);

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    VkSampleCountFlagBits counts[2] = {VK_SAMPLE_COUNT_2_BIT, VK_SAMPLE_COUNT_2_BIT};
    auto samples_info = LvlInitStruct<VkAttachmentSampleCountInfoAMD>();

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>(&samples_info);

    VkPipelineObj pipe1(m_device);
    pipe1.AddShader(&vs);
    pipe1.AddShader(&fs);
    pipe1.AddDefaultColorAttachment();
    pipe1.SetViewport(m_viewports);
    pipe1.SetScissor(m_scissors);

    VkFormat color_formats[] = {VK_FORMAT_R8G8B8A8_UNORM};
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = color_formats;

    samples_info.colorAttachmentCount = 1;
    samples_info.pColorAttachmentSamples = counts;

    auto create_info1 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe1.InitGraphicsPipelineCreateInfo(&create_info1);
    create_info1.pNext = &pipeline_rendering_info;

    samples_info.colorAttachmentCount = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06063");
    pipe1.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info1);
    m_errorMonitor->VerifyFound();

    samples_info.colorAttachmentCount = 1;
    pipe1.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info1);

    VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(gpu());
    ASSERT_TRUE(depthStencilFormat != 0);

    VkPipelineObj pipe2(m_device);
    pipe2.AddShader(&vs);
    pipe2.AddShader(&fs);
    pipe2.AddDefaultColorAttachment();
    pipe2.SetViewport(m_viewports);
    pipe2.SetScissor(m_scissors);

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.pColorAttachmentFormats = nullptr;
    pipeline_rendering_info.depthAttachmentFormat = depthStencilFormat;

    samples_info.colorAttachmentCount = 0;
    samples_info.pColorAttachmentSamples = nullptr;
    samples_info.depthStencilAttachmentSamples = counts[0];

    auto create_info2 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe2.InitGraphicsPipelineCreateInfo(&create_info2);
    create_info2.pNext = &pipeline_rendering_info;

    pipe2.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info2);

    VkPipelineObj pipe3(m_device);

    pipe3.AddShader(&vs);
    pipe3.AddShader(&fs);
    pipe3.AddDefaultColorAttachment();
    pipe3.SetViewport(m_viewports);
    pipe3.SetScissor(m_scissors);

    pipeline_rendering_info.colorAttachmentCount = 0;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    pipeline_rendering_info.stencilAttachmentFormat = depthStencilFormat;

    auto create_info3 = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe3.InitGraphicsPipelineCreateInfo(&create_info3);
    create_info3.pNext = &pipeline_rendering_info;

    pipe3.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info3);

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView colorImageView = colorImage.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkImageObj depthStencilImage(m_device);
    depthStencilImage.Init(32, 32, 1, depthStencilFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkImageView depthStencilImageView =
        depthStencilImage.targetView(depthStencilFormat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = colorImageView;

    VkRenderingAttachmentInfoKHR depth_stencil_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_stencil_attachment.imageView = depthStencilImageView;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    m_commandBuffer->begin();

    // Mismatching color samples
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-colorAttachmentCount-06185");
    m_commandBuffer->Draw(1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Mismatching depth samples
    begin_rendering_info.colorAttachmentCount = 0;
    begin_rendering_info.pColorAttachments = nullptr;
    begin_rendering_info.pDepthAttachment = &depth_stencil_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-pDepthAttachment-06186");
    m_commandBuffer->Draw(1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    // Mismatching stencil samples
    begin_rendering_info.pDepthAttachment = nullptr;
    begin_rendering_info.pStencilAttachment = &depth_stencil_attachment;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe3.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-pStencilAttachment-06187");
    m_commandBuffer->Draw(1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, DynamicRenderingAttachmentInfo) {
    TEST_DESCRIPTION("AttachmentInfo Dynamic Rendering Tests.");

    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_2);
    if (version < VK_API_VERSION_1_2) {
        printf("%s At least Vulkan version 1.2 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequestedExtensionsEnabled()) {
        GTEST_SKIP() << RequestedExtensionsNotSupported() << " not supported";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    VkFormat depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.depthAttachmentFormat = depth_format;

    auto pipeline_create_info = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe.InitGraphicsPipelineCreateInfo(&pipeline_create_info);
    pipeline_create_info.pNext = &pipeline_rendering_info;

    VkResult err = pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &pipeline_create_info);
    ASSERT_VK_SUCCESS(err);

    VkImageObj image(m_device);
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = depth_format;
    image_create_info.extent = {64, 64, 4};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    image.Init(image_create_info);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo ivci = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                  nullptr,
                                  0,
                                  image.handle(),
                                  VK_IMAGE_VIEW_TYPE_2D,
                                  depth_format,
                                  {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                   VK_COMPONENT_SWIZZLE_IDENTITY},
                                  {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1}};

    VkImageView depth_image_view;
    err = vk::CreateImageView(m_device->device(), &ivci, nullptr, &depth_image_view);
    ASSERT_VK_SUCCESS(err);

    VkRenderingAttachmentInfoKHR depth_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    depth_attachment.imageView = depth_image_view;
    depth_attachment.resolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
    depth_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.pDepthAttachment = &depth_attachment;
    begin_rendering_info.viewMask = 0x4;

    VkRenderingFragmentDensityMapAttachmentInfoEXT fragment_density_map =
        LvlInitStruct<VkRenderingFragmentDensityMapAttachmentInfoEXT>();
    fragment_density_map.imageView = depth_image_view;
    fragment_density_map.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    begin_rendering_info.pNext = &fragment_density_map;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-multiview-06127");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06107");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06108");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06116");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06145");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06146");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DynamicRenderingBufferBeginInfoLegacy) {
    TEST_DESCRIPTION("VkCommandBufferBeginInfo Dynamic Rendering Tests.");

    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_2);
    if (version < VK_API_VERSION_1_2) {
        printf("%s At least Vulkan version 1.2 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequestedExtensionsEnabled()) {
        GTEST_SKIP() << RequestedExtensionsNotSupported() << " not supported";
    }

    auto cmd_buffer_inheritance_rendering_info = LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    cmd_buffer_inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    auto cmd_buffer_inheritance_info = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    cmd_buffer_inheritance_info.pNext = &cmd_buffer_inheritance_rendering_info;
    cmd_buffer_inheritance_info.renderPass = VK_NULL_HANDLE;

    auto cmd_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    cmd_buffer_allocate_info.commandPool = m_commandPool->handle();
    cmd_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    cmd_buffer_allocate_info.commandBufferCount = 0x1;

    VkCommandBuffer secondary_cmd_buffer;
    VkResult err = vk::AllocateCommandBuffers(m_device->device(), &cmd_buffer_allocate_info, &secondary_cmd_buffer);
    ASSERT_VK_SUCCESS(err);

    // Invalid RenderPass
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferBeginInfo-flags-00053");
    VkCommandBufferBeginInfo cmd_buffer_begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cmd_buffer_begin_info.pInheritanceInfo = &cmd_buffer_inheritance_info;
    vk::BeginCommandBuffer(secondary_cmd_buffer, &cmd_buffer_begin_info);
    m_errorMonitor->VerifyFound();

    // Valid RenderPass
    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_B8G8R8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference att_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &att_ref, nullptr, nullptr, 0, nullptr};
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 1, &subpass, 0, nullptr};

    vk_testing::RenderPass rp1;
    rp1.init(*m_device, rpci);

    cmd_buffer_inheritance_info.renderPass = rp1.handle();
    cmd_buffer_inheritance_info.subpass = 0x5;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferBeginInfo-flags-00054");
    vk::BeginCommandBuffer(secondary_cmd_buffer, &cmd_buffer_begin_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DynamicRenderingSecondaryCommandBuffer) {
    TEST_DESCRIPTION("VkCommandBufferBeginInfo Dynamic Rendering Tests.");

    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_3);
    if (version < VK_API_VERSION_1_3) {
        printf("%s At least Vulkan version 1.3 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(Init());

    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    VkCommandBufferObj cb(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    // Force the failure by not setting the Renderpass and Framebuffer fields
    VkCommandBufferInheritanceInfo cmd_buf_hinfo = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    cmd_buf_hinfo.renderPass = VkRenderPass(0x1);
    VkCommandBufferBeginInfo cmd_buf_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    cmd_buf_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cmd_buf_info.pInheritanceInfo = &cmd_buf_hinfo;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferBeginInfo-flags-06000");
    vk::BeginCommandBuffer(cb.handle(), &cmd_buf_info);
    m_errorMonitor->VerifyFound();

    // Valid RenderPass
    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_B8G8R8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference att_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &att_ref, nullptr, nullptr, 0, nullptr};
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 1, &subpass, 0, nullptr};

    vk_testing::RenderPass rp1;
    rp1.init(*m_device, rpci);

    cmd_buf_hinfo.renderPass = rp1.handle();
    cmd_buf_hinfo.subpass = 0x5;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferBeginInfo-flags-06001");
    vk::BeginCommandBuffer(cb.handle(), &cmd_buf_info);
    m_errorMonitor->VerifyFound();

    cmd_buf_hinfo.renderPass = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferBeginInfo-flags-06002");
    vk::BeginCommandBuffer(cb.handle(), &cmd_buf_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestDynamicRenderingPipelineMissingFlags) {
    TEST_DESCRIPTION("Test dynamic rendering with pipeline missing flags.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)) {
        printf("%s %s not supported, skipping test\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }
    bool fragment_density = DeviceExtensionSupported(gpu(), nullptr, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    bool shading_rate = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();

    auto phys_dev_props_2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&fsr_properties);
    vk::GetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(gpu());
    ASSERT_TRUE(depthStencilFormat != 0);

    VkImageObj image(m_device);
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = depthStencilFormat;
    image_create_info.extent = {64, 64, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    image.Init(image_create_info);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo ivci = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                  nullptr,
                                  0,
                                  image.handle(),
                                  VK_IMAGE_VIEW_TYPE_2D,
                                  depthStencilFormat,
                                  {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                   VK_COMPONENT_SWIZZLE_IDENTITY},
                                  {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1}};

    vk_testing::ImageView depth_image_view;
    depth_image_view.init(*m_device, ivci);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    if (shading_rate) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-imageView-06183");
        VkRenderingFragmentShadingRateAttachmentInfoKHR fragment_shading_rate =
            LvlInitStruct<VkRenderingFragmentShadingRateAttachmentInfoKHR>();
        fragment_shading_rate.imageView = depth_image_view.handle();
        fragment_shading_rate.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        fragment_shading_rate.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

        VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&fragment_shading_rate);
        begin_rendering_info.layerCount = 1;
        begin_rendering_info.colorAttachmentCount = 1;
        begin_rendering_info.pColorAttachments = &color_attachment;

        VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

        auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
        pipeline_rendering_info.colorAttachmentCount = 1;
        pipeline_rendering_info.pColorAttachmentFormats = &color_format;

        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.gp_ci_.pNext = &pipeline_rendering_info;
        pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        m_commandBuffer->begin();
        m_commandBuffer->BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
        m_commandBuffer->EndRendering();
        m_commandBuffer->end();

        m_errorMonitor->VerifyFound();
    }

    if (fragment_density) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-imageView-06184");
        VkRenderingFragmentDensityMapAttachmentInfoEXT fragment_density_map =
            LvlInitStruct<VkRenderingFragmentDensityMapAttachmentInfoEXT>();
        fragment_density_map.imageView = depth_image_view.handle();

        VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&fragment_density_map);
        begin_rendering_info.layerCount = 1;
        begin_rendering_info.colorAttachmentCount = 1;
        begin_rendering_info.pColorAttachments = &color_attachment;

        VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

        auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
        pipeline_rendering_info.colorAttachmentCount = 1;
        pipeline_rendering_info.pColorAttachmentFormats = &color_format;

        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.gp_ci_.pNext = &pipeline_rendering_info;
        pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        m_commandBuffer->begin();
        m_commandBuffer->BeginRendering(begin_rendering_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
        m_commandBuffer->EndRendering();
        m_commandBuffer->end();

        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, DynamicRenderingLayerCount) {
    TEST_DESCRIPTION("Test dynamic rendering with viewMask 0 and invalid layer count.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)) {
        printf("%s %s not supported, skipping test\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-viewMask-06069");
    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestRenderingInfoMismatchedSamples) {
    TEST_DESCRIPTION("Test beginning rendering with mismatched sample counts.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)) {
        printf("%s %s not supported, skipping test\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkImageCreateInfo image_ci = LvlInitStruct<VkImageCreateInfo>();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.extent.width = 64;
    image_ci.extent.height = 64;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_2_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageObj color_image(m_device);
    color_image.init(&image_ci);

    VkImageViewCreateInfo civ_ci = LvlInitStruct<VkImageViewCreateInfo>();
    civ_ci.image = color_image.handle();
    civ_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    civ_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    civ_ci.subresourceRange.layerCount = 1;
    civ_ci.subresourceRange.baseMipLevel = 0;
    civ_ci.subresourceRange.levelCount = 1;
    civ_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    vk_testing::ImageView color_image_view;
    color_image_view.init(*m_device, civ_ci);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = color_image_view.handle();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_NONE;

    const VkFormat depth_format = FindSupportedDepthOnlyFormat(gpu());
    if (depth_format == VK_FORMAT_UNDEFINED) {
        printf("%s requires a depth only format, skipping.\n", kSkipPrefix);
        return;
    }

    VkImageObj depth_image(m_device);
    depth_image.Init(64, 64, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);

    VkImageViewCreateInfo div_ci = LvlInitStruct<VkImageViewCreateInfo>();
    div_ci.image = depth_image.handle();
    div_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    div_ci.format = depth_format;
    div_ci.subresourceRange.layerCount = 1;
    div_ci.subresourceRange.baseMipLevel = 0;
    div_ci.subresourceRange.levelCount = 1;
    div_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    vk_testing::ImageView depth_image_view;
    depth_image_view.init(*m_device, div_ci);

    VkRenderingAttachmentInfoKHR depth_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    depth_attachment.imageView = depth_image_view.handle();
    depth_attachment.resolveMode = VK_RESOLVE_MODE_NONE;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.pDepthAttachment = &depth_attachment;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06070");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestBeginRenderingFragmentShadingRate) {
    TEST_DESCRIPTION("Test BeginRenderingInfo with FragmentShadingRateAttachment.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequestedExtensionsEnabled()) {
        GTEST_SKIP() << RequestedExtensionsNotSupported() << " not supported";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto features11 = LvlInitStruct<VkPhysicalDeviceVulkan11Features>(&dynamic_rendering_features);
    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&features11);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (features11.multiview == VK_FALSE) {
        printf("%s Test requires (unsupported) multiview , skipping\n", kSkipPrefix);
        return;
    }
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();

    auto phys_dev_props_2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&fsr_properties);
    vk::GetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto image_ci = LvlInitStruct<VkImageCreateInfo>(nullptr);
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.extent.width = 32;
    image_ci.extent.height = 32;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 2;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;

    VkImageObj image(m_device);
    image.init(&image_ci);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2);

    auto fragment_shading_rate = LvlInitStruct<VkRenderingFragmentShadingRateAttachmentInfoKHR>();
    fragment_shading_rate.imageView = image_view;
    fragment_shading_rate.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    fragment_shading_rate.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&fragment_shading_rate);
    begin_rendering_info.layerCount = 4;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06123");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.viewMask = 0xF;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06124");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    begin_rendering_info.layerCount = 2;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.viewMask = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06125");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, TestDeviceGroupRenderPassBeginInfo) {
    TEST_DESCRIPTION("Test render area of DeviceGroupRenderPassBeginInfo.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)) {
        printf("%s %s not supported, skipping test\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    VkRect2D render_area = {};
    render_area.offset.x = 0;
    render_area.offset.y = 0;
    render_area.extent.width = 32;
    render_area.extent.height = 32;

    auto device_group_render_pass_begin_info = LvlInitStruct<VkDeviceGroupRenderPassBeginInfo>();
    device_group_render_pass_begin_info.deviceRenderAreaCount = 1;
    device_group_render_pass_begin_info.pDeviceRenderAreas = &render_area;

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView colorImageView = colorImage.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = colorImageView;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&device_group_render_pass_begin_info);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_commandBuffer->begin();

    m_errorMonitor->ExpectSuccess();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyNotFound();

    render_area.offset.x = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06083");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    render_area.offset.x = 0;
    render_area.offset.y = 16;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06084");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, TestBeginRenderingInvalidFragmentShadingRateImage) {
    TEST_DESCRIPTION("Test BeginRendering with FragmentShadingRateAttachmentInfo with missing image usage bit.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequestedExtensionsEnabled()) {
        GTEST_SKIP() << RequestedExtensionsNotSupported() << " not supported";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&fsr_properties);
    vk::GetPhysicalDeviceProperties2(gpu(), &properties2);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkImageObj invalid_image(m_device);
    invalid_image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView invalid_image_view = invalid_image.targetView(VK_FORMAT_R8G8B8A8_UINT);

    auto fragment_shading_rate = LvlInitStruct<VkRenderingFragmentShadingRateAttachmentInfoKHR>();
    fragment_shading_rate.imageView = invalid_image_view;
    fragment_shading_rate.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    fragment_shading_rate.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&fragment_shading_rate);
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06148");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    fragment_shading_rate.imageView = image_view;

    fragment_shading_rate.shadingRateAttachmentTexelSize.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06149");
    if (fragment_shading_rate.shadingRateAttachmentTexelSize.width >
        fsr_properties.minFragmentShadingRateAttachmentTexelSize.width) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06150");
    }
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    if (fsr_properties.minFragmentShadingRateAttachmentTexelSize.width > 1) {
        fragment_shading_rate.shadingRateAttachmentTexelSize.width =
            fsr_properties.minFragmentShadingRateAttachmentTexelSize.width / 2;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06151");
        if (fragment_shading_rate.shadingRateAttachmentTexelSize.height /
                fragment_shading_rate.shadingRateAttachmentTexelSize.width >=
            fsr_properties.maxFragmentShadingRateAttachmentTexelSizeAspectRatio) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06156");
        }
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
    }

    fragment_shading_rate.shadingRateAttachmentTexelSize.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;

    fragment_shading_rate.shadingRateAttachmentTexelSize.height =
        fsr_properties.minFragmentShadingRateAttachmentTexelSize.height + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06152");
    if (fragment_shading_rate.shadingRateAttachmentTexelSize.height >
        fsr_properties.minFragmentShadingRateAttachmentTexelSize.height) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06153");
    }
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    if (fsr_properties.minFragmentShadingRateAttachmentTexelSize.height > 1) {
        fragment_shading_rate.shadingRateAttachmentTexelSize.height =
            fsr_properties.minFragmentShadingRateAttachmentTexelSize.height / 2;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06154");
        if (fragment_shading_rate.shadingRateAttachmentTexelSize.width /
                fragment_shading_rate.shadingRateAttachmentTexelSize.height >
            fsr_properties.maxFragmentShadingRateAttachmentTexelSizeAspectRatio) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06155");
        }
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, BeginRenderingInvalidDepthAttachmentFormat) {
    TEST_DESCRIPTION("Test begin rendering with a depth attachment that has an invalid format");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequestedExtensionsEnabled()) {
        GTEST_SKIP() << RequestedExtensionsNotSupported() << " not supported";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkFormat stencil_format = FindSupportedStencilOnlyFormat(gpu());
    if (stencil_format == VK_FORMAT_UNDEFINED) {
        printf("%s requires a stencil only format format.\n", kSkipPrefix);
        return;
    }

    VkImageObj image(m_device);
    image.Init(32, 32, 1, stencil_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkImageView image_view = image.targetView(stencil_format, VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfoKHR depth_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depth_attachment.imageView = image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.pDepthAttachment = &depth_attachment;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-06547");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestFragmentDensityMapRenderArea) {
    TEST_DESCRIPTION("Validate VkRenderingFragmentDensityMapAttachmentInfo attachment image view extent.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)) {
        printf("%s %s not supported, skipping test\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME)) {
        printf("%s %s not supported, skipping test\n", kSkipPrefix, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto fdm_props = LvlInitStruct<VkPhysicalDeviceFragmentDensityMapPropertiesEXT>();
    auto props2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&fdm_props);
    vk::GetPhysicalDeviceProperties2(gpu(), &props2);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);

    auto fragment_density_map = LvlInitStruct<VkRenderingFragmentDensityMapAttachmentInfoEXT>();
    fragment_density_map.imageView = image_view;
    fragment_density_map.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&fragment_density_map);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea.extent.width = 64 * fdm_props.maxFragmentDensityTexelSize.width;
    begin_rendering_info.renderArea.extent.height = 32 * fdm_props.maxFragmentDensityTexelSize.height;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06112");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.extent.width = 32 * fdm_props.maxFragmentDensityTexelSize.width;
    begin_rendering_info.renderArea.extent.height = 64 * fdm_props.maxFragmentDensityTexelSize.height;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06114");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    VkRect2D device_render_area = {};
    device_render_area.extent.width = 64 * fdm_props.maxFragmentDensityTexelSize.width;
    device_render_area.extent.height = 32 * fdm_props.maxFragmentDensityTexelSize.height;
    auto device_group_render_pass_begin_info = LvlInitStruct<VkDeviceGroupRenderPassBeginInfo>();
    device_group_render_pass_begin_info.deviceRenderAreaCount = 1;
    device_group_render_pass_begin_info.pDeviceRenderAreas = &device_render_area;
    fragment_density_map.pNext = &device_group_render_pass_begin_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06113");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    device_render_area.extent.width = 32 * fdm_props.maxFragmentDensityTexelSize.width;
    device_render_area.extent.height = 64 * fdm_props.maxFragmentDensityTexelSize.height;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06115");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, TestFragmentDensityMapRenderAreaWithoutDeviceGroupExt) {
    TEST_DESCRIPTION("Validate VkRenderingFragmentDensityMapAttachmentInfo attachment image view extent.");

    SetTargetApiVersion(VK_API_VERSION_1_0);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() != VK_API_VERSION_1_0) {
        printf("%s Tests requires Vulkan 1.0, skipping test\n", kSkipPrefix);
        return;
    }

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)) {
        printf("%s %s not supported, skipping test\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME)) {
        printf("%s %s not supported, skipping test\n", kSkipPrefix, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
        return;
    }

    auto vkGetPhysicalDeviceFeatures2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR"));
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&dynamic_rendering_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto fdm_props = LvlInitStruct<VkPhysicalDeviceFragmentDensityMapPropertiesEXT>();
    auto props2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&fdm_props);
    vk::GetPhysicalDeviceProperties2(gpu(), &props2);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);

    auto fragment_density_map = LvlInitStruct<VkRenderingFragmentDensityMapAttachmentInfoEXT>();
    fragment_density_map.imageView = image_view;
    fragment_density_map.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&fragment_density_map);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea.extent.width = 64 * fdm_props.maxFragmentDensityTexelSize.width;
    begin_rendering_info.renderArea.extent.height = 32 * fdm_props.maxFragmentDensityTexelSize.height;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06110");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.extent.width = 32 * fdm_props.maxFragmentDensityTexelSize.width;
    begin_rendering_info.renderArea.extent.height = 64 * fdm_props.maxFragmentDensityTexelSize.height;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06111");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, TestBarrierWithDynamicRendering) {
    TEST_DESCRIPTION("Test setting buffer memory barrier when dynamic rendering is active.");

    SetTargetApiVersion(VK_API_VERSION_1_3);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }

    auto vk13features = LvlInitStruct<VkPhysicalDeviceVulkan13Features>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&vk13features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!vk13features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering, skipping\n", kSkipPrefix);
        return;
    }
    if (!vk13features.synchronization2) {
        printf("%s Test requires (unsupported) synchronization2, skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    PFN_vkCmdBeginRendering vkCmdBeginRendering =
        reinterpret_cast<PFN_vkCmdBeginRendering>(vk::GetDeviceProcAddr(m_device->device(), "vkCmdBeginRendering"));
    assert(vkCmdBeginRendering != nullptr);
    PFN_vkCmdEndRendering vkCmdEndRendering =
        reinterpret_cast<PFN_vkCmdEndRendering>(vk::GetDeviceProcAddr(m_device->device(), "vkCmdEndRendering"));
    assert(vkCmdEndRendering != nullptr);

    m_commandBuffer->begin();

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    VkClearRect clear_rect = {{{0, 0}, {(uint32_t)m_width, (uint32_t)m_height}}, 0, 1};
    begin_rendering_info.renderArea = clear_rect.rect;
    begin_rendering_info.layerCount = 1;

    vkCmdBeginRendering(m_commandBuffer->handle(), &begin_rendering_info);

    VkBufferObj buffer;
    VkMemoryPropertyFlags mem_reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    buffer.init_as_src_and_dst(*m_device, 256, mem_reqs);

    auto buf_barrier = lvl_init_struct<VkBufferMemoryBarrier2KHR>();
    buf_barrier.buffer = buffer.handle();
    buf_barrier.offset = 0;
    buf_barrier.size = VK_WHOLE_SIZE;
    buf_barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    buf_barrier.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    auto dependency_info = lvl_init_struct<VkDependencyInfoKHR>();
    dependency_info.bufferMemoryBarrierCount = 1;
    dependency_info.pBufferMemoryBarriers = &buf_barrier;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier2-None-06191");
    vk::CmdPipelineBarrier2(m_commandBuffer->handle(), &dependency_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-None-06191");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    vkCmdEndRendering(m_commandBuffer->handle());
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, BeginRenderingInvalidStencilAttachmentFormat) {
    TEST_DESCRIPTION("Test begin rendering with a stencil attachment that has an invalid format");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequestedExtensionsEnabled()) {
        GTEST_SKIP() << RequestedExtensionsNotSupported() << " not supported";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkFormat depth_format = FindSupportedDepthOnlyFormat(gpu());
    if (depth_format == VK_FORMAT_UNDEFINED) {
        printf("%s requires a stencil only format format.\n", kSkipPrefix);
        return;
    }

    VkImageObj image(m_device);
    image.Init(32, 32, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkImageView image_view = image.targetView(depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);

    VkRenderingAttachmentInfoKHR stencil_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    stencil_attachment.imageView = image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.pStencilAttachment = &stencil_attachment;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pStencilAttachment-06548");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestInheritanceRenderingInfoStencilAttachmentFormat) {
    TEST_DESCRIPTION("Test begin rendering with a stencil attachment that has an invalid format");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequestedExtensionsEnabled()) {
        GTEST_SKIP() << RequestedExtensionsNotSupported() << " not supported";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkFormat depth_format = FindSupportedDepthOnlyFormat(gpu());
    if (depth_format == VK_FORMAT_UNDEFINED) {
        printf("%s requires a stencil only format format.\n", kSkipPrefix);
        return;
    }

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

    auto cmd_buffer_inheritance_rendering_info = LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    cmd_buffer_inheritance_rendering_info.colorAttachmentCount = 1;
    cmd_buffer_inheritance_rendering_info.pColorAttachmentFormats = &color_format;
    cmd_buffer_inheritance_rendering_info.depthAttachmentFormat = depth_format;
    cmd_buffer_inheritance_rendering_info.stencilAttachmentFormat = depth_format;
    cmd_buffer_inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    auto cmd_buffer_inheritance_info = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    cmd_buffer_inheritance_info.pNext = &cmd_buffer_inheritance_rendering_info;
    cmd_buffer_inheritance_info.renderPass = VK_NULL_HANDLE;

    VkCommandBufferBeginInfo cmd_buffer_begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cmd_buffer_begin_info.pInheritanceInfo = &cmd_buffer_inheritance_info;

    auto cmd_buffer_allocate_info = LvlInitStruct<VkCommandBufferAllocateInfo>();
    cmd_buffer_allocate_info.commandPool = m_commandPool->handle();
    cmd_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    cmd_buffer_allocate_info.commandBufferCount = 1;

    VkCommandBuffer secondary_cmd_buffer;
    vk::AllocateCommandBuffers(m_device->device(), &cmd_buffer_allocate_info, &secondary_cmd_buffer);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCommandBufferInheritanceRenderingInfo-stencilAttachmentFormat-06541");
    vk::BeginCommandBuffer(secondary_cmd_buffer, &cmd_buffer_begin_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreateGraphicsPipelineWithInvalidAttachmentSampleCount) {
    TEST_DESCRIPTION("Create pipeline with fragment shader that uses samples, but multisample state not begin set");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Extension %s or %s not supported, skipping tests\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
               VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering, skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

    auto sample_count_info_amd = LvlInitStruct<VkAttachmentSampleCountInfoNV>();
    sample_count_info_amd.colorAttachmentCount = 1;
    sample_count_info_amd.depthStencilAttachmentSamples = static_cast<VkSampleCountFlagBits>(0x3);

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>(&sample_count_info_amd);
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-depthStencilAttachmentSamples-06593");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreatePipelineUsingDynamicRenderingWithoutFeature) {
    TEST_DESCRIPTION("Create graphcis pipeline that uses dynamic rendering, but feature is not enabled");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Extension %s or %s not supported, skipping tests\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
               VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-dynamicRendering-06576");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DynamicRenderingAreaGreaterThanAttachmentExtent) {
    TEST_DESCRIPTION("Begin dynamic rendering with render area greater than extent of attachments");

    SetTargetApiVersion(VK_API_VERSION_1_0);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() != VK_API_VERSION_1_0) {
        printf("%s Tests requires Vulkan 1.0, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Extension %s not supported, skipping tests\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto vkGetPhysicalDeviceFeatures2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR"));
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&dynamic_rendering_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView colorImageView = colorImage.targetView(VK_FORMAT_R8G8B8A8_UINT);

    auto color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = colorImageView;

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.renderArea.extent.width = 64;
    begin_rendering_info.renderArea.extent.height = 32;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06075");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.extent.width = 32;
    begin_rendering_info.renderArea.extent.height = 64;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06076");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    const VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());
    if (ds_format != VK_FORMAT_UNDEFINED) {
        VkImageObj depthImage(m_device);
        depthImage.Init(32, 32, 1, ds_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        VkImageView depthImageView = depthImage.targetView(ds_format, VK_IMAGE_ASPECT_DEPTH_BIT);

        VkRenderingAttachmentInfoKHR depth_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
        depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depth_attachment.imageView = depthImageView;

        begin_rendering_info.colorAttachmentCount = 0;
        begin_rendering_info.pDepthAttachment = &depth_attachment;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06076");
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, DynamicRenderingDeviceGroupAreaGreaterThanAttachmentExtent) {
    TEST_DESCRIPTION("Begin dynamic rendering with device group with render area greater than extent of attachments");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Extension %s not supported, skipping tests\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkImageObj colorImage(m_device);
    colorImage.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView colorImageView = colorImage.targetView(VK_FORMAT_R8G8B8A8_UINT);

    auto color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = colorImageView;

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.renderArea.extent.width = 64;
    begin_rendering_info.renderArea.extent.height = 32;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06079");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.extent.width = 32;
    begin_rendering_info.renderArea.extent.height = 64;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06080");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    const VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());
    if (ds_format != VK_FORMAT_UNDEFINED) {
        VkImageObj depthImage(m_device);
        depthImage.Init(32, 32, 1, ds_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        VkImageView depthImageView = depthImage.targetView(ds_format, VK_IMAGE_ASPECT_DEPTH_BIT);

        VkRenderingAttachmentInfoKHR depth_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
        depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depth_attachment.imageView = depthImageView;

        begin_rendering_info.colorAttachmentCount = 0;
        begin_rendering_info.pDepthAttachment = &depth_attachment;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06080");
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, SecondaryCommandBufferIncompatibleRenderPass) {
    TEST_DESCRIPTION("Execute secondary command buffers within render pass instance with incompatible render pass");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkSubpassDescription subpass = {};
    auto render_pass_ci = LvlInitStruct<VkRenderPassCreateInfo>();
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;

    vk_testing::RenderPass render_pass;
    render_pass.init(*m_device, render_pass_ci);

    VkCommandBufferObj cb(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBuffer secondary_handle = cb.handle();

    auto cmd_buffer_inheritance_info = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    cmd_buffer_inheritance_info.renderPass = render_pass.handle();
    VkCommandBufferBeginInfo cmd_buffer_begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cmd_buffer_begin_info.pInheritanceInfo = &cmd_buffer_inheritance_info;

    cb.begin(&cmd_buffer_begin_info);
    cb.end();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pBeginInfo-06020");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pInheritanceInfo-00098");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_handle);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, SecondaryCommandBufferIncompatibleSubpass) {
    TEST_DESCRIPTION("Execute secondary command buffers with different subpass");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkSubpassDescription subpasses[2] = {};

    auto render_pass_ci = LvlInitStruct<VkRenderPassCreateInfo>();
    render_pass_ci.subpassCount = 2;
    render_pass_ci.pSubpasses = subpasses;

    vk_testing::RenderPass render_pass;
    render_pass.init(*m_device, render_pass_ci);

    auto framebuffer_ci = LvlInitStruct<VkFramebufferCreateInfo>();
    framebuffer_ci.renderPass = render_pass.handle();
    framebuffer_ci.width = 32;
    framebuffer_ci.height = 32;
    framebuffer_ci.layers = 1;

    vk_testing::Framebuffer framebuffer;
    framebuffer.init(*m_device, framebuffer_ci);

    VkCommandBufferObj cb(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBuffer secondary_handle = cb.handle();

    auto cmd_buffer_inheritance_info = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    cmd_buffer_inheritance_info.renderPass = render_pass.handle();
    VkCommandBufferBeginInfo cmd_buffer_begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cmd_buffer_begin_info.pInheritanceInfo = &cmd_buffer_inheritance_info;

    cb.begin(&cmd_buffer_begin_info);
    cb.end();

    auto render_pass_begin_info = LvlInitStruct<VkRenderPassBeginInfo>();
    render_pass_begin_info.renderPass = render_pass.handle();
    render_pass_begin_info.renderArea.extent = {32, 32};
    render_pass_begin_info.framebuffer = framebuffer.handle();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(render_pass_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vk::CmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pCommandBuffers-00097");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pCommandBuffers-06019");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_handle);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, SecondaryCommandBufferInvalidContents) {
    TEST_DESCRIPTION("Execute secondary command buffers within active render pass that was not begun with VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkCommandBufferObj cb(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    VkCommandBuffer secondary_handle = cb.handle();

    auto cmd_buffer_inheritance_info = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    cmd_buffer_inheritance_info.renderPass = m_renderPass;
    VkCommandBufferBeginInfo cmd_buffer_begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cmd_buffer_begin_info.pInheritanceInfo = &cmd_buffer_inheritance_info;

    cb.begin(&cmd_buffer_begin_info);
    cb.end();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-contents-00095");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-contents-06018");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary_handle);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, BindPipelineWithIncompatibleRenderingInfoDepthFormat) {
    TEST_DESCRIPTION("Bind pipeline that has RenderingInfo DepthFormat incompatible with previously used pipeline");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_errorMonitor->ExpectSuccess();

    std::vector<VkFormat> depth_formats;
    const VkFormat ds_formats[] = {VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT};
    for (uint32_t i = 0; i < size(ds_formats); ++i) {
        VkFormatProperties format_props;
        vk::GetPhysicalDeviceFormatProperties(gpu(), ds_formats[i], &format_props);

        if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            depth_formats.push_back(ds_formats[i]);
        }
    }

    if (depth_formats.size() < 2) {
        printf("%s Test requires 2 depth formats, skipping test.\n", kSkipPrefix);
        return;
    }

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.depthAttachmentFormat = depth_formats[0];

    auto ds_ci = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();

    CreatePipelineHelper pipe1(*this);
    pipe1.InitInfo();
    pipe1.gp_ci_.pNext = &pipeline_rendering_info;
    pipe1.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe1.InitState();
    pipe1.ds_ci_ = ds_ci;
    pipe1.CreateGraphicsPipeline();

    pipeline_rendering_info.depthAttachmentFormat = depth_formats[1];

    CreatePipelineHelper pipe2(*this);
    pipe2.InitInfo();
    pipe2.gp_ci_.pNext = &pipeline_rendering_info;
    pipe2.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe2.InitState();
    pipe2.ds_ci_ = ds_ci;
    pipe2.CreateGraphicsPipeline();

    VkImageObj image(m_device);
    image.Init(32, 32, 1, depth_formats[0], VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_LINEAR, 0);
    VkImageView depth_image_view = image.targetView(depth_formats[0], VK_IMAGE_ASPECT_DEPTH_BIT);

    VkRenderingAttachmentInfoKHR depth_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depth_attachment.imageView = depth_image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.pDepthAttachment = &depth_attachment;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-pipeline-06197");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.pipeline_);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, BindPipelineWithIncompatibleRenderingInfoColorAttachments) {
    TEST_DESCRIPTION("Bind pipeline that has RenderingInfo color attachment incompatible with previously used pipeline");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_errorMonitor->ExpectSuccess();

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &format;

    CreatePipelineHelper pipe1(*this);
    pipe1.InitInfo();
    pipe1.gp_ci_.pNext = &pipeline_rendering_info;
    pipe1.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe1.InitState();
    pipe1.CreateGraphicsPipeline();

    format = VK_FORMAT_B8G8R8A8_UNORM;

    CreatePipelineHelper pipe2(*this);
    pipe2.InitInfo();
    pipe2.gp_ci_.pNext = &pipeline_rendering_info;
    pipe2.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe2.InitState();
    pipe2.CreateGraphicsPipeline();

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_LINEAR, 0);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-pipeline-06196");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.pipeline_);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, BindPipelineWithIncompatibleRenderingInfoColorAttachmentCount) {
    TEST_DESCRIPTION("Bind pipeline that has RenderingInfo color attachment count incompatible with previously used pipeline");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_errorMonitor->ExpectSuccess();

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &format;

    CreatePipelineHelper pipe1(*this);
    pipe1.InitInfo();
    pipe1.gp_ci_.pNext = &pipeline_rendering_info;
    pipe1.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe1.InitState();
    pipe1.CreateGraphicsPipeline();

    pipeline_rendering_info.colorAttachmentCount = 0;

    CreatePipelineHelper pipe2(*this);
    pipe2.InitInfo();
    pipe2.gp_ci_.pNext = &pipeline_rendering_info;
    pipe2.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe2.InitState();
    pipe2.CreateGraphicsPipeline();

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_LINEAR, 0);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-pipeline-06195");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.pipeline_);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, BindPipelineWithIncompatibleRenderingInfoStencilFormat) {
    TEST_DESCRIPTION("Bind pipeline that has RenderingInfo StencilFormat incompatible with previously used pipeline");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_errorMonitor->ExpectSuccess();

    std::vector<VkFormat> stencil_formats;
    const VkFormat ds_formats[] = {VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT};
    for (uint32_t i = 0; i < size(ds_formats); ++i) {
        VkFormatProperties format_props;
        vk::GetPhysicalDeviceFormatProperties(gpu(), ds_formats[i], &format_props);

        if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            stencil_formats.push_back(ds_formats[i]);
        }
    }

    if (stencil_formats.size() < 2) {
        printf("%s Test requires 2 stencil formats, skipping test.\n", kSkipPrefix);
        return;
    }

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.stencilAttachmentFormat = stencil_formats[0];

    auto ds_ci = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();

    CreatePipelineHelper pipe1(*this);
    pipe1.InitInfo();
    pipe1.gp_ci_.pNext = &pipeline_rendering_info;
    pipe1.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe1.InitState();
    pipe1.ds_ci_ = ds_ci;
    pipe1.CreateGraphicsPipeline();

    pipeline_rendering_info.stencilAttachmentFormat = stencil_formats[1];

    CreatePipelineHelper pipe2(*this);
    pipe2.InitInfo();
    pipe2.gp_ci_.pNext = &pipeline_rendering_info;
    pipe2.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe2.InitState();
    pipe2.ds_ci_ = ds_ci;
    pipe2.CreateGraphicsPipeline();

    VkImageObj image(m_device);
    image.Init(32, 32, 1, stencil_formats[0], VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_LINEAR, 0);
    VkImageView stencil_image_view = image.targetView(stencil_formats[0], VK_IMAGE_ASPECT_DEPTH_BIT);

    VkRenderingAttachmentInfoKHR stencil_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    stencil_attachment.imageView = stencil_image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.pStencilAttachment = &stencil_attachment;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-pipeline-06194");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.pipeline_);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, DynamicRenderingWithInvalidShaderLayerBuiltIn) {
    TEST_DESCRIPTION("Create invalid pipeline that writes to Layer built-in");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>(&dynamic_rendering_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&multiview_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }
    if (multiview_features.multiview == VK_FALSE) {
        printf("%s Test requires (unsupported) multiview , skipping\n", kSkipPrefix);
        return;
    }
    if (multiview_features.multiviewGeometryShader == VK_FALSE) {
        printf("%s Test requires (unsupported) multiviewGeometryShader , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    static char const *gsSource = R"glsl(
        #version 450
        layout (triangles) in;
        layout (triangle_strip) out;
        layout (max_vertices = 1) out;
        void main() {
            gl_Position = vec4(1.0, 0.5, 0.5, 0.0);
            EmitVertex();
            gl_Layer = 4;
        }
    )glsl";

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;
    pipeline_rendering_info.viewMask = 0x1;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06059");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DynamicRenderingWithInputAttachmentCapability) {
    TEST_DESCRIPTION("Create invalid pipeline that uses InputAttachment capability");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const std::string fsSource = R"(
               OpCapability Shader
               OpCapability InputAttachment
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06061");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidRenderingInfoColorAttachmentFormat) {
    TEST_DESCRIPTION("Create pipeline with invalid color attachment format");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkFormat color_format = VK_FORMAT_MAX_ENUM;

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06579");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06580");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidDynamicRenderingLibraryViewMask) {
    TEST_DESCRIPTION("Create pipeline with invalid view mask");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s or %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
               VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
        return;
    }

    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto library_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>(&multiview_features);
    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>(&library_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }
    if (library_features.graphicsPipelineLibrary == VK_FALSE) {
        printf("%s Test requires (unsupported) graphicsPipelineLibrary , skipping\n", kSkipPrefix);
        return;
    }
    if (multiview_features.multiview == VK_FALSE) {
        printf("%s Test requires (unsupported) multiview , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    auto graphics_library_create_info = LvlInitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(&pipeline_rendering_info);
    graphics_library_create_info.flags = VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT;
    auto library_create_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>(&graphics_library_create_info);

    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
    auto color_blend_state_create_info = LvlInitStruct<VkPipelineColorBlendStateCreateInfo>();
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments = &color_blend_attachment_state;

    m_errorMonitor->ExpectSuccess();
    CreatePipelineHelper lib(*this);
    lib.cb_ci_ = color_blend_state_create_info;
    lib.InitInfo();
    lib.gp_ci_.pNext = &library_create_info;
    lib.gp_ci_.renderPass = VK_NULL_HANDLE;
    lib.InitState();
    lib.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();

    graphics_library_create_info.flags = VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT;
    library_create_info.libraryCount = 1;
    library_create_info.pLibraries = &lib.pipeline_;
    pipeline_rendering_info.viewMask = 0x1;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pNext = &library_create_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-flags-06626");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidAttachmentSampleCount) {
    TEST_DESCRIPTION("Create pipeline with invalid color attachment samples");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkSampleCountFlagBits color_attachment_samples = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;

    auto samples_info = LvlInitStruct<VkAttachmentSampleCountInfoAMD>();
    samples_info.colorAttachmentCount = 1;
    samples_info.pColorAttachmentSamples = &color_attachment_samples;
    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>(&samples_info);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pColorAttachmentSamples-06592");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidDynamicRenderingLibrariesViewMask) {
    TEST_DESCRIPTION("Create pipeline with libaries that have incompatible view mask");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s or %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
               VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
        return;
    }

    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto library_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>(&multiview_features);
    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>(&library_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }
    if (library_features.graphicsPipelineLibrary == VK_FALSE) {
        printf("%s Test requires (unsupported) graphicsPipelineLibrary , skipping\n", kSkipPrefix);
        return;
    }
    if (multiview_features.multiview == VK_FALSE) {
        printf("%s Test requires (unsupported) multiview , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    auto graphics_library_create_info = LvlInitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(&pipeline_rendering_info);
    graphics_library_create_info.flags = VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT;
    auto library_create_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>(&graphics_library_create_info);

    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
    auto color_blend_state_create_info = LvlInitStruct<VkPipelineColorBlendStateCreateInfo>();
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments = &color_blend_attachment_state;

    m_errorMonitor->ExpectSuccess();
    CreatePipelineHelper lib1(*this);
    lib1.cb_ci_ = color_blend_state_create_info;
    lib1.InitInfo();
    lib1.gp_ci_.pNext = &library_create_info;
    lib1.gp_ci_.renderPass = VK_NULL_HANDLE;
    lib1.InitState();
    lib1.CreateGraphicsPipeline();

    graphics_library_create_info.flags = VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT;
    pipeline_rendering_info.viewMask = 0x1;

    auto ds_ci = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();

    CreatePipelineHelper lib2(*this);
    lib2.cb_ci_ = color_blend_state_create_info;
    lib2.InitInfo();
    lib2.gp_ci_.pNext = &library_create_info;
    lib2.gp_ci_.renderPass = VK_NULL_HANDLE;
    lib2.ds_ci_ = ds_ci;
    lib2.InitState();
    lib2.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();

    graphics_library_create_info.flags =
        VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT | VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT;
    library_create_info.libraryCount = 2;
    VkPipeline libraries[2] = {lib1.pipeline_, lib2.pipeline_};
    library_create_info.pLibraries = libraries;
    pipeline_rendering_info.viewMask = 0;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pNext = &library_create_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06627");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidDynamicRenderingLibraryRenderPass) {
    TEST_DESCRIPTION("Create pipeline with invalid library render pass");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s or %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
               VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
        return;
    }

    auto library_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>(&library_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }
    if (library_features.graphicsPipelineLibrary == VK_FALSE) {
        printf("%s Test requires (unsupported) graphicsPipelineLibrary , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;

    auto graphics_library_create_info = LvlInitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(&pipeline_rendering_info);
    graphics_library_create_info.flags = VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT;
    auto library_create_info = LvlInitStruct<VkPipelineLibraryCreateInfoKHR>(&graphics_library_create_info);

    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
    auto color_blend_state_create_info = LvlInitStruct<VkPipelineColorBlendStateCreateInfo>();
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments = &color_blend_attachment_state;

    m_errorMonitor->ExpectSuccess();
    CreatePipelineHelper lib(*this);
    lib.cb_ci_ = color_blend_state_create_info;
    lib.InitInfo();
    lib.gp_ci_.pNext = &library_create_info;
    lib.InitState();
    lib.CreateGraphicsPipeline();
    m_errorMonitor->VerifyNotFound();

    graphics_library_create_info.flags = VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT;
    library_create_info.libraryCount = 1;
    library_create_info.pLibraries = &lib.pipeline_;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pNext = &library_create_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderpass-06625");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, PipelineMissingMultisampleState) {
    TEST_DESCRIPTION("Create pipeline with missing multisample state");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s or %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
               VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pMultisampleState = nullptr;
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderpass-06631");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidRenderingFragmentDensityMapAttachment) {
    TEST_DESCRIPTION("Use invalid VkRenderingFragmentDensityMapAttachmentInfoEXT");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s or %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
               VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
        return;
    }

    auto multiview_featuers = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>(&multiview_featuers);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }
    if (multiview_featuers.multiview == VK_FALSE) {
        printf("%s Test requires (unsupported) multiview , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT, VK_IMAGE_TILING_LINEAR, 0);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    auto rendering_fragment_density = LvlInitStruct<VkRenderingFragmentDensityMapAttachmentInfoEXT>();
    rendering_fragment_density.imageView = image_view;
    rendering_fragment_density.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&rendering_fragment_density);
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-imageView-06157");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    rendering_fragment_density.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 2;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV | VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
    VkImageObj image2(m_device);
    image2.Init(image_create_info);
    VkImageView image_view2 = image2.targetView(VK_FORMAT_R8G8B8A8_UINT);
    rendering_fragment_density.imageView = image_view2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-imageView-06160");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06109");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidRenderingFragmentDensityMapAttachmentUsage) {
    TEST_DESCRIPTION("Use VkRenderingFragmentDensityMapAttachmentInfoEXT with invalid imageLayout");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s or %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
               VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_LINEAR, 0);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    auto rendering_fragment_density = LvlInitStruct<VkRenderingFragmentDensityMapAttachmentInfoEXT>();
    rendering_fragment_density.imageView = image_view;
    rendering_fragment_density.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&rendering_fragment_density);
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-imageView-06158");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidRenderingFragmentDensityMapAttachmentCreateFlags) {
    TEST_DESCRIPTION("Use VkRenderingFragmentDensityMapAttachmentInfoEXT with invalid image create flags");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s or %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
               VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.flags = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 4};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    auto rendering_fragment_density = LvlInitStruct<VkRenderingFragmentDensityMapAttachmentInfoEXT>();
    rendering_fragment_density.imageView = image_view;
    rendering_fragment_density.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&rendering_fragment_density);
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-imageView-06159");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidRenderingFragmentDensityMapAttachmentLayerCount) {
    TEST_DESCRIPTION("Use VkRenderingFragmentDensityMapAttachmentInfoEXT with invalid layer count");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s or %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
               VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
        return;
    }

    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>(&multiview_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }
    if (!multiview_features.multiview) {
        printf("%s Test requires (unsupported) multiview , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 4};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 2;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    auto rendering_fragment_density = LvlInitStruct<VkRenderingFragmentDensityMapAttachmentInfoEXT>();
    rendering_fragment_density.imageView = image_view;
    rendering_fragment_density.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&rendering_fragment_density);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.viewMask = 0x1;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-imageView-06160");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidRenderingPNextImageView) {
    TEST_DESCRIPTION(
        "Use different image views in VkRenderingFragmentShadingRateAttachmentInfoKHR and "
        "VkRenderingFragmentDensityMapAttachmentInfoEXT");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s or %s or %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
               VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
        return;
    }

    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>(&multiview_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }
    if (!multiview_features.multiview) {
        printf("%s Test requires (unsupported) multiview , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();

    auto phys_dev_props_2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&fsr_properties);
    vk::GetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);

    VkImageObj image1(m_device);
    image1.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
                VK_IMAGE_TILING_LINEAR, 0);
    VkImageView image_view1 = image1.targetView(VK_FORMAT_R8G8B8A8_UNORM);
    VkImageObj image2(m_device);
    image2.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT, VK_IMAGE_TILING_LINEAR, 0);
    VkImageView image_view2 = image2.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    auto rendering_fragment_shading_rate = LvlInitStruct<VkRenderingFragmentShadingRateAttachmentInfoKHR>();
    rendering_fragment_shading_rate.imageView = image_view1;
    rendering_fragment_shading_rate.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    rendering_fragment_shading_rate.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    auto rendering_fragment_density =
        LvlInitStruct<VkRenderingFragmentDensityMapAttachmentInfoEXT>(&rendering_fragment_shading_rate);
    rendering_fragment_density.imageView = image_view2;
    rendering_fragment_density.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&rendering_fragment_density);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.viewMask = 0x1;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06126");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidRenderingRenderArea) {
    TEST_DESCRIPTION("Use negative offset in RenderingInfo render area");

    SetTargetApiVersion(VK_API_VERSION_1_0);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() != VK_API_VERSION_1_0) {
        printf("%s Tests requires Vulkan 1.0, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto vkGetPhysicalDeviceFeatures2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR"));
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&dynamic_rendering_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea.offset.x = -1;
    begin_rendering_info.renderArea.extent.width = 32;
    begin_rendering_info.renderArea.extent.height = 32;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-renderArea-06071");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = 0;
    begin_rendering_info.renderArea.offset.y = -1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-renderArea-06072");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.y = 0;
    begin_rendering_info.renderArea.offset.x = m_device->phy().properties().limits.maxFramebufferWidth - 16;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-renderArea-06073");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = 0;
    begin_rendering_info.renderArea.offset.y = m_device->phy().properties().limits.maxFramebufferHeight - 16;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-renderArea-06074");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidRenderingInfoViewMask) {
    TEST_DESCRIPTION("Use negative offset in RenderingInfo render area");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>(&multiview_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }
    if (multiview_features.multiview == VK_FALSE) {
        printf("%s Test requires (unsupported) multiview , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPhysicalDeviceMultiviewProperties multiview_props = LvlInitStruct<VkPhysicalDeviceMultiviewProperties>();
    VkPhysicalDeviceProperties2 pd_props2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&multiview_props);
    vk::GetPhysicalDeviceProperties2(gpu(), &pd_props2);

    if (multiview_props.maxMultiviewViewCount == 32) {
        printf("%s VUID is not testable as maxMultiviewViewCount is 32, skipping test\n", kSkipPrefix);
        return;
    }

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea.extent.width = 32;
    begin_rendering_info.renderArea.extent.height = 32;
    begin_rendering_info.viewMask = 1u << multiview_props.maxMultiviewViewCount;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-viewMask-06128");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidRenderingColorAttachmentFormat) {
    TEST_DESCRIPTION("Use format with missing potential format features in rendering color attachment");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_LINEAR_COLOR_ATTACHMENT_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s or %s is not supported, skipping test.\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
               VK_NV_LINEAR_COLOR_ATTACHMENT_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkFormat format = FindSupportedDepthStencilFormat(gpu());
    if (format == VK_FORMAT_UNDEFINED) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }

    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &format;

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.pNext = &pipeline_rendering_info;
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06582");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidResolveModeWithNonIntegerColorFormat) {
    TEST_DESCRIPTION("Use invalid resolve mode with non integer color format");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequestedExtensionsEnabled()) {
        GTEST_SKIP() << RequestedExtensionsNotSupported() << " not supported";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;  // not int color
    image_create_info.extent = {32, 32, 4};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.resolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;  // not allowed for format
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06129");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidResolveModeWithIntegerColorFormat) {
    TEST_DESCRIPTION("Use invalid resolve mode with integer color format");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UINT;
    image_create_info.extent = {32, 32, 4};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_MAX_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06130");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidResolveModeSamples) {
    TEST_DESCRIPTION("Use invalid sample count with resolve mode that is not none");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06132");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidResolveImageViewSamples) {
    TEST_DESCRIPTION("Use resolve image view with invalid sample count");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UINT;
    image_create_info.extent = {32, 32, 4};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkImageObj resolve_image(m_device);
    resolve_image.Init(image_create_info);
    VkImageView resolve_image_view = resolve_image.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06133");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidResolveImageViewFormatMatch) {
    TEST_DESCRIPTION("Use resolve image view with different format from image view");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UINT;
    image_create_info.extent = {32, 32, 4};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UINT);

    VkImageObj resolve_image(m_device);
    resolve_image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView resolve_image_view = resolve_image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06134");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidRenderingAttachmentImageViewLayout) {
    TEST_DESCRIPTION("Use rendering attachment image view with invalid layout");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06135");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidResolveImageViewLayout) {
    TEST_DESCRIPTION("Use resolve image view with invalid layout");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 4};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkImageObj resolve_image(m_device);
    resolve_image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView resolve_image_view = resolve_image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06136");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidResolveImageViewLayoutSeparateDepthStencil) {
    TEST_DESCRIPTION("Use resolve image view with invalid layout");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s or %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
               VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 4};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkImageObj resolve_image(m_device);
    resolve_image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView resolve_image_view = resolve_image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06137");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidRenderingAttachmentImageViewShadingRateLayout) {
    TEST_DESCRIPTION("Use image view with invalid layout");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s or %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
               VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06138");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06143");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidResolveImageViewShadingRateLayout) {
    TEST_DESCRIPTION("Use resolve image view with invalid shading ratelayout");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s or %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
               VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 4};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkImageObj resolve_image(m_device);
    resolve_image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView resolve_image_view = resolve_image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06139");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06144");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidRenderingAttachmentImageViewFragmentDensityLayout) {
    TEST_DESCRIPTION("Use image view with invalid layout");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s or %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
               VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06140");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidResolveImageViewFragmentDensityLayout) {
    TEST_DESCRIPTION("Use resolve image view with invalid fragment density layout");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s or %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
               VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 4};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkImageObj resolve_image(m_device);
    resolve_image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView resolve_image_view = resolve_image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06141");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidResolveImageViewReadOnlyOptimalLayout) {
    TEST_DESCRIPTION("Use resolve image view with invalid read only optimal layout");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 4};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkImageObj resolve_image(m_device);
    resolve_image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView resolve_image_view = resolve_image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06142");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, TestBeginRenderingFragmentShadingRateImageView) {
    TEST_DESCRIPTION("Test BeginRenderingInfo image view with FragmentShadingRateAttachment.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s or %s not supported, skipping test\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
               VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();

    auto phys_dev_props_2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&fsr_properties);
    vk::GetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
               VK_IMAGE_TILING_LINEAR, 0);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    auto fragment_shading_rate = LvlInitStruct<VkRenderingFragmentShadingRateAttachmentInfoKHR>();
    fragment_shading_rate.imageView = image_view;
    fragment_shading_rate.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    fragment_shading_rate.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&fragment_shading_rate);
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06147");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, TestRenderingInfoColorAttachment) {
    TEST_DESCRIPTION("Test RenderingInfo color attachment.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s not supported, skipping test\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj invalid_image(m_device);
    invalid_image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_LINEAR, 0);
    VkImageView invalid_image_view = invalid_image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 4};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkImageObj image(m_device);
    image.Init(image_create_info);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageView = invalid_image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-colorAttachmentCount-06087");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    color_attachment.imageView = image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-colorAttachmentCount-06090");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    color_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-colorAttachmentCount-06096");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    color_attachment.imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-colorAttachmentCount-06100");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-colorAttachmentCount-06091");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-colorAttachmentCount-06097");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-colorAttachmentCount-06101");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    color_attachment.resolveMode = VK_RESOLVE_MODE_NONE;

    const uint32_t max_color_attachments = m_device->phy().properties().limits.maxColorAttachments + 1;
    std::vector<VkRenderingAttachmentInfoKHR> color_attachments(max_color_attachments);
    for (auto &attachment : color_attachments) {
        attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
        attachment.imageView = image_view;
        attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    begin_rendering_info.colorAttachmentCount = max_color_attachments;
    begin_rendering_info.pColorAttachments = color_attachments.data();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-colorAttachmentCount-06106");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, TestRenderingInfoDepthAttachment) {
    TEST_DESCRIPTION("Test RenderingInfo depth attachment.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s or %s not supported, skipping test\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
               VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());
    if (ds_format == VK_FORMAT_UNDEFINED) {
        printf("%s No Depth + Stencil format found, skipping test..\n", kSkipPrefix);
        return;
    }

    auto depth_stencil_resolve_properties = LvlInitStruct<VkPhysicalDeviceDepthStencilResolveProperties>();
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&depth_stencil_resolve_properties);
    vk::GetPhysicalDeviceProperties2(gpu(), &properties2);
    bool has_depth_resolve_mode_average =
        (depth_stencil_resolve_properties.supportedDepthResolveModes & VK_RESOLVE_MODE_AVERAGE_BIT) != 0;
    bool has_stencil_resolve_mode_average =
        (depth_stencil_resolve_properties.supportedStencilResolveModes & VK_RESOLVE_MODE_AVERAGE_BIT) != 0;

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = ds_format;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    VkImageObj image(m_device);
    image.Init(image_create_info);

    VkImageObj depth_image(m_device);
    depth_image.Init(image_create_info);
    VkImageView depth_image_view = depth_image.targetView(ds_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    VkImageObj stencil_image(m_device);
    stencil_image.Init(image_create_info);
    VkImageView stencil_image_view = stencil_image.targetView(ds_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkImageObj depth_resolvel_image(m_device);
    depth_resolvel_image.Init(32, 32, 1, ds_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView depth_resolve_image_view =
        depth_resolvel_image.targetView(ds_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkImageObj stencil_resolvel_image(m_device);
    stencil_resolvel_image.Init(32, 32, 1, ds_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView stencil_resolve_image_view =
        stencil_resolvel_image.targetView(ds_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkImageObj invalid_image(m_device);
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    invalid_image.Init(image_create_info);
    VkImageView invalid_image_view = invalid_image.targetView(ds_format, VK_IMAGE_ASPECT_DEPTH_BIT);

    auto depth_attachment = LvlInitStruct<VkRenderingAttachmentInfo>();
    depth_attachment.imageView = depth_image_view;
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    auto stencil_attachment = LvlInitStruct<VkRenderingAttachmentInfo>();
    stencil_attachment.imageView = stencil_image_view;
    stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.pDepthAttachment = &depth_attachment;
    begin_rendering_info.pStencilAttachment = &stencil_attachment;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-06085");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    depth_attachment.imageView = VK_NULL_HANDLE;
    stencil_attachment.imageView = VK_NULL_HANDLE;
    depth_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    depth_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment.resolveImageView = depth_resolve_image_view;
    stencil_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    stencil_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    stencil_attachment.resolveImageView = stencil_resolve_image_view;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-06086");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    depth_attachment.imageView = depth_image_view;
    stencil_attachment.imageView = depth_image_view;
    depth_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    stencil_attachment.resolveImageView = depth_resolve_image_view;

    if (!has_depth_resolve_mode_average) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-06102");
    }
    if (!has_stencil_resolve_mode_average) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pStencilAttachment-06103");
    }
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-06093");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    depth_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
    if (has_depth_resolve_mode_average && has_stencil_resolve_mode_average) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-06098");
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
    }

    depth_attachment.imageView = invalid_image_view;
    stencil_attachment.imageView = invalid_image_view;
    depth_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
    stencil_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-06088");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pStencilAttachment-06089");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    depth_attachment.imageView = depth_image_view;
    stencil_attachment.imageView = depth_image_view;
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-06092");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    stencil_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    if (depth_stencil_resolve_properties.independentResolveNone == VK_FALSE && has_depth_resolve_mode_average) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-06104");
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
    }
    if (depth_stencil_resolve_properties.independentResolve == VK_FALSE && has_depth_resolve_mode_average) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pDepthAttachment-06105");
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
    }

    stencil_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    stencil_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    if (has_stencil_resolve_mode_average) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pStencilAttachment-06095");
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
    }
    stencil_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
    if (has_stencil_resolve_mode_average) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pStencilAttachment-06099");
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
    }

    depth_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
    stencil_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
    stencil_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pStencilAttachment-06094");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidRenderingRenderAreaWithDeviceGroupExt) {
    TEST_DESCRIPTION("Use negative offset in RenderingInfo render area");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

        auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea.offset.x = -1;
    begin_rendering_info.renderArea.extent.width = 32;
    begin_rendering_info.renderArea.extent.height = 32;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06077");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = 0;
    begin_rendering_info.renderArea.offset.y = -1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06078");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, TestDynamicRenderingPipeline) {
    TEST_DESCRIPTION("Use pipeline created with render pass in dynamic render pass.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s not supported, skipping test\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-renderPass-06198");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, TestBeginRenderingFragmentShadingRateAttachmentSize) {
    TEST_DESCRIPTION("Test FragmentShadingRateAttachment size.");

    SetTargetApiVersion(VK_API_VERSION_1_0);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() != VK_API_VERSION_1_0) {
        printf("%s Tests requires Vulkan 1.0, skipping test\n", kSkipPrefix);
        return;
    }

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s or %s not supported, skipping test\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
               VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
        return;
    }

    auto vkGetPhysicalDeviceFeatures2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR"));
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto features11 = LvlInitStruct<VkPhysicalDeviceVulkan11Features>(&dynamic_rendering_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&features11);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (features11.multiview == VK_FALSE) {
        printf("%s Test requires (unsupported) multiview , skipping\n", kSkipPrefix);
        return;
    }
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();

    auto phys_dev_props_2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&fsr_properties);
    vk::GetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
               VK_IMAGE_TILING_LINEAR, 0);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UINT);

    auto fragment_shading_rate = LvlInitStruct<VkRenderingFragmentShadingRateAttachmentInfoKHR>();
    fragment_shading_rate.imageView = image_view;
    fragment_shading_rate.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    fragment_shading_rate.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&fragment_shading_rate);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea.offset.x = fragment_shading_rate.shadingRateAttachmentTexelSize.width * 64;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06117");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = 0;
    begin_rendering_info.renderArea.offset.y = fragment_shading_rate.shadingRateAttachmentTexelSize.height * 64;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06118");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestBeginRenderingFragmentShadingRateAttachmentSizeWithDeviceGroupExt) {
    TEST_DESCRIPTION("Test FragmentShadingRateAttachment size with device group extension.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s or %s not supported, skipping test\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
               VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto features11 = LvlInitStruct<VkPhysicalDeviceVulkan11Features>(&dynamic_rendering_features);
    VkPhysicalDeviceFeatures2 features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&features11);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (features11.multiview == VK_FALSE) {
        printf("%s Test requires (unsupported) multiview , skipping\n", kSkipPrefix);
        return;
    }
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    auto fsr_properties = LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();

    auto phys_dev_props_2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&fsr_properties);
    vk::GetPhysicalDeviceProperties2(gpu(), &phys_dev_props_2);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
               VK_IMAGE_TILING_LINEAR, 0);
    VkImageView image_view = image.targetView(VK_FORMAT_R8G8B8A8_UINT);

    auto fragment_shading_rate = LvlInitStruct<VkRenderingFragmentShadingRateAttachmentInfoKHR>();
    fragment_shading_rate.imageView = image_view;
    fragment_shading_rate.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    fragment_shading_rate.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&fragment_shading_rate);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea.offset.x = fragment_shading_rate.shadingRateAttachmentTexelSize.width * 64;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06119");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.renderArea.offset.x = 0;
    begin_rendering_info.renderArea.offset.y = fragment_shading_rate.shadingRateAttachmentTexelSize.height * 64;

    VkRect2D render_area = {};
    render_area.offset.x = 0;
    render_area.offset.y = 0;
    render_area.extent.width = 64 * fragment_shading_rate.shadingRateAttachmentTexelSize.width;
    render_area.extent.height = 32;

    auto device_group_render_pass_begin_info = LvlInitStruct<VkDeviceGroupRenderPassBeginInfo>();
    device_group_render_pass_begin_info.deviceRenderAreaCount = 1;
    device_group_render_pass_begin_info.pDeviceRenderAreas = &render_area;
    fragment_shading_rate.pNext = &device_group_render_pass_begin_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06120");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    render_area.extent.width = 32;
    render_area.extent.height = 64 * fragment_shading_rate.shadingRateAttachmentTexelSize.height;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-pNext-06122");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestSuspendingRenderPassInstance) {
    TEST_DESCRIPTION("Test suspending render pass instance.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s not supported, skipping test\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    m_errorMonitor->ExpectSuccess();

    VkCommandPoolObj command_pool(m_device, m_device->graphics_queue_node_index_);
    VkCommandBufferObj cmd_buffer1(m_device, &command_pool);
    VkCommandBufferObj cmd_buffer2(m_device, &command_pool);
    VkCommandBufferObj cmd_buffer3(m_device, &command_pool);

    VkRenderingInfo suspend_rendering_info = LvlInitStruct<VkRenderingInfo>();
    suspend_rendering_info.flags = VK_RENDERING_SUSPENDING_BIT;
    suspend_rendering_info.layerCount = 1;

    VkRenderingInfo resume_rendering_info = LvlInitStruct<VkRenderingInfo>();
    resume_rendering_info.flags = VK_RENDERING_RESUMING_BIT;
    resume_rendering_info.layerCount = 1;

    VkRenderingInfo rendering_info = LvlInitStruct<VkRenderingInfo>();
    rendering_info.layerCount = 1;

    auto cmd_begin = LvlInitStruct<VkCommandBufferBeginInfo>();

    cmd_buffer1.begin(&cmd_begin);
    cmd_buffer1.BeginRendering(suspend_rendering_info);
    cmd_buffer1.EndRendering();
    cmd_buffer1.end();

    cmd_buffer2.begin(&cmd_begin);
    cmd_buffer2.BeginRendering(resume_rendering_info);
    cmd_buffer2.EndRendering();
    cmd_buffer2.end();

    cmd_buffer3.begin(&cmd_begin);
    cmd_buffer3.BeginRendering(rendering_info);
    cmd_buffer3.EndRendering();
    cmd_buffer3.end();

    VkCommandBuffer command_buffers[3] = {cmd_buffer1.handle(), cmd_buffer2.handle()};

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 2;
    submit_info.pCommandBuffers = command_buffers;
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pCommandBuffers-06014");

    submit_info.commandBufferCount = 1;
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pCommandBuffers-06016");

    command_buffers[1] = cmd_buffer3.handle();
    command_buffers[2] = cmd_buffer2.handle();
    submit_info.commandBufferCount = 3;
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pCommandBuffers-06193");

    command_buffers[0] = cmd_buffer2.handle();
    submit_info.commandBufferCount = 1;
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestSuspendingRenderPassInstanceQueueSubmit2) {
    TEST_DESCRIPTION("Test suspending render pass instance with QueueSubmit2.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s or %s not supported, skipping test\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
               VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
        return;
    }

    auto synchronization2 = LvlInitStruct<VkPhysicalDeviceSynchronization2Features>();
    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeatures>(&synchronization2);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (dynamic_rendering_features.dynamicRendering == VK_FALSE) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }
    if (synchronization2.synchronization2 == VK_FALSE) {
        printf("%s Test requires (unsupported) synchronization2 , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto vkQueueSubmit2KHR =
        reinterpret_cast<PFN_vkQueueSubmit2KHR>(vk::GetDeviceProcAddr(m_device->device(), "vkQueueSubmit2KHR"));
    ASSERT_TRUE(vkQueueSubmit2KHR != nullptr);

    m_errorMonitor->ExpectSuccess();

    VkCommandPoolObj command_pool(m_device, m_device->graphics_queue_node_index_);
    VkCommandBufferObj cmd_buffer1(m_device, &command_pool);
    VkCommandBufferObj cmd_buffer2(m_device, &command_pool);
    VkCommandBufferObj cmd_buffer3(m_device, &command_pool);

    VkRenderingInfo suspend_rendering_info = LvlInitStruct<VkRenderingInfo>();
    suspend_rendering_info.flags = VK_RENDERING_SUSPENDING_BIT;
    suspend_rendering_info.layerCount = 1;

    VkRenderingInfo resume_rendering_info = LvlInitStruct<VkRenderingInfo>();
    resume_rendering_info.flags = VK_RENDERING_RESUMING_BIT;
    resume_rendering_info.layerCount = 1;

    VkRenderingInfo rendering_info = LvlInitStruct<VkRenderingInfo>();
    rendering_info.layerCount = 1;

    auto cmd_begin = LvlInitStruct<VkCommandBufferBeginInfo>();

    cmd_buffer1.begin(&cmd_begin);
    cmd_buffer1.BeginRendering(suspend_rendering_info);
    cmd_buffer1.EndRendering();
    cmd_buffer1.end();

    cmd_buffer2.begin(&cmd_begin);
    cmd_buffer2.BeginRendering(resume_rendering_info);
    cmd_buffer2.EndRendering();
    cmd_buffer2.end();

    cmd_buffer3.begin(&cmd_begin);
    cmd_buffer3.BeginRendering(rendering_info);
    cmd_buffer3.EndRendering();
    cmd_buffer3.end();

    VkCommandBufferSubmitInfo command_buffer_submit_info[3];
    command_buffer_submit_info[0] = LvlInitStruct<VkCommandBufferSubmitInfo>();
    command_buffer_submit_info[1] = LvlInitStruct<VkCommandBufferSubmitInfo>();
    command_buffer_submit_info[2] = LvlInitStruct<VkCommandBufferSubmitInfo>();

    command_buffer_submit_info[0].commandBuffer = cmd_buffer1.handle();
    command_buffer_submit_info[1].commandBuffer = cmd_buffer2.handle();

    VkSubmitInfo2KHR submit_info = LvlInitStruct<VkSubmitInfo2KHR>();
    submit_info.commandBufferInfoCount = 2;
    submit_info.pCommandBufferInfos = command_buffer_submit_info;
    vkQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2KHR-commandBuffer-06010");

    submit_info.commandBufferInfoCount = 1;
    vkQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2KHR-commandBuffer-06012");

    command_buffer_submit_info[1].commandBuffer = cmd_buffer3.handle();
    command_buffer_submit_info[2].commandBuffer = cmd_buffer2.handle();
    submit_info.commandBufferInfoCount = 3;
    vkQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo2KHR-commandBuffer-06192");

    command_buffer_submit_info[0].commandBuffer = cmd_buffer2.handle();
    submit_info.commandBufferInfoCount = 1;
    vkQueueSubmit2KHR(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    m_errorMonitor->VerifyFound();
}
