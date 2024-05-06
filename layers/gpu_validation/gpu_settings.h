
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
// Default values for those settings should match layers/VkLayer_khronos_validation.json.in

struct GpuAVSettings {
    bool shader_instrumentation_enabled = true;
    bool validate_descriptors = true;
    bool warn_on_robust_oob = true;
    bool validate_bda = true;
    uint32_t max_bda_in_use = 10000;
    bool validate_ray_query = true;
    bool cache_instrumented_shaders = true;
    bool select_instrumented_shaders = false;

    bool buffers_validation_enabled = true;
    bool validate_indirect_draws_buffers = true;
    bool validate_indirect_dispatches_buffers = true;
    bool validate_indirect_trace_rays_buffers = true;
    bool validate_buffer_copies = true;

    bool vma_linear_output = true;

    bool debug_validate_instrumented_shaders = false;
    bool debug_dump_instrumented_shaders = false;

    bool IsShaderInstrumentationEnabled() const { return validate_descriptors || validate_bda || validate_ray_query; }
    // Also disables shader caching and select shader instrumentation
    void DisableShaderInstrumentationAndOptions() {
        validate_descriptors = false;
        warn_on_robust_oob = false;
        validate_bda = false;
        validate_ray_query = false;
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
};

struct DebugPrintfSettings {
    bool to_stdout = false;
    bool verbose = false;
    uint32_t buffer_size = 1024;
};