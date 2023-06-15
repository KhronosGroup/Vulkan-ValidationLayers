/* Copyright (c) 2020-2023 The Khronos Group Inc.
 * Copyright (c) 2020-2023 Valve Corporation
 * Copyright (c) 2020-2023 LunarG, Inc.
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
 */

#include "layer_options.h"
#include "xxhash.h"

// Include new / delete overrides if using mimalloc. This needs to be include exactly once in a file that is
// part of the VVL but not the layer utils library.
#if defined(USE_MIMALLOC) && defined(_WIN64)
#include "mimalloc-new-delete.h"
#endif

const char *SETTING_ENABLES = "enables";
const char *SETTING_VALIDATE_BEST_PRACTICES = "validate_best_practices";
const char *SETTING_VALIDATE_BEST_PRACTICES_ARM = "validate_best_practices_arm";
const char *SETTING_VALIDATE_BEST_PRACTICES_AMD = "validate_best_practices_amd";
const char *SETTING_VALIDATE_BEST_PRACTICES_IMG = "validate_best_practices_img";
const char *SETTING_VALIDATE_BEST_PRACTICES_NVIDIA = "validate_best_practices_nvidia";
const char *SETTING_VALIDATE_SYNC = "validate_sync";
const char *SETTING_VALIDATE_SYNC_QUEUE_SUBMIT = "sync_queue_submit";
const char *SETTING_VALIDATE_GPU_BASED = "validate_gpu_based";
const char *SETTING_RESERVE_BINDING_SLOT = "reserve_binding_slot";

const char *SETTING_DISABLES = "disables";
const char *SETTING_STATELESS_PARAM = "stateless_param";
const char *SETTING_THREAD_SAFETY = "thread_safety";
const char *SETTING_VALIDATE_CORE = "validate_core";
const char *SETTING_CHECK_COMMAND_BUFFER = "check_command_buffer";
const char *SETTING_CHECK_OBJECT_IN_USE = "check_object_in_use";
const char *SETTING_CHECK_QUERY = "check_query";
const char *SETTING_CHECK_IMAGE_LAYOUT = "check_image_layout";
const char *SETTING_UNIQUE_HANDLES = "unique_handles";
const char *SETTING_OBJECT_LIFETIME = "object_lifetime";
const char *SETTING_CHECK_SHADERS = "check_shaders";
const char *SETTING_CHECK_SHADERS_CACHING = "check_shaders_caching";

const char *SETTING_MESSAGE_ID_FILTER = "message_id_filter";
const char *SETTING_CUSTOM_STYPE_LIST = "custom_stype_list";
const char *SETTING_DUPLICATE_MESSAGE_LIMIT = "duplicate_message_limit";
const char *SETTING_FINE_GRAINED_LOCKING = "fine_grained_locking";

// Set the local disable flag for the appropriate VALIDATION_CHECK_DISABLE enum
void SetValidationDisable(CHECK_DISABLED &disable_data, const ValidationCheckDisables disable_id) {
    switch (disable_id) {
        case VALIDATION_CHECK_DISABLE_COMMAND_BUFFER_STATE:
            disable_data[command_buffer_state] = true;
            break;
        case VALIDATION_CHECK_DISABLE_OBJECT_IN_USE:
            disable_data[object_in_use] = true;
            break;
        case VALIDATION_CHECK_DISABLE_QUERY_VALIDATION:
            disable_data[query_validation] = true;
            break;
        case VALIDATION_CHECK_DISABLE_IMAGE_LAYOUT_VALIDATION:
            disable_data[image_layout_validation] = true;
            break;
        default:
            assert(true);
    }
}

// Set the local disable flag for a single VK_VALIDATION_FEATURE_DISABLE_* flag
void SetValidationFeatureDisable(CHECK_DISABLED &disable_data, const VkValidationFeatureDisableEXT feature_disable) {
    switch (feature_disable) {
        case VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT:
            disable_data[shader_validation] = true;
            break;
        case VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT:
            disable_data[thread_safety] = true;
            break;
        case VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT:
            disable_data[stateless_checks] = true;
            break;
        case VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT:
            disable_data[object_tracking] = true;
            break;
        case VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT:
            disable_data[core_checks] = true;
            break;
        case VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT:
            disable_data[handle_wrapping] = true;
            break;
        case VK_VALIDATION_FEATURE_DISABLE_SHADER_VALIDATION_CACHE_EXT:
            disable_data[shader_validation_caching] = true;
            break;
        case VK_VALIDATION_FEATURE_DISABLE_ALL_EXT:
            // Set all disabled flags to true
            std::fill(disable_data.begin(), disable_data.end(), true);
            break;
        default:
            break;
    }
}

// Set the local enable flag for the appropriate VALIDATION_CHECK_ENABLE enum
void SetValidationEnable(CHECK_ENABLED &enable_data, const ValidationCheckEnables enable_id) {
    switch (enable_id) {
        case VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ARM:
            enable_data[vendor_specific_arm] = true;
            break;
        case VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_AMD:
            enable_data[vendor_specific_amd] = true;
            break;
        case VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_IMG:
            enable_data[vendor_specific_img] = true;
            break;
        case VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_NVIDIA:
            enable_data[vendor_specific_nvidia] = true;
            break;
        case VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ALL:
            enable_data[vendor_specific_arm] = true;
            enable_data[vendor_specific_amd] = true;
            enable_data[vendor_specific_img] = true;
            enable_data[vendor_specific_nvidia] = true;
            break;
        case VALIDATION_CHECK_ENABLE_SYNCHRONIZATION_VALIDATION_QUEUE_SUBMIT:
            enable_data[sync_validation_queue_submit] = true;
            break;
        default:
            assert(true);
    }
}

// Set the local enable flag for a single VK_VALIDATION_FEATURE_ENABLE_* flag
void SetValidationFeatureEnable(CHECK_ENABLED &enable_data, const VkValidationFeatureEnableEXT feature_enable) {
    switch (feature_enable) {
        case VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT:
            enable_data[gpu_validation] = true;
            break;
        case VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT:
            enable_data[gpu_validation_reserve_binding_slot] = true;
            break;
        case VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT:
            enable_data[best_practices] = true;
            break;
        case VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT:
            enable_data[debug_printf] = true;
            break;
        case VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT:
            enable_data[sync_validation] = true;
            break;
        default:
            break;
    }
}

void SetValidationFeatureEnable2(CHECK_ENABLED &enable_data, const VkValidationFeatureEnable feature_enable) {
    switch (feature_enable) {
        case VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION:
            enable_data[sync_validation] = true;
            break;
        default:
            break;
    }
}

// Set the local disable flag for settings specified through the VK_EXT_validation_flags extension
void SetValidationFlags(CHECK_DISABLED &disables, const VkValidationFlagsEXT *val_flags_struct) {
    for (uint32_t i = 0; i < val_flags_struct->disabledValidationCheckCount; ++i) {
        switch (val_flags_struct->pDisabledValidationChecks[i]) {
            case VK_VALIDATION_CHECK_SHADERS_EXT:
                disables[shader_validation] = true;
                break;
            case VK_VALIDATION_CHECK_ALL_EXT:
                // Set all disabled flags to true
                disables[shader_validation] = true;
                break;
            default:
                break;
        }
    }
}

// Process Validation Features flags specified through the ValidationFeature extension
void SetValidationFeatures(CHECK_DISABLED &disable_data, CHECK_ENABLED &enable_data,
                           const VkValidationFeaturesEXT *val_features_struct) {
    for (uint32_t i = 0; i < val_features_struct->disabledValidationFeatureCount; ++i) {
        SetValidationFeatureDisable(disable_data, val_features_struct->pDisabledValidationFeatures[i]);
    }
    for (uint32_t i = 0; i < val_features_struct->enabledValidationFeatureCount; ++i) {
        SetValidationFeatureEnable(enable_data, val_features_struct->pEnabledValidationFeatures[i]);
    }
}

std::string GetNextToken(std::string *token_list, const std::string &delimiter, size_t *pos) {
    std::string token;
    *pos = token_list->find(delimiter);
    if (*pos != std::string::npos) {
        token = token_list->substr(0, *pos);
    } else {
        *pos = token_list->length() - delimiter.length();
        token = *token_list;
    }
    token_list->erase(0, *pos + delimiter.length());

    // Remove quotes from quoted strings
    if ((token.length() > 0) && (token[0] == '\"')) {
        token.erase(token.begin());
        if ((token.length() > 0) && (token[token.length() - 1] == '\"')) {
            token.erase(--token.end());
        }
    }
    return token;
}

// Given a string representation of a list of enable enum values, call the appropriate setter function
void SetLocalEnableSetting(std::string list_of_enables, const std::string &delimiter, CHECK_ENABLED &enables) {
    size_t pos = 0;
    std::string token;
    while (list_of_enables.length() != 0) {
        token = GetNextToken(&list_of_enables, delimiter, &pos);
        if (token.find("VK_VALIDATION_FEATURE_ENABLE_") != std::string::npos) {
            auto result = VkValFeatureEnableLookup.find(token);
            if (result != VkValFeatureEnableLookup.end()) {
                SetValidationFeatureEnable(enables, result->second);
            } else {
                auto result2 = VkValFeatureEnableLookup2.find(token);
                if (result2 != VkValFeatureEnableLookup2.end()) {
                    SetValidationFeatureEnable2(enables, result2->second);
                }
            }
        } else if (token.find("VALIDATION_CHECK_ENABLE_") != std::string::npos) {
            auto result = ValidationEnableLookup.find(token);
            if (result != ValidationEnableLookup.end()) {
                SetValidationEnable(enables, result->second);
            }
        }
    }
}

// Given a string representation of a list of disable enum values, call the appropriate setter function
void SetLocalDisableSetting(std::string list_of_disables, const std::string &delimiter, CHECK_DISABLED &disables) {
    size_t pos = 0;
    std::string token;
    while (list_of_disables.length() != 0) {
        token = GetNextToken(&list_of_disables, delimiter, &pos);
        if (token.find("VK_VALIDATION_FEATURE_DISABLE_") != std::string::npos) {
            auto result = VkValFeatureDisableLookup.find(token);
            if (result != VkValFeatureDisableLookup.end()) {
                SetValidationFeatureDisable(disables, result->second);
            }
        } else if (token.find("VALIDATION_CHECK_DISABLE_") != std::string::npos) {
            auto result = ValidationDisableLookup.find(token);
            if (result != ValidationDisableLookup.end()) {
                SetValidationDisable(disables, result->second);
            }
        }
    }
}

uint32_t TokenToUint(std::string &token) {
    uint32_t int_id = 0;
    if ((token.find("0x") == 0) || token.find("0X") == 0) {  // Handle hex format
        int_id = static_cast<uint32_t>(std::strtoul(token.c_str(), nullptr, 16));
    } else {
        int_id = static_cast<uint32_t>(std::strtoul(token.c_str(), nullptr, 10));  // Decimal format
    }
    return int_id;
}

void CreateFilterMessageIdList(std::string raw_id_list, const std::string &delimiter, std::vector<uint32_t> &filter_list) {
    size_t pos = 0;
    std::string token;
    while (raw_id_list.length() != 0) {
        token = GetNextToken(&raw_id_list, delimiter, &pos);
        uint32_t int_id = TokenToUint(token);
        if (int_id == 0) {
            const uint32_t id_hash = vvl_vuid_hash(token);
            if (id_hash != 0) {
                int_id = id_hash;
            }
        }
        if ((int_id != 0) && (std::find(filter_list.begin(), filter_list.end(), int_id)) == filter_list.end()) {
            filter_list.push_back(int_id);
        }
    }
}

void SetCustomStypeInfo(std::string raw_id_list, const std::string &delimiter) {
    size_t pos = 0;
    std::string token;
    // List format is a list of integer pairs
    while (raw_id_list.length() != 0) {
        token = GetNextToken(&raw_id_list, delimiter, &pos);
        uint32_t stype_id = TokenToUint(token);
        token = GetNextToken(&raw_id_list, delimiter, &pos);
        uint32_t struct_size_in_bytes = TokenToUint(token);
        if ((stype_id != 0) && (struct_size_in_bytes != 0)) {
            bool found = false;
            // Prevent duplicate entries
            for (const auto &item : custom_stype_info) {
                if (item.first == stype_id) {
                    found = true;
                    break;
                }
            }
            if (!found) custom_stype_info.push_back(std::make_pair(stype_id, struct_size_in_bytes));
        }
    }
}

uint32_t SetMessageDuplicateLimit(const std::string &config_message_limit, const std::string &env_message_limit) {
    uint32_t limit = 0;
    auto get_num = [](const std::string &source_string) {
        uint32_t limit = 0;
        int radix = ((source_string.find("0x") == 0) ? 16 : 10);
        limit = static_cast<uint32_t>(std::strtoul(source_string.c_str(), nullptr, radix));
        return limit;
    };
    // ENV var takes precedence over settings file
    limit = get_num(env_message_limit);
    if (limit == 0) {
        limit = get_num(config_message_limit);
    }
    return limit;
}

const VkLayerSettingsEXT *FindSettingsInChain(const void *next) {
    const VkBaseOutStructure *current = reinterpret_cast<const VkBaseOutStructure *>(next);
    const VkLayerSettingsEXT *found = nullptr;
    while (current) {
        if (VK_STRUCTURE_TYPE_INSTANCE_LAYER_SETTINGS_EXT == current->sType) {
            found = reinterpret_cast<const VkLayerSettingsEXT *>(current);
            current = nullptr;
        } else {
            current = current->pNext;
        }
    }
    return found;
}

static bool SetBool(const std::string &config_string, const std::string &env_string, bool default_val) {
    bool result = default_val;

    std::string setting;
    if (!env_string.empty()) {
        setting = env_string;
    } else if (!config_string.empty()) {
        setting = config_string;
    }
    if (!setting.empty()) {
        vvl::ToLower(setting);
        if (setting == "true") {
            result = true;
        } else {
            result = std::atoi(setting.c_str()) != 0;
        }
    }
    return result;
}

static std::string GetSettingKey(const char *setting) {
    const std::string prefix("khronos_validation.");
    return prefix + setting;
}

static std::string GetConfigValue(const char *setting) {
    const std::string key(GetSettingKey(setting));
    return getLayerOption(key.c_str());
}

static std::string GetEnvVarValue(const char *setting) {
    std::string env_var = setting;
    vvl::ToUpper(env_var);
    return GetEnvironment((std::string("VK_LAYER_") + env_var).c_str());
}

static std::optional<std::string> GetSettingValue(const char *setting) {
    const std::string env_value = GetEnvVarValue(setting);
    if (!env_value.empty()) {
        return env_value;
    }

    const std::string cfg_value = GetConfigValue(setting);
    if (!cfg_value.empty()) {
        return cfg_value;
    }
    return {};
}

static void SetValidationSetting(CHECK_DISABLED &disable_data, const DisableFlags feature_disable, const char *setting) {
    const std::optional<std::string> setting_value = GetSettingValue(setting);

    if (setting_value) {
        disable_data[feature_disable] = setting_value != "true";
    }
}

static void SetValidationSetting(CHECK_ENABLED &enable_data, const EnableFlags feature_enable, const char *setting) {
    const std::optional<std::string> setting_value = GetSettingValue(setting);

    if (setting_value) {
        enable_data[feature_enable] = setting_value == "true";
    }
}

static void SetValidationGPUBasedSetting(CHECK_ENABLED &enable_data, const char *setting) {
    const std::optional<std::string> setting_value = GetSettingValue(setting);

    if (setting_value) {
        enable_data[gpu_validation] = setting_value->find("GPU_BASED_GPU_ASSISTED") != std::string::npos;
        enable_data[debug_printf] = setting_value->find("GPU_BASED_DEBUG_PRINTF") != std::string::npos;
    }
}

// Process enables and disables set though the vk_layer_settings.txt config file or through an environment variable
void ProcessConfigAndEnvSettings(ConfigAndEnvSettings *settings_data) {
    // If not cleared, garbage has been seen in some Android run effecting the error message
    custom_stype_info.clear();

    const auto layer_settings_ext = FindSettingsInChain(settings_data->pnext_chain);
    if (layer_settings_ext) {
        for (uint32_t i = 0; i < layer_settings_ext->settingCount; i++) {
            auto cur_setting = layer_settings_ext->pSettings[i];
            std::string name(cur_setting.name);
            if (name == SETTING_ENABLES) {
                std::string data(cur_setting.data.arrayString.pCharArray);
                SetLocalEnableSetting(data, ",", settings_data->enables);
            } else if (name == SETTING_DISABLES) {
                std::string data(cur_setting.data.arrayString.pCharArray);
                SetLocalDisableSetting(data, ",", settings_data->disables);
            } else if (name == SETTING_MESSAGE_ID_FILTER) {
                std::string data(cur_setting.data.arrayString.pCharArray);
                CreateFilterMessageIdList(data, ",", settings_data->message_filter_list);
            } else if (name == SETTING_DUPLICATE_MESSAGE_LIMIT) {
                *settings_data->duplicate_message_limit = cur_setting.data.value32;
            } else if (name == SETTING_CUSTOM_STYPE_LIST) {
                if (cur_setting.type == VK_LAYER_SETTING_VALUE_TYPE_STRING_ARRAY_EXT) {
                    std::string data(cur_setting.data.arrayString.pCharArray);
                    SetCustomStypeInfo(data, ",");
                } else if (cur_setting.type == VK_LAYER_SETTING_VALUE_TYPE_UINT32_ARRAY_EXT) {
                    for (uint32_t j = 0; j < cur_setting.data.arrayInt32.count / 2; j++) {
                        auto stype_id = cur_setting.data.arrayInt32.pInt32Array[j * 2];
                        auto struct_size = cur_setting.data.arrayInt32.pInt32Array[(j * 2) + 1];
                        bool found = false;
                        // Prevent duplicate entries
                        for (const auto &item : custom_stype_info) {
                            if (item.first == stype_id) {
                                found = true;
                                break;
                            }
                        }
                        if (!found) custom_stype_info.push_back(std::make_pair(stype_id, struct_size));
                    }
                }
            }
        }
    }
    const auto *validation_features_ext = LvlFindInChain<VkValidationFeaturesEXT>(settings_data->pnext_chain);
    if (validation_features_ext) {
        SetValidationFeatures(settings_data->disables, settings_data->enables, validation_features_ext);
    }
    const auto *validation_flags_ext = LvlFindInChain<VkValidationFlagsEXT>(settings_data->pnext_chain);
    if (validation_flags_ext) {
        SetValidationFlags(settings_data->disables, validation_flags_ext);
    }

#if defined(_WIN32)
    std::string env_delimiter = ";";
#else
    std::string env_delimiter = ":";
#endif

    // Read legacy enables and disables settings
    const std::string &config_value_disables = GetConfigValue(SETTING_DISABLES);
    const std::string &envvar_value_disables = GetEnvVarValue(SETTING_DISABLES);
    const bool use_disables_fine_gain_settings = config_value_disables.empty() && envvar_value_disables.empty();
    const std::string &config_value_enables = GetConfigValue(SETTING_ENABLES);
    const std::string &envvar_value_enables = GetEnvVarValue(SETTING_ENABLES);
    const bool use_enables_fine_gain_settings = config_value_enables.empty() && envvar_value_enables.empty();
    const bool use_fine_gain_settings = use_disables_fine_gain_settings && use_enables_fine_gain_settings;

    // Process layer enable settings
    SetLocalEnableSetting(config_value_enables, ",", settings_data->enables);
    SetLocalEnableSetting(envvar_value_enables, env_delimiter, settings_data->enables);

    // Only read the legacy enables flags when used, not their replacement.
    // Avoid Android C.I. performance regression from reading Android env variables
    if (use_fine_gain_settings) {
        SetValidationSetting(settings_data->enables, best_practices, SETTING_VALIDATE_BEST_PRACTICES);
        SetValidationSetting(settings_data->enables, vendor_specific_arm, SETTING_VALIDATE_BEST_PRACTICES_ARM);
        SetValidationSetting(settings_data->enables, vendor_specific_amd, SETTING_VALIDATE_BEST_PRACTICES_AMD);
        SetValidationSetting(settings_data->enables, vendor_specific_img, SETTING_VALIDATE_BEST_PRACTICES_IMG);
        SetValidationSetting(settings_data->enables, vendor_specific_nvidia, SETTING_VALIDATE_BEST_PRACTICES_NVIDIA);
        SetValidationSetting(settings_data->enables, sync_validation, SETTING_VALIDATE_SYNC);
        SetValidationSetting(settings_data->enables, sync_validation_queue_submit, SETTING_VALIDATE_SYNC_QUEUE_SUBMIT);
        SetValidationGPUBasedSetting(settings_data->enables, SETTING_VALIDATE_GPU_BASED);
        SetValidationSetting(settings_data->enables, gpu_validation_reserve_binding_slot, SETTING_RESERVE_BINDING_SLOT);
    }

    // Process layer disable settings
    SetLocalDisableSetting(config_value_disables, ",", settings_data->disables);
    SetLocalDisableSetting(envvar_value_disables, env_delimiter, settings_data->disables);

    // Only read the legacy disables flags when used, not their replacement.
    // Avoid Android C.I. performance regression from reading Android env variables
    if (use_fine_gain_settings) {
        SetValidationSetting(settings_data->disables, stateless_checks, SETTING_STATELESS_PARAM);
        SetValidationSetting(settings_data->disables, thread_safety, SETTING_THREAD_SAFETY);
        SetValidationSetting(settings_data->disables, core_checks, SETTING_VALIDATE_CORE);
        SetValidationSetting(settings_data->disables, command_buffer_state, SETTING_CHECK_COMMAND_BUFFER);
        SetValidationSetting(settings_data->disables, object_in_use, SETTING_CHECK_OBJECT_IN_USE);
        SetValidationSetting(settings_data->disables, query_validation, SETTING_CHECK_QUERY);
        SetValidationSetting(settings_data->disables, image_layout_validation, SETTING_CHECK_IMAGE_LAYOUT);
        SetValidationSetting(settings_data->disables, handle_wrapping, SETTING_UNIQUE_HANDLES);
        SetValidationSetting(settings_data->disables, object_tracking, SETTING_OBJECT_LIFETIME);
        SetValidationSetting(settings_data->disables, shader_validation, SETTING_CHECK_SHADERS);
        SetValidationSetting(settings_data->disables, shader_validation_caching, SETTING_CHECK_SHADERS_CACHING);
    }

    // Process message filter ID list
    CreateFilterMessageIdList(GetConfigValue(SETTING_MESSAGE_ID_FILTER), ",", settings_data->message_filter_list);
    CreateFilterMessageIdList(GetEnvVarValue(SETTING_MESSAGE_ID_FILTER), env_delimiter, settings_data->message_filter_list);

    // Process custom stype struct list
    SetCustomStypeInfo(GetConfigValue(SETTING_CUSTOM_STYPE_LIST), ",");
    SetCustomStypeInfo(GetEnvVarValue(SETTING_CUSTOM_STYPE_LIST), env_delimiter);

    // Process message limit
    const uint32_t config_limit_setting =
        SetMessageDuplicateLimit(GetConfigValue(SETTING_DUPLICATE_MESSAGE_LIMIT), GetEnvVarValue(SETTING_DUPLICATE_MESSAGE_LIMIT));
    if (config_limit_setting != 0) {
        *settings_data->duplicate_message_limit = config_limit_setting;
    }

    // Fine Grained Locking
    *settings_data->fine_grained_locking =
        SetBool(GetConfigValue(SETTING_FINE_GRAINED_LOCKING), GetConfigValue(SETTING_FINE_GRAINED_LOCKING), true);
}
