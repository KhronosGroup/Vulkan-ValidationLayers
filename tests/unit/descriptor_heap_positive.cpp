/*
 * Copyright (C) 2025-2026 Advanced Micro Devices, Inc. All rights reserved.
 * Copyright (c) 2025-2026 LunarG, Inc.
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
#include "utils/math_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

constexpr uint32_t kMaxSSBO = 128;  // max bufferDescriptorSize is allowed to be

class PositiveDescriptorHeap : public DescriptorHeapTest {};

TEST_F(PositiveDescriptorHeap, Basic) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride * 2);

    vkt::Buffer buffer_a(*m_device, 512, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkHostAddressRangeEXT descriptor_host[2];
    descriptor_host[0].address = resource_heap_data_;
    descriptor_host[0].size = static_cast<size_t>(resource_stride);
    descriptor_host[1].address = resource_heap_data_ + resource_stride;
    descriptor_host[1].size = static_cast<size_t>(resource_stride);
    VkDeviceAddressRangeEXT device_ranges[2];
    device_ranges[0].address = buffer_a.Address() + 256;
    device_ranges[0].size = 256;
    device_ranges[1].address = buffer_b.Address();
    device_ranges[1].size = 256;
    VkResourceDescriptorInfoEXT descriptor_info[2];
    descriptor_info[0] = vku::InitStructHelper();
    descriptor_info[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info[0].data.pAddressRange = &device_ranges[0];
    descriptor_info[1] = vku::InitStructHelper();
    descriptor_info[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info[1].data.pAddressRange = &device_ranges[1];

    vk::WriteResourceDescriptorsEXT(*m_device, 2, descriptor_info, descriptor_host);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = 0;
    mappings[0].sourceData.constantOffset.heapArrayStride = 0;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = static_cast<uint32_t>(resource_stride);
    mappings[1].sourceData.constantOffset.heapArrayStride = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer A { uint a; };
        layout(set = 0, binding = 1) buffer B { uint b; };
        void main() {
            a = 2;
            b = 4;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    pipe.CreateComputePipeline(false);

    m_command_buffer.Begin();
    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveDescriptorHeap, NormalUsageWithFeature) {
    TEST_DESCRIPTION("Ensure just enabling descriptor heap doesn't have false positive with a normal workflow");
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    const char* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo {
            uvec4 a[8];
            vec4 result;
        } Foo[2];
        layout(set = 0, binding = 1) uniform sampler2D tex;

        void main() {
            Foo[0].result = texture(tex, vec2(0, 0));
            Foo[1].a[3].y = 44;
        }
    )glsl";

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    vkt::Image image(*m_device, 16, 16, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkt::ImageView image_view = image.CreateView();
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, sampler);
    descriptor_set.UpdateDescriptorSets();

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveDescriptorHeap, ComputeBuffer) {
    TEST_DESCRIPTION("Basic descriptor heap test with compute pipeline and storage buffer");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const uint32_t expected_value = 0x42424242;
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    const VkDeviceSize data_buffer_size = 256;

    CreateResourceHeap(resource_stride * 2);

    // Output buffer descriptor
    vkt::Buffer out_buffer(*m_device, data_buffer_size, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkHostAddressRangeEXT out_descriptor{resource_heap_data_, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT out_address_range = {out_buffer.Address(), data_buffer_size};
    VkResourceDescriptorInfoEXT out_descriptor_info = vku::InitStructHelper();
    out_descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    out_descriptor_info.data.pAddressRange = &out_address_range;

    vk::WriteResourceDescriptorsEXT(*m_device, 1, &out_descriptor_info, &out_descriptor);

    // Input buffer descriptor
    vkt::Buffer in_buffer(*m_device, data_buffer_size, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkHostAddressRangeEXT in_descriptor{resource_heap_data_ + resource_stride, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT in_address_range = {in_buffer.Address(), data_buffer_size};
    VkResourceDescriptorInfoEXT in_descriptor_info = vku::InitStructHelper();
    in_descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    in_descriptor_info.data.pAddressRange = &in_address_range;

    vk::WriteResourceDescriptorsEXT(*m_device, 1, &in_descriptor_info, &in_descriptor);

    uint32_t* in_buffer_ptr = (uint32_t*)in_buffer.Memory().Map();
    in_buffer_ptr[0] = expected_value;

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = 0u * (uint32_t)resource_stride;
    mappings[0].sourceData.constantOffset.heapArrayStride = 0;

    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = 1u * (uint32_t)resource_stride;
    mappings[1].sourceData.constantOffset.heapArrayStride = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer Output { int result[]; };
        layout(set = 0, binding = 1) buffer Input0 { int data[]; };
        void main() {
            result[gl_LocalInvocationID.x] = data[gl_LocalInvocationID.x];
        }
    )glsl";

    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    pipe.CreateComputePipeline(false);

    m_command_buffer.Begin();
    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveDescriptorHeap, GraphicsPushData) {
    TEST_DESCRIPTION("Basic descriptor heap test with graphics pipeline and push data");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    InitRenderTarget();

    const uint32_t expected_value = 3;
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;

    CreateResourceHeap(resource_stride * 4);
    // Test needs sizeof(uint32_t), but AS require align to 256 bytes
    const VkDeviceSize data_buffer_size = 256;

    VkPushDataInfoEXT push_data_info = vku::InitStructHelper();
    push_data_info.offset = 0;
    push_data_info.data.size = sizeof(expected_value);
    push_data_info.data.address = &expected_value;

    vkt::Buffer out_buffer(*m_device, data_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    // Input buffer descriptor
    vkt::Buffer in_buffer(*m_device, 256, VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT_KHR, vkt::device_address);

    VkHostAddressRangeEXT in_descriptor{resource_heap_data_ + resource_stride, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT input_address_range = {in_buffer.Address(), data_buffer_size};
    VkResourceDescriptorInfoEXT in_descriptor_info = vku::InitStructHelper();
    in_descriptor_info.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    in_descriptor_info.data.pAddressRange = &input_address_range;

    vk::WriteResourceDescriptorsEXT(*m_device, 1, &in_descriptor_info, &in_descriptor);

    uint32_t* in_buffer_ptr = (uint32_t*)in_buffer.Memory().Map();
    in_buffer_ptr[0] = expected_value;

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 20);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0u;
    mapping.sourceData.constantOffset.heapArrayStride = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mappingInfo = vku::InitStructHelper();
    mappingInfo.mappingCount = 1;
    mappingInfo.pMappings = &mapping;

    char const* vert_source = R"glsl(
        #version 450
        void main() {
            gl_Position = vec4(vec3(0),1);
        }
    )glsl";

    char const* frag_source = R"glsl(
        #version 450
        layout(location = 0) out vec4 result;
        layout(set = 0, binding = 20) uniform UniformBuffer0 { int data; };
        void main() {
            result = vec4(0.66);
        }
    )glsl";

    VkShaderObj vert_module = VkShaderObj(*m_device, vert_source, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj frag_module = VkShaderObj(*m_device, frag_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkPipelineShaderStageCreateInfo stages[2] = {vert_module.GetStageCreateInfo(), frag_module.GetStageCreateInfo()};
    stages[0].pNext = &mappingInfo;
    stages[1].pNext = &mappingInfo;

    CreatePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.gp_ci_.layout = VK_NULL_HANDLE;
    pipe.gp_ci_.stageCount = 2;
    pipe.gp_ci_.pStages = stages;
    pipe.CreateGraphicsPipeline(false);

    vkt::Buffer vertex_buffer(*m_device, 12, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    const VkDeviceSize offset = 0;

    m_command_buffer.Begin();
    vk::CmdPushDataEXT(m_command_buffer, &push_data_info);
    BindResourceHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindVertexBuffers(m_command_buffer, 0, 1, &vertex_buffer.handle(), &offset);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();

    VkImageMemoryBarrier image_memory_barrier = vku::InitStructHelper();
    image_memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.image = m_renderTargets[0]->handle();
    image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_memory_barrier.subresourceRange.baseMipLevel = 0u;
    image_memory_barrier.subresourceRange.levelCount = 1u;
    image_memory_barrier.subresourceRange.baseArrayLayer = 0u;
    image_memory_barrier.subresourceRange.layerCount = 1u;
    vk::CmdPipelineBarrier(m_command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0u, 0u,
                           nullptr, 0u, nullptr, 1u, &image_memory_barrier);

    VkBufferImageCopy copy_region = {};
    copy_region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.imageOffset.x = 1;
    copy_region.imageOffset.y = 1;
    copy_region.imageExtent = {1, 1, 1};

    vk::CmdCopyImageToBuffer(m_command_buffer, m_renderTargets[0]->handle(), VK_IMAGE_LAYOUT_GENERAL, out_buffer, 1u, &copy_region);

    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveDescriptorHeap, ResourceParameterDataNull) {
    TEST_DESCRIPTION("Validate vkWriteResourceDescriptorsEXT null pointer when nullDescriptor enabled");
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::nullDescriptor);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    std::vector<VkDescriptorType> types{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                        VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
                                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
    for (auto type : types) {
        const VkDeviceSize size = vk::GetPhysicalDeviceDescriptorSizeEXT(Gpu(), type);
        auto data = std::make_unique<uint8_t[]>(static_cast<size_t>(size));

        VkResourceDescriptorInfoEXT resource_desc_info = vku::InitStructHelper();
        resource_desc_info.type = type;
        VkHostAddressRangeEXT descriptors = {data.get(), static_cast<size_t>(size)};

        // We do not expect error messages due to nullDescriptor enables pTexelBuffer, pAddressRange, pImage to be null
        vk::WriteResourceDescriptorsEXT(device(), 1u, &resource_desc_info, &descriptors);
    }
}

TEST_F(PositiveDescriptorHeap, ResetCommandBufferOverlappingResource) {
    TEST_DESCRIPTION("Validate reservedRangeOffset improperly bind to resource heap");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    if (heap_props.minResourceHeapReservedRange == 0) {
        GTEST_SKIP() << "Test requires minResourceHeapReservedRange != 0";
    }

    const VkDeviceSize resource_stride = std::max(heap_props.resourceHeapAlignment, heap_props.imageDescriptorAlignment);
    CreateResourceHeap(resource_stride * 4);

    vkt::CommandPool command_pool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer cmd_buffer1(*m_device, command_pool);
    vkt::CommandBuffer cmd_buffer2(*m_device, command_pool);

    VkBindHeapInfoEXT bind_info1 = vku::InitStructHelper();
    bind_info1.heapRange.address = resource_heap_.Address();
    bind_info1.heapRange.size = resource_heap_.CreateInfo().size;
    bind_info1.reservedRangeOffset = 0;
    bind_info1.reservedRangeSize = heap_props.minResourceHeapReservedRange;

    VkBindHeapInfoEXT bind_info2 = vku::InitStructHelper();
    bind_info2.heapRange.address = resource_heap_.Address();
    bind_info2.heapRange.size = resource_heap_.CreateInfo().size - resource_stride;
    bind_info2.reservedRangeOffset = resource_stride;
    bind_info2.reservedRangeSize = heap_props.minResourceHeapReservedRange;

    cmd_buffer1.Begin();
    vk::CmdBindResourceHeapEXT(cmd_buffer1, &bind_info1);
    cmd_buffer1.End();
    vk::ResetCommandBuffer(cmd_buffer1, 0u);
    cmd_buffer2.Begin();
    vk::CmdBindResourceHeapEXT(cmd_buffer2, &bind_info2);
    cmd_buffer2.End();
}

TEST_F(PositiveDescriptorHeap, ResetCommandBufferTypeChange) {
    TEST_DESCRIPTION(
        "Validate that command buffer reset also resets descriptor heap binding as a sampler descriptor heap and buffer can be "
        "bound as a resource descriptor heap");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    if (heap_props.minResourceHeapReservedRange == 0) {
        GTEST_SKIP() << "Test requires minResourceHeapReservedRange != 0";
    }
    if (heap_props.minSamplerHeapReservedRange == 0) {
        GTEST_SKIP() << "Test requires minSamplerHeapReservedRange != 0";
    }

    CreateResourceHeap(std::max(heap_props.samplerDescriptorSize, heap_props.bufferDescriptorSize));

    vkt::CommandPool command_pool(*m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer cmd_buffer1(*m_device, command_pool);

    VkBindHeapInfoEXT bind_info_sampler = vku::InitStructHelper();
    bind_info_sampler.heapRange.address = resource_heap_.Address();
    bind_info_sampler.heapRange.size = heap_props.samplerDescriptorSize + heap_props.minSamplerHeapReservedRange;
    bind_info_sampler.reservedRangeOffset = 0;
    bind_info_sampler.reservedRangeSize = heap_props.minSamplerHeapReservedRange;

    VkBindHeapInfoEXT bind_info_resource = vku::InitStructHelper();
    bind_info_resource.heapRange.address = resource_heap_.Address();
    bind_info_resource.heapRange.size = heap_props.bufferDescriptorSize + heap_props.minResourceHeapReservedRange;
    bind_info_resource.reservedRangeOffset = 0;
    bind_info_resource.reservedRangeSize = heap_props.minResourceHeapReservedRange;

    cmd_buffer1.Begin();
    vk::CmdBindSamplerHeapEXT(cmd_buffer1, &bind_info_sampler);
    cmd_buffer1.End();
    vk::ResetCommandBuffer(cmd_buffer1, 0u);
    // Buffer is now can be bound as a resource buffer, due to it is not bound as a sampler buffer
    cmd_buffer1.Begin();
    vk::CmdBindResourceHeapEXT(cmd_buffer1, &bind_info_resource);
    cmd_buffer1.End();
}

TEST_F(PositiveDescriptorHeap, PushData) {
    TEST_DESCRIPTION("Descriptor heap with VkPushDataInfoEXT, but vkCmdPushConstants() is called before and invalidated later");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

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

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer A { uint a; };
        layout(push_constant) uniform PushConstant {
          uint b;
        };
        void main() {
            a = b;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    pipe.CreateComputePipeline(false);

    uint32_t src_data = 4321u;

    VkPushDataInfoEXT push_data_info = vku::InitStructHelper();
    push_data_info.offset = 0u;
    push_data_info.data.size = sizeof(uint32_t);
    push_data_info.data.address = &src_data;

    VkPushConstantRange push_const_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)};
    VkPipelineLayoutCreateInfo pipeline_layout_info = vku::InitStructHelper();
    pipeline_layout_info.pushConstantRangeCount = 1u;
    pipeline_layout_info.pPushConstantRanges = &push_const_range;
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_info);

    m_command_buffer.Begin();
    vk::CmdPushConstants(m_command_buffer, pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &src_data);
    BindResourceHeap();
    vk::CmdPushDataEXT(m_command_buffer, &push_data_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* data = static_cast<uint32_t*>(buffer.Memory().Map());
        ASSERT_EQ(data[0], src_data);
    }
}

TEST_F(PositiveDescriptorHeap, MixedDraws) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, sizeof(uint32_t) * 4, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range{buffer.Address(), buffer.CreateInfo().size};

    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;

    vk::WriteResourceDescriptorsEXT(*m_device, 1, &descriptor_info, &descriptor_host);

    VkDescriptorSetAndBindingMappingEXT mapping =
        MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mapping.sourceData.constantOffset.heapOffset = 0u;
    mapping.sourceData.constantOffset.heapArrayStride = 0u;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                                 });

    descriptor_set.WriteDescriptorBufferInfo(0u, buffer, 0u, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    char const* vert_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer Data {
            uvec4 data;
        };
        void main() {
            data[gl_VertexIndex] = 1u;
            gl_Position = vec4(1.0f);
            gl_PointSize = 1.0f;
        }
    )glsl";

    VkShaderObj vert_module = VkShaderObj(*m_device, vert_source, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj frag_module = VkShaderObj(*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper regular_pipe(*this);
    regular_pipe.shader_stages_[0] = vert_module.GetStageCreateInfo();
    regular_pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    regular_pipe.gp_ci_.layout = pipeline_layout;
    regular_pipe.CreateGraphicsPipeline();

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkPipelineShaderStageCreateInfo stages[2];
    stages[0] = vert_module.GetStageCreateInfo();
    stages[0].pNext = &mapping_info;
    stages[1] = frag_module.GetStageCreateInfo();

    CreatePipelineHelper descriptor_heap_pipe(*this, &pipeline_create_flags_2_create_info);
    descriptor_heap_pipe.ia_ci_.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    descriptor_heap_pipe.gp_ci_.layout = VK_NULL_HANDLE;
    descriptor_heap_pipe.gp_ci_.stageCount = 2u;
    descriptor_heap_pipe.gp_ci_.pStages = stages;
    descriptor_heap_pipe.CreateGraphicsPipeline(false);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, regular_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0u, 1u, &descriptor_set.set_, 0u,
                              nullptr);
    vk::CmdDraw(m_command_buffer, 1u, 1u, 0u, 0u);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, descriptor_heap_pipe);
    BindResourceHeap();
    vk::CmdDraw(m_command_buffer, 1u, 1u, 1u, 0u);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, regular_pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0u, 1u, &descriptor_set.set_, 0u,
                              nullptr);
    vk::CmdDraw(m_command_buffer, 1u, 1u, 2u, 0u);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* data = static_cast<uint32_t*>(buffer.Memory().Map());
        for (uint32_t i = 0; i < 3; ++i) {
            ASSERT_EQ(data[i], 1u);
        }
    }
}

TEST_F(PositiveDescriptorHeap, SamplerInheritance) {
    TEST_DESCRIPTION("Validate that inherited ranges match primary buffer");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    CreateSamplerHeap(heap_props.samplerDescriptorSize * 2);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
    bind_info.heapRange = {sampler_heap_.Address(), sampler_heap_.CreateInfo().size};
    bind_info.reservedRangeOffset = 0;
    bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange;

    VkCommandBufferInheritanceDescriptorHeapInfoEXT inh_desc_heap_info = vku::InitStructHelper();
    inh_desc_heap_info.pSamplerHeapBindInfo = &bind_info;

    VkCommandBufferInheritanceInfo inh = vku::InitStructHelper(&inh_desc_heap_info);
    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.pInheritanceInfo = &inh;

    secondary.Begin(&cbbi);
    secondary.End();

    m_command_buffer.Begin();
    vk::CmdBindSamplerHeapEXT(m_command_buffer, &bind_info);
    vk::CmdExecuteCommands(m_command_buffer, 1, &secondary.handle());
    m_command_buffer.End();
}

TEST_F(PositiveDescriptorHeap, ResourceInheritance) {
    TEST_DESCRIPTION("Validate that inherited ranges match primary buffer");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    CreateResourceHeap(heap_props.bufferDescriptorSize * 2);

    VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
    bind_info.heapRange = {resource_heap_.Address(), resource_heap_.CreateInfo().size};
    bind_info.reservedRangeOffset = 0;
    bind_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferInheritanceDescriptorHeapInfoEXT inh_desc_heap_info = vku::InitStructHelper();
    inh_desc_heap_info.pResourceHeapBindInfo = &bind_info;

    VkCommandBufferInheritanceInfo inh = vku::InitStructHelper(&inh_desc_heap_info);
    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.pInheritanceInfo = &inh;

    secondary.Begin(&cbbi);
    secondary.End();

    m_command_buffer.Begin();

    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_info);
    vk::CmdExecuteCommands(m_command_buffer, 1, &secondary.handle());

    m_command_buffer.End();
}

TEST_F(PositiveDescriptorHeap, Sampler) {
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    InitRenderTarget();

    VkDeviceSize resource_heap_tracker = 0u;
    const VkDeviceSize image_offset = AlignedAppend(resource_heap_tracker, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    const VkDeviceSize image_size = resource_heap_tracker - image_offset;
    const VkDeviceSize buffer_offset = AlignedAppend(resource_heap_tracker, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    const VkDeviceSize buffer_size = resource_heap_tracker - buffer_offset;
    const VkDeviceSize resource_heap_size = resource_heap_tracker;

    CreateResourceHeap(resource_heap_size);

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

    VkDeviceAddressRangeEXT buffer_address_range = {buffer.Address(), buffer.CreateInfo().size};

    VkResourceDescriptorInfoEXT descriptor_info[2];
    descriptor_info[0] = vku::InitStructHelper();
    descriptor_info[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptor_info[0].data.pImage = &image_info;
    descriptor_info[1] = vku::InitStructHelper();
    descriptor_info[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info[1].data.pAddressRange = &buffer_address_range;
    vk::WriteResourceDescriptorsEXT(*m_device, 2u, descriptor_info, resource_host);

    VkDeviceSize sampler_desc_heap_size_tracker =
        Align(heap_props.minSamplerHeapReservedRange, heap_props.samplerDescriptorAlignment);
    const VkDeviceSize sampler_offset = AlignedAppend(sampler_desc_heap_size_tracker, VK_DESCRIPTOR_TYPE_SAMPLER);
    const VkDeviceSize sampler_size = sampler_desc_heap_size_tracker - sampler_offset;
    const VkDeviceSize sampler_heap_size = sampler_desc_heap_size_tracker;

    vkt::Buffer sampler_heap(*m_device, sampler_heap_size, VK_BUFFER_USAGE_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);
    uint8_t* sampler_heap_data = static_cast<uint8_t*>(sampler_heap.Memory().Map());

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();

    VkHostAddressRangeEXT sampler_host = {sampler_heap_data + sampler_offset, static_cast<size_t>(sampler_size)};
    vk::WriteSamplerDescriptorsEXT(*m_device, 1u, &sampler_info, &sampler_host);

    VkBindHeapInfoEXT sampler_bind_info = vku::InitStructHelper();
    sampler_bind_info.heapRange = {sampler_heap.Address(), sampler_heap_size};
    sampler_bind_info.reservedRangeOffset = 0;
    sampler_bind_info.reservedRangeSize = heap_props.minSamplerHeapReservedRange;

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform texture2D tex;
        layout(set = 0, binding = 1) uniform sampler sampl;
        layout(set = 1, binding = 0) buffer ssbo {
            vec4 data;
        };
        void main() {
            data = texture(sampler2D(tex, sampl), vec2(0.5f));
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkDescriptorSetAndBindingMappingEXT mappings[3];
    mappings[0] = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_SAMPLED_IMAGE_BIT_EXT);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset = {};
    mappings[0].sourceData.constantOffset.heapOffset = static_cast<uint32_t>(image_offset);
    mappings[1] = MakeSetAndBindingMapping(0, 1, 1, VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset = {};
    mappings[1].sourceData.constantOffset.heapOffset = static_cast<uint32_t>(sampler_offset);
    mappings[2] = MakeSetAndBindingMapping(1, 0);
    mappings[2].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[2].sourceData.constantOffset = {};
    mappings[2].sourceData.constantOffset.heapOffset = static_cast<uint32_t>(buffer_offset);

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 3u;
    mapping_info.pMappings = mappings;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
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
    vk::CmdBindSamplerHeapEXT(m_command_buffer, &sampler_bind_info);
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

TEST_F(PositiveDescriptorHeap, CombinedImageSampler) {
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    InitRenderTarget();

    const VkDeviceSize image_offset = 0u;
    const VkDeviceSize buffer_offset = Align(heap_props.imageDescriptorSize, heap_props.resourceHeapAlignment);
    CreateResourceHeap(buffer_offset + heap_props.bufferDescriptorSize);
    CreateSamplerHeap(heap_props.samplerDescriptorAlignment);

    vkt::Buffer buffer(*m_device, sizeof(float) * 4u, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    VkHostAddressRangeEXT resource_host[2];
    resource_host[0].address = resource_heap_data_ + image_offset;
    resource_host[0].size = static_cast<size_t>(heap_props.imageDescriptorSize);
    resource_host[1].address = resource_heap_data_ + buffer_offset;
    resource_host[1].size = static_cast<size_t>(heap_props.bufferDescriptorSize);

    VkImageViewCreateInfo view_info = image.BasicViewCreatInfo(VK_IMAGE_ASPECT_COLOR_BIT);

    VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
    image_info.pView = &view_info;
    image_info.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDeviceAddressRangeEXT buffer_address_range = {buffer.Address(), buffer.CreateInfo().size};

    VkResourceDescriptorInfoEXT descriptor_info[2];
    descriptor_info[0] = vku::InitStructHelper();
    descriptor_info[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptor_info[0].data.pImage = &image_info;
    descriptor_info[1] = vku::InitStructHelper();
    descriptor_info[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info[1].data.pAddressRange = &buffer_address_range;
    vk::WriteResourceDescriptorsEXT(*m_device, 2u, descriptor_info, resource_host);

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();

    VkHostAddressRangeEXT sampler_host = {sampler_heap_data_, static_cast<size_t>(heap_props.samplerDescriptorSize)};
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
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_COMBINED_SAMPLED_IMAGE_BIT_EXT);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset = {};
    mappings[1] = MakeSetAndBindingMapping(1, 0);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset = {};
    mappings[1].sourceData.constantOffset.heapOffset = static_cast<uint32_t>(buffer_offset);

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2u;
    mapping_info.pMappings = mappings;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
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

    if (!IsPlatformMockICD()) {
        float* data = static_cast<float*>(buffer.Memory().Map());
        for (uint32_t i = 0; i < 4; ++i) {
            ASSERT_EQ(data[i], color.float32[i]);
        }
    }
}

TEST_F(PositiveDescriptorHeap, EmbeddedSampler) {
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    InitRenderTarget();

    const VkDeviceSize image_offset = 0u;
    const VkDeviceSize buffer_offset = Align(heap_props.imageDescriptorSize, heap_props.resourceHeapAlignment);
    CreateResourceHeap(buffer_offset + heap_props.bufferDescriptorSize);
    CreateSamplerHeap(heap_props.samplerDescriptorAlignment, true);

    vkt::Buffer buffer(*m_device, sizeof(float) * 4u, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    VkHostAddressRangeEXT resource_host[2];
    resource_host[0].address = resource_heap_data_ + image_offset;
    resource_host[0].size = static_cast<size_t>(heap_props.imageDescriptorSize);
    resource_host[1].address = resource_heap_data_ + buffer_offset;
    resource_host[1].size = static_cast<size_t>(heap_props.bufferDescriptorSize);

    VkImageViewCreateInfo view_info = image.BasicViewCreatInfo(VK_IMAGE_ASPECT_COLOR_BIT);

    VkImageDescriptorInfoEXT image_info = vku::InitStructHelper();
    image_info.pView = &view_info;
    image_info.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDeviceAddressRangeEXT buffer_address_range = {buffer.Address(), buffer.CreateInfo().size};

    VkResourceDescriptorInfoEXT descriptor_info[2];
    descriptor_info[0] = vku::InitStructHelper();
    descriptor_info[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptor_info[0].data.pImage = &image_info;
    descriptor_info[1] = vku::InitStructHelper();
    descriptor_info[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info[1].data.pAddressRange = &buffer_address_range;
    vk::WriteResourceDescriptorsEXT(*m_device, 2u, descriptor_info, resource_host);

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();

    char const* cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform texture2D tex;
        layout(set = 0, binding = 1) uniform sampler sampl;
        layout(set = 1, binding = 0) buffer ssbo {
            vec4 data;
        };
        void main() {
            data = texture(sampler2D(tex, sampl), vec2(0.5f));
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkDescriptorSetAndBindingMappingEXT mappings[3];
    mappings[0] = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_SAMPLED_IMAGE_BIT_EXT);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset = {};
    mappings[1] = MakeSetAndBindingMapping(0, 1, 1, VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset = {};
    mappings[1].sourceData.constantOffset.pEmbeddedSampler = &sampler_info;
    mappings[2] = MakeSetAndBindingMapping(1, 0);
    mappings[2].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[2].sourceData.constantOffset = {};
    mappings[2].sourceData.constantOffset.heapOffset = static_cast<uint32_t>(buffer_offset);

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 3u;
    mapping_info.pMappings = mappings;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
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

    if (!IsPlatformMockICD()) {
        float* data = static_cast<float*>(buffer.Memory().Map());
        for (uint32_t i = 0; i < 4; ++i) {
            ASSERT_EQ(data[i], color.float32[i]);
        }
    }
}

TEST_F(PositiveDescriptorHeap, EmbeddedSamplerNoBoundHeap) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/11558");
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    // Resource descriptor heap buffer
    const VkDeviceSize resource_stride = Align(heap_props.bufferDescriptorSize, heap_props.samplerDescriptorAlignment);
    const VkDeviceSize resource_descriptor_count = 4;
    const VkDeviceSize resource_heap_size_app = AlignResource(resource_descriptor_count * resource_stride);
    const VkDeviceSize resource_heap_size = resource_heap_size_app + heap_props.minResourceHeapReservedRange;
    const VkDeviceSize out_data_buffer_size = 256;

    vkt::Buffer resource_heap(*m_device, resource_heap_size, VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT, vkt::device_address);
    const auto resource_heap_ptr = static_cast<char*>(resource_heap.Memory().Map());

    // Output buffer descriptor
    vkt::Buffer out_buffer(*m_device, out_data_buffer_size, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkHostAddressRangeEXT out_descriptor{resource_heap_ptr, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT out_address_range = {out_buffer.Address(), out_data_buffer_size};
    VkResourceDescriptorInfoEXT out_descriptor_info = vku::InitStructHelper();
    out_descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    out_descriptor_info.data.pAddressRange = &out_address_range;
    vk::WriteResourceDescriptorsEXT(*m_device, 1, &out_descriptor_info, &out_descriptor);

    VkDescriptorSetAndBindingMappingEXT mappings[3];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = 0u * (uint32_t)resource_stride;
    mappings[0].sourceData.constantOffset.heapArrayStride = 0;

    VkSamplerCreateInfo embedded_sampler = SafeSaneSamplerCreateInfo();
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = 1u * (uint32_t)resource_stride;
    mappings[1].sourceData.constantOffset.heapArrayStride = 0;
    mappings[1].sourceData.constantOffset.pEmbeddedSampler = &embedded_sampler;

    mappings[2] = MakeSetAndBindingMapping(0, 2);
    mappings[2].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[2].sourceData.constantOffset.heapOffset = 2u * (uint32_t)resource_stride;
    mappings[2].sourceData.constantOffset.heapArrayStride = 0;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 3;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer Output { int result[]; };
        layout(set = 0, binding = 1) uniform sampler samp;
        layout(set = 0, binding = 2) uniform texture2D tex;
        void main() {
            ivec2 uv = ivec2(gl_GlobalInvocationID.xy);
            vec2 texCoord = vec2(gl_GlobalInvocationID.xy) / 1.0;
            vec4 color = texture(sampler2D(tex, samp), texCoord);
            result[gl_LocalInvocationIndex] = int(color.r);
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineCreateFlags2CreateInfo pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    pipe.CreateComputePipeline(false);

    // Don't need to call vkCmdBindSamplerHeapEXT if only using embedded
    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveDescriptorHeap, MappingSourceHeapWithPushIndex) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, sizeof(uint32_t) * 4, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    const uint32_t push_data_offset = 8u;
    const uint32_t push_offset = 4u;
    const uint32_t heap_offset = 3u;
    const VkDeviceSize offset = heap_props.bufferDescriptorAlignment * (push_offset + heap_offset);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(offset + resource_stride);

    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_ + offset, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = {buffer.Address(), buffer.CreateInfo().size};

    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;

    vk::WriteResourceDescriptorsEXT(*m_device, 1, &descriptor_info, &descriptor_host);

    VkDescriptorSetAndBindingMappingEXT mapping =
        MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT;
    mapping.sourceData.pushIndex = {};
    mapping.sourceData.pushIndex.heapOffset = heap_offset * static_cast<uint32_t>(heap_props.bufferDescriptorAlignment);
    mapping.sourceData.pushIndex.pushOffset = push_data_offset;
    mapping.sourceData.pushIndex.heapIndexStride = static_cast<uint32_t>(heap_props.bufferDescriptorAlignment);

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    char const* vert_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer Data {
            uvec4 data;
        };
        void main() {
            data[gl_VertexIndex] = 1u;
            gl_Position = vec4(1.0f);
            gl_PointSize = 1.0f;
        }
    )glsl";

    VkShaderObj vert_module = VkShaderObj(*m_device, vert_source, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj frag_module = VkShaderObj(*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkPipelineShaderStageCreateInfo stages[2];
    stages[0] = vert_module.GetStageCreateInfo();
    stages[0].pNext = &mapping_info;
    stages[1] = frag_module.GetStageCreateInfo();

    CreatePipelineHelper descriptor_heap_pipe(*this, &pipeline_create_flags_2_create_info);
    descriptor_heap_pipe.gp_ci_.layout = VK_NULL_HANDLE;
    descriptor_heap_pipe.gp_ci_.stageCount = 2u;
    descriptor_heap_pipe.gp_ci_.pStages = stages;
    descriptor_heap_pipe.CreateGraphicsPipeline(false);

    VkPushDataInfoEXT push_data = vku::InitStructHelper();
    push_data.offset = push_data_offset;
    push_data.data.address = &push_offset;
    push_data.data.size = sizeof(uint32_t);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, descriptor_heap_pipe);
    BindResourceHeap();
    vk::CmdPushDataEXT(m_command_buffer, &push_data);
    vk::CmdDraw(m_command_buffer, 3u, 1u, 0u, 0u);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* data = static_cast<uint32_t*>(buffer.Memory().Map());
        for (uint32_t i = 0; i < 3; ++i) {
            ASSERT_EQ(data[i], 1u);
        }
    }
}

TEST_F(PositiveDescriptorHeap, MappingSourceHeapWithIndirectIndex) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    InitRenderTarget();

    const VkDeviceSize offset = heap_props.bufferDescriptorAlignment * 7u;
    const uint32_t push_offset = 8u;
    const uint32_t address_offset = 16u;
    vkt::Buffer heap_index(*m_device, sizeof(uint32_t) + address_offset, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR,
                           vkt::device_address);
    uint32_t* heap_index_data = static_cast<uint32_t*>(heap_index.Memory().Map());
    heap_index_data[address_offset / sizeof(uint32_t)] = static_cast<uint32_t>(offset / heap_props.bufferDescriptorAlignment);

    vkt::Buffer buffer(*m_device, sizeof(uint32_t) * 4, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(offset + resource_stride);

    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_ + offset, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = {buffer.Address(), buffer.CreateInfo().size};

    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;

    vk::WriteResourceDescriptorsEXT(*m_device, 1, &descriptor_info, &descriptor_host);

    VkDescriptorSetAndBindingMappingEXT mapping =
        MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT;
    mapping.sourceData.indirectIndex = {};
    mapping.sourceData.indirectIndex.heapOffset = 0u;
    mapping.sourceData.indirectIndex.pushOffset = push_offset;
    mapping.sourceData.indirectIndex.addressOffset = address_offset;
    mapping.sourceData.indirectIndex.heapIndexStride = static_cast<uint32_t>(heap_props.bufferDescriptorAlignment);
    mapping.sourceData.indirectIndex.heapArrayStride = static_cast<uint32_t>(heap_props.bufferDescriptorAlignment);

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    char const* vert_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer Data {
            uvec4 data;
        };
        void main() {
            data[gl_VertexIndex] = 1u;
            gl_Position = vec4(1.0f);
            gl_PointSize = 1.0f;
        }
    )glsl";

    VkShaderObj vert_module = VkShaderObj(*m_device, vert_source, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj frag_module = VkShaderObj(*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkPipelineShaderStageCreateInfo stages[2];
    stages[0] = vert_module.GetStageCreateInfo();
    stages[0].pNext = &mapping_info;
    stages[1] = frag_module.GetStageCreateInfo();

    CreatePipelineHelper descriptor_heap_pipe(*this, &pipeline_create_flags_2_create_info);
    descriptor_heap_pipe.gp_ci_.layout = VK_NULL_HANDLE;
    descriptor_heap_pipe.gp_ci_.stageCount = 2u;
    descriptor_heap_pipe.gp_ci_.pStages = stages;
    descriptor_heap_pipe.CreateGraphicsPipeline(false);

    VkDeviceAddress heap_index_address = heap_index.Address();

    VkPushDataInfoEXT push_data = vku::InitStructHelper();
    push_data.offset = push_offset;
    push_data.data.address = &heap_index_address;
    push_data.data.size = sizeof(heap_index_address);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, descriptor_heap_pipe);
    BindResourceHeap();
    vk::CmdPushDataEXT(m_command_buffer, &push_data);
    vk::CmdDraw(m_command_buffer, 3u, 1u, 0u, 0u);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* data = static_cast<uint32_t*>(buffer.Memory().Map());
        for (uint32_t i = 0; i < 3; ++i) {
            ASSERT_EQ(data[i], 1u);
        }
    }
}

TEST_F(PositiveDescriptorHeap, MappingSourceHeapWithIndirectIndexArray) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    InitRenderTarget();

    const VkDeviceSize offset = heap_props.bufferDescriptorAlignment * 7u;
    const uint32_t push_offset = 8u;
    const uint32_t address_offset = 16u;
    vkt::Buffer heap_index(*m_device, sizeof(uint32_t) + address_offset, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR,
                           vkt::device_address);
    uint32_t* heap_index_data = static_cast<uint32_t*>(heap_index.Memory().Map());
    heap_index_data[address_offset / sizeof(uint32_t)] = static_cast<uint32_t>(offset / heap_props.bufferDescriptorAlignment);

    vkt::Buffer buffer(*m_device, sizeof(uint32_t) * 4, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(offset + resource_stride);

    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_ + offset, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = {buffer.Address(), buffer.CreateInfo().size};

    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;

    vk::WriteResourceDescriptorsEXT(*m_device, 1, &descriptor_info, &descriptor_host);

    VkDescriptorSetAndBindingMappingEXT mapping =
        MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT;
    mapping.sourceData.indirectIndexArray = {};
    mapping.sourceData.indirectIndexArray.heapOffset = 0u;
    mapping.sourceData.indirectIndexArray.pushOffset = push_offset;
    mapping.sourceData.indirectIndexArray.addressOffset = address_offset;
    mapping.sourceData.indirectIndexArray.heapIndexStride = static_cast<uint32_t>(heap_props.bufferDescriptorAlignment);

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    char const* vert_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer Data {
            uvec4 data;
        };
        void main() {
            data[gl_VertexIndex] = 1u;
            gl_Position = vec4(1.0f);
            gl_PointSize = 1.0f;
        }
    )glsl";

    VkShaderObj vert_module = VkShaderObj(*m_device, vert_source, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj frag_module = VkShaderObj(*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkPipelineShaderStageCreateInfo stages[2];
    stages[0] = vert_module.GetStageCreateInfo();
    stages[0].pNext = &mapping_info;
    stages[1] = frag_module.GetStageCreateInfo();

    CreatePipelineHelper descriptor_heap_pipe(*this, &pipeline_create_flags_2_create_info);
    descriptor_heap_pipe.gp_ci_.layout = VK_NULL_HANDLE;
    descriptor_heap_pipe.gp_ci_.stageCount = 2u;
    descriptor_heap_pipe.gp_ci_.pStages = stages;
    descriptor_heap_pipe.CreateGraphicsPipeline(false);

    VkDeviceAddress heap_index_address = heap_index.Address();

    VkPushDataInfoEXT push_data = vku::InitStructHelper();
    push_data.offset = push_offset;
    push_data.data.address = &heap_index_address;
    push_data.data.size = sizeof(heap_index_address);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, descriptor_heap_pipe);
    BindResourceHeap();
    vk::CmdPushDataEXT(m_command_buffer, &push_data);
    vk::CmdDraw(m_command_buffer, 3u, 1u, 0u, 0u);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* data = static_cast<uint32_t*>(buffer.Memory().Map());
        for (uint32_t i = 0; i < 3; ++i) {
            ASSERT_EQ(data[i], 1u);
        }
    }
}

TEST_F(PositiveDescriptorHeap, MappingSourceHeapData) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    InitRenderTarget();

    vkt::Buffer write_buffer(*m_device, sizeof(uint32_t) * 4, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    const uint32_t read_heap_offset = static_cast<uint32_t>(physDevProps_.limits.minUniformBufferOffsetAlignment);
    const int32_t read_push_offset = -8;
    const uint32_t read_push_data_offset = 48u;
    const uint32_t read_offset = read_heap_offset + read_push_offset;
    const VkDeviceSize write_offset =
        Align(read_offset + heap_props.bufferDescriptorAlignment * 7u, heap_props.bufferDescriptorAlignment);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;

    CreateResourceHeap(write_offset + resource_stride);

    uint32_t* read_data = reinterpret_cast<uint32_t*>(resource_heap_data_ + read_offset);
    for (uint32_t i = 0; i < 4; ++i) {
        read_data[i] = i + 1;
    }

    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_ + write_offset, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = {write_buffer.Address(), write_buffer.CreateInfo().size};

    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;

    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT;
    mappings[0].sourceData.heapData.heapOffset = static_cast<uint32_t>(read_heap_offset);
    mappings[0].sourceData.heapData.pushOffset = static_cast<uint32_t>(read_push_data_offset);
    mappings[1] = MakeSetAndBindingMapping(1, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = static_cast<uint32_t>(write_offset);
    mappings[1].sourceData.constantOffset.heapArrayStride = 0u;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2u;
    mapping_info.pMappings = mappings;

    char const* vert_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform ReadData {
            uvec4 read_data;
        };
        layout(set = 1, binding = 0) buffer WriteData {
            uvec4 write_data;
        };
        void main() {
            if (gl_VertexIndex == 0) {
                write_data[0] = read_data[0];
                write_data[1] = read_data[1];
                write_data[2] = read_data[2];
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
    stages[0] = vert_module.GetStageCreateInfo();
    stages[0].pNext = &mapping_info;
    stages[1] = frag_module.GetStageCreateInfo();

    CreatePipelineHelper descriptor_heap_pipe(*this, &pipeline_create_flags_2_create_info);
    descriptor_heap_pipe.gp_ci_.layout = VK_NULL_HANDLE;
    descriptor_heap_pipe.gp_ci_.stageCount = 2u;
    descriptor_heap_pipe.gp_ci_.pStages = stages;
    descriptor_heap_pipe.CreateGraphicsPipeline(false);

    VkPushDataInfoEXT push_data = vku::InitStructHelper();
    push_data.offset = read_push_data_offset;
    push_data.data.address = &read_push_offset;
    push_data.data.size = sizeof(read_push_offset);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, descriptor_heap_pipe);
    BindResourceHeap();
    vk::CmdPushDataEXT(m_command_buffer, &push_data);
    vk::CmdDraw(m_command_buffer, 3u, 1u, 0u, 0u);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* write_data = static_cast<uint32_t*>(write_buffer.Memory().Map());
        for (uint32_t i = 0; i < 3; ++i) {
            ASSERT_EQ(write_data[i], read_data[i]);
        }
    }
}

TEST_F(PositiveDescriptorHeap, MappingSourcePushData) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    InitRenderTarget();

    vkt::Buffer write_buffer(*m_device, sizeof(uint32_t) * 4, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    const uint32_t read_offset = 48u;
    const VkDeviceSize write_offset =
        Align(read_offset + heap_props.bufferDescriptorAlignment * 7u, heap_props.bufferDescriptorAlignment);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;

    CreateResourceHeap(write_offset + resource_stride);

    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_ + write_offset, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = {write_buffer.Address(), write_buffer.CreateInfo().size};

    VkResourceDescriptorInfoEXT descriptor_info;
    descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;

    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT;
    mappings[0].sourceData.pushDataOffset = static_cast<uint32_t>(read_offset);
    mappings[1] = MakeSetAndBindingMapping(1, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = static_cast<uint32_t>(write_offset);
    mappings[1].sourceData.constantOffset.heapArrayStride = 0u;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2u;
    mapping_info.pMappings = mappings;

    char const* vert_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform ReadData {
            uvec4 read_data;
        };
        layout(set = 1, binding = 0) buffer WriteData {
            uvec4 write_data;
        };
        void main() {
            if (gl_VertexIndex == 0) {
                write_data[0] = read_data[0];
                write_data[1] = read_data[1];
                write_data[2] = read_data[2];
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
    stages[0] = vert_module.GetStageCreateInfo();
    stages[0].pNext = &mapping_info;
    stages[1] = frag_module.GetStageCreateInfo();

    CreatePipelineHelper descriptor_heap_pipe(*this, &pipeline_create_flags_2_create_info);
    descriptor_heap_pipe.gp_ci_.layout = VK_NULL_HANDLE;
    descriptor_heap_pipe.gp_ci_.stageCount = 2u;
    descriptor_heap_pipe.gp_ci_.pStages = stages;
    descriptor_heap_pipe.CreateGraphicsPipeline(false);

    uint32_t read_data[4];
    for (uint32_t i = 0; i < 4; ++i) {
        read_data[i] = i + 1;
    }
    VkPushDataInfoEXT push_data = vku::InitStructHelper();
    push_data.offset = read_offset;
    push_data.data.address = read_data;
    push_data.data.size = sizeof(uint32_t) * 4;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, descriptor_heap_pipe);
    BindResourceHeap();
    vk::CmdPushDataEXT(m_command_buffer, &push_data);
    vk::CmdDraw(m_command_buffer, 3u, 1u, 0u, 0u);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* write_data = static_cast<uint32_t*>(write_buffer.Memory().Map());
        for (uint32_t i = 0; i < 3; ++i) {
            ASSERT_EQ(write_data[i], read_data[i]);
        }
    }
}

TEST_F(PositiveDescriptorHeap, MappingSourcePushAddress) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    InitRenderTarget();

    vkt::Buffer read_buffer(*m_device, sizeof(uint32_t) * 4, VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT_KHR, vkt::device_address);
    uint32_t* read_data = static_cast<uint32_t*>(read_buffer.Memory().Map());
    for (uint32_t i = 0; i < 4; ++i) {
        read_data[i] = i + 1;
    }
    vkt::Buffer write_buffer(*m_device, sizeof(uint32_t) * 4, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    const uint32_t read_offset = 48u;
    const VkDeviceSize write_offset =
        Align(read_offset + heap_props.bufferDescriptorAlignment * 7u, heap_props.bufferDescriptorAlignment);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(write_offset + resource_stride);

    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_ + write_offset, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = {write_buffer.Address(), write_buffer.CreateInfo().size};

    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;

    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mappings[0].sourceData.pushAddressOffset = static_cast<uint32_t>(read_offset);
    mappings[1] = MakeSetAndBindingMapping(1, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = static_cast<uint32_t>(write_offset);
    mappings[1].sourceData.constantOffset.heapArrayStride = 0u;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2u;
    mapping_info.pMappings = mappings;

    char const* vert_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform ReadData {
            uvec4 read_data;
        };
        layout(set = 1, binding = 0) buffer WriteData {
            uvec4 write_data;
        };
        void main() {
            if (gl_VertexIndex == 0) {
                write_data[0] = read_data[0];
                write_data[1] = read_data[1];
                write_data[2] = read_data[2];
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
    stages[0] = vert_module.GetStageCreateInfo();
    stages[0].pNext = &mapping_info;
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
    BindResourceHeap();
    vk::CmdPushDataEXT(m_command_buffer, &push_data);
    vk::CmdDraw(m_command_buffer, 3u, 1u, 0u, 0u);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* write_data = static_cast<uint32_t*>(write_buffer.Memory().Map());
        for (uint32_t i = 0; i < 3; ++i) {
            ASSERT_EQ(write_data[i], read_data[i]);
        }
    }
}

TEST_F(PositiveDescriptorHeap, MappingSourceIndirectAddress) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    InitRenderTarget();

    const uint32_t read_offset = 48u;
    const uint32_t indirect_offset = 288u;

    vkt::Buffer read_buffer(*m_device, sizeof(uint32_t) * 4, VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT_KHR, vkt::device_address);
    uint32_t* read_data = static_cast<uint32_t*>(read_buffer.Memory().Map());
    for (uint32_t i = 0; i < 4; ++i) {
        read_data[i] = i + 1;
    }
    vkt::Buffer write_buffer(*m_device, sizeof(uint32_t) * 4, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    vkt::Buffer indirect_buffer(*m_device, sizeof(uint32_t) * 4, VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT_KHR, vkt::device_address);
    uint8_t* indirect_data = static_cast<uint8_t*>(indirect_buffer.Memory().Map());
    VkDeviceAddress* indirect_data_address = reinterpret_cast<VkDeviceAddress*>(indirect_data + indirect_offset);
    *indirect_data_address = read_buffer.Address();

    const VkDeviceSize write_offset =
        Align(read_offset + heap_props.bufferDescriptorAlignment * 7u, heap_props.bufferDescriptorAlignment);
    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(write_offset + resource_stride);

    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_ + write_offset, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = {write_buffer.Address(), write_buffer.CreateInfo().size};

    VkResourceDescriptorInfoEXT descriptor_info = vku::InitStructHelper();
    descriptor_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info.data.pAddressRange = &device_range;

    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &descriptor_info, &descriptor_host);

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT;
    mappings[0].sourceData.indirectAddress.pushOffset = static_cast<uint32_t>(read_offset);
    mappings[0].sourceData.indirectAddress.addressOffset = static_cast<uint32_t>(indirect_offset);
    mappings[1] = MakeSetAndBindingMapping(1, 0, 1, VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = static_cast<uint32_t>(write_offset);
    mappings[1].sourceData.constantOffset.heapArrayStride = 0u;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2u;
    mapping_info.pMappings = mappings;

    char const* vert_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform ReadData {
            uvec4 read_data;
        };
        layout(set = 1, binding = 0) buffer WriteData {
            uvec4 write_data;
        };
        void main() {
            if (gl_VertexIndex == 0) {
                write_data[0] = read_data[0];
                write_data[1] = read_data[1];
                write_data[2] = read_data[2];
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
    stages[0] = vert_module.GetStageCreateInfo();
    stages[0].pNext = &mapping_info;
    stages[1] = frag_module.GetStageCreateInfo();

    CreatePipelineHelper descriptor_heap_pipe(*this, &pipeline_create_flags_2_create_info);
    descriptor_heap_pipe.gp_ci_.layout = VK_NULL_HANDLE;
    descriptor_heap_pipe.gp_ci_.stageCount = 2u;
    descriptor_heap_pipe.gp_ci_.pStages = stages;
    descriptor_heap_pipe.CreateGraphicsPipeline(false);

    VkDeviceAddress indirect_address = indirect_buffer.Address();

    VkPushDataInfoEXT push_data = vku::InitStructHelper();
    push_data.offset = read_offset;
    push_data.data.address = &indirect_address;
    push_data.data.size = sizeof(indirect_address);

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, descriptor_heap_pipe);
    BindResourceHeap();
    vk::CmdPushDataEXT(m_command_buffer, &push_data);
    vk::CmdDraw(m_command_buffer, 3u, 1u, 0u, 0u);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* write_data = static_cast<uint32_t*>(write_buffer.Memory().Map());
        for (uint32_t i = 0; i < 3; ++i) {
            ASSERT_EQ(write_data[i], read_data[i]);
        }
    }
}

TEST_F(PositiveDescriptorHeap, UntypedPointerBuffer) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

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

    const std::string spirv = R"(
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

    VkShaderObj cs_module = VkShaderObj::CreateFromASM(this, spirv.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    pipe.CreateComputePipeline(false);

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

TEST_F(PositiveDescriptorHeap, NestedResourceInheritance) {
    TEST_DESCRIPTION("Validate that inherited ranges match primary buffer");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::nestedCommandBuffer);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize heap_size = heap_props.minResourceHeapReservedRange + 2 * heap_props.bufferDescriptorSize;
    CreateResourceHeap(heap_size);

    vkt::CommandBuffer secondary1(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    vkt::CommandBuffer secondary2(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkBindHeapInfoEXT bind_info = vku::InitStructHelper();
    bind_info.heapRange = {resource_heap_.Address(), heap_size};
    bind_info.reservedRangeOffset = 0;
    bind_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;

    VkCommandBufferInheritanceDescriptorHeapInfoEXT inh_desc_heap_info = vku::InitStructHelper();
    inh_desc_heap_info.pResourceHeapBindInfo = &bind_info;

    VkCommandBufferInheritanceInfo inh = vku::InitStructHelper(&inh_desc_heap_info);
    VkCommandBufferBeginInfo cbbi = vku::InitStructHelper();
    cbbi.pInheritanceInfo = &inh;

    secondary1.Begin(&cbbi);
    secondary1.End();

    secondary2.Begin(&cbbi);
    vk::CmdExecuteCommands(secondary2, 1, &secondary1.handle());
    secondary2.End();

    m_command_buffer.Begin();
    vk::CmdBindResourceHeapEXT(m_command_buffer, &bind_info);
    vk::CmdExecuteCommands(m_command_buffer, 1, &secondary1.handle());
    m_command_buffer.End();
}

TEST_F(PositiveDescriptorHeap, ConstantMemoryAccess) {
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkDescriptorSetAndBindingMappingEXT mapping = MakeSetAndBindingMapping(0, 0);
    mapping.source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mapping.sourceData.pushAddressOffset = 0u;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 1u;
    mapping_info.pMappings = &mapping;

    char const* cs_source1 = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer Data {
            uint data[];
        };
        void main() {
	        data[0] = 4u;
        }
    )glsl";
    VkShaderObj cs_module1 = VkShaderObj(*m_device, cs_source1, VK_SHADER_STAGE_COMPUTE_BIT);

    char const* cs_source2 = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer Data {
            uint data[];
        };
        void main() {
            atomicExchange(data[0], 4u);
        }
    )glsl";
    VkShaderObj cs_module2 = VkShaderObj(*m_device, cs_source2, VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineCreateFlags2CreateInfoKHR flags2_ci = vku::InitStructHelper();
    flags2_ci.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &flags2_ci);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module1.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    pipe.CreateComputePipeline(false);
    pipe.Destroy();

    pipe.cp_ci_.stage.module = cs_module2;
    pipe.CreateComputePipeline(false);
}

TEST_F(PositiveDescriptorHeap, ConstantImageMemoryAccess) {
    AddRequiredFeature(vkt::Feature::runtimeDescriptorArray);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkDescriptorSetAndBindingMappingEXT mappings[3];
    mappings[0] = MakeSetAndBindingMapping(0, 0, 1, VK_SPIRV_RESOURCE_TYPE_SAMPLED_IMAGE_BIT_EXT);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT;
    mappings[0].sourceData.indirectIndexArray = {};
    mappings[1] = MakeSetAndBindingMapping(0, 1, 1, VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[1].sourceData.constantOffset = {};
    mappings[1].sourceData.constantOffset.heapOffset = 1024;
    mappings[2] = MakeSetAndBindingMapping(1, 0);
    mappings[2].source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT;
    mappings[2].sourceData.constantOffset = {};
    mappings[2].sourceData.constantOffset.heapOffset = 2048;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 3u;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 0) uniform texture2D tex[];
        layout(set = 0, binding = 1) uniform sampler sampl;
        layout(set = 1, binding = 0) buffer ssbo {
	        vec4 data;
        };
        void main() {
	        data = texture(sampler2D(tex[7], sampl), vec2(0.5f));
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineCreateFlags2CreateInfoKHR flags2_ci = vku::InitStructHelper();
    flags2_ci.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &flags2_ci);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    pipe.CreateComputePipeline(false);
}

TEST_F(PositiveDescriptorHeap, CmdPushData) {
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    std::vector<uint8_t> payload(static_cast<size_t>(heap_props.maxPushDataSize));

    VkPushDataInfoEXT push_data_info = vku::InitStructHelper();
    push_data_info.data.address = payload.data();

    m_command_buffer.Begin();

    push_data_info.offset = 0u;
    push_data_info.data.size = static_cast<size_t>(heap_props.maxPushDataSize);
    vk::CmdPushDataEXT(m_command_buffer, &push_data_info);

    push_data_info.offset = (uint32_t)heap_props.maxPushDataSize / 2u;
    push_data_info.data.size = static_cast<size_t>(heap_props.maxPushDataSize / 2u);
    vk::CmdPushDataEXT(m_command_buffer, &push_data_info);

    push_data_info.offset = (uint32_t)heap_props.maxPushDataSize - sizeof(uint32_t);
    push_data_info.data.size = sizeof(uint32_t);
    vk::CmdPushDataEXT(m_command_buffer, &push_data_info);

    m_command_buffer.End();
}

TEST_F(PositiveDescriptorHeap, ResourceMask) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    uint32_t resource_offset = 0;
    uint32_t sampled_image_offset = Align(resource_offset, (uint32_t)heap_props.imageDescriptorSize);
    resource_offset += (uint32_t)heap_props.imageDescriptorSize;
    uint32_t read_only_image_offset = Align(resource_offset, (uint32_t)heap_props.imageDescriptorSize);
    resource_offset += (uint32_t)heap_props.imageDescriptorSize;
    uint32_t read_write_image_offset = Align(resource_offset, (uint32_t)heap_props.imageDescriptorSize);
    resource_offset += (uint32_t)heap_props.imageDescriptorSize;
    uint32_t combined_sampled_image_offset = Align(resource_offset, (uint32_t)heap_props.imageDescriptorSize);
    resource_offset += (uint32_t)heap_props.imageDescriptorSize;
    uint32_t uniform_buffer_offset = Align(resource_offset, (uint32_t)heap_props.bufferDescriptorSize);
    resource_offset += (uint32_t)heap_props.bufferDescriptorSize;
    uint32_t read_only_storage_buffer_offset = Align(resource_offset, (uint32_t)heap_props.bufferDescriptorSize);
    resource_offset += (uint32_t)heap_props.bufferDescriptorSize;
    uint32_t read_write_storage_buffer_offset = Align(resource_offset, (uint32_t)heap_props.bufferDescriptorSize);
    resource_offset += (uint32_t)heap_props.bufferDescriptorSize;

    const VkDeviceSize resource_heap_size_app = Align(resource_offset, (uint32_t)heap_props.resourceHeapAlignment);
    CreateResourceHeap(resource_heap_size_app);

    const VkDeviceSize sampler_heap_size_app = heap_props.samplerDescriptorSize;
    CreateSamplerHeap(sampler_heap_size_app);

    vkt::Image sampled_image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM,
                             VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image read_only_image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM,
                               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    vkt::Image read_write_image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM,
                                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    vkt::Image combined_image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM,
                              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Buffer uniform_buffer(*m_device, 64u, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    vkt::Buffer read_only_storage_buffer(*m_device, 64u, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::Buffer read_write_storage_buffer(*m_device, 64u, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    VkImageViewCreateInfo sampled_view_info = sampled_image.BasicViewCreatInfo();
    VkImageViewCreateInfo read_only_view_info = read_only_image.BasicViewCreatInfo();
    VkImageViewCreateInfo read_write_view_info = read_write_image.BasicViewCreatInfo();
    VkImageViewCreateInfo combined_view_info = combined_image.BasicViewCreatInfo();

    VkHostAddressRangeEXT descriptor_host[7];
    descriptor_host[0].address = resource_heap_data_ + sampled_image_offset;
    descriptor_host[0].size = static_cast<size_t>(heap_props.imageDescriptorSize);
    descriptor_host[1].address = resource_heap_data_ + read_only_image_offset;
    descriptor_host[1].size = static_cast<size_t>(heap_props.imageDescriptorSize);
    descriptor_host[2].address = resource_heap_data_ + read_write_image_offset;
    descriptor_host[2].size = static_cast<size_t>(heap_props.imageDescriptorSize);
    descriptor_host[3].address = resource_heap_data_ + combined_sampled_image_offset;
    descriptor_host[3].size = static_cast<size_t>(heap_props.imageDescriptorSize);
    descriptor_host[4].address = resource_heap_data_ + uniform_buffer_offset;
    descriptor_host[4].size = static_cast<size_t>(heap_props.bufferDescriptorSize);
    descriptor_host[5].address = resource_heap_data_ + read_only_storage_buffer_offset;
    descriptor_host[5].size = static_cast<size_t>(heap_props.bufferDescriptorSize);
    descriptor_host[6].address = resource_heap_data_ + read_write_storage_buffer_offset;
    descriptor_host[6].size = static_cast<size_t>(heap_props.bufferDescriptorSize);

    VkImageDescriptorInfoEXT image_descriptor_info[4];
    image_descriptor_info[0] = vku::InitStructHelper();
    image_descriptor_info[0].pView = &sampled_view_info;
    image_descriptor_info[0].layout = VK_IMAGE_LAYOUT_GENERAL;
    image_descriptor_info[1] = vku::InitStructHelper();
    image_descriptor_info[1].pView = &read_only_view_info;
    image_descriptor_info[1].layout = VK_IMAGE_LAYOUT_GENERAL;
    image_descriptor_info[2] = vku::InitStructHelper();
    image_descriptor_info[2].pView = &read_write_view_info;
    image_descriptor_info[2].layout = VK_IMAGE_LAYOUT_GENERAL;
    image_descriptor_info[3] = vku::InitStructHelper();
    image_descriptor_info[3].pView = &combined_view_info;
    image_descriptor_info[3].layout = VK_IMAGE_LAYOUT_GENERAL;

    VkDeviceAddressRangeEXT device_ranges[3];
    device_ranges[0].address = uniform_buffer.Address();
    device_ranges[0].size = 64;
    device_ranges[1].address = read_only_storage_buffer.Address();
    device_ranges[1].size = 64;
    device_ranges[2].address = read_write_storage_buffer.Address();
    device_ranges[2].size = 64;

    VkResourceDescriptorInfoEXT descriptor_info[7];
    descriptor_info[0] = vku::InitStructHelper();
    descriptor_info[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptor_info[0].data.pImage = &image_descriptor_info[0];
    descriptor_info[1] = vku::InitStructHelper();
    descriptor_info[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptor_info[1].data.pImage = &image_descriptor_info[1];
    descriptor_info[2] = vku::InitStructHelper();
    descriptor_info[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptor_info[2].data.pImage = &image_descriptor_info[2];
    descriptor_info[3] = vku::InitStructHelper();
    descriptor_info[3].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptor_info[3].data.pImage = &image_descriptor_info[3];
    descriptor_info[4] = vku::InitStructHelper();
    descriptor_info[4].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_info[4].data.pAddressRange = &device_ranges[0];
    descriptor_info[5] = vku::InitStructHelper();
    descriptor_info[5].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info[5].data.pAddressRange = &device_ranges[1];
    descriptor_info[6] = vku::InitStructHelper();
    descriptor_info[6].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info[6].data.pAddressRange = &device_ranges[2];
    vk::WriteResourceDescriptorsEXT(*m_device, 7, descriptor_info, descriptor_host);

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    VkHostAddressRangeEXT sampler_host = {sampler_heap_data_, static_cast<size_t>(heap_props.samplerDescriptorSize)};
    vk::WriteSamplerDescriptorsEXT(*m_device, 1, &sampler_info, &sampler_host);

    VkDescriptorSetAndBindingMappingEXT mappings[8];
    for (uint32_t i = 0; i < 8; ++i) {
        mappings[i] = MakeSetAndBindingMapping(0, i, 1, VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT);
    }
    mappings[0].resourceMask = VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = 0;
    mappings[1].resourceMask = VK_SPIRV_RESOURCE_TYPE_SAMPLED_IMAGE_BIT_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = sampled_image_offset;
    mappings[2].resourceMask = VK_SPIRV_RESOURCE_TYPE_READ_ONLY_IMAGE_BIT_EXT;
    mappings[2].sourceData.constantOffset.heapOffset = read_only_image_offset;
    mappings[3].resourceMask = VK_SPIRV_RESOURCE_TYPE_READ_WRITE_IMAGE_BIT_EXT;
    mappings[3].sourceData.constantOffset.heapOffset = read_write_image_offset;
    mappings[4].resourceMask = VK_SPIRV_RESOURCE_TYPE_COMBINED_SAMPLED_IMAGE_BIT_EXT;
    mappings[4].sourceData.constantOffset.heapOffset = combined_sampled_image_offset;
    mappings[4].sourceData.constantOffset.samplerHeapOffset = 0u;
    mappings[5].resourceMask = VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT;
    mappings[5].sourceData.constantOffset.heapOffset = uniform_buffer_offset;
    mappings[6].resourceMask = VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT;
    mappings[6].sourceData.constantOffset.heapOffset = read_only_storage_buffer_offset;
    mappings[7].resourceMask = VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT;
    mappings[7].sourceData.constantOffset.heapOffset = read_write_storage_buffer_offset;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 8;
    mapping_info.pMappings = mappings;

    char const* cs_source = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) uniform sampler sampl;
        layout(set = 0, binding = 1) uniform texture2D sampledImage;
        layout(set = 0, binding = 2, rgba8) readonly uniform image2D readOnlyImage;
        layout(set = 0, binding = 3) writeonly uniform image2D readWriteImage;
        layout(set = 0, binding = 4) uniform sampler2D combinedSampledImage;
        layout(set = 0, binding = 5) uniform uniformBuffer { vec4 data; } unif;
        layout(set = 0, binding = 6) readonly buffer readOnlyStorageBuffer { vec4 data; } readOnlyBuf;
        layout(set = 0, binding = 7) buffer readWriteStorageBuffer { vec4 data; } readWriteBuf;
        void main() {
            readWriteBuf.data = unif.data + readOnlyBuf.data;
            vec4 color = vec4(0.0f);
            color += texture(sampler2D(sampledImage, sampl), vec2(0.5f));
            color += imageLoad(readOnlyImage, ivec2(0, 0));
            color += texture(combinedSampledImage, vec2(0.5f));
            imageStore(readWriteImage, ivec2(0, 0), color);
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    pipe.CreateComputePipeline(false);

    m_command_buffer.Begin();
    BindResourceHeap();
    BindSamplerHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveDescriptorHeap, ResourceMaskSameBinding) {
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    uint32_t resource_offset = 0;
    uint32_t sampled_image_offset = Align(resource_offset, (uint32_t)heap_props.imageDescriptorSize);
    resource_offset += (uint32_t)heap_props.imageDescriptorSize;
    uint32_t read_only_image_offset = Align(resource_offset, (uint32_t)heap_props.imageDescriptorSize);
    resource_offset += (uint32_t)heap_props.imageDescriptorSize;
    uint32_t read_write_image_offset = Align(resource_offset, (uint32_t)heap_props.imageDescriptorSize);
    resource_offset += (uint32_t)heap_props.imageDescriptorSize;
    uint32_t combined_sampled_image_offset = Align(resource_offset, (uint32_t)heap_props.imageDescriptorSize);
    resource_offset += (uint32_t)heap_props.imageDescriptorSize;
    uint32_t uniform_buffer_offset = Align(resource_offset, (uint32_t)heap_props.bufferDescriptorSize);
    resource_offset += (uint32_t)heap_props.bufferDescriptorSize;
    uint32_t read_only_storage_buffer_offset = Align(resource_offset, (uint32_t)heap_props.bufferDescriptorSize);
    resource_offset += (uint32_t)heap_props.bufferDescriptorSize;
    uint32_t read_write_storage_buffer_offset = Align(resource_offset, (uint32_t)heap_props.bufferDescriptorSize);
    resource_offset += (uint32_t)heap_props.bufferDescriptorSize;

    const VkDeviceSize resource_heap_size_app = Align(resource_offset, (uint32_t)heap_props.resourceHeapAlignment);
    CreateResourceHeap(resource_heap_size_app);

    const VkDeviceSize sampler_heap_size_app = heap_props.samplerDescriptorSize;
    CreateSamplerHeap(sampler_heap_size_app);

    vkt::Image sampled_image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM,
                             VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image read_only_image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM,
                               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    vkt::Image read_write_image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM,
                                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    vkt::Image combined_image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM,
                              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Buffer uniform_buffer(*m_device, 64u, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, vkt::device_address);
    vkt::Buffer read_only_storage_buffer(*m_device, 64u, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    vkt::Buffer read_write_storage_buffer(*m_device, 64u, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);

    VkImageViewCreateInfo sampled_view_info = sampled_image.BasicViewCreatInfo();
    VkImageViewCreateInfo read_only_view_info = read_only_image.BasicViewCreatInfo();
    VkImageViewCreateInfo read_write_view_info = read_write_image.BasicViewCreatInfo();
    VkImageViewCreateInfo combined_view_info = combined_image.BasicViewCreatInfo();

    VkHostAddressRangeEXT descriptor_host[7];
    descriptor_host[0].address = resource_heap_data_ + sampled_image_offset;
    descriptor_host[0].size = static_cast<size_t>(heap_props.imageDescriptorSize);
    descriptor_host[1].address = resource_heap_data_ + read_only_image_offset;
    descriptor_host[1].size = static_cast<size_t>(heap_props.imageDescriptorSize);
    descriptor_host[2].address = resource_heap_data_ + read_write_image_offset;
    descriptor_host[2].size = static_cast<size_t>(heap_props.imageDescriptorSize);
    descriptor_host[3].address = resource_heap_data_ + combined_sampled_image_offset;
    descriptor_host[3].size = static_cast<size_t>(heap_props.imageDescriptorSize);
    descriptor_host[4].address = resource_heap_data_ + uniform_buffer_offset;
    descriptor_host[4].size = static_cast<size_t>(heap_props.bufferDescriptorSize);
    descriptor_host[5].address = resource_heap_data_ + read_only_storage_buffer_offset;
    descriptor_host[5].size = static_cast<size_t>(heap_props.bufferDescriptorSize);
    descriptor_host[6].address = resource_heap_data_ + read_write_storage_buffer_offset;
    descriptor_host[6].size = static_cast<size_t>(heap_props.bufferDescriptorSize);

    VkImageDescriptorInfoEXT image_descriptor_info[4];
    image_descriptor_info[0] = vku::InitStructHelper();
    image_descriptor_info[0].pView = &sampled_view_info;
    image_descriptor_info[0].layout = VK_IMAGE_LAYOUT_GENERAL;
    image_descriptor_info[1] = vku::InitStructHelper();
    image_descriptor_info[1].pView = &read_only_view_info;
    image_descriptor_info[1].layout = VK_IMAGE_LAYOUT_GENERAL;
    image_descriptor_info[2] = vku::InitStructHelper();
    image_descriptor_info[2].pView = &read_write_view_info;
    image_descriptor_info[2].layout = VK_IMAGE_LAYOUT_GENERAL;
    image_descriptor_info[3] = vku::InitStructHelper();
    image_descriptor_info[3].pView = &combined_view_info;
    image_descriptor_info[3].layout = VK_IMAGE_LAYOUT_GENERAL;

    VkDeviceAddressRangeEXT device_ranges[3];
    device_ranges[0].address = uniform_buffer.Address();
    device_ranges[0].size = 64;
    device_ranges[1].address = read_only_storage_buffer.Address();
    device_ranges[1].size = 64;
    device_ranges[2].address = read_write_storage_buffer.Address();
    device_ranges[2].size = 64;

    VkResourceDescriptorInfoEXT descriptor_info[7];
    descriptor_info[0] = vku::InitStructHelper();
    descriptor_info[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptor_info[0].data.pImage = &image_descriptor_info[0];
    descriptor_info[1] = vku::InitStructHelper();
    descriptor_info[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptor_info[1].data.pImage = &image_descriptor_info[1];
    descriptor_info[2] = vku::InitStructHelper();
    descriptor_info[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptor_info[2].data.pImage = &image_descriptor_info[2];
    descriptor_info[3] = vku::InitStructHelper();
    descriptor_info[3].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptor_info[3].data.pImage = &image_descriptor_info[3];
    descriptor_info[4] = vku::InitStructHelper();
    descriptor_info[4].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_info[4].data.pAddressRange = &device_ranges[0];
    descriptor_info[5] = vku::InitStructHelper();
    descriptor_info[5].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info[5].data.pAddressRange = &device_ranges[1];
    descriptor_info[6] = vku::InitStructHelper();
    descriptor_info[6].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_info[6].data.pAddressRange = &device_ranges[2];
    vk::WriteResourceDescriptorsEXT(*m_device, 7, descriptor_info, descriptor_host);

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    VkHostAddressRangeEXT sampler_host = {sampler_heap_data_, static_cast<size_t>(heap_props.samplerDescriptorSize)};
    vk::WriteSamplerDescriptorsEXT(*m_device, 1, &sampler_info, &sampler_host);

    VkDescriptorSetAndBindingMappingEXT mappings[8];
    for (uint32_t i = 0; i < 8; ++i) {
        mappings[i] = MakeSetAndBindingMapping(0, 0, 1, VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT);
    }
    mappings[0].resourceMask = VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT;
    mappings[0].sourceData.constantOffset.heapOffset = 0;
    mappings[1].resourceMask = VK_SPIRV_RESOURCE_TYPE_SAMPLED_IMAGE_BIT_EXT;
    mappings[1].sourceData.constantOffset.heapOffset = sampled_image_offset;
    mappings[2].resourceMask = VK_SPIRV_RESOURCE_TYPE_READ_ONLY_IMAGE_BIT_EXT;
    mappings[2].sourceData.constantOffset.heapOffset = read_only_image_offset;
    mappings[3].resourceMask = VK_SPIRV_RESOURCE_TYPE_READ_WRITE_IMAGE_BIT_EXT;
    mappings[3].sourceData.constantOffset.heapOffset = read_write_image_offset;
    mappings[4].resourceMask = VK_SPIRV_RESOURCE_TYPE_COMBINED_SAMPLED_IMAGE_BIT_EXT;
    mappings[4].sourceData.constantOffset.heapOffset = combined_sampled_image_offset;
    mappings[4].sourceData.constantOffset.samplerHeapOffset = 0u;
    mappings[5].resourceMask = VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT;
    mappings[5].sourceData.constantOffset.heapOffset = uniform_buffer_offset;
    mappings[6].resourceMask = VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT;
    mappings[6].sourceData.constantOffset.heapOffset = read_only_storage_buffer_offset;
    mappings[7].resourceMask = VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT;
    mappings[7].sourceData.constantOffset.heapOffset = read_write_storage_buffer_offset;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 8;
    mapping_info.pMappings = mappings;

    const std::string spirv = R"(
               OpCapability Shader
               OpCapability StorageImageWriteWithoutFormat
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"                  ; id %4
               OpName %readWriteStorageBuffer "readWriteStorageBuffer"  ; id %8
               OpMemberName %readWriteStorageBuffer 0 "data"
               OpName %readWriteBuf "readWriteBuf"  ; id %10
               OpName %uniformBuffer "uniformBuffer"    ; id %13
               OpMemberName %uniformBuffer 0 "data"
               OpName %unif "unif"                  ; id %15
               OpName %readOnlyStorageBuffer "readOnlyStorageBuffer"    ; id %19
               OpMemberName %readOnlyStorageBuffer 0 "data"
               OpName %readOnlyBuf "readOnlyBuf"    ; id %21
               OpName %color "color"                ; id %27
               OpName %sampledImage "sampledImage"  ; id %32
               OpName %sampl "sampl"                ; id %36
               OpName %readOnlyImage "readOnlyImage"    ; id %48
               OpName %combinedSampledImage "combinedSampledImage"  ; id %56
               OpName %readWriteImage "readWriteImage"              ; id %63

               ; Annotations
               OpDecorate %readWriteStorageBuffer BufferBlock
               OpMemberDecorate %readWriteStorageBuffer 0 Offset 0
               OpDecorate %readWriteBuf Binding 0
               OpDecorate %readWriteBuf DescriptorSet 0
               OpDecorate %uniformBuffer Block
               OpMemberDecorate %uniformBuffer 0 Offset 0
               OpDecorate %unif Binding 0
               OpDecorate %unif DescriptorSet 0
               OpDecorate %readOnlyStorageBuffer BufferBlock
               OpMemberDecorate %readOnlyStorageBuffer 0 NonWritable
               OpMemberDecorate %readOnlyStorageBuffer 0 Offset 0
               OpDecorate %readOnlyBuf NonWritable
               OpDecorate %readOnlyBuf Binding 0
               OpDecorate %readOnlyBuf DescriptorSet 0
               OpDecorate %sampledImage Binding 0
               OpDecorate %sampledImage DescriptorSet 0
               OpDecorate %sampl Binding 0
               OpDecorate %sampl DescriptorSet 0
               OpDecorate %readOnlyImage Binding 0
               OpDecorate %readOnlyImage DescriptorSet 0
               OpDecorate %combinedSampledImage Binding 0
               OpDecorate %combinedSampledImage DescriptorSet 0
               OpDecorate %readWriteImage NonReadable
               OpDecorate %readWriteImage Binding 0
               OpDecorate %readWriteImage DescriptorSet 0
               OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%readWriteStorageBuffer = OpTypeStruct %v4float     ; BufferBlock
%_ptr_Uniform_readWriteStorageBuffer = OpTypePointer Uniform %readWriteStorageBuffer
%readWriteBuf = OpVariable %_ptr_Uniform_readWriteStorageBuffer Uniform     ; Binding 0, DescriptorSet 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%uniformBuffer = OpTypeStruct %v4float              ; Block
%_ptr_Uniform_uniformBuffer = OpTypePointer Uniform %uniformBuffer
       %unif = OpVariable %_ptr_Uniform_uniformBuffer Uniform   ; Binding 0, DescriptorSet 0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%readOnlyStorageBuffer = OpTypeStruct %v4float      ; BufferBlock
%_ptr_Uniform_readOnlyStorageBuffer = OpTypePointer Uniform %readOnlyStorageBuffer
%readOnlyBuf = OpVariable %_ptr_Uniform_readOnlyStorageBuffer Uniform   ; NonWritable, Binding 0, DescriptorSet 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_0 = OpConstant %float 0
         %29 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
         %30 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_30 = OpTypePointer UniformConstant %30
%sampledImage = OpVariable %_ptr_UniformConstant_30 UniformConstant     ; Binding 0, DescriptorSet 0
         %34 = OpTypeSampler
%_ptr_UniformConstant_34 = OpTypePointer UniformConstant %34
      %sampl = OpVariable %_ptr_UniformConstant_34 UniformConstant  ; Binding 0, DescriptorSet 0
         %38 = OpTypeSampledImage %30
    %v2float = OpTypeVector %float 2
  %float_0_5 = OpConstant %float 0.5
         %42 = OpConstantComposite %v2float %float_0_5 %float_0_5
         %46 = OpTypeImage %float 2D 0 0 0 2 Rgba8
%_ptr_UniformConstant_46 = OpTypePointer UniformConstant %46
%readOnlyImage = OpVariable %_ptr_UniformConstant_46 UniformConstant    ; Binding 0, DescriptorSet 0
      %v2int = OpTypeVector %int 2
         %51 = OpConstantComposite %v2int %int_0 %int_0
%_ptr_UniformConstant_38 = OpTypePointer UniformConstant %38
%combinedSampledImage = OpVariable %_ptr_UniformConstant_38 UniformConstant     ; Binding 0, DescriptorSet 0
         %61 = OpTypeImage %float 2D 0 0 0 2 Unknown
%_ptr_UniformConstant_61 = OpTypePointer UniformConstant %61
%readWriteImage = OpVariable %_ptr_UniformConstant_61 UniformConstant   ; NonReadable, Binding 0, DescriptorSet 0
       %uint = OpTypeInt 32 0
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
%gl_WorkGroupSize = OpConstantComposite %v3uint %uint_1 %uint_1 %uint_1     ; BuiltIn WorkgroupSize

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
      %color = OpVariable %_ptr_Function_v4float Function
         %17 = OpAccessChain %_ptr_Uniform_v4float %unif %int_0
         %18 = OpLoad %v4float %17
         %22 = OpAccessChain %_ptr_Uniform_v4float %readOnlyBuf %int_0
         %23 = OpLoad %v4float %22
         %24 = OpFAdd %v4float %18 %23
         %25 = OpAccessChain %_ptr_Uniform_v4float %readWriteBuf %int_0
               OpStore %25 %24
               OpStore %color %29
         %33 = OpLoad %30 %sampledImage
         %37 = OpLoad %34 %sampl
         %39 = OpSampledImage %38 %33 %37
         %43 = OpImageSampleExplicitLod %v4float %39 %42 Lod %float_0
         %44 = OpLoad %v4float %color
         %45 = OpFAdd %v4float %44 %43
               OpStore %color %45
         %49 = OpLoad %46 %readOnlyImage
         %52 = OpImageRead %v4float %49 %51
         %53 = OpLoad %v4float %color
         %54 = OpFAdd %v4float %53 %52
               OpStore %color %54
         %57 = OpLoad %38 %combinedSampledImage
         %58 = OpImageSampleExplicitLod %v4float %57 %42 Lod %float_0
         %59 = OpLoad %v4float %color
         %60 = OpFAdd %v4float %59 %58
               OpStore %color %60
         %64 = OpLoad %61 %readWriteImage
         %65 = OpLoad %v4float %color
               OpImageWrite %64 %51 %65
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj cs_module = VkShaderObj::CreateFromASM(this, spirv.c_str(), VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    pipe.CreateComputePipeline(false);

    m_command_buffer.Begin();
    BindResourceHeap();
    BindSamplerHeap();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveDescriptorHeap, ArrayStrideIdEXT) {
    SetTargetApiVersion(VK_API_VERSION_1_4);
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    std::stringstream cs_source;
    cs_source << R"(
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

    VkPipelineCreateFlags2CreateInfo pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkShaderObj cs_module =
        VkShaderObj(*m_device, cs_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_4, SPV_SOURCE_ASM);

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.CreateComputePipeline(false);
}

TEST_F(PositiveDescriptorHeap, OffsetIdEXT) {
    SetTargetApiVersion(VK_API_VERSION_1_4);
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    std::stringstream cs_source;
    cs_source << R"(
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpCapability Sampled1D
               OpExtension "SPV_KHR_untyped_pointers"
               OpExtension "SPV_EXT_descriptor_heap"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main" %2 %3
               OpExecutionMode %1 LocalSize 1 1 1
               OpDecorate %4 SpecId 0
               OpDecorate %5 SpecId 1
               OpDecorate %6 SpecId 2
               OpDecorate %2 BuiltIn ResourceHeapEXT
               OpDecorate %3 BuiltIn SamplerHeapEXT
               OpDecorateId %7 OffsetIdEXT %4
               OpDecorateId %8 OffsetIdEXT %5
               OpDecorateId %9 OffsetIdEXT %6
               OpMemberDecorate %10 0 Offset 0
         %11 = OpTypeVoid
         %12 = OpTypeFunction %11
         %13 = OpTypeInt 32 0
         %14 = OpTypeVector %13 4
         %15 = OpTypeFloat 32
         %16 = OpConstant %13 0
         %17 = OpConstant %13 1
         %18 = OpConstant %15 0
         %19 = OpConstant %15 1
          %4 = OpSpecConstant %13 0
          %5 = OpSpecConstant %13 0
          %6 = OpSpecConstant %13 0
         %20 = OpTypeImage %13 1D 0 0 0 1 Unknown
         %21 = OpTypeBufferEXT StorageBuffer
         %22 = OpTypeSampler
         %23 = OpTypeSampledImage %20
          %7 = OpTypeRuntimeArray %20
          %8 = OpTypeRuntimeArray %21
          %9 = OpTypeRuntimeArray %22
         %10 = OpTypeStruct %13
         %24 = OpTypeUntypedPointerKHR UniformConstant
         %25 = OpTypeUntypedPointerKHR StorageBuffer
          %2 = OpUntypedVariableKHR %24 UniformConstant
          %3 = OpUntypedVariableKHR %24 UniformConstant
          %1 = OpFunction %11 None %12
         %26 = OpLabel
         %27 = OpUntypedAccessChainKHR %24 %7 %2 %16
         %28 = OpLoad %20 %27
         %29 = OpUntypedAccessChainKHR %24 %9 %3 %16
         %30 = OpLoad %22 %29
         %31 = OpSampledImage %23 %28 %30
         %32 = OpImageSampleExplicitLod %14 %31 %19 Lod %18
         %33 = OpCompositeExtract %13 %32 0
         %34 = OpUntypedAccessChainKHR %24 %8 %2 %16
         %35 = OpBufferPointerEXT %25 %34
               OpStore %35 %33
               OpReturn
               OpFunctionEnd
    )";

    VkPipelineCreateFlags2CreateInfo pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    VkShaderObj cs_module =
        VkShaderObj(*m_device, cs_source.str().c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_4, SPV_SOURCE_ASM);

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    pipe.CreateComputePipeline(false);
}

TEST_F(PositiveDescriptorHeap, LayoutDescriptorHeap) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = {buffer.Address(), 256};
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
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.CreateComputePipeline(false);

    uint32_t src_data = 4321u;

    VkPushDataInfoEXT push_data_info = vku::InitStructHelper();
    push_data_info.offset = 0u;
    push_data_info.data.size = sizeof(uint32_t);
    push_data_info.data.address = &src_data;

    VkPushConstantRange push_const_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)};
    VkPipelineLayoutCreateInfo pipeline_layout_info = vku::InitStructHelper();
    pipeline_layout_info.pushConstantRangeCount = 1u;
    pipeline_layout_info.pPushConstantRanges = &push_const_range;
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_info);

    m_command_buffer.Begin();
    vk::CmdPushConstants(m_command_buffer, pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &src_data);
    BindResourceHeap();
    vk::CmdPushDataEXT(m_command_buffer, &push_data_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* data = static_cast<uint32_t*>(buffer.Memory().Map());
        ASSERT_EQ(data[0], src_data);
    }
}

TEST_F(PositiveDescriptorHeap, UntypedImageAndSampler) {
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    InitRenderTarget();

    const uint32_t buffer_index = 16u;

    const VkDeviceSize image_offset = 0u;
    const VkDeviceSize image_size = heap_props.imageDescriptorSize;
    const VkDeviceSize buffer_offset = heap_props.bufferDescriptorAlignment * buffer_index;
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

    if (!IsPlatformMockICD()) {
        float* data = static_cast<float*>(buffer.Memory().Map());
        for (uint32_t i = 0; i < 4; ++i) {
            ASSERT_EQ(data[i], color.float32[i]);
        }
    }
}

TEST_F(PositiveDescriptorHeap, UboAndSsboBindings) {
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    char const* cs_source = R"glsl(
        #version 450
        #extension GL_EXT_descriptor_heap : require
        layout(set = 0, binding = 0) buffer ubo0 { vec4 data; } Ubo0[];
        layout(set = 0, binding = 1) buffer ubo1 { vec4 data; } Ubo1[];
        layout(set = 0, binding = 2) buffer ubo2 { vec4 data; } Ubo2[];
        layout(set = 0, binding = 3) buffer ubo3 { vec4 data; } Ubo3[];
        layout(set = 0, binding = 4) buffer ubo4 { vec4 data; } Ubo4[];
        layout(set = 0, binding = 5) buffer ubo5 { vec4 data; } Ubo5[];
        layout(set = 0, binding = 6) buffer ubo6 { vec4 data; } Ubo6[];
        layout(set = 0, binding = 7) buffer ubo7 { vec4 data; } Ubo7[];
        layout(set = 0, binding = 8) buffer ubo8 { vec4 data; } Ubo8[];
        layout(set = 0, binding = 9) buffer ubo9 { vec4 data; } Ubo9[];
        layout(set = 0, binding = 10) buffer ubo10 { vec4 data; } Ubo10[];
        layout(set = 0, binding = 11) buffer ubo11 { vec4 data; } Ubo11[];
        layout(set = 0, binding = 12) buffer ubo12 { vec4 data; } Ubo12[];
        layout(set = 0, binding = 13) buffer ubo13 { vec4 data; } Ubo13[];
        layout(set = 0, binding = 14) buffer ubo14 { vec4 data; } Ubo14[];
        layout(set = 0, binding = 15) buffer ubo15 { vec4 data; } Ubo15[];
        layout(set = 0, binding = 0) buffer ssbo0 { vec4 data; } Ssbo0[];
        layout(set = 0, binding = 1) buffer ssbo1 { vec4 data; } Ssbo1[];
        layout(set = 0, binding = 2) buffer ssbo2 { vec4 data; } Ssbo2[];
        layout(set = 0, binding = 3) buffer ssbo3 { vec4 data; } Ssbo3[];
        layout(set = 0, binding = 4) buffer ssbo4 { vec4 data; } Ssbo4[];
        layout(set = 0, binding = 5) buffer ssbo5 { vec4 data; } Ssbo5[];
        layout(set = 0, binding = 6) buffer ssbo6 { vec4 data; } Ssbo6[];
        layout(set = 0, binding = 7) buffer ssbo7 { vec4 data; } Ssbo7[];
        layout(set = 0, binding = 8) buffer ssbo8 { vec4 data; } Ssbo8[];
        layout(set = 0, binding = 9) buffer ssbo9 { vec4 data; } Ssbo9[];
        layout(set = 0, binding = 10) buffer ssbo10 { vec4 data; } Ssbo10[];
        layout(set = 0, binding = 11) buffer ssbo11 { vec4 data; } Ssbo11[];
        layout(set = 0, binding = 12) buffer ssbo12 { vec4 data; } Ssbo12[];
        layout(set = 0, binding = 13) buffer ssbo13 { vec4 data; } Ssbo13[];
        layout(set = 0, binding = 14) buffer ssbo14 { vec4 data; } Ssbo14[];
        layout(set = 0, binding = 15) buffer ssbo15 { vec4 data; } Ssbo15[];
        void main() {
            Ssbo0[0].data = Ubo0[0].data;
            Ssbo1[0].data = Ubo1[0].data;
            Ssbo2[0].data = Ubo2[0].data;
            Ssbo3[0].data = Ubo3[0].data;
            Ssbo4[0].data = Ubo4[0].data;
            Ssbo5[0].data = Ubo5[0].data;
            Ssbo6[0].data = Ubo6[0].data;
            Ssbo7[0].data = Ubo7[0].data;
            Ssbo8[0].data = Ubo8[0].data;
            Ssbo9[0].data = Ubo9[0].data;
            Ssbo10[0].data = Ubo10[0].data;
            Ssbo11[0].data = Ubo11[0].data;
            Ssbo12[0].data = Ubo12[0].data;
            Ssbo13[0].data = Ubo13[0].data;
            Ssbo14[0].data = Ubo14[0].data;
            Ssbo15[0].data = Ubo15[0].data;
        }
    )glsl";
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);

    VkDescriptorSetAndBindingMappingEXT mappings[32];
    for (uint32_t i = 0; i < 16; ++i) {
        mappings[i] = MakeSetAndBindingMapping(0, i, 1, VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT);
        if (i % 2 == 0) {
            mappings[i].source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
            mappings[i].sourceData.pushAddressOffset = i * 8u;
        } else {
            mappings[i].source = VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT;
            mappings[i].sourceData.indirectAddress.pushOffset = 0;
            mappings[i].sourceData.indirectAddress.addressOffset = i * 8u;
        }

        mappings[16 + i] = MakeSetAndBindingMapping(
            0, i, 1,
            VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT | VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT);
        if (i % 3 == 0) {
            mappings[16 + i].source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
            mappings[16 + i].sourceData.pushAddressOffset = i * 8u;
        } else {
            mappings[16 + i].source = VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT;
            mappings[16 + i].sourceData.indirectAddress.pushOffset = 0;
            mappings[16 + i].sourceData.indirectAddress.addressOffset = i * 8u;
        }
    }

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 32;
    mapping_info.pMappings = mappings;

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.cp_ci_.stage.pNext = &mapping_info;
    pipe.CreateComputePipeline(false);
}

TEST_F(PositiveDescriptorHeap, NonConstantMemoryAccess) {
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    VkDescriptorSetAndBindingMappingEXT mappings[2];
    mappings[0] = MakeSetAndBindingMapping(0, 0);
    mappings[0].source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mappings[0].sourceData.pushAddressOffset = 0u;
    mappings[1] = MakeSetAndBindingMapping(0, 1);
    mappings[1].source = VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT;
    mappings[1].sourceData.pushAddressOffset = 8u;

    VkShaderDescriptorSetAndBindingMappingInfoEXT mapping_info = vku::InitStructHelper();
    mapping_info.mappingCount = 2u;
    mapping_info.pMappings = mappings;

    char const* cs_source1 = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) buffer Data {
            uint index;
            uint data[];
        };
        void main() {
	        data[index] = 4u;
        }
    )glsl";

    char const* cs_source2 = R"glsl(
        #version 450
        layout(local_size_x = 1) in;
        layout(set = 0, binding = 0) uniform Index {
            uint index;
        };
        layout(set = 0, binding = 0) buffer Data {
            uint data[];
        } ssbos[];
        void main() {
	        ssbos[37].data[index] = 4u;
        }
    )glsl";

    for (uint32_t i = 0; i < 2; ++i) {
        VkShaderObj cs_module = VkShaderObj(*m_device, i == 0 ? cs_source1 : cs_source2, VK_SHADER_STAGE_COMPUTE_BIT);

        VkPipelineCreateFlags2CreateInfoKHR flags2_ci = vku::InitStructHelper();
        flags2_ci.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

        CreateComputePipelineHelper pipe(*this, &flags2_ci);
        pipe.cp_ci_.layout = VK_NULL_HANDLE;
        pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
        pipe.cp_ci_.stage.pNext = &mapping_info;
        pipe.CreateComputePipeline(false);
    }
}

TEST_F(PositiveDescriptorHeap, UntypedStorageImage) {
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    InitRenderTarget();

    const uint32_t buffer_index = 16u;

    const VkDeviceSize image_offset = 0u;
    const VkDeviceSize image_size = heap_props.imageDescriptorSize;
    const VkDeviceSize buffer_offset = heap_props.bufferDescriptorAlignment * buffer_index;
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

    VkDeviceAddressRangeEXT buffer_address_range = {buffer.Address(), buffer.CreateInfo().size};

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

// Currently crashing in latest NVIDIA driver (should have a fix, need to test)
// Confirms works on AMD/Intel Mesa
TEST_F(PositiveDescriptorHeap, DISABLED_SecondaryCmdBufferCompute) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = {buffer.Address(), 256};

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
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.CreateComputePipeline(false);

    uint32_t src_data = 4321u;

    VkPushDataInfoEXT push_data_info = vku::InitStructHelper();
    push_data_info.offset = 0u;
    push_data_info.data.size = sizeof(uint32_t);
    push_data_info.data.address = &src_data;

    VkPushConstantRange push_const_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)};
    VkPipelineLayoutCreateInfo pipeline_layout_info = vku::InitStructHelper();
    pipeline_layout_info.pushConstantRangeCount = 1u;
    pipeline_layout_info.pPushConstantRanges = &push_const_range;
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_info);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange.address = resource_heap_.Address();
    bind_resource_info.heapRange.size = resource_heap_.CreateInfo().size;
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
    vk::CmdPushDataEXT(secondary, &push_data_info);
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

TEST_F(PositiveDescriptorHeap, SecondaryCmdBufferGraphics) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());
    InitRenderTarget();

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);

    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = {buffer.Address(), 256};
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

    VkPushDataInfoEXT push_data_info = vku::InitStructHelper();
    push_data_info.offset = 0u;
    push_data_info.data.size = sizeof(uint32_t);
    push_data_info.data.address = &src_data;

    VkPushConstantRange push_const_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)};
    VkPipelineLayoutCreateInfo pipeline_layout_info = vku::InitStructHelper();
    pipeline_layout_info.pushConstantRangeCount = 1u;
    pipeline_layout_info.pPushConstantRanges = &push_const_range;
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_info);

    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange.address = resource_heap_.Address();
    bind_resource_info.heapRange.size = resource_heap_.CreateInfo().size;
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
    vk::CmdPushDataEXT(secondary, &push_data_info);
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

TEST_F(PositiveDescriptorHeap, HardcodedOffsetIntoStruct) {
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

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
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.CreateComputePipeline(false);

    uint32_t src_data = 4321u;

    VkPushDataInfoEXT push_data_info = vku::InitStructHelper();
    push_data_info.offset = 0u;
    push_data_info.data.size = sizeof(uint32_t);
    push_data_info.data.address = &src_data;

    VkPushConstantRange push_const_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)};
    VkPipelineLayoutCreateInfo pipeline_layout_info = vku::InitStructHelper();
    pipeline_layout_info.pushConstantRangeCount = 1u;
    pipeline_layout_info.pPushConstantRanges = &push_const_range;
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_info);

    m_command_buffer.Begin();
    vk::CmdPushConstants(m_command_buffer, pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &src_data);
    BindResourceHeap();
    vk::CmdPushDataEXT(m_command_buffer, &push_data_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* data = static_cast<uint32_t*>(buffer_b.Memory().Map());
        ASSERT_EQ(data[0], src_data);
    }
}

TEST_F(PositiveDescriptorHeap, SingleElementNoArray) {
    TEST_DESCRIPTION("GLSL only allows arrays of descriptor, force it to be a single element");
    AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize resource_stride = heap_props.bufferDescriptorSize;
    CreateResourceHeap(resource_stride);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR, vkt::device_address);
    VkHostAddressRangeEXT descriptor_host = {resource_heap_data_, static_cast<size_t>(resource_stride)};
    VkDeviceAddressRangeEXT device_range = {buffer.Address(), 256};
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
    VkShaderObj cs_module = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3, SPV_SOURCE_ASM);

    VkPipelineCreateFlags2CreateInfoKHR pipeline_create_flags_2_create_info = vku::InitStructHelper();
    pipeline_create_flags_2_create_info.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT;

    CreateComputePipelineHelper pipe(*this, &pipeline_create_flags_2_create_info);
    pipe.cp_ci_.layout = VK_NULL_HANDLE;
    pipe.cp_ci_.stage = cs_module.GetStageCreateInfo();
    pipe.CreateComputePipeline(false);

    uint32_t src_data = 4321u;

    VkPushDataInfoEXT push_data_info = vku::InitStructHelper();
    push_data_info.offset = 0u;
    push_data_info.data.size = sizeof(uint32_t);
    push_data_info.data.address = &src_data;

    VkPushConstantRange push_const_range = {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t)};
    VkPipelineLayoutCreateInfo pipeline_layout_info = vku::InitStructHelper();
    pipeline_layout_info.pushConstantRangeCount = 1u;
    pipeline_layout_info.pPushConstantRanges = &push_const_range;
    vkt::PipelineLayout pipeline_layout(*m_device, pipeline_layout_info);

    m_command_buffer.Begin();
    vk::CmdPushConstants(m_command_buffer, pipeline_layout.handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &src_data);
    BindResourceHeap();
    vk::CmdPushDataEXT(m_command_buffer, &push_data_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);

    if (!IsPlatformMockICD()) {
        uint32_t* data = static_cast<uint32_t*>(buffer.Memory().Map());
        ASSERT_EQ(data[0], src_data);
    }
}

TEST_F(PositiveDescriptorHeap, PartitionedAccelerationStructure) {
    TEST_DESCRIPTION("Test WriteResourceDescriptorsEXT with PTLAS descriptor type");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_NV_PARTITIONED_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::rayTracingPipeline);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::partitionedAccelerationStructure);
    RETURN_IF_SKIP(InitBasicDescriptorHeap());

    const VkDeviceSize descriptor_size =
        vk::GetPhysicalDeviceDescriptorSizeEXT(gpu_, VK_DESCRIPTOR_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_NV);
    std::vector<uint8_t> data(static_cast<size_t>(descriptor_size));

    // Create buffer with required usage flags for PTLAS
    vkt::Buffer ptlas_buffer(*m_device, 4096,
                             VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                             vkt::device_address);

    VkDeviceAddressRangeEXT device_address_range = {ptlas_buffer.Address(), descriptor_size};

    VkResourceDescriptorInfoEXT resource_info = vku::InitStructHelper();
    resource_info.type = VK_DESCRIPTOR_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_NV;
    resource_info.data.pAddressRange = &device_address_range;

    VkHostAddressRangeEXT descriptor = {data.data(), static_cast<size_t>(descriptor_size)};
    vk::WriteResourceDescriptorsEXT(*m_device, 1u, &resource_info, &descriptor);
}