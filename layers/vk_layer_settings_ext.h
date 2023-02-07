/* Copyright (c) 2020 The Khronos Group Inc.
 * Copyright (c) 2020-2023 Valve Corporation
 * Copyright (c) 2020-2023 LunarG, Inc.
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
#include "vulkan/vulkan.h"

// VK_EXT_layer_settings
//
// Name String
//   VK_EXT_layer_settings
//
// Extension Type
//   Instance extension
//
// Revision
//    1
//
// Extension and Version Dependencies
//    Requires Vulkan 1.0
//
// Contact
//    Mark Lobodzinski mark-lunarg
//
// Description
//    This extension provides the VkLayerSettingsEXT struct that can be included in the pNext chain of the
//    VkInstanceCreateInfo structure passed as the pCreateInfo parameter of vkCreateInstance
//        The structure contains an array of VkLayerSettingValueEXT structures that define layer specific settings
//    The extension also provides the vkEnumerateInstanceLayerSettingsEXT and vkEnumerateInstanceLayerSettingOptionEXT
//    commands, useful for enumerating all layer settings and their possible values, respectively.
//
// Note
//    The VK_EXT_layer_settings extension subsumes all the functionality provided in the [VK_EXT_validation_flags] extension
//    and the [VK_EXT_validation_features] extension.
//
// New Commands
//    vkEnumerateInstanceLayerSettingsEXT
//    vkEnumerateInstanceLayerSettingOptionEXT
//
// New Structures
//    array_int32
//    array_int64
//    array_float
//    array_bool
//    array_char
//    VkLayerSettingValueEXT
//    VkLayerSettingDescriptionEXT
//    VkLayerSettingOptionEXT
//    Extending VkInstanceCreateInfo :
//        VkLayerSettingsEXT
//
// New Unions
//    VkLayerSettingValueDataEXT
//
// New Enums
//    VkLayerSettingValueTypeEXT

#define VK_EXT_layer_settings 1
#define VK_EXT_LAYER_SETTINGS_SPEC_VERSION 1
#define VK_EXT_LAYER_SETTINGS_EXTENSION_NAME "VK_EXT_layer_settings"

// This extension is exclusively used by VVL, and is NOT intended as a deliverable.
// The value of the VK_STRUCTURE_TYPE is arbitrary. The only requirement,
// is that it must not conflict with existing sTypes.
constexpr VkStructureType VK_STRUCTURE_TYPE_INSTANCE_LAYER_SETTINGS_EXT =
    static_cast<VkStructureType>(VK_STRUCTURE_TYPE_MAX_ENUM - 1);

typedef enum VkLayerSettingValueTypeEXT {
    VK_LAYER_SETTING_VALUE_TYPE_UINT32_EXT,
    VK_LAYER_SETTING_VALUE_TYPE_UINT32_ARRAY_EXT,
    VK_LAYER_SETTING_VALUE_TYPE_UINT64_EXT,
    VK_LAYER_SETTING_VALUE_TYPE_UINT64_ARRAY_EXT,
    VK_LAYER_SETTING_VALUE_TYPE_FLOAT_EXT,
    VK_LAYER_SETTING_VALUE_TYPE_FLOAT_ARRAY_EXT,
    VK_LAYER_SETTING_VALUE_TYPE_BOOL_EXT,
    VK_LAYER_SETTING_VALUE_TYPE_BOOL_ARRAY_EXT,
    VK_LAYER_SETTING_VALUE_TYPE_STRING_ARRAY_EXT,
} VkLayerSettingValueTypeEXT;

typedef struct array_int32 {
    uint32_t* pInt32Array;
    uint32_t count;
} array_int32;

typedef struct array_int64 {
    uint64_t* pInt64Array;
    uint32_t count;
} array_int64;

typedef struct array_float {
    float* pFloatArray;
    uint32_t count;
} array_float;

typedef struct array_bool {
    bool* pBoolArray;
    uint32_t count;
} array_bool;

typedef struct array_char {
    const char* pCharArray;
    uint32_t count;
} array_char;

typedef union VkLayerSettingValueDataEXT {
    uint32_t value32;
    array_int32 arrayInt32;
    uint64_t value64;
    array_int64 arrayInt64;
    float valueFloat;
    array_float arrayFloat;
    VkBool32 valueBool;
    array_bool arrayBool;
    array_char arrayString;
} VkLayerSettingValueDataEXT;

typedef struct VkLayerSettingValueEXT {
    char name[VK_MAX_EXTENSION_NAME_SIZE];
    VkLayerSettingValueTypeEXT type;
    VkLayerSettingValueDataEXT data;
} VkLayerSettingValueEXT;

typedef struct VkLayerSettingsEXT {
    VkStructureType sType;
    void* pNext;
    uint32_t settingCount;
    VkLayerSettingValueEXT* pSettings;
} VkLayerSettingsEXT;

typedef struct VkLayerSettingDescriptionEXT {
    char name[VK_MAX_EXTENSION_NAME_SIZE];
    VkLayerSettingValueTypeEXT type;
    char description[VK_MAX_DESCRIPTION_SIZE];
    VkLayerSettingValueDataEXT currentValue;
    uint32_t parent;
    VkLayerSettingValueDataEXT parentState;
} VkLayerSettingDescriptionEXT;

typedef struct VkLayerSettingOptionEXT {
    char optionName[VK_MAX_EXTENSION_NAME_SIZE];
    char description[VK_MAX_DESCRIPTION_SIZE];
} VkLayerSettingOptionEXT;

VkResult vkEnumerateInstanceLayerSettingsEXT(const char* pLayerName, uint32_t* pSettingCount,
                                             VkLayerSettingDescriptionEXT* pSettings);

VkResult vkEnumerateInstanceLayerSettingOptionEXT(const char* pLayerName, const char* pSettingName, uint32_t* pSettingOptionCount,
                                                  VkLayerSettingOptionEXT* pSettingOptions);
