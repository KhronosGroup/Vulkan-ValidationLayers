/*
 * Copyright (c) 2023 LunarG, Inc.
 * Copyright (c) 2023 Valve Corporation
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

// This file list all VUID that are no possible to validate.
// This file should never be included, but here for searchability and statistics

const char* unimplementable_validation[] = {
    // TODO - This might be able to be rewritten as how VUID-VkWaylandSurfaceCreateInfoKHR-display-01304 is
    //
    // Not possible to validate a "valid" handle for fd
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5431#issuecomment-1484167442
    "VUID-vkGetMemoryFdPropertiesKHR-fd-00673",

    // sparseAddressSpaceSize can't be tracked in a layer
    // https://gitlab.khronos.org/vulkan/vulkan/-/issues/2403
    "VUID-vkCreateBuffer-flags-00911"};
