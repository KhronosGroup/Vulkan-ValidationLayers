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

TEST_F(NegativeMesh, BasicUsage) {
    TEST_DESCRIPTION("Test VK_EXT_mesh_shader.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Create a device that enables mesh_shader
    auto vertex_input_dynamic_state = LvlInitStruct<VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT>();
    auto extended_dynamic_state2 = LvlInitStruct<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>(&vertex_input_dynamic_state);
    auto maintenance4 = LvlInitStruct<VkPhysicalDeviceMaintenance4Features>(&extended_dynamic_state2);
    auto multiview_feature = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>(&maintenance4);
    auto xfb_feature = LvlInitStruct<VkPhysicalDeviceTransformFeedbackFeaturesEXT>(&multiview_feature);
    auto mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesEXT>(&xfb_feature);
    auto features2 = GetPhysicalDeviceFeatures2(mesh_shader_features);
    mesh_shader_features.multiviewMeshShader = VK_FALSE;
    features2.features.multiDrawIndirect = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    static const char vert_shader_text[] = R"glsl(
        #version 460
        vec2 vertices[3];
        void main() {
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           gl_PointSize = 1.0f;
        }
    )glsl";

    // #version 460
    // #extension GL_EXT_mesh_shader : require
    // layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
    // layout(max_vertices = 3) out;
    // layout(max_primitives = 1) out;
    // layout(triangles) out;
    // void main() {
    //       SetMeshOutputsEXT(3,1);
    //       gl_MeshVerticesEXT[0].gl_Position = vec4(-1.0, -1.0, 0, 1);
    //       gl_MeshVerticesEXT[1].gl_Position = vec4( 1.0, -1.0, 0, 1);
    //       gl_MeshVerticesEXT[2].gl_Position = vec4( 0.0,  1.0, 0, 1);
    //       gl_PrimitiveTriangleIndicesEXT[0] = uvec3(0,1,2);
    // }
    static const char mesh_shader_text[] = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main" %gl_MeshVerticesEXT %gl_PrimitiveTriangleIndicesEXT
               OpExecutionMode %main LocalSize 1 1 1
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main OutputPrimitivesNV 1
               OpExecutionMode %main OutputTrianglesNV
               OpMemberDecorate %gl_MeshPerVertexEXT 0 BuiltIn Position
               OpMemberDecorate %gl_MeshPerVertexEXT 1 BuiltIn PointSize
               OpMemberDecorate %gl_MeshPerVertexEXT 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_MeshPerVertexEXT 3 BuiltIn CullDistance
               OpDecorate %gl_MeshPerVertexEXT Block
               OpDecorate %gl_PrimitiveTriangleIndicesEXT BuiltIn PrimitiveTriangleIndicesEXT
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_MeshPerVertexEXT = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_arr_gl_MeshPerVertexEXT_uint_3 = OpTypeArray %gl_MeshPerVertexEXT %uint_3
%_ptr_Output__arr_gl_MeshPerVertexEXT_uint_3 = OpTypePointer Output %_arr_gl_MeshPerVertexEXT_uint_3
%gl_MeshVerticesEXT = OpVariable %_ptr_Output__arr_gl_MeshPerVertexEXT_uint_3 Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
   %float_n1 = OpConstant %float -1
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
         %21 = OpConstantComposite %v4float %float_n1 %float_n1 %float_0 %float_1
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %int_1 = OpConstant %int 1
         %25 = OpConstantComposite %v4float %float_1 %float_n1 %float_0 %float_1
      %int_2 = OpConstant %int 2
         %28 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
     %v3uint = OpTypeVector %uint 3
%_arr_v3uint_uint_3 = OpTypeArray %v3uint %uint_3
%_ptr_Output__arr_v3uint_uint_3 = OpTypePointer Output %_arr_v3uint_uint_3
%gl_PrimitiveTriangleIndicesEXT = OpVariable %_ptr_Output__arr_v3uint_uint_3 Output
     %uint_0 = OpConstant %uint 0
     %uint_2 = OpConstant %uint 2
         %36 = OpConstantComposite %v3uint %uint_0 %uint_1 %uint_2
%_ptr_Output_v3uint = OpTypePointer Output %v3uint
         %39 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpSetMeshOutputsEXT %uint_3 %uint_1
         %23 = OpAccessChain %_ptr_Output_v4float %gl_MeshVerticesEXT %int_0 %int_0
               OpStore %23 %21
         %26 = OpAccessChain %_ptr_Output_v4float %gl_MeshVerticesEXT %int_1 %int_0
               OpStore %26 %25
         %29 = OpAccessChain %_ptr_Output_v4float %gl_MeshVerticesEXT %int_2 %int_0
               OpStore %29 %28
         %38 = OpAccessChain %_ptr_Output_v3uint %gl_PrimitiveTriangleIndicesEXT %int_0
               OpStore %38 %36
               OpReturn
               OpFunctionEnd
    )";

    static const char mesh_shader_xfb_text[] = R"(
               OpCapability MeshShadingEXT
               OpCapability TransformFeedback
               OpExtension "SPV_EXT_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main" %tf
               OpExecutionModeId %main LocalSizeId %uint_2 %uint_1 %uint_1
               OpExecutionMode %main Xfb
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main OutputPrimitivesEXT 1
               OpExecutionMode %main OutputTrianglesEXT
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_mesh_shader"
               OpName %main "main"
               OpName %tf "tf"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
         %tf = OpVariable %_ptr_Output_float Output
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
         %10 = OpConstantComposite %v3uint %uint_2 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj vs(this, vert_shader_text, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj ms(this, mesh_shader_text, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);
    VkShaderObj ms_xfb(this, mesh_shader_xfb_text, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Test pipeline creation
    {
        // can't mix mesh with vertex
        const auto break_vp = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo(), ms.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit,
                                          vector<std::string>({"VUID-VkGraphicsPipelineCreateInfo-pStages-02095"}));

        // vertex or mesh must be present
        // 02096 overlaps with 06896
        const auto break_vp2 = [&](CreatePipelineHelper &helper) { helper.shader_stages_ = {fs.GetStageCreateInfo()}; };
        CreatePipelineHelper::OneshotTest(*this, break_vp2, kErrorBit,
                                          vector<std::string>({"VUID-VkGraphicsPipelineCreateInfo-stage-02096",
                                                               "VUID-VkGraphicsPipelineCreateInfo-pStages-06896"}));

        // vertexinput and inputassembly must be valid when vertex stage is present
        const auto break_vp3 = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
            helper.gp_ci_.pVertexInputState = nullptr;
            helper.gp_ci_.pInputAssemblyState = nullptr;
        };
        CreatePipelineHelper::OneshotTest(
            *this, break_vp3, kErrorBit,
            vector<std::string>({"VUID-VkGraphicsPipelineCreateInfo-pStages-02097",
                                 "VUID-VkGraphicsPipelineCreateInfo-dynamicPrimitiveTopologyUnrestricted-09031"}));

        // xfb with mesh shader
        const auto break_vp4 = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {ms_xfb.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp4, kErrorBit,
                                          vector<std::string>({" VUID-VkGraphicsPipelineCreateInfo-None-02322"}));

        // invalid dynamic state with mesh shader
        vector<VkDynamicState> dyn_states[] = {
            {VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT},
            {VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT},
            {VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE_EXT},
            {VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT},
            {VK_DYNAMIC_STATE_VERTEX_INPUT_EXT},
        };
        const char *err_vuids[] = {
            "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07065", "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07065",
            "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07066", "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07066",
            "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07067"};
        VkPipelineDynamicStateCreateInfo dyn_state = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        for (int i = 0; i < 5; i++) {
            dyn_state.dynamicStateCount = dyn_states[i].size();
            dyn_state.pDynamicStates = dyn_states[i].data();
            if (!extended_dynamic_state2.extendedDynamicState2PatchControlPoints &&
                *dyn_state.pDynamicStates == VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT)
                continue;
            const auto break_vp5 = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
                helper.gp_ci_.pDynamicState = &dyn_state;
            };
            CreatePipelineHelper::OneshotTest(*this, break_vp5, kErrorBit, vector<std::string>({err_vuids[i]}));
        }

        // viewMask without enabling multiviewMeshShader feature
        auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
        pipeline_rendering_info.viewMask = 0x2;

        const auto break_vp5 = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
            helper.gp_ci_.pNext = &pipeline_rendering_info;
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp5, kErrorBit,
                                          vector<std::string>({"VUID-VkGraphicsPipelineCreateInfo-renderPass-07064"}));
    }
}

TEST_F(NegativeMesh, ExtensionDisabled) {
    TEST_DESCRIPTION("Test VK_EXT_mesh_shader VUs with EXT_mesh_shader disabled.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto maintenance4 = LvlInitStruct<VkPhysicalDeviceMaintenance4Features>();
    auto mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesEXT>(&maintenance4);
    GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (mesh_shader_features.meshShader != VK_TRUE) {
        GTEST_SKIP() << "Mesh shader feature not supported";
    }

    mesh_shader_features.meshShader = VK_FALSE;
    mesh_shader_features.taskShader = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &mesh_shader_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);

    // #version 450
    // #extension GL_EXT_mesh_shader : require
    // layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
    // struct Task {
    //     uint baseID;
    // };
    // taskPayloadSharedEXT Task OUT;
    // void main() {
    //     OUT.baseID = 1;
    // }
    const char *task_src = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TaskEXT %main "main" %OUT
               OpExecutionModeId %main LocalSizeId %uint_1 %uint_1 %uint_1
               OpMemberName %Task 0 "baseID"
               OpName %OUT "OUT"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
       %Task = OpTypeStruct %uint
%_ptr_TaskPayloadWorkgroupEXT_Task = OpTypePointer TaskPayloadWorkgroupEXT %Task
        %OUT = OpVariable %_ptr_TaskPayloadWorkgroupEXT_Task TaskPayloadWorkgroupEXT
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_TaskPayloadWorkgroupEXT_uint = OpTypePointer TaskPayloadWorkgroupEXT %uint
     %v3uint = OpTypeVector %uint 3
         %16 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
         %14 = OpAccessChain %_ptr_TaskPayloadWorkgroupEXT_uint %OUT %int_0
               OpStore %14 %uint_1
               OpReturn
               OpFunctionEnd
    )";

    // #version 450
    // #extension GL_EXT_mesh_shader : enable
    // layout (triangles) out;
    // layout (max_vertices = 3, max_primitives = 1) out;
    // struct Task {
    //     uint baseID;
    // };
    // taskPayloadSharedEXT Task IN;
    // void main() {}
    const char *mesh_src = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main" %IN
               OpExecutionMode %main LocalSize 1 1 1
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main OutputPrimitivesEXT 1
               OpExecutionMode %main OutputTrianglesEXT
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_mesh_shader"
               OpName %main "main"
               OpName %Task "Task"
               OpMemberName %Task 0 "baseID"
               OpName %IN "IN"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
       %Task = OpTypeStruct %uint
%_ptr_TaskPayloadWorkgroupEXT_Task = OpTypePointer TaskPayloadWorkgroupEXT %Task
         %IN = OpVariable %_ptr_TaskPayloadWorkgroupEXT_Task TaskPayloadWorkgroupEXT
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj task_shader(this, task_src, VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);
    VkShaderObj mesh_shader(this, mesh_src, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    // mesh and task shaders not supported
    const auto break_vp = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {task_shader.GetStageCreateInfo(), mesh_shader.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit,
                                      vector<std::string>({"VUID-VkPipelineShaderStageCreateInfo-stage-02091",
                                                           "VUID-VkPipelineShaderStageCreateInfo-stage-02092"}));
}

TEST_F(NegativeMesh, RuntimeSpirv) {
    TEST_DESCRIPTION("Test VK_EXT_mesh_shader spirv related VUIDs.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto maintenance4 = LvlInitStruct<VkPhysicalDeviceMaintenance4Features>();
    auto mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesEXT>(&maintenance4);
    GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (mesh_shader_features.meshShader != VK_TRUE) {
        GTEST_SKIP() << "Mesh shader feature not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &mesh_shader_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = LvlInitStruct<VkPhysicalDeviceMeshShaderPropertiesEXT>();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    vector<std::string> error_vuids, error_vuids_1;
    uint32_t max_task_workgroup_size_x = mesh_shader_properties.maxTaskWorkGroupSize[0];
    uint32_t max_task_workgroup_size_y = mesh_shader_properties.maxTaskWorkGroupSize[1];
    uint32_t max_task_workgroup_size_z = mesh_shader_properties.maxTaskWorkGroupSize[2];

    uint32_t max_mesh_workgroup_size_x = mesh_shader_properties.maxMeshWorkGroupSize[0];
    uint32_t max_mesh_workgroup_size_y = mesh_shader_properties.maxMeshWorkGroupSize[1];
    uint32_t max_mesh_workgroup_size_z = mesh_shader_properties.maxMeshWorkGroupSize[2];

    uint32_t max_mesh_output_vertices = mesh_shader_properties.maxMeshOutputVertices;
    uint32_t max_mesh_output_primitives = mesh_shader_properties.maxMeshOutputPrimitives;

    if (max_task_workgroup_size_x < vvl::MaxTypeValue(max_task_workgroup_size_x)) {
        error_vuids.push_back("VUID-RuntimeSpirv-TaskEXT-07291");
        max_task_workgroup_size_x += 1;
    }

    if (max_task_workgroup_size_y < vvl::MaxTypeValue(max_task_workgroup_size_y)) {
        error_vuids.push_back("VUID-RuntimeSpirv-TaskEXT-07292");
        max_task_workgroup_size_y += 1;
    }

    if (max_task_workgroup_size_z < vvl::MaxTypeValue(max_task_workgroup_size_z)) {
        error_vuids.push_back("VUID-RuntimeSpirv-TaskEXT-07293");
        max_task_workgroup_size_z += 1;
    }
    error_vuids.push_back("VUID-RuntimeSpirv-TaskEXT-07294");

    if (max_mesh_workgroup_size_x < vvl::MaxTypeValue(max_mesh_workgroup_size_x)) {
        error_vuids.push_back("VUID-RuntimeSpirv-MeshEXT-07295");
        max_mesh_workgroup_size_x += 1;
    }

    if (max_mesh_workgroup_size_y < vvl::MaxTypeValue(max_mesh_workgroup_size_y)) {
        error_vuids.push_back("VUID-RuntimeSpirv-MeshEXT-07296");
        max_mesh_workgroup_size_y += 1;
    }

    if (max_mesh_workgroup_size_z < vvl::MaxTypeValue(max_mesh_workgroup_size_z)) {
        error_vuids.push_back("VUID-RuntimeSpirv-MeshEXT-07297");
        max_mesh_workgroup_size_z += 1;
    }
    error_vuids.push_back("VUID-RuntimeSpirv-MeshEXT-07298");

    if (max_mesh_output_vertices < vvl::MaxTypeValue(max_mesh_output_vertices)) {
        error_vuids_1.push_back("VUID-RuntimeSpirv-MeshEXT-07115");
        max_mesh_output_vertices += 1;
    }

    if (max_mesh_output_primitives < vvl::MaxTypeValue(max_mesh_output_primitives)) {
        error_vuids_1.push_back("VUID-RuntimeSpirv-MeshEXT-07116");
        max_mesh_output_primitives += 1;
    }

    std::string task_src = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TaskEXT %main "main" %OUT
               OpExecutionMode %main LocalSize )";
    task_src += std::to_string(max_task_workgroup_size_x);
    task_src += " ";
    task_src += std::to_string(max_task_workgroup_size_y);
    task_src += " ";
    task_src += std::to_string(max_task_workgroup_size_z);
    task_src += R"(
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
          %9 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %Task = OpTypeStruct %uint
%_ptr_TaskPayloadWorkgroupEXT_Task = OpTypePointer TaskPayloadWorkgroupEXT %Task
        %OUT = OpVariable %_ptr_TaskPayloadWorkgroupEXT_Task TaskPayloadWorkgroupEXT
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    std::string mesh_src = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main"
               OpExecutionMode %main LocalSize )";
    mesh_src += std::to_string(max_mesh_workgroup_size_x);
    mesh_src += " ";
    mesh_src += std::to_string(max_mesh_workgroup_size_y);
    mesh_src += " ";
    mesh_src += std::to_string(max_mesh_workgroup_size_z);
    mesh_src += R"(
               OpExecutionMode %main OutputVertices 1
               OpExecutionMode %main OutputPrimitivesEXT 1
               OpExecutionMode %main OutputTrianglesEXT
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
          %9 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    std::string mesh_src_2 = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main"
               OpExecutionModeId %main LocalSizeId %uint_2 %uint_1 %uint_1
               OpExecutionMode %main OutputVertices )";
    mesh_src_2 += std::to_string(max_mesh_output_vertices);
    mesh_src_2 += R"(
               OpExecutionMode %main OutputPrimitivesEXT )";
    mesh_src_2 += std::to_string(max_mesh_output_primitives);
    mesh_src_2 += R"(
               OpExecutionMode %main OutputTrianglesEXT
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_mesh_shader"
               OpName %main "main"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
         %10 = OpConstantComposite %v3uint %uint_2 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj task_shader(this, task_src.c_str(), VK_SHADER_STAGE_TASK_BIT_EXT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);
    VkShaderObj mesh_shader(this, mesh_src.c_str(), VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);
    VkShaderObj mesh_shader_2(this, mesh_src_2.c_str(), VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    // mesh and task shaders which exceeds workgroup size limits
    const auto break_vp = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {task_shader.GetStageCreateInfo(), mesh_shader.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit, error_vuids);

    const auto break_vp1 = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {mesh_shader_2.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, break_vp1, kErrorBit, error_vuids_1);
}

TEST_F(NegativeMesh, RuntimeSpirvNV) {
    TEST_DESCRIPTION("Test VK_NV_mesh_shader spirv related VUIDs");

    AddRequiredExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesNV>();
    auto features2 = GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (mesh_shader_features.meshShader != VK_TRUE) {
        GTEST_SKIP() << "Mesh shader feature not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPhysicalDeviceMeshShaderPropertiesNV mesh_shader_properties = LvlInitStruct<VkPhysicalDeviceMeshShaderPropertiesNV>();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    std::string mesh_src = R"(
               OpCapability MeshShadingNV
               OpExtension "SPV_NV_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshNV %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpExecutionMode %main OutputVertices )";
    mesh_src += std::to_string(mesh_shader_properties.maxMeshOutputVertices + 1);
    mesh_src += R"(
               OpExecutionMode %main OutputPrimitivesNV )";
    mesh_src += std::to_string(mesh_shader_properties.maxMeshOutputPrimitives + 1);
    mesh_src += R"(
               OpExecutionMode %main OutputTrianglesNV
               OpSource GLSL 450
               OpSourceExtension "GL_NV_mesh_shader"
               OpName %main "main"
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj ms(this, mesh_src.c_str(), VK_SHADER_STAGE_MESH_BIT_NV, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto break_vp1 = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, break_vp1, kErrorBit,
                                      vector<std::string>({"VUID-RuntimeSpirv-MeshNV-07113", "VUID-RuntimeSpirv-MeshNV-07114"}));
}

TEST_F(NegativeMesh, BasicUsageNV) {
    TEST_DESCRIPTION("Test VK_NV_mesh_shader.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Create a device that enables mesh_shader
    auto mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesNV>();
    auto features2 = GetPhysicalDeviceFeatures2(mesh_shader_features);
    features2.features.multiDrawIndirect = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    static const char vertShaderText[] = R"glsl(
        #version 450
        vec2 vertices[3];
        void main() {
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           gl_PointSize = 1.0f;
        }
    )glsl";

    static const char meshShaderText[] = R"glsl(
        #version 450
        #extension GL_NV_mesh_shader : require
        layout(local_size_x = 1) in;
        layout(max_vertices = 3) out;
        layout(max_primitives = 1) out;
        layout(triangles) out;
        void main() {
              gl_MeshVerticesNV[0].gl_Position = vec4(-1.0, -1.0, 0, 1);
              gl_MeshVerticesNV[1].gl_Position = vec4( 1.0, -1.0, 0, 1);
              gl_MeshVerticesNV[2].gl_Position = vec4( 0.0,  1.0, 0, 1);
              gl_PrimitiveIndicesNV[0] = 0;
              gl_PrimitiveIndicesNV[1] = 1;
              gl_PrimitiveIndicesNV[2] = 2;
              gl_PrimitiveCountNV = 1;
        }
    )glsl";

    VkShaderObj vs(this, vertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj ms(this, meshShaderText, VK_SHADER_STAGE_MESH_BIT_NV);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Test pipeline creation
    {
        // can't mix mesh with vertex
        const auto break_vp = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo(), ms.GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit,
                                          vector<std::string>({"VUID-VkGraphicsPipelineCreateInfo-pStages-02095"}));

        // vertex or mesh must be present
        // 02096 overlaps with 06896
        const auto break_vp2 = [&](CreatePipelineHelper &helper) { helper.shader_stages_ = {fs.GetStageCreateInfo()}; };
        CreatePipelineHelper::OneshotTest(*this, break_vp2, kErrorBit,
                                          vector<std::string>({"VUID-VkGraphicsPipelineCreateInfo-stage-02096",
                                                               "VUID-VkGraphicsPipelineCreateInfo-pStages-06896"}));

        // vertexinput and inputassembly must be valid when vertex stage is present
        const auto break_vp3 = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
            helper.gp_ci_.pVertexInputState = nullptr;
            helper.gp_ci_.pInputAssemblyState = nullptr;
        };
        CreatePipelineHelper::OneshotTest(
            *this, break_vp3, kErrorBit,
            vector<std::string>({"VUID-VkGraphicsPipelineCreateInfo-pStages-02097",
                                 "VUID-VkGraphicsPipelineCreateInfo-dynamicPrimitiveTopologyUnrestricted-09031"}));
    }

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = sizeof(uint32_t);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    VkBuffer buffer;
    VkResult result = vk::CreateBuffer(m_device->device(), &buffer_create_info, nullptr, &buffer);
    ASSERT_VK_SUCCESS(result);

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02146");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02718");
    vk::CmdDrawMeshTasksIndirectNV(m_commandBuffer->handle(), buffer, 0, 2, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

    vk::DestroyBuffer(m_device->device(), buffer, 0);
}

TEST_F(NegativeMesh, ExtensionDisabledNV) {
    TEST_DESCRIPTION("Test VK_NV_mesh_shader VUs with NV_mesh_shader disabled.");

    AddRequiredExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesNV>();
    GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (mesh_shader_features.meshShader != VK_TRUE) {
        GTEST_SKIP() << "Mesh shader feature not supported";
    }

    mesh_shader_features.meshShader = VK_FALSE;
    mesh_shader_features.taskShader = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &mesh_shader_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    vk_testing::Event event_obj(*m_device);
    const auto event = event_obj.handle();
    ASSERT_TRUE(event_obj.initialized());

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetEvent-stageMask-04095");
    vk::CmdSetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetEvent-stageMask-04096");
    vk::CmdSetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetEvent-stageMask-04095");
    vk::CmdResetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdResetEvent-stageMask-04096");
    vk::CmdResetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-srcStageMask-04095");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-dstStageMask-04095");
    vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event, VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV,
                      VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV, 0, nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-srcStageMask-04096");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-dstStageMask-04096");
    vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event, VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV,
                      VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV, 0, nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-srcStageMask-04095");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-dstStageMask-04095");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV, VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV, 0,
                           0, nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-srcStageMask-04096");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-dstStageMask-04096");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV, VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV, 0,
                           0, nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

    vk_testing::Semaphore semaphore_obj(*m_device);
    const auto semaphore = semaphore_obj.handle();
    ASSERT_TRUE(semaphore_obj.initialized());

    VkPipelineStageFlags stage_flags = VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV | VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV;
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();

    // Signal the semaphore so the next test can wait on it.
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore;
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = nullptr;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore;
    submit_info.pWaitDstStageMask = &stage_flags;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pWaitDstStageMask-04095");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubmitInfo-pWaitDstStageMask-04096");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_device->m_queue);

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    // #version 450
    // #extension GL_NV_mesh_shader : require
    // layout(local_size_x = 1) in;
    // taskNV out Task {
    //     uint baseID;
    // } OUT;
    // void main() {
    //     OUT.baseID = 1;
    // }
    const char *task_src = R"(
               OpCapability MeshShadingNV
               OpExtension "SPV_NV_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TaskNV %main "main" %OUT
               OpExecutionMode %main LocalSize 1 1 1
               OpMemberDecorate %Task 0 PerTaskNV
               OpMemberDecorate %Task 0 Offset 0
               OpDecorate %Task Block
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
       %Task = OpTypeStruct %uint
%_ptr_Output_Task = OpTypePointer Output %Task
        %OUT = OpVariable %_ptr_Output_Task Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %uint_1 = OpConstant %uint 1
%_ptr_Output_uint = OpTypePointer Output %uint
     %v3uint = OpTypeVector %uint 3
    %uint_32 = OpConstant %uint 32
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_32 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
         %14 = OpAccessChain %_ptr_Output_uint %OUT %int_0
               OpStore %14 %uint_1
               OpReturn
               OpFunctionEnd
    )";

    static const char mesh_src[] = R"glsl(
        #version 450

        #extension GL_NV_mesh_shader : require

        layout(local_size_x = 1) in;
        layout(max_vertices = 3) out;
        layout(max_primitives = 1) out;
        layout(triangles) out;

        taskNV in Task {
          uint baseID;
        } IN;

        void main() {
        }
    )glsl";

    VkShaderObj task_shader(this, task_src, VK_SHADER_STAGE_TASK_BIT_NV, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    VkShaderObj mesh_shader(this, mesh_src, VK_SHADER_STAGE_MESH_BIT_NV);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    // mesh and task shaders not supported
    const auto break_vp = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {task_shader.GetStageCreateInfo(), mesh_shader.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit,
                                      vector<std::string>({"VUID-VkPipelineShaderStageCreateInfo-stage-02091",
                                                           "VUID-VkPipelineShaderStageCreateInfo-stage-02092"}));
}

TEST_F(NegativeMesh, DrawCmds) {
    TEST_DESCRIPTION("Test VK_EXT_mesh_shader draw commands.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto maintenance4 = LvlInitStruct<VkPhysicalDeviceMaintenance4Features>();
    auto mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesEXT>(&maintenance4);
    auto features2 = GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (mesh_shader_features.meshShader != VK_TRUE) {
        GTEST_SKIP() << "Mesh shader feature not supported";
    }
    features2.features.multiDrawIndirect = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = LvlInitStruct<VkPhysicalDeviceMeshShaderPropertiesEXT>();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    // #version 450
    // #extension GL_EXT_mesh_shader : enable
    // layout (triangles) out;
    // layout (max_vertices = 3, max_primitives = 1) out;
    // struct Task {
    //     uint baseID;
    // };
    // taskPayloadSharedEXT Task IN;
    // void main() {}
    const char *mesh_src = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main" %IN
               OpExecutionMode %main LocalSize 1 1 1
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main OutputPrimitivesEXT 1
               OpExecutionMode %main OutputTrianglesEXT
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_mesh_shader"
               OpName %main "main"
               OpName %Task "Task"
               OpMemberName %Task 0 "baseID"
               OpName %IN "IN"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
       %Task = OpTypeStruct %uint
%_ptr_TaskPayloadWorkgroupEXT_Task = OpTypePointer TaskPayloadWorkgroupEXT %Task
         %IN = OpVariable %_ptr_TaskPayloadWorkgroupEXT_Task TaskPayloadWorkgroupEXT
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj mesh_shader(this, mesh_src, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);
    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    buffer_create_info.size = 2 * sizeof(VkDrawMeshTasksIndirectCommandEXT);
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);

    VkBufferObj count_buffer;
    buffer_create_info.size = 64;
    count_buffer.init(*m_device, buffer_create_info);

    VkPipelineLayoutObj pipeline_layout(m_device);
    VkPipelineObj pipe(m_device);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&mesh_shader);
    pipe.AddShader(&fs);
    pipe.SetViewport(m_viewports);
    pipe.SetScissor(m_scissors);
    pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    VkPipelineObj pipe1(m_device);
    pipe1.AddDefaultColorAttachment();
    pipe1.AddShader(&vs);
    pipe1.AddShader(&fs);
    pipe1.SetViewport(m_viewports);
    pipe1.SetScissor(m_scissors);
    pipe1.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(renderPassBeginInfo());
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());

    uint32_t max_group_count_X = mesh_shader_properties.maxTaskWorkGroupCount[0];
    uint32_t max_group_count_Y = mesh_shader_properties.maxTaskWorkGroupCount[1];
    uint32_t max_group_count_Z = mesh_shader_properties.maxTaskWorkGroupCount[2];

    if (max_group_count_X < vvl::MaxTypeValue(max_group_count_X)) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07322");
        max_group_count_X = vvl::MaxTypeValue(max_group_count_X);
    }

    if (max_group_count_Y < vvl::MaxTypeValue(max_group_count_Y)) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07323");
        max_group_count_Y = vvl::MaxTypeValue(max_group_count_Y);
    }

    if (max_group_count_Z < vvl::MaxTypeValue(max_group_count_Z)) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07324");
        max_group_count_Z = vvl::MaxTypeValue(max_group_count_Z);
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksEXT-TaskEXT-07325");
    vk::CmdDrawMeshTasksEXT(m_commandBuffer->handle(), max_group_count_X, max_group_count_Y, max_group_count_Z);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-02718");
    vk::CmdDrawMeshTasksIndirectEXT(m_commandBuffer->handle(), buffer.handle(), 0, 2, sizeof(VkDrawMeshTasksIndirectCommandEXT));
    m_errorMonitor->VerifyFound();

    if (m_device->props.limits.maxDrawIndirectCount < vvl::MaxTypeValue(m_device->props.limits.maxDrawIndirectCount)) {
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-02718");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-02719");
        vk::CmdDrawMeshTasksIndirectEXT(m_commandBuffer->handle(), buffer.handle(), 0,
                                        m_device->props.limits.maxDrawIndirectCount + 1, sizeof(VkDrawMeshTasksIndirectCommandEXT));
        m_errorMonitor->VerifyFound();
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-07089");
    vk::CmdDrawMeshTasksIndirectEXT(m_commandBuffer->handle(), buffer.handle(), sizeof(VkDrawMeshTasksIndirectCommandEXT) * 2, 1,
                                    sizeof(VkDrawMeshTasksIndirectCommandEXT));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-stage-06481");
    m_commandBuffer->Draw(3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksEXT-stage-06480");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksEXT-MeshEXT-07087");
    vk::CmdDrawMeshTasksEXT(m_commandBuffer->handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectEXT-MeshEXT-07091");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectEXT-stage-06480");
    vk::CmdDrawMeshTasksIndirectEXT(m_commandBuffer->handle(), buffer.handle(), 0, 1, sizeof(VkDrawMeshTasksIndirectCommandEXT));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectCountEXT-MeshEXT-07100");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectCountEXT-stage-06480");
    vk::CmdDrawMeshTasksIndirectCountEXT(m_commandBuffer->handle(), buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                         sizeof(VkDrawMeshTasksIndirectCommandEXT));
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeMesh, MultiDrawIndirect) {
    TEST_DESCRIPTION("Test VK_EXT_mesh_shader indirect draw command.");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto maintenance4 = LvlInitStruct<VkPhysicalDeviceMaintenance4Features>();
    auto mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesEXT>(&maintenance4);
    auto features2 = GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (mesh_shader_features.meshShader != VK_TRUE) {
        GTEST_SKIP() << "Mesh shader feature not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPhysicalDeviceMeshShaderPropertiesEXT mesh_shader_properties = LvlInitStruct<VkPhysicalDeviceMeshShaderPropertiesEXT>();
    GetPhysicalDeviceProperties2(mesh_shader_properties);

    // #version 450
    // #extension GL_EXT_mesh_shader : require
    // layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
    // layout(max_vertices = 3, max_primitives = 1) out;
    // layout(triangles) out;
    // struct Task {
    //   uint baseID;
    // };
    // taskPayloadSharedEXT Task IN;
    // void main() {}
    static const char mesh_src[] = R"(
               OpCapability MeshShadingEXT
               OpExtension "SPV_EXT_mesh_shader"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint MeshEXT %main "main" %IN
               OpExecutionModeId %main LocalSizeId %uint_1 %uint_1 %uint_1
               OpExecutionMode %main OutputVertices 3
               OpExecutionMode %main OutputPrimitivesNV 1
               OpExecutionMode %main OutputTrianglesNV
               OpSource GLSL 450
               OpSourceExtension "GL_EXT_mesh_shader"
               OpName %main "main"
               OpName %Task "Task"
               OpMemberName %Task 0 "baseID"
               OpName %IN "IN"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
          %9 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %Task = OpTypeStruct %uint
%_ptr_TaskPayloadWorkgroupEXT_Task = OpTypePointer TaskPayloadWorkgroupEXT %Task
         %IN = OpVariable %_ptr_TaskPayloadWorkgroupEXT_Task TaskPayloadWorkgroupEXT
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj mesh_shader(this, mesh_src, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    buffer_create_info.size = 2 * sizeof(VkDrawMeshTasksIndirectCommandEXT);
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);

    VkPipelineLayoutObj pipeline_layout(m_device);
    VkPipelineObj pipe(m_device);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&mesh_shader);
    pipe.AddShader(&fs);
    pipe.SetViewport(m_viewports);
    pipe.SetScissor(m_scissors);
    pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(renderPassBeginInfo());
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-07088");
    vk::CmdDrawMeshTasksIndirectEXT(m_commandBuffer->handle(), buffer.handle(), 0, 2,
                                    sizeof(VkDrawMeshTasksIndirectCommandEXT) - 2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectEXT-drawCount-07090");
    vk::CmdDrawMeshTasksIndirectEXT(m_commandBuffer->handle(), buffer.handle(), 0, 4, sizeof(VkDrawMeshTasksIndirectCommandEXT));
    m_errorMonitor->VerifyFound();

    VkBufferObj draw_buffer;
    draw_buffer.init_no_mem(*m_device, buffer_create_info);
    ASSERT_TRUE(draw_buffer.initialized());
    draw_buffer.allocate_and_bind_memory(*m_device);

    VkBufferCreateInfo count_buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    count_buffer_create_info.size = 64;
    count_buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

    VkBufferObj count_buffer;
    count_buffer.init(*m_device, count_buffer_create_info);
    ASSERT_TRUE(count_buffer.initialized());

    VkBufferObj count_buffer_unbound;
    count_buffer_unbound.init_no_mem(*m_device, count_buffer_create_info);
    ASSERT_TRUE(count_buffer_unbound.initialized());

    VkBufferObj count_buffer_wrong_usage;
    count_buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    count_buffer_wrong_usage.init(*m_device, count_buffer_create_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectCountEXT-countBuffer-02714");
    vk::CmdDrawMeshTasksIndirectCountEXT(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer_unbound.handle(), 0, 1,
                                         sizeof(VkDrawMeshTasksIndirectCommandEXT));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectCountEXT-countBuffer-02715");
    vk::CmdDrawMeshTasksIndirectCountEXT(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer_wrong_usage.handle(), 0,
                                         1, sizeof(VkDrawMeshTasksIndirectCommandEXT));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectCountEXT-stride-07096");
    vk::CmdDrawMeshTasksIndirectCountEXT(m_commandBuffer->handle(), draw_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                         sizeof(VkDrawMeshTasksIndirectCommandEXT) - 3);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectCountEXT-maxDrawCount-07097");
    vk::CmdDrawMeshTasksIndirectCountEXT(m_commandBuffer->handle(), draw_buffer.handle(),
                                         2 * sizeof(VkDrawMeshTasksIndirectCommandEXT), count_buffer.handle(), 0, 4,
                                         sizeof(VkDrawMeshTasksIndirectCommandEXT));
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeMesh, DrawCmdsNV) {
    TEST_DESCRIPTION("Test VK_NV_mesh_shader draw commands.");

    AddRequiredExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesNV>();
    auto features2 = GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (mesh_shader_features.meshShader != VK_TRUE) {
        GTEST_SKIP() << "Mesh shader feature not supported";
    }
    features2.features.multiDrawIndirect = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    static const char mesh_src[] = R"glsl(
        #version 450
        #extension GL_NV_mesh_shader : require
        layout(local_size_x = 1) in;
        layout(max_vertices = 3) out;
        layout(max_primitives = 1) out;
        layout(triangles) out;
        void main() {
              gl_MeshVerticesNV[0].gl_Position = vec4(-1.0, -1.0, 0, 1);
              gl_MeshVerticesNV[1].gl_Position = vec4( 1.0, -1.0, 0, 1);
              gl_MeshVerticesNV[2].gl_Position = vec4( 0.0,  1.0, 0, 1);
              gl_PrimitiveIndicesNV[0] = 0;
              gl_PrimitiveIndicesNV[1] = 1;
              gl_PrimitiveIndicesNV[2] = 2;
              gl_PrimitiveCountNV = 1;
        }
    )glsl";

    VkShaderObj mesh_shader(this, mesh_src, VK_SHADER_STAGE_MESH_BIT_NV);
    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    buffer_create_info.size = 2 * sizeof(VkDrawMeshTasksIndirectCommandNV);
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);

    VkBufferObj count_buffer;
    buffer_create_info.size = 64;
    count_buffer.init(*m_device, buffer_create_info);

    VkPipelineLayoutObj pipeline_layout(m_device);
    VkPipelineObj pipe(m_device);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&mesh_shader);
    pipe.AddShader(&fs);
    pipe.SetViewport(m_viewports);
    pipe.SetScissor(m_scissors);
    pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    VkPipelineObj pipe1(m_device);
    pipe1.AddDefaultColorAttachment();
    pipe1.AddShader(&vs);
    pipe1.AddShader(&fs);
    pipe1.SetViewport(m_viewports);
    pipe1.SetScissor(m_scissors);
    pipe1.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(renderPassBeginInfo());
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectNV-drawCount-02156");
    vk::CmdDrawMeshTasksIndirectNV(m_commandBuffer->handle(), buffer.handle(), sizeof(VkDrawMeshTasksIndirectCommandNV) * 2, 1,
                                   sizeof(VkDrawMeshTasksIndirectCommandNV));
    m_errorMonitor->VerifyFound();

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.handle());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksNV-MeshNV-07080");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksNV-stage-06480");
    vk::CmdDrawMeshTasksNV(m_commandBuffer->handle(), 1, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectNV-MeshNV-07081");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectNV-stage-06480");
    vk::CmdDrawMeshTasksIndirectNV(m_commandBuffer->handle(), buffer.handle(), 0, 1, sizeof(VkDrawMeshTasksIndirectCommandNV));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectCountNV-MeshNV-07082");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksIndirectCountNV-stage-06480");
    vk::CmdDrawMeshTasksIndirectCountNV(m_commandBuffer->handle(), buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                        sizeof(VkDrawMeshTasksIndirectCommandNV));
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}
