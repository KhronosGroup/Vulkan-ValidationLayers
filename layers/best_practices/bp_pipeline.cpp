/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include "best_practices/best_practices_validation.h"
#include "best_practices/best_practices_error_enums.h"

static inline bool FormatHasFullThroughputBlendingArm(VkFormat format) {
    switch (format) {
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_R16G16B16_SFLOAT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_R32G32B32_SFLOAT:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return false;

        default:
            return true;
    }
}

bool BestPractices::ValidateMultisampledBlendingArm(uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                    const Location& create_info_loc) const {
    bool skip = false;

    for (uint32_t i = 0; i < createInfoCount; i++) {
        auto create_info = &pCreateInfos[i];

        if (!create_info->pColorBlendState || !create_info->pMultisampleState ||
            create_info->pMultisampleState->rasterizationSamples == VK_SAMPLE_COUNT_1_BIT ||
            create_info->pMultisampleState->sampleShadingEnable) {
            return skip;
        }

        auto rp_state = Get<RENDER_PASS_STATE>(create_info->renderPass);
        const auto& subpass = rp_state->createInfo.pSubpasses[create_info->subpass];

        // According to spec, pColorBlendState must be ignored if subpass does not have color attachments.
        uint32_t num_color_attachments = std::min(subpass.colorAttachmentCount, create_info->pColorBlendState->attachmentCount);

        for (uint32_t j = 0; j < num_color_attachments; j++) {
            const auto& blend_att = create_info->pColorBlendState->pAttachments[j];
            uint32_t att = subpass.pColorAttachments[j].attachment;

            if (att != VK_ATTACHMENT_UNUSED && blend_att.blendEnable && blend_att.colorWriteMask) {
                if (!FormatHasFullThroughputBlendingArm(rp_state->createInfo.pAttachments[att].format)) {
                    skip |= LogPerformanceWarning(kVUID_BestPractices_CreatePipelines_MultisampledBlending, device, create_info_loc,
                                                  "%s Pipeline is multisampled and "
                                                  "color attachment #%u makes use "
                                                  "of a format which cannot be blended at full throughput when using MSAA.",
                                                  VendorSpecificTag(kBPVendorArm), j);
                }
            }
        }
    }

    return skip;
}

void BestPractices::ManualPostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                               uint32_t createInfoCount,
                                                               const VkComputePipelineCreateInfo* pCreateInfos,
                                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                               const RecordObject& record_obj, void* pipe_state) {
    // AMD best practice
    pipeline_cache_ = pipelineCache;
}

bool BestPractices::PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                           const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                           const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                           const ErrorObject& error_obj, void* cgpl_state_data) const {
    bool skip = StateTracker::PreCallValidateCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos,
                                                                     pAllocator, pPipelines, error_obj, cgpl_state_data);
    if (skip) {
        return skip;
    }
    create_graphics_pipeline_api_state* cgpl_state = reinterpret_cast<create_graphics_pipeline_api_state*>(cgpl_state_data);

    if ((createInfoCount > 1) && (!pipelineCache)) {
        skip |= LogPerformanceWarning(
            kVUID_BestPractices_CreatePipelines_MultiplePipelines, device, error_obj.location,
            "Performance Warning: This vkCreateGraphicsPipelines call is creating multiple pipelines but is not using a "
            "pipeline cache, which may help with performance");
    }

    for (uint32_t i = 0; i < createInfoCount; i++) {
        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
        const auto& create_info = pCreateInfos[i];
        const auto& pipeline = *cgpl_state->pipe_state[i].get();

        if (!(pipeline.active_shaders & VK_SHADER_STAGE_MESH_BIT_EXT) && create_info.pVertexInputState) {
            const auto& vertex_input = *create_info.pVertexInputState;
            uint32_t count = 0;
            for (uint32_t j = 0; j < vertex_input.vertexBindingDescriptionCount; j++) {
                if (vertex_input.pVertexBindingDescriptions[j].inputRate == VK_VERTEX_INPUT_RATE_INSTANCE) {
                    count++;
                }
            }
            if (count > kMaxInstancedVertexBuffers) {
                skip |= LogPerformanceWarning(
                    kVUID_BestPractices_CreatePipelines_TooManyInstancedVertexBuffers, device, create_info_loc,
                    "The pipeline is using %u instanced vertex buffers (current limit: %u), but this can be inefficient on the "
                    "GPU. If using instanced vertex attributes prefer interleaving them in a single buffer.",
                    count, kMaxInstancedVertexBuffers);
            }
        }

        if ((pCreateInfos[i].pRasterizationState) && (pCreateInfos[i].pRasterizationState->depthBiasEnable) &&
            (pCreateInfos[i].pRasterizationState->depthBiasConstantFactor == 0.0f) &&
            (pCreateInfos[i].pRasterizationState->depthBiasSlopeFactor == 0.0f) && VendorCheckEnabled(kBPVendorArm)) {
            skip |= LogPerformanceWarning(
                kVUID_BestPractices_CreatePipelines_DepthBias_Zero, device, create_info_loc,
                "%s Performance Warning: This vkCreateGraphicsPipelines call is created with depthBiasEnable set to true "
                "and both depthBiasConstantFactor and depthBiasSlopeFactor are set to 0. This can cause reduced "
                "efficiency during rasterization. Consider disabling depthBias or increasing either "
                "depthBiasConstantFactor or depthBiasSlopeFactor.",
                VendorSpecificTag(kBPVendorArm));
        }

        skip |= VendorCheckEnabled(kBPVendorArm) && ValidateMultisampledBlendingArm(createInfoCount, pCreateInfos, create_info_loc);

        if (pCreateInfos[i].renderPass == VK_NULL_HANDLE &&
            !vku::FindStructInPNextChain<VkPipelineRenderingCreateInfoKHR>(pCreateInfos[i].pNext)) {
            skip |= LogWarning(kVUID_BestPractices_Pipeline_NoRendering, device, create_info_loc,
                               "renderPass is VK_NULL_HANDLE and pNext chain does not contain VkPipelineRenderingCreateInfoKHR.");
        }

        if (VendorCheckEnabled(kBPVendorAMD)) {
            if (pCreateInfos[i].pInputAssemblyState && pCreateInfos[i].pInputAssemblyState->primitiveRestartEnable) {
                skip |= LogPerformanceWarning(kVUID_BestPractices_CreatePipelines_AvoidPrimitiveRestart, device, create_info_loc,
                                              "%s Performance warning: Use of primitive restart is not recommended",
                                              VendorSpecificTag(kBPVendorAMD));
            }

            // TODO: this might be too aggressive of a check
            if (pCreateInfos[i].pDynamicState && pCreateInfos[i].pDynamicState->dynamicStateCount > kDynamicStatesWarningLimitAMD) {
                skip |= LogPerformanceWarning(
                    kVUID_BestPractices_CreatePipelines_MinimizeNumDynamicStates, device, create_info_loc,
                    "%s Performance warning: Dynamic States usage incurs a performance cost. Ensure that they are truly needed",
                    VendorSpecificTag(kBPVendorAMD));
            }
        }
    }
    if (VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorNVIDIA)) {
        auto prev_pipeline = pipeline_cache_.load();
        if (pipelineCache && prev_pipeline && pipelineCache != prev_pipeline) {
            skip |= LogPerformanceWarning(kVUID_BestPractices_CreatePipelines_MultiplePipelineCaches, device, error_obj.location,
                                          "%s %s Performance Warning: A second pipeline cache is in use. "
                                          "Consider using only one pipeline cache to improve cache hit rate.",
                                          VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorNVIDIA));
        }
    }
    if (VendorCheckEnabled(kBPVendorAMD)) {
        if (num_pso_ > kMaxRecommendedNumberOfPSOAMD) {
            skip |= LogPerformanceWarning(kVUID_BestPractices_CreatePipelines_TooManyPipelines, device, error_obj.location,
                                          "%s Performance warning: Too many pipelines created, consider consolidation",
                                          VendorSpecificTag(kBPVendorAMD));
        }
    }

    return skip;
}

static std::vector<bp_state::AttachmentInfo> GetAttachmentAccess(bp_state::Pipeline& pipe_state) {
    std::vector<bp_state::AttachmentInfo> result;
    auto rp = pipe_state.RenderPassState();
    if (!rp || rp->UsesDynamicRendering()) {
        return result;
    }
    auto& create_info = pipe_state.GetCreateInfo<VkGraphicsPipelineCreateInfo>();
    const auto& subpass = rp->createInfo.pSubpasses[create_info.subpass];

    // NOTE: see PIPELINE_LAYOUT and safe_VkGraphicsPipelineCreateInfo constructors. pColorBlendState and pDepthStencilState
    // are only non-null if they are enabled.
    if (create_info.pColorBlendState && !(pipe_state.ignore_color_attachments)) {
        // According to spec, pColorBlendState must be ignored if subpass does not have color attachments.
        uint32_t num_color_attachments = std::min(subpass.colorAttachmentCount, create_info.pColorBlendState->attachmentCount);
        for (uint32_t j = 0; j < num_color_attachments; j++) {
            if (create_info.pColorBlendState->pAttachments[j].colorWriteMask != 0) {
                uint32_t attachment = subpass.pColorAttachments[j].attachment;
                if (attachment != VK_ATTACHMENT_UNUSED) {
                    result.push_back({attachment, VK_IMAGE_ASPECT_COLOR_BIT});
                }
            }
        }
    }

    if (create_info.pDepthStencilState &&
        (create_info.pDepthStencilState->depthTestEnable || create_info.pDepthStencilState->depthBoundsTestEnable ||
         create_info.pDepthStencilState->stencilTestEnable)) {
        uint32_t attachment = subpass.pDepthStencilAttachment ? subpass.pDepthStencilAttachment->attachment : VK_ATTACHMENT_UNUSED;
        if (attachment != VK_ATTACHMENT_UNUSED) {
            VkImageAspectFlags aspects = 0;
            if (create_info.pDepthStencilState->depthTestEnable || create_info.pDepthStencilState->depthBoundsTestEnable) {
                aspects |= VK_IMAGE_ASPECT_DEPTH_BIT;
            }
            if (create_info.pDepthStencilState->stencilTestEnable) {
                aspects |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
            result.push_back({attachment, aspects});
        }
    }
    return result;
}

bp_state::Pipeline::Pipeline(const ValidationStateTracker* state_data, const VkGraphicsPipelineCreateInfo* pCreateInfo,
                             std::shared_ptr<const RENDER_PASS_STATE>&& rpstate,
                             std::shared_ptr<const PIPELINE_LAYOUT_STATE>&& layout, CreateShaderModuleStates* csm_states)
    : PIPELINE_STATE(state_data, pCreateInfo, std::move(rpstate), std::move(layout), csm_states),
      access_framebuffer_attachments(GetAttachmentAccess(*this)) {}

std::shared_ptr<PIPELINE_STATE> BestPractices::CreateGraphicsPipelineState(const VkGraphicsPipelineCreateInfo* pCreateInfo,
                                                                           std::shared_ptr<const RENDER_PASS_STATE>&& render_pass,
                                                                           std::shared_ptr<const PIPELINE_LAYOUT_STATE>&& layout,
                                                                           CreateShaderModuleStates* csm_states) const {
    return std::static_pointer_cast<PIPELINE_STATE>(
        std::make_shared<bp_state::Pipeline>(this, pCreateInfo, std::move(render_pass), std::move(layout), csm_states));
}

void BestPractices::ManualPostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                                const RecordObject& record_obj, void* cgpl_state_data) {
    // AMD best practice
    pipeline_cache_ = pipelineCache;
}

bool BestPractices::PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                          const VkComputePipelineCreateInfo* pCreateInfos,
                                                          const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                          const ErrorObject& error_obj, void* ccpl_state_data) const {
    bool skip = StateTracker::PreCallValidateCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos,
                                                                    pAllocator, pPipelines, error_obj, ccpl_state_data);

    if ((createInfoCount > 1) && (!pipelineCache)) {
        skip |= LogPerformanceWarning(
            kVUID_BestPractices_CreatePipelines_MultiplePipelines, device, error_obj.location,
            "Performance Warning: This vkCreateComputePipelines call is creating multiple pipelines but is not using a "
            "pipeline cache, which may help with performance");
    }

    if (VendorCheckEnabled(kBPVendorAMD)) {
        auto prev_pipeline = pipeline_cache_.load();
        if (pipelineCache && prev_pipeline && pipelineCache != prev_pipeline) {
            skip |= LogPerformanceWarning(
                kVUID_BestPractices_CreatePipelines_MultiplePipelines, device, error_obj.location,
                "%s Performance Warning: A second pipeline cache is in use. Consider using only one pipeline cache to "
                "improve cache hit rate",
                VendorSpecificTag(kBPVendorAMD));
        }
    }

    for (uint32_t i = 0; i < createInfoCount; i++) {
        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
        const VkComputePipelineCreateInfo& createInfo = pCreateInfos[i];
        if (VendorCheckEnabled(kBPVendorArm)) {
            skip |= ValidateCreateComputePipelineArm(createInfo, create_info_loc);
        }

        if (VendorCheckEnabled(kBPVendorAMD)) {
            skip |= ValidateCreateComputePipelineAmd(createInfo, create_info_loc);
        }

        if (IsExtEnabled(device_extensions.vk_khr_maintenance4)) {
            auto module_state = Get<SHADER_MODULE_STATE>(createInfo.stage.module);
            if (module_state &&
                module_state->spirv->static_data_.has_builtin_workgroup_size) {  // No module if creating from module identifier
                skip |= LogWarning(kVUID_BestPractices_SpirvDeprecated_WorkgroupSize, device, create_info_loc,
                                   "is using the Workgroup built-in which SPIR-V 1.6 deprecated. The VK_KHR_maintenance4 "
                                   "extension exposes a new LocalSizeId execution mode that should be used instead.");
            }
        }
    }

    return skip;
}

bool BestPractices::ValidateCreateComputePipelineArm(const VkComputePipelineCreateInfo& createInfo,
                                                     const Location& create_info_loc) const {
    bool skip = false;
    auto module_state = Get<SHADER_MODULE_STATE>(createInfo.stage.module);
    if (!module_state || !module_state->spirv) {
        return false;  // No module if creating from module identifier
    }

    // Generate warnings about work group sizes based on active resources.
    auto entrypoint = module_state->spirv->FindEntrypoint(createInfo.stage.pName, createInfo.stage.stage);
    if (!entrypoint) return false;

    uint32_t x = {}, y = {}, z = {};
    if (!module_state->spirv->FindLocalSize(*entrypoint, x, y, z)) {
        return false;
    }

    const uint32_t thread_count = x * y * z;

    // Generate a priori warnings about work group sizes.
    if (thread_count > kMaxEfficientWorkGroupThreadCountArm) {
        skip |= LogPerformanceWarning(
            kVUID_BestPractices_CreateComputePipelines_ComputeWorkGroupSize, device, create_info_loc,
            "%s compute shader with work group dimensions (%u, %u, "
            "%u) (%u threads total), has more threads than advised in a single work group. It is advised to use work "
            "groups with less than %u threads, especially when using barrier() or shared memory.",
            VendorSpecificTag(kBPVendorArm), x, y, z, thread_count, kMaxEfficientWorkGroupThreadCountArm);
    }

    if (thread_count == 1 || ((x > 1) && (x & (kThreadGroupDispatchCountAlignmentArm - 1))) ||
        ((y > 1) && (y & (kThreadGroupDispatchCountAlignmentArm - 1))) ||
        ((z > 1) && (z & (kThreadGroupDispatchCountAlignmentArm - 1)))) {
        skip |= LogPerformanceWarning(
            kVUID_BestPractices_CreateComputePipelines_ComputeThreadGroupAlignment, device, create_info_loc,
            "%s compute shader with work group dimensions (%u, "
            "%u, %u) is not aligned to %u "
            "threads. On Arm Mali architectures, not aligning work group sizes to %u may "
            "leave threads idle on the shader "
            "core.",
            VendorSpecificTag(kBPVendorArm), x, y, z, kThreadGroupDispatchCountAlignmentArm, kThreadGroupDispatchCountAlignmentArm);
    }

    unsigned dimensions = 0;
    if (x > 1) dimensions++;
    if (y > 1) dimensions++;
    if (z > 1) dimensions++;
    // Here the dimension will really depend on the dispatch grid, but assume it's 1D.
    dimensions = std::max(dimensions, 1u);

    // If we're accessing images, we almost certainly want to have a 2D workgroup for cache reasons.
    // There are some false positives here. We could simply have a shader that does this within a 1D grid,
    // or we may have a linearly tiled image, but these cases are quite unlikely in practice.
    bool accesses_2d = false;
    for (const auto& variable : entrypoint->resource_interface_variables) {
        if (variable.image_dim != spv::Dim1D && variable.image_dim != spv::DimBuffer) {
            accesses_2d = true;
            break;
        }
    }

    if (accesses_2d && dimensions < 2) {
        LogPerformanceWarning(kVUID_BestPractices_CreateComputePipelines_ComputeSpatialLocality, device, create_info_loc,
                              "%s compute shader has work group dimensions (%u, %u, %u), which "
                              "suggests a 1D dispatch, but the shader is accessing 2D or 3D images. The shader may be "
                              "exhibiting poor spatial locality with respect to one or more shader resources.",
                              VendorSpecificTag(kBPVendorArm), x, y, z);
    }

    return skip;
}

bool BestPractices::ValidateCreateComputePipelineAmd(const VkComputePipelineCreateInfo& createInfo,
                                                     const Location& create_info_loc) const {
    bool skip = false;
    auto module_state = Get<SHADER_MODULE_STATE>(createInfo.stage.module);
    if (!module_state || !module_state->spirv) {
        return false;
    }
    auto entrypoint = module_state->spirv->FindEntrypoint(createInfo.stage.pName, createInfo.stage.stage);
    if (!entrypoint) {
        return false;
    }

    uint32_t x = {}, y = {}, z = {};
    if (!module_state->spirv->FindLocalSize(*entrypoint, x, y, z)) {
        return false;
    }

    const uint32_t thread_count = x * y * z;

    const bool multiple_64 = ((thread_count % 64) == 0);

    if (!multiple_64) {
        skip |= LogPerformanceWarning(kVUID_BestPractices_LocalWorkgroup_Multiple64, device, create_info_loc,
                                      "%s compute shader with work group dimensions (%" PRIu32 ", %" PRIu32 ", %" PRIu32
                                      "), workgroup size (%" PRIu32
                                      "), is not a multiple of 64. Make the workgroup size a multiple of 64 to obtain best "
                                      "performance across all AMD GPU generations.",
                                      VendorSpecificTag(kBPVendorAMD), x, y, z, thread_count);
    }

    return skip;
}

void BestPractices::PreCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                 VkPipeline pipeline) {
    StateTracker::PreCallRecordCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);

    auto pipeline_info = Get<PIPELINE_STATE>(pipeline);
    auto cb = GetWrite<bp_state::CommandBuffer>(commandBuffer);

    assert(pipeline_info);
    assert(cb);

    if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS && VendorCheckEnabled(kBPVendorNVIDIA)) {
        using TessGeometryMeshState = bp_state::CommandBufferStateNV::TessGeometryMesh::State;
        auto& tgm = cb->nv.tess_geometry_mesh;

        // Make sure the message is only signaled once per command buffer
        tgm.threshold_signaled = tgm.num_switches >= kNumBindPipelineTessGeometryMeshSwitchesThresholdNVIDIA;

        // Track pipeline switches with tessellation, geometry, and/or mesh shaders enabled, and disabled
        auto tgm_stages = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT |
                          VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
        auto new_tgm_state =
            (pipeline_info->active_shaders & tgm_stages) != 0 ? TessGeometryMeshState::Enabled : TessGeometryMeshState::Disabled;
        if (tgm.state != new_tgm_state && tgm.state != TessGeometryMeshState::Unknown) {
            tgm.num_switches++;
        }
        tgm.state = new_tgm_state;

        // Track depthTestEnable and depthCompareOp
        auto& pipeline_create_info = pipeline_info->GetCreateInfo<VkGraphicsPipelineCreateInfo>();
        auto depth_stencil_state = pipeline_create_info.pDepthStencilState;
        auto dynamic_state = pipeline_create_info.pDynamicState;
        if (depth_stencil_state && dynamic_state) {
            auto dynamic_state_begin = dynamic_state->pDynamicStates;
            auto dynamic_state_end = dynamic_state->pDynamicStates + dynamic_state->dynamicStateCount;

            const bool dynamic_depth_test_enable =
                std::find(dynamic_state_begin, dynamic_state_end, VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE) != dynamic_state_end;
            const bool dynamic_depth_func =
                std::find(dynamic_state_begin, dynamic_state_end, VK_DYNAMIC_STATE_DEPTH_COMPARE_OP) != dynamic_state_end;

            if (!dynamic_depth_test_enable) {
                RecordSetDepthTestState(*cb, cb->nv.depth_compare_op, depth_stencil_state->depthTestEnable != VK_FALSE);
            }
            if (!dynamic_depth_func) {
                RecordSetDepthTestState(*cb, depth_stencil_state->depthCompareOp, cb->nv.depth_test_enable);
            }
        }
    }
}

void BestPractices::PostCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                  VkPipeline pipeline, const RecordObject& record_obj) {
    StateTracker::PostCallRecordCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline, record_obj);

    // AMD best practice
    PipelineUsedInFrame(pipeline);

    if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        auto pipeline_state = Get<bp_state::Pipeline>(pipeline);
        // check for depth/blend state tracking
        if (pipeline_state) {
            auto cb_node = GetWrite<bp_state::CommandBuffer>(commandBuffer);
            assert(cb_node);
            auto& render_pass_state = cb_node->render_pass_state;

            render_pass_state.nextDrawTouchesAttachments = pipeline_state->access_framebuffer_attachments;
            render_pass_state.drawTouchAttachments = true;

            const auto* blend_state = pipeline_state->ColorBlendState();
            const auto* stencil_state = pipeline_state->DepthStencilState();

            if (blend_state && !(pipeline_state->ignore_color_attachments)) {
                // assume the pipeline is depth-only unless any of the attachments have color writes enabled
                render_pass_state.depthOnly = true;
                for (size_t i = 0; i < blend_state->attachmentCount; i++) {
                    if (blend_state->pAttachments[i].colorWriteMask != 0) {
                        render_pass_state.depthOnly = false;
                    }
                }
            }

            // check for depth value usage
            render_pass_state.depthEqualComparison = false;

            if (stencil_state && stencil_state->depthTestEnable) {
                switch (stencil_state->depthCompareOp) {
                    case VK_COMPARE_OP_EQUAL:
                    case VK_COMPARE_OP_GREATER_OR_EQUAL:
                    case VK_COMPARE_OP_LESS_OR_EQUAL:
                        render_pass_state.depthEqualComparison = true;
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

void BestPractices::PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                         const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                         const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                         void* cgpl_state) {
    ValidationStateTracker::PreCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                                 pPipelines);
    // AMD best practice
    num_pso_ += createInfoCount;
}

bool BestPractices::PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout,
                                                        const ErrorObject& error_obj) const {
    bool skip = false;
    if (VendorCheckEnabled(kBPVendorAMD)) {
        uint32_t descriptor_size = enabled_features.robustBufferAccess ? 4 : 2;
        // Descriptor sets cost 1 DWORD each.
        // Dynamic buffers cost 2 DWORDs each when robust buffer access is OFF.
        // Dynamic buffers cost 4 DWORDs each when robust buffer access is ON.
        // Push constants cost 1 DWORD per 4 bytes in the Push constant range.
        uint32_t pipeline_size = pCreateInfo->setLayoutCount;  // in DWORDS
        for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; i++) {
            auto descriptor_set_layout_state = Get<cvdescriptorset::DescriptorSetLayout>(pCreateInfo->pSetLayouts[i]);
            pipeline_size += descriptor_set_layout_state->GetDynamicDescriptorCount() * descriptor_size;
        }

        for (uint32_t i = 0; i < pCreateInfo->pushConstantRangeCount; i++) {
            pipeline_size += pCreateInfo->pPushConstantRanges[i].size / 4;
        }

        if (pipeline_size > kPipelineLayoutSizeWarningLimitAMD) {
            skip |=
                LogPerformanceWarning(kVUID_BestPractices_CreatePipelinesLayout_KeepLayoutSmall, device, error_obj.location,
                                      "%s Performance warning: pipeline layout size is too large. Prefer smaller pipeline layouts."
                                      "Descriptor sets cost 1 DWORD each. "
                                      "Dynamic buffers cost 2 DWORDs each when robust buffer access is OFF. "
                                      "Dynamic buffers cost 4 DWORDs each when robust buffer access is ON. "
                                      "Push constants cost 1 DWORD per 4 bytes in the Push constant range. ",
                                      VendorSpecificTag(kBPVendorAMD));
        }
    }

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        bool has_separate_sampler = false;
        size_t fast_space_usage = 0;

        for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; ++i) {
            auto descriptor_set_layout_state = Get<cvdescriptorset::DescriptorSetLayout>(pCreateInfo->pSetLayouts[i]);
            for (const auto& binding : descriptor_set_layout_state->GetBindings()) {
                if (binding.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER) {
                    has_separate_sampler = true;
                }

                if ((descriptor_set_layout_state->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT) ==
                    0U) {
                    size_t descriptor_type_size = 0;

                    switch (binding.descriptorType) {
                        case VK_DESCRIPTOR_TYPE_SAMPLER:
                        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                            descriptor_type_size = 4;
                            break;
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
                        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
                            descriptor_type_size = 8;
                            break;
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                        case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
                            descriptor_type_size = 16;
                            break;
                        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
                            descriptor_type_size = 1;
                            break;
                        default:
                            // Unknown type.
                            break;
                    }

                    size_t descriptor_size = descriptor_type_size * binding.descriptorCount;
                    fast_space_usage += descriptor_size;
                }
            }
        }

        if (has_separate_sampler) {
            skip |= LogPerformanceWarning(
                kVUID_BestPractices_CreatePipelineLayout_SeparateSampler, device, error_obj.location,
                "%s Consider using combined image samplers instead of separate samplers for marginally better performance.",
                VendorSpecificTag(kBPVendorNVIDIA));
        }

        if (fast_space_usage > kPipelineLayoutFastDescriptorSpaceNVIDIA) {
            skip |= LogPerformanceWarning(
                kVUID_BestPractices_CreatePipelinesLayout_LargePipelineLayout, device, error_obj.location,
                "%s Pipeline layout size is too large, prefer using pipeline-specific descriptor set layouts. "
                "Aim for consuming less than %" PRIu32
                " bytes to allow fast reads for all non-bindless descriptors. "
                "Samplers, textures, texel buffers, and combined image samplers consume 4 bytes each. "
                "Uniform buffers and acceleration structures consume 8 bytes. "
                "Storage buffers consume 16 bytes. "
                "Push constants do not consume space.",
                VendorSpecificTag(kBPVendorNVIDIA), kPipelineLayoutFastDescriptorSpaceNVIDIA);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                   VkPipeline pipeline, const ErrorObject& error_obj) const {
    bool skip = false;

    auto cb = Get<bp_state::CommandBuffer>(commandBuffer);

    if (VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorNVIDIA)) {
        if (IsPipelineUsedInFrame(pipeline)) {
            skip |= LogPerformanceWarning(
                kVUID_BestPractices_Pipeline_SortAndBind, commandBuffer, error_obj.location,
                "%s %s Performance warning: Pipeline %s was bound twice in the frame. "
                "Keep pipeline state changes to a minimum, for example, by sorting draw calls by pipeline.",
                VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorNVIDIA), FormatHandle(pipeline).c_str());
        }
    }
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        const auto& tgm = cb->nv.tess_geometry_mesh;
        if (tgm.num_switches >= kNumBindPipelineTessGeometryMeshSwitchesThresholdNVIDIA && !tgm.threshold_signaled) {
            LogPerformanceWarning(kVUID_BestPractices_BindPipeline_SwitchTessGeometryMesh, commandBuffer, error_obj.location,
                                  "%s Avoid switching between pipelines with and without tessellation, geometry, task, "
                                  "and/or mesh shaders. Group draw calls using these shader stages together.",
                                  VendorSpecificTag(kBPVendorNVIDIA));
            // Do not set 'skip' so the number of switches gets properly counted after the message.
        }
    }

    return skip;
}
