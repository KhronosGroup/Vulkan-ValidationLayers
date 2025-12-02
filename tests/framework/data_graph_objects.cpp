/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (C) 2025 Arm Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "data_graph_objects.h"
#include "binding.h"
#include "generated/pnext_chain_extraction.h"
#include "shader_templates.h"
#include <iostream>

namespace vkt {
namespace dg {

void DataGraphPipelineHelper::CreateShaderModule(const char* spirv_source, const char* entrypoint) {
    spvtools::SpirvTools tools{SPV_ENV_UNIVERSAL_1_6};

    std::string error_msg;
    tools.SetMessageConsumer([&](spv_message_level_t, const char*, const spv_position_t& position, const char* message) {
        std::stringstream ss;
        ss << "on line " << position.line << ", column " << position.column << ": " << message;
        error_msg = ss.str();
    });

    // TODO - Replace with ASMtoSPV
    std::vector<uint32_t> spirv_binary;
    if (!tools.Assemble(spirv_source, &spirv_binary)) {
        GTEST_FAIL() << "Failed to compile SPIRV shader module. Error:\n" << error_msg << std::endl
        << "SpirV:\n" << spirv_source << std::endl;
    }

    VkShaderModuleCreateInfo shader_module_create_info = vku::InitStructHelper();
    shader_module_create_info.codeSize = spirv_binary.size() * sizeof(uint32_t);
    shader_module_create_info.pCode = spirv_binary.data();

    shader_.Init(*device_, shader_module_create_info);
    shader_module_ci_ = vku::InitStructHelper();
    shader_module_ci_.module = shader_.handle();
    shader_module_ci_.pName = entrypoint ? entrypoint : "main";

    vvl::PnextChainAdd(&pipeline_ci_, &shader_module_ci_);
}

std::string DataGraphPipelineHelper::GetSpirvMultiEntryTwoDataGraph() {
    return R"(
                                  OpCapability GraphARM
                                  OpCapability TensorsARM
                                  OpCapability Int8
                                  OpCapability Shader
                                  OpCapability VulkanMemoryModel
                                  OpCapability Matrix
                                  OpExtension "SPV_ARM_graph"
                                  OpExtension "SPV_ARM_tensors"
                                  OpExtension "SPV_KHR_vulkan_memory_model"
                          %tosa = OpExtInstImport "TOSA.001000.1"
                                  OpMemoryModel Logical Vulkan
                                  OpName %main_arg_0 "main_arg_0"
                                  OpName %main_res_0 "main_res_0"
                                  OpDecorate %main_arg_0 Binding 0
                                  OpDecorate %main_arg_0 DescriptorSet 0
                                  OpDecorate %main_res_0 Binding 1
                                  OpDecorate %main_res_0 DescriptorSet 0
                         %uchar = OpTypeInt 8 0
                          %uint = OpTypeInt 32 0
                       %uchar_0 = OpConstant %uchar 0
                        %uint_4 = OpConstant %uint 4
                        %uint_1 = OpConstant %uint 1
                        %uint_8 = OpConstant %uint 8
                       %uint_16 = OpConstant %uint 16
                        %uint_0 = OpConstant %uint 0
                        %uint_2 = OpConstant %uint 2
                    %uint_arr_4 = OpTypeArray %uint %uint_4
                    %uint_arr_1 = OpTypeArray %uint %uint_1
           %uint_arr_4_1_8_16_4 = OpConstantComposite %uint_arr_4 %uint_1 %uint_8 %uint_16 %uint_4
            %uint_arr_4_1_2_4_4 = OpConstantComposite %uint_arr_4 %uint_1 %uint_2 %uint_4 %uint_4
            %uint_arr_4_1_4_8_4 = OpConstantComposite %uint_arr_4 %uint_1 %uint_4 %uint_8 %uint_4
                  %uint_arr_1_1 = OpConstantComposite %uint_arr_1 %uint_1
                  %uint_arr_1_2 = OpConstantComposite %uint_arr_1 %uint_2
                  %uint_arr_1_4 = OpConstantComposite %uint_arr_1 %uint_4
         %uchar_1_8_16_4_tensor = OpTypeTensorARM %uchar %uint_4 %uint_arr_4_1_8_16_4
          %uchar_1_2_4_4_tensor = OpTypeTensorARM %uchar %uint_4 %uint_arr_4_1_2_4_4
          %uchar_1_4_8_4_tensor = OpTypeTensorARM %uchar %uint_4 %uint_arr_4_1_4_8_4
                %uchar_1_tensor = OpTypeTensorARM %uchar %uint_1 %uint_arr_1_1
                 %uint_2_tensor = OpTypeTensorARM %uint %uint_1 %uint_arr_1_2
                 %uint_4_tensor = OpTypeTensorARM %uint %uint_1 %uint_arr_1_4
             %uint_2_tensor_2_2 = OpConstantComposite %uint_2_tensor %uint_2 %uint_2
         %uint_4_tensor_0_0_0_0 = OpConstantComposite %uint_4_tensor %uint_0 %uint_0 %uint_0 %uint_0
              %uchar_1_tensor_0 = OpConstantComposite %uchar_1_tensor %uchar_0
     %uchar_1_8_16_4_tensor_ptr = OpTypePointer UniformConstant %uchar_1_8_16_4_tensor
      %uchar_1_2_4_4_tensor_ptr = OpTypePointer UniformConstant %uchar_1_2_4_4_tensor
                    %main_arg_0 = OpVariable %uchar_1_8_16_4_tensor_ptr UniformConstant
                    %main_res_0 = OpVariable %uchar_1_2_4_4_tensor_ptr UniformConstant
                    %graph_type = OpTypeGraphARM 1 %uchar_1_8_16_4_tensor %uchar_1_2_4_4_tensor
; entrypoint 1: MAX_POOL2D -> MAX_POOL2D
                       %graph_1 = OpGraphARM %graph_type
                          %in_0 = OpGraphInputARM %uchar_1_8_16_4_tensor %uint_0
                          %op_0 = OpExtInst %uchar_1_4_8_4_tensor %tosa MAX_POOL2D %uint_2_tensor_2_2 %uint_2_tensor_2_2 %uint_4_tensor_0_0_0_0 %uint_0 %in_0
                          %op_1 = OpExtInst %uchar_1_2_4_4_tensor %tosa MAX_POOL2D %uint_2_tensor_2_2 %uint_2_tensor_2_2 %uint_4_tensor_0_0_0_0 %uint_0 %op_0
                                  OpGraphSetOutputARM %op_1 %uint_0
                                  OpGraphEndARM
                                  OpGraphEntryPointARM %graph_1 "entrypoint_1" %main_arg_0 %main_res_0
; entrypoint 2: AVG_POOL2D -> AVG_POOL2D
                       %graph_2 = OpGraphARM %graph_type
                          %in_1 = OpGraphInputARM %uchar_1_8_16_4_tensor %uint_0
                          %op_2 = OpExtInst %uchar_1_4_8_4_tensor %tosa AVG_POOL2D %uint_2_tensor_2_2 %uint_2_tensor_2_2 %uint_4_tensor_0_0_0_0 %uint_1 %in_1 %uchar_1_tensor_0 %uchar_1_tensor_0
                          %op_3 = OpExtInst %uchar_1_2_4_4_tensor %tosa AVG_POOL2D %uint_2_tensor_2_2 %uint_2_tensor_2_2 %uint_4_tensor_0_0_0_0 %uint_1 %op_2 %uchar_1_tensor_0 %uchar_1_tensor_0
                                  OpGraphSetOutputARM %op_3 %uint_0
                                  OpGraphEndARM
                                  OpGraphEntryPointARM %graph_2 "entrypoint_2" %main_arg_0 %main_res_0
)";
}

// Spirv source. For testing purposes it includes:
// - unused OpGraphConstantARM
// - `inserted_line` to cause different errors
std::string DataGraphPipelineHelper::GetSpirvBasicDataGraph(const char* inserted_line) {
    std::stringstream ss;
    ss << R"(
                                  OpCapability GraphARM
                                  OpCapability TensorsARM
                                  OpCapability Int8
                                  OpCapability Shader
                                  OpCapability VulkanMemoryModel
                                  OpCapability Matrix
                                  OpExtension "SPV_ARM_graph"
                                  OpExtension "SPV_ARM_tensors"
                                  OpExtension "SPV_KHR_vulkan_memory_model"
                          %tosa = OpExtInstImport "TOSA.001000.1"
                                  OpMemoryModel Logical Vulkan
                                  OpName %main_arg_0 "main_arg_0"
                                  OpName %main_res_0 "main_res_0"
                                  OpDecorate %main_arg_0 Binding 0
                                  OpDecorate %main_arg_0 DescriptorSet 0
                                  OpDecorate %main_res_0 Binding 1
                                  OpDecorate %main_res_0 DescriptorSet 0
                         %uchar = OpTypeInt 8 0
                          %uint = OpTypeInt 32 0
                        %uint_4 = OpConstant %uint 4
                        %uint_1 = OpConstant %uint 1
                        %uint_8 = OpConstant %uint 8
                       %uint_16 = OpConstant %uint 16
                        %uint_0 = OpConstant %uint 0
                        %uint_2 = OpConstant %uint 2
                    %uint_arr_4 = OpTypeArray %uint %uint_4
                    %uint_arr_1 = OpTypeArray %uint %uint_1
           %uint_arr_4_1_8_16_4 = OpConstantComposite %uint_arr_4 %uint_1 %uint_8 %uint_16 %uint_4
            %uint_arr_4_1_2_4_4 = OpConstantComposite %uint_arr_4 %uint_1 %uint_2 %uint_4 %uint_4
            %uint_arr_4_1_4_8_4 = OpConstantComposite %uint_arr_4 %uint_1 %uint_4 %uint_8 %uint_4
                  %uint_arr_1_2 = OpConstantComposite %uint_arr_1 %uint_2
                  %uint_arr_1_4 = OpConstantComposite %uint_arr_1 %uint_4
         %uchar_1_8_16_4_tensor = OpTypeTensorARM %uchar %uint_4 %uint_arr_4_1_8_16_4
          %uchar_1_2_4_4_tensor = OpTypeTensorARM %uchar %uint_4 %uint_arr_4_1_2_4_4
          %uchar_1_4_8_4_tensor = OpTypeTensorARM %uchar %uint_4 %uint_arr_4_1_4_8_4
                 %uint_2_tensor = OpTypeTensorARM %uint %uint_1 %uint_arr_1_2
                 %uint_4_tensor = OpTypeTensorARM %uint %uint_1 %uint_arr_1_4
)" << inserted_line << R"(
                     %constant0 = OpGraphConstantARM %uint_2_tensor 1
                     %constant1 = OpGraphConstantARM %uint_4_tensor 0
             %uint_2_tensor_2_2 = OpConstantComposite %uint_2_tensor %uint_2 %uint_2
         %uint_4_tensor_0_0_0_0 = OpConstantComposite %uint_4_tensor %uint_0 %uint_0 %uint_0 %uint_0
     %uchar_1_8_16_4_tensor_ptr = OpTypePointer UniformConstant %uchar_1_8_16_4_tensor
      %uchar_1_2_4_4_tensor_ptr = OpTypePointer UniformConstant %uchar_1_2_4_4_tensor
                    %main_arg_0 = OpVariable %uchar_1_8_16_4_tensor_ptr UniformConstant
                    %main_res_0 = OpVariable %uchar_1_2_4_4_tensor_ptr UniformConstant
                    %graph_type = OpTypeGraphARM 1 %uchar_1_8_16_4_tensor %uchar_1_2_4_4_tensor
                                  OpGraphEntryPointARM %graph_0 "main" %main_arg_0 %main_res_0
                       %graph_0 = OpGraphARM %graph_type
                          %in_0 = OpGraphInputARM %uchar_1_8_16_4_tensor %uint_0
                          %op_0 = OpExtInst %uchar_1_4_8_4_tensor %tosa MAX_POOL2D  %uint_2_tensor_2_2 %uint_2_tensor_2_2 %uint_4_tensor_0_0_0_0 %uint_0 %in_0
                          %op_1 = OpExtInst %uchar_1_2_4_4_tensor %tosa MAX_POOL2D  %uint_2_tensor_2_2 %uint_2_tensor_2_2 %uint_4_tensor_0_0_0_0 %uint_0 %op_0
                                  OpGraphSetOutputARM %op_1 %uint_0
                                  OpGraphEndARM
)";

    return ss.str();
}

// A command shader (for the majority of cases) that can be modified inserting
// instructions in given sections. Without any insertions it's a basic shader.
std::string DataGraphPipelineHelper::GetSpirvModifiableShader(const ModifiableShaderParameters& params) {
    std::stringstream ss;
    ss << R"(
; SPIRV
; Version: 1.6
; Generator: Khronos Glslang Reference Front End; 11
; Bound: 19
; Schema: 0
               OpCapability Shader
)" << params.capabilities << R"(
               OpCapability TensorsARM
               OpExtension "SPV_ARM_tensors"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %tens
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpSourceExtension "GL_ARM_tensors"
               OpSourceExtension "GL_EXT_shader_explicit_arithmetic_types"
               OpName %main "main"
               OpName %size_x "size_x"
               OpName %tens "tens"
               OpDecorate %tens Binding 0
               OpDecorate %tens DescriptorSet 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
        %int = OpTypeInt 32 1
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
         %11 = OpTypeTensorARM %int %uint_1
)" << params.types << R"(
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
       %tens = OpVariable %_ptr_UniformConstant_11 UniformConstant
     %v3uint = OpTypeVector %uint 3
         %18 = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
     %size_x = OpVariable %_ptr_Function_uint Function
%loaded_tens = OpLoad %11 %tens
         %16 = OpTensorQuerySizeARM %uint %loaded_tens %uint_0
               OpStore %size_x %16
)" << params.instructions << R"(
               OpReturn
               OpFunctionEnd
)";
    return ss.str();
}

// spirv using a descriptor array
std::string DataGraphPipelineHelper::GetSpirvTensorArrayDataGraph(bool is_runtime) {
    std::stringstream ss;
    ss << R"(
                            OpCapability GraphARM
                            OpCapability TensorsARM
)" << (is_runtime ? "OpCapability RuntimeDescriptorArray" : "")
       << R"(
                            OpCapability Int8
                            OpCapability Shader
                            OpCapability VulkanMemoryModel
                            OpCapability Matrix
                            OpExtension "SPV_ARM_graph"
                            OpExtension "SPV_ARM_tensors"
                            OpExtension "SPV_KHR_vulkan_memory_model"
                    %tosa = OpExtInstImport "TOSA.001000.1"
                            OpMemoryModel Logical Vulkan
                            OpName %main_arg_0 "main_arg_0"
                            OpName %main_res_0 "main_res_0"
                            OpDecorate %main_arg_0 Binding 0
                            OpDecorate %main_arg_0 DescriptorSet 0
                            OpDecorate %main_res_0 Binding 1
                            OpDecorate %main_res_0 DescriptorSet 0
                      %i8 = OpTypeInt 8 0
                     %i32 = OpTypeInt 32 0
                   %i32_0 = OpConstant %i32 0
                   %i32_1 = OpConstant %i32 1
                   %i32_2 = OpConstant %i32 2
                   %i32_4 = OpConstant %i32 4
               %i32_arr_4 = OpTypeArray %i32 %i32_4
            %tensor_shape = OpConstantComposite %i32_arr_4 %i32_1 %i32_4 %i32_4 %i32_2
                  %tensor = OpTypeTensorARM %i8 %i32_4 %tensor_shape
)" << (is_runtime ? "%tensor_array = OpTypeRuntimeArray %tensor"
                  : "%tensor_array = OpTypeArray %tensor %i32_2")
       << R"(
        %ptr_tensor_array = OpTypePointer UniformConstant %tensor_array
              %ptr_tensor = OpTypePointer UniformConstant %tensor
              %main_arg_0 = OpVariable %ptr_tensor_array UniformConstant
              %main_res_0 = OpVariable %ptr_tensor UniformConstant
              %graph_type = OpTypeGraphARM 1 %tensor_array %tensor
                            OpGraphEntryPointARM %graph_0 "main" %main_arg_0 %main_res_0
                 %graph_0 = OpGraphARM %graph_type
                    %in_0 = OpGraphInputARM %tensor %i32_0 %i32_0
                    %in_1 = OpGraphInputARM %tensor %i32_0 %i32_1
                   %out_0 = OpExtInst %tensor %tosa ADD %in_0 %in_1
                            OpGraphSetOutputARM %out_0 %i32_0
                            OpGraphEndARM
)";
    return ss.str();
}

// shapes for 2-layer maxpool 2x2: output tensor is 1/4 the size of the input tensor
const std::vector<int64_t> in_tensor_dims = {1, 8, 16, 4};
const std::vector<int64_t> out_tensor_dims = {in_tensor_dims[0], in_tensor_dims[1] / 4, in_tensor_dims[2] / 4, in_tensor_dims[3]};

void DataGraphPipelineHelper::InitPipelineResources(VkDescriptorType desc_type) {
    if (params_.graph_variant == AddTensorArraySpirv || params_.graph_variant == AddRuntimeTensorArraySpirv) {

        // tensors for GetSpirvTensorArrayDataGraph(): array of 2 input, 1 output

        const std::vector<int64_t> dimensions{1, 4, 4, 2};
        VkTensorDescriptionARM desc = vku::InitStructHelper();
        desc.tiling = VK_TENSOR_TILING_LINEAR_ARM;
        desc.format = VK_FORMAT_R32_SINT;
        desc.dimensionCount = dimensions.size();
        desc.pDimensions = dimensions.data();
        desc.pStrides = nullptr;
        desc.usage = VK_TENSOR_USAGE_DATA_GRAPH_BIT_ARM;

        tensors_.resize(3);
        tensors_[0] = std::make_shared<vkt::Tensor>(*device_, desc);
        tensors_[1] = std::make_shared<vkt::Tensor>(*device_, desc);
        tensors_[2] = std::make_shared<vkt::Tensor>(*device_, desc);

        resources_.resize(3);
        // last 3 numbers are: descriptor, binding, array index
        // 2 input tensors, as array
        resources_[0] = {VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_RESOURCE_INFO_ARM, &tensors_[0]->Description(), 0, 0, 0};
        resources_[1] = {VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_RESOURCE_INFO_ARM, &tensors_[1]->Description(), 0, 0, 1};
        // 1 output tensor
        resources_[2] = {VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_RESOURCE_INFO_ARM, &tensors_[2]->Description(), 1, 0, 0};

        // binding 0: 2 x inputs; binding 1: 1 x output
        descriptor_set_layout_bindings_.resize(2);
        descriptor_set_layout_bindings_[0] = {0, desc_type, 2, VK_SHADER_STAGE_ALL, nullptr};
        descriptor_set_layout_bindings_[1] = {1, desc_type, 1, VK_SHADER_STAGE_ALL, nullptr};
    } else {  // default: BasicSpirv

        // tensors for GetSpirvBasicDataGraph(): 1 input, 1 output

        tensors_.resize(2);
        tensor_views_.resize(tensors_.size());
        descriptor_set_layout_bindings_.resize(tensors_.size());
        resources_.resize(tensors_.size());
        for (uint32_t i = 0; i < tensors_.size(); i++) {
            tensors_[i] = std::make_shared<vkt::Tensor>();
            tensor_views_[i] = std::make_shared<vkt::TensorView>();
            const std::vector<int64_t>& dims = (i == 0) ? in_tensor_dims : out_tensor_dims;
            InitTensor(*tensors_[i], *tensor_views_[i], dims, params_.protected_tensors);

            // last 3 numbers are: descriptor, binding, array index
            resources_[i] = {VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_RESOURCE_INFO_ARM, &tensors_[i]->Description(), 0, i, 0};
            descriptor_set_layout_bindings_[i] = {i, desc_type, 1, VK_SHADER_STAGE_ALL, nullptr};
        }
    }
    pipeline_ci_.resourceInfoCount = resources_.size();
    pipeline_ci_.pResourceInfos = resources_.data();

    descriptor_set_.reset(new OneOffDescriptorSet(device_, descriptor_set_layout_bindings_));
    CreatePipelineLayout();
}

void DataGraphPipelineHelper::CreatePipelineLayout(const std::vector<VkPushConstantRange>& push_constant_ranges) {
    pipeline_layout_ci_ = vku::InitStructHelper();
    pipeline_layout_ci_.flags = 0;
    pipeline_layout_ci_.pushConstantRangeCount = push_constant_ranges.size();
    pipeline_layout_ci_.pPushConstantRanges = push_constant_ranges.data();
    pipeline_layout_ = vkt::PipelineLayout(*device_, pipeline_layout_ci_, {&descriptor_set_->layout_});

    pipeline_ci_.layout = pipeline_layout_;
}

void DataGraphPipelineHelper::InitTensor(vkt::Tensor& tensor, vkt::TensorView& tensor_view, const std::vector<int64_t>& tensor_dims,
                                         bool is_protected) {
    VkTensorDescriptionARM tensor_desc = vku::InitStructHelper();
    tensor_desc.tiling = VK_TENSOR_TILING_LINEAR_ARM;
    tensor_desc.format = VK_FORMAT_R8_SINT;
    tensor_desc.dimensionCount = tensor_dims.size();
    tensor_desc.pDimensions = tensor_dims.data();
    tensor_desc.usage = VK_TENSOR_USAGE_DATA_GRAPH_BIT_ARM;

    VkTensorCreateInfoARM tensor_info = vku::InitStructHelper();
    tensor_info.pDescription = &tensor_desc;
    tensor_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkFlags memory_flags = 0;
    if (is_protected) {
        tensor_info.flags |= VK_TENSOR_CREATE_PROTECTED_BIT_ARM;
        memory_flags = VK_MEMORY_PROPERTY_PROTECTED_BIT;
    }

    tensor.InitNoMem(*device_, tensor_info);
    tensor.BindToMem(memory_flags);

    VkTensorViewCreateInfoARM tensor_view_ci = vku::InitStructHelper();
    tensor_view_ci.tensor = tensor.handle();
    tensor_view_ci.format = tensor.Format();
    tensor_view.Init(*device_, tensor_view_ci);
}

DataGraphPipelineHelper::DataGraphPipelineHelper(VkLayerTest& test, const HelperParameters& params)
    : layer_test_(test), params_(params) {
    device_ = layer_test_.DeviceObj();
    pipeline_ci_ = vku::InitStructHelper();

    std::string spirv_string(params_.spirv_source                                  ? params_.spirv_source
                             : params_.graph_variant == AddTensorArraySpirv        ? GetSpirvTensorArrayDataGraph(false)
                             : params_.graph_variant == AddRuntimeTensorArraySpirv ? GetSpirvTensorArrayDataGraph(true)
                                                                                   : GetSpirvBasicDataGraph());
    CreateShaderModule(spirv_string.c_str(), params_.entrypoint);
    InitPipelineResources(params_.desc_type);

    // Check that the initialisation of the pipeline has been successful
    layer_test_.Monitor().Finish();
}

VkResult DataGraphPipelineHelper::CreateDataGraphPipeline() {
    return vk::CreateDataGraphPipelinesARM(device_->handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipeline_ci_, nullptr,
                                           &pipeline_);
}

void DataGraphPipelineHelper::Destroy() {
    if (pipeline_ != VK_NULL_HANDLE) {
        vk::DestroyPipeline(device_->handle(), pipeline_, nullptr);
        pipeline_ = VK_NULL_HANDLE;
    }
}

DataGraphPipelineHelper::~DataGraphPipelineHelper() { Destroy(); }
}  // namespace dg
}  // namespace vkt
