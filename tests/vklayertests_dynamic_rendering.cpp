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
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        printf("%s At least Vulkan version 1.2 is required for device, skipping test\n", kSkipPrefix);
        return;
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkCommandBufferInheritanceRenderingInfo-stencilAttachmentFormat-06199");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkCommandBufferInheritanceRenderingInfo-depthAttachmentFormat-06200");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkCommandBufferInheritanceRenderingInfo-pColorAttachmentFormats-06006");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkAttachmentSampleCountInfoAMD-pColorAttachmentSamples-parameter");

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
        printf("%s At least Vulkan version 1.2 is required for device, skipping test\n", kSkipPrefix);
        return;
    }

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
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

    uint32_t version = SetTargetApiVersion(VK_API_VERSION_1_2);
    if (version < VK_API_VERSION_1_2) {
        printf("%s At least Vulkan version 1.2 is required, skipping test.\n", kSkipPrefix);
        return;
    }

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        printf("%s At least Vulkan version 1.2 is required for device, skipping test\n", kSkipPrefix);
        return;
    }

     if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
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
    create_info.pColorBlendState = &color_blend_state_create_info;
    create_info.pNext = &pipeline_rendering_info;
    create_info.pTessellationState = &pipeline_tessellation_state_info;
    create_info.pInputAssemblyState = &pipeline_input_assembly_state_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06053");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06055");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06057");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06058");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06060");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineRenderingCreateInfo-multiview-06066");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineRenderingCreateInfo-pColorAttachmentFormats-06064");
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06062");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineRenderingCreateInfo-pColorAttachmentFormats-06064");
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
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
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
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
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
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
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

TEST_F(VkLayerTest, DynamicRenderingWithMistmatchingMixedAttachmentSamples) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering with mismatching mixed color/depth/stencil sample counts");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
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

    VkSampleCountFlagBits counts = {VK_SAMPLE_COUNT_2_BIT};
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
    samples_info.pColorAttachmentSamples = &counts;

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
    samples_info.depthStencilAttachmentSamples = counts;

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
        printf("%s At least Vulkan version 1.2 is required for device, skipping test\n", kSkipPrefix);
        return;
    }

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
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
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
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
    begin_rendering_info.pNext = &fragment_density_map;

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-multiview-06127");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06107");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06108");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06116");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06129");
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
        printf("%s At least Vulkan version 1.2 is required for device, skipping test\n", kSkipPrefix);
        return;
    }

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
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
        printf("%s At least Vulkan version 1.3 is required for device, skipping test\n", kSkipPrefix);
        return;
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

TEST_F(VkLayerTest, DynamicRenderingDepthFormatAndResolveMode) {
    TEST_DESCRIPTION("Test VkRenderingAttachmentInfo imageView with depth format.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() != VK_API_VERSION_1_1) {
        printf("%s Vulkan version 1.1 is required for device, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
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

    auto depth_stencil_props =
        LvlInitStruct<VkPhysicalDeviceDepthStencilResolveProperties>();
    auto props2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&depth_stencil_props);
    vk::GetPhysicalDeviceProperties2(gpu(), &props2);

    if ((depth_stencil_props.supportedDepthResolveModes & VK_RESOLVE_MODE_AVERAGE_BIT) == 0) {
        printf("%s Required depth resolve mode not supported, skipping.\n", kSkipPrefix);
        return;
    }

    VkFormat depth_format = FindSupportedDepthStencilFormat(gpu());
    if (depth_format == VK_FORMAT_UNDEFINED) {
        printf("%s No Depth + Stencil format found, skipping.\n", kSkipPrefix);
        return;
    }

    VkImageObj image(m_device);
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = depth_format;
    image_create_info.extent = {64, 64, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
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

    vk_testing::ImageView depth_image_view;
    depth_image_view.init(*m_device, ivci);

    VkRenderingAttachmentInfoKHR depth_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depth_attachment.imageView = depth_image_view.handle();
    depth_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    depth_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.pDepthAttachment = &depth_attachment;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06131");

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_commandBuffer->end();

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
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
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
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
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
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
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
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
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

TEST_F(VkLayerTest, TestDeviceGroupRenderPassBeginInfo) {
    TEST_DESCRIPTION("Test render area of DeviceGroupRenderPassBeginInfo.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
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

TEST_F(VkLayerTest, BeginRenderingInvalidDepthAttachmentFormat) {
    TEST_DESCRIPTION("Test begin rendering with a depth attachment that has an invalid format");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
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

TEST_F(VkLayerTest, BeginRenderingInvalidStencilAttachmentFormat) {
    TEST_DESCRIPTION("Test begin rendering with a stencil attachment that has an invalid format");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
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
