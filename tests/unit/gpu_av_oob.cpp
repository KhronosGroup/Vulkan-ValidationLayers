/*
 * Copyright (c) 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2020-2024 Valve Corporation
 * Copyright (c) 2020-2024 LunarG, Inc.
 * Copyright (c) 2020-2024 Google, Inc.
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
#include "../framework/gpu_av_helper.h"

TEST_F(NegativeGpuAVOOB, RobustBuffer) {
    TEST_DESCRIPTION("Check buffer oob validation when per pipeline robustness is enabled");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    VkPhysicalDevicePipelineRobustnessFeaturesEXT pipeline_robustness_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(pipeline_robustness_features);
    features2.features.robustBufferAccess = VK_FALSE;
    if (!pipeline_robustness_features.pipelineRobustness) {
        GTEST_SKIP() << "pipelineRobustness feature not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();
    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer uniform_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, reqs);
    vkt::Buffer storage_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, reqs);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, uniform_buffer.handle(), 0, 4);
    descriptor_set.WriteDescriptorBufferInfo(1, storage_buffer.handle(), 0, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const char *vertshader = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform foo { uint index[]; } u_index;
        layout(set = 0, binding = 1) buffer StorageBuffer { uint data[]; } Data;

        void main() {
            vec4 x;
            if (u_index.index[0] == 0)
                x[0] = u_index.index[1]; // Uniform read OOB
            else
                Data.data[8] = 0xdeadca71; // Storage write OOB
        }
    )glsl";

    VkShaderObj vs(this, vertshader, VK_SHADER_STAGE_VERTEX_BIT);

    VkPipelineRobustnessCreateInfoEXT pipeline_robustness_ci = vku::InitStructHelper();
    pipeline_robustness_ci.uniformBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT;
    pipeline_robustness_ci.storageBuffers = VK_PIPELINE_ROBUSTNESS_BUFFER_BEHAVIOR_ROBUST_BUFFER_ACCESS_EXT;

    CreatePipelineHelper robust_pipe(*this);
    robust_pipe.InitState();
    robust_pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    robust_pipe.gp_ci_.layout = pipeline_layout.handle();
    robust_pipe.gp_ci_.pNext = &pipeline_robustness_ci;
    robust_pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_commandBuffer->begin(&begin_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, robust_pipe.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    uint32_t *data = (uint32_t *)uniform_buffer.memory().map();
    *data = 0;
    uniform_buffer.memory().unmap();
    // normally VUID-vkCmdDraw-None-08612
    m_errorMonitor->SetDesiredFailureMsg(
        kWarningBit, "Descriptor index 0 access out of bounds. Descriptor size is 4 and highest byte accessed was 19");
    m_commandBuffer->QueueCommandBuffer();
    m_errorMonitor->VerifyFound();
    data = (uint32_t *)uniform_buffer.memory().map();
    *data = 1;
    uniform_buffer.memory().unmap();
    // normally VUID-vkCmdDraw-None-08613
    m_errorMonitor->SetDesiredFailureMsg(
        kWarningBit, "Descriptor index 0 access out of bounds. Descriptor size is 16 and highest byte accessed was 35");
    m_commandBuffer->QueueCommandBuffer();
    m_errorMonitor->VerifyFound();
}

// TODO - https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6980
TEST_F(NegativeGpuAVOOB, DISABLED_Basic) {
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    VkPhysicalDeviceMultiDrawFeaturesEXT multi_draw_features = vku::InitStructHelper();
    const bool multi_draw = IsExtensionsEnabled(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    auto robustness2_features =
        vku::InitStruct<VkPhysicalDeviceRobustness2FeaturesEXT>(multi_draw ? &multi_draw_features : nullptr);
    auto features2 = GetPhysicalDeviceFeatures2(robustness2_features);
    if (!robustness2_features.nullDescriptor) {
        GTEST_SKIP() << "nullDescriptor feature not supported";
    }
    features2.features.robustBufferAccess = VK_FALSE;
    robustness2_features.robustBufferAccess2 = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer offset_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, reqs);
    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, reqs);

    vkt::Buffer uniform_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, reqs);
    vkt::Buffer storage_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, reqs);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = uniform_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView uniform_buffer_view(*m_device, bvci);
    bvci.buffer = storage_texel_buffer.handle();
    vkt::BufferView storage_buffer_view(*m_device, bvci);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {3, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {4, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, offset_buffer.handle(), 0, 4);
    descriptor_set.WriteDescriptorBufferInfo(1, write_buffer.handle(), 0, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(2, VK_NULL_HANDLE, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferView(3, uniform_buffer_view.handle(), VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
    descriptor_set.WriteDescriptorBufferView(4, storage_buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    static const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform ufoo { uint index[]; } u_index;      // index[1]
        layout(set = 0, binding = 1) buffer StorageBuffer { uint data[]; } Data;  // data[4]
        layout(set = 0, binding = 2) buffer NullBuffer { uint data[]; } Null;     // VK_NULL_HANDLE
        layout(set = 0, binding = 3) uniform samplerBuffer u_buffer;              // texel_buffer[4]
        layout(set = 0, binding = 4, r32f) uniform imageBuffer s_buffer;          // texel_buffer[4]
        void main() {
            vec4 x;
            if (u_index.index[0] == 8)
                Data.data[u_index.index[0]] = 0xdeadca71;
            else if (u_index.index[0] == 0)
                Data.data[0] = u_index.index[4];
            else if (u_index.index[0] == 1)
                Data.data[0] = Null.data[40];  // No error
            else if (u_index.index[0] == 2)
                x = texelFetch(u_buffer, 5);
            else if (u_index.index[0] == 3)
                x = imageLoad(s_buffer, 5);
            else if (u_index.index[0] == 4)
                imageStore(s_buffer, 5, x);
            else if (u_index.index[0] == 5)  // No Error
                imageStore(s_buffer, 0, x);
            else if (u_index.index[0] == 6)  // No Error
                x = imageLoad(s_buffer, 0);
        }
        )glsl";

    VkShaderObj vs(this, vertshader, VK_SHADER_STAGE_VERTEX_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_commandBuffer->begin(&begin_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    struct TestCase {
        bool positive;
        uint32_t index;
        char const *expected_error;
    };
    std::vector<TestCase> tests;
    // "VUID-vkCmdDispatchBase-None-08613" Storage
    tests.push_back({false, 8, "Descriptor size is 16 and highest byte accessed was 35"});
    // Uniform buffer stride rounded up to the alignment of a vec4 (16 bytes)
    // so u_index.index[4] accesses bytes 64, 65, 66, and 67
    // "VUID-vkCmdDispatchBase-None-08612" Uniform
    tests.push_back({false, 0, "Descriptor size is 4 and highest byte accessed was 67"});
    tests.push_back({true, 1, ""});
    // "VUID-vkCmdDispatchBase-None-08612" Uniform
    tests.push_back({false, 2, "Descriptor size is 4 texels and highest texel accessed was 5"});
    // "VUID-vkCmdDispatchBase-None-08613" Storage
    tests.push_back({false, 3, "Descriptor size is 4 texels and highest texel accessed was 5"});
    // "VUID-vkCmdDispatchBase-None-08613" Storage
    tests.push_back({false, 4, "Descriptor size is 4 texels and highest texel accessed was 5"});

    for (const auto &test : tests) {
        uint32_t *data = (uint32_t *)offset_buffer.memory().map();
        *data = test.index;
        offset_buffer.memory().unmap();
        if (test.positive) {
        } else {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test.expected_error);
        }
        m_commandBuffer->QueueCommandBuffer();
        if (test.positive) {
        } else {
            m_errorMonitor->VerifyFound();
        }
        m_default_queue->wait();
    }

    if (multi_draw && multi_draw_features.multiDraw) {
        VkMultiDrawInfoEXT multi_draws[3] = {};
        multi_draws[0].vertexCount = multi_draws[1].vertexCount = multi_draws[2].vertexCount = 3;
        VkMultiDrawIndexedInfoEXT multi_draw_indices[3] = {};
        multi_draw_indices[0].indexCount = multi_draw_indices[1].indexCount = multi_draw_indices[2].indexCount = 3;

        vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        m_commandBuffer->begin(&begin_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        vk::CmdBindIndexBuffer(m_commandBuffer->handle(), buffer.handle(), 0, VK_INDEX_TYPE_UINT16);
        vk::CmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();

        uint32_t *data = (uint32_t *)offset_buffer.memory().map();
        *data = 8;
        offset_buffer.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiIndexedEXT-None-08613");
        m_commandBuffer->QueueCommandBuffer();
        m_errorMonitor->VerifyFound();

        m_commandBuffer->begin(&begin_info);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        vk::CmdDrawMultiEXT(m_commandBuffer->handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT));
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();

        data = (uint32_t *)offset_buffer.memory().map();
        *data = 0;
        offset_buffer.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMultiEXT-None-08612");
        m_commandBuffer->QueueCommandBuffer();
        m_errorMonitor->VerifyFound();
    }
}

void NegativeGpuAVOOB::ShaderBufferSizeTest(VkDeviceSize buffer_size, VkDeviceSize binding_offset, VkDeviceSize binding_range,
                                            VkDescriptorType descriptor_type, const char *fragment_shader,
                                            const char *expected_error, bool shader_objects) {
    if (shader_objects) {
        AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    }
    RETURN_IF_SKIP(InitGpuAvFramework());

    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper();
    dynamic_rendering_features.dynamicRendering = VK_TRUE;
    VkPhysicalDeviceShaderObjectFeaturesEXT shader_object_features = vku::InitStructHelper(&dynamic_rendering_features);
    shader_object_features.shaderObject = VK_TRUE;
    VkPhysicalDeviceFeatures2 features = vku::InitStructHelper();  // Make sure robust buffer access is not enabled
    if (shader_objects) {
        features.pNext = &shader_object_features;
    }
    RETURN_IF_SKIP(InitState(nullptr, &features));
    if (shader_objects) {
        InitDynamicRenderTarget();
    } else {
        InitRenderTarget();
    }

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, expected_error);

    OneOffDescriptorSet ds(m_device, {{0, descriptor_type, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&ds.layout_});

    vkt::Buffer buffer(*m_device, buffer_size,
                       (descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) ? VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
                                                                              : VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    VkDescriptorBufferInfo buffer_info;
    buffer_info.buffer = buffer.handle();
    buffer_info.offset = binding_offset;
    buffer_info.range = binding_range;

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = descriptor_type;
    descriptor_write.pBufferInfo = &buffer_info;

    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    vkt::Shader *vso = nullptr;
    vkt::Shader *fso = nullptr;
    if (shader_objects) {
        vso = new vkt::Shader(*m_device, VK_SHADER_STAGE_VERTEX_BIT,
                              GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexDrawPassthroughGlsl), &ds.layout_.handle());
        fso = new vkt::Shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader),
                              &ds.layout_.handle());
    }

    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    if (shader_objects) {
        m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget());
    } else {
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    }

    if (shader_objects) {
        const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                                VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                                VK_SHADER_STAGE_FRAGMENT_BIT};
        const VkShaderEXT shaders[] = {vso->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, fso->handle()};
        vk::CmdBindShadersEXT(m_commandBuffer->handle(), 5u, stages, shaders);
        SetDefaultDynamicStates(m_commandBuffer->handle());
    } else {
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    }
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1, &ds.set_,
                              0, nullptr);

    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    if (shader_objects) {
        m_commandBuffer->EndRendering();
    } else {
        m_commandBuffer->EndRenderPass();
    }
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer(true);
    m_errorMonitor->VerifyFound();
    DestroyRenderTarget();
    if (shader_objects) {
        delete vso;
        delete fso;
    }
}

TEST_F(NegativeGpuAVOOB, UniformBufferTooSmall) {
    TEST_DESCRIPTION("Test that an error is produced when trying to access uniform buffer outside the bound region.");
    char const *fsSource = R"glsl(
        #version 450

        layout(location=0) out vec4 x;
        layout(set=0, binding=0) uniform readonly foo { int x; int y; } bar;
        void main(){
           x = vec4(bar.x, bar.y, 0, 1);
        }
        )glsl";

    ShaderBufferSizeTest(4,  // buffer size
                         0,  // binding offset
                         4,  // binding range
                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, fsSource, "VUID-vkCmdDraw-None-08612");
}

TEST_F(NegativeGpuAVOOB, StorageBufferTooSmall) {
    TEST_DESCRIPTION("Test that an error is produced when trying to access storage buffer outside the bound region.");

    char const *fsSource = R"glsl(
        #version 450

        layout(location=0) out vec4 x;
        layout(set=0, binding=0) buffer readonly foo { int x; int y; } bar;
        void main(){
           x = vec4(bar.x, bar.y, 0, 1);
        }
        )glsl";

    ShaderBufferSizeTest(4,  // buffer size
                         0,  // binding offset
                         4,  // binding range
                         VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, fsSource, "VUID-vkCmdDraw-None-08613");
}

TEST_F(NegativeGpuAVOOB, UniformBufferTooSmallArray) {
    TEST_DESCRIPTION(
        "Test that an error is produced when trying to access uniform buffer outside the bound region. Uses array in block "
        "definition.");

    char const *fsSource = R"glsl(
        #version 450

        layout(location=0) out vec4 x;
        layout(set=0, binding=0) uniform readonly foo { int x[17]; } bar;
        void main(){
           int y = 0;
           for (int i = 0; i < 17; i++)
               y += bar.x[i];
           x = vec4(y, 0, 0, 1);
        }
        )glsl";

    ShaderBufferSizeTest(64,  // buffer size
                         0,   // binding offset
                         64,  // binding range
                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, fsSource, "VUID-vkCmdDraw-None-08612");
}

TEST_F(NegativeGpuAVOOB, UniformBufferTooSmallNestedStruct) {
    TEST_DESCRIPTION(
        "Test that an error is produced when trying to access uniform buffer outside the bound region. Uses nested struct in block "
        "definition.");

    char const *fsSource = R"glsl(
        #version 450

        struct S {
            int x;
            int y;
        };
        layout(location=0) out vec4 x;
        layout(set=0, binding=0) uniform readonly foo { int a; S b; } bar;
        void main(){
           x = vec4(bar.a, bar.b.x, bar.b.y, 1);
        }
        )glsl";

    ShaderBufferSizeTest(8,  // buffer size
                         0,  // binding offset
                         8,  // binding range
                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, fsSource, "VUID-vkCmdDraw-None-08612");
}

TEST_F(NegativeGpuAVOOB, ObjectUniformBufferTooSmall) {
    TEST_DESCRIPTION("Test that an error is produced when trying to access uniform buffer outside the bound region.");
    char const *fsSource = R"glsl(
        #version 450

        layout(location=0) out vec4 x;
        layout(set=0, binding=0) uniform readonly foo { int x; int y; } bar;
        void main(){
           x = vec4(bar.x, bar.y, 0, 1);
        }
        )glsl";

    ShaderBufferSizeTest(4,  // buffer size
                         0,  // binding offset
                         4,  // binding range
                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, fsSource, "Descriptor size is 4 and highest byte accessed was 7", true);
}

TEST_F(NegativeGpuAVOOB, GPL) {
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);

    RETURN_IF_SKIP(InitGpuAvFramework());

    VkPhysicalDeviceRobustness2FeaturesEXT robustness2_features = vku::InitStructHelper();
    VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT gpl_features = vku::InitStructHelper(&robustness2_features);
    auto features2 = GetPhysicalDeviceFeatures2(gpl_features);
    if (!robustness2_features.nullDescriptor) {
        GTEST_SKIP() << "nullDescriptor feature not supported";
    }
    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }
    features2.features.robustBufferAccess = VK_FALSE;
    robustness2_features.robustBufferAccess2 = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer offset_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, reqs);
    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, reqs);

    vkt::Buffer uniform_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, reqs);
    vkt::Buffer storage_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, reqs);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = uniform_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView uniform_buffer_view(*m_device, bvci);
    bvci.buffer = storage_texel_buffer.handle();
    vkt::BufferView storage_buffer_view(*m_device, bvci);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {3, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {4, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, offset_buffer.handle(), 0, 4);
    descriptor_set.WriteDescriptorBufferInfo(1, write_buffer.handle(), 0, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(2, VK_NULL_HANDLE, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferView(3, uniform_buffer_view.handle(), VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
    descriptor_set.WriteDescriptorBufferView(4, storage_buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    static const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform ufoo { uint index[]; } u_index;      // index[1]
        layout(set = 0, binding = 1) buffer StorageBuffer { uint data[]; } Data;  // data[4]
        layout(set = 0, binding = 2) buffer NullBuffer { uint data[]; } Null;     // VK_NULL_HANDLE
        layout(set = 0, binding = 3) uniform samplerBuffer u_buffer;              // texel_buffer[4]
        layout(set = 0, binding = 4, r32f) uniform imageBuffer s_buffer;          // texel_buffer[4]
        void main() {
            vec4 x;
            if (u_index.index[0] == 8)
                Data.data[u_index.index[0]] = 0xdeadca71;
            else if (u_index.index[0] == 0)
                Data.data[0] = u_index.index[4];
            else if (u_index.index[0] == 1)
                Data.data[0] = Null.data[40];  // No error
            else if (u_index.index[0] == 2)
                x = texelFetch(u_buffer, 5);
            else if (u_index.index[0] == 3)
                x = imageLoad(s_buffer, 5);
            else if (u_index.index[0] == 4)
                imageStore(s_buffer, 5, x);
            else if (u_index.index[0] == 5)  // No Error
                imageStore(s_buffer, 0, x);
            else if (u_index.index[0] == 6)  // No Error
                x = imageLoad(s_buffer, 0);
        }
    )glsl";
    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vertshader);
    vkt::GraphicsPipelineLibraryStage pre_raster_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper vi(*this);
    vi.InitVertexInputLibInfo();
    vi.InitState();
    vi.CreateGraphicsPipeline(false);

    CreatePipelineHelper pre_raster(*this);
    pre_raster.InitPreRasterLibInfo(&pre_raster_stage.stage_ci);
    pre_raster.InitState();
    pre_raster.gp_ci_.layout = pipeline_layout.handle();
    pre_raster.CreateGraphicsPipeline(false);

    const auto render_pass = pre_raster.gp_ci_.renderPass;
    const auto subpass = pre_raster.gp_ci_.subpass;

    CreatePipelineHelper fragment(*this);
    fragment.InitFragmentLibInfo(nullptr);
    fragment.gp_ci_.stageCount = 0;
    fragment.shader_stages_.clear();
    fragment.gp_ci_.layout = pipeline_layout.handle();
    fragment.gp_ci_.renderPass = render_pass;
    fragment.gp_ci_.subpass = subpass;
    fragment.CreateGraphicsPipeline(false);

    CreatePipelineHelper frag_out(*this);
    frag_out.InitFragmentOutputLibInfo();
    frag_out.gp_ci_.renderPass = render_pass;
    frag_out.gp_ci_.subpass = subpass;
    frag_out.CreateGraphicsPipeline(false);

    std::array<VkPipeline, 4> libraries = {
        vi.pipeline_,
        pre_raster.pipeline_,
        fragment.pipeline_,
        frag_out.pipeline_,
    };
    vkt::GraphicsPipelineFromLibraries pipe(*m_device, libraries, pipeline_layout.handle());

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_commandBuffer->begin(&begin_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    struct TestCase {
        bool positive;
        uint32_t index;
        char const *expected_error;
    };
    std::vector<TestCase> tests;
    // "VUID-vkCmdDispatchBase-None-08613" Storage
    tests.push_back({false, 8, "Descriptor size is 16 and highest byte accessed was 35"});
    // Uniform buffer stride rounded up to the alignment of a vec4 (16 bytes)
    // so u_index.index[4] accesses bytes 64, 65, 66, and 67
    // "VUID-vkCmdDispatchBase-None-08612" Uniform
    tests.push_back({false, 0, "Descriptor size is 4 and highest byte accessed was 67"});
    tests.push_back({true, 1, ""});
    // "VUID-vkCmdDispatchBase-None-08612" Uniform
    tests.push_back({false, 2, "Descriptor size is 4 texels and highest texel accessed was 5"});
    // "VUID-vkCmdDispatchBase-None-08613" Storage
    tests.push_back({false, 3, "Descriptor size is 4 texels and highest texel accessed was 5"});
    // "VUID-vkCmdDispatchBase-None-08613" Storage
    tests.push_back({false, 4, "Descriptor size is 4 texels and highest texel accessed was 5"});

    for (const auto &test : tests) {
        uint32_t *data = (uint32_t *)offset_buffer.memory().map();
        *data = test.index;
        offset_buffer.memory().unmap();
        if (test.positive) {
        } else {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test.expected_error);
        }
        m_commandBuffer->QueueCommandBuffer();
        if (test.positive) {
        } else {
            m_errorMonitor->VerifyFound();
        }
        m_default_queue->wait();
    }
}

TEST_F(NegativeGpuAVOOB, GPLIndependentSets) {
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);

    RETURN_IF_SKIP(InitGpuAvFramework());

    VkPhysicalDeviceRobustness2FeaturesEXT robustness2_features = vku::InitStructHelper();
    VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT gpl_features = vku::InitStructHelper(&robustness2_features);
    auto features2 = GetPhysicalDeviceFeatures2(gpl_features);
    if (!robustness2_features.nullDescriptor) {
        GTEST_SKIP() << "nullDescriptor feature not supported";
    }
    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }
    features2.features.robustBufferAccess = VK_FALSE;
    robustness2_features.robustBufferAccess2 = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer offset_buffer(*m_device, 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, reqs);
    vkt::Buffer write_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, reqs);

    vkt::Buffer uniform_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, reqs);
    vkt::Buffer storage_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, reqs);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = uniform_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView uniform_buffer_view(*m_device, bvci);
    bvci.buffer = storage_texel_buffer.handle();
    vkt::BufferView storage_buffer_view(*m_device, bvci);

    OneOffDescriptorSet vertex_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}});
    OneOffDescriptorSet common_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    OneOffDescriptorSet fragment_set(m_device,
                                     {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                      {1, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                      {2, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});

    const vkt::PipelineLayout pipeline_layout_vs(*m_device, {&vertex_set.layout_, &common_set.layout_, nullptr}, {},
                                                 VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout_fs(*m_device, {nullptr, &common_set.layout_, &fragment_set.layout_}, {},
                                                 VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&vertex_set.layout_, &common_set.layout_, &fragment_set.layout_}, {},
                                              VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    vertex_set.WriteDescriptorBufferInfo(0, write_buffer.handle(), 0, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    vertex_set.UpdateDescriptorSets();
    common_set.WriteDescriptorBufferInfo(0, offset_buffer.handle(), 0, 4);
    common_set.UpdateDescriptorSets();
    fragment_set.WriteDescriptorBufferInfo(0, VK_NULL_HANDLE, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    fragment_set.WriteDescriptorBufferView(1, uniform_buffer_view.handle(), VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
    fragment_set.WriteDescriptorBufferView(2, storage_buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    fragment_set.UpdateDescriptorSets();

    const std::array<VkDescriptorSet, 3> desc_sets = {vertex_set.set_, common_set.set_, fragment_set.set_};

    static const char vertshader[] = R"glsl(
        #version 450
        layout(set = 1, binding = 0) uniform ufoo { uint index[]; } u_index;      // index[1]
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } Data;  // data[4]
        const vec2 vertices[3] = vec2[](
            vec2(-1.0, -1.0),
            vec2(1.0, -1.0),
            vec2(0.0, 1.0)
        );
        void main() {
            if (u_index.index[0] == 8) {
                Data.data[u_index.index[0]] = 0xdeadca71;
            } else if (u_index.index[0] == 0) {
                Data.data[0] = u_index.index[4];
            }
            gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
        }
    )glsl";
    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vertshader);
    vkt::GraphicsPipelineLibraryStage pre_raster_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper vi(*this);
    vi.InitVertexInputLibInfo();
    vi.InitState();
    vi.CreateGraphicsPipeline(false);

    CreatePipelineHelper pre_raster(*this);
    pre_raster.InitPreRasterLibInfo(&pre_raster_stage.stage_ci);
    pre_raster.InitState();
    pre_raster.gp_ci_.layout = pipeline_layout_vs.handle();
    pre_raster.CreateGraphicsPipeline(false);

    static const char frag_shader[] = R"glsl(
        #version 450
        layout(set = 1, binding = 0) uniform ufoo { uint index[]; } u_index;      // index[1]
        layout(set = 2, binding = 0) buffer NullBuffer { uint data[]; } Null;     // VK_NULL_HANDLE
        layout(set = 2, binding = 1) uniform samplerBuffer u_buffer;              // texel_buffer[4]
        layout(set = 2, binding = 2, r32f) uniform imageBuffer s_buffer;          // texel_buffer[4]
        layout(location = 0) out vec4 c_out;
        void main() {
            vec4 x;
            if (u_index.index[0] == 2) {
                x = texelFetch(u_buffer, 5);
            } else if (u_index.index[0] == 3) {
                x = imageLoad(s_buffer, 5);
            } else if (u_index.index[0] == 4) {
                imageStore(s_buffer, 5, x);
            } else if (u_index.index[0] == 5) { // No Error
                imageStore(s_buffer, 0, x);
            } else if (u_index.index[0] == 6) { // No Error
                x = imageLoad(s_buffer, 0);
            }
            c_out = x;
        }
    )glsl";
    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_shader);
    vkt::GraphicsPipelineLibraryStage fragment_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper fragment(*this);
    fragment.InitFragmentLibInfo(&fragment_stage.stage_ci);
    fragment.gp_ci_.layout = pipeline_layout_fs.handle();
    fragment.CreateGraphicsPipeline(false);

    CreatePipelineHelper frag_out(*this);
    frag_out.InitFragmentOutputLibInfo();
    frag_out.CreateGraphicsPipeline(false);

    std::array<VkPipeline, 4> libraries = {
        vi.pipeline_,
        pre_raster.pipeline_,
        fragment.pipeline_,
        frag_out.pipeline_,
    };
    vkt::GraphicsPipelineFromLibraries pipe(*m_device, libraries, pipeline_layout.handle());

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_commandBuffer->begin(&begin_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0,
                              static_cast<uint32_t>(desc_sets.size()), desc_sets.data(), 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    struct TestCase {
        bool positive;
        uint32_t index;
        char const *expected_error;
    };
    std::vector<TestCase> tests;
    // "VUID-vkCmdDispatchBase-None-08613" Storage
    // tests.push_back({false, 8, "Descriptor size is 16 and highest byte accessed was 35"});
    // Uniform buffer stride rounded up to the alignment of a vec4 (16 bytes)
    // so u_index.index[4] accesses bytes 64, 65, 66, and 67
    // "VUID-vkCmdDispatchBase-None-08612" Uniform
    // tests.push_back({false, 0, "Descriptor size is 4 and highest byte accessed was 67"});
    // tests.push_back({true, 1, ""});
    // "VUID-vkCmdDispatchBase-None-08612" Uniform
    // tests.push_back({false, 2, "Descriptor size is 4 texels and highest texel accessed was 5"});
    // // "VUID-vkCmdDispatchBase-None-08613" Storage
    // tests.push_back({false, 3, "Descriptor size is 4 texels and highest texel accessed was 5"});
    // "VUID-vkCmdDispatchBase-None-08613" Storage
    tests.push_back({false, 4, "Descriptor size is 4 texels and highest texel accessed was 5"});

    for (const auto &test : tests) {
        uint32_t *data = (uint32_t *)offset_buffer.memory().map();
        *data = test.index;
        offset_buffer.memory().unmap();
        if (test.positive) {
        } else {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test.expected_error);
        }
        m_commandBuffer->QueueCommandBuffer();
        if (test.positive) {
        } else {
            m_errorMonitor->VerifyFound();
        }
        m_default_queue->wait();
    }
}

TEST_F(NegativeGpuAVOOB, StorageBuffer) {
    TEST_DESCRIPTION("Make suree OOB is still checked when result is from a BufferDeviceAddress");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        #extension GL_ARB_gpu_shader_int64 : enable

        struct Test {
            float a;
        };

        layout(buffer_reference, std430, buffer_reference_align = 16) buffer TestBuffer {
            Test test;
        };

        Test GetTest(uint64_t ptr) {
            return TestBuffer(ptr).test;
        }

        // 12 bytes large
        layout(set = 0, binding = 0) buffer foo {
            TestBuffer data;
            float x;
        } in_buffer;

        void main() {
            in_buffer.x = GetTest(uint64_t(in_buffer.data)).a;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.InitState();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer block_buffer(*m_device, 16, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR, mem_props, &allocate_flag_info);

    // too small
    vkt::Buffer in_buffer(*m_device, 8, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, mem_props);

    VkDeviceAddress block_ptr = block_buffer.address();

    uint8_t *in_buffer_ptr = (uint8_t *)in_buffer.memory().map();
    memcpy(in_buffer_ptr, &block_ptr, sizeof(VkDeviceAddress));
    in_buffer.memory().unmap();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-None-08613");
    m_default_queue->submit(*m_commandBuffer, false);
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
}

void NegativeGpuAVOOB::ComputeStorageBufferTest(const char *expected_error, const char *shader, VkDeviceSize buffer_size) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.InitState();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    // too small
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer in_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, mem_props);

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, expected_error);
    m_default_queue->submit(*m_commandBuffer, false);
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVOOB, Vector) {
    TEST_DESCRIPTION("index into a vector OOB");

    char const *shader_source = R"glsl(
        #version 450

        // 28 bytes large
        layout(set = 0, binding = 0, std430) buffer foo {
            int a;
            vec3 b; // offset 16
        } in_buffer;

        void main() {
            in_buffer.b.y = 0.0;
        }
    )glsl";
    ComputeStorageBufferTest("VUID-vkCmdDispatch-None-08613", shader_source, 20);
}

TEST_F(NegativeGpuAVOOB, Matrix) {
    TEST_DESCRIPTION("index into a matrix OOB");

    char const *shader_source = R"glsl(
        #version 450

        // 32 bytes large
        layout(set = 0, binding = 0, std430) buffer foo {
            int a;
            mat3x2 b; // offset 8
        } in_buffer;

        void main() {
            in_buffer.b[2][1] = 0.0;
        }
    )glsl";
    ComputeStorageBufferTest("VUID-vkCmdDispatch-None-08613", shader_source, 30);
}

TEST_F(NegativeGpuAVOOB, TexelFetch) {
    TEST_DESCRIPTION("index into a texelFetch OOB");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450

        layout(set = 0, binding = 0, std430) buffer foo {
            vec4 a;
        } out_buffer;

        layout(set = 0, binding = 1) uniform samplerBuffer u_buffer; // texel_buffer[4]

        void main() {
            out_buffer.a = texelFetch(u_buffer, 4);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                          {1, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.InitState();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer out_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, mem_props);

    // 16 bytes only holds 4 indexes
    vkt::Buffer uniform_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, mem_props);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = uniform_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView uniform_buffer_view(*m_device, bvci);

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, out_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->WriteDescriptorBufferView(1, uniform_buffer_view.handle(), VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-None-08612");
    m_default_queue->submit(*m_commandBuffer, false);
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVOOB, ImageLoad) {
    TEST_DESCRIPTION("index into a imageLoad OOB");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0, std430) buffer foo {
            vec4 a;
        } out_buffer;

        layout(set = 0, binding = 1, r32f) uniform imageBuffer s_buffer;  // texel_buffer[4]

        void main() {
            out_buffer.a = imageLoad(s_buffer, 4);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                          {1, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.InitState();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer out_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, mem_props);

    // 16 bytes only holds 4 indexes
    vkt::Buffer storage_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, mem_props);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = storage_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView storage_buffer_view(*m_device, bvci);

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, out_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->WriteDescriptorBufferView(1, storage_buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-None-08613");
    m_default_queue->submit(*m_commandBuffer, false);
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVOOB, ImageStore) {
    TEST_DESCRIPTION("index into a imageStore OOB");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0, r32f) uniform imageBuffer s_buffer;  // texel_buffer[4]

        void main() {
            vec4 x = imageLoad(s_buffer, 4); // valid load
            imageStore(s_buffer, 5, x);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.InitState();
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    // 16 bytes only holds 4 indexes
    vkt::Buffer storage_texel_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, mem_props);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = storage_texel_buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView storage_buffer_view(*m_device, bvci);

    pipe.descriptor_set_->WriteDescriptorBufferView(0, storage_buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-None-08613");
    m_default_queue->submit(*m_commandBuffer, false);
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVOOB, Geometry) {
    TEST_DESCRIPTION("Basic Geometry shader test");
    AddRequiredFeature(vkt::Feature::geometryShader);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    char const *gsSource = R"glsl(
        #version 450
        layout(triangles) in;
        layout(triangle_strip, max_vertices=3) out;
        layout(set = 0, binding = 0, std430) buffer foo {
            vec3 x;
        } in_buffer;

        void main() {
            in_buffer.x.y = 0.0;
            gl_Position = vec4(1);
            EmitVertex();
        }
    )glsl";

    VkShaderObj vs(this, kVertexPointSizeGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    // too small
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer in_buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, mem_props);

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08613");
    m_default_queue->submit(*m_commandBuffer, false);
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
}

// TODO - Not being triggered, shader is instrumented, but doesn't seem tessellation shader is executed
TEST_F(NegativeGpuAVOOB, DISABLED_TessellationControl) {
    TEST_DESCRIPTION("Basic TessellationControl shader test");
    AddRequiredFeature(vkt::Feature::tessellationShader);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    static const char shader_source[] = R"glsl(
        #version 460
        layout(vertices=3) out;
        layout(set = 0, binding = 0, std430) buffer foo {
            vec3 x;
        } in_buffer;

        void main() {
            in_buffer.x.y = 0.0;
            gl_TessLevelOuter[0] = gl_TessLevelOuter[1] = gl_TessLevelOuter[2] = 1;
            gl_TessLevelInner[0] = 1;
        }
    )glsl";

    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj tcs(this, shader_source, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj tes(this, kTessellationEvalMinimalGlsl, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    VkPipelineTessellationStateCreateInfo tess_ci = vku::InitStructHelper();
    tess_ci.patchControlPoints = 4u;

    CreatePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.InitState();
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    pipe.tess_ci_ = tess_ci;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), tcs.GetStageCreateInfo(), tes.GetStageCreateInfo(),
                           pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    // too small
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer in_buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, mem_props);

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08613");
    m_default_queue->submit(*m_commandBuffer, false);
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
}

// TODO - Not being triggered, shader is instrumented, but doesn't seem tessellation shader is executed
TEST_F(NegativeGpuAVOOB, DISABLED_TessellationEvaluation) {
    TEST_DESCRIPTION("Basic TessellationEvaluation shader test");
    AddRequiredFeature(vkt::Feature::tessellationShader);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    static const char shader_source[] = R"glsl(
        #version 460
        layout(triangles, equal_spacing, cw) in;
        layout(set = 0, binding = 0, std430) buffer foo {
            vec3 x;
        } in_buffer;
        void main() {
            in_buffer.x.y = 0.0;
            gl_Position = vec4(1);
        }
    )glsl";

    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj tcs(this, kTessellationControlMinimalGlsl, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    VkShaderObj tes(this, shader_source, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    VkPipelineTessellationStateCreateInfo tess_ci = vku::InitStructHelper();
    tess_ci.patchControlPoints = 4u;

    CreatePipelineHelper pipe(*this);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.InitState();
    pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    pipe.tess_ci_ = tess_ci;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), tcs.GetStageCreateInfo(), tes.GetStageCreateInfo(),
                           pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    // too small
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer in_buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, mem_props);

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08613");
    m_default_queue->submit(*m_commandBuffer, false);
    m_default_queue->wait();
    m_errorMonitor->VerifyFound();
}