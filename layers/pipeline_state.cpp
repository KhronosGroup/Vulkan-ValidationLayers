/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (C) 2015-2021 Google Inc.
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
 *
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
 */
#include "pipeline_state.h"
#include "descriptor_sets.h"
#include "cmd_buffer_state.h"
#include "render_pass_state.h"
#include "state_tracker.h"

// Dictionary of canonical form of the pipeline set layout of descriptor set layouts
static PipelineLayoutSetLayoutsDict pipeline_layout_set_layouts_dict;

// Dictionary of canonical form of the "compatible for set" records
static PipelineLayoutCompatDict pipeline_layout_compat_dict;

static PushConstantRangesDict push_constant_ranges_dict;

size_t PipelineLayoutCompatDef::hash() const {
    hash_util::HashCombiner hc;
    // The set number is integral to the CompatDef's distinctiveness
    hc << set << push_constant_ranges.get();
    const auto &descriptor_set_layouts = *set_layouts_id.get();
    for (uint32_t i = 0; i <= set; i++) {
        hc << descriptor_set_layouts[i].get();
    }
    return hc.Value();
}

bool PipelineLayoutCompatDef::operator==(const PipelineLayoutCompatDef &other) const {
    if ((set != other.set) || (push_constant_ranges != other.push_constant_ranges)) {
        return false;
    }

    if (set_layouts_id == other.set_layouts_id) {
        // if it's the same set_layouts_id, then *any* subset will match
        return true;
    }

    // They aren't exactly the same PipelineLayoutSetLayouts, so we need to check if the required subsets match
    const auto &descriptor_set_layouts = *set_layouts_id.get();
    assert(set < descriptor_set_layouts.size());
    const auto &other_ds_layouts = *other.set_layouts_id.get();
    assert(set < other_ds_layouts.size());
    for (uint32_t i = 0; i <= set; i++) {
        if (descriptor_set_layouts[i] != other_ds_layouts[i]) {
            return false;
        }
    }
    return true;
}

static PipelineLayoutCompatId GetCanonicalId(const uint32_t set_index, const PushConstantRangesId pcr_id,
                                             const PipelineLayoutSetLayoutsId set_layouts_id) {
    return pipeline_layout_compat_dict.look_up(PipelineLayoutCompatDef(set_index, pcr_id, set_layouts_id));
}

// For repeatable sorting, not very useful for "memory in range" search
struct PushConstantRangeCompare {
    bool operator()(const VkPushConstantRange *lhs, const VkPushConstantRange *rhs) const {
        if (lhs->offset == rhs->offset) {
            if (lhs->size == rhs->size) {
                // The comparison is arbitrary, but avoids false aliasing by comparing all fields.
                return lhs->stageFlags < rhs->stageFlags;
            }
            // If the offsets are the same then sorting by the end of range is useful for validation
            return lhs->size < rhs->size;
        }
        return lhs->offset < rhs->offset;
    }
};

static PushConstantRangesId GetCanonicalId(const VkPipelineLayoutCreateInfo *info) {
    if (!info->pPushConstantRanges) {
        // Hand back the empty entry (creating as needed)...
        return push_constant_ranges_dict.look_up(PushConstantRanges());
    }

    // Sort the input ranges to ensure equivalent ranges map to the same id
    std::set<const VkPushConstantRange *, PushConstantRangeCompare> sorted;
    for (uint32_t i = 0; i < info->pushConstantRangeCount; i++) {
        sorted.insert(info->pPushConstantRanges + i);
    }

    PushConstantRanges ranges;
    ranges.reserve(sorted.size());
    for (const auto *range : sorted) {
        ranges.emplace_back(*range);
    }
    return push_constant_ranges_dict.look_up(std::move(ranges));
}

static PIPELINE_LAYOUT_STATE::SetLayoutVector GetSetLayouts(ValidationStateTracker *dev_data,
                                                            const VkPipelineLayoutCreateInfo *pCreateInfo) {
    PIPELINE_LAYOUT_STATE::SetLayoutVector set_layouts(pCreateInfo->setLayoutCount);

    for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; ++i) {
        set_layouts[i] = dev_data->GetDescriptorSetLayoutShared(pCreateInfo->pSetLayouts[i]);
    }
    return set_layouts;
}

static std::vector<PipelineLayoutCompatId> GetCompatForSet(const PIPELINE_LAYOUT_STATE::SetLayoutVector &set_layouts,
                                                           const PushConstantRangesId &push_constant_ranges) {
    PipelineLayoutSetLayoutsDef set_layout_ids(set_layouts.size());
    for (size_t i = 0; i < set_layouts.size(); i++) {
        set_layout_ids[i] = set_layouts[i]->GetLayoutId();
    }
    auto set_layouts_id = pipeline_layout_set_layouts_dict.look_up(set_layout_ids);

    std::vector<PipelineLayoutCompatId> compat_for_set;
    compat_for_set.reserve(set_layouts.size());

    for (uint32_t i = 0; i < set_layouts.size(); i++) {
        compat_for_set.emplace_back(GetCanonicalId(i, push_constant_ranges, set_layouts_id));
    }
    return compat_for_set;
}

PIPELINE_LAYOUT_STATE::PIPELINE_LAYOUT_STATE(ValidationStateTracker *dev_data, VkPipelineLayout l,
                                             const VkPipelineLayoutCreateInfo *pCreateInfo)
    : BASE_NODE(l, kVulkanObjectTypePipelineLayout),
      set_layouts(GetSetLayouts(dev_data, pCreateInfo)),
      push_constant_ranges(GetCanonicalId(pCreateInfo)),
      compat_for_set(GetCompatForSet(set_layouts, push_constant_ranges)) {}

void PIPELINE_STATE::initGraphicsPipeline(const ValidationStateTracker *state_data, const VkGraphicsPipelineCreateInfo *pCreateInfo,
                                          std::shared_ptr<const RENDER_PASS_STATE> &&rpstate) {
    reset();
    bool uses_color_attachment = false;
    bool uses_depthstencil_attachment = false;
    if (pCreateInfo->subpass < rpstate->createInfo.subpassCount) {
        const auto &subpass = rpstate->createInfo.pSubpasses[pCreateInfo->subpass];

        for (uint32_t i = 0; i < subpass.colorAttachmentCount; ++i) {
            if (subpass.pColorAttachments[i].attachment != VK_ATTACHMENT_UNUSED) {
                uses_color_attachment = true;
                break;
            }
        }

        if (subpass.pDepthStencilAttachment && subpass.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
            uses_depthstencil_attachment = true;
        }
    }
    graphicsPipelineCI.initialize(pCreateInfo, uses_color_attachment, uses_depthstencil_attachment);
    if (graphicsPipelineCI.pInputAssemblyState) {
        topology_at_rasterizer = graphicsPipelineCI.pInputAssemblyState->topology;
    }

    stage_state.resize(pCreateInfo->stageCount);
    // Graphics pipeline shader stages need to be recorded in order
    for (uint32_t stage_idx = 0; stage_idx < 5; ++stage_idx) {
        for (uint32_t i = 0; i < pCreateInfo->stageCount; i++) {
            const VkPipelineShaderStageCreateInfo &pssci = pCreateInfo->pStages[i];
            if (pssci.stage == (1 << stage_idx)) {
                this->duplicate_shaders |= this->active_shaders & pssci.stage;
                this->active_shaders |= pssci.stage;
                state_data->RecordPipelineShaderStage(&pssci, this, &stage_state[i]);
            }
        }
    }
    // Record non-graphics pipeline stages
    for (uint32_t i = 0; i < pCreateInfo->stageCount; i++) {
        const VkPipelineShaderStageCreateInfo &pssci = pCreateInfo->pStages[i];
        if ((pssci.stage & VK_SHADER_STAGE_ALL_GRAPHICS) == 0) {
            this->duplicate_shaders |= this->active_shaders & pssci.stage;
            this->active_shaders |= pssci.stage;
            state_data->RecordPipelineShaderStage(&pssci, this, &stage_state[i]);
        }
    }

    if (graphicsPipelineCI.pVertexInputState) {
        const auto vici = graphicsPipelineCI.pVertexInputState;
        if (vici->vertexBindingDescriptionCount) {
            this->vertex_binding_descriptions_ = std::vector<VkVertexInputBindingDescription>(
                vici->pVertexBindingDescriptions, vici->pVertexBindingDescriptions + vici->vertexBindingDescriptionCount);

            this->vertex_binding_to_index_map_.reserve(vici->vertexBindingDescriptionCount);
            for (uint32_t i = 0; i < vici->vertexBindingDescriptionCount; ++i) {
                this->vertex_binding_to_index_map_[vici->pVertexBindingDescriptions[i].binding] = i;
            }
        }
        if (vici->vertexAttributeDescriptionCount) {
            this->vertex_attribute_descriptions_ = std::vector<VkVertexInputAttributeDescription>(
                vici->pVertexAttributeDescriptions, vici->pVertexAttributeDescriptions + vici->vertexAttributeDescriptionCount);
            for (uint32_t i = 0; i < vici->vertexAttributeDescriptionCount; ++i) {
                const auto attribute_format = vici->pVertexAttributeDescriptions[i].format;
                VkDeviceSize vtx_attrib_req_alignment = FormatElementSize(attribute_format);
                if (FormatElementIsTexel(attribute_format)) {
                    vtx_attrib_req_alignment = SafeDivision(vtx_attrib_req_alignment, FormatChannelCount(attribute_format));
                }
                this->vertex_attribute_alignments_.push_back(vtx_attrib_req_alignment);
            }
        }
    }
    if (graphicsPipelineCI.pColorBlendState) {
        const auto cbci = graphicsPipelineCI.pColorBlendState;
        if (cbci->attachmentCount) {
            this->attachments =
                std::vector<VkPipelineColorBlendAttachmentState>(cbci->pAttachments, cbci->pAttachments + cbci->attachmentCount);
        }
    }
    rp_state = rpstate;
}

void PIPELINE_STATE::initComputePipeline(const ValidationStateTracker *state_data, const VkComputePipelineCreateInfo *pCreateInfo) {
    reset();
    computePipelineCI.initialize(pCreateInfo);
    switch (computePipelineCI.stage.stage) {
        case VK_SHADER_STAGE_COMPUTE_BIT:
            this->active_shaders |= VK_SHADER_STAGE_COMPUTE_BIT;
            stage_state.resize(1);
            state_data->RecordPipelineShaderStage(&pCreateInfo->stage, this, &stage_state[0]);
            break;
        default:
            // TODO : Flag error
            break;
    }
}

template <typename CreateInfo>
void PIPELINE_STATE::initRayTracingPipeline(const ValidationStateTracker *state_data, const CreateInfo *pCreateInfo) {
    reset();
    raytracingPipelineCI.initialize(pCreateInfo);

    stage_state.resize(pCreateInfo->stageCount);
    for (uint32_t stage_index = 0; stage_index < pCreateInfo->stageCount; stage_index++) {
        const auto &shader_stage = pCreateInfo->pStages[stage_index];
        switch (shader_stage.stage) {
            case VK_SHADER_STAGE_RAYGEN_BIT_NV:
                this->active_shaders |= VK_SHADER_STAGE_RAYGEN_BIT_NV;
                break;
            case VK_SHADER_STAGE_ANY_HIT_BIT_NV:
                this->active_shaders |= VK_SHADER_STAGE_ANY_HIT_BIT_NV;
                break;
            case VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV:
                this->active_shaders |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
                break;
            case VK_SHADER_STAGE_MISS_BIT_NV:
                this->active_shaders |= VK_SHADER_STAGE_MISS_BIT_NV;
                break;
            case VK_SHADER_STAGE_INTERSECTION_BIT_NV:
                this->active_shaders |= VK_SHADER_STAGE_INTERSECTION_BIT_NV;
                break;
            case VK_SHADER_STAGE_CALLABLE_BIT_NV:
                this->active_shaders |= VK_SHADER_STAGE_CALLABLE_BIT_NV;
                break;
            default:
                // TODO : Flag error
                break;
        }
        state_data->RecordPipelineShaderStage(&shader_stage, this, &stage_state[stage_index]);
    }
}

template void PIPELINE_STATE::initRayTracingPipeline(const ValidationStateTracker *state_data,
                                                     const VkRayTracingPipelineCreateInfoNV *pCreateInfo);
template void PIPELINE_STATE::initRayTracingPipeline(const ValidationStateTracker *state_data,
                                                     const VkRayTracingPipelineCreateInfoKHR *pCreateInfo);

void LAST_BOUND_STATE::UpdateSamplerDescriptorsUsedByImage() {
    if (!pipeline_state) return;
    if (per_set.empty()) return;

    for (auto &slot : pipeline_state->active_slots) {
        for (auto &req : slot.second) {
            for (auto &samplers : req.second.samplers_used_by_image) {
                for (auto &sampler : samplers) {
                    if (sampler.first.sampler_slot.set < per_set.size() &&
                        per_set[sampler.first.sampler_slot.set].bound_descriptor_set) {
                        sampler.second = per_set[sampler.first.sampler_slot.set].bound_descriptor_set->GetDescriptorFromBinding(
                            sampler.first.sampler_slot.binding, sampler.first.sampler_index);
                    }
                }
            }
        }
    }
}

void LAST_BOUND_STATE::UnbindAndResetPushDescriptorSet(CMD_BUFFER_STATE *cb_state, cvdescriptorset::DescriptorSet *ds) {
    if (push_descriptor_set) {
        for (auto &ps : per_set) {
            if (ps.bound_descriptor_set == push_descriptor_set.get()) {
                cb_state->RemoveChild(ps.bound_descriptor_set);
                ps.bound_descriptor_set = nullptr;
            }
        }
    }
    cb_state->AddChild(ds);
    push_descriptor_set.reset(ds);
}

void LAST_BOUND_STATE::Reset() {
    pipeline_state = nullptr;
    pipeline_layout = VK_NULL_HANDLE;
    if (push_descriptor_set) {
        push_descriptor_set->Reset();
    }
    push_descriptor_set = nullptr;
    per_set.clear();
}
