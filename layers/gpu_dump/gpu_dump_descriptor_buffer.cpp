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

#include "containers/custom_containers.h"
#include "generated/error_location_helper.h"
#include "gpu_dump_state.h"
#include "gpu_dump.h"
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include "containers/range.h"
#include "containers/small_vector.h"
#include "containers/limits.h"
#include "generated/dispatch_functions.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/descriptor_hashing.h"
#include "state_tracker/pipeline_layout_state.h"
#include "state_tracker/pipeline_state.h"
#include "state_tracker/shader_module.h"
#include "state_tracker/shader_stage_state.h"
#include "state_tracker/state_tracker.h"
#include "utils/descriptor_utils.h"

namespace gpudump {

struct BindingInfo {
    uint32_t index;
    VkDescriptorType type;
    bool embedded;
    uint32_t count;
    uint32_t size;
    vvl::Field size_field;
    VkDeviceSize offset;
    vvl::range<VkDeviceAddress> range;
    std::string variable_name;

    // We want to sort the bindings we print by their address
    bool operator<(const BindingInfo& other) const { return range.begin < other.range.begin; }

    void Print(std::ostringstream& ss) {
        ss << "    - SPIR-V Binding " << std::dec << index << " [\"" << variable_name << "\"]\n";
        if (!embedded) {
            ss << "      - " << string_VkDescriptorType(type) << "\n";
            ss << "      - offset: " << std::dec << offset << ", range: " << string_range_hex(range) << "\n";
            ss << "      - size: " << String(size_field) << " [" << std::dec << size << "]";
            if (count > 1) {
                ss << " * descriptorCount [" << std::dec << count << "]";
            }
            ss << "\n";
        }
    }

    bool ValidateDescriptor(std::ostringstream& ss, GpuDump& dev_data);
};

struct SetInfo {
    bool embedded;
    uint32_t index;          // set of the descriptor
    uint32_t binding_index;  // into pBindingInfos[]
    VkDeviceSize binding_offset;  // pBindingInfos[binding_index].offset
    VkBufferUsageFlagBits2 buffer_usage_flags;
    vvl::range<VkDeviceAddress> range;
    const vvl::DescriptorSetLayout* dsl;
    std::vector<BindingInfo> bindings{};

    // We want to sort the sets we print by their address
    bool operator<(const SetInfo& other) const { return range.begin < other.range.begin; }

    void Print(std::ostringstream& ss) {
        ss << "  - SPIR-V Set " << std::dec << index << "\n";
        if (embedded) {
            ss << "    - Embedded Sampler";
        } else {
            ss << "    - offset: " << std::dec << binding_offset << ", size: " << range.size()
               << " bytes, range: " << string_range_hex(range);
        }
        if (binding_index != vvl::kNoIndex32) {
            // Only print if there are multiple bindings
            ss << " (specified in pBindingInfos[" << std::dec << binding_index << "])";
        }
        ss << '\n';
    }

    bool ValidateBufferUsage(std::ostringstream& ss, const BindingInfo& binding_info);
};

bool SetInfo::ValidateBufferUsage(std::ostringstream& ss, const BindingInfo& binding_info) {
    const bool resource_buffer = (buffer_usage_flags & VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT) != 0;
    const bool sampler_buffer = (buffer_usage_flags & VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT) != 0;
    if (!resource_buffer && !sampler_buffer) {
        return false;  // caught elsewhere
    }

    if (binding_info.type == VK_DESCRIPTOR_TYPE_SAMPLER || binding_info.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        if (!sampler_buffer) {
            ss << "      - [WARNING] BUFFER USAGE - Using a sampler but buffer not created with "
                  "VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT\n";
            return true;
        }
    } else if (!resource_buffer) {
        ss << "      - [WARNING] BUFFER USAGE - Using a resource but buffer not created with "
              "VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT\n";
        return true;
    }
    return false;
}

bool BindingInfo::ValidateDescriptor(std::ostringstream& ss, GpuDump& dev_data) {
    if (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        return false;  // TODO - handle
    }

    const auto& descriptor_hashing = *dev_data.device_state->descriptor_hashing;

    ReadLockGuard guard(descriptor_hashing.map_lock);
    if (descriptor_hashing.table.limit_reported) {
        // If hit limit, likely to detect false positives
        return false;
    }

    // For VK_EXT_descriptor_buffer where each descriptor type might be a different size.
    //
    // For example, if a UBO is 32 bytes, but an image is 64 bytes:
    // If we grab the heap data from [N, N+31] and hash it, we have no way to know if it would have
    // really been an Image hash if we have done [N, N+63]
    //
    // We need a way to re-hash to find the key for a potential different type
    bool check_larger_sizes = true;
    std::vector<uint8_t> descriptor_bytes = dev_data.CopyDataFromMemory(range.begin, descriptor_hashing.buffer_max_descriptor_size);
    if (descriptor_bytes.empty()) {
        descriptor_bytes = dev_data.CopyDataFromMemory(range.begin, size);
        if (descriptor_bytes.empty()) {
            return false;  // heap buffer might not be visible
        } else {
            // this means we are at the end of the heap buffer and can only check the exact size (or smaller)
            check_larger_sizes = false;
        }
    }

    uint64_t key = descriptor_hashing.Hash(descriptor_bytes.data(), size);
    const vvl::DescriptorHashTable::Entry* entry = descriptor_hashing.table.Find(key);
    if (!entry) {
        // Before saying we can't find it, check the other descriptor sizes if we can spot the wrong descriptor type
        for (VkDeviceSize other_size : descriptor_hashing.buffer_all_descriptor_sizes) {
            if (other_size == size) {
                continue;
            } else if (!check_larger_sizes && other_size > size) {
                continue;
            }
            key = descriptor_hashing.Hash(descriptor_bytes.data(), other_size);
            entry = descriptor_hashing.table.Find(key);
            if (entry) {
                break;
            }
        }
        if (!entry) {
            ss << "      - [WARNING] NO DESCRIPTOR - No known descriptor found in bound descriptor buffer\n";
            return true;
        }
    }

    if (entry->IsNullDescriptor()) {
        ss << "      - ";
        if (!descriptor_hashing.null_descriptor_allowed) {
            ss << "[WARNING] NO DESCRIPTOR - ";
        }
        ss << "Found descriptor mapped to [Null Descriptor]";
        if (!descriptor_hashing.null_descriptor_allowed) {
            ss << " but the nullDescriptor feature was not enabled";
        }
        ss << "\n";
        return !descriptor_hashing.null_descriptor_allowed;
    } else if (!entry->HasType(type)) {
        ss << "      - [WARNING] WRONG DESCRIPTOR - expected a " << string_VkDescriptorType(type)
           << " descriptor, but instead found a descriptor for: " << descriptor_hashing.Describe(*dev_data.device_state, key)
           << "\n";
        return true;
    }

    ss << "      - Found descriptor mapped to: " << descriptor_hashing.Describe(*dev_data.device_state, key) << "\n";
    return false;
}

// Return true if found warning
bool CommandBufferSubState::DumpDescriptorBuffer(std::ostringstream& ss, const LastBound& last_bound) const {
    const vvl::CommandBuffer& cb_state = last_bound.cb_state;
    bool found_warning = false;
    ss << "- vkCmdBindDescriptorBuffersEXT last bound the following descriptor buffers:\n";

    vvl::unordered_map<uint32_t, VkBufferUsageFlagBits2> binding_usage_flags;
    uint32_t binding_info_count = (uint32_t)cb_state.descriptor_buffer.binding_info.size();
    // Can be zero if using only push constants
    if (dev_data.enabled[gpu_validation] && dev_data.gpuav_settings.IsShaderInstrumentationEnabled() && binding_info_count != 0) {
        // We warn the users to not have both enabled, but if this occurs GPU-AV currently adds a buffer
        // but it is not tracked in any state tracking. (nor are the instrumented variables)
        // We just ignore the last binding
        binding_info_count--;
    }

    for (uint32_t binding_i = 0; binding_i < binding_info_count; binding_i++) {
        const VkDeviceAddress address = cb_state.descriptor_buffer.binding_info[binding_i].address;

        VkBufferUsageFlagBits2 usage_flags = 0;
        auto buffer_states = dev_data.GetBuffersByAddress(address);
        for (const auto& buffer_state : buffer_states) {
            usage_flags |= buffer_state->usage;
        }
        binding_usage_flags[binding_i] = usage_flags;
        const bool resource_buffer = (usage_flags & VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT) != 0;
        const bool sampler_buffer = (usage_flags & VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT) != 0;

        ss << "  - pBindingInfos[" << std::dec << binding_i << "].address 0x" << std::hex << address << ' ';
        if (resource_buffer && sampler_buffer) {
            ss << "(resource and sampler buffer)";
        } else if (resource_buffer) {
            ss << "(resource buffer)";
        } else if (sampler_buffer) {
            ss << "(sampler buffer)";
        } else {
            ss << "\n  - [WARNING] missing VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT or "
                  "VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT";
        }
        ss << '\n';

        found_warning |= dev_data.ListBuffers(ss, address, 1);
    }

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
                if (var_set >= last_bound.ds_slots.size()) {
                    ss << "  - [WARNING] SPIRV-Set " << std::dec << var_set
                       << " is not a valid bound set as vkCmdSetDescriptorBufferOffsetsEXT was either not called or invalidated "
                          "for "
                       << string_VkPipelineBindPoint(last_bound.bind_point) << '\n';
                    found_warning = true;
                    continue;
                }
                const auto& descriptor_buffer_binding = last_bound.ds_slots[var_set].descriptor_buffer_binding;

                // Will print the invalid/unknown sets first, no need to sort these
                if (!dsl || dsl->Destroyed()) {
                    ss << "  - [WARNING] SPIR-V Set " << std::dec << var_set
                       << " VkDescriptorSetLayout was destroyed (TODO - Track more info in pipeline layout)\n";
                    found_warning = true;
                    continue;
                } else if ((dsl->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) == 0) {
                    ss << "  - [WARNING] SPIR-V Set " << std::dec << var_set
                       << " was not created with VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT\n";
                    found_warning = true;
                    continue;
                } else if ((dsl->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT) != 0) {
                    ss << "  - SPIR-V Set " << std::dec << var_set
                       << " was created with VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT\n";
                    continue;
                } else if ((dsl->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT) != 0) {
                    ss << "  - SPIR-V Set " << std::dec << var_set
                       << " was created with VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT\n";
                    continue;
                } else if (!descriptor_buffer_binding.has_value()) {
                    ss << "  - [WARNING] SPIR-V Set " << std::dec << var_set
                       << " was never bound with offset. This is only valid if descriptor is not used in the shader";
                    if (dsl->HasImmutableSamplers()) {
                        ss << " or because all bindings are using Immutable Samplers";
                    }
                    ss << "\n";
                    found_warning = true;
                    continue;
                }

                uint32_t binding_index = vvl::kNoIndex32;
                VkDeviceSize binding_offset = descriptor_buffer_binding->offset;
                vvl::range<VkDeviceAddress> set_range;

                const bool embedded = descriptor_buffer_binding->embedded != vvl::kNoIndex32;
                if (!embedded) {
                    const VkDeviceAddress start_address =
                        cb_state.descriptor_buffer.binding_info[descriptor_buffer_binding->index].address + binding_offset;
                    const VkDeviceSize dsl_size = dsl->GetLayoutSizeInBytes();
                    set_range = {start_address, start_address + dsl_size};

                    // only care to print index in pBindingInfos
                    if (cb_state.descriptor_buffer.binding_info.size() > 1) {
                        binding_index = descriptor_buffer_binding->index;
                    }
                }

                VkBufferUsageFlagBits2 usage_flags = 0;
                if (binding_usage_flags.find(binding_index) != binding_usage_flags.end()) {
                    usage_flags = binding_usage_flags[binding_index];
                }

                set_info = &sorted_sets.emplace_back(
                    SetInfo{embedded, var_set, binding_index, binding_offset, usage_flags, set_range, dsl, {}});
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

            uint32_t descriptor_size = 0;
            VkDeviceSize binding_offset = 0;
            vvl::range<VkDeviceAddress> binding_range;

            const auto& ds_layout_def = *set_info->dsl->GetLayoutDef();
            const VkDescriptorType type = ds_layout_def.GetTypeFromBinding(var_binding);
            const uint32_t count = ds_layout_def.GetDescriptorCountFromBinding(var_binding);
            vvl::Field size_field = DescriptorBufferSizeField(dev_data.enabled_features.robustBufferAccess, type);

            if (!set_info->embedded) {
                if (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                    descriptor_size =
                        (uint32_t)dev_data.phys_dev_ext_props.descriptor_buffer_props.combinedImageSamplerDescriptorSize;
                } else {
                    descriptor_size = (uint32_t)dev_data.device_state->cached_descriptor_size.GetSize(type, false);
                }
                DispatchGetDescriptorSetLayoutBindingOffsetEXT(dev_data.device, set_info->dsl->VkHandle(), var_binding,
                                                               &binding_offset);
                const VkDeviceAddress binding_start_address = set_info->range.begin + binding_offset;
                binding_range = {binding_start_address, binding_start_address + (descriptor_size * count)};
            }

            set_info->bindings.emplace_back(BindingInfo{var_binding, type, set_info->embedded, count, descriptor_size, size_field,
                                                        binding_offset, binding_range, resource_variable.debug_name});
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
                binding_info.Print(ss);

                found_warning |= set_info.ValidateBufferUsage(ss, binding_info);

                if (dev_data.global_settings.descriptor_hashing && !binding_info.embedded) {
                    found_warning |= binding_info.ValidateDescriptor(ss, dev_data);
                }
            }
        }
    }
    return found_warning;
}
}  // namespace gpudump
