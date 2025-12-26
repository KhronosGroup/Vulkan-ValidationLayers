/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 LunarG, Inc.
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

#include <vulkan/utility/vk_format_utils.h>
#include "error_message/error_location.h"
#include "stateless/stateless_validation.h"
#include "stateless/sl_vuid_maps.h"

namespace stateless {

bool Device::ValidateCreateDataGraphPipelinesFlags(const VkPipelineCreateFlags2 flags, const Location &flags_loc) const {
    bool skip = false;

    constexpr VkPipelineCreateFlags2 valid_flag_mask =
        VK_PIPELINE_CREATE_2_NO_PROTECTED_ACCESS_BIT | VK_PIPELINE_CREATE_2_PROTECTED_ACCESS_ONLY_BIT |
        VK_PIPELINE_CREATE_2_DISABLE_OPTIMIZATION_BIT | VK_PIPELINE_CREATE_2_DESCRIPTOR_BUFFER_BIT_EXT |
        VK_PIPELINE_CREATE_2_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT | VK_PIPELINE_CREATE_2_EARLY_RETURN_ON_FAILURE_BIT;

    if ((flags & ~(valid_flag_mask)) != 0) {
        // TODO - print the flags that violated the list
        skip |= LogError("VUID-VkDataGraphPipelineCreateInfoARM-flags-09764", device, flags_loc, "(%s) contains invalid values.",
                         string_VkPipelineCreateFlags2(flags).c_str());
    }

    if ((flags & VK_PIPELINE_CREATE_2_DESCRIPTOR_BUFFER_BIT_EXT) != 0) {
        if (!enabled_features.dataGraphDescriptorBuffer) {
            skip |= LogError("VUID-VkDataGraphPipelineCreateInfoARM-dataGraphDescriptorBuffer-09885", device, flags_loc,
                             "(%s) includes VK_PIPELINE_CREATE_2_DESCRIPTOR_BUFFER_BIT_EXT but the dataGraphDescriptorBuffer "
                             "feature is not enabled.",
                             string_VkPipelineCreateFlags2(flags).c_str());
        }
    }

    if ((flags & (VK_PIPELINE_CREATE_2_NO_PROTECTED_ACCESS_BIT | VK_PIPELINE_CREATE_2_PROTECTED_ACCESS_ONLY_BIT)) != 0) {
        if (!enabled_features.pipelineProtectedAccess) {
            skip |= LogError("VUID-VkDataGraphPipelineCreateInfoARM-pipelineProtectedAccess-09772", device, flags_loc,
                             "is %s, but pipelineProtectedAccess feature was not enabled.",
                             string_VkPipelineCreateFlags2(flags).c_str());
        }
        if ((flags & VK_PIPELINE_CREATE_2_NO_PROTECTED_ACCESS_BIT) && (flags & VK_PIPELINE_CREATE_2_PROTECTED_ACCESS_ONLY_BIT)) {
            skip |= LogError("VUID-VkDataGraphPipelineCreateInfoARM-flags-09773", device, flags_loc,
                             "is %s (contains both NO_PROTECTED_ACCESS_BIT and PROTECTED_ACCESS_ONLY_BIT).",
                             string_VkPipelineCreateFlags2(flags).c_str());
        }
    }

    return skip;
}

bool Device::manual_PreCallValidateCreateDataGraphPipelinesARM(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                               VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                               const VkDataGraphPipelineCreateInfoARM *pCreateInfos,
                                                               const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                               const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (!enabled_features.dataGraph) {
        skip |= LogError("VUID-vkCreateDataGraphPipelinesARM-dataGraph-09760", device, error_obj.location,
                         "dataGraph feature is not enabled");
    }
    if (deferredOperation != VK_NULL_HANDLE) {
        skip |= LogError("VUID-vkCreateDataGraphPipelinesARM-deferredOperation-09761", deferredOperation,
                         error_obj.location.dot(Field::deferredOperation), "must be VK_NULL_HANDLE");
    }

    for (uint32_t i = 0; i < createInfoCount; i++) {
        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
        const VkDataGraphPipelineCreateInfoARM &create_info = pCreateInfos[i];

        skip |= ValidateCreatePipelinesFlagsCommon(create_info.flags, create_info_loc.dot(Field::flags));

        // TODO - Enable and test
        // skip |= ValidatePipelineShaderStageCreateInfoCommon(context, create_info.stage, create_info_loc.dot(Field::stage));
        // skip |= ValidatePipelineBinaryInfo(create_info.pNext, create_info.flags, pipelineCache, create_info_loc);
    }

    return skip;
}

}  // namespace stateless