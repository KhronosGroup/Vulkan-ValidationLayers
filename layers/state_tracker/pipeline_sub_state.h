/* Copyright (c) 2015-2017, 2019-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2017, 2019-2024 Valve Corporation
 * Copyright (c) 2015-2017, 2019-2024 LunarG, Inc.
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
#include <vulkan/utility/vk_safe_struct.hpp>

// Graphics pipeline sub-state as defined by VK_KHR_graphics_pipeline_library

class ValidationStateTracker;
namespace vvl {
class RenderPass;
class Pipeline;
class PipelineLayout;
struct ShaderModule;
}  // namespace vvl

namespace spirv {
struct EntryPoint;
}  // namespace spirv

template <typename CreateInfoType>
static inline VkGraphicsPipelineLibraryFlagsEXT GetGraphicsLibType(const CreateInfoType &create_info) {
    const auto lib_ci = vku::FindStructInPNextChain<VkGraphicsPipelineLibraryCreateInfoEXT>(create_info.pNext);
    if (lib_ci) {
        return lib_ci->flags;
    }
    return static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0);
}

// Common amoung all pipeline sub state
struct PipelineSubState {
    PipelineSubState(const vvl::Pipeline &p) : parent(p) {}
    const vvl::Pipeline &parent;

    VkPipelineLayoutCreateFlags PipelineLayoutCreateFlags() const;
};

struct VertexInputState : public PipelineSubState {
    VertexInputState(const vvl::Pipeline &p, const vku::safe_VkGraphicsPipelineCreateInfo &create_info);

    vku::safe_VkPipelineVertexInputStateCreateInfo *input_state = nullptr;
    vku::safe_VkPipelineInputAssemblyStateCreateInfo *input_assembly_state = nullptr;

    std::vector<VkVertexInputBindingDescription> binding_descriptions;

    vvl::unordered_map<uint32_t, uint32_t> binding_to_index_map;

    std::vector<VkVertexInputAttributeDescription2EXT> vertex_attribute_descriptions;

    std::shared_ptr<VertexInputState> FromCreateInfo(const ValidationStateTracker &state,
                                                     const vku::safe_VkGraphicsPipelineCreateInfo &create_info);
};

struct PreRasterState : public PipelineSubState {
    PreRasterState(const vvl::Pipeline &p, const ValidationStateTracker &dev_data,
                   const vku::safe_VkGraphicsPipelineCreateInfo &create_info, std::shared_ptr<const vvl::RenderPass> rp);

    static inline VkShaderStageFlags ValidShaderStages() {
        return VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT |
               VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
    }

    std::shared_ptr<const vvl::PipelineLayout> pipeline_layout;
    vku::safe_VkPipelineViewportStateCreateInfo *viewport_state = nullptr;
    vku::safe_VkPipelineRasterizationStateCreateInfo *raster_state = nullptr;
    const vku::safe_VkPipelineTessellationStateCreateInfo *tessellation_state = nullptr;

    std::shared_ptr<const vvl::RenderPass> rp_state;
    uint32_t subpass = 0;

    VkShaderStageFlagBits last_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;

    std::shared_ptr<const vvl::ShaderModule> tessc_shader, tesse_shader;
    const vku::safe_VkPipelineShaderStageCreateInfo *tessc_shader_ci = nullptr, *tesse_shader_ci = nullptr;

    std::shared_ptr<const vvl::ShaderModule> vertex_shader, geometry_shader, task_shader, mesh_shader;
    const vku::safe_VkPipelineShaderStageCreateInfo *vertex_shader_ci = nullptr, *geometry_shader_ci = nullptr,
                                                   *task_shader_ci = nullptr, *mesh_shader_ci = nullptr;
};

std::unique_ptr<const vku::safe_VkPipelineColorBlendStateCreateInfo> ToSafeColorBlendState(
    const vku::safe_VkPipelineColorBlendStateCreateInfo &cbs);
std::unique_ptr<const vku::safe_VkPipelineColorBlendStateCreateInfo> ToSafeColorBlendState(
    const VkPipelineColorBlendStateCreateInfo &cbs);
std::unique_ptr<const vku::safe_VkPipelineMultisampleStateCreateInfo> ToSafeMultisampleState(
    const vku::safe_VkPipelineMultisampleStateCreateInfo &cbs);
std::unique_ptr<const vku::safe_VkPipelineMultisampleStateCreateInfo> ToSafeMultisampleState(
    const VkPipelineMultisampleStateCreateInfo &cbs);
std::unique_ptr<const vku::safe_VkPipelineDepthStencilStateCreateInfo> ToSafeDepthStencilState(
    const vku::safe_VkPipelineDepthStencilStateCreateInfo &cbs);
std::unique_ptr<const vku::safe_VkPipelineDepthStencilStateCreateInfo> ToSafeDepthStencilState(
    const VkPipelineDepthStencilStateCreateInfo &cbs);
std::unique_ptr<const vku::safe_VkPipelineShaderStageCreateInfo> ToShaderStageCI(
    const vku::safe_VkPipelineShaderStageCreateInfo &cbs);
std::unique_ptr<const vku::safe_VkPipelineShaderStageCreateInfo> ToShaderStageCI(const VkPipelineShaderStageCreateInfo &cbs);

struct FragmentShaderState : public PipelineSubState {
    FragmentShaderState(const vvl::Pipeline &p, const ValidationStateTracker &dev_data, std::shared_ptr<const vvl::RenderPass> rp,
                        uint32_t subpass, VkPipelineLayout layout);

    template <typename CreateInfo>
    FragmentShaderState(const vvl::Pipeline &p, const ValidationStateTracker &dev_data, const CreateInfo &create_info,
                        std::shared_ptr<const vvl::RenderPass> rp)
        : FragmentShaderState(p, dev_data, rp, create_info.subpass, create_info.layout) {
        if (create_info.pMultisampleState) {
            ms_state = ToSafeMultisampleState(*create_info.pMultisampleState);
        }
        if (create_info.pDepthStencilState) {
            ds_state = ToSafeDepthStencilState(*create_info.pDepthStencilState);
        }
        FragmentShaderState::SetFragmentShaderInfo(*this, dev_data, create_info);
    }

    static inline VkShaderStageFlags ValidShaderStages() { return VK_SHADER_STAGE_FRAGMENT_BIT; }

    std::shared_ptr<const vvl::RenderPass> rp_state;
    uint32_t subpass = 0;

    std::shared_ptr<const vvl::PipelineLayout> pipeline_layout;
    std::unique_ptr<const vku::safe_VkPipelineMultisampleStateCreateInfo> ms_state;
    std::unique_ptr<const vku::safe_VkPipelineDepthStencilStateCreateInfo> ds_state;

    std::shared_ptr<const vvl::ShaderModule> fragment_shader;
    std::unique_ptr<const vku::safe_VkPipelineShaderStageCreateInfo> fragment_shader_ci;
    // many times we need to quickly get the entry point to access the SPIR-V static data
    std::shared_ptr<const spirv::EntryPoint> fragment_entry_point;

  private:
    static void SetFragmentShaderInfo(FragmentShaderState &fs_state, const ValidationStateTracker &state_data,
                                      const VkGraphicsPipelineCreateInfo &create_info);
    static void SetFragmentShaderInfo(FragmentShaderState &fs_state, const ValidationStateTracker &state_data,
                                      const vku::safe_VkGraphicsPipelineCreateInfo &create_info);
};

template <typename CreateInfo>
static bool IsSampleLocationEnabled(const CreateInfo &create_info) {
    bool result = false;
    if (create_info.pMultisampleState) {
        const auto *sample_location_state =
            vku::FindStructInPNextChain<VkPipelineSampleLocationsStateCreateInfoEXT>(create_info.pMultisampleState->pNext);
        if (sample_location_state != nullptr) {
            result = (sample_location_state->sampleLocationsEnable != 0);
        }
    }
    return result;
}

struct FragmentOutputState : public PipelineSubState {
    using AttachmentStateVector = std::vector<VkPipelineColorBlendAttachmentState>;

    FragmentOutputState(const vvl::Pipeline &p, std::shared_ptr<const vvl::RenderPass> rp, uint32_t sp);
    // For a graphics library, a "non-safe" create info must be passed in in order for pColorBlendState and pMultisampleState to not
    // get stripped out. If this is a "normal" pipeline, then we want to keep the logic from vku::safe_VkGraphicsPipelineCreateInfo
    // that strips out pointers that should be ignored.
    template <typename CreateInfo>
    FragmentOutputState(const vvl::Pipeline &p, const CreateInfo &create_info, std::shared_ptr<const vvl::RenderPass> rp)
        : FragmentOutputState(p, rp, create_info.subpass) {
        if (create_info.pColorBlendState) {
            const auto &cbci = *create_info.pColorBlendState;
            color_blend_state = ToSafeColorBlendState(cbci);
            // In case of being dynamic state
            if (cbci.pAttachments) {
                dual_source_blending = GetDualSourceBlending(color_blend_state.get());
                if (cbci.attachmentCount) {
                    attachment_states.reserve(cbci.attachmentCount);
                    std::copy(cbci.pAttachments, cbci.pAttachments + cbci.attachmentCount, std::back_inserter(attachment_states));
                }
                blend_constants_enabled = IsBlendConstantsEnabled(attachment_states);
            }
        }

        if (create_info.pMultisampleState) {
            ms_state = ToSafeMultisampleState(*create_info.pMultisampleState);
            sample_location_enabled = IsSampleLocationEnabled(create_info);
        }

        // TODO
        // auto format_ci = vku::FindStructInPNextChain<VkPipelineRenderingFormatCreateInfoKHR>(gpci->pNext);
    }

    static bool IsBlendConstantsEnabled(const AttachmentStateVector &attachment_states);
    static bool GetDualSourceBlending(const vku::safe_VkPipelineColorBlendStateCreateInfo *color_blend_state);

    std::shared_ptr<const vvl::RenderPass> rp_state;
    uint32_t subpass = 0;

    std::unique_ptr<const vku::safe_VkPipelineColorBlendStateCreateInfo> color_blend_state;
    std::unique_ptr<const vku::safe_VkPipelineMultisampleStateCreateInfo> ms_state;

    AttachmentStateVector attachment_states;

    bool blend_constants_enabled = false;  // Blend constants enabled for any attachments
    bool sample_location_enabled = false;
    bool dual_source_blending = false;
};
