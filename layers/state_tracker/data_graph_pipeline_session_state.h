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
#pragma once
#include <vector>
#include "device_memory_state.h"
#include "state_object.h"
#include "state_tracker/device_memory_state.h"

namespace vvl {

class DeviceState;

class DataGraphPipelineSession : public StateObject {
  public:
    const vku::safe_VkDataGraphPipelineSessionCreateInfoARM safe_create_info;
    const VkDataGraphPipelineSessionCreateInfoARM &create_info;

    explicit DataGraphPipelineSession(DeviceState &dev_data, VkDataGraphPipelineSessionARM handle,
                                      const VkDataGraphPipelineSessionCreateInfoARM *pCreateInfo);

    bool Unprotected() const { return unprotected_; }
    const std::vector<VkDataGraphPipelineSessionBindPointRequirementARM> &BindPointReqs() const { return bind_point_reqs_; }
    const std::unordered_map<VkDataGraphPipelineSessionBindPointARM, std::vector<VkMemoryRequirements>> &MemReqsMap() const { return mem_reqs_map_; }
    const std::unordered_map<VkDataGraphPipelineSessionBindPointARM, std::vector<vvl::MemoryBinding>> &BoundMemoryMap() const { return bound_memory_map_; }

    void InitMemoryRequirements(VkDevice device, const VkDataGraphPipelineSessionBindPointRequirementARM *p_bind_point_reqs, uint32_t n_reqs);
    void AddBoundMemory(VkDataGraphPipelineSessionBindPointARM bind_point, MemoryBinding binding);
    const VkDataGraphPipelineSessionBindPointRequirementARM *FindBindPointRequirement(VkDataGraphPipelineSessionBindPointARM bind_point) const;

  private:
    VkDataGraphPipelineSessionARM session_;
    bool unprotected_ = true;
    std::vector<VkDataGraphPipelineSessionBindPointRequirementARM> bind_point_reqs_;
    std::unordered_map<VkDataGraphPipelineSessionBindPointARM, std::vector<VkMemoryRequirements>> mem_reqs_map_;    /* requirements */
    std::unordered_map<VkDataGraphPipelineSessionBindPointARM, std::vector<vvl::MemoryBinding>> bound_memory_map_;  /* actually bound */
};
}  // namespace vvl
