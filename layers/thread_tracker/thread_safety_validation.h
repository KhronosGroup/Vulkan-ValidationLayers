/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google Inc.
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

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "utils/vk_layer_utils.h"

VK_DEFINE_NON_DISPATCHABLE_HANDLE(DISTINCT_NONDISPATCHABLE_PHONY_HANDLE)
// The following line must match the vulkan_core.h condition guarding VK_DEFINE_NON_DISPATCHABLE_HANDLE
#if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__)) || defined(_M_X64) || defined(__ia64) || \
    defined(_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
// If pointers are 64-bit, then there can be separate counters for each
// NONDISPATCHABLE_HANDLE type.  Otherwise they are all typedef uint64_t.
#define DISTINCT_NONDISPATCHABLE_HANDLES
// Make sure we catch any disagreement between us and the vulkan definition
static_assert(std::is_pointer<DISTINCT_NONDISPATCHABLE_PHONY_HANDLE>::value,
              "Mismatched non-dispatchable handle handle, expected pointer type.");
#else
// Make sure we catch any disagreement between us and the vulkan definition
static_assert(std::is_same<uint64_t, DISTINCT_NONDISPATCHABLE_PHONY_HANDLE>::value,
              "Mismatched non-dispatchable handle handle, expected uint64_t.");
#endif

[[maybe_unused]] static const char *kVUID_Threading_Info = "UNASSIGNED-Threading-Info";
[[maybe_unused]] static const char *kVUID_Threading_MultipleThreads = "UNASSIGNED-Threading-MultipleThreads";
[[maybe_unused]] static const char *kVUID_Threading_SingleThreadReuse = "UNASSIGNED-Threading-SingleThreadReuse";

class alignas(get_hardware_destructive_interference_size()) ObjectUseData {
  public:
    class WriteReadCount {
      public:
        explicit WriteReadCount(int64_t v) : count(v) {}

        int32_t GetReadCount() const { return static_cast<int32_t>(count & 0xFFFFFFFF); }
        int32_t GetWriteCount() const { return static_cast<int32_t>(count >> 32); }

      private:
        int64_t count{};
    };

    WriteReadCount AddWriter() {
        int64_t prev = writer_reader_count.fetch_add(1ULL << 32);
        return WriteReadCount(prev);
    }
    WriteReadCount AddReader() {
        int64_t prev = writer_reader_count.fetch_add(1ULL);
        return WriteReadCount(prev);
    }
    WriteReadCount RemoveWriter() {
        int64_t prev = writer_reader_count.fetch_add(-(1LL << 32));
        return WriteReadCount(prev);
    }
    WriteReadCount RemoveReader() {
        int64_t prev = writer_reader_count.fetch_add(-1LL);
        return WriteReadCount(prev);
    }
    WriteReadCount GetCount() { return WriteReadCount(writer_reader_count); }

    void WaitForObjectIdle(bool is_writer) {
        // Wait for thread-safe access to object instead of skipping call.
        while (GetCount().GetReadCount() > (int)(!is_writer) || GetCount().GetWriteCount() > (int)is_writer) {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }

    std::atomic<std::thread::id> thread{};

  private:
    // Need to update write and read counts atomically. Writer in high 32 bits, reader in low 32 bits.
    std::atomic<int64_t> writer_reader_count{};
};

template <typename T>
class counter {
  public:
    const char *typeName;
    VulkanObjectType object_type;
    ValidationObject *object_data;

    vl_concurrent_unordered_map<T, std::shared_ptr<ObjectUseData>, 6> object_table;

    void CreateObject(T object) { object_table.insert(object, std::make_shared<ObjectUseData>()); }

    void DestroyObject(T object) {
        if (object) {
            object_table.erase(object);
        }
    }

    std::shared_ptr<ObjectUseData> FindObject(T object) {
        assert(object_table.contains(object));
        auto iter = object_table.find(object);
        if (iter != object_table.end()) {
            return iter->second;
        } else {
            object_data->LogError(object, kVUID_Threading_Info,
                                  "Couldn't find %s Object 0x%" PRIxLEAST64
                                  ". This should not happen and may indicate a bug in the application.",
                                  object_string[object_type], (uint64_t)(object));
            return nullptr;
        }
    }

    void StartWrite(T object, const char *api_name) {
        if (object == VK_NULL_HANDLE) {
            return;
        }
        bool skip = false;
        std::thread::id tid = std::this_thread::get_id();

        auto use_data = FindObject(object);
        if (!use_data) {
            return;
        }
        const ObjectUseData::WriteReadCount prevCount = use_data->AddWriter();

        if (prevCount.GetReadCount() == 0 && prevCount.GetWriteCount() == 0) {
            // There is no current use of the object.  Record writer thread.
            use_data->thread = tid;
        } else {
            if (prevCount.GetReadCount() == 0) {
                assert(prevCount.GetWriteCount() != 0);
                // There are no readers.  Two writers just collided.
                if (use_data->thread != tid) {
                    std::stringstream err_str;
                    err_str << "THREADING ERROR : " << api_name << "(): object of type " << typeName
                            << " is simultaneously used in thread " << use_data->thread.load(std::memory_order_relaxed)
                            << " and thread " << tid;
                    skip |= object_data->LogError(object, kVUID_Threading_MultipleThreads, "%s", err_str.str().c_str());
                    if (skip) {
                        // Wait for thread-safe access to object instead of skipping call.
                        use_data->WaitForObjectIdle(true);
                        // There is now no current use of the object.  Record writer thread.
                        use_data->thread = tid;
                    } else {
                        // There is now no current use of the object.  Record writer thread.
                        use_data->thread = tid;
                    }
                } else {
                    // This is either safe multiple use in one call, or recursive use.
                    // There is no way to make recursion safe.  Just forge ahead.
                }
            } else {
                // There are readers.  This writer collided with them.
                if (use_data->thread != tid) {
                    std::stringstream err_str;
                    err_str << "THREADING ERROR : " << api_name << "(): object of type " << typeName
                            << " is simultaneously used in thread " << use_data->thread.load(std::memory_order_relaxed)
                            << " and thread " << tid;
                    skip |= object_data->LogError(object, kVUID_Threading_MultipleThreads, "%s", err_str.str().c_str());
                    if (skip) {
                        // Wait for thread-safe access to object instead of skipping call.
                        use_data->WaitForObjectIdle(true);
                        // There is now no current use of the object.  Record writer thread.
                        use_data->thread = tid;
                    } else {
                        // Continue with an unsafe use of the object.
                        use_data->thread = tid;
                    }
                } else {
                    // This is either safe multiple use in one call, or recursive use.
                    // There is no way to make recursion safe.  Just forge ahead.
                }
            }
        }
    }

    void FinishWrite(T object, const char *api_name) {
        if (object == VK_NULL_HANDLE) {
            return;
        }
        // Object is no longer in use
        auto use_data = FindObject(object);
        if (!use_data) {
            return;
        }
        use_data->RemoveWriter();
    }

    void StartRead(T object, const char *api_name) {
        if (object == VK_NULL_HANDLE) {
            return;
        }
        bool skip = false;
        std::thread::id tid = std::this_thread::get_id();

        auto use_data = FindObject(object);
        if (!use_data) {
            return;
        }
        const ObjectUseData::WriteReadCount prevCount = use_data->AddReader();

        if (prevCount.GetReadCount() == 0 && prevCount.GetWriteCount() == 0) {
            // There is no current use of the object.
            use_data->thread = tid;
        } else if (prevCount.GetWriteCount() > 0 && use_data->thread != tid) {
            // There is a writer of the object.
            std::stringstream err_str;
            err_str << "THREADING ERROR : " << api_name << "(): object of type " << typeName << " is simultaneously used in thread "
                    << use_data->thread.load(std::memory_order_relaxed) << " and thread " << tid;
            skip |= object_data->LogError(object, kVUID_Threading_MultipleThreads, "%s", err_str.str().c_str());
            if (skip) {
                // Wait for thread-safe access to object instead of skipping call.
                use_data->WaitForObjectIdle(false);
                use_data->thread = tid;
            }
        } else {
            // There are other readers of the object.
        }
    }
    void FinishRead(T object, const char *api_name) {
        if (object == VK_NULL_HANDLE) {
            return;
        }

        auto use_data = FindObject(object);
        if (!use_data) {
            return;
        }
        use_data->RemoveReader();
    }
    counter(const char *name = "", VulkanObjectType type = kVulkanObjectTypeUnknown, ValidationObject *val_obj = nullptr) {
        typeName = name;
        object_type = type;
        object_data = val_obj;
    }

  private:
};

class ThreadSafety : public ValidationObject {
  public:
    std::shared_mutex thread_safety_lock;

    // Override chassis read/write locks for this validation object
    // This override takes a deferred lock. i.e. it is not acquired.
    ReadLockGuard ReadLock() const override;
    WriteLockGuard WriteLock() override;

    vl_concurrent_unordered_map<VkCommandBuffer, VkCommandPool, 6> command_pool_map;
    vvl::unordered_map<VkCommandPool, vvl::unordered_set<VkCommandBuffer>> pool_command_buffers_map;
    vvl::unordered_map<VkDevice, vvl::unordered_set<VkQueue>> device_queues_map;

    // Track per-descriptorsetlayout and per-descriptorset whether read_only is used.
    // This is used to (sloppily) implement the relaxed externsync rules for read_only
    // descriptors. We model updates of read_only descriptors as if they were reads
    // rather than writes, because they only conflict with the set being freed or reset.
    //
    // We don't track the read_only state per-binding for a couple reasons:
    // (1) We only have one counter per object, and if we treated non-UAB as writes
    //     and UAB as reads then they'd appear to conflict with each other.
    // (2) Avoid additional tracking of descriptor binding state in the descriptor set
    //     layout, and tracking of which bindings are accessed by a VkDescriptorUpdateTemplate.
    // Descriptor sets using VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT can also
    // be used simultaneously in multiple threads
    vl_concurrent_unordered_map<VkDescriptorSetLayout, bool, 4> dsl_read_only_map;
    vl_concurrent_unordered_map<VkDescriptorSet, bool, 6> ds_read_only_map;
    bool DsReadOnly(VkDescriptorSet) const;

    counter<VkCommandBuffer> c_VkCommandBuffer;
    counter<VkDevice> c_VkDevice;
    counter<VkInstance> c_VkInstance;
    counter<VkQueue> c_VkQueue;
#ifdef DISTINCT_NONDISPATCHABLE_HANDLES

    // Special entry to allow tracking of command pool Reset and Destroy
    counter<VkCommandPool> c_VkCommandPoolContents;

#include "generated/thread_safety_counter_definitions.h"

#else   // DISTINCT_NONDISPATCHABLE_HANDLES
    // Special entry to allow tracking of command pool Reset and Destroy
    counter<uint64_t> c_VkCommandPoolContents;

    counter<uint64_t> c_uint64_t;
#endif  // DISTINCT_NONDISPATCHABLE_HANDLES

    // If this ThreadSafety is for a VkDevice, then parent_instance points to the
    // ThreadSafety object of its parent VkInstance. This is used to get to the counters
    // for objects created with the instance as parent.
    ThreadSafety *parent_instance;

    ThreadSafety(ThreadSafety *parent)
        : c_VkCommandBuffer("VkCommandBuffer", kVulkanObjectTypeCommandBuffer, this),
          c_VkDevice("VkDevice", kVulkanObjectTypeDevice, this),
          c_VkInstance("VkInstance", kVulkanObjectTypeInstance, this),
          c_VkQueue("VkQueue", kVulkanObjectTypeQueue, this),
          c_VkCommandPoolContents("VkCommandPool", kVulkanObjectTypeCommandPool, this),
#ifdef DISTINCT_NONDISPATCHABLE_HANDLES
#include "generated/thread_safety_counter_instances.h"
#else   // DISTINCT_NONDISPATCHABLE_HANDLES
          c_uint64_t("NON_DISPATCHABLE_HANDLE", kVulkanObjectTypeUnknown, this),
#endif  // DISTINCT_NONDISPATCHABLE_HANDLES
          parent_instance(parent) {
        container_type = LayerObjectTypeThreading;
    };

#define WRAPPER(type)                                                                                     \
    void StartWriteObject(type object, const char *api_name) { c_##type.StartWrite(object, api_name); }   \
    void FinishWriteObject(type object, const char *api_name) { c_##type.FinishWrite(object, api_name); } \
    void StartReadObject(type object, const char *api_name) { c_##type.StartRead(object, api_name); }     \
    void FinishReadObject(type object, const char *api_name) { c_##type.FinishRead(object, api_name); }   \
    void CreateObject(type object) { c_##type.CreateObject(object); }                                     \
    void DestroyObject(type object) {                                                                     \
        c_##type.DestroyObject(object);                                                                   \
        c_##type.DestroyObject(object);                                                                   \
    }

#define WRAPPER_PARENT_INSTANCE(type)                                                                                           \
    void StartWriteObjectParentInstance(type object, const char *api_name) {                                                    \
        (parent_instance ? parent_instance : this)->c_##type.StartWrite(object, api_name);                                      \
    }                                                                                                                           \
    void FinishWriteObjectParentInstance(type object, const char *api_name) {                                                   \
        (parent_instance ? parent_instance : this)->c_##type.FinishWrite(object, api_name);                                     \
    }                                                                                                                           \
    void StartReadObjectParentInstance(type object, const char *api_name) {                                                     \
        (parent_instance ? parent_instance : this)->c_##type.StartRead(object, api_name);                                       \
    }                                                                                                                           \
    void FinishReadObjectParentInstance(type object, const char *api_name) {                                                    \
        (parent_instance ? parent_instance : this)->c_##type.FinishRead(object, api_name);                                      \
    }                                                                                                                           \
    void CreateObjectParentInstance(type object) { (parent_instance ? parent_instance : this)->c_##type.CreateObject(object); } \
    void DestroyObjectParentInstance(type object) { (parent_instance ? parent_instance : this)->c_##type.DestroyObject(object); }

    WRAPPER_PARENT_INSTANCE(VkDevice)
    WRAPPER_PARENT_INSTANCE(VkInstance)
    WRAPPER(VkQueue)
#ifdef DISTINCT_NONDISPATCHABLE_HANDLES
#include "generated/thread_safety_counter_bodies.h"
#else   // DISTINCT_NONDISPATCHABLE_HANDLES
    WRAPPER(uint64_t)
    WRAPPER_PARENT_INSTANCE(uint64_t)
#endif  // DISTINCT_NONDISPATCHABLE_HANDLES

    void CreateObject(VkCommandBuffer object) { c_VkCommandBuffer.CreateObject(object); }
    void DestroyObject(VkCommandBuffer object) { c_VkCommandBuffer.DestroyObject(object); }

    // VkCommandBuffer needs check for implicit use of command pool
    void StartWriteObject(VkCommandBuffer object, const char *api_name, bool lockPool = true) {
        if (lockPool) {
            auto iter = command_pool_map.find(object);
            if (iter != command_pool_map.end()) {
                VkCommandPool pool = iter->second;
                StartWriteObject(pool, api_name);
            }
        }
        c_VkCommandBuffer.StartWrite(object, api_name);
    }
    void FinishWriteObject(VkCommandBuffer object, const char *api_name, bool lockPool = true) {
        c_VkCommandBuffer.FinishWrite(object, api_name);
        if (lockPool) {
            auto iter = command_pool_map.find(object);
            if (iter != command_pool_map.end()) {
                VkCommandPool pool = iter->second;
                FinishWriteObject(pool, api_name);
            }
        }
    }
    void StartReadObject(VkCommandBuffer object, const char *api_name) {
        auto iter = command_pool_map.find(object);
        if (iter != command_pool_map.end()) {
            VkCommandPool pool = iter->second;
            // We set up a read guard against the "Contents" counter to catch conflict vs. vkResetCommandPool and
            // vkDestroyCommandPool while *not* establishing a read guard against the command pool counter itself to avoid false
            // positive for non-externally sync'd command buffers
            c_VkCommandPoolContents.StartRead(pool, api_name);
        }
        c_VkCommandBuffer.StartRead(object, api_name);
    }
    void FinishReadObject(VkCommandBuffer object, const char *api_name) {
        c_VkCommandBuffer.FinishRead(object, api_name);
        auto iter = command_pool_map.find(object);
        if (iter != command_pool_map.end()) {
            VkCommandPool pool = iter->second;
            c_VkCommandPoolContents.FinishRead(pool, api_name);
        }
    }

    void PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                                  VkDisplayPlanePropertiesKHR *pProperties,
                                                                  VkResult result) override;

    void PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                                   VkDisplayPlaneProperties2KHR *pProperties,
                                                                   VkResult result) override;

    void PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                             VkDisplayPropertiesKHR *pProperties, VkResult result) override;

    void PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                              VkDisplayProperties2KHR *pProperties, VkResult result) override;

    void PreCallRecordGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                      const VkDisplayPlaneInfo2KHR *pDisplayPlaneInfo,
                                                      VkDisplayPlaneCapabilities2KHR *pCapabilities) override;

    void PostCallRecordGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                       const VkDisplayPlaneInfo2KHR *pDisplayPlaneInfo,
                                                       VkDisplayPlaneCapabilities2KHR *pCapabilities, VkResult result) override;

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

    void PostCallRecordGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display *dpy, RROutput rrOutput,
                                                VkDisplayKHR *pDisplay, VkResult result) override;

#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT

    void PostCallRecordGetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId, VkDisplayKHR *display,
                                        VkResult result) override;

#include "generated/thread_safety_commands.h"
};
