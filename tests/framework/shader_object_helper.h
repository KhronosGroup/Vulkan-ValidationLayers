/*
 * Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once

#include "layer_validation_tests.h"
#include "shader_helper.h"

// Assumes VK_SHADER_CODE_TYPE_SPIRV_EXT as that is how must tests will create the shader
VkShaderCreateInfoEXT ShaderCreateInfo(const std::vector<u32>& spirv, VkShaderStageFlagBits stage, u32 set_layout_count = 0,
                                       const VkDescriptorSetLayout* set_layouts = nullptr, u32 pc_range_count = 0,
                                       const VkPushConstantRange* pc_ranges = nullptr,
                                       const VkSpecializationInfo* specialization_info = nullptr);

VkShaderCreateInfoEXT ShaderCreateInfoFlag(const std::vector<u32>& spirv, VkShaderStageFlagBits stage,
                                           VkShaderCreateFlagsEXT flags);

VkShaderCreateInfoEXT ShaderCreateInfoLink(const std::vector<u32>& spirv, VkShaderStageFlagBits stage,
                                           VkShaderStageFlags next_stage = 0);