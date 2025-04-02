/*
 * Copyright (c) 2024-2025 The Khronos Group Inc.
 * Copyright (c) 2024-2025 Valve Corporation
 * Copyright (c) 2024-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "shader_object_helper.h"
#include <vulkan/utility/vk_struct_helper.hpp>

void SetNextStage(VkShaderCreateInfoEXT& info, bool tessShaders, bool geomShaders) {
    if (info.stage == VK_SHADER_STAGE_VERTEX_BIT) {
        info.nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;
        if (tessShaders) {
            info.nextStage |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        }
        if (geomShaders) {
            info.nextStage |= VK_SHADER_STAGE_GEOMETRY_BIT;
        }
    } else if (info.stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
        info.nextStage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    } else if (info.stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
        info.nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;
        if (geomShaders) {
            info.nextStage |= VK_SHADER_STAGE_GEOMETRY_BIT;
        }
    } else if (info.stage == VK_SHADER_STAGE_GEOMETRY_BIT) {
        info.nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;
    } else if (info.stage == VK_SHADER_STAGE_TASK_BIT_EXT) {
        // This code assumes if meshShader feature is enabled, taskShader feature is too, for other cases use custom
        // VkShaderCreateInfoEXT and not these helpers
        info.nextStage = VK_SHADER_STAGE_MESH_BIT_EXT;
    } else if (info.stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
        info.nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;
    } else if (info.stage == VK_SHADER_STAGE_TASK_BIT_EXT) {
        info.nextStage = VK_SHADER_STAGE_MESH_BIT_NV;
    } else if (info.stage == VK_SHADER_STAGE_MESH_BIT_NV) {
        info.nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;
    }
}

VkShaderCreateInfoEXT ShaderCreateInfo(const std::vector<uint32_t>& spirv, VkShaderStageFlagBits stage, uint32_t set_layout_count,
                                       const VkDescriptorSetLayout* set_layouts, uint32_t pc_range_count,
                                       const VkPushConstantRange* pc_ranges, const VkSpecializationInfo* specialization_info) {
    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.flags = 0;
    create_info.stage = stage;
    SetNextStage(create_info);
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = spirv.size() * sizeof(uint32_t);
    create_info.pCode = spirv.data();
    create_info.pName = "main";
    create_info.setLayoutCount = set_layout_count;
    create_info.pSetLayouts = set_layouts;
    create_info.pushConstantRangeCount = pc_range_count;
    create_info.pPushConstantRanges = pc_ranges;
    create_info.pSpecializationInfo = specialization_info;
    return create_info;
}

VkShaderCreateInfoEXT ShaderCreateInfoNoNextStage(const std::vector<uint32_t>& spirv, VkShaderStageFlagBits stage,
                                                  uint32_t set_layout_count, const VkDescriptorSetLayout* set_layouts,
                                                  uint32_t pc_range_count, const VkPushConstantRange* pc_ranges,
                                                  const VkSpecializationInfo* specialization_info) {
    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.flags = 0;
    create_info.stage = stage;
    create_info.nextStage = 0u;
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = spirv.size() * sizeof(uint32_t);
    create_info.pCode = spirv.data();
    create_info.pName = "main";
    create_info.setLayoutCount = set_layout_count;
    create_info.pSetLayouts = set_layouts;
    create_info.pushConstantRangeCount = pc_range_count;
    create_info.pPushConstantRanges = pc_ranges;
    create_info.pSpecializationInfo = specialization_info;
    return create_info;
}

VkShaderCreateInfoEXT ShaderCreateInfoFlag(const std::vector<uint32_t>& spirv, VkShaderStageFlagBits stage,
                                           VkShaderCreateFlagsEXT flags) {
    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.flags = flags;
    create_info.stage = stage;
    SetNextStage(create_info);
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = spirv.size() * sizeof(uint32_t);
    create_info.pCode = spirv.data();
    create_info.pName = "main";
    create_info.setLayoutCount = 0;
    create_info.pSetLayouts = nullptr;
    create_info.pushConstantRangeCount = 0;
    create_info.pPushConstantRanges = nullptr;
    create_info.pSpecializationInfo = nullptr;
    return create_info;
}

VkShaderCreateInfoEXT ShaderCreateInfoLink(const std::vector<uint32_t>& spirv, VkShaderStageFlagBits stage,
                                           VkShaderStageFlags next_stage) {
    VkShaderCreateInfoEXT create_info = vku::InitStructHelper();
    create_info.flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
    create_info.stage = stage;
    create_info.nextStage = next_stage;
    create_info.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    create_info.codeSize = spirv.size() * sizeof(uint32_t);
    create_info.pCode = spirv.data();
    create_info.pName = "main";
    return create_info;
}
