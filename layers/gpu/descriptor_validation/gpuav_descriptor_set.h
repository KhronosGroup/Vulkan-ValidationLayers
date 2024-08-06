/* Copyright (c) 2023-2024 The Khronos Group Inc.
 * Copyright (c) 2023-2024 Valve Corporation
 * Copyright (c) 2023-2024 LunarG, Inc.
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

#pragma once

#include <atomic>
#include <mutex>
#include "state_tracker/descriptor_sets.h"
#include "vma/vma.h"

namespace gpuav {

class Validator;

// TODO - This probably could be used elsewhere and a more universal object for creating buffers.
struct AddressBuffer {
    const Validator &gpuav;
    VmaAllocation allocation{nullptr};
    VkBuffer buffer{VK_NULL_HANDLE};
    VkDeviceAddress device_addr{0};

    AddressBuffer(Validator &gpuav) : gpuav(gpuav) {}

    // Warps VMA calls so we can report (unlikely) errors if found while making the usages of these clean
    void MapMemory(const Location &loc, void **data) const;
    void UnmapMemory() const;
    void FlushAllocation(const Location &loc, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;
    void InvalidateAllocation(const Location &loc, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;

    void CreateBuffer(const Location &loc, const VkBufferCreateInfo *buffer_create_info,
                      const VmaAllocationCreateInfo *allocation_create_info);
    void DestroyBuffer();
};

class DescriptorSet : public vvl::DescriptorSet {
  public:
    DescriptorSet(const VkDescriptorSet set, vvl::DescriptorPool *pool,
                  const std::shared_ptr<vvl::DescriptorSetLayout const> &layout, uint32_t variable_count,
                  ValidationStateTracker *state_data);
    virtual ~DescriptorSet();
    void Destroy() override { last_used_state_.reset(); };
    struct State {
        State(VkDescriptorSet set, uint32_t version, Validator &gpuav) : set(set), version(version), buffer(gpuav) {}
        ~State();

        const VkDescriptorSet set;
        const uint32_t version;
        AddressBuffer buffer;

        std::map<uint32_t, std::vector<uint32_t>> UsedDescriptors(const Location &loc, const DescriptorSet &set,
                                                                  uint32_t shader_set) const;
    };
    void PerformPushDescriptorsUpdate(uint32_t write_count, const VkWriteDescriptorSet *write_descs) override;
    void PerformWriteUpdate(const VkWriteDescriptorSet &) override;
    void PerformCopyUpdate(const VkCopyDescriptorSet &, const vvl::DescriptorSet &) override;

    VkDeviceAddress GetLayoutState(Validator &gpuav, const Location &loc);
    std::shared_ptr<State> GetCurrentState(Validator &gpuav, const Location &loc);
    std::shared_ptr<State> GetOutputState(Validator &gpuav, const Location &loc);

  protected:
    bool SkipBinding(const vvl::DescriptorBinding &binding, bool is_dynamic_accessed) const override { return true; }

  private:
    std::lock_guard<std::mutex> Lock() const { return std::lock_guard<std::mutex>(state_lock_); }

    AddressBuffer layout_;
    std::atomic<uint32_t> current_version_{0};
    std::shared_ptr<State> last_used_state_;
    std::shared_ptr<State> output_state_;
    mutable std::mutex state_lock_;
};

typedef uint32_t DescriptorId;
class DescriptorHeap {
  public:
    DescriptorHeap(Validator &gpuav, uint32_t max_descriptors, const Location &loc);
    ~DescriptorHeap();
    DescriptorId NextId(const VulkanTypedHandle &handle);
    void DeleteId(DescriptorId id);

    VkDeviceAddress GetDeviceAddress() const { return buffer_.device_addr; }

  private:
    std::lock_guard<std::mutex> Lock() const { return std::lock_guard<std::mutex>(lock_); }

    mutable std::mutex lock_;

    const uint32_t max_descriptors_;
    DescriptorId next_id_{1};
    vvl::unordered_map<DescriptorId, VulkanTypedHandle> alloc_map_;

    AddressBuffer buffer_;
    uint32_t *gpu_heap_state_{nullptr};
};

}  // namespace gpuav
