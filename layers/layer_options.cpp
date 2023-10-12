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
#include <vulkan/layer/vk_layer_settings.hpp>

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

const char *SETTING_GPUAV_VALIDATE_DESCRIPTORS = "gpuav_descriptor_checks";
const char *SETTING_GPUAV_VALIDATE_DRAW_INDIRECT = "validate_draw_indirect";
const char *SETTING_GPUAV_VALIDATE_DISPATCH_INDIRECT = "validate_dispatch_indirect";
const char *SETTING_GPUAV_VMA_LINEAR_OUTPUT = "vma_linear_output";
const char *SETTING_GPUAV_WARN_ON_ROBUST_OOB = "warn_on_robust_oob";
const char *SETTING_GPUAV_USE_INSTRUMENTED_SHADER_CACHE = "use_instrumented_shader_cache";
const char *SETTING_GPUAV_SELECT_INSTRUMENTED_SHADERS = "select_instrumented_shaders";
const char *SETTING_GPUAV_MAX_BUFFER_DEVICE_ADDRESS_BUFFERS = "gpuav_max_buffer_device_addresses";

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

void CreateFilterMessageIdList(std::string raw_id_list, const std::string &delimiter, std::unordered_set<uint32_t> &filter_list) {
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
        if ((int_id != 0) && filter_list.find(int_id) == filter_list.end()) {
            filter_list.insert(int_id);
        }
    }
}

static void SetValidationSetting(VkuLayerSettingSet layer_setting_set, CHECK_DISABLED &disable_data,
                                 const DisableFlags feature_disable, const char *setting) {
    if (vkuHasLayerSetting(layer_setting_set, setting)) {
        bool enabled = true;
        vkuGetLayerSettingValue(layer_setting_set, setting, enabled);
        disable_data[feature_disable] = !enabled;
    }
}

static void SetValidationSetting(VkuLayerSettingSet layer_setting_set, CHECK_ENABLED &enable_data, const EnableFlags feature_enable,
                                 const char *setting) {
    if (vkuHasLayerSetting(layer_setting_set, setting)) {
        bool enabled = true;
        vkuGetLayerSettingValue(layer_setting_set, setting, enabled);
        enable_data[feature_enable] = enabled;
    }
}

static std::string Merge(const std::vector<std::string> &strings) {
    std::string result;

    for (std::size_t i = 0, n = strings.size(); i < n; ++i) {
        if (!result.empty()) {
            result += ",";
        }
        result += strings[i];
    }

    return result;
}

static const char *GetDefaultPrefix() {
#ifdef __ANDROID__
    return "vvl";
#else
    return "LAYER";
#endif
}

// Process enables and disables set though the vk_layer_settings.txt config file or through an environment variable
void ProcessConfigAndEnvSettings(ConfigAndEnvSettings *settings_data) {
    // If not cleared, garbage has been seen in some Android run effecting the error message
    custom_stype_info.clear();

    VkuLayerSettingSet layer_setting_set = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet(OBJECT_LAYER_NAME, vkuFindLayerSettingsCreateInfo(settings_data->create_info), nullptr, nullptr,
                             &layer_setting_set);

    vkuSetLayerSettingCompatibilityNamespace(layer_setting_set, GetDefaultPrefix());

    // Read legacy "enables" flags for backward compatibility
    std::vector<std::string> enables;
    if (vkuHasLayerSetting(layer_setting_set, SETTING_ENABLES)) {
        vkuGetLayerSettingValues(layer_setting_set, SETTING_ENABLES, enables);
    }
    const std::string &string_enables = Merge(enables);
    SetLocalEnableSetting(string_enables, ",", settings_data->enables);

    // Read legacy "disables" flags for backward compatibility
    std::vector<std::string> disables;
    if (vkuHasLayerSetting(layer_setting_set, SETTING_DISABLES)) {
        vkuGetLayerSettingValues(layer_setting_set, SETTING_DISABLES, disables);
    }
    const std::string &string_disables = Merge(disables);
    SetLocalDisableSetting(string_disables, ",", settings_data->disables);

    // Fine Grained Locking
    *settings_data->fine_grained_locking = true;
    if (vkuHasLayerSetting(layer_setting_set, SETTING_FINE_GRAINED_LOCKING)) {
        vkuGetLayerSettingValue(layer_setting_set, SETTING_FINE_GRAINED_LOCKING, *settings_data->fine_grained_locking);
    }

    // Message ID Filtering
    std::vector<std::string> message_id_filter;
    if (vkuHasLayerSetting(layer_setting_set, SETTING_MESSAGE_ID_FILTER)) {
        vkuGetLayerSettingValues(layer_setting_set, SETTING_MESSAGE_ID_FILTER, message_id_filter);
    }
    const std::string &string_message_id_filter = Merge(message_id_filter);
    CreateFilterMessageIdList(string_message_id_filter, ",", settings_data->message_filter_list);

    // Duplicate message limit
    if (vkuHasLayerSetting(layer_setting_set, SETTING_DUPLICATE_MESSAGE_LIMIT)) {
        uint32_t config_limit_setting = 0;
        vkuGetLayerSettingValue(layer_setting_set, SETTING_DUPLICATE_MESSAGE_LIMIT, config_limit_setting);
        if (config_limit_setting != 0) {
            *settings_data->duplicate_message_limit = config_limit_setting;
        }
    }

    if (vkuHasLayerSetting(layer_setting_set, SETTING_CUSTOM_STYPE_LIST)) {
        vkuGetLayerSettingValues(layer_setting_set, SETTING_CUSTOM_STYPE_LIST, custom_stype_info);
    }

    if (vkuHasLayerSetting(layer_setting_set, SETTING_GPUAV_VALIDATE_DESCRIPTORS)) {
        vkuGetLayerSettingValue(layer_setting_set, SETTING_GPUAV_VALIDATE_DESCRIPTORS,
                                settings_data->gpuav_settings->validate_descriptors);
    }

    if (vkuHasLayerSetting(layer_setting_set, SETTING_GPUAV_VALIDATE_DRAW_INDIRECT)) {
        vkuGetLayerSettingValue(layer_setting_set, SETTING_GPUAV_VALIDATE_DRAW_INDIRECT,
                                settings_data->gpuav_settings->validate_draw_indirect);
    }

    if (vkuHasLayerSetting(layer_setting_set, SETTING_GPUAV_VALIDATE_DISPATCH_INDIRECT)) {
        vkuGetLayerSettingValue(layer_setting_set, SETTING_GPUAV_VALIDATE_DISPATCH_INDIRECT,
                                settings_data->gpuav_settings->validate_dispatch_indirect);
    }

    if (vkuHasLayerSetting(layer_setting_set, SETTING_GPUAV_VMA_LINEAR_OUTPUT)) {
        vkuGetLayerSettingValue(layer_setting_set, SETTING_GPUAV_VMA_LINEAR_OUTPUT,
                                settings_data->gpuav_settings->vma_linear_output);
    }

    if (vkuHasLayerSetting(layer_setting_set, SETTING_GPUAV_WARN_ON_ROBUST_OOB)) {
        vkuGetLayerSettingValue(layer_setting_set, SETTING_GPUAV_WARN_ON_ROBUST_OOB,
                                settings_data->gpuav_settings->warn_on_robust_oob);
    }

    if (vkuHasLayerSetting(layer_setting_set, SETTING_GPUAV_USE_INSTRUMENTED_SHADER_CACHE)) {
        vkuGetLayerSettingValue(layer_setting_set, SETTING_GPUAV_USE_INSTRUMENTED_SHADER_CACHE,
                                settings_data->gpuav_settings->cache_instrumented_shaders);
    }

    if (vkuHasLayerSetting(layer_setting_set, SETTING_GPUAV_SELECT_INSTRUMENTED_SHADERS)) {
        vkuGetLayerSettingValue(layer_setting_set, SETTING_GPUAV_SELECT_INSTRUMENTED_SHADERS,
                                settings_data->gpuav_settings->select_instrumented_shaders);
    }

    if (vkuHasLayerSetting(layer_setting_set, SETTING_GPUAV_MAX_BUFFER_DEVICE_ADDRESS_BUFFERS)) {
        vkuGetLayerSettingValue(layer_setting_set, SETTING_GPUAV_MAX_BUFFER_DEVICE_ADDRESS_BUFFERS,
                                settings_data->gpuav_settings->gpuav_max_buffer_device_addresses);
    }

    const auto *validation_features_ext = vku::FindStructInPNextChain<VkValidationFeaturesEXT>(settings_data->create_info);
    if (validation_features_ext) {
        SetValidationFeatures(settings_data->disables, settings_data->enables, validation_features_ext);
    }
    const auto *validation_flags_ext = vku::FindStructInPNextChain<VkValidationFlagsEXT>(settings_data->create_info);
    if (validation_flags_ext) {
        SetValidationFlags(settings_data->disables, validation_flags_ext);
    }

    const bool use_fine_grained_settings = disables.empty() && enables.empty();

    // Only read the legacy enables flags when used, not their replacement.
    // Avoid Android C.I. performance regression from reading Android env variables
    if (use_fine_grained_settings) {
        SetValidationSetting(layer_setting_set, settings_data->enables, best_practices, SETTING_VALIDATE_BEST_PRACTICES);
        SetValidationSetting(layer_setting_set, settings_data->enables, vendor_specific_arm, SETTING_VALIDATE_BEST_PRACTICES_ARM);
        SetValidationSetting(layer_setting_set, settings_data->enables, vendor_specific_amd, SETTING_VALIDATE_BEST_PRACTICES_AMD);
        SetValidationSetting(layer_setting_set, settings_data->enables, vendor_specific_img, SETTING_VALIDATE_BEST_PRACTICES_IMG);
        SetValidationSetting(layer_setting_set, settings_data->enables, vendor_specific_nvidia,
                             SETTING_VALIDATE_BEST_PRACTICES_NVIDIA);
        SetValidationSetting(layer_setting_set, settings_data->enables, sync_validation, SETTING_VALIDATE_SYNC);
        SetValidationSetting(layer_setting_set, settings_data->enables, sync_validation_queue_submit,
                             SETTING_VALIDATE_SYNC_QUEUE_SUBMIT);

        if (vkuHasLayerSetting(layer_setting_set, SETTING_VALIDATE_GPU_BASED)) {
            std::string setting_value;
            vkuGetLayerSettingValue(layer_setting_set, SETTING_VALIDATE_GPU_BASED, setting_value);
            settings_data->enables[gpu_validation] = setting_value == "GPU_BASED_GPU_ASSISTED";
            settings_data->enables[debug_printf] = setting_value == "GPU_BASED_DEBUG_PRINTF";
        }

        SetValidationSetting(layer_setting_set, settings_data->enables, gpu_validation_reserve_binding_slot,
                             SETTING_RESERVE_BINDING_SLOT);
    }

    // Only read the legacy disables flags when used, not their replacement.
    // Avoid Android C.I. performance regression from reading Android env variables
    if (use_fine_grained_settings) {
        SetValidationSetting(layer_setting_set, settings_data->disables, stateless_checks, SETTING_STATELESS_PARAM);
        SetValidationSetting(layer_setting_set, settings_data->disables, thread_safety, SETTING_THREAD_SAFETY);
        SetValidationSetting(layer_setting_set, settings_data->disables, core_checks, SETTING_VALIDATE_CORE);
        SetValidationSetting(layer_setting_set, settings_data->disables, command_buffer_state, SETTING_CHECK_COMMAND_BUFFER);
        SetValidationSetting(layer_setting_set, settings_data->disables, object_in_use, SETTING_CHECK_OBJECT_IN_USE);
        SetValidationSetting(layer_setting_set, settings_data->disables, query_validation, SETTING_CHECK_QUERY);
        SetValidationSetting(layer_setting_set, settings_data->disables, image_layout_validation, SETTING_CHECK_IMAGE_LAYOUT);
        SetValidationSetting(layer_setting_set, settings_data->disables, handle_wrapping, SETTING_UNIQUE_HANDLES);
        SetValidationSetting(layer_setting_set, settings_data->disables, object_tracking, SETTING_OBJECT_LIFETIME);
        SetValidationSetting(layer_setting_set, settings_data->disables, shader_validation, SETTING_CHECK_SHADERS);
        SetValidationSetting(layer_setting_set, settings_data->disables, shader_validation_caching, SETTING_CHECK_SHADERS_CACHING);
    }

    vkuDestroyLayerSettingSet(layer_setting_set, nullptr);
}
