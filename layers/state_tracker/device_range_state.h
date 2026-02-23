/* Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
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

#include <vulkan/vulkan_core.h>
#include "containers/range.h"
#include "containers/small_vector.h"

namespace vvl {
struct InternalDeviceRange {
    const vvl::range<VkDeviceAddress> range;
    const VkBufferUsageFlags2 usage;

    small_vector<VkBuffer, 2> invalidated_handles;

    InternalDeviceRange() : range(), usage() {}
    InternalDeviceRange(vvl::range<VkDeviceAddress> range, VkBufferUsageFlags2 usage) : range(range), usage(usage) {}

    InternalDeviceRange(const InternalDeviceRange& other) = default;
};
}  // namespace vvl