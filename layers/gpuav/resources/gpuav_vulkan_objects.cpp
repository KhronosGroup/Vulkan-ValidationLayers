/* Copyright (c) 2018-2025 The Khronos Group Inc.
 * Copyright (c) 2018-2025 Valve Corporation
 * Copyright (c) 2018-2025 LunarG, Inc.
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

#include "gpuav/resources/gpuav_vulkan_objects.h"

#include "gpuav/core/gpuav.h"
#include "generated/dispatch_functions.h"
#include "utils/math_utils.h"
#include <vulkan/utility/vk_struct_helper.hpp>

namespace gpuav {
namespace vko {

// Implementation for Descriptor Set Manager class
DescriptorSetManager::DescriptorSetManager(VkDevice device, uint32_t num_bindings_in_set)
    : device(device), num_bindings_in_set(num_bindings_in_set) {}

DescriptorSetManager::~DescriptorSetManager() {
    for (auto &pool : desc_pool_map_) {
        DispatchDestroyDescriptorPool(device, pool.first, nullptr);
    }
    desc_pool_map_.clear();
}

VkResult DescriptorSetManager::GetDescriptorSet(VkDescriptorPool *out_desc_pool, VkDescriptorSetLayout ds_layout,
                                                VkDescriptorSet *out_desc_sets) {
    std::vector<VkDescriptorSet> desc_sets;
    VkResult result = GetDescriptorSets(1, out_desc_pool, ds_layout, &desc_sets);
    assert(result == VK_SUCCESS);
    if (result == VK_SUCCESS) {
        *out_desc_sets = desc_sets[0];
    }
    return result;
}

VkResult DescriptorSetManager::GetDescriptorSets(uint32_t count, VkDescriptorPool *out_pool, VkDescriptorSetLayout ds_layout,
                                                 std::vector<VkDescriptorSet> *out_desc_sets) {
    auto guard = Lock();

    VkResult result = VK_SUCCESS;
    VkDescriptorPool desc_pool_to_use = VK_NULL_HANDLE;

    assert(count > 0);
    if (count == 0) {
        return result;
    }
    out_desc_sets->clear();
    out_desc_sets->resize(count);

    for (auto &[desc_pool, pool_tracker] : desc_pool_map_) {
        if (pool_tracker.used + count < pool_tracker.size) {
            desc_pool_to_use = desc_pool;
            break;
        }
    }
    if (desc_pool_to_use == VK_NULL_HANDLE) {
        constexpr uint32_t kDefaultMaxSetsPerPool = 512;
        const uint32_t max_sets = std::max(kDefaultMaxSetsPerPool, count);

        // TODO: The logic to compute descriptor pool sizes should not be
        // hardcoded like so, should be dynamic depending on the descriptor sets
        // to be created. Not too dramatic as Vulkan will gracefully fail if there is a
        // mismatch between this and created descriptor sets.
        const std::array<VkDescriptorPoolSize, 2> pool_sizes = {{{
                                                                     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                                                     max_sets * num_bindings_in_set,
                                                                 },
                                                                 {
                                                                     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
                                                                     max_sets * num_bindings_in_set,
                                                                 }}};

        VkDescriptorPoolCreateInfo desc_pool_info = vku::InitStructHelper();
        desc_pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        desc_pool_info.maxSets = max_sets;
        desc_pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        desc_pool_info.pPoolSizes = pool_sizes.data();
        result = DispatchCreateDescriptorPool(device, &desc_pool_info, nullptr, &desc_pool_to_use);
        assert(result == VK_SUCCESS);
        if (result != VK_SUCCESS) {
            return result;
        }
        desc_pool_map_[desc_pool_to_use].size = desc_pool_info.maxSets;
        desc_pool_map_[desc_pool_to_use].used = 0;
    }

    std::vector<VkDescriptorSetLayout> desc_layouts(count, ds_layout);
    VkDescriptorSetAllocateInfo desc_set_alloc_info = vku::InitStructHelper();
    desc_set_alloc_info.descriptorPool = desc_pool_to_use;
    desc_set_alloc_info.descriptorSetCount = count;
    desc_set_alloc_info.pSetLayouts = desc_layouts.data();
    result = DispatchAllocateDescriptorSets(device, &desc_set_alloc_info, out_desc_sets->data());
    assert(result == VK_SUCCESS);
    if (result != VK_SUCCESS) {
        return result;
    }

    *out_pool = desc_pool_to_use;
    desc_pool_map_[desc_pool_to_use].used += count;

    return result;
}

void DescriptorSetManager::PutBackDescriptorSet(VkDescriptorPool desc_pool, VkDescriptorSet desc_set) {
    auto guard = Lock();

    auto iter = desc_pool_map_.find(desc_pool);
    assert(iter != desc_pool_map_.end());
    if (iter == desc_pool_map_.end()) {
        return;
    }

    VkResult result = DispatchFreeDescriptorSets(device, desc_pool, 1, &desc_set);
    assert(result == VK_SUCCESS);
    if (result != VK_SUCCESS) {
        return;
    }
    desc_pool_map_[desc_pool].used--;
    if (desc_pool_map_[desc_pool].used == 0) {
        DispatchDestroyDescriptorPool(device, desc_pool, nullptr);
        desc_pool_map_.erase(desc_pool);
    }

    return;
}

void SharedResourcesCache::Clear() {
    for (auto &[key, value] : shared_validation_resources_map_) {
        auto &[object, destructor] = value;
        destructor(object);
    }
    shared_validation_resources_map_.clear();
}

void *Buffer::GetMappedPtr() const { return mapped_ptr; }

void Buffer::FlushAllocation(VkDeviceSize offset, VkDeviceSize size) const {
    VkResult result = vmaFlushAllocation(gpuav.vma_allocator_, allocation, offset, size);
    if (result != VK_SUCCESS) {
        gpuav.InternalVmaError(gpuav.device, result, "Unable to flush device memory.");
    }
}

void Buffer::InvalidateAllocation(VkDeviceSize offset, VkDeviceSize size) const {
    VkResult result = vmaInvalidateAllocation(gpuav.vma_allocator_, allocation, offset, size);
    if (result != VK_SUCCESS) {
        gpuav.InternalVmaError(gpuav.device, result, "Unable to invalidate device memory.");
    }
}

bool Buffer::Create(const VkBufferCreateInfo *buffer_create_info, const VmaAllocationCreateInfo *allocation_create_info) {
    VkResult result =
        vmaCreateBuffer(gpuav.vma_allocator_, buffer_create_info, allocation_create_info, &buffer, &allocation, nullptr);
    if (result != VK_SUCCESS) {
        gpuav.InternalVmaError(gpuav.device, result, "Unable to allocate device memory for internal buffer.");
        return false;
    }
    size = buffer_create_info->size;

    if (buffer_create_info->usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        // After creating the buffer, get the address right away
        device_address = gpuav.device_state->GetBufferDeviceAddressHelper(buffer, &gpuav.modified_extensions);
        if (device_address == 0) {
            gpuav.InternalVmaError(gpuav.device, VK_ERROR_UNKNOWN, "Failed to get address with DispatchGetBufferDeviceAddress.");
            return false;
        }
    }
    if ((allocation_create_info->requiredFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) ||
        (allocation_create_info->flags & VMA_ALLOCATION_CREATE_MAPPED_BIT)) {
        result = vmaMapMemory(gpuav.vma_allocator_, allocation, &mapped_ptr);
        if (result != VK_SUCCESS) {
            mapped_ptr = nullptr;
            gpuav.InternalVmaError(gpuav.device, result, "Unable to map device memory.");
            return false;
        }
    }
    return true;
}

void Buffer::Destroy() {
    if (buffer != VK_NULL_HANDLE) {
        if (mapped_ptr != nullptr) {
            vmaUnmapMemory(gpuav.vma_allocator_, allocation);
            mapped_ptr = nullptr;
        }
        vmaDestroyBuffer(gpuav.vma_allocator_, buffer, allocation);
        buffer = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
        device_address = 0;
    }
}

void Buffer::Clear() const {
    // Caller is in charge of calling Flush/Invalidate as needed
    assert(mapped_ptr);
    memset((uint8_t *)mapped_ptr, 0, static_cast<size_t>(size));
}

GpuResourcesManager::GpuResourcesManager(Validator &gpuav) : gpuav_(gpuav) {
    {
        VmaAllocationCreateInfo alloc_ci = {};
        alloc_ci.usage = VMA_MEMORY_USAGE_AUTO;
        alloc_ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        host_visible_buffer_cache_.Create(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, alloc_ci);
    }

    {
        VmaAllocationCreateInfo alloc_ci = {};
        alloc_ci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        device_local_indirect_buffer_cache_.Create(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                                   alloc_ci);
    }
}

VkDescriptorSet GpuResourcesManager::GetManagedDescriptorSet(VkDescriptorSetLayout desc_set_layout) {
    // Look for a descriptor set layout matching input,
    // if found get or add an associated descriptor set
    for (LayoutToSets &layout_to_sets : cache_layouts_to_sets_) {
        if (layout_to_sets.desc_set_layout != desc_set_layout) {
            continue;
        }

        if (layout_to_sets.first_available_desc_set == layout_to_sets.cached_descriptors.size()) {
            CachedDescriptor cached_descriptor;
            const VkResult result = gpuav_.desc_set_manager_->GetDescriptorSet(&cached_descriptor.desc_pool, desc_set_layout,
                                                                               &cached_descriptor.desc_set);
            if (result != VK_SUCCESS) {
                return VK_NULL_HANDLE;
            }
            layout_to_sets.cached_descriptors.emplace_back(cached_descriptor);
        }

        assert(layout_to_sets.first_available_desc_set < layout_to_sets.cached_descriptors.size());
        return layout_to_sets.cached_descriptors[layout_to_sets.first_available_desc_set++].desc_set;
    }

    // Did not find input descriptor set layout,
    // add a new cache entry and just re-run search
    LayoutToSets layout_to_sets;
    layout_to_sets.desc_set_layout = desc_set_layout;
    cache_layouts_to_sets_.emplace_back(layout_to_sets);
    return GetManagedDescriptorSet(desc_set_layout);
}

vko::BufferRange GpuResourcesManager::GetHostVisibleBufferRange(VkDeviceSize size) {
    // Kind of arbitrary, considered "big enough"
    constexpr VkDeviceSize min_buffer_block_size = 4 * 1024;
    // Buffers are used as storage buffers, align to corresponding limit
    const VkDeviceSize alignment = gpuav_.phys_dev_props.limits.minStorageBufferOffsetAlignment;
    return host_visible_buffer_cache_.GetBufferRange(gpuav_, size, alignment, min_buffer_block_size);
}

vko::BufferRange GpuResourcesManager::GetDeviceLocalIndirectBufferRange(VkDeviceSize size) {
    // Kind of arbitrary, considered "big enough"
    constexpr VkDeviceSize min_buffer_block_size = 4 * 1024;
    // Buffers are used as storage buffers, align to corresponding limit
    const VkDeviceSize alignment = gpuav_.phys_dev_props.limits.minStorageBufferOffsetAlignment;
    return device_local_indirect_buffer_cache_.GetBufferRange(gpuav_, size, alignment, min_buffer_block_size);
}

void GpuResourcesManager::ReturnResources() {
    for (LayoutToSets &layout_to_set : cache_layouts_to_sets_) {
        layout_to_set.first_available_desc_set = 0;
    }

    host_visible_buffer_cache_.ReturnBuffers();
    device_local_indirect_buffer_cache_.ReturnBuffers();
}

void GpuResourcesManager::DestroyResources() {
    for (LayoutToSets &layout_to_set : cache_layouts_to_sets_) {
        for (CachedDescriptor &cached_descriptor : layout_to_set.cached_descriptors) {
            gpuav_.desc_set_manager_->PutBackDescriptorSet(cached_descriptor.desc_pool, cached_descriptor.desc_set);
        }
        layout_to_set.cached_descriptors.clear();
    }
    cache_layouts_to_sets_.clear();

    host_visible_buffer_cache_.DestroyBuffers();
    device_local_indirect_buffer_cache_.DestroyBuffers();
}

void GpuResourcesManager::BufferCache::Create(VkBufferUsageFlags buffer_usage_flags, const VmaAllocationCreateInfo allocation_ci) {
    buffer_usage_flags_ = buffer_usage_flags;
    allocation_ci_ = allocation_ci;
}

GpuResourcesManager::BufferCache::~BufferCache() { DestroyBuffers(); }

vko::BufferRange GpuResourcesManager::BufferCache::GetBufferRange(Validator &gpuav, VkDeviceSize byte_size, VkDeviceSize alignment,
                                                                  VkDeviceSize min_buffer_block_byte_size) {
    // Try to find a cached buffer block big enough to sub-allocate from it
    if (total_available_byte_size_ >= byte_size) {
        for (size_t i = 0; i < cached_buffers_blocks_.size(); ++i) {
            const size_t cached_buffer_i = (next_avail_buffer_pos_hint_ + i) % cached_buffers_blocks_.size();
            CachedBufferBlock &cached_buffer = cached_buffers_blocks_[cached_buffer_i];

            // Is there enough space in the current cached buffer to fit the aligned sub-allocation?
            const VkDeviceSize aligned_free_range_begin = Align(cached_buffer.used_range.end, alignment);
            const vvl::range<VkDeviceSize> aligned_free_range = {aligned_free_range_begin, cached_buffer.total_range.end};
            if (aligned_free_range.non_empty() && aligned_free_range.size() >= byte_size) {
                // There is enough space, sub-allocate
                const vvl::range<VkDeviceSize> returned_range = {aligned_free_range_begin, aligned_free_range_begin + byte_size};
                assert(returned_range.non_empty());
                const vvl::range<VkDeviceSize> pad_range = {cached_buffer.used_range.end, aligned_free_range.begin};
                assert(pad_range.valid());
                total_available_byte_size_ -= returned_range.size() + pad_range.size();

                cached_buffer.used_range.end = returned_range.end;

                // Heuristic: next call to the cache will ask for the same size and alignment.
                // => If current block is big enough, hint at it. Else, hint at next block.
                const vvl::range<VkDeviceSize> available_aligned_byte_range = {Align(cached_buffer.used_range.end, alignment),
                                                                               cached_buffer.total_range.end};
                if (available_aligned_byte_range.non_empty() && available_aligned_byte_range.size() >= byte_size) {
                    next_avail_buffer_pos_hint_ = cached_buffer_i;
                } else {
                    next_avail_buffer_pos_hint_ = (cached_buffer_i + 1) % cached_buffers_blocks_.size();
                }
                uint8_t *offset_mapped_ptr = nullptr;
                if (cached_buffer.buffer.GetMappedPtr()) {
                    offset_mapped_ptr = (uint8_t *)cached_buffer.buffer.GetMappedPtr() + returned_range.begin;
                }

                return {cached_buffer.buffer.VkHandle(), returned_range.begin, returned_range.size(), offset_mapped_ptr};
            }
        }
    }

    // Did not find a cached buffer, create one, cache it and return its handle
    Buffer buffer(gpuav);
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = std::max(min_buffer_block_byte_size, byte_size);
    buffer_ci.usage = buffer_usage_flags_;
    const bool success = buffer.Create(&buffer_ci, &allocation_ci_);
    if (!success) {
        return {VK_NULL_HANDLE, 0, 0, nullptr};
    }
    CachedBufferBlock cached_buffer_block{buffer, {0, buffer_ci.size}, {0, byte_size}};
    cached_buffers_blocks_.emplace_back(cached_buffer_block);

    total_available_byte_size_ += buffer_ci.size;

    return {buffer.VkHandle(), cached_buffer_block.used_range.begin, cached_buffer_block.used_range.size(),
            cached_buffer_block.buffer.GetMappedPtr()};
}

void GpuResourcesManager::BufferCache::ReturnBuffers() {
    total_available_byte_size_ = 0;
    for (CachedBufferBlock &cached_buffer_block : cached_buffers_blocks_) {
        cached_buffer_block.used_range = {0, cached_buffer_block.total_range.end};
        total_available_byte_size_ += cached_buffer_block.total_range.size();
    }
}

void GpuResourcesManager::BufferCache::DestroyBuffers() {
    for (CachedBufferBlock &cached_buffer_block : cached_buffers_blocks_) {
        cached_buffer_block.buffer.Destroy();
    }
    cached_buffers_blocks_.clear();
}

}  // namespace vko
}  // namespace gpuav
