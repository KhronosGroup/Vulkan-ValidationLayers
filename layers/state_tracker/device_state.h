/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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
#include "state_tracker/base_node.h"
#include "generated/layer_chassis_dispatch.h"
#include "generated/vk_safe_struct.h"
#include <vector>

class QUEUE_FAMILY_PERF_COUNTERS {
  public:
    std::vector<VkPerformanceCounterKHR> counters;
};

class SURFACELESS_QUERY_STATE {
  public:
    std::vector<safe_VkSurfaceFormat2KHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
    safe_VkSurfaceCapabilities2KHR capabilities;
};

class PHYSICAL_DEVICE_STATE : public BASE_NODE {
  public:
    uint32_t queue_family_known_count = 1;  // spec implies one QF must always be supported
    const std::vector<VkQueueFamilyProperties> queue_family_properties;
    // TODO These are currently used by CoreChecks, but should probably be refactored
    bool vkGetPhysicalDeviceDisplayPlanePropertiesKHR_called = false;
    uint32_t display_plane_property_count = 0;

    // Map of queue family index to QUEUE_FAMILY_PERF_COUNTERS
    vvl::unordered_map<uint32_t, std::unique_ptr<QUEUE_FAMILY_PERF_COUNTERS>> perf_counters;

    // Surfaceless Query extension needs 'global' surface_state data
    SURFACELESS_QUERY_STATE surfaceless_query_state{};

    PHYSICAL_DEVICE_STATE(VkPhysicalDevice phys_dev)
        : BASE_NODE(phys_dev, kVulkanObjectTypePhysicalDevice), queue_family_properties(GetQueueFamilyProps(phys_dev)) {}

    VkPhysicalDevice PhysDev() const { return handle_.Cast<VkPhysicalDevice>(); }

  private:
    const std::vector<VkQueueFamilyProperties> GetQueueFamilyProps(VkPhysicalDevice phys_dev) {
        std::vector<VkQueueFamilyProperties> result;
        uint32_t count;
        DispatchGetPhysicalDeviceQueueFamilyProperties(phys_dev, &count, nullptr);
        result.resize(count);
        DispatchGetPhysicalDeviceQueueFamilyProperties(phys_dev, &count, result.data());
        return result;
    }
};

class DISPLAY_MODE_STATE : public BASE_NODE {
  public:
    const VkPhysicalDevice physical_device;

    DISPLAY_MODE_STATE(VkDisplayModeKHR dm, VkPhysicalDevice phys_dev)
        : BASE_NODE(dm, kVulkanObjectTypeDisplayModeKHR), physical_device(phys_dev) {}

    VkDisplayModeKHR display_mode() const { return handle_.Cast<VkDisplayModeKHR>(); }
};
