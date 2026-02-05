/* Copyright (c) 2025-2026 The Khronos Group Inc.
 * Copyright (c) 2025-2026 LunarG, Inc.
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
#include "containers/container_utils.h"
#include "utils/hash_vk_types.h"

namespace stateless {

bool Device::ValidateDataGraphProcessingEngineCreateInfoARM(const VkDataGraphProcessingEngineCreateInfoARM& engine_ci,
                                                            const Location& loc) const {
    bool skip = false;
    vvl::unordered_set<VkPhysicalDeviceDataGraphProcessingEngineARM, HashCombineDataGraphProcessingEngineARMInfo> unique_engine_set{};

    if (engine_ci.processingEngineCount > 0) {
        unique_engine_set.reserve(engine_ci.processingEngineCount);
    }

    if (!enabled_features.dataGraph) {
        skip |= LogError("VUID-VkDataGraphProcessingEngineCreateInfoARM-dataGraph-09953",
                         device,
                         loc,
                         "required to enable VkPhysicalDeviceDataGraphFeaturesARM::dataGraph feature.");
    }

    for (uint32_t engine_index = 0; engine_index < engine_ci.processingEngineCount; ++engine_index) {
        const auto& processing_engine = engine_ci.pProcessingEngines[engine_index];
        const bool is_qcom_engine = IsValueIn(processing_engine.type,
                                              { VK_PHYSICAL_DEVICE_DATA_GRAPH_PROCESSING_ENGINE_TYPE_NEURAL_QCOM,
                                                VK_PHYSICAL_DEVICE_DATA_GRAPH_PROCESSING_ENGINE_TYPE_COMPUTE_QCOM });
        const bool is_valid_processing_engine = is_qcom_engine || IsValueIn(processing_engine.type,
                                                { VK_PHYSICAL_DEVICE_DATA_GRAPH_PROCESSING_ENGINE_TYPE_DEFAULT_ARM });

        if (!unique_engine_set.insert(processing_engine).second) {
            skip |= LogError("VUID-VkDataGraphProcessingEngineCreateInfoARM-pProcessingEngines-09918",
                             device,
                             loc.dot(Field::pProcessingEngines, engine_index),
                             "element is duplicate, VkDataGraphProcessingEngineCreateInfoARM::pProcessingEngines "
                             "contains two or more identical VkPhysicalDeviceDataGraphProcessingEngineARM structures.");
        }

        if (!is_valid_processing_engine) {
            skip |= LogError("VUID-VkDataGraphProcessingEngineCreateInfoARM-pProcessingEngines-09956",
                             device,
                             loc.dot(Field::pProcessingEngines, engine_index),
                             "element has a type of %s, which is invalid.",
                             string_VkPhysicalDeviceDataGraphProcessingEngineTypeARM(processing_engine.type));
        }

        if ((is_qcom_engine) && (engine_ci.processingEngineCount != 1) && (processing_engine.isForeign)) {
            skip |= LogError("VUID-VkDataGraphProcessingEngineCreateInfoARM-pProcessingEngines-11843",
                              device,
                             loc.dot(Field::pProcessingEngines, engine_index),
                             "element with members isForeign = %u and type = %s; "
                             "while actual VkDataGraphProcessingEngineCreateInfoARM::processingEngineCount == %" PRIu32 ", "
                             "which must be equal to 1 according to Vulkan Spec.",
                             static_cast<uint32_t>(processing_engine.isForeign),
                             string_VkPhysicalDeviceDataGraphProcessingEngineTypeARM(processing_engine.type),
                             engine_ci.processingEngineCount);
        }

        if ((is_qcom_engine) && (!enabled_features.dataGraphModel)) {
            skip |= LogError("VUID-VkDataGraphProcessingEngineCreateInfoARM-pProcessingEngines-11844",
                             device,
                             loc.dot(Field::pProcessingEngines, engine_index),
                             "element has a type of %s, while VkPhysicalDeviceDataGraphModelFeaturesQCOM::dataGraphModel "
                             "feature in the VkDeviceCreateInfo pNext chain is disabled.",
                             string_VkPhysicalDeviceDataGraphProcessingEngineTypeARM(processing_engine.type));
        }
    }

    return skip;
}

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

        // Retrieve and validate data graph processing engine information
        if (const auto* processing_engine_info =
            vku::FindStructInPNextChain<VkDataGraphProcessingEngineCreateInfoARM>(create_info.pNext)) {
            const Location processing_engine_ci_loc = create_info_loc.pNext(Struct::VkDataGraphProcessingEngineCreateInfoARM);
            skip |= ValidateDataGraphProcessingEngineCreateInfoARM(*processing_engine_info, processing_engine_ci_loc);
        }

        // TODO - Enable and test
        // skip |= ValidatePipelineShaderStageCreateInfoCommon(context, create_info.stage, create_info_loc.dot(Field::stage));
        // skip |= ValidatePipelineBinaryInfo(create_info.pNext, create_info.flags, pipelineCache, create_info_loc);
    }

    return skip;
}

}  // namespace stateless