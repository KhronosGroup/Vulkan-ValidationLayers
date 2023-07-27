/* Copyright (c) 2023 Nintendo
 * Copyright (c) 2023 LunarG, Inc.
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

#include "stateless/stateless_validation.h"

bool StatelessValidation::manual_PreCallValidateCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                                 const VkShaderCreateInfoEXT *pCreateInfos,
                                                                 const VkAllocationCallbacks *pAllocator,
                                                                 VkShaderEXT *pShaders) const {
    bool skip = false;

    // offset needs to be a multiple of 4.
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        const VkShaderCreateInfoEXT &createInfo = pCreateInfos[i];
        if (SafeModulo(createInfo.codeSize, 4) != 0) {
            skip |= LogError(device, "VUID-VkShaderCreateInfoEXT-codeSize-08735",
                             "vkCreateShadersEXT(): pCreateInfos[%" PRIu32 "].codeSize (%" PRIu64 ") is not a multiple of 4.", i,
                             static_cast<uint64_t>(createInfo.codeSize));
        }

        auto pCode = reinterpret_cast<std::uintptr_t>(createInfo.pCode);
        if (createInfo.codeType == VK_SHADER_CODE_TYPE_BINARY_EXT) {
            if (SafeModulo(pCode, 16 * sizeof(unsigned char)) != 0) {
                skip |= LogError(device, "VUID-VkShaderCreateInfoEXT-pCode-08492",
                                 "vkCreateShadersEXT(): pCreateInfos[%" PRIu32
                                 "].codeType is VK_SHADER_CODE_TYPE_BINARY_EXT, but pCreateInfos[%" PRIu32
                                 "].pCode is not aligned to 16 bytes.",
                                 i, i);
            }
        } else if (createInfo.codeType == VK_SHADER_CODE_TYPE_SPIRV_EXT) {
            if (SafeModulo(pCode, 4 * sizeof(unsigned char)) != 0) {
                skip |= LogError(device, "VUID-VkShaderCreateInfoEXT-pCode-08493",
                                 "vkCreateShadersEXT(): pCreateInfos[%" PRIu32
                                 "].codeType is VK_SHADER_CODE_TYPE_SPIRV_EXT, but pCreateInfos[%" PRIu32
                                 "].codeType is not aligned to 4 bytes.",
                                 i, i);
            }
        }

        const VkShaderStageFlags linkedStages = VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT |
                                                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                                                VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_GEOMETRY_BIT |
                                                VK_SHADER_STAGE_FRAGMENT_BIT;
        if ((createInfo.stage & linkedStages) == 0 && (createInfo.flags & VK_SHADER_CREATE_LINK_STAGE_BIT_EXT) != 0) {
            skip |= LogError(
                device, "VUID-VkShaderCreateInfoEXT-flags-08412",
                "vkCreateShadersEXT(): pCreateInfos[%" PRIu32
                "].flags (%s) contains VK_SHADER_CREATE_LINK_STAGE_BIT_EXT bit, but pCreateInfos[%" PRIu32 "].stage is %s.",
                i, string_VkShaderCreateFlagsEXT(createInfo.flags).c_str(), i, string_VkShaderStageFlagBits(createInfo.stage));
        }
        if (((createInfo.flags & VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT) != 0) &&
            ((createInfo.stage & (VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_COMPUTE_BIT)) ==
             0)) {
            skip |= LogError(
                device, "VUID-VkShaderCreateInfoEXT-flags-08992",
                "vkCreateShadersEXT(): pCreateInfos[%" PRIu32 "].flags (%s), but pCreateInfos[%" PRIu32 "].stage is %s.", i,
                string_VkShaderCreateFlagsEXT(createInfo.flags).c_str(), i, string_VkShaderStageFlagBits(createInfo.stage));
        }
        if ((createInfo.stage != VK_SHADER_STAGE_COMPUTE_BIT) &&
            ((createInfo.flags & VK_SHADER_CREATE_DISPATCH_BASE_BIT_EXT) != 0)) {
            skip |= LogError(device, "VUID-VkShaderCreateInfoEXT-flags-08485",
                             "vkCreateShadersEXT(): pCreateInfos[%" PRIu32
                             "].flags include VK_SHADER_CREATE_DISPATCH_BASE_BIT_EXT, but pCreateInfos[%" PRIu32 "].stage is %s.",
                             i, i, string_VkShaderStageFlagBits(createInfo.stage));
        }
        if (createInfo.stage != VK_SHADER_STAGE_FRAGMENT_BIT) {
            if ((createInfo.flags & VK_SHADER_CREATE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_EXT) != 0) {
                skip |=
                    LogError(device, "VUID-VkShaderCreateInfoEXT-flags-08486",
                             "vkCreateShadersEXT(): pCreateInfos[%" PRIu32
                             "].flags include VK_SHADER_CREATE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_EXT, but pCreateInfos[%" PRIu32
                             "].stage is %s.",
                             i, i, string_VkShaderStageFlagBits(createInfo.stage));
            }
            if ((createInfo.flags & VK_SHADER_CREATE_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT) != 0) {
                skip |=
                    LogError(device, "VUID-VkShaderCreateInfoEXT-flags-08488",
                             "vkCreateShadersEXT(): pCreateInfos[%" PRIu32
                             "].flags include VK_SHADER_CREATE_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT, but pCreateInfos[%" PRIu32
                             "].stage is %s.",
                             i, i, string_VkShaderStageFlagBits(createInfo.stage));
            }
        }

        if (createInfo.stage != VK_SHADER_STAGE_MESH_BIT_EXT &&
            (createInfo.flags & VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT) != 0) {
            skip |= LogError(
                device, "VUID-VkShaderCreateInfoEXT-flags-08414",
                "vkCreateShadersEXT(): pCreateInfos[%" PRIu32 "].flags are %s, but pCreateInfos[%" PRIu32 "].stage is %s.", i,
                string_VkShaderCreateFlagsEXT(createInfo.flags).c_str(), i, string_VkShaderStageFlagBits(createInfo.stage));
        }

        if (createInfo.stage == VK_SHADER_STAGE_VERTEX_BIT) {
            if ((createInfo.nextStage &
                 ~(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)) != 0) {
                skip |= LogError(device, "VUID-VkShaderCreateInfoEXT-nextStage-08427",
                                 "vkCreateShadersEXT(): pCreateInfos[%" PRIu32
                                 "].stage is VK_SHADER_STAGE_VERTEX_BIT, but pCreateInfos[%" PRIu32 "].nextStage is %s.",
                                 i, i, string_VkShaderStageFlags(createInfo.nextStage).c_str());
            }
        } else if (createInfo.stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
            if ((createInfo.nextStage & ~(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)) != 0) {
                skip |=
                    LogError(device, "VUID-VkShaderCreateInfoEXT-nextStage-08430",
                             "vkCreateShadersEXT(): pCreateInfos[%" PRIu32
                             "].stage is VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, but pCreateInfos[%" PRIu32 "].nextStage is %s.",
                             i, i, string_VkShaderStageFlags(createInfo.nextStage).c_str());
            }
        } else if (createInfo.stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
            if ((createInfo.nextStage & ~(VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)) != 0) {
                skip |= LogError(device, "VUID-VkShaderCreateInfoEXT-nextStage-08431",
                                 "vkCreateShadersEXT(): pCreateInfos[%" PRIu32
                                 "].stage is VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, but pCreateInfos[%" PRIu32
                                 "].nextStage is %s.",
                                 i, i, string_VkShaderStageFlags(createInfo.nextStage).c_str());
            }
        } else if (createInfo.stage == VK_SHADER_STAGE_GEOMETRY_BIT) {
            if ((createInfo.nextStage & ~VK_SHADER_STAGE_FRAGMENT_BIT) != 0) {
                skip |= LogError(device, "VUID-VkShaderCreateInfoEXT-nextStage-08433",
                                 "vkCreateShadersEXT(): pCreateInfos[%" PRIu32
                                 "].stage is VK_SHADER_STAGE_GEOMETRY_BIT, but pCreateInfos[%" PRIu32 "].nextStage is %s.",
                                 i, i, string_VkShaderStageFlags(createInfo.nextStage).c_str());
            }
        } else if (createInfo.stage == VK_SHADER_STAGE_FRAGMENT_BIT || createInfo.stage == VK_SHADER_STAGE_COMPUTE_BIT) {
            if (createInfo.nextStage != 0) {
                skip |= LogError(
                    device, "VUID-VkShaderCreateInfoEXT-nextStage-08434",
                    "vkCreateShadersEXT(): pCreateInfos[%" PRIu32 "].stage is %s, but pCreateInfos[%" PRIu32 "].nextStage is %s.",
                    i, string_VkShaderStageFlagBits(createInfo.stage), i, string_VkShaderStageFlags(createInfo.nextStage).c_str());
            }
        } else if (createInfo.stage == VK_SHADER_STAGE_TASK_BIT_EXT) {
            if ((createInfo.nextStage & ~VK_SHADER_STAGE_MESH_BIT_EXT) != 0) {
                skip |= LogError(device, "VUID-VkShaderCreateInfoEXT-nextStage-08435",
                                 "vkCreateShadersEXT(): pCreateInfos[%" PRIu32
                                 "].stage is VK_SHADER_STAGE_TASK_BIT_EXT, but pCreateInfos[%" PRIu32 "].nextStage is %s.",
                                 i, i, string_VkShaderStageFlags(createInfo.nextStage).c_str());
            }
        } else if (createInfo.stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
            if ((createInfo.nextStage & ~VK_SHADER_STAGE_FRAGMENT_BIT) != 0) {
                skip |= LogError(device, "VUID-VkShaderCreateInfoEXT-nextStage-08436",
                                 "vkCreateShadersEXT(): pCreateInfos[%" PRIu32
                                 "].stage is VK_SHADER_STAGE_MESH_BIT_EXT, but pCreateInfos[%" PRIu32 "].nextStage is %s.",
                                 i, i, string_VkShaderStageFlags(createInfo.nextStage).c_str());
            }
        } else if (createInfo.stage == VK_SHADER_STAGE_ALL_GRAPHICS || createInfo.stage == VK_SHADER_STAGE_ALL) {
            skip |= LogError(device, "VUID-VkShaderCreateInfoEXT-stage-08418",
                             "vkCreateShadersEXT(): pCreateInfos[%" PRIu32 "].stage is %s.", i,
                             string_VkShaderStageFlagBits(createInfo.stage));
        } else if (createInfo.stage == VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI) {
            skip |= LogError(device, "VUID-VkShaderCreateInfoEXT-stage-08425",
                             "vkCreateShadersEXT(): pCreateInfos[%" PRIu32 "].stage is %s.", i,
                             string_VkShaderStageFlagBits(createInfo.stage));
        } else if (createInfo.stage == VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI) {
            skip |= LogError(device, "VUID-VkShaderCreateInfoEXT-stage-08426",
                             "vkCreateShadersEXT(): pCreateInfos[%" PRIu32 "].stage is %s.", i,
                             string_VkShaderStageFlagBits(createInfo.stage));
        }
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t *pDataSize,
                                                                       void *pData) const {
    bool skip = false;

    if (pData) {
        auto ptr = reinterpret_cast<std::uintptr_t>(pData);
        if (SafeModulo(ptr, 16 * sizeof(unsigned char)) != 0) {
            skip |= LogError(device, "VUID-vkGetShaderBinaryDataEXT-None-08499",
                             "vkCreateShadersEXT(): pData is not aligned to 16 bytes.");
        }
    }

    return skip;
}
