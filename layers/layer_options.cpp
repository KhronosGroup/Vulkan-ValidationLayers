/* Copyright (c) 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2020-2024 Valve Corporation
 * Copyright (c) 2020-2024 LunarG, Inc.
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
#include "utils/hash_util.h"
#include <vulkan/layer/vk_layer_settings.hpp>

#include "gpu_validation/gpu_settings.h"
#include "error_message/logging.h"

// Include new / delete overrides if using mimalloc. This needs to be include exactly once in a file that is
// part of the VVL but not the layer utils library.
#if defined(USE_MIMALLOC) && defined(_WIN64)
#include "mimalloc-new-delete.h"
#endif

// To enable "my_setting",
// Set env var VK_LAYER_MY_SETTING to 1
//
// The ["VK_LAYER_" + toUpper(key)] logic is don in vk_layer_setting (VUL)
//
// To quickly be able to find the env var corresponding to a setting,
// the following `const char*` holding setting names match their corresponding environment variable
const char *VK_LAYER_ENABLES = "enables";
const char *VK_LAYER_VALIDATE_BEST_PRACTICES = "validate_best_practices";
const char *VK_LAYER_VALIDATE_BEST_PRACTICES_ARM = "validate_best_practices_arm";
const char *VK_LAYER_VALIDATE_BEST_PRACTICES_AMD = "validate_best_practices_amd";
const char *VK_LAYER_VALIDATE_BEST_PRACTICES_IMG = "validate_best_practices_img";
const char *VK_LAYER_VALIDATE_BEST_PRACTICES_NVIDIA = "validate_best_practices_nvidia";
const char *VK_LAYER_VALIDATE_SYNC = "validate_sync";
const char *VK_LAYER_VALIDATE_GPU_BASED = "validate_gpu_based";

const char *VK_LAYER_DISABLES = "disables";
const char *VK_LAYER_STATELESS_PARAM = "stateless_param";
const char *VK_LAYER_THREAD_SAFETY = "thread_safety";
const char *VK_LAYER_VALIDATE_CORE = "validate_core";
const char *VK_LAYER_CHECK_COMMAND_BUFFER = "check_command_buffer";
const char *VK_LAYER_CHECK_OBJECT_IN_USE = "check_object_in_use";
const char *VK_LAYER_CHECK_QUERY = "check_query";
const char *VK_LAYER_CHECK_IMAGE_LAYOUT = "check_image_layout";
const char *VK_LAYER_UNIQUE_HANDLES = "unique_handles";
const char *VK_LAYER_OBJECT_LIFETIME = "object_lifetime";
const char *VK_LAYER_CHECK_SHADERS = "check_shaders";
const char *VK_LAYER_CHECK_SHADERS_CACHING = "check_shaders_caching";
const char *VK_LAYER_VALIDATE_SYNC_QUEUE_SUBMIT = "sync_queue_submit";

const char *VK_LAYER_MESSAGE_ID_FILTER = "message_id_filter";
const char *VK_LAYER_CUSTOM_STYPE_LIST = "custom_stype_list";
const char *VK_LAYER_DUPLICATE_MESSAGE_LIMIT = "duplicate_message_limit";
const char *VK_LAYER_FINE_GRAINED_LOCKING = "fine_grained_locking";

const char *VK_LAYER_PRINTF_TO_STDOUT = "printf_to_stdout";
const char *VK_LAYER_PRINTF_VERBOSE = "printf_verbose";
const char *VK_LAYER_PRINTF_BUFFER_SIZE = "printf_buffer_size";

// GPU-AV
// ---
const char *VK_LAYER_GPUAV_SHADER_INSTRUMENTATION = "gpuav_shader_instrumentation";
const char *VK_LAYER_GPUAV_VALIDATE_DESCRIPTORS = "gpuav_descriptor_checks";
const char *VK_LAYER_GPUAV_WARN_ON_ROBUST_OOB = "gpuav_warn_on_robust_oob";
const char *VK_LAYER_GPUAV_BUFFER_ADDRESS_OOB = "gpuav_buffer_address_oob";
const char *VK_LAYER_GPUAV_MAX_BUFFER_DEVICE_ADDRESS_BUFFERS = "gpuav_max_buffer_device_addresses";
const char *VK_LAYER_GPUAV_VALIDATE_RAY_QUERY = "gpuav_validate_ray_query";
const char *VK_LAYER_GPUAV_CACHE_INSTRUMENTED_SHADERS = "gpuav_cache_instrumented_shaders";
const char *VK_LAYER_GPUAV_SELECT_INSTRUMENTED_SHADERS = "gpuav_select_instrumented_shaders";

const char *VK_LAYER_GPUAV_BUFFERS_VALIDATION = "gpuav_buffers_validation";
const char *VK_LAYER_GPUAV_VALIDATE_INDIRECT_DRAWS_BUFFERS = "gpuav_indirect_draws_buffers";
const char *VK_LAYER_GPUAV_VALIDATE_INDIRECT_DISPATCHES_BUFFERS = "gpuav_indirect_dispatches_buffers";
const char *VK_LAYER_GPUAV_VALIDATE_INDIRECT_TRACE_RAYS_BUFFERS = "gpuav_indirect_trace_rays_buffers";
const char *VK_LAYER_GPUAV_VALIDATE_BUFFER_COPIES = "gpuav_buffer_copies";

const char *VK_LAYER_GPUAV_RESERVE_BINDING_SLOT = "gpuav_reserve_binding_slot";
const char *VK_LAYER_GPUAV_VMA_LINEAR_OUTPUT = "gpuav_vma_linear_output";

const char *VK_LAYER_GPUAV_DEBUG_VALIDATE_INSTRUMENTED_SHADERS = "gpuav_debug_validate_instrumented_shaders";
const char *VK_LAYER_GPUAV_DEBUG_DUMP_INSTRUMENTED_SHADERS = "gpuav_debug_dump_instrumented_shaders";

// Message Formatting
const char *VK_LAYER_MESSAGE_FORMAT_DISPLAY_APPLICATION_NAME = "message_format_display_application_name";

// These were deprecated after the 1.3.280 SDK release
const char *DEPRECATED_VK_LAYER_GPUAV_VALIDATE_COPIES = "gpuav_validate_copies";
const char *DEPRECATED_VK_LAYER_GPUAV_VALIDATE_INDIRECT_BUFFER = "gpuav_validate_indirect_buffer";
const char *DEPRECATED_VK_LAYER_RESERVE_BINDING_SLOT = "reserve_binding_slot";
const char *DEPRECATED_GPUAV_VMA_LINEAR_OUTPUT = "vma_linear_output";
const char *DEPRECATED_GPUAV_WARN_ON_ROBUST_OOB = "warn_on_robust_oob";
const char *DEPRECATED_GPUAV_USE_INSTRUMENTED_SHADER_CACHE = "use_instrumented_shader_cache";
const char *DEPRECATED_GPUAV_SELECT_INSTRUMENTED_SHADERS = "select_instrumented_shaders";

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
        case VALIDATION_CHECK_DISABLE_SYNCHRONIZATION_VALIDATION_QUEUE_SUBMIT:
            disable_data[sync_validation_queue_submit] = true;
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
            enable_data[debug_printf_validation] = true;
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

void CreateFilterMessageIdList(std::string raw_id_list, const std::string &delimiter, vvl::unordered_set<uint32_t> &filter_list) {
    size_t pos = 0;
    std::string token;
    while (raw_id_list.length() != 0) {
        token = GetNextToken(&raw_id_list, delimiter, &pos);
        uint32_t int_id = TokenToUint(token);
        if (int_id == 0) {
            const uint32_t id_hash = hash_util::VuidHash(token);
            if (id_hash != 0) {
                int_id = id_hash;
            }
        }
        if ((int_id != 0) && filter_list.find(int_id) == filter_list.end()) {
            filter_list.insert(int_id);
        }
    }
}

#if !defined(BUILD_SELF_VVL)
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
#endif
// Process enables and disables set though the vk_layer_settings.txt config file or through an environment variable
void ProcessConfigAndEnvSettings(ConfigAndEnvSettings *settings_data) {
    // When compiling a build for self validation, ProcessConfigAndEnvSettings immediately returns,
    // so that the layer always defaults to the standard validation options we want,
    // and does not try to process option coming from the VVL we are debugging
#if defined(BUILD_SELF_VVL)
    (void)settings_data;
    return;
#else
    // If not cleared, garbage has been seen in some Android run effecting the error message
    custom_stype_info.clear();

    VkuLayerSettingSet layer_setting_set = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet(OBJECT_LAYER_NAME, vkuFindLayerSettingsCreateInfo(settings_data->create_info), nullptr, nullptr,
                             &layer_setting_set);

    vkuSetLayerSettingCompatibilityNamespace(layer_setting_set, GetDefaultPrefix());

    // Read legacy "enables" flags for backward compatibility
    std::vector<std::string> enables;
    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_ENABLES)) {
        vkuGetLayerSettingValues(layer_setting_set, VK_LAYER_ENABLES, enables);
    }
    const std::string &string_enables = Merge(enables);
    SetLocalEnableSetting(string_enables, ",", settings_data->enables);

    // Read legacy "disables" flags for backward compatibility
    std::vector<std::string> disables;
    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_DISABLES)) {
        vkuGetLayerSettingValues(layer_setting_set, VK_LAYER_DISABLES, disables);
    }
    const std::string &string_disables = Merge(disables);
    SetLocalDisableSetting(string_disables, ",", settings_data->disables);

    // Fine Grained Locking
    *settings_data->fine_grained_locking = true;
    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_FINE_GRAINED_LOCKING)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_FINE_GRAINED_LOCKING, *settings_data->fine_grained_locking);
    }

    // Message ID Filtering
    std::vector<std::string> message_id_filter;
    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_MESSAGE_ID_FILTER)) {
        vkuGetLayerSettingValues(layer_setting_set, VK_LAYER_MESSAGE_ID_FILTER, message_id_filter);
    }
    const std::string &string_message_id_filter = Merge(message_id_filter);
    CreateFilterMessageIdList(string_message_id_filter, ",", settings_data->message_filter_list);

    // Duplicate message limit
    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_DUPLICATE_MESSAGE_LIMIT)) {
        uint32_t config_limit_setting = 0;
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_DUPLICATE_MESSAGE_LIMIT, config_limit_setting);
        if (config_limit_setting != 0) {
            *settings_data->duplicate_message_limit = config_limit_setting;
        }
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_CUSTOM_STYPE_LIST)) {
        vkuGetLayerSettingValues(layer_setting_set, VK_LAYER_CUSTOM_STYPE_LIST, custom_stype_info);
    }

    DebugPrintfSettings &printf_settings = *settings_data->printf_settings;
    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_PRINTF_TO_STDOUT)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_PRINTF_TO_STDOUT, printf_settings.to_stdout);
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_PRINTF_VERBOSE)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_PRINTF_VERBOSE, printf_settings.verbose);
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_PRINTF_BUFFER_SIZE)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_PRINTF_BUFFER_SIZE, printf_settings.buffer_size);
    }

    GpuAVSettings &gpuav_settings = *settings_data->gpuav_settings;
    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_SHADER_INSTRUMENTATION)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_SHADER_INSTRUMENTATION,
                                gpuav_settings.shader_instrumentation_enabled);
    }
    if (!gpuav_settings.shader_instrumentation_enabled) {
        gpuav_settings.DisableShaderInstrumentationAndOptions();
    } else {
        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_VALIDATE_DESCRIPTORS)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_VALIDATE_DESCRIPTORS, gpuav_settings.validate_descriptors);
        }

        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_WARN_ON_ROBUST_OOB)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_WARN_ON_ROBUST_OOB, gpuav_settings.warn_on_robust_oob);
        } else if (vkuHasLayerSetting(layer_setting_set, DEPRECATED_GPUAV_WARN_ON_ROBUST_OOB)) {
            vkuGetLayerSettingValue(layer_setting_set, DEPRECATED_GPUAV_WARN_ON_ROBUST_OOB, gpuav_settings.warn_on_robust_oob);
            printf("Validation Setting Warning - %s was set, this is deprecated, please use %s\n",
                   DEPRECATED_GPUAV_WARN_ON_ROBUST_OOB, VK_LAYER_GPUAV_WARN_ON_ROBUST_OOB);
        }

        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_BUFFER_ADDRESS_OOB)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_BUFFER_ADDRESS_OOB, gpuav_settings.validate_bda);
        }
        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_MAX_BUFFER_DEVICE_ADDRESS_BUFFERS)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_MAX_BUFFER_DEVICE_ADDRESS_BUFFERS,
                                    gpuav_settings.max_bda_in_use);
        }

        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_VALIDATE_RAY_QUERY)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_VALIDATE_RAY_QUERY, gpuav_settings.validate_ray_query);
        }

        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_CACHE_INSTRUMENTED_SHADERS)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_CACHE_INSTRUMENTED_SHADERS,
                                    gpuav_settings.cache_instrumented_shaders);
        } else if (vkuHasLayerSetting(layer_setting_set, DEPRECATED_GPUAV_USE_INSTRUMENTED_SHADER_CACHE)) {
            vkuGetLayerSettingValue(layer_setting_set, DEPRECATED_GPUAV_USE_INSTRUMENTED_SHADER_CACHE,
                                    gpuav_settings.cache_instrumented_shaders);
            printf("Validation Setting Warning - %s was set, this is deprecated, please use %s\n",
                   DEPRECATED_GPUAV_USE_INSTRUMENTED_SHADER_CACHE, VK_LAYER_GPUAV_CACHE_INSTRUMENTED_SHADERS);
        }

        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_SELECT_INSTRUMENTED_SHADERS)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_SELECT_INSTRUMENTED_SHADERS,
                                    gpuav_settings.select_instrumented_shaders);
        } else if (vkuHasLayerSetting(layer_setting_set, DEPRECATED_GPUAV_SELECT_INSTRUMENTED_SHADERS)) {
            vkuGetLayerSettingValue(layer_setting_set, DEPRECATED_GPUAV_SELECT_INSTRUMENTED_SHADERS,
                                    gpuav_settings.select_instrumented_shaders);
            printf("Validation Setting Warning - %s was set, this is deprecated, please use %s\n",
                   DEPRECATED_GPUAV_SELECT_INSTRUMENTED_SHADERS, VK_LAYER_GPUAV_SELECT_INSTRUMENTED_SHADERS);
        }

        // No need to enable shader instrumentation options is no instrumentation is done
        if (!gpuav_settings.IsShaderInstrumentationEnabled()) {
            gpuav_settings.DisableShaderInstrumentationAndOptions();
        }
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_BUFFERS_VALIDATION)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_BUFFERS_VALIDATION, gpuav_settings.buffers_validation_enabled);
    }
    if (!gpuav_settings.buffers_validation_enabled) {
        gpuav_settings.SetBufferValidationEnabled(false);
    } else {
        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_VALIDATE_INDIRECT_DRAWS_BUFFERS)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_VALIDATE_INDIRECT_DRAWS_BUFFERS,
                                    gpuav_settings.validate_indirect_draws_buffers);
        }
        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_VALIDATE_INDIRECT_DISPATCHES_BUFFERS)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_VALIDATE_INDIRECT_DISPATCHES_BUFFERS,
                                    gpuav_settings.validate_indirect_dispatches_buffers);
        }
        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_VALIDATE_INDIRECT_TRACE_RAYS_BUFFERS)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_VALIDATE_INDIRECT_TRACE_RAYS_BUFFERS,
                                    gpuav_settings.validate_indirect_trace_rays_buffers);
        }
        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_VALIDATE_BUFFER_COPIES)) {
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_VALIDATE_BUFFER_COPIES,
                                    gpuav_settings.validate_buffer_copies);
        } else if (vkuHasLayerSetting(layer_setting_set, DEPRECATED_VK_LAYER_GPUAV_VALIDATE_COPIES)) {
            vkuGetLayerSettingValue(layer_setting_set, DEPRECATED_VK_LAYER_GPUAV_VALIDATE_COPIES,
                                    gpuav_settings.validate_buffer_copies);
            printf("Validation Setting Warning - %s was set, this is deprecated, please use %s\n",
                   DEPRECATED_VK_LAYER_GPUAV_VALIDATE_COPIES, VK_LAYER_GPUAV_VALIDATE_BUFFER_COPIES);
        }
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_RESERVE_BINDING_SLOT)) {
        SetValidationSetting(layer_setting_set, settings_data->enables, gpu_validation_reserve_binding_slot,
                             VK_LAYER_GPUAV_RESERVE_BINDING_SLOT);
    } else if (vkuHasLayerSetting(layer_setting_set, DEPRECATED_VK_LAYER_RESERVE_BINDING_SLOT)) {
        SetValidationSetting(layer_setting_set, settings_data->enables, gpu_validation_reserve_binding_slot,
                             DEPRECATED_VK_LAYER_RESERVE_BINDING_SLOT);
        printf("Validation Setting Warning - %s was set, this is deprecated, please use %s\n",
               DEPRECATED_VK_LAYER_RESERVE_BINDING_SLOT, VK_LAYER_GPUAV_RESERVE_BINDING_SLOT);
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_VMA_LINEAR_OUTPUT)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_VMA_LINEAR_OUTPUT, gpuav_settings.vma_linear_output);
    } else if (vkuHasLayerSetting(layer_setting_set, DEPRECATED_GPUAV_VMA_LINEAR_OUTPUT)) {
        vkuGetLayerSettingValue(layer_setting_set, DEPRECATED_GPUAV_VMA_LINEAR_OUTPUT, gpuav_settings.vma_linear_output);
        printf("Validation Setting Warning - %s was set, this is deprecated, please use %s\n", DEPRECATED_GPUAV_VMA_LINEAR_OUTPUT,
               VK_LAYER_GPUAV_VMA_LINEAR_OUTPUT);
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_DEBUG_VALIDATE_INSTRUMENTED_SHADERS)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_DEBUG_VALIDATE_INSTRUMENTED_SHADERS,
                                gpuav_settings.debug_validate_instrumented_shaders);
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_GPUAV_DEBUG_DUMP_INSTRUMENTED_SHADERS)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_GPUAV_DEBUG_DUMP_INSTRUMENTED_SHADERS,
                                gpuav_settings.debug_dump_instrumented_shaders);
    }

    if (gpuav_settings.debug_validate_instrumented_shaders || gpuav_settings.debug_dump_instrumented_shaders) {
        // When debugging instrumented shaders, if it is cached, it will never get to the InstrumentShader() call
        gpuav_settings.cache_instrumented_shaders = false;
    }

    if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_MESSAGE_FORMAT_DISPLAY_APPLICATION_NAME)) {
        vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_MESSAGE_FORMAT_DISPLAY_APPLICATION_NAME,
                                settings_data->message_format_settings->display_application_name);
    }
    // Grab application name here while we have access to it and know if to save it or not
    if (settings_data->message_format_settings->display_application_name) {
        settings_data->message_format_settings->application_name =
            settings_data->create_info->pApplicationInfo ? settings_data->create_info->pApplicationInfo->pApplicationName : "";
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
        SetValidationSetting(layer_setting_set, settings_data->enables, best_practices, VK_LAYER_VALIDATE_BEST_PRACTICES);
        SetValidationSetting(layer_setting_set, settings_data->enables, vendor_specific_arm, VK_LAYER_VALIDATE_BEST_PRACTICES_ARM);
        SetValidationSetting(layer_setting_set, settings_data->enables, vendor_specific_amd, VK_LAYER_VALIDATE_BEST_PRACTICES_AMD);
        SetValidationSetting(layer_setting_set, settings_data->enables, vendor_specific_img, VK_LAYER_VALIDATE_BEST_PRACTICES_IMG);
        SetValidationSetting(layer_setting_set, settings_data->enables, vendor_specific_nvidia,
                             VK_LAYER_VALIDATE_BEST_PRACTICES_NVIDIA);
        SetValidationSetting(layer_setting_set, settings_data->enables, sync_validation, VK_LAYER_VALIDATE_SYNC);

        if (vkuHasLayerSetting(layer_setting_set, VK_LAYER_VALIDATE_GPU_BASED)) {
            std::string setting_value;
            vkuGetLayerSettingValue(layer_setting_set, VK_LAYER_VALIDATE_GPU_BASED, setting_value);
            settings_data->enables[gpu_validation] = setting_value == "GPU_BASED_GPU_ASSISTED";
            settings_data->enables[debug_printf_validation] = setting_value == "GPU_BASED_DEBUG_PRINTF";
        }
    }

    // Only read the legacy disables flags when used, not their replacement.
    // Avoid Android C.I. performance regression from reading Android env variables
    if (use_fine_grained_settings) {
        SetValidationSetting(layer_setting_set, settings_data->disables, stateless_checks, VK_LAYER_STATELESS_PARAM);
        SetValidationSetting(layer_setting_set, settings_data->disables, thread_safety, VK_LAYER_THREAD_SAFETY);
        SetValidationSetting(layer_setting_set, settings_data->disables, core_checks, VK_LAYER_VALIDATE_CORE);
        SetValidationSetting(layer_setting_set, settings_data->disables, command_buffer_state, VK_LAYER_CHECK_COMMAND_BUFFER);
        SetValidationSetting(layer_setting_set, settings_data->disables, object_in_use, VK_LAYER_CHECK_OBJECT_IN_USE);
        SetValidationSetting(layer_setting_set, settings_data->disables, query_validation, VK_LAYER_CHECK_QUERY);
        SetValidationSetting(layer_setting_set, settings_data->disables, image_layout_validation, VK_LAYER_CHECK_IMAGE_LAYOUT);
        SetValidationSetting(layer_setting_set, settings_data->disables, handle_wrapping, VK_LAYER_UNIQUE_HANDLES);
        SetValidationSetting(layer_setting_set, settings_data->disables, object_tracking, VK_LAYER_OBJECT_LIFETIME);
        SetValidationSetting(layer_setting_set, settings_data->disables, shader_validation, VK_LAYER_CHECK_SHADERS);
        SetValidationSetting(layer_setting_set, settings_data->disables, shader_validation_caching, VK_LAYER_CHECK_SHADERS_CACHING);
        SetValidationSetting(layer_setting_set, settings_data->disables, sync_validation_queue_submit,
                             VK_LAYER_VALIDATE_SYNC_QUEUE_SUBMIT);
    }

    vkuDestroyLayerSettingSet(layer_setting_set, nullptr);
#endif
}
