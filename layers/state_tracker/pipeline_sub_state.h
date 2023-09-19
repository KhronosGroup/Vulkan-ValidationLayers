/* Copyright (c) 2015-2017, 2019-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2017, 2019-2023 Valve Corporation
 * Copyright (c) 2015-2017, 2019-2023 LunarG, Inc.
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
#include "generated/vk_safe_struct.h"

// Graphics pipeline sub-state as defined by VK_KHR_graphics_pipeline_library
//
class RENDER_PASS_STATE;
struct SHADER_MODULE_STATE;

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
    PipelineSubState(const PIPELINE_STATE &p) : parent(p) {}
    const PIPELINE_STATE &parent;

    VkPipelineLayoutCreateFlags PipelineLayoutCreateFlags() const;
};

struct VertexInputState : public PipelineSubState {
    VertexInputState(const PIPELINE_STATE &p, const safe_VkGraphicsPipelineCreateInfo &create_info);

    safe_VkPipelineVertexInputStateCreateInfo *input_state = nullptr;
    safe_VkPipelineInputAssemblyStateCreateInfo *input_assembly_state = nullptr;

    using VertexBindingVector = std::vector<VkVertexInputBindingDescription>;
    VertexBindingVector binding_descriptions;

    using VertexBindingIndexMap = vvl::unordered_map<uint32_t, uint32_t>;
    VertexBindingIndexMap binding_to_index_map;

    using VertexAttrVector = std::vector<VkVertexInputAttributeDescription>;
    VertexAttrVector vertex_attribute_descriptions;

    using VertexAttrAlignmentVector = std::vector<VkDeviceSize>;
    VertexAttrAlignmentVector vertex_attribute_alignments;

    std::shared_ptr<VertexInputState> FromCreateInfo(const ValidationStateTracker &state,
                                                     const safe_VkGraphicsPipelineCreateInfo &create_info);
};

struct PreRasterState : public PipelineSubState {
    PreRasterState(const PIPELINE_STATE &p, const ValidationStateTracker &dev_data,
                   const safe_VkGraphicsPipelineCreateInfo &create_info, std::shared_ptr<const RENDER_PASS_STATE> rp);

    static inline VkShaderStageFlags ValidShaderStages() {
        return VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT |
               VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
    }

    std::shared_ptr<const PIPELINE_LAYOUT_STATE> pipeline_layout;
    safe_VkPipelineViewportStateCreateInfo *viewport_state = nullptr;

    safe_VkPipelineRasterizationStateCreateInfo *raster_state = nullptr;

    std::shared_ptr<const RENDER_PASS_STATE> rp_state;
    uint32_t subpass = 0;

    VkShaderStageFlagBits last_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;

    std::shared_ptr<const SHADER_MODULE_STATE> tessc_shader, tesse_shader;
    const safe_VkPipelineShaderStageCreateInfo *tessc_shader_ci = nullptr, *tesse_shader_ci = nullptr;
    const safe_VkPipelineTessellationStateCreateInfo *tess_create_info = nullptr;

    std::shared_ptr<const SHADER_MODULE_STATE> vertex_shader, geometry_shader, task_shader, mesh_shader;
    const safe_VkPipelineShaderStageCreateInfo *vertex_shader_ci = nullptr, *geometry_shader_ci = nullptr,
                                               *task_shader_ci = nullptr, *mesh_shader_ci = nullptr;
};

std::unique_ptr<const safe_VkPipelineColorBlendStateCreateInfo> ToSafeColorBlendState(
    const safe_VkPipelineColorBlendStateCreateInfo &cbs);
std::unique_ptr<const safe_VkPipelineColorBlendStateCreateInfo> ToSafeColorBlendState(
    const VkPipelineColorBlendStateCreateInfo &cbs);
std::unique_ptr<const safe_VkPipelineMultisampleStateCreateInfo> ToSafeMultisampleState(
    const safe_VkPipelineMultisampleStateCreateInfo &cbs);
std::unique_ptr<const safe_VkPipelineMultisampleStateCreateInfo> ToSafeMultisampleState(
    const VkPipelineMultisampleStateCreateInfo &cbs);
std::unique_ptr<const safe_VkPipelineDepthStencilStateCreateInfo> ToSafeDepthStencilState(
    const safe_VkPipelineDepthStencilStateCreateInfo &cbs);
std::unique_ptr<const safe_VkPipelineDepthStencilStateCreateInfo> ToSafeDepthStencilState(
    const VkPipelineDepthStencilStateCreateInfo &cbs);
std::unique_ptr<const safe_VkPipelineShaderStageCreateInfo> ToShaderStageCI(const safe_VkPipelineShaderStageCreateInfo &cbs);
std::unique_ptr<const safe_VkPipelineShaderStageCreateInfo> ToShaderStageCI(const VkPipelineShaderStageCreateInfo &cbs);

struct FragmentShaderState : public PipelineSubState {
    FragmentShaderState(const PIPELINE_STATE &p, const ValidationStateTracker &dev_data,
                        std::shared_ptr<const RENDER_PASS_STATE> rp, uint32_t subpass, VkPipelineLayout layout);

    template <typename CreateInfo>
    FragmentShaderState(const PIPELINE_STATE &p, const ValidationStateTracker &dev_data, const CreateInfo &create_info,
                        std::shared_ptr<const RENDER_PASS_STATE> rp)
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

    std::shared_ptr<const RENDER_PASS_STATE> rp_state;
    uint32_t subpass = 0;

    std::shared_ptr<const PIPELINE_LAYOUT_STATE> pipeline_layout;
    std::unique_ptr<const safe_VkPipelineMultisampleStateCreateInfo> ms_state;
    std::unique_ptr<const safe_VkPipelineDepthStencilStateCreateInfo> ds_state;

    std::shared_ptr<const SHADER_MODULE_STATE> fragment_shader;
    std::unique_ptr<const safe_VkPipelineShaderStageCreateInfo> fragment_shader_ci;

  private:
    static void SetFragmentShaderInfo(FragmentShaderState &fs_state, const ValidationStateTracker &state_data,
                                      const VkGraphicsPipelineCreateInfo &create_info);
    static void SetFragmentShaderInfo(FragmentShaderState &fs_state, const ValidationStateTracker &state_data,
                                      const safe_VkGraphicsPipelineCreateInfo &create_info);
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
    using AttachmentVector = std::vector<VkPipelineColorBlendAttachmentState>;

    FragmentOutputState(const PIPELINE_STATE &p, std::shared_ptr<const RENDER_PASS_STATE> rp, uint32_t sp);
    // For a graphics library, a "non-safe" create info must be passed in in order for pColorBlendState and pMultisampleState to not
    // get stripped out. If this is a "normal" pipeline, then we want to keep the logic from safe_VkGraphicsPipelineCreateInfo that
    // strips out pointers that should be ignored.
    template <typename CreateInfo>
    FragmentOutputState(const PIPELINE_STATE &p, const CreateInfo &create_info, std::shared_ptr<const RENDER_PASS_STATE> rp)
        : FragmentOutputState(p, rp, create_info.subpass) {
        if (create_info.pColorBlendState) {
            const auto &cbci = *create_info.pColorBlendState;
            color_blend_state = ToSafeColorBlendState(cbci);
            // In case of being dynamic state
            if (cbci.pAttachments) {
                dual_source_blending = GetDualSourceBlending(color_blend_state.get());
                if (cbci.attachmentCount) {
                    attachments.reserve(cbci.attachmentCount);
                    std::copy(cbci.pAttachments, cbci.pAttachments + cbci.attachmentCount, std::back_inserter(attachments));
                }
                blend_constants_enabled = IsBlendConstantsEnabled(attachments);
            }
        }

        if (create_info.pMultisampleState) {
            ms_state = ToSafeMultisampleState(*create_info.pMultisampleState);
            sample_location_enabled = IsSampleLocationEnabled(create_info);
        }

        // TODO
        // auto format_ci = vku::FindStructInPNextChain<VkPipelineRenderingFormatCreateInfoKHR>(gpci->pNext);
    }

    static bool IsBlendConstantsEnabled(const AttachmentVector &attachments);
    static bool GetDualSourceBlending(const safe_VkPipelineColorBlendStateCreateInfo *color_blend_state);

    std::shared_ptr<const RENDER_PASS_STATE> rp_state;
    uint32_t subpass = 0;

    std::unique_ptr<const safe_VkPipelineColorBlendStateCreateInfo> color_blend_state;
    std::unique_ptr<const safe_VkPipelineMultisampleStateCreateInfo> ms_state;

    AttachmentVector attachments;

    bool blend_constants_enabled = false;  // Blend constants enabled for any attachments
    bool sample_location_enabled = false;
    bool dual_source_blending = false;
};
