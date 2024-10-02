
/* Copyright (c) 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2020-2024 Valve Corporation
 * Copyright (c) 2020-2024 LunarG, Inc.
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
#include <cstdint>
// Default values for those settings should match layers/VkLayer_khronos_validation.json.in

struct GpuAVSettings {
    bool warn_on_robust_oob = true;
    uint32_t max_bda_in_use = 10000;
    bool cache_instrumented_shaders = true;
    bool select_instrumented_shaders = false;

    // Turned off until we can fix things
    // see https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8579
    bool validate_indirect_draws_buffers = false;
    bool validate_indirect_dispatches_buffers = false;
    bool validate_indirect_trace_rays_buffers = false;
    bool validate_buffer_copies = true;

    bool vma_linear_output = true;

    bool debug_validate_instrumented_shaders = false;
    bool debug_dump_instrumented_shaders = false;
    uint32_t debug_max_instrumented_count = 0;  // zero is same as "unlimited"
    bool debug_print_instrumentation_info = false;

    // Note - even though DebugPrintf basically fits in here, from the user point of view they are different and that is reflected
    // in the settings (which are reflected in VkConfig). To make our lives easier, we just make these settings with the hierarchy
    // of the settings exposed
    struct ShaderInstrumentation {
        bool bindless_descriptor = true;
        bool buffer_device_address = true;
        bool ray_query = true;
    } shader_instrumentation;

    bool IsShaderInstrumentationEnabled() const {
        return shader_instrumentation.bindless_descriptor || shader_instrumentation.buffer_device_address ||
               shader_instrumentation.ray_query;
    }
    bool IsSpirvModified() const { return IsShaderInstrumentationEnabled() || debug_printf_enabled; }

    // Also disables shader caching and select shader instrumentation
    void DisableShaderInstrumentationAndOptions() {
        shader_instrumentation.bindless_descriptor = false;
        shader_instrumentation.buffer_device_address = false;
        shader_instrumentation.ray_query = false;
        // Because of those 2 settings, cannot really have an "enabled" parameter to pass to this method
        cache_instrumented_shaders = false;
        select_instrumented_shaders = false;
    }
    bool IsBufferValidationEnabled() const {
        return validate_indirect_draws_buffers || validate_indirect_dispatches_buffers || validate_indirect_trace_rays_buffers ||
               validate_buffer_copies;
    }
    void SetBufferValidationEnabled(bool enabled) {
        validate_indirect_draws_buffers = enabled;
        validate_indirect_dispatches_buffers = enabled;
        validate_indirect_trace_rays_buffers = enabled;
        validate_buffer_copies = enabled;
    }

    // For people who are using VkValidationFeatureEnableEXT to set only DebugPrintf (and want the rest of GPU-AV off)
    bool debug_printf_only = false;
    void SetOnlyDebugPrintf() {
        DisableShaderInstrumentationAndOptions();
        SetBufferValidationEnabled(false);

        // Turn on the minmal settings for DebugPrintf
        debug_printf_enabled = true;
        debug_printf_only = true;
    }

    bool debug_printf_enabled = false;
    bool debug_printf_to_stdout = false;
    bool debug_printf_verbose = false;
    uint32_t debug_printf_buffer_size = 1024;
};