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

// Define CPU zones
#define VVL_ZoneScoped ZoneScoped
#define VVL_ZoneScopedN(name) ZoneScopedN(name)
#define VVL_TracyCZone(zone_name, active) TracyCZone(zone_name, active)
#define VVL_TracyCZoneEnd(zone_name) TracyCZoneEnd(zone_name)
#define VVL_TracyCFrameMark TracyCFrameMark

// Print messages
#define VVL_TracyMessage TracyMessage
#define VVL_TracyMessageStream(message)                \
    {                                                  \
        std::stringstream tracy_ss;                    \
        tracy_ss << message;                           \
        const std::string tracy_s = tracy_ss.str();    \
        TracyMessage(tracy_s.c_str(), tracy_s.size()); \
    }
#define VVL_TracyMessageMap(map, key_printer, value_printer)           \
    {                                                                  \
        static int tracy_map_log_i = 0;                                \
        std::string tracy_map_log_str = #map " ";                      \
        tracy_map_log_str += std::to_string(tracy_map_log_i++);        \
        tracy_map_log_str += " - size: ";                              \
        tracy_map_log_str += std::to_string(map.size());               \
        tracy_map_log_str += " - one pair: ";                          \
        for (const auto& [key, value] : map) {                         \
            std::string key_value_str = tracy_map_log_str;             \
            key_value_str += " | key: ";                               \
            key_value_str += key_printer(key);                         \
            key_value_str += " - value: ";                             \
            key_value_str += value_printer(value);                     \
            TracyMessage(key_value_str.c_str(), key_value_str.size()); \
        }                                                              \
    }

#else
#define VVL_ZoneScoped
#define VVL_ZoneScopedN(name)
#define VVL_TracyCZone(zone_name, active)
#define VVL_TracyCZoneEnd(zone_name)
#define VVL_TracyCFrameMark
#define VVL_TracyMessage
#define VVL_TracyMessageStream(message)
#define VVL_TracyMessageMap(map, key_printer, value_printer)
#endif

#if defined(VVL_TRACY_CPU_MEMORY)
#define VVL_TracyAlloc(ptr, size) TracySecureAlloc(ptr, size)
#define VVL_TracyFree(ptr) TracySecureFree(ptr)
#else
#define VVL_TracyAlloc(ptr, size)
#define VVL_TracyFree(ptr)
#endif
