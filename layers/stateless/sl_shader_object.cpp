/* Copyright (c) 2023-2024 Nintendo
 * Copyright (c) 2023-2025 LunarG, Inc.
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
#include <spirv/unified1/spirv.hpp>
#include "utils/math_utils.h"

namespace stateless {

bool Device::ValidateCreateShadersFlags(VkShaderCreateFlagsEXT flags, VkShaderStageFlagBits stage, const Location &flag_loc) const {
    bool skip = false;
    if ((flags & VK_SHADER_CREATE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_EXT) != 0 &&
        enabled_features.attachmentFragmentShadingRate == VK_FALSE) {
        skip |= LogError("VUID-VkShaderCreateInfoEXT-flags-08487", device, flag_loc,
                         "is %s, but the attachmentFragmentShadingRate feature was not enabled.",
                         string_VkShaderCreateFlagsEXT(flags).c_str());
    }
    if ((flags & VK_SHADER_CREATE_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT) != 0 &&
        enabled_features.fragmentDensityMap == VK_FALSE) {
        skip |=
            LogError("VUID-VkShaderCreateInfoEXT-flags-08489", device, flag_loc,
                     "is %s, but the fragmentDensityMap feature was not enabled.", string_VkShaderCreateFlagsEXT(flags).c_str());
    }
    if ((flags & VK_SHADER_CREATE_64_BIT_INDEXING_BIT_EXT) != 0 && enabled_features.shader64BitIndexing == VK_FALSE) {
        skip |=
            LogError("VUID-VkShaderCreateInfoEXT-flags-11758", device, flag_loc,
                     "is %s, but the shader64BitIndexing feature was not enabled.", string_VkShaderCreateFlagsEXT(flags).c_str());
    }
    if ((flags & VK_SHADER_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT) != 0 && enabled_features.subgroupSizeControl == VK_FALSE) {
        skip |= LogError(
            "VUID-VkShaderCreateInfoEXT-flags-09404", device, flag_loc,
            "contains VK_SHADER_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT, but subgroupSizeControl feature is not enabled.");
    }
    if ((flags & VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT) != 0 && enabled_features.computeFullSubgroups == VK_FALSE) {
        skip |=
            LogError("VUID-VkShaderCreateInfoEXT-flags-09405", device, flag_loc,
                     "contains VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT, but computeFullSubgroups feature is not enabled.");
    }
    if ((flags & VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT) != 0 && enabled_features.deviceGeneratedCommands == VK_FALSE) {
        skip |=
            LogError("VUID-VkShaderCreateInfoEXT-flags-11005", device, flag_loc,
                     "contains VK_SHADER_CREATE_INDIRECT_BINDABLE_BIT_EXT, but deviceGeneratedCommands feature is not enabled.");
    }

    if ((stage != VK_SHADER_STAGE_COMPUTE_BIT) && ((flags & VK_SHADER_CREATE_DISPATCH_BASE_BIT_EXT) != 0)) {
        skip |=
            LogError("VUID-VkShaderCreateInfoEXT-flags-08485", device, flag_loc,
                     "includes VK_SHADER_CREATE_DISPATCH_BASE_BIT_EXT but the stage is %s.", string_VkShaderStageFlagBits(stage));
    }

    if (stage != VK_SHADER_STAGE_FRAGMENT_BIT) {
        if ((flags & VK_SHADER_CREATE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_EXT) != 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-flags-08486", device, flag_loc,
                             "includes VK_SHADER_CREATE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_EXT but the stage is %s.",
                             string_VkShaderStageFlagBits(stage));
        }
        if ((flags & VK_SHADER_CREATE_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT) != 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-flags-08488", device, flag_loc,
                             "includes VK_SHADER_CREATE_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT but the stage is %s.",
                             string_VkShaderStageFlagBits(stage));
        }
    }

    if (stage != VK_SHADER_STAGE_MESH_BIT_EXT && (flags & VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT) != 0) {
        skip |=
            LogError("VUID-VkShaderCreateInfoEXT-flags-08414", device, flag_loc,
                     "includes VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT but the stage is %s.", string_VkShaderStageFlagBits(stage));
    }

    if (((flags & VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT) != 0) &&
        ((stage & (VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_COMPUTE_BIT)) == 0)) {
        skip |= LogError("VUID-VkShaderCreateInfoEXT-flags-08992", device, flag_loc,
                         "includes VK_SHADER_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT but the stage is %s.",
                         string_VkShaderStageFlagBits(stage));
    }

    return skip;
}

bool Device::manual_PreCallValidateCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                    const VkShaderCreateInfoEXT *pCreateInfos,
                                                    const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                                    const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    for (uint32_t i = 0; i < createInfoCount; ++i) {
        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
        const VkShaderCreateInfoEXT &create_info = pCreateInfos[i];
        auto pCode = reinterpret_cast<std::uintptr_t>(create_info.pCode);

        skip |= ValidateCreateShadersFlags(create_info.flags, create_info.stage, create_info_loc.dot(Field::flags));

        if (create_info.codeType == VK_SHADER_CODE_TYPE_SPIRV_EXT) {
            if (SafeModulo(pCode, 4 * sizeof(unsigned char)) != 0) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-pCode-08493", device, create_info_loc.dot(Field::codeType),
                                 "is VK_SHADER_CODE_TYPE_SPIRV_EXT, but pCode (%p) is not aligned to 4 bytes.", create_info.pCode);
            } else if (SafeModulo(create_info.codeSize, 4) != 0) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-codeSize-08735", device, create_info_loc.dot(Field::codeSize),
                                 "(%" PRIu64 ") is not a multiple of 4. You might have forget to multiply by sizeof(uint32_t).",
                                 static_cast<uint64_t>(create_info.codeSize));
            } else {
                // Can't cast this until we know it is aligned to 4 bytes or USAN will catch it
                const uint32_t first_dword = ((uint32_t *)create_info.pCode)[0];
                if (first_dword != spv::MagicNumber) {
                    skip |= LogError("VUID-VkShaderCreateInfoEXT-pCode-08738", device, create_info_loc.dot(Field::pCode),
                                     "doesn't point to a SPIR-V module. The first dword (0x%" PRIx32
                                     ") is not the SPIR-V MagicNumber (0x07230203).",
                                     first_dword);
                }
            }

        } else if (create_info.codeType == VK_SHADER_CODE_TYPE_BINARY_EXT) {
            if (SafeModulo(pCode, 16 * sizeof(unsigned char)) != 0) {
                skip |=
                    LogError("VUID-VkShaderCreateInfoEXT-pCode-08492", device, create_info_loc.dot(Field::codeType),
                             "is VK_SHADER_CODE_TYPE_BINARY_EXT, but pCode (%p) is not aligned to 16 bytes.", create_info.pCode);
            }
        }

        const VkShaderStageFlags linkedStages = VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT |
                                                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                                                VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_GEOMETRY_BIT |
                                                VK_SHADER_STAGE_FRAGMENT_BIT;
        if ((create_info.stage & linkedStages) == 0 && (create_info.flags & VK_SHADER_CREATE_LINK_STAGE_BIT_EXT) != 0) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-flags-08412", device, create_info_loc.dot(Field::flags),
                             "includes VK_SHADER_CREATE_LINK_STAGE_BIT_EXT but the stage is %s.",
                             string_VkShaderStageFlagBits(create_info.stage));
        }

        if (create_info.stage == VK_SHADER_STAGE_VERTEX_BIT) {
            if ((create_info.nextStage &
                 ~(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)) != 0) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08427", device, create_info_loc.dot(Field::stage),
                                 "is VK_SHADER_STAGE_VERTEX_BIT, but nextStage is %s.",
                                 string_VkShaderStageFlags(create_info.nextStage).c_str());
            }
        } else if (create_info.stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
            if ((create_info.nextStage & ~(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)) != 0) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08430", device, create_info_loc.dot(Field::stage),
                                 "is VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, but nextStage is %s.",
                                 string_VkShaderStageFlags(create_info.nextStage).c_str());
            }
        } else if (create_info.stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
            if ((create_info.nextStage & ~(VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)) != 0) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08431", device, create_info_loc.dot(Field::stage),
                                 "is VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, but nextStage is %s.",
                                 string_VkShaderStageFlags(create_info.nextStage).c_str());
            }
        } else if (create_info.stage == VK_SHADER_STAGE_GEOMETRY_BIT) {
            if ((create_info.nextStage & ~VK_SHADER_STAGE_FRAGMENT_BIT) != 0) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08433", device, create_info_loc.dot(Field::stage),
                                 "is VK_SHADER_STAGE_GEOMETRY_BIT, but nextStage is %s.",
                                 string_VkShaderStageFlags(create_info.nextStage).c_str());
            }
        } else if (create_info.stage == VK_SHADER_STAGE_FRAGMENT_BIT || create_info.stage == VK_SHADER_STAGE_COMPUTE_BIT) {
            if (create_info.nextStage != 0) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08434", device, create_info_loc.dot(Field::stage),
                                 "is %s, but nextStage is %s.", string_VkShaderStageFlagBits(create_info.stage),
                                 string_VkShaderStageFlags(create_info.nextStage).c_str());
            }
        } else if (create_info.stage == VK_SHADER_STAGE_TASK_BIT_EXT) {
            if ((create_info.nextStage & ~VK_SHADER_STAGE_MESH_BIT_EXT) != 0) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08435", device, create_info_loc.dot(Field::stage),
                                 "is VK_SHADER_STAGE_TASK_BIT_EXT, but nextStage is %s.",
                                 string_VkShaderStageFlags(create_info.nextStage).c_str());
            }
        } else if (create_info.stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
            if ((create_info.nextStage & ~VK_SHADER_STAGE_FRAGMENT_BIT) != 0) {
                skip |= LogError("VUID-VkShaderCreateInfoEXT-nextStage-08436", device, create_info_loc.dot(Field::stage),
                                 "is VK_SHADER_STAGE_MESH_BIT_EXT, but nextStage is %s.",
                                 string_VkShaderStageFlags(create_info.nextStage).c_str());
            }
        } else if (create_info.stage == VK_SHADER_STAGE_ALL_GRAPHICS) {
            // string_VkShaderStageFlagBits can't print these, so list manually
            skip |= LogError("VUID-VkShaderCreateInfoEXT-stage-08418", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_ALL_GRAPHICS.");
        } else if (create_info.stage == VK_SHADER_STAGE_ALL) {
            // string_VkShaderStageFlagBits can't print these, so list manually
            skip |= LogError("VUID-VkShaderCreateInfoEXT-stage-08418", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_ALL.");
        } else if (create_info.stage == VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-stage-08425", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI.");
        } else if (create_info.stage == VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI) {
            skip |= LogError("VUID-VkShaderCreateInfoEXT-stage-08426", device, create_info_loc.dot(Field::stage),
                             "is VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI.");
        }

        skip |= ValidatePushConstantRange(create_info.pushConstantRangeCount, create_info.pPushConstantRanges, create_info_loc);
    }

    return skip;
}

bool Device::manual_PreCallValidateGetShaderBinaryDataEXT(VkDevice device, VkShaderEXT shader, size_t *pDataSize, void *pData,
                                                          const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (pData) {
        auto ptr = reinterpret_cast<std::uintptr_t>(pData);
        if (ptr % 16 != 0) {
            skip |= LogError("VUID-vkGetShaderBinaryDataEXT-None-08499", shader, error_obj.location.dot(Field::pData),
                             "(%p) is not aligned to 16 bytes.", pData);
        }
    }

    return skip;
}
}  // namespace stateless
