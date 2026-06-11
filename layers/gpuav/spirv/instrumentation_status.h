/* Copyright (c) 2026 LunarG, Inc.
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

// See https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/12031
//
// The idea of this is we need to be able to "bake" information at instrumentation time and then at draw/dispatch time
// we need to match it as well.
// The normal way to do this was to make sure the "logic/algo" how data was set was identical, which is a bit brittle.
// Instead we have the shader instrumentation return some "status" information that later at draw time,
// the CPU code can just look and match it.

#include <vulkan/vulkan_core.h>
#include <vector>

namespace gpuav {
namespace spirv {

struct HeapMappingStatus {
    uint32_t mapping_index = 0;  // index into pMappings[]
    uint32_t binding = 0;        // the mapping_data doesn't encode the exact binding
    uint32_t variable_id = 0;
    VkDescriptorSetAndBindingMappingEXT mapping_data;
};

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/main/docs/gpu_av_shader_instrumentation.md#instrumentation-status
//
// Goal is this needs to be very light when empty.
struct InstrumentationStatus {
    // Things that will be used at draw time on the CPU
    //
    // Note, things like |unique_shader_id| don't work in here as this object can both be used for a shaderModule and pipeline,
    // while something like |unique_shader_id| is tied to only a single shader.
    struct Host {
        // if anything was instrumented at all
        bool is_instrumented = false;

        // Prevent allocating DebugPrintf buffers if the shader didn't use it
        bool has_debug_printf = false;
    } host;

    // Things that will be read back only on an error message
    // This data is tied to a single shader, keyed on |unique_shader_id|
    struct Device {
        // The mappings used, index is encoded into the shader
        std::vector<HeapMappingStatus> heap_mappings;
    } device;

    // For things like pipeline/GPL we want to add on information we find from a single shader/library
    void Append(InstrumentationStatus other) {
        host.is_instrumented |= other.host.is_instrumented;
        host.has_debug_printf |= other.host.has_debug_printf;
    }
};

}  // namespace spirv
}  // namespace gpuav