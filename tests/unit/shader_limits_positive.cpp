/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

TEST_F(PositiveShaderLimits, MaxSampleMaskWords) {
    TEST_DESCRIPTION("Test limit of maxSampleMaskWords.");

    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // Valid input of sample mask
    char const *fs_source = R"glsl(
        #version 450
        layout(location = 0) out vec4 uFragColor;
        void main(){
           int y = gl_SampleMaskIn[0];
           uFragColor = vec4(0,1,0,1) * y;
        }
    )glsl";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto validPipeline = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, validPipeline, kErrorBit);
}

TEST_F(PositiveShaderLimits, ComputeSharedMemoryWorkgroupMemoryExplicitLayout) {
    TEST_DESCRIPTION(
        "Validate compute shader shared memory does not exceed maxComputeSharedMemorySize when using "
        "VK_KHR_workgroup_memory_explicit_layout");
    // More background: When workgroupMemoryExplicitLayout is enabled and there are 2 or more structs, the
    // maxComputeSharedMemorySize is the MAX of the structs since they share the same WorkGroup memory. Test makes sure validation
    // is not doing an ADD and correctly doing a MAX operation in this case.

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    // need at least SPIR-V 1.4 for SPV_KHR_workgroup_memory_explicit_layout

    VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR explicit_layout_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(explicit_layout_features);
    RETURN_IF_SKIP(InitState(nullptr, &features2));

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
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderLimits, ComputeSharedMemoryAtLimit) {
    TEST_DESCRIPTION("Validate compute shader shared memory is valid at the exact maxComputeSharedMemorySize");

    RETURN_IF_SKIP(Init());

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
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderLimits, ComputeSharedMemoryBooleanAtLimit) {
    TEST_DESCRIPTION("Validate compute shader shared memory is valid at the exact maxComputeSharedMemorySize using Booleans");

    RETURN_IF_SKIP(Init());

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
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderLimits, MeshSharedMemoryAtLimit) {
    TEST_DESCRIPTION("Validate mesh shader shared memory is valid at the exact maxMeshSharedMemorySize");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddDisabledFeature(vkt::Feature::multiviewMeshShader);
    AddDisabledFeature(vkt::Feature::primitiveFragmentShadingRateMeshShader);
    RETURN_IF_SKIP(Init());
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
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::taskShader);
    AddDisabledFeature(vkt::Feature::multiviewMeshShader);
    AddDisabledFeature(vkt::Feature::primitiveFragmentShadingRateMeshShader);
    RETURN_IF_SKIP(Init());
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

