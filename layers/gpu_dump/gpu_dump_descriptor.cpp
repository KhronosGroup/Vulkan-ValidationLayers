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

#include "generated/spirv_grammar_helper.h"
#include "gpu_dump_state.h"
#include "gpu_dump.h"
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
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
#include "containers/container_utils.h"
#include "containers/custom_containers.h"
#include "containers/limits.h"
#include "generated/dispatch_functions.h"
#include "error_message/log_message_type.h"
#include "generated/error_location_helper.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/descriptor_mode.h"
#include "state_tracker/pipeline_layout_state.h"
#include "state_tracker/pipeline_state.h"
#include "state_tracker/shader_instruction.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/shader_stage_state.h"
#include "state_tracker/state_tracker.h"
#include "utils/math_utils.h"
#include "utils/descriptor_utils.h"
#include "utils/spirv_tools_utils.h"

namespace gpudump {

// Return true if found warning
bool CommandBufferSubState::DumpDescriptorBuffer(std::ostringstream& ss, const LastBound& last_bound) const {
    const vvl::CommandBuffer& cb_state = last_bound.cb_state;
    bool found_warning = false;
    ss << "- vkCmdBindDescriptorBuffersEXT last bound the following descriptor buffers:\n";
    for (uint32_t binding_i = 0; binding_i < cb_state.descriptor_buffer.binding_info.size(); binding_i++) {
        const VkDeviceAddress address = cb_state.descriptor_buffer.binding_info[binding_i].address;
        ss << "  - pBindingInfos[" << std::dec << binding_i << "].address 0x" << std::hex << address << '\n';
        found_warning |= dev_data.ListBuffers(ss, address, 1);
    }

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
            binding_ss << "    - SPIR-V Binding " << std::dec << index << " [\"" << variable_name << "\"]\n";
            binding_ss << "      - " << string_VkDescriptorType(type) << "\n";
            binding_ss << "      - offset: " << offset << ", range: " << string_range_hex(range) << "\n";
            binding_ss << "      - size: " << DescribeDescriptorBufferSize(robust_buffer, type) << " [0x" << std::hex << size
                       << "]";
            if (count > 1) {
                binding_ss << " * descriptorCount [" << std::dec << count << "]";
            }
            binding_ss << "\n";
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
            set_ss << "  - SPIR-V Set " << std::dec << index << "\n    - size: " << range.size()
                   << " bytes, range: " << string_range_hex(range);
            if (binding_index != vvl::kNoIndex32) {
                // Only print if there are multiple bindings
                set_ss << " (specified in pBindingInfos[" << std::dec << binding_index << "])";
            }
            set_ss << '\n';
        }
    };

    small_vector<const ShaderStageState*, 3> stages = last_bound.GetStages();

    // Can be a push constant only shader, which is valid here
    // But if there are descriptors it is only valid if they are no accessed, which is warning territory
    if (!last_bound.desc_set_pipeline_layout) {
        ss << "- No VkPipelineLayout found from a previous vkCmdSetDescriptorBufferOffsetsEXT call\n";

        bool uses_descriptors = false;
        for (const ShaderStageState* stage : stages) {
            if (stage->HasSpirv() && !stage->entrypoint->resource_interface_variables.empty()) {
                uses_descriptors = true;
                break;
            }
        }
        if (uses_descriptors) {
            ss << "- [WARNING] no vkCmdSetDescriptorBufferOffsetsEXT was called so any accesses to the descriptors in the shader "
                  "will be invalid.\n";
            // quickly check if they set the wrong bind point (only for the more common one)
            if (last_bound.bind_point != VK_PIPELINE_BIND_POINT_GRAPHICS &&
                cb_state.GetLastBoundGraphics().desc_set_pipeline_layout) {
                ss << "    - vkCmdSetDescriptorBufferOffsetsEXT was called with VK_PIPELINE_BIND_POINT_GRAPHICS, did you mean "
                   << string_VkPipelineBindPoint(last_bound.bind_point) << "?\n";
            } else if (last_bound.bind_point != VK_PIPELINE_BIND_POINT_COMPUTE &&
                       cb_state.GetLastBoundCompute().desc_set_pipeline_layout) {
                ss << "    - vkCmdSetDescriptorBufferOffsetsEXT was called with VK_PIPELINE_BIND_POINT_COMPUTE, did you mean "
                   << string_VkPipelineBindPoint(last_bound.bind_point) << "?\n";
            } else if (last_bound.bind_point != VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR &&
                       cb_state.GetLastBoundRayTracing().desc_set_pipeline_layout) {
                ss << "    - vkCmdSetDescriptorBufferOffsetsEXT was called with VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, did you "
                      "mean "
                   << string_VkPipelineBindPoint(last_bound.bind_point) << "?\n";
            }
            found_warning = true;
        }
        return found_warning;
    }

    const vvl::PipelineLayout& pipeline_layout = *last_bound.desc_set_pipeline_layout;
    ss << "- vkCmdSetDescriptorBufferOffsetsEXT last bound with " << dev_data.FormatHandle(pipeline_layout.VkHandle()) << "\n";

    ss << "- Shader descriptors:\n";
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
            ss << " " << dev_data.FormatHandle(stage->module_state->VkHandle());
        }
        ss << "\n";

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
                    ss << "  - [WARNING] Set " << std::dec << var_set
                       << " VkDescriptorSetLayout was destroyed (TODO - Track more info in pipeline layout)\n";
                    found_warning = true;
                    continue;
                } else if ((dsl->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) == 0) {
                    ss << "  - [WARNING] Set " << std::dec << var_set
                       << " was not created with VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT\n";
                    found_warning = true;
                    continue;
                } else if (!descriptor_buffer_binding.has_value()) {
                    ss << "  - [WARNING] Set " << std::dec << var_set
                       << " was never bound with offset. This is only valid if descriptor is not used in the shader";
                    if (dsl->HasImmutableSamplers()) {
                        ss << " or because all bindings are using Immutable Samplers";
                    }
                    ss << "\n";
                    found_warning = true;
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
                    info.variable_name = info.variable_name + ", " + resource_variable.debug_name + "";
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
    return found_warning;
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

static VkDeviceSize GetDescriptorAlignment(VkDescriptorType type, const vvl::DeviceExtensionProperties& phys_dev_ext_props) {
    if (type == VK_DESCRIPTOR_TYPE_TENSOR_ARM) {
        return phys_dev_ext_props.descriptor_heap_tensor_props.tensorDescriptorAlignment;
    } else if (type == VK_DESCRIPTOR_TYPE_SAMPLER) {
        return phys_dev_ext_props.descriptor_heap_props.samplerDescriptorAlignment;
    } else if (IsValueIn(type,
                         {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                          VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER})) {
        return phys_dev_ext_props.descriptor_heap_props.imageDescriptorAlignment;
    } else if (IsValueIn(type, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR})) {
        return phys_dev_ext_props.descriptor_heap_props.bufferDescriptorAlignment;
    }
    assert(false);
    return 0;
}

static vvl::Field GetDescriptorAlignmentField(VkDescriptorType type) {
    if (type == VK_DESCRIPTOR_TYPE_TENSOR_ARM) {
        return vvl::Field::tensorDescriptorAlignment;
    } else if (type == VK_DESCRIPTOR_TYPE_SAMPLER) {
        return vvl::Field::samplerDescriptorAlignment;
    } else if (IsValueIn(type,
                         {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                          VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER})) {
        return vvl::Field::imageDescriptorAlignment;
    } else if (IsValueIn(type, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR})) {
        return vvl::Field::bufferDescriptorAlignment;
    }
    assert(false);
    return vvl::Field::Empty;
}

bool CommandBufferSubState::DumpDescriptorHeapMapping(std::ostringstream& ss, const MappingInfo& mapping_info) const {
    const VkDescriptorSetAndBindingMappingEXT& mapping = *mapping_info.mapping;
    const spirv::ResourceInterfaceVariable& resource_variable = *mapping_info.resource_variable;
    vvl::CommandBuffer::DescriptorHeap& heap = base.descriptor_heap;

    const char* new_line = "\n        ";
    const char* new_bullet_line = "\n      - ";

    ss << "    - SPIR-V Binding " << std::dec << resource_variable.decorations.binding;
    if (!resource_variable.debug_name.empty()) {
        ss << " [\"" << resource_variable.debug_name << "\"]";
    }

    ss << new_bullet_line << "specified in pMappings[" << mapping_info.index << "]: binding count = " << mapping.bindingCount
       << ", source: " << string_VkDescriptorMappingSourceEXT(mapping.source);
    const bool is_sampler = resource_variable.is_sampler;
    const char* main_heap_type = is_sampler ? "Sampler" : "Resource";
    const vvl::range<VkDeviceAddress>& heap_range = is_sampler ? heap.sampler_range : heap.resource_range;
    const vvl::range<VkDeviceAddress>& heap_reserved = is_sampler ? heap.sampler_reserved : heap.resource_reserved;

    // Will be false if mapping uses embedded samplers instead
    bool dump_sampler = resource_variable.is_combined_image_sampler;

    const bool is_array = resource_variable.IsArray();
    const bool is_runtime_array = resource_variable.IsRuntimeArray();
    uint32_t array_length = 0;
    if (is_array && !is_runtime_array && resource_variable.array_length != spirv::kSpecConstant) {
        array_length = resource_variable.array_length;
    }

    VkDescriptorType descriptor_type = resource_variable.GetPotentialDescriptorType();

    VkDeviceSize descriptor_size = 0;
    VkDeviceSize sampler_descriptor_size = dev_data.phys_dev_ext_props.descriptor_heap_props.samplerDescriptorSize;
    if (descriptor_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        assert(resource_variable.is_combined_image_sampler);
        // not valid to query this type, we just want the "resource" portion as the sampler is handled itself later
        descriptor_type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descriptor_size = dev_data.phys_dev_ext_props.descriptor_heap_props.imageDescriptorSize;
    } else if (descriptor_type != VK_DESCRIPTOR_TYPE_MAX_ENUM) {
        descriptor_size = dev_data.device_state->cached_descriptor_size.GetSize(descriptor_type);
    }

    VkDeviceSize required_alignment = GetDescriptorAlignment(descriptor_type, dev_data.phys_dev_ext_props);
    vvl::Field alignment_name = GetDescriptorAlignmentField(descriptor_type);

    // attempts to catch obvious OOB offsets in mappings
    std::ostringstream warn_ss;
    bool warn_indirect_buffer = false;
    uint32_t warn_reserved_range_start = vvl::kNoIndex32;
    uint32_t warn_reserved_range_end = vvl::kNoIndex32;

    auto warn_oob = [&](VkDeviceSize offset, bool from_sampler) {
        if (from_sampler) {
            if (offset > heap.sampler_range.size()) {
                warn_ss
                    << new_bullet_line
                    << "[WARNING] OUT OF BOUNDS - descriptor not in sampler heap and any access to this descriptor will be invalid";
            }
        } else {
            if (offset > heap_range.size()) {
                warn_ss << new_bullet_line
                        << "[WARNING] OUT OF BOUNDS - descriptor not in resource heap and any access to this descriptor will be "
                           "invalid";
            }
        }
    };

    auto warn_alignment_scalar_indirect = [&](VkDeviceAddress address, VkDeviceSize alignment) {
        if (!IsPointerAligned(address, alignment)) {
            warn_ss << new_bullet_line << "[WARNING] MISALIGNED - the indirect address is not aligned to ";
            if (alignment == 4) {
                warn_ss << "4 (scalar alignment for a uint32_t)";
            } else if (alignment == 8) {
                warn_ss << "8 (scalar alignment for a VkDeviceAddress)";
            } else {
                assert(false);
            }
            warn_ss << " and any access to this descriptor will be invalid";
        }
    };

    auto warn_indirect_uniform_usage = [&](VkDeviceAddress address) {
        auto buffer_states = dev_data.GetBuffersByAddress(address);
        // warning elsewhere if this is empty
        if (!buffer_states.empty()) {
            bool found = false;
            for (const auto& buffer_state : buffer_states) {
                if (buffer_state->usage & VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                if (buffer_states.size() == 1) {
                    const vvl::Buffer& buffer_state = *buffer_states.front();
                    warn_ss << new_bullet_line << "[WARNING] BUFFER TYPE - the indirect address (0x" << std::hex << address
                            << ") belongs to " << buffer_state.Describe(dev_data) << ", but that VkBuffer was created with usage "
                            << string_VkBufferUsageFlags2(buffer_state.usage)
                            << " (missing VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT) and any access to this descriptor will be "
                               "invalid";
                } else {
                    warn_ss
                        << new_bullet_line << "[WARNING] BUFFER TYPE - the indirect address (0x" << std::hex << address
                        << ") is not accessing any VkBuffer created with VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT and any access to "
                           "this descriptor will be invalid";
                }
            }
        }
    };

    auto warn_resource_buffer_usage = [&](VkDeviceAddress address) {
        // Core validation ensure this for the given mappings
        assert((resource_variable.is_storage_buffer && descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) ||
               (resource_variable.is_uniform_buffer && descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));
        auto buffer_states = dev_data.GetBuffersByAddress(address);
        // warning elsewhere if this is empty
        if (!buffer_states.empty()) {
            const VkBufferUsageFlagBits2 search_usage =
                resource_variable.is_storage_buffer ? VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT : VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT;
            bool found = false;
            for (const auto& buffer_state : buffer_states) {
                if (buffer_state->usage & search_usage) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                warn_ss << new_bullet_line << "[WARNING] BUFFER TYPE - the resource address (0x" << std::hex << address
                        << ") is not accessing any VkBuffer created with "
                        << (resource_variable.is_storage_buffer ? "VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT"
                                                                : "VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT")
                        << " and any access to this descriptor will be invalid";
            }
        }
    };

    auto warn_alignment_indirect_address = [&](VkDeviceAddress address, bool from_resource = false) {
        VkDeviceSize alignment = 0;
        if (resource_variable.is_uniform_buffer) {
            alignment = dev_data.phys_dev_props.limits.minUniformBufferOffsetAlignment;
        } else if (resource_variable.is_storage_buffer) {
            alignment = dev_data.phys_dev_props.limits.minStorageBufferOffsetAlignment;
        } else if (resource_variable.is_acceleration_structure) {
            // TODO - confirm
            alignment = 256;
        }

        if (!IsPointerAligned(address, alignment)) {
            warn_ss << new_bullet_line << "[WARNING] MISALIGNED - the ";
            if (from_resource) {
                warn_ss << "resource";
            } else {
                warn_ss << "indirect";
            }
            warn_ss << " address is not aligned to ";

            if (resource_variable.is_uniform_buffer) {
                warn_ss << "minUniformBufferOffsetAlignment (0x" << std::hex << alignment
                        << ") and any access to this descriptor will be invalid";
            } else if (resource_variable.is_storage_buffer) {
                warn_ss << "minStorageBufferOffsetAlignment (0x" << std::hex << alignment << ");";
            } else if (resource_variable.is_acceleration_structure) {
                // TODO - confirm
                warn_ss << "0x" << std::hex << alignment;
            }
            warn_ss << " and any access to this descriptor will be invalid";
        }
    };

    auto warn_alignment_descriptor = [&](VkDeviceAddress address) {
        if (!IsPointerAligned(address, required_alignment)) {
            warn_ss << new_bullet_line << "[WARNING] MISALIGNED - the final address";
            if (resource_variable.IsArray()) {
                warn_ss << ", to the first element of the array,";
            }
            warn_ss << " is not aligned to ";
            if (alignment_name == vvl::Field::Empty) {
                warn_ss << "[UNKNOWN type]";
            } else {
                warn_ss << String(alignment_name) << " ";
            }
            warn_ss << "(0x" << std::hex << required_alignment << ") and any access to this descriptor will be invalid";
        }
    };

    auto warn_alignment_sampler = [&](VkDeviceAddress address) {
        if (!IsPointerAligned(address, dev_data.phys_dev_ext_props.descriptor_heap_props.samplerDescriptorAlignment)) {
            warn_ss << new_bullet_line << "[WARNING] MISALIGNED - the final address";
            if (resource_variable.IsArray()) {
                warn_ss << ", to the first element of the array,";
            }
            warn_ss << " is not aligned to samplerDescriptorAlignment " << "(0x" << std::hex
                    << dev_data.phys_dev_ext_props.descriptor_heap_props.samplerDescriptorAlignment
                    << ") and any access to this descriptor will be invalid";
        }
    };

    auto warn_array_stride = [&](uint32_t array_stride, bool is_sampler) {
        if (array_stride == 0) {
            warn_ss << new_bullet_line << "[WARNING] ZERO ARRAY STRIDE - "
                    << (is_sampler ? "samplerHeapArrayStride" : "heapArrayStride")
                    << " is zero, this mean every index of the descriptor array will be the same descriptor, which is likely not "
                       "desired.";
        }
    };

    auto warn_index_oob = [&](uint32_t max_index) {
        if (array_length > (max_index + 1)) {
            warn_ss << new_bullet_line << "[WARNING] OUT OF BOUNDS - descriptor has an array length of [" << std::dec
                    << array_length << "] but any element accessed starting at [" << max_index + 1 << std::hex
                    << "] will be invalid if accessed";
        }
    };

    auto warn_index_array = [&](std::vector<uint32_t>& bad_indexes) {
        if (!bad_indexes.empty()) {
            warn_ss << new_bullet_line << "[WARNING] OUT OF BOUNDS - descriptors indexes at [" << std::dec;
            for (uint32_t i = 0; i < bad_indexes.size(); i++) {
                if (i != 0) warn_ss << ", ";
                warn_ss << bad_indexes[i];
            }
            warn_ss << std::hex << "] will be invalid if accessed";
        }
    };

    auto warn_alignment_index_array = [&](std::vector<uint32_t>& bad_indexes) {
        if (!bad_indexes.empty()) {
            warn_ss << new_bullet_line << "[WARNING] MISALIGNED - descriptors indexes at [" << std::dec;
            for (uint32_t i = 0; i < bad_indexes.size(); i++) {
                if (i != 0) warn_ss << ", ";
                warn_ss << bad_indexes[i];
            }
            warn_ss << "] will not be aligned to ";
            if (alignment_name != vvl::Field::Empty) {
                warn_ss << String(alignment_name) << " ";
            }
            warn_ss << "(0x" << std::hex << required_alignment << ") and any access to this descriptor will be invalid";
        }
    };

    auto warn_reserved_range_index_array = [&](std::vector<uint32_t>& bad_indexes) {
        if (!bad_indexes.empty()) {
            warn_ss << new_bullet_line << "[WARNING] RESERVED RANGE - descriptors indexes at [" << std::dec;
            for (uint32_t i = 0; i < bad_indexes.size(); i++) {
                if (i != 0) warn_ss << ", ";
                warn_ss << bad_indexes[i];
            }
            warn_ss << std::hex << "] will overlap with the reserved range and any access will be invalid";
        }
    };

    auto warn_alignment_index_array_sampler = [&](std::vector<uint32_t>& bad_indexes) {
        if (!bad_indexes.empty()) {
            warn_ss << new_bullet_line << "[WARNING] MISALIGNED - descriptors indexes at [" << std::dec;
            for (uint32_t i = 0; i < bad_indexes.size(); i++) {
                if (i != 0) warn_ss << ", ";
                warn_ss << bad_indexes[i];
            }
            warn_ss << "] will not be aligned to samplerDescriptorAlignment (0x" << std::hex << required_alignment
                    << ") and any access to this descriptor will be invalid";
        }
    };

    ss << new_bullet_line;
    if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT) {
        const VkDescriptorMappingSourceConstantOffsetEXT& map_data = mapping.sourceData.constantOffset;

        ss << "heapOffset: 0x" << std::hex << map_data.heapOffset << ", heapArrayStride: 0x" << map_data.heapArrayStride;
        VkDeviceSize index_zero_offset = map_data.heapOffset;
        // CONSTANT_OFFSET will be fully caught already in normal VVL if alignment is off
        VkDeviceAddress index_zero_address = heap_range.begin + index_zero_offset;
        ss << new_line << main_heap_type << " Heap address: 0x" << index_zero_address;
        if (is_array) {
            ss << " + (descriptor_index * 0x" << map_data.heapArrayStride << ")";
            const VkDeviceSize available_space = (heap_range.size() - index_zero_offset) - descriptor_size;
            warn_array_stride(map_data.heapArrayStride, false);
            const uint32_t max_index = map_data.heapArrayStride == 0 ? 1 : (uint32_t)(available_space / map_data.heapArrayStride);
            if (array_length != 0) {
                const VkDeviceSize final_array_offset = (array_length - 1) * map_data.heapArrayStride;
                ss << new_line << "    The final descriptor index at [" << std::dec << array_length << std::hex
                   << "] will access [0x" << (index_zero_address + final_array_offset) << ", 0x"
                   << (index_zero_address + final_array_offset + descriptor_size) << ")";
                warn_index_oob(max_index);
            } else if (is_runtime_array) {
                const VkDeviceSize final_array_offset = (max_index * map_data.heapArrayStride);
                ss << new_line << "    The final descriptor index in bounds is [" << std::dec << max_index << std::hex
                   << "] which would be accessed at [0x" << (index_zero_address + final_array_offset) << ", 0x"
                   << (index_zero_address + final_array_offset + descriptor_size) << ")";
            }

            if (!heap_reserved.empty()) {
                const uint32_t max_search_index = array_length != 0 ? array_length : max_index + 1;
                for (uint32_t i = 0; i < max_search_index; i++) {
                    // TODO - be smart where to start searching if reserved range is at the end of huge buffer
                    VkDeviceAddress next_index_address = index_zero_address + (i * map_data.heapArrayStride);
                    vvl::range<VkDeviceAddress> next_index_range{next_index_address, next_index_address + descriptor_size};
                    if (next_index_range.intersects(heap_reserved)) {
                        if (warn_reserved_range_start == vvl::kNoIndex32) {
                            warn_reserved_range_start = i;
                        }
                        warn_reserved_range_end = i;
                    } else if (warn_reserved_range_end != vvl::kNoIndex32) {
                        break; // found end of reserved range
                    }
                }
            }
        } else if (!heap_reserved.empty()) {
            vvl::range<VkDeviceAddress> index_zero_range{index_zero_address, index_zero_address + descriptor_size};
            if (index_zero_range.intersects(heap_reserved)) {
                warn_reserved_range_start = 0;
            }
        }

        warn_oob(index_zero_offset + descriptor_size, false);

        dump_sampler &= map_data.pEmbeddedSampler == nullptr;
        if (dump_sampler) {
            ss << new_bullet_line << "samplerHeapOffset: 0x" << std::hex << map_data.samplerHeapOffset
               << ", samplerHeapArrayStride: 0x" << map_data.samplerHeapArrayStride;
            index_zero_offset = map_data.samplerHeapOffset;
            index_zero_address = heap.sampler_range.begin + index_zero_offset;
            ss << new_line << "Sampler Heap address: 0x" << index_zero_address;
            if (is_array) {
                ss << " + (descriptor_index * 0x" << map_data.samplerHeapArrayStride << ")";
                const VkDeviceSize available_space = (heap.sampler_range.size() - index_zero_offset) - sampler_descriptor_size;
                warn_array_stride(map_data.samplerHeapArrayStride, true);
                const uint32_t max_index =
                    map_data.samplerHeapArrayStride == 0 ? 1 : (uint32_t)(available_space / map_data.samplerHeapArrayStride);
                if (array_length != 0) {
                    const VkDeviceSize final_array_offset = (array_length - 1) * map_data.samplerHeapArrayStride;
                    ss << new_line << "    The final descriptor index at [" << std::dec << array_length << std::hex
                       << "] will access [0x" << (index_zero_address + final_array_offset) << ", 0x"
                       << (index_zero_address + final_array_offset + sampler_descriptor_size) << ")";
                    warn_index_oob(max_index);
                } else if (is_runtime_array) {
                    const VkDeviceSize final_array_offset =
                        (map_data.samplerHeapOffset + (max_index * map_data.samplerHeapArrayStride));
                    ss << new_line << "    The final descriptor index in bounds is [" << std::dec << max_index << std::hex
                       << "] which would be accessed at [0x" << (index_zero_address + final_array_offset) << ", 0x"
                       << (index_zero_address + final_array_offset + sampler_descriptor_size) << ")";
                }

                if (!heap.sampler_reserved.empty() && warn_reserved_range_start == vvl::kNoIndex32) {
                    const uint32_t max_search_index = array_length != 0 ? array_length : max_index + 1;
                    for (uint32_t i = 0; i < max_search_index; i++) {
                        VkDeviceAddress next_index_address = index_zero_address + (i * map_data.samplerHeapArrayStride);
                        vvl::range<VkDeviceAddress> next_index_range{next_index_address,
                                                                     next_index_address + sampler_descriptor_size};
                        if (next_index_range.intersects(heap.sampler_reserved)) {
                            if (warn_reserved_range_start == vvl::kNoIndex32) {
                                warn_reserved_range_start = i;
                            }
                            warn_reserved_range_end = i;
                        } else if (warn_reserved_range_end != vvl::kNoIndex32) {
                            break; // found end of reserved range
                        }
                    }
                }
            } else if (!heap.sampler_reserved.empty()) {
                vvl::range<VkDeviceAddress> index_zero_range{index_zero_address, index_zero_address + sampler_descriptor_size};
                if (index_zero_range.intersects(heap.sampler_reserved)) {
                    warn_reserved_range_start = 0;
                }
            }
            warn_oob(index_zero_offset + sampler_descriptor_size, true);
        }
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT) {
        const VkDescriptorMappingSourcePushIndexEXT& map_data = mapping.sourceData.pushIndex;
        // This is caught by core validation already
        uint32_t push_index =
            map_data.pushOffset < push_data_value.size() ? *((uint32_t*)&push_data_value[map_data.pushOffset]) : 0xBAD00B;

        ss << "pushOffset: 0x" << std::hex << map_data.pushOffset << ", heapOffset: 0x" << map_data.heapOffset
           << ", heapIndexStride: 0x" << map_data.heapIndexStride << ", heapArrayStride: 0x" << map_data.heapArrayStride;
        ss << new_line << "pushIndex: 0x" << push_index;
        ss << new_line << main_heap_type << " Heap address: 0x" << heap.resource_range.begin + map_data.heapOffset << " + (0x"
           << push_index << " * 0x" << map_data.heapIndexStride << ")";
        VkDeviceSize index_zero_offset = map_data.heapOffset + (push_index * map_data.heapIndexStride);
        VkDeviceAddress index_zero_address = heap_range.begin + index_zero_offset;

        warn_alignment_descriptor(index_zero_address);

        if (is_array) {
            ss << " + (descriptor_index * 0x" << map_data.heapArrayStride << ")";
            const VkDeviceSize available_space = (heap_range.size() - index_zero_offset) - descriptor_size;
            warn_array_stride(map_data.heapArrayStride, false);
            const uint32_t max_index = map_data.heapArrayStride == 0 ? 1 : (uint32_t)(available_space / map_data.heapArrayStride);
            if (array_length != 0) {
                const VkDeviceSize final_array_offset = (array_length - 1) * map_data.heapArrayStride;
                ss << new_line << "    The final descriptor index at [" << std::dec << array_length << std::hex
                   << "] will access [0x" << (index_zero_address + final_array_offset) << ", 0x"
                   << (index_zero_address + final_array_offset + descriptor_size) << ")";
                warn_index_oob(max_index);
            } else if (is_runtime_array) {
                const VkDeviceSize final_array_offset = max_index * map_data.heapArrayStride;
                ss << new_line << "    The final descriptor index in bounds is [" << std::dec << max_index << std::hex
                   << "] which would be accessed at [0x" << (index_zero_address + final_array_offset) << ", 0x"
                   << (index_zero_address + final_array_offset + descriptor_size) << ")";
            }

            if (!heap_reserved.empty()) {
                const uint32_t max_search_index = array_length != 0 ? array_length : max_index + 1;
                for (uint32_t i = 0; i < max_search_index; i++) {
                    // TODO - be smart where to start searching if reserved range is at the end of huge buffer
                    VkDeviceAddress next_index_address = index_zero_address + (i * map_data.heapArrayStride);
                    vvl::range<VkDeviceAddress> next_index_range{next_index_address, next_index_address + descriptor_size};
                    if (next_index_range.intersects(heap_reserved)) {
                        if (warn_reserved_range_start == vvl::kNoIndex32) {
                            warn_reserved_range_start = i;
                        }
                        warn_reserved_range_end = i;
                    } else if (warn_reserved_range_end != vvl::kNoIndex32) {
                        break; // found end of reserved range
                    }
                }
            }
        } else {
            ss << " [final address 0x" << index_zero_address << "]";

            if (!heap_reserved.empty()) {
                vvl::range<VkDeviceAddress> index_zero_range{index_zero_address, index_zero_address + descriptor_size};
                if (index_zero_range.intersects(heap_reserved)) {
                    warn_reserved_range_start = 0;
                }
            }
        }
        warn_oob(index_zero_offset + descriptor_size, false);

        dump_sampler &= map_data.pEmbeddedSampler == nullptr;
        if (dump_sampler) {
            push_index = map_data.samplerPushOffset < push_data_value.size()
                             ? *((uint32_t*)&push_data_value[map_data.samplerPushOffset])
                             : 0xBAD00B;

            ss << new_bullet_line << "samplerPushOffset: 0x" << std::hex << map_data.samplerPushOffset << ", samplerHeapOffset: 0x"
               << map_data.samplerHeapOffset << ", samplerHeapIndexStride: 0x" << map_data.samplerHeapIndexStride
               << ", samplerHeapArrayStride: 0x" << map_data.samplerHeapArrayStride;
            ss << new_line << "pushIndex: 0x" << push_index;
            ss << new_line << "Sampler Heap address: 0x" << heap.sampler_range.begin + map_data.samplerHeapOffset << " + (0x"
               << push_index << " * 0x" << map_data.samplerHeapIndexStride << ")";
            index_zero_offset = map_data.samplerHeapOffset + (push_index * map_data.samplerHeapIndexStride);
            index_zero_address = heap.sampler_range.begin + index_zero_offset;

            warn_alignment_sampler(index_zero_address);

            if (is_array) {
                ss << " + (descriptor_index * 0x" << map_data.samplerHeapArrayStride << ")";
                const VkDeviceSize available_space = (heap.sampler_range.size() - index_zero_offset) - sampler_descriptor_size;
                warn_array_stride(map_data.samplerHeapArrayStride, true);
                const uint32_t max_index =
                    map_data.samplerHeapArrayStride == 0 ? 1 : (uint32_t)(available_space / map_data.samplerHeapArrayStride);
                if (array_length != 0) {
                    const VkDeviceSize final_array_offset = (array_length - 1) * map_data.samplerHeapArrayStride;
                    ss << new_line << "    The final descriptor index at [" << std::dec << array_length << std::hex
                       << "] will access [0x" << (index_zero_address + final_array_offset) << ", 0x"
                       << (index_zero_address + final_array_offset + sampler_descriptor_size) << ")";
                    warn_index_oob(max_index);
                } else if (is_runtime_array) {
                    const VkDeviceSize final_array_offset = max_index * map_data.samplerHeapArrayStride;
                    ss << new_line << "    The final descriptor index in bounds is [" << std::dec << max_index << std::hex
                       << "] which would be accessed at [0x" << (index_zero_address + final_array_offset) << ", 0x"
                       << (index_zero_address + final_array_offset + sampler_descriptor_size) << ")";
                }

                if (!heap.sampler_reserved.empty() && warn_reserved_range_start == vvl::kNoIndex32) {
                    const uint32_t max_search_index = array_length != 0 ? array_length : max_index + 1;
                    for (uint32_t i = 0; i < max_search_index; i++) {
                        VkDeviceAddress next_index_address = index_zero_address + (i * map_data.samplerHeapArrayStride);
                        vvl::range<VkDeviceAddress> next_index_range{next_index_address,
                                                                     next_index_address + sampler_descriptor_size};
                        if (next_index_range.intersects(heap.sampler_reserved)) {
                            if (warn_reserved_range_start == vvl::kNoIndex32) {
                                warn_reserved_range_start = i;
                            }
                            warn_reserved_range_end = i;
                        } else if (warn_reserved_range_end != vvl::kNoIndex32) {
                            break; // found end of reserved range
                        }
                    }
                }
            } else {
                ss << " [final address 0x" << index_zero_address << "]";

                if (!heap.sampler_reserved.empty()) {
                    vvl::range<VkDeviceAddress> index_zero_range{index_zero_address, index_zero_address + sampler_descriptor_size};
                    if (index_zero_range.intersects(heap.sampler_reserved)) {
                        warn_reserved_range_start = 0;
                    }
                }
            }
            warn_oob(index_zero_offset + descriptor_size, true);
        }
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
        const VkDescriptorMappingSourceIndirectIndexEXT& map_data = mapping.sourceData.indirectIndex;
        VkDeviceAddress push_indirect_address =
            map_data.pushOffset < push_data_value.size() ? *((VkDeviceAddress*)&push_data_value[map_data.pushOffset]) : 0xBAD00B;

        VkDeviceAddress final_indirect_address = push_indirect_address + map_data.addressOffset;

        warn_alignment_scalar_indirect(final_indirect_address, 4);
        warn_indirect_uniform_usage(final_indirect_address);

        std::vector<uint8_t> indirect_index_data = dev_data.CopyDataFromMemory(final_indirect_address, 4);
        bool know_ubo = !indirect_index_data.empty();
        uint32_t indirect_index = know_ubo ? *((uint32_t*)indirect_index_data.data()) : 0;

        ss << "pushOffset: 0x" << std::hex << map_data.pushOffset << ", addressOffset: 0x" << map_data.addressOffset
           << ", heapOffset: 0x" << map_data.heapOffset << ", heapIndexStride: 0x" << map_data.heapIndexStride
           << ", heapArrayStride: 0x" << map_data.heapArrayStride;
        ss << new_line << "indirectAddress: 0x" << final_indirect_address << " (0x" << push_indirect_address << " + 0x"
           << map_data.addressOffset << ")";

        warn_indirect_buffer |= dev_data.ListBuffers(ss, final_indirect_address, 3, true);

        if (know_ubo) {
            ss << new_line << "indirectIndex: 0x" << indirect_index;
        }
        ss << new_line << main_heap_type << " Heap address: 0x" << heap_range.begin + map_data.heapOffset << " + (";
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
                VkDeviceAddress index_zero_address = heap_range.begin + index_zero_offset;
                warn_alignment_descriptor(index_zero_address);

                VkDeviceSize available_space = (heap_range.size() - index_zero_offset) - descriptor_size;
                warn_array_stride(map_data.heapArrayStride, false);
                const uint32_t max_index =
                    map_data.heapArrayStride == 0 ? 1 : (uint32_t)(available_space / map_data.heapArrayStride);
                if (array_length != 0) {
                    const VkDeviceSize final_array_offset = (array_length - 1) * map_data.heapArrayStride;
                    ss << new_line << "    The final descriptor index at [" << std::dec << array_length << std::hex
                       << "] will access [0x" << (index_zero_address + final_array_offset) << ", 0x"
                       << (index_zero_address + final_array_offset + descriptor_size) << ")";
                    warn_index_oob(max_index);
                } else if (is_runtime_array) {
                    const VkDeviceSize final_array_offset = max_index * map_data.heapArrayStride;
                    ss << new_line << "    The final descriptor index in bounds is [" << std::dec << max_index << std::hex
                       << "] which would be accessed at [0x" << (index_zero_address + final_array_offset) << ", 0x"
                       << (index_zero_address + final_array_offset + descriptor_size) << ")";
                }

                if (!heap_reserved.empty()) {
                    const uint32_t max_search_index = array_length != 0 ? array_length : max_index + 1;
                    for (uint32_t i = 0; i < max_search_index; i++) {
                        // TODO - be smart where to start searching if reserved range is at the end of huge buffer
                        VkDeviceAddress next_index_address = index_zero_address + (i * map_data.heapArrayStride);
                        vvl::range<VkDeviceAddress> next_index_range{next_index_address, next_index_address + descriptor_size};
                        if (next_index_range.intersects(heap_reserved)) {
                            if (warn_reserved_range_start == vvl::kNoIndex32) {
                                warn_reserved_range_start = i;
                            }
                            warn_reserved_range_end = i;
                        } else if (warn_reserved_range_end != vvl::kNoIndex32) {
                            break; // found end of reserved range
                        }
                    }
                }
            }
        } else if (know_ubo) {
            VkDeviceAddress final_offset = map_data.heapOffset + (indirect_index * map_data.heapIndexStride);
            VkDeviceAddress index_zero_address = heap_range.begin + final_offset;
            warn_alignment_descriptor(index_zero_address);

            ss << " [final address 0x" << (heap_range.begin + final_offset) << "]";

            warn_oob(final_offset + descriptor_size, false);

            if (!heap_reserved.empty()) {
                vvl::range<VkDeviceAddress> index_zero_range{index_zero_address, index_zero_address + descriptor_size};
                if (index_zero_range.intersects(heap_reserved)) {
                    warn_reserved_range_start = 0;
                }
            }
        }

        warn_oob(map_data.heapOffset + descriptor_size, false);

        dump_sampler &= map_data.pEmbeddedSampler == nullptr;
        if (dump_sampler) {
            push_indirect_address = map_data.samplerPushOffset < push_data_value.size()
                                        ? *((VkDeviceAddress*)&push_data_value[map_data.samplerPushOffset])
                                        : 0xBAD00B;

            final_indirect_address = push_indirect_address + map_data.samplerAddressOffset;

            warn_alignment_scalar_indirect(final_indirect_address, 4);
            warn_indirect_uniform_usage(final_indirect_address);

            indirect_index_data = dev_data.CopyDataFromMemory(final_indirect_address, 4);
            know_ubo = !indirect_index_data.empty();
            indirect_index = know_ubo ? *((uint32_t*)indirect_index_data.data()) : 0;

            ss << new_bullet_line << "samplerPushOffset: 0x" << std::hex << map_data.samplerPushOffset
               << ", samplerAddressOffset: 0x" << map_data.samplerAddressOffset << ", samplerHeapOffset: 0x"
               << map_data.samplerHeapOffset << ", samplerHeapIndexStride: 0x" << map_data.samplerHeapIndexStride
               << ", samplerHeapArrayStride: 0x" << map_data.samplerHeapArrayStride;
            ss << new_line << "indirectAddress: 0x" << final_indirect_address << " (0x" << push_indirect_address << " + 0x"
               << map_data.samplerAddressOffset << ")";

            warn_indirect_buffer |= dev_data.ListBuffers(ss, final_indirect_address, 3, true);

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
                    VkDeviceAddress index_zero_address = heap.sampler_range.begin + index_zero_offset;
                    warn_alignment_sampler(index_zero_address);

                    VkDeviceSize available_space = (heap.sampler_range.size() - index_zero_offset) - sampler_descriptor_size;
                    warn_array_stride(map_data.samplerHeapArrayStride, true);
                    const uint32_t max_index =
                        map_data.samplerHeapArrayStride == 0 ? 1 : (uint32_t)(available_space / map_data.samplerHeapArrayStride);
                    if (array_length != 0) {
                        const VkDeviceSize final_array_offset = (array_length - 1) * map_data.samplerHeapArrayStride;
                        ss << new_line << "    The final descriptor index at [" << std::dec << array_length << std::hex
                           << "] will access [0x" << (index_zero_address + final_array_offset) << ", 0x"
                           << (index_zero_address + final_array_offset + sampler_descriptor_size) << ")";
                        warn_index_oob(max_index);
                    } else if (is_runtime_array) {
                        const VkDeviceSize final_array_offset = max_index * map_data.samplerHeapArrayStride;
                        ss << new_line << "    The final descriptor index in bounds is [" << std::dec << max_index << std::hex
                           << "] which would be accessed at [0x" << (index_zero_address + final_array_offset) << ", 0x"
                           << (index_zero_address + final_array_offset + sampler_descriptor_size) << ")";
                    }

                    if (!heap.sampler_reserved.empty() && warn_reserved_range_start == vvl::kNoIndex32) {
                        const uint32_t max_search_index = array_length != 0 ? array_length : max_index + 1;
                        for (uint32_t i = 0; i < max_search_index; i++) {
                            VkDeviceAddress next_index_address = index_zero_address + (i * map_data.samplerHeapArrayStride);
                            vvl::range<VkDeviceAddress> next_index_range{next_index_address,
                                                                         next_index_address + sampler_descriptor_size};
                            if (next_index_range.intersects(heap.sampler_reserved)) {
                                if (warn_reserved_range_start == vvl::kNoIndex32) {
                                    warn_reserved_range_start = i;
                                }
                                warn_reserved_range_end = i;
                            } else if (warn_reserved_range_end != vvl::kNoIndex32) {
                                break; // found end of reserved range
                            }
                        }
                    }
                }
            } else if (know_ubo) {
                VkDeviceAddress final_offset = map_data.samplerHeapOffset + (indirect_index * map_data.samplerHeapIndexStride);
                VkDeviceAddress index_zero_address = heap.sampler_range.begin + final_offset;
                warn_alignment_sampler(index_zero_address);

                ss << " [final address 0x" << (index_zero_address) << "]";
                warn_oob(final_offset + sampler_descriptor_size, true);

                if (!heap.sampler_range.empty()) {
                    vvl::range<VkDeviceAddress> index_zero_range{index_zero_address, index_zero_address + sampler_descriptor_size};
                    if (index_zero_range.intersects(heap.sampler_range)) {
                        warn_reserved_range_start = 0;
                    }
                }
            }

            warn_oob(map_data.samplerHeapOffset + sampler_descriptor_size, true);
        }
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
        const VkDescriptorMappingSourceIndirectIndexArrayEXT& map_data = mapping.sourceData.indirectIndexArray;
        VkDeviceAddress push_indirect_address =
            map_data.pushOffset < push_data_value.size() ? *((VkDeviceAddress*)&push_data_value[map_data.pushOffset]) : 0xBAD00B;
        VkDeviceAddress final_indirect_address = push_indirect_address + map_data.addressOffset;

        warn_alignment_scalar_indirect(final_indirect_address, 4);
        warn_indirect_uniform_usage(final_indirect_address);

        ss << "pushOffset: 0x" << std::hex << map_data.pushOffset << ", addressOffset: 0x" << map_data.addressOffset
           << ", heapOffset: 0x" << map_data.heapOffset << ", heapIndexStride: 0x" << map_data.heapIndexStride;
        ss << new_line << "indirectAddress: 0x" << final_indirect_address << " (0x" << push_indirect_address << " + 0x"
           << map_data.addressOffset << ")";

        warn_indirect_buffer |= dev_data.ListBuffers(ss, final_indirect_address, 3, true);

        ss << new_line << main_heap_type << " Heap address: 0x" << heap_range.begin + map_data.heapOffset
           << " + (indirectIndex * 0x" << map_data.heapIndexStride << ")";

        if (const vvl::Buffer* buffer_state = GetLargestBuffer(dev_data, final_indirect_address)) {
            uint32_t available_bytes = (uint32_t)(buffer_state->DeviceAddressRange().end - final_indirect_address);
            uint32_t available_slots = available_bytes / sizeof(uint32_t);
            // We can assume this is an array, otherwise, not sure why people are using this mapping
            uint32_t search_slots = !is_array ? 1 : is_runtime_array ? available_slots : array_length;

            if (array_length != 0 && array_length > available_slots) {
                warn_index_oob(available_slots - 1);  // will report warning
                search_slots = available_slots;
            }
            uint32_t search_bytes = search_slots * sizeof(uint32_t);

            std::vector<uint8_t> indirect_index_data = dev_data.CopyDataFromMemory(final_indirect_address, search_bytes);
            bool know_ubo = !indirect_index_data.empty();

            if (is_runtime_array) {
                // Currently don't go searching in, no way to know desired upper bound
                ss << new_line << "    Any descriptor index starting at [" << std::dec << search_slots << std::hex
                   << "] will be invalid as there are no more values found for indirectIndex inside "
                   << dev_data.FormatHandle(buffer_state->Handle());
            }

            if (know_ubo) {
                uint32_t* indirect_index_words = (uint32_t*)indirect_index_data.data();
                if (!is_array) {
                    uint32_t indirect_index = indirect_index_words[0];
                    VkDeviceSize final_offset = map_data.heapOffset + (indirect_index * map_data.heapIndexStride);
                    VkDeviceAddress final_address = heap_range.begin + final_offset;
                    warn_alignment_descriptor(final_address);

                    ss << " [final address 0x" << final_address << " where indirectIndex is 0x" << indirect_index << "]";
                    warn_oob(final_offset + descriptor_size, false);
                } else if (!is_runtime_array) {
                    // Runtime arrays are unbounded and not idea where to stop looking,
                    // can add if people find valuable.
                    ss << new_line << "indirectIndex values from buffer: [";
                    std::vector<uint32_t> bad_array_indexes;
                    std::vector<uint32_t> bad_alignment_indexes;
                    std::vector<uint32_t> bad_reserve_indexes;
                    for (uint32_t i = 0; i < search_slots; i++) {
                        const uint32_t current_index_value = indirect_index_words[i];
                        VkDeviceSize final_offset = map_data.heapOffset + (current_index_value * map_data.heapIndexStride);
                        if (final_offset + descriptor_size > heap_range.size()) {
                            bad_array_indexes.emplace_back(i);
                        }
                        if (i != 0) ss << ", ";
                        ss << "0x" << current_index_value;

                        VkDeviceAddress next_index_address = heap_range.begin + final_offset;
                        if (!IsPointerAligned(next_index_address, required_alignment)) {
                            bad_alignment_indexes.emplace_back(i);
                        }

                        if (!heap_reserved.empty()) {
                            vvl::range<VkDeviceAddress> next_index_range{next_index_address, next_index_address + descriptor_size};
                            if (next_index_range.intersects(heap_reserved)) {
                                bad_reserve_indexes.emplace_back(i);
                            }
                        }
                    }
                    warn_index_array(bad_array_indexes);
                    warn_alignment_index_array(bad_alignment_indexes);
                    warn_reserved_range_index_array(bad_reserve_indexes);
                    ss << "]";
                }
            }
        } else {
            assert(warn_indirect_buffer);
        }
        warn_oob(map_data.heapOffset + descriptor_size, false);

        dump_sampler &= map_data.pEmbeddedSampler == nullptr;
        if (dump_sampler) {
            push_indirect_address = map_data.samplerPushOffset < push_data_value.size()
                                        ? *((VkDeviceAddress*)&push_data_value[map_data.samplerPushOffset])
                                        : 0xBAD00B;
            final_indirect_address = push_indirect_address + map_data.samplerAddressOffset;

            warn_alignment_scalar_indirect(final_indirect_address, 4);
            warn_indirect_uniform_usage(final_indirect_address);

            ss << new_bullet_line << "samplerPushOffset: 0x" << std::hex << map_data.samplerPushOffset
               << ", samplerAddressOffset: 0x" << map_data.samplerAddressOffset << ", samplerHeapOffset: 0x"
               << map_data.samplerHeapOffset << ", samplerHeapIndexStride: 0x" << map_data.samplerHeapIndexStride;
            ss << new_line << "indirectAddress: 0x" << final_indirect_address << " (0x" << push_indirect_address << " + 0x"
               << map_data.samplerAddressOffset << ")";

            warn_indirect_buffer |= dev_data.ListBuffers(ss, final_indirect_address, 3, true);

            ss << new_line << "Sampler Heap address: 0x" << heap.sampler_range.begin + map_data.samplerHeapOffset
               << " + (indirectIndex * 0x" << map_data.samplerHeapIndexStride << ")";

            if (const vvl::Buffer* buffer_state = GetLargestBuffer(dev_data, final_indirect_address)) {
                uint32_t available_bytes = (uint32_t)(buffer_state->DeviceAddressRange().end - final_indirect_address);
                uint32_t available_slots = available_bytes / sizeof(uint32_t);
                uint32_t search_slots = !is_array ? 1 : is_runtime_array ? available_slots : array_length;

                if (array_length != 0 && array_length > available_slots) {
                    warn_index_oob(available_slots - 1);  // will report warning
                    search_slots = available_slots;
                }
                uint32_t search_bytes = search_slots * sizeof(uint32_t);

                std::vector<uint8_t> indirect_index_data = dev_data.CopyDataFromMemory(final_indirect_address, search_bytes);
                bool know_ubo = !indirect_index_data.empty();

                if (is_runtime_array) {
                    ss << new_line << "    Any descriptor index starting at [" << std::dec << search_slots << std::hex
                       << "] will be invalid as there are no more values found for indirectIndex inside "
                       << dev_data.FormatHandle(buffer_state->Handle());
                }

                if (know_ubo) {
                    uint32_t* indirect_index_words = (uint32_t*)indirect_index_data.data();
                    if (!is_array) {
                        uint32_t indirect_index = indirect_index_words[0];
                        VkDeviceSize final_offset = map_data.samplerHeapOffset + (indirect_index * map_data.samplerHeapIndexStride);
                        VkDeviceAddress final_address = heap.sampler_range.begin + final_offset;
                        warn_alignment_sampler(final_address);

                        ss << " [final address 0x" << final_address << "]";
                        warn_oob(final_offset + sampler_descriptor_size, true);
                    } else if (!is_runtime_array) {
                        ss << new_line << "indirectIndex values from buffer: [";
                        std::vector<uint32_t> bad_array_indexes;
                        std::vector<uint32_t> bad_alignment_indexes;
                        std::vector<uint32_t> bad_reserve_indexes;
                        for (uint32_t i = 0; i < search_slots; i++) {
                            const uint32_t current_index_value = indirect_index_words[i];
                            VkDeviceSize final_offset =
                                map_data.samplerHeapOffset + (current_index_value * map_data.samplerHeapIndexStride);
                            if (final_offset + sampler_descriptor_size > heap.sampler_range.size()) {
                                bad_array_indexes.emplace_back(i);
                            }
                            if (i != 0) ss << ", ";
                            ss << "0x" << current_index_value;

                            VkDeviceAddress next_index_address = heap.sampler_range.begin + final_offset;
                            if (!IsPointerAligned(next_index_address,
                                                  dev_data.phys_dev_ext_props.descriptor_heap_props.samplerDescriptorAlignment)) {
                                bad_alignment_indexes.emplace_back(i);
                            }

                            if (!heap.sampler_reserved.empty()) {
                                vvl::range<VkDeviceAddress> next_index_range{next_index_address,
                                                                             next_index_address + sampler_descriptor_size};
                                if (next_index_range.intersects(heap.sampler_reserved)) {
                                    bad_reserve_indexes.emplace_back(i);
                                }
                            }
                        }
                        warn_index_array(bad_array_indexes);
                        warn_alignment_index_array_sampler(bad_alignment_indexes);
                        warn_reserved_range_index_array(bad_reserve_indexes);
                        ss << "]";
                    }
                }
            } else {
                assert(warn_indirect_buffer);
            }
            warn_oob(map_data.samplerHeapOffset + sampler_descriptor_size, true);
        }
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT) {
        const VkDescriptorMappingSourceHeapDataEXT& map_data = mapping.sourceData.heapData;
        uint32_t push_data =
            map_data.pushOffset < push_data_value.size() ? *((uint32_t*)&push_data_value[map_data.pushOffset]) : 0xBAD00B;

        ss << "pushOffset: 0x" << std::hex << map_data.pushOffset << ", heapOffset: 0x" << map_data.heapOffset;
        ss << new_line << "Push data at 0x" << std::hex << map_data.pushOffset << ": 0x" << push_data;
        ss << new_line << main_heap_type << " Heap address: 0x" << heap_range.begin + map_data.heapOffset << " + 0x"
           << push_data;
        VkDeviceAddress final_offset = map_data.heapOffset + push_data;
        VkDeviceAddress final_address = heap_range.begin + final_offset;
        ss << " [final address 0x" << final_address << "]";

        warn_oob(final_offset + descriptor_size, false);

        if (!heap_reserved.empty()) {
            vvl::range<VkDeviceAddress> index_zero_range{final_address, final_address + descriptor_size};
            if (index_zero_range.intersects(heap_reserved)) {
                warn_reserved_range_start = 0;
            }
        }

    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT) {
        ss << "pushDataOffset: 0x" << std::hex << mapping.sourceData.pushDataOffset;
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT) {
        uint64_t indirect_address = mapping.sourceData.pushAddressOffset < push_data_value.size()
                                        ? *((uint64_t*)&push_data_value[mapping.sourceData.pushAddressOffset])
                                        : 0xBAD00B;

        ss << "pushAddressOffset: 0x" << std::hex << mapping.sourceData.pushAddressOffset;
        ss << new_line << "Indirect Adresss 0x" << indirect_address;

        warn_alignment_indirect_address(indirect_address);
        warn_resource_buffer_usage(indirect_address);
        warn_indirect_buffer |= dev_data.ListBuffers(ss, indirect_address, 3, true);

    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT) {
        const VkDescriptorMappingSourceIndirectAddressEXT& map_data = mapping.sourceData.indirectAddress;
        VkDeviceAddress push_indirect_address =
            map_data.pushOffset < push_data_value.size() ? *((VkDeviceAddress*)&push_data_value[map_data.pushOffset]) : 0xBAD00B;

        VkDeviceAddress final_indirect_address = push_indirect_address + map_data.addressOffset;

        ss << "pushOffset: 0x" << std::hex << map_data.pushOffset << ", addressOffset: 0x" << map_data.addressOffset;
        ss << new_line << "Indirect Address: 0x" << final_indirect_address << " (0x" << push_indirect_address << " + 0x"
           << map_data.addressOffset << ")";

        warn_alignment_scalar_indirect(final_indirect_address, 8);
        warn_indirect_uniform_usage(final_indirect_address);
        warn_indirect_buffer |= dev_data.ListBuffers(ss, final_indirect_address, 3, true);

        std::vector<uint8_t> indirect_address_data = dev_data.CopyDataFromMemory(final_indirect_address, 8);
        if (!indirect_address_data.empty()) {
            const VkDeviceAddress resource_address = *((VkDeviceAddress*)indirect_address_data.data());
            ss << new_line << "Resource Adresss 0x" << resource_address;

            warn_alignment_indirect_address(resource_address, true);
            warn_resource_buffer_usage(resource_address);
            warn_indirect_buffer |= dev_data.ListBuffers(ss, resource_address, 3, true);
        }
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_SHADER_RECORD_INDEX_EXT) {
        // TODO - Add address for RTX
        const VkDescriptorMappingSourceShaderRecordIndexEXT& map_data = mapping.sourceData.shaderRecordIndex;
        ss << "heapOffset: 0x" << std::hex << map_data.heapOffset << ", shaderRecordOffset: 0x" << map_data.shaderRecordOffset
           << ", heapIndexStride: 0x" << map_data.heapIndexStride << ", heapArrayStride: 0x" << map_data.heapArrayStride;

        dump_sampler &= map_data.pEmbeddedSampler == nullptr;
        if (dump_sampler) {
            ss << new_bullet_line << "samplerHeapOffset: 0x" << std::hex << map_data.samplerHeapOffset
               << ", samplerShaderRecordOffset: 0x" << map_data.samplerShaderRecordOffset << ", samplerHeapIndexStride: 0x"
               << map_data.samplerHeapIndexStride << ", samplerHeapArrayStride: 0x" << map_data.samplerHeapArrayStride;
        }
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_DATA_EXT) {
        // TODO - Add more info probably
        ss << "shaderRecordDataOffset: " << std::hex << mapping.sourceData.shaderRecordDataOffset;
    } else if (mapping.source == VK_DESCRIPTOR_MAPPING_SOURCE_SHADER_RECORD_ADDRESS_EXT) {
        // TODO - Add more info probably
        ss << "shaderRecordAddressOffset: " << std::hex << mapping.sourceData.shaderRecordAddressOffset;
    }

    if (descriptor_type != VK_DESCRIPTOR_TYPE_MAX_ENUM) {
        if (dump_sampler) {
            ss << new_bullet_line << "Descriptor size: 0x" << std::hex << descriptor_size << " (imageDescriptorSize) and 0x"
               << sampler_descriptor_size << " (samplerDescriptorAlignment)";
        } else {
            ss << new_bullet_line << "Descriptor size: 0x" << std::hex << descriptor_size << " ("
               << string_VkDescriptorType(descriptor_type) << ")";
        }
    }

    if (warn_reserved_range_start != vvl::kNoIndex32) {
        if (warn_reserved_range_start == warn_reserved_range_end) {
            warn_ss << new_line << "[WARNING] RESERVED RANGE - descriptor index at [" << std::dec << warn_reserved_range_start
                    << std::hex << "] will overlap with the reserved range and the access will be invalid";
        } else if (warn_reserved_range_end != vvl::kNoIndex32) {
            warn_ss << new_line << "[WARNING] RESERVED RANGE - descriptor index starting at [" << std::dec
                    << warn_reserved_range_start << "] to [" << warn_reserved_range_end << std::hex
                    << "] will overlap with the reserved range and the access will be invalid";
        } else {
            warn_ss << new_line
                    << "[WARNING] RESERVED RANGE - this descriptor overlaps with the reserved range and any access will be invalid";
        }
    }

    bool found_warning = warn_indirect_buffer;
    if (!warn_ss.str().empty()) {
        ss << warn_ss.str();
        found_warning = true;
    }

    ss << '\n';

    return found_warning;
}

static uint32_t GetUntypedSize(const spirv::Module& module, const spirv::Instruction& size_inst, VkDeviceSize descriptor_size) {
    if (size_inst.Opcode() == spv::OpConstantSizeOfEXT) {
        const spirv::Instruction* constant_type_inst = module.FindDef(size_inst.Word(3));
        if (!IsValueIn((spv::Op)constant_type_inst->Opcode(), {spv::OpTypeBufferEXT, spv::OpTypeImage, spv::OpTypeSampler})) {
            assert(false);  // assumptions
            return 0;
        }
        return (uint32_t)descriptor_size;
    } else if (size_inst.Opcode() == spv::OpConstant) {
        return size_inst.GetConstantValue();
    } else if (size_inst.Opcode() == spv::OpSpecConstantOp) {
        // TODO - We will need some sort of internal constant folding here... fun!
        return 0;
    }
    assert(false);  // assumptions
    return 0;
}

static uint32_t GetUntypedHeapOffset(const spirv::Module& module, const spirv::Instruction& access_chain,
                                     const spirv::Instruction& base_type_inst, VkDeviceSize descriptor_size) {
    if (base_type_inst.Opcode() != spv::OpTypeStruct) {
        return 0;
    }

    const uint32_t struct_id = base_type_inst.ResultId();
    for (const auto& type_struct : module.static_data_.type_structs) {
        if (type_struct->id == struct_id) {
            uint32_t struct_member = module.FindDef(access_chain.Word(5))->GetConstantValue();
            const auto& member = type_struct->members[struct_member];
            if (member.decorations->offset != vvl::kNoIndex32) {
                return member.decorations->offset;
            }
            const spirv::Instruction* offset_id_inst = module.FindDef(member.decorations->offset_id);
            return GetUntypedSize(module, *offset_id_inst, descriptor_size);
        }
    }
    assert(false);
    return 0;
}

static uint32_t GetUntypedArrayStride(const spirv::Module& module, const spirv::Instruction& access_chain,
                                      const spirv::Instruction& base_type_inst, VkDeviceSize descriptor_size) {
    uint32_t type_array_id = vvl::kNoIndex32;
    if (base_type_inst.Opcode() == spv::OpTypeBufferEXT) {
        return 0;  // single element
    } else if (base_type_inst.Opcode() == spv::OpTypeStruct) {
        // required to be a constant into the struct
        uint32_t struct_member = module.FindDef(access_chain.Word(5))->GetConstantValue();
        type_array_id = base_type_inst.Word(2 + struct_member);
    } else if (base_type_inst.IsArray()) {
        type_array_id = base_type_inst.ResultId();
    }
    const auto decoration_set = module.GetDecorationSet(type_array_id);
    if (decoration_set.array_stride_id == spirv::kInvalidValue) {
        return 0; // struct with no array in it
    }

    const spirv::Instruction* array_stride_inst = module.FindDef(decoration_set.array_stride_id);
    return GetUntypedSize(module, *array_stride_inst, descriptor_size);
}

static uint32_t GetUntypedDescriptorIndex(const spirv::Module& module, const spirv::Instruction& access_chain,
                                          const spirv::Instruction& base_type_inst) {
    const spirv::Instruction* index_inst = nullptr;
    if (base_type_inst.Opcode() == spv::OpTypeBufferEXT) {
        return 0;  // single element
    } else if (base_type_inst.IsArray()) {
        if (access_chain.Length() < 6) {
            return 0;  // implicit zero index
        } else {
            index_inst = module.FindDef(access_chain.Word(5));
        }
    } else if (base_type_inst.Opcode() == spv::OpTypeStruct) {
        if (access_chain.Length() < 7) {
            return 0;  // implicit zero index
        } else {
            index_inst = module.FindDef(access_chain.Word(6));
        }
    }

    uint32_t descriptor_index = vvl::kNoIndex32;
    if (index_inst) {
        module.GetInt32IfConstant(*index_inst, &descriptor_index);
    }
    return descriptor_index;
}

static uint32_t GetUntypedVariableId(const spirv::Module& module, const spirv::Instruction& access_chain) {
    assert(access_chain.Opcode() == spv::OpUntypedAccessChainKHR);

    const spirv::Instruction* base_inst = module.FindDef(access_chain.Word(4));
    if (base_inst->Opcode() != spv::OpUntypedVariableKHR) {
        assert(false);  // assumptions
        return vvl::kNoIndex32;
    }
    return base_inst->ResultId();
}

// "Friends to let friends become compiler engineers" ~Spencer
vvl::unordered_map<uint32_t, HeapAccesses> CommandBufferSubState::DumpDescriptorHeapUntypedFindAccess(
    const spirv::Module& module, const spirv::EntryPoint& entrypoint) const {
    vvl::unordered_map<uint32_t, HeapAccesses> accesses;

    auto add_access = [&](const VkDescriptorType descriptor_type, const spirv::Instruction& ac_inst) {
        const VkDeviceSize descriptor_size = dev_data.device_state->cached_descriptor_size.GetSize(descriptor_type);
        assert(ac_inst.Opcode() == spv::OpUntypedAccessChainKHR);
        // Totally asserts a 1D array access here
        // https://gitlab.khronos.org/spirv/SPIR-V/-/issues/942
        const spirv::Instruction* base_type_inst = module.FindDef(ac_inst.Word(3));
        if (base_type_inst->Opcode() != spv::OpTypeBufferEXT && base_type_inst->Opcode() != spv::OpTypeStruct &&
            !base_type_inst->IsArray()) {
            assert(false);
            return;
        }

        const uint32_t heap_offset = GetUntypedHeapOffset(module, ac_inst, *base_type_inst, descriptor_size);

        const uint32_t array_stride_value = GetUntypedArrayStride(module, ac_inst, *base_type_inst, descriptor_size);

        const uint32_t descriptor_index = GetUntypedDescriptorIndex(module, ac_inst, *base_type_inst);

        const uint32_t variable_id = GetUntypedVariableId(module, ac_inst);
        if (variable_id == vvl::kNoIndex32) {
            return;
        }

        accesses[variable_id].insert(HeapAccess{descriptor_type, heap_offset, array_stride_value, descriptor_index});
    };

    for (const spirv::Instruction* memory_access_inst : entrypoint.accessible.memory_accesses) {
        // Basically there are 2 flows, images and non-images
        uint32_t ptr_id = OpcodeImageAccessPosition(memory_access_inst->Opcode());
        const bool image_access = ptr_id != 0;

        if (image_access) {
            const spirv::Instruction* sampler_load_inst = nullptr;

            const spirv::Instruction* next_inst = module.FindDef(memory_access_inst->Word(ptr_id));
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
                continue;
            }

            // From here two types of images, sampled and non-sampled images
            if (sampler_load_inst) {
                // TODO - Assertion sampled images are 1D
                const spirv::Instruction* ac_inst = module.FindDef(image_load_inst->Word(3));
                if (ac_inst->Opcode() != spv::OpUntypedAccessChainKHR) {
                    assert(false);  // assumptions
                    continue;
                }
                add_access(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, *ac_inst);

                ac_inst = module.FindDef(sampler_load_inst->Word(3));
                if (ac_inst->Opcode() != spv::OpUntypedAccessChainKHR) {
                    assert(false);  // assumptions
                    continue;
                }
                add_access(VK_DESCRIPTOR_TYPE_SAMPLER, *ac_inst);
            } else {
                const spirv::Instruction* image_type = module.FindDef(image_load_inst->TypeId());
                assert(image_type->Opcode() == spv::OpTypeImage);
                const VkDescriptorType image_descriptor_type = image_type->GetImageType();

                const spirv::Instruction* ac_inst = module.FindDef(image_load_inst->Word(3));
                if (ac_inst->Opcode() != spv::OpUntypedAccessChainKHR) {
                    assert(false);  // assumptions
                    continue;
                }
                add_access(image_descriptor_type, *ac_inst);
            }
        } else {
            // |Operand 0| works for both Store/Load
            ptr_id = memory_access_inst->Operand(0);

            // We need to walk down possibly multiple chained OpAccessChains
            const spirv::Instruction* next_access_chain = module.FindDef(ptr_id);
            while (next_access_chain && next_access_chain->IsNonPtrAccessChain()) {
                const uint32_t base_operand = next_access_chain->IsUntypedAccessChain() ? 1 : 0;
                const uint32_t access_chain_base_id = next_access_chain->Operand(base_operand);
                next_access_chain = module.FindDef(access_chain_base_id);
            }
            const spirv::Instruction* base_type = next_access_chain;

            if (base_type && base_type->Opcode() == spv::OpBufferPointerEXT) {
                const spirv::Instruction* ac_inst = module.FindDef(base_type->Word(3));
                if (ac_inst->Opcode() != spv::OpUntypedAccessChainKHR) {
                    assert(false);  // assumptions
                    continue;
                }

                // Ignore old BufferBlock + Uniform
                const VkDescriptorType descriptor_type =
                    module.FindDef(base_type->TypeId())->StorageClass() == spv::StorageClassUniform
                        ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
                        : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                add_access(descriptor_type, *ac_inst);
            }
        }
    }
    return accesses;
}

bool CommandBufferSubState::DumpDescriptorHeapUntyped(std::ostringstream& ss, const ShaderStageState& stage) const {
    bool found_warning = false;

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

    const spirv::Module& module = *module_state_ptr;
    const spirv::EntryPoint& entrypoint = *entrypoint_ptr;

    // The idea is to go through and find all the access and afterwards sort/group them to make sense to the user
    // < variable id, [accesses]>
    vvl::unordered_map<uint32_t, HeapAccesses> accesses = DumpDescriptorHeapUntypedFindAccess(module, entrypoint);

    // We print out the variables, but the real "value" comes from scaning the accesses
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
        auto heap_range = is_sampler ? base.descriptor_heap.sampler_range : base.descriptor_heap.resource_range;
        auto reserve_range = is_sampler ? base.descriptor_heap.sampler_reserved : base.descriptor_heap.resource_reserved;

        for (const HeapAccess& access : it->second) {
            ss << "    - heap offset: 0x" << std::hex << access.heap_offset;
            if (access.array_stride != 0) {
                ss << ", array stride: 0x" << access.array_stride;
            }

            const bool dynamic_index = access.descriptor_index == vvl::kNoIndex32;
            VkDeviceSize final_address = vvl::kNoIndex32;
            if (!dynamic_index) {
                if (access.descriptor_index != 0) {
                    ss << ", array index[" << std::dec << access.descriptor_index << "]\n";
                } else if (access.array_stride != 0) {
                    ss << ", single element\n";  // if not array stride, this is pointless
                }
                const VkDeviceSize final_offset = access.heap_offset + (access.array_stride * access.descriptor_index);
                final_address = heap_range.begin + final_offset;
                ss << "      - " << (is_sampler ? "Sampler" : "Resource") << " heap address: 0x" << std::hex << final_address
                   << '\n';
            } else {
                ss << " (dynamic index)\n";
            }

            VkDeviceSize descriptor_size = dev_data.device_state->cached_descriptor_size.GetSize(access.descriptor_type);
            ss << "      - descriptor size: 0x" << std::hex << descriptor_size << " ("
               << string_VkDescriptorType(access.descriptor_type) << ")\n";

            // if we know the real address, see if invalid
            if (final_address != vvl::kNoIndex32) {
                if (final_address > heap_range.end) {
                    ss << "      - [WARNING] OUT OF BOUNDS - the descriptor is not in the " << (is_sampler ? "sampler" : "resource")
                       << " heap\n";
                    found_warning = true;
                }
                vvl::range<VkDeviceAddress> final_range{final_address, final_address + descriptor_size};
                if (final_range.intersects(reserve_range)) {
                    ss << "      - [WARNING] RESERVED RANGE - this descriptor overlaps with the reserved range\n";
                    found_warning = true;
                }

                VkDeviceSize required_alignment = GetDescriptorAlignment(access.descriptor_type, dev_data.phys_dev_ext_props);
                if (!IsPointerAligned(final_address, required_alignment)) {
                    vvl::Field alignment_name = GetDescriptorAlignmentField(access.descriptor_type);
                    ss << "      - [WARNING] MISALIGNED - the descriptor is not aligned to " << String(alignment_name);
                    ss << " (0x" << std::hex << required_alignment << ")\n";
                    found_warning = true;
                }
            }
        }
    }

    return found_warning;
}

bool CommandBufferSubState::DumpDescriptorHeap(std::ostringstream& ss, const LastBound& last_bound) const {
    bool found_warning = false;
    const vvl::CommandBuffer& cb_state = last_bound.cb_state;
    if (!cb_state.descriptor_heap.resource_range.empty()) {
        ss << "- vkCmdBindResourceHeapEXT last bound the resource heap to "
           << string_range_hex(cb_state.descriptor_heap.resource_range);
        if (!cb_state.descriptor_heap.resource_reserved.empty()) {
            ss << " (reserved range " << std::dec << cb_state.descriptor_heap.resource_reserved.size() << " bytes at " << string_range_hex(cb_state.descriptor_heap.resource_reserved) << ")";
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
            ss << " (reserved range " << std::dec << cb_state.descriptor_heap.sampler_reserved.size() << " bytes at "
               << string_range_hex(cb_state.descriptor_heap.sampler_reserved) << ")";
        } else {
            ss << " (no reserved range)";
        }
        ss << '\n';
        found_warning |= dev_data.ListBuffers(ss, cb_state.descriptor_heap.sampler_range.begin, 1);
    }

    if (last_bound.pipeline_state) {
        ss << "- Last bound pipeline: " << dev_data.FormatHandle(last_bound.pipeline_state->VkHandle()) << " ("
           << string_VkPipelineBindPoint(last_bound.pipeline_state->pipeline_type) << ")\n";
    }

    ss << "- Shader descriptors:\n";
    small_vector<const ShaderStageState*, 3> stages = last_bound.GetStages();
    for (const ShaderStageState* stage : stages) {
        if (!stage->HasSpirv()) {
            ss << "[No SPIR-V found for " << string_VkShaderStageFlagBits(stage->GetStage())
               << ", can't detect which descriptors are being accessed]\n";
            continue;
        }
        const spirv::EntryPoint& entry_point = *stage->entrypoint;
        ss << "  " << entry_point.Describe();
        // TODO - add util in ShaderStageState to get ShaderObject handle here
        if (stage->module_state && stage->module_state->VkHandle() != VK_NULL_HANDLE) {
            ss << " " << dev_data.FormatHandle(stage->module_state->VkHandle());
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
            const uint32_t var_set = resource_variable.decorations.set;

            for (uint32_t i = 0; i < mapping_info->mappingCount; i++) {
                const VkDescriptorSetAndBindingMappingEXT& mapping = mapping_info->pMappings[i];
                if (!IsResourceVaribleInMapping(mapping, resource_variable)) {
                    continue;
                }
                mapping_info_map[var_set].emplace_back(MappingInfo{&mapping, &resource_variable, i});
            }
        }

        if (mapping_info_map.empty() && has_non_heap) {
            ss << "    - No VkDescriptorSetAndBindingMappingEXT were found for this shader\n";
            continue;
        }

        for (auto& [set_index, mapping_info_list] : mapping_info_map) {
            ss << "  - SPIR-V Set " << std::dec << set_index << ":\n";
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
