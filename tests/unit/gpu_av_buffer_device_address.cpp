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

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4983
TEST_F(NegativeGpuAVBufferDeviceAddress, DISABLED_Basic) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddOptionalExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    const bool mesh_shader_supported = IsExtensionsEnabled(VK_NV_MESH_SHADER_EXTENSION_NAME);

    VkPhysicalDeviceMeshShaderFeaturesNV mesh_shader_features = vku::InitStructHelper();
    auto bda_features =
        vku::InitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>(mesh_shader_supported ? &mesh_shader_features : nullptr);

    VkPhysicalDeviceFeatures2KHR features2 = GetPhysicalDeviceFeatures2(bda_features);
    if (!features2.features.shaderInt64) {
        GTEST_SKIP() << "shaderInt64 is not supported";
    }
    features2.features.robustBufferAccess = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the pointer and write count
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    // 64 bit pointer + int
    vkt::Buffer buffer0(*m_device, 12, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);

    // Make another buffer to write to
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    // Buffer should be 16*4 = 64 bytes
    vkt::Buffer buffer1(*m_device, 64, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR, mem_props, &allocate_flag_info);

    // Get device address of buffer to write to
    auto pBuffer = buffer1.address();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    VkCommandBufferInheritanceInfo hinfo = vku::InitStructHelper();
    begin_info.pInheritanceInfo = &hinfo;

    struct TestCase {
        std::string name;
        std::string error;
        VkDeviceAddress push_constants[2] = {};
    };
    std::array<TestCase, 3> testcases{{{"starting address too low", "access out of bounds", {pBuffer - 16, 4}},
                                       {"run past the end", "access out of bounds", {pBuffer, 5}},
                                       {"positive", "", {pBuffer, 4}}}};

    // push constant version
    {
        VkPushConstantRange push_constant_ranges = {};
        push_constant_ranges.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        push_constant_ranges.offset = 0;
        push_constant_ranges.size = 2 * sizeof(VkDeviceAddress);

        VkPipelineLayoutCreateInfo plci = vku::InitStructHelper();
        plci = vku::InitStructHelper();
        plci.pushConstantRangeCount = 1;
        plci.pPushConstantRanges = &push_constant_ranges;
        plci.setLayoutCount = 0;
        plci.pSetLayouts = nullptr;

        vkt::PipelineLayout pipeline_layout(*m_device, plci);

        char const *shader_source = R"glsl(
            #version 450
            #extension GL_EXT_buffer_reference : enable
            layout(buffer_reference, buffer_reference_align = 16) buffer bufStruct;
            layout(push_constant) uniform ufoo {
                bufStruct data;
                int nWrites;
            } u_info;
            layout(buffer_reference, std140) buffer bufStruct {
                int a[4];
            };
            void main() {
                for (int i=0; i < u_info.nWrites; ++i) {
                    u_info.data.a[i] = 0xdeadca71;
                }
            }
        )glsl";
        VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main", true);

        CreatePipelineHelper pipe(*this);
        pipe.InitState();
        pipe.shader_stages_[0] = vs.GetStageCreateInfo();
        pipe.gp_ci_.layout = pipeline_layout.handle();
        pipe.CreateGraphicsPipeline();

        for (const auto &test : testcases) {
            m_commandBuffer->begin(&begin_info);
            m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
            vk::CmdPushConstants(m_commandBuffer->handle(), pipeline_layout.handle(), VK_SHADER_STAGE_VERTEX_BIT, 0,
                                 sizeof(test.push_constants), test.push_constants);
            vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
            m_commandBuffer->EndRenderPass();
            m_commandBuffer->end();

            if (!test.error.empty()) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "access out of bounds");
            }
            vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
            vk::QueueWaitIdle(m_default_queue);
            if (!test.error.empty()) {
                m_errorMonitor->VerifyFound();
            }
            m_commandBuffer->reset();
        }
    }
    // descriptor set version
    {
        OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

        const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
        VkDescriptorBufferInfo buffer_test_buffer_info = {};
        buffer_test_buffer_info.buffer = buffer0.handle();
        buffer_test_buffer_info.offset = 0;
        buffer_test_buffer_info.range = sizeof(uint32_t);

        VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
        descriptor_write.dstSet = descriptor_set.set_;
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.pBufferInfo = &buffer_test_buffer_info;
        vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

        char const *shader_source = R"glsl(
            #version 450
            #extension GL_EXT_buffer_reference : enable
            layout(buffer_reference, buffer_reference_align = 16) buffer bufStruct;
            layout(set = 0, binding = 0) uniform ufoo {
                bufStruct data;
                int nWrites;
            } u_info;
            layout(buffer_reference, std140) buffer bufStruct {
                int a[4];
            };
            void main() {
                for (int i=0; i < u_info.nWrites; ++i) {
                    u_info.data.a[i] = 0xdeadca71;
                }
            }
        )glsl";
        VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main", true);

        CreatePipelineHelper pipe(*this);
        pipe.InitState();
        pipe.shader_stages_[0] = vs.GetStageCreateInfo();
        pipe.gp_ci_.layout = pipeline_layout.handle();
        pipe.CreateGraphicsPipeline();

        m_commandBuffer->begin(&begin_info);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();

        for (const auto &test : testcases) {
            auto *data = static_cast<VkDeviceAddress *>(buffer0.memory().map());
            data[0] = test.push_constants[0];
            data[1] = test.push_constants[1];
            buffer0.memory().unmap();

            if (!test.error.empty()) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "access out of bounds");
            }
            vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
            vk::QueueWaitIdle(m_default_queue);
            if (!test.error.empty()) {
                m_errorMonitor->VerifyFound();
            }
        }
    }

    if (mesh_shader_supported) {
        const unsigned push_constant_range_count = 1;
        VkPushConstantRange push_constant_ranges[push_constant_range_count] = {};
        push_constant_ranges[0].stageFlags = VK_SHADER_STAGE_MESH_BIT_NV;
        push_constant_ranges[0].offset = 0;
        push_constant_ranges[0].size = 2 * sizeof(VkDeviceAddress);

        VkPipelineLayoutCreateInfo plci = vku::InitStructHelper();
        plci.pushConstantRangeCount = push_constant_range_count;
        plci.pPushConstantRanges = push_constant_ranges;
        plci.setLayoutCount = 0;
        plci.pSetLayouts = nullptr;
        vkt::PipelineLayout mesh_pipeline_layout(*m_device, plci);

        char const *mesh_shader_source = R"glsl(
            #version 460
            #extension GL_NV_mesh_shader : require
            #extension GL_EXT_buffer_reference : enable
            layout(buffer_reference, buffer_reference_align = 16) buffer bufStruct;
            layout(push_constant) uniform ufoo {
                bufStruct data;
                int nWrites;
            } u_info;
            layout(buffer_reference, std140) buffer bufStruct {
                int a[4];
            };

            layout(local_size_x = 32) in;
            layout(max_vertices = 64, max_primitives = 126) out;
            layout(triangles) out;

            uint invocationID = gl_LocalInvocationID.x;
            void main() {
                if (invocationID == 0) {
                    for (int i=0; i < u_info.nWrites; ++i) {
                        u_info.data.a[i] = 0xdeadca71;
                    }
                }
            }
        )glsl";
        VkShaderObj ms(this, mesh_shader_source, VK_SHADER_STAGE_MESH_BIT_NV, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main",
                       true);

        CreatePipelineHelper mesh_pipe(*this);
        mesh_pipe.InitState();
        mesh_pipe.shader_stages_ = {ms.GetStageCreateInfo()};
        mesh_pipe.gp_ci_.layout = mesh_pipeline_layout.handle();
        mesh_pipe.CreateGraphicsPipeline();

        m_commandBuffer->begin(&begin_info);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, mesh_pipe.Handle());
        VkDeviceAddress push_constants[2] = {pBuffer, 5};
        vk::CmdPushConstants(m_commandBuffer->handle(), mesh_pipeline_layout.handle(), VK_SHADER_STAGE_MESH_BIT_NV, 0,
                             sizeof(push_constants), push_constants);
        vk::CmdDrawMeshTasksNV(m_commandBuffer->handle(), 1, 0);
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "access out of bounds");
        vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(m_default_queue);
        m_errorMonitor->VerifyFound();
    }
}
