/* Copyright (c) 2020-2023 The Khronos Group Inc.
 * Copyright (c) 2020-2023 Valve Corporation
 * Copyright (c) 2020-2023 LunarG, Inc.
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
#include "generated/chassis.h"

void UtilGenerateStageMessage(const uint32_t *debug_record, std::string &msg);
void UtilGenerateCommonMessage(const debug_report_data *report_data, const VkCommandBuffer commandBuffer,
                               const uint32_t *debug_record, const VkShaderModule shader_module_handle,
                               const VkPipeline pipeline_handle, const VkShaderEXT shader_object_handle,
                               const VkPipelineBindPoint pipeline_bind_point, const uint32_t operation_index, std::string &msg);
void UtilGenerateSourceMessages(vvl::span<const uint32_t> pgm, const uint32_t *debug_record, bool from_printf,
                                std::string &filename_msg, std::string &source_msg);
