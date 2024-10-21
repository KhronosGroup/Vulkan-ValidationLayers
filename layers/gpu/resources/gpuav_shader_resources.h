/* Copyright (c) 2018-2024 The Khronos Group Inc.
 * Copyright (c) 2018-2024 Valve Corporation
 * Copyright (c) 2018-2024 LunarG, Inc.
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

#include <vector>

#include "gpu/descriptor_validation/gpuav_descriptor_set.h"
#include "gpu/shaders/gpuav_shaders_constants.h"
#include "gpu/resources/gpuav_resources.h"

namespace gpuav {

struct DescSetState {
    std::shared_ptr<DescriptorSet> state = {};
    BindingVariableMap binding_req_map = {};
};

struct DescBindingInfo {
    DeviceMemoryBlock bindless_state;
    // Hold a buffer for each descriptor set
    // Note: The index here is from vkCmdBindDescriptorSets::firstSet
    std::vector<DescSetState> descriptor_set_buffers;

    DescBindingInfo(Validator &gpuav) : bindless_state(gpuav) {}
};

// These match the Structures found in the instrumentation GLSL logic
namespace glsl {

struct DescriptorSetRecord {
    VkDeviceAddress descriptor_index_lut;
    VkDeviceAddress ds_type;
    VkDeviceAddress descriptor_index_post_process;
};

struct BindlessStateBuffer {
    VkDeviceAddress global_state;
    DescriptorSetRecord desc_sets[kDebugInputBindlessMaxDescSets];
};

// Represented as a uvec2 in the shader
struct BindingLayout {
    uint32_t count;
    uint32_t state_start;
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
            default:
                assert(false);
        }
        return 0;
    }
};

}  // namespace glsl

}  // namespace gpuav
