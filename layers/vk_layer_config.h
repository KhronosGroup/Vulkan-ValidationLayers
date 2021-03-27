/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
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
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 **************************************************************************/
#pragma once

#include <stdio.h>
#include <string>

#include "vulkan/vk_layer.h"
#include "vulkan/vulkan.h"
#include "vk_layer_data.h"

#if defined(WIN32)
#define DEFAULT_VK_REGISTRY_HIVE HKEY_LOCAL_MACHINE
#define DEFAULT_VK_REGISTRY_HIVE_STR "HKEY_LOCAL_MACHINE"
#define SECONDARY_VK_REGISTRY_HIVE HKEY_CURRENT_USER
#define SECONDARY_VK_REGISTRY_HIVE_STR "HKEY_CURRENT_USER"
#endif

std::string GetEnvironment(const char *variable);

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    kVkConfig,
    kEnvVar,
    kLocal,
} SettingsFileSource;

typedef struct SettingsFileInfo {
    bool file_found = false;
    std::string location{};
    SettingsFileSource source = kLocal;
} SettingsFileInfo;

typedef enum {
    kInformationBit = 0x00000001,
    kWarningBit = 0x00000002,
    kPerformanceWarningBit = 0x00000004,
    kErrorBit = 0x00000008,
    kDebugBit = 0x00000010,
} LogMessageTypeBits;
typedef VkFlags LogMessageTypeFlags;

// Definitions for Debug Actions
typedef enum VkLayerDbgActionBits {
    VK_DBG_LAYER_ACTION_IGNORE = 0x00000000,
    VK_DBG_LAYER_ACTION_CALLBACK = 0x00000001,
    VK_DBG_LAYER_ACTION_LOG_MSG = 0x00000002,
    VK_DBG_LAYER_ACTION_BREAK = 0x00000004,
    VK_DBG_LAYER_ACTION_DEBUG_OUTPUT = 0x00000008,
    VK_DBG_LAYER_ACTION_DEFAULT = 0x40000000,
} VkLayerDbgActionBits;
typedef VkFlags VkLayerDbgActionFlags;

const layer_data::unordered_map<std::string, VkFlags> debug_actions_option_definitions = {
    {std::string("VK_DBG_LAYER_ACTION_IGNORE"), VK_DBG_LAYER_ACTION_IGNORE},
    {std::string("VK_DBG_LAYER_ACTION_CALLBACK"), VK_DBG_LAYER_ACTION_CALLBACK},
    {std::string("VK_DBG_LAYER_ACTION_LOG_MSG"), VK_DBG_LAYER_ACTION_LOG_MSG},
    {std::string("VK_DBG_LAYER_ACTION_BREAK"), VK_DBG_LAYER_ACTION_BREAK},
#if defined(WIN32)
    {std::string("VK_DBG_LAYER_ACTION_DEBUG_OUTPUT"), VK_DBG_LAYER_ACTION_DEBUG_OUTPUT},
#endif
    {std::string("VK_DBG_LAYER_ACTION_DEFAULT"), VK_DBG_LAYER_ACTION_DEFAULT}};

const layer_data::unordered_map<std::string, VkFlags> report_flags_option_definitions = {
    {std::string("warn"), VK_DEBUG_REPORT_WARNING_BIT_EXT},
    {std::string("info"), VK_DEBUG_REPORT_INFORMATION_BIT_EXT},
    {std::string("perf"), VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT},
    {std::string("error"), VK_DEBUG_REPORT_ERROR_BIT_EXT},
    {std::string("debug"), VK_DEBUG_REPORT_DEBUG_BIT_EXT}};

const layer_data::unordered_map<std::string, VkFlags> log_msg_type_option_definitions = {{std::string("warn"), kWarningBit},
                                                                                  {std::string("info"), kInformationBit},
                                                                                  {std::string("perf"), kPerformanceWarningBit},
                                                                                  {std::string("error"), kErrorBit},
                                                                                  {std::string("debug"), kDebugBit}};

VK_LAYER_EXPORT const char *getLayerOption(const char *option);
VK_LAYER_EXPORT const char *GetLayerEnvVar(const char *option);
VK_LAYER_EXPORT const SettingsFileInfo *GetLayerSettingsFileInfo();

VK_LAYER_EXPORT FILE *getLayerLogOutput(const char *option, const char *layer_name);
VK_LAYER_EXPORT VkFlags GetLayerOptionFlags(std::string option, layer_data::unordered_map<std::string, VkFlags> const &enum_data,
                                            uint32_t option_default);

VK_LAYER_EXPORT void setLayerOption(const char *option, const char *val);
VK_LAYER_EXPORT void PrintMessageFlags(VkFlags vk_flags, char *msg_flags);
VK_LAYER_EXPORT void PrintMessageSeverity(VkFlags vk_flags, char *msg_flags);
VK_LAYER_EXPORT void PrintMessageType(VkFlags vk_flags, char *msg_flags);

#ifdef __cplusplus
}
#endif
