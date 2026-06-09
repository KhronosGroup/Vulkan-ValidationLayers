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

namespace gpuav {

void RegisterDescriptorChecksHeapValidation(Validator& gpuav, CommandBufferSubState& cb) {
    if (!gpuav.gpuav_settings.shader_instrumentation.descriptor_checks) {
        return;
    }
    if (!gpuav.enabled_features.descriptorHeap) {
        return;
    }

    // TODO - See NegativeGpuAVDescriptorHeap.ResourceOOBRebindHeap
    // This will provide false positive, instead of capturing the CommandBufferSubState here, we need to take snapshots of the last
    // bound data, which in practice apps should only being binding once per command buffers.
    cb.on_instrumentation_error_logger_register_functions.emplace_back([](Validator& gpuav, CommandBufferSubState& cb,
                                                                          const LastBound& last_bound) {
        CommandBufferSubState::InstrumentationErrorLogger inst_error_logger =
            [&cb](Validator& gpuav, const Location& loc, const uint32_t* error_record, const InstrumentedShader*,
                  std::string& out_error_msg, std::string& out_vuid_msg) {
                using namespace glsl;

                bool error_found = false;
                if (GetErrorGroup(error_record) != kErrorGroup_InstDescriptorHeap) {
                    return error_found;
                }
                error_found = true;

                std::ostringstream strm;

                const uint32_t index = error_record[kInst_LogError_ParameterOffset_0];
                const uint32_t offset = error_record[kInst_LogError_ParameterOffset_1];

                const uint32_t alignment_encoding = error_record[kInst_LogError_ParameterOffset_2];
                const uint32_t desc_type =
                    (alignment_encoding & kInst_DescriptorHeap_DescriptorTypeMask) >> kInst_DescriptorHeap_DescriptorTypeShift;
                const uint32_t alignment_value = alignment_encoding & kInst_DescriptorHeap_AlignmentValueMask;

                const bool is_buffer = desc_type & gpuav::descriptor::TYPE_BUFFER_MASK;
                const bool is_image = desc_type & gpuav::descriptor::TYPE_IMAGE_MASK;
                const bool is_sampler = desc_type == gpuav::descriptor::TYPE_SAMPLER;

                VkDeviceAddress heap_address =
                    (is_sampler ? cb.base.descriptor_heap.sampler_range.begin : cb.base.descriptor_heap.resource_range.begin) +
                    offset;

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
                        strm << "Index " << index << " is accessing the heap at offset 0x" << std::hex << offset << " (address 0x"
                             << std::hex << heap_address
                             << ") which is inside the reserved range.\nThe reserved range was bound at ";
                        if (is_sampler) {
                            strm << string_range_hex(cb.base.descriptor_heap.sampler_reserved) << " (size of 0x" << std::hex
                                 << cb.base.descriptor_heap.sampler_reserved.size() << " bytes)\n";
                        } else {
                            strm << string_range_hex(cb.base.descriptor_heap.resource_reserved) << " (size of 0x" << std::hex
                                 << cb.base.descriptor_heap.resource_reserved.size() << " bytes)\n";
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
                    if (!cb.base.descriptor_heap.sampler_bound) {
                        strm << "not bound with vkCmdBindSamplerHeapEXT";
                    } else {
                        strm << "bound at " << string_range_hex(cb.base.descriptor_heap.sampler_range) << " (size of 0x" << std::hex
                             << cb.base.descriptor_heap.sampler_range.size() << " bytes)\n";
                    }
                } else {
                    if (!cb.base.descriptor_heap.resource_bound) {
                        strm << "not bound with vkCmdBindResourceHeapEXT";
                    } else {
                        strm << "bound at " << string_range_hex(cb.base.descriptor_heap.resource_range) << " (size of 0x"
                             << std::hex << cb.base.descriptor_heap.resource_range.size() << " bytes)\n";
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
        const uint32_t bound_heap_info_size = sizeof(glsl::BoundHeapInfo) * 2;
        uint32_t buffer_size = bound_heap_info_size + (uint32_t)gpuav.phys_dev_ext_props.descriptor_heap_props.maxPushDataSize;
        vko::BufferRange buffer_range = cb.gpu_resources_manager.GetHostCoherentBufferRange(buffer_size);

        glsl::BoundHeapInfo* bound_heap_info = (glsl::BoundHeapInfo*)buffer_range.offset_mapped_ptr;
        glsl::BoundHeapInfo& bound_resource_heap = bound_heap_info[0];
        glsl::BoundHeapInfo& bound_sampler_heap = bound_heap_info[1];
        auto& cb_heap = cb.base.descriptor_heap;
        assert(cb_heap.resource_range.size() < (VkDeviceAddress)vvl::kU32Max);
        assert(cb_heap.sampler_range.size() < (VkDeviceAddress)vvl::kU32Max);
        bound_resource_heap.heap_size = (uint32_t)cb_heap.resource_range.size();
        bound_resource_heap.reserved_begin = (uint32_t)(cb_heap.resource_reserved.begin - cb_heap.resource_range.begin);
        bound_resource_heap.reserved_end = (uint32_t)(cb_heap.resource_reserved.end - cb_heap.resource_range.begin);
        bound_sampler_heap.heap_size = (uint32_t)cb_heap.sampler_range.size();
        bound_sampler_heap.reserved_begin = (uint32_t)(cb_heap.sampler_reserved.begin - cb_heap.sampler_range.begin);
        bound_sampler_heap.reserved_end = (uint32_t)(cb_heap.sampler_reserved.end - cb_heap.sampler_range.begin);

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
