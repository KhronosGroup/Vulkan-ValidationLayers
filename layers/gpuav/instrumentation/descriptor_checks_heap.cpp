/* Copyright (c) 2026 LunarG, Inc.
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
#include <cstring>
#include <vulkan/utility/vk_struct_helper.hpp>
#include "containers/limits.h"
#include "drawdispatch/drawdispatch_vuids.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/descriptor_validation/gpuav_descriptor_validation.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/resources/gpuav_shader_resources.h"
#include "gpuav/shaders/gpuav_error_codes.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "gpuav/spirv/instrumentation_status.h"
#include "state_tracker/cmd_buffer_state.h"
#include "error_message/spirv_logging.h"
#include "state_tracker/descriptor_hashing.h"
#include "utils/descriptor_utils.h"
#include "utils/lock_utils.h"

namespace gpuav {

// Way to hold the last snapshot
struct DescriptorChecksHeapCbState {
    vko::BufferRange last_bound_heap_info_resource;
    vko::BufferRange last_bound_heap_info_sampler;
};

// We hope people don't ever use Descriptor Buffer and Heap together or we will have double allocation for no reason
struct DescriptorChecksHeapHashTable {
    DescriptorChecksHeapHashTable(CommandBufferSubState& cb, VkDeviceSize size) {
        buffer_range = cb.gpu_resources_manager.GetDeviceLocalBufferRange(size);
    }
    vko::BufferRange buffer_range{};
};

void RegisterDescriptorChecksHeapValidation(Validator& gpuav, CommandBufferSubState& cb) {
    if (!gpuav.gpuav_settings.shader_instrumentation.descriptor_checks) {
        return;
    }
    if (!gpuav.enabled_features.descriptorHeap) {
        return;
    }

    DescriptorHeapBindings& desc_heap_bindings = cb.shared_resources_cache.GetOrCreate<DescriptorHeapBindings>();

    desc_heap_bindings.on_update_bound_descriptor_heap =
        [](Validator& gpuav, CommandBufferSubState& cb, DescriptorHeapBindings::BindingCommand& desc_binding_cmd, bool is_sampler) {
            DescriptorChecksHeapCbState& dc_cb_state = cb.shared_resources_cache.GetOrCreate<DescriptorChecksHeapCbState>();

            auto& cb_heap = cb.base.descriptor_heap;
            assert(cb_heap.resource_range.size() < (VkDeviceAddress)vvl::kU32Max);
            assert(cb_heap.sampler_range.size() < (VkDeviceAddress)vvl::kU32Max);

            // We only bind one at a time via vkCmdBindResourceHeapEXT/vkCmdBindSamplerHeapEXT
            // but might be binding both with secondary command buffer inheritance
            const bool has_inheritance =
                cb.base.descriptor_heap.is_sampler_inherited || cb.base.descriptor_heap.is_resource_inherited;
            const bool update_sampler = (!has_inheritance && is_sampler) || cb.base.descriptor_heap.is_sampler_inherited;
            const bool update_resource = (!has_inheritance && !is_sampler) || cb.base.descriptor_heap.is_resource_inherited;

            if (update_sampler) {
                dc_cb_state.last_bound_heap_info_sampler =
                    cb.gpu_resources_manager.GetHostCoherentBufferRange(sizeof(glsl::BoundHeapInfo));
                dc_cb_state.last_bound_heap_info_sampler.Clear();
                auto desc_heap_info = static_cast<glsl::BoundHeapInfo*>(dc_cb_state.last_bound_heap_info_sampler.offset_mapped_ptr);

                desc_heap_info->heap_size = (uint32_t)cb_heap.sampler_range.size();
                desc_heap_info->reserved_begin_offset = (uint32_t)(cb_heap.sampler_reserved.begin - cb_heap.sampler_range.begin);
                desc_heap_info->reserved_end_offset = (uint32_t)(cb_heap.sampler_reserved.end - cb_heap.sampler_range.begin);
                desc_heap_info->heap_begin_addr = cb_heap.sampler_range.begin;
            }
            if (update_resource) {
                dc_cb_state.last_bound_heap_info_resource =
                    cb.gpu_resources_manager.GetHostCoherentBufferRange(sizeof(glsl::BoundHeapInfo));
                dc_cb_state.last_bound_heap_info_resource.Clear();
                auto desc_heap_info =
                    static_cast<glsl::BoundHeapInfo*>(dc_cb_state.last_bound_heap_info_resource.offset_mapped_ptr);

                desc_heap_info->heap_size = (uint32_t)cb_heap.resource_range.size();
                desc_heap_info->reserved_begin_offset = (uint32_t)(cb_heap.resource_reserved.begin - cb_heap.resource_range.begin);
                desc_heap_info->reserved_end_offset = (uint32_t)(cb_heap.resource_reserved.end - cb_heap.resource_range.begin);
                desc_heap_info->heap_begin_addr = cb_heap.resource_range.begin;
            }

            // Data for the GPU
            desc_binding_cmd.bound_heap_info_resource = dc_cb_state.last_bound_heap_info_resource;
            desc_binding_cmd.bound_heap_info_sampler = dc_cb_state.last_bound_heap_info_sampler;
            // Data for the CPU (since the command buffer might updated)
            desc_binding_cmd.heap_cb_state = cb_heap;
        };

    // When using VkCommandBufferInheritanceDescriptorHeapInfoEXT we emulate calling
    // vkCmdBindResourceHeapEXT/vkCmdBindSamplerHeapEXT here.
    // We can't do this in CommandBufferSubState::RecordBindResourceHeap because that is before
    // RegisterDescriptorChecksHeapValidation is called.
    if (cb.base.descriptor_heap.is_resource_inherited || cb.base.descriptor_heap.is_sampler_inherited) {
        DescriptorHeapBindings::BindingCommand bound_heap_snapshot{};
        desc_heap_bindings.on_update_bound_descriptor_heap(gpuav, cb, bound_heap_snapshot, false);
        desc_heap_bindings.bound_heap_snapshots.emplace_back(std::move(bound_heap_snapshot));
    }

    cb.on_instrumentation_error_logger_register_functions.emplace_back([](Validator& gpuav, CommandBufferSubState& cb,
                                                                          const LastBound& last_bound) {
        // Capture the last index into the snapshot on record time
        uint32_t heap_binding_index = vvl::kNoIndex32;  // if no heap was ever bound
        uint32_t push_data_index = vvl::kNoIndex32;     // if no push data was ever captured
        const DescriptorHeapBindings& desc_heap_bindings = cb.shared_resources_cache.Get<DescriptorHeapBindings>();
        if (!desc_heap_bindings.bound_heap_snapshots.empty()) {
            heap_binding_index = uint32_t(desc_heap_bindings.bound_heap_snapshots.size() - 1);
        }
        if (!desc_heap_bindings.push_data_snapshots.empty()) {
            push_data_index = uint32_t(desc_heap_bindings.push_data_snapshots.size() - 1);
        }

        CommandBufferSubState::InstrumentationErrorLogger inst_error_logger =
            [&cb, heap_binding_index, push_data_index](Validator& gpuav, const Location& loc, const uint32_t* error_record,
                                                       const InstrumentedShader* instrumented_shader, std::string& out_error_msg,
                                                       std::string& out_vuid_msg) {
                using namespace glsl;

                bool error_found = false;
                if (GetErrorGroup(error_record) != kErrorGroup_InstDescriptorHeap) {
                    return error_found;
                }
                error_found = true;

                std::ostringstream ss;

                // The user might have called vkCmdBindResourceHeap since recording, need to get the version at record time
                const vvl::CommandBuffer::DescriptorHeap* heap_cb_state = nullptr;
                // TODO - this is not being set for seconary command buffers
                if (heap_binding_index != vvl::kNoIndex32) {
                    const DescriptorHeapBindings& desc_heap_bindings = cb.shared_resources_cache.Get<DescriptorHeapBindings>();
                    heap_cb_state = &desc_heap_bindings.bound_heap_snapshots[heap_binding_index].heap_cb_state;
                }

                const uint32_t offset = error_record[kInst_LogError_ParameterOffset_1];

                const uint32_t desc_encoding = error_record[kInst_LogError_ParameterOffset_2];
                const vvlDescriptorType desc_type = static_cast<vvlDescriptorType>(
                    (desc_encoding & kInst_DescriptorHeap_DescriptorTypeMask) >> kInst_DescriptorHeap_DescriptorTypeShift);
                const uint32_t desc_size =
                    (desc_encoding & kInst_DescriptorHeap_DescriptorSizeMask) >> kInst_DescriptorHeap_DescriptorSizeShift;
                const uint32_t mapping_index_decoded =
                    (desc_encoding & kInst_DescriptorHeap_MappingIndexMask) >> kInst_DescriptorHeap_MappingIndexShift;
                const uint32_t alignment_value = 1u << (desc_encoding & kInst_DescriptorHeap_AlignmentShiftMask);

                const bool is_untyped = mapping_index_decoded == glsl::kInst_DescriptorHeap_MappingIndexUntyped;
                const bool is_buffer = static_cast<uint8_t>(desc_type) & vvlDescriptorBufferMask;
                const bool is_image = static_cast<uint8_t>(desc_type) & vvlDescriptorImageMask;
                const bool is_combined_sampler = desc_type == vvlDescriptorType::CombinedSampler;
                const bool is_sampler = desc_type == vvlDescriptorType::Sampler || is_combined_sampler;

                VkDeviceAddress heap_address = 0;
                if (heap_cb_state) {
                    heap_address = (is_sampler ? heap_cb_state->sampler_range.begin : heap_cb_state->resource_range.begin) + offset;
                }

                const VkDescriptorSetAndBindingMappingEXT* mapping_info = nullptr;
                const spirv::HeapMappingStatus* heap_status = nullptr;
                if (is_untyped) {
                    // TODO - Print some info for untyped pointers
                } else if (!instrumented_shader || mapping_index_decoded > instrumented_shader->status.heap_mappings.size()) {
                    ss << "(VkDescriptorSetAndBindingMappingEXT not found) ";
                } else {
                    heap_status = &instrumented_shader->status.heap_mappings[mapping_index_decoded];
                    mapping_info = &heap_status->mapping_data;
                    ss << "[Set " << std::dec << mapping_info->descriptorSet << ", Binding " << heap_status->binding
                       << ", Variable \"";
                    ::spirv::FindGlobalName(ss, instrumented_shader->original_spirv, (uint32_t)spv::OpVariable,
                                            heap_status->variable_id);
                    ss << "\"] ";
                }

                // We have 3 options
                // 1. Repeat error message logic for each mapping
                // 2. Report different subcode per mapping
                // 3. Just have a few boolean to know which mapping it came from
                const bool is_source_heap_data =
                    mapping_info && mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT;
                const bool is_combined_index = mapping_info && HasCombinedImageSamplerIndex(*mapping_info);

                // If mapping uses push data, provide the value to the user (or show they just forgot to set it!)
                uint32_t push_offset = 0;
                const bool use_sampler_push_offset = is_combined_sampler && !is_combined_index;
                const gpuav::DescriptorHeapBindings::PushDataSnapshot* push_data_snapshot = nullptr;
                bool push_data_set = true;
                bool has_two_push_dwords = false;
                if (mapping_info) {
                    if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT) {
                        push_offset = use_sampler_push_offset ? mapping_info->sourceData.pushIndex.samplerPushOffset
                                                              : mapping_info->sourceData.pushIndex.pushOffset;
                    } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
                        push_offset = use_sampler_push_offset ? mapping_info->sourceData.indirectIndex.samplerPushOffset
                                                              : mapping_info->sourceData.indirectIndex.pushOffset;
                        has_two_push_dwords = true;
                    } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
                        push_offset = use_sampler_push_offset ? mapping_info->sourceData.indirectIndexArray.samplerPushOffset
                                                              : mapping_info->sourceData.indirectIndexArray.pushOffset;
                        has_two_push_dwords = true;
                    } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT) {
                        push_offset = mapping_info->sourceData.heapData.pushOffset;
                    } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT) {
                        push_offset = mapping_info->sourceData.pushAddressOffset;
                        has_two_push_dwords = true;
                    } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT) {
                        push_offset = mapping_info->sourceData.indirectAddress.pushOffset;
                        has_two_push_dwords = true;
                    }

                    if (push_data_index != vvl::kNoIndex32) {
                        const DescriptorHeapBindings& desc_heap_bindings = cb.shared_resources_cache.Get<DescriptorHeapBindings>();
                        push_data_snapshot = &desc_heap_bindings.push_data_snapshots[push_data_index];

                        const uint32_t push_offset_dword = push_offset / 4;
                        const bool dword_0_set = (push_offset_dword < push_data_snapshot->dword_mask.size() &&
                                                  push_data_snapshot->dword_mask[push_offset_dword]);
                        const bool dword_1_set =
                            !has_two_push_dwords || (push_offset_dword + 1 < push_data_snapshot->dword_mask.size() &&
                                                     push_data_snapshot->dword_mask[push_offset_dword + 1]);
                        push_data_set = dword_0_set && dword_1_set;
                    } else if (mapping_info->source != VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT) {
                        // This means they just never called it once
                        // CONSTANT_DATA is the only mapping that doesn't use push data
                        push_data_set = false;
                    }
                }

                const uint32_t error_sub_code = GetSubError(error_record);
                switch (error_sub_code) {
                    case kErrorSubCode_DescriptorHeap_HeapOOB: {
                        const uint32_t descriptor_index = error_record[kInst_LogError_ParameterOffset_0];
                        if (!is_source_heap_data) {
                            ss << "descriptor index " << std::dec << descriptor_index << " ";
                        }
                        ss << "is accessing the " << (is_sampler ? "sampler" : "resource") << " heap at offset 0x" << std::hex
                           << offset << " (address 0x" << heap_address << ") which is OOB of the heap memory.\n";
                        out_vuid_msg = vvl::CreateActionVuid(loc.function, vvl::ActionVUID::DESCRIPTOR_HEAP_OOB_11309);
                        error_found = true;
                    } break;

                    case kErrorSubCode_DescriptorHeap_ReservedRange: {
                        assert(heap_cb_state);
                        if (!is_source_heap_data) {
                            const uint32_t descriptor_index = error_record[kInst_LogError_ParameterOffset_0];
                            ss << "descriptor index " << std::dec << descriptor_index << " ";
                        }
                        ss << "is accessing the " << (is_sampler ? "sampler" : "resource") << " heap at offset 0x" << std::hex
                           << offset << " (address 0x" << heap_address
                           << ") which is inside the reserved range.\nThe reserved range was bound at ";
                        if (is_sampler) {
                            ss << string_range_hex(heap_cb_state->sampler_reserved) << " (size of " << std::dec
                               << heap_cb_state->sampler_reserved.size() << " bytes)\n";
                        } else {
                            ss << string_range_hex(heap_cb_state->resource_reserved) << " (size of " << std::dec
                               << heap_cb_state->resource_reserved.size() << " bytes)\n";
                        }
                        out_vuid_msg = vvl::CreateActionVuid(loc.function, vvl::ActionVUID::DESCRIPTOR_HEAP_OOB_11309);
                        error_found = true;
                    } break;

                    case kErrorSubCode_DescriptorHeap_DescriptorAlignment:
                    case kErrorSubCode_DescriptorHeap_DescriptorAlignmentUntyped: {
                        const uint32_t descriptor_index = error_record[kInst_LogError_ParameterOffset_0];
                        ss << "descriptor index " << std::dec << descriptor_index << " is accessing the "
                           << (is_sampler ? "sampler" : "resource") << " heap at offset 0x" << std::hex << offset << " (address 0x"
                           << std::hex << heap_address << ") which is not aligned to ";
                        if (is_buffer) {
                            ss << "bufferDescriptorAlignment";
                        } else if (is_image) {
                            ss << "imageDescriptorAlignment";
                        } else if (is_sampler) {
                            ss << "samplerDescriptorAlignment";
                        }
                        ss << " (" << std::dec << alignment_value << ")\n";

                        if (error_sub_code == kErrorSubCode_DescriptorHeap_DescriptorAlignmentUntyped) {
                            out_vuid_msg = is_sampler ? "VUID-RuntimeSpirv-samplerDescriptorAlignment-11348"
                                           : is_image ? "VUID-RuntimeSpirv-imageDescriptorAlignment-11349"  // (and 11383)
                                                      : "VUID-RuntimeSpirv-bufferDescriptorAlignment-11384";
                        } else {
                            vvl::ActionVUID action_vuid = is_buffer  ? vvl::ActionVUID::DESCRIPTOR_HEAP_ALIGNMENT_11297
                                                          : is_image ? vvl::ActionVUID::DESCRIPTOR_HEAP_ALIGNMENT_11298
                                                                     : vvl::ActionVUID::DESCRIPTOR_HEAP_ALIGNMENT_11299;
                            out_vuid_msg = vvl::CreateActionVuid(loc.function, action_vuid);
                        }
                        error_found = true;
                    } break;

                    case kErrorSubCode_DescriptorHeap_IndirectIndexPushAlignment:
                    case kErrorSubCode_DescriptorHeap_IndirectAddressPushAlignment: {
                        if (error_sub_code == kErrorSubCode_DescriptorHeap_IndirectIndexPushAlignment) {
                            const uint32_t descriptor_index = error_record[kInst_LogError_ParameterOffset_0];
                            ss << "descriptor index " << std::dec << descriptor_index << " ";
                        }
                        ss << "has a " << (use_sampler_push_offset ? "samplerPushOffset" : "pushOffset") << " of ";

                        if (mapping_info) {
                            ss << std::dec << push_offset;
                        } else {
                            ss << "(unknown offset)";
                        }

                        ss << " that value at this time is ";
                        if (!push_data_snapshot) {
                            ss << "[unknown] which";
                        } else {
                            if (push_data_set) {
                                const VkDeviceAddress push_data = *((VkDeviceAddress*)&push_data_snapshot->value[push_offset]);
                                ss << "the VkDeviceAddress 0x" << std::hex << push_data << " which";
                            } else {
                                ss << "[unknown] because vkCmdPushDataEXT was not called and the undefined value";
                            }
                        }

                        ss << " is not a multiple of ";
                        if (has_two_push_dwords) {
                            ss << "8 needed to access the VkDeviceAddress in the indirect buffer\n";
                        } else {
                            ss << "4 needed to access the uint32_t in the indirect buffer\n";
                        }

                        vvl::ActionVUID action_vuid = (error_sub_code == kErrorSubCode_DescriptorHeap_IndirectIndexPushAlignment)
                                                          ? vvl::ActionVUID::DESCRIPTOR_HEAP_INDIRECT_INDEX_PUSH_11300
                                                          : vvl::ActionVUID::DESCRIPTOR_HEAP_INDIRECT_ADDRESS_PUSH_11304;
                        out_vuid_msg = vvl::CreateActionVuid(loc.function, action_vuid);
                        error_found = true;
                    } break;

                    case kErrorSubCode_DescriptorHeap_AddressBufferAlignment: {
                        const bool is_uniform = desc_type == vvlDescriptorType::UniformBuffer;
                        const uint64_t final_address =
                            *reinterpret_cast<const uint64_t*>(error_record + kInst_LogError_ParameterOffset_0);
                        const bool push_address =
                            (mapping_info && mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT);

                        ss << "is accessing the descriptor at its " << (push_address ? "indirect" : "resource") << " buffer at 0x"
                           << std::hex << final_address << ", but this not aligned to "
                           << (is_uniform ? "minUniformBufferOffsetAlignment" : "minStorageBufferOffsetAlignment") << " ("
                           << std::dec
                           << (is_uniform ? gpuav.phys_dev_props.limits.minUniformBufferOffsetAlignment
                                          : gpuav.phys_dev_props.limits.minStorageBufferOffsetAlignment)
                           << ")\n";

                        vvl::ActionVUID action_vuid = (is_uniform)
                                                          ? vvl::ActionVUID::DESCRIPTOR_HEAP_ADDRESS_BUFFER_ALIGNMENT_11441
                                                          : vvl::ActionVUID::DESCRIPTOR_HEAP_ADDRESS_BUFFER_ALIGNMENT_11442;
                        out_vuid_msg = vvl::CreateActionVuid(loc.function, action_vuid);
                        error_found = true;
                    } break;

                    case kErrorSubCode_DescriptorHeap_HeapBufferAlignment: {
                        ss << "is accessing the resource heap at offset 0x" << std::hex << offset << " (address 0x" << heap_address
                           << ") which is not aligned to minUniformBufferOffsetAlignment (0x"
                           << gpuav.phys_dev_props.limits.minUniformBufferOffsetAlignment << ")\n";

                        out_vuid_msg =
                            vvl::CreateActionVuid(loc.function, vvl::ActionVUID::DESCRIPTOR_HEAP_ADDRESS_BUFFER_ALIGNMENT_11441);
                        error_found = true;
                    } break;

                    case kErrorSubCode_DescriptorHeap_InvalidDeviceAddress:
                    case kErrorSubCode_DescriptorHeap_InvalidDeviceAddressResource: {
                        vvl::ActionVUID action_vuid = vvl::ActionVUID::UNKNOWN;
                        if (mapping_info) {
                            if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT ||
                                mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
                                action_vuid = vvl::ActionVUID::DESCRIPTOR_HEAP_INVALID_ADDRESS_11301;
                            } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT) {
                                action_vuid = vvl::ActionVUID::DESCRIPTOR_HEAP_INVALID_ADDRESS_11302;
                            } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT) {
                                action_vuid = error_sub_code == kErrorSubCode_DescriptorHeap_InvalidDeviceAddress
                                                  ? vvl::ActionVUID::DESCRIPTOR_HEAP_INVALID_ADDRESS_11305
                                                  : vvl::ActionVUID::DESCRIPTOR_HEAP_INVALID_ADDRESS_11306;
                            }
                        }

                        ss << "is accessing the indirect buffer, but the indirect address is null.\n";
                        out_vuid_msg = vvl::CreateActionVuid(loc.function, action_vuid);
                        error_found = true;
                    } break;
                    case kErrorSubCode_DescriptorHeap_DescriptorHashing_None:
                    case kErrorSubCode_DescriptorHeap_DescriptorHashing_Wrong: {
                        ReadLockGuard guard(gpuav.device_state->descriptor_hashing->map_lock);
                        if (gpuav.device_state->descriptor_hashing->table.limit_reported) {
                            // If we have hit the limit, we are likely going to spam errors and want them to see
                            // the DESCRIPTOR-HASHING-LIMIT only instead
                            return error_found;
                        }

                        if (error_sub_code == kErrorSubCode_DescriptorHeap_DescriptorHashing_None) {
                            const uint32_t descriptor_index = error_record[kInst_LogError_ParameterOffset_0];
                            ss << "descriptor index " << std::dec << descriptor_index << " is accessing the "
                               << (is_sampler ? "sampler" : "resource") << " heap at offset 0x" << std::hex << offset
                               << " (address 0x" << heap_address << ") but there is no valid descriptor at that location.\n";
                            out_vuid_msg = "UNASSIGNED-DescriptorHeap-No-Descriptor";
                        } else {
                            // We trade the descriptor_index for getting the slot_index to find what descriptor this was
                            const uint32_t slot_index = error_record[kInst_LogError_ParameterOffset_0];
                            const auto& entry = gpuav.device_state->descriptor_hashing->table.slots[slot_index].entry;
                            ss << "is accessing the " << (is_sampler ? "sampler" : "resource") << " heap at offset 0x" << std::hex
                               << offset << " (address 0x" << heap_address
                               << ") but there is a descriptor there of the wrong type: \n";
                            entry.Describe(*gpuav.device_state, ss);
                            ss << '\n';
                            out_vuid_msg = "UNASSIGNED-DescriptorHeap-Wrong-DescriptorType";
                        }
                        error_found = true;
                    } break;
                }

                if (mapping_info) {
                    // only can occur if have mappings
                    if (!push_data_set) {
                        ss << "- vkCmdPushDataEXT[" << std::dec << push_offset << ":"
                           << (has_two_push_dwords ? push_offset + 7 : push_offset + 3)
                           << "] was not called and the values are undefined\n";
                    }

                    ss << "- Mapped with pMapping[" << std::dec << heap_status->mapping_index << "] with "
                       << string_VkDescriptorMappingSourceEXT(mapping_info->source) << std::hex;
                    const uint32_t bind_offset = heap_status->binding - mapping_info->firstBinding;
                    if (bind_offset != 0) {
                        ss << " (firstBindng: " << std::dec << mapping_info->firstBinding
                           << ", bindingCount: " << mapping_info->bindingCount << ")";
                    }
                    if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT) {
                        const VkDescriptorMappingSourceConstantOffsetEXT& map_data = mapping_info->sourceData.constantOffset;
                        if (is_combined_sampler) {
                            ss << "\n  - samplerHeapOffset = 0x" << std::hex << map_data.samplerHeapOffset;
                            ss << "\n  - samplerHeapArrayStride = " << std::dec << map_data.samplerHeapArrayStride;
                        } else {
                            ss << "\n  - heapOffset = 0x" << std::hex << map_data.heapOffset;
                            ss << "\n  - heapArrayStride = " << std::dec << map_data.heapArrayStride;
                        }
                    } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT) {
                        const VkDescriptorMappingSourcePushIndexEXT& map_data = mapping_info->sourceData.pushIndex;
                        if (is_combined_sampler) {
                            ss << "\n  - samplerHeapOffset = 0x" << std::hex << map_data.samplerHeapOffset;
                            if (is_combined_index) {
                                ss << "\n  - pushOffset = " << std::dec << map_data.pushOffset;
                            } else {
                                ss << "\n  - samplerPushOffset = " << std::dec << map_data.samplerPushOffset;
                            }
                            ss << "\n  - samplerHeapIndexStride = " << map_data.samplerHeapIndexStride;
                            ss << "\n  - samplerHeapArrayStride = " << map_data.samplerHeapArrayStride;
                            ss << "\n  - useCombinedImageSamplerIndex = "
                               << (map_data.useCombinedImageSamplerIndex ? "true" : "false");
                        } else {
                            ss << "\n  - heapOffset = 0x" << std::hex << map_data.heapOffset;
                            ss << "\n  - pushOffset = " << std::dec << map_data.pushOffset;
                            ss << "\n  - heapIndexStride = " << map_data.heapIndexStride;
                            ss << "\n  - heapArrayStride = " << map_data.heapArrayStride;
                        }
                    } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
                        const VkDescriptorMappingSourceIndirectIndexEXT& map_data = mapping_info->sourceData.indirectIndex;
                        if (is_combined_sampler) {
                            ss << "\n  - samplerHeapOffset = 0x" << std::hex << map_data.samplerHeapOffset;
                            if (is_combined_index) {
                                ss << "\n  - addressOffset = 0x" << map_data.addressOffset;
                                ss << "\n  - pushOffset = " << std::dec << map_data.pushOffset;
                            } else {
                                ss << "\n  - samplerAddressOffset = 0x" << map_data.samplerAddressOffset;
                                ss << "\n  - samplerPushOffset = " << std::dec << map_data.samplerPushOffset;
                            }
                            ss << "\n  - samplerHeapIndexStride = " << map_data.samplerHeapIndexStride;
                            ss << "\n  - samplerHeapArrayStride = " << map_data.samplerHeapArrayStride;
                            ss << "\n  - useCombinedImageSamplerIndex = "
                               << (map_data.useCombinedImageSamplerIndex ? "true" : "false");
                        } else {
                            ss << "\n  - heapOffset = 0x" << std::hex << map_data.heapOffset;
                            ss << "\n  - addressOffset = 0x" << map_data.addressOffset;
                            ss << "\n  - pushOffset = " << std::dec << map_data.pushOffset;
                            ss << "\n  - heapIndexStride = " << map_data.heapIndexStride;
                            ss << "\n  - heapArrayStride = " << map_data.heapArrayStride;
                        }
                    } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
                        const VkDescriptorMappingSourceIndirectIndexArrayEXT& map_data =
                            mapping_info->sourceData.indirectIndexArray;
                        if (is_combined_sampler) {
                            ss << "\n  - samplerHeapOffset = 0x" << std::hex << map_data.samplerHeapOffset;
                            if (is_combined_index) {
                                ss << "\n  - addressOffset = 0x" << map_data.addressOffset;
                                ss << "\n  - pushOffset = " << std::dec << map_data.pushOffset;
                            } else {
                                ss << "\n  - samplerAddressOffset = 0x" << map_data.samplerAddressOffset;
                                ss << "\n  - samplerPushOffset = " << std::dec << map_data.samplerPushOffset;
                            }
                            ss << "\n  - samplerHeapIndexStride = " << map_data.samplerHeapIndexStride;
                            ss << "\n  - useCombinedImageSamplerIndex = "
                               << (map_data.useCombinedImageSamplerIndex ? "true" : "false");
                        } else {
                            ss << "\n  - heapOffset = 0x" << std::hex << map_data.heapOffset;
                            ss << "\n  - addressOffset = 0x" << map_data.addressOffset;
                            ss << "\n  - pushOffset = " << std::dec << map_data.pushOffset;
                            ss << "\n  - heapIndexStride = " << map_data.heapIndexStride;
                        }
                    } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT) {
                        const VkDescriptorMappingSourceHeapDataEXT& map_data = mapping_info->sourceData.heapData;
                        ss << "\n  - heapOffset = 0x" << std::hex << map_data.heapOffset;
                        ss << "\n  - pushOffset = " << std::dec << map_data.pushOffset << " (which loaded the value 0x"
                           << std::hex << error_record[kInst_LogError_ParameterOffset_0] << ")";
                    } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT) {
                        ss << "\n  - pushDataOffset = " << std::dec << mapping_info->sourceData.pushDataOffset;
                    } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT) {
                        ss << "\n  - pushAddressOffset = " << std::dec << mapping_info->sourceData.pushAddressOffset;
                    } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT) {
                        const VkDescriptorMappingSourceIndirectAddressEXT& map_data = mapping_info->sourceData.indirectAddress;
                        ss << "\n  - pushOffset = " << std::dec << map_data.pushOffset;
                        ss << "\n  - addressOffset = 0x" << std::hex << map_data.addressOffset;
                    }
                    ss << '\n';
                }

                ss << "- Descriptor type: " << string_VkDescriptorType(GetDescriptorTypeFromMask(desc_type)) << ", size "
                   << std::dec << desc_size;
                if (is_untyped) {
                    if (is_buffer) {
                        ss << " (bufferDescriptorSize)";
                    } else if (is_image) {
                        ss << " (imageDescriptorSize)";
                    } else if (is_sampler) {
                        ss << " (samplerDescriptorSize)";
                    }
                } else if (is_combined_sampler) {
                    ss << " (from a combined image sampler)";
                }
                ss << '\n';

                // Unless find otherwise, this information is useful for all errors
                ss << "- The " << (is_sampler ? "sampler" : "resource") << " heap was ";
                if (is_sampler) {
                    if (!heap_cb_state || !heap_cb_state->sampler_bound) {
                        ss << "not bound with vkCmdBindSamplerHeapEXT";
                    } else {
                        ss << "bound at " << string_range_hex(heap_cb_state->sampler_range) << " (size of " << std::dec
                           << heap_cb_state->sampler_range.size() << " bytes)\n";
                    }
                } else {
                    if (!heap_cb_state || !heap_cb_state->resource_bound) {
                        ss << "not bound with vkCmdBindResourceHeapEXT";
                    } else {
                        ss << "bound at " << string_range_hex(heap_cb_state->resource_range) << " (size of " << std::dec
                           << heap_cb_state->resource_range.size() << " bytes)\n";
                    }
                }

                out_error_msg += ss.str();
                return error_found;
            };

        return inst_error_logger;
    });

    cb.on_instrumentation_common_desc_update_functions.emplace_back([&gpuav](CommandBufferSubState& cb, const LastBound& last_bound,
                                                                             const Location&,
                                                                             CommonDescriptorUpdate& out_update) mutable {
        // struct DescriptorHeapEncoding
        const uint32_t bound_heap_info_size = sizeof(VkDeviceAddress) * 3;
        uint32_t buffer_size = bound_heap_info_size + (uint32_t)gpuav.phys_dev_ext_props.descriptor_heap_props.maxPushDataSize;
        vko::BufferRange output_range = cb.gpu_resources_manager.GetHostCoherentBufferRange(buffer_size);

        VkDeviceAddress* bound_heap_info = (VkDeviceAddress*)output_range.offset_mapped_ptr;

        // If these are zero that is valid, core validation gives error if heap is not set when needed
        DescriptorChecksHeapCbState* dc_cb_state = cb.shared_resources_cache.TryGet<DescriptorChecksHeapCbState>();
        if (dc_cb_state) {
            bound_heap_info[0] = dc_cb_state->last_bound_heap_info_resource.offset_address;
            bound_heap_info[1] = dc_cb_state->last_bound_heap_info_sampler.offset_address;
            if (gpuav.global_settings.descriptor_hashing) {
                // TODO -
                //  See if this could be maintained at the gpuav level
                //  using gpuav.shared_resources_cache (it supports concurrent accesses).
                // It's device global by nature and only the snapshots need to be maintained at CB level
                ReadLockGuard guard(gpuav.device_state->descriptor_hashing->map_lock);
                DescriptorChecksHeapHashTable& hash_table_state =
                    cb.shared_resources_cache.GetOrCreate<DescriptorChecksHeapHashTable>(
                        cb, gpuav.device_state->descriptor_hashing->table.Size());
                bound_heap_info[2] = hash_table_state.buffer_range.offset_address;
            }
        }

        uint8_t* gpu_push_data_ptr = ((uint8_t*)output_range.offset_mapped_ptr) + bound_heap_info_size;
        memcpy(gpu_push_data_ptr, cb.push_data_value.data(), cb.push_data_value.size());

        // If using the same vkCmdPushData, can use last snapshot
        if (cb.push_data_updated) {
            // If people are using only CONSTANT_DATA mapping or untyped, no reason to waste memory copying this
            small_vector<const ShaderStageState*, 3> stages = last_bound.GetStages();
            bool update_push_data = false;
            for (const ShaderStageState* stage : stages) {
                update_push_data |= stage->heap.uses_push_data;
            }
            if (update_push_data) {
                DescriptorHeapBindings& desc_heap_bindings = cb.shared_resources_cache.Get<DescriptorHeapBindings>();
                DescriptorHeapBindings::PushDataSnapshot push_data_snapshot{cb.base.push_data_dword_mask, cb.push_data_value};
                desc_heap_bindings.push_data_snapshots.emplace_back(std::move(push_data_snapshot));
            }
        }

        out_update.buffer = output_range.buffer;
        out_update.offset = output_range.offset;
        out_update.range = output_range.size;
        out_update.address = output_range.offset_address;
        out_update.binding = glsl::kBindingInstDescriptorHeap;
    });

    cb.on_pre_cb_submission_functions.emplace_back(
        [](Validator& gpuav, CommandBufferSubState& cb, VkCommandBuffer per_submission_cb) {
            if (!gpuav.global_settings.descriptor_hashing) {
                return;
            }

            DescriptorChecksHeapHashTable* hash_table_state = cb.shared_resources_cache.TryGet<DescriptorChecksHeapHashTable>();
            if (!hash_table_state) {
                return;
            }

            // Current issue with |descriptor_hashing.table| is it done on the CPU because
            // - it can be both large (few MB)
            // - We expect MANY small writes to it on the CPU
            // - GPU Dump also plans to use it, and it doesn't need GPU memory to do it
            //
            // TODO - We are doing a VERY wasteful double buffer allocation per cmd buffer
            // We need to design a way to at least make only a single staging buffer for the entire
            // application
            vvl::DescriptorHashing& descriptor_hashing = *gpuav.device_state->descriptor_hashing;
            ReadLockGuard guard(descriptor_hashing.map_lock);
            const VkDeviceSize table_size = descriptor_hashing.table.Size();

            vko::BufferRange staging_buffer = cb.gpu_resources_manager.GetHostCachedBufferRange(table_size);
            memcpy(staging_buffer.offset_mapped_ptr, descriptor_hashing.table.slots.data(), static_cast<size_t>(table_size));
            cb.gpu_resources_manager.FlushAllocation(staging_buffer);

            vko::CmdSynchronizedCopyBufferRange(per_submission_cb, hash_table_state->buffer_range, staging_buffer);
        });
}

}  // namespace gpuav
