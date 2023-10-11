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

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

TEST_F(NegativePipelineTopology, PolygonMode) {
    TEST_DESCRIPTION("Attempt to use invalid polygon fill modes.");
    VkPhysicalDeviceFeatures device_features = {};
    device_features.fillModeNonSolid = VK_FALSE;
    // The sacrificial device object
    RETURN_IF_SKIP(Init(&device_features));
    InitRenderTarget();

    VkPipelineRasterizationStateCreateInfo rs_ci = vku::InitStructHelper();
    rs_ci.lineWidth = 1.0f;
    rs_ci.rasterizerDiscardEnable = VK_TRUE;

    auto set_polygonMode = [&](CreatePipelineHelper &helper) { helper.rs_state_ci_ = rs_ci; };

    // Set polygonMode to POINT while the non-solid fill mode feature is disabled.
    // Introduce failure by setting unsupported polygon mode
    rs_ci.polygonMode = VK_POLYGON_MODE_POINT;
    CreatePipelineHelper::OneshotTest(*this, set_polygonMode, kErrorBit,
                                      "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01507");

    // Set polygonMode to LINE while the non-solid fill mode feature is disabled.
    // Introduce failure by setting unsupported polygon mode
    rs_ci.polygonMode = VK_POLYGON_MODE_LINE;
    CreatePipelineHelper::OneshotTest(*this, set_polygonMode, kErrorBit,
                                      "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01507");

    // Set polygonMode to FILL_RECTANGLE_NV while the extension is not enabled.
    // Introduce failure by setting unsupported polygon mode
    rs_ci.polygonMode = VK_POLYGON_MODE_FILL_RECTANGLE_NV;
    CreatePipelineHelper::OneshotTest(*this, set_polygonMode, kErrorBit,
                                      "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01414");
}

// Create VS declaring PointSize but not writing to it
static const char *NoPointSizeVertShader = R"glsl(
    #version 450
    vec2 vertices[3];
    out gl_PerVertex
    {
        vec4 gl_Position;
        float gl_PointSize;
    };
    void main() {
        vertices[0] = vec2(-1.0, -1.0);
        vertices[1] = vec2( 1.0, -1.0);
        vertices[2] = vec2( 0.0,  1.0);
        gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
    }
)glsl";

TEST_F(NegativePipelineTopology, PointSize) {
    TEST_DESCRIPTION("Create a pipeline using TOPOLOGY_POINT_LIST but do not set PointSize in vertex shader.");

    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkShaderObj vs(this, NoPointSizeVertShader, VK_SHADER_STAGE_VERTEX_BIT);

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-topology-08773");
}

TEST_F(NegativePipelineTopology, PointSizeNonDynamicAndRestricted) {
    TEST_DESCRIPTION(
        "Create a pipeline using TOPOLOGY_POINT_LIST but do not set PointSize in vertex shader, with no dynamic state and "
        "dynamicPrimitiveTopologyUnrestricted is false.");

    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    VkPhysicalDeviceExtendedDynamicState3PropertiesEXT dynamic_state_3_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(dynamic_state_3_props);
    if (dynamic_state_3_props.dynamicPrimitiveTopologyUnrestricted) {
        GTEST_SKIP() << "dynamicPrimitiveTopologyUnrestricted is VK_TRUE";
    }

    VkShaderObj vs(this, NoPointSizeVertShader, VK_SHADER_STAGE_VERTEX_BIT);

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-topology-08773");
}

TEST_F(NegativePipelineTopology, PointSizeNonDynamicAndUnrestricted) {
    TEST_DESCRIPTION(
        "Create a pipeline using TOPOLOGY_POINT_LIST but do not set PointSize in vertex shader, with "
        "dynamicPrimitiveTopologyUnrestricted is true, but not dynamic state.");

    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())
    InitRenderTarget();

    VkPhysicalDeviceExtendedDynamicState3PropertiesEXT dynamic_state_3_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(dynamic_state_3_props);
    if (!dynamic_state_3_props.dynamicPrimitiveTopologyUnrestricted) {
        GTEST_SKIP() << "dynamicPrimitiveTopologyUnrestricted is VK_FALSE";
    }

    VkShaderObj vs(this, NoPointSizeVertShader, VK_SHADER_STAGE_VERTEX_BIT);

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-topology-08773");
}

TEST_F(NegativePipelineTopology, PointSizeDynamicAndRestricted) {
    TEST_DESCRIPTION(
        "Create a pipeline using TOPOLOGY_POINT_LIST but do not set PointSize in vertex shader, with dynamic state but "
        "dynamicPrimitiveTopologyUnrestricted is false.");

    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(extended_dynamic_state_features);
    if (!extended_dynamic_state_features.extendedDynamicState) {
        GTEST_SKIP() << "Test requires (unsupported) extendedDynamicState";
    }

    RETURN_IF_SKIP(InitState(nullptr, &extended_dynamic_state_features));
    InitRenderTarget();

    VkPhysicalDeviceExtendedDynamicState3PropertiesEXT dynamic_state_3_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(dynamic_state_3_props);
    if (dynamic_state_3_props.dynamicPrimitiveTopologyUnrestricted) {
        GTEST_SKIP() << "dynamicPrimitiveTopologyUnrestricted is VK_TRUE";
    }

    VkShaderObj vs(this, NoPointSizeVertShader, VK_SHADER_STAGE_VERTEX_BIT);

    const VkDynamicState dyn_state = VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY;
    VkPipelineDynamicStateCreateInfo dyn_state_ci = vku::InitStructHelper();
    dyn_state_ci.dynamicStateCount = 1;
    dyn_state_ci.pDynamicStates = &dyn_state;

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        helper.dyn_state_ci_ = dyn_state_ci;
        helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-topology-08773");
}

TEST_F(NegativePipelineTopology, PrimitiveTopology) {
    TEST_DESCRIPTION("InvalidTopology.");
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.geometryShader = VK_FALSE;
    deviceFeatures.tessellationShader = VK_FALSE;

    RETURN_IF_SKIP(Init(&deviceFeatures));
    InitRenderTarget();

    VkShaderObj vs(this, kVertexPointSizeGlsl, VK_SHADER_STAGE_VERTEX_BIT);

    VkPrimitiveTopology topology;

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = topology;
        helper.ia_ci_.primitiveRestartEnable = VK_TRUE;
        helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };

    topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-06252");

    topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-06252");

    topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-06252");

    {
        topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
        constexpr std::array vuids = {"VUID-VkPipelineInputAssemblyStateCreateInfo-topology-06252",
                                      "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00429"};
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
    }

    {
        topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
        constexpr std::array vuids = {"VUID-VkPipelineInputAssemblyStateCreateInfo-topology-06252",
                                      "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00429"};
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
    }

    {
        topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        constexpr std::array vuids = {"VUID-VkPipelineInputAssemblyStateCreateInfo-topology-06253",
                                      "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00430",
                                      "VUID-VkGraphicsPipelineCreateInfo-topology-08889"};
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
    }

    topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00429");

    topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00429");
}

TEST_F(NegativePipelineTopology, PrimitiveTopologyListRestart) {
    TEST_DESCRIPTION("Test VK_EXT_primitive_topology_list_restart");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_PRIMITIVE_TOPOLOGY_LIST_RESTART_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT ptl_restart_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(ptl_restart_features);

    if (!ptl_restart_features.primitiveTopologyListRestart) {
        GTEST_SKIP() << "primitive topology list restart feature is not available, skipping test";
    }
    ptl_restart_features.primitiveTopologyListRestart = false;
    ptl_restart_features.primitiveTopologyPatchListRestart = false;
    RETURN_IF_SKIP(InitState(nullptr, &ptl_restart_features));
    InitRenderTarget();

    VkShaderObj vs(this, kVertexPointSizeGlsl, VK_SHADER_STAGE_VERTEX_BIT);

    VkPrimitiveTopology topology;

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.ia_ci_.topology = topology;
        helper.ia_ci_.primitiveRestartEnable = VK_TRUE;
        helper.shader_stages_ = { vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo() };
    };

    topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-06252");

    if (m_device->phy().features().tessellationShader) {
        topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        constexpr std::array vuids = {"VUID-VkPipelineInputAssemblyStateCreateInfo-topology-06253",
                                      "VUID-VkGraphicsPipelineCreateInfo-topology-08889"};
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
    }
}

TEST_F(NegativePipelineTopology, PatchListNoTessellation) {
    TEST_DESCRIPTION("Use VK_PRIMITIVE_TOPOLOGY_PATCH_LIST without tessellation shader");

    AddOptionalExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    if (!m_device->phy().features().tessellationShader) {
        GTEST_SKIP() << "Device does not support tessellation shaders";
    }

    auto set_info = [&](CreatePipelineHelper &helper) { helper.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST; };
    const char *vuid = IsExtensionsEnabled(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME)
                           ? "VUID-VkGraphicsPipelineCreateInfo-topology-08889"
                           : "VUID-VkGraphicsPipelineCreateInfo-topology-08889";
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuid);
}

TEST_F(NegativePipelineTopology, FillRectangleNV) {
    TEST_DESCRIPTION("Verify VK_NV_fill_rectangle");
    AddRequiredExtensions(VK_NV_FILL_RECTANGLE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFeatures device_features = {};
    GetPhysicalDeviceFeatures(&device_features);

    // Disable non-solid fill modes to make sure that the usage of VK_POLYGON_MODE_LINE and
    // VK_POLYGON_MODE_POINT will cause an error when the VK_NV_fill_rectangle extension is enabled.
    device_features.fillModeNonSolid = VK_FALSE;

    RETURN_IF_SKIP(InitState(&device_features));
    InitRenderTarget();

    VkPolygonMode polygon_mode = VK_POLYGON_MODE_LINE;

    auto set_polygon_mode = [&polygon_mode](CreatePipelineHelper &helper) { helper.rs_state_ci_.polygonMode = polygon_mode; };

    // Set unsupported polygon mode VK_POLYGON_MODE_LINE
    CreatePipelineHelper::OneshotTest(*this, set_polygon_mode, kErrorBit,
                                      "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01507");

    // Set unsupported polygon mode VK_POLYGON_MODE_POINT
    polygon_mode = VK_POLYGON_MODE_POINT;
    CreatePipelineHelper::OneshotTest(*this, set_polygon_mode, kErrorBit,
                                      "VUID-VkPipelineRasterizationStateCreateInfo-polygonMode-01507");

    // Set supported polygon mode VK_POLYGON_MODE_FILL
    polygon_mode = VK_POLYGON_MODE_FILL;
    CreatePipelineHelper::OneshotTest(*this, set_polygon_mode, kErrorBit);

    // Set supported polygon mode VK_POLYGON_MODE_FILL_RECTANGLE_NV
    polygon_mode = VK_POLYGON_MODE_FILL_RECTANGLE_NV;
    CreatePipelineHelper::OneshotTest(*this, set_polygon_mode, kErrorBit);
}
