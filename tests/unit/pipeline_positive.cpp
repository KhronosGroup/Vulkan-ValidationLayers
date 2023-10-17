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
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "generated/vk_extension_helper.h"

TEST_F(PositivePipeline, ComplexTypes) {
    TEST_DESCRIPTION("Smoke test for complex types across VS/FS boundary");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    if (!m_device->phy().features().tessellationShader) {
        GTEST_SKIP() << "Device does not support tessellation shaders";
    }

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj tcs(this, kTessellationControlMinimalGlsl, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj tes(this, kTessellationEvalMinimalGlsl, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineInputAssemblyStateCreateInfo iasci{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                 VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, VK_FALSE};
    VkPipelineTessellationStateCreateInfo tsci{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, nullptr, 0, 3};

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.pTessellationState = &tsci;
    pipe.gp_ci_.pInputAssemblyState = &iasci;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), tcs.GetStageCreateInfo(), tes.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositivePipeline, BasicUsage) {
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositivePipeline, MissingDescriptorUnused) {
    TEST_DESCRIPTION(
        "Test that pipeline validation accepts a compute pipeline which declares a descriptor-backed resource which is not "
        "provided, but the shader does not statically use it. This is interesting because it requires compute pipelines to have a "
        "proper descriptor use walk, which they didn't for some time.");

    RETURN_IF_SKIP(Init())

    char const *csSource = R"glsl(
        #version 450
        layout(local_size_x=1) in;
        layout(set=0, binding=0) buffer block { vec4 x; };
        void main(){
           // x is not used.
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.InitState();
    pipe.CreateComputePipeline();
}

TEST_F(PositivePipeline, FragmentShadingRate) {
    TEST_DESCRIPTION("Verify that pipeline validation accepts a compute pipeline with fragment shading rate extension enabled");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = vku::InitStructHelper();
    VkPhysicalDeviceFeatures2 features2 = GetPhysicalDeviceFeatures2(fsr_features);
    if (fsr_features.pipelineFragmentShadingRate == VK_FALSE || fsr_features.primitiveFragmentShadingRate == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) pipelineFragmentShadingRate and primitiveFragmentShadingRate";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    char const *csSource = R"glsl(
        #version 450
        layout(local_size_x=1) in;
        layout(set=0, binding=0) buffer block { vec4 x; };
        void main(){
           // x is not used.
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.InitState();
    pipe.CreateComputePipeline();
}

TEST_F(PositivePipeline, CombinedImageSamplerConsumedAsSampler) {
    TEST_DESCRIPTION(
        "Test that pipeline validation accepts a shader consuming only the sampler portion of a combined image + sampler");

    RETURN_IF_SKIP(Init())

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };

    char const *csSource = R"glsl(
        #version 450
        layout(local_size_x=1) in;
        layout(set=0, binding=0) uniform sampler s;
        layout(set=0, binding=1) uniform texture2D t;
        layout(set=0, binding=2) buffer block { vec4 x; };
        void main() {
           x = texture(sampler2D(t, s), vec2(0));
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_.resize(bindings.size());
    memcpy(pipe.dsl_bindings_.data(), bindings.data(), bindings.size() * sizeof(VkDescriptorSetLayoutBinding));
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.InitState();
    pipe.CreateComputePipeline();
}

TEST_F(PositivePipeline, CombinedImageSamplerConsumedAsImage) {
    TEST_DESCRIPTION(
        "Test that pipeline validation accepts a shader consuming only the image portion of a combined image + sampler");

    RETURN_IF_SKIP(Init())

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };

    char const *csSource = R"glsl(
        #version 450
        layout(local_size_x=1) in;
        layout(set=0, binding=0) uniform texture2D t;
        layout(set=0, binding=1) uniform sampler s;
        layout(set=0, binding=2) buffer block { vec4 x; };
        void main() {
           x = texture(sampler2D(t, s), vec2(0));
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_.resize(bindings.size());
    memcpy(pipe.dsl_bindings_.data(), bindings.data(), bindings.size() * sizeof(VkDescriptorSetLayoutBinding));
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.InitState();
    pipe.CreateComputePipeline();
}

TEST_F(PositivePipeline, CombinedImageSamplerConsumedAsBoth) {
    TEST_DESCRIPTION(
        "Test that pipeline validation accepts a shader consuming both the sampler and the image of a combined image+sampler but "
        "via separate variables");

    RETURN_IF_SKIP(Init())

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };

    char const *csSource = R"glsl(
        #version 450
        layout(local_size_x=1) in;
        layout(set=0, binding=0) uniform texture2D t;
        layout(set=0, binding=0) uniform sampler s;  // both binding 0!
        layout(set=0, binding=1) buffer block { vec4 x; };
        void main() {
           x = texture(sampler2D(t, s), vec2(0));
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_.resize(bindings.size());
    memcpy(pipe.dsl_bindings_.data(), bindings.data(), bindings.size() * sizeof(VkDescriptorSetLayoutBinding));
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.InitState();
    pipe.CreateComputePipeline();
}

TEST_F(VkPositiveLayerTest, CreateGraphicsPipelineWithIgnoredPointers) {
    TEST_DESCRIPTION("Create Graphics Pipeline with pointers that must be ignored by layers");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init())

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    m_depthStencil->Init(m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                         VK_IMAGE_TILING_OPTIMAL);
    VkImageView depth_image_view =
        m_depthStencil->targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view);

    const uint64_t fake_address_64 = 0xCDCDCDCDCDCDCDCD;
    const uint64_t fake_address_32 = 0xCDCDCDCD;
    void *hopefully_undereferencable_pointer =
        sizeof(void *) == 8 ? reinterpret_cast<void *>(fake_address_64) : reinterpret_cast<void *>(fake_address_32);

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkPipelineShaderStageCreateInfo stages[2] = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};

    const VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        nullptr,  // pNext
        0,        // flags
        0,
        nullptr,  // bindings
        0,
        nullptr  // attributes
    };

    const VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        nullptr,  // pNext
        0,        // flags
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_FALSE  // primitive restart
    };

    const VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info_template{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        nullptr,   // pNext
        0,         // flags
        VK_FALSE,  // depthClamp
        VK_FALSE,  // rasterizerDiscardEnable
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_NONE,
        VK_FRONT_FACE_COUNTER_CLOCKWISE,
        VK_FALSE,  // depthBias
        0.0f,
        0.0f,
        0.0f,  // depthBias params
        1.0f   // lineWidth
    };

    const VkPipelineMultisampleStateCreateInfo pipeline_multisample_state_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        nullptr,  // pNext
        0,        // flags
        VK_SAMPLE_COUNT_1_BIT,
        VK_FALSE,  // sample shading
        0.0f,      // minSampleShading
        nullptr,   // pSampleMask
        VK_FALSE,  // alphaToCoverageEnable
        VK_FALSE   // alphaToOneEnable
    };

    vkt::PipelineLayout pipeline_layout;
    {
        VkPipelineLayoutCreateInfo pipeline_layout_create_info{
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            nullptr,  // pNext
            0,        // flags
            0,
            nullptr,  // layouts
            0,
            nullptr  // push constants
        };

        pipeline_layout.init(*m_device, pipeline_layout_create_info, {});
    }

    // try disabled rasterizer and no tessellation
    {
        VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info =
            pipeline_rasterization_state_create_info_template;
        pipeline_rasterization_state_create_info.rasterizerDiscardEnable = VK_TRUE;

        // If rasterizer is disabled, fragment shader state must not be present.
        VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{
            VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            nullptr,  // pNext
            0,        // flags
            1,        // stageCount
            stages,
            &pipeline_vertex_input_state_create_info,
            &pipeline_input_assembly_state_create_info,
            reinterpret_cast<const VkPipelineTessellationStateCreateInfo *>(hopefully_undereferencable_pointer),
            reinterpret_cast<const VkPipelineViewportStateCreateInfo *>(hopefully_undereferencable_pointer),
            &pipeline_rasterization_state_create_info,
            &pipeline_multisample_state_create_info,
            reinterpret_cast<const VkPipelineDepthStencilStateCreateInfo *>(hopefully_undereferencable_pointer),
            reinterpret_cast<const VkPipelineColorBlendStateCreateInfo *>(hopefully_undereferencable_pointer),
            nullptr,  // dynamic states
            pipeline_layout.handle(),
            m_renderPass,
            0,  // subpass
            VK_NULL_HANDLE,
            0};

        vkt::Pipeline pipeline(*m_device, graphics_pipeline_create_info);

        m_commandBuffer->begin();
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle());
    }

    // try enabled rasterizer but no subpass attachments
    {
        VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info =
            pipeline_rasterization_state_create_info_template;
        pipeline_rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;

        VkViewport viewport = {0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
        VkRect2D scissor = {{0, 0}, {m_width, m_height}};

        const VkPipelineViewportStateCreateInfo pipeline_viewport_state_create_info{
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            nullptr,  // pNext
            0,        // flags
            1,
            &viewport,
            1,
            &scissor};

        vkt::RenderPass render_pass;
        {
            VkSubpassDescription subpass_desc = {};

            VkRenderPassCreateInfo render_pass_create_info{
                VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                nullptr,  // pNext
                0,        // flags
                0,
                nullptr,  // attachments
                1,
                &subpass_desc,
                0,
                nullptr  // subpass dependencies
            };

            render_pass.init(*m_device, render_pass_create_info);
        }

        VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{
            VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            nullptr,                              // pNext
            0,                                    // flags
            static_cast<uint32_t>(size(stages)),  // stageCount
            stages,
            &pipeline_vertex_input_state_create_info,
            &pipeline_input_assembly_state_create_info,
            nullptr,
            &pipeline_viewport_state_create_info,
            &pipeline_rasterization_state_create_info,
            &pipeline_multisample_state_create_info,
            reinterpret_cast<const VkPipelineDepthStencilStateCreateInfo *>(hopefully_undereferencable_pointer),
            reinterpret_cast<const VkPipelineColorBlendStateCreateInfo *>(hopefully_undereferencable_pointer),
            nullptr,  // dynamic states
            pipeline_layout.handle(),
            render_pass.handle(),
            0,  // subpass
            VK_NULL_HANDLE,
            0};

        vkt::Pipeline pipeline(*m_device, graphics_pipeline_create_info);
    }

    // try dynamic viewport and scissor
    {
        VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info =
            pipeline_rasterization_state_create_info_template;
        pipeline_rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;

        const VkPipelineViewportStateCreateInfo pipeline_viewport_state_create_info{
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            nullptr,  // pNext
            0,        // flags
            1,
            reinterpret_cast<const VkViewport *>(hopefully_undereferencable_pointer),
            1,
            reinterpret_cast<const VkRect2D *>(hopefully_undereferencable_pointer)};

        const VkPipelineDepthStencilStateCreateInfo pipeline_depth_stencil_state_create_info{
            VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            nullptr,  // pNext
            0,        // flags
        };

        const VkPipelineColorBlendAttachmentState pipeline_color_blend_attachment_state = {};

        const VkPipelineColorBlendStateCreateInfo pipeline_color_blend_state_create_info{
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            nullptr,  // pNext
            0,        // flags
            VK_FALSE,
            VK_LOGIC_OP_CLEAR,
            1,
            &pipeline_color_blend_attachment_state,
            {0.0f, 0.0f, 0.0f, 0.0f}};

        const VkDynamicState dynamic_states[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        const VkPipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info{
            VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            nullptr,  // pNext
            0,        // flags
            2, dynamic_states};

        VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                                                   nullptr,                              // pNext
                                                                   0,                                    // flags
                                                                   static_cast<uint32_t>(size(stages)),  // stageCount
                                                                   stages,
                                                                   &pipeline_vertex_input_state_create_info,
                                                                   &pipeline_input_assembly_state_create_info,
                                                                   nullptr,
                                                                   &pipeline_viewport_state_create_info,
                                                                   &pipeline_rasterization_state_create_info,
                                                                   &pipeline_multisample_state_create_info,
                                                                   &pipeline_depth_stencil_state_create_info,
                                                                   &pipeline_color_blend_state_create_info,
                                                                   &pipeline_dynamic_state_create_info,  // dynamic states
                                                                   pipeline_layout.handle(),
                                                                   m_renderPass,
                                                                   0,  // subpass
                                                                   VK_NULL_HANDLE,
                                                                   0};

        vkt::Pipeline pipeline(*m_device, graphics_pipeline_create_info);
    }
}

TEST_F(PositivePipeline, CoreChecksDisabled) {
    TEST_DESCRIPTION("Test CreatePipeline while the CoreChecks validation object is disabled");

    // Enable KHR validation features extension
    VkValidationFeatureDisableEXT disables[] = {VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};
    VkValidationFeaturesEXT features = vku::InitStructHelper();
    features.disabledValidationFeatureCount = 1;
    features.pDisabledValidationFeatures = disables;

    RETURN_IF_SKIP(Init(nullptr, nullptr, &features));
    InitRenderTarget();
    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkPipelineInputAssemblyStateCreateInfo iasci{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                 VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE};

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.pInputAssemblyState = &iasci;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositivePipeline, TessellationDomainOrigin) {
    TEST_DESCRIPTION(
        "Test CreatePipeline when VkPipelineTessellationStateCreateInfo.pNext include "
        "VkPipelineTessellationDomainOriginStateCreateInfo");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    if (!m_device->phy().features().tessellationShader) {
        GTEST_SKIP() << "Device does not support tessellation shaders";
    }

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj tcs(this, kTessellationControlMinimalGlsl, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj tes(this, kTessellationEvalMinimalGlsl, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineInputAssemblyStateCreateInfo iasci{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
                                                 VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, VK_FALSE};

    VkPipelineTessellationDomainOriginStateCreateInfo tessellationDomainOriginStateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO, VK_NULL_HANDLE,
        VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT};

    VkPipelineTessellationStateCreateInfo tsci{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
                                               &tessellationDomainOriginStateInfo, 0, 3};

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.pTessellationState = &tsci;
    pipe.gp_ci_.pInputAssemblyState = &iasci;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), tcs.GetStageCreateInfo(), tes.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositivePipeline, ViewportArray2NV) {
    TEST_DESCRIPTION("Test to validate VK_NV_viewport_array2");

    AddRequiredExtensions(VK_NV_VIEWPORT_ARRAY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFeatures available_features = {};
    GetPhysicalDeviceFeatures(&available_features);

    if (!available_features.multiViewport) {
        GTEST_SKIP() << "VkPhysicalDeviceFeatures::multiViewport is not supported";
    }
    if (!available_features.tessellationShader) {
        GTEST_SKIP() << "VkPhysicalDeviceFeatures::tessellationShader is not supported";
    }
    if (!available_features.geometryShader) {
        GTEST_SKIP() << "VkPhysicalDeviceFeatures::geometryShader is not supported";
    }

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    const char tcs_src[] = R"glsl(
        #version 450
        layout(vertices = 3) out;

        void main() {
            gl_TessLevelOuter[0] = 4.0f;
            gl_TessLevelOuter[1] = 4.0f;
            gl_TessLevelOuter[2] = 4.0f;
            gl_TessLevelInner[0] = 3.0f;

            gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
        }
    )glsl";

    // Create tessellation control and fragment shader here since they will not be
    // modified by the different test cases.
    VkShaderObj tcs(this, tcs_src, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    const float fp_width = static_cast<float>(m_width);
    const float fp_height = static_cast<float>(m_height);

    std::vector<VkViewport> vps = {{0.0f, 0.0f, fp_width / 2.0f, fp_height}, {fp_width / 2.0f, 0.0f, fp_width / 2.0f, fp_height}};
    std::vector<VkRect2D> scs = {{{0, 0}, {m_width / 2, m_height}},
                                 {{static_cast<int32_t>(m_width) / 2, 0}, {m_width / 2, m_height}}};

    enum class TestStage { VERTEX = 0, TESSELLATION_EVAL = 1, GEOMETRY = 2 };
    std::array<TestStage, 3> vertex_stages = {{TestStage::VERTEX, TestStage::TESSELLATION_EVAL, TestStage::GEOMETRY}};

    // Verify that the usage of gl_ViewportMask[] in the allowed vertex processing
    // stages does not cause any errors.
    for (auto stage : vertex_stages) {
        CreatePipelineHelper pipe(*this);
        pipe.InitState();
        pipe.ia_ci_.topology =
            (stage != TestStage::VERTEX) ? VK_PRIMITIVE_TOPOLOGY_PATCH_LIST : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipe.gp_ci_.renderPass = renderPass();
        pipe.shader_stages_.clear();

        std::stringstream vs_src, tes_src, geom_src;

        vs_src << R"(
            #version 450
            #extension GL_NV_viewport_array2 : require

            vec2 positions[3] = { vec2( 0.0f, -0.5f),
                                  vec2( 0.5f,  0.5f),
                                  vec2(-0.5f,  0.5f)
                                };
            void main() {)";
        // Write viewportMask if the vertex shader is the last vertex processing stage.
        if (stage == TestStage::VERTEX) {
            vs_src << "gl_ViewportMask[0] = 3;\n";
        }
        vs_src << R"(
                gl_Position = vec4(positions[gl_VertexIndex % 3], 0.0, 1.0);
            })";

        VkShaderObj vs(this, vs_src.str().c_str(), VK_SHADER_STAGE_VERTEX_BIT);
        pipe.shader_stages_.push_back(vs.GetStageCreateInfo());

        std::unique_ptr<VkShaderObj> tes, geom;

        if (stage >= TestStage::TESSELLATION_EVAL) {
            tes_src << R"(
                #version 450
                #extension GL_NV_viewport_array2 : require
                layout(triangles) in;

                void main() {
                   gl_Position = (gl_in[0].gl_Position * gl_TessCoord.x +
                                  gl_in[1].gl_Position * gl_TessCoord.y +
                                  gl_in[2].gl_Position * gl_TessCoord.z);)";
            // Write viewportMask if the tess eval shader is the last vertex processing stage.
            if (stage == TestStage::TESSELLATION_EVAL) {
                tes_src << "gl_ViewportMask[0] = 3;\n";
            }
            tes_src << "}";

            tes = std::make_unique<VkShaderObj>(this, tes_src.str().c_str(), VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
            pipe.shader_stages_.push_back(tes.get()->GetStageCreateInfo());
            pipe.shader_stages_.push_back(tcs.GetStageCreateInfo());
            pipe.tess_ci_ = vku::InitStructHelper();
            pipe.tess_ci_.patchControlPoints = 3;
        }

        if (stage >= TestStage::GEOMETRY) {
            geom_src << R"(
                #version 450
                #extension GL_NV_viewport_array2 : require
                layout(triangles)   in;
                layout(triangle_strip, max_vertices = 3) out;

                void main() {
                   gl_ViewportMask[0] = 3;
                   for(int i = 0; i < 3; ++i) {
                       gl_Position = gl_in[i].gl_Position;
                       EmitVertex();
                    }
                })";

            geom = std::make_unique<VkShaderObj>(this, geom_src.str().c_str(), VK_SHADER_STAGE_GEOMETRY_BIT);
            pipe.shader_stages_.push_back(geom.get()->GetStageCreateInfo());
        }

        pipe.shader_stages_.push_back(fs.GetStageCreateInfo());
        pipe.CreateGraphicsPipeline();
    }
}

TEST_F(PositivePipeline, AttachmentUnused) {
    TEST_DESCRIPTION("Make sure unused attachments are correctly ignored.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 x;
        void main(){
           x = vec4(1);  // attachment is unused
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkAttachmentReference const color_attachments[1]{{VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};

    VkSubpassDescription const subpass_descriptions[1]{
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, color_attachments, nullptr, nullptr, 0, nullptr}};

    VkAttachmentDescription const attachment_descriptions[1]{{0, VK_FORMAT_B8G8R8A8_UNORM, VK_SAMPLE_COUNT_1_BIT,
                                                              VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
                                                              VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                              VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};

    VkRenderPassCreateInfo const render_pass_info{
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attachment_descriptions, 1, subpass_descriptions, 0, nullptr};
    vkt::RenderPass render_pass(*m_device, render_pass_info);

    const auto override_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.gp_ci_.renderPass = render_pass.handle();
    };
    CreatePipelineHelper::OneshotTest(*this, override_info, kErrorBit | kWarningBit);
}

TEST_F(PositivePipeline, SampleMaskOverrideCoverageNV) {
    TEST_DESCRIPTION("Test to validate VK_NV_sample_mask_override_coverage");

    AddRequiredExtensions(VK_NV_SAMPLE_MASK_OVERRIDE_COVERAGE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())

    const char vs_src[] = R"glsl(
        #version 450
        layout(location=0) out vec4  fragColor;

        const vec2 pos[3] = { vec2( 0.0f, -0.5f),
                              vec2( 0.5f,  0.5f),
                              vec2(-0.5f,  0.5f)
                            };
        void main()
        {
            gl_Position = vec4(pos[gl_VertexIndex % 3], 0.0f, 1.0f);
            fragColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
        }
    )glsl";

    const char fs_src[] = R"glsl(
        #version 450
        #extension GL_NV_sample_mask_override_coverage : require

        layout(location = 0) in  vec4 fragColor;
        layout(location = 0) out vec4 outColor;

        layout(override_coverage) out int gl_SampleMask[];

        void main()
        {
            gl_SampleMask[0] = 0xff;
            outColor = fragColor;
        }
    )glsl";

    const VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_8_BIT;

    VkAttachmentDescription cAttachment = {};
    cAttachment.format = VK_FORMAT_B8G8R8A8_UNORM;
    cAttachment.samples = sampleCount;
    cAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    cAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    cAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    cAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    cAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    cAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference cAttachRef = {};
    cAttachRef.attachment = 0;
    cAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &cAttachRef;

    VkRenderPassCreateInfo rpci = vku::InitStructHelper();
    rpci.attachmentCount = 1;
    rpci.pAttachments = &cAttachment;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    vkt::RenderPass rp(*m_device, rpci);

    const vkt::PipelineLayout pl(*m_device);

    VkSampleMask sampleMask = 0x01;
    VkPipelineMultisampleStateCreateInfo msaa = vku::InitStructHelper();
    msaa.rasterizationSamples = sampleCount;
    msaa.sampleShadingEnable = VK_FALSE;
    msaa.pSampleMask = &sampleMask;

    VkShaderObj vs(this, vs_src, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_src, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.gp_ci_.layout = pl.handle();
    pipe.gp_ci_.renderPass = rp.handle();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.pMultisampleState = &msaa;

    // Create pipeline and make sure that the usage of NV_sample_mask_override_coverage
    // in the fragment shader does not cause any errors.
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositivePipeline, RasterizationDiscardEnableTrue) {
    TEST_DESCRIPTION("Ensure it doesn't crash and trigger error msg when rasterizerDiscardEnable = true");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkAttachmentDescription att[1] = {{}};
    att[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    att[0].samples = VK_SAMPLE_COUNT_4_BIT;
    att[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    att[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    att[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkAttachmentReference cr = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription sp = {};
    sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sp.colorAttachmentCount = 1;
    sp.pColorAttachments = &cr;
    VkRenderPassCreateInfo rpi = vku::InitStructHelper();
    rpi.attachmentCount = 1;
    rpi.pAttachments = att;
    rpi.subpassCount = 1;
    rpi.pSubpasses = &sp;
    vkt::RenderPass rp(*m_device, rpi);
    ASSERT_TRUE(rp.initialized());

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.pViewportState = nullptr;
    pipe.gp_ci_.pMultisampleState = nullptr;
    pipe.gp_ci_.pDepthStencilState = nullptr;
    pipe.gp_ci_.pColorBlendState = nullptr;
    pipe.gp_ci_.renderPass = rp.handle();

    // When rasterizer discard is enabled, fragment shader state must not be present.
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo()};

    // Skip the test in NexusPlayer. The driver crashes when pViewportState, pMultisampleState, pDepthStencilState, pColorBlendState
    // are NULL.
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositivePipeline, SamplerDataForCombinedImageSampler) {
    TEST_DESCRIPTION("Shader code uses sampler data for CombinedImageSampler");
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    const char *fsSource = R"(
                   OpCapability Shader
                   OpMemoryModel Logical GLSL450
                   OpEntryPoint Fragment %main "main"
                   OpExecutionMode %main OriginUpperLeft

                   OpDecorate %InputData DescriptorSet 0
                   OpDecorate %InputData Binding 0
                   OpDecorate %SamplerData DescriptorSet 0
                   OpDecorate %SamplerData Binding 0

               %void = OpTypeVoid
                %f32 = OpTypeFloat 32
              %Image = OpTypeImage %f32 2D 0 0 0 1 Rgba32f
           %ImagePtr = OpTypePointer UniformConstant %Image
          %InputData = OpVariable %ImagePtr UniformConstant
            %Sampler = OpTypeSampler
         %SamplerPtr = OpTypePointer UniformConstant %Sampler
        %SamplerData = OpVariable %SamplerPtr UniformConstant
       %SampledImage = OpTypeSampledImage %Image

               %func = OpTypeFunction %void
               %main = OpFunction %void None %func
                 %40 = OpLabel
           %call_smp = OpLoad %Sampler %SamplerData
                   OpReturn
                   OpFunctionEnd)";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    CreatePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    pipe.shader_stages_ = {fs.GetStageCreateInfo(), pipe.vs_->GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView view = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);

    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositivePipeline, ConditionalRendering) {
    TEST_DESCRIPTION("Create renderpass and CmdPipelineBarrier with VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceConditionalRenderingFeaturesEXT cond_rendering_feature = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(cond_rendering_feature);
    if (cond_rendering_feature.conditionalRendering == VK_FALSE) {
        GTEST_SKIP() << "conditionalRendering feature not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    // A renderpass with a single subpass that declared a self-dependency
    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
    };

    VkSubpassDependency dependency = {0,
                                      0,
                                      VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                                      VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT,
                                      VK_ACCESS_SHADER_WRITE_BIT,
                                      VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT,
                                      (VkDependencyFlags)0};
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 1, subpasses, 1, &dependency};
    vkt::RenderPass rp(*m_device, rpci);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp.handle(), 1, &imageView, 32, 32, 1};
    vkt::Framebuffer fb(*m_device, fbci);

    m_commandBuffer->begin();
    VkRenderPassBeginInfo rpbi =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, rp.handle(), fb.handle(), VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);

    VkImageMemoryBarrier imb = vku::InitStructHelper();
    imb.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    imb.dstAccessMask = VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT;
    imb.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imb.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imb.image = image.handle();
    imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imb.subresourceRange.baseMipLevel = 0;
    imb.subresourceRange.levelCount = 1;
    imb.subresourceRange.baseArrayLayer = 0;
    imb.subresourceRange.layerCount = 1;

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                           VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT, 0, 0, nullptr, 0, nullptr, 1, &imb);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositivePipeline, ShaderTileImage) {
    TEST_DESCRIPTION("Create graphics pipeline with shader tile image extension.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features = vku::InitStructHelper();
    VkPhysicalDeviceShaderTileImageFeaturesEXT shader_tile_image_features = vku::InitStructHelper();
    dynamic_rendering_features.pNext = &shader_tile_image_features;
    auto features2 = GetPhysicalDeviceFeatures2(dynamic_rendering_features);
    if (!dynamic_rendering_features.dynamicRendering) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering";
    }

    // None of the shader tile image read features supported skip the test.
    if (!shader_tile_image_features.shaderTileImageColorReadAccess && !shader_tile_image_features.shaderTileImageDepthReadAccess &&
        !shader_tile_image_features.shaderTileImageStencilReadAccess) {
        GTEST_SKIP() << "Test requires (unsupported) shader tile image extension.";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkFormat depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    VkFormat color_format = VK_FORMAT_B8G8R8A8_UNORM;
    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_format;
    pipeline_rendering_info.depthAttachmentFormat = depth_format;
    pipeline_rendering_info.stencilAttachmentFormat = depth_format;

    VkPipelineDepthStencilStateCreateInfo ds_ci = vku::InitStructHelper();
    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);

    if (shader_tile_image_features.shaderTileImageDepthReadAccess) {
        auto fs = VkShaderObj::CreateFromASM(this, kShaderTileImageDepthReadSpv, VK_SHADER_STAGE_FRAGMENT_BIT);

        ds_ci.depthWriteEnable = false;
        CreatePipelineHelper pipe(*this);
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs->GetStageCreateInfo()};
        pipe.gp_ci_.pNext = &pipeline_rendering_info;
        pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
        pipe.gp_ci_.pDepthStencilState = &ds_ci;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
    }

    if (shader_tile_image_features.shaderTileImageStencilReadAccess) {
        auto fs = VkShaderObj::CreateFromASM(this, kShaderTileImageStencilReadSpv, VK_SHADER_STAGE_FRAGMENT_BIT);

        VkStencilOpState stencil_state = {};
        stencil_state.failOp = VK_STENCIL_OP_KEEP;
        stencil_state.depthFailOp = VK_STENCIL_OP_KEEP;
        stencil_state.passOp = VK_STENCIL_OP_REPLACE;
        stencil_state.compareOp = VK_COMPARE_OP_LESS;
        stencil_state.compareMask = 0xff;
        stencil_state.writeMask = 0;
        stencil_state.reference = 0xf;

        ds_ci = {};
        ds_ci.front = stencil_state;
        ds_ci.back = stencil_state;

        CreatePipelineHelper pipe(*this);
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs->GetStageCreateInfo()};
        pipe.gp_ci_.pNext = &pipeline_rendering_info;
        pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
        pipe.gp_ci_.pDepthStencilState = &ds_ci;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();
    }

    if (shader_tile_image_features.shaderTileImageColorReadAccess) {
        auto fs = VkShaderObj::CreateFromASM(this, kShaderTileImageColorReadSpv, VK_SHADER_STAGE_FRAGMENT_BIT);

        VkPipelineMultisampleStateCreateInfo ms_ci = vku::InitStructHelper();
        ms_ci.sampleShadingEnable = VK_TRUE;
        ms_ci.minSampleShading = 1.0;
        ms_ci.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;

        CreatePipelineHelper ms_pipeline(*this);
        ms_pipeline.shader_stages_ = {vs.GetStageCreateInfo(), fs->GetStageCreateInfo()};
        ms_pipeline.gp_ci_.pNext = &pipeline_rendering_info;
        ms_pipeline.gp_ci_.renderPass = VK_NULL_HANDLE;
        ms_pipeline.gp_ci_.pDepthStencilState = &ds_ci;
        ms_pipeline.gp_ci_.pMultisampleState = &ms_ci;
        ms_pipeline.InitState();
        ms_pipeline.CreateGraphicsPipeline();
    }
}

TEST_F(VkPositiveLayerTest, TestPervertexNVShaderAttributes) {
    TEST_DESCRIPTION("Test using TestRasterizationStateStreamCreateInfoEXT with invalid rasterizationStream.");

    AddRequiredExtensions(VK_NV_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV fragment_shader_barycentric_features =
        vku::InitStructHelper();
    fragment_shader_barycentric_features.fragmentShaderBarycentric = VK_TRUE;
    VkPhysicalDeviceFeatures2KHR features2 = vku::InitStructHelper(&fragment_shader_barycentric_features);
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    InitRenderTarget();

    char const *vsSource = R"glsl(
                #version 450

                layout(location = 0) out PerVertex {
                    vec3 vtxPos;
                } outputs;

                vec2 triangle_positions[3] = vec2[](
                    vec2(0.5, -0.5),
                    vec2(0.5, 0.5),
                    vec2(-0.5, 0.5)
                );

                void main() {
                    gl_Position = vec4(triangle_positions[gl_VertexIndex], 0.0, 1.0);
                    outputs.vtxPos = gl_Position.xyz;
                }
            )glsl";

    char const *fsSource = R"glsl(
                #version 450

                #extension GL_NV_fragment_shader_barycentric : enable

                layout(location = 0) in pervertexNV PerVertex {
                    vec3 vtxPos;
                } inputs[3];

                layout(location = 0) out vec4 out_color;

                void main() {
                    vec3 b = gl_BaryCoordNV;
                    if (b.x > b.y && b.x > b.z) {
                        out_color = vec4(inputs[0].vtxPos, 1.0);
                    }
                    else if(b.y > b.z) {
                        out_color = vec4(inputs[1].vtxPos, 1.0);
                    }
                    else {
                        out_color = vec4(inputs[2].vtxPos, 1.0);
                    }
                }
            )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositivePipeline, MutableStorageImageFormatWriteForFormat) {
    TEST_DESCRIPTION("Create a shader writing a storage image without an image format");

    // need to be compatible to use VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT
    const VkFormat image_format = VK_FORMAT_B8G8R8A8_SRGB;
    const VkFormat image_view_format = VK_FORMAT_R32_SFLOAT;

    AddRequiredExtensions(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    PFN_vkSetPhysicalDeviceFormatProperties2EXT fpvkSetPhysicalDeviceFormatProperties2EXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatProperties2EXT fpvkGetOriginalPhysicalDeviceFormatProperties2EXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatProperties2EXT, fpvkGetOriginalPhysicalDeviceFormatProperties2EXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkFormatProperties3 fmt_props_3 = vku::InitStructHelper();
    VkFormatProperties2 fmt_props = vku::InitStructHelper(&fmt_props_3);

    fpvkGetOriginalPhysicalDeviceFormatProperties2EXT(gpu(), image_format, &fmt_props);
    fmt_props.formatProperties.optimalTilingFeatures =
        (fmt_props.formatProperties.optimalTilingFeatures & ~VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT);
    fmt_props_3.optimalTilingFeatures = (fmt_props_3.optimalTilingFeatures & ~VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT);
    fmt_props_3.optimalTilingFeatures = (fmt_props_3.optimalTilingFeatures & ~VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT);
    fpvkSetPhysicalDeviceFormatProperties2EXT(gpu(), image_format, fmt_props);

    fpvkGetOriginalPhysicalDeviceFormatProperties2EXT(gpu(), image_view_format, &fmt_props);
    fmt_props.formatProperties.optimalTilingFeatures |= VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT;
    fmt_props_3.optimalTilingFeatures |= VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT;
    fmt_props_3.optimalTilingFeatures |= VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT;
    fpvkSetPhysicalDeviceFormatProperties2EXT(gpu(), image_view_format, fmt_props);

    // Make sure compute pipeline has a compute shader stage set
    const char *csSource = R"(
                  OpCapability Shader
                  OpCapability StorageImageWriteWithoutFormat
             %1 = OpExtInstImport "GLSL.std.450"
                  OpMemoryModel Logical GLSL450
                  OpEntryPoint GLCompute %main "main"
                  OpExecutionMode %main LocalSize 1 1 1
                  OpSource GLSL 450
                  OpName %main "main"
                  OpName %img "img"
                  OpDecorate %img DescriptorSet 0
                  OpDecorate %img Binding 0
                  OpDecorate %img NonWritable
          %void = OpTypeVoid
             %3 = OpTypeFunction %void
         %float = OpTypeFloat 32
             %7 = OpTypeImage %float 2D 0 0 0 2 Unknown
%_ptr_UniformConstant_7 = OpTypePointer UniformConstant %7
           %img = OpVariable %_ptr_UniformConstant_7 UniformConstant
           %int = OpTypeInt 32 1
         %v2int = OpTypeVector %int 2
         %int_0 = OpConstant %int 0
            %14 = OpConstantComposite %v2int %int_0 %int_0
       %v4float = OpTypeVector %float 4
       %float_0 = OpConstant %float 0
            %17 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
          %uint = OpTypeInt 32 0
        %v3uint = OpTypeVector %uint 3
        %uint_1 = OpConstant %uint 1
          %main = OpFunction %void None %3
             %5 = OpLabel
            %10 = OpLoad %7 %img
                  OpImageWrite %10 %14 %17
                  OpReturn
                  OpFunctionEnd
                  )";

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                     });

    CreateComputePipelineHelper cs_pipeline(*this);
    cs_pipeline.cs_ =
        std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    cs_pipeline.InitState();
    cs_pipeline.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&ds.layout_});
    cs_pipeline.LateBindPipelineInfo();
    cs_pipeline.cp_ci_.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;  // override with wrong value
    cs_pipeline.CreateComputePipeline(false);                      // need false to prevent late binding

    // Messing with format support, make sure device will handle the image combination
    VkImageFormatProperties format_props;
    if (VK_SUCCESS != vk::GetPhysicalDeviceImageFormatProperties(gpu(), image_format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                                                 VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
                                                                 &format_props)) {
        GTEST_SKIP() << "Device will not be able to initialize buffer view skipped";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = image_format;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_STORAGE_BIT;
    VkImageObj image(m_device);
    image.Init(image_create_info);

    VkDescriptorImageInfo image_info = {};
    image_info.imageView = image.targetView(image_view_format);
    image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptor_write.pImageInfo = &image_info;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    m_commandBuffer->reset();
    m_commandBuffer->begin();

    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = VK_ACCESS_HOST_READ_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.image = image.handle();  // Image mis-matches with FB image
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipeline.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, cs_pipeline.pipeline_layout_.handle(), 0,
                              1, &ds.set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();
}

TEST_F(PositivePipeline, CreateGraphicsPipelineRasterizationOrderAttachmentAccessFlags) {
    TEST_DESCRIPTION("Test for a creating a pipeline with VK_ARM_rasterization_order_attachment_access enabled");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesARM rasterization_order_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(rasterization_order_features);

    if (!rasterization_order_features.rasterizationOrderColorAttachmentAccess &&
        !rasterization_order_features.rasterizationOrderDepthAttachmentAccess &&
        !rasterization_order_features.rasterizationOrderStencilAttachmentAccess) {
        GTEST_SKIP() << "Test requires (unsupported) rasterizationOrder*AttachmentAccess";
    }

    RETURN_IF_SKIP(InitState(nullptr, &rasterization_order_features));

    VkPipelineDepthStencilStateCreateInfo ds_ci = vku::InitStructHelper();
    VkPipelineColorBlendAttachmentState cb_as = {};
    VkPipelineColorBlendStateCreateInfo cb_ci = vku::InitStructHelper();
    cb_ci.attachmentCount = 1;
    cb_ci.pAttachments = &cb_as;
    VkRenderPass render_pass_handle = VK_NULL_HANDLE;

    auto create_render_pass = [&](VkPipelineDepthStencilStateCreateFlags subpass_flags, vkt::RenderPass &render_pass) {
        VkAttachmentDescription attachments[2] = {};
        attachments[0].flags = 0;
        attachments[0].format = VK_FORMAT_B8G8R8A8_UNORM;
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachments[1].flags = 0;
        attachments[1].format = FindSupportedDepthStencilFormat(this->gpu());
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference cAttachRef = {};
        cAttachRef.attachment = 0;
        cAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference dsAttachRef = {};
        dsAttachRef.attachment = 1;
        dsAttachRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &cAttachRef;
        subpass.pDepthStencilAttachment = &dsAttachRef;
        subpass.flags = subpass_flags;

        VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        rpci.attachmentCount = 2;
        rpci.pAttachments = attachments;
        rpci.subpassCount = 1;
        rpci.pSubpasses = &subpass;

        render_pass.init(*this->m_device, rpci);
    };

    auto set_flgas_pipeline_createinfo = [&](CreatePipelineHelper &helper) {
        helper.gp_ci_.pDepthStencilState = &ds_ci;
        helper.gp_ci_.pColorBlendState = &cb_ci;
        helper.gp_ci_.renderPass = render_pass_handle;
    };

    // Color attachment
    if (rasterization_order_features.rasterizationOrderColorAttachmentAccess) {
        cb_ci.flags = VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_BIT_ARM;
        ds_ci.flags = 0;

        vkt::RenderPass render_pass;
        create_render_pass(VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_COLOR_ACCESS_BIT_ARM, render_pass);
        render_pass_handle = render_pass.handle();
        CreatePipelineHelper::OneshotTest(*this, set_flgas_pipeline_createinfo, kErrorBit);
    }

    // Depth attachment
    if (rasterization_order_features.rasterizationOrderDepthAttachmentAccess) {
        cb_ci.flags = 0;
        ds_ci.flags = VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_ARM;

        vkt::RenderPass render_pass;
        create_render_pass(VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_ARM, render_pass);
        render_pass_handle = render_pass.handle();
        CreatePipelineHelper::OneshotTest(*this, set_flgas_pipeline_createinfo, kErrorBit);
    }

    // Stencil attachment
    if (rasterization_order_features.rasterizationOrderStencilAttachmentAccess) {
        cb_ci.flags = 0;
        ds_ci.flags = VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_ARM;

        vkt::RenderPass render_pass;
        create_render_pass(VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_ARM, render_pass);
        render_pass_handle = render_pass.handle();

        CreatePipelineHelper::OneshotTest(*this, set_flgas_pipeline_createinfo, kErrorBit);
    }
}

TEST_F(PositivePipeline, AttachmentsDisableRasterization) {
    TEST_DESCRIPTION(
        "Create a pipeline with rasterization disabled, containing a valid pColorBlendState and color attachments, "
        "without a fragment shader");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    // Rasterization discard enable prohibits fragment shader.
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositivePipeline, DualBlendShader) {
    TEST_DESCRIPTION("Test drawing with dual source blending with too many fragment output attachments.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(features2);

    if (features2.features.dualSrcBlend == VK_FALSE) {
        GTEST_SKIP() << "dualSrcBlend feature is not available";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    char const *fsSource = R"glsl(
        #version 450
        layout(location = 0, index = 0) out vec4 c1;
        layout(location = 0, index = 1) out vec4 c2;
        void main(){
            c1 = vec4(0.5f);
            c2 = vec4(0.5f);
        }
    )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineColorBlendAttachmentState cb_attachments = {};
    cb_attachments.blendEnable = VK_TRUE;
    cb_attachments.srcColorBlendFactor = VK_BLEND_FACTOR_SRC1_COLOR;  // bad!
    cb_attachments.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    cb_attachments.colorBlendOp = VK_BLEND_OP_ADD;
    cb_attachments.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    cb_attachments.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    cb_attachments.alphaBlendOp = VK_BLEND_OP_ADD;

    CreatePipelineHelper pipe(*this);
    pipe.cb_attachments_[0] = cb_attachments;
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

// TODO: CTS was written, but still fails on many older drivers in CI
// https://gitlab.khronos.org/Tracker/vk-gl-cts/-/issues/3736
TEST_F(PositivePipeline, DISABLED_CreationFeedbackCount0) {
    TEST_DESCRIPTION("Test graphics pipeline feedback stage count check with 0.");

    AddRequiredExtensions(VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME);
    // need for IsDriver check
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    // This test hits a bug in the driver, CTS was written, but incase using an old driver
    if (IsDriver(VK_DRIVER_ID_NVIDIA_PROPRIETARY)) {
        GTEST_SKIP() << "This test should not be run on the NVIDIA proprietary driver.";
    }
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    VkPipelineCreationFeedbackCreateInfoEXT feedback_info = vku::InitStructHelper();
    VkPipelineCreationFeedbackEXT feedbacks[1] = {};
    // Set flags to known value that the driver has to overwrite
    feedbacks[0].flags = VK_PIPELINE_CREATION_FEEDBACK_FLAG_BITS_MAX_ENUM;

    feedback_info.pPipelineCreationFeedback = &feedbacks[0];
    feedback_info.pipelineStageCreationFeedbackCount = 0;

    auto set_feedback = [&feedback_info](CreatePipelineHelper &helper) { helper.gp_ci_.pNext = &feedback_info; };

    CreatePipelineHelper::OneshotTest(*this, set_feedback, kErrorBit);
}

TEST_F(PositivePipeline, ShaderModuleIdentifier) {
    TEST_DESCRIPTION("Create a pipeline using a shader module identifier");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_SHADER_MODULE_IDENTIFIER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDevicePipelineCreationCacheControlFeatures shader_cache_control_features = vku::InitStructHelper();
    VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT shader_module_id_features =
        vku::InitStructHelper(&shader_cache_control_features);
    auto features2 = GetPhysicalDeviceFeatures2(shader_module_id_features);

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();
    VkPipelineShaderStageModuleIdentifierCreateInfoEXT sm_id_create_info = vku::InitStructHelper();
    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);

    VkShaderModuleIdentifierEXT get_identifier = vku::InitStructHelper();
    vk::GetShaderModuleIdentifierEXT(device(), vs.handle(), &get_identifier);
    sm_id_create_info.identifierSize = get_identifier.identifierSize;
    sm_id_create_info.pIdentifier = get_identifier.identifier;

    VkPipelineShaderStageCreateInfo stage_ci = vku::InitStructHelper(&sm_id_create_info);
    stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";

    CreatePipelineHelper pipe(*this);
    pipe.gp_ci_.stageCount = 1;
    pipe.gp_ci_.pStages = &stage_ci;
    pipe.gp_ci_.flags = VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT;
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositivePipeline, ViewportSwizzleNV) {
    AddRequiredExtensions(VK_NV_VIEWPORT_SWIZZLE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    std::array<VkViewportSwizzleNV, 2> swizzle = {};

    swizzle.fill({VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_X_NV, VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Y_NV,
                  VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_Z_NV, VK_VIEWPORT_COORDINATE_SWIZZLE_POSITIVE_W_NV});

    std::array<VkViewport, 2> viewports = {};
    std::array<VkRect2D, 2> scissors = {};

    viewports.fill({0, 0, 16, 16, 0, 1});
    scissors.fill({{0, 0}, {16, 16}});

    // Test case where VkPipelineViewportSwizzleStateCreateInfoNV::viewportCount is EQUAL TO viewportCount set in
    // VkPipelineViewportStateCreateInfo
    {
        VkPipelineViewportSwizzleStateCreateInfoNV vp_swizzle_state = vku::InitStructHelper();
        vp_swizzle_state.viewportCount = size32(viewports);
        vp_swizzle_state.pViewportSwizzles = swizzle.data();

        auto break_vp_count = [&vp_swizzle_state, &viewports, &scissors](CreatePipelineHelper &helper) {
            helper.vp_state_ci_.viewportCount = size32(viewports);
            helper.vp_state_ci_.pViewports = viewports.data();
            helper.vp_state_ci_.scissorCount = size32(scissors);
            helper.vp_state_ci_.pScissors = scissors.data();
            helper.vp_state_ci_.pNext = &vp_swizzle_state;
            ASSERT_TRUE(vp_swizzle_state.viewportCount == helper.vp_state_ci_.viewportCount);
        };

        CreatePipelineHelper::OneshotTest(*this, break_vp_count, kErrorBit);
    }

    // Test case where VkPipelineViewportSwizzleStateCreateInfoNV::viewportCount is GREATER THAN viewportCount set in
    // VkPipelineViewportStateCreateInfo
    {
        VkPipelineViewportSwizzleStateCreateInfoNV vp_swizzle_state = vku::InitStructHelper();
        vp_swizzle_state.viewportCount = size32(viewports);
        vp_swizzle_state.pViewportSwizzles = swizzle.data();

        auto break_vp_count = [&vp_swizzle_state, &viewports, &scissors](CreatePipelineHelper &helper) {
            helper.vp_state_ci_.viewportCount = 1;
            helper.vp_state_ci_.pViewports = viewports.data();
            helper.vp_state_ci_.scissorCount = 1;
            helper.vp_state_ci_.pScissors = scissors.data();
            helper.vp_state_ci_.pNext = &vp_swizzle_state;
            ASSERT_TRUE(vp_swizzle_state.viewportCount > helper.vp_state_ci_.viewportCount);
        };

        CreatePipelineHelper::OneshotTest(*this, break_vp_count, kErrorBit);
    }
}

TEST_F(PositivePipeline, RasterStateWithDepthBiasRepresentationInfo) {
    TEST_DESCRIPTION("VkDepthBiasRepresentationInfoEXT in VkPipelineRasterizationStateCreateInfo pNext chain");

    AddRequiredExtensions(VK_EXT_DEPTH_BIAS_CONTROL_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceDepthBiasControlFeaturesEXT depth_bias_control_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(depth_bias_control_features);
    features2.features.depthBiasClamp =
        VK_FALSE;  // Make sure validation of VkDepthBiasRepresentationInfoEXT in VkPipelineRasterizationStateCreateInfo does not
                   // rely on depthBiasClamp being enabled

    if (!depth_bias_control_features.depthBiasControl) {
        GTEST_SKIP() << "depthBiasControl not supported";
    }

    if (!depth_bias_control_features.leastRepresentableValueForceUnormRepresentation) {
        GTEST_SKIP() << "leastRepresentableValueForceUnormRepresentation not supported";
    }
    if (!depth_bias_control_features.floatRepresentation) {
        GTEST_SKIP() << "floatRepresentation not supported";
    }
    if (!depth_bias_control_features.depthBiasExact) {
        GTEST_SKIP() << "depthBiasExact not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    const auto create_pipe_with_depth_bias_representation = [this](VkDepthBiasRepresentationInfoEXT &depth_bias_representation) {
        CreatePipelineHelper pipe(*this);
        pipe.InitState();
        pipe.AddDynamicState(VK_DYNAMIC_STATE_DEPTH_BIAS);
        pipe.rs_state_ci_.pNext = &depth_bias_representation;
        pipe.CreateGraphicsPipeline();
    };

    VkDepthBiasRepresentationInfoEXT depth_bias_representation = vku::InitStructHelper();
    depth_bias_representation.depthBiasRepresentation = VK_DEPTH_BIAS_REPRESENTATION_LEAST_REPRESENTABLE_VALUE_FORCE_UNORM_EXT;
    depth_bias_representation.depthBiasExact = VK_TRUE;
    create_pipe_with_depth_bias_representation(depth_bias_representation);

    depth_bias_representation.depthBiasRepresentation = VK_DEPTH_BIAS_REPRESENTATION_FLOAT_EXT;
    create_pipe_with_depth_bias_representation(depth_bias_representation);
}
