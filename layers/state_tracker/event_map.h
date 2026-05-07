/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include "vulkan/vulkan.h"
#include "containers/custom_containers.h"
#include <vulkan/utility/vk_safe_struct.hpp>

// TODO: this is very similar to EventSignalingState (but signal field has different semantics).
// This will be reworked/removed soon.
struct EventInfo {
    VkPipelineStageFlags2 src_stage_mask = VK_PIPELINE_STAGE_2_NONE;
    bool signal = false;  // signal (SetEvent) or unsignal (ResetEvent)
    vku::safe_VkDependencyInfo dependency_info = {};
};
using EventMap = vvl::unordered_map<VkEvent, EventInfo>;

struct EventSignalingState {
    // Tracks how the event signaling state changes as command buffer recording progresses.
    // When recording is finished, this is the event state at the end of the command buffer
    bool signaled = false;

    // If signaled is true, this is the stage mask specified by the CmdSetEvent command.
    // If signaled is false, this is NONE
    VkPipelineStageFlags2 src_stage_mask = VK_PIPELINE_STAGE_2_NONE;

    // If signaled is true and the event was set by CmdSetEvent2, this is the dependency info from that command
    vku::safe_VkDependencyInfo dependency_info = {};

    EventSignalingState(bool signaled, VkPipelineStageFlags2 src_stage_mask = VK_PIPELINE_STAGE_2_NONE,
                        vku::safe_VkDependencyInfo dependency_info = {})
        : signaled(signaled), src_stage_mask(src_stage_mask), dependency_info(dependency_info) {}
};
