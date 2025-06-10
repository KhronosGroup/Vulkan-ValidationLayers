/* Copyright (c) 2018-2025 The Khronos Group Inc.
 * Copyright (c) 2018-2025 Valve Corporation
 * Copyright (c) 2018-2025 LunarG, Inc.
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

// This file helps maps the layout of resources that we find in the instrumented shaders

#pragma once

#include "state_tracker/descriptor_sets.h"
#include "gpuav/shaders/gpuav_shaders_constants.h"

namespace gpuav {

// These match the Structures found in the instrumentation GLSL logic
namespace glsl {

// Every descriptor set has various BDA pointers to data from the CPU
// Shared among all Descriptor Indexing GPU-AV checks (so we only have to create a single buffer)
struct BoundDescriptorSetsStateSSBO {
    // Used to know if descriptors are initialized or not
    VkDeviceAddress descriptor_init_status;
    VkDeviceAddress descriptor_set_types[kDebugInputBindlessMaxDescSets];
};

// Outputs
struct PostProcessSSBO {
    VkDeviceAddress descriptor_index_post_process_buffers[kDebugInputBindlessMaxDescSets];
};

// Represented as a uvec2 in the shader
// TODO - Currently duplicated as needed by various parts
// see interface.h for details
struct BindingLayout {
    uint32_t start;
    uint32_t count;
};

// Represented as a uvec2 in the shader
// For each descriptor index we have a "slot" to mark what happend on the GPU.
struct PostProcessDescriptorIndexSlot {
    // see gpuav_shaders_constants.h for how we split this metadata up
    uint32_t meta_data;
    // OpVariable ID of descriptor accessed.
    // This is required to distinguish between 2 aliased descriptors
    uint32_t variable_id;
};

// Represented as a uvec2 in the shader
struct DescriptorState {
    DescriptorState() : id(0), extra_data(0) {}
    DescriptorState(vvl::DescriptorClass dc, uint32_t id_, uint32_t extra_data_ = 1)
        : id(ClassToShaderBits(dc) | id_), extra_data(extra_data_) {}
    uint32_t id;
    uint32_t extra_data;

    static uint32_t ClassToShaderBits(vvl::DescriptorClass dc) {
        switch (dc) {
            case vvl::DescriptorClass::PlainSampler:
                return (kSamplerDesc << kDescBitShift);
            case vvl::DescriptorClass::ImageSampler:
                return (kImageSamplerDesc << kDescBitShift);
            case vvl::DescriptorClass::Image:
                return (kImageDesc << kDescBitShift);
            case vvl::DescriptorClass::TexelBuffer:
                return (kTexelDesc << kDescBitShift);
            case vvl::DescriptorClass::GeneralBuffer:
                return (kBufferDesc << kDescBitShift);
            case vvl::DescriptorClass::InlineUniform:
                return (kInlineUniformDesc << kDescBitShift);
            case vvl::DescriptorClass::AccelerationStructure:
                return (kAccelDesc << kDescBitShift);
            case vvl::DescriptorClass::Mutable:
            case vvl::DescriptorClass::Invalid:
                assert(false);
                break;
        }
        return 0;
    }
};

}  // namespace glsl

}  // namespace gpuav
