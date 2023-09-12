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

#pragma once

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <vector>

#include "state_tracker/base_node.h"
#include "utils/shader_utils.h"
#include "descriptor_sets.h"

// Represents a VkShaderEXT (VK_EXT_shader_object) handle
struct SHADER_OBJECT_STATE : public BASE_NODE {
    SHADER_OBJECT_STATE(ValidationStateTracker *dev_data, const VkShaderCreateInfoEXT &create_info, VkShaderEXT shader_object,
                        std::shared_ptr<SPIRV_MODULE_STATE> &spirv_module, uint32_t createInfoCount, VkShaderEXT *pShaders,
                        uint32_t unique_shader_id = 0);

    const safe_VkShaderCreateInfoEXT create_info;
    std::shared_ptr<const SPIRV_MODULE_STATE> spirv;
    std::shared_ptr<const EntryPoint> entrypoint;
    std::vector<VkShaderEXT> linked_shaders;

    // Used as way to match instrumented GPU-AV shader to a VkShaderEXT handle
    uint32_t gpu_validation_shader_id = 0;

    // NOTE: this map is 'almost' const and used in performance critical code paths.
    // The values of existing entries in the samplers_used_by_image map
    // are updated at various times. Locking requirements are TBD.
    const ActiveSlotMap active_slots;
    const uint32_t max_active_slot = 0;  // the highest set number in active_slots for pipeline layout compatibility checks

    using SetLayoutVector = std::vector<std::shared_ptr<cvdescriptorset::DescriptorSetLayout const>>;
    const SetLayoutVector set_layouts;
    const PushConstantRangesId push_constant_ranges;
    const std::vector<PipelineLayoutCompatId> set_compat_ids;

    VkShaderEXT shader() const { return handle_.Cast<VkShaderEXT>(); }
    bool IsGraphicsShaderState() const { return create_info.stage != VK_SHADER_STAGE_COMPUTE_BIT; };
    VkPrimitiveTopology GetTopology() const;
};
