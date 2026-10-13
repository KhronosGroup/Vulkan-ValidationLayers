/* Copyright (c) 2026 Valve Corporation
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

#include "containers/container_utils.h"
#include "generated/spirv_grammar_helper.h"
#include "gpu_dump_state.h"
#include "gpu_dump.h"
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <spirv-tools/libspirv.hpp>
#include <spirv-tools/optimizer.hpp>
#include <spirv/unified1/spirv.hpp>
#include <sstream>
#include <algorithm>
#include <vector>
#include "containers/range.h"
#include "containers/small_vector.h"
#include "containers/custom_containers.h"
#include "containers/limits.h"
#include "generated/error_location_helper.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/descriptor_hashing.h"
#include "state_tracker/pipeline_layout_state.h"
#include "state_tracker/pipeline_state.h"
#include "state_tracker/shader_instruction.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/shader_stage_state.h"
#include "state_tracker/state_tracker.h"
#include "utils/lock_utils.h"
#include "utils/math_utils.h"
#include "utils/descriptor_utils.h"
#include "utils/spirv_tools_utils.h"

namespace gpudump {

// Only for descriptors using the mapping API
// create a struct here so we can gather the information and sort it
struct MappingInfo {
    const VkDescriptorSetAndBindingMappingEXT* mapping;
    uint32_t index;  // into pMappings
    // pointers so we can do a sort
    const spirv::ResourceInterfaceVariable* resource_variable;

    // Sort by binding
    bool operator<(const MappingInfo& other) const {
        return resource_variable->decorations.binding < other.resource_variable->decorations.binding;
    }
};

// Single "view" of the heap
// One for Resource and one for Sampler heap
struct HeapView {
    const vvl::range<VkDeviceAddress>& range;
    const vvl::range<VkDeviceAddress>& reserved;
    VkDescriptorType descriptor_type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    VkDeviceSize descriptor_size = 0;
    VkDeviceSize alignment = 0;
    vvl::Field alignment_name = vvl::Field::Empty;
    bool is_sampler_heap = false;
    bool is_combined_sampler = false;

    // When using useCombinedImageSamplerIndex
    uint32_t CombinedIndex(uint32_t index) const { return is_combined_sampler ? ((index >> 20) & 0xFFF) : (index & 0xFFFFF); }
};

// Data to be used around all dumping logic
struct DumpInfo {
    GpuDump& dev_data;
    const VkPhysicalDeviceDescriptorHeapPropertiesEXT& heap_props;
    const spirv::ResourceInterfaceVariable& resource_variable;
    const bool is_sampler;
    const bool is_embedded_sampler;

    const uint32_t binding_offset;
    const bool is_alias;

    const bool combined_index;

    const bool is_array;
    const bool is_runtime_array;
    const uint32_t array_length;

    HeapView heap;
    // The sampler of a combined image sampler, only used if |inspect_sampler|
    HeapView sampler_heap;

    // Will be false if mapping uses embedded samplers instead
    bool inspect_sampler = false;

    DumpInfo(GpuDump& dev_data, const MappingInfo& mapping_info, const vvl::CommandBuffer::DescriptorHeap& cb_heap)
        : dev_data(dev_data),
          heap_props(dev_data.phys_dev_ext_props.descriptor_heap_props),
          resource_variable(*mapping_info.resource_variable),
          is_sampler(resource_variable.is_sampler),
          is_embedded_sampler(GetEmbeddedSampler(*mapping_info.mapping) != nullptr),
          binding_offset(resource_variable.decorations.binding - mapping_info.mapping->firstBinding),
          is_alias(binding_offset != 0),
          combined_index(resource_variable.is_combined_image_sampler && HasCombinedImageSamplerIndex(*mapping_info.mapping)),
          is_array(resource_variable.IsArray()),
          is_runtime_array(resource_variable.IsRuntimeArray()),
          array_length((is_array && !is_runtime_array && resource_variable.array_length != spirv::kSpecConstant)
                           ? resource_variable.array_length
                           : 0),
          heap{is_sampler ? cb_heap.sampler_range : cb_heap.resource_range,
               is_sampler ? cb_heap.sampler_reserved : cb_heap.resource_reserved},
          sampler_heap{cb_heap.sampler_range, cb_heap.sampler_reserved} {
        heap.descriptor_type = resource_variable.GetPotentialDescriptorType();
        if (heap.descriptor_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
            assert(resource_variable.is_combined_image_sampler);
            // not valid to query this type, we just want the "resource" portion as the sampler is handled itself later
            heap.descriptor_type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            heap.descriptor_size = heap_props.imageDescriptorSize;
        } else if (heap.descriptor_type != VK_DESCRIPTOR_TYPE_MAX_ENUM) {
            heap.descriptor_size = dev_data.device_state->cached_descriptor_size.GetSize(heap.descriptor_type);
        }
        heap.alignment = GetDescriptorHeapAlignment(heap_props, heap.descriptor_type);
        heap.alignment_name = GetDescriptorHeapAlignmentField(heap.descriptor_type);
        heap.is_sampler_heap = is_sampler;

        sampler_heap.descriptor_type = VK_DESCRIPTOR_TYPE_SAMPLER;
        sampler_heap.descriptor_size = heap_props.samplerDescriptorSize;
        sampler_heap.alignment = heap_props.samplerDescriptorAlignment;
        sampler_heap.alignment_name = vvl::Field::samplerDescriptorAlignment;
        sampler_heap.is_sampler_heap = true;
        sampler_heap.is_combined_sampler = true;

        inspect_sampler = resource_variable.is_combined_image_sampler && !is_embedded_sampler;
    }
};

// If returns null, should be caught by another error
static const vvl::Buffer* GetLargestBuffer(const GpuDump& dev_data, uint64_t address) {
    auto buffer_states = dev_data.GetBuffersByAddress(address);
    const vvl::Buffer* longer_buffer = nullptr;
    VkDeviceAddress max_end = 0;
    for (auto& buffer_state : buffer_states) {
        VkDeviceAddress new_end = buffer_state->DeviceAddressRange().end;
        if (new_end > max_end) {
            max_end = new_end;
            longer_buffer = buffer_state;
        }
    }
    return longer_buffer;
}

const char* new_bullet_line = "\n      - ";
const char* new_sub_line = "\n            ";

// Holds all the warnings we will find along the way
struct WarnInfo {
    std::ostringstream ss;
    bool found = false;
    uint32_t reserved_range_start = vvl::kNoIndex32;
    uint32_t reserved_range_end = vvl::kNoIndex32;

    const DumpInfo& dump;
    WarnInfo(const DumpInfo& dump) : dump(dump) {}

    bool FoundWarning() { return found; }

    void HeapOOB(const HeapView& heap, VkDeviceSize offset);
    void AlignmentScalarIndirect(VkDeviceAddress address, VkDeviceSize alignment);
    void IndirectUniformUsage(VkDeviceAddress address);
    void ResourceBufferUsage(VkDeviceAddress address);
    void AlignmentHeapUBO(VkDeviceAddress address);
    void AlignmentIndirectAddress(VkDeviceAddress address);
    void AlignmentDescriptor(const HeapView& heap, VkDeviceAddress address);
    void ArrayStride(const HeapView& heap, uint32_t array_stride);
    void IndexOOB(VkDeviceSize first_oob_index);
    void IndexArray(std::vector<uint32_t>& bad_indexes);
    void AlignmentIndexArray(const HeapView& heap, std::vector<uint32_t>& bad_indexes);
    void ReservedRangeIndexArray(std::vector<uint32_t>& bad_indexes);
    void FindReservedIndexes(const HeapView& heap, VkDeviceAddress index_zero_address, uint32_t array_stride, uint32_t index_count);
    void ReservedRangeFinal();
    std::string ValidateDescriptor(const HeapView& heap, VkDeviceAddress address);
};

void WarnInfo::HeapOOB(const HeapView& heap, VkDeviceSize offset) {
    if (offset > heap.range.size()) {
        found = true;
        ss << new_bullet_line << "[WARNING] OUT OF BOUNDS - descriptor is not in the "
           << (heap.is_sampler_heap ? "sampler" : "resource") << " heap and any access to this descriptor will be invalid";
    }
};

void WarnInfo::AlignmentScalarIndirect(VkDeviceAddress address, VkDeviceSize alignment) {
    if (!IsPointerAligned(address, alignment)) {
        found = true;
        ss << new_bullet_line << "[WARNING] MISALIGNED - the indirect address is not aligned to ";
        if (alignment == 4) {
            ss << "4 (scalar alignment for a uint32_t)";
        } else if (alignment == 8) {
            ss << "8 (scalar alignment for a VkDeviceAddress)";
        } else {
            assert(false);
        }
        ss << " and any access to this descriptor will be invalid";
    }
};

void WarnInfo::IndirectUniformUsage(VkDeviceAddress address) {
    auto buffer_states = dump.dev_data.GetBuffersByAddress(address);
    // warning elsewhere if this is empty
    if (!buffer_states.empty()) {
        bool buffer_found = false;
        for (const auto& buffer_state : buffer_states) {
            if (buffer_state->usage & VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT) {
                buffer_found = true;
                break;
            }
        }
        if (!buffer_found) {
            found = true;
            if (buffer_states.size() == 1) {
                const vvl::Buffer& buffer_state = *buffer_states.front();
                ss << new_bullet_line << "[WARNING] BUFFER TYPE - the indirect address (" << Hex(address) << ") belongs to "
                   << buffer_state.Describe(dump.dev_data) << ", but that VkBuffer was created with usage "
                   << string_VkBufferUsageFlags2(buffer_state.usage)
                   << " (missing VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT) and any access to this descriptor will be "
                      "invalid";
            } else {
                ss << new_bullet_line << "[WARNING] BUFFER TYPE - the indirect address (" << Hex(address)
                   << ") is not accessing any VkBuffer created with VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT and any access to "
                      "this descriptor will be invalid";
            }
        }
    }
};

void WarnInfo::ResourceBufferUsage(VkDeviceAddress address) {
    // Core validation ensures this for the given mappings
    auto buffer_states = dump.dev_data.GetBuffersByAddress(address);
    // warning elsewhere if this is empty
    if (!buffer_states.empty()) {
        const bool is_storage_buffer = dump.resource_variable.is_storage_buffer;
        const VkBufferUsageFlagBits2 search_usage =
            is_storage_buffer ? VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT : VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT;
        bool buffer_found = false;
        for (const auto& buffer_state : buffer_states) {
            if (buffer_state->usage & search_usage) {
                buffer_found = true;
                break;
            }
        }
        if (!buffer_found) {
            found = true;
            ss << new_bullet_line << "[WARNING] BUFFER TYPE - the resource address (" << Hex(address)
               << ") is not accessing any VkBuffer created with "
               << (is_storage_buffer ? "VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT" : "VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT")
               << " and any access to this descriptor will be invalid";
        }
    }
};

void WarnInfo::AlignmentHeapUBO(VkDeviceAddress address) {
    VkDeviceSize alignment = dump.dev_data.phys_dev_props.limits.minUniformBufferOffsetAlignment;
    if (!IsPointerAligned(address, alignment)) {
        found = true;
        ss << new_bullet_line << "[WARNING] MISALIGNED - the heap address is not aligned to minUniformBufferOffsetAlignment ("
           << alignment << ") and any access to this descriptor will be invalid";
    }
};

void WarnInfo::AlignmentIndirectAddress(VkDeviceAddress address) {
    VkDeviceSize alignment = 0;
    const char* alignment_name = nullptr;
    if (dump.resource_variable.is_uniform_buffer) {
        alignment = dump.dev_data.phys_dev_props.limits.minUniformBufferOffsetAlignment;
        alignment_name = "minUniformBufferOffsetAlignment";
    } else if (dump.resource_variable.is_storage_buffer) {
        alignment = dump.dev_data.phys_dev_props.limits.minStorageBufferOffsetAlignment;
        alignment_name = "minStorageBufferOffsetAlignment";
    } else if (dump.resource_variable.is_acceleration_structure) {
        // TODO - confirm
        alignment = 256;
    }

    if (alignment != 0 && !IsPointerAligned(address, alignment)) {
        found = true;
        ss << new_bullet_line << "[WARNING] MISALIGNED - the resource address is not aligned to ";
        if (alignment_name) {
            ss << alignment_name << " ";
        }
        ss << "(" << alignment << ") and any access to this descriptor will be invalid";
    }
};

void WarnInfo::AlignmentDescriptor(const HeapView& heap, VkDeviceAddress address) {
    if (heap.alignment != 0 && !IsPointerAligned(address, heap.alignment)) {
        found = true;
        ss << new_bullet_line << "[WARNING] MISALIGNED - the final address";
        if (dump.resource_variable.IsArray()) {
            ss << " (of the first element of the array)";
        }
        ss << " is not aligned to ";
        if (heap.alignment_name == vvl::Field::Empty) {
            ss << "[UNKNOWN type]";
        } else {
            ss << String(heap.alignment_name) << " ";
        }
        ss << "(" << heap.alignment << ") and any access to this descriptor will be invalid";
    }
};

void WarnInfo::ArrayStride(const HeapView& heap, uint32_t array_stride) {
    if (array_stride == 0) {
        found = true;
        ss << new_bullet_line << "[WARNING] ZERO ARRAY STRIDE - "
           << (heap.is_combined_sampler ? "samplerHeapArrayStride" : "heapArrayStride")
           << " is zero, this means every index of the descriptor array will be the same descriptor, which is likely not "
              "desired.";
    }
};

void WarnInfo::IndexOOB(VkDeviceSize first_oob_index) {
    if (dump.array_length > first_oob_index) {
        found = true;
        ss << new_bullet_line << "[WARNING] OUT OF BOUNDS - descriptor has an array length of [" << dump.array_length
           << "] but any element starting at index [" << first_oob_index
           << "] will be out of bounds of the heap and any access to those indexes will be invalid";
    }
};

static void PrintIndexes(std::ostringstream& ss, const std::vector<uint32_t>& indexes) {
    for (uint32_t i = 0; i < indexes.size(); i++) {
        if (i != 0) ss << ", ";
        ss << indexes[i];
    }
}

void WarnInfo::IndexArray(std::vector<uint32_t>& bad_indexes) {
    if (!bad_indexes.empty()) {
        found = true;
        ss << new_bullet_line << "[WARNING] OUT OF BOUNDS - descriptor indexes at [";
        PrintIndexes(ss, bad_indexes);
        ss << "] will be out of bounds of the heap and any access to those indexes will be invalid";
    }
};

void WarnInfo::AlignmentIndexArray(const HeapView& heap, std::vector<uint32_t>& bad_indexes) {
    if (!bad_indexes.empty()) {
        found = true;
        ss << new_bullet_line << "[WARNING] MISALIGNED - descriptor indexes at [";
        PrintIndexes(ss, bad_indexes);
        ss << "] will not be aligned to ";
        if (heap.alignment_name != vvl::Field::Empty) {
            ss << String(heap.alignment_name) << " ";
        }
        ss << "(" << heap.alignment << ") and any access to those indexes will be invalid";
    }
};

void WarnInfo::ReservedRangeIndexArray(std::vector<uint32_t>& bad_indexes) {
    if (!bad_indexes.empty()) {
        found = true;
        ss << new_bullet_line << "[WARNING] RESERVED RANGE - descriptor indexes at [";
        PrintIndexes(ss, bad_indexes);
        ss << "] will overlap with the reserved range and any access to those indexes will be invalid";
    }
};

// Finds the descriptor indexes in [0, index_count) that overlap with the reserved range
// For a combined image sampler, only the first heap found to overlap is reported.
void WarnInfo::FindReservedIndexes(const HeapView& heap, VkDeviceAddress index_zero_address, uint32_t array_stride,
                                   uint32_t index_count) {
    if (heap.reserved.empty() || reserved_range_start != vvl::kNoIndex32) {
        return;
    }

    if (!dump.is_array) {
        vvl::range<VkDeviceAddress> index_zero_range{index_zero_address, index_zero_address + heap.descriptor_size};
        if (index_zero_range.intersects(heap.reserved)) {
            reserved_range_start = 0;
        }
        return;
    }

    if (index_count == 0 || index_zero_address >= heap.reserved.end) {
        return;
    }

    // Index covers [index_zero_address + i * array_stride, index_zero_address + i * array_stride + descriptor_size)
    // overlaps with the reserved range if
    //     index_zero_address + i * array_stride < reserved.end
    //     index_zero_address + i * array_stride + descriptor_size > reserved.begin
    // The indexes that overlap are contiguous, so calculate the first and last one instead of checking every index
    //  - a runtime array is checked up to the end of the heap
    //  - descriptor size of zero still overlaps if it starts in the reserved range
    const VkDeviceAddress index_zero_end = index_zero_address + std::max<VkDeviceSize>(heap.descriptor_size, 1);
    VkDeviceSize first_index = 0;
    VkDeviceSize last_index = index_count - 1;
    if (array_stride == 0) {
        // Every index is index zero
        if (index_zero_end <= heap.reserved.begin) {
            return;
        }
    } else {
        if (index_zero_end <= heap.reserved.begin) {
            first_index = ((heap.reserved.begin - index_zero_end) / array_stride) + 1;
        }
        last_index = std::min(last_index, (heap.reserved.end - 1 - index_zero_address) / array_stride);
        if (first_index > last_index) {
            return;  // the array ends before the reserved range, or the reserved range is between two descriptors
        }
    }
    reserved_range_start = (uint32_t)first_index;
    reserved_range_end = (uint32_t)last_index;
}

// Found in FindReservedIndexes()
void WarnInfo::ReservedRangeFinal() {
    if (reserved_range_start != vvl::kNoIndex32) {
        // If using a runtime array, the user was likely not going to access the reserved range
        // It is still worth informing them, but not as a warning
        // (Don't do for ReservedRangeIndexArray as the user should be actively setting those elements)
        if (dump.is_runtime_array) {
            ss << new_bullet_line << "[INFO] ";
        } else {
            found = true;
            ss << new_bullet_line << "[WARNING] ";
        }

        ss << "RESERVED RANGE - ";
        if (reserved_range_start == reserved_range_end) {
            ss << "descriptor index at [" << reserved_range_start
               << "] will overlap with the reserved range and any access to that index will be invalid";
        } else if (reserved_range_end != vvl::kNoIndex32) {
            ss << "descriptor index starting at [" << reserved_range_start << "] to [" << reserved_range_end
               << "] will overlap with the reserved range and any access to those indexes will be invalid";
        } else {
            ss << "this descriptor overlaps with the reserved range and any access to this descriptor will be invalid";
        }
    }
};

std::string WarnInfo::ValidateDescriptor(const HeapView& heap, VkDeviceAddress address) {
    if (!dump.dev_data.global_settings.descriptor_hashing) {
        return "";
    }
    if (FoundWarning()) {
        return "";  // no point checking if we know something else is wrong
    }
    const VkDeviceSize descriptor_size = heap.descriptor_size;
    const auto& descriptor_hashing = *dump.dev_data.device_state->descriptor_hashing;
    ReadLockGuard guard(descriptor_hashing.map_lock);
    if (descriptor_hashing.table.limit_reported) {
        // If hit limit, likely to detect false positives
        return "";
    }

    // For VK_EXT_descriptor_heap where each descriptor type might be a different size.
    //
    // For example, if a UBO is 32 bytes, but an image is 64 bytes:
    // If we grab the heap data from [N, N+31] and hash it, we have no way to know if it would have
    // really been an Image hash if we have done [N, N+63]
    //
    // We need a way to re-hash to find the key for a potential different type
    bool check_larger_sizes = true;
    std::vector<uint8_t> descriptor_bytes = dump.dev_data.CopyDataFromMemory(address, descriptor_hashing.heap_max_descriptor_size);
    if (descriptor_bytes.empty()) {
        if (descriptor_size < descriptor_hashing.heap_max_descriptor_size) {
            descriptor_bytes = dump.dev_data.CopyDataFromMemory(address, descriptor_size);
        }
        if (descriptor_bytes.empty()) {
            return "";  // heap buffer might not be visible
        } else {
            // this means we are at the end of the heap buffer and can only check the exact size (or smaller)
            check_larger_sizes = false;
        }
    }

    uint64_t key = descriptor_hashing.Hash(descriptor_bytes.data(), descriptor_size);
    const vvl::DescriptorHashTable::Entry* entry = descriptor_hashing.table.Find(key);
    if (!entry) {
        // Before saying we can't find it, check the other descriptor sizes if we can spot the wrong descriptor type
        for (VkDeviceSize size : descriptor_hashing.heap_all_descriptor_sizes) {
            if (size == descriptor_size) {
                continue;
            } else if (!check_larger_sizes && size > descriptor_size) {
                continue;
            }
            key = descriptor_hashing.Hash(descriptor_bytes.data(), size);
            entry = descriptor_hashing.table.Find(key);
            if (entry) {
                break;
            }
        }
        if (!entry) {
            found = true;
            ss << new_bullet_line << "[WARNING] NO DESCRIPTOR - No known descriptor found in the heap at " << Hex(address);
            return "";
        }
    }

    const VkDescriptorType descriptor_type = heap.descriptor_type;
    if (entry->IsNullDescriptor()) {
        if (descriptor_hashing.null_descriptor_allowed) {
            return "[Null Descriptor]";
        }
        found = true;
        ss << new_bullet_line
           << "[WARNING] NO DESCRIPTOR - Found descriptor mapped to [Null Descriptor] but the nullDescriptor feature was not "
              "enabled";
        return "";
    } else if (!entry->HasType(descriptor_type)) {
        found = true;
        ss << new_bullet_line << "[WARNING] WRONG DESCRIPTOR - At " << Hex(address) << " expected a "
           << string_VkDescriptorType(descriptor_type)
           << " descriptor, but instead found a descriptor for: " << descriptor_hashing.Describe(*dump.dev_data.device_state, key);
        return "";
    }

    return descriptor_hashing.Describe(*dump.dev_data.device_state, key);
}

VkDeviceSize CommandBufferSubState::GetPushData(std::ostringstream& ss, WarnInfo& warn, uint32_t offset, uint32_t size) const {
    VkDeviceSize push_index = 0;
    ss << new_bullet_line << "pushData[" << offset << ":" << offset + (size - 1) << "] = ";

    // This is caught by core validation already, but don't want to crash here
    const VkDeviceSize max_push_data_size = warn.dump.heap_props.maxPushDataSize;
    if ((offset + size) > max_push_data_size) {
        ss << "UNDEFINED";
        warn.ss << new_bullet_line << "[WARNING] PUSH DATA - [" << offset << ":" << (offset + size) - 1
                << "] is over the maxPushDataSize (" << max_push_data_size << ") limit";
        warn.found = true;
        return push_index;  // something to not blow up other calculations too bad
    } else if (push_data_value.empty() || !base.VerifyPushData(offset, size)) {
        // |push_data_value| is empty if vkCmdPushData was never called
        ss << "UNDEFINED";
        warn.ss << new_bullet_line << "[WARNING] PUSH DATA - [" << offset << ":" << (offset + size) - 1
                << "] has not been set by vkCmdPushDataEXT and the value being read is undefined";
        warn.found = true;
        return push_index;  // value being tracked in state tracking is undefined
    }

    if (size == 4) {
        push_index = (VkDeviceSize) * ((uint32_t*)&push_data_value[offset]);
        ss << push_index;
    } else if (size == 8) {
        push_index = (VkDeviceSize) * ((VkDeviceAddress*)&push_data_value[offset]);
        ss << Hex(push_index);
    }

    return push_index;
}

static VkDeviceSize AddBindingOffset(std::ostringstream& ss, const DumpInfo& dump, VkDeviceSize offset, uint32_t array_stride) {
    if (dump.is_alias) {
        offset += (VkDeviceSize)array_stride * dump.binding_offset;
        ss << new_bullet_line << "binding offset: " << dump.binding_offset << " * " << array_stride;
    }
    return offset;
}

// Prints how a descriptor array is laid out in the heap and finds which indexes are out of bounds or in the reserved range
static void DumpDescriptorArray(std::ostringstream& ss, WarnInfo& warn, const HeapView& heap, VkDeviceSize index_zero_offset,
                                uint32_t array_stride) {
    const DumpInfo& dump = warn.dump;
    const VkDeviceAddress index_zero_address = heap.range.begin + index_zero_offset;
    warn.ArrayStride(heap, array_stride);

    // The first index that is not fully inside the heap
    constexpr VkDeviceSize not_obb = std::numeric_limits<VkDeviceSize>::max();
    VkDeviceSize first_oob_index = 0;
    if (index_zero_offset + heap.descriptor_size > heap.range.size()) {
        first_oob_index = 0;  // even index zero is not in the heap
    } else if (array_stride == 0) {
        first_oob_index = not_obb;  // every index is the same descriptor as index zero
    } else {
        const VkDeviceSize available_space = heap.range.size() - index_zero_offset - heap.descriptor_size;
        first_oob_index = (available_space / array_stride) + 1;
    }

    if (dump.array_length != 0) {
        const VkDeviceSize final_array_offset = (VkDeviceSize)(dump.array_length - 1) * array_stride;
        vvl::range<VkDeviceAddress> final_index_range{index_zero_address + final_array_offset,
                                                      index_zero_address + final_array_offset + heap.descriptor_size};
        ss << new_sub_line << "The final descriptor index at [" << dump.array_length - 1 << "] will access "
           << string_range_hex(final_index_range);
        warn.IndexOOB(first_oob_index);
    } else if (dump.is_runtime_array && first_oob_index != not_obb) {
        ss << new_sub_line << "The descriptor runtime array will be out of bounds of the heap starting at index ["
           << first_oob_index << "]";
    }

    // Runtime arrays are only searched up to the end of the heap
    uint32_t index_count = dump.array_length;
    if (index_count == 0) {
        index_count = first_oob_index == not_obb ? 1 : (uint32_t)std::min<VkDeviceSize>(first_oob_index, vvl::kU32Max);
    }
    warn.FindReservedIndexes(heap, index_zero_address, array_stride, index_count);
}

// Prints where index zero of the descriptor is in the heap and checks it (and the rest of the array)
static void DumpHeapLocation(std::ostringstream& ss, WarnInfo& warn, const HeapView& heap, VkDeviceSize index_zero_offset,
                             uint32_t array_stride, bool check_alignment) {
    const DumpInfo& dump = warn.dump;
    const VkDeviceAddress index_zero_address = heap.range.begin + index_zero_offset;
    ss << new_bullet_line << (heap.is_sampler_heap ? "Sampler" : "Resource") << " heap address: " << Hex(index_zero_address);
    if (check_alignment) {
        warn.AlignmentDescriptor(heap, index_zero_address);
    }

    if (dump.is_array) {
        ss << " + (descriptor_index * " << array_stride << ")";
        DumpDescriptorArray(ss, warn, heap, index_zero_offset, array_stride);
    } else {
        warn.FindReservedIndexes(heap, index_zero_address, 0, 1);
    }
    warn.HeapOOB(heap, index_zero_offset + heap.descriptor_size);

    // Currently only will look at the descriptor if not an array as it is common to have a large array where only the indexes
    // accessed will have a valid descriptor ready.
    // Also want at the end after everything has been checked so we don't try and access invalid memory
    if (!dump.is_array) {
        std::string descriptor_entry = warn.ValidateDescriptor(heap, index_zero_address);
        if (!descriptor_entry.empty()) {
            ss << new_bullet_line << "Found descriptor mapped to: " << descriptor_entry;
        }
    }
}

void CommandBufferSubState::DumpDescriptorHeapConstantOffset(std::ostringstream& ss, DumpInfo& dump, WarnInfo& warn,
                                                             const VkDescriptorMappingSourceConstantOffsetEXT& map_data) const {
    ss << "heapOffset: " << Hex(map_data.heapOffset) << ", heapArrayStride: " << map_data.heapArrayStride;

    VkDeviceSize index_zero_offset = AddBindingOffset(ss, dump, map_data.heapOffset, map_data.heapArrayStride);

    // CONSTANT_OFFSET will be fully caught already in normal VVL if alignment is off
    DumpHeapLocation(ss, warn, dump.heap, index_zero_offset, map_data.heapArrayStride, false);

    if (dump.inspect_sampler) {
        ss << new_bullet_line << "samplerHeapOffset: " << Hex(map_data.samplerHeapOffset)
           << ", samplerHeapArrayStride: " << map_data.samplerHeapArrayStride;

        index_zero_offset = AddBindingOffset(ss, dump, map_data.samplerHeapOffset, map_data.samplerHeapArrayStride);
        DumpHeapLocation(ss, warn, dump.sampler_heap, index_zero_offset, map_data.samplerHeapArrayStride, false);
    }
}

static uint32_t PrintCombinedIndex(std::ostringstream& ss, const HeapView& heap, const char* name, uint32_t index) {
    ss << name << ": ";
    if (heap.is_combined_sampler) {
        ss << "((" << index << " >> 20) & 0xFFF) ";
    } else {
        ss << "(" << index << " & 0xFFFFF) ";
    }
    index = heap.CombinedIndex(index);
    ss << "[" << index << "]";
    return index;
}

void CommandBufferSubState::DumpDescriptorHeapPushIndexHeap(std::ostringstream& ss, WarnInfo& warn, const HeapView& heap,
                                                            uint32_t push_offset, uint32_t heap_offset, uint32_t index_stride,
                                                            uint32_t array_stride) const {
    const DumpInfo& dump = warn.dump;
    uint32_t push_index = (uint32_t)GetPushData(ss, warn, push_offset, 4);
    if (dump.combined_index) {
        if (heap.is_combined_sampler) {
            ss << " (using pushOffset)";
        }
        ss << new_sub_line;
        push_index = PrintCombinedIndex(ss, heap, "pushIndex", push_index);
    }
    ss << new_bullet_line << "pushIndex offset: " << push_index << " * " << index_stride;

    const VkDeviceSize index_zero_offset =
        AddBindingOffset(ss, dump, heap_offset + ((VkDeviceSize)push_index * index_stride), array_stride);
    DumpHeapLocation(ss, warn, heap, index_zero_offset, array_stride, true);
}

void CommandBufferSubState::DumpDescriptorHeapPushIndex(std::ostringstream& ss, DumpInfo& dump, WarnInfo& warn,
                                                        const VkDescriptorMappingSourcePushIndexEXT& map_data) const {
    ss << "pushOffset: " << map_data.pushOffset << ", heapOffset: " << Hex(map_data.heapOffset)
       << ", heapIndexStride: " << map_data.heapIndexStride << ", heapArrayStride: " << map_data.heapArrayStride;
    DumpDescriptorHeapPushIndexHeap(ss, warn, dump.heap, map_data.pushOffset, map_data.heapOffset, map_data.heapIndexStride,
                                    map_data.heapArrayStride);

    if (dump.inspect_sampler) {
        ss << new_bullet_line << "samplerPushOffset: " << map_data.samplerPushOffset
           << ", samplerHeapOffset: " << Hex(map_data.samplerHeapOffset)
           << ", samplerHeapIndexStride: " << map_data.samplerHeapIndexStride
           << ", samplerHeapArrayStride: " << map_data.samplerHeapArrayStride
           << ", useCombinedImageSamplerIndex: " << (map_data.useCombinedImageSamplerIndex ? "true" : "false");
        DumpDescriptorHeapPushIndexHeap(
            ss, warn, dump.sampler_heap, dump.combined_index ? map_data.pushOffset : map_data.samplerPushOffset,
            map_data.samplerHeapOffset, map_data.samplerHeapIndexStride, map_data.samplerHeapArrayStride);
    }
}

void CommandBufferSubState::DumpDescriptorHeapIndirectIndexHeap(std::ostringstream& ss, WarnInfo& warn, const HeapView& heap,
                                                                uint32_t push_offset, uint32_t address_offset, uint32_t heap_offset,
                                                                uint32_t index_stride, uint32_t array_stride) const {
    const DumpInfo& dump = warn.dump;
    const bool using_combined_index = dump.combined_index && heap.is_combined_sampler;
    const VkDeviceAddress push_indirect_address = GetPushData(ss, warn, push_offset, 8);
    if (using_combined_index) {
        ss << " (using pushOffset)";
    }

    const VkDeviceAddress final_indirect_address = push_indirect_address + address_offset;
    ss << new_bullet_line << "indirectAddress: " << Hex(final_indirect_address) << " (" << Hex(push_indirect_address) << " + "
       << Hex(address_offset) << ")";
    if (using_combined_index) {
        ss << " (using addressOffset)";
    }

    warn.AlignmentScalarIndirect(final_indirect_address, 4);
    warn.IndirectUniformUsage(final_indirect_address);
    warn.found |= dev_data.ListBuffers(ss, final_indirect_address, 3, true);

    std::vector<uint8_t> indirect_index_data = dev_data.CopyDataFromMemory(final_indirect_address, 4);
    const bool know_index = !indirect_index_data.empty();
    uint32_t indirect_index = know_index ? *((uint32_t*)indirect_index_data.data()) : 0;
    if (know_index) {
        if (dump.combined_index) {
            ss << new_bullet_line;
            indirect_index = PrintCombinedIndex(ss, heap, "indirectIndex", indirect_index);
        }
        ss << new_bullet_line << "indirectIndex offset: " << indirect_index << " * " << index_stride;
    } else {
        ss << new_bullet_line << "indirectIndex offset: UNKNOWN (can't read buffer)";
    }

    const VkDeviceSize index_zero_offset =
        AddBindingOffset(ss, dump, heap_offset + ((VkDeviceSize)indirect_index * index_stride), array_stride);
    if (know_index) {
        DumpHeapLocation(ss, warn, heap, index_zero_offset, array_stride, true);
    } else {
        // Can only check that the start of the mapping is inside the heap
        ss << new_bullet_line << (heap.is_sampler_heap ? "Sampler" : "Resource")
           << " heap address: " << Hex(heap.range.begin + index_zero_offset);
        if (dump.is_array) {
            ss << " + (descriptor_index * " << array_stride << ")";
        }
        warn.HeapOOB(heap, index_zero_offset + heap.descriptor_size);
    }
}

void CommandBufferSubState::DumpDescriptorHeapIndirectIndex(std::ostringstream& ss, DumpInfo& dump, WarnInfo& warn,
                                                            const VkDescriptorMappingSourceIndirectIndexEXT& map_data) const {
    ss << "pushOffset: " << map_data.pushOffset << ", addressOffset: " << Hex(map_data.addressOffset)
       << ", heapOffset: " << Hex(map_data.heapOffset) << ", heapIndexStride: " << map_data.heapIndexStride
       << ", heapArrayStride: " << map_data.heapArrayStride;
    DumpDescriptorHeapIndirectIndexHeap(ss, warn, dump.heap, map_data.pushOffset, map_data.addressOffset, map_data.heapOffset,
                                        map_data.heapIndexStride, map_data.heapArrayStride);

    if (dump.inspect_sampler) {
        ss << new_bullet_line << "samplerPushOffset: " << map_data.samplerPushOffset
           << ", samplerAddressOffset: " << Hex(map_data.samplerAddressOffset)
           << ", samplerHeapOffset: " << Hex(map_data.samplerHeapOffset)
           << ", samplerHeapIndexStride: " << map_data.samplerHeapIndexStride
           << ", samplerHeapArrayStride: " << map_data.samplerHeapArrayStride
           << ", useCombinedImageSamplerIndex: " << (map_data.useCombinedImageSamplerIndex ? "true" : "false");
        DumpDescriptorHeapIndirectIndexHeap(
            ss, warn, dump.sampler_heap, dump.combined_index ? map_data.pushOffset : map_data.samplerPushOffset,
            dump.combined_index ? map_data.addressOffset : map_data.samplerAddressOffset, map_data.samplerHeapOffset,
            map_data.samplerHeapIndexStride, map_data.samplerHeapArrayStride);
    }
}

void CommandBufferSubState::DumpDescriptorHeapIndirectIndexArrayHeap(std::ostringstream& ss, WarnInfo& warn, const HeapView& heap,
                                                                     uint32_t push_offset, uint32_t address_offset,
                                                                     uint32_t heap_offset, uint32_t index_stride) const {
    const DumpInfo& dump = warn.dump;
    const bool using_combined_index = dump.combined_index && heap.is_combined_sampler;
    const VkDeviceAddress push_indirect_address = GetPushData(ss, warn, push_offset, 8);
    if (using_combined_index) {
        ss << " (using pushOffset)";
    }

    // Aliased bindings start further into the indirect index array
    const VkDeviceSize binding_offset = sizeof(uint32_t) * dump.binding_offset;
    const VkDeviceAddress final_indirect_address = push_indirect_address + address_offset + binding_offset;
    if (dump.is_alias) {
        ss << new_bullet_line << "binding offset: " << dump.binding_offset << " * sizeof(uint32_t)";
    }

    ss << new_bullet_line << "indirectAddress: " << Hex(final_indirect_address) << " (" << Hex(push_indirect_address) << " + "
       << Hex(address_offset);
    if (dump.is_alias) {
        ss << " + " << Hex(binding_offset);
    }
    ss << ")";
    if (using_combined_index) {
        ss << " (using addressOffset)";
    }

    warn.AlignmentScalarIndirect(final_indirect_address, 4);
    warn.IndirectUniformUsage(final_indirect_address);
    warn.found |= dev_data.ListBuffers(ss, final_indirect_address, 3, true);

    ss << new_bullet_line << (heap.is_sampler_heap ? "Sampler" : "Resource")
       << " heap address: " << Hex(heap.range.begin + heap_offset) << " + (indirectIndex * " << index_stride << ")";

    if (const vvl::Buffer* buffer_state = GetLargestBuffer(dev_data, final_indirect_address)) {
        uint32_t available_bytes = (uint32_t)(buffer_state->DeviceAddressRange().end - final_indirect_address);
        uint32_t available_slots = available_bytes / sizeof(uint32_t);
        // We can assume this is an array, otherwise, not sure why people are using this mapping
        uint32_t search_slots = !dump.is_array ? 1 : dump.is_runtime_array ? available_slots : dump.array_length;

        if (dump.array_length != 0 && dump.array_length > available_slots) {
            warn.IndexOOB(available_slots);
            search_slots = available_slots;
        }
        uint32_t search_bytes = search_slots * sizeof(uint32_t);

        std::vector<uint8_t> indirect_index_data = dev_data.CopyDataFromMemory(final_indirect_address, search_bytes);
        bool know_ubo = !indirect_index_data.empty();

        if (dump.is_runtime_array) {
            // Currently don't go searching in, no way to know desired upper bound
            ss << new_sub_line << "Any descriptor index starting at [" << search_slots
               << "] will be invalid as there are no more values found for indirectIndex inside "
               << dev_data.FormatHandle(buffer_state->Handle());
        }

        if (know_ubo) {
            uint32_t* indirect_index_words = (uint32_t*)indirect_index_data.data();
            if (!dump.is_array) {
                uint32_t indirect_index = indirect_index_words[0];
                if (dump.combined_index) {
                    ss << new_sub_line;
                    indirect_index = PrintCombinedIndex(ss, heap, "indirectIndex", indirect_index);
                }

                VkDeviceSize final_offset = heap_offset + ((VkDeviceSize)indirect_index * index_stride);
                VkDeviceAddress final_address = heap.range.begin + final_offset;
                warn.AlignmentDescriptor(heap, final_address);
                warn.HeapOOB(heap, final_offset + heap.descriptor_size);

                ss << new_sub_line << "Final address: " << Hex(final_address) << " (indirectIndex: " << indirect_index << ")";
            } else if (!dump.is_runtime_array) {
                // Runtime arrays are unbounded and there is no idea where to stop looking,
                // can add if people find valuable.
                ss << new_bullet_line << "indirectIndex values from buffer: [";
                std::vector<uint32_t> bad_array_indexes;
                std::vector<uint32_t> bad_alignment_indexes;
                std::vector<uint32_t> bad_reserve_indexes;
                for (uint32_t i = 0; i < search_slots; i++) {
                    uint32_t current_index_value = indirect_index_words[i];
                    if (dump.combined_index) {
                        current_index_value = heap.CombinedIndex(current_index_value);
                    }

                    VkDeviceSize final_offset = heap_offset + ((VkDeviceSize)current_index_value * index_stride);
                    if (final_offset + heap.descriptor_size > heap.range.size()) {
                        bad_array_indexes.emplace_back(i);
                    }
                    if (i != 0) ss << ", ";
                    ss << current_index_value;

                    VkDeviceAddress next_index_address = heap.range.begin + final_offset;
                    if (heap.alignment != 0 && !IsPointerAligned(next_index_address, heap.alignment)) {
                        bad_alignment_indexes.emplace_back(i);
                    }

                    if (!heap.reserved.empty()) {
                        vvl::range<VkDeviceAddress> next_index_range{next_index_address, next_index_address + heap.descriptor_size};
                        if (next_index_range.intersects(heap.reserved)) {
                            bad_reserve_indexes.emplace_back(i);
                        }
                    }
                }
                warn.IndexArray(bad_array_indexes);
                warn.AlignmentIndexArray(heap, bad_alignment_indexes);
                warn.ReservedRangeIndexArray(bad_reserve_indexes);
                ss << "]";
                if (dump.combined_index) {
                    ss << new_sub_line << "Applied "
                       << (heap.is_combined_sampler ? "((indirectIndex >> 20) & 0xFFF)" : "(indirectIndex & 0xFFFFF)")
                       << " to each";
                }
            }
        }
    } else {
        assert(warn.found);
    }
    warn.HeapOOB(heap, heap_offset + heap.descriptor_size);
}

void CommandBufferSubState::DumpDescriptorHeapIndirectIndexArray(
    std::ostringstream& ss, DumpInfo& dump, WarnInfo& warn, const VkDescriptorMappingSourceIndirectIndexArrayEXT& map_data) const {
    ss << "pushOffset: " << map_data.pushOffset << ", addressOffset: " << Hex(map_data.addressOffset)
       << ", heapOffset: " << Hex(map_data.heapOffset) << ", heapIndexStride: " << map_data.heapIndexStride;
    DumpDescriptorHeapIndirectIndexArrayHeap(ss, warn, dump.heap, map_data.pushOffset, map_data.addressOffset, map_data.heapOffset,
                                             map_data.heapIndexStride);

    if (dump.inspect_sampler) {
        ss << new_bullet_line << "samplerPushOffset: " << map_data.samplerPushOffset
           << ", samplerAddressOffset: " << Hex(map_data.samplerAddressOffset)
           << ", samplerHeapOffset: " << Hex(map_data.samplerHeapOffset)
           << ", samplerHeapIndexStride: " << map_data.samplerHeapIndexStride
           << ", useCombinedImageSamplerIndex: " << (map_data.useCombinedImageSamplerIndex ? "true" : "false");

        DumpDescriptorHeapIndirectIndexArrayHeap(ss, warn, dump.sampler_heap,
                                                 dump.combined_index ? map_data.pushOffset : map_data.samplerPushOffset,
                                                 dump.combined_index ? map_data.addressOffset : map_data.samplerAddressOffset,
                                                 map_data.samplerHeapOffset, map_data.samplerHeapIndexStride);
    }
}

void CommandBufferSubState::DumpDescriptorHeapHeapData(std::ostringstream& ss, DumpInfo& dump, WarnInfo& warn,
                                                       const VkDescriptorMappingSourceHeapDataEXT& map_data) const {
    ss << "pushOffset: " << map_data.pushOffset << ", heapOffset: " << Hex(map_data.heapOffset);
    VkDeviceSize push_data = GetPushData(ss, warn, map_data.pushOffset, 4);

    ss << new_bullet_line << (dump.is_sampler ? "Sampler" : "Resource")
       << " heap address: " << Hex(dump.heap.range.begin + map_data.heapOffset) << " + " << Hex(push_data);
    VkDeviceAddress final_offset = map_data.heapOffset + push_data;
    VkDeviceAddress final_address = dump.heap.range.begin + final_offset;
    ss << new_sub_line << "Final address: " << Hex(final_address);

    warn.AlignmentHeapUBO(final_address);
    warn.HeapOOB(dump.heap, final_offset + dump.heap.descriptor_size);

    warn.FindReservedIndexes(dump.heap, final_address, 0, 1);
}

void CommandBufferSubState::DumpDescriptorHeapPushAddress(std::ostringstream& ss, WarnInfo& warn,
                                                          uint32_t pushAddressOffset) const {
    ss << "pushAddressOffset: " << pushAddressOffset;
    VkDeviceSize indirect_address = GetPushData(ss, warn, pushAddressOffset, 8);

    warn.AlignmentIndirectAddress(indirect_address);

    if (warn.dump.resource_variable.is_acceleration_structure) {
        warn.found |= dev_data.ListAccelerationStructures(ss, indirect_address, 3, true);
    } else {
        warn.ResourceBufferUsage(indirect_address);
        warn.found |= dev_data.ListBuffers(ss, indirect_address, 3, true);
    }
}

void CommandBufferSubState::DumpDescriptorHeapIndirectAddress(std::ostringstream& ss, DumpInfo& dump, WarnInfo& warn,
                                                              const VkDescriptorMappingSourceIndirectAddressEXT& map_data) const {
    ss << "pushOffset: " << map_data.pushOffset << ", addressOffset: " << Hex(map_data.addressOffset);
    VkDeviceSize push_indirect_address = GetPushData(ss, warn, map_data.pushOffset, 8);

    VkDeviceAddress final_indirect_address = push_indirect_address + map_data.addressOffset;

    ss << new_bullet_line << "indirectAddress: " << Hex(final_indirect_address) << " (" << Hex(push_indirect_address) << " + "
       << Hex(map_data.addressOffset) << ")";

    warn.AlignmentScalarIndirect(final_indirect_address, 8);
    warn.IndirectUniformUsage(final_indirect_address);
    warn.found |= dev_data.ListBuffers(ss, final_indirect_address, 3, true);

    std::vector<uint8_t> indirect_address_data = dev_data.CopyDataFromMemory(final_indirect_address, 8);
    if (!indirect_address_data.empty()) {
        const VkDeviceAddress resource_address = *((VkDeviceAddress*)indirect_address_data.data());
        ss << new_bullet_line << "Resource address: " << Hex(resource_address);

        warn.AlignmentIndirectAddress(resource_address);

        if (warn.dump.resource_variable.is_acceleration_structure) {
            warn.found |= dev_data.ListAccelerationStructures(ss, resource_address, 3, true);
        } else {
            warn.ResourceBufferUsage(resource_address);
            warn.found |= dev_data.ListBuffers(ss, resource_address, 3, true);
        }
    }
}

void CommandBufferSubState::DumpDescriptorHeapShaderRecordIndex(
    std::ostringstream& ss, DumpInfo& dump, WarnInfo& warn, const VkDescriptorMappingSourceShaderRecordIndexEXT& map_data) const {
    // Reading shader record data is not really possible:
    // it depends on what ray tracing shader is actually executed
    ss << "heapOffset: " << Hex(map_data.heapOffset) << ", shaderRecordOffset: " << map_data.shaderRecordOffset
       << ", heapIndexStride: " << map_data.heapIndexStride << ", heapArrayStride: " << map_data.heapArrayStride;

    if (dump.inspect_sampler) {
        ss << new_bullet_line << "samplerHeapOffset: " << Hex(map_data.samplerHeapOffset)
           << ", samplerShaderRecordOffset: " << map_data.samplerShaderRecordOffset
           << ", samplerHeapIndexStride: " << map_data.samplerHeapIndexStride
           << ", samplerHeapArrayStride: " << map_data.samplerHeapArrayStride;
    }
}

void CommandBufferSubState::DumpDescriptorHeapShaderRecordAddress(std::ostringstream& ss,
                                                                  uint32_t shaderRecordAddressOffset) const {
    // Reading shader record data is not really possible:
    // it depends on what ray tracing shader is actually executed
    ss << "shaderRecordAddressOffset: " << shaderRecordAddressOffset;
}

bool CommandBufferSubState::DumpDescriptorHeapMapping(std::ostringstream& ss, const MappingInfo& mapping_info) const {
    const VkDescriptorSetAndBindingMappingEXT& mapping = *mapping_info.mapping;

    DumpInfo dump(dev_data, mapping_info, base.descriptor_heap);
    WarnInfo warn(dump);

    ss << "    - SPIR-V Binding " << dump.resource_variable.decorations.binding;
    if (!dump.resource_variable.debug_name.empty()) {
        ss << " [\"" << dump.resource_variable.debug_name << "\"]";
    }

    ss << new_bullet_line << "specified in pMappings[" << mapping_info.index << "] - "
       << string_VkDescriptorMappingSourceEXT(mapping.source);
    if (dump.is_alias) {
        ss << " (firstBinding: " << mapping.firstBinding << ", bindingCount: " << mapping.bindingCount << ")";
        if (dump.is_array) {
            ss << new_sub_line << "starting at descriptor index [" << dump.binding_offset << "]";
        }
    }

    if (dump.is_embedded_sampler) {
        ss << new_bullet_line << "Embedded Sampler\n";
        return false;
    }

    ss << new_bullet_line;
    if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT) {
        const VkDescriptorMappingSourceConstantOffsetEXT& map_data = mapping.sourceData.constantOffset;
        DumpDescriptorHeapConstantOffset(ss, dump, warn, map_data);
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT) {
        const VkDescriptorMappingSourcePushIndexEXT& map_data = mapping.sourceData.pushIndex;
        DumpDescriptorHeapPushIndex(ss, dump, warn, map_data);
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
        const VkDescriptorMappingSourceIndirectIndexEXT& map_data = mapping.sourceData.indirectIndex;
        DumpDescriptorHeapIndirectIndex(ss, dump, warn, map_data);
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
        const VkDescriptorMappingSourceIndirectIndexArrayEXT& map_data = mapping.sourceData.indirectIndexArray;
        DumpDescriptorHeapIndirectIndexArray(ss, dump, warn, map_data);
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT) {
        const VkDescriptorMappingSourceHeapDataEXT& map_data = mapping.sourceData.heapData;
        DumpDescriptorHeapHeapData(ss, dump, warn, map_data);
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT) {
        ss << "pushDataOffset: " << mapping.sourceData.pushDataOffset;
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT) {
        DumpDescriptorHeapPushAddress(ss, warn, mapping.sourceData.pushAddressOffset);
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT) {
        const VkDescriptorMappingSourceIndirectAddressEXT& map_data = mapping.sourceData.indirectAddress;
        DumpDescriptorHeapIndirectAddress(ss, dump, warn, map_data);
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT) {
        const VkDescriptorMappingSourceShaderRecordIndexEXT& map_data = mapping.sourceData.shaderRecordIndex;
        DumpDescriptorHeapShaderRecordIndex(ss, dump, warn, map_data);
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_DATA_EXT) {
        ss << "shaderRecordDataOffset: " << mapping.sourceData.shaderRecordDataOffset;
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_ADDRESS_EXT) {
        DumpDescriptorHeapShaderRecordAddress(ss, mapping.sourceData.shaderRecordAddressOffset);
    }

    // These mappings don't read a descriptor from the heap
    const bool no_descriptor =
        IsValueIn(mapping.source,
                  {VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT,
                   VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT,
                   VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_DATA_EXT, VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_ADDRESS_EXT});
    if (dump.heap.descriptor_type != VK_DESCRIPTOR_TYPE_MAX_ENUM && !no_descriptor) {
        if (dump.inspect_sampler) {
            ss << new_bullet_line << "Descriptor size: " << dump.heap.descriptor_size << " (imageDescriptorSize) and "
               << dump.sampler_heap.descriptor_size << " (samplerDescriptorSize)";
        } else {
            ss << new_bullet_line << "Descriptor size: " << dump.heap.descriptor_size << " ("
               << string_VkDescriptorType(dump.heap.descriptor_type) << ")";
        }
    }

    warn.ReservedRangeFinal();

    if (!warn.ss.str().empty()) {
        ss << warn.ss.str();
    }

    ss << '\n';

    return warn.FoundWarning();
}

//
// You have hit Untyped Pointer territory, brace yourselves!
//
static const uint32_t kDynamicIndex = vvl::kNoIndex32;
static const uint32_t kNoPushData = vvl::kNoIndex32;
// If there is some math logic to calculate this
static const uint32_t kPushDataRange = vvl::kNoIndex32 - 1;
struct HeapAccess {
    VkDescriptorType descriptor_type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    // These are zero because if not found, it means it is implicitly zero
    // This also allows for better hashes being the same
    uint32_t heap_offset = 0;
    // Might have multidimensional array and need an index for each
    struct Index {
        uint32_t array_stride = 0;
        uint32_t element = kDynamicIndex;
        uint32_t push_data_index = kNoPushData;

        bool operator==(const Index& other) const {
            return array_stride == other.array_stride && element == other.element && push_data_index == other.push_data_index;
        }

        // Required so we can compare std::vector<Index> in tie-breakers
        bool operator<(const Index& other) const { return element < other.element; }
    };
    std::vector<Index> descriptor_indexes;

    // Lets us know this access is done inside a function
    bool function_call = false;

    // Values that are calculated from the above values
    // (So no reason to apply to the hashing)
    bool is_array = false;
    bool is_sampler = false;
    bool dynamic_index = false;
    // Zero so dynamic values are first in the sorting
    VkDeviceSize final_address = 0;

    HeapAccess(const CommandBufferSubState& cb_sub_state, VkDescriptorType descriptor_type, uint32_t heap_offset,
               std::vector<Index> descriptor_indexes, bool function_call)
        : descriptor_type(descriptor_type),
          heap_offset(heap_offset),
          descriptor_indexes(descriptor_indexes),
          function_call(function_call),
          is_array(!descriptor_indexes.empty()),
          is_sampler(descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLER) {
        auto heap_range =
            is_sampler ? cb_sub_state.base.descriptor_heap.sampler_range : cb_sub_state.base.descriptor_heap.resource_range;
        VkDeviceSize final_offset = heap_offset;
        for (const auto& descriptor_index : descriptor_indexes) {
            if (descriptor_index.element == kDynamicIndex) {
                dynamic_index = true;
            } else {
                final_offset += (VkDeviceSize)descriptor_index.array_stride * descriptor_index.element;
            }
        }

        if (!is_array) {
            final_address = heap_range.begin + heap_offset;
        } else if (!dynamic_index) {
            final_address = heap_range.begin + final_offset;
        }
    }

    struct compare {
        bool operator()(HeapAccess const& lhs, HeapAccess const& rhs) const {
            // Main thing want to sort by
            // May change sorting logic here if we find a better way to sort
            if (lhs.final_address != rhs.final_address) {
                return lhs.final_address < rhs.final_address;
            }

            if (lhs.descriptor_type != rhs.descriptor_type) {
                return lhs.descriptor_type < rhs.descriptor_type;
            }
            if (lhs.heap_offset != rhs.heap_offset) {
                return lhs.heap_offset < rhs.heap_offset;
            }
            if (lhs.function_call != rhs.function_call) {
                return lhs.function_call < rhs.function_call;  // false < true
            }
            return lhs.descriptor_indexes < rhs.descriptor_indexes;
        }
    };
};
// std::set so duplicate accesses are merged and the output is sorted by HeapAccess::compare
using HeapAccesses = std::set<HeapAccess, HeapAccess::compare>;

// Tracks all the OpFunction/OpFunctionCall/OpFunctionParameter info we need
struct FunctionInfo {
    struct ParamInfo {
        uint32_t function_id;  // OpFunction
        uint32_t index;        // of the parameters
    };
    // < OpFunctionParameter ID, ParamInfo >
    vvl::unordered_map<uint32_t, ParamInfo> param_map;
    // < OpFunction ID, vector<OpFunctionCall ID>>
    vvl::unordered_map<uint32_t, std::vector<uint32_t>> call_map;
};

struct UntypedContext {
    const CommandBufferSubState& cb_sub_state;
    const vvl::DeviceState& device_state;
    const spirv::Module& module;
    const spirv::EntryPoint& entrypoint;
    const VkPhysicalDeviceDescriptorHeapPropertiesEXT& props;

    // The idea is to go through and find all the access and afterwards sort/group them to make sense to the user
    // < variable id, [accesses]>
    vvl::unordered_map<uint32_t, HeapAccesses> accesses;

    FunctionInfo function_info;

    UntypedContext(const CommandBufferSubState& cb_sub_state, GpuDump& dev_data, const spirv::Module& module,
                   const spirv::EntryPoint& entrypoint)
        : cb_sub_state(cb_sub_state),
          device_state(*dev_data.device_state),
          module(module),
          entrypoint(entrypoint),
          props(dev_data.phys_dev_ext_props.descriptor_heap_props) {
        if (module.static_data_.has_untyped_pointer_function_params) {
            uint32_t current_function = 0;
            uint32_t param_index = 0;
            for (const spirv::Instruction& inst : module.static_data_.instructions) {
                if (inst.Opcode() == spv::OpFunction) {
                    current_function = inst.ResultId();
                    param_index = 0;
                } else if (inst.Opcode() == spv::OpFunctionParameter) {
                    function_info.param_map[inst.ResultId()] = FunctionInfo::ParamInfo{current_function, param_index};
                    param_index++;
                } else if (inst.Opcode() == spv::OpFunctionCall) {
                    function_info.call_map[inst.Word(3)].emplace_back(inst.ResultId());
                }
            }
        }
    }

    uint32_t FindPushDataIndex(const spirv::Instruction& access_inst, uint32_t* out_result);
    void GetUntypedDescriptorIndexes(std::vector<HeapAccess::Index>& out_descriptor_indexes,
                                     std::vector<const spirv::Instruction*>& ac_indexes, const spirv::Instruction& type_array_inst);
    uint32_t GetUntypedArrayStride(uint32_t type_array_id);
    uint32_t GetUntypedHeapOffset(const uint32_t struct_id, uint32_t struct_member_index);

    std::vector<const spirv::Instruction*> FindFunctionCallers(const spirv::Instruction& func_param_inst);
    void AddAccess(const VkDescriptorType descriptor_type, const spirv::Instruction* ac_inst, bool function_call);
    void FindAccess(const spirv::Instruction* next_inst, bool image_access, bool from_function_call);

    bool Print(std::ostringstream& ss, vvl::CommandBuffer& cb_state);
};

uint32_t UntypedContext::GetUntypedHeapOffset(const uint32_t struct_id, uint32_t struct_member_index) {
    for (const auto& type_struct : module.static_data_.type_structs) {
        if (type_struct->id == struct_id) {
            const auto& member = type_struct->members[struct_member_index];
            return member.decorations->GetOffset(module, props);
        }
    }
    assert(false);
    return 0;
}

uint32_t UntypedContext::GetUntypedArrayStride(uint32_t type_array_id) {
    const auto decoration_set = module.GetDecorationSet(type_array_id);
    return decoration_set.GetArrayStride(module, props);
}

// This is a quick, low effort attempt to see if we can detect the index into the descriptor is a push data value
// Anything more than this would require a "proper" solution for static analysis
uint32_t UntypedContext::FindPushDataIndex(const spirv::Instruction& access_inst, uint32_t* out_result) {
    uint32_t push_data_index = kNoPushData;
    if (!entrypoint.push_constant_variable || !entrypoint.push_constant_variable->type_struct_info) {
        return push_data_index;
    }

    // support some basic math int operations
    if (IsValueIn((spv::Op)access_inst.Opcode(), {spv::OpIMul, spv::OpIAdd, spv::OpISub, spv::OpUDiv})) {
        const spirv::Instruction* operand_0_inst = module.FindDef(access_inst.Word(3));
        const spirv::Instruction* operand_1_inst = module.FindDef(access_inst.Word(4));
        uint32_t operand_0_value = 0;
        uint32_t operand_1_value = 0;
        const uint32_t operand_0_index = FindPushDataIndex(*operand_0_inst, &operand_0_value);
        const uint32_t operand_1_index = FindPushDataIndex(*operand_1_inst, &operand_1_value);
        if (operand_0_index == kNoPushData || operand_1_index == kNoPushData) {
            return kNoPushData;
        }
        if (access_inst.Opcode() == spv::OpIMul) {
            *out_result = operand_0_value * operand_1_value;
        } else if (access_inst.Opcode() == spv::OpIAdd) {
            *out_result = operand_0_value + operand_1_value;
        } else if (access_inst.Opcode() == spv::OpISub) {
            *out_result = operand_0_value - operand_1_value;
        } else if (access_inst.Opcode() == spv::OpUDiv) {
            if (operand_1_value == 0) {
                return kNoPushData;
            }
            *out_result = operand_0_value / operand_1_value;
        }
        return (operand_0_index == operand_1_index) ? operand_0_index : kPushDataRange;
    } else if (access_inst.Opcode() == spv::OpConstant) {
        // if here, part of a calculation above
        *out_result = access_inst.GetConstantValue();
        return kPushDataRange;
    } else if (access_inst.Opcode() != spv::OpLoad) {
        return push_data_index;
    }

    if (module.FindDef(access_inst.TypeId())->Opcode() != spv::OpTypeInt) {
        return push_data_index;
    }
    const spirv::Instruction* access_chain_inst = module.FindDef(access_inst.Word(3));
    if (access_chain_inst->Opcode() != spv::OpAccessChain) {
        return push_data_index;
    }
    const spirv::Instruction* pc_var = module.FindDef(access_chain_inst->Word(3));
    if (pc_var->ResultId() != entrypoint.push_constant_variable->id) {
        return push_data_index;
    }
    const spirv::Instruction* pc_member_offset_inst = module.FindDef(access_chain_inst->Word(4));
    if (pc_member_offset_inst->Opcode() != spv::OpConstant) {
        return push_data_index;
    }
    const uint32_t pc_member_offset = pc_member_offset_inst->GetConstantValue();
    if (pc_member_offset >= entrypoint.push_constant_variable->type_struct_info->members.size()) {
        return push_data_index;
    }
    const uint32_t pc_offset = entrypoint.push_constant_variable->type_struct_info->members[pc_member_offset].decorations->offset;
    if (pc_offset == spirv::kInvalidValue) {
        return push_data_index;
    }
    if (pc_offset >= cb_sub_state.push_data_value.size()) {
        return push_data_index;
    }

    assert(out_result);
    *out_result = *((uint32_t*)&cb_sub_state.push_data_value[pc_offset]);
    return pc_offset;
}

void UntypedContext::GetUntypedDescriptorIndexes(std::vector<HeapAccess::Index>& out_descriptor_indexes,
                                                 std::vector<const spirv::Instruction*>& ac_indexes,
                                                 const spirv::Instruction& type_array_inst) {
    assert(type_array_inst.IsArray());
    const uint32_t type_array_id = type_array_inst.ResultId();

    const uint32_t array_stride_value = GetUntypedArrayStride(type_array_id);

    uint32_t element = kDynamicIndex;
    uint32_t push_data_index = kNoPushData;
    if (ac_indexes.empty()) {
        element = 0;  // implicit zero if no AC index is found
    } else {
        const spirv::Instruction* element_inst = ac_indexes.back();
        ac_indexes.pop_back();
        const bool found = module.GetInt32IfConstant(*element_inst, &element);
        if (!found) {
            push_data_index = FindPushDataIndex(*element_inst, &element);
        }
    }

    out_descriptor_indexes.emplace_back(HeapAccess::Index{array_stride_value, element, push_data_index});

    // Keep checking for multidimensional arrays
    const spirv::Instruction* element_type_inst = module.FindDef(type_array_inst.Word(2));
    if (element_type_inst->IsArray()) {
        GetUntypedDescriptorIndexes(out_descriptor_indexes, ac_indexes, *element_type_inst);
    }
}

void UntypedContext::AddAccess(const VkDescriptorType descriptor_type, const spirv::Instruction* ac_inst, bool function_call) {
    if (ac_inst->Opcode() != spv::OpUntypedAccessChainKHR) {
        // will happen when mixing typed (set/binding) and untyped
        // just a dirty way to detect it is not going to be from the heap
        //
        // Also will occur from OpFunctionCall someone passing something else
        return;
    }

    const spirv::Instruction* base_type_inst = module.FindDef(ac_inst->Word(3));
    if (base_type_inst->Opcode() != spv::OpTypeBufferEXT && base_type_inst->Opcode() != spv::OpTypeStruct &&
        !base_type_inst->IsArray()) {
        // Can hit this if using OpUntypedAccessChainKHR on Set/Binding descriptors
        return;
    }

    // walk the Access chains, build up the indexes
    // [0] == farthest from heap
    // [size] == closest to the heap
    std::vector<const spirv::Instruction*> ac_indexes;
    const uint32_t untyped_ac_index_start = 5;
    const spirv::Instruction* base_inst = ac_inst;
    while (base_inst->Opcode() == spv::OpUntypedAccessChainKHR) {
        // there is a chance there are no indexes, that is an implicit 0
        for (uint32_t i = base_inst->Length() - 1; i >= untyped_ac_index_start; i--) {
            ac_indexes.emplace_back(module.FindDef(base_inst->Word(i)));
        }
        base_type_inst = module.FindDef(base_inst->Word(3));
        base_inst = module.FindDef(base_inst->Word(4));
    }
    if (base_inst->Opcode() != spv::OpUntypedVariableKHR) {
        assert(false);
        return;
    }
    const uint32_t variable_id = base_inst->ResultId();

    uint32_t heap_offset = 0;
    std::vector<HeapAccess::Index> descriptor_indexes;

    if (base_type_inst->Opcode() == spv::OpTypeBufferEXT) {
        // single element
    } else if (base_type_inst->Opcode() == spv::OpTypeStruct) {
        const uint32_t struct_id = base_type_inst->ResultId();
        uint32_t struct_member_index = 0;
        if (!ac_indexes.empty()) {
            struct_member_index = ac_indexes.back()->GetConstantValue();
            ac_indexes.pop_back();
        }
        heap_offset = GetUntypedHeapOffset(struct_id, struct_member_index);

        const uint32_t struct_member_id = base_type_inst->Word(2 + struct_member_index);
        const spirv::Instruction* struct_member_insn = module.FindDef(struct_member_id);
        if (struct_member_insn->IsArray()) {
            GetUntypedDescriptorIndexes(descriptor_indexes, ac_indexes, *struct_member_insn);
        }
    } else if (base_type_inst->IsArray()) {
        GetUntypedDescriptorIndexes(descriptor_indexes, ac_indexes, *base_type_inst);
    }

    accesses[variable_id].insert({cb_sub_state, descriptor_type, heap_offset, descriptor_indexes, function_call});
}

std::vector<const spirv::Instruction*> UntypedContext::FindFunctionCallers(const spirv::Instruction& func_param_inst) {
    std::vector<const spirv::Instruction*> callers;
    const spirv::Instruction* param_type = module.FindDef(func_param_inst.TypeId());
    if (param_type->Opcode() == spv::OpTypeUntypedPointerKHR) {
        assert(function_info.param_map.find(func_param_inst.ResultId()) != function_info.param_map.end());
        FunctionInfo::ParamInfo param_info = function_info.param_map[func_param_inst.ResultId()];

        auto func_it = function_info.call_map.find(param_info.function_id);
        // There is a chance no one calls this function, will be dead code eliminated
        if (func_it != function_info.call_map.end()) {
            for (uint32_t function_call_id : func_it->second) {
                const spirv::Instruction* function_call = module.FindDef(function_call_id);
                assert(function_call->Opcode() == spv::OpFunctionCall);
                const uint32_t param_index_operand = function_call->Word(4 + param_info.index);
                const spirv::Instruction* called_param = module.FindDef(param_index_operand);
                callers.emplace_back(called_param);
            }
        }
    }
    return callers;
}

// "Friends don't let friends become compiler engineers" ~Spencer
void UntypedContext::FindAccess(const spirv::Instruction* next_inst, bool image_access, bool from_function_call) {
    if (image_access) {
        const spirv::Instruction* sampler_load_inst = nullptr;

        while (next_inst && (next_inst->Opcode() == spv::OpSampledImage || next_inst->Opcode() == spv::OpImage ||
                             next_inst->Opcode() == spv::OpCopyObject)) {
            if (next_inst->Opcode() == spv::OpSampledImage) {
                sampler_load_inst = module.FindDef(next_inst->Operand(1));
            }
            next_inst = module.FindDef(next_inst->Operand(0));
        }
        const spirv::Instruction* image_load_inst = next_inst;
        if (!image_load_inst || image_load_inst->Opcode() != spv::OpLoad) {
            assert(false);
            return;
        }

        // From here two types of images, sampled and non-sampled images
        if (sampler_load_inst) {
            // TODO - Assertion sampled images are 1D
            const spirv::Instruction* ac_inst = module.FindDef(image_load_inst->Word(3));
            AddAccess(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, ac_inst, from_function_call);

            ac_inst = module.FindDef(sampler_load_inst->Word(3));
            if (ac_inst->Opcode() == spv::OpFunctionParameter) {
                auto callers = FindFunctionCallers(*ac_inst);
                for (const spirv::Instruction* caller : callers) {
                    AddAccess(VK_DESCRIPTOR_TYPE_SAMPLER, caller, true);
                }
            } else {
                AddAccess(VK_DESCRIPTOR_TYPE_SAMPLER, ac_inst, from_function_call);
            }
        } else {
            const spirv::Instruction* image_type = module.FindDef(image_load_inst->TypeId());
            assert(image_type->Opcode() == spv::OpTypeImage);
            const VkDescriptorType image_descriptor_type = image_type->GetImageType();

            const spirv::Instruction* ac_inst = module.FindDef(image_load_inst->Word(3));
            if (ac_inst->Opcode() == spv::OpFunctionParameter) {
                // Not sure this is 100% correct, but only way I could find passing a image
                // descriptor heap as a parameter.
                // If worse case, and this is wrong, we will just not detect accesses.
                auto callers = FindFunctionCallers(*ac_inst);
                for (const spirv::Instruction* caller : callers) {
                    AddAccess(image_descriptor_type, caller, true);
                }
            } else {
                AddAccess(image_descriptor_type, ac_inst, from_function_call);
            }
        }
    } else {
        // We need to walk down possibly multiple chained OpAccessChains
        const spirv::Instruction* next_access_chain = next_inst;
        while (next_access_chain && next_access_chain->IsNonPtrAccessChain()) {
            const uint32_t base_operand = next_access_chain->IsUntypedAccessChain() ? 1 : 0;
            const uint32_t access_chain_base_id = next_access_chain->Operand(base_operand);
            next_access_chain = module.FindDef(access_chain_base_id);
        }
        const spirv::Instruction* base_type = next_access_chain;
        if (!base_type) {
            return;
        }

        if (base_type->Opcode() == spv::OpBufferPointerEXT) {
            // Ignore old BufferBlock + Uniform
            const VkDescriptorType descriptor_type = module.FindDef(base_type->TypeId())->StorageClass() == spv::StorageClassUniform
                                                         ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
                                                         : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

            const spirv::Instruction* ac_inst = module.FindDef(base_type->Word(3));
            AddAccess(descriptor_type, ac_inst, from_function_call);
        } else if (base_type->Opcode() == spv::OpUntypedImageTexelPointerEXT) {
            // Atomic storage image/texel buffer
            const spirv::Instruction* image_type = module.FindDef(base_type->Word(3));
            assert(image_type->Opcode() == spv::OpTypeImage);
            const VkDescriptorType descriptor_type = image_type->GetImageType();

            const spirv::Instruction* ac_inst = module.FindDef(base_type->Word(4));
            AddAccess(descriptor_type, ac_inst, from_function_call);
        } else if (base_type->Opcode() == spv::OpFunctionParameter) {
            auto callers = FindFunctionCallers(*base_type);
            for (const spirv::Instruction* caller : callers) {
                FindAccess(caller, image_access, true);
            }
        }
    }
}

bool UntypedContext::Print(std::ostringstream& ss, vvl::CommandBuffer& cb_state) {
    bool found_warning = false;

    // We print out the variables, but the real "value" comes from scanning the accesses
    // (due to how untyped pointers work)
    for (const spirv::ResourceInterfaceVariable& resource_variable : entrypoint.resource_interface_variables) {
        if (!resource_variable.IsHeap()) {
            continue;
        }
        ss << "  - " << resource_variable.DescribeDescriptor() << " accesses detected:\n";
        auto it = accesses.find(resource_variable.id);
        if (it == accesses.end()) {
            ss << "    - No accesses found to this heap variable\n";
            continue;
        }

        const bool is_sampler = resource_variable.is_sampler_heap;
        auto heap_range = is_sampler ? cb_state.descriptor_heap.sampler_range : cb_state.descriptor_heap.resource_range;
        auto reserve_range = is_sampler ? cb_state.descriptor_heap.sampler_reserved : cb_state.descriptor_heap.resource_reserved;

        for (const HeapAccess& access : it->second) {
            assert(is_sampler == (access.descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLER));

            ss << "    - heap offset: " << Hex(access.heap_offset);

            if (!access.is_array) {
                ss << ", single element\n";
            } else {
                ss << ", array stride: ";
                for (const auto& descriptor_index : access.descriptor_indexes) {
                    ss << "[" << descriptor_index.array_stride << "]";
                }

                ss << ", array index: ";
                // To print multidimensional arrays
                for (const auto& descriptor_index : access.descriptor_indexes) {
                    if (descriptor_index.element == kDynamicIndex) {
                        ss << "[dynamic]";
                    } else {
                        ss << "[" << descriptor_index.element << "]";
                    }
                }

                for (const auto& descriptor_index : access.descriptor_indexes) {
                    if (descriptor_index.element != kDynamicIndex) {
                        if (descriptor_index.push_data_index == kPushDataRange) {
                            ss << " (from vkCmdPushDataEXT values)";
                            break;  // only need once
                        } else if (descriptor_index.push_data_index != vvl::kNoIndex32) {
                            ss << " (from vkCmdPushDataEXT[" << descriptor_index.push_data_index << ":"
                               << descriptor_index.push_data_index + 3 << "])";
                        }
                    }
                }

                if (access.function_call) {
                    // TODO - print the functions passed in
                    ss << " (is a function argument)";
                }

                ss << '\n';
            }

            if (access.final_address != 0) {
                ss << "      - " << (is_sampler ? "Sampler" : "Resource") << " heap address: " << Hex(access.final_address) << '\n';
            }

            const VkDeviceSize descriptor_size = device_state.cached_descriptor_size.GetSize(access.descriptor_type);
            ss << "      - Descriptor size: " << descriptor_size << " (" << string_VkDescriptorType(access.descriptor_type)
               << ")\n";

            // if we know the real address, see if invalid
            if (access.final_address != 0) {
                if (access.final_address + descriptor_size > heap_range.end) {
                    ss << "      - [WARNING] OUT OF BOUNDS - the descriptor is not in the " << (is_sampler ? "sampler" : "resource")
                       << " heap and any access to this descriptor will be invalid\n";
                    found_warning = true;
                }
                if (!reserve_range.empty()) {
                    vvl::range<VkDeviceAddress> final_range{access.final_address, access.final_address + descriptor_size};
                    if (final_range.intersects(reserve_range)) {
                        ss << "      - [WARNING] RESERVED RANGE - this descriptor overlaps with the reserved range and any access "
                              "to this descriptor will be invalid\n";
                        found_warning = true;
                    }
                }

                VkDeviceSize required_alignment = GetDescriptorHeapAlignment(props, access.descriptor_type);
                if (required_alignment != 0 && !IsPointerAligned(access.final_address, required_alignment)) {
                    vvl::Field alignment_name = GetDescriptorHeapAlignmentField(access.descriptor_type);
                    ss << "      - [WARNING] MISALIGNED - the descriptor is not aligned to " << String(alignment_name);
                    ss << " (" << required_alignment << ") and any access to this descriptor will be invalid\n";
                    found_warning = true;
                }
            }
        }
    }
    return found_warning;
}

bool CommandBufferSubState::DumpDescriptorHeapUntyped(std::ostringstream& ss, const ShaderStageState& stage) const {
    std::shared_ptr<const spirv::Module> module_state_ptr = stage.spirv_state;
    std::shared_ptr<const spirv::EntryPoint> entrypoint_ptr = stage.entrypoint;

    // TODO - try to not to have to call spirv-opt here
    auto const& specialization_info = stage.GetSpecializationInfo();
    if (module_state_ptr->static_data_.has_specialization_constants && specialization_info != nullptr &&
        specialization_info->mapEntryCount > 0 && specialization_info->pMapEntries != nullptr) {
        spvtools::OptimizerOptions spirv_options;
        spvtools::Optimizer optimizer(PickSpirvEnv(dev_data.api_version, IsExtEnabled(dev_data.extensions.vk_khr_spirv_1_4)));

        // Gather the specialization-constant values.
        auto const& specialization_data = reinterpret_cast<uint8_t const*>(specialization_info->pData);
        std::unordered_map<uint32_t, std::vector<uint32_t>> id_value_map;
        id_value_map.reserve(specialization_info->mapEntryCount);

        // < spec_id, map_entry_index >
        vvl::unordered_map<uint32_t, uint32_t> spec_constant_data;

        for (const auto& [result_id, spec_id] : module_state_ptr->static_data_.id_to_spec_id) {
            for (uint32_t i = 0; i < specialization_info->mapEntryCount; i++) {
                if (specialization_info->pMapEntries[i].constantID != spec_id) {
                    continue;
                }
                const VkSpecializationMapEntry map_entry = specialization_info->pMapEntries[i];
                if ((map_entry.offset + map_entry.size) <= specialization_info->dataSize) {
                    // Allocate enough room for ceil(map_entry.size / 4) to store entries
                    std::vector<uint32_t> entry_data((map_entry.size + 4 - 1) / 4, 0);
                    uint8_t* out_p = reinterpret_cast<uint8_t*>(entry_data.data());
                    const uint8_t* const start_in_p = specialization_data + map_entry.offset;
                    const uint8_t* const end_in_p = start_in_p + map_entry.size;

                    std::copy(start_in_p, end_in_p, out_p);
                    id_value_map.emplace(map_entry.constantID, std::move(entry_data));
                }
                break;
            }
        }
        optimizer.RegisterPass(spvtools::CreateSetSpecConstantDefaultValuePass(id_value_map));
        optimizer.RegisterPass(spvtools::CreateFreezeSpecConstantValuePass());
        optimizer.RegisterPass(spvtools::CreateFoldSpecConstantOpAndCompositePass());

        std::vector<uint32_t> specialized_spirv;
        auto const optimized =
            optimizer.Run(module_state_ptr->words_.data(), module_state_ptr->words_.size(), &specialized_spirv, spirv_options);
        if (optimized) {
            module_state_ptr =
                std::make_shared<spirv::Module>(vvl::make_span<const uint32_t>(specialized_spirv.data(), specialized_spirv.size()));
            entrypoint_ptr = module_state_ptr->FindEntrypoint(entrypoint_ptr->name.c_str(), entrypoint_ptr->stage);
        }
    }

    UntypedContext context(*this, dev_data, *module_state_ptr, *entrypoint_ptr);

    for (const spirv::Instruction* memory_access_inst : context.entrypoint.accessible.memory_accesses) {
        // Basically there are 2 flows, images and non-images
        uint32_t ptr_id = OpcodeImageAccessPosition(memory_access_inst->Opcode());
        const bool image_access = ptr_id != 0;
        if (image_access) {
            ptr_id = memory_access_inst->Word(ptr_id);
        } else {
            // |Operand 0| works for both Store/Load
            ptr_id = memory_access_inst->Operand(0);
        }
        const spirv::Instruction* next_inst = context.module.FindDef(ptr_id);
        context.FindAccess(next_inst, image_access, false);
    }

    bool found_warning = context.Print(ss, base);

    return found_warning;
}

bool CommandBufferSubState::DumpDescriptorHeap(std::ostringstream& ss, const LastBound& last_bound) const {
    bool found_warning = false;
    const vvl::CommandBuffer& cb_state = last_bound.cb_state;
    if (!cb_state.descriptor_heap.resource_range.empty()) {
        ss << "- vkCmdBindResourceHeapEXT last bound the resource heap to "
           << string_range_hex(cb_state.descriptor_heap.resource_range);
        if (!cb_state.descriptor_heap.resource_reserved.empty()) {
            VkDeviceSize rr_offset =
                cb_state.descriptor_heap.resource_reserved.begin - cb_state.descriptor_heap.resource_range.begin;
            ss << " (reserved range: " << cb_state.descriptor_heap.resource_reserved.size() << " bytes at offset " << rr_offset
               << ")";
        } else {
            ss << " (no reserved range)";
        }
        ss << '\n';
        found_warning |= dev_data.ListBuffers(ss, cb_state.descriptor_heap.resource_range.begin, 1);
    }
    if (!cb_state.descriptor_heap.sampler_range.empty()) {
        ss << "- vkCmdBindSamplerHeapEXT last bound the sampler heap to "
           << string_range_hex(cb_state.descriptor_heap.sampler_range);
        if (!cb_state.descriptor_heap.sampler_reserved.empty()) {
            VkDeviceSize rr_offset = cb_state.descriptor_heap.sampler_reserved.begin - cb_state.descriptor_heap.sampler_range.begin;
            ss << " (reserved range: " << cb_state.descriptor_heap.sampler_reserved.size() << " bytes at offset " << rr_offset
               << ")";
        } else {
            ss << " (no reserved range)";
        }
        ss << '\n';
        found_warning |= dev_data.ListBuffers(ss, cb_state.descriptor_heap.sampler_range.begin, 1);
    }

    if (last_bound.pipeline_state) {
        ss << "- Last bound pipeline: " << dev_data.FormatHandle(last_bound.pipeline_state->Handle()) << " ("
           << string_VkPipelineBindPoint(last_bound.pipeline_state->pipeline_type) << ")\n";
    }

    ss << "- Shader descriptors:\n";
    small_vector<const ShaderStageState*, 3> stages = last_bound.GetStages();
    for (const ShaderStageState* stage : stages) {
        if (!stage->HasSpirv()) {
            ss << "  - [No SPIR-V found for " << string_VkShaderStageFlagBits(stage->GetStage())
               << ", can't detect which descriptors are being accessed]\n";
            continue;
        }
        const spirv::EntryPoint& entry_point = *stage->entrypoint;
        ss << "  " << entry_point.Describe();
        // TODO - add util in ShaderStageState to get ShaderObject handle here
        if (stage->module_state && stage->module_state->VkHandle() != VK_NULL_HANDLE) {
            ss << " " << dev_data.FormatHandle(stage->module_state->Handle());
        }
        ss << "\n";

        const auto* mapping_info = vku::FindStructInPNextChain<VkShaderDescriptorSetAndBindingMappingInfoEXT>(stage->GetPNext());

        // Want to sort and print all the mappings for a given Set together
        std::map<uint32_t, std::vector<MappingInfo>> mapping_info_map;

        // possible to have a mix-and-match
        bool has_non_heap = false;
        bool has_heap = false;
        for (const spirv::ResourceInterfaceVariable& resource_variable : entry_point.resource_interface_variables) {
            if (resource_variable.IsHeap()) {
                has_heap = true;
                continue;  // handled later
            }
            has_non_heap = true;
            if (!mapping_info) {
                continue;  // Core validation will report this
            }
            const uint32_t var_set = resource_variable.decorations.set;

            for (uint32_t i = 0; i < mapping_info->mappingCount; i++) {
                const VkDescriptorSetAndBindingMappingEXT& mapping = mapping_info->pMappings[i];
                if (!IsResourceVariableInMapping(mapping, resource_variable)) {
                    continue;
                }
                mapping_info_map[var_set].emplace_back(MappingInfo{&mapping, i, &resource_variable});
            }
        }

        if (!has_heap && !has_non_heap) {
            ss << "  - No descriptors were detected in the shader\n";
            continue;
        } else if (mapping_info_map.empty() && has_non_heap) {
            // Still want to print the untyped heap accesses below
            ss << "    - No VkDescriptorSetAndBindingMappingEXT found for this shader\n";
        }

        for (auto& [set_index, mapping_info_list] : mapping_info_map) {
            ss << "  - SPIR-V Set " << set_index << ":\n";
            std::sort(mapping_info_list.begin(), mapping_info_list.end());
            for (const MappingInfo& set_info : mapping_info_list) {
                found_warning |= DumpDescriptorHeapMapping(ss, set_info);
            }
        }

        if (has_heap) {
            found_warning |= DumpDescriptorHeapUntyped(ss, *stage);
        }
    }
    return found_warning;
}

}  // namespace gpudump
