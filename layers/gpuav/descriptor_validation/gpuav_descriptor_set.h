/* Copyright (c) 2023-2025 The Khronos Group Inc.
 * Copyright (c) 2023-2025 Valve Corporation
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

#pragma once

#include <atomic>
#include <mutex>
#include <vector>
#include "state_tracker/descriptor_sets.h"
#include "gpuav/resources/gpuav_vulkan_objects.h"
#include "gpuav/spirv/interface.h"
#include "containers/limits.h"

namespace gpuav {
class Validator;

// Information about how each descriptor was accessed
struct DescriptorAccess {
    uint32_t binding = vvl::kU32Max;       // binding number in the descriptor set
    uint32_t index = vvl::kU32Max;         // index into descriptor array
    uint32_t variable_id = vvl::kU32Max;   // OpVariableID
    uint32_t instruction_position = vvl::kU32Max;  // Instruction Position to map to source
    uint32_t error_logger_i = vvl::kU32Max;  // Index of error logger stored in command buffer state
};

class DescriptorSetSubState : public vvl::DescriptorSetSubState {
  public:
    DescriptorSetSubState(const vvl::DescriptorSet &set, Validator &state_data);
    virtual ~DescriptorSetSubState();

    void NotifyUpdate() override;

    VkDeviceAddress GetTypeAddress(Validator &gpuav);

    const std::vector<gpuav::spirv::BindingLayout> &GetBindingLayouts() const { return binding_layouts_; }

  private:
    void BuildBindingLayouts();

    std::vector<gpuav::spirv::BindingLayout> binding_layouts_;

    // Since we will re-bind the same descriptor set many times, keeping a version allows us to know if things have changed and
    // worth re-saving the new information
    std::atomic<uint32_t> current_version_{0};
    // Set when created the last used state
    uint32_t last_used_version_{0};
    vko::Buffer input_buffer_;

    mutable std::mutex state_lock_;
};

static inline DescriptorSetSubState &SubState(vvl::DescriptorSet &set) {
    return static_cast<DescriptorSetSubState &>(*set.SubState(LayerObjectTypeGpuAssisted));
}

}  // namespace gpuav
