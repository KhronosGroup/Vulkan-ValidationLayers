/* Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
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

namespace gpuav_state {

class DescriptorSet : public cvdescriptorset::DescriptorSet {
  public:
    DescriptorSet(const VkDescriptorSet set, DESCRIPTOR_POOL_STATE *pool,
                  const std::shared_ptr<cvdescriptorset::DescriptorSetLayout const> &layout, uint32_t variable_count,
                  ValidationStateTracker *state_data);
    virtual ~DescriptorSet();
    void Destroy() override { last_used_state_.reset(); };
    struct State {
        ~State();

        VkDescriptorSet set{VK_NULL_HANDLE};
        uint32_t version{0};
        VmaAllocator allocator{nullptr};
        VmaAllocation allocation{nullptr};
        VkBuffer buffer{VK_NULL_HANDLE};
        VkDeviceAddress device_addr{0};
    };
    void PerformPushDescriptorsUpdate(uint32_t write_count, const VkWriteDescriptorSet *write_descs) override;
    void PerformWriteUpdate(const VkWriteDescriptorSet &) override;
    void PerformCopyUpdate(const VkCopyDescriptorSet &, const cvdescriptorset::DescriptorSet &) override;

    VkDeviceAddress GetLayoutState();
    std::shared_ptr<State> GetCurrentState();

  private:
    struct Layout {
        VmaAllocation allocation{nullptr};
        VkBuffer buffer{VK_NULL_HANDLE};
        VkDeviceAddress device_addr{0};
    };
    std::lock_guard<std::mutex> Lock() const { return std::lock_guard<std::mutex>(state_lock_); }

    Layout layout_;
    std::atomic<uint32_t> current_version_{0};
    std::shared_ptr<State> last_used_state_;
    mutable std::mutex state_lock_;
};

}  // namespace gpuav_state
