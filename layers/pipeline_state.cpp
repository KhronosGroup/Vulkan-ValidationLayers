/* Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (C) 2015-2022 Google Inc.
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

static bool HasWriteableDescriptor(const std::vector<PipelineStageState::DescriptorUse> &descriptor_uses) {
    return std::any_of(descriptor_uses.begin(), descriptor_uses.end(),
                       [](const PipelineStageState::DescriptorUse &use) { return use.second.is_writable; });
}

static bool HasAtomicDescriptor(const std::vector<PipelineStageState::DescriptorUse> &descriptor_uses) {
    return std::any_of(descriptor_uses.begin(), descriptor_uses.end(),
                       [](const PipelineStageState::DescriptorUse &use) { return use.second.is_atomic_operation; });
}

static bool WrotePrimitiveShadingRate(VkShaderStageFlagBits stage_flag, spirv_inst_iter entrypoint,
                                      const SHADER_MODULE_STATE *module_state) {
    bool primitiverate_written = false;
    if (stage_flag == VK_SHADER_STAGE_VERTEX_BIT || stage_flag == VK_SHADER_STAGE_GEOMETRY_BIT ||
        stage_flag == VK_SHADER_STAGE_MESH_BIT_NV) {
        for (const auto &set : module_state->GetBuiltinDecorationList()) {
            auto insn = module_state->at(set.offset);
            if (set.builtin == spv::BuiltInPrimitiveShadingRateKHR) {
                primitiverate_written = module_state->IsBuiltInWritten(insn, entrypoint);
            }
            if (primitiverate_written) {
                break;
            }
        }
    }
    return primitiverate_written;
}

PipelineStageState::PipelineStageState(const VkPipelineShaderStageCreateInfo *stage,
                                       std::shared_ptr<const SHADER_MODULE_STATE> &module_state)
    : module_state(module_state),
      create_info(stage),
      stage_flag(stage->stage),
      entrypoint(module_state->FindEntrypoint(stage->pName, stage->stage)),
      accessible_ids(module_state->MarkAccessibleIds(entrypoint)),
      descriptor_uses(module_state->CollectInterfaceByDescriptorSlot(accessible_ids)),
      has_writable_descriptor(HasWriteableDescriptor(descriptor_uses)),
      has_atomic_descriptor(HasAtomicDescriptor(descriptor_uses)),
      wrote_primitive_shading_rate(WrotePrimitiveShadingRate(stage_flag, entrypoint, module_state.get())) {}

static PIPELINE_STATE::StageStateVec GetStageStates(const ValidationStateTracker *state_data,
                                                    const safe_VkPipelineShaderStageCreateInfo *stages, uint32_t stage_count) {
    PIPELINE_STATE::StageStateVec stage_states;
    stage_states.reserve(stage_count);
    // shader stages need to be recorded in pipeline order
    for (uint32_t stage_idx = 0; stage_idx < 32; ++stage_idx) {
        for (uint32_t i = 0; i < stage_count; i++) {
            if (stages[i].stage == (1 << stage_idx)) {
                auto module_state = state_data->Get<SHADER_MODULE_STATE>(stages[i].module);
                stage_states.emplace_back(stages[i].ptr(), module_state);
            }
        }
    }
    return stage_states;
}

static PIPELINE_STATE::ActiveSlotMap GetActiveSlots(const PIPELINE_STATE::StageStateVec &stage_states) {
    PIPELINE_STATE::ActiveSlotMap active_slots;
    for (const auto &stage : stage_states) {
        if (stage.entrypoint == stage.module_state->end()) {
            continue;
        }
        // Capture descriptor uses for the pipeline
        for (const auto &use : stage.descriptor_uses) {
            // While validating shaders capture which slots are used by the pipeline
            auto &entry = active_slots[use.first.set][use.first.binding];
            entry.is_writable |= use.second.is_writable;

            auto &reqs = entry.reqs;
            reqs |= stage.module_state->DescriptorTypeToReqs(use.second.type_id);
            if (use.second.is_atomic_operation) reqs |= DESCRIPTOR_REQ_VIEW_ATOMIC_OPERATION;
            if (use.second.is_sampler_implicitLod_dref_proj) reqs |= DESCRIPTOR_REQ_SAMPLER_IMPLICITLOD_DREF_PROJ;
            if (use.second.is_sampler_bias_offset) reqs |= DESCRIPTOR_REQ_SAMPLER_BIAS_OFFSET;
            if (use.second.is_read_without_format) reqs |= DESCRIPTOR_REQ_IMAGE_READ_WITHOUT_FORMAT;
            if (use.second.is_write_without_format) reqs |= DESCRIPTOR_REQ_IMAGE_WRITE_WITHOUT_FORMAT;
            if (use.second.is_dref_operation) reqs |= DESCRIPTOR_REQ_IMAGE_DREF;

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
        if (stage.entrypoint == stage.module_state->end()) {
            continue;
        }
        if (stage.stage_flag == VK_SHADER_STAGE_FRAGMENT_BIT) {
            result = stage.module_state->CollectWritableOutputLocationinFS(stage.entrypoint);
            break;
        }
    }
    return result;
}

static VkPrimitiveTopology GetTopologyAtRasterizer(const PIPELINE_STATE::StageStateVec &stage_states,
                                                   const safe_VkPipelineInputAssemblyStateCreateInfo *assembly_state) {
    VkPrimitiveTopology result = assembly_state ? assembly_state->topology : static_cast<VkPrimitiveTopology>(0);
    for (const auto &stage : stage_states) {
        if (stage.entrypoint == stage.module_state->end()) {
            continue;
        }
        auto stage_topo = stage.module_state->GetTopology(stage.entrypoint);
        if (stage_topo) {
            result = *stage_topo;
        }
    }
    return result;
}

// static
std::shared_ptr<VertexInputState> PIPELINE_STATE::CreateVertexInputState(const ValidationStateTracker &state,
                                                                         const safe_VkGraphicsPipelineCreateInfo &create_info) {
    const auto lib_type = GetGraphicsLibType(create_info);
    if (lib_type & VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT) {  // Vertex input graphics library
        return std::make_shared<VertexInputState>(create_info);
    }

    const auto link_info = LvlFindInChain<VkPipelineLibraryCreateInfoKHR>(create_info.pNext);
    if (link_info) {
        auto ss = GetLibSubState<VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT>(state, *link_info);
        if (ss) {
            return ss;
        }
    }

    if (lib_type == static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0)) {  // Not a graphics library
        return std::make_shared<VertexInputState>(create_info);
    }

    // We shouldn't get here...
    return {};
}

// static
std::shared_ptr<PreRasterState> PIPELINE_STATE::CreatePreRasterState(const ValidationStateTracker &state,
                                                                     const safe_VkGraphicsPipelineCreateInfo &create_info) {
    const auto lib_type = GetGraphicsLibType(create_info);
    if (lib_type & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT) {  // Pre-raster graphics library
        return std::make_shared<PreRasterState>(state, create_info);
    }

    const auto link_info = LvlFindInChain<VkPipelineLibraryCreateInfoKHR>(create_info.pNext);
    if (link_info) {
        auto ss = GetLibSubState<VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT>(state, *link_info);
        if (ss) {
            return ss;
        }
    }

    if (lib_type == static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0)) {  // Not a graphics library
        return std::make_shared<PreRasterState>(state, create_info);
    }

    // We shouldn't get here...
    return {};
}

// static
std::shared_ptr<FragmentShaderState> PIPELINE_STATE::CreateFragmentShaderState(
    const ValidationStateTracker &state, const VkGraphicsPipelineCreateInfo &create_info,
    const safe_VkGraphicsPipelineCreateInfo &safe_create_info) {
    const auto lib_type = GetGraphicsLibType(create_info);
    if (lib_type & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) {  // Fragment shader graphics library
        return std::make_shared<FragmentShaderState>(state, create_info);
    }

    const auto link_info = LvlFindInChain<VkPipelineLibraryCreateInfoKHR>(create_info.pNext);
    if (link_info) {
        auto ss = GetLibSubState<VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT>(state, *link_info);
        if (ss) {
            return ss;
        }
    }

    if (lib_type == static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0)) {  // Not a graphics library
        return std::make_shared<FragmentShaderState>(state, safe_create_info);
    }

    // We shouldn't get here...
    return {};
}

// static
// Pointers that should be ignored have been set to null in safe_create_info, but if this is a graphics library we need the "raw"
// create_info.
std::shared_ptr<FragmentOutputState> PIPELINE_STATE::CreateFragmentOutputState(
    const ValidationStateTracker &state, const VkGraphicsPipelineCreateInfo &create_info,
    const safe_VkGraphicsPipelineCreateInfo &safe_create_info) {
    const auto lib_type = GetGraphicsLibType(create_info);
    if (lib_type & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT) {  // Fragment output graphics library
        return std::make_shared<FragmentOutputState>(state, create_info);
    }

    const auto link_info = LvlFindInChain<VkPipelineLibraryCreateInfoKHR>(create_info.pNext);
    if (link_info) {
        auto ss = GetLibSubState<VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT>(state, *link_info);
        if (ss) {
            return ss;
        }
    }

    if (lib_type == static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0)) {  // Not a graphics library
        return std::make_shared<FragmentOutputState>(state, safe_create_info);
    }

    // We shouldn't get here...
    return {};
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

PIPELINE_STATE::PIPELINE_STATE(const ValidationStateTracker *state_data, const VkRayTracingPipelineCreateInfoNV *pCreateInfo,
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

PIPELINE_STATE::PIPELINE_STATE(const ValidationStateTracker *state_data, const VkRayTracingPipelineCreateInfoKHR *pCreateInfo,
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

void LAST_BOUND_STATE::UnbindAndResetPushDescriptorSet(CMD_BUFFER_STATE *cb_state,
                                                       std::shared_ptr<cvdescriptorset::DescriptorSet> &&ds) {
    if (push_descriptor_set) {
        for (auto &ps : per_set) {
            if (ps.bound_descriptor_set == push_descriptor_set) {
                cb_state->RemoveChild(ps.bound_descriptor_set);
                ps.bound_descriptor_set.reset();
            }
        }
    }
    cb_state->AddChild(ds);
    push_descriptor_set = std::move(ds);
}

void LAST_BOUND_STATE::Reset() {
    pipeline_state = nullptr;
    pipeline_layout = VK_NULL_HANDLE;
    if (push_descriptor_set) {
        push_descriptor_set->Destroy();
    }
    push_descriptor_set.reset();
    per_set.clear();
}
