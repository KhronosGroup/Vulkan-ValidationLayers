/* Copyright (c) 2018-2023 The Khronos Group Inc.
 * Copyright (c) 2018-2023 Valve Corporation
 * Copyright (c) 2018-2023 LunarG, Inc.
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

#include "gpu_subclasses.h"
#include "gpu_validation.h"

gpuav_state::DescriptorHeap::DescriptorHeap(GpuAssisted &gpu_dev, uint32_t max_descriptors)
    : max_descriptors_(max_descriptors), allocator_(gpu_dev.vmaAllocator) {

     // If max_descriptors_ is 0, GPU-AV aborted during vkCreateDevice(). We still need to
     // support calls into this class as no-ops if this happens.
     if (max_descriptors_ == 0) {
         return;
     }

    VkBufferCreateInfo buffer_info = vku::InitStruct<VkBufferCreateInfo>();
    buffer_info.size = ((max_descriptors_ + 31) & ~31) * sizeof(uint32_t);
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    [[maybe_unused]] VkResult result;
    result = vmaCreateBuffer(allocator_, &buffer_info, &alloc_info, &buffer_, &allocation_, nullptr);
    assert(result == VK_SUCCESS);

    result = vmaMapMemory(allocator_, allocation_, reinterpret_cast<void **>(&gpu_heap_state_));
    assert(result == VK_SUCCESS);
    memset(gpu_heap_state_, 0, static_cast<size_t>(buffer_info.size));

    auto buffer_device_address_info = vku::InitStruct<VkBufferDeviceAddressInfo>();
    buffer_device_address_info.buffer = buffer_;
    // We cannot rely on device_extensions here, since we may be enabling BDA support even
    // though the application has not requested it.
    if (gpu_dev.api_version >= VK_API_VERSION_1_2) {
        device_address_ = DispatchGetBufferDeviceAddress(gpu_dev.device, &buffer_device_address_info);
    } else {
        device_address_ = DispatchGetBufferDeviceAddressKHR(gpu_dev.device, &buffer_device_address_info);
    }
    assert(device_address_ != 0);
}

gpuav_state::DescriptorHeap::~DescriptorHeap() {
    if (max_descriptors_ > 0) {
        vmaUnmapMemory(allocator_, allocation_);
        gpu_heap_state_ = nullptr;
        vmaDestroyBuffer(allocator_, buffer_, allocation_);
    }
}

gpuav_state::DescriptorId gpuav_state::DescriptorHeap::NextId(const VulkanTypedHandle &handle) {
    if (max_descriptors_ == 0) {
        return 0;
    }
    gpuav_state::DescriptorId result;

    auto guard = Lock();
    assert(alloc_map_.size() < max_descriptors_);
    do {
        result = next_id_++;
        if (next_id_ == max_descriptors_) {
            next_id_ = 1;
            result = next_id_;
        }
    } while (alloc_map_.count(result) > 0);
    alloc_map_[result] = handle;
    gpu_heap_state_[result/32] |= 1u << (result & 31);
    return result;
}

void gpuav_state::DescriptorHeap::DeleteId(gpuav_state::DescriptorId id) {
    if (max_descriptors_ > 0) {
        auto guard = Lock();
        // Note: We don't mess with next_id_ here because ids should be signed in LRU order.
        gpu_heap_state_[id/32] &= ~(1u << (id & 31));
        alloc_map_.erase(id);
    }
}

gpuav_state::Buffer::Buffer(ValidationStateTracker *dev_data, VkBuffer buff, const VkBufferCreateInfo *pCreateInfo,
                            DescriptorHeap &desc_heap_)
    : BUFFER_STATE(dev_data, buff, pCreateInfo),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(buff, kVulkanObjectTypeBuffer))) {}

void gpuav_state::Buffer::Destroy() {
    desc_heap.DeleteId(id);
    BUFFER_STATE::Destroy();
}

void gpuav_state::Buffer::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    BUFFER_STATE::NotifyInvalidate(invalid_nodes, unlink);
}

gpuav_state::BufferView::BufferView(const std::shared_ptr<BUFFER_STATE> &bf, VkBufferView bv, const VkBufferViewCreateInfo *ci,
                                    VkFormatFeatureFlags2KHR buf_ff, DescriptorHeap &desc_heap_)
    : BUFFER_VIEW_STATE(bf, bv, ci, buf_ff),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(bv, kVulkanObjectTypeBufferView))) {}

void gpuav_state::BufferView::Destroy() {
    desc_heap.DeleteId(id);
    BUFFER_VIEW_STATE::Destroy();
}

void gpuav_state::BufferView::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    BUFFER_VIEW_STATE::NotifyInvalidate(invalid_nodes, unlink);
}

gpuav_state::ImageView::ImageView(const std::shared_ptr<IMAGE_STATE> &image_state, VkImageView iv, const VkImageViewCreateInfo *ci,
                                  VkFormatFeatureFlags2KHR ff, const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props,
                                  DescriptorHeap &desc_heap_)
    : IMAGE_VIEW_STATE(image_state, iv, ci, ff, cubic_props),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(iv, kVulkanObjectTypeImageView))) {}

void gpuav_state::ImageView::Destroy() {
    desc_heap.DeleteId(id);
    IMAGE_VIEW_STATE::Destroy();
}

void gpuav_state::ImageView::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    IMAGE_VIEW_STATE::NotifyInvalidate(invalid_nodes, unlink);
}

gpuav_state::Sampler::Sampler(const VkSampler s, const VkSamplerCreateInfo *pci, DescriptorHeap &desc_heap_)
    : SAMPLER_STATE(s, pci), desc_heap(desc_heap_), id(desc_heap.NextId(VulkanTypedHandle(s, kVulkanObjectTypeSampler))) {}

void gpuav_state::Sampler::Destroy() {
    desc_heap.DeleteId(id);
    SAMPLER_STATE::Destroy();
}

void gpuav_state::Sampler::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    SAMPLER_STATE::NotifyInvalidate(invalid_nodes, unlink);
}

gpuav_state::AccelerationStructureKHR::AccelerationStructureKHR(VkAccelerationStructureKHR as,
                                                                const VkAccelerationStructureCreateInfoKHR *ci,
                                                                std::shared_ptr<BUFFER_STATE> &&buf_state, VkDeviceAddress address,
                                                                DescriptorHeap &desc_heap_)
    : ACCELERATION_STRUCTURE_STATE_KHR(as, ci, std::move(buf_state), address),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(as, kVulkanObjectTypeAccelerationStructureKHR))) {}

void gpuav_state::AccelerationStructureKHR::Destroy() {
    desc_heap.DeleteId(id);
    ACCELERATION_STRUCTURE_STATE_KHR::Destroy();
}

void gpuav_state::AccelerationStructureKHR::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    ACCELERATION_STRUCTURE_STATE_KHR::NotifyInvalidate(invalid_nodes, unlink);
}

gpuav_state::AccelerationStructureNV::AccelerationStructureNV(VkDevice device, VkAccelerationStructureNV as,
                                                              const VkAccelerationStructureCreateInfoNV *ci,
                                                              DescriptorHeap &desc_heap_)
    : ACCELERATION_STRUCTURE_STATE_NV(device, as, ci),
      desc_heap(desc_heap_),
      id(desc_heap.NextId(VulkanTypedHandle(as, kVulkanObjectTypeAccelerationStructureNV))) {}

void gpuav_state::AccelerationStructureNV::Destroy() {
    desc_heap.DeleteId(id);
    ACCELERATION_STRUCTURE_STATE_NV::Destroy();
}

void gpuav_state::AccelerationStructureNV::NotifyInvalidate(const NodeList &invalid_nodes, bool unlink) {
    desc_heap.DeleteId(id);
    ACCELERATION_STRUCTURE_STATE_NV::NotifyInvalidate(invalid_nodes, unlink);
}
