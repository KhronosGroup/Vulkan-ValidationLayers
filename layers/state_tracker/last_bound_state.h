/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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

#include "state_tracker/pipeline_layout_state.h"
#include "utils/shader_utils.h"
#include "generated/dynamic_state_helper.h"
#include "generated/error_location_helper.h"
#include <optional>

namespace vvl {
class DescriptorSet;
class Descriptor;
class CommandBuffer;
class Pipeline;
struct ShaderObject;
class PipelineLayout;
}  // namespace vvl

namespace spirv {
struct EntryPoint;
}  // namespace spirv

// Track last states that are bound per pipeline bind point (Gfx & Compute)
struct LastBound {
    LastBound(vvl::CommandBuffer &cb) : cb_state(cb) {}

    vvl::CommandBuffer &cb_state;
    vvl::Pipeline *pipeline_state = nullptr;
    // All shader stages for a used pipeline bind point must be bound to with a valid shader or VK_NULL_HANDLE
    // We have to track shader_object_bound, because shader_object_states will be nullptr when VK_NULL_HANDLE is used
    bool shader_object_bound[kShaderObjectStageCount]{false};
    vvl::ShaderObject *shader_object_states[kShaderObjectStageCount]{nullptr};
    // The compatible layout used binding descriptor sets (track location to provide better error message)
    std::shared_ptr<const vvl::PipelineLayout> desc_set_pipeline_layout;
    vvl::Func desc_set_bound_command = vvl::Func::Empty;  // will be something like vkCmdBindDescriptorSets
    std::shared_ptr<vvl::DescriptorSet> push_descriptor_set;

    struct DescriptorBufferBinding {
        uint32_t index = 0;
        VkDeviceSize offset = 0;
    };

    // Each command buffer has a "slot" to hold a descriptor set binding. This "slot" also might be empty
    struct DescriptorSetSlot {
        std::shared_ptr<vvl::DescriptorSet> ds_state;
        std::optional<DescriptorBufferBinding> descriptor_buffer_binding;

        // one dynamic offset per dynamic descriptor bound to this CB
        std::vector<uint32_t> dynamic_offsets;
        PipelineLayoutCompatId compat_id_for_set{0};

        // Cache most recently validated descriptor state for ValidateActionState/UpdateImageLayoutDrawState
        const vvl::DescriptorSet *validated_set{nullptr};
        uint64_t validated_set_change_count{~0ULL};
        uint64_t validated_set_image_layout_change_count{~0ULL};

        void Reset() {
            ds_state.reset();
            descriptor_buffer_binding.reset();
            dynamic_offsets.clear();
        }
    };

    // Ordered bound set tracking where index is set# that given set is bound to
    std::vector<DescriptorSetSlot> ds_slots;

    void Reset();

    void UnbindAndResetPushDescriptorSet(std::shared_ptr<vvl::DescriptorSet> &&ds);

    // For shaderObject, everything is dynamic
    bool IsDynamic(const CBDynamicState state) const;

    // Dynamic State helpers that require both the Pipeline and CommandBuffer state are here
    bool IsDepthTestEnable() const;
    bool IsDepthBoundTestEnable() const;
    bool IsDepthWriteEnable() const;
    bool IsDepthBiasEnable() const;
    bool IsDepthClampEnable() const;
    bool IsStencilTestEnable() const;
    VkStencilOpState GetStencilOpStateFront() const;
    VkStencilOpState GetStencilOpStateBack() const;
    VkSampleCountFlagBits GetRasterizationSamples() const;
    bool IsRasterizationDisabled() const;
    bool IsLogicOpEnabled() const;
    VkColorComponentFlags GetColorWriteMask(uint32_t i) const;
    bool IsColorWriteEnabled(uint32_t i) const;
    VkPrimitiveTopology GetPrimitiveTopology() const;
    VkCullModeFlags GetCullMode() const;
    VkConservativeRasterizationModeEXT GetConservativeRasterizationMode() const;
    bool IsSampleLocationsEnable() const;
    bool IsExclusiveScissorEnabled() const;
    bool IsCoverageToColorEnabled() const;
    bool IsCoverageModulationTableEnable() const;
    bool IsDiscardRectangleEnable() const;
    bool IsStippledLineEnable() const;
    bool IsShadingRateImageEnable() const;
    bool IsViewportWScalingEnable() const;
    bool IsPrimitiveRestartEnable() const;
    bool IsAlphaToCoverageEnable() const;
    bool IsAlphaToOneEnable() const;
    VkCoverageModulationModeNV GetCoverageModulationMode() const;

    bool ValidShaderObjectCombination(const VkPipelineBindPoint bind_point, const DeviceFeatures &device_features) const;
    VkShaderEXT GetShader(ShaderObjectStage stage) const;
    vvl::ShaderObject *GetShaderState(ShaderObjectStage stage) const;
    const vvl::ShaderObject *GetShaderStateIfValid(ShaderObjectStage stage) const;
    // Return compute shader for compute pipeline, vertex or mesh shader for graphics
    const vvl::ShaderObject *GetFirstShader(VkPipelineBindPoint bind_point) const;
    bool HasShaderObjects() const;
    bool IsValidShaderBound(ShaderObjectStage stage) const;
    bool IsValidShaderOrNullBound(ShaderObjectStage stage) const;
    std::vector<vvl::ShaderObject *> GetAllBoundGraphicsShaders();
    bool IsAnyGraphicsShaderBound() const;
    VkShaderStageFlags GetAllActiveBoundStages() const;

    bool IsBoundSetCompatible(uint32_t set, const vvl::PipelineLayout &pipeline_layout) const;
    bool IsBoundSetCompatible(uint32_t set, const vvl::ShaderObject &shader_object_state) const;
    std::string DescribeNonCompatibleSet(uint32_t set, const vvl::PipelineLayout &pipeline_layout) const;
    std::string DescribeNonCompatibleSet(uint32_t set, const vvl::ShaderObject &shader_object_state) const;

    const spirv::EntryPoint *GetVertexEntryPoint() const;
    const spirv::EntryPoint *GetFragmentEntryPoint() const;
};

enum LvlBindPoint {
    BindPoint_Graphics = VK_PIPELINE_BIND_POINT_GRAPHICS,
    BindPoint_Compute = VK_PIPELINE_BIND_POINT_COMPUTE,
    BindPoint_Ray_Tracing = 2,
    BindPoint_Count = 3,
};

static VkPipelineBindPoint inline ConvertToPipelineBindPoint(LvlBindPoint bind_point) {
    switch (bind_point) {
        case BindPoint_Graphics:
            return VK_PIPELINE_BIND_POINT_GRAPHICS;
        case BindPoint_Compute:
            return VK_PIPELINE_BIND_POINT_COMPUTE;
        case BindPoint_Ray_Tracing:
            return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
        default:
            break;
    }
    assert(false);
    return VK_PIPELINE_BIND_POINT_GRAPHICS;
}

static LvlBindPoint inline ConvertToLvlBindPoint(VkPipelineBindPoint bind_point) {
    switch (bind_point) {
        case VK_PIPELINE_BIND_POINT_GRAPHICS:
            return BindPoint_Graphics;
        case VK_PIPELINE_BIND_POINT_COMPUTE:
            return BindPoint_Compute;
        case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:
            return BindPoint_Ray_Tracing;
        default:
            break;
    }
    assert(false);
    return BindPoint_Graphics;
}

static VkPipelineBindPoint inline ConvertToPipelineBindPoint(VkShaderStageFlagBits stage) {
    switch (stage) {
        case VK_SHADER_STAGE_VERTEX_BIT:
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
        case VK_SHADER_STAGE_GEOMETRY_BIT:
        case VK_SHADER_STAGE_FRAGMENT_BIT:
        case VK_SHADER_STAGE_TASK_BIT_EXT:
        case VK_SHADER_STAGE_MESH_BIT_EXT:
            return VK_PIPELINE_BIND_POINT_GRAPHICS;
        case VK_SHADER_STAGE_COMPUTE_BIT:
            return VK_PIPELINE_BIND_POINT_COMPUTE;
        case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
        case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
        case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
        case VK_SHADER_STAGE_MISS_BIT_KHR:
        case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
        case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
            return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
        default:
            break;
    }
    assert(false);
    return VK_PIPELINE_BIND_POINT_GRAPHICS;
}

static VkPipelineBindPoint inline ConvertToPipelineBindPoint(VkShaderStageFlags stage) {
    // Assumes the call has checked stages have not been mixed
    if (stage & kShaderStageAllGraphics) {
        return VK_PIPELINE_BIND_POINT_GRAPHICS;
    } else if (stage & VK_SHADER_STAGE_COMPUTE_BIT) {
        return VK_PIPELINE_BIND_POINT_COMPUTE;
    } else if (stage & kShaderStageAllRayTracing) {
        return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
    } else {
        assert(false);
        return VK_PIPELINE_BIND_POINT_GRAPHICS;
    }
}
static LvlBindPoint inline ConvertToLvlBindPoint(VkShaderStageFlagBits stage) {
    switch (stage) {
        case VK_SHADER_STAGE_VERTEX_BIT:
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
        case VK_SHADER_STAGE_GEOMETRY_BIT:
        case VK_SHADER_STAGE_FRAGMENT_BIT:
        case VK_SHADER_STAGE_TASK_BIT_EXT:
        case VK_SHADER_STAGE_MESH_BIT_EXT:
            return BindPoint_Graphics;
        case VK_SHADER_STAGE_COMPUTE_BIT:
            return BindPoint_Compute;
        case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
        case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
        case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
        case VK_SHADER_STAGE_MISS_BIT_KHR:
        case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
        case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
            return BindPoint_Ray_Tracing;
        default:
            break;
    }
    assert(false);
    return BindPoint_Graphics;
}
