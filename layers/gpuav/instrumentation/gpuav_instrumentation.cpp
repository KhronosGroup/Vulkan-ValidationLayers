/* Copyright (c) 2020-2026 The Khronos Group Inc.
 * Copyright (c) 2020-2026 Valve Corporation
 * Copyright (c) 2020-2026 LunarG, Inc.
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

#include "gpuav/instrumentation/gpuav_instrumentation.h"
#include <vulkan/vulkan_core.h>
#include <spirv/unified1/spirv.hpp>
#include <vulkan/utility/vk_struct_helper.hpp>

#include "chassis/chassis_modification_state.h"
#include "containers/small_vector.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/core/gpuav_constants.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/shaders/gpuav_error_header.h"
#include "gpuav/debug_printf/debug_printf.h"
#include "containers/limits.h"

#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/last_bound_state.h"
#include "state_tracker/shader_object_state.h"
#include "state_tracker/pipeline_state.h"
#include "state_tracker/pipeline_layout_state.h"
#include "state_tracker/descriptor_mode.h"

namespace gpuav {

enum class PipelineLayoutSource { NoPipelineLayout, LastBoundPipeline, LastBoundDescriptorSet, LastPushedConstants };

struct InstBindingPipeLayout {
    std::shared_ptr<const vvl::PipelineLayout> state;
    VkPipelineLayout handle = VK_NULL_HANDLE;
    PipelineLayoutSource source = PipelineLayoutSource::NoPipelineLayout;
};

struct CommonInstrumentationErrorInfo {
    uint32_t action_command_index = 0;
    // Used to know if from draw, dispatch, or traceRays
    VkPipelineBindPoint pipeline_bind_point = VK_PIPELINE_BIND_POINT_MAX_ENUM;
};

// If application is using shader objects, bindings count will be computed from bound shaders
static uint32_t LastBoundPipelineOrShaderDescSetBindingsCount(const LastBound &last_bound) {
    // App uses pipeline or graphics pipeline libraries
    if (last_bound.pipeline_state && last_bound.pipeline_state->PipelineLayoutState()) {
        return uint32_t(last_bound.pipeline_state->PipelineLayoutState()->set_layouts.list.size());
    }

    // App uses shader objects
    if (const vvl::ShaderObject *main_bound_shader = last_bound.GetFirstShader()) {
        return static_cast<uint32_t>(main_bound_shader->set_layouts.list.size());
    }

    // Should not get there, it would mean no pipeline nor shader object was bound
    assert(false);
    return 0;
}

// If application is using shader objects, bindings count will be computed from bound shaders
static uint32_t LastBoundPipelineOrShaderPushConstantsRangesCount(const LastBound &last_bound) {
    if (last_bound.pipeline_state && last_bound.pipeline_state->PreRasterPipelineLayoutState()) {
        return static_cast<uint32_t>(
            last_bound.pipeline_state->PreRasterPipelineLayoutState()->push_constant_ranges_layout->size());
    }

    if (const vvl::ShaderObject *main_bound_shader = last_bound.GetFirstShader()) {
        return static_cast<uint32_t>(main_bound_shader->push_constant_ranges->size());
    }

    // Should not get there, it would mean no pipeline nor shader object was bound
    assert(false);
    return 0;
}

static VkPipelineLayout CreateInstrumentationPipelineLayout(Validator &gpuav, const Location &loc, const LastBound &last_bound,
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
        for (const auto &set_layout : last_bound_pipeline_pipe_layout->set_layouts.list) {
            set_layouts.push_back(set_layout->VkHandle());
        }
        for (uint32_t set_i = static_cast<uint32_t>(last_bound_pipeline_pipe_layout->set_layouts.list.size());
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

        const vvl::ShaderObject *main_bound_shader = last_bound.GetFirstShader();
        if (!main_bound_shader) {
            // Should not get there, it would mean no pipeline nor shader object was bound
            gpuav.InternalError(gpuav.device, loc, "Could not retrieve last bound computer/vertex/mesh shader");
            return VK_NULL_HANDLE;
        }

        // From those VUIDs:
        // VUID-vkCmdDraw-None-08878
        // - All bound graphics shader objects must have been created with identical or identically defined push constant ranges
        // VUID-vkCmdDraw-None-08879
        // - All bound graphics shader objects must have been created with identical or identically defined arrays of descriptor set
        // layouts
        // => To compose a VkPipelineLayout, only need to get compute or vertex/mesh shader and look at their bindings,
        // no need to check other shaders.
        const vvl::DescriptorSetLayoutList &set_layouts = main_bound_shader->set_layouts;
        PushConstantRangesId push_constants_layouts = main_bound_shader->push_constant_ranges;

        if (last_bound.desc_set_pipeline_layout) {
            pipe_layout_ci.flags = last_bound.desc_set_pipeline_layout->CreateFlags();
        }
        std::vector<VkDescriptorSetLayout> set_layout_handles;
        {
            set_layout_handles.reserve(inst_desc_set_binding + 1);
            for (const auto &set_layout : set_layouts.list) {
                set_layout_handles.push_back(set_layout->VkHandle());
            }
            for (uint32_t set_i = static_cast<uint32_t>(set_layouts.list.size()); set_i < inst_desc_set_binding; ++set_i) {
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

void UpdateInstrumentationDescBuffer(Validator &gpuav, CommandBufferSubState &cb_state, const LastBound &last_bound,
                                     const Location &loc, CommonInstrumentationErrorInfo &out_error_info) {
    void *descriptor_start = gpuav.global_resource_descriptor_buffer_.GetMappedPtr();

    for (const auto &func : vvl::make_span(cb_state.on_instrumentation_desc_buffer_update_functions)) {
        VkDescriptorGetInfoEXT get_info = vku::InitStructHelper();
        get_info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

        VkDescriptorAddressInfoEXT address_info = vku::InitStructHelper();
        uint32_t binding = 0;

        func(cb_state, last_bound.bind_point, address_info, binding);
        get_info.data.pStorageBuffer = &address_info;

        const VkDeviceSize binding_offset = gpuav.resource_descriptor_buffer_offsets_[binding];
        uint8_t *descriptor_offset = (uint8_t *)descriptor_start + binding_offset;
        DispatchGetDescriptorEXT(gpuav.device, &get_info,
                                 gpuav.phys_dev_ext_props.descriptor_buffer_props.storageBufferDescriptorSize, descriptor_offset);
    }
}

void UpdateInstrumentationDescSet(Validator &gpuav, CommandBufferSubState &cb_state, VkPipelineBindPoint bind_point,
                                  VkDescriptorSet instrumentation_desc_set, const Location &loc,
                                  CommonInstrumentationErrorInfo &out_error_info) {
    small_vector<VkWriteDescriptorSet, 8> desc_writes = {};

    VkDescriptorBufferInfo error_output_desc_buffer_info = {};
    VkDescriptorBufferInfo indices_desc_buffer_info = {};
    VkDescriptorBufferInfo cmd_errors_counts_desc_buffer_info = {};
    if (gpuav.gpuav_settings.IsShaderInstrumentationEnabled()) {
        // Error output buffer

        {
            error_output_desc_buffer_info.buffer = cb_state.GetErrorOutputBufferRange().buffer;
            error_output_desc_buffer_info.offset = cb_state.GetErrorOutputBufferRange().offset;
            error_output_desc_buffer_info.range = cb_state.GetErrorOutputBufferRange().size;

            VkWriteDescriptorSet wds = vku::InitStructHelper();
            wds.dstBinding = glsl::kBindingInstErrorBuffer;
            wds.descriptorCount = 1;
            wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            wds.pBufferInfo = &error_output_desc_buffer_info;
            wds.dstSet = instrumentation_desc_set;
            desc_writes.emplace_back(wds);
        }

        // Buffer holding action command index in command buffer

        {
            indices_desc_buffer_info.range = sizeof(uint32_t);
            indices_desc_buffer_info.buffer = gpuav.global_indices_buffer_.VkHandle();
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
    }

    std::vector<VkDescriptorBufferInfo> buffer_infos(cb_state.on_instrumentation_desc_set_update_functions.size());
    for (const auto [func_i, func] : vvl::enumerate(cb_state.on_instrumentation_desc_set_update_functions)) {
        VkWriteDescriptorSet wds = vku::InitStructHelper();
        wds.dstSet = instrumentation_desc_set;
        wds.dstBinding = vvl::kNoIndex32;
        wds.descriptorCount = 1;
        wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        wds.pBufferInfo = &buffer_infos[func_i];

        func(cb_state, bind_point, loc, buffer_infos[func_i], wds.dstBinding);

        if (buffer_infos[func_i].buffer != VK_NULL_HANDLE) {
            assert(wds.dstBinding != vvl::kNoIndex32);
            desc_writes.emplace_back(wds);
        }
    }

    DispatchUpdateDescriptorSets(gpuav.device, static_cast<uint32_t>(desc_writes.size()), desc_writes.data(), 0, nullptr);
}

static bool WasInstrumented(const LastBound &last_bound) {
    if (last_bound.pipeline_state) {
        return last_bound.pipeline_state->instrumentation_data.was_instrumented;
    }
    for (uint32_t i = 0; i < kShaderObjectStageCount; ++i) {
        const auto stage = static_cast<ShaderObjectStage>(i);
        if (!last_bound.IsValidShaderObjectBound(stage)) {
            continue;
        }
        if (const vvl::ShaderObject *shader_object_state = last_bound.GetShaderObjectState(stage)) {
            auto &sub_state = SubState(*shader_object_state);
            if (sub_state.was_instrumented) {
                return true;
            }
        }
    }
    return false;
}

// Pull together all the information from the debug record to build the error message strings,
// and then assemble them into a single message string.
// Retrieve the shader program referenced by the unique shader ID provided in the debug record.
// We had to keep a copy of the shader program with the same lifecycle as the pipeline to make
// sure it is available when the pipeline is submitted.  (The ShaderModule tracking object also
// keeps a copy, but it can be destroyed after the pipeline is created and before it is submitted.)
//
bool LogInstrumentationError(Validator &gpuav, const CommandBufferSubState &cb_state, const LogObjectList &objlist,
                             const CommonInstrumentationErrorInfo &error_info, const uint32_t *error_record,
                             const Location &loc_with_debug_region,
                             const std::vector<CommandBufferSubState::InstrumentationErrorLogger> &error_loggers) {
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
    bool error_found = false;

    for (const CommandBufferSubState::InstrumentationErrorLogger &error_logger : error_loggers) {
        error_found = error_logger(gpuav, loc_with_debug_region, error_record, error_msg, vuid_msg);
        if (error_found) {
            break;
        }
    }

    if (error_found) {
        // Lookup the VkShaderModule handle and SPIR-V code used to create the shader, using the unique shader ID value returned
        // by the instrumented shader.
        const InstrumentedShader *instrumented_shader = nullptr;
        const uint32_t shader_id = error_record[glsl::kHeader_ShaderIdErrorOffset] & glsl::kShaderIdMask;
        auto it = gpuav.instrumented_shaders_map_.find(shader_id);
        if (it != gpuav.instrumented_shaders_map_.end()) {
            instrumented_shader = &it->second;
        }

        const uint32_t stage_id = error_record[glsl::kHeader_StageInstructionIdOffset] >> glsl::kStageId_Shift;
        const uint32_t instruction_position_offset =
            error_record[glsl::kHeader_StageInstructionIdOffset] & glsl::kInstructionId_Mask;
        GpuShaderInstrumentor::ShaderMessageInfo shader_info{stage_id,
                                                             error_record[glsl::kHeader_StageInfoOffset_0],
                                                             error_record[glsl::kHeader_StageInfoOffset_1],
                                                             error_record[glsl::kHeader_StageInfoOffset_2],
                                                             instruction_position_offset,
                                                             shader_id};
        std::string debug_info_message = gpuav.GenerateDebugInfoMessage(
            cb_state.VkHandle(), shader_info, instrumented_shader, error_info.pipeline_bind_point, error_info.action_command_index);

        gpuav.LogError(vuid_msg.c_str(), objlist, loc_with_debug_region, "%s\n%s", error_msg.c_str(), debug_info_message.c_str());
    }

    return error_found;
}

void PreCallSetupShaderInstrumentationResourcesClassic(Validator &gpuav, CommandBufferSubState &cb_state,
                                                       const LastBound &last_bound,
                                                       const InstBindingPipeLayout &inst_binding_pipe_layout, const Location &loc) {
    VkDescriptorSet instrumentation_desc_set =
        cb_state.gpu_resources_manager.GetManagedDescriptorSet(cb_state.GetInstrumentationDescriptorSetLayout());
    if (!instrumentation_desc_set) {
        gpuav.InternalError(cb_state.VkHandle(), loc, "Unable to allocate instrumentation descriptor sets.");
        return;
    }

    CommonInstrumentationErrorInfo error_info;
    UpdateInstrumentationDescSet(gpuav, cb_state, last_bound.bind_point, instrumentation_desc_set, loc, error_info);

    error_info.action_command_index = cb_state.GetActionCommandIndex(last_bound.bind_point);
    error_info.pipeline_bind_point = last_bound.bind_point;

    // Bind instrumentation descriptor set, using an appropriate pipeline layout
    // ---

    const uint32_t error_logger_index = cb_state.GetErrorLoggerIndex();

    assert(error_logger_index < gpuav.gpuav_settings.indices_count);
    assert(error_info.action_command_index < gpuav.gpuav_settings.indices_count);
    const std::array<uint32_t, 2> dynamic_offsets = {
        {error_info.action_command_index * gpuav.indices_buffer_alignment_, error_logger_index * gpuav.indices_buffer_alignment_}};

    if (inst_binding_pipe_layout.handle != VK_NULL_HANDLE) {
        if (inst_binding_pipe_layout.state &&
            (uint32_t)inst_binding_pipe_layout.state->set_layouts.list.size() > gpuav.instrumentation_desc_set_bind_index_) {
            gpuav.InternalWarning(cb_state.Handle(), loc,
                                  "Unable to bind instrumentation descriptor set, it would override application's bound set");
            return;
        }

        switch (inst_binding_pipe_layout.source) {
            case PipelineLayoutSource::NoPipelineLayout:
                // should not get there, because inst_desc_set_binding_pipe_layout_state is not null
                assert(false);
                break;
            case PipelineLayoutSource::LastBoundPipeline:
                DispatchCmdBindDescriptorSets(cb_state.VkHandle(), last_bound.bind_point, inst_binding_pipe_layout.handle,
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

                const uint32_t last_pipe_bindings_count = LastBoundPipelineOrShaderDescSetBindingsCount(last_bound);
                const uint32_t last_pipe_pcr_count = LastBoundPipelineOrShaderPushConstantsRangesCount(last_bound);

                // If the number of binding of the currently bound pipeline's layout (or the equivalent for shader objects) is
                // less that the number of bindings in the pipeline layout used to bind descriptor sets,
                // GPU-AV needs to create a temporary pipeline layout matching the the currently bound pipeline's layout
                // to bind the instrumentation descriptor set
                if (last_pipe_bindings_count < (uint32_t)inst_binding_pipe_layout.state->set_layouts.list.size() ||
                    last_pipe_pcr_count < (uint32_t)inst_binding_pipe_layout.state->push_constant_ranges_layout->size()) {
                    VkPipelineLayout instrumentation_pipe_layout = CreateInstrumentationPipelineLayout(
                        gpuav, loc, last_bound, gpuav.dummy_desc_layout_[vvl::DescriptorModeClassic],
                        gpuav.GetInstrumentationDescriptorSetLayout(vvl::DescriptorModeClassic),
                        gpuav.instrumentation_desc_set_bind_index_);

                    if (instrumentation_pipe_layout != VK_NULL_HANDLE) {
                        DispatchCmdBindDescriptorSets(cb_state.VkHandle(), last_bound.bind_point, instrumentation_pipe_layout,
                                                      gpuav.instrumentation_desc_set_bind_index_, 1, &instrumentation_desc_set,
                                                      static_cast<uint32_t>(dynamic_offsets.size()), dynamic_offsets.data());
                        DispatchDestroyPipelineLayout(gpuav.device, instrumentation_pipe_layout, nullptr);
                    } else {
                        // Could not create instrumentation pipeline layout
                        return;
                    }
                } else {
                    // No incompatibility detected, safe to use pipeline layout for last bound descriptor set/push constants.
                    DispatchCmdBindDescriptorSets(cb_state.VkHandle(), last_bound.bind_point,
                                                  inst_binding_pipe_layout.state->VkHandle(),
                                                  gpuav.instrumentation_desc_set_bind_index_, 1, &instrumentation_desc_set,
                                                  static_cast<uint32_t>(dynamic_offsets.size()), dynamic_offsets.data());
                }
            } break;
        }

    } else {
        // If no pipeline layout was bound when using shader objects that don't use any descriptor set, and no push constants, bind
        // the instrumentation pipeline layout
        DispatchCmdBindDescriptorSets(cb_state.VkHandle(), last_bound.bind_point,
                                      gpuav.GetInstrumentationPipelineLayout(vvl::DescriptorModeClassic),
                                      gpuav.instrumentation_desc_set_bind_index_, 1, &instrumentation_desc_set,
                                      static_cast<uint32_t>(dynamic_offsets.size()), dynamic_offsets.data());
    }

    std::vector<CommandBufferSubState::InstrumentationErrorLogger> error_loggers = {};
    for (const auto &func : vvl::make_span(cb_state.on_instrumentation_error_logger_register_functions)) {
        error_loggers.emplace_back(func(gpuav, cb_state, last_bound));
    }

    CommandBufferSubState::ErrorLoggerFunc error_logger = [&gpuav, &cb_state, error_info, error_loggers = std::move(error_loggers)](
                                                              const uint32_t *error_record, const Location &loc_with_debug_region,
                                                              const LogObjectList &objlist) {
        bool skip = false;
        skip |= LogInstrumentationError(gpuav, cb_state, objlist, error_info, error_record, loc_with_debug_region, error_loggers);
        return skip;
    };

    cb_state.AddCommandErrorLogger(loc, &last_bound, std::move(error_logger));
}

void PreCallSetupShaderInstrumentationResourcesDescriptorBuffer(Validator &gpuav, CommandBufferSubState &cb_state,
                                                                const LastBound &last_bound,
                                                                const InstBindingPipeLayout &inst_binding_pipe_layout,
                                                                const Location &loc) {
    if (!gpuav.gpuav_settings.debug_printf_enabled) {
        return;  // currently only thing enabled
    }

    VkPipelineLayout bind_pipeline_layout_handle = inst_binding_pipe_layout.handle;

    if (inst_binding_pipe_layout.handle == VK_NULL_HANDLE) {
        // This can only happen if there are no pipeline layout bound when using shader objects, with no descriptor set nor push
        // constants and the user called vkCmdBindDescriptorBuffersEXT but never vkCmdSetDescriptorBufferOffsetsEXT
        bind_pipeline_layout_handle = gpuav.GetInstrumentationPipelineLayout(vvl::DescriptorModeBuffer);
    } else if (inst_binding_pipe_layout.state &&
               (uint32_t)inst_binding_pipe_layout.state->set_layouts.list.size() > gpuav.instrumentation_desc_set_bind_index_) {
        gpuav.InternalWarning(cb_state.Handle(), loc,
                              "Unable to bind instrumentation descriptor set, it would override application's bound set");
        return;
    }

    // There likely is only Push Constants (and BDA) used, but because the VkDescriptorSetLayout is using VK_EXT_descriptor_buffer,
    // and GPU-AV requires using a previous vkCmdBindDescriptorBuffersEXT to inject our code, so we have to inject it ourselves
    if (last_bound.GetDescriptorMode() == vvl::DescriptorModeUnknown) {
        VkDescriptorBufferBindingInfoEXT binding_info = vku::InitStructHelper();
        binding_info.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
        binding_info.address = gpuav.global_resource_descriptor_buffer_.Address();
        DispatchCmdBindDescriptorBuffersEXT(cb_state.VkHandle(), 1, &binding_info);

        cb_state.resource_descriptor_buffer_index_ = 0;
    }

    // Currently only DebugPrintf is using the Descriptor Buffer so we just put our information in front
    const VkDeviceSize front_offset = 0;
    DispatchCmdSetDescriptorBufferOffsetsEXT(cb_state.VkHandle(), last_bound.bind_point, bind_pipeline_layout_handle,
                                             gpuav.instrumentation_desc_set_bind_index_, 1,
                                             &cb_state.resource_descriptor_buffer_index_, &front_offset);

    CommonInstrumentationErrorInfo error_info;
    UpdateInstrumentationDescBuffer(gpuav, cb_state, last_bound, loc, error_info);

    // TODO - Add callback for non-DebugPrintf checks
    (void)error_info;
}

void PreCallSetupShaderInstrumentationResources(Validator &gpuav, CommandBufferSubState &cb_state, const LastBound &last_bound,
                                                const Location &loc) {
    if (!gpuav.gpuav_settings.IsSpirvModified()) {
        return;
    }

    // If nothing was updated, we don't want to bind anything
    if (!WasInstrumented(last_bound)) {
        return;
    }

    if (!last_bound.pipeline_state && !last_bound.HasShaderObjects()) {
        gpuav.InternalError(cb_state.VkHandle(), loc, "Neither pipeline state nor shader object states were found.");
        return;
    }

    InstBindingPipeLayout inst_binding_pipe_layout;

    // App uses regular pipelines or graphics pipeline libraries
    if (last_bound.pipeline_state) {
        const PipelineSubState &pipeline_sub_state = SubState(*last_bound.pipeline_state);
        const vvl::DescriptorMode mode = last_bound.GetActionDescriptorMode();
        inst_binding_pipe_layout.handle = pipeline_sub_state.GetPipelineLayoutUnion(loc, mode);
        assert(inst_binding_pipe_layout.handle != VK_NULL_HANDLE);
        if (gpuav.aborted_) {
            return;
        }
        inst_binding_pipe_layout.source = PipelineLayoutSource::LastBoundPipeline;
    }
    // App uses shader objects
    else {
        const vvl::BindPoint vvl_bind_point = ConvertToVvlBindPoint(last_bound.bind_point);
        if (last_bound.desc_set_pipeline_layout) {
            inst_binding_pipe_layout.state = last_bound.desc_set_pipeline_layout;
            inst_binding_pipe_layout.handle = inst_binding_pipe_layout.state->VkHandle();
            inst_binding_pipe_layout.source = PipelineLayoutSource::LastBoundDescriptorSet;
        } else if (cb_state.push_constant_latest_used_layout[vvl_bind_point] != VK_NULL_HANDLE) {
            inst_binding_pipe_layout.state =
                gpuav.Get<vvl::PipelineLayout>(cb_state.push_constant_latest_used_layout[vvl_bind_point]);
            inst_binding_pipe_layout.handle = inst_binding_pipe_layout.state->VkHandle();
            inst_binding_pipe_layout.source = PipelineLayoutSource::LastPushedConstants;
        }
    }

    if (last_bound.GetActionDescriptorMode() == vvl::DescriptorModeBuffer) {
        PreCallSetupShaderInstrumentationResourcesDescriptorBuffer(gpuav, cb_state, last_bound, inst_binding_pipe_layout, loc);
    } else {
        PreCallSetupShaderInstrumentationResourcesClassic(gpuav, cb_state, last_bound, inst_binding_pipe_layout, loc);
    }
}

void PostCallSetupShaderInstrumentationResourcesClassic(Validator &gpuav, CommandBufferSubState &cb_state,
                                                        const LastBound &last_bound) {
    // Only need to rebind application desc sets if they have been disturbed by GPU-AV binding its instrumentation desc set.
    // - Can happen if the pipeline layout used to bind instrumentation descriptor set is not compatible with the one used by the
    // app to bind the last/all the last desc set.
    // => We create this incompatibility when we add our empty descriptor set.
    // See PositiveGpuAVDescriptorIndexing.SharedPipelineLayoutSubsetGraphics for instance
    if (last_bound.desc_set_pipeline_layout) {
        const uint32_t desc_set_bindings_counts_from_last_pipeline = LastBoundPipelineOrShaderDescSetBindingsCount(last_bound);

        const bool any_disturbed_desc_sets_bindings =
            desc_set_bindings_counts_from_last_pipeline <
            static_cast<uint32_t>(last_bound.desc_set_pipeline_layout->set_layouts.list.size());

        if (any_disturbed_desc_sets_bindings) {
            const uint32_t disturbed_bindings_count = static_cast<uint32_t>(
                last_bound.desc_set_pipeline_layout->set_layouts.list.size() - desc_set_bindings_counts_from_last_pipeline);
            const uint32_t first_disturbed_set = desc_set_bindings_counts_from_last_pipeline;

            for (uint32_t set_i = 0; set_i < disturbed_bindings_count; ++set_i) {
                const uint32_t last_bound_set_i = set_i + first_disturbed_set;
                const auto &last_bound_set_state = last_bound.ds_slots[last_bound_set_i].ds_state;
                // last_bound.ds_slot is a LUT, and descriptor sets before the last one could be unbound.
                if (!last_bound_set_state) {
                    continue;
                }
                VkDescriptorSet last_bound_set = last_bound_set_state->VkHandle();
                const std::vector<uint32_t> &dynamic_offset = last_bound.ds_slots[last_bound_set_i].dynamic_offsets;
                const uint32_t dynamic_offset_count = static_cast<uint32_t>(dynamic_offset.size());
                DispatchCmdBindDescriptorSets(cb_state.VkHandle(), last_bound.bind_point,
                                              last_bound.desc_set_pipeline_layout->VkHandle(), last_bound_set_i, 1, &last_bound_set,
                                              dynamic_offset_count, dynamic_offset.data());
            }
        }
    }
}

void PostCallSetupShaderInstrumentationResources(Validator &gpuav, CommandBufferSubState &cb_state, const LastBound &last_bound) {
    if (!gpuav.gpuav_settings.IsSpirvModified()) {
        return;
    }

    // If nothing was updated, we don't want to bind anything
    if (!WasInstrumented(last_bound)) {
        return;
    }

    if (last_bound.GetActionDescriptorMode() == vvl::DescriptorModeClassic) {
        PostCallSetupShaderInstrumentationResourcesClassic(gpuav, cb_state, last_bound);
    }
}

}  // namespace gpuav
