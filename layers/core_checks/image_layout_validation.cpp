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
#include <vector>

#include "vk_enum_string_helper.h"
#include "chassis.h"
#include "core_checks/core_validation.h"
#include "sync/sync_vuid_maps.h"

using LayoutRange = image_layout_map::ImageSubresourceLayoutMap::RangeType;
using LayoutEntry = image_layout_map::ImageSubresourceLayoutMap::LayoutEntry;

static VkImageLayout NormalizeDepthImageLayout(VkImageLayout layout) {
    switch (layout) {
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

        default:
            return layout;
    }
}

static VkImageLayout NormalizeStencilImageLayout(VkImageLayout layout) {
    switch (layout) {
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            return VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;

        default:
            return layout;
    }
}

static VkImageLayout NormalizeSynchronization2Layout(const VkImageAspectFlags aspect_mask, VkImageLayout layout) {
    if (layout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR) {
        if (aspect_mask == VK_IMAGE_ASPECT_COLOR_BIT) {
            layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        } else if (aspect_mask == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
            layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        } else if (aspect_mask == VK_IMAGE_ASPECT_DEPTH_BIT) {
            layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        } else if (aspect_mask == VK_IMAGE_ASPECT_STENCIL_BIT) {
            layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        }
    } else if (layout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR) {
        if (aspect_mask == VK_IMAGE_ASPECT_COLOR_BIT) {
            layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        } else if (aspect_mask == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
            layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        } else if (aspect_mask == VK_IMAGE_ASPECT_DEPTH_BIT) {
            layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
        } else if (aspect_mask == VK_IMAGE_ASPECT_STENCIL_BIT) {
            layout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
        }
    }
    return layout;
}

static bool ImageLayoutMatches(const VkImageAspectFlags aspect_mask, VkImageLayout a, VkImageLayout b) {
    bool matches = (a == b);
    if (!matches) {
        a = NormalizeSynchronization2Layout(aspect_mask, a);
        b = NormalizeSynchronization2Layout(aspect_mask, b);
        matches = (a == b);
        if (!matches) {
            // Relaxed rules when referencing *only* the depth or stencil aspects.
            // When accessing both, normalize layouts for aspects separately.
            if (aspect_mask == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
                matches = NormalizeDepthImageLayout(a) == NormalizeDepthImageLayout(b) &&
                          NormalizeStencilImageLayout(a) == NormalizeStencilImageLayout(b);
            } else if (aspect_mask == VK_IMAGE_ASPECT_DEPTH_BIT) {
                matches = NormalizeDepthImageLayout(a) == NormalizeDepthImageLayout(b);
            } else if (aspect_mask == VK_IMAGE_ASPECT_STENCIL_BIT) {
                matches = NormalizeStencilImageLayout(a) == NormalizeStencilImageLayout(b);
            }
        }
    }
    return matches;
}

// Utility type for checking Image layouts
struct LayoutUseCheckAndMessage {
    const static VkImageAspectFlags kDepthOrStencil = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    const VkImageLayout expected_layout;
    const VkImageAspectFlags aspect_mask;
    const char *message;
    VkImageLayout layout;

    LayoutUseCheckAndMessage() = delete;
    LayoutUseCheckAndMessage(VkImageLayout expected, const VkImageAspectFlags aspect_mask_ = 0)
        : expected_layout{expected}, aspect_mask{aspect_mask_}, message(nullptr), layout(kInvalidLayout) {}
    bool Check(const LayoutEntry &layout_entry) {
        message = nullptr;
        layout = kInvalidLayout;  // Success status
        if (layout_entry.current_layout != kInvalidLayout) {
            if (!ImageLayoutMatches(aspect_mask, expected_layout, layout_entry.current_layout)) {
                message = "previous known";
                layout = layout_entry.current_layout;
            }
        } else if (layout_entry.initial_layout != kInvalidLayout) {
            if (!ImageLayoutMatches(aspect_mask, expected_layout, layout_entry.initial_layout)) {
                assert(layout_entry.state);  // If we have an initial layout, we better have a state for it
                if (!((layout_entry.state->aspect_mask & kDepthOrStencil) &&
                      ImageLayoutMatches(layout_entry.state->aspect_mask, expected_layout, layout_entry.initial_layout))) {
                    message = "previously used";
                    layout = layout_entry.initial_layout;
                }
            }
        }
        return layout == kInvalidLayout;
    }
};

template <typename RangeFactory>
bool CoreChecks::VerifyImageLayoutRange(const CMD_BUFFER_STATE &cb_state, const IMAGE_STATE &image_state,
                                        VkImageAspectFlags aspect_mask, VkImageLayout explicit_layout,
                                        const RangeFactory &range_factory, const char *caller, const char *layout_mismatch_msg_code,
                                        bool *error) const {
    bool skip = false;
    const auto *subresource_map = cb_state.GetImageSubresourceLayoutMap(image_state);
    if (!subresource_map) return skip;

    LayoutUseCheckAndMessage layout_check(explicit_layout, aspect_mask);
    skip |= subresource_map->AnyInRange(
        range_factory(*subresource_map), [this, subresource_map, &cb_state, &image_state, &layout_check, layout_mismatch_msg_code,
                                          caller, error](const LayoutRange &range, const LayoutEntry &state) {
            bool subres_skip = false;
            if (!layout_check.Check(state)) {
                *error = true;
                auto subres = subresource_map->Decode(range.begin);
                subres_skip |= LogError(cb_state.commandBuffer(), layout_mismatch_msg_code,
                                        "%s: Cannot use %s (layer=%" PRIu32 " mip=%" PRIu32
                                        ") with specific layout %s that doesn't match the "
                                        "%s layout %s.",
                                        caller, report_data->FormatHandle(image_state.Handle()).c_str(), subres.arrayLayer,
                                        subres.mipLevel, string_VkImageLayout(layout_check.expected_layout), layout_check.message,
                                        string_VkImageLayout(layout_check.layout));
            }
            return subres_skip;
        });

    return skip;
}

bool CoreChecks::VerifyImageLayout(const CMD_BUFFER_STATE &cb_state, const IMAGE_STATE &image_state,
                                   const VkImageSubresourceRange &range, VkImageAspectFlags aspect_mask,
                                   VkImageLayout explicit_layout, VkImageLayout optimal_layout, const char *caller,
                                   const char *layout_invalid_msg_code, const char *layout_mismatch_msg_code, bool *error) const {
    if (disabled[image_layout_validation]) return false;
    bool skip = false;

    VkImageSubresourceRange normalized_isr = image_state.NormalizeSubresourceRange(range);
    auto range_factory = [&normalized_isr](const ImageSubresourceLayoutMap &map) { return map.RangeGen(normalized_isr); };
    skip |= VerifyImageLayoutRange(cb_state, image_state, aspect_mask, explicit_layout, range_factory, caller,
                                   layout_mismatch_msg_code, error);

    // If optimal_layout is not UNDEFINED, check that layout matches optimal for this case
    if ((VK_IMAGE_LAYOUT_UNDEFINED != optimal_layout) && (explicit_layout != optimal_layout)) {
        if (VK_IMAGE_LAYOUT_GENERAL == explicit_layout) {
            if (image_state.createInfo.tiling != VK_IMAGE_TILING_LINEAR) {
                // LAYOUT_GENERAL is allowed, but may not be performance optimal, flag as perf warning.
                skip |= LogPerformanceWarning(cb_state.commandBuffer(), kVUID_Core_DrawState_InvalidImageLayout,
                                              "%s: For optimal performance %s layout should be %s instead of GENERAL.", caller,
                                              report_data->FormatHandle(image_state.Handle()).c_str(),
                                              string_VkImageLayout(optimal_layout));
            }
        } else if (IsExtEnabled(device_extensions.vk_khr_shared_presentable_image)) {
            if (image_state.shared_presentable) {
                if (VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR != explicit_layout) {
                    skip |=
                        LogError(device, layout_invalid_msg_code,
                                 "%s: Layout for shared presentable image is %s but must be VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR.",
                                 caller, string_VkImageLayout(optimal_layout));
                }
            }
        } else {
            *error = true;
            skip |= LogError(cb_state.commandBuffer(), layout_invalid_msg_code,
                             "%s: Layout for %s is %s but can only be %s or VK_IMAGE_LAYOUT_GENERAL.", caller,
                             report_data->FormatHandle(image_state.Handle()).c_str(), string_VkImageLayout(explicit_layout),
                             string_VkImageLayout(optimal_layout));
        }
    }
    return skip;
}
bool CoreChecks::VerifyImageLayout(const CMD_BUFFER_STATE &cb_state, const IMAGE_STATE &image_state,
                                   const VkImageSubresourceLayers &subLayers, VkImageLayout explicit_layout,
                                   VkImageLayout optimal_layout, const char *caller, const char *layout_invalid_msg_code,
                                   const char *layout_mismatch_msg_code, bool *error) const {
    return VerifyImageLayout(cb_state, image_state, RangeFromLayers(subLayers), explicit_layout, optimal_layout, caller,
                             layout_invalid_msg_code, layout_mismatch_msg_code, error);
}

bool CoreChecks::VerifyImageLayout(const CMD_BUFFER_STATE &cb_state, const IMAGE_VIEW_STATE &image_view_state,
                                   VkImageLayout explicit_layout, const char *caller, const char *layout_mismatch_msg_code,
                                   bool *error) const {
    if (disabled[image_layout_validation]) return false;
    assert(image_view_state.image_state);
    auto range_factory = [&image_view_state](const ImageSubresourceLayoutMap &map) {
        return image_layout_map::RangeGenerator(image_view_state.range_generator);
    };

    return VerifyImageLayoutRange(cb_state, *image_view_state.image_state, image_view_state.create_info.subresourceRange.aspectMask,
                                  explicit_layout, range_factory, caller, layout_mismatch_msg_code, error);
}

bool CoreChecks::VerifyImageLayout(const CMD_BUFFER_STATE &cb_state, const IMAGE_STATE &image_state,
                                   const VkImageSubresourceRange &range, VkImageLayout explicit_layout, const char *caller,
                                   const char *layout_mismatch_msg_code, bool *error) const {
    if (disabled[image_layout_validation]) return false;

    auto range_factory = [&range](const ImageSubresourceLayoutMap &map) { return map.RangeGen(range); };

    return VerifyImageLayoutRange(cb_state, image_state, range.aspectMask, explicit_layout, range_factory, caller,
                                  layout_mismatch_msg_code, error);
}

void CoreChecks::TransitionFinalSubpassLayouts(CMD_BUFFER_STATE *cb_state, const VkRenderPassBeginInfo *pRenderPassBegin,
                                               FRAMEBUFFER_STATE *framebuffer_state) {
    auto render_pass = Get<RENDER_PASS_STATE>(pRenderPassBegin->renderPass);
    if (!render_pass) return;

    const VkRenderPassCreateInfo2 *render_pass_info = render_pass->createInfo.ptr();
    if (framebuffer_state) {
        for (uint32_t i = 0; i < render_pass_info->attachmentCount; ++i) {
            auto *view_state = cb_state->GetActiveAttachmentImageViewState(i);
            if (view_state) {
                VkImageLayout stencil_layout = kInvalidLayout;
                const auto *attachment_description_stencil_layout =
                    LvlFindInChain<VkAttachmentDescriptionStencilLayout>(render_pass_info->pAttachments[i].pNext);
                if (attachment_description_stencil_layout) {
                    stencil_layout = attachment_description_stencil_layout->stencilFinalLayout;
                }
                cb_state->SetImageViewLayout(*view_state, render_pass_info->pAttachments[i].finalLayout, stencil_layout);
            }
        }
    }
}

static GlobalImageLayoutRangeMap *GetLayoutRangeMap(GlobalImageLayoutMap &map, const IMAGE_STATE &image_state) {
    // This approach allows for a single hash lookup or/create new
    auto &layout_map = map[&image_state];
    if (!layout_map) {
        layout_map.emplace(image_state.subresource_encoder.SubresourceCount());
    }
    return &(*layout_map);
}

// Helper to update the Global or Overlay layout map
struct GlobalLayoutUpdater {
    bool update(VkImageLayout &dst, const image_layout_map::ImageSubresourceLayoutMap::LayoutEntry &src) const {
        if (src.current_layout != image_layout_map::kInvalidLayout && dst != src.current_layout) {
            dst = src.current_layout;
            return true;
        }
        return false;
    }

    std::optional<VkImageLayout> insert(const image_layout_map::ImageSubresourceLayoutMap::LayoutEntry &src) const {
        std::optional<VkImageLayout> result;
        if (src.current_layout != image_layout_map::kInvalidLayout) {
            result.emplace(src.current_layout);
        }
        return result;
    }
};

// This validates that the initial layout specified in the command buffer for the IMAGE is the same as the global IMAGE layout
bool CoreChecks::ValidateCmdBufImageLayouts(const Location &loc, const CMD_BUFFER_STATE &cb_state,
                                            GlobalImageLayoutMap &overlayLayoutMap) const {
    if (disabled[image_layout_validation]) return false;
    bool skip = false;
    // Iterate over the layout maps for each referenced image
    GlobalImageLayoutRangeMap empty_map(1);
    for (const auto &layout_map_entry : cb_state.image_layout_map) {
        const auto *image_state = layout_map_entry.first;
        const auto &subres_map = layout_map_entry.second;
        const auto &layout_map = subres_map->GetLayoutMap();
        // Validate the initial_uses for each subresource referenced
        if (layout_map.empty()) continue;

        auto *overlay_map = GetLayoutRangeMap(overlayLayoutMap, *image_state);
        const auto *global_map = image_state->layout_range_map.get();
        assert(global_map);
        auto global_map_guard = global_map->ReadLock();

        // Note: don't know if it would matter
        // if (global_map->empty() && overlay_map->empty()) // skip this next loop...;

        auto pos = layout_map.begin();
        const auto end = layout_map.end();
        sparse_container::parallel_iterator<const GlobalImageLayoutRangeMap> current_layout(*overlay_map, *global_map,
                                                                                            pos->first.begin);
        while (pos != end) {
            VkImageLayout initial_layout = pos->second.initial_layout;
            assert(initial_layout != image_layout_map::kInvalidLayout);
            if (initial_layout == image_layout_map::kInvalidLayout) {
                continue;
            }

            VkImageLayout image_layout = kInvalidLayout;

            if (current_layout->range.empty()) break;  // When we are past the end of data in overlay and global... stop looking
            if (current_layout->pos_A->valid) {        // pos_A denotes the overlay map in the parallel iterator
                image_layout = current_layout->pos_A->lower_bound->second;
            } else if (current_layout->pos_B->valid) {  // pos_B denotes the global map in the parallel iterator
                image_layout = current_layout->pos_B->lower_bound->second;
            }
            const auto intersected_range = pos->first & current_layout->range;
            if (initial_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
                // TODO: Set memory invalid which is in mem_tracker currently
            } else if (image_layout != initial_layout) {
                const auto aspect_mask = image_state->subresource_encoder.Decode(intersected_range.begin).aspectMask;
                const bool matches = ImageLayoutMatches(aspect_mask, image_layout, initial_layout);
                if (!matches) {
                    // We can report all the errors for the intersected range directly
                    for (auto index : sparse_container::range_view<decltype(intersected_range)>(intersected_range)) {
                        const auto subresource = image_state->subresource_encoder.Decode(index);
                        skip |= LogError(cb_state.commandBuffer(), kVUID_Core_DrawState_InvalidImageLayout,
                                         "%s command buffer %s expects %s (subresource: aspectMask 0x%X array layer %" PRIu32
                                         ", mip level %" PRIu32
                                         ") "
                                         "to be in layout %s--instead, current layout is %s.",
                                         loc.Message().c_str(), report_data->FormatHandle(cb_state.commandBuffer()).c_str(),
                                         report_data->FormatHandle(image_state->Handle()).c_str(), subresource.aspectMask,
                                         subresource.arrayLayer, subresource.mipLevel, string_VkImageLayout(initial_layout),
                                         string_VkImageLayout(image_layout));
                    }
                }
            }
            if (pos->first.includes(intersected_range.end)) {
                current_layout.seek(intersected_range.end);
            } else {
                ++pos;
                if (pos != end) {
                    current_layout.seek(pos->first.begin);
                }
            }
        }
        // Update all layout set operations (which will be a subset of the initial_layouts)
        sparse_container::splice(*overlay_map, subres_map->GetLayoutMap(), GlobalLayoutUpdater());
    }

    return skip;
}

void CoreChecks::UpdateCmdBufImageLayouts(const CMD_BUFFER_STATE *cb_state) {
    for (const auto &layout_map_entry : cb_state->image_layout_map) {
        const auto *image_state = layout_map_entry.first;
        const auto &subres_map = layout_map_entry.second;
        auto guard = image_state->layout_range_map->WriteLock();
        sparse_container::splice(*image_state->layout_range_map, subres_map->GetLayoutMap(), GlobalLayoutUpdater());
    }
}

// ValidateLayoutVsAttachmentDescription is a general function where we can validate various state associated with the
// VkAttachmentDescription structs that are used by the sub-passes of a renderpass. Initial check is to make sure that READ_ONLY
// layout attachments don't have CLEAR as their loadOp.
bool CoreChecks::ValidateLayoutVsAttachmentDescription(const debug_report_data *report_data, RenderPassCreateVersion rp_version,
                                                       const VkImageLayout first_layout, const uint32_t attachment,
                                                       const VkAttachmentDescription2 &attachment_description) const {
    bool skip = false;
    const bool use_rp2 = (rp_version == RENDER_PASS_VERSION_2);

    // Verify that initial loadOp on READ_ONLY attachments is not CLEAR
    // for both loadOp and stencilLoaOp rp2 has it in 1 VU while rp1 has it in 2 VU with half behind Maintenance2 extension
    // Each is VUID is below in following order: rp2 -> rp1 with Maintenance2 -> rp1 with no extenstion
    if (attachment_description.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
        if (use_rp2 && ((first_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) ||
                        (first_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) ||
                        (first_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL))) {
            skip |= LogError(device, "VUID-VkRenderPassCreateInfo2-pAttachments-02522",
                             "vkCreateRenderPass2(): Cannot clear attachment %d with invalid first layout %s.", attachment,
                             string_VkImageLayout(first_layout));
        } else if ((use_rp2 == false) && IsExtEnabled(device_extensions.vk_khr_maintenance2) &&
                   (first_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL)) {
            skip |= LogError(device, "VUID-VkRenderPassCreateInfo-pAttachments-01566",
                             "vkCreateRenderPass(): Cannot clear attachment %d with invalid first layout %s.", attachment,
                             string_VkImageLayout(first_layout));
        } else if ((use_rp2 == false) && ((first_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) ||
                                          (first_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))) {
            skip |= LogError(device, "VUID-VkRenderPassCreateInfo-pAttachments-00836",
                             "vkCreateRenderPass(): Cannot clear attachment %d with invalid first layout %s.", attachment,
                             string_VkImageLayout(first_layout));
        }
    }

    // Same as above for loadOp, but for stencilLoadOp
    if (attachment_description.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
        if (use_rp2 && ((first_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) ||
                        (first_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) ||
                        (first_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL))) {
            skip |= LogError(device, "VUID-VkRenderPassCreateInfo2-pAttachments-02523",
                             "vkCreateRenderPass2(): Cannot clear attachment %d with invalid first layout %s.", attachment,
                             string_VkImageLayout(first_layout));
        } else if ((use_rp2 == false) && IsExtEnabled(device_extensions.vk_khr_maintenance2) &&
                   (first_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL)) {
            skip |= LogError(device, "VUID-VkRenderPassCreateInfo-pAttachments-01567",
                             "vkCreateRenderPass(): Cannot clear attachment %d with invalid first layout %s.", attachment,
                             string_VkImageLayout(first_layout));
        } else if ((use_rp2 == false) && ((first_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) ||
                                          (first_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))) {
            skip |= LogError(device, "VUID-VkRenderPassCreateInfo-pAttachments-02511",
                             "vkCreateRenderPass(): Cannot clear attachment %d with invalid first layout %s.", attachment,
                             string_VkImageLayout(first_layout));
        }
    }

    return skip;
}

bool CoreChecks::ValidateMultipassRenderedToSingleSampledSampleCount(RenderPassCreateVersion rp_version, VkFramebuffer framebuffer,
                                                                     VkRenderPass renderpass, uint32_t subpass,
                                                                     IMAGE_STATE *image_state, VkSampleCountFlagBits msrtss_samples,
                                                                     uint32_t attachment_index, bool depth) const {
    bool skip = false;
    const char *function_name = (rp_version == RENDER_PASS_VERSION_2) ? "vkCmdBeginRenderPass2()" : "vkCmdBeginRenderPass()";
    const auto image_create_info = image_state->createInfo;
    if (!image_state->image_format_properties.sampleCounts) {
        skip |= GetPhysicalDeviceImageFormatProperties(*image_state, "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-07010");
    }
    if (!(image_state->image_format_properties.sampleCounts & msrtss_samples)) {
        std::stringstream msg;
        if (depth) {
            msg << "depth stencil attachment";
        } else {
            msg << "attachment " << attachment_index;
        }
        skip |= LogError(device, "VUID-VkRenderPassAttachmentBeginInfo-pAttachments-07010",
                         "%s(): Renderpass subpass %" PRIu32
                         " enables "
                         "multisampled-render-to-single-sampled and %s"
                         ", is specified with "
                         "VK_SAMPLE_COUNT_1_BIT samples, but image (%s) created with format %s imageType: %s, "
                         "tiling: %s, usage: %s, "
                         "flags: %s does not support a rasterizationSamples count of %s",
                         function_name, subpass, msg.str().c_str(), report_data->FormatHandle(image_state->Handle()).c_str(),
                         string_VkFormat(image_create_info.format), string_VkImageType(image_create_info.imageType),
                         string_VkImageTiling(image_create_info.tiling), string_VkImageUsageFlags(image_create_info.usage).c_str(),
                         string_VkImageCreateFlags(image_create_info.flags).c_str(), string_VkSampleCountFlagBits(msrtss_samples));
    }
    return skip;
}

bool CoreChecks::ValidateRenderPassLayoutAgainstFramebufferImageUsage(RenderPassCreateVersion rp_version, VkImageLayout layout,
                                                                      const IMAGE_VIEW_STATE &image_view_state,
                                                                      VkFramebuffer framebuffer, VkRenderPass renderpass,
                                                                      uint32_t attachment_index, const char *variable_name) const {
    bool skip = false;
    const auto &image_view = image_view_state.Handle();
    const auto *image_state = image_view_state.image_state.get();
    const auto &image = image_state->Handle();
    const char *vuid;
    const bool use_rp2 = (rp_version == RENDER_PASS_VERSION_2);
    const char *function_name = use_rp2 ? "vkCmdBeginRenderPass2()" : "vkCmdBeginRenderPass()";

    if (!image_state) {
        const LogObjectList objlist(image, renderpass, framebuffer, image_view);
        skip |=
            LogError(objlist, "VUID-VkRenderPassBeginInfo-framebuffer-parameter",
                     "%s: RenderPass %s uses %s where pAttachments[%" PRIu32 "] = %s, which refers to an invalid image",
                     function_name, report_data->FormatHandle(renderpass).c_str(), report_data->FormatHandle(framebuffer).c_str(),
                     attachment_index, report_data->FormatHandle(image_view).c_str());
        return skip;
    }

    auto image_usage = image_state->createInfo.usage;
    const auto stencil_usage_info = LvlFindInChain<VkImageStencilUsageCreateInfo>(image_state->createInfo.pNext);
    if (stencil_usage_info) {
        image_usage |= stencil_usage_info->stencilUsage;
    }

    // Check for layouts that mismatch image usages in the framebuffer
    if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && !(image_usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
        vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2-initialLayout-03094" : "VUID-vkCmdBeginRenderPass-initialLayout-00895";
        const LogObjectList objlist(image, renderpass, framebuffer, image_view);
        skip |= LogError(objlist, vuid,
                         "%s: Layout/usage mismatch for attachment %" PRIu32
                         " in %s"
                         " - the %s is %s but the image attached to %s via %s"
                         " was not created with VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT",
                         function_name, attachment_index, report_data->FormatHandle(renderpass).c_str(), variable_name,
                         string_VkImageLayout(layout), report_data->FormatHandle(framebuffer).c_str(),
                         report_data->FormatHandle(image_view).c_str());
    }

    if (layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
        !(image_usage & (VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT))) {
        vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2-initialLayout-03097" : "VUID-vkCmdBeginRenderPass-initialLayout-00897";
        const LogObjectList objlist(image, renderpass, framebuffer, image_view);
        skip |= LogError(objlist, vuid,
                         "%s: Layout/usage mismatch for attachment %" PRIu32
                         " in %s"
                         " - the %s is %s but the image attached to %s via %s"
                         " was not created with VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT or VK_IMAGE_USAGE_SAMPLED_BIT",
                         function_name, attachment_index, report_data->FormatHandle(renderpass).c_str(), variable_name,
                         string_VkImageLayout(layout), report_data->FormatHandle(framebuffer).c_str(),
                         report_data->FormatHandle(image_view).c_str());
    }

    if (layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && !(image_usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)) {
        vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2-initialLayout-03098" : "VUID-vkCmdBeginRenderPass-initialLayout-00898";
        const LogObjectList objlist(image, renderpass, framebuffer, image_view);
        skip |= LogError(objlist, vuid,
                         "%s: Layout/usage mismatch for attachment %" PRIu32
                         " in %s"
                         " - the %s is %s but the image attached to %s via %s"
                         " was not created with VK_IMAGE_USAGE_TRANSFER_SRC_BIT",
                         function_name, attachment_index, report_data->FormatHandle(renderpass).c_str(), variable_name,
                         string_VkImageLayout(layout), report_data->FormatHandle(framebuffer).c_str(),
                         report_data->FormatHandle(image_view).c_str());
    }

    if (layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && !(image_usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2-initialLayout-03099" : "VUID-vkCmdBeginRenderPass-initialLayout-00899";
        const LogObjectList objlist(image, renderpass, framebuffer, image_view);
        skip |= LogError(objlist, vuid,
                         "%s: Layout/usage mismatch for attachment %" PRIu32
                         " in %s"
                         " - the %s is %s but the image attached to %s via %s"
                         " was not created with VK_IMAGE_USAGE_TRANSFER_DST_BIT",
                         function_name, attachment_index, report_data->FormatHandle(renderpass).c_str(), variable_name,
                         string_VkImageLayout(layout), report_data->FormatHandle(framebuffer).c_str(),
                         report_data->FormatHandle(image_view).c_str());
    }

    if (layout == VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT) {
        if (((image_usage & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) == 0) ||
            ((image_usage & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) == 0)) {
            vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2-initialLayout-07002" : "VUID-vkCmdBeginRenderPass-initialLayout-07000";
            const LogObjectList objlist(image, renderpass, framebuffer, image_view);
            skip |=
                LogError(objlist, vuid,
                         "%s: Layout/usage mismatch for attachment %" PRIu32
                         " in %s"
                         " - the %s is %s but the image attached to %s via %s"
                         " was not created with either the VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT or "
                         "VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT usage bits, and the VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT or "
                         "VK_IMAGE_USAGE_SAMPLED_BIT usage bits",
                         function_name, attachment_index, report_data->FormatHandle(renderpass).c_str(), variable_name,
                         string_VkImageLayout(layout), report_data->FormatHandle(framebuffer).c_str(),
                         report_data->FormatHandle(image_view).c_str());
        }
        if (!(image_usage & VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT)) {
            vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2-initialLayout-07003" : "VUID-vkCmdBeginRenderPass-initialLayout-07001";
            const LogObjectList objlist(image, renderpass, framebuffer, image_view);
            skip |= LogError(objlist, vuid,
                             "%s: Layout/usage mismatch for attachment %" PRIu32
                             " in %s"
                             " - the %s is %s but the image attached to %s via %s"
                             " was not created with the VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT usage bit",
                             function_name, attachment_index, report_data->FormatHandle(renderpass).c_str(), variable_name,
                             string_VkImageLayout(layout), report_data->FormatHandle(framebuffer).c_str(),
                             report_data->FormatHandle(image_view).c_str());
        }
    }

    if (IsExtEnabled(device_extensions.vk_khr_maintenance2)) {
        if ((layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL ||
             layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
             layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
             layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) &&
            !(image_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
            vuid = use_rp2 ? "VUID-vkCmdBeginRenderPass2-initialLayout-03096" : "VUID-vkCmdBeginRenderPass-initialLayout-01758";
            const LogObjectList objlist(image, renderpass, framebuffer, image_view);
            skip |= LogError(objlist, vuid,
                             "%s: Layout/usage mismatch for attachment %" PRIu32
                             " in %s"
                             " - the %s is %s but the image attached to %s via %s"
                             " was not created with VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT",
                             function_name, attachment_index, report_data->FormatHandle(renderpass).c_str(), variable_name,
                             string_VkImageLayout(layout), report_data->FormatHandle(framebuffer).c_str(),
                             report_data->FormatHandle(image_view).c_str());
        }
    } else {
        // The create render pass 2 extension requires maintenance 2 (the previous branch), so no vuid switch needed here.
        if ((layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
             layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) &&
            !(image_usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
            const LogObjectList objlist(image, renderpass, framebuffer, image_view);
            skip |= LogError(objlist, "VUID-vkCmdBeginRenderPass-initialLayout-00896",
                             "%s: Layout/usage mismatch for attachment %" PRIu32
                             " in %s"
                             " - the %s is %s but the image attached to %s via %s"
                             " was not created with VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT",
                             function_name, attachment_index, report_data->FormatHandle(renderpass).c_str(), variable_name,
                             string_VkImageLayout(layout), report_data->FormatHandle(framebuffer).c_str(),
                             report_data->FormatHandle(image_view).c_str());
        }
    }
    return skip;
}

bool CoreChecks::VerifyFramebufferAndRenderPassLayouts(RenderPassCreateVersion rp_version, const CMD_BUFFER_STATE &cb_state,
                                                       const VkRenderPassBeginInfo *pRenderPassBegin,
                                                       const FRAMEBUFFER_STATE *framebuffer_state) const {
    bool skip = false;
    auto render_pass_state = Get<RENDER_PASS_STATE>(pRenderPassBegin->renderPass);
    const auto *render_pass_info = render_pass_state->createInfo.ptr();
    auto render_pass = render_pass_state->renderPass();
    auto const &framebuffer_info = framebuffer_state->createInfo;
    const VkImageView *attachments = framebuffer_info.pAttachments;

    auto framebuffer = framebuffer_state->framebuffer();

    if (render_pass_info->attachmentCount != framebuffer_info.attachmentCount) {
        skip |= LogError(cb_state.commandBuffer(), kVUID_Core_DrawState_InvalidRenderpass,
                         "You cannot start a render pass using a framebuffer with a different number of attachments.");
    }

    const auto *attachment_info = LvlFindInChain<VkRenderPassAttachmentBeginInfo>(pRenderPassBegin->pNext);
    if (((framebuffer_info.flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) != 0) && attachment_info != nullptr) {
        attachments = attachment_info->pAttachments;
    }

    if (attachments == nullptr) {
        return skip;
    }
    for (uint32_t i = 0; i < render_pass_info->attachmentCount && i < framebuffer_info.attachmentCount; ++i) {
        auto image_view = attachments[i];
        auto view_state = Get<IMAGE_VIEW_STATE>(image_view);

        if (!view_state) {
            const LogObjectList objlist(pRenderPassBegin->renderPass, framebuffer_state->framebuffer(), image_view);
            skip |= LogError(objlist, "VUID-VkRenderPassBeginInfo-framebuffer-parameter",
                             "vkCmdBeginRenderPass(): %s pAttachments[%" PRIu32 "] = %s is not a valid VkImageView handle",
                             report_data->FormatHandle(framebuffer_state->framebuffer()).c_str(), i,
                             report_data->FormatHandle(image_view).c_str());
            continue;
        }

        const VkImage image = view_state->create_info.image;
        const auto *image_state = view_state->image_state.get();

        if (!image_state) {
            const LogObjectList objlist(pRenderPassBegin->renderPass, framebuffer_state->framebuffer(), image_view, image);
            skip |= LogError(objlist, "VUID-VkRenderPassBeginInfo-framebuffer-parameter",
                             "vkCmdBeginRenderPass(): %s pAttachments[%" PRIu32 "] =  %s references non-extant %s.",
                             report_data->FormatHandle(framebuffer_state->framebuffer()).c_str(), i,
                             report_data->FormatHandle(image_view).c_str(), report_data->FormatHandle(image).c_str());
            continue;
        }
        auto attachment_initial_layout = render_pass_info->pAttachments[i].initialLayout;
        auto final_layout = render_pass_info->pAttachments[i].finalLayout;

        // Default to expecting stencil in the same layout.
        auto attachment_stencil_initial_layout = attachment_initial_layout;

        // If a separate layout is specified, look for that.
        const auto *attachment_description_stencil_layout =
            LvlFindInChain<VkAttachmentDescriptionStencilLayout>(render_pass_info->pAttachments[i].pNext);
        if (attachment_description_stencil_layout) {
            attachment_stencil_initial_layout = attachment_description_stencil_layout->stencilInitialLayout;
        }

        const ImageSubresourceLayoutMap *subresource_map = nullptr;
        bool has_queried_map = false;

        for (uint32_t aspect_index = 0; aspect_index < 32; aspect_index++) {
            VkImageAspectFlags test_aspect = 1u << aspect_index;
            if ((view_state->normalized_subresource_range.aspectMask & test_aspect) == 0) {
                continue;
            }

            // Allow for differing depth and stencil layouts
            VkImageLayout check_layout = attachment_initial_layout;
            if (test_aspect == VK_IMAGE_ASPECT_STENCIL_BIT) {
                check_layout = attachment_stencil_initial_layout;
            }

            // If no layout information for image yet, will be checked at QueueSubmit time
            if (check_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
                continue;
            }
            if (!has_queried_map) {
                subresource_map = cb_state.GetImageSubresourceLayoutMap(*image_state);
                has_queried_map = true;
            }
            if (!subresource_map) {
                // If no layout information for image yet, will be checked at QueueSubmit time
                continue;
            }
            auto normalized_range = view_state->normalized_subresource_range;
            normalized_range.aspectMask = test_aspect;
            LayoutUseCheckAndMessage layout_check(check_layout, test_aspect);

            skip |= subresource_map->AnyInRange(
                normalized_range, [this, &layout_check, i](const LayoutRange &range, const LayoutEntry &state) {
                    bool subres_skip = false;
                    if (!layout_check.Check(state)) {
                        subres_skip = LogError(device, kVUID_Core_DrawState_InvalidRenderpass,
                                               "You cannot start a render pass using attachment %" PRIu32
                                               " where the render pass initial "
                                               "layout is %s "
                                               "and the %s layout of the attachment is %s. The layouts must match, or the render "
                                               "pass initial layout for the attachment must be VK_IMAGE_LAYOUT_UNDEFINED",
                                               i, string_VkImageLayout(layout_check.expected_layout), layout_check.message,
                                               string_VkImageLayout(layout_check.layout));
                    }
                    return subres_skip;
                });
        }
        skip |= ValidateRenderPassLayoutAgainstFramebufferImageUsage(rp_version, attachment_initial_layout, *view_state,
                                                                     framebuffer, render_pass, i, "initial layout");

        skip |= ValidateRenderPassLayoutAgainstFramebufferImageUsage(rp_version, final_layout, *view_state, framebuffer,
                                                                     render_pass, i, "final layout");
    }

    for (uint32_t j = 0; j < render_pass_info->subpassCount; ++j) {
        auto &subpass = render_pass_info->pSubpasses[j];
        const auto *ms_rendered_to_single_sampled =
            LvlFindInChain<VkMultisampledRenderToSingleSampledInfoEXT>(render_pass_info->pSubpasses[j].pNext);
        for (uint32_t k = 0; k < render_pass_info->pSubpasses[j].inputAttachmentCount; ++k) {
            auto &attachment_ref = subpass.pInputAttachments[k];
            if (attachment_ref.attachment != VK_ATTACHMENT_UNUSED) {
                auto image_view = attachments[attachment_ref.attachment];
                auto view_state = Get<IMAGE_VIEW_STATE>(image_view);

                if (view_state) {
                    skip |= ValidateRenderPassLayoutAgainstFramebufferImageUsage(
                        rp_version, attachment_ref.layout, *view_state, framebuffer, render_pass, attachment_ref.attachment,
                        "input attachment layout");
                }
                if (ms_rendered_to_single_sampled && ms_rendered_to_single_sampled->multisampledRenderToSingleSampledEnable) {
                    if (render_pass_info->pAttachments[attachment_ref.attachment].samples == VK_SAMPLE_COUNT_1_BIT) {
                        skip |= ValidateMultipassRenderedToSingleSampledSampleCount(
                            rp_version, framebuffer, render_pass, k, view_state->image_state.get(),
                            ms_rendered_to_single_sampled->rasterizationSamples, attachment_ref.attachment);
                    }
                }
            }
        }

        for (uint32_t k = 0; k < render_pass_info->pSubpasses[j].colorAttachmentCount; ++k) {
            auto &attachment_ref = subpass.pColorAttachments[k];
            if (attachment_ref.attachment != VK_ATTACHMENT_UNUSED) {
                auto image_view = attachments[attachment_ref.attachment];
                auto view_state = Get<IMAGE_VIEW_STATE>(image_view);

                if (view_state) {
                    skip |= ValidateRenderPassLayoutAgainstFramebufferImageUsage(
                        rp_version, attachment_ref.layout, *view_state, framebuffer, render_pass, attachment_ref.attachment,
                        "color attachment layout");
                    if (subpass.pResolveAttachments) {
                        skip |= ValidateRenderPassLayoutAgainstFramebufferImageUsage(
                            rp_version, attachment_ref.layout, *view_state, framebuffer, render_pass, attachment_ref.attachment,
                            "resolve attachment layout");
                    }
                }
                if (ms_rendered_to_single_sampled && ms_rendered_to_single_sampled->multisampledRenderToSingleSampledEnable) {
                    if (render_pass_info->pAttachments[attachment_ref.attachment].samples == VK_SAMPLE_COUNT_1_BIT) {
                        skip |= ValidateMultipassRenderedToSingleSampledSampleCount(
                            rp_version, framebuffer, render_pass, k, view_state->image_state.get(),
                            ms_rendered_to_single_sampled->rasterizationSamples, attachment_ref.attachment);
                    }
                }
            }
        }

        if (render_pass_info->pSubpasses[j].pDepthStencilAttachment) {
            auto &attachment_ref = *subpass.pDepthStencilAttachment;
            if (attachment_ref.attachment != VK_ATTACHMENT_UNUSED) {
                auto image_view = attachments[attachment_ref.attachment];
                auto view_state = Get<IMAGE_VIEW_STATE>(image_view);

                if (view_state) {
                    skip |= ValidateRenderPassLayoutAgainstFramebufferImageUsage(
                        rp_version, attachment_ref.layout, *view_state, framebuffer, render_pass, attachment_ref.attachment,
                        "input attachment layout");
                }
                if (ms_rendered_to_single_sampled && ms_rendered_to_single_sampled->multisampledRenderToSingleSampledEnable) {
                    if (render_pass_info->pAttachments[attachment_ref.attachment].samples == VK_SAMPLE_COUNT_1_BIT) {
                        skip |= ValidateMultipassRenderedToSingleSampledSampleCount(
                            rp_version, framebuffer, render_pass, 0, view_state->image_state.get(),
                            ms_rendered_to_single_sampled->rasterizationSamples, attachment_ref.attachment, true);
                    }
                }
            }
        }
    }
    return skip;
}

void CoreChecks::TransitionAttachmentRefLayout(CMD_BUFFER_STATE *cb_state, FRAMEBUFFER_STATE *pFramebuffer,
                                               const safe_VkAttachmentReference2 &ref) {
    if (ref.attachment != VK_ATTACHMENT_UNUSED) {
        IMAGE_VIEW_STATE *image_view = cb_state->GetActiveAttachmentImageViewState(ref.attachment);
        if (image_view) {
            VkImageLayout stencil_layout = kInvalidLayout;
            const auto *attachment_reference_stencil_layout = LvlFindInChain<VkAttachmentReferenceStencilLayout>(ref.pNext);
            if (attachment_reference_stencil_layout) {
                stencil_layout = attachment_reference_stencil_layout->stencilLayout;
            }

            cb_state->SetImageViewLayout(*image_view, ref.layout, stencil_layout);
        }
    }
}

void CoreChecks::TransitionSubpassLayouts(CMD_BUFFER_STATE *cb_state, const RENDER_PASS_STATE *render_pass_state,
                                          const int subpass_index, FRAMEBUFFER_STATE *framebuffer_state) {
    assert(render_pass_state);

    if (framebuffer_state) {
        auto const &subpass = render_pass_state->createInfo.pSubpasses[subpass_index];
        for (uint32_t j = 0; j < subpass.inputAttachmentCount; ++j) {
            TransitionAttachmentRefLayout(cb_state, framebuffer_state, subpass.pInputAttachments[j]);
        }
        for (uint32_t j = 0; j < subpass.colorAttachmentCount; ++j) {
            TransitionAttachmentRefLayout(cb_state, framebuffer_state, subpass.pColorAttachments[j]);
        }
        if (subpass.pDepthStencilAttachment) {
            TransitionAttachmentRefLayout(cb_state, framebuffer_state, *subpass.pDepthStencilAttachment);
        }
    }
}

// Transition the layout state for renderpass attachments based on the BeginRenderPass() call. This includes:
// 1. Transition into initialLayout state
// 2. Transition from initialLayout to layout used in subpass 0
void CoreChecks::TransitionBeginRenderPassLayouts(CMD_BUFFER_STATE *cb_state, const RENDER_PASS_STATE *render_pass_state,
                                                  FRAMEBUFFER_STATE *framebuffer_state) {
    // First record expected initialLayout as a potential initial layout usage.
    auto const rpci = render_pass_state->createInfo.ptr();
    for (uint32_t i = 0; i < rpci->attachmentCount; ++i) {
        auto *view_state = cb_state->GetActiveAttachmentImageViewState(i);
        if (view_state) {
            IMAGE_STATE *image_state = view_state->image_state.get();
            const auto initial_layout = rpci->pAttachments[i].initialLayout;
            const auto *attachment_description_stencil_layout =
                LvlFindInChain<VkAttachmentDescriptionStencilLayout>(rpci->pAttachments[i].pNext);
            if (attachment_description_stencil_layout) {
                const auto stencil_initial_layout = attachment_description_stencil_layout->stencilInitialLayout;
                VkImageSubresourceRange sub_range = view_state->normalized_subresource_range;
                sub_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                cb_state->SetImageInitialLayout(*image_state, sub_range, initial_layout);
                sub_range.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
                cb_state->SetImageInitialLayout(*image_state, sub_range, stencil_initial_layout);
            } else {
                cb_state->SetImageInitialLayout(*image_state, view_state->normalized_subresource_range, initial_layout);
            }
        }
    }
    // Now transition for first subpass (index 0)
    TransitionSubpassLayouts(cb_state, render_pass_state, 0, framebuffer_state);
}

bool CoreChecks::VerifyClearImageLayout(const CMD_BUFFER_STATE &cb_state, const IMAGE_STATE *image_state,
                                        const VkImageSubresourceRange &range, VkImageLayout dest_image_layout,
                                        const char *func_name) const {
    bool skip = false;
    if (strcmp(func_name, "vkCmdClearDepthStencilImage()") == 0) {
        if ((dest_image_layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && (dest_image_layout != VK_IMAGE_LAYOUT_GENERAL)) {
            skip |= LogError(image_state->image(), "VUID-vkCmdClearDepthStencilImage-imageLayout-00012",
                             "%s: Layout for cleared image is %s but can only be TRANSFER_DST_OPTIMAL or GENERAL.", func_name,
                             string_VkImageLayout(dest_image_layout));
        }

    } else {
        assert(strcmp(func_name, "vkCmdClearColorImage()") == 0);
        if (!IsExtEnabled(device_extensions.vk_khr_shared_presentable_image)) {
            if ((dest_image_layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && (dest_image_layout != VK_IMAGE_LAYOUT_GENERAL)) {
                skip |= LogError(image_state->image(), "VUID-vkCmdClearColorImage-imageLayout-00005",
                                 "%s: Layout for cleared image is %s but can only be TRANSFER_DST_OPTIMAL or GENERAL.", func_name,
                                 string_VkImageLayout(dest_image_layout));
            }
        } else {
            if ((dest_image_layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && (dest_image_layout != VK_IMAGE_LAYOUT_GENERAL) &&
                (dest_image_layout != VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR)) {
                skip |= LogError(
                    image_state->image(), "VUID-vkCmdClearColorImage-imageLayout-01394",
                    "%s: Layout for cleared image is %s but can only be TRANSFER_DST_OPTIMAL, SHARED_PRESENT_KHR, or GENERAL.",
                    func_name, string_VkImageLayout(dest_image_layout));
            }
        }
    }

    // Cast to const to prevent creation at validate time.
    const auto *subresource_map = cb_state.GetImageSubresourceLayoutMap(*image_state);
    if (subresource_map) {
        LayoutUseCheckAndMessage layout_check(dest_image_layout);
        auto normalized_isr = image_state->NormalizeSubresourceRange(range);
        // IncrementInterval skips over all the subresources that have the same state as we just checked, incrementing to
        // the next "constant value" range
        skip |= subresource_map->AnyInRange(
            normalized_isr, [this, &cb_state, &layout_check, func_name](const LayoutRange &range, const LayoutEntry &state) {
                bool subres_skip = false;
                if (!layout_check.Check(state)) {
                    const char *error_code = "VUID-vkCmdClearColorImage-imageLayout-00004";
                    if (strcmp(func_name, "vkCmdClearDepthStencilImage()") == 0) {
                        error_code = "VUID-vkCmdClearDepthStencilImage-imageLayout-00011";
                    } else {
                        assert(strcmp(func_name, "vkCmdClearColorImage()") == 0);
                    }
                    subres_skip |= LogError(cb_state.commandBuffer(), error_code,
                                            "%s: Cannot clear an image whose layout is %s and doesn't match the %s layout %s.",
                                            func_name, string_VkImageLayout(layout_check.expected_layout), layout_check.message,
                                            string_VkImageLayout(layout_check.layout));
                }
                return subres_skip;
            });
    }

    return skip;
}

template <typename ImageBarrier>
bool CoreChecks::UpdateCommandBufferImageLayoutMap(const CMD_BUFFER_STATE *cb_state, const Location &loc,
                                                   const ImageBarrier &img_barrier, const CommandBufferImageLayoutMap &current_map,
                                                   CommandBufferImageLayoutMap &layout_updates) const {
    bool skip = false;
    auto image_state = Get<IMAGE_STATE>(img_barrier.image);
    auto &write_subresource_map = layout_updates[image_state.get()];
    bool new_write = false;
    if (!write_subresource_map) {
        write_subresource_map = std::make_shared<ImageSubresourceLayoutMap>(*image_state);
        new_write = true;
    }
    const auto &current_subresource_map = current_map.find(image_state.get());
    const auto &read_subresource_map =
        (new_write && current_subresource_map != current_map.end()) ? (*current_subresource_map).second : write_subresource_map;
    // Validate aspects in isolation.
    // This is required when handling separate depth-stencil layouts.
    for (uint32_t aspect_index = 0; aspect_index < 32; aspect_index++) {
        VkImageAspectFlags test_aspect = 1u << aspect_index;
        if ((img_barrier.subresourceRange.aspectMask & test_aspect) == 0) {
            continue;
        }
        auto old_layout = NormalizeSynchronization2Layout(img_barrier.subresourceRange.aspectMask, img_barrier.oldLayout);

        LayoutUseCheckAndMessage layout_check(old_layout, test_aspect);
        auto normalized_isr = image_state->NormalizeSubresourceRange(img_barrier.subresourceRange);
        normalized_isr.aspectMask = test_aspect;
        skip |=
            read_subresource_map->AnyInRange(normalized_isr, [this, read_subresource_map, cb_state, &layout_check, &loc,
                                                              &img_barrier](const LayoutRange &range, const LayoutEntry &state) {
                bool subres_skip = false;
                if (!layout_check.Check(state)) {
                    const auto &vuid = GetImageBarrierVUID(loc, sync_vuid_maps::ImageError::kConflictingLayout);
                    auto subres = read_subresource_map->Decode(range.begin);
                    subres_skip =
                        LogError(cb_state->commandBuffer(), vuid,
                                 "%s %s cannot transition the layout of aspect=%d level=%d layer=%d from %s when the "
                                 "%s layout is %s.",
                                 loc.Message().c_str(), report_data->FormatHandle(img_barrier.image).c_str(), subres.aspectMask,
                                 subres.mipLevel, subres.arrayLayer, string_VkImageLayout(img_barrier.oldLayout),
                                 layout_check.message, string_VkImageLayout(layout_check.layout));
                }
                return subres_skip;
            });
        write_subresource_map->SetSubresourceRangeLayout(*cb_state, normalized_isr, img_barrier.newLayout);
    }
    return skip;
}

template bool CoreChecks::UpdateCommandBufferImageLayoutMap(const CMD_BUFFER_STATE *cb_state, const Location &loc,
                                                            const VkImageMemoryBarrier &img_barrier,
                                                            const CommandBufferImageLayoutMap &current_map,
                                                            CommandBufferImageLayoutMap &layout_updates) const;
template bool CoreChecks::UpdateCommandBufferImageLayoutMap(const CMD_BUFFER_STATE *cb_state, const Location &loc,
                                                            const VkImageMemoryBarrier2KHR &img_barrier,
                                                            const CommandBufferImageLayoutMap &current_map,
                                                            CommandBufferImageLayoutMap &layout_updates) const;

bool CoreChecks::FindLayouts(const IMAGE_STATE &image_state, std::vector<VkImageLayout> &layouts) const {
    const auto *layout_range_map = image_state.layout_range_map.get();
    if (!layout_range_map) return false;

    auto guard = layout_range_map->ReadLock();
    // TODO: FindLayouts function should mutate into a ValidatePresentableLayout with the loop wrapping the LogError
    //       from the caller. You can then use decode to add the subresource of the range::begin to the error message.

    // TODO: what is this test and what is it supposed to do?! -- the logic doesn't match the comment below?!

    // TODO: Make this robust for >1 aspect mask. Now it will just say ignore potential errors in this case.
    if (layout_range_map->size() >= (image_state.createInfo.arrayLayers * image_state.createInfo.mipLevels + 1)) {
        return false;
    }

    for (const auto &entry : *layout_range_map) {
        layouts.push_back(entry.second);
    }
    return true;
}

template <typename ImgBarrier>
void CoreChecks::RecordTransitionImageLayout(CMD_BUFFER_STATE *cb_state, const IMAGE_STATE *image_state,
                                             const ImgBarrier &mem_barrier, bool is_release_op) {
    if (enabled_features.core13.synchronization2) {
        if (mem_barrier.oldLayout == mem_barrier.newLayout) {
            return;
        }
    }
    auto normalized_isr = image_state->NormalizeSubresourceRange(mem_barrier.subresourceRange);

    VkImageLayout initial_layout = NormalizeSynchronization2Layout(mem_barrier.subresourceRange.aspectMask, mem_barrier.oldLayout);
    VkImageLayout new_layout = NormalizeSynchronization2Layout(mem_barrier.subresourceRange.aspectMask, mem_barrier.newLayout);

    // Layout transitions in external instance are not tracked, so don't validate initial layout.
    if (QueueFamilyIsExternal(mem_barrier.srcQueueFamilyIndex)) {
        initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    if (is_release_op) {
        cb_state->SetImageInitialLayout(*image_state, normalized_isr, initial_layout);
    } else {
        cb_state->SetImageLayout(*image_state, normalized_isr, new_layout, initial_layout);
    }
}

template <typename ImgBarrier>
void CoreChecks::TransitionImageLayouts(CMD_BUFFER_STATE *cb_state, uint32_t barrier_count, const ImgBarrier *barriers) {
    // For ownership transfers, the barrier is specified twice; as a release
    // operation on the yielding queue family, and as an acquire operation
    // on the acquiring queue family. This barrier may also include a layout
    // transition, which occurs 'between' the two operations. For validation
    // purposes it doesn't seem important which side performs the layout
    // transition, but it must not be performed twice. We'll arbitrarily
    // choose to perform it as part of the acquire operation.
    //
    // However, we still need to record initial layout for the "initial layout" validation
    for (uint32_t i = 0; i < barrier_count; i++) {
        const auto &mem_barrier = barriers[i];
        const bool is_release_op = cb_state->IsReleaseOp(mem_barrier);
        auto image_state = Get<IMAGE_STATE>(mem_barrier.image);
        if (image_state) {
            RecordTransitionImageLayout(cb_state, image_state.get(), mem_barrier, is_release_op);
        }
    }
}

template void CoreChecks::TransitionImageLayouts(CMD_BUFFER_STATE *cb_state, uint32_t barrier_count,
                                                 const VkImageMemoryBarrier *barrier);
template void CoreChecks::TransitionImageLayouts(CMD_BUFFER_STATE *cb_state, uint32_t barrier_count,
                                                 const VkImageMemoryBarrier2KHR *barrier);