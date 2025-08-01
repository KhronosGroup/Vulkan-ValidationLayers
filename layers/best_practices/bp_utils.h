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

#include "best_practices/bp_constants.h"
#include "state_tracker/image_state.h"
#include "utils/image_utils.h"
#include "layer_options.h"

bool IsVendorCheckEnabled(const ValidationEnabled& enabled, BPVendorFlags vendors);
const char* VendorSpecificTag(BPVendorFlags vendors);

template <typename Func>
static inline void ForEachSubresource(const vvl::Image& image, const VkImageSubresourceRange& range, Func&& func) {
    const uint32_t layer_count = GetEffectiveLayerCount(range, image.full_range.layerCount);
    const uint32_t level_count = GetEffectiveLevelCount(range, image.full_range.levelCount);

    for (uint32_t i = 0; i < layer_count; ++i) {
        const uint32_t layer = range.baseArrayLayer + i;
        for (uint32_t j = 0; j < level_count; ++j) {
            const uint32_t level = range.baseMipLevel + j;
            func(layer, level);
        }
    }
}
