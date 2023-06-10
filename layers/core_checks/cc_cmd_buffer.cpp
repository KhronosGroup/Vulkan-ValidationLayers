/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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

#include <assert.h>
#include <string>
#include <sstream>

#include "generated/vk_enum_string_helper.h"
#include "generated/chassis.h"
#include "core_validation.h"
#include "generated/enum_flag_bits.h"

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
        std::string vuid;
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

bool CoreChecks::PreCallValidateBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                                   const VkCommandBufferBeginInfo *pBeginInfo) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state) return false;
    bool skip = false;
    if (cb_state->InUse()) {
        skip |= LogError(commandBuffer, "VUID-vkBeginCommandBuffer-commandBuffer-00049",
                         "vkBeginCommandBuffer() on active %s before it has completed. You must check "
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
                                         "vkBeginCommandBuffer(): Render pass must be a valid VkRenderPass.");
                    } else {
                        if (info->subpass >= render_pass->createInfo.subpassCount) {
                            skip |= LogError(commandBuffer, "VUID-VkCommandBufferBeginInfo-flags-06001",
                                             "vkBeginCommandBuffer(): Subpass member of pInheritanceInfo (%" PRIu32
                                             ") must be a valid subpass "
                                             "index within pInheritanceInfo->renderPass.",
                                             info->subpass);
                        }
                    }
                } else {
                    if (!p_inherited_rendering_info) {
                        skip |= LogError(commandBuffer, "VUID-VkCommandBufferBeginInfo-flags-06002",
                                         "vkBeginCommandBuffer():The pNext chain of pInheritanceInfo must include a "
                                         "VkCommandBufferInheritanceRenderingInfoKHR structure.");
                    }
                }
            }

            if (p_inherited_rendering_info) {
                auto p_attachment_sample_count_info_amd = LvlFindInChain<VkAttachmentSampleCountInfoAMD>(info->pNext);
                if (p_attachment_sample_count_info_amd &&
                    p_attachment_sample_count_info_amd->colorAttachmentCount != p_inherited_rendering_info->colorAttachmentCount) {
                    skip |= LogError(commandBuffer, "VUID-VkCommandBufferBeginInfo-flags-06003",
                                     "vkBeginCommandBuffer(): VkAttachmentSampleCountInfoAMD::colorAttachmentCount (%" PRIu32
                                     ") must equal VkCommandBufferInheritanceRenderingInfo::colorAttachmentCount (%" PRIu32 ").",
                                     p_attachment_sample_count_info_amd->colorAttachmentCount,
                                     p_inherited_rendering_info->colorAttachmentCount);
                }

                if ((p_inherited_rendering_info->colorAttachmentCount != 0) &&
                    (p_inherited_rendering_info->rasterizationSamples & AllVkSampleCountFlagBits) == 0) {
                    skip |= LogError(commandBuffer, "VUID-VkCommandBufferInheritanceRenderingInfo-colorAttachmentCount-06004",
                                     "vkBeginCommandBuffer(): colorAttachmentCount (%" PRIu32
                                     ") is not 0, so rasterizationSamples (%s) must be a valid VkSampleCountFlagBits value.",
                                     p_inherited_rendering_info->colorAttachmentCount,
                                     string_VkSampleCountFlagBits(p_inherited_rendering_info->rasterizationSamples));
                }

                if ((enabled_features.core.variableMultisampleRate == false) &&
                    (p_inherited_rendering_info->rasterizationSamples & AllVkSampleCountFlagBits) == 0) {
                    skip |= LogError(commandBuffer, "VUID-VkCommandBufferInheritanceRenderingInfo-variableMultisampleRate-06005",
                                     "vkBeginCommandBuffer(): rasterizationSamples is %s.",
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
                                    skip |= LogError(commandBuffer,
                                                     "VUID-VkCommandBufferInheritanceRenderingInfo-pColorAttachmentFormats-06006",
                                                     "vkBeginCommandBuffer(): "
                                                     "pColorAttachmentFormats[%" PRIu32 "] is %s.",
                                                     i, string_VkFormat(attachment_format));
                                } else if ((potential_format_features & VK_FORMAT_FEATURE_2_LINEAR_COLOR_ATTACHMENT_BIT_NV) == 0) {
                                    skip |=
                                        LogError(commandBuffer,
                                                 "VUID-VkCommandBufferInheritanceRenderingInfoKHR-pColorAttachmentFormats-06492",
                                                 "vkBeginCommandBuffer(): "
                                                 "pColorAttachmentFormats[%" PRIu32 "] is %s.",
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
                        skip |= LogError(commandBuffer, "VUID-VkCommandBufferInheritanceRenderingInfo-depthAttachmentFormat-06007",
                                         "vkBeginCommandBuffer(): depthAttachmentFormat is %s.", string_VkFormat(depth_format));
                    }
                    if (!FormatHasDepth(depth_format)) {
                        skip |= LogError(commandBuffer, "VUID-VkCommandBufferInheritanceRenderingInfo-depthAttachmentFormat-06540",
                                         "vkBeginCommandBuffer(): depthAttachmentFormat is %s.", string_VkFormat(depth_format));
                    }
                }

                const VkFormat stencil_format = p_inherited_rendering_info->stencilAttachmentFormat;
                if (stencil_format != VK_FORMAT_UNDEFINED) {
                    const VkFormatFeatureFlags2KHR potential_format_features = GetPotentialFormatFeatures(stencil_format);
                    if ((potential_format_features & valid_depth_stencil_format) == 0) {
                        skip |=
                            LogError(commandBuffer, "VUID-VkCommandBufferInheritanceRenderingInfo-stencilAttachmentFormat-06199",
                                     "vkBeginCommandBuffer(): stencilAttachmentFormat is %s.", string_VkFormat(stencil_format));
                    }
                    if (!FormatHasStencil(stencil_format)) {
                        skip |=
                            LogError(commandBuffer, "VUID-VkCommandBufferInheritanceRenderingInfo-stencilAttachmentFormat-06541",
                                     "vkBeginCommandBuffer(): stencilAttachmentFormat is %s.", string_VkFormat(stencil_format));
                    }
                }

                if ((depth_format != VK_FORMAT_UNDEFINED && stencil_format != VK_FORMAT_UNDEFINED) &&
                    (depth_format != stencil_format)) {
                    skip |= LogError(commandBuffer, "VUID-VkCommandBufferInheritanceRenderingInfo-depthAttachmentFormat-06200",
                                     "vkBeginCommandBuffer(): depthAttachmentFormat "
                                     "(%s) is not the same as stencilAttachmentFormat (%s).",
                                     string_VkFormat(depth_format), string_VkFormat(stencil_format));
                }

                if ((enabled_features.core11.multiview == VK_FALSE) && (p_inherited_rendering_info->viewMask != 0)) {
                    skip |= LogError(commandBuffer, "VUID-VkCommandBufferInheritanceRenderingInfo-multiview-06008",
                                     "vkBeginCommandBuffer(): Multiview feature is not enabled, but viewMask is (%" PRIu32 ").",
                                     p_inherited_rendering_info->viewMask);
                }

                if (MostSignificantBit(p_inherited_rendering_info->viewMask) >=
                    static_cast<int32_t>(phys_dev_props_core11.maxMultiviewViewCount)) {
                    skip |= LogError(commandBuffer, "VUID-VkCommandBufferInheritanceRenderingInfo-viewMask-06009",
                                     "vkBeginCommandBuffer(): The index of the most significant bit in "
                                     "viewMask (%" PRIu32
                                     ") is superior or equal to VkPhysicalDeviceVulkan11Properties::maxMultiviewViewCount (%" PRIu32
                                     ").",
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
                                     "viewportScissor2D is VK_TRUE, but viewportDepthCount is 0.");
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
                                                 "vkBeginCommandBuffer(): Secondary %s has a subpass index (%" PRIu32
                                                 ") that is "
                                                 "superior or equal to the number of subpasses (%" PRIu32 ").",
                                                 report_data->FormatHandle(commandBuffer).c_str(), info->subpass,
                                                 render_pass->createInfo.subpassCount);
                            }
                        }
                    } else {
                        skip |= LogError(commandBuffer, "VUID-VkCommandBufferBeginInfo-flags-00053",
                                         "vkBeginCommandBuffer(): Flags contains VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT "
                                         "but the renderpass member of pInheritanceInfo is VK_NULL_HANDLE.");
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

    for (const auto &query_obj : cb_state.activeQueries) {
        skip |= LogError(commandBuffer, "VUID-vkEndCommandBuffer-commandBuffer-00061",
                         "vkEndCommandBuffer(): Ending command buffer with in progress query: %s, query %d.",
                         report_data->FormatHandle(query_obj.pool).c_str(), query_obj.slot);
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
        const LogObjectList objlist(commandBuffer, buffer);
        skip |= LogError(objlist, "VUID-vkCmdBindIndexBuffer-offset-00432",
                         "vkCmdBindIndexBuffer() offset (0x%" PRIxLEAST64 ") does not fall on alignment (%s) boundary.", offset,
                         string_VkIndexType(indexType));
    }
    if (offset >= buffer_state->requirements.size) {
        const LogObjectList objlist(commandBuffer, buffer);
        skip |= LogError(objlist, "VUID-vkCmdBindIndexBuffer-offset-00431",
                         "vkCmdBindIndexBuffer() offset (0x%" PRIxLEAST64 ") is not less than the size (0x%" PRIxLEAST64 ").",
                         offset, buffer_state->requirements.size);
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
                const LogObjectList objlist(commandBuffer, buffer_state->buffer());
                skip |=
                    LogError(objlist, "VUID-vkCmdBindVertexBuffers-pOffsets-00626",
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
        const LogObjectList objlist(commandBuffer, dstBuffer);
        skip |= LogError(objlist, "VUID-vkCmdUpdateBuffer-dstOffset-00032",
                         "vkCmdUpdateBuffer() dstOffset (0x%" PRIxLEAST64 ") is not less than the size (0x%" PRIxLEAST64 ").",
                         dstOffset, dst_buffer_state->createInfo.size);
    } else if (dataSize > dst_buffer_state->createInfo.size - dstOffset) {
        const LogObjectList objlist(commandBuffer, dstBuffer);
        skip |= LogError(objlist, "VUID-vkCmdUpdateBuffer-dataSize-00033",
                         "vkCmdUpdateBuffer() dataSize (0x%" PRIxLEAST64 ") is not less than the buffer size (0x%" PRIxLEAST64
                         ") minus dstOffset (0x%" PRIxLEAST64 ").",
                         dataSize, dst_buffer_state->createInfo.size, dstOffset);
    }
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

bool CoreChecks::ValidateSecondaryCommandBufferState(const CMD_BUFFER_STATE &cb_state, const CMD_BUFFER_STATE &sub_cb_state) const {
    bool skip = false;

    vvl::unordered_set<int> active_types;
    if (!disabled[query_validation]) {
        for (const auto &query_object : cb_state.activeQueries) {
            auto query_pool_state = Get<QUERY_POOL_STATE>(query_object.pool);
            if (query_pool_state) {
                if (query_pool_state->createInfo.queryType == VK_QUERY_TYPE_PIPELINE_STATISTICS &&
                    sub_cb_state.beginInfo.pInheritanceInfo) {
                    VkQueryPipelineStatisticFlags cmd_buf_statistics = sub_cb_state.beginInfo.pInheritanceInfo->pipelineStatistics;
                    if ((cmd_buf_statistics & query_pool_state->createInfo.pipelineStatistics) != cmd_buf_statistics) {
                        const LogObjectList objlist(cb_state.commandBuffer(), sub_cb_state.commandBuffer(), query_object.pool);
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
                const LogObjectList objlist(cb_state.commandBuffer(), sub_cb_state.commandBuffer(), query_object.pool);
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
                viewports_to_inherit_[n] = primary_state->dynamic_state_value.viewports[n];
            }
        }

        viewport_count_to_inherit_ = primary_state->dynamic_state_value.viewport_count;
        scissor_count_to_inherit_ = primary_state->dynamic_state_value.scissor_count;
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
                viewports_to_inherit_[n] = secondary_state->dynamic_state_value.viewports[n];
                viewport_trashed_by_[n] = kNotTrashed;
            }
            if ((secondary_state->scissorMask | secondary_state->scissorWithCountMask) & bit) {
                scissor_trashed_by_[n] = kNotTrashed;
            }
            if (secondary_state->dynamic_state_value.viewport_count != 0) {
                viewport_count_to_inherit_ = secondary_state->dynamic_state_value.viewport_count;
                viewport_count_trashed_by_ = kNotTrashed;
            }
            if (secondary_state->dynamic_state_value.scissor_count != 0) {
                scissor_count_to_inherit_ = secondary_state->dynamic_state_value.scissor_count;
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
                        "vkCmdExecuteCommands(): Draw commands in pCommandBuffers[%" PRIu32
                        "] (%s) consume inherited viewport %" PRIu32
                        " %s"
                        "but this state was not inherited as its depth range [%f, %f] does not match "
                        "pViewportDepths[%" PRIu32 "] = [%f, %f]",
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
                case VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT:
                    state_name = "dynamic viewport count";
                    break;
                case VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT:
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
                                              VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
            } else {
                check_viewport_count = viewport_count_to_inherit_;
            }
        }
        if (secondary_state->usedDynamicScissorCount) {
            if (scissor_count_to_inherit_ == 0 || scissor_count_trashed_by_ != kNotTrashed) {
                skip |= check_missing_inherit(scissor_count_to_inherit_, scissor_count_trashed_by_,
                                              VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
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
                "Draw commands in pCommandBuffers[%" PRIu32
                "] (%s) consume inherited dynamic viewport with count state "
                "but the dynamic viewport count (%" PRIu32 ") exceeds the inheritance limit (viewportDepthCount=%" PRIu32 ").",
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
    vvl::unordered_set<const CMD_BUFFER_STATE *> linked_command_buffers;
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

        if (VK_COMMAND_BUFFER_LEVEL_SECONDARY != sub_cb_state->createInfo.level) {
            const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
            skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-pCommandBuffers-00088",
                             "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32 "] is not VK_COMMAND_BUFFER_LEVEL_SECONDARY.", i);
        } else {
            if (!cb_state->activeRenderPass) {
                if (sub_cb_state->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) {
                    const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                    skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-pCommandBuffers-00100",
                                     "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                     "] %s is executed outside a render pass "
                                     "instance scope, but the Secondary Command Buffer does have the "
                                     "VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT set in VkCommandBufferBeginInfo::flags when "
                                     "the vkBeginCommandBuffer() was called.",
                                     i, report_data->FormatHandle(pCommandBuffers[i]).c_str());
                }
            } else if (sub_cb_state->beginInfo.pInheritanceInfo != nullptr) {
                const uint32_t inheritance_subpass = sub_cb_state->beginInfo.pInheritanceInfo->subpass;
                const VkRenderPass inheritance_render_pass = sub_cb_state->beginInfo.pInheritanceInfo->renderPass;
                auto secondary_rp_state = Get<RENDER_PASS_STATE>(inheritance_render_pass);
                if (!(sub_cb_state->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)) {
                    const LogObjectList objlist(pCommandBuffers[i], cb_state->activeRenderPass->renderPass());
                    skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-pCommandBuffers-00096",
                                     "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                     "] %s is executed within a %s "
                                     "instance scope, but the Secondary Command Buffer does not have the "
                                     "VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT set in VkCommandBufferBeginInfo::flags when "
                                     "the vkBeginCommandBuffer() was called.",
                                     i, report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                     report_data->FormatHandle(cb_state->activeRenderPass->renderPass()).c_str());
                } else if (sub_cb_state->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) {
                    if (!cb_state->activeRenderPass->UsesDynamicRendering()) {
                        // Make sure render pass is compatible with parent command buffer pass if secondary command buffer has
                        // "render pass continue" usage flag
                        if (cb_state->activeRenderPass->renderPass() != secondary_rp_state->renderPass()) {
                            skip |= ValidateRenderPassCompatibility(
                                "primary command buffer", *cb_state->activeRenderPass.get(), "secondary command buffer",
                                *secondary_rp_state.get(), "vkCmdExecuteCommands()", "VUID-vkCmdExecuteCommands-pBeginInfo-06020");
                        }
                        //  If framebuffer for secondary CB is not NULL, then it must match active FB from primaryCB
                        skip |= ValidateInheritanceInfoFramebuffer(commandBuffer, *cb_state, pCommandBuffers[i], *sub_cb_state,
                                                                   "vkCmdExecuteCommands()");
                    }
                    // Inherit primary's activeFramebuffer, or null if using dynamic rendering,
                    // and while running validate functions
                    for (auto &function : sub_cb_state->cmd_execute_commands_functions) {
                        skip |= function(*sub_cb_state, cb_state.get(), cb_state->activeFramebuffer.get());
                    }
                }

                if (!cb_state->activeRenderPass->UsesDynamicRendering() && (cb_state->GetActiveSubpass() != inheritance_subpass)) {
                    const LogObjectList objlist(commandBuffer, pCommandBuffers[i], cb_state->activeRenderPass->renderPass());
                    skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-pCommandBuffers-06019",
                                     "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                     "] %s is executed within a %s "
                                     "instance scope begun by vkCmdBeginRenderPass(), but "
                                     "VkCommandBufferInheritanceInfo::subpass (%" PRIu32
                                     ") does not "
                                     "match the current subpass (%" PRIu32 ").",
                                     i, report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                     report_data->FormatHandle(cb_state->activeRenderPass->renderPass()).c_str(),
                                     inheritance_subpass, cb_state->GetActiveSubpass());
                } else if (cb_state->activeRenderPass->UsesDynamicRendering()) {
                    if (inheritance_render_pass != VK_NULL_HANDLE) {
                        const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                        skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-pBeginInfo-06025",
                                         "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                         "] %s is executed within a dynamic renderpass instance scope begun "
                                         "by %s(), but "
                                         "VkCommandBufferInheritanceInfo::pInheritanceInfo::renderPass is not VK_NULL_HANDLE.",
                                         i, report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                         cb_state->begin_rendering_func_name.c_str());
                    }

                    if (sub_cb_state->activeRenderPass->use_dynamic_rendering_inherited) {
                        const auto rendering_info = cb_state->activeRenderPass->dynamic_rendering_begin_rendering_info;
                        const auto inheritance_rendering_info = sub_cb_state->activeRenderPass->inheritance_rendering_info;
                        if (inheritance_rendering_info.flags !=
                            (rendering_info.flags & ~VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR)) {
                            const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                            skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-flags-06026",
                                             "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                             "] %s is executed within a dynamic renderpass instance scope begun "
                                             "by %s(), but VkCommandBufferInheritanceRenderingInfo::flags (%" PRIu32
                                             ") does "
                                             "not match VkRenderingInfo::flags (%" PRIu32
                                             "), excluding "
                                             "VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR.",
                                             i, report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                             cb_state->begin_rendering_func_name.c_str(), inheritance_rendering_info.flags,
                                             rendering_info.flags);
                        }

                        if (inheritance_rendering_info.colorAttachmentCount != rendering_info.colorAttachmentCount) {
                            const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                            skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-colorAttachmentCount-06027",
                                             "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                             "] %s is executed within a dynamic renderpass instance scope begun "
                                             "by %s(), but "
                                             "VkCommandBufferInheritanceRenderingInfo::colorAttachmentCount (%" PRIu32
                                             ") does "
                                             "not match VkRenderingInfo::colorAttachmentCount (%" PRIu32 ").",
                                             i, report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                             cb_state->begin_rendering_func_name.c_str(),
                                             inheritance_rendering_info.colorAttachmentCount, rendering_info.colorAttachmentCount);
                        }

                        for (uint32_t color_i = 0, count = std::min(inheritance_rendering_info.colorAttachmentCount,
                                                                    rendering_info.colorAttachmentCount);
                             color_i < count; color_i++) {
                            if (rendering_info.pColorAttachments[color_i].imageView == VK_NULL_HANDLE) {
                                if (inheritance_rendering_info.pColorAttachmentFormats[color_i] != VK_FORMAT_UNDEFINED) {
                                    const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                                    skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-imageView-07606",
                                                     "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                                     "] %s is executed within a dynamic render pass instance "
                                                     "scope begun "
                                                     "by %s(), VkRenderingInfo::pColorAttachments[%" PRIu32
                                                     "].imageView is VK_NULL_HANDLE but "
                                                     "VkCommandBufferInheritanceRenderingInfo::pColorAttachmentFormats[%" PRIu32
                                                     "] is %s.",
                                                     i, report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                                     cb_state->begin_rendering_func_name.c_str(), color_i, color_i,
                                                     string_VkFormat(inheritance_rendering_info.pColorAttachmentFormats[color_i]));
                                }
                            } else {
                                auto image_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pColorAttachments[color_i].imageView);

                                if (image_view_state->create_info.format !=
                                    inheritance_rendering_info.pColorAttachmentFormats[color_i]) {
                                    const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                                    skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-imageView-06028",
                                                     "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                                     "] %s is executed within a dynamic render pass instance "
                                                     "scope begun "
                                                     "by %s(), VkRenderingInfo::pColorAttachments[%" PRIu32
                                                     "].imageView format is %s but "
                                                     "VkCommandBufferInheritanceRenderingInfo::pColorAttachmentFormats[%" PRIu32
                                                     "] is %s.",
                                                     i, report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                                     cb_state->begin_rendering_func_name.c_str(), color_i,
                                                     string_VkFormat(image_view_state->create_info.format), color_i,
                                                     string_VkFormat(inheritance_rendering_info.pColorAttachmentFormats[color_i]));
                                }
                            }
                        }

                        if ((rendering_info.pDepthAttachment != nullptr) &&
                            rendering_info.pDepthAttachment->imageView != VK_NULL_HANDLE) {
                            auto image_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pDepthAttachment->imageView);

                            if (image_view_state->create_info.format != inheritance_rendering_info.depthAttachmentFormat) {
                                const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                                skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-pDepthAttachment-06029",
                                                 "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                                 "] %s is executed within a dynamic renderpass "
                                                 "instance scope begun "
                                                 "by %s(), but "
                                                 "VkCommandBufferInheritanceRenderingInfo::depthAttachmentFormat does "
                                                 "not match the format of the imageView in VkRenderingInfo::pDepthAttachment.",
                                                 i, report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                                 cb_state->begin_rendering_func_name.c_str());
                            }
                        }

                        if ((rendering_info.pStencilAttachment != nullptr) &&
                            rendering_info.pStencilAttachment->imageView != VK_NULL_HANDLE) {
                            auto image_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pStencilAttachment->imageView);

                            if (image_view_state->create_info.format != inheritance_rendering_info.stencilAttachmentFormat) {
                                const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                                skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-pStencilAttachment-06030",
                                                 "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                                 "] %s is executed within a dynamic renderpass "
                                                 "instance scope begun "
                                                 "by %s(), but "
                                                 "VkCommandBufferInheritanceRenderingInfo::stencilAttachmentFormat does "
                                                 "not match the format of the imageView in VkRenderingInfo::pStencilAttachment.",
                                                 i, report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                                 cb_state->begin_rendering_func_name.c_str());
                            }
                        }

                        if (rendering_info.pDepthAttachment == nullptr ||
                            rendering_info.pDepthAttachment->imageView == VK_NULL_HANDLE) {
                            VkFormat format = inheritance_rendering_info.depthAttachmentFormat;
                            if (format != VK_FORMAT_UNDEFINED) {
                                const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                                skip |= LogError(
                                    objlist, "VUID-vkCmdExecuteCommands-pDepthAttachment-06774",
                                    "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                    "] %s is executed within a dynamic renderpass "
                                    "instance scope begun by %s(), and VkRenderingInfo::pDepthAttachment does not define an "
                                    "image view but VkCommandBufferInheritanceRenderingInfo::depthAttachmentFormat "
                                    "is %s instead of VK_FORMAT_UNDEFINED.",
                                    i, report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                    cb_state->begin_rendering_func_name.c_str(), string_VkFormat(format));
                            }
                        }

                        if (rendering_info.pStencilAttachment == nullptr ||
                            rendering_info.pStencilAttachment->imageView == VK_NULL_HANDLE) {
                            VkFormat format = inheritance_rendering_info.stencilAttachmentFormat;
                            if (format != VK_FORMAT_UNDEFINED) {
                                const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                                skip |= LogError(
                                    objlist, "VUID-vkCmdExecuteCommands-pStencilAttachment-06775",
                                    "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                    "] %s is executed within a dynamic renderpass "
                                    "instance scope begun by %s(), and VkRenderingInfo::pStencilAttachment does not define an "
                                    "image view but VkCommandBufferInheritanceRenderingInfo::stencilAttachmentFormat "
                                    "is %s instead of VK_FORMAT_UNDEFINED.",
                                    i, report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                    cb_state->begin_rendering_func_name.c_str(), string_VkFormat(format));
                            }
                        }

                        if (rendering_info.viewMask != inheritance_rendering_info.viewMask) {
                            const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                            skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-viewMask-06031",
                                             "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                             "] %s is executed within a dynamic renderpass instance scope begun "
                                             "by %s(), but "
                                             "VkCommandBufferInheritanceRenderingInfo::viewMask (%" PRIu32
                                             ") does "
                                             "not match VkRenderingInfo::viewMask (%" PRIu32 ").",
                                             i, report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                             cb_state->begin_rendering_func_name.c_str(), inheritance_rendering_info.viewMask,
                                             rendering_info.viewMask);
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
                                    const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                                    skip |= LogError(
                                        objlist, "VUID-vkCmdExecuteCommands-pNext-06032",
                                        "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                        "] %s is executed within a dynamic renderpass instance "
                                        "scope begun "
                                        "by vkCmdBeginRenderingKHR(), but "
                                        "VkAttachmentSampleCountInfo(AMD/NV)::pColorAttachmentSamples at index (%" PRIu32
                                        ") "
                                        "does "
                                        "not match the sample count of the imageView in VkRenderingInfoKHR::pColorAttachments.",
                                        i, report_data->FormatHandle(pCommandBuffers[i]).c_str(), index);
                                }
                            }

                            if ((rendering_info.pDepthAttachment != nullptr) &&
                                rendering_info.pDepthAttachment->imageView != VK_NULL_HANDLE) {
                                auto image_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pDepthAttachment->imageView);

                                if (image_view_state->samples != amd_sample_count->depthStencilAttachmentSamples) {
                                    const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                                    skip |= LogError(
                                        objlist, "VUID-vkCmdExecuteCommands-pNext-06033",
                                        "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                        "] %s is executed within a dynamic renderpass instance "
                                        "scope begun "
                                        "by vkCmdBeginRenderingKHR(), but "
                                        "VkAttachmentSampleCountInfo(AMD/NV)::depthStencilAttachmentSamples does "
                                        "not match the sample count of the imageView in VkRenderingInfoKHR::pDepthAttachment.",
                                        i, report_data->FormatHandle(pCommandBuffers[i]).c_str());
                                }
                            }

                            if ((rendering_info.pStencilAttachment != nullptr) &&
                                rendering_info.pStencilAttachment->imageView != VK_NULL_HANDLE) {
                                auto image_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pStencilAttachment->imageView);

                                if (image_view_state->samples != amd_sample_count->depthStencilAttachmentSamples) {
                                    const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                                    skip |= LogError(
                                        objlist, "VUID-vkCmdExecuteCommands-pNext-06034",
                                        "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                        "] %s is executed within a dynamic renderpass instance "
                                        "scope begun "
                                        "by vkCmdBeginRenderingKHR(), but "
                                        "VkAttachmentSampleCountInfo(AMD/NV)::depthStencilAttachmentSamples does "
                                        "not match the sample count of the imageView in VkRenderingInfoKHR::pStencilAttachment.",
                                        i, report_data->FormatHandle(pCommandBuffers[i]).c_str());
                                }
                            }
                        } else {
                            for (uint32_t index = 0; index < rendering_info.colorAttachmentCount; index++) {
                                if (rendering_info.pColorAttachments[index].imageView == VK_NULL_HANDLE) {
                                    continue;
                                }
                                auto image_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pColorAttachments[index].imageView);

                                if (image_view_state->samples != inheritance_rendering_info.rasterizationSamples) {
                                    const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                                    skip |= LogError(
                                        objlist, "VUID-vkCmdExecuteCommands-pNext-06035",
                                        "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                        "] %s is executed within a dynamic renderpass instance "
                                        "scope begun "
                                        "by vkCmdBeginRenderingKHR(), but the sample count of the image view at index (%" PRIu32
                                        ") of "
                                        "VkRenderingInfoKHR::pColorAttachments does not match "
                                        "VkCommandBufferInheritanceRenderingInfoKHR::rasterizationSamples.",
                                        i, report_data->FormatHandle(pCommandBuffers[i]).c_str(), index);
                                }
                            }

                            if ((rendering_info.pDepthAttachment != nullptr) &&
                                rendering_info.pDepthAttachment->imageView != VK_NULL_HANDLE) {
                                auto image_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pDepthAttachment->imageView);

                                if (image_view_state->samples != inheritance_rendering_info.rasterizationSamples) {
                                    const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                                    skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-pNext-06036",
                                                     "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                                     "] %s is executed within a dynamic renderpass "
                                                     "instance scope begun "
                                                     "by vkCmdBeginRenderingKHR(), but the sample count of the image view for "
                                                     "VkRenderingInfoKHR::pDepthAttachment does not match "
                                                     "VkCommandBufferInheritanceRenderingInfoKHR::rasterizationSamples.",
                                                     i, report_data->FormatHandle(pCommandBuffers[i]).c_str());
                                }
                            }

                            if ((rendering_info.pStencilAttachment != nullptr) &&
                                rendering_info.pStencilAttachment->imageView != VK_NULL_HANDLE) {
                                auto image_view_state = Get<IMAGE_VIEW_STATE>(rendering_info.pStencilAttachment->imageView);

                                if (image_view_state->samples != inheritance_rendering_info.rasterizationSamples) {
                                    const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                                    skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-pNext-06037",
                                                     "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                                     "] %s is executed within a dynamic renderpass "
                                                     "instance scope begun "
                                                     "by vkCmdBeginRenderingKHR(), but the sample count of the image view for "
                                                     "VkRenderingInfoKHR::pStencilAttachment does not match "
                                                     "VkCommandBufferInheritanceRenderingInfoKHR::rasterizationSamples.",
                                                     i, report_data->FormatHandle(pCommandBuffers[i]).c_str());
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
                const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-pCommandBuffers-00091",
                                 "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                 "] Cannot execute pending %s without VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set.",
                                 i, report_data->FormatHandle(sub_cb_state->commandBuffer()).c_str());
            }
            // We use an const_cast, because one cannot query a container keyed on a non-const pointer using a const pointer
            if (cb_state->linkedCommandBuffers.count(const_cast<CMD_BUFFER_STATE *>(sub_cb_state.get()))) {
                const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-pCommandBuffers-00092",
                                 "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                 "] Cannot execute %s without VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT "
                                 "set if previously executed in %s",
                                 i, report_data->FormatHandle(sub_cb_state->commandBuffer()).c_str(),
                                 report_data->FormatHandle(cb_state->commandBuffer()).c_str());
            }

            const auto insert_pair = linked_command_buffers.insert(sub_cb_state.get());
            if (!insert_pair.second) {
                const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-pCommandBuffers-00093",
                                 "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                 "] Cannot duplicate %s in pCommandBuffers without "
                                 "VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set.",
                                 i, report_data->FormatHandle(cb_state->commandBuffer()).c_str());
            }

            if (cb_state->beginInfo.flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT) {
                // Warn that non-simultaneous secondary cmd buffer renders primary non-simultaneous
                const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
                skip |= LogWarning(objlist, kVUID_Core_DrawState_InvalidCommandBufferSimultaneousUse,
                                   "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                                   "] %s does not have "
                                   "VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set and will cause primary "
                                   "%s to be treated as if it does not have "
                                   "VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT set, even though it does.",
                                   i, report_data->FormatHandle(pCommandBuffers[i]).c_str(),
                                   report_data->FormatHandle(cb_state->commandBuffer()).c_str());
            }
        }
        if (!cb_state->activeQueries.empty() && !enabled_features.core.inheritedQueries) {
            const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
            skip |= LogError(pCommandBuffers[i], "VUID-vkCmdExecuteCommands-commandBuffer-00101",
                             "vkCmdExecuteCommands(): pCommandBuffers[%" PRIu32
                             "] %s cannot be submitted with a query in flight and "
                             "inherited queries not supported on this device.",
                             i, report_data->FormatHandle(pCommandBuffers[i]).c_str());
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
                        skip |= LogError(
                            pCommandBuffers[i], "UNASSIGNED-vkCmdExecuteCommands-commandBuffer-00001",
                            "%s: Executed secondary command buffer using %s (subresource: aspectMask 0x%x array layer %" PRIu32
                            ", "
                            "mip level %" PRIu32 ") which expects layout %s--instead, image %s layout is %s.",
                            "vkCmdExecuteCommands():", report_data->FormatHandle(image).c_str(), subresource.aspectMask,
                            subresource.arrayLayer, subresource.mipLevel, string_VkImageLayout(sub_layout), layout_type,
                            string_VkImageLayout(cb_layout));
                    }
                }
            }
        }

        // All commands buffers involved must be protected or unprotected
        if ((cb_state->unprotected == false) && (sub_cb_state->unprotected == true)) {
            const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
            skip |= LogError(
                objlist, "VUID-vkCmdExecuteCommands-commandBuffer-01820",
                "vkCmdExecuteCommands(): command buffer %s is protected while secondary command buffer %s is a unprotected",
                report_data->FormatHandle(cb_state->commandBuffer()).c_str(),
                report_data->FormatHandle(sub_cb_state->commandBuffer()).c_str());
        } else if ((cb_state->unprotected == true) && (sub_cb_state->unprotected == false)) {
            const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
            skip |= LogError(
                objlist, "VUID-vkCmdExecuteCommands-commandBuffer-01821",
                "vkCmdExecuteCommands(): command buffer %s is unprotected while secondary command buffer %s is a protected",
                report_data->FormatHandle(cb_state->commandBuffer()).c_str(),
                report_data->FormatHandle(sub_cb_state->commandBuffer()).c_str());
        }
        if (active_occlusion_query && sub_cb_state->inheritanceInfo.occlusionQueryEnable != VK_TRUE) {
            const LogObjectList objlist(commandBuffer, pCommandBuffers[i]);
            skip |= LogError(objlist, "VUID-vkCmdExecuteCommands-commandBuffer-00102",
                             "vkCmdExecuteCommands(): command buffer %s has an active occlusion query, but secondary command "
                             "buffer %s (pCommandBuffers[%" PRIu32
                             "]) was recorded with VkCommandBufferInheritanceInfo::occlusionQueryEnable set to VK_FALSE",
                             report_data->FormatHandle(cb_state->commandBuffer()).c_str(),
                             report_data->FormatHandle(sub_cb_state->commandBuffer()).c_str(), i);
        }
    }

    if (cb_state->transform_feedback_active) {
        skip |= LogError(commandBuffer, "VUID-vkCmdExecuteCommands-None-02286",
                         "vkCmdExecuteCommands(): transform feedback is active.");
    }

    skip |= ValidateCmd(*cb_state, CMD_EXECUTECOMMANDS);
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
        const LogObjectList objlist(commandBuffer, buffer_state->buffer());
        skip |= LogError(objlist, vuid,
                         "stride[%d] * (drawCount[%d] - 1) + offset[%" PRIx64 "] + sizeof(%s)[%d] = %" PRIx64
                         " is greater than the buffer size[%" PRIx64 "].",
                         stride, drawCount, offset, struct_name, struct_size, validation_value, buffer_state->createInfo.size);
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
            const LogObjectList objlist(commandBuffer, pBuffers[i]);
            skip |= LogError(objlist, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pOffsets-02358",
                             "%s: pOffset[%" PRIu32 "](0x%" PRIxLEAST64
                             ") is greater than or equal to the size of pBuffers[%" PRIu32 "](0x%" PRIxLEAST64 ").",
                             cmd_name, i, pOffsets[i], i, buffer_state->createInfo.size);
        }

        if ((buffer_state->createInfo.usage & VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT) == 0) {
            const LogObjectList objlist(commandBuffer, pBuffers[i]);
            skip |= LogError(objlist, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pBuffers-02360",
                             "%s: pBuffers[%" PRIu32
                             "]  was not created with the VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT flag.",
                             cmd_name, i);
        }

        // pSizes is optional and may be nullptr. Also might be VK_WHOLE_SIZE which VU don't apply
        if ((pSizes != nullptr) && (pSizes[i] != VK_WHOLE_SIZE)) {
            // only report one to prevent redundant error if the size is larger since adding offset will be as well
            if (pSizes[i] > buffer_state->createInfo.size) {
                const LogObjectList objlist(commandBuffer, pBuffers[i]);
                skip |= LogError(objlist, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pSizes-02362",
                                 "%s: pSizes[%" PRIu32 "](0x%" PRIxLEAST64 ") is greater than the size of pBuffers[%" PRIu32
                                 "](0x%" PRIxLEAST64 ").",
                                 cmd_name, i, pSizes[i], i, buffer_state->createInfo.size);
            } else if (pOffsets[i] + pSizes[i] > buffer_state->createInfo.size) {
                const LogObjectList objlist(commandBuffer, pBuffers[i]);
                skip |= LogError(objlist, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pOffsets-02363",
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
                    const LogObjectList objlist(commandBuffer, pCounterBuffers[i]);
                    skip |= LogError(objlist, "VUID-vkCmdBeginTransformFeedbackEXT-pCounterBufferOffsets-02370",
                                     "%s: pCounterBuffers[%" PRIu32
                                     "] is not large enough to hold 4 bytes at pCounterBufferOffsets[%" PRIu32 "](0x%" PRIx64 ").",
                                     cmd_name, i, i, pCounterBufferOffsets[i]);
                }

                if ((buffer_state->createInfo.usage & VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT) == 0) {
                    const LogObjectList objlist(commandBuffer, pCounterBuffers[i]);
                    skip |= LogError(objlist, "VUID-vkCmdBeginTransformFeedbackEXT-pCounterBuffers-02372",
                                     "%s: pCounterBuffers[%" PRIu32
                                     "] was not created with the VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT flag.",
                                     cmd_name, i);
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
                    const LogObjectList objlist(commandBuffer, pCounterBuffers[i]);
                    skip |= LogError(objlist, "VUID-vkCmdEndTransformFeedbackEXT-pCounterBufferOffsets-02378",
                                     "%s: pCounterBuffers[%" PRIu32
                                     "] is not large enough to hold 4 bytes at pCounterBufferOffsets[%" PRIu32 "](0x%" PRIx64 ").",
                                     cmd_name, i, i, pCounterBufferOffsets[i]);
                }

                if ((buffer_state->createInfo.usage & VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT) == 0) {
                    const LogObjectList objlist(commandBuffer, pCounterBuffers[i]);
                    skip |= LogError(objlist, "VUID-vkCmdEndTransformFeedbackEXT-pCounterBuffers-02380",
                                     "%s: pCounterBuffers[%" PRIu32
                                     "] was not created with the VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT flag.",
                                     cmd_name, i);
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
                const LogObjectList objlist(commandBuffer, pBuffers[i]);
                skip |= LogError(objlist, "VUID-vkCmdBindVertexBuffers2-pOffsets-03357",
                                 "%s offset (0x%" PRIxLEAST64 ") is beyond the end of the buffer.", api_call, pOffsets[i]);
            }
            if (pSizes && pOffsets[i] + pSizes[i] > buffer_state->createInfo.size) {
                const LogObjectList objlist(commandBuffer, pBuffers[i]);
                skip |= LogError(objlist, "VUID-vkCmdBindVertexBuffers2-pSizes-03358",
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
                const LogObjectList objlist(commandBuffer, buffer_state->buffer());
                skip |= LogError(commandBuffer, "VUID-VkConditionalRenderingBeginInfoEXT-buffer-01982",
                                 "vkCmdBeginConditionalRenderingEXT(): pConditionalRenderingBegin->buffer (%s) was not create with "
                                 "VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT bit.",
                                 report_data->FormatHandle(pConditionalRenderingBegin->buffer).c_str());
            }
            if (pConditionalRenderingBegin->offset + 4 > buffer_state->createInfo.size) {
                const LogObjectList objlist(commandBuffer, buffer_state->buffer());
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
            cb_state->conditional_rendering_subpass != cb_state->GetActiveSubpass()) {
            skip |= LogError(commandBuffer, "VUID-vkCmdEndConditionalRenderingEXT-None-01987",
                             "vkCmdBeginConditionalRenderingEXT(): Conditional rendering was begun in subpass %" PRIu32
                             ", but the current subpass is %" PRIu32 ".",
                             cb_state->conditional_rendering_subpass, cb_state->GetActiveSubpass());
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
        const LogObjectList objlist(commandBuffer, imageView);
        skip |= LogError(objlist, "VUID-vkCmdBindShadingRateImageNV-imageView-02059",
                         "vkCmdBindShadingRateImageNV: If imageView is not VK_NULL_HANDLE, it must be a valid "
                         "VkImageView handle.");
        return skip;
    }
    const auto &ivci = view_state->create_info;
    if (ivci.viewType != VK_IMAGE_VIEW_TYPE_2D && ivci.viewType != VK_IMAGE_VIEW_TYPE_2D_ARRAY) {
        const LogObjectList objlist(commandBuffer, imageView);
        skip |= LogError(objlist, "VUID-vkCmdBindShadingRateImageNV-imageView-02059",
                         "vkCmdBindShadingRateImageNV: If imageView is not VK_NULL_HANDLE, it must be a valid "
                         "VkImageView handle of type VK_IMAGE_VIEW_TYPE_2D or VK_IMAGE_VIEW_TYPE_2D_ARRAY.");
    }

    if (ivci.format != VK_FORMAT_R8_UINT) {
        const LogObjectList objlist(commandBuffer, imageView);
        skip |= LogError(objlist, "VUID-vkCmdBindShadingRateImageNV-imageView-02060",
                         "vkCmdBindShadingRateImageNV: If imageView is not VK_NULL_HANDLE, it must have a format of "
                         "VK_FORMAT_R8_UINT but is %s.",
                         string_VkFormat(ivci.format));
    }

    const auto *image_state = view_state->image_state.get();
    auto usage = image_state->createInfo.usage;
    if (!(usage & VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV)) {
        const LogObjectList objlist(commandBuffer, imageView);
        skip |= LogError(objlist, "VUID-vkCmdBindShadingRateImageNV-imageView-02061",
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

void CoreChecks::PostCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT *pLabelInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    cb_state->BeginLabel();
}

bool CoreChecks::PreCallValidateCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    bool skip = false;

    if (cb_state->IsPrimary()) {
        return skip;
    }

    if (cb_state->LabelStackDepth() < 1) {
        skip |= LogError(commandBuffer, "VUID-vkCmdEndDebugUtilsLabelEXT-commandBuffer-01913",
                         "vkCmdEndDebugUtilsLabelEXT() called without a corresponding vkCmdBeginDebugUtilsLabelEXT first");
    }
    return skip;
}

void CoreChecks::PostCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    cb_state->EndLabel();
}
