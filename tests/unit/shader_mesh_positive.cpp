/*
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "shader_helper.h"
#include "shader_templates.h"

class PositiveShaderMesh : public VkLayerTest {};

TEST_F(PositiveShaderMesh, MeshShaderPayloadMemoryOverLimit) {
    TEST_DESCRIPTION("Validate Mesh shader shared memory limit");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::taskShader);
    AddRequiredFeature(vkt::Feature::meshShader);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "Hard to get limit tests to work everywhere without being too complex";
    }

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_mesh_payload_and_shared_memory_size = mesh_shader_properties.maxMeshPayloadAndSharedMemorySize;
    const uint32_t max_mesh_payload_and_shared_ints = max_mesh_payload_and_shared_memory_size / 4;

    std::stringstream task_source;
    task_source << R"glsl(
            #version 460
            #extension GL_EXT_mesh_shader : require
            struct Task {
                uint baseID[)glsl";
    task_source << (max_mesh_payload_and_shared_ints / 2);
    task_source << R"glsl(];
            };
            taskPayloadSharedEXT Task IN;
            void main(){
                IN.baseID[0] = 0;
                EmitMeshTasksEXT(1u, 1u, 1u);
            }
        )glsl";

    std::stringstream mesh_source;
    mesh_source << R"glsl(
            #version 460
            #extension GL_EXT_mesh_shader : require
            layout(max_vertices = 3, max_primitives=1) out;
            layout(triangles) out;
            struct Task {
                uint baseID[)glsl";
    mesh_source << (max_mesh_payload_and_shared_ints / 2);
    mesh_source << R"glsl(];
            };
            taskPayloadSharedEXT Task IN;
            shared int a[)glsl";
    mesh_source << (max_mesh_payload_and_shared_ints / 2);
    mesh_source << R"glsl(];
            void main(){}
        )glsl";

    VkShaderObj task(*m_device, task_source.str().c_str(), VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj mesh(*m_device, mesh_source.str().c_str(), VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {task.GetStageCreateInfo(), mesh.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderMesh, MeshShaderPayloadSpecConstantSet) {
    TEST_DESCRIPTION("Validate Mesh shader shared memory limit");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::taskShader);
    AddRequiredFeature(vkt::Feature::meshShader);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "Hard to get limit tests to work everywhere without being too complex";
    }

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_mesh_payload_and_shared_memory_size = mesh_shader_properties.maxMeshPayloadAndSharedMemorySize;
    const uint32_t max_mesh_payload_and_shared_ints = max_mesh_payload_and_shared_memory_size / 4;

    const char *task_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        layout(constant_id = 0) const int SIZE = 64;
        struct Task {
            uint baseID[SIZE];
        };
        taskPayloadSharedEXT Task IN;
        void main() {
            IN.baseID[0] = 0;
            EmitMeshTasksEXT(1u, 1u, 1u);
        }
    )glsl";

    const char *mesh_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        layout(max_vertices = 3, max_primitives=1) out;
        layout(triangles) out;
        layout(constant_id = 0) const int SIZE = 64;
        struct Task {
            uint baseID[SIZE];
        };
        taskPayloadSharedEXT Task IN;
        shared int a[SIZE];
        void main(){}
    )glsl";

    uint32_t size = max_mesh_payload_and_shared_ints / 2;

    VkSpecializationMapEntry map_entry;
    map_entry.constantID = 0u;
    map_entry.offset = 0u;
    map_entry.size = sizeof(uint32_t);

    VkSpecializationInfo spec_info;
    spec_info.mapEntryCount = 1u;
    spec_info.pMapEntries = &map_entry;
    spec_info.dataSize = sizeof(uint32_t);
    spec_info.pData = &size;

    VkShaderObj task(*m_device, task_source, VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj mesh(*m_device, mesh_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_GLSL, &spec_info);
    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {task.GetStageCreateInfo(), mesh.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderMesh, TaskSharedMemory) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::taskShader);
    AddRequiredFeature(vkt::Feature::meshShader);
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitRenderTarget());

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_shared_memory_size = mesh_shader_properties.maxTaskSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    VkShaderObj mesh(*m_device, kMeshMinimalGlsl, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);

    std::stringstream task_source;
    task_source << R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        layout (constant_id = 0) const int v = )glsl";
    task_source << (max_shared_ints + 16);
    task_source << R"glsl(;
        shared int a[v];
        void main(){}
    )glsl";

    uint32_t data = max_shared_ints;
    VkSpecializationMapEntry map_entry = {0, 0, sizeof(uint32_t)};
    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &map_entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    VkShaderObj task(*m_device, task_source.str().c_str(), VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_GLSL,
                     &specialization_info);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {task.GetStageCreateInfo(), mesh.GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();
}