/* Copyright (c) 2018-2025 The Khronos Group Inc.
 * Copyright (c) 2018-2026 Valve Corporation
 * Copyright (c) 2018-2026 LunarG, Inc.
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

#include "gpuav/core/gpuav.h"
#include "gpuav/core/gpuav_validation_pipeline.h"
#include "gpuav/resources/gpuav_state_trackers.h"

struct Location;
namespace gpuav {
namespace valcmd {

class ValidationCommandsGpuavState {
  public:
    ValidationCommandsGpuavState(Validator& gpuav, const Location& loc);
    ~ValidationCommandsGpuavState();

    struct DescSetMode {
        VkDescriptorSetLayout error_logging_desc_set_layout_ = VK_NULL_HANDLE;
    } desc_set_mode;

    auto& GetBindings() const { return validation_cmd_bindings; };

  private:
    Validator& gpuav_;

    std::array<VkDescriptorSetLayoutBinding, 4> validation_cmd_bindings = {};
};

class ValidationCommandsCbState {
  public:
    ValidationCommandsCbState(Validator& gpuav, CommandBufferSubState& cb, VkDescriptorSetLayout error_logging_desc_set_layout,
                              const Location& loc);
    ~ValidationCommandsCbState();

    VkDescriptorSet error_logging_desc_set_ = VK_NULL_HANDLE;
    VkDescriptorPool validation_cmd_desc_pool_ = VK_NULL_HANDLE;

  private:
    Validator& gpuav_;
};
}  // namespace valcmd

}  // namespace gpuav
