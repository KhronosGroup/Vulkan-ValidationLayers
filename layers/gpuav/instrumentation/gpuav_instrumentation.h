/* Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
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

#pragma once

#include <vulkan/vulkan.h>

#include "containers/limits.h"
#include "state_tracker/vertex_index_buffer_state.h"

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <limits>

struct Location;
struct LastBound;
struct LogObjectList;

namespace vvl {
struct LabelCommand;
class DescriptorSet;
class PipelineLayout;
}

namespace gpuav {

class Validator;
class CommandBufferSubState;
class Queue;
struct InstrumentationErrorBlob;

enum class PipelineLayoutSource { NoPipelineLayout, LastBoundPipeline, LastBoundDescriptorSet, LastPushedConstants };
struct InstBindingPipeLayout {
    std::shared_ptr<const vvl::PipelineLayout> state;
    VkPipelineLayout handle = VK_NULL_HANDLE;
    PipelineLayoutSource source = PipelineLayoutSource::NoPipelineLayout;
};

void UpdateInstrumentationDescSet(Validator& gpuav, CommandBufferSubState& cb_state, VkPipelineBindPoint bind_point,
                                  VkDescriptorSet instrumentation_desc_set, const Location& loc,
                                  InstrumentationErrorBlob& out_instrumentation_error_blob);
void UpdateInstrumentationDescBuffer(Validator& gpuav, CommandBufferSubState& cb_state, VkPipelineBindPoint bind_point,
                                     const Location& loc, InstrumentationErrorBlob& out_instrumentation_error_blob);

void PreCallSetupShaderInstrumentationResources(Validator& gpuav, CommandBufferSubState& cb_state, const LastBound& last_bound,
                                                const Location& loc);
void PreCallSetupShaderInstrumentationResourcesClassic(Validator& gpuav, CommandBufferSubState& cb_state,
                                                       const LastBound& last_bound,
                                                       const InstBindingPipeLayout& inst_binding_pipe_layout, const Location& loc);
void PreCallSetupShaderInstrumentationResourcesDescriptorBuffer(Validator& gpuav, CommandBufferSubState& cb_state,
                                                                const LastBound& last_bound,
                                                                const InstBindingPipeLayout& inst_binding_pipe_layout,
                                                                const Location& loc);
void PostCallSetupShaderInstrumentationResources(Validator& gpuav, CommandBufferSubState& cb_state, const LastBound& last_bound);
void PostCallSetupShaderInstrumentationResourcesClassic(Validator& gpuav, CommandBufferSubState& cb_state,
                                                        const LastBound& last_bound);

struct VertexAttributeFetchLimit {
    // Default value indicates that no vertex buffer attribute fetching will be OOB
    VkDeviceSize max_vertex_attributes_count = std::numeric_limits<VkDeviceSize>::max();
    vvl::VertexBufferBinding binding_info{};
    VkVertexInputAttributeDescription attribute{};
    uint32_t instance_rate_divisor = vvl::kNoIndex32;
};

// This is data we capture in our lambda at command buffer recording time.
// We use this later upon an error to help hold information needed print the message
struct InstrumentationErrorBlob {
    std::optional<VertexAttributeFetchLimit> vertex_attribute_fetch_limit_vertex_input_rate{};
    std::optional<VertexAttributeFetchLimit> vertex_attribute_fetch_limit_instance_input_rate{};
    std::optional<vvl::IndexBufferBinding> index_buffer_binding{};

    // used to know which action command this occured at
    uint32_t action_command_index = vvl::kNoIndex32;

    // Used to know if from draw, dispatch, or traceRays
    VkPipelineBindPoint pipeline_bind_point = VK_PIPELINE_BIND_POINT_MAX_ENUM;
    // Used pick which VUID to report
    bool uses_shader_object = false;

    // index into the last vkCmdBindDescriptors prior to a draw/dispatch
    uint32_t descriptor_binding_index = vvl::kNoIndex32;
};

// Return true iff an error has been found
bool LogInstrumentationError(Validator& gpuav, const CommandBufferSubState& cb_state, const LogObjectList& objlist,
                             const InstrumentationErrorBlob& instrumentation_error_blob, const uint32_t* error_record,
                             const Location& loc_with_debug_region);

// Return true iff an error has been found in error_record, among the list of errors this function manages
bool LogMessageInstDescriptorIndexingOOB(Validator& gpuav, const CommandBufferSubState& cb_state, const uint32_t* error_record,
                                         std::string& out_error_msg, std::string& out_vuid_msg, const Location& loc,
                                         const InstrumentationErrorBlob& instrumentation_error_blob);
bool LogMessageInstDescriptorClass(Validator& gpuav, const CommandBufferSubState& cb_state, const uint32_t* error_record,
                                   std::string& out_error_msg, std::string& out_vuid_msg, const Location& loc,
                                   const InstrumentationErrorBlob& instrumentation_error_blob);
bool LogMessageInstBufferDeviceAddress(const uint32_t* error_record, std::string& out_error_msg, std::string& out_vuid_msg);
bool LogMessageInstRayQuery(const uint32_t* error_record, std::string& out_error_msg, std::string& out_vuid_msg);

}  // namespace gpuav
