/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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

#include "cc_state_tracker.h"
#include <vulkan/vulkan_core.h>
#include "core_validation.h"
#include "cc_sync_vuid_maps.h"
#include "error_message/error_strings.h"
#include "generated/error_location_helper.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/image_state.h"
#include "state_tracker/event_map.h"
#include "state_tracker/pipeline_state.h"
#include "state_tracker/query_state.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/shader_object_state.h"

// Location to add per-queue submit debug info if built with -D DEBUG_CAPTURE_KEYBOARD=ON
void CoreChecks::DebugCapture() {}

void CoreChecks::Created(vvl::CommandBuffer& cb) {
    cb.SetSubState(container_type, std::make_unique<core::CommandBufferSubState>(cb, *this));
}

void CoreChecks::Created(vvl::Queue& queue) {
    queue.SetSubState(container_type, std::make_unique<core::QueueSubState>(*this, queue));
}

namespace core {

CommandBufferSubState::CommandBufferSubState(vvl::CommandBuffer& cb, CoreChecks& validator)
    : vvl::CommandBufferSubState(cb), validator(validator) {
    ResetCBState();
}

void CommandBufferSubState::Begin(const VkCommandBufferBeginInfo& begin_info) {
    if (begin_info.pInheritanceInfo && base.IsSecondary()) {
        // If we are a secondary command-buffer and inheriting.  Update the items we should inherit.
        if (begin_info.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) {
            // Check for VkCommandBufferInheritanceViewportScissorInfoNV (VK_NV_inherited_viewport_scissor)
            auto p_inherited_viewport_scissor_info =
                vku::FindStructInPNextChain<VkCommandBufferInheritanceViewportScissorInfoNV>(begin_info.pInheritanceInfo->pNext);
            if (p_inherited_viewport_scissor_info != nullptr && p_inherited_viewport_scissor_info->viewportScissor2D) {
                auto p_viewport_depths = p_inherited_viewport_scissor_info->pViewportDepths;
                viewport.inherited_depths.assign(p_viewport_depths,
                                                 p_viewport_depths + p_inherited_viewport_scissor_info->viewportDepthCount);
            }
        }
    }
}

void CommandBufferSubState::UpdateActionPipelineState(LastBound& last_bound, const vvl::Pipeline& pipeline_state) {
    // Update the consumed viewport/scissor count.
    {
        const auto* viewport_state = pipeline_state.ViewportState();
        // If rasterization disabled (no viewport/scissors used), or the actual number of viewports/scissors is dynamic (unknown at
        // this time), then these are set to 0 to disable this checking.
        const auto has_dynamic_viewport_count = pipeline_state.IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
        const auto has_dynamic_scissor_count = pipeline_state.IsDynamic(CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
        const uint32_t pipeline_viewport_count =
            (has_dynamic_viewport_count || !viewport_state) ? 0 : viewport_state->viewportCount;
        const uint32_t pipeline_scissor_count = (has_dynamic_scissor_count || !viewport_state) ? 0 : viewport_state->scissorCount;

        // For each draw command D recorded to this command buffer, let
        //  * g_D be the graphics pipeline used
        //  * v_G be the viewportCount of g_D (0 if g_D disables rasterization or enables VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT)
        //  * s_G be the scissorCount  of g_D (0 if g_D disables rasterization or enables VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT)
        // Then this value is max(0, max(v_G for all D in cb), max(s_G for all D in cb))
        used_viewport_scissor_count = std::max({used_viewport_scissor_count, pipeline_viewport_count, pipeline_scissor_count});
        viewport.used_dynamic_count |= pipeline_state.IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
        scissor.used_dynamic_count |= pipeline_state.IsDynamic(CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
    }

    if (pipeline_state.IsDynamic(CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT) &&
        base.IsDynamicStateSet(CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT)) {
        base.SetActiveSubpassRasterizationSampleCount(base.dynamic_state_value.rasterization_samples);
    }

    if (last_bound.desc_set_pipeline_layout) {
        UpdateActiveSlotsState(last_bound, pipeline_state.active_slots);
    }
}

void CommandBufferSubState::UpdateActionShaderObjectState(LastBound& last_bound) {
    if (last_bound.desc_set_pipeline_layout) {
        for (uint32_t stage = 0; stage < kShaderObjectStageCount; ++stage) {
            const auto shader_object = last_bound.GetShaderState(static_cast<ShaderObjectStage>(stage));
            if (shader_object) {
                UpdateActiveSlotsState(last_bound, shader_object->active_slots);
            }
        }
    }
}

void CommandBufferSubState::UpdateActiveSlotsState(LastBound& last_bound, const ActiveSlotMap& active_slots) {
    for (const auto& [set_index, binding_req_map] : active_slots) {
        if (set_index >= last_bound.ds_slots.size()) {
            continue;
        }
        auto& ds_slot = last_bound.ds_slots[set_index];
        // Pull the set node
        auto& descriptor_set = ds_slot.ds_state;
        if (!descriptor_set) {
            continue;
        }

        // For the "bindless" style resource usage with many descriptors, need to optimize command <-> descriptor binding

        // We can skip updating the state if "nothing" has changed since the last validation.
        // See CoreChecks::ValidateActionState for more details.
        const bool need_update =  // Update if descriptor set (or contents) has changed
            ds_slot.validated_set != descriptor_set.get() ||
            ds_slot.validated_set_change_count != descriptor_set->GetChangeCount() ||
            (!base.dev_data.disabled[image_layout_validation] &&
             ds_slot.validated_set_image_layout_change_count != base.image_layout_change_count);
        if (need_update) {
            if (!base.dev_data.disabled[command_buffer_state] && !descriptor_set->IsPushDescriptor()) {
                base.AddChild(descriptor_set);
            }

            // Bind this set and its active descriptor resources to the command buffer
            descriptor_set->UpdateImageLayoutDrawStates(&base.dev_data, base, binding_req_map);

            ds_slot.validated_set = descriptor_set.get();
            ds_slot.validated_set_change_count = descriptor_set->GetChangeCount();
            ds_slot.validated_set_image_layout_change_count = base.image_layout_change_count;
        }
    }
}

// Common logic after any draw/dispatch/traceRays
void CommandBufferSubState::RecordActionCommand(LastBound& last_bound, const Location&) {
    if (last_bound.pipeline_state) {
        UpdateActionPipelineState(last_bound, *last_bound.pipeline_state);
    } else {
        UpdateActionShaderObjectState(last_bound);
    }
}

void CommandBufferSubState::RecordBindPipeline(VkPipelineBindPoint bind_point, vvl::Pipeline& pipeline) {
    if (bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        // Trash dynamic viewport/scissor state if pipeline defines static state and enabled rasterization.
        // akeley98 NOTE: There's a bit of an ambiguity in the spec, whether binding such a pipeline overwrites
        // the entire viewport (scissor) array, or only the subsection defined by the viewport (scissor) count.
        // I am taking the latter interpretation based on the implementation details of NVIDIA's Vulkan driver.
        const auto* viewport_state = pipeline.ViewportState();
        if (!pipeline.IsDynamic(CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT)) {
            viewport.trashed_count = true;
            if (viewport_state && (!pipeline.IsDynamic(CB_DYNAMIC_STATE_VIEWPORT))) {
                viewport.trashed_mask |= (1u << viewport_state->viewportCount) - 1u;
                // should become = ~uint32_t(0) if the other interpretation is correct.
            }
        }
        if (!pipeline.IsDynamic(CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT)) {
            scissor.trashed_count = true;
            if (viewport_state && (!pipeline.IsDynamic(CB_DYNAMIC_STATE_SCISSOR))) {
                scissor.trashed_mask |= (1u << viewport_state->scissorCount) - 1u;
                // should become = ~uint32_t(0) if the other interpretation is correct.
            }
        }
    }
}

void CommandBufferSubState::RecordSetViewport(uint32_t first_viewport, uint32_t viewport_count) {
    uint32_t bits = ((1u << viewport_count) - 1u) << first_viewport;
    viewport.mask |= bits;
    viewport.trashed_mask &= ~bits;
}

void CommandBufferSubState::RecordSetViewportWithCount(uint32_t viewport_count) {
    uint32_t bits = (1u << viewport_count) - 1u;
    viewport.count_mask |= bits;
    viewport.trashed_mask &= ~bits;
    viewport.trashed_count = false;
}

void CommandBufferSubState::RecordSetScissor(uint32_t first_scissor, uint32_t scissor_count) {
    uint32_t bits = ((1u << scissor_count) - 1u) << first_scissor;
    scissor.mask |= bits;
    scissor.trashed_mask &= ~bits;
}

void CommandBufferSubState::RecordSetScissorWithCount(uint32_t scissor_count) {
    uint32_t bits = (1u << scissor_count) - 1u;
    scissor.count_mask |= bits;
    scissor.trashed_mask &= ~bits;
    scissor.trashed_count = false;
}

void CommandBufferSubState::RecordNextSubpass(const VkSubpassBeginInfo&, const VkSubpassEndInfo*, const Location&) {
    ASSERT_AND_RETURN(base.active_render_pass);
    validator.TransitionSubpassLayouts(base, *base.active_render_pass, base.GetActiveSubpass());
}

void CommandBufferSubState::RecordBeginRenderPass(const VkRenderPassBeginInfo& render_pass_begin, const VkSubpassBeginInfo&,
                                                  const Location&) {
    ASSERT_AND_RETURN(base.active_render_pass);
    // transition attachments to the correct layouts for beginning of renderPass and first subpass
    validator.TransitionBeginRenderPassLayouts(base, *base.active_render_pass);
}

void CommandBufferSubState::RecordEndRendering(const VkRenderingEndInfoEXT* pRenderingEndInfo) {
    // Only track the first call to vkCmdEndRendering2EXT for pFragmentDensityOffsets, because they must match due to VU 10730
    if (fragment_density_offsets.empty()) {
        std::vector<VkOffset2D> new_offsets = {{0, 0}};
        if (pRenderingEndInfo) {
            const auto* fdm_offset_end_info =
                vku::FindStructInPNextChain<VkRenderPassFragmentDensityMapOffsetEndInfoEXT>(pRenderingEndInfo->pNext);
            if (fdm_offset_end_info) {
                new_offsets.resize(fdm_offset_end_info->fragmentDensityOffsetCount);
                for (uint32_t i = 0; i < fdm_offset_end_info->fragmentDensityOffsetCount; ++i) {
                    new_offsets[i] = fdm_offset_end_info->pFragmentDensityOffsets[i];
                }
            }
        }
        fragment_density_offsets = new_offsets;
    }
}

void CommandBufferSubState::RecordEndRenderPass(const VkSubpassEndInfo*, const Location&) {
    validator.TransitionFinalSubpassLayouts(base);
}

template <typename RegionType>
void CommandBufferSubState::RecordCopyBufferCommon(vvl::Buffer& src_buffer_state, vvl::Buffer& dst_buffer_state,
                                                   uint32_t region_count, const RegionType* regions, const Location& loc) {
    if (region_count == 0 || (!src_buffer_state.sparse && !dst_buffer_state.sparse)) {
        return;
    }

    using BufferRange = vvl::BindableMemoryTracker::BufferRange;

    std::vector<BufferRange> src_ranges(region_count);
    std::vector<BufferRange> dst_ranges(region_count);
    BufferRange src_ranges_bounds(regions[0].srcOffset, regions[0].srcOffset + regions[0].size);
    BufferRange dst_ranges_bounds(regions[0].dstOffset, regions[0].dstOffset + regions[0].size);

    for (uint32_t i = 0; i < region_count; ++i) {
        const RegionType& region = regions[i];
        src_ranges[i] = vvl::range<VkDeviceSize>{region.srcOffset, region.srcOffset + region.size};
        dst_ranges[i] = vvl::range<VkDeviceSize>{region.dstOffset, region.dstOffset + region.size};

        src_ranges_bounds.begin = std::min(src_ranges_bounds.begin, region.srcOffset);
        src_ranges_bounds.end = std::max(src_ranges_bounds.end, region.srcOffset + region.size);

        dst_ranges_bounds.begin = std::min(dst_ranges_bounds.begin, region.dstOffset);
        dst_ranges_bounds.end = std::max(dst_ranges_bounds.end, region.dstOffset + region.size);
    }

    auto queue_submit_validation = [this, &src_buffer_state, &dst_buffer_state, src_ranges = std::move(src_ranges),
                                    dst_ranges = std::move(dst_ranges), src_ranges_bounds, dst_ranges_bounds,
                                    loc](const class vvl::Queue& queue_state, const vvl::CommandBuffer& cb_state) -> bool {
        bool skip = false;

        auto src_vk_memory_to_ranges_map = src_buffer_state.GetBoundRanges(src_ranges_bounds, src_ranges);
        auto dst_vk_memory_to_ranges_map = dst_buffer_state.GetBoundRanges(dst_ranges_bounds, dst_ranges);

        for (const auto& [vk_memory, src_ranges] : src_vk_memory_to_ranges_map) {
            const auto find_mem_it = dst_vk_memory_to_ranges_map.find(vk_memory);
            if (find_mem_it == dst_vk_memory_to_ranges_map.end()) {
                continue;
            }
            // Some source and destination ranges are bound to the same VkDeviceMemory, look for overlaps.
            // Memory ranges are sorted, so looking for overlaps can be done in linear time

            auto& dst_ranges_vec = find_mem_it->second;
            auto src_ranges_it = src_ranges.cbegin();
            auto dst_ranges_it = dst_ranges_vec.cbegin();

            while (src_ranges_it != src_ranges.cend() && dst_ranges_it != dst_ranges_vec.cend()) {
                if (src_ranges_it->first.intersects(dst_ranges_it->first)) {
                    auto memory_range_overlap = src_ranges_it->first & dst_ranges_it->first;

                    const LogObjectList objlist(cb_state.Handle(), src_buffer_state.Handle(), dst_buffer_state.Handle(), vk_memory);
                    const bool is_2 = loc.function == vvl::Func::vkCmdCopyBuffer2 || loc.function == vvl::Func::vkCmdCopyBuffer2KHR;
                    const char* vuid = is_2 ? "VUID-VkCopyBufferInfo2-pRegions-00117" : "VUID-vkCmdCopyBuffer-pRegions-00117";
                    skip |= validator.LogError(
                        vuid, objlist, loc,
                        "Copy source buffer range %s (from buffer %s) and destination buffer range %s (from buffer %s) are "
                        "bound to the same memory (%s), "
                        "and end up overlapping on memory range %s.",
                        vvl::string_range(src_ranges_it->second).c_str(),
                        validator.FormatHandle(src_buffer_state.VkHandle()).c_str(),
                        vvl::string_range(dst_ranges_it->second).c_str(),
                        validator.FormatHandle(dst_buffer_state.VkHandle()).c_str(), validator.FormatHandle(vk_memory).c_str(),
                        vvl::string_range(memory_range_overlap).c_str());
                }

                if (src_ranges_it->first < dst_ranges_it->first) {
                    ++src_ranges_it;
                } else {
                    ++dst_ranges_it;
                }
            }
        }
        return skip;
    };

    queue_submit_functions.emplace_back(queue_submit_validation);
}

void CommandBufferSubState::RecordCopyBuffer(vvl::Buffer& src_buffer_state, vvl::Buffer& dst_buffer_state, uint32_t region_count,
                                             const VkBufferCopy* regions, const Location& loc) {
    RecordCopyBufferCommon(src_buffer_state, dst_buffer_state, region_count, regions, loc);
}

void CommandBufferSubState::RecordCopyBuffer2(vvl::Buffer& src_buffer_state, vvl::Buffer& dst_buffer_state, uint32_t region_count,
                                              const VkBufferCopy2* regions, const Location& loc) {
    RecordCopyBufferCommon(src_buffer_state, dst_buffer_state, region_count, regions, loc);
}

void CommandBufferSubState::RecordCopyImage(vvl::Image& src_image_state, vvl::Image& dst_image_state,
                                            VkImageLayout src_image_layout, VkImageLayout dst_image_layout, uint32_t region_count,
                                            const VkImageCopy* regions, const Location& loc) {
    for (const VkImageCopy& region : vvl::make_span(regions, region_count)) {
        base.TrackImageFirstLayout(src_image_state, RangeFromLayers(region.srcSubresource), region.srcOffset.z, region.extent.depth,
                                   src_image_layout);
        base.TrackImageFirstLayout(dst_image_state, RangeFromLayers(region.dstSubresource), region.dstOffset.z, region.extent.depth,
                                   dst_image_layout);
    }
}

void CommandBufferSubState::RecordCopyImage2(vvl::Image& src_image_state, vvl::Image& dst_image_state,
                                             VkImageLayout src_image_layout, VkImageLayout dst_image_layout, uint32_t region_count,
                                             const VkImageCopy2* regions, const Location& loc) {
    for (const VkImageCopy2& region : vvl::make_span(regions, region_count)) {
        base.TrackImageFirstLayout(src_image_state, RangeFromLayers(region.srcSubresource), region.srcOffset.z, region.extent.depth,
                                   src_image_layout);
        base.TrackImageFirstLayout(dst_image_state, RangeFromLayers(region.dstSubresource), region.dstOffset.z, region.extent.depth,
                                   dst_image_layout);
    }
}

void CommandBufferSubState::RecordCopyBufferToImage(vvl::Buffer&, vvl::Image& dst_image_state, VkImageLayout dst_image_layout,
                                                    uint32_t region_count, const VkBufferImageCopy* regions, const Location& loc) {
    for (const VkBufferImageCopy& region : vvl::make_span(regions, region_count)) {
        base.TrackImageFirstLayout(dst_image_state, RangeFromLayers(region.imageSubresource), region.imageOffset.z,
                                   region.imageExtent.depth, dst_image_layout);
    }
}

void CommandBufferSubState::RecordCopyBufferToImage2(vvl::Buffer&, vvl::Image& dst_image_state, VkImageLayout dst_image_layout,
                                                     uint32_t region_count, const VkBufferImageCopy2* regions,
                                                     const Location& loc) {
    for (const VkBufferImageCopy2& region : vvl::make_span(regions, region_count)) {
        base.TrackImageFirstLayout(dst_image_state, RangeFromLayers(region.imageSubresource), region.imageOffset.z,
                                   region.imageExtent.depth, dst_image_layout);
    }
}

void CommandBufferSubState::RecordCopyImageToBuffer(vvl::Image& src_image_state, vvl::Buffer&, VkImageLayout src_image_layout,
                                                    uint32_t region_count, const VkBufferImageCopy* regions, const Location& loc) {
    for (const VkBufferImageCopy& region : vvl::make_span(regions, region_count)) {
        base.TrackImageFirstLayout(src_image_state, RangeFromLayers(region.imageSubresource), region.imageOffset.z,
                                   region.imageExtent.depth, src_image_layout);
    }
}

void CommandBufferSubState::RecordCopyImageToBuffer2(vvl::Image& src_image_state, vvl::Buffer&, VkImageLayout src_image_layout,
                                                     uint32_t region_count, const VkBufferImageCopy2* regions,
                                                     const Location& loc) {
    for (const VkBufferImageCopy2& region : vvl::make_span(regions, region_count)) {
        base.TrackImageFirstLayout(src_image_state, RangeFromLayers(region.imageSubresource), region.imageOffset.z,
                                   region.imageExtent.depth, src_image_layout);
    }
}

void CommandBufferSubState::RecordBlitImage(vvl::Image& src_image_state, vvl::Image& dst_image_state,
                                            VkImageLayout src_image_layout, VkImageLayout dst_image_layout, uint32_t region_count,
                                            const VkImageBlit* regions, const Location& loc) {
    for (const VkImageBlit& region : vvl::make_span(regions, region_count)) {
        const int32_t src_depth_offset = (int32_t)std::min(region.srcOffsets[0].z, region.srcOffsets[1].z);
        const uint32_t src_depth_extent = (uint32_t)std::abs(region.srcOffsets[1].z - region.srcOffsets[0].z);
        base.TrackImageFirstLayout(src_image_state, RangeFromLayers(region.srcSubresource), src_depth_offset, src_depth_extent,
                                   src_image_layout);

        const int32_t dst_depth_offset = (int32_t)std::min(region.dstOffsets[0].z, region.dstOffsets[1].z);
        const uint32_t dst_depth_extent = (uint32_t)std::abs(region.dstOffsets[1].z - region.dstOffsets[0].z);
        base.TrackImageFirstLayout(dst_image_state, RangeFromLayers(region.dstSubresource), dst_depth_offset, dst_depth_extent,
                                   dst_image_layout);
    }
}

void CommandBufferSubState::RecordBlitImage2(vvl::Image& src_image_state, vvl::Image& dst_image_state,
                                             VkImageLayout src_image_layout, VkImageLayout dst_image_layout, uint32_t region_count,
                                             const VkImageBlit2* regions, const Location& loc) {
    for (const VkImageBlit2& region : vvl::make_span(regions, region_count)) {
        const int32_t src_depth_offset = (int32_t)std::min(region.srcOffsets[0].z, region.srcOffsets[1].z);
        const uint32_t src_depth_extent = (uint32_t)std::abs(region.srcOffsets[1].z - region.srcOffsets[0].z);
        base.TrackImageFirstLayout(src_image_state, RangeFromLayers(region.srcSubresource), src_depth_offset, src_depth_extent,
                                   src_image_layout);

        const int32_t dst_depth_offset = (int32_t)std::min(region.dstOffsets[0].z, region.dstOffsets[1].z);
        const uint32_t dst_depth_extent = (uint32_t)std::abs(region.dstOffsets[1].z - region.dstOffsets[0].z);
        base.TrackImageFirstLayout(dst_image_state, RangeFromLayers(region.dstSubresource), dst_depth_offset, dst_depth_extent,
                                   dst_image_layout);
    }
}

void CommandBufferSubState::RecordClearColorImage(vvl::Image& image_state, VkImageLayout image_layout, const VkClearColorValue*,
                                                  uint32_t range_count, const VkImageSubresourceRange* ranges, const Location&) {
    for (uint32_t i = 0; i < range_count; ++i) {
        base.TrackImageFirstLayout(image_state, ranges[i], 0, 0, image_layout);
    }
}

void CommandBufferSubState::RecordClearDepthStencilImage(vvl::Image& image_state, VkImageLayout image_layout,
                                                         const VkClearDepthStencilValue*, uint32_t range_count,
                                                         const VkImageSubresourceRange* ranges, const Location&) {
    for (uint32_t i = 0; i < range_count; ++i) {
        base.TrackImageFirstLayout(image_state, ranges[i], 0, 0, image_layout);
    }
}

void CommandBufferSubState::RecordClearAttachments(uint32_t attachment_count, const VkClearAttachment* pAttachments,
                                                   uint32_t rect_count, const VkClearRect* pRects, const Location& loc) {
    const vvl::RenderPass* rp_state = base.active_render_pass.get();
    if (!rp_state || base.IsPrimary()) {
        return;
    }

    std::shared_ptr<std::vector<VkClearRect>> clear_rect_copy;
    if (rp_state->use_dynamic_rendering_inherited) {
        for (uint32_t attachment_index = 0; attachment_index < attachment_count; attachment_index++) {
            const auto clear_desc = &pAttachments[attachment_index];
            auto colorAttachmentCount = rp_state->inheritance_rendering_info.colorAttachmentCount;
            int image_index = -1;
            if ((clear_desc->aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) && (clear_desc->colorAttachment < colorAttachmentCount)) {
                image_index = base.GetDynamicRenderingColorAttachmentIndex(clear_desc->colorAttachment);
            } else if (clear_desc->aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT)) {
                image_index = base.GetDynamicRenderingAttachmentIndex(AttachmentInfo::Type::Depth);
            } else if (clear_desc->aspectMask & (VK_IMAGE_ASPECT_STENCIL_BIT)) {
                image_index = base.GetDynamicRenderingAttachmentIndex(AttachmentInfo::Type::Stencil);
            }

            if (image_index != -1) {
                if (!clear_rect_copy) {
                    // We need a copy of the clear rectangles that will persist until the last lambda executes
                    // but we want to create it as lazily as possible
                    clear_rect_copy.reset(new std::vector<VkClearRect>(pRects, pRects + rect_count));
                }
                // if a secondary level command buffer inherits the framebuffer from the primary command buffer
                // (see VkCommandBufferInheritanceInfo), this validation must be deferred until queue submit time
                auto val_fn = [this, rect_count, clear_rect_copy, loc](const vvl::CommandBuffer& secondary,
                                                                       const vvl::CommandBuffer* prim_cb, const vvl::Framebuffer*) {
                    assert(rect_count == clear_rect_copy->size());
                    return validator.ValidateClearAttachmentExtent(
                        secondary, prim_cb->render_area,
                        prim_cb->active_render_pass->dynamic_rendering_begin_rendering_info.layerCount, rect_count,
                        clear_rect_copy->data(), loc);
                };
                cmd_execute_commands_functions.emplace_back(val_fn);
            }
        }
    } else if (!rp_state->use_dynamic_rendering) {
        const VkRenderPassCreateInfo2* renderpass_create_info = rp_state->create_info.ptr();
        const VkSubpassDescription2* subpass_desc = &renderpass_create_info->pSubpasses[base.GetActiveSubpass()];

        for (uint32_t attachment_index = 0; attachment_index < attachment_count; attachment_index++) {
            const auto clear_desc = &pAttachments[attachment_index];
            uint32_t fb_attachment = VK_ATTACHMENT_UNUSED;
            if ((clear_desc->aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) &&
                (clear_desc->colorAttachment < subpass_desc->colorAttachmentCount)) {
                fb_attachment = subpass_desc->pColorAttachments[clear_desc->colorAttachment].attachment;
            } else if ((clear_desc->aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) &&
                       subpass_desc->pDepthStencilAttachment) {
                fb_attachment = subpass_desc->pDepthStencilAttachment->attachment;
            }
            if (fb_attachment != VK_ATTACHMENT_UNUSED) {
                if (!clear_rect_copy) {
                    // We need a copy of the clear rectangles that will persist until the last lambda executes
                    // but we want to create it as lazily as possible
                    clear_rect_copy.reset(new std::vector<VkClearRect>(pRects, pRects + rect_count));
                }
                // if a secondary level command buffer inherits the framebuffer from the primary command buffer
                // (see VkCommandBufferInheritanceInfo), this validation must be deferred until queue submit time
                auto val_fn = [this, rect_count, clear_rect_copy, loc](const vvl::CommandBuffer& secondary,
                                                                       const vvl::CommandBuffer* prim_cb,
                                                                       const vvl::Framebuffer* fb) {
                    assert(rect_count == clear_rect_copy->size());
                    bool skip = false;

                    if (fb && prim_cb->IsPrimary()) {
                        skip |= validator.ValidateClearAttachmentExtent(secondary, prim_cb->render_area, fb->create_info.layers,
                                                                        rect_count, clear_rect_copy->data(), loc);
                    }
                    return skip;
                };
                cmd_execute_commands_functions.emplace_back(val_fn);
            }
        }
    }
}

void CommandBufferSubState::RecordSetEvent(VkEvent event, VkPipelineStageFlags2 stage_mask,
                                           const VkDependencyInfo* dependency_info) {
    vku::safe_VkDependencyInfo safe_dependency_info = {};
    if (dependency_info) {
        safe_dependency_info.initialize(dependency_info);
    } else {
        // Set sType to invalid, so following code can check sType to see if the struct is valid
        safe_dependency_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    }
    event_updates.emplace_back([event, stage_mask, safe_dependency_info](vvl::CommandBuffer&, bool do_validate,
                                                                         EventMap& local_event_signal_info, VkQueue,
                                                                         const Location& loc) {
        local_event_signal_info[event] = EventInfo{stage_mask, true, safe_dependency_info};
        return false;  // skip
    });
}

void CommandBufferSubState::RecordResetEvent(VkEvent event, VkPipelineStageFlags2) {
    event_updates.emplace_back(
        [event](vvl::CommandBuffer&, bool do_validate, EventMap& local_event_signal_info, VkQueue, const Location& loc) {
            local_event_signal_info[event] = EventInfo{VK_PIPELINE_STAGE_2_NONE, false};
            return false;  // skip
        });
}

void CommandBufferSubState::RecordWaitEvents(uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags2 src_stage_mask,
                                             const VkDependencyInfo* dependency_info, const Location& loc) {
    // vvl::CommandBuffer will add to the events vector. TODO this is now incorrect
    auto first_event_index = base.events.size();
    auto event_added_count = eventCount;

    vku::safe_VkDependencyInfo safe_dependency_info = {};
    if (dependency_info) {
        safe_dependency_info.initialize(dependency_info);
    } else {
        // Set sType to invalid, so following code can check sType to see if the struct is valid
        safe_dependency_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    }

    event_updates.emplace_back(
        [event_added_count, first_event_index, src_stage_mask, safe_dependency_info](
            vvl::CommandBuffer& cb_state, bool do_validate, EventMap& local_event_signal_info, VkQueue queue, const Location& loc) {
            if (!do_validate) return false;
            return CoreChecks::ValidateWaitEventsAtSubmit(cb_state, event_added_count, first_event_index, src_stage_mask,
                                                          safe_dependency_info, local_event_signal_info, queue, loc);
        });
}

void CommandBufferSubState::RecordBarriers(uint32_t buffer_barrier_count, const VkBufferMemoryBarrier* buffer_barriers,
                                           uint32_t image_barrier_count, const VkImageMemoryBarrier* image_barriers,
                                           VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
                                           const Location& loc) {
    for (uint32_t i = 0; i < buffer_barrier_count; i++) {
        Location barrier_loc(loc.function, vvl::Struct::VkBufferMemoryBarrier, vvl::Field::pBufferMemoryBarriers, i);
        const BufferBarrier barrier(buffer_barriers[i], src_stage_mask, dst_stage_mask);
        validator.RecordBarrierValidationInfo(barrier_loc, base, barrier, qfo_transfer_buffer_barriers);
    }
    for (uint32_t i = 0; i < image_barrier_count; i++) {
        auto image_state = base.dev_data.Get<vvl::Image>(image_barriers[i].image);
        ASSERT_AND_CONTINUE(image_state);

        Location barrier_loc(loc.function, vvl::Struct::VkImageMemoryBarrier, vvl::Field::pImageMemoryBarriers, i);
        const ImageBarrier img_barrier(image_barriers[i], src_stage_mask, dst_stage_mask);
        validator.RecordBarrierValidationInfo(barrier_loc, base, img_barrier, *image_state, qfo_transfer_image_barriers);
        validator.EnqueueValidateImageBarrierAttachment(barrier_loc, *this, img_barrier);
        validator.EnqueueValidateDynamicRenderingImageBarrierLayouts(barrier_loc, base, img_barrier);

        // Update layouts at the end. Submit time enqueuing logic above needs pre-update layout map.
        validator.RecordTransitionImageLayout(base, img_barrier, *image_state);
    }
}

void CommandBufferSubState::RecordBarriers2(const VkDependencyInfo& dep_info, const Location& loc) {
    for (uint32_t i = 0; i < dep_info.bufferMemoryBarrierCount; i++) {
        Location barrier_loc(loc.function, vvl::Struct::VkBufferMemoryBarrier2, vvl::Field::pBufferMemoryBarriers, i);
        const BufferBarrier barrier(dep_info.pBufferMemoryBarriers[i]);
        validator.RecordBarrierValidationInfo(barrier_loc, base, barrier, qfo_transfer_buffer_barriers);
    }
    for (uint32_t i = 0; i < dep_info.imageMemoryBarrierCount; i++) {
        auto image_state = base.dev_data.Get<vvl::Image>(dep_info.pImageMemoryBarriers[i].image);
        ASSERT_AND_CONTINUE(image_state);

        Location barrier_loc(loc.function, vvl::Struct::VkImageMemoryBarrier2, vvl::Field::pImageMemoryBarriers, i);
        const ImageBarrier img_barrier(dep_info.pImageMemoryBarriers[i]);
        validator.RecordBarrierValidationInfo(barrier_loc, base, img_barrier, *image_state, qfo_transfer_image_barriers);
        validator.EnqueueValidateImageBarrierAttachment(barrier_loc, *this, img_barrier);
        validator.EnqueueValidateDynamicRenderingImageBarrierLayouts(barrier_loc, base, img_barrier);

        // Update layouts at the end. Submit time enqueuing logic above needs pre-update layout map.
        validator.RecordTransitionImageLayout(base, img_barrier, *image_state);
    }
    if (const auto tensor_barrier_dep_info = vku::FindStructInPNextChain<VkTensorDependencyInfoARM>(dep_info.pNext)) {
        const Location tensor_dep_info_loc(loc.function, vvl::Struct::VkTensorDependencyInfoARM, vvl::Field::pNext);
        for (uint32_t i = 0; i < tensor_barrier_dep_info->tensorMemoryBarrierCount; ++i) {
            const TensorBarrier barrier(tensor_barrier_dep_info->pTensorMemoryBarriers[i]);
            base.tensor_barriers.emplace_back(barrier);
        }
    }

}

static void SetQueryState(const QueryObject& object, QueryState value, QueryMap* local_query_to_state_map) {
    (*local_query_to_state_map)[object] = value;
}

static void SetQueryStateMulti(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, uint32_t perf_query_pass,
                               QueryState value, QueryMap* local_query_to_state_map) {
    for (uint32_t i = 0; i < queryCount; i++) {
        QueryObject query_obj = {queryPool, firstQuery + i, perf_query_pass};
        (*local_query_to_state_map)[query_obj] = value;
    }
}

void CommandBufferSubState::RecordBeginQuery(const QueryObject& query_obj, const Location& loc) {
    query_updates.emplace_back([this, query_obj, loc](vvl::CommandBuffer& cb_state_arg, bool do_validate,
                                                      VkQueryPool& first_perf_query_pool, uint32_t perf_query_pass,
                                                      QueryMap* local_query_to_state_map) {
        bool skip = false;
        // Need to enqueue validation before we update
        if (do_validate) {
            skip |= validator.ValidatePerformanceQuery(cb_state_arg, query_obj, loc, first_perf_query_pool, perf_query_pass,
                                                       local_query_to_state_map);
            skip |= validator.VerifyQueryIsReset(cb_state_arg, query_obj, loc, perf_query_pass, local_query_to_state_map);
        }

        SetQueryState(QueryObject(query_obj, perf_query_pass), QUERYSTATE_RUNNING, local_query_to_state_map);
        return skip;
    });
}

void CommandBufferSubState::RecordEndQuery(const QueryObject& query_obj, const Location& loc) {
    query_updates.emplace_back([this, query_obj, loc](vvl::CommandBuffer& cb_state_arg, bool do_validate, VkQueryPool&,
                                                      uint32_t perf_query_pass, QueryMap* local_query_to_state_map) {
        bool skip = false;
        if (do_validate) {
            auto query_pool_state = base.dev_data.Get<vvl::QueryPool>(query_obj.pool);
            ASSERT_AND_RETURN_SKIP(query_pool_state);
            if (query_pool_state->has_perf_scope_command_buffer && cb_state_arg.command_count != query_obj.end_command_index) {
                const LogObjectList objlist(cb_state_arg.Handle(), query_pool_state->Handle());
                skip |= validator.LogError(
                    "VUID-vkCmdEndQuery-queryPool-03227", objlist, loc,
                    "Query pool %s was created with a counter of scope "
                    "VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_BUFFER_KHR but the end of the query is not the last "
                    "command in the command buffer %s.",
                    validator.FormatHandle(query_obj.pool).c_str(), validator.FormatHandle(cb_state_arg).c_str());
            }
        }

        SetQueryState(QueryObject(query_obj, perf_query_pass), QUERYSTATE_ENDED, local_query_to_state_map);
        return skip;
    });
}

void CommandBufferSubState::RecordWriteTimestamp(const QueryObject& query_obj, const Location& loc) {
    query_updates.emplace_back([this, query_obj, loc](vvl::CommandBuffer& cb_state_arg, bool do_validate, VkQueryPool&,
                                                      uint32_t perf_query_pass, QueryMap* local_query_to_state_map) {
        bool skip = false;
        if (do_validate) {
            skip |= validator.VerifyQueryIsReset(cb_state_arg, query_obj, loc, perf_query_pass, local_query_to_state_map);
        }
        SetQueryState(QueryObject(query_obj, perf_query_pass), QUERYSTATE_ENDED, local_query_to_state_map);
        return skip;
    });
}

void CommandBufferSubState::RecordEndQueries(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    query_updates.emplace_back([queryPool, firstQuery, queryCount](vvl::CommandBuffer& cb_state_arg, bool do_validate, VkQueryPool&,
                                                                   uint32_t perf_query_pass, QueryMap* local_query_to_state_map) {
        SetQueryStateMulti(queryPool, firstQuery, queryCount, perf_query_pass, QUERYSTATE_ENDED, local_query_to_state_map);
        return false;
    });
}

static QueryState GetLocalQueryState(const QueryMap* local_query_to_state_map, VkQueryPool queryPool, uint32_t queryIndex,
                                     uint32_t perf_query_pass) {
    QueryObject query = QueryObject(queryPool, queryIndex, perf_query_pass);

    auto iter = local_query_to_state_map->find(query);
    if (iter != local_query_to_state_map->end()) return iter->second;

    return QUERYSTATE_UNKNOWN;
}

void CommandBufferSubState::RecordResetQueryPool(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                                                 bool is_perf_query, const Location& loc) {
    query_updates.emplace_back(
        [queryPool, firstQuery, queryCount, is_perf_query, loc](vvl::CommandBuffer& cb_state_arg, bool do_validate, VkQueryPool&,
                                                                uint32_t perf_query_pass, QueryMap* local_query_to_state_map) {
            bool skip = false;
            if (is_perf_query && do_validate) {
                const auto& state_data = cb_state_arg.dev_data;
                for (uint32_t i = 0; i < queryCount; i++) {
                    QueryState state = GetLocalQueryState(local_query_to_state_map, queryPool, firstQuery + i, perf_query_pass);
                    if (state == QUERYSTATE_ENDED) {
                        const LogObjectList objlist(cb_state_arg.Handle(), queryPool);
                        skip |= state_data.LogError("VUID-vkCmdResetQueryPool-firstQuery-02862", objlist, loc,
                                                    "Query index %" PRIu32 " was begun and reset in the same command buffer.",
                                                    firstQuery + i);
                        break;
                    }
                }
            }
            SetQueryStateMulti(queryPool, firstQuery, queryCount, perf_query_pass, QUERYSTATE_RESET, local_query_to_state_map);
            return skip;
        });
}

void CommandBufferSubState::RecordCopyQueryPoolResults(vvl::QueryPool& pool_state, vvl::Buffer&, uint32_t first_query,
                                                       uint32_t query_count, VkDeviceSize, VkDeviceSize, VkQueryResultFlags flags,
                                                       const Location& loc) {
    query_updates.emplace_back(
        [this, &pool_state, first_query, query_count, flags, loc](vvl::CommandBuffer& cb_state_arg, bool do_validate, VkQueryPool&,
                                                                  uint32_t perf_query_pass, QueryMap* local_query_to_state_map) {
            if (!do_validate) {
                return false;
            }
            bool skip = false;
            for (uint32_t i = 0; i < query_count; i++) {
                QueryState state =
                    GetLocalQueryState(local_query_to_state_map, pool_state.VkHandle(), first_query + i, perf_query_pass);
                QueryResultType result_type = pool_state.GetQueryResultType(state, flags);
                if (result_type != QUERYRESULT_SOME_DATA && result_type != QUERYRESULT_UNKNOWN) {
                    const LogObjectList objlist(cb_state_arg.Handle(), pool_state.Handle());
                    skip |= validator.LogError("VUID-vkCmdCopyQueryPoolResults-None-08752", objlist, loc,
                                               "Requesting a copy from query to buffer on %s query %" PRIu32 ": %s",
                                               validator.FormatHandle(pool_state.Handle()).c_str(), first_query + i,
                                               string_QueryResultType(result_type));
                }
            }

            skip |= validator.ValidateQueryPoolWasReset(pool_state, first_query, query_count, loc, local_query_to_state_map,
                                                        perf_query_pass);

            return skip;
        });
}

void CommandBufferSubState::RecordWriteAccelerationStructuresProperties(VkQueryPool queryPool, uint32_t firstQuery,
                                                                        uint32_t accelerationStructureCount, const Location& loc) {
    query_updates.emplace_back([this, accelerationStructureCount, firstQuery, queryPool, loc](
                                   vvl::CommandBuffer& cb_state_arg, bool do_validate, VkQueryPool&, uint32_t perf_query_pass,
                                   QueryMap* local_query_to_state_map) {
        bool skip = false;
        if (do_validate) {
            for (uint32_t i = 0; i < accelerationStructureCount; i++) {
                QueryObject query_obj = {queryPool, firstQuery + i, perf_query_pass};
                skip |= validator.VerifyQueryIsReset(cb_state_arg, query_obj, loc, perf_query_pass, local_query_to_state_map);
            }
        }
        SetQueryStateMulti(queryPool, firstQuery, accelerationStructureCount, perf_query_pass, QUERYSTATE_ENDED,
                           local_query_to_state_map);
        return skip;
    });
}

void CommandBufferSubState::RecordVideoInlineQueries(const VkVideoInlineQueryInfoKHR& query_info) {
    query_updates.emplace_back([query_info](vvl::CommandBuffer& cb_state_arg, bool do_validate, VkQueryPool&,
                                            uint32_t perf_query_pass, QueryMap* local_query_to_state_map) {
        for (uint32_t i = 0; i < query_info.queryCount; i++) {
            SetQueryState(QueryObject(query_info.queryPool, query_info.firstQuery + i), QUERYSTATE_ENDED, local_query_to_state_map);
        }
        return false;
    });
}

void CommandBufferSubState::RecordBeginVideoCoding(vvl::VideoSession& vs_state, const VkVideoBeginCodingInfoKHR& begin_info,
                                                   const Location& loc) {
    if (begin_info.referenceSlotCount > 0) {
        std::vector<vvl::VideoReferenceSlot> expected_slots{};
        expected_slots.reserve(begin_info.referenceSlotCount);

        for (uint32_t i = 0; i < begin_info.referenceSlotCount; ++i) {
            if (begin_info.pReferenceSlots[i].slotIndex >= 0) {
                expected_slots.emplace_back(*validator.device_state, *vs_state.profile, begin_info.pReferenceSlots[i], false);
            }
        }

        // Enqueue submission time validation of DPB slots
        base.video_session_updates[vs_state.VkHandle()].emplace_back(
            [this, expected_slots, loc](const vvl::VideoSession* vs_state, vvl::VideoSessionDeviceState& dev_state,
                                        bool do_validate) {
                if (!do_validate) return false;
                bool skip = false;
                for (const auto& slot : expected_slots) {
                    if (!dev_state.IsSlotActive(slot.index)) {
                        skip |= validator.LogError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239", vs_state->Handle(), loc,
                                                   "DPB slot index %d is not active in %s.", slot.index,
                                                   validator.FormatHandle(*vs_state).c_str());
                    } else if (slot.resource && !dev_state.IsSlotPicture(slot.index, slot.resource)) {
                        skip |= validator.LogError("VUID-vkCmdBeginVideoCodingKHR-pPictureResource-07265", vs_state->Handle(), loc,
                                                   "DPB slot index %d of %s is not currently associated with the specified "
                                                   "video picture resource: %s, layer %" PRIu32 ", offset (%s), extent (%s).",
                                                   slot.index, validator.FormatHandle(*vs_state).c_str(),
                                                   validator.FormatHandle(slot.resource.image_state->Handle()).c_str(),
                                                   slot.resource.range.baseArrayLayer,
                                                   string_VkOffset2D(slot.resource.coded_offset).c_str(),
                                                   string_VkExtent2D(slot.resource.coded_extent).c_str());
                    }
                }
                return skip;
            });
    }

    if (vs_state.IsEncode()) {
        vku::safe_VkVideoBeginCodingInfoKHR safe_begin_info(&begin_info);

        // Enqueue submission time validation of rate control state
        base.video_session_updates[vs_state.VkHandle()].emplace_back(
            [this, safe_begin_info, loc](const vvl::VideoSession* vs_state, vvl::VideoSessionDeviceState& dev_state,
                                         bool do_validate) {
                if (!do_validate) return false;
                return dev_state.ValidateRateControlState(validator, vs_state, safe_begin_info, loc);
            });
    }
}

void CommandBufferSubState::EnqueueVerifyVideoSessionInitialized(vvl::VideoSession& vs_state, const Location& loc,
                                                                 const char* vuid) {
    base.video_session_updates[vs_state.VkHandle()].emplace_back(
        [this, loc, vuid](const vvl::VideoSession* vs_state, vvl::VideoSessionDeviceState& dev_state, bool do_validate) {
            bool skip = false;
            if (!dev_state.IsInitialized()) {
                skip |= validator.LogError(vuid, vs_state->Handle(), loc, "Bound video session %s is uninitialized.",
                                           validator.FormatHandle(*vs_state).c_str());
            }
            return skip;
        });
}

void CommandBufferSubState::EnqueueVerifyVideoInlineQueryUnavailable(const VkVideoInlineQueryInfoKHR& query_info,
                                                                     vvl::Func command) {
    if (validator.disabled[query_validation]) return;
    query_updates.emplace_back([this, query_info, command](vvl::CommandBuffer& cb_state_arg, bool do_validate, VkQueryPool&,
                                                           uint32_t perf_query_pass, QueryMap* local_query_to_state_map) {
        if (!do_validate) return false;
        bool skip = false;
        for (uint32_t i = 0; i < query_info.queryCount; i++) {
            QueryObject query_obj = {query_info.queryPool, query_info.firstQuery + i, perf_query_pass};
            skip |= validator.VerifyQueryIsReset(cb_state_arg, query_obj, command, perf_query_pass, local_query_to_state_map);
        }
        return skip;
    });
}

void CommandBufferSubState::RecordControlVideoCoding(vvl::VideoSession& vs_state, const VkVideoCodingControlInfoKHR& control_info,
                                                     const Location& loc) {
    if ((control_info.flags & VK_VIDEO_CODING_CONTROL_RESET_BIT_KHR) == 0) {
        EnqueueVerifyVideoSessionInitialized(vs_state, loc, "VUID-vkCmdControlVideoCodingKHR-flags-07017");
    }
}

void CommandBufferSubState::RecordDecodeVideo(vvl::VideoSession& vs_state, const VkVideoDecodeInfoKHR& decode_info,
                                              const Location& loc) {
    EnqueueVerifyVideoSessionInitialized(vs_state, loc, "VUID-vkCmdDecodeVideoKHR-None-07011");

    if (vs_state.GetCodecOp() == VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR) {
        std::vector<vvl::VideoReferenceSlot> reference_slots{};
        reference_slots.reserve(decode_info.referenceSlotCount);
        for (uint32_t i = 0; i < decode_info.referenceSlotCount; ++i) {
            reference_slots.emplace_back(*validator.device_state, *vs_state.profile, decode_info.pReferenceSlots[i]);
        }

        // Enqueue submission time validation of picture kind (frame, top field, bottom field) for H.264
        base.video_session_updates[vs_state.VkHandle()].emplace_back(
            [this, reference_slots, loc](const vvl::VideoSession* vs_state, vvl::VideoSessionDeviceState& dev_state,
                                         bool do_validate) {
                if (!do_validate) return false;
                bool skip = false;
                const auto log_picture_kind_error = [&](const vvl::VideoReferenceSlot& slot, const char* vuid,
                                                        const char* picture_kind) -> bool {
                    return validator.LogError(vuid, vs_state->Handle(), loc,
                                              "DPB slot index %d of %s does not currently contain a %s with the specified "
                                              "video picture resource: %s, layer %" PRIu32 ", offset (%s), extent (%s).",
                                              slot.index, validator.FormatHandle(*vs_state).c_str(), picture_kind,
                                              validator.FormatHandle(slot.resource.image_state->Handle()).c_str(),
                                              slot.resource.range.baseArrayLayer,
                                              string_VkOffset2D(slot.resource.coded_offset).c_str(),
                                              string_VkExtent2D(slot.resource.coded_extent).c_str());
                };
                for (const auto& slot : reference_slots) {
                    if (slot.picture_id.IsFrame() &&
                        !dev_state.IsSlotPicture(slot.index, vvl::VideoPictureID::Frame(), slot.resource)) {
                        skip |= log_picture_kind_error(slot, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07266", "frame");
                    }
                    if (slot.picture_id.ContainsTopField() &&
                        !dev_state.IsSlotPicture(slot.index, vvl::VideoPictureID::TopField(), slot.resource)) {
                        skip |= log_picture_kind_error(slot, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07267", "top field");
                    }
                    if (slot.picture_id.ContainsBottomField() &&
                        !dev_state.IsSlotPicture(slot.index, vvl::VideoPictureID::BottomField(), slot.resource)) {
                        skip |= log_picture_kind_error(slot, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07268", "bottom field");
                    }
                }
                return skip;
            });
    }

    if (vs_state.create_info.flags & VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR) {
        const auto inline_query_info = vku::FindStructInPNextChain<VkVideoInlineQueryInfoKHR>(decode_info.pNext);
        if (inline_query_info != nullptr && inline_query_info->queryPool != VK_NULL_HANDLE) {
            EnqueueVerifyVideoInlineQueryUnavailable(*inline_query_info, loc.function);
        }
    }
}

void CommandBufferSubState::RecordEncodeVideo(vvl::VideoSession& vs_state, const VkVideoEncodeInfoKHR& encode_info,
                                              const Location& loc) {
    EnqueueVerifyVideoSessionInitialized(vs_state, loc, "VUID-vkCmdEncodeVideoKHR-None-07012");

    // For encode sessions also verify encode quality level match for the bound parameters object
    if (vs_state.IsEncode() && base.bound_video_session_parameters) {
        if (!base.video_encode_quality_level.has_value()) {
            // If we already know the current encode quality level already at command buffer recording
            // time, because it was set in this command buffer, then that was already checked outside
            // so we only have to do submit-time validation if that's not the case
            base.video_session_updates[vs_state.VkHandle()].emplace_back(
                [this, vsp_state = base.bound_video_session_parameters, loc](
                    const vvl::VideoSession* vs_state, vvl::VideoSessionDeviceState& dev_state, bool do_validate) {
                    if (!do_validate) return false;
                    bool skip = false;
                    if (vsp_state->GetEncodeQualityLevel() != dev_state.GetEncodeQualityLevel()) {
                        const LogObjectList objlist(vs_state->Handle(), vsp_state->Handle());
                        skip |= validator.LogError("VUID-vkCmdEncodeVideoKHR-None-08318", objlist, loc,
                                                   "The currently configured encode quality level (%" PRIu32
                                                   ") for %s "
                                                   "does not match the encode quality level (%" PRIu32 ") %s was created with.",
                                                   dev_state.GetEncodeQualityLevel(), validator.FormatHandle(*vs_state).c_str(),
                                                   vsp_state->GetEncodeQualityLevel(), validator.FormatHandle(*vsp_state).c_str());
                    }
                    return skip;
                });
        }
    }

    if (vs_state.create_info.flags & VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR) {
        const auto inline_query_info = vku::FindStructInPNextChain<VkVideoInlineQueryInfoKHR>(encode_info.pNext);
        if (inline_query_info != nullptr && inline_query_info->queryPool != VK_NULL_HANDLE) {
            EnqueueVerifyVideoInlineQueryUnavailable(*inline_query_info, loc.function);
        }
    }
}

void CommandBufferSubState::Retire(uint32_t perf_submit_pass,
                                   const std::function<bool(const QueryObject&)>& is_query_updated_after) {
    QueryMap local_query_to_state_map;
    VkQueryPool first_pool = VK_NULL_HANDLE;
    for (auto& function : query_updates) {
        function(base, /*do_validate*/ false, first_pool, perf_submit_pass, &local_query_to_state_map);
    }

    for (const auto& [query_object, query_state] : local_query_to_state_map) {
        if (query_state == QUERYSTATE_ENDED && !is_query_updated_after(query_object)) {
            auto query_pool_state = base.dev_data.Get<vvl::QueryPool>(query_object.pool);
            if (!query_pool_state) continue;
            query_pool_state->SetQueryState(query_object.slot, query_object.perf_pass, QUERYSTATE_AVAILABLE);
        }
    }
}

void CommandBufferSubState::Reset(const Location& loc) { ResetCBState(); }

void CommandBufferSubState::Destroy() { ResetCBState(); }

void CommandBufferSubState::ResetCBState() {
    // QFO Tranfser
    qfo_transfer_image_barriers.Reset();
    qfo_transfer_buffer_barriers.Reset();

    // VK_EXT_nested_command_buffer
    nesting_level = 0;

    // Submit time validation
    queue_submit_functions.clear();
    submit_validate_dynamic_rendering_barrier_subresources.clear();
    event_updates.clear();
    cmd_execute_commands_functions.clear();
    query_updates.clear();

    // Inherited Viewport/Scissor
    used_viewport_scissor_count = 0;
    viewport.mask = 0;
    viewport.count_mask = 0;
    viewport.trashed_mask = 0;
    viewport.trashed_count = false;
    viewport.used_dynamic_count = false;
    viewport.inherited_depths.clear();
    scissor.mask = 0;
    scissor.count_mask = 0;
    scissor.trashed_mask = 0;
    scissor.trashed_count = false;
    scissor.used_dynamic_count = false;
}

void CommandBufferSubState::RecordExecuteCommand(vvl::CommandBuffer& secondary_command_buffer, uint32_t, const Location&) {
    auto& secondary_sub_state = SubState(secondary_command_buffer);
    if (secondary_command_buffer.IsSecondary()) {
        nesting_level = std::max(nesting_level, secondary_sub_state.nesting_level + 1);
    }

    for (auto& function : secondary_sub_state.event_updates) {
        event_updates.push_back(function);
    }

    for (auto& function : secondary_sub_state.queue_submit_functions) {
        queue_submit_functions.push_back(function);
    }

    // State is trashed after executing secondary command buffers.
    // Importantly, this function runs after CoreChecks::PreCallValidateCmdExecuteCommands.
    viewport.trashed_mask = vvl::kU32Max;
    viewport.trashed_count = true;
    scissor.trashed_mask = vvl::kU32Max;
    scissor.trashed_count = true;

    // Add a query update that runs all the query updates that happen in the sub command buffer.
    // This avoids locking ambiguity because primary command buffers are locked when these
    // callbacks run, but secondary command buffers are not.
    const VkCommandBuffer sub_command_buffer = secondary_command_buffer.VkHandle();
    query_updates.emplace_back([sub_command_buffer](vvl::CommandBuffer& cb_state_arg, bool do_validate,
                                                    VkQueryPool& first_perf_query_pool, uint32_t perf_query_pass,
                                                    QueryMap* local_query_to_state_map) {
        bool skip = false;
        auto secondary_cb_state_arg = cb_state_arg.dev_data.GetWrite<vvl::CommandBuffer>(sub_command_buffer);
        auto& secondary_sub_state_arg = SubState(*secondary_cb_state_arg);
        for (auto& function : secondary_sub_state_arg.query_updates) {
            skip |=
                function(*secondary_cb_state_arg, do_validate, first_perf_query_pool, perf_query_pass, local_query_to_state_map);
        }
        return skip;
    });
}

void CommandBufferSubState::Submit(vvl::Queue& queue_state, uint32_t perf_submit_pass, const Location& loc) {
    for (auto& func : queue_submit_functions) {
        func(queue_state, base);
    }

    // Update vvl::Event with src_stage from the last recorded SetEvent.
    // Ultimately, it tracks the last SetEvent for the entire submission.
    {
        EventMap local_event_signal_info;
        for (const auto& function : event_updates) {
            function(base, /*do_validate*/ false, local_event_signal_info,
                     VK_NULL_HANDLE /* when do_validate is false then wait handler is inactive */, loc);
        }
        for (const auto& [event, info] : local_event_signal_info) {
            auto event_state = base.dev_data.Get<vvl::Event>(event);
            event_state->signaled = info.signal;
            event_state->dependency_info = info.dependency_info;
            event_state->signal_src_stage_mask = info.src_stage_mask;
            event_state->signaling_queue = queue_state.VkHandle();
        }
    }

    // Update vvl::QueryPool with a query state at the end of the command buffer.
    // Ultimately, it tracks the final query state for the entire submission.
    {
        VkQueryPool first_pool = VK_NULL_HANDLE;
        QueryMap local_query_to_state_map;
        for (auto& function : query_updates) {
            function(base, /*do_validate*/ false, first_pool, perf_submit_pass, &local_query_to_state_map);
        }
        for (const auto& [query_object, query_state] : local_query_to_state_map) {
            auto query_pool_state = base.dev_data.Get<vvl::QueryPool>(query_object.pool);
            if (!query_pool_state) continue;
            query_pool_state->SetQueryState(query_object.slot, query_object.perf_pass, query_state);
        }
    }
}

void CommandBufferSubState::SubmitTimeValidate() {
    for (const auto& [image, subresources] : submit_validate_dynamic_rendering_barrier_subresources) {
        const auto image_state = validator.Get<vvl::Image>(image);
        if (!image_state) {
            continue;
        }
        const auto global_layout_map = image_state->layout_map.get();
        ASSERT_AND_CONTINUE(global_layout_map);
        auto global_layout_map_guard = image_state->LayoutMapReadLock();

        for (const std::pair<VkImageSubresourceRange, vvl::LocationCapture>& entry : subresources) {
            const VkImageSubresourceRange& subresource = entry.first;
            const Location& barrier_loc = entry.second.Get();
            subresource_adapter::RangeGenerator range_gen(image_state->subresource_encoder, subresource);
            ForEachMatchingLayoutMapRange(
                *global_layout_map, std::move(range_gen),
                [this, &barrier_loc, &image_state](const ImageLayoutMap::key_type& range, const VkImageLayout& layout) {
                    if (layout != VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ && layout != VK_IMAGE_LAYOUT_GENERAL) {
                        const auto& vuid =
                            GetDynamicRenderingBarrierVUID(barrier_loc, vvl::DynamicRenderingBarrierError::kImageLayout);
                        const LogObjectList objlist(base.Handle(), image_state->Handle());
                        const Location& image_loc = barrier_loc.dot(vvl::Field::image);
                        const VkImageSubresource subresource =
                            static_cast<VkImageSubresource>(image_state->subresource_encoder.Decode(range.begin));
                        return validator.LogError(vuid, objlist, image_loc, "(%s, %s) has layout %s.",
                                                  validator.FormatHandle(image_state->Handle()).c_str(),
                                                  string_VkImageSubresource(subresource).c_str(), string_VkImageLayout(layout));
                    }
                    return false;
                });
        }
    }
}

QueueSubState::QueueSubState(CoreChecks& core_checks, vvl::Queue& q)
    : vvl::QueueSubState(q), queue_submission_validator_(core_checks) {}

void QueueSubState::PreSubmit(std::vector<vvl::QueueSubmission>& submissions) {
    for (const auto& submission : submissions) {
        for (auto& cb : submission.cb_submissions) {
            auto guard = cb.cb->ReadLock();
            CommandBufferSubState& cb_substate = SubState(*cb.cb);
            cb_substate.SubmitTimeValidate();
        }
    }
}

void QueueSubState::Retire(vvl::QueueSubmission& submission) {
    queue_submission_validator_.Validate(submission);
    queue_submission_validator_.Update(submission);

    auto is_query_updated_after = [this](const QueryObject& query_object) {
        auto guard = base.Lock();
        bool first_queue_submission = true;
        for (const vvl::QueueSubmission& queue_submission : base.Submissions()) {
            // The current submission is still on the deque, so skip it
            if (first_queue_submission) {
                first_queue_submission = false;
                continue;
            }
            for (const vvl::CommandBufferSubmission& cb_submission : queue_submission.cb_submissions) {
                if (query_object.perf_pass != queue_submission.perf_submit_pass) {
                    continue;
                }
                if (cb_submission.cb->UpdatesQuery(query_object)) {
                    return true;
                }
            }
        }
        return false;
    };

    for (vvl::CommandBufferSubmission& cb_submission : submission.cb_submissions) {
        CommandBufferSubState& cb_sub_state = SubState(*cb_submission.cb);
        auto cb_guard = cb_sub_state.base.WriteLock();
        for (vvl::CommandBuffer* secondary_cmd_buffer : cb_submission.cb->linked_command_buffers) {
            CommandBufferSubState& secondary_sub_state = SubState(*secondary_cmd_buffer);
            auto secondary_guard = secondary_sub_state.base.WriteLock();
            secondary_sub_state.Retire(submission.perf_submit_pass, is_query_updated_after);
        }
        cb_sub_state.Retire(submission.perf_submit_pass, is_query_updated_after);
    }
}

}  // namespace core