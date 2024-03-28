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

#include "state_tracker/pipeline_sub_state.h"
#include "state_tracker/pipeline_state.h"
#include "state_tracker/shader_module.h"

VkPipelineLayoutCreateFlags PipelineSubState::PipelineLayoutCreateFlags() const {
    const auto layout_state = parent.PipelineLayoutState();
    return (layout_state) ? layout_state->CreateFlags() : static_cast<VkPipelineLayoutCreateFlags>(0);
}

VertexInputState::VertexInputState(const vvl::Pipeline &p, const vku::safe_VkGraphicsPipelineCreateInfo &create_info)
    : PipelineSubState(p) {
    for (uint32_t i = 0; i < create_info.stageCount; i++) {
        if (create_info.pStages && create_info.pStages[i].stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
            return;  // if mesh shaders are used, all vertex input state is ignored
        }
    }
    input_state = create_info.pVertexInputState;
    input_assembly_state = create_info.pInputAssemblyState;

    if (input_state) {
        if (input_state->vertexBindingDescriptionCount) {
            const uint32_t count = input_state->vertexBindingDescriptionCount;
            binding_descriptions.reserve(count);
            binding_to_index_map.reserve(count);

            for (uint32_t i = 0; i < count; i++) {
                binding_descriptions.emplace_back(input_state->pVertexBindingDescriptions[i]);
                binding_to_index_map[binding_descriptions.back().binding] = i;
            }
        }

        vertex_attribute_descriptions.reserve(input_state->vertexAttributeDescriptionCount);
        for (const auto [i, description] :
             vvl::enumerate(input_state->pVertexAttributeDescriptions, input_state->vertexAttributeDescriptionCount)) {
            vertex_attribute_descriptions.emplace_back(vku::InitStruct<VkVertexInputAttributeDescription2EXT>(
                nullptr, description->location, description->binding, description->format, description->offset));
        }
    }
}

PreRasterState::PreRasterState(const vvl::Pipeline &p, const ValidationStateTracker &state_data,
                               const vku::safe_VkGraphicsPipelineCreateInfo &create_info, std::shared_ptr<const vvl::RenderPass> rp)
    : PipelineSubState(p),
      pipeline_layout(state_data.Get<vvl::PipelineLayout>(create_info.layout)),
      viewport_state(create_info.pViewportState),
      raster_state(create_info.pRasterizationState),
      tessellation_state(create_info.pTessellationState),
      rp_state(rp),
      subpass(create_info.subpass) {
    VkShaderStageFlags all_stages = 0;

    for (uint32_t i = 0; i < create_info.stageCount; ++i) {
        const auto &stage_ci = create_info.pStages[i];
        const VkShaderStageFlagBits stage = stage_ci.stage;
        // TODO might need to filter out more than just fragment shaders here
        if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
            continue;
        }
        all_stages |= stage;

        auto module_state = state_data.Get<vvl::ShaderModule>(stage_ci.module);
        if (!module_state) {
            // If module is null and there is a VkShaderModuleCreateInfo in the pNext chain of the stage info, then this
            // module is part of a library and the state must be created
            const auto shader_ci = vku::FindStructInPNextChain<VkShaderModuleCreateInfo>(stage_ci.pNext);
            if (shader_ci) {
                // don't need to worry about GroupDecoration in GPL
                auto spirv_module = std::make_shared<spirv::Module>(shader_ci->codeSize, shader_ci->pCode);
                module_state = std::make_shared<vvl::ShaderModule>(VK_NULL_HANDLE, spirv_module, 0);
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

std::unique_ptr<const vku::safe_VkPipelineColorBlendStateCreateInfo> ToSafeColorBlendState(
    const vku::safe_VkPipelineColorBlendStateCreateInfo &cbs) {
    // This is needlessly copied here. Might better to make this a plain pointer, with an optional "backing unique_ptr"
    return std::make_unique<const vku::safe_VkPipelineColorBlendStateCreateInfo>(cbs);
}
std::unique_ptr<const vku::safe_VkPipelineColorBlendStateCreateInfo> ToSafeColorBlendState(
    const VkPipelineColorBlendStateCreateInfo &cbs) {
    return std::make_unique<const vku::safe_VkPipelineColorBlendStateCreateInfo>(&cbs);
}
std::unique_ptr<const vku::safe_VkPipelineMultisampleStateCreateInfo> ToSafeMultisampleState(
    const vku::safe_VkPipelineMultisampleStateCreateInfo &cbs) {
    // This is needlessly copied here. Might better to make this a plain pointer, with an optional "backing unique_ptr"
    return std::make_unique<const vku::safe_VkPipelineMultisampleStateCreateInfo>(cbs);
}
std::unique_ptr<const vku::safe_VkPipelineMultisampleStateCreateInfo> ToSafeMultisampleState(
    const VkPipelineMultisampleStateCreateInfo &cbs) {
    return std::make_unique<const vku::safe_VkPipelineMultisampleStateCreateInfo>(&cbs);
}
std::unique_ptr<const vku::safe_VkPipelineDepthStencilStateCreateInfo> ToSafeDepthStencilState(
    const vku::safe_VkPipelineDepthStencilStateCreateInfo &cbs) {
    // This is needlessly copied here. Might better to make this a plain pointer, with an optional "backing unique_ptr"
    return std::make_unique<const vku::safe_VkPipelineDepthStencilStateCreateInfo>(cbs);
}
std::unique_ptr<const vku::safe_VkPipelineDepthStencilStateCreateInfo> ToSafeDepthStencilState(
    const VkPipelineDepthStencilStateCreateInfo &cbs) {
    return std::make_unique<const vku::safe_VkPipelineDepthStencilStateCreateInfo>(&cbs);
}
std::unique_ptr<const vku::safe_VkPipelineShaderStageCreateInfo> ToShaderStageCI(
    const vku::safe_VkPipelineShaderStageCreateInfo &cbs) {
    // This is needlessly copied here. Might better to make this a plain pointer, with an optional "backing unique_ptr"
    return std::make_unique<const vku::safe_VkPipelineShaderStageCreateInfo>(cbs);
}
std::unique_ptr<const vku::safe_VkPipelineShaderStageCreateInfo> ToShaderStageCI(const VkPipelineShaderStageCreateInfo &cbs) {
    return std::make_unique<const vku::safe_VkPipelineShaderStageCreateInfo>(&cbs);
}

template <typename CreateInfo>
void SetFragmentShaderInfoPrivate(FragmentShaderState &fs_state, const ValidationStateTracker &state_data,
                                  const CreateInfo &create_info) {
    for (uint32_t i = 0; i < create_info.stageCount; ++i) {
        if (create_info.pStages[i].stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
            auto module_state = state_data.Get<vvl::ShaderModule>(create_info.pStages[i].module);
            if (!module_state) {
                // If module is null and there is a VkShaderModuleCreateInfo in the pNext chain of the stage info, then this
                // module is part of a library and the state must be created
                const auto shader_ci = vku::FindStructInPNextChain<VkShaderModuleCreateInfo>(create_info.pStages[i].pNext);
                if (shader_ci) {
                    // don't need to worry about GroupDecoration in GPL
                    auto spirv_module = std::make_shared<spirv::Module>(shader_ci->codeSize, shader_ci->pCode);
                    module_state = std::make_shared<vvl::ShaderModule>(VK_NULL_HANDLE, spirv_module, 0);
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
                // can be null if using VK_EXT_shader_module_identifier
                if (fs_state.fragment_shader->spirv) {
                    fs_state.fragment_entry_point = fs_state.fragment_shader->spirv->FindEntrypoint(
                        fs_state.fragment_shader_ci->pName, fs_state.fragment_shader_ci->stage);
                }
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
                                                const vku::safe_VkGraphicsPipelineCreateInfo &create_info) {
    SetFragmentShaderInfoPrivate(fs_state, state_data, create_info);
}

FragmentShaderState::FragmentShaderState(const vvl::Pipeline &p, const ValidationStateTracker &dev_data,
                                         std::shared_ptr<const vvl::RenderPass> rp, uint32_t subp, VkPipelineLayout layout)
    : PipelineSubState(p), rp_state(rp), subpass(subp), pipeline_layout(dev_data.Get<vvl::PipelineLayout>(layout)) {}

FragmentOutputState::FragmentOutputState(const vvl::Pipeline &p, std::shared_ptr<const vvl::RenderPass> rp, uint32_t sp)
    : PipelineSubState(p), rp_state(rp), subpass(sp) {}

// static
bool FragmentOutputState::IsBlendConstantsEnabled(const AttachmentStateVector &attachment_states) {
    bool result = false;
    for (const auto &attachment : attachment_states) {
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
bool FragmentOutputState::GetDualSourceBlending(const vku::safe_VkPipelineColorBlendStateCreateInfo *color_blend_state) {
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
