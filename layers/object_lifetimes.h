/* Copyright (c) 2015-2018 The Khronos Group Inc.
 * Copyright (c) 2015-2018 Valve Corporation
 * Copyright (c) 2015-2018 LunarG, Inc.
 * Copyright (C) 2015-2018 Google Inc.
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
 *
 * Author: Mark Lobodzinski <mark@lunarg.com>
 */

#pragma once

#include <vector>
#include <unordered_map>
#include "vulkan/vulkan.h"
#include "vk_object_types.h"

namespace object_tracker {

// Object Status -- used to track state of individual objects
typedef VkFlags ObjectStatusFlags;
enum ObjectStatusFlagBits {
    OBJSTATUS_NONE = 0x00000000,                      // No status is set
    OBJSTATUS_FENCE_IS_SUBMITTED = 0x00000001,        // Fence has been submitted
    OBJSTATUS_VIEWPORT_BOUND = 0x00000002,            // Viewport state object has been bound
    OBJSTATUS_RASTER_BOUND = 0x00000004,              // Viewport state object has been bound
    OBJSTATUS_COLOR_BLEND_BOUND = 0x00000008,         // Viewport state object has been bound
    OBJSTATUS_DEPTH_STENCIL_BOUND = 0x00000010,       // Viewport state object has been bound
    OBJSTATUS_GPU_MEM_MAPPED = 0x00000020,            // Memory object is currently mapped
    OBJSTATUS_COMMAND_BUFFER_SECONDARY = 0x00000040,  // Command Buffer is of type SECONDARY
    OBJSTATUS_CUSTOM_ALLOCATOR = 0x00000080,          // Allocated with custom allocator
};

// Object and state information structure
struct ObjTrackState {
    uint64_t handle;               // Object handle (new)
    VulkanObjectType object_type;  // Object type identifier
    ObjectStatusFlags status;      // Object state
    uint64_t parent_object;        // Parent object
};

// Track Queue information
struct ObjTrackQueueInfo {
    uint32_t queue_node_index;
    VkQueue queue;
};

typedef std::unordered_map<uint64_t, ObjTrackState *> object_map_type;

struct object_lifetime {
    // Include an object_tracker structure
    uint64_t num_objects[kVulkanObjectTypeMax + 1];
    uint64_t num_total_objects;
    // Vector of unordered_maps per object type to hold ObjTrackState info
    std::vector<object_map_type> object_map;
    // Special-case map for swapchain images
    std::unordered_map<uint64_t, ObjTrackState *> swapchainImageMap;
    // Map of queue information structures, one per queue
    std::unordered_map<VkQueue, ObjTrackQueueInfo *> queue_info_map;

    std::vector<VkQueueFamilyProperties> queue_family_properties;

    // Default constructor
    object_lifetime() : num_objects{}, num_total_objects(0), object_map{} { object_map.resize(kVulkanObjectTypeMax + 1); }
};

}  // namespace object_tracker
