/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

TEST_F(PositiveShaderLimits, ComputeSharedMemoryWorkgroupMemoryExplicitLayout) {
    TEST_DESCRIPTION(
        "Validate compute shader shared memory does not exceed maxComputeSharedMemorySize when using "
        "VK_KHR_workgroup_memory_explicit_layout");
    // More background: When workgroupMemoryExplicitLayout is enabled and there are 2 or more structs, the
    // maxComputeSharedMemorySize is the MAX of the structs since they share the same WorkGroup memory. Test makes sure validation
    // is not doing an ADD and correctly doing a MAX operation in this case.

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    // need at least SPIR-V 1.4 for SPV_KHR_workgroup_memory_explicit_layout

    VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR explicit_layout_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(explicit_layout_features);
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    if (!explicit_layout_features.workgroupMemoryExplicitLayout) {
        GTEST_SKIP() << "workgroupMemoryExplicitLayout feature not supported.";
    }

    const uint32_t max_shared_memory_size = m_device->phy().limits_.maxComputeSharedMemorySize;
    const uint32_t max_shared_vec4 = max_shared_memory_size / 16;

    std::stringstream csSource;
    csSource << R"glsl(
        #version 450
        #extension GL_EXT_shared_memory_block : enable

        // Both structs by themselves are 16 bytes less than the max
        shared X {
            vec4 x1[)glsl";
    csSource << (max_shared_vec4 - 1);
    csSource << R"glsl(];
            vec4 x2;
        };

        void main() {
            x2.x = 0.0f; // prevent dead-code elimination
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.InitState();
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderLimits, ComputeSharedMemoryWorkgroupMemoryExplicitLayoutSpec) {
    TEST_DESCRIPTION(
        "Same test as ComputeSharedMemoryLimitWorkgroupMemoryExplicitLayout but making sure the path when using spec constants "
        "works");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    // need at least SPIR-V 1.4 for SPV_KHR_workgroup_memory_explicit_layout

    VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR explicit_layout_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(explicit_layout_features);
    RETURN_IF_SKIP(InitState(nullptr, &explicit_layout_features));

    if (!explicit_layout_features.workgroupMemoryExplicitLayout) {
        GTEST_SKIP() << "workgroupMemoryExplicitLayout feature not supported.";
    }

    const uint32_t max_shared_memory_size = m_device->phy().limits_.maxComputeSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;
    const uint32_t max_shared_vec4 = max_shared_memory_size / 16;

    std::stringstream csSource;
    csSource << R"glsl(
        #version 450
        #extension GL_EXT_shared_memory_block : enable

        // will be over the max if the spec constant uses default value
        layout(constant_id = 0) const uint value = )glsl";
    csSource << (max_shared_vec4 + 16);
    csSource << R"glsl(;

        // Both structs by themselves are 16 bytes less than the max
        shared X {
            vec4 x1[value];
            vec4 x2;
        };

        shared Y {
            int y1[)glsl";
    csSource << (max_shared_ints - 4);
    csSource << R"glsl(];
            int y2;
        };

        void main() {
            x2.x = 0.0f; // prevent dead-code elimination
            y2 = 0;
        }
    )glsl";

    uint32_t data = max_shared_vec4 - 16;

    VkSpecializationMapEntry entry;
    entry.constantID = 0;
    entry.offset = 0;
    entry.size = sizeof(uint32_t);

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2,
                                             SPV_SOURCE_GLSL, &specialization_info);
    pipe.InitState();
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderLimits, ComputeSharedMemoryAtLimit) {
    TEST_DESCRIPTION("Validate compute shader shared memory is valid at the exact maxComputeSharedMemorySize");

    RETURN_IF_SKIP(Init())

    const uint32_t max_shared_memory_size = m_device->phy().limits_.maxComputeSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream csSource;
    csSource << R"glsl(
        #version 450
        shared int a[)glsl";
    csSource << (max_shared_ints);
    csSource << R"glsl(];
        void main(){}
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.InitState();
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderLimits, ComputeSharedMemoryBooleanAtLimit) {
    TEST_DESCRIPTION("Validate compute shader shared memory is valid at the exact maxComputeSharedMemorySize using Booleans");

    RETURN_IF_SKIP(Init())

    const uint32_t max_shared_memory_size = m_device->phy().limits_.maxComputeSharedMemorySize;
    // "Boolean values considered as 32-bit integer values for the purpose of this calculation."
    const uint32_t max_shared_bools = max_shared_memory_size / 4;

    std::stringstream csSource;
    csSource << R"glsl(
        #version 450
        shared bool a[)glsl";
    csSource << (max_shared_bools);
    csSource << R"glsl(];
        void main(){}
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.InitState();
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderLimits, MeshSharedMemoryAtLimit) {
    TEST_DESCRIPTION("Validate mesh shader shared memory is valid at the exact maxMeshSharedMemorySize");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (!mesh_shader_features.meshShader) {
        GTEST_SKIP() << "Mesh shader not supported";
    }
    mesh_shader_features.multiviewMeshShader = VK_FALSE;
    mesh_shader_features.primitiveFragmentShadingRateMeshShader = VK_FALSE;

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
    mesh_source << (max_shared_ints);
    mesh_source << R"glsl(];
        void main(){}
    )glsl";

    VkShaderObj mesh(this, mesh_source.str().c_str(), VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.fs_->GetStageCreateInfo(), mesh.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderLimits, TaskSharedMemoryAtLimit) {
    TEST_DESCRIPTION("Validate Task shader shared memory is valid at the exact maxTaskSharedMemorySize");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (!mesh_shader_features.meshShader || !mesh_shader_features.taskShader) {
        GTEST_SKIP() << "Mesh and Task shader not supported";
    }
    mesh_shader_features.multiviewMeshShader = VK_FALSE;
    mesh_shader_features.primitiveFragmentShadingRateMeshShader = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &mesh_shader_features));
    InitRenderTarget();

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    const uint32_t max_shared_memory_size = mesh_shader_properties.maxMeshSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    std::stringstream task_source;
    task_source << R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : require
        shared int a[)glsl";
    task_source << (max_shared_ints);
    task_source << R"glsl(];
        void main(){}
    )glsl";

    VkShaderObj task(this, task_source.str().c_str(), VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj mesh(this, kMeshMinimalGlsl, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {task.GetStageCreateInfo(), mesh.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

