// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See spirv_validation_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2020-2025 The Khronos Group Inc.
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
 *
 * This file is related to anything that is found in the Vulkan XML related
 * to SPIR-V. Anything related to the SPIR-V grammar belongs in spirv_grammar_helper
 *
 ****************************************************************************/

// NOLINTBEGIN

#pragma once
#include <vulkan/vulkan_core.h>

// This is the one function that requires mapping SPIR-V enums to Vulkan enums
VkFormat CompatibleSpirvImageFormat(uint32_t spirv_image_format);
// Since we keep things in VkFormat for checking, we need a way to get the original SPIR-V
// Format name for any error message
const char* string_SpirvImageFormat(VkFormat format);

// NOLINTEND
