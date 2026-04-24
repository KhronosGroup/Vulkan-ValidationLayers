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

void CommandBufferSubState::DumpDescriptorBuffer(std::ostringstream& ss, const LastBound& last_bound) const {
    const vvl::CommandBuffer& cb_state = last_bound.cb_state;
    ss << "vkCmdBindDescriptorBuffersEXT last bound the following descriptor buffers:\n";
    for (uint32_t binding_i = 0; binding_i < cb_state.descriptor_buffer.binding_info.size(); binding_i++) {
        const VkDeviceAddress address = cb_state.descriptor_buffer.binding_info[binding_i].address;
        ss << "  - pBindingInfos[" << binding_i << "].address 0x" << std::hex << address << '\n';
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
                binding_ss << "descriptorCount [" << std::dec << count << "] times ";
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
}

// TODO - Make a helper in ResourceInterfaceVariable
static VkDescriptorType GetResourceInterfaceVariableType(const spirv::ResourceInterfaceVariable& resource_variable) {
    if (resource_variable.is_storage_image) {
        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    } else if (resource_variable.is_storage_texel_buffer) {
        return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    } else if (resource_variable.is_storage_buffer) {
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    } else if (resource_variable.is_uniform_buffer) {
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    } else if (resource_variable.is_input_attachment) {
        return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    } else if (resource_variable.is_storage_tensor) {
        return VK_DESCRIPTOR_TYPE_TENSOR_ARM;
    } else if (resource_variable.is_sampler) {
        return VK_DESCRIPTOR_TYPE_SAMPLER;
    }
    // TODO - Add support in ResourceInterfaceVariable to detect
    // - VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
    // - VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
    // - VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR
    return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

void CommandBufferSubState::DumpDescriptorHeap(std::ostringstream& ss, const LastBound& last_bound) const {
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

    struct MappingInfo {
        const VkDescriptorSetAndBindingMappingEXT* mapping;
        const spirv::ResourceInterfaceVariable* resource_variable;
        uint32_t mapping_index;

        // Sort by binding
        bool operator<(const MappingInfo& other) const {
            return resource_variable->decorations.binding < other.resource_variable->decorations.binding;
        }

        void Print(const CommandBufferSubState& cb_sub_state, std::ostringstream& map_ss, const VkPhysicalDevice physical_device) {
            map_ss << "    - Binding " << std::dec << resource_variable->decorations.binding << ", "
                   << string_VkDescriptorMappingSourceEXT(mapping->source) << " (from pMappings[" << mapping_index << "])\n";
            const bool is_sampler =
                (resource_variable->base_type.Opcode() == spv::OpTypeSampledImage) || resource_variable->is_type_sampled_image;
            const bool is_array = resource_variable->IsArray();
            uint32_t array_length = 0;
            if (is_array && !resource_variable->IsRuntimeArray() && resource_variable->array_length != spirv::kSpecConstant) {
                array_length = resource_variable->array_length;
            }

            const VkDescriptorType descriptor_type = GetResourceInterfaceVariableType(*resource_variable);
            // TODO - Cache these once on device creation
            VkDeviceSize descriptor_size = descriptor_type == VK_DESCRIPTOR_TYPE_MAX_ENUM
                                               ? 0
                                               : DispatchGetPhysicalDeviceDescriptorSizeEXT(physical_device, descriptor_type);

            // attempts to catch obvious OOB offsets in mappings
            bool warn_oob = false;
            bool warn_index_oob = false;

            map_ss << "      - ";

            vvl::CommandBuffer::DescriptorHeap& heap = cb_sub_state.base.descriptor_heap;
            if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT) {
                const VkDescriptorMappingSourceConstantOffsetEXT& map_data = mapping->sourceData.constantOffset;
                if (is_sampler) {
                    map_ss << "samplerHeapOffset: 0x" << std::hex << map_data.samplerHeapOffset << ", samplerHeapArrayStride: 0x"
                           << map_data.samplerHeapArrayStride;
                    map_ss << "\n        Heap address: 0x" << heap.sampler_range.begin + map_data.samplerHeapOffset;
                    if (is_array) {
                        map_ss << " + (descriptor_index * 0x" << map_data.samplerHeapArrayStride << ")";
                        if (array_length != 0) {
                            warn_index_oob = ((map_data.samplerHeapOffset + ((array_length - 1) * map_data.samplerHeapArrayStride) +
                                               descriptor_size) > heap.sampler_range.size());
                        }
                    }
                    warn_oob |= (map_data.samplerHeapOffset + descriptor_size > heap.sampler_range.size());
                } else {
                    map_ss << "heapOffset: 0x" << std::hex << map_data.heapOffset << ", heapArrayStride: 0x"
                           << map_data.heapArrayStride;
                    map_ss << "\n        Heap address: 0x" << heap.resource_range.begin + map_data.heapOffset;
                    if (is_array) {
                        map_ss << " + (descriptor_index * 0x" << map_data.heapArrayStride << ")";
                        if (array_length != 0) {
                            warn_index_oob = ((map_data.heapOffset + ((array_length - 1) * map_data.heapArrayStride) +
                                               descriptor_size) > heap.resource_range.size());
                        }
                    }
                    warn_oob |= (map_data.heapOffset + descriptor_size > heap.resource_range.size());
                }
            } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT) {
                const VkDescriptorMappingSourcePushIndexEXT& map_data = mapping->sourceData.pushIndex;
                uint32_t push_index = *((uint32_t*)&cb_sub_state.push_data_value[map_data.pushOffset]);

                if (is_sampler) {
                    map_ss << "pushOffset: 0x" << std::hex << map_data.pushOffset << ", samplerHeapOffset: 0x"
                           << map_data.samplerHeapOffset << ", samplerHeapIndexStride: 0x" << map_data.samplerHeapIndexStride
                           << ", samplerHeapArrayStride: 0x" << map_data.samplerHeapArrayStride;
                    map_ss << "\n        pushIndex: 0x" << push_index;
                    map_ss << "\n        Heap address: 0x" << heap.sampler_range.begin + map_data.samplerHeapOffset << " + (0x"
                           << push_index << " * 0x" << map_data.samplerHeapIndexStride << ")";
                    if (is_array) {
                        map_ss << " + (descriptor_index * 0x" << map_data.samplerHeapArrayStride << ")";
                        if (array_length != 0) {
                            warn_index_oob = ((map_data.samplerHeapOffset + ((array_length - 1) * map_data.samplerHeapArrayStride) +
                                               descriptor_size) > heap.sampler_range.size());
                        }
                    } else {
                        uint64_t final_offset = map_data.samplerHeapOffset + (push_index * map_data.samplerHeapIndexStride);
                        map_ss << " [final address 0x" << (heap.sampler_range.begin + final_offset) << "]";
                        warn_oob |= (final_offset + descriptor_size > heap.sampler_range.size());
                    }
                    warn_oob |= (map_data.samplerHeapOffset + descriptor_size > heap.sampler_range.size());
                } else {
                    map_ss << "pushOffset: 0x" << std::hex << map_data.pushOffset << ", heapOffset: 0x" << map_data.heapOffset
                           << ", heapIndexStride: 0x" << map_data.heapIndexStride << ", heapArrayStride: 0x"
                           << map_data.heapArrayStride;
                    map_ss << "\n        pushIndex: 0x" << push_index;
                    map_ss << "\n        Heap address: 0x" << heap.resource_range.begin + map_data.heapOffset << " + (0x"
                           << push_index << " * 0x" << map_data.heapIndexStride << ")";
                    if (is_array) {
                        map_ss << " + (descriptor_index * 0x" << map_data.heapArrayStride << ")";
                        if (array_length != 0) {
                            warn_index_oob = ((map_data.heapOffset + ((array_length - 1) * map_data.heapArrayStride) +
                                               descriptor_size) > heap.resource_range.size());
                        }
                    } else {
                        uint64_t final_offset = map_data.heapOffset + (push_index * map_data.heapIndexStride);
                        map_ss << " [final address 0x" << (heap.resource_range.begin + final_offset) << "]";
                        warn_oob |= (final_offset + descriptor_size > heap.resource_range.size());
                    }
                    warn_oob |= (map_data.heapOffset + descriptor_size > heap.resource_range.size());
                }
            } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
                const VkDescriptorMappingSourceIndirectIndexEXT& map_data = mapping->sourceData.indirectIndex;
                uint64_t indirect_address = *((uint64_t*)&cb_sub_state.push_data_value[map_data.pushOffset]);

                if (is_sampler) {
                    map_ss << "pushOffset: 0x" << std::hex << map_data.pushOffset << ", addressOffset: 0x" << map_data.addressOffset
                           << ", samplerHeapOffset: 0x" << map_data.samplerHeapOffset << ", samplerHeapIndexStride: 0x"
                           << map_data.samplerHeapIndexStride << ", samplerHeapArrayStride: 0x" << map_data.samplerHeapArrayStride;
                    map_ss << "\n        indirectAddress: 0x" << indirect_address;
                    map_ss << "\n        Heap address: 0x" << heap.sampler_range.begin + map_data.samplerHeapOffset
                           << " + (indirectIndex * 0x" << map_data.samplerHeapIndexStride << ")";
                    if (is_array) {
                        map_ss << " + (descriptor_index * 0x" << map_data.samplerHeapArrayStride << ")";
                        if (array_length != 0) {
                            warn_index_oob = ((map_data.samplerHeapOffset + ((array_length - 1) * map_data.samplerHeapArrayStride) +
                                               descriptor_size) > heap.sampler_range.size());
                        }
                    }
                    warn_oob |= (map_data.samplerHeapOffset + descriptor_size > heap.sampler_range.size());
                } else {
                    map_ss << "pushOffset: 0x" << std::hex << map_data.pushOffset << ", addressOffset: 0x" << map_data.addressOffset
                           << ", heapOffset: 0x" << map_data.heapOffset << ", heapIndexStride: 0x" << map_data.heapIndexStride
                           << ", heapArrayStride: 0x" << map_data.heapArrayStride;
                    map_ss << "\n        indirectAddress: 0x" << indirect_address;
                    map_ss << "\n        Heap address: 0x" << heap.resource_range.begin + map_data.heapOffset
                           << " + (indirectIndex * 0x" << map_data.heapIndexStride << ")";
                    if (is_array) {
                        map_ss << " + (descriptor_index * 0x" << map_data.heapArrayStride << ")";
                        if (array_length != 0) {
                            warn_index_oob = ((map_data.heapOffset + ((array_length - 1) * map_data.heapArrayStride) +
                                               descriptor_size) > heap.resource_range.size());
                        }
                    }
                    warn_oob |= (map_data.heapOffset + descriptor_size > heap.resource_range.size());
                }
            } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
                const VkDescriptorMappingSourceIndirectIndexArrayEXT& map_data = mapping->sourceData.indirectIndexArray;
                uint64_t indirect_address = *((uint64_t*)&cb_sub_state.push_data_value[map_data.pushOffset]);

                if (is_sampler) {
                    map_ss << "pushOffset: 0x" << std::hex << map_data.pushOffset << ", addressOffset: 0x" << map_data.addressOffset
                           << ", samplerHeapOffset: 0x" << map_data.samplerHeapOffset << ", samplerHeapIndexStride: 0x"
                           << map_data.samplerHeapIndexStride;
                    map_ss << "\n        indirectAddress: 0x" << indirect_address;
                    map_ss << "\n        Heap address: 0x" << heap.sampler_range.begin + map_data.samplerHeapOffset
                           << " + (indirectIndex * 0x" << map_data.samplerHeapIndexStride << ")";
                    warn_oob |= (map_data.samplerHeapOffset + descriptor_size > heap.sampler_range.size());
                } else {
                    map_ss << "pushOffset: 0x" << std::hex << map_data.pushOffset << ", addressOffset: 0x" << map_data.addressOffset
                           << ", heapOffset: 0x" << map_data.heapOffset << ", heapIndexStride: 0x" << map_data.heapIndexStride;
                    map_ss << "\n        indirectAddress: 0x" << indirect_address;
                    map_ss << "\n        Heap address: 0x" << heap.resource_range.begin + map_data.heapOffset
                           << " + (indirectIndex * 0x" << map_data.heapIndexStride << ")";
                    warn_oob |= (map_data.heapOffset + descriptor_size > heap.resource_range.size());
                }
            } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT) {
                const VkDescriptorMappingSourceHeapDataEXT& map_data = mapping->sourceData.heapData;
                uint32_t push_data = *((uint32_t*)&cb_sub_state.push_data_value[map_data.pushOffset]);

                map_ss << "pushOffset: 0x" << std::hex << map_data.pushOffset << ", heapOffset: 0x" << map_data.heapOffset;
                map_ss << "\n        Push data at 0x" << std::hex << map_data.pushOffset << ": 0x" << push_data;
                map_ss << "\n        Heap address: 0x" << heap.resource_range.begin + map_data.heapOffset << " + 0x" << push_data;
                uint64_t final_offset = map_data.heapOffset + push_data;
                map_ss << " [final address 0x" << (heap.resource_range.begin + final_offset) << "]";
                warn_oob |= (final_offset + descriptor_size > heap.resource_range.size());
            } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT) {
                map_ss << "pushDataOffset: 0x" << std::hex << mapping->sourceData.pushDataOffset;
            } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT) {
                uint64_t indirect_address = *((uint64_t*)&cb_sub_state.push_data_value[mapping->sourceData.pushAddressOffset]);
                map_ss << "pushAddressOffset: 0x" << std::hex << mapping->sourceData.pushAddressOffset;
                map_ss << "\n        Indirect Adresss 0x" << indirect_address;
            } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT) {
                const VkDescriptorMappingSourceIndirectAddressEXT& map_data = mapping->sourceData.indirectAddress;
                uint64_t indirect_address = *((uint64_t*)&cb_sub_state.push_data_value[map_data.pushOffset]);
                map_ss << "pushOffset: 0x" << std::hex << map_data.pushOffset << ", addressOffset: 0x" << map_data.addressOffset;
                map_ss << "\n        Indirect Adresss 0x" << indirect_address;
            } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT) {
                // TODO - Add address for RTX
                const VkDescriptorMappingSourceShaderRecordIndexEXT& map_data = mapping->sourceData.shaderRecordIndex;
                if (is_sampler) {
                    map_ss << "samplerHeapOffset: 0x" << std::hex << map_data.samplerHeapOffset << ", samplerShaderRecordOffset: 0x"
                           << map_data.samplerShaderRecordOffset << ", samplerHeapIndexStride: 0x"
                           << map_data.samplerHeapIndexStride << ", samplerHeapArrayStride: 0x" << map_data.samplerHeapArrayStride;
                } else {
                    map_ss << "heapOffset: 0x" << std::hex << map_data.heapOffset << ", shaderRecordOffset: 0x"
                           << map_data.shaderRecordOffset << ", heapIndexStride: 0x" << map_data.heapIndexStride
                           << ", heapArrayStride: 0x" << map_data.heapArrayStride;
                }
            } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_DATA_EXT) {
                // TODO - Add more info probably
                map_ss << "shaderRecordDataOffset: " << std::hex << mapping->sourceData.shaderRecordDataOffset;
            } else if (mapping->source == VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_ADDRESS_EXT) {
                // TODO - Add more info probably
                map_ss << "shaderRecordAddressOffset: " << std::hex << mapping->sourceData.shaderRecordAddressOffset;
            }

            if (descriptor_type != VK_DESCRIPTOR_TYPE_MAX_ENUM) {
                map_ss << "\n        Descriptor size: 0x" << descriptor_size << " (" << string_VkDescriptorType(descriptor_type)
                       << ")";
            }

            if (warn_oob) {
                map_ss << "\n        [WARNING] OUT OF BOUNDS - any access will be invalid";
            } else if (warn_index_oob) {
                map_ss << "\n        [WARNING] OUT OF BOUNDS - descriptor has an array length of " << array_length
                       << " and some elements will be invalid if accessed";
            }

            map_ss << '\n';
        }
    };

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
            for (MappingInfo& set_info : mapping_info_list) {
                set_info.Print(*this, ss, dev_data.physical_device);
            }
        }
    }
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

    if (descriptor_mode == vvl::DescriptorModeBuffer) {
        DumpDescriptorBuffer(ss, last_bound);
    } else if (descriptor_mode == vvl::DescriptorModeHeap) {
        DumpDescriptorHeap(ss, last_bound);
    }

    if (dev_data.gpu_dump_settings.to_stdout) {
        std::cout << "GPU-DUMP " << ss.str() << '\n';
    } else {
        // Don't provide a LogObjectList, embed it into the message instead to keep things cleaner
        // (because the default callback will list them at the bottom)
        dev_data.debug_report->LogMessage(kInformationBit, "GPU-DUMP", {}, loc, ss.str().c_str());
    }
}

}  // namespace gpudump
