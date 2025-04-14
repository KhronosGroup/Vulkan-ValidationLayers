/* Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
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
#include <string>
#include <vector>

// Default values for those settings should match layers/VkLayer_khronos_validation.json.in

struct GpuAVSettings {
    bool safe_mode = false;

    bool force_on_robustness = false;
    uint32_t max_bda_in_use = 10'000;
    bool select_instrumented_shaders = false;
    std::vector<std::string> shader_selection_regexes{};

    bool validate_indirect_draws_buffers = true;
    bool validate_indirect_dispatches_buffers = true;
    bool validate_indirect_trace_rays_buffers = true;
    bool validate_buffer_copies = true;
    bool validate_index_buffers = true;

    // Currently turned of due to some false positives still observed
    bool validate_image_layout = false;

    bool vma_linear_output = true;

    bool debug_validate_instrumented_shaders = false;
    bool debug_dump_instrumented_shaders = false;
    uint32_t debug_max_instrumentations_count = 0;  // zero is same as "unlimited"
    bool debug_print_instrumentation_info = false;

    // Note - even though DebugPrintf basically fits in here, from the user point of view they are different and that is reflected
    // in the settings (which are reflected in VkConfig). To make our lives easier, we just make these settings with the hierarchy
    // of the settings exposed
    struct ShaderInstrumentation {
        bool descriptor_checks = true;
        bool buffer_device_address = true;
        bool ray_query = true;
        bool post_process_descriptor_indexing = true;
        bool vertex_attribute_fetch_oob = true;
    } shader_instrumentation;

    bool IsShaderInstrumentationEnabled() const;
    bool IsSpirvModified() const;

    // Also disables shader caching and select shader instrumentation
    void DisableShaderInstrumentationAndOptions();
    bool IsBufferValidationEnabled() const;
    void SetBufferValidationEnabled(bool enabled);

    void SetShaderSelectionRegexes(std::vector<std::string> &&shader_selection_regexes);
    bool MatchesAnyShaderSelectionRegex(const std::string &debug_name);

    // For people who are using VkValidationFeatureEnableEXT to set only DebugPrintf (and want the rest of GPU-AV off)
    bool debug_printf_only = false;
    void SetOnlyDebugPrintf();

    bool debug_printf_enabled = false;
    bool debug_printf_to_stdout = false;
    bool debug_printf_verbose = false;
    uint32_t debug_printf_buffer_size = 1024;
};
