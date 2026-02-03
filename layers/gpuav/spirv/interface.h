/* Copyright (c) 2024-2026 LunarG, Inc.
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
#include <stdint.h>
#include <vulkan/vulkan_core.h>
#include <string>
#include <vector>

// The goal is to keep instrumentation a seperate library to draw a strong line where the GPU-AV SPIR-V logic is.
// This header is designed as the interface that can be shared between the instrumentation passes and the rest of GPU-AV

struct Location;

namespace gpuav {
namespace spirv {

// Each descriptor set can be tought as a linear, single buffer of descriptors (ignoring binding and arrays for the moment)
//
// Example:
//    layout(binding = 0) buffer a[4];
//    layout(binding = 2) buffer b;
//    layout(binding = 3) buffer c[2];
//
// We can think of this as being in a buffer as
//    [ a0, a1, a2, a3, b0, c0, c1]
//
// In order to do this, we need some sort of LUT, per BINDING, to know where in this LAYOUT of descriptors the binding starts.
// This means given the index into any binding, we can locate the exact descriptor in the entire descriptor set.
//
// This information used to be in a BDA buffer that the GPU would do a look-up and produced slow SPIR-V to compile/execute.
// Now that we do instrumentation at Pipeline creation time, we can just view the DescriptorSetLayout and inject this offset into
// the instrumentation.
//
// ** With Variable Descriptor Count, the buffer will only get smaller from the end.
//    We will still validate as being "uninitialized" in that case.
struct BindingLayout {
    uint32_t start;
    uint32_t count;
};

// When instrumenting, we need information about the array of VkDescriptorSetLayouts. The core issue is that for pipelines, we
// might have to merge 2 pipeline layouts together (because of GPL) and therefore both ShaderObject and PipelineLayout state
// objects don't have a single way to describe their VkDescriptorSetLayouts. If there are multiple shaders, we also want to only
// build this information once.
// This struct is designed to be filled in from both Pipeline and ShaderObject and then passed down to the SPIR-V Instrumentation,
// and afterwards we don't need to save it.
struct InstrumentationDescriptorSetLayouts {
    bool has_bindless_descriptors = false;
    // < set , [ bindings ] >
    std::vector<std::vector<spirv::BindingLayout>> set_index_to_bindings_layout_lut;

    // Pipeline flags for ray tracing validation hit objects
    bool pipeline_has_skip_aabbs_flag = false;
    bool pipeline_has_skip_triangles_flag = false;
    uint32_t max_shader_binding_table_record_index = 0;
};

// Top level struct to hold all the things we want to pass in from the Vulkan GPU-AV code into the SPIR-V instrumentation passes
struct InstrumentationInterface {
    // Used to do a look up the corresponding shader handle GpuShaderInstrumentor::instrumented_shaders_map_
    // (Embedded into the GLSL at linking time with SpecConstantLinkShaderId)
    uint32_t unique_shader_id = 0;

    // We only need to instrument the functions in the entry point
    const char* entry_point_name = nullptr;
    VkShaderStageFlagBits entry_point_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;

    InstrumentationDescriptorSetLayouts instrumentation_dsl;

    const Location& loc;

    explicit InstrumentationInterface(const Location& loc) : loc(loc) {}
};

// Global settings we would know at vkCreateDevice
// All setting must be set in FinishDeviceSetup, where defaults values are set
struct DeviceSettings {
    // Will replace the "OpDecorate DescriptorSet" for the output buffer in the incoming linked module
    // This allows anything to be set in the GLSL for the set value, as we change it at runtime
    uint32_t output_buffer_descriptor_set;
    // When off (unsafe mode) reduce amount of work so compiling the pipeline/shader is quicker
    // This is a global setting for all passes
    bool safe_mode;
    // Used to help debug
    bool print_debug_info;
    // zero is same as "unlimited"
    uint32_t max_instrumentations_count;
    bool support_non_semantic_info;
};

// When running the DebugPrintf pass, if we detect an instrumented shader has a printf call (for debugging) we can hold them until
// we need them after GPU execution. (Note, this is needed because we don't store the instrumented SPIR-V and have no way to get the
// OpString back afterwards)
struct InternalOnlyDebugPrintf {
    uint32_t unique_shader_id;
    uint32_t op_string_id;
    std::string op_string_text;
};

}  // namespace spirv
}  // namespace gpuav