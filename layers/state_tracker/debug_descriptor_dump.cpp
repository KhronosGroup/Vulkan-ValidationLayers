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

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <sstream>
#include "containers/small_vector.h"
#include "generated/dispatch_functions.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/descriptor_mode.h"
#include "state_tracker/pipeline_layout_state.h"
#include "state_tracker/pipeline_state.h"
#include "state_tracker/shader_object_state.h"
#include "state_tracker/shader_stage_state.h"
#include "state_tracker/state_tracker.h"

namespace vvl {

static size_t GetDescriptorBufferSize(const VkPhysicalDeviceDescriptorBufferPropertiesEXT& props, bool robust, VkDescriptorType type) {
    switch(type) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
            return props.samplerDescriptorSize;
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            return props.combinedImageSamplerDescriptorSize;
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            return props.sampledImageDescriptorSize;
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            return props.storageImageDescriptorSize;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            return robust ? props.robustUniformTexelBufferDescriptorSize : props.uniformTexelBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        return robust ? props.robustStorageTexelBufferDescriptorSize : props.storageTexelBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        return robust ? props.robustUniformBufferDescriptorSize : props.uniformBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        return robust ? props.robustStorageBufferDescriptorSize : props.storageBufferDescriptorSize;
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            return props.inputAttachmentDescriptorSize;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            return props.accelerationStructureDescriptorSize;
        default:
        break;
    }
    return 0;
}

static const char* DescribeDescriptorBufferSize(bool robust, VkDescriptorType type) {
    switch(type) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
            return "samplerDescriptorSize";
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            return "combinedImageSamplerDescriptorSize";
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            return "sampledImageDescriptorSize";
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            return "storageImageDescriptorSize";
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            return robust ? "robustUniformTexelBufferDescriptorSize" : "uniformTexelBufferDescriptorSize";
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        return robust ? "robustStorageTexelBufferDescriptorSize" : "storageTexelBufferDescriptorSize";
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        return robust ? "robustUniformBufferDescriptorSize" : "uniformBufferDescriptorSize";
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        return robust ? "robustStorageBufferDescriptorSize" : "storageBufferDescriptorSize";
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            return "inputAttachmentDescriptorSize";
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            return "accelerationStructureDescriptorSize";
        default:
        break;
    }
    return "[Unknown]";
}

// We look at what the SPIR-V is using (from active_slots) and then using the VkDescriptorSetLayouts bound, we can figure out where the shader expects all the addresses to be read from
void CommandBuffer::DebugDumpDescriptors(const LastBound& last_bound, const Location& loc) const {
    DescriptorMode descriptor_mode = last_bound.GetDescriptorMode();
    if (descriptor_mode != DescriptorModeBuffer && descriptor_mode != DescriptorModeHeap) {
        return;
    }
    const vvl::CommandBuffer& cb_state = last_bound.cb_state;

    std::ostringstream ss;
    ss << "(" << dev_data.FormatHandle(cb_state.Handle()) << ")\n";

    if (descriptor_mode == DescriptorModeBuffer) {
        const bool robust_buffer = dev_data.enabled_features.robustBufferAccess;
        ss << "vkCmdBindDescriptorBuffersEXT last bound the following descriptor buffers:\n";
        for (uint32_t binding_i = 0; binding_i < cb_state.descriptor_buffer.binding_info.size(); binding_i++) {
            const VkDeviceAddress address = cb_state.descriptor_buffer.binding_info[binding_i].address;
            ss << "  - pBindingInfos[" << binding_i << "].address 0x" << std::hex << address << '\n';
            auto buffer_states = dev_data.GetBuffersByAddress(address);
            for (auto& buffer_state : buffer_states) {
                ss << "    - " << buffer_state->Describe(dev_data) << "\n";
            }
            if (buffer_states.empty()) {
                ss << "    - No VkBuffer found\n";
            }
        }

        const vvl::PipelineLayout& pipeline_layout = *last_bound.desc_set_pipeline_layout;
        ss << "vkCmdSetDescriptorBufferOffsetsEXT has bound the following with " << dev_data.FormatHandle(pipeline_layout.VkHandle()) << ":\n";

        // Dumb quick way to handle the fact active_slots are per ShaderObject but one single thing for Pipeline
        small_vector<const ActiveSlotMap*, 2> active_slot_list;
        if (last_bound.pipeline_state) {
            active_slot_list.emplace_back(&last_bound.pipeline_state->active_slots);
        } else {
            for (const auto& shader_object : last_bound.shader_object_states) {
                if (!shader_object) {
                    continue;
                }
                active_slot_list.emplace_back(&shader_object->active_slots);
            }
        }

        for (const auto& active_slots : active_slot_list) {
            for (const auto& [set_index, binding_req_map] : *active_slots) {
                const vvl::DescriptorSetLayout& dsl = *pipeline_layout.set_layouts.list[set_index];
                const auto& descriptor_buffer_binding = last_bound.ds_slots[set_index].descriptor_buffer_binding;

                if (dsl.Destroyed()) {
                    ss << "  - Set " << std::dec  << set_index << " VkDescriptorSetLayout was destroyed (TODO - Track more info in pipeline layout)";
                    continue;
                } else if ((dsl.GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) == 0) {
                    ss << "  - Set " << std::dec  << set_index << " was not created with VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT";
                    continue;
                } else if (!descriptor_buffer_binding.has_value()) {
                    ss << "  - Set " << std::dec  << set_index << " was never bound with offset. (WARNING - only valid if descriptor is not used in the shader)\n";
                    continue;
                }

                const VkDeviceAddress start_address = cb_state.descriptor_buffer.binding_info[descriptor_buffer_binding->index].address + descriptor_buffer_binding->offset;
                const VkDeviceSize dsl_size = dsl.GetLayoutSizeInBytes();
                ss << "  - Set " << std::dec  << set_index << " is " << dsl_size << " bytes at [0x"  << std::hex << start_address << ", 0x" << (start_address + dsl_size) << "]";
                if (cb_state.descriptor_buffer.binding_info.size() > 1) {
                    ss << " (pBindingInfos[" << std::dec << descriptor_buffer_binding->index << "])";
                }
                ss << '\n';

                const auto& ds_layout_def = *dsl.GetLayoutDef();
                for (const auto& [binding_index, desc_set_reqs] : binding_req_map) {
                    const VkDescriptorType type = ds_layout_def.GetTypeFromBinding(binding_index);
                    const uint32_t count = ds_layout_def.GetDescriptorCountFromBinding(binding_index);

                    ss << "    - Binding " << binding_index << " (" << string_VkDescriptorType(type) << ") ";
                    const uint32_t descriptor_size = (uint32_t)GetDescriptorBufferSize(dev_data.phys_dev_ext_props.descriptor_buffer_props, robust_buffer, type);
                    VkDeviceSize binding_offset = 0;
                    DispatchGetDescriptorSetLayoutBindingOffsetEXT(dev_data.device, dsl.VkHandle(), binding_index, &binding_offset);
                    ss << "with an offset of " << binding_offset << " is at [0x" << std::hex << start_address + binding_offset << ", 0x" << (start_address + binding_offset + (descriptor_size * count)) << "] (";
                    if (count > 1) {
                        ss << "descriptorCount [" << std::dec << count << "] times ";
                    }
                    ss << DescribeDescriptorBufferSize(robust_buffer, type) << " [" << std::dec << descriptor_size << "])\n";

                }
            }
        }
    } else if (descriptor_mode == DescriptorModeHeap) {
        // TODO
    }

    // Don't provide a LogObjectList, embed it into the messsage instead to keep things cleaner
    dev_data.debug_report->LogMessage(kInformationBit, "DEBUG-DUMP-DESCRIPTOR", {}, loc,
                                      ss.str().c_str());
}

}  // namespace vvl
