/*
 * Copyright (c) 2025-2026 The Khronos Group Inc.
 * Copyright (c) 2025-2026 Valve Corporation
 * Copyright (c) 2025-2026 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../utils/math_utils.h"
#include "test_framework.h"

class PositiveGpuAVDescriptorHeap : public GpuAVDescriptorHeap {};

void GpuAVDescriptorHeap::InitGpuAVDescriptorHeap(std::vector<VkLayerSettingEXT> layer_settings, bool safe_mode) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::descriptorHeap);
    RETURN_IF_SKIP(InitGpuAvFramework(layer_settings, safe_mode));
    RETURN_IF_SKIP(InitState());
    GetPhysicalDeviceProperties2(heap_props);
}

TEST_F(PositiveGpuAVDescriptorHeap, BufferPointerOffset) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    InitRenderTarget();

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    vkt::Buffer buffer(*m_device, sizeof(float) * 4, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkHostAddressRangeEXT descriptor_host;
    descriptor_host.address = resource_heap_data_;
    descriptor_host.size = resource_stride;

    VkDeviceAddressRangeEXT device_range = buffer.AddressRange();

    VkResourceDescriptorInfoEXT descriptor_info;
    descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;
    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require
        layout(local_size_x = 1) in;
        layout(descriptor_heap) buffer A { vec4 a; } heap[];
        void main() {
            heap[0].a = vec4(0.5f);
        }
    )glsl";

    VkShaderObj cs_module(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.CreateComputePipeline(false);

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    BindResourceHeap();
    vk::CmdDispatch(m_command_buffer, 1u, 1u, 1u);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVDescriptorHeap, SamplerPointerOffset) {
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    InitRenderTarget();

    const uint32_t buffer_index = 16u;

    const VkDeviceSize image_offset = 0u;
    const VkDeviceSize image_size = heap_props.imageDescriptorSize;
    const VkDeviceSize buffer_offset = heap_props.bufferDescriptorSize * buffer_index;
    const VkDeviceSize buffer_size = heap_props.bufferDescriptorSize;
    const VkDeviceSize resource_heap_app_size = buffer_offset + buffer_size;

    CreateResourceHeap(resource_heap_app_size);

    vkt::Buffer buffer(*m_device, sizeof(float) * 4u, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    VkHostAddressRangeEXT resource_host[2];
    resource_host[0].address = resource_heap_data_ + image_offset;
    resource_host[0].size = image_size;
    resource_host[1].address = resource_heap_data_ + buffer_offset;
    resource_host[1].size = buffer_size;

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

    const VkDeviceSize sampler_offset = 0u;
    const VkDeviceSize sampler_size = heap_props.samplerDescriptorSize;

    CreateSamplerHeap(sampler_size);

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();

    VkHostAddressRangeEXT sampler_host;
    sampler_host.address = sampler_heap_data_ + sampler_offset;
    sampler_host.size = sampler_size;
    vk::WriteSamplerDescriptorsEXT(*m_device, 1u, &sampler_info, &sampler_host);

    char const *cs_source = R"glsl(
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
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.CreateComputePipeline(false);

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
    BindSamplerHeap();
    vk::CmdDispatch(m_command_buffer, 1u, 1u, 1u);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVDescriptorHeap, HeapBoundInMultimpleCmdBuffers) {
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    InitRenderTarget();

    const VkDeviceSize heap_size = 256;
    CreateResourceHeap(heap_size);

    vkt::CommandBuffer cmd_buffer(*m_device, m_command_pool);

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange.address = resource_heap_.Address();
    bind_resource_info.heapRange.size = resource_heap_.CreateInfo().size;
    bind_resource_info.reservedRangeOffset = resource_heap_.CreateInfo().size - heap_props.minResourceHeapReservedRange;
    bind_resource_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;

    m_command_buffer.Begin();
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);
    m_command_buffer.End();

    cmd_buffer.Begin();
    vk::CmdBindResourceHeapEXT(cmd_buffer, &bind_resource_info);
    cmd_buffer.End();
}

TEST_F(PositiveGpuAVDescriptorHeap, PushAddress) {
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
    InitRenderTarget();

    vkt::Buffer read_buffer(*m_device, sizeof(uint32_t) * 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::Buffer write_buffer(*m_device, sizeof(uint32_t) * 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    const uint32_t read_offset = 48u;
    const VkDeviceSize write_offset =
        Align(read_offset + heap_props.bufferDescriptorSize * 7u, heap_props.bufferDescriptorSize);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    const VkDeviceSize descriptor_size = AlignResource(write_offset + resource_stride);
    const VkDeviceSize heap_size = descriptor_size + heap_props.minResourceHeapReservedRange;

    vkt::Buffer heap(*m_device, heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);
    uint8_t *heap_data = static_cast<uint8_t *>(heap.Memory().Map());

    VkHostAddressRangeEXT descriptor_host;
    descriptor_host.address = heap_data + write_offset;
    descriptor_host.size = resource_stride;

    VkDeviceAddressRangeEXT device_range = write_buffer.AddressRange();

    VkResourceDescriptorInfoEXT descriptor_info;
    descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;

    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = vku::InitStructHelper();
    mappings[0].descriptorSet = 2u;
    mappings[0].firstBinding = 3u;
    mappings[0].bindingCount = 1u;
    mappings[0].resourceMask = VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT;
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mappings[0].sourceData.pushAddressOffset = static_cast<uint32_t>(read_offset);
    mappings[1] = vku::InitStructHelper();
    mappings[1].descriptorSet = 1u;
    mappings[1].firstBinding = 0u;
    mappings[1].bindingCount = 1u;
    mappings[1].resourceMask = VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT;
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = static_cast<uint32_t>(write_offset);
    mappings[1].sourceData.constantOffset.heapArrayStride = 0u;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2u;
    mapping_info.pMappings = mappings;

    char const *vert_source = R"glsl(
        #version 450
        layout(set = 2, binding = 3) buffer ReadData {
            uvec4 read_data;
        };
        layout(set = 1, binding = 0) buffer WriteData {
            uvec4 write_data;
        };
        void main() {
            if (gl_VertexIndex == 0) {
                write_data[0] = read_data[3];
            }
            gl_Position = vec4(1.0f);
            gl_PointSize = 1.0f;
        }
    )glsl";

    VkShaderObj vert_module = VkShaderObj(*m_device, vert_source, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj frag_module = VkShaderObj(*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkPipelineShaderStageCreateInfo stages[2];
    stages[0] = vert_module.GetStageCreateInfo(&mapping_info);
    stages[1] = frag_module.GetStageCreateInfo();

    CreatePipelineHelper descriptor_heap_pipe(*this, &pipeline_create_flags_2_create_info);
    descriptor_heap_pipe.gp_ci_.layout = VK_NULL_HANDLE;
    descriptor_heap_pipe.gp_ci_.stageCount = 2u;
    descriptor_heap_pipe.gp_ci_.pStages = stages;
    descriptor_heap_pipe.CreateGraphicsPipeline(false);

    VkDeviceAddress read_address = read_buffer.Address();

    VkPushDataInfoEXT push_data = vku::InitStructHelper();
    push_data.offset = read_offset;
    push_data.data.address = &read_address;
    push_data.data.size = sizeof(read_address);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, descriptor_heap_pipe);

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange.address = heap.Address();
    bind_resource_info.heapRange.size = heap_size;
    bind_resource_info.reservedRangeOffset = descriptor_size;
    bind_resource_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    vk::CmdPushDataEXT(m_command_buffer, &push_data);
    vk::CmdDraw(m_command_buffer, 3u, 1u, 0u, 0u);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVDescriptorHeap, ResourceHeapImage) {
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap());
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

    VkImageCreateInfo image_ci = image.CreateInfo();
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.format = VK_FORMAT_R8G8B8A8_SINT;
    image_ci.extent = {32u, 32u, 1u};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    vkt::Image image_3d(*m_device, image_ci);

    VkHostAddressRangeEXT resource_host[2];
    resource_host[0].address = resource_heap_data_ + image_offset;
    resource_host[0].size = image_size;
    resource_host[1].address = resource_heap_data_ + buffer_offset;
    resource_host[1].size = buffer_size;

    VkImageViewCreateInfo view_info = image.BasicViewCreatInfo(VK_IMAGE_ASPECT_COLOR_BIT);

    VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
    image_info.pView = &view_info;
    image_info.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkDeviceAddressRangeEXT buffer_address_range = buffer.AddressRange();

    VkResourceDescriptorInfoEXT descriptor_info[2];
    descriptor_info[0] = vku::InitStructHelper();
    descriptor_info[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptor_info[0].data.pImage = &image_info;
    descriptor_info[1] = vku::InitStructHelper();
    descriptor_info[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info[1].data.pAddressRange = &buffer_address_range;
    vk::WriteResourceDescriptorsEXT(*m_device, 2u, descriptor_info, resource_host);

    char const *cs_source = R"glsl(
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
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.CreateComputePipeline(false);

    m_command_buffer.Begin();

    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcAccessMask = image_barrier.dstAccessMask;
    image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    image_barrier.oldLayout = image_barrier.newLayout;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.image = image;
    image_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u};
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0u, 0u, nullptr,
                           0u, nullptr, 1u, &image_barrier);

    image_barrier.image = image_3d;
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0u, 0u, nullptr,
                           0u, nullptr, 1u, &image_barrier);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    BindResourceHeap();
    vk::CmdDispatch(m_command_buffer, 1u, 1u, 1u);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVDescriptorHeap, ForceNullDescriptor) {
    TEST_DESCRIPTION(
        "Never set anything in the heap, turn on gpuav_force_on_robustness, then pray hard that GPU-AV will prevent crashing");
    std::vector<VkLayerSettingEXT> layer_settings = {
        {OBJECT_LAYER_NAME, "gpuav_force_on_robustness", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
    };
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap(layer_settings, false));
    uint32_t random_size = 16384;
    CreateResourceHeap(random_size * 4);
    CreateSamplerHeap(random_size * 2);

    VkBufferUsageFlags2CreateInfo buffer_usage = vku::InitStructHelper();
    buffer_usage.usage = VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper(&buffer_usage);
    buffer_ci.size = random_size + heap_props.minResourceHeapReservedRange + heap_props.minSamplerHeapReservedRange;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    // Using as both the resource and sampler... because why not
    vkt::Buffer descriptor_heap_device(*m_device, buffer_ci, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocate_flag_info);

    // Few extra allocation to stress our internal way to call vkCmdFillBuffer
    {
        buffer_ci.size = 1024;
        vkt::Buffer dummy_buffer_1(*m_device, buffer_ci, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocate_flag_info);
        vkt::Buffer dummy_buffer_2(*m_device, buffer_ci, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocate_flag_info);
    }

    VkDescriptorSetAndBindingMappingEXT mappings[4];
    mappings[0] = MakeSetAndBindingMapping(0, 1);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = (uint32_t)heap_props.bufferDescriptorSize;
    mappings[1] = MakeSetAndBindingMapping(0, 2);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT;
    mappings[1].sourceData.pushIndex.heapOffset = (uint32_t)heap_props.imageDescriptorSize * 2;
    mappings[1].sourceData.pushIndex.heapIndexStride = 1;
    mappings[1].sourceData.pushIndex.pushOffset = 8;
    mappings[2] = MakeSetAndBindingMapping(0, 0);
    mappings[2].source = VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT;
    mappings[2].sourceData.heapData.heapOffset = (uint32_t)heap_props.bufferDescriptorSize * 4;
    mappings[2].sourceData.heapData.pushOffset = 8;
    mappings[3] = MakeSetAndBindingMapping(0, 3);
    mappings[3].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[3].sourceData.constantOffset.heapOffset = (uint32_t)heap_props.samplerDescriptorSize * 3;
    mappings[3].sourceData.constantOffset.heapArrayStride = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 4;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) uniform A { vec4 a; };
        layout(set = 0, binding = 1) buffer B { vec4 b; };
        layout(set = 0, binding = 2) uniform texture2D t;
        layout(set = 0, binding = 3) uniform sampler s;
        void main() {
            b = texture(sampler2D(t, s), vec2(0.5f)) * a;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo(&mapping_info);
    pipe.CreateComputePipeline(false);

    m_command_buffer.Begin();
    BindResourceHeap();
    BindSamplerHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    uint32_t push_offset = 0;
    VkPushDataInfoEXT push_data = vku::InitStructHelper();
    push_data.offset = 8;
    push_data.data.address = &push_offset;
    push_data.data.size = sizeof(uint32_t);
    vk::CmdPushDataEXT(m_command_buffer, &push_data);

    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = descriptor_heap_device.AddressRange();
    bind_resource_info.reservedRangeOffset = random_size;
    bind_resource_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    bind_resource_info.reservedRangeOffset = random_size + heap_props.minResourceHeapReservedRange;
    bind_resource_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange;
    vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_resource_info);

    vk::CmdDispatch(m_command_buffer, 8, 8, 2);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}