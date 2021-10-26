/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (C) 2015-2021 Google Inc.
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
    std::unique_ptr<layer_data::unordered_set<uint64_t> > child_objects;  // Child objects (used for VkDescriptorPool only)
};

typedef vl_concurrent_unordered_map<uint64_t, std::shared_ptr<ObjTrackState>, 6> object_map_type;

class ObjectLifetimes : public ValidationObject {
  public:
    // Override chassis read/write locks for this validation object
    // This override takes a deferred lock. i.e. it is not acquired.
    // This class does its own locking with a shared mutex.
    ReadLockGuard ReadLock() override;
    WriteLockGuard WriteLock() override;

    mutable ReadWriteLock object_lifetime_mutex;
    WriteLockGuard WriteSharedLock() { return WriteLockGuard(object_lifetime_mutex); }
    ReadLockGuard ReadSharedLock() const { return ReadLockGuard(object_lifetime_mutex); }

    std::atomic<uint64_t> num_objects[kVulkanObjectTypeMax + 1];
    std::atomic<uint64_t> num_total_objects;
    // Vector of unordered_maps per object type to hold ObjTrackState info
    object_map_type object_map[kVulkanObjectTypeMax + 1];
    // Special-case map for swapchain images
    object_map_type swapchainImageMap;

    void *device_createinfo_pnext;
    bool null_descriptor_enabled;

    // Constructor for object lifetime tracking
    ObjectLifetimes() : num_objects{}, num_total_objects(0), device_createinfo_pnext(nullptr), null_descriptor_enabled(false) {
        container_type = LayerObjectTypeObjectTracker;
    }
    ~ObjectLifetimes() {
        if (device_createinfo_pnext) {
            FreePnextChain(device_createinfo_pnext);
        }
    }

    template <typename T1>
    void InsertObject(object_map_type &map, T1 object, VulkanObjectType object_type, std::shared_ptr<ObjTrackState> pNode) {
        uint64_t object_handle = HandleToUint64(object);
        bool inserted = map.insert(object_handle, pNode);
        if (!inserted) {
            // The object should not already exist. If we couldn't add it to the map, there was probably
            // a race condition in the app. Report an error and move on.
            (void)LogError(object, kVUID_ObjectTracker_Info,
                           "Couldn't insert %s Object 0x%" PRIxLEAST64
                           ", already existed. This should not happen and may indicate a "
                           "race condition in the application.",
                           object_string[object_type], object_handle);
        }
    }

    bool ReportUndestroyedInstanceObjects(VkInstance instance, const std::string &error_code) const;
    bool ReportUndestroyedDeviceObjects(VkDevice device, const std::string &error_code) const;

    bool ReportLeakedDeviceObjects(VkDevice device, VulkanObjectType object_type, const std::string &error_code) const;
    bool ReportLeakedInstanceObjects(VkInstance instance, VulkanObjectType object_type, const std::string &error_code) const;

    void DestroyUndestroyedObjects(VulkanObjectType object_type);

    void CreateQueue(VkQueue vkObj);
    void AllocateCommandBuffer(const VkCommandPool command_pool, const VkCommandBuffer command_buffer, VkCommandBufferLevel level);
    void AllocateDescriptorSet(VkDescriptorPool descriptor_pool, VkDescriptorSet descriptor_set);
    void CreateSwapchainImageObject(VkImage swapchain_image, VkSwapchainKHR swapchain);
    void DestroyLeakedInstanceObjects();
    void DestroyLeakedDeviceObjects();
    bool ValidateDeviceObject(const VulkanTypedHandle &device_typed, const char *invalid_handle_code,
                              const char *wrong_device_code) const;
    void DestroyQueueDataStructures();
    bool ValidateCommandBuffer(VkCommandPool command_pool, VkCommandBuffer command_buffer) const;
    bool ValidateDescriptorSet(VkDescriptorPool descriptor_pool, VkDescriptorSet descriptor_set) const;
    bool ValidateSamplerObjects(const VkDescriptorSetLayoutCreateInfo *pCreateInfo) const;
    bool ValidateDescriptorWrite(VkWriteDescriptorSet const *desc, bool isPush) const;
    bool ValidateAnonymousObject(uint64_t object, VkObjectType core_object_type, bool null_allowed, const char *invalid_handle_code,
                                 const char *wrong_device_code) const;
    bool ValidateAccelerationStructures(const char *dst_handle_vuid, uint32_t count,
                                        const VkAccelerationStructureBuildGeometryInfoKHR *infos) const;

    ObjectLifetimes *GetObjectLifetimeData(std::vector<ValidationObject *> &object_dispatch) const {
        for (auto *layer_object : object_dispatch) {
            if (layer_object->container_type == LayerObjectTypeObjectTracker) {
                return (reinterpret_cast<ObjectLifetimes *>(layer_object));
            }
        }
        return nullptr;
    };

    bool CheckObjectValidity(uint64_t object_handle, VulkanObjectType object_type, bool null_allowed,
                             const char *invalid_handle_code, const char *wrong_device_code) const {
        // Look for object in object map
        if (!object_map[object_type].contains(object_handle)) {
            // If object is an image, also look for it in the swapchain image map
            if ((object_type != kVulkanObjectTypeImage) || (swapchainImageMap.find(object_handle) == swapchainImageMap.end())) {
                // Object not found, look for it in other device object maps
                for (const auto &other_device_data : layer_data_map) {
                    for (auto *layer_object_data : other_device_data.second->object_dispatch) {
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
                                        return LogError(instance, wrong_device_code,
                                                        "Object 0x%" PRIxLEAST64
                                                        " of type %s"
                                                        " was not created, allocated or retrieved from the correct device.",
                                                        object_handle, object_string[object_type]);

                                    } else {
                                        return false;
                                    }
                                }
                            }
                        }
                    }
                }
                // Report an error if object was not found anywhere
                return LogError(instance, invalid_handle_code, "Invalid %s Object 0x%" PRIxLEAST64 ".", object_string[object_type],
                                object_handle);
            }
        }
        return false;
    }

    template <typename T1>
    bool ValidateObject(T1 object, VulkanObjectType object_type, bool null_allowed, const char *invalid_handle_code,
                        const char *wrong_device_code) const {
        if (null_allowed && (object == VK_NULL_HANDLE)) {
            return false;
        }

        if (object_type == kVulkanObjectTypeDevice) {
            return ValidateDeviceObject(VulkanTypedHandle(object, object_type), invalid_handle_code, wrong_device_code);
        }

        return CheckObjectValidity(HandleToUint64(object), object_type, null_allowed, invalid_handle_code, wrong_device_code);
    }

    template <typename T1>
    void CreateObject(T1 object, VulkanObjectType object_type, const VkAllocationCallbacks *pAllocator) {
        uint64_t object_handle = HandleToUint64(object);
        bool custom_allocator = (pAllocator != nullptr);
        if (!object_map[object_type].contains(object_handle)) {
            auto pNewObjNode = std::make_shared<ObjTrackState>();
            pNewObjNode->object_type = object_type;
            pNewObjNode->status = custom_allocator ? OBJSTATUS_CUSTOM_ALLOCATOR : OBJSTATUS_NONE;
            pNewObjNode->handle = object_handle;

            InsertObject(object_map[object_type], object, object_type, pNewObjNode);
            num_objects[object_type]++;
            num_total_objects++;

            if (object_type == kVulkanObjectTypeDescriptorPool) {
                pNewObjNode->child_objects.reset(new layer_data::unordered_set<uint64_t>);
            }
        }
    }

    void DestroyObjectSilently(uint64_t object, VulkanObjectType object_type) {
        assert(object != HandleToUint64(VK_NULL_HANDLE));

        auto item = object_map[object_type].pop(object);
        if (item == object_map[object_type].end()) {
            // We've already checked that the object exists. If we couldn't find and atomically remove it
            // from the map, there must have been a race condition in the app. Report an error and move on.
            (void)LogError(device, kVUID_ObjectTracker_Info,
                           "Couldn't destroy %s Object 0x%" PRIxLEAST64
                           ", not found. This should not happen and may indicate a race condition in the application.",
                           object_string[object_type], object);

            return;
        }
        assert(num_total_objects > 0);

        num_total_objects--;
        assert(num_objects[item->second->object_type] > 0);

        num_objects[item->second->object_type]--;
    }

    template <typename T1>
    void RecordDestroyObject(T1 object_handle, VulkanObjectType object_type) {
        auto object = HandleToUint64(object_handle);
        if (object != HandleToUint64(VK_NULL_HANDLE)) {
            if (object_map[object_type].contains(object)) {
                DestroyObjectSilently(object, object_type);
            }
        }
    }

    template <typename T1>
    bool ValidateDestroyObject(T1 object_handle, VulkanObjectType object_type, const VkAllocationCallbacks *pAllocator,
                               const char *expected_custom_allocator_code, const char *expected_default_allocator_code) const {
        auto object = HandleToUint64(object_handle);
        bool custom_allocator = pAllocator != nullptr;
        bool skip = false;

        if ((expected_custom_allocator_code != kVUIDUndefined || expected_default_allocator_code != kVUIDUndefined) &&
            object != HandleToUint64(VK_NULL_HANDLE)) {
            auto item = object_map[object_type].find(object);
            if (item != object_map[object_type].end()) {
                auto allocated_with_custom = (item->second->status & OBJSTATUS_CUSTOM_ALLOCATOR) ? true : false;
                if (allocated_with_custom && !custom_allocator && expected_custom_allocator_code != kVUIDUndefined) {
                    // This check only verifies that custom allocation callbacks were provided to both Create and Destroy calls,
                    // it cannot verify that these allocation callbacks are compatible with each other.
                    skip |= LogError(object_handle, expected_custom_allocator_code,
                                     "Custom allocator not specified while destroying %s obj 0x%" PRIxLEAST64
                                     " but specified at creation.",
                                     object_string[object_type], object);

                } else if (!allocated_with_custom && custom_allocator && expected_default_allocator_code != kVUIDUndefined) {
                    skip |= LogError(object_handle, expected_default_allocator_code,
                                     "Custom allocator specified while destroying %s obj 0x%" PRIxLEAST64
                                     " but not specified at creation.",
                                     object_string[object_type], object);
                }
            }
        }
        return skip;
    }

#include "object_tracker.h"
};
