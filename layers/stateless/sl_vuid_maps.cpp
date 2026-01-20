/* Copyright (c) 2024-2026 The Khronos Group Inc.
 * Copyright (c) 2024-2026 LunarG, Inc.
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
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
#include "sl_vuid_maps.h"
#include "error_message/error_location.h"
#include <map>

namespace vvl {

const std::string &GetPipelineBinaryInfoVUID(const Location &loc, PipelineBinaryInfoError error) {
    static const std::map<PipelineBinaryInfoError, std::array<Entry, 5>> errors{
        {PipelineBinaryInfoError::PNext_09616,
         {{
             {Key(Func::vkCreateGraphicsPipelines), "VUID-vkCreateGraphicsPipelines-pNext-09616"},
             {Key(Func::vkCreateRayTracingPipelinesNV), "VUID-vkCreateRayTracingPipelinesNV-pNext-09616"},
             {Key(Func::vkCreateRayTracingPipelinesKHR), "VUID-vkCreateRayTracingPipelinesKHR-pNext-09616"},
             {Key(Func::vkCreateExecutionGraphPipelinesAMDX), "VUID-vkCreateExecutionGraphPipelinesAMDX-pNext-09616"},
             {Key(Func::vkCreateComputePipelines), "VUID-vkCreateComputePipelines-pNext-09616"},
         }}},
        {PipelineBinaryInfoError::PNext_09617,
         {{
             {Key(Func::vkCreateGraphicsPipelines), "VUID-vkCreateGraphicsPipelines-pNext-09617"},
             {Key(Func::vkCreateRayTracingPipelinesNV), "VUID-vkCreateRayTracingPipelinesNV-pNext-09617"},
             {Key(Func::vkCreateRayTracingPipelinesKHR), "VUID-vkCreateRayTracingPipelinesKHR-pNext-09617"},
             {Key(Func::vkCreateExecutionGraphPipelinesAMDX), "VUID-vkCreateExecutionGraphPipelinesAMDX-pNext-09617"},
             {Key(Func::vkCreateComputePipelines), "VUID-vkCreateComputePipelines-pNext-09617"},
         }}},
        {PipelineBinaryInfoError::BinaryCount_09620,
         {{
             {Key(Func::vkCreateGraphicsPipelines), "VUID-vkCreateGraphicsPipelines-binaryCount-09620"},
             {Key(Func::vkCreateRayTracingPipelinesNV), "VUID-vkCreateRayTracingPipelinesNV-binaryCount-09620"},
             {Key(Func::vkCreateRayTracingPipelinesKHR), "VUID-vkCreateRayTracingPipelinesKHR-binaryCount-09620"},
             {Key(Func::vkCreateExecutionGraphPipelinesAMDX), "VUID-vkCreateExecutionGraphPipelinesAMDX-binaryCount-09620"},
             {Key(Func::vkCreateComputePipelines), "VUID-vkCreateComputePipelines-binaryCount-09620"},
         }}},
        {PipelineBinaryInfoError::BinaryCount_09621,
         {{
             {Key(Func::vkCreateGraphicsPipelines), "VUID-vkCreateGraphicsPipelines-binaryCount-09621"},
             {Key(Func::vkCreateRayTracingPipelinesNV), "VUID-vkCreateRayTracingPipelinesNV-binaryCount-09621"},
             {Key(Func::vkCreateRayTracingPipelinesKHR), "VUID-vkCreateRayTracingPipelinesKHR-binaryCount-09621"},
             {Key(Func::vkCreateExecutionGraphPipelinesAMDX), "VUID-vkCreateExecutionGraphPipelinesAMDX-binaryCount-09621"},
             {Key(Func::vkCreateComputePipelines), "VUID-vkCreateComputePipelines-binaryCount-09621"},
         }}},
        {PipelineBinaryInfoError::BinaryCount_09622,
         {{
             {Key(Func::vkCreateGraphicsPipelines), "VUID-vkCreateGraphicsPipelines-binaryCount-09622"},
             {Key(Func::vkCreateRayTracingPipelinesNV), "VUID-vkCreateRayTracingPipelinesNV-binaryCount-09622"},
             {Key(Func::vkCreateRayTracingPipelinesKHR), "VUID-vkCreateRayTracingPipelinesKHR-binaryCount-09622"},
             {Key(Func::vkCreateExecutionGraphPipelinesAMDX), "VUID-vkCreateExecutionGraphPipelinesAMDX-binaryCount-09622"},
             {Key(Func::vkCreateComputePipelines), "VUID-vkCreateComputePipelines-binaryCount-09622"},
         }}},
    };

    const auto &result = FindVUID(error, loc, errors);
    assert(!result.empty());
    if (result.empty()) {
        static const std::string unhandled("UNASSIGNED-Stateless-unhandled-pipelinebinaryinfo-error");
        return unhandled;
    }
    return result;
}

// clang-format off
const char *GetPipelineCreateFlagVUID(const Location &loc, PipelineCreateFlagError error) {
    switch (error) {
        case PipelineCreateFlagError::CacheControl_02878:
            return
                loc.function == Func::vkCreateGraphicsPipelines       ? "VUID-VkGraphicsPipelineCreateInfo-pipelineCreationCacheControl-02878" :
                loc.function == Func::vkCreateComputePipelines        ? "VUID-VkComputePipelineCreateInfo-pipelineCreationCacheControl-02878" :
                loc.function == Func::vkCreateRayTracingPipelinesKHR  ? "VUID-VkRayTracingPipelineCreateInfoKHR-pipelineCreationCacheControl-02878" :
                loc.function == Func::vkCreateRayTracingPipelinesNV   ? "VUID-VkRayTracingPipelineCreateInfoNV-pipelineCreationCacheControl-02878" :
                loc.function == Func::vkCreateDataGraphPipelinesARM   ? "VUID-VkDataGraphPipelineCreateInfoARM-pipelineCreationCacheControl-09871" :
                kVUIDUndefined;
        case PipelineCreateFlagError::Shader64BitIndexing_11798:
            return
                loc.function == Func::vkCreateGraphicsPipelines       ? "VUID-VkGraphicsPipelineCreateInfo-flags-11798" :
                loc.function == Func::vkCreateComputePipelines        ? "VUID-VkComputePipelineCreateInfo-flags-11798" :
                loc.function == Func::vkCreateRayTracingPipelinesKHR  ? "VUID-VkRayTracingPipelineCreateInfoKHR-flags-11798" :
                loc.function == Func::vkCreateRayTracingPipelinesNV   ? "VUID-VkRayTracingPipelineCreateInfoNV-flags-11798" :
                // TOOD - Missing vkCreateDataGraphPipelinesARM
                kVUIDUndefined;
        case PipelineCreateFlagError::ProtectedAccess_07368:
            return
                loc.function == Func::vkCreateGraphicsPipelines       ? "VUID-VkGraphicsPipelineCreateInfo-pipelineProtectedAccess-07368" :
                loc.function == Func::vkCreateComputePipelines        ? "VUID-VkComputePipelineCreateInfo-pipelineProtectedAccess-07368" :
                loc.function == Func::vkCreateRayTracingPipelinesKHR  ? "VUID-VkRayTracingPipelineCreateInfoKHR-pipelineProtectedAccess-07368" :
                loc.function == Func::vkCreateRayTracingPipelinesNV   ? "VUID-VkRayTracingPipelineCreateInfoNV-pipelineProtectedAccess-07368" :
                loc.function == Func::vkCreateDataGraphPipelinesARM   ? "VUID-VkDataGraphPipelineCreateInfoARM-pipelineProtectedAccess-09772" :
                kVUIDUndefined;
        case PipelineCreateFlagError::ProtectedAccess_07369:
            return
                loc.function == Func::vkCreateGraphicsPipelines       ? "VUID-VkGraphicsPipelineCreateInfo-flags-07369" :
                loc.function == Func::vkCreateComputePipelines        ? "VUID-VkComputePipelineCreateInfo-flags-07369" :
                loc.function == Func::vkCreateRayTracingPipelinesKHR  ? "VUID-VkRayTracingPipelineCreateInfoKHR-flags-07369" :
                loc.function == Func::vkCreateRayTracingPipelinesNV   ? "VUID-VkRayTracingPipelineCreateInfoNV-flags-07369" :
                loc.function == Func::vkCreateDataGraphPipelinesARM   ? "VUID-VkDataGraphPipelineCreateInfoARM-flags-09773" :
                kVUIDUndefined;
    }
    return "UNASSIGNED-CoreChecks-unhandled-pipeline-create-flage";
}
// clang-format on

}  // namespace vvl