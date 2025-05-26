/* Copyright (c) 2019-2025 The Khronos Group Inc.
 * Copyright (c) 2019-2025 Valve Corporation
 * Copyright (c) 2019-2025 LunarG, Inc.
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

#include <vulkan/utility/vk_struct_helper.hpp>
#include "vulkan/vk_layer.h"
#include "generated/vk_layer_dispatch_table.h"

typedef void *dispatch_key;
static inline dispatch_key GetDispatchKey(const void *object) { return (dispatch_key) * (VkLayerDispatchTable **)object; }

VkLayerInstanceCreateInfo *GetChainInfo(const VkInstanceCreateInfo *pCreateInfo, VkLayerFunction func);
VkLayerDeviceCreateInfo *GetChainInfo(const VkDeviceCreateInfo *pCreateInfo, VkLayerFunction func);
