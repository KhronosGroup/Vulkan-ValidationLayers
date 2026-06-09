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

#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

struct Location;
struct LastBound;

namespace gpuav {
class CommandBufferSubState;
class Validator;

namespace descriptor {
void UpdateBoundDescriptors(Validator& gpuav, CommandBufferSubState& cb_state, VkPipelineBindPoint pipeline_bind_point,
                            const Location& loc);

// We need a way to emulate the VkDescriptorType enum to cover all the "real" types (those seen in
// vkGetPhysicalDeviceDescriptorSizeEXT).
//
// This information is discovered at access time (thanks untyped pointer!) so we need to decide which type at shader instrumentation
// pass time. From there we will want to inject this information into the shader. Final step will be to decode it on an error
// message (which will save having to do the reverse look up in the SPIR-V then).
//
// This all should be capable of being packed in 4 bits as there are only 14 known types currently
// (currently 0x1, 0x5, 0x6, 0x7 are not being used)
// Sampler is a speical case, it is never by itself and instead is is provided along with an image
const uint8_t TYPE_SAMPLER = 0x0;  // VK_DESCRIPTOR_TYPE_SAMPLER
// Buffers
const uint8_t TYPE_BUFFER_MASK = 0x2;
const uint8_t TYPE_UNIFORM_BUFFER = 0x2;  // VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
const uint8_t TYPE_STORAGE_BUFFER = 0x3;  // VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
// RT
const uint8_t TYPE_ACCELERATION_STRUCTURE = 0x4;  // VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR
// Images
const uint8_t TYPE_IMAGE_MASK = 0x8;
const uint8_t TYPE_IMAGE_SAMPLED = 0x8;               // VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
const uint8_t TYPE_IMAGE_STORAGE = 0x9;               // VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
const uint8_t TYPE_IMAGE_TEXEL_BUFFER_UNIFORM = 0xA;  // VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
const uint8_t TYPE_IMAGE_TEXEL_BUFFER_STORAGE = 0xB;  // VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
const uint8_t TYPE_IMAGE_INPUT_ATTACHMENT = 0xC;      // VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
}  // namespace descriptor
}  // namespace gpuav
