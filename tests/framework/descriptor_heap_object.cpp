/*
 * Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "descriptor_heap_object.h"

#include "containers/container_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/ray_tracing_objects.h"
#include "utils/math_utils.h"

namespace vkt {

void DescriptorHeap::AddDescriptorHeapRequirements(VkLayerTest& test) {
    test.AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    test.AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    test.AddRequiredFeature(vkt::Feature::descriptorHeap);
}

void DescriptorHeap::AddUntypedDescriptorHeapRequirements(VkLayerTest &test) {
    test.AddRequiredExtensions(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
    test.AddRequiredExtensions(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
    test.AddRequiredFeature(vkt::Feature::shaderUntypedPointers);
    test.AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    test.AddRequiredFeature(vkt::Feature::descriptorHeap);
}

DescriptorHeap::DescriptorHeap(VkLayerTest& test) : test_(&test) {
    heap_props.pNext = &tensor_heap_props;
    test_->GetPhysicalDeviceProperties2(heap_props);
}

void DescriptorHeap::CreateResourceHeap(VkDeviceSize app_size, bool reserved_range_in_front) {
    resource_reserved_range_in_front_ = reserved_range_in_front;
    const VkDeviceSize heap_size = AlignResource(app_size + heap_props.minResourceHeapReservedRange);

    VkBufferUsageFlags2CreateInfo buffer_usage = vku::InitStructHelper();
    buffer_usage.usage = VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    resource_heap_.Init(*test_->DeviceObj(), vkt::Buffer::CreateInfo(heap_size, 0, {}, &buffer_usage),
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocate_flag_info);
    resource_heap_data_ = static_cast<uint8_t*>(resource_heap_.Memory().Map());
    if (reserved_range_in_front) {
        heap_offset_ = AlignResource(heap_props.minResourceHeapReservedRange);
    }
}

void DescriptorHeap::CreateSamplerHeap(VkDeviceSize app_size, bool reserved_range_in_front, bool use_embedded_samplers) {
    embedded_samplers = use_embedded_samplers;
    sampler_reserved_range_in_front_ = reserved_range_in_front;
    const VkDeviceSize reserved_range =
        (embedded_samplers ? heap_props.minSamplerHeapReservedRangeWithEmbedded : heap_props.minSamplerHeapReservedRange);
    const VkDeviceSize heap_size = AlignSampler(app_size + reserved_range);

    VkBufferUsageFlags2CreateInfo buffer_usage = vku::InitStructHelper();
    buffer_usage.usage = VK_BUFFER_USAGE_2_DESCRIPTOR_HEAP_BIT_EXT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    sampler_heap_.Init(*test_->DeviceObj(), vkt::Buffer::CreateInfo(heap_size, 0, {}, &buffer_usage),
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocate_flag_info);
    sampler_heap_data_ = static_cast<uint8_t*>(sampler_heap_.Memory().Map());
    if (reserved_range_in_front) {
        heap_offset_ = AlignSampler(reserved_range);
    }
}

VkDeviceSize DescriptorHeap::WriteBufferDescriptor(const vkt::Buffer& buffer, VkDescriptorType desc_type) {
    const VkDeviceAddressRangeKHR addr_range = buffer.AddressRange();
    return WriteBufferDescriptor(addr_range, desc_type);
}

VkDeviceSize DescriptorHeap::WriteBufferDescriptor(VkDeviceAddressRangeKHR addr_range, VkDescriptorType desc_type) {
    heap_offset_ = Align(heap_offset_, heap_props.bufferDescriptorAlignment);
    const VkDeviceSize write_offset = WriteBufferDescriptorAtOffset(addr_range, desc_type, heap_offset_);

    const VkDeviceSize buffer_desc_size = vk::GetPhysicalDeviceDescriptorSizeEXT(test_->DeviceObj()->Physical(), desc_type);
    heap_offset_ += buffer_desc_size;
    assert(heap_offset_ <= resource_heap_.CreateInfo().size);

    return write_offset;
}

VkDeviceSize DescriptorHeap::WriteBufferDescriptorAtOffset(VkDeviceAddressRangeKHR addr_range, VkDescriptorType desc_type,
                                                           VkDeviceSize heap_offset) {
    assert(IsValueIn(desc_type, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                 VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER}));
    return WriteDescriptorAtOffset(addr_range, desc_type, heap_offset);
}

VkDeviceSize DescriptorHeap::WriteAccelerationStructureDescriptor(vkt::as::AccelerationStructureKHR& as) {
    const VkDeviceAddress as_addr = as.GetAccelerationStructureDeviceAddress();
    return WriteAccelerationStructureDescriptor(as_addr);
}

VkDeviceSize DescriptorHeap::WriteAccelerationStructureDescriptor(VkDeviceAddress as_addr) {
    constexpr VkDescriptorType as_desc_type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    heap_offset_ = Align(heap_offset_, heap_props.bufferDescriptorAlignment);
    // Per the spec, size can be 0
    const VkDeviceAddressRangeEXT as_addr_range = {as_addr, 0};
    const VkDeviceSize write_offset = WriteDescriptorAtOffset(as_addr_range, as_desc_type, heap_offset_);

    const VkDeviceSize buffer_desc_size = vk::GetPhysicalDeviceDescriptorSizeEXT(test_->DeviceObj()->Physical(), as_desc_type);
    heap_offset_ += buffer_desc_size;
    assert(heap_offset_ <= resource_heap_.CreateInfo().size);

    return write_offset;
}

VkDeviceSize DescriptorHeap::AlignResource(VkDeviceSize offset) {
    VkDeviceSize aligned_offset = Align(offset, heap_props.bufferDescriptorAlignment);
    aligned_offset = Align(aligned_offset, heap_props.imageDescriptorAlignment);
    return aligned_offset;
}

VkDeviceSize DescriptorHeap::AlignSampler(VkDeviceSize offset) { return Align(offset, heap_props.samplerDescriptorAlignment); }

VkDeviceSize DescriptorHeap::GetResourceHeapReservedRangeOffset() const {
    if (resource_reserved_range_in_front_) {
        return 0;
    } else {
        return resource_heap_.CreateInfo().size - heap_props.minResourceHeapReservedRange;
    }
}

VkDeviceSize DescriptorHeap::GetSamplerHeapReservedRangeOffset() const {
    const VkDeviceSize min_reserved_range =
        embedded_samplers ? heap_props.minSamplerHeapReservedRangeWithEmbedded : heap_props.minSamplerHeapReservedRange;
    if (resource_reserved_range_in_front_) {
        return 0;
    } else {
        return sampler_heap_.CreateInfo().size - min_reserved_range;
    }
}

void DescriptorHeap::BindResourceHeap(vkt::CommandBuffer& cmd_buffer) {
    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = resource_heap_.AddressRange();
    bind_resource_info.reservedRangeOffset = GetResourceHeapReservedRangeOffset();
    bind_resource_info.reservedRangeSize = heap_props.minResourceHeapReservedRange;
    vk::CmdBindResourceHeapEXT(cmd_buffer, &bind_resource_info);
}

void DescriptorHeap::BindSamplerHeap(vkt::CommandBuffer &cmd_buffer) {
    const VkDeviceSize min_reserved_range =
        embedded_samplers ? heap_props.minSamplerHeapReservedRangeWithEmbedded : heap_props.minSamplerHeapReservedRange;
    VkBindHeapInfoEXT bind_resource_info = vku::InitStructHelper();
    bind_resource_info.heapRange = sampler_heap_.AddressRange();
    bind_resource_info.reservedRangeOffset = GetSamplerHeapReservedRangeOffset();
    bind_resource_info.reservedRangeSize = min_reserved_range;
    vk::CmdBindSamplerHeapEXT(cmd_buffer, &bind_resource_info);
}

VkDeviceSize DescriptorHeap::WriteDescriptorAtOffset(VkDeviceAddressRangeKHR addr_range, VkDescriptorType desc_type,
                                                     VkDeviceSize heap_offset) {
    assert(resource_heap_.handle() != VK_NULL_HANDLE);
    VkResourceDescriptorInfoEXT desc_info = vku::InitStructHelper();
    desc_info.type = desc_type;
    desc_info.data.pAddressRange = &addr_range;

    const VkDeviceSize desc_size = vk::GetPhysicalDeviceDescriptorSizeEXT(test_->DeviceObj()->Physical(), desc_type);

    VkHostAddressRangeEXT desc_host_data{};
    desc_host_data.address = resource_heap_data_ + heap_offset;
    desc_host_data.size = desc_size;

    vk::WriteResourceDescriptorsEXT(*test_->DeviceObj(), 1, &desc_info, &desc_host_data);

    return heap_offset;
}

}  // namespace vkt
