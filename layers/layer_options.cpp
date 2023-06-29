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
#include "generated/chassis.h"

// Include new / delete overrides if using mimalloc. This needs to be include exactly once in a file that is
// part of the VVL but not the layer utils library.
#if defined(USE_MIMALLOC) && defined(_WIN64)
#include "mimalloc-new-delete.h"
#endif

#define SETTING_CUSTOM_STYPE_INFO "custom_stype_list"

#define SETTING_FINE_GRAINED_LOCKING "fine_grained_locking"
#define SETTING_CORE "validate_core"
#define SETTING_CORE_IMAGE_LAYOUT "check_image_layout"
#define SETTING_CORE_COMMAND_BUFFER "check_command_buffer"
#define SETTING_CORE_OBJECT_IN_USE "check_object_in_use"
#define SETTING_CORE_QUERY "check_query"
#define SETTING_CORE_SHADERS "check_shaders"
#define SETTING_CORE_SHADERS_CACHING "check_shaders_caching"

#define SETTING_UNIQUE_HANDLES "unique_handles"
#define SETTING_OBJECT_LIFETIME "object_lifetime"
#define SETTING_STATELESS_PARAM "stateless_param"
#define SETTING_THREAD_SAFETY "thread_safety"

#define SETTING_SYNC "validate_sync"
#define SETTING_SYNC_QUEUE_SUBMIT "sync_queue_submit"

#define SETTING_GPU_BASED "validate_gpu_based"

#define SETTING_PRINTF_TO_STDOUT "printf_to_stdout"
#define SETTING_PRINTF_VERBOSE "printf_verbose"
#define SETTING_PRINTF_BUFFER_SIZE "printf_buffer_size"
#define SETTING_PRINTF_VMA_LINEAR_OUTPUT "vma_linear_output"

#define SETTING_GPUAV_DESCRIPTOR "gpuav_descriptor_checks"
#define SETTING_GPUAV_RESERVE_BINDING_SLOT "reserve_binding_slot"
#define SETTING_GPUAV_ROBUST_OOB "warn_on_robust_oob"
#define SETTING_GPUAV_DRAW_INDIRECT "validate_draw_indirect"
#define SETTING_GPUAV_DISPATCH_INDIRECT "validate_dispatch_indirect"
#define SETTING_GPUAV_MAX_BUFFER_DEVICE_ADDRESSES "max_buffer_device_addresses"

#define SETTING_BEST_PRACTICES "validate_best_practices"
#define SETTING_BEST_PRACTICES_ARM "validate_best_practices_arm"
#define SETTING_BEST_PRACTICES_AMD "validate_best_practices_amd"
#define SETTING_BEST_PRACTICES_IMG "validate_best_practices_img"
#define SETTING_BEST_PRACTICES_NV "validate_best_practices_nvidia"

#define SETTING_DEBUG_ACTION "debug_action"
#define SETTING_DEBUG_LOG_FILENAME "log_filename"
#define SETTING_DEBUG_REPORT "report_flags"
#define SETTING_DEBUG_ENABLE_MESSAGE_LIMIT "enable_message_limit"
#define SETTING_DEBUG_DUPLICATE_MESAGE_LIMIT "duplicate_message_limit"
#define SETTING_DEBUG_MESSAGE_ID_FILTER "message_id_filter"

#define SETTING_DISABLES "disables"
#define SETTING_ENABLES "enables"

static std::string format(const char *message, ...) {
    std::size_t const STRING_BUFFER(4096);

    assert(message != nullptr);
    assert(strlen(message) < STRING_BUFFER);

    char buffer[STRING_BUFFER];
    va_list list;

    va_start(list, message);
    vsnprintf(buffer, STRING_BUFFER, message, list);
    va_end(list);

    return buffer;
}

const char* GetToken(ValidateGPUBased value) {
    static const char *TABLE[]{
        "GPU_BASED_NONE", // VALIDATE_GPU_BASED_NONE
        "GPU_BASED_DEBUG_PRINTF", // VALIDATE_GPU_BASED_DEBUG_PRINTF
        "GPU_BASED_GPU_ASSISTED" // VALIDATE_GPU_BASED_GPU_ASSISTED
    };
    static_assert(std::size(TABLE) == VALIDATE_GPU_BASED_COUNT);


    return TABLE[value];
}

ValidateGPUBased GetValidateGPUBased(const std::string &token) {
    for (int i = 0, n = VALIDATE_GPU_BASED_COUNT; i < n; ++i) {
        const ValidateGPUBased value = static_cast<ValidateGPUBased>(i);
        if (GetToken(value) == token) {
            return value;
        }
    }

    return VALIDATE_GPU_BASED_NONE;
}

std::string GetDebugActionLog(int flags) {
    static const char *TABLE[]{
        "VK_DBG_LAYER_ACTION_LOG_MSG", // DEBUG_ACTION_LOG_MSG_BIT
        "VK_DBG_LAYER_ACTION_CALLBACK", // DEBUG_ACTION_CALLBACK_BIT
        "VK_DBG_LAYER_ACTION_DEBUG_OUTPUT", // DEBUG_ACTION_DEBUG_OUTPUT_BIT
        "VK_DBG_LAYER_ACTION_BREAK" // DEBUG_ACTION_BREAK_BIT
    };
    static_assert(std::size(TABLE) == DEBUG_ACTIONE_COUNT);

    std::string result;

    for (int i = 0, n = DEBUG_ACTIONE_COUNT; i < n; ++i) {
        if (flags & (1 << i)) {
            if (!result.empty()) {
                result += ',';
            }
            result += TABLE[i];
        }
    }

    return result;
}

int GetDebugAction(const std::vector<std::string>& flags) {
    int results = DEBUG_ACTION_NONE;

    for (std::size_t i = 0, n = flags.size(); i < n; ++i) {
        if (flags[i] == "VK_DBG_LAYER_ACTION_LOG_MSG") {
            results |= DEBUG_ACTION_LOG_MSG_BIT;
        } else if (flags[i] == "VK_DBG_LAYER_ACTION_CALLBACK") {
            results |= DEBUG_ACTION_CALLBACK_BIT;
        } else if (flags[i] == "VK_DBG_LAYER_ACTION_DEBUG_OUTPUT") {
            results |= DEBUG_ACTION_DEBUG_OUTPUT_BIT;
        } else if (flags[i] == "VK_DBG_LAYER_ACTION_BREAK") {
            results |= DEBUG_ACTION_BREAK_BIT;
        }
    }

    return results;
}

std::string GetDebugMessageLog(int flags) {
    static const char *TABLE[]{
        "info",  // DEBUG_MESSAGE_INFO_BIT
        "warn",  // DEBUG_MESSAGE_WARN_BIT
        "perf",  // DEBUG_MESSAGE_PERF_BIT
        "error", // DEBUG_MESSAGE_ERROR_BIT
        "debug"  // DEBUG_MESSAGE_DEBUG_BIT
    };
    static_assert(std::size(TABLE) == DEBUG_MESSAGE_COUNT);

    std::string result;

    for (int i = 0, n = DEBUG_MESSAGE_COUNT; i < n; ++i) {
        if (flags & (1 << i)) {
            if (!result.empty()) {
                result += ',';
            }
            result += TABLE[i];
        }
    }

    return result;
}

int GetDebugMessage(const std::vector<std::string>& flags) {
    int results = DEBUG_MESSAGE_NONE;

    for (std::size_t i = 0, n = flags.size(); i < n; ++i) {
        if (flags[i] == "info") {
            results |= DEBUG_MESSAGE_INFO_BIT;
        } else if (flags[i] == "warn") {
            results |= DEBUG_MESSAGE_WARN_BIT;
        } else if (flags[i] == "perf") {
            results |= DEBUG_MESSAGE_PERF_BIT;
        } else if (flags[i] == "error") {
            results |= DEBUG_MESSAGE_ERROR_BIT;
        } else if (flags[i] == "debug") {
            results |= DEBUG_MESSAGE_DEBUG_BIT;
        }
    }

    return results;
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

std::unordered_set<uint32_t> GetMessageIdFilter(const std::vector<std::string> &message_id_filter) {
    std::unordered_set<uint32_t> results;

    for (std::size_t i = 0, n = message_id_filter.size(); i < n; ++i) {
        std::string token = message_id_filter[i];
        uint32_t int_id = TokenToUint(token);
        if (int_id == 0) {
            const uint32_t id_hash = vvl_vuid_hash(token);
            if (id_hash != 0) {
                int_id = id_hash;
            }
        }
        if ((int_id != 0) && results.find(int_id) == results.end()) {
            results.insert(int_id);
        }
    }

    return results;
}

void LogLayerSettings(ValidationObject *context) {
    std::string log;

    log += "Khronos Validation Layer Settings:\n";
    log += format("- %s: %s\n", SETTING_FINE_GRAINED_LOCKING, context->layer_settings.validate.fine_grained_locking ? "true" : "false");
    log += format("- %s: %s\n", SETTING_CORE, context->layer_settings.validate.core ? "true" : "false");
    log += format("- %s: %s\n", SETTING_CORE_IMAGE_LAYOUT, context->layer_settings.validate.core_image_layout ? "true" : "false");
    log += format("- %s: %s\n", SETTING_CORE_COMMAND_BUFFER, context->layer_settings.validate.core_command_buffer ? "true" : "false");
    log += format("- %s: %s\n", SETTING_CORE_OBJECT_IN_USE, context->layer_settings.validate.core_object_in_use ? "true" : "false");
    log += format("- %s: %s\n", SETTING_CORE_QUERY, context->layer_settings.validate.core_query ? "true" : "false");
    log += format("- %s: %s\n", SETTING_CORE_SHADERS, context->layer_settings.validate.core_shaders ? "true" : "false");
    log += format("- %s: %s\n", SETTING_CORE_SHADERS_CACHING, context->layer_settings.validate.core_shaders_caching ? "true" : "false");
    log += "\n";

    log += format("- %s: %s\n", SETTING_UNIQUE_HANDLES, context->layer_settings.validate.unique_handles ? "true" : "false");
    log += format("- %s: %s\n", SETTING_OBJECT_LIFETIME, context->layer_settings.validate.object_lifetime ? "true" : "false");
    log += format("- %s: %s\n", SETTING_STATELESS_PARAM, context->layer_settings.validate.stateless_param ? "true" : "false");
    log += format("- %s: %s\n", SETTING_THREAD_SAFETY, context->layer_settings.validate.thread_safety ? "true" : "false");
    log += "\n";

    log += format("- %s: %s\n", SETTING_SYNC, context->layer_settings.validate.sync ? "true" : "false");
    log += format("- %s: %s\n", SETTING_SYNC_QUEUE_SUBMIT, context->layer_settings.validate.sync_queue_submit ? "true" : "false");
    log += "\n";

    log += format("- %s: %s\n", SETTING_GPU_BASED, GetToken(context->layer_settings.validate.gpu_based));
    log += "\n";

    log += format("- %s: %s\n", SETTING_PRINTF_TO_STDOUT, context->layer_settings.validate.printf_to_stdout ? "true" : "false");
    log += format("- %s: %s\n", SETTING_PRINTF_VERBOSE, context->layer_settings.validate.printf_verbose ? "true" : "false");
    log += format("- %s: %d\n", SETTING_PRINTF_BUFFER_SIZE, context->layer_settings.validate.printf_buffer_size ? "true" : "false");
    log += format("- %s: %s\n", SETTING_PRINTF_VMA_LINEAR_OUTPUT, context->layer_settings.validate.printf_vma_linear_output ? "true" : "false");
    log += "\n";

    log += format("- %s: %s\n", SETTING_GPUAV_DESCRIPTOR, context->layer_settings.validate.gpuav_descriptor ? "true" : "false");
    log += format("- %s: %s\n", SETTING_GPUAV_RESERVE_BINDING_SLOT, context->layer_settings.validate.gpuav_reserve_binding_slot ? "true" : "false");
    log += format("- %s: %s\n", SETTING_GPUAV_ROBUST_OOB, context->layer_settings.validate.gpuav_robust_oob ? "true" : "false");
    log += format("- %s: %s\n", SETTING_GPUAV_DRAW_INDIRECT, context->layer_settings.validate.gpuav_draw_indirect ? "true" : "false");
    log += format("- %s: %s\n", SETTING_GPUAV_DISPATCH_INDIRECT, context->layer_settings.validate.gpuav_dispatch_indirect ? "true" : "false");
    log += format("- %s: %d\n", SETTING_GPUAV_MAX_BUFFER_DEVICE_ADDRESSES, context->layer_settings.validate.gpuav_max_buffer_device_addresses);
    log += "\n";

    log += format("- %s: %s\n", SETTING_BEST_PRACTICES, context->layer_settings.validate.best_practices ? "true" : "false");
    log += format("- %s: %s\n", SETTING_BEST_PRACTICES_ARM, context->layer_settings.validate.best_practices_arm ? "true" : "false");
    log += format("- %s: %s\n", SETTING_BEST_PRACTICES_AMD, context->layer_settings.validate.best_practices_amd ? "true" : "false");
    log += format("- %s: %s\n", SETTING_BEST_PRACTICES_IMG, context->layer_settings.validate.best_practices_img ? "true" : "false");
    log += format("- %s: %s\n", SETTING_BEST_PRACTICES_NV, context->layer_settings.validate.best_practices_nv ? "true" : "false");
    log += "\n";

    log += format("- %s: %s\n", SETTING_DEBUG_ACTION, GetDebugActionLog(context->layer_settings.debug.actions).c_str());
    log += format("- %s: %s\n", SETTING_DEBUG_LOG_FILENAME, context->layer_settings.debug.log_filename.c_str());
    log += format("- %s: %s\n", SETTING_DEBUG_REPORT, GetDebugMessageLog(context->layer_settings.debug.messages).c_str());
    log += format("- %s: %s\n", SETTING_DEBUG_ENABLE_MESSAGE_LIMIT, context->layer_settings.debug.enable_message_limit ? "true" : "false");
    log += format("- %s: %d\n", SETTING_DEBUG_DUPLICATE_MESAGE_LIMIT, context->layer_settings.debug.duplicate_message_limit);
    log += "\n";

    // Output layer status information message
    context->LogInfo(context->instance, "UNASSIGNED-CreateInstance-status-message", "%s", log.c_str());

    // Create warning message if user is running debug layers.
#ifndef NDEBUG
    context->LogPerformanceWarning(
        context->instance, "UNASSIGNED-CreateInstance-debug-warning",
        "VALIDATION LAYERS WARNING: Using debug builds of the validation layers *will* adversely affect performance.");
#endif
    if (!context->layer_settings.validate.fine_grained_locking) {
        context->LogPerformanceWarning(
            context->instance, "UNASSIGNED-CreateInstance-locking-warning",
            "Fine-grained locking is disabled, this will adversely affect performance of multithreaded applications.");
    }
}

static bool HasString(const std::vector<std::string>& src, const char* value) {
    return std::find(src.begin(), src.end(), value) != src.end();
}

static bool Has(const VkValidationFeaturesEXT *validation_features_ext, VkValidationFeatureEnableEXT enable) {
    for (uint32_t i = 0; i < validation_features_ext->enabledValidationFeatureCount; ++i) {
        if (validation_features_ext->pEnabledValidationFeatures[i] == enable) {
            return true;
        }
    }
    return false;
}

static bool Has(const VkValidationFeaturesEXT *validation_features_ext, VkValidationFeatureDisableEXT disable) {
    for (uint32_t i = 0; i < validation_features_ext->disabledValidationFeatureCount; ++i) {
        if (validation_features_ext->pDisabledValidationFeatures[i] == VK_VALIDATION_FEATURE_DISABLE_ALL_EXT) {
            return true;
        }
        if (validation_features_ext->pDisabledValidationFeatures[i] == disable) {
            return true;
        }
    }
    return false;
}

void InitLayerSettings(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, LayerSettings *settings) {
    assert(settings != nullptr);

    const VkValidationFeaturesEXT *validation_features_ext = LvlFindInChain<VkValidationFeaturesEXT>(pCreateInfo->pNext);

    VlLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vlCreateLayerSettingSet(OBJECT_LAYER_NAME, vlFindLayerSettingsCreateInfo(pCreateInfo), pAllocator, nullptr, &layerSettingSet);

    std::vector<std::string> enables;
    if (vlHasLayerSetting(layerSettingSet, SETTING_ENABLES)) {
        vlGetLayerSettingValues(layerSettingSet, SETTING_ENABLES, enables);
    }

    std::vector<std::string> disables;
    if (vlHasLayerSetting(layerSettingSet, SETTING_DISABLES)) {
        vlGetLayerSettingValues(layerSettingSet, SETTING_DISABLES, disables);
    }

    // Read legacy enables and disables settings
    const bool use_legacy_settings = !(enables.empty() && disables.empty());

    // Legacy validation feature extension
    if (validation_features_ext != nullptr) {
        if (Has(validation_features_ext, VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT)) {
            settings->validate.core = false;
        }
        if (Has(validation_features_ext, VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT)) {
            settings->validate.core_shaders = false;
        }
        if (Has(validation_features_ext, VK_VALIDATION_FEATURE_DISABLE_SHADER_VALIDATION_CACHE_EXT)) {
            settings->validate.core_shaders_caching = false;
        }
        if (Has(validation_features_ext, VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT)) {
            settings->validate.unique_handles = false;
        }
        if (Has(validation_features_ext, VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT)) {
            settings->validate.object_lifetime = false;
        }
        if (Has(validation_features_ext, VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT)) {
            settings->validate.stateless_param = false;
        }
        if (Has(validation_features_ext, VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT)) {
            settings->validate.thread_safety = false;
        }

        if (Has(validation_features_ext, VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT)) {
            settings->validate.gpu_based = VALIDATE_GPU_BASED_GPU_ASSISTED;
            if (Has(validation_features_ext, VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT)) {
                settings->validate.gpuav_reserve_binding_slot = true;
            }
        } else if (Has(validation_features_ext, VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT)) {
            settings->validate.gpu_based = VALIDATE_GPU_BASED_DEBUG_PRINTF;
        } else {
            settings->validate.gpu_based = VALIDATE_GPU_BASED_NONE;
        }

        if (Has(validation_features_ext, VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT)) {
            settings->validate.best_practices = true;
        }
        if (Has(validation_features_ext, VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT)) {
            settings->validate.sync = true;
        }
    }

    if (use_legacy_settings) {
        // Legacy settings for backward compatibility

        if (HasString(disables, "VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT")) {
            settings->validate.core = false;
        }
        if (HasString(disables, "VALIDATION_CHECK_DISABLE_IMAGE_LAYOUT_VALIDATION")) {
            settings->validate.core_image_layout = false;
        }

        if (HasString(disables, "VALIDATION_CHECK_DISABLE_COMMAND_BUFFER_STATE")) {
            settings->validate.core_command_buffer = false;
        }
        if (HasString(disables, "VALIDATION_CHECK_DISABLE_OBJECT_IN_USE")) {
            settings->validate.core_object_in_use = false;
        }
        if (HasString(disables, "VALIDATION_CHECK_DISABLE_QUERY_VALIDATION")) {
            settings->validate.core_query = false;
        }
        if (HasString(disables, "VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT")) {
            settings->validate.core_shaders = false;
        }
        if (HasString(disables, "VK_VALIDATION_FEATURE_DISABLE_SHADER_VALIDATION_CACHING_EXT")) {
            settings->validate.core_shaders_caching = false;
        }

        if (HasString(disables, "VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT")) {
            settings->validate.unique_handles = false;
        }
        if (HasString(disables, "VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT")) {
            settings->validate.object_lifetime = false;
        }
        if (HasString(disables, "VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT")) {
            settings->validate.stateless_param = false;
        }
        if (HasString(disables, "VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT")) {
            settings->validate.thread_safety = false;
        }

        if (HasString(enables, "VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT")) {
            settings->validate.best_practices = true;
        }
        if (HasString(enables, "VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ARM")) {
            settings->validate.best_practices_arm = true;
        }
        if (HasString(enables, "VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_AMD")) {
            settings->validate.best_practices_amd = true;
        }
        if (HasString(enables, "VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_IMG")) {
            settings->validate.best_practices_img = true;
        }
        if (HasString(enables, "VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_NVIDIA")) {
            settings->validate.best_practices_nv = true;
        }

        if (HasString(enables, "VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION")) {
            settings->validate.sync = true;
        }
        if (HasString(enables, "VALIDATION_CHECK_ENABLE_SYNCHRONIZATION_VALIDATION_QUEUE_SUBMIT")) {
            settings->validate.sync_queue_submit = true;
        }

        if (HasString(enables, "VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT")) {
            settings->validate.gpu_based = VALIDATE_GPU_BASED_GPU_ASSISTED;

            if (HasString(enables, "VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT")) {
                settings->validate.gpuav_reserve_binding_slot = true;
            }
        } else if (HasString(enables, "VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT")) {
            settings->validate.gpu_based = VALIDATE_GPU_BASED_DEBUG_PRINTF;
        }

        if (vlHasLayerSetting(layerSettingSet, SETTING_FINE_GRAINED_LOCKING)) {
            vlGetLayerSettingValue(layerSettingSet, SETTING_FINE_GRAINED_LOCKING, settings->validate.fine_grained_locking);
        }
    } else {
        if (vlHasLayerSetting(layerSettingSet, SETTING_CORE)) {
            vlGetLayerSettingValue(layerSettingSet, SETTING_CORE, settings->validate.core);

            if (vlHasLayerSetting(layerSettingSet, SETTING_FINE_GRAINED_LOCKING)) {
                vlGetLayerSettingValue(layerSettingSet, SETTING_FINE_GRAINED_LOCKING, settings->validate.fine_grained_locking);
            }

            if (vlHasLayerSetting(layerSettingSet, SETTING_CORE_IMAGE_LAYOUT)) {
                vlGetLayerSettingValue(layerSettingSet, SETTING_CORE_IMAGE_LAYOUT, settings->validate.core_image_layout);
            }

            if (vlHasLayerSetting(layerSettingSet, SETTING_CORE_COMMAND_BUFFER)) {
                vlGetLayerSettingValue(layerSettingSet, SETTING_CORE_COMMAND_BUFFER, settings->validate.core_command_buffer);
            }

            if (vlHasLayerSetting(layerSettingSet, SETTING_CORE_OBJECT_IN_USE)) {
                vlGetLayerSettingValue(layerSettingSet, SETTING_CORE_OBJECT_IN_USE, settings->validate.core_object_in_use);
            }

            if (vlHasLayerSetting(layerSettingSet, SETTING_CORE_QUERY)) {
                vlGetLayerSettingValue(layerSettingSet, SETTING_CORE_QUERY, settings->validate.core_query);
            }

            if (vlHasLayerSetting(layerSettingSet, SETTING_CORE_SHADERS)) {
                vlGetLayerSettingValue(layerSettingSet, SETTING_CORE_SHADERS, settings->validate.core_shaders);

                if (vlHasLayerSetting(layerSettingSet, SETTING_CORE_SHADERS_CACHING)) {
                    vlGetLayerSettingValue(layerSettingSet, SETTING_CORE_SHADERS_CACHING, settings->validate.core_shaders_caching);
                }
            }
        }

        if (vlHasLayerSetting(layerSettingSet, SETTING_UNIQUE_HANDLES)) {
            vlGetLayerSettingValue(layerSettingSet, SETTING_UNIQUE_HANDLES, settings->validate.unique_handles);
        }

        if (vlHasLayerSetting(layerSettingSet, SETTING_OBJECT_LIFETIME)) {
            vlGetLayerSettingValue(layerSettingSet, SETTING_OBJECT_LIFETIME, settings->validate.object_lifetime);
        }

        if (vlHasLayerSetting(layerSettingSet, SETTING_STATELESS_PARAM)) {
            vlGetLayerSettingValue(layerSettingSet, SETTING_STATELESS_PARAM, settings->validate.stateless_param);
        }

        if (vlHasLayerSetting(layerSettingSet, SETTING_THREAD_SAFETY)) {
            vlGetLayerSettingValue(layerSettingSet, SETTING_THREAD_SAFETY, settings->validate.thread_safety);
        }

        if (vlHasLayerSetting(layerSettingSet, SETTING_SYNC)) {
            vlGetLayerSettingValue(layerSettingSet, SETTING_SYNC, settings->validate.sync);

            if (vlHasLayerSetting(layerSettingSet, SETTING_SYNC_QUEUE_SUBMIT)) {
                vlGetLayerSettingValue(layerSettingSet, SETTING_SYNC_QUEUE_SUBMIT, settings->validate.sync_queue_submit);
            }
        }

        if (vlHasLayerSetting(layerSettingSet, SETTING_BEST_PRACTICES)) {
            bool best_practices = false;
            vlGetLayerSettingValue(layerSettingSet, SETTING_BEST_PRACTICES, best_practices);
            if (best_practices) {
                settings->validate.best_practices = true;
            }

            if (settings->validate.best_practices) {
                if (vlHasLayerSetting(layerSettingSet, SETTING_BEST_PRACTICES_ARM)) {
                    bool best_practices_vendor = false;
                    vlGetLayerSettingValue(layerSettingSet, SETTING_BEST_PRACTICES_ARM, best_practices_vendor);
                    if (best_practices_vendor) {
                        settings->validate.best_practices_arm = true;
                    }
                }

                if (vlHasLayerSetting(layerSettingSet, SETTING_BEST_PRACTICES_AMD)) {
                    bool best_practices_vendor = false;
                    vlGetLayerSettingValue(layerSettingSet, SETTING_BEST_PRACTICES_AMD, best_practices_vendor);
                    if (best_practices_vendor) {
                        settings->validate.best_practices_amd = true;
                    }
                }

                if (vlHasLayerSetting(layerSettingSet, SETTING_BEST_PRACTICES_IMG)) {
                    bool best_practices_vendor = false;
                    vlGetLayerSettingValue(layerSettingSet, SETTING_BEST_PRACTICES_IMG, best_practices_vendor);
                    if (best_practices_vendor) {
                        settings->validate.best_practices_img = true;
                    }
                }

                if (vlHasLayerSetting(layerSettingSet, SETTING_BEST_PRACTICES_NV)) {
                    bool best_practices_vendor = false;
                    vlGetLayerSettingValue(layerSettingSet, SETTING_BEST_PRACTICES_NV, best_practices_vendor);
                    if (best_practices_vendor) {
                        settings->validate.best_practices_nv = true;
                    }
                }
            }
        }

        if (vlHasLayerSetting(layerSettingSet, SETTING_GPU_BASED)) {
            std::string validate_gpu_based;
            vlGetLayerSettingValue(layerSettingSet, SETTING_GPU_BASED, validate_gpu_based);
            settings->validate.gpu_based = GetValidateGPUBased(validate_gpu_based);

            if (vlHasLayerSetting(layerSettingSet, SETTING_GPUAV_RESERVE_BINDING_SLOT)) {
                vlGetLayerSettingValue(layerSettingSet, SETTING_GPUAV_RESERVE_BINDING_SLOT, settings->validate.gpuav_reserve_binding_slot);
            }
        }
    }

    switch (settings->validate.gpu_based) {
        case VALIDATE_GPU_BASED_NONE: {
            break;
        }
        case VALIDATE_GPU_BASED_DEBUG_PRINTF: {
            if (vlHasLayerSetting(layerSettingSet, SETTING_PRINTF_TO_STDOUT)) {
                vlGetLayerSettingValue(layerSettingSet, SETTING_PRINTF_TO_STDOUT, settings->validate.printf_to_stdout);
            }

            if (vlHasLayerSetting(layerSettingSet, SETTING_PRINTF_VERBOSE)) {
                vlGetLayerSettingValue(layerSettingSet, SETTING_PRINTF_VERBOSE, settings->validate.printf_verbose);
            }

            if (vlHasLayerSetting(layerSettingSet, SETTING_PRINTF_BUFFER_SIZE)) {
                vlGetLayerSettingValue(layerSettingSet, SETTING_PRINTF_BUFFER_SIZE, settings->validate.printf_buffer_size);
            }

            if (vlHasLayerSetting(layerSettingSet, SETTING_PRINTF_VMA_LINEAR_OUTPUT)) {
                vlGetLayerSettingValue(layerSettingSet, SETTING_PRINTF_VMA_LINEAR_OUTPUT, settings->validate.printf_vma_linear_output);
            }
            break;
        }
        case VALIDATE_GPU_BASED_GPU_ASSISTED: {
            if (vlHasLayerSetting(layerSettingSet, SETTING_GPUAV_DESCRIPTOR)) {
                vlGetLayerSettingValue(layerSettingSet, SETTING_GPUAV_DESCRIPTOR, settings->validate.gpuav_descriptor);
            }

            if (vlHasLayerSetting(layerSettingSet, SETTING_GPUAV_ROBUST_OOB)) {
                vlGetLayerSettingValue(layerSettingSet, SETTING_GPUAV_ROBUST_OOB, settings->validate.gpuav_robust_oob);
            }

            if (vlHasLayerSetting(layerSettingSet, SETTING_GPUAV_DRAW_INDIRECT)) {
                vlGetLayerSettingValue(layerSettingSet, SETTING_GPUAV_DRAW_INDIRECT, settings->validate.gpuav_draw_indirect);
            }

            if (vlHasLayerSetting(layerSettingSet, SETTING_GPUAV_DISPATCH_INDIRECT)) {
                vlGetLayerSettingValue(layerSettingSet, SETTING_GPUAV_DISPATCH_INDIRECT, settings->validate.gpuav_dispatch_indirect);
            }

            if (vlHasLayerSetting(layerSettingSet, SETTING_GPUAV_MAX_BUFFER_DEVICE_ADDRESSES)) {
                vlGetLayerSettingValue(layerSettingSet, SETTING_GPUAV_MAX_BUFFER_DEVICE_ADDRESSES, settings->validate.gpuav_max_buffer_device_addresses);
            }

            break;
        }
    }

    if (vlHasLayerSetting(layerSettingSet, SETTING_DEBUG_ACTION)) {
        std::vector<std::string> actions;
        vlGetLayerSettingValues(layerSettingSet, SETTING_DEBUG_ACTION, actions);
        settings->debug.actions = GetDebugAction(actions);
    }

    if (settings->debug.actions != DEBUG_ACTION_NONE) {
        if (vlHasLayerSetting(layerSettingSet, SETTING_DEBUG_LOG_FILENAME)) {
            vlGetLayerSettingValue(layerSettingSet, SETTING_DEBUG_LOG_FILENAME, settings->debug.log_filename);
        }

        if (vlHasLayerSetting(layerSettingSet, SETTING_DEBUG_REPORT)) {
            std ::vector<std::string> messages;
            vlGetLayerSettingValues(layerSettingSet, SETTING_DEBUG_REPORT, messages);
            settings->debug.messages = GetDebugMessage(messages);
        }

        if (vlHasLayerSetting(layerSettingSet, SETTING_DEBUG_ENABLE_MESSAGE_LIMIT)) {
            vlGetLayerSettingValue(layerSettingSet, SETTING_DEBUG_ENABLE_MESSAGE_LIMIT, settings->debug.enable_message_limit);
        }

        if (vlHasLayerSetting(layerSettingSet, SETTING_DEBUG_DUPLICATE_MESAGE_LIMIT)) {
            vlGetLayerSettingValue(layerSettingSet, SETTING_DEBUG_DUPLICATE_MESAGE_LIMIT, settings->debug.duplicate_message_limit);
        }

        if (vlHasLayerSetting(layerSettingSet, SETTING_DEBUG_MESSAGE_ID_FILTER)) {
            std::vector<std::string> message_id_filter;
            vlGetLayerSettingValues(layerSettingSet, SETTING_DEBUG_MESSAGE_ID_FILTER, message_id_filter);
            settings->debug.message_id_filter = GetMessageIdFilter(message_id_filter);
        }
    }

    if (vlHasLayerSetting(layerSettingSet, SETTING_CUSTOM_STYPE_INFO)) {
        vlGetLayerSettingValues(layerSettingSet, SETTING_CUSTOM_STYPE_INFO, custom_stype_info);
    }

    vlDestroyLayerSettingSet(layerSettingSet, pAllocator);
}

