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

    VkBool32 android_external_format_resolve_feature = false;
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    const auto *external_format_resolve_features =
        vku::FindStructInPNextChain<VkPhysicalDeviceExternalFormatResolveFeaturesANDROID>(device_createinfo_pnext);
    if (external_format_resolve_features) {
        android_external_format_resolve_feature = external_format_resolve_features->externalFormatResolve;
    }
#endif

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
        if (attachment_format == VK_FORMAT_UNDEFINED) {
            if (use_rp2 && android_external_format_resolve_feature) {
                if (GetExternalFormat(pNext) == 0) {
                    skip |= LogError("VUID-VkAttachmentDescription2-format-09334", device, attachment_loc.dot(Field::format),
                                     "is VK_FORMAT_UNDEFINED.");
                }
            } else {
                vuid = use_rp2 ? "VUID-VkAttachmentDescription2-format-09332" : "VUID-VkAttachmentDescription-format-06698";
                skip |= LogError(vuid, device, attachment_loc.dot(Field::format), "is VK_FORMAT_UNDEFINED.");
            }
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

bool StatelessValidation::ValidateCmdBeginRenderPass(const VkRenderPassBeginInfo *const rp_begin,
                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    if ((rp_begin->clearValueCount != 0) && !rp_begin->pClearValues) {
        skip |= LogError("VUID-VkRenderPassBeginInfo-clearValueCount-04962", rp_begin->renderPass,
                         error_obj.location.dot(Field::pRenderPassBegin).dot(Field::clearValueCount),
                         "(%" PRIu32 ") is not zero, but pRenderPassBegin->pClearValues is NULL.", rp_begin->clearValueCount);
    }
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBeginRenderPass(VkCommandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                                   VkSubpassContents, const ErrorObject &error_obj) const {
    bool skip = ValidateCmdBeginRenderPass(pRenderPassBegin, error_obj);
    return skip;
}

bool StatelessValidation::manual_PreCallValidateCmdBeginRenderPass2(VkCommandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                                    const VkSubpassBeginInfo *,
                                                                    const ErrorObject &error_obj) const {
    bool skip = ValidateCmdBeginRenderPass(pRenderPassBegin, error_obj);
    return skip;
}

static bool UniqueRenderingInfoImageViews(const VkRenderingInfo *pRenderingInfo, VkImageView imageView) {
    bool unique_views = true;
    for (uint32_t i = 0; i < pRenderingInfo->colorAttachmentCount; ++i) {
        if (pRenderingInfo->pColorAttachments[i].imageView == imageView) {
            unique_views = false;
        }

        if (pRenderingInfo->pColorAttachments[i].resolveImageView == imageView) {
            unique_views = false;
        }
    }

    if (pRenderingInfo->pDepthAttachment) {
        if (pRenderingInfo->pDepthAttachment->imageView == imageView) {
            unique_views = false;
        }

        if (pRenderingInfo->pDepthAttachment->resolveImageView == imageView) {
            unique_views = false;
        }
    }

    if (pRenderingInfo->pStencilAttachment) {
        if (pRenderingInfo->pStencilAttachment->imageView == imageView) {
            unique_views = false;
        }

        if (pRenderingInfo->pStencilAttachment->resolveImageView == imageView) {
            unique_views = false;
        }
    }
    return unique_views;
}

bool StatelessValidation::manual_PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer,
                                                                  const VkRenderingInfo *pRenderingInfo,
                                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    const Location rendering_info_loc = error_obj.location.dot(Field::pRenderingInfo);

    if (pRenderingInfo->viewMask == 0 && pRenderingInfo->layerCount == 0) {
        skip |= LogError("VUID-VkRenderingInfo-viewMask-06069", commandBuffer, rendering_info_loc,
                         "viewMask and layerCount are both zero");
    }

    if (pRenderingInfo->colorAttachmentCount > device_limits.maxColorAttachments) {
        skip |= LogError("VUID-VkRenderingInfo-colorAttachmentCount-06106", commandBuffer,
                         rendering_info_loc.dot(Field::colorAttachmentCount),
                         "(%" PRIu32
                         ") must be less than or equal to "
                         "maxColorAttachments (%" PRIu32 ").",
                         pRenderingInfo->colorAttachmentCount, device_limits.maxColorAttachments);
    }

    const auto rendering_fragment_shading_rate_attachment_info =
        vku::FindStructInPNextChain<VkRenderingFragmentShadingRateAttachmentInfoKHR>(pRenderingInfo->pNext);
    if (rendering_fragment_shading_rate_attachment_info &&
        (rendering_fragment_shading_rate_attachment_info->imageView != VK_NULL_HANDLE)) {
        if (UniqueRenderingInfoImageViews(pRenderingInfo, rendering_fragment_shading_rate_attachment_info->imageView) == false) {
            skip |= LogError("VUID-VkRenderingInfo-imageView-06125", commandBuffer,
                             rendering_info_loc.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::imageView),
                             "is %s.", FormatHandle(rendering_fragment_shading_rate_attachment_info->imageView).c_str());
        }

        const VkImageLayout image_layout = rendering_fragment_shading_rate_attachment_info->imageLayout;
        if (image_layout != VK_IMAGE_LAYOUT_GENERAL &&
            image_layout != VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR) {
            skip |= LogError("VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06147", commandBuffer,
                             rendering_info_loc.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::layout),
                             "is (%s).", string_VkImageLayout(image_layout));
        }

        if (!IsPowerOfTwo(rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width)) {
            skip |=
                LogError("VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06149", commandBuffer,
                         rendering_info_loc
                             .pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::shadingRateAttachmentTexelSize)
                             .dot(Field::width),
                         "(%" PRIu32 ") must be a power of two.",
                         rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width);
        }

        const uint32_t max_frs_attach_texel_width =
            phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSize.width;
        if (rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width > max_frs_attach_texel_width) {
            skip |= LogError(
                "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06150", commandBuffer,
                rendering_info_loc
                    .pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::shadingRateAttachmentTexelSize)
                    .dot(Field::width),
                "(%" PRIu32
                ") must be less than or equal to "
                "maxFragmentShadingRateAttachmentTexelSize.width (%" PRIu32 ").",
                rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width, max_frs_attach_texel_width);
        }

        const uint32_t min_frs_attach_texel_width =
            phys_dev_ext_props.fragment_shading_rate_props.minFragmentShadingRateAttachmentTexelSize.width;
        if (rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width < min_frs_attach_texel_width) {
            skip |= LogError(
                "VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06151", commandBuffer,
                rendering_info_loc
                    .pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::shadingRateAttachmentTexelSize)
                    .dot(Field::width),
                "(%" PRIu32
                ") must be greater than or equal to "
                "minFragmentShadingRateAttachmentTexelSize.width (%" PRIu32 ").",
                rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width, min_frs_attach_texel_width);
        }

        if (!IsPowerOfTwo(rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height)) {
            skip |=
                LogError("VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06152", commandBuffer,
                         rendering_info_loc
                             .pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::shadingRateAttachmentTexelSize)
                             .dot(Field::height),
                         "(%" PRIu32 ") must be a power of two.",
                         rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height);
        }

        const uint32_t max_frs_attach_texel_height =
            phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSize.height;
        if (rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height > max_frs_attach_texel_height) {
            skip |=
                LogError("VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06153", commandBuffer,
                         rendering_info_loc
                             .pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::shadingRateAttachmentTexelSize)
                             .dot(Field::height),
                         "(%" PRIu32
                         ") must be less than or equal to "
                         "maxFragmentShadingRateAttachmentTexelSize.height (%" PRIu32 ").",
                         rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height,
                         max_frs_attach_texel_height);
        }

        const uint32_t min_frs_attach_texel_height =
            phys_dev_ext_props.fragment_shading_rate_props.minFragmentShadingRateAttachmentTexelSize.height;
        if (rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height < min_frs_attach_texel_height) {
            skip |=
                LogError("VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06154", commandBuffer,
                         rendering_info_loc
                             .pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR, Field::shadingRateAttachmentTexelSize)
                             .dot(Field::height),
                         "(%" PRIu32
                         ") must be greater than or equal to "
                         "minFragmentShadingRateAttachmentTexelSize.height (%" PRIu32 ").",
                         rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height,
                         min_frs_attach_texel_height);
        }

        const uint32_t max_frs_attach_texel_aspect_ratio =
            phys_dev_ext_props.fragment_shading_rate_props.maxFragmentShadingRateAttachmentTexelSizeAspectRatio;
        if ((rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width /
             rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height) >
            max_frs_attach_texel_aspect_ratio) {
            skip |= LogError("VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06155", commandBuffer,
                             rendering_info_loc.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR,
                                                      Field::shadingRateAttachmentTexelSize),
                             "the quotient of width (%" PRIu32 ") and height (%" PRIu32
                             ") "
                             "must be less than or equal to maxFragmentShadingRateAttachmentTexelSizeAspectRatio (%" PRIu32 ").",
                             rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width,
                             rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height,
                             max_frs_attach_texel_aspect_ratio);
        }

        if ((rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height /
             rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width) >
            max_frs_attach_texel_aspect_ratio) {
            skip |= LogError("VUID-VkRenderingFragmentShadingRateAttachmentInfoKHR-imageView-06156", commandBuffer,
                             rendering_info_loc.pNext(Struct::VkRenderingFragmentShadingRateAttachmentInfoKHR,
                                                      Field::shadingRateAttachmentTexelSize),
                             "the quotient of height (%" PRIu32 ") and width (%" PRIu32
                             ") "
                             "must be less than or equal to maxFragmentShadingRateAttachmentTexelSizeAspectRatio (%" PRIu32 ").",
                             rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.height,
                             rendering_fragment_shading_rate_attachment_info->shadingRateAttachmentTexelSize.width,
                             max_frs_attach_texel_aspect_ratio);
        }
    }

    const auto fragment_density_map_attachment_info =
        vku::FindStructInPNextChain<VkRenderingFragmentDensityMapAttachmentInfoEXT>(pRenderingInfo->pNext);
    if (fragment_density_map_attachment_info && (fragment_density_map_attachment_info->imageView != VK_NULL_HANDLE)) {
        if (UniqueRenderingInfoImageViews(pRenderingInfo, fragment_density_map_attachment_info->imageView) == false) {
            skip |= LogError("VUID-VkRenderingInfo-imageView-06116", commandBuffer,
                             rendering_info_loc.pNext(Struct::VkRenderingFragmentDensityMapAttachmentInfoEXT, Field::imageView),
                             "is %s.", FormatHandle(fragment_density_map_attachment_info->imageView).c_str());
        }

        if (fragment_density_map_attachment_info->imageLayout != VK_IMAGE_LAYOUT_GENERAL &&
            fragment_density_map_attachment_info->imageLayout != VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT) {
            skip |= LogError("VUID-VkRenderingFragmentDensityMapAttachmentInfoEXT-imageView-06157", commandBuffer,
                             rendering_info_loc.pNext(Struct::VkRenderingFragmentDensityMapAttachmentInfoEXT, Field::imageView),
                             "is %s, but "
                             "VkRenderingFragmentDensityMapAttachmentInfoEXT::imageLayout is %s.",
                             FormatHandle(fragment_density_map_attachment_info->imageView).c_str(),
                             string_VkImageLayout(fragment_density_map_attachment_info->imageLayout));
        }

        if (rendering_fragment_shading_rate_attachment_info &&
            (rendering_fragment_shading_rate_attachment_info->imageView == fragment_density_map_attachment_info->imageView)) {
            skip |= LogError("VUID-VkRenderingInfo-imageView-06126", commandBuffer,
                             rendering_info_loc.pNext(Struct::VkRenderingFragmentDensityMapAttachmentInfoEXT, Field::imageView),
                             "and VkRenderingFragmentShadingRateAttachmentInfoKHR::imageView are the same (%s).",
                             FormatHandle(fragment_density_map_attachment_info->imageView).c_str());
        }
    }

    for (uint32_t j = 0; j < pRenderingInfo->colorAttachmentCount; ++j) {
        if (pRenderingInfo->pColorAttachments[j].imageView == VK_NULL_HANDLE) {
            continue;
        }
        const Location attachment_loc = rendering_info_loc.dot(Field::pColorAttachments, j);
        const VkImageLayout image_layout = pRenderingInfo->pColorAttachments[j].imageLayout;
        const VkResolveModeFlagBits resolve_mode = pRenderingInfo->pColorAttachments[j].resolveMode;
        const VkImageLayout resolve_image_layout = pRenderingInfo->pColorAttachments[j].resolveImageLayout;
        if (image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
            image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
            skip |= LogError("VUID-VkRenderingInfo-colorAttachmentCount-06090", commandBuffer,
                             attachment_loc.dot(Field::imageLayout), "is %s.", string_VkImageLayout(image_layout));
        }

        if (resolve_mode != VK_RESOLVE_MODE_NONE) {
            if (resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
                skip |= LogError("VUID-VkRenderingInfo-colorAttachmentCount-06091", commandBuffer,
                                 attachment_loc.dot(Field::resolveImageLayout), "is %s and resolve_mode is %s.",
                                 string_VkImageLayout(resolve_image_layout), string_VkResolveModeFlagBits(resolve_mode));
            }
        }

        if (IsExtEnabled(device_extensions.vk_khr_maintenance2)) {
            if (image_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL ||
                image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL) {
                skip |= LogError("VUID-VkRenderingInfo-colorAttachmentCount-06096", commandBuffer,
                                 attachment_loc.dot(Field::imageLayout), "is %s.", string_VkImageLayout(image_layout));
            }

            if (resolve_mode != VK_RESOLVE_MODE_NONE) {
                if (resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL ||
                    resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL) {
                    skip |= LogError("VUID-VkRenderingInfo-colorAttachmentCount-06097", commandBuffer,
                                     attachment_loc.dot(Field::resolveImageLayout), "is %s and resolve_mode is %s.",
                                     string_VkImageLayout(resolve_image_layout), string_VkResolveModeFlagBits(resolve_mode));
                }
            }
        }

        if (IsImageLayoutDepthOnly(image_layout) || IsImageLayoutStencilOnly(image_layout)) {
            skip |= LogError("VUID-VkRenderingInfo-colorAttachmentCount-06100", commandBuffer,
                             attachment_loc.dot(Field::imageLayout), "is %s.", string_VkImageLayout(image_layout));
        }

        if (resolve_mode != VK_RESOLVE_MODE_NONE) {
            if (resolve_image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
                resolve_image_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL) {
                skip |= LogError("VUID-VkRenderingInfo-colorAttachmentCount-06101", commandBuffer,
                                 attachment_loc.dot(Field::resolveImageLayout), "is %s and resolve_mode is %s.",
                                 string_VkImageLayout(resolve_image_layout), string_VkResolveModeFlagBits(resolve_mode));
            }
        }
    }

    if (pRenderingInfo->pDepthAttachment && pRenderingInfo->pDepthAttachment->imageView != VK_NULL_HANDLE) {
        const Location attachment_loc = rendering_info_loc.dot(Field::pDepthAttachment);
        const VkImageLayout layout = pRenderingInfo->pDepthAttachment->imageLayout;
        if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            skip |= LogError("VUID-VkRenderingInfo-pDepthAttachment-06092", commandBuffer, attachment_loc.dot(Field::imageLayout),
                             "is VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.");
        } else if (IsImageLayoutStencilOnly(layout)) {
            skip |= LogError("VUID-VkRenderingInfo-pDepthAttachment-07732", commandBuffer, attachment_loc.dot(Field::imageLayout),
                             "is %s.", string_VkImageLayout(layout));
        }

        if (pRenderingInfo->pDepthAttachment->resolveMode != VK_RESOLVE_MODE_NONE) {
            const VkImageLayout resolve_layout = pRenderingInfo->pDepthAttachment->resolveImageLayout;
            if (resolve_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                skip |= LogError("VUID-VkRenderingInfo-pDepthAttachment-06093", commandBuffer,
                                 attachment_loc.dot(Field::resolveImageLayout),
                                 "is VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL and resolve_mode is %s.",
                                 string_VkResolveModeFlagBits(pRenderingInfo->pDepthAttachment->resolveMode));
            } else if (IsImageLayoutStencilOnly(resolve_layout)) {
                skip |= LogError("VUID-VkRenderingInfo-pDepthAttachment-07733", commandBuffer,
                                 attachment_loc.dot(Field::resolveImageLayout), "is %s and resolve_mode is %s.",
                                 string_VkImageLayout(resolve_layout),
                                 string_VkResolveModeFlagBits(pRenderingInfo->pDepthAttachment->resolveMode));
            }

            if (IsExtEnabled(device_extensions.vk_khr_maintenance2) &&
                resolve_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) {
                skip |= LogError("VUID-VkRenderingInfo-pDepthAttachment-06098", commandBuffer,
                                 attachment_loc.dot(Field::resolveImageLayout),
                                 "is VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL.");
            }

            if (!(pRenderingInfo->pDepthAttachment->resolveMode &
                  phys_dev_ext_props.depth_stencil_resolve_props.supportedDepthResolveModes)) {
                skip |= LogError(
                    "VUID-VkRenderingInfo-pDepthAttachment-06102", commandBuffer, attachment_loc.dot(Field::resolveMode),
                    "is %s, but supportedDepthResolveModes is %s.",
                    string_VkResolveModeFlagBits(pRenderingInfo->pDepthAttachment->resolveMode),
                    string_VkResolveModeFlags(phys_dev_ext_props.depth_stencil_resolve_props.supportedDepthResolveModes).c_str());
            }
        }
    }

    if (pRenderingInfo->pStencilAttachment != nullptr && pRenderingInfo->pStencilAttachment->imageView != VK_NULL_HANDLE) {
        const Location attachment_loc = rendering_info_loc.dot(Field::pStencilAttachment);
        const VkImageLayout layout = pRenderingInfo->pStencilAttachment->imageLayout;
        if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            skip |= LogError("VUID-VkRenderingInfo-pStencilAttachment-06094", commandBuffer, attachment_loc.dot(Field::imageLayout),
                             "is VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.");
        } else if (IsImageLayoutDepthOnly(layout)) {
            skip |= LogError("VUID-VkRenderingInfo-pStencilAttachment-07734", commandBuffer, attachment_loc.dot(Field::imageLayout),
                             "is %s.", string_VkImageLayout(layout));
        }

        if (pRenderingInfo->pStencilAttachment->resolveMode != VK_RESOLVE_MODE_NONE) {
            const VkImageLayout resolve_layout = pRenderingInfo->pStencilAttachment->resolveImageLayout;
            if (resolve_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                skip |= LogError("VUID-VkRenderingInfo-pStencilAttachment-06095", commandBuffer,
                                 attachment_loc.dot(Field::resolveImageLayout),
                                 "is VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL and resolve_mode is %s.",
                                 string_VkResolveModeFlagBits(pRenderingInfo->pStencilAttachment->resolveMode));
            } else if (IsImageLayoutDepthOnly(resolve_layout)) {
                skip |= LogError("VUID-VkRenderingInfo-pStencilAttachment-07735", commandBuffer,
                                 attachment_loc.dot(Field::resolveImageLayout), "is %s and resolve_mode is %s.",
                                 string_VkImageLayout(resolve_layout),
                                 string_VkResolveModeFlagBits(pRenderingInfo->pStencilAttachment->resolveMode));
            }

            if (IsExtEnabled(device_extensions.vk_khr_maintenance2) &&
                resolve_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL) {
                skip |= LogError("VUID-VkRenderingInfo-pStencilAttachment-06099", commandBuffer,
                                 attachment_loc.dot(Field::resolveImageLayout),
                                 "is VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL.");
            }

            if (!(pRenderingInfo->pStencilAttachment->resolveMode &
                  phys_dev_ext_props.depth_stencil_resolve_props.supportedStencilResolveModes)) {
                skip |= LogError(
                    "VUID-VkRenderingInfo-pStencilAttachment-06103", commandBuffer, attachment_loc.dot(Field::resolveMode),
                    "is %s, but supportedStencilResolveModes is %s.",
                    string_VkResolveModeFlagBits(pRenderingInfo->pStencilAttachment->resolveMode),
                    string_VkResolveModeFlags(phys_dev_ext_props.depth_stencil_resolve_props.supportedStencilResolveModes).c_str());
            }
        }
    }

    return skip;
}
