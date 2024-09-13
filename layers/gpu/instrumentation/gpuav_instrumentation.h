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

#pragma once

#include "gpu/resources/gpuav_subclasses.h"

namespace gpuav {

struct DescSetState;

void PreCallSetupShaderInstrumentationResources(Validator& gpuav, CommandBuffer& cb_state, VkPipelineBindPoint bind_point,
                                                const Location& loc);

void PostCallSetupShaderInstrumentationResources(Validator& gpuav, CommandBuffer& cb_statee, VkPipelineBindPoint bind_point,
                                                 const Location& loc);

// Return true iff a error has been found
bool LogInstrumentationError(Validator& gpuav, VkCommandBuffer cmd_buffer, const LogObjectList& objlist, uint32_t operation_index,
                             const uint32_t* error_record, const std::vector<DescSetState>& descriptor_sets,
                             VkPipelineBindPoint pipeline_bind_point, bool uses_shader_object, bool uses_robustness,
                             const Location& loc);

// Return true iff an error has been found in error_record, among the list of errors this function manages
bool LogMessageInstBindlessDescriptor(Validator& gpuav, const uint32_t* error_record, std::string& out_error_msg,
                                      std::string& out_vuid_msg, const std::vector<DescSetState>& descriptor_sets,
                                      const Location& loc, bool uses_shader_object, bool& out_oob_access);
bool LogMessageInstNonBindlessOOB(Validator& gpuav, const uint32_t* error_record, std::string& out_error_msg,
                                  std::string& out_vuid_msg, const std::vector<DescSetState>& descriptor_sets, const Location& loc,
                                  bool uses_shader_object, bool& out_oob_access);
bool LogMessageInstBufferDeviceAddress(const uint32_t* error_record, std::string& out_error_msg, std::string& out_vuid_msg,
                                       bool& out_oob_access);
bool LogMessageInstRayQuery(const uint32_t* error_record, std::string& out_error_msg, std::string& out_vuid_msg);

}  // namespace gpuav
