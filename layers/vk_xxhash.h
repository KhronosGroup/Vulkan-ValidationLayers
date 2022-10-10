/* Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (C) 2015-2022 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
 * Author: Juan Ramos
 */

// XXH_NO_LONG_LONG: removes compilation of algorithms relying on 64-bit types (XXH3 and XXH64). Only XXH32 will be compiled.
// We only need XXH32 due to restrictions requiring a 32 bit hash. This also reduces binary size.
//
// v0.8.1 also has compilation issues that are removed by setting this define.
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/4639
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4640
#define XXH_NO_LONG_LONG

#include "xxhash.h"
