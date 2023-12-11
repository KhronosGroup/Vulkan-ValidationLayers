/*
 * Copyright (c) 2020-2023 The Khronos Group Inc.
 * Copyright (c) 2020-2023 Valve Corporation
 * Copyright (c) 2020-2023 LunarG, Inc.
 * Copyright (c) 2020-2023 Google, Inc.
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
        vk::QueueWaitIdle(m_default_queue);
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

    char const *vsSource = R"glsl(
        #version 450
        vec2 vertices[3];
        void main(){
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
              gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
        }
        )glsl";

    vkt::Shader *vso = nullptr;
    vkt::Shader *fso = nullptr;
    if (shader_objects) {
        vso = new vkt::Shader(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vsSource),
                              &ds.layout_.handle());
        fso = new vkt::Shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader),
                              &ds.layout_.handle());
    }

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
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
        vk::QueueWaitIdle(m_default_queue);
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
        vk::QueueWaitIdle(m_default_queue);
    }
}
