/*
 * Copyright (c) 2026 LunarG, Inc.
 * Copyright (c) 2026 Valve Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include <cstdint>
#include "descriptor_heap_object.h"
#include "shader_helper.h"
#include "layer_validation_tests.h"
#include "pipeline_helper.h"

class NegativeDescriptorHeapUntyped : public DescriptorHeapTest {};

TEST_F(NegativeDescriptorHeapUntyped, PipelineLayoutNotNull) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());

    char const* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require
        layout(local_size_x = 1) in;
        layout(descriptor_heap) buffer A { uint a; } heap[];
        void main() {
            heap[0].a = 0;
        }
    )glsl";
    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    m_errorMonitor->SetDesiredError("VUID-VkComputePipelineCreateInfo-layout-07988");
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeapUntyped, ResourceHeapNotBound) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());

    vkt::Buffer buffer_a(*m_device, 32, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeZeroSetAndBindingMapping(0, 0);
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require
        layout(local_size_x = 1) in;
        layout(descriptor_heap) buffer A { uint a; } heap[];
        void main() {
            heap[73].a = 2;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2, &mapping_info);

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11308");
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptorHeapUntyped, SamplerHeapNotBound) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());
    InitRenderTarget();

    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(256);

    char const* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require
        layout(descriptor_heap) uniform texture2D heapTextures[];
        layout(descriptor_heap) uniform sampler heapSamplers[];
        layout(descriptor_heap) buffer ssbo {
            vec4 data;
        } heapBuffer[];
        void main() {
            heapBuffer[16].data = texture(sampler2D(heapTextures[0], heapSamplers[0]), vec2(0.5f));
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2);

    m_command_buffer.Begin();

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    desc_heap.BindResourceHeap(m_command_buffer);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11308");
    vk::CmdDispatch(m_command_buffer, 1u, 1u, 1u);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptorHeapUntyped, SecondaryCmdBufferHeapMissingInheritance) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());
    InitRenderTarget();

    vkt::DescriptorHeap desc_heap(*this);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    desc_heap.CreateResourceHeap(resource_stride);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    desc_heap.WriteBufferDescriptorAtOffset(buffer.AddressRange(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);

    char const* vs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require

        layout(descriptor_heap) buffer A { uint a; } heap[];
        layout(push_constant) uniform PushConstant {
            uint b;
        };
        void main() {
            heap[0].a = b;
            gl_Position = vec4(1.0f);
        }
    )glsl";
    VkShaderObj vert_module = VkShaderObj(*m_device, vs_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_2);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkPipelineShaderStageCreateInfo stage;
    stage = vert_module.GetStageCreateInfo();

    CreatePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.gp_ci_.layout = VK_NULL_HANDLE;
    pipe.gp_ci_.stageCount = 1;
    pipe.gp_ci_.pStages = &stage;
    pipe.CreateGraphicsPipeline(false);

    uint32_t src_data = 4321u;

    VkPushConstantRange push_const_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)};
    VkPipelineLayoutCreateInfo pipeline_layout_info = vku::InitStructHelper();
    pipeline_layout_info.pushConstantRangeCount = 1u;
    pipeline_layout_info.pPushConstantRanges = &push_const_range;
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_info);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceDescriptorHeapInfoEXT inheritance_heap_info = vku::InitStructHelper();

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper(&inheritance_heap_info);
    inheritance_info.renderPass = RenderPass();
    inheritance_info.framebuffer = Framebuffer();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    begin_info.pInheritanceInfo = &inheritance_info;

    secondary.Begin(&begin_info);
    vk::CmdBindPipeline(secondary, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    secondary.PushData(0, sizeof(uint32_t), &src_data);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-11308");
    vk::CmdDraw(secondary, 3u, 1u, 0u, 0u);
    m_errorMonitor->VerifyFound();
    secondary.End();
}

TEST_F(NegativeDescriptorHeapUntyped, SecondaryCmdBufferResourceHeapUnbound) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());
    InitRenderTarget();

    vkt::DescriptorHeap desc_heap(*this);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    desc_heap.CreateResourceHeap(resource_stride);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    desc_heap.WriteBufferDescriptorAtOffset(buffer.AddressRange(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);

    char const* vs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require

        layout(descriptor_heap) buffer A { uint a; } heap[];
        layout(push_constant) uniform PushConstant {
            uint b;
        };
        void main() {
            heap[0].a = b;
            gl_Position = vec4(1.0f);
        }
    )glsl";
    VkShaderObj vert_module = VkShaderObj(*m_device, vs_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_2);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkPipelineShaderStageCreateInfo stage;
    stage = vert_module.GetStageCreateInfo();

    CreatePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.gp_ci_.layout = VK_NULL_HANDLE;
    pipe.gp_ci_.stageCount = 1;
    pipe.gp_ci_.pStages = &stage;
    pipe.CreateGraphicsPipeline(false);

    uint32_t src_data = 4321u;

    VkPushConstantRange push_const_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)};
    VkPipelineLayoutCreateInfo pipeline_layout_info = vku::InitStructHelper();
    pipeline_layout_info.pushConstantRangeCount = 1u;
    pipeline_layout_info.pPushConstantRanges = &push_const_range;
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_info);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = desc_heap.resource_heap_.AddressRange();
    bind_resource_info.reservedRangeOffset = desc_heap.resource_heap_.CreateInfo().size - heap_props.minResourceHeapReservedRange;
    bind_resource_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;

    VkCommandBufferInheritanceDescriptorHeapInfoEXT inheritance_heap_info = vku::InitStructHelper();
    inheritance_heap_info.pResourceHeapBindInfo = &bind_resource_info;

    VkCommandBufferInheritanceInfo inheritance_info = vku::InitStructHelper(&inheritance_heap_info);
    inheritance_info.renderPass = RenderPass();
    inheritance_info.framebuffer = Framebuffer();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    begin_info.pInheritanceInfo = &inheritance_info;

    secondary.Begin(&begin_info);
    vk::CmdBindPipeline(secondary, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    secondary.PushData(0, sizeof(uint32_t), &src_data);
    vk::CmdDraw(secondary, 3u, 1u, 0u, 0u);
    secondary.End();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-commandBuffer-11474");
    vk::CmdExecuteCommands(m_command_buffer, 1, &secondary.handle());
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptorHeapUntyped, OffsetIdNotAlignedBuffer) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());

    // What the shader looks like
    //
    // layout(descriptor_heap) struct {
    //    layout(offset = 0) buffer a { uint data; } x;
    //    layout(offset = 5) buffer a { uint data; } y;
    // };
    char const* cs_source = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap %_
               OpExecutionMode %main LocalSize 1 1 1
               OpName %heap_struct "heap_struct"
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %A Block
               OpMemberDecorate %A 0 Offset 0
               OpDecorate %PushConstant Block
               OpMemberDecorate %PushConstant 0 Offset 0
               OpMemberDecorate %heap_struct 0 Offset 0
               OpMemberDecorate %heap_struct 1 Offset 5
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
       %uint = OpTypeInt 32 0
          %A = OpTypeStruct %uint
      %int_0 = OpConstant %int 0
%PushConstant = OpTypeStruct %uint
%_ptr_PushConstant_PushConstant = OpTypePointer PushConstant %PushConstant
          %_ = OpVariable %_ptr_PushConstant_PushConstant PushConstant
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer
         %21 = OpTypeBufferEXT StorageBuffer
%heap_struct = OpTypeStruct %21 %21
       %main = OpFunction %void None %3
          %5 = OpLabel
         %17 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
         %18 = OpLoad %uint %17
         %20 = OpUntypedAccessChainKHR %_ptr_UniformConstant %heap_struct %resource_heap %int_1
         %24 = OpBufferPointerEXT %_ptr_StorageBuffer %20
         %25 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %A %24 %int_0
               OpStore %25 %18
               OpReturn
               OpFunctionEnd

    )";
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-bufferDescriptorAlignment-11478");
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_3, nullptr, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeapUntyped, OffsetIdNotAlignedMixedType) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());

    bool is_aligned = (heap_props.samplerDescriptorSize & (heap_props.imageDescriptorAlignment - 1)) == 0;
    if (is_aligned) {
        GTEST_SKIP() << "properties aren't different to trigger error";
    }

    // struct heap {
    //     (offset = samplerDescriptorSize) image;
    //     (offset = samplerDescriptorSize) sampler;
    // };
    char const* cs_source = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap %sampler_heap
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %sampler_heap BuiltIn SamplerHeapEXT
               OpMemberDecorateIdEXT %heap_struct 0 OffsetIdEXT %sampler_size
               OpMemberDecorateIdEXT %heap_struct 1 OffsetIdEXT %sampler_size
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
%_ptr_Uniform = OpTypeUntypedPointerKHR Uniform
%sampler_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
  %float_0_5 = OpConstant %float 0.5
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %30 = OpConstantComposite %v2float %float_0_5 %float_0_5
 %type_image = OpTypeImage %float 2D 0 0 0 1 Unknown
%type_sampler = OpTypeSampler
         %26 = OpTypeSampledImage %type_image

%sampler_size = OpConstantSizeOfEXT %int %type_sampler
%image_size = OpConstantSizeOfEXT %int %type_image
%heap_struct = OpTypeStruct %type_image %type_sampler

       %main = OpFunction %void None %3
          %5 = OpLabel
       %data = OpVariable %_ptr_Function_v4float Function
         %16 = OpUntypedAccessChainKHR %_ptr_UniformConstant %heap_struct %resource_heap %int_0
         %19 = OpLoad %type_image %16
         %22 = OpUntypedAccessChainKHR %_ptr_UniformConstant %heap_struct %sampler_heap %int_1
         %25 = OpLoad %type_sampler %22
         %27 = OpSampledImage %26 %19 %25
         %32 = OpImageSampleExplicitLod %v4float %27 %30 Lod %float_0
               OpStore %data %32
               OpReturn
               OpFunctionEnd
    )";
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-imageDescriptorAlignment-11477");
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_3, nullptr, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeapUntyped, OffsetIdNotAlignedImageAndSampler) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());

    char const* cs_source = R"(
            OpCapability Shader
            OpCapability UntypedPointersKHR
            OpCapability DescriptorHeapEXT
            OpExtension "SPV_EXT_descriptor_heap"
            OpExtension "SPV_KHR_untyped_pointers"
            OpMemoryModel Logical GLSL450
            OpEntryPoint GLCompute %main "main"
            OpExecutionMode %main LocalSize 1 1 1
            OpName %struct_a "struct_a"
            OpName %struct_b "struct_b"
            OpMemberDecorate %struct_a 0 Offset 0
            OpMemberDecorate %struct_a 1 Offset 7
            OpMemberDecorate %struct_b 0 Offset 0
            OpMemberDecorate %struct_b 1 Offset 32
            OpMemberDecorate %struct_b 2 Offset 39
    %void = OpTypeVoid
       %3 = OpTypeFunction %void
     %int = OpTypeInt 32 1
   %float = OpTypeFloat 32
   %image = OpTypeImage %float 2D 0 0 0 1 Unknown
 %sampler = OpTypeSampler
%struct_a = OpTypeStruct %int %image
%struct_b = OpTypeStruct %struct_a %int %sampler
    %main = OpFunction %void None %3
       %5 = OpLabel
            OpReturn
            OpFunctionEnd

    )";
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-samplerDescriptorAlignment-11476");
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-imageDescriptorAlignment-11477");
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_3, nullptr, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeapUntyped, OffsetIdNotAlignedConstant) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());

    char const* cs_source = R"(
             OpCapability Shader
             OpCapability UntypedPointersKHR
             OpCapability DescriptorHeapEXT
             OpExtension "SPV_EXT_descriptor_heap"
             OpExtension "SPV_KHR_untyped_pointers"
             OpMemoryModel Logical GLSL450
             OpEntryPoint GLCompute %main "main"
             OpExecutionMode %main LocalSize 1 1 1
             OpName %struct_a "struct_a"
             OpMemberDecorateIdEXT %struct_a 0 OffsetIdEXT %int_0
             OpMemberDecorateIdEXT %struct_a 1 OffsetIdEXT %int_7
     %void = OpTypeVoid
        %3 = OpTypeFunction %void
     %int = OpTypeInt 32 1
   %int_0 = OpConstant %int 0
   %int_7 = OpConstant %int 7
   %float = OpTypeFloat 32
   %image = OpTypeImage %float 2D 0 0 0 1 Unknown
%struct_a = OpTypeStruct %int %image
    %main = OpFunction %void None %3
       %5 = OpLabel
            OpReturn
            OpFunctionEnd
    )";
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-imageDescriptorAlignment-11477");
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_3, nullptr, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeapUntyped, OffsetIdNotAlignedSpecConstantDefault) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());

    char const* cs_source = R"(
             OpCapability Shader
             OpCapability UntypedPointersKHR
             OpCapability DescriptorHeapEXT
             OpExtension "SPV_EXT_descriptor_heap"
             OpExtension "SPV_KHR_untyped_pointers"
             OpMemoryModel Logical GLSL450
             OpEntryPoint GLCompute %main "main"
             OpExecutionMode %main LocalSize 1 1 1
             OpName %struct_a "struct_a"
             OpMemberDecorateIdEXT %struct_a 0 OffsetIdEXT %uint_0
             OpMemberDecorateIdEXT %struct_a 1 OffsetIdEXT %result
     %void = OpTypeVoid
        %3 = OpTypeFunction %void
    %uint = OpTypeInt 32 0
  %uint_0 = OpConstant %uint 0
  %uint_1 = OpConstant %uint 1
  %uint_4 = OpConstant %uint 4
  %uint_8 = OpConstant %uint 8
     %mul = OpSpecConstantOp %uint IMul %uint_4 %uint_8
  %result = OpSpecConstantOp %uint IAdd %mul %uint_1
   %float = OpTypeFloat 32
   %image = OpTypeImage %float 2D 0 0 0 1 Unknown
%struct_a = OpTypeStruct %uint %image
    %main = OpFunction %void None %3
       %5 = OpLabel
            OpReturn
            OpFunctionEnd
    )";
    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-imageDescriptorAlignment-11477");
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_3, nullptr, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorHeapUntyped, SamplerHeapBoundResourceHeapNotBound) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());
    InitRenderTarget();

    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateSamplerHeap(heap_props.samplerDescriptorSize);

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    VkHostAddressRangeEXT sampler_host = {desc_heap.sampler_heap_data_, static_cast<size_t>(heap_props.samplerDescriptorSize)};
    vk::WriteSamplerDescriptorsEXT(*m_device, 1u, &sampler_info, &sampler_host);

    char const* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require
        layout(descriptor_heap) uniform texture2D heapTextures[];
        layout(descriptor_heap) uniform sampler heapSamplers[];
        layout(descriptor_heap) buffer ssbo {
            vec4 data;
        } heapBuffer[];
        void main() {
            heapBuffer[16].data = texture(sampler2D(heapTextures[0], heapSamplers[0]), vec2(0.5f));
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2);

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    desc_heap.BindSamplerHeap(m_command_buffer);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11308");
    vk::CmdDispatch(m_command_buffer, 1u, 1u, 1u);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}