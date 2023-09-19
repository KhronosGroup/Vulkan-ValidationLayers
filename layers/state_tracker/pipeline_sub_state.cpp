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

#include "state_tracker/pipeline_sub_state.h"
#include "state_tracker/pipeline_state.h"

VkPipelineLayoutCreateFlags PipelineSubState::PipelineLayoutCreateFlags() const {
    const auto layout_state = parent.PipelineLayoutState();
    return (layout_state) ? layout_state->CreateFlags() : static_cast<VkPipelineLayoutCreateFlags>(0);
}

VertexInputState::VertexInputState(const PIPELINE_STATE &p, const safe_VkGraphicsPipelineCreateInfo &create_info)
    : PipelineSubState(p), input_state(create_info.pVertexInputState), input_assembly_state(create_info.pInputAssemblyState) {
    if (create_info.pVertexInputState) {
        const auto *vici = create_info.pVertexInputState;
        if (vici->vertexBindingDescriptionCount) {
            const auto count = vici->vertexBindingDescriptionCount;
            binding_descriptions.reserve(count);
            binding_to_index_map.reserve(count);

            for (uint32_t i = 0; i < count; i++) {
                binding_descriptions.emplace_back(vici->pVertexBindingDescriptions[i]);
                binding_to_index_map[binding_descriptions.back().binding] = i;
            }
        }

        if (vici->vertexAttributeDescriptionCount) {
            vertex_attribute_descriptions.reserve(vici->vertexAttributeDescriptionCount);
            std::copy(vici->pVertexAttributeDescriptions,
                      vici->pVertexAttributeDescriptions + vici->vertexAttributeDescriptionCount,
                      std::back_inserter(vertex_attribute_descriptions));
        }

        vertex_attribute_alignments.reserve(vertex_attribute_descriptions.size());
        for (const auto &attr : vertex_attribute_descriptions) {
            VkDeviceSize vtx_attrib_req_alignment = vkuFormatElementSize(attr.format);
            if (vkuFormatElementIsTexel(attr.format)) {
                vtx_attrib_req_alignment = SafeDivision(vtx_attrib_req_alignment, vkuFormatComponentCount(attr.format));
            }
            vertex_attribute_alignments.push_back(vtx_attrib_req_alignment);
        }
    }
}

PreRasterState::PreRasterState(const PIPELINE_STATE &p, const ValidationStateTracker &state_data,
                               const safe_VkGraphicsPipelineCreateInfo &create_info, std::shared_ptr<const RENDER_PASS_STATE> rp)
    : PipelineSubState(p), rp_state(rp), subpass(create_info.subpass) {
    pipeline_layout = state_data.Get<PIPELINE_LAYOUT_STATE>(create_info.layout);

    viewport_state = create_info.pViewportState;

    rp_state = state_data.Get<RENDER_PASS_STATE>(create_info.renderPass);

    raster_state = create_info.pRasterizationState;

    tess_create_info = create_info.pTessellationState;

    VkShaderStageFlags all_stages = 0;

    for (uint32_t i = 0; i < create_info.stageCount; ++i) {
        const auto &stage_ci = create_info.pStages[i];
        const VkShaderStageFlagBits stage = stage_ci.stage;
        // TODO might need to filter out more than just fragment shaders here
        if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
            continue;
        }
        all_stages |= stage;

        auto module_state = state_data.Get<SHADER_MODULE_STATE>(stage_ci.module);
        if (!module_state) {
            // If module is null and there is a VkShaderModuleCreateInfo in the pNext chain of the stage info, then this
            // module is part of a library and the state must be created
            const auto shader_ci = vku::FindStructInPNextChain<VkShaderModuleCreateInfo>(stage_ci.pNext);
            if (shader_ci) {
                // don't need to worry about GroupDecoration in GPL
                auto spirv_module = std::make_shared<SPIRV_MODULE_STATE>(shader_ci->codeSize, shader_ci->pCode);
                module_state = std::make_shared<SHADER_MODULE_STATE>(VK_NULL_HANDLE, spirv_module, 0);
            }
        }

        // Check if a shader module identifier is used to reference the shader module.
        if (!module_state) {
            if (const auto shader_stage_id = vku::FindStructInPNextChain<VkPipelineShaderStageModuleIdentifierCreateInfoEXT>(stage_ci.pNext);
                shader_stage_id) {
                module_state = state_data.GetShaderModuleStateFromIdentifier(*shader_stage_id);
            }
        }

        if (module_state) {
            switch (stage) {
                case VK_SHADER_STAGE_VERTEX_BIT:
                    vertex_shader = std::move(module_state);
                    vertex_shader_ci = &stage_ci;
                    break;
                case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
                    tessc_shader = std::move(module_state);
                    tessc_shader_ci = &stage_ci;
                    break;
                case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
                    tesse_shader = std::move(module_state);
                    tesse_shader_ci = &stage_ci;
                    break;
                case VK_SHADER_STAGE_GEOMETRY_BIT:
                    geometry_shader = std::move(module_state);
                    geometry_shader_ci = &stage_ci;
                    break;
                case VK_SHADER_STAGE_TASK_BIT_EXT:
                    task_shader = std::move(module_state);
                    task_shader_ci = &stage_ci;
                    break;
                case VK_SHADER_STAGE_MESH_BIT_EXT:
                    mesh_shader = std::move(module_state);
                    mesh_shader_ci = &stage_ci;
                    break;
                default:
                    // TODO is this an error?
                    break;
            }
        }
    }

    if (all_stages & VK_SHADER_STAGE_MESH_BIT_EXT) {
        last_stage = VK_SHADER_STAGE_MESH_BIT_EXT;
    } else if (all_stages & VK_SHADER_STAGE_TASK_BIT_EXT) {
        last_stage = VK_SHADER_STAGE_TASK_BIT_EXT;
    } else if (all_stages & VK_SHADER_STAGE_GEOMETRY_BIT) {
        last_stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    } else if (all_stages & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
        last_stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    } else if (all_stages & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
        last_stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    } else if (all_stages & VK_SHADER_STAGE_VERTEX_BIT) {
        last_stage = VK_SHADER_STAGE_VERTEX_BIT;
    }
}

std::unique_ptr<const safe_VkPipelineColorBlendStateCreateInfo> ToSafeColorBlendState(
    const safe_VkPipelineColorBlendStateCreateInfo &cbs) {
    // This is needlessly copied here. Might better to make this a plain pointer, with an optional "backing unique_ptr"
    return std::make_unique<const safe_VkPipelineColorBlendStateCreateInfo>(cbs);
}
std::unique_ptr<const safe_VkPipelineColorBlendStateCreateInfo> ToSafeColorBlendState(
    const VkPipelineColorBlendStateCreateInfo &cbs) {
    return std::make_unique<const safe_VkPipelineColorBlendStateCreateInfo>(&cbs);
}
std::unique_ptr<const safe_VkPipelineMultisampleStateCreateInfo> ToSafeMultisampleState(
    const safe_VkPipelineMultisampleStateCreateInfo &cbs) {
    // This is needlessly copied here. Might better to make this a plain pointer, with an optional "backing unique_ptr"
    return std::make_unique<const safe_VkPipelineMultisampleStateCreateInfo>(cbs);
}
std::unique_ptr<const safe_VkPipelineMultisampleStateCreateInfo> ToSafeMultisampleState(
    const VkPipelineMultisampleStateCreateInfo &cbs) {
    return std::make_unique<const safe_VkPipelineMultisampleStateCreateInfo>(&cbs);
}
std::unique_ptr<const safe_VkPipelineDepthStencilStateCreateInfo> ToSafeDepthStencilState(
    const safe_VkPipelineDepthStencilStateCreateInfo &cbs) {
    // This is needlessly copied here. Might better to make this a plain pointer, with an optional "backing unique_ptr"
    return std::make_unique<const safe_VkPipelineDepthStencilStateCreateInfo>(cbs);
}
std::unique_ptr<const safe_VkPipelineDepthStencilStateCreateInfo> ToSafeDepthStencilState(
    const VkPipelineDepthStencilStateCreateInfo &cbs) {
    return std::make_unique<const safe_VkPipelineDepthStencilStateCreateInfo>(&cbs);
}
std::unique_ptr<const safe_VkPipelineShaderStageCreateInfo> ToShaderStageCI(const safe_VkPipelineShaderStageCreateInfo &cbs) {
    // This is needlessly copied here. Might better to make this a plain pointer, with an optional "backing unique_ptr"
    return std::make_unique<const safe_VkPipelineShaderStageCreateInfo>(cbs);
}
std::unique_ptr<const safe_VkPipelineShaderStageCreateInfo> ToShaderStageCI(const VkPipelineShaderStageCreateInfo &cbs) {
    return std::make_unique<const safe_VkPipelineShaderStageCreateInfo>(&cbs);
}

template <typename CreateInfo>
void SetFragmentShaderInfoPrivate(FragmentShaderState &fs_state, const ValidationStateTracker &state_data,
                                  const CreateInfo &create_info) {
    for (uint32_t i = 0; i < create_info.stageCount; ++i) {
        if (create_info.pStages[i].stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
            auto module_state = state_data.Get<SHADER_MODULE_STATE>(create_info.pStages[i].module);
            if (!module_state) {
                // If module is null and there is a VkShaderModuleCreateInfo in the pNext chain of the stage info, then this
                // module is part of a library and the state must be created
                const auto shader_ci = vku::FindStructInPNextChain<VkShaderModuleCreateInfo>(create_info.pStages[i].pNext);
                if (shader_ci) {
                    // don't need to worry about GroupDecoration in GPL
                    auto spirv_module = std::make_shared<SPIRV_MODULE_STATE>(shader_ci->codeSize, shader_ci->pCode);
                    module_state = std::make_shared<SHADER_MODULE_STATE>(VK_NULL_HANDLE, spirv_module, 0);
                }
            }

            // Check if a shader module identifier is used to reference the shader module.
            if (!module_state) {
                if (const auto shader_stage_id =
                        vku::FindStructInPNextChain<VkPipelineShaderStageModuleIdentifierCreateInfoEXT>(create_info.pStages[i].pNext);
                    shader_stage_id) {
                    module_state = state_data.GetShaderModuleStateFromIdentifier(*shader_stage_id);
                }
            }

            if (module_state) {
                fs_state.fragment_shader = std::move(module_state);
                fs_state.fragment_shader_ci = ToShaderStageCI(create_info.pStages[i]);
            }
        }
    }
}

// static
void FragmentShaderState::SetFragmentShaderInfo(FragmentShaderState &fs_state, const ValidationStateTracker &state_data,
                                                const VkGraphicsPipelineCreateInfo &create_info) {
    SetFragmentShaderInfoPrivate(fs_state, state_data, create_info);
}

// static
void FragmentShaderState::SetFragmentShaderInfo(FragmentShaderState &fs_state, const ValidationStateTracker &state_data,
                                                const safe_VkGraphicsPipelineCreateInfo &create_info) {
    SetFragmentShaderInfoPrivate(fs_state, state_data, create_info);
}

FragmentShaderState::FragmentShaderState(const PIPELINE_STATE &p, const ValidationStateTracker &dev_data,
                                         std::shared_ptr<const RENDER_PASS_STATE> rp, uint32_t subp, VkPipelineLayout layout)
    : PipelineSubState(p), rp_state(rp), subpass(subp), pipeline_layout(dev_data.Get<PIPELINE_LAYOUT_STATE>(layout)) {}

FragmentOutputState::FragmentOutputState(const PIPELINE_STATE &p, std::shared_ptr<const RENDER_PASS_STATE> rp, uint32_t sp)
    : PipelineSubState(p), rp_state(rp), subpass(sp) {}

// static
bool FragmentOutputState::IsBlendConstantsEnabled(const AttachmentVector &attachments) {
    bool result = false;
    for (const auto &attachment : attachments) {
        if (VK_TRUE == attachment.blendEnable) {
            if (((attachment.dstAlphaBlendFactor >= VK_BLEND_FACTOR_CONSTANT_COLOR) &&
                 (attachment.dstAlphaBlendFactor <= VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA)) ||
                ((attachment.dstColorBlendFactor >= VK_BLEND_FACTOR_CONSTANT_COLOR) &&
                 (attachment.dstColorBlendFactor <= VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA)) ||
                ((attachment.srcAlphaBlendFactor >= VK_BLEND_FACTOR_CONSTANT_COLOR) &&
                 (attachment.srcAlphaBlendFactor <= VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA)) ||
                ((attachment.srcColorBlendFactor >= VK_BLEND_FACTOR_CONSTANT_COLOR) &&
                 (attachment.srcColorBlendFactor <= VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA))) {
                result = true;
                break;
            }
        }
    }
    return result;
}

// static
bool FragmentOutputState::GetDualSourceBlending(const safe_VkPipelineColorBlendStateCreateInfo *color_blend_state) {
    if (!color_blend_state) {
        return false;
    }
    for (uint32_t i = 0; i < color_blend_state->attachmentCount; ++i) {
        const auto &attachment = color_blend_state->pAttachments[i];
        if (attachment.blendEnable) {
            if (IsSecondaryColorInputBlendFactor(attachment.srcColorBlendFactor) ||
                IsSecondaryColorInputBlendFactor(attachment.dstColorBlendFactor) ||
                IsSecondaryColorInputBlendFactor(attachment.srcAlphaBlendFactor) ||
                IsSecondaryColorInputBlendFactor(attachment.dstAlphaBlendFactor)) {
                return true;
            }
        }
    }
    return false;
}
