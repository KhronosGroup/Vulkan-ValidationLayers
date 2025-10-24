// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See generate_settings.py for modifications

/***************************************************************************
 *
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
// clang-format off

// This function is generated from scripts/generate_settings.py
static void ValidateLayerSettingsProvided(const VkLayerSettingsCreateInfoEXT &layer_setting_create_info,
                                          std::vector<std::string> &setting_warnings) {
    // Found that a set of <const char*> doesn't detect duplicates on all compilers
    vvl::unordered_set<std::string> used_settings;

    for (uint32_t i = 0; i < layer_setting_create_info.settingCount; i++) {
        const VkLayerSettingEXT &setting = layer_setting_create_info.pSettings[i];
        if (strcmp(OBJECT_LAYER_NAME, setting.pLayerName) != 0) {
            continue;
        }

        // used as a backup for settings not listed below
        VkLayerSettingTypeEXT required_type = VK_LAYER_SETTING_TYPE_MAX_ENUM_EXT;

        // Debugging settings are not added here as those are for internal development
        // and not designed for an app to use via VkLayerSettings API
        const char* name = setting.pSettingName;
        if (strcmp(VK_LAYER_ENABLES, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_STRING_EXT; }
        else if (strcmp(VK_LAYER_DISABLES, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_STRING_EXT; }
        else if (strcmp(VK_LAYER_CHECK_COMMAND_BUFFER, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_CHECK_IMAGE_LAYOUT, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_CHECK_OBJECT_IN_USE, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_CHECK_QUERY, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_CHECK_SHADERS, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_CHECK_SHADERS_CACHING, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_DEBUG_ACTION, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_STRING_EXT; }
        else if (strcmp(VK_LAYER_DUPLICATE_MESSAGE_LIMIT, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_UINT32_EXT; }
        else if (strcmp(VK_LAYER_ENABLE_MESSAGE_LIMIT, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_FINE_GRAINED_LOCKING, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_GPUAV_BUFFER_ADDRESS_OOB, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_GPUAV_BUFFER_COPIES, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_GPUAV_BUFFERS_VALIDATION, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_GPUAV_DESCRIPTOR_CHECKS, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_GPUAV_ENABLE, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_GPUAV_FORCE_ON_ROBUSTNESS, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_GPUAV_INDEX_BUFFERS, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_GPUAV_INDIRECT_DISPATCHES_BUFFERS, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_GPUAV_INDIRECT_DRAWS_BUFFERS, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_GPUAV_INDIRECT_TRACE_RAYS_BUFFERS, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_GPUAV_POST_PROCESS_DESCRIPTOR_INDEXING, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_GPUAV_SAFE_MODE, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_GPUAV_SELECT_INSTRUMENTED_SHADERS, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_GPUAV_SHADER_INSTRUMENTATION, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_GPUAV_SHADERS_TO_INSTRUMENT, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_STRING_EXT; }
        else if (strcmp(VK_LAYER_GPUAV_VALIDATE_RAY_QUERY, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_GPUAV_VERTEX_ATTRIBUTE_FETCH_OOB, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_LEGACY_DETECTION, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_LOG_FILENAME, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_STRING_EXT; }
        else if (strcmp(VK_LAYER_MESSAGE_FORMAT_DISPLAY_APPLICATION_NAME, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_MESSAGE_FORMAT_JSON, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_MESSAGE_ID_FILTER, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_STRING_EXT; }
        else if (strcmp(VK_LAYER_OBJECT_LIFETIME, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_PRINTF_BUFFER_SIZE, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_UINT32_EXT; }
        else if (strcmp(VK_LAYER_PRINTF_ENABLE, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_PRINTF_ONLY_PRESET, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_PRINTF_TO_STDOUT, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_PRINTF_VERBOSE, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_REPORT_FLAGS, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_STRING_EXT; }
        else if (strcmp(VK_LAYER_STATELESS_PARAM, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_SYNCVAL_MESSAGE_EXTRA_PROPERTIES, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_SYNCVAL_SHADER_ACCESSES_HEURISTIC, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_SYNCVAL_SUBMIT_TIME_VALIDATION, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_THREAD_SAFETY, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_UNIQUE_HANDLES, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_VALIDATE_BEST_PRACTICES, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_VALIDATE_BEST_PRACTICES_AMD, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_VALIDATE_BEST_PRACTICES_ARM, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_VALIDATE_BEST_PRACTICES_IMG, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_VALIDATE_BEST_PRACTICES_NVIDIA, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_VALIDATE_CORE, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else if (strcmp(VK_LAYER_VALIDATE_SYNC, name) == 0) { required_type = VK_LAYER_SETTING_TYPE_BOOL32_EXT; }
        else {
            setting_warnings.emplace_back("The setting \"" + std::string(name) +
                                          "\" in VkLayerSettingsCreateInfoEXT was not recognized by the Validation Layers. Please "
                                          "view the VkLayer_khronos_validation.json for a list of all settings.");
        }

        if (required_type != VK_LAYER_SETTING_TYPE_MAX_ENUM_EXT && setting.type != required_type) {
            setting_warnings.emplace_back(
                "The setting \"" + std::string(name) + "\" in VkLayerSettingsCreateInfoEXT was set to type " +
                std::string(string_VkLayerSettingTypeEXT(setting.type)) + " but requires type " +
                std::string(string_VkLayerSettingTypeEXT(required_type)) + " and the value may be parsed incorrectly.");
        }

        if (used_settings.count(name)) {
            setting_warnings.emplace_back(
                "The setting \"" + std::string(name) +
                "\" in VkLayerSettingsCreateInfoEXT was listed twice and only the first one listed will be recognized.");
        }
        used_settings.insert(name);
    }
}
// clang-format on
