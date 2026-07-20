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
#include "test_framework.h"
#include "layer_validation_tests.h"
#include "pipeline_helper.h"

constexpr uint32_t kMaxSSBO = 128;  // max bufferDescriptorSize is allowed to be

class PositiveDescriptorHeapUntyped : public DescriptorHeapTest {};

void DescriptorHeapTest::InitUntypedDescriptorHeap() {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::descriptorHeap);
    RETURN_IF_SKIP(Init());
    heap_props.pNext = &tensor_heap_props;
    GetPhysicalDeviceProperties2(heap_props);
}

TEST_F(PositiveDescriptorHeapUntyped, BasicBuffer) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());

    vkt::DescriptorHeap desc_heap(*this);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    desc_heap.CreateResourceHeap(resource_stride);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    desc_heap.WriteBufferDescriptorAtOffset(buffer.AddressRange(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);

    char const* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require
        layout(local_size_x = 1) in;
        layout(descriptor_heap) buffer A { uint a; } heap[];
        layout(push_constant) uniform PushConstant {
            uint b;
        };
        void main() {
            heap[0].a = b;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2);

    uint32_t src_data = 4321u;

    VkPushConstantRange push_const_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)};
    VkPipelineLayoutCreateInfo pipeline_layout_info = vku::InitStructHelper();
    pipeline_layout_info.pushConstantRangeCount = 1u;
    pipeline_layout_info.pPushConstantRanges = &push_const_range;
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_info);

    m_command_buffer.Begin();
    vk::CmdPushConstants(m_command_buffer, pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &src_data);
    desc_heap.BindResourceHeap(m_command_buffer);
    m_command_buffer.PushData(0, sizeof(uint32_t), &src_data);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* data = static_cast<uint32_t*>(buffer.Memory().Map());
        ASSERT_EQ(data[0], src_data);
    }
}

TEST_F(PositiveDescriptorHeapUntyped, Buffer) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());

    vkt::DescriptorHeap desc_heap(*this);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    desc_heap.CreateResourceHeap(resource_stride);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    desc_heap.WriteBufferDescriptorAtOffset(buffer.AddressRange(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeZeroSetAndBindingMapping(0, 0);
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    const char* spirv = R"(
        OpCapability Shader
        OpCapability UntypedPointersKHR
        OpExtension "SPV_KHR_untyped_pointers"
        OpMemoryModel Logical GLSL450
        OpEntryPoint GLCompute %main "main" %var
        OpExecutionMode %main LocalSize 1 1 1
        OpDecorate %block Block
        OpMemberDecorate %block 0 Offset 0
        OpDecorate %var DescriptorSet 0
        OpDecorate %var Binding 0
        %void = OpTypeVoid
        %void_fn = OpTypeFunction %void
        %int = OpTypeInt 32 0
        %int_0 = OpConstant %int 0
        %int_1 = OpConstant %int 1
        %block = OpTypeStruct %int
        %ptr = OpTypeUntypedPointerKHR StorageBuffer
        %var = OpUntypedVariableKHR %ptr StorageBuffer %block
        %main = OpFunction %void None %void_fn
        %entry = OpLabel
        %access = OpUntypedAccessChainKHR %ptr %block %var %int_0
        OpAtomicStore %access %int_1 %int_0 %int_1
        OpReturn
        OpFunctionEnd
    )";
    vkt::HeapComputePipeline pipe(*m_device, spirv, SPV_ENV_VULKAN_1_2, &mapping_info, SPV_SOURCE_ASM);

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* data = static_cast<uint32_t*>(buffer.Memory().Map());
        ASSERT_EQ(data[0], 1u);
    }
}

TEST_F(PositiveDescriptorHeapUntyped, ArrayStrideIdEXT) {
    SetTargetApiVersion(VK_API_VERSION_1_4);
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());

    const char* cs_source = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpCapability ImageBuffer
               OpExtension "SPV_KHR_untyped_pointers"
               OpExtension "SPV_EXT_descriptor_heap"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main" %2
               OpExecutionMode %1 LocalSize 1 1 1
               OpDecorate %2 BuiltIn ResourceHeapEXT
               OpDecorate %3 SpecId 0
               OpDecorateId %4 ArrayStrideIdEXT %3
          %5 = OpTypeVoid
          %6 = OpTypeFunction %5
          %7 = OpTypeInt 32 0
          %8 = OpConstant %7 0
          %9 = OpConstant %7 2
         %10 = OpConstant %7 51966
         %11 = OpConstant %7 0
          %3 = OpSpecConstant %7 0
         %12 = OpTypeUntypedPointerKHR UniformConstant
          %2 = OpUntypedVariableKHR %12 UniformConstant
         %13 = OpTypeImage %7 Buffer 0 0 0 2 R32ui
          %4 = OpTypeRuntimeArray %13
          %1 = OpFunction %5 None %6
         %14 = OpLabel
         %15 = OpUntypedAccessChainKHR %12 %4 %2 %9
         %16 = OpLoad %13 %15
               OpImageWrite %16 %8 %10
               OpReturn
               OpFunctionEnd
    )";

    const uint32_t data = (uint32_t)heap_props.imageDescriptorSize;
    const VkSpecializationMapEntry entry = {0, 0, sizeof(uint32_t)};
    const VkSpecializationInfo specialization_info = {1, &entry, sizeof(uint32_t), &data};

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_4, nullptr, SPV_SOURCE_ASM, &specialization_info);
}

TEST_F(PositiveDescriptorHeapUntyped, ImageAndSampler) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());
    InitRenderTarget();

    const uint32_t buffer_index = 16u;

    const VkDeviceSize image_offset = 0u;
    const VkDeviceSize image_size = heap_props.imageDescriptorSize;
    const VkDeviceSize buffer_offset = heap_props.bufferDescriptorSize * buffer_index;
    const VkDeviceSize buffer_size = heap_props.bufferDescriptorSize;
    const VkDeviceSize resource_heap_app_size = buffer_offset + buffer_size;

    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(resource_heap_app_size);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    VkHostAddressRangeEXT resource_host[2];
    resource_host[0].address = desc_heap.resource_heap_data_ + image_offset;
    resource_host[0].size = static_cast<size_t>(image_size);
    resource_host[1].address = desc_heap.resource_heap_data_ + buffer_offset;
    resource_host[1].size = static_cast<size_t>(buffer_size);

    VkImageViewCreateInfo view_info = image.BasicViewCreatInfo(VK_IMAGE_ASPECT_COLOR_BIT);

    VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
    image_info.pView = &view_info;
    image_info.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDeviceAddressRangeEXT buffer_address_range = {buffer.Address(), 256};

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

    desc_heap.CreateSamplerHeap(sampler_desc_heap_size_tracker);

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    VkHostAddressRangeEXT sampler_host = {desc_heap.sampler_heap_data_ + sampler_offset, static_cast<size_t>(sampler_size)};
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

    m_command_buffer.TransitionLayout(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkClearColorValue color = {{0.2f, 0.4f, 0.6f, 0.8f}};
    VkImageSubresourceRange sub_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u};
    vk::CmdClearColorImage(m_command_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &color, 1u, &sub_range);

    m_command_buffer.TransitionLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    desc_heap.BindResourceHeap(m_command_buffer);
    desc_heap.BindSamplerHeap(m_command_buffer);
    vk::CmdDispatch(m_command_buffer, 1u, 1u, 1u);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        float* data = static_cast<float*>(buffer.Memory().Map());
        for (uint32_t i = 0; i < 4; ++i) {
            ASSERT_EQ(data[i], color.float32[i]);
        }
    }
}

TEST_F(PositiveDescriptorHeapUntyped, ImageAndSamplerSlang) {
    TEST_DESCRIPTION("called without -spirv-unified-descriptor-heap-stride");
    AddRequiredExtensions(VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::computeDerivativeGroupQuads);
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());
    InitRenderTarget();

    const VkDeviceSize resource_stride = std::max(heap_props.imageDescriptorSize, heap_props.bufferDescriptorSize);
    const uint32_t image_index = 8;
    const uint32_t buffer_index = 1;

    const VkDeviceSize image_offset = heap_props.imageDescriptorSize * image_index;
    const VkDeviceSize buffer_offset = heap_props.bufferDescriptorSize * buffer_index;

    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(resource_stride * 10);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    desc_heap.WriteBufferDescriptorAtOffset(buffer.AddressRange(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer_offset);
    desc_heap.WriteImageDescriptorAtOffset(image, image_offset);

    desc_heap.CreateSamplerHeap(heap_props.samplerDescriptorSize);
    desc_heap.WriteSamplerDescriptor();

    // struct SSBO { float4 data; };
    // struct PushConstants {
    //     uint buffer_index;
    //     uint image_index;
    // };
    // [[vk::push_constant]] PushConstants g_Push;
    // [shader("compute")]
    // [numthreads(2, 2, 1)]
    // void main() {
    //     Texture2D<float4> heapTexture = ResourceDescriptorHeap[g_Push.image_index];
    //     SamplerState heapSampler = SamplerDescriptorHeap[0];
    //     float4 sampleColor = heapTexture.Sample(heapSampler, float2(0.5f, 0.5f));
    //     RWStructuredBuffer<SSBO> heapBuffer = ResourceDescriptorHeap[g_Push.buffer_index];
    //     heapBuffer[0].data = sampleColor;
    // }
    char const* cs_source = R"(
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpCapability ComputeDerivativeGroupQuadsKHR
               OpCapability Shader
               OpExtension "SPV_KHR_untyped_pointers"
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_compute_shader_derivatives"
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %slang_resourceHeap %slang_samplerHeap %g_Push
               OpExecutionMode %main DerivativeGroupQuadsKHR
               OpExecutionMode %main LocalSize 2 2 1
               OpDecorate %PushConstants_std430 Block
               OpMemberDecorate %PushConstants_std430 0 Offset 0
               OpMemberDecorate %PushConstants_std430 1 Offset 4
               OpDecorateId %_runtimearr_19 ArrayStrideIdEXT %20
               OpDecorate %slang_resourceHeap BuiltIn ResourceHeapEXT
               OpDecorateId %_runtimearr_28 ArrayStrideIdEXT %29
               OpDecorate %slang_samplerHeap BuiltIn SamplerHeapEXT
               OpDecorateId %_runtimearr_47 ArrayStrideIdEXT %48
               OpMemberDecorate %SSBO_std430 0 Offset 0
               OpDecorate %_runtimearr_SSBO_std430 ArrayStride 16
               OpDecorate %RWStructuredBuffer Block
               OpMemberDecorate %RWStructuredBuffer 0 Offset 0
               OpDecorate %_ptr_StorageBuffer_SSBO_std430 ArrayStride 16
               OpDecorate %_ptr_StorageBuffer_v4float ArrayStride 16
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%PushConstants_std430 = OpTypeStruct %uint %uint
%_ptr_PushConstant_PushConstants_std430 = OpTypePointer PushConstant %PushConstants_std430
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
     %uint_0 = OpConstant %uint 0
      %float = OpTypeFloat 32
         %19 = OpTypeImage %float 2D 2 0 0 1 Unknown
         %20 = OpConstantSizeOfEXT %uint %19
%_runtimearr_19 = OpTypeRuntimeArray %19
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
         %28 = OpTypeSampler
         %29 = OpConstantSizeOfEXT %uint %28
%_runtimearr_28 = OpTypeRuntimeArray %28
         %34 = OpTypeSampledImage %19
    %v4float = OpTypeVector %float 4
    %v2float = OpTypeVector %float 2
  %float_0_5 = OpConstant %float 0.5
         %39 = OpConstantComposite %v2float %float_0_5 %float_0_5
      %int_0 = OpConstant %int 0
         %47 = OpTypeBufferEXT StorageBuffer
         %48 = OpConstantSizeOfEXT %uint %47
%_runtimearr_47 = OpTypeRuntimeArray %47
%SSBO_std430 = OpTypeStruct %v4float
%_runtimearr_SSBO_std430 = OpTypeRuntimeArray %SSBO_std430
%RWStructuredBuffer = OpTypeStruct %_runtimearr_SSBO_std430
%_ptr_StorageBuffer_RWStructuredBuffer = OpTypePointer StorageBuffer %RWStructuredBuffer
%_ptr_StorageBuffer_SSBO_std430 = OpTypePointer StorageBuffer %SSBO_std430
%_ptr_StorageBuffer_v4float = OpTypePointer StorageBuffer %v4float
     %g_Push = OpVariable %_ptr_PushConstant_PushConstants_std430 PushConstant
%slang_resourceHeap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
%slang_samplerHeap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
       %main = OpFunction %void None %3
          %4 = OpLabel
         %12 = OpAccessChain %_ptr_PushConstant_uint %g_Push %int_1
         %13 = OpLoad %uint %12
         %23 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_19 %slang_resourceHeap %13
         %25 = OpLoad %19 %23
         %31 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_28 %slang_samplerHeap %uint_0
         %33 = OpLoad %28 %31
         %35 = OpSampledImage %34 %25 %33
  %__sampled = OpImageSampleImplicitLod %v4float %35 %39 None
         %43 = OpAccessChain %_ptr_PushConstant_uint %g_Push %int_0
         %44 = OpLoad %uint %43
         %50 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_47 %slang_resourceHeap %44
         %55 = OpBufferPointerEXT %_ptr_StorageBuffer_RWStructuredBuffer %50
         %57 = OpAccessChain %_ptr_StorageBuffer_SSBO_std430 %55 %int_0 %int_0
         %59 = OpAccessChain %_ptr_StorageBuffer_v4float %57 %int_0
               OpStore %59 %__sampled
               OpReturn
               OpFunctionEnd
    )";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2, nullptr, SPV_SOURCE_ASM);

    m_command_buffer.Begin();
    m_command_buffer.PushData(0, 4, &buffer_index);
    m_command_buffer.PushData(4, 4, &image_index);

    m_command_buffer.TransitionLayout(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkClearColorValue color = {{0.2f, 0.4f, 0.6f, 0.8f}};
    VkImageSubresourceRange sub_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u};
    vk::CmdClearColorImage(m_command_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &color, 1u, &sub_range);

    m_command_buffer.TransitionLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    desc_heap.BindResourceHeap(m_command_buffer);
    desc_heap.BindSamplerHeap(m_command_buffer);
    vk::CmdDispatch(m_command_buffer, 1u, 1u, 1u);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        float* data = static_cast<float*>(buffer.Memory().Map());
        for (uint32_t i = 0; i < 4; ++i) {
            ASSERT_EQ(data[i], color.float32[i]);
        }
    }
}

TEST_F(PositiveDescriptorHeapUntyped, ImageAndSamplerSlangUnified) {
    TEST_DESCRIPTION("called with -spirv-unified-descriptor-heap-stride");
    AddRequiredExtensions(VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::computeDerivativeGroupQuads);
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());
    InitRenderTarget();

    const VkDeviceSize resource_stride = std::max(heap_props.imageDescriptorSize, heap_props.bufferDescriptorSize);
    const uint32_t image_index = 1;
    const uint32_t buffer_index = 2;

    const VkDeviceSize image_offset = resource_stride * image_index;
    const VkDeviceSize buffer_offset = resource_stride * buffer_index;

    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(resource_stride * 4);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    desc_heap.WriteBufferDescriptorAtOffset(buffer.AddressRange(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, buffer_offset);
    desc_heap.WriteImageDescriptorAtOffset(image, image_offset);

    desc_heap.CreateSamplerHeap(heap_props.samplerDescriptorSize);
    desc_heap.WriteSamplerDescriptor();

    // struct SSBO { float4 data; };
    // struct PushConstants {
    //     uint buffer_index;
    //     uint image_index;
    // };
    // [[vk::push_constant]] PushConstants g_Push;
    // [shader("compute")]
    // [numthreads(2, 2, 1)]
    // void main() {
    //     Texture2D<float4> heapTexture = ResourceDescriptorHeap[g_Push.image_index];
    //     SamplerState heapSampler = SamplerDescriptorHeap[0];
    //     float4 sampleColor = heapTexture.Sample(heapSampler, float2(0.5f, 0.5f));
    //     RWStructuredBuffer<SSBO> heapBuffer = ResourceDescriptorHeap[g_Push.buffer_index];
    //     heapBuffer[0].data = sampleColor;
    // }
    char const* cs_source = R"(
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpCapability ComputeDerivativeGroupQuadsKHR
               OpCapability Shader
               OpExtension "SPV_KHR_untyped_pointers"
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_compute_shader_derivatives"
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %slang_resourceHeap %slang_samplerHeap %g_Push
               OpExecutionMode %main DerivativeGroupQuadsKHR
               OpExecutionMode %main LocalSize 2 2 1
               OpDecorate %PushConstants_std430 Block
               OpMemberDecorate %PushConstants_std430 0 Offset 0
               OpMemberDecorate %PushConstants_std430 1 Offset 4
               OpDecorateId %_runtimearr_19 ArrayStrideIdEXT %25
               OpDecorate %slang_resourceHeap BuiltIn ResourceHeapEXT
               OpDecorateId %_runtimearr_33 ArrayStrideIdEXT %34
               OpDecorate %slang_samplerHeap BuiltIn SamplerHeapEXT
               OpDecorateId %_runtimearr_52 ArrayStrideIdEXT %25
               OpMemberDecorate %SSBO_std430 0 Offset 0
               OpDecorate %_runtimearr_SSBO_std430 ArrayStride 16
               OpDecorate %RWStructuredBuffer Block
               OpMemberDecorate %RWStructuredBuffer 0 Offset 0
               OpDecorate %_ptr_StorageBuffer_SSBO_std430 ArrayStride 16
               OpDecorate %_ptr_StorageBuffer_v4float ArrayStride 16
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%PushConstants_std430 = OpTypeStruct %uint %uint
%_ptr_PushConstant_PushConstants_std430 = OpTypePointer PushConstant %PushConstants_std430
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
     %uint_0 = OpConstant %uint 0
      %float = OpTypeFloat 32
         %19 = OpTypeImage %float 2D 2 0 0 1 Unknown
         %20 = OpTypeBufferEXT Uniform
         %21 = OpConstantSizeOfEXT %uint %19
         %22 = OpConstantSizeOfEXT %uint %20
       %bool = OpTypeBool
         %24 = OpSpecConstantOp %bool UGreaterThan %21 %22
         %25 = OpSpecConstantOp %uint Select %24 %21 %22
%_runtimearr_19 = OpTypeRuntimeArray %19
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
         %33 = OpTypeSampler
         %34 = OpConstantSizeOfEXT %uint %33
%_runtimearr_33 = OpTypeRuntimeArray %33
         %39 = OpTypeSampledImage %19
    %v4float = OpTypeVector %float 4
    %v2float = OpTypeVector %float 2
  %float_0_5 = OpConstant %float 0.5
         %44 = OpConstantComposite %v2float %float_0_5 %float_0_5
      %int_0 = OpConstant %int 0
         %52 = OpTypeBufferEXT StorageBuffer
%_runtimearr_52 = OpTypeRuntimeArray %52
%SSBO_std430 = OpTypeStruct %v4float
%_runtimearr_SSBO_std430 = OpTypeRuntimeArray %SSBO_std430
%RWStructuredBuffer = OpTypeStruct %_runtimearr_SSBO_std430
%_ptr_StorageBuffer_RWStructuredBuffer = OpTypePointer StorageBuffer %RWStructuredBuffer
%_ptr_StorageBuffer_SSBO_std430 = OpTypePointer StorageBuffer %SSBO_std430
%_ptr_StorageBuffer_v4float = OpTypePointer StorageBuffer %v4float
     %g_Push = OpVariable %_ptr_PushConstant_PushConstants_std430 PushConstant
%slang_resourceHeap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
%slang_samplerHeap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
       %main = OpFunction %void None %3
          %4 = OpLabel
         %12 = OpAccessChain %_ptr_PushConstant_uint %g_Push %int_1
         %13 = OpLoad %uint %12
         %28 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_19 %slang_resourceHeap %13
         %30 = OpLoad %19 %28
         %36 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_33 %slang_samplerHeap %uint_0
         %38 = OpLoad %33 %36
         %40 = OpSampledImage %39 %30 %38
  %__sampled = OpImageSampleImplicitLod %v4float %40 %44 None
         %48 = OpAccessChain %_ptr_PushConstant_uint %g_Push %int_0
         %49 = OpLoad %uint %48
         %54 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_52 %slang_resourceHeap %49
         %59 = OpBufferPointerEXT %_ptr_StorageBuffer_RWStructuredBuffer %54
         %61 = OpAccessChain %_ptr_StorageBuffer_SSBO_std430 %59 %int_0 %int_0
         %63 = OpAccessChain %_ptr_StorageBuffer_v4float %61 %int_0
               OpStore %63 %__sampled
               OpReturn
               OpFunctionEnd
    )";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2, nullptr, SPV_SOURCE_ASM);

    m_command_buffer.Begin();
    m_command_buffer.PushData(0, 4, &buffer_index);
    m_command_buffer.PushData(4, 4, &image_index);

    m_command_buffer.TransitionLayout(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkClearColorValue color = {{0.2f, 0.4f, 0.6f, 0.8f}};
    VkImageSubresourceRange sub_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u};
    vk::CmdClearColorImage(m_command_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &color, 1u, &sub_range);

    m_command_buffer.TransitionLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    desc_heap.BindResourceHeap(m_command_buffer);
    desc_heap.BindSamplerHeap(m_command_buffer);
    vk::CmdDispatch(m_command_buffer, 1u, 1u, 1u);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        float* data = static_cast<float*>(buffer.Memory().Map());
        for (uint32_t i = 0; i < 4; ++i) {
            ASSERT_EQ(data[i], color.float32[i]);
        }
    }
}

TEST_F(PositiveDescriptorHeapUntyped, StorageImage) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());
    InitRenderTarget();

    const uint32_t buffer_index = 16u;

    const VkDeviceSize image_offset = 0u;
    const VkDeviceSize image_size = heap_props.imageDescriptorSize;
    const VkDeviceSize buffer_offset = heap_props.bufferDescriptorSize * buffer_index;
    const VkDeviceSize buffer_size = heap_props.bufferDescriptorSize;
    const VkDeviceSize resource_heap_app_size = buffer_offset + buffer_size;

    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(resource_heap_app_size);

    vkt::Buffer buffer(*m_device, sizeof(float) * 4u, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_SINT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT);

    VkHostAddressRangeEXT resource_host[2];
    resource_host[0].address = desc_heap.resource_heap_data_ + image_offset;
    resource_host[0].size = static_cast<size_t>(image_size);
    resource_host[1].address = desc_heap.resource_heap_data_ + buffer_offset;
    resource_host[1].size = static_cast<size_t>(buffer_size);

    VkImageViewCreateInfo view_info = image.BasicViewCreatInfo(VK_IMAGE_ASPECT_COLOR_BIT);

    VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
    image_info.pView = &view_info;
    image_info.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDeviceAddressRangeEXT buffer_address_range = buffer.AddressRange();

    VkResourceDescriptorInfoEXT descriptor_info[2];
    descriptor_info[0] = vku::InitStructHelper();
    descriptor_info[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptor_info[0].data.pImage = &image_info;
    descriptor_info[1] = vku::InitStructHelper();
    descriptor_info[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info[1].data.pAddressRange = &buffer_address_range;
    vk::WriteResourceDescriptorsEXT(*m_device, 2u, descriptor_info, resource_host);

    char const* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require
        layout(descriptor_heap, rgba8i) uniform iimage2D heapImages[];
        layout(descriptor_heap) buffer ssbo {
	        ivec4 data;
        } heapBuffer[];
        void main() {
	        heapBuffer[16].data = imageLoad(heapImages[0], ivec2(0));
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
    color.int32[0] = 1;
    color.int32[1] = 2;
    color.int32[2] = 3;
    color.int32[3] = 4;
    vk::CmdClearColorImage(m_command_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &color, 1u,
                           &image_barrier.subresourceRange);

    image_barrier.srcAccessMask = image_barrier.dstAccessMask;
    image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    image_barrier.oldLayout = image_barrier.newLayout;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0u, 0u, nullptr,
                           0u, nullptr, 1u, &image_barrier);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    desc_heap.BindResourceHeap(m_command_buffer);
    vk::CmdDispatch(m_command_buffer, 1u, 1u, 1u);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        int* data = static_cast<int*>(buffer.Memory().Map());
        for (uint32_t i = 0; i < 4; ++i) {
            ASSERT_EQ(data[i], color.int32[i]);
        }
    }
}

TEST_F(PositiveDescriptorHeapUntyped, SecondaryCmdBufferCompute) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());

    vkt::DescriptorHeap desc_heap(*this);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    desc_heap.CreateResourceHeap(resource_stride);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    desc_heap.WriteBufferDescriptorAtOffset(buffer.AddressRange(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);

    char const* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require
        layout(local_size_x = 1) in;
        layout(descriptor_heap) buffer A { uint a; } heap[];
        layout(push_constant) uniform PushConstant {
            uint b;
        };
        void main() {
            heap[0].a = b;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2);

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

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    begin_info.pInheritanceInfo = &inheritance_info;

    secondary.Begin(&begin_info);
    vk::CmdBindPipeline(secondary, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    secondary.PushData(0, sizeof(uint32_t), &src_data);
    vk::CmdDispatch(secondary, 1, 1, 1);
    secondary.End();

    m_command_buffer.Begin();
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);
    vk::CmdExecuteCommands(m_command_buffer, 1, &secondary.handle());
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* data = static_cast<uint32_t*>(buffer.Memory().Map());
        ASSERT_EQ(data[0], src_data);
    }
}

TEST_F(PositiveDescriptorHeapUntyped, SecondaryCmdBufferGraphics) {
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
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vk::CmdExecuteCommands(m_command_buffer, 1, &secondary.handle());
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* data = static_cast<uint32_t*>(buffer.Memory().Map());
        ASSERT_EQ(data[0], src_data);
    }
}

TEST_F(PositiveDescriptorHeapUntyped, HardcodedOffsetIntoStruct) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    if (resource_stride != 64) {
        GTEST_SKIP() << "SPIR-V hardcoded for bufferDescriptorSize of 64";
    }
    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(resource_stride * 4);

    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    uint8_t host_decriptor_a[kMaxSSBO];
    uint8_t host_decriptor_b[kMaxSSBO];
    VkHostAddressRangeEXT descriptor_host = {host_decriptor_a, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = {buffer_a.Address(), 256};
    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;
    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

    device_range.address = buffer_b.Address();
    descriptor_host.address = host_decriptor_b;
    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

    // Set |A| to descriptor index [0, 2, 3] and |B| to [1]
    memcpy(&desc_heap.resource_heap_data_[0], host_decriptor_a, static_cast<size_t>(resource_stride));
    memcpy(&desc_heap.resource_heap_data_[resource_stride], host_decriptor_b, static_cast<size_t>(resource_stride));
    memcpy(&desc_heap.resource_heap_data_[resource_stride * 2], host_decriptor_a, static_cast<size_t>(resource_stride));
    memcpy(&desc_heap.resource_heap_data_[resource_stride * 3], host_decriptor_a, static_cast<size_t>(resource_stride));

    // What the shader looks like
    //
    // layout(descriptor_heap) struct {
    //    layout(offset = 0)   buffer a { uint data; } x;
    //    layout(offset = 128) buffer a { uint data; } y;
    //    layout(offset = 64)  buffer b { uint data; } z;
    //    layout(offset = 192) buffer a { uint data; } w;
    // };
    // layout(push_constant) uniform PushConstant { uint payload; };
    // void main() {
    //     heap.z.data = payload;
    // }
    char const* cs_source = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap %_
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %A Block
               OpMemberDecorate %A 0 Offset 0
               OpDecorate %PushConstant Block
               OpMemberDecorate %PushConstant 0 Offset 0

               ; Use hardcoded offset
               OpMemberDecorate %heap_struct 0 Offset 0
               OpMemberDecorate %heap_struct 1 Offset 128
               OpMemberDecorate %heap_struct 2 Offset 64
               OpMemberDecorate %heap_struct 3 Offset 192

       %void = OpTypeVoid
          %3 = OpTypeFunction %void
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
       %uint = OpTypeInt 32 0
          %A = OpTypeStruct %uint
      %int_0 = OpConstant %int 0
%PushConstant = OpTypeStruct %uint
%_ptr_PushConstant_PushConstant = OpTypePointer PushConstant %PushConstant
          %_ = OpVariable %_ptr_PushConstant_PushConstant PushConstant
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer
         %21 = OpTypeBufferEXT StorageBuffer
%heap_struct = OpTypeStruct %21 %21 %21 %21
       %main = OpFunction %void None %3
          %5 = OpLabel
         %17 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
         %18 = OpLoad %uint %17
         %20 = OpUntypedAccessChainKHR %_ptr_UniformConstant %heap_struct %resource_heap %int_2
         %24 = OpBufferPointerEXT %_ptr_StorageBuffer %20
         %25 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %A %24 %int_0
               OpStore %25 %18
               OpReturn
               OpFunctionEnd

    )";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_3, nullptr, SPV_SOURCE_ASM);

    uint32_t src_data = 4321u;

    VkPushConstantRange push_const_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)};
    VkPipelineLayoutCreateInfo pipeline_layout_info = vku::InitStructHelper();
    pipeline_layout_info.pushConstantRangeCount = 1u;
    pipeline_layout_info.pPushConstantRanges = &push_const_range;
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_info);

    m_command_buffer.Begin();
    vk::CmdPushConstants(m_command_buffer, pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &src_data);
    desc_heap.BindResourceHeap(m_command_buffer);
    m_command_buffer.PushData(0, sizeof(uint32_t), &src_data);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* data = static_cast<uint32_t*>(buffer_b.Memory().Map());
        ASSERT_EQ(data[0], src_data);
    }
}

TEST_F(PositiveDescriptorHeapUntyped, SingleElementNoArray) {
    TEST_DESCRIPTION("GLSL only allows arrays of descriptor, force it to be a single element");
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());

    vkt::DescriptorHeap desc_heap(*this);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    desc_heap.CreateResourceHeap(resource_stride);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    desc_heap.WriteBufferDescriptorAtOffset(buffer.AddressRange(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);

    // GLSL can go
    //    layout(descriptor_heap) buffer A { uint a; } heap[];
    // but this shader looks like
    //
    //     layout(descriptor_heap) buffer A { uint a; } heap;
    //     layout(push_constant) uniform PushConstant { uint b; };
    //     void main() {
    //         heap.a = b;
    //     }
    char const* cs_source = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap %_
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %A Block
               OpMemberDecorate %A 0 Offset 0
               OpDecorate %PushConstant Block
               OpMemberDecorate %PushConstant 0 Offset 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
          %A = OpTypeStruct %uint
%PushConstant = OpTypeStruct %uint
%_ptr_PushConstant_PushConstant = OpTypePointer PushConstant %PushConstant
          %_ = OpVariable %_ptr_PushConstant_PushConstant PushConstant
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer
         %20 = OpTypeBufferEXT StorageBuffer
       %main = OpFunction %void None %3
          %5 = OpLabel
         %16 = OpAccessChain %_ptr_PushConstant_uint %_ %int_0
         %17 = OpLoad %uint %16
         %19 = OpUntypedAccessChainKHR %_ptr_UniformConstant %20 %resource_heap
         %23 = OpBufferPointerEXT %_ptr_StorageBuffer %19
         %24 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %A %23 %int_0
               OpStore %24 %17
               OpReturn
               OpFunctionEnd
    )";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_3, nullptr, SPV_SOURCE_ASM);

    uint32_t src_data = 4321u;

    VkPushConstantRange push_const_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)};
    VkPipelineLayoutCreateInfo pipeline_layout_info = vku::InitStructHelper();
    pipeline_layout_info.pushConstantRangeCount = 1u;
    pipeline_layout_info.pPushConstantRanges = &push_const_range;
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_info);

    m_command_buffer.Begin();
    vk::CmdPushConstants(m_command_buffer, pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &src_data);
    desc_heap.BindResourceHeap(m_command_buffer);
    m_command_buffer.PushData(0, sizeof(uint32_t), &src_data);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* data = static_cast<uint32_t*>(buffer.Memory().Map());
        ASSERT_EQ(data[0], src_data);
    }
}

TEST_F(PositiveDescriptorHeapUntyped, BadArrayStrideWithSingleDescriptor) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());

    vkt::DescriptorHeap desc_heap(*this);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    desc_heap.CreateResourceHeap(resource_stride);

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    desc_heap.WriteBufferDescriptorAtOffset(buffer.AddressRange(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);

    char const* cs_source = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %A Block
               OpMemberDecorate %A 0 Offset 0
               OpMemberDecorate %A 1 Offset 4
               ; Stride is bad, but only using index 0
               OpDecorateId %runtime_buffer_1 ArrayStrideIdEXT %int_1
               OpDecorateId %runtime_buffer_99999 ArrayStrideIdEXT %uint_99999
       %void = OpTypeVoid
    %void_fn = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
    %uint_42 = OpConstant %uint 42
 %uint_99999 = OpConstant %uint 99999
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
          %A = OpTypeStruct %uint %uint
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer
%type_buffer = OpTypeBufferEXT StorageBuffer
%runtime_buffer_1 = OpTypeRuntimeArray %type_buffer
%runtime_buffer_99999 = OpTypeRuntimeArray %type_buffer
       %main = OpFunction %void None %void_fn
          %5 = OpLabel
         %14 = OpUntypedAccessChainKHR %_ptr_UniformConstant %runtime_buffer_1 %resource_heap %int_0
         %18 = OpBufferPointerEXT %_ptr_StorageBuffer %14
         %19 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %A %18 %int_0
               OpStore %19 %uint_42

         %20 = OpUntypedAccessChainKHR %_ptr_UniformConstant %runtime_buffer_99999 %resource_heap %int_0
         %21 = OpBufferPointerEXT %_ptr_StorageBuffer %20
         %22 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %A %21 %int_1
               OpStore %22 %uint_42
               OpReturn
               OpFunctionEnd
    )";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2, nullptr, SPV_SOURCE_ASM);

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* data = static_cast<uint32_t*>(buffer.Memory().Map());
        ASSERT_EQ(data[0], 42);
        ASSERT_EQ(data[1], 42);
    }
}

TEST_F(PositiveDescriptorHeapUntyped, StrideByTwoSpecConstantOp) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());

    vkt::DescriptorHeap desc_heap(*this);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    desc_heap.CreateResourceHeap(resource_stride * 6);

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    desc_heap.WriteBufferDescriptorAtOffset(buffer.AddressRange(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    desc_heap.WriteBufferDescriptorAtOffset(buffer.AddressRange(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, resource_stride * 2);
    desc_heap.WriteBufferDescriptorAtOffset(buffer.AddressRange(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, resource_stride * 4);

    // layout() buffer SSBO {
    //                         uint a;
    //                         uint b;
    //                         uint c;
    // } b[3];
    //
    // where each index is the SAME descriptor
    // the heap looks like
    // [ssbo_0, null, ssbo_1, null, ssbo_2, null]
    char const* cs_source = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpMemberDecorate %SSBO 1 Offset 4
               OpMemberDecorate %SSBO 2 Offset 8
               OpDecorateId %runtime_buffer ArrayStrideIdEXT %stride_2
       %void = OpTypeVoid
    %void_fn = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
    %uint_42 = OpConstant %uint 42
    %uint_43 = OpConstant %uint 43
    %uint_44 = OpConstant %uint 44
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %SSBO = OpTypeStruct %uint %uint %uint
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer

%type_buffer = OpTypeBufferEXT StorageBuffer
   %buf_size = OpConstantSizeOfEXT %int %type_buffer
%stride_2 = OpSpecConstantOp %int IMul %buf_size %int_2

%runtime_buffer = OpTypeRuntimeArray %type_buffer
       %main = OpFunction %void None %void_fn
          %5 = OpLabel

          ;; Lack of index imply zero (%int_0)
    %index_0 = OpUntypedAccessChainKHR %_ptr_UniformConstant %runtime_buffer %resource_heap
  %buf_ptr_0 = OpBufferPointerEXT %_ptr_StorageBuffer %index_0
   %member_0 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %SSBO %buf_ptr_0
               OpStore %member_0 %uint_42

    %index_1 = OpUntypedAccessChainKHR %_ptr_UniformConstant %runtime_buffer %resource_heap %int_1
    %buf_ptr_1 = OpBufferPointerEXT %_ptr_StorageBuffer %index_1
   %member_1 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %SSBO %buf_ptr_1 %int_1
               OpStore %member_1 %uint_43

    %index_2 = OpUntypedAccessChainKHR %_ptr_UniformConstant %runtime_buffer %resource_heap %int_2
    %buf_ptr_2 = OpBufferPointerEXT %_ptr_StorageBuffer %index_2
   %member_2 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %SSBO %buf_ptr_2 %int_2
               OpStore %member_2 %uint_44

               OpReturn
               OpFunctionEnd
    )";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2, nullptr, SPV_SOURCE_ASM);

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* data = static_cast<uint32_t*>(buffer.Memory().Map());
        ASSERT_EQ(data[0], 42);
        ASSERT_EQ(data[1], 43);
        ASSERT_EQ(data[2], 44);
    }
}

TEST_F(PositiveDescriptorHeapUntyped, StaticHeapArraySize) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());

    vkt::DescriptorHeap desc_heap(*this);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    desc_heap.CreateResourceHeap(resource_stride * 6);

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    desc_heap.WriteBufferDescriptorAtOffset(buffer.AddressRange(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    desc_heap.WriteBufferDescriptorAtOffset(buffer.AddressRange(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, resource_stride * 2);
    desc_heap.WriteBufferDescriptorAtOffset(buffer.AddressRange(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, resource_stride * 4);

    // Heap is a buffer[6] and accesses [0], [2], [4]
    // where each index is the SAME descriptor
    char const* cs_source = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpMemberDecorate %SSBO 1 Offset 4
               OpMemberDecorate %SSBO 2 Offset 8
               OpDecorateId %buffer_array_6 ArrayStrideIdEXT %stride_2
       %void = OpTypeVoid
    %void_fn = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
     %uint_6 = OpConstant %uint 6
    %uint_42 = OpConstant %uint 42
    %uint_43 = OpConstant %uint 43
    %uint_44 = OpConstant %uint 44
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %SSBO = OpTypeStruct %uint %uint %uint
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer

%type_buffer = OpTypeBufferEXT StorageBuffer
   %buf_size = OpConstantSizeOfEXT %int %type_buffer
%stride_2 = OpSpecConstantOp %int IAdd %buf_size %buf_size

%buffer_array_6 = OpTypeArray %type_buffer %uint_6
       %main = OpFunction %void None %void_fn
          %5 = OpLabel

    %index_0 = OpUntypedAccessChainKHR %_ptr_UniformConstant %buffer_array_6 %resource_heap %int_0
  %buf_ptr_0 = OpBufferPointerEXT %_ptr_StorageBuffer %index_0
   %member_0 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %SSBO %buf_ptr_0 %int_0
               OpStore %member_0 %uint_42

    %index_1 = OpUntypedAccessChainKHR %_ptr_UniformConstant %buffer_array_6 %resource_heap %int_1
    %buf_ptr_1 = OpBufferPointerEXT %_ptr_StorageBuffer %index_1
   %member_1 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %SSBO %buf_ptr_1 %int_1
               OpStore %member_1 %uint_43

    %index_2 = OpUntypedAccessChainKHR %_ptr_UniformConstant %buffer_array_6 %resource_heap %int_2
    %buf_ptr_2 = OpBufferPointerEXT %_ptr_StorageBuffer %index_2
   %member_2 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %SSBO %buf_ptr_2 %int_2
               OpStore %member_2 %uint_44

               OpReturn
               OpFunctionEnd
    )";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2, nullptr, SPV_SOURCE_ASM);

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* data = static_cast<uint32_t*>(buffer.Memory().Map());
        ASSERT_EQ(data[0], 42);
        ASSERT_EQ(data[1], 43);
        ASSERT_EQ(data[2], 44);
    }
}

TEST_F(PositiveDescriptorHeapUntyped, OffsetId) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());

    vkt::DescriptorHeap desc_heap(*this);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    desc_heap.CreateResourceHeap(resource_stride * 6);

    vkt::Buffer buffer_0(*m_device, 64, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    vkt::Buffer buffer_1(*m_device, 64, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    vkt::Buffer buffer_2(*m_device, 64, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    desc_heap.WriteBufferDescriptorAtOffset(buffer_0.AddressRange(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    desc_heap.WriteBufferDescriptorAtOffset(buffer_1.AddressRange(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, resource_stride);
    desc_heap.WriteBufferDescriptorAtOffset(buffer_2.AddressRange(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, resource_stride * 2);

    // layout(storage_buffer) SSBO {
    //     uint a;
    //     uint b;
    //     uint c;
    // };
    // layout(offset = buffer_size) heap {
    //     SSBO runtime_buffer[];
    // } heap_layout;
    char const* cs_source = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpMemberDecorate %SSBO 1 Offset 4
               OpMemberDecorate %SSBO 2 Offset 8
               OpDecorate %heap_layout Block
               OpMemberDecorateIdEXT %heap_layout 0 OffsetIdEXT %buf_size
               OpDecorateId %runtime_buffer ArrayStrideIdEXT %buf_size
       %void = OpTypeVoid
    %void_fn = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
    %uint_42 = OpConstant %uint 42
    %uint_43 = OpConstant %uint 43
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %SSBO = OpTypeStruct %uint %uint %uint
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer

%type_buffer = OpTypeBufferEXT StorageBuffer
   %buf_size = OpConstantSizeOfEXT %int %type_buffer
%runtime_buffer = OpTypeRuntimeArray %type_buffer

 %heap_layout = OpTypeStruct %runtime_buffer

       %main = OpFunction %void None %void_fn
          %5 = OpLabel

%heap_index_0 = OpUntypedAccessChainKHR %_ptr_UniformConstant %heap_layout %resource_heap %int_0 %int_0
  %buf_ptr_0 = OpBufferPointerEXT %_ptr_StorageBuffer %heap_index_0
   %member_0 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %SSBO %buf_ptr_0 %int_0
               OpStore %member_0 %uint_42

%heap_index_1 = OpUntypedAccessChainKHR %_ptr_UniformConstant %heap_layout %resource_heap %int_0 %int_1
   %buf_ptr_1 = OpBufferPointerEXT %_ptr_StorageBuffer %heap_index_1
    %member_1 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %SSBO %buf_ptr_1 %int_1
                OpStore %member_1 %uint_43

               OpReturn
               OpFunctionEnd
    )";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2, nullptr, SPV_SOURCE_ASM);

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* data = static_cast<uint32_t*>(buffer_1.Memory().Map());
        ASSERT_EQ(data[0], 42);

        data = static_cast<uint32_t*>(buffer_2.Memory().Map());
        ASSERT_EQ(data[1], 43);
    }
}

TEST_F(PositiveDescriptorHeapUntyped, OffsetIdStaticArraySize) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());

    vkt::DescriptorHeap desc_heap(*this);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    desc_heap.CreateResourceHeap(resource_stride * 4);

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    // heap: [null, null, null, buffer]
    desc_heap.WriteBufferDescriptorAtOffset(buffer.AddressRange(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, resource_stride * 3);

    // layout(storage_buffer) SSBO {
    //     uint a;
    //     uint b;
    // };
    // layout(offset = buffer_size) heap {
    //     SSBO buffer_array[3];
    // } heap_layout;
    //
    // heap_layout.buffer_array[2].b = 42;
    char const* cs_source = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpMemberDecorate %SSBO 1 Offset 4
               OpMemberDecorate %SSBO 2 Offset 8
               OpDecorate %heap_layout Block
               OpMemberDecorateIdEXT %heap_layout 0 OffsetIdEXT %buf_size
               OpDecorateId %buffer_array ArrayStrideIdEXT %buf_size
       %void = OpTypeVoid
    %void_fn = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
     %uint_3 = OpConstant %uint 3
    %uint_42 = OpConstant %uint 42
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %SSBO = OpTypeStruct %uint %uint %uint
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer

%type_buffer = OpTypeBufferEXT StorageBuffer
   %buf_size = OpConstantSizeOfEXT %int %type_buffer
%buffer_array = OpTypeArray %type_buffer %uint_3

 %heap_layout = OpTypeStruct %buffer_array

       %main = OpFunction %void None %void_fn
          %5 = OpLabel

%heap_index_0 = OpUntypedAccessChainKHR %_ptr_UniformConstant %heap_layout %resource_heap %int_0 %int_2
  %buf_ptr_0 = OpBufferPointerEXT %_ptr_StorageBuffer %heap_index_0
   %member_1 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %SSBO %buf_ptr_0 %int_1
               OpStore %member_1 %uint_42

               OpReturn
               OpFunctionEnd
    )";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2, nullptr, SPV_SOURCE_ASM);

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* data = static_cast<uint32_t*>(buffer.Memory().Map());
        ASSERT_EQ(data[1], 42);
    }
}

TEST_F(PositiveDescriptorHeapUntyped, MultidimensionalArray) {
    TEST_DESCRIPTION("https://gitlab.khronos.org/Tracker/vk-gl-cts/-/issues/6645");
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());

    vkt::DescriptorHeap desc_heap(*this);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    desc_heap.CreateResourceHeap(resource_stride * 32);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    const VkDeviceAddress offset = ((1 * 3) + 2) * resource_stride;
    desc_heap.WriteBufferDescriptorAtOffset(buffer.AddressRange(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, offset);

    // layout(descriptor_heap) buffer Heap { uint data; } heap[3][3];
    // void main() {
    //     heap[1][2].data = 42;
    // }
    char const* cs_source = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %Heap Block
               OpMemberDecorate %Heap 0 Offset 0
               OpDecorateId %in_array ArrayStrideIdEXT %buf_size
               OpDecorateId %out_array ArrayStrideIdEXT %stride_3
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %uint_0 = OpConstant %uint 0
      %uint_1 = OpConstant %uint 1
      %uint_2 = OpConstant %uint 2
      %uint_3 = OpConstant %uint 3
    %uint_42 = OpConstant %uint 42
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
       %Heap = OpTypeStruct %uint
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer
   %buf_type = OpTypeBufferEXT StorageBuffer
   %buf_size = OpConstantSizeOfEXT %uint %buf_type
   %stride_3 = OpSpecConstantOp %int IMul %buf_size %uint_3
 %in_array = OpTypeArray %buf_type %uint_3
%out_array = OpTypeArray %in_array %uint_3
       %main = OpFunction %void None %3
          %5 = OpLabel
         %15 = OpUntypedAccessChainKHR %_ptr_UniformConstant %out_array %resource_heap %uint_1 %uint_2
         %19 = OpBufferPointerEXT %_ptr_StorageBuffer %15
         %20 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %Heap %19 %uint_0
               OpStore %20 %uint_42
               OpReturn
               OpFunctionEnd
    )";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2, nullptr, SPV_SOURCE_ASM);

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* data = static_cast<uint32_t*>(buffer.Memory().Map());
        ASSERT_EQ(data[0], 42);
    }
}

TEST_F(PositiveDescriptorHeapUntyped, BufferAsFunctionParameter) {
    AddRequiredFeature(vkt::Feature::variablePointers);
    AddRequiredFeature(vkt::Feature::variablePointersStorageBuffer);
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());

    vkt::DescriptorHeap desc_heap(*this);
    desc_heap.CreateResourceHeap(heap_props.bufferDescriptorSize * 4);

    vkt::Buffer buffer_0(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::Buffer buffer_1(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::Buffer buffer_2(*m_device, 32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    desc_heap.WriteBufferDescriptor(buffer_0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    desc_heap.WriteBufferDescriptor(buffer_1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    desc_heap.WriteBufferDescriptor(buffer_2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

    uint32_t* data = static_cast<uint32_t*>(buffer_1.Memory().Map());
    data[0] = 11;
    data = static_cast<uint32_t*>(buffer_2.Memory().Map());
    data[0] = 22;

    // layout(descriptor_heap) buffer SSBO {
    //     uint data;
    // } buf[];
    //
    // layout(push_constant) uniform PC {
    //     uint pc_index;
    // };
    //
    // uint foo(uint index, uint* data_a, uint* data_b) {
    //     if (index == 1) {
    //         return *data_a;
    //     } else if (index < 10) {
    //         return *data_b;
    //     }
    //     return 42;
    // }
    //
    // void main() {
    //     uint x = foo(pc_index, &buf[1], &buf[pc_index]);
    //     buf[0].data = x;
    // }
    char const* cs_source = R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpCapability VariablePointers
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %resource_heap %pc_var
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %PC Block
               OpMemberDecorate %PC 0 Offset 0
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpDecorateId %buffer_stride ArrayStrideIdEXT %buffer_size

       %void = OpTypeVoid
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
    %uint_10 = OpConstant %uint 10
    %uint_42 = OpConstant %uint 42
         %PC = OpTypeStruct %uint
     %ptr_pc = OpTypePointer PushConstant %PC
     %pc_var = OpVariable %ptr_pc PushConstant
       %SSBO = OpTypeStruct %uint
%_ptr_Function_uint = OpTypePointer Function %uint
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer
%type_buffer = OpTypeBufferEXT StorageBuffer
%buffer_size = OpConstantSizeOfEXT %int %type_buffer
%buffer_stride = OpTypeRuntimeArray %type_buffer

  %main_type = OpTypeFunction %void
   %foo_type = OpTypeFunction %uint %_ptr_StorageBuffer %_ptr_StorageBuffer

        %foo = OpFunction %uint None %foo_type
    %param_a = OpFunctionParameter %_ptr_StorageBuffer
    %param_b = OpFunctionParameter %_ptr_StorageBuffer
      %foo_l = OpLabel
         %44 = OpAccessChain %_ptr_PushConstant_uint %pc_var %int_0
   %pc_index = OpLoad %uint %44
         %17 = OpIEqual %bool %pc_index %uint_1
               OpSelectionMerge %19 None
               OpBranchConditional %17 %18 %22
         %18 = OpLabel
         %20 = OpLoad %uint %param_a
               OpReturnValue %20
         %22 = OpLabel
         %25 = OpULessThan %bool %pc_index %uint_10
               OpSelectionMerge %27 None
               OpBranchConditional %25 %26 %27
         %26 = OpLabel
         %28 = OpLoad %uint %param_b
               OpReturnValue %28
         %27 = OpLabel
               OpBranch %19
         %19 = OpLabel
               OpReturnValue %uint_42
               OpFunctionEnd

       %main = OpFunction %void None %main_type
     %main_l = OpLabel
          %x = OpVariable %_ptr_Function_uint Function
         %50 = OpAccessChain %_ptr_PushConstant_uint %pc_var %int_0
  %pc_index_ = OpLoad %uint %50

   %ptr_buf_1 = OpUntypedAccessChainKHR %_ptr_UniformConstant %buffer_stride %resource_heap %int_1
       %buf_a = OpBufferPointerEXT %_ptr_StorageBuffer %ptr_buf_1
  %buf_a_data = OpUntypedAccessChainKHR %_ptr_StorageBuffer %SSBO %buf_a %int_0
  %ptr_buf_pc = OpUntypedAccessChainKHR %_ptr_UniformConstant %buffer_stride %resource_heap %pc_index_
       %buf_b = OpBufferPointerEXT %_ptr_StorageBuffer %ptr_buf_pc
  %buf_b_data = OpUntypedAccessChainKHR %_ptr_StorageBuffer %SSBO %buf_b %int_0

         %66 = OpFunctionCall %uint %foo %buf_a_data %buf_b_data
               OpStore %x %66
         %68 = OpLoad %uint %x
         %69 = OpUntypedAccessChainKHR %_ptr_UniformConstant %buffer_stride %resource_heap %int_0
         %72 = OpBufferPointerEXT %_ptr_StorageBuffer %69
         %73 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %SSBO %72 %int_0
               OpStore %73 %68
               OpReturn
               OpFunctionEnd
    )";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2, nullptr, SPV_SOURCE_ASM);

    data = static_cast<uint32_t*>(buffer_0.Memory().Map());

    // skips loading both
    uint32_t pc_index = 100;
    {
        m_command_buffer.Begin();
        m_command_buffer.PushData(0, sizeof(uint32_t), &pc_index);
        desc_heap.BindResourceHeap(m_command_buffer);
        vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
        vk::CmdDispatch(m_command_buffer, 1, 1, 1);
        m_command_buffer.End();
        m_default_queue->SubmitAndWait(m_command_buffer);

        if (!IsPlatformMockICD()) {
            ASSERT_EQ(data[0], 42);
        }
    }

    // load buffer_1
    pc_index = 1;
    {
        m_command_buffer.Begin();
        m_command_buffer.PushData(0, sizeof(uint32_t), &pc_index);
        desc_heap.BindResourceHeap(m_command_buffer);
        vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
        vk::CmdDispatch(m_command_buffer, 1, 1, 1);
        m_command_buffer.End();
        m_default_queue->SubmitAndWait(m_command_buffer);

        if (!IsPlatformMockICD()) {
            ASSERT_EQ(data[0], 11);
        }
    }

    // load buffer_2
    pc_index = 2;
    {
        m_command_buffer.Begin();
        m_command_buffer.PushData(0, sizeof(uint32_t), &pc_index);
        desc_heap.BindResourceHeap(m_command_buffer);
        vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
        vk::CmdDispatch(m_command_buffer, 1, 1, 1);
        m_command_buffer.End();
        m_default_queue->SubmitAndWait(m_command_buffer);

        if (!IsPlatformMockICD()) {
            ASSERT_EQ(data[0], 22);
        }
    }
}

TEST_F(PositiveDescriptorHeapUntyped, SlangBasicBuffer) {
    RETURN_IF_SKIP(InitUntypedDescriptorHeap());

    vkt::DescriptorHeap desc_heap(*this);
    // Because of -spirv-unified-descriptor-heap-stride
    const VkDeviceSize resource_stride = std::max(heap_props.imageDescriptorSize, heap_props.bufferDescriptorSize);
    desc_heap.CreateResourceHeap(resource_stride * 2);

    vkt::Buffer buffer_0(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    vkt::Buffer buffer_1(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    desc_heap.WriteBufferDescriptorAtOffset(buffer_0.AddressRange(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    desc_heap.WriteBufferDescriptorAtOffset(buffer_1.AddressRange(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, resource_stride);

    // Heap support not yet in any release build
    //   will need -capability spvDescriptorHeapEXT -spirv-unified-descriptor-heap-stride
    // struct PushConstants {
    //     uint index;
    // };
    // [[vk::push_constant]] PushConstants g_Push;
    //
    // [shader("compute")]
    // [numthreads(1, 1, 1)]
    // void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
    //     RWStructuredBuffer<uint> outBuffer = ResourceDescriptorHeap[g_Push.index];
    //     outBuffer[0] = 42 + g_Push.index;
    // }
    char const* cs_source = R"(
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpCapability Shader
               OpExtension "SPV_KHR_untyped_pointers"
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main" %slang_resourceHeap %g_Push
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %PushConstants_std430 Block
               OpMemberDecorate %PushConstants_std430 0 Offset 0
               OpDecorateId %_runtimearr_18 ArrayStrideIdEXT %26
               OpDecorate %slang_resourceHeap BuiltIn ResourceHeapEXT
               OpDecorate %_runtimearr_uint ArrayStride 4
               OpDecorate %RWStructuredBuffer Block
               OpMemberDecorate %RWStructuredBuffer 0 Offset 0
               OpDecorate %_ptr_StorageBuffer_uint ArrayStride 4
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%PushConstants_std430 = OpTypeStruct %uint
%_ptr_PushConstant_PushConstants_std430 = OpTypePointer PushConstant %PushConstants_std430
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_PushConstant_uint = OpTypePointer PushConstant %uint
         %18 = OpTypeBufferEXT StorageBuffer
      %float = OpTypeFloat 32
         %20 = OpTypeImage %float 2D 2 0 0 1 Unknown
         %21 = OpTypeBufferEXT Uniform
         %22 = OpConstantSizeOfEXT %uint %20
         %23 = OpConstantSizeOfEXT %uint %21
       %bool = OpTypeBool
         %25 = OpSpecConstantOp %bool UGreaterThan %22 %23
         %26 = OpSpecConstantOp %uint Select %25 %22 %23
%_runtimearr_18 = OpTypeRuntimeArray %18
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%_runtimearr_uint = OpTypeRuntimeArray %uint
%RWStructuredBuffer = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer_RWStructuredBuffer = OpTypePointer StorageBuffer %RWStructuredBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
    %uint_42 = OpConstant %uint 42
     %g_Push = OpVariable %_ptr_PushConstant_PushConstants_std430 PushConstant
%slang_resourceHeap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
       %main = OpFunction %void None %3
          %4 = OpLabel
         %12 = OpAccessChain %_ptr_PushConstant_uint %g_Push %int_0
         %13 = OpLoad %uint %12
         %29 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_18 %slang_resourceHeap %13
         %34 = OpBufferPointerEXT %_ptr_StorageBuffer_RWStructuredBuffer %29
         %36 = OpAccessChain %_ptr_StorageBuffer_uint %34 %int_0 %int_0
         %37 = OpLoad %uint %12
         %38 = OpIAdd %uint %uint_42 %37
               OpStore %36 %38
               OpReturn
               OpFunctionEnd
    )";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2, nullptr, SPV_SOURCE_ASM);

    m_command_buffer.Begin();
    uint32_t index = 0;
    m_command_buffer.PushData(0, sizeof(uint32_t), &index);
    desc_heap.BindResourceHeap(m_command_buffer);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    index = 1;
    m_command_buffer.PushData(0, sizeof(uint32_t), &index);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* data = static_cast<uint32_t*>(buffer_0.Memory().Map());
        ASSERT_EQ(data[0], 42);
        data = static_cast<uint32_t*>(buffer_1.Memory().Map());
        ASSERT_EQ(data[0], 43);
    }
}