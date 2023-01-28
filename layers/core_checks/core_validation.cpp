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

// these templates are defined in buffer_validation.cpp so we need to pull in the explicit instantiations from there
extern template void CoreChecks::TransitionImageLayouts(CMD_BUFFER_STATE *cb_state, uint32_t barrier_count,
                                                        const VkImageMemoryBarrier *barrier);
extern template void CoreChecks::TransitionImageLayouts(CMD_BUFFER_STATE *cb_state, uint32_t barrier_count,
                                                        const VkImageMemoryBarrier2KHR *barrier);
extern template bool CoreChecks::ValidateImageBarrierAttachment(const Location &loc, CMD_BUFFER_STATE const *cb_state,
                                                                const FRAMEBUFFER_STATE *framebuffer, uint32_t active_subpass,
                                                                const safe_VkSubpassDescription2 &sub_desc,
                                                                const VkRenderPass rp_handle,
                                                                const VkImageMemoryBarrier &img_barrier,
                                                                const CMD_BUFFER_STATE *primary_cb_state) const;
extern template bool CoreChecks::ValidateImageBarrierAttachment(const Location &loc, CMD_BUFFER_STATE const *cb_state,
                                                                const FRAMEBUFFER_STATE *framebuffer, uint32_t active_subpass,
                                                                const safe_VkSubpassDescription2 &sub_desc,
                                                                const VkRenderPass rp_handle,
                                                                const VkImageMemoryBarrier2KHR &img_barrier,
                                                                const CMD_BUFFER_STATE *primary_cb_state) const;

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
                                                layer_data::insert_iterator<BindingReqMap>(delta_reqs, delta_reqs.begin()));
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

bool CoreChecks::ValidateCmdQueueFlags(const CMD_BUFFER_STATE &cb_state, const char *caller_name, VkQueueFlags required_flags,
                                       const char *error_code) const {
    auto pool = cb_state.command_pool;
    if (pool) {
        const uint32_t queue_family_index = pool->queueFamilyIndex;
        const VkQueueFlags queue_flags = physical_device_state->queue_family_properties[queue_family_index].queueFlags;
        if (!(required_flags & queue_flags)) {
            string required_flags_string;
            for (auto flag : {VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_SPARSE_BINDING_BIT,
                              VK_QUEUE_PROTECTED_BIT}) {
                if (flag & required_flags) {
                    if (required_flags_string.size()) {
                        required_flags_string += " or ";
                    }
                    required_flags_string += string_VkQueueFlagBits(flag);
                }
            }
            return LogError(cb_state.commandBuffer(), error_code,
                            "%s(): Called in command buffer %s which was allocated from the command pool %s which was created with "
                            "queueFamilyIndex %u which doesn't contain the required %s capability flags.",
                            caller_name, report_data->FormatHandle(cb_state.commandBuffer()).c_str(),
                            report_data->FormatHandle(pool->commandPool()).c_str(), queue_family_index,
                            required_flags_string.c_str());
        }
    }
    return false;
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

static char const *GetCauseStr(VulkanTypedHandle obj) {
    if (obj.type == kVulkanObjectTypeDescriptorSet) return "destroyed or updated";
    if (obj.type == kVulkanObjectTypeCommandBuffer) return "destroyed or rerecorded";
    return "destroyed";
}

bool CoreChecks::ReportInvalidCommandBuffer(const CMD_BUFFER_STATE &cb_state, const char *call_source) const {
    bool skip = false;
    for (const auto &entry : cb_state.broken_bindings) {
        const auto &obj = entry.first;
        const char *cause_str = GetCauseStr(obj);
        string vuid;
        std::ostringstream str;
        str << kVUID_Core_DrawState_InvalidCommandBuffer << "-" << object_string[obj.type];
        vuid = str.str();
        auto objlist = entry.second;  // intentional copy
        objlist.add(cb_state.commandBuffer());
        skip |= LogError(objlist, vuid, "You are adding %s to %s that is invalid because bound %s was %s.", call_source,
                         report_data->FormatHandle(cb_state.commandBuffer()).c_str(), report_data->FormatHandle(obj).c_str(),
                         cause_str);
    }
    return skip;
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

bool CoreChecks::ValidateCommandBufferSimultaneousUse(const Location &loc, const CMD_BUFFER_STATE &cb_state,
                                                      int current_submit_count) const {
    using sync_vuid_maps::GetQueueSubmitVUID;
    using sync_vuid_maps::SubmitError;

    bool skip = false;
    if ((cb_state.InUse() || current_submit_count > 1) &&
        !(cb_state.beginInfo.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)) {
        const auto &vuid = sync_vuid_maps::GetQueueSubmitVUID(loc, SubmitError::kCmdNotSimultaneous);

        skip |= LogError(device, vuid, "%s %s is already in use and is not marked for simultaneous use.", loc.Message().c_str(),
                         report_data->FormatHandle(cb_state.commandBuffer()).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateCommandBufferState(const CMD_BUFFER_STATE &cb_state, const char *call_source, int current_submit_count,
                                            const char *vu_id) const {
    bool skip = false;
    if (disabled[command_buffer_state]) return skip;
    // Validate ONE_TIME_SUBMIT_BIT CB is not being submitted more than once
    if ((cb_state.beginInfo.flags & VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) &&
        (cb_state.submitCount + current_submit_count > 1)) {
        skip |= LogError(cb_state.commandBuffer(), kVUID_Core_DrawState_CommandBufferSingleSubmitViolation,
                         "%s was begun w/ VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT set, but has been submitted 0x%" PRIxLEAST64
                         "times.",
                         report_data->FormatHandle(cb_state.commandBuffer()).c_str(), cb_state.submitCount + current_submit_count);
    }

    // Validate that cmd buffers have been updated
    switch (cb_state.state) {
        case CB_INVALID_INCOMPLETE:
        case CB_INVALID_COMPLETE:
            skip |= ReportInvalidCommandBuffer(cb_state, call_source);
            break;

        case CB_NEW:
            skip |= LogError(cb_state.commandBuffer(), vu_id, "%s used in the call to %s is unrecorded and contains no commands.",
                             report_data->FormatHandle(cb_state.commandBuffer()).c_str(), call_source);
            break;

        case CB_RECORDING:
            skip |= LogError(cb_state.commandBuffer(), kVUID_Core_DrawState_NoEndCommandBuffer,
                             "You must call vkEndCommandBuffer() on %s before this call to %s!",
                             report_data->FormatHandle(cb_state.commandBuffer()).c_str(), call_source);
            break;

        default: /* recorded */
            break;
    }
    return skip;
}

// Check that the queue family index of 'queue' matches one of the entries in pQueueFamilyIndices
bool CoreChecks::ValidImageBufferQueue(const CMD_BUFFER_STATE &cb_state, const VulkanTypedHandle &object, uint32_t queueFamilyIndex,
                                       uint32_t count, const uint32_t *indices) const {
    bool found = false;
    bool skip = false;
    for (uint32_t i = 0; i < count; i++) {
        if (indices[i] == queueFamilyIndex) {
            found = true;
            break;
        }
    }

    if (!found) {
        const LogObjectList objlist(cb_state.commandBuffer(), object);
        skip = LogError(objlist, "VUID-vkQueueSubmit-pSubmits-04626",
                        "vkQueueSubmit: %s contains %s which was not created allowing concurrent access to "
                        "this queue family %d.",
                        report_data->FormatHandle(cb_state.commandBuffer()).c_str(), report_data->FormatHandle(object).c_str(),
                        queueFamilyIndex);
    }
    return skip;
}

// Validate that queueFamilyIndices of primary command buffers match this queue
// Secondary command buffers were previously validated in vkCmdExecuteCommands().
bool CoreChecks::ValidateQueueFamilyIndices(const Location &loc, const CMD_BUFFER_STATE &cb_state, VkQueue queue) const {
    using sync_vuid_maps::GetQueueSubmitVUID;
    using sync_vuid_maps::SubmitError;
    bool skip = false;
    auto pool = cb_state.command_pool;
    auto queue_state = Get<QUEUE_STATE>(queue);

    if (pool && queue_state) {
        if (pool->queueFamilyIndex != queue_state->queueFamilyIndex) {
            const LogObjectList objlist(cb_state.commandBuffer(), queue);
            const auto &vuid = GetQueueSubmitVUID(loc, SubmitError::kCmdWrongQueueFamily);
            skip |= LogError(objlist, vuid,
                             "%s Primary %s created in queue family %d is being submitted on %s "
                             "from queue family %d.",
                             loc.Message().c_str(), report_data->FormatHandle(cb_state.commandBuffer()).c_str(),
                             pool->queueFamilyIndex, report_data->FormatHandle(queue).c_str(), queue_state->queueFamilyIndex);
        }

        // Ensure that any bound images or buffers created with SHARING_MODE_CONCURRENT have access to the current queue family
        for (const auto &base_node : cb_state.object_bindings) {
            switch (base_node->Type()) {
                case kVulkanObjectTypeImage: {
                    auto image_state = static_cast<const IMAGE_STATE *>(base_node.get());
                    if (image_state && image_state->createInfo.sharingMode == VK_SHARING_MODE_CONCURRENT) {
                        skip |= ValidImageBufferQueue(cb_state, image_state->Handle(), queue_state->queueFamilyIndex,
                                                      image_state->createInfo.queueFamilyIndexCount,
                                                      image_state->createInfo.pQueueFamilyIndices);
                    }
                    break;
                }
                case kVulkanObjectTypeBuffer: {
                    auto buffer_state = static_cast<const BUFFER_STATE *>(base_node.get());
                    if (buffer_state && buffer_state->createInfo.sharingMode == VK_SHARING_MODE_CONCURRENT) {
                        skip |= ValidImageBufferQueue(cb_state, buffer_state->Handle(), queue_state->queueFamilyIndex,
                                                      buffer_state->createInfo.queueFamilyIndexCount,
                                                      buffer_state->createInfo.pQueueFamilyIndices);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidatePrimaryCommandBufferState(
    const Location &loc, const CMD_BUFFER_STATE &cb_state, int current_submit_count,
    QFOTransferCBScoreboards<QFOImageTransferBarrier> *qfo_image_scoreboards,
    QFOTransferCBScoreboards<QFOBufferTransferBarrier> *qfo_buffer_scoreboards) const {
    using sync_vuid_maps::GetQueueSubmitVUID;
    using sync_vuid_maps::SubmitError;

    // Track in-use for resources off of primary and any secondary CBs
    bool skip = false;

    if (cb_state.createInfo.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
        const auto &vuid = GetQueueSubmitVUID(loc, SubmitError::kSecondaryCmdInSubmit);
        skip |=
            LogError(cb_state.commandBuffer(), vuid, "%s Command buffer %s must be allocated with VK_COMMAND_BUFFER_LEVEL_PRIMARY.",
                     loc.Message().c_str(), report_data->FormatHandle(cb_state.commandBuffer()).c_str());
    } else {
        for (const auto *sub_cb : cb_state.linkedCommandBuffers) {
            skip |= ValidateQueuedQFOTransfers(*sub_cb, qfo_image_scoreboards, qfo_buffer_scoreboards);
            // TODO: replace with InvalidateCommandBuffers() at recording.
            if ((sub_cb->primaryCommandBuffer != cb_state.commandBuffer()) &&
                !(sub_cb->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)) {
                const auto &vuid = GetQueueSubmitVUID(loc, SubmitError::kSecondaryCmdNotSimultaneous);
                const LogObjectList objlist(device, cb_state.commandBuffer(), sub_cb->commandBuffer(),
                                            sub_cb->primaryCommandBuffer);
                skip |= LogError(objlist, vuid,
                                 "%s %s was submitted with secondary %s but that buffer has subsequently been bound to "
                                 "primary %s and it does not have VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set.",
                                 loc.Message().c_str(), report_data->FormatHandle(cb_state.commandBuffer()).c_str(),
                                 report_data->FormatHandle(sub_cb->commandBuffer()).c_str(),
                                 report_data->FormatHandle(sub_cb->primaryCommandBuffer).c_str());
            }

            if (sub_cb->state != CB_RECORDED) {
                const char *const finished_cb_vuid = (loc.function == Func::vkQueueSubmit)
                                                         ? "VUID-vkQueueSubmit-pCommandBuffers-00072"
                                                         : "VUID-vkQueueSubmit2-commandBuffer-03876";
                const LogObjectList objlist(device, cb_state.commandBuffer(), sub_cb->commandBuffer(),
                                            sub_cb->primaryCommandBuffer);
                skip |= LogError(objlist, finished_cb_vuid,
                                 "%s: Secondary command buffer %s is not in a valid (pending or executable) state.",
                                 loc.StringFunc().c_str(), report_data->FormatHandle(sub_cb->commandBuffer()).c_str());
            }
        }
    }

    // If USAGE_SIMULTANEOUS_USE_BIT not set then CB cannot already be executing on device
    skip |= ValidateCommandBufferSimultaneousUse(loc, cb_state, current_submit_count);

    skip |= ValidateQueuedQFOTransfers(cb_state, qfo_image_scoreboards, qfo_buffer_scoreboards);

    const char *const vuid = (loc.function == Func::vkQueueSubmit) ? "VUID-vkQueueSubmit-pCommandBuffers-00070"
                                                                   : "VUID-vkQueueSubmit2-commandBuffer-03874";
    skip |= ValidateCommandBufferState(cb_state, loc.StringFunc().c_str(), current_submit_count, vuid);
    return skip;
}

void CoreChecks::PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                           VkResult result) {
    StateTracker::PostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, result);

    if (result != VK_SUCCESS) return;
    // The triply nested for duplicates that in the StateTracker, but avoids the need for two additional callbacks.
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo *submit = &pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit->commandBufferCount; i++) {
            auto cb_state = GetWrite<CMD_BUFFER_STATE>(submit->pCommandBuffers[i]);
            if (cb_state) {
                for (auto *secondary_cmd_buffer : cb_state->linkedCommandBuffers) {
                    UpdateCmdBufImageLayouts(secondary_cmd_buffer);
                    RecordQueuedQFOTransfers(secondary_cmd_buffer);
                }
                UpdateCmdBufImageLayouts(cb_state.get());
                RecordQueuedQFOTransfers(cb_state.get());
            }
        }
    }
}

void CoreChecks::RecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits, VkFence fence,
                                    VkResult result) {
    if (result != VK_SUCCESS) return;
    // The triply nested for duplicates that in the StateTracker, but avoids the need for two additional callbacks.
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo2KHR *submit = &pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit->commandBufferInfoCount; i++) {
            auto cb_state = GetWrite<CMD_BUFFER_STATE>(submit->pCommandBufferInfos[i].commandBuffer);
            if (cb_state) {
                for (auto *secondaryCmdBuffer : cb_state->linkedCommandBuffers) {
                    UpdateCmdBufImageLayouts(secondaryCmdBuffer);
                    RecordQueuedQFOTransfers(secondaryCmdBuffer);
                }
                UpdateCmdBufImageLayouts(cb_state.get());
                RecordQueuedQFOTransfers(cb_state.get());
            }
        }
    }
}

void CoreChecks::PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits, VkFence fence,
                                               VkResult result) {
    StateTracker::PostCallRecordQueueSubmit2KHR(queue, submitCount, pSubmits, fence, result);
    RecordQueueSubmit2(queue, submitCount, pSubmits, fence, result);
}

void CoreChecks::PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                                            VkResult result) {
    StateTracker::PostCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence, result);
    RecordQueueSubmit2(queue, submitCount, pSubmits, fence, result);
}

struct CommandBufferSubmitState {
    const CoreChecks *core;
    const QUEUE_STATE *queue_state;
    QFOTransferCBScoreboards<QFOImageTransferBarrier> qfo_image_scoreboards;
    QFOTransferCBScoreboards<QFOBufferTransferBarrier> qfo_buffer_scoreboards;
    vector<VkCommandBuffer> current_cmds;
    GlobalImageLayoutMap overlay_image_layout_map;
    QueryMap local_query_to_state_map;
    EventToStageMap local_event_to_stage_map;

    CommandBufferSubmitState(const CoreChecks *c, const char *func, const QUEUE_STATE *q) : core(c), queue_state(q) {}

    bool Validate(const core_error::Location &loc, const CMD_BUFFER_STATE &cb_state, uint32_t perf_pass) {
        bool skip = false;
        skip |= core->ValidateCmdBufImageLayouts(loc, cb_state, overlay_image_layout_map);
        auto cmd = cb_state.commandBuffer();
        current_cmds.push_back(cmd);
        skip |= core->ValidatePrimaryCommandBufferState(loc, cb_state,
                                                        static_cast<int>(std::count(current_cmds.begin(), current_cmds.end(), cmd)),
                                                        &qfo_image_scoreboards, &qfo_buffer_scoreboards);
        skip |= core->ValidateQueueFamilyIndices(loc, cb_state, queue_state->Queue());

        for (const auto &descriptor_set : cb_state.validate_descriptorsets_in_queuesubmit) {
            auto set_node = core->Get<cvdescriptorset::DescriptorSet>(descriptor_set.first);
            if (!set_node) {
                continue;
            }
            for (const auto &cmd_info : descriptor_set.second) {
                // dynamic data isn't allowed in UPDATE_AFTER_BIND, so dynamicOffsets is always empty.
                std::vector<uint32_t> dynamic_offsets;
                std::optional<layer_data::unordered_map<VkImageView, VkImageLayout>> checked_layouts;

                std::string function = loc.StringFunc();
                function += ", ";
                function += CommandTypeString(cmd_info.cmd_type);
                CoreChecks::DescriptorContext context{function.c_str(),
                                                      core->GetDrawDispatchVuid(cmd_info.cmd_type),
                                                      cb_state,
                                                      *set_node,
                                                      cmd_info.framebuffer,
                                                      false,  // This is submit time not record time...
                                                      dynamic_offsets,
                                                      checked_layouts};

                for (const auto &binding_info : cmd_info.binding_infos) {
                    std::string error;
                    if (set_node->GetTotalDescriptorCount() > cvdescriptorset::PrefilterBindRequestMap::kManyDescriptors_) {
                        context.checked_layouts.emplace();
                    }
                    const auto *binding = set_node->GetBinding(binding_info.first);
                    skip |= core->ValidateDescriptorSetBindingData(context, binding_info, *binding);
                }
            }
        }

        // Potential early exit here as bad object state may crash in delayed function calls
        if (skip) {
            return true;
        }

        // Call submit-time functions to validate or update local mirrors of state (to preserve const-ness at validate time)
        for (auto &function : cb_state.queue_submit_functions) {
            skip |= function(*core, *queue_state, cb_state);
        }
        for (auto &function : cb_state.eventUpdates) {
            skip |= function(const_cast<CMD_BUFFER_STATE &>(cb_state), /*do_validate*/ true, &local_event_to_stage_map);
        }
        VkQueryPool first_perf_query_pool = VK_NULL_HANDLE;
        for (auto &function : cb_state.queryUpdates) {
            skip |= function(const_cast<CMD_BUFFER_STATE &>(cb_state), /*do_validate*/ true, first_perf_query_pool, perf_pass,
                             &local_query_to_state_map);
        }
        for (const auto &it : cb_state.video_session_updates) {
            auto video_session_state = core->Get<VIDEO_SESSION_STATE>(it.first);
            VideoSessionDeviceState local_state = video_session_state->DeviceStateCopy();
            for (const auto &function : it.second) {
                skip |= function(core, video_session_state.get(), local_state, /*do_validate*/ true);
            }
        }
        return skip;
    }
};

bool CoreChecks::PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits,
                                            VkFence fence) const {
    auto fence_state = Get<FENCE_STATE>(fence);
    bool skip = ValidateFenceForSubmit(fence_state.get(), "VUID-vkQueueSubmit-fence-00064", "VUID-vkQueueSubmit-fence-00063",
                                       "vkQueueSubmit()");
    if (skip) {
        return true;
    }
    auto queue_state = Get<QUEUE_STATE>(queue);
    CommandBufferSubmitState cb_submit_state(this, "vkQueueSubmit()", queue_state.get());
    SemaphoreSubmitState sem_submit_state(this, queue,
                                          physical_device_state->queue_family_properties[queue_state->queueFamilyIndex].queueFlags);

    // Now verify each individual submit
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo &submit = pSubmits[submit_idx];
        const auto perf_submit = LvlFindInChain<VkPerformanceQuerySubmitInfoKHR>(submit.pNext);
        uint32_t perf_pass = perf_submit ? perf_submit->counterPassIndex : 0;

        Location loc(Func::vkQueueSubmit, Struct::VkSubmitInfo, Field::pSubmits, submit_idx);
        bool suspended_render_pass_instance = false;
        for (uint32_t i = 0; i < submit.commandBufferCount; i++) {
            auto cb_state = GetRead<CMD_BUFFER_STATE>(submit.pCommandBuffers[i]);
            if (cb_state) {
                skip |= cb_submit_state.Validate(loc.dot(Field::pCommandBuffers, i), *cb_state, perf_pass);

                // Validate flags for dynamic rendering
                if (suspended_render_pass_instance && cb_state->hasRenderPassInstance && !cb_state->resumesRenderPassInstance) {
                    skip |= LogError(queue, "VUID-VkSubmitInfo-pCommandBuffers-06016",
                                     "pSubmits[%" PRIu32 "] has a suspended render pass instance, but pCommandBuffers[%" PRIu32
                                     "] has its own render pass instance that does not resume it.",
                                     submit_idx, i);
                }
                if (cb_state->resumesRenderPassInstance) {
                    if (!suspended_render_pass_instance) {
                        skip |= LogError(queue, "VUID-VkSubmitInfo-pCommandBuffers-06193",
                                         "pSubmits[%" PRIu32 "]->pCommandBuffers[%" PRIu32
                                         "] resumes a render pass instance, but there is no suspended render pass instance.",
                                         submit_idx, i);
                    }
                    suspended_render_pass_instance = false;
                }
                if (cb_state->suspendsRenderPassInstance) {
                    suspended_render_pass_instance = true;
                }
            }
        }
        // Renderpass should not be in suspended state after the final cmdbuf
        if (suspended_render_pass_instance) {
            skip |= LogError(queue, "VUID-VkSubmitInfo-pCommandBuffers-06014",
                             "pSubmits[%" PRIu32 "] has a suspended render pass instance that was not resumed.", submit_idx);
        }
        skip |= ValidateSemaphoresForSubmit(sem_submit_state, submit, loc);

        auto chained_device_group_struct = LvlFindInChain<VkDeviceGroupSubmitInfo>(submit.pNext);
        if (chained_device_group_struct && chained_device_group_struct->commandBufferCount > 0) {
            for (uint32_t i = 0; i < chained_device_group_struct->commandBufferCount; ++i) {
                const LogObjectList objlist(queue);
                skip |= ValidateDeviceMaskToPhysicalDeviceCount(chained_device_group_struct->pCommandBufferDeviceMasks[i], objlist,
                                                                "VUID-VkDeviceGroupSubmitInfo-pCommandBufferDeviceMasks-00086");
            }
            if (chained_device_group_struct->signalSemaphoreCount != submit.signalSemaphoreCount) {
                skip |= LogError(queue, "VUID-VkDeviceGroupSubmitInfo-signalSemaphoreCount-00084",
                                 "pSubmits[%" PRIu32 "] signalSemaphoreCount (%" PRIu32
                                 ") is different than signalSemaphoreCount (%" PRIu32
                                 ") of the VkDeviceGroupSubmitInfo in its pNext chain",
                                 submit_idx, submit.signalSemaphoreCount, chained_device_group_struct->signalSemaphoreCount);
            }
            if (chained_device_group_struct->waitSemaphoreCount != submit.waitSemaphoreCount) {
                skip |=
                    LogError(queue, "VUID-VkDeviceGroupSubmitInfo-waitSemaphoreCount-00082",
                             "pSubmits[%" PRIu32 "] waitSemaphoreCount (%" PRIu32 ") is different than waitSemaphoreCount (%" PRIu32
                             ") of the VkDeviceGroupSubmitInfo in its pNext chain",
                             submit_idx, submit.waitSemaphoreCount, chained_device_group_struct->waitSemaphoreCount);
            }
            if (chained_device_group_struct->commandBufferCount != submit.commandBufferCount) {
                skip |=
                    LogError(queue, "VUID-VkDeviceGroupSubmitInfo-commandBufferCount-00083",
                             "pSubmits[%" PRIu32 "] commandBufferCount (%" PRIu32 ") is different than commandBufferCount (%" PRIu32
                             ") of the VkDeviceGroupSubmitInfo in its pNext chain",
                             submit_idx, submit.commandBufferCount, chained_device_group_struct->commandBufferCount);
            }
        }

        auto protected_submit_info = LvlFindInChain<VkProtectedSubmitInfo>(submit.pNext);
        if (protected_submit_info) {
            const bool protected_submit = protected_submit_info->protectedSubmit == VK_TRUE;
            if ((protected_submit == true) && ((queue_state->flags & VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT) == 0)) {
                skip |= LogError(queue, "VUID-vkQueueSubmit-queue-06448",
                                 "vkQueueSubmit(): pSubmits[%u] contains a protected submission to %s which was not created with "
                                 "VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT",
                                 submit_idx, report_data->FormatHandle(queue).c_str());
            }

            // Make sure command buffers are all protected or unprotected
            for (uint32_t i = 0; i < submit.commandBufferCount; i++) {
                auto cb_state = GetRead<CMD_BUFFER_STATE>(submit.pCommandBuffers[i]);
                if (cb_state) {
                    if ((cb_state->unprotected == true) && (protected_submit == true)) {
                        const LogObjectList objlist(cb_state->commandBuffer(), queue);
                        skip |= LogError(objlist, "VUID-VkSubmitInfo-pNext-04148",
                                         "vkQueueSubmit(): command buffer %s is unprotected while queue %s pSubmits[%u] has "
                                         "VkProtectedSubmitInfo:protectedSubmit set to VK_TRUE",
                                         report_data->FormatHandle(cb_state->commandBuffer()).c_str(),
                                         report_data->FormatHandle(queue).c_str(), submit_idx);
                    }
                    if ((cb_state->unprotected == false) && (protected_submit == false)) {
                        const LogObjectList objlist(cb_state->commandBuffer(), queue);
                        skip |= LogError(objlist, "VUID-VkSubmitInfo-pNext-04120",
                                         "vkQueueSubmit(): command buffer %s is protected while queue %s pSubmits[%u] has "
                                         "VkProtectedSubmitInfo:protectedSubmit set to VK_FALSE",
                                         report_data->FormatHandle(cb_state->commandBuffer()).c_str(),
                                         report_data->FormatHandle(queue).c_str(), submit_idx);
                    }
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits, VkFence fence,
                                      bool is_2khr) const {
    auto pFence = Get<FENCE_STATE>(fence);
    const char *func_name = is_2khr ? "vkQueueSubmit2KHR()" : "vkQueueSubmit2()";
    bool skip =
        ValidateFenceForSubmit(pFence.get(), "VUID-vkQueueSubmit2-fence-04895", "VUID-vkQueueSubmit2-fence-04894", func_name);
    if (skip) {
        return true;
    }

    if (!enabled_features.core13.synchronization2) {
        skip |=
            LogError(queue, "VUID-vkQueueSubmit2-synchronization2-03866", "%s: Synchronization2 feature is not enabled", func_name);
    }

    auto queue_state = Get<QUEUE_STATE>(queue);
    CommandBufferSubmitState cb_submit_state(this, func_name, queue_state.get());
    SemaphoreSubmitState sem_submit_state(this, queue,
                                          physical_device_state->queue_family_properties[queue_state->queueFamilyIndex].queueFlags);

    // Now verify each individual submit
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        const VkSubmitInfo2KHR &submit = pSubmits[submit_idx];
        const auto perf_submit = LvlFindInChain<VkPerformanceQuerySubmitInfoKHR>(submit.pNext);
        uint32_t perf_pass = perf_submit ? perf_submit->counterPassIndex : 0;
        Location loc(Func::vkQueueSubmit2, Struct::VkSubmitInfo2, Field::pSubmits, submit_idx);

        skip |= ValidateSemaphoresForSubmit(sem_submit_state, submit, loc);

        const bool protected_submit = (submit.flags & VK_SUBMIT_PROTECTED_BIT_KHR) != 0;
        if ((protected_submit == true) && ((queue_state->flags & VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT)) == 0) {
            skip |= LogError(queue, "VUID-vkQueueSubmit2-queue-06447",
                             "%s: pSubmits[%u] contains a protected submission to %s which was not created with "
                             "VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT",
                             func_name, submit_idx, report_data->FormatHandle(queue).c_str());
        }

        bool suspended_render_pass_instance = false;
        for (uint32_t i = 0; i < submit.commandBufferInfoCount; i++) {
            auto info_loc = loc.dot(Field::pCommandBufferInfos, i);
            info_loc.structure = Struct::VkCommandBufferSubmitInfo;
            auto cb_state = GetRead<CMD_BUFFER_STATE>(submit.pCommandBufferInfos[i].commandBuffer);
            skip |= cb_submit_state.Validate(info_loc.dot(Field::commandBuffer), *cb_state, perf_pass);

            {
                const LogObjectList objlist(queue);
                skip |= ValidateDeviceMaskToPhysicalDeviceCount(submit.pCommandBufferInfos[i].deviceMask, queue,
                                                                "VUID-VkCommandBufferSubmitInfo-deviceMask-03891");
            }

            if (cb_state != nullptr) {
                // Make sure command buffers are all protected or unprotected
                if ((cb_state->unprotected == true) && (protected_submit == true)) {
                    const LogObjectList objlist(cb_state->commandBuffer(), queue);
                    skip |= LogError(objlist, "VUID-VkSubmitInfo2-flags-03886",
                                     "%s: command buffer %s is unprotected while queue %s pSubmits[%u] has "
                                     "VK_SUBMIT_PROTECTED_BIT_KHR set",
                                     func_name, report_data->FormatHandle(cb_state->commandBuffer()).c_str(),
                                     report_data->FormatHandle(queue).c_str(), submit_idx);
                }
                if ((cb_state->unprotected == false) && (protected_submit == false)) {
                    const LogObjectList objlist(cb_state->commandBuffer(), queue);
                    skip |= LogError(objlist, "VUID-VkSubmitInfo2-flags-03887",
                                     "%s: command buffer %s is protected while queue %s pSubmitInfos[%u] has "
                                     "VK_SUBMIT_PROTECTED_BIT_KHR not set",
                                     func_name, report_data->FormatHandle(cb_state->commandBuffer()).c_str(),
                                     report_data->FormatHandle(queue).c_str(), submit_idx);
                }

                if (suspended_render_pass_instance && cb_state->hasRenderPassInstance && !cb_state->resumesRenderPassInstance) {
                    skip |= LogError(queue, "VUID-VkSubmitInfo2KHR-commandBuffer-06012",
                                     "pSubmits[%" PRIu32 "] has a suspended render pass instance, but pCommandBuffers[%" PRIu32
                                     "] has its own render pass instance that does not resume it.",
                                     submit_idx, i);
                }
                if (cb_state->suspendsRenderPassInstance) {
                    suspended_render_pass_instance = true;
                }
                if (cb_state->resumesRenderPassInstance) {
                    if (!suspended_render_pass_instance) {
                        skip |= LogError(queue, "VUID-VkSubmitInfo2KHR-commandBuffer-06192",
                                         "pSubmits[%" PRIu32 "]->pCommandBuffers[%" PRIu32
                                         "] resumes a render pass instance, but there is no suspended render pass instance.",
                                         submit_idx, i);
                    }
                    suspended_render_pass_instance = false;
                }
            }
        }
        if (suspended_render_pass_instance) {
            skip |= LogError(queue, "VUID-VkSubmitInfo2KHR-commandBuffer-06010",
                             "pSubmits[%" PRIu32 "] has a suspended render pass instance that was not resumed.", submit_idx);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits,
                                                VkFence fence) const {
    return ValidateQueueSubmit2(queue, submitCount, pSubmits, fence, true);
}

bool CoreChecks::PreCallValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits,
                                             VkFence fence) const {
    return ValidateQueueSubmit2(queue, submitCount, pSubmits, fence, false);
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

// Verify cmdBuffer in given cb_state is not in global in-flight set, and return skip result
//  If this is a secondary command buffer, then make sure its primary is also in-flight
//  If primary is not in-flight, then remove secondary from global in-flight set
// This function is only valid at a point when cmdBuffer is being reset or freed
bool CoreChecks::CheckCommandBufferInFlight(const CMD_BUFFER_STATE *cb_state, const char *action, const char *error_code) const {
    bool skip = false;
    if (cb_state->InUse()) {
        skip |= LogError(cb_state->commandBuffer(), error_code, "Attempt to %s %s which is in use.", action,
                         report_data->FormatHandle(cb_state->commandBuffer()).c_str());
    }
    return skip;
}

// Iterate over all cmdBuffers in given commandPool and verify that each is not in use
bool CoreChecks::CheckCommandBuffersInFlight(const COMMAND_POOL_STATE *pPool, const char *action, const char *error_code) const {
    bool skip = false;
    for (auto &entry : pPool->commandBuffers) {
        auto cb_state = entry.second;
        skip |= CheckCommandBufferInFlight(cb_state, action, error_code);
    }
    return skip;
}

bool CoreChecks::PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                   const VkCommandBuffer *pCommandBuffers) const {
    bool skip = false;
    for (uint32_t i = 0; i < commandBufferCount; i++) {
        auto cb_state = GetRead<CMD_BUFFER_STATE>(pCommandBuffers[i]);
        // Delete CB information structure, and remove from commandBufferMap
        if (cb_state) {
            skip |= CheckCommandBufferInFlight(cb_state.get(), "free", "VUID-vkFreeCommandBuffers-pCommandBuffers-00047");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                                                   const VkAllocationCallbacks *pAllocator) const {
    auto framebuffer_state = Get<FRAMEBUFFER_STATE>(framebuffer);
    bool skip = false;
    if (framebuffer_state) {
        skip |=
            ValidateObjectNotInUse(framebuffer_state.get(), "vkDestroyFramebuffer", "VUID-vkDestroyFramebuffer-framebuffer-00892");
    }
    return skip;
}

bool CoreChecks::PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                                   const VkCommandBufferBeginInfo *pBeginInfo) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state) return false;
    bool skip = false;
    if (cb_state->InUse()) {
        skip |= LogError(commandBuffer, "VUID-vkBeginCommandBuffer-commandBuffer-00049",
                         "Calling vkBeginCommandBuffer() on active %s before it has completed. You must check "
                         "command buffer fence before this call.",
                         report_data->FormatHandle(commandBuffer).c_str());
    }
    if (cb_state->createInfo.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
        // Primary Command Buffer
        const VkCommandBufferUsageFlags invalid_usage =
            (VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
        if ((pBeginInfo->flags & invalid_usage) == invalid_usage) {
            skip |= LogError(commandBuffer, "VUID-vkBeginCommandBuffer-commandBuffer-02840",
                             "vkBeginCommandBuffer(): Primary %s can't have both VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT and "
                             "VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set.",
                             report_data->FormatHandle(commandBuffer).c_str());
        }
    } else {
        // Secondary Command Buffer
        const VkCommandBufferInheritanceInfo *info = pBeginInfo->pInheritanceInfo;
        if (!info) {
            skip |= LogError(commandBuffer, "VUID-vkBeginCommandBuffer-commandBuffer-00051",
                             "vkBeginCommandBuffer(): Secondary %s must have inheritance info.",
                             report_data->FormatHandle(commandBuffer).c_str());
        } else {
            auto p_inherited_rendering_info = LvlFindInChain<VkCommandBufferInheritanceRenderingInfoKHR>(info->pNext);

            if ((pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) &&
                ((api_version >= VK_API_VERSION_1_3) || enabled_features.core13.dynamicRendering)) {
                auto framebuffer = Get<FRAMEBUFFER_STATE>(info->framebuffer);
                if (framebuffer) {
                    if (framebuffer->createInfo.renderPass != info->renderPass) {
                        auto render_pass = Get<RENDER_PASS_STATE>(info->renderPass);
                        // renderPass that framebuffer was created with must be compatible with local renderPass
                        skip |= ValidateRenderPassCompatibility("framebuffer", *framebuffer->rp_state.get(), "command buffer",
                                                                *render_pass.get(), "vkBeginCommandBuffer()",
                                                                "VUID-VkCommandBufferBeginInfo-flags-00055");
                    }
                }

                if (info->renderPass != VK_NULL_HANDLE) {
                    auto render_pass = Get<RENDER_PASS_STATE>(info->renderPass);
                    if (!render_pass) {
                        skip |= LogError(commandBuffer, "VUID-VkCommandBufferBeginInfo-flags-06000",
                                         "vkBeginCommandBuffer(): Renderpass must be a valid VkRenderPass");
                    } else {
                        if (info->subpass >= render_pass->createInfo.subpassCount) {
                            skip |= LogError(commandBuffer, "VUID-VkCommandBufferBeginInfo-flags-06001",
                                             "vkBeginCommandBuffer(): Subpass member of pInheritanceInfo must be a valid subpass "
                                             "index within pInheritanceInfo->renderPass");
                        }
                    }
                } else {
                    if (!p_inherited_rendering_info) {
                        skip |= LogError(commandBuffer, "VUID-VkCommandBufferBeginInfo-flags-06002",
                                         "vkBeginCommandBuffer():The pNext chain of pInheritanceInfo must include a "
                                         "VkCommandBufferInheritanceRenderingInfoKHR structure");
                    }
                }
            }

            if (p_inherited_rendering_info) {
                auto p_attachment_sample_count_info_amd = LvlFindInChain<VkAttachmentSampleCountInfoAMD>(info->pNext);
                if (p_attachment_sample_count_info_amd &&
                    p_attachment_sample_count_info_amd->colorAttachmentCount != p_inherited_rendering_info->colorAttachmentCount) {
                    skip |= LogError(commandBuffer, "VUID-VkCommandBufferBeginInfo-flags-06003",
                                     "vkBeginCommandBuffer(): VkAttachmentSampleCountInfo{AMD,NV}->colorAttachmentCount[%u] must "
                                     "equal VkCommandBufferInheritanceRenderingInfoKHR->colorAttachmentCount[%u]",
                                     p_attachment_sample_count_info_amd->colorAttachmentCount,
                                     p_inherited_rendering_info->colorAttachmentCount);
                }

                if ((p_inherited_rendering_info->colorAttachmentCount != 0) &&
                    (p_inherited_rendering_info->rasterizationSamples & AllVkSampleCountFlagBits) == 0) {
                    skip |=
                        LogError(commandBuffer, "VUID-VkCommandBufferInheritanceRenderingInfo-colorAttachmentCount-06004",
                                 "vkBeginCommandBuffer(): VkCommandBufferInheritanceRenderingInfo->colorAttachmentCount (%" PRIu32
                                 ") is not 0, rasterizationSamples (%s) must be valid VkSampleCountFlagBits value",
                                 p_inherited_rendering_info->colorAttachmentCount,
                                 string_VkSampleCountFlagBits(p_inherited_rendering_info->rasterizationSamples));
                }

                if ((enabled_features.core.variableMultisampleRate == false) &&
                    (p_inherited_rendering_info->rasterizationSamples & AllVkSampleCountFlagBits) == 0) {
                    skip |= LogError(commandBuffer, "VUID-VkCommandBufferInheritanceRenderingInfo-variableMultisampleRate-06005",
                                     "vkBeginCommandBuffer(): If the variableMultisampleRate feature is not enabled, "
                                     "rasterizationSamples (%s) must be a valid VkSampleCountFlagBits",
                                     string_VkSampleCountFlagBits(p_inherited_rendering_info->rasterizationSamples));
                }

                for (uint32_t i = 0; i < p_inherited_rendering_info->colorAttachmentCount; ++i) {
                    if (p_inherited_rendering_info->pColorAttachmentFormats != nullptr) {
                        const VkFormat attachment_format = p_inherited_rendering_info->pColorAttachmentFormats[i];
                        if (attachment_format != VK_FORMAT_UNDEFINED) {
                            const VkFormatFeatureFlags2KHR potential_format_features =
                                GetPotentialFormatFeatures(attachment_format);
                            if ((potential_format_features & VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT_KHR) == 0) {
                                if (!enabled_features.linear_color_attachment_features.linearColorAttachment) {
                                    skip |= LogError(
                                        commandBuffer, "VUID-VkCommandBufferInheritanceRenderingInfo-pColorAttachmentFormats-06006",
                                        "vkBeginCommandBuffer(): "
                                        "VkCommandBufferInheritanceRenderingInfo->pColorAttachmentFormats[%u] (%s) must be a "
                                        "format with potential format features that include VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT",
                                        i, string_VkFormat(attachment_format));
                                } else if ((potential_format_features & VK_FORMAT_FEATURE_2_LINEAR_COLOR_ATTACHMENT_BIT_NV) == 0) {
                                    skip |= LogError(
                                        commandBuffer,
                                        "VUID-VkCommandBufferInheritanceRenderingInfoKHR-pColorAttachmentFormats-06492",
                                        "vkBeginCommandBuffer(): "
                                        "VkCommandBufferInheritanceRenderingInfo->pColorAttachmentFormats[%u] (%s) must be a "
                                        "format with potential format features that include VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT "
                                        "or VK_FORMAT_FEATURE_2_LINEAR_COLOR_ATTACHMENT_BIT_NV",
                                        i, string_VkFormat(attachment_format));
                                }
                            }
                        }
                    }
                }

                const VkFormatFeatureFlags2KHR valid_depth_stencil_format = VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT_KHR;
                const VkFormat depth_format = p_inherited_rendering_info->depthAttachmentFormat;
                if (depth_format != VK_FORMAT_UNDEFINED) {
                    const VkFormatFeatureFlags2KHR potential_format_features = GetPotentialFormatFeatures(depth_format);
                    if ((potential_format_features & valid_depth_stencil_format) == 0) {
                        skip |= LogError(
                            commandBuffer, "VUID-VkCommandBufferInheritanceRenderingInfo-depthAttachmentFormat-06007",
                            "vkBeginCommandBuffer(): VkCommandBufferInheritanceRenderingInfo->depthAttachmentFormat (%s) must be a "
                            "format with potential format features that include VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT",
                            string_VkFormat(depth_format));
                    }
                    if (!FormatHasDepth(depth_format)) {
                        skip |= LogError(
                            commandBuffer, "VUID-VkCommandBufferInheritanceRenderingInfo-depthAttachmentFormat-06540",
                            "vkBeginCommandBuffer(): VkCommandBufferInheritanceRenderingInfo->depthAttachmentFormat (%s) must be a "
                            "format with a depth aspect.",
                            string_VkFormat(depth_format));
                    }
                }

                const VkFormat stencil_format = p_inherited_rendering_info->stencilAttachmentFormat;
                if (stencil_format != VK_FORMAT_UNDEFINED) {
                    const VkFormatFeatureFlags2KHR potential_format_features = GetPotentialFormatFeatures(stencil_format);
                    if ((potential_format_features & valid_depth_stencil_format) == 0) {
                        skip |= LogError(
                            commandBuffer, "VUID-VkCommandBufferInheritanceRenderingInfo-stencilAttachmentFormat-06199",
                            "vkBeginCommandBuffer(): VkCommandBufferInheritanceRenderingInfo->stencilAttachmentFormat (%s) must be "
                            "a format with potential format features that include VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT",
                            string_VkFormat(stencil_format));
                    }
                    if (!FormatHasStencil(stencil_format)) {
                        skip |=
                            LogError(commandBuffer, "VUID-VkCommandBufferInheritanceRenderingInfo-stencilAttachmentFormat-06541",
                                     "vkBeginCommandBuffer(): VkCommandBufferInheritanceRenderingInfo->stencilAttachmentFormat "
                                     "(%s) must include stencil aspect.",
                                     string_VkFormat(stencil_format));
                    }
                }

                if ((depth_format != VK_FORMAT_UNDEFINED && stencil_format != VK_FORMAT_UNDEFINED) &&
                    (depth_format != stencil_format)) {
                    skip |= LogError(commandBuffer, "VUID-VkCommandBufferInheritanceRenderingInfo-depthAttachmentFormat-06200",
                                     "vkBeginCommandBuffer(): VkCommandBufferInheritanceRenderingInfoKHR->depthAttachmentFormat "
                                     "(%s) must equal VkCommandBufferInheritanceRenderingInfoKHR->stencilAttachmentFormat (%s)",
                                     string_VkFormat(depth_format), string_VkFormat(stencil_format));
                }

                if ((enabled_features.core11.multiview == VK_FALSE) && (p_inherited_rendering_info->viewMask != 0)) {
                    skip |= LogError(commandBuffer, "VUID-VkCommandBufferInheritanceRenderingInfo-multiview-06008",
                                     "vkBeginCommandBuffer(): If the multiview feature is not enabled, viewMask must be 0 (%u)",
                                     p_inherited_rendering_info->viewMask);
                }

                if (MostSignificantBit(p_inherited_rendering_info->viewMask) >=
                    static_cast<int32_t>(phys_dev_props_core11.maxMultiviewViewCount)) {
                    skip |=
                        LogError(commandBuffer, "VUID-VkCommandBufferInheritanceRenderingInfo-viewMask-06009",
                                 "vkBeginCommandBuffer(): Most significant bit "
                                 "VkCommandBufferInheritanceRenderingInfoKHR->viewMask(%u) must be less maxMultiviewViewCount(%u)",
                                 p_inherited_rendering_info->viewMask, phys_dev_props_core11.maxMultiviewViewCount);
                }
            }
        }

        if (info) {
            if ((info->occlusionQueryEnable == VK_FALSE || enabled_features.core.occlusionQueryPrecise == VK_FALSE) &&
                (info->queryFlags & VK_QUERY_CONTROL_PRECISE_BIT)) {
                skip |= LogError(commandBuffer, "VUID-vkBeginCommandBuffer-commandBuffer-00052",
                                 "vkBeginCommandBuffer(): Secondary %s must not have VK_QUERY_CONTROL_PRECISE_BIT if "
                                 "occulusionQuery is disabled or the device does not support precise occlusion queries.",
                                 report_data->FormatHandle(commandBuffer).c_str());
            }
            auto p_inherited_viewport_scissor_info = LvlFindInChain<VkCommandBufferInheritanceViewportScissorInfoNV>(info->pNext);
            if (p_inherited_viewport_scissor_info != nullptr && p_inherited_viewport_scissor_info->viewportScissor2D) {
                if (!enabled_features.inherited_viewport_scissor_features.inheritedViewportScissor2D) {
                    skip |= LogError(commandBuffer, "VUID-VkCommandBufferInheritanceViewportScissorInfoNV-viewportScissor2D-04782",
                                     "vkBeginCommandBuffer(): inheritedViewportScissor2D feature not enabled.");
                }
                if (!(pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)) {
                    skip |= LogError(commandBuffer, "VUID-VkCommandBufferInheritanceViewportScissorInfoNV-viewportScissor2D-04786",
                                     "vkBeginCommandBuffer(): Secondary %s must be recorded with the"
                                     "VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT if viewportScissor2D is VK_TRUE.",
                                     report_data->FormatHandle(commandBuffer).c_str());
                }
                if (p_inherited_viewport_scissor_info->viewportDepthCount == 0) {
                    skip |= LogError(commandBuffer, "VUID-VkCommandBufferInheritanceViewportScissorInfoNV-viewportScissor2D-04784",
                                     "vkBeginCommandBuffer(): "
                                     "If viewportScissor2D is VK_TRUE, then viewportDepthCount must be greater than 0.");
                }
            }

            // Check for dynamic rendering feature enabled or 1.3
            if ((api_version < VK_API_VERSION_1_3) && (!enabled_features.core13.dynamicRendering)) {
                if (pBeginInfo->flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) {
                    if (info->renderPass != VK_NULL_HANDLE) {
                        auto render_pass = Get<RENDER_PASS_STATE>(info->renderPass);
                        if (render_pass) {
                            if (info->subpass >= render_pass->createInfo.subpassCount) {
                                skip |= LogError(commandBuffer, "VUID-VkCommandBufferBeginInfo-flags-00054",
                                                 "vkBeginCommandBuffer(): Secondary %s must have a subpass index (%d) that is "
                                                 "less than the number of subpasses (%d).",
                                                 report_data->FormatHandle(commandBuffer).c_str(), info->subpass,
                                                 render_pass->createInfo.subpassCount);
                            }
                        }
                    } else {
                        skip |= LogError(commandBuffer, "VUID-VkCommandBufferBeginInfo-flags-00053",
                                         "vkBeginCommandBuffer(): Flags contains VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT, "
                                         "the renderpass member of pInheritanceInfo must be a valid VkRenderPass.");
                    }
                }
            }
        }
    }
    if (CB_RECORDING == cb_state->state) {
        skip |= LogError(commandBuffer, "VUID-vkBeginCommandBuffer-commandBuffer-00049",
                         "vkBeginCommandBuffer(): Cannot call Begin on %s in the RECORDING state. Must first call "
                         "vkEndCommandBuffer().",
                         report_data->FormatHandle(commandBuffer).c_str());
    } else if (CB_RECORDED == cb_state->state || CB_INVALID_COMPLETE == cb_state->state) {
        VkCommandPool cmd_pool = cb_state->createInfo.commandPool;
        const auto *pool = cb_state->command_pool;
        if (!(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT & pool->createFlags)) {
            const LogObjectList objlist(commandBuffer, cmd_pool);
            skip |= LogError(objlist, "VUID-vkBeginCommandBuffer-commandBuffer-00050",
                             "Call to vkBeginCommandBuffer() on %s attempts to implicitly reset cmdBuffer created from "
                             "%s that does NOT have the VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT bit set.",
                             report_data->FormatHandle(commandBuffer).c_str(), report_data->FormatHandle(cmd_pool).c_str());
        }
    }
    auto chained_device_group_struct = LvlFindInChain<VkDeviceGroupCommandBufferBeginInfo>(pBeginInfo->pNext);
    if (chained_device_group_struct) {
        const LogObjectList objlist(commandBuffer);
        skip |= ValidateDeviceMaskToPhysicalDeviceCount(chained_device_group_struct->deviceMask, objlist,
                                                        "VUID-VkDeviceGroupCommandBufferBeginInfo-deviceMask-00106");
        skip |= ValidateDeviceMaskToZero(chained_device_group_struct->deviceMask, objlist,
                                         "VUID-VkDeviceGroupCommandBufferBeginInfo-deviceMask-00107");
    }
    return skip;
}

bool CoreChecks::PreCallValidateEndCommandBuffer(VkCommandBuffer commandBuffer) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state_ptr) {
        return skip;
    }
    const CMD_BUFFER_STATE &cb_state = *cb_state_ptr;
    if ((VK_COMMAND_BUFFER_LEVEL_PRIMARY == cb_state.createInfo.level) ||
        !(cb_state.beginInfo.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)) {
        // This needs spec clarification to update valid usage, see comments in PR:
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/165
        skip |= InsideRenderPass(cb_state, "vkEndCommandBuffer()", "VUID-vkEndCommandBuffer-commandBuffer-00060");
    }

    if (cb_state.state == CB_INVALID_COMPLETE || cb_state.state == CB_INVALID_INCOMPLETE) {
        skip |= ReportInvalidCommandBuffer(cb_state, "vkEndCommandBuffer()");
    } else if (CB_RECORDING != cb_state.state) {
        skip |= LogError(
            commandBuffer, "VUID-vkEndCommandBuffer-commandBuffer-00059",
            "vkEndCommandBuffer(): Cannot call End on %s when not in the RECORDING state. Must first call vkBeginCommandBuffer().",
            report_data->FormatHandle(commandBuffer).c_str());
    }

    for (const auto &query : cb_state.activeQueries) {
        skip |= LogError(commandBuffer, "VUID-vkEndCommandBuffer-commandBuffer-00061",
                         "vkEndCommandBuffer(): Ending command buffer with in progress query: %s, query %d.",
                         report_data->FormatHandle(query.pool).c_str(), query.query);
    }
    if (cb_state.conditional_rendering_active) {
        skip |= LogError(commandBuffer, "VUID-vkEndCommandBuffer-None-01978",
                         "vkEndCommandBuffer(): Ending command buffer with active conditional rendering.");
    }

    skip |= InsideVideoCodingScope(cb_state, "vkEndCommandBuffer()", "VUID-vkEndCommandBuffer-None-06991");

    return skip;
}

bool CoreChecks::PreCallValidateResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state) return false;
    VkCommandPool cmd_pool = cb_state->createInfo.commandPool;
    const auto *pool = cb_state->command_pool;

    if (!(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT & pool->createFlags)) {
        const LogObjectList objlist(commandBuffer, cmd_pool);
        skip |= LogError(objlist, "VUID-vkResetCommandBuffer-commandBuffer-00046",
                         "vkResetCommandBuffer(): Attempt to reset %s created from %s that does NOT have the "
                         "VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT bit set.",
                         report_data->FormatHandle(commandBuffer).c_str(), report_data->FormatHandle(cmd_pool).c_str());
    }
    skip |= CheckCommandBufferInFlight(cb_state.get(), "reset", "VUID-vkResetCommandBuffer-commandBuffer-00045");

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

bool CoreChecks::PreCallValidateCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                          VkImageLayout imageLayout) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    bool skip = false;

    skip |= ValidateCmd(*cb_state, CMD_BINDSHADINGRATEIMAGENV);

    if (!enabled_features.shading_rate_image_features.shadingRateImage) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBindShadingRateImageNV-None-02058",
                         "vkCmdBindShadingRateImageNV: The shadingRateImage feature is disabled.");
    }

    if (imageView == VK_NULL_HANDLE) {
        return skip;
    }
    auto view_state = Get<IMAGE_VIEW_STATE>(imageView);
    if (!view_state) {
        skip |= LogError(imageView, "VUID-vkCmdBindShadingRateImageNV-imageView-02059",
                         "vkCmdBindShadingRateImageNV: If imageView is not VK_NULL_HANDLE, it must be a valid "
                         "VkImageView handle.");
        return skip;
    }
    const auto &ivci = view_state->create_info;
    if (ivci.viewType != VK_IMAGE_VIEW_TYPE_2D && ivci.viewType != VK_IMAGE_VIEW_TYPE_2D_ARRAY) {
        skip |= LogError(imageView, "VUID-vkCmdBindShadingRateImageNV-imageView-02059",
                         "vkCmdBindShadingRateImageNV: If imageView is not VK_NULL_HANDLE, it must be a valid "
                         "VkImageView handle of type VK_IMAGE_VIEW_TYPE_2D or VK_IMAGE_VIEW_TYPE_2D_ARRAY.");
    }

    if (ivci.format != VK_FORMAT_R8_UINT) {
        skip |= LogError(
            imageView, "VUID-vkCmdBindShadingRateImageNV-imageView-02060",
            "vkCmdBindShadingRateImageNV: If imageView is not VK_NULL_HANDLE, it must have a format of VK_FORMAT_R8_UINT.");
    }

    const auto *image_state = view_state->image_state.get();
    auto usage = image_state->createInfo.usage;
    if (!(usage & VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV)) {
        skip |= LogError(imageView, "VUID-vkCmdBindShadingRateImageNV-imageView-02061",
                         "vkCmdBindShadingRateImageNV: If imageView is not VK_NULL_HANDLE, the image must have been "
                         "created with VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV set.");
    }

    bool hit_error = false;

    // XXX TODO: While the VUID says "each subresource", only the base mip level is
    // actually used. Since we don't have an existing convenience function to iterate
    // over all mip levels, just don't bother with non-base levels.
    const VkImageSubresourceRange &range = view_state->normalized_subresource_range;
    VkImageSubresourceLayers subresource = {range.aspectMask, range.baseMipLevel, range.baseArrayLayer, range.layerCount};

    if (image_state) {
        skip |= VerifyImageLayout(*cb_state, *image_state, subresource, imageLayout, VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV,
                                  "vkCmdCopyImage()", "VUID-vkCmdBindShadingRateImageNV-imageLayout-02063",
                                  "VUID-vkCmdBindShadingRateImageNV-imageView-02062", &hit_error);
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

bool CoreChecks::PreCallValidateCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   VkIndexType indexType) const {
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(buffer_state);
    assert(cb_state_ptr);

    bool skip = ValidateBufferUsageFlags(commandBuffer, *buffer_state, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, true,
                                         "VUID-vkCmdBindIndexBuffer-buffer-00433", "vkCmdBindIndexBuffer()",
                                         "VK_BUFFER_USAGE_INDEX_BUFFER_BIT");
    skip |= ValidateCmd(*cb_state_ptr, CMD_BINDINDEXBUFFER);
    skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *buffer_state, "vkCmdBindIndexBuffer()",
                                          "VUID-vkCmdBindIndexBuffer-buffer-00434");
    const auto offset_align = static_cast<VkDeviceSize>(GetIndexAlignment(indexType));
    if (offset % offset_align) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBindIndexBuffer-offset-00432",
                         "vkCmdBindIndexBuffer() offset (0x%" PRIxLEAST64 ") does not fall on alignment (%s) boundary.", offset,
                         string_VkIndexType(indexType));
    }
    if (offset >= buffer_state->requirements.size) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBindIndexBuffer-offset-00431",
                         "vkCmdBindIndexBuffer() offset (0x%" PRIxLEAST64 ") is not less than the size (0x%" PRIxLEAST64
                         ") of buffer (%s).",
                         offset, buffer_state->requirements.size, report_data->FormatHandle(buffer_state->buffer()).c_str());
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                     const VkBuffer *pBuffers, const VkDeviceSize *pOffsets) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);

    bool skip = false;
    skip |= ValidateCmd(*cb_state, CMD_BINDVERTEXBUFFERS);
    for (uint32_t i = 0; i < bindingCount; ++i) {
        auto buffer_state = Get<BUFFER_STATE>(pBuffers[i]);
        if (buffer_state) {
            skip |= ValidateBufferUsageFlags(commandBuffer, *buffer_state, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, true,
                                             "VUID-vkCmdBindVertexBuffers-pBuffers-00627", "vkCmdBindVertexBuffers()",
                                             "VK_BUFFER_USAGE_VERTEX_BUFFER_BIT");
            skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *buffer_state, "vkCmdBindVertexBuffers()",
                                                  "VUID-vkCmdBindVertexBuffers-pBuffers-00628");
            if (pOffsets[i] >= buffer_state->createInfo.size) {
                skip |=
                    LogError(buffer_state->buffer(), "VUID-vkCmdBindVertexBuffers-pOffsets-00626",
                             "vkCmdBindVertexBuffers() offset (0x%" PRIxLEAST64 ") is beyond the end of the buffer.", pOffsets[i]);
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                VkDeviceSize dataSize, const void *pData) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    auto dst_buffer_state = Get<BUFFER_STATE>(dstBuffer);
    if (!cb_state_ptr || !dst_buffer_state) {
        return skip;
    }
    const CMD_BUFFER_STATE &cb_state = *cb_state_ptr;

    skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *dst_buffer_state, "vkCmdUpdateBuffer()",
                                          "VUID-vkCmdUpdateBuffer-dstBuffer-00035");
    // Validate that DST buffer has correct usage flags set
    skip |= ValidateBufferUsageFlags(commandBuffer, *dst_buffer_state, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true,
                                     "VUID-vkCmdUpdateBuffer-dstBuffer-00034", "vkCmdUpdateBuffer()",
                                     "VK_BUFFER_USAGE_TRANSFER_DST_BIT");
    skip |= ValidateCmd(cb_state, CMD_UPDATEBUFFER);
    skip |=
        ValidateProtectedBuffer(cb_state, *dst_buffer_state, "vkCmdUpdateBuffer()", "VUID-vkCmdUpdateBuffer-commandBuffer-01813");
    skip |=
        ValidateUnprotectedBuffer(cb_state, *dst_buffer_state, "vkCmdUpdateBuffer()", "VUID-vkCmdUpdateBuffer-commandBuffer-01814");
    if (dstOffset >= dst_buffer_state->createInfo.size) {
        skip |= LogError(
            commandBuffer, "VUID-vkCmdUpdateBuffer-dstOffset-00032",
            "vkCmdUpdateBuffer() dstOffset (0x%" PRIxLEAST64 ") is not less than the size (0x%" PRIxLEAST64 ") of buffer (%s).",
            dstOffset, dst_buffer_state->createInfo.size, report_data->FormatHandle(dst_buffer_state->buffer()).c_str());
    } else if (dataSize > dst_buffer_state->createInfo.size - dstOffset) {
        skip |= LogError(commandBuffer, "VUID-vkCmdUpdateBuffer-dataSize-00033",
                         "vkCmdUpdateBuffer() dataSize (0x%" PRIxLEAST64 ") is not less than the size (0x%" PRIxLEAST64
                         ") of buffer (%s) minus dstOffset (0x%" PRIxLEAST64 ").",
                         dataSize, dst_buffer_state->createInfo.size, report_data->FormatHandle(dst_buffer_state->buffer()).c_str(),
                         dstOffset);
    }
    return skip;
}

bool CoreChecks::MatchUsage(uint32_t count, const VkAttachmentReference2 *attachments, const VkFramebufferCreateInfo *fbci,
                            VkImageUsageFlagBits usage_flag, const char *error_code) const {
    bool skip = false;

    if (attachments) {
        for (uint32_t attach = 0; attach < count; attach++) {
            if (attachments[attach].attachment != VK_ATTACHMENT_UNUSED) {
                // Attachment counts are verified elsewhere, but prevent an invalid access
                if (attachments[attach].attachment < fbci->attachmentCount) {
                    if ((fbci->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) == 0) {
                        const VkImageView *image_view = &fbci->pAttachments[attachments[attach].attachment];
                        auto view_state = Get<IMAGE_VIEW_STATE>(*image_view);
                        if (view_state) {
                            const auto &ici = view_state->image_state->createInfo;
                            auto creation_usage = ici.usage;
                            const auto stencil_usage_info = LvlFindInChain<VkImageStencilUsageCreateInfo>(ici.pNext);
                            if (stencil_usage_info) {
                                creation_usage |= stencil_usage_info->stencilUsage;
                            }
                            if ((creation_usage & usage_flag) == 0) {
                                skip |= LogError(device, error_code,
                                                 "vkCreateFramebuffer:  Framebuffer Attachment (%d) conflicts with the image's "
                                                 "IMAGE_USAGE flags (%s).",
                                                 attachments[attach].attachment, string_VkImageUsageFlagBits(usage_flag));
                            }
                        }
                    } else {
                        const VkFramebufferAttachmentsCreateInfo *fbaci =
                            LvlFindInChain<VkFramebufferAttachmentsCreateInfo>(fbci->pNext);
                        if (fbaci != nullptr && fbaci->pAttachmentImageInfos != nullptr &&
                            fbaci->attachmentImageInfoCount > attachments[attach].attachment) {
                            uint32_t image_usage = fbaci->pAttachmentImageInfos[attachments[attach].attachment].usage;
                            if ((image_usage & usage_flag) == 0) {
                                skip |=
                                    LogError(device, error_code,
                                             "vkCreateFramebuffer:  Framebuffer attachment info (%d) conflicts with the image's "
                                             "IMAGE_USAGE flags (%s).",
                                             attachments[attach].attachment, string_VkImageUsageFlagBits(usage_flag));
                            }
                        }
                    }
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateFramebufferCreateInfo(const VkFramebufferCreateInfo *pCreateInfo) const {
    bool skip = false;

    const VkFramebufferAttachmentsCreateInfo *framebuffer_attachments_create_info =
        LvlFindInChain<VkFramebufferAttachmentsCreateInfo>(pCreateInfo->pNext);
    if ((pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) != 0) {
        if (!enabled_features.core12.imagelessFramebuffer) {
            skip |= LogError(device, "VUID-VkFramebufferCreateInfo-flags-03189",
                             "vkCreateFramebuffer(): VkFramebufferCreateInfo flags includes VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT, "
                             "but the imagelessFramebuffer feature is not enabled.");
        }

        if (framebuffer_attachments_create_info == nullptr) {
            skip |= LogError(device, "VUID-VkFramebufferCreateInfo-flags-03190",
                             "vkCreateFramebuffer(): VkFramebufferCreateInfo flags includes VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT, "
                             "but no instance of VkFramebufferAttachmentsCreateInfo is present in the pNext chain.");
        } else {
            if (framebuffer_attachments_create_info->attachmentImageInfoCount != 0 &&
                framebuffer_attachments_create_info->attachmentImageInfoCount != pCreateInfo->attachmentCount) {
                skip |= LogError(device, "VUID-VkFramebufferCreateInfo-flags-03191",
                                 "vkCreateFramebuffer(): VkFramebufferCreateInfo attachmentCount is %u, but "
                                 "VkFramebufferAttachmentsCreateInfo attachmentImageInfoCount is %u.",
                                 pCreateInfo->attachmentCount, framebuffer_attachments_create_info->attachmentImageInfoCount);
            }
        }
    }
    if (framebuffer_attachments_create_info) {
        for (uint32_t i = 0; i < framebuffer_attachments_create_info->attachmentImageInfoCount; ++i) {
            if (framebuffer_attachments_create_info->pAttachmentImageInfos[i].pNext != nullptr) {
                skip |= LogError(device, "VUID-VkFramebufferAttachmentImageInfo-pNext-pNext",
                                 "vkCreateFramebuffer(): VkFramebufferAttachmentsCreateInfo[%" PRIu32 "].pNext is not NULL.", i);
            }
        }
    }

    auto rp_state = Get<RENDER_PASS_STATE>(pCreateInfo->renderPass);
    if (rp_state) {
        const VkRenderPassCreateInfo2 *rpci = rp_state->createInfo.ptr();

        bool b_has_non_zero_view_masks = false;
        for (uint32_t i = 0; i < rpci->subpassCount; ++i) {
            if (rpci->pSubpasses[i].viewMask != 0) {
                b_has_non_zero_view_masks = true;
                break;
            }
        }

        if (rpci->attachmentCount != pCreateInfo->attachmentCount) {
            skip |= LogError(pCreateInfo->renderPass, "VUID-VkFramebufferCreateInfo-attachmentCount-00876",
                             "vkCreateFramebuffer(): VkFramebufferCreateInfo attachmentCount of %u does not match attachmentCount "
                             "of %u of %s being used to create Framebuffer.",
                             pCreateInfo->attachmentCount, rpci->attachmentCount,
                             report_data->FormatHandle(pCreateInfo->renderPass).c_str());
        } else {
            // attachmentCounts match, so make sure corresponding attachment details line up
            if ((pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) == 0) {
                const VkImageView *image_views = pCreateInfo->pAttachments;
                for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
                    auto view_state = Get<IMAGE_VIEW_STATE>(image_views[i]);
                    if (view_state == nullptr) {
                        skip |= LogError(
                            image_views[i], "VUID-VkFramebufferCreateInfo-flags-02778",
                            "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u is not a valid VkImageView.", i);
                    } else {
                        auto &ivci = view_state->create_info;
                        auto &subresource_range = view_state->normalized_subresource_range;
                        if (ivci.format != rpci->pAttachments[i].format) {
                            skip |= LogError(
                                pCreateInfo->renderPass, "VUID-VkFramebufferCreateInfo-pAttachments-00880",
                                "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u has format of %s that does not "
                                "match the format of %s used by the corresponding attachment for %s.",
                                i, string_VkFormat(ivci.format), string_VkFormat(rpci->pAttachments[i].format),
                                report_data->FormatHandle(pCreateInfo->renderPass).c_str());
                        }
                        const auto &ici = view_state->image_state->createInfo;
                        if (ici.samples != rpci->pAttachments[i].samples) {
                            skip |=
                                LogError(pCreateInfo->renderPass, "VUID-VkFramebufferCreateInfo-pAttachments-00881",
                                         "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u has %s samples that do not "
                                         "match the %s "
                                         "samples used by the corresponding attachment for %s.",
                                         i, string_VkSampleCountFlagBits(ici.samples),
                                         string_VkSampleCountFlagBits(rpci->pAttachments[i].samples),
                                         report_data->FormatHandle(pCreateInfo->renderPass).c_str());
                        }

                        // Verify that image memory is valid
                        auto image_data = Get<IMAGE_STATE>(ivci.image);
                        skip |= ValidateMemoryIsBoundToImage(device, *image_data, "vkCreateFramebuffer()",
                                                             kVUID_Core_Bound_Resource_FreedMemoryAccess);

                        // Verify that view only has a single mip level
                        if (subresource_range.levelCount != 1) {
                            skip |= LogError(
                                device, "VUID-VkFramebufferCreateInfo-pAttachments-00883",
                                "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u has mip levelCount of %u but "
                                "only a single mip level (levelCount ==  1) is allowed when creating a Framebuffer.",
                                i, subresource_range.levelCount);
                        }
                        const uint32_t mip_level = subresource_range.baseMipLevel;
                        uint32_t mip_width = max(1u, ici.extent.width >> mip_level);
                        uint32_t mip_height = max(1u, ici.extent.height >> mip_level);
                        bool used_as_input_color_resolve_depth_stencil_attachment = false;
                        bool used_as_fragment_shading_rate_attachment = false;
                        bool fsr_non_zero_viewmasks = false;

                        for (uint32_t j = 0; j < rpci->subpassCount; ++j) {
                            const VkSubpassDescription2 &subpass = rpci->pSubpasses[j];

                            int highest_view_bit = MostSignificantBit(subpass.viewMask);

                            for (uint32_t k = 0; k < rpci->pSubpasses[j].inputAttachmentCount; ++k) {
                                if (subpass.pInputAttachments[k].attachment == i) {
                                    used_as_input_color_resolve_depth_stencil_attachment = true;
                                    break;
                                }
                            }

                            for (uint32_t k = 0; k < rpci->pSubpasses[j].colorAttachmentCount; ++k) {
                                if (subpass.pColorAttachments[k].attachment == i ||
                                    (subpass.pResolveAttachments && subpass.pResolveAttachments[k].attachment == i)) {
                                    used_as_input_color_resolve_depth_stencil_attachment = true;
                                    break;
                                }
                            }

                            if (subpass.pDepthStencilAttachment && subpass.pDepthStencilAttachment->attachment == i) {
                                used_as_input_color_resolve_depth_stencil_attachment = true;
                            }

                            if (used_as_input_color_resolve_depth_stencil_attachment) {
                                if (static_cast<int32_t>(subresource_range.layerCount) <= highest_view_bit) {
                                    skip |= LogError(
                                        device, "VUID-VkFramebufferCreateInfo-renderPass-04536",
                                        "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u has a layer count (%u) "
                                        "less than or equal to the highest bit in the view mask (%i) of subpass %u.",
                                        i, subresource_range.layerCount, highest_view_bit, j);
                                }
                            }

                            if (enabled_features.fragment_shading_rate_features.attachmentFragmentShadingRate) {
                                const VkFragmentShadingRateAttachmentInfoKHR *fsr_attachment;
                                fsr_attachment = LvlFindInChain<VkFragmentShadingRateAttachmentInfoKHR>(subpass.pNext);
                                if (fsr_attachment && fsr_attachment->pFragmentShadingRateAttachment &&
                                    fsr_attachment->pFragmentShadingRateAttachment->attachment == i) {
                                    used_as_fragment_shading_rate_attachment = true;
                                    if ((mip_width * fsr_attachment->shadingRateAttachmentTexelSize.width) < pCreateInfo->width) {
                                        skip |= LogError(device, "VUID-VkFramebufferCreateInfo-flags-04539",
                                                         "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u mip level "
                                                         "%u is used as a "
                                                         "fragment shading rate attachment in subpass %u, but the product of its "
                                                         "width (%u) and the "
                                                         "specified shading rate texel width (%u) are smaller than the "
                                                         "corresponding framebuffer width (%u).",
                                                         i, subresource_range.baseMipLevel, j, mip_width,
                                                         fsr_attachment->shadingRateAttachmentTexelSize.width, pCreateInfo->width);
                                    }
                                    if ((mip_height * fsr_attachment->shadingRateAttachmentTexelSize.height) <
                                        pCreateInfo->height) {
                                        skip |=
                                            LogError(device, "VUID-VkFramebufferCreateInfo-flags-04540",
                                                     "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u mip level %u "
                                                     "is used as a "
                                                     "fragment shading rate attachment in subpass %u, but the product of its "
                                                     "height (%u) and the "
                                                     "specified shading rate texel height (%u) are smaller than the corresponding "
                                                     "framebuffer height (%u).",
                                                     i, subresource_range.baseMipLevel, j, mip_height,
                                                     fsr_attachment->shadingRateAttachmentTexelSize.height, pCreateInfo->height);
                                    }
                                    if (highest_view_bit >= 0) {
                                        fsr_non_zero_viewmasks = true;
                                    }
                                    if (static_cast<int32_t>(subresource_range.layerCount) <= highest_view_bit &&
                                        subresource_range.layerCount != 1) {
                                        skip |= LogError(
                                            device, "VUID-VkFramebufferCreateInfo-flags-04537",
                                            "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u has a layer count (%u) "
                                            "less than or equal to the highest bit in the view mask (%i) of subpass %u.",
                                            i, subresource_range.layerCount, highest_view_bit, j);
                                    }
                                }
                            }

                            if (enabled_features.fragment_density_map_features.fragmentDensityMap &&
                                api_version >= VK_API_VERSION_1_1) {
                                const VkRenderPassFragmentDensityMapCreateInfoEXT *fdm_attachment;
                                fdm_attachment = LvlFindInChain<VkRenderPassFragmentDensityMapCreateInfoEXT>(rpci->pNext);

                                if (fdm_attachment && fdm_attachment->fragmentDensityMapAttachment.attachment == i) {
                                    int32_t layer_count = view_state->normalized_subresource_range.layerCount;
                                    if (b_has_non_zero_view_masks && layer_count != 1 && layer_count <= highest_view_bit) {
                                        skip |= LogError(device, "VUID-VkFramebufferCreateInfo-renderPass-02746",
                                                         "vkCreateFrameBuffer(): VkFramebufferCreateInfo attachment #%" PRIu32
                                                         " has a layer count (%" PRIi32
                                                         ") different than 1 or lower than the most significant bit in viewMask (%i"
                                                         ") but renderPass (%s) was specified with non-zero view masks\n",
                                                         i, layer_count, highest_view_bit,
                                                         report_data->FormatHandle(pCreateInfo->renderPass).c_str());
                                    }

                                    if (!b_has_non_zero_view_masks && layer_count != 1) {
                                        skip |= LogError(
                                            device, "VUID-VkFramebufferCreateInfo-renderPass-02747",
                                            "vkCreateFrameBuffer(): VkFramebufferCreateInfo attachment #%" PRIu32
                                            " had a layer count (%" PRIu32
                                            ") not equal to 1 but renderPass (%s) was not specified with non-zero view masks\n",
                                            i, layer_count, report_data->FormatHandle(pCreateInfo->renderPass).c_str());
                                    }
                                }
                            }
                        }

                        if (enabled_features.fragment_density_map_features.fragmentDensityMap) {
                            const VkRenderPassFragmentDensityMapCreateInfoEXT *fdm_attachment;
                            fdm_attachment = LvlFindInChain<VkRenderPassFragmentDensityMapCreateInfoEXT>(rpci->pNext);
                            if (fdm_attachment && fdm_attachment->fragmentDensityMapAttachment.attachment == i) {
                                uint32_t ceiling_width = layer_data::GetQuotientCeil(
                                    pCreateInfo->width,
                                    phys_dev_ext_props.fragment_density_map_props.maxFragmentDensityTexelSize.width);
                                if (mip_width < ceiling_width) {
                                    skip |= LogError(
                                        device, "VUID-VkFramebufferCreateInfo-pAttachments-02555",
                                        "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u mip level %u has width "
                                        "smaller than the corresponding the ceiling of framebuffer width / "
                                        "maxFragmentDensityTexelSize.width "
                                        "Here are the respective dimensions for attachment #%u, the ceiling value:\n "
                                        "attachment #%u, framebuffer:\n"
                                        "width: %u, the ceiling value: %u\n",
                                        i, subresource_range.baseMipLevel, i, i, mip_width, ceiling_width);
                                }
                                uint32_t ceiling_height = layer_data::GetQuotientCeil(
                                    pCreateInfo->height,
                                    phys_dev_ext_props.fragment_density_map_props.maxFragmentDensityTexelSize.height);
                                if (mip_height < ceiling_height) {
                                    skip |= LogError(
                                        device, "VUID-VkFramebufferCreateInfo-pAttachments-02556",
                                        "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u mip level %u has height "
                                        "smaller than the corresponding the ceiling of framebuffer height / "
                                        "maxFragmentDensityTexelSize.height "
                                        "Here are the respective dimensions for attachment #%u, the ceiling value:\n "
                                        "attachment #%u, framebuffer:\n"
                                        "height: %u, the ceiling value: %u\n",
                                        i, subresource_range.baseMipLevel, i, i, mip_height, ceiling_height);
                                }
                                if (view_state->normalized_subresource_range.layerCount != 1 &&
                                    !(api_version >= VK_API_VERSION_1_1 || IsExtEnabled(device_extensions.vk_khr_multiview))) {
                                    skip |= LogError(device, "VUID-VkFramebufferCreateInfo-pAttachments-02744",
                                                     "vkCreateFramebuffer(): pCreateInfo->pAttachments[%" PRIu32
                                                     "] is referenced by "
                                                     "VkRenderPassFragmentDensityMapCreateInfoEXT::fragmentDensityMapAttachment in "
                                                     "the pNext chain, but it was create with subresourceRange.layerCount (%" PRIu32
                                                     ") different from 1.",
                                                     i, view_state->normalized_subresource_range.layerCount);
                                }
                            }
                        }

                        if (used_as_input_color_resolve_depth_stencil_attachment) {
                            if (mip_width < pCreateInfo->width) {
                                skip |= LogError(device, "VUID-VkFramebufferCreateInfo-flags-04533",
                                                 "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u mip level %u has "
                                                 "width (%u) smaller than the corresponding framebuffer width (%u).",
                                                 i, mip_level, mip_width, pCreateInfo->width);
                            }
                            if (mip_height < pCreateInfo->height) {
                                skip |= LogError(device, "VUID-VkFramebufferCreateInfo-flags-04534",
                                                 "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u mip level %u has "
                                                 "height (%u) smaller than the corresponding framebuffer height (%u).",
                                                 i, mip_level, mip_height, pCreateInfo->height);
                            }
                            uint32_t layerCount = view_state->GetAttachmentLayerCount();
                            if (layerCount < pCreateInfo->layers) {
                                skip |=
                                    LogError(device, "VUID-VkFramebufferCreateInfo-flags-04535",
                                             "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u has a layer count (%u) "
                                             "smaller than the corresponding framebuffer layer count (%u).",
                                             i, layerCount, pCreateInfo->layers);
                            }
                        }

                        if (used_as_fragment_shading_rate_attachment && !fsr_non_zero_viewmasks) {
                            if (subresource_range.layerCount != 1 && subresource_range.layerCount < pCreateInfo->layers) {
                                skip |=
                                    LogError(device, "VUID-VkFramebufferCreateInfo-flags-04538",
                                             "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u has a layer count (%u) "
                                             "smaller than the corresponding framebuffer layer count (%u).",
                                             i, subresource_range.layerCount, pCreateInfo->layers);
                            }
                        }

                        if (IsIdentitySwizzle(ivci.components) == false) {
                            skip |= LogError(
                                device, "VUID-VkFramebufferCreateInfo-pAttachments-00884",
                                "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u has non-identy swizzle. All "
                                "framebuffer attachments must have been created with the identity swizzle. Here are the actual "
                                "swizzle values:\n"
                                "r swizzle = %s\n"
                                "g swizzle = %s\n"
                                "b swizzle = %s\n"
                                "a swizzle = %s\n",
                                i, string_VkComponentSwizzle(ivci.components.r), string_VkComponentSwizzle(ivci.components.g),
                                string_VkComponentSwizzle(ivci.components.b), string_VkComponentSwizzle(ivci.components.a));
                        }
                        if ((ivci.viewType == VK_IMAGE_VIEW_TYPE_2D) || (ivci.viewType == VK_IMAGE_VIEW_TYPE_2D)) {
                            auto image_state = Get<IMAGE_STATE>(ivci.image);
                            if (image_state->createInfo.imageType == VK_IMAGE_TYPE_3D) {
                                if (FormatIsDepthOrStencil(ivci.format)) {
                                    const LogObjectList objlist(device, ivci.image);
                                    skip |= LogError(
                                        objlist, "VUID-VkFramebufferCreateInfo-pAttachments-00891",
                                        "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u has an image view type of "
                                        "%s "
                                        "which was taken from image %s of type VK_IMAGE_TYPE_3D, but the image view format is a "
                                        "depth/stencil format %s",
                                        i, string_VkImageViewType(ivci.viewType), report_data->FormatHandle(ivci.image).c_str(),
                                        string_VkFormat(ivci.format));
                                }
                            }
                        }
                        if (ivci.viewType == VK_IMAGE_VIEW_TYPE_3D) {
                            const LogObjectList objlist(device, image_views[i]);
                            skip |= LogError(objlist, "VUID-VkFramebufferCreateInfo-flags-04113",
                                             "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u has an image view type "
                                             "of VK_IMAGE_VIEW_TYPE_3D",
                                             i);
                        }
                    }
                }
            } else if (framebuffer_attachments_create_info) {
                // VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT is set
                for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
                    auto &aii = framebuffer_attachments_create_info->pAttachmentImageInfos[i];
                    bool format_found = false;
                    for (uint32_t j = 0; j < aii.viewFormatCount; ++j) {
                        if (aii.pViewFormats[j] == rpci->pAttachments[i].format) {
                            format_found = true;
                        }
                    }
                    if (!format_found) {
                        skip |= LogError(pCreateInfo->renderPass, "VUID-VkFramebufferCreateInfo-flags-03205",
                                         "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment info #%u does not include "
                                         "format %s used "
                                         "by the corresponding attachment for renderPass (%s).",
                                         i, string_VkFormat(rpci->pAttachments[i].format),
                                         report_data->FormatHandle(pCreateInfo->renderPass).c_str());
                    }

                    bool used_as_input_color_resolve_depth_stencil_attachment = false;
                    bool used_as_fragment_shading_rate_attachment = false;
                    bool fsr_non_zero_viewmasks = false;

                    for (uint32_t j = 0; j < rpci->subpassCount; ++j) {
                        const VkSubpassDescription2 &subpass = rpci->pSubpasses[j];

                        int highest_view_bit = MostSignificantBit(subpass.viewMask);

                        for (uint32_t k = 0; k < rpci->pSubpasses[j].inputAttachmentCount; ++k) {
                            if (subpass.pInputAttachments[k].attachment == i) {
                                used_as_input_color_resolve_depth_stencil_attachment = true;
                                break;
                            }
                        }

                        for (uint32_t k = 0; k < rpci->pSubpasses[j].colorAttachmentCount; ++k) {
                            if (subpass.pColorAttachments[k].attachment == i ||
                                (subpass.pResolveAttachments && subpass.pResolveAttachments[k].attachment == i)) {
                                used_as_input_color_resolve_depth_stencil_attachment = true;
                                break;
                            }
                        }

                        if (subpass.pDepthStencilAttachment && subpass.pDepthStencilAttachment->attachment == i) {
                            used_as_input_color_resolve_depth_stencil_attachment = true;
                        }

                        if (enabled_features.fragment_shading_rate_features.attachmentFragmentShadingRate) {
                            const VkFragmentShadingRateAttachmentInfoKHR *fsr_attachment;
                            fsr_attachment = LvlFindInChain<VkFragmentShadingRateAttachmentInfoKHR>(subpass.pNext);
                            if (fsr_attachment && fsr_attachment->pFragmentShadingRateAttachment->attachment == i) {
                                used_as_fragment_shading_rate_attachment = true;
                                if ((aii.width * fsr_attachment->shadingRateAttachmentTexelSize.width) < pCreateInfo->width) {
                                    skip |= LogError(
                                        device, "VUID-VkFramebufferCreateInfo-flags-04543",
                                        "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u is used as a "
                                        "fragment shading rate attachment in subpass %u, but the product of its width (%u) and the "
                                        "specified shading rate texel width (%u) are smaller than the corresponding framebuffer "
                                        "width (%u).",
                                        i, j, aii.width, fsr_attachment->shadingRateAttachmentTexelSize.width, pCreateInfo->width);
                                }
                                if ((aii.height * fsr_attachment->shadingRateAttachmentTexelSize.height) < pCreateInfo->height) {
                                    skip |= LogError(device, "VUID-VkFramebufferCreateInfo-flags-04544",
                                                     "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u is used as a "
                                                     "fragment shading rate attachment in subpass %u, but the product of its "
                                                     "height (%u) and the "
                                                     "specified shading rate texel height (%u) are smaller than the corresponding "
                                                     "framebuffer height (%u).",
                                                     i, j, aii.height, fsr_attachment->shadingRateAttachmentTexelSize.height,
                                                     pCreateInfo->height);
                                }
                                if (highest_view_bit >= 0) {
                                    fsr_non_zero_viewmasks = true;
                                }
                                if (aii.layerCount != 1 && static_cast<int32_t>(aii.layerCount) <= highest_view_bit) {
                                    skip |= LogError(
                                        device, kVUIDUndefined,
                                        "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u has a layer count (%u) "
                                        "less than or equal to the highest bit in the view mask (%i) of subpass %u.",
                                        i, aii.layerCount, highest_view_bit, j);
                                }
                            }
                        }
                    }

                    if (used_as_input_color_resolve_depth_stencil_attachment) {
                        if (aii.width < pCreateInfo->width) {
                            skip |= LogError(
                                device, "VUID-VkFramebufferCreateInfo-flags-04541",
                                "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment info #%u has a width of only #%u, "
                                "but framebuffer has a width of #%u.",
                                i, aii.width, pCreateInfo->width);
                        }

                        if (aii.height < pCreateInfo->height) {
                            skip |= LogError(
                                device, "VUID-VkFramebufferCreateInfo-flags-04542",
                                "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment info #%u has a height of only #%u, "
                                "but framebuffer has a height of #%u.",
                                i, aii.height, pCreateInfo->height);
                        }

                        const char *mismatched_layers_no_multiview_vuid = IsExtEnabled(device_extensions.vk_khr_multiview)
                                                                              ? "VUID-VkFramebufferCreateInfo-renderPass-04546"
                                                                              : "VUID-VkFramebufferCreateInfo-flags-04547";
                        if ((rpci->subpassCount == 0) || (rpci->pSubpasses[0].viewMask == 0)) {
                            if (aii.layerCount < pCreateInfo->layers) {
                                skip |= LogError(
                                    device, mismatched_layers_no_multiview_vuid,
                                    "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment info #%u has only #%u layers, "
                                    "but framebuffer has #%u layers.",
                                    i, aii.layerCount, pCreateInfo->layers);
                            }
                        }
                    }

                    if (used_as_fragment_shading_rate_attachment && !fsr_non_zero_viewmasks) {
                        if (aii.layerCount != 1 && aii.layerCount < pCreateInfo->layers) {
                            skip |= LogError(device, "VUID-VkFramebufferCreateInfo-flags-04545",
                                             "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment #%u has a layer count (%u) "
                                             "smaller than the corresponding framebuffer layer count (%u).",
                                             i, aii.layerCount, pCreateInfo->layers);
                        }
                    }
                }

                // Validate image usage
                uint32_t attachment_index = VK_ATTACHMENT_UNUSED;
                for (uint32_t i = 0; i < rpci->subpassCount; ++i) {
                    skip |= MatchUsage(rpci->pSubpasses[i].colorAttachmentCount, rpci->pSubpasses[i].pColorAttachments, pCreateInfo,
                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, "VUID-VkFramebufferCreateInfo-flags-03201");
                    skip |=
                        MatchUsage(rpci->pSubpasses[i].colorAttachmentCount, rpci->pSubpasses[i].pResolveAttachments, pCreateInfo,
                                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, "VUID-VkFramebufferCreateInfo-flags-03201");
                    skip |= MatchUsage(1, rpci->pSubpasses[i].pDepthStencilAttachment, pCreateInfo,
                                       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, "VUID-VkFramebufferCreateInfo-flags-03202");
                    skip |= MatchUsage(rpci->pSubpasses[i].inputAttachmentCount, rpci->pSubpasses[i].pInputAttachments, pCreateInfo,
                                       VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, "VUID-VkFramebufferCreateInfo-flags-03204");

                    const VkSubpassDescriptionDepthStencilResolve *depth_stencil_resolve =
                        LvlFindInChain<VkSubpassDescriptionDepthStencilResolve>(rpci->pSubpasses[i].pNext);
                    if (IsExtEnabled(device_extensions.vk_khr_depth_stencil_resolve) && depth_stencil_resolve != nullptr) {
                        skip |= MatchUsage(1, depth_stencil_resolve->pDepthStencilResolveAttachment, pCreateInfo,
                                           VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, "VUID-VkFramebufferCreateInfo-flags-03203");
                    }

                    const VkFragmentShadingRateAttachmentInfoKHR *fragment_shading_rate_attachment_info =
                        LvlFindInChain<VkFragmentShadingRateAttachmentInfoKHR>(rpci->pSubpasses[i].pNext);
                    if (enabled_features.fragment_shading_rate_features.attachmentFragmentShadingRate &&
                        fragment_shading_rate_attachment_info != nullptr) {
                        skip |= MatchUsage(1, fragment_shading_rate_attachment_info->pFragmentShadingRateAttachment, pCreateInfo,
                                           VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
                                           "VUID-VkFramebufferCreateInfo-flags-04549");
                    }
                }

                if (IsExtEnabled(device_extensions.vk_khr_multiview)) {
                    if ((rpci->subpassCount > 0) && (rpci->pSubpasses[0].viewMask != 0)) {
                        for (uint32_t i = 0; i < rpci->subpassCount; ++i) {
                            const VkSubpassDescriptionDepthStencilResolve *depth_stencil_resolve =
                                LvlFindInChain<VkSubpassDescriptionDepthStencilResolve>(rpci->pSubpasses[i].pNext);
                            uint32_t view_bits = rpci->pSubpasses[i].viewMask;
                            int highest_view_bit = MostSignificantBit(view_bits);

                            for (uint32_t j = 0; j < rpci->pSubpasses[i].colorAttachmentCount; ++j) {
                                attachment_index = rpci->pSubpasses[i].pColorAttachments[j].attachment;
                                if (attachment_index != VK_ATTACHMENT_UNUSED) {
                                    int32_t layer_count =
                                        framebuffer_attachments_create_info->pAttachmentImageInfos[attachment_index].layerCount;
                                    if (layer_count <= highest_view_bit) {
                                        skip |= LogError(
                                            pCreateInfo->renderPass, "VUID-VkFramebufferCreateInfo-renderPass-03198",
                                            "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment info %u "
                                            "only specifies %" PRIi32
                                            " layers, but the view mask for subpass %u in renderPass (%s) "
                                            "includes layer %i, with that attachment specified as a color attachment %u.",
                                            attachment_index, layer_count, i,
                                            report_data->FormatHandle(pCreateInfo->renderPass).c_str(), highest_view_bit, j);
                                    }
                                }
                                if (rpci->pSubpasses[i].pResolveAttachments) {
                                    attachment_index = rpci->pSubpasses[i].pResolveAttachments[j].attachment;
                                    if (attachment_index != VK_ATTACHMENT_UNUSED) {
                                        int32_t layer_count =
                                            framebuffer_attachments_create_info->pAttachmentImageInfos[attachment_index].layerCount;
                                        if (layer_count <= highest_view_bit) {
                                            skip |= LogError(
                                                pCreateInfo->renderPass, "VUID-VkFramebufferCreateInfo-renderPass-03198",
                                                "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment info %u "
                                                "only specifies %" PRIi32
                                                " layers, but the view mask for subpass %u in renderPass (%s) "
                                                "includes layer %i, with that attachment specified as a resolve attachment %u.",
                                                attachment_index, layer_count, i,
                                                report_data->FormatHandle(pCreateInfo->renderPass).c_str(), highest_view_bit, j);
                                        }
                                    }
                                }
                            }

                            for (uint32_t j = 0; j < rpci->pSubpasses[i].inputAttachmentCount; ++j) {
                                attachment_index = rpci->pSubpasses[i].pInputAttachments[j].attachment;
                                if (attachment_index != VK_ATTACHMENT_UNUSED) {
                                    int32_t layer_count =
                                        framebuffer_attachments_create_info->pAttachmentImageInfos[attachment_index].layerCount;
                                    if (layer_count <= highest_view_bit) {
                                        skip |= LogError(
                                            pCreateInfo->renderPass, "VUID-VkFramebufferCreateInfo-renderPass-03198",
                                            "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment info %u "
                                            "only specifies %" PRIi32
                                            " layers, but the view mask for subpass %u in renderPass (%s) "
                                            "includes layer %i, with that attachment specified as an input attachment %u.",
                                            attachment_index, layer_count, i,
                                            report_data->FormatHandle(pCreateInfo->renderPass).c_str(), highest_view_bit, j);
                                    }
                                }
                            }

                            if (rpci->pSubpasses[i].pDepthStencilAttachment != nullptr) {
                                attachment_index = rpci->pSubpasses[i].pDepthStencilAttachment->attachment;
                                if (attachment_index != VK_ATTACHMENT_UNUSED) {
                                    int32_t layer_count =
                                        framebuffer_attachments_create_info->pAttachmentImageInfos[attachment_index].layerCount;
                                    if (layer_count <= highest_view_bit) {
                                        skip |= LogError(
                                            pCreateInfo->renderPass, "VUID-VkFramebufferCreateInfo-renderPass-03198",
                                            "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment info %u "
                                            "only specifies %" PRIi32
                                            " layers, but the view mask for subpass %u in renderPass (%s) "
                                            "includes layer %i, with that attachment specified as a depth/stencil attachment.",
                                            attachment_index, layer_count, i,
                                            report_data->FormatHandle(pCreateInfo->renderPass).c_str(), highest_view_bit);
                                    }
                                }

                                if (IsExtEnabled(device_extensions.vk_khr_depth_stencil_resolve) &&
                                    depth_stencil_resolve != nullptr &&
                                    depth_stencil_resolve->pDepthStencilResolveAttachment != nullptr) {
                                    attachment_index = depth_stencil_resolve->pDepthStencilResolveAttachment->attachment;
                                    if (attachment_index != VK_ATTACHMENT_UNUSED) {
                                        int32_t layer_count =
                                            framebuffer_attachments_create_info->pAttachmentImageInfos[attachment_index].layerCount;
                                        if (layer_count <= highest_view_bit) {
                                            skip |= LogError(
                                                pCreateInfo->renderPass, "VUID-VkFramebufferCreateInfo-renderPass-03198",
                                                "vkCreateFramebuffer(): VkFramebufferCreateInfo attachment info %u "
                                                "only specifies %" PRIi32
                                                " layers, but the view mask for subpass %u in renderPass (%s) "
                                                "includes layer %i, with that attachment specified as a depth/stencil resolve "
                                                "attachment.",
                                                attachment_index, layer_count, i,
                                                report_data->FormatHandle(pCreateInfo->renderPass).c_str(), highest_view_bit);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if ((pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) == 0) {
                // Verify correct attachment usage flags
                for (uint32_t subpass = 0; subpass < rpci->subpassCount; subpass++) {
                    const VkSubpassDescription2 &subpass_description = rpci->pSubpasses[subpass];
                    const auto *ms_rendered_to_single_sampled =
                        LvlFindInChain<VkMultisampledRenderToSingleSampledInfoEXT>(subpass_description.pNext);
                    // Verify input attachments:
                    skip |= MatchUsage(subpass_description.inputAttachmentCount, subpass_description.pInputAttachments, pCreateInfo,
                                       VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, "VUID-VkFramebufferCreateInfo-pAttachments-00879");
                    // Verify color attachments:
                    skip |= MatchUsage(subpass_description.colorAttachmentCount, subpass_description.pColorAttachments, pCreateInfo,
                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, "VUID-VkFramebufferCreateInfo-pAttachments-00877");
                    // Verify depth/stencil attachments:
                    skip |=
                        MatchUsage(1, subpass_description.pDepthStencilAttachment, pCreateInfo,
                                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, "VUID-VkFramebufferCreateInfo-pAttachments-02633");
                    // Verify depth/stecnil resolve
                    if (IsExtEnabled(device_extensions.vk_khr_depth_stencil_resolve)) {
                        const VkSubpassDescriptionDepthStencilResolve *ds_resolve =
                            LvlFindInChain<VkSubpassDescriptionDepthStencilResolve>(subpass_description.pNext);
                        if (ds_resolve) {
                            skip |= MatchUsage(1, ds_resolve->pDepthStencilResolveAttachment, pCreateInfo,
                                               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                               "VUID-VkFramebufferCreateInfo-pAttachments-02634");
                        }
                    }

                    // Verify fragment shading rate attachments
                    if (enabled_features.fragment_shading_rate_features.attachmentFragmentShadingRate) {
                        const VkFragmentShadingRateAttachmentInfoKHR *fragment_shading_rate_attachment_info =
                            LvlFindInChain<VkFragmentShadingRateAttachmentInfoKHR>(subpass_description.pNext);
                        if (fragment_shading_rate_attachment_info) {
                            skip |= MatchUsage(1, fragment_shading_rate_attachment_info->pFragmentShadingRateAttachment,
                                               pCreateInfo, VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
                                               "VUID-VkFramebufferCreateInfo-flags-04548");
                        }
                    }
                    if (ms_rendered_to_single_sampled && ms_rendered_to_single_sampled->multisampledRenderToSingleSampledEnable) {
                        skip |= MsRenderedToSingleSampledValidateFBAttachments(
                            subpass_description.inputAttachmentCount, subpass_description.pInputAttachments, pCreateInfo, rpci,
                            subpass, ms_rendered_to_single_sampled->rasterizationSamples);
                        skip |= MsRenderedToSingleSampledValidateFBAttachments(
                            subpass_description.colorAttachmentCount, subpass_description.pColorAttachments, pCreateInfo, rpci,
                            subpass, ms_rendered_to_single_sampled->rasterizationSamples);
                        if (subpass_description.pDepthStencilAttachment) {
                            skip |= MsRenderedToSingleSampledValidateFBAttachments(
                                1, subpass_description.pDepthStencilAttachment, pCreateInfo, rpci, subpass,
                                ms_rendered_to_single_sampled->rasterizationSamples);
                        }
                    }
                }
            }

            if (b_has_non_zero_view_masks && pCreateInfo->layers != 1) {
                skip |= LogError(pCreateInfo->renderPass, "VUID-VkFramebufferCreateInfo-renderPass-02531",
                                 "vkCreateFramebuffer(): VkFramebufferCreateInfo has #%u layers but "
                                 "renderPass (%s) was specified with non-zero view masks\n",
                                 pCreateInfo->layers, report_data->FormatHandle(pCreateInfo->renderPass).c_str());
            }
        }
    }
    // Verify FB dimensions are within physical device limits
    if (pCreateInfo->width > phys_dev_props.limits.maxFramebufferWidth) {
        skip |= LogError(device, "VUID-VkFramebufferCreateInfo-width-00886",
                         "vkCreateFramebuffer(): Requested VkFramebufferCreateInfo width exceeds physical device limits. Requested "
                         "width: %u, device max: %u\n",
                         pCreateInfo->width, phys_dev_props.limits.maxFramebufferWidth);
    }
    if (pCreateInfo->height > phys_dev_props.limits.maxFramebufferHeight) {
        skip |=
            LogError(device, "VUID-VkFramebufferCreateInfo-height-00888",
                     "vkCreateFramebuffer(): Requested VkFramebufferCreateInfo height exceeds physical device limits. Requested "
                     "height: %u, device max: %u\n",
                     pCreateInfo->height, phys_dev_props.limits.maxFramebufferHeight);
    }
    if (pCreateInfo->layers > phys_dev_props.limits.maxFramebufferLayers) {
        skip |=
            LogError(device, "VUID-VkFramebufferCreateInfo-layers-00890",
                     "vkCreateFramebuffer(): Requested VkFramebufferCreateInfo layers exceeds physical device limits. Requested "
                     "layers: %u, device max: %u\n",
                     pCreateInfo->layers, phys_dev_props.limits.maxFramebufferLayers);
    }
    // Verify FB dimensions are greater than zero
    if (pCreateInfo->width <= 0) {
        skip |= LogError(device, "VUID-VkFramebufferCreateInfo-width-00885",
                         "vkCreateFramebuffer(): Requested VkFramebufferCreateInfo width must be greater than zero.");
    }
    if (pCreateInfo->height <= 0) {
        skip |= LogError(device, "VUID-VkFramebufferCreateInfo-height-00887",
                         "vkCreateFramebuffer(): Requested VkFramebufferCreateInfo height must be greater than zero.");
    }
    if (pCreateInfo->layers <= 0) {
        skip |= LogError(device, "VUID-VkFramebufferCreateInfo-layers-00889",
                         "vkCreateFramebuffer(): Requested VkFramebufferCreateInfo layers must be greater than zero.");
    }
    return skip;
}

bool CoreChecks::MsRenderedToSingleSampledValidateFBAttachments(uint32_t count, const VkAttachmentReference2 *attachments,
                                                                const VkFramebufferCreateInfo *fbci,
                                                                const VkRenderPassCreateInfo2 *rpci, uint32_t subpass,
                                                                VkSampleCountFlagBits sample_count) const {
    bool skip = false;

    for (uint32_t attach = 0; attach < count; attach++) {
        if (attachments[attach].attachment != VK_ATTACHMENT_UNUSED) {
            if (attachments[attach].attachment < fbci->attachmentCount) {
                const auto renderpass_samples = rpci->pAttachments[attachments[attach].attachment].samples;
                if (renderpass_samples == VK_SAMPLE_COUNT_1_BIT) {
                    const VkImageView *image_view = &fbci->pAttachments[attachments[attach].attachment];
                    auto view_state = Get<IMAGE_VIEW_STATE>(*image_view);
                    auto image_state = view_state->image_state;
                    if (!(image_state->createInfo.flags & VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT)) {
                        skip |= LogError(device, "VUID-VkFramebufferCreateInfo-samples-06881",
                                         "vkCreateFramebuffer(): Renderpass subpass %" PRIu32
                                         " enables "
                                         "multisampled-render-to-single-sampled and attachment %" PRIu32
                                         ", is specified from with "
                                         "VK_SAMPLE_COUNT_1_BIT samples, but image (%s) was created without "
                                         "VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT in its createInfo.flags.",
                                         subpass, attachments[attach].attachment,
                                         report_data->FormatHandle(image_state->Handle()).c_str());
                    }
                    const VkImageCreateInfo image_create_info = image_state->createInfo;
                    if (!image_state->image_format_properties.sampleCounts) {
                        skip |= GetPhysicalDeviceImageFormatProperties(*image_state.get(),
                                                                       "VUID-VkFramebufferCreateInfo-samples-07009");
                    }
                    if (!(image_state->image_format_properties.sampleCounts & sample_count)) {
                        skip |= LogError(
                            device, "VUID-VkFramebufferCreateInfo-samples-07009",
                            "vkCreateFramebuffer(): Renderpass subpass %" PRIu32
                            " enables "
                            "multisampled-render-to-single-sampled and attachment %" PRIu32
                            ", is specified from with "
                            "VK_SAMPLE_COUNT_1_BIT samples, but image (%s) created with format %s imageType: %s, "
                            "tiling: %s, usage: %s, "
                            "flags: %s does not support a rasterizationSamples count of %s",
                            subpass, attachments[attach].attachment, report_data->FormatHandle(image_state->Handle()).c_str(),
                            string_VkFormat(image_create_info.format), string_VkImageType(image_create_info.imageType),
                            string_VkImageTiling(image_create_info.tiling),
                            string_VkImageUsageFlags(image_create_info.usage).c_str(),
                            string_VkImageCreateFlags(image_create_info.flags).c_str(), string_VkSampleCountFlagBits(sample_count));
                    }
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer) const {
    // TODO : Verify that renderPass FB is created with is compatible with FB
    bool skip = false;
    skip |= ValidateFramebufferCreateInfo(pCreateInfo);
    return skip;
}

bool CoreChecks::ValidatePrimaryCommandBuffer(const CMD_BUFFER_STATE &cb_state, char const *cmd_name,
                                              const char *error_code) const {
    bool skip = false;
    if (cb_state.createInfo.level != VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
        skip |=
            LogError(cb_state.commandBuffer(), error_code, "Cannot execute command %s on a secondary command buffer.", cmd_name);
    }
    return skip;
}

bool CoreChecks::ValidateFramebuffer(VkCommandBuffer primaryBuffer, const CMD_BUFFER_STATE &cb_state,
                                     VkCommandBuffer secondaryBuffer, const CMD_BUFFER_STATE &sub_cb_state,
                                     const char *caller) const {
    bool skip = false;
    if (!sub_cb_state.beginInfo.pInheritanceInfo) {
        return skip;
    }
    VkFramebuffer primary_fb = cb_state.activeFramebuffer ? cb_state.activeFramebuffer->framebuffer() : VK_NULL_HANDLE;
    VkFramebuffer secondary_fb = sub_cb_state.beginInfo.pInheritanceInfo->framebuffer;
    if (secondary_fb != VK_NULL_HANDLE) {
        if (primary_fb != secondary_fb) {
            const LogObjectList objlist(primaryBuffer, secondaryBuffer, secondary_fb, primary_fb);
            skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-pCommandBuffers-00099",
                             "vkCmdExecuteCommands() called w/ invalid secondary %s which has a %s"
                             " that is not the same as the primary command buffer's current active %s.",
                             report_data->FormatHandle(secondaryBuffer).c_str(), report_data->FormatHandle(secondary_fb).c_str(),
                             report_data->FormatHandle(primary_fb).c_str());
        }
        auto fb = Get<FRAMEBUFFER_STATE>(secondary_fb);
        if (!fb) {
            const LogObjectList objlist(primaryBuffer, secondaryBuffer, secondary_fb);
            skip |= LogError(objlist, kVUID_Core_DrawState_InvalidSecondaryCommandBuffer,
                             "vkCmdExecuteCommands() called w/ invalid %s which has invalid %s.",
                             report_data->FormatHandle(secondaryBuffer).c_str(), report_data->FormatHandle(secondary_fb).c_str());
            return skip;
        }
    }
    return skip;
}

bool CoreChecks::ValidateSecondaryCommandBufferState(const CMD_BUFFER_STATE &cb_state, const CMD_BUFFER_STATE &sub_cb_state) const {
    bool skip = false;

    layer_data::unordered_set<int> active_types;
    if (!disabled[query_validation]) {
        for (const auto &query_object : cb_state.activeQueries) {
            auto query_pool_state = Get<QUERY_POOL_STATE>(query_object.pool);
            if (query_pool_state) {
                if (query_pool_state->createInfo.queryType == VK_QUERY_TYPE_PIPELINE_STATISTICS &&
                    sub_cb_state.beginInfo.pInheritanceInfo) {
                    VkQueryPipelineStatisticFlags cmd_buf_statistics = sub_cb_state.beginInfo.pInheritanceInfo->pipelineStatistics;
                    if ((cmd_buf_statistics & query_pool_state->createInfo.pipelineStatistics) != cmd_buf_statistics) {
                        const LogObjectList objlist(cb_state.commandBuffer(), query_object.pool);
                        skip |= LogError(
                            objlist, "VUID-vkCmdExecuteCommands-commandBuffer-00104",
                            "vkCmdExecuteCommands() called w/ invalid %s which has invalid active %s"
                            ". Pipeline statistics is being queried so the command buffer must have all bits set on the queryPool.",
                            report_data->FormatHandle(cb_state.commandBuffer()).c_str(),
                            report_data->FormatHandle(query_object.pool).c_str());
                    }
                }
                active_types.insert(query_pool_state->createInfo.queryType);
            }
        }
        for (const auto &query_object : sub_cb_state.startedQueries) {
            auto query_pool_state = Get<QUERY_POOL_STATE>(query_object.pool);
            if (query_pool_state && active_types.count(query_pool_state->createInfo.queryType)) {
                const LogObjectList objlist(cb_state.commandBuffer(), query_object.pool);
                skip |= LogError(objlist, kVUID_Core_DrawState_InvalidSecondaryCommandBuffer,
                                 "vkCmdExecuteCommands() called w/ invalid %s which has invalid active %s"
                                 " of type %d but a query of that type has been started on secondary %s.",
                                 report_data->FormatHandle(cb_state.commandBuffer()).c_str(),
                                 report_data->FormatHandle(query_object.pool).c_str(), query_pool_state->createInfo.queryType,
                                 report_data->FormatHandle(sub_cb_state.commandBuffer()).c_str());
            }
        }
    }
    const auto primary_pool = cb_state.command_pool;
    const auto secondary_pool = sub_cb_state.command_pool;
    if (primary_pool && secondary_pool && (primary_pool->queueFamilyIndex != secondary_pool->queueFamilyIndex)) {
        const LogObjectList objlist(sub_cb_state.commandBuffer(), cb_state.commandBuffer());
        skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-pCommandBuffers-00094",
                         "vkCmdExecuteCommands(): Primary %s created in queue family %d has secondary "
                         "%s created in queue family %d.",
                         report_data->FormatHandle(cb_state.commandBuffer()).c_str(), primary_pool->queueFamilyIndex,
                         report_data->FormatHandle(sub_cb_state.commandBuffer()).c_str(), secondary_pool->queueFamilyIndex);
    }

    return skip;
}

// Object that simulates the inherited viewport/scissor state as the device executes the called secondary command buffers.
// Visit the calling primary command buffer first, then the called secondaries in order.
// Contact David Zhao Akeley <dakeley@nvidia.com> for clarifications and bug fixes.
class CoreChecks::ViewportScissorInheritanceTracker {
    static_assert(4 == sizeof(CMD_BUFFER_STATE::viewportMask), "Adjust max_viewports to match viewportMask bit width");
    static constexpr uint32_t kMaxViewports = 32, kNotTrashed = uint32_t(-2), kTrashedByPrimary = uint32_t(-1);

    const ValidationObject &validation_;
    const CMD_BUFFER_STATE *primary_state_ = nullptr;
    uint32_t viewport_mask_;
    uint32_t scissor_mask_;
    uint32_t viewport_trashed_by_[kMaxViewports];  // filled in VisitPrimary.
    uint32_t scissor_trashed_by_[kMaxViewports];
    VkViewport viewports_to_inherit_[kMaxViewports];
    uint32_t viewport_count_to_inherit_;  // 0 if viewport count (EXT state) has never been defined (but not trashed)
    uint32_t scissor_count_to_inherit_;   // 0 if scissor count (EXT state) has never been defined (but not trashed)
    uint32_t viewport_count_trashed_by_;
    uint32_t scissor_count_trashed_by_;

  public:
    ViewportScissorInheritanceTracker(const ValidationObject &validation) : validation_(validation) {}

    bool VisitPrimary(const CMD_BUFFER_STATE *primary_state) {
        assert(!primary_state_);
        primary_state_ = primary_state;

        viewport_mask_ = primary_state->viewportMask | primary_state->viewportWithCountMask;
        scissor_mask_ = primary_state->scissorMask | primary_state->scissorWithCountMask;

        for (uint32_t n = 0; n < kMaxViewports; ++n) {
            uint32_t bit = uint32_t(1) << n;
            viewport_trashed_by_[n] = primary_state->trashedViewportMask & bit ? kTrashedByPrimary : kNotTrashed;
            scissor_trashed_by_[n] = primary_state->trashedScissorMask & bit ? kTrashedByPrimary : kNotTrashed;
            if (viewport_mask_ & bit) {
                viewports_to_inherit_[n] = primary_state->dynamicViewports[n];
            }
        }

        viewport_count_to_inherit_ = primary_state->viewportWithCountCount;
        scissor_count_to_inherit_ = primary_state->scissorWithCountCount;
        viewport_count_trashed_by_ = primary_state->trashedViewportCount ? kTrashedByPrimary : kNotTrashed;
        scissor_count_trashed_by_ = primary_state->trashedScissorCount ? kTrashedByPrimary : kNotTrashed;
        return false;
    }

    bool VisitSecondary(uint32_t cmd_buffer_idx, const CMD_BUFFER_STATE *secondary_state) {
        bool skip = false;
        if (secondary_state->inheritedViewportDepths.empty()) {
            skip |= VisitSecondaryNoInheritance(cmd_buffer_idx, secondary_state);
        } else {
            skip |= VisitSecondaryInheritance(cmd_buffer_idx, secondary_state);
        }

        // See note at end of VisitSecondaryNoInheritance.
        if (secondary_state->trashedViewportCount) {
            viewport_count_trashed_by_ = cmd_buffer_idx;
        }
        if (secondary_state->trashedScissorCount) {
            scissor_count_trashed_by_ = cmd_buffer_idx;
        }
        return skip;
    }

  private:
    // Track state inheritance as specified by VK_NV_inherited_scissor_viewport, including states
    // overwritten to undefined value by bound pipelines with non-dynamic state.
    bool VisitSecondaryNoInheritance(uint32_t cmd_buffer_idx, const CMD_BUFFER_STATE *secondary_state) {
        viewport_mask_ |= secondary_state->viewportMask | secondary_state->viewportWithCountMask;
        scissor_mask_ |= secondary_state->scissorMask | secondary_state->scissorWithCountMask;

        for (uint32_t n = 0; n < kMaxViewports; ++n) {
            uint32_t bit = uint32_t(1) << n;
            if ((secondary_state->viewportMask | secondary_state->viewportWithCountMask) & bit) {
                viewports_to_inherit_[n] = secondary_state->dynamicViewports[n];
                viewport_trashed_by_[n] = kNotTrashed;
            }
            if ((secondary_state->scissorMask | secondary_state->scissorWithCountMask) & bit) {
                scissor_trashed_by_[n] = kNotTrashed;
            }
            if (secondary_state->viewportWithCountCount != 0) {
                viewport_count_to_inherit_ = secondary_state->viewportWithCountCount;
                viewport_count_trashed_by_ = kNotTrashed;
            }
            if (secondary_state->scissorWithCountCount != 0) {
                scissor_count_to_inherit_ = secondary_state->scissorWithCountCount;
                scissor_count_trashed_by_ = kNotTrashed;
            }
            // Order of above vs below matters here.
            if (secondary_state->trashedViewportMask & bit) {
                viewport_trashed_by_[n] = cmd_buffer_idx;
            }
            if (secondary_state->trashedScissorMask & bit) {
                scissor_trashed_by_[n] = cmd_buffer_idx;
            }
            // Check trashing dynamic viewport/scissor count in VisitSecondary (at end) as even secondary command buffers enabling
            // viewport/scissor state inheritance may define this state statically in bound graphics pipelines.
        }
        return false;
    }

    // Validate needed inherited state as specified by VK_NV_inherited_scissor_viewport.
    bool VisitSecondaryInheritance(uint32_t cmd_buffer_idx, const CMD_BUFFER_STATE *secondary_state) {
        bool skip = false;
        uint32_t check_viewport_count = 0, check_scissor_count = 0;

        // Common code for reporting missing inherited state (for a myriad of reasons).
        auto check_missing_inherit = [&](uint32_t was_ever_defined, uint32_t trashed_by, VkDynamicState state, uint32_t index = 0,
                                         uint32_t static_use_count = 0, const VkViewport *inherited_viewport = nullptr,
                                         const VkViewport *expected_viewport_depth = nullptr) {
            if (was_ever_defined && trashed_by == kNotTrashed) {
                if (state != VK_DYNAMIC_STATE_VIEWPORT) return false;

                assert(inherited_viewport != nullptr && expected_viewport_depth != nullptr);
                if (inherited_viewport->minDepth != expected_viewport_depth->minDepth ||
                    inherited_viewport->maxDepth != expected_viewport_depth->maxDepth) {
                    return validation_.LogError(
                        primary_state_->commandBuffer(), "VUID-vkCmdDraw-None-07850",
                        "vkCmdExecuteCommands(): Draw commands in pCommandBuffers[%u] (%s) consume inherited viewport %u %s"
                        "but this state was not inherited as its depth range [%f, %f] does not match "
                        "pViewportDepths[%u] = [%f, %f]",
                        unsigned(cmd_buffer_idx), validation_.report_data->FormatHandle(secondary_state->commandBuffer()).c_str(),
                        unsigned(index), index >= static_use_count ? "(with count) " : "", inherited_viewport->minDepth,
                        inherited_viewport->maxDepth, unsigned(cmd_buffer_idx), expected_viewport_depth->minDepth,
                        expected_viewport_depth->maxDepth);
                    // akeley98 note: This VUID is not ideal; however, there isn't a more relevant VUID as
                    // it isn't illegal in itself to have mismatched inherited viewport depths.
                    // The error only occurs upon attempting to consume the viewport.
                } else {
                    return false;
                }
            }

            const char *state_name;
            bool format_index = false;

            switch (state) {
                case VK_DYNAMIC_STATE_SCISSOR:
                    state_name = "scissor";
                    format_index = true;
                    break;
                case VK_DYNAMIC_STATE_VIEWPORT:
                    state_name = "viewport";
                    format_index = true;
                    break;
                case VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT:
                    state_name = "dynamic viewport count";
                    break;
                case VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT:
                    state_name = "dynamic scissor count";
                    break;
                default:
                    assert(0);
                    state_name = "<unknown state, report bug>";
                    break;
            }

            std::stringstream ss;
            ss << "vkCmdExecuteCommands(): Draw commands in pCommandBuffers[" << cmd_buffer_idx << "] ("
               << validation_.report_data->FormatHandle(secondary_state->commandBuffer()).c_str() << ") consume inherited "
               << state_name << " ";
            if (format_index) {
                if (index >= static_use_count) {
                    ss << "(with count) ";
                }
                ss << index << " ";
            }
            ss << "but this state ";
            if (!was_ever_defined) {
                ss << "was never defined.";
            } else if (trashed_by == kTrashedByPrimary) {
                ss << "was left undefined after vkCmdExecuteCommands or vkCmdBindPipeline (with non-dynamic state) in "
                      "the calling primary command buffer.";
            } else {
                ss << "was left undefined after vkCmdBindPipeline (with non-dynamic state) in pCommandBuffers[" << trashed_by
                   << "].";
            }
            return validation_.LogError(primary_state_->commandBuffer(), "VUID-vkCmdDraw-None-07850", "%s", ss.str().c_str());
        };

        // Check if secondary command buffer uses viewport/scissor-with-count state, and validate this state if so.
        if (secondary_state->usedDynamicViewportCount) {
            if (viewport_count_to_inherit_ == 0 || viewport_count_trashed_by_ != kNotTrashed) {
                skip |= check_missing_inherit(viewport_count_to_inherit_, viewport_count_trashed_by_,
                                              VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT);
            } else {
                check_viewport_count = viewport_count_to_inherit_;
            }
        }
        if (secondary_state->usedDynamicScissorCount) {
            if (scissor_count_to_inherit_ == 0 || scissor_count_trashed_by_ != kNotTrashed) {
                skip |= check_missing_inherit(scissor_count_to_inherit_, scissor_count_trashed_by_,
                                              VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT);
            } else {
                check_scissor_count = scissor_count_to_inherit_;
            }
        }

        // Check the maximum of (viewports used by pipelines with static viewport count, "" dynamic viewport count)
        // but limit to length of inheritedViewportDepths array and uint32_t bit width (validation layer limit).
        check_viewport_count = std::min(std::min(kMaxViewports, uint32_t(secondary_state->inheritedViewportDepths.size())),
                                        std::max(check_viewport_count, secondary_state->usedViewportScissorCount));
        check_scissor_count = std::min(kMaxViewports, std::max(check_scissor_count, secondary_state->usedViewportScissorCount));

        if (secondary_state->usedDynamicViewportCount &&
            viewport_count_to_inherit_ > secondary_state->inheritedViewportDepths.size()) {
            skip |= validation_.LogError(
                primary_state_->commandBuffer(), "VUID-vkCmdDraw-None-07850",
                "vkCmdExecuteCommands(): "
                "Draw commands in pCommandBuffers[%u] (%s) consume inherited dynamic viewport with count state "
                "but the dynamic viewport count (%u) exceeds the inheritance limit (viewportDepthCount=%u).",
                unsigned(cmd_buffer_idx), validation_.report_data->FormatHandle(secondary_state->commandBuffer()).c_str(),
                unsigned(viewport_count_to_inherit_), unsigned(secondary_state->inheritedViewportDepths.size()));
        }

        for (uint32_t n = 0; n < check_viewport_count; ++n) {
            skip |= check_missing_inherit(viewport_mask_ & uint32_t(1) << n, viewport_trashed_by_[n], VK_DYNAMIC_STATE_VIEWPORT, n,
                                          secondary_state->usedViewportScissorCount, &viewports_to_inherit_[n],
                                          &secondary_state->inheritedViewportDepths[n]);
        }

        for (uint32_t n = 0; n < check_scissor_count; ++n) {
            skip |= check_missing_inherit(scissor_mask_ & uint32_t(1) << n, scissor_trashed_by_[n], VK_DYNAMIC_STATE_SCISSOR, n,
                                          secondary_state->usedViewportScissorCount);
        }
        return skip;
    }
};

constexpr uint32_t CoreChecks::ViewportScissorInheritanceTracker::kMaxViewports;
constexpr uint32_t CoreChecks::ViewportScissorInheritanceTracker::kNotTrashed;
constexpr uint32_t CoreChecks::ViewportScissorInheritanceTracker::kTrashedByPrimary;

bool CoreChecks::PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount,
                                                   const VkCommandBuffer *pCommandBuffers) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    bool skip = false;
    layer_data::unordered_set<const CMD_BUFFER_STATE *> linked_command_buffers;
    ViewportScissorInheritanceTracker viewport_scissor_inheritance{*this};

    if (enabled_features.inherited_viewport_scissor_features.inheritedViewportScissor2D) {
        skip |= viewport_scissor_inheritance.VisitPrimary(cb_state.get());
    }

    bool active_occlusion_query = false;
    for (const auto &active_query : cb_state->activeQueries) {
        auto query_pool_state = Get<QUERY_POOL_STATE>(active_query.pool);
        if (query_pool_state->createInfo.queryType == VK_QUERY_TYPE_OCCLUSION) {
            active_occlusion_query = true;
            break;
        }
    }

    if (cb_state->activeRenderPass) {
        if (!cb_state->activeRenderPass->UsesDynamicRendering() &&
            (cb_state->activeSubpassContents != VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS)) {
            skip |= LogError(commandBuffer, "VUID-vkCmdExecuteCommands-contents-06018",
                             "vkCmdExecuteCommands(): contents must be set to VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS "
                             "when calling vkCmdExecuteCommands() within a render pass instance begun with "
                             "vkCmdBeginRenderPass().");
        }

        if (cb_state->activeRenderPass->UsesDynamicRendering() &&
            !(cb_state->activeRenderPass->dynamic_rendering_begin_rendering_info.flags &
              VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR)) {
            skip |= LogError(commandBuffer, "VUID-vkCmdExecuteCommands-flags-06024",
                             "vkCmdExecuteCommands(): VkRenderingInfo::flags must include "
                             "VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR when calling vkCmdExecuteCommands() within a "
                             "render pass instance begun with %s().",
                             cb_state->begin_rendering_func_name.c_str());
        }
    }

    for (uint32_t i = 0; i < commandBuffersCount; i++) {
        auto sub_cb_state = GetRead<CMD_BUFFER_STATE>(pCommandBuffers[i]);
        assert(sub_cb_state);

        if (enabled_features.inherited_viewport_scissor_features.inheritedViewportScissor2D) {
            skip |= viewport_scissor_inheritance.VisitSecondary(i, sub_cb_state.get());
        }

        if (VK_COMMAND_BUFFER_LEVEL_PRIMARY == sub_cb_state->createInfo.level) {
            skip |= LogError(pCommandBuffers[i], "VUID-vkCmdExecuteCommands-pCommandBuffers-00088",
                             "vkCmdExecuteCommands() called w/ Primary %s in element %u of pCommandBuffers array. All "
                             "cmd buffers in pCommandBuffers array must be secondary.",
                             report_data->FormatHandle(pCommandBuffers[i]).c_str(), i);
        } else if (VK_COMMAND_BUFFER_LEVEL_SECONDARY == sub_cb_state->createInfo.level) {
            if (!cb_state->activeRenderPass) {
                if (sub_cb_state->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) {
                    skip |= LogError(pCommandBuffers[i], "VUID-vkCmdExecuteCommands-pCommandBuffers-00100",
                                     "vkCmdExecuteCommands(): Secondary %s is executed outside a render pass "
                                     "instance scope, but the Secondary Command Buffer does have the "
                                     "VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT set in VkCommandBufferBeginInfo::flags when "
                                     "the vkBeginCommandBuffer() was called.",
                                     report_data->FormatHandle(pCommandBuffers[i]).c_str());
                }
            } else if (sub_cb_state->beginInfo.pInheritanceInfo != nullptr) {
                const uint32_t inheritance_subpass = sub_cb_state->beginInfo.pInheritanceInfo->subpass;
                const VkRenderPass inheritance_render_pass = sub_cb_state->beginInfo.pInheritanceInfo->renderPass;
                auto secondary_rp_state = Get<RENDER_PASS_STATE>(inheritance_render_pass);
                if (!(sub_cb_state->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)) {
                    const LogObjectList objlist(pCommandBuffers[i], cb_state->activeRenderPass->renderPass());
                    skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-pCommandBuffers-00096",
                                     "vkCmdExecuteCommands(): Secondary %s is executed within a %s "
                                     "instance scope, but the Secondary Command Buffer does not have the "
                                     "VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT set in VkCommandBufferBeginInfo::flags when "
                                     "the vkBeginCommandBuffer() was called.",
                                     report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                     report_data->FormatHandle(cb_state->activeRenderPass->renderPass()).c_str());
                } else if (!cb_state->activeRenderPass->UsesDynamicRendering() &&
                           (sub_cb_state->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)) {
                    // Make sure render pass is compatible with parent command buffer pass if has continue
                    if (cb_state->activeRenderPass->renderPass() != secondary_rp_state->renderPass()) {
                        skip |= ValidateRenderPassCompatibility(
                            "primary command buffer", *cb_state->activeRenderPass.get(), "secondary command buffer",
                            *secondary_rp_state.get(), "vkCmdExecuteCommands()", "VUID-vkCmdExecuteCommands-pBeginInfo-06020");
                    }
                    //  If framebuffer for secondary CB is not NULL, then it must match active FB from primaryCB
                    skip |=
                        ValidateFramebuffer(commandBuffer, *cb_state, pCommandBuffers[i], *sub_cb_state, "vkCmdExecuteCommands()");
                    if (!sub_cb_state->cmd_execute_commands_functions.empty()) {
                        //  Inherit primary's activeFramebuffer and while running validate functions
                        for (auto &function : sub_cb_state->cmd_execute_commands_functions) {
                            skip |= function(*sub_cb_state, cb_state.get(), cb_state->activeFramebuffer.get());
                        }
                    }
                }

                if (!IsExtEnabled(device_extensions.vk_khr_dynamic_rendering)) {
                    if (cb_state->activeRenderPass->renderPass() != secondary_rp_state->renderPass()) {
                        skip |= ValidateRenderPassCompatibility("primary command buffer", *cb_state->activeRenderPass.get(),
                                                                "secondary command buffer", *secondary_rp_state.get(),
                                                                "vkCmdExecuteCommands()",
                                                                "VUID-vkCmdExecuteCommands-pInheritanceInfo-00098");
                    }
                    if (inheritance_subpass != cb_state->activeSubpass) {
                        skip |= LogError(pCommandBuffers[i], "VUID-vkCmdExecuteCommands-pCommandBuffers-00097",
                                         "vkCmdExecuteCommands(): Secondary %s subpass %" PRIu32
                                         " is different than the active subpass %" PRIu32 ".",
                                         report_data->FormatHandle(pCommandBuffers[i]).c_str(), inheritance_subpass,
                                         cb_state->activeSubpass);
                    }
                    if (cb_state->activeSubpassContents != VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS) {
                        skip |= LogError(pCommandBuffers[i], "VUID-vkCmdExecuteCommands-contents-00095",
                                         "vkCmdExecuteCommands(): render pass instance is active, but was not begun with "
                                         "VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS.");
                    }
                }

                if (!cb_state->activeRenderPass->UsesDynamicRendering() && (cb_state->activeSubpass != inheritance_subpass)) {
                    const LogObjectList objlist(pCommandBuffers[i], cb_state->activeRenderPass->renderPass());
                    skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-pCommandBuffers-06019",
                                     "vkCmdExecuteCommands(): Secondary %s is executed within a %s "
                                     "instance scope begun by vkCmdBeginRenderPass(), but "
                                     "VkCommandBufferInheritanceInfo::subpass (%u) does not "
                                     "match the current subpass (%u).",
                                     report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                     report_data->FormatHandle(cb_state->activeRenderPass->renderPass()).c_str(),
                                     inheritance_subpass, cb_state->activeSubpass);
                } else if (cb_state->activeRenderPass->UsesDynamicRendering()) {
                    if (inheritance_render_pass != VK_NULL_HANDLE) {
                        skip |= LogError(
                            pCommandBuffers[i], "VUID-vkCmdExecuteCommands-pBeginInfo-06025",
                            "vkCmdExecuteCommands(): Secondary %s is executed within a dynamic renderpass instance scope begun "
                            "by %s(), but "
                            "VkCommandBufferInheritanceInfo::pInheritanceInfo::renderPass is not VK_NULL_HANDLE.",
                            report_data->FormatHandle(pCommandBuffers[i]).c_str(), cb_state->begin_rendering_func_name.c_str());
                    }

                    if (sub_cb_state->activeRenderPass->use_dynamic_rendering_inherited) {
                        const auto rendering_info = cb_state->activeRenderPass->dynamic_rendering_begin_rendering_info;
                        const auto inheritance_rendering_info = sub_cb_state->activeRenderPass->inheritance_rendering_info;
                        if (inheritance_rendering_info.flags !=
                            (rendering_info.flags & ~VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR)) {
                            skip |= LogError(
                                pCommandBuffers[i], "VUID-vkCmdExecuteCommands-flags-06026",
                                "vkCmdExecuteCommands(): Secondary %s is executed within a dynamic renderpass instance scope begun "
                                "by %s(), but VkCommandBufferInheritanceRenderingInfo::flags (%u) does "
                                "not match VkRenderingInfo::flags (%u), excluding "
                                "VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR.",
                                report_data->FormatHandle(pCommandBuffers[i]).c_str(), cb_state->begin_rendering_func_name.c_str(),
                                inheritance_rendering_info.flags, rendering_info.flags);
                        }

                        if (inheritance_rendering_info.colorAttachmentCount != rendering_info.colorAttachmentCount) {
                            skip |= LogError(
                                pCommandBuffers[i], "VUID-vkCmdExecuteCommands-colorAttachmentCount-06027",
                                "vkCmdExecuteCommands(): Secondary %s is executed within a dynamic renderpass instance scope begun "
                                "by %s(), but "
                                "VkCommandBufferInheritanceRenderingInfo::colorAttachmentCount (%u) does "
                                "not match VkRenderingInfo::colorAttachmentCount (%u).",
                                report_data->FormatHandle(pCommandBuffers[i]).c_str(), cb_state->begin_rendering_func_name.c_str(),
                                inheritance_rendering_info.colorAttachmentCount, rendering_info.colorAttachmentCount);
                        }

                        for (uint32_t index = 0; index < rendering_info.colorAttachmentCount; index++) {
                            if (rendering_info.pColorAttachments[index].imageView == VK_NULL_HANDLE) {
                                continue;
                            }
                            auto image_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pColorAttachments[index].imageView);

                            if (image_view_state->create_info.format != inheritance_rendering_info.pColorAttachmentFormats[index]) {
                                skip |= LogError(
                                    pCommandBuffers[i], "VUID-vkCmdExecuteCommands-imageView-06028",
                                    "vkCmdExecuteCommands(): Secondary %s is executed within a dynamic renderpass instance "
                                    "scope begun "
                                    "by %s(), but "
                                    "VkCommandBufferInheritanceRenderingInfo::pColorAttachmentFormats at index (%u) does "
                                    "not match the format of the imageView in VkRenderingInfo::pColorAttachments.",
                                    report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                    cb_state->begin_rendering_func_name.c_str(), index);
                            }
                        }

                        if ((rendering_info.pDepthAttachment != nullptr) &&
                            rendering_info.pDepthAttachment->imageView != VK_NULL_HANDLE) {
                            auto image_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pDepthAttachment->imageView);

                            if (image_view_state->create_info.format != inheritance_rendering_info.depthAttachmentFormat) {
                                skip |= LogError(pCommandBuffers[i], "VUID-vkCmdExecuteCommands-pDepthAttachment-06029",
                                                 "vkCmdExecuteCommands(): Secondary %s is executed within a dynamic renderpass "
                                                 "instance scope begun "
                                                 "by %s(), but "
                                                 "VkCommandBufferInheritanceRenderingInfo::depthAttachmentFormat does "
                                                 "not match the format of the imageView in VkRenderingInfo::pDepthAttachment.",
                                                 report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                                 cb_state->begin_rendering_func_name.c_str());
                            }
                        }

                        if ((rendering_info.pStencilAttachment != nullptr) &&
                            rendering_info.pStencilAttachment->imageView != VK_NULL_HANDLE) {
                            auto image_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pStencilAttachment->imageView);

                            if (image_view_state->create_info.format != inheritance_rendering_info.stencilAttachmentFormat) {
                                skip |= LogError(pCommandBuffers[i], "VUID-vkCmdExecuteCommands-pStencilAttachment-06030",
                                                 "vkCmdExecuteCommands(): Secondary %s is executed within a dynamic renderpass "
                                                 "instance scope begun "
                                                 "by %s(), but "
                                                 "VkCommandBufferInheritanceRenderingInfo::stencilAttachmentFormat does "
                                                 "not match the format of the imageView in VkRenderingInfo::pStencilAttachment.",
                                                 report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                                 cb_state->begin_rendering_func_name.c_str());
                            }
                        }

                        if (rendering_info.pDepthAttachment == nullptr ||
                            rendering_info.pDepthAttachment->imageView == VK_NULL_HANDLE) {
                            VkFormat format = inheritance_rendering_info.depthAttachmentFormat;
                            if (format != VK_FORMAT_UNDEFINED) {
                                skip |= LogError(
                                    pCommandBuffers[i], "VUID-vkCmdExecuteCommands-pDepthAttachment-06774",
                                    "vkCmdExecuteCommands(): Secondary %s is executed within a dynamic renderpass "
                                    "instance scope begun by %s(), and VkRenderingInfo::pDepthAttachment does not define an "
                                    "image view but VkCommandBufferInheritanceRenderingInfo::depthAttachmentFormat "
                                    "is %s instead of VK_FORMAT_UNDEFINED.",
                                    report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                    cb_state->begin_rendering_func_name.c_str(), string_VkFormat(format));
                            }
                        }

                        if (rendering_info.pStencilAttachment == nullptr ||
                            rendering_info.pStencilAttachment->imageView == VK_NULL_HANDLE) {
                            VkFormat format = inheritance_rendering_info.stencilAttachmentFormat;
                            if (format != VK_FORMAT_UNDEFINED) {
                                skip |= LogError(
                                    pCommandBuffers[i], "VUID-vkCmdExecuteCommands-pStencilAttachment-06775",
                                    "vkCmdExecuteCommands(): Secondary %s is executed within a dynamic renderpass "
                                    "instance scope begun by %s(), and VkRenderingInfo::pStencilAttachment does not define an "
                                    "image view but VkCommandBufferInheritanceRenderingInfo::stencilAttachmentFormat "
                                    "is %s instead of VK_FORMAT_UNDEFINED.",
                                    report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                    cb_state->begin_rendering_func_name.c_str(), string_VkFormat(format));
                            }
                        }

                        if (rendering_info.viewMask != inheritance_rendering_info.viewMask) {
                            skip |= LogError(
                                pCommandBuffers[i], "VUID-vkCmdExecuteCommands-viewMask-06031",
                                "vkCmdExecuteCommands(): Secondary %s is executed within a dynamic renderpass instance scope begun "
                                "by %s(), but "
                                "VkCommandBufferInheritanceRenderingInfo::viewMask (%u) does "
                                "not match VkRenderingInfo::viewMask (%u).",
                                report_data->FormatHandle(pCommandBuffers[i]).c_str(), cb_state->begin_rendering_func_name.c_str(),
                                inheritance_rendering_info.viewMask, rendering_info.viewMask);
                        }

                        // VkAttachmentSampleCountInfoAMD == VkAttachmentSampleCountInfoNV
                        const auto amd_sample_count =
                            LvlFindInChain<VkAttachmentSampleCountInfoAMD>(inheritance_rendering_info.pNext);

                        if (amd_sample_count) {
                            for (uint32_t index = 0; index < rendering_info.colorAttachmentCount; index++) {
                                if (rendering_info.pColorAttachments[index].imageView == VK_NULL_HANDLE) {
                                    continue;
                                }
                                auto image_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pColorAttachments[index].imageView);

                                if (image_view_state->samples != amd_sample_count->pColorAttachmentSamples[index]) {
                                    skip |= LogError(
                                        pCommandBuffers[i], "VUID-vkCmdExecuteCommands-pNext-06032",
                                        "vkCmdExecuteCommands(): Secondary %s is executed within a dynamic renderpass instance "
                                        "scope begun "
                                        "by vkCmdBeginRenderingKHR(), but "
                                        "VkAttachmentSampleCountInfo(AMD/NV)::pColorAttachmentSamples at index (%u) "
                                        "does "
                                        "not match the sample count of the imageView in VkRenderingInfoKHR::pColorAttachments.",
                                        report_data->FormatHandle(pCommandBuffers[i]).c_str(), index);
                                }
                            }

                            if ((rendering_info.pDepthAttachment != nullptr) &&
                                rendering_info.pDepthAttachment->imageView != VK_NULL_HANDLE) {
                                auto image_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pDepthAttachment->imageView);

                                if (image_view_state->samples != amd_sample_count->depthStencilAttachmentSamples) {
                                    skip |= LogError(
                                        pCommandBuffers[i], "VUID-vkCmdExecuteCommands-pNext-06033",
                                        "vkCmdExecuteCommands(): Secondary %s is executed within a dynamic renderpass instance "
                                        "scope begun "
                                        "by vkCmdBeginRenderingKHR(), but "
                                        "VkAttachmentSampleCountInfo(AMD/NV)::depthStencilAttachmentSamples does "
                                        "not match the sample count of the imageView in VkRenderingInfoKHR::pDepthAttachment.",
                                        report_data->FormatHandle(pCommandBuffers[i]).c_str());
                                }
                            }

                            if ((rendering_info.pStencilAttachment != nullptr) &&
                                rendering_info.pStencilAttachment->imageView != VK_NULL_HANDLE) {
                                auto image_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pStencilAttachment->imageView);

                                if (image_view_state->samples != amd_sample_count->depthStencilAttachmentSamples) {
                                    skip |= LogError(
                                        pCommandBuffers[i], "VUID-vkCmdExecuteCommands-pNext-06034",
                                        "vkCmdExecuteCommands(): Secondary %s is executed within a dynamic renderpass instance "
                                        "scope begun "
                                        "by vkCmdBeginRenderingKHR(), but "
                                        "VkAttachmentSampleCountInfo(AMD/NV)::depthStencilAttachmentSamples does "
                                        "not match the sample count of the imageView in VkRenderingInfoKHR::pStencilAttachment.",
                                        report_data->FormatHandle(pCommandBuffers[i]).c_str());
                                }
                            }
                        } else {
                            for (uint32_t index = 0; index < rendering_info.colorAttachmentCount; index++) {
                                if (rendering_info.pColorAttachments[index].imageView == VK_NULL_HANDLE) {
                                    continue;
                                }
                                auto image_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pColorAttachments[index].imageView);

                                if (image_view_state->samples != inheritance_rendering_info.rasterizationSamples) {
                                    skip |= LogError(
                                        pCommandBuffers[i], "VUID-vkCmdExecuteCommands-pNext-06035",
                                        "vkCmdExecuteCommands(): Secondary %s is executed within a dynamic renderpass instance "
                                        "scope begun "
                                        "by vkCmdBeginRenderingKHR(), but the sample count of the image view at index (%u) of "
                                        "VkRenderingInfoKHR::pColorAttachments does not match "
                                        "VkCommandBufferInheritanceRenderingInfoKHR::rasterizationSamples.",
                                        report_data->FormatHandle(pCommandBuffers[i]).c_str(), index);
                                }
                            }

                            if ((rendering_info.pDepthAttachment != nullptr) &&
                                rendering_info.pDepthAttachment->imageView != VK_NULL_HANDLE) {
                                auto image_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pDepthAttachment->imageView);

                                if (image_view_state->samples != inheritance_rendering_info.rasterizationSamples) {
                                    skip |= LogError(pCommandBuffers[i], "VUID-vkCmdExecuteCommands-pNext-06036",
                                                     "vkCmdExecuteCommands(): Secondary %s is executed within a dynamic renderpass "
                                                     "instance scope begun "
                                                     "by vkCmdBeginRenderingKHR(), but the sample count of the image view for "
                                                     "VkRenderingInfoKHR::pDepthAttachment does not match "
                                                     "VkCommandBufferInheritanceRenderingInfoKHR::rasterizationSamples.",
                                                     report_data->FormatHandle(pCommandBuffers[i]).c_str());
                                }
                            }

                            if ((rendering_info.pStencilAttachment != nullptr) &&
                                rendering_info.pStencilAttachment->imageView != VK_NULL_HANDLE) {
                                auto image_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pStencilAttachment->imageView);

                                if (image_view_state->samples != inheritance_rendering_info.rasterizationSamples) {
                                    skip |= LogError(pCommandBuffers[i], "VUID-vkCmdExecuteCommands-pNext-06037",
                                                     "vkCmdExecuteCommands(): Secondary %s is executed within a dynamic renderpass "
                                                     "instance scope begun "
                                                     "by vkCmdBeginRenderingKHR(), but the sample count of the image view for "
                                                     "VkRenderingInfoKHR::pStencilAttachment does not match "
                                                     "VkCommandBufferInheritanceRenderingInfoKHR::rasterizationSamples.",
                                                     report_data->FormatHandle(pCommandBuffers[i]).c_str());
                                }
                            }
                        }
                    }
                }
            }
        }

        // TODO(mlentine): Move more logic into this method
        skip |= ValidateSecondaryCommandBufferState(*cb_state, *sub_cb_state);
        skip |= ValidateCommandBufferState(*sub_cb_state, "vkCmdExecuteCommands()", 0,
                                           "VUID-vkCmdExecuteCommands-pCommandBuffers-00089");
        if (!(sub_cb_state->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT)) {
            if (sub_cb_state->InUse()) {
                skip |= LogError(
                    cb_state->commandBuffer(), "VUID-vkCmdExecuteCommands-pCommandBuffers-00091",
                    "vkCmdExecuteCommands(): Cannot execute pending %s without VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set.",
                    report_data->FormatHandle(sub_cb_state->commandBuffer()).c_str());
            }
            // We use an const_cast, because one cannot query a container keyed on a non-const pointer using a const pointer
            if (cb_state->linkedCommandBuffers.count(const_cast<CMD_BUFFER_STATE *>(sub_cb_state.get()))) {
                const LogObjectList objlist(cb_state->commandBuffer(), sub_cb_state->commandBuffer());
                skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-pCommandBuffers-00092",
                                 "vkCmdExecuteCommands(): Cannot execute %s without VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT "
                                 "set if previously executed in %s",
                                 report_data->FormatHandle(sub_cb_state->commandBuffer()).c_str(),
                                 report_data->FormatHandle(cb_state->commandBuffer()).c_str());
            }

            const auto insert_pair = linked_command_buffers.insert(sub_cb_state.get());
            if (!insert_pair.second) {
                skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdExecuteCommands-pCommandBuffers-00093",
                                 "vkCmdExecuteCommands(): Cannot duplicate %s in pCommandBuffers without "
                                 "VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set.",
                                 report_data->FormatHandle(cb_state->commandBuffer()).c_str());
            }

            if (cb_state->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT) {
                // Warn that non-simultaneous secondary cmd buffer renders primary non-simultaneous
                const LogObjectList objlist(pCommandBuffers[i], cb_state->commandBuffer());
                skip |= LogWarning(objlist, kVUID_Core_DrawState_InvalidCommandBufferSimultaneousUse,
                                   "vkCmdExecuteCommands(): Secondary %s does not have "
                                   "VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set and will cause primary "
                                   "%s to be treated as if it does not have "
                                   "VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set, even though it does.",
                                   report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                   report_data->FormatHandle(cb_state->commandBuffer()).c_str());
            }
        }
        if (!cb_state->activeQueries.empty() && !enabled_features.core.inheritedQueries) {
            skip |= LogError(pCommandBuffers[i], "VUID-vkCmdExecuteCommands-commandBuffer-00101",
                             "vkCmdExecuteCommands(): Secondary %s cannot be submitted with a query in flight and "
                             "inherited queries not supported on this device.",
                             report_data->FormatHandle(pCommandBuffers[i]).c_str());
        }
        // Validate initial layout uses vs. the primary cmd buffer state
        // Novel Valid usage: "UNASSIGNED-vkCmdExecuteCommands-commandBuffer-00001"
        // initial layout usage of secondary command buffers resources must match parent command buffer
        const auto const_cb_state = std::static_pointer_cast<const CMD_BUFFER_STATE>(cb_state);
        for (const auto &sub_layout_map_entry : sub_cb_state->image_layout_map) {
            const auto *image_state = sub_layout_map_entry.first;
            const auto image = image_state->image();

            const auto *cb_subres_map = const_cb_state->GetImageSubresourceLayoutMap(*image_state);
            // Const getter can be null in which case we have nothing to check against for this image...
            if (!cb_subres_map) continue;

            const auto &sub_layout_map = sub_layout_map_entry.second->GetLayoutMap();
            const auto &cb_layout_map = cb_subres_map->GetLayoutMap();
            for (sparse_container::parallel_iterator<const ImageSubresourceLayoutMap::LayoutMap> iter(sub_layout_map, cb_layout_map,
                                                                                                      0);
                 !iter->range.empty(); ++iter) {
                VkImageLayout cb_layout = kInvalidLayout, sub_layout = kInvalidLayout;
                const char *layout_type;

                if (!iter->pos_A->valid || !iter->pos_B->valid) continue;

                // pos_A denotes the sub CB map in the parallel iterator
                sub_layout = iter->pos_A->lower_bound->second.initial_layout;
                if (VK_IMAGE_LAYOUT_UNDEFINED == sub_layout) continue;  // secondary doesn't care about current or initial

                // pos_B denotes the main CB map in the parallel iterator
                const auto &cb_layout_state = iter->pos_B->lower_bound->second;
                if (cb_layout_state.current_layout != kInvalidLayout) {
                    layout_type = "current";
                    cb_layout = cb_layout_state.current_layout;
                } else if (cb_layout_state.initial_layout != kInvalidLayout) {
                    layout_type = "initial";
                    cb_layout = cb_layout_state.initial_layout;
                } else {
                    continue;
                }
                if (sub_layout != cb_layout) {
                    // We can report all the errors for the intersected range directly
                    for (auto index = iter->range.begin; index < iter->range.end; index++) {
                        const auto subresource = image_state->subresource_encoder.Decode(index);
                        skip |=
                            LogError(pCommandBuffers[i], "UNASSIGNED-vkCmdExecuteCommands-commandBuffer-00001",
                                     "%s: Executed secondary command buffer using %s (subresource: aspectMask 0x%X array layer %u, "
                                     "mip level %u) which expects layout %s--instead, image %s layout is %s.",
                                     "vkCmdExecuteCommands():", report_data->FormatHandle(image).c_str(), subresource.aspectMask,
                                     subresource.arrayLayer, subresource.mipLevel, string_VkImageLayout(sub_layout), layout_type,
                                     string_VkImageLayout(cb_layout));
                    }
                }
            }
        }

        // All commands buffers involved must be protected or unprotected
        if ((cb_state->unprotected == false) && (sub_cb_state->unprotected == true)) {
            const LogObjectList objlist(cb_state->commandBuffer(), sub_cb_state->commandBuffer());
            skip |= LogError(
                objlist, "VUID-vkCmdExecuteCommands-commandBuffer-01820",
                "vkCmdExecuteCommands(): command buffer %s is protected while secondary command buffer %s is a unprotected",
                report_data->FormatHandle(cb_state->commandBuffer()).c_str(),
                report_data->FormatHandle(sub_cb_state->commandBuffer()).c_str());
        } else if ((cb_state->unprotected == true) && (sub_cb_state->unprotected == false)) {
            const LogObjectList objlist(cb_state->commandBuffer(), sub_cb_state->commandBuffer());
            skip |= LogError(
                objlist, "VUID-vkCmdExecuteCommands-commandBuffer-01821",
                "vkCmdExecuteCommands(): command buffer %s is unprotected while secondary command buffer %s is a protected",
                report_data->FormatHandle(cb_state->commandBuffer()).c_str(),
                report_data->FormatHandle(sub_cb_state->commandBuffer()).c_str());
        }
        if (active_occlusion_query && sub_cb_state->inheritanceInfo.occlusionQueryEnable != VK_TRUE) {
            skip |= LogError(pCommandBuffers[i], "VUID-vkCmdExecuteCommands-commandBuffer-00102",
                             "vkCmdExecuteCommands(): command buffer %s has an active occlusion query, but secondary command "
                             "buffer %s was recorded with VkCommandBufferInheritanceInfo::occlusionQueryEnable set to VK_FALSE",
                             report_data->FormatHandle(cb_state->commandBuffer()).c_str(),
                             report_data->FormatHandle(sub_cb_state->commandBuffer()).c_str());
        }
    }

    if (cb_state->transform_feedback_active) {
        skip |= LogError(commandBuffer, "VUID-vkCmdExecuteCommands-None-02286",
                         "vkCmdExecuteCommands(): transform feedback is active.");
    }

    skip |= ValidateCmd(*cb_state, CMD_EXECUTECOMMANDS);
    return skip;
}

bool CoreChecks::PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo *pBindInfo,
                                                VkFence fence) const {
    auto queue_data = Get<QUEUE_STATE>(queue);
    auto fence_state = Get<FENCE_STATE>(fence);
    bool skip = ValidateFenceForSubmit(fence_state.get(), "VUID-vkQueueBindSparse-fence-01114",
                                       "VUID-vkQueueBindSparse-fence-01113", "VkQueueBindSparse()");
    if (skip) {
        return true;
    }

    const auto queue_flags = physical_device_state->queue_family_properties[queue_data->queueFamilyIndex].queueFlags;
    if (!(queue_flags & VK_QUEUE_SPARSE_BINDING_BIT)) {
        skip |= LogError(queue, "VUID-vkQueueBindSparse-queuetype",
                         "vkQueueBindSparse(): a non-memory-management capable queue -- VK_QUEUE_SPARSE_BINDING_BIT not set.");
    }

    SemaphoreSubmitState sem_submit_state(this, queue,
                                          physical_device_state->queue_family_properties[queue_data->queueFamilyIndex].queueFlags);
    for (uint32_t bind_idx = 0; bind_idx < bindInfoCount; ++bind_idx) {
        Location loc(Func::vkQueueBindSparse, Struct::VkBindSparseInfo);
        const VkBindSparseInfo &bind_info = pBindInfo[bind_idx];

        skip |= ValidateSemaphoresForSubmit(sem_submit_state, bind_info, loc);

        if (bind_info.pBufferBinds) {
            for (uint32_t buffer_idx = 0; buffer_idx < bind_info.bufferBindCount; ++buffer_idx) {
                const VkSparseBufferMemoryBindInfo &buffer_bind = bind_info.pBufferBinds[buffer_idx];
                if (buffer_bind.pBinds) {
                    auto buffer_state = Get<BUFFER_STATE>(buffer_bind.buffer);
                    for (uint32_t buffer_bind_idx = 0; buffer_bind_idx < buffer_bind.bindCount; ++buffer_bind_idx) {
                        const VkSparseMemoryBind &memory_bind = buffer_bind.pBinds[buffer_bind_idx];
                        std::stringstream parameter_name;
                        parameter_name << "pBindInfo[" << bind_idx << "].pBufferBinds[" << buffer_idx << " ].pBinds["
                                       << buffer_bind_idx << "]";
                        skip |= ValidateSparseMemoryBind(memory_bind, buffer_state->requirements.size, "vkQueueBindSparse()",
                                                         parameter_name.str().c_str());
                    }
                }
            }
        }

        if (bind_info.pImageOpaqueBinds) {
            for (uint32_t image_opaque_idx = 0; image_opaque_idx < bind_info.bufferBindCount; ++image_opaque_idx) {
                const VkSparseImageOpaqueMemoryBindInfo &image_opaque_bind = bind_info.pImageOpaqueBinds[image_opaque_idx];
                if (image_opaque_bind.pBinds) {
                    auto image_state = Get<IMAGE_STATE>(image_opaque_bind.image);
                    for (uint32_t image_opaque_bind_idx = 0; image_opaque_bind_idx < image_opaque_bind.bindCount;
                         ++image_opaque_bind_idx) {
                        const VkSparseMemoryBind &memory_bind = image_opaque_bind.pBinds[image_opaque_bind_idx];
                        std::stringstream parameter_name;
                        parameter_name << "pBindInfo[" << bind_idx << "].pImageOpaqueBinds[" << image_opaque_idx << " ].pBinds["
                                       << image_opaque_bind_idx << "]";
                        // Assuming that no multiplanar disjointed images are possible with sparse memory binding. Needs
                        // confirmation
                        skip |= ValidateSparseMemoryBind(memory_bind, image_state->requirements[0].size, "vkQueueBindSparse()",
                                                         parameter_name.str().c_str());
                    }
                }
            }
        }

        if (bind_info.pImageBinds) {
            for (uint32_t image_idx = 0; image_idx < bind_info.imageBindCount; ++image_idx) {
                const VkSparseImageMemoryBindInfo &image_bind = bind_info.pImageBinds[image_idx];
                auto image_state = Get<IMAGE_STATE>(image_bind.image);

                if (image_state && !(image_state->createInfo.flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT)) {
                    skip |= LogError(image_bind.image, "VUID-VkSparseImageMemoryBindInfo-image-02901",
                                     "vkQueueBindSparse(): pBindInfo[%u].pImageBinds[%u]: image must have been created with "
                                     "VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT set",
                                     bind_idx, image_idx);
                }

                if (image_bind.pBinds) {
                    for (uint32_t image_bind_idx = 0; image_bind_idx < image_bind.bindCount; ++image_bind_idx) {
                        const VkSparseImageMemoryBind &memory_bind = image_bind.pBinds[image_bind_idx];
                        skip |= ValidateSparseImageMemoryBind(image_state.get(), memory_bind, image_idx, image_bind_idx);
                    }
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer,
                                                       const VkDebugMarkerMarkerInfoEXT *pMarkerInfo) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    return ValidateCmd(*cb_state, CMD_DEBUGMARKERBEGINEXT);
}

bool CoreChecks::PreCallValidateCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    return ValidateCmd(*cb_state, CMD_DEBUGMARKERENDEXT);
}

bool CoreChecks::ValidateCmdDrawStrideWithStruct(VkCommandBuffer commandBuffer, const std::string &vuid, const uint32_t stride,
                                                 const char *struct_name, const uint32_t struct_size) const {
    bool skip = false;
    static const int condition_multiples = 0b0011;
    if ((stride & condition_multiples) || (stride < struct_size)) {
        skip |= LogError(commandBuffer, vuid, "stride %d is invalid or less than sizeof(%s) %d.", stride, struct_name, struct_size);
    }
    return skip;
}

bool CoreChecks::ValidateCmdDrawStrideWithBuffer(VkCommandBuffer commandBuffer, const std::string &vuid, const uint32_t stride,
                                                 const char *struct_name, const uint32_t struct_size, const uint32_t drawCount,
                                                 const VkDeviceSize offset, const BUFFER_STATE *buffer_state) const {
    bool skip = false;
    uint64_t validation_value = stride * (drawCount - 1) + offset + struct_size;
    if (validation_value > buffer_state->createInfo.size) {
        skip |= LogError(commandBuffer, vuid,
                         "stride[%d] * (drawCount[%d] - 1) + offset[%" PRIx64 "] + sizeof(%s)[%d] = %" PRIx64
                         " is greater than the size[%" PRIx64 "] of %s.",
                         stride, drawCount, offset, struct_name, struct_size, validation_value, buffer_state->createInfo.size,
                         report_data->FormatHandle(buffer_state->buffer()).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                   uint32_t bindingCount, const VkBuffer *pBuffers,
                                                                   const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes) const {
    bool skip = false;
    char const *const cmd_name = "CmdBindTransformFeedbackBuffersEXT";
    if (!enabled_features.transform_feedback_features.transformFeedback) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBindTransformFeedbackBuffersEXT-transformFeedback-02355",
                         "%s: transformFeedback feature is not enabled.", cmd_name);
    }

    {
        auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
        if (cb_state->transform_feedback_active) {
            skip |= LogError(commandBuffer, "VUID-vkCmdBindTransformFeedbackBuffersEXT-None-02365",
                             "%s: transform feedback is active.", cmd_name);
        }
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        auto buffer_state = Get<BUFFER_STATE>(pBuffers[i]);
        assert(buffer_state != nullptr);

        if (pOffsets[i] >= buffer_state->createInfo.size) {
            skip |= LogError(buffer_state->buffer(), "VUID-vkCmdBindTransformFeedbackBuffersEXT-pOffsets-02358",
                             "%s: pOffset[%" PRIu32 "](0x%" PRIxLEAST64
                             ") is greater than or equal to the size of pBuffers[%" PRIu32 "](0x%" PRIxLEAST64 ").",
                             cmd_name, i, pOffsets[i], i, buffer_state->createInfo.size);
        }

        if ((buffer_state->createInfo.usage & VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT) == 0) {
            skip |= LogError(buffer_state->buffer(), "VUID-vkCmdBindTransformFeedbackBuffersEXT-pBuffers-02360",
                             "%s: pBuffers[%" PRIu32
                             "] (%s)"
                             " was not created with the VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT flag.",
                             cmd_name, i, report_data->FormatHandle(pBuffers[i]).c_str());
        }

        // pSizes is optional and may be nullptr. Also might be VK_WHOLE_SIZE which VU don't apply
        if ((pSizes != nullptr) && (pSizes[i] != VK_WHOLE_SIZE)) {
            // only report one to prevent redundant error if the size is larger since adding offset will be as well
            if (pSizes[i] > buffer_state->createInfo.size) {
                skip |= LogError(buffer_state->buffer(), "VUID-vkCmdBindTransformFeedbackBuffersEXT-pSizes-02362",
                                 "%s: pSizes[%" PRIu32 "](0x%" PRIxLEAST64 ") is greater than the size of pBuffers[%" PRIu32
                                 "](0x%" PRIxLEAST64 ").",
                                 cmd_name, i, pSizes[i], i, buffer_state->createInfo.size);
            } else if (pOffsets[i] + pSizes[i] > buffer_state->createInfo.size) {
                skip |= LogError(buffer_state->buffer(), "VUID-vkCmdBindTransformFeedbackBuffersEXT-pOffsets-02363",
                                 "%s: The sum of pOffsets[%" PRIu32 "](Ox%" PRIxLEAST64 ") and pSizes[%" PRIu32 "](0x%" PRIxLEAST64
                                 ") is greater than the size of pBuffers[%" PRIu32 "](0x%" PRIxLEAST64 ").",
                                 cmd_name, i, pOffsets[i], i, pSizes[i], i, buffer_state->createInfo.size);
            }
        }

        skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *buffer_state, cmd_name,
                                              "VUID-vkCmdBindTransformFeedbackBuffersEXT-pBuffers-02364");
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                             uint32_t counterBufferCount, const VkBuffer *pCounterBuffers,
                                                             const VkDeviceSize *pCounterBufferOffsets) const {
    bool skip = false;
    char const *const cmd_name = "CmdBeginTransformFeedbackEXT";
    if (!enabled_features.transform_feedback_features.transformFeedback) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBeginTransformFeedbackEXT-transformFeedback-02366",
                         "%s: transformFeedback feature is not enabled.", cmd_name);
    }

    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);

    const auto *pipe = cb_state->lastBound[VK_PIPELINE_BIND_POINT_GRAPHICS].pipeline_state;
    if (!pipe) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBeginTransformFeedbackEXT-None-06233",
                         "%s: No graphics pipeline has been bound yet.", cmd_name);
    }

    if (cb_state) {
        if (cb_state->transform_feedback_active) {
            skip |= LogError(commandBuffer, "VUID-vkCmdBeginTransformFeedbackEXT-None-02367", "%s: transform feedback is active.",
                             cmd_name);
        }
        if (cb_state->activeRenderPass) {
            const auto &rp_ci = cb_state->activeRenderPass->createInfo;
            for (uint32_t i = 0; i < rp_ci.subpassCount; ++i) {
                // When a subpass uses a non-zero view mask, multiview functionality is considered to be enabled
                if (rp_ci.pSubpasses[i].viewMask > 0) {
                    skip |= LogError(commandBuffer, "VUID-vkCmdBeginTransformFeedbackEXT-None-02373",
                                     "%s: active render pass (%s) has multiview enabled.", cmd_name,
                                     report_data->FormatHandle(cb_state->activeRenderPass->renderPass()).c_str());
                    break;
                }
            }
        }
    }

    // pCounterBuffers and pCounterBufferOffsets are optional and may be nullptr. Additionaly, pCounterBufferOffsets must be nullptr
    // if pCounterBuffers is nullptr.
    if (pCounterBuffers == nullptr) {
        if (pCounterBufferOffsets != nullptr) {
            skip |= LogError(commandBuffer, "VUID-vkCmdBeginTransformFeedbackEXT-pCounterBuffer-02371",
                             "%s: pCounterBuffers is NULL and pCounterBufferOffsets is not NULL.", cmd_name);
        }
    } else {
        for (uint32_t i = 0; i < counterBufferCount; ++i) {
            if (pCounterBuffers[i] != VK_NULL_HANDLE) {
                auto buffer_state = Get<BUFFER_STATE>(pCounterBuffers[i]);
                assert(buffer_state != nullptr);

                if (pCounterBufferOffsets != nullptr && pCounterBufferOffsets[i] + 4 > buffer_state->createInfo.size) {
                    skip |=
                        LogError(buffer_state->buffer(), "VUID-vkCmdBeginTransformFeedbackEXT-pCounterBufferOffsets-02370",
                                 "%s: pCounterBuffers[%" PRIu32
                                 "](%s) is not large enough to hold 4 bytes at pCounterBufferOffsets[%" PRIu32 "](0x%" PRIx64 ").",
                                 cmd_name, i, report_data->FormatHandle(pCounterBuffers[i]).c_str(), i, pCounterBufferOffsets[i]);
                }

                if ((buffer_state->createInfo.usage & VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT) == 0) {
                    skip |=
                        LogError(buffer_state->buffer(), "VUID-vkCmdBeginTransformFeedbackEXT-pCounterBuffers-02372",
                                 "%s: pCounterBuffers[%" PRIu32
                                 "] (%s) was not created with the VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT flag.",
                                 cmd_name, i, report_data->FormatHandle(pCounterBuffers[i]).c_str());
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                           uint32_t counterBufferCount, const VkBuffer *pCounterBuffers,
                                                           const VkDeviceSize *pCounterBufferOffsets) const {
    bool skip = false;
    char const *const cmd_name = "CmdEndTransformFeedbackEXT";
    if (!enabled_features.transform_feedback_features.transformFeedback) {
        skip |= LogError(commandBuffer, "VUID-vkCmdEndTransformFeedbackEXT-transformFeedback-02374",
                         "%s: transformFeedback feature is not enabled.", cmd_name);
    }

    {
        auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
        if (!cb_state->transform_feedback_active) {
            skip |= LogError(commandBuffer, "VUID-vkCmdEndTransformFeedbackEXT-None-02375", "%s: transform feedback is not active.",
                             cmd_name);
        }
    }

    // pCounterBuffers and pCounterBufferOffsets are optional and may be nullptr. Additionaly, pCounterBufferOffsets must be nullptr
    // if pCounterBuffers is nullptr.
    if (pCounterBuffers == nullptr) {
        if (pCounterBufferOffsets != nullptr) {
            skip |= LogError(commandBuffer, "VUID-vkCmdEndTransformFeedbackEXT-pCounterBuffer-02379",
                             "%s: pCounterBuffers is NULL and pCounterBufferOffsets is not NULL.", cmd_name);
        }
    } else {
        for (uint32_t i = 0; i < counterBufferCount; ++i) {
            if (pCounterBuffers[i] != VK_NULL_HANDLE) {
                auto buffer_state = Get<BUFFER_STATE>(pCounterBuffers[i]);
                assert(buffer_state != nullptr);

                if (pCounterBufferOffsets != nullptr && pCounterBufferOffsets[i] + 4 > buffer_state->createInfo.size) {
                    skip |=
                        LogError(buffer_state->buffer(), "VUID-vkCmdEndTransformFeedbackEXT-pCounterBufferOffsets-02378",
                                 "%s: pCounterBuffers[%" PRIu32
                                 "](%s) is not large enough to hold 4 bytes at pCounterBufferOffsets[%" PRIu32 "](0x%" PRIx64 ").",
                                 cmd_name, i, report_data->FormatHandle(pCounterBuffers[i]).c_str(), i, pCounterBufferOffsets[i]);
                }

                if ((buffer_state->createInfo.usage & VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT) == 0) {
                    skip |=
                        LogError(buffer_state->buffer(), "VUID-vkCmdEndTransformFeedbackEXT-pCounterBuffers-02380",
                                 "%s: pCounterBuffers[%" PRIu32
                                 "] (%s) was not created with the VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT flag.",
                                 cmd_name, i, report_data->FormatHandle(pCounterBuffers[i]).c_str());
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                               const VkBuffer *pBuffers, const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                               const VkDeviceSize *pStrides, CMD_TYPE cmd_type) const {
    const char *api_call = CommandTypeString(cmd_type);
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);

    bool skip = false;
    skip |= ValidateCmd(*cb_state, cmd_type);
    for (uint32_t i = 0; i < bindingCount; ++i) {
        auto buffer_state = Get<BUFFER_STATE>(pBuffers[i]);
        if (buffer_state) {
            skip |= ValidateBufferUsageFlags(commandBuffer, *buffer_state, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, true,
                                             "VUID-vkCmdBindVertexBuffers2-pBuffers-03359", api_call,
                                             "VK_BUFFER_USAGE_VERTEX_BUFFER_BIT");
            skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *buffer_state, api_call,
                                                  "VUID-vkCmdBindVertexBuffers2-pBuffers-03360");

            if (pOffsets[i] >= buffer_state->createInfo.size) {
                skip |= LogError(buffer_state->buffer(), "VUID-vkCmdBindVertexBuffers2-pOffsets-03357",
                                 "%s offset (0x%" PRIxLEAST64 ") is beyond the end of the buffer.", api_call, pOffsets[i]);
            }
            if (pSizes && pOffsets[i] + pSizes[i] > buffer_state->createInfo.size) {
                skip |= LogError(buffer_state->buffer(), "VUID-vkCmdBindVertexBuffers2-pSizes-03358",
                                 "%s size (0x%" PRIxLEAST64 ") is beyond the end of the buffer.", api_call, pSizes[i]);
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                         uint32_t bindingCount, const VkBuffer *pBuffers,
                                                         const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                         const VkDeviceSize *pStrides) const {
    bool skip = ValidateCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides,
                                              CMD_BINDVERTEXBUFFERS2EXT);
    return skip;
}

bool CoreChecks::PreCallValidateCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount,
                                                      const VkBuffer *pBuffers, const VkDeviceSize *pOffsets,
                                                      const VkDeviceSize *pSizes, const VkDeviceSize *pStrides) const {
    bool skip = ValidateCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides,
                                              CMD_BINDVERTEXBUFFERS2);
    return skip;
}

bool CoreChecks::PreCallValidateCmdBeginConditionalRenderingEXT(
    VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT *pConditionalRenderingBegin) const {
    bool skip = false;

    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_state && cb_state->conditional_rendering_active) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBeginConditionalRenderingEXT-None-01980",
                         "vkCmdBeginConditionalRenderingEXT(): Conditional rendering is already active.");
    }

    if (pConditionalRenderingBegin) {
        auto buffer_state = Get<BUFFER_STATE>(pConditionalRenderingBegin->buffer);
        if (buffer_state) {
            if ((buffer_state->createInfo.usage & VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT) == 0) {
                skip |= LogError(commandBuffer, "VUID-VkConditionalRenderingBeginInfoEXT-buffer-01982",
                                 "vkCmdBeginConditionalRenderingEXT(): pConditionalRenderingBegin->buffer (%s) was not create with "
                                 "VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT bit.",
                                 report_data->FormatHandle(pConditionalRenderingBegin->buffer).c_str());
            }
            if (pConditionalRenderingBegin->offset + 4 > buffer_state->createInfo.size) {
                skip |= LogError(commandBuffer, "VUID-VkConditionalRenderingBeginInfoEXT-offset-01983",
                                 "vkCmdBeginConditionalRenderingEXT(): pConditionalRenderingBegin->offset (%" PRIu64
                                 ") + 4 bytes is not less than the size of pConditionalRenderingBegin->buffer (%" PRIu64 ").",
                                 pConditionalRenderingBegin->offset, buffer_state->createInfo.size);
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) const {
    bool skip = false;

    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_state) {
        if (!cb_state->conditional_rendering_active) {
            skip |= LogError(commandBuffer, "VUID-vkCmdEndConditionalRenderingEXT-None-01985",
                             "vkCmdBeginConditionalRenderingEXT(): Conditional rendering is not active.");
        }
        if (!cb_state->conditional_rendering_inside_render_pass && cb_state->activeRenderPass != nullptr) {
            skip |= LogError(commandBuffer, "VUID-vkCmdEndConditionalRenderingEXT-None-01986",
                             "vkCmdBeginConditionalRenderingEXT(): Conditional rendering was begun outside outside of a render "
                             "pass instance, but a render pass instance is currently active in the command buffer.");
        }
        if (cb_state->conditional_rendering_inside_render_pass && cb_state->activeRenderPass != nullptr &&
            cb_state->conditional_rendering_subpass != cb_state->activeSubpass) {
            skip |= LogError(commandBuffer, "VUID-vkCmdEndConditionalRenderingEXT-None-01987",
                             "vkCmdBeginConditionalRenderingEXT(): Conditional rendering was begun in subpass %" PRIu32
                             ", but the current subpass is %" PRIu32 ".",
                             cb_state->conditional_rendering_subpass, cb_state->activeSubpass);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetImageSubresourceLayout2EXT(VkDevice device, VkImage image,
                                                              const VkImageSubresource2EXT *pSubresource,
                                                              VkSubresourceLayout2EXT *pLayout) const

{
    bool skip = false;
    const auto imageState = Get<IMAGE_STATE>(image);

    if (imageState) {
        const VkImageAspectFlags aspectMask = pSubresource->imageSubresource.aspectMask;
        const VkFormat imageFormat = imageState->createInfo.format;
        const uint32_t imageMipLevels = imageState->createInfo.mipLevels;
        const uint32_t imageArrayLayers = imageState->createInfo.arrayLayers;

        if (aspectMask == 0 || (aspectMask & (aspectMask - 1))) {  // 0 or Multiple bit set
            skip |= LogError(image, "VUID-vkGetImageSubresourceLayout2EXT-aspectMask-00997",
                             "vkGetImageSubresourceLayout2EXT: aspect mask should set a bit but "
                             "pSubresource->imageSubresource.aspectMask is 0x%x",
                             pSubresource->imageSubresource.aspectMask);
        }

        if (pSubresource->imageSubresource.mipLevel >= imageMipLevels) {
            skip |= LogError(image, "VUID-vkGetImageSubresourceLayout2EXT-mipLevel-01716",
                             "vkGetImageSubresourceLayout2EXT: subresource mipLevel should be less then image mipLevels but image "
                             "mipLevels %" PRIu32 " but subresource miplevel is %" PRIu32,
                             imageMipLevels, pSubresource->imageSubresource.mipLevel);
        }

        if (pSubresource->imageSubresource.arrayLayer >= imageArrayLayers) {
            skip |= LogError(image, "VUID-vkGetImageSubresourceLayout2EXT-arrayLayer-01717",
                             "vkGetImageSubresourceLayout2EXT: subresource array layer should be less then image array layers but "
                             "image array layers are %" PRIu32 " but subresource array layer is %" PRIu32,
                             imageArrayLayers, pSubresource->imageSubresource.arrayLayer);
        }

        if (FormatIsColor(imageFormat)) {  // single plane color format
            if (aspectMask != VK_IMAGE_ASPECT_COLOR_BIT) {
                skip |=
                    LogError(image, "VUID-vkGetImageSubresourceLayout2EXT-format-04461",
                             "vkGetImageSubresourceLayout2EXT: format of image is %s which is a color format but aspectMask is %s",
                             string_VkFormat(imageFormat), string_VkImageAspectFlags(aspectMask).c_str());
            }
        }

        if (FormatHasDepth(imageFormat)) {
            if ((aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) == 0) {
                skip |= LogError(image, "VUID-vkGetImageSubresourceLayout2EXT-format-04462",
                                 "vkGetImageSubresourceLayout2EXT: format of image is %s which has depth component "
                                 "but aspectMask is %s",
                                 string_VkFormat(imageFormat), string_VkImageAspectFlags(aspectMask).c_str());
            }
        }

        if (FormatHasStencil(imageFormat)) {
            if ((aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) == 0) {
                skip |= LogError(image, "VUID-vkGetImageSubresourceLayout2EXT-format-04463",
                                 "vkGetImageSubresourceLayout2EXT: format of image is %s which which has stencil "
                                 "component but aspectMask is %s",
                                 string_VkFormat(imageFormat), string_VkImageAspectFlags(aspectMask).c_str());
            }
        }

        if (!FormatHasDepth(imageFormat) && !FormatHasStencil(imageFormat)) {
            if ((aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != 0) {
                skip |= LogError(image, "VUID-vkGetImageSubresourceLayout2EXT-format-04464",
                                 "vkGetImageSubresourceLayout2EXT: format of image is %s which which does not have depth or stencil"
                                 "component but aspectMask is %s",
                                 string_VkFormat(imageFormat), string_VkImageAspectFlags(aspectMask).c_str());
            }
        }

        if (FormatPlaneCount(imageFormat) == 2) {
            if ((aspectMask != VK_IMAGE_ASPECT_PLANE_0_BIT) && (aspectMask != VK_IMAGE_ASPECT_PLANE_1_BIT)) {
                skip |= LogError(image, "VUID-vkGetImageSubresourceLayout2EXT-format-01581",
                                 "vkGetImageSubresourceLayout2EXT: plane count of image format(%s) is 2 but aspectMask is %s",
                                 string_VkFormat(imageFormat), string_VkImageAspectFlags(aspectMask).c_str());
            }
        }

        if (FormatPlaneCount(imageFormat) == 3) {
            if ((aspectMask != VK_IMAGE_ASPECT_PLANE_0_BIT) && (aspectMask != VK_IMAGE_ASPECT_PLANE_1_BIT) &&
                (aspectMask != VK_IMAGE_ASPECT_PLANE_2_BIT))
                skip |= LogError(image, "VUID-vkGetImageSubresourceLayout2EXT-format-01582",
                                 "vkGetImageSubresourceLayout2EXT: plane count of image format(%s) is 3 but aspectMask is %s",
                                 string_VkFormat(imageFormat), string_VkImageAspectFlags(aspectMask).c_str());
        }

        if ((imageState->IsExternalAHB()) && (0 == imageState->GetBoundMemoryStates().size())) {
            skip |=
                LogError(image, "VUID-vkGetImageSubresourceLayout2EXT-image-01895",
                         "vkGetImageSubresourceLayout2EXT: image type is android hardware buffer but bound memory is not valid");
        }
    }

    return skip;
}
