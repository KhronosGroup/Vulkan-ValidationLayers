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

#include <algorithm>
#include <assert.h>
#include <sstream>
#include <vector>

#include <vulkan/vk_enum_string_helper.h>
#include "generated/chassis.h"
#include "core_validation.h"
#include "sync/sync_utils.h"
#include "utils/convert_utils.h"

bool CoreChecks::LogInvalidAttachmentMessage(const char *type1_string, const RENDER_PASS_STATE &rp1_state, const char *type2_string,
                                             const RENDER_PASS_STATE &rp2_state, uint32_t primary_attach, uint32_t secondary_attach,
                                             const char *msg, const Location &loc, const char *error_code) const {
    const LogObjectList objlist(rp1_state.renderPass(), rp2_state.renderPass());
    return LogError(error_code, objlist, loc,
                    "RenderPasses incompatible between %s w/ %s and %s w/ %s Attachment %" PRIu32
                    " is not "
                    "compatible with %" PRIu32 ": %s.",
                    type1_string, FormatHandle(rp1_state).c_str(), type2_string, FormatHandle(rp2_state).c_str(), primary_attach,
                    secondary_attach, msg);
}

bool CoreChecks::ValidateAttachmentCompatibility(const char *type1_string, const RENDER_PASS_STATE &rp1_state,
                                                 const char *type2_string, const RENDER_PASS_STATE &rp2_state,
                                                 uint32_t primary_attach, uint32_t secondary_attach, const Location &loc,
                                                 const char *error_code) const {
    bool skip = false;
    const auto &primary_pass_ci = rp1_state.createInfo;
    const auto &secondary_pass_ci = rp2_state.createInfo;
    if (primary_pass_ci.attachmentCount <= primary_attach) {
        primary_attach = VK_ATTACHMENT_UNUSED;
    }
    if (secondary_pass_ci.attachmentCount <= secondary_attach) {
        secondary_attach = VK_ATTACHMENT_UNUSED;
    }
    if (primary_attach == VK_ATTACHMENT_UNUSED && secondary_attach == VK_ATTACHMENT_UNUSED) {
        return skip;
    }
    if (primary_attach == VK_ATTACHMENT_UNUSED) {
        skip |= LogInvalidAttachmentMessage(type1_string, rp1_state, type2_string, rp2_state, primary_attach, secondary_attach,
                                            "The first is unused while the second is not.", loc, error_code);
        return skip;
    }
    if (secondary_attach == VK_ATTACHMENT_UNUSED) {
        skip |= LogInvalidAttachmentMessage(type1_string, rp1_state, type2_string, rp2_state, primary_attach, secondary_attach,
                                            "The second is unused while the first is not.", loc, error_code);
        return skip;
    }
    if (primary_pass_ci.pAttachments[primary_attach].format != secondary_pass_ci.pAttachments[secondary_attach].format) {
        skip |= LogInvalidAttachmentMessage(type1_string, rp1_state, type2_string, rp2_state, primary_attach, secondary_attach,
                                            "They have different formats.", loc, error_code);
    }
    if (primary_pass_ci.pAttachments[primary_attach].samples != secondary_pass_ci.pAttachments[secondary_attach].samples) {
        skip |= LogInvalidAttachmentMessage(type1_string, rp1_state, type2_string, rp2_state, primary_attach, secondary_attach,
                                            "They have different samples.", loc, error_code);
    }
    if (primary_pass_ci.pAttachments[primary_attach].flags != secondary_pass_ci.pAttachments[secondary_attach].flags) {
        skip |= LogInvalidAttachmentMessage(type1_string, rp1_state, type2_string, rp2_state, primary_attach, secondary_attach,
                                            "They have different flags.", loc, error_code);
    }

    return skip;
}

bool CoreChecks::ValidateSubpassCompatibility(const char *type1_string, const RENDER_PASS_STATE &rp1_state,
                                              const char *type2_string, const RENDER_PASS_STATE &rp2_state, const int subpass,
                                              const Location &loc, const char *error_code) const {
    bool skip = false;
    const auto &primary_desc = rp1_state.createInfo.pSubpasses[subpass];
    const auto &secondary_desc = rp2_state.createInfo.pSubpasses[subpass];
    uint32_t max_input_attachment_count = std::max(primary_desc.inputAttachmentCount, secondary_desc.inputAttachmentCount);
    for (uint32_t i = 0; i < max_input_attachment_count; ++i) {
        uint32_t primary_input_attach = VK_ATTACHMENT_UNUSED, secondary_input_attach = VK_ATTACHMENT_UNUSED;
        if (i < primary_desc.inputAttachmentCount) {
            primary_input_attach = primary_desc.pInputAttachments[i].attachment;
        }
        if (i < secondary_desc.inputAttachmentCount) {
            secondary_input_attach = secondary_desc.pInputAttachments[i].attachment;
        }
        skip |= ValidateAttachmentCompatibility(type1_string, rp1_state, type2_string, rp2_state, primary_input_attach,
                                                secondary_input_attach, loc, error_code);
    }
    uint32_t max_color_attachment_count = std::max(primary_desc.colorAttachmentCount, secondary_desc.colorAttachmentCount);
    for (uint32_t i = 0; i < max_color_attachment_count; ++i) {
        uint32_t primary_color_attach = VK_ATTACHMENT_UNUSED, secondary_color_attach = VK_ATTACHMENT_UNUSED;
        if (i < primary_desc.colorAttachmentCount) {
            primary_color_attach = primary_desc.pColorAttachments[i].attachment;
        }
        if (i < secondary_desc.colorAttachmentCount) {
            secondary_color_attach = secondary_desc.pColorAttachments[i].attachment;
        }
        skip |= ValidateAttachmentCompatibility(type1_string, rp1_state, type2_string, rp2_state, primary_color_attach,
                                                secondary_color_attach, loc, error_code);
        if (rp1_state.createInfo.subpassCount > 1) {
            uint32_t primary_resolve_attach = VK_ATTACHMENT_UNUSED, secondary_resolve_attach = VK_ATTACHMENT_UNUSED;
            if (i < primary_desc.colorAttachmentCount && primary_desc.pResolveAttachments) {
                primary_resolve_attach = primary_desc.pResolveAttachments[i].attachment;
            }
            if (i < secondary_desc.colorAttachmentCount && secondary_desc.pResolveAttachments) {
                secondary_resolve_attach = secondary_desc.pResolveAttachments[i].attachment;
            }
            skip |= ValidateAttachmentCompatibility(type1_string, rp1_state, type2_string, rp2_state, primary_resolve_attach,
                                                    secondary_resolve_attach, loc, error_code);
        }
    }
    uint32_t primary_depthstencil_attach = VK_ATTACHMENT_UNUSED, secondary_depthstencil_attach = VK_ATTACHMENT_UNUSED;
    if (primary_desc.pDepthStencilAttachment) {
        primary_depthstencil_attach = primary_desc.pDepthStencilAttachment[0].attachment;
    }
    if (secondary_desc.pDepthStencilAttachment) {
        secondary_depthstencil_attach = secondary_desc.pDepthStencilAttachment[0].attachment;
    }
    skip |= ValidateAttachmentCompatibility(type1_string, rp1_state, type2_string, rp2_state, primary_depthstencil_attach,
                                            secondary_depthstencil_attach, loc, error_code);

    // Both renderpasses must agree on Multiview usage
    if (primary_desc.viewMask && secondary_desc.viewMask) {
        if (primary_desc.viewMask != secondary_desc.viewMask) {
            std::stringstream ss;
            ss << "For subpass " << subpass << ", they have a different viewMask. The first has view mask " << primary_desc.viewMask
               << " while the second has view mask " << secondary_desc.viewMask << ".";
            skip |= LogInvalidPnextMessage(type1_string, rp1_state, type2_string, rp2_state, ss.str().c_str(), loc, error_code);
        }
    } else if (primary_desc.viewMask) {
        skip |= LogInvalidPnextMessage(type1_string, rp1_state, type2_string, rp2_state,
                                       "The first uses Multiview (has non-zero viewMasks) while the second one does not.", loc,
                                       error_code);
    } else if (secondary_desc.viewMask) {
        skip |= LogInvalidPnextMessage(type1_string, rp1_state, type2_string, rp2_state,
                                       "The second uses Multiview (has non-zero viewMasks) while the first one does not.", loc,
                                       error_code);
    }

    // Find Fragment Shading Rate attachment entries in render passes if they
    // exist.
    const auto fsr1 = vku::FindStructInPNextChain<VkFragmentShadingRateAttachmentInfoKHR>(primary_desc.pNext);
    const auto fsr2 = vku::FindStructInPNextChain<VkFragmentShadingRateAttachmentInfoKHR>(secondary_desc.pNext);

    if (fsr1 && fsr2) {
        if ((fsr1->shadingRateAttachmentTexelSize.width != fsr2->shadingRateAttachmentTexelSize.width) ||
            (fsr1->shadingRateAttachmentTexelSize.height != fsr2->shadingRateAttachmentTexelSize.height)) {
            std::stringstream ss;
            ss << "Shading rate attachment texel sizes do not match (width is " << fsr1->shadingRateAttachmentTexelSize.width
               << " and " << fsr2->shadingRateAttachmentTexelSize.width << ", height is "
               << fsr1->shadingRateAttachmentTexelSize.height << " and " << fsr1->shadingRateAttachmentTexelSize.height << ".";
            skip |= LogInvalidPnextMessage(type1_string, rp1_state, type2_string, rp2_state, ss.str().c_str(), loc, error_code);
        }
    } else if (fsr1) {
        skip |= LogInvalidPnextMessage(type1_string, rp1_state, type2_string, rp2_state,
                                       "The first uses a fragment "
                                       "shading rate attachment while "
                                       "the second one does not.",
                                       loc, error_code);
    } else if (fsr2) {
        skip |= LogInvalidPnextMessage(type1_string, rp1_state, type2_string, rp2_state,
                                       "The second uses a fragment "
                                       "shading rate attachment while "
                                       "the first one does not.",
                                       loc, error_code);
    }

    return skip;
}

bool CoreChecks::ValidateDependencyCompatibility(const char *type1_string, const RENDER_PASS_STATE &rp1_state,
                                                 const char *type2_string, const RENDER_PASS_STATE &rp2_state,
                                                 const uint32_t dependency, const Location &loc, const char *error_code) const {
    bool skip = false;

    const auto &primary_dep = rp1_state.createInfo.pDependencies[dependency];
    const auto &secondary_dep = rp2_state.createInfo.pDependencies[dependency];

    VkPipelineStageFlags2 primary_src_stage_mask = primary_dep.srcStageMask;
    VkPipelineStageFlags2 primary_dst_stage_mask = primary_dep.dstStageMask;
    VkAccessFlags2 primary_src_access_mask = primary_dep.srcAccessMask;
    VkAccessFlags2 primary_dst_access_mask = primary_dep.dstAccessMask;
    VkPipelineStageFlags2 secondary_src_stage_mask = secondary_dep.srcStageMask;
    VkPipelineStageFlags2 secondary_dst_stage_mask = secondary_dep.dstStageMask;
    VkAccessFlags2 secondary_src_access_mask = secondary_dep.srcAccessMask;
    VkAccessFlags2 secondary_dst_access_mask = secondary_dep.dstAccessMask;

    if (const auto primary_barrier = vku::FindStructInPNextChain<VkMemoryBarrier2KHR>(rp1_state.createInfo.pNext); primary_barrier) {
        primary_src_stage_mask = primary_barrier->srcStageMask;
        primary_dst_stage_mask = primary_barrier->dstStageMask;
        primary_src_access_mask = primary_barrier->srcAccessMask;
        primary_dst_access_mask = primary_barrier->dstAccessMask;
    }
    if (const auto secondary_barrier = vku::FindStructInPNextChain<VkMemoryBarrier2KHR>(rp2_state.createInfo.pNext); secondary_barrier) {
        secondary_src_stage_mask = secondary_barrier->srcStageMask;
        secondary_dst_stage_mask = secondary_barrier->dstStageMask;
        secondary_src_access_mask = secondary_barrier->srcAccessMask;
        secondary_dst_access_mask = secondary_barrier->dstAccessMask;
    }

    if (primary_dep.srcSubpass != secondary_dep.srcSubpass) {
        std::stringstream ss;
        ss << "First srcSubpass is " << primary_dep.srcSubpass << ", but second srcSubpass is " << secondary_dep.srcSubpass << ".";
        skip |= LogInvalidDependencyMessage(type1_string, rp1_state, type2_string, rp2_state, ss.str().c_str(), loc, error_code);
    }
    if (primary_dep.dstSubpass != secondary_dep.dstSubpass) {
        std::stringstream ss;
        ss << "First dstSubpass is " << primary_dep.dstSubpass << ", but second dstSubpass is " << secondary_dep.dstSubpass << ".";
        skip |= LogInvalidDependencyMessage(type1_string, rp1_state, type2_string, rp2_state, ss.str().c_str(), loc, error_code);
    }
    if (primary_src_stage_mask != secondary_src_stage_mask) {
        std::stringstream ss;
        ss << "First srcStageMask is " << string_VkPipelineStageFlags2(primary_src_stage_mask) << ", but second srcStageMask is "
           << string_VkPipelineStageFlags2(secondary_src_stage_mask) << ".";
        skip |= LogInvalidDependencyMessage(type1_string, rp1_state, type2_string, rp2_state, ss.str().c_str(), loc, error_code);
    }
    if (primary_dst_stage_mask != secondary_dst_stage_mask) {
        std::stringstream ss;
        ss << "First dstStageMask is " << string_VkPipelineStageFlags2(primary_dst_stage_mask) << ", but second dstStageMask is "
           << string_VkPipelineStageFlags2(secondary_dst_stage_mask) << ".";
        skip |= LogInvalidDependencyMessage(type1_string, rp1_state, type2_string, rp2_state, ss.str().c_str(), loc, error_code);
    }
    if (primary_src_access_mask != secondary_src_access_mask) {
        std::stringstream ss;
        ss << "First srcAccessMask is " << string_VkAccessFlags2(primary_src_access_mask) << ", but second srcAccessMask is "
           << string_VkAccessFlags2(secondary_src_access_mask) << ".";
        skip |= LogInvalidDependencyMessage(type1_string, rp1_state, type2_string, rp2_state, ss.str().c_str(), loc, error_code);
    }
    if (primary_dst_access_mask != secondary_dst_access_mask) {
        std::stringstream ss;
        ss << "First dstAccessMask is " << string_VkAccessFlags2(primary_dst_access_mask) << ", but second dstAccessMask is "
           << string_VkAccessFlags2(secondary_dst_access_mask) << ".";
        skip |= LogInvalidDependencyMessage(type1_string, rp1_state, type2_string, rp2_state, ss.str().c_str(), loc, error_code);
    }
    if (primary_dep.dependencyFlags != secondary_dep.dependencyFlags) {
        std::stringstream ss;
        ss << "First dependencyFlags are " << string_VkDependencyFlags(primary_dep.dependencyFlags)
           << ", but second dependencyFlags are " << string_VkDependencyFlags(secondary_dep.dependencyFlags) << ".";
        skip |= LogInvalidDependencyMessage(type1_string, rp1_state, type2_string, rp2_state, ss.str().c_str(), loc, error_code);
    }
    if (primary_dep.viewOffset != secondary_dep.viewOffset) {
        std::stringstream ss;
        ss << "First viewOffset are " << primary_dep.viewOffset << ", but second viewOffset are " << secondary_dep.viewOffset
           << ".";
        skip |= LogInvalidDependencyMessage(type1_string, rp1_state, type2_string, rp2_state, ss.str().c_str(), loc, error_code);
    }

    return skip;
}

bool CoreChecks::LogInvalidPnextMessage(const char *type1_string, const RENDER_PASS_STATE &rp1_state, const char *type2_string,
                                        const RENDER_PASS_STATE &rp2_state, const char *msg, const Location &loc,
                                        const char *error_code) const {
    const LogObjectList objlist(rp1_state.renderPass(), rp2_state.renderPass());
    return LogError(error_code, objlist, loc, "RenderPasses incompatible between %s w/ %s and %s w/ %s: %s", type1_string,
                    FormatHandle(rp1_state).c_str(), type2_string, FormatHandle(rp2_state).c_str(), msg);
}

bool CoreChecks::LogInvalidDependencyMessage(const char *type1_string, const RENDER_PASS_STATE &rp1_state, const char *type2_string,
                                             const RENDER_PASS_STATE &rp2_state, const char *msg, const Location &loc,
                                             const char *error_code) const {
    const LogObjectList objlist(rp1_state.renderPass(), rp2_state.renderPass());
    return LogError(error_code, objlist, loc, "RenderPasses incompatible between %s w/ %s and %s w/ %s: %s", type1_string,
                    FormatHandle(rp1_state).c_str(), type2_string, FormatHandle(rp2_state).c_str(), msg);
}

// Verify that given renderPass CreateInfo for primary and secondary command buffers are compatible.
//  This function deals directly with the CreateInfo, there are overloaded versions below that can take the renderPass handle and
//  will then feed into this function
bool CoreChecks::ValidateRenderPassCompatibility(const char *type1_string, const RENDER_PASS_STATE &rp1_state,
                                                 const char *type2_string, const RENDER_PASS_STATE &rp2_state, const Location &loc,
                                                 const char *vuid) const {
    bool skip = false;

    // createInfo flags must be identical for the renderpasses to be compatible.
    if (rp1_state.createInfo.flags != rp2_state.createInfo.flags) {
        const LogObjectList objlist(rp1_state.renderPass(), rp2_state.renderPass());
        skip |= LogError(vuid, objlist, loc,
                         "RenderPasses incompatible between %s w/ %s with flags of %" PRIu32
                         " and %s w/ "
                         "%s with a flags of %" PRIu32 ".",
                         type1_string, FormatHandle(rp1_state).c_str(), rp1_state.createInfo.flags, type2_string,
                         FormatHandle(rp2_state).c_str(), rp2_state.createInfo.flags);
    }

    if (rp1_state.createInfo.subpassCount != rp2_state.createInfo.subpassCount) {
        const LogObjectList objlist(rp1_state.renderPass(), rp2_state.renderPass());
        skip |= LogError(vuid, objlist, loc,
                         "RenderPasses incompatible between %s w/ %s with a subpassCount of %" PRIu32
                         " and %s w/ "
                         "%s with a subpassCount of %" PRIu32 ".",
                         type1_string, FormatHandle(rp1_state).c_str(), rp1_state.createInfo.subpassCount, type2_string,
                         FormatHandle(rp2_state).c_str(), rp2_state.createInfo.subpassCount);
    } else {
        for (uint32_t i = 0; i < rp1_state.createInfo.subpassCount; ++i) {
            skip |= ValidateSubpassCompatibility(type1_string, rp1_state, type2_string, rp2_state, i, loc, vuid);
        }
    }

    if (rp1_state.createInfo.dependencyCount != rp2_state.createInfo.dependencyCount) {
        const LogObjectList objlist(rp1_state.renderPass(), rp2_state.renderPass());
        skip |= LogError(vuid, objlist, loc,
                         "RenderPasses incompatible between %s w/ %s with a dependencyCount of %" PRIu32
                         " and %s w/ %s with a dependencyCount of %" PRIu32 ".",
                         type1_string, FormatHandle(rp1_state).c_str(), rp1_state.createInfo.dependencyCount, type2_string,
                         FormatHandle(rp2_state).c_str(), rp2_state.createInfo.dependencyCount);
    } else {
        for (uint32_t i = 0; i < rp1_state.createInfo.dependencyCount; ++i) {
            skip |= ValidateDependencyCompatibility(type1_string, rp1_state, type2_string, rp2_state, i, loc, vuid);
        }
    }
    if (rp1_state.createInfo.correlatedViewMaskCount != rp2_state.createInfo.correlatedViewMaskCount) {
        const LogObjectList objlist(rp1_state.renderPass(), rp2_state.renderPass());
        skip |= LogError(vuid, objlist, loc,
                         "RenderPasses incompatible between %s w/ %s with a correlatedViewMaskCount of %" PRIu32
                         " and %s w/ %s with a correlatedViewMaskCount of %" PRIu32 ".",
                         type1_string, FormatHandle(rp1_state).c_str(), rp1_state.createInfo.correlatedViewMaskCount, type2_string,
                         FormatHandle(rp2_state).c_str(), rp2_state.createInfo.correlatedViewMaskCount);
    } else {
        for (uint32_t i = 0; i < rp1_state.createInfo.correlatedViewMaskCount; ++i) {
            if (rp1_state.createInfo.pCorrelatedViewMasks[i] != rp2_state.createInfo.pCorrelatedViewMasks[i]) {
                const LogObjectList objlist(rp1_state.renderPass(), rp2_state.renderPass());
                skip |= LogError(vuid, objlist, loc,
                                 "RenderPasses incompatible between %s w/ %s with a pCorrelatedViewMasks[%" PRIu32 "] of %" PRIu32
                                 " and %s w/ %s with a pCorrelatedViewMasks[%" PRIu32 "] of %" PRIu32 ".",
                                 type1_string, FormatHandle(rp1_state).c_str(), i, rp1_state.createInfo.pCorrelatedViewMasks[i],
                                 type2_string, FormatHandle(rp2_state).c_str(), i, rp1_state.createInfo.pCorrelatedViewMasks[i]);
            }
        }
    }

    // Find an entry of the Fragment Density Map type in the pNext chain, if it exists
    const auto fdm1 = vku::FindStructInPNextChain<VkRenderPassFragmentDensityMapCreateInfoEXT>(rp1_state.createInfo.pNext);
    const auto fdm2 = vku::FindStructInPNextChain<VkRenderPassFragmentDensityMapCreateInfoEXT>(rp2_state.createInfo.pNext);

    // Both renderpasses must agree on usage of a Fragment Density Map type
    if (fdm1 && fdm2) {
        uint32_t primary_input_attach = fdm1->fragmentDensityMapAttachment.attachment;
        uint32_t secondary_input_attach = fdm2->fragmentDensityMapAttachment.attachment;
        skip |= ValidateAttachmentCompatibility(type1_string, rp1_state, type2_string, rp2_state, primary_input_attach,
                                                secondary_input_attach, loc, vuid);
    } else if (fdm1) {
        skip |= LogInvalidPnextMessage(type1_string, rp1_state, type2_string, rp2_state,
                                       "The first uses a Fragment Density Map while the second one does not.", loc, vuid);
    } else if (fdm2) {
        skip |= LogInvalidPnextMessage(type1_string, rp1_state, type2_string, rp2_state,
                                       "The second uses a Fragment Density Map while the first one does not.", loc, vuid);
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks *pAllocator,
                                                  const ErrorObject &error_obj) const {
    auto rp_state = Get<RENDER_PASS_STATE>(renderPass);
    bool skip = false;
    if (rp_state) {
        skip |= ValidateObjectNotInUse(rp_state.get(), error_obj.location, "VUID-vkDestroyRenderPass-renderPass-00873");
    }
    return skip;
}

// If this is a stencil format, make sure the stencil[Load|Store]Op flag is checked, while if it is a depth/color attachment the
// [load|store]Op flag must be checked
// TODO: The memory valid flag in DEVICE_MEMORY_STATE should probably be split to track the validity of stencil memory separately.
template <typename T>
static bool FormatSpecificLoadAndStoreOpSettings(VkFormat format, T color_depth_op, T stencil_op, T op) {
    if (color_depth_op != op && stencil_op != op) {
        return false;
    }
    const bool check_color_depth_load_op = !vkuFormatIsStencilOnly(format);
    const bool check_stencil_load_op = vkuFormatIsDepthAndStencil(format) || !check_color_depth_load_op;

    return ((check_color_depth_load_op && (color_depth_op == op)) || (check_stencil_load_op && (stencil_op == op)));
}

bool CoreChecks::ValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                            const ErrorObject &error_obj) const {
    bool skip = false;
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    const auto &rp_state = *Get<RENDER_PASS_STATE>(pRenderPassBegin->renderPass);
    const auto &fb_state = *Get<FRAMEBUFFER_STATE>(pRenderPassBegin->framebuffer);
    const Location rp_begin_loc = error_obj.location.dot(Field::pRenderPassBegin);

    skip |= ValidateCmd(cb_state, error_obj.location);

    uint32_t clear_op_size = 0;  // Make sure pClearValues is at least as large as last LOAD_OP_CLEAR

    // Handle extension struct from EXT_sample_locations
    const auto *sample_locations_begin_info = vku::FindStructInPNextChain<VkRenderPassSampleLocationsBeginInfoEXT>(pRenderPassBegin->pNext);
    if (sample_locations_begin_info) {
        for (uint32_t i = 0; i < sample_locations_begin_info->attachmentInitialSampleLocationsCount; ++i) {
            const Location sampler_loc =
                rp_begin_loc.pNext(Struct::VkRenderPassSampleLocationsBeginInfoEXT, Field::pAttachmentInitialSampleLocations, i);
            const VkAttachmentSampleLocationsEXT &sample_location =
                sample_locations_begin_info->pAttachmentInitialSampleLocations[i];
            skip |= ValidateSampleLocationsInfo(&sample_location.sampleLocationsInfo, sampler_loc.dot(Field::sampleLocationsInfo));
            if (sample_location.attachmentIndex >= rp_state.createInfo.attachmentCount) {
                const LogObjectList objlist(commandBuffer, pRenderPassBegin->renderPass);
                skip |= LogError(
                    "VUID-VkAttachmentSampleLocationsEXT-attachmentIndex-01531", objlist, sampler_loc.dot(Field::attachmentIndex),
                    "(%" PRIu32 ") is greater than the attachment count of %" PRIu32 " for the render pass being begun.",
                    sample_location.attachmentIndex, rp_state.createInfo.attachmentCount);
            }
        }

        for (uint32_t i = 0; i < sample_locations_begin_info->postSubpassSampleLocationsCount; ++i) {
            const Location sampler_loc =
                rp_begin_loc.pNext(Struct::VkRenderPassSampleLocationsBeginInfoEXT, Field::pPostSubpassSampleLocations, i);
            const VkSubpassSampleLocationsEXT &sample_location = sample_locations_begin_info->pPostSubpassSampleLocations[i];
            skip |= ValidateSampleLocationsInfo(&sample_location.sampleLocationsInfo, sampler_loc.dot(Field::sampleLocationsInfo));
            if (sample_location.subpassIndex >= rp_state.createInfo.subpassCount) {
                const LogObjectList objlist(commandBuffer, pRenderPassBegin->renderPass);
                skip |=
                    LogError("VUID-VkSubpassSampleLocationsEXT-subpassIndex-01532", objlist, sampler_loc.dot(Field::subpassIndex),
                             "(%" PRIu32 ") is greater than the subpass count of %" PRIu32 " for the render pass being begun.",
                             sample_location.subpassIndex, rp_state.createInfo.subpassCount);
            }
        }
    }

    for (uint32_t i = 0; i < rp_state.createInfo.attachmentCount; ++i) {
        auto attachment = &rp_state.createInfo.pAttachments[i];
        if (FormatSpecificLoadAndStoreOpSettings(attachment->format, attachment->loadOp, attachment->stencilLoadOp,
                                                 VK_ATTACHMENT_LOAD_OP_CLEAR)) {
            clear_op_size = static_cast<uint32_t>(i) + 1;

            if (vkuFormatHasDepth(attachment->format) && pRenderPassBegin->pClearValues) {
                skip |= ValidateClearDepthStencilValue(commandBuffer, pRenderPassBegin->pClearValues[i].depthStencil,
                                                       rp_begin_loc.dot(Field::pClearValues, i).dot(Field::depthStencil));
            }
        }
    }

    if (clear_op_size > pRenderPassBegin->clearValueCount) {
        const LogObjectList objlist(commandBuffer, pRenderPassBegin->renderPass);
        skip |= LogError("VUID-VkRenderPassBeginInfo-clearValueCount-00902", objlist, rp_begin_loc.dot(Field::clearValueCount),
                         "is %" PRIu32 " but there must be at least %" PRIu32
                         " entries in pClearValues array to account for the highest index attachment in %s that uses "
                         "VK_ATTACHMENT_LOAD_OP_CLEAR is %" PRIu32
                         ". Note that the pClearValues array is indexed by "
                         "attachment number so even if some pClearValues entries between 0 and %" PRIu32
                         " correspond to attachments "
                         "that aren't cleared they will be ignored.",
                         pRenderPassBegin->clearValueCount, clear_op_size, FormatHandle(rp_state.renderPass()).c_str(),
                         clear_op_size, clear_op_size - 1);
    }
    skip |= VerifyFramebufferAndRenderPassImageViews(pRenderPassBegin, rp_begin_loc);
    skip |= VerifyRenderAreaBounds(pRenderPassBegin, rp_begin_loc);

    skip |= VerifyFramebufferAndRenderPassLayouts(cb_state, pRenderPassBegin, fb_state, rp_begin_loc);
    if (fb_state.rp_state->renderPass() != rp_state.renderPass()) {
        skip |= ValidateRenderPassCompatibility("render pass", rp_state, "framebuffer", *fb_state.rp_state, rp_begin_loc,
                                                "VUID-VkRenderPassBeginInfo-renderPass-00904");
    }

    skip |= ValidateDependencies(fb_state, rp_state, error_obj);

    auto chained_device_group_struct = vku::FindStructInPNextChain<VkDeviceGroupRenderPassBeginInfo>(pRenderPassBegin->pNext);
    const bool non_zero_device_render_area = chained_device_group_struct && chained_device_group_struct->deviceRenderAreaCount != 0;
    if (chained_device_group_struct) {
        const LogObjectList objlist(commandBuffer, pRenderPassBegin->renderPass);
        skip |=
            ValidateDeviceMaskToPhysicalDeviceCount(chained_device_group_struct->deviceMask, objlist,
                                                    rp_begin_loc.pNext(Struct::VkDeviceGroupRenderPassBeginInfo, Field::deviceMask),
                                                    "VUID-VkDeviceGroupRenderPassBeginInfo-deviceMask-00905");
        skip |= ValidateDeviceMaskToZero(chained_device_group_struct->deviceMask, objlist,
                                         rp_begin_loc.pNext(Struct::VkDeviceGroupRenderPassBeginInfo, Field::deviceMask),
                                         "VUID-VkDeviceGroupRenderPassBeginInfo-deviceMask-00906");
        skip |= ValidateDeviceMaskToCommandBuffer(cb_state, chained_device_group_struct->deviceMask, objlist,
                                                  rp_begin_loc.pNext(Struct::VkDeviceGroupRenderPassBeginInfo, Field::deviceMask),
                                                  "VUID-VkDeviceGroupRenderPassBeginInfo-deviceMask-00907");

        if (chained_device_group_struct->deviceRenderAreaCount != 0 &&
            chained_device_group_struct->deviceRenderAreaCount != physical_device_count) {
            skip |= LogError("VUID-VkDeviceGroupRenderPassBeginInfo-deviceRenderAreaCount-00908", objlist,
                             rp_begin_loc.pNext(Struct::VkDeviceGroupRenderPassBeginInfo, Field::deviceRenderAreaCount),
                             "is %" PRIu32 " but the physical device count is %" PRIu32 ".",
                             chained_device_group_struct->deviceRenderAreaCount, physical_device_count);
        }
    }

    if (!non_zero_device_render_area) {
        if (pRenderPassBegin->renderArea.extent.width == 0) {
            skip |= LogError("VUID-VkRenderPassBeginInfo-None-08996", commandBuffer,
                             rp_begin_loc.dot(Field::renderArea).dot(Field::extent).dot(Field::width), "is zero.");
        } else if (pRenderPassBegin->renderArea.extent.height == 0) {
            skip |= LogError("VUID-VkRenderPassBeginInfo-None-08997", commandBuffer,
                             rp_begin_loc.dot(Field::renderArea).dot(Field::extent).dot(Field::height), "is zero.");
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                   VkSubpassContents contents, const ErrorObject &error_obj) const {
    bool skip = ValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, error_obj);
    return skip;
}

bool CoreChecks::PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                       const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                       const ErrorObject &error_obj) const {
    return PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, error_obj);
}

bool CoreChecks::PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                    const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                    const ErrorObject &error_obj) const {
    bool skip = ValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, error_obj);
    return skip;
}

void CoreChecks::RecordCmdBeginRenderPassLayouts(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                 const VkSubpassContents contents) {
    if (!pRenderPassBegin) {
        return;
    }
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto render_pass_state = Get<RENDER_PASS_STATE>(pRenderPassBegin->renderPass);
    if (cb_state && render_pass_state) {
        // transition attachments to the correct layouts for beginning of renderPass and first subpass
        TransitionBeginRenderPassLayouts(cb_state.get(), *render_pass_state);
    }
}

void CoreChecks::PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                 VkSubpassContents contents) {
    StateTracker::PreCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    RecordCmdBeginRenderPassLayouts(commandBuffer, pRenderPassBegin, contents);
}

void CoreChecks::PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                     const VkSubpassBeginInfo *pSubpassBeginInfo) {
    StateTracker::PreCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    RecordCmdBeginRenderPassLayouts(commandBuffer, pRenderPassBegin, pSubpassBeginInfo->contents);
}

void CoreChecks::PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                  const VkSubpassBeginInfo *pSubpassBeginInfo) {
    StateTracker::PreCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    RecordCmdBeginRenderPassLayouts(commandBuffer, pRenderPassBegin, pSubpassBeginInfo->contents);
}

bool CoreChecks::ValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                          const ErrorObject &error_obj) const {
    const auto &cb_state = *GetRead<CMD_BUFFER_STATE>(commandBuffer);
    bool skip = false;
    const bool use_rp2 = error_obj.location.function != Func::vkCmdEndRenderPass;
    const char *vuid;

    skip |= ValidateCmd(cb_state, error_obj.location);

    const auto *rp_state_ptr = cb_state.activeRenderPass.get();
    if (!rp_state_ptr) {
        return skip;
    }

    const auto &rp_state = *rp_state_ptr;
    const VkRenderPassCreateInfo2 *rpci = rp_state.createInfo.ptr();
    if (!rp_state.UsesDynamicRendering() && (cb_state.GetActiveSubpass() != rp_state.createInfo.subpassCount - 1)) {
        vuid = use_rp2 ? "VUID-vkCmdEndRenderPass2-None-03103" : "VUID-vkCmdEndRenderPass-None-00910";
        const LogObjectList objlist(commandBuffer, rp_state.renderPass());
        skip |= LogError(vuid, objlist, error_obj.location, "Called before reaching final subpass.");
    }

    if (rp_state.UsesDynamicRendering()) {
        const LogObjectList objlist(commandBuffer, rp_state.renderPass());
        vuid = use_rp2 ? "VUID-vkCmdEndRenderPass2-None-06171" : "VUID-vkCmdEndRenderPass-None-06170";
        skip |= LogError(vuid, objlist, error_obj.location,
                         "Called when the render pass instance was begun with vkCmdBeginRendering().");
    }

    if (pSubpassEndInfo && pSubpassEndInfo->pNext) {
        const auto *fdm_offset_info = vku::FindStructInPNextChain<VkSubpassFragmentDensityMapOffsetEndInfoQCOM>(pSubpassEndInfo->pNext);
        if (fdm_offset_info && fdm_offset_info->fragmentDensityOffsetCount != 0) {
            if ((!enabled_features.fragmentDensityMapOffset) || (!enabled_features.fragmentDensityMap)) {
                const LogObjectList objlist(commandBuffer, rp_state.renderPass());
                skip |= LogError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-fragmentDensityMapOffsets-06503", objlist,
                                 error_obj.location.pNext(Struct::VkSubpassFragmentDensityMapOffsetEndInfoQCOM,
                                                          Field::fragmentDensityOffsetCount),
                                 "is %" PRIu32 " but must be 0 when feature is not enabled.",
                                 fdm_offset_info->fragmentDensityOffsetCount);
            }

            bool fdm_non_zero_offsets = false;
            for (uint32_t k = 0; k < fdm_offset_info->fragmentDensityOffsetCount; k++) {
                if ((fdm_offset_info->pFragmentDensityOffsets[k].x != 0) || (fdm_offset_info->pFragmentDensityOffsets[k].y != 0)) {
                    const Location offset_loc = error_obj.location.pNext(Struct::VkSubpassFragmentDensityMapOffsetEndInfoQCOM,
                                                                         Field::pFragmentDensityOffsets, k);
                    fdm_non_zero_offsets = true;
                    uint32_t width = phys_dev_ext_props.fragment_density_map_offset_props.fragmentDensityOffsetGranularity.width;
                    uint32_t height = phys_dev_ext_props.fragment_density_map_offset_props.fragmentDensityOffsetGranularity.height;

                    if (SafeModulo(fdm_offset_info->pFragmentDensityOffsets[k].x, width) != 0) {
                        const LogObjectList objlist(commandBuffer, rp_state.renderPass());
                        skip |= LogError(
                            "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-x-06512", objlist, offset_loc.dot(Field::x),
                            "(%" PRIu32 ") is not an integer multiple of fragmentDensityOffsetGranularity.width (%" PRIu32 ").",
                            fdm_offset_info->pFragmentDensityOffsets[k].x, width);
                    }

                    if (SafeModulo(fdm_offset_info->pFragmentDensityOffsets[k].y, height) != 0) {
                        const LogObjectList objlist(commandBuffer, rp_state.renderPass());
                        skip |= LogError(
                            "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-y-06513", objlist, offset_loc.dot(Field::y),
                            "(%" PRIu32 ") is not an integer multiple of fragmentDensityOffsetGranularity.height (%" PRIu32 ").",
                            fdm_offset_info->pFragmentDensityOffsets[k].y, height);
                    }
                }
            }

            const VkImageView *image_views = cb_state.activeFramebuffer.get()->createInfo.pAttachments;
            for (uint32_t i = 0; i < rpci->attachmentCount; ++i) {
                const auto &view_state = *Get<IMAGE_VIEW_STATE>(image_views[i]);
                const auto &ici = view_state.image_state->createInfo;

                if ((fdm_non_zero_offsets == true) && ((ici.flags & VK_IMAGE_CREATE_FRAGMENT_DENSITY_MAP_OFFSET_BIT_QCOM) == 0)) {
                    const LogObjectList objlist(commandBuffer, rp_state.renderPass(), view_state.image_view());
                    skip |= LogError("VUID-VkFramebufferCreateInfo-renderPass-06502", objlist, error_obj.location,
                                     "pAttachments[%" PRIu32
                                     "] is not created with flag value"
                                     " VK_IMAGE_CREATE_FRAGMENT_DENSITY_MAP_OFFSET_BIT_QCOM and renderPass"
                                     " uses non-zero fdm offsets.",
                                     i);
                }

                // fdm attachment
                const auto *fdm_attachment = vku::FindStructInPNextChain<VkRenderPassFragmentDensityMapCreateInfoEXT>(rpci->pNext);
                const VkSubpassDescription2 &subpass = rpci->pSubpasses[cb_state.GetActiveSubpass()];
                if (fdm_attachment && fdm_attachment->fragmentDensityMapAttachment.attachment != VK_ATTACHMENT_UNUSED) {
                    if (fdm_attachment->fragmentDensityMapAttachment.attachment == i) {
                        if ((ici.flags & VK_IMAGE_CREATE_FRAGMENT_DENSITY_MAP_OFFSET_BIT_QCOM) == 0) {
                            const LogObjectList objlist(commandBuffer, rp_state.renderPass(), view_state.image_view());
                            skip |= LogError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-fragmentDensityMapAttachment-06504",
                                             objlist, error_obj.location,
                                             "Fragment density map attachment %" PRIu32
                                             " is not created with"
                                             " VK_IMAGE_CREATE_FRAGMENT_DENSITY_MAP_OFFSET_BIT_QCOM and fragmentDensityOffsetCount"
                                             " is %" PRIu32 " but must be 0 due to missing fragmentDensityOffset feature bit.",
                                             i, fdm_offset_info->fragmentDensityOffsetCount);
                        }

                        if ((subpass.viewMask != 0) &&
                            (view_state.create_info.subresourceRange.layerCount != fdm_offset_info->fragmentDensityOffsetCount)) {
                            const LogObjectList objlist(commandBuffer, rp_state.renderPass(), view_state.image_view());
                            skip |= LogError(
                                "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-fragmentDensityOffsetCount-06510", objlist,
                                error_obj.location,
                                "fragmentDensityOffsetCount %" PRIu32
                                " does not match the fragment density map attachment (%" PRIu32 ") view layer count (%" PRIu32 ").",
                                fdm_offset_info->fragmentDensityOffsetCount, i, view_state.create_info.subresourceRange.layerCount);
                        }

                        if ((subpass.viewMask == 0) && (fdm_offset_info->fragmentDensityOffsetCount != 1)) {
                            const LogObjectList objlist(commandBuffer, rp_state.renderPass(), view_state.image_view());
                            skip |= LogError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-fragmentDensityOffsetCount-06511",
                                             objlist, error_obj.location,
                                             "fragmentDensityOffsetCount %" PRIu32 " should be 1 when multiview is not enabled.",
                                             fdm_offset_info->fragmentDensityOffsetCount);
                        }
                    }
                }

                // depth stencil attachment
                if (subpass.pDepthStencilAttachment && (subpass.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) &&
                    (subpass.pDepthStencilAttachment->attachment == i) &&
                    ((ici.flags & VK_IMAGE_CREATE_FRAGMENT_DENSITY_MAP_OFFSET_BIT_QCOM) == 0)) {
                    const LogObjectList objlist(commandBuffer, rp_state.renderPass(), view_state.image_view());
                    skip |= LogError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pDepthStencilAttachment-06505", objlist,
                                     error_obj.location,
                                     "pDepthStencilAttachment[%" PRIu32
                                     "] is not created with"
                                     " VK_IMAGE_CREATE_FRAGMENT_DENSITY_MAP_OFFSET_BIT_QCOM and fragmentDensityOffsetCount"
                                     " is %" PRIu32 " but must be 0 due to missing fragmentDensityOffset feature bit.",
                                     i, fdm_offset_info->fragmentDensityOffsetCount);
                }

                // input attachments
                for (uint32_t k = 0; k < subpass.inputAttachmentCount; k++) {
                    const auto attachment = subpass.pInputAttachments[k].attachment;
                    if ((attachment != VK_ATTACHMENT_UNUSED) && (attachment == i) &&
                        ((ici.flags & VK_IMAGE_CREATE_FRAGMENT_DENSITY_MAP_OFFSET_BIT_QCOM) == 0)) {
                        const LogObjectList objlist(commandBuffer, rp_state.renderPass(), view_state.image_view());
                        skip |= LogError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pInputAttachments-06506", objlist,
                                         error_obj.location,
                                         "pInputAttachments[%" PRIu32
                                         "] is not created with"
                                         " VK_IMAGE_CREATE_FRAGMENT_DENSITY_MAP_OFFSET_BIT_QCOM and fragmentDensityOffsetCount"
                                         " is %" PRIu32 " but must be 0 due to missing fragmentDensityOffset feature bit.",
                                         i, fdm_offset_info->fragmentDensityOffsetCount);
                    }
                }

                // color attachments
                for (uint32_t k = 0; k < subpass.colorAttachmentCount; k++) {
                    const auto attachment = subpass.pColorAttachments[k].attachment;
                    if ((attachment != VK_ATTACHMENT_UNUSED) && (attachment == i) &&
                        ((ici.flags & VK_IMAGE_CREATE_FRAGMENT_DENSITY_MAP_OFFSET_BIT_QCOM) == 0)) {
                        const LogObjectList objlist(commandBuffer, rp_state.renderPass(), view_state.image_view());
                        skip |= LogError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pColorAttachments-06507", objlist,
                                         error_obj.location,
                                         "pColorAttachments[%" PRIu32
                                         "] is not created with"
                                         " VK_IMAGE_CREATE_FRAGMENT_DENSITY_MAP_OFFSET_BIT_QCOM and fragmentDensityOffsetCount"
                                         " is %" PRIu32 " but must be 0 due to missing fragmentDensityOffset feature bit.",
                                         i, fdm_offset_info->fragmentDensityOffsetCount);
                    }
                }

                // Resolve attachments
                if (subpass.pResolveAttachments != nullptr) {
                    for (uint32_t k = 0; k < subpass.colorAttachmentCount; k++) {
                        const auto attachment = subpass.pResolveAttachments[k].attachment;
                        if ((attachment != VK_ATTACHMENT_UNUSED) && (attachment == i) &&
                            ((ici.flags & VK_IMAGE_CREATE_FRAGMENT_DENSITY_MAP_OFFSET_BIT_QCOM) == 0)) {
                            const LogObjectList objlist(commandBuffer, rp_state.renderPass(), view_state.image_view());
                            skip |= LogError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pResolveAttachments-06508", objlist,
                                             error_obj.location,
                                             "pResolveAttachments[%" PRIu32
                                             "] is not created with"
                                             " VK_IMAGE_CREATE_FRAGMENT_DENSITY_MAP_OFFSET_BIT_QCOM and fragmentDensityOffsetCount"
                                             " is %" PRIu32 " but must be 0 due to missing fragmentDensityOffset feature bit.",
                                             i, fdm_offset_info->fragmentDensityOffsetCount);
                        }
                    }
                }

                // Preserve attachments
                for (uint32_t k = 0; k < subpass.preserveAttachmentCount; k++) {
                    const auto attachment = subpass.pPreserveAttachments[k];
                    if ((attachment != VK_ATTACHMENT_UNUSED) && (attachment == i) &&
                        ((ici.flags & VK_IMAGE_CREATE_FRAGMENT_DENSITY_MAP_OFFSET_BIT_QCOM) == 0)) {
                        const LogObjectList objlist(commandBuffer, rp_state.renderPass(), view_state.image_view());
                        skip |= LogError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pPreserveAttachments-06509", objlist,
                                         error_obj.location,
                                         "pPreserveAttachments[%" PRIu32
                                         "] is not created with"
                                         " VK_IMAGE_CREATE_FRAGMENT_DENSITY_MAP_OFFSET_BIT_QCOM and fragmentDensityOffsetCount"
                                         " is %" PRIu32 " but must be 0 due to missing fragmentDensityOffset feature bit.",
                                         i, fdm_offset_info->fragmentDensityOffsetCount);
                    }
                }
            }
        }
    }

    if (cb_state.transform_feedback_active) {
        vuid = use_rp2 ? "VUID-vkCmdEndRenderPass2-None-02352" : "VUID-vkCmdEndRenderPass-None-02351";
        const LogObjectList objlist(commandBuffer, rp_state.renderPass());
        skip |= LogError(vuid, objlist, error_obj.location, "transform feedback is active.");
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const {
    bool skip = ValidateCmdEndRenderPass(commandBuffer, VK_NULL_HANDLE, error_obj);
    return skip;
}

bool CoreChecks::PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                     const ErrorObject &error_obj) const {
    return PreCallValidateCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, error_obj);
}

bool CoreChecks::PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                  const ErrorObject &error_obj) const {
    bool skip = ValidateCmdEndRenderPass(commandBuffer, pSubpassEndInfo, error_obj);
    return skip;
}

void CoreChecks::RecordCmdEndRenderPassLayouts(VkCommandBuffer commandBuffer) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_state) {
        TransitionFinalSubpassLayouts(cb_state.get());
    }
}

void CoreChecks::PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    // Record the end at the CoreLevel to ensure StateTracker cleanup doesn't step on anything we need.
    RecordCmdEndRenderPassLayouts(commandBuffer);
    StateTracker::PostCallRecordCmdEndRenderPass(commandBuffer, record_obj);
}

void CoreChecks::PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                    const RecordObject &record_obj) {
    // Record the end at the CoreLevel to ensure StateTracker cleanup doesn't step on anything we need.
    RecordCmdEndRenderPassLayouts(commandBuffer);
    StateTracker::PostCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo, record_obj);
}

void CoreChecks::PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                 const RecordObject &record_obj) {
    RecordCmdEndRenderPassLayouts(commandBuffer);
    StateTracker::PostCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, record_obj);
}

bool CoreChecks::VerifyRenderAreaBounds(const VkRenderPassBeginInfo *pRenderPassBegin, const Location &loc) const {
    bool skip = false;

    const auto *device_group_render_pass_begin_info = vku::FindStructInPNextChain<VkDeviceGroupRenderPassBeginInfo>(pRenderPassBegin->pNext);
    const uint32_t device_group_area_count =
        device_group_render_pass_begin_info ? device_group_render_pass_begin_info->deviceRenderAreaCount : 0;

    auto framebuffer_state = Get<FRAMEBUFFER_STATE>(pRenderPassBegin->framebuffer);
    const auto *framebuffer_info = &framebuffer_state->createInfo;
    // These VUs depend on count being non-zero, or else acts like struct is not there
    if (device_group_area_count > 0) {
        for (uint32_t i = 0; i < device_group_area_count; ++i) {
            const Location render_area_loc = loc.pNext(Struct::VkDeviceGroupRenderPassBeginInfo, Field::pDeviceRenderAreas, i);
            const auto &deviceRenderArea = device_group_render_pass_begin_info->pDeviceRenderAreas[i];
            if (deviceRenderArea.offset.x < 0) {
                skip |= LogError("VUID-VkDeviceGroupRenderPassBeginInfo-offset-06166", pRenderPassBegin->renderPass,
                                 render_area_loc.dot(Field::offset).dot(Field::x), "is negative (%" PRId32 ").",
                                 deviceRenderArea.offset.x);
            }
            if (deviceRenderArea.offset.y < 0) {
                skip |= LogError("VUID-VkDeviceGroupRenderPassBeginInfo-offset-06167", pRenderPassBegin->renderPass,
                                 render_area_loc.dot(Field::offset).dot(Field::y), "is negative (%" PRId32 ").",
                                 deviceRenderArea.offset.y);
            }
            if ((deviceRenderArea.offset.x + deviceRenderArea.extent.width) > framebuffer_info->width) {
                const LogObjectList objlist(pRenderPassBegin->renderPass, framebuffer_state->framebuffer());
                skip |=
                    LogError("VUID-VkRenderPassBeginInfo-pNext-02856", objlist, render_area_loc,
                             "offset.x (%" PRId32 ") + extent.width (%" PRId32 ") is greater than framebuffer width (%" PRId32 ").",
                             deviceRenderArea.offset.x, deviceRenderArea.extent.width, framebuffer_info->width);
            }
            if ((deviceRenderArea.offset.y + deviceRenderArea.extent.height) > framebuffer_info->height) {
                const LogObjectList objlist(pRenderPassBegin->renderPass, framebuffer_state->framebuffer());
                skip |= LogError("VUID-VkRenderPassBeginInfo-pNext-02857", objlist, render_area_loc,
                                 "offset.y (%" PRId32 ") + extent.height (%" PRId32 ") is greater than framebuffer height (%" PRId32
                                 ").",
                                 deviceRenderArea.offset.y, deviceRenderArea.extent.height, framebuffer_info->height);
            }
        }
    } else {
        const Location render_area_loc = loc.dot(Field::renderArea);
        if (pRenderPassBegin->renderArea.offset.x < 0) {
            skip |= LogError("VUID-VkRenderPassBeginInfo-pNext-02850", pRenderPassBegin->renderPass,
                             render_area_loc.dot(Field::offset).dot(Field::x), "is %" PRId32 " (offset can't be negative).",
                             pRenderPassBegin->renderArea.offset.x);
        }
        if (pRenderPassBegin->renderArea.offset.y < 0) {
            skip |= LogError("VUID-VkRenderPassBeginInfo-pNext-02851", pRenderPassBegin->renderPass,
                             render_area_loc.dot(Field::offset).dot(Field::y), "is %" PRId32 " (offset can't be negative).",
                             pRenderPassBegin->renderArea.offset.y);
        }

        const auto x_adjusted_extent = static_cast<int64_t>(pRenderPassBegin->renderArea.offset.x) +
                                       static_cast<int64_t>(pRenderPassBegin->renderArea.extent.width);
        if (x_adjusted_extent > static_cast<int64_t>(framebuffer_info->width)) {
            const LogObjectList objlist(pRenderPassBegin->renderPass, framebuffer_state->framebuffer());
            skip |=
                LogError("VUID-VkRenderPassBeginInfo-pNext-02852", objlist, render_area_loc,
                         "offset.x (%" PRId32 ") + extent.width (%" PRId32 ") is greater than framebuffer width (%" PRId32 ").",
                         pRenderPassBegin->renderArea.offset.x, pRenderPassBegin->renderArea.extent.width, framebuffer_info->width);
        }

        const auto y_adjusted_extent = static_cast<int64_t>(pRenderPassBegin->renderArea.offset.y) +
                                       static_cast<int64_t>(pRenderPassBegin->renderArea.extent.height);
        if (y_adjusted_extent > static_cast<int64_t>(framebuffer_info->height)) {
            const LogObjectList objlist(pRenderPassBegin->renderPass, framebuffer_state->framebuffer());
            skip |= LogError(
                "VUID-VkRenderPassBeginInfo-pNext-02853", objlist, render_area_loc,
                "offset.y (%" PRId32 ") + extent.height (%" PRId32 ") is greater than framebuffer height (%" PRId32 ").",
                pRenderPassBegin->renderArea.offset.y, pRenderPassBegin->renderArea.extent.height, framebuffer_info->height);
        }
    }
    return skip;
}

bool CoreChecks::VerifyFramebufferAndRenderPassImageViews(const VkRenderPassBeginInfo *pRenderPassBeginInfo,
                                                          const Location &loc) const {
    bool skip = false;
    const auto *render_pass_attachment_begin_info = vku::FindStructInPNextChain<VkRenderPassAttachmentBeginInfo>(pRenderPassBeginInfo->pNext);
    if (!render_pass_attachment_begin_info || render_pass_attachment_begin_info->attachmentCount == 0) {
        return false;
    }

    const auto &framebuffer_state = *Get<FRAMEBUFFER_STATE>(pRenderPassBeginInfo->framebuffer);
    const auto &framebuffer_create_info = framebuffer_state.createInfo;
    const auto *framebuffer_attachments_create_info =
        vku::FindStructInPNextChain<VkFramebufferAttachmentsCreateInfo>(framebuffer_create_info.pNext);
    if ((framebuffer_create_info.flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) == 0) {
        const LogObjectList objlist(pRenderPassBeginInfo->renderPass, pRenderPassBeginInfo->framebuffer);
        skip |= LogError("VUID-VkRenderPassBeginInfo-framebuffer-03207", objlist,
                         loc.pNext(Struct::VkRenderPassAttachmentBeginInfo, Field::attachmentCount),
                         "is %" PRIu32 ", but the VkFramebuffer create flags (%s) has no VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT.",
                         render_pass_attachment_begin_info->attachmentCount,
                         string_VkFramebufferCreateFlags(framebuffer_create_info.flags).c_str());
    } else if (framebuffer_attachments_create_info) {
        if (framebuffer_attachments_create_info->attachmentImageInfoCount != render_pass_attachment_begin_info->attachmentCount) {
            const LogObjectList objlist(pRenderPassBeginInfo->renderPass, pRenderPassBeginInfo->framebuffer);
            skip |= LogError(
                "VUID-VkRenderPassBeginInfo-framebuffer-03208", objlist,
                loc.pNext(Struct::VkRenderPassAttachmentBeginInfo, Field::attachmentCount),
                "is %" PRIu32
                ", but VkFramebuffer was created with VkFramebufferAttachmentsCreateInfo::attachmentImageInfoCount = %" PRIu32 ".",
                render_pass_attachment_begin_info->attachmentCount, framebuffer_attachments_create_info->attachmentImageInfoCount);
        } else {
            auto render_pass_state = Get<RENDER_PASS_STATE>(pRenderPassBeginInfo->renderPass);
            const auto *render_pass_create_info = &render_pass_state->createInfo;
            for (uint32_t i = 0; i < render_pass_attachment_begin_info->attachmentCount; ++i) {
                const Location attachment_loc = loc.pNext(Struct::VkRenderPassAttachmentBeginInfo, Field::pAttachments, i);
                auto image_view_state = Get<IMAGE_VIEW_STATE>(render_pass_attachment_begin_info->pAttachments[i]);
                const VkImageViewCreateInfo *image_view_create_info = &image_view_state->create_info;
                const auto &subresource_range = image_view_state->normalized_subresource_range;
                const VkFramebufferAttachmentImageInfo *framebuffer_attachment_image_info =
                    &framebuffer_attachments_create_info->pAttachmentImageInfos[i];
                const auto *image_create_info = &image_view_state->image_state->createInfo;
                const LogObjectList objlist(pRenderPassBeginInfo->renderPass, pRenderPassBeginInfo->framebuffer,
                                            image_view_state->image_view(), image_view_state->image_state->image());

                if (framebuffer_attachment_image_info->flags != image_create_info->flags) {
                    skip |= LogError("VUID-VkRenderPassBeginInfo-framebuffer-03209", objlist, attachment_loc.dot(Field::flags),
                                     "is %s, but the VkFramebuffer was created with "
                                     "VkFramebufferAttachmentsCreateInfo::pAttachmentImageInfos[%" PRIu32 "].flags = %s",
                                     string_VkImageCreateFlags(image_create_info->flags).c_str(), i,
                                     string_VkImageCreateFlags(framebuffer_attachment_image_info->flags).c_str());
                }

                if (framebuffer_attachment_image_info->usage != image_view_state->inherited_usage) {
                    // Give clearer message if this error is due to the "inherited" part or not
                    if (image_create_info->usage == image_view_state->inherited_usage) {
                        skip |= LogError("VUID-VkRenderPassBeginInfo-framebuffer-04627", objlist, attachment_loc.dot(Field::usage),
                                         "is (%s), but the VkFramebuffer was created with "
                                         "vkFramebufferAttachmentsCreateInfo::pAttachmentImageInfos[%" PRIu32 "].usage = %s.",
                                         string_VkImageUsageFlags(image_create_info->usage).c_str(), i,
                                         string_VkImageUsageFlags(framebuffer_attachment_image_info->usage).c_str());
                    } else {
                        skip |= LogError(
                            "VUID-VkRenderPassBeginInfo-framebuffer-04627", objlist, attachment_loc.dot(Field::usage),
                            "is (%s), which has an inherited usage subset from VkImageViewUsageCreateInfo of (%s), but the "
                            "VkFramebuffer was created with vkFramebufferAttachmentsCreateInfo::pAttachmentImageInfos[%" PRIu32
                            "].usage = %s.",
                            string_VkImageUsageFlags(image_create_info->usage).c_str(),
                            string_VkImageUsageFlags(image_view_state->inherited_usage).c_str(), i,
                            string_VkImageUsageFlags(framebuffer_attachment_image_info->usage).c_str());
                    }
                }

                const auto view_width = std::max(1u, image_create_info->extent.width >> subresource_range.baseMipLevel);
                if (framebuffer_attachment_image_info->width != view_width) {
                    skip |= LogError("VUID-VkRenderPassBeginInfo-framebuffer-03211", objlist, attachment_loc,
                                     "has VkImageView width (%" PRIu32 ") at mip level %" PRIu32 " (%" PRIu32
                                     ") != VkFramebufferAttachmentsCreateInfo::pAttachments[%" PRIu32 "].width (%" PRIu32 ").",
                                     image_create_info->extent.width, subresource_range.baseMipLevel, view_width, i,
                                     framebuffer_attachment_image_info->width);
                }

                const bool is_1d = (image_view_create_info->viewType == VK_IMAGE_VIEW_TYPE_1D) ||
                                   (image_view_create_info->viewType == VK_IMAGE_VIEW_TYPE_1D_ARRAY);
                const auto view_height = (!is_1d) ? std::max(1u, image_create_info->extent.height >> subresource_range.baseMipLevel)
                                                  : image_create_info->extent.height;
                if (framebuffer_attachment_image_info->height != view_height) {
                    skip |= LogError("VUID-VkRenderPassBeginInfo-framebuffer-03212", objlist, attachment_loc,
                                     "has VkImageView height (%" PRIu32 ") at mip level %" PRIu32 " (%" PRIu32
                                     ") != VkFramebufferAttachmentsCreateInfo::pAttachments[%" PRIu32 "].height (%" PRIu32 ").",
                                     image_create_info->extent.height, subresource_range.baseMipLevel, view_height, i,
                                     framebuffer_attachment_image_info->height);
                }

                const uint32_t layerCount = image_view_state->create_info.subresourceRange.layerCount != VK_REMAINING_ARRAY_LAYERS
                                                ? image_view_state->create_info.subresourceRange.layerCount
                                                : image_create_info->extent.depth;
                if (framebuffer_attachment_image_info->layerCount != layerCount) {
                    skip |=
                        LogError("VUID-VkRenderPassBeginInfo-framebuffer-03213", objlist, attachment_loc,
                                 "has a subresource range with a layerCount of %" PRIu32
                                 ", but VkFramebufferAttachmentsCreateInfo::pAttachments[%" PRIu32 "].layerCount is %" PRIu32 ".",
                                 layerCount, i, framebuffer_attachment_image_info->layerCount);
                }

                const auto *image_format_list_create_info = vku::FindStructInPNextChain<VkImageFormatListCreateInfo>(image_create_info->pNext);
                if (image_format_list_create_info) {
                    if (image_format_list_create_info->viewFormatCount != framebuffer_attachment_image_info->viewFormatCount) {
                        skip |= LogError(
                            "VUID-VkRenderPassBeginInfo-framebuffer-03214", objlist, attachment_loc,
                            "internal VkImage was created with a VkImageFormatListCreateInfo::viewFormatCount of %" PRIu32
                            " but VkFramebufferAttachmentsCreateInfo::pAttachments[%" PRIu32 "].viewFormatCount is %" PRIu32 ".",
                            image_format_list_create_info->viewFormatCount, i, framebuffer_attachment_image_info->viewFormatCount);
                    }

                    for (uint32_t j = 0; j < image_format_list_create_info->viewFormatCount; ++j) {
                        bool format_found = false;
                        for (uint32_t k = 0; k < framebuffer_attachment_image_info->viewFormatCount; ++k) {
                            if (image_format_list_create_info->pViewFormats[j] ==
                                framebuffer_attachment_image_info->pViewFormats[k]) {
                                format_found = true;
                            }
                        }
                        if (!format_found) {
                            skip |= LogError("VUID-VkRenderPassBeginInfo-framebuffer-03215", objlist, attachment_loc,
                                             "internal VkImage was created with VkImageFormatListCreateInfo::pViewFormats[%" PRIu32
                                             "] = %s,"
                                             "but now found in vkFramebufferAttachmentsCreateInfo::pAttachmentImageInfos[%" PRIu32
                                             "].pViewFormats.",
                                             j, string_VkFormat(image_format_list_create_info->pViewFormats[j]), i);
                        }
                    }
                }

                if (render_pass_create_info->pAttachments[i].format != image_view_create_info->format) {
                    skip |= LogError("VUID-VkRenderPassBeginInfo-framebuffer-03216", objlist, attachment_loc.dot(Field::format),
                                     "is %s, but the VkRenderPass was created with a pAttachments[%" PRIu32 "].format of %s.",
                                     string_VkFormat(image_view_create_info->format), i,
                                     string_VkFormat(render_pass_create_info->pAttachments[i].format));
                } else if (image_view_create_info->format == VK_FORMAT_UNDEFINED) {
                    // both have external foramts
                    const uint64_t attachment_external_format = GetExternalFormat(render_pass_create_info->pAttachments[i].pNext);
                    if (image_view_state->image_state->ahb_format != attachment_external_format) {
                        skip |=
                            LogError("VUID-VkRenderPassBeginInfo-framebuffer-09354", objlist, attachment_loc,
                                     "externalFormat is %" PRIu64 ", but the VkRenderPass was created with a pAttachments[%" PRIu32
                                     "] with externalFormat of %" PRIu64 ".",
                                     image_view_state->image_state->ahb_format, i, attachment_external_format);
                    }
                }

                const VkSampleCountFlagBits attachment_samples = render_pass_create_info->pAttachments[i].samples;
                const auto *ms_render_to_single_sample =
                    vku::FindStructInPNextChain<VkMultisampledRenderToSingleSampledInfoEXT>(pRenderPassBeginInfo->pNext);
                const bool single_sample_enabled = ms_render_to_single_sample &&
                                                   ms_render_to_single_sample->multisampledRenderToSingleSampledEnable &&
                                                   (attachment_samples == VK_SAMPLE_COUNT_1_BIT);
                if (attachment_samples != image_create_info->samples && !single_sample_enabled) {
                    skip |= LogError("VUID-VkRenderPassBeginInfo-framebuffer-09047", objlist, attachment_loc,
                                     "internal VkImage was created with %s samples, "
                                     "but the VkRenderPass was created with a pAttachments[%" PRIu32 "].samples of %s.",
                                     string_VkSampleCountFlagBits(image_create_info->samples), i,
                                     string_VkSampleCountFlagBits(render_pass_create_info->pAttachments[i].samples));
                }

                if (subresource_range.levelCount != 1) {
                    skip |= LogError("VUID-VkRenderPassAttachmentBeginInfo-pAttachments-03218", objlist, attachment_loc,
                                     "was created with multiple mip levels (%" PRIu32 ").", subresource_range.levelCount);
                }

                if (IsIdentitySwizzle(image_view_create_info->components) == false) {
                    skip |=
                        LogError("VUID-VkRenderPassAttachmentBeginInfo-pAttachments-03219", objlist, attachment_loc,
                                 "was created with non-identity swizzle. All "
                                 "framebuffer attachments must have been created with the identity swizzle. Here are the actual "
                                 "swizzle values:\n"
                                 "r swizzle = %s\n"
                                 "g swizzle = %s\n"
                                 "b swizzle = %s\n"
                                 "a swizzle = %s\n",
                                 string_VkComponentSwizzle(image_view_create_info->components.r),
                                 string_VkComponentSwizzle(image_view_create_info->components.g),
                                 string_VkComponentSwizzle(image_view_create_info->components.b),
                                 string_VkComponentSwizzle(image_view_create_info->components.a));
                }

                if (image_view_create_info->viewType == VK_IMAGE_VIEW_TYPE_3D) {
                    skip |= LogError("VUID-VkRenderPassAttachmentBeginInfo-pAttachments-04114", objlist, attachment_loc,
                                     "was created with viewType of VK_IMAGE_VIEW_TYPE_3D.");
                }
            }
        }
    }

    return skip;
}

static bool FindDependency(const uint32_t index, const uint32_t dependent, const std::vector<DAGNode> &subpass_to_node,
                           vvl::unordered_set<uint32_t> &processed_nodes) {
    // If we have already checked this node we have not found a dependency path so return false.
    if (processed_nodes.count(index)) return false;
    processed_nodes.insert(index);
    const DAGNode &node = subpass_to_node[index];
    // Look for a dependency path. If one exists return true else recurse on the previous nodes.
    if (std::find(node.prev.begin(), node.prev.end(), dependent) == node.prev.end()) {
        for (auto elem : node.prev) {
            if (FindDependency(elem, dependent, subpass_to_node, processed_nodes)) return true;
        }
    } else {
        return true;
    }
    return false;
}

bool CoreChecks::CheckDependencyExists(const VkRenderPass renderpass, const uint32_t subpass, const VkImageLayout layout,
                                       const std::vector<SubpassLayout> &dependent_subpasses,
                                       const std::vector<DAGNode> &subpass_to_node, const Location &attachment_loc,
                                       bool &skip) const {
    bool result = true;
    const bool b_image_layout_read_only = IsImageLayoutReadOnly(layout);
    // Loop through all subpasses that share the same attachment and make sure a dependency exists
    for (uint32_t k = 0; k < dependent_subpasses.size(); ++k) {
        const SubpassLayout &sp = dependent_subpasses[k];
        if (subpass == sp.index) continue;
        if (b_image_layout_read_only && IsImageLayoutReadOnly(sp.layout)) continue;

        const DAGNode &node = subpass_to_node[subpass];
        // Check for a specified dependency between the two nodes. If one exists we are done.
        auto prev_elem = std::find(node.prev.begin(), node.prev.end(), sp.index);
        auto next_elem = std::find(node.next.begin(), node.next.end(), sp.index);
        if (prev_elem == node.prev.end() && next_elem == node.next.end()) {
            // If no dependency exits an implicit dependency still might. If not, throw an error.
            vvl::unordered_set<uint32_t> processed_nodes;
            if (!(FindDependency(subpass, sp.index, subpass_to_node, processed_nodes) ||
                  FindDependency(sp.index, subpass, subpass_to_node, processed_nodes))) {
                skip |=
                    LogError(kVUID_Core_DrawState_InvalidRenderpass, renderpass, attachment_loc,
                             "A dependency between subpasses %d and %d must exist but one is not specified.", subpass, sp.index);
                result = false;
            }
        }
    }
    return result;
}

bool CoreChecks::CheckPreserved(const VkRenderPass renderpass, const VkRenderPassCreateInfo2 *pCreateInfo, const int index,
                                const uint32_t attachment, const std::vector<DAGNode> &subpass_to_node, int depth,
                                const Location &attachment_loc, bool &skip) const {
    const DAGNode &node = subpass_to_node[index];
    // If this node writes to the attachment return true as next nodes need to preserve the attachment.
    const VkSubpassDescription2 &subpass = pCreateInfo->pSubpasses[index];
    for (uint32_t j = 0; j < subpass.colorAttachmentCount; ++j) {
        if (attachment == subpass.pColorAttachments[j].attachment) return true;
    }
    for (uint32_t j = 0; j < subpass.inputAttachmentCount; ++j) {
        if (attachment == subpass.pInputAttachments[j].attachment) return true;
    }
    if (subpass.pDepthStencilAttachment && subpass.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
        if (attachment == subpass.pDepthStencilAttachment->attachment) return true;
    }
    bool result = false;
    // Loop through previous nodes and see if any of them write to the attachment.
    for (auto elem : node.prev) {
        result |= CheckPreserved(renderpass, pCreateInfo, elem, attachment, subpass_to_node, depth + 1, attachment_loc, skip);
    }
    // If the attachment was written to by a previous node than this node needs to preserve it.
    if (result && depth > 0) {
        bool has_preserved = false;
        for (uint32_t j = 0; j < subpass.preserveAttachmentCount; ++j) {
            if (subpass.pPreserveAttachments[j] == attachment) {
                has_preserved = true;
                break;
            }
        }
        if (!has_preserved) {
            skip |= LogError(kVUID_Core_DrawState_InvalidRenderpass, renderpass, attachment_loc,
                             "Attachment %d is used by a later subpass and must be preserved in subpass %d.", attachment, index);
        }
    }
    return result;
}

template <class T>
bool IsRangeOverlapping(T offset1, T size1, T offset2, T size2) {
    return (((offset1 + size1) > offset2) && ((offset1 + size1) < (offset2 + size2))) ||
           ((offset1 > offset2) && (offset1 < (offset2 + size2)));
}

bool IsRegionOverlapping(VkImageSubresourceRange range1, VkImageSubresourceRange range2) {
    return (IsRangeOverlapping(range1.baseMipLevel, range1.levelCount, range2.baseMipLevel, range2.levelCount) &&
            IsRangeOverlapping(range1.baseArrayLayer, range1.layerCount, range2.baseArrayLayer, range2.layerCount));
}

bool CoreChecks::ValidateAttachmentIndex(uint32_t attachment, uint32_t attachment_count, const Location &loc) const {
    bool skip = false;
    const bool use_rp2 = loc.function != Func::vkCreateRenderPass;
    assert(attachment != VK_ATTACHMENT_UNUSED);
    if (attachment >= attachment_count) {
        const char *vuid =
            use_rp2 ? "VUID-VkRenderPassCreateInfo2-attachment-03051" : "VUID-VkRenderPassCreateInfo-attachment-00834";
        skip |= LogError(vuid, device, loc.dot(Field::attachment),
                         "is %" PRIu32 ", but must be less than the total number of attachments (%" PRIu32 ").", attachment,
                         attachment_count);
    }
    return skip;
}

enum AttachmentType {
    ATTACHMENT_COLOR = 1,
    ATTACHMENT_DEPTH = 2,
    ATTACHMENT_INPUT = 4,
    ATTACHMENT_PRESERVE = 8,
    ATTACHMENT_RESOLVE = 16,
};

char const *StringAttachmentType(uint8_t type) {
    switch (type) {
        case ATTACHMENT_COLOR:
            return "color";
        case ATTACHMENT_DEPTH:
            return "depth";
        case ATTACHMENT_INPUT:
            return "input";
        case ATTACHMENT_PRESERVE:
            return "preserve";
        case ATTACHMENT_RESOLVE:
            return "resolve";
        default:
            return "(multiple)";
    }
}

bool CoreChecks::AddAttachmentUse(std::vector<uint8_t> &attachment_uses, std::vector<VkImageLayout> &attachment_layouts,
                                  uint32_t attachment, uint8_t new_use, VkImageLayout new_layout, const Location loc) const {
    if (attachment >= attachment_uses.size()) return false; /* out of range, but already reported */

    bool skip = false;
    auto &uses = attachment_uses[attachment];
    const bool use_rp2 = loc.function != Func::vkCreateRenderPass;
    const char *vuid;

    if (uses & new_use) {
        if (attachment_layouts[attachment] != new_layout) {
            vuid = use_rp2 ? "VUID-VkSubpassDescription2-layout-02528" : "VUID-VkSubpassDescription-layout-02519";
            skip |= LogError(vuid, device, loc, "already uses attachment %" PRIu32 " with a different image layout (%s vs %s).",
                             attachment, string_VkImageLayout(attachment_layouts[attachment]), string_VkImageLayout(new_layout));
        }
    } else if (((new_use & ATTACHMENT_COLOR) && (uses & ATTACHMENT_DEPTH)) ||
               ((uses & ATTACHMENT_COLOR) && (new_use & ATTACHMENT_DEPTH))) {
        vuid = use_rp2 ? "VUID-VkSubpassDescription2-pDepthStencilAttachment-04440"
                       : "VUID-VkSubpassDescription-pDepthStencilAttachment-04438";
        skip |= LogError(vuid, device, loc, "uses attachment %" PRIu32 " as both %s and %s attachment.", attachment,
                         StringAttachmentType(uses), StringAttachmentType(new_use));
    } else if ((uses && (new_use & ATTACHMENT_PRESERVE)) || (new_use && (uses & ATTACHMENT_PRESERVE))) {
        vuid = use_rp2 ? "VUID-VkSubpassDescription2-pPreserveAttachments-03074"
                       : "VUID-VkSubpassDescription-pPreserveAttachments-00854";
        skip |= LogError(vuid, device, loc, "uses attachment %" PRIu32 " as both %s and %s attachment.", attachment,
                         StringAttachmentType(uses), StringAttachmentType(new_use));
    } else {
        attachment_layouts[attachment] = new_layout;
        uses |= new_use;
    }

    return skip;
}

// Handles attachment references regardless of type (input, color, depth, etc)
// Input attachments have extra VUs associated with them
bool CoreChecks::ValidateAttachmentReference(VkAttachmentReference2 reference, const VkFormat attachment_format, bool input,
                                             const Location &loc) const {
    bool skip = false;
    const bool use_rp2 = loc.function != Func::vkCreateRenderPass;
    const char *vuid;

    // Currently all VUs require attachment to not be UNUSED
    assert(reference.attachment != VK_ATTACHMENT_UNUSED);

    // currently VkAttachmentReference and VkAttachmentReference2 have no overlapping VUs
    const auto *attachment_reference_stencil_layout = vku::FindStructInPNextChain<VkAttachmentReferenceStencilLayout>(reference.pNext);
    switch (reference.layout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            vuid = (use_rp2) ? "VUID-VkAttachmentReference2-layout-03077" : "VUID-VkAttachmentReference-layout-03077";
            skip |= LogError(vuid, device, loc, "is %s.", string_VkImageLayout(reference.layout));
            break;

        // Only other layouts in VUs to be checked
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
            // First need to make sure feature bit is enabled and the format is actually a depth and/or stencil
            if (!enabled_features.separateDepthStencilLayouts) {
                vuid = (use_rp2) ? "VUID-VkAttachmentReference2-separateDepthStencilLayouts-03313"
                                 : "VUID-VkAttachmentReference-separateDepthStencilLayouts-03313";
                skip |= LogError(vuid, device, loc, "is %s (and separateDepthStencilLayouts was not enabled).",
                                 string_VkImageLayout(reference.layout));
            } else if (IsImageLayoutDepthOnly(reference.layout)) {
                if (attachment_reference_stencil_layout) {
                    // This check doesn't rely on the aspect mask value
                    const VkImageLayout stencil_layout = attachment_reference_stencil_layout->stencilLayout;
                    if (stencil_layout == VK_IMAGE_LAYOUT_UNDEFINED || stencil_layout == VK_IMAGE_LAYOUT_PREINITIALIZED ||
                        stencil_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
                        stencil_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                        stencil_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL ||
                        stencil_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                        stencil_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL ||
                        stencil_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
                        stencil_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL ||
                        stencil_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
                        skip |= LogError("VUID-VkAttachmentReferenceStencilLayout-stencilLayout-03318", device,
                                         loc.pNext(Struct::VkAttachmentReferenceStencilLayout, Field::stencilLayout),
                                         "(%s) is not a valid VkImageLayout.", string_VkImageLayout(stencil_layout));
                    }
                }
            }
            break;
        case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR:
        case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR:
            if (!enabled_features.synchronization2) {
                vuid = (use_rp2) ? "VUID-VkAttachmentReference2-synchronization2-06910"
                                 : "VUID-VkAttachmentReference-synchronization2-06910";
                skip |= LogError(vuid, device, loc, "is %s (and synchronization2 was not enabled).",
                                 string_VkImageLayout(reference.layout));
            }
            break;
        case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
            if (!enabled_features.attachmentFeedbackLoopLayout) {
                vuid = (use_rp2) ? "VUID-VkAttachmentReference2-attachmentFeedbackLoopLayout-07311"
                                 : "VUID-VkAttachmentReference-attachmentFeedbackLoopLayout-07311";
                skip |= LogError(vuid, device, loc,
                                 "is VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT, but the "
                                 "attachmentFeedbackLoopLayout feature was not enabled.");
            }
            break;

        default:
            break;
    }

    return skip;
}

bool CoreChecks::ValidateRenderpassAttachmentUsage(const VkRenderPassCreateInfo2 *pCreateInfo, const ErrorObject &error_obj) const {
    bool skip = false;
    const bool use_rp2 = error_obj.location.function != Func::vkCreateRenderPass;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);

    // Track when we're observing the first use of an attachment
    std::vector<bool> attach_first_use(pCreateInfo->attachmentCount, true);

    for (uint32_t i = 0; i < pCreateInfo->subpassCount; ++i) {
        const Location subpass_loc = create_info_loc.dot(Field::pSubpasses, i);
        const VkSubpassDescription2 &subpass = pCreateInfo->pSubpasses[i];
        const auto ms_render_to_single_sample = vku::FindStructInPNextChain<VkMultisampledRenderToSingleSampledInfoEXT>(subpass.pNext);
        const auto subpass_depth_stencil_resolve = vku::FindStructInPNextChain<VkSubpassDescriptionDepthStencilResolve>(subpass.pNext);
        std::vector<uint8_t> attachment_uses(pCreateInfo->attachmentCount);
        std::vector<VkImageLayout> attachment_layouts(pCreateInfo->attachmentCount);

        // Track if attachments are used as input as well as another type
        vvl::unordered_set<uint32_t> input_attachments;

        if (subpass.pipelineBindPoint != VK_PIPELINE_BIND_POINT_GRAPHICS &&
            subpass.pipelineBindPoint != VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI) {
            const char *vuid = use_rp2 ? "VUID-VkSubpassDescription2-pipelineBindPoint-04953"
                                       : "VUID-VkSubpassDescription-pipelineBindPoint-04952";
            skip |= LogError(vuid, device, subpass_loc.dot(Field::pipelineBindPoint), "is %s.",
                             string_VkPipelineBindPoint(subpass.pipelineBindPoint));
        }

        // Check input attachments first
        // - so we can detect first-use-as-input for VU #00349
        // - if other color or depth/stencil is also input, it limits valid layouts
        for (uint32_t j = 0; j < subpass.inputAttachmentCount; ++j) {
            auto const &attachment_ref = subpass.pInputAttachments[j];
            const uint32_t attachment_index = attachment_ref.attachment;
            if (attachment_index == VK_ATTACHMENT_UNUSED) {
                continue;
            }
            const VkImageAspectFlags aspect_mask = attachment_ref.aspectMask;
            const Location input_loc = subpass_loc.dot(Field::pInputAttachments, j);
            const Location attachment_loc = create_info_loc.dot(Field::pAttachments, attachment_index);

            input_attachments.insert(attachment_index);
            skip |= ValidateAttachmentIndex(attachment_index, pCreateInfo->attachmentCount, input_loc);

            if (aspect_mask & VK_IMAGE_ASPECT_METADATA_BIT) {
                const char *vuid = use_rp2 ? "VUID-VkSubpassDescription2-attachment-02801"
                                           : "VUID-VkInputAttachmentAspectReference-aspectMask-01964";
                skip |= LogError(vuid, device, input_loc.dot(Field::aspectMask), "is %s.",
                                 string_VkImageAspectFlags(aspect_mask).c_str());
            } else if (aspect_mask & (VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT |
                                      VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT)) {
                const char *vuid = use_rp2 ? "VUID-VkSubpassDescription2-attachment-04563"
                                           : "VUID-VkInputAttachmentAspectReference-aspectMask-02250";
                skip |= LogError(vuid, device, input_loc.dot(Field::aspectMask), "is %s.",
                                 string_VkImageAspectFlags(aspect_mask).c_str());
            }

            const VkImageLayout attachment_layout = attachment_ref.layout;
            if (IsValueIn(attachment_layout,
                          {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL})) {
                const char *vuid =
                    use_rp2 ? "VUID-VkSubpassDescription2-attachment-06912" : "VUID-VkSubpassDescription-attachment-06912";
                skip |= LogError(vuid, device, input_loc.dot(Field::layout), "(%s) is invalid.",
                                 string_VkImageLayout(attachment_layout));
            }
            if (IsValueIn(attachment_layout,
                          {VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL})) {
                const char *vuid =
                    use_rp2 ? "VUID-VkSubpassDescription2-attachment-06918" : "VUID-VkSubpassDescription-attachment-06918";
                skip |= LogError(vuid, device, input_loc.dot(Field::layout), "(%s) is invalid.",
                                 string_VkImageLayout(attachment_layout));
            }
            if (attachment_layout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR) {
                const char *vuid =
                    use_rp2 ? "VUID-VkSubpassDescription2-attachment-06921" : "VUID-VkSubpassDescription-attachment-06921";
                skip |= LogError(vuid, device, input_loc.dot(Field::layout), "(%s) is invalid.",
                                 string_VkImageLayout(attachment_layout));
            }

            // safe to dereference pCreateInfo->pAttachments[]
            if (attachment_index < pCreateInfo->attachmentCount) {
                const VkAttachmentDescription2 &attachment_description = pCreateInfo->pAttachments[attachment_index];
                const VkFormat attachment_format = attachment_description.format;
                skip |= ValidateAttachmentReference(attachment_ref, attachment_format, true, input_loc);

                skip |= AddAttachmentUse(attachment_uses, attachment_layouts, attachment_index, ATTACHMENT_INPUT,
                                         attachment_ref.layout, input_loc);

                {
                    const char *vuid =
                        use_rp2 ? "VUID-VkRenderPassCreateInfo2-attachment-02525" : "VUID-VkRenderPassCreateInfo-pNext-01963";
                    // Assuming no disjoint image since there's no handle
                    skip |=
                        ValidateImageAspectMask(VK_NULL_HANDLE, attachment_format, aspect_mask, false, error_obj.location, vuid);
                }

                if (attach_first_use[attachment_index]) {
                    skip |= ValidateLayoutVsAttachmentDescription(subpass.pInputAttachments[j].layout, attachment_index,
                                                                  attachment_description, input_loc.dot(Field::layout));

                    const bool used_as_depth = (subpass.pDepthStencilAttachment != NULL &&
                                                subpass.pDepthStencilAttachment->attachment == attachment_index);
                    bool used_as_color = false;
                    for (uint32_t k = 0; !used_as_depth && !used_as_color && k < subpass.colorAttachmentCount; ++k) {
                        used_as_color = (subpass.pColorAttachments[k].attachment == attachment_index);
                    }
                    if (!used_as_depth && !used_as_color && attachment_description.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
                        const char *vuid =
                            use_rp2 ? "VUID-VkSubpassDescription2-loadOp-03064" : "VUID-VkSubpassDescription-loadOp-00846";
                        skip |= LogError(vuid, device, attachment_loc.dot(Field::loadOp), "is VK_ATTACHMENT_LOAD_OP_CLEAR.");
                    }
                }
                attach_first_use[attachment_index] = false;

                const VkFormatFeatureFlags2 valid_flags =
                    VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT_KHR | VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT_KHR;
                const VkFormatFeatureFlags2 format_features = GetPotentialFormatFeatures(attachment_format);
                const void *pNext = (use_rp2) ? attachment_description.pNext : nullptr;
                if ((format_features & valid_flags) == 0 && GetExternalFormat(pNext) == 0) {
                    if (!enabled_features.linearColorAttachment) {
                        const char *vuid = use_rp2 ? "VUID-VkSubpassDescription2-pInputAttachments-02897"
                                                   : "VUID-VkSubpassDescription-pInputAttachments-02647";
                        skip |= LogError(vuid, device, attachment_loc.dot(Field::format),
                                         "(%s) format features are %s (referenced by %s).", string_VkFormat(attachment_format),
                                         string_VkFormatFeatureFlags2(format_features).c_str(), input_loc.Fields().c_str());
                    } else if ((format_features & VK_FORMAT_FEATURE_2_LINEAR_COLOR_ATTACHMENT_BIT_NV) == 0) {
                        const char *vuid = use_rp2 ? "VUID-VkSubpassDescription2-linearColorAttachment-06499"
                                                   : "VUID-VkSubpassDescription-linearColorAttachment-06496";
                        skip |= LogError(vuid, device, attachment_loc.dot(Field::format),
                                         "(%s) format features are %s (referenced by %s).", string_VkFormat(attachment_format),
                                         string_VkFormatFeatureFlags2(format_features).c_str(), input_loc.Fields().c_str());
                    }
                }
            }

            if (use_rp2) {
                // These are validated automatically as part of parameter validation for create renderpass 1
                // as they are in a struct that only applies to input attachments - not so for v2.

                // Check for 0
                if (aspect_mask == 0) {
                    skip |= LogError("VUID-VkSubpassDescription2-attachment-02800", device, input_loc.dot(Field::aspectMask),
                                     "is zero.");
                } else {
                    const VkImageAspectFlags valid_bits =
                        (VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT |
                         VK_IMAGE_ASPECT_METADATA_BIT | VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT |
                         VK_IMAGE_ASPECT_PLANE_2_BIT | VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT |
                         VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT |
                         VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT);

                    // Check for valid aspect mask bits
                    if (aspect_mask & ~valid_bits) {
                        skip |= LogError("VUID-VkSubpassDescription2-attachment-02799", device, input_loc.dot(Field::aspectMask),
                                         "(%s) is invalid.", string_VkImageAspectFlags(aspect_mask).c_str());
                    }
                }
            }
        }

        for (uint32_t j = 0; j < subpass.preserveAttachmentCount; ++j) {
            const Location preserve_loc = subpass_loc.dot(Field::preserveAttachmentCount, j);
            uint32_t attachment = subpass.pPreserveAttachments[j];
            if (attachment == VK_ATTACHMENT_UNUSED) {
                const char *vuid =
                    use_rp2 ? "VUID-VkSubpassDescription2-attachment-03073" : "VUID-VkSubpassDescription-attachment-00853";
                skip |= LogError(vuid, device, preserve_loc, "must not be VK_ATTACHMENT_UNUSED.");
            } else {
                skip |= ValidateAttachmentIndex(attachment, pCreateInfo->attachmentCount, preserve_loc);
                if (attachment < pCreateInfo->attachmentCount) {
                    skip |= AddAttachmentUse(attachment_uses, attachment_layouts, attachment, ATTACHMENT_PRESERVE,
                                             VkImageLayout(0) /* preserve doesn't have any layout */, preserve_loc);
                }
            }
        }

        bool subpass_performs_resolve = false;

        for (uint32_t j = 0; j < subpass.colorAttachmentCount; ++j) {
            if (!subpass.pResolveAttachments) {
                continue;
            }
            auto const &attachment_ref = subpass.pResolveAttachments[j];
            if (attachment_ref.attachment == VK_ATTACHMENT_UNUSED) {
                continue;
            }
            const Location resolve_loc = subpass_loc.dot(Field::pResolveAttachments, j);
            const Location attachment_loc = create_info_loc.dot(Field::pAttachments, attachment_ref.attachment);

            skip |= ValidateAttachmentIndex(attachment_ref.attachment, pCreateInfo->attachmentCount, resolve_loc);

            const VkImageLayout attachment_layout = attachment_ref.layout;
            if (IsValueIn(attachment_layout,
                          {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL})) {
                const char *vuid =
                    use_rp2 ? "VUID-VkSubpassDescription2-attachment-06914" : "VUID-VkSubpassDescription-attachment-06914";
                skip |= LogError(vuid, device, resolve_loc.dot(Field::layout), "(%s) is invalid.",
                                 string_VkImageLayout(attachment_layout));
            }
            if (IsValueIn(attachment_layout, {VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
                                              VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL})) {
                const char *vuid =
                    use_rp2 ? "VUID-VkSubpassDescription2-attachment-06917" : "VUID-VkSubpassDescription-attachment-06917";
                skip |= LogError(vuid, device, resolve_loc.dot(Field::layout), "(%s) is invalid.",
                                 string_VkImageLayout(attachment_layout));
            }
            if (IsValueIn(attachment_layout,
                          {VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
                           VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL})) {
                const char *vuid =
                    use_rp2 ? "VUID-VkSubpassDescription2-attachment-06920" : "VUID-VkSubpassDescription-attachment-06920";
                skip |= LogError(vuid, device, resolve_loc.dot(Field::layout), "(%s) is invalid.",
                                 string_VkImageLayout(attachment_layout));
            }
            if (attachment_layout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR) {
                const char *vuid =
                    use_rp2 ? "VUID-VkSubpassDescription2-attachment-06923" : "VUID-VkSubpassDescription-attachment-06923";
                skip |= LogError(vuid, device, resolve_loc.dot(Field::layout), "(%s) is invalid.",
                                 string_VkImageLayout(attachment_layout));
            }

            // safe to dereference pCreateInfo->pAttachments[]
            if (attachment_ref.attachment < pCreateInfo->attachmentCount) {
                const VkFormat attachment_format = pCreateInfo->pAttachments[attachment_ref.attachment].format;
                skip |= ValidateAttachmentReference(attachment_ref, attachment_format, false, resolve_loc);
                skip |= AddAttachmentUse(attachment_uses, attachment_layouts, attachment_ref.attachment, ATTACHMENT_RESOLVE,
                                         attachment_ref.layout, resolve_loc);

                subpass_performs_resolve = true;

                if (pCreateInfo->pAttachments[attachment_ref.attachment].samples != VK_SAMPLE_COUNT_1_BIT) {
                    const char *vuid = use_rp2 ? "VUID-VkSubpassDescription2-pResolveAttachments-03067"
                                               : "VUID-VkSubpassDescription-pResolveAttachments-00849";
                    skip |= LogError(vuid, device, attachment_loc.dot(Field::samples), "is %s (referenced by %s).",
                                     string_VkSampleCountFlagBits(pCreateInfo->pAttachments[attachment_ref.attachment].samples),
                                     resolve_loc.Fields().c_str());
                }

                const VkFormatFeatureFlags2 format_features = GetPotentialFormatFeatures(attachment_format);
                // Can be VK_FORMAT_UNDEFINED with VK_ANDROID_external_format_resolve
                if ((format_features & VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT_KHR) == 0 &&
                    attachment_format != VK_FORMAT_UNDEFINED) {
                    if (!enabled_features.linearColorAttachment) {
                        const char *vuid = use_rp2 ? "VUID-VkSubpassDescription2-pResolveAttachments-09343"
                                                   : "VUID-VkSubpassDescription-pResolveAttachments-02649";

                        skip |= LogError(vuid, device, attachment_loc.dot(Field::format),
                                         "(%s) format features are %s (referenced by %s).", string_VkFormat(attachment_format),
                                         string_VkFormatFeatureFlags2(format_features).c_str(), resolve_loc.Fields().c_str());
                    } else if ((format_features & VK_FORMAT_FEATURE_2_LINEAR_COLOR_ATTACHMENT_BIT_NV) == 0) {
                        const char *vuid = use_rp2 ? "VUID-VkSubpassDescription2-linearColorAttachment-06501"
                                                   : "VUID-VkSubpassDescription-linearColorAttachment-06498";
                        skip |= LogError(vuid, device, attachment_loc.dot(Field::format),
                                         "(%s) format features are %s (referenced by %s).", string_VkFormat(attachment_format),
                                         string_VkFormatFeatureFlags2(format_features).c_str(), resolve_loc.Fields().c_str());
                    }
                }

                //  VK_QCOM_render_pass_shader_resolve check of resolve attachmnents
                if ((subpass.flags & VK_SUBPASS_DESCRIPTION_SHADER_RESOLVE_BIT_QCOM) != 0) {
                    const char *vuid =
                        use_rp2 ? "VUID-VkRenderPassCreateInfo2-flags-04907" : "VUID-VkSubpassDescription-flags-03341";
                    skip |= LogError(vuid, device, resolve_loc,
                                     "contains a reference to attachment %" PRIu32 " instead of being VK_ATTACHMENT_UNUSED.",
                                     attachment_ref.attachment);
                }
            }
        }

        if (subpass.pDepthStencilAttachment && subpass.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
            const uint32_t attachment = subpass.pDepthStencilAttachment->attachment;
            const VkImageLayout image_layout = subpass.pDepthStencilAttachment->layout;
            const Location ds_loc = subpass_loc.dot(Field::pDepthStencilAttachment);
            const Location attachment_loc = create_info_loc.dot(Field::pAttachments, attachment);

            const auto depth_stencil_attachment = pCreateInfo->pAttachments[attachment];

            skip |= ValidateAttachmentIndex(attachment, pCreateInfo->attachmentCount, ds_loc);

            const VkImageLayout attachment_layout = subpass.pDepthStencilAttachment->layout;
            if (IsValueIn(attachment_layout,
                          {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL})) {
                const char *vuid =
                    use_rp2 ? "VUID-VkSubpassDescription2-attachment-06915" : "VUID-VkSubpassDescription-attachment-06915";
                skip |=
                    LogError(vuid, device, ds_loc.dot(Field::layout), "(%s) is invalid.", string_VkImageLayout(attachment_layout));
            }

            if (use_rp2 && IsValueIn(attachment_layout,
                                     {VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL})) {
                if (vku::FindStructInPNextChain<VkAttachmentReferenceStencilLayout>(subpass.pDepthStencilAttachment->pNext)) {
                    const char *vuid = "VUID-VkSubpassDescription2-attachment-06251";
                    skip |= LogError(vuid, device, ds_loc.dot(Field::layout), "(%s) is invalid.",
                                     string_VkImageLayout(attachment_layout));
                }
            }

            // safe to dereference pCreateInfo->pAttachments[]
            if (attachment < pCreateInfo->attachmentCount) {
                const VkFormat attachment_format = depth_stencil_attachment.format;
                skip |= ValidateAttachmentReference(*subpass.pDepthStencilAttachment, attachment_format, false, ds_loc);
                skip |= AddAttachmentUse(attachment_uses, attachment_layouts, attachment, ATTACHMENT_DEPTH, image_layout, ds_loc);

                if (attach_first_use[attachment]) {
                    skip |= ValidateLayoutVsAttachmentDescription(image_layout, attachment, depth_stencil_attachment,
                                                                  ds_loc.dot(Field::layout));
                }
                attach_first_use[attachment] = false;

                const VkFormatFeatureFlags2 format_features = GetPotentialFormatFeatures(attachment_format);
                if ((format_features & VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT_KHR) == 0) {
                    const char *vuid = use_rp2 ? "VUID-VkSubpassDescription2-pDepthStencilAttachment-02900"
                                               : "VUID-VkSubpassDescription-pDepthStencilAttachment-02650";
                    skip |= LogError(vuid, device, attachment_loc.dot(Field::format),
                                     "(%s) format features are %s (referenced by %s).", string_VkFormat(attachment_format),
                                     string_VkFormatFeatureFlags2(format_features).c_str(), ds_loc.Fields().c_str());
                }

                if (use_rp2 && enabled_features.multisampledRenderToSingleSampled && ms_render_to_single_sample &&
                    ms_render_to_single_sample->multisampledRenderToSingleSampledEnable) {
                    const auto depth_stencil_sample_count = depth_stencil_attachment.samples;
                    if ((depth_stencil_sample_count == VK_SAMPLE_COUNT_1_BIT) &&
                        (!subpass_depth_stencil_resolve ||
                         (subpass_depth_stencil_resolve->pDepthStencilResolveAttachment != VK_NULL_HANDLE &&
                          subpass_depth_stencil_resolve->pDepthStencilResolveAttachment->attachment != VK_ATTACHMENT_UNUSED))) {
                        std::stringstream message;
                        message << "has a VkMultisampledRenderToSingleSampledInfoEXT struct in its "
                                   "VkSubpassDescription2 pNext chain with multisampledRenderToSingleSampled set to "
                                   "VK_TRUE and pDepthStencilAttachment has a sample count of VK_SAMPLE_COUNT_1_BIT ";
                        if (!subpass_depth_stencil_resolve) {
                            message << "but there is no VkSubpassDescriptionDepthStencilResolve in the pNext chain of "
                                       "the VkSubpassDescription2 struct for this subpass";
                        } else {
                            message << "but the pSubpassResolveAttachment member of the "
                                       "VkSubpassDescriptionDepthStencilResolve in the pNext chain of "
                                       "the VkSubpassDescription2 struct for this subpass is not NULL, and its attachment "
                                       "is not VK_ATTACHMENT_UNUSED";
                        }
                        skip |= LogError("VUID-VkSubpassDescription2-pNext-06871", device, ds_loc, "%s", message.str().c_str());
                    }
                    if (subpass_depth_stencil_resolve) {
                        if (subpass_depth_stencil_resolve->depthResolveMode == VK_RESOLVE_MODE_NONE &&
                            subpass_depth_stencil_resolve->stencilResolveMode == VK_RESOLVE_MODE_NONE) {
                            skip |= LogError(
                                "VUID-VkSubpassDescriptionDepthStencilResolve-pNext-06873", device,
                                subpass_loc.pNext(Struct::VkSubpassDescriptionDepthStencilResolve, Field::depthResolveMode),
                                "and stencilResolveMode are VK_RESOLVE_MODE_NONE, and "
                                "VkMultisampledRenderToSingleSampledInfoEXT::multisampledRenderToSingleSampled was set to "
                                "VK_TRUE.");
                        }
                        if (vkuFormatHasDepth(attachment_format)) {
                            if (subpass_depth_stencil_resolve->depthResolveMode != VK_RESOLVE_MODE_NONE &&
                                !(subpass_depth_stencil_resolve->depthResolveMode &
                                  phys_dev_props_core12.supportedDepthResolveModes)) {
                                skip |= LogError(
                                    "VUID-VkSubpassDescriptionDepthStencilResolve-pNext-06874", device,
                                    subpass_loc.pNext(Struct::VkSubpassDescriptionDepthStencilResolve, Field::depthResolveMode),
                                    "(%s) is not supported in supportedDepthResolveModes (%s), and %s is %s, and "
                                    "VkMultisampledRenderToSingleSampledInfoEXT::multisampledRenderToSingleSampled was set to "
                                    "VK_TRUE.",
                                    string_VkResolveModeFlagBits(subpass_depth_stencil_resolve->depthResolveMode),
                                    string_VkResolveModeFlags(phys_dev_props_core12.supportedDepthResolveModes).c_str(),
                                    attachment_loc.dot(Field::format).Fields().c_str(), string_VkFormat(attachment_format));
                            }
                        }
                        if (vkuFormatHasStencil(attachment_format)) {
                            if (subpass_depth_stencil_resolve->stencilResolveMode != VK_RESOLVE_MODE_NONE &&
                                !(subpass_depth_stencil_resolve->stencilResolveMode &
                                  phys_dev_props_core12.supportedStencilResolveModes)) {
                                skip |= LogError(
                                    "VUID-VkSubpassDescriptionDepthStencilResolve-pNext-06875", device,
                                    subpass_loc.pNext(Struct::VkSubpassDescriptionDepthStencilResolve, Field::stencilResolveMode),
                                    "(%s) is not supported in supportedStencilResolveModes (%s), and %s is %s, and "
                                    "VkMultisampledRenderToSingleSampledInfoEXT::multisampledRenderToSingleSampled was set to "
                                    "VK_TRUE.",
                                    string_VkResolveModeFlagBits(subpass_depth_stencil_resolve->stencilResolveMode),
                                    string_VkResolveModeFlags(phys_dev_props_core12.supportedStencilResolveModes).c_str(),
                                    attachment_loc.dot(Field::format).Fields().c_str(), string_VkFormat(attachment_format));
                            }
                        }
                        if (vkuFormatIsDepthAndStencil(attachment_format)) {
                            if (phys_dev_props_core12.independentResolve == VK_FALSE &&
                                phys_dev_props_core12.independentResolveNone == VK_FALSE &&
                                (subpass_depth_stencil_resolve->stencilResolveMode !=
                                 subpass_depth_stencil_resolve->depthResolveMode)) {
                                skip |= LogError(
                                    "VUID-VkSubpassDescriptionDepthStencilResolve-pNext-06876", device,
                                    subpass_loc.pNext(Struct::VkSubpassDescriptionDepthStencilResolve, Field::depthResolveMode),
                                    "(%s) is different from  stencilResolveMode (%s), and %s is %s, and "
                                    "VkMultisampledRenderToSingleSampledInfoEXT::multisampledRenderToSingleSampled was set to "
                                    "VK_TRUE.",
                                    string_VkResolveModeFlagBits(subpass_depth_stencil_resolve->stencilResolveMode),
                                    string_VkResolveModeFlagBits(subpass_depth_stencil_resolve->depthResolveMode),
                                    attachment_loc.dot(Field::format).Fields().c_str(), string_VkFormat(attachment_format));
                            }
                            if (phys_dev_props_core12.independentResolve == VK_FALSE &&
                                phys_dev_props_core12.independentResolveNone == VK_TRUE &&
                                ((subpass_depth_stencil_resolve->stencilResolveMode !=
                                  subpass_depth_stencil_resolve->depthResolveMode) &&
                                 ((subpass_depth_stencil_resolve->depthResolveMode != VK_RESOLVE_MODE_NONE) &&
                                  (subpass_depth_stencil_resolve->stencilResolveMode != VK_RESOLVE_MODE_NONE)))) {
                                skip |= LogError(
                                    "VUID-VkSubpassDescriptionDepthStencilResolve-pNext-06877", device,
                                    subpass_loc.pNext(Struct::VkSubpassDescriptionDepthStencilResolve, Field::depthResolveMode),
                                    "(%s) is different from  stencilResolveMode (%s), and %s is %s, and "
                                    "VkMultisampledRenderToSingleSampledInfoEXT::multisampledRenderToSingleSampled was set to "
                                    "VK_TRUE.",
                                    string_VkResolveModeFlagBits(subpass_depth_stencil_resolve->stencilResolveMode),
                                    string_VkResolveModeFlagBits(subpass_depth_stencil_resolve->depthResolveMode),
                                    attachment_loc.dot(Field::format).Fields().c_str(), string_VkFormat(attachment_format));
                            }
                        }
                    }
                }

                if (IsImageLayoutDepthOnly(subpass.pDepthStencilAttachment->layout)) {
                    if (vku::FindStructInPNextChain<VkAttachmentReferenceStencilLayout>(subpass.pDepthStencilAttachment->pNext) == nullptr) {
                        if (vkuFormatIsDepthAndStencil(attachment_format)) {
                            skip |=
                                LogError("VUID-VkRenderPassCreateInfo2-attachment-06244", device, attachment_loc.dot(Field::format),
                                         "(%s) has both depth and stencil components (referenced by %s).",
                                         string_VkFormat(attachment_format), ds_loc.Fields().c_str());
                        }
                    }
                    if (vkuFormatIsStencilOnly(attachment_format)) {
                        skip |= LogError("VUID-VkRenderPassCreateInfo2-attachment-06246", device, attachment_loc.dot(Field::format),
                                         "(%s) only has a stencil component (referenced by %s).",
                                         string_VkFormat(attachment_format), ds_loc.Fields().c_str());
                    }
                }

                if (IsImageLayoutStencilOnly(subpass.pDepthStencilAttachment->layout)) {
                    if (!vkuFormatIsStencilOnly(attachment_format)) {
                        skip |= LogError("VUID-VkRenderPassCreateInfo2-attachment-06245", device, attachment_loc.dot(Field::format),
                                         "(%s) does not only have a stencil component (referenced by %s).",
                                         string_VkFormat(attachment_format), ds_loc.Fields().c_str());
                    }
                }
            }
        }

        uint32_t last_sample_count_attachment = VK_ATTACHMENT_UNUSED;
        for (uint32_t j = 0; j < subpass.colorAttachmentCount; ++j) {
            auto const &attachment_ref = subpass.pColorAttachments[j];
            const uint32_t attachment_index = attachment_ref.attachment;
            const Location color_loc = subpass_loc.dot(Field::pColorAttachments, j);
            if (attachment_index != VK_ATTACHMENT_UNUSED) {
                const Location attachment_loc = create_info_loc.dot(Field::pAttachments, attachment_index);
                skip |= ValidateAttachmentIndex(attachment_index, pCreateInfo->attachmentCount, color_loc);

                const VkImageLayout attachment_layout = attachment_ref.layout;
                if (IsValueIn(attachment_layout,
                              {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL})) {
                    const char *vuid =
                        use_rp2 ? "VUID-VkSubpassDescription2-attachment-06913" : "VUID-VkSubpassDescription-attachment-06913";
                    skip |= LogError(vuid, device, color_loc.dot(Field::layout), "(%s) is invalid.",
                                     string_VkImageLayout(attachment_layout));
                }
                if (IsValueIn(attachment_layout, {VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
                                                  VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL})) {
                    const char *vuid =
                        use_rp2 ? "VUID-VkSubpassDescription2-attachment-06916" : "VUID-VkSubpassDescription-attachment-06916";
                    skip |= LogError(vuid, device, color_loc.dot(Field::layout), "(%s) is invalid.",
                                     string_VkImageLayout(attachment_layout));
                }
                if (IsValueIn(attachment_layout,
                              {VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
                               VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL})) {
                    const char *vuid =
                        use_rp2 ? "VUID-VkSubpassDescription2-attachment-06919" : "VUID-VkSubpassDescription-attachment-06919";
                    skip |= LogError(vuid, device, color_loc.dot(Field::layout), "(%s) is invalid.",
                                     string_VkImageLayout(attachment_layout));
                }
                if (attachment_layout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR) {
                    const char *vuid =
                        use_rp2 ? "VUID-VkSubpassDescription2-attachment-06922" : "VUID-VkSubpassDescription-attachment-06922";
                    skip |= LogError(vuid, device, color_loc.dot(Field::layout), "(%s) is invalid.",
                                     string_VkImageLayout(attachment_layout));
                }

                // safe to dereference pCreateInfo->pAttachments[]
                if (attachment_index < pCreateInfo->attachmentCount) {
                    const VkAttachmentDescription2 &attachment_description = pCreateInfo->pAttachments[attachment_index];
                    const VkFormat attachment_format = attachment_description.format;
                    skip |= ValidateAttachmentReference(attachment_ref, attachment_format, false, color_loc);
                    skip |= AddAttachmentUse(attachment_uses, attachment_layouts, attachment_index, ATTACHMENT_COLOR,
                                             attachment_ref.layout, color_loc);

                    VkSampleCountFlagBits current_sample_count = attachment_description.samples;
                    if (use_rp2 &&
                        enabled_features.multisampledRenderToSingleSampled &&
                        ms_render_to_single_sample && ms_render_to_single_sample->multisampledRenderToSingleSampledEnable) {
                        if (current_sample_count != VK_SAMPLE_COUNT_1_BIT &&
                            current_sample_count != ms_render_to_single_sample->rasterizationSamples) {
                            skip |= LogError("VUID-VkSubpassDescription2-pNext-06870", device,
                                             create_info_loc.pNext(Struct::VkMultisampledRenderToSingleSampledInfoEXT,
                                                                   Field::rasterizationSamples),
                                             "(%s) doesn't match %s (%s) (referenced by %s).",
                                             string_VkSampleCountFlagBits(ms_render_to_single_sample->rasterizationSamples),
                                             attachment_loc.dot(Field::samples).Fields().c_str(),
                                             string_VkSampleCountFlagBits(current_sample_count), color_loc.Fields().c_str());
                        }
                    }
                    if (last_sample_count_attachment != VK_ATTACHMENT_UNUSED) {
                        if (!(IsExtEnabled(device_extensions.vk_amd_mixed_attachment_samples) ||
                              IsExtEnabled(device_extensions.vk_nv_framebuffer_mixed_samples) ||
                              (enabled_features.multisampledRenderToSingleSampled && use_rp2))) {
                            VkSampleCountFlagBits last_sample_count =
                                pCreateInfo->pAttachments[subpass.pColorAttachments[last_sample_count_attachment].attachment]
                                    .samples;
                            if (current_sample_count != last_sample_count) {
                                const char *vuid = use_rp2 ? "VUID-VkSubpassDescription2-multisampledRenderToSingleSampled-06872"
                                                           : "VUID-VkSubpassDescription-pColorAttachments-06868";
                                skip |= LogError(vuid, device, attachment_loc.dot(Field::samples),
                                                 "is %s, but the pColorAttachments[%" PRIu32 "] has sample count %s.",
                                                 string_VkSampleCountFlagBits(current_sample_count), last_sample_count_attachment,
                                                 string_VkSampleCountFlagBits(last_sample_count));
                            }
                        }
                    }
                    last_sample_count_attachment = j;

                    if (subpass_performs_resolve && current_sample_count == VK_SAMPLE_COUNT_1_BIT &&
                        !enabled_features.externalFormatResolve) {
                        const char *vuid = use_rp2 ? "VUID-VkSubpassDescription2-externalFormatResolve-09338"
                                                   : "VUID-VkSubpassDescription-pResolveAttachments-00848";
                        skip |= LogError(vuid, device, attachment_loc.dot(Field::samples), "is VK_SAMPLE_COUNT_1_BIT.");
                    }

                    if (subpass.pDepthStencilAttachment && subpass.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED &&
                        subpass.pDepthStencilAttachment->attachment < pCreateInfo->attachmentCount) {
                        const auto depth_stencil_sample_count =
                            pCreateInfo->pAttachments[subpass.pDepthStencilAttachment->attachment].samples;

                        if (IsExtEnabled(device_extensions.vk_amd_mixed_attachment_samples)) {
                            if (current_sample_count > depth_stencil_sample_count) {
                                const char *vuid = use_rp2 ? "VUID-VkSubpassDescription2-pColorAttachments-03070"
                                                           : "VUID-VkSubpassDescription-pColorAttachments-01506";
                                skip |= LogError(vuid, device, attachment_loc.dot(Field::samples),
                                                 "%s) (referenced by %s) is larger than from pCreateInfo->pAttachments[%" PRIu32
                                                 "].samples (%s) (referenced by %s).",
                                                 string_VkSampleCountFlagBits(current_sample_count), color_loc.Fields().c_str(),
                                                 subpass.pDepthStencilAttachment->attachment,
                                                 string_VkSampleCountFlagBits(depth_stencil_sample_count),
                                                 subpass_loc.dot(Field::pDepthStencilAttachment).Fields().c_str());

                                break;
                            }
                        }

                        if (!IsExtEnabled(device_extensions.vk_amd_mixed_attachment_samples) &&
                            !IsExtEnabled(device_extensions.vk_nv_framebuffer_mixed_samples) &&
                            !(use_rp2 && enabled_features.multisampledRenderToSingleSampled) &&
                            current_sample_count != depth_stencil_sample_count) {
                            const char *vuid = use_rp2 ? "VUID-VkSubpassDescription2-multisampledRenderToSingleSampled-06872"
                                                       : "VUID-VkSubpassDescription-pDepthStencilAttachment-01418";
                            skip |= LogError(vuid, device, attachment_loc.dot(Field::samples),
                                             "%s) (referenced by %s) is different from pCreateInfo->pAttachments[%" PRIu32
                                             "].samples (%s) (referenced by %s).",
                                             string_VkSampleCountFlagBits(current_sample_count), color_loc.Fields().c_str(),
                                             subpass.pDepthStencilAttachment->attachment,
                                             string_VkSampleCountFlagBits(depth_stencil_sample_count),
                                             subpass_loc.dot(Field::pDepthStencilAttachment).Fields().c_str());
                            break;
                        }
                    }

                    const VkFormatFeatureFlags2 format_features = GetPotentialFormatFeatures(attachment_format);
                    // Can be VK_FORMAT_UNDEFINED with VK_ANDROID_external_format_resolve
                    if ((format_features & VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT_KHR) == 0 &&
                        attachment_format != VK_FORMAT_UNDEFINED) {
                        if (!enabled_features.linearColorAttachment) {
                            const char *vuid = use_rp2 ? "VUID-VkSubpassDescription2-pColorAttachments-02898"
                                                       : "VUID-VkSubpassDescription-pColorAttachments-02648";
                            skip |= LogError(vuid, device, attachment_loc.dot(Field::format),
                                             "(%s) format features are %s (referenced by %s).", string_VkFormat(attachment_format),
                                             string_VkFormatFeatureFlags2(format_features).c_str(), color_loc.Fields().c_str());
                        } else if ((format_features & VK_FORMAT_FEATURE_2_LINEAR_COLOR_ATTACHMENT_BIT_NV) == 0) {
                            const char *vuid = use_rp2 ? "VUID-VkSubpassDescription2-linearColorAttachment-06500"
                                                       : "VUID-VkSubpassDescription-linearColorAttachment-06497";
                            skip |= LogError(vuid, device, attachment_loc.dot(Field::format),
                                             "(%s) format features are %s (referenced by %s).", string_VkFormat(attachment_format),
                                             string_VkFormatFeatureFlags2(format_features).c_str(), color_loc.Fields().c_str());
                        }
                    }

                    if (attach_first_use[attachment_index]) {
                        skip |= ValidateLayoutVsAttachmentDescription(subpass.pColorAttachments[j].layout, attachment_index,
                                                                      attachment_description, color_loc.dot(Field::layout));
                    }
                    attach_first_use[attachment_index] = false;
                }
            }

            if (subpass_performs_resolve && subpass.pResolveAttachments[j].attachment != VK_ATTACHMENT_UNUSED &&
                subpass.pResolveAttachments[j].attachment < pCreateInfo->attachmentCount) {
                auto const &resolve_attachment_ref = subpass.pResolveAttachments[j];
                const uint32_t resolve_attachment_index = resolve_attachment_ref.attachment;
                const auto &resolve_desc = pCreateInfo->pAttachments[resolve_attachment_index];
                const Location &resolve_loc = subpass_loc.dot(Field::pResolveAttachments, j);
                if (enabled_features.externalFormatResolve && resolve_desc.format == VK_FORMAT_UNDEFINED) {
                    if (attachment_index == VK_ATTACHMENT_UNUSED) {
                        if (!android_external_format_resolve_null_color_attachment_prop) {
                            skip |= LogError("VUID-VkSubpassDescription2-nullColorAttachmentWithExternalFormatResolve-09336",
                                             device, resolve_loc.dot(Field::attachment),
                                             "is %" PRIu32 ", pAttachments[%" PRIu32
                                             "].format is VK_FORMAT_UNDEFINED, nullColorAttachmentWithExternalFormatResolve is "
                                             "VK_FALSE, but %s is VK_ATTACHMENT_UNUSED.",
                                             resolve_attachment_index, resolve_attachment_index,
                                             color_loc.dot(Field::attachment).Fields().c_str());
                        }

                    } else {
                        if (android_external_format_resolve_null_color_attachment_prop) {
                            skip |= LogError("VUID-VkSubpassDescription2-nullColorAttachmentWithExternalFormatResolve-09337",
                                             device, resolve_loc.dot(Field::attachment),
                                             "is %" PRIu32 ", pAttachments[%" PRIu32
                                             "].format is VK_FORMAT_UNDEFINED, nullColorAttachmentWithExternalFormatResolve is "
                                             "VK_TRUE, but %s is %" PRIu32 ".",
                                             resolve_attachment_index, resolve_attachment_index,
                                             color_loc.dot(Field::attachment).Fields().c_str(), attachment_index);
                        }

                        const auto &color_desc = pCreateInfo->pAttachments[attachment_index];
                        if (color_desc.samples != VK_SAMPLE_COUNT_1_BIT) {
                            skip |= LogError("VUID-VkSubpassDescription2-externalFormatResolve-09345", device,
                                             create_info_loc.dot(Field::pAttachments, attachment_index).dot(Field::samples),
                                             "is %s (referenced by %s).", string_VkSampleCountFlagBits(color_desc.samples),
                                             color_loc.Fields().c_str());
                        }
                    }

                    if (subpass.colorAttachmentCount != 1) {
                        skip |= LogError("VUID-VkSubpassDescription2-externalFormatResolve-09344", device,
                                         resolve_loc.dot(Field::attachment),
                                         "is %" PRIu32 ", pAttachments[%" PRIu32
                                         "].format is VK_FORMAT_UNDEFINED, but colorAttachmentCount is %" PRIu32 ".",
                                         resolve_attachment_index, resolve_attachment_index, subpass.colorAttachmentCount);
                        // if this is more then 1, will spam for every instance so just break loop
                        break;
                    }
                    if (subpass.viewMask != 0) {
                        skip |= LogError(
                            "VUID-VkSubpassDescription2-externalFormatResolve-09346", device, resolve_loc.dot(Field::attachment),
                            "is %" PRIu32 ", pAttachments[%" PRIu32 "].format is VK_FORMAT_UNDEFINED, but viewMask is %" PRIu32 ".",
                            resolve_attachment_index, resolve_attachment_index, subpass.viewMask);
                    }

                    for (uint32_t k = 0; k < subpass.inputAttachmentCount; ++k) {
                        const uint32_t input_attachment_index = subpass.pInputAttachments[k].attachment;
                        if (input_attachment_index == VK_ATTACHMENT_UNUSED) {
                            continue;
                        }
                        if (input_attachment_index == resolve_attachment_index &&
                            IsAnyPlaneAspect(resolve_attachment_ref.aspectMask)) {
                            skip |= LogError("VUID-VkSubpassDescription2-externalFormatResolve-09348", device,
                                             resolve_loc.dot(Field::attachment),
                                             "is %" PRIu32 " (reference also by pInputAttachments[%" PRIu32
                                             "]), but the resolve aspectMask is %s.",
                                             resolve_attachment_index, k,
                                             string_VkImageAspectFlags(resolve_attachment_ref.aspectMask).c_str());
                        } else if (input_attachment_index == attachment_index && IsAnyPlaneAspect(attachment_ref.aspectMask)) {
                            skip |= LogError("VUID-VkSubpassDescription2-externalFormatResolve-09348", device,
                                             color_loc.dot(Field::attachment),
                                             "is %" PRIu32 " (reference also by pInputAttachments[%" PRIu32
                                             "]), but the resolve aspectMask is %s.",
                                             attachment_index, k, string_VkImageAspectFlags(attachment_ref.aspectMask).c_str());
                        }
                    }

                    const auto *fragment_density_map_info =
                        vku::FindStructInPNextChain<VkRenderPassFragmentDensityMapCreateInfoEXT>(pCreateInfo->pNext);
                    if (fragment_density_map_info &&
                        fragment_density_map_info->fragmentDensityMapAttachment.attachment != VK_ATTACHMENT_UNUSED) {
                        skip |= LogError(
                            "VUID-VkRenderPassCreateInfo2-pResolveAttachments-09331", device,
                            create_info_loc.dot(Field::pAttachments, attachment_index).dot(Field::format),
                            "is VK_FORMAT_UNDEFINED (referenced by %s) but %s is %" PRIu32 ".", resolve_loc.Fields().c_str(),
                            create_info_loc
                                .pNext(Struct::VkRenderPassFragmentDensityMapCreateInfoEXT, Field::fragmentDensityMapAttachment)
                                .dot(Field::attachment)
                                .Fields()
                                .c_str(),
                            fragment_density_map_info->fragmentDensityMapAttachment.attachment);
                    }
                } else if (attachment_index == VK_ATTACHMENT_UNUSED) {
                    const char *vuid = use_rp2 ? "VUID-VkSubpassDescription2-externalFormatResolve-09335"
                                               : "VUID-VkSubpassDescription-pResolveAttachments-00847";
                    skip |= LogError(vuid, device, resolve_loc.dot(Field::attachment),
                                     "is %" PRIu32 ", but %s is VK_ATTACHMENT_UNUSED.", resolve_attachment_index,
                                     color_loc.dot(Field::attachment).Fields().c_str());
                } else {
                    const auto &color_desc = pCreateInfo->pAttachments[attachment_index];
                    if (color_desc.format != resolve_desc.format) {
                        const char *vuid = use_rp2 ? "VUID-VkSubpassDescription2-externalFormatResolve-09339"
                                                   : "VUID-VkSubpassDescription-pResolveAttachments-00850";
                        skip |= LogError(
                            vuid, device, create_info_loc.dot(Field::pAttachments, attachment_index).dot(Field::format),
                            "(%s) (referenced by %s) is different than pAttachments[%" PRIu32 "].format (%s) (referenced by %s).",
                            string_VkFormat(color_desc.format), color_loc.Fields().c_str(), resolve_attachment_index,
                            string_VkFormat(resolve_desc.format), resolve_loc.Fields().c_str());
                    }
                }
            }
        }

        if (use_rp2 && enabled_features.multisampledRenderToSingleSampled && ms_render_to_single_sample) {
            if (ms_render_to_single_sample->rasterizationSamples == VK_SAMPLE_COUNT_1_BIT) {
                skip |=
                    LogError("VUID-VkMultisampledRenderToSingleSampledInfoEXT-rasterizationSamples-06878", device,
                             create_info_loc.pNext(Struct::VkMultisampledRenderToSingleSampledInfoEXT, Field::rasterizationSamples),
                             "is VK_SAMPLE_COUNT_1_BIT, which is not allowed.");
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateDependencies(const FRAMEBUFFER_STATE &framebuffer_state, const RENDER_PASS_STATE &render_pass_state,
                                      const ErrorObject &error_obj) const {
    bool skip = false;
    auto const framebuffer_info = framebuffer_state.createInfo.ptr();
    auto const create_info = render_pass_state.createInfo.ptr();
    auto const &subpass_to_node = render_pass_state.subpass_to_node;

    struct Attachment {
        std::vector<SubpassLayout> outputs;
        std::vector<SubpassLayout> inputs;
        std::vector<uint32_t> overlapping;
    };

    std::vector<Attachment> attachments(create_info->attachmentCount);

    if (!(framebuffer_info->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT)) {
        // Find overlapping attachments
        for (uint32_t i = 0; i < framebuffer_info->attachmentCount; ++i) {
            for (uint32_t j = i + 1; j < framebuffer_info->attachmentCount; ++j) {
                VkImageView viewi = framebuffer_info->pAttachments[i];
                VkImageView viewj = framebuffer_info->pAttachments[j];
                if (viewi == viewj) {
                    attachments[i].overlapping.emplace_back(j);
                    attachments[j].overlapping.emplace_back(i);
                    continue;
                }
                if (i >= framebuffer_state.attachments_view_state.size() || j >= framebuffer_state.attachments_view_state.size()) {
                    continue;
                }
                auto *view_state_i = framebuffer_state.attachments_view_state[i].get();
                auto *view_state_j = framebuffer_state.attachments_view_state[j].get();
                if (!view_state_i || !view_state_j) {
                    continue;
                }
                auto view_ci_i = view_state_i->create_info;
                auto view_ci_j = view_state_j->create_info;
                if (view_ci_i.image == view_ci_j.image &&
                    IsRegionOverlapping(view_ci_i.subresourceRange, view_ci_j.subresourceRange)) {
                    attachments[i].overlapping.emplace_back(j);
                    attachments[j].overlapping.emplace_back(i);
                    continue;
                }
                auto *image_data_i = view_state_i->image_state.get();
                auto *image_data_j = view_state_j->image_state.get();
                if (!image_data_i || !image_data_j) {
                    continue;
                }

                if (!image_data_i->sparse && !image_data_j->sparse) {
                    subresource_adapter::ImageRangeGenerator generator_i{*image_data_i->fragment_encoder.get(),
                                                                         view_state_i->create_info.subresourceRange, 0u,
                                                                         view_state_i->IsDepthSliced()};

                    subresource_adapter::ImageRangeGenerator generator_j{*image_data_j->fragment_encoder.get(),
                                                                         view_state_j->create_info.subresourceRange, 0u,
                                                                         view_state_j->IsDepthSliced()};
                    for (; generator_i->non_empty(); ++generator_i) {
                        subresource_adapter::ImageRangeGenerator generator_j_copy = generator_j;
                        for (; generator_j_copy->non_empty(); ++generator_j_copy) {
                            sparse_container::range<VkDeviceSize> range_i{generator_i->begin, generator_i->end};
                            sparse_container::range<VkDeviceSize> range_j{generator_j_copy->begin, generator_j_copy->end};

                            if (image_data_i->DoesResourceMemoryOverlap(range_i, image_data_j, range_j)) {
                                attachments[i].overlapping.emplace_back(j);
                                attachments[j].overlapping.emplace_back(i);
                            }
                        }
                    }
                }
            }
        }
    }
    // Find for each attachment the subpasses that use them.
    vvl::unordered_set<uint32_t> attachment_indices;
    for (uint32_t i = 0; i < create_info->subpassCount; ++i) {
        const VkSubpassDescription2 &subpass = create_info->pSubpasses[i];
        attachment_indices.clear();
        for (uint32_t j = 0; j < subpass.inputAttachmentCount; ++j) {
            uint32_t attachment = subpass.pInputAttachments[j].attachment;
            if (attachment == VK_ATTACHMENT_UNUSED) continue;
            SubpassLayout sp = {i, subpass.pInputAttachments[j].layout};
            attachments[attachment].inputs.emplace_back(sp);
            for (auto overlapping_attachment : attachments[attachment].overlapping) {
                attachments[overlapping_attachment].inputs.emplace_back(sp);
            }
        }
        for (uint32_t j = 0; j < subpass.colorAttachmentCount; ++j) {
            uint32_t attachment = subpass.pColorAttachments[j].attachment;
            if (attachment == VK_ATTACHMENT_UNUSED) continue;
            SubpassLayout sp = {i, subpass.pColorAttachments[j].layout};
            attachments[attachment].outputs.emplace_back(sp);
            for (auto overlapping_attachment : attachments[attachment].overlapping) {
                attachments[overlapping_attachment].outputs.emplace_back(sp);
            }
            attachment_indices.insert(attachment);
        }
        if (subpass.pDepthStencilAttachment && subpass.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
            uint32_t attachment = subpass.pDepthStencilAttachment->attachment;
            SubpassLayout sp = {i, subpass.pDepthStencilAttachment->layout};
            attachments[attachment].outputs.emplace_back(sp);
            for (auto overlapping_attachment : attachments[attachment].overlapping) {
                attachments[overlapping_attachment].outputs.emplace_back(sp);
            }

            if (attachment_indices.count(attachment)) {
                skip |= LogError(kVUID_Core_DrawState_InvalidRenderpass, render_pass_state.renderPass(), error_obj.location,
                                 "Cannot use same attachment (%" PRIu32 ") as both color and depth output in same subpass (%" PRIu32
                                 ").",
                                 attachment, i);
            }
        }
    }
    // If there is a dependency needed make sure one exists
    for (uint32_t i = 0; i < create_info->subpassCount; ++i) {
        const Location subpass_loc = error_obj.location.dot(Field::pSubpasses, i);
        const VkSubpassDescription2 &subpass = create_info->pSubpasses[i];
        // If the attachment is an input then all subpasses that output must have a dependency relationship
        for (uint32_t j = 0; j < subpass.inputAttachmentCount; ++j) {
            uint32_t attachment = subpass.pInputAttachments[j].attachment;
            if (attachment == VK_ATTACHMENT_UNUSED) continue;
            const Location attachment_loc = subpass_loc.dot(Field::pInputAttachments, j);
            CheckDependencyExists(render_pass_state.renderPass(), i, subpass.pInputAttachments[j].layout,
                                  attachments[attachment].outputs, subpass_to_node, attachment_loc, skip);
        }
        // If the attachment is an output then all subpasses that use the attachment must have a dependency relationship
        for (uint32_t j = 0; j < subpass.colorAttachmentCount; ++j) {
            uint32_t attachment = subpass.pColorAttachments[j].attachment;
            if (attachment == VK_ATTACHMENT_UNUSED) continue;
            const Location attachment_loc = subpass_loc.dot(Field::pColorAttachments, j);
            CheckDependencyExists(render_pass_state.renderPass(), i, subpass.pColorAttachments[j].layout,
                                  attachments[attachment].outputs, subpass_to_node, attachment_loc, skip);
            CheckDependencyExists(render_pass_state.renderPass(), i, subpass.pColorAttachments[j].layout,
                                  attachments[attachment].inputs, subpass_to_node, attachment_loc, skip);
        }
        if (subpass.pDepthStencilAttachment && subpass.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
            const uint32_t &attachment = subpass.pDepthStencilAttachment->attachment;
            const Location attachment_loc = subpass_loc.dot(Field::pDepthStencilAttachment);
            CheckDependencyExists(render_pass_state.renderPass(), i, subpass.pDepthStencilAttachment->layout,
                                  attachments[attachment].outputs, subpass_to_node, attachment_loc, skip);
            CheckDependencyExists(render_pass_state.renderPass(), i, subpass.pDepthStencilAttachment->layout,
                                  attachments[attachment].inputs, subpass_to_node, attachment_loc, skip);
        }
    }
    // Loop through implicit dependencies, if this pass reads make sure the attachment is preserved for all passes after it was
    // written.
    for (uint32_t i = 0; i < create_info->subpassCount; ++i) {
        const Location subpass_loc = error_obj.location.dot(Field::pSubpasses, i);
        const VkSubpassDescription2 &subpass = create_info->pSubpasses[i];
        for (uint32_t j = 0; j < subpass.inputAttachmentCount; ++j) {
            const Location attachment_loc = subpass_loc.dot(Field::pInputAttachments, j);
            CheckPreserved(render_pass_state.renderPass(), create_info, i, subpass.pInputAttachments[j].attachment, subpass_to_node,
                           0, attachment_loc, skip);
        }
    }
    return skip;
}

bool CoreChecks::ValidateRenderPassDAG(const VkRenderPassCreateInfo2 *pCreateInfo, const ErrorObject &error_obj) const {
    bool skip = false;
    const char *vuid;
    const bool use_rp2 = error_obj.location.function != Func::vkCreateRenderPass;

    for (uint32_t i = 0; i < pCreateInfo->dependencyCount; ++i) {
        const Location dependencies_loc = error_obj.location.dot(Field::pDependencies, i);
        const VkSubpassDependency2 &dependency = pCreateInfo->pDependencies[i];

        // The first subpass here serves as a good proxy for "is multiview enabled" - since all view masks need to be non-zero if
        // any are, which enables multiview.
        if (use_rp2 && (dependency.dependencyFlags & VK_DEPENDENCY_VIEW_LOCAL_BIT) && (pCreateInfo->pSubpasses[0].viewMask == 0)) {
            skip |= LogError("VUID-VkRenderPassCreateInfo2-viewMask-03059", device, dependencies_loc,
                             "specifies the VK_DEPENDENCY_VIEW_LOCAL_BIT, but multiview is not enabled for this render pass.");
        } else if (use_rp2 && !(dependency.dependencyFlags & VK_DEPENDENCY_VIEW_LOCAL_BIT) && dependency.viewOffset != 0) {
            skip |= LogError("VUID-VkSubpassDependency2-dependencyFlags-03092", device, dependencies_loc,
                             "specifies the VK_DEPENDENCY_VIEW_LOCAL_BIT, but also specifies a view offset of %" PRIu32 ".",
                             dependency.viewOffset);
        } else if (dependency.srcSubpass == VK_SUBPASS_EXTERNAL || dependency.dstSubpass == VK_SUBPASS_EXTERNAL) {
            if (dependency.srcSubpass == dependency.dstSubpass) {
                vuid = use_rp2 ? "VUID-VkSubpassDependency2-srcSubpass-03085" : "VUID-VkSubpassDependency-srcSubpass-00865";
                skip |= LogError(vuid, device, dependencies_loc, "srcSubpass and dstSubpass both VK_SUBPASS_EXTERNAL.");
            } else if (dependency.dependencyFlags & VK_DEPENDENCY_VIEW_LOCAL_BIT) {
                if (dependency.srcSubpass == VK_SUBPASS_EXTERNAL) {
                    vuid = "VUID-VkSubpassDependency-dependencyFlags-02520";
                } else {  // dependency.dstSubpass == VK_SUBPASS_EXTERNAL
                    vuid = "VUID-VkSubpassDependency-dependencyFlags-02521";
                }
                if (use_rp2) {
                    // Create render pass 2 distinguishes between source and destination external dependencies.
                    if (dependency.srcSubpass == VK_SUBPASS_EXTERNAL) {
                        vuid = "VUID-VkSubpassDependency2-dependencyFlags-03090";
                    } else {
                        vuid = "VUID-VkSubpassDependency2-dependencyFlags-03091";
                    }
                }
                skip |= LogError(vuid, device, dependencies_loc,
                                 "specifies an external dependency but also specifies VK_DEPENDENCY_VIEW_LOCAL_BIT.");
            }
        } else if (dependency.srcSubpass > dependency.dstSubpass) {
            vuid = use_rp2 ? "VUID-VkSubpassDependency2-srcSubpass-03084" : "VUID-VkSubpassDependency-srcSubpass-00864";
            skip |= LogError(vuid, device, dependencies_loc,
                             "specifies a dependency from a later subpass (%" PRIu32 ") to an earlier subpass (%" PRIu32
                             "), which is "
                             "disallowed to prevent cyclic dependencies.",
                             dependency.srcSubpass, dependency.dstSubpass);
        } else if (dependency.srcSubpass == dependency.dstSubpass) {
            if (dependency.viewOffset != 0) {
                vuid = use_rp2 ? "VUID-VkSubpassDependency2-viewOffset-02530" : "VUID-VkRenderPassCreateInfo-pNext-01930";
                skip |=
                    LogError(vuid, device, dependencies_loc,
                             "specifies a self-dependency but has a non-zero view offset of %" PRIu32 "", dependency.viewOffset);
            } else if ((dependency.dependencyFlags | VK_DEPENDENCY_VIEW_LOCAL_BIT) != dependency.dependencyFlags &&
                       pCreateInfo->pSubpasses[dependency.srcSubpass].viewMask > 1) {
                vuid = use_rp2 ? "VUID-VkRenderPassCreateInfo2-pDependencies-03060" : "VUID-VkSubpassDependency-srcSubpass-00872";
                skip |= LogError(vuid, device, dependencies_loc,
                                 "specifies a self-dependency for subpass %" PRIu32
                                 " with a non-zero view mask, but does not "
                                 "specify VK_DEPENDENCY_VIEW_LOCAL_BIT.",
                                 dependency.srcSubpass);
            } else if (HasFramebufferStagePipelineStageFlags(dependency.srcStageMask) &&
                       HasNonFramebufferStagePipelineStageFlags(dependency.dstStageMask)) {
                vuid = use_rp2 ? "VUID-VkSubpassDependency2-srcSubpass-06810" : "VUID-VkSubpassDependency-srcSubpass-06809";
                skip |= LogError(vuid, device, dependencies_loc,
                                 "specifies a self-dependency from stage(s) that access framebuffer space %s to stage(s) that "
                                 "access non-framebuffer space %s.",
                                 string_VkPipelineStageFlags(dependency.srcStageMask).c_str(),
                                 string_VkPipelineStageFlags(dependency.dstStageMask).c_str());
            } else if ((HasNonFramebufferStagePipelineStageFlags(dependency.srcStageMask) == false) &&
                       (HasNonFramebufferStagePipelineStageFlags(dependency.dstStageMask) == false) &&
                       ((dependency.dependencyFlags & VK_DEPENDENCY_BY_REGION_BIT) == 0)) {
                vuid = use_rp2 ? "VUID-VkSubpassDependency2-srcSubpass-02245" : "VUID-VkSubpassDependency-srcSubpass-02243";
                skip |= LogError(vuid, device, dependencies_loc,
                                 "specifies a self-dependency for subpass %" PRIu32
                                 " with both stages including a "
                                 "framebuffer-space stage, but does not specify VK_DEPENDENCY_BY_REGION_BIT in dependencyFlags.",
                                 dependency.srcSubpass);
            }
        } else if ((dependency.srcSubpass < dependency.dstSubpass) &&
                   ((pCreateInfo->pSubpasses[dependency.srcSubpass].flags & VK_SUBPASS_DESCRIPTION_SHADER_RESOLVE_BIT_QCOM) != 0)) {
            vuid = use_rp2 ? "VUID-VkRenderPassCreateInfo2-flags-04909" : "VUID-VkSubpassDescription-flags-03343";
            skip |= LogError(vuid, device, dependencies_loc,
                             "specifies that subpass %" PRIu32
                             " has a dependency on a later subpass"
                             "and includes VK_SUBPASS_DESCRIPTION_SHADER_RESOLVE_BIT_QCOM subpass flags.",
                             dependency.srcSubpass);
        }
    }
    return skip;
}

bool CoreChecks::ValidateCreateRenderPass(const VkRenderPassCreateInfo2 *pCreateInfo, const ErrorObject &error_obj) const {
    bool skip = false;
    const bool use_rp2 = error_obj.location.function != Func::vkCreateRenderPass;
    const char *vuid;

    skip |= ValidateRenderpassAttachmentUsage(pCreateInfo, error_obj);

    skip |= ValidateRenderPassDAG(pCreateInfo, error_obj);

    // Validate multiview correlation and view masks
    bool view_mask_zero = false;
    bool view_mask_non_zero = false;

    for (uint32_t i = 0; i < pCreateInfo->subpassCount; ++i) {
        const Location subpass_loc = error_obj.location.dot(Field::pSubpasses, i);
        const VkSubpassDescription2 &subpass = pCreateInfo->pSubpasses[i];
        if (subpass.viewMask != 0) {
            view_mask_non_zero = true;
            if (!enabled_features.multiview) {
                skip |= LogError("VUID-VkSubpassDescription2-multiview-06558", device, subpass_loc.dot(Field::viewMask),
                                 "is %" PRIu32 ", but multiview feature is not enabled.", subpass.viewMask);
            }
            int highest_view_bit = MostSignificantBit(subpass.viewMask);
            if (highest_view_bit > 0 && static_cast<uint32_t>(highest_view_bit) >= phys_dev_props_core11.maxMultiviewViewCount) {
                skip |= LogError("VUID-VkSubpassDescription2-viewMask-06706", device, subpass_loc,
                                 "highest bit (%" PRIu32
                                 ") is not less than VkPhysicalDeviceMultiviewProperties::maxMultiviewViewCount (%" PRIu32 ").",
                                 highest_view_bit, phys_dev_props_core11.maxMultiviewViewCount);
            }
        } else {
            view_mask_zero = true;
        }

        if ((subpass.flags & VK_SUBPASS_DESCRIPTION_PER_VIEW_POSITION_X_ONLY_BIT_NVX) != 0 &&
            (subpass.flags & VK_SUBPASS_DESCRIPTION_PER_VIEW_ATTRIBUTES_BIT_NVX) == 0) {
            vuid = use_rp2 ? "VUID-VkSubpassDescription2-flags-03076" : "VUID-VkSubpassDescription-flags-00856";
            skip |= LogError(vuid, device, subpass_loc,
                             "The flags parameter of subpass description %" PRIu32
                             " includes "
                             "VK_SUBPASS_DESCRIPTION_PER_VIEW_POSITION_X_ONLY_BIT_NVX but does not also include "
                             "VK_SUBPASS_DESCRIPTION_PER_VIEW_ATTRIBUTES_BIT_NVX.",
                             i);
        }
    }

    if (use_rp2) {
        if (view_mask_non_zero && view_mask_zero) {
            skip |= LogError("VUID-VkRenderPassCreateInfo2-viewMask-03058", device, error_obj.location,
                             "Some view masks are non-zero whilst others are zero.");
        }

        if (view_mask_zero && pCreateInfo->correlatedViewMaskCount != 0) {
            skip |= LogError("VUID-VkRenderPassCreateInfo2-viewMask-03057", device, error_obj.location,
                             "Multiview is not enabled but correlation masks are still provided");
        }
    }
    uint32_t aggregated_cvms = 0;
    for (uint32_t i = 0; i < pCreateInfo->correlatedViewMaskCount; ++i) {
        if (aggregated_cvms & pCreateInfo->pCorrelatedViewMasks[i]) {
            vuid = use_rp2 ? "VUID-VkRenderPassCreateInfo2-pCorrelatedViewMasks-03056"
                           : "VUID-VkRenderPassMultiviewCreateInfo-pCorrelationMasks-00841";
            skip |= LogError(vuid, device, error_obj.location.dot(Field::pCorrelatedViewMasks, i),
                             "contains a previously appearing view bit.");
        }
        aggregated_cvms |= pCreateInfo->pCorrelatedViewMasks[i];
    }

    const auto *fragment_density_map_info = vku::FindStructInPNextChain<VkRenderPassFragmentDensityMapCreateInfoEXT>(pCreateInfo->pNext);
    if (fragment_density_map_info) {
        if (fragment_density_map_info->fragmentDensityMapAttachment.attachment != VK_ATTACHMENT_UNUSED) {
            const Location fragment_loc =
                error_obj.location.pNext(Struct::VkRenderPassFragmentDensityMapCreateInfoEXT, Field::fragmentDensityMapAttachment);
            const Location attchment_loc = fragment_loc.dot(Field::attachment);

            if (fragment_density_map_info->fragmentDensityMapAttachment.attachment >= pCreateInfo->attachmentCount) {
                vuid = use_rp2 ? "VUID-VkRenderPassCreateInfo2-fragmentDensityMapAttachment-06472"
                               : "VUID-VkRenderPassCreateInfo-fragmentDensityMapAttachment-06471";
                skip |= LogError(vuid, device, attchment_loc,
                                 "(%" PRIu32 ") must be less than attachmentCount %" PRIu32 " of for this render pass.",
                                 fragment_density_map_info->fragmentDensityMapAttachment.attachment, pCreateInfo->attachmentCount);
            } else {
                if (!(fragment_density_map_info->fragmentDensityMapAttachment.layout ==
                          VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT ||
                      fragment_density_map_info->fragmentDensityMapAttachment.layout == VK_IMAGE_LAYOUT_GENERAL)) {
                    skip |= LogError("VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02549", device,
                                     attchment_loc,
                                     "(%" PRIu32
                                     ") layout must be equal to "
                                     "VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT or VK_IMAGE_LAYOUT_GENERAL.",
                                     fragment_density_map_info->fragmentDensityMapAttachment.attachment);
                }
                if (!(pCreateInfo->pAttachments[fragment_density_map_info->fragmentDensityMapAttachment.attachment].loadOp ==
                          VK_ATTACHMENT_LOAD_OP_LOAD ||
                      pCreateInfo->pAttachments[fragment_density_map_info->fragmentDensityMapAttachment.attachment].loadOp ==
                          VK_ATTACHMENT_LOAD_OP_DONT_CARE)) {
                    skip |= LogError("VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02550", device,
                                     attchment_loc,
                                     "(%" PRIu32
                                     ") must reference an attachment with a loadOp "
                                     "equal to VK_ATTACHMENT_LOAD_OP_LOAD or VK_ATTACHMENT_LOAD_OP_DONT_CARE.",
                                     fragment_density_map_info->fragmentDensityMapAttachment.attachment);
                }
                if (pCreateInfo->pAttachments[fragment_density_map_info->fragmentDensityMapAttachment.attachment].storeOp !=
                    VK_ATTACHMENT_STORE_OP_DONT_CARE) {
                    skip |= LogError("VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02551", device,
                                     attchment_loc,
                                     "(%" PRIu32
                                     ") must reference an attachment with a storeOp "
                                     "equal to VK_ATTACHMENT_STORE_OP_DONT_CARE.",
                                     fragment_density_map_info->fragmentDensityMapAttachment.attachment);
                }
            }
        }
    }

    auto func_name = use_rp2 ? Func::vkCreateRenderPass2 : Func::vkCreateRenderPass;
    auto structure = use_rp2 ? Struct::VkSubpassDependency2 : Struct::VkSubpassDependency;
    for (uint32_t i = 0; i < pCreateInfo->dependencyCount; ++i) {
        auto const &dependency = pCreateInfo->pDependencies[i];
        Location loc(func_name, structure, Field::pDependencies, i);
        skip |= ValidateSubpassDependency(error_obj, loc, dependency);
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    // Handle extension structs from KHR_multiview and KHR_maintenance2 that can only be validated for RP1 (indices out of bounds)
    const VkRenderPassMultiviewCreateInfo *multiview_info = vku::FindStructInPNextChain<VkRenderPassMultiviewCreateInfo>(pCreateInfo->pNext);
    if (multiview_info) {
        if (multiview_info->subpassCount && multiview_info->subpassCount != pCreateInfo->subpassCount) {
            skip |= LogError("VUID-VkRenderPassCreateInfo-pNext-01928", device, error_obj.location,
                             "Subpass count is %" PRIu32 " but multiview info has a subpass count of %" PRIu32 ".",
                             pCreateInfo->subpassCount, multiview_info->subpassCount);
        } else if (multiview_info->dependencyCount && multiview_info->dependencyCount != pCreateInfo->dependencyCount) {
            skip |= LogError("VUID-VkRenderPassCreateInfo-pNext-01929", device, error_obj.location,
                             "Dependency count is %" PRIu32 " but multiview info has a dependency count of %" PRIu32 ".",
                             pCreateInfo->dependencyCount, multiview_info->dependencyCount);
        }
        bool all_zero = true;
        bool all_not_zero = true;
        for (uint32_t i = 0; i < multiview_info->subpassCount; ++i) {
            all_zero &= multiview_info->pViewMasks[i] == 0;
            all_not_zero &= multiview_info->pViewMasks[i] != 0;
            if (MostSignificantBit(multiview_info->pViewMasks[i]) >=
                static_cast<int32_t>(phys_dev_props_core11.maxMultiviewViewCount)) {
                skip |= LogError("VUID-VkRenderPassMultiviewCreateInfo-pViewMasks-06697", device, error_obj.location,
                                 "Most significant bit in "
                                 "VkRenderPassMultiviewCreateInfo->pViewMask[%" PRIu32 "] (%" PRIu32
                                 ") must be less than maxMultiviewViewCount(%" PRIu32 ").",
                                 i, multiview_info->pViewMasks[i], phys_dev_props_core11.maxMultiviewViewCount);
            }
        }
        if (!all_zero && !all_not_zero) {
            skip |= LogError("VUID-VkRenderPassCreateInfo-pNext-02513", device, error_obj.location,
                             "elements of VkRenderPassMultiviewCreateInfo pViewMasks must all be either 0 or not 0.");
        }
        if (all_zero && multiview_info->correlationMaskCount != 0) {
            skip |= LogError("VUID-VkRenderPassCreateInfo-pNext-02515", device, error_obj.location,
                             "VkRenderPassCreateInfo::correlationMaskCount is %" PRIu32 ", but all elements of pViewMasks are 0.",
                             multiview_info->correlationMaskCount);
        }
        for (uint32_t i = 0; i < pCreateInfo->dependencyCount; ++i) {
            if ((pCreateInfo->pDependencies[i].dependencyFlags & VK_DEPENDENCY_VIEW_LOCAL_BIT) == 0) {
                if (i < multiview_info->dependencyCount && multiview_info->pViewOffsets[i] != 0) {
                    skip |= LogError("VUID-VkRenderPassCreateInfo-pNext-02512", device,
                                     error_obj.location.dot(Field::pDependencies, i).dot(Field::dependencyFlags),
                                     "does not have VK_DEPENDENCY_VIEW_LOCAL_BIT bit set, but the corresponding "
                                     "VkRenderPassMultiviewCreateInfo::pViewOffsets[%" PRIu32 "] is %" PRId32 ".",
                                     i, multiview_info->pViewOffsets[i]);
                }
            } else if (all_zero) {
                skip |= LogError("VUID-VkRenderPassCreateInfo-pNext-02514", device,
                                 error_obj.location.dot(Field::pDependencies, i).dot(Field::dependencyFlags),
                                 "contains VK_DEPENDENCY_VIEW_LOCAL_BIT bit, but all elements of pViewMasks are 0.");
            }
        }
    }
    const VkRenderPassInputAttachmentAspectCreateInfo *input_attachment_aspect_info =
        vku::FindStructInPNextChain<VkRenderPassInputAttachmentAspectCreateInfo>(pCreateInfo->pNext);
    if (input_attachment_aspect_info) {
        for (uint32_t i = 0; i < input_attachment_aspect_info->aspectReferenceCount; ++i) {
            uint32_t subpass = input_attachment_aspect_info->pAspectReferences[i].subpass;
            uint32_t attachment = input_attachment_aspect_info->pAspectReferences[i].inputAttachmentIndex;
            if (subpass >= pCreateInfo->subpassCount) {
                skip |= LogError(
                    "VUID-VkRenderPassCreateInfo-pNext-01926", device,
                    error_obj.location.pNext(Struct::VkRenderPassInputAttachmentAspectCreateInfo, Field::pAspectReferences, i)
                        .dot(Field::subpass),
                    "is %" PRIu32 " which is not less than pCreateInfo->subpassCount (%" PRIu32 ").", subpass,
                    pCreateInfo->subpassCount);
            } else if (pCreateInfo->pSubpasses && attachment >= pCreateInfo->pSubpasses[subpass].inputAttachmentCount) {
                skip |= LogError(
                    "VUID-VkRenderPassCreateInfo-pNext-01927", device,
                    error_obj.location.pNext(Struct::VkRenderPassInputAttachmentAspectCreateInfo, Field::pAspectReferences, i)
                        .dot(Field::inputAttachmentIndex),
                    "is %" PRIu32 " which is not less then pCreateInfo->pSubpasses[%" PRIu32 "].inputAttachmentCount (%" PRIu32
                    ").",
                    attachment, subpass, pCreateInfo->pSubpasses[subpass].inputAttachmentCount);
            }
        }
    }

    if (!skip) {
        safe_VkRenderPassCreateInfo2 create_info_2 = ConvertVkRenderPassCreateInfoToV2KHR(*pCreateInfo);
        skip |= ValidateCreateRenderPass(create_info_2.ptr(), error_obj);
    }

    return skip;
}

// VK_KHR_depth_stencil_resolve was added with a requirement on VK_KHR_create_renderpass2 so this will never be able to use
// VkRenderPassCreateInfo
bool CoreChecks::ValidateDepthStencilResolve(const VkRenderPassCreateInfo2 *pCreateInfo, const ErrorObject &error_obj) const {
    bool skip = false;

    // If the pNext list of VkSubpassDescription2 includes a VkSubpassDescriptionDepthStencilResolve structure,
    // then that structure describes depth/stencil resolve operations for the subpass.
    for (uint32_t i = 0; i < pCreateInfo->subpassCount; i++) {
        const Location subpass_loc = error_obj.location.dot(Field::pSubpasses, i);
        const VkSubpassDescription2 &subpass = pCreateInfo->pSubpasses[i];
        const auto *resolve = vku::FindStructInPNextChain<VkSubpassDescriptionDepthStencilResolve>(subpass.pNext);

        // All of the VUs are wrapped in the wording:
        // "If pDepthStencilResolveAttachment is not NULL"
        if (resolve == nullptr || resolve->pDepthStencilResolveAttachment == nullptr) {
            continue;
        }

        // The spec says
        // "If pDepthStencilAttachment is NULL, or if its attachment index is VK_ATTACHMENT_UNUSED, it indicates that no
        // depth/stencil attachment will be used in the subpass."
        if (subpass.pDepthStencilAttachment == nullptr) {
            continue;
        } else if (subpass.pDepthStencilAttachment->attachment == VK_ATTACHMENT_UNUSED) {
            // while should be ignored, this is an explicit VU and some drivers will crash if this is let through
            skip |=
                LogError("VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03177", device, subpass_loc,
                         "includes a VkSubpassDescriptionDepthStencilResolve "
                         "structure with resolve attachment %" PRIu32 ", but pDepthStencilAttachment=VK_ATTACHMENT_UNUSED.",
                         resolve->pDepthStencilResolveAttachment->attachment);
            continue;
        }

        const uint32_t ds_attachment = subpass.pDepthStencilAttachment->attachment;
        const uint32_t resolve_attachment = resolve->pDepthStencilResolveAttachment->attachment;

        // ValidateAttachmentIndex() should catch if this is invalid, but skip to avoid crashing
        if (ds_attachment >= pCreateInfo->attachmentCount) {
            continue;
        }

        // All VUs in VkSubpassDescriptionDepthStencilResolve are wrapped with language saying it is not unused
        if (resolve_attachment == VK_ATTACHMENT_UNUSED) {
            continue;
        }

        const Location ds_resolve_loc = subpass_loc.pNext(Struct::VkSubpassDescriptionDepthStencilResolve,
                                                          Field::pDepthStencilResolveAttachment, resolve_attachment);
        if (resolve_attachment >= pCreateInfo->attachmentCount) {
            skip |=
                LogError("VUID-VkRenderPassCreateInfo2-pSubpasses-06473", device, ds_resolve_loc,
                         "must be less than attachmentCount %" PRIu32 " of for this render pass.", pCreateInfo->attachmentCount);
            // if the index is invalid need to skip everything else to prevent out of bounds index accesses crashing
            continue;
        }

        const VkFormat ds_attachment_format = pCreateInfo->pAttachments[ds_attachment].format;
        const VkFormat resolve_attachment_format = pCreateInfo->pAttachments[resolve_attachment].format;

        // "depthResolveMode is ignored if the VkFormat of the pDepthStencilResolveAttachment does not have a depth component"
        const bool resolve_has_depth = vkuFormatHasDepth(resolve_attachment_format);
        // "stencilResolveMode is ignored if the VkFormat of the pDepthStencilResolveAttachment does not have a stencil component"
        const bool resolve_has_stencil = vkuFormatHasStencil(resolve_attachment_format);

        if (resolve_has_depth) {
            if (!(resolve->depthResolveMode == VK_RESOLVE_MODE_NONE ||
                  resolve->depthResolveMode & phys_dev_props_core12.supportedDepthResolveModes)) {
                skip |= LogError("VUID-VkSubpassDescriptionDepthStencilResolve-depthResolveMode-03183", device,
                                 subpass_loc.pNext(Struct::VkSubpassDescriptionDepthStencilResolve, Field::depthResolveMode),
                                 "(%s), must be VK_RESOLVE_MODE_NONE or a value from "
                                 "supportedDepthResolveModes (%s).",
                                 string_VkResolveModeFlagBits(resolve->depthResolveMode),
                                 string_VkResolveModeFlags(phys_dev_props_core12.supportedDepthResolveModes).c_str());
            }
        }

        if (resolve_has_stencil) {
            if (!(resolve->stencilResolveMode == VK_RESOLVE_MODE_NONE ||
                  resolve->stencilResolveMode & phys_dev_props_core12.supportedStencilResolveModes)) {
                skip |= LogError("VUID-VkSubpassDescriptionDepthStencilResolve-stencilResolveMode-03184", device,
                                 subpass_loc.pNext(Struct::VkSubpassDescriptionDepthStencilResolve, Field::stencilResolveMode),
                                 "(%s), must be VK_RESOLVE_MODE_NONE or a value from "
                                 "supportedStencilResolveModes (%s).",
                                 string_VkResolveModeFlagBits(resolve->stencilResolveMode),
                                 string_VkResolveModeFlags(phys_dev_props_core12.supportedStencilResolveModes).c_str());
            }
        }

        if (resolve_has_depth && resolve_has_stencil) {
            if (phys_dev_props_core12.independentResolve == VK_FALSE && phys_dev_props_core12.independentResolveNone == VK_FALSE &&
                resolve->depthResolveMode != resolve->stencilResolveMode) {
                skip |= LogError("VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03185", device,
                                 subpass_loc,
                                 "includes a VkSubpassDescriptionDepthStencilResolve "
                                 "structure. The values of depthResolveMode (%s) and stencilResolveMode (%s) must be identical.",
                                 string_VkResolveModeFlagBits(resolve->depthResolveMode),
                                 string_VkResolveModeFlagBits(resolve->stencilResolveMode));
            }

            if (phys_dev_props_core12.independentResolve == VK_FALSE && phys_dev_props_core12.independentResolveNone == VK_TRUE &&
                !(resolve->depthResolveMode == resolve->stencilResolveMode || resolve->depthResolveMode == VK_RESOLVE_MODE_NONE ||
                  resolve->stencilResolveMode == VK_RESOLVE_MODE_NONE)) {
                skip |= LogError("VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03186", device,
                                 subpass_loc,
                                 "includes a VkSubpassDescriptionDepthStencilResolve "
                                 "structure. The values of depthResolveMode (%s) and stencilResolveMode (%s) must be identical, or "
                                 "one of them must be VK_RESOLVE_MODE_NONE.",
                                 string_VkResolveModeFlagBits(resolve->depthResolveMode),
                                 string_VkResolveModeFlagBits(resolve->stencilResolveMode));
            }
        }

        // Same VU, but better error message if one of the resolves are ignored
        if (resolve_has_depth && !resolve_has_stencil && resolve->depthResolveMode == VK_RESOLVE_MODE_NONE) {
            skip |= LogError("VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03178", device,
                             ds_resolve_loc,
                             "is not NULL, but the depth resolve mode is VK_RESOLVE_MODE_NONE (stencil resolve mode is "
                             "ignored due to format not having stencil component).");
        } else if (!resolve_has_depth && resolve_has_stencil && resolve->stencilResolveMode == VK_RESOLVE_MODE_NONE) {
            skip |= LogError("VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03178", device,
                             ds_resolve_loc,
                             "is not NULL, but the stencil resolve mode is VK_RESOLVE_MODE_NONE (depth resolve mode is "
                             "ignored due to format not having depth component).");
        } else if (resolve_has_depth && resolve_has_stencil && resolve->depthResolveMode == VK_RESOLVE_MODE_NONE &&
                   resolve->stencilResolveMode == VK_RESOLVE_MODE_NONE) {
            skip |= LogError("VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03178", device,
                             ds_resolve_loc, "is not NULL, but both depth and stencil resolve modes are VK_RESOLVE_MODE_NONE.");
        }

        const uint32_t resolve_depth_size = vkuFormatDepthSize(resolve_attachment_format);
        const uint32_t resolve_stencil_size = vkuFormatStencilSize(resolve_attachment_format);

        if (resolve_depth_size > 0 &&
            ((vkuFormatDepthSize(ds_attachment_format) != resolve_depth_size) ||
             (vkuFormatDepthNumericalType(ds_attachment_format) != vkuFormatDepthNumericalType(ds_attachment_format)))) {
            skip |= LogError(
                "VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03181", device, ds_resolve_loc,
                "has a depth component (size %" PRIu32
                "). The depth component "
                "of pDepthStencilAttachment must have the same number of bits (currently %" PRIu32 ") and the same numerical type.",
                resolve_depth_size, vkuFormatDepthSize(ds_attachment_format));
        }

        if (resolve_stencil_size > 0 &&
            ((vkuFormatStencilSize(ds_attachment_format) != resolve_stencil_size) ||
             (vkuFormatStencilNumericalType(ds_attachment_format) != vkuFormatStencilNumericalType(resolve_attachment_format)))) {
            skip |= LogError(
                "VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03182", device, ds_resolve_loc,
                "has a stencil component (size %" PRIu32
                "). The stencil component "
                "of pDepthStencilAttachment must have the same number of bits (currently %" PRIu32 ") and the same numerical type.",
                resolve_stencil_size, vkuFormatStencilSize(ds_attachment_format));
        }

        if (pCreateInfo->pAttachments[ds_attachment].samples == VK_SAMPLE_COUNT_1_BIT) {
            skip |=
                LogError("VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03179", device,
                         ds_resolve_loc, "is not NULL, however pDepthStencilAttachment has sample count=VK_SAMPLE_COUNT_1_BIT.");
        }

        if (pCreateInfo->pAttachments[resolve_attachment].samples != VK_SAMPLE_COUNT_1_BIT) {
            skip |= LogError("VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03180", device,
                             ds_resolve_loc, "has sample count of VK_SAMPLE_COUNT_1_BIT.");
        }

        const VkFormatFeatureFlags2 potential_format_features = GetPotentialFormatFeatures(resolve_attachment_format);
        if ((potential_format_features & VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT_KHR) == 0) {
            skip |= LogError("VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-02651", device,
                             ds_resolve_loc,
                             "has a format (%s) whose features do not contain VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT.",
                             string_VkFormat(resolve_attachment_format));
        }

        //  VK_QCOM_render_pass_shader_resolve check of depth/stencil attachmnent
        if ((subpass.flags & VK_SUBPASS_DESCRIPTION_SHADER_RESOLVE_BIT_QCOM) != 0) {
            skip |= LogError("VUID-VkRenderPassCreateInfo2-flags-04908", device, subpass_loc,
                             "enables shader resolve, which requires the depth/stencil resolve attachment"
                             " must be VK_ATTACHMENT_UNUSED, but a reference to attachment %" PRIu32 " was found instead.",
                             resolve_attachment);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;

    skip |= ValidateDepthStencilResolve(pCreateInfo, error_obj);
    skip |= ValidateFragmentShadingRateAttachments(pCreateInfo, error_obj);

    safe_VkRenderPassCreateInfo2 create_info_2(pCreateInfo);
    skip |= ValidateCreateRenderPass(create_info_2.ptr(), error_obj);

    return skip;
}

bool CoreChecks::ValidateFragmentShadingRateAttachments(const VkRenderPassCreateInfo2 *pCreateInfo,
                                                        const ErrorObject &error_obj) const {
    bool skip = false;

    if (!enabled_features.attachmentFragmentShadingRate) {
        return false;
    }

    for (uint32_t attachment_description = 0; attachment_description < pCreateInfo->attachmentCount; ++attachment_description) {
        std::vector<uint32_t> used_as_fragment_shading_rate_attachment;

        // Prepass to find any use as a fragment shading rate attachment structures and validate them independently
        for (uint32_t subpass = 0; subpass < pCreateInfo->subpassCount; ++subpass) {
            const auto *fragment_shading_rate_attachment =
                vku::FindStructInPNextChain<VkFragmentShadingRateAttachmentInfoKHR>(pCreateInfo->pSubpasses[subpass].pNext);

            if (fragment_shading_rate_attachment && fragment_shading_rate_attachment->pFragmentShadingRateAttachment) {
                const Location subpass_loc = error_obj.location.dot(Field::pSubpasses, subpass);
                const Location fragment_loc =
                    subpass_loc.pNext(Struct::VkFragmentShadingRateAttachmentInfoKHR, Field::pFragmentShadingRateAttachment);
                const VkAttachmentReference2 &attachment_reference =
                    *(fragment_shading_rate_attachment->pFragmentShadingRateAttachment);
                if (attachment_reference.attachment == attachment_description) {
                    used_as_fragment_shading_rate_attachment.push_back(subpass);
                }

                if (attachment_reference.attachment != VK_ATTACHMENT_UNUSED) {
                    if ((pCreateInfo->flags & VK_RENDER_PASS_CREATE_TRANSFORM_BIT_QCOM) != 0) {
                        skip |= LogError(
                            "VUID-VkRenderPassCreateInfo2-flags-04521", device, fragment_loc.dot(Field::attachment),
                            "is not VK_ATTACHMENT_UNUSED, but render pass includes VK_RENDER_PASS_CREATE_TRANSFORM_BIT_QCOM");
                    }

                    const VkFormatFeatureFlags2 potential_format_features =
                        GetPotentialFormatFeatures(pCreateInfo->pAttachments[attachment_reference.attachment].format);

                    if (!(potential_format_features & VK_FORMAT_FEATURE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)) {
                        skip |= LogError(
                            "VUID-VkRenderPassCreateInfo2-pAttachments-04586", device,
                            error_obj.location.dot(Field::pAttachments, attachment_reference.attachment).dot(Field::format),
                            "is %s and used in %s as a fragment shading rate attachment, but the format does not support "
                            "VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR.",
                            string_VkFormat(pCreateInfo->pAttachments[attachment_reference.attachment].format),
                            subpass_loc.Fields().c_str());
                    }

                    if (attachment_reference.layout != VK_IMAGE_LAYOUT_GENERAL &&
                        attachment_reference.layout != VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR) {
                        skip |= LogError("VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04524", device,
                                         fragment_loc.dot(Field::layout), "has a layout of %s.",
                                         string_VkImageLayout(attachment_reference.layout));
                    }

                    const VkExtent2D texel_size = fragment_shading_rate_attachment->shadingRateAttachmentTexelSize;
                    const Location texel_loc =
                        subpass_loc.pNext(Struct::VkFragmentShadingRateAttachmentInfoKHR, Field::shadingRateAttachmentTexelSize);
                    if (!IsPowerOfTwo(texel_size.width)) {
                        skip |= LogError("VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04525", device,
                                         texel_loc.dot(Field::width), "(%" PRIu32 ") is a non-power-of-two.", texel_size.width);
                    }
                    if (texel_size.width <
                        phys_dev_ext_props.fragment_shading_rate_props.minFragmentShadingRateAttachmentTexelSize.width) {
                        skip |= LogError(
                            "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04526", device,
                            texel_loc.dot(Field::width), "(%" PRIu32 ") is lower than the advertised minimum width %" PRIu32 ".",
                            texel_size.width,
                            phys_dev_ext_props.fragment_shading_rate_props.minFragmentShadingRateAttachmentTexelSize.width);
                    }
                    if (texel_size.width >
                        phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSize.width) {
                        skip |= LogError(
                            "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04527", device,
                            texel_loc.dot(Field::width), "(%" PRIu32 ") is higher than the advertised maximum width %" PRIu32 ".",
                            texel_size.width,
                            phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSize.width);
                    }
                    if (!IsPowerOfTwo(texel_size.height)) {
                        skip |= LogError("VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04528", device,
                                         texel_loc.dot(Field::height), "(%" PRIu32 ") is a non-power-of-two.", texel_size.height);
                    }
                    if (texel_size.height <
                        phys_dev_ext_props.fragment_shading_rate_props.minFragmentShadingRateAttachmentTexelSize.height) {
                        skip |= LogError(
                            "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04529", device,
                            texel_loc.dot(Field::height), "(%" PRIu32 ") is lower than the advertised minimum height %" PRIu32 ".",
                            texel_size.height,
                            phys_dev_ext_props.fragment_shading_rate_props.minFragmentShadingRateAttachmentTexelSize.height);
                    }
                    if (texel_size.height >
                        phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSize.height) {
                        skip |= LogError(
                            "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04530", device,
                            texel_loc.dot(Field::height), "(%" PRIu32 ") is higher than the advertised maximum height %" PRIu32 ".",
                            texel_size.height,
                            phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSize.height);
                    }
                    uint32_t aspect_ratio = texel_size.width / texel_size.height;
                    uint32_t inverse_aspect_ratio = texel_size.height / texel_size.width;
                    if (aspect_ratio >
                        phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSizeAspectRatio) {
                        skip |= LogError(
                            "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04531", device, texel_loc,
                            "has a texel size of %" PRIu32 " by %" PRIu32 ", which has an aspect ratio %" PRIu32
                            ", which is higher than the advertised maximum aspect ratio %" PRIu32 ".",
                            texel_size.width, texel_size.height, aspect_ratio,
                            phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSizeAspectRatio);
                    }
                    if (inverse_aspect_ratio >
                        phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSizeAspectRatio) {
                        skip |= LogError(
                            "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04532", device, texel_loc,
                            "has a texel size of %" PRIu32 " by %" PRIu32 ", which has an inverse aspect ratio of %" PRIu32
                            ", which is higher than the advertised maximum aspect ratio %" PRIu32 ".",
                            texel_size.width, texel_size.height, inverse_aspect_ratio,
                            phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSizeAspectRatio);
                    }
                }
            }
        }

        // Lambda function turning a vector of integers into a string
        auto vector_to_string = [&](std::vector<uint32_t> vector) {
            std::stringstream ss;
            size_t size = vector.size();
            for (size_t i = 0; i < used_as_fragment_shading_rate_attachment.size(); i++) {
                if (size == 2 && i == 1) {
                    ss << " and ";
                } else if (size > 2 && i == size - 2) {
                    ss << ", and ";
                } else if (i != 0) {
                    ss << ", ";
                }
                ss << vector[i];
            }
            return ss.str();
        };

        // Search for other uses of the same attachment
        if (!used_as_fragment_shading_rate_attachment.empty()) {
            for (uint32_t subpass = 0; subpass < pCreateInfo->subpassCount; ++subpass) {
                const Location subpass_loc = error_obj.location.dot(Field::pSubpasses, subpass);
                const VkSubpassDescription2 &subpass_info = pCreateInfo->pSubpasses[subpass];

                std::string fsr_attachment_subpasses_string = vector_to_string(used_as_fragment_shading_rate_attachment);

                for (uint32_t attachment = 0; attachment < subpass_info.colorAttachmentCount; ++attachment) {
                    if (subpass_info.pColorAttachments[attachment].attachment == attachment_description) {
                        skip |= LogError("VUID-VkRenderPassCreateInfo2-pAttachments-04585", device,
                                         subpass_loc.dot(Field::pColorAttachments, attachment).dot(Field::attachment),
                                         "is also used in pAttachments[%" PRIu32
                                         "] as a fragment shading rate attachment in subpass(es) %s",
                                         attachment_description, fsr_attachment_subpasses_string.c_str());
                    }
                }
                for (uint32_t attachment = 0; attachment < subpass_info.colorAttachmentCount; ++attachment) {
                    if (subpass_info.pResolveAttachments &&
                        subpass_info.pResolveAttachments[attachment].attachment == attachment_description) {
                        skip |= LogError("VUID-VkRenderPassCreateInfo2-pAttachments-04585", device,
                                         subpass_loc.dot(Field::pResolveAttachments, attachment).dot(Field::attachment),
                                         "is also used in pAttachments[%" PRIu32
                                         "] as a fragment shading rate attachment in subpass(es) %s",
                                         attachment_description, fsr_attachment_subpasses_string.c_str());
                    }
                }
                for (uint32_t attachment = 0; attachment < subpass_info.inputAttachmentCount; ++attachment) {
                    if (subpass_info.pInputAttachments[attachment].attachment == attachment_description) {
                        skip |= LogError("VUID-VkRenderPassCreateInfo2-pAttachments-04585", device,
                                         subpass_loc.dot(Field::pInputAttachments, attachment).dot(Field::attachment),
                                         "is also used in pAttachments[%" PRIu32
                                         "] as a fragment shading rate attachment in subpass(es) %s",
                                         attachment_description, fsr_attachment_subpasses_string.c_str());
                    }
                }
                if (subpass_info.pDepthStencilAttachment) {
                    if (subpass_info.pDepthStencilAttachment->attachment == attachment_description) {
                        skip |= LogError("VUID-VkRenderPassCreateInfo2-pAttachments-04585", device,
                                         subpass_loc.dot(Field::pDepthStencilAttachment).dot(Field::attachment),
                                         "is also used in pAttachments[%" PRIu32
                                         "] as a fragment shading rate attachment in subpass(es) %s",
                                         attachment_description, fsr_attachment_subpasses_string.c_str());
                    }
                }
                const auto *depth_stencil_resolve_attachment =
                    vku::FindStructInPNextChain<VkSubpassDescriptionDepthStencilResolve>(subpass_info.pNext);
                if (depth_stencil_resolve_attachment && depth_stencil_resolve_attachment->pDepthStencilResolveAttachment) {
                    if (depth_stencil_resolve_attachment->pDepthStencilResolveAttachment->attachment == attachment_description) {
                        skip |= LogError(
                            "VUID-VkRenderPassCreateInfo2-pAttachments-04585", device,
                            subpass_loc
                                .pNext(Struct::VkSubpassDescriptionDepthStencilResolve, Field::pDepthStencilResolveAttachment)
                                .dot(Field::attachment),
                            "is also used in pAttachments[%" PRIu32 "] as a fragment shading rate attachment in subpass(es) %s",
                            attachment_description, fsr_attachment_subpasses_string.c_str());
                    }
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                     const ErrorObject &error_obj) const {
    return PreCallValidateCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass, error_obj);
}

bool CoreChecks::ValidateRenderingInfoAttachment(const std::shared_ptr<const IMAGE_VIEW_STATE> &image_view,
                                                 const VkRenderingInfo *pRenderingInfo, const LogObjectList &objlist,
                                                 const Location &loc) const {
    bool skip = false;

    // Upcasting to handle overflow
    const bool x_extent_valid =
        static_cast<int64_t>(image_view->image_state->createInfo.extent.width) >=
        static_cast<int64_t>(pRenderingInfo->renderArea.offset.x) + static_cast<int64_t>(pRenderingInfo->renderArea.extent.width);
    const bool y_extent_valid =
        static_cast<int64_t>(image_view->image_state->createInfo.extent.height) >=
        static_cast<int64_t>(pRenderingInfo->renderArea.offset.y) + static_cast<int64_t>(pRenderingInfo->renderArea.extent.height);

    auto device_group_render_pass_begin_info = vku::FindStructInPNextChain<VkDeviceGroupRenderPassBeginInfo>(pRenderingInfo->pNext);
    if (!device_group_render_pass_begin_info || device_group_render_pass_begin_info->deviceRenderAreaCount == 0) {
        if (!x_extent_valid) {
            skip |= LogError("VUID-VkRenderingInfo-pNext-06079", objlist, loc,
                             "width (%" PRIu32 ") is less than pRenderingInfo->renderArea.offset.x (%" PRId32
                             ") + pRenderingInfo->renderArea.extent.width (%" PRIu32 ").",
                             image_view->image_state->createInfo.extent.width, pRenderingInfo->renderArea.offset.x,
                             pRenderingInfo->renderArea.extent.width);
        }

        if (!y_extent_valid) {
            skip |= LogError("VUID-VkRenderingInfo-pNext-06080", objlist, loc,
                             "height (%" PRIu32 ") is less than pRenderingInfo->renderArea.offset.y (%" PRId32
                             ") + pRenderingInfo->renderArea.extent.height (%" PRIu32 ").",
                             image_view->image_state->createInfo.extent.height, pRenderingInfo->renderArea.offset.y,
                             pRenderingInfo->renderArea.extent.height);
        }
    }

    return skip;
}

bool CoreChecks::ValidateRenderingAttachmentInfo(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo,
                                                 const VkRenderingAttachmentInfo &attachment_info,
                                                 const Location &attachment_loc) const {
    bool skip = false;

    // "If imageView is VK_NULL_HANDLE, and resolveMode is not VK_RESOLVE_MODE_EXTERNAL_FORMAT_DOWNSAMPLE_ANDROID, other members of
    // this structure are ignored"
    if (attachment_info.imageView == VK_NULL_HANDLE) {
        return false;
    }
    const auto &image_view_state = *Get<IMAGE_VIEW_STATE>(attachment_info.imageView);

    if (attachment_info.imageLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        skip |= LogError("VUID-VkRenderingAttachmentInfo-imageView-06145", commandBuffer, attachment_loc.dot(Field::imageLayout),
                         "must not be VK_IMAGE_LAYOUT_PRESENT_SRC_KHR");
    }

    if ((!vkuFormatIsSINT(image_view_state.create_info.format) && !vkuFormatIsUINT(image_view_state.create_info.format)) &&
        vkuFormatIsColor(image_view_state.create_info.format) &&
        !(attachment_info.resolveMode == VK_RESOLVE_MODE_NONE || attachment_info.resolveMode == VK_RESOLVE_MODE_AVERAGE_BIT)) {
        const LogObjectList objlist(commandBuffer, attachment_info.imageView);
        skip |= LogError("VUID-VkRenderingAttachmentInfo-imageView-06129", objlist, attachment_loc.dot(Field::resolveMode),
                         "(%s) must be VK_RESOLVE_MODE_NONE or VK_RESOLVE_MODE_AVERAGE_BIT for non-integer formats (%s)",
                         string_VkResolveModeFlags(attachment_info.resolveMode).c_str(),
                         string_VkFormat(image_view_state.create_info.format));
    }

    if ((vkuFormatIsSINT(image_view_state.create_info.format) || vkuFormatIsUINT(image_view_state.create_info.format)) &&
        vkuFormatIsColor(image_view_state.create_info.format) &&
        !(attachment_info.resolveMode == VK_RESOLVE_MODE_NONE || attachment_info.resolveMode == VK_RESOLVE_MODE_SAMPLE_ZERO_BIT)) {
        const LogObjectList objlist(commandBuffer, attachment_info.imageView);
        skip |= LogError("VUID-VkRenderingAttachmentInfo-imageView-06130", objlist, attachment_loc.dot(Field::resolveMode),
                         "(%s) must be VK_RESOLVE_MODE_NONE or VK_RESOLVE_MODE_SAMPLE_ZERO_BIT for integer formats (%s)",
                         string_VkResolveModeFlags(attachment_info.resolveMode).c_str(),
                         string_VkFormat(image_view_state.create_info.format));
    }

    if (attachment_info.imageLayout == VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR) {
        const char *vuid = IsExtEnabled(device_extensions.vk_khr_fragment_shading_rate)
                               ? "VUID-VkRenderingAttachmentInfo-imageView-06143"
                               : "VUID-VkRenderingAttachmentInfo-imageView-06138";
        skip |= LogError(vuid, commandBuffer, attachment_loc.dot(Field::imageLayout),
                         "must not be VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR (or the alias "
                         "VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV)");
    }

    if (attachment_info.imageLayout == VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT) {
        skip |= LogError("VUID-VkRenderingAttachmentInfo-imageView-06140", commandBuffer, attachment_loc.dot(Field::imageLayout),
                         "must not be VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT");
    }

    if (attachment_info.resolveMode != VK_RESOLVE_MODE_NONE && image_view_state.samples == VK_SAMPLE_COUNT_1_BIT) {
        const auto msrtss_info = vku::FindStructInPNextChain<VkMultisampledRenderToSingleSampledInfoEXT>(pRenderingInfo->pNext);
        if (!msrtss_info || !msrtss_info->multisampledRenderToSingleSampledEnable) {
            const LogObjectList objlist(commandBuffer, attachment_info.imageView);
            skip |= LogError("VUID-VkRenderingAttachmentInfo-imageView-06861", objlist, attachment_loc.dot(Field::imageView),
                             "must not have a VK_SAMPLE_COUNT_1_BIT when resolveMode is %s",
                             string_VkResolveModeFlags(attachment_info.resolveMode).c_str());
        }
        if (msrtss_info && msrtss_info->multisampledRenderToSingleSampledEnable &&
            (attachment_info.resolveImageView != VK_NULL_HANDLE)) {
            const LogObjectList objlist(commandBuffer, attachment_info.resolveImageView);
            skip |= LogError(
                "VUID-VkRenderingAttachmentInfo-imageView-06863", objlist, attachment_loc.dot(Field::resolveMode),
                "is %s and VkMultisampledRenderToSingleSampledInfoEXT::multisampledRenderToSingleSampledEnable is VK_TRUE, and "
                "%s.imageView has a sample count of VK_SAMPLE_COUNT_1_BIT, and resolveImageView (%s) is not VK_NULL_HANDLE.",
                string_VkResolveModeFlags(attachment_info.resolveMode).c_str(),
                attachment_loc.dot(Field::resolveMode).Fields().c_str(), FormatHandle(attachment_info.resolveImageView).c_str());
        }
    }

    if (attachment_info.resolveMode != VK_RESOLVE_MODE_NONE && attachment_info.resolveImageView == VK_NULL_HANDLE) {
        const auto msrtss_info = vku::FindStructInPNextChain<VkMultisampledRenderToSingleSampledInfoEXT>(pRenderingInfo->pNext);
        if (!msrtss_info || !msrtss_info->multisampledRenderToSingleSampledEnable) {
            skip |=
                LogError("VUID-VkRenderingAttachmentInfo-imageView-06862", commandBuffer, attachment_loc.dot(Field::resolveMode),
                         "(%s) is not VK_RESOLVE_MODE_NONE, resolveImageView must not be VK_NULL_HANDLE",
                         string_VkResolveModeFlags(attachment_info.resolveMode).c_str());
        }
    }

    auto resolve_view_state = Get<IMAGE_VIEW_STATE>(attachment_info.resolveImageView);
    if (resolve_view_state && (attachment_info.resolveMode != VK_RESOLVE_MODE_NONE) &&
        (resolve_view_state->samples != VK_SAMPLE_COUNT_1_BIT)) {
        const LogObjectList objlist(commandBuffer, attachment_info.resolveImageView);
        skip |= LogError("VUID-VkRenderingAttachmentInfo-imageView-06864", commandBuffer, attachment_loc.dot(Field::resolveMode),
                         "%s but resolveImageView has a sample count of %s",
                         string_VkResolveModeFlags(attachment_info.resolveMode).c_str(),
                         string_VkSampleCountFlagBits(resolve_view_state->samples));
    }

    if (attachment_info.resolveMode != VK_RESOLVE_MODE_NONE) {
        if (attachment_info.resolveImageLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
            skip |= LogError("VUID-VkRenderingAttachmentInfo-imageView-06146", commandBuffer,
                             attachment_loc.dot(Field::resolveImageLayout), "must not be VK_IMAGE_LAYOUT_PRESENT_SRC_KHR");
        }

        if (attachment_info.resolveImageLayout == VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR) {
            const char *vuid = IsExtEnabled(device_extensions.vk_khr_fragment_shading_rate)
                                   ? "VUID-VkRenderingAttachmentInfo-imageView-06144"
                                   : "VUID-VkRenderingAttachmentInfo-imageView-06139";
            skip |= LogError(vuid, commandBuffer, attachment_loc.dot(Field::resolveImageLayout),
                             "must not be VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR "
                             "(or the alias VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV)");
        }

        if (attachment_info.resolveImageLayout == VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT) {
            skip |= LogError("VUID-VkRenderingAttachmentInfo-imageView-06141", commandBuffer,
                             attachment_loc.dot(Field::resolveImageLayout),
                             "must not be VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT");
        }

        if (attachment_info.resolveImageLayout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR) {
            skip |= LogError("VUID-VkRenderingAttachmentInfo-imageView-06142", commandBuffer,
                             attachment_loc.dot(Field::resolveImageLayout), "must not be VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR");
        }

        if (resolve_view_state && (image_view_state.create_info.format != resolve_view_state->create_info.format)) {
            const LogObjectList objlist(commandBuffer, attachment_info.resolveImageView);
            skip |= LogError(
                "VUID-VkRenderingAttachmentInfo-imageView-06865", objlist, attachment_loc.dot(Field::resolveImageView),
                "format (%s) and %s format (%s) are different.", string_VkFormat(resolve_view_state->create_info.format),
                attachment_loc.dot(Field::imageView).Fields().c_str(), string_VkFormat(image_view_state.create_info.format));
        }

        if (IsValueIn(attachment_info.resolveImageLayout,
                      {VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PREINITIALIZED})) {
            skip |= LogError("VUID-VkRenderingAttachmentInfo-imageView-06136", commandBuffer,
                             attachment_loc.dot(Field::resolveImageLayout), "is %s.",
                             string_VkImageLayout(attachment_info.resolveImageLayout));
        }

        if (((attachment_info.resolveImageLayout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL) ||
             (attachment_info.resolveImageLayout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL))) {
            skip |= LogError("VUID-VkRenderingAttachmentInfo-imageView-06137", commandBuffer,
                             attachment_loc.dot(Field::resolveImageLayout), "is %s.",
                             string_VkImageLayout(attachment_info.resolveImageLayout));
        }
    }

    if (IsValueIn(attachment_info.imageLayout,
                  {VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PREINITIALIZED})) {
        skip |= LogError("VUID-VkRenderingAttachmentInfo-imageView-06135", commandBuffer, attachment_loc.dot(Field::imageLayout),
                         "is %s.", string_VkImageLayout(attachment_info.imageLayout));
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo,
                                                  const ErrorObject &error_obj) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state) return false;
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);

    const auto *chained_device_group_struct = vku::FindStructInPNextChain<VkDeviceGroupRenderPassBeginInfo>(pRenderingInfo->pNext);
    const bool non_zero_device_render_area = chained_device_group_struct && chained_device_group_struct->deviceRenderAreaCount != 0;

    if (!enabled_features.dynamicRendering) {
        skip |= LogError("VUID-vkCmdBeginRendering-dynamicRendering-06446", commandBuffer, error_obj.location,
                         "dynamicRendering is not enabled.");
    }

    const Location rendering_info = error_obj.location.dot(Field::pRenderingInfo);
    if (pRenderingInfo->layerCount > phys_dev_props.limits.maxFramebufferLayers) {
        skip |= LogError("VUID-VkRenderingInfo-layerCount-07817", commandBuffer, rendering_info.dot(Field::layerCount),
                         "(%" PRIu32 ") is greater than maxFramebufferLayers (%" PRIu32 ").", pRenderingInfo->layerCount,
                         phys_dev_props.limits.maxFramebufferLayers);
    }

    if ((cb_state->createInfo.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) &&
        ((pRenderingInfo->flags & VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR) != 0) &&
        !enabled_features.nestedCommandBuffer) {
        skip |= LogError("VUID-vkCmdBeginRendering-commandBuffer-06068", commandBuffer, rendering_info.dot(Field::flags),
                         "must not include VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR in a secondary command buffer.");
    }

    const auto *rendering_fragment_shading_rate_attachment_info =
        vku::FindStructInPNextChain<VkRenderingFragmentShadingRateAttachmentInfoKHR>(pRenderingInfo->pNext);
    // Upcasting to handle overflow
    const int64_t x_adjusted_extent =
        static_cast<int64_t>(pRenderingInfo->renderArea.offset.x) + static_cast<int64_t>(pRenderingInfo->renderArea.extent.width);
    const int64_t y_adjusted_extent =
        static_cast<int64_t>(pRenderingInfo->renderArea.offset.y) + static_cast<int64_t>(pRenderingInfo->renderArea.extent.height);
    if (rendering_fragment_shading_rate_attachment_info &&
        (rendering_fragment_shading_rate_attachment_info->imageView != VK_NULL_HANDLE)) {
        auto view_state = Get<IMAGE_VIEW_STATE>(rendering_fragment_shading_rate_attachment_info->imageView);
        const LogObjectList objlist(commandBuffer, view_state->image_view());
        if (pRenderingInfo->viewMask == 0) {
            if (view_state->create_info.subresourceRange.layerCount != 1 &&
                view_state->create_info.subresourceRange.layerCount < pRenderingInfo->layerCount) {
                skip |= LogError("VUID-VkRenderingInfo-imageView-06123", objlist, rendering_info.dot(Field::layerCount),
                                 "is (%" PRIu32
                                 ") but VkRenderingFragmentShadingRateAttachmentInfoKHR::imageView was created with (%" PRIu32 ").",
                                 pRenderingInfo->layerCount, view_state->create_info.subresourceRange.layerCount);
            }
        } else {
            int highest_view_bit = MostSignificantBit(pRenderingInfo->viewMask);
            int32_t layer_count = view_state->normalized_subresource_range.layerCount;
            if (layer_count != 1 && layer_count < highest_view_bit) {
                skip |= LogError("VUID-VkRenderingInfo-imageView-06124", objlist,
                                 rendering_info.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::imageView),
                                 "has a layerCount (%" PRId32
                                 ") but must either is equal to 1 or greater than "
                                 " or equal to the index of the most significant bit in viewMask (%d)",
                                 layer_count, highest_view_bit);
            }
        }

        if ((view_state->inherited_usage & VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR) == 0) {
            skip |= LogError("VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06148", objlist,
                             rendering_info.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::imageView),
                             "was not created with VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR.");
        }

        if (!non_zero_device_render_area) {
            if (static_cast<int64_t>(view_state->image_state->createInfo.extent.width) <
                vvl::GetQuotientCeil(
                    x_adjusted_extent,
                    static_cast<int64_t>(rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width))) {
                skip |= LogError("VUID-VkRenderingInfo-pNext-06119", objlist,
                                 rendering_info.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::imageView),
                                 "width (%" PRIu32 ") must not be less than (pRenderingInfo->renderArea.offset.x (%" PRId32
                                 ") + pRenderingInfo->renderArea.extent.width (%" PRIu32
                                 ") ) / shadingRateAttachmentTexelSize.width (%" PRIu32 ").",
                                 view_state->image_state->createInfo.extent.width, pRenderingInfo->renderArea.offset.x,
                                 pRenderingInfo->renderArea.extent.width,
                                 rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width);
            }

            if (static_cast<int64_t>(view_state->image_state->createInfo.extent.height) <
                vvl::GetQuotientCeil(
                    y_adjusted_extent,
                    static_cast<int64_t>(rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height))) {
                skip |= LogError("VUID-VkRenderingInfo-pNext-06121", objlist,
                                 rendering_info.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::imageView),
                                 "height (%" PRIu32 ") must not be less than (pRenderingInfo->renderArea.offset.y (%" PRId32
                                 ") + pRenderingInfo->renderArea.extent.height (%" PRIu32
                                 ") ) / shadingRateAttachmentTexelSize.height (%" PRIu32 ").",
                                 view_state->image_state->createInfo.extent.height, pRenderingInfo->renderArea.offset.y,
                                 pRenderingInfo->renderArea.extent.height,
                                 rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height);
            }
        } else {
            if (chained_device_group_struct) {
                for (uint32_t deviceRenderAreaIndex = 0; deviceRenderAreaIndex < chained_device_group_struct->deviceRenderAreaCount;
                     ++deviceRenderAreaIndex) {
                    const int32_t offset_x = chained_device_group_struct->pDeviceRenderAreas[deviceRenderAreaIndex].offset.x;
                    const uint32_t width = chained_device_group_struct->pDeviceRenderAreas[deviceRenderAreaIndex].extent.width;
                    const int32_t offset_y = chained_device_group_struct->pDeviceRenderAreas[deviceRenderAreaIndex].offset.y;
                    const uint32_t height = chained_device_group_struct->pDeviceRenderAreas[deviceRenderAreaIndex].extent.height;

                    IMAGE_STATE *image_state = view_state->image_state.get();
                    if (image_state->createInfo.extent.width <
                        vvl::GetQuotientCeil(
                            offset_x + width,
                            rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width)) {
                        skip |= LogError(
                            "VUID-VkRenderingInfo-pNext-06120", objlist,
                            rendering_info.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::imageView),
                            "width (%" PRIu32
                            ") must not be less than (VkDeviceGroupRenderPassBeginInfo::pDeviceRenderAreas[%" PRIu32
                            "].offset.x (%" PRId32 ") + VkDeviceGroupRenderPassBeginInfo::pDeviceRenderAreas[%" PRIu32
                            "].extent.width (%" PRIu32 ") ) / shadingRateAttachmentTexelSize.width (%" PRIu32 ").",
                            image_state->createInfo.extent.width, deviceRenderAreaIndex, offset_x, deviceRenderAreaIndex, width,
                            rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width);
                    }
                    if (image_state->createInfo.extent.height <
                        vvl::GetQuotientCeil(
                            offset_y + height,
                            rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height)) {
                        skip |= LogError(
                            "VUID-VkRenderingInfo-pNext-06122", objlist,
                            rendering_info.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::imageView),
                            "height (%" PRIu32
                            ") must not be less than (VkDeviceGroupRenderPassBeginInfo::pDeviceRenderAreas[%" PRIu32
                            "].offset.y (%" PRId32 ") + VkDeviceGroupRenderPassBeginInfo::pDeviceRenderAreas[%" PRIu32
                            "].extent.height (%" PRIu32
                            ") ) / shadingRateAttachmentTexelSize.height "
                            "(%" PRIu32 ").",
                            image_state->createInfo.extent.height, deviceRenderAreaIndex, offset_y, deviceRenderAreaIndex, height,
                            rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height);
                    }
                }
            }
        }
    }

    if (!(IsExtEnabled(device_extensions.vk_amd_mixed_attachment_samples) ||
          IsExtEnabled(device_extensions.vk_nv_framebuffer_mixed_samples) ||
          (enabled_features.multisampledRenderToSingleSampled))) {
        uint32_t first_sample_count_attachment = VK_ATTACHMENT_UNUSED;
        for (uint32_t j = 0; j < pRenderingInfo->colorAttachmentCount; ++j) {
            if (pRenderingInfo->pColorAttachments[j].imageView != VK_NULL_HANDLE) {
                const auto image_view = Get<IMAGE_VIEW_STATE>(pRenderingInfo->pColorAttachments[j].imageView);
                first_sample_count_attachment = (first_sample_count_attachment == VK_ATTACHMENT_UNUSED)
                                                    ? static_cast<uint32_t>(image_view->samples)
                                                    : first_sample_count_attachment;
                const LogObjectList objlist(commandBuffer, pRenderingInfo->pColorAttachments[j].imageView);
                if (first_sample_count_attachment != image_view->samples) {
                    skip |= LogError("VUID-VkRenderingInfo-multisampledRenderToSingleSampled-06857", objlist,
                                     rendering_info.dot(Field::pColorAttachments, j).dot(Field::imageView),
                                     "has sample count %s, whereas first used color "
                                     "attachment ref has sample count %" PRIu32 ".",
                                     string_VkSampleCountFlagBits(image_view->samples), first_sample_count_attachment);
                }
                skip |= ValidateRenderingInfoAttachment(image_view, pRenderingInfo, objlist,
                                                        rendering_info.dot(Field::pColorAttachments, j).dot(Field::imageView));
            }
        }
        if (pRenderingInfo->pDepthAttachment && pRenderingInfo->pDepthAttachment->imageView != VK_NULL_HANDLE) {
            const auto image_view = Get<IMAGE_VIEW_STATE>(pRenderingInfo->pDepthAttachment->imageView);
            first_sample_count_attachment = (first_sample_count_attachment == VK_ATTACHMENT_UNUSED)
                                                ? static_cast<uint32_t>(image_view->samples)
                                                : first_sample_count_attachment;
            const LogObjectList objlist(commandBuffer, pRenderingInfo->pDepthAttachment->imageView);
            if (first_sample_count_attachment != image_view->samples) {
                skip |= LogError("VUID-VkRenderingInfo-multisampledRenderToSingleSampled-06857", objlist,
                                 rendering_info.dot(Field::pDepthAttachment).dot(Field::imageView),
                                 "has sample count %s, whereas first used color attachment ref has sample count %" PRIu32 ".",
                                 string_VkSampleCountFlagBits(image_view->samples), (first_sample_count_attachment));
            }
            skip |= ValidateRenderingInfoAttachment(image_view, pRenderingInfo, objlist,
                                                    rendering_info.dot(Field::pDepthAttachment).dot(Field::imageView));
        }
        if (pRenderingInfo->pStencilAttachment && pRenderingInfo->pStencilAttachment->imageView != VK_NULL_HANDLE) {
            const auto image_view = Get<IMAGE_VIEW_STATE>(pRenderingInfo->pStencilAttachment->imageView);
            first_sample_count_attachment = (first_sample_count_attachment == VK_ATTACHMENT_UNUSED)
                                                ? static_cast<uint32_t>(image_view->samples)
                                                : first_sample_count_attachment;
            const LogObjectList objlist(commandBuffer, pRenderingInfo->pStencilAttachment->imageView);
            if (first_sample_count_attachment != image_view->samples) {
                skip |= LogError("VUID-VkRenderingInfo-multisampledRenderToSingleSampled-06857", objlist,
                                 rendering_info.dot(Field::pStencilAttachment).dot(Field::imageView),
                                 "has sample count %s, whereas first used color attachment ref has sample count %" PRIu32 ".",
                                 string_VkSampleCountFlagBits(image_view->samples), (first_sample_count_attachment));
            }
            skip |= ValidateRenderingInfoAttachment(image_view, pRenderingInfo, objlist,
                                                    rendering_info.dot(Field::pStencilAttachment).dot(Field::imageView));
        }
    }

    const auto *fragment_density_map_attachment_info =
        vku::FindStructInPNextChain<VkRenderingFragmentDensityMapAttachmentInfoEXT>(pRenderingInfo->pNext);
    if (fragment_density_map_attachment_info) {
        if (!enabled_features.fragmentDensityMapNonSubsampledImages) {
            for (uint32_t j = 0; j < pRenderingInfo->colorAttachmentCount; ++j) {
                if (pRenderingInfo->pColorAttachments[j].imageView != VK_NULL_HANDLE) {
                    auto image_view_state = Get<IMAGE_VIEW_STATE>(pRenderingInfo->pColorAttachments[j].imageView);
                    if (!(image_view_state->image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT)) {
                        const LogObjectList objlist(commandBuffer, pRenderingInfo->pColorAttachments[j].imageView);
                        skip |= LogError("VUID-VkRenderingInfo-imageView-06107", objlist,
                                         rendering_info.dot(Field::pColorAttachments, j).dot(Field::imageView),
                                         "must be created with VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT.");
                    }
                }
            }

            if (pRenderingInfo->pDepthAttachment && (pRenderingInfo->pDepthAttachment->imageView != VK_NULL_HANDLE)) {
                auto depth_view_state = Get<IMAGE_VIEW_STATE>(pRenderingInfo->pDepthAttachment->imageView);
                if (!(depth_view_state->image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT)) {
                    const LogObjectList objlist(commandBuffer, pRenderingInfo->pStencilAttachment->imageView);
                    skip |= LogError("VUID-VkRenderingInfo-imageView-06107", objlist,
                                     rendering_info.dot(Field::pDepthAttachment).dot(Field::imageView),
                                     "must be created with VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT.");
                }
            }

            if (pRenderingInfo->pStencilAttachment && (pRenderingInfo->pStencilAttachment->imageView != VK_NULL_HANDLE)) {
                auto stencil_view_state = Get<IMAGE_VIEW_STATE>(pRenderingInfo->pStencilAttachment->imageView);
                if (!(stencil_view_state->image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT)) {
                    const LogObjectList objlist(commandBuffer, pRenderingInfo->pStencilAttachment->imageView);
                    skip |= LogError("VUID-VkRenderingInfo-imageView-06107", objlist,
                                     rendering_info.dot(Field::pStencilAttachment).dot(Field::imageView),
                                     "must be created with VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT.");
                }
            }
        }

        if (fragment_density_map_attachment_info->imageView != VK_NULL_HANDLE) {
            const Location view_loc =
                rendering_info.pNext(Struct::VkRenderingFragmentDensityMapAttachmentInfoEXT, Field::imageView);
            auto fragment_density_map_view_state = Get<IMAGE_VIEW_STATE>(fragment_density_map_attachment_info->imageView);
            if ((fragment_density_map_view_state->inherited_usage & VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT) == 0) {
                const LogObjectList objlist(commandBuffer, fragment_density_map_attachment_info->imageView);
                skip |= LogError("VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-imageView-06158", objlist, view_loc,
                                 "usage (%s) does not include VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT.",
                                 string_VkImageUsageFlags(fragment_density_map_view_state->inherited_usage).c_str());
            }
            if ((fragment_density_map_view_state->image_state->createInfo.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) > 0) {
                const LogObjectList objlist(commandBuffer, fragment_density_map_attachment_info->imageView,
                                            fragment_density_map_view_state->image_state->image());
                skip |= LogError("VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-imageView-06159", objlist, view_loc,
                                 "internal image was created with flags %s.",
                                 string_VkImageCreateFlags(fragment_density_map_view_state->image_state->createInfo.flags).c_str());
            }
            int32_t layer_count = static_cast<int32_t>(fragment_density_map_view_state->normalized_subresource_range.layerCount);
            if (layer_count != 1) {
                const LogObjectList objlist(commandBuffer, fragment_density_map_attachment_info->imageView);
                skip |= LogError("VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-apiVersion-07908", objlist, view_loc,
                                 "must have a layer count (%" PRId32 ") equal to 1.", layer_count);
            }
            if ((pRenderingInfo->viewMask == 0) && (layer_count != 1)) {
                const LogObjectList objlist(commandBuffer, fragment_density_map_attachment_info->imageView);
                skip |= LogError("VUID-VkRenderingInfo-imageView-06109", objlist, view_loc,
                                 "must have a layer count (%" PRId32 ") equal to 1 when viewMask is equal to 0", layer_count);
            }

            if ((pRenderingInfo->viewMask != 0) && (layer_count < MostSignificantBit(pRenderingInfo->viewMask))) {
                const LogObjectList objlist(commandBuffer, fragment_density_map_attachment_info->imageView);
                skip |= LogError("VUID-VkRenderingInfo-imageView-06108", objlist, view_loc,
                                 "must have a layer count (%" PRId32
                                 ") greater than or equal to the most significant bit in viewMask (%" PRIu32 ")",
                                 layer_count, pRenderingInfo->viewMask);
            }
        }
    }

    if ((enabled_features.multiview == VK_FALSE) && (pRenderingInfo->viewMask != 0)) {
        skip |= LogError("VUID-VkRenderingInfo-multiview-06127", commandBuffer, rendering_info.dot(Field::viewMask),
                         "%" PRId32 " but the multiview feature is not enabled.", pRenderingInfo->viewMask);
    }

    if (!non_zero_device_render_area) {
        // if the renderArea was set with garbage, only want to report 1 error
        if (pRenderingInfo->renderArea.offset.x < 0) {
            skip |= LogError("VUID-VkRenderingInfo-pNext-06077", commandBuffer,
                             rendering_info.dot(Field::renderArea).dot(Field::offset).dot(Field::x),
                             "is %" PRId32 " (offset can't be negative).", pRenderingInfo->renderArea.offset.x);
        } else if (pRenderingInfo->renderArea.offset.y < 0) {
            skip |= LogError("VUID-VkRenderingInfo-pNext-06078", commandBuffer,
                             rendering_info.dot(Field::renderArea).dot(Field::offset).dot(Field::y),
                             "is %" PRId32 " (offset can't be negative).", pRenderingInfo->renderArea.offset.y);
        } else if (pRenderingInfo->renderArea.extent.width == 0) {
            skip |= LogError("VUID-VkRenderingInfo-None-08994", commandBuffer,
                             rendering_info.dot(Field::renderArea).dot(Field::extent).dot(Field::width), "is zero.");
        } else if (pRenderingInfo->renderArea.extent.height == 0) {
            skip |= LogError("VUID-VkRenderingInfo-None-08995", commandBuffer,
                             rendering_info.dot(Field::renderArea).dot(Field::extent).dot(Field::height), "is zero.");
        }
        if (x_adjusted_extent > phys_dev_props.limits.maxFramebufferWidth) {
            skip |= LogError("VUID-VkRenderingInfo-pNext-07815", commandBuffer, rendering_info.dot(Field::renderArea),
                             "offset.x (%" PRId32 ") + extent.width (%" PRIu32
                             ") is not less than or equal to maxFramebufferWidth (%" PRIu32 ").",
                             pRenderingInfo->renderArea.offset.x, pRenderingInfo->renderArea.extent.width,
                             phys_dev_props.limits.maxFramebufferWidth);
        }
        if (y_adjusted_extent > phys_dev_props.limits.maxFramebufferHeight) {
            skip |= LogError("VUID-VkRenderingInfo-pNext-07816", commandBuffer, rendering_info.dot(Field::renderArea),
                             "offset.y (%" PRId32 ") + extent.height (%" PRIu32
                             ") is not less than or equal to maxFramebufferHeight (%" PRIu32 ").",
                             pRenderingInfo->renderArea.offset.y, pRenderingInfo->renderArea.extent.height,
                             phys_dev_props.limits.maxFramebufferHeight);
        }

        if (fragment_density_map_attachment_info && fragment_density_map_attachment_info->imageView != VK_NULL_HANDLE) {
            auto view_state = Get<IMAGE_VIEW_STATE>(fragment_density_map_attachment_info->imageView);
            IMAGE_STATE *image_state = view_state->image_state.get();
            if (image_state->createInfo.extent.width <
                vvl::GetQuotientCeil(
                    x_adjusted_extent,
                    static_cast<int64_t>(phys_dev_ext_props.fragment_density_map_props.maxFragmentDensityTexelSize.width))) {
                const LogObjectList objlist(commandBuffer, fragment_density_map_attachment_info->imageView, image_state->image());
                skip |= LogError(
                    "VUID-VkRenderingInfo-pNext-06112", objlist,
                    rendering_info.pNext(Struct::VkRenderingFragmentDensityMapAttachmentInfoEXT, Field::imageView),
                    "width  (%" PRIu32 ") must not be less than (pRenderingInfo->renderArea.offset.x (%" PRId32
                    ") + pRenderingInfo->renderArea.extent.width (%" PRIu32
                    ") ) / VkPhysicalDeviceFragmentDensityMapPropertiesEXT::maxFragmentDensityTexelSize.width (%" PRIu32 ").",
                    image_state->createInfo.extent.width, pRenderingInfo->renderArea.offset.x,
                    pRenderingInfo->renderArea.extent.width,
                    phys_dev_ext_props.fragment_density_map_props.maxFragmentDensityTexelSize.width);
            }
            if (image_state->createInfo.extent.height <
                vvl::GetQuotientCeil(
                    y_adjusted_extent,
                    static_cast<int64_t>(phys_dev_ext_props.fragment_density_map_props.maxFragmentDensityTexelSize.height))) {
                const LogObjectList objlist(commandBuffer, fragment_density_map_attachment_info->imageView, image_state->image());
                skip |= LogError(
                    "VUID-VkRenderingInfo-pNext-06114", objlist,
                    rendering_info.pNext(Struct::VkRenderingFragmentDensityMapAttachmentInfoEXT, Field::imageView),
                    "height (%" PRIu32 ") must not be less than (pRenderingInfo->renderArea.offset.y (%" PRId32
                    ") + pRenderingInfo->renderArea.extent.height (%" PRIu32
                    ") ) / VkPhysicalDeviceFragmentDensityMapPropertiesEXT::maxFragmentDensityTexelSize.height (%" PRIu32 ").",
                    image_state->createInfo.extent.height, pRenderingInfo->renderArea.offset.y,
                    pRenderingInfo->renderArea.extent.height,
                    phys_dev_ext_props.fragment_density_map_props.maxFragmentDensityTexelSize.height);
            }
        }
    }

    if (chained_device_group_struct) {
        for (uint32_t deviceRenderAreaIndex = 0; deviceRenderAreaIndex < chained_device_group_struct->deviceRenderAreaCount;
             ++deviceRenderAreaIndex) {
            const Location group_loc =
                rendering_info.pNext(Struct::VkDeviceGroupRenderPassBeginInfo, Field::pDeviceRenderAreas, deviceRenderAreaIndex);
            const VkRect2D render_area = chained_device_group_struct->pDeviceRenderAreas[deviceRenderAreaIndex];
            const int32_t offset_x = render_area.offset.x;
            const uint32_t width = render_area.extent.width;
            const int32_t offset_y = render_area.offset.y;
            const uint32_t height = render_area.extent.height;

            if (offset_x < 0) {
                skip |=
                    LogError("VUID-VkDeviceGroupRenderPassBeginInfo-offset-06166", commandBuffer,
                             group_loc.dot(Field::offset).dot(Field::x), "is %" PRId32 " (offset can't be negative).", offset_x);
            } else if (offset_y < 0) {
                skip |=
                    LogError("VUID-VkDeviceGroupRenderPassBeginInfo-offset-06167", commandBuffer,
                             group_loc.dot(Field::offset).dot(Field::y), "is %" PRId32 " (offset can't be negative).", offset_y);
            } else if (width == 0) {
                skip |= LogError("VUID-VkDeviceGroupRenderPassBeginInfo-extent-08998", commandBuffer,
                                 group_loc.dot(Field::extent).dot(Field::width), "is zero.");
            } else if (height == 0) {
                skip |= LogError("VUID-VkDeviceGroupRenderPassBeginInfo-extent-08999", commandBuffer,
                                 group_loc.dot(Field::extent).dot(Field::height), "is zero.");
            }

            if ((offset_x + width) > phys_dev_props.limits.maxFramebufferWidth) {
                skip |= LogError("VUID-VkDeviceGroupRenderPassBeginInfo-offset-06168", commandBuffer, group_loc,
                                 "sum of offset.x (%" PRId32 ") and extent.width (%" PRIu32
                                 ") is greater than maxFramebufferWidth (%" PRIu32 ").",
                                 offset_x, width, phys_dev_props.limits.maxFramebufferWidth);
            }
            if ((offset_y + height) > phys_dev_props.limits.maxFramebufferHeight) {
                skip |= LogError("VUID-VkDeviceGroupRenderPassBeginInfo-offset-06169", commandBuffer, group_loc,
                                 "sum of offset.y (%" PRId32 ") and extent.height (%" PRIu32
                                 ") is greater than maxFramebufferHeight (%" PRIu32 ").",
                                 offset_y, height, phys_dev_props.limits.maxFramebufferHeight);
            }

            for (uint32_t j = 0; j < pRenderingInfo->colorAttachmentCount; ++j) {
                if (pRenderingInfo->pColorAttachments[j].imageView != VK_NULL_HANDLE) {
                    auto image_view_state = Get<IMAGE_VIEW_STATE>(pRenderingInfo->pColorAttachments[j].imageView);
                    IMAGE_STATE *image_state = image_view_state->image_state.get();
                    if (image_state->createInfo.extent.width < offset_x + width) {
                        const LogObjectList objlist(commandBuffer, image_view_state->image_view(), image_state->image());
                        skip |= LogError("VUID-VkRenderingInfo-pNext-06083", objlist,
                                         rendering_info.dot(Field::pColorAttachments, j).dot(Field::imageView),
                                         "width (%" PRIu32
                                         ") must be greater than or equal to"
                                         "renderArea.offset.x (%" PRId32 ") + renderArea.extent.width (%" PRIu32 ").",
                                         image_state->createInfo.extent.width, offset_x, width);
                    }
                    if (image_state->createInfo.extent.height < offset_y + height) {
                        const LogObjectList objlist(commandBuffer, image_view_state->image_view(), image_state->image());
                        skip |= LogError("VUID-VkRenderingInfo-pNext-06084", objlist,
                                         rendering_info.dot(Field::pColorAttachments, j).dot(Field::imageView),
                                         "height (%" PRIu32
                                         ") must be greater than or equal to"
                                         "renderArea.offset.y (%" PRId32 ") + renderArea.extent.height (%" PRIu32 ").",
                                         image_state->createInfo.extent.height, offset_y, height);
                    }
                }
            }

            if (pRenderingInfo->pDepthAttachment != VK_NULL_HANDLE) {
                auto depth_view_state = Get<IMAGE_VIEW_STATE>(pRenderingInfo->pDepthAttachment->imageView);
                IMAGE_STATE *image_state = depth_view_state->image_state.get();
                if (image_state->createInfo.extent.width < offset_x + width) {
                    const LogObjectList objlist(commandBuffer, depth_view_state->image_view(), image_state->image());
                    skip |= LogError("VUID-VkRenderingInfo-pNext-06083", objlist,
                                     rendering_info.dot(Field::pDepthAttachment).dot(Field::imageView),
                                     "width (%" PRIu32
                                     ") must be greater than or equal to"
                                     "renderArea.offset.x (%" PRId32 ") + renderArea.extent.width (%" PRIu32 ").",
                                     image_state->createInfo.extent.width, offset_x, width);
                }
                if (image_state->createInfo.extent.height < offset_y + height) {
                    const LogObjectList objlist(commandBuffer, depth_view_state->image_view(), image_state->image());
                    skip |= LogError("VUID-VkRenderingInfo-pNext-06084", objlist,
                                     rendering_info.dot(Field::pDepthAttachment).dot(Field::imageView),
                                     "height (%" PRIu32
                                     ") must be greater than or equal to"
                                     "renderArea.offset.y (%" PRId32 ") + renderArea.extent.height (%" PRIu32 ").",
                                     image_state->createInfo.extent.height, offset_y, height);
                }
            }

            if (pRenderingInfo->pStencilAttachment != VK_NULL_HANDLE) {
                auto stencil_view_state = Get<IMAGE_VIEW_STATE>(pRenderingInfo->pStencilAttachment->imageView);
                IMAGE_STATE *image_state = stencil_view_state->image_state.get();
                if (image_state->createInfo.extent.width < offset_x + width) {
                    const LogObjectList objlist(commandBuffer, stencil_view_state->image_view(), image_state->image());
                    skip |= LogError("VUID-VkRenderingInfo-pNext-06083", objlist,
                                     rendering_info.dot(Field::pStencilAttachment).dot(Field::imageView),
                                     "width (%" PRIu32
                                     ") must be greater than or equal to"
                                     "renderArea.offset.x (%" PRId32 ") + renderArea.extent.width (%" PRIu32 ").",
                                     image_state->createInfo.extent.width, offset_x, width);
                }
                if (image_state->createInfo.extent.height < offset_y + height) {
                    const LogObjectList objlist(commandBuffer, stencil_view_state->image_view(), image_state->image());
                    skip |= LogError("VUID-VkRenderingInfo-pNext-06084", objlist,
                                     rendering_info.dot(Field::pStencilAttachment).dot(Field::imageView),
                                     "height (%" PRIu32
                                     ") must be greater than or equal to"
                                     "renderArea.offset.y (%" PRId32 ") + renderArea.extent.height(%" PRIu32 ").",
                                     image_state->createInfo.extent.height, offset_y, height);
                }
            }

            if (fragment_density_map_attachment_info && fragment_density_map_attachment_info->imageView != VK_NULL_HANDLE) {
                auto view_state = Get<IMAGE_VIEW_STATE>(fragment_density_map_attachment_info->imageView);
                IMAGE_STATE *image_state = view_state->image_state.get();
                if (image_state->createInfo.extent.width <
                    vvl::GetQuotientCeil(offset_x + width,
                                                phys_dev_ext_props.fragment_density_map_props.maxFragmentDensityTexelSize.width)) {
                    const LogObjectList objlist(commandBuffer, view_state->image_view(), image_state->image());
                    skip |= LogError(
                        "VUID-VkRenderingInfo-pNext-06113", objlist,
                        rendering_info.pNext(Struct::VkRenderingFragmentDensityMapAttachmentInfoEXT, Field::imageView),
                        "width (%" PRIu32 ") must not be less than (VkDeviceGroupRenderPassBeginInfo::pDeviceRenderAreas[%" PRIu32
                        "].offset.x (%" PRId32 ") + VkDeviceGroupRenderPassBeginInfo::pDeviceRenderAreas[%" PRIu32
                        "].extent.width (%" PRIu32
                        ") ) / VkPhysicalDeviceFragmentDensityMapPropertiesEXT::maxFragmentDensityTexelSize.width (%" PRIu32 ").",
                        image_state->createInfo.extent.width, deviceRenderAreaIndex, offset_x, deviceRenderAreaIndex, width,
                        phys_dev_ext_props.fragment_density_map_props.maxFragmentDensityTexelSize.width);
                }
                if (image_state->createInfo.extent.height <
                    vvl::GetQuotientCeil(offset_y + height,
                                                phys_dev_ext_props.fragment_density_map_props.maxFragmentDensityTexelSize.height)) {
                    const LogObjectList objlist(commandBuffer, view_state->image_view(), image_state->image());
                    skip |= LogError(
                        "VUID-VkRenderingInfo-pNext-06115", objlist,
                        rendering_info.pNext(Struct::VkRenderingFragmentDensityMapAttachmentInfoEXT, Field::imageView),
                        "height (%" PRIu32 ") must not be less than (VkDeviceGroupRenderPassBeginInfo::pDeviceRenderAreas[%" PRIu32
                        "].offset.y (%" PRId32 ") + VkDeviceGroupRenderPassBeginInfo::pDeviceRenderAreas[%" PRIu32
                        "].extent.height (%" PRIu32
                        ") ) / VkPhysicalDeviceFragmentDensityMapPropertiesEXT::maxFragmentDensityTexelSize.height (%" PRIu32 ").",
                        image_state->createInfo.extent.height, deviceRenderAreaIndex, offset_y, deviceRenderAreaIndex, height,
                        phys_dev_ext_props.fragment_density_map_props.maxFragmentDensityTexelSize.height);
                }
            }
        }
    }

    if (pRenderingInfo->pDepthAttachment != nullptr && pRenderingInfo->pStencilAttachment != nullptr) {
        if (pRenderingInfo->pDepthAttachment->imageView != VK_NULL_HANDLE &&
            pRenderingInfo->pStencilAttachment->imageView != VK_NULL_HANDLE) {
            if (pRenderingInfo->pDepthAttachment->imageView != pRenderingInfo->pStencilAttachment->imageView) {
                const LogObjectList objlist(commandBuffer, pRenderingInfo->pDepthAttachment->imageView,
                                            pRenderingInfo->pStencilAttachment->imageView);
                skip |= LogError("VUID-VkRenderingInfo-pDepthAttachment-06085", objlist, rendering_info,
                                 "imageView of pDepthAttachment and pStencilAttachment must be the same.");
            }

            if ((phys_dev_props_core12.independentResolveNone == VK_FALSE) &&
                (pRenderingInfo->pDepthAttachment->resolveMode != pRenderingInfo->pStencilAttachment->resolveMode)) {
                skip |= LogError(
                    "VUID-VkRenderingInfo-pDepthAttachment-06104", commandBuffer, rendering_info,
                    "values of pDepthAttachment->resolveMode (%s) and pStencilAttachment->resolveMode (%s) must be identical.",
                    string_VkResolveModeFlagBits(pRenderingInfo->pDepthAttachment->resolveMode),
                    string_VkResolveModeFlagBits(pRenderingInfo->pStencilAttachment->resolveMode));
            }

            if ((phys_dev_props_core12.independentResolve == VK_FALSE) &&
                (pRenderingInfo->pDepthAttachment->resolveMode != VK_RESOLVE_MODE_NONE) &&
                (pRenderingInfo->pStencilAttachment->resolveMode != VK_RESOLVE_MODE_NONE) &&
                (pRenderingInfo->pStencilAttachment->resolveMode != pRenderingInfo->pDepthAttachment->resolveMode)) {
                skip |= LogError("VUID-VkRenderingInfo-pDepthAttachment-06105", commandBuffer, rendering_info,
                                 "values of pDepthAttachment->resolveMode (%s) and pStencilAttachment->resolveMode (%s) must "
                                 "be identical, or one of them must be VK_RESOLVE_MODE_NONE.",
                                 string_VkResolveModeFlagBits(pRenderingInfo->pDepthAttachment->resolveMode),
                                 string_VkResolveModeFlagBits(pRenderingInfo->pStencilAttachment->resolveMode));
            }
        }

        if (pRenderingInfo->pDepthAttachment->resolveMode != VK_RESOLVE_MODE_NONE &&
            pRenderingInfo->pStencilAttachment->resolveMode != VK_RESOLVE_MODE_NONE) {
            if (pRenderingInfo->pDepthAttachment->resolveImageView != pRenderingInfo->pStencilAttachment->resolveImageView) {
                const LogObjectList objlist(commandBuffer, pRenderingInfo->pDepthAttachment->resolveImageView,
                                            pRenderingInfo->pStencilAttachment->resolveImageView);
                skip |= LogError("VUID-VkRenderingInfo-pDepthAttachment-06086", objlist, rendering_info,
                                 "resolveImageView of pDepthAttachment and pStencilAttachment must be the same.");
            }
        }
    }

    for (uint32_t j = 0; j < pRenderingInfo->colorAttachmentCount; ++j) {
        const auto &attachment_info = pRenderingInfo->pColorAttachments[j];
        const Location color_loc = rendering_info.dot(Field::pColorAttachments, j);
        skip |= ValidateRenderingAttachmentInfo(commandBuffer, pRenderingInfo, attachment_info, color_loc);

        if (pRenderingInfo->pColorAttachments[j].imageView != VK_NULL_HANDLE) {
            auto image_view_state = Get<IMAGE_VIEW_STATE>(pRenderingInfo->pColorAttachments[j].imageView);
            IMAGE_STATE *image_state = image_view_state->image_state.get();
            if (!(image_state->createInfo.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
                const LogObjectList objlist(commandBuffer, image_view_state->image_view(), image_state->image());
                skip |= LogError("VUID-VkRenderingInfo-colorAttachmentCount-06087", objlist, color_loc.dot(Field::imageView),
                                 "must have been created with VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT.");
            }
        }

        if (attachment_info.resolveMode == VK_RESOLVE_MODE_EXTERNAL_FORMAT_DOWNSAMPLE_ANDROID) {
            if (!enabled_features.externalFormatResolve) {
                skip |= LogError("VUID-VkRenderingAttachmentInfo-externalFormatResolve-09323", commandBuffer,
                                 color_loc.dot(Field::resolveMode), "is VK_RESOLVE_MODE_EXTERNAL_FORMAT_DOWNSAMPLE_ANDROID.");
            }
            if (pRenderingInfo->colorAttachmentCount != 1) {
                skip |=
                    LogError("VUID-VkRenderingInfo-colorAttachmentCount-09320", commandBuffer, color_loc.dot(Field::resolveMode),
                             "is VK_RESOLVE_MODE_EXTERNAL_FORMAT_DOWNSAMPLE_ANDROID and colorAttachmentCount is %" PRIu32 ".",
                             pRenderingInfo->colorAttachmentCount);
                break;  // only print first index with error
            }
            const auto *fragment_density_info_ext =
                vku::FindStructInPNextChain<VkRenderingFragmentDensityMapAttachmentInfoEXT>(pRenderingInfo->pNext);
            const auto *fragment_shading_rate_info_khr =
                vku::FindStructInPNextChain<VkRenderingFragmentShadingRateAttachmentInfoKHR>(pRenderingInfo->pNext);
            if (fragment_density_info_ext && fragment_density_info_ext->imageView != VK_NULL_HANDLE) {
                skip |= LogError("VUID-VkRenderingInfo-resolveMode-09321", commandBuffer,
                                 rendering_info.pNext(Struct::VkRenderingFragmentDensityMapAttachmentInfoEXT, Field::imageView),
                                 "is not null (%s).", FormatHandle(fragment_density_info_ext->imageView).c_str());
            }
            if (fragment_shading_rate_info_khr && fragment_shading_rate_info_khr->imageView != VK_NULL_HANDLE) {
                skip |= LogError("VUID-VkRenderingInfo-resolveMode-09322", commandBuffer,
                                 rendering_info.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::imageView),
                                 "is not null (%s).", FormatHandle(fragment_shading_rate_info_khr->imageView).c_str());
            }

            auto resolve_view_state = Get<IMAGE_VIEW_STATE>(attachment_info.resolveImageView);
            if (!resolve_view_state) {
                skip |= LogError("VUID-VkRenderingAttachmentInfo-resolveMode-09324", commandBuffer,
                                 color_loc.dot(Field::resolveImageView), "is not valid (%s).",
                                 FormatHandle(attachment_info.resolveImageView).c_str());
            } else {
                if (android_external_format_resolve_null_color_attachment_prop &&
                    resolve_view_state->image_state->createInfo.samples != VK_SAMPLE_COUNT_1_BIT) {
                    skip |= LogError("VUID-VkRenderingAttachmentInfo-nullColorAttachmentWithExternalFormatResolve-09325",
                                     commandBuffer, color_loc.dot(Field::resolveImageView), "image was created with %s.",
                                     string_VkSampleCountFlagBits(resolve_view_state->image_state->createInfo.samples));
                }
                if (!resolve_view_state->image_state->HasAHBFormat()) {
                    skip |= LogError("VUID-VkRenderingAttachmentInfo-resolveMode-09326", commandBuffer,
                                     color_loc.dot(Field::resolveImageView), "was not created with an external format.");
                }
                if (resolve_view_state->create_info.subresourceRange.layerCount != 1) {
                    skip |= LogError("VUID-VkRenderingAttachmentInfo-resolveMode-09327", commandBuffer,
                                     color_loc.dot(Field::resolveImageView),
                                     "was created with subresourceRange.layerCount of %" PRIu32 ".",
                                     resolve_view_state->create_info.subresourceRange.layerCount);
                }
                auto color_view_state = Get<IMAGE_VIEW_STATE>(attachment_info.imageView);
                if (color_view_state) {
                    if (android_external_format_resolve_null_color_attachment_prop) {
                        skip |= LogError("VUID-VkRenderingAttachmentInfo-resolveMode-09328", commandBuffer,
                                         color_loc.dot(Field::imageView), "is not null (%s).",
                                         FormatHandle(attachment_info.imageView).c_str());
                    }
                } else if (!android_external_format_resolve_null_color_attachment_prop) {
                    skip |= LogError("VUID-VkRenderingAttachmentInfo-resolveMode-09329", commandBuffer,
                                     color_loc.dot(Field::imageView), "is not valid.");
                }
            }
        }
    }

    if (pRenderingInfo->pDepthAttachment) {
        const auto &depth_attachment_info = *pRenderingInfo->pDepthAttachment;
        skip |= ValidateRenderingAttachmentInfo(commandBuffer, pRenderingInfo, depth_attachment_info,
                                                rendering_info.dot(Field::pDepthAttachment));

        if (depth_attachment_info.imageView != VK_NULL_HANDLE) {
            auto depth_view_state = Get<IMAGE_VIEW_STATE>(depth_attachment_info.imageView);
            IMAGE_STATE *image_state = depth_view_state->image_state.get();
            if (!(image_state->createInfo.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
                const LogObjectList objlist(commandBuffer, depth_view_state->image_view(), image_state->image());
                skip |= LogError("VUID-VkRenderingInfo-pDepthAttachment-06088", objlist,
                                 rendering_info.dot(Field::pDepthAttachment).dot(Field::imageView),
                                 "internal image must have been created with VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT.");
            }

            if (!vkuFormatHasDepth(depth_view_state->create_info.format)) {
                const LogObjectList objlist(commandBuffer, depth_view_state->image_view());
                skip |= LogError("VUID-VkRenderingInfo-pDepthAttachment-06547", objlist,
                                 rendering_info.dot(Field::pDepthAttachment).dot(Field::imageView),
                                 "was created with a format (%s) that does not have a depth aspect.",
                                 string_VkFormat(depth_view_state->create_info.format));
            }
        }
        if (depth_attachment_info.resolveMode == VK_RESOLVE_MODE_EXTERNAL_FORMAT_DOWNSAMPLE_ANDROID) {
            skip |= LogError("VUID-VkRenderingInfo-pDepthAttachment-09318", commandBuffer,
                             rendering_info.dot(Field::pDepthAttachment).dot(Field::resolveMode),
                             "is VK_RESOLVE_MODE_EXTERNAL_FORMAT_DOWNSAMPLE_ANDROID.");
        }
    }

    if (pRenderingInfo->pStencilAttachment) {
        const auto &stencil_attachment_info = *pRenderingInfo->pStencilAttachment;
        skip |= ValidateRenderingAttachmentInfo(commandBuffer, pRenderingInfo, stencil_attachment_info,
                                                rendering_info.dot(Field::pStencilAttachment));

        if (stencil_attachment_info.imageView != VK_NULL_HANDLE) {
            auto stencil_view_state = Get<IMAGE_VIEW_STATE>(stencil_attachment_info.imageView);
            IMAGE_STATE *image_state = stencil_view_state->image_state.get();
            if (!(image_state->createInfo.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
                const LogObjectList objlist(commandBuffer, stencil_view_state->image_view(), image_state->image());
                skip |= LogError("VUID-VkRenderingInfo-pStencilAttachment-06089", objlist,
                                 rendering_info.dot(Field::pStencilAttachment).dot(Field::imageView),
                                 "internal image must have been created with VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT.");
            }

            if (!vkuFormatHasStencil(stencil_view_state->create_info.format)) {
                const LogObjectList objlist(commandBuffer, stencil_view_state->image_view());
                skip |= LogError("VUID-VkRenderingInfo-pStencilAttachment-06548", objlist,
                                 rendering_info.dot(Field::pStencilAttachment).dot(Field::imageView),
                                 "was created with a format (%s) that does not have a stencil aspect.",
                                 string_VkFormat(stencil_view_state->create_info.format));
            }
        }
        if (stencil_attachment_info.resolveMode == VK_RESOLVE_MODE_EXTERNAL_FORMAT_DOWNSAMPLE_ANDROID) {
            skip |= LogError("VUID-VkRenderingInfo-pStencilAttachment-09319", commandBuffer,
                             rendering_info.dot(Field::pStencilAttachment).dot(Field::resolveMode),
                             "is VK_RESOLVE_MODE_EXTERNAL_FORMAT_DOWNSAMPLE_ANDROID.");
        }
    }

    if (MostSignificantBit(pRenderingInfo->viewMask) >= static_cast<int32_t>(phys_dev_props_core11.maxMultiviewViewCount)) {
        skip |= LogError("VUID-VkRenderingInfo-viewMask-06128", commandBuffer, rendering_info.dot(Field::viewMask),
                         "(0x%" PRIx32 ") most significant bit must be less maxMultiviewViewCount (%" PRIu32 ")",
                         pRenderingInfo->viewMask, phys_dev_props_core11.maxMultiviewViewCount);
    }

    if (IsExtEnabled(device_extensions.vk_ext_multisampled_render_to_single_sampled)) {
        const auto *msrtss_info = vku::FindStructInPNextChain<VkMultisampledRenderToSingleSampledInfoEXT>(pRenderingInfo->pNext);
        if (msrtss_info) {
            for (uint32_t j = 0; j < pRenderingInfo->colorAttachmentCount; ++j) {
                if (pRenderingInfo->pColorAttachments[j].imageView != VK_NULL_HANDLE) {
                    const auto image_view_state = Get<IMAGE_VIEW_STATE>(pRenderingInfo->pColorAttachments[j].imageView);
                    skip |= ValidateMultisampledRenderToSingleSampleView(
                        commandBuffer, image_view_state, msrtss_info,
                        rendering_info.dot(Field::pColorAttachments, j).dot(Field::imageView), rendering_info);
                }
            }
            if (pRenderingInfo->pDepthAttachment && pRenderingInfo->pDepthAttachment->imageView != VK_NULL_HANDLE) {
                const auto depth_view_state = Get<IMAGE_VIEW_STATE>(pRenderingInfo->pDepthAttachment->imageView);
                skip |= ValidateMultisampledRenderToSingleSampleView(
                    commandBuffer, depth_view_state, msrtss_info, rendering_info.dot(Field::pDepthAttachment).dot(Field::imageView),
                    rendering_info);
            }
            if (pRenderingInfo->pStencilAttachment && pRenderingInfo->pStencilAttachment->imageView != VK_NULL_HANDLE) {
                const auto stencil_view_state = Get<IMAGE_VIEW_STATE>(pRenderingInfo->pStencilAttachment->imageView);
                skip |= ValidateMultisampledRenderToSingleSampleView(
                    commandBuffer, stencil_view_state, msrtss_info,
                    rendering_info.dot(Field::pStencilAttachment).dot(Field::imageView), rendering_info);
            }
            if (msrtss_info->rasterizationSamples == VK_SAMPLE_COUNT_1_BIT) {
                skip |=
                    LogError("VUID-VkMultisampledRenderToSingleSampledInfoEXT-rasterizationSamples-06878", commandBuffer,
                             rendering_info.pNext(Struct::VkMultisampledRenderToSingleSampledInfoEXT, Field::rasterizationSamples),
                             "is VK_SAMPLE_COUNT_1_BIT.");
            }
        }
    }
    return skip;
}

// Flags validation error if the associated call is made inside a render pass. The apiName routine should ONLY be called outside a
// render pass.
bool CoreChecks::InsideRenderPass(const CMD_BUFFER_STATE &cb_state, const Location &loc, const char *vuid) const {
    bool inside = false;
    if (cb_state.activeRenderPass) {
        inside = LogError(vuid, cb_state.commandBuffer(), loc, "It is invalid to issue this call inside an active %s.",
                          FormatHandle(cb_state.activeRenderPass->renderPass()).c_str());
    }
    return inside;
}

// Flags validation error if the associated call is made outside a render pass. The apiName
// routine should ONLY be called inside a render pass.
bool CoreChecks::OutsideRenderPass(const CMD_BUFFER_STATE &cb_state, const Location &loc, const char *vuid) const {
    bool outside = false;
    if (((cb_state.createInfo.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) && (!cb_state.activeRenderPass)) ||
        ((cb_state.createInfo.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) && (!cb_state.activeRenderPass) &&
         !(cb_state.beginInfo.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT))) {
        outside = LogError(vuid, cb_state.commandBuffer(), loc, "This call must be issued inside an active render pass.");
    }
    return outside;
}

bool CoreChecks::PreCallValidateCmdEndRendering(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state) return false;
    bool skip = false;
    skip |= ValidateCmd(*cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    if (!cb_state->activeRenderPass->UsesDynamicRendering()) {
        skip |= LogError("VUID-vkCmdEndRendering-None-06161", commandBuffer, error_obj.location,
                         "in a render pass instance that was not begun with vkCmdBeginRendering().");
    }
    if (cb_state->activeRenderPass->use_dynamic_rendering_inherited == true) {
        skip |= LogError("VUID-vkCmdEndRendering-commandBuffer-06162", commandBuffer, error_obj.location,
                         "in a render pass instance that was not begun in this command buffer.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const {
    return PreCallValidateCmdEndRendering(commandBuffer, error_obj);
}

bool CoreChecks::ValidateMultisampledRenderToSingleSampleView(VkCommandBuffer commandBuffer,
                                                              const std::shared_ptr<const IMAGE_VIEW_STATE> &image_view_state,
                                                              const VkMultisampledRenderToSingleSampledInfoEXT *msrtss_info,
                                                              const Location &attachment_loc, const Location &loc) const {
    bool skip = false;
    if (!msrtss_info->multisampledRenderToSingleSampledEnable) {
        return false;
    }
    const LogObjectList objlist(commandBuffer, image_view_state->Handle());
    if ((image_view_state->samples != VK_SAMPLE_COUNT_1_BIT) && (image_view_state->samples != msrtss_info->rasterizationSamples)) {
        skip |=
            LogError("VUID-VkRenderingInfo-imageView-06858", objlist,
                     loc.pNext(Struct::VkMultisampledRenderToSingleSampledInfoEXT, Field::multisampledRenderToSingleSampledEnable),
                     "is %s, but %s was created with %s, which is not VK_SAMPLE_COUNT_1_BIT.",
                     string_VkSampleCountFlagBits(msrtss_info->rasterizationSamples), attachment_loc.Fields().c_str(),
                     string_VkSampleCountFlagBits(image_view_state->samples));
    }
    IMAGE_STATE *image_state = image_view_state->image_state.get();
    if ((image_view_state->samples == VK_SAMPLE_COUNT_1_BIT) &&
        !(image_state->createInfo.flags & VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT)) {
        skip |= LogError("VUID-VkRenderingInfo-imageView-06859", objlist, attachment_loc,
                         "was created with VK_SAMPLE_COUNT_1_BIT but "
                         "VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT was not set in "
                         "pImageCreateInfo.flags when the image used to create the imageView (%s) was created",
                         FormatHandle(image_state->image()).c_str());
    }
    if (!image_state->image_format_properties.sampleCounts) {
        if (GetPhysicalDeviceImageFormatProperties(*image_state, "VUID-VkMultisampledRenderToSingleSampledInfoEXT-pNext-06880",
                                                   attachment_loc))
            return true;
    }
    if (!(image_state->image_format_properties.sampleCounts & msrtss_info->rasterizationSamples)) {
        skip |= LogError(
            "VUID-VkMultisampledRenderToSingleSampledInfoEXT-pNext-06880", objlist,
            loc.pNext(Struct::VkMultisampledRenderToSingleSampledInfoEXT, Field::rasterizationSamples),
            "is %s, but %s format %s does not support sample count %s from an image with imageType: %s, tiling: "
            "%s, usage: %s, flags: %s.",
            string_VkSampleCountFlagBits(msrtss_info->rasterizationSamples), attachment_loc.Fields().c_str(),
            string_VkFormat(image_view_state->create_info.format), string_VkSampleCountFlagBits(msrtss_info->rasterizationSamples),
            string_VkImageType(image_state->createInfo.imageType), string_VkImageTiling(image_state->createInfo.tiling),
            string_VkImageUsageFlags(image_state->createInfo.usage).c_str(),
            string_VkImageCreateFlags(image_state->createInfo.flags).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfoKHR *pRenderingInfo,
                                                     const ErrorObject &error_obj) const {
    return PreCallValidateCmdBeginRendering(commandBuffer, pRenderingInfo, error_obj);
}

// If a renderpass is active, verify that the given command type is appropriate for current subpass state
bool CoreChecks::ValidateCmdSubpassState(const CMD_BUFFER_STATE &cb_state, const Location &loc, const char *vuid) const {
    if (!cb_state.activeRenderPass || cb_state.activeRenderPass->UsesDynamicRendering()) return false;
    bool skip = false;
    if (cb_state.createInfo.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY &&
        cb_state.activeSubpassContents == VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS &&
        (loc.function != Func::vkCmdExecuteCommands && loc.function != Func::vkCmdNextSubpass &&
         loc.function != Func::vkCmdEndRenderPass && loc.function != Func::vkCmdNextSubpass2 &&
         loc.function != Func::vkCmdNextSubpass2KHR && loc.function != Func::vkCmdEndRenderPass2 &&
         loc.function != Func::vkCmdEndRenderPass2KHR)) {
        skip |= LogError(vuid, cb_state.commandBuffer(), loc, "cannot be called in a subpass using secondary command buffers.");
    }
    return skip;
}

bool CoreChecks::ValidateCmdNextSubpass(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const {
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    bool skip = false;
    const bool use_rp2 = error_obj.location.function != Func::vkCmdNextSubpass;
    const char *vuid;

    skip |= ValidateCmd(*cb_state, error_obj.location);
    if (skip) return skip;  // basic validation failed, might have null pointers

    auto subpass_count = cb_state->activeRenderPass->createInfo.subpassCount;
    if (cb_state->GetActiveSubpass() == subpass_count - 1) {
        vuid = use_rp2 ? "VUID-vkCmdNextSubpass2-None-03102" : "VUID-vkCmdNextSubpass-None-00909";
        skip |= LogError(vuid, commandBuffer, error_obj.location, "Attempted to advance beyond final subpass.");
    }
    if (cb_state->transform_feedback_active) {
        vuid = use_rp2 ? "VUID-vkCmdNextSubpass2-None-02350" : "VUID-vkCmdNextSubpass-None-02349";
        skip |= LogError(vuid, commandBuffer, error_obj.location, "transform feedback is active.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                               const ErrorObject &error_obj) const {
    return ValidateCmdNextSubpass(commandBuffer, error_obj);
}

bool CoreChecks::PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                   const VkSubpassEndInfo *pSubpassEndInfo, const ErrorObject &error_obj) const {
    return PreCallValidateCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, error_obj);
}

bool CoreChecks::PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                const VkSubpassEndInfo *pSubpassEndInfo, const ErrorObject &error_obj) const {
    return ValidateCmdNextSubpass(commandBuffer, error_obj);
}

void CoreChecks::RecordCmdNextSubpassLayouts(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    TransitionSubpassLayouts(cb_state.get(), *cb_state->activeRenderPass, cb_state->GetActiveSubpass());
}

void CoreChecks::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                              const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdNextSubpass(commandBuffer, contents, record_obj);
    RecordCmdNextSubpassLayouts(commandBuffer, contents);
}

void CoreChecks::PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                  const VkSubpassEndInfo *pSubpassEndInfo, const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, record_obj);
    RecordCmdNextSubpassLayouts(commandBuffer, pSubpassBeginInfo->contents);
}

void CoreChecks::PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                               const VkSubpassEndInfo *pSubpassEndInfo, const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, record_obj);
    RecordCmdNextSubpassLayouts(commandBuffer, pSubpassBeginInfo->contents);
}

bool CoreChecks::MatchUsage(uint32_t count, const VkAttachmentReference2 *attachments, const VkFramebufferCreateInfo *fbci,
                            VkImageUsageFlagBits usage_flag, const char *vuid, const Location &create_info_loc) const {
    bool skip = false;

    if (!attachments) {
        return false;
    }
    for (uint32_t attach = 0; attach < count; attach++) {
        const uint32_t fb_attachment = attachments[attach].attachment;
        // Attachment counts are verified elsewhere, but prevent an invalid access
        if (fb_attachment == VK_ATTACHMENT_UNUSED || fb_attachment >= fbci->attachmentCount) {
            continue;
        }
        if ((fbci->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) == 0) {
            const VkImageView *image_view = &fbci->pAttachments[fb_attachment];
            auto view_state = Get<IMAGE_VIEW_STATE>(*image_view);
            if (view_state) {
                const auto &ici = view_state->image_state->createInfo;
                auto creation_usage = ici.usage;
                const auto stencil_usage_info = vku::FindStructInPNextChain<VkImageStencilUsageCreateInfo>(ici.pNext);
                if (stencil_usage_info) {
                    creation_usage |= stencil_usage_info->stencilUsage;
                }
                if ((creation_usage & usage_flag) == 0) {
                    skip |= LogError(vuid, device, create_info_loc.dot(Field::pAttachments, fb_attachment),
                                     "conflicts with the image's flags (%s).", string_VkImageUsageFlagBits(usage_flag));
                }
            }
        } else {
            const VkFramebufferAttachmentsCreateInfo *fbaci = vku::FindStructInPNextChain<VkFramebufferAttachmentsCreateInfo>(fbci->pNext);
            if (fbaci != nullptr && fbaci->pAttachmentImageInfos != nullptr && fbaci->attachmentImageInfoCount > fb_attachment) {
                uint32_t image_usage = fbaci->pAttachmentImageInfos[fb_attachment].usage;
                if ((image_usage & usage_flag) == 0) {
                    skip |= LogError(vuid, device, create_info_loc.dot(Field::pAttachments, fb_attachment),
                                     "conflicts with the image's flags (%s).", string_VkImageUsageFlagBits(usage_flag));
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::MsRenderedToSingleSampledValidateFBAttachments(uint32_t count, const VkAttachmentReference2 *attachments,
                                                                const VkFramebufferCreateInfo *fbci,
                                                                const VkRenderPassCreateInfo2 *rpci, uint32_t subpass,
                                                                VkSampleCountFlagBits sample_count,
                                                                const Location &create_info_loc) const {
    bool skip = false;

    for (uint32_t attach = 0; attach < count; attach++) {
        const uint32_t fb_attachment = attachments[attach].attachment;
        if (fb_attachment == VK_ATTACHMENT_UNUSED || fb_attachment >= fbci->attachmentCount) {
            continue;
        }

        const auto renderpass_samples = rpci->pAttachments[fb_attachment].samples;
        if (renderpass_samples == VK_SAMPLE_COUNT_1_BIT) {
            const VkImageView *image_view = &fbci->pAttachments[fb_attachment];
            auto view_state = Get<IMAGE_VIEW_STATE>(*image_view);
            auto image_state = view_state->image_state;
            if (!(image_state->createInfo.flags & VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT)) {
                skip |= LogError("VUID-VkFramebufferCreateInfo-samples-06881", device, create_info_loc,
                                 "Renderpass subpass %" PRIu32
                                 " enables "
                                 "multisampled-render-to-single-sampled and attachment %" PRIu32
                                 ", is specified from with "
                                 "VK_SAMPLE_COUNT_1_BIT samples, but image (%s) was created without "
                                 "VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT in its createInfo.flags.",
                                 subpass, fb_attachment, FormatHandle(*image_state).c_str());
            }
            const VkImageCreateInfo image_create_info = image_state->createInfo;
            if (!image_state->image_format_properties.sampleCounts) {
                skip |= GetPhysicalDeviceImageFormatProperties(*image_state.get(), "VUID-VkFramebufferCreateInfo-samples-07009",
                                                               create_info_loc);
            }
            if (!(image_state->image_format_properties.sampleCounts & sample_count)) {
                skip |= LogError(
                    "VUID-VkFramebufferCreateInfo-samples-07009", device, create_info_loc,
                    "Renderpass subpass %" PRIu32
                    " enables "
                    "multisampled-render-to-single-sampled and attachment %" PRIu32
                    ", is specified from with "
                    "VK_SAMPLE_COUNT_1_BIT samples, but image (%s) created with format %s imageType: %s, "
                    "tiling: %s, usage: %s, "
                    "flags: %s does not support a rasterizationSamples count of %s",
                    subpass, fb_attachment, FormatHandle(*image_state).c_str(), string_VkFormat(image_create_info.format),
                    string_VkImageType(image_create_info.imageType), string_VkImageTiling(image_create_info.tiling),
                    string_VkImageUsageFlags(image_create_info.usage).c_str(),
                    string_VkImageCreateFlags(image_create_info.flags).c_str(), string_VkSampleCountFlagBits(sample_count));
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer,
                                                  const ErrorObject &error_obj) const {
    // TODO : Verify that renderPass FB is created with is compatible with FB
    bool skip = false;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    const auto *framebuffer_attachments_create_info = vku::FindStructInPNextChain<VkFramebufferAttachmentsCreateInfo>(pCreateInfo->pNext);
    if ((pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) != 0) {
        if (!enabled_features.imagelessFramebuffer) {
            skip |= LogError("VUID-VkFramebufferCreateInfo-flags-03189", device, create_info_loc.dot(Field::flags),
                             "includes VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT, but the imagelessFramebuffer feature is not enabled.");
        }

        if (framebuffer_attachments_create_info == nullptr) {
            skip |= LogError("VUID-VkFramebufferCreateInfo-flags-03190", device, create_info_loc.dot(Field::flags),
                             "includes VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT, but no instance of VkFramebufferAttachmentsCreateInfo "
                             "is present in the pNext chain.");
        } else {
            if (framebuffer_attachments_create_info->attachmentImageInfoCount != 0 &&
                framebuffer_attachments_create_info->attachmentImageInfoCount != pCreateInfo->attachmentCount) {
                skip |= LogError("VUID-VkFramebufferCreateInfo-flags-03191", device,
                                 create_info_loc.pNext(Struct::VkFramebufferAttachmentsCreateInfo, Field::attachmentImageInfoCount),
                                 "is %" PRIu32 " which is not equal to pCreateInfo->attachmentCount (%" PRIu32 ").",
                                 framebuffer_attachments_create_info->attachmentImageInfoCount, pCreateInfo->attachmentCount);
            }
        }
    }
    if (framebuffer_attachments_create_info) {
        for (uint32_t i = 0; i < framebuffer_attachments_create_info->attachmentImageInfoCount; ++i) {
            if (framebuffer_attachments_create_info->pAttachmentImageInfos[i].pNext != nullptr) {
                skip |= LogError("VUID-VkFramebufferAttachmentImageInfo-pNext-pNext", device,
                                 create_info_loc.pNext(Struct::VkFramebufferAttachmentsCreateInfo, Field::pAttachmentImageInfos, i)
                                     .dot(Field::pNext),
                                 "is not NULL.");
            }
        }
    }

    // Verify FB dimensions are within physical device limits
    if (pCreateInfo->width > phys_dev_props.limits.maxFramebufferWidth) {
        skip |= LogError("VUID-VkFramebufferCreateInfo-width-00886", device, create_info_loc.dot(Field::width),
                         "(%" PRIu32 ") exceeds physical device limits (%" PRIu32 ").", pCreateInfo->width,
                         phys_dev_props.limits.maxFramebufferWidth);
    }
    if (pCreateInfo->height > phys_dev_props.limits.maxFramebufferHeight) {
        skip |= LogError("VUID-VkFramebufferCreateInfo-height-00888", device, create_info_loc.dot(Field::height),
                         "(%" PRIu32 ") exceeds physical device limits (%" PRIu32 ").", pCreateInfo->height,
                         phys_dev_props.limits.maxFramebufferHeight);
    }
    if (pCreateInfo->layers > phys_dev_props.limits.maxFramebufferLayers) {
        skip |= LogError("VUID-VkFramebufferCreateInfo-layers-00890", device, create_info_loc.dot(Field::layers),
                         "(%" PRIu32 ") exceeds physical device limits (%" PRIu32 ").", pCreateInfo->layers,
                         phys_dev_props.limits.maxFramebufferLayers);
    }

    auto rp_state = Get<RENDER_PASS_STATE>(pCreateInfo->renderPass);
    if (!rp_state) {
        return skip;
    }
    const VkRenderPassCreateInfo2 *rpci = rp_state->createInfo.ptr();

    bool b_has_non_zero_view_masks = false;
    for (uint32_t i = 0; i < rpci->subpassCount; ++i) {
        if (rpci->pSubpasses[i].viewMask != 0) {
            b_has_non_zero_view_masks = true;
            break;
        }
    }

    if (rpci->attachmentCount != pCreateInfo->attachmentCount) {
        skip |= LogError("VUID-VkFramebufferCreateInfo-attachmentCount-00876", pCreateInfo->renderPass,
                         create_info_loc.dot(Field::attachmentCount),
                         "%" PRIu32 " does not match attachmentCount of %" PRIu32 " of %s being used to create Framebuffer.",
                         pCreateInfo->attachmentCount, rpci->attachmentCount, FormatHandle(pCreateInfo->renderPass).c_str());
        return skip;  // nothing else to validate
    }

    // attachmentCounts match, so make sure corresponding attachment details line up
    if ((pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) == 0) {
        const VkImageView *image_views = pCreateInfo->pAttachments;
        for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
            const Location attachment_loc = create_info_loc.dot(Field::pAttachments, i);
            auto view_state = Get<IMAGE_VIEW_STATE>(image_views[i]);
            auto &ivci = view_state->create_info;
            auto &subresource_range = view_state->normalized_subresource_range;
            if (ivci.format != rpci->pAttachments[i].format) {
                LogObjectList objlist(pCreateInfo->renderPass, image_views[i]);
                skip |=
                    LogError("VUID-VkFramebufferCreateInfo-pAttachments-00880", objlist, attachment_loc,
                             "has format of %s that does not match the format of %s used by the corresponding attachment for %s.",
                             string_VkFormat(ivci.format), string_VkFormat(rpci->pAttachments[i].format),
                             FormatHandle(pCreateInfo->renderPass).c_str());
            } else if (ivci.format == VK_FORMAT_UNDEFINED) {
                // both have external foramts
                const uint64_t attachment_external_format = GetExternalFormat(rpci->pAttachments[i].pNext);
                if (view_state->image_state->ahb_format != attachment_external_format) {
                    LogObjectList objlist(pCreateInfo->renderPass, image_views[i]);
                    skip |= LogError("VUID-VkFramebufferCreateInfo-pAttachments-09350", objlist, attachment_loc,
                                     "has externalFormat %" PRIu64 " that does not match the externalFormat of %" PRIu64
                                     " used by the corresponding attachment for %s.",
                                     view_state->image_state->ahb_format, attachment_external_format,
                                     FormatHandle(pCreateInfo->renderPass).c_str());
                }
            }

            const auto &ici = view_state->image_state->createInfo;
            if (ici.samples != rpci->pAttachments[i].samples) {
                LogObjectList objlist(pCreateInfo->renderPass, image_views[i], ivci.image);
                skip |=
                    LogError("VUID-VkFramebufferCreateInfo-pAttachments-00881", objlist, attachment_loc,
                             "has %s samples that do not match the %s samples used by the corresponding attachment for %s.",
                             string_VkSampleCountFlagBits(ici.samples), string_VkSampleCountFlagBits(rpci->pAttachments[i].samples),
                             FormatHandle(pCreateInfo->renderPass).c_str());
            }

            // Verify that image memory is valid
            auto image_data = Get<IMAGE_STATE>(ivci.image);
            skip |= ValidateMemoryIsBoundToImage(LogObjectList(ivci.image), *image_data, attachment_loc,
                                                 kVUID_Core_Bound_Resource_FreedMemoryAccess);

            // Verify that view only has a single mip level
            if (subresource_range.levelCount != 1) {
                LogObjectList objlist(pCreateInfo->renderPass, image_views[i], ivci.image);
                skip |= LogError("VUID-VkFramebufferCreateInfo-pAttachments-00883", objlist, attachment_loc,
                                 "has mip levelCount of %" PRIu32
                                 " but only a single mip level (levelCount ==  1) is allowed when creating a Framebuffer.",
                                 subresource_range.levelCount);
            }
            const uint32_t mip_level = subresource_range.baseMipLevel;
            uint32_t mip_width = std::max(1u, ici.extent.width >> mip_level);
            uint32_t mip_height = std::max(1u, ici.extent.height >> mip_level);
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
                        LogObjectList objlist(pCreateInfo->renderPass, image_views[i], ivci.image);
                        skip |= LogError("VUID-VkFramebufferCreateInfo-renderPass-04536", objlist, attachment_loc,
                                         "has a layer count (%" PRIu32
                                         ") less than or equal to the highest bit in the view mask (%i) of subpass %" PRIu32 ".",
                                         subresource_range.layerCount, highest_view_bit, j);
                    }
                }

                if (enabled_features.attachmentFragmentShadingRate) {
                    const auto *fsr_attachment = vku::FindStructInPNextChain<VkFragmentShadingRateAttachmentInfoKHR>(subpass.pNext);
                    if (fsr_attachment && fsr_attachment->pFragmentShadingRateAttachment &&
                        fsr_attachment->pFragmentShadingRateAttachment->attachment == i) {
                        used_as_fragment_shading_rate_attachment = true;
                        if ((mip_width * fsr_attachment->shadingRateAttachmentTexelSize.width) < pCreateInfo->width) {
                            LogObjectList objlist(pCreateInfo->renderPass, image_views[i], ivci.image);
                            skip |= LogError("VUID-VkFramebufferCreateInfo-flags-04539", objlist, attachment_loc,
                                             "mip level %" PRIu32
                                             " is used as a "
                                             "fragment shading rate attachment in subpass %" PRIu32
                                             ", but the product of its "
                                             "width (%" PRIu32
                                             ") and the "
                                             "specified shading rate texel width (%" PRIu32
                                             ") are smaller than the "
                                             "corresponding framebuffer width (%" PRIu32 ").",
                                             subresource_range.baseMipLevel, j, mip_width,
                                             fsr_attachment->shadingRateAttachmentTexelSize.width, pCreateInfo->width);
                        }
                        if ((mip_height * fsr_attachment->shadingRateAttachmentTexelSize.height) < pCreateInfo->height) {
                            LogObjectList objlist(pCreateInfo->renderPass, image_views[i], ivci.image);
                            skip |= LogError("VUID-VkFramebufferCreateInfo-flags-04540", objlist, attachment_loc,
                                             "mip level %" PRIu32
                                             " is used as a "
                                             "fragment shading rate attachment in subpass %" PRIu32
                                             ", but the product of its "
                                             "height (%" PRIu32
                                             ") and the "
                                             "specified shading rate texel height (%" PRIu32
                                             ") are smaller than the corresponding "
                                             "framebuffer height (%" PRIu32 ").",
                                             subresource_range.baseMipLevel, j, mip_height,
                                             fsr_attachment->shadingRateAttachmentTexelSize.height, pCreateInfo->height);
                        }
                        if (highest_view_bit >= 0) {
                            fsr_non_zero_viewmasks = true;
                        }
                        if (static_cast<int32_t>(subresource_range.layerCount) <= highest_view_bit &&
                            subresource_range.layerCount != 1) {
                            LogObjectList objlist(pCreateInfo->renderPass, image_views[i], ivci.image);
                            skip |=
                                LogError("VUID-VkFramebufferCreateInfo-flags-04537", objlist, attachment_loc,
                                         "has a layer count (%" PRIu32
                                         ") less than or equal to the highest bit in the view mask (%i) of subpass %" PRIu32 ".",
                                         subresource_range.layerCount, highest_view_bit, j);
                        }
                    }
                }

                if (enabled_features.fragmentDensityMap && api_version >= VK_API_VERSION_1_1) {
                    const auto *fdm_attachment =
                        vku::FindStructInPNextChain<VkRenderPassFragmentDensityMapCreateInfoEXT>(rpci->pNext);

                    if (fdm_attachment && fdm_attachment->fragmentDensityMapAttachment.attachment == i) {
                        int32_t layer_count = view_state->normalized_subresource_range.layerCount;
                        if (b_has_non_zero_view_masks && layer_count != 1 && layer_count <= highest_view_bit) {
                            LogObjectList objlist(pCreateInfo->renderPass, image_views[i], ivci.image);
                            skip |= LogError("VUID-VkFramebufferCreateInfo-renderPass-02746", objlist, attachment_loc,
                                             "has a layer count (%" PRId32
                                             ") different than 1 or lower than the most significant bit in viewMask (%i"
                                             ") but renderPass (%s) was specified with non-zero view masks.",
                                             layer_count, highest_view_bit, FormatHandle(pCreateInfo->renderPass).c_str());
                        }

                        if (!b_has_non_zero_view_masks && layer_count != 1) {
                            LogObjectList objlist(pCreateInfo->renderPass, image_views[i], ivci.image);
                            skip |= LogError("VUID-VkFramebufferCreateInfo-renderPass-02747", objlist, attachment_loc,
                                             "has a layer count (%" PRIu32
                                             ") not equal to 1 but renderPass (%s) was not specified with non-zero view masks.",
                                             layer_count, FormatHandle(pCreateInfo->renderPass).c_str());
                        }
                    }
                }
            }

            if (enabled_features.fragmentDensityMap) {
                const auto *fdm_attachment = vku::FindStructInPNextChain<VkRenderPassFragmentDensityMapCreateInfoEXT>(rpci->pNext);
                if (fdm_attachment && fdm_attachment->fragmentDensityMapAttachment.attachment == i) {
                    uint32_t ceiling_width = vvl::GetQuotientCeil(
                        pCreateInfo->width, phys_dev_ext_props.fragment_density_map_props.maxFragmentDensityTexelSize.width);
                    if (mip_width < ceiling_width) {
                        LogObjectList objlist(pCreateInfo->renderPass, image_views[i], ivci.image);
                        skip |= LogError("VUID-VkFramebufferCreateInfo-pAttachments-02555", objlist, attachment_loc,
                                         "mip level %" PRIu32
                                         " has width "
                                         "smaller than the corresponding the ceiling of framebuffer width / "
                                         "maxFragmentDensityTexelSize.width "
                                         "Here are the respective dimensions for attachment #%" PRIu32
                                         ", the ceiling value:\n "
                                         "attachment #%" PRIu32
                                         ", framebuffer:\n"
                                         "width: %" PRIu32 ", the ceiling value: %" PRIu32 "\n",
                                         subresource_range.baseMipLevel, i, i, mip_width, ceiling_width);
                    }
                    uint32_t ceiling_height = vvl::GetQuotientCeil(
                        pCreateInfo->height, phys_dev_ext_props.fragment_density_map_props.maxFragmentDensityTexelSize.height);
                    if (mip_height < ceiling_height) {
                        LogObjectList objlist(pCreateInfo->renderPass, image_views[i], ivci.image);
                        skip |= LogError("VUID-VkFramebufferCreateInfo-pAttachments-02556", objlist, attachment_loc,
                                         "mip level %" PRIu32
                                         " has height smaller than the corresponding the ceiling of framebuffer height / "
                                         "maxFragmentDensityTexelSize.height "
                                         "Here are the respective dimensions for attachment #%" PRIu32
                                         ", the ceiling value:\n "
                                         "attachment #%" PRIu32
                                         ", framebuffer:\n"
                                         "height: %" PRIu32 ", the ceiling value: %" PRIu32 "\n",
                                         subresource_range.baseMipLevel, i, i, mip_height, ceiling_height);
                    }
                    if (view_state->normalized_subresource_range.layerCount != 1 &&
                        !IsExtEnabled(device_extensions.vk_khr_multiview)) {
                        LogObjectList objlist(pCreateInfo->renderPass, image_views[i], ivci.image);
                        skip |= LogError("VUID-VkFramebufferCreateInfo-renderPass-02747", objlist, attachment_loc,
                                         "is referenced by "
                                         "VkRenderPassFragmentDensityMapCreateInfoEXT::fragmentDensityMapAttachment in "
                                         "the pNext chain, but it was create with subresourceRange.layerCount (%" PRIu32
                                         ") different from 1.",
                                         view_state->normalized_subresource_range.layerCount);
                    }
                }
            }

            if (used_as_input_color_resolve_depth_stencil_attachment) {
                if (mip_width < pCreateInfo->width) {
                    LogObjectList objlist(pCreateInfo->renderPass, image_views[i], ivci.image);
                    skip |= LogError("VUID-VkFramebufferCreateInfo-flags-04533", objlist, attachment_loc,
                                     "mip level %" PRIu32 " has width (%" PRIu32
                                     ") smaller than the corresponding framebuffer width (%" PRIu32 ").",
                                     mip_level, mip_width, pCreateInfo->width);
                }
                if (mip_height < pCreateInfo->height) {
                    LogObjectList objlist(pCreateInfo->renderPass, image_views[i], ivci.image);
                    skip |= LogError("VUID-VkFramebufferCreateInfo-flags-04534", objlist, attachment_loc,
                                     "mip level %" PRIu32 " has height (%" PRIu32
                                     ") smaller than the corresponding framebuffer height (%" PRIu32 ").",
                                     mip_level, mip_height, pCreateInfo->height);
                }
                uint32_t layerCount = view_state->GetAttachmentLayerCount();
                if (layerCount < pCreateInfo->layers) {
                    LogObjectList objlist(pCreateInfo->renderPass, image_views[i], ivci.image);
                    skip |= LogError("VUID-VkFramebufferCreateInfo-flags-04535", objlist, attachment_loc,
                                     "has a layer count (%" PRIu32
                                     ") smaller than the corresponding framebuffer layer count (%" PRIu32 ").",
                                     layerCount, pCreateInfo->layers);
                }
            }

            if (used_as_fragment_shading_rate_attachment && !fsr_non_zero_viewmasks) {
                if (subresource_range.layerCount != 1 && subresource_range.layerCount < pCreateInfo->layers) {
                    LogObjectList objlist(pCreateInfo->renderPass, image_views[i], ivci.image);
                    skip |= LogError("VUID-VkFramebufferCreateInfo-flags-04538", objlist, attachment_loc,
                                     "has a layer count (%" PRIu32
                                     ") "
                                     "smaller than the corresponding framebuffer layer count (%" PRIu32 ").",
                                     subresource_range.layerCount, pCreateInfo->layers);
                }
            }

            if (IsIdentitySwizzle(ivci.components) == false) {
                LogObjectList objlist(pCreateInfo->renderPass, image_views[i]);
                skip |= LogError("VUID-VkFramebufferCreateInfo-pAttachments-00884", objlist, attachment_loc,
                                 "has non-identy swizzle. All "
                                 "framebuffer attachments must have been created with the identity swizzle. Here are the actual "
                                 "swizzle values:\n"
                                 "r swizzle = %s\n"
                                 "g swizzle = %s\n"
                                 "b swizzle = %s\n"
                                 "a swizzle = %s\n",
                                 string_VkComponentSwizzle(ivci.components.r), string_VkComponentSwizzle(ivci.components.g),
                                 string_VkComponentSwizzle(ivci.components.b), string_VkComponentSwizzle(ivci.components.a));
            }
            if ((ivci.viewType == VK_IMAGE_VIEW_TYPE_2D) || (ivci.viewType == VK_IMAGE_VIEW_TYPE_2D)) {
                auto image_state = Get<IMAGE_STATE>(ivci.image);
                if (image_state->createInfo.imageType == VK_IMAGE_TYPE_3D) {
                    if (vkuFormatIsDepthOrStencil(ivci.format)) {
                        LogObjectList objlist(pCreateInfo->renderPass, image_views[i], ivci.image);
                        skip |= LogError("VUID-VkFramebufferCreateInfo-pAttachments-00891", objlist, attachment_loc,
                                         "has an image view type of %s which was taken from image %s of type "
                                         "VK_IMAGE_TYPE_3D, but the image view format is a "
                                         "depth/stencil format %s.",
                                         string_VkImageViewType(ivci.viewType), FormatHandle(ivci.image).c_str(),
                                         string_VkFormat(ivci.format));
                    }
                }
            }
            if (ivci.viewType == VK_IMAGE_VIEW_TYPE_3D) {
                LogObjectList objlist(pCreateInfo->renderPass, image_views[i]);
                skip |= LogError("VUID-VkFramebufferCreateInfo-flags-04113", objlist, attachment_loc,
                                 "has an image view type of VK_IMAGE_VIEW_TYPE_3D.");
            }
        }
    } else if (framebuffer_attachments_create_info) {
        // VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT is set
        for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
            const Location attachment_loc = create_info_loc.dot(Field::pAttachments, i);
            auto &aii = framebuffer_attachments_create_info->pAttachmentImageInfos[i];
            bool format_found = false;
            for (uint32_t j = 0; j < aii.viewFormatCount; ++j) {
                if (aii.pViewFormats[j] == rpci->pAttachments[i].format) {
                    format_found = true;
                }
            }
            if (!format_found) {
                skip |= LogError("VUID-VkFramebufferCreateInfo-flags-03205", pCreateInfo->renderPass,
                                 create_info_loc.pNext(Struct::VkFramebufferAttachmentsCreateInfo, Field::pAttachmentImageInfos, i)
                                     .dot(Field::pViewFormats),
                                 "does not include format %s used by the corresponding attachment for renderPass (%s).",
                                 string_VkFormat(rpci->pAttachments[i].format), FormatHandle(pCreateInfo->renderPass).c_str());
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

                if (enabled_features.attachmentFragmentShadingRate) {
                    const auto *fsr_attachment = vku::FindStructInPNextChain<VkFragmentShadingRateAttachmentInfoKHR>(subpass.pNext);
                    if (fsr_attachment && fsr_attachment->pFragmentShadingRateAttachment->attachment == i) {
                        used_as_fragment_shading_rate_attachment = true;
                        if ((aii.width * fsr_attachment->shadingRateAttachmentTexelSize.width) < pCreateInfo->width) {
                            skip |=
                                LogError("VUID-VkFramebufferCreateInfo-flags-04543", pCreateInfo->renderPass, attachment_loc,
                                         "is used as a fragment shading rate attachment in subpass %" PRIu32
                                         ", but the product of its width (%" PRIu32
                                         ") and the "
                                         "specified shading rate texel width (%" PRIu32
                                         ") are smaller than the corresponding framebuffer "
                                         "width (%" PRIu32 ").",
                                         j, aii.width, fsr_attachment->shadingRateAttachmentTexelSize.width, pCreateInfo->width);
                        }
                        if ((aii.height * fsr_attachment->shadingRateAttachmentTexelSize.height) < pCreateInfo->height) {
                            skip |=
                                LogError("VUID-VkFramebufferCreateInfo-flags-04544", pCreateInfo->renderPass, attachment_loc,
                                         "is used as a fragment shading rate attachment in subpass %" PRIu32
                                         ", but the product of its "
                                         "height (%" PRIu32
                                         ") and the "
                                         "specified shading rate texel height (%" PRIu32
                                         ") are smaller than the corresponding "
                                         "framebuffer height (%" PRIu32 ").",
                                         j, aii.height, fsr_attachment->shadingRateAttachmentTexelSize.height, pCreateInfo->height);
                        }
                        if (highest_view_bit >= 0) {
                            fsr_non_zero_viewmasks = true;
                        }
                        if (aii.layerCount != 1 && static_cast<int32_t>(aii.layerCount) <= highest_view_bit) {
                            skip |=
                                LogError("VUID-VkFramebufferCreateInfo-renderPass-08921", pCreateInfo->renderPass, attachment_loc,
                                         "has a layer count (%" PRIu32
                                         ") "
                                         "less than or equal to the highest bit in the view mask (%i) of subpass %" PRIu32 ".",
                                         aii.layerCount, highest_view_bit, j);
                        }
                    }
                }
            }

            if (used_as_input_color_resolve_depth_stencil_attachment) {
                if (aii.width < pCreateInfo->width) {
                    skip |= LogError("VUID-VkFramebufferCreateInfo-flags-04541", pCreateInfo->renderPass, attachment_loc,
                                     "has a width of only %" PRIu32 ", but framebuffer has a width of %" PRIu32 ".", aii.width,
                                     pCreateInfo->width);
                }

                if (aii.height < pCreateInfo->height) {
                    skip |= LogError("VUID-VkFramebufferCreateInfo-flags-04542", pCreateInfo->renderPass, attachment_loc,
                                     "has a height of only %" PRIu32 ", but framebuffer has a height of %" PRIu32 ".", aii.height,
                                     pCreateInfo->height);
                }

                if ((rpci->subpassCount == 0) || (rpci->pSubpasses[0].viewMask == 0)) {
                    if (aii.layerCount < pCreateInfo->layers) {
                        skip |= LogError("VUID-VkFramebufferCreateInfo-renderPass-04546", pCreateInfo->renderPass, attachment_loc,
                                         "has only %" PRIu32 " layers, but framebuffer has %" PRIu32 " layers.", aii.layerCount,
                                         pCreateInfo->layers);
                    }
                }
            }

            if (used_as_fragment_shading_rate_attachment && !fsr_non_zero_viewmasks) {
                if (aii.layerCount != 1 && aii.layerCount < pCreateInfo->layers) {
                    skip |= LogError("VUID-VkFramebufferCreateInfo-flags-04545", pCreateInfo->renderPass, attachment_loc,
                                     "has a layer count (%" PRIu32
                                     ") smaller than the corresponding framebuffer layer count (%" PRIu32 ").",
                                     aii.layerCount, pCreateInfo->layers);
                }
            }
        }

        // Validate image usage
        uint32_t attachment_index = VK_ATTACHMENT_UNUSED;
        for (uint32_t i = 0; i < rpci->subpassCount; ++i) {
            skip |= MatchUsage(rpci->pSubpasses[i].colorAttachmentCount, rpci->pSubpasses[i].pColorAttachments, pCreateInfo,
                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, "VUID-VkFramebufferCreateInfo-flags-03201", create_info_loc);
            skip |= MatchUsage(rpci->pSubpasses[i].colorAttachmentCount, rpci->pSubpasses[i].pResolveAttachments, pCreateInfo,
                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, "VUID-VkFramebufferCreateInfo-flags-03201", create_info_loc);
            skip |=
                MatchUsage(1, rpci->pSubpasses[i].pDepthStencilAttachment, pCreateInfo, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                           "VUID-VkFramebufferCreateInfo-flags-03202", create_info_loc);
            skip |= MatchUsage(rpci->pSubpasses[i].inputAttachmentCount, rpci->pSubpasses[i].pInputAttachments, pCreateInfo,
                               VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, "VUID-VkFramebufferCreateInfo-flags-03204", create_info_loc);

            const auto *depth_stencil_resolve =
                vku::FindStructInPNextChain<VkSubpassDescriptionDepthStencilResolve>(rpci->pSubpasses[i].pNext);
            if (depth_stencil_resolve != nullptr) {
                skip |= MatchUsage(1, depth_stencil_resolve->pDepthStencilResolveAttachment, pCreateInfo,
                                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, "VUID-VkFramebufferCreateInfo-flags-03203",
                                   create_info_loc);
            }

            const auto *fragment_shading_rate_attachment_info =
                vku::FindStructInPNextChain<VkFragmentShadingRateAttachmentInfoKHR>(rpci->pSubpasses[i].pNext);
            if (enabled_features.attachmentFragmentShadingRate &&
                fragment_shading_rate_attachment_info != nullptr) {
                skip |= MatchUsage(1, fragment_shading_rate_attachment_info->pFragmentShadingRateAttachment, pCreateInfo,
                                   VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
                                   "VUID-VkFramebufferCreateInfo-flags-04549", create_info_loc);
            }
        }

        // VUID-VkSubpassDescription2-multiview-06558 forces viewMask to be zero if not using multiView
        if ((rpci->subpassCount > 0) && (rpci->pSubpasses[0].viewMask != 0)) {
            for (uint32_t i = 0; i < rpci->subpassCount; ++i) {
                const auto *depth_stencil_resolve =
                    vku::FindStructInPNextChain<VkSubpassDescriptionDepthStencilResolve>(rpci->pSubpasses[i].pNext);
                uint32_t view_bits = rpci->pSubpasses[i].viewMask;
                int highest_view_bit = MostSignificantBit(view_bits);

                for (uint32_t j = 0; j < rpci->pSubpasses[i].colorAttachmentCount; ++j) {
                    attachment_index = rpci->pSubpasses[i].pColorAttachments[j].attachment;
                    if (attachment_index != VK_ATTACHMENT_UNUSED) {
                        int32_t layer_count =
                            framebuffer_attachments_create_info->pAttachmentImageInfos[attachment_index].layerCount;
                        if (layer_count <= highest_view_bit) {
                            skip |= LogError("VUID-VkFramebufferCreateInfo-renderPass-03198", pCreateInfo->renderPass,
                                             create_info_loc
                                                 .pNext(Struct::VkFramebufferAttachmentsCreateInfo, Field::pAttachmentImageInfos,
                                                        attachment_index)
                                                 .dot(Field::layerCount),
                                             "is %" PRId32 ", but the view mask for subpass %" PRIu32
                                             " in renderPass (%s) "
                                             "includes layer %i, with that attachment specified as a color attachment %" PRIu32 ".",
                                             layer_count, i, FormatHandle(pCreateInfo->renderPass).c_str(), highest_view_bit, j);
                        }
                    }
                    if (rpci->pSubpasses[i].pResolveAttachments) {
                        attachment_index = rpci->pSubpasses[i].pResolveAttachments[j].attachment;
                        if (attachment_index != VK_ATTACHMENT_UNUSED) {
                            int32_t layer_count =
                                framebuffer_attachments_create_info->pAttachmentImageInfos[attachment_index].layerCount;
                            if (layer_count <= highest_view_bit) {
                                skip |= LogError(
                                    "VUID-VkFramebufferCreateInfo-renderPass-03198", pCreateInfo->renderPass,
                                    create_info_loc
                                        .pNext(Struct::VkFramebufferAttachmentsCreateInfo, Field::pAttachmentImageInfos,
                                               attachment_index)
                                        .dot(Field::layerCount),
                                    "is %" PRId32 ", but the view mask for subpass %" PRIu32
                                    " in renderPass (%s) "
                                    "includes layer %i, with that attachment specified as a resolve attachment %" PRIu32 ".",
                                    layer_count, i, FormatHandle(pCreateInfo->renderPass).c_str(), highest_view_bit, j);
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
                            skip |=
                                LogError("VUID-VkFramebufferCreateInfo-renderPass-03198", pCreateInfo->renderPass,
                                         create_info_loc
                                             .pNext(Struct::VkFramebufferAttachmentsCreateInfo, Field::pAttachmentImageInfos,
                                                    attachment_index)
                                             .dot(Field::layerCount),
                                         "is %" PRId32 ", but the view mask for subpass %" PRIu32
                                         " in renderPass (%s) "
                                         "includes layer %i, with that attachment specified as an input attachment %" PRIu32 ".",
                                         layer_count, i, FormatHandle(pCreateInfo->renderPass).c_str(), highest_view_bit, j);
                        }
                    }
                }

                if (rpci->pSubpasses[i].pDepthStencilAttachment != nullptr) {
                    attachment_index = rpci->pSubpasses[i].pDepthStencilAttachment->attachment;
                    if (attachment_index != VK_ATTACHMENT_UNUSED) {
                        int32_t layer_count =
                            framebuffer_attachments_create_info->pAttachmentImageInfos[attachment_index].layerCount;
                        if (layer_count <= highest_view_bit) {
                            skip |= LogError("VUID-VkFramebufferCreateInfo-renderPass-03198", pCreateInfo->renderPass,
                                             create_info_loc
                                                 .pNext(Struct::VkFramebufferAttachmentsCreateInfo, Field::pAttachmentImageInfos,
                                                        attachment_index)
                                                 .dot(Field::layerCount),
                                             "is %" PRId32 ", but the view mask for subpass %" PRIu32
                                             " in renderPass (%s) "
                                             "includes layer %i, with that attachment specified as a depth/stencil attachment.",
                                             layer_count, i, FormatHandle(pCreateInfo->renderPass).c_str(), highest_view_bit);
                        }
                    }

                    if (depth_stencil_resolve != nullptr && depth_stencil_resolve->pDepthStencilResolveAttachment != nullptr) {
                        attachment_index = depth_stencil_resolve->pDepthStencilResolveAttachment->attachment;
                        if (attachment_index != VK_ATTACHMENT_UNUSED) {
                            int32_t layer_count =
                                framebuffer_attachments_create_info->pAttachmentImageInfos[attachment_index].layerCount;
                            if (layer_count <= highest_view_bit) {
                                skip |= LogError("VUID-VkFramebufferCreateInfo-renderPass-03198", pCreateInfo->renderPass,
                                                 create_info_loc
                                                     .pNext(Struct::VkFramebufferAttachmentsCreateInfo,
                                                            Field::pAttachmentImageInfos, attachment_index)
                                                     .dot(Field::layerCount),
                                                 "is %" PRId32 ", but the view mask for subpass %" PRIu32
                                                 " in renderPass (%s) "
                                                 "includes layer %i, with that attachment specified as a depth/stencil resolve "
                                                 "attachment.",
                                                 layer_count, i, FormatHandle(pCreateInfo->renderPass).c_str(), highest_view_bit);
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
                vku::FindStructInPNextChain<VkMultisampledRenderToSingleSampledInfoEXT>(subpass_description.pNext);
            // Verify input attachments:
            skip |=
                MatchUsage(subpass_description.inputAttachmentCount, subpass_description.pInputAttachments, pCreateInfo,
                           VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, "VUID-VkFramebufferCreateInfo-pAttachments-00879", create_info_loc);
            // Verify color attachments:
            skip |=
                MatchUsage(subpass_description.colorAttachmentCount, subpass_description.pColorAttachments, pCreateInfo,
                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, "VUID-VkFramebufferCreateInfo-pAttachments-00877", create_info_loc);
            // Verify depth/stencil attachments:
            skip |=
                MatchUsage(1, subpass_description.pDepthStencilAttachment, pCreateInfo, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                           "VUID-VkFramebufferCreateInfo-pAttachments-02633", create_info_loc);
            // Verify depth/stecnil resolve
            const auto *ds_resolve =
                vku::FindStructInPNextChain<VkSubpassDescriptionDepthStencilResolve>(subpass_description.pNext);
            if (ds_resolve) {
                skip |= MatchUsage(1, ds_resolve->pDepthStencilResolveAttachment, pCreateInfo,
                                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, "VUID-VkFramebufferCreateInfo-pAttachments-02634",
                                   create_info_loc);
            }

            // Verify fragment shading rate attachments
            if (enabled_features.attachmentFragmentShadingRate) {
                const auto *fragment_shading_rate_attachment_info =
                    vku::FindStructInPNextChain<VkFragmentShadingRateAttachmentInfoKHR>(subpass_description.pNext);
                if (fragment_shading_rate_attachment_info) {
                    skip |= MatchUsage(1, fragment_shading_rate_attachment_info->pFragmentShadingRateAttachment, pCreateInfo,
                                       VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
                                       "VUID-VkFramebufferCreateInfo-flags-04548", create_info_loc);
                }
            }
            if (ms_rendered_to_single_sampled && ms_rendered_to_single_sampled->multisampledRenderToSingleSampledEnable) {
                skip |= MsRenderedToSingleSampledValidateFBAttachments(
                    subpass_description.inputAttachmentCount, subpass_description.pInputAttachments, pCreateInfo, rpci, subpass,
                    ms_rendered_to_single_sampled->rasterizationSamples, create_info_loc);
                skip |= MsRenderedToSingleSampledValidateFBAttachments(
                    subpass_description.colorAttachmentCount, subpass_description.pColorAttachments, pCreateInfo, rpci, subpass,
                    ms_rendered_to_single_sampled->rasterizationSamples, create_info_loc);
                if (subpass_description.pDepthStencilAttachment) {
                    skip |= MsRenderedToSingleSampledValidateFBAttachments(
                        1, subpass_description.pDepthStencilAttachment, pCreateInfo, rpci, subpass,
                        ms_rendered_to_single_sampled->rasterizationSamples, create_info_loc);
                }
            }
        }
    }

    if (b_has_non_zero_view_masks && pCreateInfo->layers != 1) {
        skip |=
            LogError("VUID-VkFramebufferCreateInfo-renderPass-02531", pCreateInfo->renderPass, create_info_loc.dot(Field::layers),
                     "is %" PRIu32 " but renderPass (%s) was specified with non-zero view masks.", pCreateInfo->layers,
                     FormatHandle(pCreateInfo->renderPass).c_str());
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                                                   const VkAllocationCallbacks *pAllocator, const ErrorObject &error_obj) const {
    auto framebuffer_state = Get<FRAMEBUFFER_STATE>(framebuffer);
    bool skip = false;
    if (framebuffer_state) {
        skip |= ValidateObjectNotInUse(framebuffer_state.get(), error_obj.location, "VUID-vkDestroyFramebuffer-framebuffer-00892");
    }
    return skip;
}

bool CoreChecks::ValidateInheritanceInfoFramebuffer(VkCommandBuffer primaryBuffer, const CMD_BUFFER_STATE &cb_state,
                                                    VkCommandBuffer secondaryBuffer, const CMD_BUFFER_STATE &sub_cb_state,
                                                    const Location &loc) const {
    bool skip = false;
    if (!sub_cb_state.beginInfo.pInheritanceInfo) {
        return skip;
    }
    VkFramebuffer primary_fb = cb_state.activeFramebuffer ? cb_state.activeFramebuffer->framebuffer() : VK_NULL_HANDLE;
    VkFramebuffer secondary_fb = sub_cb_state.beginInfo.pInheritanceInfo->framebuffer;
    if (secondary_fb != VK_NULL_HANDLE) {
        if (primary_fb != secondary_fb) {
            const LogObjectList objlist(primaryBuffer, secondaryBuffer, secondary_fb, primary_fb);
            skip |= LogError("VUID-vkCmdExecuteCommands-pCommandBuffers-00099", objlist, loc,
                             "called w/ invalid secondary %s which has a %s"
                             " that is not the same as the primary command buffer's current active %s.",
                             FormatHandle(secondaryBuffer).c_str(), FormatHandle(secondary_fb).c_str(),
                             FormatHandle(primary_fb).c_str());
        }
        auto fb = Get<FRAMEBUFFER_STATE>(secondary_fb);
        if (!fb) {
            const LogObjectList objlist(primaryBuffer, secondaryBuffer, secondary_fb);
            skip |= LogError(kVUID_Core_DrawState_InvalidSecondaryCommandBuffer, objlist, loc,
                             "called w/ invalid %s which has invalid %s.", FormatHandle(secondaryBuffer).c_str(),
                             FormatHandle(secondary_fb).c_str());
            return skip;
        }
    }
    return skip;
}
