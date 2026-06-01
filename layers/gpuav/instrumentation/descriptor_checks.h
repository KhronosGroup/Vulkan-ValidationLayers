/* Copyright (c) 2024-2026 LunarG, Inc.
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

#include <mutex>

#include "containers/custom_containers.h"
#include "gpuav/resources/gpuav_vulkan_objects.h"
#include "generated/vk_object_types.h"

namespace gpuav {
class Validator;
class CommandBufferSubState;

using DescriptorId = uint32_t;
class DescriptorIdPool {
  public:
    DescriptorIdPool(Validator& gpuav, uint32_t max_descriptors);
    ~DescriptorIdPool();

    DescriptorId NextId(const VulkanTypedHandle &handle);
    void DeleteId(DescriptorId id);

    VkDeviceAddress GetDeviceAddress() const { return buffer_.Address(); }

  private:
    mutable std::mutex lock_;

    const uint32_t max_descriptors_;
    DescriptorId next_id_{1};
    vvl::unordered_map<DescriptorId, VulkanTypedHandle> alloc_map_;

    vko::Buffer buffer_;
    uint32_t* gpu_id_pool_state_{nullptr};
};

// Descriptor Ids are used on the GPU to identify if a given descriptor is valid.
// In some applications there are very large bindless descriptor arrays where it isn't feasible to track validity
// via the StateObject::parent_nodes_ map as usual. Instead, these ids are stored in a giant GPU accessible bitmap
// so that the instrumentation can decide if a descriptor is actually valid when it is used in a shader.
class DescriptorIdTracker {
  public:
    DescriptorIdTracker(DescriptorIdPool& id_pool_, VulkanTypedHandle handle) : id_pool(id_pool_), id(id_pool_.NextId(handle)) {}

    DescriptorIdTracker(const DescriptorIdTracker &) = delete;
    DescriptorIdTracker &operator=(const DescriptorIdTracker &) = delete;

    ~DescriptorIdTracker() { id_pool.DeleteId(id); }

    DescriptorIdPool& id_pool;
    const DescriptorId id{};
};

void DescriptorChecksOnFinishDeviceSetup(Validator &gpuav);
void RegisterDescriptorChecksValidation(Validator& gpuav, CommandBufferSubState& cb);

}  // namespace gpuav
