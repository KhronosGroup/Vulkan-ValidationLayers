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
#include "test_framework.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

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

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = buffer.AddressRange();
    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;

    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

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
    BindResourceHeap();
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

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = {buffer.Address(), 256};

    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;

    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;
    mapping.sourceData.constantOffset.heapArrayStride = 0;

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
    BindResourceHeap();
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

    m_command_buffer.TransitionLayout(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkClearColorValue color = {{0.2f, 0.4f, 0.6f, 0.8f}};
    VkImageSubresourceRange sub_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u};
    vk::CmdClearColorImage(m_command_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &color, 1u, &sub_range);

    m_command_buffer.TransitionLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    BindResourceHeap();
    BindSamplerHeap();
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

    CreateResourceHeap(resource_heap_app_size);

    vkt::Buffer buffer(*m_device, sizeof(float) * 4u, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_SINT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT);

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
    BindResourceHeap();
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

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = buffer.AddressRange();

    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;

    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

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
    bind_resource_info.heapRange = resource_heap_.AddressRange();
    bind_resource_info.reservedRangeOffset = resource_heap_.CreateInfo().size - heap_props.minResourceHeapReservedRange;
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
    CreateResourceHeap(resource_stride * 4);

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
    memcpy(&resource_heap_data_[0], host_decriptor_a, static_cast<size_t>(resource_stride));
    memcpy(&resource_heap_data_[resource_stride], host_decriptor_b, static_cast<size_t>(resource_stride));
    memcpy(&resource_heap_data_[resource_stride * 2], host_decriptor_a, static_cast<size_t>(resource_stride));
    memcpy(&resource_heap_data_[resource_stride * 3], host_decriptor_a, static_cast<size_t>(resource_stride));

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
    BindResourceHeap();
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

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = buffer.AddressRange();
    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;
    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

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
    BindResourceHeap();
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

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = buffer.AddressRange();
    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;
    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

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
    BindResourceHeap();
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

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride * 6);

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = buffer.AddressRange();
    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;
    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);
    descriptor_host.address = resource_heap_data_ + (resource_stride * 2);
    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);
    descriptor_host.address = resource_heap_data_ + (resource_stride * 4);
    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

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
    BindResourceHeap();
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

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride * 6);

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = buffer.AddressRange();
    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;
    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);
    descriptor_host.address = resource_heap_data_ + (resource_stride * 2);
    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);
    descriptor_host.address = resource_heap_data_ + (resource_stride * 4);
    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

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
    BindResourceHeap();
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

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride * 6);

    vkt::Buffer buffer_0(*m_device, 64, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    vkt::Buffer buffer_1(*m_device, 64, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    vkt::Buffer buffer_2(*m_device, 64, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = buffer_0.AddressRange();
    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;
    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

    device_range = buffer_1.AddressRange();
    descriptor_host.address = resource_heap_data_ + (resource_stride * 1);
    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

    device_range = buffer_2.AddressRange();
    descriptor_host.address = resource_heap_data_ + (resource_stride * 2);
    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

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
    BindResourceHeap();
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

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride * 4);

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    // heap: [null, null, null, buffer]
    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_ + (resource_stride * 3), static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = buffer.AddressRange();
    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;
    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

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
    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* data = static_cast<uint32_t*>(buffer.Memory().Map());
        ASSERT_EQ(data[1], 42);
    }
}