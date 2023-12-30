/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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
    std::unique_ptr<vvl::unordered_set<uint64_t> > child_objects;  // Child objects (used for VkDescriptorPool only)
};

typedef vl_concurrent_unordered_map<uint64_t, std::shared_ptr<ObjTrackState>, 6> object_map_type;

class ObjectLifetimes : public ValidationObject {
    using Func = vvl::Func;
    using Struct = vvl::Struct;
    using Field = vvl::Field;

  public:
    // Override chassis read/write locks for this validation object
    // This override takes a deferred lock. i.e. it is not acquired.
    // This class does its own locking with a shared mutex.
    ReadLockGuard ReadLock() const override;
    WriteLockGuard WriteLock() override;

    mutable std::shared_mutex object_lifetime_mutex;
    WriteLockGuard WriteSharedLock() { return WriteLockGuard(object_lifetime_mutex); }
    ReadLockGuard ReadSharedLock() const { return ReadLockGuard(object_lifetime_mutex); }

    std::atomic<uint64_t> num_objects[kVulkanObjectTypeMax + 1];
    std::atomic<uint64_t> num_total_objects;
    // Vector of unordered_maps per object type to hold ObjTrackState info
    object_map_type object_map[kVulkanObjectTypeMax + 1];
    // Special-case map for swapchain images
    object_map_type swapchain_image_map;

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
    void InsertObject(object_map_type &map, T1 object, VulkanObjectType object_type, const Location &loc,
                      std::shared_ptr<ObjTrackState> pNode) {
        uint64_t object_handle = HandleToUint64(object);
        const bool inserted = map.insert(object_handle, pNode);
        if (!inserted) {
            // The object should not already exist. If we couldn't add it to the map, there was probably
            // a race condition in the app. Report an error and move on.
            // TODO should this be an error? https://gitlab.khronos.org/vulkan/vulkan/-/issues/3616
            (void)LogError("UNASSIGNED-ObjectTracker-Insert", object, loc,
                           "Couldn't insert %s Object 0x%" PRIxLEAST64
                           ", already existed. This should not happen and may indicate a "
                           "race condition in the application.",
                           object_string[object_type], object_handle);
        }
    }

    bool ReportUndestroyedInstanceObjects(VkInstance instance, const Location &loc) const;
    bool ReportUndestroyedDeviceObjects(VkDevice device, const Location &loc) const;

    bool ReportLeakedDeviceObjects(VkDevice device, VulkanObjectType object_type, const std::string &error_code,
                                   const Location &loc) const;
    bool ReportLeakedInstanceObjects(VkInstance instance, VulkanObjectType object_type, const std::string &error_code,
                                     const Location &loc) const;

    void DestroyUndestroyedObjects(VulkanObjectType object_type);

    void CreateQueue(VkQueue vkObj, const Location &loc);
    void AllocateCommandBuffer(const VkCommandPool command_pool, const VkCommandBuffer command_buffer, VkCommandBufferLevel level,
                               const Location &loc);
    void AllocateDescriptorSet(VkDescriptorPool descriptor_pool, VkDescriptorSet descriptor_set, const Location &loc);
    void AllocateDisplayKHR(VkPhysicalDevice physical_device, VkDisplayKHR display, const Location &loc);
    void CreateSwapchainImageObject(VkImage swapchain_image, VkSwapchainKHR swapchain, const Location &loc);
    void DestroyLeakedInstanceObjects();
    void DestroyLeakedDeviceObjects();
    void DestroyQueueDataStructures();
    bool ValidateCommandBuffer(VkCommandPool command_pool, VkCommandBuffer command_buffer, const Location &loc) const;
    bool ValidateDescriptorSet(VkDescriptorPool descriptor_pool, VkDescriptorSet descriptor_set, const Location &loc) const;
    bool ValidateSamplerObjects(const VkDescriptorSetLayoutCreateInfo *pCreateInfo, const Location &loc) const;
    bool ValidateDescriptorWrite(VkWriteDescriptorSet const *desc, bool isPush, const Location &loc) const;
    bool ValidateAnonymousObject(uint64_t object, VkObjectType core_object_type, const char *invalid_handle_vuid,
                                 const char *wrong_parent_vuid, const Location &loc) const;
    bool ValidateAccelerationStructures(const char *src_handle_vuid, const char *dst_handle_vuid, uint32_t count,
                                        const VkAccelerationStructureBuildGeometryInfoKHR *infos, const Location &loc) const;

    bool TracksObject(uint64_t object_handle, VulkanObjectType object_type) const;
    bool CheckObjectValidity(uint64_t object_handle, VulkanObjectType object_type, const char *invalid_handle_vuid,
                             const char *wrong_parent_vuid, const Location &loc, VulkanObjectType parent_type) const;

    template <typename T1>
    bool ValidateObject(T1 object, VulkanObjectType object_type, bool null_allowed, const char *invalid_handle_vuid,
                        const char *wrong_parent_vuid, const Location &loc,
                        VulkanObjectType parent_type = kVulkanObjectTypeDevice) const {
        if (null_allowed && (object == VK_NULL_HANDLE)) {
            return false;
        }
        return CheckObjectValidity(HandleToUint64(object), object_type, invalid_handle_vuid, wrong_parent_vuid, loc, parent_type);
    }

    template <typename T1>
    void CreateObject(T1 object, VulkanObjectType object_type, const VkAllocationCallbacks *pAllocator, const Location &loc) {
        uint64_t object_handle = HandleToUint64(object);
        const bool custom_allocator = (pAllocator != nullptr);
        if (!object_map[object_type].contains(object_handle)) {
            auto pNewObjNode = std::make_shared<ObjTrackState>();
            pNewObjNode->object_type = object_type;
            pNewObjNode->status = custom_allocator ? OBJSTATUS_CUSTOM_ALLOCATOR : OBJSTATUS_NONE;
            pNewObjNode->handle = object_handle;

            InsertObject(object_map[object_type], object, object_type, loc, pNewObjNode);
            num_objects[object_type]++;
            num_total_objects++;

            if (object_type == kVulkanObjectTypeDescriptorPool) {
                pNewObjNode->child_objects.reset(new vvl::unordered_set<uint64_t>);
            }
        }
    }

    void DestroyObjectSilently(uint64_t object, VulkanObjectType object_type);

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
                               const char *expected_custom_allocator_code, const char *expected_default_allocator_code,
                               const Location &loc) const {
        auto object = HandleToUint64(object_handle);
        const bool custom_allocator = pAllocator != nullptr;
        bool skip = false;

        if ((expected_custom_allocator_code != kVUIDUndefined || expected_default_allocator_code != kVUIDUndefined) &&
            object != HandleToUint64(VK_NULL_HANDLE)) {
            auto item = object_map[object_type].find(object);
            if (item != object_map[object_type].end()) {
                auto allocated_with_custom = (item->second->status & OBJSTATUS_CUSTOM_ALLOCATOR) ? true : false;
                if (allocated_with_custom && !custom_allocator && expected_custom_allocator_code != kVUIDUndefined) {
                    // This check only verifies that custom allocation callbacks were provided to both Create and Destroy calls,
                    // it cannot verify that these allocation callbacks are compatible with each other.
                    skip |= LogError(expected_custom_allocator_code, object_handle, loc,
                                     "Custom allocator not specified while destroying %s obj 0x%" PRIxLEAST64
                                     " but specified at creation.",
                                     object_string[object_type], object);

                } else if (!allocated_with_custom && custom_allocator && expected_default_allocator_code != kVUIDUndefined) {
                    skip |= LogError(expected_default_allocator_code, object_handle, loc,
                                     "Custom allocator specified while destroying %s obj 0x%" PRIxLEAST64
                                     " but not specified at creation.",
                                     object_string[object_type], object);
                }
            }
        }
        return skip;
    }

#include "generated/object_tracker.h"
};
