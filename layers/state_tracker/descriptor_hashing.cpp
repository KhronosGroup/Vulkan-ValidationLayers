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

#include "descriptor_hashing.h"
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include "containers/limits.h"
#include "error_message/error_location.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/state_tracker.h"
#include "utils/descriptor_utils.h"

namespace vvl {

DescriptorHashTable::DescriptorHashTable(uint32_t capacity) : capacity(capacity) {
    slots.resize(capacity, {DescriptorHashTable::EMPTY, {}});
}

// This is a basic "Linear Probing" hashmap
// (https://www.geeksforgeeks.org/dsa/implementing-hash-table-open-addressing-linear-probing-cpp/)
// Where we use the fact the capacity is a Power Of Two in order to quickly find the bucket
//
// The two goals are:
// 1. We can statically allocate the size
// 2. It is simple to implement find() on the GPU
//
// When we get more content, we can profile and get real world numbers
// and decide if we want to change up the implementation then.
//
// The idea is if we have 4 slots it starts as
//
// |   0   |   1   |   2   |   3   |
// | EMPTY | EMPTY | EMPTY | EMPTY |
//
// and the if the first entry is index is 1, we add it and now have
//
// |   0   |   1   |   2   |   3   |
// | EMPTY |   A   | EMPTY | EMPTY |
//
// but if the next item is index 1 as well we "linear probe" until
// we find an open slot
//
// |   0   |   1   |   2   |   3   |
// | EMPTY |   A   |   B   | EMPTY |
//
// This is easy to find on the GPU (in GLSL) and hopefully we only have to
// linear probe a few slots, but GPU normally load a row of memory so we might
// get some cache hits if that happens
//
// returns true if added
bool DescriptorHashTable::Insert(uint64_t key, const Entry& entry, const DeviceState& dev_data, const Location& loc) {
    // Keep to 32-bit to match the GPU version
    // (which needs 32-bit indexing into array)
    // We know capacity is powe-of-two and much faster to use & over a %
    const uint64_t raw_index = key & (static_cast<uint64_t>(capacity) - 1ULL);
    assert(raw_index <= vvl::kU32Max);

    uint32_t slot_index = static_cast<uint32_t>(raw_index);
    const uint32_t start_index = slot_index;
    uint32_t first_tombstone_index = vvl::kNoIndex32;

    while (slots[slot_index].key != EMPTY) {
        Slot& slot = slots[slot_index];
        if (slot.key == key) {
            // It is possible that 2 descriptor of different types create the same descriptor
            // In this case, we can ignore the Entry (because they should be the same "group" of descriptor)
            // and just append the type
            if (((slot.entry.types & entry.types) & ~NULL_DESCRIPTOR_MASK) == 0) {
                slot.entry.types |= entry.types;
                dirty = true;
                return true;  // considered added
            }
            slot.entry = entry;
            return false;
        }

        // If we see a tombstone the key might be right after, so save and only use if the key doesn't exist
        if (slot.key == TOMBSTONE && first_tombstone_index == vvl::kNoIndex32) {
            first_tombstone_index = slot_index;
        }

        // Linear Probe
        // We know capacity is powe-of-two and much faster to use & over a %
        slot_index = (slot_index + 1) & (capacity - 1);

        if (slot_index == start_index) {
            if (first_tombstone_index == vvl::kNoIndex32 && !limit_reported) {
                limit_reported = true;
                dev_data.LogError("DESCRIPTOR-HASHING-LIMIT", {}, loc,
                                  "The internal descriptor_hashing hash map capacity is %" PRIu32
                                  " which has been reached and no more descriptors hashes can be saved.\nThis can be adjusted "
                                  "setting VK_LAYER_DESCRIPTOR_HASHING_TOTAL_DESCRIPTORS (descriptor_hashing_total_descriptors) to "
                                  "larger value (suggest %" PRIu32 ")",
                                  capacity, capacity << 1);
                // We "could" just update the newer descriptor hash as it should be used,
                // but better to just have the user increase the hashmap size
                return false;
            }
            break;
        }
    }

    // If the key is brand new, place it in the first available slot
    const uint32_t new_index = (first_tombstone_index == vvl::kNoIndex32) ? slot_index : first_tombstone_index;
    slots[new_index] = Slot{key, entry};
    dirty = true;
    return true;
}

// When we erase we replace the spot with TOMBSTONE
// The one down side of this is over time we could fragment and cause longer linear probing
void DescriptorHashTable::Erase(uint64_t key) {
    const uint64_t raw_index = key & (static_cast<uint64_t>(capacity) - 1ULL);
    assert(raw_index <= vvl::kU32Max);

    uint32_t slot_index = static_cast<uint32_t>(raw_index);
    const uint32_t start_index = slot_index;

    while (slots[slot_index].key != EMPTY) {
        if (slots[slot_index].key == key) {
            slots[slot_index].key = TOMBSTONE;
            dirty = true;
            return;
        }

        // Linear Probe
        slot_index = (slot_index + 1) & (capacity - 1);
        if (slot_index == start_index) {
            break;
        }
    }
}

// CPU implementation of find()
// Designed to be simple so we can reproduce in GLSL for the GPU
const DescriptorHashTable::Entry* DescriptorHashTable::Find(uint64_t key) const {
    const uint64_t raw_index = key & (static_cast<uint64_t>(capacity) - 1ULL);
    assert(raw_index <= vvl::kU32Max);

    uint32_t slot_index = static_cast<uint32_t>(raw_index);
    const uint32_t start_index = slot_index;

    while (slots[slot_index].key != EMPTY) {
        if (slots[slot_index].key == key) {
            return &slots[slot_index].entry;
        }

        // Linear probe
        slot_index = (slot_index + 1) & (capacity - 1);
        if (slot_index == start_index) {
            break;
        }
    }
    return nullptr;
}

bool DescriptorHashTable::Entry::HasType(VkDescriptorType vk_type) const {
    return (types & (1 << (uint8_t)GetMaskFromDescriptorType(vk_type))) != 0;
}

bool DescriptorHashTable::Entry::IsNullDescriptor() const { return (types & DescriptorHashTable::NULL_DESCRIPTOR_MASK) != 0; }

DescriptorHashing::DescriptorHashing(uint32_t capacity) : table(capacity) {}

// We will want a 64-bit hash, not 32-bit because we can expect 100k unique descriptors in a lifetime of an app and don't want a
// hash collision. This is also designed to be simple that we can duplicate this on the GPU in GLSL as well
uint64_t DescriptorHashing::Hash(const void* descriptor_ptr, const VkDeviceSize descriptor_size) const {
    // the descriptor is not guaranteed to be a power of 2, but will be a multiple of 4
    const uint32_t* dword_ptr = reinterpret_cast<const uint32_t*>(descriptor_ptr);
    // API uses VkDeviceSize, but size is max 256
    const uint32_t dword_count = (uint32_t)descriptor_size / 4;

    // MurmurHash3 fmix64 Avalanche Mixer has algorithm
    uint64_t hash = 14695981039346656037ULL;  // FNV-1a 64-bit basis
    for (uint64_t i = 0; i < dword_count; ++i) {
        hash ^= static_cast<uint64_t>(dword_ptr[i]);
        hash *= 1099511628211ULL;  // FNV-1a 64-bit prime
    }
    hash ^= hash >> 33;
    hash *= 0xff51afd7ed558ccdULL;
    hash ^= hash >> 33;
    hash *= 0xc4ceb9fe1a85ec53ULL;
    hash ^= hash >> 33;

    return (hash == DescriptorHashTable::EMPTY || hash == DescriptorHashTable::TOMBSTONE) ? 1 : hash;
}

std::string DescriptorHashing::Describe(const DeviceState& device_state, uint64_t key) const {
    const DescriptorHashTable::Entry* entry = table.Find(key);
    if (!entry) {
        assert(false);
        return "[Bad Key]";
    }

    std::ostringstream ss;
    entry->Describe(device_state, ss);

    auto debug_it = debug_names.find(key);
    if (debug_it != debug_names.end()) {
        ss << " [" << debug_it->second << "]";
    }

    return ss.str();
}

void DescriptorHashTable::Entry::Describe(const DeviceState& device_state, std::ostringstream& ss) const {
    vvlDescriptorType vvl_type = vvlDescriptorType::Invalid;

    bool first = true;
    // Loop all possible vvlDescriptorType (as might be multiple types)
    for (uint8_t i = 0; i < vvlDescriptorMaxBit; i++) {
        if (types & (1 << i)) {
            // ok to set the second time (if more than one type)
            // as they should be in the same group below
            vvl_type = static_cast<vvlDescriptorType>(i);
            VkDescriptorType vk_type = GetDescriptorTypeFromMask(vvl_type);
            if (!first) {
                ss << " | ";
            }
            ss << string_VkDescriptorType(vk_type);
            first = false;
        }
    }

    auto list_buffers = [&device_state, &ss](VkDeviceAddress address) {
        auto buffer_states = device_state.GetBuffersByAddress(address);
        if (buffer_states.empty()) {
            ss << "[No VkBuffer found] ";
        }
        for (uint32_t i = 0; i < buffer_states.size(); i++) {
            if (i != 0) {
                ss << " | ";
            }
            auto& buffer_state = buffer_states[i];
            ss << buffer_state->Describe(device_state);
        }
        return !buffer_states.empty();
    };

    switch (vvl_type) {
        case vvlDescriptorType::UniformBuffer:
        case vvlDescriptorType::StorageBuffer:
        case vvlDescriptorType::ImageTexelBufferUniform:
        case vvlDescriptorType::ImageTexelBufferStorage: {
            ss << ", ";
            const VkDeviceAddressRangeEXT& range = data.buffer.range;
            // If we find buffers, just print that instead of the address range
            const bool found = list_buffers(range.address);
            if (!found) {
                ss << "address 0x" << std::hex << range.address << ", size " << std::dec << range.size;
            }
            break;
        }
        case vvlDescriptorType::Sampler:
            // currently nothing extra to print
            break;
        case vvlDescriptorType::ImageSampled:
        case vvlDescriptorType::ImageStorage:
        case vvlDescriptorType::ImageInputAttachment: {
            const auto& image = data.image;
            ss << ", " << string_VkImageViewType(image.type) << ", " << string_VkFormat(image.format) << ", "
               << device_state.FormatHandle(image.image);
            break;
        }
        case vvlDescriptorType::AccelerationStructure: {
            const VkDeviceAddressRangeEXT& range = data.buffer.range;
            ss << ", address 0x" << std::hex << range.address << ", size " << std::dec << range.size;
            break;
        }
        case vvlDescriptorType::CombinedSampler:
        case vvlDescriptorType::Invalid:
            assert(false);  // this should not be hit
            break;
    }
}

}  // namespace vvl