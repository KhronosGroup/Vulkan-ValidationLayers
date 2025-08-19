/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

class PositiveShaderCompute : public VkLayerTest {};

TEST_F(PositiveShaderCompute, WorkGroupSizePrecedenceOverLocalSize) {
    // "If an object is decorated with the WorkgroupSize decoration, this takes precedence over any LocalSize or LocalSizeId
    // execution mode."
    TEST_DESCRIPTION("Make sure work WorkgroupSize decoration is used over LocalSize");

    RETURN_IF_SKIP(Init());

    uint32_t x_size_limit = m_device->Physical().limits_.maxComputeWorkGroupSize[0];
    uint32_t y_size_limit = m_device->Physical().limits_.maxComputeWorkGroupSize[1];
    uint32_t z_size_limit = m_device->Physical().limits_.maxComputeWorkGroupSize[2];

    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize )";
    spv_source << std::to_string(x_size_limit + 1) << " " << std::to_string(y_size_limit + 1) << " "
               << std::to_string(z_size_limit + 1);
    spv_source << R"(
               OpSource GLSL 450
               OpName %main "main"
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = VkShaderObj(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0,
                                                   SPV_SOURCE_ASM);
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderCompute, WorkGroupSizeSpecConstantUnder) {
    TEST_DESCRIPTION("Make sure spec constants get applied to to be under maxComputeWorkGroupSize");

    RETURN_IF_SKIP(Init());

    uint32_t x_size_limit = m_device->Physical().limits_.maxComputeWorkGroupSize[0];

    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpDecorate %7 SpecId 0
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
          %7 = OpSpecConstant %uint )";
    spv_source << std::to_string(x_size_limit + 1);
    spv_source << R"(
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
%gl_WorkGroupSize = OpSpecConstantComposite %v3uint %7 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    uint32_t data = 1;

    VkSpecializationMapEntry entry;
    entry.constantID = 0;
    entry.offset = 0;
    entry.size = sizeof(uint32_t);

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = VkShaderObj(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_0,
                                                   SPV_SOURCE_ASM, &specialization_info);
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderCompute, WorkGroupSizeLocalSizeId) {
    TEST_DESCRIPTION("Validate LocalSizeId doesn't triggers maxComputeWorkGroupSize limit");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::maintenance4);
    RETURN_IF_SKIP(Init());

    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionModeId %main LocalSizeId %uint_2 %uint_1 %uint_1
               OpSource GLSL 450
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_1 = OpConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = VkShaderObj(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3,
                                                   SPV_SOURCE_ASM);
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderCompute, WorkGroupSizeLocalSizeIdSpecConstant) {
    TEST_DESCRIPTION("Validate LocalSizeId doesn't triggers maxComputeWorkGroupSize limit with spec constants");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::maintenance4);
    RETURN_IF_SKIP(Init());

    uint32_t x_size_limit = m_device->Physical().limits_.maxComputeWorkGroupSize[0];

    // layout(local_size_x_id = 18, local_size_z_id = 19) in;
    // layout(local_size_x = 32) in;
    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionModeId %main LocalSizeId %spec_x %uint_1 %spec_z
               OpSource GLSL 450
               OpDecorate %spec_x SpecId 18
               OpDecorate %spec_z SpecId 19
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %spec_x = OpSpecConstant %uint 32
     %uint_1 = OpConstant %uint 1
     %spec_z = OpSpecConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    uint32_t data = x_size_limit - 1;

    VkSpecializationMapEntry entry;
    entry.constantID = 18;
    entry.offset = 0;
    entry.size = sizeof(uint32_t);

    VkSpecializationInfo specialization_info = {};
    specialization_info.mapEntryCount = 1;
    specialization_info.pMapEntries = &entry;
    specialization_info.dataSize = sizeof(uint32_t);
    specialization_info.pData = &data;

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = VkShaderObj(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3,
                                                   SPV_SOURCE_ASM, &specialization_info);
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderCompute, WorkGroupSizePrecedenceOverLocalSizeId) {
    // "If an object is decorated with the WorkgroupSize decoration, this takes precedence over any LocalSize or LocalSizeId
    // execution mode."
    TEST_DESCRIPTION("Make sure work WorkgroupSize decoration is used over LocalSizeId");

    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::maintenance4);
    RETURN_IF_SKIP(Init());

    uint32_t x_size_limit = m_device->Physical().limits_.maxComputeWorkGroupSize[0];

    std::stringstream spv_source;
    spv_source << R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionModeId %main LocalSizeId %spec_x %uint_1 %uint_1
               OpSource GLSL 450
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
               OpDecorate %spec_x SpecId 18
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %spec_x = OpSpecConstant %uint )";
    spv_source << std::to_string(x_size_limit + 1);
    spv_source << R"(
     %uint_1 = OpConstant %uint 1
     %v3uint = OpTypeVector %uint 3
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = VkShaderObj(this, spv_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3,
                                                   SPV_SOURCE_ASM);
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderCompute, SharedMemorySpecConstantOp) {
    TEST_DESCRIPTION("Validate compute shader shared memory");

    RETURN_IF_SKIP(Init());

    const uint32_t max_shared_memory_size = m_device->Physical().limits_.maxComputeSharedMemorySize;
    const uint32_t max_shared_ints = max_shared_memory_size / 4;

    if (max_shared_ints < 16 * 7) {
        GTEST_SKIP() << "Supported compute shader shared memory size is too small";
    }

    char const *cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

        layout(constant_id = 0) const uint Condition = 0;
        layout(constant_id = 1) const uint SharedSize = 16;

        #define enableSharedMemoryOpt (Condition == 1 || Condition == 2 || Condition == 3)
        shared uint arr[enableSharedMemoryOpt ? SharedSize : 1][enableSharedMemoryOpt ? 7 : 1];

        void main() {}
    )glsl";

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = VkShaderObj(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderCompute, SharedMemory) {
    TEST_DESCRIPTION("Validate compute shader shared memory does not exceed maxComputeSharedMemorySize");

    RETURN_IF_SKIP(Init());

    // Make sure compute pipeline has a compute shader stage set
    char const *csSource = R"glsl(
        #version 450
        shared uint a;
        shared float b;
        shared vec2 c;
        shared mat3 d;
        shared mat4 e[3];
        struct A {
            int f;
            float g;
            uint h;
        };
        shared A f;
        void main(){
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.CreateComputePipeline();
}

TEST_F(PositiveShaderCompute, ZeroInitializeWorkgroupMemoryFeature) {
    TEST_DESCRIPTION("Enable and use shaderZeroInitializeWorkgroupMemory feature");

    AddRequiredExtensions(VK_KHR_ZERO_INITIALIZE_WORKGROUP_MEMORY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderZeroInitializeWorkgroupMemory);
    RETURN_IF_SKIP(Init());

    const char *spv_source = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource GLSL 450
               OpName %main "main"
               OpName %counter "counter"
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Workgroup_uint = OpTypePointer Workgroup %uint
  %zero_uint = OpConstantNull %uint
    %counter = OpVariable %_ptr_Workgroup_uint Workgroup %zero_uint
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    auto cs = VkShaderObj::CreateFromASM(this, spv_source, VK_SHADER_STAGE_COMPUTE_BIT);
    const auto set_info = [&cs](CreateComputePipelineHelper &helper) { helper.cs_ = std::move(cs); };
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);
}

TEST_F(PositiveShaderCompute, MemoryModelOperand2) {
    TEST_DESCRIPTION("Offset a buffer device address in a OpAccessChain");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModelDeviceScope);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    RETURN_IF_SKIP(Init());

    // SPIR-V based of:
    /*
    #version 460
            #pragma use_vulkan_memory_model
            #extension GL_KHR_memory_scope_semantics : enable
            #extension GL_EXT_buffer_reference : enable
            #extension GL_ARB_gpu_shader_int64 : enable

            shared bool sharedSkip;
            layout(buffer_reference) buffer Node { uint x[]; };
            layout(set=0, binding=0) buffer SSBO {
                Node node;
                uint a;
                uint b;
            };

            void foo(uint64_t x) {
                b = uint(x);
            }

            void main() {
                foo(node.x[128]);
            }
    */
    // But manually modified to instead pass something similar in spirit to:
    // foo( uint64_t(node) + 128 * sizeof(uint32_t) );
    const char *spv_shader_source = R"(
               OpCapability Shader
               OpCapability Int64
               OpCapability VulkanMemoryModel
               OpCapability PhysicalStorageBufferAddresses
          %2 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 Vulkan
               OpEntryPoint GLCompute %main "main" %_ %sharedSkip
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpMemberDecorate %SSBO 1 Offset 8
               OpMemberDecorate %SSBO 2 Offset 12
               OpDecorate %_runtimearr_uint ArrayStride 4
               OpDecorate %Node Block
               OpMemberDecorate %Node 0 Offset 0
               OpDecorate %_ Binding 0
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
      %ulong = OpTypeInt 64 0
          %9 = OpTypeFunction %void %ulong
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_Node PhysicalStorageBuffer
       %uint = OpTypeInt 32 0
       %SSBO = OpTypeStruct %_ptr_PhysicalStorageBuffer_Node %uint %uint    
%_runtimearr_uint = OpTypeRuntimeArray %uint                                
       %Node = OpTypeStruct %_runtimearr_uint                               
%_ptr_PhysicalStorageBuffer_Node = OpTypePointer PhysicalStorageBuffer %Node
%_ptr_StorageBuffer_SSBO = OpTypePointer StorageBuffer %SSBO
          %_ = OpVariable %_ptr_StorageBuffer_SSBO StorageBuffer    
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_Node = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_Node
    %int_128 = OpConstant %int 128
%_ptr_PhysicalStorageBuffer_uint = OpTypePointer PhysicalStorageBuffer %uint
       %bool = OpTypeBool
%_ptr_Workgroup_bool = OpTypePointer Workgroup %bool
 %sharedSkip = OpVariable %_ptr_Workgroup_bool Workgroup
       %main = OpFunction %void None %4
          %6 = OpLabel
         %28 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_Node %_ %int_0
         %29 = OpLoad %_ptr_PhysicalStorageBuffer_Node %28
         
         ; ===== PROBLEM HERE =====
         ; This OpAccessChain does not seem to be correctly executed by the driver,
         ; it does not apply the 128 * sizeof(uint32_t) offset and instead just 
         ; ends up accessing the base address
         %32 = OpAccessChain %_ptr_PhysicalStorageBuffer_uint %29 %int_0 %int_128
         ; ========================

         %param = OpConvertPtrToU %ulong %32
         %36 = OpFunctionCall %void %foo_u641_ %param
               OpReturn
               OpFunctionEnd
  %foo_u641_ = OpFunction %void None %9
          %x = OpFunctionParameter %ulong
         %12 = OpLabel
         %23 = OpUConvert %uint %x
         %25 = OpAccessChain %_ptr_StorageBuffer_uint %_ %int_2
               OpStore %25 %23
               OpReturn
               OpFunctionEnd
        )";

    vkt::Buffer bda_buffer(*m_device, 256 * sizeof(uint32_t), 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    auto in_buffer_ptr = static_cast<VkDeviceAddress *>(in_buffer.Memory().Map());
    in_buffer_ptr[0] = bda_buffer.Address();
    in_buffer_ptr[1] = 0;  // set SSBO.a and SSBO.b to be zero

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(this, spv_shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.CreateComputePipeline();

    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    auto in_buffer_u32_ptr = (uint32_t *)in_buffer_ptr;
    ASSERT_EQ(in_buffer_u32_ptr[3], (uint32_t)bda_buffer.Address() + 128 * sizeof(uint32_t));
}
