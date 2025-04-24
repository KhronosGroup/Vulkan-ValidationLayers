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

#include "state_tracker/vertex_index_buffer_state.h"

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <limits>

struct Location;
struct LogObjectList;

namespace vvl {
struct LabelCommand;
class DescriptorSet;
}

namespace gpuav {

class Validator;
class CommandBufferSubState;
class Queue;
struct DescriptorBindingCommand;
struct InstrumentationErrorBlob;

void UpdateInstrumentationDescSet(Validator& gpuav, CommandBufferSubState& cb_state, VkDescriptorSet instrumentation_desc_set,
                                  const Location& loc, InstrumentationErrorBlob& out_instrumentation_error_blob);

void PreCallSetupShaderInstrumentationResources(Validator& gpuav, CommandBufferSubState& cb_state, VkPipelineBindPoint bind_point,
                                                const Location& loc);
void PostCallSetupShaderInstrumentationResources(Validator& gpuav, CommandBufferSubState& cb_state, VkPipelineBindPoint bind_point,
                                                 const Location& loc);

struct VertexAttributeFetchLimit {
    // Default value indicates that no vertex buffer attribute fetching will be OOB
    VkDeviceSize max_vertex_attributes_count = std::numeric_limits<VkDeviceSize>::max();
    vvl::VertexBufferBinding binding_info{};
    VkVertexInputAttributeDescription attribute{};
    uint32_t instance_rate_divisor = std::numeric_limits<uint32_t>::max();
};

// This is data we capture in our lambda at command buffer recording time.
// We use this later upon an error to help hold information needed print the message
struct InstrumentationErrorBlob {
    std::optional<VertexAttributeFetchLimit> vertex_attribute_fetch_limit_vertex_input_rate{};
    std::optional<VertexAttributeFetchLimit> vertex_attribute_fetch_limit_instance_input_rate{};
    std::optional<vvl::IndexBufferBinding> index_buffer_binding;

    // indexing into the VkDebugUtilsLabelEXT
    uint32_t label_command_i;
    // used to know which action command this occured at
    uint32_t operation_index;

    // Used to know if from draw, dispatch, or traceRays
    VkPipelineBindPoint pipeline_bind_point;
    // Used pick which VUID to report
    bool uses_shader_object;

    // index into the last vkCmdBindDescriptors prior to a draw/dispatch
    uint32_t descriptor_binding_index;
};

// Return true iff an error has been found
bool LogInstrumentationError(Validator& gpuav, const CommandBufferSubState& cb_state, const LogObjectList& objlist,
                             const InstrumentationErrorBlob& instrumentation_error_blob,
                             const std::vector<std::string>& initial_label_stack, const uint32_t* error_record,
                             const Location& loc);

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
