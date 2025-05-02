/*
 * Copyright (c) 2023-2024 Nintendo
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/shader_object_helper.h"
#include "../framework/pipeline_helper.h"

class PositiveGpuAVShaderObject : public GpuAVTest {
  public:
    void InitBasicShaderObject() {
        SetTargetApiVersion(VK_API_VERSION_1_2);
        AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
        AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        AddRequiredFeature(vkt::Feature::dynamicRendering);
        AddRequiredFeature(vkt::Feature::shaderObject);
    }
};

TEST_F(PositiveGpuAVShaderObject, RestoreUserPushConstants) {
    TEST_DESCRIPTION("Test that user supplied push constants are correctly restored. One graphics pipeline, indirect draw.");
    InitBasicShaderObject();
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitDynamicRenderTarget();

    vkt::Buffer indirect_draw_parameters_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                                kHostVisibleMemProps);
    auto &indirect_draw_parameters = *static_cast<VkDrawIndirectCommand *>(indirect_draw_parameters_buffer.Memory().Map());
    indirect_draw_parameters.vertexCount = 3;
    indirect_draw_parameters.instanceCount = 1;
    indirect_draw_parameters.firstVertex = 0;
    indirect_draw_parameters.firstInstance = 0;

    constexpr int32_t int_count = 16;
    vkt::Buffer storage_buffer(*m_device, int_count * sizeof(int32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    // Use different push constant ranges for vertex and fragment shader.
    // The underlying storage buffer is the same.
    // Vertex shader will fill the first 8 integers, fragment shader the other 8
    struct PushConstants {
        // Vertex shader
        VkDeviceAddress storage_buffer_ptr_1;
        int32_t integers_1[int_count / 2];
        // Fragment shader
        VkDeviceAddress storage_buffer_ptr_2;
        int32_t integers_2[int_count / 2];
    } push_constants;

    push_constants.storage_buffer_ptr_1 = storage_buffer.Address();
    push_constants.storage_buffer_ptr_2 = storage_buffer.Address() + sizeof(int32_t) * (int_count / 2);
    for (int32_t i = 0; i < int_count / 2; ++i) {
        push_constants.integers_1[i] = i;
        push_constants.integers_2[i] = (int_count / 2) + i;
    }

    constexpr uint32_t shader_pcr_byte_size = uint32_t(sizeof(VkDeviceAddress)) + uint32_t(sizeof(int32_t)) * (int_count / 2);
    std::array<VkPushConstantRange, 2> push_constant_ranges = {{
        {VK_SHADER_STAGE_VERTEX_BIT, 0, shader_pcr_byte_size},
        {VK_SHADER_STAGE_FRAGMENT_BIT, shader_pcr_byte_size, shader_pcr_byte_size},
    }};
    VkPipelineLayoutCreateInfo plci = vku::InitStructHelper();
    plci.pushConstantRangeCount = size32(push_constant_ranges);
    plci.pPushConstantRanges = push_constant_ranges.data();
    vkt::PipelineLayout pipeline_layout(*m_device, plci);

    char const *vs_source = R"glsl(
            #version 450
            #extension GL_EXT_buffer_reference : enable

            layout(buffer_reference, std430, buffer_reference_align = 16) buffer MyPtrType {
                int out_array[8];
            };

            layout(push_constant) uniform PushConstants {
                MyPtrType ptr;
                int in_array[8];
            } pc;

            vec2 vertices[3];

            void main() {
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
              gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);

              for (int i = 0; i < 8; ++i) {
                  pc.ptr.out_array[i] = pc.in_array[i];
              }
            }
        )glsl";

    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vs_source);

    char const *fs_source = R"glsl(
            #version 450
            #extension GL_EXT_buffer_reference : enable

            layout(buffer_reference, std430, buffer_reference_align = 16) buffer MyPtrType {
                int out_array[8];
            };

            layout(push_constant) uniform PushConstants {
                layout(offset = 40) MyPtrType ptr;
                int in_array[8];
            } pc;

            layout(location = 0) out vec4 uFragColor;

            void main() {
                for (int i = 0; i < 8; ++i) {
                      pc.ptr.out_array[i] = pc.in_array[i];
                }
            }
        )glsl";
    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fs_source);

    VkShaderCreateInfoEXT vs_ci = ShaderCreateInfo(vs_spv, VK_SHADER_STAGE_VERTEX_BIT, 0, nullptr,
                                                   static_cast<uint32_t>(push_constant_ranges.size()), push_constant_ranges.data());
    VkShaderCreateInfoEXT fs_ci = ShaderCreateInfo(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT, 0, nullptr,
                                                   static_cast<uint32_t>(push_constant_ranges.size()), push_constant_ranges.data());

    const vkt::Shader vs(*m_device, vs_ci);
    const vkt::Shader fs(*m_device, fs_ci);

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.Begin(&begin_info);
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindShaders(vs, fs);

    vk::CmdPushConstants(m_command_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, shader_pcr_byte_size, &push_constants);
    vk::CmdPushConstants(m_command_buffer, pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, shader_pcr_byte_size,
                         shader_pcr_byte_size, &push_constants.storage_buffer_ptr_2);
    // Make sure pushing the same push constants twice does not break internal management
    vk::CmdPushConstants(m_command_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, shader_pcr_byte_size, &push_constants);
    vk::CmdPushConstants(m_command_buffer, pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, shader_pcr_byte_size,
                         shader_pcr_byte_size, &push_constants.storage_buffer_ptr_2);
    // Vertex shader will write 8 values to storage buffer, fragment shader another 8
    vk::CmdDrawIndirect(m_command_buffer, indirect_draw_parameters_buffer.handle(), 0, 1, sizeof(VkDrawIndirectCommand));
    m_command_buffer.EndRendering();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    auto storage_buffer_ptr = static_cast<int32_t *>(storage_buffer.Memory().Map());
    for (int32_t i = 0; i < int_count; ++i) {
        ASSERT_EQ(storage_buffer_ptr[i], i);
    }
}

TEST_F(PositiveGpuAVShaderObject, RestoreUserPushConstants2) {
    TEST_DESCRIPTION(
        "Test that user supplied push constants are correctly restored. One graphics pipeline, one compute pipeline, indirect draw "
        "and dispatch.");
    InitBasicShaderObject();
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitDynamicRenderTarget();

    constexpr int32_t int_count = 8;

    struct PushConstants {
        VkDeviceAddress storage_buffer;
        int32_t integers[int_count];
    };

    // Graphics pipeline
    // ---

    char const *vs_source = R"glsl(
            #version 450
            #extension GL_EXT_buffer_reference : enable

            layout(buffer_reference, std430, buffer_reference_align = 16) buffer MyPtrType {
                int out_array[8];
            };

            layout(push_constant) uniform PushConstants {
                MyPtrType ptr;
                int in_array[8];
            } pc;

            vec2 vertices[3];

            void main() {
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
              gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);

              for (int i = 0; i < 4; ++i) {
                  pc.ptr.out_array[i] = pc.in_array[i];
              }
            }
        )glsl";

    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vs_source);

    char const *fs_source = R"glsl(
            #version 450
            #extension GL_EXT_buffer_reference : enable

            layout(buffer_reference, std430, buffer_reference_align = 16) buffer MyPtrType {
                int out_array[8];
            };

            layout(push_constant) uniform PushConstants {
                MyPtrType ptr;
                int in_array[8];
            } pc;

            layout(location = 0) out vec4 uFragColor;

            void main() {
                for (int i = 4; i < 8; ++i) {
                      pc.ptr.out_array[i] = pc.in_array[i];
                }
            }
        )glsl";

    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fs_source);

    VkPushConstantRange graphics_push_constant_ranges = {VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                                         sizeof(PushConstants)};
    VkPipelineLayoutCreateInfo graphics_plci = vku::InitStructHelper();
    graphics_plci.pushConstantRangeCount = 1;
    graphics_plci.pPushConstantRanges = &graphics_push_constant_ranges;
    vkt::PipelineLayout graphics_pipeline_layout(*m_device, graphics_plci);

    vkt::Buffer graphics_storage_buffer(*m_device, int_count * sizeof(int32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                        vkt::device_address);

    PushConstants graphics_push_constants;
    graphics_push_constants.storage_buffer = graphics_storage_buffer.Address();
    for (int32_t i = 0; i < int_count; ++i) {
        graphics_push_constants.integers[i] = i;
    }

    vkt::Buffer indirect_draw_parameters_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                                kHostVisibleMemProps);
    auto &indirect_draw_parameters = *static_cast<VkDrawIndirectCommand *>(indirect_draw_parameters_buffer.Memory().Map());
    indirect_draw_parameters.vertexCount = 3;
    indirect_draw_parameters.instanceCount = 1;
    indirect_draw_parameters.firstVertex = 0;
    indirect_draw_parameters.firstInstance = 0;

    VkShaderCreateInfoEXT vs_ci =
        ShaderCreateInfo(vs_spv, VK_SHADER_STAGE_VERTEX_BIT, 0, nullptr, 1, &graphics_push_constant_ranges);
    VkShaderCreateInfoEXT fs_ci =
        ShaderCreateInfo(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT, 0, nullptr, 1, &graphics_push_constant_ranges);

    const vkt::Shader vs(*m_device, vs_ci);
    const vkt::Shader fs(*m_device, fs_ci);

    // Compute pipeline
    // ---

    char const *cs_source = R"glsl(
            #version 450
            #extension GL_EXT_buffer_reference : enable

            layout(buffer_reference, std430, buffer_reference_align = 16) buffer MyPtrType {
                int out_array[8];
            };

            layout(push_constant) uniform PushConstants {
                MyPtrType ptr;
                int in_array[8];
            } pc;

            layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

            void main() {
              for (int i = 0; i < 8; ++i) {
                  pc.ptr.out_array[i] = pc.in_array[i];
              }
            }
        )glsl";

    const auto cs_spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, cs_source);

    VkPushConstantRange compute_push_constant_ranges = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants)};
    VkPipelineLayoutCreateInfo compute_plci = vku::InitStructHelper();
    compute_plci.pushConstantRangeCount = 1;
    compute_plci.pPushConstantRanges = &compute_push_constant_ranges;
    vkt::PipelineLayout compute_pipeline_layout(*m_device, compute_plci);

    vkt::Buffer compute_storage_buffer(*m_device, int_count * sizeof(int32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                       vkt::device_address);

    PushConstants compute_push_constants;
    compute_push_constants.storage_buffer = compute_storage_buffer.Address();
    for (int32_t i = 0; i < int_count; ++i) {
        compute_push_constants.integers[i] = int_count + i;
    }

    VkShaderCreateInfoEXT cs_ci =
        ShaderCreateInfo(cs_spv, VK_SHADER_STAGE_COMPUTE_BIT, 0, nullptr, 1, &compute_push_constant_ranges);

    const vkt::Shader cs(*m_device, cs_ci);

    vkt::Buffer indirect_dispatch_parameters_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                                    kHostVisibleMemProps);
    auto &indirect_dispatch_parameters =
        *static_cast<VkDispatchIndirectCommand *>(indirect_dispatch_parameters_buffer.Memory().Map());
    indirect_dispatch_parameters.x = 1;
    indirect_dispatch_parameters.y = 1;
    indirect_dispatch_parameters.z = 1;

    // Submit commands
    // ---

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();

    m_command_buffer.BindShaders(vs, fs);
    m_command_buffer.BindCompShader(cs);

    vk::CmdPushConstants(m_command_buffer, graphics_pipeline_layout.handle(),
                         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(graphics_push_constants),
                         &graphics_push_constants);

    // Vertex shader will write 4 values to graphics storage buffer, fragment shader another 4
    vk::CmdDrawIndirect(m_command_buffer, indirect_draw_parameters_buffer.handle(), 0, 1, sizeof(VkDrawIndirectCommand));
    m_command_buffer.EndRendering();

    vk::CmdPushConstants(m_command_buffer, compute_pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0,
                         sizeof(compute_push_constants), &compute_push_constants);
    // Compute shaders will write 8 values to compute storage buffer
    vk::CmdDispatchIndirect(m_command_buffer, indirect_dispatch_parameters_buffer.handle(), 0);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    auto compute_storage_buffer_ptr = static_cast<int32_t *>(compute_storage_buffer.Memory().Map());
    for (int32_t i = 0; i < int_count; ++i) {
        ASSERT_EQ(compute_storage_buffer_ptr[i], int_count + i);
    }

    auto graphics_storage_buffer_ptr = static_cast<int32_t *>(graphics_storage_buffer.Memory().Map());
    for (int32_t i = 0; i < int_count; ++i) {
        ASSERT_EQ(graphics_storage_buffer_ptr[i], i);
    }
}

TEST_F(PositiveGpuAVShaderObject, DispatchShaderObjectAndPipeline) {
    TEST_DESCRIPTION("GPU validation: Validate selection of which shaders get instrumented for GPU-AV");
    InitBasicShaderObject();

    AddRequiredFeature(vkt::Feature::robustBufferAccess);
    std::vector<VkLayerSettingEXT> layer_settings = {
        {OBJECT_LAYER_NAME, "gpuav_select_instrumented_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue}};
    RETURN_IF_SKIP(InitGpuAvFramework(layer_settings));
    RETURN_IF_SKIP(InitState());

    static const char comp_src[] = R"glsl(
        #version 450
        layout(local_size_x=16, local_size_x=1, local_size_x=1) in;

        void main() {
        }
    )glsl";

    const auto comp_spv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, comp_src);
    VkShaderCreateInfoEXT comp_create_info = ShaderCreateInfo(comp_spv, VK_SHADER_STAGE_COMPUTE_BIT);

    VkValidationFeatureEnableEXT enabled[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
    VkValidationFeaturesEXT features = vku::InitStructHelper();
    features.enabledValidationFeatureCount = 1;
    features.pEnabledValidationFeatures = enabled;
    comp_create_info.pNext = &features;

    const vkt::Shader compShader(*m_device, comp_create_info);

    CreateComputePipelineHelper compute_pipe(*this);
    compute_pipe.cs_ = std::make_unique<VkShaderObj>(this, comp_src, VK_SHADER_STAGE_COMPUTE_BIT);
    compute_pipe.CreateComputePipeline();

    vkt::Buffer indirect_dispatch_parameters_buffer(*m_device, sizeof(VkDispatchIndirectCommand),
                                                    VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, kHostVisibleMemProps);
    auto &indirect_dispatch_parameters =
        *static_cast<VkDispatchIndirectCommand *>(indirect_dispatch_parameters_buffer.Memory().Map());
    indirect_dispatch_parameters.x = 1u;
    indirect_dispatch_parameters.y = 1u;
    indirect_dispatch_parameters.z = 1u;

    m_command_buffer.Begin();
    SetDefaultDynamicStatesExclude();
    m_command_buffer.BindCompShader(compShader);
    vk::CmdDispatchIndirect(m_command_buffer, indirect_dispatch_parameters_buffer.handle(), 0u);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe.Handle());
    vk::CmdDispatchIndirect(m_command_buffer, indirect_dispatch_parameters_buffer.handle(), 0u);

    m_command_buffer.End();
}

TEST_F(PositiveGpuAVShaderObject, GetShaderBinaryDataSimple) {
    InitBasicShaderObject();
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    // no descriptor and nothing to instrument
    vkt::Shader frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, kFragmentMinimalGlsl);

    size_t binary_data_size;
    vk::GetShaderBinaryDataEXT(*m_device, frag_shader, &binary_data_size, nullptr);
    std::vector<uint8_t> frag_data(binary_data_size);
    vk::GetShaderBinaryDataEXT(*m_device, frag_shader, &binary_data_size, frag_data.data());

    vkt::Shader binary_frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, frag_data);
}

TEST_F(PositiveGpuAVShaderObject, GetShaderBinaryData) {
    InitBasicShaderObject();
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    static const char frag_src[] = R"glsl(
        #version 450
        layout(location = 0) in highp vec4 vtxColor;
        layout(location = 0) out highp vec4 fragColor;
        layout(set = 0, binding = 0) uniform Block { vec4 color; };
        void main (void) {
            fragColor = vtxColor + color;
        }
    )glsl";

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    vkt::Shader frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, frag_src, &descriptor_set.layout_.handle());

    size_t binary_data_size;
    vk::GetShaderBinaryDataEXT(*m_device, frag_shader, &binary_data_size, nullptr);
    std::vector<uint8_t> frag_data(binary_data_size);
    vk::GetShaderBinaryDataEXT(*m_device, frag_shader, &binary_data_size, frag_data.data());

    vkt::Shader binary_frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, frag_data, &descriptor_set.layout_.handle());
}

TEST_F(PositiveGpuAVShaderObject, BinaryShaderObjects) {
    InitBasicShaderObject();
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitDynamicRenderTarget();

    static const char vert_src[] = R"glsl(
        #version 450
        layout(location = 0) out highp vec4 vtxColor;
        layout(set = 0, binding = 0) uniform Block { vec4 color; };
        void main() {
            gl_Position = vec4(1.0f);
            vtxColor = color;
        }
    )glsl";

    static const char frag_src[] = R"glsl(
        #version 450
        layout(location = 0) in highp vec4 vtxColor;
        layout(location = 0) out highp vec4 fragColor;
        layout(set = 0, binding = 0) uniform Block { vec4 color; };

        void main (void) {
            fragColor = vtxColor + color;
        }
    )glsl";

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer buffer(*m_device, 16u, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE);
    descriptor_set.UpdateDescriptorSets();

    vkt::Shader vert_shader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, vert_src, &descriptor_set.layout_.handle());
    vkt::Shader frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, frag_src, &descriptor_set.layout_.handle());

    size_t binary_data_size;
    vk::GetShaderBinaryDataEXT(*m_device, vert_shader, &binary_data_size, nullptr);
    std::vector<uint8_t> vert_data(binary_data_size);
    vk::GetShaderBinaryDataEXT(*m_device, vert_shader, &binary_data_size, vert_data.data());

    vk::GetShaderBinaryDataEXT(*m_device, frag_shader, &binary_data_size, nullptr);
    std::vector<uint8_t> frag_data(binary_data_size);
    vk::GetShaderBinaryDataEXT(*m_device, frag_shader, &binary_data_size, frag_data.data());

    vkt::Shader binary_vert_shader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, vert_data, &descriptor_set.layout_.handle());
    vkt::Shader binary_frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, frag_data, &descriptor_set.layout_.handle());

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    SetDefaultDynamicStatesExclude();

    VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    VkShaderEXT shaders[] = {binary_vert_shader, binary_frag_shader};
    vk::CmdBindShadersEXT(m_command_buffer, 2u, stages, shaders);

    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0u, 1u, &descriptor_set.set_, 0u,
                              nullptr);
    vk::CmdDraw(m_command_buffer, 3u, 1u, 0u, 0u);
    m_command_buffer.EndRendering();
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}
