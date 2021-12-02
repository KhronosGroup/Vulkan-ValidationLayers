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
 * Author: Jeremy Gebben <jeremyg@lunarg.com>
 */
#include "pipeline_state.h"
#include "descriptor_sets.h"
#include "cmd_buffer_state.h"
#include "state_tracker.h"
#include "shader_module.h"

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
        set_layouts[i] = dev_data->GetShared<cvdescriptorset::DescriptorSetLayout>(pCreateInfo->pSetLayouts[i]);
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

static PIPELINE_STATE::VertexBindingVector GetVertexBindingDescriptions(const safe_VkGraphicsPipelineCreateInfo &create_info) {
    PIPELINE_STATE::VertexBindingVector result;
    if (create_info.pVertexInputState) {
        const auto vici = create_info.pVertexInputState;
        if (vici->vertexBindingDescriptionCount) {
            result.reserve(vici->vertexBindingDescriptionCount);
            std::copy(vici->pVertexBindingDescriptions, vici->pVertexBindingDescriptions + vici->vertexBindingDescriptionCount,
                      std::back_inserter(result));
        }
    }
    return result;
}

static PIPELINE_STATE::VertexBindingIndexMap GetVertexBindingMap(const PIPELINE_STATE::VertexBindingVector &bindings) {
    PIPELINE_STATE::VertexBindingIndexMap result;
    for (uint32_t i = 0; i < bindings.size(); i++) {
        result[bindings[i].binding] = i;
    }
    return result;
}

static PIPELINE_STATE::VertexAttrVector GetVertexAttributeDescriptions(const safe_VkGraphicsPipelineCreateInfo &create_info) {
    PIPELINE_STATE::VertexAttrVector result;
    if (create_info.pVertexInputState) {
        const auto vici = create_info.pVertexInputState;
        if (vici->vertexAttributeDescriptionCount) {
            result.reserve(vici->vertexAttributeDescriptionCount);
            std::copy(vici->pVertexAttributeDescriptions,
                      vici->pVertexAttributeDescriptions + vici->vertexAttributeDescriptionCount, std::back_inserter(result));
        }
    }
    return result;
}

static PIPELINE_STATE::VertexAttrAlignmentVector GetAttributeAlignments(const PIPELINE_STATE::VertexAttrVector &attributes) {
    PIPELINE_STATE::VertexAttrAlignmentVector result;
    result.reserve(attributes.size());
    for (const auto &attr : attributes) {
        VkDeviceSize vtx_attrib_req_alignment = FormatElementSize(attr.format);
        if (FormatElementIsTexel(attr.format)) {
            vtx_attrib_req_alignment = SafeDivision(vtx_attrib_req_alignment, FormatComponentCount(attr.format));
        }
        result.push_back(vtx_attrib_req_alignment);
    }
    return result;
}

static PIPELINE_STATE::AttachmentVector GetAttachments(const safe_VkGraphicsPipelineCreateInfo &create_info) {
    PIPELINE_STATE::AttachmentVector result;
    if (create_info.pColorBlendState) {
        const auto cbci = create_info.pColorBlendState;
        if (cbci->attachmentCount) {
            result.reserve(cbci->attachmentCount);
            std::copy(cbci->pAttachments, cbci->pAttachments + cbci->attachmentCount, std::back_inserter(result));
        }
    }
    return result;
}

static bool IsBlendConstantsEnabled(const PIPELINE_STATE::AttachmentVector &attachments) {
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

static bool IsSampleLocationEnabled(const safe_VkGraphicsPipelineCreateInfo &create_info) {
    bool result = false;
    if (create_info.pMultisampleState) {
        const auto *sample_location_state =
            LvlFindInChain<VkPipelineSampleLocationsStateCreateInfoEXT>(create_info.pMultisampleState->pNext);
        if (sample_location_state != nullptr) {
            result = (sample_location_state->sampleLocationsEnable != 0);
        }
    }
    return result;
}

static bool HasWriteableDescriptor(const std::vector<PipelineStageState::DescriptorUse> &descriptor_uses) {
    return std::any_of(descriptor_uses.begin(), descriptor_uses.end(),
                       [](const PipelineStageState::DescriptorUse &use) { return use.second.is_atomic_operation; });
}

static bool HasAtomicDescriptor(const std::vector<PipelineStageState::DescriptorUse> &descriptor_uses) {
    return std::any_of(descriptor_uses.begin(), descriptor_uses.end(),
                       [](const PipelineStageState::DescriptorUse &use) { return use.second.is_writable; });
}

static bool WrotePrimitiveShadingRate(VkShaderStageFlagBits stage_flag, spirv_inst_iter entrypoint,
                                      const SHADER_MODULE_STATE *module) {
    bool primitiverate_written = false;
    if (stage_flag == VK_SHADER_STAGE_VERTEX_BIT || stage_flag == VK_SHADER_STAGE_GEOMETRY_BIT ||
        stage_flag == VK_SHADER_STAGE_MESH_BIT_NV) {
        for (const auto &set : module->GetBuiltinDecorationList()) {
            auto insn = module->at(set.offset);
            if (set.builtin == spv::BuiltInPrimitiveShadingRateKHR) {
                primitiverate_written = module->IsBuiltInWritten(insn, entrypoint);
            }
            if (primitiverate_written) {
                break;
            }
        }
    }
    return primitiverate_written;
}

PipelineStageState::PipelineStageState(const VkPipelineShaderStageCreateInfo *stage,
                                       std::shared_ptr<const SHADER_MODULE_STATE> &module_)
    : module(module_),
      create_info(stage),
      stage_flag(stage->stage),
      entrypoint(module->FindEntrypoint(stage->pName, stage->stage)),
      accessible_ids(module->MarkAccessibleIds(entrypoint)),
      descriptor_uses(module->CollectInterfaceByDescriptorSlot(accessible_ids)),
      has_writable_descriptor(HasWriteableDescriptor(descriptor_uses)),
      has_atomic_descriptor(HasAtomicDescriptor(descriptor_uses)),
      wrote_primitive_shading_rate(WrotePrimitiveShadingRate(stage_flag, entrypoint, module.get())) {}

static PIPELINE_STATE::StageStateVec GetStageStates(const ValidationStateTracker *state_data,
                                                    const safe_VkPipelineShaderStageCreateInfo *stages, uint32_t stage_count) {
    PIPELINE_STATE::StageStateVec stage_states;
    stage_states.reserve(stage_count);
    // shader stages need to be recorded in pipeline order
    for (uint32_t stage_idx = 0; stage_idx < 32; ++stage_idx) {
        for (uint32_t i = 0; i < stage_count; i++) {
            if (stages[i].stage == (1 << stage_idx)) {
                auto module = state_data->GetShared<SHADER_MODULE_STATE>(stages[i].module);
                stage_states.emplace_back(stages[i].ptr(), module);
            }
        }
    }
    return stage_states;
}

static PIPELINE_STATE::ActiveSlotMap GetActiveSlots(const PIPELINE_STATE::StageStateVec &stage_states) {
    PIPELINE_STATE::ActiveSlotMap active_slots;
    for (const auto &stage : stage_states) {
        if (stage.entrypoint == stage.module->end()) {
            continue;
        }
        // Capture descriptor uses for the pipeline
        for (const auto &use : stage.descriptor_uses) {
            // While validating shaders capture which slots are used by the pipeline
            auto &entry = active_slots[use.first.set][use.first.binding];
            entry.is_writable |= use.second.is_writable;

            auto &reqs = entry.reqs;
            reqs |= stage.module->DescriptorTypeToReqs(use.second.type_id);
            if (use.second.is_atomic_operation) reqs |= DESCRIPTOR_REQ_VIEW_ATOMIC_OPERATION;
            if (use.second.is_sampler_implicitLod_dref_proj) reqs |= DESCRIPTOR_REQ_SAMPLER_IMPLICITLOD_DREF_PROJ;
            if (use.second.is_sampler_bias_offset) reqs |= DESCRIPTOR_REQ_SAMPLER_BIAS_OFFSET;

            if (use.second.samplers_used_by_image.size()) {
                if (use.second.samplers_used_by_image.size() > entry.samplers_used_by_image.size()) {
                    entry.samplers_used_by_image.resize(use.second.samplers_used_by_image.size());
                }
                uint32_t image_index = 0;
                for (const auto &samplers : use.second.samplers_used_by_image) {
                    for (const auto &sampler : samplers) {
                        entry.samplers_used_by_image[image_index].emplace(sampler);
                    }
                    ++image_index;
                }
            }
        }
    }
    return active_slots;
}

static uint32_t GetMaxActiveSlot(const PIPELINE_STATE::ActiveSlotMap &active_slots) {
    uint32_t max_active_slot = 0;
    for (const auto &entry : active_slots) {
        max_active_slot = std::max(max_active_slot, entry.first);
    }
    return max_active_slot;
}

static uint32_t GetActiveShaders(const VkPipelineShaderStageCreateInfo *stages, uint32_t stage_count) {
    uint32_t result = 0;
    for (uint32_t i = 0; i < stage_count; i++) {
        result |= stages[i].stage;
    }
    return result;
}

static layer_data::unordered_set<uint32_t> GetFSOutputLocations(const PIPELINE_STATE::StageStateVec &stage_states) {
    layer_data::unordered_set<uint32_t> result;
    for (const auto &stage : stage_states) {
        if (stage.entrypoint == stage.module->end()) {
            continue;
        }
        if (stage.stage_flag == VK_SHADER_STAGE_FRAGMENT_BIT) {
            result = stage.module->CollectWritableOutputLocationinFS(stage.entrypoint);
            break;
        }
    }
    return result;
}

static VkPrimitiveTopology GetTopologyAtRasterizer(const PIPELINE_STATE::StageStateVec &stage_states,
                                                   const safe_VkPipelineInputAssemblyStateCreateInfo *assembly_state) {
    VkPrimitiveTopology result = assembly_state ? assembly_state->topology : static_cast<VkPrimitiveTopology>(0);
    for (const auto &stage : stage_states) {
        if (stage.entrypoint == stage.module->end()) {
            continue;
        }
        auto stage_topo = stage.module->GetTopology(stage.entrypoint);
        if (stage_topo) {
            result = *stage_topo;
        }
    }
    return result;
}

PIPELINE_STATE::PIPELINE_STATE(const ValidationStateTracker *state_data, const VkGraphicsPipelineCreateInfo *pCreateInfo,
                               std::shared_ptr<const RENDER_PASS_STATE> &&rpstate,
                               std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout)
    : BASE_NODE(static_cast<VkPipeline>(VK_NULL_HANDLE), kVulkanObjectTypePipeline),
      create_info(pCreateInfo, rpstate),
      pipeline_layout(std::move(layout)),
      rp_state(rpstate),
      stage_state(GetStageStates(state_data, create_info.graphics.pStages, create_info.graphics.stageCount)),
      active_slots(GetActiveSlots(stage_state)),
      max_active_slot(GetMaxActiveSlot(active_slots)),
      fragmentShader_writable_output_location_list(GetFSOutputLocations(stage_state)),
      vertex_binding_descriptions_(GetVertexBindingDescriptions(create_info.graphics)),
      vertex_attribute_descriptions_(GetVertexAttributeDescriptions(create_info.graphics)),
      vertex_attribute_alignments_(GetAttributeAlignments(vertex_attribute_descriptions_)),
      vertex_binding_to_index_map_(GetVertexBindingMap(vertex_binding_descriptions_)),
      attachments(GetAttachments(create_info.graphics)),
      blend_constants_enabled(IsBlendConstantsEnabled(attachments)),
      sample_location_enabled(IsSampleLocationEnabled(create_info.graphics)),
      active_shaders(GetActiveShaders(pCreateInfo->pStages, pCreateInfo->stageCount)),
      topology_at_rasterizer(GetTopologyAtRasterizer(stage_state, create_info.graphics.pInputAssemblyState)) {}

PIPELINE_STATE::PIPELINE_STATE(const ValidationStateTracker *state_data, const VkComputePipelineCreateInfo *pCreateInfo,
                               std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout)
    : BASE_NODE(static_cast<VkPipeline>(VK_NULL_HANDLE), kVulkanObjectTypePipeline),
      create_info(pCreateInfo),
      pipeline_layout(std::move(layout)),
      stage_state(GetStageStates(state_data, &create_info.compute.stage, 1)),
      active_slots(GetActiveSlots(stage_state)),
      blend_constants_enabled(false),
      sample_location_enabled(false),
      active_shaders(GetActiveShaders(&pCreateInfo->stage, 1)),
      topology_at_rasterizer{} {
    assert(active_shaders == VK_SHADER_STAGE_COMPUTE_BIT);
}

template <typename CreateInfoStruct>
PIPELINE_STATE::PIPELINE_STATE(const ValidationStateTracker *state_data, const CreateInfoStruct *pCreateInfo,
                               std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout)
    : BASE_NODE(static_cast<VkPipeline>(VK_NULL_HANDLE), kVulkanObjectTypePipeline),
      create_info(pCreateInfo),
      pipeline_layout(std::move(layout)),
      stage_state(GetStageStates(state_data, create_info.raytracing.pStages, create_info.raytracing.stageCount)),
      active_slots(GetActiveSlots(stage_state)),
      blend_constants_enabled(false),
      sample_location_enabled(false),
      active_shaders(GetActiveShaders(pCreateInfo->pStages, pCreateInfo->stageCount)),
      topology_at_rasterizer{} {
    assert(0 == (active_shaders &
                 ~(VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
                   VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_INTERSECTION_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR)));
}

template PIPELINE_STATE::PIPELINE_STATE(const ValidationStateTracker *, const VkRayTracingPipelineCreateInfoNV *,
                                        std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&);
template PIPELINE_STATE::PIPELINE_STATE(const ValidationStateTracker *, const VkRayTracingPipelineCreateInfoKHR *,
                                        std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&);

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
