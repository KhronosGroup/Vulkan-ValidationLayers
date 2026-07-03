/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (c) 2015-2024 Google Inc.
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
#include <shared_mutex>
#include <string>
#include <thread>
#include "chassis/validation_object.h"

namespace threadsafety {

VK_DEFINE_NON_DISPATCHABLE_HANDLE(DISTINCT_NONDISPATCHABLE_PHONY_HANDLE)

// The following line must match the vulkan_core.h condition guarding VK_USE_64_BIT_PTR_DEFINES
#if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__)) || defined(_M_X64) || defined(__ia64) || \
    defined(_M_IA64) || defined(__aarch64__) || defined(__powerpc64__) || (defined(__riscv) && __riscv_xlen == 64)

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

struct ThreadInfo {
    std::thread::id tid;
    std::string thread_name;
};

// Alternative to std::thread::id that is guaranteed to be uint32_t.
// Internally implemented as atomic counter.
// We combine this 32-bit value and vvl::Func to get 64-bit value for atomic operations
uint32_t GetCurrentInternalThreadId();

// Map internal id to std::thread::id
ThreadInfo GetThreadInfo(uint32_t internal_thread_id);

// Return OS name of the calling thread or an empty string if the thread is unnamed
// or the name cannot be retrieved
std::string GetCurrentThreadName();

// Align ObjectUseData to a cache line to avoid false sharing of its atomics.
// Some CPUs (e.g. Apple M1) have 128-byte lines. We still use 64 to save memory,
// accepting possible false sharing there.
inline constexpr size_t kObjectUseDataAlignment = 64;

// Sanity check on the build machine
static_assert(vku::concurrent::get_hardware_destructive_interference_size() % kObjectUseDataAlignment == 0);

class alignas(kObjectUseDataAlignment) ObjectUseData {
  public:
    struct UseStatus {
        explicit UseStatus(uint64_t v) : has_read((v & 0xFFFFFFFF) != 0), has_write((v >> 32) != 0) {}
        bool has_read;
        bool has_write;
    };

    UseStatus AddWriter() {
        const uint64_t prev = writer_reader_count.fetch_add(uint64_t(1) << 32);
        return UseStatus(prev);
    }
    UseStatus AddReader() {
        const uint64_t prev = writer_reader_count.fetch_add(uint64_t(1));
        return UseStatus(prev);
    }
    void RemoveWriter() {
        [[maybe_unused]] const uint64_t prev = writer_reader_count.fetch_sub(uint64_t(1) << 32);
        assert((prev >> 32) != 0);
    }
    void RemoveReader() {
        [[maybe_unused]] const uint64_t prev = writer_reader_count.fetch_sub(uint64_t(1));
        assert((prev & 0xFFFFFFFF) != 0);
    }

    void UpdateThreadAndFunc(uint32_t internal_tid, vvl::Func func) {
        const uint64_t value = static_cast<uint64_t>(func) << 32 | internal_tid;
        thread_and_func.store(value);
    }

    uint32_t GetStoredInternalThreadId() const { return thread_and_func.load() & 0xffffffff; }

    // 32-bit internal thread id and vvl::Func
    std::atomic<uint64_t> thread_and_func{};

  private:
    // Need to update write and read counts atomically. Writer in high 32 bits, reader in low 32 bits.
    std::atomic<uint64_t> writer_reader_count{};
};

template <typename T>
class Counter {
  public:
    void Init(VulkanObjectType type, Logger* logger) {
        object_type = type;
        this->logger = logger;
    }

    void CreateObject(T object) { object_table.insert(object, std::make_shared<ObjectUseData>()); }

    void DestroyObject(T object) {
        if (object) {
            object_table.erase(object);
        }
    }

    void StartWrite(T object, const Location& loc) {
        if (object == VK_NULL_HANDLE) {
            return;
        }
        auto use_data = FindObject(object, loc);
        if (!use_data) {
            return;
        }
        const uint32_t current_internal_tid = GetCurrentInternalThreadId();
        const auto [prev_read, prev_write] = use_data->AddWriter();

        if (!prev_read && !prev_write) {
            // There is no current use of the object. Record writer thread
            use_data->UpdateThreadAndFunc(current_internal_tid, loc.function);
        } else if (use_data->GetStoredInternalThreadId() != current_internal_tid) {
            // Write collided with existing write or read
            ReportError("UNASSIGNED-Threading-MultipleThreads-Write", use_data, object, loc);
        } else {
            // We have other uses in the same call which is safe
        }
    }

    void FinishWrite(T object, const Location& loc) {
        if (object == VK_NULL_HANDLE) {
            return;
        }
        if (auto use_data = FindObject(object, loc)) {
            use_data->RemoveWriter();
        }
    }

    void StartRead(T object, const Location& loc) {
        if (object == VK_NULL_HANDLE) {
            return;
        }
        auto use_data = FindObject(object, loc);
        if (!use_data) {
            return;
        }
        const uint32_t current_internal_tid = GetCurrentInternalThreadId();
        const auto [prev_read, prev_write] = use_data->AddReader();

        if (!prev_read && !prev_write) {
            // There is no current use of the object. Record reader thread
            use_data->UpdateThreadAndFunc(current_internal_tid, loc.function);
        } else if (prev_write && use_data->GetStoredInternalThreadId() != current_internal_tid) {
            // Read collided with existing write
            ReportError("UNASSIGNED-Threading-MultipleThreads-Read", use_data, object, loc);
        } else {
            // There are other readers of the object or we have other uses in the
            // same call and this is safe
        }
    }

    void FinishRead(T object, const Location& loc) {
        if (object == VK_NULL_HANDLE) {
            return;
        }
        if (auto use_data = FindObject(object, loc)) {
            use_data->RemoveReader();
        }
    }

  private:
    std::shared_ptr<ObjectUseData> FindObject(T object, const Location& loc) {
        assert(object_table.contains(object));
        auto iter = object_table.find(object);
        if (iter != object_table.end()) {
            return iter->second;
        } else {
            logger->LogError("UNASSIGNED-Threading-Info", object, loc,
                             "Couldn't find %s Object 0x%" PRIxLEAST64
                             ". This should not happen and may indicate a bug in the application.",
                             string_VulkanObjectType(object_type), (uint64_t)(object));
            return nullptr;
        }
    }

    void ReportError(const char* vuid, const std::shared_ptr<ObjectUseData>& use_data, T object, const Location& loc) {
        const uint64_t value = use_data->thread_and_func.load();
        const uint32_t other_internal_tid = value & 0xffffffff;
        const vvl::Func other_func = static_cast<vvl::Func>(value >> 32);

        const std::thread::id current_tid = std::this_thread::get_id();
        const std::string current_name = GetCurrentThreadName();

        std::ostringstream ss;
        ss << "THREADING ERROR : object of type " << string_VulkanObjectType(object_type)
           << " is simultaneously used in current thread " << current_tid;
        if (!current_name.empty()) {
            ss << " \"" << current_name << "\"";
        }
        ss << " (" << vvl::String(loc.function) << ") and ";

        if (other_internal_tid != 0) {  // common case
            const ThreadInfo other_info = GetThreadInfo(other_internal_tid);
            ss << "thread " << other_info.tid;
            if (!other_info.thread_name.empty()) {
                ss << " \"" << other_info.thread_name << "\"";
            }
            ss << " (" << vvl::String(other_func) << ")";
        } else {  // rare case
            // The object was just created and two racing threads access it for the first time.
            // The other side of the race is not recorded yet (other_internal_tid == 0).
            // Report only this side.
            ss << "another thread";
        }

        logger->LogError(vuid, object, loc, "%s", ss.str().c_str());
    }

  private:
    VulkanObjectType object_type{};
    Logger* logger{};
    vvl::concurrent_unordered_map<T, std::shared_ptr<ObjectUseData>, 6> object_table;
};

#define WRAPPER(type)                                                                               \
    void StartWriteObject(type object, const Location &loc) { c_##type.StartWrite(object, loc); }   \
    void FinishWriteObject(type object, const Location &loc) { c_##type.FinishWrite(object, loc); } \
    void StartReadObject(type object, const Location &loc) { c_##type.StartRead(object, loc); }     \
    void FinishReadObject(type object, const Location &loc) { c_##type.FinishRead(object, loc); }   \
    void CreateObject(type object) { c_##type.CreateObject(object); }                               \
    void DestroyObject(type object) { c_##type.DestroyObject(object); }

class Instance : public vvl::BaseInstance {
  public:
    std::shared_mutex thread_safety_lock;

    Instance(vvl::DispatchInstance* dispatch) : BaseInstance(dispatch, LayerObjectTypeThreading) { InitCounters(); }

    void PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                                  VkDisplayPlanePropertiesKHR *pProperties,
                                                                  const RecordObject &record_obj) override;

    void PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                                   VkDisplayPlaneProperties2KHR *pProperties,
                                                                   const RecordObject &record_obj) override;

    void PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                             VkDisplayPropertiesKHR *pProperties,
                                                             const RecordObject &record_obj) override;

    void PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                              VkDisplayProperties2KHR *pProperties,
                                                              const RecordObject &record_obj) override;

    void PreCallRecordGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                      const VkDisplayPlaneInfo2KHR *pDisplayPlaneInfo,
                                                      VkDisplayPlaneCapabilities2KHR *pCapabilities,
                                                      const RecordObject &record_obj) override;

    void PostCallRecordGetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice,
                                                       const VkDisplayPlaneInfo2KHR *pDisplayPlaneInfo,
                                                       VkDisplayPlaneCapabilities2KHR *pCapabilities,
                                                       const RecordObject &record_obj) override;

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

    void PostCallRecordGetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display *dpy, RROutput rrOutput,
                                                VkDisplayKHR *pDisplay, const RecordObject &record_obj) override;

#endif  // VK_USE_PLATFORM_XLIB_XRANDR_EXT

    void PostCallRecordGetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId, VkDisplayKHR *display,
                                        const RecordObject &record_obj) override;
#include "generated/thread_safety_instance_defs.h"
};

#define WRAPPER_PARENT_INSTANCE(type)                                                                                           \
    void StartWriteObjectParentInstance(type object, const Location &loc) { parent_instance->StartWriteObject(object, loc); }   \
    void FinishWriteObjectParentInstance(type object, const Location &loc) { parent_instance->FinishWriteObject(object, loc); } \
    void StartReadObjectParentInstance(type object, const Location &loc) { parent_instance->StartReadObject(object, loc); }     \
    void FinishReadObjectParentInstance(type object, const Location &loc) { parent_instance->FinishReadObject(object, loc); }

class Device : public vvl::BaseDevice {
  public:
    std::shared_mutex thread_safety_lock;

    // Override chassis read/write locks for this validation object
    // This override takes a deferred lock. i.e. it is not acquired.
    ReadLockGuard ReadLock() const override;
    WriteLockGuard WriteLock() override;

    vvl::concurrent_unordered_map<VkCommandBuffer, VkCommandPool, 6> command_pool_map;
    vvl::unordered_map<VkCommandPool, vvl::unordered_set<VkCommandBuffer>> pool_command_buffers_map;
    vvl::unordered_map<VkDevice, vvl::unordered_set<VkQueue>> device_queues_map;
    std::vector<VkQueue> internally_synchronized_queues;

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
    vvl::concurrent_unordered_map<VkDescriptorSetLayout, bool, 4> dsl_read_only_map;
    vvl::concurrent_unordered_map<VkDescriptorSet, bool, 6> ds_read_only_map;
    bool DsReadOnly(VkDescriptorSet) const;
    // Map of wrapped swapchain handles to arrays of wrapped swapchain image IDs
    // Each swapchain has an immutable list of wrapped swapchain image IDs -- always return these IDs if they exist
    vvl::unordered_map<VkSwapchainKHR, std::vector<VkImage>> swapchain_wrapped_image_handle_map;
    // Map of wrapped descriptor pools to set of wrapped descriptor sets allocated from each pool
    vvl::unordered_map<VkDescriptorPool, vvl::unordered_set<VkDescriptorSet>> pool_descriptor_sets_map;

    // Special entry to allow tracking of command pool Reset and Destroy
#ifdef DISTINCT_NONDISPATCHABLE_HANDLES
    Counter<VkCommandPool> c_VkCommandPoolContents;
#else   // DISTINCT_NONDISPATCHABLE_HANDLES
    Counter<uint64_t> c_VkCommandPoolContents;
#endif  // DISTINCT_NONDISPATCHABLE_HANDLES

    Instance *parent_instance;

    Device(vvl::DispatchDevice* dev, Instance* instance_vo)
        : BaseDevice(dev, instance_vo, LayerObjectTypeThreading), parent_instance(instance_vo) {
        c_VkCommandPoolContents.Init(kVulkanObjectTypeCommandPool, this);
        InitCounters();
    }

    void CreateObject(VkCommandBuffer object) { c_VkCommandBuffer.CreateObject(object); }
    void DestroyObject(VkCommandBuffer object) { c_VkCommandBuffer.DestroyObject(object); }

    // VkCommandBuffer needs check for implicit use of command pool
    void StartWriteObject(VkCommandBuffer object, const Location& loc, bool lockPool = true) {
        if (lockPool) {
            auto iter = command_pool_map.find(object);
            if (iter != command_pool_map.end()) {
                VkCommandPool pool = iter->second;
                StartWriteObject(pool, loc);
            }
        }
        c_VkCommandBuffer.StartWrite(object, loc);
    }
    void FinishWriteObject(VkCommandBuffer object, const Location& loc, bool lockPool = true) {
        c_VkCommandBuffer.FinishWrite(object, loc);
        if (lockPool) {
            auto iter = command_pool_map.find(object);
            if (iter != command_pool_map.end()) {
                VkCommandPool pool = iter->second;
                FinishWriteObject(pool, loc);
            }
        }
    }
    void StartReadObject(VkCommandBuffer object, const Location& loc) {
        auto iter = command_pool_map.find(object);
        if (iter != command_pool_map.end()) {
            VkCommandPool pool = iter->second;
            // We set up a read guard against the "Contents" counter to catch conflict vs. vkResetCommandPool and
            // vkDestroyCommandPool while *not* establishing a read guard against the command pool counter itself to avoid false
            // positive for non-externally sync'd command buffers
            c_VkCommandPoolContents.StartRead(pool, loc);
        }
        c_VkCommandBuffer.StartRead(object, loc);
    }
    void FinishReadObject(VkCommandBuffer object, const Location& loc) {
        c_VkCommandBuffer.FinishRead(object, loc);
        auto iter = command_pool_map.find(object);
        if (iter != command_pool_map.end()) {
            VkCommandPool pool = iter->second;
            c_VkCommandPoolContents.FinishRead(pool, loc);
        }
    }

#include "generated/thread_safety_device_defs.h"
};
}  // namespace threadsafety
