/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
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

#include "stateless/stateless_validation.h"
#include "utils/convert_utils.h"

bool StatelessValidation::ValidateSubpassGraphicsFlags(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                       uint32_t subpass, VkPipelineStageFlags2 stages, const char *vuid,
                                                       const Location &loc) const {
    bool skip = false;
    // make sure we consider all of the expanded and un-expanded graphics bits to be valid
    const auto kExcludeStages = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT_KHR | VK_PIPELINE_STAGE_2_COPY_BIT_KHR |
                                VK_PIPELINE_STAGE_2_RESOLVE_BIT_KHR | VK_PIPELINE_STAGE_2_BLIT_BIT_KHR |
                                VK_PIPELINE_STAGE_2_CLEAR_BIT_KHR;
    const auto kMetaGraphicsStages = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR | VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR |
                                     VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT_KHR;
    const auto kGraphicsStages =
        (sync_utils::ExpandPipelineStages(VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_QUEUE_GRAPHICS_BIT) | kMetaGraphicsStages) &
        ~kExcludeStages;

    const auto IsPipeline = [pCreateInfo](uint32_t subpass, const VkPipelineBindPoint stage) {
        if (subpass == VK_SUBPASS_EXTERNAL || subpass >= pCreateInfo->subpassCount)
            return false;
        else
            return pCreateInfo->pSubpasses[subpass].pipelineBindPoint == stage;
    };

    const bool is_all_graphics_stages = (stages & ~kGraphicsStages) == 0;
    if (IsPipeline(subpass, VK_PIPELINE_BIND_POINT_GRAPHICS) && !is_all_graphics_stages) {
        skip |= LogError(vuid, device, loc,
                         "dependency contains a stage mask (%s) that are not part "
                         "of the Graphics pipeline",
                         sync_utils::StringPipelineStageFlags(stages & ~kGraphicsStages).c_str());
    }

    return skip;
}

bool StatelessValidation::ValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                   const ErrorObject &error_obj) const {
    bool skip = false;
    uint32_t max_color_attachments = device_limits.maxColorAttachments;
    const bool use_rp2 = error_obj.location.function != Func::vkCreateRenderPass;
    const char *vuid = nullptr;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    VkBool32 separate_depth_stencil_layouts = false;
    const auto *vulkan_12_features = vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(device_createinfo_pnext);
    if (vulkan_12_features) {
        separate_depth_stencil_layouts = vulkan_12_features->separateDepthStencilLayouts;
    } else {
        const auto *separate_depth_stencil_layouts_features =
            vku::FindStructInPNextChain<VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures>(device_createinfo_pnext);
        if (separate_depth_stencil_layouts_features) {
            separate_depth_stencil_layouts = separate_depth_stencil_layouts_features->separateDepthStencilLayouts;
        }
    }

    VkBool32 attachment_feedback_loop_layout = false;
    const auto *attachment_feedback_loop_layout_features =
        vku::FindStructInPNextChain<VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT>(device_createinfo_pnext);
    if (attachment_feedback_loop_layout_features) {
        attachment_feedback_loop_layout = attachment_feedback_loop_layout_features->attachmentFeedbackLoopLayout;
    }

    VkBool32 synchronization2 = false;
    const auto *vulkan_13_features = vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Features>(device_createinfo_pnext);
    if (vulkan_13_features) {
        synchronization2 = vulkan_13_features->synchronization2;
    } else {
        const auto *synchronization2_features = vku::FindStructInPNextChain<VkPhysicalDeviceSynchronization2Features>(device_createinfo_pnext);
        if (synchronization2_features) {
            synchronization2 = synchronization2_features->synchronization2;
        }
    }
    for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
        const Location &attachment_loc = create_info_loc.dot(Field::pAttachments, i);

        // if not null, also confirms rp2 is being used
        const void *pNext =
            (use_rp2) ? reinterpret_cast<VkAttachmentDescription2 const *>(&pCreateInfo->pAttachments[i])->pNext : nullptr;
        const auto *attachment_description_stencil_layout =
            (use_rp2) ? vku::FindStructInPNextChain<VkAttachmentDescriptionStencilLayout>(pNext) : nullptr;

        const VkFormat attachment_format = pCreateInfo->pAttachments[i].format;
        const VkImageLayout initial_layout = pCreateInfo->pAttachments[i].initialLayout;
        const VkImageLayout final_layout = pCreateInfo->pAttachments[i].finalLayout;
        if (attachment_format == VK_FORMAT_UNDEFINED && GetExternalFormat(pNext) == 0) {
            vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-09334" : "VUID-VkAttachmentDescription-format-06698";
            skip |= LogError(vuid, device, attachment_loc.dot(Field::format), "is VK_FORMAT_UNDEFINED.");
        }
        if (final_layout == VK_IMAGE_LAYOUT_UNDEFINED || final_layout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
            vuid = use_rp2 ? "VUID-VkAttachmentDescription2-finalLayout-00843" : "VUID-VkAttachmentDescription-finalLayout-00843";
            skip |= LogError(vuid, device, attachment_loc.dot(Field::finalLayout), "is %s.", string_VkImageLayout(final_layout));
        }
        if (!separate_depth_stencil_layouts) {
            if (IsImageLayoutDepthOnly(initial_layout) || IsImageLayoutStencilOnly(initial_layout)) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03284"
                               : "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03284";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout), "is %s.",
                                 string_VkImageLayout(initial_layout));
            }
            if (IsImageLayoutDepthOnly(final_layout) || IsImageLayoutStencilOnly(final_layout)) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03285"
                               : "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03285";
                skip |=
                    LogError(vuid, device, attachment_loc.dot(Field::finalLayout), "is %s.", string_VkImageLayout(final_layout));
            }
        }
        if (!attachment_feedback_loop_layout) {
            if (initial_layout == VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-attachmentFeedbackLoopLayout-07309"
                               : "VUID-VkAttachmentDescription-attachmentFeedbackLoopLayout-07309";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout),
                                 "is VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT but the "
                                 "attachmentFeedbackLoopLayout feature is not enabled.");
            }
            if (final_layout == VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-attachmentFeedbackLoopLayout-07310"
                               : "VUID-VkAttachmentDescription-attachmentFeedbackLoopLayout-07310";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::finalLayout),
                                 "is VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT but the "
                                 "attachmentFeedbackLoopLayout feature is not enabled.");
            }
        }
        if (!synchronization2) {
            if (initial_layout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR ||
                initial_layout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-synchronization2-06908"
                               : "VUID-VkAttachmentDescription-synchronization2-06908";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout),
                                 "is %s but the synchronization2 feature is not enabled.", string_VkImageLayout(initial_layout));
            }
            if (final_layout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR || final_layout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-synchronization2-06909"
                               : "VUID-VkAttachmentDescription-synchronization2-06909";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::finalLayout),
                                 "is %s but the synchronization2 feature is not enabled.", string_VkImageLayout(final_layout));
            }
        }
        if (!vkuFormatIsDepthOrStencil(attachment_format)) {  // color format
            if (IsImageLayoutDepthOnly(initial_layout) || IsImageLayoutStencilOnly(initial_layout)) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03286" : "VUID-VkAttachmentDescription-format-03286";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout), "is %s.",
                                 string_VkImageLayout(initial_layout));
            }
            if (IsImageLayoutDepthOnly(final_layout) || IsImageLayoutStencilOnly(final_layout)) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03287" : "VUID-VkAttachmentDescription-format-03287";
                skip |=
                    LogError(vuid, device, attachment_loc.dot(Field::finalLayout), "is %s.", string_VkImageLayout(final_layout));
            }
        } else if (vkuFormatIsDepthAndStencil(attachment_format)) {
            if (IsImageLayoutStencilOnly(initial_layout)) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06906" : "VUID-VkAttachmentDescription-format-06906";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout), "is %s.",
                                 string_VkImageLayout(initial_layout));
            }
            if (IsImageLayoutStencilOnly(final_layout)) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06907" : "VUID-VkAttachmentDescription-format-06907";
                skip |=
                    LogError(vuid, device, attachment_loc.dot(Field::finalLayout), "is %s.", string_VkImageLayout(final_layout));
            }

            if (!attachment_description_stencil_layout) {
                if (IsImageLayoutDepthOnly(initial_layout)) {
                    vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06249" : "VUID-VkAttachmentDescription-format-06242";
                    skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout),
                                     "is %s but no VkAttachmentDescriptionStencilLayout provided.",
                                     string_VkImageLayout(initial_layout));
                }
                if (IsImageLayoutDepthOnly(final_layout)) {
                    vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06250" : "VUID-VkAttachmentDescription-format-06243";
                    skip |=
                        LogError(vuid, device, attachment_loc.dot(Field::finalLayout),
                                 "is %s but no VkAttachmentDescriptionStencilLayout provided.", string_VkImageLayout(final_layout));
                }
            }
        } else if (vkuFormatIsDepthOnly(attachment_format)) {
            if (IsImageLayoutStencilOnly(initial_layout)) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03290" : "VUID-VkAttachmentDescription-format-03290";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout), "is %s.",
                                 string_VkImageLayout(initial_layout));
            }
            if (IsImageLayoutStencilOnly(final_layout)) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03291" : "VUID-VkAttachmentDescription-format-03291";
                skip |=
                    LogError(vuid, device, attachment_loc.dot(Field::finalLayout), "is %s.", string_VkImageLayout(final_layout));
            }
        } else if (vkuFormatIsStencilOnly(attachment_format) && !attachment_description_stencil_layout) {
            if (IsImageLayoutDepthOnly(initial_layout)) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06247" : "VUID-VkAttachmentDescription-format-03292";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout), "is %s.",
                                 string_VkImageLayout(initial_layout));
            }
            if (IsImageLayoutDepthOnly(final_layout)) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06248" : "VUID-VkAttachmentDescription-format-03293";
                skip |=
                    LogError(vuid, device, attachment_loc.dot(Field::finalLayout), "is %s.", string_VkImageLayout(final_layout));
            }
        }
        if (attachment_description_stencil_layout) {
            const VkImageLayout stencil_initial_layout = attachment_description_stencil_layout->stencilInitialLayout;
            const VkImageLayout stencil_final_layout = attachment_description_stencil_layout->stencilFinalLayout;

            if (stencil_initial_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
                stencil_initial_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                stencil_initial_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL ||
                stencil_initial_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                stencil_initial_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL ||
                stencil_initial_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
                stencil_initial_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) {
                skip |= LogError("VUID-VkAttachmentDescriptionStencilLayout-stencilInitialLayout-03308", device,
                                 attachment_loc.pNext(Struct::VkAttachmentDescriptionStencilLayout, Field::stencilInitialLayout),
                                 "is %s.", string_VkImageLayout(stencil_initial_layout));
            }
            if (stencil_final_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
                stencil_final_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                stencil_final_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL ||
                stencil_final_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                stencil_final_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL ||
                stencil_final_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
                stencil_final_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) {
                skip |= LogError("VUID-VkAttachmentDescriptionStencilLayout-stencilFinalLayout-03309", device,
                                 attachment_loc.pNext(Struct::VkAttachmentDescriptionStencilLayout, Field::stencilFinalLayout),
                                 "is %s.", string_VkImageLayout(stencil_final_layout));
            }
            if (stencil_final_layout == VK_IMAGE_LAYOUT_UNDEFINED || stencil_final_layout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
                skip |= LogError("VUID-VkAttachmentDescriptionStencilLayout-stencilFinalLayout-03310", device,
                                 attachment_loc.pNext(Struct::VkAttachmentDescriptionStencilLayout, Field::stencilFinalLayout),
                                 "is %s.", string_VkImageLayout(stencil_final_layout));
            }
        }

        if (vkuFormatIsDepthOrStencil(attachment_format)) {
            if (initial_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03281" : "VUID-VkAttachmentDescription-format-03281";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout),
                                 "must not be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL when using a Depth or Stencil format (%s)",
                                 string_VkFormat(attachment_format));
            }
            if (final_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03283" : "VUID-VkAttachmentDescription-format-03283";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::finalLayout),
                                 "must not be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL when using a Depth or Stencil format (%s)",
                                 string_VkFormat(attachment_format));
            }
        }
        if (vkuFormatIsColor(attachment_format)) {
            if (initial_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                initial_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03280" : "VUID-VkAttachmentDescription-format-03280";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout), "is %s, but using a Color format (%s)",
                                 string_VkImageLayout(initial_layout), string_VkFormat(attachment_format));

            } else if (initial_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
                       initial_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06487" : "VUID-VkAttachmentDescription-format-06487";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::initialLayout), "is %s, but using a Color format (%s)",
                                 string_VkImageLayout(initial_layout), string_VkFormat(attachment_format));
            }
            if (final_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                final_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-03282" : "VUID-VkAttachmentDescription-format-03282";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::finalLayout), "is %s, but using a Color format (%s)",
                                 string_VkImageLayout(final_layout), string_VkFormat(attachment_format));
            } else if (final_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
                       final_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06488" : "VUID-VkAttachmentDescription-format-06488";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::finalLayout), "is %s, but using a Color format (%s)",
                                 string_VkImageLayout(final_layout), string_VkFormat(attachment_format));
            }
        }
        if (vkuFormatIsColor(attachment_format) || vkuFormatHasDepth(attachment_format)) {
            if (pCreateInfo->pAttachments[i].loadOp == VK_ATTACHMENT_LOAD_OP_LOAD && initial_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-06699" : "VUID-VkAttachmentDescription-format-06699";
                skip |= LogError(
                    vuid, device, attachment_loc,
                    "format is %s and loadOp is VK_ATTACHMENT_LOAD_OP_LOAD, but initialLayout is VK_IMAGE_LAYOUT_UNDEFINED.",
                    string_VkFormat(attachment_format));
            }
        }
        if (vkuFormatHasStencil(attachment_format) && pCreateInfo->pAttachments[i].stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
            if (initial_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-pNext-06704" : "VUID-VkAttachmentDescription-format-06700";
                skip |= LogError(vuid, device, attachment_loc,
                                 "format (%s) includes stencil aspect and stencilLoadOp is VK_ATTACHMENT_LOAD_OP_LOAD, but "
                                 "the initialLayout is VK_IMAGE_LAYOUT_UNDEFINED.",
                                 string_VkFormat(attachment_format));
            }

            // rp2 can have seperate depth/stencil layout and need to look in pNext
            if (attachment_description_stencil_layout) {
                if (attachment_description_stencil_layout->stencilInitialLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
                    skip |=
                        LogError("VUID-VkAttachmentDescription2-pNext-06705", device, attachment_loc,
                                 "format includes stencil aspect and stencilLoadOp is VK_ATTACHMENT_LOAD_OP_LOAD, but "
                                 "the VkAttachmentDescriptionStencilLayout::stencilInitialLayout is VK_IMAGE_LAYOUT_UNDEFINED.");
                }
            }
        }
    }

    for (uint32_t i = 0; i < pCreateInfo->subpassCount; ++i) {
        if (pCreateInfo->pSubpasses[i].colorAttachmentCount > max_color_attachments) {
            vuid = use_rp2 ? "VUID-VkSubpassDescription2-colorAttachmentCount-03063"
                           : "VUID-VkSubpassDescription-colorAttachmentCount-00845";
            skip |= LogError(vuid, device, create_info_loc.dot(Field::pSubpasses, i),
                             "cannot be used to create a render pass. maxColorAttachments is %d.", max_color_attachments);
        }
    }

    for (uint32_t i = 0; i < pCreateInfo->dependencyCount; ++i) {
        const auto &dependency = pCreateInfo->pDependencies[i];

        // Need to check first so layer doesn't segfault from out of bound array access
        // src subpass bound check
        if ((dependency.srcSubpass != VK_SUBPASS_EXTERNAL) && (dependency.srcSubpass >= pCreateInfo->subpassCount)) {
            vuid = use_rp2 ? "VUID-VkRenderPassCreateInfo2-srcSubpass-02526" : "VUID-VkRenderPassCreateInfo-pDependencies-06866";
            skip |= LogError(vuid, device, create_info_loc.dot(Field::pDependencies, i).dot(Field::srcSubpass),
                             "index (%" PRIu32 ") has to be less than subpassCount (%" PRIu32 ")", dependency.srcSubpass,
                             pCreateInfo->subpassCount);
        }

        // dst subpass bound check
        if ((dependency.dstSubpass != VK_SUBPASS_EXTERNAL) && (dependency.dstSubpass >= pCreateInfo->subpassCount)) {
            vuid = use_rp2 ? "VUID-VkRenderPassCreateInfo2-dstSubpass-02527" : "VUID-VkRenderPassCreateInfo-pDependencies-06867";
            skip |= LogError(vuid, device, create_info_loc.dot(Field::pDependencies, i).dot(Field::dstSubpass),
                             "index (%" PRIu32 ") has to be less than subpassCount (%" PRIu32 ")", dependency.dstSubpass,
                             pCreateInfo->subpassCount);
        }

        VkPipelineStageFlags2 srcStageMask = dependency.srcStageMask;
        VkPipelineStageFlags2 dstStageMask = dependency.dstStageMask;
        if (const auto barrier = vku::FindStructInPNextChain<VkMemoryBarrier2KHR>(pCreateInfo->pDependencies[i].pNext); barrier) {
            srcStageMask = barrier->srcStageMask;
            dstStageMask = barrier->dstStageMask;
        }

        // Spec currently only supports Graphics pipeline in render pass -- so only that pipeline is currently checked
        vuid = use_rp2 ? "VUID-VkRenderPassCreateInfo2-pDependencies-03054" : "VUID-VkRenderPassCreateInfo-pDependencies-00837";
        skip |= ValidateSubpassGraphicsFlags(device, pCreateInfo, dependency.srcSubpass, srcStageMask, vuid,
                                             create_info_loc.dot(Field::pDependencies, i).dot(Field::srcSubpass));

        vuid = use_rp2 ? "VUID-VkRenderPassCreateInfo2-pDependencies-03055" : "VUID-VkRenderPassCreateInfo-pDependencies-00838";
        skip |= ValidateSubpassGraphicsFlags(device, pCreateInfo, dependency.dstSubpass, dstStageMask, vuid,
                                             create_info_loc.dot(Field::pDependencies, i).dot(Field::dstSubpass));
    }

    return skip;
}

bool StatelessValidation::manual_PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                                 const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                                 const ErrorObject &error_obj) const {
    safe_VkRenderPassCreateInfo2 create_info_2 = ConvertVkRenderPassCreateInfoToV2KHR(*pCreateInfo);
    return ValidateCreateRenderPass(device, create_info_2.ptr(), pAllocator, pRenderPass, error_obj);
}

bool StatelessValidation::manual_PreCallValidateCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                                  const VkAllocationCallbacks *pAllocator,
                                                                  VkRenderPass *pRenderPass, const ErrorObject &error_obj) const {
    safe_VkRenderPassCreateInfo2 create_info_2(pCreateInfo);
    return ValidateCreateRenderPass(device, create_info_2.ptr(), pAllocator, pRenderPass, error_obj);
}

bool StatelessValidation::manual_PreCallValidateCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkRenderPass *pRenderPass,
                                                                     const ErrorObject &error_obj) const {
    return manual_PreCallValidateCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass, error_obj);
}

void StatelessValidation::RecordRenderPass(VkRenderPass renderPass, const VkRenderPassCreateInfo2 *pCreateInfo) {
    std::unique_lock<std::mutex> lock(renderpass_map_mutex);
    auto &renderpass_state = renderpasses_states[renderPass];
    lock.unlock();

    renderpass_state.subpasses_flags.resize(pCreateInfo->subpassCount);
    for (uint32_t subpass = 0; subpass < pCreateInfo->subpassCount; ++subpass) {
        bool uses_color = false;
        renderpass_state.color_attachment_count = pCreateInfo->pSubpasses[subpass].colorAttachmentCount;

        for (uint32_t i = 0; i < pCreateInfo->pSubpasses[subpass].colorAttachmentCount && !uses_color; ++i)
            if (pCreateInfo->pSubpasses[subpass].pColorAttachments[i].attachment != VK_ATTACHMENT_UNUSED) uses_color = true;

        bool uses_depthstencil = false;
        if (pCreateInfo->pSubpasses[subpass].pDepthStencilAttachment)
            if (pCreateInfo->pSubpasses[subpass].pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED)
                uses_depthstencil = true;

        if (uses_color) renderpass_state.subpasses_using_color_attachment.insert(subpass);
        if (uses_depthstencil) renderpass_state.subpasses_using_depthstencil_attachment.insert(subpass);
        renderpass_state.subpasses_flags[subpass] = pCreateInfo->pSubpasses[subpass].flags;
    }
}
void StatelessValidation::PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                         const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    safe_VkRenderPassCreateInfo2 create_info_2 = ConvertVkRenderPassCreateInfoToV2KHR(*pCreateInfo);
    RecordRenderPass(*pRenderPass, create_info_2.ptr());
}

void StatelessValidation::PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                             const RecordObject &record_obj) {
    // Track the state necessary for checking vkCreateGraphicsPipeline (subpass usage of depth and color attachments)
    if (record_obj.result != VK_SUCCESS) return;
    safe_VkRenderPassCreateInfo2 create_info_2(pCreateInfo);
    RecordRenderPass(*pRenderPass, create_info_2.ptr());
}

void StatelessValidation::PostCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                                                          const VkAllocationCallbacks *pAllocator, const RecordObject &record_obj) {
    // Track the state necessary for checking vkCreateGraphicsPipeline (subpass usage of depth and color attachments)
    std::unique_lock<std::mutex> lock(renderpass_map_mutex);
    renderpasses_states.erase(renderPass);
}
