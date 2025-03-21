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

void Buffer::FlushAllocation(const Location &loc, VkDeviceSize offset, VkDeviceSize size) const {
    VkResult result = vmaFlushAllocation(gpuav.vma_allocator_, allocation, offset, size);
    if (result != VK_SUCCESS) {
        gpuav.InternalVmaError(gpuav.device, loc, "Unable to flush device memory.");
    }
}

void Buffer::InvalidateAllocation(const Location &loc, VkDeviceSize offset, VkDeviceSize size) const {
    VkResult result = vmaInvalidateAllocation(gpuav.vma_allocator_, allocation, offset, size);
    if (result != VK_SUCCESS) {
        gpuav.InternalVmaError(gpuav.device, loc, "Unable to invalidate device memory.");
    }
}

bool Buffer::Create(const Location &loc, const VkBufferCreateInfo *buffer_create_info,
                    const VmaAllocationCreateInfo *allocation_create_info) {
    VkResult result =
        vmaCreateBuffer(gpuav.vma_allocator_, buffer_create_info, allocation_create_info, &buffer, &allocation, nullptr);
    if (result != VK_SUCCESS) {
        gpuav.InternalVmaError(gpuav.device, loc, "Unable to allocate device memory for internal buffer.");
        return false;
    }
    size = buffer_create_info->size;

    if (buffer_create_info->usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        // After creating the buffer, get the address right away
        device_address = gpuav.device_state->GetBufferDeviceAddressHelper(buffer, &gpuav.modified_extensions);
        if (device_address == 0) {
            gpuav.InternalError(gpuav.device, loc, "Failed to get address with DispatchGetBufferDeviceAddress.");
            return false;
        }
    }
    if (allocation_create_info->requiredFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        result = vmaMapMemory(gpuav.vma_allocator_, allocation, &mapped_ptr);
        if (result != VK_SUCCESS) {
            mapped_ptr = nullptr;
            gpuav.InternalVmaError(gpuav.device, loc, "Unable to map device memory.");
            return false;
        }
    }
    return true;
}

void Buffer::Destroy() {
    if (buffer != VK_NULL_HANDLE) {
        if (mapped_ptr != nullptr) {
            vmaUnmapMemory(gpuav.vma_allocator_, allocation);
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

VkDescriptorSet GpuResourcesManager::GetManagedDescriptorSet(VkDescriptorSetLayout desc_set_layout) {
    // Look for a descriptor set layout matching input,
    // if found get or add an associated descriptor set
    for (LayoutToSets &layout_to_sets : cache_layouts_to_sets_) {
        if (layout_to_sets.desc_set_layout != desc_set_layout) {
            continue;
        }

        if (layout_to_sets.first_available_desc_set == layout_to_sets.cached_descriptors.size()) {
            CachedDescriptor cached_descriptor;
            const VkResult result = descriptor_set_manager_.GetDescriptorSet(&cached_descriptor.desc_pool, desc_set_layout,
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

vko::Buffer GpuResourcesManager::GetManagedBuffer(Validator &gpuav, const Location &loc, const VkBufferCreateInfo &buffer_ci,
                                                  const VmaAllocationCreateInfo &alloc_ci) {
    assert(buffer_ci.pNext == nullptr);
    assert(buffer_ci.sharingMode == VK_SHARING_MODE_EXCLUSIVE);
    assert(alloc_ci.pUserData == nullptr);

    // Try to find a cached and available buffer created with equivalent characteristics
    for (CachedBuffer &cached_buffer : cached_buffers_) {
        if (!(cached_buffer.status == CachedStatus::Available)) {
            continue;
        }
        const bool same_buffer_ci = buffer_ci.flags == cached_buffer.buffer_ci.flags &&
                                    buffer_ci.size <= cached_buffer.buffer_ci.size &&
                                    buffer_ci.usage == cached_buffer.buffer_ci.usage;

        const bool same_alloc_ci =
            alloc_ci.flags == cached_buffer.allocation_ci.flags && alloc_ci.usage == cached_buffer.allocation_ci.usage &&
            alloc_ci.requiredFlags == cached_buffer.allocation_ci.requiredFlags &&
            alloc_ci.preferredFlags == cached_buffer.allocation_ci.preferredFlags &&
            alloc_ci.memoryTypeBits == cached_buffer.allocation_ci.memoryTypeBits &&
            alloc_ci.pool == cached_buffer.allocation_ci.pool && alloc_ci.priority == cached_buffer.allocation_ci.priority;

        if (same_buffer_ci && same_alloc_ci) {
            cached_buffer.status = CachedStatus::InUse;
            return cached_buffer.buffer;
        }
    }

    // Did not find a cached buffer, create one, cache it and return its handle
    Buffer buffer(gpuav);
    const bool success = buffer.Create(loc, &buffer_ci, &alloc_ci);
    if (!success) {
        return buffer;
    }
    CachedBuffer cached_buffer = {buffer_ci, alloc_ci, buffer, CachedStatus::InUse};
    cached_buffers_.emplace_back(cached_buffer);
    return buffer;
}

void GpuResourcesManager::ReturnResources() {
    for (LayoutToSets &layout_to_set : cache_layouts_to_sets_) {
        layout_to_set.first_available_desc_set = 0;
    }

    for (CachedBuffer &cached_buffer : cached_buffers_) {
        cached_buffer.status = CachedStatus::Available;
    }
}

void GpuResourcesManager::DestroyResources() {
    for (LayoutToSets &layout_to_set : cache_layouts_to_sets_) {
        for (CachedDescriptor &cached_descriptor : layout_to_set.cached_descriptors) {
            descriptor_set_manager_.PutBackDescriptorSet(cached_descriptor.desc_pool, cached_descriptor.desc_set);
        }
        layout_to_set.cached_descriptors.clear();
    }
    cache_layouts_to_sets_.clear();

    for (CachedBuffer &cached_buffer : cached_buffers_) {
        cached_buffer.buffer.Destroy();
    }
    cached_buffers_.clear();
}
}  // namespace vko
}  // namespace gpuav
