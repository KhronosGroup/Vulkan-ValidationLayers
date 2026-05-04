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

#include "containers/limits.h"
#include "error_message/log_message_type.h"
#include "gpu_dump_state.h"
#include "gpu_dump.h"
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "containers/range.h"
#include "containers/small_vector.h"
#include "generated/dispatch_functions.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/descriptor_mode.h"
#include "state_tracker/pipeline_layout_state.h"
#include "state_tracker/pipeline_state.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/shader_object_state.h"
#include "state_tracker/shader_stage_state.h"
#include "state_tracker/state_tracker.h"
#include "utils/vk_api_utils.h"

namespace gpudump {

// Return true if found warning
bool CommandBufferSubState::DumpDescriptorBuffer(std::ostringstream& ss, const LastBound& last_bound) const {
    const vvl::CommandBuffer& cb_state = last_bound.cb_state;
    ss << "vkCmdBindDescriptorBuffersEXT last bound the following descriptor buffers:\n";
    for (uint32_t binding_i = 0; binding_i < cb_state.descriptor_buffer.binding_info.size(); binding_i++) {
        const VkDeviceAddress address = cb_state.descriptor_buffer.binding_info[binding_i].address;
        ss << "  - pBindingInfos[" << std::dec << binding_i << "].address 0x" << std::hex << address << '\n';
        auto buffer_states = dev_data.GetBuffersByAddress(address);
        for (auto& buffer_state : buffer_states) {
            ss << "    - " << buffer_state->Describe(dev_data) << "\n";
        }
        if (buffer_states.empty()) {
            ss << "    - No VkBuffer found at 0x" << std::hex << address << "\n";
        }
    }

    const vvl::PipelineLayout& pipeline_layout = *last_bound.desc_set_pipeline_layout;
    ss << "vkCmdSetDescriptorBufferOffsetsEXT has bound the following with " << dev_data.FormatHandle(pipeline_layout.VkHandle())
       << ":\n";

    struct BindingInfo {
        uint32_t index;
        VkDescriptorType type;
        uint32_t count;
        uint32_t size;
        VkDeviceSize offset;
        vvl::range<VkDeviceAddress> range;
        std::string variable_name;

        // We want to sort the bindings we print by their address
        bool operator<(const BindingInfo& other) const { return range.begin < other.range.begin; }

        void Print(std::ostringstream& binding_ss, bool robust_buffer) {
            binding_ss << "    - Binding " << std::dec << index << " (" << string_VkDescriptorType(type) << ")";
            binding_ss << ", offset: " << offset << ", range: " << string_range_hex(range) << " (";
            if (count > 1) {
                binding_ss << "descriptorCount [" << std::dec << count << "] * ";
            }
            binding_ss << DescribeDescriptorBufferSize(robust_buffer, type) << " [" << std::dec << size << "])\n";
        }
    };

    struct SetInfo {
        uint32_t index;          // set of the descriptor
        uint32_t binding_index;  // into pBindingInfos[]
        vvl::range<VkDeviceAddress> range;
        const vvl::DescriptorSetLayout* dsl;
        std::vector<BindingInfo> bindings{};

        // We want to sort the sets we print by their address
        bool operator<(const SetInfo& other) const { return range.begin < other.range.begin; }

        void Print(std::ostringstream& set_ss) {
            set_ss << "  - Set " << std::dec << index << ", size: " << range.size() << " bytes, range: " << string_range_hex(range);
            if (binding_index != vvl::kNoIndex32) {
                // Only print if there are multiple bindings
                set_ss << " (pBindingInfos[" << std::dec << binding_index << "])";
            }
            set_ss << '\n';
        }
    };

    // Quick way to combine shaderObjects and pipelines
    small_vector<const ShaderStageState*, 2> stages;
    if (last_bound.pipeline_state) {
        for (const ShaderStageState& stage : last_bound.pipeline_state->stage_states) {
            stages.emplace_back(&stage);
        }
    } else {
        for (const auto& shader_object : last_bound.shader_object_states) {
            if (shader_object) {
                stages.emplace_back(&shader_object->stage);
            }
        }
    }

    for (const ShaderStageState* stage : stages) {
        if (!stage->HasSpirv()) {
            ss << "[No SPIR-V found for " << string_VkShaderStageFlagBits(stage->GetStage())
               << ", can't detect which descriptors are being accessed]\n";
            continue;
        }
        const spirv::EntryPoint& entry_point = *stage->entrypoint;
        ss << entry_point.Describe() << "\n";

        std::vector<SetInfo> sorted_sets;
        sorted_sets.reserve(pipeline_layout.set_layouts.list.size());

        for (const spirv::ResourceInterfaceVariable& resource_variable : entry_point.resource_interface_variables) {
            const uint32_t var_set = resource_variable.decorations.set;
            const uint32_t var_binding = resource_variable.decorations.binding;

            SetInfo* set_info = nullptr;
            for (SetInfo& info : sorted_sets) {
                if (info.index == var_set) {
                    set_info = &info;
                    break;
                }
            }

            if (set_info == nullptr) {
                const vvl::DescriptorSetLayout* dsl = pipeline_layout.set_layouts.list[var_set].get();
                const auto& descriptor_buffer_binding = last_bound.ds_slots[var_set].descriptor_buffer_binding;

                // Will print the invalid/unknown sets first, no need to sort these
                if (!dsl || dsl->Destroyed()) {
                    ss << "  - Set " << std::dec << var_set
                       << " VkDescriptorSetLayout was destroyed (TODO - Track more info in pipeline layout)";
                    continue;
                } else if ((dsl->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) == 0) {
                    ss << "  - Set " << std::dec << var_set
                       << " was not created with VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT";
                    continue;
                } else if (!descriptor_buffer_binding.has_value()) {
                    ss << "  - Set " << std::dec << var_set
                       << " was never bound with offset. (WARNING - only valid if descriptor is not used in the shader";
                    if (dsl->HasImmutableSamplers()) {
                        ss << " or because all bindings are using Immutable Samplers";
                    }
                    ss << ")\n";
                    continue;
                }

                const VkDeviceAddress start_address =
                    cb_state.descriptor_buffer.binding_info[descriptor_buffer_binding->index].address +
                    descriptor_buffer_binding->offset;
                const VkDeviceSize dsl_size = dsl->GetLayoutSizeInBytes();
                vvl::range<VkDeviceAddress> set_range{start_address, start_address + dsl_size};

                // only care to print index in pBindingInfos
                const uint32_t binding_index =
                    cb_state.descriptor_buffer.binding_info.size() > 1 ? descriptor_buffer_binding->index : vvl::kNoIndex32;

                set_info = &sorted_sets.emplace_back(SetInfo{var_set, binding_index, set_range, dsl, {}});
            }

            // To variables might be the same set/binding if doing descriptor indexing aliasing
            bool alias_binding = false;
            for (BindingInfo& info : set_info->bindings) {
                if (info.index == var_binding && !info.variable_name.empty() && !resource_variable.debug_name.empty()) {
                    info.variable_name = info.variable_name + ", \"" + resource_variable.debug_name + "\"";
                    alias_binding = true;
                    break;
                }
            }
            if (alias_binding) {
                continue;
            }

            const auto& ds_layout_def = *set_info->dsl->GetLayoutDef();
            const VkDescriptorType type = ds_layout_def.GetTypeFromBinding(var_binding);
            const uint32_t count = ds_layout_def.GetDescriptorCountFromBinding(var_binding);

            const uint32_t descriptor_size = (uint32_t)GetDescriptorBufferSize(dev_data.phys_dev_ext_props.descriptor_buffer_props,
                                                                               dev_data.enabled_features.robustBufferAccess, type);
            VkDeviceSize binding_offset = 0;
            DispatchGetDescriptorSetLayoutBindingOffsetEXT(dev_data.device, set_info->dsl->VkHandle(), var_binding,
                                                           &binding_offset);
            const VkDeviceAddress binding_start_address = set_info->range.begin + binding_offset;
            vvl::range<VkDeviceAddress> binding_range{binding_start_address, binding_start_address + (descriptor_size * count)};

            set_info->bindings.emplace_back(BindingInfo{var_binding, type, count, descriptor_size, binding_offset, binding_range,
                                                        resource_variable.debug_name});
        }

        if (sorted_sets.empty()) {
            ss << "  - No descriptors were detected in the shader\n";
            continue;
        }

        // Sort by address and print out everything
        std::sort(sorted_sets.begin(), sorted_sets.end());
        for (SetInfo& set_info : sorted_sets) {
            set_info.Print(ss);

            std::sort(set_info.bindings.begin(), set_info.bindings.end());
            for (BindingInfo& binding_info : set_info.bindings) {
                binding_info.Print(ss, dev_data.enabled_features.robustBufferAccess);
            }
        }
    }
    return false;
}

struct MappingInfo {
    const VkDescriptorSetAndBindingMappingEXT* mapping;
    const spirv::ResourceInterfaceVariable* resource_variable;
    uint32_t index;  // into pMappings

    // Sort by binding
    bool operator<(const MappingInfo& other) const {
        return resource_variable->decorations.binding < other.resource_variable->decorations.binding;
    }
};

static const vvl::Buffer& GetLargestBuffer(const GpuDump& dev_data, uint64_t address) {
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
    assert(longer_buffer);
    return *longer_buffer;
}

bool CommandBufferSubState::DumpDescriptorHeapMapping(std::ostringstream& ss, const MappingInfo& mapping_info) const {
    const VkDescriptorSetAndBindingMappingEXT& mapping = *mapping_info.mapping;
    const spirv::ResourceInterfaceVariable& resource_variable = *mapping_info.resource_variable;

    // bool Print(const CommandBufferSubState& cb_sub_state, std::ostringstream& map_ss, const VkPhysicalDevice) {

    ss << "    - Binding " << std::dec << resource_variable.decorations.binding << ", "
       << string_VkDescriptorMappingSourceEXT(mapping.source) << " (from pMappings[" << mapping_info.index << "])\n";
    const bool is_combined_image_sampler = resource_variable.is_combined_image_sampler;
    const char* main_heap_type = resource_variable.is_sampler ? "Sampler" : "Resource";
    const bool is_array = resource_variable.IsArray();
    const bool is_runtime_array = resource_variable.IsRuntimeArray();
    uint32_t array_length = 0;
    if (is_array && !is_runtime_array && resource_variable.array_length != spirv::kSpecConstant) {
        array_length = resource_variable.array_length;
    }

    const VkDescriptorType descriptor_type = resource_variable.GetPotentialDescriptorType();
    // TODO - Cache these once on device creation
    VkDeviceSize descriptor_size = descriptor_type == VK_DESCRIPTOR_TYPE_MAX_ENUM
                                       ? 0
                                       : DispatchGetPhysicalDeviceDescriptorSizeEXT(dev_data.physical_device, descriptor_type);

    // attempts to catch obvious OOB offsets in mappings
    bool warn_oob = false;
    uint32_t warn_index_oob = 0;
    std::vector<uint32_t> warn_index_array;

    ss << "      - ";
    const char* new_line = "\n        ";

    vvl::CommandBuffer::DescriptorHeap& heap = base.descriptor_heap;
    if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT) {
        const VkDescriptorMappingSourceConstantOffsetEXT& map_data = mapping.sourceData.constantOffset;

        ss << "heapOffset: 0x" << std::hex << map_data.heapOffset << ", heapArrayStride: 0x" << map_data.heapArrayStride;
        VkDeviceSize index_zero_offset = map_data.heapOffset;
        VkDeviceSize index_zero_address = heap.resource_range.begin + index_zero_offset;
        ss << new_line << main_heap_type << " Heap address: 0x" << index_zero_address;
        if (is_array) {
            ss << " + (descriptor_index * 0x" << map_data.heapArrayStride << ")";
            const VkDeviceSize available_space = (heap.resource_range.size() - index_zero_offset) - descriptor_size;
            const uint32_t max_index = (uint32_t)(available_space / map_data.heapArrayStride);
            if (array_length != 0) {
                const VkDeviceSize final_array_offset = (array_length - 1) * map_data.heapArrayStride;
                ss << new_line << "    The final descriptor index at [" << std::dec << array_length << std::hex
                   << "] will access [0x" << (index_zero_address + final_array_offset) << ", 0x"
                   << (index_zero_address + final_array_offset + descriptor_size) << ")";
                warn_index_oob = array_length > max_index ? max_index : 0;
            } else if (is_runtime_array) {
                const VkDeviceSize final_array_offset = (max_index * map_data.heapArrayStride);
                ss << new_line << "    The final descriptor index in bounds is [" << std::dec << max_index << std::hex
                   << "] which would be accessed at [0x" << (index_zero_address + final_array_offset) << ", 0x"
                   << (index_zero_address + final_array_offset + descriptor_size) << ")";
            }
        }
        warn_oob |= (index_zero_offset + descriptor_size > heap.resource_range.size());

        if (is_combined_image_sampler) {
            ss << new_line << "samplerHeapOffset: 0x" << std::hex << map_data.samplerHeapOffset << ", samplerHeapArrayStride: 0x"
               << map_data.samplerHeapArrayStride;
            index_zero_offset = map_data.samplerHeapOffset;
            index_zero_address = heap.sampler_range.begin + index_zero_offset;
            ss << new_line << "Sampler Heap address: 0x" << index_zero_address;
            if (is_array) {
                ss << " + (descriptor_index * 0x" << map_data.samplerHeapArrayStride << ")";
                const VkDeviceSize available_space = (heap.sampler_range.size() - index_zero_offset) - descriptor_size;
                const uint32_t max_index = (uint32_t)(available_space / map_data.samplerHeapArrayStride);
                if (array_length != 0) {
                    const VkDeviceSize final_array_offset = (array_length - 1) * map_data.samplerHeapArrayStride;
                    ss << new_line << "    The final descriptor index at [" << std::dec << array_length << std::hex
                       << "] will access [0x" << (index_zero_address + final_array_offset) << ", 0x"
                       << (index_zero_address + final_array_offset + descriptor_size) << ")";
                    warn_index_oob = array_length > max_index ? max_index : 0;
                } else if (is_runtime_array) {
                    const VkDeviceSize final_array_offset =
                        (map_data.samplerHeapOffset + (max_index * map_data.samplerHeapArrayStride));
                    ss << new_line << "    The final descriptor index in bounds is [" << std::dec << max_index << std::hex
                       << "] which would be accessed at [0x" << (index_zero_address + final_array_offset) << ", 0x"
                       << (index_zero_address + final_array_offset + descriptor_size) << ")";
                }
            }
            warn_oob |= (index_zero_offset + descriptor_size > heap.sampler_range.size());
        }
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT) {
        const VkDescriptorMappingSourcePushIndexEXT& map_data = mapping.sourceData.pushIndex;
        uint32_t push_index = *((uint32_t*)&push_data_value[map_data.pushOffset]);

        ss << "pushOffset: 0x" << std::hex << map_data.pushOffset << ", heapOffset: 0x" << map_data.heapOffset
           << ", heapIndexStride: 0x" << map_data.heapIndexStride << ", heapArrayStride: 0x" << map_data.heapArrayStride;
        ss << new_line << "pushIndex: 0x" << push_index;
        ss << new_line << main_heap_type << " Heap address: 0x" << heap.resource_range.begin + map_data.heapOffset << " + (0x"
           << push_index << " * 0x" << map_data.heapIndexStride << ")";
        VkDeviceSize index_zero_offset = map_data.heapOffset + (push_index * map_data.heapIndexStride);
        VkDeviceSize index_zero_address = heap.resource_range.begin + index_zero_offset;
        if (is_array) {
            ss << " + (descriptor_index * 0x" << map_data.heapArrayStride << ")";
            const VkDeviceSize available_space = (heap.resource_range.size() - index_zero_offset) - descriptor_size;
            const uint32_t max_index = (uint32_t)(available_space / map_data.heapArrayStride);
            if (array_length != 0) {
                const VkDeviceSize final_array_offset = (array_length - 1) * map_data.heapArrayStride;
                ss << new_line << "    The final descriptor index at [" << std::dec << array_length << std::hex
                   << "] will access [0x" << (index_zero_address + final_array_offset) << ", 0x"
                   << (index_zero_address + final_array_offset + descriptor_size) << ")";
                warn_index_oob = array_length > max_index ? max_index : 0;
            } else if (is_runtime_array) {
                const VkDeviceSize final_array_offset = max_index * map_data.heapArrayStride;
                ss << new_line << "    The final descriptor index in bounds is [" << std::dec << max_index << std::hex
                   << "] which would be accessed at [0x" << (index_zero_address + final_array_offset) << ", 0x"
                   << (index_zero_address + final_array_offset + descriptor_size) << ")";
            }
        } else {
            ss << " [final address 0x" << index_zero_address << "]";
        }
        warn_oob |= (index_zero_offset + descriptor_size > heap.resource_range.size());

        if (is_combined_image_sampler) {
            ss << new_line << "pushOffset: 0x" << std::hex << map_data.pushOffset << ", samplerHeapOffset: 0x"
               << map_data.samplerHeapOffset << ", samplerHeapIndexStride: 0x" << map_data.samplerHeapIndexStride
               << ", samplerHeapArrayStride: 0x" << map_data.samplerHeapArrayStride;
            ss << new_line << "pushIndex: 0x" << push_index;
            ss << new_line << "Sampler Heap address: 0x" << heap.sampler_range.begin + map_data.samplerHeapOffset << " + (0x"
               << push_index << " * 0x" << map_data.samplerHeapIndexStride << ")";
            index_zero_offset = map_data.samplerHeapOffset + (push_index * map_data.samplerHeapIndexStride);
            index_zero_address = heap.sampler_range.begin + index_zero_offset;
            if (is_array) {
                ss << " + (descriptor_index * 0x" << map_data.samplerHeapArrayStride << ")";
                const VkDeviceSize available_space = (heap.sampler_range.size() - index_zero_offset) - descriptor_size;
                const uint32_t max_index = (uint32_t)(available_space / map_data.samplerHeapArrayStride);
                if (array_length != 0) {
                    const VkDeviceSize final_array_offset = (array_length - 1) * map_data.samplerHeapArrayStride;
                    ss << new_line << "    The final descriptor index at [" << std::dec << array_length << std::hex
                       << "] will access [0x" << (index_zero_address + final_array_offset) << ", 0x"
                       << (index_zero_address + final_array_offset + descriptor_size) << ")";
                    warn_index_oob = array_length > max_index ? max_index : 0;
                } else if (is_runtime_array) {
                    const VkDeviceSize final_array_offset = max_index * map_data.samplerHeapArrayStride;
                    ss << new_line << "    The final descriptor index in bounds is [" << std::dec << max_index << std::hex
                       << "] which would be accessed at [0x" << (index_zero_address + final_array_offset) << ", 0x"
                       << (index_zero_address + final_array_offset + descriptor_size) << ")";
                }
            } else {
                ss << " [final address 0x" << (heap.sampler_range.begin + index_zero_offset) << "]";
            }
            warn_oob |= (index_zero_offset + descriptor_size > heap.sampler_range.size());
        }
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
        const VkDescriptorMappingSourceIndirectIndexEXT& map_data = mapping.sourceData.indirectIndex;
        VkDeviceAddress push_indirect_address = *((VkDeviceAddress*)&push_data_value[map_data.pushOffset]);
        VkDeviceAddress final_indirect_address = push_indirect_address + map_data.addressOffset;

        std::vector<uint8_t> indirect_index_data = dev_data.CopyDataFromMemory(final_indirect_address, 4);
        bool know_ubo = !indirect_index_data.empty();
        uint32_t indirect_index = know_ubo ? *((uint32_t*)indirect_index_data.data()) : 0;

        ss << "pushOffset: 0x" << std::hex << map_data.pushOffset << ", addressOffset: 0x" << map_data.addressOffset
           << ", heapOffset: 0x" << map_data.heapOffset << ", heapIndexStride: 0x" << map_data.heapIndexStride
           << ", heapArrayStride: 0x" << map_data.heapArrayStride;
        ss << new_line << "indirectAddress: 0x" << final_indirect_address << " (0x" << push_indirect_address << " + 0x"
           << map_data.addressOffset << ")";
        if (know_ubo) {
            ss << new_line << "indirectIndex: 0x" << indirect_index;
        }
        ss << new_line << main_heap_type << " Heap address: 0x" << heap.resource_range.begin + map_data.heapOffset << " + (";
        if (know_ubo) {
            ss << "0x" << indirect_index;
        } else {
            ss << "indirectIndex";
        }
        ss << " * 0x" << map_data.heapIndexStride << ")";
        if (is_array) {
            ss << " + (descriptor_index * 0x" << map_data.heapArrayStride << ")";

            if (know_ubo) {
                VkDeviceSize index_zero_offset = map_data.heapOffset + (indirect_index * map_data.heapIndexStride);
                VkDeviceSize index_zero_address = heap.resource_range.begin + index_zero_offset;
                VkDeviceSize available_space = (heap.resource_range.size() - index_zero_offset) - descriptor_size;
                uint32_t max_index = (uint32_t)(available_space / map_data.heapArrayStride);
                if (array_length != 0) {
                    const VkDeviceSize final_array_offset = (array_length - 1) * map_data.heapArrayStride;
                    ss << new_line << "    The final descriptor index at [" << std::dec << array_length << std::hex
                       << "] will access [0x" << (index_zero_address + final_array_offset) << ", 0x"
                       << (index_zero_address + final_array_offset + descriptor_size) << ")";
                    warn_index_oob = array_length > max_index ? max_index : 0;
                } else if (is_runtime_array) {
                    const VkDeviceSize final_array_offset = max_index * map_data.heapArrayStride;
                    ss << new_line << "    The final descriptor index in bounds is [" << std::dec << max_index << std::hex
                       << "] which would be accessed at [0x" << (index_zero_address + final_array_offset) << ", 0x"
                       << (index_zero_address + final_array_offset + descriptor_size) << ")";
                }
            }
        } else if (know_ubo) {
            uint64_t final_offset = map_data.heapOffset + (indirect_index * map_data.heapIndexStride);
            ss << " [final address 0x" << (heap.resource_range.begin + final_offset) << "]";
            warn_oob |= (final_offset + descriptor_size > heap.resource_range.size());
        }
        warn_oob |= (map_data.heapOffset + descriptor_size > heap.resource_range.size());

        if (is_combined_image_sampler) {
            push_indirect_address = *((VkDeviceAddress*)&push_data_value[map_data.samplerPushOffset]);
            final_indirect_address = push_indirect_address + map_data.samplerAddressOffset;

            indirect_index_data = dev_data.CopyDataFromMemory(final_indirect_address, 4);
            know_ubo = !indirect_index_data.empty();
            indirect_index = know_ubo ? *((uint32_t*)indirect_index_data.data()) : 0;

            ss << new_line << "samplerPushOffset: 0x" << std::hex << map_data.samplerPushOffset << ", samplerAddressOffset: 0x"
               << map_data.samplerAddressOffset << ", samplerHeapOffset: 0x" << map_data.samplerHeapOffset
               << ", samplerHeapIndexStride: 0x" << map_data.samplerHeapIndexStride << ", samplerHeapArrayStride: 0x"
               << map_data.samplerHeapArrayStride;
            ss << new_line << "indirectAddress: 0x" << final_indirect_address << " (0x" << push_indirect_address << " + 0x"
               << map_data.samplerAddressOffset << ")";
            if (know_ubo) {
                ss << new_line << "indirectIndex: 0x" << indirect_index;
            }
            ss << new_line << "Sampler Heap address: 0x" << heap.sampler_range.begin + map_data.samplerHeapOffset << " + (";
            if (know_ubo) {
                ss << "0x" << indirect_index;
            } else {
                ss << "indirectIndex";
            }
            ss << " * 0x" << map_data.samplerHeapIndexStride << ")";
            if (is_array) {
                ss << " + (descriptor_index * 0x" << map_data.samplerHeapArrayStride << ")";

                if (know_ubo) {
                    VkDeviceSize index_zero_offset =
                        map_data.samplerHeapOffset + (indirect_index * map_data.samplerHeapIndexStride);
                    VkDeviceSize index_zero_address = heap.sampler_range.begin + index_zero_offset;
                    VkDeviceSize available_space = (heap.sampler_range.size() - index_zero_offset) - descriptor_size;
                    uint32_t max_index = (uint32_t)(available_space / map_data.samplerHeapArrayStride);
                    if (array_length != 0) {
                        const VkDeviceSize final_array_offset = (array_length - 1) * map_data.samplerHeapArrayStride;
                        ss << new_line << "    The final descriptor index at [" << std::dec << array_length << std::hex
                           << "] will access [0x" << (index_zero_address + final_array_offset) << ", 0x"
                           << (index_zero_address + final_array_offset + descriptor_size) << ")";
                        warn_index_oob = array_length > max_index ? max_index : 0;
                    } else if (is_runtime_array) {
                        const VkDeviceSize final_array_offset = max_index * map_data.samplerHeapArrayStride;
                        ss << new_line << "    The final descriptor index in bounds is [" << std::dec << max_index << std::hex
                           << "] which would be accessed at [0x" << (index_zero_address + final_array_offset) << ", 0x"
                           << (index_zero_address + final_array_offset + descriptor_size) << ")";
                    }
                }
            } else if (know_ubo) {
                uint64_t final_offset = map_data.samplerHeapOffset + (indirect_index * map_data.samplerHeapIndexStride);
                ss << " [final address 0x" << (heap.sampler_range.begin + final_offset) << "]";
                warn_oob |= (final_offset + descriptor_size > heap.sampler_range.size());
            }
            warn_oob |= (map_data.samplerHeapOffset + descriptor_size > heap.sampler_range.size());
        }
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
        const VkDescriptorMappingSourceIndirectIndexArrayEXT& map_data = mapping.sourceData.indirectIndexArray;
        VkDeviceAddress push_indirect_address = *((VkDeviceAddress*)&push_data_value[map_data.pushOffset]);
        VkDeviceAddress final_indirect_address = push_indirect_address + map_data.addressOffset;

        ss << "pushOffset: 0x" << std::hex << map_data.pushOffset << ", addressOffset: 0x" << map_data.addressOffset
           << ", heapOffset: 0x" << map_data.heapOffset << ", heapIndexStride: 0x" << map_data.heapIndexStride;
        ss << new_line << "indirectAddress: 0x" << final_indirect_address << " (0x" << push_indirect_address << " + 0x"
           << map_data.addressOffset << ")";

        ss << new_line << main_heap_type << " Heap address: 0x" << heap.resource_range.begin + map_data.heapOffset
           << " + (indirectIndex * 0x" << map_data.heapIndexStride << ")";

        const vvl::Buffer& buffer_state = GetLargestBuffer(dev_data, final_indirect_address);
        uint32_t available_bytes = (uint32_t)(buffer_state.DeviceAddressRange().end - final_indirect_address);
        uint32_t available_slots = available_bytes / sizeof(uint32_t);
        // We can assume this is an array, otherwise, not sure why people are using this mapping
        uint32_t search_slots = !is_array ? 1 : is_runtime_array ? available_slots : array_length;

        if (array_length != 0 && array_length > available_slots) {
            warn_index_oob = available_slots;
            search_slots = available_slots;
        }
        uint32_t search_bytes = search_slots * sizeof(uint32_t);

        std::vector<uint8_t> indirect_index_data = dev_data.CopyDataFromMemory(final_indirect_address, search_bytes);
        bool know_ubo = !indirect_index_data.empty();

        if (is_runtime_array) {
            // Currently don't go searching in, no way to know desired upper bound
            ss << new_line << "    Any descriptor index starting at [" << std::dec << search_slots << std::hex
               << "] will be invalid as there are no more values found for indirectIndex inside "
               << dev_data.FormatHandle(buffer_state.Handle());
        }

        if (know_ubo) {
            uint32_t* indirect_index_words = (uint32_t*)indirect_index_data.data();
            if (!is_array) {
                uint32_t indirect_index = indirect_index_words[0];
                uint64_t final_offset = map_data.heapOffset + (indirect_index * map_data.heapIndexStride);
                ss << " [final address 0x" << (heap.resource_range.begin + final_offset) << "]";
                warn_oob |= (final_offset + descriptor_size > heap.resource_range.size());
            } else if (!is_runtime_array) {
                ss << new_line << "indirectIndex values from buffer: [";
                for (uint32_t i = 0; i < search_slots; i++) {
                    const uint32_t current_index_value = indirect_index_words[i];
                    uint64_t final_offset = map_data.heapOffset + (current_index_value * map_data.heapIndexStride);
                    if (final_offset + descriptor_size > heap.resource_range.size()) {
                        warn_index_array.emplace_back(i);
                    }
                    if (i != 0) ss << ", ";
                    ss << "0x" << current_index_value;
                }
                ss << "]";
            }
        }
        warn_oob |= (map_data.heapOffset + descriptor_size > heap.resource_range.size());

        if (is_combined_image_sampler) {
            push_indirect_address = *((VkDeviceAddress*)&push_data_value[map_data.samplerPushOffset]);
            final_indirect_address = push_indirect_address + map_data.samplerAddressOffset;

            ss << new_line << "samplerPushOffset: 0x" << std::hex << map_data.samplerPushOffset << ", samplerAddressOffset: 0x"
               << map_data.samplerAddressOffset << ", samplerHeapOffset: 0x" << map_data.samplerHeapOffset
               << ", samplerHeapIndexStride: 0x" << map_data.samplerHeapIndexStride;
            ss << new_line << "indirectAddress: 0x" << final_indirect_address << " (0x" << push_indirect_address << " + 0x"
               << map_data.samplerAddressOffset << ")";
            ss << new_line << "Sampler Heap address: 0x" << heap.sampler_range.begin + map_data.samplerHeapOffset
               << " + (indirectIndex * 0x" << map_data.samplerHeapIndexStride << ")";

            const vvl::Buffer& sampler_buffer_state = GetLargestBuffer(dev_data, final_indirect_address);
            available_bytes = (uint32_t)(sampler_buffer_state.DeviceAddressRange().end - final_indirect_address);
            available_slots = available_bytes / sizeof(uint32_t);
            search_slots = !is_array ? 1 : is_runtime_array ? available_slots : array_length;

            if (array_length != 0 && array_length > available_slots) {
                warn_index_oob = available_slots;
                search_slots = available_slots;
            }
            search_bytes = search_slots * sizeof(uint32_t);

            indirect_index_data = dev_data.CopyDataFromMemory(final_indirect_address, search_bytes);
            know_ubo = !indirect_index_data.empty();

            if (is_runtime_array) {
                ss << new_line << "    Any descriptor index starting at [" << std::dec << search_slots << std::hex
                   << "] will be invalid as there are no more values found for indirectIndex inside "
                   << dev_data.FormatHandle(sampler_buffer_state.Handle());
            }

            if (know_ubo) {
                uint32_t* indirect_index_words = (uint32_t*)indirect_index_data.data();
                if (!is_array) {
                    uint32_t indirect_index = indirect_index_words[0];
                    uint64_t final_offset = map_data.samplerHeapOffset + (indirect_index * map_data.samplerHeapIndexStride);
                    ss << " [final address 0x" << (heap.sampler_range.begin + final_offset) << "]";
                    warn_oob |= (final_offset + descriptor_size > heap.sampler_range.size());
                } else if (!is_runtime_array) {
                    ss << new_line << "indirectIndex values from buffer: [";
                    for (uint32_t i = 0; i < search_slots; i++) {
                        const uint32_t current_index_value = indirect_index_words[i];
                        uint64_t final_offset =
                            map_data.samplerHeapOffset + (current_index_value * map_data.samplerHeapIndexStride);
                        if (final_offset + descriptor_size > heap.sampler_range.size()) {
                            warn_index_array.emplace_back(i);
                        }
                        if (i != 0) ss << ", ";
                        ss << "0x" << current_index_value;
                    }
                    ss << "]";
                }
            }
            warn_oob |= (map_data.samplerHeapOffset + descriptor_size > heap.sampler_range.size());
        }
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT) {
        const VkDescriptorMappingSourceHeapDataEXT& map_data = mapping.sourceData.heapData;
        uint32_t push_data = *((uint32_t*)&push_data_value[map_data.pushOffset]);

        ss << "pushOffset: 0x" << std::hex << map_data.pushOffset << ", heapOffset: 0x" << map_data.heapOffset;
        ss << new_line << "Push data at 0x" << std::hex << map_data.pushOffset << ": 0x" << push_data;
        ss << new_line << main_heap_type << " Heap address: 0x" << heap.resource_range.begin + map_data.heapOffset << " + 0x"
           << push_data;
        uint64_t final_offset = map_data.heapOffset + push_data;
        ss << " [final address 0x" << (heap.resource_range.begin + final_offset) << "]";
        warn_oob |= (final_offset + descriptor_size > heap.resource_range.size());
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT) {
        ss << "pushDataOffset: 0x" << std::hex << mapping.sourceData.pushDataOffset;
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT) {
        uint64_t indirect_address = *((uint64_t*)&push_data_value[mapping.sourceData.pushAddressOffset]);
        ss << "pushAddressOffset: 0x" << std::hex << mapping.sourceData.pushAddressOffset;
        ss << new_line << "Indirect Adresss 0x" << indirect_address;
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT) {
        const VkDescriptorMappingSourceIndirectAddressEXT& map_data = mapping.sourceData.indirectAddress;
        VkDeviceAddress push_indirect_address = *((VkDeviceAddress*)&push_data_value[map_data.pushOffset]);
        VkDeviceAddress final_indirect_address = push_indirect_address + map_data.addressOffset;

        ss << "pushOffset: 0x" << std::hex << map_data.pushOffset << ", addressOffset: 0x" << map_data.addressOffset;
        ss << new_line << "Indirect Address: 0x" << final_indirect_address << " (0x" << push_indirect_address << " + 0x"
           << map_data.addressOffset << ")";

        std::vector<uint8_t> indirect_address_data = dev_data.CopyDataFromMemory(final_indirect_address, 8);
        if (!indirect_address_data.empty()) {
            ss << new_line << "Resource Adresss 0x" << *((VkDeviceAddress*)indirect_address_data.data());
        }
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT) {
        // TODO - Add address for RTX
        const VkDescriptorMappingSourceShaderRecordIndexEXT& map_data = mapping.sourceData.shaderRecordIndex;
        ss << "heapOffset: 0x" << std::hex << map_data.heapOffset << ", shaderRecordOffset: 0x" << map_data.shaderRecordOffset
           << ", heapIndexStride: 0x" << map_data.heapIndexStride << ", heapArrayStride: 0x" << map_data.heapArrayStride;
        if (is_combined_image_sampler) {
            ss << new_line << "samplerHeapOffset: 0x" << std::hex << map_data.samplerHeapOffset << ", samplerShaderRecordOffset: 0x"
               << map_data.samplerShaderRecordOffset << ", samplerHeapIndexStride: 0x" << map_data.samplerHeapIndexStride
               << ", samplerHeapArrayStride: 0x" << map_data.samplerHeapArrayStride;
        }
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_DATA_EXT) {
        // TODO - Add more info probably
        ss << "shaderRecordDataOffset: " << std::hex << mapping.sourceData.shaderRecordDataOffset;
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_ADDRESS_EXT) {
        // TODO - Add more info probably
        ss << "shaderRecordAddressOffset: " << std::hex << mapping.sourceData.shaderRecordAddressOffset;
    }

    if (descriptor_type != VK_DESCRIPTOR_TYPE_MAX_ENUM) {
        ss << new_line << "Descriptor size: 0x" << descriptor_size << " (" << string_VkDescriptorType(descriptor_type) << ")";
    }

    if (warn_oob) {
        ss << new_line << "[WARNING] OUT OF BOUNDS - any access to this descriptor will be invalid";
    } else if (!warn_index_array.empty()) {
        ss << new_line << "[WARNING] OUT OF BOUNDS - descriptors indexes at [" << std::dec;
        for (uint32_t i = 0; i < warn_index_array.size(); i++) {
            if (i != 0) ss << ", ";
            ss << warn_index_array[i];
        }
        ss << std::hex << "] will be invalid if accessed";
    } else if (warn_index_oob != 0) {
        ss << new_line << "[WARNING] OUT OF BOUNDS - descriptor has an array length of [" << std::dec << array_length
           << "] but any element accessed starting at [" << warn_index_oob + 1 << std::hex << "] will be invalid if accessed";
    }

    ss << '\n';

    return warn_oob || (warn_index_oob != 0) || !warn_index_array.empty();
}

// Return true if warning found
bool CommandBufferSubState::DumpDescriptorHeap(std::ostringstream& ss, const LastBound& last_bound) const {
    const vvl::CommandBuffer& cb_state = last_bound.cb_state;
    if (!cb_state.descriptor_heap.resource_range.empty()) {
        ss << "vkCmdBindResourceHeapEXT last bound the resource heap to "
           << string_range_hex(cb_state.descriptor_heap.resource_range);
        if (!cb_state.descriptor_heap.resource_reserved.empty()) {
            ss << " (reserved range " << string_range_hex(cb_state.descriptor_heap.resource_reserved) << ")";
        } else {
            ss << " (no reserved range)";
        }
        ss << '\n';
        auto buffer_states = dev_data.GetBuffersByAddress(cb_state.descriptor_heap.resource_range.begin);
        for (auto& buffer_state : buffer_states) {
            ss << "  - " << buffer_state->Describe(dev_data) << "\n";
        }
        if (buffer_states.empty()) {
            ss << "  - No VkBuffer found at 0x" << std::hex << cb_state.descriptor_heap.resource_range.begin << "\n";
        }
    }
    if (!cb_state.descriptor_heap.sampler_range.empty()) {
        ss << "vkCmdBindSamplerHeapEXT last bound the sampler heap to "
           << string_range_hex(cb_state.descriptor_heap.resource_range);
        if (!cb_state.descriptor_heap.sampler_reserved.empty()) {
            ss << " (reserved range " << string_range_hex(cb_state.descriptor_heap.sampler_reserved) << ")";
        } else {
            ss << " (no reserved range)";
        }
        ss << '\n';
        auto buffer_states = dev_data.GetBuffersByAddress(cb_state.descriptor_heap.sampler_range.begin);
        for (auto& buffer_state : buffer_states) {
            ss << "  - " << buffer_state->Describe(dev_data) << "\n";
        }
        if (buffer_states.empty()) {
            ss << "  - No VkBuffer found at 0x" << std::hex << cb_state.descriptor_heap.sampler_range.begin << "\n";
        }
    }

    // Quick way to combine shaderObjects and pipelines
    small_vector<const ShaderStageState*, 2> stages;
    if (last_bound.pipeline_state) {
        for (const ShaderStageState& stage : last_bound.pipeline_state->stage_states) {
            stages.emplace_back(&stage);
        }
    } else {
        for (const auto& shader_object : last_bound.shader_object_states) {
            if (shader_object) {
                stages.emplace_back(&shader_object->stage);
            }
        }
    }

    bool found_warning = false;
    for (const ShaderStageState* stage : stages) {
        if (!stage->HasSpirv()) {
            ss << "[No SPIR-V found for " << string_VkShaderStageFlagBits(stage->GetStage())
               << ", can't detect which descriptors are being accessed]\n";
            continue;
        }
        const spirv::EntryPoint& entry_point = *stage->entrypoint;
        ss << entry_point.Describe() << "\n";

        const auto* mapping_info = vku::FindStructInPNextChain<VkShaderDescriptorSetAndBindingMappingInfoEXT>(stage->GetPNext());

        // Want to sort and print all the mappings for a given Set together
        std::map<uint32_t, std::vector<MappingInfo>> mapping_info_map;

        for (const spirv::ResourceInterfaceVariable& resource_variable : entry_point.resource_interface_variables) {
            if (resource_variable.IsHeap()) {
                // TODO detect offsets from start of heap and other info
                ss << "  - " << resource_variable.DescribeDescriptor();
                continue;
            }

            const uint32_t var_set = resource_variable.decorations.set;

            for (uint32_t i = 0; i < mapping_info->mappingCount; i++) {
                const VkDescriptorSetAndBindingMappingEXT& mapping = mapping_info->pMappings[i];
                if (!IsResourceVaribleInMapping(mapping, resource_variable)) {
                    continue;
                }
                mapping_info_map[var_set].emplace_back(MappingInfo{&mapping, &resource_variable, i});
            }
        }

        if (mapping_info_map.empty()) {
            ss << "  No VkDescriptorSetAndBindingMappingEXT were found for this shader\n";
            continue;
        }

        for (auto& [set_index, mapping_info_list] : mapping_info_map) {
            ss << "  - Set " << set_index << ":\n";
            std::sort(mapping_info_list.begin(), mapping_info_list.end());
            for (const MappingInfo& set_info : mapping_info_list) {
                found_warning |= DumpDescriptorHeapMapping(ss, set_info);
            }
        }
    }
    return found_warning;
}

void CommandBufferSubState::DumpDescriptors(const LastBound& last_bound, const Location& loc) const {
    vvl::DescriptorMode descriptor_mode = last_bound.GetDescriptorMode();
    if (descriptor_mode != vvl::DescriptorModeBuffer && descriptor_mode != vvl::DescriptorModeHeap) {
        return;
    }
    std::ostringstream ss;
    ss << "[Dump Descriptor] (";
    // Embedded the objects into the message at the top instead of providing them in the callback we normally do
    const LogObjectList objlist = last_bound.cb_state.GetObjectList(last_bound.bind_point);
    bool first_obj = true;
    for (auto object : objlist) {
        if (first_obj) {
            first_obj = false;
        } else {
            ss << ", ";
        }
        ss << dev_data.FormatHandle(object);
    }
    ss << ")\n";

    bool found_warning = false;
    if (descriptor_mode == vvl::DescriptorModeBuffer) {
        found_warning = DumpDescriptorBuffer(ss, last_bound);
    } else if (descriptor_mode == vvl::DescriptorModeHeap) {
        found_warning = DumpDescriptorHeap(ss, last_bound);
    }

    if (dev_data.gpu_dump_settings.to_stdout) {
        std::cout << "GPU-DUMP " << ss.str() << '\n';
    } else {
        const VkFlags log_level = found_warning ? kWarningBit : kInformationBit;
        // Don't provide a LogObjectList, embed it into the message instead to keep things cleaner
        // (because the default callback will list them at the bottom)
        dev_data.debug_report->LogMessage(log_level, "GPU-DUMP", {}, loc, ss.str().c_str());
    }
}

}  // namespace gpudump
