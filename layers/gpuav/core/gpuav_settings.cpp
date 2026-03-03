/* Copyright (c) 2020-2026 The Khronos Group Inc.
 * Copyright (c) 2020-2026 Valve Corporation
 * Copyright (c) 2020-2026 LunarG, Inc.
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
           shader_instrumentation.sanitizer || shader_instrumentation.shared_memory_data_race;
}
bool GpuAVSettings::IsSpirvModified() const { return IsShaderInstrumentationEnabled() || debug_printf_enabled; }

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
    shader_instrumentation.shared_memory_data_race = false;
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

    // Turn on the minimal settings for DebugPrintf
    debug_printf_enabled = true;
    debug_printf_only = true;
}

void GpuAVSettings::TracyLogSettings() const {
#if defined(TRACY_ENABLE)
#define VVL_TRACY_PRINT_GPUAV_SETTING(setting) VVL_TracyMessageStream("  " #setting ": " << setting);
    VVL_TracyMessageStream("GpuAVSettings:");
    VVL_TRACY_PRINT_GPUAV_SETTING(safe_mode);
    VVL_TRACY_PRINT_GPUAV_SETTING(force_on_robustness);
    VVL_TRACY_PRINT_GPUAV_SETTING(select_instrumented_shaders);
    if (!shader_selection_regexes.empty()) {
        VVL_TracyMessageStream("  shader_selection_regexes:");
        for (size_t i = 0; i < shader_selection_regexes.size(); ++i) {
            VVL_TracyMessageStream("    [" << i << "]: " << shader_selection_regexes[i]);
        }
    } else {
        VVL_TracyMessageStream("  shader_selection_regexes: (empty)");
    }
    VVL_TRACY_PRINT_GPUAV_SETTING(validate_indirect_draws_buffers);
    VVL_TRACY_PRINT_GPUAV_SETTING(validate_indirect_dispatches_buffers);
    VVL_TRACY_PRINT_GPUAV_SETTING(validate_indirect_trace_rays_buffers);
    VVL_TRACY_PRINT_GPUAV_SETTING(validate_buffer_copies);
    VVL_TRACY_PRINT_GPUAV_SETTING(validate_copy_memory_indirect);
    VVL_TRACY_PRINT_GPUAV_SETTING(validate_index_buffers);
    VVL_TRACY_PRINT_GPUAV_SETTING(validate_acceleration_structures_builds);
    VVL_TRACY_PRINT_GPUAV_SETTING(ray_tracing_buffers_consistency);
    VVL_TRACY_PRINT_GPUAV_SETTING(validate_image_layout);
    VVL_TRACY_PRINT_GPUAV_SETTING(vma_linear_output);
    VVL_TRACY_PRINT_GPUAV_SETTING(debug_validate_instrumented_shaders);
    VVL_TRACY_PRINT_GPUAV_SETTING(debug_dump_instrumented_shaders);
    VVL_TRACY_PRINT_GPUAV_SETTING(debug_max_instrumentations_count);
    VVL_TRACY_PRINT_GPUAV_SETTING(debug_print_instrumentation_info);
#define VVL_TRACY_PRINT_INSTRUMENTATION_SETTING(setting) \
    VVL_TracyMessageStream("    " #setting ": " << shader_instrumentation.setting);
    VVL_TracyMessageStream("  ShaderInstrumentation:");
    VVL_TRACY_PRINT_INSTRUMENTATION_SETTING(descriptor_checks);
    VVL_TRACY_PRINT_INSTRUMENTATION_SETTING(buffer_device_address);
    VVL_TRACY_PRINT_INSTRUMENTATION_SETTING(ray_query);
    VVL_TRACY_PRINT_INSTRUMENTATION_SETTING(ray_hit_object);
    VVL_TRACY_PRINT_INSTRUMENTATION_SETTING(mesh_shading);
    VVL_TRACY_PRINT_INSTRUMENTATION_SETTING(post_process_descriptor_indexing);
    VVL_TRACY_PRINT_INSTRUMENTATION_SETTING(vertex_attribute_fetch_oob);
    VVL_TRACY_PRINT_INSTRUMENTATION_SETTING(sanitizer);
    VVL_TRACY_PRINT_INSTRUMENTATION_SETTING(shared_memory_data_race);
    VVL_TRACY_PRINT_GPUAV_SETTING(debug_printf_only);
    VVL_TRACY_PRINT_GPUAV_SETTING(debug_printf_enabled);
    VVL_TRACY_PRINT_GPUAV_SETTING(debug_printf_to_stdout);
    VVL_TRACY_PRINT_GPUAV_SETTING(debug_printf_verbose);
    VVL_TRACY_PRINT_GPUAV_SETTING(debug_printf_buffer_size);
#undef VVL_TRACY_PRINT_GPUAV_SETTING
#undef VVL_TRACY_PRINT_INSTRUMENTATION_SETTING
#endif
}
