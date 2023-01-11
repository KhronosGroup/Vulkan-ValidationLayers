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
#include <valarray>

#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)

#include <unistd.h>
#include <sys/types.h>
#endif

#include "vk_enum_string_helper.h"
#include "chassis.h"
#include "core_validation.h"
#include "shader_validation.h"
#include "vk_layer_utils.h"
#include "sync_utils.h"
#include "sync_vuid_maps.h"
#include "stateless_validation.h"
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

bool CoreChecks::ValidateDeviceQueueFamily(uint32_t queue_family, const char *cmd_name, const char *parameter_name,
                                           const char *error_code, bool optional = false) const {
    bool skip = false;
    if (!optional && queue_family == VK_QUEUE_FAMILY_IGNORED) {
        skip |= LogError(device, error_code,
                         "%s: %s is VK_QUEUE_FAMILY_IGNORED, but it is required to provide a valid queue family index value.",
                         cmd_name, parameter_name);
    } else if (queue_family_index_set.find(queue_family) == queue_family_index_set.end()) {
        skip |=
            LogError(device, error_code,
                     "%s: %s (= %" PRIu32
                     ") is not one of the queue families given via VkDeviceQueueCreateInfo structures when the device was created.",
                     cmd_name, parameter_name, queue_family);
    }

    return skip;
}

// Validate the specified queue families against the families supported by the physical device that owns this device
bool CoreChecks::ValidatePhysicalDeviceQueueFamilies(uint32_t queue_family_count, const uint32_t *queue_families,
                                                     const char *cmd_name, const char *array_parameter_name,
                                                     const char *vuid) const {
    bool skip = false;
    if (queue_families) {
        layer_data::unordered_set<uint32_t> set;
        for (uint32_t i = 0; i < queue_family_count; ++i) {
            std::string parameter_name = std::string(array_parameter_name) + "[" + std::to_string(i) + "]";

            if (set.count(queue_families[i])) {
                skip |= LogError(device, vuid, "%s: %s (=%" PRIu32 ") is not unique within %s array.", cmd_name,
                                 parameter_name.c_str(), queue_families[i], array_parameter_name);
            } else {
                set.insert(queue_families[i]);
                if (queue_families[i] == VK_QUEUE_FAMILY_IGNORED) {
                    skip |= LogError(
                        device, vuid,
                        "%s: %s is VK_QUEUE_FAMILY_IGNORED, but it is required to provide a valid queue family index value.",
                        cmd_name, parameter_name.c_str());
                } else if (queue_families[i] >= physical_device_state->queue_family_known_count) {
                    const LogObjectList objlist(physical_device, device);
                    skip |=
                        LogError(objlist, vuid,
                                 "%s: %s (= %" PRIu32
                                 ") is not one of the queue families supported by the parent PhysicalDevice %s of this device %s.",
                                 cmd_name, parameter_name.c_str(), queue_families[i],
                                 report_data->FormatHandle(physical_device).c_str(), report_data->FormatHandle(device).c_str());
                }
            }
        }
    }
    return skip;
}

// Check object status for selected flag state
bool CoreChecks::ValidateCBDynamicStatus(const CMD_BUFFER_STATE &cb_state, CBDynamicStatus status, CMD_TYPE cmd_type,
                                         const char *msg_code) const {
    if (!(cb_state.status[status])) {
        return LogError(cb_state.commandBuffer(), msg_code, "%s: %s state not set for this command buffer.",
                        CommandTypeString(cmd_type), DynamicStateString(status).c_str());
    }
    return false;
}

static void ListBits(std::ostream &s, uint32_t bits) {
    for (int i = 0; i < 32 && bits; i++) {
        if (bits & (1 << i)) {
            s << i;
            bits &= ~(1 << i);
            if (bits) {
                s << ",";
            }
        }
    }
}

bool CoreChecks::ValidateDrawDynamicState(const CMD_BUFFER_STATE &cb_state, const PIPELINE_STATE &pipeline,
                                          CMD_TYPE cmd_type) const {
    bool skip = false;
    const DrawDispatchVuid vuid = GetDrawDispatchVuid(cmd_type);

    // Check all state with no additional requirements
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_PATCH_CONTROL_POINTS_EXT_SET, cmd_type, vuid.patch_control_points);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_RASTERIZER_DISCARD_ENABLE_SET, cmd_type, vuid.rasterizer_discard_enable);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_DEPTH_BIAS_ENABLE_SET, cmd_type, vuid.depth_bias_enable);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_LOGIC_OP_EXT_SET, cmd_type, vuid.logic_op);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_PRIMITIVE_RESTART_ENABLE_SET, cmd_type, vuid.primitive_restart_enable);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_VERTEX_INPUT_BINDING_STRIDE_SET, cmd_type, vuid.vertex_input_binding_stride);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_VERTEX_INPUT_EXT_SET, cmd_type, vuid.vertex_input);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_COLOR_WRITE_ENABLE_EXT_SET, cmd_type, vuid.dynamic_color_write_enable);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_TESSELLATION_DOMAIN_ORIGIN_EXT_SET, cmd_type,
                                    vuid.dynamic_tessellation_domain_origin);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_DEPTH_CLAMP_ENABLE_EXT_SET, cmd_type, vuid.dynamic_depth_clamp_enable);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_POLYGON_MODE_EXT_SET, cmd_type, vuid.dynamic_polygon_mode);
    skip |=
        ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_RASTERIZATION_SAMPLES_EXT_SET, cmd_type, vuid.dynamic_rasterization_samples);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_SAMPLE_MASK_EXT_SET, cmd_type, vuid.dynamic_sample_mask);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_ALPHA_TO_COVERAGE_ENABLE_EXT_SET, cmd_type,
                                    vuid.dynamic_alpha_to_coverage_enable);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_ALPHA_TO_ONE_ENABLE_EXT_SET, cmd_type, vuid.dynamic_alpha_to_one_enable);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_LOGIC_OP_ENABLE_EXT_SET, cmd_type, vuid.dynamic_logic_op_enable);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_COLOR_BLEND_ENABLE_EXT_SET, cmd_type, vuid.dynamic_color_blend_enable);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_COLOR_BLEND_EQUATION_EXT_SET, cmd_type, vuid.dynamic_color_blend_equation);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_COLOR_WRITE_MASK_EXT_SET, cmd_type, vuid.dynamic_color_write_mask);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_RASTERIZATION_STREAM_EXT_SET, cmd_type, vuid.dynamic_rasterization_stream);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_CONSERVATIVE_RASTERIZATION_MODE_EXT_SET, cmd_type,
                                    vuid.dynamic_conservative_rasterization_mode);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT_SET, cmd_type,
                                    vuid.dynamic_extra_primitive_overestimation_size);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_DEPTH_CLIP_ENABLE_EXT_SET, cmd_type, vuid.dynamic_depth_clip_enable);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_SAMPLE_LOCATIONS_ENABLE_EXT_SET, cmd_type,
                                    vuid.dynamic_sample_locations_enable);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_COLOR_BLEND_ADVANCED_EXT_SET, cmd_type, vuid.dynamic_color_blend_advanced);
    skip |=
        ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_PROVOKING_VERTEX_MODE_EXT_SET, cmd_type, vuid.dynamic_provoking_vertex_mode);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_LINE_RASTERIZATION_MODE_EXT_SET, cmd_type,
                                    vuid.dynamic_line_rasterization_mode);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_LINE_STIPPLE_ENABLE_EXT_SET, cmd_type, vuid.dynamic_line_stipple_enable);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT_SET, cmd_type,
                                    vuid.dynamic_depth_clip_negative_one_to_one);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_VIEWPORT_W_SCALING_ENABLE_NV_SET, cmd_type,
                                    vuid.dynamic_viewport_w_scaling_enable);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_VIEWPORT_SWIZZLE_NV_SET, cmd_type, vuid.dynamic_viewport_swizzle);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_COVERAGE_TO_COLOR_ENABLE_NV_SET, cmd_type,
                                    vuid.dynamic_coverage_to_color_enable);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_COVERAGE_TO_COLOR_LOCATION_NV_SET, cmd_type,
                                    vuid.dynamic_coverage_to_color_location);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_COVERAGE_MODULATION_MODE_NV_SET, cmd_type,
                                    vuid.dynamic_coverage_modulation_mode);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_COVERAGE_MODULATION_TABLE_ENABLE_NV_SET, cmd_type,
                                    vuid.dynamic_coverage_modulation_table_enable);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_COVERAGE_MODULATION_TABLE_NV_SET, cmd_type,
                                    vuid.dynamic_coverage_modulation_table);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_SHADING_RATE_IMAGE_ENABLE_NV_SET, cmd_type,
                                    vuid.dynamic_shading_rate_image_enable);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV_SET, cmd_type,
                                    vuid.dynamic_representative_fragment_test_enable);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_COVERAGE_REDUCTION_MODE_NV_SET, cmd_type,
                                    vuid.dynamic_coverage_reduction_mode);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_SAMPLE_LOCATIONS_EXT_SET, cmd_type, vuid.dynamic_sample_locations);
    skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_PRIMITIVE_TOPOLOGY_SET, cmd_type, vuid.primitive_topology);

    const auto rp_state = pipeline.RasterizationState();
    if (rp_state && (rp_state->depthBiasEnable == VK_TRUE)) {
        skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_DEPTH_BIAS_SET, cmd_type, vuid.dynamic_state);
    }

    // Any line topology
    if (pipeline.topology_at_rasterizer == VK_PRIMITIVE_TOPOLOGY_LINE_LIST ||
        pipeline.topology_at_rasterizer == VK_PRIMITIVE_TOPOLOGY_LINE_STRIP ||
        pipeline.topology_at_rasterizer == VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY ||
        pipeline.topology_at_rasterizer == VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY) {
        skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_LINE_WIDTH_SET, cmd_type, vuid.dynamic_state);
        const auto *line_state = LvlFindInChain<VkPipelineRasterizationLineStateCreateInfoEXT>(rp_state);
        if (line_state && line_state->stippledLineEnable) {
            skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_LINE_STIPPLE_EXT_SET, cmd_type, vuid.dynamic_state);
        }
    }

    if (pipeline.BlendConstantsEnabled()) {
        skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_BLEND_CONSTANTS_SET, cmd_type, vuid.dynamic_state);
    }

    const auto ds_state = pipeline.DepthStencilState();
    if (ds_state) {
        if (ds_state->depthBoundsTestEnable == VK_TRUE) {
            skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_DEPTH_BOUNDS_SET, cmd_type, vuid.dynamic_state);
        }
        if (ds_state->stencilTestEnable == VK_TRUE) {
            skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_STENCIL_COMPARE_MASK_SET, cmd_type, vuid.dynamic_state);
            skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_STENCIL_WRITE_MASK_SET, cmd_type, vuid.dynamic_state);
            skip |= ValidateCBDynamicStatus(cb_state, CB_DYNAMIC_STENCIL_REFERENCE_SET, cmd_type, vuid.dynamic_state);
        }
    }

    // vkCmdSetDiscardRectangleEXT needs to be set on each rectangle
    const auto *discard_rectangle_state = LvlFindInChain<VkPipelineDiscardRectangleStateCreateInfoEXT>(pipeline.PNext());
    if (discard_rectangle_state && pipeline.IsDynamic(VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT)) {
        for (uint32_t i = 0; i < discard_rectangle_state->discardRectangleCount; i++) {
            if (!cb_state.dynamic_state_value.discard_rectangles.test(i)) {
                const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline());
                skip |= LogError(objlist, vuid.dynamic_discard_rectangle,
                                 "%s: vkCmdSetDiscardRectangleEXT was not set for discard rectangle index %" PRIu32
                                 " for this command buffer.",
                                 CommandTypeString(cmd_type), i);
                break;
            }
        }
    }

    // Verify if using dynamic state setting commands that it doesn't set up in pipeline
    CBDynamicFlags invalid_status(~CBDynamicFlags(0));
    invalid_status &= ~cb_state.dynamic_status;
    invalid_status &= ~cb_state.static_status;

    if (invalid_status.any()) {
        const LogObjectList objlist(cb_state.commandBuffer(), pipeline.pipeline());
        skip |= LogError(objlist, vuid.dynamic_state_setting_commands,
                         "%s: %s doesn't set up %s, but it calls the related dynamic state setting commands",
                         CommandTypeString(cmd_type), report_data->FormatHandle(pipeline.pipeline()).c_str(),
                         DynamicStateString(invalid_status).c_str());
    }

    // If Viewport or scissors are dynamic, verify that dynamic count matches PSO count.
    // Skip check if rasterization is disabled, if there is no viewport, or if viewport/scissors are being inherited.
    const bool dyn_viewport = pipeline.IsDynamic(VK_DYNAMIC_STATE_VIEWPORT);
    const auto *viewport_state = pipeline.ViewportState();
    if ((!rp_state || (rp_state->rasterizerDiscardEnable == VK_FALSE)) && viewport_state &&
        (cb_state.inheritedViewportDepths.size() == 0)) {
        const bool dyn_scissor = pipeline.IsDynamic(VK_DYNAMIC_STATE_SCISSOR);

        // NB (akeley98): Current validation layers do not detect the error where vkCmdSetViewport (or scissor) was called, but
        // the dynamic state set is overwritten by binding a graphics pipeline with static viewport (scissor) state.
        // This condition be detected by checking trashedViewportMask & viewportMask (trashedScissorMask & scissorMask) is
        // nonzero in the range of bits needed by the pipeline.
        if (dyn_viewport) {
            const auto required_viewports_mask = (1 << viewport_state->viewportCount) - 1;
            const auto missing_viewport_mask = ~cb_state.viewportMask & required_viewports_mask;
            if (missing_viewport_mask) {
                std::stringstream ss;
                ss << CommandTypeString(cmd_type) << ": Dynamic viewport(s) ";
                ListBits(ss, missing_viewport_mask);
                ss << " are used by pipeline state object, but were not provided via calls to vkCmdSetViewport().";
                skip |= LogError(device, vuid.dynamic_state, "%s", ss.str().c_str());
            }
        }

        if (dyn_scissor) {
            const auto required_scissor_mask = (1 << viewport_state->scissorCount) - 1;
            const auto missing_scissor_mask = ~cb_state.scissorMask & required_scissor_mask;
            if (missing_scissor_mask) {
                std::stringstream ss;
                ss << CommandTypeString(cmd_type) << ": Dynamic scissor(s) ";
                ListBits(ss, missing_scissor_mask);
                ss << " are used by pipeline state object, but were not provided via calls to vkCmdSetScissor().";
                skip |= LogError(device, vuid.dynamic_state, "%s", ss.str().c_str());
            }
        }

        const bool dyn_viewport_count = pipeline.IsDynamic(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT);
        const bool dyn_scissor_count = pipeline.IsDynamic(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT);

        if (dyn_viewport_count && !dyn_scissor_count) {
            const auto required_viewport_mask = (1 << viewport_state->scissorCount) - 1;
            const auto missing_viewport_mask = ~cb_state.viewportWithCountMask & required_viewport_mask;
            if (missing_viewport_mask) {
                std::stringstream ss;
                ss << CommandTypeString(cmd_type) << ": Dynamic viewport with count ";
                ListBits(ss, missing_viewport_mask);
                ss << " are used by pipeline state object, but were not provided via calls to vkCmdSetViewportWithCountEXT().";
                skip |= LogError(device, vuid.viewport_count, "%s", ss.str().c_str());
            }
        }

        if (dyn_scissor_count && !dyn_viewport_count) {
            const auto required_scissor_mask = (1 << viewport_state->viewportCount) - 1;
            const auto missing_scissor_mask = ~cb_state.scissorWithCountMask & required_scissor_mask;
            if (missing_scissor_mask) {
                std::stringstream ss;
                ss << CommandTypeString(cmd_type) << ": Dynamic scissor with count ";
                ListBits(ss, missing_scissor_mask);
                ss << " are used by pipeline state object, but were not provided via calls to vkCmdSetScissorWithCountEXT().";
                skip |= LogError(device, vuid.scissor_count, "%s", ss.str().c_str());
            }
        }

        if (dyn_scissor_count && dyn_viewport_count) {
            if (cb_state.viewportWithCountMask != cb_state.scissorWithCountMask) {
                std::stringstream ss;
                ss << CommandTypeString(cmd_type) << ": Dynamic viewport and scissor with count ";
                ListBits(ss, cb_state.viewportWithCountMask ^ cb_state.scissorWithCountMask);
                ss << " are used by pipeline state object, but were not provided via matching calls to "
                      "vkCmdSetViewportWithCountEXT and vkCmdSetScissorWithCountEXT().";
                skip |= LogError(device, vuid.viewport_scissor_count, "%s", ss.str().c_str());
            }
        }
    }

    // If inheriting viewports, verify that not using more than inherited.
    if (cb_state.inheritedViewportDepths.size() != 0 && dyn_viewport) {
        const uint32_t viewport_count = viewport_state->viewportCount;
        const uint32_t max_inherited = uint32_t(cb_state.inheritedViewportDepths.size());
        if (viewport_count > max_inherited) {
            skip |= LogError(device, vuid.dynamic_state,
                             "Pipeline requires more viewports (%u) than inherited (viewportDepthCount=%u).",
                             unsigned(viewport_count), unsigned(max_inherited));
        }
    }

    if (pipeline.IsDynamic(VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT) && cb_state.status[CB_DYNAMIC_COLOR_WRITE_ENABLE_EXT_SET]) {
        const auto color_blend_state = cb_state.GetCurrentPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS)->ColorBlendState();
        if (color_blend_state) {
            uint32_t blend_attachment_count = color_blend_state->attachmentCount;
            if (cb_state.dynamicColorWriteEnableAttachmentCount < blend_attachment_count) {
                skip |= LogError(
                    cb_state.commandBuffer(), vuid.dynamic_color_write_enable_count,
                    "%s(): Currently bound pipeline was created with VkPipelineColorBlendStateCreateInfo::attachmentCount %" PRIu32
                    " and VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT, but the number of attachments written by "
                    "vkCmdSetColorWriteEnableEXT() is %" PRIu32 ".",
                    CommandTypeString(cmd_type), blend_attachment_count, cb_state.dynamicColorWriteEnableAttachmentCount);
            }
        }
    }

    if (pipeline.IsDynamic(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT) &&
        !phys_dev_ext_props.extended_dynamic_state3_props.dynamicPrimitiveTopologyUnrestricted) {
        bool compatible_topology = false;
        const auto input_assembly_state = pipeline.InputAssemblyState();
        switch (input_assembly_state->topology) {
            case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
                switch (cb_state.primitiveTopology) {
                    case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
                        compatible_topology = true;
                        break;
                    default:
                        break;
                }
                break;
            case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
            case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
            case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
            case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
                switch (cb_state.primitiveTopology) {
                    case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
                    case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
                    case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
                    case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
                        compatible_topology = true;
                        break;
                    default:
                        break;
                }
                break;
            case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
            case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
            case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
            case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
            case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
                switch (cb_state.primitiveTopology) {
                    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
                    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
                    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
                    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
                    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
                        compatible_topology = true;
                        break;
                    default:
                        break;
                }
                break;
            case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST:
                switch (cb_state.primitiveTopology) {
                    case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST:
                        compatible_topology = true;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
        if (!compatible_topology) {
            skip |= LogError(pipeline.pipeline(), vuid.primitive_topology,
                             "%s: the last primitive topology %s state set by vkCmdSetPrimitiveTopologyEXT is "
                             "not compatible with the pipeline topology %s.",
                             CommandTypeString(cmd_type), string_VkPrimitiveTopology(cb_state.primitiveTopology),
                             string_VkPrimitiveTopology(input_assembly_state->topology));
        }
    }

    return skip;
}

// For given pipeline, return number of MSAA samples, or one if MSAA disabled
static VkSampleCountFlagBits GetNumSamples(PIPELINE_STATE const &pipeline) {
    const auto ms_state = pipeline.MultisampleState();
    if (ms_state) {
        return ms_state->rasterizationSamples;
    }
    return VK_SAMPLE_COUNT_1_BIT;
}

bool CoreChecks::GetPhysicalDeviceImageFormatProperties(IMAGE_STATE &image_state, const char *vuid_string) const {
    bool skip = false;
    const auto image_create_info = image_state.createInfo;
    VkResult image_properties_result = VK_SUCCESS;
    if (image_create_info.tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        image_properties_result = DispatchGetPhysicalDeviceImageFormatProperties(
            physical_device, image_create_info.format, image_create_info.imageType, image_create_info.tiling,
            image_create_info.usage, image_create_info.flags, &image_state.image_format_properties);
    } else {
        auto image_format_info = LvlInitStruct<VkPhysicalDeviceImageFormatInfo2>();
        image_format_info.type = image_create_info.imageType;
        image_format_info.format = image_create_info.format;
        image_format_info.tiling = image_create_info.tiling;
        image_format_info.usage = image_create_info.usage;
        image_format_info.flags = image_create_info.flags;
        auto image_format_properties = LvlInitStruct<VkImageFormatProperties2>();
        image_properties_result =
            DispatchGetPhysicalDeviceImageFormatProperties2(physical_device, &image_format_info, &image_format_properties);
        image_state.image_format_properties = image_format_properties.imageFormatProperties;
    }
    if (image_properties_result != VK_SUCCESS) {
        skip |= LogError(device, vuid_string,
                         "vkGetPhysicalDeviceImageFormatProperties() or vkGetPhysicalDeviceImageFormatProperties2() unexpectedly "
                         "failed with result = %s, "
                         "when called for validation with following params: "
                         "format: %s, imageType: %s, "
                         "tiling: %s, usage: %s, "
                         "flags: %s.",
                         string_VkResult(image_properties_result), string_VkFormat(image_create_info.format),
                         string_VkImageType(image_create_info.imageType), string_VkImageTiling(image_create_info.tiling),
                         string_VkImageUsageFlags(image_create_info.usage).c_str(),
                         string_VkImageCreateFlags(image_create_info.flags).c_str());
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
            for (uint32_t i = 0; i < rendering_info.colorAttachmentCount; ++i) {
                if (rendering_info.pColorAttachments[i].imageView == VK_NULL_HANDLE) {
                    continue;
                }
                auto view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pColorAttachments[i].imageView);
                auto samples = Get<IMAGE_STATE>(view_state->create_info.image)->createInfo.samples;

                if (samples != pipeline.GetNumSamples()) {
                    const char *vuid_string = IsExtEnabled(device_extensions.vk_ext_multisampled_render_to_single_sampled)
                                                  ? vuid.dynamic_rendering_07285
                                                  : vuid.dynamic_rendering_multi_sample;
                    skip |= LogError(cb_state.commandBuffer(), vuid_string,
                                     "%s: Color attachment (%" PRIu32
                                     ") sample count (%s) must match corresponding VkPipelineMultisampleStateCreateInfo "
                                     "sample count (%s)",
                                     caller, i, string_VkSampleCountFlagBits(samples),
                                     string_VkSampleCountFlagBits(pipeline.GetNumSamples()));
                }
            }

            if ((rendering_info.pDepthAttachment != nullptr) && (rendering_info.pDepthAttachment->imageView != VK_NULL_HANDLE)) {
                const auto &depth_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pDepthAttachment->imageView);
                const auto &depth_image_samples = Get<IMAGE_STATE>(depth_view_state->create_info.image)->createInfo.samples;
                if (depth_image_samples != pipeline.GetNumSamples()) {
                    const char *vuid_string = IsExtEnabled(device_extensions.vk_ext_multisampled_render_to_single_sampled)
                                                  ? vuid.dynamic_rendering_07286
                                                  : vuid.dynamic_rendering_06189;
                    skip |= LogError(cb_state.commandBuffer(), vuid_string,
                                     "%s: Depth attachment sample count (%s) must match corresponding "
                                     "VkPipelineMultisampleStateCreateInfo::rasterizationSamples count (%s)",
                                     caller, string_VkSampleCountFlagBits(depth_image_samples),
                                     string_VkSampleCountFlagBits(pipeline.GetNumSamples()));
                }
            }

            if ((rendering_info.pStencilAttachment != nullptr) &&
                (rendering_info.pStencilAttachment->imageView != VK_NULL_HANDLE)) {
                const auto &stencil_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pStencilAttachment->imageView);
                const auto &stencil_image_samples = Get<IMAGE_STATE>(stencil_view_state->create_info.image)->createInfo.samples;
                if (stencil_image_samples != pipeline.GetNumSamples()) {
                    const char *vuid_string = IsExtEnabled(device_extensions.vk_ext_multisampled_render_to_single_sampled)
                                                  ? vuid.dynamic_rendering_07287
                                                  : vuid.dynamic_rendering_06190;
                    skip |= LogError(cb_state.commandBuffer(), vuid_string,
                                     "%s: Stencil attachment sample count (%s) must match corresponding "
                                     "VkPipelineMultisampleStateCreateInfo::rasterizationSamples count (%s)",
                                     caller, string_VkSampleCountFlagBits(stencil_image_samples),
                                     string_VkSampleCountFlagBits(pipeline.GetNumSamples()));
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
                skip |= ValidateProtectedBuffer(cb_state, buffer_binding.buffer_state.get(), caller,
                                                vuid.unprotected_command_buffer, "Buffer is vertex buffer");
            }
        }
        if (cb_state.index_buffer_binding.bound()) {
            skip |= ValidateProtectedBuffer(cb_state, cb_state.index_buffer_binding.buffer_state.get(), caller,
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
        VkSampleCountFlagBits pso_num_samples = pipeline.GetNumSamples();
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

                if (!(IsExtEnabled(device_extensions.vk_amd_mixed_attachment_samples) ||
                      IsExtEnabled(device_extensions.vk_nv_framebuffer_mixed_samples) ||
                      enabled_features.multisampled_render_to_single_sampled_features.multisampledRenderToSingleSampled) &&
                    ((subpass_num_samples & static_cast<unsigned>(pso_num_samples)) != subpass_num_samples)) {
                    const LogObjectList objlist(pipeline.pipeline(), cb_state.activeRenderPass->renderPass());
                    const char *vuid_string = IsExtEnabled(device_extensions.vk_ext_multisampled_render_to_single_sampled)
                                                  ? vuid.msrtss_rasterization_samples
                                                  : vuid.rasterization_samples;
                    skip |= LogError(objlist, vuid_string,
                                     "%s: In %s the sample count is %s while the current %s has %s and they need to be the same.",
                                     caller, report_data->FormatHandle(pipeline.pipeline()).c_str(),
                                     string_VkSampleCountFlagBits(pso_num_samples),
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
                            result |= ValidateUnprotectedImage(cb_state, view_state->image_state.get(), function,
                                                               vuid.protected_command_buffer, image_desc.c_str());
                        }
                        result |= ValidateProtectedImage(cb_state, view_state->image_state.get(), function,
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
                } else if (!VerifySetLayoutCompatibility(*set_info.bound_descriptor_set, *pipeline_layout, set_index, error_string)) {
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

// Block of code at start here specifically for managing/tracking DSs

// Validate that given set is valid and that it's not being used by an in-flight CmdBuffer
// func_str is the name of the calling function
// Return false if no errors occur
// Return true if validation error occurs and callback returns true (to skip upcoming API call down the chain)
bool CoreChecks::ValidateIdleDescriptorSet(VkDescriptorSet set, const char *func_str) const {
    if (disabled[object_in_use]) return false;
    bool skip = false;
    auto set_node = Get<cvdescriptorset::DescriptorSet>(set);
    if (set_node) {
        // TODO : This covers various error cases so should pass error enum into this function and use passed in enum here
        if (set_node->InUse()) {
            skip |= LogError(set, "VUID-vkFreeDescriptorSets-pDescriptorSets-00309",
                             "Cannot call %s() on %s that is in use by a command buffer.", func_str,
                             report_data->FormatHandle(set).c_str());
        }
    }
    return skip;
}

// If a renderpass is active, verify that the given command type is appropriate for current subpass state
bool CoreChecks::ValidateCmdSubpassState(const CMD_BUFFER_STATE &cb_state, const CMD_TYPE cmd_type) const {
    if (!cb_state.activeRenderPass) return false;
    bool skip = false;
    if (cb_state.createInfo.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY &&
        cb_state.activeSubpassContents == VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS &&
        (cmd_type != CMD_EXECUTECOMMANDS && cmd_type != CMD_NEXTSUBPASS && cmd_type != CMD_ENDRENDERPASS &&
         cmd_type != CMD_NEXTSUBPASS2 && cmd_type != CMD_NEXTSUBPASS2KHR && cmd_type != CMD_ENDRENDERPASS2 &&
         cmd_type != CMD_ENDRENDERPASS2KHR)) {
        skip |=
            LogError(cb_state.commandBuffer(), kVUID_Core_DrawState_InvalidCommandBuffer,
                     "%s() cannot be called in a subpass using secondary command buffers.", kGeneratedCommandNameList[cmd_type]);
    }
    return skip;
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
        const auto& obj = entry.first;
        const char *cause_str = GetCauseStr(obj);
        string vuid;
        std::ostringstream str;
        str << kVUID_Core_DrawState_InvalidCommandBuffer << "-" << object_string[obj.type];
        vuid = str.str();
        auto objlist = entry.second; //intentional copy
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

    skip |= ValidateMemoryIsBoundToBuffer(&buffer_state, caller_name, vuid.indirect_contiguous_memory);
    skip |= ValidateBufferUsageFlags(&buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true, vuid.indirect_buffer_bit,
                                     caller_name, "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    if (cb_state.unprotected == false) {
        skip |= LogError(cb_state.Handle(), vuid.indirect_protected_cb,
                         "%s: Indirect commands can't be used in protected command buffers.", caller_name);
    }
    return skip;
}

bool CoreChecks::ValidateIndirectCountCmd(const BUFFER_STATE &count_buffer_state, VkDeviceSize count_buffer_offset,
                                          CMD_TYPE cmd_type) const {
    bool skip = false;
    const DrawDispatchVuid vuid = GetDrawDispatchVuid(cmd_type);
    const char *caller_name = CommandTypeString(cmd_type);

    skip |= ValidateMemoryIsBoundToBuffer(&count_buffer_state, caller_name, vuid.indirect_count_contiguous_memory);
    skip |= ValidateBufferUsageFlags(&count_buffer_state, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, true, vuid.indirect_count_buffer_bit,
                                     caller_name, "VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT");
    if (count_buffer_offset + sizeof(uint32_t) > count_buffer_state.createInfo.size) {
        skip |= LogError(count_buffer_state.buffer(), vuid.indirect_count_offset,
                         "%s: countBufferOffset (%" PRIu64 ") + sizeof(uint32_t) is greater than the buffer size of %" PRIu64 ".",
                         caller_name, count_buffer_offset, count_buffer_state.createInfo.size);
    }
    return skip;
}

bool CoreChecks::ValidateDeviceMaskToPhysicalDeviceCount(uint32_t deviceMask, const LogObjectList &objlist,
                                                         const char *VUID) const {
    bool skip = false;
    uint32_t count = 1 << physical_device_count;
    if (count <= deviceMask) {
        skip |= LogError(objlist, VUID, "deviceMask(0x%" PRIx32 ") is invalid. Physical device count is %" PRIu32 ".", deviceMask,
                         physical_device_count);
    }
    return skip;
}

bool CoreChecks::ValidateDeviceMaskToZero(uint32_t deviceMask, const LogObjectList &objlist, const char *VUID) const {
    bool skip = false;
    if (deviceMask == 0) {
        skip |= LogError(objlist, VUID, "deviceMask(0x%" PRIx32 ") must be non-zero.", deviceMask);
    }
    return skip;
}

bool CoreChecks::ValidateDeviceMaskToCommandBuffer(const CMD_BUFFER_STATE &cb_state, uint32_t deviceMask,
                                                   const LogObjectList &objlist, const char *VUID) const {
    bool skip = false;
    if ((deviceMask & cb_state.initial_device_mask) != deviceMask) {
        skip |= LogError(objlist, VUID, "deviceMask(0x%" PRIx32 ") is not a subset of %s initial device mask(0x%" PRIx32 ").",
                         deviceMask, report_data->FormatHandle(cb_state.commandBuffer()).c_str(), cb_state.initial_device_mask);
    }
    return skip;
}

bool CoreChecks::ValidateDeviceMaskToRenderPass(const CMD_BUFFER_STATE &cb_state, uint32_t deviceMask, const char *VUID) const {
    bool skip = false;
    if ((deviceMask & cb_state.active_render_pass_device_mask) != deviceMask) {
        skip |=
            LogError(cb_state.commandBuffer(), VUID, "deviceMask(0x%" PRIx32 ") is not a subset of %s device mask(0x%" PRIx32 ").",
                     deviceMask, report_data->FormatHandle(cb_state.activeRenderPass->renderPass()).c_str(),
                     cb_state.active_render_pass_device_mask);
    }
    return skip;
}

bool CoreChecks::ValidateQueueFamilyIndex(const PHYSICAL_DEVICE_STATE *pd_state, uint32_t requested_queue_family,
                                          const char *err_code, const char *cmd_name, const char *queue_family_var_name) const {
    bool skip = false;

    if (requested_queue_family >= pd_state->queue_family_known_count) {
        const char *conditional_ext_cmd =
            instance_extensions.vk_khr_get_physical_device_properties2 ? " or vkGetPhysicalDeviceQueueFamilyProperties2[KHR]" : "";

        skip |= LogError(pd_state->Handle(), err_code,
                         "%s: %s (= %" PRIu32
                         ") is not less than any previously obtained pQueueFamilyPropertyCount from "
                         "vkGetPhysicalDeviceQueueFamilyProperties%s (i.e. is not less than %s).",
                         cmd_name, queue_family_var_name, requested_queue_family, conditional_ext_cmd,
                         std::to_string(pd_state->queue_family_known_count).c_str());
    }
    return skip;
}

// Verify VkDeviceQueueCreateInfos
bool CoreChecks::ValidateDeviceQueueCreateInfos(const PHYSICAL_DEVICE_STATE *pd_state, uint32_t info_count,
                                                const VkDeviceQueueCreateInfo *infos) const {
    bool skip = false;

    const uint32_t not_used = std::numeric_limits<uint32_t>::max();
    struct create_flags {
        // uint32_t is to represent the queue family index to allow for better error messages
        uint32_t unprocted_index;
        uint32_t protected_index;
        create_flags(uint32_t a, uint32_t b) : unprocted_index(a), protected_index(b) {}
    };
    layer_data::unordered_map<uint32_t, create_flags> queue_family_map;
    layer_data::unordered_map<uint32_t, VkQueueGlobalPriorityKHR> global_priorities;

    for (uint32_t i = 0; i < info_count; ++i) {
        const auto requested_queue_family = infos[i].queueFamilyIndex;
        const bool protected_create_bit = (infos[i].flags & VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT) != 0;

        std::string queue_family_var_name = "pCreateInfo->pQueueCreateInfos[" + std::to_string(i) + "].queueFamilyIndex";
        skip |= ValidateQueueFamilyIndex(pd_state, requested_queue_family, "VUID-VkDeviceQueueCreateInfo-queueFamilyIndex-00381",
                                         "vkCreateDevice", queue_family_var_name.c_str());
        if (skip) {  // Skip if queue family index is invalid, as it will be used as index in arrays
            continue;
        }

        if (api_version == VK_API_VERSION_1_0) {
            // Vulkan 1.0 didn't have protected memory so always needed unique info
            create_flags flags = {requested_queue_family, not_used};
            if (queue_family_map.emplace(requested_queue_family, flags).second == false) {
                skip |= LogError(pd_state->Handle(), "VUID-VkDeviceCreateInfo-queueFamilyIndex-00372",
                                 "CreateDevice(): %s (=%" PRIu32
                                 ") is not unique and was also used in pCreateInfo->pQueueCreateInfos[%d].",
                                 queue_family_var_name.c_str(), requested_queue_family,
                                 queue_family_map.at(requested_queue_family).unprocted_index);
            }
        } else {
            // Vulkan 1.1 and up can have 2 queues be same family index if one is protected and one isn't
            auto it = queue_family_map.find(requested_queue_family);
            if (it == queue_family_map.end()) {
                // Add first time seeing queue family index and what the create flags were
                create_flags new_flags = {not_used, not_used};
                if (protected_create_bit) {
                    new_flags.protected_index = requested_queue_family;
                } else {
                    new_flags.unprocted_index = requested_queue_family;
                }
                queue_family_map.emplace(requested_queue_family, new_flags);
            } else {
                // The queue family was seen, so now need to make sure the flags were different
                if (protected_create_bit) {
                    if (it->second.protected_index != not_used) {
                        skip |= LogError(pd_state->Handle(), "VUID-VkDeviceCreateInfo-queueFamilyIndex-02802",
                                         "CreateDevice(): %s (=%" PRIu32
                                         ") is not unique and was also used in pCreateInfo->pQueueCreateInfos[%d] which both have "
                                         "VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT.",
                                         queue_family_var_name.c_str(), requested_queue_family,
                                         queue_family_map.at(requested_queue_family).protected_index);
                    } else {
                        it->second.protected_index = requested_queue_family;
                    }
                } else {
                    if (it->second.unprocted_index != not_used) {
                        skip |= LogError(pd_state->Handle(), "VUID-VkDeviceCreateInfo-queueFamilyIndex-02802",
                                         "CreateDevice(): %s (=%" PRIu32
                                         ") is not unique and was also used in pCreateInfo->pQueueCreateInfos[%d].",
                                         queue_family_var_name.c_str(), requested_queue_family,
                                         queue_family_map.at(requested_queue_family).unprocted_index);
                    } else {
                        it->second.unprocted_index = requested_queue_family;
                    }
                }
            }
        }

        VkQueueGlobalPriorityKHR global_priority = VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_KHR;  // Implicit default value
        const auto *global_priority_ci = LvlFindInChain<VkDeviceQueueGlobalPriorityCreateInfoKHR>(infos[i].pNext);
        if (global_priority_ci) {
            global_priority = global_priority_ci->globalPriority;
        }
        const auto prev_global_priority = global_priorities.find(infos[i].queueFamilyIndex);
        if (prev_global_priority != global_priorities.end()) {
            if (prev_global_priority->second != global_priority) {
                skip |= LogError(pd_state->Handle(), "VUID-VkDeviceCreateInfo-pQueueCreateInfos-06654",
                                 "vkCreateDevice(): Multiple queues are created with queueFamilyIndex %" PRIu32
                                 ", but one has global priority %s and another %s.",
                                 infos[i].queueFamilyIndex, string_VkQueueGlobalPriorityKHR(prev_global_priority->second),
                                 string_VkQueueGlobalPriorityKHR(global_priority));
            }
        } else {
            global_priorities.insert({infos[i].queueFamilyIndex, global_priority});
        }

        const VkQueueFamilyProperties requested_queue_family_props = pd_state->queue_family_properties[requested_queue_family];

        // if using protected flag, make sure queue supports it
        if (protected_create_bit && ((requested_queue_family_props.queueFlags & VK_QUEUE_PROTECTED_BIT) == 0)) {
            skip |= LogError(
                pd_state->Handle(), "VUID-VkDeviceQueueCreateInfo-flags-06449",
                "CreateDevice(): %s (=%" PRIu32
                ") does not have VK_QUEUE_PROTECTED_BIT supported, but VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT is being used.",
                queue_family_var_name.c_str(), requested_queue_family);
        }

        // Verify that requested queue count of queue family is known to be valid at this point in time
        if (requested_queue_family < pd_state->queue_family_known_count) {
            const auto requested_queue_count = infos[i].queueCount;
            const bool queue_family_has_props = requested_queue_family < pd_state->queue_family_properties.size();
            // spec guarantees at least one queue for each queue family
            const uint32_t available_queue_count = queue_family_has_props ? requested_queue_family_props.queueCount : 1;
            const char *conditional_ext_cmd = instance_extensions.vk_khr_get_physical_device_properties2
                                                  ? " or vkGetPhysicalDeviceQueueFamilyProperties2[KHR]"
                                                  : "";

            if (requested_queue_count > available_queue_count) {
                const std::string count_note =
                    queue_family_has_props
                        ? "i.e. is not less than or equal to " + std::to_string(requested_queue_family_props.queueCount)
                        : "the pQueueFamilyProperties[" + std::to_string(requested_queue_family) + "] was never obtained";

                skip |= LogError(
                    pd_state->Handle(), "VUID-VkDeviceQueueCreateInfo-queueCount-00382",
                    "vkCreateDevice: pCreateInfo->pQueueCreateInfos[%" PRIu32 "].queueCount (=%" PRIu32
                    ") is not less than or equal to available queue count for this pCreateInfo->pQueueCreateInfos[%" PRIu32
                    "].queueFamilyIndex} (=%" PRIu32 ") obtained previously from vkGetPhysicalDeviceQueueFamilyProperties%s (%s).",
                    i, requested_queue_count, i, requested_queue_family, conditional_ext_cmd, count_note.c_str());
            }
        }

        const VkQueueFlags queue_flags = pd_state->queue_family_properties[requested_queue_family].queueFlags;
        if ((infos[i].flags == VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT) && ((queue_flags & VK_QUEUE_PROTECTED_BIT) == VK_FALSE)) {
            skip |= LogError(pd_state->Handle(), "VUID-VkDeviceQueueCreateInfo-flags-06449",
                             "vkCreateDevice: pCreateInfo->flags set to VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT on a queue that "
                             "doesn't include VK_QUEUE_PROTECTED_BIT capability");
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator, VkDevice *pDevice) const {
    bool skip = false;
    auto pd_state = Get<PHYSICAL_DEVICE_STATE>(gpu);

    // TODO: object_tracker should perhaps do this instead
    //       and it does not seem to currently work anyway -- the loader just crashes before this point
    if (!pd_state) {
        skip |= LogError(device, "VUID-vkCreateDevice-physicalDevice-parameter",
                         "Invalid call to vkCreateDevice() w/o first calling vkEnumeratePhysicalDevices().");
    } else {
        skip |= ValidateDeviceQueueCreateInfos(pd_state.get(), pCreateInfo->queueCreateInfoCount, pCreateInfo->pQueueCreateInfos);

        const VkPhysicalDeviceFragmentShadingRateFeaturesKHR *fragment_shading_rate_features =
            LvlFindInChain<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>(pCreateInfo->pNext);

        if (fragment_shading_rate_features) {
            const VkPhysicalDeviceShadingRateImageFeaturesNV *shading_rate_image_features =
                LvlFindInChain<VkPhysicalDeviceShadingRateImageFeaturesNV>(pCreateInfo->pNext);

            if (shading_rate_image_features && shading_rate_image_features->shadingRateImage) {
                if (fragment_shading_rate_features->pipelineFragmentShadingRate) {
                    skip |= LogError(
                        pd_state->Handle(), "VUID-VkDeviceCreateInfo-shadingRateImage-04478",
                        "vkCreateDevice: Cannot enable shadingRateImage and pipelineFragmentShadingRate features simultaneously.");
                }
                if (fragment_shading_rate_features->primitiveFragmentShadingRate) {
                    skip |= LogError(
                        pd_state->Handle(), "VUID-VkDeviceCreateInfo-shadingRateImage-04479",
                        "vkCreateDevice: Cannot enable shadingRateImage and primitiveFragmentShadingRate features simultaneously.");
                }
                if (fragment_shading_rate_features->attachmentFragmentShadingRate) {
                    skip |= LogError(pd_state->Handle(), "VUID-VkDeviceCreateInfo-shadingRateImage-04480",
                                     "vkCreateDevice: Cannot enable shadingRateImage and attachmentFragmentShadingRate features "
                                     "simultaneously.");
                }
            }

            const VkPhysicalDeviceFragmentDensityMapFeaturesEXT *fragment_density_map_features =
                LvlFindInChain<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>(pCreateInfo->pNext);

            if (fragment_density_map_features && fragment_density_map_features->fragmentDensityMap) {
                if (fragment_shading_rate_features->pipelineFragmentShadingRate) {
                    skip |= LogError(pd_state->Handle(), "VUID-VkDeviceCreateInfo-fragmentDensityMap-04481",
                                     "vkCreateDevice: Cannot enable fragmentDensityMap and pipelineFragmentShadingRate features "
                                     "simultaneously.");
                }
                if (fragment_shading_rate_features->primitiveFragmentShadingRate) {
                    skip |= LogError(pd_state->Handle(), "VUID-VkDeviceCreateInfo-fragmentDensityMap-04482",
                                     "vkCreateDevice: Cannot enable fragmentDensityMap and primitiveFragmentShadingRate features "
                                     "simultaneously.");
                }
                if (fragment_shading_rate_features->attachmentFragmentShadingRate) {
                    skip |= LogError(pd_state->Handle(), "VUID-VkDeviceCreateInfo-fragmentDensityMap-04483",
                                     "vkCreateDevice: Cannot enable fragmentDensityMap and attachmentFragmentShadingRate features "
                                     "simultaneously.");
                }
            }
        }

        const auto *shader_image_atomic_int64_features =
            LvlFindInChain<VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT>(pCreateInfo->pNext);
        if (shader_image_atomic_int64_features) {
            if (shader_image_atomic_int64_features->sparseImageInt64Atomics &&
                !shader_image_atomic_int64_features->shaderImageInt64Atomics) {
                skip |= LogError(pd_state->Handle(), "VUID-VkDeviceCreateInfo-None-04896",
                                 "vkCreateDevice: if shaderImageInt64Atomics feature is enabled then sparseImageInt64Atomics "
                                 "feature must also be enabled.");
            }
        }
        const auto *shader_atomic_float_features = LvlFindInChain<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>(pCreateInfo->pNext);
        if (shader_atomic_float_features) {
            if (shader_atomic_float_features->sparseImageFloat32Atomics &&
                !shader_atomic_float_features->shaderImageFloat32Atomics) {
                skip |= LogError(pd_state->Handle(), "VUID-VkDeviceCreateInfo-None-04897",
                                 "vkCreateDevice: if sparseImageFloat32Atomics feature is enabled then shaderImageFloat32Atomics "
                                 "feature must also be enabled.");
            }
            if (shader_atomic_float_features->sparseImageFloat32AtomicAdd &&
                !shader_atomic_float_features->shaderImageFloat32AtomicAdd) {
                skip |=
                    LogError(pd_state->Handle(), "VUID-VkDeviceCreateInfo-None-04898",
                             "vkCreateDevice: if sparseImageFloat32AtomicAdd feature is enabled then shaderImageFloat32AtomicAdd "
                             "feature must also be enabled.");
            }
        }
        const auto *shader_atomic_float2_features =
            LvlFindInChain<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT>(pCreateInfo->pNext);
        if (shader_atomic_float2_features) {
            if (shader_atomic_float2_features->sparseImageFloat32AtomicMinMax &&
                !shader_atomic_float2_features->shaderImageFloat32AtomicMinMax) {
                skip |= LogError(
                    pd_state->Handle(), "VUID-VkDeviceCreateInfo-sparseImageFloat32AtomicMinMax-04975",
                    "vkCreateDevice: if sparseImageFloat32AtomicMinMax feature is enabled then shaderImageFloat32AtomicMinMax "
                    "feature must also be enabled.");
            }
        }
        const auto *device_group_ci = LvlFindInChain<VkDeviceGroupDeviceCreateInfo>(pCreateInfo->pNext);
        if (device_group_ci) {
            for (uint32_t i = 0; i < device_group_ci->physicalDeviceCount - 1; ++i) {
                for (uint32_t j = i + 1; j < device_group_ci->physicalDeviceCount; ++j) {
                    if (device_group_ci->pPhysicalDevices[i] == device_group_ci->pPhysicalDevices[j]) {
                        skip |= LogError(pd_state->Handle(), "VUID-VkDeviceGroupDeviceCreateInfo-pPhysicalDevices-00375",
                                         "vkCreateDevice: VkDeviceGroupDeviceCreateInfo has a duplicated physical device "
                                         "in pPhysicalDevices [%" PRIu32 "] and [%" PRIu32 "].",
                                         i, j);
                    }
                }
            }
        }
    }
    return skip;
}

void CoreChecks::CreateDevice(const VkDeviceCreateInfo *pCreateInfo) {
    // The state tracker sets up the device state
    StateTracker::CreateDevice(pCreateInfo);

    // Add the callback hooks for the functions that are either broadly or deeply used and that the ValidationStateTracker refactor
    // would be messier without.
    // TODO: Find a good way to do this hooklessly.
    SetSetImageViewInitialLayoutCallback(
        [](CMD_BUFFER_STATE *cb_state, const IMAGE_VIEW_STATE &iv_state, VkImageLayout layout) -> void {
            cb_state->SetImageViewInitialLayout(iv_state, layout);
        });

    // Allocate shader validation cache
    if (!disabled[shader_validation_caching] && !disabled[shader_validation] && !core_validation_cache) {
        auto tmp_path = GetEnvironment("XDG_CACHE_HOME");
        if (!tmp_path.size()) {
            auto cachepath = GetEnvironment("HOME") + "/.cache";
            struct stat info;
            if (stat(cachepath.c_str(), &info) == 0) {
                if ((info.st_mode & S_IFMT) == S_IFDIR) {
                    tmp_path = cachepath;
                }
            }
        }
        if (!tmp_path.size()) tmp_path = GetEnvironment("TMPDIR");
        if (!tmp_path.size()) tmp_path = GetEnvironment("TMP");
        if (!tmp_path.size()) tmp_path = GetEnvironment("TEMP");
        if (!tmp_path.size()) tmp_path = "/tmp";
        validation_cache_path = tmp_path + "/shader_validation_cache";
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
        validation_cache_path += "-" + std::to_string(getuid());
#endif
        validation_cache_path += ".bin";

        std::vector<char> validation_cache_data;
        std::ifstream read_file(validation_cache_path.c_str(), std::ios::in | std::ios::binary);

        if (read_file) {
            std::copy(std::istreambuf_iterator<char>(read_file), {}, std::back_inserter(validation_cache_data));
            read_file.close();
        } else {
            LogInfo(device, "UNASSIGNED-cache-file-error",
                    "Cannot open shader validation cache at %s for reading (it may not exist yet)", validation_cache_path.c_str());
        }

        VkValidationCacheCreateInfoEXT cacheCreateInfo = LvlInitStruct<VkValidationCacheCreateInfoEXT>();
        cacheCreateInfo.initialDataSize = validation_cache_data.size();
        cacheCreateInfo.pInitialData = validation_cache_data.data();
        cacheCreateInfo.flags = 0;
        CoreLayerCreateValidationCacheEXT(device, &cacheCreateInfo, nullptr, &core_validation_cache);
    }
}

void CoreChecks::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    if (!device) return;

    StateTracker::PreCallRecordDestroyDevice(device, pAllocator);

    if (core_validation_cache) {
        size_t validation_cache_size = 0;
        void *validation_cache_data = nullptr;

        CoreLayerGetValidationCacheDataEXT(device, core_validation_cache, &validation_cache_size, nullptr);

        validation_cache_data = (char *)malloc(sizeof(char) * validation_cache_size);
        if (!validation_cache_data) {
            LogInfo(device, "UNASSIGNED-cache-memory-error", "Validation Cache Memory Error");
            return;
        }

        VkResult result =
            CoreLayerGetValidationCacheDataEXT(device, core_validation_cache, &validation_cache_size, validation_cache_data);

        if (result != VK_SUCCESS) {
            LogInfo(device, "UNASSIGNED-cache-retrieval-error", "Validation Cache Retrieval Error");
            free(validation_cache_data);
            return;
        }

        if (validation_cache_path.size() > 0) {
            std::ofstream write_file(validation_cache_path.c_str(), std::ios::out | std::ios::binary);
            if (write_file) {
                write_file.write(static_cast<char *>(validation_cache_data), validation_cache_size);
                write_file.close();
            } else {
                LogInfo(device, "UNASSIGNED-cache-write-error", "Cannot open shader validation cache at %s for writing",
                        validation_cache_path.c_str());
            }
        }
        free(validation_cache_data);
        CoreLayerDestroyValidationCacheEXT(device, core_validation_cache, NULL);
    }
}

bool CoreChecks::ValidateStageMaskHost(const Location &loc, VkPipelineStageFlags2KHR stageMask) const {
    bool skip = false;
    if ((stageMask & VK_PIPELINE_STAGE_HOST_BIT) != 0) {
        const auto &vuid = sync_vuid_maps::GetQueueSubmitVUID(loc, sync_vuid_maps::SubmitError::kHostStageMask);
        skip |= LogError(
            device, vuid,
            "%s stage mask must not include VK_PIPELINE_STAGE_HOST_BIT as the stage can't be invoked inside a command buffer.",
            loc.Message().c_str());
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

bool CoreChecks::ValidateFenceForSubmit(const FENCE_STATE *fence_state, const char *inflight_vuid, const char *retired_vuid,
                                        const char *func_name) const {
    bool skip = false;

    if (fence_state && fence_state->Scope() == kSyncScopeInternal) {
        switch (fence_state->State()) {
            case FENCE_INFLIGHT:
                skip |= LogError(fence_state->fence(), inflight_vuid, "%s: %s is already in use by another submission.", func_name,
                                 report_data->FormatHandle(fence_state->fence()).c_str());
                break;
            case FENCE_RETIRED:
                skip |= LogError(fence_state->fence(), retired_vuid,
                                 "%s: %s submitted in SIGNALED state.  Fences must be reset before being submitted", func_name,
                                 report_data->FormatHandle(fence_state->fence()).c_str());
                break;
            default:
                break;
        }
    }

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

bool SemaphoreSubmitState::ValidateBinaryWait(const core_error::Location &loc, VkQueue queue,
                                              const SEMAPHORE_STATE &semaphore_state) {
    using sync_vuid_maps::GetQueueSubmitVUID;
    using sync_vuid_maps::SubmitError;

    bool skip = false;
    auto semaphore = semaphore_state.semaphore();
    if ((semaphore_state.Scope() == kSyncScopeInternal || internal_semaphores.count(semaphore))) {
        VkQueue other_queue = AnotherQueueWaits(semaphore_state);
        if (other_queue) {
            const auto &vuid = GetQueueSubmitVUID(loc, SubmitError::kOtherQueueWaiting);
            const LogObjectList objlist(semaphore, queue, other_queue);
            skip |= core->LogError(objlist, vuid, "%s Queue %s is already waiting on semaphore (%s).", loc.Message().c_str(),
                                   core->report_data->FormatHandle(other_queue).c_str(),
                                   core->report_data->FormatHandle(semaphore).c_str());
        } else if (CannotWait(semaphore_state)) {
            auto error = IsExtEnabled(core->device_extensions.vk_khr_timeline_semaphore) ? SubmitError::kBinaryCannotBeSignalled
                                                                                         : SubmitError::kOldBinaryCannotBeSignalled;
            const auto &vuid = GetQueueSubmitVUID(loc, error);
            const LogObjectList objlist(semaphore, queue);
            skip |= core->LogError(
                objlist, semaphore_state.Scope() == kSyncScopeInternal ? vuid : kVUID_Core_DrawState_QueueForwardProgress,
                "%s Queue %s is waiting on semaphore (%s) that has no way to be signaled.", loc.Message().c_str(),
                core->report_data->FormatHandle(queue).c_str(), core->report_data->FormatHandle(semaphore).c_str());
        } else {
            signaled_semaphores.erase(semaphore);
            unsignaled_semaphores.insert(semaphore);
        }
    } else if (semaphore_state.Scope() == kSyncScopeExternalTemporary) {
        internal_semaphores.insert(semaphore);
    }
    return skip;
}

bool SemaphoreSubmitState::ValidateWaitSemaphore(const core_error::Location &loc, VkSemaphore semaphore, uint64_t value) {
    using sync_vuid_maps::GetQueueSubmitVUID;
    using sync_vuid_maps::SubmitError;
    bool skip = false;

    auto semaphore_state = core->Get<SEMAPHORE_STATE>(semaphore);
    if (!semaphore_state) {
        return skip;
    }
    switch (semaphore_state->type) {
        case VK_SEMAPHORE_TYPE_BINARY:
            skip = ValidateBinaryWait(loc, queue, *semaphore_state);
            break;
        case VK_SEMAPHORE_TYPE_TIMELINE: {
            uint64_t bad_value = 0;
            std::string where;
            TimelineMaxDiffCheck exceeds_max_diff(value, core->phys_dev_props_core12.maxTimelineSemaphoreValueDifference);
            if (CheckSemaphoreValue(*semaphore_state, where, bad_value, exceeds_max_diff)) {
                const auto &vuid = GetQueueSubmitVUID(loc, SubmitError::kTimelineSemMaxDiff);
                skip |= core->LogError(
                    semaphore, vuid, "%s value (%" PRIu64 ") exceeds limit regarding %s semaphore %s value (%" PRIu64 ").",
                    loc.Message().c_str(), value, where.c_str(), core->report_data->FormatHandle(semaphore).c_str(), bad_value);
                break;
            }
            timeline_waits[semaphore] = value;
        } break;
        default:
            break;
    }
    return skip;
}

bool SemaphoreSubmitState::ValidateSignalSemaphore(const core_error::Location &loc, VkSemaphore semaphore, uint64_t value) {
    using sync_vuid_maps::GetQueueSubmitVUID;
    using sync_vuid_maps::SubmitError;
    bool skip = false;
    LogObjectList objlist(semaphore, queue);

    auto semaphore_state = core->Get<SEMAPHORE_STATE>(semaphore);
    if (!semaphore_state) {
        return skip;
    }
    switch (semaphore_state->type) {
        case VK_SEMAPHORE_TYPE_BINARY: {
            if ((semaphore_state->Scope() == kSyncScopeInternal || internal_semaphores.count(semaphore))) {
                VkQueue other_queue = VK_NULL_HANDLE;
                if (CannotSignal(*semaphore_state, other_queue)) {
                    objlist.add(other_queue);
                    skip |= core->LogError(objlist, kVUID_Core_DrawState_QueueForwardProgress,
                                           "%s is signaling %s (%s) that was previously "
                                           "signaled by %s but has not since been waited on by any queue.",
                                           loc.Message().c_str(), core->report_data->FormatHandle(queue).c_str(),
                                           core->report_data->FormatHandle(semaphore).c_str(),
                                           core->report_data->FormatHandle(other_queue).c_str());
                } else {
                    unsignaled_semaphores.erase(semaphore);
                    signaled_semaphores.insert(semaphore);
                }
            }
            break;
        }
        case VK_SEMAPHORE_TYPE_TIMELINE: {
            uint64_t bad_value = 0;
            std::string where;
            auto must_be_greater = [value](const SEMAPHORE_STATE::SemOp &op, bool is_pending) {
                if (!op.IsSignal()) {
                    return false;
                }
                // duplicate signal values are never allowed.
                if (value == op.payload) {
                    return true;
                }
                // exact value ordering cannot be determined until execution time
                return !is_pending && value < op.payload;
            };
            if (CheckSemaphoreValue(*semaphore_state, where, bad_value, must_be_greater)) {
                const auto &vuid = GetQueueSubmitVUID(loc, SubmitError::kTimelineSemSmallValue);
                skip |= core->LogError(objlist, vuid,
                                       "At submit time, %s signal value (0x%" PRIx64
                                       ") in %s must be greater than %s timeline semaphore %s value (0x%" PRIx64 ")",
                                       loc.Message().c_str(), value, core->report_data->FormatHandle(queue).c_str(), where.c_str(),
                                       core->report_data->FormatHandle(semaphore).c_str(), bad_value);
                break;
            }
            TimelineMaxDiffCheck exceeds_max_diff(value, core->phys_dev_props_core12.maxTimelineSemaphoreValueDifference);
            if (CheckSemaphoreValue(*semaphore_state, where, bad_value, exceeds_max_diff)) {
                const auto &vuid = GetQueueSubmitVUID(loc, SubmitError::kTimelineSemMaxDiff);
                skip |= core->LogError(
                    semaphore, vuid, "%s value (%" PRIu64 ") exceeds limit regarding %s semaphore %s value (%" PRIu64 ").",
                    loc.Message().c_str(), value, where.c_str(), core->report_data->FormatHandle(semaphore).c_str(), bad_value);
                break;
            }
            timeline_signals[semaphore] = value;
            break;
        }
        default:
            break;
    }
    return skip;
}

bool CoreChecks::ValidateSemaphoresForSubmit(SemaphoreSubmitState &state, const VkSubmitInfo &submit,
                                             const Location &outer_loc) const {
    bool skip = false;
    auto *timeline_semaphore_submit_info = LvlFindInChain<VkTimelineSemaphoreSubmitInfo>(submit.pNext);
    for (uint32_t i = 0; i < submit.waitSemaphoreCount; ++i) {
        uint64_t value = 0;
        VkSemaphore semaphore = submit.pWaitSemaphores[i];

        if (submit.pWaitDstStageMask) {
            const LogObjectList objlist(semaphore, state.queue);
            auto loc = outer_loc.dot(Field::pWaitDstStageMask, i);
            skip |= ValidatePipelineStage(objlist, loc, state.queue_flags, submit.pWaitDstStageMask[i]);
            skip |= ValidateStageMaskHost(loc, submit.pWaitDstStageMask[i]);
        }
        auto semaphore_state = Get<SEMAPHORE_STATE>(semaphore);
        if (!semaphore_state) {
            continue;
        }
        auto loc = outer_loc.dot(Field::pWaitSemaphores, i);
        if (semaphore_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            if (timeline_semaphore_submit_info == nullptr) {
                skip |= LogError(semaphore, "VUID-VkSubmitInfo-pWaitSemaphores-03239",
                                 "%s (%s) is a timeline semaphore, but VkSubmitInfo does "
                                 "not include an instance of VkTimelineSemaphoreSubmitInfo",
                                 loc.Message().c_str(), report_data->FormatHandle(semaphore).c_str());
                break;
            } else if (submit.waitSemaphoreCount != timeline_semaphore_submit_info->waitSemaphoreValueCount) {
                skip |= LogError(semaphore, "VUID-VkSubmitInfo-pNext-03240",
                                 "%s (%s) is a timeline semaphore, it contains an "
                                 "instance of VkTimelineSemaphoreSubmitInfo, but waitSemaphoreValueCount (%u) is different than "
                                 "waitSemaphoreCount (%u)",
                                 loc.Message().c_str(), report_data->FormatHandle(semaphore).c_str(),
                                 timeline_semaphore_submit_info->waitSemaphoreValueCount, submit.waitSemaphoreCount);
                break;
            }
            value = timeline_semaphore_submit_info->pWaitSemaphoreValues[i];
        }
        skip |= state.ValidateWaitSemaphore(outer_loc.dot(Field::pWaitSemaphores, i), semaphore, value);
    }
    for (uint32_t i = 0; i < submit.signalSemaphoreCount; ++i) {
        VkSemaphore semaphore = submit.pSignalSemaphores[i];
        uint64_t value = 0;
        auto semaphore_state = Get<SEMAPHORE_STATE>(semaphore);
        if (!semaphore_state) {
            continue;
        }
        auto loc = outer_loc.dot(Field::pSignalSemaphores, i);
        if (semaphore_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            if (timeline_semaphore_submit_info == nullptr) {
                skip |= LogError(semaphore, "VUID-VkSubmitInfo-pWaitSemaphores-03239",
                                 "%s (%s) is a timeline semaphore, but VkSubmitInfo"
                                 "does not include an instance of VkTimelineSemaphoreSubmitInfo",
                                 loc.Message().c_str(), report_data->FormatHandle(semaphore).c_str());
                break;
            } else if (submit.signalSemaphoreCount != timeline_semaphore_submit_info->signalSemaphoreValueCount) {
                skip |= LogError(semaphore, "VUID-VkSubmitInfo-pNext-03241",
                                 "%s (%s) is a timeline semaphore, it contains an "
                                 "instance of VkTimelineSemaphoreSubmitInfo, but signalSemaphoreValueCount (%u) is different than "
                                 "signalSemaphoreCount (%u)",
                                 loc.Message().c_str(), report_data->FormatHandle(semaphore).c_str(),
                                 timeline_semaphore_submit_info->signalSemaphoreValueCount, submit.signalSemaphoreCount);
                break;
            }
            value = timeline_semaphore_submit_info->pSignalSemaphoreValues[i];
        }
        skip |= state.ValidateSignalSemaphore(loc, semaphore, value);
    }
    return skip;
}

bool CoreChecks::ValidateSemaphoresForSubmit(SemaphoreSubmitState &state, const VkSubmitInfo2KHR &submit,
                                             const Location &outer_loc) const {
    bool skip = false;
    for (uint32_t i = 0; i < submit.waitSemaphoreInfoCount; ++i) {
        const auto &wait_info = submit.pWaitSemaphoreInfos[i];
        Location loc = outer_loc.dot(Field::pWaitSemaphoreInfos, i);
        skip |= ValidatePipelineStage(LogObjectList(wait_info.semaphore), loc.dot(Field::stageMask), state.queue_flags,
                                      wait_info.stageMask);
        skip |= ValidateStageMaskHost(loc.dot(Field::stageMask), wait_info.stageMask);
        skip |= state.ValidateWaitSemaphore(loc, wait_info.semaphore, wait_info.value);

        auto semaphore_state = Get<SEMAPHORE_STATE>(wait_info.semaphore);
        if (semaphore_state && semaphore_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            for (uint32_t sig_index = 0; sig_index < submit.signalSemaphoreInfoCount; sig_index++) {
                const auto &sig_info = submit.pSignalSemaphoreInfos[sig_index];
                if (wait_info.semaphore == sig_info.semaphore && wait_info.value >= sig_info.value) {
                    Location sig_loc = outer_loc.dot(Field::pSignalSemaphoreInfos, sig_index);
                    const LogObjectList objlist(wait_info.semaphore, state.queue);
                    skip |= LogError(wait_info.semaphore, "VUID-VkSubmitInfo2-semaphore-03881",
                                     "%s has value (%" PRIu64 ") but %s has value (%" PRIu64 ")", loc.Message().c_str(),
                                     wait_info.value, sig_loc.Message().c_str(), sig_info.value);
                }
            }
        }
    }
    for (uint32_t i = 0; i < submit.signalSemaphoreInfoCount; ++i) {
        const auto &sem_info = submit.pSignalSemaphoreInfos[i];
        auto loc = outer_loc.dot(Field::pSignalSemaphoreInfos, i);
        skip |= ValidatePipelineStage(LogObjectList(sem_info.semaphore), loc.dot(Field::stageMask), state.queue_flags,
                                      sem_info.stageMask);
        skip |= ValidateStageMaskHost(loc.dot(Field::stageMask), sem_info.stageMask);
        skip |= state.ValidateSignalSemaphore(loc, sem_info.semaphore, sem_info.value);
    }
    return skip;
}

bool CoreChecks::ValidateSemaphoresForSubmit(SemaphoreSubmitState &state, const VkBindSparseInfo &submit,
                                             const Location &outer_loc) const {
    bool skip = false;
    auto *timeline_semaphore_submit_info = LvlFindInChain<VkTimelineSemaphoreSubmitInfo>(submit.pNext);
    for (uint32_t i = 0; i < submit.waitSemaphoreCount; ++i) {
        uint64_t value = 0;
        VkSemaphore semaphore = submit.pWaitSemaphores[i];

        const LogObjectList objlist(semaphore, state.queue);
        // NOTE: there are no stage masks in bind sparse submissions
        auto semaphore_state = Get<SEMAPHORE_STATE>(semaphore);
        if (!semaphore_state) {
            continue;
        }
        auto loc = outer_loc.dot(Field::pWaitSemaphores, i);
        if (semaphore_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            if (timeline_semaphore_submit_info == nullptr) {
                skip |= LogError(semaphore, "VUID-VkBindSparseInfo-pWaitSemaphores-03246",
                                 "%s (%s) is a timeline semaphore, but VkSubmitInfo does "
                                 "not include an instance of VkTimelineSemaphoreSubmitInfo",
                                 loc.Message().c_str(), report_data->FormatHandle(semaphore).c_str());
                break;
            } else if (submit.waitSemaphoreCount != timeline_semaphore_submit_info->waitSemaphoreValueCount) {
                skip |= LogError(semaphore, "VUID-VkBindSparseInfo-pNext-03247",
                                 "%s (%s) is a timeline semaphore, it contains an "
                                 "instance of VkTimelineSemaphoreSubmitInfo, but waitSemaphoreValueCount (%u) is different than "
                                 "waitSemaphoreCount (%u)",
                                 loc.Message().c_str(), report_data->FormatHandle(semaphore).c_str(),
                                 timeline_semaphore_submit_info->waitSemaphoreValueCount, submit.waitSemaphoreCount);
                break;
            }
            value = timeline_semaphore_submit_info->pWaitSemaphoreValues[i];
        }
        skip |= state.ValidateWaitSemaphore(outer_loc.dot(Field::pWaitSemaphores, i), semaphore, value);
    }
    for (uint32_t i = 0; i < submit.signalSemaphoreCount; ++i) {
        VkSemaphore semaphore = submit.pSignalSemaphores[i];
        uint64_t value = 0;
        auto semaphore_state = Get<SEMAPHORE_STATE>(semaphore);
        if (!semaphore_state) {
            continue;
        }
        auto loc = outer_loc.dot(Field::pSignalSemaphores, i);
        if (semaphore_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            if (timeline_semaphore_submit_info == nullptr) {
                skip |= LogError(semaphore, "VUID-VkBindSparseInfo-pWaitSemaphores-03246",
                                 "%s (%s) is a timeline semaphore, but VkSubmitInfo"
                                 "does not include an instance of VkTimelineSemaphoreSubmitInfo",
                                 loc.Message().c_str(), report_data->FormatHandle(semaphore).c_str());
                break;
            } else if (submit.signalSemaphoreCount != timeline_semaphore_submit_info->signalSemaphoreValueCount) {
                skip |= LogError(semaphore, "VUID-VkBindSparseInfo-pNext-03248",
                                 "%s (%s) is a timeline semaphore, it contains an "
                                 "instance of VkTimelineSemaphoreSubmitInfo, but signalSemaphoreValueCount (%u) is different than "
                                 "signalSemaphoreCount (%u)",
                                 loc.Message().c_str(), report_data->FormatHandle(semaphore).c_str(),
                                 timeline_semaphore_submit_info->signalSemaphoreValueCount, submit.signalSemaphoreCount);
                break;
            }
            value = timeline_semaphore_submit_info->pSignalSemaphoreValues[i];
        }
        skip |= state.ValidateSignalSemaphore(loc, semaphore, value);
    }
    return skip;
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

bool CoreChecks::PreCallValidateGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex,
                                               VkQueue *pQueue) const {
    bool skip = false;

    skip |= ValidateDeviceQueueFamily(queueFamilyIndex, "vkGetDeviceQueue", "queueFamilyIndex",
                                      "VUID-vkGetDeviceQueue-queueFamilyIndex-00384");

    for (size_t i = 0; i < device_queue_info_list.size(); i++) {
        const auto device_queue_info = device_queue_info_list.at(i);
        if (device_queue_info.queue_family_index != queueFamilyIndex) {
            continue;
        }

        // flag must be zero
        if (device_queue_info.flags != 0) {
            skip |= LogError(
                device, "VUID-vkGetDeviceQueue-flags-01841",
                "vkGetDeviceQueue: queueIndex (=%" PRIu32
                ") was created with a non-zero VkDeviceQueueCreateFlags in vkCreateDevice::pCreateInfo->pQueueCreateInfos[%" PRIu32
                "]. Need to use vkGetDeviceQueue2 instead.",
                queueIndex, device_queue_info.index);
        }

        if (device_queue_info.queue_count <= queueIndex) {
            skip |= LogError(device, "VUID-vkGetDeviceQueue-queueIndex-00385",
                             "vkGetDeviceQueue: queueIndex (=%" PRIu32
                             ") is not less than the number of queues requested from queueFamilyIndex (=%" PRIu32
                             ") when the device was created vkCreateDevice::pCreateInfo->pQueueCreateInfos[%" PRIu32
                             "] (i.e. is not less than %" PRIu32 ").",
                             queueIndex, queueFamilyIndex, device_queue_info.index, device_queue_info.queue_count);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue) const {
    bool skip = false;

    if (pQueueInfo) {
        const uint32_t queueFamilyIndex = pQueueInfo->queueFamilyIndex;
        const uint32_t queueIndex = pQueueInfo->queueIndex;
        const VkDeviceQueueCreateFlags flags = pQueueInfo->flags;

        skip |= ValidateDeviceQueueFamily(queueFamilyIndex, "vkGetDeviceQueue2", "pQueueInfo->queueFamilyIndex",
                                          "VUID-VkDeviceQueueInfo2-queueFamilyIndex-01842");

        // ValidateDeviceQueueFamily() already checks if queueFamilyIndex but need to make sure flags match with it
        bool valid_flags = false;

        for (size_t i = 0; i < device_queue_info_list.size(); i++) {
            const auto device_queue_info = device_queue_info_list.at(i);
            // vkGetDeviceQueue2 only checks if both family index AND flags are same as device creation
            // this handle case where the same queueFamilyIndex is used with/without the protected flag
            if ((device_queue_info.queue_family_index != queueFamilyIndex) || (device_queue_info.flags != flags)) {
                continue;
            }
            valid_flags = true;

            if (device_queue_info.queue_count <= queueIndex) {
                skip |= LogError(
                    device, "VUID-VkDeviceQueueInfo2-queueIndex-01843",
                    "vkGetDeviceQueue2: queueIndex (=%" PRIu32
                    ") is not less than the number of queues requested from [queueFamilyIndex (=%" PRIu32
                    "), flags (%s)] combination when the device was created vkCreateDevice::pCreateInfo->pQueueCreateInfos[%" PRIu32
                    "] (i.e. is not less than %" PRIu32 ").",
                    queueIndex, queueFamilyIndex, string_VkDeviceQueueCreateFlags(flags).c_str(), device_queue_info.index,
                    device_queue_info.queue_count);
            }
        }

        // Don't double error message if already skipping from ValidateDeviceQueueFamily
        if (!valid_flags && !skip) {
            skip |= LogError(device, "VUID-VkDeviceQueueInfo2-flags-06225",
                             "vkGetDeviceQueue2: The combination of queueFamilyIndex (=%" PRIu32
                             ") and flags (%s) were never both set together in any element of "
                             "vkCreateDevice::pCreateInfo->pQueueCreateInfos at device creation time.",
                             queueFamilyIndex, string_VkDeviceQueueCreateFlags(flags).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                                const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore) const {
    bool skip = false;
    auto *sem_type_create_info = LvlFindInChain<VkSemaphoreTypeCreateInfo>(pCreateInfo->pNext);

    if (sem_type_create_info) {
        if (sem_type_create_info->semaphoreType == VK_SEMAPHORE_TYPE_TIMELINE && !enabled_features.core12.timelineSemaphore) {
            skip |= LogError(device, "VUID-VkSemaphoreTypeCreateInfo-timelineSemaphore-03252",
                             "VkCreateSemaphore: timelineSemaphore feature is not enabled, can not create timeline semaphores");
        }

        if (sem_type_create_info->semaphoreType == VK_SEMAPHORE_TYPE_BINARY && sem_type_create_info->initialValue != 0) {
            skip |= LogError(device, "VUID-VkSemaphoreTypeCreateInfo-semaphoreType-03279",
                             "vkCreateSemaphore: if semaphoreType is VK_SEMAPHORE_TYPE_BINARY, initialValue must be zero");
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout) const {
    return ValidateWaitSemaphores(device, pWaitInfo, timeout, "VkWaitSemaphores");
}

bool CoreChecks::PreCallValidateWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout) const {
    return ValidateWaitSemaphores(device, pWaitInfo, timeout, "VkWaitSemaphoresKHR");
}

bool CoreChecks::ValidateWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout,
                                        const char *apiName) const {
    bool skip = false;

    for (uint32_t i = 0; i < pWaitInfo->semaphoreCount; i++) {
        auto semaphore_state = Get<SEMAPHORE_STATE>(pWaitInfo->pSemaphores[i]);
        if (semaphore_state && semaphore_state->type != VK_SEMAPHORE_TYPE_TIMELINE) {
            skip |= LogError(pWaitInfo->pSemaphores[i], "VUID-VkSemaphoreWaitInfo-pSemaphores-03256",
                             "%s(): all semaphores in pWaitInfo must be timeline semaphores, but %s is not", apiName,
                             report_data->FormatHandle(pWaitInfo->pSemaphores[i]).c_str());
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks *pAllocator) const {
    auto fence_node = Get<FENCE_STATE>(fence);
    bool skip = false;
    if (fence_node) {
        if (fence_node->Scope() == kSyncScopeInternal && fence_node->State() == FENCE_INFLIGHT) {
            skip |= LogError(fence, "VUID-vkDestroyFence-fence-01120", "%s is in use.", report_data->FormatHandle(fence).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroySemaphore(VkDevice device, VkSemaphore semaphore,
                                                 const VkAllocationCallbacks *pAllocator) const {
    auto sema_node = Get<SEMAPHORE_STATE>(semaphore);
    bool skip = false;
    if (sema_node) {
        skip |= ValidateObjectNotInUse(sema_node.get(), "vkDestroySemaphore", "VUID-vkDestroySemaphore-semaphore-01137");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator) const {
    auto event_state = Get<EVENT_STATE>(event);
    bool skip = false;
    if (event_state) {
        skip |= ValidateObjectNotInUse(event_state.get(), "vkDestroyEvent", "VUID-vkDestroyEvent-event-01145");
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                        const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                        VkImageFormatProperties2 *pImageFormatProperties) const {
    // Can't wrap AHB-specific validation in a device extension check here, but no harm
    bool skip = ValidateGetPhysicalDeviceImageFormatProperties2ANDROID(pImageFormatInfo, pImageFormatProperties);
    return skip;
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                           const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                           VkImageFormatProperties2 *pImageFormatProperties) const {
    // Can't wrap AHB-specific validation in a device extension check here, but no harm
    bool skip = ValidateGetPhysicalDeviceImageFormatProperties2ANDROID(pImageFormatInfo, pImageFormatProperties);
    return skip;
}

bool CoreChecks::PreCallValidateDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks *pAllocator) const {
    auto sampler_state = Get<SAMPLER_STATE>(sampler);
    bool skip = false;
    if (sampler_state) {
        skip |= ValidateObjectNotInUse(sampler_state.get(), "vkDestroySampler", "VUID-vkDestroySampler-sampler-01082");
    }
    return skip;
}

bool CoreChecks::PreCallValidateDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                      const VkAllocationCallbacks *pAllocator) const {
    auto desc_pool_state = Get<DESCRIPTOR_POOL_STATE>(descriptorPool);
    bool skip = false;
    if (desc_pool_state) {
        skip |= ValidateObjectNotInUse(desc_pool_state.get(), "vkDestroyDescriptorPool",
                                       "VUID-vkDestroyDescriptorPool-descriptorPool-00303");
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

bool CoreChecks::PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkCommandPool *pCommandPool) const {
    bool skip = false;
    skip |= ValidateDeviceQueueFamily(pCreateInfo->queueFamilyIndex, "vkCreateCommandPool", "pCreateInfo->queueFamilyIndex",
                                      "VUID-vkCreateCommandPool-queueFamilyIndex-01937");
    if ((enabled_features.core11.protectedMemory == VK_FALSE) &&
        ((pCreateInfo->flags & VK_COMMAND_POOL_CREATE_PROTECTED_BIT) != 0)) {
        skip |= LogError(device, "VUID-VkCommandPoolCreateInfo-flags-02860",
                         "vkCreateCommandPool(): the protectedMemory device feature is disabled: CommandPools cannot be created "
                         "with the VK_COMMAND_POOL_CREATE_PROTECTED_BIT set.");
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                                   const VkAllocationCallbacks *pAllocator) const {
    auto cp_state = Get<COMMAND_POOL_STATE>(commandPool);
    bool skip = false;
    if (cp_state) {
        // Verify that command buffers in pool are complete (not in-flight)
        skip |=
            CheckCommandBuffersInFlight(cp_state.get(), "destroy command pool with", "VUID-vkDestroyCommandPool-commandPool-00041");
    }
    return skip;
}

bool CoreChecks::PreCallValidateResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) const {
    auto command_pool_state = Get<COMMAND_POOL_STATE>(commandPool);
    return CheckCommandBuffersInFlight(command_pool_state.get(), "reset command pool with",
                                       "VUID-vkResetCommandPool-commandPool-00040");
}

bool CoreChecks::PreCallValidateResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences) const {
    bool skip = false;
    for (uint32_t i = 0; i < fenceCount; ++i) {
        auto fence_state = Get<FENCE_STATE>(pFences[i]);
        if (fence_state && fence_state->Scope() == kSyncScopeInternal && fence_state->State() == FENCE_INFLIGHT) {
            skip |= LogError(pFences[i], "VUID-vkResetFences-pFences-01123", "%s is in use.",
                             report_data->FormatHandle(pFences[i]).c_str());
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

// Access helper functions for external modules
VkFormatProperties3KHR CoreChecks::GetPDFormatProperties(const VkFormat format) const {
    auto fmt_props_3 = LvlInitStruct<VkFormatProperties3KHR>();
    auto fmt_props_2 = LvlInitStruct<VkFormatProperties2>(&fmt_props_3);

    if (has_format_feature2) {
        DispatchGetPhysicalDeviceFormatProperties2(physical_device, format, &fmt_props_2);
    } else {
        VkFormatProperties format_properties;
        DispatchGetPhysicalDeviceFormatProperties(physical_device, format, &format_properties);
        fmt_props_3.linearTilingFeatures = format_properties.linearTilingFeatures;
        fmt_props_3.optimalTilingFeatures = format_properties.optimalTilingFeatures;
        fmt_props_3.bufferFeatures = format_properties.bufferFeatures;
    }
    return fmt_props_3;
}

enum DSL_DESCRIPTOR_GROUPS {
    DSL_TYPE_SAMPLERS = 0,
    DSL_TYPE_UNIFORM_BUFFERS,
    DSL_TYPE_STORAGE_BUFFERS,
    DSL_TYPE_SAMPLED_IMAGES,
    DSL_TYPE_STORAGE_IMAGES,
    DSL_TYPE_INPUT_ATTACHMENTS,
    DSL_TYPE_INLINE_UNIFORM_BLOCK,
    DSL_TYPE_ACCELERATION_STRUCTURE,
    DSL_TYPE_ACCELERATION_STRUCTURE_NV,
    DSL_NUM_DESCRIPTOR_GROUPS
};

// Used by PreCallValidateCreatePipelineLayout.
// Returns an array of size DSL_NUM_DESCRIPTOR_GROUPS of the maximum number of descriptors used in any single pipeline stage
std::valarray<uint32_t> GetDescriptorCountMaxPerStage(
    const DeviceFeatures *enabled_features,
    const std::vector<std::shared_ptr<cvdescriptorset::DescriptorSetLayout const>> &set_layouts, bool skip_update_after_bind) {
    // Identify active pipeline stages
    std::vector<VkShaderStageFlags> stage_flags = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                   VK_SHADER_STAGE_COMPUTE_BIT};
    if (enabled_features->core.geometryShader) {
        stage_flags.push_back(VK_SHADER_STAGE_GEOMETRY_BIT);
    }
    if (enabled_features->core.tessellationShader) {
        stage_flags.push_back(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
        stage_flags.push_back(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    }

    // Allow iteration over enum values
    std::vector<DSL_DESCRIPTOR_GROUPS> dsl_groups = {
        DSL_TYPE_SAMPLERS,
        DSL_TYPE_UNIFORM_BUFFERS,
        DSL_TYPE_STORAGE_BUFFERS,
        DSL_TYPE_SAMPLED_IMAGES,
        DSL_TYPE_STORAGE_IMAGES,
        DSL_TYPE_INPUT_ATTACHMENTS,
        DSL_TYPE_INLINE_UNIFORM_BLOCK,
        DSL_TYPE_ACCELERATION_STRUCTURE,
        DSL_TYPE_ACCELERATION_STRUCTURE_NV,
    };

    // Sum by layouts per stage, then pick max of stages per type
    std::valarray<uint32_t> max_sum(0U, DSL_NUM_DESCRIPTOR_GROUPS);  // max descriptor sum among all pipeline stages
    for (auto stage : stage_flags) {
        std::valarray<uint32_t> stage_sum(0U, DSL_NUM_DESCRIPTOR_GROUPS);  // per-stage sums
        for (const auto &dsl : set_layouts) {
            if (!dsl) {
                continue;
            }
            if (skip_update_after_bind && (dsl->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) {
                continue;
            }

            for (uint32_t binding_idx = 0; binding_idx < dsl->GetBindingCount(); binding_idx++) {
                const VkDescriptorSetLayoutBinding *binding = dsl->GetDescriptorSetLayoutBindingPtrFromIndex(binding_idx);
                // Bindings with a descriptorCount of 0 are "reserved" and should be skipped
                if (0 != (stage & binding->stageFlags) && binding->descriptorCount > 0) {
                    switch (binding->descriptorType) {
                        case VK_DESCRIPTOR_TYPE_SAMPLER:
                            stage_sum[DSL_TYPE_SAMPLERS] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                            stage_sum[DSL_TYPE_UNIFORM_BUFFERS] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                            stage_sum[DSL_TYPE_STORAGE_BUFFERS] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                            stage_sum[DSL_TYPE_SAMPLED_IMAGES] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                            stage_sum[DSL_TYPE_STORAGE_IMAGES] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                            stage_sum[DSL_TYPE_SAMPLED_IMAGES] += binding->descriptorCount;
                            stage_sum[DSL_TYPE_SAMPLERS] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                            stage_sum[DSL_TYPE_INPUT_ATTACHMENTS] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT:
                            // count one block per binding. descriptorCount is number of bytes
                            stage_sum[DSL_TYPE_INLINE_UNIFORM_BLOCK]++;
                            break;
                        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
                            stage_sum[DSL_TYPE_ACCELERATION_STRUCTURE] += binding->descriptorCount;
                            break;
                        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
                            stage_sum[DSL_TYPE_ACCELERATION_STRUCTURE_NV] += binding->descriptorCount;
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        for (auto type : dsl_groups) {
            max_sum[type] = std::max(stage_sum[type], max_sum[type]);
        }
    }
    return max_sum;
}

// Used by PreCallValidateCreatePipelineLayout.
// Returns a map indexed by VK_DESCRIPTOR_TYPE_* enum of the summed descriptors by type.
// Note: descriptors only count against the limit once even if used by multiple stages.
std::map<uint32_t, uint32_t> GetDescriptorSum(
    const std::vector<std::shared_ptr<cvdescriptorset::DescriptorSetLayout const>> &set_layouts, bool skip_update_after_bind) {
    std::map<uint32_t, uint32_t> sum_by_type;
    for (const auto &dsl : set_layouts) {
        if (!dsl) {
            continue;
        }
        if (skip_update_after_bind && (dsl->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) {
            continue;
        }

        for (uint32_t binding_idx = 0; binding_idx < dsl->GetBindingCount(); binding_idx++) {
            const VkDescriptorSetLayoutBinding *binding = dsl->GetDescriptorSetLayoutBindingPtrFromIndex(binding_idx);
            // Bindings with a descriptorCount of 0 are "reserved" and should be skipped
            if (binding->descriptorCount > 0) {
                if (binding->descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT) {
                    // count one block per binding. descriptorCount is number of bytes
                    sum_by_type[binding->descriptorType]++;
                } else {
                    sum_by_type[binding->descriptorType] += binding->descriptorCount;
                }
            }
        }
    }
    return sum_by_type;
}

bool CoreChecks::PreCallValidateCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator,
                                                     VkPipelineLayout *pPipelineLayout) const {
    bool skip = false;

    std::vector<std::shared_ptr<cvdescriptorset::DescriptorSetLayout const>> set_layouts(pCreateInfo->setLayoutCount, nullptr);
    unsigned int push_descriptor_set_count = 0;
    unsigned int descriptor_buffer_set_count = 0;
    unsigned int valid_set_count = 0;
    {
        for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; ++i) {
            set_layouts[i] = Get<cvdescriptorset::DescriptorSetLayout>(pCreateInfo->pSetLayouts[i]);
            if (set_layouts[i]) {
                if (set_layouts[i]->IsPushDescriptor()) ++push_descriptor_set_count;
                if (set_layouts[i]->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT) {
                    skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-04606",
                                     "vkCreatePipelineLayout(): pCreateInfo->pSetLayouts[%" PRIu32
                                     "] was created with VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT bit.",
                                     i);
                }
                ++valid_set_count;
                if (set_layouts[i]->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) {
                    ++descriptor_buffer_set_count;
                }
            }
        }
    }

    if ((descriptor_buffer_set_count != 0) && (valid_set_count != descriptor_buffer_set_count)) {
        skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-08008",
                         "vkCreatePipelineLayout() All sets must be created with "
                         "VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT or none of them.");
    }

    if (push_descriptor_set_count > 1) {
        skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00293",
                         "vkCreatePipelineLayout() Multiple push descriptor sets found.");
    }

    // Max descriptors by type, within a single pipeline stage
    std::valarray<uint32_t> max_descriptors_per_stage = GetDescriptorCountMaxPerStage(&enabled_features, set_layouts, true);
    // Samplers
    if (max_descriptors_per_stage[DSL_TYPE_SAMPLERS] > phys_dev_props.limits.maxPerStageDescriptorSamplers) {
        const char *vuid = IsExtEnabled(device_extensions.vk_ext_descriptor_indexing)
                               ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03016"
                               : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00287";
        skip |= LogError(device, vuid,
                         "vkCreatePipelineLayout(): max per-stage sampler bindings count (%d) exceeds device "
                         "maxPerStageDescriptorSamplers limit (%d).",
                         max_descriptors_per_stage[DSL_TYPE_SAMPLERS], phys_dev_props.limits.maxPerStageDescriptorSamplers);
    }

    // Uniform buffers
    if (max_descriptors_per_stage[DSL_TYPE_UNIFORM_BUFFERS] > phys_dev_props.limits.maxPerStageDescriptorUniformBuffers) {
        const char *vuid = IsExtEnabled(device_extensions.vk_ext_descriptor_indexing)
                               ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03017"
                               : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00288";
        skip |= LogError(device, vuid,
                         "vkCreatePipelineLayout(): max per-stage uniform buffer bindings count (%d) exceeds device "
                         "maxPerStageDescriptorUniformBuffers limit (%d).",
                         max_descriptors_per_stage[DSL_TYPE_UNIFORM_BUFFERS],
                         phys_dev_props.limits.maxPerStageDescriptorUniformBuffers);
    }

    // Storage buffers
    if (max_descriptors_per_stage[DSL_TYPE_STORAGE_BUFFERS] > phys_dev_props.limits.maxPerStageDescriptorStorageBuffers) {
        const char *vuid = IsExtEnabled(device_extensions.vk_ext_descriptor_indexing)
                               ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03018"
                               : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00289";
        skip |= LogError(device, vuid,
                         "vkCreatePipelineLayout(): max per-stage storage buffer bindings count (%d) exceeds device "
                         "maxPerStageDescriptorStorageBuffers limit (%d).",
                         max_descriptors_per_stage[DSL_TYPE_STORAGE_BUFFERS],
                         phys_dev_props.limits.maxPerStageDescriptorStorageBuffers);
    }

    // Sampled images
    if (max_descriptors_per_stage[DSL_TYPE_SAMPLED_IMAGES] > phys_dev_props.limits.maxPerStageDescriptorSampledImages) {
        const char *vuid = IsExtEnabled(device_extensions.vk_ext_descriptor_indexing)
                               ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03019"
                               : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00290";
        skip |=
            LogError(device, vuid,
                     "vkCreatePipelineLayout(): max per-stage sampled image bindings count (%d) exceeds device "
                     "maxPerStageDescriptorSampledImages limit (%d).",
                     max_descriptors_per_stage[DSL_TYPE_SAMPLED_IMAGES], phys_dev_props.limits.maxPerStageDescriptorSampledImages);
    }

    // Storage images
    if (max_descriptors_per_stage[DSL_TYPE_STORAGE_IMAGES] > phys_dev_props.limits.maxPerStageDescriptorStorageImages) {
        const char *vuid = IsExtEnabled(device_extensions.vk_ext_descriptor_indexing)
                               ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03020"
                               : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-00291";
        skip |=
            LogError(device, vuid,
                     "vkCreatePipelineLayout(): max per-stage storage image bindings count (%d) exceeds device "
                     "maxPerStageDescriptorStorageImages limit (%d).",
                     max_descriptors_per_stage[DSL_TYPE_STORAGE_IMAGES], phys_dev_props.limits.maxPerStageDescriptorStorageImages);
    }

    // Input attachments
    if (max_descriptors_per_stage[DSL_TYPE_INPUT_ATTACHMENTS] > phys_dev_props.limits.maxPerStageDescriptorInputAttachments) {
        const char *vuid = IsExtEnabled(device_extensions.vk_ext_descriptor_indexing)
                               ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03021"
                               : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01676";
        skip |= LogError(device, vuid,
                         "vkCreatePipelineLayout(): max per-stage input attachment bindings count (%d) exceeds device "
                         "maxPerStageDescriptorInputAttachments limit (%d).",
                         max_descriptors_per_stage[DSL_TYPE_INPUT_ATTACHMENTS],
                         phys_dev_props.limits.maxPerStageDescriptorInputAttachments);
    }

    // Inline uniform blocks
    if (max_descriptors_per_stage[DSL_TYPE_INLINE_UNIFORM_BLOCK] >
        phys_dev_ext_props.inline_uniform_block_props.maxPerStageDescriptorInlineUniformBlocks) {
        const char *vuid = IsExtEnabled(device_extensions.vk_ext_descriptor_indexing)
                               ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-02214"
                               : "VUID-VkPipelineLayoutCreateInfo-descriptorType-02212";
        skip |= LogError(device, vuid,
                         "vkCreatePipelineLayout(): max per-stage inline uniform block bindings count (%d) exceeds device "
                         "maxPerStageDescriptorInlineUniformBlocks limit (%d).",
                         max_descriptors_per_stage[DSL_TYPE_INLINE_UNIFORM_BLOCK],
                         phys_dev_ext_props.inline_uniform_block_props.maxPerStageDescriptorInlineUniformBlocks);
    }
    if (max_descriptors_per_stage[DSL_TYPE_ACCELERATION_STRUCTURE] >
        phys_dev_ext_props.acc_structure_props.maxPerStageDescriptorUpdateAfterBindAccelerationStructures) {
        skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03572",
                         "vkCreatePipelineLayout(): max per-stage acceleration structure bindings count (%" PRIu32
                         ") exceeds device "
                         "maxPerStageDescriptorInlineUniformBlocks limit (%" PRIu32 ").",
                         max_descriptors_per_stage[DSL_TYPE_ACCELERATION_STRUCTURE],
                         phys_dev_ext_props.inline_uniform_block_props.maxPerStageDescriptorInlineUniformBlocks);
    }

    // Total descriptors by type
    //
    std::map<uint32_t, uint32_t> sum_all_stages = GetDescriptorSum(set_layouts, true);
    // Samplers
    uint32_t sum = sum_all_stages[VK_DESCRIPTOR_TYPE_SAMPLER] + sum_all_stages[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER];
    if (sum > phys_dev_props.limits.maxDescriptorSetSamplers) {
        const char *vuid = IsExtEnabled(device_extensions.vk_ext_descriptor_indexing)
                               ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03028"
                               : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01677";
        skip |= LogError(device, vuid,
                         "vkCreatePipelineLayout(): sum of sampler bindings among all stages (%d) exceeds device "
                         "maxDescriptorSetSamplers limit (%d).",
                         sum, phys_dev_props.limits.maxDescriptorSetSamplers);
    }

    // Uniform buffers
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] > phys_dev_props.limits.maxDescriptorSetUniformBuffers) {
        const char *vuid = IsExtEnabled(device_extensions.vk_ext_descriptor_indexing)
                               ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03029"
                               : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01678";
        skip |= LogError(device, vuid,
                         "vkCreatePipelineLayout(): sum of uniform buffer bindings among all stages (%d) exceeds device "
                         "maxDescriptorSetUniformBuffers limit (%d).",
                         sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER], phys_dev_props.limits.maxDescriptorSetUniformBuffers);
    }

    // Dynamic uniform buffers
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] > phys_dev_props.limits.maxDescriptorSetUniformBuffersDynamic) {
        const char *vuid = IsExtEnabled(device_extensions.vk_ext_descriptor_indexing)
                               ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03030"
                               : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01679";
        skip |= LogError(device, vuid,
                         "vkCreatePipelineLayout(): sum of dynamic uniform buffer bindings among all stages (%d) exceeds device "
                         "maxDescriptorSetUniformBuffersDynamic limit (%d).",
                         sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC],
                         phys_dev_props.limits.maxDescriptorSetUniformBuffersDynamic);
    }

    // Storage buffers
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER] > phys_dev_props.limits.maxDescriptorSetStorageBuffers) {
        const char *vuid = IsExtEnabled(device_extensions.vk_ext_descriptor_indexing)
                               ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03031"
                               : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01680";
        skip |= LogError(device, vuid,
                         "vkCreatePipelineLayout(): sum of storage buffer bindings among all stages (%d) exceeds device "
                         "maxDescriptorSetStorageBuffers limit (%d).",
                         sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER], phys_dev_props.limits.maxDescriptorSetStorageBuffers);
    }

    // Dynamic storage buffers
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC] > phys_dev_props.limits.maxDescriptorSetStorageBuffersDynamic) {
        const char *vuid = IsExtEnabled(device_extensions.vk_ext_descriptor_indexing)
                               ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03032"
                               : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01681";
        skip |= LogError(device, vuid,
                         "vkCreatePipelineLayout(): sum of dynamic storage buffer bindings among all stages (%d) exceeds device "
                         "maxDescriptorSetStorageBuffersDynamic limit (%d).",
                         sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC],
                         phys_dev_props.limits.maxDescriptorSetStorageBuffersDynamic);
    }

    //  Sampled images
    sum = sum_all_stages[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE] + sum_all_stages[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] +
          sum_all_stages[VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER];
    if (sum > phys_dev_props.limits.maxDescriptorSetSampledImages) {
        const char *vuid = IsExtEnabled(device_extensions.vk_ext_descriptor_indexing)
                               ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03033"
                               : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01682";
        skip |= LogError(device, vuid,
                         "vkCreatePipelineLayout(): sum of sampled image bindings among all stages (%d) exceeds device "
                         "maxDescriptorSetSampledImages limit (%d).",
                         sum, phys_dev_props.limits.maxDescriptorSetSampledImages);
    }

    //  Storage images
    sum = sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_IMAGE] + sum_all_stages[VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER];
    if (sum > phys_dev_props.limits.maxDescriptorSetStorageImages) {
        const char *vuid = IsExtEnabled(device_extensions.vk_ext_descriptor_indexing)
                               ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03034"
                               : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01683";
        skip |= LogError(device, vuid,
                         "vkCreatePipelineLayout(): sum of storage image bindings among all stages (%d) exceeds device "
                         "maxDescriptorSetStorageImages limit (%d).",
                         sum, phys_dev_props.limits.maxDescriptorSetStorageImages);
    }

    // Input attachments
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT] > phys_dev_props.limits.maxDescriptorSetInputAttachments) {
        const char *vuid = IsExtEnabled(device_extensions.vk_ext_descriptor_indexing)
                               ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-03035"
                               : "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-01684";
        skip |=
            LogError(device, vuid,
                     "vkCreatePipelineLayout(): sum of input attachment bindings among all stages (%d) exceeds device "
                     "maxDescriptorSetInputAttachments limit (%d).",
                     sum_all_stages[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT], phys_dev_props.limits.maxDescriptorSetInputAttachments);
    }

    // Inline uniform blocks
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT] >
        phys_dev_ext_props.inline_uniform_block_props.maxDescriptorSetInlineUniformBlocks) {
        const char *vuid = IsExtEnabled(device_extensions.vk_ext_descriptor_indexing)
                               ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-02216"
                               : "VUID-VkPipelineLayoutCreateInfo-descriptorType-02213";
        skip |= LogError(device, vuid,
                         "vkCreatePipelineLayout(): sum of inline uniform block bindings among all stages (%d) exceeds device "
                         "maxDescriptorSetInlineUniformBlocks limit (%d).",
                         sum_all_stages[VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT],
                         phys_dev_ext_props.inline_uniform_block_props.maxDescriptorSetInlineUniformBlocks);
    }

    // Acceleration structures NV
    if (sum_all_stages[VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV] >
        phys_dev_ext_props.ray_tracing_propsNV.maxDescriptorSetAccelerationStructures) {
        skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-descriptorType-02381",
                         "vkCreatePipelineLayout(): sum of acceleration structures NV bindings among all stages (%" PRIu32
                         ") exceeds device "
                         "VkPhysicalDeviceRayTracingPropertiesNV::maxDescriptorSetAccelerationStructures limit (%" PRIu32 ").",
                         sum_all_stages[VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV],
                         phys_dev_ext_props.ray_tracing_propsNV.maxDescriptorSetAccelerationStructures);
    }

    if (IsExtEnabled(device_extensions.vk_ext_descriptor_indexing)) {
        // XXX TODO: replace with correct VU messages

        // Max descriptors by type, within a single pipeline stage
        std::valarray<uint32_t> max_descriptors_per_stage_update_after_bind =
            GetDescriptorCountMaxPerStage(&enabled_features, set_layouts, false);
        // Samplers
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_SAMPLERS] >
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindSamplers) {
            skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03022",
                             "vkCreatePipelineLayout(): max per-stage sampler bindings count (%d) exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindSamplers limit (%d).",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_SAMPLERS],
                             phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindSamplers);
        }

        // Uniform buffers
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_UNIFORM_BUFFERS] >
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindUniformBuffers) {
            skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03023",
                             "vkCreatePipelineLayout(): max per-stage uniform buffer bindings count (%d) exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindUniformBuffers limit (%d).",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_UNIFORM_BUFFERS],
                             phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindUniformBuffers);
        }

        // Storage buffers
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_STORAGE_BUFFERS] >
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindStorageBuffers) {
            skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03024",
                             "vkCreatePipelineLayout(): max per-stage storage buffer bindings count (%d) exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindStorageBuffers limit (%d).",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_STORAGE_BUFFERS],
                             phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindStorageBuffers);
        }

        // Sampled images
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_SAMPLED_IMAGES] >
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindSampledImages) {
            skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03025",
                             "vkCreatePipelineLayout(): max per-stage sampled image bindings count (%d) exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindSampledImages limit (%d).",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_SAMPLED_IMAGES],
                             phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindSampledImages);
        }

        // Storage images
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_STORAGE_IMAGES] >
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindStorageImages) {
            skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03026",
                             "vkCreatePipelineLayout(): max per-stage storage image bindings count (%d) exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindStorageImages limit (%d).",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_STORAGE_IMAGES],
                             phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindStorageImages);
        }

        // Input attachments
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_INPUT_ATTACHMENTS] >
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindInputAttachments) {
            skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03027",
                             "vkCreatePipelineLayout(): max per-stage input attachment bindings count (%d) exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindInputAttachments limit (%d).",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_INPUT_ATTACHMENTS],
                             phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindInputAttachments);
        }

        // Inline uniform blocks
        if (max_descriptors_per_stage_update_after_bind[DSL_TYPE_INLINE_UNIFORM_BLOCK] >
            phys_dev_ext_props.inline_uniform_block_props.maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks) {
            skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-descriptorType-02215",
                             "vkCreatePipelineLayout(): max per-stage inline uniform block bindings count (%d) exceeds device "
                             "maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks limit (%d).",
                             max_descriptors_per_stage_update_after_bind[DSL_TYPE_INLINE_UNIFORM_BLOCK],
                             phys_dev_ext_props.inline_uniform_block_props.maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks);
        }

        // Total descriptors by type, summed across all pipeline stages
        //
        std::map<uint32_t, uint32_t> sum_all_stages_update_after_bind = GetDescriptorSum(set_layouts, false);
        // Samplers
        sum = sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_SAMPLER] +
              sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER];
        if (sum > phys_dev_props_core12.maxDescriptorSetUpdateAfterBindSamplers) {
            skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03036",
                             "vkCreatePipelineLayout(): sum of sampler bindings among all stages (%d) exceeds device "
                             "maxDescriptorSetUpdateAfterBindSamplers limit (%d).",
                             sum, phys_dev_props_core12.maxDescriptorSetUpdateAfterBindSamplers);
        }

        // Uniform buffers
        if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] >
            phys_dev_props_core12.maxDescriptorSetUpdateAfterBindUniformBuffers) {
            skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03037",
                             "vkCreatePipelineLayout(): sum of uniform buffer bindings among all stages (%d) exceeds device "
                             "maxDescriptorSetUpdateAfterBindUniformBuffers limit (%d).",
                             sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER],
                             phys_dev_props_core12.maxDescriptorSetUpdateAfterBindUniformBuffers);
        }

        // Dynamic uniform buffers
        if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] >
            phys_dev_props_core12.maxDescriptorSetUpdateAfterBindUniformBuffersDynamic) {
            skip |=
                LogError(device, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03038",
                         "vkCreatePipelineLayout(): sum of dynamic uniform buffer bindings among all stages (%d) exceeds device "
                         "maxDescriptorSetUpdateAfterBindUniformBuffersDynamic limit (%d).",
                         sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC],
                         phys_dev_props_core12.maxDescriptorSetUpdateAfterBindUniformBuffersDynamic);
        }

        // Storage buffers
        if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER] >
            phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageBuffers) {
            skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03039",
                             "vkCreatePipelineLayout(): sum of storage buffer bindings among all stages (%d) exceeds device "
                             "maxDescriptorSetUpdateAfterBindStorageBuffers limit (%d).",
                             sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER],
                             phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageBuffers);
        }

        // Dynamic storage buffers
        if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC] >
            phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageBuffersDynamic) {
            skip |=
                LogError(device, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03040",
                         "vkCreatePipelineLayout(): sum of dynamic storage buffer bindings among all stages (%d) exceeds device "
                         "maxDescriptorSetUpdateAfterBindStorageBuffersDynamic limit (%d).",
                         sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC],
                         phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageBuffersDynamic);
        }

        //  Sampled images
        sum = sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE] +
              sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] +
              sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER];
        if (sum > phys_dev_props_core12.maxDescriptorSetUpdateAfterBindSampledImages) {
            skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03041",
                             "vkCreatePipelineLayout(): sum of sampled image bindings among all stages (%d) exceeds device "
                             "maxDescriptorSetUpdateAfterBindSampledImages limit (%d).",
                             sum, phys_dev_props_core12.maxDescriptorSetUpdateAfterBindSampledImages);
        }

        //  Storage images
        sum = sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_IMAGE] +
              sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER];
        if (sum > phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageImages) {
            skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03042",
                             "vkCreatePipelineLayout(): sum of storage image bindings among all stages (%d) exceeds device "
                             "maxDescriptorSetUpdateAfterBindStorageImages limit (%d).",
                             sum, phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageImages);
        }

        // Input attachments
        if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT] >
            phys_dev_props_core12.maxDescriptorSetUpdateAfterBindInputAttachments) {
            skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-03043",
                             "vkCreatePipelineLayout(): sum of input attachment bindings among all stages (%d) exceeds device "
                             "maxDescriptorSetUpdateAfterBindInputAttachments limit (%d).",
                             sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT],
                             phys_dev_props_core12.maxDescriptorSetUpdateAfterBindInputAttachments);
        }

        // Inline uniform blocks
        if (sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT] >
            phys_dev_ext_props.inline_uniform_block_props.maxDescriptorSetUpdateAfterBindInlineUniformBlocks) {
            skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-descriptorType-02217",
                             "vkCreatePipelineLayout(): sum of inline uniform block bindings among all stages (%d) exceeds device "
                             "maxDescriptorSetUpdateAfterBindInlineUniformBlocks limit (%d).",
                             sum_all_stages_update_after_bind[VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT],
                             phys_dev_ext_props.inline_uniform_block_props.maxDescriptorSetUpdateAfterBindInlineUniformBlocks);
        }
    }

    if (IsExtEnabled(device_extensions.vk_ext_fragment_density_map2)) {
        uint32_t sum_subsampled_samplers = 0;
        for (const auto &dsl : set_layouts) {
            // find the number of subsampled samplers across all stages
            // NOTE: this does not use the GetDescriptorSum patter because it needs the Get<SAMPLER_STATE> method
            if ((dsl->GetCreateFlags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)) {
                continue;
            }
            for (uint32_t binding_idx = 0; binding_idx < dsl->GetBindingCount(); binding_idx++) {
                const VkDescriptorSetLayoutBinding *binding = dsl->GetDescriptorSetLayoutBindingPtrFromIndex(binding_idx);

                // Bindings with a descriptorCount of 0 are "reserved" and should be skipped
                if (binding->descriptorCount > 0) {
                    if (((binding->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) ||
                         (binding->descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER)) &&
                        (binding->pImmutableSamplers != nullptr)) {
                        for (uint32_t sampler_idx = 0; sampler_idx < binding->descriptorCount; sampler_idx++) {
                            auto state = Get<SAMPLER_STATE>(binding->pImmutableSamplers[sampler_idx]);
                            if (state && (state->createInfo.flags & (VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT |
                                                                     VK_SAMPLER_CREATE_SUBSAMPLED_COARSE_RECONSTRUCTION_BIT_EXT))) {
                                sum_subsampled_samplers++;
                            }
                        }
                    }
                }
            }
        }
        if (sum_subsampled_samplers > phys_dev_ext_props.fragment_density_map2_props.maxDescriptorSetSubsampledSamplers) {
            skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-pImmutableSamplers-03566",
                             "vkCreatePipelineLayout(): sum of sampler bindings with flags containing "
                             "VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT or "
                             "VK_SAMPLER_CREATE_SUBSAMPLED_COARSE_RECONSTRUCTION_BIT_EXT among all stages(% d) "
                             "exceeds device maxDescriptorSetSubsampledSamplers limit (%d).",
                             sum_subsampled_samplers,
                             phys_dev_ext_props.fragment_density_map2_props.maxDescriptorSetSubsampledSamplers);
        }
    }

    if (!enabled_features.graphics_pipeline_library_features.graphicsPipelineLibrary) {
        for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; ++i) {
            if (!pCreateInfo->pSetLayouts[i]) {
                skip |= LogError(device, "VUID-VkPipelineLayoutCreateInfo-graphicsPipelineLibrary-06753",
                                 "vkCreatePipelineLayout(): pSetLayouts[%" PRIu32
                                 "] is VK_NULL_HANDLE, but graphicsPipelineLibrary is not enabled.",
                                 i);
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                    VkDescriptorPoolResetFlags flags) const {
    // Make sure sets being destroyed are not currently in-use
    if (disabled[object_in_use]) return false;
    bool skip = false;
    auto pool = Get<DESCRIPTOR_POOL_STATE>(descriptorPool);
    if (pool && pool->InUse()) {
        skip |= LogError(descriptorPool, "VUID-vkResetDescriptorPool-descriptorPool-00313",
                         "It is invalid to call vkResetDescriptorPool() with descriptor sets in use by a command buffer.");
    }
    return skip;
}

// Ensure the pool contains enough descriptors and descriptor sets to satisfy
// an allocation request. Fills common_data with the total number of descriptors of each type required,
// as well as DescriptorSetLayout ptrs used for later update.
bool CoreChecks::PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                       VkDescriptorSet *pDescriptorSets, void *ads_state_data) const {
    StateTracker::PreCallValidateAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, ads_state_data);

    cvdescriptorset::AllocateDescriptorSetsData *ads_state =
        reinterpret_cast<cvdescriptorset::AllocateDescriptorSetsData *>(ads_state_data);
    // All state checks for AllocateDescriptorSets is done in single function
    return ValidateAllocateDescriptorSets(pAllocateInfo, ads_state);
}

bool CoreChecks::PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count,
                                                   const VkDescriptorSet *pDescriptorSets) const {
    // Make sure that no sets being destroyed are in-flight
    bool skip = false;
    // First make sure sets being destroyed are not currently in-use
    for (uint32_t i = 0; i < count; ++i) {
        if (pDescriptorSets[i] != VK_NULL_HANDLE) {
            skip |= ValidateIdleDescriptorSet(pDescriptorSets[i], "vkFreeDescriptorSets");
        }
    }
    auto pool_state = Get<DESCRIPTOR_POOL_STATE>(descriptorPool);
    if (pool_state && !(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT & pool_state->createInfo.flags)) {
        // Can't Free from a NON_FREE pool
        skip |= LogError(descriptorPool, "VUID-vkFreeDescriptorSets-descriptorPool-00312",
                         "It is invalid to call vkFreeDescriptorSets() with a pool created without setting "
                         "VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                     const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount,
                                                     const VkCopyDescriptorSet *pDescriptorCopies) const {
    // First thing to do is perform map look-ups.
    // NOTE : UpdateDescriptorSets is somewhat unique in that it's operating on a number of DescriptorSets
    //  so we can't just do a single map look-up up-front, but do them individually in functions below

    // Now make call(s) that validate state, but don't perform state updates in this function
    // Note, here DescriptorSets is unique in that we don't yet have an instance. Using a helper function in the
    //  namespace which will parse params and make calls into specific class instances
    return ValidateUpdateDescriptorSets(descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies,
                                        "vkUpdateDescriptorSets()");
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

bool CoreChecks::ForbidInheritedViewportScissor(VkCommandBuffer commandBuffer, const CMD_BUFFER_STATE *cb_state, const char *vuid,
                                                const CMD_TYPE cmd_type) const {
    bool skip = false;
    if (cb_state->inheritedViewportDepths.size() != 0) {
        skip |=
            LogError(commandBuffer, vuid,
                     "%s: commandBuffer must not have VkCommandBufferInheritanceViewportScissorInfoNV::viewportScissor2D enabled.",
                     CommandTypeString(cmd_type));
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
                                               const VkViewport *pViewports) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETVIEWPORT, VK_TRUE, nullptr, nullptr);
    skip |=
        ForbidInheritedViewportScissor(commandBuffer, cb_state.get(), "VUID-vkCmdSetViewport-commandBuffer-04821", CMD_SETVIEWPORT);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                              const VkRect2D *pScissors) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETSCISSOR, VK_TRUE, nullptr, nullptr);
    skip |= ForbidInheritedViewportScissor(commandBuffer, cb_state.get(), "VUID-vkCmdSetScissor-viewportScissor2D-04789",
                                           CMD_SETSCISSOR);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                         uint32_t exclusiveScissorCount, const VkRect2D *pExclusiveScissors) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETEXCLUSIVESCISSORNV,
                                        enabled_features.exclusive_scissor_features.exclusiveScissor,
                                        "VUID-vkCmdSetExclusiveScissorNV-None-02031", "exclusiveScissor");
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

bool CoreChecks::PreCallValidateCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                   uint32_t viewportCount,
                                                                   const VkShadingRatePaletteNV *pShadingRatePalettes) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;

    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETVIEWPORTSHADINGRATEPALETTENV,
                                         enabled_features.shading_rate_image_features.shadingRateImage,
                                         "VUID-vkCmdSetViewportShadingRatePaletteNV-None-02064", "shadingRateImage");

    for (uint32_t i = 0; i < viewportCount; ++i) {
        auto *palette = &pShadingRatePalettes[i];
        if (palette->shadingRatePaletteEntryCount == 0 ||
            palette->shadingRatePaletteEntryCount > phys_dev_ext_props.shading_rate_image_props.shadingRatePaletteSize) {
            skip |= LogError(
                commandBuffer, "VUID-VkShadingRatePaletteNV-shadingRatePaletteEntryCount-02071",
                "vkCmdSetViewportShadingRatePaletteNV: shadingRatePaletteEntryCount must be between 1 and shadingRatePaletteSize.");
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                         uint32_t viewportCount,
                                                         const VkViewportWScalingNV *pViewportWScalings) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETVIEWPORTWSCALINGNV, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETLINEWIDTH, VK_TRUE, nullptr, nullptr);
    ;
}

bool CoreChecks::PreCallValidateCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                     uint16_t lineStipplePattern) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETLINESTIPPLEEXT, VK_TRUE, nullptr, nullptr);
    ;
}

bool CoreChecks::PreCallValidateCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
                                                float depthBiasSlopeFactor) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHBIAS, VK_TRUE, nullptr, nullptr);
    if ((depthBiasClamp != 0.0) && (!enabled_features.core.depthBiasClamp)) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetDepthBias-depthBiasClamp-00790",
                         "vkCmdSetDepthBias(): the depthBiasClamp device feature is disabled: the depthBiasClamp parameter must "
                         "be set to 0.0.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETBLENDCONSTANTS, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHBOUNDS, VK_TRUE, nullptr, nullptr);

    // The extension was not created with a feature bit whichs prevents displaying the 2 variations of the VUIDs
    if (!IsExtEnabled(device_extensions.vk_ext_depth_range_unrestricted)) {
        if (!(minDepthBounds >= 0.0) || !(minDepthBounds <= 1.0)) {
            // Also VUID-vkCmdSetDepthBounds-minDepthBounds-00600
            skip |= LogError(commandBuffer, "VUID-vkCmdSetDepthBounds-minDepthBounds-02508",
                             "vkCmdSetDepthBounds(): VK_EXT_depth_range_unrestricted extension is not enabled and minDepthBounds "
                             "(=%f) is not within the [0.0, 1.0] range.",
                             minDepthBounds);
        }

        if (!(maxDepthBounds >= 0.0) || !(maxDepthBounds <= 1.0)) {
            // Also VUID-vkCmdSetDepthBounds-maxDepthBounds-00601
            skip |= LogError(commandBuffer, "VUID-vkCmdSetDepthBounds-maxDepthBounds-02509",
                             "vkCmdSetDepthBounds(): VK_EXT_depth_range_unrestricted extension is not enabled and maxDepthBounds "
                             "(=%f) is not within the [0.0, 1.0] range.",
                             maxDepthBounds);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                         uint32_t compareMask) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETSTENCILCOMPAREMASK, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                       uint32_t writeMask) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETSTENCILWRITEMASK, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                       uint32_t reference) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETSTENCILREFERENCE, VK_TRUE, nullptr, nullptr);
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

bool CoreChecks::PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                        VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                        const VkWriteDescriptorSet *pDescriptorWrites) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    const char *func_name = "vkCmdPushDescriptorSetKHR()";
    bool skip = false;
    skip |= ValidateCmd(*cb_state, CMD_PUSHDESCRIPTORSETKHR);

    static const std::map<VkPipelineBindPoint, std::string> bind_errors = {
        std::make_pair(VK_PIPELINE_BIND_POINT_GRAPHICS, "VUID-vkCmdPushDescriptorSetKHR-pipelineBindPoint-00363"),
        std::make_pair(VK_PIPELINE_BIND_POINT_COMPUTE, "VUID-vkCmdPushDescriptorSetKHR-pipelineBindPoint-00363"),
        std::make_pair(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, "VUID-vkCmdPushDescriptorSetKHR-pipelineBindPoint-00363")};

    skip |= ValidatePipelineBindPoint(cb_state.get(), pipelineBindPoint, func_name, bind_errors);
    auto layout_data = Get<PIPELINE_LAYOUT_STATE>(layout);

    // Validate the set index points to a push descriptor set and is in range
    if (layout_data) {
        const auto &set_layouts = layout_data->set_layouts;
        if (set < set_layouts.size()) {
            const auto &dsl = set_layouts[set];
            if (dsl) {
                if (!dsl->IsPushDescriptor()) {
                    skip = LogError(layout, "VUID-vkCmdPushDescriptorSetKHR-set-00365",
                                    "%s: Set index %" PRIu32 " does not match push descriptor set layout index for %s.", func_name,
                                    set, report_data->FormatHandle(layout).c_str());
                } else {
                    // Create an empty proxy in order to use the existing descriptor set update validation
                    // TODO move the validation (like this) that doesn't need descriptor set state to the DSL object so we
                    // don't have to do this.
                    cvdescriptorset::DescriptorSet proxy_ds(VK_NULL_HANDLE, nullptr, dsl, 0, this);
                    skip |= ValidatePushDescriptorsUpdate(&proxy_ds, descriptorWriteCount, pDescriptorWrites, func_name);
                }
            }
        } else {
            skip = LogError(layout, "VUID-vkCmdPushDescriptorSetKHR-set-00364",
                            "%s: Set index %" PRIu32 " is outside of range for %s (set < %" PRIu32 ").", func_name, set,
                            report_data->FormatHandle(layout).c_str(), static_cast<uint32_t>(set_layouts.size()));
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

    bool skip = ValidateBufferUsageFlags(buffer_state.get(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, true,
                                         "VUID-vkCmdBindIndexBuffer-buffer-00433", "vkCmdBindIndexBuffer()",
                                         "VK_BUFFER_USAGE_INDEX_BUFFER_BIT");
    skip |= ValidateCmd(*cb_state_ptr, CMD_BINDINDEXBUFFER);
    skip |= ValidateMemoryIsBoundToBuffer(buffer_state.get(), "vkCmdBindIndexBuffer()", "VUID-vkCmdBindIndexBuffer-buffer-00434");
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
            skip |= ValidateBufferUsageFlags(buffer_state.get(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, true,
                                             "VUID-vkCmdBindVertexBuffers-pBuffers-00627", "vkCmdBindVertexBuffers()",
                                             "VK_BUFFER_USAGE_VERTEX_BUFFER_BIT");
            skip |= ValidateMemoryIsBoundToBuffer(buffer_state.get(), "vkCmdBindVertexBuffers()",
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

// Validate that an image's sampleCount matches the requirement for a specific API call
bool CoreChecks::ValidateImageSampleCount(const IMAGE_STATE *image_state, VkSampleCountFlagBits sample_count, const char *location,
                                          const std::string &msgCode) const {
    bool skip = false;
    if (image_state->createInfo.samples != sample_count) {
        skip = LogError(image_state->image(), msgCode, "%s for %s was created with a sample count of %s but must be %s.", location,
                        report_data->FormatHandle(image_state->image()).c_str(),
                        string_VkSampleCountFlagBits(image_state->createInfo.samples), string_VkSampleCountFlagBits(sample_count));
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

    skip |= ValidateMemoryIsBoundToBuffer(dst_buffer_state.get(), "vkCmdUpdateBuffer()", "VUID-vkCmdUpdateBuffer-dstBuffer-00035");
    // Validate that DST buffer has correct usage flags set
    skip |= ValidateBufferUsageFlags(dst_buffer_state.get(), VK_BUFFER_USAGE_TRANSFER_DST_BIT, true,
                                     "VUID-vkCmdUpdateBuffer-dstBuffer-00034", "vkCmdUpdateBuffer()",
                                     "VK_BUFFER_USAGE_TRANSFER_DST_BIT");
    skip |= ValidateCmd(cb_state, CMD_UPDATEBUFFER);
    skip |= ValidateProtectedBuffer(cb_state, dst_buffer_state.get(), "vkCmdUpdateBuffer()",
                                    "VUID-vkCmdUpdateBuffer-commandBuffer-01813");
    skip |= ValidateUnprotectedBuffer(cb_state, dst_buffer_state.get(), "vkCmdUpdateBuffer()",
                                      "VUID-vkCmdUpdateBuffer-commandBuffer-01814");
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

bool CoreChecks::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETEVENT, VK_TRUE, nullptr, nullptr);
    Location loc(Func::vkCmdSetEvent, Field::stageMask);
    const LogObjectList objlist(commandBuffer);
    skip |= ValidatePipelineStage(objlist, loc, cb_state->GetQueueFlags(), stageMask);
    skip |= ValidateStageMaskHost(loc, stageMask);
    return skip;
}

bool CoreChecks::ValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event, const VkDependencyInfo *pDependencyInfo,
                                      CMD_TYPE cmd_type) const {
    const LogObjectList objlist(commandBuffer, event);

    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, cmd_type, enabled_features.core13.synchronization2,
                                         "VUID-vkCmdSetEvent2-synchronization2-03824", "synchronization2");
    Location loc(Func::vkCmdSetEvent2, Field::pDependencyInfo);
    if (pDependencyInfo->dependencyFlags != 0) {
        skip |= LogError(objlist, "VUID-vkCmdSetEvent2-dependencyFlags-03825", "%s (%s) must be 0",
                         loc.dot(Field::dependencyFlags).Message().c_str(),
                         string_VkDependencyFlags(pDependencyInfo->dependencyFlags).c_str());
    }
    skip |= ValidateDependencyInfo(objlist, loc, cb_state.get(), pDependencyInfo);
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                const VkDependencyInfoKHR *pDependencyInfo) const {
    return ValidateCmdSetEvent2(commandBuffer, event, pDependencyInfo, CMD_SETEVENT2KHR);
}

bool CoreChecks::PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                             const VkDependencyInfo *pDependencyInfo) const {
    return ValidateCmdSetEvent2(commandBuffer, event, pDependencyInfo, CMD_SETEVENT2);
}

bool CoreChecks::PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    const LogObjectList objlist(commandBuffer);
    Location loc(Func::vkCmdResetEvent, Field::stageMask);

    bool skip = false;
    skip |= ValidateCmd(*cb_state, CMD_RESETEVENT);
    skip |= ValidatePipelineStage(objlist, loc, cb_state->GetQueueFlags(), stageMask);
    skip |= ValidateStageMaskHost(loc, stageMask);
    return skip;
}

bool CoreChecks::ValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                        CMD_TYPE cmd_type) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    const LogObjectList objlist(commandBuffer);
    Location loc(Func::vkCmdResetEvent2, Field::stageMask);

    bool skip = false;
    if (!enabled_features.core13.synchronization2) {
        skip |= LogError(commandBuffer, "VUID-vkCmdResetEvent2-synchronization2-03829",
                         "vkCmdResetEvent2KHR(): Synchronization2 feature is not enabled");
    }
    skip |= ValidateCmd(*cb_state, cmd_type);
    skip |= ValidatePipelineStage(objlist, loc, cb_state->GetQueueFlags(), stageMask);
    skip |= ValidateStageMaskHost(loc, stageMask);
    return skip;
}

bool CoreChecks::PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                  VkPipelineStageFlags2KHR stageMask) const {
    return ValidateCmdResetEvent2(commandBuffer, event, stageMask, CMD_RESETEVENT2KHR);
}

bool CoreChecks::PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                               VkPipelineStageFlags2 stageMask) const {
    return ValidateCmdResetEvent2(commandBuffer, event, stageMask, CMD_RESETEVENT2);
}

// transient helper struct for checking parts of VUID 02285
struct RenderPassDepState {
    using Location = core_error::Location;
    using Func = core_error::Func;
    using Struct = core_error::Struct;
    using Field = core_error::Field;

    const CoreChecks *core;
    const std::string func_name;
    const std::string vuid;
    uint32_t active_subpass;
    const VkRenderPass rp_handle;
    const VkPipelineStageFlags2KHR disabled_features;
    const std::vector<uint32_t> &self_dependencies;
    const safe_VkSubpassDependency2 *dependencies;

    RenderPassDepState(const CoreChecks *c, const std::string &f, const std::string &v, uint32_t subpass, const VkRenderPass handle,
                       const DeviceFeatures &features, const std::vector<uint32_t> &self_deps,
                       const safe_VkSubpassDependency2 *deps)
        : core(c),
          func_name(f),
          vuid(v),
          active_subpass(subpass),
          rp_handle(handle),
          disabled_features(sync_utils::DisabledPipelineStages(features)),
          self_dependencies(self_deps),
          dependencies(deps) {}

    VkMemoryBarrier2KHR GetSubPassDepBarrier(const safe_VkSubpassDependency2 &dep) {
        VkMemoryBarrier2KHR result;

        const auto *barrier = LvlFindInChain<VkMemoryBarrier2KHR>(dep.pNext);
        if (barrier) {
            result = *barrier;
        } else {
            result.srcStageMask = dep.srcStageMask;
            result.dstStageMask = dep.dstStageMask;
            result.srcAccessMask = dep.srcAccessMask;
            result.dstAccessMask = dep.dstAccessMask;
        }
        return result;
    }

    bool ValidateStage(const Location &loc, VkPipelineStageFlags2KHR src_stage_mask, VkPipelineStageFlags2KHR dst_stage_mask) {
        // Look for matching mask in any self-dependency
        bool match = false;
        for (const auto self_dep_index : self_dependencies) {
            const auto sub_dep = GetSubPassDepBarrier(dependencies[self_dep_index]);
            auto sub_src_stage_mask =
                sync_utils::ExpandPipelineStages(sub_dep.srcStageMask, sync_utils::kAllQueueTypes, disabled_features);
            auto sub_dst_stage_mask =
                sync_utils::ExpandPipelineStages(sub_dep.dstStageMask, sync_utils::kAllQueueTypes, disabled_features);
            match = ((sub_src_stage_mask == VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) ||
                     (src_stage_mask == (sub_src_stage_mask & src_stage_mask))) &&
                    ((sub_dst_stage_mask == VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) ||
                     (dst_stage_mask == (sub_dst_stage_mask & dst_stage_mask)));
            if (match) break;
        }
        if (!match) {
            std::stringstream self_dep_ss;
            stream_join(self_dep_ss, ", ", self_dependencies);
            core->LogError(rp_handle, vuid,
                           "%s (0x%" PRIx64
                           ") is not a subset of VkSubpassDependency srcAccessMask "
                           "for any self-dependency of subpass %d of %s for which dstAccessMask is also a subset. "
                           "Candidate VkSubpassDependency are pDependencies entries [%s].",
                           loc.dot(Field::srcStageMask).Message().c_str(), src_stage_mask, active_subpass,
                           core->report_data->FormatHandle(rp_handle).c_str(), self_dep_ss.str().c_str());
            core->LogError(rp_handle, vuid,
                           "%s (0x%" PRIx64
                           ") is not a subset of VkSubpassDependency dstAccessMask "
                           "for any self-dependency of subpass %d of %s for which srcAccessMask is also a subset. "
                           "Candidate VkSubpassDependency are pDependencies entries [%s].",
                           loc.dot(Field::dstStageMask).Message().c_str(), dst_stage_mask, active_subpass,
                           core->report_data->FormatHandle(rp_handle).c_str(), self_dep_ss.str().c_str());
        }
        return !match;
    }

    bool ValidateAccess(const Location &loc, VkAccessFlags2KHR src_access_mask, VkAccessFlags2KHR dst_access_mask) {
        bool match = false;

        for (const auto self_dep_index : self_dependencies) {
            const auto sub_dep = GetSubPassDepBarrier(dependencies[self_dep_index]);
            match = (src_access_mask == (sub_dep.srcAccessMask & src_access_mask)) &&
                    (dst_access_mask == (sub_dep.dstAccessMask & dst_access_mask));
            if (match) break;
        }
        if (!match) {
            std::stringstream self_dep_ss;
            stream_join(self_dep_ss, ", ", self_dependencies);
            core->LogError(rp_handle, vuid,
                           "%s (0x%" PRIx64
                           ") is not a subset of VkSubpassDependency "
                           "srcAccessMask of subpass %d of %s. Candidate VkSubpassDependency are pDependencies entries [%s].",
                           loc.dot(Field::srcAccessMask).Message().c_str(), src_access_mask, active_subpass,
                           core->report_data->FormatHandle(rp_handle).c_str(), self_dep_ss.str().c_str());
            core->LogError(rp_handle, vuid,
                           "%s (0x%" PRIx64
                           ") is not a subset of VkSubpassDependency "
                           "dstAccessMask of subpass %d of %s. Candidate VkSubpassDependency are pDependencies entries [%s].",
                           loc.dot(Field::dstAccessMask).Message().c_str(), dst_access_mask, active_subpass,
                           core->report_data->FormatHandle(rp_handle).c_str(), self_dep_ss.str().c_str());
        }
        return !match;
    }

    bool ValidateDependencyFlag(VkDependencyFlags dependency_flags) {
        bool match = false;

        for (const auto self_dep_index : self_dependencies) {
            const auto &sub_dep = dependencies[self_dep_index];
            match = sub_dep.dependencyFlags == dependency_flags;
            if (match) break;
        }
        if (!match) {
            std::stringstream self_dep_ss;
            stream_join(self_dep_ss, ", ", self_dependencies);
            core->LogError(rp_handle, vuid,
                           "%s: dependencyFlags param (0x%X) does not equal VkSubpassDependency dependencyFlags value for any "
                           "self-dependency of subpass %d of %s. Candidate VkSubpassDependency are pDependencies entries [%s].",
                           func_name.c_str(), dependency_flags, active_subpass, core->report_data->FormatHandle(rp_handle).c_str(),
                           self_dep_ss.str().c_str());
        }
        return !match;
    }
};

// Validate VUs for Pipeline Barriers that are within a renderPass
// Pre: cb_state->activeRenderPass must be a pointer to valid renderPass state
bool CoreChecks::ValidateRenderPassPipelineBarriers(const Location &outer_loc, const CMD_BUFFER_STATE *cb_state,
                                                    VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                                                    VkDependencyFlags dependency_flags, uint32_t mem_barrier_count,
                                                    const VkMemoryBarrier *mem_barriers, uint32_t buffer_mem_barrier_count,
                                                    const VkBufferMemoryBarrier *buffer_mem_barriers,
                                                    uint32_t image_mem_barrier_count,
                                                    const VkImageMemoryBarrier *image_barriers) const {
    bool skip = false;
    const auto &rp_state = cb_state->activeRenderPass;
    RenderPassDepState state(this, outer_loc.StringFunc().c_str(), "VUID-vkCmdPipelineBarrier-pDependencies-02285",
                             cb_state->activeSubpass, rp_state->renderPass(), enabled_features,
                             rp_state->self_dependencies[cb_state->activeSubpass], rp_state->createInfo.pDependencies);
    if (state.self_dependencies.size() == 0) {
        skip |= LogError(state.rp_handle, "VUID-vkCmdPipelineBarrier-pDependencies-02285",
                         "%s Barriers cannot be set during subpass %d of %s with no self-dependency specified.",
                         outer_loc.Message().c_str(), state.active_subpass, report_data->FormatHandle(state.rp_handle).c_str());
        return skip;
    }
    // Grab ref to current subpassDescription up-front for use below
    const auto &sub_desc = rp_state->createInfo.pSubpasses[state.active_subpass];
    skip |= state.ValidateStage(outer_loc, src_stage_mask, dst_stage_mask);

    if (0 != buffer_mem_barrier_count) {
        skip |= LogError(state.rp_handle, "VUID-vkCmdPipelineBarrier-bufferMemoryBarrierCount-01178",
                         "%s: bufferMemoryBarrierCount is non-zero (%d) for subpass %d of %s.", state.func_name.c_str(),
                         buffer_mem_barrier_count, state.active_subpass, report_data->FormatHandle(rp_state->renderPass()).c_str());
    }
    for (uint32_t i = 0; i < mem_barrier_count; ++i) {
        const auto &mem_barrier = mem_barriers[i];
        Location loc(outer_loc.function, Struct::VkMemoryBarrier, Field::pMemoryBarriers, i);
        skip |= state.ValidateAccess(loc, mem_barrier.srcAccessMask, mem_barrier.dstAccessMask);
    }

    for (uint32_t i = 0; i < image_mem_barrier_count; ++i) {
        const auto &img_barrier = image_barriers[i];
        Location loc(outer_loc.function, Struct::VkImageMemoryBarrier, Field::pImageMemoryBarriers, i);
        skip |= state.ValidateAccess(loc, img_barrier.srcAccessMask, img_barrier.dstAccessMask);

        if (VK_QUEUE_FAMILY_IGNORED != img_barrier.srcQueueFamilyIndex ||
            VK_QUEUE_FAMILY_IGNORED != img_barrier.dstQueueFamilyIndex) {
            skip |= LogError(state.rp_handle, "VUID-vkCmdPipelineBarrier-srcQueueFamilyIndex-01182",
                             "%s is %d and dstQueueFamilyIndex is %d but both must be VK_QUEUE_FAMILY_IGNORED.",
                             loc.dot(Field::srcQueueFamilyIndex).Message().c_str(), img_barrier.srcQueueFamilyIndex,
                             img_barrier.dstQueueFamilyIndex);
        }
        // Secondary CBs can have null framebuffer so record will queue up validation in that case 'til FB is known
        if (VK_NULL_HANDLE != cb_state->activeFramebuffer) {
            skip |= ValidateImageBarrierAttachment(loc, cb_state, cb_state->activeFramebuffer.get(), state.active_subpass, sub_desc,
                                                   state.rp_handle, img_barrier);
        }
    }
    skip |= state.ValidateDependencyFlag(dependency_flags);
    return skip;
}

bool CoreChecks::ValidateRenderPassPipelineBarriers(const Location &outer_loc, const CMD_BUFFER_STATE *cb_state,
                                                    const VkDependencyInfoKHR *dep_info) const {
    bool skip = false;
    const auto &rp_state = cb_state->activeRenderPass;
    if (rp_state->UsesDynamicRendering()) {
        return skip;
    }
    RenderPassDepState state(this, outer_loc.StringFunc().c_str(), "VUID-vkCmdPipelineBarrier2-pDependencies-02285",
                             cb_state->activeSubpass, rp_state->renderPass(), enabled_features,
                             rp_state->self_dependencies[cb_state->activeSubpass], rp_state->createInfo.pDependencies);

    if (state.self_dependencies.size() == 0) {
        skip |= LogError(state.rp_handle, state.vuid,
                         "%s: Barriers cannot be set during subpass %d of %s with no self-dependency specified.",
                         state.func_name.c_str(), state.active_subpass, report_data->FormatHandle(rp_state->renderPass()).c_str());
        return skip;
    }
    // Grab ref to current subpassDescription up-front for use below
    const auto &sub_desc = rp_state->createInfo.pSubpasses[state.active_subpass];
    for (uint32_t i = 0; i < dep_info->memoryBarrierCount; ++i) {
        const auto &mem_barrier = dep_info->pMemoryBarriers[i];
        Location loc(outer_loc.function, Struct::VkMemoryBarrier2, Field::pMemoryBarriers, i);
        skip |= state.ValidateStage(loc, mem_barrier.srcStageMask, mem_barrier.dstStageMask);
        skip |= state.ValidateAccess(loc, mem_barrier.srcAccessMask, mem_barrier.dstAccessMask);
    }
    if (0 != dep_info->bufferMemoryBarrierCount) {
        skip |=
            LogError(state.rp_handle, "VUID-vkCmdPipelineBarrier2-bufferMemoryBarrierCount-01178",
                     "%s: bufferMemoryBarrierCount is non-zero (%d) for subpass %d of %s.", state.func_name.c_str(),
                     dep_info->bufferMemoryBarrierCount, state.active_subpass, report_data->FormatHandle(state.rp_handle).c_str());
    }
    for (uint32_t i = 0; i < dep_info->imageMemoryBarrierCount; ++i) {
        const auto &img_barrier = dep_info->pImageMemoryBarriers[i];
        Location loc(outer_loc.function, Struct::VkImageMemoryBarrier2, Field::pImageMemoryBarriers, i);

        skip |= state.ValidateStage(loc, img_barrier.srcStageMask, img_barrier.dstStageMask);
        skip |= state.ValidateAccess(loc, img_barrier.srcAccessMask, img_barrier.dstAccessMask);

        if (VK_QUEUE_FAMILY_IGNORED != img_barrier.srcQueueFamilyIndex ||
            VK_QUEUE_FAMILY_IGNORED != img_barrier.dstQueueFamilyIndex) {
            skip |= LogError(state.rp_handle, "VUID-vkCmdPipelineBarrier2-srcQueueFamilyIndex-01182",
                             "%s is %d and dstQueueFamilyIndex is %d but both must be VK_QUEUE_FAMILY_IGNORED.",
                             loc.dot(Field::srcQueueFamilyIndex).Message().c_str(), img_barrier.srcQueueFamilyIndex,
                             img_barrier.dstQueueFamilyIndex);
        }
        // Secondary CBs can have null framebuffer so record will queue up validation in that case 'til FB is known
        if (VK_NULL_HANDLE != cb_state->activeFramebuffer) {
            skip |= ValidateImageBarrierAttachment(loc, cb_state, cb_state->activeFramebuffer.get(), state.active_subpass, sub_desc,
                                                   state.rp_handle, img_barrier);
        }
    }
    skip |= state.ValidateDependencyFlag(dep_info->dependencyFlags);
    return skip;
}

bool CoreChecks::ValidateStageMasksAgainstQueueCapabilities(const LogObjectList &objlist, const Location &loc,
                                                            VkQueueFlags queue_flags, VkPipelineStageFlags2KHR stage_mask) const {
    bool skip = false;
    // these are always allowed.
    stage_mask &= ~(VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR | VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT_KHR |
                    VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT_KHR | VK_PIPELINE_STAGE_2_HOST_BIT_KHR);
    if (stage_mask == 0) {
        return skip;
    }

    static const std::map<VkPipelineStageFlags2KHR, VkQueueFlags> metaFlags{
        {VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR, VK_QUEUE_GRAPHICS_BIT},
        {VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT_KHR, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT},
        {VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT_KHR, VK_QUEUE_GRAPHICS_BIT},
        {VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR, VK_QUEUE_GRAPHICS_BIT},
    };

    for (const auto &entry : metaFlags) {
        if (((entry.first & stage_mask) != 0) && ((entry.second & queue_flags) == 0)) {
            const auto &vuid = sync_vuid_maps::GetStageQueueCapVUID(loc, entry.first);
            skip |= LogError(objlist, vuid,
                             "%s flag %s is not compatible with the queue family properties (%s) of this command buffer.",
                             loc.Message().c_str(), sync_utils::StringPipelineStageFlags(entry.first).c_str(),
                             string_VkQueueFlags(queue_flags).c_str());
        }
        stage_mask &= ~entry.first;
    }
    if (stage_mask == 0) {
        return skip;
    }

    auto supported_flags = sync_utils::ExpandPipelineStages(VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR, queue_flags);

    auto bad_flags = stage_mask & ~supported_flags;

    // Lookup each bit in the stagemask and check for overlap between its table bits and queue_flags
    for (size_t i = 0; i < sizeof(bad_flags) * 8; i++) {
        VkPipelineStageFlags2KHR bit = (1ULL << i) & bad_flags;
        if (bit) {
            const auto &vuid = sync_vuid_maps::GetStageQueueCapVUID(loc, bit);
            skip |= LogError(
                objlist, vuid, "%s flag %s is not compatible with the queue family properties (%s) of this command buffer.",
                loc.Message().c_str(), sync_utils::StringPipelineStageFlags(bit).c_str(), string_VkQueueFlags(queue_flags).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidatePipelineStageFeatureEnables(const LogObjectList &objlist, const Location &loc,
                                                     VkPipelineStageFlags2KHR stage_mask) const {
    bool skip = false;
    if (!enabled_features.core13.synchronization2 && stage_mask == 0) {
        const auto &vuid = sync_vuid_maps::GetBadFeatureVUID(loc, 0);
        std::stringstream msg;
        msg << loc.Message() << " must not be 0 unless synchronization2 is enabled.";
        skip |= LogError(objlist, vuid, "%s", msg.str().c_str());
    }

    auto disabled_stages = sync_utils::DisabledPipelineStages(enabled_features);
    auto bad_bits = stage_mask & disabled_stages;
    if (bad_bits == 0) {
        return skip;
    }
    for (size_t i = 0; i < sizeof(bad_bits) * 8; i++) {
        VkPipelineStageFlags2KHR bit = 1ULL << i;
        if (bit & bad_bits) {
            const auto &vuid = sync_vuid_maps::GetBadFeatureVUID(loc, bit);
            std::stringstream msg;
            msg << loc.Message() << " includes " << sync_utils::StringPipelineStageFlags(bit) << " when the device does not have "
                << sync_vuid_maps::kFeatureNameMap.at(bit) << " feature enabled.";

            skip |= LogError(objlist, vuid, "%s", msg.str().c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidatePipelineStage(const LogObjectList &objlist, const Location &loc, VkQueueFlags queue_flags,
                                       VkPipelineStageFlags2KHR stage_mask) const {
    bool skip = false;
    skip |= ValidateStageMasksAgainstQueueCapabilities(objlist, loc, queue_flags, stage_mask);
    skip |= ValidatePipelineStageFeatureEnables(objlist, loc, stage_mask);
    return skip;
}

bool CoreChecks::ValidateAccessMask(const LogObjectList &objlist, const Location &loc, VkQueueFlags queue_flags,
                                    VkAccessFlags2KHR access_mask, VkPipelineStageFlags2KHR stage_mask) const {
    bool skip = false;
    // Early out if all commands set
    if ((stage_mask & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR) != 0) return skip;

    // or if only generic memory accesses are specified (or we got a 0 mask)
    access_mask &= ~(VK_ACCESS_2_MEMORY_READ_BIT_KHR | VK_ACCESS_2_MEMORY_WRITE_BIT_KHR);
    if (access_mask == 0) return skip;

    auto expanded_stages = sync_utils::ExpandPipelineStages(stage_mask, queue_flags);  // TODO:
    auto valid_accesses = sync_utils::CompatibleAccessMask(expanded_stages);
    auto bad_accesses = (access_mask & ~valid_accesses);
    if (bad_accesses == 0) {
        return skip;
    }
    for (size_t i = 0; i < sizeof(bad_accesses) * 8; i++) {
        VkAccessFlags2KHR bit = (1ULL << i);
        if (bad_accesses & bit) {
            const auto &vuid = sync_vuid_maps::GetBadAccessFlagsVUID(loc, bit);
            std::stringstream msg;
            msg << loc.Message() << " bit " << sync_utils::StringAccessFlags(bit) << " is not supported by stage mask ("
                << sync_utils::StringPipelineStageFlags(stage_mask) << ").";
            skip |= LogError(objlist, vuid, "%s", msg.str().c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidateEventStageMask(const CMD_BUFFER_STATE &cb_state, size_t eventCount, size_t firstEventIndex,
                                        VkPipelineStageFlags2KHR sourceStageMask, EventToStageMap *localEventToStageMap) {
    bool skip = false;
    const ValidationStateTracker *state_data = cb_state.dev_data;
    VkPipelineStageFlags2KHR stage_mask = 0;
    const auto max_event = std::min((firstEventIndex + eventCount), cb_state.events.size());
    for (size_t event_index = firstEventIndex; event_index < max_event; ++event_index) {
        auto event = cb_state.events[event_index];
        auto event_data = localEventToStageMap->find(event);
        if (event_data != localEventToStageMap->end()) {
            stage_mask |= event_data->second;
        } else {
            auto global_event_data = state_data->Get<EVENT_STATE>(event);
            if (!global_event_data) {
                skip |= state_data->LogError(event, kVUID_Core_DrawState_InvalidEvent,
                                             "%s cannot be waited on if it has never been set.",
                                             state_data->report_data->FormatHandle(event).c_str());
            } else {
                stage_mask |= global_event_data->stageMask;
            }
        }
    }
    // TODO: Need to validate that host_bit is only set if set event is called
    // but set event can be called at any time.
    if (sourceStageMask != stage_mask && sourceStageMask != (stage_mask | VK_PIPELINE_STAGE_HOST_BIT)) {
        skip |= state_data->LogError(
            cb_state.commandBuffer(), "VUID-vkCmdWaitEvents-srcStageMask-parameter",
            "Submitting cmdbuffer with call to VkCmdWaitEvents using srcStageMask 0x%" PRIx64
            " which must be the bitwise OR of "
            "the stageMask parameters used in calls to vkCmdSetEvent and VK_PIPELINE_STAGE_HOST_BIT if used with "
            "vkSetEvent but instead is 0x%" PRIx64 ".",
            sourceStageMask, stage_mask);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                              VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                              uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                              uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                              uint32_t imageMemoryBarrierCount,
                                              const VkImageMemoryBarrier *pImageMemoryBarriers) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);

    auto queue_flags = cb_state->GetQueueFlags();
    const LogObjectList objlist(commandBuffer);
    Location loc(Func::vkCmdWaitEvents);

    skip |= ValidatePipelineStage(objlist, loc.dot(Field::srcStageMask), queue_flags, srcStageMask);
    skip |= ValidatePipelineStage(objlist, loc.dot(Field::dstStageMask), queue_flags, dstStageMask);

    skip |= ValidateCmd(*cb_state, CMD_WAITEVENTS);
    skip |= ValidateBarriers(loc.dot(Field::pDependencyInfo), cb_state.get(), srcStageMask, dstStageMask, memoryBarrierCount,
                             pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
                             pImageMemoryBarriers);
    for (uint32_t i = 0; i < bufferMemoryBarrierCount; ++i) {
        if (pBufferMemoryBarriers[i].srcQueueFamilyIndex != pBufferMemoryBarriers[i].dstQueueFamilyIndex) {
            skip |= LogError(commandBuffer, "VUID-vkCmdWaitEvents-srcQueueFamilyIndex-02803",
                             "vkCmdWaitEvents(): pBufferMemoryBarriers[%" PRIu32 "] has different srcQueueFamilyIndex (%" PRIu32
                             ") and dstQueueFamilyIndex (%" PRIu32 ").",
                             i, pBufferMemoryBarriers[i].srcQueueFamilyIndex, pBufferMemoryBarriers[i].dstQueueFamilyIndex);
        }
    }
    for (uint32_t i = 0; i < imageMemoryBarrierCount; ++i) {
        if (pImageMemoryBarriers[i].srcQueueFamilyIndex != pImageMemoryBarriers[i].dstQueueFamilyIndex) {
            skip |= LogError(commandBuffer, "VUID-vkCmdWaitEvents-srcQueueFamilyIndex-02803",
                             "vkCmdWaitEvents(): pImageMemoryBarriers[%" PRIu32 "] has different srcQueueFamilyIndex (%" PRIu32
                             ") and dstQueueFamilyIndex (%" PRIu32 ").",
                             i, pImageMemoryBarriers[i].srcQueueFamilyIndex, pImageMemoryBarriers[i].dstQueueFamilyIndex);
        }
    }
    return skip;
}

bool CoreChecks::ValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                        const VkDependencyInfo *pDependencyInfos, CMD_TYPE cmd_type) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);

    bool skip = false;
    const char *func_name = CommandTypeString(cmd_type);
    if (!enabled_features.core13.synchronization2) {
        skip |= LogError(commandBuffer, "VUID-vkCmdWaitEvents2-synchronization2-03836",
                         "%s(): Synchronization2 feature is not enabled", func_name);
    }
    for (uint32_t i = 0; (i < eventCount) && !skip; i++) {
        const LogObjectList objlist(commandBuffer, pEvents[i]);
        Location loc(Func::vkCmdWaitEvents2, Field::pDependencyInfos, i);
        if (pDependencyInfos[i].dependencyFlags != 0) {
            skip |= LogError(objlist, "VUID-vkCmdWaitEvents2-dependencyFlags-03844", "%s (%s) must be 0.",
                             loc.dot(Field::dependencyFlags).Message().c_str(),
                             string_VkDependencyFlags(pDependencyInfos[i].dependencyFlags).c_str());
        }
        skip |= ValidateDependencyInfo(objlist, loc, cb_state.get(), &pDependencyInfos[i]);
    }
    skip |= ValidateCmd(*cb_state, cmd_type);
    return skip;
}

bool CoreChecks::PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                  const VkDependencyInfoKHR *pDependencyInfos) const {
    return ValidateCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, CMD_WAITEVENTS2KHR);
}

bool CoreChecks::PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                               const VkDependencyInfo *pDependencyInfos) const {
    return ValidateCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, CMD_WAITEVENTS2);
}

void CORE_CMD_BUFFER_STATE::RecordWaitEvents(CMD_TYPE cmd_type, uint32_t eventCount, const VkEvent *pEvents,
                                             VkPipelineStageFlags2KHR srcStageMask) {
    // CMD_BUFFER_STATE will add to the events vector.
    auto first_event_index = events.size();
    CMD_BUFFER_STATE::RecordWaitEvents(cmd_type, eventCount, pEvents, srcStageMask);
    auto event_added_count = events.size() - first_event_index;
    eventUpdates.emplace_back([event_added_count, first_event_index, srcStageMask](CMD_BUFFER_STATE &cb_state, bool do_validate,
                                                                                   EventToStageMap *localEventToStageMap) {
        if (!do_validate) return false;
        return CoreChecks::ValidateEventStageMask(cb_state, event_added_count, first_event_index, srcStageMask,
                                                  localEventToStageMap);
    });
}

void CoreChecks::PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                            VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                            uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                            uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                            uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers) {
    StateTracker::PreCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, sourceStageMask, dstStageMask, memoryBarrierCount,
                                             pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                                             imageMemoryBarrierCount, pImageMemoryBarriers);
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    TransitionImageLayouts(cb_state.get(), imageMemoryBarrierCount, pImageMemoryBarriers);
}

void CoreChecks::RecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                      const VkDependencyInfo *pDependencyInfos, CMD_TYPE cmd_type) {
    // don't hold read lock during the base class method
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    for (uint32_t i = 0; i < eventCount; i++) {
        const auto &dep_info = pDependencyInfos[i];
        TransitionImageLayouts(cb_state.get(), dep_info.imageMemoryBarrierCount, dep_info.pImageMemoryBarriers);
    }
}

void CoreChecks::PreCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                const VkDependencyInfoKHR *pDependencyInfos) {
    StateTracker::PreCallRecordCmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);
    RecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, CMD_WAITEVENTS2KHR);
}

void CoreChecks::PreCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                             const VkDependencyInfo *pDependencyInfos) {
    StateTracker::PreCallRecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos);
    RecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, CMD_WAITEVENTS2);
}

void CoreChecks::PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                             VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                             uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                             uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                             uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    RecordBarriers(Func::vkCmdWaitEvents, cb_state.get(), bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
                   pImageMemoryBarriers);
}

void CoreChecks::PostCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                 const VkDependencyInfoKHR *pDependencyInfos) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    for (uint32_t i = 0; i < eventCount; i++) {
        const auto &dep_info = pDependencyInfos[i];
        RecordBarriers(Func::vkCmdWaitEvents2, cb_state.get(), dep_info);
    }
}

void CoreChecks::PostCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                              const VkDependencyInfo *pDependencyInfos) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    for (uint32_t i = 0; i < eventCount; i++) {
        const auto &dep_info = pDependencyInfos[i];
        RecordBarriers(Func::vkCmdWaitEvents2, cb_state.get(), dep_info);
    }
}

bool CoreChecks::PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                   VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                   uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                   uint32_t bufferMemoryBarrierCount,
                                                   const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                   uint32_t imageMemoryBarrierCount,
                                                   const VkImageMemoryBarrier *pImageMemoryBarriers) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    const LogObjectList objlist(commandBuffer);
    auto queue_flags = cb_state->GetQueueFlags();
    Location loc(Func::vkCmdPipelineBarrier);

    skip |= ValidatePipelineStage(objlist, loc.dot(Field::srcStageMask), queue_flags, srcStageMask);
    skip |= ValidatePipelineStage(objlist, loc.dot(Field::dstStageMask), queue_flags, dstStageMask);
    skip |= ValidateCmd(*cb_state, CMD_PIPELINEBARRIER);
    if (cb_state->activeRenderPass && !cb_state->activeRenderPass->UsesDynamicRendering()) {
        skip |= ValidateRenderPassPipelineBarriers(loc, cb_state.get(), srcStageMask, dstStageMask, dependencyFlags,
                                                   memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
                                                   pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
        if (skip) return true;  // Early return to avoid redundant errors from below calls
    } else {
        if (dependencyFlags & VK_DEPENDENCY_VIEW_LOCAL_BIT) {
            skip = LogError(objlist, "VUID-vkCmdPipelineBarrier-dependencyFlags-01186",
                            "%s VK_DEPENDENCY_VIEW_LOCAL_BIT must not be set outside of a render pass instance",
                            loc.dot(Field::dependencyFlags).Message().c_str());
        }
    }
    if (cb_state->activeRenderPass && cb_state->activeRenderPass->UsesDynamicRendering()) {
        skip |= LogError(commandBuffer, "VUID-vkCmdPipelineBarrier-None-06191",
                         "vkCmdPipelineBarrier(): a dynamic render pass instance is active.");
    }
    skip |= ValidateBarriers(loc, cb_state.get(), srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers,
                             bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    return skip;
}

bool CoreChecks::ValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo,
                                             CMD_TYPE cmd_type) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    const LogObjectList objlist(commandBuffer);
    const char *func_name = CommandTypeString(cmd_type);

    Location loc(Func::vkCmdPipelineBarrier2, Field::pDependencyInfo);
    if (!enabled_features.core13.synchronization2) {
        skip |= LogError(commandBuffer, "VUID-vkCmdPipelineBarrier2-synchronization2-03848",
                         "%s(): Synchronization2 feature is not enabled", func_name);
    }
    skip |= ValidateCmd(*cb_state, cmd_type);
    if (cb_state->activeRenderPass) {
        skip |= ValidateRenderPassPipelineBarriers(loc, cb_state.get(), pDependencyInfo);
        if (skip) return true;  // Early return to avoid redundant errors from below calls
    } else {
        if (pDependencyInfo->dependencyFlags & VK_DEPENDENCY_VIEW_LOCAL_BIT) {
            skip = LogError(objlist, "VUID-vkCmdPipelineBarrier2-dependencyFlags-01186",
                            "%s VK_DEPENDENCY_VIEW_LOCAL_BIT must not be set outside of a render pass instance",
                            loc.dot(Field::dependencyFlags).Message().c_str());
        }
    }
    if (cb_state->activeRenderPass && cb_state->activeRenderPass->UsesDynamicRendering()) {
        skip |= LogError(commandBuffer, "VUID-vkCmdPipelineBarrier2-None-06191",
                         "vkCmdPipelineBarrier(): a dynamic render pass instance is active.");
    }
    skip |= ValidateDependencyInfo(objlist, loc, cb_state.get(), pDependencyInfo);
    return skip;
}

bool CoreChecks::PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer,
                                                       const VkDependencyInfoKHR *pDependencyInfo) const {
    return ValidateCmdPipelineBarrier2(commandBuffer, pDependencyInfo, CMD_PIPELINEBARRIER2KHR);
}

bool CoreChecks::PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo) const {
    return ValidateCmdPipelineBarrier2(commandBuffer, pDependencyInfo, CMD_PIPELINEBARRIER2);
}

void CoreChecks::PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                 VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                 uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                 uint32_t bufferMemoryBarrierCount,
                                                 const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                 uint32_t imageMemoryBarrierCount,
                                                 const VkImageMemoryBarrier *pImageMemoryBarriers) {
    StateTracker::PreCallRecordCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount,
                                                  pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                                                  imageMemoryBarrierCount, pImageMemoryBarriers);

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);

    RecordBarriers(Func::vkCmdPipelineBarrier, cb_state.get(), bufferMemoryBarrierCount, pBufferMemoryBarriers,
                   imageMemoryBarrierCount, pImageMemoryBarriers);
    TransitionImageLayouts(cb_state.get(), imageMemoryBarrierCount, pImageMemoryBarriers);
}

void CoreChecks::PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo) {
    StateTracker::PreCallRecordCmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    RecordBarriers(Func::vkCmdPipelineBarrier2, cb_state.get(), *pDependencyInfo);
    TransitionImageLayouts(cb_state.get(), pDependencyInfo->imageMemoryBarrierCount, pDependencyInfo->pImageMemoryBarriers);
}

void CoreChecks::PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo) {
    StateTracker::PreCallRecordCmdPipelineBarrier2(commandBuffer, pDependencyInfo);

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    RecordBarriers(Func::vkCmdPipelineBarrier2, cb_state.get(), *pDependencyInfo);
    TransitionImageLayouts(cb_state.get(), pDependencyInfo->imageMemoryBarrierCount, pDependencyInfo->pImageMemoryBarriers);
}

bool CoreChecks::PreCallValidateCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                 VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                                                 const void *pValues) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    skip |= ValidateCmd(*cb_state, CMD_PUSHCONSTANTS);

    // Check if pipeline_layout VkPushConstantRange(s) overlapping offset, size have stageFlags set for each stage in the command
    // stageFlags argument, *and* that the command stageFlags argument has bits set for the stageFlags in each overlapping range.
    if (!skip) {
        auto layout_state = Get<PIPELINE_LAYOUT_STATE>(layout);
        const auto &ranges = *layout_state->push_constant_ranges;
        VkShaderStageFlags found_stages = 0;
        for (const auto &range : ranges) {
            if ((offset >= range.offset) && (offset + size <= range.offset + range.size)) {
                VkShaderStageFlags matching_stages = range.stageFlags & stageFlags;
                if (matching_stages != range.stageFlags) {
                    skip |=
                        LogError(commandBuffer, "VUID-vkCmdPushConstants-offset-01796",
                                 "vkCmdPushConstants(): stageFlags (%s, offset (%" PRIu32 "), and size (%" PRIu32
                                 "),  must contain all stages in overlapping VkPushConstantRange stageFlags (%s), offset (%" PRIu32
                                 "), and size (%" PRIu32 ") in %s.",
                                 string_VkShaderStageFlags(stageFlags).c_str(), offset, size,
                                 string_VkShaderStageFlags(range.stageFlags).c_str(), range.offset, range.size,
                                 report_data->FormatHandle(layout).c_str());
                }

                // Accumulate all stages we've found
                found_stages = matching_stages | found_stages;
            }
        }
        if (found_stages != stageFlags) {
            uint32_t missing_stages = ~found_stages & stageFlags;
            skip |= LogError(
                commandBuffer, "VUID-vkCmdPushConstants-offset-01795",
                "vkCmdPushConstants(): %s, VkPushConstantRange in %s overlapping offset = %d and size = %d, do not contain %s.",
                string_VkShaderStageFlags(stageFlags).c_str(), report_data->FormatHandle(layout).c_str(), offset, size,
                string_VkShaderStageFlags(missing_stages).c_str());
        }
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
                        skip |= ValidateMemoryIsBoundToImage(image_data.get(), "vkCreateFramebuffer()",
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

bool CoreChecks::ValidateCmdNextSubpass(RenderPassCreateVersion rp_version, VkCommandBuffer commandBuffer,
                                        CMD_TYPE cmd_type) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    bool skip = false;
    const bool use_rp2 = (rp_version == RENDER_PASS_VERSION_2);
    const char *vuid;
    const char *function_name = CommandTypeString(cmd_type);

    skip |= ValidateCmd(*cb_state, cmd_type);

    auto subpass_count = cb_state->activeRenderPass->createInfo.subpassCount;
    if (cb_state->activeSubpass == subpass_count - 1) {
        vuid = use_rp2 ? "VUID-vkCmdNextSubpass2-None-03102" : "VUID-vkCmdNextSubpass-None-00909";
        skip |= LogError(commandBuffer, vuid, "%s: Attempted to advance beyond final subpass.", function_name);
    }
    if (cb_state->transform_feedback_active) {
        vuid = use_rp2 ? "VUID-vkCmdNextSubpass2-None-02350" : "VUID-vkCmdNextSubpass-None-02349";
        skip |= LogError(commandBuffer, vuid, "%s: transform feedback is active.", function_name);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) const {
    return ValidateCmdNextSubpass(RENDER_PASS_VERSION_1, commandBuffer, CMD_NEXTSUBPASS);
}

bool CoreChecks::PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                   const VkSubpassEndInfo *pSubpassEndInfo) const {
    return ValidateCmdNextSubpass(RENDER_PASS_VERSION_2, commandBuffer, CMD_NEXTSUBPASS2KHR);
}

bool CoreChecks::PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                const VkSubpassEndInfo *pSubpassEndInfo) const {
    return ValidateCmdNextSubpass(RENDER_PASS_VERSION_2, commandBuffer, CMD_NEXTSUBPASS2);
}

void CoreChecks::RecordCmdNextSubpassLayouts(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto framebuffer = Get<FRAMEBUFFER_STATE>(cb_state->activeRenderPassBeginInfo.framebuffer);
    TransitionSubpassLayouts(cb_state.get(), cb_state->activeRenderPass.get(), cb_state->activeSubpass, framebuffer.get());
}

void CoreChecks::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    StateTracker::PostCallRecordCmdNextSubpass(commandBuffer, contents);
    RecordCmdNextSubpassLayouts(commandBuffer, contents);
}

void CoreChecks::PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                  const VkSubpassEndInfo *pSubpassEndInfo) {
    StateTracker::PostCallRecordCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    RecordCmdNextSubpassLayouts(commandBuffer, pSubpassBeginInfo->contents);
}

void CoreChecks::PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                               const VkSubpassEndInfo *pSubpassEndInfo) {
    StateTracker::PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    RecordCmdNextSubpassLayouts(commandBuffer, pSubpassBeginInfo->contents);
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
                        primary_state_->commandBuffer(), "VUID-vkCmdDraw-commandBuffer-02701",
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
            return validation_.LogError(primary_state_->commandBuffer(), "VUID-vkCmdDraw-commandBuffer-02701", "%s", ss.str().c_str());
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
                primary_state_->commandBuffer(), "VUID-vkCmdDraw-commandBuffer-02701",
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

    if (enabled_features.inherited_viewport_scissor_features.inheritedViewportScissor2D)
    {
        skip |= viewport_scissor_inheritance.VisitPrimary(cb_state.get());
    }

    bool active_occlusion_query = false;
    for (const auto& active_query : cb_state->activeQueries) {
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

        if (enabled_features.inherited_viewport_scissor_features.inheritedViewportScissor2D)
        {
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
            for (sparse_container::parallel_iterator<const ImageSubresourceLayoutMap::LayoutMap> iter(sub_layout_map, cb_layout_map, 0);
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
                    for (auto index = iter->range.begin; index < iter->range.end;  index++) {
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

bool CoreChecks::PreCallValidateSetEvent(VkDevice device, VkEvent event) const {
    bool skip = false;
    auto event_state = Get<EVENT_STATE>(event);
    if (event_state) {
        if (event_state->write_in_use) {
            skip |=
                LogError(event, kVUID_Core_DrawState_QueueForwardProgress,
                         "vkSetEvent(): %s that is already in use by a command buffer.", report_data->FormatHandle(event).c_str());
        }
        if (event_state->flags & VK_EVENT_CREATE_DEVICE_ONLY_BIT_KHR) {
            skip |= LogError(event, "VUID-vkSetEvent-event-03941",
                             "vkSetEvent(): %s was created with VK_EVENT_CREATE_DEVICE_ONLY_BIT_KHR.",
                             report_data->FormatHandle(event).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateResetEvent(VkDevice device, VkEvent event) const {
    bool skip = false;
    auto event_state = Get<EVENT_STATE>(event);
    if (event_state) {
        if (event_state->flags & VK_EVENT_CREATE_DEVICE_ONLY_BIT_KHR) {
            skip |= LogError(event, "VUID-vkResetEvent-event-03823",
                             "vkResetEvent(): %s was created with VK_EVENT_CREATE_DEVICE_ONLY_BIT_KHR.",
                             report_data->FormatHandle(event).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetEventStatus(VkDevice device, VkEvent event) const {
    bool skip = false;
    auto event_state = Get<EVENT_STATE>(event);
    if (event_state) {
        if (event_state->flags & VK_EVENT_CREATE_DEVICE_ONLY_BIT_KHR) {
            skip |= LogError(event, "VUID-vkGetEventStatus-event-03940",
                             "vkGetEventStatus(): %s was created with VK_EVENT_CREATE_DEVICE_ONLY_BIT_KHR.",
                             report_data->FormatHandle(event).c_str());
        }
    }
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

bool CoreChecks::ValidateSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo, const char *api_name) const {
    bool skip = false;
    auto semaphore_state = Get<SEMAPHORE_STATE>(pSignalInfo->semaphore);
    if (!semaphore_state) {
        return skip;
    }
    if (semaphore_state->type != VK_SEMAPHORE_TYPE_TIMELINE) {
        skip |= LogError(pSignalInfo->semaphore, "VUID-VkSemaphoreSignalInfo-semaphore-03257",
                         "%s(): semaphore %s must be of VK_SEMAPHORE_TYPE_TIMELINE type.", api_name,
                         report_data->FormatHandle(pSignalInfo->semaphore).c_str());
        return skip;
    }

    const auto completed = semaphore_state->Completed();
    if (completed.payload >= pSignalInfo->value) {
        skip |= LogError(pSignalInfo->semaphore, "VUID-VkSemaphoreSignalInfo-value-03258",
                         "%s(): value (%" PRIu64 ") must be greater than current semaphore %s value (%" PRIu64 ").", api_name,
                         pSignalInfo->value, report_data->FormatHandle(pSignalInfo->semaphore).c_str(), completed.payload);
        return skip;
    }
    auto exceeds_pending = [pSignalInfo](const SEMAPHORE_STATE::SemOp &op, bool is_pending) {
        return is_pending && op.IsSignal() && pSignalInfo->value >= op.payload;
    };
    auto last_op = semaphore_state->LastOp(exceeds_pending);
    if (last_op) {
        skip |= LogError(pSignalInfo->semaphore, "VUID-VkSemaphoreSignalInfo-value-03259",
                         "%s(): value (%" PRIu64 ") must be less than value of any pending signal operation (%" PRIu64
                         ") for semaphore %s.",
                         api_name, pSignalInfo->value, last_op->payload, report_data->FormatHandle(pSignalInfo->semaphore).c_str());
        return skip;
    }

    uint64_t bad_value = 0;
    const char *where = nullptr;
    TimelineMaxDiffCheck exceeds_max_diff(pSignalInfo->value, phys_dev_props_core12.maxTimelineSemaphoreValueDifference);
    last_op = semaphore_state->LastOp(exceeds_max_diff);
    if (last_op) {
        bad_value = last_op->payload;
        if (last_op->payload == semaphore_state->Completed().payload) {
            where = "current";
        } else {
            where = "pending";
        }
    }
    if (where) {
        Location loc(Func::vkSignalSemaphore, Struct::VkSemaphoreSignalInfo, Field::value);
        const auto &vuid = sync_vuid_maps::GetQueueSubmitVUID(loc, sync_vuid_maps::SubmitError::kTimelineSemMaxDiff);
        skip |=
            LogError(semaphore_state->Handle(), vuid,
                     "%s value (%" PRIu64 ") exceeds limit regarding %s semaphore %s payload (%" PRIu64 ").", loc.Message().c_str(),
                     pSignalInfo->value, report_data->FormatHandle(semaphore_state->Handle()).c_str(), where, bad_value);
    }
    return skip;
}

bool CoreChecks::PreCallValidateSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo) const {
    return ValidateSignalSemaphore(device, pSignalInfo, "vkSignalSemaphore");
}

bool CoreChecks::PreCallValidateSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo) const {
    return ValidateSignalSemaphore(device, pSignalInfo, "vkSignalSemaphoreKHR");
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecks::PreCallValidateImportSemaphoreWin32HandleKHR(VkDevice device,
                                                              const VkImportSemaphoreWin32HandleInfoKHR *info) const {
    bool skip = false;
    const char *func_name = "vkImportSemaphoreWin32HandleKHR";
    auto sem_state = Get<SEMAPHORE_STATE>(info->semaphore);
    if (sem_state) {
        skip |= ValidateObjectNotInUse(sem_state.get(), func_name, kVUIDUndefined);

        if ((info->flags & VK_SEMAPHORE_IMPORT_TEMPORARY_BIT) != 0 && sem_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            skip |= LogError(
                sem_state->Handle(), "VUID-VkImportSemaphoreWin32HandleInfoKHR-flags-03322",
                "vkImportSemaphoreWin32HandleKHR(): VK_SEMAPHORE_IMPORT_TEMPORARY_BIT not allowed for timeline semaphores");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR *info,
                                                           HANDLE *pHandle) const {
    bool skip = false;
    const char *func_name = "vkGetSemaphoreWin32HandleKHR";
    auto sem_state = Get<SEMAPHORE_STATE>(info->semaphore);
    if (sem_state) {
        if ((info->handleType & sem_state->exportHandleTypes) == 0) {
            skip |= LogError(sem_state->Handle(), "VUID-VkSemaphoreGetWin32HandleInfoKHR-handleType-01126",
                             "%s: handleType %s was not VkExportSemaphoreCreateInfo::handleTypes (%s)", func_name,
                             string_VkExternalSemaphoreHandleTypeFlagBits(info->handleType),
                             string_VkExternalSemaphoreHandleTypeFlags(sem_state->exportHandleTypes).c_str());
        }
    }
    return skip;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

bool CoreChecks::PreCallValidateImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR *info) const {
    bool skip = false;
    const char *func_name = "vkImportSemaphoreFdKHR";
    auto sem_state = Get<SEMAPHORE_STATE>(info->semaphore);
    if (sem_state) {
        skip |= ValidateObjectNotInUse(sem_state.get(), func_name, kVUIDUndefined);

        if ((info->flags & VK_SEMAPHORE_IMPORT_TEMPORARY_BIT) != 0 && sem_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            skip |= LogError(sem_state->Handle(), "VUID-VkImportSemaphoreFdInfoKHR-flags-03323",
                             "%s(): VK_SEMAPHORE_IMPORT_TEMPORARY_BIT not allowed for timeline semaphores", func_name);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR *info, int *pFd) const {
    bool skip = false;
    const char *func_name = "vkGetSemaphoreFdKHR";
    auto sem_state = Get<SEMAPHORE_STATE>(info->semaphore);
    if (sem_state) {
        if ((info->handleType & sem_state->exportHandleTypes) == 0) {
            skip |= LogError(sem_state->Handle(), "VUID-VkSemaphoreGetFdInfoKHR-handleType-01132",
                             "%s(): handleType %s was not VkExportSemaphoreCreateInfo::handleTypes (%s)", func_name,
                             string_VkExternalSemaphoreHandleTypeFlagBits(info->handleType),
                             string_VkExternalSemaphoreHandleTypeFlags(sem_state->exportHandleTypes).c_str());
        }

        if (info->handleType == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT) {
            if (sem_state->type != VK_SEMAPHORE_TYPE_BINARY) {
                skip |= LogError(sem_state->Handle(), "VUID-VkSemaphoreGetFdInfoKHR-handleType-03253",
                                 "%s(): can only export binary semaphores to %s", func_name,
                                 string_VkExternalSemaphoreHandleTypeFlagBits(info->handleType));
            }
            if (!sem_state->CanBeWaited()) {
                skip |= LogError(sem_state->Handle(), "VUID-VkSemaphoreGetFdInfoKHR-handleType-03254",
                                 "%s(): must be signaled or have a pending signal operation", func_name);
            }
        }
    }
    return skip;
}

#ifdef VK_USE_PLATFORM_FUCHSIA
bool CoreChecks::PreCallValidateImportSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                                   const VkImportSemaphoreZirconHandleInfoFUCHSIA *info) const {
    bool skip = false;
    const char *func_name = "vkImportSemaphoreZirconHandleFUCHSIA";
    auto sem_state = Get<SEMAPHORE_STATE>(info->semaphore);
    if (sem_state) {
        skip |= ValidateObjectNotInUse(sem_state.get(), func_name, kVUIDUndefined);

        if (sem_state->type == VK_SEMAPHORE_TYPE_TIMELINE) {
            skip |=
                LogError(sem_state->Handle(), "VUID-VkImportSemaphoreZirconHandleInfoFUCHSIA-semaphoreType-04768",
                         "%s(): VkSemaphoreTypeCreateInfo::semaphoreType field must not be VK_SEMAPHORE_TYPE_TIMELINE", func_name);
        }
    }
    return skip;
}

void CoreChecks::PostCallRecordImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device, const VkImportSemaphoreZirconHandleInfoFUCHSIA *pImportSemaphoreZirconHandleInfo, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordImportSemaphoreState(pImportSemaphoreZirconHandleInfo->semaphore, pImportSemaphoreZirconHandleInfo->handleType,
                               pImportSemaphoreZirconHandleInfo->flags);
}

void CoreChecks::PostCallRecordGetSemaphoreZirconHandleFUCHSIA(VkDevice device,
                                                               const VkSemaphoreGetZirconHandleInfoFUCHSIA *pGetZirconHandleInfo,
                                                               zx_handle_t *pZirconHandle, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordGetExternalSemaphoreState(pGetZirconHandleInfo->semaphore, pGetZirconHandleInfo->handleType);
}
#endif

bool CoreChecks::ValidateImportFence(VkFence fence, const char *vuid, const char *caller_name) const {
    auto fence_node = Get<FENCE_STATE>(fence);
    bool skip = false;
    if (fence_node && fence_node->Scope() == kSyncScopeInternal && fence_node->State() == FENCE_INFLIGHT) {
        skip |=
            LogError(fence, vuid, "%s: Fence %s that is currently in use.", caller_name, report_data->FormatHandle(fence).c_str());
    }
    return skip;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool CoreChecks::PreCallValidateImportFenceWin32HandleKHR(
    VkDevice device, const VkImportFenceWin32HandleInfoKHR *pImportFenceWin32HandleInfo) const {
    return ValidateImportFence(pImportFenceWin32HandleInfo->fence, "VUID-vkImportFenceWin32HandleKHR-fence-04448",
                               "vkImportFenceWin32HandleKHR");
}

bool CoreChecks::PreCallValidateGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR *info,
                                                       HANDLE *pHandle) const {
    bool skip = false;
    const char *func_name = "vkGetFenceWin32HandleKHR";
    auto fence_state = Get<FENCE_STATE>(info->fence);
    if (fence_state) {
        if ((info->handleType & fence_state->exportHandleTypes) == 0) {
            skip |= LogError(fence_state->Handle(), "VUID-VkFenceGetWin32HandleInfoKHR-handleType-01448",
                             "%s: handleType %s was not VkExportFenceCreateInfo::handleTypes (%s)", func_name,
                             string_VkExternalFenceHandleTypeFlagBits(info->handleType),
                             string_VkExternalFenceHandleTypeFlags(fence_state->exportHandleTypes).c_str());
        }
    }
    return skip;
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

bool CoreChecks::PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *pImportFenceFdInfo) const {
    return ValidateImportFence(pImportFenceFdInfo->fence, "VUID-vkImportFenceFdKHR-fence-01463", "vkImportFenceFdKHR");
}

bool CoreChecks::PreCallValidateGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *info, int *pFd) const {
    bool skip = false;
    const char *func_name = "vkGetFenceFdKHR";
    auto fence_state = Get<FENCE_STATE>(info->fence);
    if (fence_state) {
        if ((info->handleType & fence_state->exportHandleTypes) == 0) {
            skip |= LogError(fence_state->Handle(), "VUID-VkFenceGetFdInfoKHR-handleType-01453",
                             "%s: handleType %s was not VkExportFenceCreateInfo::handleTypes (%s)", func_name,
                             string_VkExternalFenceHandleTypeFlagBits(info->handleType),
                             string_VkExternalFenceHandleTypeFlags(fence_state->exportHandleTypes).c_str());
        }
        if (info->handleType == VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT && fence_state->State() == FENCE_UNSIGNALED) {
            skip |= LogError(fence_state->Handle(), "VUID-VkFenceGetFdInfoKHR-handleType-01454",
                             "%s(): cannot export to %s unless the fence has a pending signal operation or is already signaled",
                             func_name, string_VkExternalFenceHandleTypeFlagBits(info->handleType));
        }
    }
    return skip;
}

bool CoreChecks::ValidateDescriptorUpdateTemplate(const char *func_name,
                                                  const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo) const {
    bool skip = false;
    auto layout = Get<cvdescriptorset::DescriptorSetLayout>(pCreateInfo->descriptorSetLayout);
    if (VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET == pCreateInfo->templateType && !layout) {
        skip |= LogError(pCreateInfo->descriptorSetLayout, "VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00350",
                         "%s: Invalid pCreateInfo->descriptorSetLayout (%s)", func_name,
                         report_data->FormatHandle(pCreateInfo->descriptorSetLayout).c_str());
    } else if (VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR == pCreateInfo->templateType) {
        auto bind_point = pCreateInfo->pipelineBindPoint;
        const bool valid_bp = (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) || (bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) ||
                              (bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
        if (!valid_bp) {
            skip |=
                LogError(device, "VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00351",
                         "%s: Invalid pCreateInfo->pipelineBindPoint (%" PRIu32 ").", func_name, static_cast<uint32_t>(bind_point));
        }
        auto pipeline_layout = Get<PIPELINE_LAYOUT_STATE>(pCreateInfo->pipelineLayout);
        if (!pipeline_layout) {
            skip |= LogError(pCreateInfo->pipelineLayout, "VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00352",
                             "%s: Invalid pCreateInfo->pipelineLayout (%s)", func_name,
                             report_data->FormatHandle(pCreateInfo->pipelineLayout).c_str());
        } else {
            const uint32_t pd_set = pCreateInfo->set;
            if ((pd_set >= pipeline_layout->set_layouts.size()) || !pipeline_layout->set_layouts[pd_set] ||
                !pipeline_layout->set_layouts[pd_set]->IsPushDescriptor()) {
                skip |= LogError(pCreateInfo->pipelineLayout, "VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00353",
                                 "%s: pCreateInfo->set (%" PRIu32
                                 ") does not refer to the push descriptor set layout for pCreateInfo->pipelineLayout (%s).",
                                 func_name, pd_set, report_data->FormatHandle(pCreateInfo->pipelineLayout).c_str());
            }
        }
    } else if (VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET == pCreateInfo->templateType) {
        for (const auto &binding : layout->GetBindings()) {
            if (binding.descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
                skip |= LogError(
                    device, "VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-04615",
                    "%s: pCreateInfo->templateType is VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET, but "
                    "pCreateInfo->descriptorSetLayout contains a binding with descriptor type VK_DESCRIPTOR_TYPE_MUTABLE_EXT.",
                    func_name);
            }
        }
    }
    for (uint32_t i = 0; i < pCreateInfo->descriptorUpdateEntryCount; ++i) {
        const auto &descriptor_update = pCreateInfo->pDescriptorUpdateEntries[i];
        if (descriptor_update.descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT) {
            if (descriptor_update.dstArrayElement & 3) {
                skip |= LogError(pCreateInfo->pipelineLayout, "VUID-VkDescriptorUpdateTemplateEntry-descriptor-02226",
                                 "%s: pCreateInfo->pDescriptorUpdateEntries[%" PRIu32
                                 "] has descriptorType VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT, but dstArrayElement (%" PRIu32
                                 ") is not a "
                                 "multiple of 4).",
                                 func_name, i, descriptor_update.dstArrayElement);
            }
            if (descriptor_update.descriptorCount & 3) {
                skip |= LogError(pCreateInfo->pipelineLayout, "VUID-VkDescriptorUpdateTemplateEntry-descriptor-02227",
                                 "%s: pCreateInfo->pDescriptorUpdateEntries[%" PRIu32
                                 "] has descriptorType VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT, but descriptorCount (%" PRIu32
                                 ")is not a "
                                 "multiple of 4).",
                                 func_name, i, descriptor_update.descriptorCount);
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateDescriptorUpdateTemplate(VkDevice device,
                                                               const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate) const {
    bool skip = ValidateDescriptorUpdateTemplate("vkCreateDescriptorUpdateTemplate()", pCreateInfo);
    return skip;
}

bool CoreChecks::PreCallValidateCreateDescriptorUpdateTemplateKHR(VkDevice device,
                                                                  const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
                                                                  const VkAllocationCallbacks *pAllocator,
                                                                  VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate) const {
    bool skip = ValidateDescriptorUpdateTemplate("vkCreateDescriptorUpdateTemplateKHR()", pCreateInfo);
    return skip;
}

bool CoreChecks::ValidateUpdateDescriptorSetWithTemplate(VkDescriptorSet descriptorSet,
                                                         VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                         const void *pData) const {
    bool skip = false;
    auto template_state = Get<UPDATE_TEMPLATE_STATE>(descriptorUpdateTemplate);
    // Object tracker will report errors for invalid descriptorUpdateTemplate values, avoiding a crash in release builds
    // but retaining the assert as template support is new enough to want to investigate these in debug builds.
    assert(template_state);
    // TODO: Validate template push descriptor updates
    if (template_state->create_info.templateType == VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET) {
        skip = ValidateUpdateDescriptorSetsWithTemplateKHR(descriptorSet, template_state.get(), pData);
    }
    return skip;
}

bool CoreChecks::PreCallValidateUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                                VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                const void *pData) const {
    return ValidateUpdateDescriptorSetWithTemplate(descriptorSet, descriptorUpdateTemplate, pData);
}

bool CoreChecks::PreCallValidateUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                                   VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                   const void *pData) const {
    return ValidateUpdateDescriptorSetWithTemplate(descriptorSet, descriptorUpdateTemplate, pData);
}

bool CoreChecks::PreCallValidateCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                                    VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                    VkPipelineLayout layout, uint32_t set,
                                                                    const void *pData) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    const char *const func_name = "vkPushDescriptorSetWithTemplateKHR()";
    bool skip = false;
    skip |= ValidateCmd(*cb_state, CMD_PUSHDESCRIPTORSETWITHTEMPLATEKHR);

    auto layout_data = Get<PIPELINE_LAYOUT_STATE>(layout);
    const auto dsl = layout_data ? layout_data->GetDsl(set) : nullptr;
    // Validate the set index points to a push descriptor set and is in range
    if (dsl) {
        if (!dsl->IsPushDescriptor()) {
            skip = LogError(layout, "VUID-vkCmdPushDescriptorSetKHR-set-00365",
                            "%s: Set index %" PRIu32 " does not match push descriptor set layout index for %s.", func_name, set,
                            report_data->FormatHandle(layout).c_str());
        }
    } else if (layout_data && (set >= layout_data->set_layouts.size())) {
        skip = LogError(layout, "VUID-vkCmdPushDescriptorSetKHR-set-00364",
                        "%s: Set index %" PRIu32 " is outside of range for %s (set < %" PRIu32 ").", func_name, set,
                        report_data->FormatHandle(layout).c_str(), static_cast<uint32_t>(layout_data->set_layouts.size()));
    }

    auto template_state = Get<UPDATE_TEMPLATE_STATE>(descriptorUpdateTemplate);
    if (template_state) {
        const auto &template_ci = template_state->create_info;
        static const std::map<VkPipelineBindPoint, std::string> bind_errors = {
            std::make_pair(VK_PIPELINE_BIND_POINT_GRAPHICS, "VUID-vkCmdPushDescriptorSetWithTemplateKHR-commandBuffer-00366"),
            std::make_pair(VK_PIPELINE_BIND_POINT_COMPUTE, "VUID-vkCmdPushDescriptorSetWithTemplateKHR-commandBuffer-00366"),
            std::make_pair(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                           "VUID-vkCmdPushDescriptorSetWithTemplateKHR-commandBuffer-00366")};
        skip |= ValidatePipelineBindPoint(cb_state.get(), template_ci.pipelineBindPoint, func_name, bind_errors);

        if (template_ci.templateType != VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR) {
            skip |= LogError(cb_state->commandBuffer(), kVUID_Core_PushDescriptorUpdate_TemplateType,
                             "%s: descriptorUpdateTemplate %s was not created with flag "
                             "VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR.",
                             func_name, report_data->FormatHandle(descriptorUpdateTemplate).c_str());
        }
        if (template_ci.set != set) {
            skip |= LogError(cb_state->commandBuffer(), kVUID_Core_PushDescriptorUpdate_Template_SetMismatched,
                             "%s: descriptorUpdateTemplate %s created with set %" PRIu32
                             " does not match command parameter set %" PRIu32 ".",
                             func_name, report_data->FormatHandle(descriptorUpdateTemplate).c_str(), template_ci.set, set);
        }
        auto template_layout = Get<PIPELINE_LAYOUT_STATE>(template_ci.pipelineLayout);
        if (!IsPipelineLayoutSetCompat(set, layout_data.get(), template_layout.get())) {
            const LogObjectList objlist(cb_state->commandBuffer(), descriptorUpdateTemplate, template_ci.pipelineLayout, layout);
            skip |= LogError(objlist, kVUID_Core_PushDescriptorUpdate_Template_LayoutMismatched,
                             "%s: descriptorUpdateTemplate %s created with %s is incompatible with command parameter "
                             "%s for set %" PRIu32,
                             func_name, report_data->FormatHandle(descriptorUpdateTemplate).c_str(),
                             report_data->FormatHandle(template_ci.pipelineLayout).c_str(),
                             report_data->FormatHandle(layout).c_str(), set);
        }
    }

    if (dsl && template_state) {
        // Create an empty proxy in order to use the existing descriptor set update validation
        cvdescriptorset::DescriptorSet proxy_ds(VK_NULL_HANDLE, nullptr, dsl, 0, this);
        // Decode the template into a set of write updates
        cvdescriptorset::DecodedTemplateUpdate decoded_template(this, VK_NULL_HANDLE, template_state.get(), pData,
                                                                dsl->GetDescriptorSetLayout());
        // Validate the decoded update against the proxy_ds
        skip |= ValidatePushDescriptorsUpdate(&proxy_ds, static_cast<uint32_t>(decoded_template.desc_writes.size()),
                                              decoded_template.desc_writes.data(), func_name);
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

bool CoreChecks::PreCallValidateCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                          uint32_t discardRectangleCount,
                                                          const VkRect2D *pDiscardRectangles) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    // Minimal validation for command buffer state
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETDISCARDRECTANGLEEXT, VK_TRUE, nullptr, nullptr);
    skip |= ForbidInheritedViewportScissor(
        commandBuffer, cb_state.get(), "VUID-vkCmdSetDiscardRectangleEXT-viewportScissor2D-04788", CMD_SETDISCARDRECTANGLEEXT);
    for (uint32_t i = 0; i < discardRectangleCount; ++i) {
        if (pDiscardRectangles[i].offset.x < 0) {
            skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetDiscardRectangleEXT-x-00587",
                             "vkCmdSetDiscardRectangleEXT(): pDiscardRectangles[%" PRIu32 "].x (%" PRIi32 ") is negative.", i,
                             pDiscardRectangles[i].offset.x);
        }
        if (pDiscardRectangles[i].offset.y < 0) {
            skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetDiscardRectangleEXT-x-00587",
                             "vkCmdSetDiscardRectangleEXT(): pDiscardRectangles[%" PRIu32 "].y (%" PRIi32 ") is negative.", i,
                             pDiscardRectangles[i].offset.y);
        }
    }
    if (firstDiscardRectangle + discardRectangleCount > phys_dev_ext_props.discard_rectangle_props.maxDiscardRectangles) {
        skip |=
            LogError(cb_state->commandBuffer(), "VUID-vkCmdSetDiscardRectangleEXT-firstDiscardRectangle-00585",
                     "vkCmdSetDiscardRectangleEXT(): firstDiscardRectangle (%" PRIu32 ") + discardRectangleCount (%" PRIu32
                     ") is not less than VkPhysicalDeviceDiscardRectanglePropertiesEXT::maxDiscardRectangles (%" PRIu32 ").",
                     firstDiscardRectangle, discardRectangleCount, phys_dev_ext_props.discard_rectangle_props.maxDiscardRectangles);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                         const VkSampleLocationsInfoEXT *pSampleLocationsInfo) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    // Minimal validation for command buffer state
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETSAMPLELOCATIONSEXT, VK_TRUE, nullptr, nullptr);
    skip |= ValidateSampleLocationsInfo(pSampleLocationsInfo, "vkCmdSetSampleLocationsEXT");

    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    const auto *pipe = cb_state->lastBound[lv_bind_point].pipeline_state;
    if (pipe != nullptr) {
        // Check same error with different log messages
        const auto *multisample_state = pipe->MultisampleState();
        if (multisample_state == nullptr) {
            skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetSampleLocationsEXT-sampleLocationsPerPixel-01529",
                             "vkCmdSetSampleLocationsEXT(): pSampleLocationsInfo->sampleLocationsPerPixel must be equal to "
                             "rasterizationSamples, but the bound graphics pipeline was created without a multisample state");
        } else if (multisample_state->rasterizationSamples != pSampleLocationsInfo->sampleLocationsPerPixel) {
            skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetSampleLocationsEXT-sampleLocationsPerPixel-01529",
                             "vkCmdSetSampleLocationsEXT(): pSampleLocationsInfo->sampleLocationsPerPixel (%s) is not equal to "
                             "the last bound pipeline's rasterizationSamples (%s)",
                             string_VkSampleCountFlagBits(pSampleLocationsInfo->sampleLocationsPerPixel),
                             string_VkSampleCountFlagBits(multisample_state->rasterizationSamples));
        }
    }

    return skip;
}

bool CoreChecks::ValidateCreateSamplerYcbcrConversion(const char *func_name,
                                                      const VkSamplerYcbcrConversionCreateInfo *create_info) const {
    bool skip = false;
    const VkFormat conversion_format = create_info->format;

    // Need to check for external format conversion first as it allows for non-UNORM format
    bool external_format = false;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    const VkExternalFormatANDROID *ext_format_android = LvlFindInChain<VkExternalFormatANDROID>(create_info->pNext);
    if ((nullptr != ext_format_android) && (0 != ext_format_android->externalFormat)) {
        external_format = true;
        if (VK_FORMAT_UNDEFINED != create_info->format) {
            return LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-format-01904",
                            "%s: CreateInfo format is not VK_FORMAT_UNDEFINED while "
                            "there is a chained VkExternalFormatANDROID struct with a non-zero externalFormat.",
                            func_name);
        }
    }
#endif  // VK_USE_PLATFORM_ANDROID_KHR

    if ((external_format == false) && (FormatIsUNORM(conversion_format) == false)) {
        const char *vuid = IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)
                               ? "VUID-VkSamplerYcbcrConversionCreateInfo-format-04061"
                               : "VUID-VkSamplerYcbcrConversionCreateInfo-format-04060";
        skip |=
            LogError(device, vuid,
                     "%s: CreateInfo format (%s) is not an UNORM format and there is no external format conversion being created.",
                     func_name, string_VkFormat(conversion_format));
    }

    // Gets VkFormatFeatureFlags according to Sampler Ycbcr Conversion Format Features
    // (vkspec.html#potential-format-features)
    VkFormatFeatureFlags2KHR format_features = ~0ULL;
    if (conversion_format == VK_FORMAT_UNDEFINED) {
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        // only check for external format inside VK_FORMAT_UNDEFINED check to prevent unnecessary extra errors from no format
        // features being supported
        if (external_format == true) {
            auto it = ahb_ext_formats_map.find(ext_format_android->externalFormat);
            if (it != ahb_ext_formats_map.end()) {
                format_features = it->second;
            }
        }
#endif  // VK_USE_PLATFORM_ANDROID_KHR
    } else {
        format_features = GetPotentialFormatFeatures(conversion_format);
    }

    // Check all VUID that are based off of VkFormatFeatureFlags
    // These can't be in StatelessValidation due to needing possible External AHB state for feature support
    if (((format_features & VK_FORMAT_FEATURE_2_MIDPOINT_CHROMA_SAMPLES_BIT_KHR) == 0) &&
        ((format_features & VK_FORMAT_FEATURE_2_COSITED_CHROMA_SAMPLES_BIT_KHR) == 0)) {
        skip |= LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-format-01650",
                         "%s: Format %s does not support either VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT or "
                         "VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT",
                         func_name, string_VkFormat(conversion_format));
    }
    if ((format_features & VK_FORMAT_FEATURE_2_COSITED_CHROMA_SAMPLES_BIT_KHR) == 0) {
        if (FormatIsXChromaSubsampled(conversion_format) && create_info->xChromaOffset == VK_CHROMA_LOCATION_COSITED_EVEN) {
            skip |= LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01651",
                             "%s: Format %s does not support VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT so xChromaOffset can't "
                             "be VK_CHROMA_LOCATION_COSITED_EVEN",
                             func_name, string_VkFormat(conversion_format));
        }
        if (FormatIsYChromaSubsampled(conversion_format) && create_info->yChromaOffset == VK_CHROMA_LOCATION_COSITED_EVEN) {
            skip |= LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01651",
                             "%s: Format %s does not support VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT so yChromaOffset can't "
                             "be VK_CHROMA_LOCATION_COSITED_EVEN",
                             func_name, string_VkFormat(conversion_format));
        }
    }
    if ((format_features & VK_FORMAT_FEATURE_2_MIDPOINT_CHROMA_SAMPLES_BIT_KHR) == 0) {
        if (FormatIsXChromaSubsampled(conversion_format) && create_info->xChromaOffset == VK_CHROMA_LOCATION_MIDPOINT) {
            skip |= LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01652",
                             "%s: Format %s does not support VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT so xChromaOffset can't "
                             "be VK_CHROMA_LOCATION_MIDPOINT",
                             func_name, string_VkFormat(conversion_format));
        }
        if (FormatIsYChromaSubsampled(conversion_format) && create_info->yChromaOffset == VK_CHROMA_LOCATION_MIDPOINT) {
            skip |= LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01652",
                             "%s: Format %s does not support VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT so yChromaOffset can't "
                             "be VK_CHROMA_LOCATION_MIDPOINT",
                             func_name, string_VkFormat(conversion_format));
        }
    }
    if (((format_features & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT_KHR) ==
         0) &&
        (create_info->forceExplicitReconstruction == VK_TRUE)) {
        skip |= LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-forceExplicitReconstruction-01656",
                         "%s: Format %s does not support "
                         "VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT so "
                         "forceExplicitReconstruction must be VK_FALSE",
                         func_name, string_VkFormat(conversion_format));
    }
    if (((format_features & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT_KHR) == 0) &&
        (create_info->chromaFilter == VK_FILTER_LINEAR)) {
        skip |= LogError(device, "VUID-VkSamplerYcbcrConversionCreateInfo-chromaFilter-01657",
                         "%s: Format %s does not support VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT so "
                         "chromaFilter must not be VK_FILTER_LINEAR",
                         func_name, string_VkFormat(conversion_format));
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator,
                                                             VkSamplerYcbcrConversion *pYcbcrConversion) const {
    return ValidateCreateSamplerYcbcrConversion("vkCreateSamplerYcbcrConversion()", pCreateInfo);
}

bool CoreChecks::PreCallValidateCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                                const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                VkSamplerYcbcrConversion *pYcbcrConversion) const {
    return ValidateCreateSamplerYcbcrConversion("vkCreateSamplerYcbcrConversionKHR()", pCreateInfo);
}

bool CoreChecks::PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                              const VkAllocationCallbacks *pAllocator, VkSampler *pSampler) const {
    bool skip = false;

    auto num_samplers = Count<SAMPLER_STATE>();
    if (num_samplers >= phys_dev_props.limits.maxSamplerAllocationCount) {
        skip |= LogError(
            device, "VUID-vkCreateSampler-maxSamplerAllocationCount-04110",
            "vkCreateSampler(): Number of currently valid sampler objects (%zu) is not less than the maximum allowed (%u).",
            num_samplers, phys_dev_props.limits.maxSamplerAllocationCount);
    }

    const auto sampler_reduction = LvlFindInChain<VkSamplerReductionModeCreateInfo>(pCreateInfo->pNext);
    if (sampler_reduction && sampler_reduction->reductionMode != VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE) {
        if ((api_version >= VK_API_VERSION_1_2) && !enabled_features.core12.samplerFilterMinmax) {
            skip |= LogError(
                device, "VUID-VkSamplerCreateInfo-pNext-06726",
                "vkCreateSampler(): VkSamplerReductionModeCreateInfo is included in the pNext chain, samplerFilterMinmax is not "
                "enabled, and VkSamplerReductionModeCreateInfo::reductionMode (%s) != VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE.",
                string_VkSamplerReductionMode(sampler_reduction->reductionMode));
        } else if ((api_version < VK_API_VERSION_1_2) && !IsExtEnabled(device_extensions.vk_ext_sampler_filter_minmax)) {
            // NOTE: technically this VUID is only if the corresponding _feature_ is not enabled, and only if on api_version
            // >= 1.2, but there doesn't appear to be a similar VUID for when api_version < 1.2
            skip |= LogError(device, "VUID-VkSamplerCreateInfo-pNext-06726",
                             "vkCreateSampler(): sampler reduction mode is %s, but extension %s is not enabled.",
                             string_VkSamplerReductionMode(sampler_reduction->reductionMode),
                             VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
        }
    }
    if (enabled_features.core11.samplerYcbcrConversion == VK_TRUE) {
        const VkSamplerYcbcrConversionInfo *conversion_info = LvlFindInChain<VkSamplerYcbcrConversionInfo>(pCreateInfo->pNext);
        if (conversion_info != nullptr) {
            const VkSamplerYcbcrConversion sampler_ycbcr_conversion = conversion_info->conversion;
            auto ycbcr_state = Get<SAMPLER_YCBCR_CONVERSION_STATE>(sampler_ycbcr_conversion);
            if ((ycbcr_state->format_features &
                 VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT_KHR) == 0) {
                const VkFilter chroma_filter = ycbcr_state->chromaFilter;
                if (pCreateInfo->minFilter != chroma_filter) {
                    skip |= LogError(
                        device, "VUID-VkSamplerCreateInfo-minFilter-01645",
                        "VkCreateSampler: VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT is "
                        "not supported for SamplerYcbcrConversion's (%s) format %s so minFilter (%s) needs to be equal to "
                        "chromaFilter (%s)",
                        report_data->FormatHandle(sampler_ycbcr_conversion).c_str(), string_VkFormat(ycbcr_state->format),
                        string_VkFilter(pCreateInfo->minFilter), string_VkFilter(chroma_filter));
                }
                if (pCreateInfo->magFilter != chroma_filter) {
                    skip |= LogError(
                        device, "VUID-VkSamplerCreateInfo-minFilter-01645",
                        "VkCreateSampler: VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT is "
                        "not supported for SamplerYcbcrConversion's (%s) format %s so minFilter (%s) needs to be equal to "
                        "chromaFilter (%s)",
                        report_data->FormatHandle(sampler_ycbcr_conversion).c_str(), string_VkFormat(ycbcr_state->format),
                        string_VkFilter(pCreateInfo->minFilter), string_VkFilter(chroma_filter));
                }
            }
            // At this point there is a known sampler YCbCr conversion enabled
            if (sampler_reduction) {
                if (sampler_reduction->reductionMode != VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE) {
                    skip |= LogError(device, "VUID-VkSamplerCreateInfo-None-01647",
                                     "A sampler YCbCr Conversion is being used creating this sampler so the sampler reduction mode "
                                     "must be VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE.");
                }
            }
        }
    }

    if (pCreateInfo->borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT ||
        pCreateInfo->borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT) {
        if (!enabled_features.custom_border_color_features.customBorderColors) {
            skip |=
                LogError(device, "VUID-VkSamplerCreateInfo-customBorderColors-04085",
                         "vkCreateSampler(): A custom border color was specified without enabling the custom border color feature");
        }
        auto custom_create_info = LvlFindInChain<VkSamplerCustomBorderColorCreateInfoEXT>(pCreateInfo->pNext);
        if (custom_create_info) {
            if (custom_create_info->format == VK_FORMAT_UNDEFINED &&
                !enabled_features.custom_border_color_features.customBorderColorWithoutFormat) {
                skip |= LogError(device, "VUID-VkSamplerCustomBorderColorCreateInfoEXT-format-04014",
                                 "vkCreateSampler(): A custom border color was specified as VK_FORMAT_UNDEFINED without the "
                                 "customBorderColorWithoutFormat feature being enabled");
            }
        }
        if (custom_border_color_sampler_count >= phys_dev_ext_props.custom_border_color_props.maxCustomBorderColorSamplers) {
            skip |= LogError(device, "VUID-VkSamplerCreateInfo-None-04012",
                             "vkCreateSampler(): Creating a sampler with a custom border color will exceed the "
                             "maxCustomBorderColorSamplers limit of %d",
                             phys_dev_ext_props.custom_border_color_props.maxCustomBorderColorSamplers);
        }
    }

    if (IsExtEnabled(device_extensions.vk_khr_portability_subset)) {
        if ((VK_FALSE == enabled_features.portability_subset_features.samplerMipLodBias) && pCreateInfo->mipLodBias != 0) {
            skip |= LogError(device, "VUID-VkSamplerCreateInfo-samplerMipLodBias-04467",
                             "vkCreateSampler (portability error): mip LOD bias not supported.");
        }
    }

    // If any of addressModeU, addressModeV or addressModeW are VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE, the
    // VK_KHR_sampler_mirror_clamp_to_edge extension or promoted feature must be enabled
    if ((device_extensions.vk_khr_sampler_mirror_clamp_to_edge != kEnabledByCreateinfo) &&
        (enabled_features.core12.samplerMirrorClampToEdge == VK_FALSE)) {
        // Use 'else' because getting 3 large error messages is redundant and assume developer, if set all 3, will notice and fix
        // all at once
        if (pCreateInfo->addressModeU == VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE) {
            skip |=
                LogError(device, "VUID-VkSamplerCreateInfo-addressModeU-01079",
                         "vkCreateSampler(): addressModeU is set to VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE but the "
                         "VK_KHR_sampler_mirror_clamp_to_edge extension or samplerMirrorClampToEdge feature has not been enabled.");
        } else if (pCreateInfo->addressModeV == VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE) {
            skip |=
                LogError(device, "VUID-VkSamplerCreateInfo-addressModeU-01079",
                         "vkCreateSampler(): addressModeV is set to VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE but the "
                         "VK_KHR_sampler_mirror_clamp_to_edge extension or samplerMirrorClampToEdge feature has not been enabled.");
        } else if (pCreateInfo->addressModeW == VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE) {
            skip |=
                LogError(device, "VUID-VkSamplerCreateInfo-addressModeU-01079",
                         "vkCreateSampler(): addressModeW is set to VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE but the "
                         "VK_KHR_sampler_mirror_clamp_to_edge extension or samplerMirrorClampToEdge feature has not been enabled.");
        }
    }

    if ((pCreateInfo->flags & VK_SAMPLER_CREATE_NON_SEAMLESS_CUBE_MAP_BIT_EXT) && (!enabled_features.non_seamless_cube_map_features.nonSeamlessCubeMap)) {
        skip |= LogError(device, "VUID-VkSamplerCreateInfo-nonSeamlessCubeMap-06788",
                         "vkCreateSampler(): flags contains VK_SAMPLER_CREATE_NON_SEAMLESS_CUBE_MAP_BIT_EXT but the "
                         "VK_EXT_non_seamless_cube_map feature has not been enabled.");
    }

    if ((pCreateInfo->flags & VK_SAMPLER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT) &&
        !enabled_features.descriptor_buffer_features.descriptorBufferCaptureReplay) {
        skip |= LogError(
            device, "VUID-VkSamplerCreateInfo-flags-08110",
            "vkCreateSampler(): the descriptorBufferCaptureReplay device feature is disabled: Samplers cannot be created with "
            "the VK_SAMPLER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT.");
    }

    auto opaque_capture_descriptor_buffer = LvlFindInChain<VkOpaqueCaptureDescriptorDataCreateInfoEXT>(pCreateInfo->pNext);
    if (opaque_capture_descriptor_buffer && !(pCreateInfo->flags & VK_SAMPLER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
        skip |= LogError(device, "VUID-VkSamplerCreateInfo-pNext-08111",
                         "vkCreateSampler(): VkOpaqueCaptureDescriptorDataCreateInfoEXT is in pNext chain, but "
                         "VK_SAMPLER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT is not set.");
    }

    return skip;
}

VkResult CoreChecks::CoreLayerCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator,
                                                       VkValidationCacheEXT *pValidationCache) {
    *pValidationCache = ValidationCache::Create(pCreateInfo);
    return *pValidationCache ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED;
}

void CoreChecks::CoreLayerDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                                    const VkAllocationCallbacks *pAllocator) {
    delete CastFromHandle<ValidationCache *>(validationCache);
}

VkResult CoreChecks::CoreLayerGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t *pDataSize,
                                                        void *pData) {
    size_t in_size = *pDataSize;
    CastFromHandle<ValidationCache *>(validationCache)->Write(pDataSize, pData);
    return (pData && *pDataSize != in_size) ? VK_INCOMPLETE : VK_SUCCESS;
}

VkResult CoreChecks::CoreLayerMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                                       const VkValidationCacheEXT *pSrcCaches) {
    bool skip = false;
    auto dst = CastFromHandle<ValidationCache *>(dstCache);
    VkResult result = VK_SUCCESS;
    for (uint32_t i = 0; i < srcCacheCount; i++) {
        auto src = CastFromHandle<const ValidationCache *>(pSrcCaches[i]);
        if (src == dst) {
            skip |= LogError(device, "VUID-vkMergeValidationCachesEXT-dstCache-01536",
                             "vkMergeValidationCachesEXT: dstCache (0x%" PRIx64 ") must not appear in pSrcCaches array.",
                             HandleToUint64(dstCache));
            result = VK_ERROR_VALIDATION_FAILED_EXT;
        }
        if (!skip) {
            dst->Merge(src);
        }
    }

    return result;
}

bool CoreChecks::ValidateCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask, CMD_TYPE cmd_type) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state_ptr) {
        return skip;
    }
    const CMD_BUFFER_STATE &cb_state = *cb_state_ptr;
    const LogObjectList objlist(commandBuffer);
    skip |= ValidateExtendedDynamicState(cb_state, cmd_type, VK_TRUE, nullptr, nullptr);
    skip |= ValidateDeviceMaskToPhysicalDeviceCount(deviceMask, objlist, "VUID-vkCmdSetDeviceMask-deviceMask-00108");
    skip |= ValidateDeviceMaskToZero(deviceMask, objlist, "VUID-vkCmdSetDeviceMask-deviceMask-00109");
    skip |= ValidateDeviceMaskToCommandBuffer(cb_state, deviceMask, objlist, "VUID-vkCmdSetDeviceMask-deviceMask-00110");
    if (cb_state.activeRenderPass) {
        skip |= ValidateDeviceMaskToRenderPass(cb_state, deviceMask, "VUID-vkCmdSetDeviceMask-deviceMask-00111");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) const {
    return ValidateCmdSetDeviceMask(commandBuffer, deviceMask, CMD_SETDEVICEMASK);
}

bool CoreChecks::PreCallValidateCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask) const {
    return ValidateCmdSetDeviceMask(commandBuffer, deviceMask, CMD_SETDEVICEMASKKHR);
}

bool CoreChecks::ValidateGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t *pValue,
                                                  const char *apiName) const {
    bool skip = false;
    auto semaphore_state = Get<SEMAPHORE_STATE>(semaphore);
    if (semaphore_state && semaphore_state->type != VK_SEMAPHORE_TYPE_TIMELINE) {
        skip |= LogError(semaphore, "VUID-vkGetSemaphoreCounterValue-semaphore-03255",
                         "%s(): semaphore %s must be of VK_SEMAPHORE_TYPE_TIMELINE type", apiName,
                         report_data->FormatHandle(semaphore).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t *pValue) const {
    return ValidateGetSemaphoreCounterValue(device, semaphore, pValue, "vkGetSemaphoreCounterValueKHR");
}
bool CoreChecks::PreCallValidateGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t *pValue) const {
    return ValidateGetSemaphoreCounterValue(device, semaphore, pValue, "vkGetSemaphoreCounterValue");
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

bool CoreChecks::PreCallValidateReleaseProfilingLockKHR(VkDevice device) const {
    bool skip = false;

    if (!performance_lock_acquired) {
        skip |= LogError(device, "VUID-vkReleaseProfilingLockKHR-device-03235",
                         "vkReleaseProfilingLockKHR(): The profiling lock of device must have been held via a previous successful "
                         "call to vkAcquireProfilingLockKHR.");
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfoEXT *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator,
                                                         VkPrivateDataSlotEXT *pPrivateDataSlot) const {
    bool skip = false;
    if (!enabled_features.core13.privateData) {
        skip |= LogError(device, "VUID-vkCreatePrivateDataSlot-privateData-04564",
                         "vkCreatePrivateDataSlotEXT(): The privateData feature must be enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo *pCreateInfo,
                                                      const VkAllocationCallbacks *pAllocator,
                                                      VkPrivateDataSlot *pPrivateDataSlot) const {
    bool skip = false;
    if (!enabled_features.core13.privateData) {
        skip |= LogError(device, "VUID-vkCreatePrivateDataSlot-privateData-04564",
                         "vkCreatePrivateDataSlot(): The privateData feature must be enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void *pCheckpointMarker) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETCHECKPOINTNV, VK_TRUE, nullptr, nullptr);
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
                             "%s: pBuffers[%" PRIu32 "] (%s)"
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

        skip |=
            ValidateMemoryIsBoundToBuffer(buffer_state.get(), cmd_name, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pBuffers-02364");
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
                                 "%s: pCounterBuffers[%" PRIu32 "](%s) is not large enough to hold 4 bytes at pCounterBufferOffsets[%" PRIu32 "](0x%" PRIx64 ").",
                             cmd_name, i, report_data->FormatHandle(pCounterBuffers[i]).c_str(), i, pCounterBufferOffsets[i]);
                }

                if ((buffer_state->createInfo.usage & VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT) == 0) {
                    skip |= LogError(buffer_state->buffer(), "VUID-vkCmdBeginTransformFeedbackEXT-pCounterBuffers-02372",
                                     "%s: pCounterBuffers[%" PRIu32 "] (%s) was not created with the VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT flag.",
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
                                 "%s: pCounterBuffers[%" PRIu32 "](%s) is not large enough to hold 4 bytes at pCounterBufferOffsets[%" PRIu32 "](0x%" PRIx64 ").",
                                 cmd_name, i, report_data->FormatHandle(pCounterBuffers[i]).c_str(), i, pCounterBufferOffsets[i]);
                }

                if ((buffer_state->createInfo.usage & VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT) == 0) {
                    skip |= LogError(buffer_state->buffer(), "VUID-vkCmdEndTransformFeedbackEXT-pCounterBuffers-02380",
                                     "%s: pCounterBuffers[%" PRIu32 "] (%s) was not created with the VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT flag.",
                                     cmd_name, i, report_data->FormatHandle(pCounterBuffers[i]).c_str());
                }
            }
        }
    }

    return skip;
}

// Used for all vkCmdSet* functions
// Some calls are behind a feature bit that needs to be enabled
bool CoreChecks::ValidateExtendedDynamicState(const CMD_BUFFER_STATE &cb_state, const CMD_TYPE cmd_type, VkBool32 feature,
                                              const char *vuid, const char *feature_name) const {
    bool skip = false;
    skip |= ValidateCmd(cb_state, cmd_type);

    if (!feature) {
        const char *func_name = CommandTypeString(cmd_type);
        skip |= LogError(cb_state.Handle(), vuid, "%s(): %s feature is not enabled.", func_name, feature_name);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETLOGICOPEXT,
                                        enabled_features.extended_dynamic_state2_features.extendedDynamicState2LogicOp,
                                        "VUID-vkCmdSetLogicOpEXT-None-04867", "extendedDynamicState2LogicOp");
}

bool CoreChecks::PreCallValidateCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |=
        ValidateExtendedDynamicState(*cb_state, CMD_SETPATCHCONTROLPOINTSEXT,
                                     enabled_features.extended_dynamic_state2_features.extendedDynamicState2PatchControlPoints,
                                     "VUID-vkCmdSetPatchControlPointsEXT-None-04873", "extendedDynamicState2PatchControlPoints");

    if (patchControlPoints > phys_dev_props.limits.maxTessellationPatchSize) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetPatchControlPointsEXT-patchControlPoints-04874",
                         "vkCmdSetPatchControlPointsEXT: The value of patchControlPoints must be less than "
                         "VkPhysicalDeviceLimits::maxTessellationPatchSize");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer,
                                                                 VkBool32 rasterizerDiscardEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETRASTERIZERDISCARDENABLEEXT,
                                        enabled_features.extended_dynamic_state2_features.extendedDynamicState2,
                                        "VUID-vkCmdSetRasterizerDiscardEnable-None-04871", "extendedDynamicState2");
}

bool CoreChecks::PreCallValidateCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer,
                                                              VkBool32 rasterizerDiscardEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETRASTERIZERDISCARDENABLE, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHBIASENABLEEXT,
                                        enabled_features.extended_dynamic_state2_features.extendedDynamicState2,
                                        "VUID-vkCmdSetDepthBiasEnable-None-04872", "extendedDynamicState2");
}

bool CoreChecks::PreCallValidateCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHBIASENABLE, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer,
                                                                VkBool32 primitiveRestartEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETPRIMITIVERESTARTENABLEEXT,
                                        enabled_features.extended_dynamic_state2_features.extendedDynamicState2,
                                        "VUID-vkCmdSetPrimitiveRestartEnable-None-04866", "extendedDynamicState2");
}

bool CoreChecks::PreCallValidateCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETPRIMITIVERESTARTENABLE, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETCULLMODEEXT,
                                        enabled_features.extended_dynamic_state_features.extendedDynamicState,
                                        "VUID-vkCmdSetCullMode-None-03384", "extendedDynamicState");
}

bool CoreChecks::PreCallValidateCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETCULLMODE, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETFRONTFACEEXT,
                                        enabled_features.extended_dynamic_state_features.extendedDynamicState,
                                        "VUID-vkCmdSetFrontFace-None-03383", "extendedDynamicState");
}

bool CoreChecks::PreCallValidateCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETFRONTFACE, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer,
                                                           VkPrimitiveTopology primitiveTopology) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETPRIMITIVETOPOLOGYEXT,
                                        enabled_features.extended_dynamic_state_features.extendedDynamicState,
                                        "VUID-vkCmdSetPrimitiveTopology-None-03347", "extendedDynamicState");
}

bool CoreChecks::PreCallValidateCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer,
                                                        VkPrimitiveTopology primitiveTopology) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETPRIMITIVETOPOLOGY, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                           const VkViewport *pViewports) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip = ValidateExtendedDynamicState(*cb_state, CMD_SETVIEWPORTWITHCOUNTEXT,
                                        enabled_features.extended_dynamic_state_features.extendedDynamicState,
                                        "VUID-vkCmdSetViewportWithCount-None-03393", "extendedDynamicState");
    skip |= ForbidInheritedViewportScissor(commandBuffer, cb_state.get(), "VUID-vkCmdSetViewportWithCount-commandBuffer-04819",
                                           CMD_SETVIEWPORTWITHCOUNTEXT);

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                        const VkViewport *pViewports) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip = ValidateExtendedDynamicState(*cb_state, CMD_SETVIEWPORTWITHCOUNT, VK_TRUE, nullptr, nullptr);
    skip |= ForbidInheritedViewportScissor(commandBuffer, cb_state.get(), "VUID-vkCmdSetViewportWithCount-commandBuffer-04819",
                                           CMD_SETVIEWPORTWITHCOUNT);

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                          const VkRect2D *pScissors) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip = ValidateExtendedDynamicState(*cb_state, CMD_SETSCISSORWITHCOUNTEXT,
                                        enabled_features.extended_dynamic_state_features.extendedDynamicState,
                                        "VUID-vkCmdSetScissorWithCount-None-03396", "extendedDynamicState");
    skip |= ForbidInheritedViewportScissor(commandBuffer, cb_state.get(), "VUID-vkCmdSetScissorWithCount-commandBuffer-04820",
                                           CMD_SETSCISSORWITHCOUNTEXT);

    return skip;
}

bool CoreChecks::PreCallValidateCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                       const VkRect2D *pScissors) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip = ValidateExtendedDynamicState(*cb_state, CMD_SETSCISSORWITHCOUNT, VK_TRUE, nullptr, nullptr);
    skip |= ForbidInheritedViewportScissor(commandBuffer, cb_state.get(), "VUID-vkCmdSetScissorWithCount-commandBuffer-04820",
                                           CMD_SETSCISSORWITHCOUNT);

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
            skip |= ValidateBufferUsageFlags(buffer_state.get(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, true,
                                             "VUID-vkCmdBindVertexBuffers2-pBuffers-03359", api_call,
                                             "VK_BUFFER_USAGE_VERTEX_BUFFER_BIT");
            skip |= ValidateMemoryIsBoundToBuffer(buffer_state.get(), api_call, "VUID-vkCmdBindVertexBuffers2-pBuffers-03360");

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

bool CoreChecks::PreCallValidateCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHTESTENABLEEXT,
                                        enabled_features.extended_dynamic_state_features.extendedDynamicState,
                                        "VUID-vkCmdSetDepthTestEnable-None-03352", "extendedDynamicState");
}

bool CoreChecks::PreCallValidateCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHTESTENABLE, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHWRITEENABLEEXT,
                                        enabled_features.extended_dynamic_state_features.extendedDynamicState,
                                        "VUID-vkCmdSetDepthWriteEnable-None-03354", "extendedDynamicState");
}

bool CoreChecks::PreCallValidateCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHWRITEENABLE, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHCOMPAREOPEXT,
                                        enabled_features.extended_dynamic_state_features.extendedDynamicState,
                                        "VUID-vkCmdSetDepthCompareOp-None-03353", "extendedDynamicState");
}

bool CoreChecks::PreCallValidateCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHCOMPAREOP, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer,
                                                               VkBool32 depthBoundsTestEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHBOUNDSTESTENABLEEXT,
                                        enabled_features.extended_dynamic_state_features.extendedDynamicState,
                                        "VUID-vkCmdSetDepthBoundsTestEnable-None-03349", "extendedDynamicState");
}

bool CoreChecks::PreCallValidateCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHBOUNDSTESTENABLE, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETSTENCILTESTENABLEEXT,
                                        enabled_features.extended_dynamic_state_features.extendedDynamicState,
                                        "VUID-vkCmdSetStencilTestEnable-None-03350", "extendedDynamicState");
}

bool CoreChecks::PreCallValidateCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETSTENCILTESTENABLE, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                                   VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETSTENCILOPEXT,
                                        enabled_features.extended_dynamic_state_features.extendedDynamicState,
                                        "VUID-vkCmdSetStencilOp-None-03351", "extendedDynamicState");
}

bool CoreChecks::PreCallValidateCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                                VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETSTENCILOP, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer,
                                                                  VkTessellationDomainOrigin domainOrigin) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETTESSELLATIONDOMAINORIGINEXT,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3TessellationDomainOrigin,
        "VUID-vkCmdSetTessellationDomainOriginEXT-extendedDynamicState3TessellationDomainOrigin-07444",
        "extendedDynamicState3TessellationDomainOrigin");
}

bool CoreChecks::PreCallValidateCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHCLAMPENABLEEXT,
                                         enabled_features.extended_dynamic_state3_features.extendedDynamicState3DepthClampEnable,
                                         "VUID-vkCmdSetDepthClampEnableEXT-extendedDynamicState3DepthClampEnable-07448",
                                         "extendedDynamicState3DepthClampEnable");
    if (depthClampEnable != VK_FALSE && !enabled_features.core.depthClamp) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetDepthClampEnableEXT-depthClamp-07449",
                         "vkCmdSetDepthClampEnableEXT(): depthClampEnable is VK_TRUE but the depthClamp feature is not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, CMD_SETPOLYGONMODEEXT, enabled_features.extended_dynamic_state3_features.extendedDynamicState3PolygonMode,
        "VUID-vkCmdSetPolygonModeEXT-extendedDynamicState3PolygonMode-07422", "extendedDynamicState3PolygonMode");
    if ((polygonMode == VK_POLYGON_MODE_LINE || polygonMode == VK_POLYGON_MODE_POINT) && !enabled_features.core.fillModeNonSolid) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetPolygonModeEXT-fillModeNonSolid-07424",
                         "vkCmdSetPolygonModeEXT(): polygonMode is %s but the "
                         "fillModeNonSolid feature is not enabled.",
                         string_VkPolygonMode(polygonMode));
    } else if (polygonMode == VK_POLYGON_MODE_FILL_RECTANGLE_NV && !IsExtEnabled(device_extensions.vk_nv_fill_rectangle)) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetPolygonModeEXT-polygonMode-07425",
                         "vkCmdSetPolygonModeEXT(): polygonMode is VK_POLYGON_MODE_FILL_RECTANGLE_NV but the VK_NV_fill_rectangle "
                         "extension is not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer,
                                                              VkSampleCountFlagBits rasterizationSamples) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETRASTERIZATIONSAMPLESEXT,
                                        enabled_features.extended_dynamic_state3_features.extendedDynamicState3RasterizationSamples,
                                        "VUID-vkCmdSetRasterizationSamplesEXT-extendedDynamicState3RasterizationSamples-07414",
                                        "extendedDynamicState3RasterizationSamples");
}

bool CoreChecks::PreCallValidateCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                                    const VkSampleMask *pSampleMask) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETSAMPLEMASKEXT, enabled_features.extended_dynamic_state3_features.extendedDynamicState3SampleMask,
        "VUID-vkCmdSetSampleMaskEXT-extendedDynamicState3SampleMask-07342", "extendedDynamicState3SampleMask");
}

bool CoreChecks::PreCallValidateCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer,
                                                               VkBool32 alphaToCoverageEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETALPHATOCOVERAGEENABLEEXT,
                                     enabled_features.extended_dynamic_state3_features.extendedDynamicState3AlphaToCoverageEnable,
                                     "VUID-vkCmdSetAlphaToCoverageEnableEXT-extendedDynamicState3AlphaToCoverageEnable-07343",
                                     "extendedDynamicState3AlphaToCoverageEnable");
}

bool CoreChecks::PreCallValidateCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETALPHATOONEENABLEEXT,
                                        enabled_features.extended_dynamic_state3_features.extendedDynamicState3AlphaToOneEnable,
                                        "VUID-vkCmdSetAlphaToOneEnableEXT-extendedDynamicState3AlphaToOneEnable-07345",
                                        "extendedDynamicState3AlphaToOneEnable");
    if (alphaToOneEnable != VK_FALSE && !enabled_features.core.alphaToOne) {
        skip |= LogError(
            cb_state->Handle(), "VUID-vkCmdSetAlphaToOneEnableEXT-alphaToOne-07607",
            "vkCmdSetAlphaToOneEnableEXT(): alphaToOneEnable is VK_TRUE but the alphaToOne feature is not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, CMD_SETLOGICOPENABLEEXT, enabled_features.extended_dynamic_state3_features.extendedDynamicState3LogicOpEnable,
        "VUID-vkCmdSetLogicOpEnableEXT-extendedDynamicState3LogicOpEnable-07365", "extendedDynamicState3LogicOpEnable");
    if (logicOpEnable != VK_FALSE && !enabled_features.core.logicOp) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetLogicOpEnableEXT-logicOp-07366",
                         "vkCmdSetLogicOpEnableEXT(): logicOpEnable is VK_TRUE but the logicOp feature is not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                          uint32_t attachmentCount, const VkBool32 *pColorBlendEnables) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETCOLORBLENDENABLEEXT,
                                        enabled_features.extended_dynamic_state3_features.extendedDynamicState3ColorBlendEnable,
                                        "VUID-vkCmdSetColorBlendEnableEXT-extendedDynamicState3ColorBlendEnable-07355",
                                        "extendedDynamicState3ColorBlendEnable");
}

bool CoreChecks::PreCallValidateCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                            uint32_t attachmentCount,
                                                            const VkColorBlendEquationEXT *pColorBlendEquations) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETCOLORBLENDEQUATIONEXT,
                                         enabled_features.extended_dynamic_state3_features.extendedDynamicState3ColorBlendEquation,
                                         "VUID-vkCmdSetColorBlendEquationEXT-extendedDynamicState3ColorBlendEquation-07356",
                                         "extendedDynamicState3ColorBlendEquation");
    for (uint32_t attachment = 0U; attachment < attachmentCount; ++attachment) {
        VkColorBlendEquationEXT const &equation = pColorBlendEquations[attachment];
        if (!enabled_features.core.dualSrcBlend) {
            if (IsSecondaryColorInputBlendFactor(equation.srcColorBlendFactor)) {
                skip |= LogError(cb_state->Handle(), "VUID-VkColorBlendEquationEXT-dualSrcBlend-07357",
                                 "vkCmdSetColorBlendEquationEXT(): pColorBlendEquations[%u].srcColorBlendFactor is %s but the "
                                 "dualSrcBlend feature is not enabled.",
                                 attachment, string_VkBlendFactor(equation.srcColorBlendFactor));
            }
            if (IsSecondaryColorInputBlendFactor(equation.dstColorBlendFactor)) {
                skip |= LogError(cb_state->Handle(), "VUID-VkColorBlendEquationEXT-dualSrcBlend-07358",
                                 "vkCmdSetColorBlendEquationEXT(): pColorBlendEquations[%u].dstColorBlendFactor is %s but the "
                                 "dualSrcBlend feature is not enabled.",
                                 attachment, string_VkBlendFactor(equation.dstColorBlendFactor));
            }
            if (IsSecondaryColorInputBlendFactor(equation.srcAlphaBlendFactor)) {
                skip |= LogError(cb_state->Handle(), "VUID-VkColorBlendEquationEXT-dualSrcBlend-07359",
                                 "vkCmdSetColorBlendEquationEXT(): pColorBlendEquations[%u].srcAlphaBlendFactor is %s but the "
                                 "dualSrcBlend feature is not enabled.",
                                 attachment, string_VkBlendFactor(equation.srcAlphaBlendFactor));
            }
            if (IsSecondaryColorInputBlendFactor(equation.dstAlphaBlendFactor)) {
                skip |= LogError(cb_state->Handle(), "VUID-VkColorBlendEquationEXT-dualSrcBlend-07360",
                                 "vkCmdSetColorBlendEquationEXT(): pColorBlendEquations[%u].dstAlphaBlendFactor is %s but the "
                                 "dualSrcBlend feature is not enabled.",
                                 attachment, string_VkBlendFactor(equation.dstAlphaBlendFactor));
            }
        }
        if (IsAdvanceBlendOperation(equation.colorBlendOp) || IsAdvanceBlendOperation(equation.alphaBlendOp)) {
            skip |= LogError(cb_state->Handle(), "VUID-VkColorBlendEquationEXT-colorBlendOp-07361",
                             "vkCmdSetColorBlendEquationEXT(): pColorBlendEquations[%u].colorBlendOp and "
                             "pColorBlendEquations[%u].alphaBlendOp must not be an advanced blending operation.",
                             attachment, attachment);
        }
        if (IsExtEnabled(device_extensions.vk_khr_portability_subset) &&
            !enabled_features.portability_subset_features.constantAlphaColorBlendFactors) {
            if (equation.srcColorBlendFactor == VK_BLEND_FACTOR_CONSTANT_ALPHA ||
                equation.srcColorBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA) {
                skip |= LogError(cb_state->Handle(), "VUID-VkColorBlendEquationEXT-constantAlphaColorBlendFactors-07362",
                                 "vkCmdSetColorBlendEquationEXT(): pColorBlendEquations[%u].srcColorBlendFactor must not be %s "
                                 "when constantAlphaColorBlendFactors is not supported.",
                                 attachment, string_VkBlendFactor(equation.srcColorBlendFactor));
            }
            if (equation.dstColorBlendFactor == VK_BLEND_FACTOR_CONSTANT_ALPHA ||
                equation.dstColorBlendFactor == VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA) {
                skip |= LogError(cb_state->Handle(), "VUID-VkColorBlendEquationEXT-constantAlphaColorBlendFactors-07363",
                                 "vkCmdSetColorBlendEquationEXT(): pColorBlendEquations[%u].dstColorBlendFactor must not be %s "
                                 "constantAlphaColorBlendFactors is not supported.",
                                 attachment, string_VkBlendFactor(equation.dstColorBlendFactor));
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                        uint32_t attachmentCount,
                                                        const VkColorComponentFlags *pColorWriteMasks) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETCOLORWRITEMASKEXT, enabled_features.extended_dynamic_state3_features.extendedDynamicState3ColorWriteMask,
        "VUID-vkCmdSetColorWriteMaskEXT-extendedDynamicState3ColorWriteMask-07364", "extendedDynamicState3ColorWriteMask");
}

bool CoreChecks::PreCallValidateCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETRASTERIZATIONSTREAMEXT,
                                         enabled_features.extended_dynamic_state3_features.extendedDynamicState3RasterizationStream,
                                         "VUID-vkCmdSetRasterizationStreamEXT-extendedDynamicState3RasterizationStream-07410",
                                         "extendedDynamicState3RasterizationStream");
    if (!enabled_features.transform_feedback_features.transformFeedback) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetRasterizationStreamEXT-transformFeedback-07411",
                         "vkCmdSetRasterizationStreamEXT(): the transformFeedback feature is not enabled.");
    }
    if (rasterizationStream >= phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetRasterizationStreamEXT-rasterizationStream-07412",
                         "vkCmdSetRasterizationStreamEXT(): rasterizationStream (%" PRIu32
                         ") must be less than maxTransformFeedbackStreams (%" PRIu32 ").",
                         rasterizationStream, phys_dev_ext_props.transform_feedback_props.maxTransformFeedbackStreams);
    }
    if (rasterizationStream != 0U &&
        phys_dev_ext_props.transform_feedback_props.transformFeedbackRasterizationStreamSelect == VK_FALSE) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetRasterizationStreamEXT-rasterizationStream-07413",
                         "vkCmdSetRasterizationStreamEXT(): rasterizationStream is non-zero but "
                         "transformFeedbackRasterizationStreamSelect is not supported.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetConservativeRasterizationModeEXT(
    VkCommandBuffer commandBuffer, VkConservativeRasterizationModeEXT conservativeRasterizationMode) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETCONSERVATIVERASTERIZATIONMODEEXT,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3ConservativeRasterizationMode,
        "VUID-vkCmdSetConservativeRasterizationModeEXT-extendedDynamicState3ConservativeRasterizationMode-07426",
        "extendedDynamicState3ConservativeRasterizationMode");
}

bool CoreChecks::PreCallValidateCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                                          float extraPrimitiveOverestimationSize) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, CMD_SETEXTRAPRIMITIVEOVERESTIMATIONSIZEEXT,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3ExtraPrimitiveOverestimationSize,
        "VUID-vkCmdSetExtraPrimitiveOverestimationSizeEXT-extendedDynamicState3ExtraPrimitiveOverestimationSize-07427",
        "extendedDynamicState3ExtraPrimitiveOverestimationSize");
    if (extraPrimitiveOverestimationSize < 0.0f ||
        extraPrimitiveOverestimationSize >
            phys_dev_ext_props.conservative_rasterization_props.maxExtraPrimitiveOverestimationSize) {
        skip |=
            LogError(cb_state->Handle(), "VUID-vkCmdSetExtraPrimitiveOverestimationSizeEXT-extraPrimitiveOverestimationSize-07428",
                     "vkCmdSetExtraPrimitiveOverestimationSizeEXT(): extraPrimitiveOverestimationSize (%f) must be less then zero "
                     "or greater "
                     "than maxExtraPrimitiveOverestimationSize (%f).",
                     extraPrimitiveOverestimationSize,
                     phys_dev_ext_props.conservative_rasterization_props.maxExtraPrimitiveOverestimationSize);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETDEPTHCLIPENABLEEXT,
                                         enabled_features.extended_dynamic_state3_features.extendedDynamicState3DepthClipEnable,
                                         "VUID-vkCmdSetDepthClipEnableEXT-extendedDynamicState3DepthClipEnable-07450",
                                         "extendedDynamicState3DepthClipEnable");
    if (!enabled_features.depth_clip_enable_features.depthClipEnable) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetDepthClipEnableEXT-depthClipEnable-07451",
                         "vkCmdSetDepthClipEnableEXT(): the depthClipEnable feature is not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer,
                                                               VkBool32 sampleLocationsEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETSAMPLELOCATIONSENABLEEXT,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3SampleLocationsEnable,
        "VUID-vkCmdSetSampleLocationsEnableEXT-extendedDynamicState3SampleLocationsEnable-07415",
        "extendedDynamicState3SampleLocationsEnable");
}

bool CoreChecks::PreCallValidateCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                            uint32_t attachmentCount,
                                                            const VkColorBlendAdvancedEXT *pColorBlendAdvanced) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETCOLORBLENDADVANCEDEXT,
                                         enabled_features.extended_dynamic_state3_features.extendedDynamicState3ColorBlendAdvanced,
                                         "VUID-vkCmdSetColorBlendAdvancedEXT-extendedDynamicState3ColorBlendAdvanced-07504",
                                         "extendedDynamicState3ColorBlendAdvanced");
    for (uint32_t attachment = 0U; attachment < attachmentCount; ++attachment) {
        VkColorBlendAdvancedEXT const &advanced = pColorBlendAdvanced[attachment];
        // VUID-VkColorBlendAdvancedEXT-srcPremultiplied-07505
        if (advanced.srcPremultiplied != VK_FALSE &&
            !phys_dev_ext_props.blend_operation_advanced_props.advancedBlendNonPremultipliedSrcColor) {
            skip |= LogError(cb_state->Handle(), "VUID-VkColorBlendAdvancedEXT-srcPremultiplied-07505",
                             "vkCmdSetColorBlendAdvancedEXT(): pColorBlendAdvanced[%u].srcPremultiplied must not be VK_TRUE when "
                             "advancedBlendNonPremultipliedSrcColor is not supported.",
                             attachment);
        }
        // VUID-VkColorBlendAdvancedEXT-dstPremultiplied-07506
        if (advanced.dstPremultiplied != VK_FALSE &&
            !phys_dev_ext_props.blend_operation_advanced_props.advancedBlendNonPremultipliedDstColor) {
            skip |= LogError(cb_state->Handle(), "VUID-VkColorBlendAdvancedEXT-dstPremultiplied-07506",
                             "vkCmdSetColorBlendAdvancedEXT(): pColorBlendAdvanced[%u].dstPremultiplied must not be VK_TRUE when "
                             "advancedBlendNonPremultipliedDstColor is not supported.",
                             attachment);
        }
        // VUID-VkColorBlendAdvancedEXT-blendOverlap-07507
        if (advanced.blendOverlap != VK_BLEND_OVERLAP_UNCORRELATED_EXT &&
            !phys_dev_ext_props.blend_operation_advanced_props.advancedBlendCorrelatedOverlap) {
            skip |= LogError(cb_state->Handle(), "VUID-VkColorBlendAdvancedEXT-blendOverlap-07507",
                             "vkCmdSetColorBlendAdvancedEXT(): pColorBlendAdvanced[%u].blendOverlap must be "
                             "VK_BLEND_OVERLAP_UNCORRELATED_EXT when advancedBlendCorrelatedOverlap is not supported.",
                             attachment);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer,
                                                             VkProvokingVertexModeEXT provokingVertexMode) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(*cb_state, CMD_SETPROVOKINGVERTEXMODEEXT,
                                         enabled_features.extended_dynamic_state3_features.extendedDynamicState3ProvokingVertexMode,
                                         "VUID-vkCmdSetProvokingVertexModeEXT-extendedDynamicState3ProvokingVertexMode-07446",
                                         "extendedDynamicState3ProvokingVertexMode");
    if (provokingVertexMode == VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT &&
        enabled_features.provoking_vertex_features.provokingVertexLast == VK_FALSE) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetProvokingVertexModeEXT-provokingVertexMode-07447",
                         "vkCmdSetProvokingVertexModeEXT(): provokingVertexMode is VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT but "
                         "the provokingVertexLast feature is not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                               VkLineRasterizationModeEXT lineRasterizationMode) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |=
        ValidateExtendedDynamicState(*cb_state, CMD_SETLINERASTERIZATIONMODEEXT,
                                     enabled_features.extended_dynamic_state3_features.extendedDynamicState3LineRasterizationMode,
                                     "VUID-vkCmdSetLineRasterizationModeEXT-extendedDynamicState3LineRasterizationMode-07417",
                                     "extendedDynamicState3LineRasterizationMode");
    if (lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT &&
        !enabled_features.line_rasterization_features.rectangularLines) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetLineRasterizationModeEXT-lineRasterizationMode-07418",
                         "vkCmdSetLineRasterizationModeEXT(): lineRasterizationMode is VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT "
                         "but the rectangularLines feature is not enabled.");
    } else if (lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT &&
               !enabled_features.line_rasterization_features.bresenhamLines) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetLineRasterizationModeEXT-lineRasterizationMode-07419",
                         "vkCmdSetLineRasterizationModeEXT(): lineRasterizationMode is VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT "
                         "but the bresenhamLines feature is not enabled.");
    } else if (lineRasterizationMode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT &&
               !enabled_features.line_rasterization_features.smoothLines) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetLineRasterizationModeEXT-lineRasterizationMode-07420",
                         "vkCmdSetLineRasterizationModeEXT(): lineRasterizationMode is "
                         "VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT but the smoothLines feature is not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETLINESTIPPLEENABLEEXT,
                                        enabled_features.extended_dynamic_state3_features.extendedDynamicState3LineStippleEnable,
                                        "VUID-vkCmdSetLineStippleEnableEXT-extendedDynamicState3LineStippleEnable-07421",
                                        "extendedDynamicState3LineStippleEnable");
}

bool CoreChecks::PreCallValidateCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer, VkBool32 negativeOneToOne) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    skip |= ValidateExtendedDynamicState(
        *cb_state, CMD_SETDEPTHCLIPNEGATIVEONETOONEEXT,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3DepthClipNegativeOneToOne,
        "VUID-vkCmdSetDepthClipNegativeOneToOneEXT-extendedDynamicState3DepthClipNegativeOneToOne-07452",
        "extendedDynamicState3DepthClipNegativeOneToOne");
    if (enabled_features.depth_clip_control_features.depthClipControl == VK_FALSE) {
        skip |= LogError(cb_state->Handle(), "VUID-vkCmdSetDepthClipNegativeOneToOneEXT-depthClipControl-07453",
                         "vkCmdSetDepthClipNegativeOneToOneEXT(): the depthClipControl feature is not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer,
                                                               VkBool32 viewportWScalingEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETVIEWPORTWSCALINGENABLENV,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3ViewportWScalingEnable,
        "VUID-vkCmdSetViewportWScalingEnableNV-extendedDynamicState3ViewportWScalingEnable-07580",
        "extendedDynamicState3ViewportWScalingEnable");
}

bool CoreChecks::PreCallValidateCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                        uint32_t viewportCount,
                                                        const VkViewportSwizzleNV *pViewportSwizzles) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETVIEWPORTSWIZZLENV, enabled_features.extended_dynamic_state3_features.extendedDynamicState3ViewportSwizzle,
        "VUID-vkCmdSetViewportSwizzleNV-extendedDynamicState3ViewportSwizzle-07445", "extendedDynamicState3ViewportSwizzle");
}

bool CoreChecks::PreCallValidateCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer, VkBool32 coverageToColorEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETCOVERAGETOCOLORENABLENV,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3CoverageToColorEnable,
        "VUID-vkCmdSetCoverageToColorEnableNV-extendedDynamicState3CoverageToColorEnable-07347",
        "extendedDynamicState3CoverageToColorEnable");
}

bool CoreChecks::PreCallValidateCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer,
                                                                uint32_t coverageToColorLocation) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETCOVERAGETOCOLORLOCATIONNV,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3CoverageToColorLocation,
        "VUID-vkCmdSetCoverageToColorLocationNV-extendedDynamicState3CoverageToColorLocation-07348",
        "extendedDynamicState3CoverageToColorLocation");
}

bool CoreChecks::PreCallValidateCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                               VkCoverageModulationModeNV coverageModulationMode) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETCOVERAGEMODULATIONMODENV,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3CoverageModulationMode,
        "VUID-vkCmdSetCoverageModulationModeNV-extendedDynamicState3CoverageModulationMode-07350",
        "extendedDynamicState3CoverageModulationMode");
}

bool CoreChecks::PreCallValidateCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer,
                                                                      VkBool32 coverageModulationTableEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETCOVERAGEMODULATIONTABLEENABLENV,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3CoverageModulationTableEnable,
        "VUID-vkCmdSetCoverageModulationTableEnableNV-extendedDynamicState3CoverageModulationTableEnable-07351",
        "extendedDynamicState3CoverageModulationTableEnable");
}

bool CoreChecks::PreCallValidateCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer,
                                                                uint32_t coverageModulationTableCount,
                                                                const float *pCoverageModulationTable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETCOVERAGEMODULATIONTABLENV,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3CoverageModulationTable,
        "VUID-vkCmdSetCoverageModulationTableNV-extendedDynamicState3CoverageModulationTable-07352",
        "extendedDynamicState3CoverageModulationTable");
}

bool CoreChecks::PreCallValidateCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer,
                                                               VkBool32 shadingRateImageEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETSHADINGRATEIMAGEENABLENV,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3ShadingRateImageEnable,
        "VUID-vkCmdSetShadingRateImageEnableNV-extendedDynamicState3ShadingRateImageEnable-07416",
        "extendedDynamicState3ShadingRateImageEnable");
}

bool CoreChecks::PreCallValidateCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                                         VkBool32 representativeFragmentTestEnable) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETREPRESENTATIVEFRAGMENTTESTENABLENV,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3RepresentativeFragmentTestEnable,
        "VUID-vkCmdSetRepresentativeFragmentTestEnableNV-extendedDynamicState3RepresentativeFragmentTestEnable-07346",
        "extendedDynamicState3RepresentativeFragmentTestEnable");
}

bool CoreChecks::PreCallValidateCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
                                                              VkCoverageReductionModeNV coverageReductionMode) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETCOVERAGEREDUCTIONMODENV,
        enabled_features.extended_dynamic_state3_features.extendedDynamicState3CoverageReductionMode,
        "VUID-vkCmdSetCoverageReductionModeNV-extendedDynamicState3CoverageReductionMode-07349",
        "extendedDynamicState3CoverageReductionMode");
}

bool CoreChecks::PreCallValidateCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkEvent *pEvent) const {
    bool skip = false;
    if (IsExtEnabled(device_extensions.vk_khr_portability_subset)) {
        if (VK_FALSE == enabled_features.portability_subset_features.events) {
            skip |= LogError(device, "VUID-vkCreateEvent-events-04468",
                             "vkCreateEvent: events are not supported via VK_KHR_portability_subset");
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer, const VkExtent2D *pFragmentSize,
                                                             const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    const char *cmd_name = "vkCmdSetFragmentShadingRateKHR()";
    bool skip = false;
    skip |=
        ValidateExtendedDynamicState(*cb_state, CMD_SETFRAGMENTSHADINGRATEKHR,
                                     enabled_features.fragment_shading_rate_features.pipelineFragmentShadingRate ||
                                         enabled_features.fragment_shading_rate_features.primitiveFragmentShadingRate ||
                                         enabled_features.fragment_shading_rate_features.attachmentFragmentShadingRate,
                                     "VUID-vkCmdSetFragmentShadingRateKHR-pipelineFragmentShadingRate-04509",
                                     "pipelineFragmentShadingRate, primitiveFragmentShadingRate, or attachmentFragmentShadingRate");

    if (!enabled_features.fragment_shading_rate_features.pipelineFragmentShadingRate && pFragmentSize->width != 1) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-pipelineFragmentShadingRate-04507",
                         "vkCmdSetFragmentShadingRateKHR: Pipeline fragment width of %u has been specified in %s, but "
                         "pipelineFragmentShadingRate is not enabled",
                         pFragmentSize->width, cmd_name);
    }

    if (!enabled_features.fragment_shading_rate_features.pipelineFragmentShadingRate && pFragmentSize->height != 1) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-pipelineFragmentShadingRate-04508",
                         "vkCmdSetFragmentShadingRateKHR: Pipeline fragment height of %u has been specified in %s, but "
                         "pipelineFragmentShadingRate is not enabled",
                         pFragmentSize->height, cmd_name);
    }

    if (!enabled_features.fragment_shading_rate_features.primitiveFragmentShadingRate &&
        combinerOps[0] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-primitiveFragmentShadingRate-04510",
                         "vkCmdSetFragmentShadingRateKHR: First combiner operation of %s has been specified in %s, but "
                         "primitiveFragmentShadingRate is not enabled",
                         string_VkFragmentShadingRateCombinerOpKHR(combinerOps[0]), cmd_name);
    }

    if (!enabled_features.fragment_shading_rate_features.attachmentFragmentShadingRate &&
        combinerOps[1] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-attachmentFragmentShadingRate-04511",
                         "vkCmdSetFragmentShadingRateKHR: Second combiner operation of %s has been specified in %s, but "
                         "attachmentFragmentShadingRate is not enabled",
                         string_VkFragmentShadingRateCombinerOpKHR(combinerOps[1]), cmd_name);
    }

    if (!phys_dev_ext_props.fragment_shading_rate_props.fragmentShadingRateNonTrivialCombinerOps &&
        (combinerOps[0] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR &&
         combinerOps[0] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR)) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-fragmentSizeNonTrivialCombinerOps-04512",
                         "vkCmdSetFragmentShadingRateKHR: First combiner operation of %s has been specified in %s, but "
                         "fragmentShadingRateNonTrivialCombinerOps is "
                         "not supported",
                         string_VkFragmentShadingRateCombinerOpKHR(combinerOps[0]), cmd_name);
    }

    if (!phys_dev_ext_props.fragment_shading_rate_props.fragmentShadingRateNonTrivialCombinerOps &&
        (combinerOps[1] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR &&
         combinerOps[1] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR)) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-fragmentSizeNonTrivialCombinerOps-04512",
                         "vkCmdSetFragmentShadingRateKHR: Second combiner operation of %s has been specified in %s, but "
                         "fragmentShadingRateNonTrivialCombinerOps "
                         "is not supported",
                         string_VkFragmentShadingRateCombinerOpKHR(combinerOps[1]), cmd_name);
    }

    if (pFragmentSize->width == 0) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04513",
                         "vkCmdSetFragmentShadingRateKHR: Fragment width of %u has been specified in %s.", pFragmentSize->width,
                         cmd_name);
    }

    if (pFragmentSize->height == 0) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04514",
                         "vkCmdSetFragmentShadingRateKHR: Fragment height of %u has been specified in %s.", pFragmentSize->height,
                         cmd_name);
    }

    if (pFragmentSize->width != 0 && !IsPowerOfTwo(pFragmentSize->width)) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04515",
                         "vkCmdSetFragmentShadingRateKHR: Non-power-of-two fragment width of %u has been specified in %s.",
                         pFragmentSize->width, cmd_name);
    }

    if (pFragmentSize->height != 0 && !IsPowerOfTwo(pFragmentSize->height)) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04516",
                         "vkCmdSetFragmentShadingRateKHR: Non-power-of-two fragment height of %u has been specified in %s.",
                         pFragmentSize->height, cmd_name);
    }

    if (pFragmentSize->width > 4) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04517",
                         "vkCmdSetFragmentShadingRateKHR: Fragment width of %u specified in %s is too large.", pFragmentSize->width,
                         cmd_name);
    }

    if (pFragmentSize->height > 4) {
        skip |= LogError(cb_state->commandBuffer(), "VUID-vkCmdSetFragmentShadingRateKHR-pFragmentSize-04518",
                         "vkCmdSetFragmentShadingRateKHR: Fragment height of %u specified in %s is too large",
                         pFragmentSize->height, cmd_name);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                          const VkBool32 *pColorWriteEnables) const {
    bool skip = false;

    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    skip |=
        ValidateExtendedDynamicState(*cb_state, CMD_SETCOLORWRITEENABLEEXT, enabled_features.color_write_features.colorWriteEnable,
                                     "VUID-vkCmdSetColorWriteEnableEXT-None-04803", "colorWriteEnable");

    if (attachmentCount > phys_dev_props.limits.maxColorAttachments) {
        skip |= LogError(commandBuffer, "VUID-vkCmdSetColorWriteEnableEXT-attachmentCount-06656",
                         "vkCmdSetColorWriteEnableEXT(): attachmentCount (%" PRIu32
                         ") is greater than the VkPhysicalDeviceLimits::maxColorAttachments limit (%" PRIu32 ").",
                         attachmentCount, phys_dev_props.limits.maxColorAttachments);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetVertexInputEXT(
    VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
    const VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount,
    const VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETVERTEXINPUTEXT,
                                        enabled_features.vertex_input_dynamic_state_features.vertexInputDynamicState,
                                        "VUID-vkCmdSetVertexInputEXT-None-04790", "vertexInputDynamicState");
}

bool CoreChecks::PreCallValidateCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType,
                                                          uint32_t customSampleOrderCount,
                                                          const VkCoarseSampleOrderCustomNV *pCustomSampleOrders) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETCOARSESAMPLEORDERNV, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetFragmentShadingRateEnumNV(VkCommandBuffer commandBuffer, VkFragmentShadingRateNV shadingRate,
                                                                const VkFragmentShadingRateCombinerOpKHR combinerOps[2]) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(
        *cb_state, CMD_SETFRAGMENTSHADINGRATEENUMNV, enabled_features.fragment_shading_rate_enums_features.fragmentShadingRateEnums,
        "VUID-vkCmdSetFragmentShadingRateEnumNV-fragmentShadingRateEnums-04579", "fragmentShadingRateEnums");
}

bool CoreChecks::PreCallValidateCmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer,
                                                             const VkPerformanceMarkerInfoINTEL *pMarkerInfo) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETPERFORMANCEMARKERINTEL, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer,
                                                                   const VkPerformanceStreamMarkerInfoINTEL *pMarkerInfo) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETPERFORMANCEOVERRIDEINTEL, VK_TRUE, nullptr, nullptr);
}

bool CoreChecks::PreCallValidateCmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer,
                                                               const VkPerformanceOverrideInfoINTEL *pOverrideInfo) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    return ValidateExtendedDynamicState(*cb_state, CMD_SETCOARSESAMPLEORDERNV, VK_TRUE, nullptr, nullptr);
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
            if ((aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != 0){
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

#ifdef VK_USE_PLATFORM_METAL_EXT
bool CoreChecks::PreCallValidateExportMetalObjectsEXT(VkDevice device, VkExportMetalObjectsInfoEXT *pMetalObjectsInfo) const {
    bool skip = false;
    const VkBaseOutStructure *metal_objects_info_ptr = reinterpret_cast<const VkBaseOutStructure *>(pMetalObjectsInfo->pNext);
    while (metal_objects_info_ptr) {
        switch (metal_objects_info_ptr->sType) {
            case VK_STRUCTURE_TYPE_EXPORT_METAL_DEVICE_INFO_EXT:
                if (std::find(instance_state->export_metal_flags.begin(), instance_state->export_metal_flags.end(),
                              VK_EXPORT_METAL_OBJECT_TYPE_METAL_DEVICE_BIT_EXT) == instance_state->export_metal_flags.end()) {
                    skip |= LogError(
                        device, "VUID-VkExportMetalObjectsInfoEXT-pNext-06791",
                        "ExportMetalObjectsEXT: pNext chain contains a VkExportMetalDeviceInfoEXT structure "
                        "but instance %s did not have a "
                        "VkExportMetalObjectCreateInfoEXT struct with exportObjectType of "
                        "VK_EXPORT_METAL_OBJECT_TYPE_METAL_DEVICE_BIT_EXT in the pNext chain of its VkInstanceCreateInfo structure",
                        report_data->FormatHandle(instance_state->instance).c_str());
                }
                break;

            case VK_STRUCTURE_TYPE_EXPORT_METAL_COMMAND_QUEUE_INFO_EXT:
                if (std::find(instance_state->export_metal_flags.begin(), instance_state->export_metal_flags.end(),
                              VK_EXPORT_METAL_OBJECT_TYPE_METAL_COMMAND_QUEUE_BIT_EXT) ==
                    instance_state->export_metal_flags.end()) {
                    skip |= LogError(device, "VUID-VkExportMetalObjectsInfoEXT-pNext-06792",
                                     "ExportMetalObjectsEXT: pNext chain contains a VkExportMetalCommandQueueInfoEXT structure "
                                     "but instance %s did not have a "
                                     "VkExportMetalObjectCreateInfoEXT struct with exportObjectType of "
                                     "VK_EXPORT_METAL_OBJECT_TYPE_METAL_COMMAND_QUEUE_BIT_EXT in the pNext chain of its "
                                     "VkInstanceCreateInfo structure",
                                     report_data->FormatHandle(instance_state->instance).c_str());
                }
                break;

            case VK_STRUCTURE_TYPE_EXPORT_METAL_BUFFER_INFO_EXT: {
                auto metal_buffer_ptr = reinterpret_cast<const VkExportMetalBufferInfoEXT *>(metal_objects_info_ptr);
                auto mem_info = Get<DEVICE_MEMORY_STATE>(metal_buffer_ptr->memory);
                if (mem_info) {
                    if (!mem_info->metal_buffer_export) {
                        skip |= LogError(
                            device, "VUID-VkExportMetalObjectsInfoEXT-pNext-06793",
                            "ExportMetalObjectsEXT: pNext chain contains a VkExportMetalBufferInfoEXT structure with memory = "
                            "%s, but that memory was not allocated with a VkExportMetalObjectCreateInfoEXT whose exportObjectType "
                            "member was set to VK_EXPORT_METAL_OBJECT_TYPE_METAL_BUFFER_BIT_EXT in the pNext chain of the "
                            "VkMemoryAllocateInfo structure",
                            report_data->FormatHandle(metal_buffer_ptr->memory).c_str());
                    }
                }
            } break;

            case VK_STRUCTURE_TYPE_EXPORT_METAL_TEXTURE_INFO_EXT: {
                auto metal_texture_ptr = reinterpret_cast<const VkExportMetalTextureInfoEXT *>(metal_objects_info_ptr);
                if ((metal_texture_ptr->image == VK_NULL_HANDLE && metal_texture_ptr->imageView == VK_NULL_HANDLE &&
                     metal_texture_ptr->bufferView == VK_NULL_HANDLE) ||
                    (metal_texture_ptr->image &&
                     ((metal_texture_ptr->imageView != VK_NULL_HANDLE) || (metal_texture_ptr->bufferView != VK_NULL_HANDLE))) ||
                    (metal_texture_ptr->imageView &&
                     ((metal_texture_ptr->image != VK_NULL_HANDLE) || (metal_texture_ptr->bufferView != VK_NULL_HANDLE))) ||
                    (metal_texture_ptr->bufferView &&
                     ((metal_texture_ptr->image != VK_NULL_HANDLE) || (metal_texture_ptr->imageView != VK_NULL_HANDLE)))) {
                    skip |=
                        LogError(device, "VUID-VkExportMetalObjectsInfoEXT-pNext-06794",
                                 "ExportMetalObjectsEXT: pNext chain contains a VkExportMetalTextureInfoEXT structure with image = "
                                 "%s, imageView = %s and bufferView = %s, but exactly one of those 3 must not be VK_NULL_HANDLE",
                                 report_data->FormatHandle(metal_texture_ptr->image).c_str(),
                                 report_data->FormatHandle(metal_texture_ptr->imageView).c_str(),
                                 report_data->FormatHandle(metal_texture_ptr->bufferView).c_str());
                }
                if (metal_texture_ptr->image) {
                    auto image_info = Get<IMAGE_STATE>(metal_texture_ptr->image);
                    if (image_info) {
                        if (!image_info->metal_image_export) {
                            skip |= LogError(
                                device, "VUID-VkExportMetalObjectsInfoEXT-pNext-06795",
                                "ExportMetalObjectsEXT: pNext chain contains a VkExportMetalTextureInfoEXT structure with image = "
                                "%s, but that image was not created with a VkExportMetalObjectCreateInfoEXT whose exportObjectType "
                                "member was set to VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT in the pNext chain of the "
                                "VkImageCreateInfo structure",
                                report_data->FormatHandle(metal_texture_ptr->image).c_str());
                        }
                        auto format_plane_count = FormatPlaneCount(image_info->createInfo.format);
                        auto image_plane = metal_texture_ptr->plane;
                        if (!(format_plane_count > 1) && (image_plane != VK_IMAGE_ASPECT_PLANE_0_BIT)) {
                            skip |= LogError(
                                device, "VUID-VkExportMetalObjectsInfoEXT-pNext-06799",
                                "ExportMetalObjectsEXT: pNext chain contains a VkExportMetalTextureInfoEXT structure with image = "
                                "%s, and plane = %s, but image was created with format %s, which is not multiplaner and plane is "
                                "required to be VK_IMAGE_ASPECT_PLANE_0_BIT",
                                report_data->FormatHandle(metal_texture_ptr->image).c_str(),
                                string_VkImageAspectFlags(image_plane).c_str(), string_VkFormat(image_info->createInfo.format));
                        }
                        if ((format_plane_count == 2) && (image_plane == VK_IMAGE_ASPECT_PLANE_2_BIT)) {
                            skip |= LogError(
                                device, "VUID-VkExportMetalObjectsInfoEXT-pNext-06800",
                                "ExportMetalObjectsEXT: pNext chain contains a VkExportMetalTextureInfoEXT structure with image = "
                                "%s, and plane = %s, but image was created with format %s, which has exactly 2 planes and plane "
                                "cannot"
                                "be VK_IMAGE_ASPECT_PLANE_2_BIT",
                                report_data->FormatHandle(metal_texture_ptr->image).c_str(),
                                string_VkImageAspectFlags(image_plane).c_str(), string_VkFormat(image_info->createInfo.format));
                        }
                    }
                }
                if (metal_texture_ptr->imageView) {
                    auto image_view_info = Get<IMAGE_VIEW_STATE>(metal_texture_ptr->imageView);
                    if (image_view_info) {
                        if (!image_view_info->metal_imageview_export) {
                            skip |= LogError(
                                device, "VUID-VkExportMetalObjectsInfoEXT-pNext-06796",
                                "ExportMetalObjectsEXT: pNext chain contains a VkExportMetalTextureInfoEXT structure with "
                                "imageView = "
                                "%s, but that image view was not created with a VkExportMetalObjectCreateInfoEXT whose "
                                "exportObjectType "
                                "member was set to VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT in the pNext chain of the "
                                "VkImageViewCreateInfo structure",
                                report_data->FormatHandle(metal_texture_ptr->imageView).c_str());
                        }
                        auto format_plane_count = FormatPlaneCount(image_view_info->create_info.format);
                        auto image_plane = metal_texture_ptr->plane;
                        if (!(format_plane_count > 1) && (image_plane != VK_IMAGE_ASPECT_PLANE_0_BIT)) {
                            skip |= LogError(
                                device, "VUID-VkExportMetalObjectsInfoEXT-pNext-06801",
                                "ExportMetalObjectsEXT: pNext chain contains a VkExportMetalTextureInfoEXT structure with "
                                "imageView = "
                                "%s, and plane = %s, but imageView was created with format %s, which is not multiplaner and "
                                "plane is "
                                "required to be VK_IMAGE_ASPECT_PLANE_0_BIT",
                                report_data->FormatHandle(metal_texture_ptr->imageView).c_str(),
                                string_VkImageAspectFlags(image_plane).c_str(),
                                string_VkFormat(image_view_info->create_info.format));
                        }
                        if ((format_plane_count == 2) && (image_plane == VK_IMAGE_ASPECT_PLANE_2_BIT)) {
                            skip |= LogError(device, "VUID-VkExportMetalObjectsInfoEXT-pNext-06802",
                                             "ExportMetalObjectsEXT: pNext chain contains a VkExportMetalTextureInfoEXT structure "
                                             "with imageView = "
                                             "%s, and plane = %s, but imageView was created with format %s, which has exactly 2 "
                                             "planes and plane "
                                             "cannot"
                                             "be VK_IMAGE_ASPECT_PLANE_2_BIT",
                                             report_data->FormatHandle(metal_texture_ptr->imageView).c_str(),
                                             string_VkImageAspectFlags(image_plane).c_str(),
                                             string_VkFormat(image_view_info->create_info.format));
                        }
                    }
                }
                if (metal_texture_ptr->bufferView) {
                    auto buffer_view_info = Get<BUFFER_VIEW_STATE>(metal_texture_ptr->bufferView);
                    if (buffer_view_info) {
                        if (!buffer_view_info->metal_bufferview_export) {
                            skip |= LogError(
                                device, "VUID-VkExportMetalObjectsInfoEXT-pNext-06797",
                                "ExportMetalObjectsEXT: pNext chain contains a VkExportMetalTextureInfoEXT structure with "
                                "bufferView = "
                                "%s, but that buffer view was not created with a VkExportMetalObjectCreateInfoEXT whose "
                                "exportObjectType "
                                "member was set to VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT in the pNext chain of the "
                                "VkBufferViewCreateInfo structure",
                                report_data->FormatHandle(metal_texture_ptr->bufferView).c_str());
                        }
                    }
                }
                if (metal_texture_ptr->image || metal_texture_ptr->imageView) {
                    if ((metal_texture_ptr->plane != VK_IMAGE_ASPECT_PLANE_0_BIT) &&
                        (metal_texture_ptr->plane != VK_IMAGE_ASPECT_PLANE_1_BIT) &&
                        (metal_texture_ptr->plane != VK_IMAGE_ASPECT_PLANE_2_BIT)) {
                        skip |= LogError(
                            device, "VUID-VkExportMetalObjectsInfoEXT-pNext-06798",
                            "ExportMetalObjectsEXT: pNext chain contains a VkExportMetalTextureInfoEXT structure with "
                            "image = %s and imageView = "
                            "%s, but plane = %s which is not one of  VK_IMAGE_ASPECT_PLANE_0_BIT,  VK_IMAGE_ASPECT_PLANE_1_BIT, "
                            "or  VK_IMAGE_ASPECT_PLANE_2_BIT",
                            report_data->FormatHandle(metal_texture_ptr->image).c_str(),
                            report_data->FormatHandle(metal_texture_ptr->imageView).c_str(),
                            string_VkImageAspectFlags(metal_texture_ptr->plane).c_str());
                    }
                }
            } break;

            case VK_STRUCTURE_TYPE_EXPORT_METAL_IO_SURFACE_INFO_EXT: {
                auto metal_io_surface_ptr = reinterpret_cast<const VkExportMetalIOSurfaceInfoEXT *>(metal_objects_info_ptr);
                auto image_info = Get<IMAGE_STATE>(metal_io_surface_ptr->image);
                if (image_info) {
                    if (!image_info->metal_io_surface_export) {
                        skip |= LogError(
                            device, "VUID-VkExportMetalObjectsInfoEXT-pNext-06803",
                            "ExportMetalObjectsEXT: pNext chain contains a VkExportMetalIOSurfaceInfoEXT structure with image = "
                            "%s, but that image was not created with a VkExportMetalObjectCreateInfoEXT whose exportObjectType "
                            "member was set to VK_EXPORT_METAL_OBJECT_TYPE_METAL_IOSURFACE_BIT_EXT in the pNext chain of the "
                            "VkImageCreateInfo structure",
                            report_data->FormatHandle(metal_io_surface_ptr->image).c_str());
                    }
                }
            } break;

            case VK_STRUCTURE_TYPE_EXPORT_METAL_SHARED_EVENT_INFO_EXT: {
                auto metal_shared_event_ptr = reinterpret_cast<const VkExportMetalSharedEventInfoEXT *>(metal_objects_info_ptr);
                if ((metal_shared_event_ptr->event == VK_NULL_HANDLE && metal_shared_event_ptr->semaphore == VK_NULL_HANDLE) ||
                    (metal_shared_event_ptr->event != VK_NULL_HANDLE && metal_shared_event_ptr->semaphore != VK_NULL_HANDLE)) {
                    skip |= LogError(
                        device, "VUID-VkExportMetalObjectsInfoEXT-pNext-06804",
                        "ExportMetalObjectsEXT: pNext chain contains a VkExportMetalSharedEventInfoEXT structure with semaphore = "
                        "%s, and event = %s, but exactly one of them must not be VK_NULL_HANDLE",
                        report_data->FormatHandle(metal_shared_event_ptr->semaphore).c_str(),
                        report_data->FormatHandle(metal_shared_event_ptr->event).c_str());
                }

                if (metal_shared_event_ptr->semaphore) {
                    auto semaphore_info = Get<SEMAPHORE_STATE>(metal_shared_event_ptr->semaphore);
                    if (semaphore_info && !(semaphore_info->metal_semaphore_export)) {
                        skip |= LogError(
                            device, "VUID-VkExportMetalObjectsInfoEXT-pNext-06805",
                            "ExportMetalObjectsEXT: pNext chain contains a VkExportMetalSharedEventInfoEXT structure "
                            "with semaphore = "
                            "%s, but that semaphore was not created with a VkExportMetalObjectCreateInfoEXT whose exportObjectType "
                            "member was set to VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT in the pNext chain of the "
                            "VkSemaphoreCreateInfo structure",
                            report_data->FormatHandle(metal_shared_event_ptr->semaphore).c_str());
                    }
                }
                if (metal_shared_event_ptr->event) {
                    auto event_info = Get<EVENT_STATE>(metal_shared_event_ptr->event);
                    if (event_info && !(event_info->metal_event_export)) {
                        skip |= LogError(
                            device, "VUID-VkExportMetalObjectsInfoEXT-pNext-06806",
                            "ExportMetalObjectsEXT: pNext chain contains a VkExportMetalSharedEventInfoEXT structure "
                            "with event = "
                            "%s, but that event was not created with a VkExportMetalObjectCreateInfoEXT whose exportObjectType "
                            "member was set to VK_EXPORT_METAL_OBJECT_TYPE_METAL_SHARED_EVENT_BIT_EXT in the pNext chain of the "
                            "VkEventCreateInfo structure",
                            report_data->FormatHandle(metal_shared_event_ptr->event).c_str());
                    }
                }
            } break;
            default:
                break;
        }
        metal_objects_info_ptr = metal_objects_info_ptr->pNext;
    }
    return skip;
}
#endif  // VK_USE_PLATFORM_METAL_EXT
