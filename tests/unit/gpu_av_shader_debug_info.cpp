/*
 * Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"

class NegativeGpuAVShaderDebugInfo : public GpuAVBufferDeviceAddressTest {
  public:
    void BasicSingleStorageBufferComputeOOB(const char *shader, const char *error);
};

// shader must have a SSBO at (set = 0, binding = 0)
void NegativeGpuAVShaderDebugInfo::BasicSingleStorageBufferComputeOOB(const char *shader, const char *error) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipe.CreateComputePipeline();

    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    auto data = static_cast<VkDeviceAddress *>(in_buffer.memory().map());
    data[0] = block_buffer.address();
    in_buffer.memory().unmap();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    // UNASSIGNED-Device address out of bounds
    m_errorMonitor->SetDesiredError(error);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, OpLine) {
    TEST_DESCRIPTION("Make sure basic OpLine works");

    char const *shader_source = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
          %2 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
          %1 = OpString ""
               OpMemberDecorate %foo 0 Offset 0
               OpMemberDecorate %foo 1 Offset 8
               OpDecorate %foo Block
               OpDecorate %_runtimearr_int ArrayStride 4
               OpMemberDecorate %IndexBuffer 0 NonWritable
               OpMemberDecorate %IndexBuffer 0 Offset 0
               OpDecorate %IndexBuffer Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_IndexBuffer PhysicalStorageBuffer
        %int = OpTypeInt 32 1
        %foo = OpTypeStruct %_ptr_PhysicalStorageBuffer_IndexBuffer %int
%_runtimearr_int = OpTypeRuntimeArray %int
%IndexBuffer = OpTypeStruct %_runtimearr_int
%_ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer PhysicalStorageBuffer %IndexBuffer
%_ptr_StorageBuffer_foo = OpTypePointer StorageBuffer %foo
          %_ = OpVariable %_ptr_StorageBuffer_foo StorageBuffer
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_IndexBuffer
     %int_16 = OpConstant %int 16
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
       %main = OpFunction %void None %4
          %6 = OpLabel
         %17 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer %_ %int_0
               OpLine %1 42 24
         %18 = OpLoad %_ptr_PhysicalStorageBuffer_IndexBuffer %17
         %21 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %18 %int_0 %int_16
         %22 = OpLoad %int %21 Aligned 4
         %24 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
               OpStore %24 %22
               OpReturn
               OpFunctionEnd
    )";

    BasicSingleStorageBufferComputeOOB(shader_source, "Shader validation error occurred at line 42, column 24");
}

TEST_F(NegativeGpuAVShaderDebugInfo, OpLineColumn) {
    TEST_DESCRIPTION("Make sure the column in OpLine will add value to show which part the error occured");

    char const *shader_source = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
          %2 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
          %1 = OpString "a.comp"
               OpSource GLSL 450 %1 "#version 450
#extension GL_EXT_buffer_reference : enable
layout(buffer_reference, std430) readonly buffer IndexBuffer { int indices[]; };
layout(set = 0, binding = 0) buffer foo { IndexBuffer data; int x; };
void main() {
    x = data.indices[16];
}"
               OpMemberDecorate %foo 0 Offset 0
               OpMemberDecorate %foo 1 Offset 8
               OpDecorate %foo Block
               OpDecorate %_runtimearr_int ArrayStride 4
               OpMemberDecorate %IndexBuffer 0 NonWritable
               OpMemberDecorate %IndexBuffer 0 Offset 0
               OpDecorate %IndexBuffer Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_IndexBuffer PhysicalStorageBuffer
        %int = OpTypeInt 32 1
        %foo = OpTypeStruct %_ptr_PhysicalStorageBuffer_IndexBuffer %int
%_runtimearr_int = OpTypeRuntimeArray %int
%IndexBuffer = OpTypeStruct %_runtimearr_int
%_ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer PhysicalStorageBuffer %IndexBuffer
%_ptr_StorageBuffer_foo = OpTypePointer StorageBuffer %foo
          %_ = OpVariable %_ptr_StorageBuffer_foo StorageBuffer
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_IndexBuffer
     %int_16 = OpConstant %int 16
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
       %main = OpFunction %void None %4
          %6 = OpLabel
         %17 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer %_ %int_0
               OpLine %1 6 14
         %18 = OpLoad %_ptr_PhysicalStorageBuffer_IndexBuffer %17
         %21 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %18 %int_0 %int_16
         %22 = OpLoad %int %21 Aligned 4
         %24 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
               OpStore %24 %22
               OpReturn
               OpFunctionEnd
    )";

    BasicSingleStorageBufferComputeOOB(shader_source, "    x = data.indices[16];\n             ^");
}

TEST_F(NegativeGpuAVShaderDebugInfo, BasicGlslang) {
    TEST_DESCRIPTION("Make sure basic OpLine and OpSource are working with glslang");

    // Manually ran:
    //   glslangValidator -V -g in.comp -o out.spv --target-env vulkan1.2
    char const *shader_source = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
          %2 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
          %1 = OpString "a.comp"
               OpSource GLSL 450 %1 "#version 450
#extension GL_EXT_buffer_reference : enable
layout(buffer_reference, std430) readonly buffer IndexBuffer {
    int indices[];
};
layout(set = 0, binding = 0) buffer foo {
    IndexBuffer data;
    int x;
};
void main() {
    x = data.indices[16];
}"
               OpSourceExtension "GL_EXT_buffer_reference"
               OpName %main "main"
               OpName %foo "foo"
               OpMemberName %foo 0 "data"
               OpMemberName %foo 1 "x"
               OpName %IndexBuffer "IndexBuffer"
               OpMemberName %IndexBuffer 0 "indices"
               OpName %_ ""
               OpModuleProcessed "client vulkan100"
               OpModuleProcessed "target-env spirv1.5"
               OpModuleProcessed "target-env vulkan1.2"
               OpModuleProcessed "entry-point main"
               OpMemberDecorate %foo 0 Offset 0
               OpMemberDecorate %foo 1 Offset 8
               OpDecorate %foo Block
               OpDecorate %_runtimearr_int ArrayStride 4
               OpMemberDecorate %IndexBuffer 0 NonWritable
               OpMemberDecorate %IndexBuffer 0 Offset 0
               OpDecorate %IndexBuffer Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_IndexBuffer PhysicalStorageBuffer
        %int = OpTypeInt 32 1
        %foo = OpTypeStruct %_ptr_PhysicalStorageBuffer_IndexBuffer %int
%_runtimearr_int = OpTypeRuntimeArray %int
%IndexBuffer = OpTypeStruct %_runtimearr_int
%_ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer PhysicalStorageBuffer %IndexBuffer
%_ptr_StorageBuffer_foo = OpTypePointer StorageBuffer %foo
          %_ = OpVariable %_ptr_StorageBuffer_foo StorageBuffer
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_IndexBuffer
     %int_16 = OpConstant %int 16
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
               OpLine %1 10 11
       %main = OpFunction %void None %4
          %6 = OpLabel
               OpLine %1 11 0
         %17 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer %_ %int_0
         %18 = OpLoad %_ptr_PhysicalStorageBuffer_IndexBuffer %17
         %21 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %18 %int_0 %int_16
         %22 = OpLoad %int %21 Aligned 4
         %24 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
               OpStore %24 %22
               OpReturn
               OpFunctionEnd
    )";

    BasicSingleStorageBufferComputeOOB(
        shader_source,
        "SPIR-V Instruction Index = 52\nShader validation error occurred in file a.comp at line 11\n\n    x = data.indices[16];");
}

TEST_F(NegativeGpuAVShaderDebugInfo, GlslLineDerective) {
    TEST_DESCRIPTION("Use the #line derective in GLSL");

    char const *shader_source = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
          %2 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint GLCompute %main "main" %_
               OpExecutionMode %main LocalSize 1 1 1
          %1 = OpString "a.comp"
               OpSource GLSL 450 %1 "#version 450
#extension GL_EXT_buffer_reference : enable
layout(buffer_reference, std430) readonly buffer IndexBuffer {
    int indices[];
};
layout(set = 0, binding = 0) buffer foo {
    IndexBuffer data;
    int x;
};
void main()  {
#line 9000
    x = data.indices[16];
}"
               OpMemberDecorate %foo 0 Offset 0
               OpMemberDecorate %foo 1 Offset 8
               OpDecorate %foo Block
               OpDecorate %_runtimearr_int ArrayStride 4
               OpMemberDecorate %IndexBuffer 0 NonWritable
               OpMemberDecorate %IndexBuffer 0 Offset 0
               OpDecorate %IndexBuffer Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
               OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_IndexBuffer PhysicalStorageBuffer
        %int = OpTypeInt 32 1
        %foo = OpTypeStruct %_ptr_PhysicalStorageBuffer_IndexBuffer %int
%_runtimearr_int = OpTypeRuntimeArray %int
%IndexBuffer = OpTypeStruct %_runtimearr_int
%_ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer PhysicalStorageBuffer %IndexBuffer
%_ptr_StorageBuffer_foo = OpTypePointer StorageBuffer %foo
          %_ = OpVariable %_ptr_StorageBuffer_foo StorageBuffer
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_IndexBuffer
     %int_16 = OpConstant %int 16
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
%_ptr_StorageBuffer_int = OpTypePointer StorageBuffer %int
               OpLine %1 10 11
       %main = OpFunction %void None %4
          %6 = OpLabel
               OpLine %1 9000 0
         %17 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_IndexBuffer %_ %int_0
         %18 = OpLoad %_ptr_PhysicalStorageBuffer_IndexBuffer %17
         %21 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %18 %int_0 %int_16
         %22 = OpLoad %int %21 Aligned 4
         %24 = OpAccessChain %_ptr_StorageBuffer_int %_ %int_1
               OpStore %24 %22
               OpReturn
               OpFunctionEnd
    )";

    BasicSingleStorageBufferComputeOOB(
        shader_source, "Shader validation error occurred in file a.comp at line 9000\n\n9000:     x = data.indices[16];");
}

TEST_F(NegativeGpuAVShaderDebugInfo, PipelineHandles) {
    TEST_DESCRIPTION("Make sure we are printing out which pipeline the error is from");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, std430) readonly buffer IndexBuffer {
            int indices[];
        };
        layout(set = 0, binding = 0) buffer foo {
            IndexBuffer data;
            int x;
        };
        void main()  {
            x = data.indices[16];
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipe.CreateComputePipeline();

    const char *object_name = "bad_pipeline";
    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_PIPELINE;
    name_info.objectHandle = (uint64_t)pipe.Handle();
    name_info.pObjectName = object_name;
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    auto data = static_cast<VkDeviceAddress *>(in_buffer.memory().map());
    data[0] = block_buffer.address();
    in_buffer.memory().unmap();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    // UNASSIGNED-Device address out of bounds
    m_errorMonitor->SetDesiredError("Pipeline (bad_pipeline)");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, ShaderObjectHandle) {
    TEST_DESCRIPTION("Make sure we are printing out which shader object the error is from");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddRequiredFeature(vkt::Feature::shaderObject);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    static const char comp_src[] = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, std430) readonly buffer IndexBuffer {
            int indices[];
        };
        layout(set = 0, binding = 0) buffer foo {
            IndexBuffer data;
            int x;
        };
        void main()  {
            x = data.indices[16];
        }
    )glsl";

    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    auto data = static_cast<VkDeviceAddress *>(in_buffer.memory().map());
    data[0] = block_buffer.address();
    in_buffer.memory().unmap();

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    descriptor_set.WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    VkDescriptorSetLayout descriptorSetLayout = descriptor_set.layout_.handle();
    VkShaderStageFlagBits shader_stages[] = {VK_SHADER_STAGE_COMPUTE_BIT};
    const vkt::Shader comp_shader(*m_device, shader_stages[0], GLSLToSPV(shader_stages[0], comp_src), &descriptorSetLayout);

    const char *object_name = "bad_shader_object";
    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_SHADER_EXT;
    name_info.objectHandle = (uint64_t)comp_shader.handle();
    name_info.pObjectName = object_name;
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    m_command_buffer.begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindShadersEXT(m_command_buffer.handle(), 1, shader_stages, &comp_shader.handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    // UNASSIGNED-Device address out of bounds
    m_errorMonitor->SetDesiredError("Shader Object (bad_shader_object)");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, CommandBufferCommandIndex) {
    TEST_DESCRIPTION("Make sure we print which index in the command buffer the issue occured");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, std430) readonly buffer IndexBuffer {
            int indices[];
        };
        layout(set = 0, binding = 0) buffer foo {
            IndexBuffer data;
            int x;
        };
        void main()  {
            x = data.indices[16];
        }
    )glsl";

    CreateComputePipelineHelper bad_pipe(*this);
    bad_pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    bad_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    bad_pipe.CreateComputePipeline();

    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    auto data = static_cast<VkDeviceAddress *>(in_buffer.memory().map());
    data[0] = block_buffer.address();
    in_buffer.memory().unmap();

    bad_pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    bad_pipe.descriptor_set_->UpdateDescriptorSets();

    CreateComputePipelineHelper empty_compute_pipe(*this);
    empty_compute_pipe.CreateComputePipeline();
    CreatePipelineHelper empty_graphics_pipe(*this);
    empty_graphics_pipe.CreateGraphicsPipeline();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, empty_compute_pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);  // dispatch index 0

    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, empty_graphics_pipe.Handle());
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);  // draw index 0
    m_command_buffer.EndRenderPass();

    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe.pipeline_layout_.handle(), 0, 1,
                              &bad_pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);  // dispatch index 1
    m_command_buffer.end();

    // UNASSIGNED-Device address out of bounds
    m_errorMonitor->SetDesiredError("Compute Dispatch Index 1");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAVShaderDebugInfo, DISABLED_StageInfo) {
    TEST_DESCRIPTION("Make sure we print the stage info correctly");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderInt64);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, std430) readonly buffer IndexBuffer {
            int indices[];
        };
        layout(set = 0, binding = 0) buffer foo {
            IndexBuffer data;
            int x;
        };
        void main()  {
            if (gl_WorkGroupSize.x == 1) {
                x = data.indices[16];
            }
        }
    )glsl";

    CreateComputePipelineHelper bad_pipe(*this);
    bad_pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    bad_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    bad_pipe.CreateComputePipeline();

    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    auto data = static_cast<VkDeviceAddress *>(in_buffer.memory().map());
    data[0] = block_buffer.address();
    in_buffer.memory().unmap();

    bad_pipe.descriptor_set_->WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    bad_pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe.pipeline_layout_.handle(), 0, 1,
                              &bad_pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer.handle(), 2, 1, 1);
    m_command_buffer.end();

    // UNASSIGNED-Device address out of bounds
    m_errorMonitor->SetDesiredError("Global invocation ID (x, y, z) = (1, 0, 0)");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}
