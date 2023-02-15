/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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

#include <string>
#include <vector>

#include "vk_enum_string_helper.h"
#include "chassis.h"
#include "core_checks/core_validation.h"
#include "enum_flag_bits.h"

bool CoreChecks::ValidatePipelineLibraryFlags(const VkGraphicsPipelineLibraryFlagsEXT lib_flags,
                                              const VkPipelineLibraryCreateInfoKHR &link_info,
                                              const VkPipelineRenderingCreateInfo *rendering_struct, uint32_t pipe_index,
                                              int lib_index, const char *vuid) const {
    const bool current_pipeline = lib_index == -1;
    bool skip = false;

    VkGraphicsPipelineLibraryFlagsEXT flags = VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
                                              VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
                                              VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT;

    const uint32_t flags_count = GetBitSetCount(lib_flags & (VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
                                                             VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
                                                             VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT));
    if (flags_count >= 1 && flags_count <= 2) {
        // We start iterating at the index after lib_index to avoid duplicating checks, because the caller will iterate the same
        // loop
        for (int i = lib_index + 1; i < static_cast<int>(link_info.libraryCount); ++i) {
            const auto lib = Get<PIPELINE_STATE>(link_info.pLibraries[i]);
            const auto lib_rendering_struct = lib->GetPipelineRenderingCreateInfo();
            const bool other_flag = (lib->graphics_lib_type & flags) && (lib->graphics_lib_type & ~lib_flags);
            if (other_flag) {
                if (current_pipeline) {
                    if (lib->GetCreateInfo<VkGraphicsPipelineCreateInfo>().renderPass != VK_NULL_HANDLE) {
                        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderpass-06625",
                                         "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                         "] renderPass is VK_NULL_HANDLE and includes "
                                         "VkGraphicsPipelineLibraryCreateInfoEXT::flags (%s), but pLibraries[%" PRIu32
                                         "] includes VkGraphicsPipelineLibraryCreateInfoEXT::flags (%s) and "
                                         "render pass is not VK_NULL_HANDLE.",
                                         pipe_index, string_VkGraphicsPipelineLibraryFlagsEXT(lib_flags).c_str(), i,
                                         string_VkGraphicsPipelineLibraryFlagsEXT(lib->graphics_lib_type).c_str());
                    }
                }
                uint32_t view_mask = rendering_struct ? rendering_struct->viewMask : 0;
                uint32_t lib_view_mask = lib_rendering_struct ? lib_rendering_struct->viewMask : 0;
                if (view_mask != lib_view_mask) {
                    std::stringstream msg;
                    if (!current_pipeline) {
                        msg << "pLibraries[" << lib_index << "]";
                    }
                    skip |= LogError(device, vuid,
                                     "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                     "] %s includes VkGraphicsPipelineLibraryCreateInfoEXT::flags (%s) and "
                                     "VkPipelineRenderingCreateInfo::viewMask (%" PRIu32 "), but pLibraries[%" PRIu32
                                     "] includes VkGraphicsPipelineLibraryCreateInfoEXT::flags (%s) and "
                                     "VkPipelineRenderingCreateInfo::viewMask (%" PRIu32 ")",
                                     pipe_index, msg.str().c_str(), string_VkGraphicsPipelineLibraryFlagsEXT(lib_flags).c_str(),
                                     view_mask, i, string_VkGraphicsPipelineLibraryFlagsEXT(lib->graphics_lib_type).c_str(),
                                     lib_view_mask);
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidatePipeline(std::vector<std::shared_ptr<PIPELINE_STATE>> const &pipelines, uint32_t pipe_index) const {
    bool skip = false;
    safe_VkSubpassDescription2 *subpass_desc = nullptr;
    const auto &pipeline = *pipelines[pipe_index].get();

    // If create derivative bit is set, check that we've specified a base
    // pipeline correctly, and that the base pipeline was created to allow
    // derivatives.
    if (pipeline.GetPipelineCreateFlags() & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
        std::shared_ptr<const PIPELINE_STATE> base_pipeline;
        const VkPipeline base_handle = pipeline.BasePipeline<VkGraphicsPipelineCreateInfo>();
        const int32_t base_index = pipeline.BasePipelineIndex<VkGraphicsPipelineCreateInfo>();
        if (!((base_handle != VK_NULL_HANDLE) ^ (base_index != -1))) {
            // TODO: This check is a superset of VUID-VkGraphicsPipelineCreateInfo-flags-00724 and
            // TODO: VUID-VkGraphicsPipelineCreateInfo-flags-00725
            skip |= LogError(device, kVUID_Core_DrawState_InvalidPipelineCreateState,
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                             "]: exactly one of base pipeline index and handle must be specified",
                             pipeline.create_index);
        } else if (base_index != -1) {
            if (static_cast<uint32_t>(base_index) >= pipeline.create_index) {
                skip |= LogError(base_handle, "VUID-vkCreateGraphicsPipelines-flags-00720",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "]: base pipeline must occur earlier in array than derivative pipeline.",
                                 pipeline.create_index);
            } else {
                base_pipeline = pipelines[base_index];
            }
        } else if (base_handle != VK_NULL_HANDLE) {
            base_pipeline = Get<PIPELINE_STATE>(base_handle);
        }

        if (base_pipeline && !(base_pipeline->GetPipelineCreateFlags() & VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT)) {
            skip |= LogError(base_pipeline->pipeline(), "VUID-vkCreateGraphicsPipelines-flags-00721",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32 "]: base pipeline does not allow derivatives.",
                             pipeline.create_index);
        }
    }

    // Check for portability errors
    if (IsExtEnabled(device_extensions.vk_khr_portability_subset)) {
        if ((VK_FALSE == enabled_features.portability_subset_features.triangleFans) &&
            (VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN == pipeline.topology_at_rasterizer)) {
            skip |= LogError(device, "VUID-VkPipelineInputAssemblyStateCreateInfo-triangleFans-04452",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                             "] (portability error): VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN is not supported",
                             pipeline.create_index);
        }

        // Validate vertex inputs
        for (const auto &desc : pipeline.vertex_input_state->binding_descriptions) {
            const auto min_alignment = phys_dev_ext_props.portability_props.minVertexInputBindingStrideAlignment;
            if ((desc.stride < min_alignment) || (min_alignment == 0) || ((desc.stride % min_alignment) != 0)) {
                skip |=
                    LogError(device, "VUID-VkVertexInputBindingDescription-stride-04456",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                             "] (portability error): Vertex input stride must be at least as large as and a "
                             "multiple of VkPhysicalDevicePortabilitySubsetPropertiesKHR::minVertexInputBindingStrideAlignment.",
                             pipeline.create_index);
            }
        }

        // Validate vertex attributes
        if (VK_FALSE == enabled_features.portability_subset_features.vertexAttributeAccessBeyondStride) {
            for (const auto &attrib : pipeline.vertex_input_state->vertex_attribute_descriptions) {
                const auto vertex_binding_map_it = pipeline.vertex_input_state->binding_to_index_map.find(attrib.binding);
                if (vertex_binding_map_it != pipeline.vertex_input_state->binding_to_index_map.cend()) {
                    const auto &desc = pipeline.vertex_input_state->binding_descriptions[vertex_binding_map_it->second];
                    if ((attrib.offset + FormatElementSize(attrib.format)) > desc.stride) {
                        skip |= LogError(device, "VUID-VkVertexInputAttributeDescription-vertexAttributeAccessBeyondStride-04457",
                                         "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                         "] (portability error): (attribute.offset + "
                                         "sizeof(vertex_description.format)) is larger than the vertex stride",
                                         pipeline.create_index);
                    }
                }
            }
        }

        // Validate polygon mode
        auto raster_state_ci = pipeline.RasterizationState();
        if ((VK_FALSE == enabled_features.portability_subset_features.pointPolygons) && raster_state_ci &&
            (VK_FALSE == raster_state_ci->rasterizerDiscardEnable) && (VK_POLYGON_MODE_POINT == raster_state_ci->polygonMode)) {
            skip |= LogError(device, "VUID-VkPipelineRasterizationStateCreateInfo-pointPolygons-04458",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                             "] (portability error): point polygons are not supported",
                             pipeline.create_index);
        }
    }

    const auto &rp_state = pipeline.RenderPassState();
    if (pipeline.IsRenderPassStateRequired()) {
        if (!rp_state) {
            const char *vuid = nullptr;
            if (!IsExtEnabled(device_extensions.vk_khr_dynamic_rendering)) {
                if (api_version >= VK_API_VERSION_1_3) {
                    vuid = "VUID-VkGraphicsPipelineCreateInfo-renderPass-06575";
                } else {
                    vuid = "VUID-VkGraphicsPipelineCreateInfo-renderPass-06574";
                }
            } else if (!enabled_features.core13.dynamicRendering) {
                vuid = "VUID-VkGraphicsPipelineCreateInfo-dynamicRendering-06576";
            }
            if (vuid) {
                skip |= LogError(device, vuid,
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "] requires a valid renderPass, but one was not provided",
                                 pipeline.create_index);
            }
        }
    }

    const auto subpass = pipeline.Subpass();
    if (rp_state && !rp_state->UsesDynamicRendering()) {
        // Ensure the subpass index is valid. If not, then ValidateGraphicsPipelineShaderState
        // produces nonsense errors that confuse users. Other layers should already
        // emit errors for renderpass being invalid.
        subpass_desc = &rp_state->createInfo.pSubpasses[subpass];
        if (subpass >= rp_state->createInfo.subpassCount) {
            skip |= LogError(rp_state->renderPass(), "VUID-VkGraphicsPipelineCreateInfo-renderPass-06046",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                             "] State: Subpass index %u is out of range for this renderpass (0..%u).",
                             pipeline.create_index, subpass, rp_state->createInfo.subpassCount - 1);
            subpass_desc = nullptr;
        }
    }

    const auto ds_state = pipeline.DepthStencilState();
    if (ds_state) {
        if ((((ds_state->flags & VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_ARM) !=
              0) ||
             ((ds_state->flags & VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_ARM) !=
              0)) &&
            (!rp_state || rp_state->renderPass() == VK_NULL_HANDLE)) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-06483",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                             "].pDepthStencilState[%s] contains "
                             "VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_ARM or"
                             " VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_ARM, "
                             "renderpass must"
                             "not be VK_NULL_HANDLE.",
                             pipeline.create_index, string_VkPipelineDepthStencilStateCreateFlags(ds_state->flags).c_str());
        }
    }

    if (rp_state && rp_state->renderPass() != VK_NULL_HANDLE &&
        IsExtEnabled(device_extensions.vk_ext_multisampled_render_to_single_sampled) &&
        !pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) {
        const auto msrtss_info = LvlFindInChain<VkMultisampledRenderToSingleSampledInfoEXT>(subpass_desc->pNext);
        if (msrtss_info && msrtss_info->multisampledRenderToSingleSampledEnable) {
            if (msrtss_info->rasterizationSamples != pipeline.MultisampleState()->rasterizationSamples) {
                skip |= LogError(
                    rp_state->renderPass(), "VUID-VkGraphicsPipelineCreateInfo-renderPass-06854",
                    "vkCreateGraphicsPipelines(): A VkMultisampledRenderToSingleSampledInfoEXT struct in the pNext chain of "
                    "pCreateInfo[%" PRIu32 "], subpass index %" PRIu32
                    "'s VkSubpassDescription2 has a rasterizationSamples of (%" PRIu32
                    ") which is not equal to  pMultisampleState.rasterizationSamples which is (%" PRIu32 ").",
                    pipeline.create_index, subpass, msrtss_info->rasterizationSamples,
                    pipeline.MultisampleState()->rasterizationSamples);
            }
        }
    }

    skip |= ValidateGraphicsPipelineLibrary(pipeline);
    skip |= ValidateGraphicsPipelinePreRasterState(pipeline);
    skip |= ValidateGraphicsPipelineColorBlendState(pipeline, subpass_desc);
    skip |= ValidateGraphicsPipelineRasterizationState(pipeline, subpass_desc);
    skip |= ValidateGraphicsPipelineMultisampleState(pipeline, subpass_desc);
    skip |= ValidateGraphicsPipelineDynamicState(pipeline);
    skip |= ValidateGraphicsPipelineFragmentShadingRateState(pipeline);
    skip |= ValidateGraphicsPipelineDynamicRendering(pipeline);
    skip |= ValidateGraphicsPipelineShaderState(pipeline);
    skip |= ValidateGraphicsPipelineBlendEnable(pipeline);

    const uint32_t active_shaders = pipeline.active_shaders;
    if (pipeline.pre_raster_state || pipeline.fragment_shader_state) {
        vvl::unordered_set<VkShaderStageFlags> unique_stage_set;
        const auto stages = pipeline.GetShaderStages();
        for (const auto &stage : stages) {
            if (!unique_stage_set.insert(stage.stage).second) {
                skip |=
                    LogError(device, "VUID-VkGraphicsPipelineCreateInfo-stage-06897",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32 "] State: Multiple shaders provided for stage %s",
                             pipeline.create_index, string_VkShaderStageFlagBits(stage.stage));
            }
        }
    }

    if (pipeline.pre_raster_state && !pipeline.fragment_shader_state &&
        ((active_shaders & FragmentShaderState::ValidShaderStages()) != 0)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pStages-06894",
                         "vkCreateGraphicsPipelines(): pCreateInfo[%" PRIu32
                         "] does not have fragment shader state, but stages (%s) contains VK_SHADER_STAGE_FRAGMENT_BIT.",
                         pipeline.create_index, string_VkShaderStageFlags(active_shaders).c_str());
    }

    if (pipeline.fragment_shader_state && !pipeline.pre_raster_state &&
        ((active_shaders & PreRasterState::ValidShaderStages()) != 0)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pStages-06895",
                         "vkCreateGraphicsPipelines(): pCreateInfo[%" PRIu32
                         "] does not have pre-raster state, but stages (%s) contains pre-raster shader stages.",
                         pipeline.create_index, string_VkShaderStageFlags(active_shaders).c_str());
    }

    // Check if a Vertex Input State is used
    // Need to make sure it has a vertex shader and if a GPL, that it contains a GPL vertex input state
    // vkspec.html#pipelines-graphics-subsets-vertex-input
    if ((active_shaders & VK_SHADER_STAGE_VERTEX_BIT) &&
        (!pipeline.IsGraphicsLibrary() || (pipeline.IsGraphicsLibrary() && pipeline.vertex_input_state))) {
        if (!pipeline.IsDynamic(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT)) {
            const auto *input_state = pipeline.InputState();
            if (!input_state) {
                const char *vuid = IsExtEnabled(device_extensions.vk_ext_vertex_input_dynamic_state)
                                       ? "VUID-VkGraphicsPipelineCreateInfo-pVertexInputState-04910"
                                       : "VUID-VkGraphicsPipelineCreateInfo-pStages-02097";
                skip |= LogError(device, vuid,
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32 "] State: Missing pVertexInputState.",
                                 pipeline.create_index);
            } else if (IsExtEnabled(device_extensions.vk_ext_vertex_attribute_divisor)) {
                const auto *binding_descriptions = pipeline.BindingDescriptions();
                if (binding_descriptions) {
                    skip |= ValidatePipelineVertexDivisors(*input_state, *binding_descriptions, pipeline.create_index);
                }
            }
        }

        if (!pipeline.InputAssemblyState()) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pStages-02098",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32 "] State: Missing pInputAssemblyState.",
                             pipeline.create_index);
        }
    }

    skip |= ValidatePipelineCacheControlFlags(pipeline.GetPipelineCreateFlags(), pipeline.create_index, "vkCreateGraphicsPipelines",
                                              "VUID-VkGraphicsPipelineCreateInfo-pipelineCreationCacheControl-02878");
    skip |= ValidatePipelineProtectedAccessFlags(pipeline.GetPipelineCreateFlags(), pipeline.create_index);

    const auto *discard_rectangle_state = LvlFindInChain<VkPipelineDiscardRectangleStateCreateInfoEXT>(pipeline.PNext());
    if (discard_rectangle_state) {
        if (discard_rectangle_state->discardRectangleCount > phys_dev_ext_props.discard_rectangle_props.maxDiscardRectangles) {
            skip |= LogError(
                device, "VUID-VkPipelineDiscardRectangleStateCreateInfoEXT-discardRectangleCount-00582",
                "vkCreateGraphicsPipelines(): VkPipelineDiscardRectangleStateCreateInfoEXT::discardRectangleCount (%" PRIu32
                ") in pNext chain of pCreateInfo[%" PRIu32
                "] is not less than VkPhysicalDeviceDiscardRectanglePropertiesEXT::maxDiscardRectangles (%" PRIu32 ".",
                discard_rectangle_state->discardRectangleCount, pipeline.create_index,
                phys_dev_ext_props.discard_rectangle_props.maxDiscardRectangles);
        }
    }

    // VkAttachmentSampleCountInfoAMD == VkAttachmentSampleCountInfoNV
    const auto attachment_sample_count_info = LvlFindInChain<VkAttachmentSampleCountInfoAMD>(pipeline.PNext());
    if (attachment_sample_count_info) {
        const uint32_t bits = GetBitSetCount(attachment_sample_count_info->depthStencilAttachmentSamples);
        if (pipeline.fragment_output_state && attachment_sample_count_info->depthStencilAttachmentSamples != 0 &&
            ((attachment_sample_count_info->depthStencilAttachmentSamples & AllVkSampleCountFlagBits) == 0 || bits > 1)) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-depthStencilAttachmentSamples-06593",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                             "] includes VkAttachmentSampleCountInfoAMD with invalid depthStencilAttachmentSamples (%" PRIx32 ").",
                             pipeline.create_index, attachment_sample_count_info->depthStencilAttachmentSamples);
        }
    }

    if (IsExtEnabled(device_extensions.vk_ext_graphics_pipeline_library) &&
        !IsExtEnabled(device_extensions.vk_khr_dynamic_rendering)) {
        if (pipeline.fragment_output_state && pipeline.MultisampleState() == nullptr) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pMultisampleState-06630",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                             "] is being created with fragment shader that uses samples, but pMultisampleState is not set.",
                             pipeline.create_index);
        }
    }

    if (IsExtEnabled(device_extensions.vk_khr_dynamic_rendering) && IsExtEnabled(device_extensions.vk_khr_multiview)) {
        if (pipeline.fragment_shader_state && pipeline.GetCreateInfo<VkGraphicsPipelineCreateInfo>().renderPass == VK_NULL_HANDLE) {
            for (const auto &stage : pipeline.stage_state) {
                if (stage.stage_flag == VK_SHADER_STAGE_FRAGMENT_BIT && stage.has_input_attachment_capability) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06061",
                                     "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                     "] is being created with fragment shader state and renderPass = VK_NULL_HANDLE, but fragment "
                                     "shader includes InputAttachment capability.",
                                     pipeline.create_index);
                    break;
                }
            }
        }
    }

    if (pipeline.fragment_shader_state && pipeline.GetCreateInfo<VkGraphicsPipelineCreateInfo>().renderPass != VK_NULL_HANDLE &&
        pipeline.GetCreateInfo<VkGraphicsPipelineCreateInfo>().pMultisampleState == nullptr &&
        IsExtEnabled(device_extensions.vk_khr_dynamic_rendering) &&
        IsExtEnabled(device_extensions.vk_ext_graphics_pipeline_library)) {
        skip |= LogError(
            device, "VUID-VkGraphicsPipelineCreateInfo-renderpass-06631",
            "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
            "] is being created with fragment shader state and renderPass != VK_NULL_HANDLE, but pMultisampleState is NULL.",
            pipeline.create_index);
    }

    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineLibrary(const PIPELINE_STATE &pipeline) const {
    bool skip = false;

    // If VK_EXT_graphics_pipeline_library is not enabled, a complete set of state must be defined at this point
    std::string full_pipeline_state_msg;
    if (!pipeline.vertex_input_state) {
        full_pipeline_state_msg += "<vertex input> ";
    }
    if (!pipeline.pre_raster_state) {
        full_pipeline_state_msg += "<pre-raster> ";
    }
    if (!pipeline.fragment_shader_state) {
        full_pipeline_state_msg += "<fragment shader> ";
    }
    if (!pipeline.fragment_output_state) {
        full_pipeline_state_msg += "<fragment output> ";
    }

    if (!IsExtEnabled(device_extensions.vk_ext_graphics_pipeline_library)) {
        if (!pipeline.HasFullState()) {
            skip |=
                LogError(device, "VUID-VkGraphicsPipelineCreateInfo-None-06573",
                         "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                         "] does not contain a complete set of state and %s is not enabled. The following state is missing: [ %s].",
                         pipeline.create_index, VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME, full_pipeline_state_msg.c_str());
        }

        if (!pipeline.PipelineLayoutState()) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-layout-06602",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                             "].layout is not a valid pipeline layout, but %s is not enabled.",
                             pipeline.create_index, VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
        }

        if (!pipeline.RenderPassState()) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06603",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                             "].renderPass is not a valid render pass, but %s is not enabled.",
                             pipeline.create_index, VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
        }
    } else {
        const VkPipelineCreateFlags pipeline_flags = pipeline.GetPipelineCreateFlags();
        const bool is_library = (pipeline_flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) != 0;

        if (!enabled_features.graphics_pipeline_library_features.graphicsPipelineLibrary) {
            if (is_library) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-graphicsPipelineLibrary-06606",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "].flags (%s) contains VK_PIPELINE_CREATE_LIBRARY_BIT_KHR, but "
                                 "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary is not enabled.",
                                 pipeline.create_index, string_VkPipelineCreateFlags(pipeline_flags).c_str());
            }

            if (!pipeline.HasFullState()) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-graphicsPipelineLibrary-06607",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "] does not contain a complete set of state and "
                                 "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary is not enabled. "
                                 "The following state is missing: [ %s].",
                                 pipeline.create_index, full_pipeline_state_msg.c_str());
            }
        }

        if (pipeline.HasFullState() && is_library) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-06608",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                             "] defines a complete set of state, but pCreateInfos[%" PRIu32
                             "].flags (%s) includes VK_PIPELINE_CREATE_LIBRARY_BIT_KHR.",
                             pipeline.create_index, pipeline.create_index, string_VkPipelineCreateFlags(pipeline_flags).c_str());
        }

        enum GPLInitInfo : uint8_t {
            uninitialized = 0,
            from_gpl_info,
            from_link_info,
        };
        struct GPLValidInfo {
            GPLValidInfo() = default;
            GPLValidInfo(GPLInitInfo ii, const PIPELINE_LAYOUT_STATE *pls) : init_type(ii), layout_state(pls) {}
            GPLInitInfo init_type = GPLInitInfo::uninitialized;
            const PIPELINE_LAYOUT_STATE *layout_state = nullptr;
        };
        std::pair<VkPipelineLayoutCreateFlags, GPLValidInfo> pre_raster_flags = std::make_pair(
                                                                 VK_PIPELINE_LAYOUT_CREATE_FLAG_BITS_MAX_ENUM, GPLValidInfo{}),
                                                             fs_flags = std::make_pair(VK_PIPELINE_LAYOUT_CREATE_FLAG_BITS_MAX_ENUM,
                                                                                       GPLValidInfo{});
        const auto gpl_info = LvlFindInChain<VkGraphicsPipelineLibraryCreateInfoEXT>(pipeline.PNext());
        if (gpl_info) {
            if ((gpl_info->flags & (VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
                                    VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT)) != 0) {
                if (!pipeline.PipelineLayoutState()) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-06642",
                                     "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                     "] is a graphics library created with %s state, but does not have a valid layout specified.",
                                     pipeline.create_index, string_VkGraphicsPipelineLibraryFlagsEXT(gpl_info->flags).c_str());
                }
            }

            if ((gpl_info->flags & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT) &&
                !(gpl_info->flags & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT)) {
                pre_raster_flags.first =
                    (pipeline.pre_raster_state->pipeline_layout) ? pipeline.pre_raster_state->pipeline_layout->CreateFlags() : 0;
                pre_raster_flags.second = {GPLInitInfo::from_gpl_info, pipeline.PreRasterPipelineLayoutState().get()};
            } else if ((gpl_info->flags & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) &&
                       !(gpl_info->flags & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT)) {
                fs_flags.first = (pipeline.fragment_shader_state->pipeline_layout)
                                     ? pipeline.fragment_shader_state->pipeline_layout->CreateFlags()
                                     : 0;
                fs_flags.second = {GPLInitInfo::from_gpl_info, pipeline.FragmentShaderPipelineLayoutState().get()};
            }
        }

        const auto link_info = LvlFindInChain<VkPipelineLibraryCreateInfoKHR>(pipeline.PNext());
        if (link_info) {
            const bool has_link_time_opt = (pipeline_flags & VK_PIPELINE_CREATE_LINK_TIME_OPTIMIZATION_BIT_EXT) != 0;
            const bool has_retain_link_time_opt =
                (pipeline_flags & VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT) != 0;
            const bool has_capture_internal = (pipeline_flags & VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR) != 0;
            unsigned int descriptor_buffer_library_count = 0;
            bool lib_all_has_capture_internal = false;

            for (uint32_t i = 0; i < link_info->libraryCount; ++i) {
                const auto lib = Get<PIPELINE_STATE>(link_info->pLibraries[i]);
                if (lib) {
                    const VkPipelineCreateFlags lib_pipeline_flags = lib->GetPipelineCreateFlags();
                    if (lib->PipelineLayoutState()) {
                        if (lib->graphics_lib_type == VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT) {
                            pre_raster_flags.first = lib->PipelineLayoutState()->CreateFlags();
                            pre_raster_flags.second = {GPLInitInfo::from_link_info, lib->PipelineLayoutState().get()};
                        } else if (lib->graphics_lib_type == VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) {
                            fs_flags.first = lib->PipelineLayoutState()->CreateFlags();
                            fs_flags.second = {GPLInitInfo::from_link_info, lib->PipelineLayoutState().get()};
                        }
                    }

                    const bool lib_has_retain_link_time_opt =
                        (lib_pipeline_flags & VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT) != 0;
                    if (has_link_time_opt && !lib_has_retain_link_time_opt) {
                        const LogObjectList objlist(device, lib->Handle());
                        skip |= LogError(objlist, "VUID-VkGraphicsPipelineCreateInfo-flags-06609",
                                         "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                         "] has VK_PIPELINE_CREATE_LINK_TIME_OPTIMIZATION_BIT_EXT set, but "
                                         "VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT is not set for the %s "
                                         "library included in VkPipelineLibraryCreateInfoKHR::pLibraries[%" PRIu32 "].",
                                         pipeline.create_index,
                                         string_VkGraphicsPipelineLibraryFlagsEXT(lib->graphics_lib_type).c_str(), i);
                    }

                    if (has_retain_link_time_opt && !lib_has_retain_link_time_opt) {
                        const LogObjectList objlist(device, lib->Handle());
                        skip |= LogError(objlist, "VUID-VkGraphicsPipelineCreateInfo-flags-06610",
                                         "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                         "] has VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT set, but "
                                         "VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT is not set for the %s "
                                         "library included in VkPipelineLibraryCreateInfoKHR::pLibraries[%" PRIu32 "].",
                                         pipeline.create_index,
                                         string_VkGraphicsPipelineLibraryFlagsEXT(lib->graphics_lib_type).c_str(), i);
                    }

                    const bool lib_has_capture_internal =
                        (lib_pipeline_flags & VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR) != 0;
                    const bool non_zero_gpl = gpl_info && gpl_info->flags != 0;
                    if (lib_has_capture_internal) {
                        lib_all_has_capture_internal = true;
                        if (!has_capture_internal && non_zero_gpl) {
                            const LogObjectList objlist(device, lib->Handle());
                            skip |= LogError(objlist, "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06647",
                                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                             "] has VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR not set, but "
                                             "VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR is set for the %s "
                                             "library included in VkPipelineLibraryCreateInfoKHR::pLibraries[%" PRIu32 "].",
                                             pipeline.create_index,
                                             string_VkGraphicsPipelineLibraryFlagsEXT(lib->graphics_lib_type).c_str(), i);
                        }
                    } else {
                        if (lib_all_has_capture_internal) {
                            const LogObjectList objlist(device, lib->Handle());
                            skip |= LogError(objlist, "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06646",
                                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                             "] VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR is not set for the %s "
                                             "library included in VkPipelineLibraryCreateInfoKHR::pLibraries[%" PRIu32
                                             "] but needs to be set for all libraries.",
                                             pipeline.create_index,
                                             string_VkGraphicsPipelineLibraryFlagsEXT(lib->graphics_lib_type).c_str(), i);
                        } else if (has_capture_internal && non_zero_gpl) {
                            const LogObjectList objlist(device, lib->Handle());
                            skip |= LogError(objlist, "VUID-VkGraphicsPipelineCreateInfo-flags-06645",
                                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                             "] has VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR set, but "
                                             "VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR is not set for the %s "
                                             "library included in VkPipelineLibraryCreateInfoKHR::pLibraries[%" PRIu32 "].",
                                             pipeline.create_index,
                                             string_VkGraphicsPipelineLibraryFlagsEXT(lib->graphics_lib_type).c_str(), i);
                        }
                    }

                    if ((lib->uses_shader_module_id) &&
                        !(pipeline_flags & VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT)) {
                        const LogObjectList objlist(device);
                        skip |= LogError(objlist, "VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-06855",
                                         "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                         "] does not have the VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT set, but "
                                         "the %s "
                                         "library included in VkPipelineLibraryCreateInfoKHR::pLibraries[%" PRIu32
                                         "] was created with a shader stage with "
                                         "VkPipelineShaderStageModuleIdentifierCreateInfoEXT and identifierSize not equal to 0",
                                         pipeline.create_index,
                                         string_VkGraphicsPipelineLibraryFlagsEXT(lib->graphics_lib_type).c_str(), i);
                    }
                    struct check_struct {
                        VkPipelineCreateFlagBits bit;
                        std::string first_vuid;
                        std::string second_vuid;
                    };
                    static const std::array<check_struct, 2> check_infos = {
                        {{VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT, "VUID-VkPipelineLibraryCreateInfoKHR-pipeline-07404",
                          "VUID-VkPipelineLibraryCreateInfoKHR-pipeline-07405"},
                         {VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT, "VUID-VkPipelineLibraryCreateInfoKHR-pipeline-07406",
                          "VUID-VkPipelineLibraryCreateInfoKHR-pipeline-07407"}}};
                    for (const auto &check_info : check_infos) {
                        if ((pipeline_flags & check_info.bit)) {
                            if (!(lib_pipeline_flags & check_info.bit)) {
                                const LogObjectList objlist(device, lib->Handle());
                                skip |= LogError(objlist, check_info.first_vuid.c_str(),
                                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                                 "] has the %s bit set, but "
                                                 "the library included in VkPipelineLibraryCreateInfoKHR::pLibraries[%" PRIu32
                                                 "] was created without it",
                                                 pipeline.create_index, string_VkPipelineCreateFlagBits(check_info.bit), i);
                            }
                        } else {
                            if ((lib_pipeline_flags & check_info.bit)) {
                                const LogObjectList objlist(device, lib->Handle());
                                skip |= LogError(objlist, check_info.second_vuid.c_str(),
                                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                                 "] does not have the %s bit set, but "
                                                 "the library included in VkPipelineLibraryCreateInfoKHR::pLibraries[%" PRIu32
                                                 "] was created with it set",
                                                 pipeline.create_index, string_VkPipelineCreateFlagBits(check_info.bit), i);
                            }
                        }
                    }
                    if (lib->descriptor_buffer_mode) {
                        ++descriptor_buffer_library_count;
                    }
                }
            }

            if ((descriptor_buffer_library_count != 0) && (link_info->libraryCount != descriptor_buffer_library_count)) {
                skip |= LogError(device, "VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-08096",
                                 "vkCreateGraphicsPipelines(): All or none of the elements of pCreateInfo[%" PRIu32
                                 "].pLibraryInfo->pLibraries must be created "
                                 "with VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT.",
                                 pipeline.create_index);
            }
        }

        if (link_info && pipeline.GetCreateInfo<VkGraphicsPipelineCreateInfo>().renderPass == VK_NULL_HANDLE) {
            const auto rendering_struct = LvlFindInChain<VkPipelineRenderingCreateInfo>(pipeline.PNext());
            if (gpl_info) {
                skip |= ValidatePipelineLibraryFlags(gpl_info->flags, *link_info, rendering_struct, pipeline.create_index, -1,
                                                     "VUID-VkGraphicsPipelineCreateInfo-flags-06626");

                const uint32_t flags_count =
                    GetBitSetCount(gpl_info->flags & (VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
                                                      VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
                                                      VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT));
                if (flags_count >= 1 && flags_count <= 2) {
                    for (uint32_t i = 0; i < link_info->libraryCount; ++i) {
                        const auto lib = Get<PIPELINE_STATE>(link_info->pLibraries[i]);
                        const auto lib_gpl_info = LvlFindInChain<VkGraphicsPipelineLibraryCreateInfoEXT>(lib->PNext());
                        if (!lib_gpl_info) {
                            continue;
                        }
                        const std::array<VkGraphicsPipelineLibraryFlagBitsEXT, 3> flags = {
                            VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT,
                            VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT,
                            VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT};
                        for (const auto flag : flags) {
                            if ((lib_gpl_info->flags & flag) > 0 && (gpl_info->flags & flag) == 0) {
                                break;
                            }
                        }
                    }
                }
            }
            for (uint32_t i = 0; i < link_info->libraryCount; ++i) {
                const auto lib = Get<PIPELINE_STATE>(link_info->pLibraries[i]);
                const auto lib_rendering_struct = lib->GetPipelineRenderingCreateInfo();
                skip |=
                    ValidatePipelineLibraryFlags(lib->graphics_lib_type, *link_info, lib_rendering_struct, pipeline.create_index, i,
                                                 "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06627");
            }
        }

        if ((pre_raster_flags.second.init_type != GPLInitInfo::uninitialized) &&
            (fs_flags.second.init_type != GPLInitInfo::uninitialized)) {
            const char *vuid = nullptr;
            const bool only_libs = (pre_raster_flags.second.init_type == GPLInitInfo::from_link_info) &&
                                   (fs_flags.second.init_type == GPLInitInfo::from_link_info);
            if (pre_raster_flags.second.init_type != fs_flags.second.init_type) {
                vuid = "VUID-VkGraphicsPipelineCreateInfo-flags-06614";
            } else if (only_libs) {
                vuid = "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06615";
            }

            // vuid != null => pre-raster and fragemnt shader state is defined by some combination of this library and pLibraries
            if (vuid) {
                // Check for consistent independent sets across libraries
                const auto pre_raster_indset = (pre_raster_flags.first & VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
                const auto fs_indset = (fs_flags.first & VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
                if (pre_raster_indset ^ fs_indset) {
                    const char *pre_raster_str = (pre_raster_indset != 0) ? "defined with" : "not defined with";
                    const char *fs_str = (fs_indset != 0) ? "defined with" : "not defined with";
                    skip |= LogError(
                        device, vuid,
                        "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                        "] is attempting to create a graphics pipeline library with pre-raster and fragment shader state. However "
                        "the "
                        "pre-raster layout create flags (%s) are %s VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT, and the "
                        "fragment shader layout create flags (%s) are %s VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT",
                        pipeline.create_index, string_VkPipelineLayoutCreateFlags(pre_raster_flags.first).c_str(), pre_raster_str,
                        string_VkPipelineLayoutCreateFlags(fs_flags.first).c_str(), fs_str);
                }

                // Check for consistent shader bindings + layout across libraries
                const char *const vuid_only_libs_binding =
                    (only_libs) ? "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06758" : nullptr;
                const auto &pre_raster_set_layouts = pre_raster_flags.second.layout_state->set_layouts;
                const auto &fs_set_layouts = fs_flags.second.layout_state->set_layouts;
                const auto num_set_layouts = std::max(pre_raster_set_layouts.size(), fs_set_layouts.size());
                for (size_t i = 0; i < num_set_layouts; ++i) {
                    const auto pre_raster_dsl = (i < pre_raster_set_layouts.size())
                                                    ? pre_raster_set_layouts[i]
                                                    : vvl::base_type<decltype(pre_raster_set_layouts)>::value_type{};
                    const auto fs_dsl = (i < fs_set_layouts.size()) ? fs_set_layouts[i]
                                                                    : vvl::base_type<decltype(fs_set_layouts)>::value_type{};
                    const char *vuid_tmp = nullptr;
                    std::ostringstream msg("vkCreateGraphicsPipelines(): ");
                    msg << "pCreateInfos[" << pipeline.create_index << "] ";
                    if (!pre_raster_dsl && fs_dsl) {
                        // Null DSL at pSetLayouts[i] in pre-raster state. Make sure that shader bindings in corresponding DSL in
                        // fragment shader state do not overlap.
                        for (const auto &fs_binding : fs_dsl->GetBindings()) {
                            if (fs_binding.stageFlags & (PreRasterState::ValidShaderStages())) {
                                const auto pre_raster_layout_handle_str =
                                    report_data->FormatHandle(pre_raster_flags.second.layout_state->Handle());
                                const auto fs_layout_handle_str = report_data->FormatHandle(fs_flags.second.layout_state->Handle());
                                if (pre_raster_flags.second.init_type == GPLInitInfo::from_gpl_info) {
                                    vuid_tmp = "VUID-VkGraphicsPipelineCreateInfo-flags-06756";
                                    msg << "represents a library containing pre-raster state, and descriptor set layout (from "
                                           "layout "
                                        << pre_raster_layout_handle_str << ") at pSetLayouts[" << i << "] is null. "
                                        << "However, a library with fragment shader state is specified in "
                                           "VkPipelineLibraryCreateInfoKHR::pLibraries with non-null descriptor set layout at the "
                                           "same pSetLayouts index ("
                                        << i << ") from layout " << fs_layout_handle_str << " and bindings ("
                                        << string_VkShaderStageFlags(fs_binding.stageFlags)
                                        << ") that overlap with pre-raster state.";
                                } else if (fs_flags.second.init_type == GPLInitInfo::from_gpl_info) {
                                    vuid_tmp = "VUID-VkGraphicsPipelineCreateInfo-flags-06757";
                                    msg << "represents a library containing fragment shader state, and descriptor set layout (from "
                                           "layout "
                                        << fs_layout_handle_str << ") at pSetLayouts[" << i << "] contains bindings ("
                                        << string_VkShaderStageFlags(fs_binding.stageFlags)
                                        << ") that overlap with pre-raster state. "
                                        << "However, a library with pre-raster state is specified in "
                                           "VkPipelineLibraryCreateInfoKHR::pLibraries with a null descriptor set layout at the "
                                           "same pSetLayouts index ("
                                        << i << ") from layout " << pre_raster_layout_handle_str << ".";
                                } else {
                                    vuid_tmp = vuid_only_libs_binding;
                                    msg << "is linking libraries with pre-raster and fragment shader state. The descriptor set "
                                           "layout at index "
                                        << i << " in pSetLayouts from " << pre_raster_layout_handle_str
                                        << " in the pre-raster state is null. "
                                        << "However, the descriptor set layout at the same index (" << i << ") in "
                                        << fs_layout_handle_str << " is non-null with bindings ("
                                        << string_VkShaderStageFlags(fs_binding.stageFlags)
                                        << ") that overlap with pre-raster state.";
                                }
                                break;
                            }
                        }
                    } else if (pre_raster_dsl && !fs_dsl) {
                        // Null DSL at pSetLayouts[i] in FS state. Make sure that shader bindings in corresponding DSL in pre-raster
                        // state do not overlap.
                        for (const auto &pre_raster_binding : pre_raster_dsl->GetBindings()) {
                            if (pre_raster_binding.stageFlags & (FragmentShaderState::ValidShaderStages())) {
                                const auto pre_raster_layout_handle_str =
                                    report_data->FormatHandle(pre_raster_flags.second.layout_state->Handle());
                                const auto fs_layout_handle_str = report_data->FormatHandle(fs_flags.second.layout_state->Handle());
                                if (fs_flags.second.init_type == GPLInitInfo::from_gpl_info) {
                                    vuid_tmp = "VUID-VkGraphicsPipelineCreateInfo-flags-06756";
                                    msg << "represents a library containing fragment shader state, and descriptor set layout (from "
                                           "layout "
                                        << fs_layout_handle_str << ") at pSetLayouts[" << i << "] is null. "
                                        << "However, a library with pre-raster state is specified in "
                                           "VkPipelineLibraryCreateInfoKHR::pLibraries with non-null descriptor set layout at the "
                                           "same pSetLayouts index ("
                                        << i << ") from layout " << pre_raster_layout_handle_str << " and bindings ("
                                        << string_VkShaderStageFlags(pre_raster_binding.stageFlags)
                                        << ") that overlap with fragment shader state.";
                                    break;
                                } else if (pre_raster_flags.second.init_type == GPLInitInfo::from_gpl_info) {
                                    vuid_tmp = "VUID-VkGraphicsPipelineCreateInfo-flags-06757";
                                    msg << "represents a library containing pre-raster state, and descriptor set layout (from "
                                           "layout "
                                        << pre_raster_layout_handle_str << ") at pSetLayouts[" << i << "] contains bindings ("
                                        << string_VkShaderStageFlags(pre_raster_binding.stageFlags)
                                        << ") that overlap with fragment shader state. "
                                        << "However, a library with fragment shader state is specified in "
                                           "VkPipelineLibraryCreateInfoKHR::pLibraries with a null descriptor set layout at the "
                                           "same pSetLayouts index ("
                                        << i << ") from layout " << fs_layout_handle_str << ".";
                                    break;
                                } else {
                                    vuid_tmp = vuid_only_libs_binding;
                                    msg << "is linking libraries with pre-raster and fragment shader state. The descriptor set "
                                           "layout at index "
                                        << i << " in pSetLayouts from " << fs_layout_handle_str
                                        << " in the fragment shader state is null. "
                                        << "However, the descriptor set layout at the same index (" << i << ") in "
                                        << pre_raster_layout_handle_str << " in the pre-raster state is non-null with bindings ("
                                        << string_VkShaderStageFlags(pre_raster_binding.stageFlags)
                                        << ") that overlap with fragment shader "
                                           "state.";
                                    break;
                                }
                            }
                        }
                    }
                    if (vuid_tmp) {
                        skip |= LogError(device, vuid_tmp, "%s", msg.str().c_str());
                    }
                }
            }
        }

        const auto &rp_state = pipeline.RenderPassState();
        if ((!rp_state || !rp_state->renderPass()) && pipeline.fragment_shader_state && !pipeline.fragment_output_state) {
            if (!pipeline.DepthStencilState() ||
                (pipeline.DepthStencilState()->sType != VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO)) {
                skip |= LogError(
                    device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06590",
                    "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                    "] does contains fragment shader state and no fragment output state, pDepthStencilState does not point to "
                    "a valid VkPipelineDepthStencilStateCreateInfo struct.",
                    pipeline.create_index);
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidatePipelineVertexDivisors(const safe_VkPipelineVertexInputStateCreateInfo &input_state,
                                                const std::vector<VkVertexInputBindingDescription> &binding_descriptions,
                                                const uint32_t pipe_index) const {
    bool skip = false;
    const auto divisor_state_info = LvlFindInChain<VkPipelineVertexInputDivisorStateCreateInfoEXT>(input_state.pNext);
    if (divisor_state_info) {
        const VkPhysicalDeviceLimits *device_limits = &phys_dev_props.limits;
        for (uint32_t j = 0; j < divisor_state_info->vertexBindingDivisorCount; j++) {
            const VkVertexInputBindingDivisorDescriptionEXT *vibdd = &(divisor_state_info->pVertexBindingDivisors[j]);
            if (vibdd->binding >= device_limits->maxVertexInputBindings) {
                skip |= LogError(device, "VUID-VkVertexInputBindingDivisorDescriptionEXT-binding-01869",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "] with chained VkPipelineVertexInputDivisorStateCreateInfoEXT, "
                                 "pVertexBindingDivisors[%1u] binding index of (%1u) exceeds device maxVertexInputBindings (%1u).",
                                 pipe_index, j, vibdd->binding, device_limits->maxVertexInputBindings);
            }
            if (vibdd->divisor > phys_dev_ext_props.vtx_attrib_divisor_props.maxVertexAttribDivisor) {
                skip |= LogError(device, "VUID-VkVertexInputBindingDivisorDescriptionEXT-divisor-01870",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "] with chained VkPipelineVertexInputDivisorStateCreateInfoEXT, "
                                 "pVertexBindingDivisors[%1u] divisor of (%1u) exceeds extension maxVertexAttribDivisor (%1u).",
                                 pipe_index, j, vibdd->divisor, phys_dev_ext_props.vtx_attrib_divisor_props.maxVertexAttribDivisor);
            }
            if ((0 == vibdd->divisor) && !enabled_features.vtx_attrib_divisor_features.vertexAttributeInstanceRateZeroDivisor) {
                skip |= LogError(
                    device, "VUID-VkVertexInputBindingDivisorDescriptionEXT-vertexAttributeInstanceRateZeroDivisor-02228",
                    "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                    "] with chained VkPipelineVertexInputDivisorStateCreateInfoEXT, "
                    "pVertexBindingDivisors[%1u] divisor must not be 0 when vertexAttributeInstanceRateZeroDivisor feature is not "
                    "enabled.",
                    pipe_index, j);
            }
            if ((1 != vibdd->divisor) && !enabled_features.vtx_attrib_divisor_features.vertexAttributeInstanceRateDivisor) {
                skip |= LogError(
                    device, "VUID-VkVertexInputBindingDivisorDescriptionEXT-vertexAttributeInstanceRateDivisor-02229",
                    "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                    "] with chained VkPipelineVertexInputDivisorStateCreateInfoEXT, "
                    "pVertexBindingDivisors[%1u] divisor (%1u) must be 1 when vertexAttributeInstanceRateDivisor feature is not "
                    "enabled.",
                    pipe_index, j, vibdd->divisor);
            }

            // Find the corresponding binding description and validate input rate setting
            bool failed_01871 = true;
            for (size_t k = 0; k < binding_descriptions.size(); k++) {
                if ((vibdd->binding == binding_descriptions[k].binding) &&
                    (VK_VERTEX_INPUT_RATE_INSTANCE == binding_descriptions[k].inputRate)) {
                    failed_01871 = false;
                    break;
                }
            }
            if (failed_01871) {  // Description not found, or has incorrect inputRate value
                skip |= LogError(device, "VUID-VkVertexInputBindingDivisorDescriptionEXT-inputRate-01871",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "] with chained VkPipelineVertexInputDivisorStateCreateInfoEXT, "
                                 "pVertexBindingDivisors[%1u] specifies binding index (%1u), but that binding index's "
                                 "VkVertexInputBindingDescription.inputRate member is not VK_VERTEX_INPUT_RATE_INSTANCE.",
                                 pipe_index, j, vibdd->binding);
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidatePipelineCacheControlFlags(VkPipelineCreateFlags flags, uint32_t index, const char *caller_name,
                                                   const char *vuid) const {
    bool skip = false;
    if (enabled_features.core13.pipelineCreationCacheControl == VK_FALSE) {
        const VkPipelineCreateFlags invalid_flags =
            VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT | VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT_EXT;
        if ((flags & invalid_flags) != 0) {
            skip |= LogError(device, vuid,
                             "%s(): pipelineCreationCacheControl is turned off but pCreateInfos[%" PRIu32
                             "]has VkPipelineCreateFlags "
                             "containing VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT or "
                             "VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT_EXT",
                             caller_name, index);
        }
    }
    return skip;
}

bool CoreChecks::ValidatePipelineProtectedAccessFlags(VkPipelineCreateFlags flags, uint32_t index) const {
    bool skip = false;
    if (enabled_features.pipeline_protected_access_features.pipelineProtectedAccess == VK_FALSE) {
        const VkPipelineCreateFlags invalid_flags =
            VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT | VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT;
        if ((flags & invalid_flags) != 0) {
            skip |= LogError(
                device, "VUID-VkGraphicsPipelineCreateInfo-pipelineProtectedAccess-07368",
                "vkCreateGraphicsPipelines(): pipelineProtectedAccess is turned off but pCreateInfos[%" PRIu32
                "] has VkPipelineCreateFlags "
                "(%s) "
                "that contain VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT or VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT",
                index, string_VkPipelineCreateFlags(flags).c_str());
        }
    }
    if ((flags & VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT) && (flags & VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT)) {
        skip |= LogError(
            device, "VUID-VkGraphicsPipelineCreateInfo-flags-07369",
            "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
            "] has VkPipelineCreateFlags that "
            "contains both VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT and VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT",
            index);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo *pCreateInfo,
                                                    const VkAllocationCallbacks *pAllocator,
                                                    VkPipelineCache *pPipelineCache) const {
    bool skip = false;
    if (enabled_features.core13.pipelineCreationCacheControl == VK_FALSE) {
        if ((pCreateInfo->flags & VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT_EXT) != 0) {
            skip |= LogError(device, "VUID-VkPipelineCacheCreateInfo-pipelineCreationCacheControl-02892",
                             "vkCreatePipelineCache(): pipelineCreationCacheControl is turned off but pCreateInfo::flags contains "
                             "VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT_EXT");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                        const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                        const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                        void *cgpl_state_data) const {
    bool skip = StateTracker::PreCallValidateCreateGraphicsPipelines(device, pipelineCache, count, pCreateInfos, pAllocator,
                                                                     pPipelines, cgpl_state_data);
    create_graphics_pipeline_api_state *cgpl_state = reinterpret_cast<create_graphics_pipeline_api_state *>(cgpl_state_data);

    for (uint32_t i = 0; i < count; i++) {
        skip |= ValidatePipeline(cgpl_state->pipe_state, i);
    }

    if (IsExtEnabled(device_extensions.vk_khr_portability_subset)) {
        for (uint32_t i = 0; i < count; ++i) {
            // Validate depth-stencil state
            auto raster_state_ci = pCreateInfos[i].pRasterizationState;
            if ((VK_FALSE == enabled_features.portability_subset_features.separateStencilMaskRef) && raster_state_ci &&
                (VK_CULL_MODE_NONE == raster_state_ci->cullMode)) {
                auto depth_stencil_ci = pCreateInfos[i].pDepthStencilState;
                if (depth_stencil_ci && (VK_TRUE == depth_stencil_ci->stencilTestEnable) &&
                    (depth_stencil_ci->front.reference != depth_stencil_ci->back.reference)) {
                    skip |= LogError(device, "VUID-VkPipelineDepthStencilStateCreateInfo-separateStencilMaskRef-04453",
                                     "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                     "] (portability error): VkStencilOpState::reference must be the "
                                     "same for front and back",
                                     i);
                }
            }

            // Validate color attachments
            uint32_t subpass = pCreateInfos[i].subpass;
            auto render_pass = Get<RENDER_PASS_STATE>(pCreateInfos[i].renderPass);
            const auto rendering_struct = LvlFindInChain<VkPipelineRenderingCreateInfo>(pCreateInfos[i].pNext);
            bool ignore_color_blend_state =
                pCreateInfos[i].pRasterizationState->rasterizerDiscardEnable ||
                (rendering_struct ? (rendering_struct->colorAttachmentCount == 0)
                                  : (render_pass->createInfo.pSubpasses[subpass].colorAttachmentCount == 0));

            if ((VK_FALSE == enabled_features.portability_subset_features.constantAlphaColorBlendFactors) &&
                !ignore_color_blend_state) {
                auto color_blend_state = pCreateInfos[i].pColorBlendState;
                const auto attachments = color_blend_state->pAttachments;
                for (uint32_t color_attachment_index = 0; i < color_blend_state->attachmentCount; ++i) {
                    if ((VK_BLEND_FACTOR_CONSTANT_ALPHA == attachments[color_attachment_index].srcColorBlendFactor) ||
                        (VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA == attachments[color_attachment_index].srcColorBlendFactor)) {
                        skip |= LogError(device, "VUID-VkPipelineColorBlendAttachmentState-constantAlphaColorBlendFactors-04454",
                                         "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                         "] (portability error): srcColorBlendFactor for color attachment %d must "
                                         "not be VK_BLEND_FACTOR_CONSTANT_ALPHA or VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA",
                                         i, color_attachment_index);
                    }
                    if ((VK_BLEND_FACTOR_CONSTANT_ALPHA == attachments[color_attachment_index].dstColorBlendFactor) ||
                        (VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA == attachments[color_attachment_index].dstColorBlendFactor)) {
                        skip |= LogError(device, "VUID-VkPipelineColorBlendAttachmentState-constantAlphaColorBlendFactors-04455",
                                         "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                         "] (portability error): dstColorBlendFactor for color attachment %d must "
                                         "not be VK_BLEND_FACTOR_CONSTANT_ALPHA or VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA",
                                         i, color_attachment_index);
                    }
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                       const VkComputePipelineCreateInfo *pCreateInfos,
                                                       const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                       void *ccpl_state_data) const {
    bool skip = StateTracker::PreCallValidateCreateComputePipelines(device, pipelineCache, count, pCreateInfos, pAllocator,
                                                                    pPipelines, ccpl_state_data);

    auto *ccpl_state = reinterpret_cast<create_compute_pipeline_api_state *>(ccpl_state_data);
    for (uint32_t i = 0; i < count; i++) {
        const PIPELINE_STATE *pipeline = ccpl_state->pipe_state[i].get();
        if (!pipeline) {
            continue;
        }
        skip |= ValidateComputePipelineShaderState(*pipeline);
        skip |= ValidatePipelineCacheControlFlags(pCreateInfos->flags, i, "vkCreateComputePipelines",
                                                  "VUID-VkComputePipelineCreateInfo-pipelineCreationCacheControl-02875");
    }
    return skip;
}

bool CoreChecks::ValidateRayTracingPipeline(const PIPELINE_STATE &pipeline,
                                            const safe_VkRayTracingPipelineCreateInfoCommon &create_info,
                                            VkPipelineCreateFlags flags, bool isKHR) const {
    bool skip = false;

    if (isKHR) {
        if (create_info.maxPipelineRayRecursionDepth > phys_dev_ext_props.ray_tracing_props_khr.maxRayRecursionDepth) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-maxPipelineRayRecursionDepth-03589",
                             "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32
                             "] maxPipelineRayRecursionDepth (%d ) must be less than or equal to "
                             "VkPhysicalDeviceRayTracingPipelinePropertiesKHR::maxRayRecursionDepth %d",
                             pipeline.create_index, create_info.maxPipelineRayRecursionDepth,
                             phys_dev_ext_props.ray_tracing_props_khr.maxRayRecursionDepth);
        }
        if (create_info.pLibraryInfo) {
            for (uint32_t i = 0; i < create_info.pLibraryInfo->libraryCount; ++i) {
                const auto library_pipelinestate = Get<PIPELINE_STATE>(create_info.pLibraryInfo->pLibraries[i]);
                const auto &library_create_info = library_pipelinestate->GetCreateInfo<VkRayTracingPipelineCreateInfoKHR>();
                if (library_create_info.maxPipelineRayRecursionDepth != create_info.maxPipelineRayRecursionDepth) {
                    skip |= LogError(
                        device, "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraries-03591",
                        "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32 "] pLibraries[%" PRIu32
                        "] member of libraries must have been"
                        "created with the value of maxPipelineRayRecursionDepth (%d) equal to that in this pipeline (%d) .",
                        pipeline.create_index, i, library_create_info.maxPipelineRayRecursionDepth,
                        create_info.maxPipelineRayRecursionDepth);
                }
                if (library_create_info.pLibraryInfo && (library_create_info.pLibraryInterface->maxPipelineRayHitAttributeSize !=
                                                             create_info.pLibraryInterface->maxPipelineRayHitAttributeSize ||
                                                         library_create_info.pLibraryInterface->maxPipelineRayPayloadSize !=
                                                             create_info.pLibraryInterface->maxPipelineRayPayloadSize)) {
                    skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-pLibraryInfo-03593",
                                     "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32 "] pLibraries[%" PRIu32
                                     "] must have been created with values of the maxPipelineRayPayloadSize and "
                                     "maxPipelineRayHitAttributeSize members of pLibraryInterface equal to those in this pipeline",
                                     pipeline.create_index, i);
                }
                if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR) &&
                    !(library_create_info.flags & VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR)) {
                    skip |=
                        LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-03594",
                                 "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32
                                 "] If flags includes "
                                 "VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR, pLibraries[%" PRIu32
                                 "] must have been created with the "
                                 "VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR bit set",
                                 pipeline.create_index, i);
                }
            }
        }
    } else {
        if (create_info.maxRecursionDepth > phys_dev_ext_props.ray_tracing_props_nv.maxRecursionDepth) {
            skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoNV-maxRecursionDepth-03457",
                             "vkCreateRayTracingPipelinesNV: pCreateInfos[%" PRIu32
                             "] maxRecursionDepth (%d) must be less than or equal to "
                             "VkPhysicalDeviceRayTracingPropertiesNV::maxRecursionDepth (%d)",
                             pipeline.create_index, create_info.maxRecursionDepth,
                             phys_dev_ext_props.ray_tracing_props_nv.maxRecursionDepth);
        }
    }
    const auto *groups = create_info.ptr()->pGroups;

    for (uint32_t stage_index = 0; stage_index < create_info.stageCount; stage_index++) {
        skip |= ValidatePipelineShaderStage(pipeline, pipeline.stage_state[stage_index]);
    }

    if ((create_info.flags & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) == 0) {
        const uint32_t raygen_stages_count = CalcShaderStageCount(pipeline, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
        if (raygen_stages_count == 0) {
            skip |= LogError(
                device,
                isKHR ? "VUID-VkRayTracingPipelineCreateInfoKHR-stage-03425" : "VUID-VkRayTracingPipelineCreateInfoNV-stage-06232",
                "vkCreateRayTracingPipelinesKHR : pCreateInfos[%" PRIu32
                "] The stage member of at least one element of pStages must be VK_SHADER_STAGE_RAYGEN_BIT_KHR.",
                pipeline.create_index);
        }
    }
    if ((flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR) != 0 &&
        (flags & VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR) != 0) {
        skip |= LogError(device, "VUID-VkRayTracingPipelineCreateInfoKHR-flags-06546",
                         "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32
                         "] flags (%s) contains both VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR and "
                         "VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR bits.",
                         pipeline.create_index, string_VkPipelineCreateFlags(flags).c_str());
    }

    for (uint32_t group_index = 0; group_index < create_info.groupCount; group_index++) {
        const auto &group = groups[group_index];

        if (group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV) {
            if (!GroupHasValidIndex(
                    pipeline, group.generalShader,
                    VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_MISS_BIT_NV | VK_SHADER_STAGE_CALLABLE_BIT_NV)) {
                skip |= LogError(device,
                                 isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03474"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02413",
                                 "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32 "] pGroups[%d]", pipeline.create_index,
                                 group_index);
            }
            if (group.anyHitShader != VK_SHADER_UNUSED_NV || group.closestHitShader != VK_SHADER_UNUSED_NV ||
                group.intersectionShader != VK_SHADER_UNUSED_NV) {
                skip |= LogError(device,
                                 isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03475"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02414",
                                 "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32 "] pGroups[%d]", pipeline.create_index,
                                 group_index);
            }
        } else if (group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV) {
            if (!GroupHasValidIndex(pipeline, group.intersectionShader, VK_SHADER_STAGE_INTERSECTION_BIT_NV)) {
                skip |= LogError(device,
                                 isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03476"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02415",
                                 "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32 "] pGroups[%d]", pipeline.create_index,
                                 group_index);
            }
        } else if (group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV) {
            if (group.intersectionShader != VK_SHADER_UNUSED_NV) {
                skip |= LogError(device,
                                 isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-type-03477"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-type-02416",
                                 "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32 "] pGroups[%d]", pipeline.create_index,
                                 group_index);
            }
        }

        if (group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV ||
            group.type == VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV) {
            if (!GroupHasValidIndex(pipeline, group.anyHitShader, VK_SHADER_STAGE_ANY_HIT_BIT_KHR)) {
                skip |= LogError(device,
                                 isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-anyHitShader-03479"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-anyHitShader-02418",
                                 "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32 "] pGroups[%d]", pipeline.create_index,
                                 group_index);
            }
            if (!GroupHasValidIndex(pipeline, group.closestHitShader, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR)) {
                skip |= LogError(device,
                                 isKHR ? "VUID-VkRayTracingShaderGroupCreateInfoKHR-closestHitShader-03478"
                                       : "VUID-VkRayTracingShaderGroupCreateInfoNV-closestHitShader-02417",
                                 "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32 "] pGroups[%d]", pipeline.create_index,
                                 group_index);
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                            const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                            const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                            void *crtpl_state_data) const {
    bool skip = StateTracker::PreCallValidateCreateRayTracingPipelinesNV(device, pipelineCache, count, pCreateInfos, pAllocator,
                                                                         pPipelines, crtpl_state_data);

    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_api_state *>(crtpl_state_data);
    for (uint32_t i = 0; i < count; i++) {
        const PIPELINE_STATE *pipeline = crtpl_state->pipe_state[i].get();
        if (!pipeline) {
            continue;
        }
        using CIType = vvl::base_type<decltype(pCreateInfos)>;
        if (pipeline->GetPipelineCreateFlags() & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
            std::shared_ptr<const PIPELINE_STATE> base_pipeline;
            const auto bpi = pipeline->BasePipelineIndex<CIType>();
            const auto bph = pipeline->BasePipeline<CIType>();
            if (bpi != -1) {
                base_pipeline = crtpl_state->pipe_state[bpi];
            } else if (bph != VK_NULL_HANDLE) {
                base_pipeline = Get<PIPELINE_STATE>(bph);
            }
            if (!base_pipeline || !(base_pipeline->GetPipelineCreateFlags() & VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT)) {
                skip |=
                    LogError(device, "VUID-vkCreateRayTracingPipelinesNV-flags-03416",
                             "vkCreateRayTracingPipelinesNV: pCreateInfos[%" PRIu32
                             "]  If the flags member of any element of pCreateInfos contains the "
                             "VK_PIPELINE_CREATE_DERIVATIVE_BIT flag,"
                             "the base pipeline must have been created with the VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT flag set.",
                             i);
            }
        }
        skip |= ValidateRayTracingPipeline(*pipeline, pipeline->GetCreateInfo<CIType>(), pCreateInfos[i].flags, /*isKHR*/ false);
        skip |= ValidatePipelineCacheControlFlags(pCreateInfos[i].flags, i, "vkCreateRayTracingPipelinesNV",
                                                  "VUID-VkRayTracingPipelineCreateInfoNV-pipelineCreationCacheControl-02905");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                             VkPipelineCache pipelineCache, uint32_t count,
                                                             const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                             const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                             void *crtpl_state_data) const {
    bool skip = StateTracker::PreCallValidateCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, count,
                                                                          pCreateInfos, pAllocator, pPipelines, crtpl_state_data);

    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_khr_api_state *>(crtpl_state_data);
    for (uint32_t i = 0; i < count; i++) {
        const PIPELINE_STATE *pipeline = crtpl_state->pipe_state[i].get();
        if (!pipeline) {
            continue;
        }
        using CIType = vvl::base_type<decltype(pCreateInfos)>;
        if (pipeline->GetPipelineCreateFlags() & VK_PIPELINE_CREATE_DERIVATIVE_BIT) {
            std::shared_ptr<const PIPELINE_STATE> base_pipeline;
            const auto bpi = pipeline->BasePipelineIndex<CIType>();
            const auto bph = pipeline->BasePipeline<CIType>();
            if (bpi != -1) {
                base_pipeline = crtpl_state->pipe_state[bpi];
            } else if (bph != VK_NULL_HANDLE) {
                base_pipeline = Get<PIPELINE_STATE>(bph);
            }
            if (!base_pipeline || !(base_pipeline->GetPipelineCreateFlags() & VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT)) {
                skip |=
                    LogError(device, "VUID-vkCreateRayTracingPipelinesKHR-flags-03416",
                             "vkCreateRayTracingPipelinesKHR: pCreateInfos[%" PRIu32
                             "]  If the flags member of any element of pCreateInfos contains the "
                             "VK_PIPELINE_CREATE_DERIVATIVE_BIT flag,"
                             "the base pipeline must have been created with the VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT flag set.",
                             i);
            }
        }
        skip |= ValidateRayTracingPipeline(*pipeline, pipeline->GetCreateInfo<CIType>(), pCreateInfos[i].flags, /*isKHR*/ true);
        skip |= ValidatePipelineCacheControlFlags(pCreateInfos[i].flags, i, "vkCreateRayTracingPipelinesKHR",
                                                  "VUID-VkRayTracingPipelineCreateInfoKHR-pipelineCreationCacheControl-02905");
        const auto create_info = pipeline->GetCreateInfo<VkRayTracingPipelineCreateInfoKHR>();
        if (create_info.pLibraryInfo) {
            constexpr std::array<std::pair<const char *, VkPipelineCreateFlags>, 7> vuid_map = {{
                {"VUID-VkRayTracingPipelineCreateInfoKHR-flags-04718", VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR},
                {"VUID-VkRayTracingPipelineCreateInfoKHR-flags-04719", VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR},
                {"VUID-VkRayTracingPipelineCreateInfoKHR-flags-04720",
                 VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR},
                {"VUID-VkRayTracingPipelineCreateInfoKHR-flags-04721",
                 VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR},
                {"VUID-VkRayTracingPipelineCreateInfoKHR-flags-04722",
                 VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR},
                {"VUID-VkRayTracingPipelineCreateInfoKHR-flags-04723", VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR},
                {"VUID-VkRayTracingPipelineCreateInfoKHR-flags-07403", VK_PIPELINE_CREATE_RAY_TRACING_OPACITY_MICROMAP_BIT_EXT},
            }};
            unsigned int descriptor_buffer_library_count = 0;
            for (uint32_t j = 0; j < create_info.pLibraryInfo->libraryCount; ++j) {
                const auto lib = Get<PIPELINE_STATE>(create_info.pLibraryInfo->pLibraries[j]);
                if ((lib->GetPipelineCreateFlags() & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) == 0) {
                    skip |= LogError(device, "VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-03381",
                                     "vkCreateRayTracingPipelinesKHR(): pCreateInfo[%" PRIu32 "].pLibraryInfo->pLibraries[%" PRIu32
                                     "] was not created with VK_PIPELINE_CREATE_LIBRARY_BIT_KHR.",
                                     i, j);
                }
                if (lib->descriptor_buffer_mode) {
                    ++descriptor_buffer_library_count;
                }
                for (const auto &pair : vuid_map) {
                    if (pipeline->GetPipelineCreateFlags() & pair.second) {
                        if ((lib->GetPipelineCreateFlags() & pair.second) == 0) {
                            skip |= LogError(device, pair.first,
                                             "vkCreateRayTracingPipelinesKHR(): pCreateInfo[%" PRIu32
                                             "].flags contains %s bit, but pCreateInfo[%" PRIu32
                                             "].pLibraryInfo->pLibraries[%" PRIu32 "] was created without it.",
                                             i, string_VkPipelineCreateFlags(pair.second).c_str(), i, j);
                        }
                    }
                }
            }
            if ((descriptor_buffer_library_count != 0) &&
                (create_info.pLibraryInfo->libraryCount != descriptor_buffer_library_count)) {
                skip |= LogError(device, "VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-08096",
                                 "vkCreateRayTracingPipelinesKHR(): All or none of the elements of pCreateInfo[%" PRIu32
                                 "].pLibraryInfo->pLibraries must be created "
                                 "with VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT.",
                                 i);
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetPipelineExecutablePropertiesKHR(VkDevice device, const VkPipelineInfoKHR *pPipelineInfo,
                                                                   uint32_t *pExecutableCount,
                                                                   VkPipelineExecutablePropertiesKHR *pProperties) const {
    bool skip = false;
    skip |= ValidatePipelineExecutableInfo(device, nullptr, "vkGetPipelineExecutablePropertiesKHR",
                                           "VUID-vkGetPipelineExecutablePropertiesKHR-pipelineExecutableInfo-03270");
    return skip;
}

bool CoreChecks::ValidatePipelineExecutableInfo(VkDevice device, const VkPipelineExecutableInfoKHR *pExecutableInfo,
                                                const char *caller_name, const char *feature_vuid) const {
    bool skip = false;

    if (!enabled_features.pipeline_exe_props_features.pipelineExecutableInfo) {
        skip |= LogError(device, feature_vuid, "%s(): called when pipelineExecutableInfo feature is not enabled.", caller_name);
    }

    // vkGetPipelineExecutablePropertiesKHR will not have struct to validate further
    if (pExecutableInfo) {
        auto pi = LvlInitStruct<VkPipelineInfoKHR>();
        pi.pipeline = pExecutableInfo->pipeline;

        // We could probably cache this instead of fetching it every time
        uint32_t executable_count = 0;
        DispatchGetPipelineExecutablePropertiesKHR(device, &pi, &executable_count, NULL);

        if (pExecutableInfo->executableIndex >= executable_count) {
            skip |= LogError(
                pExecutableInfo->pipeline, "VUID-VkPipelineExecutableInfoKHR-executableIndex-03275",
                "%s(): VkPipelineExecutableInfo::executableIndex (%1u) must be less than the number of executables associated with "
                "the pipeline (%1u) as returned by vkGetPipelineExecutablePropertiessKHR",
                caller_name, pExecutableInfo->executableIndex, executable_count);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetPipelineExecutableStatisticsKHR(VkDevice device,
                                                                   const VkPipelineExecutableInfoKHR *pExecutableInfo,
                                                                   uint32_t *pStatisticCount,
                                                                   VkPipelineExecutableStatisticKHR *pStatistics) const {
    bool skip = false;
    skip |= ValidatePipelineExecutableInfo(device, pExecutableInfo, "vkGetPipelineExecutableStatisticsKHR",
                                           "VUID-vkGetPipelineExecutableStatisticsKHR-pipelineExecutableInfo-03272");

    auto pipeline_state = Get<PIPELINE_STATE>(pExecutableInfo->pipeline);
    if (!(pipeline_state->GetPipelineCreateFlags() & VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR)) {
        skip |= LogError(pExecutableInfo->pipeline, "VUID-vkGetPipelineExecutableStatisticsKHR-pipeline-03274",
                         "vkGetPipelineExecutableStatisticsKHR called on a pipeline created without the "
                         "VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR flag set");
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device, const VkPipelineExecutableInfoKHR *pExecutableInfo, uint32_t *pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR *pStatistics) const {
    bool skip = false;
    skip |= ValidatePipelineExecutableInfo(device, pExecutableInfo, "vkGetPipelineExecutableInternalRepresentationsKHR",
                                           "VUID-vkGetPipelineExecutableInternalRepresentationsKHR-pipelineExecutableInfo-03276");

    auto pipeline_state = Get<PIPELINE_STATE>(pExecutableInfo->pipeline);
    if (!(pipeline_state->GetPipelineCreateFlags() & VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR)) {
        skip |= LogError(pExecutableInfo->pipeline, "VUID-vkGetPipelineExecutableInternalRepresentationsKHR-pipeline-03278",
                         "vkGetPipelineExecutableInternalRepresentationsKHR called on a pipeline created without the "
                         "VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR flag set");
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyPipeline(VkDevice device, VkPipeline pipeline,
                                                const VkAllocationCallbacks *pAllocator) const {
    auto pipeline_state = Get<PIPELINE_STATE>(pipeline);
    bool skip = false;
    if (pipeline_state) {
        skip |= ValidateObjectNotInUse(pipeline_state.get(), "vkDestroyPipeline", "VUID-vkDestroyPipeline-pipeline-00765");
    }
    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineBlendEnable(const PIPELINE_STATE &pipeline) const {
    bool skip = false;
    const auto *color_blend_state = pipeline.ColorBlendState();
    const auto &rp_state = pipeline.RenderPassState();
    if (rp_state && color_blend_state) {
        const auto subpass = pipeline.Subpass();
        const auto *subpass_desc = &rp_state->createInfo.pSubpasses[subpass];

        uint32_t numberColorAttachments = rp_state->UsesDynamicRendering()
                                              ? rp_state->dynamic_rendering_pipeline_create_info.colorAttachmentCount
                                              : subpass_desc->colorAttachmentCount;

        for (uint32_t i = 0; i < pipeline.Attachments().size() && i < numberColorAttachments; ++i) {
            VkFormatFeatureFlags2KHR format_features;

            if (rp_state->UsesDynamicRendering()) {
                if (color_blend_state->attachmentCount != numberColorAttachments) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06055",
                                     "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                     "] VkPipelineRenderingCreateInfoKHR::colorAttachmentCount (%" PRIu32
                                     ") must equal pColorBlendState->attachmentCount (%" PRIu32 ")",
                                     pipeline.create_index, numberColorAttachments, color_blend_state->attachmentCount);
                }
            } else {
                const auto attachment = subpass_desc->pColorAttachments[i].attachment;
                if (attachment == VK_ATTACHMENT_UNUSED) continue;

                const auto attachment_desc = rp_state->createInfo.pAttachments[attachment];
                format_features = GetPotentialFormatFeatures(attachment_desc.format);

                const auto *raster_state = pipeline.RasterizationState();
                if (raster_state && !raster_state->rasterizerDiscardEnable && pipeline.Attachments()[i].blendEnable &&
                    !(format_features & VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BLEND_BIT_KHR)) {
                    skip |= LogError(
                        device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06041",
                        "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32 "]->pColorBlendState.pAttachments[%" PRIu32
                        "].blendEnable is VK_TRUE but format %s of the corresponding attachment description (subpass %" PRIu32
                        ", attachment %" PRIu32 ") does not support VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT.",
                        pipeline.create_index, i, string_VkFormat(attachment_desc.format), subpass, attachment);
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateGraphicsPipelinePreRasterState(const PIPELINE_STATE &pipeline) const {
    bool skip = false;
    if (pipeline.pre_raster_state) {
        const uint32_t active_shaders = pipeline.active_shaders;
        if (pipeline.pre_raster_state && ((active_shaders & PreRasterState::ValidShaderStages()) == 0)) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pStages-06896",
                             "vkCreateGraphicsPipelines(): pCreateInfo[%" PRIu32
                             "] contains pre-raster state, but stages (%s) does not contain any pre-raster shaders.",
                             pipeline.create_index, string_VkShaderStageFlags(active_shaders).c_str());
        }

        if (!enabled_features.core.geometryShader && (active_shaders & VK_SHADER_STAGE_GEOMETRY_BIT)) {
            skip |= LogError(device, "VUID-VkPipelineShaderStageCreateInfo-stage-00704",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32 "] State: Geometry Shader not supported.",
                             pipeline.create_index);
        }
        if (!enabled_features.core.tessellationShader && (active_shaders & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT ||
                                                          active_shaders & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)) {
            skip |= LogError(device, "VUID-VkPipelineShaderStageCreateInfo-stage-00705",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32 "] State: Tessellation Shader not supported.",
                             pipeline.create_index);
        }
        if (IsExtEnabled(device_extensions.vk_nv_mesh_shader) || IsExtEnabled(device_extensions.vk_ext_mesh_shader)) {
            // VS or mesh is required
            if (!(active_shaders & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_MESH_BIT_NV))) {
                skip |=
                    LogError(device, "VUID-VkGraphicsPipelineCreateInfo-stage-02096",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32 "] State: Vertex Shader or Mesh Shader required.",
                             pipeline.create_index);
            }
            // Can't mix mesh and VTG
            if ((active_shaders & (VK_SHADER_STAGE_MESH_BIT_NV | VK_SHADER_STAGE_TASK_BIT_NV)) &&
                (active_shaders & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT |
                                   VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT))) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pStages-02095",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "] State: Geometric shader stages must either be all mesh (mesh | task) "
                                 "or all VTG (vertex, tess control, tess eval, geom).",
                                 pipeline.create_index);
            }
        } else if (IsExtEnabled(device_extensions.vk_ext_mesh_shader)) {
            // VS or mesh is required
            if (!(active_shaders & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_MESH_BIT_EXT))) {
                skip |=
                    LogError(device, "VUID-VkGraphicsPipelineCreateInfo-stage-02096",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32 "] State: Vertex Shader or Mesh Shader required.",
                             pipeline.create_index);
            }
            // Can't mix mesh and VTG
            if ((active_shaders & (VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT)) &&
                (active_shaders & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT |
                                   VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT))) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pStages-02095",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "] State: Geometric shader stages must either be all mesh (mesh | task) "
                                 "or all VTG (vertex, tess control, tess eval, geom).",
                                 pipeline.create_index);
            }
        } else if (!(active_shaders & VK_SHADER_STAGE_VERTEX_BIT)) {
            // VS is required if this is a "normal" pipeline or is a pre-raster graphics library
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-stage-00727",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32 "] State: Vertex Shader required.",
                             pipeline.create_index);
        }

        // VK_SHADER_STAGE_MESH_BIT_EXT and VK_SHADER_STAGE_MESH_BIT_NV are equivalent
        if (!(enabled_features.mesh_shader_features.meshShader) && (active_shaders & VK_SHADER_STAGE_MESH_BIT_EXT)) {
            skip |= LogError(device, "VUID-VkPipelineShaderStageCreateInfo-stage-02091",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32 "] State: Mesh Shader feature is not enabled.",
                             pipeline.create_index);
        }

        // VK_SHADER_STAGE_TASK_BIT_EXT and VK_SHADER_STAGE_TASK_BIT_NV are equivalent
        if (!(enabled_features.mesh_shader_features.taskShader) && (active_shaders & VK_SHADER_STAGE_TASK_BIT_EXT)) {
            skip |= LogError(device, "VUID-VkPipelineShaderStageCreateInfo-stage-02092",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32 "] State: Task Shader feature is not enabled.",
                             pipeline.create_index);
        }

        // Either both or neither TC/TE shaders should be defined
        const bool has_control = (active_shaders & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) != 0;
        const bool has_eval = (active_shaders & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) != 0;
        if (has_control && !has_eval) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pStages-00729",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                             "] State: TE and TC shaders must be included or excluded as a pair.",
                             pipeline.create_index);
        }
        if (!has_control && has_eval) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pStages-00730",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                             "] State: TE and TC shaders must be included or excluded as a pair.",
                             pipeline.create_index);
        }

        const auto *ia_state = pipeline.InputAssemblyState();
        // VK_PRIMITIVE_TOPOLOGY_PATCH_LIST primitive topology is only valid for tessellation pipelines.
        // Mismatching primitive topology and tessellation fails graphics pipeline creation.
        // NOTE: For GPL, vertex input state must be present to test this
        if (has_control && has_eval && pipeline.vertex_input_state && pipeline.pre_raster_state &&
            (!ia_state || ia_state->topology != VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pStages-00736",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                             "] State: VK_PRIMITIVE_TOPOLOGY_PATCH_LIST must be set as IA topology for "
                             "tessellation pipelines.",
                             pipeline.create_index);
        }
        if (ia_state) {
            if (ia_state->topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST) {
                if (!has_control || !has_eval) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-topology-00737",
                                     "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                     "] State: VK_PRIMITIVE_TOPOLOGY_PATCH_LIST primitive topology is only valid "
                                     "for tessellation pipelines.",
                                     pipeline.create_index);
                }
            }

            if ((ia_state->primitiveRestartEnable == VK_TRUE) &&
                (ia_state->topology == VK_PRIMITIVE_TOPOLOGY_POINT_LIST || ia_state->topology == VK_PRIMITIVE_TOPOLOGY_LINE_LIST ||
                 ia_state->topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST ||
                 ia_state->topology == VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY ||
                 ia_state->topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY ||
                 ia_state->topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)) {
                if (IsExtEnabled(device_extensions.vk_ext_primitive_topology_list_restart)) {
                    if (ia_state->topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST) {
                        if (!enabled_features.primitive_topology_list_restart_features.primitiveTopologyPatchListRestart) {
                            skip |= LogError(device, "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-06253",
                                             "vkCreateGraphicsPipelines() pCreateInfo[%" PRIu32
                                             "]: topology is %s and primitiveRestartEnable is VK_TRUE and the "
                                             "primitiveTopologyPatchListRestart feature is not enabled.",
                                             pipeline.create_index, string_VkPrimitiveTopology(ia_state->topology));
                        }
                    } else if (!enabled_features.primitive_topology_list_restart_features.primitiveTopologyListRestart) {
                        skip |= LogError(
                            device, "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-06252",
                            "vkCreateGraphicsPipelines() pCreateInfo[%" PRIu32
                            "]: topology is %s and primitiveRestartEnable is VK_TRUE and the primitiveTopologyListRestart feature "
                            "is not enabled.",
                            pipeline.create_index, string_VkPrimitiveTopology(ia_state->topology));
                    }
                } else {
                    skip |= LogError(device, "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00428",
                                     "vkCreateGraphicsPipelines() pCreateInfo[%" PRIu32
                                     "]: topology is %s and primitiveRestartEnable is VK_TRUE. It is invalid.",
                                     pipeline.create_index, string_VkPrimitiveTopology(ia_state->topology));
                }
            }
            if ((enabled_features.core.geometryShader == VK_FALSE) &&
                (ia_state->topology == VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY ||
                 ia_state->topology == VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY ||
                 ia_state->topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY ||
                 ia_state->topology == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY)) {
                skip |= LogError(device, "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00429",
                                 "vkCreateGraphicsPipelines() pCreateInfo[%" PRIu32
                                 "]: topology is %s and geometry shaders feature is not enabled. "
                                 "It is invalid.",
                                 pipeline.create_index, string_VkPrimitiveTopology(ia_state->topology));
            }
            if ((enabled_features.core.tessellationShader == VK_FALSE) &&
                (ia_state->topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)) {
                skip |= LogError(device, "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00430",
                                 "vkCreateGraphicsPipelines() pCreateInfo[%" PRIu32
                                 "]: topology is %s and tessellation shaders feature is not "
                                 "enabled. It is invalid.",
                                 pipeline.create_index, string_VkPrimitiveTopology(ia_state->topology));
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineColorBlendState(const PIPELINE_STATE &pipeline,
                                                         const safe_VkSubpassDescription2 *subpass_desc) const {
    bool skip = false;
    const auto color_blend_state = pipeline.ColorBlendState();
    if (color_blend_state) {
        const auto &rp_state = pipeline.RenderPassState();
        if (((color_blend_state->flags & VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_BIT_ARM) !=
             0) &&
            (!rp_state || rp_state->renderPass() == VK_NULL_HANDLE)) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-flags-06482",
                             "vkCreateGraphicsPipelines() pCreateInfo[%" PRIu32
                             "]: created with"
                             "VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_BIT_ARM,"
                             "renderpass must not be VK_NULL_HANDLE.",
                             pipeline.create_index);
        }

        // VkAttachmentSampleCountInfoAMD == VkAttachmentSampleCountInfoNV
        auto attachment_sample_count_info = LvlFindInChain<VkAttachmentSampleCountInfoAMD>(pipeline.PNext());
        const auto rendering_struct = LvlFindInChain<VkPipelineRenderingCreateInfo>(pipeline.PNext());
        if (rendering_struct && attachment_sample_count_info &&
            (attachment_sample_count_info->colorAttachmentCount != rendering_struct->colorAttachmentCount)) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06063",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                             "] interface VkAttachmentSampleCountInfo->attachmentCount %" PRIu32
                             " and "
                             "VkPipelineRenderingCreateInfoKHR->colorAttachmentCount %" PRIu32 " must be equal",
                             pipeline.create_index, attachment_sample_count_info->colorAttachmentCount,
                             rendering_struct->colorAttachmentCount);
        }

        if (subpass_desc && color_blend_state->attachmentCount != subpass_desc->colorAttachmentCount) {
            const char *vuid = IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state3)
                                   ? "VUID-VkGraphicsPipelineCreateInfo-renderPass-07609"
                                   : "VUID-VkGraphicsPipelineCreateInfo-renderPass-06042";
            skip |= LogError(device, vuid,
                             "vkCreateGraphicsPipelines() pCreateInfo[%" PRIu32
                             "]: %s subpass %u has colorAttachmentCount of %u which doesn't "
                             "match the pColorBlendState->attachmentCount of %u.",
                             pipeline.create_index, report_data->FormatHandle(rp_state->renderPass()).c_str(), pipeline.Subpass(),
                             subpass_desc->colorAttachmentCount, color_blend_state->attachmentCount);
        }
        const auto &pipe_attachments = pipeline.Attachments();
        if (!enabled_features.core.independentBlend) {
            if (pipe_attachments.size() > 1) {
                const auto *const attachments = &pipe_attachments[0];
                for (size_t i = 1; i < pipe_attachments.size(); i++) {
                    // Quoting the spec: "If [the independent blend] feature is not enabled, the VkPipelineColorBlendAttachmentState
                    // settings for all color attachments must be identical." VkPipelineColorBlendAttachmentState contains
                    // only attachment state, so memcmp is best suited for the comparison
                    if (memcmp(static_cast<const void *>(attachments), static_cast<const void *>(&attachments[i]),
                               sizeof(attachments[0]))) {
                        skip |= LogError(device, "VUID-VkPipelineColorBlendStateCreateInfo-pAttachments-00605",
                                         "vkCreateGraphicsPipelines() pCreateInfo[%" PRIu32
                                         "]: If independent blend feature not enabled, all elements of "
                                         "pAttachments must be identical.",
                                         pipeline.create_index);
                        break;
                    }
                }
            }
        }
        if (!enabled_features.core.logicOp && (color_blend_state->logicOpEnable != VK_FALSE)) {
            skip |= LogError(device, "VUID-VkPipelineColorBlendStateCreateInfo-logicOpEnable-00606",
                             "vkCreateGraphicsPipelines() pCreateInfo[%" PRIu32
                             "]: If logic operations feature not enabled, logicOpEnable must be VK_FALSE.",
                             pipeline.create_index);
        }
        for (size_t i = 0; i < pipe_attachments.size(); i++) {
            if (IsSecondaryColorInputBlendFactor(pipe_attachments[i].srcColorBlendFactor)) {
                if (!enabled_features.core.dualSrcBlend) {
                    skip |= LogError(device, "VUID-VkPipelineColorBlendAttachmentState-srcColorBlendFactor-00608",
                                     "vkCreateGraphicsPipelines(): pCreateInfo[%" PRIu32
                                     "].pColorBlendState.pAttachments[%zu]"
                                     ".srcColorBlendFactor uses a dual-source blend factor (%d), but this device feature is not "
                                     "enabled.",
                                     pipeline.create_index, i, pipe_attachments[i].srcColorBlendFactor);
                }
            }
            if (IsSecondaryColorInputBlendFactor(pipe_attachments[i].dstColorBlendFactor)) {
                if (!enabled_features.core.dualSrcBlend) {
                    skip |= LogError(device, "VUID-VkPipelineColorBlendAttachmentState-dstColorBlendFactor-00609",
                                     "vkCreateGraphicsPipelines(): pCreateInfo[%" PRIu32
                                     "].pColorBlendState.pAttachments[%zu]"
                                     ".dstColorBlendFactor uses a dual-source blend factor (%d), but this device feature is not "
                                     "enabled.",
                                     pipeline.create_index, i, pipe_attachments[i].dstColorBlendFactor);
                }
            }
            if (IsSecondaryColorInputBlendFactor(pipe_attachments[i].srcAlphaBlendFactor)) {
                if (!enabled_features.core.dualSrcBlend) {
                    skip |= LogError(device, "VUID-VkPipelineColorBlendAttachmentState-srcAlphaBlendFactor-00610",
                                     "vkCreateGraphicsPipelines(): pCreateInfo[%" PRIu32
                                     "].pColorBlendState.pAttachments[%zu]"
                                     ".srcAlphaBlendFactor uses a dual-source blend factor (%d), but this device feature is not "
                                     "enabled.",
                                     pipeline.create_index, i, pipe_attachments[i].srcAlphaBlendFactor);
                }
            }
            if (IsSecondaryColorInputBlendFactor(pipe_attachments[i].dstAlphaBlendFactor)) {
                if (!enabled_features.core.dualSrcBlend) {
                    skip |= LogError(device, "VUID-VkPipelineColorBlendAttachmentState-dstAlphaBlendFactor-00611",
                                     "vkCreateGraphicsPipelines(): pCreateInfo[%" PRIu32
                                     "].pColorBlendState.pAttachments[%zu]"
                                     ".dstAlphaBlendFactor uses a dual-source blend factor (%d), but this device feature is not "
                                     "enabled.",
                                     pipeline.create_index, i, pipe_attachments[i].dstAlphaBlendFactor);
                }
            }
        }
        auto color_write = LvlFindInChain<VkPipelineColorWriteCreateInfoEXT>(color_blend_state->pNext);
        if (color_write) {
            if (color_write->attachmentCount > phys_dev_props.limits.maxColorAttachments) {
                skip |= LogError(
                    device, "VUID-VkPipelineColorWriteCreateInfoEXT-attachmentCount-06655",
                    "vkCreateGraphicsPipelines(): VkPipelineColorWriteCreateInfoEXT in the pNext chain of pCreateInfo[%" PRIu32
                    "].pColorBlendState has an attachmentCount of (%" PRIu32
                    ") which is greater than the VkPhysicalDeviceLimits::maxColorAttachments limit (%" PRIu32 ").",
                    pipeline.create_index, color_write->attachmentCount, phys_dev_props.limits.maxColorAttachments);
            }
            if (!enabled_features.color_write_features.colorWriteEnable) {
                for (uint32_t i = 0; i < color_write->attachmentCount; ++i) {
                    if (color_write->pColorWriteEnables[i] != VK_TRUE) {
                        skip |= LogError(device, "VUID-VkPipelineColorWriteCreateInfoEXT-pAttachments-04801",
                                         "vkCreateGraphicsPipelines(): pCreateInfo[%" PRIu32
                                         "].pColorBlendState pNext chain includes VkPipelineColorWriteCreateInfoEXT with "
                                         "pColorWriteEnables[%" PRIu32 "] = VK_FALSE, but colorWriteEnable is not enabled.",
                                         pipeline.create_index, i);
                    }
                }
            }
        }
        const auto *color_blend_advanced = LvlFindInChain<VkPipelineColorBlendAdvancedStateCreateInfoEXT>(color_blend_state->pNext);
        if (color_blend_advanced) {
            if (!phys_dev_ext_props.blend_operation_advanced_props.advancedBlendCorrelatedOverlap &&
                color_blend_advanced->blendOverlap != VK_BLEND_OVERLAP_UNCORRELATED_EXT) {
                skip |= LogError(
                    device, "VUID-VkPipelineColorBlendAdvancedStateCreateInfoEXT-blendOverlap-01426",
                    "vkCreateGraphicsPipelines(): pCreateInfo[%" PRIu32
                    "].pColorBlendState pNext chain contains VkPipelineColorBlendAdvancedStateCreateInfoEXT structure with "
                    "blendOverlap equal to %s, but "
                    "VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT::advancedBlendCorrelatedOverlap is not supported.",
                    pipeline.create_index, string_VkBlendOverlapEXT(color_blend_advanced->blendOverlap));
            }
            if (!phys_dev_ext_props.blend_operation_advanced_props.advancedBlendNonPremultipliedDstColor &&
                color_blend_advanced->dstPremultiplied != VK_TRUE) {
                skip |= LogError(
                    device, "VUID-VkPipelineColorBlendAdvancedStateCreateInfoEXT-dstPremultiplied-01425",
                    "vkCreateGraphicsPipelines(): pCreateInfo[%" PRIu32
                    "].pColorBlendState pNext chain contains VkPipelineColorBlendAdvancedStateCreateInfoEXT structure with "
                    "dstPremultiplied equal to VK_FALSE, but "
                    "VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT::advancedBlendNonPremultipliedDstColor is not supported.",
                    pipeline.create_index);
            }
            if (!phys_dev_ext_props.blend_operation_advanced_props.advancedBlendNonPremultipliedSrcColor &&
                color_blend_advanced->srcPremultiplied != VK_TRUE) {
                skip |= LogError(
                    device, "VUID-VkPipelineColorBlendAdvancedStateCreateInfoEXT-srcPremultiplied-01424",
                    "vkCreateGraphicsPipelines(): pCreateInfo[%" PRIu32
                    "].pColorBlendState pNext chain contains VkPipelineColorBlendAdvancedStateCreateInfoEXT structure with "
                    "srcPremultiplied equal to VK_FALSE, but "
                    "VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT::advancedBlendNonPremultipliedSrcColor is not supported.",
                    pipeline.create_index);
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineRasterizationState(const PIPELINE_STATE &pipeline,
                                                            const safe_VkSubpassDescription2 *subpass_desc) const {
    bool skip = false;
    const auto raster_state = pipeline.RasterizationState();
    if (raster_state) {
        if ((raster_state->depthClampEnable == VK_TRUE) && (!enabled_features.core.depthClamp)) {
            skip |= LogError(device, "VUID-VkPipelineRasterizationStateCreateInfo-depthClampEnable-00782",
                             "vkCreateGraphicsPipelines() pCreateInfo[%" PRIu32
                             "]: the depthClamp device feature is disabled: the "
                             "depthClampEnable member "
                             "of the VkPipelineRasterizationStateCreateInfo structure must be set to VK_FALSE.",
                             pipeline.create_index);
        }

        if (!pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_BIAS) && (raster_state->depthBiasClamp != 0.0) &&
            (!enabled_features.core.depthBiasClamp)) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00754",
                             "vkCreateGraphicsPipelines() pCreateInfo[%" PRIu32
                             "]: the depthBiasClamp device feature is disabled: the "
                             "depthBiasClamp member "
                             "of the VkPipelineRasterizationStateCreateInfo structure must be set to 0.0 unless the "
                             "VK_DYNAMIC_STATE_DEPTH_BIAS dynamic state is enabled",
                             pipeline.create_index);
        }

        // If rasterization is enabled...
        if (raster_state->rasterizerDiscardEnable == VK_FALSE) {
            // pMultisampleState can be null for graphics library
            const bool has_ms_state =
                !pipeline.IsGraphicsLibrary() ||
                ((pipeline.graphics_lib_type & (VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
                                                VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT)) != 0);
            if (has_ms_state) {
                const auto ms_state = pipeline.MultisampleState();
                if (ms_state && (ms_state->alphaToOneEnable == VK_TRUE) && (!enabled_features.core.alphaToOne)) {
                    skip |= LogError(device, "VUID-VkPipelineMultisampleStateCreateInfo-alphaToOneEnable-00785",
                                     "vkCreateGraphicsPipelines() pCreateInfo[%" PRIu32
                                     "]: the alphaToOne device feature is disabled: the alphaToOneEnable "
                                     "member of the VkPipelineMultisampleStateCreateInfo structure must be set to VK_FALSE.",
                                     pipeline.create_index);
                }
            }

            // If subpass uses a depth/stencil attachment, pDepthStencilState must be a pointer to a valid structure
            const bool has_ds_state = (!pipeline.IsGraphicsLibrary() && pipeline.HasFullState()) ||
                                      ((pipeline.graphics_lib_type & (VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT)) != 0);
            if (has_ds_state) {
                if (subpass_desc && subpass_desc->pDepthStencilAttachment &&
                    subpass_desc->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
                    const auto ds_state = pipeline.DepthStencilState();
                    if (!ds_state) {
                        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06043",
                                         "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                         "] State: pDepthStencilState is NULL when rasterization is enabled "
                                         "and subpass uses a depth/stencil attachment.",
                                         pipeline.create_index);
                    } else if (ds_state->depthBoundsTestEnable == VK_TRUE) {
                        if (!enabled_features.core.depthBounds) {
                            skip |= LogError(
                                device, "VUID-VkPipelineDepthStencilStateCreateInfo-depthBoundsTestEnable-00598",
                                "vkCreateGraphicsPipelines() pCreateInfo[%" PRIu32
                                "]: the depthBounds device feature is disabled: the "
                                "depthBoundsTestEnable member of the VkPipelineDepthStencilStateCreateInfo structure must be "
                                "set to VK_FALSE.",
                                pipeline.create_index);
                        }

                        // The extension was not created with a feature bit whichs prevents displaying the 2 variations of the VUIDs
                        if (!IsExtEnabled(device_extensions.vk_ext_depth_range_unrestricted) &&
                            !pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_BOUNDS)) {
                            const float minDepthBounds = ds_state->minDepthBounds;
                            const float maxDepthBounds = ds_state->maxDepthBounds;
                            // Also VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00755
                            if (!(minDepthBounds >= 0.0) || !(minDepthBounds <= 1.0)) {
                                skip |= LogError(
                                    device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-02510",
                                    "vkCreateGraphicsPipelines() pCreateInfo[%" PRIu32
                                    "]: VK_EXT_depth_range_unrestricted extension "
                                    "is not enabled, VK_DYNAMIC_STATE_DEPTH_BOUNDS is not used, depthBoundsTestEnable is "
                                    "true, and pDepthStencilState::minDepthBounds (=%f) is not within the [0.0, 1.0] range.",
                                    pipeline.create_index, minDepthBounds);
                            }
                            if (!(maxDepthBounds >= 0.0) || !(maxDepthBounds <= 1.0)) {
                                skip |= LogError(
                                    device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-02510",
                                    "vkCreateGraphicsPipelines() pCreateInfo[%" PRIu32
                                    "]: VK_EXT_depth_range_unrestricted extension "
                                    "is not enabled, VK_DYNAMIC_STATE_DEPTH_BOUNDS is not used, depthBoundsTestEnable is "
                                    "true, and pDepthStencilState::maxDepthBounds (=%f) is not within the [0.0, 1.0] range.",
                                    pipeline.create_index, maxDepthBounds);
                            }
                        }
                    }
                }
            }

            // If subpass uses color attachments, pColorBlendState must be valid pointer
            const bool has_color_blend_state =
                !pipeline.IsGraphicsLibrary() ||
                ((pipeline.graphics_lib_type & (VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT)) != 0);
            if (has_color_blend_state && subpass_desc) {
                uint32_t color_attachment_count = 0;
                for (uint32_t i = 0; i < subpass_desc->colorAttachmentCount; ++i) {
                    if (subpass_desc->pColorAttachments[i].attachment != VK_ATTACHMENT_UNUSED) {
                        ++color_attachment_count;
                    }
                }

                if (pipeline.fragment_output_state && (color_attachment_count > 0) &&
                    !pipeline.fragment_output_state->color_blend_state) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06044",
                                     "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                     "] State: pColorBlendState is NULL when rasterization is enabled and "
                                     "subpass uses color attachments.",
                                     pipeline.create_index);
                }

                const uint32_t active_shaders = pipeline.active_shaders;
                if (GetBitSetCount(subpass_desc->viewMask) > 1) {
                    if (!enabled_features.core11.multiviewTessellationShader &&
                        (active_shaders & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT ||
                         active_shaders & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)) {
                        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06047",
                                         "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32 "] State: subpass (%" PRIx32
                                         ") has more than 1 bit set in viewMask and pStages includes tessellation shaders, but the "
                                         "VkPhysicalDeviceMultiviewFeatures::multiviewTessellationShader features is not enabled.",
                                         pipeline.create_index, subpass_desc->viewMask);
                    }
                    if (!enabled_features.core11.multiviewGeometryShader && active_shaders & VK_SHADER_STAGE_GEOMETRY_BIT) {
                        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06048",
                                         "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32 "] State:subpass (%" PRIx32
                                         ") has more than 1 bit set in viewMask and pStages includes geometry shader, but the "
                                         "VkPhysicalDeviceMultiviewFeatures::multiviewGeometryShader features is not enabled.",
                                         pipeline.create_index, subpass_desc->viewMask);
                    }
                }
            }
        }

        auto provoking_vertex_state_ci =
            LvlFindInChain<VkPipelineRasterizationProvokingVertexStateCreateInfoEXT>(raster_state->pNext);
        if (provoking_vertex_state_ci &&
            provoking_vertex_state_ci->provokingVertexMode == VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT &&
            !enabled_features.provoking_vertex_features.provokingVertexLast) {
            skip |= LogError(device, "VUID-VkPipelineRasterizationProvokingVertexStateCreateInfoEXT-provokingVertexMode-04883",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32 "] - provokingVertexLast feature is not enabled.",
                             pipeline.create_index);
        }

        const auto rasterization_state_stream_ci =
            LvlFindInChain<VkPipelineRasterizationStateStreamCreateInfoEXT>(raster_state->pNext);
        if (rasterization_state_stream_ci) {
            if (!enabled_features.transform_feedback_features.geometryStreams) {
                skip |= LogError(device, "VUID-VkPipelineRasterizationStateStreamCreateInfoEXT-geometryStreams-02324",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "].pRasterizationState pNext chain includes VkPipelineRasterizationStateStreamCreateInfoEXT, but "
                                 "geometryStreams feature is not enabled.",
                                 pipeline.create_index);
            } else if (phys_dev_ext_props.transform_feedback_props.transformFeedbackRasterizationStreamSelect == VK_FALSE &&
                       rasterization_state_stream_ci->rasterizationStream != 0) {
                skip |= LogError(device, "VUID-VkPipelineRasterizationStateStreamCreateInfoEXT-rasterizationStream-02326",
                                 "vkCreateGraphicsPipelines(): "
                                 "VkPhysicalDeviceTransformFeedbackPropertiesEXT::transformFeedbackRasterizationStreamSelect is "
                                 "VK_FALSE, but pCreateInfos[%" PRIu32
                                 "].pRasterizationState pNext chain includes VkPipelineRasterizationStateStreamCreateInfoEXT with "
                                 "rasterizationStream (%" PRIu32 ") not equal to 0.",
                                 pipeline.create_index, rasterization_state_stream_ci->rasterizationStream);
            } else if (rasterization_state_stream_ci->rasterizationStream >=
                       phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams) {
                skip |= LogError(
                    device, "VUID-VkPipelineRasterizationStateStreamCreateInfoEXT-rasterizationStream-02325",
                    "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                    "].pRasterizationState pNext chain includes VkPipelineRasterizationStateStreamCreateInfoEXT with "
                    "rasterizationStream (%" PRIu32
                    ") not less than VkPhysicalDeviceTransformFeedbackPropertiesEXT::maxTransformFeedbackStreams (%" PRIu32 ").",
                    pipeline.create_index, rasterization_state_stream_ci->rasterizationStream,
                    phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams);
            }
        }

        const auto rasterization_conservative_state_ci =
            LvlFindInChain<VkPipelineRasterizationConservativeStateCreateInfoEXT>(raster_state->pNext);
        if (rasterization_conservative_state_ci) {
            if (rasterization_conservative_state_ci->extraPrimitiveOverestimationSize < 0.0f ||
                rasterization_conservative_state_ci->extraPrimitiveOverestimationSize >
                    phys_dev_ext_props.conservative_rasterization_props.maxExtraPrimitiveOverestimationSize) {
                skip |= LogError(
                    device, "VUID-VkPipelineRasterizationConservativeStateCreateInfoEXT-extraPrimitiveOverestimationSize-01769",
                    "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                    "].pRasterizationState pNext chain includes VkPipelineRasterizationConservativeStateCreateInfoEXT with "
                    "extraPrimitiveOverestimationSize (%f), which is not between 0.0 and "
                    "VkPipelineRasterizationConservativeStateCreateInfoEXT::maxExtraPrimitiveOverestimationSize (%f).",
                    pipeline.create_index, rasterization_conservative_state_ci->extraPrimitiveOverestimationSize,
                    phys_dev_ext_props.conservative_rasterization_props.maxExtraPrimitiveOverestimationSize);
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateSampleLocationsInfo(const VkSampleLocationsInfoEXT *pSampleLocationsInfo, const char *apiName) const {
    bool skip = false;
    const VkSampleCountFlagBits sample_count = pSampleLocationsInfo->sampleLocationsPerPixel;
    const uint32_t sample_total_size = pSampleLocationsInfo->sampleLocationGridSize.width *
                                       pSampleLocationsInfo->sampleLocationGridSize.height * SampleCountSize(sample_count);
    if (pSampleLocationsInfo->sampleLocationsCount != sample_total_size) {
        skip |= LogError(device, "VUID-VkSampleLocationsInfoEXT-sampleLocationsCount-01527",
                         "%s: VkSampleLocationsInfoEXT::sampleLocationsCount (%u) must equal grid width * grid height * pixel "
                         "sample rate which currently is (%u * %u * %u).",
                         apiName, pSampleLocationsInfo->sampleLocationsCount, pSampleLocationsInfo->sampleLocationGridSize.width,
                         pSampleLocationsInfo->sampleLocationGridSize.height, SampleCountSize(sample_count));
    }
    if ((phys_dev_ext_props.sample_locations_props.sampleLocationSampleCounts & sample_count) == 0) {
        skip |= LogError(device, "VUID-VkSampleLocationsInfoEXT-sampleLocationsPerPixel-01526",
                         "%s: VkSampleLocationsInfoEXT::sampleLocationsPerPixel of %s is not supported by the device, please check "
                         "VkPhysicalDeviceSampleLocationsPropertiesEXT::sampleLocationSampleCounts for valid sample counts.",
                         apiName, string_VkSampleCountFlagBits(sample_count));
    }

    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineMultisampleState(const PIPELINE_STATE &pipeline,
                                                          const safe_VkSubpassDescription2 *subpass_desc) const {
    bool skip = false;
    const auto *multisample_state = pipeline.MultisampleState();
    if (subpass_desc && multisample_state) {
        const auto &rp_state = pipeline.RenderPassState();
        auto accum_color_samples = [subpass_desc, &rp_state](uint32_t &samples) {
            for (uint32_t i = 0; i < subpass_desc->colorAttachmentCount; i++) {
                const auto attachment = subpass_desc->pColorAttachments[i].attachment;
                if (attachment != VK_ATTACHMENT_UNUSED) {
                    samples |= static_cast<uint32_t>(rp_state->createInfo.pAttachments[attachment].samples);
                }
            }
        };

        const uint32_t raster_samples = SampleCountSize(multisample_state->rasterizationSamples);
        if (!pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) {
            if (!(IsExtEnabled(device_extensions.vk_amd_mixed_attachment_samples) ||
                  IsExtEnabled(device_extensions.vk_nv_framebuffer_mixed_samples) ||
                  (enabled_features.multisampled_render_to_single_sampled_features.multisampledRenderToSingleSampled))) {
                uint32_t subpass_num_samples = 0;

                accum_color_samples(subpass_num_samples);

                if (subpass_desc->pDepthStencilAttachment &&
                    subpass_desc->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
                    const auto attachment = subpass_desc->pDepthStencilAttachment->attachment;
                    subpass_num_samples |= static_cast<uint32_t>(rp_state->createInfo.pAttachments[attachment].samples);
                }

                // subpass_num_samples is 0 when the subpass has no attachments or if all attachments are VK_ATTACHMENT_UNUSED.
                // Only validate the value of subpass_num_samples if the subpass has attachments that are not VK_ATTACHMENT_UNUSED.
                if (subpass_num_samples && (!IsPowerOfTwo(subpass_num_samples) || (subpass_num_samples != raster_samples))) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-multisampledRenderToSingleSampled-06853",
                                     "vkCreateGraphicsPipelines: pCreateInfo[%" PRIu32
                                     "].pMultisampleState->rasterizationSamples (%u) "
                                     "does not match the number of samples of the RenderPass color and/or depth attachment.",
                                     pipeline.create_index, raster_samples);
                }
            }

            if (IsExtEnabled(device_extensions.vk_amd_mixed_attachment_samples)) {
                VkSampleCountFlagBits max_sample_count = static_cast<VkSampleCountFlagBits>(0);
                for (uint32_t i = 0; i < subpass_desc->colorAttachmentCount; ++i) {
                    if (subpass_desc->pColorAttachments[i].attachment != VK_ATTACHMENT_UNUSED) {
                        max_sample_count =
                            std::max(max_sample_count,
                                     rp_state->createInfo.pAttachments[subpass_desc->pColorAttachments[i].attachment].samples);
                    }
                }
                if (subpass_desc->pDepthStencilAttachment &&
                    subpass_desc->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
                    max_sample_count =
                        std::max(max_sample_count,
                                 rp_state->createInfo.pAttachments[subpass_desc->pDepthStencilAttachment->attachment].samples);
                }
                const auto raster_state = pipeline.RasterizationState();
                if ((raster_state && raster_state->rasterizerDiscardEnable == VK_FALSE) &&
                    (max_sample_count != static_cast<VkSampleCountFlagBits>(0)) &&
                    (multisample_state->rasterizationSamples != max_sample_count)) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-subpass-01505",
                                     "vkCreateGraphicsPipelines: pCreateInfo[%" PRIu32
                                     "].pMultisampleState->rasterizationSamples (%s) != max "
                                     "attachment samples (%s) used in subpass %u.",
                                     pipeline.create_index, string_VkSampleCountFlagBits(multisample_state->rasterizationSamples),
                                     string_VkSampleCountFlagBits(max_sample_count), pipeline.Subpass());
                }
            }

            if (IsExtEnabled(device_extensions.vk_nv_framebuffer_mixed_samples)) {
                uint32_t subpass_color_samples = 0;

                accum_color_samples(subpass_color_samples);

                if (subpass_desc->pDepthStencilAttachment &&
                    subpass_desc->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
                    const auto attachment = subpass_desc->pDepthStencilAttachment->attachment;
                    const uint32_t subpass_depth_samples =
                        static_cast<uint32_t>(rp_state->createInfo.pAttachments[attachment].samples);
                    const auto ds_state = pipeline.DepthStencilState();
                    if (ds_state) {
                        const bool ds_test_enabled = (ds_state->depthTestEnable == VK_TRUE) ||
                                                     (ds_state->depthBoundsTestEnable == VK_TRUE) ||
                                                     (ds_state->stencilTestEnable == VK_TRUE);

                        if (ds_test_enabled &&
                            (!IsPowerOfTwo(subpass_depth_samples) || (raster_samples != subpass_depth_samples))) {
                            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-subpass-01411",
                                             "vkCreateGraphicsPipelines: pCreateInfo[%" PRIu32
                                             "].pMultisampleState->rasterizationSamples (%u) "
                                             "does not match the number of samples of the RenderPass depth attachment (%u).",
                                             pipeline.create_index, raster_samples, subpass_depth_samples);
                        }
                    }
                }

                if (IsPowerOfTwo(subpass_color_samples)) {
                    if (raster_samples < subpass_color_samples) {
                        skip |=
                            LogError(device, "VUID-VkGraphicsPipelineCreateInfo-subpass-01412",
                                     "vkCreateGraphicsPipelines: pCreateInfo[%" PRIu32
                                     "].pMultisampleState->rasterizationSamples (%u) "
                                     "is not greater or equal to the number of samples of the RenderPass color attachment (%u).",
                                     pipeline.create_index, raster_samples, subpass_color_samples);
                    }

                    if (multisample_state) {
                        if ((raster_samples > subpass_color_samples) && (multisample_state->sampleShadingEnable == VK_TRUE)) {
                            skip |= LogError(device, "VUID-VkPipelineMultisampleStateCreateInfo-rasterizationSamples-01415",
                                             "vkCreateGraphicsPipelines: pCreateInfo[%" PRIu32
                                             "].pMultisampleState->sampleShadingEnable must be "
                                             "VK_FALSE when "
                                             "pCreateInfo[%" PRIu32
                                             "].pMultisampleState->rasterizationSamples (%u) is greater than the number of "
                                             "samples of the "
                                             "subpass color attachment (%u).",
                                             pipeline.create_index, pipeline.create_index, raster_samples, subpass_color_samples);
                        }

                        const auto *coverage_modulation_state =
                            LvlFindInChain<VkPipelineCoverageModulationStateCreateInfoNV>(multisample_state->pNext);

                        if (coverage_modulation_state && (coverage_modulation_state->coverageModulationTableEnable == VK_TRUE)) {
                            if (coverage_modulation_state->coverageModulationTableCount !=
                                (raster_samples / subpass_color_samples)) {
                                skip |= LogError(
                                    device,
                                    "VUID-VkPipelineCoverageModulationStateCreateInfoNV-coverageModulationTableEnable-01405",
                                    "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                    "] VkPipelineCoverageModulationStateCreateInfoNV "
                                    "coverageModulationTableCount of %u is invalid.",
                                    pipeline.create_index, coverage_modulation_state->coverageModulationTableCount);
                            }
                        }
                    }
                }
            }

            if (IsExtEnabled(device_extensions.vk_nv_coverage_reduction_mode)) {
                uint32_t subpass_color_samples = 0;
                uint32_t subpass_depth_samples = 0;

                accum_color_samples(subpass_color_samples);

                if (subpass_desc->pDepthStencilAttachment &&
                    subpass_desc->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
                    const auto attachment = subpass_desc->pDepthStencilAttachment->attachment;
                    subpass_depth_samples = static_cast<uint32_t>(rp_state->createInfo.pAttachments[attachment].samples);
                }

                if (multisample_state && IsPowerOfTwo(subpass_color_samples) &&
                    (subpass_depth_samples == 0 || IsPowerOfTwo(subpass_depth_samples))) {
                    const auto *coverage_reduction_state =
                        LvlFindInChain<VkPipelineCoverageReductionStateCreateInfoNV>(multisample_state->pNext);

                    if (coverage_reduction_state) {
                        const VkCoverageReductionModeNV coverage_reduction_mode = coverage_reduction_state->coverageReductionMode;
                        uint32_t combination_count = 0;
                        std::vector<VkFramebufferMixedSamplesCombinationNV> combinations;
                        DispatchGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physical_device, &combination_count,
                                                                                                nullptr);
                        combinations.resize(combination_count);
                        DispatchGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physical_device, &combination_count,
                                                                                                &combinations[0]);

                        bool combination_found = false;
                        for (const auto &combination : combinations) {
                            if (coverage_reduction_mode == combination.coverageReductionMode &&
                                raster_samples == combination.rasterizationSamples &&
                                subpass_depth_samples == combination.depthStencilSamples &&
                                subpass_color_samples == combination.colorSamples) {
                                combination_found = true;
                                break;
                            }
                        }

                        if (!combination_found) {
                            skip |= LogError(
                                device, "VUID-VkGraphicsPipelineCreateInfo-coverageReductionMode-02722",
                                "vkCreateGraphicsPipelines: pCreateInfos[%" PRIu32
                                "] the specified combination of coverage "
                                "reduction mode (%s), pMultisampleState->rasterizationSamples (%u), sample counts for "
                                "the subpass color and depth/stencil attachments is not a valid combination returned by "
                                "vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV.",
                                pipeline.create_index, string_VkCoverageReductionModeNV(coverage_reduction_mode), raster_samples);
                        }
                    }
                }
            }
        }

        if (IsExtEnabled(device_extensions.vk_nv_fragment_coverage_to_color)) {
            const auto coverage_to_color_state = LvlFindInChain<VkPipelineCoverageToColorStateCreateInfoNV>(multisample_state);

            if (coverage_to_color_state && coverage_to_color_state->coverageToColorEnable == VK_TRUE) {
                bool attachment_is_valid = false;
                std::string error_detail;

                if (coverage_to_color_state->coverageToColorLocation < subpass_desc->colorAttachmentCount) {
                    const auto &color_attachment_ref =
                        subpass_desc->pColorAttachments[coverage_to_color_state->coverageToColorLocation];
                    if (color_attachment_ref.attachment != VK_ATTACHMENT_UNUSED) {
                        const auto &color_attachment = rp_state->createInfo.pAttachments[color_attachment_ref.attachment];

                        switch (color_attachment.format) {
                            case VK_FORMAT_R8_UINT:
                            case VK_FORMAT_R8_SINT:
                            case VK_FORMAT_R16_UINT:
                            case VK_FORMAT_R16_SINT:
                            case VK_FORMAT_R32_UINT:
                            case VK_FORMAT_R32_SINT:
                                attachment_is_valid = true;
                                break;
                            default:
                                std::ostringstream str;
                                str << "references an attachment with an invalid format ("
                                    << string_VkFormat(color_attachment.format) << ").";
                                error_detail = str.str();
                                break;
                        }
                    } else {
                        std::ostringstream str;
                        str << "references an invalid attachment. The subpass pColorAttachments["
                            << coverage_to_color_state->coverageToColorLocation
                            << "].attachment has the value VK_ATTACHMENT_UNUSED.";
                        error_detail = str.str();
                    }
                } else {
                    std::ostringstream str;
                    str << "references an non-existing attachment since the subpass colorAttachmentCount is "
                        << subpass_desc->colorAttachmentCount << ".";
                    error_detail = str.str();
                }

                if (!attachment_is_valid) {
                    skip |= LogError(device, "VUID-VkPipelineCoverageToColorStateCreateInfoNV-coverageToColorEnable-01404",
                                     "vkCreateGraphicsPipelines: pCreateInfos[%" PRId32
                                     "].pMultisampleState VkPipelineCoverageToColorStateCreateInfoNV "
                                     "coverageToColorLocation = %" PRIu32 " %s",
                                     pipeline.create_index, coverage_to_color_state->coverageToColorLocation, error_detail.c_str());
                }
            }
        }

        if (IsExtEnabled(device_extensions.vk_ext_sample_locations)) {
            const VkPipelineSampleLocationsStateCreateInfoEXT *sample_location_state =
                LvlFindInChain<VkPipelineSampleLocationsStateCreateInfoEXT>(multisample_state->pNext);

            if (sample_location_state != nullptr) {
                if ((sample_location_state->sampleLocationsEnable == VK_TRUE) &&
                    (pipeline.IsDynamic(VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT) == false) &&
                    (pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT) == false)) {
                    const VkSampleLocationsInfoEXT sample_location_info = sample_location_state->sampleLocationsInfo;
                    skip |= ValidateSampleLocationsInfo(&sample_location_info, "vkCreateGraphicsPipelines");
                    const VkExtent2D grid_size = sample_location_info.sampleLocationGridSize;

                    auto multisample_prop = LvlInitStruct<VkMultisamplePropertiesEXT>();
                    DispatchGetPhysicalDeviceMultisamplePropertiesEXT(physical_device, multisample_state->rasterizationSamples,
                                                                      &multisample_prop);
                    const VkExtent2D max_grid_size = multisample_prop.maxSampleLocationGridSize;

                    // Note order or "divide" in "sampleLocationsInfo must evenly divide VkMultisamplePropertiesEXT"
                    if (SafeModulo(max_grid_size.width, grid_size.width) != 0) {
                        const char *vuid = IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state3)
                                               ? "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07610"
                                               : "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-01521";
                        skip |= LogError(
                            device, vuid,
                            "vkCreateGraphicsPipelines() pCreateInfo[%" PRIu32
                            "]: Because there is no dynamic state for Sample Location "
                            "and sampleLocationEnable is true, the "
                            "VkPipelineSampleLocationsStateCreateInfoEXT::sampleLocationsInfo::sampleLocationGridSize.width (%u) "
                            "must be evenly divided by VkMultisamplePropertiesEXT::sampleLocationGridSize.width (%u).",
                            pipeline.create_index, grid_size.width, max_grid_size.width);
                    }
                    if (SafeModulo(max_grid_size.height, grid_size.height) != 0) {
                        const char *vuid = IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state3)
                                               ? "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07611"
                                               : "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-01522";
                        skip |= LogError(
                            device, vuid,
                            "vkCreateGraphicsPipelines() pCreateInfo[%" PRIu32
                            "]: Because there is no dynamic state for Sample Location "
                            "and sampleLocationEnable is true, the "
                            "VkPipelineSampleLocationsStateCreateInfoEXT::sampleLocationsInfo::sampleLocationGridSize.height (%u) "
                            "must be evenly divided by VkMultisamplePropertiesEXT::sampleLocationGridSize.height (%u).",
                            pipeline.create_index, grid_size.height, max_grid_size.height);
                    }
                    if (sample_location_info.sampleLocationsPerPixel != multisample_state->rasterizationSamples) {
                        const char *vuid = IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state3)
                                               ? "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07612"
                                               : "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-01523";
                        skip |= LogError(
                            device, vuid,
                            "vkCreateGraphicsPipelines() pCreateInfo[%" PRIu32
                            "]: Because there is no dynamic state for Sample Location "
                            "and sampleLocationEnable is true, the "
                            "VkPipelineSampleLocationsStateCreateInfoEXT::sampleLocationsInfo::sampleLocationsPerPixel (%s) must "
                            "be the same as the VkPipelineMultisampleStateCreateInfo::rasterizationSamples (%s).",
                            pipeline.create_index, string_VkSampleCountFlagBits(sample_location_info.sampleLocationsPerPixel),
                            string_VkSampleCountFlagBits(multisample_state->rasterizationSamples));
                    }
                }
            }
        }

        if (IsExtEnabled(device_extensions.vk_qcom_render_pass_shader_resolve)) {
            uint32_t subpass_input_attachment_samples = 0;

            for (uint32_t i = 0; i < subpass_desc->inputAttachmentCount; i++) {
                const auto attachment = subpass_desc->pInputAttachments[i].attachment;
                if (attachment != VK_ATTACHMENT_UNUSED) {
                    subpass_input_attachment_samples |=
                        static_cast<uint32_t>(rp_state->createInfo.pAttachments[attachment].samples);
                }
            }

            if ((subpass_desc->flags & VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM) != 0) {
                if ((raster_samples != subpass_input_attachment_samples) &&
                    !pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) {
                    skip |=
                        LogError(device, "VUID-VkGraphicsPipelineCreateInfo-rasterizationSamples-04899",
                                 "vkCreateGraphicsPipelines() pCreateInfo[%" PRIu32
                                 "]: The subpass includes "
                                 "VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM "
                                 "but the input attachment VkSampleCountFlagBits (%u) does not match the "
                                 "VkPipelineMultisampleStateCreateInfo::rasterizationSamples (%u) VkSampleCountFlagBits.",
                                 pipeline.create_index, subpass_input_attachment_samples, multisample_state->rasterizationSamples);
                }
                if (multisample_state->sampleShadingEnable == VK_TRUE) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-sampleShadingEnable-04900",
                                     "vkCreateGraphicsPipelines() pCreateInfo[%" PRIu32
                                     "]: The subpass includes "
                                     "VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM "
                                     "which requires sample shading is disabled, but "
                                     "VkPipelineMultisampleStateCreateInfo::sampleShadingEnable is true. ",
                                     pipeline.create_index);
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineDynamicState(const PIPELINE_STATE &pipeline) const {
    bool skip = false;
    const uint32_t active_shaders = pipeline.active_shaders;
    if (active_shaders & VK_SHADER_STAGE_MESH_BIT_EXT) {
        if (pipeline.IsDynamic(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY) ||
            pipeline.IsDynamic(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE)) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07065",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                             "] pDynamicState must not contain "
                             "VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY or VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE because "
                             "the pipeline contains a mesh shader.",
                             pipeline.create_index);
        }

        if (pipeline.IsDynamic(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE) ||
            pipeline.IsDynamic(VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT)) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07066",
                             "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                             "] pDynamicState must not contain "
                             "VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE or VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT because "
                             "the pipeline contains a mesh shader.",
                             pipeline.create_index);
        }

        if (pipeline.IsDynamic(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT)) {
            skip |= LogError(
                device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07067",
                "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                "] pDynamicState must not contain VK_DYNAMIC_STATE_VERTEX_INPUT_EXT because the pipeline contains a mesh shader.",
                pipeline.create_index);
        }
    }

    if (api_version < VK_API_VERSION_1_3 && !enabled_features.extended_dynamic_state_features.extendedDynamicState &&
        (pipeline.IsDynamic(VK_DYNAMIC_STATE_CULL_MODE) || pipeline.IsDynamic(VK_DYNAMIC_STATE_FRONT_FACE) ||
         pipeline.IsDynamic(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY) || pipeline.IsDynamic(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT) ||
         pipeline.IsDynamic(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT) ||
         pipeline.IsDynamic(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE) ||
         pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE) || pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE) ||
         pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_COMPARE_OP) || pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE) ||
         pipeline.IsDynamic(VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE) || pipeline.IsDynamic(VK_DYNAMIC_STATE_STENCIL_OP))) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03378",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the extendedDynamicState "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (api_version < VK_API_VERSION_1_3 && !enabled_features.extended_dynamic_state2_features.extendedDynamicState2 &&
        (pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE) || pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE) ||
         pipeline.IsDynamic(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE))) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04868",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the extendedDynamicState2 "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state2_features.extendedDynamicState2LogicOp &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_LOGIC_OP_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04869",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState2LogicOp feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state2_features.extendedDynamicState2PatchControlPoints &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04870",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState2PatchControlPoints "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3TessellationDomainOrigin &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3TessellationDomainOrigin-07370",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3TessellationDomainOrigin "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3DepthClampEnable &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3DepthClampEnable-07371",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3DepthClampEnable "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3PolygonMode &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_POLYGON_MODE_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3PolygonMode-07372",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3PolygonMode "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3RasterizationSamples &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3RasterizationSamples-07373",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3RasterizationSamples "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3SampleMask &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_SAMPLE_MASK_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3SampleMask-07374",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3SampleMask "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3AlphaToCoverageEnable &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3AlphaToCoverageEnable-07375",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3AlphaToCoverageEnable "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3AlphaToOneEnable &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3AlphaToOneEnable-07376",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3AlphaToOneEnable "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3LogicOpEnable &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3LogicOpEnable-07377",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3LogicOpEnable "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3ColorBlendEnable &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ColorBlendEnable-07378",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3ColorBlendEnable "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3ColorBlendEquation &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ColorBlendEquation-07379",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3ColorBlendEquation "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3ColorWriteMask &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ColorWriteMask-07380",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3ColorWriteMask "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3RasterizationStream &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3RasterizationStream-07381",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3RasterizationStream "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3ConservativeRasterizationMode &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ConservativeRasterizationMode-07382",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3ConservativeRasterizationMode "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3ExtraPrimitiveOverestimationSize &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ExtraPrimitiveOverestimationSize-07383",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3ExtraPrimitiveOverestimationSize "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3DepthClipEnable &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3DepthClipEnable-07384",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3DepthClipEnable "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3SampleLocationsEnable &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3SampleLocationsEnable-07385",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3SampleLocationsEnable "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3ColorBlendAdvanced &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ColorBlendAdvanced-07386",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3ColorBlendAdvanced "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3ProvokingVertexMode &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ProvokingVertexMode-07387",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3ProvokingVertexMode "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3LineRasterizationMode &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3LineRasterizationMode-07388",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3LineRasterizationMode "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3LineStippleEnable &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3LineStippleEnable-07389",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3LineStippleEnable "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3DepthClipNegativeOneToOne &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3DepthClipNegativeOneToOne-07390",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3DepthClipNegativeOneToOne "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3ViewportWScalingEnable &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ViewportWScalingEnable-07391",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3ViewportWScalingEnable "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3ViewportSwizzle &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ViewportSwizzle-07392",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3ViewportSwizzle "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3CoverageToColorEnable &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageToColorEnable-07393",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3CoverageToColorEnable "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3CoverageToColorLocation &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageToColorLocation-07394",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3CoverageToColorLocation "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3CoverageModulationMode &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageModulationMode-07395",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3CoverageModulationMode "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3CoverageModulationTableEnable &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageModulationTableEnable-07396",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3CoverageModulationTableEnable "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3CoverageModulationTable &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageModulationTable-07397",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3CoverageModulationTable "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3CoverageReductionMode &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageReductionMode-07398",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3CoverageReductionMode "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3RepresentativeFragmentTestEnable &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3RepresentativeFragmentTestEnable-07399",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3RepresentativeFragmentTestEnable "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.extended_dynamic_state3_features.extendedDynamicState3ShadingRateImageEnable &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ShadingRateImageEnable-07400",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: Extended dynamic state used by the "
                         "extendedDynamicState3ShadingRateImageEnable "
                         "feature is not enabled",
                         pipeline.create_index);
    }

    if (!enabled_features.vertex_input_dynamic_state_features.vertexInputDynamicState &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04807",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: The vertexInputDynamicState feature must be enabled to use "
                         "the VK_DYNAMIC_STATE_VERTEX_INPUT_EXT dynamic state",
                         pipeline.create_index);
    }

    if (!enabled_features.color_write_features.colorWriteEnable && pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT)) {
        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04800",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                         "]: The colorWriteEnable feature must be enabled to use the "
                         "VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT dynamic state",
                         pipeline.create_index);
    }

    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineFragmentShadingRateState(const PIPELINE_STATE &pipeline) const {
    bool skip = false;
    const VkPipelineFragmentShadingRateStateCreateInfoKHR *fragment_shading_rate_state =
        LvlFindInChain<VkPipelineFragmentShadingRateStateCreateInfoKHR>(pipeline.PNext());
    if (fragment_shading_rate_state && !pipeline.IsDynamic(VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR)) {
        const char *struct_name = "VkPipelineFragmentShadingRateStateCreateInfoKHR";

        if (fragment_shading_rate_state->fragmentSize.width == 0) {
            skip |=
                LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04494",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32 "]: Fragment width of %u has been specified in %s.",
                         pipeline.create_index, fragment_shading_rate_state->fragmentSize.width, struct_name);
        }

        if (fragment_shading_rate_state->fragmentSize.height == 0) {
            skip |=
                LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04495",
                         "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32 "]: Fragment height of %u has been specified in %s.",
                         pipeline.create_index, fragment_shading_rate_state->fragmentSize.height, struct_name);
        }

        if (fragment_shading_rate_state->fragmentSize.width != 0 &&
            !IsPowerOfTwo(fragment_shading_rate_state->fragmentSize.width)) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04496",
                             "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                             "]: Non-power-of-two fragment width of %u has been specified in %s.",
                             pipeline.create_index, fragment_shading_rate_state->fragmentSize.width, struct_name);
        }

        if (fragment_shading_rate_state->fragmentSize.height != 0 &&
            !IsPowerOfTwo(fragment_shading_rate_state->fragmentSize.height)) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04497",
                             "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                             "]: Non-power-of-two fragment height of %u has been specified in %s.",
                             pipeline.create_index, fragment_shading_rate_state->fragmentSize.height, struct_name);
        }

        if (fragment_shading_rate_state->fragmentSize.width > 4) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04498",
                             "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                             "]: Fragment width of %u specified in %s is too large.",
                             pipeline.create_index, fragment_shading_rate_state->fragmentSize.width, struct_name);
        }

        if (fragment_shading_rate_state->fragmentSize.height > 4) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04499",
                             "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                             "]: Fragment height of %u specified in %s is too large",
                             pipeline.create_index, fragment_shading_rate_state->fragmentSize.height, struct_name);
        }

        if (!enabled_features.fragment_shading_rate_features.pipelineFragmentShadingRate &&
            fragment_shading_rate_state->fragmentSize.width != 1) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04500",
                             "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                             "]: Pipeline fragment width of %u has been specified in %s, but "
                             "pipelineFragmentShadingRate is not enabled",
                             pipeline.create_index, fragment_shading_rate_state->fragmentSize.width, struct_name);
        }

        if (!enabled_features.fragment_shading_rate_features.pipelineFragmentShadingRate &&
            fragment_shading_rate_state->fragmentSize.height != 1) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04500",
                             "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                             "]: Pipeline fragment height of %u has been specified in %s, but "
                             "pipelineFragmentShadingRate is not enabled",
                             pipeline.create_index, fragment_shading_rate_state->fragmentSize.height, struct_name);
        }

        if (!enabled_features.fragment_shading_rate_features.primitiveFragmentShadingRate &&
            fragment_shading_rate_state->combinerOps[0] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04501",
                             "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                             "]: First combiner operation of %s has been specified in %s, but "
                             "primitiveFragmentShadingRate is not enabled",
                             pipeline.create_index,
                             string_VkFragmentShadingRateCombinerOpKHR(fragment_shading_rate_state->combinerOps[0]), struct_name);
        }

        if (!enabled_features.fragment_shading_rate_features.attachmentFragmentShadingRate &&
            fragment_shading_rate_state->combinerOps[1] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04502",
                             "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                             "]: Second combiner operation of %s has been specified in %s, but "
                             "attachmentFragmentShadingRate is not enabled",
                             pipeline.create_index,
                             string_VkFragmentShadingRateCombinerOpKHR(fragment_shading_rate_state->combinerOps[1]), struct_name);
        }

        if (!phys_dev_ext_props.fragment_shading_rate_props.fragmentShadingRateNonTrivialCombinerOps &&
            (fragment_shading_rate_state->combinerOps[0] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR &&
             fragment_shading_rate_state->combinerOps[0] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR)) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-fragmentShadingRateNonTrivialCombinerOps-04506",
                             "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                             "]: First combiner operation of %s has been specified in %s, but "
                             "fragmentShadingRateNonTrivialCombinerOps is not supported",
                             pipeline.create_index,
                             string_VkFragmentShadingRateCombinerOpKHR(fragment_shading_rate_state->combinerOps[0]), struct_name);
        }

        if (!phys_dev_ext_props.fragment_shading_rate_props.fragmentShadingRateNonTrivialCombinerOps &&
            (fragment_shading_rate_state->combinerOps[1] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR &&
             fragment_shading_rate_state->combinerOps[1] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR)) {
            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-fragmentShadingRateNonTrivialCombinerOps-04506",
                             "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                             "]: Second combiner operation of %s has been specified in %s, but "
                             "fragmentShadingRateNonTrivialCombinerOps is not supported",
                             pipeline.create_index,
                             string_VkFragmentShadingRateCombinerOpKHR(fragment_shading_rate_state->combinerOps[1]), struct_name);
        }

        const auto combiner_ops = fragment_shading_rate_state->combinerOps;
        if (pipeline.pre_raster_state || pipeline.fragment_shader_state) {
            const auto enums = ValidParamValues<VkFragmentShadingRateCombinerOpKHR>();
            if (std::find(enums.begin(), enums.end(), combiner_ops[0]) == enums.end()) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-06567",
                                 "vkCreateGraphicsPipelines(): in pCreateInfos[%" PRIu32
                                 "], combinerOp[0] (%s) is not a valid VkFragmentShadingRateCombinerOpKHR value.",
                                 pipeline.create_index, string_VkFragmentShadingRateCombinerOpKHR(combiner_ops[0]));
            }
            if (std::find(enums.begin(), enums.end(), combiner_ops[1]) == enums.end()) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-pDynamicState-06568",
                                 "vkCreateGraphicsPipelines(): in pCreateInfos[%" PRIu32
                                 "], combinerOp[1] (%s) is not a valid VkFragmentShadingRateCombinerOpKHR value.",
                                 pipeline.create_index, string_VkFragmentShadingRateCombinerOpKHR(combiner_ops[1]));
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineDynamicRendering(const PIPELINE_STATE &pipeline) const {
    bool skip = false;
    const auto rendering_struct = LvlFindInChain<VkPipelineRenderingCreateInfo>(pipeline.PNext());
    if (rendering_struct) {
        const auto color_blend_state = pipeline.ColorBlendState();
        const auto raster_state = pipeline.RasterizationState();
        const bool has_rasterization = raster_state && (raster_state->rasterizerDiscardEnable == VK_FALSE);
        if (has_rasterization) {
            if (pipeline.fragment_shader_state && pipeline.fragment_output_state &&
                ((rendering_struct->depthAttachmentFormat != VK_FORMAT_UNDEFINED) ||
                 (rendering_struct->stencilAttachmentFormat != VK_FORMAT_UNDEFINED)) &&
                !pipeline.DepthStencilState()) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06053",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "]: has fragment state and a depth format (%s) or stencil format (%s) and an invalid "
                                 "pDepthStencilState structure",
                                 pipeline.create_index, string_VkFormat(rendering_struct->depthAttachmentFormat),
                                 string_VkFormat(rendering_struct->stencilAttachmentFormat));
            }

            if (pipeline.fragment_output_state && (rendering_struct->colorAttachmentCount != 0) && !color_blend_state &&
                pipeline.GetCreateInfo<VkGraphicsPipelineCreateInfo>().renderPass == VK_NULL_HANDLE) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06054",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "] has VkPipelineRenderingCreateInfoKHR::colorAttachmentCount (%" PRIu32
                                 ") and an invalid pColorBlendState structure",
                                 pipeline.create_index, rendering_struct->colorAttachmentCount);
            }
        }

        if (rendering_struct->viewMask != 0) {
            const uint32_t active_shaders = pipeline.active_shaders;
            if (!enabled_features.core11.multiviewTessellationShader &&
                (active_shaders & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT ||
                 active_shaders & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06057",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "] has VkPipelineRenderingCreateInfoKHR->viewMask(%" PRIu32
                                 ") and "
                                 "multiviewTessellationShader is not enabled, contains tesselation shaders",
                                 pipeline.create_index, rendering_struct->viewMask);
            }

            if (!enabled_features.core11.multiviewGeometryShader && (active_shaders & VK_SHADER_STAGE_GEOMETRY_BIT)) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06058",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "] has VkPipelineRenderingCreateInfoKHR->viewMask(%" PRIu32
                                 ") and "
                                 "multiviewGeometryShader is not enabled, contains geometry shader",
                                 pipeline.create_index, rendering_struct->viewMask);
            }

            if (!enabled_features.mesh_shader_features.multiviewMeshShader && (active_shaders & VK_SHADER_STAGE_MESH_BIT_EXT)) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-07064",
                                 "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                 "] has VkPipelineRenderingCreateInfoKHR->viewMask(%" PRIu32
                                 ") and "
                                 "multiviewMeshShader is not enabled, contains mesh shader",
                                 pipeline.create_index, rendering_struct->viewMask);
            }

            if (pipeline.GetCreateInfo<VkGraphicsPipelineCreateInfo>().renderPass == VK_NULL_HANDLE && raster_state) {
                for (const auto &stage : pipeline.stage_state) {
                    if (stage.writes_to_gl_layer) {
                        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06059",
                                         "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                         "] is being created with fragment shader state and renderPass != VK_NULL_HANDLE, but "
                                         "pMultisampleState is NULL.",
                                         pipeline.create_index);
                    }
                }
            }
        }

        if (pipeline.fragment_output_state) {
            for (uint32_t color_index = 0; color_index < rendering_struct->colorAttachmentCount; color_index++) {
                const VkFormat color_format = rendering_struct->pColorAttachmentFormats[color_index];
                if (color_format != VK_FORMAT_UNDEFINED) {
                    VkFormatFeatureFlags2KHR format_features = GetPotentialFormatFeatures(color_format);
                    if (((format_features & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT) == 0) &&
                        (color_blend_state && (color_index < color_blend_state->attachmentCount) &&
                         (color_blend_state->pAttachments[color_index].blendEnable != VK_FALSE))) {
                        skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06062",
                                         "vkCreateGraphicsPipelines(): pCreateInfos[%" PRIu32
                                         "]: pColorBlendState->blendEnable must be false ",
                                         pipeline.create_index);
                    }

                    if (!IsExtEnabled(device_extensions.vk_nv_linear_color_attachment)) {
                        if ((format_features & VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT) == 0) {
                            skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06581",
                                             "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                                             "]: color_format (%s) must be a format with potential format features that include "
                                             "VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT",
                                             pipeline.create_index, string_VkFormat(color_format));
                        }
                    } else {
                        if ((format_features & (VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT |
                                                VK_FORMAT_FEATURE_2_LINEAR_COLOR_ATTACHMENT_BIT_NV)) == 0) {
                            skip |= LogError(
                                device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06582",
                                "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                                "]: color_format (%s) must be a format with potential format features that include "
                                "VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT or VK_FORMAT_FEATURE_2_LINEAR_COLOR_ATTACHMENT_BIT_NV",
                                pipeline.create_index, string_VkFormat(color_format));
                        }
                    }
                }
            }

            if (rendering_struct->depthAttachmentFormat != VK_FORMAT_UNDEFINED) {
                VkFormatFeatureFlags2 format_features = GetPotentialFormatFeatures(rendering_struct->depthAttachmentFormat);
                if ((format_features & VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT) == 0) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06585",
                                     "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                                     "]: depthAttachmentFormat (%s) must be a format with potential format features that include "
                                     "VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT",
                                     pipeline.create_index, string_VkFormat(rendering_struct->depthAttachmentFormat));
                }
            }

            if (rendering_struct->stencilAttachmentFormat != VK_FORMAT_UNDEFINED) {
                VkFormatFeatureFlags2 format_features = GetPotentialFormatFeatures(rendering_struct->stencilAttachmentFormat);
                if ((format_features & VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT) == 0) {
                    skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06586",
                                     "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                                     "]: stencilAttachmentFormat (%s) must be a format with potential format features that include "
                                     "VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT",
                                     pipeline.create_index, string_VkFormat(rendering_struct->stencilAttachmentFormat));
                }
            }

            if ((rendering_struct->depthAttachmentFormat != VK_FORMAT_UNDEFINED) &&
                (rendering_struct->stencilAttachmentFormat != VK_FORMAT_UNDEFINED) &&
                (rendering_struct->depthAttachmentFormat != rendering_struct->stencilAttachmentFormat)) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06589",
                                 "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                                 "]: depthAttachmentFormat is not VK_FORMAT_UNDEFINED and stencilAttachmentFormat is not "
                                 "VK_FORMAT_UNDEFINED, but depthAttachmentFormat (%s) does not equal stencilAttachmentFormat (%s)",
                                 pipeline.create_index, string_VkFormat(rendering_struct->depthAttachmentFormat),
                                 string_VkFormat(rendering_struct->stencilAttachmentFormat));
            }
        }

        if (pipeline.IsRenderPassStateRequired()) {
            if ((enabled_features.core11.multiview == VK_FALSE) && (rendering_struct->viewMask != 0)) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-multiview-06577",
                                 "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                                 "]: multiview is not enabled but viewMask is (%u).",
                                 pipeline.create_index, rendering_struct->viewMask);
            }

            if (MostSignificantBit(rendering_struct->viewMask) >=
                static_cast<int32_t>(phys_dev_props_core11.maxMultiviewViewCount)) {
                skip |= LogError(device, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06578",
                                 "vkCreateGraphicsPipelines() pCreateInfos[%" PRIu32
                                 "]: Most significant bit in "
                                 "VkPipelineRenderingCreateInfo->viewMask(%u) must be less maxMultiviewViewCount(%u)",
                                 pipeline.create_index, rendering_struct->viewMask, phys_dev_props_core11.maxMultiviewViewCount);
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineBindPoint(const CMD_BUFFER_STATE *cb_state, const PIPELINE_STATE &pipeline) const {
    bool skip = false;

    if (cb_state->inheritedViewportDepths.size() != 0) {
        bool dyn_viewport =
            pipeline.IsDynamic(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT) || pipeline.IsDynamic(VK_DYNAMIC_STATE_VIEWPORT);
        bool dyn_scissor =
            pipeline.IsDynamic(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT) || pipeline.IsDynamic(VK_DYNAMIC_STATE_SCISSOR);
        if (!dyn_viewport || !dyn_scissor) {
            skip |= LogError(device, "VUID-vkCmdBindPipeline-commandBuffer-04808",
                             "Graphics pipeline incompatible with viewport/scissor inheritance.");
        }
        const auto *discard_rectangle_state = LvlFindInChain<VkPipelineDiscardRectangleStateCreateInfoEXT>(pipeline.PNext());
        if (discard_rectangle_state && discard_rectangle_state->discardRectangleCount != 0) {
            if (!pipeline.IsDynamic(VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT)) {
                skip |= LogError(device, "VUID-vkCmdBindPipeline-commandBuffer-04809",
                                 "vkCmdBindPipeline(): commandBuffer is a secondary command buffer with "
                                 "VkCommandBufferInheritanceViewportScissorInfoNV::viewportScissor2D enabled, pipelineBindPoint is "
                                 "VK_PIPELINE_BIND_POINT_GRAPHICS and pipeline was created with "
                                 "VkPipelineDiscardRectangleStateCreateInfoEXT::discardRectangleCount = %" PRIu32
                                 ", but without VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT.",
                                 discard_rectangle_state->discardRectangleCount);
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                VkPipeline pipeline) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);

    bool skip = false;
    skip |= ValidateCmd(*cb_state, CMD_BINDPIPELINE);
    static const std::map<VkPipelineBindPoint, std::string> bindpoint_errors = {
        std::make_pair(VK_PIPELINE_BIND_POINT_GRAPHICS, "VUID-vkCmdBindPipeline-pipelineBindPoint-00777"),
        std::make_pair(VK_PIPELINE_BIND_POINT_COMPUTE, "VUID-vkCmdBindPipeline-pipelineBindPoint-00778"),
        std::make_pair(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, "VUID-vkCmdBindPipeline-pipelineBindPoint-02391")};

    skip |= ValidatePipelineBindPoint(cb_state.get(), pipelineBindPoint, "vkCmdBindPipeline()", bindpoint_errors);

    auto pPipeline = Get<PIPELINE_STATE>(pipeline);
    assert(pPipeline);
    const PIPELINE_STATE &pipeline_state = *pPipeline;

    const auto pipeline_state_bind_point = pipeline_state.GetPipelineType();

    if (pipelineBindPoint != pipeline_state_bind_point) {
        if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) {
            skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdBindPipeline-pipelineBindPoint-00779",
                             "Cannot bind a pipeline of type %s to the graphics pipeline bind point",
                             string_VkPipelineBindPoint(pipeline_state_bind_point));
        } else if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_COMPUTE) {
            skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdBindPipeline-pipelineBindPoint-00780",
                             "Cannot bind a pipeline of type %s to the compute pipeline bind point",
                             string_VkPipelineBindPoint(pipeline_state_bind_point));
        } else if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR) {
            skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdBindPipeline-pipelineBindPoint-02392",
                             "Cannot bind a pipeline of type %s to the ray-tracing pipeline bind point",
                             string_VkPipelineBindPoint(pipeline_state_bind_point));
        }
    } else {
        if (pipelineBindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) {
            skip |= ValidateGraphicsPipelineBindPoint(cb_state.get(), pipeline_state);

            if (cb_state->activeRenderPass &&
                phys_dev_ext_props.provoking_vertex_props.provokingVertexModePerPipeline == VK_FALSE) {
                const auto lvl_bind_point = ConvertToLvlBindPoint(pipelineBindPoint);
                const auto &last_bound = cb_state->lastBound[lvl_bind_point];
                if (last_bound.pipeline_state) {
                    auto last_bound_provoking_vertex_state_ci =
                        LvlFindInChain<VkPipelineRasterizationProvokingVertexStateCreateInfoEXT>(
                            last_bound.pipeline_state->RasterizationState()->pNext);

                    auto current_provoking_vertex_state_ci =
                        LvlFindInChain<VkPipelineRasterizationProvokingVertexStateCreateInfoEXT>(
                            pipeline_state.RasterizationState()->pNext);

                    if (last_bound_provoking_vertex_state_ci && !current_provoking_vertex_state_ci) {
                        skip |= LogError(pipeline, "VUID-vkCmdBindPipeline-pipelineBindPoint-04881",
                                         "Previous %s's provokingVertexMode is %s, but %s doesn't chain "
                                         "VkPipelineRasterizationProvokingVertexStateCreateInfoEXT.",
                                         report_data->FormatHandle(last_bound.pipeline_state->pipeline()).c_str(),
                                         string_VkProvokingVertexModeEXT(last_bound_provoking_vertex_state_ci->provokingVertexMode),
                                         report_data->FormatHandle(pipeline).c_str());
                    } else if (!last_bound_provoking_vertex_state_ci && current_provoking_vertex_state_ci) {
                        skip |= LogError(pipeline, "VUID-vkCmdBindPipeline-pipelineBindPoint-04881",
                                         " %s's provokingVertexMode is %s, but previous %s doesn't chain "
                                         "VkPipelineRasterizationProvokingVertexStateCreateInfoEXT.",
                                         report_data->FormatHandle(pipeline).c_str(),
                                         string_VkProvokingVertexModeEXT(current_provoking_vertex_state_ci->provokingVertexMode),
                                         report_data->FormatHandle(last_bound.pipeline_state->pipeline()).c_str());
                    } else if (last_bound_provoking_vertex_state_ci && current_provoking_vertex_state_ci &&
                               last_bound_provoking_vertex_state_ci->provokingVertexMode !=
                                   current_provoking_vertex_state_ci->provokingVertexMode) {
                        skip |=
                            LogError(pipeline, "VUID-vkCmdBindPipeline-pipelineBindPoint-04881",
                                     "%s's provokingVertexMode is %s, but previous %s's provokingVertexMode is %s.",
                                     report_data->FormatHandle(pipeline).c_str(),
                                     string_VkProvokingVertexModeEXT(current_provoking_vertex_state_ci->provokingVertexMode),
                                     report_data->FormatHandle(last_bound.pipeline_state->pipeline()).c_str(),
                                     string_VkProvokingVertexModeEXT(last_bound_provoking_vertex_state_ci->provokingVertexMode));
                    }
                }
            }

            if (cb_state->activeRenderPass && phys_dev_ext_props.sample_locations_props.variableSampleLocations == VK_FALSE) {
                const auto *sample_locations = LvlFindInChain<VkPipelineSampleLocationsStateCreateInfoEXT>(pipeline_state.PNext());
                if (sample_locations && sample_locations->sampleLocationsEnable == VK_TRUE &&
                    !pipeline_state.IsDynamic(VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT)) {
                    const VkRenderPassSampleLocationsBeginInfoEXT *sample_locations_begin_info =
                        LvlFindInChain<VkRenderPassSampleLocationsBeginInfoEXT>(cb_state->activeRenderPassBeginInfo.pNext);
                    bool found = false;
                    if (sample_locations_begin_info) {
                        for (uint32_t i = 0; i < sample_locations_begin_info->postSubpassSampleLocationsCount; ++i) {
                            if (sample_locations_begin_info->pPostSubpassSampleLocations[i].subpassIndex ==
                                cb_state->activeSubpass) {
                                if (MatchSampleLocationsInfo(
                                        &sample_locations_begin_info->pPostSubpassSampleLocations[i].sampleLocationsInfo,
                                        &sample_locations->sampleLocationsInfo)) {
                                    found = true;
                                }
                            }
                        }
                    }
                    if (!found) {
                        skip |=
                            LogError(pipeline, "VUID-vkCmdBindPipeline-variableSampleLocations-01525",
                                     "vkCmdBindPipeline(): VkPhysicalDeviceSampleLocationsPropertiesEXT::variableSampleLocations "
                                     "is false, pipeline is a graphics pipeline with "
                                     "VkPipelineSampleLocationsStateCreateInfoEXT::sampleLocationsEnable equal to true and without "
                                     "VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT, but the current render pass (%" PRIu32
                                     ") was not begun with any element of "
                                     "VkRenderPassSampleLocationsBeginInfoEXT::pPostSubpassSampleLocations subpassIndex "
                                     "matching the current subpass index and sampleLocationsInfo matching sampleLocationsInfo of "
                                     "VkPipelineSampleLocationsStateCreateInfoEXT the pipeline was created with.",
                                     cb_state->activeSubpass);
                    }
                }
            }
        }
        if (pipeline_state.GetPipelineCreateFlags() & VK_PIPELINE_CREATE_LIBRARY_BIT_KHR) {
            skip |= LogError(
                pipeline, "VUID-vkCmdBindPipeline-pipeline-03382",
                "vkCmdBindPipeline(): Cannot bind a pipeline that was created with the VK_PIPELINE_CREATE_LIBRARY_BIT_KHR flag.");
        }
        if (cb_state->transform_feedback_active) {
            skip |= LogError(pipeline, "VUID-vkCmdBindPipeline-None-02323", "vkCmdBindPipeline(): transform feedback is active.");
        }
        if (cb_state->activeRenderPass && cb_state->activeRenderPass->UsesDynamicRendering()) {
            const auto rendering_info = cb_state->activeRenderPass->dynamic_rendering_begin_rendering_info;
            const auto msrtss_info = LvlFindInChain<VkMultisampledRenderToSingleSampledInfoEXT>(rendering_info.pNext);
            // If no color attachment exists, this can be nullptr.
            const auto multisample_state = pipeline_state.MultisampleState();
            const auto pipeline_rasterization_samples = multisample_state ? multisample_state->rasterizationSamples : 0;
            if (msrtss_info && msrtss_info->multisampledRenderToSingleSampledEnable &&
                (msrtss_info->rasterizationSamples != pipeline_rasterization_samples)) {
                skip |= LogError(pipeline, "VUID-vkCmdBindPipeline-pipeline-06856",
                                 "vkCmdBindPipeline(): A VkMultisampledRenderToSingleSampledInfoEXT struct in the pNext chain of "
                                 "VkRenderingInfo passed to vkCmdBeginRendering has a rasterizationSamples of (%" PRIu32
                                 ") which is not equal to pMultisampleState.rasterizationSamples used to create the pipeline, "
                                 "which is (%" PRIu32 ").",
                                 msrtss_info->rasterizationSamples, pipeline_rasterization_samples);
            }
        }
        if (enabled_features.pipeline_protected_access_features.pipelineProtectedAccess) {
            if (cb_state->unprotected) {
                if (pipeline_state.GetPipelineCreateFlags() & VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT) {
                    skip |= LogError(pipeline, "VUID-vkCmdBindPipeline-pipelineProtectedAccess-07409",
                                     "vkCmdBindPipeline(): Binding pipeline created with "
                                     "VK_PIPELINE_CREATE_PROTECTED_ACCESS_ONLY_BIT_EXT in an unprotected command buffer");
                }
            } else {
                if (pipeline_state.GetPipelineCreateFlags() & VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT) {
                    skip |= LogError(pipeline, "VUID-vkCmdBindPipeline-pipelineProtectedAccess-07408",
                                     "vkCmdBindPipeline(): Binding pipeline created with "
                                     "VK_PIPELINE_CREATE_NO_PROTECTED_ACCESS_BIT_EXT in a protected command buffer");
                }
            }
        }
    }

    return skip;
}

// Validates that the supplied bind point is supported for the command buffer (vis. the command pool)
// Takes array of error codes as some of the VUID's (e.g. vkCmdBindPipeline) are written per bindpoint
// TODO add vkCmdBindPipeline bind_point validation using this call.
bool CoreChecks::ValidatePipelineBindPoint(const CMD_BUFFER_STATE *cb_state, VkPipelineBindPoint bind_point, const char *func_name,
                                           const std::map<VkPipelineBindPoint, std::string> &bind_errors) const {
    bool skip = false;
    auto pool = cb_state->command_pool;
    if (pool) {  // The loss of a pool in a recording cmd is reported in DestroyCommandPool
        static const std::map<VkPipelineBindPoint, VkQueueFlags> flag_mask = {
            std::make_pair(VK_PIPELINE_BIND_POINT_GRAPHICS, static_cast<VkQueueFlags>(VK_QUEUE_GRAPHICS_BIT)),
            std::make_pair(VK_PIPELINE_BIND_POINT_COMPUTE, static_cast<VkQueueFlags>(VK_QUEUE_COMPUTE_BIT)),
            std::make_pair(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                           static_cast<VkQueueFlags>(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)),
        };
        const auto &qfp = physical_device_state->queue_family_properties[pool->queueFamilyIndex];
        if (0 == (qfp.queueFlags & flag_mask.at(bind_point))) {
            const std::string &error = bind_errors.at(bind_point);
            const LogObjectList objlist(cb_state->commandBuffer(), cb_state->createInfo.commandPool);
            skip |= LogError(objlist, error, "%s: %s was allocated from %s that does not support bindpoint %s.", func_name,
                             report_data->FormatHandle(cb_state->commandBuffer()).c_str(),
                             report_data->FormatHandle(cb_state->createInfo.commandPool).c_str(),
                             string_VkPipelineBindPoint(bind_point));
        }
    }
    return skip;
}

// Validate draw-time state related to the PSO
bool CoreChecks::ValidatePipelineDrawtimeState(const LAST_BOUND_STATE &state, const CMD_BUFFER_STATE &cb_state, CMD_TYPE cmd_type,
                                               const PIPELINE_STATE &pipeline) const {
    bool skip = false;
    const auto &current_vtx_bfr_binding_info = cb_state.current_vertex_buffer_binding_info.vertex_buffer_bindings;
    const DrawDispatchVuid vuid = GetDrawDispatchVuid(cmd_type);
    const char *caller = CommandTypeString(cmd_type);
    const auto pipeline_flags = pipeline.GetPipelineCreateFlags();
    const auto &dynamic_state_value = cb_state.dynamic_state_value;

    if (cb_state.activeRenderPass && cb_state.activeRenderPass->UsesDynamicRendering()) {
        const auto rendering_info = cb_state.activeRenderPass->dynamic_rendering_begin_rendering_info;
        const auto &rp_state = pipeline.RenderPassState();
        if (rp_state) {
            const auto rendering_view_mask = cb_state.activeRenderPass->GetDynamicRenderingViewMask();
            if (rp_state->renderPass() != VK_NULL_HANDLE) {
                const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline(), rp_state->renderPass());
                skip |= LogError(objlist, vuid.dynamic_rendering_06198,
                                 "%s: Currently bound pipeline %s must have been created with a "
                                 "VkGraphicsPipelineCreateInfo::renderPass equal to VK_NULL_HANDLE",
                                 caller, report_data->FormatHandle(state.pipeline_state->pipeline()).c_str());
            }

            const auto pipeline_rendering_ci = rp_state->dynamic_rendering_pipeline_create_info;

            if (pipeline_rendering_ci.viewMask != rendering_view_mask) {
                const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline(), cb_state.activeRenderPass->renderPass());
                skip |= LogError(objlist, vuid.dynamic_rendering_view_mask,
                                 "%s: Currently bound pipeline %s viewMask ([%" PRIu32
                                 ") must be equal to VkRenderingInfo::viewMask ([%" PRIu32 ")",
                                 caller, report_data->FormatHandle(state.pipeline_state->pipeline()).c_str(),
                                 pipeline_rendering_ci.viewMask, rendering_view_mask);
            }

            const auto color_attachment_count = pipeline_rendering_ci.colorAttachmentCount;
            const auto rendering_color_attachment_count = cb_state.activeRenderPass->GetDynamicRenderingColorAttachmentCount();
            if (color_attachment_count && (color_attachment_count != rendering_color_attachment_count)) {
                const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline(), cb_state.activeRenderPass->renderPass());
                skip |= LogError(objlist, vuid.dynamic_rendering_color_count,
                                 "%s: Currently bound pipeline %s VkPipelineRenderingCreateInfo::colorAttachmentCount ([%" PRIu32
                                 ") must be equal to VkRenderingInfo::colorAttachmentCount ([%" PRIu32 ")",
                                 caller, report_data->FormatHandle(state.pipeline_state->pipeline()).c_str(),
                                 pipeline_rendering_ci.colorAttachmentCount, rendering_color_attachment_count);
            }

            for (uint32_t i = 0; i < rendering_info.colorAttachmentCount; ++i) {
                if (rendering_info.pColorAttachments[i].imageView == VK_NULL_HANDLE) {
                    continue;
                }
                auto view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pColorAttachments[i].imageView);
                if ((pipeline_rendering_ci.colorAttachmentCount > i) &&
                    view_state->create_info.format != pipeline_rendering_ci.pColorAttachmentFormats[i]) {
                    const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline(),
                                                cb_state.activeRenderPass->renderPass());
                    skip |= LogError(objlist, vuid.dynamic_rendering_color_formats,
                                     "%s: VkRenderingInfo::pColorAttachments[%" PRIu32
                                     "].imageView format (%s) must match corresponding format in "
                                     "VkPipelineRenderingCreateInfo::pColorAttachmentFormats[%" PRIu32 "] (%s)",
                                     caller, i, string_VkFormat(view_state->create_info.format), i,
                                     string_VkFormat(pipeline_rendering_ci.pColorAttachmentFormats[i]));
                }
            }

            if (rendering_info.pDepthAttachment && rendering_info.pDepthAttachment->imageView != VK_NULL_HANDLE) {
                auto view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pDepthAttachment->imageView);
                if (view_state->create_info.format != pipeline_rendering_ci.depthAttachmentFormat) {
                    const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline(),
                                                cb_state.activeRenderPass->renderPass());
                    skip |= LogError(objlist, vuid.dynamic_rendering_depth_format,
                                     "%s: VkRenderingInfo::pDepthAttachment->imageView format (%s) must match corresponding format "
                                     "in VkPipelineRenderingCreateInfo::depthAttachmentFormat (%s)",
                                     caller, string_VkFormat(view_state->create_info.format),
                                     string_VkFormat(pipeline_rendering_ci.depthAttachmentFormat));
                }
            }

            if (rendering_info.pStencilAttachment && rendering_info.pStencilAttachment->imageView != VK_NULL_HANDLE) {
                auto view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pStencilAttachment->imageView);
                if (view_state->create_info.format != pipeline_rendering_ci.stencilAttachmentFormat) {
                    const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline(),
                                                cb_state.activeRenderPass->renderPass());
                    skip |= LogError(objlist, vuid.dynamic_rendering_stencil_format,
                                     "%s: VkRenderingInfo::pStencilAttachment->imageView format (%s) must match corresponding "
                                     "format in VkPipelineRenderingCreateInfo::stencilAttachmentFormat (%s)",
                                     caller, string_VkFormat(view_state->create_info.format),
                                     string_VkFormat(pipeline_rendering_ci.stencilAttachmentFormat));
                }
            }

            auto rendering_fragment_shading_rate_attachment_info =
                LvlFindInChain<VkRenderingFragmentShadingRateAttachmentInfoKHR>(rendering_info.pNext);
            if (rendering_fragment_shading_rate_attachment_info &&
                (rendering_fragment_shading_rate_attachment_info->imageView != VK_NULL_HANDLE)) {
                if (!(pipeline_flags & VK_PIPELINE_RASTERIZATION_STATE_CREATE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
                    const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline(),
                                                cb_state.activeRenderPass->renderPass());
                    skip |= LogError(objlist, vuid.dynamic_rendering_fsr,
                                     "%s: Currently bound graphics pipeline %s must have been created with "
                                     "VK_PIPELINE_RASTERIZATION_STATE_CREATE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR",
                                     caller, report_data->FormatHandle(state.pipeline_state->pipeline()).c_str());
                }
            }

            auto rendering_fragment_shading_rate_density_map =
                LvlFindInChain<VkRenderingFragmentDensityMapAttachmentInfoEXT>(rendering_info.pNext);
            if (rendering_fragment_shading_rate_density_map &&
                (rendering_fragment_shading_rate_density_map->imageView != VK_NULL_HANDLE)) {
                if (!(pipeline_flags & VK_PIPELINE_RASTERIZATION_STATE_CREATE_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT)) {
                    const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline(),
                                                cb_state.activeRenderPass->renderPass());
                    skip |= LogError(objlist, vuid.dynamic_rendering_fdm,
                                     "%s: Currently bound graphics pipeline %s must have been created with "
                                     "VK_PIPELINE_RASTERIZATION_STATE_CREATE_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT",
                                     caller, report_data->FormatHandle(state.pipeline_state->pipeline()).c_str());
                }
            }
        }

        // VkAttachmentSampleCountInfoAMD == VkAttachmentSampleCountInfoNV
        auto p_attachment_sample_count_info = LvlFindInChain<VkAttachmentSampleCountInfoAMD>(pipeline.PNext());

        if (p_attachment_sample_count_info) {
            for (uint32_t i = 0; i < rendering_info.colorAttachmentCount; ++i) {
                if (rendering_info.pColorAttachments[i].imageView == VK_NULL_HANDLE) {
                    continue;
                }
                auto color_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pColorAttachments[i].imageView);
                auto color_image_samples = Get<IMAGE_STATE>(color_view_state->create_info.image)->createInfo.samples;

                if (p_attachment_sample_count_info &&
                    (color_image_samples != p_attachment_sample_count_info->pColorAttachmentSamples[i])) {
                    skip |= LogError(cb_state.commandBuffer(), vuid.dynamic_rendering_color_sample,
                                     "%s: Color attachment (%" PRIu32
                                     ") sample count (%s) must match corresponding VkAttachmentSampleCountInfoAMD "
                                     "sample count (%s)",
                                     caller, i, string_VkSampleCountFlagBits(color_image_samples),
                                     string_VkSampleCountFlagBits(p_attachment_sample_count_info->pColorAttachmentSamples[i]));
                }
            }

            if (rendering_info.pDepthAttachment != nullptr) {
                auto depth_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pDepthAttachment->imageView);
                auto depth_image_samples = Get<IMAGE_STATE>(depth_view_state->create_info.image)->createInfo.samples;

                if (p_attachment_sample_count_info) {
                    if (depth_image_samples != p_attachment_sample_count_info->depthStencilAttachmentSamples) {
                        skip |= LogError(
                            cb_state.commandBuffer(), vuid.dynamic_rendering_depth_sample,
                            "%s: Depth attachment sample count (%s) must match corresponding VkAttachmentSampleCountInfoAMD sample "
                            "count (%s)",
                            caller, string_VkSampleCountFlagBits(depth_image_samples),
                            string_VkSampleCountFlagBits(p_attachment_sample_count_info->depthStencilAttachmentSamples));
                    }
                }
            }

            if (rendering_info.pStencilAttachment != nullptr) {
                auto stencil_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pStencilAttachment->imageView);
                auto stencil_image_samples = Get<IMAGE_STATE>(stencil_view_state->create_info.image)->createInfo.samples;

                if (p_attachment_sample_count_info) {
                    if (stencil_image_samples != p_attachment_sample_count_info->depthStencilAttachmentSamples) {
                        skip |= LogError(
                            cb_state.commandBuffer(), vuid.dynamic_rendering_stencil_sample,
                            "%s: Stencil attachment sample count (%s) must match corresponding VkAttachmentSampleCountInfoAMD "
                            "sample count (%s)",
                            caller, string_VkSampleCountFlagBits(stencil_image_samples),
                            string_VkSampleCountFlagBits(p_attachment_sample_count_info->depthStencilAttachmentSamples));
                    }
                }
            }
        } else if (!enabled_features.multisampled_render_to_single_sampled_features.multisampledRenderToSingleSampled) {
            const VkSampleCountFlagBits rasterization_samples = cb_state.GetRasterizationSamples(pipeline);
            for (uint32_t i = 0; i < rendering_info.colorAttachmentCount; ++i) {
                if (rendering_info.pColorAttachments[i].imageView == VK_NULL_HANDLE) {
                    continue;
                }
                auto view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pColorAttachments[i].imageView);
                auto samples = Get<IMAGE_STATE>(view_state->create_info.image)->createInfo.samples;

                if (samples != rasterization_samples) {
                    const char *vuid_string = IsExtEnabled(device_extensions.vk_ext_multisampled_render_to_single_sampled)
                                                  ? vuid.dynamic_rendering_07285
                                                  : vuid.dynamic_rendering_multi_sample;
                    skip |= LogError(cb_state.commandBuffer(), vuid_string,
                                     "%s: Color attachment (%" PRIu32
                                     ") sample count (%s) must match corresponding VkPipelineMultisampleStateCreateInfo "
                                     "sample count (%s)",
                                     caller, i, string_VkSampleCountFlagBits(samples),
                                     string_VkSampleCountFlagBits(rasterization_samples));
                }
            }

            if ((rendering_info.pDepthAttachment != nullptr) && (rendering_info.pDepthAttachment->imageView != VK_NULL_HANDLE)) {
                const auto &depth_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pDepthAttachment->imageView);
                const auto &depth_image_samples = Get<IMAGE_STATE>(depth_view_state->create_info.image)->createInfo.samples;
                if (depth_image_samples != rasterization_samples) {
                    const char *vuid_string = IsExtEnabled(device_extensions.vk_ext_multisampled_render_to_single_sampled)
                                                  ? vuid.dynamic_rendering_07286
                                                  : vuid.dynamic_rendering_06189;
                    skip |= LogError(cb_state.commandBuffer(), vuid_string,
                                     "%s: Depth attachment sample count (%s) must match corresponding "
                                     "VkPipelineMultisampleStateCreateInfo::rasterizationSamples count (%s)",
                                     caller, string_VkSampleCountFlagBits(depth_image_samples),
                                     string_VkSampleCountFlagBits(rasterization_samples));
                }
            }

            if ((rendering_info.pStencilAttachment != nullptr) &&
                (rendering_info.pStencilAttachment->imageView != VK_NULL_HANDLE)) {
                const auto &stencil_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pStencilAttachment->imageView);
                const auto &stencil_image_samples = Get<IMAGE_STATE>(stencil_view_state->create_info.image)->createInfo.samples;
                if (stencil_image_samples != rasterization_samples) {
                    const char *vuid_string = IsExtEnabled(device_extensions.vk_ext_multisampled_render_to_single_sampled)
                                                  ? vuid.dynamic_rendering_07287
                                                  : vuid.dynamic_rendering_06190;
                    skip |= LogError(cb_state.commandBuffer(), vuid_string,
                                     "%s: Stencil attachment sample count (%s) must match corresponding "
                                     "VkPipelineMultisampleStateCreateInfo::rasterizationSamples count (%s)",
                                     caller, string_VkSampleCountFlagBits(stencil_image_samples),
                                     string_VkSampleCountFlagBits(rasterization_samples));
                }
            }
        }
    }

    if (IsExtEnabled(device_extensions.vk_ext_primitives_generated_query)) {
        bool primitives_generated_query_with_rasterizer_discard =
            enabled_features.primitives_generated_query_features.primitivesGeneratedQueryWithRasterizerDiscard == VK_TRUE;
        bool primitives_generated_query_with_non_zero_streams =
            enabled_features.primitives_generated_query_features.primitivesGeneratedQueryWithNonZeroStreams == VK_TRUE;
        if (!primitives_generated_query_with_rasterizer_discard || !primitives_generated_query_with_non_zero_streams) {
            bool primitives_generated_query = false;
            for (const auto &query : cb_state.activeQueries) {
                auto query_pool_state = Get<QUERY_POOL_STATE>(query.pool);
                if (query_pool_state && query_pool_state->createInfo.queryType == VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT) {
                    primitives_generated_query = true;
                    break;
                }
            }
            const auto rp_state = pipeline.RasterizationState();
            if (primitives_generated_query) {
                if (!primitives_generated_query_with_rasterizer_discard && rp_state &&
                    rp_state->rasterizerDiscardEnable == VK_TRUE) {
                    skip |= LogError(cb_state.commandBuffer(), vuid.primitives_generated,
                                     "%s: a VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT query is active and pipeline was created with "
                                     "VkPipelineRasterizationStateCreateInfo::rasterizerDiscardEnable set to VK_TRUE, but  "
                                     "primitivesGeneratedQueryWithRasterizerDiscard feature is not enabled.",
                                     caller);
                }
                if (!primitives_generated_query_with_non_zero_streams) {
                    const auto rasterization_state_stream_ci =
                        LvlFindInChain<VkPipelineRasterizationStateStreamCreateInfoEXT>(rp_state->pNext);
                    if (rasterization_state_stream_ci && rasterization_state_stream_ci->rasterizationStream != 0) {
                        skip |=
                            LogError(cb_state.commandBuffer(), vuid.primitives_generated_streams,
                                     "%s: a VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT query is active and pipeline was created with "
                                     "VkPipelineRasterizationStateStreamCreateInfoEXT::rasterizationStream set to %" PRIu32
                                     ", but  "
                                     "primitivesGeneratedQueryWithNonZeroStreams feature is not enabled.",
                                     caller, rasterization_state_stream_ci->rasterizationStream);
                    }
                }
            }
        }
    }

    // Verify vertex & index buffer for unprotected command buffer.
    // Because vertex & index buffer is read only, it doesn't need to care protected command buffer case.
    if (enabled_features.core11.protectedMemory == VK_TRUE) {
        for (const auto &buffer_binding : current_vtx_bfr_binding_info) {
            if (buffer_binding.buffer_state && !buffer_binding.buffer_state->Destroyed()) {
                skip |= ValidateProtectedBuffer(cb_state, *buffer_binding.buffer_state, caller, vuid.unprotected_command_buffer,
                                                "Buffer is vertex buffer");
            }
        }
        if (cb_state.index_buffer_binding.bound()) {
            skip |= ValidateProtectedBuffer(cb_state, *cb_state.index_buffer_binding.buffer_state, caller,
                                            vuid.unprotected_command_buffer, "Buffer is index buffer");
        }
    }

    // Verify vertex binding
    if (pipeline.vertex_input_state) {
        for (size_t i = 0; i < pipeline.vertex_input_state->binding_descriptions.size(); i++) {
            const auto vertex_binding = pipeline.vertex_input_state->binding_descriptions[i].binding;
            if (current_vtx_bfr_binding_info.size() < (vertex_binding + 1)) {
                skip |= LogError(cb_state.commandBuffer(), vuid.vertex_binding,
                                 "%s: %s expects that this Command Buffer's vertex binding Index %u should be set via "
                                 "vkCmdBindVertexBuffers. This is because pVertexBindingDescriptions[%zu].binding value is %u.",
                                 caller, report_data->FormatHandle(state.pipeline_state->pipeline()).c_str(), vertex_binding, i,
                                 vertex_binding);
            } else if ((current_vtx_bfr_binding_info[vertex_binding].buffer_state == nullptr) &&
                       !enabled_features.robustness2_features.nullDescriptor) {
                skip |= LogError(cb_state.commandBuffer(), vuid.vertex_binding_null,
                                 "%s: Vertex binding %d must not be VK_NULL_HANDLE %s expects that this Command Buffer's vertex "
                                 "binding Index %u should be set via "
                                 "vkCmdBindVertexBuffers. This is because pVertexBindingDescriptions[%zu].binding value is %u.",
                                 caller, vertex_binding, report_data->FormatHandle(state.pipeline_state->pipeline()).c_str(),
                                 vertex_binding, i, vertex_binding);
            }
        }

        // Verify vertex attribute address alignment
        for (size_t i = 0; i < pipeline.vertex_input_state->vertex_attribute_descriptions.size(); i++) {
            const auto &attribute_description = pipeline.vertex_input_state->vertex_attribute_descriptions[i];
            const auto vertex_binding = attribute_description.binding;
            const auto attribute_offset = attribute_description.offset;

            const auto &vertex_binding_map_it = pipeline.vertex_input_state->binding_to_index_map.find(vertex_binding);
            if ((vertex_binding_map_it != pipeline.vertex_input_state->binding_to_index_map.cend()) &&
                (vertex_binding < current_vtx_bfr_binding_info.size()) &&
                ((current_vtx_bfr_binding_info[vertex_binding].buffer_state) ||
                 enabled_features.robustness2_features.nullDescriptor)) {
                auto vertex_buffer_stride = pipeline.vertex_input_state->binding_descriptions[vertex_binding_map_it->second].stride;
                if (pipeline.IsDynamic(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT)) {
                    vertex_buffer_stride = static_cast<uint32_t>(current_vtx_bfr_binding_info[vertex_binding].stride);
                    uint32_t attribute_binding_extent =
                        attribute_description.offset + FormatElementSize(attribute_description.format);
                    if (vertex_buffer_stride != 0 && vertex_buffer_stride < attribute_binding_extent) {
                        skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdBindVertexBuffers2-pStrides-06209",
                                         "The pStrides[%u] (%u) parameter in the last call to %s is not 0 "
                                         "and less than the extent of the binding for attribute %zu (%u).",
                                         vertex_binding, vertex_buffer_stride, CommandTypeString(cmd_type), i,
                                         attribute_binding_extent);
                    }
                }
                const VkDeviceSize vertex_buffer_offset = current_vtx_bfr_binding_info[vertex_binding].offset;

                // Use 1 as vertex/instance index to use buffer stride as well
                const VkDeviceSize attrib_address = vertex_buffer_offset + vertex_buffer_stride + attribute_offset;

                const VkDeviceSize vtx_attrib_req_alignment = pipeline.vertex_input_state->vertex_attribute_alignments[i];

                if (SafeModulo(attrib_address, vtx_attrib_req_alignment) != 0) {
                    const LogObjectList objlist(current_vtx_bfr_binding_info[vertex_binding].buffer_state->buffer(),
                                                state.pipeline_state->pipeline());
                    skip |= LogError(objlist, vuid.vertex_binding_attribute,
                                     "%s: Format %s has an alignment of %" PRIu64 " but the alignment of attribAddress (%" PRIu64
                                     ") is not aligned in pVertexAttributeDescriptions[%zu]"
                                     "(binding=%u location=%u) where attribAddress = vertex buffer offset (%" PRIu64
                                     ") + binding stride (%u) + attribute offset (%u).",
                                     caller, string_VkFormat(attribute_description.format), vtx_attrib_req_alignment,
                                     attrib_address, i, vertex_binding, attribute_description.location, vertex_buffer_offset,
                                     vertex_buffer_stride, attribute_offset);
                }
            } else {
                const LogObjectList objlist(cb_state.commandBuffer(), state.pipeline_state->pipeline());
                skip |= LogError(objlist, vuid.vertex_binding_attribute,
                                 "%s: binding #%" PRIu32
                                 " in pVertexAttributeDescriptions[%zu]"
                                 " of %s is an invalid value for command buffer %s.",
                                 caller, vertex_binding, i, report_data->FormatHandle(state.pipeline_state->pipeline()).c_str(),
                                 report_data->FormatHandle(cb_state.commandBuffer()).c_str());
            }
        }
    }

    // Verify that any MSAA request in PSO matches sample# in bound FB
    // Verify that blend is enabled only if supported by subpasses image views format features
    // Skip the check if rasterization is disabled.
    const auto *raster_state = pipeline.RasterizationState();
    if (!raster_state || (raster_state->rasterizerDiscardEnable == VK_FALSE)) {
        if (cb_state.activeRenderPass) {
            if (cb_state.activeRenderPass->UsesDynamicRendering()) {
                // TODO: Mirror the below VUs but using dynamic rendering
                const auto dynamic_rendering_info = cb_state.activeRenderPass->dynamic_rendering_begin_rendering_info;
            } else {
                const auto render_pass_info = cb_state.activeRenderPass->createInfo.ptr();
                const VkSubpassDescription2 *subpass_desc = &render_pass_info->pSubpasses[cb_state.activeSubpass];
                uint32_t i;
                unsigned subpass_num_samples = 0;

                for (i = 0; i < subpass_desc->colorAttachmentCount; i++) {
                    const auto attachment = subpass_desc->pColorAttachments[i].attachment;
                    if (attachment != VK_ATTACHMENT_UNUSED) {
                        subpass_num_samples |= static_cast<unsigned>(render_pass_info->pAttachments[attachment].samples);

                        const auto *imageview_state = cb_state.GetActiveAttachmentImageViewState(attachment);
                        const auto *color_blend_state = pipeline.ColorBlendState();
                        if (imageview_state && color_blend_state && (attachment < color_blend_state->attachmentCount)) {
                            if ((imageview_state->format_features & VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BLEND_BIT_KHR) == 0 &&
                                color_blend_state->pAttachments[i].blendEnable != VK_FALSE) {
                                skip |=
                                    LogError(pipeline.pipeline(), vuid.blend_enable,
                                             "%s: Image view's format features of the color attachment (%" PRIu32
                                             ") of the active subpass do not contain VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT "
                                             "bit, but active pipeline's pAttachments[%" PRIu32 "].blendEnable is not VK_FALSE.",
                                             caller, attachment, attachment);
                            }
                        }
                    }
                }

                if (subpass_desc->pDepthStencilAttachment &&
                    subpass_desc->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
                    const auto attachment = subpass_desc->pDepthStencilAttachment->attachment;
                    subpass_num_samples |= static_cast<unsigned>(render_pass_info->pAttachments[attachment].samples);
                }

                const VkSampleCountFlagBits rasterization_samples = cb_state.GetRasterizationSamples(pipeline);
                if (!(IsExtEnabled(device_extensions.vk_amd_mixed_attachment_samples) ||
                      IsExtEnabled(device_extensions.vk_nv_framebuffer_mixed_samples) ||
                      enabled_features.multisampled_render_to_single_sampled_features.multisampledRenderToSingleSampled) &&
                    ((subpass_num_samples & static_cast<unsigned>(rasterization_samples)) != subpass_num_samples)) {
                    const LogObjectList objlist(pipeline.pipeline(), cb_state.activeRenderPass->renderPass());
                    const char *vuid_string = IsExtEnabled(device_extensions.vk_ext_multisampled_render_to_single_sampled)
                                                  ? vuid.msrtss_rasterization_samples
                                                  : vuid.rasterization_samples;
                    skip |= LogError(objlist, vuid_string,
                                     "%s: In %s the sample count is %s while the current %s has %s and they need to be the same.",
                                     caller, report_data->FormatHandle(pipeline.pipeline()).c_str(),
                                     string_VkSampleCountFlagBits(rasterization_samples),
                                     report_data->FormatHandle(cb_state.activeRenderPass->renderPass()).c_str(),
                                     string_VkSampleCountFlags(static_cast<VkSampleCountFlags>(subpass_num_samples)).c_str());
                }

                const bool dynamic_line_raster_mode = pipeline.IsDynamic(VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT);
                const bool dynamic_line_stipple_enable = pipeline.IsDynamic(VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT);
                if (dynamic_line_stipple_enable || dynamic_line_raster_mode) {
                    const auto raster_line_state =
                        LvlFindInChain<VkPipelineRasterizationLineStateCreateInfoEXT>(raster_state->pNext);

                    const VkLineRasterizationModeEXT line_rasterization_mode =
                        (dynamic_line_raster_mode) ? cb_state.dynamic_state_value.line_rasterization_mode
                                                   : raster_line_state->lineRasterizationMode;
                    const bool stippled_line_enable = (dynamic_line_stipple_enable)
                                                          ? cb_state.dynamic_state_value.stippled_line_enable
                                                          : raster_line_state->stippledLineEnable;

                    if (stippled_line_enable) {
                        if (line_rasterization_mode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT &&
                            (!enabled_features.line_rasterization_features.stippledRectangularLines)) {
                            skip |=
                                LogError(device, vuid.stippled_rectangular_lines,
                                         "%s(): lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT (set %s) with "
                                         "stippledLineEnable (set %s) but the stippledRectangularLines feature is not enabled.",
                                         caller, dynamic_line_raster_mode ? "dynamically" : "in pipeline",
                                         dynamic_line_stipple_enable ? "dynamically" : "in pipeline");
                        }
                        if (line_rasterization_mode == VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT &&
                            (!enabled_features.line_rasterization_features.stippledBresenhamLines)) {
                            skip |= LogError(device, vuid.stippled_bresenham_lines,
                                             "%s(): lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT (set %s) with "
                                             "stippledLineEnable (set %s) but the stippledBresenhamLines feature is not enabled.",
                                             caller, dynamic_line_raster_mode ? "dynamically" : "in pipeline",
                                             dynamic_line_stipple_enable ? "dynamically" : "in pipeline");
                        }
                        if (line_rasterization_mode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT &&
                            (!enabled_features.line_rasterization_features.stippledSmoothLines)) {
                            skip |= LogError(
                                device, vuid.stippled_smooth_lines,
                                "%s(): lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT (set %s) with "
                                "stippledLineEnable (set %s) but the stippledSmoothLines feature is not enabled.",
                                caller, dynamic_line_raster_mode ? "dynamically" : "in pipeline",
                                dynamic_line_stipple_enable ? "dynamically" : "in pipeline");
                        }
                        if (line_rasterization_mode == VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT &&
                            (!enabled_features.line_rasterization_features.stippledRectangularLines ||
                             !phys_dev_props.limits.strictLines)) {
                            skip |= LogError(
                                device, vuid.stippled_default_strict,
                                "%s(): lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT (set %s) with "
                                "stippledLineEnable (set %s), the stippledRectangularLines features is %s and strictLines is %s.",
                                caller, dynamic_line_raster_mode ? "dynamically" : "in pipeline",
                                dynamic_line_stipple_enable ? "dynamically" : "in pipeline",
                                enabled_features.line_rasterization_features.stippledRectangularLines ? "enabled" : "not enabled",
                                phys_dev_props.limits.strictLines ? "VK_TRUE" : "VK_FALSE");
                        }
                    }
                }
            }
        } else {
            skip |= LogError(pipeline.pipeline(), kVUID_Core_DrawState_NoActiveRenderpass,
                             "%s: No active render pass found at draw-time in %s!", caller,
                             report_data->FormatHandle(pipeline.pipeline()).c_str());
        }
    }
    // Verify that PSO creation renderPass is compatible with active renderPass
    if (cb_state.activeRenderPass && !cb_state.activeRenderPass->UsesDynamicRendering()) {
        const auto &rp_state = pipeline.RenderPassState();
        // TODO: AMD extension codes are included here, but actual function entrypoints are not yet intercepted
        if (cb_state.activeRenderPass->renderPass() != rp_state->renderPass()) {
            // renderPass that PSO was created with must be compatible with active renderPass that PSO is being used with
            skip |= ValidateRenderPassCompatibility("active render pass", *cb_state.activeRenderPass.get(), "pipeline state object",
                                                    *rp_state.get(), caller, vuid.render_pass_compatible);
        }
        const auto subpass = pipeline.Subpass();
        if (subpass != cb_state.activeSubpass) {
            skip |=
                LogError(pipeline.pipeline(), vuid.subpass_index, "%s: Pipeline was built for subpass %u but used in subpass %u.",
                         caller, subpass, cb_state.activeSubpass);
        }
        const safe_VkAttachmentReference2 *ds_attachment =
            cb_state.activeRenderPass->createInfo.pSubpasses[cb_state.activeSubpass].pDepthStencilAttachment;
        if (ds_attachment != nullptr) {
            // Check if depth stencil attachment was created with sample location compatible bit
            if (pipeline.SampleLocationEnabled() == VK_TRUE) {
                const uint32_t attachment = ds_attachment->attachment;
                if (attachment != VK_ATTACHMENT_UNUSED) {
                    const auto *imageview_state = cb_state.GetActiveAttachmentImageViewState(attachment);
                    if (imageview_state != nullptr) {
                        const auto *image_state = imageview_state->image_state.get();
                        if (image_state != nullptr) {
                            if ((image_state->createInfo.flags & VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT) == 0) {
                                skip |= LogError(pipeline.pipeline(), vuid.sample_location,
                                                 "%s: sampleLocationsEnable is true for the pipeline, but the subpass (%u) depth "
                                                 "stencil attachment's VkImage was not created with "
                                                 "VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT.",
                                                 caller, cb_state.activeSubpass);
                            }
                        }
                    }
                }
            }
            const auto ds_state = pipeline.DepthStencilState();
            if (ds_state) {
                // Set with static values and update for anything dynamically set
                const bool depth_write_enable = pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE)
                                                    ? dynamic_state_value.depth_write_enable
                                                    : ds_state->depthWriteEnable;
                VkStencilOpState front = ds_state->front;
                VkStencilOpState back = ds_state->back;

                if (pipeline.IsDynamic(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK)) {
                    front.writeMask = dynamic_state_value.write_mask_front;
                    back.writeMask = dynamic_state_value.write_mask_back;
                }
                if (pipeline.IsDynamic(VK_DYNAMIC_STATE_STENCIL_OP)) {
                    front.failOp = dynamic_state_value.fail_op_front;
                    front.passOp = dynamic_state_value.pass_op_front;
                    front.depthFailOp = dynamic_state_value.depth_fail_op_front;
                    back.failOp = dynamic_state_value.fail_op_back;
                    back.passOp = dynamic_state_value.pass_op_back;
                    back.depthFailOp = dynamic_state_value.depth_fail_op_back;
                }

                if (depth_write_enable == VK_TRUE && IsImageLayoutDepthReadOnly(ds_attachment->layout)) {
                    const LogObjectList objlist(pipeline.pipeline(), cb_state.activeRenderPass->renderPass(),
                                                cb_state.commandBuffer());
                    skip |= LogError(objlist, vuid.depth_read_only,
                                     "%s: depthWriteEnable is VK_TRUE, while the layout (%s) of "
                                     "the depth aspect of the depth/stencil attachment in the render pass is read only.",
                                     caller, string_VkImageLayout(ds_attachment->layout));
                }

                const bool all_keep_op = ((front.failOp == VK_STENCIL_OP_KEEP) && (front.passOp == VK_STENCIL_OP_KEEP) &&
                                          (front.depthFailOp == VK_STENCIL_OP_KEEP) && (back.failOp == VK_STENCIL_OP_KEEP) &&
                                          (back.passOp == VK_STENCIL_OP_KEEP) && (back.depthFailOp == VK_STENCIL_OP_KEEP));

                const bool write_mask_enabled = (front.writeMask != 0) && (back.writeMask != 0);

                if (IsImageLayoutStencilReadOnly(ds_attachment->layout) && !all_keep_op && write_mask_enabled) {
                    const LogObjectList objlist(pipeline.pipeline(), cb_state.activeRenderPass->renderPass(),
                                                cb_state.commandBuffer());
                    skip |= LogError(objlist, vuid.stencil_read_only,
                                     "%s: The layout (%s) of the stencil aspect of the depth/stencil attachment in the render pass "
                                     "is read only but not all stencil ops are VK_STENCIL_OP_KEEP.\n"
                                     "front = { .failOp = %s,  .passOp = %s , .depthFailOp = %s }\n"
                                     "back = { .failOp = %s, .passOp = %s, .depthFailOp = %s }\n",
                                     caller, string_VkImageLayout(ds_attachment->layout), string_VkStencilOp(front.failOp),
                                     string_VkStencilOp(front.passOp), string_VkStencilOp(front.depthFailOp),
                                     string_VkStencilOp(back.failOp), string_VkStencilOp(back.passOp),
                                     string_VkStencilOp(back.depthFailOp));
                }
            }
        }
    }

    if (pipeline.fragment_output_state->dual_source_blending && cb_state.activeRenderPass) {
        uint32_t count = cb_state.activeRenderPass->UsesDynamicRendering()
                             ? cb_state.activeRenderPass->dynamic_rendering_begin_rendering_info.colorAttachmentCount
                             : cb_state.activeRenderPass->createInfo.pSubpasses[cb_state.activeSubpass].colorAttachmentCount;
        if (count > phys_dev_props.limits.maxFragmentDualSrcAttachments) {
            skip |=
                LogError(pipeline.pipeline(), "VUID-RuntimeSpirv-Fragment-06427",
                         "%s: Dual source blend mode is used, but the number of written fragment shader output attachment (%" PRIu32
                         ") is greater than maxFragmentDualSrcAttachments (%" PRIu32 ")",
                         caller, count, phys_dev_props.limits.maxFragmentDualSrcAttachments);
        }
    }

    if (enabled_features.fragment_shading_rate_features.primitiveFragmentShadingRate) {
        skip |= ValidateGraphicsPipelineShaderDynamicState(pipeline, cb_state, caller, vuid);
    }

    return skip;
}
