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
#include "../framework/shader_object_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/gpu_av_helper.h"

void DebugPrintfTests::InitDebugPrintfFramework(void *p_next, bool reserve_slot) {
    VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
                                              VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT};
    VkValidationFeatureDisableEXT disables[] = {
        VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT, VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT,
        VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT, VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};
    VkValidationFeaturesEXT features = vku::InitStructHelper(p_next);
    // Most tests don't need to reserve the slot, so keep it as an option for now
    features.enabledValidationFeatureCount = reserve_slot ? 2 : 1;
    features.disabledValidationFeatureCount = 4;
    features.pEnabledValidationFeatures = enables;
    features.pDisabledValidationFeatures = disables;

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework(&features));

    if (!CanEnableGpuAV(*this)) {
        GTEST_SKIP() << "Requirements for GPU-AV are not met";
    }
    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        GTEST_SKIP() << "Currently disabled for Portability";
    }
}

class NegativeDebugPrintf : public DebugPrintfTests {};

TEST_F(NegativeDebugPrintf, BasicCompute) {
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
        )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "float == 3.141500");
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, BasicUsage) {
    TEST_DESCRIPTION("Verify that calls to debugPrintfEXT are received in debug stream");
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the test number
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer_in(*m_device, 8, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer_in.handle(), 0, sizeof(uint32_t));
    descriptor_set.UpdateDescriptorSets();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 0, binding = 0) uniform ufoo {
            int whichtest;
        } u_info;
        void main() {
            float myfloat = 3.1415f;
            int foo = -135;
            if (gl_VertexIndex == 0) {
                switch(u_info.whichtest) {
                    case 0:
                        debugPrintfEXT("Here are two float values %f, %f", 1.0, myfloat);
                        break;
                    case 1:
                        debugPrintfEXT("Here's a smaller float value %1.2f", myfloat);
                        break;
                    case 2:
                        debugPrintfEXT("Here's an integer %i with text before and after it", foo);
                        break;
                    case 3:
                        foo = 256;
                        debugPrintfEXT("Here's an integer in octal %o and hex 0x%x", foo, foo);
                        break;
                    case 4:
                        debugPrintfEXT("%d is a negative integer", foo);
                        break;
                    case 5:
                        vec4 floatvec = vec4(1.2f, 2.2f, 3.2f, 4.2f);
                        debugPrintfEXT("Here's a vector of floats %1.2v4f", floatvec);
                        break;
                    case 6:
                        debugPrintfEXT("Here's a float in sn %e", myfloat);
                        break;
                    case 7:
                        debugPrintfEXT("Here's a float in sn %1.2e", myfloat);
                        break;
                    case 8:
                        debugPrintfEXT("Here's a float in shortest %g", myfloat);
                        break;
                    case 9:
                        debugPrintfEXT("Here's a float in hex %1.9a", myfloat);
                        break;
                    case 10:
                        debugPrintfEXT("First printf with a %% and no value");
                        debugPrintfEXT("Second printf with a value %i", foo);
                        break;
                }
            }
            gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
        }
        )glsl";
    std::vector<char const *> messages;
    messages.push_back("Here are two float values 1.000000, 3.141500");
    messages.push_back("Here's a smaller float value 3.14");
    messages.push_back("Here's an integer -135 with text before and after it");
    messages.push_back("Here's an integer in octal 400 and hex 0x100");
    messages.push_back("-135 is a negative integer");
    messages.push_back("Here's a vector of floats 1.20, 2.20, 3.20, 4.20");
    messages.push_back("Here's a float in sn 3.141500e+00");
    messages.push_back("Here's a float in sn 3.14e+00");
    messages.push_back("Here's a float in shortest 3.1415");
    messages.push_back("Here's a float in hex 0x1.921cac000p+1");
    // Two error messages have to be last in the vector
    messages.push_back("First printf with a % and no value");
    messages.push_back("Second printf with a value -135");
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo()};
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    for (uint32_t i = 0; i < messages.size(); i++) {
        VkDeviceAddress *data = (VkDeviceAddress *)buffer_in.memory().map();
        data[0] = i;
        buffer_in.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kInformationBit, messages[i]);
        if (10 == i) {
            m_errorMonitor->SetDesiredFailureMsg(kInformationBit, messages[i + 1]);
            i++;
        }
        m_default_queue->Submit(*m_commandBuffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDebugPrintf, MultiDraw) {
    TEST_DESCRIPTION("Verify that calls to debugPrintfEXT are received in debug stream");
    AddRequiredExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiDraw);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer_in(*m_device, 8, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer_in.handle(), 0, sizeof(uint32_t));
    descriptor_set.UpdateDescriptorSets();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 0, binding = 0) uniform ufoo {
            int whichtest;
        } u_info;
        void main() {
            float myfloat = 3.1415f;
            int foo = -135;
            // referencing gl_InstanceIndex appears to be required to ensure this shader runs multiple times
            // when called from vkCmdDrawMultiEXT().
            if (gl_VertexIndex == 0 && gl_InstanceIndex < 10000) {
                switch(u_info.whichtest) {
                    case 0:
                        debugPrintfEXT("Here are two float values %f, %f", 1.0, myfloat);
                        break;
                    case 1:
                        debugPrintfEXT("Here's a smaller float value %1.2f", myfloat);
                        break;
                }
            }
            gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
        }
        )glsl";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo()};
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    VkMultiDrawInfoEXT multi_draws[3] = {};
    multi_draws[0].vertexCount = multi_draws[1].vertexCount = multi_draws[2].vertexCount = 3;
    VkMultiDrawIndexedInfoEXT multi_draw_indices[3] = {};
    multi_draw_indices[0].indexCount = multi_draw_indices[1].indexCount = multi_draw_indices[2].indexCount = 3;
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDrawMultiEXT(m_commandBuffer->handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    VkDeviceAddress *data = (VkDeviceAddress *)buffer_in.memory().map();
    data[0] = 0;
    buffer_in.memory().unmap();
    for (auto i = 0; i < 3; i++) {
        m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Here are two float values 1.000000, 3.141500");
    }
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    uint16_t *ptr = static_cast<uint16_t *>(buffer.memory().map());
    ptr[0] = 0;
    ptr[1] = 1;
    ptr[2] = 2;
    buffer.memory().unmap();
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), buffer.handle(), 0, VK_INDEX_TYPE_UINT16);
    vk::CmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    data = (VkDeviceAddress *)buffer_in.memory().map();
    data[0] = 1;
    buffer_in.memory().unmap();
    for (auto i = 0; i < 3; i++) {
        m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Here's a smaller float value 3.14");
    }
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, Int64) {
    TEST_DESCRIPTION("Verify that calls to debugPrintfEXT are received in debug stream");
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    if (!m_device->phy().features().shaderInt64) {
        GTEST_SKIP() << "shaderInt64 not supported";
    }
    InitRenderTarget();

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer_in(*m_device, 8, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer_in.handle(), 0, sizeof(uint32_t));
    descriptor_set.UpdateDescriptorSets();

    char const *shader_source_int64 = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_ARB_gpu_shader_int64 : enable
        layout(set = 0, binding = 0) uniform ufoo {
            int whichtest;
        } u_info;
        void main() {
            uint64_t bigvar = 0x2000000000000001ul;
            if (gl_VertexIndex == 0) {
                switch(u_info.whichtest) {
                    case 0:
                        debugPrintfEXT("Here's an unsigned long 0x%ul", bigvar);
                        break;
                    case 1:
                        u64vec4 vecul = u64vec4(bigvar, bigvar, bigvar, bigvar);
                        debugPrintfEXT("Here's a vector of ul %v4ul", vecul);
                        break;
                    case 2:
                        debugPrintfEXT("Unsigned long as decimal %lu and as hex 0x%lx", bigvar, bigvar);
                        break;
                }
            }
            gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
        }
        )glsl";
    VkShaderObj vs_int64(this, shader_source_int64, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs_int64.GetStageCreateInfo()};
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    VkDeviceAddress *data = (VkDeviceAddress *)buffer_in.memory().map();
    data[0] = 0;
    buffer_in.memory().unmap();
    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Here's an unsigned long 0x2000000000000001");
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    data = (VkDeviceAddress *)buffer_in.memory().map();
    data[0] = 1;
    buffer_in.memory().unmap();
    m_errorMonitor->SetDesiredFailureMsg(
        kInformationBit, "Here's a vector of ul 2000000000000001, 2000000000000001, 2000000000000001, 2000000000000001");
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    data = (VkDeviceAddress *)buffer_in.memory().map();
    data[0] = 2;
    buffer_in.memory().unmap();
    m_errorMonitor->SetDesiredFailureMsg(kInformationBit,
                                         "Unsigned long as decimal 2305843009213693953 and as hex 0x2000000000000001");
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, MeshTaskShaders) {
    TEST_DESCRIPTION("Test debug printf in mesh and task shaders.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDebugPrintfFramework());

    // Create a device that enables mesh_shader
    VkPhysicalDeviceMeshShaderFeaturesNV mesh_shader_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(mesh_shader_features);

    RETURN_IF_SKIP(InitState(nullptr, &mesh_shader_features));
    InitRenderTarget();

    static const char taskShaderText[] = R"glsl(
        #version 460
        #extension GL_NV_mesh_shader : enable
        #extension GL_EXT_debug_printf : enable
        layout(local_size_x = 32) in;
        uint invocationID = gl_LocalInvocationID.x;
        void main() {
            if (invocationID == 0) {
                gl_TaskCountNV = 1;
                debugPrintfEXT("hello from task shader");
            }
        }
        )glsl";

    static const char meshShaderText[] = R"glsl(
        #version 450
        #extension GL_NV_mesh_shader : require
        #extension GL_EXT_debug_printf : enable
        layout(local_size_x = 1) in;
        layout(max_vertices = 3) out;
        layout(max_primitives = 1) out;
        layout(triangles) out;
        uint invocationID = gl_LocalInvocationID.x;
        void main() {
            if (invocationID == 0) {
                debugPrintfEXT("hello from mesh shader");
            }
        }
        )glsl";

    VkShaderObj ts(this, taskShaderText, VK_SHADER_STAGE_TASK_BIT_NV);
    VkShaderObj ms(this, meshShaderText, VK_SHADER_STAGE_MESH_BIT_NV);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {ts.GetStageCreateInfo(), ms.GetStageCreateInfo()};
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDrawMeshTasksNV(m_commandBuffer->handle(), 1, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "hello from task shader");
    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "hello from mesh shader");
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, GPL) {
    TEST_DESCRIPTION("Verify that calls to debugPrintfEXT are received in debug stream");
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the test number
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer_in(*m_device, 8, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    descriptor_set.WriteDescriptorBufferInfo(0, buffer_in.handle(), 0, sizeof(uint32_t));
    descriptor_set.UpdateDescriptorSets();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 0, binding = 0) uniform ufoo {
            int whichtest;
        } u_info;
        void main() {
            float myfloat = 3.1415f;
            int foo = -135;
            if (gl_VertexIndex == 0) {
                switch(u_info.whichtest) {
                    case 0:
                        debugPrintfEXT("Here are two float values %f, %f", 1.0, myfloat);
                        break;
                    case 1:
                        debugPrintfEXT("Here's a smaller float value %1.2f", myfloat);
                        break;
                    case 2:
                        debugPrintfEXT("Here's an integer %i with text before and after it", foo);
                        break;
                    case 3:
                        foo = 256;
                        debugPrintfEXT("Here's an integer in octal %o and hex 0x%x", foo, foo);
                        break;
                    case 4:
                        debugPrintfEXT("%d is a negative integer", foo);
                        break;
                    case 5:
                        vec4 floatvec = vec4(1.2f, 2.2f, 3.2f, 4.2f);
                        debugPrintfEXT("Here's a vector of floats %1.2v4f", floatvec);
                        break;
                    case 6:
                        debugPrintfEXT("Here's a float in sn %e", myfloat);
                        break;
                    case 7:
                        debugPrintfEXT("Here's a float in sn %1.2e", myfloat);
                        break;
                    case 8:
                        debugPrintfEXT("Here's a float in shortest %g", myfloat);
                        break;
                    case 9:
                        debugPrintfEXT("Here's a float in hex %1.9a", myfloat);
                        break;
                    case 10:
                        debugPrintfEXT("First printf with a %% and no value");
                        debugPrintfEXT("Second printf with a value %i", foo);
                        break;
                }
            }
            gl_Position = vec4(0.0);
        }
    )glsl";

    vkt::SimpleGPL pipe(*this, pipeline_layout.handle(), shader_source);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    std::vector<char const *> messages;
    messages.push_back("Here are two float values 1.000000, 3.141500");
    messages.push_back("Here's a smaller float value 3.14");
    messages.push_back("Here's an integer -135 with text before and after it");
    messages.push_back("Here's an integer in octal 400 and hex 0x100");
    messages.push_back("-135 is a negative integer");
    messages.push_back("Here's a vector of floats 1.20, 2.20, 3.20, 4.20");
    messages.push_back("Here's a float in sn 3.141500e+00");
    messages.push_back("Here's a float in sn 3.14e+00");
    messages.push_back("Here's a float in shortest 3.1415");
    messages.push_back("Here's a float in hex 0x1.921cac000p+1");
    // Two error messages have to be last in the vector
    messages.push_back("First printf with a % and no value");
    messages.push_back("Second printf with a value -135");
    for (uint32_t i = 0; i < messages.size(); i++) {
        VkDeviceAddress *data = (VkDeviceAddress *)buffer_in.memory().map();
        data[0] = i;
        buffer_in.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kInformationBit, messages[i]);
        if (10 == i) {
            m_errorMonitor->SetDesiredFailureMsg(kInformationBit, messages[i + 1]);
            i++;
        }
        m_default_queue->Submit(*m_commandBuffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDebugPrintf, GPLMultiDraw) {
    AddRequiredExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::multiDraw);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the test number
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer_in(*m_device, 8, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    descriptor_set.WriteDescriptorBufferInfo(0, buffer_in.handle(), 0, sizeof(uint32_t));
    descriptor_set.UpdateDescriptorSets();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 0, binding = 0) uniform ufoo {
            int whichtest;
        } u_info;
        void main() {
            float myfloat = 3.1415f;
            int foo = -135;
            // referencing gl_InstanceIndex appears to be required to ensure this shader runs multiple times
            // when called from vkCmdDrawMultiEXT().
            if (gl_VertexIndex == 0 && gl_InstanceIndex < 10000) {
                switch(u_info.whichtest) {
                    case 0:
                        debugPrintfEXT("Here are two float values %f, %f", 1.0, myfloat);
                        break;
                    case 1:
                        debugPrintfEXT("Here's a smaller float value %1.2f", myfloat);
                        break;
                }
            }
            gl_Position = vec4(0.0);
        }
    )glsl";
    vkt::SimpleGPL pipe(*this, pipeline_layout.handle(), shader_source);

    VkMultiDrawInfoEXT multi_draws[3] = {};
    multi_draws[0].vertexCount = multi_draws[1].vertexCount = multi_draws[2].vertexCount = 3;
    VkMultiDrawIndexedInfoEXT multi_draw_indices[3] = {};
    multi_draw_indices[0].indexCount = multi_draw_indices[1].indexCount = multi_draw_indices[2].indexCount = 3;
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDrawMultiEXT(m_commandBuffer->handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    VkDeviceAddress *data = (VkDeviceAddress *)buffer_in.memory().map();
    data[0] = 0;
    buffer_in.memory().unmap();
    for (auto i = 0; i < 3; i++) {
        m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Here are two float values 1.000000, 3.141500");
    }
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    uint16_t *ptr = static_cast<uint16_t *>(buffer.memory().map());
    ptr[0] = 0;
    ptr[1] = 1;
    ptr[2] = 2;
    buffer.memory().unmap();
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), buffer.handle(), 0, VK_INDEX_TYPE_UINT16);
    vk::CmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    data = (VkDeviceAddress *)buffer_in.memory().map();
    data[0] = 1;
    buffer_in.memory().unmap();
    for (auto i = 0; i < 3; i++) {
        m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Here's a smaller float value 3.14");
    }
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, GPLInt64) {
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    if (!m_device->phy().features().shaderInt64) {
        GTEST_SKIP() << "shaderInt64 not supported";
    }
    InitRenderTarget();

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer_in(*m_device, 8, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer_in.handle(), 0, sizeof(uint32_t));
    descriptor_set.UpdateDescriptorSets();

    char const *shader_source_int64 = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        #extension GL_ARB_gpu_shader_int64 : enable
        layout(set = 0, binding = 0) uniform ufoo {
            int whichtest;
        } u_info;
        void main() {
            uint64_t bigvar = 0x2000000000000001ul;
            if (gl_VertexIndex == 0) {
                switch(u_info.whichtest) {
                    case 0:
                        debugPrintfEXT("Here's an unsigned long 0x%ul", bigvar);
                        break;
                    case 1:
                        u64vec4 vecul = u64vec4(bigvar, bigvar, bigvar, bigvar);
                        debugPrintfEXT("Here's a vector of ul %v4ul", vecul);
                        break;
                    case 2:
                        debugPrintfEXT("Unsigned long as decimal %lu and as hex 0x%lx", bigvar, bigvar);
                        break;
                }
            }
            gl_Position = vec4(0.0);
        }
    )glsl";

    vkt::SimpleGPL pipe(*this, pipeline_layout.handle(), shader_source_int64);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    VkDeviceAddress *data = (VkDeviceAddress *)buffer_in.memory().map();
    data[0] = 0;
    buffer_in.memory().unmap();
    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Here's an unsigned long 0x2000000000000001");
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    data = (VkDeviceAddress *)buffer_in.memory().map();
    data[0] = 1;
    buffer_in.memory().unmap();
    m_errorMonitor->SetDesiredFailureMsg(
        kInformationBit, "Here's a vector of ul 2000000000000001, 2000000000000001, 2000000000000001, 2000000000000001");
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    data = (VkDeviceAddress *)buffer_in.memory().map();
    data[0] = 2;
    buffer_in.memory().unmap();
    m_errorMonitor->SetDesiredFailureMsg(kInformationBit,
                                         "Unsigned long as decimal 2305843009213693953 and as hex 0x2000000000000001");
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, GPLFragment) {
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkDeviceSize buffer_size = 4;
    vkt::Buffer vs_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, reqs);
    vkt::Buffer fs_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, reqs);

    OneOffDescriptorSet vertex_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}});
    OneOffDescriptorSet fragment_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});

    // "Normal" sets
    const vkt::PipelineLayout pipeline_layout(*m_device, {&vertex_set.layout_, &fragment_set.layout_});
    vertex_set.WriteDescriptorBufferInfo(0, vs_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    vertex_set.UpdateDescriptorSets();
    fragment_set.WriteDescriptorBufferInfo(0, fs_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    fragment_set.UpdateDescriptorSets();

    {
        vvl::span<uint32_t> vert_data(static_cast<uint32_t *>(vs_buffer.memory().map()),
                                      static_cast<uint32_t>(buffer_size) / sizeof(uint32_t));
        for (auto &v : vert_data) {
            v = 0x01030507;
        }
        vs_buffer.memory().unmap();
    }
    {
        vvl::span<uint32_t> frag_data(static_cast<uint32_t *>(fs_buffer.memory().map()),
                                      static_cast<uint32_t>(buffer_size) / sizeof(uint32_t));
        for (auto &v : frag_data) {
            v = 0x02040608;
        }
        fs_buffer.memory().unmap();
    }

    const std::array<VkDescriptorSet, 2> desc_sets = {vertex_set.set_, fragment_set.set_};

    static const char vert_shader[] = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 0, binding = 0) buffer Input { uint u_buffer[]; } v_in; // texel_buffer[4]
        const vec2 vertices[3] = vec2[](
            vec2(-1.0, -1.0),
            vec2(1.0, -1.0),
            vec2(0.0, 1.0)
        );
        void main() {
            if (gl_VertexIndex == 0) {
                const uint t = v_in.u_buffer[0];
                debugPrintfEXT("Vertex shader %i, 0x%x", gl_VertexIndex, t);
            }
            gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
        }
    )glsl";

    static const char frag_shader[] = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 1, binding = 0) buffer Input { uint u_buffer[]; } f_in; // texel_buffer[4]
        layout(location = 0) out vec4 c_out;
        void main() {
            c_out = vec4(1.0);
            const uint t = f_in.u_buffer[0];
            debugPrintfEXT("Fragment shader 0x%x\n", t);
        }
    )glsl";

    vkt::SimpleGPL pipe(*this, pipeline_layout.handle(), vert_shader, frag_shader);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0,
                              static_cast<uint32_t>(desc_sets.size()), desc_sets.data(), 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Vertex shader 0, 0x1030507");
    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Fragment shader 0x2040608");
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, GPLFragmentIndependentSets) {
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkDeviceSize buffer_size = 4;
    vkt::Buffer vs_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, reqs);
    vkt::Buffer fs_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, reqs);

    OneOffDescriptorSet vertex_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}});
    OneOffDescriptorSet fragment_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});

    // Independent sets
    const vkt::PipelineLayout pipeline_layout_vs(*m_device, {&vertex_set.layout_, nullptr}, {},
                                                 VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const auto vs_layout = pipeline_layout_vs.handle();
    const vkt::PipelineLayout pipeline_layout_fs(*m_device, {nullptr, &fragment_set.layout_}, {},
                                                 VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const auto fs_layout = pipeline_layout_fs.handle();
    const vkt::PipelineLayout pipeline_layout(*m_device, {&vertex_set.layout_, &fragment_set.layout_}, {},
                                              VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const auto layout = pipeline_layout.handle();

    vertex_set.WriteDescriptorBufferInfo(0, vs_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    vertex_set.UpdateDescriptorSets();
    fragment_set.WriteDescriptorBufferInfo(0, fs_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    fragment_set.UpdateDescriptorSets();

    {
        vvl::span<uint32_t> vert_data(static_cast<uint32_t *>(vs_buffer.memory().map()),
                                      static_cast<uint32_t>(buffer_size) / sizeof(uint32_t));
        for (auto &v : vert_data) {
            v = 0x01030507;
        }
        vs_buffer.memory().unmap();
    }
    {
        vvl::span<uint32_t> frag_data(static_cast<uint32_t *>(fs_buffer.memory().map()),
                                      static_cast<uint32_t>(buffer_size) / sizeof(uint32_t));
        for (auto &v : frag_data) {
            v = 0x02040608;
        }
        fs_buffer.memory().unmap();
    }

    const std::array<VkDescriptorSet, 2> desc_sets = {vertex_set.set_, fragment_set.set_};

    CreatePipelineHelper vertex_input_lib(*this);
    vertex_input_lib.InitVertexInputLibInfo();
    vertex_input_lib.CreateGraphicsPipeline(false);

    static const char vertshader[] = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 0, binding = 0) buffer Input { uint u_buffer[]; } v_in; // texel_buffer[4]
        const vec2 vertices[3] = vec2[](
            vec2(-1.0, -1.0),
            vec2(1.0, -1.0),
            vec2(0.0, 1.0)
        );
        void main() {
            if (gl_VertexIndex == 0) {
                const uint t = v_in.u_buffer[0];
                debugPrintfEXT("Vertex shader %i, 0x%x", gl_VertexIndex, t);
            }
            gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
        }
    )glsl";
    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vertshader);
    vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT);

    VkViewport viewport = {0, 0, 1, 1, 0, 1};
    VkRect2D scissor = {{0, 0}, {1, 1}};
    CreatePipelineHelper pre_raster_lib(*this);
    pre_raster_lib.InitPreRasterLibInfo(&vs_stage.stage_ci);
    pre_raster_lib.vp_state_ci_.pViewports = &viewport;
    pre_raster_lib.vp_state_ci_.pScissors = &scissor;
    pre_raster_lib.gp_ci_.layout = vs_layout;
    pre_raster_lib.CreateGraphicsPipeline(false);

    static const char frag_shader[] = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 1, binding = 0) buffer Input { uint u_buffer[]; } f_in; // texel_buffer[4]
        layout(location = 0) out vec4 c_out;
        void main() {
            c_out = vec4(1.0);
            const uint t = f_in.u_buffer[0];
            debugPrintfEXT("Fragment shader 0x%x\n", t);
        }
    )glsl";
    const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, frag_shader);
    vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper frag_shader_lib(*this);
    frag_shader_lib.InitFragmentLibInfo(&fs_stage.stage_ci);
    frag_shader_lib.gp_ci_.layout = fs_layout;
    frag_shader_lib.CreateGraphicsPipeline(false);

    CreatePipelineHelper frag_out_lib(*this);
    frag_out_lib.InitFragmentOutputLibInfo();
    frag_out_lib.CreateGraphicsPipeline(false);

    VkPipeline libraries[4] = {
        vertex_input_lib.Handle(),
        pre_raster_lib.Handle(),
        frag_shader_lib.Handle(),
        frag_out_lib.Handle(),
    };
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper();
    link_info.libraryCount = size(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = pre_raster_lib.gp_ci_.layout;
    vkt::Pipeline pipe(*m_device, exe_pipe_ci);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0,
                              static_cast<uint32_t>(desc_sets.size()), desc_sets.data(), 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Vertex shader 0, 0x1030507");
    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Fragment shader 0x2040608");
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, BasicUsageShaderObjects) {
    TEST_DESCRIPTION("Verify that calls to debugPrintfEXT are received in debug stream");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper();
    VkPhysicalDeviceShaderObjectFeaturesEXT shader_object_features = vku::InitStructHelper(&dynamic_rendering_features);
    VkPhysicalDeviceMultiDrawFeaturesEXT multi_draw_features = vku::InitStructHelper(&shader_object_features);
    GetPhysicalDeviceFeatures2(multi_draw_features);
    RETURN_IF_SKIP(InitState(nullptr, &multi_draw_features));

    InitDynamicRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the test number
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer_in(*m_device, 8, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    descriptor_set.WriteDescriptorBufferInfo(0, buffer_in.handle(), 0, sizeof(uint32_t));
    descriptor_set.UpdateDescriptorSets();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout(set = 0, binding = 0) uniform ufoo {
            int whichtest;
        } u_info;
        void main() {
            float myfloat = 3.1415f;
            int foo = -135;
            // referencing gl_InstanceIndex appears to be required to ensure this shader runs multiple times
            // when called from vkCmdDrawMultiEXT().
            if (gl_VertexIndex == 0 && gl_InstanceIndex < 10000) {
                switch(u_info.whichtest) {
                    case 0:
                        debugPrintfEXT("Here are two float values %f, %f", 1.0, myfloat);
                        break;
                    case 1:
                        debugPrintfEXT("Here's a smaller float value %1.2f", myfloat);
                        break;
                    case 2:
                        debugPrintfEXT("Here's an integer %i with text before and after it", foo);
                        break;
                    case 3:
                        foo = 256;
                        debugPrintfEXT("Here's an integer in octal %o and hex 0x%x", foo, foo);
                        break;
                    case 4:
                        debugPrintfEXT("%d is a negative integer", foo);
                        break;
                    case 5:
                        vec4 floatvec = vec4(1.2f, 2.2f, 3.2f, 4.2f);
                        debugPrintfEXT("Here's a vector of floats %1.2v4f", floatvec);
                        break;
                    case 6:
                        debugPrintfEXT("Here's a float in sn %e", myfloat);
                        break;
                    case 7:
                        debugPrintfEXT("Here's a float in sn %1.2e", myfloat);
                        break;
                    case 8:
                        debugPrintfEXT("Here's a float in shortest %g", myfloat);
                        break;
                    case 9:
                        debugPrintfEXT("Here's a float in hex %1.9a", myfloat);
                        break;
                    case 10:
                        debugPrintfEXT("First printf with a %% and no value");
                        debugPrintfEXT("Second printf with a value %i", foo);
                        break;
                }
            }
            gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
        }
        )glsl";
    std::vector<char const *> messages;
    messages.push_back("Here are two float values 1.000000, 3.141500");
    messages.push_back("Here's a smaller float value 3.14");
    messages.push_back("Here's an integer -135 with text before and after it");
    messages.push_back("Here's an integer in octal 400 and hex 0x100");
    messages.push_back("-135 is a negative integer");
    messages.push_back("Here's a vector of floats 1.20, 2.20, 3.20, 4.20");
    messages.push_back("Here's a float in sn 3.141500e+00");
    messages.push_back("Here's a float in sn 3.14e+00");
    messages.push_back("Here's a float in shortest 3.1415");
    messages.push_back("Here's a float in hex 0x1.921cac000p+1");
    // Two error messages have to be last in the vector
    messages.push_back("First printf with a % and no value");
    messages.push_back("Second printf with a value -135");

    const vkt::Shader vs(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, shader_source),
                         &descriptor_set.layout_.handle());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
    {
        const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                                VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                                VK_SHADER_STAGE_FRAGMENT_BIT};
        const VkShaderEXT shaders[] = {vs.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE};
        vk::CmdBindShadersEXT(m_commandBuffer->handle(), 5u, stages, shaders);
        SetDefaultDynamicStatesAll(m_commandBuffer->handle());
    }
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    for (uint32_t i = 0; i < messages.size(); i++) {
        VkDeviceAddress *data = (VkDeviceAddress *)buffer_in.memory().map();
        data[0] = i;
        buffer_in.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kInformationBit, messages[i]);
        if (10 == i) {
            m_errorMonitor->SetDesiredFailureMsg(kInformationBit, messages[i + 1]);
            i++;
        }
        m_default_queue->Submit(*m_commandBuffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }

    if (multi_draw_features.multiDraw) {
        VkMultiDrawInfoEXT multi_draws[3] = {};
        multi_draws[0].vertexCount = multi_draws[1].vertexCount = multi_draws[2].vertexCount = 3;
        VkMultiDrawIndexedInfoEXT multi_draw_indices[3] = {};
        multi_draw_indices[0].indexCount = multi_draw_indices[1].indexCount = multi_draw_indices[2].indexCount = 3;
        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
        {
            const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                                    VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                                    VK_SHADER_STAGE_FRAGMENT_BIT};
            const VkShaderEXT shaders[] = {vs.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE};
            vk::CmdBindShadersEXT(m_commandBuffer->handle(), 5u, stages, shaders);
            SetDefaultDynamicStatesAll(m_commandBuffer->handle());
        }
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        vk::CmdDrawMultiEXT(m_commandBuffer->handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT));
        m_commandBuffer->EndRendering();
        m_commandBuffer->end();

        VkDeviceAddress *data = (VkDeviceAddress *)buffer_in.memory().map();
        data[0] = 0;
        buffer_in.memory().unmap();
        for (auto i = 0; i < 3; i++) {
            m_errorMonitor->SetDesiredFailureMsg(kInformationBit, messages[0]);
        }
        m_default_queue->Submit(*m_commandBuffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();

        vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        uint16_t *ptr = static_cast<uint16_t *>(buffer.memory().map());
        ptr[0] = 0;
        ptr[1] = 1;
        ptr[2] = 2;
        buffer.memory().unmap();
        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
        {
            const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                                    VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                                    VK_SHADER_STAGE_FRAGMENT_BIT};
            const VkShaderEXT shaders[] = {vs.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE};
            vk::CmdBindShadersEXT(m_commandBuffer->handle(), 5u, stages, shaders);
            SetDefaultDynamicStatesAll(m_commandBuffer->handle());
        }
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        vk::CmdBindIndexBuffer(m_commandBuffer->handle(), buffer.handle(), 0, VK_INDEX_TYPE_UINT16);
        vk::CmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
        m_commandBuffer->EndRendering();
        m_commandBuffer->end();

        data = (VkDeviceAddress *)buffer_in.memory().map();
        data[0] = 1;
        buffer_in.memory().unmap();
        for (auto i = 0; i < 3; i++) {
            m_errorMonitor->SetDesiredFailureMsg(kInformationBit, messages[1]);
        }
        m_default_queue->Submit(*m_commandBuffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }

    if (m_device->phy().features().shaderInt64) {
        char const *shader_source_int64 = R"glsl(
            #version 450
            #extension GL_EXT_debug_printf : enable
            #extension GL_ARB_gpu_shader_int64 : enable
            layout(set = 0, binding = 0) uniform ufoo {
                int whichtest;
            } u_info;
            void main() {
                uint64_t bigvar = 0x2000000000000001ul;
                if (gl_VertexIndex == 0) {
                    switch(u_info.whichtest) {
                        case 0:
                            debugPrintfEXT("Here's an unsigned long 0x%ul", bigvar);
                            break;
                        case 1:
                            u64vec4 vecul = u64vec4(bigvar, bigvar, bigvar, bigvar);
                            debugPrintfEXT("Here's a vector of ul %v4ul", vecul);
                            break;
                        case 2:
                            debugPrintfEXT("Unsigned long as decimal %lu and as hex 0x%lx", bigvar, bigvar);
                            break;
                    }
                }
                gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
            }
            )glsl";
        vkt::Shader vs_int64(*m_device, VK_SHADER_STAGE_VERTEX_BIT, GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, shader_source_int64),
                             &descriptor_set.layout_.handle());

        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());
        {
            const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                                    VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                                    VK_SHADER_STAGE_FRAGMENT_BIT};
            const VkShaderEXT shaders[] = {vs_int64.handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE};
            vk::CmdBindShadersEXT(m_commandBuffer->handle(), 5u, stages, shaders);
            SetDefaultDynamicStatesAll(m_commandBuffer->handle());
        }
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
        m_commandBuffer->EndRendering();
        m_commandBuffer->end();

        VkDeviceAddress *data = (VkDeviceAddress *)buffer_in.memory().map();
        data[0] = 0;
        buffer_in.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Here's an unsigned long 0x2000000000000001");
        m_default_queue->Submit(*m_commandBuffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();

        data = (VkDeviceAddress *)buffer_in.memory().map();
        data[0] = 1;
        buffer_in.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(
            kInformationBit, "Here's a vector of ul 2000000000000001, 2000000000000001, 2000000000000001, 2000000000000001");
        m_default_queue->Submit(*m_commandBuffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();

        data = (VkDeviceAddress *)buffer_in.memory().map();
        data[0] = 2;
        buffer_in.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kInformationBit,
                                             "Unsigned long as decimal 2305843009213693953 and as hex 0x2000000000000001");
        m_default_queue->Submit(*m_commandBuffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDebugPrintf, MeshTaskShaderObjects) {
    TEST_DESCRIPTION("Test debug printf in mesh and task shader objects.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    RETURN_IF_SKIP(InitDebugPrintfFramework());

    // Create a device that enables mesh_shader
    VkPhysicalDeviceMaintenance4Features maintenance_4_features = vku::InitStructHelper();
    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = vku::InitStructHelper(&maintenance_4_features);
    VkPhysicalDeviceShaderObjectFeaturesEXT shader_object_features = vku::InitStructHelper(&dynamic_rendering_features);
    VkPhysicalDeviceMultiviewFeaturesKHR multiview_features = vku::InitStructHelper(&shader_object_features);
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR shading_rate_features = vku::InitStructHelper(&multiview_features);
    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = vku::InitStructHelper(&shading_rate_features);
    GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (!mesh_shader_features.taskShader || !mesh_shader_features.meshShader) {
        GTEST_SKIP() << "Task or mesh shader not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &mesh_shader_features));
    InitDynamicRenderTarget();

    static const char *taskShaderText = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require // Requires SPIR-V 1.5 (Vulkan 1.2)
        #extension GL_EXT_debug_printf : enable
        layout (local_size_x=1, local_size_y=1, local_size_z=1) in;
        void main() {
            debugPrintfEXT("hello from task shader");
            EmitMeshTasksEXT(1u, 1u, 1u);
        }
    )glsl";

    static const char *meshShaderText = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require // Requires SPIR-V 1.5 (Vulkan 1.2)
        #extension GL_EXT_debug_printf : enable
        layout(max_vertices = 3, max_primitives=1) out;
        layout(triangles) out;
        void main() {
            debugPrintfEXT("hello from mesh shader");
        }
    )glsl";

    const vkt::Shader ts(*m_device, VK_SHADER_STAGE_TASK_BIT_EXT,
                         GLSLToSPV(VK_SHADER_STAGE_TASK_BIT_EXT, taskShaderText, SPV_ENV_VULKAN_1_3));
    const vkt::Shader ms(*m_device, VK_SHADER_STAGE_MESH_BIT_EXT,
                         GLSLToSPV(VK_SHADER_STAGE_MESH_BIT_EXT, meshShaderText, SPV_ENV_VULKAN_1_3));

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderingColor(GetDynamicRenderTarget(), GetRenderTargetArea());

    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                            VK_SHADER_STAGE_GEOMETRY_BIT,
                                            VK_SHADER_STAGE_FRAGMENT_BIT,
                                            VK_SHADER_STAGE_TASK_BIT_EXT,
                                            VK_SHADER_STAGE_MESH_BIT_EXT};
    const VkShaderEXT shaders[] = {VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE,
                                   VK_NULL_HANDLE, ts.handle(),    ms.handle()};
    SetDefaultDynamicStatesAll(m_commandBuffer->handle());
    vk::CmdSetRasterizerDiscardEnableEXT(m_commandBuffer->handle(), VK_TRUE);
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 7u, stages, shaders);
    vk::CmdDrawMeshTasksEXT(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "hello from task shader");
    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "hello from mesh shader");
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, VertexFragmentSeparateShader) {
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    static const char vert_shader[] = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable

        const vec2 vertices[3] = vec2[]( vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(0.0, 1.0) );
        void main() {
            debugPrintfEXT("Vertex value is %i", 4);
            gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
        }
    )glsl";
    static const char frag_shader[] = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable

        layout(location = 0) out vec4 c_out;
        void main() {
            debugPrintfEXT("Fragment value is %i", 8);
            c_out = vec4(0.0);
        }
    )glsl";
    VkShaderObj vs(this, vert_shader, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, frag_shader, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkViewport viewport = {0, 0, 1, 1, 0, 1};
    VkRect2D scissor = {{0, 0}, {1, 1}};
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.vp_state_ci_.pViewports = &viewport;
    pipe.vp_state_ci_.pScissors = &scissor;
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    for (auto i = 0; i < 3; i++) {
        m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Vertex value is 4");
    }
    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Fragment value is 8");
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

// TODO - Add multi-entry support for Debug PrintF
TEST_F(NegativeDebugPrintf, DISABLED_VertexFragmentMultiEntrypoint) {
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    // void vert_main() {
    //     debugPrintfEXT("Vertex value is %i", 4);
    //     gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
    // }
    // layout(location = 0) out vec4 c_out;
    // void frag_main() {
    //     debugPrintfEXT("Fragment value is %i", 8);
    //     c_out = vec4(0.0);
    // }
    const char *shader_source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %9 = OpExtInstImport "NonSemantic.DebugPrintf"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %frag_main "frag_main" %c_out
               OpEntryPoint Vertex %vert_main "vert_main" %_ %gl_VertexIndex
               OpExecutionMode %frag_main OriginUpperLeft
          %6 = OpString "Vertex value is %i"
               OpDecorate %c_out Location 0
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
               OpDecorate %gl_VertexIndex BuiltIn VertexIndex
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_4 = OpConstant %int 4
      %int_8 = OpConstant %int 8
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
      %int_0 = OpConstant %int 0
    %v2float = OpTypeVector %float 2
     %uint_3 = OpConstant %uint 3
%_arr_v2float_uint_3 = OpTypeArray %v2float %uint_3
   %float_n1 = OpConstant %float -1
         %24 = OpConstantComposite %v2float %float_n1 %float_n1
    %float_1 = OpConstant %float 1
         %26 = OpConstantComposite %v2float %float_1 %float_n1
    %float_0 = OpConstant %float 0
         %28 = OpConstantComposite %v2float %float_0 %float_1
         %29 = OpConstantComposite %_arr_v2float_uint_3 %24 %26 %28
%_ptr_Input_int = OpTypePointer Input %int
%gl_VertexIndex = OpVariable %_ptr_Input_int Input
      %int_3 = OpConstant %int 3
%_ptr_Function__arr_v2float_uint_3 = OpTypePointer Function %_arr_v2float_uint_3
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %c_out = OpVariable %_ptr_Output_v4float Output
         %16 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
       %vert_main = OpFunction %void None %3
          %5 = OpLabel
  %indexable = OpVariable %_ptr_Function__arr_v2float_uint_3 Function
         %10 = OpExtInst %void %9 1 %6 %int_4
         %32 = OpLoad %int %gl_VertexIndex
         %34 = OpSMod %int %32 %int_3
               OpStore %indexable %29
         %38 = OpAccessChain %_ptr_Function_v2float %indexable %34
         %39 = OpLoad %v2float %38
         %40 = OpCompositeExtract %float %39 0
         %41 = OpCompositeExtract %float %39 1
         %42 = OpCompositeConstruct %v4float %40 %41 %float_0 %float_1
         %44 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %44 %42
               OpReturn
               OpFunctionEnd
       %frag_main = OpFunction %void None %3
          %f5 = OpLabel
         %f10 = OpExtInst %void %9 1 %6 %int_8
               OpStore %c_out %16
               OpReturn
               OpFunctionEnd
        )";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM, nullptr, "vert_main");
    VkShaderObj fs(this, shader_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM, nullptr, "frag_main");

    VkViewport viewport = {0, 0, 1, 1, 0, 1};
    VkRect2D scissor = {{0, 0}, {1, 1}};
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.vp_state_ci_.pViewports = &viewport;
    pipe.vp_state_ci_.pScissors = &scissor;
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    for (auto i = 0; i < 3; i++) {
        m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Vertex value is 4");
    }
    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Fragment value is 8");
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, ShaderObjectFragment) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitDynamicRenderTarget();

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";

    const vkt::Shader vert_shader(*m_device, VK_SHADER_STAGE_VERTEX_BIT,
                                  GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexDrawPassthroughGlsl));
    const vkt::Shader frag_shader(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fs_source));

    VkRenderingInfoKHR renderingInfo = vku::InitStructHelper();
    renderingInfo.colorAttachmentCount = 0;
    renderingInfo.layerCount = 1;
    renderingInfo.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(renderingInfo);
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT});
    m_commandBuffer->BindVertFragShader(vert_shader, frag_shader);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "float == 3.141500");
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, ShaderObjectCompute) {
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";
    const vkt::Shader comp_shader(*m_device, VK_SHADER_STAGE_COMPUTE_BIT, GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, cs_source));

    m_commandBuffer->begin();
    m_commandBuffer->BindCompShader(comp_shader);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "float == 3.141500");
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, SetupErrorVersion) {
    TEST_DESCRIPTION("Verify DebugPrintF can gracefully fail if not using Vulkan 1.1+");
    SetTargetApiVersion(VK_API_VERSION_1_0);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);

    VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT};
    VkValidationFeatureDisableEXT disables[] = {
        VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT, VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT,
        VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT, VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};
    VkValidationFeaturesEXT features = vku::InitStructHelper();
    features.enabledValidationFeatureCount = 1;
    features.disabledValidationFeatureCount = 4;
    features.pEnabledValidationFeatures = enables;
    features.pDisabledValidationFeatures = disables;
    RETURN_IF_SKIP(InitFramework(&features));

    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        GTEST_SKIP() << "Currently disabled for Portability";
    }

    m_errorMonitor->SetDesiredError("Debug Printf requires Vulkan 1.1 or later");
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->VerifyFound();

    // Still make sure we can use Vulkan as expected without errors

    InitRenderTarget();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
        )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
}

TEST_F(NegativeDebugPrintf, LocalSizeId) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::maintenance4);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
         %30 = OpExtInstImport "NonSemantic.DebugPrintf"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %gl_GlobalInvocationID
               OpExecutionModeId %main LocalSizeId %8 %9 %10
         %29 = OpString "TEST"
               OpDecorate %8 SpecId 0
               OpDecorate %9 SpecId 1
               OpDecorate %10 SpecId 2
               OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
          %8 = OpSpecConstant %uint 1
          %9 = OpSpecConstant %uint 1
         %10 = OpSpecConstant %uint 1
       %bool = OpTypeBool
     %v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
     %uint_0 = OpConstant %uint 0
%_ptr_Input_uint = OpTypePointer Input %uint
     %uint_1 = OpConstant %uint 1
       %main = OpFunction %void None %4
          %6 = OpLabel
         %17 = OpAccessChain %_ptr_Input_uint %gl_GlobalInvocationID %uint_0
         %18 = OpLoad %uint %17
         %19 = OpIEqual %bool %18 %uint_0
               OpSelectionMerge %21 None
               OpBranchConditional %19 %20 %21
         %20 = OpLabel
         %23 = OpAccessChain %_ptr_Input_uint %gl_GlobalInvocationID %uint_1
         %24 = OpLoad %uint %23
         %25 = OpIEqual %bool %24 %uint_0
               OpBranch %21
         %21 = OpLabel
         %26 = OpPhi %bool %19 %6 %25 %20
               OpSelectionMerge %28 None
               OpBranchConditional %26 %27 %28
         %27 = OpLabel
         %31 = OpExtInst %void %30 1 %29
               OpBranch %28
         %28 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    uint32_t workgroup_size[3] = {32, 32, 1};
    VkSpecializationMapEntry entries[3];
    entries[0] = {0, 0, sizeof(uint32_t)};
    entries[1] = {1, sizeof(uint32_t), sizeof(uint32_t)};
    entries[2] = {2, sizeof(uint32_t) * 2, sizeof(uint32_t)};

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 3;
    specialization_info.pMapEntries = entries;
    specialization_info.dataSize = sizeof(uint32_t) * 3;
    specialization_info.pData = workgroup_size;

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM,
                                             &specialization_info);
    pipe.CreateComputePipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_commandBuffer->handle(), 32, 32, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "TEST");
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, Maintenance5) {
    TEST_DESCRIPTION("Test SPIRV is still checked if using new pNext in VkPipelineShaderStageCreateInfo");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";

    std::vector<uint32_t> shader;
    this->GLSLtoSPV(&m_device->phy().limits_, VK_SHADER_STAGE_COMPUTE_BIT, shader_source, shader);

    VkShaderModuleCreateInfo module_create_info = vku::InitStructHelper();
    module_create_info.pCode = shader.data();
    module_create_info.codeSize = shader.size() * sizeof(uint32_t);

    VkPipelineShaderStageCreateInfo stage_ci = vku::InitStructHelper(&module_create_info);
    stage_ci.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stage_ci.module = VK_NULL_HANDLE;
    stage_ci.pName = "main";

    vkt::PipelineLayout layout(*m_device, {});
    CreateComputePipelineHelper pipe(*this);
    pipe.cp_ci_.stage = stage_ci;
    pipe.cp_ci_.layout = layout.handle();
    pipe.CreateComputePipeline(false);

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "float == 3.141500");
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugPrintf, UseAllDescriptorSlotsPipelineReserved) {
    TEST_DESCRIPTION("Reserve a descriptor slot and proceed to use them all anyway so debug printf can't");
    RETURN_IF_SKIP(InitDebugPrintfFramework(nullptr, true));
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit | kInformationBit);

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";

    // Add one to use the descriptor slot we tried to reserve
    const uint32_t set_limit = m_device->phy().limits_.maxBoundDescriptorSets + 1;
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    // First try to use too many sets in the pipeline layout
    {
        m_errorMonitor->SetDesiredWarning(
            "This Pipeline Layout has too many descriptor sets that will not allow GPU shader instrumentation to be setup for "
            "pipelines created with it");
        std::vector<const vkt::DescriptorSetLayout *> layouts(set_limit);
        for (uint32_t i = 0; i < set_limit; i++) {
            layouts[i] = &descriptor_set.layout_;
        }
        vkt::PipelineLayout pipe_layout(*m_device, layouts);
        m_errorMonitor->VerifyFound();

        CreateComputePipelineHelper pipe(*this);
        pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
        pipe.cp_ci_.layout = pipe_layout.handle();
        pipe.CreateComputePipeline();

        m_commandBuffer->begin();
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
        vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
        m_commandBuffer->end();

        // Will not print out because no slot was possible to put output buffer
        m_default_queue->Submit(*m_commandBuffer);
        m_default_queue->Wait();
    }

    // Reduce by one (so there is room now) and print something
    {
        std::vector<const vkt::DescriptorSetLayout *> layouts(set_limit - 1);
        for (uint32_t i = 0; i < set_limit - 1; i++) {
            layouts[i] = &descriptor_set.layout_;
        }
        vkt::PipelineLayout pipe_layout(*m_device, layouts);

        CreateComputePipelineHelper pipe(*this);
        pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
        pipe.cp_ci_.layout = pipe_layout.handle();
        pipe.CreateComputePipeline();

        m_commandBuffer->begin();
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
        vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
        m_commandBuffer->end();

        m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "float == 3.141500");
        m_default_queue->Submit(*m_commandBuffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDebugPrintf, UseAllDescriptorSlotsPipelineNotReserved) {
    TEST_DESCRIPTION("Do not reserve a descriptor slot and proceed to use them all anyway so debug printf can't");
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit | kInformationBit);

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";

    const uint32_t set_limit = m_device->phy().limits_.maxBoundDescriptorSets;
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    // First try to use too many sets in the pipeline layout
    {
        m_errorMonitor->SetDesiredWarning(
            "This Pipeline Layout has too many descriptor sets that will not allow GPU shader instrumentation to be setup for "
            "pipelines created with it");
        std::vector<const vkt::DescriptorSetLayout *> layouts(set_limit);
        for (uint32_t i = 0; i < set_limit; i++) {
            layouts[i] = &descriptor_set.layout_;
        }
        vkt::PipelineLayout pipe_layout(*m_device, layouts);
        m_errorMonitor->VerifyFound();

        CreateComputePipelineHelper pipe(*this);
        pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
        pipe.cp_ci_.layout = pipe_layout.handle();
        pipe.CreateComputePipeline();

        m_commandBuffer->begin();
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
        vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
        m_commandBuffer->end();

        // Will not print out because no slot was possible to put output buffer
        m_default_queue->Submit(*m_commandBuffer);
        m_default_queue->Wait();
    }

    // Reduce by one (so there is room now) and print something
    {
        std::vector<const vkt::DescriptorSetLayout *> layouts(set_limit - 1);
        for (uint32_t i = 0; i < set_limit - 1; i++) {
            layouts[i] = &descriptor_set.layout_;
        }
        vkt::PipelineLayout pipe_layout(*m_device, layouts);

        CreateComputePipelineHelper pipe(*this);
        pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT);
        pipe.cp_ci_.layout = pipe_layout.handle();
        pipe.CreateComputePipeline();

        m_commandBuffer->begin();
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
        vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
        m_commandBuffer->end();

        m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "float == 3.141500");
        m_default_queue->Submit(*m_commandBuffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }
}

// TODO - https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7178
TEST_F(NegativeDebugPrintf, DISABLED_UseAllDescriptorSlotsShaderObjectReserved) {
    TEST_DESCRIPTION("Reserve a descriptor slot and proceed to use them all anyway so debug printf can't");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitDebugPrintfFramework(nullptr, true));
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit | kInformationBit);

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";
    auto cs_spirv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, shader_source);

    // Add one to use the descriptor slot we tried to reserve
    const uint32_t set_limit = m_device->phy().limits_.maxBoundDescriptorSets + 1;
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    std::vector<VkDescriptorSetLayout> layouts;
    for (uint32_t i = 0; i < set_limit; i++) {
        layouts.push_back(descriptor_set.layout_.handle());
    }

    // First try to use too many sets in the Shader Object
    {
        m_errorMonitor->SetDesiredWarning(
            "This Shader Object has too many descriptor sets that will not allow GPU shader instrumentation to be setup for "
            "VkShaderEXT created with it");

        const vkt::Shader comp_shader(*m_device,
                                      ShaderCreateInfo(cs_spirv, VK_SHADER_STAGE_COMPUTE_BIT, set_limit, layouts.data()));
        m_errorMonitor->VerifyFound();

        m_commandBuffer->begin();
        m_commandBuffer->BindCompShader(comp_shader);
        vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
        m_commandBuffer->end();

        // Will not print out because no slot was possible to put output buffer
        m_default_queue->Submit(*m_commandBuffer);
        m_default_queue->Wait();
    }

    // Reduce by one (so there is room now) and print something
    {
        const vkt::Shader comp_shader(*m_device,
                                      ShaderCreateInfo(cs_spirv, VK_SHADER_STAGE_COMPUTE_BIT, set_limit - 1, layouts.data()));

        m_commandBuffer->begin();
        m_commandBuffer->BindCompShader(comp_shader);
        vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
        m_commandBuffer->end();

        m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "float == 3.141500");
        m_default_queue->Submit(*m_commandBuffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }
}

// TODO - https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7178
TEST_F(NegativeDebugPrintf, DISABLED_UseAllDescriptorSlotsShaderObjectNotReserved) {
    TEST_DESCRIPTION("Dont reserve a descriptor slot and proceed to use them all anyway so debug printf can't");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit | kInformationBit);

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";
    auto cs_spirv = GLSLToSPV(VK_SHADER_STAGE_COMPUTE_BIT, shader_source);

    const uint32_t set_limit = m_device->phy().limits_.maxBoundDescriptorSets;
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    std::vector<VkDescriptorSetLayout> layouts;
    for (uint32_t i = 0; i < set_limit; i++) {
        layouts.push_back(descriptor_set.layout_.handle());
    }

    // First try to use too many sets in the Shader Object
    {
        m_errorMonitor->SetDesiredWarning(
            "This Shader Object has too many descriptor sets that will not allow GPU shader instrumentation to be setup for "
            "VkShaderEXT created with it");

        const vkt::Shader comp_shader(*m_device,
                                      ShaderCreateInfo(cs_spirv, VK_SHADER_STAGE_COMPUTE_BIT, set_limit, layouts.data()));
        m_errorMonitor->VerifyFound();

        m_commandBuffer->begin();
        m_commandBuffer->BindCompShader(comp_shader);
        vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
        m_commandBuffer->end();

        // Will not print out because no slot was possible to put output buffer
        m_default_queue->Submit(*m_commandBuffer);
        m_default_queue->Wait();
    }

    // Reduce by one (so there is room now) and print something
    {
        const vkt::Shader comp_shader(*m_device,
                                      ShaderCreateInfo(cs_spirv, VK_SHADER_STAGE_COMPUTE_BIT, set_limit - 1, layouts.data()));

        m_commandBuffer->begin();
        m_commandBuffer->BindCompShader(comp_shader);
        vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
        m_commandBuffer->end();

        m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "float == 3.141500");
        m_default_queue->Submit(*m_commandBuffer);
        m_default_queue->Wait();
        m_errorMonitor->VerifyFound();
    }
}

// TODO - https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7178
TEST_F(NegativeDebugPrintf, DISABLED_ShaderObjectMultiCreate) {
    TEST_DESCRIPTION("Make sure we instrument every index of VkShaderCreateInfoEXT");
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::shaderObject);
    RETURN_IF_SKIP(InitDebugPrintfFramework());
    RETURN_IF_SKIP(InitState());
    InitDynamicRenderTarget();

    char const *fs_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        void main() {
            float myfloat = 3.1415f;
            debugPrintfEXT("float == %f", myfloat);
        }
    )glsl";

    const auto vert_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, kVertexDrawPassthroughGlsl);
    const auto frag_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fs_source);

    VkShaderCreateInfoEXT shader_create_infos[2];
    shader_create_infos[0] = ShaderCreateInfoLink(vert_spv, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
    shader_create_infos[1] = ShaderCreateInfoLink(frag_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkShaderEXT shaders[2];
    vk::CreateShadersEXT(m_device->handle(), 2, shader_create_infos, nullptr, shaders);

    VkRenderingInfoKHR renderingInfo = vku::InitStructHelper();
    renderingInfo.colorAttachmentCount = 0;
    renderingInfo.layerCount = 1;
    renderingInfo.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(renderingInfo);
    SetDefaultDynamicStatesExclude({VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT});
    const VkShaderStageFlagBits stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    vk::CmdBindShadersEXT(m_commandBuffer->handle(), 2, stages, shaders);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "float == 3.141500");
    m_default_queue->Submit(*m_commandBuffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();

    for (uint32_t i = 0; i < 2; ++i) {
        vk::DestroyShaderEXT(m_device->handle(), shaders[i], nullptr);
    }
}