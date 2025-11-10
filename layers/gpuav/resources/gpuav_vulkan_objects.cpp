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
#include <mutex>
#include <vulkan/utility/vk_struct_helper.hpp>

#include "profiling/profiling.h"

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
    std::lock_guard guard(lock_);

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
    std::lock_guard guard(lock_);

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

    VkMemoryPropertyFlags mem_prop_flags = {};
    vmaGetAllocationMemoryProperties(gpuav.vma_allocator_, allocation, &mem_prop_flags);
    if (mem_prop_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        result = vmaMapMemory(gpuav.vma_allocator_, allocation, &mapped_ptr);
        if (result != VK_SUCCESS) {
            mapped_ptr = nullptr;
            gpuav.InternalVmaError(gpuav.device, result, "Unable to map device memory.");
            return false;
        }
    }
#if defined(VVL_TRACY_GPU)
    static_assert(VK_MAX_MEMORY_HEAPS <= 16u);
    VmaBudget budgets[VK_MAX_MEMORY_HEAPS] = {};
    vmaGetHeapBudgets(gpuav.vma_allocator_, budgets);
    constexpr std::array<const char *, VK_MAX_MEMORY_HEAPS> heap_names = {
        {"GPU Heap 0 (kB)", "GPU Heap 1 (kB)", "GPU Heap 2 (kB)", "GPU Heap 3 (kB)", "GPU Heap 4 (kB)", "GPU Heap 5 (kB)",
         "GPU Heap 6 (kB)", "GPU Heap 7 (kB)", "GPU Heap 8 (kB)", "GPU Heap 9 (kB)", "GPU Heap 10 (kB)", "GPU Heap 11 (kB)",
         "GPU Heap 12 (kB)", "GPU Heap 13 (kB)", "GPU Heap 14 (kB)", "GPU Heap 15 (kB)"}};
    for (uint32_t heap_i = 0; heap_i < gpuav.phys_dev_mem_props.memoryHeapCount; ++heap_i) {
        VVL_TracyPlot(heap_names[heap_i], int64_t(budgets[heap_i].statistics.blockBytes / 1024));
    }
#endif

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
#if defined(VVL_TRACY_GPU)
    static_assert(VK_MAX_MEMORY_HEAPS <= 16u);
    VmaBudget budgets[VK_MAX_MEMORY_HEAPS] = {};
    vmaGetHeapBudgets(gpuav.vma_allocator_, budgets);
    constexpr std::array<const char *, VK_MAX_MEMORY_HEAPS> heap_names = {
        {"GPU Heap 0 (kB)", "GPU Heap 1 (kB)", "GPU Heap 2 (kB)", "GPU Heap 3 (kB)", "GPU Heap 4 (kB)", "GPU Heap 5 (kB)",
         "GPU Heap 6 (kB)", "GPU Heap 7 (kB)", "GPU Heap 8 (kB)", "GPU Heap 9 (kB)", "GPU Heap 10 (kB)", "GPU Heap 11 (kB)",
         "GPU Heap 12 (kB)", "GPU Heap 13 (kB)", "GPU Heap 14 (kB)", "GPU Heap 15 (kB)"}};
    for (uint32_t heap_i = 0; heap_i < gpuav.phys_dev_mem_props.memoryHeapCount; ++heap_i) {
        VVL_TracyPlot(heap_names[heap_i], int64_t(budgets[heap_i].statistics.blockBytes / 1024));
    }
#endif
}

void Buffer::Clear() const {
    // Caller is in charge of calling Flush/Invalidate as needed
    assert(mapped_ptr);
    memset((uint8_t *)mapped_ptr, 0, static_cast<size_t>(size));
}

void BufferRange::Clear() const {
    // Caller is in charge of calling Flush/Invalidate as needed
    assert(offset_mapped_ptr);
    memset((uint8_t *)offset_mapped_ptr, 0, static_cast<size_t>(size));
}

GpuResourcesManager::GpuResourcesManager(Validator &gpuav) : gpuav_(gpuav) {
    const bool force_host_access = gpuav.IsAllDeviceLocalMappable();

    {
        VmaAllocationCreateInfo alloc_ci = {};
        alloc_ci.usage = VMA_MEMORY_USAGE_AUTO;
        alloc_ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        alloc_ci.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        buffer_caches_.host_coherent.Create(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                                VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                            alloc_ci);
    }

    {
        VmaAllocationCreateInfo alloc_ci = {};
        alloc_ci.usage = VMA_MEMORY_USAGE_AUTO;
        alloc_ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        buffer_caches_.host_cached.Create(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                          alloc_ci);
    }

    {
        VmaAllocationCreateInfo alloc_ci = {};
        alloc_ci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        if (force_host_access) {
            alloc_ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        }
        buffer_caches_.device_local.Create(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                               VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                           alloc_ci);
    }

    {
        VmaAllocationCreateInfo alloc_ci = {};
        alloc_ci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        if (force_host_access) {
            alloc_ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        }
        buffer_caches_.device_local_indirect.Create(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                                    alloc_ci);
    }

    {
        VmaAllocationCreateInfo alloc_ci = {};
        alloc_ci.usage = VMA_MEMORY_USAGE_AUTO;
        alloc_ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT |
                         VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        buffer_caches_.staging.Create(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                          VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
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

// Arbitrary, big enough
constexpr VkDeviceSize buffer_address_alignment = 128;

vko::BufferRange GpuResourcesManager::GetHostCoherentBufferRange(VkDeviceSize size) {
    // Kind of arbitrary, considered "big enough"
    constexpr VkDeviceSize min_buffer_block_size = 4 * 1024;
    // Buffers are used as storage buffers, align to corresponding limit
    const VkDeviceSize alignment =
        std::max<VkDeviceSize>(gpuav_.phys_dev_props.limits.minStorageBufferOffsetAlignment, buffer_address_alignment);
    return buffer_caches_.host_coherent.GetBufferRange(gpuav_, size, alignment, min_buffer_block_size);
}

vko::BufferRange GpuResourcesManager::GetHostCachedBufferRange(VkDeviceSize size) {
    // Kind of arbitrary, considered "big enough"
    constexpr VkDeviceSize min_buffer_block_size = 4 * 1024;
    // Buffers are used as storage buffers, align to corresponding limit
    const VkDeviceSize alignment =
        std::max<VkDeviceSize>(gpuav_.phys_dev_props.limits.minStorageBufferOffsetAlignment, buffer_address_alignment);
    return buffer_caches_.host_cached.GetBufferRange(gpuav_, size, alignment, min_buffer_block_size);
}

// Only used for Host Cache
void GpuResourcesManager::FlushAllocation(const vko::BufferRange &buffer_range) {
    vmaFlushAllocation(gpuav_.vma_allocator_, buffer_range.vma_alloc, 0, VK_WHOLE_SIZE);
}

// Only used for Host Cache
void GpuResourcesManager::InvalidateAllocation(const vko::BufferRange &buffer_range) {
    vmaInvalidateAllocation(gpuav_.vma_allocator_, buffer_range.vma_alloc, 0, VK_WHOLE_SIZE);
}

vko::BufferRange GpuResourcesManager::GetDeviceLocalBufferRange(VkDeviceSize size) {
    // Kind of arbitrary, considered "big enough"
    constexpr VkDeviceSize min_buffer_block_size = 4 * 1024;
    // Buffers are used as storage buffers, align to corresponding limit
    const VkDeviceSize alignment =
        std::max<VkDeviceSize>(gpuav_.phys_dev_props.limits.minStorageBufferOffsetAlignment, buffer_address_alignment);
    return buffer_caches_.device_local.GetBufferRange(gpuav_, size, alignment, min_buffer_block_size);
}

vko::BufferRange GpuResourcesManager::GetDeviceLocalIndirectBufferRange(VkDeviceSize size) {
    // Kind of arbitrary, considered "big enough"
    constexpr VkDeviceSize min_buffer_block_size = 4 * 1024;
    // Buffers are used as storage buffers, align to corresponding limit
    const VkDeviceSize alignment =
        std::max<VkDeviceSize>(gpuav_.phys_dev_props.limits.minStorageBufferOffsetAlignment, buffer_address_alignment);
    return buffer_caches_.device_local_indirect.GetBufferRange(gpuav_, size, alignment, min_buffer_block_size);
}

vko::BufferRange GpuResourcesManager::GetStagingBufferRange(VkDeviceSize size) {
    // Kind of arbitrary, considered "big enough"
    constexpr VkDeviceSize min_buffer_block_size = 4 * 1024;
    // Buffers are used as storage buffers, align to corresponding limit
    const VkDeviceSize alignment =
        std::max<VkDeviceSize>(gpuav_.phys_dev_props.limits.minStorageBufferOffsetAlignment, buffer_address_alignment);
    return buffer_caches_.staging.GetBufferRange(gpuav_, size, alignment, min_buffer_block_size);
}

void GpuResourcesManager::ReturnResources() {
    for (LayoutToSets &layout_to_set : cache_layouts_to_sets_) {
        layout_to_set.first_available_desc_set = 0;
    }

    buffer_caches_.ReturnBuffers();
}

void GpuResourcesManager::DestroyResources() {
    for (LayoutToSets &layout_to_set : cache_layouts_to_sets_) {
        for (CachedDescriptor &cached_descriptor : layout_to_set.cached_descriptors) {
            gpuav_.desc_set_manager_->PutBackDescriptorSet(cached_descriptor.desc_pool, cached_descriptor.desc_set);
        }
        layout_to_set.cached_descriptors.clear();
    }
    cache_layouts_to_sets_.clear();

    buffer_caches_.DestroyBuffers();
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
                VkDeviceAddress offset_address = 0;
                if (cached_buffer.buffer.Address()) {
                    offset_address = cached_buffer.buffer.Address() + returned_range.begin;
                }

                return {cached_buffer.buffer.VkHandle(),
                        returned_range.begin,
                        returned_range.size(),
                        offset_mapped_ptr,
                        offset_address,
                        cached_buffer.buffer.Allocation()};
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
        return {};
    }
    CachedBufferBlock cached_buffer_block{buffer, {0, buffer_ci.size}, {0, byte_size}};
    cached_buffers_blocks_.emplace_back(cached_buffer_block);

    total_available_byte_size_ += buffer_ci.size - byte_size;

    return {buffer.VkHandle(),
            cached_buffer_block.used_range.begin,
            cached_buffer_block.used_range.size(),
            cached_buffer_block.buffer.GetMappedPtr(),
            cached_buffer_block.buffer.Address(),
            cached_buffer_block.buffer.Allocation()};
}

void GpuResourcesManager::BufferCache::ReturnBuffers() {
    total_available_byte_size_ = 0;
    for (CachedBufferBlock &cached_buffer_block : cached_buffers_blocks_) {
        cached_buffer_block.used_range = {0, 0};
        total_available_byte_size_ += cached_buffer_block.total_range.size();
    }
}

void GpuResourcesManager::BufferCache::DestroyBuffers() {
    for (CachedBufferBlock &cached_buffer_block : cached_buffers_blocks_) {
        cached_buffer_block.buffer.Destroy();
    }
    cached_buffers_blocks_.clear();
}

bool StagingBuffer::CanDeviceEverStage(Validator &gpuav) {
    return gpuav.phys_dev_props.deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
}

StagingBuffer::StagingBuffer(GpuResourcesManager &gpu_resources_manager, VkDeviceSize size, VkCommandBuffer cb)
    : gpu_resources_manager(gpu_resources_manager) {
    VVL_ZoneScoped;
    // Never use staging buffer on integrated GPUs
    if (gpu_resources_manager.gpuav_.phys_dev_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
        device_buffer_range = gpu_resources_manager.GetHostCachedBufferRange(size);
        host_buffer_range = device_buffer_range;
    }
    // On other types of GPUs, let VMA decide whether to stage or not
    else {
        device_buffer_range = gpu_resources_manager.GetStagingBufferRange(size);
    }
    vmaGetAllocationMemoryProperties(gpu_resources_manager.gpuav_.vma_allocator_, device_buffer_range.vma_alloc,
                                     &device_buffer_mem_prop_flags);

    if (device_buffer_mem_prop_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        host_buffer_range = device_buffer_range;
    } else {
        VkBufferMemoryBarrier barrier_access_before_write = vku::InitStructHelper();
        barrier_access_before_write.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier_access_before_write.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier_access_before_write.buffer = device_buffer_range.buffer;
        barrier_access_before_write.offset = device_buffer_range.offset;
        barrier_access_before_write.size = device_buffer_range.size;

        DispatchCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1,
                                   &barrier_access_before_write, 0, nullptr);

        DispatchCmdFillBuffer(cb, device_buffer_range.buffer, device_buffer_range.offset, device_buffer_range.size, 0);

        VkBufferMemoryBarrier barrier_access_after_write = vku::InitStructHelper();
        barrier_access_after_write.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier_access_after_write.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        barrier_access_after_write.buffer = device_buffer_range.buffer;
        barrier_access_after_write.offset = device_buffer_range.offset;
        barrier_access_after_write.size = device_buffer_range.size;

        DispatchCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 1,
                                   &barrier_access_after_write, 0, nullptr);

        // #ARNO_TODO reconsider using host cached memory
        host_buffer_range = gpu_resources_manager.GetHostCachedBufferRange(size);
    }

    std::memset(host_buffer_range.offset_mapped_ptr, 0, (size_t)host_buffer_range.size);
    gpu_resources_manager.FlushAllocation(host_buffer_range);
}

void StagingBuffer::CmdCopyDeviceToHost(VkCommandBuffer cb) const {
    if (device_buffer_mem_prop_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        // nothing to do
        return;
    }

    // Dispatch a copy command, copying staging buffer device local memory to host visible
    VkBufferMemoryBarrier barrier_read_after_write = vku::InitStructHelper();
    barrier_read_after_write.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    barrier_read_after_write.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier_read_after_write.buffer = device_buffer_range.buffer;
    barrier_read_after_write.offset = device_buffer_range.offset;
    barrier_read_after_write.size = device_buffer_range.size;

    DispatchCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1,
                               &barrier_read_after_write, 0, nullptr);

    VkBufferCopy copy = {};
    copy.srcOffset = device_buffer_range.offset;
    copy.dstOffset = host_buffer_range.offset;
    copy.size = device_buffer_range.size;
    DispatchCmdCopyBuffer(cb, device_buffer_range.buffer, host_buffer_range.buffer, 1, &copy);

    // No additional barrier, host_visible_range will be read on the host
    // => will wait for cb's fence before reading.
}

CommandPool::CommandPool(Validator &gpuav, uint32_t queue_family_i, const Location &loc) : gpuav_(gpuav) {
    VkCommandPoolCreateInfo cmd_pool_ci = vku::InitStructHelper();
    cmd_pool_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmd_pool_ci.queueFamilyIndex = queue_family_i;
    VkResult result = DispatchCreateCommandPool(gpuav_.device, &cmd_pool_ci, nullptr, &cmd_pool_);
    if (result != VK_SUCCESS) {
        gpuav_.InternalError(LogObjectList(), loc, "Failed to create command buffer pool");
    }

    VkCommandBufferAllocateInfo cmd_buf_ai = vku::InitStructHelper();
    cmd_buf_ai.commandPool = cmd_pool_;
    cmd_buf_ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_buf_ai.commandBufferCount = 512;  // #ARNO_TODO do not hardcode commandBufferCount

    cmd_buffers_.resize(cmd_buf_ai.commandBufferCount);
    result = DispatchAllocateCommandBuffers(gpuav_.device, &cmd_buf_ai, cmd_buffers_.data());
    if (result != VK_SUCCESS) {
        gpuav_.InternalError(LogObjectList(), loc, "Failed to create command buffers");
    }

    for (VkCommandBuffer cb : cmd_buffers_) {
        gpuav.vk_set_device_loader_data_(gpuav.device, cb);
    }

    fences_.resize(cmd_buf_ai.commandBufferCount);
    for (VkFence &fence : fences_) {
        VkFenceCreateInfo fence_ci = vku::InitStructHelper();
        fence_ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        result = DispatchCreateFence(gpuav_.device, &fence_ci, nullptr, &fence);
        if (result != VK_SUCCESS) {
            gpuav_.InternalError(LogObjectList(), loc, "Failed to create fences");
        }
    }
}

CommandPool::~CommandPool() {
    DispatchDestroyCommandPool(gpuav_.device, cmd_pool_, nullptr);
    for (VkFence fence : fences_) {
        DispatchDestroyFence(gpuav_.device, fence, nullptr);
    }
}

std::pair<VkCommandBuffer, VkFence> CommandPool::GetCommandBuffer() {
    VVL_ZoneScoped;
    const size_t cb_i = cmd_buffer_ring_head_++ % cmd_buffers_.size();

    VkResult result = DispatchWaitForFences(gpuav_.device, 1, &fences_[cb_i], VK_TRUE, UINT64_MAX);
    if (result != VK_SUCCESS) {
        gpuav_.InternalError(fences_[cb_i], Location(vvl::Func::Empty), "Failed to wait for fence");
        return {VK_NULL_HANDLE, VK_NULL_HANDLE};
    }

    DispatchResetFences(gpuav_.device, 1, &fences_[cb_i]);
    DispatchResetCommandBuffer(cmd_buffers_[cb_i], 0);
    return {cmd_buffers_[cb_i], fences_[cb_i]};
}

}  // namespace vko
}  // namespace gpuav
