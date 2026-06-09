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
#include "state_tracker/cmd_buffer_state.h"

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
        uint32_t heap_binding_index = vvl::kNoIndex32;
        DescriptorHeapBindings* desc_heap_bindings = cb.shared_resources_cache.TryGet<DescriptorHeapBindings>();
        if (desc_heap_bindings && !desc_heap_bindings->bound_heap_snapshots.empty()) {
            heap_binding_index = uint32_t(desc_heap_bindings->bound_heap_snapshots.size() - 1);
        }

        CommandBufferSubState::InstrumentationErrorLogger inst_error_logger =
            [&cb, heap_binding_index](Validator& gpuav, const Location& loc, const uint32_t* error_record,
                                      const InstrumentedShader*, std::string& out_error_msg, std::string& out_vuid_msg) {
                using namespace glsl;

                bool error_found = false;
                if (GetErrorGroup(error_record) != kErrorGroup_InstDescriptorHeap) {
                    return error_found;
                }
                error_found = true;

                std::ostringstream strm;

                // The user might have called vkCmdBindResourceHeap since recording, need to get the version at record time
                const vvl::CommandBuffer::DescriptorHeap* heap_cb_state = nullptr;
                if (heap_binding_index != vvl::kNoIndex32) {
                    const DescriptorHeapBindings& desc_heap_bindings = cb.shared_resources_cache.Get<DescriptorHeapBindings>();
                    heap_cb_state = &desc_heap_bindings.bound_heap_snapshots[heap_binding_index].heap_cb_state;
                }

                const uint32_t index = error_record[kInst_LogError_ParameterOffset_0];
                const uint32_t offset = error_record[kInst_LogError_ParameterOffset_1];

                const uint32_t alignment_encoding = error_record[kInst_LogError_ParameterOffset_2];
                const uint32_t desc_type =
                    (alignment_encoding & kInst_DescriptorHeap_DescriptorTypeMask) >> kInst_DescriptorHeap_DescriptorTypeShift;
                const uint32_t alignment_value = alignment_encoding & kInst_DescriptorHeap_AlignmentValueMask;

                const bool is_buffer = desc_type & gpuav::descriptor::TYPE_BUFFER_MASK;
                const bool is_image = desc_type & gpuav::descriptor::TYPE_IMAGE_MASK;
                const bool is_sampler = desc_type == gpuav::descriptor::TYPE_SAMPLER;

                VkDeviceAddress heap_address = 0;
                if (heap_cb_state) {
                    heap_address = (is_sampler ? heap_cb_state->sampler_range.begin : heap_cb_state->resource_range.begin) + offset;
                }

                const uint32_t error_sub_code = GetSubError(error_record);
                switch (error_sub_code) {
                    case kErrorSubCode_DescriptorHeap_HeapOOB: {
                        // TODO - Reverse info to get good error message
                        strm << "Index " << index << " is accessing the heap at offset 0x" << std::hex << offset << " (address 0x"
                             << std::hex << heap_address << ") which is OOB of the heap memory.\n";
                        out_vuid_msg = vvl::CreateActionVuid(loc.function, vvl::ActionVUID::DESCRIPTOR_HEAP_OOB_11309);
                        error_found = true;
                    } break;

                    case kErrorSubCode_DescriptorHeap_ReservedRange: {
                        assert(heap_cb_state);
                        strm << "Index " << index << " is accessing the heap at offset 0x" << std::hex << offset << " (address 0x"
                             << std::hex << heap_address
                             << ") which is inside the reserved range.\nThe reserved range was bound at ";
                        if (is_sampler) {
                            strm << string_range_hex(heap_cb_state->sampler_reserved) << " (size of 0x" << std::hex
                                 << heap_cb_state->sampler_reserved.size() << " bytes)\n";
                        } else {
                            strm << string_range_hex(heap_cb_state->resource_reserved) << " (size of 0x" << std::hex
                                 << heap_cb_state->resource_reserved.size() << " bytes)\n";
                        }
                        out_vuid_msg = vvl::CreateActionVuid(loc.function, vvl::ActionVUID::DESCRIPTOR_HEAP_OOB_11309);
                        error_found = true;
                    } break;

                    case kErrorSubCode_DescriptorHeap_DescriptorAlignment:
                    case kErrorSubCode_DescriptorHeap_DescriptorAlignmentUntyped: {
                        strm << "Index " << index << " is accessing the heap at offset 0x" << std::hex << offset << " (address 0x"
                             << std::hex << heap_address << ") which is not aligned to ";
                        if (is_buffer) {
                            strm << "bufferDescriptorAlignment";
                        } else if (is_image) {
                            strm << "imageDescriptorAlignment";
                        } else if (is_sampler) {
                            strm << "samplerDescriptorAlignment";
                        }
                        strm << " (0x" << std::hex << alignment_value << ")\n";

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
                        // TODO - Preserve pushOffset instead of saving it through the shader
                        const uint32_t push_offset = offset;  // alias param
                        // TODO - this will report the wrong pushData, see above about capturing the cb state
                        const VkDeviceAddress push_data = *((VkDeviceAddress*)&cb.push_data_value[push_offset]);

                        if (error_sub_code == kErrorSubCode_DescriptorHeap_IndirectIndexPushAlignment) {
                            strm << "Index " << index << " is ";
                        }
                        strm << "has a " << (is_sampler ? "samplerPushOffset" : "pushOffset") << " of 0x" << std::hex << push_offset
                             << " which holds the VkDeviceAddress 0x" << std::hex << push_data << " which is not a multiple of ";
                        if (error_sub_code == kErrorSubCode_DescriptorHeap_IndirectIndexPushAlignment) {
                            strm << "4 needed to access the uint32_t in the indirect buffer\n";
                        } else {
                            strm << "8 needed to access the VkDeviceAddress in the indirect buffer\n";
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

                        strm << "has a final of 0x" << std::hex << final_address << " which is not aligned to "
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

                    case kErrorSubCode_DescriptorHeap_InvalidDeviceAddress: {
                        // Quick error to catch everything
                        // TODO - sort by mapping and provide proper VUID for each
                        strm << "Try to do an indirect buffer, but the indirect address is null.\n";
                        // is 11301, 11302, 11305, 11306, and 11319 depending on the mapping
                        out_vuid_msg = "UNASSIGNED-Heap-IndirectBuffer-Null";
                        error_found = true;
                    } break;
                }

                // Unless find otherwise, this information is useful for all errors
                strm << "The " << (is_sampler ? "sampler" : "resource") << " heap was ";
                if (is_sampler) {
                    if (!heap_cb_state || !heap_cb_state->sampler_bound) {
                        strm << "not bound with vkCmdBindSamplerHeapEXT";
                    } else {
                        strm << "bound at " << string_range_hex(heap_cb_state->sampler_range) << " (size of 0x" << std::hex
                             << heap_cb_state->sampler_range.size() << " bytes)\n";
                    }
                } else {
                    if (!heap_cb_state || !heap_cb_state->resource_bound) {
                        strm << "not bound with vkCmdBindResourceHeapEXT";
                    } else {
                        strm << "bound at " << string_range_hex(heap_cb_state->resource_range) << " (size of 0x" << std::hex
                             << heap_cb_state->resource_range.size() << " bytes)\n";
                    }
                }

                out_error_msg += strm.str();
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
