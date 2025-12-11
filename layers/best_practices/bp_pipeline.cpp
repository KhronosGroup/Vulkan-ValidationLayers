/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
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
#include "best_practices/bp_state.h"
#include "generated/spirv_grammar_helper.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/pipeline_state.h"
#include "chassis/chassis_modification_state.h"

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

bool BestPractices::ValidateMultisampledBlendingArm(const vvl::Pipeline& pipeline, const Location& create_info_loc) const {
    bool skip = false;

    const auto* color_blend_state = pipeline.ColorBlendState();
    const auto* ms_state = pipeline.MultisampleState();
    if (!color_blend_state || !ms_state || ms_state->rasterizationSamples == VK_SAMPLE_COUNT_1_BIT ||
        ms_state->sampleShadingEnable) {
        return skip;
    }

    auto rp_state = Get<vvl::RenderPass>(pipeline.GraphicsCreateInfo().renderPass);
    if (!rp_state) {
        return skip;
    }

    const auto& subpass = rp_state->create_info.pSubpasses[pipeline.Subpass()];

    // According to spec, pColorBlendState must be ignored if subpass does not have color attachments.
    uint32_t num_color_attachments = std::min(subpass.colorAttachmentCount, color_blend_state->attachmentCount);

    for (uint32_t j = 0; j < num_color_attachments; j++) {
        const auto& blend_att = color_blend_state->pAttachments[j];
        uint32_t att = subpass.pColorAttachments[j].attachment;

        if (att != VK_ATTACHMENT_UNUSED && blend_att.blendEnable && blend_att.colorWriteMask) {
            if (!FormatHasFullThroughputBlendingArm(rp_state->create_info.pAttachments[att].format)) {
                skip |= LogPerformanceWarning("BestPractices-Arm-vkCreatePipelines-multisampled-blending", device, create_info_loc,
                                              "%s Pipeline is multisampled and "
                                              "color attachment %" PRIu32
                                              " makes use "
                                              "of a format which cannot be blended at full throughput when using MSAA.",
                                              VendorSpecificTag(kBPVendorArm), j);
            }
        }
    }

    return skip;
}

void BestPractices::ManualPostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache,
                                                               uint32_t createInfoCount,
                                                               const VkComputePipelineCreateInfo* pCreateInfos,
                                                               const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                               const RecordObject& record_obj, PipelineStates& pipeline_states,
                                                               chassis::CreateComputePipelines& chassis_state) {
    if (record_obj.result != VK_SUCCESS) {
        return;
    }

    // AMD best practice
    pipeline_cache_ = pipelineCache;
}

bool BestPractices::ValidateCreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo& create_info, const vvl::Pipeline& pipeline,
                                                   const Location create_info_loc) const {
    bool skip = false;

    const auto* vertex_input = pipeline.InputState();
    if (!(pipeline.active_shaders & VK_SHADER_STAGE_MESH_BIT_EXT) && vertex_input) {
        uint32_t count = 0;
        for (uint32_t j = 0; j < vertex_input->vertexBindingDescriptionCount; j++) {
            if (vertex_input->pVertexBindingDescriptions[j].inputRate == VK_VERTEX_INPUT_RATE_INSTANCE) {
                count++;
            }
        }
        if (count > kMaxInstancedVertexBuffers) {
            skip |= LogPerformanceWarning("BestPractices-vkCreateGraphicsPipelines-too-many-instanced-vertex-buffers", device,
                                          create_info_loc,
                                          "The pipeline is using %" PRIu32 " instanced vertex buffers (current limit: %" PRIu32
                                          "), but this can be inefficient on the "
                                          "GPU. If using instanced vertex attributes prefer interleaving them in a single buffer.",
                                          count, kMaxInstancedVertexBuffers);
        }
    }

    const auto* raster_state = pipeline.RasterizationState();

    if (raster_state && raster_state->depthBiasEnable && raster_state->depthBiasConstantFactor == 0.0f &&
        raster_state->depthBiasSlopeFactor == 0.0f && VendorCheckEnabled(kBPVendorArm)) {
        skip |=
            LogPerformanceWarning("BestPractices-Arm-vkCreatePipelines-depthbias-zero", device, create_info_loc,
                                  "%s This vkCreateGraphicsPipelines call is created with depthBiasEnable set to true "
                                  "and both depthBiasConstantFactor and depthBiasSlopeFactor are set to 0. This can cause reduced "
                                  "efficiency during rasterization. Consider disabling depthBias or increasing either "
                                  "depthBiasConstantFactor or depthBiasSlopeFactor.",
                                  VendorSpecificTag(kBPVendorArm));
    }

    const auto* graphics_lib_info = vku::FindStructInPNextChain<VkGraphicsPipelineLibraryCreateInfoEXT>(create_info.pNext);
    if (create_info.renderPass == VK_NULL_HANDLE &&
        !vku::FindStructInPNextChain<VkPipelineRenderingCreateInfo>(create_info.pNext) &&
        (!graphics_lib_info ||
         (graphics_lib_info->flags & (VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
                                      VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT)) != 0)) {
        skip |= LogWarning(
            "BestPractices-Pipeline-NoRendering", device, create_info_loc,
            "renderPass is VK_NULL_HANDLE and pNext chain does not contain an instance of VkPipelineRenderingCreateInfo.");
    }

    if (VendorCheckEnabled(kBPVendorArm)) {
        skip |= ValidateMultisampledBlendingArm(pipeline, create_info_loc);
    }

    if (VendorCheckEnabled(kBPVendorAMD)) {
        const auto* ia_state = pipeline.InputAssemblyState();
        if (ia_state && ia_state->primitiveRestartEnable) {
            skip |= LogPerformanceWarning("BestPractices-AMD-CreatePipelines-AvoidPrimitiveRestart", device,
                                          create_info_loc.dot(Field::pInputAssemblyState).dot(Field::primitiveRestartEnable),
                                          "%s Use of primitive restart is not recommended", VendorSpecificTag(kBPVendorAMD));
        }

        // TODO: this might be too aggressive of a check
        if (create_info.pDynamicState && create_info.pDynamicState->dynamicStateCount > kDynamicStatesWarningLimitAMD) {
            skip |= LogPerformanceWarning("BestPractices-AMD-CreatePipelines-MinimizeNumDynamicStates", device, create_info_loc,
                                          "%s Dynamic States usage incurs a performance cost. Ensure that they are truly needed",
                                          VendorSpecificTag(kBPVendorAMD));
        }
    }

    for (uint32_t i = 0; i < pipeline.stage_states.size(); i++) {
        auto& stage_state = pipeline.stage_states[i];
        const VkShaderStageFlagBits stage = stage_state.GetStage();
        // Only validate the shader state once when added, not again when linked
        if ((stage & pipeline.linking_shaders) == 0) {
            skip |= ValidateShaderStage(stage_state, &pipeline, create_info_loc.dot(Field::pStages, i));
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                           const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                           const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                           const ErrorObject& error_obj, PipelineStates& pipeline_states,
                                                           chassis::CreateGraphicsPipelines& chassis_state) const {
    bool skip = false;

    if ((createInfoCount > 1) && (!pipelineCache)) {
        skip |=
            LogPerformanceWarning("BestPractices-vkCreateGraphicsPipelines-multiple-pipelines-no-cache", device, error_obj.location,
                                  "creating multiple pipelines (createInfoCount is %" PRIu32
                                  ") but is not using a "
                                  "pipeline cache, which may help with performance",
                                  createInfoCount);
    }

    for (uint32_t i = 0; i < createInfoCount; i++) {
        const auto* pipeline = pipeline_states[i].get();
        ASSERT_AND_CONTINUE(pipeline);
        skip |= ValidateCreateGraphicsPipeline(pCreateInfos[i], *pipeline, error_obj.location.dot(Field::pCreateInfos, i));
    }

    if (VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorNVIDIA)) {
        auto prev_pipeline = pipeline_cache_.load();
        if (pipelineCache && prev_pipeline && pipelineCache != prev_pipeline) {
            skip |= LogPerformanceWarning("BestPractices-vkCreatePipelines-multiple-pipelines-caches", device, error_obj.location,
                                          "%s %s A second pipeline cache is in use. "
                                          "Consider using only one pipeline cache to improve cache hit rate.",
                                          VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorNVIDIA));
        }
    }
    if (VendorCheckEnabled(kBPVendorAMD)) {
        const uint32_t pso_count = num_pso_.load();
        if (pso_count > kMaxRecommendedNumberOfPSOAMD) {
            skip |= LogPerformanceWarning("BestPractices-AMD-CreatePipelines-TooManyPipelines", device, error_obj.location,
                                          "%s Too many pipelines created (%" PRIu32 " but max recommended is %" PRIu32
                                          "), consider consolidation",
                                          VendorSpecificTag(kBPVendorAMD), pso_count, kMaxRecommendedNumberOfPSOAMD);
        }
    }

    return skip;
}

void BestPractices::ManualPostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                                const RecordObject& record_obj, PipelineStates& pipeline_states,
                                                                chassis::CreateGraphicsPipelines& chassis_state) {
    if (record_obj.result != VK_SUCCESS) {
        return;
    }
    // AMD best practice
    pipeline_cache_ = pipelineCache;
}

bool BestPractices::PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                          const VkComputePipelineCreateInfo* pCreateInfos,
                                                          const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                          const ErrorObject& error_obj, PipelineStates& pipeline_states,
                                                          chassis::CreateComputePipelines& chassis_state) const {
    bool skip = false;

    if ((createInfoCount > 1) && (!pipelineCache)) {
        skip |=
            LogPerformanceWarning("BestPractices-vkCreateComputePipelines-multiple-pipelines-no-cache", device, error_obj.location,
                                  "creating multiple pipelines (createInfoCount is %" PRIu32
                                  ") but is not using a pipeline cache, which may help with performance",
                                  createInfoCount);
    }

    if (VendorCheckEnabled(kBPVendorAMD)) {
        auto prev_pipeline = pipeline_cache_.load();
        if (pipelineCache && prev_pipeline && pipelineCache != prev_pipeline) {
            skip |= LogPerformanceWarning("BestPractices-vkCreateComputePipelines-multiple-cache", device, error_obj.location,
                                          "%s A second pipeline cache is in use. Consider using only one pipeline cache to "
                                          "improve cache hit rate",
                                          VendorSpecificTag(kBPVendorAMD));
        }
    }

    for (uint32_t i = 0; i < createInfoCount; i++) {
        const vvl::Pipeline* pipeline = pipeline_states[i].get();
        ASSERT_AND_CONTINUE(pipeline);

        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
        const Location stage_info = create_info_loc.dot(Field::stage);
        const auto& stage_state = pipeline->stage_states[0];
        skip |= ValidateShaderStage(stage_state, pipeline, stage_info);
    }

    return skip;
}

bool BestPractices::ValidateComputeShaderArm(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                             const Location& loc) const {
    bool skip = false;

    spirv::LocalSize local_size = module_state.FindLocalSize(entrypoint);
    if (local_size.x == 0) {
        return false;
    }

    const uint64_t thread_count = local_size.x * local_size.y * local_size.z;

    // Generate a priori warnings about work group sizes.
    if (thread_count > kMaxEfficientWorkGroupThreadCountArm) {
        skip |= LogPerformanceWarning(
            "BestPractices-Arm-vkCreateComputePipelines-compute-work-group-size", device, loc,
            "%s compute shader with work group dimensions (%s) (%" PRIu64
            " threads total), has more threads than advised in a single work group. It is advised to use work "
            "groups with less than %" PRIu32 " threads, especially when using barrier() or shared memory.",
            VendorSpecificTag(kBPVendorArm), local_size.ToString().c_str(), thread_count, kMaxEfficientWorkGroupThreadCountArm);
    }

    if (thread_count == 1 || ((local_size.x > 1) && (local_size.x & (kThreadGroupDispatchCountAlignmentArm - 1))) ||
        ((local_size.y > 1) && (local_size.y & (kThreadGroupDispatchCountAlignmentArm - 1))) ||
        ((local_size.z > 1) && (local_size.z & (kThreadGroupDispatchCountAlignmentArm - 1)))) {
        skip |= LogPerformanceWarning("BestPractices-Arm-vkCreateComputePipelines-compute-thread-group-alignment", device, loc,
                                      "%s compute shader with work group dimensions (%s) is not aligned to %" PRIu32
                                      " threads. On Arm Mali architectures, not aligning work group sizes to %" PRIu32
                                      " may leave threads idle on the shader core.",
                                      VendorSpecificTag(kBPVendorArm), local_size.ToString().c_str(),
                                      kThreadGroupDispatchCountAlignmentArm, kThreadGroupDispatchCountAlignmentArm);
    }

    uint32_t dimensions = 0;
    if (local_size.x > 1) dimensions++;
    if (local_size.y > 1) dimensions++;
    if (local_size.z > 1) dimensions++;

    if (dimensions == 1) {
        // If we're accessing images, we almost certainly want to have a 2D workgroup for cache reasons.
        // There are some false positives here. We could simply have a shader that does this within a 1D grid,
        // or we may have a linearly tiled image, but these cases are quite unlikely in practice.
        for (const auto& variable : entrypoint.resource_interface_variables) {
            if (variable.IsImage() && variable.info.image_dim != spv::Dim1D && variable.info.image_dim != spv::DimBuffer) {
                LogPerformanceWarning("BestPractices-Arm-vkCreateComputePipelines-compute-spatial-locality", device, loc,
                                      "%s compute shader has work group dimensions (%s), which "
                                      "suggests a 1D dispatch, but the shader is accessing 2D or 3D images. The shader may be "
                                      "exhibiting poor spatial locality with respect to one or more shader resources.",
                                      VendorSpecificTag(kBPVendorArm), local_size.ToString().c_str());
                break;  // only need to report once
            }
        }
    }

    return skip;
}

bool BestPractices::ValidateComputeShaderAmd(const spirv::Module& module_state, const spirv::EntryPoint& entrypoint,
                                             const Location& loc) const {
    bool skip = false;

    spirv::LocalSize local_size = module_state.FindLocalSize(entrypoint);
    if (local_size.x == 0) {
        return false;
    }

    const uint64_t thread_count = local_size.x * local_size.y * local_size.z;

    const bool multiple_64 = ((thread_count % 64) == 0);

    if (!multiple_64) {
        skip |= LogPerformanceWarning("BestPractices-AMD-LocalWorkgroup-Multiple64", device, loc,
                                      "%s compute shader with work group dimensions (%s), workgroup size (%" PRIu64
                                      "), is not a multiple of 64. Make the workgroup size a multiple of 64 to obtain best "
                                      "performance across all AMD GPU generations.",
                                      VendorSpecificTag(kBPVendorAMD), local_size.ToString().c_str(), thread_count);
    }

    return skip;
}

void BestPractices::PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                         const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                         const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                         const RecordObject& record_obj) {
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
            auto descriptor_set_layout_state = Get<vvl::DescriptorSetLayout>(pCreateInfo->pSetLayouts[i]);
            if (!descriptor_set_layout_state) continue;
            pipeline_size += descriptor_set_layout_state->GetDynamicDescriptorCount() * descriptor_size;
        }

        for (uint32_t i = 0; i < pCreateInfo->pushConstantRangeCount; i++) {
            pipeline_size += pCreateInfo->pPushConstantRanges[i].size / 4;
        }

        if (pipeline_size > kPipelineLayoutSizeWarningLimitAMD) {
            skip |= LogPerformanceWarning("BestPractices-AMD-CreatePipelinesLayout-KeepLayoutSmall", device, error_obj.location,
                                          "%s pipeline layout size is too large. Prefer smaller pipeline layouts."
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
            auto descriptor_set_layout_state = Get<vvl::DescriptorSetLayout>(pCreateInfo->pSetLayouts[i]);
            if (!descriptor_set_layout_state) continue;
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
                "BestPractices-NVIDIA-CreatePipelineLayout-SeparateSampler", device, error_obj.location,
                "%s Consider using combined image samplers instead of separate samplers for marginally better performance.",
                VendorSpecificTag(kBPVendorNVIDIA));
        }

        if (fast_space_usage > kPipelineLayoutFastDescriptorSpaceNVIDIA) {
            skip |= LogPerformanceWarning(
                "BestPractices-NVIDIA-CreatePipelineLayout-LargePipelineLayout", device, error_obj.location,
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

    if (VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorNVIDIA)) {
        if (IsPipelineUsedInFrame(pipeline)) {
            skip |= LogPerformanceWarning(
                "BestPractices-Pipeline-SortAndBind", commandBuffer, error_obj.location,
                "%s %s Pipeline %s was bound twice in the frame. "
                "Keep pipeline state changes to a minimum, for example, by sorting draw calls by pipeline.",
                VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorNVIDIA), FormatHandle(pipeline).c_str());
        }
    }
    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);
        auto& sub_state = bp_state::SubState(*cb_state);
        const auto& tgm = sub_state.nv.tess_geometry_mesh;
        if (tgm.num_switches >= kNumBindPipelineTessGeometryMeshSwitchesThresholdNVIDIA && !tgm.threshold_signaled) {
            LogPerformanceWarning("BestPractices-NVIDIA-BindPipeline-SwitchTessGeometryMesh", commandBuffer, error_obj.location,
                                  "%s Avoid switching between pipelines with and without tessellation, geometry, task, "
                                  "and/or mesh shaders. Group draw calls using these shader stages together.",
                                  VendorSpecificTag(kBPVendorNVIDIA));
            // Do not set 'skip' so the number of switches gets properly counted after the message.
        }
    }

    return skip;
}

// Currently only compute uses this, but this is designed to match the way CoreChecks funnels all the SPIR-V related checks
bool BestPractices::ValidateShaderStage(const ShaderStageState& stage_state, const vvl::Pipeline* pipeline,
                                        const Location& loc) const {
    bool skip = false;

    if ((pipeline && pipeline->uses_shader_module_id) || !stage_state.spirv_state) {
        return skip;  // these edge cases should be validated already
    }

    const spirv::Module& module_state = *stage_state.spirv_state.get();
    if (!module_state.valid_spirv) {
        return skip;  // checked elsewhere
    } else if (!stage_state.entrypoint) {
        return skip;  // checked elsewhere
    }

    const spirv::EntryPoint& entrypoint = *stage_state.entrypoint;

    if (entrypoint.stage == VK_SHADER_STAGE_COMPUTE_BIT) {
        if (VendorCheckEnabled(kBPVendorArm)) {
            skip |= ValidateComputeShaderArm(module_state, entrypoint, loc);
        }
        if (VendorCheckEnabled(kBPVendorAMD)) {
            skip |= ValidateComputeShaderAmd(module_state, entrypoint, loc);
        }

        if (IsExtEnabled(extensions.vk_khr_maintenance4)) {
            if (module_state.static_data_.has_builtin_workgroup_size) {
                skip |= LogWarning("BestPractices-SpirvDeprecated_WorkgroupSize", device, loc,
                                   "is using the SPIR-V Workgroup built-in which SPIR-V 1.6 deprecated. When using "
                                   "VK_KHR_maintenance4 or Vulkan 1.3+, the new SPIR-V LocalSizeId execution mode should be used "
                                   "instead. This can be done by recompiling your shader and targeting Vulkan 1.3+.");
            }
        }
    }

    if (entrypoint.stage == VK_SHADER_STAGE_TASK_BIT_EXT || entrypoint.stage == VK_SHADER_STAGE_MESH_BIT_EXT) {
        spirv::LocalSize local_size = module_state.FindLocalSize(entrypoint);
        if (local_size.x == 0) {
            return skip;
        }
        bool is_task =
            entrypoint.execution_model == spv::ExecutionModelTaskEXT || entrypoint.execution_model == spv::ExecutionModelTaskNV;

        // Assume core checks caught any overflow
        uint64_t invocations =
            static_cast<uint64_t>(local_size.x) * static_cast<uint64_t>(local_size.y) * static_cast<uint64_t>(local_size.z);
        const uint32_t preferred_size = is_task ? phys_dev_ext_props.mesh_shader_props_ext.maxPreferredTaskWorkGroupInvocations
                                                : phys_dev_ext_props.mesh_shader_props_ext.maxPreferredMeshWorkGroupInvocations;
        if (invocations > preferred_size) {
            skip |= LogPerformanceWarning(
                "BestPractices-Mesh-MaxPreferredWorkGroupInvocations", module_state.handle(), loc,
                "SPIR-V (%s) total invocation size of %" PRIu64 " (%s) is more than %s (%" PRIu32 ").",
                string_SpvExecutionModel(entrypoint.execution_model), invocations, local_size.ToString().c_str(),
                is_task ? "maxPreferredTaskWorkGroupInvocations" : "maxPreferredMeshWorkGroupInvocations", preferred_size);
        }
    }

    return skip;
}