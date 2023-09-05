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

#include "pipeline_helper.h"

CreatePipelineHelper::CreatePipelineHelper(VkLayerTest &test, uint32_t color_attachments_count)
    : cb_attachments_(color_attachments_count), layer_test_(test) {}

CreatePipelineHelper::~CreatePipelineHelper() {
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

void CreatePipelineHelper::InitDescriptorSetInfo() {
    dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
}

void CreatePipelineHelper::InitInputAndVertexInfo() {
    vi_ci_ = LvlInitStruct<VkPipelineVertexInputStateCreateInfo>();

    ia_ci_ = LvlInitStruct<VkPipelineInputAssemblyStateCreateInfo>();
    ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
}

void CreatePipelineHelper::InitMultisampleInfo() {
    pipe_ms_state_ci_ = LvlInitStruct<VkPipelineMultisampleStateCreateInfo>();
    pipe_ms_state_ci_.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipe_ms_state_ci_.sampleShadingEnable = VK_FALSE;
    pipe_ms_state_ci_.minSampleShading = 1.0;
    pipe_ms_state_ci_.pSampleMask = NULL;
}

void CreatePipelineHelper::InitPipelineLayoutInfo() {
    pipeline_layout_ci_ = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci_.setLayoutCount = 1;     // Not really changeable because InitState() sets exactly one pSetLayout
    pipeline_layout_ci_.pSetLayouts = nullptr;  // must bound after it is created
}

void CreatePipelineHelper::InitViewportInfo() {
    viewport_ = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    scissor_ = {{0, 0}, {64, 64}};

    vp_state_ci_ = LvlInitStruct<VkPipelineViewportStateCreateInfo>();
    vp_state_ci_.viewportCount = 1;
    vp_state_ci_.pViewports = &viewport_;  // ignored if dynamic
    vp_state_ci_.scissorCount = 1;
    vp_state_ci_.pScissors = &scissor_;  // ignored if dynamic
}

void CreatePipelineHelper::InitDynamicStateInfo() {
    // Use a "validity" check on the {} initialized structure to detect initialization
    // during late bind
}

void CreatePipelineHelper::InitShaderInfo() { ResetShaderInfo(kVertexMinimalGlsl, kFragmentMinimalGlsl); }

void CreatePipelineHelper::ResetShaderInfo(const char *vertex_shader_text, const char *fragment_shader_text) {
    vs_ = std::make_unique<VkShaderObj>(&layer_test_, vertex_shader_text, VK_SHADER_STAGE_VERTEX_BIT);
    fs_ = std::make_unique<VkShaderObj>(&layer_test_, fragment_shader_text, VK_SHADER_STAGE_FRAGMENT_BIT);
    // We shouldn't need a fragment shader but add it to be able to run on more devices
    shader_stages_ = {vs_->GetStageCreateInfo(), fs_->GetStageCreateInfo()};
}

void CreatePipelineHelper::InitRasterizationInfo() {
    rs_state_ci_ = LvlInitStruct<VkPipelineRasterizationStateCreateInfo>(&line_state_ci_);
    rs_state_ci_.flags = 0;
    rs_state_ci_.depthClampEnable = VK_FALSE;
    rs_state_ci_.rasterizerDiscardEnable = VK_FALSE;
    rs_state_ci_.polygonMode = VK_POLYGON_MODE_FILL;
    rs_state_ci_.cullMode = VK_CULL_MODE_BACK_BIT;
    rs_state_ci_.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rs_state_ci_.depthBiasEnable = VK_FALSE;
    rs_state_ci_.lineWidth = 1.0F;
}

void CreatePipelineHelper::InitLineRasterizationInfo() {
    line_state_ci_ = LvlInitStruct<VkPipelineRasterizationLineStateCreateInfoEXT>();
    line_state_ci_.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT;
    line_state_ci_.stippledLineEnable = VK_FALSE;
    line_state_ci_.lineStippleFactor = 0;
    line_state_ci_.lineStipplePattern = 0;
}

void CreatePipelineHelper::InitBlendStateInfo() {
    cb_ci_ = LvlInitStruct<VkPipelineColorBlendStateCreateInfo>();
    cb_ci_.logicOpEnable = VK_FALSE;
    cb_ci_.logicOp = VK_LOGIC_OP_COPY;  // ignored if enable is VK_FALSE above
    cb_ci_.attachmentCount = cb_attachments_.size();
    ASSERT_TRUE(IsValidVkStruct(layer_test_.RenderPassInfo()));
    cb_ci_.pAttachments = cb_attachments_.data();
    for (int i = 0; i < 4; i++) {
        cb_ci_.blendConstants[0] = 1.0F;
    }
}

void CreatePipelineHelper::InitGraphicsPipelineInfo() {
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
    gp_ci_ = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
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

void CreatePipelineHelper::InitPipelineCacheInfo() {
    pc_ci_ = LvlInitStruct<VkPipelineCacheCreateInfo>();
    pc_ci_.flags = 0;
    pc_ci_.initialDataSize = 0;
    pc_ci_.pInitialData = nullptr;
}

void CreatePipelineHelper::InitTesselationState() {
    // TBD -- add shaders and create_info
}

void CreatePipelineHelper::InitInfo() {
    InitDescriptorSetInfo();
    InitInputAndVertexInfo();
    InitMultisampleInfo();
    InitPipelineLayoutInfo();
    InitViewportInfo();
    InitDynamicStateInfo();
    InitShaderInfo();
    InitRasterizationInfo();
    InitLineRasterizationInfo();
    InitBlendStateInfo();
    InitGraphicsPipelineInfo();
    InitPipelineCacheInfo();
}

void CreatePipelineHelper::InitVertexInputLibInfo(void *p_next) {
    InitDescriptorSetInfo();
    InitInputAndVertexInfo();
    InitMultisampleInfo();
    InitPipelineLayoutInfo();
    InitViewportInfo();
    InitDynamicStateInfo();
    InitRasterizationInfo();
    InitLineRasterizationInfo();
    InitBlendStateInfo();

    gpl_info.emplace(LvlInitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(p_next));
    gpl_info->flags = VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT;

    gp_ci_ = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&gpl_info);
    gp_ci_.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    gp_ci_.pVertexInputState = &vi_ci_;
    gp_ci_.pInputAssemblyState = &ia_ci_;

    InitPipelineCacheInfo();
}

void CreatePipelineHelper::InitPreRasterLibInfo(uint32_t count, const VkPipelineShaderStageCreateInfo *info, void *p_next) {
    InitDescriptorSetInfo();
    InitInputAndVertexInfo();
    InitMultisampleInfo();
    InitPipelineLayoutInfo();
    InitViewportInfo();
    InitDynamicStateInfo();
    InitRasterizationInfo();
    InitLineRasterizationInfo();
    InitBlendStateInfo();

    gpl_info.emplace(LvlInitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(p_next));
    gpl_info->flags = VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT;

    gp_ci_ = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&gpl_info);
    gp_ci_.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    gp_ci_.pViewportState = &vp_state_ci_;
    gp_ci_.pRasterizationState = &rs_state_ci_;

    // If using Dynamic Rendering, will need to be set to null
    // otherwise needs to be shared across libraries in the same executable pipeline
    gp_ci_.renderPass = layer_test_.renderPass();
    gp_ci_.subpass = 0;

    gp_ci_.stageCount = count;
    gp_ci_.pStages = info;

    InitPipelineCacheInfo();
}

void CreatePipelineHelper::InitFragmentLibInfo(uint32_t count, const VkPipelineShaderStageCreateInfo *info, void *p_next) {
    InitDescriptorSetInfo();
    InitInputAndVertexInfo();
    InitMultisampleInfo();
    InitPipelineLayoutInfo();
    InitViewportInfo();
    InitDynamicStateInfo();
    InitRasterizationInfo();
    InitLineRasterizationInfo();
    InitBlendStateInfo();

    gpl_info.emplace(LvlInitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(p_next));
    gpl_info->flags = VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT;

    gp_ci_ = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&gpl_info);
    gp_ci_.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR;
    //  gp_ci_.pTessellationState = nullptr; // TODO
    gp_ci_.pViewportState = &vp_state_ci_;

    // If using Dynamic Rendering, will need to be set to null
    // otherwise needs to be shared across libraries in the same executable pipeline
    gp_ci_.renderPass = layer_test_.renderPass();
    gp_ci_.subpass = 0;

    // TODO if renderPass is null, MS info is not needed
    gp_ci_.pMultisampleState = &pipe_ms_state_ci_;

    gp_ci_.stageCount = count;
    gp_ci_.pStages = info;

    InitPipelineCacheInfo();
}

void CreatePipelineHelper::InitFragmentOutputLibInfo(void *p_next) {
    InitDescriptorSetInfo();
    InitInputAndVertexInfo();
    InitMultisampleInfo();
    InitPipelineLayoutInfo();
    InitViewportInfo();
    InitDynamicStateInfo();
    InitRasterizationInfo();
    InitLineRasterizationInfo();
    InitBlendStateInfo();

    gpl_info.emplace(LvlInitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(p_next));
    gpl_info->flags = VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT;

    gp_ci_ = LvlInitStruct<VkGraphicsPipelineCreateInfo>(&gpl_info);
    gp_ci_.flags = VK_PIPELINE_CREATE_LIBRARY_BIT_KHR | VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT;
    gp_ci_.pColorBlendState = &cb_ci_;
    gp_ci_.pMultisampleState = &pipe_ms_state_ci_;
    gp_ci_.pRasterizationState = &rs_state_ci_;

    // If using Dynamic Rendering, will need to be set to null
    // otherwise needs to be shared across libraries in the same executable pipeline
    gp_ci_.renderPass = layer_test_.renderPass();
    gp_ci_.subpass = 0;

    InitPipelineCacheInfo();
}

void CreatePipelineHelper::InitState() {
    descriptor_set_.reset(new OneOffDescriptorSet(layer_test_.DeviceObj(), dsl_bindings_));
    ASSERT_TRUE(descriptor_set_->Initialized());

    const std::vector<VkPushConstantRange> push_ranges(
        pipeline_layout_ci_.pPushConstantRanges,
        pipeline_layout_ci_.pPushConstantRanges + pipeline_layout_ci_.pushConstantRangeCount);
    pipeline_layout_ =
        VkPipelineLayoutObj(layer_test_.DeviceObj(), {&descriptor_set_->layout_}, push_ranges, pipeline_layout_ci_.flags);

    InitPipelineCache();
}

void CreatePipelineHelper::InitPipelineCache() {
    if (pipeline_cache_ != VK_NULL_HANDLE) {
        vk::DestroyPipelineCache(layer_test_.device(), pipeline_cache_, nullptr);
    }
    VkResult err = vk::CreatePipelineCache(layer_test_.device(), &pc_ci_, NULL, &pipeline_cache_);
    ASSERT_VK_SUCCESS(err);
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
    return vk::CreateGraphicsPipelines(layer_test_.device(), pipeline_cache_, 1, &gp_ci_, NULL, &pipeline_);
}

CreateComputePipelineHelper::CreateComputePipelineHelper(VkLayerTest &test) : layer_test_(test) {}

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

void CreateComputePipelineHelper::InitDescriptorSetInfo() {
    dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
}

void CreateComputePipelineHelper::InitPipelineLayoutInfo() {
    pipeline_layout_ci_ = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci_.setLayoutCount = 1;     // Not really changeable because InitState() sets exactly one pSetLayout
    pipeline_layout_ci_.pSetLayouts = nullptr;  // must bound after it is created
}

void CreateComputePipelineHelper::InitShaderInfo() {
    cs_ = std::make_unique<VkShaderObj>(&layer_test_, kMinimalShaderGlsl, VK_SHADER_STAGE_COMPUTE_BIT);
    // We shouldn't need a fragment shader but add it to be able to run on more devices
}

void CreateComputePipelineHelper::InitComputePipelineInfo() {
    cp_ci_ = LvlInitStruct<VkComputePipelineCreateInfo>();
    cp_ci_.flags = 0;
    cp_ci_.layout = VK_NULL_HANDLE;
}

void CreateComputePipelineHelper::InitPipelineCacheInfo() {
    pc_ci_ = LvlInitStruct<VkPipelineCacheCreateInfo>();
    pc_ci_.flags = 0;
    pc_ci_.initialDataSize = 0;
    pc_ci_.pInitialData = nullptr;
}

void CreateComputePipelineHelper::InitInfo() {
    InitDescriptorSetInfo();
    InitPipelineLayoutInfo();
    InitShaderInfo();
    InitComputePipelineInfo();
    InitPipelineCacheInfo();
}

void CreateComputePipelineHelper::InitState() {
    descriptor_set_.reset(new OneOffDescriptorSet(layer_test_.DeviceObj(), dsl_bindings_));
    ASSERT_TRUE(descriptor_set_->Initialized());

    const std::vector<VkPushConstantRange> push_ranges(
        pipeline_layout_ci_.pPushConstantRanges,
        pipeline_layout_ci_.pPushConstantRanges + pipeline_layout_ci_.pushConstantRangeCount);
    pipeline_layout_ = VkPipelineLayoutObj(layer_test_.DeviceObj(), {&descriptor_set_->layout_}, push_ranges);

    InitPipelineCache();
}

void CreateComputePipelineHelper::InitPipelineCache() {
    if (pipeline_cache_ != VK_NULL_HANDLE) {
        vk::DestroyPipelineCache(layer_test_.device(), pipeline_cache_, nullptr);
    }
    VkResult err = vk::CreatePipelineCache(layer_test_.device(), &pc_ci_, NULL, &pipeline_cache_);
    ASSERT_VK_SUCCESS(err);
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

RayTracingPipelineHelper::RayTracingPipelineHelper(VkLayerTest &test) : layer_test_(test) {}
RayTracingPipelineHelper::~RayTracingPipelineHelper() {
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

void RayTracingPipelineHelper::InitShaderGroups() {
    {
        VkRayTracingShaderGroupCreateInfoNV group = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group.generalShader = 0;
        group.closestHitShader = VK_SHADER_UNUSED_NV;
        group.anyHitShader = VK_SHADER_UNUSED_NV;
        group.intersectionShader = VK_SHADER_UNUSED_NV;
        groups_.push_back(group);
    }
    {
        VkRayTracingShaderGroupCreateInfoNV group = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
        group.generalShader = VK_SHADER_UNUSED_NV;
        group.closestHitShader = 1;
        group.anyHitShader = VK_SHADER_UNUSED_NV;
        group.intersectionShader = VK_SHADER_UNUSED_NV;
        groups_.push_back(group);
    }
    {
        VkRayTracingShaderGroupCreateInfoNV group = LvlInitStruct<VkRayTracingShaderGroupCreateInfoNV>();
        group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
        group.generalShader = 2;
        group.closestHitShader = VK_SHADER_UNUSED_NV;
        group.anyHitShader = VK_SHADER_UNUSED_NV;
        group.intersectionShader = VK_SHADER_UNUSED_NV;
        groups_.push_back(group);
    }
}

void RayTracingPipelineHelper::InitShaderGroupsKHR() {
    {
        VkRayTracingShaderGroupCreateInfoKHR group = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group.generalShader = 0;
        group.closestHitShader = VK_SHADER_UNUSED_KHR;
        group.anyHitShader = VK_SHADER_UNUSED_KHR;
        group.intersectionShader = VK_SHADER_UNUSED_KHR;
        groups_KHR_.push_back(group);
    }
    {
        VkRayTracingShaderGroupCreateInfoKHR group = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        group.generalShader = VK_SHADER_UNUSED_KHR;
        group.closestHitShader = 1;
        group.anyHitShader = VK_SHADER_UNUSED_KHR;
        group.intersectionShader = VK_SHADER_UNUSED_KHR;
        groups_KHR_.push_back(group);
    }
    {
        VkRayTracingShaderGroupCreateInfoKHR group = LvlInitStruct<VkRayTracingShaderGroupCreateInfoKHR>();
        group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group.generalShader = 2;
        group.closestHitShader = VK_SHADER_UNUSED_KHR;
        group.anyHitShader = VK_SHADER_UNUSED_KHR;
        group.intersectionShader = VK_SHADER_UNUSED_KHR;
        groups_KHR_.push_back(group);
    }
}
void RayTracingPipelineHelper::InitDescriptorSetInfo() {
    dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_NV, nullptr},
        {1, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1, VK_SHADER_STAGE_RAYGEN_BIT_NV, nullptr},
    };
}

void RayTracingPipelineHelper::InitDescriptorSetInfoKHR() {
    dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR, nullptr},
        {1, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR, nullptr},
    };
}

void RayTracingPipelineHelper::InitPipelineLayoutInfo() {
    pipeline_layout_ci_ = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci_.setLayoutCount = 1;     // Not really changeable because InitState() sets exactly one pSetLayout
    pipeline_layout_ci_.pSetLayouts = nullptr;  // must bound after it is created
}

void RayTracingPipelineHelper::InitShaderInfoKHR() {
    static const char rayGenShaderText[] = R"glsl(
        #version 460 core
        #extension GL_EXT_ray_tracing : enable
        layout(set = 0, binding = 0, rgba8) uniform image2D image;
        layout(set = 0, binding = 1) uniform accelerationStructureEXT as;

        layout(location = 0) rayPayloadEXT float payload;

        void main()
        {
           vec4 col = vec4(0, 0, 0, 1);

           vec3 origin = vec3(float(gl_LaunchIDEXT.x)/float(gl_LaunchSizeEXT.x), float(gl_LaunchIDEXT.y)/float(gl_LaunchSizeEXT.y), 1.0);
           vec3 dir = vec3(0.0, 0.0, -1.0);

           payload = 0.5;
           traceRayEXT(as, gl_RayFlagsCullBackFacingTrianglesEXT, 0xff, 0, 1, 0, origin, 0.0, dir, 1000.0, 0);

           col.y = payload;

           imageStore(image, ivec2(gl_LaunchIDEXT.xy), col);
        }
    )glsl";

    static char const closestHitShaderText[] = R"glsl(
        #version 460
        #extension GL_EXT_ray_tracing : enable
        layout(location = 0) rayPayloadInEXT float hitValue;

        void main() {
            hitValue = 1.0;
        }
    )glsl";

    static char const missShaderText[] = R"glsl(
        #version 460 core
        #extension GL_EXT_ray_tracing : enable
        layout(location = 0) rayPayloadInEXT float hitValue;

        void main() {
            hitValue = 0.0;
        }
    )glsl";

    rgs_ = std::make_unique<VkShaderObj>(&layer_test_, rayGenShaderText, VK_SHADER_STAGE_RAYGEN_BIT_KHR, SPV_ENV_VULKAN_1_2);
    chs_ =
        std::make_unique<VkShaderObj>(&layer_test_, closestHitShaderText, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, SPV_ENV_VULKAN_1_2);
    mis_ = std::make_unique<VkShaderObj>(&layer_test_, missShaderText, VK_SHADER_STAGE_MISS_BIT_KHR, SPV_ENV_VULKAN_1_2);

    shader_stages_ = {rgs_->GetStageCreateInfo(), chs_->GetStageCreateInfo(), mis_->GetStageCreateInfo()};
}

void RayTracingPipelineHelper::InitShaderInfo() {  // DONE
    static const char rayGenShaderText[] = R"glsl(
        #version 460 core
        #extension GL_NV_ray_tracing : require
        layout(set = 0, binding = 0, rgba8) uniform image2D image;
        layout(set = 0, binding = 1) uniform accelerationStructureNV as;

        layout(location = 0) rayPayloadNV float payload;

        void main()
        {
           vec4 col = vec4(0, 0, 0, 1);

           vec3 origin = vec3(float(gl_LaunchIDNV.x)/float(gl_LaunchSizeNV.x), float(gl_LaunchIDNV.y)/float(gl_LaunchSizeNV.y), 1.0);
           vec3 dir = vec3(0.0, 0.0, -1.0);

           payload = 0.5;
           traceNV(as, gl_RayFlagsCullBackFacingTrianglesNV, 0xff, 0, 1, 0, origin, 0.0, dir, 1000.0, 0);

           col.y = payload;

           imageStore(image, ivec2(gl_LaunchIDNV.xy), col);
        }
    )glsl";

    static char const closestHitShaderText[] = R"glsl(
        #version 460 core
        #extension GL_NV_ray_tracing : require
        layout(location = 0) rayPayloadInNV float hitValue;

        void main() {
            hitValue = 1.0;
        }
    )glsl";

    static char const missShaderText[] = R"glsl(
        #version 460 core
        #extension GL_NV_ray_tracing : require
        layout(location = 0) rayPayloadInNV float hitValue;

        void main() {
            hitValue = 0.0;
        }
    )glsl";

    rgs_ = std::make_unique<VkShaderObj>(&layer_test_, rayGenShaderText, VK_SHADER_STAGE_RAYGEN_BIT_NV);
    chs_ = std::make_unique<VkShaderObj>(&layer_test_, closestHitShaderText, VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);
    mis_ = std::make_unique<VkShaderObj>(&layer_test_, missShaderText, VK_SHADER_STAGE_MISS_BIT_NV);

    shader_stages_ = {rgs_->GetStageCreateInfo(), chs_->GetStageCreateInfo(), mis_->GetStageCreateInfo()};
}

void RayTracingPipelineHelper::InitNVRayTracingPipelineInfo() {
    rp_ci_ = LvlInitStruct<VkRayTracingPipelineCreateInfoNV>();
    rp_ci_.maxRecursionDepth = 0;
    rp_ci_.stageCount = shader_stages_.size();
    rp_ci_.pStages = shader_stages_.data();
    rp_ci_.groupCount = groups_.size();
    rp_ci_.pGroups = groups_.data();
}

void RayTracingPipelineHelper::InitKHRRayTracingPipelineInfo() {
    rp_ci_KHR_ = LvlInitStruct<VkRayTracingPipelineCreateInfoKHR>();
    rp_ci_KHR_.maxPipelineRayRecursionDepth = 0;
    rp_ci_KHR_.stageCount = shader_stages_.size();
    rp_ci_KHR_.pStages = shader_stages_.data();
    rp_ci_KHR_.groupCount = groups_KHR_.size();
    rp_ci_KHR_.pGroups = groups_KHR_.data();
}

void RayTracingPipelineHelper::InitPipelineCacheInfo() {
    pc_ci_ = LvlInitStruct<VkPipelineCacheCreateInfo>();
    pc_ci_.flags = 0;
    pc_ci_.initialDataSize = 0;
    pc_ci_.pInitialData = nullptr;
}

void RayTracingPipelineHelper::InitInfo(bool isKHR) {
    isKHR ? InitShaderGroupsKHR() : InitShaderGroups();
    isKHR ? InitDescriptorSetInfoKHR() : InitDescriptorSetInfo();
    InitPipelineLayoutInfo();
    isKHR ? InitShaderInfoKHR() : InitShaderInfo();
    isKHR ? InitKHRRayTracingPipelineInfo() : InitNVRayTracingPipelineInfo();
    InitPipelineCacheInfo();
}

void RayTracingPipelineHelper::InitState() {
    descriptor_set_.reset(new OneOffDescriptorSet(layer_test_.DeviceObj(), dsl_bindings_));
    ASSERT_TRUE(descriptor_set_->Initialized());

    pipeline_layout_ = VkPipelineLayoutObj(layer_test_.DeviceObj(), {&descriptor_set_->layout_});

    InitPipelineCache();
}

void RayTracingPipelineHelper::InitPipelineCache() {
    if (pipeline_cache_ != VK_NULL_HANDLE) {
        vk::DestroyPipelineCache(layer_test_.device(), pipeline_cache_, nullptr);
    }
    VkResult err = vk::CreatePipelineCache(layer_test_.device(), &pc_ci_, NULL, &pipeline_cache_);
    ASSERT_VK_SUCCESS(err);
}

void RayTracingPipelineHelper::LateBindPipelineInfo(bool isKHR) {
    // By value or dynamically located items must be late bound
    if (isKHR) {
        rp_ci_KHR_.layout = pipeline_layout_.handle();
        rp_ci_KHR_.stageCount = shader_stages_.size();
        rp_ci_KHR_.pStages = shader_stages_.data();
    } else {
        rp_ci_.layout = pipeline_layout_.handle();
        rp_ci_.stageCount = shader_stages_.size();
        rp_ci_.pStages = shader_stages_.data();
    }
}

VkResult RayTracingPipelineHelper::CreateNVRayTracingPipeline(bool do_late_bind) {
    if (do_late_bind) {
        LateBindPipelineInfo();
    }
    PFN_vkCreateRayTracingPipelinesNV vkCreateRayTracingPipelinesNV =
        (PFN_vkCreateRayTracingPipelinesNV)vk::GetInstanceProcAddr(layer_test_.instance(), "vkCreateRayTracingPipelinesNV");
    return vkCreateRayTracingPipelinesNV(layer_test_.device(), pipeline_cache_, 1, &rp_ci_, nullptr, &pipeline_);
}

VkResult RayTracingPipelineHelper::CreateKHRRayTracingPipeline(bool do_late_bind) {
    if (do_late_bind) {
        LateBindPipelineInfo(true /*isKHR*/);
    }
    PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR =
        (PFN_vkCreateRayTracingPipelinesKHR)vk::GetInstanceProcAddr(layer_test_.instance(), "vkCreateRayTracingPipelinesKHR");
    return vkCreateRayTracingPipelinesKHR(layer_test_.device(), 0, pipeline_cache_, 1, &rp_ci_KHR_, nullptr, &pipeline_);
}
