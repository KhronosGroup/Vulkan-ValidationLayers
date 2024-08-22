/* Copyright (c) 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2020-2024 Valve Corporation
 * Copyright (c) 2020-2024 LunarG, Inc.
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

#include "gpu/instrumentation/gpuav_instrumentation.h"

#include "chassis/chassis_modification_state.h"
#include "gpu/core/gpuav.h"
#include "gpu/error_message/gpuav_vuids.h"
#include "gpu/resources/gpu_shader_resources.h"
#include "gpu/shaders/gpu_error_header.h"

namespace gpuav {

void SetupShaderInstrumentationResources(Validator &gpuav, CommandBuffer &cb_state, VkPipelineBindPoint bind_point,
                                         const Location &loc) {
    if (!gpuav.gpuav_settings.shader_instrumentation_enabled) {
        return;
    }

    assert(bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS || bind_point == VK_PIPELINE_BIND_POINT_COMPUTE ||
           bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);

    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);
    auto const &last_bound = cb_state.lastBound[lv_bind_point];
    const auto *pipeline_state = last_bound.pipeline_state;

    if (!pipeline_state && !last_bound.HasShaderObjects()) {
        gpuav.InternalError(cb_state.VkHandle(), loc, "Neither pipeline state nor shader object states were found.");
        return;
    }

    VkDescriptorSet instrumentation_desc_set =
        cb_state.gpu_resources_manager.GetManagedDescriptorSet(cb_state.GetInstrumentationDescriptorSetLayout());
    if (!instrumentation_desc_set) {
        gpuav.InternalError(cb_state.VkHandle(), loc, "Unable to allocate instrumentation descriptor sets.");
        return;
    }

    // Update instrumentation descriptor set
    {
        // Pathetic way of trying to make sure we take care of updating all
        // bindings of the instrumentation descriptor set
        assert(gpuav.instrumentation_bindings_.size() == 6);
        std::vector<VkWriteDescriptorSet> desc_writes = {};

        // Error output buffer
        VkDescriptorBufferInfo error_output_desc_buffer_info = {};
        {
            error_output_desc_buffer_info.range = VK_WHOLE_SIZE;
            error_output_desc_buffer_info.buffer = cb_state.GetErrorOutputBuffer();
            error_output_desc_buffer_info.offset = 0;

            VkWriteDescriptorSet wds = vku::InitStructHelper();
            wds.dstBinding = glsl::kBindingInstErrorBuffer;
            wds.descriptorCount = 1;
            wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            wds.pBufferInfo = &error_output_desc_buffer_info;
            wds.dstSet = instrumentation_desc_set;
            desc_writes.emplace_back(wds);
        }

        // Buffer holding action command index in command buffer
        VkDescriptorBufferInfo indices_desc_buffer_info = {};
        {
            indices_desc_buffer_info.range = sizeof(uint32_t);
            indices_desc_buffer_info.buffer = gpuav.indices_buffer_.buffer;
            indices_desc_buffer_info.offset = 0;

            VkWriteDescriptorSet wds = vku::InitStructHelper();
            wds.dstBinding = glsl::kBindingInstActionIndex;
            wds.descriptorCount = 1;
            wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            wds.pBufferInfo = &indices_desc_buffer_info;
            wds.dstSet = instrumentation_desc_set;
            desc_writes.emplace_back(wds);
        }

        // Buffer holding a resource index from the per command buffer command resources list
        {
            VkWriteDescriptorSet wds = vku::InitStructHelper();
            wds.dstBinding = glsl::kBindingInstCmdResourceIndex;
            wds.descriptorCount = 1;
            wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            wds.pBufferInfo = &indices_desc_buffer_info;
            wds.dstSet = instrumentation_desc_set;
            desc_writes.emplace_back(wds);
        }

        // Errors count buffer
        VkDescriptorBufferInfo cmd_errors_counts_desc_buffer_info = {};
        {
            cmd_errors_counts_desc_buffer_info.range = VK_WHOLE_SIZE;
            cmd_errors_counts_desc_buffer_info.buffer = cb_state.GetCmdErrorsCountsBuffer();
            cmd_errors_counts_desc_buffer_info.offset = 0;

            VkWriteDescriptorSet wds = vku::InitStructHelper();
            wds.dstBinding = glsl::kBindingInstCmdErrorsCount;
            wds.descriptorCount = 1;
            wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            wds.pBufferInfo = &cmd_errors_counts_desc_buffer_info;
            wds.dstSet = instrumentation_desc_set;
            desc_writes.emplace_back(wds);
        }

        // Current bindless buffer
        VkDescriptorBufferInfo di_input_desc_buffer_info = {};
        if (cb_state.current_bindless_buffer != VK_NULL_HANDLE) {
            di_input_desc_buffer_info.range = VK_WHOLE_SIZE;
            di_input_desc_buffer_info.buffer = cb_state.current_bindless_buffer;
            di_input_desc_buffer_info.offset = 0;

            VkWriteDescriptorSet wds = vku::InitStructHelper();
            wds.dstBinding = glsl::kBindingInstBindlessDescriptor;
            wds.descriptorCount = 1;
            wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            wds.pBufferInfo = &di_input_desc_buffer_info;
            wds.dstSet = instrumentation_desc_set;
            desc_writes.emplace_back(wds);
        }

        // BDA snapshot buffer
        VkDescriptorBufferInfo bda_input_desc_buffer_info = {};
        if (gpuav.gpuav_settings.shader_instrumentation.buffer_device_address) {
            bda_input_desc_buffer_info.range = VK_WHOLE_SIZE;
            bda_input_desc_buffer_info.buffer = cb_state.GetBdaRangesSnapshot().buffer;
            bda_input_desc_buffer_info.offset = 0;

            VkWriteDescriptorSet wds = vku::InitStructHelper();
            wds.dstBinding = glsl::kBindingInstBufferDeviceAddress;
            wds.descriptorCount = 1;
            wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            wds.pBufferInfo = &bda_input_desc_buffer_info;
            wds.dstSet = instrumentation_desc_set;
            desc_writes.emplace_back(wds);
        }

        DispatchUpdateDescriptorSets(gpuav.device, static_cast<uint32_t>(desc_writes.size()), desc_writes.data(), 0, nullptr);
    }

    auto pipeline_layout = pipeline_state ? pipeline_state->PipelineLayoutState()
                                          : gpuav.Get<vvl::PipelineLayout>(last_bound.desc_set_pipeline_layout);
    // If GPL is used, it's possible the pipeline layout used at pipeline creation time is null. If CmdBindDescriptorSets has
    // not been called yet (i.e., state.pipeline_null), then fall back to the layout associated with pre-raster state,
    // or the last specified pipeline layout in vkCmdPushConstantRanges.
    // PipelineLayoutState should be used for the purposes of determining the number of sets in the layout, but this layout
    // may be a "pseudo layout" used to represent the union of pre-raster and fragment shader layouts, and therefore have a
    // null handle.
    VkPipelineLayout pipeline_layout_handle = VK_NULL_HANDLE;
    if (last_bound.desc_set_pipeline_layout) {
        pipeline_layout_handle = last_bound.desc_set_pipeline_layout;
    } else if (pipeline_state && !pipeline_state->PreRasterPipelineLayoutState()->Destroyed()) {
        pipeline_layout_handle = pipeline_state->PreRasterPipelineLayoutState()->VkHandle();
    } else if (cb_state.push_constant_latest_used_layout[lv_bind_point] != VK_NULL_HANDLE) {
        pipeline_layout_handle = cb_state.push_constant_latest_used_layout[lv_bind_point];
        pipeline_layout = gpuav.Get<vvl::PipelineLayout>(pipeline_layout_handle);
    }

    uint32_t operation_index = 0;
    if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS)
        operation_index = cb_state.draw_index++;
    else if (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE)
        operation_index = cb_state.compute_index++;
    else if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)
        operation_index = cb_state.trace_rays_index++;

    // TODO: Using cb_state.per_command_resources.size() is kind of a hack? Worth considering passing the resource index as a
    // parameter
    const uint32_t error_logger_i = static_cast<uint32_t>(cb_state.per_command_error_loggers.size());
    const std::array<uint32_t, 2> dynamic_offsets = {
        {operation_index * gpuav.indices_buffer_alignment_, error_logger_i * gpuav.indices_buffer_alignment_}};
    if (pipeline_layout && pipeline_layout_handle != VK_NULL_HANDLE) {
        // If we were unable to use the layout because it overlaps with the instrumented set, don't dispatch anything or we will
        // disturb the original bound set
        if (pipeline_layout->set_layouts.size() <= gpuav.desc_set_bind_index_) {
            DispatchCmdBindDescriptorSets(cb_state.VkHandle(), bind_point, pipeline_layout_handle, gpuav.desc_set_bind_index_, 1,
                                          &instrumentation_desc_set, static_cast<uint32_t>(dynamic_offsets.size()),
                                          dynamic_offsets.data());
        } else {
            // Cannot setup instrumentation descriptor set, abort for this command
            return;
        }
    } else {
        // If no pipeline layout was bound when using shader objects that don't use any descriptor set, and no push constants, bind
        // the debug pipeline layout
        DispatchCmdBindDescriptorSets(cb_state.VkHandle(), bind_point, gpuav.GetDebugPipelineLayout(), gpuav.desc_set_bind_index_,
                                      1, &instrumentation_desc_set, static_cast<uint32_t>(dynamic_offsets.size()),
                                      dynamic_offsets.data());
    }

    if (pipeline_state && pipeline_layout_handle == VK_NULL_HANDLE) {
        gpuav.InternalError(cb_state.Handle(), loc, "Unable to find pipeline layout to bind debug descriptor set");
        return;
    }

    // It is possible to have no descriptor sets bound, for example if using push constants.
    const uint32_t desc_binding_index =
        !cb_state.di_input_buffer_list.empty() ? uint32_t(cb_state.di_input_buffer_list.size()) - 1 : vvl::kU32Max;

    const bool uses_robustness = (gpuav.enabled_features.robustBufferAccess || gpuav.enabled_features.robustBufferAccess2 ||
                                  (pipeline_state && pipeline_state->uses_pipeline_robustness));

    CommandBuffer::ErrorLoggerFunc error_logger = [loc, desc_binding_index, desc_binding_list = &cb_state.di_input_buffer_list,
                                                   cb_state_handle = cb_state.VkHandle(), bind_point, operation_index,
                                                   uses_shader_object = pipeline_state == nullptr,
                                                   uses_robustness](Validator &gpuav, const uint32_t *error_record,
                                                                    const LogObjectList &objlist) {
        bool skip = false;

        const DescBindingInfo *di_info = desc_binding_index != vvl::kU32Max ? &(*desc_binding_list)[desc_binding_index] : nullptr;
        skip |= LogInstrumentationError(gpuav, cb_state_handle, objlist, operation_index, error_record,
                                        di_info ? di_info->descriptor_set_buffers : std::vector<DescSetState>(), bind_point,
                                        uses_shader_object, uses_robustness, loc);
        return skip;
    };

    cb_state.per_command_error_loggers.emplace_back(error_logger);
}

bool LogMessageInstBindlessDescriptor(Validator &gpuav, const uint32_t *error_record, std::string &out_error_msg,
                                      std::string &out_vuid_msg, const std::vector<DescSetState> &descriptor_sets,
                                      const Location &loc, bool uses_shader_object, bool &out_oob_access) {
    using namespace glsl;
    bool error_found = true;
    std::ostringstream strm;
    const GpuVuid &vuid = GetGpuVuid(loc.function);

    switch (error_record[kHeaderErrorSubCodeOffset]) {
        case kErrorSubCodeBindlessDescriptorBounds: {
            strm << "(set = " << error_record[kInstBindlessBoundsDescSetOffset]
                 << ", binding = " << error_record[kInstBindlessBoundsDescBindingOffset] << ") Index of "
                 << error_record[kInstBindlessBoundsDescIndexOffset] << " used to index descriptor array of length "
                 << error_record[kInstBindlessBoundsDescBoundOffset] << ".";
            out_vuid_msg = "UNASSIGNED-Descriptor index out of bounds";
            error_found = true;
        } break;
        case kErrorSubCodeBindlessDescriptorUninit: {
            strm << "(set = " << error_record[kInstBindlessUninitDescSetOffset]
                 << ", binding = " << error_record[kInstBindlessUninitBindingOffset] << ") Descriptor index "
                 << error_record[kInstBindlessUninitDescIndexOffset] << " is uninitialized.";
            out_vuid_msg = vuid.invalid_descriptor;
            error_found = true;
        } break;
        case kErrorSubCodeBindlessDescriptorDestroyed: {
            strm << "(set = " << error_record[kInstBindlessUninitDescSetOffset]
                 << ", binding = " << error_record[kInstBindlessUninitBindingOffset] << ") Descriptor index "
                 << error_record[kInstBindlessUninitDescIndexOffset] << " references a resource that was destroyed.";
            out_vuid_msg = "UNASSIGNED-Descriptor destroyed";
            error_found = true;
        } break;
        case kErrorSubCodeBindlessDescriptorNullPointer: {
            strm << "(set = " << error_record[kInstBindlessUninitDescSetOffset]
                 << ", binding = " << error_record[kInstBindlessUninitBindingOffset] << ") Descriptor index "
                 << error_record[kInstBindlessUninitDescIndexOffset] << " had error getting pointer to descriptor.";
            out_vuid_msg = "UNASSIGNED-Descriptor Null Pointer";
            error_found = true;
        } break;
        case kErrorSubCodeBindlessDescriptorOOB: {
            const uint32_t set_num = error_record[kInstBindlessBuffOOBDescSetOffset];
            const uint32_t binding_num = error_record[kInstBindlessBuffOOBDescBindingOffset];
            const uint32_t desc_index = error_record[kInstBindlessBuffOOBDescIndexOffset];
            const uint32_t size = error_record[kInstBindlessBuffOOBBuffSizeOffset];
            const uint32_t offset = error_record[kInstBindlessBuffOOBBuffOffOffset];
            const auto *binding_state = descriptor_sets[set_num].state->GetBinding(binding_num);
            assert(binding_state);
            if (size == 0) {
                strm << "(set = " << set_num << ", binding = " << binding_num << ") Descriptor index " << desc_index
                     << " is uninitialized.";
                out_vuid_msg = vuid.invalid_descriptor;
                error_found = true;
                break;
            }
            out_oob_access = true;
            auto desc_class = binding_state->descriptor_class;
            if (desc_class == vvl::DescriptorClass::Mutable) {
                desc_class = static_cast<const vvl::MutableBinding *>(binding_state)->descriptors[desc_index].ActiveClass();
            }

            switch (desc_class) {
                case vvl::DescriptorClass::GeneralBuffer: {
                    const vvl::Buffer *buffer_state =
                        static_cast<const vvl::BufferBinding *>(binding_state)->descriptors[desc_index].GetBufferState();
                    assert(buffer_state);

                    strm << "(set = " << set_num << ", binding = " << binding_num << ") Descriptor index " << desc_index
                         << " access out of bounds. The descriptor buffer (" << gpuav.FormatHandle(buffer_state->Handle())
                         << ") size is " << buffer_state->create_info.size << " bytes, " << size
                         << " bytes were bound, and the highest out of bounds access was at [" << offset << "] bytes";
                    if (binding_state->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                        binding_state->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
                        out_vuid_msg = uses_shader_object ? vuid.uniform_access_oob_08612 : vuid.uniform_access_oob_06935;
                    } else {
                        out_vuid_msg = uses_shader_object ? vuid.storage_access_oob_08613 : vuid.storage_access_oob_06936;
                    }
                    error_found = true;
                    break;
                }
                case vvl::DescriptorClass::TexelBuffer: {
                    const vvl::BufferView *buffer_view_state =
                        static_cast<const vvl::TexelBinding *>(binding_state)->descriptors[desc_index].GetBufferViewState();
                    strm << "(set = " << set_num << ", binding = " << binding_num << ") Descriptor index " << desc_index
                         << " access out of bounds. The descriptor texel buffer ("
                         << gpuav.FormatHandle(buffer_view_state->Handle()) << ") size is " << size
                         << " texels and highest out of bounds access was at [" << offset << "]";
                    if (binding_state->type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) {
                        out_vuid_msg = uses_shader_object ? vuid.uniform_access_oob_08612 : vuid.uniform_access_oob_06935;
                    } else {
                        out_vuid_msg = uses_shader_object ? vuid.storage_access_oob_08613 : vuid.storage_access_oob_06936;
                    }
                    error_found = true;
                    break;
                }
                default:
                    // other OOB checks are not implemented yet
                    assert(false);
            }
        } break;
    }
    out_error_msg = strm.str();
    return error_found;
}

bool LogMessageInstNonBindlessOOB(Validator &gpuav, const uint32_t *error_record, std::string &out_error_msg,
                                  std::string &out_vuid_msg, const std::vector<DescSetState> &descriptor_sets, const Location &loc,
                                  bool uses_shader_object, bool &out_oob_access) {
    using namespace glsl;
    bool error_found = true;
    out_oob_access = true;
    std::ostringstream strm;
    const GpuVuid &vuid = GetGpuVuid(loc.function);

    const uint32_t set_num = error_record[kInstNonBindlessOOBDescSetOffset];
    const uint32_t binding_num = error_record[kInstNonBindlessOOBDescBindingOffset];
    const uint32_t desc_index = error_record[kInstNonBindlessOOBDescIndexOffset];

    strm << "(set = " << set_num << ", binding = " << binding_num << ", index " << desc_index << ") ";
    switch (error_record[kHeaderErrorSubCodeOffset]) {
        case kErrorSubCodeNonBindlessOOBBufferArrays: {
            const uint32_t desc_array_size = error_record[kInstNonBindlessOOBParamOffset0];
            strm << " access out of bounds. The descriptor buffer array is " << desc_array_size
                 << " large, but as accessed at index [" << desc_index << "]";
            out_vuid_msg = "UNASSIGNED-Descriptor Buffer index out of bounds";
        } break;

        case kErrorSubCodeNonBindlessOOBBufferBounds: {
            const auto *binding_state = descriptor_sets[set_num].state->GetBinding(binding_num);
            const vvl::Buffer *buffer_state =
                static_cast<const vvl::BufferBinding *>(binding_state)->descriptors[desc_index].GetBufferState();
            assert(buffer_state);
            const uint32_t byte_offset = error_record[kInstNonBindlessOOBParamOffset0];
            const uint32_t resource_size = error_record[kInstNonBindlessOOBParamOffset1];

            strm << " access out of bounds. The descriptor buffer (" << gpuav.FormatHandle(buffer_state->Handle()) << ") size is "
                 << buffer_state->create_info.size << " bytes, " << resource_size
                 << " bytes were bound, and the highest out of bounds access was at [" << byte_offset << "] bytes";

            if (binding_state->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                binding_state->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
                out_vuid_msg = uses_shader_object ? vuid.uniform_access_oob_08612 : vuid.uniform_access_oob_06935;
            } else {
                out_vuid_msg = uses_shader_object ? vuid.storage_access_oob_08613 : vuid.storage_access_oob_06936;
            }
        } break;

        case kErrorSubCodeNonBindlessOOBTexelBufferArrays: {
            const uint32_t desc_array_size = error_record[kInstNonBindlessOOBParamOffset0];
            strm << " access out of bounds. The descriptor texel buffer array is " << desc_array_size
                 << " large, but as accessed at index [" << desc_index << "]";
            out_vuid_msg = "UNASSIGNED-Descriptor Texel Buffer index out of bounds";
        } break;

        case kErrorSubCodeNonBindlessOOBTexelBufferBounds: {
            const auto *binding_state = descriptor_sets[set_num].state->GetBinding(binding_num);
            const vvl::BufferView *buffer_view_state =
                static_cast<const vvl::TexelBinding *>(binding_state)->descriptors[desc_index].GetBufferViewState();
            assert(buffer_view_state);
            const uint32_t byte_offset = error_record[kInstNonBindlessOOBParamOffset0];
            const uint32_t resource_size = error_record[kInstNonBindlessOOBParamOffset1];

            strm << " access out of bounds. The descriptor texel buffer (" << gpuav.FormatHandle(buffer_view_state->Handle())
                 << ") size is " << resource_size << " texels and the highest out of bounds access was at [" << byte_offset
                 << "] bytes";

            // https://gitlab.khronos.org/vulkan/vulkan/-/issues/3977
            out_vuid_msg = "UNASSIGNED-Descriptor Texel Buffer texel out of bounds";
        } break;

        default:
            error_found = false;
            out_oob_access = false;
            assert(false);  // other OOB checks are not implemented yet
    }

    out_error_msg = strm.str();
    return error_found;
}

bool LogMessageInstBufferDeviceAddress(const uint32_t *error_record, std::string &out_error_msg, std::string &out_vuid_msg,
                                       bool &out_oob_access) {
    using namespace glsl;
    bool error_found = true;
    std::ostringstream strm;
    switch (error_record[kHeaderErrorSubCodeOffset]) {
        case kErrorSubCodeBufferDeviceAddressUnallocRef: {
            out_oob_access = true;
            const char *access_type = error_record[kInstBuffAddrAccessInstructionOffset] == spv::OpStore ? "written" : "read";
            uint64_t address = *reinterpret_cast<const uint64_t *>(error_record + kInstBuffAddrUnallocDescPtrLoOffset);
            strm << "Out of bounds access: " << error_record[kInstBuffAddrAccessByteSizeOffset] << " bytes " << access_type
                 << " at buffer device address 0x" << std::hex << address << '.';
            out_vuid_msg = "UNASSIGNED-Device address out of bounds";
        } break;
        default:
            error_found = false;
            break;
    }
    out_error_msg = strm.str();
    return error_found;
}

bool LogMessageInstRayQuery(const uint32_t *error_record, std::string &out_error_msg, std::string &out_vuid_msg) {
    using namespace glsl;
    bool error_found = true;
    std::ostringstream strm;
    switch (error_record[kHeaderErrorSubCodeOffset]) {
        case kErrorSubCodeRayQueryNegativeMin: {
            // TODO - Figure a way to properly use GLSL floatBitsToUint and print the float values
            strm << "OpRayQueryInitializeKHR operand Ray Tmin value is negative. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06349";
        } break;
        case kErrorSubCodeRayQueryNegativeMax: {
            strm << "OpRayQueryInitializeKHR operand Ray Tmax value is negative. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06349";
        } break;
        case kErrorSubCodeRayQueryMinMax: {
            strm << "OpRayQueryInitializeKHR operand Ray Tmax is less than RayTmin. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06350";
        } break;
        case kErrorSubCodeRayQueryMinNaN: {
            strm << "OpRayQueryInitializeKHR operand Ray Tmin is NaN. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06351";
        } break;
        case kErrorSubCodeRayQueryMaxNaN: {
            strm << "OpRayQueryInitializeKHR operand Ray Tmax is NaN. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06351";
        } break;
        case kErrorSubCodeRayQueryOriginNaN: {
            strm << "OpRayQueryInitializeKHR operand Ray Origin contains a NaN. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06351";
        } break;
        case kErrorSubCodeRayQueryDirectionNaN: {
            strm << "OpRayQueryInitializeKHR operand Ray Direction contains a NaN. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06351";
        } break;
        case kErrorSubCodeRayQueryOriginFinite: {
            strm << "OpRayQueryInitializeKHR operand Ray Origin contains a non-finite value. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06348";
        } break;
        case kErrorSubCodeRayQueryDirectionFinite: {
            strm << "OpRayQueryInitializeKHR operand Ray Direction contains a non-finite value. ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06348";
        } break;
        case kErrorSubCodeRayQueryBothSkip: {
            const uint32_t value = error_record[kInstRayQueryParamOffset_0];
            strm << "OpRayQueryInitializeKHR operand Ray Flags is 0x" << std::hex << value << ". ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06889";
        } break;
        case kErrorSubCodeRayQuerySkipCull: {
            const uint32_t value = error_record[kInstRayQueryParamOffset_0];
            strm << "OpRayQueryInitializeKHR operand Ray Flags is 0x" << std::hex << value << ". ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06890";
        } break;
        case kErrorSubCodeRayQueryOpaque: {
            const uint32_t value = error_record[kInstRayQueryParamOffset_0];
            strm << "OpRayQueryInitializeKHR operand Ray Flags is 0x" << std::hex << value << ". ";
            out_vuid_msg = "VUID-RuntimeSpirv-OpRayQueryInitializeKHR-06891";
        } break;
        default:
            error_found = false;
            break;
    }
    out_error_msg = strm.str();
    return error_found;
}

// Pull together all the information from the debug record to build the error message strings,
// and then assemble them into a single message string.
// Retrieve the shader program referenced by the unique shader ID provided in the debug record.
// We had to keep a copy of the shader program with the same lifecycle as the pipeline to make
// sure it is available when the pipeline is submitted.  (The ShaderModule tracking object also
// keeps a copy, but it can be destroyed after the pipeline is created and before it is submitted.)
//
bool LogInstrumentationError(Validator &gpuav, VkCommandBuffer cmd_buffer, const LogObjectList &objlist, uint32_t operation_index,
                             const uint32_t *error_record, const std::vector<DescSetState> &descriptor_sets,
                             VkPipelineBindPoint pipeline_bind_point, bool uses_shader_object, bool uses_robustness,
                             const Location &loc) {
    // The second word in the debug output buffer is the number of words that would have
    // been written by the shader instrumentation, if there was enough room in the buffer we provided.
    // The number of words actually written by the shaders is determined by the size of the buffer
    // we provide via the descriptor. So, we process only the number of words that can fit in the
    // buffer.
    // Each "report" written by the shader instrumentation is considered a "record". This function
    // is hard-coded to process only one record because it expects the buffer to be large enough to
    // hold only one record. If there is a desire to process more than one record, this function needs
    // to be modified to loop over records and the buffer size increased.

    std::string error_msg;
    std::string vuid_msg;
    bool oob_access = false;
    bool error_found = false;
    switch (error_record[glsl::kHeaderErrorGroupOffset]) {
        case glsl::kErrorGroupInstBindlessDescriptor:
            error_found = LogMessageInstBindlessDescriptor(gpuav, error_record, error_msg, vuid_msg, descriptor_sets, loc,
                                                           uses_shader_object, oob_access);
            break;
        case glsl::kErrorGroupInstNonBindlessOOB:
            error_found = LogMessageInstNonBindlessOOB(gpuav, error_record, error_msg, vuid_msg, descriptor_sets, loc,
                                                       uses_shader_object, oob_access);
            break;
        case glsl::kErrorGroupInstBufferDeviceAddress:
            error_found = LogMessageInstBufferDeviceAddress(error_record, error_msg, vuid_msg, oob_access);
            break;
        case glsl::kErrorGroupInstRayQuery:
            error_found = LogMessageInstRayQuery(error_record, error_msg, vuid_msg);
            break;
        default:
            break;
    }

    if (error_found) {
        // Lookup the VkShaderModule handle and SPIR-V code used to create the shader, using the unique shader ID value returned
        // by the instrumented shader.
        const gpu::GpuAssistedShaderTracker *tracker_info = nullptr;
        const uint32_t shader_id = error_record[glsl::kHeaderShaderIdOffset];
        auto it = gpuav.shader_map_.find(shader_id);
        if (it != gpuav.shader_map_.end()) {
            tracker_info = &it->second;
        }

        // If we somehow can't find our state, we can still report our error message
        std::vector<gpu::spirv::Instruction> instructions;
        if (tracker_info && !tracker_info->instrumented_spirv.empty()) {
            gpu::spirv::GenerateInstructions(tracker_info->instrumented_spirv, instructions);
        }

        std::string debug_info_message = gpuav.GenerateDebugInfoMessage(
            cmd_buffer, instructions, error_record[gpuav::glsl::kHeaderStageIdOffset],
            error_record[gpuav::glsl::kHeaderStageInfoOffset_0], error_record[gpuav::glsl::kHeaderStageInfoOffset_1],
            error_record[gpuav::glsl::kHeaderStageInfoOffset_2], error_record[gpuav::glsl::kHeaderInstructionIdOffset],
            tracker_info, pipeline_bind_point, operation_index);

        if (uses_robustness && oob_access) {
            if (gpuav.gpuav_settings.warn_on_robust_oob) {
                gpuav.LogWarning(vuid_msg.c_str(), objlist, loc, "%s\n%s", error_msg.c_str(), debug_info_message.c_str());
            }
        } else {
            gpuav.LogError(vuid_msg.c_str(), objlist, loc, "%s\n%s", error_msg.c_str(), debug_info_message.c_str());
        }
    }

    return error_found;
}

}  // namespace gpuav
