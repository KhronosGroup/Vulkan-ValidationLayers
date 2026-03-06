/* Copyright (c) 2024-2025 The Khronos Group Inc.
 * Copyright (c) 2024-2025 Valve Corporation
 * Copyright (c) 2024-2025 LunarG, Inc.
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

#include "state_tracker/device_state.h"
#include "generated/dispatch_functions.h"

namespace vvl {

void PhysicalDevice::SetCallState(Func func, CallState new_state) {
    WriteLockGuard guard(call_state_lock_);
    auto result = call_state_.emplace(func, new_state);
    if (!result.second) {
        if (result.first->second < new_state) {
            result.first->second = new_state;
        }
    }
}

void PhysicalDevice::SetCallState(Func func, bool has_ptr) {
    CallState new_state = has_ptr ? CallState::QueryDetails : CallState::QueryCount;
    SetCallState(func, new_state);
}

CallState PhysicalDevice::GetCallState(Func func) const {
    ReadLockGuard guard(call_state_lock_);
    auto iter = call_state_.find(func);
    return iter != call_state_.end() ? iter->second : CallState::Uncalled;
}

bool PhysicalDevice::WasUncalled(Func func) const { return GetCallState(func) == vvl::CallState::Uncalled; }

bool PhysicalDevice::WasCalled(Func func) const { return GetCallState(func) != vvl::CallState::Uncalled; }

const std::vector<VkQueueFamilyProperties> PhysicalDevice::GetQueueFamilyProps(VkPhysicalDevice phys_dev) {
    std::vector<VkQueueFamilyProperties> result;
    uint32_t count;
    DispatchGetPhysicalDeviceQueueFamilyProperties(phys_dev, &count, nullptr);
    result.resize(count);
    DispatchGetPhysicalDeviceQueueFamilyProperties(phys_dev, &count, result.data());
    return result;
}

const std::unordered_map<uint32_t, std::vector<VkQueueFamilyDataGraphPropertiesARM>> PhysicalDevice::GetQueueFamilyDataGraphProps(VkPhysicalDevice phys_dev) {
    uint32_t n_families = static_cast<uint32_t>(queue_family_properties.size());
    if (n_families == 0) {
        // queue_family_properties not yet initialized. We should've done it before now, this is just in case
        DispatchGetPhysicalDeviceQueueFamilyProperties(phys_dev, &n_families, nullptr);
    }

    std::unordered_map<uint32_t, std::vector<VkQueueFamilyDataGraphPropertiesARM>> all_properties;
    for (uint32_t i = 0; i < n_families; i++) {
        uint32_t n_properties = 0;
        assert(VK_SUCCESS == DispatchGetPhysicalDeviceQueueFamilyDataGraphPropertiesARM(phys_dev, i, &n_properties, nullptr));
        std::vector<VkQueueFamilyDataGraphPropertiesARM> family_properties(n_properties, vku::InitStruct<VkQueueFamilyDataGraphPropertiesARM>());
        assert(VK_SUCCESS == DispatchGetPhysicalDeviceQueueFamilyDataGraphPropertiesARM(phys_dev, i, &n_properties, family_properties.data()));
        all_properties.try_emplace(i, std::move(family_properties));
    }
    return all_properties;
}

VkQueueFlags PhysicalDevice::GetSupportedQueues() {
    VkQueueFlags flags = 0;
    for (const auto& prop : queue_family_properties) {
        flags |= prop.queueFlags;
    }
    return flags;
}

PhysicalDevice::PhysicalDevice(VkPhysicalDevice handle)
    : StateObject(handle, kVulkanObjectTypePhysicalDevice),
      queue_family_properties(GetQueueFamilyProps(handle)),
      queue_family_data_graph_properties(GetQueueFamilyDataGraphProps(handle)),
      supported_queues(GetSupportedQueues()) {}

}  // namespace vvl
