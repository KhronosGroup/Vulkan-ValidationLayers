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

#ifndef LAYER_OPTIONS_H
#define LAYER_OPTIONS_H

#include "chassis.h"

class Settings {
  public:
    enum DebugAction {
        DEBUG_ACTION_LOG_MSG_BIT = (1 << 0),
        DEBUG_ACTION_CALLBACK_BIT = (1 << 1),
        DEBUG_ACTION_DEBUG_ACTION_BIT = (1 << 2),
        DEBUG_ACTION_BREAK_BIT = (1 << 3)
    };

    enum Report {
        REPORT_INFO_BIT = (1 << 0),
        REPORT_WARN_BIT = (1 << 1),
        REPORT_PERF_BIT = (1 << 2),
        REPORT_ERROR_BIT = (1 << 3),
        REPORT_DEBUG_BIT = (1 << 4)
    };

    enum Locking { GLOBAL = 0, FINE_GRAIN };

    enum ShaderBased { SHADER_BASED_NONE = 0, SHADER_BASED_DEBUG_PRINTF, SHADER_BASED_GPU_ASSISTED };

    enum VMAMode { VMA_LINEAR = 0, VMA_BEST };

    enum BestPracticesVendor {
        BEST_ARM_BIT = (1 << 0),  // validate_best_practices_arm
        BEST_AMD_BIT = (1 << 1),  // validate_best_practices_amd
        BEST_IMG_BIT = (1 << 2),  // validate_best_practices_img
        BEST_NV_BIT = (1 << 3)    // validate_best_practices_nv
    };

  private:
    Settings();

  public:
    static const Settings& Get();

    struct {
        DebugAction debug_action;                       // debug_action
        std::string log_filename;                       // log_filename
        Report report_flags;                            // report_flags
        bool enable_message_limit;                      // enable_message_limit
        int duplicate_message_limit;                    // duplicate_message_limit
        std::vector<uint32_t> message_id_filter;        // message_id_filter
    } log;

    struct {
        bool enabled;                     // validate_core
        Locking locking;                  // validate_core_locking
        bool check_image_layout;     // core_check_image_layout
        bool check_command_buffer;   // core_check_command_buffer
        bool check_object_in_use;    // core_check_object_in_use
        bool check_query;            // core_check_query
        bool check_shaders;          // core_check_shaders
        bool check_shaders_caching;  // core_check_shaders_caching
    } core;

    bool validate_unique_handles;   // validate_unique_handles
    bool validate_object_lifetime;  // validate_object_lifetime
    bool validate_stateless_param;  // validate_stateless_param
    bool validate_thread_safety;    // validate_thread_safety

    struct {
        bool enabled;               // validate_sync
        bool sync_queue_submit;  // validate_sync_queue_submit
    } sync;

    struct {
        bool enabled;                     // validate_best_practices
        int vendors;
    } best;

    struct {
        ShaderBased mode;

        struct {
            bool to_stdout;        // debug_printf_to_stdout
            bool verbose;          // debug_printf_verbose
            uint32_t buffer_size;  // debug_printf_buffer_size
        } debug_printf;

        struct {
            bool reserve_binding_slot;       // gpuav_reserve_binding_slot
            VMAMode vma_mode;                // gpuav_vma_mode
            bool check_descriptor_indexing;  // gpuav_check_descriptor_indexing
            bool check_buffer_oob;           // gpuav_check_buffer_oob
            bool check_draw_indirect;        // gpuav_check_draw_indirect
            bool check_dispatch_indirect;    // gpuav_check_dispatch_indirect
            bool warn_on_robust_oob;         // gpuav_warn_on_robust_oob
        } gpu_assisted;
    } shader_based;
};

extern std::vector<std::pair<uint32_t, uint32_t>> custom_stype_info;

/*
typedef struct {
    const char *layer_description;
    const void *pnext_chain;
    CHECK_ENABLED &enables;
    CHECK_DISABLED &disables;
    std::vector<uint32_t> &message_filter_list;
    int32_t *duplicate_message_limit;
    bool *fine_grained_locking;
} ConfigAndEnvSettings;
*/

/*
// Process validation features, flags and settings specified through extensions, a layer settings file, or environment variables

extern const layer_data::unordered_map<std::string, VkValidationFeatureDisableEXT> VkValFeatureDisableLookup;
extern const layer_data::unordered_map<std::string, VkValidationFeatureEnableEXT> VkValFeatureEnableLookup;
extern const layer_data::unordered_map<std::string, VkValidationFeatureEnable> VkValFeatureEnableLookup2;
extern const layer_data::unordered_map<std::string, ValidationCheckDisables> ValidationDisableLookup;
extern const layer_data::unordered_map<std::string, ValidationCheckEnables> ValidationEnableLookup;
extern const std::vector<std::string> DisableFlagNameHelper;
extern const std::vector<std::string> EnableFlagNameHelper;

void ProcessConfigAndEnvSettings(ConfigAndEnvSettings *settings_data);
*/

#endif//LAYER_OPTIONS_H
