/* Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
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

#include "gpuav/descriptor_validation/gpuav_descriptor_validation.h"

#include "containers/custom_containers.h"
#include "drawdispatch/descriptor_validator.h"
#include "gpuav/core/gpuav.h"
#include "gpuav/resources/gpuav_state_trackers.h"
#include "gpuav/resources/gpuav_shader_resources.h"
#include "state_tracker/pipeline_state.h"
#include "state_tracker/shader_module.h"

#include "profiling/profiling.h"
#include "state_tracker/shader_object_state.h"

namespace gpuav {
namespace descriptor {

void UpdateBoundDescriptors(Validator &gpuav, CommandBufferSubState &cb_state, VkPipelineBindPoint pipeline_bind_point,
                            const Location &loc) {
    // If DescriptorSetBindings has not been created at this point, then no validation needs it,
    // so skip updating it.
    DescriptorSetBindings *desc_set_bindings = cb_state.shared_resources_cache.TryGet<DescriptorSetBindings>();
    if (!desc_set_bindings) {
        return;
    }

    const auto lv_bind_point = ConvertToLvlBindPoint(pipeline_bind_point);
    auto const &last_bound = cb_state.base.lastBound[lv_bind_point];

    const size_t number_of_sets = last_bound.ds_slots.size();
    if (number_of_sets == 0) {
        return;  // empty bind
    } else if (number_of_sets > glsl::kDebugInputBindlessMaxDescSets) {
        gpuav.InternalError(cb_state.VkHandle(), loc, "Binding more than kDebugInputBindlessMaxDescSets limit");
        return;
    }

    DescriptorSetBindings::BindingCommand descriptor_binding_cmd{};
    descriptor_binding_cmd.bound_descriptor_sets.reserve(number_of_sets);
    // Currently we loop through the sets multiple times to reduce complexity and seperate the various parts, can revisit if we find
    // this is actually a perf bottleneck (assume number of sets are low as people we will then to have a single large set)
    for (uint32_t i = 0; i < number_of_sets; i++) {
        const auto &ds_slot = last_bound.ds_slots[i];
        descriptor_binding_cmd.bound_descriptor_sets.emplace_back(ds_slot.ds_state);
    }

    for (auto &descriptor_binding_func : desc_set_bindings->on_update_bound_descriptor_sets) {
        descriptor_binding_func(gpuav, cb_state, descriptor_binding_cmd);
    }
    desc_set_bindings->descriptor_set_binding_commands.emplace_back(std::move(descriptor_binding_cmd));
}
}  // namespace descriptor
}  // namespace gpuav
