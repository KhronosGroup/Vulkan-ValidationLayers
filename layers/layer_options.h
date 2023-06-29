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

#pragma once

#include <vulkan/layer/vk_layer_settings.hpp>
#include <unordered_set>
#include <array>

#include "containers/custom_containers.h"
#include "error_message/logging.h"

class ValidationObject;

extern std::vector<std::pair<uint32_t, uint32_t>> custom_stype_info;

// Process validation features, flags and settings specified through extensions, a layer settings file, or environment variables

#define OBJECT_LAYER_NAME "VK_LAYER_KHRONOS_validation"
#define OBJECT_LAYER_DESCRIPTION "khronos_validation"

enum ValidateGPUBased {
    VALIDATE_GPU_BASED_NONE = 0,
    VALIDATE_GPU_BASED_DEBUG_PRINTF,
    VALIDATE_GPU_BASED_GPU_ASSISTED,

    VALIDATE_GPU_BASED_FIRST = VALIDATE_GPU_BASED_NONE,
    VALIDATE_GPU_BASED_LAST = VALIDATE_GPU_BASED_GPU_ASSISTED
};

enum { 
    VALIDATE_GPU_BASED_COUNT = VALIDATE_GPU_BASED_LAST - VALIDATE_GPU_BASED_FIRST + 1
};

ValidateGPUBased GetValidateGPUBased(const std::string &value);

enum DebugAction {
    DEBUG_ACTION_NONE = 0,
    DEBUG_ACTION_LOG_MSG_BIT = 1 << 0,
    DEBUG_ACTION_CALLBACK_BIT = 1 << 1,
    DEBUG_ACTION_DEBUG_OUTPUT_BIT = 1 << 2,
    DEBUG_ACTION_BREAK_BIT = 1 << 3
};

enum { DEBUG_ACTIONE_COUNT = 4 };

int GetDebugAction(const std::vector<std::string>& flags);

enum DebugMessage {
    DEBUG_MESSAGE_NONE = 0,
    DEBUG_MESSAGE_INFO_BIT = 1 << 0,
    DEBUG_MESSAGE_WARN_BIT = 1 << 1,
    DEBUG_MESSAGE_PERF_BIT = 1 << 2,
    DEBUG_MESSAGE_ERROR_BIT = 1 << 3,
    DEBUG_MESSAGE_DEBUG_BIT = 1 << 4
};

enum { DEBUG_MESSAGE_COUNT = 5 };

int GetDebugMessage(const std::vector<std::string> &flags);

struct LayerSettings {
    struct ValidationControl {
        bool fine_grained_locking{true};

        bool core{true};
        bool core_image_layout{true};
        bool core_command_buffer{true};
        bool core_object_in_use{true};
        bool core_query{true};
        bool core_shaders{true};
        bool core_shaders_caching{true};

        bool unique_handles{true};
        bool object_lifetime{true};
        bool stateless_param{true};
        bool thread_safety{true};

        bool sync{false};
        bool sync_queue_submit{false};

       ValidateGPUBased gpu_based{VALIDATE_GPU_BASED_NONE};

        bool printf_to_stdout{true};
        bool printf_verbose{true};
        int printf_buffer_size{1024};
        bool printf_vma_linear_output{true};

        bool gpuav_descriptor{true};
        bool gpuav_reserve_binding_slot{true};
        bool gpuav_robust_oob{true};
        bool gpuav_draw_indirect{true};
        bool gpuav_dispatch_indirect{true};
        int gpuav_max_buffer_device_addresses{10000};

        bool best_practices{false};
        bool best_practices_arm{false};
        bool best_practices_amd{false};
        bool best_practices_img{false};
        bool best_practices_nv{false};
    } ;

    ValidationControl validate;

    struct Debug {
        int actions{VK_DBG_LAYER_ACTION_LOG_MSG};
        std::string log_filename;

        int messages{DEBUG_MESSAGE_ERROR_BIT};

        bool enable_message_limit{true};
        std::uint32_t duplicate_message_limit{10};

        std::unordered_set<uint32_t> message_id_filter;
    };

    Debug debug;
};

void LogLayerSettings(ValidationObject *context);

void InitLayerSettings(
    const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, LayerSettings *layer_settings);
