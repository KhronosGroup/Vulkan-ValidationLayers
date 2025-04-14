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
           shader_instrumentation.ray_query || shader_instrumentation.post_process_descriptor_indexing ||
           shader_instrumentation.vertex_attribute_fetch_oob;
}
bool GpuAVSettings::IsSpirvModified() const { return IsShaderInstrumentationEnabled() || debug_printf_enabled; }

// Also disables shader caching and select shader instrumentation
void GpuAVSettings::DisableShaderInstrumentationAndOptions() {
    shader_instrumentation.descriptor_checks = false;
    shader_instrumentation.buffer_device_address = false;
    shader_instrumentation.ray_query = false;
    shader_instrumentation.post_process_descriptor_indexing = false;
    shader_instrumentation.vertex_attribute_fetch_oob = false;
    // Because of this setting, cannot really have an "enabled" parameter to pass to this method
    select_instrumented_shaders = false;
}
bool GpuAVSettings::IsBufferValidationEnabled() const {
    return validate_indirect_draws_buffers || validate_indirect_dispatches_buffers || validate_indirect_trace_rays_buffers ||
           validate_buffer_copies || validate_index_buffers;
}
void GpuAVSettings::SetBufferValidationEnabled(bool enabled) {
    validate_indirect_draws_buffers = enabled;
    validate_indirect_dispatches_buffers = enabled;
    validate_indirect_trace_rays_buffers = enabled;
    validate_buffer_copies = enabled;
    validate_index_buffers = enabled;
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
