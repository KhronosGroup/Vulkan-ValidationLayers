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
#include "gpu/resources/gpuav_resources.h"

namespace gpuav {
class Validator;

class DescriptorSet : public vvl::DescriptorSet {
  public:
    DescriptorSet(const VkDescriptorSet set, vvl::DescriptorPool *pool,
                  const std::shared_ptr<vvl::DescriptorSetLayout const> &layout, uint32_t variable_count,
                  ValidationStateTracker *state_data);
    virtual ~DescriptorSet();
    void PerformPushDescriptorsUpdate(uint32_t write_count, const VkWriteDescriptorSet *write_descs) override;
    void PerformWriteUpdate(const VkWriteDescriptorSet &) override;
    void PerformCopyUpdate(const VkCopyDescriptorSet &, const vvl::DescriptorSet &) override;

    VkDeviceAddress GetIndexLUTAddress(Validator &gpuav, const Location &loc);
    VkDeviceAddress GetTypeAddress(Validator &gpuav, const Location &loc);
    VkDeviceAddress GetPostProcessBuffer(Validator &gpuav, const Location &loc);
    bool HasPostProcessBuffer() const { return !post_process_block_.Destroyed(); }

    std::map<uint32_t, std::vector<uint32_t>> UsedDescriptors(const Location &loc, uint32_t shader_set) const;

  protected:
    bool SkipBinding(const vvl::DescriptorBinding &binding, bool is_dynamic_accessed) const override { return true; }

  private:
    std::lock_guard<std::mutex> Lock() const { return std::lock_guard<std::mutex>(state_lock_); }

    DeviceMemoryBlock post_process_block_;

    DeviceMemoryBlock layout_block_;

    // Since we will re-bind the same descriptor set many times, keeping a version allows us to know if things have changed and
    // worth re-saving the new information
    std::atomic<uint32_t> current_version_{0};
    // Set when created the last used state
    uint32_t last_used_version_{0};
    DeviceMemoryBlock input_block_;

    mutable std::mutex state_lock_;
};

typedef uint32_t DescriptorId;
class DescriptorHeap {
  public:
    DescriptorHeap(Validator &gpuav, uint32_t max_descriptors, const Location &loc);
    ~DescriptorHeap();
    DescriptorId NextId(const VulkanTypedHandle &handle);
    void DeleteId(DescriptorId id);

    VkDeviceAddress GetDeviceAddress() const { return buffer_.Address(); }

  private:
    std::lock_guard<std::mutex> Lock() const { return std::lock_guard<std::mutex>(lock_); }

    mutable std::mutex lock_;

    const uint32_t max_descriptors_;
    DescriptorId next_id_{1};
    vvl::unordered_map<DescriptorId, VulkanTypedHandle> alloc_map_;

    DeviceMemoryBlock buffer_;
    uint32_t *gpu_heap_state_{nullptr};
};

}  // namespace gpuav
