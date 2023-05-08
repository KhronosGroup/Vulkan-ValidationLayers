/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

class NegativeShaderPushConstants : public VkLayerTest {};

TEST_F(NegativeShaderPushConstants, NotDeclared) {
    TEST_DESCRIPTION(
        "Create a graphics pipeline in which a push constant range containing a push constant block member is not declared in the "
        "layout.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo { float x; } consts;
        void main(){
           gl_Position = vec4(consts.x);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    // Set up a push constant range
    VkPushConstantRange push_constant_range = {};
    // Set to the wrong stage to challenge core_validation
    push_constant_range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_range.size = 4;

    const VkPipelineLayoutObj pipeline_layout(m_device, {}, {push_constant_range});

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {}, {push_constant_range});

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-layout-07987");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderPushConstants, PipelineRange) {
    TEST_DESCRIPTION("Invalid use of VkPushConstantRange structs.");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkPhysicalDeviceProperties device_props = {};
    vk::GetPhysicalDeviceProperties(gpu(), &device_props);
    // will be at least 256 as required from the spec
    const uint32_t maxPushConstantsSize = device_props.limits.maxPushConstantsSize;

    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    VkPushConstantRange push_constant_range = {0, 0, 4};
    VkPipelineLayoutCreateInfo pipeline_layout_info{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &push_constant_range};

    // stageFlags of 0
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPushConstantRange-stageFlags-requiredbitmask");
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_info, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // offset over limit
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, maxPushConstantsSize, 8};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPushConstantRange-offset-00294");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPushConstantRange-size-00298");
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_info, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // offset not multiple of 4
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 1, 8};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPushConstantRange-offset-00295");
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_info, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // size of 0
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 0};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPushConstantRange-size-00296");
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_info, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // size not multiple of 4
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, 7};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPushConstantRange-size-00297");
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_info, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // size over limit
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, maxPushConstantsSize + 4};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPushConstantRange-size-00298");
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_info, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // size over limit of non-zero offset
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 4, maxPushConstantsSize};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPushConstantRange-size-00298");
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_info, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();

    // Sanity check its a valid range before making duplicate
    push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, maxPushConstantsSize};
    ASSERT_VK_SUCCESS(vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_info, NULL, &pipeline_layout));
    vk::DestroyPipelineLayout(m_device->device(), pipeline_layout, nullptr);

    // Duplicate ranges
    VkPushConstantRange push_constant_range_duplicate[2] = {push_constant_range, push_constant_range};
    pipeline_layout_info.pushConstantRangeCount = 2;
    pipeline_layout_info.pPushConstantRanges = push_constant_range_duplicate;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292");
    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_info, nullptr, &pipeline_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderPushConstants, NotInLayout) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a shader consuming push constants which are not provided in the pipeline layout");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo { float x; } consts;
        void main(){
           gl_Position = vec4(consts.x);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {});
    /* should have generated an error -- no push constant ranges provided! */
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-layout-07987");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeShaderPushConstants, Range) {
    TEST_DESCRIPTION("Invalid use of VkPushConstantRange values in vkCmdPushConstants.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    // Set limit to be same max as the shader usages
    const uint32_t maxPushConstantsSize = 16;
    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    props.limits.maxPushConstantsSize = maxPushConstantsSize;
    fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *const vsSource = R"glsl(
        #version 450
        layout(push_constant, std430) uniform foo { float x[4]; } constants;
        void main(){
           gl_Position = vec4(constants.x[0]);
        }
    )glsl";

    VkShaderObj const vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj const fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Set up a push constant range
    VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT, 0, maxPushConstantsSize};
    const VkPipelineLayoutObj pipeline_layout(m_device, {}, {push_constant_range});

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {}, {push_constant_range});
    pipe.CreateGraphicsPipeline();

    const float data[16] = {};  // dummy data to match shader size

    m_commandBuffer->begin();

    // size of 0
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushConstants-size-arraylength");
    vk::CmdPushConstants(m_commandBuffer->handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0, 0, data);
    m_errorMonitor->VerifyFound();

    // offset not multiple of 4
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushConstants-offset-00368");
    vk::CmdPushConstants(m_commandBuffer->handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 1, 4, data);
    m_errorMonitor->VerifyFound();

    // size not multiple of 4
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushConstants-size-00369");
    vk::CmdPushConstants(m_commandBuffer->handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0, 5, data);
    m_errorMonitor->VerifyFound();

    // offset at limit
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushConstants-offset-00370");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushConstants-size-00371");
    vk::CmdPushConstants(m_commandBuffer->handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT,
                         maxPushConstantsSize, 4, data);
    m_errorMonitor->VerifyFound();

    // size at limit
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPushConstants-size-00371");
    vk::CmdPushConstants(m_commandBuffer->handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0,
                         maxPushConstantsSize + 4, data);
    m_errorMonitor->VerifyFound();

    // Size at limit, should be valid
    vk::CmdPushConstants(m_commandBuffer->handle(), pipe.pipeline_layout_.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0,
                         maxPushConstantsSize, data);

    m_commandBuffer->end();
}
