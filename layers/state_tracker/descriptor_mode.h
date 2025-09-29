/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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

namespace vvl {

//
//
// use C enum to not need to cast everywhere
enum DescriptorMode {
    DescriptorModeClassic = 0,  // Vulkan 1.0
    DescriptorModeBuffer = 1,   // VK_EXT_descriptor_buffer
    DescriptorModeCount = 2,    // Used by GPU-AV to hold array for each mode

    // This means there is no "direct" bound descriptor, this is still a "valid" value if there are only push constants, if this is
    // the case, the bound pipeline/pipelineLayout can give hint which to mode to use, but this should only be done at a
    // draw/dispatch time when unknown is found.
    //
    // You can bind classic/buffer pipeline back-and-forth, but those only matter at draw/dispath time.
    // Binding descriptors, **invalidates** the other descriptor modes, so they always are used first to set the mode.
    DescriptorModeUnknown = 3
};

}  // namespace vvl