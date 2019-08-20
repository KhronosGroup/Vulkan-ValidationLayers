/* Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (C) 2015-2019 Google Inc.
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
 *
 * Mark Lobodzinski <mark@lunarg.com>
 * Dave Houlton <daveh@lunarg.com>
 */
#ifndef CORE_VALIDATION_BUFFER_VALIDATION_H_
#define CORE_VALIDATION_BUFFER_VALIDATION_H_

#include "vulkan/vk_layer.h"
#include <limits.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include <utility>
#include <algorithm>
#include <bitset>

uint32_t FullMipChainLevels(uint32_t height, uint32_t width = 1, uint32_t depth = 1);
uint32_t FullMipChainLevels(VkExtent3D);
uint32_t FullMipChainLevels(VkExtent2D);

uint32_t ResolveRemainingLevels(const VkImageSubresourceRange *range, uint32_t mip_levels);

uint32_t ResolveRemainingLayers(const VkImageSubresourceRange *range, uint32_t layers);
VkImageSubresourceRange NormalizeSubresourceRange(const IMAGE_STATE &image_state, const VkImageSubresourceRange &range);

#endif  // CORE_VALIDATION_BUFFER_VALIDATION_H_
