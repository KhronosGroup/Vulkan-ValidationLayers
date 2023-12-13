/* Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
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

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/gpu_av_helper.h"
#include "../../layers/gpu_shaders/gpu_shaders_constants.h"

void GpuAVDescriptorIndexingTest::InitGpuVUDescriptorIndexing(void *p_next) {
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuAvFramework());

    auto maintenance4_features = vku::InitStruct<VkPhysicalDeviceMaintenance4Features>(p_next);
    VkPhysicalDeviceVulkan12Features features_12 = vku::InitStructHelper(&maintenance4_features);
    VkPhysicalDeviceFeatures2 features2 = GetPhysicalDeviceFeatures2(features_12);
    if (!features2.features.shaderInt64) {
        GTEST_SKIP() << "shaderInt64 is not supported";
    }
    if (!features_12.runtimeDescriptorArray || !features_12.descriptorBindingSampledImageUpdateAfterBind ||
        !features_12.descriptorBindingPartiallyBound || !features_12.descriptorBindingVariableDescriptorCount ||
        !features_12.shaderSampledImageArrayNonUniformIndexing || !features_12.shaderStorageBufferArrayNonUniformIndexing) {
        GTEST_SKIP() << "Not all descriptor indexing features supported, skipping descriptor indexing tests";
    }

    features2.features.robustBufferAccess = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &features2));
}

TEST_F(PositiveGpuAVDescriptorIndexing, UnInitImage) {
    TEST_DESCRIPTION("Make sure there's not a crash if the sampler of a combined image sampler is initialized by the image isn't.");
    RETURN_IF_SKIP(InitGpuVUDescriptorIndexing());
    InitRenderTarget();

    const uint32_t buffer_size = 1024;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    vkt::Buffer buffer0(*m_device, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);

    VkDescriptorBindingFlags ds_binding_flags[3] = {
        0, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT};

    VkDescriptorSetLayoutBindingFlagsCreateInfo layout_createinfo_binding_flags = vku::InitStructHelper();
    layout_createinfo_binding_flags.bindingCount = 3;
    layout_createinfo_binding_flags.pBindingFlags = ds_binding_flags;

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    const VkSampler samplers[2] = {sampler.handle(), sampler.handle()};

    OneOffDescriptorSet descriptor_set_variable(
        m_device,
        {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
            {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 5, VK_SHADER_STAGE_ALL, nullptr},
            {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, samplers},
        },
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, &layout_createinfo_binding_flags,
        VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT);

    const vkt::PipelineLayout pipeline_layout_variable(*m_device, {&descriptor_set_variable.layout_});
    VkImageObj image(m_device);
    image.Init(16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_LINEAR);
    vkt::ImageView image_view = image.CreateView();

    descriptor_set_variable.WriteDescriptorBufferInfo(0, buffer0.handle(), 0, sizeof(uint32_t));
    descriptor_set_variable.WriteDescriptorImageInfo(2, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set_variable.UpdateDescriptorSets();

    // - The vertex shader fetches the invalid index from the uniform buffer and passes it to the fragment shader.
    // - The fragment shader makes the invalid array access.
    char const *vs_source = R"glsl(
        #version 450

        layout(std140, binding = 0) uniform foo { uint tex_index[1]; } uniform_index_buffer;
        layout(location = 0) out flat uint index;
        vec2 vertices[3];
        void main(){
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           index = uniform_index_buffer.tex_index[0];
        }
    )glsl";
    VkShaderObj vs(this, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 2) uniform sampler2D tex[];
        layout(location = 0) out vec4 uFragColor;
        layout(location = 0) in flat uint index;
        void main(){
           uFragColor = texture(tex[index], vec2(0, 0));
        }
    )glsl";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout_variable.handle();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_variable.handle(), 0, 1,
                              &descriptor_set_variable.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();

    uint32_t *data = (uint32_t *)buffer0.memory().map();
    data[0] = 1;
    buffer0.memory().unmap();

    m_default_queue->submit(*m_commandBuffer);
    m_default_queue->wait();
}