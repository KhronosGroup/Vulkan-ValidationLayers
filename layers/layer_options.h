/* Copyright (c) 2022-2023 The Khronos Group Inc.
 * Copyright (c) 2022-2023 Valve Corporation
 * Copyright (c) 2022-2023 LunarG, Inc.
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

#include "chassis.h"

extern std::vector<std::pair<uint32_t, uint32_t>> custom_stype_info;

// Process validation features, flags and settings specified through extensions, a layer settings file, or environment variables

typedef struct {
    const char *layer_description;
    const void *pnext_chain;
    CHECK_ENABLED &enables;
    CHECK_DISABLED &disables;
    std::vector<uint32_t> &message_filter_list;
    int32_t *duplicate_message_limit;
    bool *fine_grained_locking;
} ConfigAndEnvSettings;

static const vvl::unordered_map<std::string, VkValidationFeatureDisableEXT> VkValFeatureDisableLookup = {
    {"VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT", VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT},
    {"VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT", VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT},
    {"VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT", VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT},
    {"VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT", VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT},
    {"VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT", VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT},
    {"VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT", VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT},
    {"VK_VALIDATION_FEATURE_DISABLE_SHADER_VALIDATION_CACHE_EXT", VK_VALIDATION_FEATURE_DISABLE_SHADER_VALIDATION_CACHE_EXT},
    {"VK_VALIDATION_FEATURE_DISABLE_ALL_EXT", VK_VALIDATION_FEATURE_DISABLE_ALL_EXT},
};

static const vvl::unordered_map<std::string, VkValidationFeatureEnableEXT> VkValFeatureEnableLookup = {
    {"VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT", VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT},
    {"VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT",
     VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT},
    {"VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT", VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT},
    {"VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT", VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT},
    {"VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT", VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT},
};

static const vvl::unordered_map<std::string, VkValidationFeatureEnable> VkValFeatureEnableLookup2 = {
    {"VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION", VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION},
};

static const vvl::unordered_map<std::string, ValidationCheckDisables> ValidationDisableLookup = {
    {"VALIDATION_CHECK_DISABLE_COMMAND_BUFFER_STATE", VALIDATION_CHECK_DISABLE_COMMAND_BUFFER_STATE},
    {"VALIDATION_CHECK_DISABLE_OBJECT_IN_USE", VALIDATION_CHECK_DISABLE_OBJECT_IN_USE},
    {"VALIDATION_CHECK_DISABLE_QUERY_VALIDATION", VALIDATION_CHECK_DISABLE_QUERY_VALIDATION},
    {"VALIDATION_CHECK_DISABLE_IMAGE_LAYOUT_VALIDATION", VALIDATION_CHECK_DISABLE_IMAGE_LAYOUT_VALIDATION},
};

static const vvl::unordered_map<std::string, ValidationCheckEnables> ValidationEnableLookup = {
    {"VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ARM", VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ARM},
    {"VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_AMD", VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_AMD},
    {"VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_IMG", VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_IMG},
    {"VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_NVIDIA", VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_NVIDIA},
    {"VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ALL", VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ALL},
    {"VALIDATION_CHECK_ENABLE_SYNCHRONIZATION_VALIDATION_QUEUE_SUBMIT",
     VALIDATION_CHECK_ENABLE_SYNCHRONIZATION_VALIDATION_QUEUE_SUBMIT},
};

// This should mirror the 'DisableFlags' enumerated type
static const std::vector<std::string> DisableFlagNameHelper = {
    "VALIDATION_CHECK_DISABLE_COMMAND_BUFFER_STATE",               // command_buffer_state,
    "VALIDATION_CHECK_DISABLE_OBJECT_IN_USE",                      // object_in_use,
    "VALIDATION_CHECK_DISABLE_QUERY_VALIDATION",                   // query_validation,
    "VALIDATION_CHECK_DISABLE_IMAGE_LAYOUT_VALIDATION",            // image_layout_validation,
    "VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT",          // object_tracking,
    "VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT",               // core_checks,
    "VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT",             // thread_safety,
    "VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT",            // stateless_checks,
    "VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT",            // handle_wrapping,
    "VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT",                   // shader_validation,
    "VK_VALIDATION_FEATURE_DISABLE_SHADER_VALIDATION_CACHING_EXT"  // shader_validation_caching
};

// This should mirror the 'EnableFlags' enumerated type
static const std::vector<std::string> EnableFlagNameHelper = {
    "VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT",                       // gpu_validation,
    "VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT",  // gpu_validation_reserve_binding_slot,
    "VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT",                     // best_practices,
    "VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ARM",                         // vendor_specific_arm,
    "VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_AMD",                         // vendor_specific_amd,
    "VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_IMG",                         // vendor_specific_img,
    "VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_NVIDIA",                      // vendor_specific_nvidia,
    "VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT",                       // debug_printf,
    "VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION",             // sync_validation,
    "VALIDATION_CHECK_ENABLE_SYNCHRONIZATION_VALIDATION_QUEUE_SUBMIT",     // queuesubmit time sync_validation,
};

void ProcessConfigAndEnvSettings(ConfigAndEnvSettings *settings_data);
