/* Copyright (c) 2018-2025 The Khronos Group Inc.
 * Copyright (c) 2018-2025 Valve Corporation
 * Copyright (c) 2018-2025 LunarG, Inc.
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

#include "generated/error_location_helper.h"

struct Location;

namespace gpuav {
class Validator;
class CommandBufferSubState;

namespace valcmd {

void FlushValidationCmds(Validator &gpuav, CommandBufferSubState &cb_state);

template <typename IndirectCommand>
void FirstInstance(Validator &gpuav, CommandBufferSubState &cb_state, const Location &loc, VkBuffer draw_buffer,
                   VkDeviceSize draw_buffer_offset, uint32_t draw_count, VkBuffer count_buffer, VkDeviceSize count_buffer_offset,
                   const char *vuid);

template <>
void FirstInstance<VkDrawIndirectCommand>(Validator &gpuav, CommandBufferSubState &cb_state, const Location &loc,
                                          VkBuffer draw_buffer, VkDeviceSize draw_buffer_offset, uint32_t draw_count,
                                          VkBuffer count_buffer, VkDeviceSize count_buffer_offset, const char *vuid);
template <>
void FirstInstance<VkDrawIndexedIndirectCommand>(Validator &gpuav, CommandBufferSubState &cb_state, const Location &loc,
                                                 VkBuffer draw_buffer, VkDeviceSize draw_buffer_offset, uint32_t draw_count,
                                                 VkBuffer count_buffer, VkDeviceSize count_buffer_offset, const char *vuid);

void FirstInstance(Validator &gpuav, CommandBufferSubState &cb_state, const Location &loc, VkBuffer draw_buffer,
                   VkDeviceSize draw_buffer_offset, uint32_t draw_cmds_byte_stride, vvl::Struct draw_indirect_struct_name,
                   uint32_t first_instance_member_pos, uint32_t draw_count, VkBuffer count_buffer, VkDeviceSize count_buffer_offset,
                   const char *vuid);

void CountBuffer(Validator &gpuav, CommandBufferSubState &cb_state, const Location &loc, VkBuffer draw_buffer,
                 VkDeviceSize draw_buffer_offset, uint32_t draw_indirect_struct_byte_size, vvl::Struct draw_indirect_struct_name,
                 uint32_t draw_cmds_byte_stride, VkBuffer count_buffer, VkDeviceSize count_buffer_offset,
                 const char *vuid_max_draw_count);

void DrawMeshIndirect(Validator &gpuav, CommandBufferSubState &cb_state, const Location &loc, VkBuffer draw_buffer,
                      VkDeviceSize draw_buffer_offset, uint32_t draw_cmds_byte_stride, VkBuffer count_buffer,
                      VkDeviceSize count_buffer_offset, uint32_t draw_count);

void DrawIndexedIndirectIndexBuffer(Validator &gpuav, CommandBufferSubState &cb_state, const Location &loc, VkBuffer draw_buffer,
                                    VkDeviceSize draw_buffer_offset, uint32_t draw_cmds_byte_stride, uint32_t draw_count,
                                    VkBuffer count_buffer, VkDeviceSize count_buffer_offset, const char *vuid_oob_index);

}  // namespace valcmd
}  // namespace gpuav
