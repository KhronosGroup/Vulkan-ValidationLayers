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
 *
 * Author: Cody Northrop <cnorthrop@google.com>
 * Author: Michael Lentine <mlentine@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chia-I Wu <olv@google.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Ian Elliott <ianelliott@google.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: Dustin Graves <dustin@lunarg.com>
 * Author: Jeremy Hayes <jeremy@lunarg.com>
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Karl Schultz <karl@lunarg.com>
 * Author: Mark Young <marky@lunarg.com>
 * Author: Mike Schuchardt <mikes@lunarg.com>
 * Author: Mike Weiblen <mikew@lunarg.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 * Author: Shannon McPherson <shannon@lunarg.com>
 * Author: Jeremy Kniager <jeremyk@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
 * Author: Jeremy Gebben <jeremyg@lunarg.com>
 * Author: Daniel Rakos <daniel.rakos@rastergrid.com>
 */

#include <algorithm>
#include <array>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <string>

#include "vk_enum_string_helper.h"
#include "chassis.h"
#include "core_checks/core_validation.h"
#include "core_checks/shader_validation.h"
#include "vk_layer_utils.h"
#include "sync/sync_vuid_maps.h"
#include "stateless/stateless_validation.h"
#include "enum_flag_bits.h"

using std::max;
using std::string;
using std::stringstream;
using std::unique_ptr;
using std::vector;

ReadLockGuard CoreChecks::ReadLock() const {
    if (fine_grained_locking) {
        return ReadLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return ReadLockGuard(validation_object_mutex);
    }
}

WriteLockGuard CoreChecks::WriteLock() {
    if (fine_grained_locking) {
        return WriteLockGuard(validation_object_mutex, std::defer_lock);
    } else {
        return WriteLockGuard(validation_object_mutex);
    }
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

            const auto pipline_rendering_ci = rp_state->dynamic_rendering_pipeline_create_info;

            if (pipline_rendering_ci.viewMask != rendering_view_mask) {
                const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline(), cb_state.activeRenderPass->renderPass());
                skip |= LogError(objlist, vuid.dynamic_rendering_view_mask,
                                 "%s: Currently bound pipeline %s viewMask ([%" PRIu32
                                 ") must be equal to VkRenderingInfo::viewMask ([%" PRIu32 ")",
                                 caller, report_data->FormatHandle(state.pipeline_state->pipeline()).c_str(),
                                 pipline_rendering_ci.viewMask, rendering_view_mask);
            }

            const auto color_attachment_count = pipline_rendering_ci.colorAttachmentCount;
            const auto rendering_color_attachment_count = cb_state.activeRenderPass->GetDynamicRenderingColorAttachmentCount();
            if (color_attachment_count && (color_attachment_count != rendering_color_attachment_count)) {
                const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline(), cb_state.activeRenderPass->renderPass());
                skip |= LogError(objlist, vuid.dynamic_rendering_color_count,
                                 "%s: Currently bound pipeline %s VkPipelineRenderingCreateInfo::colorAttachmentCount ([%" PRIu32
                                 ") must be equal to VkRenderingInfo::colorAttachmentCount ([%" PRIu32 ")",
                                 caller, report_data->FormatHandle(state.pipeline_state->pipeline()).c_str(),
                                 pipline_rendering_ci.colorAttachmentCount, rendering_color_attachment_count);
            }

            for (uint32_t i = 0; i < rendering_info.colorAttachmentCount; ++i) {
                if (rendering_info.pColorAttachments[i].imageView == VK_NULL_HANDLE) {
                    continue;
                }
                auto view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pColorAttachments[i].imageView);
                if ((pipline_rendering_ci.colorAttachmentCount > i) &&
                    view_state->create_info.format != pipline_rendering_ci.pColorAttachmentFormats[i]) {
                    const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline(),
                                                cb_state.activeRenderPass->renderPass());
                    skip |= LogError(objlist, vuid.dynamic_rendering_color_formats,
                                     "%s: VkRenderingInfo::pColorAttachments[%" PRIu32
                                     "].imageView format (%s) must match corresponding format in "
                                     "VkPipelineRenderingCreateInfo::pColorAttachmentFormats[%" PRIu32 "] (%s)",
                                     caller, i, string_VkFormat(view_state->create_info.format), i,
                                     string_VkFormat(pipline_rendering_ci.pColorAttachmentFormats[i]));
                }
            }

            if (rendering_info.pDepthAttachment && rendering_info.pDepthAttachment->imageView != VK_NULL_HANDLE) {
                auto view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pDepthAttachment->imageView);
                if (view_state->create_info.format != pipline_rendering_ci.depthAttachmentFormat) {
                    const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline(),
                                                cb_state.activeRenderPass->renderPass());
                    skip |= LogError(objlist, vuid.dynamic_rendering_depth_format,
                                     "%s: VkRenderingInfo::pDepthAttachment->imageView format (%s) must match corresponding format "
                                     "in VkPipelineRenderingCreateInfo::depthAttachmentFormat (%s)",
                                     caller, string_VkFormat(view_state->create_info.format),
                                     string_VkFormat(pipline_rendering_ci.depthAttachmentFormat));
                }
            }

            if (rendering_info.pStencilAttachment && rendering_info.pStencilAttachment->imageView != VK_NULL_HANDLE) {
                auto view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pStencilAttachment->imageView);
                if (view_state->create_info.format != pipline_rendering_ci.stencilAttachmentFormat) {
                    const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline(),
                                                cb_state.activeRenderPass->renderPass());
                    skip |= LogError(objlist, vuid.dynamic_rendering_stencil_format,
                                     "%s: VkRenderingInfo::pStencilAttachment->imageView format (%s) must match corresponding "
                                     "format in VkPipelineRenderingCreateInfo::stencilAttachmentFormat (%s)",
                                     caller, string_VkFormat(view_state->create_info.format),
                                     string_VkFormat(pipline_rendering_ci.stencilAttachmentFormat));
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

// Validate overall state at the time of a draw call
bool CoreChecks::ValidateCmdBufDrawState(const CMD_BUFFER_STATE &cb_state, CMD_TYPE cmd_type, const bool indexed,
                                         const VkPipelineBindPoint bind_point) const {
    const DrawDispatchVuid vuid = GetDrawDispatchVuid(cmd_type);
    const char *function = CommandTypeString(cmd_type);
    const auto lv_bind_point = ConvertToLvlBindPoint(bind_point);
    const auto &last_bound = cb_state.lastBound[lv_bind_point];
    const auto *last_pipeline = last_bound.pipeline_state;

    if (nullptr == last_pipeline) {
        return LogError(cb_state.commandBuffer(), vuid.pipeline_bound,
                        "Must not call %s on this command buffer while there is no %s pipeline bound.", function,
                        string_VkPipelineBindPoint(bind_point));
    }
    const PIPELINE_STATE &pipeline = *last_pipeline;

    bool result = false;

    for (const auto &ds : last_bound.per_set) {
        if (pipeline.descriptor_buffer_mode) {
            if (ds.bound_descriptor_set && !ds.bound_descriptor_set->IsPushDescriptor()) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle(), ds.bound_descriptor_set->Handle());
                result |= LogError(objlist, vuid.descriptor_buffer_set_offset_missing,
                                   "%s: pipeline bound to %s requires a descriptor buffer but has a bound descriptor set (%s)",
                                   function, string_VkPipelineBindPoint(bind_point),
                                   report_data->FormatHandle(ds.bound_descriptor_set->Handle()).c_str());
                break;
            }

        } else {
            if (ds.bound_descriptor_buffer.has_value()) {
                const LogObjectList objlist(cb_state.Handle(), pipeline.Handle());
                result |= LogError(objlist, vuid.descriptor_buffer_bit_not_set,
                                   "%s: pipeline bound to %s requires a descriptor set but has a bound descriptor buffer"
                                   " (index=%" PRIu32 " offset=%" PRIu64 ")",
                                   function, string_VkPipelineBindPoint(bind_point), ds.bound_descriptor_buffer->index,
                                   ds.bound_descriptor_buffer->offset);
                break;
            }
        }
    }

    if (VK_PIPELINE_BIND_POINT_GRAPHICS == bind_point) {
        result |= ValidateDrawDynamicState(cb_state, pipeline, cmd_type);
        result |= ValidatePipelineDrawtimeState(last_bound, cb_state, cmd_type, pipeline);

        if (indexed && !cb_state.index_buffer_binding.bound()) {
            return LogError(cb_state.commandBuffer(), vuid.index_binding,
                            "%s: Index buffer object has not been bound to this command buffer.", function);
        }

        if (cb_state.activeRenderPass && cb_state.activeFramebuffer) {
            // Verify attachments for unprotected/protected command buffer.
            if (enabled_features.core11.protectedMemory == VK_TRUE && cb_state.active_attachments) {
                uint32_t i = 0;
                for (const auto &view_state : *cb_state.active_attachments.get()) {
                    const auto &subpass = cb_state.active_subpasses->at(i);
                    if (subpass.used && view_state && !view_state->Destroyed()) {
                        std::string image_desc = "Image is ";
                        image_desc.append(string_VkImageUsageFlagBits(subpass.usage));
                        // Because inputAttachment is read only, it doesn't need to care protected command buffer case.
                        // Some CMD_TYPE could not be protected. See VUID 02711.
                        if (subpass.usage != VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT &&
                            vuid.protected_command_buffer != kVUIDUndefined) {
                            result |= ValidateUnprotectedImage(cb_state, *view_state->image_state, function,
                                                               vuid.protected_command_buffer, image_desc.c_str());
                        }
                        result |= ValidateProtectedImage(cb_state, *view_state->image_state, function,
                                                         vuid.unprotected_command_buffer, image_desc.c_str());
                    }
                    ++i;
                }
            }
        }
    }
    // Now complete other state checks
    string error_string;
    auto const &pipeline_layout = pipeline.PipelineLayoutState();

    // Check if the current pipeline is compatible for the maximum used set with the bound sets.
    if (!pipeline.descriptor_buffer_mode) {
        if (!pipeline.active_slots.empty() && !IsBoundSetCompat(pipeline.max_active_slot, last_bound, *pipeline_layout)) {
            LogObjectList objlist(pipeline.pipeline());
            const auto layouts = pipeline.PipelineLayoutStateUnion();
            std::ostringstream pipe_layouts_log;
            if (layouts.size() > 1) {
                pipe_layouts_log << "a union of layouts [ ";
                for (const auto &layout : layouts) {
                    objlist.add(layout->layout());
                    pipe_layouts_log << report_data->FormatHandle(layout->layout()) << " ";
                }
                pipe_layouts_log << "]";
            } else {
                pipe_layouts_log << report_data->FormatHandle(layouts.front()->layout());
            }
            objlist.add(last_bound.pipeline_layout);
            result |= LogError(objlist, vuid.compatible_pipeline,
                               "%s(): The %s (created with %s) statically uses descriptor set (index #%" PRIu32
                               ") which is not compatible with the currently bound descriptor set's pipeline layout (%s)",
                               function, report_data->FormatHandle(pipeline.pipeline()).c_str(), pipe_layouts_log.str().c_str(),
                               pipeline.max_active_slot, report_data->FormatHandle(last_bound.pipeline_layout).c_str());
        } else {
            // if the bound set is not copmatible, the rest will just be extra redundant errors
            for (const auto &set_binding_pair : pipeline.active_slots) {
                uint32_t set_index = set_binding_pair.first;
                const auto set_info = last_bound.per_set[set_index];
                if (!set_info.bound_descriptor_set) {
                    result |= LogError(cb_state.commandBuffer(), vuid.compatible_pipeline,
                                       "%s(): %s uses set #%" PRIu32 " but that set is not bound.", function,
                                       report_data->FormatHandle(pipeline.pipeline()).c_str(), set_index);
                } else if (!VerifySetLayoutCompatibility(*set_info.bound_descriptor_set, *pipeline_layout, set_index,
                                                         error_string)) {
                    // Set is bound but not compatible w/ overlapping pipeline_layout from PSO
                    VkDescriptorSet set_handle = set_info.bound_descriptor_set->GetSet();
                    const LogObjectList objlist(set_handle, pipeline_layout->layout());
                    result |= LogError(objlist, vuid.compatible_pipeline,
                                       "%s(): %s bound as set #%u is not compatible with overlapping %s due to: %s", function,
                                       report_data->FormatHandle(set_handle).c_str(), set_index,
                                       report_data->FormatHandle(pipeline_layout->layout()).c_str(), error_string.c_str());
                } else {  // Valid set is bound and layout compatible, validate that it's updated
                    // Pull the set node
                    const auto *descriptor_set = set_info.bound_descriptor_set.get();
                    assert(descriptor_set);
                    // Validate the draw-time state for this descriptor set
                    std::string err_str;
                    // For the "bindless" style resource usage with many descriptors, need to optimize command <-> descriptor
                    // binding validation. Take the requested binding set and prefilter it to eliminate redundant validation checks.
                    // Here, the currently bound pipeline determines whether an image validation check is redundant...
                    // for images are the "req" portion of the binding_req is indirectly (but tightly) coupled to the pipeline.
                    cvdescriptorset::PrefilterBindRequestMap reduced_map(*descriptor_set, set_binding_pair.second);
                    const auto &binding_req_map = reduced_map.FilteredMap(cb_state, pipeline);

                    // We can skip validating the descriptor set if "nothing" has changed since the last validation.
                    // Same set, no image layout changes, and same "pipeline state" (binding_req_map). If there are
                    // any dynamic descriptors, always revalidate rather than caching the values. We currently only
                    // apply this optimization if IsManyDescriptors is true, to avoid the overhead of copying the
                    // binding_req_map which could potentially be expensive.
                    bool descriptor_set_changed =
                        !reduced_map.IsManyDescriptors() ||
                        // Revalidate each time if the set has dynamic offsets
                        set_info.dynamicOffsets.size() > 0 ||
                        // Revalidate if descriptor set (or contents) has changed
                        set_info.validated_set != descriptor_set ||
                        set_info.validated_set_change_count != descriptor_set->GetChangeCount() ||
                        (!disabled[image_layout_validation] &&
                         set_info.validated_set_image_layout_change_count != cb_state.image_layout_change_count);
                    bool need_validate =
                        descriptor_set_changed ||
                        // Revalidate if previous bindingReqMap doesn't include new bindingReqMap
                        !std::includes(set_info.validated_set_binding_req_map.begin(), set_info.validated_set_binding_req_map.end(),
                                       binding_req_map.begin(), binding_req_map.end());

                    if (need_validate) {
                        if (!descriptor_set_changed && reduced_map.IsManyDescriptors()) {
                            // Only validate the bindings that haven't already been validated
                            BindingReqMap delta_reqs;
                            std::set_difference(binding_req_map.begin(), binding_req_map.end(),
                                                set_info.validated_set_binding_req_map.begin(),
                                                set_info.validated_set_binding_req_map.end(),
                                                vvl::insert_iterator<BindingReqMap>(delta_reqs, delta_reqs.begin()));
                            result |=
                                ValidateDrawState(*descriptor_set, delta_reqs, set_info.dynamicOffsets, cb_state, function, vuid);
                        } else {
                            result |= ValidateDrawState(*descriptor_set, binding_req_map, set_info.dynamicOffsets, cb_state,
                                                        function, vuid);
                        }
                    }
                }
            }
        }
    }

    // Verify if push constants have been set
    // NOTE: Currently not checking whether active push constants are compatible with the active pipeline, nor whether the
    //       "life times" of push constants are correct.
    //       Discussion on validity of these checks can be found at https://gitlab.khronos.org/vulkan/vulkan/-/issues/2602.
    if (!cb_state.push_constant_data_ranges || (pipeline_layout->push_constant_ranges == cb_state.push_constant_data_ranges)) {
        for (const auto &stage : pipeline.stage_state) {
            const auto *push_constants =
                stage.module_state->FindEntrypointPushConstant(stage.create_info->pName, stage.create_info->stage);
            if (!push_constants || !push_constants->IsUsed()) {
                continue;
            }

            // Edge case where if the shader is using push constants statically and there never was a vkCmdPushConstants
            if (!cb_state.push_constant_data_ranges && !enabled_features.core13.maintenance4) {
                const LogObjectList objlist(cb_state.commandBuffer(), pipeline_layout->layout(), pipeline.pipeline());
                result |= LogError(objlist, vuid.push_constants_set,
                                   "%s(): Shader in %s uses push-constant statically but vkCmdPushConstants was not called yet for "
                                   "pipeline layout %s.",
                                   function, string_VkShaderStageFlags(stage.stage_flag).c_str(),
                                   report_data->FormatHandle(pipeline_layout->layout()).c_str());
            }

            const auto it = cb_state.push_constant_data_update.find(stage.stage_flag);
            if (it == cb_state.push_constant_data_update.end()) {
                // This error has been printed in ValidatePushConstantUsage.
                break;
            }
        }
    }

    return result;
}

bool CoreChecks::MatchSampleLocationsInfo(const VkSampleLocationsInfoEXT *pSampleLocationsInfo1,
                                          const VkSampleLocationsInfoEXT *pSampleLocationsInfo2) const {
    if (pSampleLocationsInfo1->sampleLocationsPerPixel != pSampleLocationsInfo2->sampleLocationsPerPixel ||
        pSampleLocationsInfo1->sampleLocationGridSize.width != pSampleLocationsInfo2->sampleLocationGridSize.width ||
        pSampleLocationsInfo1->sampleLocationGridSize.height != pSampleLocationsInfo2->sampleLocationGridSize.height ||
        pSampleLocationsInfo1->sampleLocationsCount != pSampleLocationsInfo2->sampleLocationsCount) {
        return false;
    }
    for (uint32_t i = 0; i < pSampleLocationsInfo1->sampleLocationsCount; ++i) {
        if (pSampleLocationsInfo1->pSampleLocations[i].x != pSampleLocationsInfo2->pSampleLocations[i].x ||
            pSampleLocationsInfo1->pSampleLocations[i].y != pSampleLocationsInfo2->pSampleLocations[i].y) {
            return false;
        }
    }
    return true;
}

bool CoreChecks::ValidateIndirectCmd(const CMD_BUFFER_STATE &cb_state, const BUFFER_STATE &buffer_state, CMD_TYPE cmd_type) const {
    bool skip = false;
    const DrawDispatchVuid vuid = GetDrawDispatchVuid(cmd_type);
    const char *caller_name = CommandTypeString(cmd_type);

    skip |= ValidateMemoryIsBoundToBuffer(cb_state.commandBuffer(), buffer_state, caller_name, vuid.indirect_contiguous_memory);
    skip |= ValidateBufferUsageFlags(cb_state.commandBuffer(), buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     vuid.indirect_buffer_bit, caller_name, "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    if (cb_state.unprotected == false) {
        skip |= LogError(cb_state.Handle(), vuid.indirect_protected_cb,
                         "%s: Indirect commands can't be used in protected command buffers.", caller_name);
    }
    return skip;
}

bool CoreChecks::ValidateIndirectCountCmd(const CMD_BUFFER_STATE &cb_state, const BUFFER_STATE &count_buffer_state,
                                          VkDeviceSize count_buffer_offset, CMD_TYPE cmd_type) const {
    bool skip = false;
    const DrawDispatchVuid vuid = GetDrawDispatchVuid(cmd_type);
    const char *caller_name = CommandTypeString(cmd_type);

    skip |= ValidateMemoryIsBoundToBuffer(cb_state.commandBuffer(), count_buffer_state, caller_name,
                                          vuid.indirect_count_contiguous_memory);
    skip |= ValidateBufferUsageFlags(cb_state.commandBuffer(), count_buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true,
                                     vuid.indirect_count_buffer_bit, caller_name, "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    if (count_buffer_offset + sizeof(uint32_t) > count_buffer_state.createInfo.size) {
        const LogObjectList objlist(cb_state.Handle(), count_buffer_state.Handle());
        skip |= LogError(objlist, vuid.indirect_count_offset,
                         "%s: countBufferOffset (%" PRIu64 ") + sizeof(uint32_t) is greater than the buffer size of %" PRIu64 ".",
                         caller_name, count_buffer_offset, count_buffer_state.createInfo.size);
    }
    return skip;
}

// For given obj node, if it is use, flag a validation error and return callback result, else return false
bool CoreChecks::ValidateObjectNotInUse(const BASE_NODE *obj_node, const char *caller_name, const char *error_code) const {
    if (disabled[object_in_use]) return false;
    auto obj_struct = obj_node->Handle();
    bool skip = false;
    if (obj_node->InUse()) {
        skip |= LogError(device, error_code, "Cannot call %s on %s that is currently in use by a command buffer.", caller_name,
                         report_data->FormatHandle(obj_struct).c_str());
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
