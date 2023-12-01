/*
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "pipeline_helper.h"

CreatePipelineHelper::CreatePipelineHelper(VkLayerTest &test, uint32_t color_attachments_count)
    : cb_attachments_(color_attachments_count), layer_test_(test) {
    // default VkDevice, can be overwritten if multi-device tests
    device_ = layer_test_.DeviceObj();

    // InitDescriptorSetInfo
    dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};

    // InitInputAndVertexInfo
    vi_ci_ = vku::InitStructHelper();

    ia_ci_ = vku::InitStructHelper();
    ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    // InitMultisampleInfo
    pipe_ms_state_ci_ = vku::InitStructHelper();
    pipe_ms_state_ci_.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipe_ms_state_ci_.sampleShadingEnable = VK_FALSE;
    pipe_ms_state_ci_.minSampleShading = 1.0;
    pipe_ms_state_ci_.pSampleMask = nullptr;

    // InitPipelineLayoutInfo
    pipeline_layout_ci_ = vku::InitStructHelper();
    pipeline_layout_ci_.setLayoutCount = 1;     // Not really changeable because InitState() sets exactly one pSetLayout
    pipeline_layout_ci_.pSetLayouts = nullptr;  // must bound after it is created

    // InitViewportInfo
    viewport_ = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    scissor_ = {{0, 0}, {64, 64}};
    vp_state_ci_ = vku::InitStructHelper();
    vp_state_ci_.viewportCount = 1;
    vp_state_ci_.pViewports = &viewport_;
    vp_state_ci_.scissorCount = 1;
    vp_state_ci_.pScissors = &scissor_;

    // InitRasterizationInfo
    rs_state_ci_ = vku::InitStructHelper(&line_state_ci_);
    rs_state_ci_.flags = 0;
    rs_state_ci_.depthClampEnable = VK_FALSE;
    rs_state_ci_.rasterizerDiscardEnable = VK_FALSE;
    rs_state_ci_.polygonMode = VK_POLYGON_MODE_FILL;
    rs_state_ci_.cullMode = VK_CULL_MODE_BACK_BIT;
    rs_state_ci_.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rs_state_ci_.depthBiasEnable = VK_FALSE;
    rs_state_ci_.lineWidth = 1.0F;

    // InitLineRasterizationInfo
    line_state_ci_ = vku::InitStructHelper();
    line_state_ci_.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT;
    line_state_ci_.stippledLineEnable = VK_FALSE;
    line_state_ci_.lineStippleFactor = 0;
    line_state_ci_.lineStipplePattern = 0;

    // InitBlendStateInfo
    cb_ci_ = vku::InitStructHelper();
    cb_ci_.logicOpEnable = VK_FALSE;
    cb_ci_.logicOp = VK_LOGIC_OP_COPY;  // ignored if enable is VK_FALSE above
    cb_ci_.attachmentCount = cb_attachments_.size();
    cb_ci_.pAttachments = cb_attachments_.data();
    for (int i = 0; i < 4; i++) {
        cb_ci_.blendConstants[0] = 1.0F;
    }

    // InitPipelineCacheInfo
    pc_ci_ = vku::InitStructHelper();
    pc_ci_.flags = 0;
    pc_ci_.initialDataSize = 0;
    pc_ci_.pInitialData = nullptr;

    InitShaderInfo();

    // Color-only rendering in a subpass with no depth/stencil attachment
    // Active Pipeline Shader Stages
    //    Vertex Shader
    //    Fragment Shader
    // Required: Fixed-Function Pipeline Stages
    //    VkPipelineVertexInputStateCreateInfo
    //    VkPipelineInputAssemblyStateCreateInfo
    //    VkPipelineViewportStateCreateInfo
    //    VkPipelineRasterizationStateCreateInfo
    //    VkPipelineMultisampleStateCreateInfo
    //    VkPipelineColorBlendStateCreateInfo
    gp_ci_ = vku::InitStructHelper();
    gp_ci_.layout = VK_NULL_HANDLE;
    gp_ci_.flags = VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT;
    gp_ci_.pVertexInputState = &vi_ci_;
    gp_ci_.pInputAssemblyState = &ia_ci_;
    gp_ci_.pTessellationState = nullptr;
    gp_ci_.pViewportState = &vp_state_ci_;
    gp_ci_.pMultisampleState = &pipe_ms_state_ci_;
    gp_ci_.pRasterizationState = &rs_state_ci_;
    gp_ci_.pDepthStencilState = nullptr;
    gp_ci_.pColorBlendState = &cb_ci_;
    gp_ci_.pDynamicState = nullptr;
    gp_ci_.renderPass = layer_test_.renderPass();
}

CreatePipelineHelper::~CreatePipelineHelper() {
    if (pipeline_cache_ != VK_NULL_HANDLE) {
        vk::DestroyPipelineCache(device_->handle(), pipeline_cache_, nullptr);
        pipeline_cache_ = VK_NULL_HANDLE;
    }
    if (pipeline_ != VK_NULL_HANDLE) {
        vk::DestroyPipeline(device_->handle(), pipeline_, nullptr);
        pipeline_ = VK_NULL_HANDLE;
    }
}

void CreatePipelineHelper::InitShaderInfo() { ResetShaderInfo(kVertexMinimalGlsl, kFragmentMinimalGlsl); }

void CreatePipelineHelper::ResetShaderInfo(const char *vertex_shader_text, const char *fragment_shader_text) {
    vs_ = std::make_unique<VkShaderObj>(&layer_test_, vertex_shader_text, VK_SHADER_STAGE_VERTEX_BIT);
    fs_ = std::make_unique<VkShaderObj>(&layer_test_, fragment_shader_text, VK_SHADER_STAGE_FRAGMENT_BIT);
    // We shouldn't need a fragment shader but add it to be able to run on more devices
    shader_stages_ = {vs_->GetStageCreateInfo(), fs_->GetStageCreateInfo()};
}

// Designed for majority of cases that just need to simply add dynamic state
void CreatePipelineHelper::AddDynamicState(VkDynamicState dynamic_state) {
    dynamic_states_.push_back(dynamic_state);
    dyn_state_ci_ = vku::InitStructHelper();
    dyn_state_ci_.pDynamicStates = dynamic_states_.data();
    dyn_state_ci_.dynamicStateCount = dynamic_states_.size();
    // Set here and don't have have to worry about late bind setting it
    gp_ci_.pDynamicState = &dyn_state_ci_;
}

void CreatePipelineHelper::InitVertexInputLibInfo(void *p_next) {
    gpl_info.emplace(vku::InitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(p_next));
    gpl_info->flags = VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT;

    gp_ci_ = vku::InitStructHelper(&gpl_info);
    gp_ci_.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    gp_ci_.pVertexInputState = &vi_ci_;
    gp_ci_.pInputAssemblyState = &ia_ci_;

    gp_ci_.stageCount = 0;
    shader_stages_.clear();
}

void CreatePipelineHelper::InitPreRasterLibInfo(const VkPipelineShaderStageCreateInfo *info, void *p_next) {
    gpl_info.emplace(vku::InitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(p_next));
    gpl_info->flags = VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT;

    gp_ci_ = vku::InitStructHelper(&gpl_info);
    gp_ci_.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    gp_ci_.pViewportState = &vp_state_ci_;
    gp_ci_.pRasterizationState = &rs_state_ci_;

    // If using Dynamic Rendering, will need to be set to null
    // otherwise needs to be shared across libraries in the same executable pipeline
    gp_ci_.renderPass = layer_test_.renderPass();
    gp_ci_.subpass = 0;

    gp_ci_.stageCount = 1;  // default is just the Vertex shader
    gp_ci_.pStages = info;
}

void CreatePipelineHelper::InitFragmentLibInfo(const VkPipelineShaderStageCreateInfo *info, void *p_next) {
    gpl_info.emplace(vku::InitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(p_next));
    gpl_info->flags = VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT;

    gp_ci_ = vku::InitStructHelper(&gpl_info);
    gp_ci_.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    //  gp_ci_.pTessellationState = nullptr; // TODO
    gp_ci_.pViewportState = &vp_state_ci_;

    // If using Dynamic Rendering, will need to be set to null
    // otherwise needs to be shared across libraries in the same executable pipeline
    gp_ci_.renderPass = layer_test_.renderPass();
    gp_ci_.subpass = 0;

    // TODO if renderPass is null, MS info is not needed
    gp_ci_.pMultisampleState = &pipe_ms_state_ci_;

    gp_ci_.stageCount = 1;  // default is just the Fragment shader
    gp_ci_.pStages = info;
}

void CreatePipelineHelper::InitFragmentOutputLibInfo(void *p_next) {
    gpl_info.emplace(vku::InitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(p_next));
    gpl_info->flags = VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT;

    gp_ci_ = vku::InitStructHelper(&gpl_info);
    gp_ci_.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR | VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT;
    gp_ci_.pColorBlendState = &cb_ci_;
    gp_ci_.pMultisampleState = &pipe_ms_state_ci_;
    gp_ci_.pRasterizationState = &rs_state_ci_;

    // If using Dynamic Rendering, will need to be set to null
    // otherwise needs to be shared across libraries in the same executable pipeline
    gp_ci_.renderPass = layer_test_.renderPass();
    gp_ci_.subpass = 0;

    gp_ci_.stageCount = 0;
    shader_stages_.clear();
}

void CreatePipelineHelper::InitState() {
    descriptor_set_.reset(new OneOffDescriptorSet(device_, dsl_bindings_));
    ASSERT_TRUE(descriptor_set_->Initialized());

    const std::vector<VkPushConstantRange> push_ranges(
        pipeline_layout_ci_.pPushConstantRanges,
        pipeline_layout_ci_.pPushConstantRanges + pipeline_layout_ci_.pushConstantRangeCount);
    pipeline_layout_ = vkt::PipelineLayout(*device_, {&descriptor_set_->layout_}, push_ranges, pipeline_layout_ci_.flags);

    InitPipelineCache();
}

void CreatePipelineHelper::InitPipelineCache() {
    if (pipeline_cache_ != VK_NULL_HANDLE) {
        vk::DestroyPipelineCache(device_->handle(), pipeline_cache_, nullptr);
    }
    VkResult err = vk::CreatePipelineCache(device_->handle(), &pc_ci_, NULL, &pipeline_cache_);
    ASSERT_EQ(VK_SUCCESS, err);
}

void CreatePipelineHelper::LateBindPipelineInfo() {
    // By value or dynamically located items must be late bound
    if (gp_ci_.layout == VK_NULL_HANDLE) {
        gp_ci_.layout = pipeline_layout_.handle();
    }
    if (gp_ci_.stageCount == 0) {
        gp_ci_.stageCount = shader_stages_.size();
        gp_ci_.pStages = shader_stages_.data();
    }
    if ((gp_ci_.pTessellationState == nullptr) && IsValidVkStruct(tess_ci_)) {
        gp_ci_.pTessellationState = &tess_ci_;
    }
    if ((gp_ci_.pDynamicState == nullptr) && IsValidVkStruct(dyn_state_ci_)) {
        gp_ci_.pDynamicState = &dyn_state_ci_;
    }
    if ((gp_ci_.pDepthStencilState == nullptr) && IsValidVkStruct(ds_ci_)) {
        gp_ci_.pDepthStencilState = &ds_ci_;
    }
}

VkResult CreatePipelineHelper::CreateGraphicsPipeline(bool do_late_bind) {
    if (do_late_bind) {
        LateBindPipelineInfo();
    }
    return vk::CreateGraphicsPipelines(device_->handle(), pipeline_cache_, 1, &gp_ci_, NULL, &pipeline_);
}

CreateComputePipelineHelper::CreateComputePipelineHelper(VkLayerTest &test) : layer_test_(test) {
    // InitDescriptorSetInfo
    dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};

    // InitPipelineLayoutInfo
    pipeline_layout_ci_ = vku::InitStructHelper();
    pipeline_layout_ci_.setLayoutCount = 1;     // Not really changeable because InitState() sets exactly one pSetLayout
    pipeline_layout_ci_.pSetLayouts = nullptr;  // must bound after it is created

    // InitPipelineCacheInfo
    pc_ci_ = vku::InitStructHelper();
    pc_ci_.flags = 0;
    pc_ci_.initialDataSize = 0;
    pc_ci_.pInitialData = nullptr;

    InitShaderInfo();

    cp_ci_ = vku::InitStructHelper();
    cp_ci_.flags = 0;
    cp_ci_.layout = VK_NULL_HANDLE;
}

CreateComputePipelineHelper::~CreateComputePipelineHelper() {
    VkDevice device = layer_test_.device();
    if (pipeline_cache_ != VK_NULL_HANDLE) {
        vk::DestroyPipelineCache(device, pipeline_cache_, nullptr);
        pipeline_cache_ = VK_NULL_HANDLE;
    }
    if (pipeline_ != VK_NULL_HANDLE) {
        vk::DestroyPipeline(device, pipeline_, nullptr);
        pipeline_ = VK_NULL_HANDLE;
    }
}

void CreateComputePipelineHelper::InitShaderInfo() {
    cs_ = std::make_unique<VkShaderObj>(&layer_test_, kMinimalShaderGlsl, VK_SHADER_STAGE_COMPUTE_BIT);
    // We shouldn't need a fragment shader but add it to be able to run on more devices
}

void CreateComputePipelineHelper::InitState() {
    descriptor_set_.reset(new OneOffDescriptorSet(layer_test_.DeviceObj(), dsl_bindings_));
    ASSERT_TRUE(descriptor_set_->Initialized());

    const std::vector<VkPushConstantRange> push_ranges(
        pipeline_layout_ci_.pPushConstantRanges,
        pipeline_layout_ci_.pPushConstantRanges + pipeline_layout_ci_.pushConstantRangeCount);
    pipeline_layout_ = vkt::PipelineLayout(*layer_test_.DeviceObj(), {&descriptor_set_->layout_}, push_ranges);

    InitPipelineCache();
}

void CreateComputePipelineHelper::InitPipelineCache() {
    if (pipeline_cache_ != VK_NULL_HANDLE) {
        vk::DestroyPipelineCache(layer_test_.device(), pipeline_cache_, nullptr);
    }
    VkResult err = vk::CreatePipelineCache(layer_test_.device(), &pc_ci_, NULL, &pipeline_cache_);
    ASSERT_EQ(VK_SUCCESS, err);
}

void CreateComputePipelineHelper::LateBindPipelineInfo() {
    // By value or dynamically located items must be late bound
    if (cp_ci_.layout == VK_NULL_HANDLE) {
        cp_ci_.layout = pipeline_layout_.handle();
    }
    cp_ci_.stage = cs_.get()->GetStageCreateInfo();
}

VkResult CreateComputePipelineHelper::CreateComputePipeline(bool do_late_bind) {
    if (do_late_bind) {
        LateBindPipelineInfo();
    }
    return vk::CreateComputePipelines(layer_test_.device(), pipeline_cache_, 1, &cp_ci_, NULL, &pipeline_);
}

void SetDefaultDynamicStates(VkCommandBuffer cmdBuffer) {
    uint32_t width = 32;
    uint32_t height = 32;
    VkViewport viewport = {0, 0, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f};
    VkRect2D scissor = {{0, 0}, {width, height}};
    vk::CmdSetViewportWithCountEXT(cmdBuffer, 1u, &viewport);
    vk::CmdSetScissorWithCountEXT(cmdBuffer, 1u, &scissor);
    vk::CmdSetLineWidth(cmdBuffer, 1.0f);
    vk::CmdSetDepthBias(cmdBuffer, 1.0f, 0.0f, 1.0f);
    float blendConstants[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    vk::CmdSetBlendConstants(cmdBuffer, blendConstants);
    vk::CmdSetDepthBounds(cmdBuffer, 0.0f, 1.0f);
    vk::CmdSetStencilCompareMask(cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, 0xFFFFFFFF);
    vk::CmdSetStencilWriteMask(cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, 0xFFFFFFFF);
    vk::CmdSetStencilReference(cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, 0xFFFFFFFF);
    vk::CmdSetCullModeEXT(cmdBuffer, VK_CULL_MODE_NONE);
    vk::CmdSetDepthBoundsTestEnableEXT(cmdBuffer, VK_FALSE);
    vk::CmdSetDepthCompareOpEXT(cmdBuffer, VK_COMPARE_OP_NEVER);
    vk::CmdSetDepthTestEnableEXT(cmdBuffer, VK_FALSE);
    vk::CmdSetDepthWriteEnableEXT(cmdBuffer, VK_FALSE);
    vk::CmdSetFrontFaceEXT(cmdBuffer, VK_FRONT_FACE_CLOCKWISE);
    vk::CmdSetPrimitiveTopologyEXT(cmdBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    vk::CmdSetStencilOpEXT(cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP,
                           VK_COMPARE_OP_NEVER);
    vk::CmdSetStencilTestEnableEXT(cmdBuffer, VK_FALSE);
    vk::CmdSetDepthBiasEnableEXT(cmdBuffer, VK_FALSE);
    vk::CmdSetPrimitiveRestartEnableEXT(cmdBuffer, VK_FALSE);
    vk::CmdSetRasterizerDiscardEnableEXT(cmdBuffer, VK_FALSE);
    vk::CmdSetVertexInputEXT(cmdBuffer, 0u, nullptr, 0u, nullptr);
    vk::CmdSetLogicOpEXT(cmdBuffer, VK_LOGIC_OP_COPY);
    vk::CmdSetPatchControlPointsEXT(cmdBuffer, 4u);
    vk::CmdSetTessellationDomainOriginEXT(cmdBuffer, VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT);
    vk::CmdSetDepthClampEnableEXT(cmdBuffer, VK_FALSE);
    vk::CmdSetPolygonModeEXT(cmdBuffer, VK_POLYGON_MODE_FILL);
    vk::CmdSetRasterizationSamplesEXT(cmdBuffer, VK_SAMPLE_COUNT_1_BIT);
    VkSampleMask sampleMask = 0xFFFFFFFF;
    vk::CmdSetSampleMaskEXT(cmdBuffer, VK_SAMPLE_COUNT_1_BIT, &sampleMask);
    vk::CmdSetAlphaToCoverageEnableEXT(cmdBuffer, VK_FALSE);
    vk::CmdSetAlphaToOneEnableEXT(cmdBuffer, VK_FALSE);
    vk::CmdSetLogicOpEnableEXT(cmdBuffer, VK_FALSE);
    VkBool32 colorBlendEnable = VK_FALSE;
    vk::CmdSetColorBlendEnableEXT(cmdBuffer, 0u, 1u, &colorBlendEnable);
    VkColorBlendEquationEXT colorBlendEquation = {
        VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD,
    };
    vk::CmdSetColorBlendEquationEXT(cmdBuffer, 0u, 1u, &colorBlendEquation);
    VkColorComponentFlags colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    vk::CmdSetColorWriteMaskEXT(cmdBuffer, 0u, 1u, &colorWriteMask);
}
