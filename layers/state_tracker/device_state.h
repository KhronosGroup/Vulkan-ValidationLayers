/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
#include "state_tracker/state_object.h"
#include "error_message/error_location.h"
#include <vulkan/utility/vk_safe_struct.hpp>
#include <vector>

class QueueFamilyPerfCounters {
  public:
    std::vector<VkPerformanceCounterKHR> counters;
};

class SurfacelessQueryState {
  public:
    std::vector<vku::safe_VkSurfaceFormat2KHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
    vku::safe_VkSurfaceCapabilities2KHR capabilities;
};

namespace vvl {

enum CALL_STATE {
    UNCALLED,       // Function has not been called
    QUERY_COUNT,    // Function called once to query a count
    QUERY_DETAILS,  // Function called w/ a count to query details
};

class PhysicalDevice : public StateObject {
  public:
    uint32_t queue_family_known_count = 1;  // spec implies one QF must always be supported
    const std::vector<VkQueueFamilyProperties> queue_family_properties;
    const VkQueueFlags supported_queues;
    uint32_t display_plane_property_count = 0;
    uint32_t surface_formats_count = 0;

    // Map of queue family index to QueueFamilyPerfCounters
    unordered_map<uint32_t, std::unique_ptr<QueueFamilyPerfCounters>> perf_counters;

    unordered_map<Func, CALL_STATE> call_state;

    // Surfaceless Query extension needs 'global' surface_state data
    SurfacelessQueryState surfaceless_query_state{};

    PhysicalDevice(VkPhysicalDevice handle);

    VkPhysicalDevice VkHandle() const { return handle_.Cast<VkPhysicalDevice>(); }

    void SetCallState(vvl::Func func, bool has_ptr) {
        CALL_STATE new_state = has_ptr ? QUERY_DETAILS : QUERY_COUNT;
        auto result = call_state.emplace(func, new_state);
        if (!result.second) {
            if (result.first->second < new_state) {
                result.first->second = new_state;
            }
        }
    }

    CALL_STATE GetCallState(vvl::Func func) const {
        auto iter = call_state.find(func);
        return iter != call_state.end() ? iter->second : UNCALLED;
    }

  private:
    const std::vector<VkQueueFamilyProperties> GetQueueFamilyProps(VkPhysicalDevice phys_dev);
    VkQueueFlags GetSupportedQueues();
};

class DisplayMode : public StateObject {
  public:
    const VkPhysicalDevice physical_device;

    DisplayMode(VkDisplayModeKHR handle, VkPhysicalDevice phys_dev)
        : StateObject(handle, kVulkanObjectTypeDisplayModeKHR), physical_device(phys_dev) {}

    VkDisplayModeKHR VkHandle() const { return handle_.Cast<VkDisplayModeKHR>(); }
};

}  // namespace vvl
