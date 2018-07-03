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
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Tobin Ehlis <tobin@lunarg.com>
 */

#include <mutex>
#include <cinttypes>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unordered_map>
#include <unordered_set>

#include "vk_loader_platform.h"
#include "vulkan/vulkan.h"
#include "vk_layer_config.h"
#include "vk_layer_data.h"
#include "vk_layer_logging.h"
#include "vk_object_types.h"
#include "vulkan/vk_layer.h"
#include "vk_enum_string_helper.h"
#include "vk_layer_extension_utils.h"
#include "vk_layer_utils.h"
#include "vulkan/vk_layer.h"
#include "vk_dispatch_table_helper.h"
#include "vk_validation_error_messages.h"
#include "vk_extension_helper.h"

namespace object_tracker {

// Suppress unused warning on Linux
#if defined(__GNUC__)
#define DECORATE_UNUSED __attribute__((unused))
#else
#define DECORATE_UNUSED
#endif

// clang-format off
static const char DECORATE_UNUSED *kVUID_ObjectTracker_Info = "UNASSIGNED-ObjectTracker-Info";
static const char DECORATE_UNUSED *kVUID_ObjectTracker_InternalError = "UNASSIGNED-ObjectTracker-InternalError";
static const char DECORATE_UNUSED *kVUID_ObjectTracker_ObjectLeak =    "UNASSIGNED-ObjectTracker-ObjectLeak";
static const char DECORATE_UNUSED *kVUID_ObjectTracker_UnknownObject = "UNASSIGNED-ObjectTracker-UnknownObject";
// clang-format on

#undef DECORATE_UNUSED

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

struct layer_data {
    VkInstance instance;
    VkPhysicalDevice physical_device;

    uint64_t num_objects[kVulkanObjectTypeMax + 1];
    uint64_t num_total_objects;
    std::unordered_set<std::string> device_extension_set;

    debug_report_data *report_data;
    std::vector<VkDebugReportCallbackEXT> logging_callback;
    std::vector<VkDebugUtilsMessengerEXT> logging_messenger;
    // The following are for keeping track of the temporary callbacks that can
    // be used in vkCreateInstance and vkDestroyInstance:
    uint32_t num_tmp_report_callbacks;
    VkDebugReportCallbackCreateInfoEXT *tmp_report_create_infos;
    VkDebugReportCallbackEXT *tmp_report_callbacks;
    uint32_t num_tmp_debug_messengers;
    VkDebugUtilsMessengerCreateInfoEXT *tmp_messenger_create_infos;
    VkDebugUtilsMessengerEXT *tmp_debug_messengers;

    std::vector<VkQueueFamilyProperties> queue_family_properties;

    // Vector of unordered_maps per object type to hold ObjTrackState info
    std::vector<object_map_type> object_map;
    // Special-case map for swapchain images
    std::unordered_map<uint64_t, ObjTrackState *> swapchainImageMap;
    // Map of queue information structures, one per queue
    std::unordered_map<VkQueue, ObjTrackQueueInfo *> queue_info_map;

    VkLayerDispatchTable device_dispatch_table;
    VkLayerInstanceDispatchTable instance_dispatch_table;
    // Default constructor
    layer_data()
        : instance(nullptr),
          physical_device(nullptr),
          num_objects{},
          num_total_objects(0),
          report_data(nullptr),
          num_tmp_report_callbacks(0),
          tmp_report_create_infos(nullptr),
          tmp_report_callbacks(nullptr),
          num_tmp_debug_messengers(0),
          tmp_messenger_create_infos(nullptr),
          tmp_debug_messengers(nullptr),
          object_map{},
          device_dispatch_table{},
          instance_dispatch_table{} {
        object_map.resize(kVulkanObjectTypeMax + 1);
    }
};

extern std::unordered_map<void *, layer_data *> layer_data_map;
extern std::mutex global_lock;
extern uint64_t object_track_index;
extern uint32_t loader_layer_if_version;
extern const std::unordered_map<std::string, void *> name_to_funcptr_map;

void DeviceReportUndestroyedObjects(VkDevice device, VulkanObjectType object_type, const std::string &error_code);
void DeviceDestroyUndestroyedObjects(VkDevice device, VulkanObjectType object_type);
void CreateQueue(VkDevice device, VkQueue vkObj);
void AddQueueInfo(VkDevice device, uint32_t queue_node_index, VkQueue queue);
void ValidateQueueFlags(VkQueue queue, const char *function);
void AllocateCommandBuffer(VkDevice device, const VkCommandPool command_pool, const VkCommandBuffer command_buffer,
                           VkCommandBufferLevel level);
void AllocateDescriptorSet(VkDevice device, VkDescriptorPool descriptor_pool, VkDescriptorSet descriptor_set);
void CreateSwapchainImageObject(VkDevice dispatchable_object, VkImage swapchain_image, VkSwapchainKHR swapchain);
void ReportUndestroyedObjects(VkDevice device, const std::string &error_code);
void DestroyUndestroyedObjects(VkDevice device);
bool ValidateDeviceObject(uint64_t device_handle, const std::string &invalid_handle_code, const std::string &wrong_device_code);

template <typename T1, typename T2>
bool ValidateObject(T1 dispatchable_object, T2 object, VulkanObjectType object_type, bool null_allowed,
                    const std::string &invalid_handle_code, const std::string &wrong_device_code) {
    if (null_allowed && (object == VK_NULL_HANDLE)) {
        return false;
    }
    auto object_handle = HandleToUint64(object);

    if (object_type == kVulkanObjectTypeDevice) {
        return ValidateDeviceObject(object_handle, invalid_handle_code, wrong_device_code);
    }

    VkDebugReportObjectTypeEXT debug_object_type = get_debug_report_enum[object_type];

    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(dispatchable_object), layer_data_map);
    // Look for object in device object map
    if (device_data->object_map[object_type].find(object_handle) == device_data->object_map[object_type].end()) {
        // If object is an image, also look for it in the swapchain image map
        if ((object_type != kVulkanObjectTypeImage) ||
            (device_data->swapchainImageMap.find(object_handle) == device_data->swapchainImageMap.end())) {
            // Object not found, look for it in other device object maps
            for (auto other_device_data : layer_data_map) {
                if (other_device_data.second != device_data) {
                    if (other_device_data.second->object_map[object_type].find(object_handle) !=
                            other_device_data.second->object_map[object_type].end() ||
                        (object_type == kVulkanObjectTypeImage && other_device_data.second->swapchainImageMap.find(object_handle) !=
                                                                      other_device_data.second->swapchainImageMap.end())) {
                        // Object found on other device, report an error if object has a device parent error code
                        if ((wrong_device_code != kVUIDUndefined) && (object_type != kVulkanObjectTypeSurfaceKHR)) {
                            return log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, debug_object_type,
                                           object_handle, wrong_device_code,
                                           "Object 0x%" PRIxLEAST64
                                           " was not created, allocated or retrieved from the correct device.",
                                           object_handle);
                        } else {
                            return false;
                        }
                    }
                }
            }
            // Report an error if object was not found anywhere
            return log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, debug_object_type, object_handle,
                           invalid_handle_code, "Invalid %s Object 0x%" PRIxLEAST64 ".", object_string[object_type], object_handle);
        }
    }
    return false;
}

template <typename T1, typename T2>
void CreateObject(T1 dispatchable_object, T2 object, VulkanObjectType object_type, const VkAllocationCallbacks *pAllocator) {
    layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(dispatchable_object), layer_data_map);

    auto object_handle = HandleToUint64(object);
    bool custom_allocator = pAllocator != nullptr;

    if (!instance_data->object_map[object_type].count(object_handle)) {
        VkDebugReportObjectTypeEXT debug_object_type = get_debug_report_enum[object_type];
        log_msg(instance_data->report_data, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, debug_object_type, object_handle,
                kVUID_ObjectTracker_Info, "OBJ[0x%" PRIxLEAST64 "] : CREATE %s object 0x%" PRIxLEAST64, object_track_index++,
                object_string[object_type], object_handle);

        ObjTrackState *pNewObjNode = new ObjTrackState;
        pNewObjNode->object_type = object_type;
        pNewObjNode->status = custom_allocator ? OBJSTATUS_CUSTOM_ALLOCATOR : OBJSTATUS_NONE;
        pNewObjNode->handle = object_handle;

        instance_data->object_map[object_type][object_handle] = pNewObjNode;
        instance_data->num_objects[object_type]++;
        instance_data->num_total_objects++;
    }
}

template <typename T1, typename T2>
void DestroyObjectSilently(T1 dispatchable_object, T2 object, VulkanObjectType object_type) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(dispatchable_object), layer_data_map);

    auto object_handle = HandleToUint64(object);
    assert(object_handle != VK_NULL_HANDLE);

    auto item = device_data->object_map[object_type].find(object_handle);
    assert(item != device_data->object_map[object_type].end());

    ObjTrackState *pNode = item->second;
    assert(device_data->num_total_objects > 0);

    device_data->num_total_objects--;
    assert(device_data->num_objects[pNode->object_type] > 0);

    device_data->num_objects[pNode->object_type]--;

    delete pNode;
    device_data->object_map[object_type].erase(item);
}

template <typename T1, typename T2>
void DestroyObject(T1 dispatchable_object, T2 object, VulkanObjectType object_type, const VkAllocationCallbacks *pAllocator,
                   const std::string &expected_custom_allocator_code, const std::string &expected_default_allocator_code) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(dispatchable_object), layer_data_map);

    auto object_handle = HandleToUint64(object);
    bool custom_allocator = pAllocator != nullptr;
    VkDebugReportObjectTypeEXT debug_object_type = get_debug_report_enum[object_type];

    if (object_handle != VK_NULL_HANDLE) {
        auto item = device_data->object_map[object_type].find(object_handle);
        if (item != device_data->object_map[object_type].end()) {
            ObjTrackState *pNode = item->second;

            log_msg(device_data->report_data, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, debug_object_type, object_handle,
                    kVUID_ObjectTracker_Info,
                    "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
                    object_string[object_type], HandleToUint64(object), device_data->num_total_objects - 1,
                    device_data->num_objects[pNode->object_type] - 1, object_string[object_type]);

            auto allocated_with_custom = (pNode->status & OBJSTATUS_CUSTOM_ALLOCATOR) ? true : false;
            if (allocated_with_custom && !custom_allocator && expected_custom_allocator_code != kVUIDUndefined) {
                // This check only verifies that custom allocation callbacks were provided to both Create and Destroy calls,
                // it cannot verify that these allocation callbacks are compatible with each other.
                log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, debug_object_type, object_handle,
                        expected_custom_allocator_code,
                        "Custom allocator not specified while destroying %s obj 0x%" PRIxLEAST64 " but specified at creation.",
                        object_string[object_type], object_handle);
            } else if (!allocated_with_custom && custom_allocator && expected_default_allocator_code != kVUIDUndefined) {
                log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, debug_object_type, object_handle,
                        expected_default_allocator_code,
                        "Custom allocator specified while destroying %s obj 0x%" PRIxLEAST64 " but not specified at creation.",
                        object_string[object_type], object_handle);
            }

            DestroyObjectSilently(dispatchable_object, object, object_type);
        } else {
            log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, object_handle,
                    kVUID_ObjectTracker_UnknownObject,
                    "Unable to remove %s obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
                    object_string[object_type], object_handle);
        }
    }
}

}  // namespace object_tracker
