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

VkQueueFlags PhysicalDevice::GetSupportedQueues() {
    VkQueueFlags flag = 0;
    for (const auto& prop : queue_family_properties) {
        flag |= prop.queueFlags;
    }
    return flag;
}

PhysicalDevice::PhysicalDevice(VkPhysicalDevice handle)
    : StateObject(handle, kVulkanObjectTypePhysicalDevice),
      queue_family_properties(GetQueueFamilyProps(handle)),
      supported_queues(GetSupportedQueues()) {}

}  // namespace vvl
