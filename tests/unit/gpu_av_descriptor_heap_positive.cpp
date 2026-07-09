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

#include <gtest/gtest.h>
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include "descriptor_heap_object.h"
#include "layer_validation_tests.h"
#include "pipeline_helper.h"
#include "utils/math_utils.h"
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
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));
    InitRenderTarget();

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    vkt::Buffer buffer(*m_device, sizeof(float) * 4, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    WriteBufferToHeap(buffer);

    char const *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require
        layout(local_size_x = 1) in;
        layout(descriptor_heap) buffer A { vec4 a; } heap[];
        void main() {
            heap[0].a = vec4(0.5f);
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2);

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
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));
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
}

TEST_F(PositiveGpuAVDescriptorHeap, HeapBoundInMultimpleCmdBuffers) {
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));
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
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));
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
    mappings[0] = MakeSetAndBindingMapping(2, 3, 1, VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mappings[0].sourceData.pushAddressOffset = static_cast<uint32_t>(read_offset);
    mappings[1] = MakeSetAndBindingMapping(1, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT);
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

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, descriptor_heap_pipe);

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange.address = heap.Address();
    bind_resource_info.heapRange.size = heap_size;
    bind_resource_info.reservedRangeOffset = descriptor_size;
    bind_resource_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);

    m_command_buffer.PushData(read_offset, sizeof(read_address), &read_address);
    vk::CmdDraw(m_command_buffer, 3u, 1u, 0u, 0u);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVDescriptorHeap, ResourceHeapImage) {
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));
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
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2);

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

TEST_F(PositiveGpuAVDescriptorHeap, CombinedAndSeparateImageSampler) {
    TEST_DESCRIPTION("Just need to test compiler pass with these two sampler usages");
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));
    const VkDeviceSize resource_stride = heap_props.imageDescriptorSize;
    const VkDeviceSize sampler_stride = heap_props.samplerDescriptorSize;

    VkDescriptorSetAndBindingMappingEXT mappings[3];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = (uint32_t)resource_stride;
    mappings[0].sourceData.constantOffset.heapArrayStride = 0;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = (uint32_t)sampler_stride;
    mappings[1].sourceData.constantOffset.heapArrayStride = 0;
    mappings[2] = MakeSetAndBindingMapping(0, 2);
    mappings[2].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[2].sourceData.constantOffset.heapOffset = (uint32_t)resource_stride * 2;
    mappings[2].sourceData.constantOffset.samplerHeapOffset = (uint32_t)sampler_stride * 2;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 3;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform texture2D t;
        layout(set = 0, binding = 1) uniform sampler s;
        layout(set = 0, binding = 2) uniform sampler2D c;

        void main() {
            vec4 a = texture(sampler2D(t, s), vec2(0));
            vec4 b = texture(c, vec2(0.5f));
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);
}

// https://gitlab.khronos.org/vulkan/vulkan/-/issues/4874
TEST_F(PositiveGpuAVDescriptorHeap, DISABLED_PushDataLimitPushIndex) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride * 4);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer, 3);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT;
    mapping.sourceData.pushIndex.heapOffset = 0;
    mapping.sourceData.pushIndex.pushOffset = 248;
    mapping.sourceData.pushIndex.heapIndexStride = (uint32_t)resource_stride;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        void main() {
            a = 42;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    BindResourceHeap();
    uint32_t index = 3;
    m_command_buffer.PushData(248, sizeof(uint32_t), &index);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    uint32_t* ssbo_data = (uint32_t*)ssbo_buffer.Memory().Map();
    ASSERT_TRUE(ssbo_data[0] == 42);
}

// https://gitlab.khronos.org/vulkan/vulkan/-/issues/4874
TEST_F(PositiveGpuAVDescriptorHeap, DISABLED_PushDataLimitIndirectIndex) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride * 4);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer, 3);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT;
    mapping.sourceData.indirectIndex.heapOffset = 0;
    mapping.sourceData.indirectIndex.pushOffset = 248;
    mapping.sourceData.indirectIndex.addressOffset = 4;
    mapping.sourceData.indirectIndex.heapIndexStride = (uint32_t)resource_stride;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        void main() {
            a = 42;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    vkt::Buffer indirect_buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    uint32_t* indirect_data = (uint32_t*)indirect_buffer.Memory().Map();
    indirect_data[1] = 3;
    VkDeviceAddress indirect_address = indirect_buffer.Address();

    m_command_buffer.Begin();
    BindResourceHeap();
    m_command_buffer.PushData(248, sizeof(VkDeviceAddress), &indirect_address);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    uint32_t* ssbo_data = (uint32_t*)ssbo_buffer.Memory().Map();
    ASSERT_TRUE(ssbo_data[0] == 42);
}

// https://gitlab.khronos.org/vulkan/vulkan/-/issues/4874
TEST_F(PositiveGpuAVDescriptorHeap, DISABLED_PushDataLimitHeapData) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride * 4);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer, 3);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = (uint32_t)(resource_stride * 3);
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT;
    mappings[1].sourceData.heapData.heapOffset = 0;
    mappings[1].sourceData.heapData.pushOffset = 248;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        layout(set = 0, binding = 1) uniform B { uint b; };
        void main() {
            a = b;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    const uint32_t min_alignment = (uint32_t)m_device->Physical().limits_.minUniformBufferOffsetAlignment;
    uint32_t* heap_data = (uint32_t*)resource_heap_data_;
    heap_data[min_alignment / 4] = 42;

    m_command_buffer.Begin();
    BindResourceHeap();
    uint32_t index = min_alignment;
    m_command_buffer.PushData(248, sizeof(uint32_t), &index);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    uint32_t* ssbo_data = (uint32_t*)ssbo_buffer.Memory().Map();
    ASSERT_TRUE(ssbo_data[0] == 42);
}

// https://gitlab.khronos.org/vulkan/vulkan/-/issues/4874
TEST_F(PositiveGpuAVDescriptorHeap, DISABLED_PushDataLimitPushData) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::scalarBlockLayout);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = 0;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT;
    mappings[1].sourceData.pushDataOffset = 128;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_scalar_block_layout : enable
        layout(set = 0, binding = 0) buffer A { uint a; };
        layout(set = 0, binding = 1, scalar) uniform B { uint b[32]; };
        void main() {
            a = b[30];
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2, &mapping_info);

    m_command_buffer.Begin();
    BindResourceHeap();
    uint32_t index = 42;
    m_command_buffer.PushData(248, sizeof(uint32_t), &index);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    uint32_t* ssbo_data = (uint32_t*)ssbo_buffer.Memory().Map();
    ASSERT_TRUE(ssbo_data[0] == 42);
}

// https://gitlab.khronos.org/vulkan/vulkan/-/issues/4874
TEST_F(PositiveGpuAVDescriptorHeap, DISABLED_PushDataLimitPushAddress) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));
    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mapping.sourceData.pushAddressOffset = 248;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        void main() {
            a = 42;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    VkDeviceAddress ssbo_address = ssbo_buffer.Address();

    m_command_buffer.Begin();
    m_command_buffer.PushData(248, sizeof(VkDeviceAddress), &ssbo_address);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    uint32_t* ssbo_data = (uint32_t*)ssbo_buffer.Memory().Map();
    ASSERT_TRUE(ssbo_data[0] == 42);
}

// https://gitlab.khronos.org/vulkan/vulkan/-/issues/4874
TEST_F(PositiveGpuAVDescriptorHeap, DISABLED_PushDataLimitIndirectAddress) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));
    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT;
    mapping.sourceData.pushAddressOffset = 248;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        void main() {
            a = 42;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    vkt::Buffer indirect_buffer(*m_device, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    VkDeviceAddress* indirect_data = (VkDeviceAddress*)indirect_buffer.Memory().Map();
    indirect_data[0] = ssbo_buffer.Address();
    VkDeviceAddress indirect_address = indirect_buffer.Address();

    m_command_buffer.Begin();
    m_command_buffer.PushData(248, sizeof(VkDeviceAddress), &indirect_address);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    uint32_t* ssbo_data = (uint32_t*)ssbo_buffer.Memory().Map();
    ASSERT_TRUE(ssbo_data[0] == 42);
}

// https://gitlab.khronos.org/vulkan/vulkan/-/issues/4874
TEST_F(PositiveGpuAVDescriptorHeap, DISABLED_PushDataLimitShader) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(push_constant) uniform PC {
            layout(offset = 128) uint data[32];
        };
        layout(set = 0, binding = 0) buffer A { uint a; };
        void main() {
            a = data[30];
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    BindResourceHeap();
    uint32_t value[32];
    value[30] = 42;
    m_command_buffer.PushData(128, sizeof(value), value);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    uint32_t* ssbo_data = (uint32_t*)ssbo_buffer.Memory().Map();
    ASSERT_TRUE(ssbo_data[0] == 42);
}

// https://gitlab.khronos.org/vulkan/vulkan/-/issues/4874
TEST_F(PositiveGpuAVDescriptorHeap, DISABLED_PushDataLimitShaderAndMapping) {
    AddRequiredFeature(vkt::Feature::shaderInt64);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mapping.sourceData.pushAddressOffset = 128;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
        layout(push_constant) uniform PC {
            u64vec4 data[8];
        };
        layout(set = 0, binding = 0) buffer A { uint64_t a; };
        void main() {
            a = data[4].x;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    VkDeviceAddress ssbo_address = ssbo_buffer.Address();

    m_command_buffer.Begin();
    uint64_t value[32];
    value[16] = ssbo_address;
    m_command_buffer.PushData(0, sizeof(value), &value);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    uint64_t* ssbo_data = (uint64_t*)ssbo_buffer.Memory().Map();
    ASSERT_TRUE(ssbo_data[0] == ssbo_address);
}

// https://gitlab.khronos.org/vulkan/vulkan/-/issues/4874
TEST_F(PositiveGpuAVDescriptorHeap, DISABLED_PushDataLimitMulitipleDispatch) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredFeature(vkt::Feature::scalarBlockLayout);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride * 2);

    vkt::Buffer ssbo1_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::Buffer ssbo2_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo1_buffer);
    WriteBufferToHeap(ssbo2_buffer, 1);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = 0;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT;
    mappings[1].sourceData.pushDataOffset = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_scalar_block_layout : enable
        layout(set = 0, binding = 0) buffer A { uint a; };
        layout(set = 0, binding = 1, scalar) uniform B { uint b[64]; };
        void main() {
            a = b[62];
        }
    )glsl";
    vkt::HeapComputePipeline pipe1(*m_device, cs_source, SPV_ENV_VULKAN_1_2, &mapping_info);

    mappings[0].sourceData.constantOffset.heapOffset = (uint32_t)resource_stride;
    vkt::HeapComputePipeline pipe2(*m_device, cs_source, SPV_ENV_VULKAN_1_2, &mapping_info);

    m_command_buffer.Begin();
    BindResourceHeap();
    uint32_t index = 42;
    m_command_buffer.PushData(248, sizeof(uint32_t), &index);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe1);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);

    index = 88;
    m_command_buffer.PushData(248, sizeof(uint32_t), &index);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe2);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    uint32_t* ssbo_data = (uint32_t*)ssbo1_buffer.Memory().Map();
    ASSERT_TRUE(ssbo_data[0] == 42);
    ssbo_data = (uint32_t*)ssbo2_buffer.Memory().Map();
    ASSERT_TRUE(ssbo_data[0] == 88);
}

TEST_F(PositiveGpuAVDescriptorHeap, PushDataLimitNoShaderInstrumentation) {
    std::vector<VkLayerSettingEXT> layer_settings = {
        {OBJECT_LAYER_NAME, "gpuav_shader_instrumentation", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkFalse},
    };
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap(layer_settings, false));
    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mapping.sourceData.pushAddressOffset = 248;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        void main() {
            a = 42;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    VkDeviceAddress ssbo_address = ssbo_buffer.Address();

    m_command_buffer.Begin();
    m_command_buffer.PushData(248, sizeof(VkDeviceAddress), &ssbo_address);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    uint32_t* ssbo_data = (uint32_t*)ssbo_buffer.Memory().Map();
    ASSERT_TRUE(ssbo_data[0] == 42);
}

TEST_F(PositiveGpuAVDescriptorHeap, SamplerHeapOffset) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));

    const VkDeviceSize resource_stride = heap_props.imageDescriptorSize;
    const VkDeviceSize sampler_stride = heap_props.samplerDescriptorSize;
    const uint32_t image_index = 8u;
    const VkDeviceSize image_offset = resource_stride * image_index;

    CreateResourceHeap(image_offset + resource_stride);
    CreateSamplerHeap(sampler_stride);

    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    WriteImageToHeap(image, image_index);
    vkt::Buffer buffer(*m_device, sizeof(float) * 4u, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    VkHostAddressRangeEXT sampler_host;
    sampler_host.address = sampler_heap_data_;
    sampler_host.size = static_cast<size_t>(sampler_stride);
    vk::WriteSamplerDescriptorsEXT(*m_device, 1u, &sampler_info, &sampler_host);

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform sampler2D tex;
        layout(set = 1, binding = 0) buffer ssbo {
            vec4 data;
        };

        void main() {
            data = texture(tex, vec2(0.5f));
        }
    )glsl";

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = static_cast<uint32_t>(image_offset);
    mappings[1] = MakeSetAndBindingMapping(1, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mappings[1].sourceData.pushAddressOffset = 0u;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2u;
    mapping_info.pMappings = mappings;
    vkt::HeapComputePipeline pipeline(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    const VkDeviceAddress buffer_address = buffer.Address();

    m_command_buffer.Begin();
    m_command_buffer.TransitionLayout(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    BindResourceHeap();
    BindSamplerHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

    m_command_buffer.PushData(0, sizeof(buffer_address), &buffer_address);
    vk::CmdDispatch(m_command_buffer, 1u, 1u, 1u);

    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVDescriptorHeap, GraphicsPipelineLibraryInlined) {
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    AddRequiredFeature(vkt::Feature::fragmentStoresAndAtomics);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));
    InitRenderTarget();
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride * 4);

    VkDescriptorSetAndBindingMappingEXT mappings[3];
    mappings[0] = MakeSetAndBindingMapping(0, 6);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = 0;
    mappings[1] = MakeSetAndBindingMapping(0, 7);  // unused
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = (uint32_t)resource_stride;
    mappings[2] = MakeSetAndBindingMapping(0, 8);
    mappings[2].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[2].sourceData.constantOffset.heapOffset = (uint32_t)resource_stride * 2;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 3u;
    mapping_info.pMappings = mappings;

    char const* vs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 6) uniform UBO {
            vec4 data;
            vec4 data2;
        };
        layout(location = 0) out vec4 outColor;
        void main() {
            gl_Position = data;
            outColor = data2;
        }
    )glsl";

    char const* fs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 8) buffer SSBO {
            vec4 data;
        };
        layout(location = 0) in vec4 inColor;
        void main() {
            data = inColor;
        }
    )glsl";

    VkPipelineCreateFlags2CreateInfoKHR create_flags = vku::InitStructHelper();
    create_flags.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT | VK_PIPELINE_CREATE_2_LIBRARY_BIT_KHR;

    CreatePipelineHelper pre_raster_lib(*this);
    {
        const auto vs_spv = GLSLToSPV(VK_SHADER_STAGE_VERTEX_BIT, vs_source);
        vkt::GraphicsPipelineLibraryStage vs_stage(vs_spv, VK_SHADER_STAGE_VERTEX_BIT, &mapping_info);

        pre_raster_lib.gpl_info.emplace(vku::InitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(&create_flags));
        pre_raster_lib.gpl_info->flags = VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT |
                                         VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT;

        pre_raster_lib.gp_ci_ = vku::InitStructHelper(&pre_raster_lib.gpl_info);
        pre_raster_lib.gp_ci_.pVertexInputState = &pre_raster_lib.vi_ci_;
        pre_raster_lib.gp_ci_.pInputAssemblyState = &pre_raster_lib.ia_ci_;
        pre_raster_lib.gp_ci_.pViewportState = &pre_raster_lib.vp_state_ci_;
        pre_raster_lib.gp_ci_.pRasterizationState = &pre_raster_lib.rs_state_ci_;
        pre_raster_lib.gp_ci_.renderPass = RenderPass();
        pre_raster_lib.gp_ci_.layout = VK_NULL_HANDLE;
        pre_raster_lib.gp_ci_.stageCount = 1;
        pre_raster_lib.gp_ci_.pStages = &vs_stage.stage_ci;

        pre_raster_lib.CreateGraphicsPipeline(false);
    }

    CreatePipelineHelper frag_shader_lib(*this);
    {
        const auto fs_spv = GLSLToSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fs_source);
        vkt::GraphicsPipelineLibraryStage fs_stage(fs_spv, VK_SHADER_STAGE_FRAGMENT_BIT, &mapping_info);

        frag_shader_lib.gpl_info.emplace(vku::InitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(&create_flags));
        frag_shader_lib.gpl_info->flags =
            VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT | VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT;

        frag_shader_lib.gp_ci_ = vku::InitStructHelper(&frag_shader_lib.gpl_info);
        frag_shader_lib.gp_ci_.pViewportState = &frag_shader_lib.vp_state_ci_;
        frag_shader_lib.gp_ci_.pColorBlendState = &frag_shader_lib.cb_ci_;
        frag_shader_lib.gp_ci_.pMultisampleState = &frag_shader_lib.ms_ci_;
        frag_shader_lib.gp_ci_.pRasterizationState = &frag_shader_lib.rs_state_ci_;
        frag_shader_lib.gp_ci_.renderPass = RenderPass();
        frag_shader_lib.gp_ci_.layout = VK_NULL_HANDLE;
        frag_shader_lib.gp_ci_.stageCount = 1;
        frag_shader_lib.gp_ci_.pStages = &fs_stage.stage_ci;

        frag_shader_lib.CreateGraphicsPipeline(false);
    }

    VkPipeline libraries[2] = {
        pre_raster_lib,
        frag_shader_lib,
    };

    create_flags.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper(&create_flags);
    link_info.libraryCount = size32(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = VK_NULL_HANDLE;
    vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());
}

TEST_F(PositiveGpuAVDescriptorHeap, GraphicsPipelineLibraryWithShaderModule) {
    TEST_DESCRIPTION("Found in CTS where using a VkShaderModule with GPL caused the injected mappings to be lost");
    AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::graphicsPipelineLibrary);
    AddRequiredFeature(vkt::Feature::fragmentStoresAndAtomics);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));
    InitRenderTarget();
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride * 4);

    VkDescriptorSetAndBindingMappingEXT mappings[3];
    mappings[0] = MakeSetAndBindingMapping(0, 6);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = 0;
    mappings[1] = MakeSetAndBindingMapping(0, 7);  // unused
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = (uint32_t)resource_stride;
    mappings[2] = MakeSetAndBindingMapping(0, 8);
    mappings[2].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[2].sourceData.constantOffset.heapOffset = (uint32_t)resource_stride * 2;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 3u;
    mapping_info.pMappings = mappings;

    char const* vs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 6) uniform UBO {
            vec4 data;
            vec4 data2;
        };
        layout(location = 0) out vec4 outColor;
        void main() {
            gl_Position = data;
            outColor = data2;
        }
    )glsl";

    char const* fs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 8) buffer SSBO {
            vec4 data;
        };
        layout(location = 0) in vec4 inColor;
        void main() {
            data = inColor;
        }
    )glsl";
    VkShaderObj vs_module(*m_device, vs_source, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs_module(*m_device, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineCreateFlags2CreateInfoKHR create_flags = vku::InitStructHelper();
    create_flags.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT | VK_PIPELINE_CREATE_2_LIBRARY_BIT_KHR;

    CreatePipelineHelper pre_raster_lib(*this);
    {
        VkPipelineShaderStageCreateInfo stage_ci = vku::InitStructHelper(&mapping_info);
        stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
        stage_ci.module = vs_module;
        stage_ci.pName = "main";

        pre_raster_lib.gpl_info.emplace(vku::InitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(&create_flags));
        pre_raster_lib.gpl_info->flags = VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT |
                                         VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT;

        pre_raster_lib.gp_ci_ = vku::InitStructHelper(&pre_raster_lib.gpl_info);
        pre_raster_lib.gp_ci_.pVertexInputState = &pre_raster_lib.vi_ci_;
        pre_raster_lib.gp_ci_.pInputAssemblyState = &pre_raster_lib.ia_ci_;
        pre_raster_lib.gp_ci_.pViewportState = &pre_raster_lib.vp_state_ci_;
        pre_raster_lib.gp_ci_.pRasterizationState = &pre_raster_lib.rs_state_ci_;
        pre_raster_lib.gp_ci_.renderPass = RenderPass();
        pre_raster_lib.gp_ci_.layout = VK_NULL_HANDLE;
        pre_raster_lib.gp_ci_.stageCount = 1;
        pre_raster_lib.gp_ci_.pStages = &stage_ci;

        pre_raster_lib.CreateGraphicsPipeline(false);
    }

    CreatePipelineHelper frag_shader_lib(*this);
    {
        VkPipelineShaderStageCreateInfo stage_ci = vku::InitStructHelper(&mapping_info);
        stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stage_ci.module = fs_module;
        stage_ci.pName = "main";

        frag_shader_lib.gpl_info.emplace(vku::InitStruct<VkGraphicsPipelineLibraryCreateInfoEXT>(&create_flags));
        frag_shader_lib.gpl_info->flags =
            VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT | VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT;

        frag_shader_lib.gp_ci_ = vku::InitStructHelper(&frag_shader_lib.gpl_info);
        frag_shader_lib.gp_ci_.pViewportState = &frag_shader_lib.vp_state_ci_;
        frag_shader_lib.gp_ci_.pColorBlendState = &frag_shader_lib.cb_ci_;
        frag_shader_lib.gp_ci_.pMultisampleState = &frag_shader_lib.ms_ci_;
        frag_shader_lib.gp_ci_.pRasterizationState = &frag_shader_lib.rs_state_ci_;
        frag_shader_lib.gp_ci_.renderPass = RenderPass();
        frag_shader_lib.gp_ci_.layout = VK_NULL_HANDLE;
        frag_shader_lib.gp_ci_.stageCount = 1;
        frag_shader_lib.gp_ci_.pStages = &stage_ci;

        frag_shader_lib.CreateGraphicsPipeline(false);
    }

    VkPipeline libraries[2] = {
        pre_raster_lib,
        frag_shader_lib,
    };

    create_flags.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;
    VkPipelineLibraryCreateInfoKHR link_info = vku::InitStructHelper(&create_flags);
    link_info.libraryCount = size32(libraries);
    link_info.pLibraries = libraries;

    VkGraphicsPipelineCreateInfo exe_pipe_ci = vku::InitStructHelper(&link_info);
    exe_pipe_ci.layout = VK_NULL_HANDLE;
    vkt::Pipeline exe_pipe(*m_device, exe_pipe_ci);
    ASSERT_TRUE(exe_pipe.initialized());
}

TEST_F(PositiveGpuAVDescriptorHeap, UntypedPointersOffsetIdNonArray) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride * 3);

    vkt::Buffer buffer_0(*m_device, 64, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    vkt::Buffer buffer_1(*m_device, 64, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    WriteBufferToHeap(buffer_0, 2);
    WriteBufferToHeap(buffer_1, 1);

    // layout(storage_buffer) SSBO {
    //     uint data;
    // };
    // heap {
    //     layout(offset = buffer_size * 2) SSBO buffer_0;
    //     layout(offset = buffer_size) SSBO buffer_1;
    // } heap_layout;
    //
    // heap_layout.buffer_0.data = 42;
    // heap_layout.buffer_1.data = 43;
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
               OpDecorate %heap_layout Block
               OpMemberDecorateIdEXT %heap_layout 0 OffsetIdEXT %buf_size2
               OpMemberDecorateIdEXT %heap_layout 1 OffsetIdEXT %buf_size
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
        %SSBO = OpTypeStruct %uint
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer

%type_buffer = OpTypeBufferEXT StorageBuffer
   %buf_size = OpConstantSizeOfEXT %int %type_buffer
  %buf_size2 = OpSpecConstantOp %int IMul %buf_size %int_2

 %heap_layout = OpTypeStruct %type_buffer %type_buffer

       %main = OpFunction %void None %void_fn
          %5 = OpLabel

%heap_index_0 = OpUntypedAccessChainKHR %_ptr_UniformConstant %heap_layout %resource_heap
  %buf_ptr_0 = OpBufferPointerEXT %_ptr_StorageBuffer %heap_index_0
   %member_0 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %SSBO %buf_ptr_0 %int_0
               OpStore %member_0 %uint_42

%heap_index_1 = OpUntypedAccessChainKHR %_ptr_UniformConstant %heap_layout %resource_heap %int_1
  %buf_ptr_1 = OpBufferPointerEXT %_ptr_StorageBuffer %heap_index_1
   %member_1 = OpUntypedAccessChainKHR %_ptr_StorageBuffer %SSBO %buf_ptr_1 %int_0
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
}

TEST_F(PositiveGpuAVDescriptorHeap, SecondaryInheritance) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        void main() {
            a = 42;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    VkBindHeapInfoEXT resource_bind_info = vku::InitStructHelper();
    resource_bind_info.heapRange = resource_heap_.AddressRange();
    resource_bind_info.reservedRangeOffset = resource_heap_.CreateInfo().size - heap_props.minResourceHeapReservedRange;
    resource_bind_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;
    VkCommandBufferInheritanceDescriptorHeapInfoEXT inh_desc_heap_info = vku::InitStructHelper();
    inh_desc_heap_info.pResourceHeapBindInfo = &resource_bind_info;
    VkCommandBufferInheritanceInfo inh = vku::InitStructHelper(&inh_desc_heap_info);
    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.pInheritanceInfo = &inh;

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin(&cbbi);
    vk::CmdBindPipeline(secondary, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(secondary, 1u, 1u, 1u);
    secondary.End();

    m_command_buffer.Begin();
    vk::CmdBindResourceHeapEXT(m_command_buffer, &resource_bind_info);
    vk::CmdExecuteCommands(m_command_buffer, 1, &secondary.handle());
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    uint32_t* ssbo_data = (uint32_t*)ssbo_buffer.Memory().Map();
    ASSERT_TRUE(ssbo_data[0] == 42);
}

TEST_F(PositiveGpuAVDescriptorHeap, ImageSamplerDynamicIndex) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));
    const VkDeviceSize image_stride = heap_props.imageDescriptorSize;
    const VkDeviceSize sampler_stride = heap_props.samplerDescriptorSize;
    const VkDeviceSize buffer_offset = AlignResource(image_stride * 2u);

    VkDescriptorSetAndBindingMappingEXT mappings[3];
    mappings[0] = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_SAMPLED_IMAGE_BIT_EXT);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapArrayStride = static_cast<uint32_t>(image_stride);
    mappings[1] = MakeSetAndBindingMapping(1, 0, 1, VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapArrayStride = static_cast<uint32_t>(sampler_stride);
    mappings[2] = MakeSetAndBindingMapping(2, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT);
    mappings[2].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[2].sourceData.constantOffset.heapOffset = static_cast<uint32_t>(buffer_offset);

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 3u;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;

        layout(set = 0, binding = 0) uniform texture2D textureImage[2];
        layout(set = 1, binding = 0) uniform sampler textureSampler[2];
        layout(set = 2, binding = 0) buffer OutBuffer {
            vec4 color;
        };

        layout(push_constant) uniform PushConsts {
            int index;
        } pushConsts;

        void main() {
            color = texture(sampler2D(textureImage[pushConsts.index], textureSampler[pushConsts.index]), vec2(0.5f));
        }
    )glsl";

    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);
}

TEST_F(PositiveGpuAVDescriptorHeap, SecondaryBind) {
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    WriteBufferToHeap(ssbo_buffer);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        void main() {
            a = 42;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    VkBindHeapInfoEXT resource_bind_info = vku::InitStructHelper();
    resource_bind_info.heapRange = resource_heap_.AddressRange();
    resource_bind_info.reservedRangeOffset = resource_heap_.CreateInfo().size - heap_props.minResourceHeapReservedRange;
    resource_bind_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;

    // No VkCommandBufferInheritanceDescriptorHeapInfoEXT
    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    secondary.Begin();
    vk::CmdBindResourceHeapEXT(secondary, &resource_bind_info);
    vk::CmdBindPipeline(secondary, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(secondary, 1u, 1u, 1u);
    secondary.End();

    m_command_buffer.Begin();
    vk::CmdExecuteCommands(m_command_buffer, 1, &secondary.handle());
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    uint32_t* ssbo_data = (uint32_t*)ssbo_buffer.Memory().Map();
    ASSERT_TRUE(ssbo_data[0] == 42);
}

TEST_F(PositiveGpuAVDescriptorHeap, HashingNullDescriptor) {
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nullDescriptor);
    static const VkLayerSettingEXT layer_setting{OBJECT_LAYER_NAME, "descriptor_hashing", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,
                                                 &kVkTrue};
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({layer_setting}, false));
    vkt::DescriptorHeap desc_heap(*this);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    desc_heap.CreateResourceHeap(resource_stride * 2);

    desc_heap.WriteNullDescriptorAtOffset(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    desc_heap.WriteNullDescriptorAtOffset(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, resource_stride);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = 0;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = (uint32_t)resource_stride;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        layout(set = 0, binding = 1) uniform B { uint b; };
        void main() {
            a = b;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVDescriptorHeap, HashingMultiSubmit) {
    static const VkLayerSettingEXT layer_setting{OBJECT_LAYER_NAME, "descriptor_hashing", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,
                                                 &kVkTrue};
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({layer_setting}));
    vkt::DescriptorHeap desc_heap(*this);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    desc_heap.CreateResourceHeap(resource_stride * 8);

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer A { uint a; };
        void main() {
            a = 0;
        }
    )glsl";

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    VkDeviceSize offset_0 = desc_heap.WriteBufferDescriptor(ssbo_buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    VkDeviceSize offset_1 = desc_heap.WriteBufferDescriptor(ssbo_buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = (uint32_t)offset_0;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1;
    mapping_info.pMappings = &mapping;
    vkt::HeapComputePipeline pipe0(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    mapping.sourceData.constantOffset.heapOffset = (uint32_t)offset_1;
    vkt::HeapComputePipeline pipe1(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe0);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe1);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    // submit with no changes
    m_default_queue->SubmitAndWait(m_command_buffer);

    // submit with no dirty info
    desc_heap.WriteBufferDescriptor(ssbo_buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    m_default_queue->SubmitAndWait(m_command_buffer);

    desc_heap.WriteBufferDescriptor(ssbo_buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    desc_heap.WriteBufferDescriptor(ssbo_buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    desc_heap.WriteBufferDescriptor(ssbo_buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    desc_heap.WriteBufferDescriptor(ssbo_buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    VkDeviceSize offset_7 = desc_heap.WriteBufferDescriptor(ssbo_buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    mapping.sourceData.constantOffset.heapOffset = (uint32_t)offset_7;
    vkt::HeapComputePipeline pipe7(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe7);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe1);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVDescriptorHeap, HashingDeviceLocal) {
    const VkLayerSettingEXT layer_setting{OBJECT_LAYER_NAME, "descriptor_hashing", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue};
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({layer_setting}, false));
    vkt::DescriptorHeap desc_heap(*this);

    VkDeviceSize resource_heap_size_app = Align(heap_props.bufferDescriptorSize, heap_props.bufferDescriptorAlignment);
    resource_heap_size_app = Align(resource_heap_size_app, heap_props.imageDescriptorAlignment);
    const VkDeviceSize resource_heap_size_driver = resource_heap_size_app + heap_props.minResourceHeapReservedRange;

    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer descriptor_heap(
        *m_device, resource_heap_size_driver,
        VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocate_flag_info);

    const char* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_debug_printf : enable
        layout (set = 0, binding = 0) buffer SSBO { uint a; };
        void main() {
            a = 0;
        }
    )glsl";

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange.address = descriptor_heap.Address();
    bind_resource_info.heapRange.size = resource_heap_size_driver;
    bind_resource_info.reservedRangeOffset = resource_heap_size_app;
    bind_resource_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;

    vkt::Buffer ssbo_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::Buffer copy_buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, kHostVisibleMemProps);
    VkHostAddressRangeEXT descriptor_host{copy_buffer.Memory().Map(), static_cast<size_t>(heap_props.bufferDescriptorSize)};
    VkDeviceAddressRangeEXT device_range = ssbo_buffer.AddressRange();
    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;
    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

    VkBufferCopy copy_region{0, 0, heap_props.bufferDescriptorSize};

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_resource_info);
    vk::CmdCopyBuffer(m_command_buffer, copy_buffer, descriptor_heap, 1u, &copy_region);
    m_command_buffer.FullMemoryBarrier();
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVDescriptorHeap, HashingTombstone) {
    TEST_DESCRIPTION("Make sure we clear up and use tombstone entries in the hash map correctly");
    const uint32_t limit = 8;
    std::vector<VkLayerSettingEXT> layer_settings = {
        {OBJECT_LAYER_NAME, "descriptor_hashing", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
        {OBJECT_LAYER_NAME, "descriptor_hashing_total_descriptors", VK_LAYER_SETTING_TYPE_UINT32_EXT, 1, &limit}};
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap(layer_settings));
    const uint32_t buffer_size = 131072;
    vkt::Buffer ssbo_buffer_1(*m_device, buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::Buffer ssbo_buffer_2(*m_device, buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    const uint32_t alignment = static_cast<uint32_t>(m_device->Physical().limits_.minStorageBufferOffsetAlignment);
    VkDeviceAddressRangeKHR addr_range{};
    addr_range.size = alignment;
    VkResourceDescriptorInfoEXT desc_info = vku::InitStructHelper();
    desc_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    desc_info.data.pAddressRange = &addr_range;
    uint8_t host_data[256];
    VkHostAddressRangeEXT desc_host_data{host_data, heap_props.bufferDescriptorSize};

    // By moving the address, each descriptor should have a different hash
    VkDeviceAddress ssbo_address = ssbo_buffer_1.Address();
    for (uint32_t i = 0; i < 8; i++) {
        addr_range.address = ssbo_address;
        vk::WriteResourceDescriptorsEXT(*m_device, 1, &desc_info, &desc_host_data);
        ssbo_address += alignment;
    }
    // All 8 slots should be filled with TOMBSTONE now
    ssbo_buffer_1.Destroy();

    // So we should be able to refill without hitting the error limit
    ssbo_address = ssbo_buffer_2.Address();
    for (uint32_t i = 0; i < 8; i++) {
        addr_range.address = ssbo_address;
        vk::WriteResourceDescriptorsEXT(*m_device, 1, &desc_info, &desc_host_data);
        ssbo_address += alignment;
    }
}

TEST_F(PositiveGpuAVDescriptorHeap, Deduplication) {
    TEST_DESCRIPTION("Make sure we deduplicate multiple access when safe mode is off");
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));
    vkt::DescriptorHeap desc_heap(*this);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    desc_heap.CreateResourceHeap(resource_stride * 4);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::Buffer ssbo2_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    VkDeviceSize offset_0 = desc_heap.WriteBufferDescriptor(ssbo_buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    VkDeviceSize offset_1 = desc_heap.WriteBufferDescriptor(ssbo_buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    VkDeviceSize offset_2 = desc_heap.WriteBufferDescriptor(ssbo2_buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    VkDeviceSize offset_3 = desc_heap.WriteBufferDescriptor(ssbo2_buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

    VkDescriptorSetAndBindingMappingEXT mappings[4];
    mappings[0] = MakeSetAndBindingMapping(1, 2);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = (uint32_t)offset_0;
    mappings[1] = MakeSetAndBindingMapping(0, 2);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = (uint32_t)offset_3;
    mappings[2] = MakeSetAndBindingMapping(1, 0);
    mappings[2].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[2].sourceData.constantOffset.heapOffset = (uint32_t)offset_1;
    mappings[3] = MakeSetAndBindingMapping(0, 1);
    mappings[3].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[3].sourceData.constantOffset.heapOffset = (uint32_t)offset_2;
    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 4;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 1) buffer A { uint a; };
        layout(set = 0, binding = 2) buffer B { uint b; };
        layout(set = 1, binding = 0) buffer C { uint c; };
        layout(set = 1, binding = 2) buffer D { uint d; };
        void main() {
            uint x = a;
            a += 3;
            b = 2;
            c = b + a * (a + d);
            d = b - a;
            if (d == 0) {
                c = 0;
                b = 0;
            }
            a = 0;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_0, &mapping_info);

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVDescriptorHeap, DeduplicationUntyped) {
    TEST_DESCRIPTION("Make sure we deduplicate multiple access when safe mode is off");
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitGpuAVDescriptorHeap({}, false));
    vkt::DescriptorHeap desc_heap(*this);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    desc_heap.CreateResourceHeap(resource_stride * 4);

    vkt::Buffer ssbo_buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    desc_heap.WriteBufferDescriptor(ssbo_buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    desc_heap.WriteBufferDescriptor(ssbo_buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    desc_heap.WriteBufferDescriptor(ssbo_buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    desc_heap.WriteBufferDescriptor(ssbo_buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

    char const* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require
        layout(descriptor_heap) buffer A { uint x; } heap[];
        void main() {
            uint x = heap[0].x;
            heap[0].x += 3;
            heap[1].x = 2;
            heap[2].x = heap[1].x + heap[0].x * (heap[0].x + heap[3].x);
            heap[3].x = heap[1].x - heap[0].x;
            if (heap[3].x == 0) {
                heap[2].x = 0;
                heap[1].x = 0;
            }
            heap[0].x = 0;
        }
    )glsl";
    vkt::HeapComputePipeline pipe(*m_device, cs_source, SPV_ENV_VULKAN_1_2, nullptr);

    m_command_buffer.Begin();
    desc_heap.BindResourceHeap(m_command_buffer);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}