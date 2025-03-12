/* Copyright (c) 2023-2025 The Khronos Group Inc.
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
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
#include <vector>
#include "state_tracker/descriptor_sets.h"
#include "gpuav/resources/gpuav_vulkan_objects.h"
#include "gpuav/spirv/interface.h"
#include "containers/custom_containers.h"

namespace gpuav {
class DescriptorHeap;
class Validator;

// Information about how each descriptor was accessed
struct DescriptorAccess {
    uint32_t binding;      // binding number in the descriptor set
    uint32_t index;        // index into descriptor array
    const ::spirv::ResourceInterfaceVariable &resource_variable;
};

class DescriptorSetSubState : public vvl::DescriptorSetSubState {
  public:
    DescriptorSetSubState(const vvl::DescriptorSet &set, Validator &state_data);
    virtual ~DescriptorSetSubState();

    void NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) override;
    void NotifyUpdate() override;

    VkDeviceAddress GetTypeAddress(Validator &gpuav, const Location &loc);
    VkDeviceAddress GetPostProcessBuffer(Validator &gpuav, const Location &loc);
    bool HasPostProcessBuffer() const { return !post_process_buffer_.IsDestroyed(); }
    bool CanPostProcess() const;

    std::vector<DescriptorAccess> GetDescriptorAccesses(const Location &loc, uint32_t shader_set,
                                                        const std::vector<const ::spirv::EntryPoint *> &entry_points) const;

  private:
    void BuildBindingLayouts();
    std::lock_guard<std::mutex> Lock() const { return std::lock_guard<std::mutex>(state_lock_); }

    vko::Buffer post_process_buffer_;

    std::vector<gpuav::spirv::BindingLayout> binding_layouts_;

    // Since we will re-bind the same descriptor set many times, keeping a version allows us to know if things have changed and
    // worth re-saving the new information
    std::atomic<uint32_t> current_version_{0};
    // Set when created the last used state
    uint32_t last_used_version_{0};
    vko::Buffer input_buffer_;

    mutable std::mutex state_lock_;
    DescriptorHeap *heap_{};
};

static inline DescriptorSetSubState &SubState(vvl::DescriptorSet &set) {
    return static_cast<DescriptorSetSubState &>(*set.SubState(LayerObjectTypeGpuAssisted));
}

typedef uint32_t DescriptorId;
class DescriptorHeap {
  public:
    DescriptorHeap(Validator &gpuav, uint32_t max_descriptors, const Location &loc);
    ~DescriptorHeap();

    DescriptorId GetId(const VulkanTypedHandle &handle);
    const VulkanTypedHandle GetHandle(DescriptorId id) const;

    void Delete(const VulkanTypedHandle &handle);

    VkDeviceAddress GetDeviceAddress() const { return buffer_.Address(); }

  private:
    std::lock_guard<std::mutex> Lock() const { return std::lock_guard<std::mutex>(lock_); }

    mutable std::mutex lock_;

    const uint32_t max_descriptors_;
    DescriptorId next_id_{1};
    vvl::unordered_map<DescriptorId, VulkanTypedHandle> alloc_map_;
    vvl::unordered_map<VulkanTypedHandle, DescriptorId> handle_map_;

    vko::Buffer buffer_;
    uint32_t *gpu_heap_state_{nullptr};
};

}  // namespace gpuav
