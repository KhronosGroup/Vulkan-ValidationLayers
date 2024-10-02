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
#include "state_tracker/shader_object_state.h"

namespace gpuav {

// If application is using shader objects, bindings count will be computed from bound shaders
static uint32_t GetBindingsCountFromLastBoundPipelineOrShader(Validator &gpuav, VkPipelineBindPoint bind_point,
                                                              CommandBuffer &cb_state, const LastBound &last_bound) {
    if (last_bound.pipeline_state && last_bound.pipeline_state->PreRasterPipelineLayoutState()) {
        return static_cast<uint32_t>(last_bound.pipeline_state->PreRasterPipelineLayoutState()->set_layouts.size());
    }

    if (const vvl::ShaderObject *main_bound_shader = last_bound.GetFirstShader(bind_point)) {
        return static_cast<uint32_t>(main_bound_shader->set_layouts.size());
    }

    // Should not get there, it would mean no pipeline nor shader object was bound
    assert(false);
    return 0;
}

static VkPipelineLayout CreateInstrumentationPipelineLayout(Validator &gpuav, VkPipelineBindPoint bind_point, const Location &loc,
                                                            const LastBound &last_bound,
                                                            VkDescriptorSetLayout dummy_desc_set_layout,
                                                            VkDescriptorSetLayout instrumentation_desc_set_layout,
                                                            uint32_t inst_desc_set_binding) {
    // If not using shader objects, GPU-AV should be able to retrieve a pipeline layout from last bound pipeline
    VkPipelineLayoutCreateInfo pipe_layout_ci = vku::InitStructHelper();
    std::shared_ptr<const vvl::PipelineLayout> last_bound_pipeline_pipe_layout;
    if (last_bound.pipeline_state && last_bound.pipeline_state->PreRasterPipelineLayoutState()) {
        last_bound_pipeline_pipe_layout = last_bound.pipeline_state->PreRasterPipelineLayoutState();
    }
    if (last_bound_pipeline_pipe_layout) {
        // Application is using classic pipelines, compose a pipeline layout from last bound pipeline
        // ---
        pipe_layout_ci.flags = last_bound_pipeline_pipe_layout->create_flags;
        std::vector<VkPushConstantRange> ranges;
        if (last_bound_pipeline_pipe_layout->push_constant_ranges_layout) {
            ranges.reserve(last_bound_pipeline_pipe_layout->push_constant_ranges_layout->size());
            for (const VkPushConstantRange &range : *last_bound_pipeline_pipe_layout->push_constant_ranges_layout) {
                ranges.push_back(range);
            }
        }
        pipe_layout_ci.pushConstantRangeCount = static_cast<uint32_t>(ranges.size());
        pipe_layout_ci.pPushConstantRanges = ranges.data();
        std::vector<VkDescriptorSetLayout> set_layouts;
        set_layouts.reserve(inst_desc_set_binding + 1);
        for (const auto &set_layout : last_bound_pipeline_pipe_layout->set_layouts) {
            set_layouts.push_back(set_layout->VkHandle());
        }
        for (uint32_t set_i = static_cast<uint32_t>(last_bound_pipeline_pipe_layout->set_layouts.size());
             set_i < inst_desc_set_binding; ++set_i) {
            set_layouts.push_back(dummy_desc_set_layout);
        }
        set_layouts.push_back(instrumentation_desc_set_layout);
        pipe_layout_ci.setLayoutCount = static_cast<uint32_t>(set_layouts.size());
        pipe_layout_ci.pSetLayouts = set_layouts.data();
        VkPipelineLayout pipe_layout_handle;
        VkResult result = DispatchCreatePipelineLayout(gpuav.device, &pipe_layout_ci, VK_NULL_HANDLE, &pipe_layout_handle);
        if (result != VK_SUCCESS) {
            gpuav.InternalError(gpuav.device, loc, "Failed to create instrumentation pipeline layout");
            return VK_NULL_HANDLE;
        }

        return pipe_layout_handle;
    } else {
        // Application is using shader objects, compose a pipeline layout from bound shaders
        // ---

        const vvl::ShaderObject *main_bound_shader = last_bound.GetFirstShader(bind_point);
        if (!main_bound_shader) {
            // Should not get there, it would mean no pipeline nor shader object was bound
            gpuav.InternalError(gpuav.device, loc, "Could not retrieve last bound computer/vertex/mesh shader");
            return VK_NULL_HANDLE;
        }

        // From those VUIDs:
        // VUID-vkCmdDraw-None-08878:
        // - All bound graphics shader objects must have been created with identical or identically defined push constant ranges
        // VUID-vkCmdDraw-None-08879
        // - All bound graphics shader objects must have been created with identical or identically defined arrays of descriptor set
        // layouts
        // => To compose a VkPipelineLayout, only need to get compute or vertex/mesh shader and look at their bindings,
        // no need to check other shaders.
        const vvl::ShaderObject::SetLayoutVector *set_layouts = &main_bound_shader->set_layouts;
        PushConstantRangesId push_constants_layouts = main_bound_shader->push_constant_ranges;

        if (last_bound.desc_set_pipeline_layout) {
            std::shared_ptr<const vvl::PipelineLayout> last_bound_desc_set_pipe_layout =
                gpuav.Get<vvl::PipelineLayout>(last_bound.desc_set_pipeline_layout);
            if (last_bound_desc_set_pipe_layout) {
                pipe_layout_ci.flags = last_bound_desc_set_pipe_layout->CreateFlags();
            }
        }
        std::vector<VkDescriptorSetLayout> set_layout_handles;
        if (set_layouts) {
            set_layout_handles.reserve(inst_desc_set_binding + 1);
            for (const auto &set_layout : *set_layouts) {
                set_layout_handles.push_back(set_layout->VkHandle());
            }
            for (uint32_t set_i = static_cast<uint32_t>(set_layouts->size()); set_i < inst_desc_set_binding; ++set_i) {
                set_layout_handles.push_back(dummy_desc_set_layout);
            }
            set_layout_handles.push_back(instrumentation_desc_set_layout);
            pipe_layout_ci.setLayoutCount = static_cast<uint32_t>(set_layout_handles.size());
            pipe_layout_ci.pSetLayouts = set_layout_handles.data();
        }

        if (push_constants_layouts) {
            pipe_layout_ci.pushConstantRangeCount = static_cast<uint32_t>(push_constants_layouts->size());
            pipe_layout_ci.pPushConstantRanges = push_constants_layouts->data();
        }
        VkPipelineLayout pipe_layout_handle;
        VkResult result = DispatchCreatePipelineLayout(gpuav.device, &pipe_layout_ci, VK_NULL_HANDLE, &pipe_layout_handle);
        if (result != VK_SUCCESS) {
            gpuav.InternalError(gpuav.device, loc, "Failed to create instrumentation pipeline layout");
            return VK_NULL_HANDLE;
        }

        return pipe_layout_handle;
    }
}

void PreCallSetupShaderInstrumentationResources(Validator &gpuav, CommandBuffer &cb_state, VkPipelineBindPoint bind_point,
                                                const Location &loc) {
    if (!gpuav.gpuav_settings.shader_instrumentation_enabled) {
        return;
    }

    assert(bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS || bind_point == VK_PIPELINE_BIND_POINT_COMPUTE ||
           bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);

    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);
    auto const &last_bound = cb_state.lastBound[lv_bind_point];

    if (!last_bound.pipeline_state && !last_bound.HasShaderObjects()) {
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

    uint32_t operation_index = 0;
    if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS)
        operation_index = cb_state.draw_index++;
    else if (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE)
        operation_index = cb_state.compute_index++;
    else if (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)
        operation_index = cb_state.trace_rays_index++;

    const bool uses_shader_object = last_bound.pipeline_state == nullptr;

    // Bind instrumentation descriptor set, using an appropriate pipeline layout
    // ---

    // First find this appropriate pipeline layout.
    // Always try to grab pipeline layout from last bound pipeline. Looking at PreRasterPipelineLayoutState
    // is enough to get the layout whether the application is using standard pipelines or GPL.
    // If GPU-AV failed to get a pipeline layout this way, fall back to pipeline layout specified in last
    // vkCmdBindDescriptorSets, or in last vkCmdPushConstantRanges.

    enum class PipelineLayoutSource { NoPipelineLayout, LastBoundPipeline, LastBoundDescriptorSet, LastPushedConstants };
    std::shared_ptr<const vvl::PipelineLayout> inst_binding_pipe_layout_state;
    PipelineLayoutSource inst_binding_pipe_layout_src = PipelineLayoutSource::NoPipelineLayout;
    if (last_bound.pipeline_state && !last_bound.pipeline_state->PreRasterPipelineLayoutState()->Destroyed()) {
        inst_binding_pipe_layout_state = last_bound.pipeline_state->PreRasterPipelineLayoutState();
        inst_binding_pipe_layout_src = PipelineLayoutSource::LastBoundPipeline;

        // One exception when using GPL is we need to look out for INDEPENDENT_SETS_BIT which will have null sets inside them.
        // We have a fake merged_graphics_layout to mimic the complete layout, but the app must bind it to descriptor set
        if (inst_binding_pipe_layout_state->create_flags & VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT) {
            inst_binding_pipe_layout_state = gpuav.Get<vvl::PipelineLayout>(last_bound.desc_set_pipeline_layout);
            inst_binding_pipe_layout_src = PipelineLayoutSource::LastBoundDescriptorSet;
        }
    } else if (last_bound.desc_set_pipeline_layout) {
        inst_binding_pipe_layout_state = gpuav.Get<vvl::PipelineLayout>(last_bound.desc_set_pipeline_layout);
        inst_binding_pipe_layout_src = PipelineLayoutSource::LastBoundDescriptorSet;
    } else if (cb_state.push_constant_latest_used_layout[lv_bind_point] != VK_NULL_HANDLE) {
        inst_binding_pipe_layout_state = gpuav.Get<vvl::PipelineLayout>(cb_state.push_constant_latest_used_layout[lv_bind_point]);
        inst_binding_pipe_layout_src = PipelineLayoutSource::LastPushedConstants;
    }

    // TODO: Using cb_state.per_command_resources.size() is kind of a hack? Worth considering passing the resource index as a
    // parameter
    const uint32_t error_logger_i = static_cast<uint32_t>(cb_state.per_command_error_loggers.size());
    const std::array<uint32_t, 2> dynamic_offsets = {
        {operation_index * gpuav.indices_buffer_alignment_, error_logger_i * gpuav.indices_buffer_alignment_}};
    if (inst_binding_pipe_layout_state) {
        if (inst_binding_pipe_layout_state->set_layouts.size() <= gpuav.instrumentation_desc_set_bind_index_) {
            switch (inst_binding_pipe_layout_src) {
                case PipelineLayoutSource::NoPipelineLayout:
                    // should not get there, because inst_desc_set_binding_pipe_layout_state is not null
                    assert(false);
                    break;
                case PipelineLayoutSource::LastBoundPipeline:
                    DispatchCmdBindDescriptorSets(cb_state.VkHandle(), bind_point, inst_binding_pipe_layout_state->VkHandle(),
                                                  gpuav.instrumentation_desc_set_bind_index_, 1, &instrumentation_desc_set,
                                                  static_cast<uint32_t>(dynamic_offsets.size()), dynamic_offsets.data());
                    break;
                case PipelineLayoutSource::LastBoundDescriptorSet:
                case PipelineLayoutSource::LastPushedConstants: {
                    // Currently bound pipeline/set of shader objects may have bindings that are not compatible with last
                    // bound descriptor sets: GPU-AV may create this incompatibility by adding its empty padding descriptor sets.
                    // To alleviate that, since we could not get a pipeline layout from last pipeline binding (it was either
                    // destroyed, or never has been created if using shader objects), a pipeline layout matching bindings of last
                    // bound pipeline or
                    // last bound shader objects is created and used.
                    // If will also be cached: heuristic is next action command will likely need the same.

                    const uint32_t bindings_counts_from_last_pipeline =
                        GetBindingsCountFromLastBoundPipelineOrShader(gpuav, bind_point, cb_state, last_bound);

                    // If the number of binding of the currently bound pipeline's layout (or the equivalent for shader objects) is
                    // less that the number of bindings in the pipeline layout used to bind descriptor sets,
                    // GPU-AV needs to create a temporary pipeline layout matching the the currently bound pipeline's layout
                    // to bind the instrumentation descriptor set
                    if (bindings_counts_from_last_pipeline <
                        static_cast<uint32_t>(inst_binding_pipe_layout_state->set_layouts.size())) {
                        VkPipelineLayout instrumentation_pipe_layout = CreateInstrumentationPipelineLayout(
                            gpuav, bind_point, loc, last_bound, gpuav.dummy_desc_layout_,
                            gpuav.GetInstrumentationDescriptorSetLayout(), gpuav.instrumentation_desc_set_bind_index_);

                        if (instrumentation_pipe_layout != VK_NULL_HANDLE) {
                            DispatchCmdBindDescriptorSets(cb_state.VkHandle(), bind_point, instrumentation_pipe_layout,
                                                          gpuav.instrumentation_desc_set_bind_index_, 1, &instrumentation_desc_set,
                                                          static_cast<uint32_t>(dynamic_offsets.size()), dynamic_offsets.data());
                            DispatchDestroyPipelineLayout(gpuav.device, instrumentation_pipe_layout, nullptr);
                        } else {
                            // Could not create instrumentation pipeline layout
                            return;
                        }
                    } else {
                        // No incompatibility detected, safe to use pipeline layout for last bound descriptor set/push constants.
                        DispatchCmdBindDescriptorSets(cb_state.VkHandle(), bind_point, inst_binding_pipe_layout_state->VkHandle(),
                                                      gpuav.instrumentation_desc_set_bind_index_, 1, &instrumentation_desc_set,
                                                      static_cast<uint32_t>(dynamic_offsets.size()), dynamic_offsets.data());
                    }
                } break;
            }
        } else {
            gpuav.InternalWarning(cb_state.Handle(), loc,
                                  "Unable to bind instrumentation descriptor set, it would override application's bound set");
            return;
        }
    } else {
        // If no pipeline layout was bound when using shader objects that don't use any descriptor set, and no push constants, bind
        // the instrumentation pipeline layout
        DispatchCmdBindDescriptorSets(cb_state.VkHandle(), bind_point, gpuav.GetInstrumentationPipelineLayout(),
                                      gpuav.instrumentation_desc_set_bind_index_, 1, &instrumentation_desc_set,
                                      static_cast<uint32_t>(dynamic_offsets.size()), dynamic_offsets.data());
    }

    // It is possible to have no descriptor sets bound, for example if using push constants.
    const uint32_t desc_binding_index =
        !cb_state.di_input_buffer_list.empty() ? uint32_t(cb_state.di_input_buffer_list.size()) - 1 : vvl::kU32Max;

    const bool uses_robustness = (gpuav.enabled_features.robustBufferAccess || gpuav.enabled_features.robustBufferAccess2 ||
                                  (last_bound.pipeline_state && last_bound.pipeline_state->uses_pipeline_robustness));

    CommandBuffer::ErrorLoggerFunc error_logger = [loc, desc_binding_index, desc_binding_list = &cb_state.di_input_buffer_list,
                                                   cb_state_handle = cb_state.VkHandle(), bind_point, operation_index,
                                                   uses_shader_object,
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

void PostCallSetupShaderInstrumentationResources(Validator &gpuav, CommandBuffer &cb_state, VkPipelineBindPoint bind_point,
                                                 const Location &loc) {
    if (!gpuav.gpuav_settings.shader_instrumentation_enabled) {
        return;
    }

    assert(bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS || bind_point == VK_PIPELINE_BIND_POINT_COMPUTE ||
           bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);

    const LvlBindPoint lv_bind_point = ConvertToLvlBindPoint(bind_point);
    const LastBound &last_bound = cb_state.lastBound[lv_bind_point];

    // Only need to rebind application desc sets if they have been disturbed by GPU-AV binding its instrumentation desc set.
    // - Can happen if the pipeline layout used to bind instrumentation descriptor set is not compatible with the one used by the
    // app to bind the last/all the last desc set. This pipeline layout is referred to as "last_bound_desc_set_pipe_layout_state"
    // hereinafter.
    // => We create this incompatibility when we add our empty descriptor set.
    // See PositiveGpuAVDescriptorIndexing.SharedPipelineLayoutSubsetGraphics for instance
    if (last_bound.desc_set_pipeline_layout) {
        std::shared_ptr<const vvl::PipelineLayout> last_bound_desc_set_pipe_layout_state =
            gpuav.Get<vvl::PipelineLayout>(last_bound.desc_set_pipeline_layout);
        if (last_bound_desc_set_pipe_layout_state) {
            const uint32_t bindings_counts_from_last_pipeline =
                GetBindingsCountFromLastBoundPipelineOrShader(gpuav, bind_point, cb_state, last_bound);

            const bool any_disturbed_bindings = bindings_counts_from_last_pipeline <
                                                static_cast<uint32_t>(last_bound_desc_set_pipe_layout_state->set_layouts.size());

            if (any_disturbed_bindings) {
                const uint32_t disturbed_bindings_count = static_cast<uint32_t>(
                    last_bound_desc_set_pipe_layout_state->set_layouts.size() - bindings_counts_from_last_pipeline);
                const uint32_t first_disturbed_set = bindings_counts_from_last_pipeline;

                for (uint32_t set_i = 0; set_i < disturbed_bindings_count; ++set_i) {
                    const uint32_t last_bound_set_i = set_i + first_disturbed_set;
                    VkDescriptorSet desc_set = last_bound.per_set[last_bound_set_i].bound_descriptor_set->VkHandle();
                    const std::vector<uint32_t> &dynamic_offset = last_bound.per_set[last_bound_set_i].dynamicOffsets;
                    const uint32_t dynamic_offset_count = static_cast<uint32_t>(dynamic_offset.size());
                    DispatchCmdBindDescriptorSets(cb_state.VkHandle(), bind_point,
                                                  last_bound_desc_set_pipe_layout_state->VkHandle(), last_bound_set_i, 1, &desc_set,
                                                  dynamic_offset_count, dynamic_offset.data());
                }
            }
        }
    }
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
                 << error_record[kInstBindlessCustomOffset_0] << ".";
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
            tracker_info, shader_id, pipeline_bind_point, operation_index);

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
