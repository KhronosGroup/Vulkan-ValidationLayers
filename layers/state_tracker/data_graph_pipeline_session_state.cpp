/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (C) 2025 Arm Limited.
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
#include "state_tracker/data_graph_pipeline_session_state.h"
#include "generated/dispatch_functions.h"

namespace vvl {

DataGraphPipelineSession::DataGraphPipelineSession(DeviceState &dev_data, VkDataGraphPipelineSessionARM handle,
                                                   const VkDataGraphPipelineSessionCreateInfoARM *pCreateInfo)
    : StateObject(handle, kVulkanObjectTypeDataGraphPipelineSessionARM),
      safe_create_info(pCreateInfo),
      create_info(*safe_create_info.ptr()),
      session_(handle),
      unprotected_((pCreateInfo->flags & VK_DATA_GRAPH_PIPELINE_SESSION_CREATE_PROTECTED_BIT_ARM) == 0) {
}

void DataGraphPipelineSession::InitMemoryRequirements(VkDevice device, const VkDataGraphPipelineSessionBindPointRequirementARM *p_bind_point_reqs, uint32_t n_reqs) {

    bind_point_reqs_.resize(n_reqs);
    memcpy(bind_point_reqs_.data(), p_bind_point_reqs, n_reqs * sizeof(VkDataGraphPipelineSessionBindPointRequirementARM));

    mem_reqs_map_.clear();
    for (uint32_t j = 0; j < n_reqs; j++) {
        const auto& bpr = p_bind_point_reqs[j];
        if (VK_DATA_GRAPH_PIPELINE_SESSION_BIND_POINT_TYPE_MEMORY_ARM == bpr.bindPointType) {
            for (uint32_t i = 0; i < bpr.numObjects; i++) {
                VkDataGraphPipelineSessionMemoryRequirementsInfoARM session_mem_info = vku::InitStructHelper();
                session_mem_info.bindPoint = bpr.bindPoint;
                session_mem_info.session = session_;
                session_mem_info.objectIndex = i;
                VkMemoryRequirements2 mem_reqs = vku::InitStructHelper();
                DispatchGetDataGraphPipelineSessionMemoryRequirementsARM(device, &session_mem_info, &mem_reqs);
                mem_reqs_map_[bpr.bindPoint].push_back(mem_reqs.memoryRequirements);
            }
        }
    }
}

void DataGraphPipelineSession::AddBoundMemory(VkDataGraphPipelineSessionBindPointARM bind_point, MemoryBinding binding) {
    bound_memory_map_[bind_point].push_back(binding);
}

const VkDataGraphPipelineSessionBindPointRequirementARM *DataGraphPipelineSession::FindBindPointRequirement(VkDataGraphPipelineSessionBindPointARM bind_point) const {
    for (auto &bpr : bind_point_reqs_) {
        if (bpr.bindPoint == bind_point) {
            return &bpr;
        }
    }
    return nullptr;
}
}  // namespace vvl
