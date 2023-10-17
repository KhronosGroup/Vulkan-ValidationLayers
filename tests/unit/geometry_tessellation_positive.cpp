/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/cast_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

TEST_F(PositiveGeometryTessellation, PointSizeGeomShaderDontWriteMaintenance5) {
    TEST_DESCRIPTION(
        "Create a pipeline using TOPOLOGY_POINT_LIST, set PointSize vertex shader, but not in the final geometry stage, but have maintenance5.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(maintenance5_features);
    RETURN_IF_SKIP(InitState(nullptr, &maintenance5_features));

    if ((!m_device->phy().features().geometryShader) || (!m_device->phy().features().shaderTessellationAndGeometryPointSize)) {
        GTEST_SKIP() << "Device does not support the required geometry shader features";
    }
    InitRenderTarget();

    // Create GS declaring PointSize and writing to it
    static char const *gsSource = R"glsl(
        #version 450
        layout (points) in;
        layout (points) out;
        layout (max_vertices = 1) out;
        void main() {
           gl_Position = vec4(1.0, 0.5, 0.5, 0.0);
           EmitVertex();
        }
    )glsl";

    VkShaderObj vs(this, kVertexPointSizeGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        helper.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };

    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveGeometryTessellation, IncompatibleDynamicPrimitiveTopology) {
    TEST_DESCRIPTION("Create pipeline with primitive topology incompatible with shaders, but use VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT eds_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(eds_features);
    if (!eds_features.extendedDynamicState) {
        GTEST_SKIP() << "extendedDynamicState not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &eds_features));
    if ((!m_device->phy().features().geometryShader) || (!m_device->phy().features().shaderTessellationAndGeometryPointSize)) {
        GTEST_SKIP() << "Device does not support the required geometry shader features";
    }
    InitRenderTarget();

    static const char *gsSource = R"glsl(
        #version 450
        layout (points) in;
        layout (triangle_strip) out;
        layout (max_vertices = 3) out;
        in gl_PerVertex
        {
            vec4 gl_Position;
            float gl_PointSize;
        } gl_in[];
        void main()
        {
            gl_Position = gl_in[0].gl_Position;
            gl_PointSize = gl_in[0].gl_PointSize;
            EmitVertex();
        }
    )glsl";

    VkShaderObj vs(this, kVertexPointSizeGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj gs(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        helper.AddDynamicState(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT);
        helper.shader_stages_ = {vs.GetStageCreateInfo(), gs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}
