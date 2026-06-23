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
#include <iostream>

namespace vkt {
namespace dg {

void DataGraphPipelineHelper::CreateShaderModule(const char* spirv_source, const char* entrypoint) {
    spvtools::SpirvTools tools{SPV_ENV_UNIVERSAL_1_6};

    std::string error_msg;
    tools.SetMessageConsumer([&](spv_message_level_t, const char*, const spv_position_t& position, const char* message) {
        std::ostringstream ss;
        ss << "on line " << position.line << ", column " << position.column << ": " << message;
        error_msg = ss.str();
    });

    // TODO - Replace with ASMtoSPV
    std::vector<uint32_t> spirv_binary;
    if (!tools.Assemble(spirv_source, &spirv_binary)) {
        GTEST_FAIL() << "Failed to compile SPIRV shader module. Error:\n"
                     << error_msg << std::endl
                     << "SpirV:\n"
                     << spirv_source << std::endl;
    }

    VkShaderModuleCreateInfo shader_module_create_info = vku::InitStructHelper();
    shader_module_create_info.codeSize = spirv_binary.size() * sizeof(uint32_t);
    shader_module_create_info.pCode = spirv_binary.data();

    shader_.Init(*device_, shader_module_create_info);
    shader_module_ci_ = vku::InitStructHelper();
    shader_module_ci_.module = shader_;
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
; constant used ONLY in entrypoint 2
                     %constant0 = OpGraphConstantARM %uchar_1_2_4_4_tensor 0
                    %main_arg_0 = OpVariable %uchar_1_8_16_4_tensor_ptr UniformConstant
                    %main_res_0 = OpVariable %uchar_1_2_4_4_tensor_ptr UniformConstant
                    %graph_type = OpTypeGraphARM 1 %uchar_1_8_16_4_tensor %uchar_1_2_4_4_tensor
; graph 1: %main_res_0 = MAX_POOL2D(MAX_POOL2D(%main_arg_0))
                       %graph_1 = OpGraphARM %graph_type
                          %in_0 = OpGraphInputARM %uchar_1_8_16_4_tensor %uint_0
                          %op_0 = OpExtInst %uchar_1_4_8_4_tensor %tosa MAX_POOL2D %uint_2_tensor_2_2 %uint_2_tensor_2_2 %uint_4_tensor_0_0_0_0 %uint_0 %in_0
                          %op_1 = OpExtInst %uchar_1_2_4_4_tensor %tosa MAX_POOL2D %uint_2_tensor_2_2 %uint_2_tensor_2_2 %uint_4_tensor_0_0_0_0 %uint_0 %op_0
                                  OpGraphSetOutputARM %op_1 %uint_0
                                  OpGraphEndARM
; graph 2: %main_res_0 = ADD(AVG_POOL2D(AVG_POOL2D(%main_arg_0)), %constant0)
                       %graph_2 = OpGraphARM %graph_type
                          %in_1 = OpGraphInputARM %uchar_1_8_16_4_tensor %uint_0
                          %op_2 = OpExtInst %uchar_1_4_8_4_tensor %tosa AVG_POOL2D %uint_2_tensor_2_2 %uint_2_tensor_2_2 %uint_4_tensor_0_0_0_0 %uint_1 %in_1 %uchar_1_tensor_0 %uchar_1_tensor_0
                          %op_3 = OpExtInst %uchar_1_2_4_4_tensor %tosa AVG_POOL2D %uint_2_tensor_2_2 %uint_2_tensor_2_2 %uint_4_tensor_0_0_0_0 %uint_1 %op_2 %uchar_1_tensor_0 %uchar_1_tensor_0
                          %op_4 = OpExtInst %uchar_1_2_4_4_tensor %tosa ADD %op_3 %constant0
                                  OpGraphSetOutputARM %op_4 %uint_0
                                  OpGraphEndARM
; bind graphs to entrypoints
                                  OpGraphEntryPointARM %graph_1 "entrypoint_1" %main_arg_0 %main_res_0
                                  OpGraphEntryPointARM %graph_2 "entrypoint_2" %main_arg_0 %main_res_0
)";
}

// Spirv source. For testing purposes it includes:
// - unused OpGraphConstantARM
// - `inserted_line` to cause different errors
std::string DataGraphPipelineHelper::GetSpirvModifyableDataGraph(const ModifiableShaderParameters& params) {
    std::ostringstream ss;
    ss << R"(
                                  OpCapability GraphARM
                                  OpCapability TensorsARM
)" << params.capabilities
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
)" << params.types
       << R"(
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
)" << params.instructions
       << R"(
                                  OpGraphSetOutputARM %op_1 %uint_0
                                  OpGraphEndARM
)";

    return ss.str();
}

// A command shader (for the majority of cases) that can be modified inserting
// instructions in given sections. Without any insertions it's a basic shader.
std::string DataGraphPipelineHelper::GetSpirvModifiableShader(const ModifiableShaderParameters& params) {
    std::ostringstream ss;
    ss << R"(
; SPIRV
; Version: 1.6
; Generator: Khronos Glslang Reference Front End; 11
; Bound: 19
; Schema: 0
               OpCapability Shader
)" << params.capabilities
       << R"(
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
)" << params.types
       << R"(
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
)" << params.instructions
       << R"(
               OpReturn
               OpFunctionEnd
)";
    return ss.str();
}

// spirv using a descriptor array
std::string DataGraphPipelineHelper::GetSpirvTensorArrayDataGraph(bool is_runtime) {
    std::ostringstream ss;
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
                  %tensor = OpTypeTensorARM %i32 %i32_4 %tensor_shape
)" << (is_runtime ? "%tensor_array = OpTypeRuntimeArray %tensor" : "%tensor_array = OpTypeArray %tensor %i32_2")
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

std::string DataGraphPipelineHelper::GetSpirvConstantDataGraph() {
    vkt::dg::ModifiableShaderParameters spirv_params;
    spirv_params.types = "%constant_0 = OpGraphConstantARM %uchar_1_2_4_4_tensor 0";
    spirv_params.instructions = "%dummy = OpExtInst %uchar_1_2_4_4_tensor %tosa ADD %op_1 %constant_0";
    return vkt::dg::DataGraphPipelineHelper::GetSpirvModifyableDataGraph(spirv_params);
}

// shapes for 2-layer maxpool 2x2: output tensor is 1/4 the size of the input tensor
const std::vector<int64_t> in_tensor_dims = {1, 8, 16, 4};
const std::vector<int64_t> out_tensor_dims = {in_tensor_dims[0], in_tensor_dims[1] / 4, in_tensor_dims[2] / 4, in_tensor_dims[3]};

// shape for ADD spirv
const std::vector<int64_t> add_tensor_dims{1, 4, 4, 2};

// Tensor description for the various spirvs
VkTensorDescriptionARM DataGraphPipelineHelper::GetTensorDesc(TensorType type) {
    VkFormat format = VK_FORMAT_UNDEFINED;
    const std::vector<int64_t>* dims = nullptr;
    switch (type) {
        case BASIC_SPIRV_IN:
            format = VK_FORMAT_R8_SINT;
            dims = &in_tensor_dims;
            break;
        case BASIC_SPIRV_OUT:
            format = VK_FORMAT_R8_SINT;
            dims = &out_tensor_dims;
            break;
        case ARRAY_SPIRV:
            format = VK_FORMAT_R32_SINT;
            dims = &add_tensor_dims;
            break;
    }

    VkTensorDescriptionARM desc = vku::InitStructHelper();
    desc.tiling = VK_TENSOR_TILING_LINEAR_ARM;
    desc.format = format;
    desc.dimensionCount = dims->size();
    desc.pDimensions = dims->data();
    desc.usage = params_.usage_bit;
    return desc;
}

void DataGraphPipelineHelper::InitPipelineResources() {
    if (params_.graph_variant == AddTensorArraySpirv || params_.graph_variant == AddRuntimeTensorArraySpirv) {
        VkTensorDescriptionARM desc = GetTensorDesc(ARRAY_SPIRV);
        tensors_.resize(3);
        tensor_views_.resize(tensors_.size());
        for (uint32_t i = 0; i < 3; i++) {
            tensors_[i] = std::make_shared<vkt::Tensor>();
            tensor_views_[i] = std::make_shared<vkt::TensorView>();
            InitTensor(*tensors_[i], *tensor_views_[i], desc, params_.protected_tensors);
        }

        resources_.resize(3);
        // last 3 numbers are: descriptor, binding, array index
        // 2 input tensors, as array
        resources_[0] = {VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_RESOURCE_INFO_ARM, &tensors_[0]->Description(), 0, 0, 0};
        resources_[1] = {VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_RESOURCE_INFO_ARM, &tensors_[1]->Description(), 0, 0, 1};
        // 1 output tensor
        resources_[2] = {VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_RESOURCE_INFO_ARM, &tensors_[2]->Description(), 0, 1, 0};

        // binding 0: 2 x inputs; binding 1: 1 x output
        descriptor_set_layout_bindings_.resize(2);
        descriptor_set_layout_bindings_[0] = {0, params_.desc_type, 2, VK_SHADER_STAGE_ALL, nullptr};
        descriptor_set_layout_bindings_[1] = {1, params_.desc_type, 1, VK_SHADER_STAGE_ALL, nullptr};
    } else {  // default: BasicSpirv

        // tensors for GetSpirvModifyableDataGraph(): 1 input, 1 output

        tensors_.resize(2);
        tensor_views_.resize(tensors_.size());
        descriptor_set_layout_bindings_.resize(tensors_.size());
        resources_.resize(tensors_.size());
        for (uint32_t i = 0; i < tensors_.size(); i++) {
            VkTensorDescriptionARM desc = GetTensorDesc(i == 0 ? BASIC_SPIRV_IN : BASIC_SPIRV_OUT);
            tensors_[i] = std::make_shared<vkt::Tensor>();
            tensor_views_[i] = std::make_shared<vkt::TensorView>();
            InitTensor(*tensors_[i], *tensor_views_[i], desc, params_.protected_tensors);

            // last 3 numbers are: descriptor, binding, array index
            resources_[i] = {VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_RESOURCE_INFO_ARM, &tensors_[i]->Description(), 0, i, 0};
            descriptor_set_layout_bindings_[i] = {i, params_.desc_type, 1, VK_SHADER_STAGE_ALL, nullptr};
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

void DataGraphPipelineHelper::InitTensor(vkt::Tensor& tensor, vkt::TensorView& tensor_view,
                                         const VkTensorDescriptionARM& tensor_desc, bool is_protected) {
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
    tensor_view_ci.tensor = tensor;
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
    InitPipelineResources();

    // Check that the initialisation of the pipeline has been successful
    layer_test_.Monitor().Finish();
}

VkResult DataGraphPipelineHelper::CreateDataGraphPipeline(VkPipelineCache pipeline_cache) {
    return vk::CreateDataGraphPipelinesARM(device_->handle(), VK_NULL_HANDLE, pipeline_cache, 1, &pipeline_ci_, nullptr,
                                           &pipeline_);
}

void DataGraphPipelineHelper::Destroy() {
    if (pipeline_ != VK_NULL_HANDLE) {
        vk::DestroyPipeline(device_->handle(), pipeline_, nullptr);
        pipeline_ = VK_NULL_HANDLE;
    }
}

DataGraphPipelineHelper::~DataGraphPipelineHelper() { Destroy(); }

std::optional<VkQueueFamilyDataGraphPropertiesARM> OpticalFlowHelper::GetOpticalFlowSupportProperty(VkLayerTest& test,
                                                                                                      uint32_t queue_index) {
    VkPhysicalDevice gpu = test.Gpu();
    uint32_t n_properties = 0;
    [[maybe_unused]] VkResult result =
        vk::GetPhysicalDeviceQueueFamilyDataGraphPropertiesARM(gpu, queue_index, &n_properties, nullptr);
    assert(VK_SUCCESS == result);
    std::vector<VkQueueFamilyDataGraphPropertiesARM> properties(n_properties,
                                                                {VK_STRUCTURE_TYPE_QUEUE_FAMILY_DATA_GRAPH_PROPERTIES_ARM});
    result = vk::GetPhysicalDeviceQueueFamilyDataGraphPropertiesARM(gpu, queue_index, &n_properties, properties.data());
    assert(VK_SUCCESS == result);

    for (uint32_t i = 0; i < n_properties; i++) {
        if (properties[i].operation.operationType == VK_PHYSICAL_DEVICE_DATA_GRAPH_OPERATION_TYPE_OPTICAL_FLOW_ARM) {
            return properties[i];
        }
    }

    return std::nullopt;
}

VkQueueFamilyDataGraphOpticalFlowPropertiesARM OpticalFlowHelper::QueryOpticalFlowProperties(VkLayerTest& test,
                                                                                             uint32_t queue_index) {
    auto of_support_property = GetOpticalFlowSupportProperty(test, queue_index);
    assert(std::nullopt != of_support_property);
    VkQueueFamilyDataGraphOpticalFlowPropertiesARM properties_query = vku::InitStructHelper();
    [[maybe_unused]] VkResult result = vk::GetPhysicalDeviceQueueFamilyDataGraphEngineOperationPropertiesARM(
        test.Gpu(), queue_index, &of_support_property.value(), reinterpret_cast<VkBaseOutStructure*>(&properties_query));
    assert(VK_SUCCESS == result);

    return properties_query;
}

VkDataGraphOpticalFlowGridSizeFlagBitsARM OpticalFlowHelper::GetAnyOpticalFlowGridSize(
    VkQueueFamilyDataGraphOpticalFlowPropertiesARM properties, VkDataGraphOpticalFlowGridSizeFlagBitsARM first_size) {
    // we need first_size to be a single bit, but enums don't enforce that
    assert(first_size == VK_DATA_GRAPH_OPTICAL_FLOW_GRID_SIZE_1X1_BIT_ARM ||
           first_size == VK_DATA_GRAPH_OPTICAL_FLOW_GRID_SIZE_2X2_BIT_ARM ||
           first_size == VK_DATA_GRAPH_OPTICAL_FLOW_GRID_SIZE_4X4_BIT_ARM ||
           first_size == VK_DATA_GRAPH_OPTICAL_FLOW_GRID_SIZE_8X8_BIT_ARM);

    VkDataGraphOpticalFlowGridSizeFlagsARM supported_grid_sizes =
        properties.supportedOutputGridSizes & properties.supportedHintGridSizes;

    const auto first_size_mask = static_cast<VkDataGraphOpticalFlowGridSizeFlagsARM>(first_size);
    const VkDataGraphOpticalFlowGridSizeFlagsARM eligible_grid_sizes =
        (first_size_mask == 0) ? supported_grid_sizes : (supported_grid_sizes & ~(first_size_mask - 1));

    return (eligible_grid_sizes == 0)
               ? VK_DATA_GRAPH_OPTICAL_FLOW_GRID_SIZE_UNKNOWN_ARM
               : static_cast<VkDataGraphOpticalFlowGridSizeFlagBitsARM>(eligible_grid_sizes & (0u - eligible_grid_sizes));
}

VkFormat OpticalFlowHelper::GetAnyOpticalFlowFormat(VkDataGraphOpticalFlowImageUsageFlagsARM usage) {
    auto of_support_property = GetOpticalFlowSupportProperty(dg_pipeline_.layer_test_, queue_index_);
    assert(std::nullopt != of_support_property);
    VkDataGraphOpticalFlowImageFormatInfoARM format_info = vku::InitStructHelper();
    format_info.usage = usage;
    VkDataGraphOpticalFlowImageFormatPropertiesARM format_properties = vku::InitStructHelper();
    uint32_t format_count = 1;

    VkResult result = vk::GetPhysicalDeviceQueueFamilyDataGraphOpticalFlowImageFormatsARM(
        dg_pipeline_.layer_test_.Gpu(), queue_index_, &of_support_property.value(),
        &format_info, &format_count, &format_properties);

    return result == VK_SUCCESS || result == VK_INCOMPLETE ? format_properties.format : VK_FORMAT_UNDEFINED;
}

void OpticalFlowHelper::CreateOpticalFlow() {
    single_node_ci_ = vku::InitStructHelper();
    single_node_ci_.nodeType = VK_DATA_GRAPH_PIPELINE_NODE_TYPE_OPTICAL_FLOW_ARM;

    params_.n_hints = optical_flow_properties_.hintSupported ? params_.n_hints : 0;
    params_.n_costs = optical_flow_properties_.costSupported ? params_.n_costs : 0;

    uint32_t kResourceCount = 0;
    kResourceCount += params_.n_inputs;
    kResourceCount += params_.n_references;
    kResourceCount += params_.n_outputs;
    kResourceCount += params_.n_hints;
    kResourceCount += params_.n_costs;
    connections_.resize(kResourceCount, vku::InitStruct<VkDataGraphPipelineSingleNodeConnectionARM>());

    uint32_t nc = 0;
    for (uint32_t i = 0; i < params_.n_inputs; i++, nc++) {
        connections_[nc].set = 0;
        connections_[nc].binding = nc;
        connections_[nc].connection = VK_DATA_GRAPH_PIPELINE_NODE_CONNECTION_TYPE_OPTICAL_FLOW_INPUT_ARM;
    }
    for (uint32_t i = 0; i < params_.n_references; i++, nc++) {
        connections_[nc].set = 0;
        connections_[nc].binding = nc;
        connections_[nc].connection = VK_DATA_GRAPH_PIPELINE_NODE_CONNECTION_TYPE_OPTICAL_FLOW_REFERENCE_ARM;
    }
    for (uint32_t i = 0; i < params_.n_outputs; i++, nc++) {
        connections_[nc].set = 0;
        connections_[nc].binding = nc;
        connections_[nc].connection = VK_DATA_GRAPH_PIPELINE_NODE_CONNECTION_TYPE_OPTICAL_FLOW_FLOW_VECTOR_ARM;
    }
    for (uint32_t i = 0; i < params_.n_hints; i++, nc++) {
        connections_[nc].set = 0;
        connections_[nc].binding = nc;
        connections_[nc].connection = VK_DATA_GRAPH_PIPELINE_NODE_CONNECTION_TYPE_OPTICAL_FLOW_HINT_ARM;
    }
    for (uint32_t i = 0; i < params_.n_costs; i++, nc++) {
        connections_[nc].set = 0;
        connections_[nc].binding = nc;
        connections_[nc].connection = VK_DATA_GRAPH_PIPELINE_NODE_CONNECTION_TYPE_OPTICAL_FLOW_COST_ARM;
    }

    single_node_ci_.connectionCount = static_cast<uint32_t>(connections_.size());
    single_node_ci_.pConnections = connections_.data();

    optical_flow_ci_ = vku::InitStructHelper(&single_node_ci_);
    optical_flow_ci_.flags = 0;
    optical_flow_ci_.height = params_.height;
    optical_flow_ci_.width = params_.width;
    optical_flow_ci_.imageFormat = GetAnyOpticalFlowFormat(VK_DATA_GRAPH_OPTICAL_FLOW_IMAGE_USAGE_INPUT_BIT_ARM);
    optical_flow_ci_.flowVectorFormat = GetAnyOpticalFlowFormat(VK_DATA_GRAPH_OPTICAL_FLOW_IMAGE_USAGE_OUTPUT_BIT_ARM);
    optical_flow_ci_.outputGridSize = params_.outputGridSize ? params_.outputGridSize
        : static_cast<VkDataGraphOpticalFlowGridSizeFlagsARM>(GetAnyOpticalFlowGridSize(optical_flow_properties_));
    optical_flow_ci_.hintGridSize = params_.hintGridSize ? params_.hintGridSize : optical_flow_ci_.outputGridSize;
    if (optical_flow_properties_.hintSupported) {
        optical_flow_ci_.flags |= VK_DATA_GRAPH_OPTICAL_FLOW_CREATE_ENABLE_HINT_BIT_ARM;
    }
    if (optical_flow_properties_.costSupported) {
        optical_flow_ci_.costFormat = GetAnyOpticalFlowFormat(VK_DATA_GRAPH_OPTICAL_FLOW_IMAGE_USAGE_COST_BIT_ARM);
        optical_flow_ci_.flags |= VK_DATA_GRAPH_OPTICAL_FLOW_CREATE_ENABLE_COST_BIT_ARM;
    }
    optical_flow_ci_.performanceLevel = VK_DATA_GRAPH_OPTICAL_FLOW_PERFORMANCE_LEVEL_MEDIUM_ARM;
}

static uint32_t scaled_size(uint32_t in_size, VkDataGraphOpticalFlowGridSizeFlagsARM grid_size) {
    switch (grid_size) {
        case VK_DATA_GRAPH_OPTICAL_FLOW_GRID_SIZE_1X1_BIT_ARM:
            return in_size;
        case VK_DATA_GRAPH_OPTICAL_FLOW_GRID_SIZE_2X2_BIT_ARM:
            return (in_size + 1) / 2;
        case VK_DATA_GRAPH_OPTICAL_FLOW_GRID_SIZE_4X4_BIT_ARM:
            return (in_size + 3) / 4;
        case VK_DATA_GRAPH_OPTICAL_FLOW_GRID_SIZE_8X8_BIT_ARM:
            return (in_size + 7) / 8;
    }
    assert(false);
    return UINT32_MAX;
}

void OpticalFlowHelper::SetupImageDescriptors() {
    images_.resize(connections_.size());
    image_views_.resize(images_.size());

    // image size: hint
    auto hint_w = scaled_size(optical_flow_ci_.width, optical_flow_ci_.hintGridSize);
    auto hint_h = scaled_size(optical_flow_ci_.height, optical_flow_ci_.hintGridSize);
    // output and cost
    auto out_w = scaled_size(optical_flow_ci_.width, optical_flow_ci_.outputGridSize);
    auto out_h = scaled_size(optical_flow_ci_.height, optical_flow_ci_.outputGridSize);

    uint32_t nc = 0;
    for (uint32_t i = 0; i < params_.n_inputs; i++, nc++) {
        images_[nc].Init(*dg_pipeline_.device_, optical_flow_ci_.width, optical_flow_ci_.height, 1, optical_flow_ci_.imageFormat, VK_IMAGE_USAGE_SAMPLED_BIT);
        images_[nc].SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        image_views_[nc] = images_[nc].CreateView();
        dg_pipeline_.descriptor_set_->WriteDescriptorImageInfo(nc, image_views_[nc], VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    }

    for (uint32_t i = 0; i < params_.n_references; i++, nc++) {
        images_[nc].Init(*dg_pipeline_.device_, optical_flow_ci_.width, optical_flow_ci_.height, 1, optical_flow_ci_.imageFormat,
                                VK_IMAGE_USAGE_SAMPLED_BIT);
        images_[nc].SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        image_views_[nc] = images_[nc].CreateView();
        dg_pipeline_.descriptor_set_->WriteDescriptorImageInfo(nc, image_views_[nc], VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    }

    for (uint32_t i = 0; i < params_.n_outputs; i++, nc++) {
        images_[nc].Init(*dg_pipeline_.device_, out_w, out_h, 1, optical_flow_ci_.flowVectorFormat, VK_IMAGE_USAGE_STORAGE_BIT);
        images_[nc].SetLayout(VK_IMAGE_LAYOUT_GENERAL);
        image_views_[nc] = images_[nc].CreateView();
        dg_pipeline_.descriptor_set_->WriteDescriptorImageInfo(nc, image_views_[nc], VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
    }

    for (uint32_t i = 0; i < params_.n_hints; i++, nc++) {
        images_[nc].Init(*dg_pipeline_.device_, hint_w, hint_h, 1, optical_flow_ci_.flowVectorFormat, VK_IMAGE_USAGE_SAMPLED_BIT);
        images_[nc].SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        image_views_[nc] = images_[nc].CreateView();
        dg_pipeline_.descriptor_set_->WriteDescriptorImageInfo(nc, image_views_[nc], VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    }

    for (uint32_t i = 0; i < params_.n_costs; i++, nc++) {
        images_[nc].Init(*dg_pipeline_.device_, out_w, out_h, 1, optical_flow_ci_.costFormat, VK_IMAGE_USAGE_STORAGE_BIT);
        images_[nc].SetLayout(VK_IMAGE_LAYOUT_GENERAL);
        image_views_[nc] = images_[nc].CreateView();
        dg_pipeline_.descriptor_set_->WriteDescriptorImageInfo(nc, image_views_[nc], VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                               VK_IMAGE_LAYOUT_GENERAL);
    }

    dg_pipeline_.descriptor_set_->UpdateDescriptorSets();
}

void OpticalFlowHelper::InitDataGraphPipeline() {

    image_layouts_.resize(connections_.size(), vku::InitStruct<VkDataGraphPipelineResourceInfoImageLayoutARM>());
    dg_pipeline_.descriptor_set_layout_bindings_.clear();
    uint32_t nc = 0;
    for (uint32_t i = 0; i < params_.n_inputs; i++, nc++) {
        dg_pipeline_.descriptor_set_layout_bindings_.push_back(
            {nc, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr});
        image_layouts_[nc].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    for (uint32_t i = 0; i < params_.n_references; i++, nc++) {
        dg_pipeline_.descriptor_set_layout_bindings_.push_back(
            {nc, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr});
        image_layouts_[nc].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    for (uint32_t i = 0; i < params_.n_outputs; i++, nc++) {
        dg_pipeline_.descriptor_set_layout_bindings_.push_back(
            {nc, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr});
        image_layouts_[nc].layout = VK_IMAGE_LAYOUT_GENERAL;
    }
    for (uint32_t i = 0; i < params_.n_hints; i++, nc++) {
        dg_pipeline_.descriptor_set_layout_bindings_.push_back(
            {nc, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr});
        image_layouts_[nc].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    };
    for (uint32_t i = 0; i < params_.n_costs; i++, nc++) {
        dg_pipeline_.descriptor_set_layout_bindings_.push_back(
            {nc, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr});
        image_layouts_[nc].layout = VK_IMAGE_LAYOUT_GENERAL;
    };

    dg_pipeline_.descriptor_set_.reset(new OneOffDescriptorSet(dg_pipeline_.device_, dg_pipeline_.descriptor_set_layout_bindings_));
    dg_pipeline_.CreatePipelineLayout();

    dg_pipeline_.resources_.resize(image_layouts_.size());
    for (uint32_t i = 0; i < dg_pipeline_.resources_.size(); i++) {
        dg_pipeline_.resources_[i] = vku::InitStructHelper(&image_layouts_[i]);
        dg_pipeline_.resources_[i].descriptorSet = 0;
        dg_pipeline_.resources_[i].binding = i;
    }

    dg_pipeline_.pipeline_ci_.resourceInfoCount = static_cast<uint32_t>(dg_pipeline_.resources_.size());
    dg_pipeline_.pipeline_ci_.pResourceInfos = dg_pipeline_.resources_.data();
    dg_pipeline_.pipeline_ci_.pNext = &optical_flow_ci_;
}

VkResult OpticalFlowHelper::CreateDataGraphPipeline() { return dg_pipeline_.CreateDataGraphPipeline(); }

OpticalFlowHelper::OpticalFlowHelper(VkLayerTest& test, const OfHelperParameters& params)
    : params_(params), dg_pipeline_(test) {
    queue_index_ = dg_pipeline_.layer_test_.DefaultQueue()->family_index;  // add to params_ if we ever need to change this
    optical_flow_properties_ = QueryOpticalFlowProperties(test, queue_index_);
    CreateOpticalFlow();
    InitDataGraphPipeline();
}
}  // namespace dg
}  // namespace vkt
