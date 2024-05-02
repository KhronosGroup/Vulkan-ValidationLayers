/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
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

#if defined(TRACY_ENABLE)
#include "tracy/Tracy.hpp"
#include "tracy/TracyC.h"
#include "tracy/../client/TracyProfiler.hpp"

#define VVL_ZoneScoped ZoneScoped
#define VVL_TracyCZone(zone_name, active) TracyCZone(zone_name, active)
#define VVL_TracyCZoneEnd(zone_name) TracyCZoneEnd(zone_name)
#define VVL_TracyCFrameMark TracyCFrameMark
#define VVL_TracyMessage TracyMessage
#else
#define VVL_ZoneScoped
#define VVL_TracyCZone(zone_name, active)
#define VVL_TracyCZoneEnd(zone_name)
#define VVL_TracyCFrameMark
#define VVL_TracyMessage
#endif

#if defined(VVL_TRACY_CPU_MEMORY)
#define VVL_TracyAlloc(ptr, size) TracyAlloc(ptr, size)
#define VVL_TracyFree(ptr) TracyFree(ptr)
#else
#define VVL_TracyAlloc(ptr, size)
#define VVL_TracyFree(ptr)
#endif
