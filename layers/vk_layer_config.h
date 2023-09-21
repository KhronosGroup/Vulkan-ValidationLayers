/* Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
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
 **************************************************************************/
#pragma once

#include <cstdio>
#include <string>

#include "vulkan/vk_layer.h"
#include "vulkan/vulkan.h"
#include "containers/custom_containers.h"

#if defined(WIN32)
#define DEFAULT_VK_REGISTRY_HIVE HKEY_LOCAL_MACHINE
#define DEFAULT_VK_REGISTRY_HIVE_STR "HKEY_LOCAL_MACHINE"
#define SECONDARY_VK_REGISTRY_HIVE HKEY_CURRENT_USER
#define SECONDARY_VK_REGISTRY_HIVE_STR "HKEY_CURRENT_USER"
#endif

std::string GetEnvironment(const char *variable);

enum SettingsFileSource {
    kVkConfig,
    kEnvVar,
    kLocal,
};

struct SettingsFileInfo {
    bool file_found = false;
    std::string location{};
    SettingsFileSource source = kLocal;
};

enum LogMessageTypeBits {
    kInformationBit = 0x00000001,
    kWarningBit = 0x00000002,
    kPerformanceWarningBit = 0x00000004,
    kErrorBit = 0x00000008,
    kVerboseBit = 0x00000010,
};
using LogMessageTypeFlags = VkFlags;

// Definitions for Debug Actions
enum VkLayerDbgActionBits {
    VK_DBG_LAYER_ACTION_IGNORE = 0x00000000,
    VK_DBG_LAYER_ACTION_CALLBACK = 0x00000001,
    VK_DBG_LAYER_ACTION_LOG_MSG = 0x00000002,
    VK_DBG_LAYER_ACTION_BREAK = 0x00000004,
    VK_DBG_LAYER_ACTION_DEBUG_OUTPUT = 0x00000008,
    VK_DBG_LAYER_ACTION_DEFAULT = 0x40000000,
};
using VkLayerDbgActionFlags = VkFlags;

const char *getLayerOption(const char *option);
const SettingsFileInfo *GetLayerSettingsFileInfo();

FILE *getLayerLogOutput(const char *option, const char *layer_name);
VkFlags GetLayerOptionFlags(const std::string &option,
                                            vvl::unordered_map<std::string, VkFlags> const &enum_data,
                                            uint32_t option_default);

void PrintMessageFlags(VkFlags vk_flags, char *msg_flags);
void PrintMessageSeverity(VkFlags vk_flags, char *msg_flags);
void PrintMessageType(VkFlags vk_flags, char *msg_flags);
