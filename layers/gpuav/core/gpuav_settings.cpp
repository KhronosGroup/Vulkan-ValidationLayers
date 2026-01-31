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

#include "gpuav/core/gpuav_settings.h"

#include "profiling/profiling.h"

// Fix GCC 13 issues with regex
#if defined(__GNUC__) && (__GNUC__ > 12)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#include <regex>
#if defined(__GNUC__) && (__GNUC__ > 12)
#pragma GCC diagnostic pop
#endif

bool GpuAVSettings::IsShaderInstrumentationEnabled() const {
    return shader_instrumentation.descriptor_checks || shader_instrumentation.buffer_device_address ||
           shader_instrumentation.ray_query || shader_instrumentation.ray_hit_object || shader_instrumentation.mesh_shading ||
           shader_instrumentation.post_process_descriptor_indexing || shader_instrumentation.vertex_attribute_fetch_oob ||
           shader_instrumentation.sanitizer;
}
bool GpuAVSettings::IsSpirvModified() const { return IsShaderInstrumentationEnabled() || debug_printf_enabled; }
uint32_t GpuAVSettings::GetInvalidIndexCommand() const { return indices_count - 1; }

// Also disables shader caching and select shader instrumentation
void GpuAVSettings::DisableShaderInstrumentationAndOptions() {
    shader_instrumentation.descriptor_checks = false;
    shader_instrumentation.buffer_device_address = false;
    shader_instrumentation.ray_query = false;
    shader_instrumentation.ray_hit_object = false;
    shader_instrumentation.mesh_shading = false;
    shader_instrumentation.post_process_descriptor_indexing = false;
    shader_instrumentation.vertex_attribute_fetch_oob = false;
    shader_instrumentation.sanitizer = false;
    // Because of this setting, cannot really have an "enabled" parameter to pass to this method
    select_instrumented_shaders = false;
}
bool GpuAVSettings::IsBufferValidationEnabled() const {
    return validate_indirect_draws_buffers || validate_indirect_dispatches_buffers || validate_indirect_trace_rays_buffers ||
           validate_buffer_copies || validate_copy_memory_indirect || validate_index_buffers;
}
void GpuAVSettings::SetBufferValidationEnabled(bool enabled) {
    validate_indirect_draws_buffers = enabled;
    validate_indirect_dispatches_buffers = enabled;
    validate_indirect_trace_rays_buffers = enabled;
    validate_buffer_copies = enabled;
    validate_copy_memory_indirect = enabled;
    validate_index_buffers = enabled;
    validate_acceleration_structures_builds = enabled;
}

void GpuAVSettings::SetShaderSelectionRegexes(std::vector<std::string> &&shader_selection_regexes) {
    this->shader_selection_regexes = std::move(shader_selection_regexes);
}

bool GpuAVSettings::MatchesAnyShaderSelectionRegex(const std::string &debug_name) {
    if (debug_name.empty()) {
        return false;
    }
    for (const std::string &shader_selection_regex_str : shader_selection_regexes) {
        std::regex regex(shader_selection_regex_str, std::regex_constants::ECMAScript);
        if (std::regex_match(debug_name, regex)) {
            return true;
        }
    }
    return false;
}

void GpuAVSettings::SetOnlyDebugPrintf() {
    DisableShaderInstrumentationAndOptions();
    SetBufferValidationEnabled(false);

    // Turn on the minmal settings for DebugPrintf
    debug_printf_enabled = true;
    debug_printf_only = true;
}

void GpuAVSettings::TracyLogSettings() const {
#if defined(TRACY_ENABLE)
    VVL_TracyMessageStream("GpuAVSettings:");
    VVL_TracyMessageStream("  safe_mode: " << safe_mode);
    VVL_TracyMessageStream("  force_on_robustness: " << force_on_robustness);
    VVL_TracyMessageStream("  select_instrumented_shaders: " << select_instrumented_shaders);
    if (!shader_selection_regexes.empty()) {
        VVL_TracyMessageStream("  shader_selection_regexes:");
        for (size_t i = 0; i < shader_selection_regexes.size(); ++i) {
            VVL_TracyMessageStream("    [" << i << "]: " << shader_selection_regexes[i]);
        }
    } else {
        VVL_TracyMessageStream("  shader_selection_regexes: (empty)");
    }
    VVL_TracyMessageStream("  validate_indirect_draws_buffers: " << validate_indirect_draws_buffers);
    VVL_TracyMessageStream("  validate_indirect_dispatches_buffers: " << validate_indirect_dispatches_buffers);
    VVL_TracyMessageStream("  validate_indirect_trace_rays_buffers: " << validate_indirect_trace_rays_buffers);
    VVL_TracyMessageStream("  validate_buffer_copies: " << validate_buffer_copies);
    VVL_TracyMessageStream("  validate_copy_memory_indirect: " << validate_copy_memory_indirect);
    VVL_TracyMessageStream("  validate_index_buffers: " << validate_index_buffers);
    VVL_TracyMessageStream("  validate_build_acceleration_structures: " << validate_acceleration_structures_builds);
    VVL_TracyMessageStream("  validate_image_layout: " << validate_image_layout);
    VVL_TracyMessageStream("  vma_linear_output: " << vma_linear_output);
    VVL_TracyMessageStream("  debug_validate_instrumented_shaders: " << debug_validate_instrumented_shaders);
    VVL_TracyMessageStream("  debug_dump_instrumented_shaders: " << debug_dump_instrumented_shaders);
    VVL_TracyMessageStream("  debug_max_instrumentations_count: " << debug_max_instrumentations_count);
    VVL_TracyMessageStream("  debug_print_instrumentation_info: " << debug_print_instrumentation_info);
    VVL_TracyMessageStream("  ShaderInstrumentation:");
    VVL_TracyMessageStream("    descriptor_checks: " << shader_instrumentation.descriptor_checks);
    VVL_TracyMessageStream("    buffer_device_address: " << shader_instrumentation.buffer_device_address);
    VVL_TracyMessageStream("    ray_query: " << shader_instrumentation.ray_query);
    VVL_TracyMessageStream("    ray_hit_object: " << shader_instrumentation.ray_hit_object);
    VVL_TracyMessageStream("    mesh_shading: " << shader_instrumentation.mesh_shading);
    VVL_TracyMessageStream("    post_process_descriptor_indexing: " << shader_instrumentation.post_process_descriptor_indexing);
    VVL_TracyMessageStream("    vertex_attribute_fetch_oob: " << shader_instrumentation.vertex_attribute_fetch_oob);
    VVL_TracyMessageStream("    sanitizer: " << shader_instrumentation.sanitizer);
    VVL_TracyMessageStream("  debug_printf_only: " << debug_printf_only);
    VVL_TracyMessageStream("  debug_printf_enabled: " << debug_printf_enabled);
    VVL_TracyMessageStream("  debug_printf_to_stdout: " << debug_printf_to_stdout);
    VVL_TracyMessageStream("  debug_printf_verbose: " << debug_printf_verbose);
    VVL_TracyMessageStream("  debug_printf_buffer_size: " << debug_printf_buffer_size);
#endif
}
