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

namespace gpuav {

// Way to hold the last snapshot
struct DescriptorChecksHeapCbState {
    vko::BufferRange last_bound_heap_info_resource;
    vko::BufferRange last_bound_heap_info_sampler;
};

void RegisterDescriptorChecksHeapValidation(Validator& gpuav, CommandBufferSubState& cb) {
    if (!gpuav.gpuav_settings.shader_instrumentation.descriptor_checks) {
        return;
    }
    if (!gpuav.enabled_features.descriptorHeap) {
        return;
    }

    DescriptorHeapBindings& desc_heap_bindings = cb.shared_resources_cache.GetOrCreate<DescriptorHeapBindings>();

    desc_heap_bindings.on_update_bound_descriptor_heap.emplace_back(
        [](Validator& gpuav, CommandBufferSubState& cb, DescriptorHeapBindings::BindingCommand& desc_binding_cmd, bool is_sampler) {
            DescriptorChecksHeapCbState& dc_cb_state = cb.shared_resources_cache.GetOrCreate<DescriptorChecksHeapCbState>();

            auto& cb_heap = cb.base.descriptor_heap;
            assert(cb_heap.resource_range.size() < (VkDeviceAddress)vvl::kU32Max);
            assert(cb_heap.sampler_range.size() < (VkDeviceAddress)vvl::kU32Max);

            if (is_sampler) {
                dc_cb_state.last_bound_heap_info_sampler =
                    cb.gpu_resources_manager.GetHostCoherentBufferRange(sizeof(glsl::BoundHeapInfo));
                dc_cb_state.last_bound_heap_info_sampler.Clear();
                auto desc_heap_info = static_cast<glsl::BoundHeapInfo*>(dc_cb_state.last_bound_heap_info_sampler.offset_mapped_ptr);

                desc_heap_info->heap_size = (uint32_t)cb_heap.sampler_range.size();
                desc_heap_info->reserved_begin = (uint32_t)(cb_heap.sampler_reserved.begin - cb_heap.sampler_range.begin);
                desc_heap_info->reserved_end = (uint32_t)(cb_heap.sampler_reserved.end - cb_heap.sampler_range.begin);
            } else {
                dc_cb_state.last_bound_heap_info_resource =
                    cb.gpu_resources_manager.GetHostCoherentBufferRange(sizeof(glsl::BoundHeapInfo));
                dc_cb_state.last_bound_heap_info_resource.Clear();
                auto desc_heap_info =
                    static_cast<glsl::BoundHeapInfo*>(dc_cb_state.last_bound_heap_info_resource.offset_mapped_ptr);

                desc_heap_info->heap_size = (uint32_t)cb_heap.resource_range.size();
                desc_heap_info->reserved_begin = (uint32_t)(cb_heap.resource_reserved.begin - cb_heap.resource_range.begin);
                desc_heap_info->reserved_end = (uint32_t)(cb_heap.resource_reserved.end - cb_heap.resource_range.begin);
            }

            // Data for the GPU
            desc_binding_cmd.bound_heap_info_resource = dc_cb_state.last_bound_heap_info_resource;
            desc_binding_cmd.bound_heap_info_sampler = dc_cb_state.last_bound_heap_info_sampler;
            // Data for the CPU (since the command buffer might updated)
            desc_binding_cmd.heap_cb_state = cb_heap;
        });

    cb.on_instrumentation_error_logger_register_functions.emplace_back([](Validator& gpuav, CommandBufferSubState& cb,
                                                                          const LastBound& last_bound) {
        // Capture the last index into the snapshot on record time
        uint32_t heap_binding_index = vvl::kNoIndex32;  // if no heap was ever bound
        const DescriptorHeapBindings& desc_heap_bindings = cb.shared_resources_cache.Get<DescriptorHeapBindings>();
        if (!desc_heap_bindings.bound_heap_snapshots.empty()) {
            heap_binding_index = uint32_t(desc_heap_bindings.bound_heap_snapshots.size() - 1);
        }

        CommandBufferSubState::InstrumentationErrorLogger inst_error_logger =
            [&cb, heap_binding_index](Validator& gpuav, const Location& loc, const uint32_t* error_record,
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
                if (heap_binding_index != vvl::kNoIndex32) {
                    const DescriptorHeapBindings& desc_heap_bindings = cb.shared_resources_cache.Get<DescriptorHeapBindings>();
                    heap_cb_state = &desc_heap_bindings.bound_heap_snapshots[heap_binding_index].heap_cb_state;
                }

                const uint32_t index = error_record[kInst_LogError_ParameterOffset_0];
                const uint32_t offset = error_record[kInst_LogError_ParameterOffset_1];

                const uint32_t desc_encoding = error_record[kInst_LogError_ParameterOffset_2];
                const uint32_t desc_type =
                    (desc_encoding & kInst_DescriptorHeap_DescriptorTypeMask) >> kInst_DescriptorHeap_DescriptorTypeShift;
                const uint32_t mapping_index_decoded =
                    (desc_encoding & kInst_DescriptorHeap_MappingIndexMask) >> kInst_DescriptorHeap_MappingIndexShift;
                const uint32_t alignment_value = 1u << (desc_encoding & kInst_DescriptorHeap_AlignmentShiftMask);

                const bool is_buffer = desc_type & gpuav::descriptor::TYPE_BUFFER_MASK;
                const bool is_image = desc_type & gpuav::descriptor::TYPE_IMAGE_MASK;
                const bool is_sampler = desc_type == gpuav::descriptor::TYPE_SAMPLER;

                VkDeviceAddress heap_address = 0;
                if (heap_cb_state) {
                    heap_address = (is_sampler ? heap_cb_state->sampler_range.begin : heap_cb_state->resource_range.begin) + offset;
                }

                const VkDescriptorSetAndBindingMappingEXT* mapping_info = nullptr;
                const spirv::HeapMappingStatus* heap_status = nullptr;
                if (mapping_index_decoded == glsl::kInst_DescriptorHeap_MappingIndexUntyped) {
                    // TODO - Print some info for untyped pointers
                } else if (!instrumented_shader || mapping_index_decoded > instrumented_shader->status.heap_mappings.size()) {
                    ss << "(VkDescriptorSetAndBindingMappingEXT not found) ";
                } else {
                    heap_status = &instrumented_shader->status.heap_mappings[mapping_index_decoded];
                    mapping_info = &heap_status->mapping_data;
                    ss << "[Set " << mapping_info->descriptorSet << ", Binding " << heap_status->binding << ", Variable \"";
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

                const uint32_t error_sub_code = GetSubError(error_record);
                switch (error_sub_code) {
                    case kErrorSubCode_DescriptorHeap_HeapOOB: {
                        if (!is_source_heap_data) {
                            ss << "descriptor index " << index << " ";
                        }
                        ss << "is accessing the heap at offset 0x" << std::hex << offset << " (address 0x" << std::hex
                           << heap_address << ") which is OOB of the heap memory.\n";
                        out_vuid_msg = vvl::CreateActionVuid(loc.function, vvl::ActionVUID::DESCRIPTOR_HEAP_OOB_11309);
                        error_found = true;
                    } break;

                    case kErrorSubCode_DescriptorHeap_ReservedRange: {
                        assert(heap_cb_state);
                        if (!is_source_heap_data) {
                            ss << "descriptor index " << index << " ";
                        }
                        ss << "is accessing the heap at offset 0x" << std::hex << offset << " (address 0x" << std::hex
                           << heap_address << ") which is inside the reserved range.\nThe reserved range was bound at ";
                        if (is_sampler) {
                            ss << string_range_hex(heap_cb_state->sampler_reserved) << " (size of 0x" << std::hex
                               << heap_cb_state->sampler_reserved.size() << " bytes)\n";
                        } else {
                            ss << string_range_hex(heap_cb_state->resource_reserved) << " (size of 0x" << std::hex
                               << heap_cb_state->resource_reserved.size() << " bytes)\n";
                        }
                        out_vuid_msg = vvl::CreateActionVuid(loc.function, vvl::ActionVUID::DESCRIPTOR_HEAP_OOB_11309);
                        error_found = true;
                    } break;

                    case kErrorSubCode_DescriptorHeap_DescriptorAlignment:
                    case kErrorSubCode_DescriptorHeap_DescriptorAlignmentUntyped: {
                        ss << "descriptor index " << index << " is accessing the heap at offset 0x" << std::hex << offset
                           << " (address 0x" << std::hex << heap_address << ") which is not aligned to ";
                        if (is_buffer) {
                            ss << "bufferDescriptorAlignment";
                        } else if (is_image) {
                            ss << "imageDescriptorAlignment";
                        } else if (is_sampler) {
                            ss << "samplerDescriptorAlignment";
                        }
                        ss << " (0x" << std::hex << alignment_value << ")\n";

                        if (error_sub_code == kErrorSubCode_DescriptorHeap_DescriptorAlignmentUntyped) {
                            out_vuid_msg = is_sampler ? "VUID-RuntimeSpirv-samplerDescriptorAlignment-11348"
                                           : is_image ? "VUID-RuntimeSpirv-imageDescriptorAlignment-11349"
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
                            ss << "descriptor index " << index << " ";
                        }
                        ss << "has a " << (is_sampler ? "samplerPushOffset" : "pushOffset") << " of ";

                        uint32_t push_offset = 0;
                        if (mapping_info) {
                            // TODO - Preserve pushOffset instead of saving it through the shader
                            if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT) {
                                push_offset = mapping_info->sourceData.indirectAddress.pushOffset;
                            } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
                                push_offset = is_sampler ? mapping_info->sourceData.indirectIndex.samplerPushOffset
                                                         : mapping_info->sourceData.indirectIndex.pushOffset;
                            } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
                                push_offset = is_sampler ? mapping_info->sourceData.indirectIndexArray.samplerPushOffset
                                                         : mapping_info->sourceData.indirectIndexArray.pushOffset;
                            }
                            ss << "0x" << std::hex << push_offset;
                        } else {
                            ss << "(unknown offset)";
                        }

                        // TODO - this will report the wrong pushData, see above about capturing the cb state
                        const VkDeviceAddress push_data = *((VkDeviceAddress*)&cb.push_data_value[push_offset]);

                        ss << " which holds the VkDeviceAddress 0x" << std::hex << push_data << " which is not a multiple of ";
                        if (error_sub_code == kErrorSubCode_DescriptorHeap_IndirectIndexPushAlignment) {
                            ss << "4 needed to access the uint32_t in the indirect buffer\n";
                        } else {
                            ss << "8 needed to access the VkDeviceAddress in the indirect buffer\n";
                        }

                        vvl::ActionVUID action_vuid = (error_sub_code == kErrorSubCode_DescriptorHeap_IndirectIndexPushAlignment)
                                                          ? vvl::ActionVUID::DESCRIPTOR_HEAP_INDIRECT_INDEX_PUSH_11300
                                                          : vvl::ActionVUID::DESCRIPTOR_HEAP_INDIRECT_ADDRESS_PUSH_11304;
                        out_vuid_msg = vvl::CreateActionVuid(loc.function, action_vuid);
                        error_found = true;
                    } break;

                    case kErrorSubCode_DescriptorHeap_AddressBufferAlignment: {
                        const bool is_uniform = desc_type == gpuav::descriptor::TYPE_UNIFORM_BUFFER;
                        const uint64_t final_address =
                            *reinterpret_cast<const uint64_t*>(error_record + kInst_LogError_ParameterOffset_0);
                        const bool push_address =
                            (mapping_info && mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT);

                        ss << "is trying to access the descriptor at its " << (push_address ? "indirect" : "resource")
                           << " buffer at 0x" << std::hex << final_address << ", but this not aligned to "
                           << (is_uniform ? "minUniformBufferOffsetAlignment" : "minStorageBufferOffsetAlignment") << " (0x"
                           << (is_uniform ? gpuav.phys_dev_props.limits.minUniformBufferOffsetAlignment
                                          : gpuav.phys_dev_props.limits.minStorageBufferOffsetAlignment)
                           << ")\n";

                        vvl::ActionVUID action_vuid = (is_uniform)
                                                          ? vvl::ActionVUID::DESCRIPTOR_HEAP_ADDRESS_BUFFER_ALIGNMENT_11441
                                                          : vvl::ActionVUID::DESCRIPTOR_HEAP_ADDRESS_BUFFER_ALIGNMENT_11442;
                        out_vuid_msg = vvl::CreateActionVuid(loc.function, action_vuid);
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

                        ss << "is trying to access the indirect buffer, but the indirect address is null.\n";
                        out_vuid_msg = vvl::CreateActionVuid(loc.function, action_vuid);
                        error_found = true;
                    } break;
                }

                if (mapping_info) {
                    ss << "Mapped with pMapping[" << std::dec << heap_status->mapping_index << "] with "
                       << string_VkDescriptorMappingSourceEXT(mapping_info->source) << std::hex;
                    const uint32_t bind_offset = heap_status->binding - mapping_info->firstBinding;
                    if (bind_offset != 0) {
                        ss << " (firstBindng: " << mapping_info->firstBinding << ", bindingCount: " << mapping_info->bindingCount
                           << ")";
                    }
                    if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT) {
                        const VkDescriptorMappingSourceConstantOffsetEXT& map_data = mapping_info->sourceData.constantOffset;
                        ss << "\n  - heapOffset = 0x" << map_data.heapOffset;
                        ss << "\n  - heapArrayStride = 0x" << map_data.heapArrayStride;
                        // TODO - have way to know if combined image sampler
                    } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_PUSH_INDEX_EXT) {
                        const VkDescriptorMappingSourcePushIndexEXT& map_data = mapping_info->sourceData.pushIndex;
                        ss << "\n  - heapOffset = 0x" << map_data.heapOffset;
                        ss << "\n  - pushOffset = 0x" << map_data.pushOffset;
                        ss << "\n  - heapIndexStride = 0x" << map_data.heapIndexStride;
                        ss << "\n  - heapArrayStride = 0x" << map_data.heapArrayStride;
                    } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
                        const VkDescriptorMappingSourceIndirectIndexEXT& map_data = mapping_info->sourceData.indirectIndex;
                        ss << "\n  - heapOffset = 0x" << map_data.heapOffset;
                        ss << "\n  - pushOffset = 0x" << map_data.pushOffset;
                        ss << "\n  - addressOffset = 0x" << map_data.addressOffset;
                        ss << "\n  - heapIndexStride = 0x" << map_data.heapIndexStride;
                        ss << "\n  - heapArrayStride = 0x" << map_data.heapArrayStride;
                    } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_ARRAY_EXT) {
                        const VkDescriptorMappingSourceIndirectIndexArrayEXT& map_data =
                            mapping_info->sourceData.indirectIndexArray;
                        ss << "\n  - heapOffset = 0x" << map_data.heapOffset;
                        ss << "\n  - pushOffset = 0x" << map_data.pushOffset;
                        ss << "\n  - addressOffset = 0x" << map_data.addressOffset;
                        ss << "\n  - heapIndexStride = 0x" << map_data.heapIndexStride;
                    } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_RESOURCE_HEAP_DATA_EXT) {
                        const VkDescriptorMappingSourceHeapDataEXT& map_data = mapping_info->sourceData.heapData;
                        ss << "\n  - heapOffset = 0x" << map_data.heapOffset;
                        ss << "\n  - pushOffset = 0x" << map_data.pushOffset << " (which loaded the value 0x"
                           << error_record[kInst_LogError_ParameterOffset_0] << ")";
                    } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_DATA_EXT) {
                        ss << "\n  - pushDataOffset = 0x" << mapping_info->sourceData.pushDataOffset;
                    } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_PUSH_ADDRESS_EXT) {
                        ss << "\n  - pushAddressOffset = 0x" << mapping_info->sourceData.pushAddressOffset;
                    } else if (mapping_info->source == VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT) {
                        const VkDescriptorMappingSourceIndirectAddressEXT& map_data = mapping_info->sourceData.indirectAddress;
                        ss << "\n  - pushOffset = 0x" << map_data.pushOffset;
                        ss << "\n  - addressOffset = 0x" << map_data.addressOffset;
                    }
                    ss << "\n";
                }

                // Unless find otherwise, this information is useful for all errors
                ss << "The " << (is_sampler ? "sampler" : "resource") << " heap was ";
                if (is_sampler) {
                    if (!heap_cb_state || !heap_cb_state->sampler_bound) {
                        ss << "not bound with vkCmdBindSamplerHeapEXT";
                    } else {
                        ss << "bound at " << string_range_hex(heap_cb_state->sampler_range) << " (size of 0x" << std::hex
                           << heap_cb_state->sampler_range.size() << " bytes)\n";
                    }
                } else {
                    if (!heap_cb_state || !heap_cb_state->resource_bound) {
                        ss << "not bound with vkCmdBindResourceHeapEXT";
                    } else {
                        ss << "bound at " << string_range_hex(heap_cb_state->resource_range) << " (size of 0x" << std::hex
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
        const uint32_t bound_heap_info_size = sizeof(VkDeviceAddress) * 2;
        uint32_t buffer_size = bound_heap_info_size + (uint32_t)gpuav.phys_dev_ext_props.descriptor_heap_props.maxPushDataSize;
        vko::BufferRange buffer_range = cb.gpu_resources_manager.GetHostCoherentBufferRange(buffer_size);

        VkDeviceAddress* bound_heap_info = (VkDeviceAddress*)buffer_range.offset_mapped_ptr;

        // If these are zero that is valid, core validation gives error if heap is not set when needed
        DescriptorChecksHeapCbState* dc_cb_state = cb.shared_resources_cache.TryGet<DescriptorChecksHeapCbState>();
        if (dc_cb_state) {
            bound_heap_info[0] = dc_cb_state->last_bound_heap_info_resource.offset_address;
            bound_heap_info[1] = dc_cb_state->last_bound_heap_info_sampler.offset_address;
        }

        uint8_t* gpu_push_data_ptr = ((uint8_t*)buffer_range.offset_mapped_ptr) + bound_heap_info_size;
        memcpy(gpu_push_data_ptr, cb.push_data_value.data(), cb.push_data_value.size());

        out_update.buffer = buffer_range.buffer;
        out_update.offset = buffer_range.offset;
        out_update.range = buffer_range.size;
        out_update.address = buffer_range.offset_address;
        out_update.binding = glsl::kBindingInstDescriptorHeap;
    });
}

}  // namespace gpuav
