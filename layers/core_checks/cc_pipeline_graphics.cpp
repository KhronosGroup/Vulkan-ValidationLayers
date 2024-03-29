/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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

#include <vulkan/vk_enum_string_helper.h>
#include "generated/chassis.h"
#include "core_validation.h"
#include "generated/enum_flag_bits.h"
#include "drawdispatch/drawdispatch_vuids.h"
#include "state_tracker/image_state.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/shader_object_state.h"
#include "chassis/chassis_modification_state.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/render_pass_state.h"

bool CoreChecks::PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                        const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                        const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                        const ErrorObject &error_obj, PipelineStates &pipeline_states,
                                                        chassis::CreateGraphicsPipelines &chassis_state) const {
    bool skip = StateTracker::PreCallValidateCreateGraphicsPipelines(device, pipelineCache, count, pCreateInfos, pAllocator,
                                                                     pPipelines, error_obj, pipeline_states, chassis_state);

    for (uint32_t i = 0; i < count; i++) {
        const Location create_info_loc = error_obj.location.dot(Field::pCreateInfos, i);
        skip |= ValidateGraphicsPipeline(*pipeline_states[i].get(), create_info_loc);
        skip |= ValidateGraphicsPipelineDerivatives(pipeline_states, i, create_info_loc);
    }
    return skip;
}

bool CoreChecks::ValidateGraphicsPipeline(const vvl::Pipeline &pipeline, const Location &create_info_loc) const {
    bool skip = false;
    vku::safe_VkSubpassDescription2 *subpass_desc = nullptr;

    const auto &rp_state = pipeline.RenderPassState();
    const auto subpass = pipeline.Subpass();
    if (rp_state && !rp_state->UsesDynamicRendering()) {
        // Ensure the subpass index is valid. If not, then ValidateGraphicsPipelineShaderState
        // produces nonsense errors that confuse users. Other layers should already
        // emit errors for renderpass being invalid.
        subpass_desc = &rp_state->create_info.pSubpasses[subpass];
        if (subpass >= rp_state->create_info.subpassCount) {
            skip |=
                LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06046", rp_state->Handle(),
                         create_info_loc.dot(Field::subpass), "(%" PRIu32 ") is out of range for this renderpass (0..%" PRIu32 ").",
                         subpass, rp_state->create_info.subpassCount - 1);
            subpass_desc = nullptr;
        }

        // Check for portability errors
        // Issue raised in https://gitlab.khronos.org/vulkan/vulkan/-/issues/3436
        // The combination of GPL/DynamicRendering and Portability has spec issues that need to be clarified
        if (IsExtEnabled(device_extensions.vk_khr_portability_subset) && !pipeline.IsGraphicsLibrary()) {
            skip |= ValidateGraphicsPipelinePortability(pipeline, create_info_loc);
        }
    }

    // While these are split into the 4 sub-states from GPL, they are validated for normal pipelines too.
    // These are VUs that fall strangely between both GPL and non-GPL pipelines
    skip |= ValidateGraphicsPipelineVertexInputState(pipeline, create_info_loc);
    skip |= ValidateGraphicsPipelinePreRasterizationState(pipeline, create_info_loc);

    skip |= ValidateGraphicsPipelineRenderPass(pipeline, create_info_loc);
    skip |= ValidateGraphicsPipelineLibrary(pipeline, create_info_loc);
    skip |= ValidateGraphicsPipelineInputAssemblyState(pipeline, create_info_loc);
    skip |= ValidateGraphicsPipelineTessellationState(pipeline, create_info_loc);
    skip |= ValidateGraphicsPipelineColorBlendState(pipeline, subpass_desc, create_info_loc);
    skip |= ValidateGraphicsPipelineRasterizationState(pipeline, subpass_desc, create_info_loc);
    skip |= ValidateGraphicsPipelineMultisampleState(pipeline, subpass_desc, create_info_loc);
    skip |= ValidateGraphicsPipelineDepthStencilState(pipeline, subpass_desc, create_info_loc);
    skip |= ValidateGraphicsPipelineDynamicState(pipeline, create_info_loc);
    skip |= ValidateGraphicsPipelineFragmentShadingRateState(pipeline, create_info_loc);
    skip |= ValidateGraphicsPipelineDynamicRendering(pipeline, create_info_loc);
    skip |= ValidateGraphicsPipelineShaderState(pipeline, create_info_loc);
    skip |= ValidateGraphicsPipelineBlendEnable(pipeline, create_info_loc);
    skip |= ValidateGraphicsPipelineMeshTask(pipeline, create_info_loc);
    skip |= ValidateGraphicsPipelineExternalFormatResolve(pipeline, subpass_desc, create_info_loc);

    if (pipeline.OwnsSubState(pipeline.pre_raster_state) || pipeline.OwnsSubState(pipeline.fragment_shader_state)) {
        vvl::unordered_map<VkShaderStageFlags, uint32_t> unique_stage_map;
        uint32_t index = 0;
        for (const auto &stage_ci : pipeline.shader_stages_ci) {
            auto it = unique_stage_map.find(stage_ci.stage);
            if (it != unique_stage_map.end()) {
                skip |=
                    LogError("VUID-VkGraphicsPipelineCreateInfo-stage-06897", device, create_info_loc.dot(Field::pStages, index),
                             "and pStages[%" PRIu32 "] both have %s", it->second, string_VkShaderStageFlagBits(stage_ci.stage));
            }
            unique_stage_map[stage_ci.stage] = index;
            index++;
        }
    }

    skip |= ValidateShaderModuleId(pipeline, create_info_loc);
    skip |= ValidatePipelineCacheControlFlags(pipeline.create_flags, create_info_loc.dot(Field::flags),
                                              "VUID-VkGraphicsPipelineCreateInfo-pipelineCreationCacheControl-02878");
    skip |= ValidatePipelineProtectedAccessFlags(pipeline.create_flags, create_info_loc.dot(Field::flags));

    const auto *discard_rectangle_state =
        vku::FindStructInPNextChain<VkPipelineDiscardRectangleStateCreateInfoEXT>(pipeline.GraphicsCreateInfo().pNext);
    if (discard_rectangle_state) {
        if (discard_rectangle_state->discardRectangleCount > phys_dev_ext_props.discard_rectangle_props.maxDiscardRectangles) {
            skip |= LogError(
                "VUID-VkPipelineDiscardRectangleStateCreateInfoEXT-discardRectangleCount-00582", device,
                create_info_loc.pNext(Struct::VkPipelineDiscardRectangleStateCreateInfoEXT, Field::discardRectangleCount),
                "(%" PRIu32 ") is not less than maxDiscardRectangles (%" PRIu32 ").",
                discard_rectangle_state->discardRectangleCount, phys_dev_ext_props.discard_rectangle_props.maxDiscardRectangles);
        }
    }

    // VkAttachmentSampleCountInfoAMD == VkAttachmentSampleCountInfoNV
    const auto &pipeline_ci = pipeline.GraphicsCreateInfo();
    const auto attachment_sample_count_info = vku::FindStructInPNextChain<VkAttachmentSampleCountInfoAMD>(pipeline_ci.pNext);
    if (attachment_sample_count_info) {
        const uint32_t bits = GetBitSetCount(attachment_sample_count_info->depthStencilAttachmentSamples);
        if (pipeline.fragment_output_state && attachment_sample_count_info->depthStencilAttachmentSamples != 0 &&
            ((attachment_sample_count_info->depthStencilAttachmentSamples & AllVkSampleCountFlagBits) == 0 || bits > 1)) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-depthStencilAttachmentSamples-06593", device,
                             create_info_loc.pNext(Struct::VkAttachmentSampleCountInfoAMD, Field::depthStencilAttachmentSamples),
                             "(0x%" PRIx32 ") is invalid.", attachment_sample_count_info->depthStencilAttachmentSamples);
        }
    }

    if (const auto *pipeline_robustness_info = vku::FindStructInPNextChain<VkPipelineRobustnessCreateInfoEXT>(pipeline_ci.pNext);
        pipeline_robustness_info) {
        skip |= ValidatePipelineRobustnessCreateInfo(pipeline, *pipeline_robustness_info, create_info_loc);
    }
    return skip;
}

bool CoreChecks::ValidateGraphicsPipelinePortability(const vvl::Pipeline &pipeline, const Location &create_info_loc) const {
    bool skip = false;
    if (!enabled_features.triangleFans && (pipeline.topology_at_rasterizer == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)) {
        skip |= LogError("VUID-VkPipelineInputAssemblyStateCreateInfo-triangleFans-04452", device, create_info_loc,
                         "(portability error): VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN is not supported.");
    }

    if (!pipeline.IsDynamic(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT) &&
        !pipeline.IsDynamic(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE)) {
        // Validate vertex inputs
        for (const auto &desc : pipeline.vertex_input_state->binding_descriptions) {
            const uint32_t min_alignment = phys_dev_ext_props.portability_props.minVertexInputBindingStrideAlignment;
            if (min_alignment != 0 && (desc.stride % min_alignment != 0)) {
                skip |= LogError(
                    "VUID-VkVertexInputBindingDescription-stride-04456", device, create_info_loc,
                    "(portability error): Vertex input stride (%" PRIu32
                    ") must be at least as large as and a "
                    "multiple of VkPhysicalDevicePortabilitySubsetPropertiesKHR::minVertexInputBindingStrideAlignment (%" PRIu32
                    ").",
                    desc.stride, min_alignment);
            }
        }

        // Validate vertex attributes
        if (!enabled_features.vertexAttributeAccessBeyondStride) {
            for (const auto &attrib : pipeline.vertex_input_state->vertex_attribute_descriptions) {
                const auto vertex_binding_map_it = pipeline.vertex_input_state->binding_to_index_map.find(attrib.binding);
                if (vertex_binding_map_it != pipeline.vertex_input_state->binding_to_index_map.cend()) {
                    const auto &desc = pipeline.vertex_input_state->binding_descriptions[vertex_binding_map_it->second];
                    if ((attrib.offset + vkuFormatElementSize(attrib.format)) > desc.stride) {
                        skip |= LogError("VUID-VkVertexInputAttributeDescription-vertexAttributeAccessBeyondStride-04457", device,
                                         create_info_loc,
                                         "(portability error): attribute.offset (%" PRIu32
                                         ") + "
                                         "sizeof(vertex_description.format) (%" PRIu32
                                         ") is larger than the vertex stride (%" PRIu32 ").",
                                         attrib.offset, vkuFormatElementSize(attrib.format), desc.stride);
                    }
                }
            }
        }
    }

    auto raster_state_ci = pipeline.RasterizationState();
    if (raster_state_ci) {
        // Validate polygon mode
        if (!enabled_features.pointPolygons && !raster_state_ci->rasterizerDiscardEnable &&
            (raster_state_ci->polygonMode == VK_POLYGON_MODE_POINT)) {
            skip |= LogError("VUID-VkPipelineRasterizationStateCreateInfo-pointPolygons-04458", device, create_info_loc,
                             "(portability error): point polygons are not supported.");
        }

        // Validate depth-stencil state
        if (!enabled_features.separateStencilMaskRef && (raster_state_ci->cullMode == VK_CULL_MODE_NONE)) {
            const auto ds_state = pipeline.DepthStencilState();
            if (ds_state && ds_state->stencilTestEnable && (ds_state->front.reference != ds_state->back.reference)) {
                skip |= LogError("VUID-VkPipelineDepthStencilStateCreateInfo-separateStencilMaskRef-04453", device, create_info_loc,
                                 "(portability error): VkStencilOpState::reference must be the "
                                 "same for front and back.");
            }
        }

        // Validate color attachments
        const uint32_t subpass = pipeline.Subpass();
        auto render_pass = Get<vvl::RenderPass>(pipeline.GraphicsCreateInfo().renderPass);
        const bool ignore_color_blend_state =
            raster_state_ci->rasterizerDiscardEnable ||
            (pipeline.rendering_create_info ? (pipeline.rendering_create_info->colorAttachmentCount == 0)
                                            : (render_pass->create_info.pSubpasses[subpass].colorAttachmentCount == 0));
        const auto *color_blend_state = pipeline.ColorBlendState();
        if (!enabled_features.constantAlphaColorBlendFactors && !ignore_color_blend_state && color_blend_state) {
            const auto attachments = color_blend_state->pAttachments;
            for (uint32_t color_attachment_index = 0; color_attachment_index < color_blend_state->attachmentCount;
                 ++color_attachment_index) {
                if ((attachments[color_attachment_index].srcColorBlendFactor == VK_BLEND_FACTOR_CONSTANT_ALPHA) ||
                    (attachments[color_attachment_index].srcColorBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA)) {
                    skip |= LogError("VUID-VkPipelineColorBlendAttachmentState-constantAlphaColorBlendFactors-04454", device,
                                     create_info_loc,
                                     "(portability error): srcColorBlendFactor for color attachment %" PRIu32
                                     " must "
                                     "not be VK_BLEND_FACTOR_CONSTANT_ALPHA or VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA",
                                     color_attachment_index);
                }
                if ((attachments[color_attachment_index].dstColorBlendFactor == VK_BLEND_FACTOR_CONSTANT_ALPHA) ||
                    (attachments[color_attachment_index].dstColorBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA)) {
                    skip |= LogError("VUID-VkPipelineColorBlendAttachmentState-constantAlphaColorBlendFactors-04455", device,
                                     create_info_loc,
                                     "(portability error): dstColorBlendFactor for color attachment %" PRIu32
                                     " must "
                                     "not be VK_BLEND_FACTOR_CONSTANT_ALPHA or VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA",
                                     color_attachment_index);
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidatePipelineLibraryCreateInfo(const vvl::Pipeline &pipeline,
                                                   const VkPipelineLibraryCreateInfoKHR &library_create_info,
                                                   const Location &create_info_loc) const {
    bool skip = false;

    const VkPipelineCreateFlags2KHR pipeline_flags = pipeline.create_flags;
    const bool has_link_time_opt = (pipeline_flags & VK_PIPELINE_CREATE_2_LINK_TIME_OPTIMIZATION_BIT_EXT) != 0;
    const bool has_retain_link_time_opt = (pipeline_flags & VK_PIPELINE_CREATE_2_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT) != 0;
    const bool has_capture_internal = (pipeline_flags & VK_PIPELINE_CREATE_2_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR) != 0;
    bool uses_descriptor_buffer = false;
    bool lib_all_has_capture_internal = false;

    const auto gpl_info = vku::FindStructInPNextChain<VkGraphicsPipelineLibraryCreateInfoEXT>(pipeline.GraphicsCreateInfo().pNext);

    for (uint32_t i = 0; i < library_create_info.libraryCount; ++i) {
        const auto lib = Get<vvl::Pipeline>(library_create_info.pLibraries[i]);
        if (!lib) {
            continue;
        }
        const Location &library_loc = create_info_loc.pNext(Struct::VkPipelineLibraryCreateInfoKHR, Field::pLibraries, i);
        const VkPipelineCreateFlags2KHR lib_pipeline_flags = lib->create_flags;

        const bool lib_has_retain_link_time_opt =
            (lib_pipeline_flags & VK_PIPELINE_CREATE_2_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT) != 0;
        if (has_link_time_opt && !lib_has_retain_link_time_opt) {
            const LogObjectList objlist(device, lib->Handle());
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-06609", objlist, library_loc,
                             "(%s) was created with %s, which is missing "
                             "VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT, %s is %s.",
                             string_VkGraphicsPipelineLibraryFlagsEXT(lib->graphics_lib_type).c_str(),
                             string_VkPipelineCreateFlags2KHR(lib_pipeline_flags).c_str(),
                             create_info_loc.dot(Field::flags).Fields().c_str(),
                             string_VkPipelineCreateFlags2KHR(pipeline_flags).c_str());
        }

        if (has_retain_link_time_opt && !lib_has_retain_link_time_opt) {
            const LogObjectList objlist(device, lib->Handle());
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-06610", objlist, library_loc,
                             "(%s) was created with %s, which is missing "
                             "VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT, %s is %s.",
                             string_VkGraphicsPipelineLibraryFlagsEXT(lib->graphics_lib_type).c_str(),
                             string_VkPipelineCreateFlags2KHR(lib_pipeline_flags).c_str(),
                             create_info_loc.dot(Field::flags).Fields().c_str(),
                             string_VkPipelineCreateFlags2KHR(pipeline_flags).c_str());
        }

        const bool lib_has_capture_internal =
            (lib_pipeline_flags & VK_PIPELINE_CREATE_2_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR) != 0;
        const bool non_zero_gpl = gpl_info && gpl_info->flags != 0;
        if (lib_has_capture_internal) {
            lib_all_has_capture_internal = true;
            if (!has_capture_internal && non_zero_gpl) {
                const Location &gpl_flags_loc = create_info_loc.pNext(Struct::VkGraphicsPipelineLibraryCreateInfoEXT, Field::flags);
                const LogObjectList objlist(device, lib->Handle());
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pLibraries-06647", objlist, library_loc,
                                 "(%s) was created with %s\n"
                                 "%s is %s\n"
                                 "%s is %s.",
                                 string_VkGraphicsPipelineLibraryFlagsEXT(lib->graphics_lib_type).c_str(),
                                 string_VkPipelineCreateFlags2KHR(lib_pipeline_flags).c_str(), gpl_flags_loc.Fields().c_str(),
                                 string_VkPipelineCreateFlags2KHR(gpl_info->flags).c_str(),
                                 create_info_loc.dot(Field::flags).Fields().c_str(),
                                 string_VkPipelineCreateFlags2KHR(pipeline_flags).c_str());
            }
        } else {
            if (lib_all_has_capture_internal) {
                const LogObjectList objlist(device, lib->Handle());
                skip |=
                    LogError("VUID-VkGraphicsPipelineCreateInfo-pLibraries-06646", objlist, library_loc,
                             "(%s) was created with %s.", string_VkGraphicsPipelineLibraryFlagsEXT(lib->graphics_lib_type).c_str(),
                             string_VkPipelineCreateFlags2KHR(lib_pipeline_flags).c_str());
            } else if (has_capture_internal && non_zero_gpl) {
                const Location &gpl_flags_loc = create_info_loc.pNext(Struct::VkGraphicsPipelineLibraryCreateInfoEXT, Field::flags);
                const LogObjectList objlist(device, lib->Handle());
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-06645", objlist, library_loc,
                                 "(%s) was created with %s\n"
                                 "%s is %s\n"
                                 "%s is %s.",
                                 string_VkGraphicsPipelineLibraryFlagsEXT(lib->graphics_lib_type).c_str(),
                                 string_VkPipelineCreateFlags2KHR(lib_pipeline_flags).c_str(), gpl_flags_loc.Fields().c_str(),
                                 string_VkPipelineCreateFlags2KHR(gpl_info->flags).c_str(),
                                 create_info_loc.dot(Field::flags).Fields().c_str(),
                                 string_VkPipelineCreateFlags2KHR(pipeline_flags).c_str());
            }
        }

        if ((lib->uses_shader_module_id) && !(pipeline_flags & VK_PIPELINE_CREATE_2_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_KHR)) {
            const LogObjectList objlist(device);
            skip |= LogError("VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-06855", objlist, library_loc,
                             "(%s) was created with %s but VkPipelineShaderStageModuleIdentifierCreateInfoEXT::identifierSize was "
                             "not equal to 0 for the pipeline",
                             string_VkGraphicsPipelineLibraryFlagsEXT(lib->graphics_lib_type).c_str(),
                             string_VkPipelineCreateFlags2KHR(lib_pipeline_flags).c_str());
        }
        struct check_struct {
            VkPipelineCreateFlagBits2KHR bit;
            std::string first_vuid;
            std::string second_vuid;
        };
        static const std::array<check_struct, 2> check_infos = {
            {{VK_PIPELINE_CREATE_2_NO_PROTECTED_ACCESS_BIT_EXT, "VUID-VkPipelineLibraryCreateInfoKHR-pipeline-07404",
              "VUID-VkPipelineLibraryCreateInfoKHR-pipeline-07405"},
             {VK_PIPELINE_CREATE_2_PROTECTED_ACCESS_ONLY_BIT_EXT, "VUID-VkPipelineLibraryCreateInfoKHR-pipeline-07406",
              "VUID-VkPipelineLibraryCreateInfoKHR-pipeline-07407"}}};
        for (const auto &check_info : check_infos) {
            if ((pipeline_flags & check_info.bit)) {
                if (!(lib_pipeline_flags & check_info.bit)) {
                    const LogObjectList objlist(device, lib->Handle());
                    skip |= LogError(check_info.first_vuid.c_str(), objlist, library_loc,
                                     "(%s) was created with %s, which is missing %s included in %s (%s).",
                                     string_VkGraphicsPipelineLibraryFlagsEXT(lib->graphics_lib_type).c_str(),
                                     string_VkPipelineCreateFlags2KHR(lib_pipeline_flags).c_str(),
                                     string_VkPipelineCreateFlagBits2KHR(check_info.bit),
                                     create_info_loc.dot(Field::flags).Fields().c_str(),
                                     string_VkPipelineCreateFlags2KHR(pipeline_flags).c_str());
                }
            } else {
                if ((lib_pipeline_flags & check_info.bit)) {
                    const LogObjectList objlist(device, lib->Handle());
                    skip |= LogError(check_info.second_vuid.c_str(), objlist, library_loc,
                                     "(%s) was created with %s, which includes %s not included in %s (%s).",
                                     string_VkGraphicsPipelineLibraryFlagsEXT(lib->graphics_lib_type).c_str(),
                                     string_VkPipelineCreateFlags2KHR(lib_pipeline_flags).c_str(),
                                     string_VkPipelineCreateFlagBits2KHR(check_info.bit),
                                     create_info_loc.dot(Field::flags).Fields().c_str(),
                                     string_VkPipelineCreateFlags2KHR(pipeline_flags).c_str());
                }
            }
        }

        if (i == 0) {
            uses_descriptor_buffer = lib->descriptor_buffer_mode;
        } else if (uses_descriptor_buffer != lib->descriptor_buffer_mode) {
            skip |= LogError("VUID-VkPipelineLibraryCreateInfoKHR-pLibraries-08096", device, library_loc,
                             "%s created with VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT which is opopposite of pLibraries[0].",
                             lib->descriptor_buffer_mode ? "was" : "was not");
            break;  // no point keep checking as might have many of same error
        }
    }

    if (pipeline.GraphicsCreateInfo().renderPass == VK_NULL_HANDLE) {
        if (gpl_info) {
            skip |= ValidatePipelineLibraryFlags(gpl_info->flags, library_create_info, pipeline.rendering_create_info,
                                                 create_info_loc, -1, "VUID-VkGraphicsPipelineCreateInfo-flags-06626");

            const uint32_t flags_count =
                GetBitSetCount(gpl_info->flags & (VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
                                                  VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
                                                  VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT));
            if (flags_count >= 1 && flags_count <= 2) {
                for (uint32_t i = 0; i < library_create_info.libraryCount; ++i) {
                    const auto lib = Get<vvl::Pipeline>(library_create_info.pLibraries[i]);
                    const auto lib_gpl_info =
                        vku::FindStructInPNextChain<VkGraphicsPipelineLibraryCreateInfoEXT>(lib->GraphicsCreateInfo().pNext);
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
        for (uint32_t i = 0; i < library_create_info.libraryCount; ++i) {
            const auto lib = Get<vvl::Pipeline>(library_create_info.pLibraries[i]);
            const auto lib_rendering_struct = lib->GetPipelineRenderingCreateInfo();
            skip |= ValidatePipelineLibraryFlags(lib->graphics_lib_type, library_create_info, lib_rendering_struct, create_info_loc,
                                                 i, "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06627");
        }
    }

    return skip;
}

// Using memcmp will fail sometimes as the alignment/padding in the struct can have undefined values, so because we can't ensure the
// memory was zero-intialized, we need to just do a normal compare on each member
static bool ComparePipelineMultisampleStateCreateInfo(const VkPipelineMultisampleStateCreateInfo &a,
                                                      const VkPipelineMultisampleStateCreateInfo &b) {
    bool valid_mask = true;
    if (a.pSampleMask && b.pSampleMask && (a.rasterizationSamples == b.rasterizationSamples)) {
        uint32_t length = (SampleCountSize(a.rasterizationSamples) + 31) / 32;
        for (uint32_t i = 0; i < length; i++) {
            if (a.pSampleMask[i] != b.pSampleMask[i]) {
                valid_mask = false;
                break;
            }
        }
    } else if (a.pSampleMask || b.pSampleMask) {
        valid_mask = false;  // one is not null
    }

    // TODO - to do a deep pNext chain check would require us generating these compare functions for all structs
    // For now, just check if pNext both null or not
    const bool valid_pNext = (a.pNext && b.pNext) || (a.pNext == b.pNext);

    return (a.sType == b.sType) && (valid_pNext) && (a.flags == b.flags) && (a.rasterizationSamples == b.rasterizationSamples) &&
           (a.sampleShadingEnable == b.sampleShadingEnable) && (a.minSampleShading == b.minSampleShading) && (valid_mask) &&
           (a.alphaToCoverageEnable == b.alphaToCoverageEnable) && (a.alphaToOneEnable == b.alphaToOneEnable);
}

static bool CompareDescriptorSetLayoutBinding(const VkDescriptorSetLayoutBinding &a, const VkDescriptorSetLayoutBinding &b) {
    return (a.binding == b.binding) && (a.descriptorType == b.descriptorType) && (a.descriptorCount == b.descriptorCount) &&
           (a.stageFlags == b.stageFlags) && (a.pImmutableSamplers == b.pImmutableSamplers);
}

static bool ComparePipelineColorBlendAttachmentState(const VkPipelineColorBlendAttachmentState &a,
                                                     const VkPipelineColorBlendAttachmentState &b) {
    return (a.blendEnable == b.blendEnable) && (a.srcColorBlendFactor == b.srcColorBlendFactor) &&
           (a.dstColorBlendFactor == b.dstColorBlendFactor) && (a.colorBlendOp == b.colorBlendOp) &&
           (a.srcAlphaBlendFactor == b.srcAlphaBlendFactor) && (a.dstAlphaBlendFactor == b.dstAlphaBlendFactor) &&
           (a.alphaBlendOp == b.alphaBlendOp) && (a.colorWriteMask == b.colorWriteMask);
}

static bool ComparePipelineFragmentShadingRateStateCreateInfo(const VkPipelineFragmentShadingRateStateCreateInfoKHR &a,
                                                              const VkPipelineFragmentShadingRateStateCreateInfoKHR &b) {
    // Since this is chained in a pnext, we don't want to check the pNext/sType
    return (a.fragmentSize.width == b.fragmentSize.width) && (a.fragmentSize.height == b.fragmentSize.height) &&
           (a.combinerOps[0] == b.combinerOps[0]) && (a.combinerOps[1] == b.combinerOps[1]);
}

// vkspec.html#pipelines-graphics-subsets-vertex-input
bool CoreChecks::ValidateGraphicsPipelineVertexInputState(const vvl::Pipeline &pipeline, const Location &create_info_loc) const {
    bool skip = false;
    // If using a mesh shader, all vertex input is ignored
    if (!pipeline.OwnsSubState(pipeline.vertex_input_state) || (pipeline.active_shaders & VK_SHADER_STAGE_MESH_BIT_EXT)) {
        return skip;
    }

    const bool ignore_vertex_input_state = pipeline.IsDynamic(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT);
    const bool ignore_input_assembly_state = IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state3) &&
                                             pipeline.IsDynamic(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE) &&
                                             pipeline.IsDynamic(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY) &&
                                             phys_dev_ext_props.extended_dynamic_state3_props.dynamicPrimitiveTopologyUnrestricted;

    if (!ignore_vertex_input_state) {
        skip |= ValidatePipelineVertexDivisors(pipeline, create_info_loc);
    }

    const auto *input_state = pipeline.InputState();
    const auto *assembly_state = pipeline.InputAssemblyState();
    const bool invalid_input_state = !ignore_vertex_input_state && !input_state;
    const bool invalid_assembly_state = !ignore_input_assembly_state && !assembly_state;

    if (invalid_input_state && invalid_assembly_state && pipeline.IsGraphicsLibrary()) {
        // Failed to defined a Vertex Input State
        if (!pipeline.pre_raster_state) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-08898", device, create_info_loc,
                             "pVertexInputState and pInputAssemblyState are both NULL so this is an invalid Vertex Input State (no "
                             "dynamic state or mesh shaders were used to ignore them).");
        } else if ((pipeline.active_shaders & VK_SHADER_STAGE_VERTEX_BIT)) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-08897", device, create_info_loc,
                             "pVertexInputState and pInputAssemblyState are both NULL so this is an invalid Vertex Input State (no "
                             "dynamic state were used to ignore them).");
        }
    } else if (invalid_input_state) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pStages-02097", device, create_info_loc.dot(Field::pVertexInputState),
                         "is NULL.");
    } else if (invalid_assembly_state) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-dynamicPrimitiveTopologyUnrestricted-09031", device,
                         create_info_loc.dot(Field::pInputAssemblyState), "is NULL.");
    }
    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineRenderPass(const vvl::Pipeline &pipeline, const Location &create_info_loc) const {
    bool skip = false;
    // If the vertex input is by itself renderpass is ignored
    if (pipeline.IsRenderPassStateRequired()) {
        if (pipeline.GraphicsCreateInfo().renderPass == VK_NULL_HANDLE) {
            if (!enabled_features.dynamicRendering) {
                skip |=
                    LogError("VUID-VkGraphicsPipelineCreateInfo-dynamicRendering-06576", device,
                             create_info_loc.dot(Field::renderPass), "is NULL, but the dynamicRendering feature was not enabled");
            }
        } else if (!pipeline.RenderPassState()) {
            const auto gpl_info =
                vku::FindStructInPNextChain<VkGraphicsPipelineLibraryCreateInfoEXT>(pipeline.GraphicsCreateInfo().pNext);
            const bool has_flags =
                gpl_info && (gpl_info->flags & (VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
                                                VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
                                                VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT)) != 0;
            const char *vuid =
                has_flags ? "VUID-VkGraphicsPipelineCreateInfo-flags-06643" : "VUID-VkGraphicsPipelineCreateInfo-renderPass-06603";
            skip |= LogError(vuid, device, create_info_loc.dot(Field::renderPass), "is not a valid render pass.");
        }
    }

    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineLibrary(const vvl::Pipeline &pipeline, const Location &create_info_loc) const {
    bool skip = false;

    // It is possible to have no FS state in a complete pipeline whether or not GPL is used
    if (pipeline.OwnsSubState(pipeline.pre_raster_state) && !pipeline.OwnsSubState(pipeline.fragment_shader_state) &&
        ((pipeline.create_info_shaders & FragmentShaderState::ValidShaderStages()) != 0)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pStages-06894", device, create_info_loc,
                         "does not have fragment shader state, but stages (%s) contains VK_SHADER_STAGE_FRAGMENT_BIT.",
                         string_VkShaderStageFlags(pipeline.create_info_shaders).c_str());
    }

    if (!pipeline.fragment_shader_state && !pipeline.pre_raster_state && pipeline.shader_stages_ci.size() > 0) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-stageCount-09587", device, create_info_loc.dot(Field::stageCount),
                         "is %zu, but the pipeline does not have a pre-rasterization or fragment shader state.",
                         pipeline.shader_stages_ci.size());
    }

    const VkPipelineCreateFlags2KHR pipeline_flags = pipeline.create_flags;
    const bool is_create_library = (pipeline_flags & VK_PIPELINE_CREATE_2_LIBRARY_BIT_KHR) != 0;

    if (is_create_library) {
        if (!enabled_features.graphicsPipelineLibrary) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-graphicsPipelineLibrary-06606", device,
                             create_info_loc.dot(Field::flags),
                             "(%s) includes VK_PIPELINE_CREATE_LIBRARY_BIT_KHR, but "
                             "graphicsPipelineLibrary feature is not enabled.",
                             string_VkPipelineCreateFlags2KHR(pipeline_flags).c_str());
        }
    } else {
        // check to make sure all required sub states are here
        // Note: You don't require a vertex input state (ex. if using Mesh Shaders)
        if (!pipeline.pre_raster_state) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-08901", device, create_info_loc,
                             "Attempting to link pipeline libraries without a pre-rasterization shader state (did you forget to "
                             "add VK_PIPELINE_CREATE_LIBRARY_BIT_KHR to your intermediate pipeline?).");
        } else if (pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE) ||
                   !pipeline.RasterizationState()->rasterizerDiscardEnable) {
            if (!pipeline.fragment_shader_state) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-08909", device, create_info_loc,
                                 "Attempting to link pipeline libraries without a fragment shader state (did you forget to add "
                                 "VK_PIPELINE_CREATE_LIBRARY_BIT_KHR to your intermediate pipeline?).");
            } else if (!pipeline.fragment_output_state) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-08909", device, create_info_loc,
                                 "Attempting to link pipeline libraries without a fragment output state (did you forget to add "
                                 "VK_PIPELINE_CREATE_LIBRARY_BIT_KHR to your intermediate pipeline?).");
            }
        }
    }

    if (pipeline.OwnsSubState(pipeline.fragment_shader_state) && !pipeline.OwnsSubState(pipeline.pre_raster_state) &&
        ((pipeline.create_info_shaders & PreRasterState::ValidShaderStages()) != 0)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pStages-06895", device, create_info_loc,
                         "does not have pre-raster state, but stages (%s) contains pre-raster shader stages.",
                         string_VkShaderStageFlags(pipeline.create_info_shaders).c_str());
    }

    // note this is the incoming layout an not ones from the pipeline library
    const auto pipeline_layout_state = Get<vvl::PipelineLayout>(pipeline.GraphicsCreateInfo().layout);

    if (pipeline.HasFullState()) {
        if (is_create_library) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-06608", device, create_info_loc.dot(Field::flags),
                             "(%s) includes VK_PIPELINE_CREATE_LIBRARY_BIT_KHR, but defines a complete set of state.",
                             string_VkPipelineCreateFlags2KHR(pipeline_flags).c_str());
        }

        // A valid pipeline layout must _always_ be provided, even if the pipeline is defined completely from libraries.
        // This a change from the original GPL spec. See https://gitlab.khronos.org/vulkan/vulkan/-/issues/3334 for some
        // context
        // If libraries are included then pipeline layout can be NULL. See
        // https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/6144
        if (!pipeline_layout_state && (!pipeline.library_create_info || pipeline.library_create_info->libraryCount == 0)) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-None-07826", device, create_info_loc.dot(Field::layout),
                             "is not a valid VkPipelineLayout, but defines a complete set of state.");
        }

        // graphics_lib_type effectively tracks which parts of the pipeline are defined by graphics libraries.
        // If the complete state is defined by libraries, we need to check for compatibility with each library's layout
        const bool from_libraries_only = pipeline.graphics_lib_type == AllVkGraphicsPipelineLibraryFlagBitsEXT;
        if (from_libraries_only) {
            const bool pre_raster_independent_set =
                pipeline.fragment_shader_state && (pipeline.fragment_shader_state->PipelineLayoutCreateFlags() &
                                                   VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT) != 0;
            // NOTE: it is possible for an executable pipeline to not contain FS state
            const bool fs_independent_set =
                pipeline.fragment_shader_state && (pipeline.fragment_shader_state->PipelineLayoutCreateFlags() &
                                                   VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT) != 0;
            if (!pre_raster_independent_set && !fs_independent_set) {
                // The layout defined at link time must be compatible with each (pre-raster and fragment shader) sub state's layout
                // (vertex input and fragment output state do not contain a layout)
                if (pipeline_layout_state) {
                    if (std::string err_msg;
                        !VerifySetLayoutCompatibility(*pipeline_layout_state, *pipeline.PreRasterPipelineLayoutState(), err_msg)) {
                        LogObjectList objlist(pipeline_layout_state->Handle(), pipeline.PreRasterPipelineLayoutState()->Handle());
                        skip |= LogError(
                            "VUID-VkGraphicsPipelineCreateInfo-layout-07827", objlist, create_info_loc.dot(Field::layout),
                            "is incompatible with the layout specified in the pre-rasterization library: %s", err_msg.c_str());
                    }
                    if (std::string err_msg; !VerifySetLayoutCompatibility(
                            *pipeline_layout_state, *pipeline.FragmentShaderPipelineLayoutState(), err_msg)) {
                        LogObjectList objlist(pipeline_layout_state->Handle(),
                                              pipeline.FragmentShaderPipelineLayoutState()->Handle());
                        skip |= LogError(
                            "VUID-VkGraphicsPipelineCreateInfo-layout-07827", objlist, create_info_loc.dot(Field::layout),
                            "is incompatible with the layout specified in the fragment shader library: %s", err_msg.c_str());
                    }
                } else {
                    skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-layout-07827", device, create_info_loc.dot(Field::layout),
                                     "is null/invalid and therefore not compatible with the libraries layout");
                }
            }

            const bool has_link_time_opt = (pipeline.create_flags & VK_PIPELINE_CREATE_2_LINK_TIME_OPTIMIZATION_BIT_EXT) != 0;
            if (!has_link_time_opt && (pre_raster_independent_set && fs_independent_set)) {
                if (pipeline_layout_state) {
                    if (std::string err_msg;
                        !VerifySetLayoutCompatibilityUnion(*pipeline_layout_state, *pipeline.PreRasterPipelineLayoutState(),
                                                           *pipeline.FragmentShaderPipelineLayoutState(), err_msg)) {
                        LogObjectList objlist(pipeline_layout_state->Handle(), pipeline.PreRasterPipelineLayoutState()->Handle(),
                                              pipeline.FragmentShaderPipelineLayoutState()->Handle());
                        skip |=
                            LogError("VUID-VkGraphicsPipelineCreateInfo-flags-06730", objlist, create_info_loc.dot(Field::layout),
                                     "is incompatible with the layout specified in the union of (pre-rasterization, fragment "
                                     "shader) libraries: %s",
                                     err_msg.c_str());
                    }
                } else {
                    skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-06730", device, create_info_loc.dot(Field::layout),
                                     "is null/invalid and therefore not compatible with the union of libraries layout");
                }
            }
        }
    }

    // The pipeline libraries can be declared in two different ways
    enum GPLInitType : uint8_t {
        uninitialized = 0,
        gpl_flags,       // VkGraphicsPipelineLibraryCreateInfoEXT::flags
        link_libraries,  // VkPipelineLibraryCreateInfoKHR::pLibraries
    };

    struct GPLValidInfo {
        GPLInitType init = GPLInitType::uninitialized;
        VkPipelineLayoutCreateFlags flags = VK_PIPELINE_LAYOUT_CREATE_FLAG_BITS_MAX_ENUM;
        const vvl::PipelineLayout *layout = nullptr;
        // Can't use MultisampleState() to get this value as we are checking here if they are identical
        const VkPipelineMultisampleStateCreateInfo *ms_state = nullptr;
        const VkPipelineFragmentShadingRateStateCreateInfoKHR *shading_rate_state = nullptr;
    };

    GPLValidInfo pre_raster_info;
    GPLValidInfo frag_shader_info;
    GPLValidInfo frag_output_info;

    const auto &pipeline_ci = pipeline.GraphicsCreateInfo();
    const auto gpl_info = vku::FindStructInPNextChain<VkGraphicsPipelineLibraryCreateInfoEXT>(pipeline_ci.pNext);
    if (gpl_info) {
        if (gpl_info->flags & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT) {
            pre_raster_info.init = GPLInitType::gpl_flags;
            pre_raster_info.flags =
                (pipeline.PreRasterPipelineLayoutState()) ? pipeline.PreRasterPipelineLayoutState()->CreateFlags() : 0;
            pre_raster_info.layout = pipeline.PreRasterPipelineLayoutState().get();
            pre_raster_info.shading_rate_state =
                vku::FindStructInPNextChain<VkPipelineFragmentShadingRateStateCreateInfoKHR>(pipeline_ci.pNext);
        }
        if (gpl_info->flags & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) {
            frag_shader_info.init = GPLInitType::gpl_flags;
            frag_shader_info.flags =
                (pipeline.FragmentShaderPipelineLayoutState()) ? pipeline.FragmentShaderPipelineLayoutState()->CreateFlags() : 0;
            frag_shader_info.layout = pipeline.FragmentShaderPipelineLayoutState().get();
            frag_shader_info.ms_state = pipeline.fragment_shader_state->ms_state.get()->ptr();
            frag_shader_info.shading_rate_state =
                vku::FindStructInPNextChain<VkPipelineFragmentShadingRateStateCreateInfoKHR>(pipeline_ci.pNext);
        }
        if (gpl_info->flags & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT) {
            frag_output_info.init = GPLInitType::gpl_flags;
            frag_output_info.ms_state = pipeline.fragment_output_state->ms_state.get()->ptr();
        }
    }

    if (pipeline.library_create_info) {
        skip |= ValidatePipelineLibraryCreateInfo(pipeline, *pipeline.library_create_info, create_info_loc);

        for (uint32_t i = 0; i < pipeline.library_create_info->libraryCount; ++i) {
            const auto lib = Get<vvl::Pipeline>(pipeline.library_create_info->pLibraries[i]);
            if (!lib) {
                continue;
            }
            const auto &lib_ci = lib->GraphicsCreateInfo();
            if (lib->graphics_lib_type & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT) {
                pre_raster_info.init = GPLInitType::link_libraries;
                const auto layout_state = lib->PreRasterPipelineLayoutState();
                if (layout_state) {
                    pre_raster_info.flags = layout_state->CreateFlags();
                    pre_raster_info.layout = layout_state.get();
                }
                pre_raster_info.shading_rate_state =
                    vku::FindStructInPNextChain<VkPipelineFragmentShadingRateStateCreateInfoKHR>(lib_ci.pNext);
            }
            if (lib->graphics_lib_type & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) {
                frag_shader_info.init = GPLInitType::link_libraries;
                const auto layout_state = lib->FragmentShaderPipelineLayoutState();
                if (layout_state) {
                    frag_shader_info.flags = layout_state->CreateFlags();
                    frag_shader_info.layout = layout_state.get();
                }
                frag_shader_info.ms_state = lib->fragment_shader_state->ms_state.get()->ptr();
                frag_shader_info.shading_rate_state =
                    vku::FindStructInPNextChain<VkPipelineFragmentShadingRateStateCreateInfoKHR>(lib_ci.pNext);
            }
            if (lib->graphics_lib_type & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT) {
                frag_output_info.init = GPLInitType::link_libraries;
                frag_output_info.ms_state = lib->fragment_output_state->ms_state.get()->ptr();
            }
        }
    }

    if ((pipeline.OwnsSubState(pipeline.pre_raster_state) || pipeline.OwnsSubState(pipeline.fragment_shader_state)) &&
        !pipeline_layout_state) {
        const char *vuid = (pre_raster_info.init == GPLInitType::gpl_flags || frag_shader_info.init == GPLInitType::gpl_flags)
                               ? "VUID-VkGraphicsPipelineCreateInfo-flags-06642"
                               : "VUID-VkGraphicsPipelineCreateInfo-layout-06602";
        skip |= LogError(vuid, device, create_info_loc.dot(Field::layout), "is not a valid VkPipelineLayout.");
    }

    if (pipeline_layout_state && pre_raster_info.init == GPLInitType::gpl_flags) {
        for (uint32_t i = 0; i < pipeline_layout_state->set_layouts.size(); i++) {
            if (pipeline_layout_state->set_layouts[i] != nullptr) {
                continue;
            }

            if (!pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE) &&
                pipeline.RasterizationState()->rasterizerDiscardEnable) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-06683", device,
                                 create_info_loc.pNext(Struct::VkGraphicsPipelineLibraryCreateInfoEXT, Field::flags),
                                 "is %s, but layout was created with pSetLayouts[%" PRIu32 "] == VK_NULL_HANDLE",
                                 string_VkGraphicsPipelineLibraryFlagsEXT(gpl_info->flags).c_str(), i);
            } else if (frag_shader_info.init == GPLInitType::gpl_flags) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-06682", device,
                                 create_info_loc.pNext(Struct::VkGraphicsPipelineLibraryCreateInfoEXT, Field::flags),
                                 "is %s, but layout was created with pSetLayouts[%" PRIu32 "] == VK_NULL_HANDLE",
                                 string_VkGraphicsPipelineLibraryFlagsEXT(gpl_info->flags).c_str(), i);
            }
        }
    }

    // pre-raster and fragemnt shader state is defined by some combination of this library and pLibraries
    if ((pre_raster_info.init != GPLInitType::uninitialized) && (frag_shader_info.init != GPLInitType::uninitialized)) {
        // For VUs saying "includes only one... pLibraries includes the other flag" this would be when |only_libs == false|
        // This is because it is not valid to have both init be from |gpl_flags|
        const bool only_libs =
            (pre_raster_info.init == GPLInitType::link_libraries) && (frag_shader_info.init == GPLInitType::link_libraries);

        // Check for consistent independent sets across libraries
        const auto pre_raster_independant_set = (pre_raster_info.flags & VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
        const auto fs_independant_set = (frag_shader_info.flags & VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT);
        const bool not_independent_sets = !pre_raster_independant_set && !fs_independant_set;
        if (pre_raster_independant_set ^ fs_independant_set) {
            const char *vuid =
                only_libs ? "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06615" : "VUID-VkGraphicsPipelineCreateInfo-flags-06614";
            LogObjectList objlist(pre_raster_info.layout->Handle(), frag_shader_info.layout->Handle());
            skip |= LogError(
                vuid, objlist, create_info_loc,
                "is attempting to create a graphics pipeline library with pre-raster and fragment shader state. However "
                "the pre-raster layout create flags (%s) are %s defined with VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT, "
                "and the fragment shader layout create flags (%s) are %s defined with "
                "VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT",
                string_VkPipelineLayoutCreateFlags(pre_raster_info.flags).c_str(), (pre_raster_independant_set != 0) ? "" : "not",
                string_VkPipelineLayoutCreateFlags(frag_shader_info.flags).c_str(), (fs_independant_set != 0) ? "" : "not");
        } else if (not_independent_sets) {
            // "layout used by this pipeline and the library must be identically defined"
            // Inside VkPipelineLayoutCreateInfo, |pSetLayouts| and |pPushConstantRanges| are checked below, this leaves this VU to
            // cover everything else. Currently no valid pNext structs are extending pipeline layout creation
            if (pre_raster_info.layout->create_flags != frag_shader_info.layout->create_flags) {
                const char *vuid = only_libs ? "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06613"
                                             : "VUID-VkGraphicsPipelineCreateInfo-flags-06612";
                LogObjectList objlist(pre_raster_info.layout->Handle(), frag_shader_info.layout->Handle());
                skip |= LogError(vuid, objlist, create_info_loc,
                                 "VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT was not set and the graphics pipeline library "
                                 "have different VkPipelineLayouts, "
                                 "pre-raster layout create flags (%s) and fragment shader layout create flags (%s).",
                                 string_VkPipelineLayoutCreateFlags(pre_raster_info.layout->create_flags).c_str(),
                                 string_VkPipelineLayoutCreateFlags(frag_shader_info.layout->create_flags).c_str());
            }
        }

        // Push Constants must match regardless of VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT
        if (!pre_raster_info.layout->push_constant_ranges->empty() && !frag_shader_info.layout->push_constant_ranges->empty()) {
            const uint32_t pre_raster_count = static_cast<uint32_t>(pre_raster_info.layout->push_constant_ranges->size());
            const uint32_t frag_shader_count = static_cast<uint32_t>(frag_shader_info.layout->push_constant_ranges->size());

            if (pre_raster_count != frag_shader_count) {
                const char *vuid = only_libs ? "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06621"
                                             : "VUID-VkGraphicsPipelineCreateInfo-flags-06620";
                LogObjectList objlist(pre_raster_info.layout->Handle(), frag_shader_info.layout->Handle());
                skip |= LogError(vuid, objlist, create_info_loc,
                                 "the graphics pipeline library have different push constants, pre-raster layout has "
                                 "pushConstantRangeCount of %" PRIu32
                                 ", fragment shader layout has pushConstantRangeCount of %" PRIu32 ".",
                                 pre_raster_count, frag_shader_count);
            } else {
                for (uint32_t i = 0; i < pre_raster_count; i++) {
                    VkPushConstantRange pre_raster_range = pre_raster_info.layout->push_constant_ranges->at(i);
                    VkPushConstantRange frag_shader_range = frag_shader_info.layout->push_constant_ranges->at(i);
                    if (pre_raster_range.stageFlags != frag_shader_range.stageFlags ||
                        pre_raster_range.offset != frag_shader_range.offset || pre_raster_range.size != frag_shader_range.size) {
                        const char *vuid = only_libs ? "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06621"
                                                     : "VUID-VkGraphicsPipelineCreateInfo-flags-06620";
                        LogObjectList objlist(pre_raster_info.layout->Handle(), frag_shader_info.layout->Handle());
                        skip |= LogError(vuid, objlist, create_info_loc,
                                         "the graphics pipeline library have different push constants, pre-raster layout has "
                                         "pPushConstantRanges[%" PRIu32 "] of {%s, %" PRIu32 ", %" PRIu32
                                         "}, fragment shader layout has pPushConstantRanges[%" PRIu32 "] of {%s, %" PRIu32
                                         ", %" PRIu32 "}.",
                                         i, string_VkShaderStageFlags(pre_raster_range.stageFlags).c_str(), pre_raster_range.offset,
                                         pre_raster_range.size, i, string_VkShaderStageFlags(frag_shader_range.stageFlags).c_str(),
                                         frag_shader_range.offset, frag_shader_range.size);
                    }
                }
            }
        }

        // Check VkPipelineFragmentShadingRateStateCreateInfoKHR
        if (frag_shader_info.shading_rate_state || pre_raster_info.shading_rate_state) {
            const char *vuid =
                only_libs ? "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06639" : "VUID-VkGraphicsPipelineCreateInfo-flags-06638";
            if (!pre_raster_info.shading_rate_state) {
                skip |= LogError(
                    vuid, device, create_info_loc,
                    "Fragment Shader has a valid VkPipelineFragmentShadingRateStateCreateInfoKHR, but Pre Rasterization has "
                    "no VkPipelineFragmentShadingRateStateCreateInfoKHR in the pNext.");
            } else if (!frag_shader_info.shading_rate_state) {
                skip |= LogError(
                    vuid, device, create_info_loc,
                    "Pre Rasterization has a valid VkPipelineFragmentShadingRateStateCreateInfoKHR, but Fragment Shader has "
                    "no VkPipelineFragmentShadingRateStateCreateInfoKHR in the pNext.");
            } else if (!ComparePipelineFragmentShadingRateStateCreateInfo(*frag_shader_info.shading_rate_state,
                                                                          *pre_raster_info.shading_rate_state)) {
                skip |= LogError(vuid, device, create_info_loc,
                                 "Fragment Shader and Pre Rasterization were created with different "
                                 "VkPipelineFragmentShadingRateStateCreateInfoKHR.\n"
                                 "Fragment Shader:\n"
                                 "\tfragmentSize: (W = %" PRIu32 ", H = %" PRIu32
                                 ")\n"
                                 "\tcombinerOps[0]: %s\n"
                                 "\tcombinerOps[1]: %s\n"
                                 "Pre Rasterization:\n"
                                 "\tfragmentSize: (W = %" PRIu32 ", H = %" PRIu32
                                 ")\n"
                                 "\tcombinerOps[0]: %s\n"
                                 "\tcombinerOps[1]: %s\n",
                                 frag_shader_info.shading_rate_state->fragmentSize.width,
                                 frag_shader_info.shading_rate_state->fragmentSize.height,
                                 string_VkFragmentShadingRateCombinerOpKHR(frag_shader_info.shading_rate_state->combinerOps[0]),
                                 string_VkFragmentShadingRateCombinerOpKHR(frag_shader_info.shading_rate_state->combinerOps[1]),
                                 pre_raster_info.shading_rate_state->fragmentSize.width,
                                 pre_raster_info.shading_rate_state->fragmentSize.height,
                                 string_VkFragmentShadingRateCombinerOpKHR(pre_raster_info.shading_rate_state->combinerOps[0]),
                                 string_VkFragmentShadingRateCombinerOpKHR(pre_raster_info.shading_rate_state->combinerOps[1]));
            }
        }

        // Check for consistent shader bindings + layout across libraries
        const auto &pre_raster_set_layouts = pre_raster_info.layout->set_layouts;
        const auto &fs_set_layouts = frag_shader_info.layout->set_layouts;
        const uint32_t pre_raster_count = static_cast<uint32_t>(pre_raster_set_layouts.size());
        const uint32_t frag_shader_count = static_cast<uint32_t>(fs_set_layouts.size());
        if (not_independent_sets && pre_raster_count != frag_shader_count) {
            const char *vuid =
                only_libs ? "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06613" : "VUID-VkGraphicsPipelineCreateInfo-flags-06612";
            LogObjectList objlist(pre_raster_info.layout->Handle(), frag_shader_info.layout->Handle());
            skip |= LogError(vuid, objlist, create_info_loc,
                             "VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT was not set and the graphics pipeline library "
                             "have different pipeline layouts, pre-raster layout has "
                             "setLayoutCount of %" PRIu32 ", fragment shader layout has setLayoutCount of %" PRIu32 ".",
                             pre_raster_count, frag_shader_count);
        }

        const auto num_set_layouts = std::max(pre_raster_count, frag_shader_count);
        for (uint32_t i = 0; i < num_set_layouts; ++i) {
            // if using VK_NULL_HANDLE, index into set_layouts will be null
            const auto pre_raster_dsl = i < pre_raster_count ? pre_raster_set_layouts[i] : nullptr;
            const auto fs_dsl = i < frag_shader_count ? fs_set_layouts[i] : nullptr;

            if (!pre_raster_dsl && fs_dsl) {
                // Null DSL at pSetLayouts[i] in pre-raster state. Make sure that shader bindings in corresponding DSL in
                // fragment shader state do not overlap.
                for (const auto &fs_binding : fs_dsl->GetBindings()) {
                    if ((fs_binding.stageFlags & PreRasterState::ValidShaderStages()) == 0) {
                        continue;
                    }
                    const auto pre_raster_layout_handle_str = FormatHandle(pre_raster_info.layout->Handle());
                    const auto fs_layout_handle_str = FormatHandle(frag_shader_info.layout->Handle());
                    const char *vuid = nullptr;
                    std::ostringstream msg;
                    if (pre_raster_info.init == GPLInitType::gpl_flags) {
                        vuid = "VUID-VkGraphicsPipelineCreateInfo-flags-06756";
                        msg << "represents a library containing pre-raster state, and descriptor set layout (from "
                               "layout "
                            << pre_raster_layout_handle_str << ") at pSetLayouts[" << i << "] is NULL. "
                            << "However, a library with fragment shader state is specified in "
                               "VkPipelineLibraryCreateInfoKHR::pLibraries with non-null descriptor set layout at the "
                               "same pSetLayouts index ("
                            << i << ") from layout " << fs_layout_handle_str << " and bindings ("
                            << string_VkShaderStageFlags(fs_binding.stageFlags) << ") that overlap with pre-raster state.";
                    } else if (frag_shader_info.init == GPLInitType::gpl_flags) {
                        vuid = "VUID-VkGraphicsPipelineCreateInfo-flags-06757";
                        msg << "represents a library containing fragment shader state, and descriptor set layout (from "
                               "layout "
                            << fs_layout_handle_str << ") at pSetLayouts[" << i << "] contains bindings ("
                            << string_VkShaderStageFlags(fs_binding.stageFlags) << ") that overlap with pre-raster state. "
                            << "However, a library with pre-raster state is specified in "
                               "VkPipelineLibraryCreateInfoKHR::pLibraries with a null descriptor set layout at the "
                               "same pSetLayouts index ("
                            << i << ") from layout " << pre_raster_layout_handle_str << ".";
                    } else {
                        vuid = "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06758";
                        msg << "is linking libraries with pre-raster and fragment shader state. The descriptor set "
                               "layout at index "
                            << i << " in pSetLayouts from " << pre_raster_layout_handle_str << " in the pre-raster state is NULL. "
                            << "However, the descriptor set layout at the same index (" << i << ") in " << fs_layout_handle_str
                            << " is non-null with bindings (" << string_VkShaderStageFlags(fs_binding.stageFlags)
                            << ") that overlap with pre-raster state.";
                    }
                    LogObjectList objlist(pre_raster_info.layout->Handle(), frag_shader_info.layout->Handle());
                    skip |= LogError(vuid, objlist, create_info_loc, "%s", msg.str().c_str());
                    break;
                }
            } else if (pre_raster_dsl && !fs_dsl) {
                // Null DSL at pSetLayouts[i] in FS state. Make sure that shader bindings in corresponding DSL in pre-raster
                // state do not overlap.
                for (const auto &pre_raster_binding : pre_raster_dsl->GetBindings()) {
                    if ((pre_raster_binding.stageFlags & FragmentShaderState::ValidShaderStages()) == 0) {
                        continue;
                    }
                    const auto pre_raster_layout_handle_str = FormatHandle(pre_raster_info.layout->Handle());
                    const auto fs_layout_handle_str = FormatHandle(frag_shader_info.layout->Handle());
                    const char *vuid = nullptr;
                    std::ostringstream msg;
                    if (frag_shader_info.init == GPLInitType::gpl_flags) {
                        vuid = "VUID-VkGraphicsPipelineCreateInfo-flags-06756";
                        msg << "represents a library containing fragment shader state, and descriptor set layout (from "
                               "layout "
                            << fs_layout_handle_str << ") at pSetLayouts[" << i << "] is null. "
                            << "However, a library with pre-raster state is specified in "
                               "VkPipelineLibraryCreateInfoKHR::pLibraries with non-null descriptor set layout at the "
                               "same pSetLayouts index ("
                            << i << ") from layout " << pre_raster_layout_handle_str << " and bindings ("
                            << string_VkShaderStageFlags(pre_raster_binding.stageFlags)
                            << ") that overlap with fragment shader state.";
                    } else if (pre_raster_info.init == GPLInitType::gpl_flags) {
                        vuid = "VUID-VkGraphicsPipelineCreateInfo-flags-06757";
                        msg << "represents a library containing pre-raster state, and descriptor set layout (from "
                               "layout "
                            << pre_raster_layout_handle_str << ") at pSetLayouts[" << i << "] contains bindings ("
                            << string_VkShaderStageFlags(pre_raster_binding.stageFlags)
                            << ") that overlap with fragment shader state. "
                            << "However, a library with fragment shader state is specified in "
                               "VkPipelineLibraryCreateInfoKHR::pLibraries with a null descriptor set layout at the "
                               "same pSetLayouts index ("
                            << i << ") from layout " << fs_layout_handle_str << ".";
                    } else {
                        vuid = "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06758";
                        msg << "is linking libraries with pre-raster and fragment shader state. The descriptor set "
                               "layout at index "
                            << i << " in pSetLayouts from " << fs_layout_handle_str << " in the fragment shader state is NULL. "
                            << "However, the descriptor set layout at the same index (" << i << ") in "
                            << pre_raster_layout_handle_str << " in the pre-raster state is non-null with bindings ("
                            << string_VkShaderStageFlags(pre_raster_binding.stageFlags)
                            << ") that overlap with fragment shader "
                               "state.";
                    }
                    LogObjectList objlist(pre_raster_info.layout->Handle(), frag_shader_info.layout->Handle());
                    skip |= LogError(vuid, objlist, create_info_loc, "%s", msg.str().c_str());
                    break;
                }
            } else if (!pre_raster_dsl && !fs_dsl) {
                const auto pre_raster_layout_handle_str = FormatHandle(pre_raster_info.layout->Handle());
                const auto fs_layout_handle_str = FormatHandle(frag_shader_info.layout->Handle());
                const char *vuid = nullptr;
                std::ostringstream msg;
                if (frag_shader_info.init == GPLInitType::gpl_flags) {
                    vuid = "VUID-VkGraphicsPipelineCreateInfo-flags-06679";
                    msg << "represents a library containing fragment shader state, and descriptor set layout (from "
                           "layout "
                        << fs_layout_handle_str << ") at pSetLayouts[" << i << "] is NULL. "
                        << "However, a library with pre-raster state is specified in "
                           "VkPipelineLibraryCreateInfoKHR::pLibraries ("
                        << pre_raster_layout_handle_str << ") with pSetLayouts[" << i << "] NULL too.";
                } else if (pre_raster_info.init == GPLInitType::gpl_flags) {
                    vuid = "VUID-VkGraphicsPipelineCreateInfo-flags-06679";
                    msg << "represents a library containing pre-raster state, and descriptor set layout (from "
                           "layout "
                        << pre_raster_layout_handle_str << ") at pSetLayouts[" << i << "] is NULL. "
                        << "However, a library with fragment shader state is specified in "
                           "VkPipelineLibraryCreateInfoKHR::pLibraries ("
                        << fs_layout_handle_str << ") with pSetLayouts[" << i << "] NULL too.";
                } else {
                    vuid = "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06681";
                    msg << "is linking libraries with pre-raster and fragment shader state. The descriptor set "
                           "layout at index "
                        << i << " in pSetLayouts from " << fs_layout_handle_str << " in the fragment shader state is NULL. "
                        << "However, the descriptor set layout at the same index (" << i << ") in " << pre_raster_layout_handle_str
                        << " in the pre-raster state is NULL too.";
                }
                LogObjectList objlist(pre_raster_info.layout->Handle(), frag_shader_info.layout->Handle());
                skip |= LogError(vuid, objlist, create_info_loc, "%s", msg.str().c_str());
                break;
            } else if (not_independent_sets) {
                // both handles are valid, but without VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT, need to check everything
                // is identically defined
                if (pre_raster_dsl->GetCreateFlags() != fs_dsl->GetCreateFlags()) {
                    const char *vuid = only_libs ? "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06613"
                                                 : "VUID-VkGraphicsPipelineCreateInfo-flags-06612";
                    LogObjectList objlist(pre_raster_info.layout->Handle(), frag_shader_info.layout->Handle());
                    skip |= LogError(vuid, objlist, create_info_loc,
                                     "VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT was not set and the graphics pipeline "
                                     "library have different pipeline layouts, pre-raster layout has "
                                     "pSetLayouts[%" PRIu32
                                     "] flags (%s)"
                                     ", fragment shader layout has pSetLayouts[%" PRIu32 "] flags (%s).",
                                     i, string_VkDescriptorSetLayoutCreateFlags(pre_raster_dsl->GetCreateFlags()).c_str(), i,
                                     string_VkDescriptorSetLayoutCreateFlags(fs_dsl->GetCreateFlags()).c_str());
                } else if (pre_raster_dsl->GetBindingCount() != fs_dsl->GetBindingCount()) {
                    const char *vuid = only_libs ? "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06613"
                                                 : "VUID-VkGraphicsPipelineCreateInfo-flags-06612";
                    LogObjectList objlist(pre_raster_info.layout->Handle(), frag_shader_info.layout->Handle());
                    skip |= LogError(vuid, objlist, create_info_loc,
                                     "VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT was not set and the graphics pipeline "
                                     "library have different pipeline layouts, pre-raster layout has "
                                     "pSetLayouts[%" PRIu32 "] bindingCount (%" PRIu32
                                     ")"
                                     ", fragment shader layout has pSetLayouts[%" PRIu32 "] bindingCount (%" PRIu32 ").",
                                     i, pre_raster_dsl->GetBindingCount(), i, fs_dsl->GetBindingCount());
                } else {
                    const uint32_t binding_count = pre_raster_dsl->GetBindingCount();
                    const auto &pre_raster_bindings = pre_raster_dsl->GetBindings();
                    const auto &fs_bindings = fs_dsl->GetBindings();
                    for (uint32_t binding_index = 0; binding_index < binding_count; binding_index++) {
                        const auto &pre_raster_binding = pre_raster_bindings[binding_index];
                        const auto &fs_binding = fs_bindings[binding_index];
                        if (!CompareDescriptorSetLayoutBinding(*pre_raster_binding.ptr(), *fs_binding.ptr())) {
                            const char *vuid = only_libs ? "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06613"
                                                         : "VUID-VkGraphicsPipelineCreateInfo-flags-06612";
                            LogObjectList objlist(pre_raster_info.layout->Handle(), frag_shader_info.layout->Handle());
                            skip |= LogError(
                                vuid, objlist, create_info_loc,
                                "VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT was not set and the graphics pipeline "
                                "library have different pipeline layouts, pre-raster layout has "
                                "pSetLayouts[%" PRIu32 "] pBindings[%" PRIu32
                                "] of:\n"
                                "\tbinding: %" PRIu32
                                "\n"
                                "\tdescriptorType: %s\n"
                                "\tdescriptorCount: %" PRIu32
                                "\n"
                                "\tstageFlags: %s\n"
                                "\tpImmutableSamplers: 0x%p\n"
                                "fragment shader layout has pSetLayouts[%" PRIu32 "] pBindings[%" PRIu32
                                "] of:\n"
                                "\tbinding: %" PRIu32
                                "\n"
                                "\tdescriptorType: %s\n"
                                "\tdescriptorCount: %" PRIu32
                                "\n"
                                "\tstageFlags: %s\n"
                                "\tpImmutableSamplers: 0x%p\n",
                                i, binding_index, pre_raster_binding.binding,
                                string_VkDescriptorType(pre_raster_binding.descriptorType), pre_raster_binding.descriptorCount,
                                string_VkShaderStageFlags(pre_raster_binding.stageFlags).c_str(),
                                pre_raster_binding.pImmutableSamplers, i, binding_index, fs_binding.binding,
                                string_VkDescriptorType(fs_binding.descriptorType), fs_binding.descriptorCount,
                                string_VkShaderStageFlags(fs_binding.stageFlags).c_str(), fs_binding.pImmutableSamplers);
                        }
                    }
                }
            }
        }
    }

    if ((frag_shader_info.init != GPLInitType::uninitialized) && (frag_output_info.init != GPLInitType::uninitialized)) {
        if (!frag_shader_info.ms_state && frag_output_info.ms_state) {
            // if Fragment Output has sampleShadingEnable == false, Fragement Shader may be null
            if (frag_output_info.ms_state->sampleShadingEnable) {
                const char *vuid =
                    (frag_shader_info.init == GPLInitType::gpl_flags)   ? "VUID-VkGraphicsPipelineCreateInfo-pLibraries-09567"
                    : (frag_output_info.init == GPLInitType::gpl_flags) ? "VUID-VkGraphicsPipelineCreateInfo-flags-06637"
                                                                        : "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06636";
                skip |= LogError(
                    vuid, device, create_info_loc,
                    "Fragment Output Interface were created with VkPipelineMultisampleStateCreateInfo::sampleShadingEnable to "
                    "VK_TRUE, but Fragment Shader has a pMultisampleState of NULL.");
            }
        } else if (frag_shader_info.ms_state && !frag_output_info.ms_state) {
            const char *vuid = (frag_shader_info.init == GPLInitType::gpl_flags) ? "VUID-VkGraphicsPipelineCreateInfo-flags-06633"
                               : (frag_output_info.init == GPLInitType::gpl_flags)
                                   ? "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06634"
                                   : "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06635";
            skip |= LogError(vuid, device, create_info_loc,
                             "Fragment Shader has a valid VkPipelineMultisampleStateCreateInfo, but Fragment Output Interface has "
                             "a pMultisampleState of NULL.");
        } else if (frag_shader_info.ms_state && frag_output_info.ms_state) {
            if (!ComparePipelineMultisampleStateCreateInfo(*frag_shader_info.ms_state, *frag_output_info.ms_state)) {
                const char *vuid =
                    (frag_shader_info.init == GPLInitType::gpl_flags)   ? "VUID-VkGraphicsPipelineCreateInfo-flags-06633"
                    : (frag_output_info.init == GPLInitType::gpl_flags) ? "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06634"
                                                                        : "VUID-VkGraphicsPipelineCreateInfo-pLibraries-06635";
                skip |= LogError(vuid, device, create_info_loc,
                                 "Fragment Shader and Fragment Output Interface were created with different "
                                 "VkPipelineMultisampleStateCreateInfo."
                                 "Fragment Shader pMultisampleState:\n"
                                 "\tpNext: %p\n"
                                 "\trasterizationSamples: %s\n"
                                 "\tsampleShadingEnable: %d\n"
                                 "\tminSampleShading: %f\n"
                                 "\tpSampleMask: %p\n"
                                 "\talphaToCoverageEnable: %d\n"
                                 "\talphaToOneEnable: %d\n"
                                 "Fragment Output Interface pMultisampleState:\n"
                                 "\tpNext: %p\n"
                                 "\trasterizationSamples: %s\n"
                                 "\tsampleShadingEnable: %d\n"
                                 "\tminSampleShading: %f\n"
                                 "\tpSampleMask: %p\n"
                                 "\talphaToCoverageEnable: %d\n"
                                 "\talphaToOneEnable: %d\n",
                                 frag_shader_info.ms_state->pNext,
                                 string_VkSampleCountFlagBits(frag_shader_info.ms_state->rasterizationSamples),
                                 frag_shader_info.ms_state->sampleShadingEnable, frag_shader_info.ms_state->minSampleShading,
                                 frag_shader_info.ms_state->pSampleMask, frag_shader_info.ms_state->alphaToCoverageEnable,
                                 frag_shader_info.ms_state->alphaToOneEnable, frag_output_info.ms_state->pNext,
                                 string_VkSampleCountFlagBits(frag_output_info.ms_state->rasterizationSamples),
                                 frag_output_info.ms_state->sampleShadingEnable, frag_output_info.ms_state->minSampleShading,
                                 frag_output_info.ms_state->pSampleMask, frag_output_info.ms_state->alphaToCoverageEnable,
                                 frag_output_info.ms_state->alphaToOneEnable);
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineBlendEnable(const vvl::Pipeline &pipeline, const Location &create_info_loc) const {
    bool skip = false;
    const auto &rp_state = pipeline.RenderPassState();
    if (!rp_state) {
        return false;
    }
    const Location color_loc = create_info_loc.dot(Field::pColorBlendState);

    if (!rp_state->UsesDynamicRendering()) {
        const auto subpass = pipeline.Subpass();
        const auto *subpass_desc = &rp_state->create_info.pSubpasses[subpass];

        for (uint32_t i = 0; i < pipeline.AttachmentStates().size() && i < subpass_desc->colorAttachmentCount; ++i) {
            const auto attachment = subpass_desc->pColorAttachments[i].attachment;
            if (attachment == VK_ATTACHMENT_UNUSED) continue;

            const auto attachment_desc = rp_state->create_info.pAttachments[attachment];
            VkFormatFeatureFlags2KHR format_features = GetPotentialFormatFeatures(attachment_desc.format);

            const auto *raster_state = pipeline.RasterizationState();
            if (raster_state && !raster_state->rasterizerDiscardEnable && pipeline.AttachmentStates()[i].blendEnable &&
                !(format_features & VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BLEND_BIT_KHR)) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06041", device,
                                 color_loc.dot(Field::pAttachments, i).dot(Field::blendEnable),
                                 "is VK_TRUE but format %s of the corresponding attachment description (subpass %" PRIu32
                                 ", attachment %" PRIu32 ") does not support VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT.",
                                 string_VkFormat(attachment_desc.format), subpass, attachment);
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineMeshTask(const vvl::Pipeline &pipeline, const Location &create_info_loc) const {
    bool skip = false;
    const bool has_mesh = (pipeline.active_shaders & VK_SHADER_STAGE_MESH_BIT_EXT) != 0;
    const bool has_task = (pipeline.active_shaders & VK_SHADER_STAGE_TASK_BIT_EXT) != 0;
    if (has_mesh && has_task) {
        for (const auto &stage : pipeline.stage_states) {
            if (stage.GetStage() == VK_SHADER_STAGE_MESH_BIT_EXT && stage.spirv_state->static_data_.has_builtin_draw_index) {
                // VUID being made in https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/6560
                skip |= LogError("UNASSIGNED-VkGraphicsPipelineCreateInfo-Mesh-DrawIndex", device, create_info_loc,
                                 "The pipeline is being created with a Task and Mesh shader bound, but the Mesh Shader "
                                 "uses DrawIndex (gl_DrawID) which will be an undefined value when reading.");
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineExternalFormatResolve(const vvl::Pipeline &pipeline,
                                                               const vku::safe_VkSubpassDescription2 *subpass_desc,
                                                               const Location &create_info_loc) const {
    bool skip = false;
    if (!enabled_features.externalFormatResolve) {
        return false;
    }

    const auto &rp_state = pipeline.RenderPassState();
    const auto *multisample_state = pipeline.MultisampleState();
    const auto *color_blend_state = pipeline.ColorBlendState();
    const auto &pipeline_ci = pipeline.GraphicsCreateInfo();
    const auto fragment_shading_rate =
        vku::FindStructInPNextChain<VkPipelineFragmentShadingRateStateCreateInfoKHR>(pipeline_ci.pNext);

    if (rp_state && !rp_state->UsesDynamicRendering()) {
        if (!subpass_desc || subpass_desc->colorAttachmentCount == 0 || !subpass_desc->pResolveAttachments) {
            return false;
        }
        // can only have 1 color attachment
        const uint32_t attachment = subpass_desc->pResolveAttachments[0].attachment;
        if (attachment == VK_ATTACHMENT_UNUSED) {
            return false;
        }
        const uint64_t external_format = GetExternalFormat(rp_state->create_info.pAttachments[attachment].pNext);
        if (external_format == 0) {
            return false;
        }

        if (!pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT) && multisample_state) {
            if (multisample_state->rasterizationSamples != VK_SAMPLE_COUNT_1_BIT) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-externalFormatResolve-09313", device,
                                 create_info_loc.dot(Field::pMultisampleState).dot(Field::rasterizationSamples),
                                 "is %" PRIu32 ", but externalFormat is %" PRIu64 " for subpass %" PRIu32 ".",
                                 multisample_state->rasterizationSamples, external_format, pipeline.Subpass());
            }
        }

        if (!pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT) && color_blend_state) {
            for (uint32_t i = 0; i < color_blend_state->attachmentCount; i++) {
                if (color_blend_state->pAttachments[i].blendEnable) {
                    skip |=
                        LogError("VUID-VkGraphicsPipelineCreateInfo-externalFormatResolve-09314", device,
                                 create_info_loc.dot(Field::pColorBlendState).dot(Field::pAttachments, i).dot(Field::blendEnable),
                                 "is VK_TRUE, but externalFormat is %" PRIu64 " for subpass %" PRIu32 ".", external_format,
                                 pipeline.Subpass());
                }
            }
        }

        if (!pipeline.IsDynamic(VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR) && fragment_shading_rate) {
            if (fragment_shading_rate->fragmentSize.width != 1) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-externalFormatResolve-09315", device,
                                 create_info_loc.pNext(Struct::VkPipelineFragmentShadingRateStateCreateInfoKHR, Field::fragmentSize)
                                     .dot(Field::width),
                                 "is %" PRIu32 ", but externalFormat is %" PRIu64 " for subpass %" PRIu32 ".",
                                 fragment_shading_rate->fragmentSize.width, external_format, pipeline.Subpass());
            }
            if (fragment_shading_rate->fragmentSize.height != 1) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-externalFormatResolve-09316", device,
                                 create_info_loc.pNext(Struct::VkPipelineFragmentShadingRateStateCreateInfoKHR, Field::fragmentSize)
                                     .dot(Field::height),
                                 "is %" PRIu32 ", but externalFormat is %" PRIu64 " for subpass %" PRIu32 ".",
                                 fragment_shading_rate->fragmentSize.height, external_format, pipeline.Subpass());
            }
        }
    } else {
        const uint64_t external_format = GetExternalFormat(pipeline.GraphicsCreateInfo().pNext);
        const auto *rendering_struct = pipeline.rendering_create_info;
        if (external_format == 0 || !rendering_struct) {
            return false;
        }

        if (rendering_struct->viewMask != 0) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-externalFormatResolve-09301", device,
                             create_info_loc.pNext(Struct::VkPipelineRenderingCreateInfo, Field::viewMask),
                             "is 0x%" PRIx32 ", but externalFormat is %" PRIu64 ".", rendering_struct->viewMask, external_format);
        }
        if (rendering_struct->colorAttachmentCount != 1) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-externalFormatResolve-09309", device,
                             create_info_loc.pNext(Struct::VkPipelineRenderingCreateInfo, Field::colorAttachmentCount),
                             "is %" PRIu32 ", but externalFormat is %" PRIu64 ".", rendering_struct->colorAttachmentCount,
                             external_format);
        }

        if (pipeline.OwnsSubState(pipeline.fragment_shader_state) && pipeline.fragment_shader_state->fragment_entry_point) {
            auto entrypoint = pipeline.fragment_shader_state->fragment_entry_point;
            if (entrypoint->execution_mode.Has(spirv::ExecutionModeSet::depth_replacing_bit)) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-externalFormatResolve-09310", device,
                                 create_info_loc.pNext(Struct::VkExternalFormatANDROID, Field::externalFormat),
                                 "is %" PRIu64 " but the fragment shader declares DepthReplacing.", external_format);
            } else if (entrypoint->execution_mode.Has(spirv::ExecutionModeSet::stencil_ref_replacing_bit)) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-externalFormatResolve-09310", device,
                                 create_info_loc.pNext(Struct::VkExternalFormatANDROID, Field::externalFormat),
                                 "is %" PRIu64 " but the fragment shader declares StencilRefReplacingEXT.", external_format);
            }
        }

        if (!pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT) && multisample_state) {
            if (multisample_state->rasterizationSamples != VK_SAMPLE_COUNT_1_BIT) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-externalFormatResolve-09304", device,
                                 create_info_loc.dot(Field::pMultisampleState).dot(Field::rasterizationSamples),
                                 "is %s, but externalFormat is %" PRIu64 ".",
                                 string_VkSampleCountFlagBits(multisample_state->rasterizationSamples), external_format);
            }
        }

        if (!pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT) && color_blend_state) {
            for (uint32_t i = 0; i < color_blend_state->attachmentCount; i++) {
                if (color_blend_state->pAttachments[i].blendEnable) {
                    skip |=
                        LogError("VUID-VkGraphicsPipelineCreateInfo-externalFormatResolve-09305", device,
                                 create_info_loc.dot(Field::pColorBlendState).dot(Field::pAttachments, i).dot(Field::blendEnable),
                                 "is VK_TRUE, but externalFormat is %" PRIu64 ".", external_format);
                }
            }
        }

        if (!pipeline.IsDynamic(VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR) && fragment_shading_rate) {
            if (fragment_shading_rate->fragmentSize.width != 1) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-externalFormatResolve-09306", device,
                                 create_info_loc.pNext(Struct::VkPipelineFragmentShadingRateStateCreateInfoKHR, Field::fragmentSize)
                                     .dot(Field::width),
                                 "is %" PRIu32 ", but externalFormat is %" PRIu64 ".", fragment_shading_rate->fragmentSize.width,
                                 external_format);
            }
            if (fragment_shading_rate->fragmentSize.height != 1) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-externalFormatResolve-09307", device,
                                 create_info_loc.pNext(Struct::VkPipelineFragmentShadingRateStateCreateInfoKHR, Field::fragmentSize)
                                     .dot(Field::height),
                                 "is %" PRIu32 ", but externalFormat is %" PRIu64 ".", fragment_shading_rate->fragmentSize.height,
                                 external_format);
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineInputAssemblyState(const vvl::Pipeline &pipeline, const Location &create_info_loc) const {
    bool skip = false;
    const Location ia_loc = create_info_loc.dot(Field::pInputAssemblyState);

    // if vertex_input_state is not set, will be null
    const auto *ia_state = pipeline.InputAssemblyState();
    if (ia_state) {
        const VkPrimitiveTopology topology = ia_state->topology;
        if ((ia_state->primitiveRestartEnable == VK_TRUE) &&
            IsValueIn(topology, {VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
                                 VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
                                 VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_PATCH_LIST})) {
            if (topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST) {
                if (!enabled_features.primitiveTopologyPatchListRestart) {
                    skip |= LogError("VUID-VkPipelineInputAssemblyStateCreateInfo-topology-06253", device, ia_loc,
                                     "topology is %s and primitiveRestartEnable is VK_TRUE and the "
                                     "primitiveTopologyPatchListRestart feature was not enabled.",
                                     string_VkPrimitiveTopology(topology));
                }
            } else if (!enabled_features.primitiveTopologyListRestart) {
                skip |=
                    LogError("VUID-VkPipelineInputAssemblyStateCreateInfo-topology-06252", device, ia_loc,
                             "topology is %s and primitiveRestartEnable is VK_TRUE and the primitiveTopologyListRestart feature "
                             "was not enabled.",
                             string_VkPrimitiveTopology(topology));
            }
        }
        if ((enabled_features.geometryShader == VK_FALSE) &&
            IsValueIn(topology,
                      {VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,
                       VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY})) {
            skip |= LogError( "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00429", device, ia_loc,
                             "topology is %s and geometryShader feature was not enabled.",
                             string_VkPrimitiveTopology(topology));
        }
        if ((enabled_features.tessellationShader == VK_FALSE) && (topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)) {
            skip |= LogError( "VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00430", device, ia_loc,
                             "topology is %s and tessellationShader feature was not enabled.",
                             string_VkPrimitiveTopology(topology));
        }

        if (!phys_dev_ext_props.conservative_rasterization_props.conservativePointAndLineRasterization &&
            pipeline.vertex_input_state && pipeline.pre_raster_state &&
            (pipeline.create_info_shaders & VK_SHADER_STAGE_GEOMETRY_BIT) == 0 &&
            IsValueIn(topology,
                      {VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP})) {
            const auto rasterization_conservative_state_ci =
                vku::FindStructInPNextChain<VkPipelineRasterizationConservativeStateCreateInfoEXT>(
                    pipeline.RasterizationState()->pNext);
            if (rasterization_conservative_state_ci &&
                rasterization_conservative_state_ci->conservativeRasterizationMode !=
                    VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT &&
                (!pipeline.IsDynamic(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY) ||
                 !phys_dev_ext_props.extended_dynamic_state3_props.dynamicPrimitiveTopologyUnrestricted)) {
                std::string msg = !pipeline.IsDynamic(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY)
                                      ? "VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY is not enabled"
                                      : "dynamicPrimitiveTopologyUnrestricted is not supported";
                skip |= LogError(
                    "VUID-VkGraphicsPipelineCreateInfo-conservativePointAndLineRasterization-08892", device, ia_loc,
                    "topology is %s, %s, but conservativeRasterizationMode is %s.", string_VkPrimitiveTopology(topology),
                    msg.c_str(),
                    string_VkConservativeRasterizationModeEXT(rasterization_conservative_state_ci->conservativeRasterizationMode));
            }
        }
    }

    const bool ignore_topology = pipeline.IsDynamic(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY) &&
                                 phys_dev_ext_props.extended_dynamic_state3_props.dynamicPrimitiveTopologyUnrestricted;
    // pre_raster has tessellation stage, vertex input has topology
    // Both are needed for these checks
    if (!ignore_topology && pipeline.pre_raster_state && pipeline.vertex_input_state) {
        // Either both or neither TC/TE shaders should be defined
        const bool has_control = (pipeline.active_shaders & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) != 0;
        const bool has_eval = (pipeline.active_shaders & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) != 0;
        const bool has_tessellation = has_control && has_eval;  // need both
        // VK_PRIMITIVE_TOPOLOGY_PATCH_LIST primitive topology is only valid for tessellation pipelines.
        // Mismatching primitive topology and tessellation fails graphics pipeline creation.
        if (has_tessellation && (!ia_state || ia_state->topology != VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pStages-08888", device, ia_loc.dot(Field::topology),
                             "is %s for tessellation shaders in pipeline (needs to be VK_PRIMITIVE_TOPOLOGY_PATCH_LIST).",
                             ia_state ? string_VkPrimitiveTopology(ia_state->topology) : "null");
        }
        if (!has_tessellation && (ia_state && ia_state->topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-topology-08889", device,  ia_loc.dot(Field::topology),
                             "is VK_PRIMITIVE_TOPOLOGY_PATCH_LIST but no tessellation shaders.");
        }
    };
    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineTessellationState(const vvl::Pipeline &pipeline, const Location &create_info_loc) const {
    bool skip = false;

    if (pipeline.OwnsSubState(pipeline.pre_raster_state) &&
        (pipeline.create_info_shaders & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT)) {
        if (!pipeline.TessellationState() && (!pipeline.IsDynamic(VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT) ||
                                              !IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state3))) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pStages-09022", device, create_info_loc.dot(Field::pStages),
                             "includes a tessellation control shader stage, but pTessellationState is NULL.");
        }
    }
    return skip;
}

bool CoreChecks::ValidateGraphicsPipelinePreRasterizationState(const vvl::Pipeline &pipeline,
                                                               const Location &create_info_loc) const {
    bool skip = false;
    // Only validate once during creation
    if (!pipeline.OwnsSubState(pipeline.pre_raster_state)) {
        return skip;
    }
    const VkShaderStageFlags stages = pipeline.create_info_shaders;
    if ((stages & PreRasterState::ValidShaderStages()) == 0) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pStages-06896", device, create_info_loc,
                         "contains pre-raster state, but stages (%s) does not contain any pre-raster shaders.",
                         string_VkShaderStageFlags(stages).c_str());
    }

    if (!enabled_features.geometryShader && (stages & VK_SHADER_STAGE_GEOMETRY_BIT)) {
        skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-stage-00704", device, create_info_loc,
                         "pStages include Geometry Shader but geometryShader feature was not enabled.");
    }
    if (!enabled_features.tessellationShader &&
        (stages & (VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT))) {
        skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-stage-00705", device, create_info_loc,
                         "pStages include Tessellation Shader but tessellationShader feature was not enabled.");
    }

    // VS or mesh is required
    if (!(stages & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_MESH_BIT_EXT))) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-stage-02096", device, create_info_loc,
                         "no stage in pStages contains a Vertex Shader or Mesh Shader.");
    }
    // Can't mix mesh and VTG
    if ((stages & (VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT)) &&
        (stages & (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                   VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT))) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pStages-02095", device, create_info_loc,
                         "in pStages, Geometric shader stages must either be all mesh (mesh | task) "
                         "or all VTG (vertex, tess control, tess eval, geom).");
    }

    // VK_SHADER_STAGE_MESH_BIT_EXT and VK_SHADER_STAGE_MESH_BIT_NV are equivalent
    if (!(enabled_features.meshShader) && (stages & VK_SHADER_STAGE_MESH_BIT_EXT)) {
        skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-stage-02091", device, create_info_loc,
                         "pStages include Mesh Shader but meshShader feature was not enabled.");
    }

    // VK_SHADER_STAGE_TASK_BIT_EXT and VK_SHADER_STAGE_TASK_BIT_NV are equivalent
    if (!(enabled_features.taskShader) && (stages & VK_SHADER_STAGE_TASK_BIT_EXT)) {
        skip |= LogError("VUID-VkPipelineShaderStageCreateInfo-stage-02092", device, create_info_loc,
                         "pStages include Task Shader but taskShader feature was not enabled.");
    }

    // Either both or neither TC/TE shaders should be defined
    const bool has_control = (stages & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) != 0;
    const bool has_eval = (stages & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) != 0;
    if (has_control && !has_eval) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pStages-00729", device, create_info_loc,
                         "pStages include a VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT but no "
                         "VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT stage.");
    }
    if (!has_control && has_eval) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pStages-00730", device, create_info_loc,
                         "pStages include a VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT but no "
                         "VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT stage.");
    }
    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineColorBlendAttachmentState(const vvl::Pipeline &pipeline,
                                                                   const vku::safe_VkSubpassDescription2 *subpass_desc,
                                                                   const Location &color_loc) const {
    bool skip = false;
    const auto &attachment_states = pipeline.AttachmentStates();
    if (attachment_states.empty()) {
        return skip;
    }
    if (!enabled_features.independentBlend) {
        if (attachment_states.size() > 1) {
            for (size_t i = 1; i < attachment_states.size(); i++) {
                if (!ComparePipelineColorBlendAttachmentState(attachment_states[0], attachment_states[i])) {
                    skip |= LogError("VUID-VkPipelineColorBlendStateCreateInfo-pAttachments-00605", device,
                                     color_loc.dot(Field::pAttachments, (uint32_t)i),
                                     "is different than pAttachments[0] and independentBlend feature was not enabled.");
                    break;
                }
            }
        }
    }

    const VkBlendOp first_color_blend_op = attachment_states[0].colorBlendOp;
    const VkBlendOp first_alpha_blend_op = attachment_states[0].alphaBlendOp;

    for (size_t i = 0; i < attachment_states.size(); i++) {
        const VkPipelineColorBlendAttachmentState &attachment_state = attachment_states[i];
        const Location &attachment_loc = color_loc.dot(Field::pAttachments, (uint32_t)i);
        if (IsSecondaryColorInputBlendFactor(attachment_state.srcColorBlendFactor)) {
            if (!enabled_features.dualSrcBlend) {
                skip |= LogError("VUID-VkPipelineColorBlendAttachmentState-srcColorBlendFactor-00608", device,
                                 attachment_loc.dot(Field::srcColorBlendFactor),
                                 "(%s) is a dual-source blend factor, but dualSrcBlend feature was not enabled.",
                                 string_VkBlendFactor(attachment_state.srcColorBlendFactor));
            }
        }
        if (IsSecondaryColorInputBlendFactor(attachment_state.dstColorBlendFactor)) {
            if (!enabled_features.dualSrcBlend) {
                skip |= LogError("VUID-VkPipelineColorBlendAttachmentState-dstColorBlendFactor-00609", device,
                                 attachment_loc.dot(Field::dstColorBlendFactor),
                                 "(%s) is a dual-source blend factor, but dualSrcBlend feature was not enabled.",
                                 string_VkBlendFactor(attachment_state.dstColorBlendFactor));
            }
        }
        if (IsSecondaryColorInputBlendFactor(attachment_state.srcAlphaBlendFactor)) {
            if (!enabled_features.dualSrcBlend) {
                skip |= LogError("VUID-VkPipelineColorBlendAttachmentState-srcAlphaBlendFactor-00610", device,
                                 attachment_loc.dot(Field::srcAlphaBlendFactor),
                                 "(%s) is a dual-source blend factor, but dualSrcBlend feature was not enabled.",
                                 string_VkBlendFactor(attachment_state.srcAlphaBlendFactor));
            }
        }
        if (IsSecondaryColorInputBlendFactor(attachment_state.dstAlphaBlendFactor)) {
            if (!enabled_features.dualSrcBlend) {
                skip |= LogError("VUID-VkPipelineColorBlendAttachmentState-dstAlphaBlendFactor-00611", device,
                                 attachment_loc.dot(Field::dstAlphaBlendFactor),
                                 "(%s) is a dual-source blend factor, but dualSrcBlend feature was not enabled.",
                                 string_VkBlendFactor(attachment_state.dstAlphaBlendFactor));
            }
        }

        // if blendEnabled is false, these values are ignored
        if (attachment_state.blendEnable) {
            bool advance_blend = false;
            if (IsAdvanceBlendOperation(attachment_state.colorBlendOp)) {
                advance_blend = true;
                if (phys_dev_ext_props.blend_operation_advanced_props.advancedBlendAllOperations == VK_FALSE) {
                    // This VUID checks if a subset of advance blend ops are allowed
                    switch (attachment_state.colorBlendOp) {
                        case VK_BLEND_OP_ZERO_EXT:
                        case VK_BLEND_OP_SRC_EXT:
                        case VK_BLEND_OP_DST_EXT:
                        case VK_BLEND_OP_SRC_OVER_EXT:
                        case VK_BLEND_OP_DST_OVER_EXT:
                        case VK_BLEND_OP_SRC_IN_EXT:
                        case VK_BLEND_OP_DST_IN_EXT:
                        case VK_BLEND_OP_SRC_OUT_EXT:
                        case VK_BLEND_OP_DST_OUT_EXT:
                        case VK_BLEND_OP_SRC_ATOP_EXT:
                        case VK_BLEND_OP_DST_ATOP_EXT:
                        case VK_BLEND_OP_XOR_EXT:
                        case VK_BLEND_OP_INVERT_EXT:
                        case VK_BLEND_OP_INVERT_RGB_EXT:
                        case VK_BLEND_OP_LINEARDODGE_EXT:
                        case VK_BLEND_OP_LINEARBURN_EXT:
                        case VK_BLEND_OP_VIVIDLIGHT_EXT:
                        case VK_BLEND_OP_LINEARLIGHT_EXT:
                        case VK_BLEND_OP_PINLIGHT_EXT:
                        case VK_BLEND_OP_HARDMIX_EXT:
                        case VK_BLEND_OP_PLUS_EXT:
                        case VK_BLEND_OP_PLUS_CLAMPED_EXT:
                        case VK_BLEND_OP_PLUS_CLAMPED_ALPHA_EXT:
                        case VK_BLEND_OP_PLUS_DARKER_EXT:
                        case VK_BLEND_OP_MINUS_EXT:
                        case VK_BLEND_OP_MINUS_CLAMPED_EXT:
                        case VK_BLEND_OP_CONTRAST_EXT:
                        case VK_BLEND_OP_INVERT_OVG_EXT:
                        case VK_BLEND_OP_RED_EXT:
                        case VK_BLEND_OP_GREEN_EXT:
                        case VK_BLEND_OP_BLUE_EXT: {
                            skip |= LogError("VUID-VkPipelineColorBlendAttachmentState-advancedBlendAllOperations-01409", device,
                                             attachment_loc.dot(Field::colorBlendOp),
                                             "(%s) but "
                                             "VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT::"
                                             "advancedBlendAllOperations is "
                                             "VK_FALSE",
                                             string_VkBlendOp(attachment_state.colorBlendOp));
                            break;
                        }
                        default:
                            break;
                    }
                }

                if (phys_dev_ext_props.blend_operation_advanced_props.advancedBlendIndependentBlend == VK_FALSE &&
                    attachment_state.colorBlendOp != first_color_blend_op) {
                    skip |= LogError("VUID-VkPipelineColorBlendAttachmentState-advancedBlendIndependentBlend-01407", device,
                                     attachment_loc.dot(Field::colorBlendOp), "(%s) is not same the other attachments (%s).",
                                     string_VkBlendOp(attachment_state.colorBlendOp), string_VkBlendOp(first_color_blend_op));
                }
            }

            if (IsAdvanceBlendOperation(attachment_state.alphaBlendOp)) {
                advance_blend = true;
                if (phys_dev_ext_props.blend_operation_advanced_props.advancedBlendIndependentBlend == VK_FALSE &&
                    attachment_state.alphaBlendOp != first_alpha_blend_op) {
                    skip |= LogError("VUID-VkPipelineColorBlendAttachmentState-advancedBlendIndependentBlend-01408", device,
                                     attachment_loc.dot(Field::alphaBlendOp), "(%s) is not same the other attachments (%s).",
                                     string_VkBlendOp(attachment_state.alphaBlendOp), string_VkBlendOp(first_alpha_blend_op));
                }
            }

            if (advance_blend) {
                const uint32_t color_attachment_count = pipeline.rendering_create_info
                                                            ? pipeline.rendering_create_info->colorAttachmentCount
                                                            : subpass_desc->colorAttachmentCount;
                if (attachment_state.colorBlendOp != attachment_state.alphaBlendOp) {
                    skip |=
                        LogError("VUID-VkPipelineColorBlendAttachmentState-colorBlendOp-01406", device, attachment_loc,
                                 "has different colorBlendOp (%s) and alphaBlendOp (%s) but one of "
                                 "them is an advance blend operation.",
                                 string_VkBlendOp(attachment_state.colorBlendOp), string_VkBlendOp(attachment_state.alphaBlendOp));
                } else if (color_attachment_count >
                           phys_dev_ext_props.blend_operation_advanced_props.advancedBlendMaxColorAttachments) {
                    // color_attachment_count is found one of multiple spots above
                    //
                    // error can guarantee it is the same VkBlendOp
                    skip |= LogError("VUID-VkPipelineColorBlendAttachmentState-colorBlendOp-01410", device, attachment_loc,
                                     "has an advance blend operation (%s) but the colorAttachmentCount (%" PRIu32
                                     ") is larger than advancedBlendMaxColorAttachments (%" PRIu32 ").",
                                     string_VkBlendOp(attachment_state.colorBlendOp), color_attachment_count,
                                     phys_dev_ext_props.blend_operation_advanced_props.advancedBlendMaxColorAttachments);
                    break;  // if this fails once, will fail every iteration
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineColorBlendState(const vvl::Pipeline &pipeline,
                                                         const vku::safe_VkSubpassDescription2 *subpass_desc,
                                                         const Location &create_info_loc) const {
    bool skip = false;
    const Location color_loc = create_info_loc.dot(Field::pColorBlendState);
    const auto color_blend_state = pipeline.ColorBlendState();
    if (!color_blend_state) {
        return skip;
    }
    const auto &rp_state = pipeline.RenderPassState();
    const bool null_rp = (!rp_state || rp_state->VkHandle() == VK_NULL_HANDLE);
    const bool raster_order_attach_color =
        color_blend_state->flags & VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_BIT_EXT;

    if (raster_order_attach_color) {
        if (!enabled_features.rasterizationOrderColorAttachmentAccess) {
            skip |= LogError("VUID-VkPipelineColorBlendStateCreateInfo-rasterizationOrderColorAttachmentAccess-06465", device,
                             color_loc.dot(Field::flags), "(%s) but rasterizationOrderColorAttachmentAccess feature is not enabled",
                             string_VkPipelineColorBlendStateCreateFlags(color_blend_state->flags).c_str());
        }

        if (!enabled_features.dynamicRenderingLocalRead && null_rp) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-flags-06482", device, color_loc.dot(Field::flags),
                             "includes "
                             "VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_BIT_EXT, but "
                             "renderpass is not VK_NULL_HANDLE.");
        }

        if (!null_rp && subpass_desc &&
            (subpass_desc->flags & VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_COLOR_ACCESS_BIT_EXT) == 0) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-09527", rp_state->Handle(), color_loc.dot(Field::flags),
                             "(%s) but VkRenderPassCreateInfo::VkSubpassDescription::flags == %s",
                             string_VkPipelineColorBlendStateCreateFlags(color_blend_state->flags).c_str(),
                             string_VkSubpassDescriptionFlags(subpass_desc->flags).c_str());
        }
    }

    // VkAttachmentSampleCountInfoAMD == VkAttachmentSampleCountInfoNV
    const auto &pipeline_ci = pipeline.GraphicsCreateInfo();
    const auto *attachment_sample_count_info = vku::FindStructInPNextChain<VkAttachmentSampleCountInfoAMD>(pipeline_ci.pNext);
    const auto *rendering_struct = pipeline.rendering_create_info;
    if (null_rp && rendering_struct && attachment_sample_count_info &&
        (attachment_sample_count_info->colorAttachmentCount != rendering_struct->colorAttachmentCount)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06063", device,
                         create_info_loc.pNext(Struct::VkAttachmentSampleCountInfoAMD, Field::attachmentCount),
                         "(%" PRIu32 ") is different then %s (%" PRIu32 ").", attachment_sample_count_info->colorAttachmentCount,
                         create_info_loc.pNext(Struct::VkPipelineRenderingCreateInfo, Field::colorAttachmentCount).Fields().c_str(),
                         rendering_struct->colorAttachmentCount);
    }

    if (!null_rp && subpass_desc && color_blend_state->attachmentCount != subpass_desc->colorAttachmentCount) {
        // attachmentCount can be ignored
        const bool ignored =
            pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT) &&
            pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT) &&
            pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT) &&
            (pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT) || !enabled_features.advancedBlendCoherentOperations);
        if (!ignored) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-07609", device, color_loc.dot(Field::attachmentCount),
                             "(%" PRIu32 ") is different than %s pSubpasses[%" PRIu32 "].colorAttachmentCount (%" PRIu32 ").",
                             color_blend_state->attachmentCount, FormatHandle(rp_state->Handle()).c_str(), pipeline.Subpass(),
                             subpass_desc->colorAttachmentCount);
        }
    }

    skip |= ValidateGraphicsPipelineColorBlendAttachmentState(pipeline, subpass_desc, color_loc);

    if (!enabled_features.logicOp && (color_blend_state->logicOpEnable != VK_FALSE)) {
        skip |= LogError("VUID-VkPipelineColorBlendStateCreateInfo-logicOpEnable-00606", device,
                         color_loc.dot(Field::logicOpEnable), "is VK_TRUE, but the logicOp feature was not enabled.");
    }

    auto color_write = vku::FindStructInPNextChain<VkPipelineColorWriteCreateInfoEXT>(color_blend_state->pNext);
    if (color_write) {
        if (color_write->attachmentCount > phys_dev_props.limits.maxColorAttachments) {
            skip |= LogError("VUID-VkPipelineColorWriteCreateInfoEXT-attachmentCount-06655", device,
                             color_loc.pNext(Struct::VkPipelineColorWriteCreateInfoEXT, Field::attachmentCount),
                             "(%" PRIu32 ") is larger than the maxColorAttachments limit (%" PRIu32 ").",
                             color_write->attachmentCount, phys_dev_props.limits.maxColorAttachments);
        }
        if (!enabled_features.colorWriteEnable) {
            for (uint32_t i = 0; i < color_write->attachmentCount; ++i) {
                if (color_write->pColorWriteEnables[i] != VK_TRUE) {
                    skip |= LogError("VUID-VkPipelineColorWriteCreateInfoEXT-pAttachments-04801", device,
                                     color_loc.pNext(Struct::VkPipelineColorWriteCreateInfoEXT, Field::pColorWriteEnables, i),
                                     "is VK_FALSE, but colorWriteEnable feature was not enabled.");
                }
            }
        }
    }
    const auto *color_blend_advanced = vku::FindStructInPNextChain<VkPipelineColorBlendAdvancedStateCreateInfoEXT>(color_blend_state->pNext);
    if (color_blend_advanced) {
        if (!phys_dev_ext_props.blend_operation_advanced_props.advancedBlendCorrelatedOverlap &&
            color_blend_advanced->blendOverlap != VK_BLEND_OVERLAP_UNCORRELATED_EXT) {
            skip |= LogError("VUID-VkPipelineColorBlendAdvancedStateCreateInfoEXT-blendOverlap-01426", device,
                             color_loc.pNext(Struct::VkPipelineColorBlendAdvancedStateCreateInfoEXT, Field::blendOverlap),
                             "is %s, but advancedBlendCorrelatedOverlap was not enabled.",
                             string_VkBlendOverlapEXT(color_blend_advanced->blendOverlap));
        }
        if (!phys_dev_ext_props.blend_operation_advanced_props.advancedBlendNonPremultipliedDstColor &&
            color_blend_advanced->dstPremultiplied != VK_TRUE) {
            skip |= LogError("VUID-VkPipelineColorBlendAdvancedStateCreateInfoEXT-dstPremultiplied-01425", device,
                             color_loc.pNext(Struct::VkPipelineColorBlendAdvancedStateCreateInfoEXT, Field::dstPremultiplied),
                             "is VK_FALSE, but advancedBlendNonPremultipliedDstColor was not enabled.");
        }
        if (!phys_dev_ext_props.blend_operation_advanced_props.advancedBlendNonPremultipliedSrcColor &&
            color_blend_advanced->srcPremultiplied != VK_TRUE) {
            skip |= LogError("VUID-VkPipelineColorBlendAdvancedStateCreateInfoEXT-srcPremultiplied-01424", device,
                             color_loc.pNext(Struct::VkPipelineColorBlendAdvancedStateCreateInfoEXT, Field::srcPremultiplied),
                             "is VK_FALSE, but advancedBlendNonPremultipliedSrcColor was not enabled.");
        }
    }
    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineRasterizationState(const vvl::Pipeline &pipeline,
                                                            const vku::safe_VkSubpassDescription2 *subpass_desc,
                                                            const Location &create_info_loc) const {
    bool skip = false;
    const Location raster_loc = create_info_loc.dot(Field::pRasterizationState);
    const auto raster_state = pipeline.RasterizationState();
    if (!raster_state) {
        return skip;
    }
    if ((raster_state->depthClampEnable == VK_TRUE) && (!enabled_features.depthClamp)) {
        skip |= LogError("VUID-VkPipelineRasterizationStateCreateInfo-depthClampEnable-00782", device,
                         raster_loc.dot(Field::depthClampEnable), "is VK_TRUE, but the depthClamp feature was not enabled.");
    }

    if (!pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_BIAS) && (raster_state->depthBiasClamp != 0.0) &&
        (!enabled_features.depthBiasClamp)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-00754", device, raster_loc.dot(Field::depthBiasClamp),
                         "is %f, but the depthBiasClamp feature was not enabled", raster_state->depthBiasClamp);
    }

    if (subpass_desc && subpass_desc->viewMask != 0) {
        skip |= ValidateMultiViewShaders(pipeline, create_info_loc.dot(Field::pSubpasses, pipeline.Subpass()).dot(Field::viewMask),
                                         subpass_desc->viewMask, false);
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
            if (ms_state && (ms_state->alphaToOneEnable == VK_TRUE) && (!enabled_features.alphaToOne)) {
                skip |= LogError("VUID-VkPipelineMultisampleStateCreateInfo-alphaToOneEnable-00785", device,
                                 create_info_loc.dot(Field::pMultisampleState).dot(Field::alphaToOneEnable),
                                 "is VK_TRUE, but the alphaToOne feature was not enabled.");
            }
        }

        // If subpass uses a depth/stencil attachment, pDepthStencilState must be a pointer to a valid structure
        if (pipeline.fragment_shader_state) {
            if (subpass_desc && subpass_desc->pDepthStencilAttachment &&
                subpass_desc->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
                const Location ds_loc = create_info_loc.dot(Field::pDepthStencilState);
                const auto ds_state = pipeline.DepthStencilState();
                if (!ds_state) {
                    if (!pipeline.IsDepthStencilStateDynamic() || !IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state3)) {
                        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-09028", device, ds_loc,
                                         "is NULL when rasterization is enabled "
                                         "and subpass %" PRIu32 " uses a depth/stencil attachment.",
                                         pipeline.Subpass());
                    }
                } else if (ds_state->depthBoundsTestEnable == VK_TRUE) {
                    if (!enabled_features.depthBounds) {
                        skip |= LogError("VUID-VkPipelineDepthStencilStateCreateInfo-depthBoundsTestEnable-00598", device,
                                         ds_loc.dot(Field::depthBoundsTestEnable),
                                         "depthBoundsTestEnable is VK_TRUE, but depthBounds feature was not enabled.");
                    }

                    if (!IsExtEnabled(device_extensions.vk_ext_depth_range_unrestricted) &&
                        !pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_BOUNDS)) {
                        const float minDepthBounds = ds_state->minDepthBounds;
                        const float maxDepthBounds = ds_state->maxDepthBounds;
                        if (!(minDepthBounds >= 0.0) || !(minDepthBounds <= 1.0)) {
                            skip |= LogError(
                                "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-02510", device, ds_loc.dot(Field::minDepthBounds),
                                "is %f, depthBoundsTestEnable is VK_TRUE, but VK_EXT_depth_range_unrestricted extension "
                                "is not enabled (and not using VK_DYNAMIC_STATE_DEPTH_BOUNDS).",
                                minDepthBounds);
                        }
                        if (!(maxDepthBounds >= 0.0) || !(maxDepthBounds <= 1.0)) {
                            skip |= LogError(
                                "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-02510", device, ds_loc.dot(Field::minDepthBounds),
                                "is %f, depthBoundsTestEnable is VK_TRUE, but VK_EXT_depth_range_unrestricted extension "
                                "is not enabled (and not using VK_DYNAMIC_STATE_DEPTH_BOUNDS).",
                                maxDepthBounds);
                        }
                    }
                }
            }
        }

        // If subpass uses color attachments, pColorBlendState must be valid pointer
        if (pipeline.fragment_output_state && subpass_desc && !pipeline.fragment_output_state->color_blend_state &&
            !pipeline.IsColorBlendStateDynamic()) {
            uint32_t color_attachment_count = 0;
            for (uint32_t i = 0; i < subpass_desc->colorAttachmentCount; ++i) {
                if (subpass_desc->pColorAttachments[i].attachment != VK_ATTACHMENT_UNUSED) {
                    ++color_attachment_count;
                }
            }

            if (color_attachment_count > 0) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-09030", device,
                                 create_info_loc.dot(Field::pColorBlendState),
                                 "is NULL when rasterization is enabled and "
                                 "subpass %" PRIu32 " uses color attachments.",
                                 pipeline.Subpass());
            }
        }
    }

    auto provoking_vertex_state_ci = vku::FindStructInPNextChain<VkPipelineRasterizationProvokingVertexStateCreateInfoEXT>(raster_state->pNext);
    if (provoking_vertex_state_ci && provoking_vertex_state_ci->provokingVertexMode == VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT &&
        !enabled_features.provokingVertexLast) {
        skip |=
            LogError("VUID-VkPipelineRasterizationProvokingVertexStateCreateInfoEXT-provokingVertexMode-04883", device,
                     raster_loc.pNext(Struct::VkPipelineRasterizationProvokingVertexStateCreateInfoEXT, Field::provokingVertexMode),
                     "is VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT but the provokingVertexLast feature was not enabled.");
    }

    const auto rasterization_state_stream_ci = vku::FindStructInPNextChain<VkPipelineRasterizationStateStreamCreateInfoEXT>(raster_state->pNext);
    if (rasterization_state_stream_ci) {
        if (!enabled_features.geometryStreams) {
            skip |= LogError("VUID-VkPipelineRasterizationStateStreamCreateInfoEXT-geometryStreams-02324", device, raster_loc,
                             "pNext chain includes VkPipelineRasterizationStateStreamCreateInfoEXT, but "
                             "geometryStreams feature was not enabled.");
        } else if (phys_dev_ext_props.transform_feedback_props.transformFeedbackRasterizationStreamSelect == VK_FALSE &&
                   rasterization_state_stream_ci->rasterizationStream != 0) {
            skip |= LogError("VUID-VkPipelineRasterizationStateStreamCreateInfoEXT-rasterizationStream-02326", device,
                             raster_loc.pNext(Struct::VkPipelineRasterizationStateStreamCreateInfoEXT, Field::rasterizationStream),
                             "is (%" PRIu32 ") but transformFeedbackRasterizationStreamSelect is VK_FALSE.",
                             rasterization_state_stream_ci->rasterizationStream);
        } else if (rasterization_state_stream_ci->rasterizationStream >=
                   phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams) {
            skip |= LogError("VUID-VkPipelineRasterizationStateStreamCreateInfoEXT-rasterizationStream-02325", device,
                             raster_loc.pNext(Struct::VkPipelineRasterizationStateStreamCreateInfoEXT, Field::rasterizationStream),
                             "(%" PRIu32 ") is not less than maxTransformFeedbackStreams (%" PRIu32 ").",
                             rasterization_state_stream_ci->rasterizationStream,
                             phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams);
        }
    }

    const auto rasterization_conservative_state_ci =
        vku::FindStructInPNextChain<VkPipelineRasterizationConservativeStateCreateInfoEXT>(raster_state->pNext);
    if (rasterization_conservative_state_ci) {
        if (rasterization_conservative_state_ci->extraPrimitiveOverestimationSize < 0.0f ||
            rasterization_conservative_state_ci->extraPrimitiveOverestimationSize >
                phys_dev_ext_props.conservative_rasterization_props.maxExtraPrimitiveOverestimationSize) {
            skip |= LogError("VUID-VkPipelineRasterizationConservativeStateCreateInfoEXT-extraPrimitiveOverestimationSize-01769",
                             device,
                             raster_loc.pNext(Struct::VkPipelineRasterizationConservativeStateCreateInfoEXT,
                                              Field::extraPrimitiveOverestimationSize),
                             "is (%f), which is not between 0.0 and "
                             "maxExtraPrimitiveOverestimationSize (%f).",
                             rasterization_conservative_state_ci->extraPrimitiveOverestimationSize,
                             phys_dev_ext_props.conservative_rasterization_props.maxExtraPrimitiveOverestimationSize);
        }

        if (!phys_dev_ext_props.conservative_rasterization_props.conservativePointAndLineRasterization) {
            if (IsValueIn(pipeline.topology_at_rasterizer,
                          {VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
                           VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
                           VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY})) {
                if (rasterization_conservative_state_ci->conservativeRasterizationMode !=
                    VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT) {
                    if ((pipeline.create_info_shaders & VK_SHADER_STAGE_GEOMETRY_BIT) != 0) {
                        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-conservativePointAndLineRasterization-06760", device,
                                         raster_loc.pNext(Struct::VkPipelineRasterizationConservativeStateCreateInfoEXT,
                                                          Field::conservativeRasterizationMode),
                                         "is %s, but geometry shader output primitive is %s and "
                                         "VkPhysicalDeviceConservativeRasterizationPropertiesEXT::"
                                         "conservativePointAndLineRasterization is false.",
                                         string_VkConservativeRasterizationModeEXT(
                                             rasterization_conservative_state_ci->conservativeRasterizationMode),
                                         string_VkPrimitiveTopology(pipeline.topology_at_rasterizer));
                    }
                    if ((pipeline.create_info_shaders & VK_SHADER_STAGE_MESH_BIT_EXT) != 0) {
                        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-conservativePointAndLineRasterization-06761", device,
                                         raster_loc.pNext(Struct::VkPipelineRasterizationConservativeStateCreateInfoEXT,
                                                          Field::conservativeRasterizationMode),
                                         "is %s, but mesh shader output primitive is %s and "
                                         "VkPhysicalDeviceConservativeRasterizationPropertiesEXT::"
                                         "conservativePointAndLineRasterization is false.",
                                         string_VkConservativeRasterizationModeEXT(
                                             rasterization_conservative_state_ci->conservativeRasterizationMode),
                                         string_VkPrimitiveTopology(pipeline.topology_at_rasterizer));
                    }
                }
            }
        }
    }

    if (const auto *depth_bias_representation = vku::FindStructInPNextChain<VkDepthBiasRepresentationInfoEXT>(raster_state->pNext);
        depth_bias_representation != nullptr) {
        ValidateDepthBiasRepresentationInfo(raster_loc, LogObjectList(device), *depth_bias_representation);
    }

    return skip;
}

bool CoreChecks::ValidateSampleLocationsInfo(const VkSampleLocationsInfoEXT &sample_location_info, const Location &loc) const {
    bool skip = false;
    const VkSampleCountFlagBits sample_count = sample_location_info.sampleLocationsPerPixel;
    const uint32_t sample_total_size = sample_location_info.sampleLocationGridSize.width *
                                       sample_location_info.sampleLocationGridSize.height * SampleCountSize(sample_count);
    if (sample_location_info.sampleLocationsCount != sample_total_size) {
        skip |= LogError("VUID-VkSampleLocationsInfoEXT-sampleLocationsCount-01527", device, loc.dot(Field::sampleLocationsCount),
                         "(%" PRIu32
                         ") must equal grid width * grid height * pixel "
                         "sample rate which currently is (%" PRIu32 " * %" PRIu32 " * %" PRIu32 ").",
                         sample_location_info.sampleLocationsCount, sample_location_info.sampleLocationGridSize.width,
                         sample_location_info.sampleLocationGridSize.height, SampleCountSize(sample_count));
    }
    if ((phys_dev_ext_props.sample_locations_props.sampleLocationSampleCounts & sample_count) == 0) {
        skip |=
            LogError("VUID-VkSampleLocationsInfoEXT-sampleLocationsPerPixel-01526", device, loc.dot(Field::sampleLocationsPerPixel),
                     "is %s, but VkPhysicalDeviceSampleLocationsPropertiesEXT::sampleLocationSampleCounts is %s.",
                     string_VkSampleCountFlagBits(sample_count),
                     string_VkSampleCountFlags(phys_dev_ext_props.sample_locations_props.sampleLocationSampleCounts).c_str());
    }

    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineMultisampleState(const vvl::Pipeline &pipeline,
                                                          const vku::safe_VkSubpassDescription2 *subpass_desc,
                                                          const Location &create_info_loc) const {
    bool skip = false;
    const Location ms_loc = create_info_loc.dot(Field::pMultisampleState);
    const auto *multisample_state = pipeline.MultisampleState();
    const auto &pipeline_ci = pipeline.GraphicsCreateInfo();
    if (subpass_desc && multisample_state) {
        const auto &rp_state = pipeline.RenderPassState();
        auto accum_color_samples = [subpass_desc, &rp_state](uint32_t &samples) {
            for (uint32_t i = 0; i < subpass_desc->colorAttachmentCount; i++) {
                const auto attachment = subpass_desc->pColorAttachments[i].attachment;
                if (attachment != VK_ATTACHMENT_UNUSED) {
                    samples |= static_cast<uint32_t>(rp_state->create_info.pAttachments[attachment].samples);
                }
            }
        };

        const uint32_t raster_samples = SampleCountSize(multisample_state->rasterizationSamples);
        if (!pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) {
            if (!(IsExtEnabled(device_extensions.vk_amd_mixed_attachment_samples) ||
                  IsExtEnabled(device_extensions.vk_nv_framebuffer_mixed_samples) ||
                  (enabled_features.multisampledRenderToSingleSampled))) {
                uint32_t subpass_num_samples = 0;

                accum_color_samples(subpass_num_samples);

                if (subpass_desc->pDepthStencilAttachment &&
                    subpass_desc->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
                    const auto attachment = subpass_desc->pDepthStencilAttachment->attachment;
                    subpass_num_samples |= static_cast<uint32_t>(rp_state->create_info.pAttachments[attachment].samples);
                }

                // subpass_num_samples is 0 when the subpass has no attachments or if all attachments are VK_ATTACHMENT_UNUSED.
                // Only validate the value of subpass_num_samples if the subpass has attachments that are not VK_ATTACHMENT_UNUSED.
                if (subpass_num_samples && (!IsPowerOfTwo(subpass_num_samples) || (subpass_num_samples != raster_samples))) {
                    skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-multisampledRenderToSingleSampled-06853", device, ms_loc.dot(Field::rasterizationSamples),
                                     "(%" PRIu32 ") does not match the number of samples of the RenderPass color and/or depth attachment (%" PRIu32 ").",
                                     raster_samples, subpass_num_samples);
                }
            }

            if (IsExtEnabled(device_extensions.vk_amd_mixed_attachment_samples)) {
                VkSampleCountFlagBits max_sample_count = static_cast<VkSampleCountFlagBits>(0);
                for (uint32_t i = 0; i < subpass_desc->colorAttachmentCount; ++i) {
                    if (subpass_desc->pColorAttachments[i].attachment != VK_ATTACHMENT_UNUSED) {
                        max_sample_count =
                            std::max(max_sample_count,
                                     rp_state->create_info.pAttachments[subpass_desc->pColorAttachments[i].attachment].samples);
                    }
                }
                if (subpass_desc->pDepthStencilAttachment &&
                    subpass_desc->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
                    max_sample_count =
                        std::max(max_sample_count,
                                 rp_state->create_info.pAttachments[subpass_desc->pDepthStencilAttachment->attachment].samples);
                }
                const auto raster_state = pipeline.RasterizationState();
                if ((raster_state && raster_state->rasterizerDiscardEnable == VK_FALSE) &&
                    (max_sample_count != static_cast<VkSampleCountFlagBits>(0)) &&
                    (multisample_state->rasterizationSamples != max_sample_count)) {
                    skip |= LogError( "VUID-VkGraphicsPipelineCreateInfo-subpass-01505", device, ms_loc.dot(Field::rasterizationSamples),
                                     "(%s) is different from the max attachment samples (%s) used in pSubpasses[%" PRIu32 "].",
                                     string_VkSampleCountFlagBits(multisample_state->rasterizationSamples),
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
                        static_cast<uint32_t>(rp_state->create_info.pAttachments[attachment].samples);
                    const auto ds_state = pipeline.DepthStencilState();
                    if (ds_state) {
                        const bool ds_test_enabled = (ds_state->depthTestEnable == VK_TRUE) ||
                                                     (ds_state->depthBoundsTestEnable == VK_TRUE) ||
                                                     (ds_state->stencilTestEnable == VK_TRUE);

                        if (ds_test_enabled &&
                            (!IsPowerOfTwo(subpass_depth_samples) || (raster_samples != subpass_depth_samples))) {
                            skip |= LogError( "VUID-VkGraphicsPipelineCreateInfo-subpass-01411", device, ms_loc.dot(Field::rasterizationSamples),
                                             "(%" PRIu32 ") does not match the number of samples of the RenderPass depth attachment (%" PRIu32 ").",
                                              raster_samples, subpass_depth_samples);
                        }
                    }
                }

                if (IsPowerOfTwo(subpass_color_samples)) {
                    if (raster_samples < subpass_color_samples) {
                        skip |=
                            LogError("VUID-VkGraphicsPipelineCreateInfo-subpass-01412", device, ms_loc.dot(Field::rasterizationSamples),
                                     "(%" PRIu32 ") "
                                     "is not greater or equal to the number of samples of the RenderPass color attachment (%" PRIu32 ").",
                                     raster_samples, subpass_color_samples);
                    }

                    if (multisample_state) {
                        if ((raster_samples > subpass_color_samples) && (multisample_state->sampleShadingEnable == VK_TRUE)) {
                            skip |= LogError("VUID-VkPipelineMultisampleStateCreateInfo-rasterizationSamples-01415", device, ms_loc.dot(Field::rasterizationSamples),
                                             "(%" PRIu32 ") is greater than the number of "
                                             "samples of the "
                                             "subpass color attachment (%" PRIu32 ") and sampleShadingEnable is VK_TRUE.",
                                              raster_samples, subpass_color_samples);
                        }

                        const auto *coverage_modulation_state =
                            vku::FindStructInPNextChain<VkPipelineCoverageModulationStateCreateInfoNV>(multisample_state->pNext);

                        if (coverage_modulation_state && (coverage_modulation_state->coverageModulationTableEnable == VK_TRUE)) {
                            if (coverage_modulation_state->coverageModulationTableCount !=
                                (raster_samples / subpass_color_samples)) {
                                skip |= LogError(
                                    "VUID-VkPipelineCoverageModulationStateCreateInfoNV-coverageModulationTableEnable-01405", device, ms_loc.pNext(Struct::VkPipelineCoverageModulationStateCreateInfoNV, Field::coverageModulationTableCount),
                                    "is %" PRIu32 ".",
                                     coverage_modulation_state->coverageModulationTableCount);
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
                    subpass_depth_samples = static_cast<uint32_t>(rp_state->create_info.pAttachments[attachment].samples);
                }

                if (multisample_state && IsPowerOfTwo(subpass_color_samples) &&
                    (subpass_depth_samples == 0 || IsPowerOfTwo(subpass_depth_samples))) {
                    const auto *coverage_reduction_state =
                        vku::FindStructInPNextChain<VkPipelineCoverageReductionStateCreateInfoNV>(multisample_state->pNext);

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
                            skip |=
                                LogError("VUID-VkGraphicsPipelineCreateInfo-coverageReductionMode-02722", device, create_info_loc,
                                         "specifies a combination of coverage "
                                         "reduction mode (%s), pMultisampleState->rasterizationSamples (%" PRIu32
                                         "), sample counts for "
                                         "the subpass color and depth/stencil attachments is not a valid combination returned by "
                                         "vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV.",
                                         string_VkCoverageReductionModeNV(coverage_reduction_mode), raster_samples);
                        }
                    }
                }
            }
            const auto msrtss_info = vku::FindStructInPNextChain<VkMultisampledRenderToSingleSampledInfoEXT>(subpass_desc->pNext);
            if (msrtss_info && msrtss_info->multisampledRenderToSingleSampledEnable &&
                (msrtss_info->rasterizationSamples != multisample_state->rasterizationSamples)) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06854", rp_state->Handle(),
                                 create_info_loc.dot(Field::pSubpasses, pipeline.Subpass())
                                     .pNext(Struct::VkMultisampledRenderToSingleSampledInfoEXT, Field::rasterizationSamples),
                                 "(%" PRIu32 ") is not equal to %s (%" PRIu32
                                 ") and multisampledRenderToSingleSampledEnable is VK_TRUE.",
                                 msrtss_info->rasterizationSamples, ms_loc.dot(Field::rasterizationSamples).Fields().c_str(),
                                 multisample_state->rasterizationSamples);
            }

            if (rp_state->UsesNoAttachment(pipeline.Subpass())) {
                if ((multisample_state->rasterizationSamples & phys_dev_props.limits.framebufferNoAttachmentsSampleCounts) == 0) {
                    skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-subpass-00758", rp_state->Handle(),
                                     ms_loc.dot(Field::rasterizationSamples),
                                     "(%s) is not in "
                                     "framebufferNoAttachmentsSampleCounts (%s) but attempting to use a zero-attachment subpass.",
                                     string_VkSampleCountFlagBits(multisample_state->rasterizationSamples),
                                     string_VkSampleCountFlags(phys_dev_props.limits.framebufferNoAttachmentsSampleCounts).c_str());
                }
            }
        }

        // VK_NV_fragment_coverage_to_color
        const auto coverage_to_color_state = vku::FindStructInPNextChain<VkPipelineCoverageToColorStateCreateInfoNV>(multisample_state);
        if (coverage_to_color_state && coverage_to_color_state->coverageToColorEnable == VK_TRUE) {
            bool attachment_is_valid = false;
            std::string error_detail;

            if (coverage_to_color_state->coverageToColorLocation < subpass_desc->colorAttachmentCount) {
                const auto &color_attachment_ref =
                    subpass_desc->pColorAttachments[coverage_to_color_state->coverageToColorLocation];
                if (color_attachment_ref.attachment != VK_ATTACHMENT_UNUSED) {
                    const auto &color_attachment = rp_state->create_info.pAttachments[color_attachment_ref.attachment];

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
                            str << "references an attachment with an invalid format (" << string_VkFormat(color_attachment.format)
                                << ").";
                            error_detail = str.str();
                            break;
                    }
                } else {
                    std::ostringstream str;
                    str << "references an invalid attachment. The subpass pColorAttachments["
                        << coverage_to_color_state->coverageToColorLocation << "].attachment has the value VK_ATTACHMENT_UNUSED.";
                    error_detail = str.str();
                }
            } else {
                std::ostringstream str;
                str << "references an non-existing attachment since the subpass colorAttachmentCount is "
                    << subpass_desc->colorAttachmentCount << ".";
                error_detail = str.str();
            }

            if (!attachment_is_valid) {
                skip |= LogError("VUID-VkPipelineCoverageToColorStateCreateInfoNV-coverageToColorEnable-01404", device, ms_loc.pNext(Struct::VkPipelineCoverageToColorStateCreateInfoNV, Field::coverageToColorLocation),
                                 "is %" PRIu32 ", but %s",
                                  coverage_to_color_state->coverageToColorLocation, error_detail.c_str());
            }
        }

        // VK_EXT_sample_locations
        const auto *sample_location_state =
            vku::FindStructInPNextChain<VkPipelineSampleLocationsStateCreateInfoEXT>(multisample_state->pNext);
        if (sample_location_state != nullptr) {
            if ((sample_location_state->sampleLocationsEnable == VK_TRUE) &&
                (pipeline.IsDynamic(VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT) == false) &&
                (pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT) == false)) {
                const VkSampleLocationsInfoEXT sample_location_info = sample_location_state->sampleLocationsInfo;
                const Location sample_info_loc = ms_loc.pNext(Struct::VkPipelineSampleLocationsStateCreateInfoEXT, Field::sampleLocationsInfo);
                skip |= ValidateSampleLocationsInfo(sample_location_info, sample_info_loc.dot(Field::sampleLocationsInfo));
                const VkExtent2D grid_size = sample_location_info.sampleLocationGridSize;

                VkMultisamplePropertiesEXT multisample_prop = vku::InitStructHelper();
                DispatchGetPhysicalDeviceMultisamplePropertiesEXT(physical_device, multisample_state->rasterizationSamples,
                                                                  &multisample_prop);
                const VkExtent2D max_grid_size = multisample_prop.maxSampleLocationGridSize;

                // Note order or "divide" in "sampleLocationsInfo must evenly divide VkMultisamplePropertiesEXT"
                if (SafeModulo(max_grid_size.width, grid_size.width) != 0) {
                    skip |= LogError(
                        "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07610", device, sample_info_loc.dot(Field::sampleLocationGridSize).dot(Field::width),
                        "(%" PRIu32 ") is not evenly divided by VkMultisamplePropertiesEXT::sampleLocationGridSize.width (%" PRIu32 ").",
                         grid_size.width, max_grid_size.width);
                }
                if (SafeModulo(max_grid_size.height, grid_size.height) != 0) {
                    skip |= LogError(
                        "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07611", device, sample_info_loc.dot(Field::sampleLocationGridSize).dot(Field::height),
                        "(%" PRIu32 ") is not evenly divided by VkMultisamplePropertiesEXT::sampleLocationGridSize.height (%" PRIu32 ").",
                         grid_size.height, max_grid_size.height);
                }
                if (sample_location_info.sampleLocationsPerPixel != multisample_state->rasterizationSamples) {
                    skip |= LogError(
                        "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07612", device, sample_info_loc.dot(Field::sampleLocationsPerPixel),
                        "(%s) is different from %s (%s).",
                        string_VkSampleCountFlagBits(sample_location_info.sampleLocationsPerPixel), ms_loc.dot(Field::rasterizationSamples).Fields().c_str(),
                        string_VkSampleCountFlagBits(multisample_state->rasterizationSamples));
                }
            }
        }

        if (IsExtEnabled(device_extensions.vk_qcom_render_pass_shader_resolve)) {
            uint32_t subpass_input_attachment_samples = 0;

            for (uint32_t i = 0; i < subpass_desc->inputAttachmentCount; i++) {
                const auto attachment = subpass_desc->pInputAttachments[i].attachment;
                if (attachment != VK_ATTACHMENT_UNUSED) {
                    subpass_input_attachment_samples |=
                        static_cast<uint32_t>(rp_state->create_info.pAttachments[attachment].samples);
                }
            }

            if ((subpass_desc->flags & VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM) != 0) {
                if ((raster_samples != subpass_input_attachment_samples) &&
                    !pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) {
                    skip |=
                        LogError("VUID-VkGraphicsPipelineCreateInfo-rasterizationSamples-04899", device, ms_loc.dot(Field::rasterizationSamples),
                                 "%s is different then pSubpasses[%" PRIu32 "] input attachment samples (%" PRIu32 ") but flags include VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM.",
                                 string_VkSampleCountFlagBits(multisample_state->rasterizationSamples), pipeline.Subpass(), subpass_input_attachment_samples);
                }
                if (multisample_state->sampleShadingEnable == VK_TRUE) {
                    skip |= LogError( "VUID-VkGraphicsPipelineCreateInfo-sampleShadingEnable-04900", device, ms_loc.dot(Field::sampleShadingEnable),
                                     "is VK_TRUE, but pSubpasses[%" PRIu32 "] includes "
                                     "VK_SUBPASS_DESCRIPTION_FRAGMENT_REGION_BIT_QCOM. ",
                                     pipeline.Subpass());
                }
            }
        }
    }

    if (IsExtEnabled(device_extensions.vk_ext_graphics_pipeline_library)) {
        if (pipeline.OwnsSubState(pipeline.fragment_output_state) && multisample_state == nullptr) {
            // if VK_KHR_dynamic_rendering is not enabled, can be null renderpass if using GPL
            if (pipeline_ci.renderPass != VK_NULL_HANDLE) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderpass-06631", device, ms_loc,
                                 "is NULL, but pipeline is being created with fragment shader that uses samples.");
            }
        }
    }

    if (!pipeline_ci.pMultisampleState && pipeline.OwnsSubState(pipeline.fragment_output_state)) {
        // Don't need to check for VK_EXT_extended_dynamic_state3 since it would be on if using these VkDynamicState
        const bool dynamic_alpha_to_one =
            pipeline.IsDynamic(VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT) || !enabled_features.alphaToOne;
        if (!pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT) ||
            !pipeline.IsDynamic(VK_DYNAMIC_STATE_SAMPLE_MASK_EXT) ||
            !pipeline.IsDynamic(VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT) || !dynamic_alpha_to_one) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pMultisampleState-09026", device, ms_loc, "is NULL.");
        }
    }

    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineDepthStencilState(const vvl::Pipeline &pipeline,
                                                           const vku::safe_VkSubpassDescription2 *subpass_desc,
                                                           const Location &create_info_loc) const {
    bool skip = false;
    const Location ds_loc = create_info_loc.dot(Field::pDepthStencilState);
    const auto ds_state = pipeline.DepthStencilState();
    const auto &rp_state = pipeline.RenderPassState();
    const bool null_rp = (!rp_state || rp_state->VkHandle() == VK_NULL_HANDLE);
    if (!ds_state) {
        if (null_rp && pipeline.fragment_shader_state && !pipeline.fragment_output_state) {
            if (!pipeline.IsDepthStencilStateDynamic() || !IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state3)) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-09035", device, ds_loc, "is NULL.");
            }
        }
        return skip;  // nothing else to validate if Depth Stencil state is empty
    }

    const bool raster_order_attach_depth =
        ds_state->flags & VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_EXT;
    const bool raster_order_attach_stencil =
        ds_state->flags & VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_EXT;

    if (null_rp) {
        if (!enabled_features.dynamicRenderingLocalRead && (raster_order_attach_depth || raster_order_attach_stencil)) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-None-09526", device, ds_loc.dot(Field::flags),
                             "is %s but renderPass is VK_NULL_HANDLE.",
                             string_VkPipelineDepthStencilStateCreateFlags(ds_state->flags).c_str());
        }
    } else if (!rp_state->UsesDynamicRendering()) {
        if (raster_order_attach_depth) {
            if (!enabled_features.rasterizationOrderDepthAttachmentAccess) {
                skip |= LogError("VUID-VkPipelineDepthStencilStateCreateInfo-rasterizationOrderDepthAttachmentAccess-06463", device,
                                 ds_loc.dot(Field::flags),
                                 "is (%s) but rasterizationOrderDepthAttachmentAccess feature was not enabled",
                                 string_VkPipelineDepthStencilStateCreateFlags(ds_state->flags).c_str());
            }

            if (subpass_desc &&
                (subpass_desc->flags & VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_EXT) == 0) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-09528", rp_state->Handle(), ds_loc.dot(Field::flags),
                                 "is (%s) but VkRenderPassCreateInfo::VkSubpassDescription::flags == %s",
                                 string_VkPipelineDepthStencilStateCreateFlags(ds_state->flags).c_str(),
                                 string_VkSubpassDescriptionFlags(subpass_desc->flags).c_str());
            }
        }
        if (raster_order_attach_stencil) {
            if (!enabled_features.rasterizationOrderStencilAttachmentAccess) {
                skip |= LogError("VUID-VkPipelineDepthStencilStateCreateInfo-rasterizationOrderStencilAttachmentAccess-06464",
                                 device, ds_loc.dot(Field::flags),
                                 "is (%s) but rasterizationOrderStencilAttachmentAccess feature was not enabled",
                                 string_VkPipelineDepthStencilStateCreateFlags(ds_state->flags).c_str());
            }

            if (subpass_desc &&
                (subpass_desc->flags & VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_EXT) == 0) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-09529", rp_state->Handle(), ds_loc.dot(Field::flags),
                                 "is (%s) but VkRenderPassCreateInfo::VkSubpassDescription::flags == %s",
                                 string_VkPipelineDepthStencilStateCreateFlags(ds_state->flags).c_str(),
                                 string_VkSubpassDescriptionFlags(subpass_desc->flags).c_str());
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineDynamicState(const vvl::Pipeline &pipeline, const Location &create_info_loc) const {
    auto get_state_index = [&pipeline](const VkDynamicState state) {
        const auto dynamic_info = pipeline.GraphicsCreateInfo().pDynamicState;
        for (uint32_t i = 0; i < dynamic_info->dynamicStateCount; i++) {
            if (dynamic_info->pDynamicStates[i] == state) {
                return i;
            }
        }
        assert(false);
        return dynamic_info->dynamicStateCount;
    };

    bool skip = false;
    if (pipeline.create_info_shaders & VK_SHADER_STAGE_MESH_BIT_EXT) {
        if (pipeline.IsDynamic(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY)) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07065", device,
                             create_info_loc.dot(Field::pDynamicState)
                                 .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY)),
                             "is VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY, but the pipeline contains a mesh shader.");
        } else if (pipeline.IsDynamic(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE)) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07065", device,
                             create_info_loc.dot(Field::pDynamicState)
                                 .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE)),
                             "is VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE, but the pipeline contains a mesh shader.");
        }

        if (pipeline.IsDynamic(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE)) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07066", device,
                             create_info_loc.dot(Field::pDynamicState)
                                 .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE)),
                             "is VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE, but the pipeline contains a mesh shader.");
        } else if (pipeline.IsDynamic(VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT)) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07066", device,
                             create_info_loc.dot(Field::pDynamicState)
                                 .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT)),
                             "is VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT, but the pipeline contains a mesh shader.");
        }

        if (pipeline.IsDynamic(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT)) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-07067", device,
                             create_info_loc.dot(Field::pDynamicState)
                                 .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT)),
                             "is VK_DYNAMIC_STATE_VERTEX_INPUT_EXT, but the pipeline contains a mesh shader.");
        }
    }

    if (api_version < VK_API_VERSION_1_3 && !enabled_features.extendedDynamicState &&
        (pipeline.IsDynamic(VK_DYNAMIC_STATE_CULL_MODE) || pipeline.IsDynamic(VK_DYNAMIC_STATE_FRONT_FACE) ||
         pipeline.IsDynamic(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY) || pipeline.IsDynamic(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT) ||
         pipeline.IsDynamic(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT) ||
         pipeline.IsDynamic(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE) ||
         pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE) || pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE) ||
         pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_COMPARE_OP) || pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE) ||
         pipeline.IsDynamic(VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE) || pipeline.IsDynamic(VK_DYNAMIC_STATE_STENCIL_OP))) {
        skip |= LogError(
            "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-03378", device, create_info_loc.dot(Field::pDynamicState),
            "contains dynamic states from VK_EXT_extended_dynamic_state, but the extendedDynamicState feature was not enabled.");
    }

    if (api_version < VK_API_VERSION_1_3 && !enabled_features.extendedDynamicState2) {
        if (pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
            skip |=
                LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04868", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)),
                         "is VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE, but the extendedDynamicState2 feature was not enabled.");
        } else if (pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE)) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04868", device,
                             create_info_loc.dot(Field::pDynamicState)
                                 .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE)),
                             "is VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE, but the extendedDynamicState2 feature was not enabled.");
        } else if (pipeline.IsDynamic(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE)) {
            skip |=
                LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04868", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE)),
                         "is VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE, but the extendedDynamicState2 feature was not enabled.");
        }
    }

    if (!enabled_features.extendedDynamicState2LogicOp && pipeline.IsDynamic(VK_DYNAMIC_STATE_LOGIC_OP_EXT)) {
        skip |= LogError(
            "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04869", device,
            create_info_loc.dot(Field::pDynamicState).dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_LOGIC_OP_EXT)),
            "is VK_DYNAMIC_STATE_LOGIC_OP_EXT, but the extendedDynamicState2LogicOp feature was not enabled.");
    }

    if (!enabled_features.extendedDynamicState2PatchControlPoints &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04870", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT)),
                         "is VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT, but the extendedDynamicState2PatchControlPoints feature "
                         "was not enabled.");
    }

    if (!enabled_features.extendedDynamicState3TessellationDomainOrigin &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3TessellationDomainOrigin-07370", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT)),
                         "is VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT, but the "
                         "extendedDynamicState3TessellationDomainOrigin feature was not enabled.");
    }

    if (!enabled_features.extendedDynamicState3DepthClampEnable && pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT)) {
        skip |= LogError(
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3DepthClampEnable-07371", device,
            create_info_loc.dot(Field::pDynamicState)
                .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT)),
            "is VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT, but the extendedDynamicState3DepthClampEnable feature was not enabled.");
    }

    if (!enabled_features.extendedDynamicState3PolygonMode && pipeline.IsDynamic(VK_DYNAMIC_STATE_POLYGON_MODE_EXT)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3PolygonMode-07372", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_POLYGON_MODE_EXT)),
                         "is VK_DYNAMIC_STATE_POLYGON_MODE_EXT, but the extendedDynamicState3PolygonMode feature was not enabled.");
    }

    if (!enabled_features.extendedDynamicState3RasterizationSamples &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3RasterizationSamples-07373", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)),
                         "is VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT but the extendedDynamicState3RasterizationSamples feature "
                         "is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3SampleMask && pipeline.IsDynamic(VK_DYNAMIC_STATE_SAMPLE_MASK_EXT)) {
        skip |= LogError(
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3SampleMask-07374", device,
            create_info_loc.dot(Field::pDynamicState).dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_SAMPLE_MASK_EXT)),
            "is VK_DYNAMIC_STATE_SAMPLE_MASK_EXT but the extendedDynamicState3SampleMask feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3AlphaToCoverageEnable &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3AlphaToCoverageEnable-07375", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT)),
                         "is VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT but the extendedDynamicState3AlphaToCoverageEnable "
                         "feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3AlphaToOneEnable && pipeline.IsDynamic(VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT)) {
        skip |= LogError(
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3AlphaToOneEnable-07376", device,
            create_info_loc.dot(Field::pDynamicState)
                .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT)),
            "is VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT but the extendedDynamicState3AlphaToOneEnable feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3LogicOpEnable && pipeline.IsDynamic(VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT)) {
        skip |=
            LogError("VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3LogicOpEnable-07377", device,
                     create_info_loc.dot(Field::pDynamicState)
                         .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT)),
                     "is VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT but the extendedDynamicState3LogicOpEnable feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3ColorBlendEnable && pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT)) {
        skip |= LogError(
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ColorBlendEnable-07378", device,
            create_info_loc.dot(Field::pDynamicState)
                .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT)),
            "is VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT but the extendedDynamicState3ColorBlendEnable feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3ColorBlendEquation &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT)) {
        skip |= LogError(
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ColorBlendEquation-07379", device,
            create_info_loc.dot(Field::pDynamicState)
                .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT)),
            "is VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT but the extendedDynamicState3ColorBlendEquation feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3ColorWriteMask && pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT)) {
        skip |= LogError(
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ColorWriteMask-07380", device,
            create_info_loc.dot(Field::pDynamicState)
                .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT)),
            "is VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT but the extendedDynamicState3ColorWriteMask feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3RasterizationStream &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3RasterizationStream-07381", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT)),
                         "is VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT but the extendedDynamicState3RasterizationStream feature is "
                         "not enabled.");
    }

    if (!enabled_features.extendedDynamicState3ConservativeRasterizationMode &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ConservativeRasterizationMode-07382", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT)),
                         "is VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT but the "
                         "extendedDynamicState3ConservativeRasterizationMode feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3ExtraPrimitiveOverestimationSize &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ExtraPrimitiveOverestimationSize-07383", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT)),
                         "is VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT but the "
                         "extendedDynamicState3ExtraPrimitiveOverestimationSize feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3DepthClipEnable && pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT)) {
        skip |= LogError(
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3DepthClipEnable-07384", device,
            create_info_loc.dot(Field::pDynamicState)
                .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT)),
            "is VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT but the extendedDynamicState3DepthClipEnable feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3SampleLocationsEnable &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3SampleLocationsEnable-07385", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT)),
                         "is VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT but the extendedDynamicState3SampleLocationsEnable "
                         "feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3ColorBlendAdvanced &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT)) {
        skip |= LogError(
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ColorBlendAdvanced-07386", device,
            create_info_loc.dot(Field::pDynamicState)
                .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT)),
            "is VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT but the extendedDynamicState3ColorBlendAdvanced feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3ProvokingVertexMode &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ProvokingVertexMode-07387", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT)),
                         "is VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT but the extendedDynamicState3ProvokingVertexMode feature "
                         "is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3LineRasterizationMode &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3LineRasterizationMode-07388", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT)),
                         "is VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT but the extendedDynamicState3LineRasterizationMode "
                         "feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3LineStippleEnable && pipeline.IsDynamic(VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT)) {
        skip |= LogError(
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3LineStippleEnable-07389", device,
            create_info_loc.dot(Field::pDynamicState)
                .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT)),
            "is VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT but the extendedDynamicState3LineStippleEnable feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3DepthClipNegativeOneToOne &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3DepthClipNegativeOneToOne-07390", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT)),
                         "is VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT but the "
                         "extendedDynamicState3DepthClipNegativeOneToOne feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3ViewportWScalingEnable &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ViewportWScalingEnable-07391", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV)),
                         "is VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV but the extendedDynamicState3ViewportWScalingEnable "
                         "feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3ViewportSwizzle && pipeline.IsDynamic(VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV)) {
        skip |= LogError(
            "VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ViewportSwizzle-07392", device,
            create_info_loc.dot(Field::pDynamicState)
                .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV)),
            "is VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV but the extendedDynamicState3ViewportSwizzle feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3CoverageToColorEnable &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageToColorEnable-07393", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV)),
                         "is VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV but the extendedDynamicState3CoverageToColorEnable "
                         "feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3CoverageToColorLocation &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageToColorLocation-07394", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV)),
                         "is VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV but the extendedDynamicState3CoverageToColorLocation "
                         "feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3CoverageModulationMode &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageModulationMode-07395", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV)),
                         "is VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV but the extendedDynamicState3CoverageModulationMode "
                         "feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3CoverageModulationTableEnable &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageModulationTableEnable-07396", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV)),
                         "is VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV but the "
                         "extendedDynamicState3CoverageModulationTableEnable feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3CoverageModulationTable &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageModulationTable-07397", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV)),
                         "is VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV but the extendedDynamicState3CoverageModulationTable "
                         "feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3CoverageReductionMode &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3CoverageReductionMode-07398", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV)),
                         "is VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV but the extendedDynamicState3CoverageReductionMode "
                         "feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3RepresentativeFragmentTestEnable &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3RepresentativeFragmentTestEnable-07399", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV)),
                         "is VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV but the "
                         "extendedDynamicState3RepresentativeFragmentTestEnable feature is not enabled.");
    }

    if (!enabled_features.extendedDynamicState3ShadingRateImageEnable &&
        pipeline.IsDynamic(VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-extendedDynamicState3ShadingRateImageEnable-07400", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV)),
                         "is VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV but the extendedDynamicState3ShadingRateImageEnable "
                         "feature is not enabled.");
    }

    if (!enabled_features.vertexInputDynamicState && pipeline.IsDynamic(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04807", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT)),
                         "is VK_DYNAMIC_STATE_VERTEX_INPUT_EXT but the vertexInputDynamicState feature is not enabled.");
    }

    if (!enabled_features.colorWriteEnable && pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-04800", device,
                         create_info_loc.dot(Field::pDynamicState)
                             .dot(Field::pDynamicStates, get_state_index(VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT)),
                         "is VK_DYNAMIC_STATE_VERTEX_INPUT_EXT but the colorWriteEnable feature is not enabled.");
    }

    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineFragmentShadingRateState(const vvl::Pipeline &pipeline,
                                                                  const Location &create_info_loc) const {
    bool skip = false;
    const auto &pipeline_ci = pipeline.GraphicsCreateInfo();
    const auto *fragment_shading_rate_state =
        vku::FindStructInPNextChain<VkPipelineFragmentShadingRateStateCreateInfoKHR>(pipeline_ci.pNext);
    if (!fragment_shading_rate_state || pipeline.IsDynamic(VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR)) {
        return skip;
    }
    const Location fragment_loc =
        create_info_loc.pNext(Struct::VkPipelineFragmentShadingRateStateCreateInfoKHR, Field::fragmentSize);

    if (fragment_shading_rate_state->fragmentSize.width == 0) {
        skip |=
            LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04494", device, fragment_loc.dot(Field::width), "is zero.");
    }

    if (fragment_shading_rate_state->fragmentSize.height == 0) {
        skip |=
            LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04495", device, fragment_loc.dot(Field::height), "is zero.");
    }

    if (fragment_shading_rate_state->fragmentSize.width != 0 && !IsPowerOfTwo(fragment_shading_rate_state->fragmentSize.width)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04496", device, fragment_loc.dot(Field::width),
                         "is %" PRIu32 ".", fragment_shading_rate_state->fragmentSize.width);
    }

    if (fragment_shading_rate_state->fragmentSize.height != 0 && !IsPowerOfTwo(fragment_shading_rate_state->fragmentSize.height)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04497", device, fragment_loc.dot(Field::height),
                         "is %" PRIu32 ".", fragment_shading_rate_state->fragmentSize.height);
    }

    if (fragment_shading_rate_state->fragmentSize.width > 4) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04498", device, fragment_loc.dot(Field::width),
                         "is %" PRIu32 ".", fragment_shading_rate_state->fragmentSize.width);
    }

    if (fragment_shading_rate_state->fragmentSize.height > 4) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04499", device, fragment_loc.dot(Field::height),
                         "is %" PRIu32 ".", fragment_shading_rate_state->fragmentSize.height);
    }

    if (!enabled_features.pipelineFragmentShadingRate && fragment_shading_rate_state->fragmentSize.width != 1) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04500", device, fragment_loc.dot(Field::width),
                         "is %" PRIu32 ", but the pipelineFragmentShadingRate feature was not enabled.",
                         fragment_shading_rate_state->fragmentSize.width);
    }

    if (!enabled_features.pipelineFragmentShadingRate && fragment_shading_rate_state->fragmentSize.height != 1) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04500", device, fragment_loc.dot(Field::height),
                         "is %" PRIu32 ", but the pipelineFragmentShadingRate feature was not enabled.",
                         fragment_shading_rate_state->fragmentSize.height);
    }

    if (!enabled_features.primitiveFragmentShadingRate &&
        fragment_shading_rate_state->combinerOps[0] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04501", device,
                         create_info_loc.pNext(Struct::VkPipelineFragmentShadingRateStateCreateInfoKHR, Field::combinerOps, 0),
                         "is %s, but the primitiveFragmentShadingRate feature was not enabled.",
                         string_VkFragmentShadingRateCombinerOpKHR(fragment_shading_rate_state->combinerOps[0]));
    }

    if (!enabled_features.attachmentFragmentShadingRate &&
        fragment_shading_rate_state->combinerOps[1] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicState-04502", device,
                         create_info_loc.pNext(Struct::VkPipelineFragmentShadingRateStateCreateInfoKHR, Field::combinerOps, 1),
                         "is %s, but the attachmentFragmentShadingRate feature was not enabled.",
                         string_VkFragmentShadingRateCombinerOpKHR(fragment_shading_rate_state->combinerOps[1]));
    }

    if (!phys_dev_ext_props.fragment_shading_rate_props.fragmentShadingRateNonTrivialCombinerOps &&
        (fragment_shading_rate_state->combinerOps[0] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR &&
         fragment_shading_rate_state->combinerOps[0] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-fragmentShadingRateNonTrivialCombinerOps-04506", device,
                         create_info_loc.pNext(Struct::VkPipelineFragmentShadingRateStateCreateInfoKHR, Field::combinerOps, 0),
                         "is %s, but the fragmentShadingRateNonTrivialCombinerOps feature is not enabled.",
                         string_VkFragmentShadingRateCombinerOpKHR(fragment_shading_rate_state->combinerOps[0]));
    }

    if (!phys_dev_ext_props.fragment_shading_rate_props.fragmentShadingRateNonTrivialCombinerOps &&
        (fragment_shading_rate_state->combinerOps[1] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR &&
         fragment_shading_rate_state->combinerOps[1] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR)) {
        skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-fragmentShadingRateNonTrivialCombinerOps-04506", device,
                         create_info_loc.pNext(Struct::VkPipelineFragmentShadingRateStateCreateInfoKHR, Field::combinerOps, 1),
                         "is %s, but the fragmentShadingRateNonTrivialCombinerOps feature is not enabled.",
                         string_VkFragmentShadingRateCombinerOpKHR(fragment_shading_rate_state->combinerOps[1]));
    }

    auto is_valid_enum_value = [](VkFragmentShadingRateCombinerOpKHR value) {
        switch (value) {
            case VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR:
            case VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR:
            case VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MIN_KHR:
            case VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MAX_KHR:
            case VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MUL_KHR:
                return true;
            default:
                return false;
        };
    };

    const auto combiner_ops = fragment_shading_rate_state->combinerOps;
    if (pipeline.OwnsSubState(pipeline.pre_raster_state) || pipeline.OwnsSubState(pipeline.fragment_shader_state)) {
        if (!is_valid_enum_value(combiner_ops[0])) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicState-06567", device,
                             create_info_loc.pNext(Struct::VkPipelineFragmentShadingRateStateCreateInfoKHR, Field::combinerOps, 0),
                             "(0x%" PRIx32 ") is invalid.", combiner_ops[0]);
        }
        if (!is_valid_enum_value(combiner_ops[1])) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-pDynamicState-06568", device,
                             create_info_loc.pNext(Struct::VkPipelineFragmentShadingRateStateCreateInfoKHR, Field::combinerOps, 1),
                             "(0x%" PRIx32 ") is invalid.", combiner_ops[1]);
        }
    }
    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineDynamicRendering(const vvl::Pipeline &pipeline, const Location &create_info_loc) const {
    bool skip = false;
    const auto &pipeline_ci = pipeline.GraphicsCreateInfo();
    if (pipeline_ci.renderPass != VK_NULL_HANDLE) {
        return skip;
    }

    const auto color_blend_state = pipeline.ColorBlendState();
    const auto *rendering_struct = pipeline.rendering_create_info;
    if (!rendering_struct) {
        // The spec says when thie struct is not included it is same as
        // viewMask = 0, colorAttachmentCount = 0, formats = VK_FORMAT_UNDEFINED.
        // Most VUs are worded around this, but some need to be validated here
        if (pipeline.OwnsSubState(pipeline.fragment_output_state) && color_blend_state && color_blend_state->attachmentCount > 0) {
            skip |= LogError(
                "VUID-VkGraphicsPipelineCreateInfo-renderPass-06055", device,
                create_info_loc.dot(Field::pColorBlendState).dot(Field::attachmentCount),
                "is %" PRIu32
                ", but VkPipelineRenderingCreateInfo::colorAttachmentCount is zero because the pNext chain was not included.",
                color_blend_state->attachmentCount);
        }
        return skip;
    }

    const auto raster_state = pipeline.RasterizationState();
    const bool has_rasterization = raster_state && (raster_state->rasterizerDiscardEnable == VK_FALSE);
    if (has_rasterization) {
        if (pipeline.fragment_shader_state && pipeline.fragment_output_state &&
            ((rendering_struct->depthAttachmentFormat != VK_FORMAT_UNDEFINED) ||
             (rendering_struct->stencilAttachmentFormat != VK_FORMAT_UNDEFINED)) &&
            !pipeline.DepthStencilState() && !pipeline.IsDepthStencilStateDynamic() &&
            !IsExtEnabled(device_extensions.vk_ext_extended_dynamic_state3)) {
            skip |= LogError(
                "VUID-VkGraphicsPipelineCreateInfo-renderPass-09033", device, create_info_loc.dot(Field::pDepthStencilState),
                "is NULL, but %s is %s and stencilAttachmentFormat is %s.",
                create_info_loc.pNext(Struct::VkPipelineRenderingCreateInfo, Field::depthAttachmentFormat).Fields().c_str(),
                string_VkFormat(rendering_struct->depthAttachmentFormat),
                string_VkFormat(rendering_struct->stencilAttachmentFormat));
        }

        if (pipeline.fragment_output_state && (rendering_struct->colorAttachmentCount != 0) && !color_blend_state &&
            !pipeline.IsColorBlendStateDynamic()) {
            for (const auto [i, format] :
                 vvl::enumerate(rendering_struct->pColorAttachmentFormats, rendering_struct->colorAttachmentCount)) {
                if (*format != VK_FORMAT_UNDEFINED) {
                    skip |= LogError(
                        "VUID-VkGraphicsPipelineCreateInfo-renderPass-09037", device, create_info_loc.dot(Field::pColorBlendState),
                        "is NULL, but %s is %" PRIu32 " and pColorAttachmentFormats[%" PRIu32 "] is %s.",
                        create_info_loc.pNext(Struct::VkPipelineRenderingCreateInfo, Field::colorAttachmentCount).Fields().c_str(),
                        rendering_struct->colorAttachmentCount, i, string_VkFormat(*format));
                }
            }
        }
        if (pipeline.fragment_shader_state && pipeline.fragment_output_state) {
            const auto input_attachment_index =
                vku::FindStructInPNextChain<VkRenderingInputAttachmentIndexInfoKHR>(pipeline_ci.pNext);
            if (input_attachment_index) {
                skip |= ValidateRenderingInputAttachmentIndicesKHR(
                    *input_attachment_index, device, create_info_loc.pNext(Struct::VkRenderingInputAttachmentIndexInfoKHR));
                if (input_attachment_index->colorAttachmentCount != rendering_struct->colorAttachmentCount) {
                    const Location loc =
                        create_info_loc.pNext(Struct::VkRenderingInputAttachmentIndexInfoKHR, Field::colorAttachmentCount);
                    skip |=
                        LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-09531", device, loc,
                                 "= %" PRIu32 " does not match VkPipelineRenderingCreateInfo.colorAttachmentCount = %" PRIu32 ".",
                                 input_attachment_index->colorAttachmentCount, rendering_struct->colorAttachmentCount);
                }
            }

            const auto attachment_location = vku::FindStructInPNextChain<VkRenderingAttachmentLocationInfoKHR>(pipeline_ci.pNext);
            if (attachment_location) {
                skip |= ValidateRenderingAttachmentLocationsKHR(
                    *attachment_location, device, create_info_loc.pNext(Struct::VkRenderingAttachmentLocationInfoKHR));
                if (attachment_location->colorAttachmentCount != rendering_struct->colorAttachmentCount) {
                    const Location loc =
                        create_info_loc.pNext(Struct::VkRenderingAttachmentLocationInfoKHR, Field::colorAttachmentCount);
                    skip |=
                        LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-09532", device, loc,
                                 "= %" PRIu32 " does not match VkPipelineRenderingCreateInfo.colorAttachmentCount = %" PRIu32 ".",
                                 attachment_location->colorAttachmentCount, rendering_struct->colorAttachmentCount);
                }
            }
        }
    }

    if (rendering_struct->viewMask != 0 && raster_state) {
        skip |= ValidateMultiViewShaders(pipeline, create_info_loc.pNext(Struct::VkPipelineRenderingCreateInfo, Field::viewMask),
                                         rendering_struct->viewMask, true);
    }

    if (pipeline.OwnsSubState(pipeline.fragment_output_state)) {
        for (uint32_t color_index = 0; color_index < rendering_struct->colorAttachmentCount; color_index++) {
            const VkFormat color_format = rendering_struct->pColorAttachmentFormats[color_index];
            if (color_format != VK_FORMAT_UNDEFINED) {
                VkFormatFeatureFlags2KHR format_features = GetPotentialFormatFeatures(color_format);
                if (((format_features & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT) == 0) &&
                    (color_blend_state && (color_index < color_blend_state->attachmentCount) &&
                     (color_blend_state->pAttachments[color_index].blendEnable != VK_FALSE))) {
                    skip |= LogError(
                        "VUID-VkGraphicsPipelineCreateInfo-renderPass-06062", device,
                        create_info_loc.dot(Field::pColorBlendState).dot(Field::pAttachments, color_index).dot(Field::blendEnable),
                        "is VK_TRUE.");
                }

                if ((format_features &
                     (VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_2_LINEAR_COLOR_ATTACHMENT_BIT_NV)) == 0) {
                    skip |= LogError(
                        "VUID-VkGraphicsPipelineCreateInfo-renderPass-06582", device,
                        create_info_loc.pNext(Struct::VkPipelineRenderingCreateInfo, Field::pColorAttachmentFormats, color_index),
                        "(%s) potential format features are %s.", string_VkFormat(color_format),
                        string_VkFormatFeatureFlags2(format_features).c_str());
                }
            }
        }

        if (rendering_struct->depthAttachmentFormat != VK_FORMAT_UNDEFINED) {
            VkFormatFeatureFlags2 format_features = GetPotentialFormatFeatures(rendering_struct->depthAttachmentFormat);
            if ((format_features & VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT) == 0) {
                skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06585", device,
                                 create_info_loc.pNext(Struct::VkPipelineRenderingCreateInfo, Field::depthAttachmentFormat),
                                 "(%s) potential format features are %s.", string_VkFormat(rendering_struct->depthAttachmentFormat),
                                 string_VkFormatFeatureFlags2(format_features).c_str());
            }
        }

        if (rendering_struct->stencilAttachmentFormat != VK_FORMAT_UNDEFINED) {
            VkFormatFeatureFlags2 format_features = GetPotentialFormatFeatures(rendering_struct->stencilAttachmentFormat);
            if ((format_features & VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT) == 0) {
                skip |=
                    LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06586", device,
                             create_info_loc.pNext(Struct::VkPipelineRenderingCreateInfo, Field::stencilAttachmentFormat),
                             "(%s) potential format features are %s.", string_VkFormat(rendering_struct->stencilAttachmentFormat),
                             string_VkFormatFeatureFlags2(format_features).c_str());
            }
        }

        if ((rendering_struct->depthAttachmentFormat != VK_FORMAT_UNDEFINED) &&
            (rendering_struct->stencilAttachmentFormat != VK_FORMAT_UNDEFINED) &&
            (rendering_struct->depthAttachmentFormat != rendering_struct->stencilAttachmentFormat)) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06589", device,
                             create_info_loc.pNext(Struct::VkPipelineRenderingCreateInfo, Field::depthAttachmentFormat),
                             "(%s) is not equal to stencilAttachmentFormat (%s).",
                             string_VkFormat(rendering_struct->depthAttachmentFormat),
                             string_VkFormat(rendering_struct->stencilAttachmentFormat));
        }

        if (color_blend_state && rendering_struct->colorAttachmentCount != color_blend_state->attachmentCount) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06055", device,
                             create_info_loc.pNext(Struct::VkPipelineRenderingCreateInfo, Field::colorAttachmentCount),
                             "(%" PRIu32 ") is different from %s (%" PRIu32 ").", rendering_struct->colorAttachmentCount,
                             create_info_loc.dot(Field::pColorBlendState).dot(Field::attachmentCount).Fields().c_str(),
                             color_blend_state->attachmentCount);
        }
    }

    if (pipeline.IsRenderPassStateRequired()) {
        if ((enabled_features.multiview == VK_FALSE) && (rendering_struct->viewMask != 0)) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-multiview-06577", device,
                             create_info_loc.pNext(Struct::VkPipelineRenderingCreateInfo, Field::viewMask),
                             "is %" PRIu32 ", but the multiview feature was not enabled.", rendering_struct->viewMask);
        }

        if (MostSignificantBit(rendering_struct->viewMask) >= static_cast<int32_t>(phys_dev_props_core11.maxMultiviewViewCount)) {
            skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderPass-06578", device,
                             create_info_loc.pNext(Struct::VkPipelineRenderingCreateInfo, Field::viewMask),
                             "is 0x%" PRIx32 ", but maxMultiviewViewCount is %" PRIu32 "", rendering_struct->viewMask,
                             phys_dev_props_core11.maxMultiviewViewCount);
        }
    }

    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineBindPoint(const vvl::CommandBuffer &cb_state, const vvl::Pipeline &pipeline,
                                                   const Location &loc) const {
    bool skip = false;

    if (cb_state.inheritedViewportDepths.empty()) {
        return skip;
    }

    bool dyn_viewport = pipeline.IsDynamic(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT) || pipeline.IsDynamic(VK_DYNAMIC_STATE_VIEWPORT);
    bool dyn_scissor = pipeline.IsDynamic(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT) || pipeline.IsDynamic(VK_DYNAMIC_STATE_SCISSOR);
    if (!dyn_viewport || !dyn_scissor) {
        const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
        skip |= LogError("VUID-vkCmdBindPipeline-commandBuffer-04808", objlist, loc,
                         "Graphics pipeline incompatible with viewport/scissor inheritance.");
    }
    const auto &pipeline_ci = pipeline.GraphicsCreateInfo();
    const auto *discard_rectangle_state =
        vku::FindStructInPNextChain<VkPipelineDiscardRectangleStateCreateInfoEXT>(pipeline_ci.pNext);
    if ((discard_rectangle_state && discard_rectangle_state->discardRectangleCount != 0) ||
        (pipeline.IsDynamic(VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT))) {
        if (!pipeline.IsDynamic(VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT)) {
            std::stringstream msg;
            if (discard_rectangle_state) {
                msg << "VkPipelineDiscardRectangleStateCreateInfoEXT::discardRectangleCount = "
                    << discard_rectangle_state->discardRectangleCount;
            } else {
                msg << "VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT";
            }
            const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
            skip |= LogError(
                "VUID-vkCmdBindPipeline-commandBuffer-04809", objlist, loc.dot(Field::commandBuffer),
                "is a secondary command buffer with VkCommandBufferInheritanceViewportScissorInfoNV::viewportScissor2D "
                "enabled, pipelineBindPoint is VK_PIPELINE_BIND_POINT_GRAPHICS and pipeline was created with %s, but without "
                "VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT.",
                msg.str().c_str());
        }
    }

    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineShaderDynamicState(const vvl::Pipeline &pipeline, const vvl::CommandBuffer &cb_state,
                                                            const Location &loc, const vvl::DrawDispatchVuid &vuid) const {
    bool skip = false;

    for (auto &stage_state : pipeline.stage_states) {
        const VkShaderStageFlagBits stage = stage_state.GetStage();
        if (!IsValueIn(stage, {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_GEOMETRY_BIT, VK_SHADER_STAGE_MESH_BIT_EXT})) {
            continue;
        }
        if (!phys_dev_ext_props.fragment_shading_rate_props.primitiveFragmentShadingRateWithMultipleViewports &&
            pipeline.IsDynamic(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT) && cb_state.dynamic_state_value.viewport_count != 1) {
            if (stage_state.entrypoint && stage_state.entrypoint->written_builtin_primitive_shading_rate_khr) {
                skip |= LogError(vuid.viewport_count_primitive_shading_rate_04552, stage_state.module_state->Handle(), loc,
                                 "%s shader of currently bound pipeline statically writes to PrimitiveShadingRateKHR built-in"
                                 "but multiple viewports are set by the last call to vkCmdSetViewportWithCountEXT,"
                                 "and the primitiveFragmentShadingRateWithMultipleViewports limit is not supported.",
                                 string_VkShaderStageFlagBits(stage));
            }
        }
    }

    return skip;
}

// Validate draw-time state related to the PSO
bool CoreChecks::ValidatePipelineDrawtimeState(const LastBound &last_bound_state, const Location &loc) const {
    bool skip = false;
    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;
    const vvl::Pipeline *pipeline = last_bound_state.pipeline_state;
    const vvl::DrawDispatchVuid &vuid = vvl::GetDrawDispatchVuid(loc.function);

    if (cb_state.activeRenderPass->UsesDynamicRendering()) {
        if (pipeline) {
            skip |= ValidatePipelineDynamicRenderpassDraw(last_bound_state, loc);
        }
    } else {
        if (pipeline) {
            skip |= ValidatePipelineRenderpassDraw(last_bound_state, loc);
        } else if (last_bound_state.HasShaderObjects()) {
            skip |= LogError(vuid.render_pass_began_08876, cb_state.Handle(), loc,
                             "Shader objects must be used with dynamic rendering, but VkRenderPass %s is active.",
                             FormatHandle(cb_state.activeRenderPass->Handle()).c_str());
        }
    }

    bool primitives_generated_query_with_rasterizer_discard =
        enabled_features.primitivesGeneratedQueryWithRasterizerDiscard == VK_TRUE;
    bool primitives_generated_query_with_non_zero_streams = enabled_features.primitivesGeneratedQueryWithNonZeroStreams == VK_TRUE;
    if (!primitives_generated_query_with_rasterizer_discard || !primitives_generated_query_with_non_zero_streams) {
        bool primitives_generated_query = false;
        for (const auto &query : cb_state.activeQueries) {
            auto query_pool_state = Get<vvl::QueryPool>(query.pool);
            if (query_pool_state && query_pool_state->create_info.queryType == VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT) {
                primitives_generated_query = true;
                break;
            }
        }
        if (primitives_generated_query) {
            bool rasterizer_discard_enabled = false;
            if (pipeline && !pipeline->IsDynamic(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE)) {
                rasterizer_discard_enabled = pipeline->RasterizationState()->rasterizerDiscardEnable;
            } else {
                rasterizer_discard_enabled = cb_state.dynamic_state_value.rasterizer_discard_enable;
            }
            if (!primitives_generated_query_with_rasterizer_discard && rasterizer_discard_enabled) {
                LogObjectList objlist(cb_state.Handle());
                if (pipeline) {
                    objlist.add(pipeline->Handle());
                }
                skip |= LogError(vuid.primitives_generated_06708, objlist, loc,
                                 "a VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT query is active and pipeline was created with "
                                 "VkPipelineRasterizationStateCreateInfo::rasterizerDiscardEnable set to VK_TRUE, but  "
                                 "primitivesGeneratedQueryWithRasterizerDiscard feature is not enabled.");
            }
            if (!primitives_generated_query_with_non_zero_streams && pipeline) {
                const auto rasterization_state_stream_ci =
                    vku::FindStructInPNextChain<VkPipelineRasterizationStateStreamCreateInfoEXT>(pipeline->RasterizationState()->pNext);
                if (rasterization_state_stream_ci && rasterization_state_stream_ci->rasterizationStream != 0) {
                    LogObjectList objlist(cb_state.Handle());
                    if (pipeline) {
                        objlist.add(pipeline->Handle());
                    }
                    skip |= LogError(vuid.primitives_generated_streams_06709, objlist, loc,
                                     "a VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT query is active and pipeline was created with "
                                     "VkPipelineRasterizationStateStreamCreateInfoEXT::rasterizationStream set to %" PRIu32
                                     ", but  "
                                     "primitivesGeneratedQueryWithNonZeroStreams feature is not enabled.",
                                     rasterization_state_stream_ci->rasterizationStream);
                }
            }
        }
    }

    // Verify vertex & index buffer for unprotected command buffer.
    // Because vertex & index buffer is read only, it doesn't need to care protected command buffer case.
    if (enabled_features.protectedMemory == VK_TRUE) {
        for (const auto &vertex_buffer_binding : cb_state.current_vertex_buffer_binding_info) {
            const auto buffer_state = Get<vvl::Buffer>(vertex_buffer_binding.second.buffer);
            if (buffer_state) {
                skip |= ValidateProtectedBuffer(cb_state, *buffer_state, loc, vuid.unprotected_command_buffer_02707,
                                                "Buffer is vertex buffer");
            }
        }
        const auto buffer_state = Get<vvl::Buffer>(cb_state.index_buffer_binding.buffer);
        if (buffer_state) {
            skip |= ValidateProtectedBuffer(cb_state, *buffer_state, loc, vuid.unprotected_command_buffer_02707,
                                            "Buffer is index buffer");
        }
    }

    // Verify vertex binding
    if (pipeline && pipeline->vertex_input_state) {
        for (const auto &vertex_buffer_binding : cb_state.current_vertex_buffer_binding_info) {
            if (vertex_buffer_binding.second.buffer == VK_NULL_HANDLE) {
                if (!enabled_features.nullDescriptor) {
                    const LogObjectList objlist(cb_state.Handle(), pipeline->Handle());
                    skip |= LogError(vuid.vertex_binding_null_04008, objlist, loc,
                                     "Vertex binding %" PRIu32
                                     " is VK_NULL_HANDLE. (Most likely you forgot to call vkCmdBindVertexBuffers)",
                                     vertex_buffer_binding.first);
                }
            } else if (!Get<vvl::Buffer>(vertex_buffer_binding.second.buffer)) {
                const LogObjectList objlist(cb_state.Handle(), pipeline->Handle());
                skip |=
                    LogError(vuid.vertex_binding_04007, objlist, loc,
                             "Vertex binding %" PRIu32 " is not a valid VkBuffer. (Check the buffer set in vkCmdBindVertexBuffers)",
                             vertex_buffer_binding.first);
            }
        }

        const auto &vertex_attribute_descriptions = pipeline->IsDynamic(VK_DYNAMIC_STATE_VERTEX_INPUT_EXT)
                                                        ? cb_state.dynamic_state_value.vertex_attribute_descriptions
                                                        : pipeline->vertex_input_state->vertex_attribute_descriptions;
        // Verify vertex attribute address alignment
        for (uint32_t i = 0; i < vertex_attribute_descriptions.size(); i++) {
            const auto &attribute_description = vertex_attribute_descriptions[i];
            const uint32_t vertex_binding = attribute_description.binding;

            const auto &vertex_binding_map_it = cb_state.current_vertex_buffer_binding_info.find(vertex_binding);
            if (vertex_binding_map_it == cb_state.current_vertex_buffer_binding_info.cend()) {
                const LogObjectList objlist(cb_state.Handle(), pipeline->Handle());
                skip |= LogError(vuid.vertex_binding_attribute_02721, objlist, loc,
                                 "pVertexAttributeDescriptions[%" PRIu32 "].binding (%" PRIu32 ") is an invalid value.",
                                 vertex_binding, i);
                break;
            } else if (vertex_binding_map_it->second.buffer == VK_NULL_HANDLE && !enabled_features.nullDescriptor) {
                const LogObjectList objlist(cb_state.Handle(), pipeline->Handle());
                skip |=
                    LogError(vuid.vertex_binding_attribute_02721, objlist, loc,
                             "pVertexAttributeDescriptions[%" PRIu32 "].binding (%" PRIu32 ") points to a VK_NULL_HANDLE buffer.",
                             vertex_binding, i);
                break;
            }
            auto const buffer_state = Get<vvl::Buffer>(vertex_binding_map_it->second.buffer);
            if (!buffer_state) {
                const LogObjectList objlist(cb_state.Handle(), pipeline->Handle());
                skip |= LogError(vuid.vertex_binding_attribute_02721, objlist, loc,
                                 "pVertexAttributeDescriptions[%" PRIu32 "].binding (%" PRIu32 ") points to an invalid buffer.",
                                 vertex_binding, i);
                break;
            }

            const VkDeviceSize attribute_offset = attribute_description.offset;
            const VkFormat attribute_format = attribute_description.format;
            const VkDeviceSize vertex_buffer_stride = vertex_binding_map_it->second.stride;
            if (pipeline->IsDynamic(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT)) {
                const VkDeviceSize attribute_binding_extent = attribute_offset + vkuFormatElementSize(attribute_format);
                if (vertex_buffer_stride != 0 && vertex_buffer_stride < attribute_binding_extent) {
                    const LogObjectList objlist(cb_state.Handle(), pipeline->Handle());
                    skip |= LogError("VUID-vkCmdBindVertexBuffers2-pStrides-06209", objlist, loc,
                                     "The pStrides[%" PRIu32 "] (%" PRIu64
                                     ") parameter in the last call to %s is not 0 "
                                     "and less than the extent of the binding for attribute %" PRIu32 " (%" PRIu64 ").",
                                     vertex_binding, vertex_buffer_stride, loc.StringFunc(), i, attribute_binding_extent);
                }
            }
            const VkDeviceSize vertex_buffer_offset = vertex_binding_map_it->second.offset;

            // Use 1 as vertex/instance index to use buffer stride as well
            const VkDeviceSize attrib_address = vertex_buffer_offset + vertex_buffer_stride + attribute_offset;

            VkDeviceSize vtx_attrib_req_alignment = vkuFormatElementSize(attribute_format);
            if (vkuFormatElementIsTexel(attribute_format)) {
                vtx_attrib_req_alignment = SafeDivision(vtx_attrib_req_alignment, vkuFormatComponentCount(attribute_format));
            }

            if (SafeModulo(attrib_address, vtx_attrib_req_alignment) != 0) {
                const LogObjectList objlist(buffer_state->Handle(), pipeline->Handle());
                skip |= LogError(vuid.vertex_binding_attribute_02721, objlist, loc,
                                 "Format %s has an alignment of %" PRIu64 " but the alignment of attribAddress (%" PRIu64
                                 ") is not aligned in pVertexAttributeDescriptions[%" PRIu32
                                 "]"
                                 "(binding=%" PRIu32 " location=%" PRIu32 ") where attribAddress = vertex buffer offset (%" PRIu64
                                 ") + binding stride (%" PRIu64 ") + attribute offset (%" PRIu64 ").",
                                 string_VkFormat(attribute_format), vtx_attrib_req_alignment, attrib_address, i, vertex_binding,
                                 attribute_description.location, vertex_buffer_offset, vertex_buffer_stride, attribute_offset);
            }
        }
    }

    // Verify that any MSAA request in PSO matches sample# in bound FB
    // Verify that blend is enabled only if supported by subpasses image views format features
    // Skip the check if rasterization is disabled.
    if (pipeline) {
        const auto *raster_state = pipeline->RasterizationState();
        if (!raster_state || (raster_state->rasterizerDiscardEnable == VK_FALSE)) {
            if (cb_state.activeRenderPass->UsesDynamicRendering()) {
                // TODO: Mirror the below VUs but using dynamic rendering
                const auto dynamic_rendering_info = cb_state.activeRenderPass->dynamic_rendering_begin_rendering_info;
            } else {
                const auto render_pass_info = cb_state.activeRenderPass->create_info.ptr();
                const VkSubpassDescription2 *subpass_desc = &render_pass_info->pSubpasses[cb_state.GetActiveSubpass()];
                uint32_t i;
                unsigned subpass_num_samples = 0;

                for (i = 0; i < subpass_desc->colorAttachmentCount; i++) {
                    const auto attachment = subpass_desc->pColorAttachments[i].attachment;
                    if (attachment != VK_ATTACHMENT_UNUSED) {
                        subpass_num_samples |= static_cast<unsigned>(render_pass_info->pAttachments[attachment].samples);

                        const auto *imageview_state = cb_state.GetActiveAttachmentImageViewState(attachment);
                        const auto *color_blend_state = pipeline->ColorBlendState();
                        if (imageview_state && color_blend_state && (attachment < color_blend_state->attachmentCount)) {
                            if ((imageview_state->format_features & VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BLEND_BIT_KHR) == 0 &&
                                color_blend_state->pAttachments[i].blendEnable != VK_FALSE) {
                                const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(),
                                                            cb_state.activeRenderPass->Handle());
                                skip |=
                                    LogError(vuid.blend_enable_04727, objlist, loc,
                                             "Image view's format features of the color attachment (%" PRIu32
                                             ") of the active subpass do not contain VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT "
                                             "bit, but active pipeline's pAttachments[%" PRIu32 "].blendEnable is not VK_FALSE.",
                                             attachment, attachment);
                            }
                        }
                    }
                }

                if (subpass_desc->pDepthStencilAttachment &&
                    subpass_desc->pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
                    const auto attachment = subpass_desc->pDepthStencilAttachment->attachment;
                    subpass_num_samples |= static_cast<unsigned>(render_pass_info->pAttachments[attachment].samples);
                }

                const VkSampleCountFlagBits rasterization_samples = last_bound_state.GetRasterizationSamples();
                if (!(IsExtEnabled(device_extensions.vk_amd_mixed_attachment_samples) ||
                      IsExtEnabled(device_extensions.vk_nv_framebuffer_mixed_samples) ||
                      enabled_features.multisampledRenderToSingleSampled) &&
                    ((subpass_num_samples & static_cast<unsigned>(rasterization_samples)) != subpass_num_samples)) {
                    const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                    skip |= LogError(vuid.msrtss_rasterization_samples_07284, objlist, loc,
                                     "In %s the sample count is %s while the current %s has %s and they need to be the same.",
                                     FormatHandle(*pipeline).c_str(), string_VkSampleCountFlagBits(rasterization_samples),
                                     FormatHandle(*cb_state.activeRenderPass).c_str(),
                                     string_VkSampleCountFlags(static_cast<VkSampleCountFlags>(subpass_num_samples)).c_str());
                }

                const bool dynamic_line_raster_mode = pipeline->IsDynamic(VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT);
                const bool dynamic_line_stipple_enable = pipeline->IsDynamic(VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT);
                if (dynamic_line_stipple_enable || dynamic_line_raster_mode) {
                    const auto raster_line_state =
                        vku::FindStructInPNextChain<VkPipelineRasterizationLineStateCreateInfoKHR>(raster_state->pNext);

                    const VkLineRasterizationModeKHR line_rasterization_mode =
                        (dynamic_line_raster_mode) ? cb_state.dynamic_state_value.line_rasterization_mode
                                                   : raster_line_state->lineRasterizationMode;
                    const bool stippled_line_enable = (dynamic_line_stipple_enable)
                                                          ? cb_state.dynamic_state_value.stippled_line_enable
                                                          : raster_line_state->stippledLineEnable;

                    if (stippled_line_enable) {
                        if (line_rasterization_mode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_KHR &&
                            (!enabled_features.stippledRectangularLines)) {
                            const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                            skip |= LogError(vuid.stippled_rectangular_lines_07495, objlist, loc,
                                             "lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_RECTANGULAR_KHR (set %s) with "
                                             "stippledLineEnable (set %s) but the stippledRectangularLines feature is not enabled.",
                                             dynamic_line_raster_mode ? "dynamically" : "in pipeline",
                                             dynamic_line_stipple_enable ? "dynamically" : "in pipeline");
                        }
                        if (line_rasterization_mode == VK_LINE_RASTERIZATION_MODE_BRESENHAM_KHR &&
                            (!enabled_features.stippledBresenhamLines)) {
                            const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                            skip |= LogError(vuid.stippled_bresenham_lines_07496, objlist, loc,
                                             "lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_BRESENHAM_KHR (set %s) with "
                                             "stippledLineEnable (set %s) but the stippledBresenhamLines feature is not enabled.",
                                             dynamic_line_raster_mode ? "dynamically" : "in pipeline",
                                             dynamic_line_stipple_enable ? "dynamically" : "in pipeline");
                        }
                        if (line_rasterization_mode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_KHR &&
                            (!enabled_features.stippledSmoothLines)) {
                            const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                            skip |=
                                LogError(vuid.stippled_smooth_lines_07497, objlist, loc,
                                         "lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_KHR (set %s) with "
                                         "stippledLineEnable (set %s) but the stippledSmoothLines feature is not enabled.",
                                         dynamic_line_raster_mode ? "dynamically" : "in pipeline",
                                         dynamic_line_stipple_enable ? "dynamically" : "in pipeline");
                        }
                        if (line_rasterization_mode == VK_LINE_RASTERIZATION_MODE_DEFAULT_KHR &&
                            (!enabled_features.stippledRectangularLines || !phys_dev_props.limits.strictLines)) {
                            const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                            skip |= LogError(
                                vuid.stippled_default_strict_07498, objlist, loc,
                                "lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_DEFAULT_KHR (set %s) with "
                                "stippledLineEnable (set %s), the stippledRectangularLines features is %s and strictLines is %s.",
                                dynamic_line_raster_mode ? "dynamically" : "in pipeline",
                                dynamic_line_stipple_enable ? "dynamically" : "in pipeline",
                                enabled_features.stippledRectangularLines ? "enabled" : "not enabled",
                                phys_dev_props.limits.strictLines ? "VK_TRUE" : "VK_FALSE");
                        }
                    }
                }
            }
        }

        if (enabled_features.primitiveFragmentShadingRate) {
            skip |= ValidateGraphicsPipelineShaderDynamicState(*pipeline, cb_state, loc, vuid);
        }
    }

    if (pipeline && pipeline->IsDynamic(VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT) &&
        cb_state.dynamic_state_value.alpha_to_coverage_enable) {
        if (pipeline->fragment_shader_state && pipeline->fragment_shader_state->fragment_entry_point) {
            // TODO - DualSource blend has two outputs at location zero, so Index == 0 is the one that's required.
            // Currently lack support to test each index.
            if (!pipeline->fragment_shader_state->fragment_entry_point->has_alpha_to_coverage_variable &&
                !pipeline->DualSourceBlending()) {
                const LogObjectList objlist(cb_state.Handle(), pipeline->Handle());
                skip |= LogError(vuid.dynamic_alpha_to_coverage_component_08919, objlist, loc,
                                 "vkCmdSetAlphaToCoverageEnableEXT set alphaToCoverageEnable to true but the bound pipeline "
                                 "fragment shader doesn't declare a variable that covers Location 0, Component 3.");
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateShaderObjectDrawtimeState(const LastBound &last_bound_state, const Location &loc) const {
    bool skip = false;
    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;
    const LogObjectList objlist(cb_state.Handle());
    const vvl::DrawDispatchVuid &vuid = vvl::GetDrawDispatchVuid(loc.function);

    if (!last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::VERTEX)) {
        skip |= LogError(vuid.vertex_shader_08684, objlist, loc,
                         "There is no graphics pipeline bound and vkCmdBindShadersEXT() was not called with stage "
                         "VK_SHADER_STAGE_VERTEX_BIT and either VK_NULL_HANDLE or a valid VK_SHADER_STAGE_VERTEX_BIT shader.");
    }
    if (enabled_features.tessellationShader &&
        !last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::TESSELLATION_CONTROL)) {
        skip |= LogError(vuid.tessellation_control_shader_08685, objlist, loc,
                         "There is no graphics pipeline bound and vkCmdBindShadersEXT() was not called with stage "
                         "VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT and either VK_NULL_HANDLE or a valid "
                         "VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT shader.");
    }
    if (enabled_features.tessellationShader &&
        !last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::TESSELLATION_EVALUATION)) {
        skip |= LogError(vuid.tessellation_evaluation_shader_08686, objlist, loc,
                         "There is no graphics pipeline bound and vkCmdBindShadersEXT() was not called with stage "
                         "VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT and either VK_NULL_HANDLE or a valid "
                         "VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT shader.");
    }
    if (enabled_features.geometryShader && !last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::GEOMETRY)) {
        skip |= LogError(vuid.geometry_shader_08687, objlist, loc,
                         "There is no graphics pipeline bound and vkCmdBindShadersEXT() was not called with stage "
                         "VK_SHADER_STAGE_GEOMETRY_BIT and either VK_NULL_HANDLE or a valid VK_SHADER_STAGE_GEOMETRY_BIT shader.");
    }
    if (!last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::FRAGMENT)) {
        skip |= LogError(vuid.fragment_shader_08688, objlist, loc,
                         "There is no graphics pipeline bound and vkCmdBindShadersEXT() was not called with stage "
                         "VK_SHADER_STAGE_FRAGMENT_BIT and either VK_NULL_HANDLE or a valid VK_SHADER_STAGE_FRAGMENT_BIT shader.");
    }
    if (enabled_features.taskShader && !last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::TASK)) {
        skip |= LogError(vuid.task_shader_08689, objlist, loc,
                         "There is no graphics pipeline bound and vkCmdBindShadersEXT() was not called with stage "
                         "VK_SHADER_STAGE_TASK_BIT and either VK_NULL_HANDLE or a valid VK_SHADER_STAGE_TASK_BIT shader.");
    }
    if (enabled_features.meshShader && !last_bound_state.IsValidShaderOrNullBound(ShaderObjectStage::MESH)) {
        skip |= LogError(vuid.mesh_shader_08690, objlist, loc,
                         "There is no graphics pipeline bound and vkCmdBindShadersEXT() was not called with stage "
                         "VK_SHADER_STAGE_MESH_BIT and either VK_NULL_HANDLE or a valid VK_SHADER_STAGE_MESH_BIT shader.");
    }

    bool validVertShader = last_bound_state.GetShader(ShaderObjectStage::VERTEX);
    bool validTaskShader = last_bound_state.GetShader(ShaderObjectStage::TASK);
    bool validMeshShader = last_bound_state.GetShader(ShaderObjectStage::MESH);

    if (enabled_features.taskShader || enabled_features.meshShader) {
        if ((validVertShader && validMeshShader) || (!validVertShader && !validMeshShader)) {
            const std::string msg = validVertShader ? "Both vertex shader and mesh shader are bound"
                                                    : "Neither vertex shader nor mesh shader are bound";
            skip |= LogError(vuid.vert_mesh_shader_08693, objlist, loc, "%s.", msg.c_str());
        }
    }
    if (enabled_features.taskShader && enabled_features.meshShader) {
        if (validMeshShader &&
            (last_bound_state.GetShaderState(ShaderObjectStage::MESH)->create_info.flags &
             VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT) == 0 &&
            !validTaskShader) {
            skip |=
                LogError(vuid.task_mesh_shader_08694, objlist, loc,
                         "Mesh shader %s was created without VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT, but no task shader is bound.",
                         debug_report->FormatHandle(last_bound_state.GetShader(ShaderObjectStage::MESH)).c_str());
        } else if (validMeshShader &&
                   (last_bound_state.GetShaderState(ShaderObjectStage::MESH)->create_info.flags &
                    VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT) != 0 &&
                   validTaskShader) {
            skip |= LogError(vuid.task_mesh_shader_08695, objlist, loc,
                             "Mesh shader %s was created with VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT, but a task shader is bound.",
                             debug_report->FormatHandle(last_bound_state.GetShader(ShaderObjectStage::MESH)).c_str());
        }
    }
    if (validVertShader && (validTaskShader || validMeshShader)) {
        std::stringstream msg;
        if (validTaskShader && validMeshShader) {
            msg << "task shader " << debug_report->FormatHandle(last_bound_state.GetShader(ShaderObjectStage::TASK))
                << "and mesh shader " << debug_report->FormatHandle(last_bound_state.GetShader(ShaderObjectStage::MESH))
                << " are bound as well";
        } else if (validTaskShader) {
            msg << "task shader " << debug_report->FormatHandle(last_bound_state.GetShader(ShaderObjectStage::TASK))
                << " is bound as well";
        } else if (validMeshShader) {
            msg << "mesh shader " << debug_report->FormatHandle(last_bound_state.GetShader(ShaderObjectStage::MESH))
                << " is bound as well";
        }
        skip |=
            LogError(vuid.vert_task_mesh_shader_08696, objlist, loc, "Vertex shader %s is bound, but %s.",
                     debug_report->FormatHandle(last_bound_state.GetShader(ShaderObjectStage::MESH)).c_str(), msg.str().c_str());
    }
    for (uint32_t i = 0; i < kShaderObjectStageCount; ++i) {
        if (i != static_cast<uint32_t>(ShaderObjectStage::COMPUTE) && last_bound_state.shader_object_states[i]) {
            for (const auto &linkedShader : last_bound_state.shader_object_states[i]->linked_shaders) {
                bool found = false;
                for (uint32_t j = 0; j < kShaderObjectStageCount; ++j) {
                    if (linkedShader == last_bound_state.GetShader(static_cast<ShaderObjectStage>(j))) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    const auto missingShader = Get<vvl::ShaderObject>(linkedShader);
                    skip |=
                        LogError(vuid.linked_shaders_08698, objlist, loc,
                                 "Shader %s (%s) was created with VK_SHADER_CREATE_LINK_STAGE_BIT_EXT, but the linked %s "
                                 "shader (%s) is not bound.",
                                 debug_report->FormatHandle(last_bound_state.GetShader(static_cast<ShaderObjectStage>(i))).c_str(),
                                 string_VkShaderStageFlagBits(last_bound_state.shader_object_states[i]->create_info.stage),
                                 debug_report->FormatHandle(linkedShader).c_str(),
                                 string_VkShaderStageFlagBits(missingShader->create_info.stage));
                    break;
                }
            }
        }
    }
    const VkShaderStageFlagBits graphics_stages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                                     VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, VK_SHADER_STAGE_GEOMETRY_BIT,
                                                     VK_SHADER_STAGE_FRAGMENT_BIT};
    VkShaderStageFlagBits prev_stage = VK_SHADER_STAGE_ALL;
    VkShaderStageFlagBits next_stage = VK_SHADER_STAGE_ALL;
    for (const auto stage : graphics_stages) {
        const auto state = last_bound_state.GetShaderState(VkShaderStageToShaderObjectStage(stage));
        if (state && next_stage != VK_SHADER_STAGE_ALL && state->create_info.stage != next_stage) {
            skip |= LogError(vuid.linked_shaders_08699, objlist, loc,
                             "Shaders %s and %s were created with VK_SHADER_CREATE_LINK_STAGE_BIT_EXT without intermediate "
                             "stage %s linked, but %s shader is bound.",
                             string_VkShaderStageFlagBits(prev_stage), string_VkShaderStageFlagBits(next_stage),
                             string_VkShaderStageFlagBits(stage), string_VkShaderStageFlagBits(stage));
            break;
        }
        if (state) {
            next_stage = VK_SHADER_STAGE_ALL;
            if (!state->linked_shaders.empty()) {
                prev_stage = stage;
                for (const auto &linked_shader : state->linked_shaders) {
                    const auto &linked_state = Get<vvl::ShaderObject>(linked_shader);
                    if (linked_state->create_info.stage == state->create_info.nextStage) {
                        next_stage = static_cast<VkShaderStageFlagBits>(state->create_info.nextStage);
                        break;
                    }
                }
            }
        }
    }

    const vvl::ShaderObject *first = nullptr;
    for (const auto shader_state : last_bound_state.shader_object_states) {
        if (!shader_state || !shader_state->IsGraphicsShaderState()) {
            continue;
        }
        if (!first) {
            first = shader_state;
        } else {
            bool pushConstsDifferent =
                first->create_info.pushConstantRangeCount != shader_state->create_info.pushConstantRangeCount;
            if (!pushConstsDifferent) {
                bool found = false;
                for (uint32_t i = 0; i < shader_state->create_info.pushConstantRangeCount; ++i) {
                    for (uint32_t j = 0; j < first->create_info.pushConstantRangeCount; ++j) {
                        if (shader_state->create_info.pPushConstantRanges[i] == first->create_info.pPushConstantRanges[j]) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        pushConstsDifferent = true;
                        break;
                    }
                }
            }
            if (pushConstsDifferent) {
                skip |= LogError(vuid.shaders_push_constants_08878, objlist, loc,
                                 "Shaders %s and %s have different push constant ranges.",
                                 string_VkShaderStageFlagBits(first->create_info.stage),
                                 string_VkShaderStageFlagBits(shader_state->create_info.stage));
            }
            bool descriptorLayoutsDifferent = first->create_info.setLayoutCount != shader_state->create_info.setLayoutCount;
            if (!descriptorLayoutsDifferent) {
                bool found = false;
                for (uint32_t i = 0; i < shader_state->create_info.setLayoutCount; ++i) {
                    for (uint32_t j = 0; j < first->create_info.setLayoutCount; ++j) {
                        if (shader_state->create_info.pSetLayouts[i] == first->create_info.pSetLayouts[j]) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        descriptorLayoutsDifferent = true;
                        break;
                    }
                }
            }
            if (descriptorLayoutsDifferent) {
                skip |= LogError(vuid.shaders_descriptor_layouts_08879, objlist, loc,
                                 "Shaders %s and %s have different descriptor set layouts.",
                                 string_VkShaderStageFlagBits(first->create_info.stage),
                                 string_VkShaderStageFlagBits(shader_state->create_info.stage));
            }
        }
    }

    if (loc.function != Func::vkCmdDrawMeshTasksNV && loc.function != Func::vkCmdDrawMeshTasksIndirectNV &&
        loc.function != Func::vkCmdDrawMeshTasksIndirectCountNV && loc.function != Func::vkCmdDrawMeshTasksEXT &&
        loc.function != Func::vkCmdDrawMeshTasksIndirectEXT && loc.function != Func::vkCmdDrawMeshTasksIndirectCountEXT) {
        ValidateShaderObjectGraphicsDrawtimeState(last_bound_state, loc);
    }

    return skip;
}

bool CoreChecks::ValidateShaderObjectGraphicsDrawtimeState(const LastBound &last_bound_state, const Location &loc) const {
    bool skip = false;

    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;
    const LogObjectList objlist(cb_state.Handle());
    const vvl::DrawDispatchVuid &vuid = vvl::GetDrawDispatchVuid(loc.function);

    bool validTaskShader = last_bound_state.GetShader(ShaderObjectStage::TASK);
    bool validMeshShader = last_bound_state.GetShader(ShaderObjectStage::MESH);
    if (validTaskShader || validMeshShader) {
        std::stringstream msg;
        if (validTaskShader && validMeshShader) {
            msg << "Task and mesh shaders are bound.";
        } else if (validTaskShader) {
            msg << "Task shader is bound.";
        } else {
            msg << "Mesh shader is bound.";
        }
        skip |= LogError(vuid.draw_shaders_no_task_mesh_08885, objlist, loc, "%s", msg.str().c_str());
    }

    return skip;
}

// Verify that PSO creation renderPass is compatible with active (non-dynamic) renderPass
bool CoreChecks::ValidatePipelineRenderpassDraw(const LastBound &last_bound_state, const Location &loc) const {
    bool skip = false;
    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;
    const vvl::Pipeline &pipeline = *last_bound_state.pipeline_state;
    const vvl::DrawDispatchVuid &vuid = vvl::GetDrawDispatchVuid(loc.function);

    const auto &rp_state = pipeline.RenderPassState();
    // TODO: AMD extension codes are included here, but actual function entrypoints are not yet intercepted
    if (cb_state.activeRenderPass->VkHandle() != rp_state->VkHandle()) {
        // renderPass that PSO was created with must be compatible with active renderPass that PSO is being used with
        skip |= ValidateRenderPassCompatibility(cb_state.Handle(), *cb_state.activeRenderPass.get(), pipeline.Handle(),
                                                *rp_state.get(), loc, vuid.render_pass_compatible_02684);
    }
    const auto subpass = pipeline.Subpass();
    if (subpass != cb_state.GetActiveSubpass()) {
        const LogObjectList objlist(cb_state.Handle(), pipeline.Handle(), cb_state.activeRenderPass->Handle());
        skip |= LogError(vuid.subpass_index_02685, objlist, loc,
                         "Pipeline was built for subpass %" PRIu32 " but used in subpass %" PRIu32 ".", subpass,
                         cb_state.GetActiveSubpass());
    }
    const vku::safe_VkAttachmentReference2 *ds_attachment =
        cb_state.activeRenderPass->create_info.pSubpasses[cb_state.GetActiveSubpass()].pDepthStencilAttachment;
    if (ds_attachment != nullptr) {
        // Check if depth stencil attachment was created with sample location compatible bit
        if (pipeline.SampleLocationEnabled() == VK_TRUE) {
            const uint32_t attachment = ds_attachment->attachment;
            if (attachment != VK_ATTACHMENT_UNUSED) {
                const auto *imageview_state = cb_state.GetActiveAttachmentImageViewState(attachment);
                if (imageview_state != nullptr) {
                    const auto *image_state = imageview_state->image_state.get();
                    if (image_state != nullptr) {
                        if ((image_state->create_info.flags & VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT) == 0) {
                            const LogObjectList objlist(cb_state.Handle(), pipeline.Handle(), cb_state.activeRenderPass->Handle());
                            skip |= LogError(vuid.sample_location_02689, objlist, loc,
                                             "sampleLocationsEnable is true for the pipeline, but the subpass (%u) depth "
                                             "stencil attachment's VkImage was not created with "
                                             "VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT.",
                                             cb_state.GetActiveSubpass());
                        }
                    }
                }
            }
        }
        const auto ds_state = pipeline.DepthStencilState();
        if (ds_state) {
            if (IsImageLayoutDepthReadOnly(ds_attachment->layout) && last_bound_state.IsDepthWriteEnable()) {
                const LogObjectList objlist(pipeline.Handle(), cb_state.activeRenderPass->Handle(), cb_state.Handle());
                skip |= LogError(vuid.depth_read_only_06886, objlist, loc,
                                 "depthWriteEnable is VK_TRUE, while the layout (%s) of "
                                 "the depth aspect of the depth/stencil attachment in the render pass is read only.",
                                 string_VkImageLayout(ds_attachment->layout));
            }

            VkStencilOpState front = last_bound_state.GetStencilOpStateFront();
            VkStencilOpState back = last_bound_state.GetStencilOpStateBack();

            const bool all_keep_op = ((front.failOp == VK_STENCIL_OP_KEEP) && (front.passOp == VK_STENCIL_OP_KEEP) &&
                                      (front.depthFailOp == VK_STENCIL_OP_KEEP) && (back.failOp == VK_STENCIL_OP_KEEP) &&
                                      (back.passOp == VK_STENCIL_OP_KEEP) && (back.depthFailOp == VK_STENCIL_OP_KEEP));

            const bool write_mask_enabled = (front.writeMask != 0) && (back.writeMask != 0);

            if (!all_keep_op && write_mask_enabled) {
                const bool is_stencil_layout_read_only = [&]() {
                    // Look for potential dedicated stencil layout
                    if (const auto *stencil_layout = vku::FindStructInPNextChain<VkAttachmentReferenceStencilLayoutKHR>(ds_attachment->pNext);
                        stencil_layout)
                        return IsImageLayoutStencilReadOnly(stencil_layout->stencilLayout);
                    // Else depth and stencil share same layout
                    return IsImageLayoutStencilReadOnly(ds_attachment->layout);
                }();

                if (is_stencil_layout_read_only) {
                    const LogObjectList objlist(pipeline.Handle(), cb_state.activeRenderPass->Handle(), cb_state.Handle());
                    skip |= LogError(vuid.stencil_read_only_06887, objlist, loc,
                                     "The layout (%s) of the stencil aspect of the depth/stencil attachment in the render pass "
                                     "is read only but not all stencil ops are VK_STENCIL_OP_KEEP.\n"
                                     "front = { .failOp = %s,  .passOp = %s , .depthFailOp = %s }\n"
                                     "back = { .failOp = %s, .passOp = %s, .depthFailOp = %s }\n",
                                     string_VkImageLayout(ds_attachment->layout), string_VkStencilOp(front.failOp),
                                     string_VkStencilOp(front.passOp), string_VkStencilOp(front.depthFailOp),
                                     string_VkStencilOp(back.failOp), string_VkStencilOp(back.passOp),
                                     string_VkStencilOp(back.depthFailOp));
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidatePipelineDynamicRenderpassDraw(const LastBound &last_bound_state, const Location &loc) const {
    bool skip = false;
    const vvl::CommandBuffer &cb_state = last_bound_state.cb_state;
    const vvl::Pipeline *pipeline = last_bound_state.pipeline_state;
    const vvl::DrawDispatchVuid &vuid = vvl::GetDrawDispatchVuid(loc.function);
    const auto &rendering_info = cb_state.activeRenderPass->dynamic_rendering_begin_rendering_info;
    const auto &rp_state = pipeline->RenderPassState();
    const auto &pipeline_ci = pipeline->GraphicsCreateInfo();
    if (rp_state) {
        const auto rendering_view_mask = cb_state.activeRenderPass->GetDynamicRenderingViewMask();
        if (rp_state->VkHandle() != VK_NULL_HANDLE) {
            const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), rp_state->Handle());
            skip |= LogError(vuid.dynamic_rendering_06198, objlist, loc,
                             "Currently bound pipeline %s must have been created with a "
                             "VkGraphicsPipelineCreateInfo::renderPass equal to VK_NULL_HANDLE",
                             FormatHandle(*pipeline).c_str());
        }

        const auto pipeline_rendering_ci = rp_state->dynamic_pipeline_rendering_create_info;

        if (pipeline_rendering_ci.viewMask != rendering_view_mask) {
            const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
            skip |= LogError(vuid.dynamic_rendering_view_mask_06178, objlist, loc,
                             "Currently bound pipeline %s viewMask ([%" PRIu32
                             ") must be equal to VkRenderingInfo::viewMask ([%" PRIu32 ")",
                             FormatHandle(*pipeline).c_str(), pipeline_rendering_ci.viewMask, rendering_view_mask);
        }

        if (!enabled_features.dynamicRenderingUnusedAttachments) {
            const auto color_attachment_count = pipeline_rendering_ci.colorAttachmentCount;
            const auto rendering_color_attachment_count = cb_state.activeRenderPass->GetDynamicRenderingColorAttachmentCount();
            if (color_attachment_count && (color_attachment_count != rendering_color_attachment_count)) {
                const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                skip |= LogError(vuid.dynamic_rendering_color_count_06179, objlist, loc,
                                 "Currently bound pipeline %s VkPipelineRenderingCreateInfo::colorAttachmentCount (%" PRIu32
                                 ") must be equal to VkRenderingInfo::colorAttachmentCount (%" PRIu32 ")",
                                 FormatHandle(*pipeline).c_str(), pipeline_rendering_ci.colorAttachmentCount,
                                 rendering_color_attachment_count);
            }
        }

        for (uint32_t i = 0; i < rendering_info.colorAttachmentCount; ++i) {
            if (enabled_features.dynamicRenderingUnusedAttachments) {
                if (rendering_info.pColorAttachments[i].imageView != VK_NULL_HANDLE) {
                    auto view_state = Get<vvl::ImageView>(rendering_info.pColorAttachments[i].imageView);
                    if ((pipeline_rendering_ci.colorAttachmentCount > i) &&
                        (view_state->create_info.format != VK_FORMAT_UNDEFINED) &&
                        (pipeline_rendering_ci.pColorAttachmentFormats[i] != VK_FORMAT_UNDEFINED) &&
                        (view_state->create_info.format != pipeline_rendering_ci.pColorAttachmentFormats[i])) {
                        const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                        skip |= LogError(vuid.dynamic_rendering_unused_attachments_08911, objlist, loc,
                                         "VkRenderingInfo::pColorAttachments[%" PRIu32
                                         "].imageView format (%s) must match corresponding format in "
                                         "VkPipelineRenderingCreateInfo::pColorAttachmentFormats[%" PRIu32
                                         "] (%s) "
                                         "when both are not VK_FORMAT_UNDEFINED",
                                         i, string_VkFormat(view_state->create_info.format), i,
                                         string_VkFormat(pipeline_rendering_ci.pColorAttachmentFormats[i]));
                    }
                }
            } else {
                if (rendering_info.pColorAttachments[i].imageView == VK_NULL_HANDLE) {
                    if ((pipeline_rendering_ci.colorAttachmentCount > i) &&
                        (pipeline_rendering_ci.pColorAttachmentFormats[i] != VK_FORMAT_UNDEFINED)) {
                        const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                        skip |= LogError(vuid.dynamic_rendering_undefined_color_formats_08912, objlist, loc,
                                         "VkRenderingInfo::pColorAttachments[%" PRIu32
                                         "].imageView is VK_NULL_HANDLE, but corresponding format in "
                                         "VkPipelineRenderingCreateInfo::pColorAttachmentFormats[%" PRIu32 "] is %s.",
                                         i, i, string_VkFormat(pipeline_rendering_ci.pColorAttachmentFormats[i]));
                    }
                } else {
                    auto view_state = Get<vvl::ImageView>(rendering_info.pColorAttachments[i].imageView);
                    if ((pipeline_rendering_ci.colorAttachmentCount > i) &&
                        view_state->create_info.format != pipeline_rendering_ci.pColorAttachmentFormats[i]) {
                        const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                        skip |= LogError(vuid.dynamic_rendering_color_formats_08910, objlist, loc,
                                         "VkRenderingInfo::pColorAttachments[%" PRIu32
                                         "].imageView format (%s) must match corresponding format in "
                                         "VkPipelineRenderingCreateInfo::pColorAttachmentFormats[%" PRIu32 "] (%s)",
                                         i, string_VkFormat(view_state->create_info.format), i,
                                         string_VkFormat(pipeline_rendering_ci.pColorAttachmentFormats[i]));
                    }
                }
            }
        }

        if (rendering_info.pDepthAttachment) {
            if (enabled_features.dynamicRenderingUnusedAttachments) {
                if (rendering_info.pDepthAttachment->imageView != VK_NULL_HANDLE) {
                    auto view_state = Get<vvl::ImageView>(rendering_info.pDepthAttachment->imageView);
                    if ((view_state->create_info.format != VK_FORMAT_UNDEFINED) &&
                        (pipeline_rendering_ci.depthAttachmentFormat != VK_FORMAT_UNDEFINED) &&
                        (view_state->create_info.format != pipeline_rendering_ci.depthAttachmentFormat)) {
                        const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                        skip |= LogError(vuid.dynamic_rendering_unused_attachments_08915, objlist, loc,
                                         "VkRenderingInfo::pDepthAttachment->imageView format (%s) must match corresponding format "
                                         "in VkPipelineRenderingCreateInfo::depthAttachmentFormat (%s) "
                                         "if both are not VK_FORMAT_UNDEFINED",
                                         string_VkFormat(view_state->create_info.format),
                                         string_VkFormat(pipeline_rendering_ci.depthAttachmentFormat));
                    }
                }
            } else {
                if (rendering_info.pDepthAttachment->imageView == VK_NULL_HANDLE) {
                    if (pipeline_rendering_ci.depthAttachmentFormat != VK_FORMAT_UNDEFINED) {
                        const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                        skip |=
                            LogError(vuid.dynamic_rendering_undefined_depth_format_08913, objlist, loc,
                                     "VkRenderingInfo::pDepthAttachment.imageView is VK_NULL_HANDLE, but corresponding format in "
                                     "VkPipelineRenderingCreateInfo::depthAttachmentFormat is %s.",
                                     string_VkFormat(pipeline_rendering_ci.depthAttachmentFormat));
                    }
                } else {
                    auto view_state = Get<vvl::ImageView>(rendering_info.pDepthAttachment->imageView);
                    if (view_state->create_info.format != pipeline_rendering_ci.depthAttachmentFormat) {
                        const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                        skip |= LogError(vuid.dynamic_rendering_depth_format_08914, objlist, loc,
                                         "VkRenderingInfo::pDepthAttachment->imageView format (%s) must match corresponding format "
                                         "in VkPipelineRenderingCreateInfo::depthAttachmentFormat (%s)",
                                         string_VkFormat(view_state->create_info.format),
                                         string_VkFormat(pipeline_rendering_ci.depthAttachmentFormat));
                    }
                }
            }
        } else if (cb_state.activeRenderPass->use_dynamic_rendering_inherited) {
            if (cb_state.activeRenderPass->inheritance_rendering_info.depthAttachmentFormat !=
                    pipeline_rendering_ci.depthAttachmentFormat &&
                !enabled_features.dynamicRenderingUnusedAttachments) {
                const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                skip |= LogError(vuid.dynamic_rendering_depth_format_08914, objlist, loc,
                                 "VkCommandBufferInheritanceRenderingInfo::depthAttachmentFormat (%s) must match corresponding "
                                 "format in VkPipelineRenderingCreateInfo::depthAttachmentFormat (%s)",
                                 string_VkFormat(cb_state.activeRenderPass->inheritance_rendering_info.depthAttachmentFormat),
                                 string_VkFormat(pipeline_rendering_ci.depthAttachmentFormat));
            }
        }

        if (rendering_info.pStencilAttachment) {
            if (enabled_features.dynamicRenderingUnusedAttachments) {
                if (rendering_info.pStencilAttachment->imageView != VK_NULL_HANDLE) {
                    auto view_state = Get<vvl::ImageView>(rendering_info.pStencilAttachment->imageView);
                    if ((view_state->create_info.format != VK_FORMAT_UNDEFINED) &&
                        (pipeline_rendering_ci.stencilAttachmentFormat != VK_FORMAT_UNDEFINED) &&
                        (view_state->create_info.format != pipeline_rendering_ci.stencilAttachmentFormat)) {
                        const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                        skip |= LogError(vuid.dynamic_rendering_unused_attachments_08918, objlist, loc,
                                         "VkRenderingInfo::pStencilAttachment->imageView format (%s) must match corresponding "
                                         "format in VkPipelineRenderingCreateInfo::stencilAttachmentFormat (%s) "
                                         "if both are not VK_FORMAT_UNDEFINED",
                                         string_VkFormat(view_state->create_info.format),
                                         string_VkFormat(pipeline_rendering_ci.stencilAttachmentFormat));
                    }
                }
            } else {
                if (rendering_info.pStencilAttachment->imageView == VK_NULL_HANDLE) {
                    if (pipeline_rendering_ci.stencilAttachmentFormat != VK_FORMAT_UNDEFINED) {
                        const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                        skip |= LogError(vuid.dynamic_rendering_undefined_stencil_format_08916, objlist, loc,
                                         "VkRenderingInfo::pStencilAttachment.imageView is VK_NULL_HANDLE, but "
                                         "corresponding format in "
                                         "VkPipelineRenderingCreateInfo::stencilAttachmentFormat is %s.",
                                         string_VkFormat(pipeline_rendering_ci.stencilAttachmentFormat));
                    }
                } else {
                    auto view_state = Get<vvl::ImageView>(rendering_info.pStencilAttachment->imageView);
                    if (view_state->create_info.format != pipeline_rendering_ci.stencilAttachmentFormat) {
                        const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                        skip |= LogError(vuid.dynamic_rendering_stencil_format_08917, objlist, loc,
                                         "VkRenderingInfo::pStencilAttachment->imageView format (%s) must match corresponding "
                                         "format in VkPipelineRenderingCreateInfo::stencilAttachmentFormat (%s)",
                                         string_VkFormat(view_state->create_info.format),
                                         string_VkFormat(pipeline_rendering_ci.stencilAttachmentFormat));
                    }
                }
            }
        } else if (cb_state.activeRenderPass->use_dynamic_rendering_inherited) {
            if (cb_state.activeRenderPass->inheritance_rendering_info.stencilAttachmentFormat !=
                    pipeline_rendering_ci.stencilAttachmentFormat &&
                !enabled_features.dynamicRenderingUnusedAttachments) {
                const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                skip |= LogError(vuid.dynamic_rendering_stencil_format_08917, objlist, loc,
                                 "VkCommandBufferInheritanceRenderingInfo::stencilAttachmentFormat (%s) must match corresponding "
                                 "format in VkPipelineRenderingCreateInfo::stencilAttachmentFormat (%s)",
                                 string_VkFormat(cb_state.activeRenderPass->inheritance_rendering_info.stencilAttachmentFormat),
                                 string_VkFormat(pipeline_rendering_ci.stencilAttachmentFormat));
            }
        }

        if (cb_state.activeRenderPass->UsesDynamicRendering()) {
            auto rendering_fragment_shading_rate_attachment_info =
                vku::FindStructInPNextChain<VkRenderingFragmentShadingRateAttachmentInfoKHR>(rendering_info.pNext);
            if (rendering_fragment_shading_rate_attachment_info &&
                (rendering_fragment_shading_rate_attachment_info->imageView != VK_NULL_HANDLE)) {
                if (!(pipeline->create_flags & VK_PIPELINE_CREATE_2_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
                    const LogObjectList objlist(cb_state.Handle(), pipeline->Handle());
                    skip |= LogError(vuid.dynamic_rendering_fsr_06183, objlist, loc,
                                     "Currently bound graphics pipeline %s must have been created with "
                                     "VK_PIPELINE_CREATE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR",
                                     FormatHandle(*pipeline).c_str());
                }
            }

            auto rendering_fragment_shading_rate_density_map =
                vku::FindStructInPNextChain<VkRenderingFragmentDensityMapAttachmentInfoEXT>(rendering_info.pNext);
            if (rendering_fragment_shading_rate_density_map &&
                (rendering_fragment_shading_rate_density_map->imageView != VK_NULL_HANDLE)) {
                if (!(pipeline->create_flags & VK_PIPELINE_RASTERIZATION_STATE_CREATE_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT)) {
                    const LogObjectList objlist(cb_state.Handle(), pipeline->Handle());
                    skip |= LogError(vuid.dynamic_rendering_fdm_06184, objlist, loc,
                                     "Currently bound graphics pipeline %s must have been created with "
                                     "VK_PIPELINE_RASTERIZATION_STATE_CREATE_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT",
                                     FormatHandle(*pipeline).c_str());
                }
            }
        }

        if ((pipeline->active_shaders & VK_SHADER_STAGE_FRAGMENT_BIT) != 0) {
            for (uint32_t i = 0; i < rendering_info.colorAttachmentCount; ++i) {
                const bool statically_writes_to_color_attachment = pipeline->fragmentShader_writable_output_location_list.find(i) !=
                                                                   pipeline->fragmentShader_writable_output_location_list.end();
                const bool mask_and_write_enabled =
                    last_bound_state.GetColorWriteMask(i) != 0 && last_bound_state.IsColorWriteEnabled(i);
                if (statically_writes_to_color_attachment && mask_and_write_enabled &&
                    rendering_info.pColorAttachments[i].imageView != VK_NULL_HANDLE) {
                    if (i >= pipeline_rendering_ci.colorAttachmentCount ||
                        pipeline_rendering_ci.pColorAttachmentFormats[i] == VK_FORMAT_UNDEFINED) {
                        const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle(),
                                                    rendering_info.pColorAttachments[i].imageView);
                        skip |= LogError(vuid.color_attachment_08963, objlist, loc,
                                         "VkRenderingInfo::pColorAttachments[%" PRIu32
                                         "] is %s, but currently bound graphics pipeline %s was created with "
                                         "VkPipelineRenderingCreateInfo::pColorAttachmentFormats[%" PRIu32
                                         "] equal to VK_FORMAT_UNDEFINED",
                                         i, FormatHandle(rendering_info.pColorAttachments[i].imageView).c_str(),
                                         FormatHandle(*pipeline).c_str(), i);
                    }
                }
            }
        }
        if (last_bound_state.IsDepthTestEnable() && last_bound_state.IsDepthWriteEnable() && rp_state->use_dynamic_rendering &&
            rendering_info.pDepthAttachment && rendering_info.pDepthAttachment->imageView != VK_NULL_HANDLE) {
            if (pipeline_rendering_ci.depthAttachmentFormat == VK_FORMAT_UNDEFINED) {
                const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                skip |= LogError(vuid.depth_attachment_08964, objlist, loc,
                                 "VkRenderingInfo::pDepthAttachment is %s, but currently bound graphics pipeline %s was created "
                                 "with VkPipelineRenderingCreateInfo::depthAttachmentFormat equal to VK_FORMAT_UNDEFINED",
                                 FormatHandle(rendering_info.pDepthAttachment->imageView).c_str(), FormatHandle(*pipeline).c_str());
            }
        }
        if (last_bound_state.IsStencilTestEnable() && rp_state->use_dynamic_rendering && rendering_info.pStencilAttachment &&
            rendering_info.pStencilAttachment->imageView != VK_NULL_HANDLE) {
            if (pipeline_rendering_ci.stencilAttachmentFormat == VK_FORMAT_UNDEFINED) {
                const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                skip |=
                    LogError(vuid.stencil_attachment_08965, objlist, loc,
                             "VkRenderingInfo::pStencilAttachment is %s, but currently bound graphics pipeline %s was created with "
                             "VkPipelineRenderingCreateInfo::stencilAttachmentFormat equal to VK_FORMAT_UNDEFINED",
                             FormatHandle(rendering_info.pStencilAttachment->imageView).c_str(), FormatHandle(*pipeline).c_str());
            }
        }

        const uint64_t pipeline_external_format = GetExternalFormat(pipeline_ci.pNext);
        if (pipeline_external_format != 0) {
            const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
            if (rendering_info.colorAttachmentCount == 1 &&
                rendering_info.pColorAttachments[0].resolveMode == VK_RESOLVE_MODE_EXTERNAL_FORMAT_DOWNSAMPLE_ANDROID) {
                auto resolve_image_view_state = Get<vvl::ImageView>(rendering_info.pColorAttachments[0].resolveImageView);
                if (resolve_image_view_state) {
                    if (resolve_image_view_state->image_state->ahb_format != pipeline_external_format) {
                        skip |= LogError(vuid.external_format_resolve_09362, objlist, loc,
                                         "pipeline externalFormat is %" PRIu64
                                         " but the resolveImageView's image was created with externalFormat %" PRIu64 "",
                                         pipeline_external_format, resolve_image_view_state->image_state->ahb_format);
                    }
                }

                auto color_image_view_state = Get<vvl::ImageView>(rendering_info.pColorAttachments[0].imageView);
                if (color_image_view_state) {
                    if (color_image_view_state->image_state->ahb_format != pipeline_external_format) {
                        skip |= LogError(vuid.external_format_resolve_09363, objlist, loc,
                                         "pipeline externalFormat is %" PRIu64
                                         " but the imageView's image was created with externalFormat %" PRIu64 "",
                                         pipeline_external_format, color_image_view_state->image_state->ahb_format);
                    }
                }
            }

            if (pipeline->IsDynamic(VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT) &&
                cb_state.dynamic_state_value.color_blend_enable_attachments.test(0)) {
                skip |= LogError(vuid.external_format_resolve_09364, objlist, loc,
                                 "pipeline externalFormat is %" PRIu64
                                 ", but dynamic blend enable for attachment zero was set to VK_TRUE.",
                                 pipeline_external_format);
            }
            if (pipeline->IsDynamic(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT) &&
                cb_state.dynamic_state_value.rasterization_samples != VK_SAMPLE_COUNT_1_BIT) {
                skip |= LogError(vuid.external_format_resolve_09365, objlist, loc,
                                 "pipeline externalFormat is %" PRIu64 ", but dynamic rasterization samples set to %s.",
                                 pipeline_external_format,
                                 string_VkSampleCountFlagBits(cb_state.dynamic_state_value.rasterization_samples));
            }
            if (pipeline->IsDynamic(VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR)) {
                if (cb_state.dynamic_state_value.fragment_size.width != 1) {
                    skip |= LogError(vuid.external_format_resolve_09368, objlist, loc,
                                     "pipeline externalFormat is %" PRIu64 ", but dynamic fragment size width is %" PRIu32 ".",
                                     pipeline_external_format, cb_state.dynamic_state_value.fragment_size.width);
                }
                if (cb_state.dynamic_state_value.fragment_size.height != 1) {
                    skip |= LogError(vuid.external_format_resolve_09369, objlist, loc,
                                     "pipeline externalFormat is %" PRIu64 ", but dynamic fragment size height is %" PRIu32 ".",
                                     pipeline_external_format, cb_state.dynamic_state_value.fragment_size.height);
                }
            }

            if (pipeline->fragment_shader_state && pipeline->fragment_shader_state->fragment_entry_point) {
                auto entrypoint = pipeline->fragment_shader_state->fragment_entry_point;
                if (entrypoint->execution_mode.Has(spirv::ExecutionModeSet::depth_replacing_bit)) {
                    skip |= LogError(vuid.external_format_resolve_09372, objlist, loc,
                                     "pipeline externalFormat is %" PRIu64 " but the fragment shader declares DepthReplacing.",
                                     pipeline_external_format);
                } else if (entrypoint->execution_mode.Has(spirv::ExecutionModeSet::stencil_ref_replacing_bit)) {
                    skip |=
                        LogError(vuid.external_format_resolve_09372, objlist, loc,
                                 "pipeline externalFormat is %" PRIu64 " but the fragment shader declares StencilRefReplacingEXT.",
                                 pipeline_external_format);
                }
            }
        }
    }

    // VkAttachmentSampleCountInfoAMD == VkAttachmentSampleCountInfoNV
    auto p_attachment_sample_count_info = vku::FindStructInPNextChain<VkAttachmentSampleCountInfoAMD>(pipeline_ci.pNext);

    if (p_attachment_sample_count_info) {
        for (uint32_t i = 0; i < rendering_info.colorAttachmentCount; ++i) {
            if (rendering_info.pColorAttachments[i].imageView == VK_NULL_HANDLE) {
                continue;
            }
            auto color_view_state = Get<vvl::ImageView>(rendering_info.pColorAttachments[i].imageView);
            auto color_image_samples = Get<vvl::Image>(color_view_state->create_info.image)->create_info.samples;

            if (p_attachment_sample_count_info &&
                (color_image_samples != p_attachment_sample_count_info->pColorAttachmentSamples[i])) {
                const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                skip |= LogError(vuid.dynamic_rendering_color_sample_06185, objlist, loc,
                                 "Color attachment (%" PRIu32
                                 ") sample count (%s) must match corresponding VkAttachmentSampleCountInfoAMD "
                                 "sample count (%s)",
                                 i, string_VkSampleCountFlagBits(color_image_samples),
                                 string_VkSampleCountFlagBits(p_attachment_sample_count_info->pColorAttachmentSamples[i]));
            }
        }

        if (rendering_info.pDepthAttachment != nullptr) {
            auto depth_view_state = Get<vvl::ImageView>(rendering_info.pDepthAttachment->imageView);
            auto depth_image_samples = Get<vvl::Image>(depth_view_state->create_info.image)->create_info.samples;

            if (p_attachment_sample_count_info) {
                if (depth_image_samples != p_attachment_sample_count_info->depthStencilAttachmentSamples) {
                    const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                    skip |= LogError(vuid.dynamic_rendering_depth_sample_06186, objlist, loc,
                                     "Depth attachment sample count (%s) must match corresponding "
                                     "VkAttachmentSampleCountInfoAMD sample "
                                     "count (%s)",
                                     string_VkSampleCountFlagBits(depth_image_samples),
                                     string_VkSampleCountFlagBits(p_attachment_sample_count_info->depthStencilAttachmentSamples));
                }
            }
        }

        if (rendering_info.pStencilAttachment != nullptr) {
            auto stencil_view_state = Get<vvl::ImageView>(rendering_info.pStencilAttachment->imageView);
            auto stencil_image_samples = Get<vvl::Image>(stencil_view_state->create_info.image)->create_info.samples;

            if (p_attachment_sample_count_info) {
                if (stencil_image_samples != p_attachment_sample_count_info->depthStencilAttachmentSamples) {
                    const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                    skip |= LogError(vuid.dynamic_rendering_stencil_sample_06187, objlist, loc,
                                     "Stencil attachment sample count (%s) must match corresponding "
                                     "VkAttachmentSampleCountInfoAMD "
                                     "sample count (%s)",
                                     string_VkSampleCountFlagBits(stencil_image_samples),
                                     string_VkSampleCountFlagBits(p_attachment_sample_count_info->depthStencilAttachmentSamples));
                }
            }
        }
    } else if (!enabled_features.multisampledRenderToSingleSampled &&
               !enabled_features.externalFormatResolve) {
        const VkSampleCountFlagBits rasterization_samples = last_bound_state.GetRasterizationSamples();
        for (uint32_t i = 0; i < rendering_info.colorAttachmentCount; ++i) {
            if (rendering_info.pColorAttachments[i].imageView == VK_NULL_HANDLE) {
                continue;
            }
            auto view_state = Get<vvl::ImageView>(rendering_info.pColorAttachments[i].imageView);
            auto samples = Get<vvl::Image>(view_state->create_info.image)->create_info.samples;

            if (samples != rasterization_samples) {
                const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                skip |= LogError(vuid.dynamic_rendering_07285, objlist, loc,
                                 "Color attachment (%" PRIu32
                                 ") sample count (%s) must match corresponding VkPipelineMultisampleStateCreateInfo "
                                 "sample count (%s)",
                                 i, string_VkSampleCountFlagBits(samples), string_VkSampleCountFlagBits(rasterization_samples));
            }
        }

        if ((rendering_info.pDepthAttachment != nullptr) && (rendering_info.pDepthAttachment->imageView != VK_NULL_HANDLE)) {
            const auto &depth_view_state = Get<vvl::ImageView>(rendering_info.pDepthAttachment->imageView);
            const auto &depth_image_samples = Get<vvl::Image>(depth_view_state->create_info.image)->create_info.samples;
            if (depth_image_samples != rasterization_samples) {
                const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                skip |= LogError(vuid.dynamic_rendering_07286, objlist, loc,
                                 "Depth attachment sample count (%s) must match corresponding "
                                 "VkPipelineMultisampleStateCreateInfo::rasterizationSamples count (%s)",
                                 string_VkSampleCountFlagBits(depth_image_samples),
                                 string_VkSampleCountFlagBits(rasterization_samples));
            }
        }

        if ((rendering_info.pStencilAttachment != nullptr) && (rendering_info.pStencilAttachment->imageView != VK_NULL_HANDLE)) {
            const auto &stencil_view_state = Get<vvl::ImageView>(rendering_info.pStencilAttachment->imageView);
            const auto &stencil_image_samples = Get<vvl::Image>(stencil_view_state->create_info.image)->create_info.samples;
            if (stencil_image_samples != rasterization_samples) {
                const LogObjectList objlist(cb_state.Handle(), pipeline->Handle(), cb_state.activeRenderPass->Handle());
                skip |= LogError(vuid.dynamic_rendering_07287, objlist, loc,
                                 "Stencil attachment sample count (%s) must match corresponding "
                                 "VkPipelineMultisampleStateCreateInfo::rasterizationSamples count (%s)",
                                 string_VkSampleCountFlagBits(stencil_image_samples),
                                 string_VkSampleCountFlagBits(rasterization_samples));
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidatePipelineVertexDivisors(const vvl::Pipeline &pipeline, const Location &create_info_loc) const {
    bool skip = false;
    const auto *input_state = pipeline.InputState();
    if (!input_state) {
        return skip;
    }
    const auto divisor_state_info = vku::FindStructInPNextChain<VkPipelineVertexInputDivisorStateCreateInfoKHR>(input_state->pNext);
    if (!divisor_state_info) {
        return skip;
    }

    const Location vertex_input_loc = create_info_loc.dot(Field::pVertexInputState);
    // Can use raw Pipeline state values because not using the stride (which can be dynamic with
    // VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE)
    const auto &binding_descriptions = pipeline.vertex_input_state->binding_descriptions;
    const VkPhysicalDeviceLimits *device_limits = &phys_dev_props.limits;
    for (uint32_t j = 0; j < divisor_state_info->vertexBindingDivisorCount; j++) {
        const Location divisor_loc =
            vertex_input_loc.pNext(Struct::VkVertexInputBindingDivisorDescriptionKHR, Field::pVertexBindingDivisors, j);
        const auto *vibdd = &(divisor_state_info->pVertexBindingDivisors[j]);
        if (vibdd->binding >= device_limits->maxVertexInputBindings) {
            skip |= LogError("VUID-VkVertexInputBindingDivisorDescriptionKHR-binding-01869", device,
                             divisor_loc.dot(Field::binding), "(%" PRIu32 ") exceeds device maxVertexInputBindings (%" PRIu32 ").",
                             vibdd->binding, device_limits->maxVertexInputBindings);
        }
        if (vibdd->divisor > phys_dev_ext_props.vtx_attrib_divisor_props.maxVertexAttribDivisor) {
            skip |= LogError("VUID-VkVertexInputBindingDivisorDescriptionKHR-divisor-01870", device,
                             divisor_loc.dot(Field::divisor), "(%" PRIu32 ") exceeds device maxVertexAttribDivisor (%" PRIu32 ").",
                             vibdd->divisor, phys_dev_ext_props.vtx_attrib_divisor_props.maxVertexAttribDivisor);
        }
        if ((0 == vibdd->divisor) && !enabled_features.vertexAttributeInstanceRateZeroDivisor) {
            skip |=
                LogError("VUID-VkVertexInputBindingDivisorDescriptionKHR-vertexAttributeInstanceRateZeroDivisor-02228", device,
                         divisor_loc.dot(Field::divisor),
                         "is (%" PRIu32 ") but vertexAttributeInstanceRateZeroDivisor feature was not enabled.", vibdd->divisor);
        }
        if ((1 != vibdd->divisor) && !enabled_features.vertexAttributeInstanceRateDivisor) {
            skip |= LogError("VUID-VkVertexInputBindingDivisorDescriptionKHR-vertexAttributeInstanceRateDivisor-02229", device,
                             divisor_loc.dot(Field::divisor),
                             "is (%" PRIu32 ") but vertexAttributeInstanceRateDivisor feature was not enabled.", vibdd->divisor);
        }

        // Find the corresponding binding description and validate input rate setting
        bool found_input_rate = false;
        for (size_t k = 0; k < binding_descriptions.size(); k++) {
            if ((vibdd->binding == binding_descriptions[k].binding) &&
                (VK_VERTEX_INPUT_RATE_INSTANCE == binding_descriptions[k].inputRate)) {
                found_input_rate = true;
                break;
            }
        }
        if (!found_input_rate) {  // Description not found, or has incorrect inputRate value
            skip |=
                LogError("VUID-VkVertexInputBindingDivisorDescriptionKHR-inputRate-01871", device, divisor_loc.dot(Field::binding),
                         "is %" PRIu32 ", but inputRate is not VK_VERTEX_INPUT_RATE_INSTANCE.", vibdd->binding);
        }
    }
    return skip;
}

bool CoreChecks::ValidatePipelineLibraryFlags(const VkGraphicsPipelineLibraryFlagsEXT lib_flags,
                                              const VkPipelineLibraryCreateInfoKHR &link_info,
                                              const VkPipelineRenderingCreateInfo *rendering_struct, const Location &loc,
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
            const auto lib = Get<vvl::Pipeline>(link_info.pLibraries[i]);
            const auto lib_rendering_struct = lib->GetPipelineRenderingCreateInfo();
            const bool other_flag = (lib->graphics_lib_type & flags) && (lib->graphics_lib_type & ~lib_flags);
            if (!other_flag) {
                continue;
            }
            if (current_pipeline) {
                if (lib->GraphicsCreateInfo().renderPass != VK_NULL_HANDLE) {
                    skip |= LogError("VUID-VkGraphicsPipelineCreateInfo-renderpass-06625", device, loc.dot(Field::renderPass),
                                     "is VK_NULL_HANDLE and includes "
                                     "VkGraphicsPipelineLibraryCreateInfoEXT::flags (%s), but pLibraries[%" PRIu32
                                     "] includes VkGraphicsPipelineLibraryCreateInfoEXT::flags (%s) and "
                                     "render pass is not VK_NULL_HANDLE.",
                                     string_VkGraphicsPipelineLibraryFlagsEXT(lib_flags).c_str(), i,
                                     string_VkGraphicsPipelineLibraryFlagsEXT(lib->graphics_lib_type).c_str());
                }
            }
            uint32_t view_mask = rendering_struct ? rendering_struct->viewMask : 0;
            uint32_t lib_view_mask = lib_rendering_struct ? lib_rendering_struct->viewMask : 0;
            if (view_mask != lib_view_mask) {
                skip |= LogError(vuid, device, loc,
                                 "pLibraries[%" PRIu32 "] is (flags = %s and viewMask = %" PRIu32 "), but pLibraries[%" PRIu32
                                 "] is (flags = %s and viewMask %" PRIu32 ").",
                                 lib_index, string_VkGraphicsPipelineLibraryFlagsEXT(lib_flags).c_str(), view_mask, i,
                                 string_VkGraphicsPipelineLibraryFlagsEXT(lib->graphics_lib_type).c_str(), lib_view_mask);
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateGraphicsPipelineDerivatives(PipelineStates &pipeline_states, uint32_t pipe_index,
                                                     const Location &loc) const {
    bool skip = false;
    const auto &pipeline = *pipeline_states[pipe_index].get();
    // If create derivative bit is set, check that we've specified a base
    // pipeline correctly, and that the base pipeline was created to allow
    // derivatives.
    if (pipeline.create_flags & VK_PIPELINE_CREATE_2_DERIVATIVE_BIT_KHR) {
        std::shared_ptr<const vvl::Pipeline> base_pipeline;
        const auto &pipeline_ci = pipeline.GraphicsCreateInfo();
        const VkPipeline base_handle = pipeline_ci.basePipelineHandle;
        const int32_t base_index = pipeline_ci.basePipelineIndex;
        if (base_index != -1 && base_index < static_cast<int32_t>(pipeline_states.size())) {
            if (static_cast<uint32_t>(base_index) >= pipe_index) {
                skip |= LogError("VUID-vkCreateGraphicsPipelines-flags-00720", base_handle, loc,
                                 "base pipeline (index %" PRId32
                                 ") must occur earlier in array than derivative pipeline (index %" PRIu32 ").",
                                 base_index, pipe_index);
            } else {
                base_pipeline = pipeline_states[base_index];
            }
        } else if (base_handle != VK_NULL_HANDLE) {
            base_pipeline = Get<vvl::Pipeline>(base_handle);
        }

        if (base_pipeline && !(base_pipeline->create_flags & VK_PIPELINE_CREATE_2_ALLOW_DERIVATIVES_BIT_KHR)) {
            skip |= LogError("VUID-vkCreateGraphicsPipelines-flags-00721", base_pipeline->Handle(), loc,
                             "base pipeline does not allow derivatives.");
        }
    }
    return skip;
}

bool CoreChecks::ValidateMultiViewShaders(const vvl::Pipeline &pipeline, const Location &multiview_loc, uint32_t view_mask,
                                          bool dynamic_rendering) const {
    bool skip = false;
    const VkShaderStageFlags stages = pipeline.create_info_shaders;
    if (!enabled_features.multiviewTessellationShader &&
        (stages & (VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT))) {
        const char *vuid = dynamic_rendering ? "VUID-VkGraphicsPipelineCreateInfo-renderPass-06057"
                                             : "VUID-VkGraphicsPipelineCreateInfo-renderPass-06047";
        skip |= LogError(vuid, device, multiview_loc,
                         "is %" PRIu32
                         " and pStages contains tesselation shaders, but the multiviewTessellationShader feature was not enabled.",
                         view_mask);
    }

    if (!enabled_features.multiviewGeometryShader && (stages & VK_SHADER_STAGE_GEOMETRY_BIT)) {
        const char *vuid = dynamic_rendering ? "VUID-VkGraphicsPipelineCreateInfo-renderPass-06058"
                                             : "VUID-VkGraphicsPipelineCreateInfo-renderPass-06048";
        skip |= LogError(vuid, device, multiview_loc,
                         "is %" PRIu32
                         " and pStages contains geometry shader, but the multiviewGeometryShader feature was not enabled.",
                         view_mask);
    }

    if (!enabled_features.multiviewMeshShader && (stages & VK_SHADER_STAGE_MESH_BIT_EXT)) {
        const char *vuid = dynamic_rendering ? "VUID-VkGraphicsPipelineCreateInfo-renderPass-07720"
                                             : "VUID-VkGraphicsPipelineCreateInfo-renderPass-07064";
        skip |= LogError(vuid, device, multiview_loc,
                         "is %" PRIu32 " and pStages contains mesh shader, but the multiviewMeshShader feature was not enabled.",
                         view_mask);
    }

    for (const auto &stage : pipeline.stage_states) {
        // Stage may not have SPIR-V data (e.g. due to the use of shader module identifier or in Vulkan SC)
        if (!stage.spirv_state) continue;

        if (stage.spirv_state->static_data_.has_builtin_layer) {
            const char *vuid = dynamic_rendering ? "VUID-VkGraphicsPipelineCreateInfo-renderPass-06059"
                                                 : "VUID-VkGraphicsPipelineCreateInfo-renderPass-06050";
            skip |= LogError(vuid, device, multiview_loc, "is %" PRIu32 " but %s stage contains a Layer decorated OpVariable.",
                             view_mask, string_VkShaderStageFlagBits(stage.GetStage()));
        }
    }

    return skip;
}
