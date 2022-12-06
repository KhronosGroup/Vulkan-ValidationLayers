/*
 * Copyright (c) 2020-2022 The Khronos Group Inc.
 * Copyright (c) 2020-2022 Valve Corporation
 * Copyright (c) 2020-2022 LunarG, Inc.
 * Copyright (c) 2020-2022 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: Nathaniel Cesario <nathaniel@lunarg.com>
 */

#include "layer_validation_tests.h"

void VkDebugPrintfTest::InitDebugPrintfFramework() {
    VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT};
    VkValidationFeatureDisableEXT disables[] = {
        VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT, VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT,
        VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT, VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};
    VkValidationFeaturesEXT features = LvlInitStruct<VkValidationFeaturesEXT>();
    features.enabledValidationFeatureCount = 1;
    features.disabledValidationFeatureCount = 4;
    features.pEnabledValidationFeatures = enables;
    features.pDisabledValidationFeatures = disables;

    InitFramework(m_errorMonitor, &features);
}

TEST_F(VkDebugPrintfTest, GpuDebugPrintf) {
    TEST_DESCRIPTION("Verify that calls to debugPrintfEXT are received in debug stream");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    InitDebugPrintfFramework();
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto multi_draw_features = LvlInitStruct<VkPhysicalDeviceMultiDrawFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(multi_draw_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto features = m_device->phy().features();
    if (!features.vertexPipelineStoresAndAtomics || !features.fragmentStoresAndAtomics) {
        GTEST_SKIP() << "GPU-Assisted printf test requires vertexPipelineStoresAndAtomics and fragmentStoresAndAtomics";
    }
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD, GPU-Assisted validation test requires a driver that can draw";
    }
    // Make a uniform buffer to be passed to the shader that contains the test number
    uint32_t qfi = 0;
    VkBufferCreateInfo bci = LvlInitStruct<VkBufferCreateInfo>();
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bci.size = 8;
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    VkBufferObj buffer0;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer0.init(*m_device, bci, mem_props);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});
    VkDescriptorBufferInfo buffer_info[2] = {};
    buffer_info[0].buffer = buffer0.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    VkWriteDescriptorSet descriptor_writes[1] = {};
    descriptor_writes[0] = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    vk::UpdateDescriptorSets(m_device->device(), 1, descriptor_writes, 0, NULL);

    char const *shader_source =
        "#version 450\n"
        "#extension GL_EXT_debug_printf : enable\n"
        "layout(set = 0, binding = 0) uniform ufoo {\n"
        "    int whichtest;\n"
        "} u_info;\n"
        "void main() {\n"
        "    float myfloat = 3.1415f;\n"
        "    int foo = -135;\n"
        "    if (gl_VertexIndex == 0) {\n"
        "        switch(u_info.whichtest) {\n"
        "            case 0:\n"
        "                debugPrintfEXT(\"Here are two float values %f, %f\", 1.0, myfloat);\n"
        "                break;\n"
        "            case 1:\n"
        "                debugPrintfEXT(\"Here's a smaller float value %1.2f\", myfloat);\n"
        "                break;\n"
        "            case 2:\n"
        "                debugPrintfEXT(\"Here's an integer %i with text before and after it\", foo);\n"
        "                break;\n"
        "            case 3:\n"
        "                foo = 256;\n"
        "                debugPrintfEXT(\"Here's an integer in octal %o and hex 0x%x\", foo, foo);\n"
        "                break;\n"
        "            case 4:\n"
        "                debugPrintfEXT(\"%d is a negative integer\", foo);\n"
        "                break;\n"
        "            case 5:\n"
        "                vec4 floatvec = vec4(1.2f, 2.2f, 3.2f, 4.2f);\n"
        "                debugPrintfEXT(\"Here's a vector of floats %1.2v4f\", floatvec);\n"
        "                break;\n"
        "            case 6:\n"
        "                debugPrintfEXT(\"Here's a float in sn %e\", myfloat);\n"
        "                break;\n"
        "            case 7:\n"
        "                debugPrintfEXT(\"Here's a float in sn %1.2e\", myfloat);\n"
        "                break;\n"
        "            case 8:\n"
        "                debugPrintfEXT(\"Here's a float in shortest %g\", myfloat);\n"
        "                break;\n"
        "            case 9:\n"
        "                debugPrintfEXT(\"Here's a float in hex %1.9a\", myfloat);\n"
        "                break;\n"
        "            case 10:\n"
        "                debugPrintfEXT(\"First printf with a %% and no value\");\n"
        "                debugPrintfEXT(\"Second printf with a value %i\", foo);\n"
        "                break;\n"
        "        }\n"
        "    }\n"
        "    gl_Position = vec4(0.0, 0.0, 0.0, 0.0);\n"
        "}\n";
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
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main", true);

    VkViewport viewport = m_viewports[0];
    VkRect2D scissors = m_scissors[0];

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddDefaultColorAttachment();
    VkResult err = pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());
    ASSERT_VK_SUCCESS(err);

    VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    VkCommandBufferInheritanceInfo hinfo = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    begin_info.pInheritanceInfo = &hinfo;

    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissors);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();

    for (uint32_t i = 0; i < messages.size(); i++) {
        VkDeviceAddress *data = (VkDeviceAddress *)buffer0.memory().map();
        data[0] = i;
        buffer0.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kInformationBit, messages[i]);
        if (10 == i) {
            m_errorMonitor->SetDesiredFailureMsg(kInformationBit, messages[i + 1]);
            i++;
        }
        err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
        ASSERT_VK_SUCCESS(err);
        err = vk::QueueWaitIdle(m_device->m_queue);
        ASSERT_VK_SUCCESS(err);
        m_errorMonitor->VerifyFound();
    }

    if (multi_draw_features.multiDraw) {
        auto vkCmdDrawMultiEXT = (PFN_vkCmdDrawMultiEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawMultiEXT");
        auto vkCmdDrawMultiIndexedEXT =
            (PFN_vkCmdDrawMultiIndexedEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawMultiIndexedEXT");
        assert(vkCmdDrawMultiEXT != nullptr && vkCmdDrawMultiIndexedEXT != nullptr);
        VkMultiDrawInfoEXT multi_draws[3] = {};
        multi_draws[0].vertexCount = multi_draws[1].vertexCount = multi_draws[2].vertexCount = 3;
        VkMultiDrawIndexedInfoEXT multi_draw_indices[3] = {};
        multi_draw_indices[0].indexCount = multi_draw_indices[1].indexCount = multi_draw_indices[2].indexCount = 3;
        m_commandBuffer->begin(&begin_info);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
        vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissors);
        vkCmdDrawMultiEXT(m_commandBuffer->handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT));
        vk::CmdEndRenderPass(m_commandBuffer->handle());
        m_commandBuffer->end();

        VkDeviceAddress *data = (VkDeviceAddress *)buffer0.memory().map();
        data[0] = 0;
        buffer0.memory().unmap();
        for (auto i = 0; i < 3; i++) {
            m_errorMonitor->SetDesiredFailureMsg(kInformationBit, messages[0]);
        }
        err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
        ASSERT_VK_SUCCESS(err);
        err = vk::QueueWaitIdle(m_device->m_queue);
        ASSERT_VK_SUCCESS(err);
        m_errorMonitor->VerifyFound();

        VkBufferObj buffer;
        buffer.init(*m_device, 1024, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        uint16_t *ptr = static_cast<uint16_t *>(buffer.memory().map());
        ptr[0] = 0;
        ptr[1] = 1;
        ptr[2] = 2;
        buffer.memory().unmap();
        m_commandBuffer->begin(&begin_info);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
        vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissors);
        m_commandBuffer->BindIndexBuffer(&buffer, 0, VK_INDEX_TYPE_UINT16);
        vkCmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
        vk::CmdEndRenderPass(m_commandBuffer->handle());
        m_commandBuffer->end();

        data = (VkDeviceAddress *)buffer0.memory().map();
        data[0] = 1;
        buffer0.memory().unmap();
        for (auto i = 0; i < 3; i++) {
            m_errorMonitor->SetDesiredFailureMsg(kInformationBit, messages[1]);
        }
        err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
        ASSERT_VK_SUCCESS(err);
        err = vk::QueueWaitIdle(m_device->m_queue);
        ASSERT_VK_SUCCESS(err);
        m_errorMonitor->VerifyFound();
    }

    if (features.shaderInt64) {
        char const *shader_source_int64 =
            "#version 450\n"
            "#extension GL_EXT_debug_printf : enable\n"
            "#extension GL_ARB_gpu_shader_int64 : enable\n"
            "layout(set = 0, binding = 0) uniform ufoo {\n"
            "    int whichtest;\n"
            "} u_info;\n"
            "void main() {\n"
            "    uint64_t bigvar = 0x2000000000000001ul;\n"
            "    if (gl_VertexIndex == 0) {\n"
            "        switch(u_info.whichtest) {\n"
            "            case 0:\n"
            "                debugPrintfEXT(\"Here's an unsigned long 0x%ul\", bigvar);\n"
            "                break;\n"
            "            case 1:\n"
            "                u64vec4 vecul = u64vec4(bigvar, bigvar, bigvar, bigvar);"
            "                debugPrintfEXT(\"Here's a vector of ul %v4ul\", vecul);\n"
            "                break;\n"
            "            case 2:\n"
            "                debugPrintfEXT(\"Unsigned long as decimal %lu and as hex 0x%lx\", bigvar, bigvar);\n"
            "                break;\n"
            "        }\n"
            "    }\n"
            "    gl_Position = vec4(0.0, 0.0, 0.0, 0.0);\n"
            "}\n";
        VkShaderObj vs_int64(this, shader_source_int64, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr,
                             "main", true);
        VkPipelineObj pipe2(m_device);
        pipe2.AddShader(&vs_int64);
        pipe2.AddDefaultColorAttachment();
        err = pipe2.CreateVKPipeline(pipeline_layout.handle(), renderPass());
        ASSERT_VK_SUCCESS(err);

        m_commandBuffer->begin(&begin_info);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.handle());
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
        vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissors);
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
        vk::CmdEndRenderPass(m_commandBuffer->handle());
        m_commandBuffer->end();

        VkDeviceAddress *data = (VkDeviceAddress *)buffer0.memory().map();
        data[0] = 0;
        buffer0.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Here's an unsigned long 0x2000000000000001");
        err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
        ASSERT_VK_SUCCESS(err);
        err = vk::QueueWaitIdle(m_device->m_queue);
        ASSERT_VK_SUCCESS(err);
        m_errorMonitor->VerifyFound();

        data = (VkDeviceAddress *)buffer0.memory().map();
        data[0] = 1;
        buffer0.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(
            kInformationBit, "Here's a vector of ul 2000000000000001, 2000000000000001, 2000000000000001, 2000000000000001");
        err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
        ASSERT_VK_SUCCESS(err);
        err = vk::QueueWaitIdle(m_device->m_queue);
        ASSERT_VK_SUCCESS(err);
        m_errorMonitor->VerifyFound();

        data = (VkDeviceAddress *)buffer0.memory().map();
        data[0] = 2;
        buffer0.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kInformationBit,
                                             "Unsigned long as decimal 2305843009213693953 and as hex 0x2000000000000001");
        err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
        ASSERT_VK_SUCCESS(err);
        err = vk::QueueWaitIdle(m_device->m_queue);
        ASSERT_VK_SUCCESS(err);
        m_errorMonitor->VerifyFound();
    }
}
TEST_F(VkDebugPrintfTest, MeshTaskShadersPrintf) {
    TEST_DESCRIPTION("Test debug printf in mesh and task shaders.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    InitDebugPrintfFramework();

    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD, GPU-Assisted validation test requires a driver that can draw";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Create a device that enables mesh_shader
    auto mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesNV>();
    auto features2 = GetPhysicalDeviceFeatures2(mesh_shader_features);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    static const char taskShaderText[] =
        "#version 460\n"
        "#extension GL_NV_mesh_shader : enable\n"
        "#extension GL_EXT_debug_printf : enable\n"
        "layout(local_size_x = 32) in;\n"
        "uint invocationID = gl_LocalInvocationID.x;\n"
        "void main() {\n"
        "    if (invocationID == 0) {\n"
        "        gl_TaskCountNV = 1;\n"
        "        debugPrintfEXT(\"hello from task shader\");\n"
        "    }\n"
        "}\n";

    static const char meshShaderText[] =
        "#version 450\n"
        "#extension GL_NV_mesh_shader : require\n"
        "#extension GL_EXT_debug_printf : enable\n"
        "layout(local_size_x = 1) in;\n"
        "layout(max_vertices = 3) out;\n"
        "layout(max_primitives = 1) out;\n"
        "layout(triangles) out;\n"
        "uint invocationID = gl_LocalInvocationID.x;\n"
        "void main() {\n"
        "    if (invocationID == 0) {\n"
        "        debugPrintfEXT(\"hello from mesh shader\");\n"
        "    }\n"
        "}\n";

    VkShaderObj ts(this, taskShaderText, VK_SHADER_STAGE_TASK_BIT_NV);
    VkShaderObj ms(this, meshShaderText, VK_SHADER_STAGE_MESH_BIT_NV);
    VkPipelineLayoutObj pipeline_layout(m_device);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&ts);
    pipe.AddShader(&ms);
    pipe.AddDefaultColorAttachment();
    VkViewport viewport{};
    viewport.width = m_width;
    viewport.height = m_height;
    pipe.SetViewport({viewport});
    VkRect2D rect{};
    rect.extent.width = (uint32_t)m_width;
    rect.extent.height = (uint32_t)m_height;
    pipe.SetScissor({rect});
    VkResult err = pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());
    ASSERT_VK_SUCCESS(err);

    PFN_vkCmdDrawMeshTasksNV vkCmdDrawMeshTasksNV =
        (PFN_vkCmdDrawMeshTasksNV)vk::GetInstanceProcAddr(instance(), "vkCmdDrawMeshTasksNV");
    ASSERT_TRUE(vkCmdDrawMeshTasksNV != nullptr);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vkCmdDrawMeshTasksNV(m_commandBuffer->handle(), 1, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "hello from task shader");
    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "hello from mesh shader");
    m_commandBuffer->QueueCommandBuffer();
    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkDebugPrintfTest, GpuDebugPrintfGPL) {
    TEST_DESCRIPTION("Verify that calls to debugPrintfEXT are received in debug stream");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MULTI_DRAW_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    InitDebugPrintfFramework();
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto multi_draw_features = LvlInitStruct<VkPhysicalDeviceMultiDrawFeaturesEXT>();
    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>(&multi_draw_features);
    auto features2 = GetPhysicalDeviceFeatures2(gpl_features);
    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto features = m_device->phy().features();
    if (!features.vertexPipelineStoresAndAtomics || !features.fragmentStoresAndAtomics) {
        GTEST_SKIP() << "GPU-Assisted printf test requires vertexPipelineStoresAndAtomics and fragmentStoresAndAtomics";
    }
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD, GPU-Assisted validation test requires a driver that can draw";
    }
    // Make a uniform buffer to be passed to the shader that contains the test number
    uint32_t qfi = 0;
    VkBufferCreateInfo bci = LvlInitStruct<VkBufferCreateInfo>();
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bci.size = 8;
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    VkBufferObj buffer0;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer0.init(*m_device, bci, mem_props);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});
    VkDescriptorBufferInfo buffer_info[2] = {};
    buffer_info[0].buffer = buffer0.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    VkWriteDescriptorSet descriptor_writes[1] = {};
    descriptor_writes[0] = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    vk::UpdateDescriptorSets(m_device->device(), 1, descriptor_writes, 0, NULL);

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

    CreatePipelineHelper vi(*this);
    vi.InitVertexInputLibInfo();
    vi.InitState();
    ASSERT_VK_SUCCESS(vi.CreateGraphicsPipeline(true, false));

    const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, shader_source);
    vk_testing::GraphicsPipelineLibraryStage pre_raster_stage(layer_data::span<const uint32_t>{vs_spv});

    CreatePipelineHelper pre_raster(*this);
    pre_raster.InitPreRasterLibInfo(1, &pre_raster_stage.stage_ci);
    pre_raster.InitState();
    pre_raster.gp_ci_.layout = pipeline_layout.handle();
    pre_raster.CreateGraphicsPipeline(true, false);

    const auto render_pass = pre_raster.gp_ci_.renderPass;
    const auto subpass = pre_raster.gp_ci_.subpass;

    CreatePipelineHelper fragment(*this);
    fragment.InitFragmentLibInfo(0, nullptr);
    fragment.gp_ci_.layout = pipeline_layout.handle();
    fragment.gp_ci_.renderPass = render_pass;
    fragment.gp_ci_.subpass = subpass;
    fragment.CreateGraphicsPipeline(true, false);

    CreatePipelineHelper frag_out(*this);
    frag_out.InitFragmentOutputLibInfo();
    frag_out.gp_ci_.renderPass = render_pass;
    frag_out.gp_ci_.subpass = subpass;
    ASSERT_VK_SUCCESS(frag_out.CreateGraphicsPipeline(true, false));

    std::array<VkPipeline, 4> libraries = {
        vi.pipeline_,
        pre_raster.pipeline_,
        fragment.pipeline_,
        frag_out.pipeline_,
    };
    vk_testing::GraphicsPipelineFromLibraries pipe(*m_device, libraries);
    ASSERT_TRUE(pipe);

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    VkCommandBufferBeginInfo begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    VkCommandBufferInheritanceInfo hinfo = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    begin_info.pInheritanceInfo = &hinfo;

    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
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
        VkDeviceAddress *data = (VkDeviceAddress *)buffer0.memory().map();
        data[0] = i;
        buffer0.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kInformationBit, messages[i]);
        if (10 == i) {
            m_errorMonitor->SetDesiredFailureMsg(kInformationBit, messages[i + 1]);
            i++;
        }
        ASSERT_VK_SUCCESS(vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE));
        ASSERT_VK_SUCCESS(vk::QueueWaitIdle(m_device->m_queue));
        m_errorMonitor->VerifyFound();
    }

    if (multi_draw_features.multiDraw) {
        auto vkCmdDrawMultiEXT = (PFN_vkCmdDrawMultiEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawMultiEXT");
        auto vkCmdDrawMultiIndexedEXT =
            (PFN_vkCmdDrawMultiIndexedEXT)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawMultiIndexedEXT");
        assert(vkCmdDrawMultiEXT != nullptr && vkCmdDrawMultiIndexedEXT != nullptr);
        VkMultiDrawInfoEXT multi_draws[3] = {};
        multi_draws[0].vertexCount = multi_draws[1].vertexCount = multi_draws[2].vertexCount = 3;
        VkMultiDrawIndexedInfoEXT multi_draw_indices[3] = {};
        multi_draw_indices[0].indexCount = multi_draw_indices[1].indexCount = multi_draw_indices[2].indexCount = 3;
        m_commandBuffer->begin(&begin_info);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        vkCmdDrawMultiEXT(m_commandBuffer->handle(), 3, multi_draws, 1, 0, sizeof(VkMultiDrawInfoEXT));
        vk::CmdEndRenderPass(m_commandBuffer->handle());
        m_commandBuffer->end();

        VkDeviceAddress *data = (VkDeviceAddress *)buffer0.memory().map();
        data[0] = 0;
        buffer0.memory().unmap();
        for (auto i = 0; i < 3; i++) {
            m_errorMonitor->SetDesiredFailureMsg(kInformationBit, messages[0]);
        }
        ASSERT_VK_SUCCESS(vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE));
        ASSERT_VK_SUCCESS(vk::QueueWaitIdle(m_device->m_queue));
        m_errorMonitor->VerifyFound();

        VkBufferObj buffer;
        buffer.init(*m_device, 1024, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        uint16_t *ptr = static_cast<uint16_t *>(buffer.memory().map());
        ptr[0] = 0;
        ptr[1] = 1;
        ptr[2] = 2;
        buffer.memory().unmap();
        m_commandBuffer->begin(&begin_info);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        m_commandBuffer->BindIndexBuffer(&buffer, 0, VK_INDEX_TYPE_UINT16);
        vkCmdDrawMultiIndexedEXT(m_commandBuffer->handle(), 3, multi_draw_indices, 1, 0, sizeof(VkMultiDrawIndexedInfoEXT), 0);
        vk::CmdEndRenderPass(m_commandBuffer->handle());
        m_commandBuffer->end();

        data = (VkDeviceAddress *)buffer0.memory().map();
        data[0] = 1;
        buffer0.memory().unmap();
        for (auto i = 0; i < 3; i++) {
            m_errorMonitor->SetDesiredFailureMsg(kInformationBit, messages[1]);
        }
        ASSERT_VK_SUCCESS(vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE));
        ASSERT_VK_SUCCESS(vk::QueueWaitIdle(m_device->m_queue));
        m_errorMonitor->VerifyFound();
    }

    if (features.shaderInt64) {
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
        const auto vs_i64_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, shader_source_int64);
        vk_testing::GraphicsPipelineLibraryStage pre_raster_i64_stage(vs_i64_spv);

        CreatePipelineHelper pre_raster_i64(*this);
        pre_raster_i64.InitPreRasterLibInfo(1, &pre_raster_i64_stage.stage_ci);
        pre_raster_i64.gp_ci_.layout = pipeline_layout.handle();
        pre_raster_i64.gp_ci_.renderPass = render_pass;
        pre_raster_i64.gp_ci_.subpass = subpass;
        pre_raster_i64.CreateGraphicsPipeline(true, false);

        std::array<VkPipeline, 4> libraries_i64 = {
            vi.pipeline_,
            pre_raster_i64.pipeline_,
            fragment.pipeline_,
            frag_out.pipeline_,
        };

        vk_testing::GraphicsPipelineFromLibraries pipe2(*m_device, libraries_i64);
        ASSERT_TRUE(pipe2);

        m_commandBuffer->begin(&begin_info);
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
        vk::CmdEndRenderPass(m_commandBuffer->handle());
        m_commandBuffer->end();

        VkDeviceAddress *data = (VkDeviceAddress *)buffer0.memory().map();
        data[0] = 0;
        buffer0.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Here's an unsigned long 0x2000000000000001");
        ASSERT_VK_SUCCESS(vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE));
        ASSERT_VK_SUCCESS(vk::QueueWaitIdle(m_device->m_queue));
        m_errorMonitor->VerifyFound();

        data = (VkDeviceAddress *)buffer0.memory().map();
        data[0] = 1;
        buffer0.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(
            kInformationBit, "Here's a vector of ul 2000000000000001, 2000000000000001, 2000000000000001, 2000000000000001");
        ASSERT_VK_SUCCESS(vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE));
        ASSERT_VK_SUCCESS(vk::QueueWaitIdle(m_device->m_queue));
        m_errorMonitor->VerifyFound();

        data = (VkDeviceAddress *)buffer0.memory().map();
        data[0] = 2;
        buffer0.memory().unmap();
        m_errorMonitor->SetDesiredFailureMsg(kInformationBit,
                                             "Unsigned long as decimal 2305843009213693953 and as hex 0x2000000000000001");
        ASSERT_VK_SUCCESS(vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE));
        ASSERT_VK_SUCCESS(vk::QueueWaitIdle(m_device->m_queue));
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkDebugPrintfTest, GpuDebugPrintfGPLFragment) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);

    InitDebugPrintfFramework();
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(gpl_features);
    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    uint32_t queue_family_index = 0;
    buffer_create_info.size = 4;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &queue_family_index;
    VkBufferObj vs_buffer(*m_device, buffer_create_info, reqs), fs_buffer(*m_device, buffer_create_info, reqs);
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    OneOffDescriptorSet vertex_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}});
    OneOffDescriptorSet fragment_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});

    // "Normal" sets
    const VkPipelineLayoutObj pipeline_layout(m_device, {&vertex_set.layout_, &fragment_set.layout_});
    const auto vs_layout = pipeline_layout.handle();
    const auto fs_layout = pipeline_layout.handle();
    const auto layout = pipeline_layout.handle();

    vertex_set.WriteDescriptorBufferInfo(0, vs_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    vertex_set.UpdateDescriptorSets();
    fragment_set.WriteDescriptorBufferInfo(0, fs_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    fragment_set.UpdateDescriptorSets();

    {
        layer_data::span<uint32_t> vert_data(static_cast<uint32_t *>(vs_buffer.memory().map()),
                                             static_cast<uint32_t>(buffer_create_info.size) / sizeof(uint32_t));
        for (auto &v : vert_data) {
            v = 0x01030507;
        }
        vs_buffer.memory().unmap();
    }
    {
        layer_data::span<uint32_t> frag_data(static_cast<uint32_t *>(fs_buffer.memory().map()),
                                             static_cast<uint32_t>(buffer_create_info.size) / sizeof(uint32_t));
        for (auto &v : frag_data) {
            v = 0x02040608;
        }
        fs_buffer.memory().unmap();
    }

    const std::array<VkDescriptorSet, 2> desc_sets = {vertex_set.set_, fragment_set.set_};

    CreatePipelineHelper vi(*this);
    vi.InitVertexInputLibInfo();
    vi.InitState();
    ASSERT_VK_SUCCESS(vi.CreateGraphicsPipeline(true, false));

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
    vk_testing::GraphicsPipelineLibraryStage pre_raster_stage(vs_spv);

    VkDynamicState dyn_states[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    auto dyn_state = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state.dynamicStateCount = size(dyn_states);
    dyn_state.pDynamicStates = dyn_states;
    CreatePipelineHelper pre_raster(*this);
    pre_raster.InitPreRasterLibInfo(1, &pre_raster_stage.stage_ci);
    pre_raster.InitState();
    pre_raster.gp_ci_.layout = vs_layout;
    pre_raster.gp_ci_.pDynamicState = &dyn_state;
    pre_raster.CreateGraphicsPipeline(true, false);

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
    vk_testing::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper fragment(*this);
    fragment.InitFragmentLibInfo(1, &fs_stage.stage_ci);
    fragment.gp_ci_.layout = fs_layout;
    fragment.CreateGraphicsPipeline(true, false);

    CreatePipelineHelper frag_out(*this);
    frag_out.InitFragmentOutputLibInfo();
    ASSERT_VK_SUCCESS(frag_out.CreateGraphicsPipeline(true, false));

    std::array<VkPipeline, 4> libraries = {
        vi.pipeline_,
        pre_raster.pipeline_,
        fragment.pipeline_,
        frag_out.pipeline_,
    };
    vk_testing::GraphicsPipelineFromLibraries pipe(*m_device, libraries);
    ASSERT_TRUE(pipe);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0,
                              static_cast<uint32_t>(desc_sets.size()), desc_sets.data(), 0, nullptr);
    VkViewport viewport = {0, 0, 1, 1, 0, 1};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {1, 1}};
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Vertex shader 0, 0x1030507");
    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Fragment shader 0x2040608");
    m_commandBuffer->QueueCommandBuffer();
    vk::QueueWaitIdle(m_device->m_queue);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkDebugPrintfTest, GpuDebugPrintfGPLFragmentIndependentSets) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);

    InitDebugPrintfFramework();
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto gpl_features = LvlInitStruct<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(gpl_features);
    if (!gpl_features.graphicsPipelineLibrary) {
        GTEST_SKIP() << "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    uint32_t queue_family_index = 0;
    buffer_create_info.size = 4;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &queue_family_index;
    VkBufferObj vs_buffer(*m_device, buffer_create_info, reqs), fs_buffer(*m_device, buffer_create_info, reqs);
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    OneOffDescriptorSet vertex_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}});
    OneOffDescriptorSet fragment_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});

    // Independent sets
    const VkPipelineLayoutObj pipeline_layout_vs(m_device, {&vertex_set.layout_, nullptr}, {},
                                                 VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const auto vs_layout = pipeline_layout_vs.handle();
    const VkPipelineLayoutObj pipeline_layout_fs(m_device, {nullptr, &fragment_set.layout_}, {},
                                                 VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const auto fs_layout = pipeline_layout_fs.handle();
    const VkPipelineLayoutObj pipeline_layout(m_device, {&vertex_set.layout_, &fragment_set.layout_}, {},
                                              VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
    const auto layout = pipeline_layout.handle();

    vertex_set.WriteDescriptorBufferInfo(0, vs_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    vertex_set.UpdateDescriptorSets();
    fragment_set.WriteDescriptorBufferInfo(0, fs_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    fragment_set.UpdateDescriptorSets();

    {
        layer_data::span<uint32_t> vert_data(static_cast<uint32_t *>(vs_buffer.memory().map()),
                                             static_cast<uint32_t>(buffer_create_info.size) / sizeof(uint32_t));
        for (auto &v : vert_data) {
            v = 0x01030507;
        }
        vs_buffer.memory().unmap();
    }
    {
        layer_data::span<uint32_t> frag_data(static_cast<uint32_t *>(fs_buffer.memory().map()),
                                             static_cast<uint32_t>(buffer_create_info.size) / sizeof(uint32_t));
        for (auto &v : frag_data) {
            v = 0x02040608;
        }
        fs_buffer.memory().unmap();
    }

    const std::array<VkDescriptorSet, 2> desc_sets = {vertex_set.set_, fragment_set.set_};

    CreatePipelineHelper vi(*this);
    vi.InitVertexInputLibInfo();
    vi.InitState();
    ASSERT_VK_SUCCESS(vi.CreateGraphicsPipeline(true, false));

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
    vk_testing::GraphicsPipelineLibraryStage pre_raster_stage(vs_spv);

    VkDynamicState dyn_states[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    auto dyn_state = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state.dynamicStateCount = size(dyn_states);
    dyn_state.pDynamicStates = dyn_states;
    CreatePipelineHelper pre_raster(*this);
    pre_raster.InitPreRasterLibInfo(1, &pre_raster_stage.stage_ci);
    pre_raster.InitState();
    pre_raster.gp_ci_.layout = vs_layout;
    pre_raster.gp_ci_.pDynamicState = &dyn_state;
    pre_raster.CreateGraphicsPipeline(true, false);

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
    vk_testing::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper fragment(*this);
    fragment.InitFragmentLibInfo(1, &fs_stage.stage_ci);
    fragment.gp_ci_.layout = fs_layout;
    fragment.CreateGraphicsPipeline(true, false);

    CreatePipelineHelper frag_out(*this);
    frag_out.InitFragmentOutputLibInfo();
    ASSERT_VK_SUCCESS(frag_out.CreateGraphicsPipeline(true, false));

    std::array<VkPipeline, 4> libraries = {
        vi.pipeline_,
        pre_raster.pipeline_,
        fragment.pipeline_,
        frag_out.pipeline_,
    };
    vk_testing::GraphicsPipelineFromLibraries pipe(*m_device, libraries);
    ASSERT_TRUE(pipe);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0,
                              static_cast<uint32_t>(desc_sets.size()), desc_sets.data(), 0, nullptr);
    VkViewport viewport = {0, 0, 1, 1, 0, 1};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {1, 1}};
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Vertex shader 0, 0x1030507");
    m_errorMonitor->SetDesiredFailureMsg(kInformationBit, "Fragment shader 0x2040608");
    m_commandBuffer->QueueCommandBuffer();
    vk::QueueWaitIdle(m_device->m_queue);
    m_errorMonitor->VerifyFound();
}
