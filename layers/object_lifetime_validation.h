/* Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (C) 2015-2019 Google Inc.
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
 * Author: Tobin Ehlis <tobine@google.com>
 */

// shared_mutex support added in MSVC 2015 update 2
#if defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 190023918 && NTDDI_VERSION > NTDDI_WIN10_RS2
#include <shared_mutex>
typedef std::shared_mutex object_lifetime_mutex_t;
typedef std::shared_lock<object_lifetime_mutex_t> read_object_lifetime_mutex_t;
typedef std::unique_lock<object_lifetime_mutex_t> write_object_lifetime_mutex_t;
#else
typedef std::mutex object_lifetime_mutex_t;
typedef std::unique_lock<object_lifetime_mutex_t> read_object_lifetime_mutex_t;
typedef std::unique_lock<object_lifetime_mutex_t> write_object_lifetime_mutex_t;
#endif

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

extern uint64_t object_track_index;

// Object Status -- used to track state of individual objects
typedef VkFlags ObjectStatusFlags;
enum ObjectStatusFlagBits {
    OBJSTATUS_NONE = 0x00000000,                      // No status is set
    OBJSTATUS_COMMAND_BUFFER_SECONDARY = 0x00000001,  // Command Buffer is of type SECONDARY
    OBJSTATUS_CUSTOM_ALLOCATOR = 0x00000002,          // Allocated with custom allocator
};

// Object and state information structure
struct ObjTrackState {
    uint64_t handle;                                               // Object handle (new)
    VulkanObjectType object_type;                                  // Object type identifier
    ObjectStatusFlags status;                                      // Object state
    uint64_t parent_object;                                        // Parent object
    std::unique_ptr<std::unordered_set<uint64_t> > child_objects;  // Child objects (used for VkDescriptorPool only)
};

typedef vl_concurrent_unordered_map<uint64_t, std::shared_ptr<ObjTrackState>, 6> object_map_type;

class ObjectLifetimes : public ValidationObject {
   public:
    // Override chassis read/write locks for this validation object
    // This override takes a deferred lock. i.e. it is not acquired.
    // This class does its own locking with a shared mutex.
    virtual std::unique_lock<std::mutex> write_lock() {
        return std::unique_lock<std::mutex>(validation_object_mutex, std::defer_lock);
    }

    object_lifetime_mutex_t object_lifetime_mutex;
    write_object_lifetime_mutex_t write_shared_lock() { return write_object_lifetime_mutex_t(object_lifetime_mutex); }
    read_object_lifetime_mutex_t read_shared_lock() { return read_object_lifetime_mutex_t(object_lifetime_mutex); }

    std::atomic<uint64_t> num_objects[kVulkanObjectTypeMax + 1];
    std::atomic<uint64_t> num_total_objects;
    // Vector of unordered_maps per object type to hold ObjTrackState info
    object_map_type object_map[kVulkanObjectTypeMax + 1];
    // Special-case map for swapchain images
    object_map_type swapchainImageMap;

    // Constructor for object lifetime tracking
    ObjectLifetimes() : num_objects{}, num_total_objects(0) {}

    void InsertObject(object_map_type &map, uint64_t object_handle, VulkanObjectType object_type,
                      std::shared_ptr<ObjTrackState> pNode) {
        bool inserted = map.insert(object_handle, pNode);
        if (!inserted) {
            // The object should not already exist. If we couldn't add it to the map, there was probably
            // a race condition in the app. Report an error and move on.
            VkDebugReportObjectTypeEXT debug_object_type = get_debug_report_enum[object_type];
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, debug_object_type, object_handle, kVUID_ObjectTracker_Info,
                    "Couldn't insert %s Object 0x%" PRIxLEAST64
                    ", already existed. This should not happen and may indicate a "
                    "race condition in the application.",
                    object_string[object_type], object_handle);
        }
    }

    bool DeviceReportUndestroyedObjects(VkDevice device, VulkanObjectType object_type, const std::string &error_code);
    void DeviceDestroyUndestroyedObjects(VkDevice device, VulkanObjectType object_type);
    void CreateQueue(VkDevice device, VkQueue vkObj);
    void AllocateCommandBuffer(VkDevice device, const VkCommandPool command_pool, const VkCommandBuffer command_buffer,
                               VkCommandBufferLevel level);
    void AllocateDescriptorSet(VkDevice device, VkDescriptorPool descriptor_pool, VkDescriptorSet descriptor_set);
    void CreateSwapchainImageObject(VkDevice dispatchable_object, VkImage swapchain_image, VkSwapchainKHR swapchain);
    bool ReportUndestroyedObjects(VkDevice device, const std::string &error_code);
    void DestroyUndestroyedObjects(VkDevice device);
    bool ValidateDeviceObject(const VulkanTypedHandle &device_typed, const char *invalid_handle_code,
                              const char *wrong_device_code);
    void DestroyQueueDataStructures(VkDevice device);
    bool ValidateCommandBuffer(VkDevice device, VkCommandPool command_pool, VkCommandBuffer command_buffer);
    bool ValidateDescriptorSet(VkDevice device, VkDescriptorPool descriptor_pool, VkDescriptorSet descriptor_set);
    bool ValidateSamplerObjects(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo);
    template <typename DispObj>
    bool ValidateDescriptorWrite(DispObj disp, VkWriteDescriptorSet const *desc, bool isPush);

    ObjectLifetimes *GetObjectLifetimeData(std::vector<ValidationObject *> &object_dispatch) {
        for (auto layer_object : object_dispatch) {
            if (layer_object->container_type == LayerObjectTypeObjectTracker) {
                return (reinterpret_cast<ObjectLifetimes *>(layer_object));
            }
        }
        return nullptr;
    };

    template <typename T1, typename T2>
    bool ValidateObject(T1 dispatchable_object, T2 object, VulkanObjectType object_type, bool null_allowed,
                        const char *invalid_handle_code, const char *wrong_device_code) {
        if (null_allowed && (object == VK_NULL_HANDLE)) {
            return false;
        }
        auto object_handle = HandleToUint64(object);

        if (object_type == kVulkanObjectTypeDevice) {
            return ValidateDeviceObject(VulkanTypedHandle(object, object_type), invalid_handle_code, wrong_device_code);
        }

        VkDebugReportObjectTypeEXT debug_object_type = get_debug_report_enum[object_type];

        // Look for object in object map
        if (!object_map[object_type].contains(object_handle)) {
            // If object is an image, also look for it in the swapchain image map
            if ((object_type != kVulkanObjectTypeImage) || (swapchainImageMap.find(object_handle) == swapchainImageMap.end())) {
                // Object not found, look for it in other device object maps
                for (auto other_device_data : layer_data_map) {
                    for (auto layer_object_data : other_device_data.second->object_dispatch) {
                        if (layer_object_data->container_type == LayerObjectTypeObjectTracker) {
                            auto object_lifetime_data = reinterpret_cast<ObjectLifetimes *>(layer_object_data);
                            if (object_lifetime_data && (object_lifetime_data != this)) {
                                if (object_lifetime_data->object_map[object_type].find(object_handle) !=
                                        object_lifetime_data->object_map[object_type].end() ||
                                    (object_type == kVulkanObjectTypeImage &&
                                     object_lifetime_data->swapchainImageMap.find(object_handle) !=
                                         object_lifetime_data->swapchainImageMap.end())) {
                                    // Object found on other device, report an error if object has a device parent error code
                                    if ((wrong_device_code != kVUIDUndefined) && (object_type != kVulkanObjectTypeSurfaceKHR)) {
                                        return log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, debug_object_type, object_handle,
                                                       wrong_device_code,
                                                       "Object 0x%" PRIxLEAST64
                                                       " was not created, allocated or retrieved from the correct device.",
                                                       object_handle);
                                    } else {
                                        return false;
                                    }
                                }
                            }
                        }
                    }
                }
                // Report an error if object was not found anywhere
                return log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, debug_object_type, object_handle, invalid_handle_code,
                               "Invalid %s Object 0x%" PRIxLEAST64 ".", object_string[object_type], object_handle);
            }
        }
        return false;
    }

    template <typename T1, typename T2>
    void CreateObject(T1 dispatchable_object, T2 object, VulkanObjectType object_type, const VkAllocationCallbacks *pAllocator) {
        uint64_t object_handle = HandleToUint64(object);
        bool custom_allocator = (pAllocator != nullptr);
        if (!object_map[object_type].contains(object_handle)) {
            auto pNewObjNode = std::make_shared<ObjTrackState>();
            pNewObjNode->object_type = object_type;
            pNewObjNode->status = custom_allocator ? OBJSTATUS_CUSTOM_ALLOCATOR : OBJSTATUS_NONE;
            pNewObjNode->handle = object_handle;

            InsertObject(object_map[object_type], object_handle, object_type, pNewObjNode);
            num_objects[object_type]++;
            num_total_objects++;

            if (object_type == kVulkanObjectTypeDescriptorPool) {
                pNewObjNode->child_objects.reset(new std::unordered_set<uint64_t>);
            }
        }
    }

    template <typename T1>
    void DestroyObjectSilently(T1 object, VulkanObjectType object_type) {
        auto object_handle = HandleToUint64(object);
        assert(object_handle != VK_NULL_HANDLE);

        auto item = object_map[object_type].pop(object_handle);
        if (item == object_map[object_type].end()) {
            // We've already checked that the object exists. If we couldn't find and atomically remove it
            // from the map, there must have been a race condition in the app. Report an error and move on.
            VkDebugReportObjectTypeEXT debug_object_type = get_debug_report_enum[object_type];
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, debug_object_type, object_handle, kVUID_ObjectTracker_Info,
                    "Couldn't destroy %s Object 0x%" PRIxLEAST64
                    ", not found. This should not happen and may indicate a "
                    "race condition in the application.",
                    object_string[object_type], object_handle);
            return;
        }
        assert(num_total_objects > 0);

        num_total_objects--;
        assert(num_objects[item->second->object_type] > 0);

        num_objects[item->second->object_type]--;
    }

    template <typename T1, typename T2>
    void RecordDestroyObject(T1 dispatchable_object, T2 object, VulkanObjectType object_type) {
        auto object_handle = HandleToUint64(object);
        if (object_handle != VK_NULL_HANDLE) {
            if (object_map[object_type].contains(object_handle)) {
                DestroyObjectSilently(object, object_type);
            }
        }
    }

    template <typename T1, typename T2>
    bool ValidateDestroyObject(T1 dispatchable_object, T2 object, VulkanObjectType object_type,
                               const VkAllocationCallbacks *pAllocator, const char *expected_custom_allocator_code,
                               const char *expected_default_allocator_code) {
        auto object_handle = HandleToUint64(object);
        bool custom_allocator = pAllocator != nullptr;
        VkDebugReportObjectTypeEXT debug_object_type = get_debug_report_enum[object_type];
        bool skip = false;

        if ((expected_custom_allocator_code != kVUIDUndefined || expected_default_allocator_code != kVUIDUndefined) &&
            object_handle != VK_NULL_HANDLE) {
            auto item = object_map[object_type].find(object_handle);
            if (item != object_map[object_type].end()) {
                auto allocated_with_custom = (item->second->status & OBJSTATUS_CUSTOM_ALLOCATOR) ? true : false;
                if (allocated_with_custom && !custom_allocator && expected_custom_allocator_code != kVUIDUndefined) {
                    // This check only verifies that custom allocation callbacks were provided to both Create and Destroy calls,
                    // it cannot verify that these allocation callbacks are compatible with each other.
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, debug_object_type, object_handle,
                                    expected_custom_allocator_code,
                                    "Custom allocator not specified while destroying %s obj 0x%" PRIxLEAST64
                                    " but specified at creation.",
                                    object_string[object_type], object_handle);
                } else if (!allocated_with_custom && custom_allocator && expected_default_allocator_code != kVUIDUndefined) {
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, debug_object_type, object_handle,
                                    expected_default_allocator_code,
                                    "Custom allocator specified while destroying %s obj 0x%" PRIxLEAST64
                                    " but not specified at creation.",
                                    object_string[object_type], object_handle);
                }
            }
        }
        return skip;
    }

#include "object_tracker.h"
};
