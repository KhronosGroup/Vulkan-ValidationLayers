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
#include "shader_helper.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

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

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;
    mapping.sourceData.constantOffset.heapArrayStride = 0;

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

    const uint32_t buffer_index = 16u;

    VkDeviceSize resource_heap_tracker = 0u;
    const VkDeviceSize image_offset = AlignedAppend(resource_heap_tracker, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    const VkDeviceSize image_size = resource_heap_tracker - image_offset;
    const VkDeviceSize buffer_offset = sizeof(float) * 4u * buffer_index;
    const VkDeviceSize buffer_size = resource_heap_tracker - buffer_offset + 256;  // 256 padding
    const VkDeviceSize resource_heap_app_size = resource_heap_tracker;

    CreateResourceHeap(resource_heap_app_size);

    vkt::Buffer buffer(*m_device, sizeof(float) * 4u, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    VkHostAddressRangeEXT resource_host[2];
    resource_host[0].address = resource_heap_data_ + image_offset;
    resource_host[0].size = static_cast<size_t>(image_size);
    resource_host[1].address = resource_heap_data_ + buffer_offset;
    resource_host[1].size = static_cast<size_t>(buffer_size);

    VkImageViewCreateInfo view_info = image.BasicViewCreatInfo(VK_IMAGE_ASPECT_COLOR_BIT);

    VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
    image_info.pView = &view_info;
    image_info.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDeviceAddressRangeEXT buffer_address_range = buffer.AddressRange();

    VkResourceDescriptorInfoEXT descriptor_info[2];
    descriptor_info[0] = vku::InitStructHelper();
    descriptor_info[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptor_info[0].data.pImage = &image_info;
    descriptor_info[1] = vku::InitStructHelper();
    descriptor_info[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info[1].data.pAddressRange = &buffer_address_range;
    vk::WriteResourceDescriptorsEXT(*m_device, 2u, descriptor_info, resource_host);

    VkDeviceSize sampler_desc_heap_size_tracker = 0u;
    const VkDeviceSize sampler_offset = AlignedAppend(sampler_desc_heap_size_tracker, VK_DESCRIPTOR_TYPE_SAMPLER);
    const VkDeviceSize sampler_size = sampler_desc_heap_size_tracker - sampler_offset;

    CreateSamplerHeap(sampler_desc_heap_size_tracker);

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();

    VkHostAddressRangeEXT sampler_host = {sampler_heap_data_ + sampler_offset, static_cast<size_t>(sampler_size)};
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

    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = VK_ACCESS_NONE;
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.image = image;
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u};
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0u, 0u, nullptr, 0u,
                           nullptr, 1u, &image_barrier);

    VkClearColorValue color = {};
    color.float32[0] = 0.2f;
    color.float32[1] = 0.4f;
    color.float32[2] = 0.6f;
    color.float32[3] = 0.8f;
    vk::CmdClearColorImage(m_command_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &color, 1u,
                           &image_barrier.subresourceRange);

    image_barrier.srcAccessMask = image_barrier.dstAccessMask;
    image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    image_barrier.oldLayout = image_barrier.newLayout;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0u, 0u, nullptr,
                           0u, nullptr, 1u, &image_barrier);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    BindResourceHeap();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11308");
    vk::CmdDispatch(m_command_buffer, 1u, 1u, 1u);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDescriptorHeapUntyped, SecondaryCmdBufferHeapMissingInheritance) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());
    InitRenderTarget();

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = buffer.AddressRange();
    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;

    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

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

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = buffer.AddressRange();
    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;

    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

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
    bind_resource_info.heapRange = resource_heap_.AddressRange();
    bind_resource_info.reservedRangeOffset = resource_heap_.CreateInfo().size - heap_props.minResourceHeapReservedRange;
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

    const uint32_t buffer_index = 16u;

    const VkDeviceSize image_offset = 0u;
    const VkDeviceSize image_size = heap_props.imageDescriptorSize;
    const VkDeviceSize buffer_offset = heap_props.bufferDescriptorSize * buffer_index;
    const VkDeviceSize buffer_size = heap_props.bufferDescriptorSize;
    const VkDeviceSize resource_heap_app_size = buffer_offset + buffer_size;

    CreateResourceHeap(resource_heap_app_size);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    VkHostAddressRangeEXT resource_host[2];
    resource_host[0].address = resource_heap_data_ + image_offset;
    resource_host[0].size = static_cast<size_t>(image_size);
    resource_host[1].address = resource_heap_data_ + buffer_offset;
    resource_host[1].size = static_cast<size_t>(buffer_size);

    VkImageViewCreateInfo view_info = image.BasicViewCreatInfo(VK_IMAGE_ASPECT_COLOR_BIT);

    VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
    image_info.pView = &view_info;
    image_info.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDeviceAddressRangeEXT buffer_address_range = buffer.AddressRange();

    VkResourceDescriptorInfoEXT descriptor_info[2];
    descriptor_info[0] = vku::InitStructHelper();
    descriptor_info[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptor_info[0].data.pImage = &image_info;
    descriptor_info[1] = vku::InitStructHelper();
    descriptor_info[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info[1].data.pAddressRange = &buffer_address_range;
    vk::WriteResourceDescriptorsEXT(*m_device, 2u, descriptor_info, resource_host);

    VkDeviceSize sampler_desc_heap_size_tracker = 0u;
    const VkDeviceSize sampler_offset = AlignedAppend(sampler_desc_heap_size_tracker, VK_DESCRIPTOR_TYPE_SAMPLER);
    const VkDeviceSize sampler_size = sampler_desc_heap_size_tracker - sampler_offset;

    CreateSamplerHeap(sampler_desc_heap_size_tracker);

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();

    VkHostAddressRangeEXT sampler_host = {sampler_heap_data_ + sampler_offset, static_cast<size_t>(sampler_size)};
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
    BindSamplerHeap();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-None-11308");
    vk::CmdDispatch(m_command_buffer, 1u, 1u, 1u);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}