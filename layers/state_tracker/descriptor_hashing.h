/* Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
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

#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <vector>
#include <sstream>
#include <string>
#include "containers/custom_containers.h"

struct Location;
namespace vvl {
class DeviceState;

// This is our custom GPU friendly version of
//    vvl::unordered_map<uint64_t, Entry> map;
// where we only need support insert(), erase(), and find()
struct DescriptorHashTable {
    // Special hash key values to know when
    constexpr static uint64_t EMPTY = 0;                          // never set
    constexpr static uint64_t TOMBSTONE = 0xFFFFFFFFFFFFFFFFULL;  // been erased

    // Buffer, TexelBuffer, and AccelerationStructure
    struct EntryBuffer {
        VkDeviceAddressRangeEXT range;
    };
    struct EntryImage {
        VkImage image;
        VkFormat format;
        VkImageViewType type;
    };
    struct EntrySampler {};
    struct EntryNull {};
    constexpr static uint32_t NULL_DESCRIPTOR_MASK = 0x80000000;
    struct Entry {
        // It is possible two different types are the same descriptor hash, so need a bit mask here
        // each type is applied here as
        //    1 << vvlDescriptorType
        // Will be updated if a second type is found
        uint32_t types;
        bool HasType(VkDescriptorType vk_type) const;
        bool IsNullDescriptor() const;

        union Data {
            EntryBuffer buffer;
            EntryImage image;
            EntrySampler sampler;
            EntryNull null_descriptor;

            explicit Data(EntryBuffer b) : buffer(b) {}
            explicit Data(EntryImage i) : image(i) {}
            explicit Data(EntrySampler s) : sampler(s) {}
            explicit Data(EntryNull n) : null_descriptor(n) {}
            Data() {}
        } data;

        // Pass in vvlDescriptorType
        Entry(uint8_t t, EntryBuffer b) : types(1 << t), data(b) {}
        Entry(uint8_t t, EntryImage i) : types(1 << t), data(i) {}
        Entry(uint8_t t, EntrySampler s) : types(1 << t), data(s) {}
        Entry(uint8_t t, EntryNull n) : types((1 << t) | NULL_DESCRIPTOR_MASK), data(n) {}
        // allow us to resize the vector
        Entry() : types(0), data() {}

        void Describe(const DeviceState& device_state, std::ostringstream& ss) const;
    };

    // Represents single "slot" in our linear buffer allocation
    struct Slot {
        uint64_t key;
        alignas(8) Entry entry;
    };
    // Want to keep 32-byte aligned for better GPU read accessing
    static_assert(sizeof(Slot) == 32, "DescriptorHashTable::Slot exceeds the 32-byte limit!");

    // So we don't need to update the GPU everytime if nothing changed
    // want it marked dirty so first time we still copy something over to the GPU
    bool dirty = true;

    // Only want to report once if user hits our static limit
    bool limit_reported = false;

    std::vector<Slot> slots;
    const uint32_t capacity;

    // std::unordered_map style calls
    bool Insert(uint64_t key, const Entry& value, const DeviceState& dev_data, const Location& loc);
    void Erase(uint64_t key);
    const Entry* Find(uint64_t key) const;
    size_t Size() const { return capacity * sizeof(Slot); }

    explicit DescriptorHashTable(uint32_t capacity);
};

struct DescriptorHashing {
    explicit DescriptorHashing(uint32_t capacity);
    ~DescriptorHashing() {};

    DescriptorHashTable table;
    // Users can pass in VkDebugUtilsObjectNameInfoEXT when getting the descriptor to provide a name
    // We make a seperate map to prevent adding 32-bytes to every Entry as this might not be used
    vvl::unordered_map<uint64_t, std::string> debug_names;
    mutable std::shared_mutex map_lock;

    uint64_t Hash(const void* descriptor_ptr, const VkDeviceSize descriptor_size) const;

    std::string Describe(const DeviceState& device_state, uint64_t key) const;

    // Information found at device creation only used for descriptor hashing
    VkDeviceSize heap_max_descriptor_size = 0;
    std::vector<VkDeviceSize> heap_all_descriptor_sizes;
    VkDeviceSize buffer_max_descriptor_size = 0;
    std::vector<VkDeviceSize> buffer_all_descriptor_sizes;
    bool null_descriptor_allowed = false;
};

}  // namespace vvl
