/* Copyright (c) 2023-2025 The Khronos Group Inc.
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "gpu_av_helper.h"

// If on Mesa, also suggest using MESA_SHADER_CACHE_DISABLE=1
class StressGpuAV : public VkLayerTest {
  public:
    void InitGpuVUDescriptorIndexing(bool safe_mode = false);
    void InitGpuAvFramework(std::vector<VkLayerSettingEXT> layer_settings = {}, bool safe_mode = true);
};

static const std::array gpu_av_enables = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
                                          VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT};
static const std::array gpu_av_disables = {VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT,
                                           VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};

void StressGpuAV::InitGpuAvFramework(std::vector<VkLayerSettingEXT> layer_settings, bool safe_mode) {
    SetTargetApiVersion(VK_API_VERSION_1_1);

    // We have defaulted GPU-AV to use unsafe mode, but all negative tests need to be "safe" or they will crash
    if (safe_mode) {
        layer_settings.emplace_back(
            VkLayerSettingEXT{OBJECT_LAYER_NAME, "gpuav_safe_mode", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue});
    }

    VkLayerSettingsCreateInfoEXT layer_setting_ci = vku::InitStructHelper();
    layer_setting_ci.settingCount = layer_settings.size();
    layer_setting_ci.pSettings = layer_settings.data();

    AddRequiredExtensions(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
    VkValidationFeaturesEXT validation_features = vku::InitStructHelper();
    validation_features.enabledValidationFeatureCount = size32(gpu_av_enables);
    validation_features.pEnabledValidationFeatures = gpu_av_enables.data();
    if (m_gpuav_disable_core) {
        validation_features.disabledValidationFeatureCount = size32(gpu_av_disables);
        validation_features.pDisabledValidationFeatures = gpu_av_disables.data();
    }

    validation_features.pNext = &layer_setting_ci;
    RETURN_IF_SKIP(InitFramework(&validation_features));
    if (!CanEnableGpuAV(*this)) {
        GTEST_SKIP() << "Requirements for GPU-AV are not met";
    }
}

void StressGpuAV::InitGpuVUDescriptorIndexing(bool safe_mode) {
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuAvFramework({}, safe_mode));
    AddRequiredFeature(vkt::Feature::maintenance4);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    AddRequiredFeature(vkt::Feature::descriptorBindingSampledImageUpdateAfterBind);
    AddRequiredFeature(vkt::Feature::descriptorBindingPartiallyBound);
    AddRequiredFeature(vkt::Feature::descriptorBindingVariableDescriptorCount);
    AddRequiredFeature(vkt::Feature::shaderSampledImageArrayNonUniformIndexing);
    AddRequiredFeature(vkt::Feature::shaderStorageBufferArrayNonUniformIndexing);

    RETURN_IF_SKIP(InitState());
}

TEST_F(StressGpuAV, DescriptorIndexing) {
    TEST_DESCRIPTION("Do many indexing into the shader");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    // send index to select in image array
    uint32_t *buffer_ptr = (uint32_t *)buffer.Memory().Map();
    buffer_ptr[0] = 0;  // index

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 64, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Image image(*m_device, 16, 16, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, sizeof(uint32_t), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 2);
    descriptor_set.UpdateDescriptorSets();

    const char *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 0) buffer Data {
            uint index;
        } data;

        layout(set = 0, binding = 1) uniform sampler2D tex[];

        vec4 abc(uint index) {
            return texture(tex[index], vec2(1.0, 1.0));
        }

        vec4 bar(uint index) {
           vec4 result = vec4(1.0);
           result -= texture(tex[index], vec2(0.1, 5.0));
           result -= texture(tex[index], vec2(0.2, 5.0));
           result -= texture(tex[index], vec2(0.3, 5.0));
           result -= texture(tex[index], vec2(0.4, 5.0));
           result -= texture(tex[index], vec2(0.5, 5.0));
           result -= texture(tex[index], vec2(0.6, 5.0));
           result -= texture(tex[index], vec2(0.7, 5.0));
           result -= texture(tex[index], vec2(0.8, 5.0));
           result -= texture(tex[index], vec2(0.9, 5.0));
           result -= abc(index);
           return result;
        }

        vec4 foo(uint index) {
           vec4 result = vec4(0.0);
           result += texture(tex[index], vec2(0.1, 2.0));
           result += texture(tex[index], vec2(0.2, 2.0));
           result += texture(tex[index], vec2(0.3, 2.0));
           result += texture(tex[index], vec2(0.4, 2.0));
           result += texture(tex[index], vec2(0.5, 2.0));
           result += texture(tex[index], vec2(0.6, 2.0));
           result += texture(tex[index], vec2(0.7, 2.0));
           result += texture(tex[index], vec2(0.8, 2.0));
           result += texture(tex[index], vec2(0.9, 2.0));
           result += abc(index);
           return result;
        }

        void main() {
           vec4 result = vec4(0.0);
           result += texture(tex[data.index], vec2(0, 0));
           result += texture(tex[data.index], vec2(0.1, 0));
           result += texture(tex[data.index], vec2(0.2, 0));
           result += texture(tex[data.index], vec2(0.3, 0));
           result += texture(tex[data.index], vec2(0.4, 0));
           result += texture(tex[data.index], vec2(0.5, 0));
           result += texture(tex[data.index], vec2(0.6, 0));
           result += texture(tex[data.index], vec2(0.7, 0));
           result += texture(tex[data.index], vec2(0.8, 0));
           result += texture(tex[data.index], vec2(0.9, 0));
           result += texture(tex[data.index], vec2(0, 0.1));

           result += foo(data.index);
           result += bar(data.index);
           result += foo(data.index + 1);
           result += bar(data.index + 1);
           result += foo(data.index + 2);
           result += bar(data.index + 2);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(StressGpuAV, DescriptorIndexing2) {
    TEST_DESCRIPTION("Do many indexing into the shader");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());

    // Will look like
    //     layout(set = 0, binding = 0) buffer SSBO {
    //         float a0;
    //         uint b0, b1, b2, ... bn;
    //     } x[2];
    //     void main() {
    //         float a = x[1].a0;
    //         x[1].b0 = floatBitsToUint(a * 0);
    //         x[1].b0 = floatBitsToUint(a * 1);
    //         x[1].b0 = floatBitsToUint(a * 2);
    //         // ...
    //         x[1].bn = floatBitsToUint(a * n);
    //     }
    const uint32_t field_count = 100;
    std::stringstream cs_source;
    cs_source << R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer SSBO {
            float a0;
            uint )glsl";
    for (uint32_t i = 0; i < field_count; i++) {
        cs_source << "b" << i << ", ";
    }
    cs_source << R"glsl(bn;
        } x[2];
        void main() {
            float a = x[1].a0;
    )glsl";

    for (uint32_t i = 0; i < field_count; i++) {
        cs_source << "x[1].b" << i << " = floatBitsToUint(a * " << i << ".0);\n";
    }
    cs_source << "\n}";

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorIndexingSet descriptor_set(m_device, {
                                                             {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr,
                                                              VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
                                                         });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(this, cs_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(StressGpuAV, DescriptorIndexingPushConstantAccess) {
    TEST_DESCRIPTION("Test DescriptroIndexPushConstantAccess optimization");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    VkPushConstantRange pc_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, 64};
    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 64, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_}, {pc_range});

    const uint32_t count = 150;
    std::stringstream cs_source;
    cs_source << R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference2 : require
        #extension GL_EXT_buffer_reference_uvec2 : require
        #extension GL_EXT_nonuniform_qualifier : require

        layout(set = 0, binding = 0, std430) buffer SSBO {
            uint _m0[];
        } data[];


        layout(push_constant, std430) uniform PC {
            uint a;
            uint b;
        };

        void main() {
            // data[b]._m0[a + 0] = 0;
            // data[b]._m0[a + 1] = 1;
            // data[b]._m0[a + 2] = 2;
            // ...
    )glsl";

    for (uint32_t i = 0; i < count; i++) {
        cs_source << "data[b + 1]._m0[" << i << "] = " << i << ";\n";
    }
    cs_source << "\n}";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(this, cs_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();
}

TEST_F(StressGpuAV, DescriptorIndexingLoop) {
    TEST_DESCRIPTION("Show the GPU overhead of doing redundant checks inside a loop");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());

    const char *cs_source = R"glsl(
        #version 450
        layout (local_size_x = 256) in;
        layout(set = 0, binding = 0) buffer Data {
            uint a;
            uint b;
            uint c;
            uint result;
        } buffers[2];

        void main() {
            uint x = 0;
            // Only does 3 instrumentations in the loop
            // But if not getting hoisted out, can take seconds to execute on the GPU
            for (int i = 0; i < 32768; i++) {
                x += buffers[0].a;
                x *= buffers[0].b;
                x -= buffers[0].c;
            }
            buffers[1].result = x;
        }
    )glsl";

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    OneOffDescriptorIndexingSet descriptor_set(m_device, {
                                                             {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr,
                                                              VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
                                                         });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer, 32, 32, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(StressGpuAV, DescriptorIndexingGeneralBufferOOB) {
    TEST_DESCRIPTION("Touching every part of a SSBO");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    // Will look like
    // layout(set = 0, binding = 0) buffer SSBO {
    //     float a;
    //     vec4 m[32];
    // };
    // void main() {
    //     vec4 b = vec4(1.0, 2.0, 1.0, 2.0);
    //     // Note - This is generated as 4 seperate OpLoads here
    //     a += dot(vec4(m[0].x, m[0].y, m[0].z, m[0].w), b);
    //     a += dot(vec4(m[31].x, m[31].y, m[31].z, m[31].w), b);
    // }
    const uint32_t array_count = 32;
    std::stringstream cs_source;
    cs_source << R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer SSBO {
            float a;
            vec4 m[)glsl";
    cs_source << array_count << "];\n};\n";
    cs_source << R"glsl(
        void main() {
            vec4 b = vec4(1.0, 2.0, 1.0, 2.0);
    )glsl";

    for (uint32_t i = 0; i < array_count; i++) {
        cs_source << "a += dot(vec4(m[" << i << "].x, m[" << i << "].y, m[" << i << "].z, m[" << i << "].w), b);\n";
    }
    cs_source << "\n}";

    vkt::Buffer buffer(*m_device, 4096, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(this, cs_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(StressGpuAV, BufferDeviceAddress) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    // About a 5x speed up to run in unsafe mode
    RETURN_IF_SKIP(InitGpuAvFramework({}, false));
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();
    const uint32_t count = 32;

    std::stringstream cs_source;
    cs_source << R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable

        layout(buffer_reference) buffer BDA {
            vec3 x;
            vec3 payload[4096];
        };

        layout(push_constant) uniform Uniforms {
            BDA ptr;
        };

        void main() {
            vec3 a = vec3(0);
            // a += fma(vec3(ptr.payload[0].x, ptr.payload[0].y, ptr.payload[0].z),
            //          vec3(ptr.payload[1].x, ptr.payload[1].y, ptr.payload[1].z),
            //          vec3(ptr.payload[2].x, ptr.payload[2].y, ptr.payload[2].z));
            //
            // .... many times
            //
            // ptr.x = a;
    )glsl";

    for (uint32_t i = 0; i < count; i += 3) {
        cs_source << "a += fma(vec3(ptr.payload[" << i << "].x, ptr.payload[" << i << "].y, ptr.payload[" << i << "].z), ";
        cs_source << "vec3(ptr.payload[" << i + 1 << "].x, ptr.payload[" << i + 1 << "].y, ptr.payload[" << i + 1 << "].z), ";
        cs_source << "vec3(ptr.payload[" << i + 2 << "].x, ptr.payload[" << i + 2 << "].y, ptr.payload[" << i + 2 << "].z));\n";
    }
    cs_source << "\nptr.x = a;\n}";

    VkPushConstantRange pc_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(VkDeviceAddress)};
    const vkt::PipelineLayout pipeline_layout(*m_device, {}, {pc_range});

    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.cs_ = VkShaderObj(this, cs_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();
}