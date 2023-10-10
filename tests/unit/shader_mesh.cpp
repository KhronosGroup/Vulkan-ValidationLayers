/*
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

void NegativeShaderMesh::InitBasicMeshShader(void *pNextFeatures) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework(m_errorMonitor));
    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (!mesh_shader_features.meshShader || !mesh_shader_features.taskShader) {
        GTEST_SKIP() << "Mesh and Task shader not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &mesh_shader_features));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required.";
    }
}

TEST_F(NegativeShaderMesh, SharedMemoryOverLimit) {
    TEST_DESCRIPTION("Validate mesh shader shared memory does not exceed maxMeshSharedMemorySize");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (!mesh_shader_features.meshShader) {
        GTEST_SKIP() << "Mesh shader not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &mesh_shader_features));
    InitRenderTarget();

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_shared_memory_size = mesh_shader_properties.maxMeshSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream mesh_source;
    mesh_source << R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        layout(max_vertices = 3, max_primitives=1) out;
        layout(triangles) out;
        shared int a[)glsl";
    mesh_source << (max_shared_ints + 16);
    mesh_source << R"glsl(];
        void main(){}
    )glsl";

    VkShaderObj mesh(this, mesh_source.str().c_str(), VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.fs_->GetStageCreateInfo(), mesh.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-maxMeshSharedMemorySize-08754");
}

TEST_F(NegativeShaderMesh, SharedMemoryOverLimitWorkgroupMemoryExplicitLayout) {
    TEST_DESCRIPTION(
        "Validate mesh shader shared memory does not exceed maxMeshSharedMemorySize when using "
        "VK_KHR_workgroup_memory_explicit_layout");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR explicit_layout_features = vku::InitStructHelper();
    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = vku::InitStructHelper(&explicit_layout_features);
    GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (!mesh_shader_features.meshShader) {
        GTEST_SKIP() << "Mesh shader not supported";
    } else if (!explicit_layout_features.workgroupMemoryExplicitLayout) {
        GTEST_SKIP() << "workgroupMemoryExplicitLayout feature not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &mesh_shader_features));
    InitRenderTarget();

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_shared_memory_size = mesh_shader_properties.maxMeshSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream mesh_source;
    mesh_source << R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        #extension GL_EXT_shared_memory_block : enable
        layout(max_vertices = 3, max_primitives=1) out;
        layout(triangles) out;

        shared X {
            int x;
        };

        shared Y {
            int y1[)glsl";
    mesh_source << (max_shared_ints + 16);
    mesh_source << R"glsl(];
            int y2;
        };

        void main() {
            x = 0; // prevent dead-code elimination
            y2 = 0;
        }
    )glsl";

    VkShaderObj mesh(this, mesh_source.str().c_str(), VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.fs_->GetStageCreateInfo(), mesh.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-maxMeshSharedMemorySize-08754");
}

TEST_F(NegativeShaderMesh, SharedMemorySpecConstantDefault) {
    TEST_DESCRIPTION("Validate shared memory exceed maxMeshSharedMemorySize limit with spec constants default");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (!mesh_shader_features.meshShader) {
        GTEST_SKIP() << "Mesh shader not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &mesh_shader_features));
    InitRenderTarget();

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_shared_memory_size = mesh_shader_properties.maxMeshSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream mesh_source;
    mesh_source << R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        layout(max_vertices = 3, max_primitives=1) out;
        layout(triangles) out;
        layout(constant_id = 0) const uint Condition = 1;
        layout(constant_id = 1) const uint SharedSize = )glsl";
    mesh_source << (max_shared_ints + 16);
    mesh_source << R"glsl(;

        #define enableSharedMemoryOpt (Condition == 1)
        shared uint arr[enableSharedMemoryOpt ? SharedSize : 1];
        void main(){}
    )glsl";

    VkShaderObj mesh(this, mesh_source.str().c_str(), VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.fs_->GetStageCreateInfo(), mesh.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-maxMeshSharedMemorySize-08754");
}

TEST_F(NegativeShaderMesh, SharedMemorySpecConstantSet) {
    TEST_DESCRIPTION("Validate shared memory exceed maxMeshSharedMemorySize limit with spec constants set");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (!mesh_shader_features.meshShader) {
        GTEST_SKIP() << "Mesh shader not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &mesh_shader_features));
    InitRenderTarget();

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_shared_memory_size = mesh_shader_properties.maxMeshSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream mesh_source;
    mesh_source << R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        layout(max_vertices = 3, max_primitives=1) out;
        layout(triangles) out;
        layout(constant_id = 0) const uint Condition = 1;
        layout(constant_id = 1) const uint SharedSize = )glsl";
    mesh_source << (max_shared_ints + 16);
    mesh_source << R"glsl(;

        #define enableSharedMemoryOpt (Condition == 1)
        shared uint arr[enableSharedMemoryOpt ? SharedSize : 1];
        void main(){}
    )glsl";

    uint32_t data = 1;  // set Condition

    VkSpecializationMapEntry entry;
    entry.constantID = 0;
    entry.offset = 0;
    entry.size = sizeof(uint32_t);

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    VkShaderObj mesh(this, mesh_source.str().c_str(), VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_GLSL,
                     &specialization_info);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.fs_->GetStageCreateInfo(), mesh.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-maxMeshSharedMemorySize-08754");
}

TEST_F(NegativeShaderMesh, TaskSharedMemoryOverLimit) {
    TEST_DESCRIPTION("Validate Task shader shared memory limit");
    RETURN_IF_SKIP(InitBasicMeshShader());

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_shared_memory_size = mesh_shader_properties.maxTaskSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    VkShaderObj mesh(this, kMeshMinimalGlsl, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);

    std::stringstream task_source;
    task_source << R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        shared int a[)glsl";
    task_source << (max_shared_ints + 16);
    task_source << R"glsl(];
        void main(){}
    )glsl";

    VkShaderObj task(this, task_source.str().c_str(), VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_2);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {task.GetStageCreateInfo(), mesh.GetStageCreateInfo()};
    };
    if (mesh_shader_properties.maxTaskSharedMemorySize == mesh_shader_properties.maxTaskPayloadAndSharedMemorySize) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-maxTaskPayloadAndSharedMemorySize-08760");
    }
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-maxTaskSharedMemorySize-08759");
}

TEST_F(NegativeShaderMesh, TaskPayloadMemoryOverLimit) {
    TEST_DESCRIPTION("Validate Task shader task payload memory limit");
    RETURN_IF_SKIP(InitBasicMeshShader());

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_payload_memory_size = mesh_shader_properties.maxTaskPayloadSize;
    const uint32_t max_payload_ints = max_payload_memory_size / 4;

    VkShaderObj mesh(this, kMeshMinimalGlsl, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);

    std::stringstream task_source;
    task_source << R"glsl(
            #version 460
            #extension GL_EXT_mesh_shader : require
            struct Task {
                uint baseID[)glsl";
    task_source << (max_payload_ints + 16);
    task_source << R"glsl(];
            };
            taskPayloadSharedEXT Task OUT;
            void main(){}
        )glsl";

    VkShaderObj task(this, task_source.str().c_str(), VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_2);
    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {task.GetStageCreateInfo(), mesh.GetStageCreateInfo()};
    };
    if (mesh_shader_properties.maxTaskPayloadSize == mesh_shader_properties.maxTaskPayloadAndSharedMemorySize) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-maxTaskPayloadAndSharedMemorySize-08760");
    }
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-maxTaskPayloadSize-08758");
}

TEST_F(NegativeShaderMesh, TaskShaderAndPayloadMemoryOverLimit) {
    TEST_DESCRIPTION("Validate Task shader task shared and payload memory limit");
    RETURN_IF_SKIP(InitBasicMeshShader());

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_shared_memory_size = mesh_shader_properties.maxTaskSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;
    const uint32_t max_payload_memory_size = mesh_shader_properties.maxTaskPayloadSize;
    const uint32_t max_payload_ints = max_payload_memory_size / 4;
    const uint32_t max_payload_and_shared_memory_size = mesh_shader_properties.maxTaskPayloadAndSharedMemorySize;
    const uint32_t max_payload_and_shared_ints = max_payload_and_shared_memory_size / 4;

    VkShaderObj mesh(this, kMeshMinimalGlsl, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);

    std::stringstream task_source;
    task_source << R"glsl(
            #version 460
            #extension GL_EXT_mesh_shader : require
            struct Task {
                uint baseID[)glsl";
    task_source << (max_payload_and_shared_ints / 2 + 1);
    task_source << R"glsl(];
            };
            taskPayloadSharedEXT Task OUT;
            shared int a[)glsl";
    task_source << (max_payload_and_shared_ints / 2 + 1);
    task_source << R"glsl(];
            void main(){}
        )glsl";

    VkShaderObj task(this, task_source.str().c_str(), VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_2);
    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {task.GetStageCreateInfo(), mesh.GetStageCreateInfo()};
    };
    if (max_payload_and_shared_ints / 2 + 1 > max_payload_ints) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-maxTaskPayloadSize-08758");
    }
    if (max_payload_and_shared_ints / 2 + 1 > max_shared_ints) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-maxTaskSharedMemorySize-08759");
    }
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-maxTaskPayloadAndSharedMemorySize-08760");
}

TEST_F(NegativeShaderMesh, MeshShaderPayloadMemoryOverLimit) {
    TEST_DESCRIPTION("Validate Mesh shader shared memory limit");
    RETURN_IF_SKIP(InitBasicMeshShader());

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_mesh_payload_and_shared_memory_size = mesh_shader_properties.maxMeshPayloadAndSharedMemorySize;
    const uint32_t max_mesh_payload_and_shared_ints = max_mesh_payload_and_shared_memory_size / 4;

    std::stringstream mesh_source;
    mesh_source << R"glsl(
            #version 460
            #extension GL_EXT_mesh_shader : require
            layout(max_vertices = 3, max_primitives=1) out;
            layout(triangles) out;
            struct Task {
                uint baseID[)glsl";
    mesh_source << (max_mesh_payload_and_shared_ints / 2 + 1);
    mesh_source << R"glsl(];
            };
            taskPayloadSharedEXT Task IN;
            shared int a[)glsl";
    mesh_source << (max_mesh_payload_and_shared_ints / 2 + 1);
    mesh_source << R"glsl(];
            void main(){}
        )glsl";

    VkShaderObj mesh(this, mesh_source.str().c_str(), VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    const auto set_info = [&](CreatePipelineHelper &helper) { helper.shader_stages_ = {mesh.GetStageCreateInfo()}; };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-maxMeshPayloadAndSharedMemorySize-08755");
}

TEST_F(NegativeShaderMesh, MeshShaderOutputAndPayloadMemoryOverLimit) {
    TEST_DESCRIPTION("Validate Mesh shader output and payload memory limit");
    RETURN_IF_SKIP(InitBasicMeshShader());

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_mesh_payload_and_output_memory_size = mesh_shader_properties.maxMeshPayloadAndOutputMemorySize;
    const uint32_t max_mesh_payload_and_output_ints = max_mesh_payload_and_output_memory_size / 4;

    uint32_t vertex_count = mesh_shader_properties.maxMeshOutputVertices;
    uint32_t output_count = max_mesh_payload_and_output_ints / vertex_count / 4 / 4 / 2 + 1;
    std::stringstream mesh_source;
    mesh_source << R"glsl(
            #version 460
            #extension GL_EXT_mesh_shader : require
            layout(max_vertices = )glsl";
    mesh_source << vertex_count;
    mesh_source << R"glsl(, max_primitives=1) out;
            layout(triangles) out;
            struct Task {
                uint baseID[)glsl";
    mesh_source << (max_mesh_payload_and_output_ints / 2 + 1);
    mesh_source << R"glsl(];
            };
            taskPayloadSharedEXT Task IN;
            perprimitiveEXT out gl_MeshPerPrimitiveEXT {
                int gl_PrimitiveID;
            } gl_MeshPrimitivesEXT[];
            )glsl";
    for (uint32_t i = 0; i < output_count; ++i) {
        mesh_source << R"glsl(layout(location = )glsl" << i << R"glsl() out vec4 v)glsl";
        mesh_source << i << "[" << vertex_count;
        mesh_source << R"glsl(];
            )glsl";
    }
    mesh_source << R"glsl(
            void main(){
                gl_MeshPrimitivesEXT[0].gl_PrimitiveID = 0;
            }
        )glsl";

    VkShaderObj mesh(this, mesh_source.str().c_str(), VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    const auto set_info = [&](CreatePipelineHelper &helper) { helper.shader_stages_ = {mesh.GetStageCreateInfo()}; };
    if (output_count * 16 * 4 * vertex_count >= mesh_shader_properties.maxMeshOutputMemorySize) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-RuntimeSpirv-maxMeshOutputMemorySize-08756");
    }
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-maxMeshPayloadAndOutputMemorySize-08757");
}

TEST_F(NegativeShaderMesh, MeshShaderOutputMemoryOverLimit) {
    TEST_DESCRIPTION("Validate Mesh shader output memory limit");
    RETURN_IF_SKIP(InitBasicMeshShader());

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_mesh_payload_and_output_memory_size = mesh_shader_properties.maxMeshPayloadAndOutputMemorySize;
    const uint32_t max_mesh_payload_and_output_ints = max_mesh_payload_and_output_memory_size / 4;

    uint32_t vertex_count = mesh_shader_properties.maxMeshOutputVertices;
    uint32_t output_count = max_mesh_payload_and_output_ints / vertex_count / 4 / 4;
    std::stringstream mesh_source;
    mesh_source << R"glsl(
            #version 460
            #extension GL_EXT_mesh_shader : require
            layout(max_vertices = )glsl";
    mesh_source << vertex_count;
    mesh_source << R"glsl(, max_primitives=1) out;
            layout(triangles) out;
            )glsl";
    for (uint32_t i = 0; i < output_count; ++i) {
        mesh_source << R"glsl(layout(location = )glsl" << i << R"glsl() out vec4 v)glsl";
        mesh_source << i << "[" << vertex_count;
        mesh_source << R"glsl(];
            )glsl";
    }
    mesh_source << R"glsl(
            void main(){
                gl_MeshPrimitivesEXT[0].gl_PrimitiveID = 0;
            }
        )glsl";

    VkShaderObj mesh(this, mesh_source.str().c_str(), VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    const auto set_info = [&](CreatePipelineHelper &helper) { helper.shader_stages_ = {mesh.GetStageCreateInfo()}; };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-maxMeshOutputMemorySize-08756");
}
