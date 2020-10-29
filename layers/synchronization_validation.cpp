/* Copyright (c) 2019-2020 The Khronos Group Inc.
 * Copyright (c) 2019-2020 Valve Corporation
 * Copyright (c) 2019-2020 LunarG, Inc.
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
 * Author: John Zulauf <jzulauf@lunarg.com>
 */

#include <limits>
#include <vector>
#include <memory>
#include <bitset>
#include "synchronization_validation.h"

static const char *string_SyncHazardVUID(SyncHazard hazard) {
    switch (hazard) {
        case SyncHazard::NONE:
            return "SYNC-HAZARD-NONE";
            break;
        case SyncHazard::READ_AFTER_WRITE:
            return "SYNC-HAZARD-READ_AFTER_WRITE";
            break;
        case SyncHazard::WRITE_AFTER_READ:
            return "SYNC-HAZARD-WRITE_AFTER_READ";
            break;
        case SyncHazard::WRITE_AFTER_WRITE:
            return "SYNC-HAZARD-WRITE_AFTER_WRITE";
            break;
        case SyncHazard::READ_RACING_WRITE:
            return "SYNC-HAZARD-READ-RACING-WRITE";
            break;
        case SyncHazard::WRITE_RACING_WRITE:
            return "SYNC-HAZARD-WRITE-RACING-WRITE";
            break;
        case SyncHazard::WRITE_RACING_READ:
            return "SYNC-HAZARD-WRITE-RACING-READ";
            break;
        default:
            assert(0);
    }
    return "SYNC-HAZARD-INVALID";
}

static bool IsHazardVsRead(SyncHazard hazard) {
    switch (hazard) {
        case SyncHazard::NONE:
            return false;
            break;
        case SyncHazard::READ_AFTER_WRITE:
            return false;
            break;
        case SyncHazard::WRITE_AFTER_READ:
            return true;
            break;
        case SyncHazard::WRITE_AFTER_WRITE:
            return false;
            break;
        case SyncHazard::READ_RACING_WRITE:
            return false;
            break;
        case SyncHazard::WRITE_RACING_WRITE:
            return false;
            break;
        case SyncHazard::WRITE_RACING_READ:
            return true;
            break;
        default:
            assert(0);
    }
    return false;
}

static const char *string_SyncHazard(SyncHazard hazard) {
    switch (hazard) {
        case SyncHazard::NONE:
            return "NONR";
            break;
        case SyncHazard::READ_AFTER_WRITE:
            return "READ_AFTER_WRITE";
            break;
        case SyncHazard::WRITE_AFTER_READ:
            return "WRITE_AFTER_READ";
            break;
        case SyncHazard::WRITE_AFTER_WRITE:
            return "WRITE_AFTER_WRITE";
            break;
        case SyncHazard::READ_RACING_WRITE:
            return "READ_RACING_WRITE";
            break;
        case SyncHazard::WRITE_RACING_WRITE:
            return "WRITE_RACING_WRITE";
            break;
        case SyncHazard::WRITE_RACING_READ:
            return "WRITE_RACING_READ";
            break;
        default:
            assert(0);
    }
    return "INVALID HAZARD";
}

static const SyncStageAccessInfoType *SyncStageAccessInfoFromMask(SyncStageAccessFlags flags) {
    // Return the info for the first bit found
    const SyncStageAccessInfoType *info = nullptr;
    uint32_t index = 0;
    while (flags) {
        if (flags & 0x1) {
            flags = 0;
            info = &syncStageAccessInfoByStageAccessIndex[index];
        } else {
            flags = flags >> 1;
            index++;
        }
    }
    return info;
}

static std::string string_SyncStageAccessFlags(SyncStageAccessFlags flags, const char *sep = "|") {
    std::string out_str;
    uint32_t index = 0;
    if (0 == flags) {
        out_str = "0";
    }
    while (flags) {
        const auto &info = syncStageAccessInfoByStageAccessIndex[index];
        if (flags & info.stage_access_bit) {
            if (!out_str.empty()) {
                out_str.append(sep);
            }
            out_str.append(info.name);
            flags = flags & ~info.stage_access_bit;
        }
        index++;
        assert(index < syncStageAccessInfoByStageAccessIndex.size());
    }
    if (out_str.length() == 0) {
        out_str.append("Unhandled SyncStageAccess");
    }
    return out_str;
}

static std::string string_UsageTag(const HazardResult &hazard) {
    const auto &tag = hazard.tag;
    assert(hazard.usage_index < static_cast<SyncStageAccessIndex>(syncStageAccessInfoByStageAccessIndex.size()));
    const auto &usage_info = syncStageAccessInfoByStageAccessIndex[hazard.usage_index];
    std::stringstream out;
    const auto *info = SyncStageAccessInfoFromMask(hazard.prior_access);
    const char *stage_access_name = info ? info->name : "INVALID_STAGE_ACCESS";
    out << "(usage: " << usage_info.name << ", prior_usage: " << stage_access_name;
    if (IsHazardVsRead(hazard.hazard)) {
        const auto barriers = hazard.access_state->GetReadBarriers(hazard.prior_access);
        out << ", read_barriers: " << string_VkPipelineStageFlags(barriers);
    } else {
        SyncStageAccessFlags write_barrier = hazard.access_state->GetWriteBarriers();
        out << ", write_barriers: " << string_SyncStageAccessFlags(write_barrier);
    }

    out << ", command: " << CommandTypeString(tag.command);
    out << ", seq_no: " << (tag.index & 0xFFFFFFFF) << ", reset_no: " << (tag.index >> 32) << ")";
    return out.str();
}

// NOTE: the attachement read flag is put *only* in the access scope and not in the exect scope, since the ordering
//       rules apply only to this specific access for this stage, and not the stage as a whole. The ordering detection
//       also reflects this special case for read hazard detection (using access instead of exec scope)
static constexpr VkPipelineStageFlags kColorAttachmentExecScope = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
static constexpr SyncStageAccessFlags kColorAttachmentAccessScope =
    SyncStageAccessFlagBits::SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ_BIT |
    SyncStageAccessFlagBits::SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT |
    SyncStageAccessFlagBits::SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE_BIT |
    SyncStageAccessFlagBits::SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ_BIT;  // Note: this is intentionally not in the exec scope
static constexpr VkPipelineStageFlags kDepthStencilAttachmentExecScope =
    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
static constexpr SyncStageAccessFlags kDepthStencilAttachmentAccessScope =
    SyncStageAccessFlagBits::SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
    SyncStageAccessFlagBits::SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
    SyncStageAccessFlagBits::SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
    SyncStageAccessFlagBits::SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
    SyncStageAccessFlagBits::SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ_BIT;  // Note: this is intentionally not in the exec scope

static constexpr SyncOrderingBarrier kColorAttachmentRasterOrder = {kColorAttachmentExecScope, kColorAttachmentAccessScope};
static constexpr SyncOrderingBarrier kDepthStencilAttachmentRasterOrder = {kDepthStencilAttachmentExecScope,
                                                                           kDepthStencilAttachmentAccessScope};
static constexpr SyncOrderingBarrier kAttachmentRasterOrder = {kDepthStencilAttachmentExecScope | kColorAttachmentExecScope,
                                                               kDepthStencilAttachmentAccessScope | kColorAttachmentAccessScope};
// Sometimes we have an internal access conflict, and we using the kCurrentCommandTag to set and detect in temporary/proxy contexts
static const ResourceUsageTag kCurrentCommandTag(ResourceUsageTag::kMaxIndex, CMD_NONE);

static VkDeviceSize ResourceBaseAddress(const BINDABLE &bindable) {
    return bindable.binding.offset + bindable.binding.mem_state->fake_base_address;
}

static bool SimpleBinding(const BINDABLE &bindable) { return !bindable.sparse && bindable.binding.mem_state; }

inline VkDeviceSize GetRealWholeSize(VkDeviceSize offset, VkDeviceSize size, VkDeviceSize whole_size) {
    if (size == VK_WHOLE_SIZE) {
        return (whole_size - offset);
    }
    return size;
}

static inline VkDeviceSize GetBufferWholeSize(const BUFFER_STATE &buf_state, VkDeviceSize offset, VkDeviceSize size) {
    return GetRealWholeSize(offset, size, buf_state.createInfo.size);
}

template <typename T>
static ResourceAccessRange MakeRange(const T &has_offset_and_size) {
    return ResourceAccessRange(has_offset_and_size.offset, (has_offset_and_size.offset + has_offset_and_size.size));
}

static ResourceAccessRange MakeRange(VkDeviceSize start, VkDeviceSize size) { return ResourceAccessRange(start, (start + size)); }

static inline ResourceAccessRange MakeRange(const BUFFER_STATE &buffer, VkDeviceSize offset, VkDeviceSize size) {
    return MakeRange(offset, GetBufferWholeSize(buffer, offset, size));
}

static inline ResourceAccessRange MakeRange(const BUFFER_VIEW_STATE &buf_view_state) {
    return MakeRange(*buf_view_state.buffer_state.get(), buf_view_state.create_info.offset, buf_view_state.create_info.range);
}

// Expand the pipeline stage without regard to whether the are valid w.r.t. queue or extension
VkPipelineStageFlags ExpandPipelineStages(VkQueueFlags queue_flags, VkPipelineStageFlags stage_mask) {
    VkPipelineStageFlags expanded = stage_mask;
    if (VK_PIPELINE_STAGE_ALL_COMMANDS_BIT & stage_mask) {
        expanded = expanded & ~VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        for (const auto &all_commands : syncAllCommandStagesByQueueFlags) {
            if (all_commands.first & queue_flags) {
                expanded |= all_commands.second;
            }
        }
    }
    if (VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT & stage_mask) {
        expanded = expanded & ~VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
        expanded |= syncAllCommandStagesByQueueFlags.at(VK_QUEUE_GRAPHICS_BIT) & ~VK_PIPELINE_STAGE_HOST_BIT;
    }
    return expanded;
}

VkPipelineStageFlags RelatedPipelineStages(VkPipelineStageFlags stage_mask,
                                           std::map<VkPipelineStageFlagBits, VkPipelineStageFlags> &map) {
    VkPipelineStageFlags unscanned = stage_mask;
    VkPipelineStageFlags related = 0;
    for (const auto &entry : map) {
        const auto &stage = entry.first;
        if (stage & unscanned) {
            related = related | entry.second;
            unscanned = unscanned & ~stage;
            if (!unscanned) break;
        }
    }
    return related;
}

VkPipelineStageFlags WithEarlierPipelineStages(VkPipelineStageFlags stage_mask) {
    return stage_mask | RelatedPipelineStages(stage_mask, syncLogicallyEarlierStages);
}

VkPipelineStageFlags WithLaterPipelineStages(VkPipelineStageFlags stage_mask) {
    return stage_mask | RelatedPipelineStages(stage_mask, syncLogicallyLaterStages);
}

static const ResourceAccessRange full_range(std::numeric_limits<VkDeviceSize>::min(), std::numeric_limits<VkDeviceSize>::max());

ResourceAccessRange GetBufferRange(VkDeviceSize offset, VkDeviceSize buf_whole_size, uint32_t first_index, uint32_t count,
                                   VkDeviceSize stride) {
    VkDeviceSize range_start = offset + first_index * stride;
    VkDeviceSize range_size = 0;
    if (count == UINT32_MAX) {
        range_size = buf_whole_size - range_start;
    } else {
        range_size = count * stride;
    }
    return MakeRange(range_start, range_size);
}

SyncStageAccessIndex GetSyncStageAccessIndexsByDescriptorSet(VkDescriptorType descriptor_type, const interface_var &descriptor_data,
                                                             VkShaderStageFlagBits stage_flag) {
    if (descriptor_type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
        assert(stage_flag == VK_SHADER_STAGE_FRAGMENT_BIT);
        return SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ;
    }
    auto stage_access = syncStageAccessMaskByShaderStage.find(stage_flag);
    if (stage_access == syncStageAccessMaskByShaderStage.end()) {
        assert(0);
    }
    if (descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
        return stage_access->second.uniform_read;
    }

    // If the desriptorSet is writable, we don't need to care SHADER_READ. SHADER_WRITE is enough.
    // Because if write hazard happens, read hazard might or might not happen.
    // But if write hazard doesn't happen, read hazard is impossible to happen.
    if (descriptor_data.is_writable) {
        return stage_access->second.shader_write;
    }
    return stage_access->second.shader_read;
}

bool IsImageLayoutDepthWritable(VkImageLayout image_layout) {
    return (image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
            image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
            image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
               ? true
               : false;
}

bool IsImageLayoutStencilWritable(VkImageLayout image_layout) {
    return (image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
            image_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL ||
            image_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL)
               ? true
               : false;
}

// Class AccessContext stores the state of accesses specific to a Command, Subpass, or Queue
const std::array<AccessContext::AddressType, AccessContext::kAddressTypeCount> AccessContext::kAddressTypes = {
    AccessContext::AddressType::kLinearAddress, AccessContext::AddressType::kIdealizedAddress};

template <typename Action>
static void ApplyOverImageRange(const IMAGE_STATE &image_state, const VkImageSubresourceRange &subresource_range_arg,
                                Action &action) {
    // At this point the "apply over range" logic only supports a single memory binding
    if (!SimpleBinding(image_state)) return;
    auto subresource_range = NormalizeSubresourceRange(image_state.createInfo, subresource_range_arg);
    subresource_adapter::ImageRangeGenerator range_gen(*image_state.fragment_encoder.get(), subresource_range, {0, 0, 0},
                                                       image_state.createInfo.extent);
    const auto base_address = ResourceBaseAddress(image_state);
    for (; range_gen->non_empty(); ++range_gen) {
        action((*range_gen + base_address));
    }
}

// Tranverse the attachment resolves for this a specific subpass, and do action() to them.
// Used by both validation and record operations
//
// The signature for Action() reflect the needs of both uses.
template <typename Action>
void ResolveOperation(Action &action, const RENDER_PASS_STATE &rp_state, const VkRect2D &render_area,
                      const std::vector<const IMAGE_VIEW_STATE *> &attachment_views, uint32_t subpass) {
    VkExtent3D extent = CastTo3D(render_area.extent);
    VkOffset3D offset = CastTo3D(render_area.offset);
    const auto &rp_ci = rp_state.createInfo;
    const auto *attachment_ci = rp_ci.pAttachments;
    const auto &subpass_ci = rp_ci.pSubpasses[subpass];

    // Color resolves -- require an inuse color attachment and a matching inuse resolve attachment
    const auto *color_attachments = subpass_ci.pColorAttachments;
    const auto *color_resolve = subpass_ci.pResolveAttachments;
    if (color_resolve && color_attachments) {
        for (uint32_t i = 0; i < subpass_ci.colorAttachmentCount; i++) {
            const auto &color_attach = color_attachments[i].attachment;
            const auto &resolve_attach = subpass_ci.pResolveAttachments[i].attachment;
            if ((color_attach != VK_ATTACHMENT_UNUSED) && (resolve_attach != VK_ATTACHMENT_UNUSED)) {
                action("color", "resolve read", color_attach, resolve_attach, attachment_views[color_attach],
                       SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ, kColorAttachmentRasterOrder, offset, extent, 0);
                action("color", "resolve write", color_attach, resolve_attach, attachment_views[resolve_attach],
                       SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, kColorAttachmentRasterOrder, offset, extent, 0);
            }
        }
    }

    // Depth stencil resolve only if the extension is present
    const auto ds_resolve = lvl_find_in_chain<VkSubpassDescriptionDepthStencilResolve>(subpass_ci.pNext);
    if (ds_resolve && ds_resolve->pDepthStencilResolveAttachment &&
        (ds_resolve->pDepthStencilResolveAttachment->attachment != VK_ATTACHMENT_UNUSED) && subpass_ci.pDepthStencilAttachment &&
        (subpass_ci.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED)) {
        const auto src_at = subpass_ci.pDepthStencilAttachment->attachment;
        const auto src_ci = attachment_ci[src_at];
        // The formats are required to match so we can pick either
        const bool resolve_depth = (ds_resolve->depthResolveMode != VK_RESOLVE_MODE_NONE) && FormatHasDepth(src_ci.format);
        const bool resolve_stencil = (ds_resolve->stencilResolveMode != VK_RESOLVE_MODE_NONE) && FormatHasStencil(src_ci.format);
        const auto dst_at = ds_resolve->pDepthStencilResolveAttachment->attachment;
        VkImageAspectFlags aspect_mask = 0u;

        // Figure out which aspects are actually touched during resolve operations
        const char *aspect_string = nullptr;
        if (resolve_depth && resolve_stencil) {
            // Validate all aspects together
            aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            aspect_string = "depth/stencil";
        } else if (resolve_depth) {
            // Validate depth only
            aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
            aspect_string = "depth";
        } else if (resolve_stencil) {
            // Validate all stencil only
            aspect_mask = VK_IMAGE_ASPECT_STENCIL_BIT;
            aspect_string = "stencil";
        }

        if (aspect_mask) {
            action(aspect_string, "resolve read", src_at, dst_at, attachment_views[src_at],
                   SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ, kDepthStencilAttachmentRasterOrder, offset, extent,
                   aspect_mask);
            action(aspect_string, "resolve write", src_at, dst_at, attachment_views[dst_at],
                   SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, kAttachmentRasterOrder, offset, extent, aspect_mask);
        }
    }
}

// Action for validating resolve operations
class ValidateResolveAction {
  public:
    ValidateResolveAction(VkRenderPass render_pass, uint32_t subpass, const AccessContext &context, const SyncValidator &sync_state,
                          const char *func_name)
        : render_pass_(render_pass),
          subpass_(subpass),
          context_(context),
          sync_state_(sync_state),
          func_name_(func_name),
          skip_(false) {}
    void operator()(const char *aspect_name, const char *attachment_name, uint32_t src_at, uint32_t dst_at,
                    const IMAGE_VIEW_STATE *view, SyncStageAccessIndex current_usage, const SyncOrderingBarrier &ordering,
                    const VkOffset3D &offset, const VkExtent3D &extent, VkImageAspectFlags aspect_mask) {
        HazardResult hazard;
        hazard = context_.DetectHazard(view, current_usage, ordering, offset, extent, aspect_mask);
        if (hazard.hazard) {
            skip_ |= sync_state_.LogError(render_pass_, string_SyncHazardVUID(hazard.hazard),
                                          "%s: Hazard %s in subpass %" PRIu32 "during %s %s, from attachment %" PRIu32
                                          " to resolve attachment %" PRIu32 ". Access info %s.",
                                          func_name_, string_SyncHazard(hazard.hazard), subpass_, aspect_name, attachment_name,
                                          src_at, dst_at, string_UsageTag(hazard).c_str());
        }
    }
    // Providing a mechanism for the constructing caller to get the result of the validation
    bool GetSkip() const { return skip_; }

  private:
    VkRenderPass render_pass_;
    const uint32_t subpass_;
    const AccessContext &context_;
    const SyncValidator &sync_state_;
    const char *func_name_;
    bool skip_;
};

// Update action for resolve operations
class UpdateStateResolveAction {
  public:
    UpdateStateResolveAction(AccessContext &context, const ResourceUsageTag &tag) : context_(context), tag_(tag) {}
    void operator()(const char *aspect_name, const char *attachment_name, uint32_t src_at, uint32_t dst_at,
                    const IMAGE_VIEW_STATE *view, SyncStageAccessIndex current_usage, const SyncOrderingBarrier &ordering,
                    const VkOffset3D &offset, const VkExtent3D &extent, VkImageAspectFlags aspect_mask) {
        // Ignores validation only arguments...
        context_.UpdateAccessState(view, current_usage, offset, extent, aspect_mask, tag_);
    }

  private:
    AccessContext &context_;
    const ResourceUsageTag &tag_;
};

void HazardResult::Set(const ResourceAccessState *access_state_, SyncStageAccessIndex usage_index_, SyncHazard hazard_,
                       SyncStageAccessFlags prior_, const ResourceUsageTag &tag_) {
    access_state = std::unique_ptr<const ResourceAccessState>(new ResourceAccessState(*access_state_));
    usage_index = usage_index_;
    hazard = hazard_;
    prior_access = prior_;
    tag = tag_;
}

AccessContext::AccessContext(uint32_t subpass, VkQueueFlags queue_flags,
                             const std::vector<SubpassDependencyGraphNode> &dependencies,
                             const std::vector<AccessContext> &contexts, const AccessContext *external_context) {
    Reset();
    const auto &subpass_dep = dependencies[subpass];
    prev_.reserve(subpass_dep.prev.size());
    prev_by_subpass_.resize(subpass, nullptr);  // Can't be more prevs than the subpass we're on
    for (const auto &prev_dep : subpass_dep.prev) {
        const auto prev_pass = prev_dep.first->pass;
        const auto &prev_barriers = prev_dep.second;
        assert(prev_dep.second.size());
        prev_.emplace_back(&contexts[prev_pass], queue_flags, prev_barriers);
        prev_by_subpass_[prev_pass] = &prev_.back();
    }

    async_.reserve(subpass_dep.async.size());
    for (const auto async_subpass : subpass_dep.async) {
        // TODO -- review why async is storing non-const
        async_.emplace_back(const_cast<AccessContext *>(&contexts[async_subpass]));
    }
    if (subpass_dep.barrier_from_external.size()) {
        src_external_ = TrackBack(external_context, queue_flags, subpass_dep.barrier_from_external);
    }
    if (subpass_dep.barrier_to_external.size()) {
        dst_external_ = TrackBack(this, queue_flags, subpass_dep.barrier_to_external);
    }
}

template <typename Detector>
HazardResult AccessContext::DetectPreviousHazard(AddressType type, const Detector &detector,
                                                 const ResourceAccessRange &range) const {
    ResourceAccessRangeMap descent_map;
    ResolvePreviousAccess(type, range, &descent_map, nullptr);

    HazardResult hazard;
    for (auto prev = descent_map.begin(); prev != descent_map.end() && !hazard.hazard; ++prev) {
        hazard = detector.Detect(prev);
    }
    return hazard;
}

// A recursive range walker for hazard detection, first for the current context and the (DetectHazardRecur) to walk
// the DAG of the contexts (for example subpasses)
template <typename Detector>
HazardResult AccessContext::DetectHazard(AddressType type, const Detector &detector, const ResourceAccessRange &range,
                                         DetectOptions options) const {
    HazardResult hazard;

    if (static_cast<uint32_t>(options) & DetectOptions::kDetectAsync) {
        // Async checks don't require recursive lookups, as the async lists are exhaustive for the top-level context
        // so we'll check these first
        for (const auto &async_context : async_) {
            hazard = async_context->DetectAsyncHazard(type, detector, range);
            if (hazard.hazard) return hazard;
        }
    }

    const bool detect_prev = (static_cast<uint32_t>(options) & DetectOptions::kDetectPrevious) != 0;

    const auto &accesses = GetAccessStateMap(type);
    const auto from = accesses.lower_bound(range);
    const auto to = accesses.upper_bound(range);
    ResourceAccessRange gap = {range.begin, range.begin};

    for (auto pos = from; pos != to; ++pos) {
        // Cover any leading gap, or gap between entries
        if (detect_prev) {
            // TODO: After profiling we may want to change the descent logic such that we don't recur per gap...
            // Cover any leading gap, or gap between entries
            gap.end = pos->first.begin;  // We know this begin is < range.end
            if (gap.non_empty()) {
                // Recur on all gaps
                hazard = DetectPreviousHazard(type, detector, gap);
                if (hazard.hazard) return hazard;
            }
            // Set up for the next gap.  If pos..end is >= range.end, loop will exit, and trailing gap will be empty
            gap.begin = pos->first.end;
        }

        hazard = detector.Detect(pos);
        if (hazard.hazard) return hazard;
    }

    if (detect_prev) {
        // Detect in the trailing empty as needed
        gap.end = range.end;
        if (gap.non_empty()) {
            hazard = DetectPreviousHazard(type, detector, gap);
        }
    }

    return hazard;
}

// A non recursive range walker for the asynchronous contexts (those we have no barriers with)
template <typename Detector>
HazardResult AccessContext::DetectAsyncHazard(AddressType type, const Detector &detector, const ResourceAccessRange &range) const {
    auto &accesses = GetAccessStateMap(type);
    const auto from = accesses.lower_bound(range);
    const auto to = accesses.upper_bound(range);

    HazardResult hazard;
    for (auto pos = from; pos != to && !hazard.hazard; ++pos) {
        hazard = detector.DetectAsync(pos);
    }

    return hazard;
}

struct ApplySubpassTransitionBarriersAction {
    ApplySubpassTransitionBarriersAction(const std::vector<SyncBarrier> &barriers_) : barriers(barriers_) {}
    void operator()(ResourceAccessState *access) const {
        assert(access);
        access->ApplyBarriers(barriers, true);
    }
    const std::vector<SyncBarrier> &barriers;
};

struct ApplyTrackbackBarriersAction {
    ApplyTrackbackBarriersAction(const std::vector<SyncBarrier> &barriers_) : barriers(barriers_) {}
    void operator()(ResourceAccessState *access) const {
        assert(access);
        assert(!access->HasPendingState());
        access->ApplyBarriers(barriers, false);
        access->ApplyPendingBarriers(kCurrentCommandTag);
    }
    const std::vector<SyncBarrier> &barriers;
};

// Splits a single map entry into piece matching the entries in [first, last) the total range over [first, last) must be
// contained with entry.  Entry must be an iterator pointing to dest, first and last must be iterators pointing to a
// *different* map from dest.
// Returns the position past the last resolved range -- the entry covering the remainder of entry->first not included in the
// range [first, last)
template <typename BarrierAction>
static void ResolveMapToEntry(ResourceAccessRangeMap *dest, ResourceAccessRangeMap::iterator entry,
                              ResourceAccessRangeMap::const_iterator first, ResourceAccessRangeMap::const_iterator last,
                              BarrierAction &barrier_action) {
    auto at = entry;
    for (auto pos = first; pos != last; ++pos) {
        // Every member of the input iterator range must fit within the remaining portion of entry
        assert(at->first.includes(pos->first));
        assert(at != dest->end());
        // Trim up at to the same size as the entry to resolve
        at = sparse_container::split(at, *dest, pos->first);
        auto access = pos->second;  // intentional copy
        barrier_action(&access);
        at->second.Resolve(access);
        ++at;  // Go to the remaining unused section of entry
    }
}

static SyncBarrier MergeBarriers(const std::vector<SyncBarrier> &barriers) {
    SyncBarrier merged = {};
    for (const auto &barrier : barriers) {
        merged.Merge(barrier);
    }
    return merged;
}

template <typename BarrierAction>
void AccessContext::ResolveAccessRange(AddressType type, const ResourceAccessRange &range, BarrierAction &barrier_action,
                                       ResourceAccessRangeMap *resolve_map, const ResourceAccessState *infill_state,
                                       bool recur_to_infill) const {
    if (!range.non_empty()) return;

    ResourceRangeMergeIterator current(*resolve_map, GetAccessStateMap(type), range.begin);
    while (current->range.non_empty() && range.includes(current->range.begin)) {
        const auto current_range = current->range & range;
        if (current->pos_B->valid) {
            const auto &src_pos = current->pos_B->lower_bound;
            auto access = src_pos->second;  // intentional copy
            barrier_action(&access);

            if (current->pos_A->valid) {
                const auto trimmed = sparse_container::split(current->pos_A->lower_bound, *resolve_map, current_range);
                trimmed->second.Resolve(access);
                current.invalidate_A(trimmed);
            } else {
                auto inserted = resolve_map->insert(current->pos_A->lower_bound, std::make_pair(current_range, access));
                current.invalidate_A(inserted);  // Update the parallel iterator to point at the insert segment
            }
        } else {
            // we have to descend to fill this gap
            if (recur_to_infill) {
                if (current->pos_A->valid) {
                    // Dest is valid, so we need to accumulate along the DAG and then resolve... in an N-to-1 resolve operation
                    ResourceAccessRangeMap gap_map;
                    ResolvePreviousAccess(type, current_range, &gap_map, infill_state);
                    ResolveMapToEntry(resolve_map, current->pos_A->lower_bound, gap_map.begin(), gap_map.end(), barrier_action);
                } else {
                    // There isn't anything in dest in current)range, so we can accumulate directly into it.
                    ResolvePreviousAccess(type, current_range, resolve_map, infill_state);
                    // Need to apply the barrier to the accesses we accumulated, noting that we haven't updated current
                    for (auto pos = resolve_map->lower_bound(current_range); pos != current->pos_A->lower_bound; ++pos) {
                        barrier_action(&pos->second);
                    }
                }
                // Given that there could be gaps we need to seek carefully to not repeatedly search the same gaps in the next
                // iterator of the outer while.

                // Set the parallel iterator to the end of this range s.t. ++ will move us to the next range whether or
                // not the end of the range is a gap.  For the seek to work, first we need to warn the parallel iterator
                // we stepped on the dest map
                const auto seek_to = current_range.end - 1;  // The subtraction is safe as range can't be empty (loop condition)
                current.invalidate_A();                      // Changes current->range
                current.seek(seek_to);
            } else if (!current->pos_A->valid && infill_state) {
                // If we didn't find anything in the current range, and we aren't reccuring... we infill if required
                auto inserted = resolve_map->insert(current->pos_A->lower_bound, std::make_pair(current->range, *infill_state));
                current.invalidate_A(inserted);  // Update the parallel iterator to point at the correct segment after insert
            }
        }
        ++current;
    }

    // Infill if range goes passed both the current and resolve map prior contents
    if (recur_to_infill && (current->range.end < range.end)) {
        ResourceAccessRange trailing_fill_range = {current->range.end, range.end};
        ResourceAccessRangeMap gap_map;
        const auto the_end = resolve_map->end();
        ResolvePreviousAccess(type, trailing_fill_range, &gap_map, infill_state);
        for (auto &access : gap_map) {
            barrier_action(&access.second);
            resolve_map->insert(the_end, access);
        }
    }
}

void AccessContext::ResolvePreviousAccess(AddressType type, const ResourceAccessRange &range, ResourceAccessRangeMap *descent_map,
                                          const ResourceAccessState *infill_state) const {
    if ((prev_.size() == 0) && (src_external_.context == nullptr)) {
        if (range.non_empty() && infill_state) {
            descent_map->insert(std::make_pair(range, *infill_state));
        }
    } else {
        // Look for something to fill the gap further along.
        for (const auto &prev_dep : prev_) {
            const ApplyTrackbackBarriersAction barrier_action(prev_dep.barriers);
            prev_dep.context->ResolveAccessRange(type, range, barrier_action, descent_map, infill_state);
        }

        if (src_external_.context) {
            const ApplyTrackbackBarriersAction barrier_action(src_external_.barriers);
            src_external_.context->ResolveAccessRange(type, range, barrier_action, descent_map, infill_state);
        }
    }
}

AccessContext::AddressType AccessContext::ImageAddressType(const IMAGE_STATE &image) {
    return (image.fragment_encoder->IsLinearImage()) ? AddressType::kLinearAddress : AddressType::kIdealizedAddress;
}


static SyncStageAccessIndex ColorLoadUsage(VkAttachmentLoadOp load_op) {
    const auto stage_access = (load_op == VK_ATTACHMENT_LOAD_OP_LOAD) ? SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ
                                                                      : SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE;
    return stage_access;
}
static SyncStageAccessIndex DepthStencilLoadUsage(VkAttachmentLoadOp load_op) {
    const auto stage_access = (load_op == VK_ATTACHMENT_LOAD_OP_LOAD) ? SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_READ
                                                                      : SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE;
    return stage_access;
}

// Caller must manage returned pointer
static AccessContext *CreateStoreResolveProxyContext(const AccessContext &context, const RENDER_PASS_STATE &rp_state,
                                                     uint32_t subpass, const VkRect2D &render_area,
                                                     std::vector<const IMAGE_VIEW_STATE *> attachment_views) {
    auto *proxy = new AccessContext(context);
    proxy->UpdateAttachmentResolveAccess(rp_state, render_area, attachment_views, subpass, kCurrentCommandTag);
    proxy->UpdateAttachmentStoreAccess(rp_state, render_area, attachment_views, subpass, kCurrentCommandTag);
    return proxy;
}

template <typename BarrierAction>
class ResolveAccessRangeFunctor {
  public:
    ResolveAccessRangeFunctor(const AccessContext &context, AccessContext::AddressType address_type,
                              ResourceAccessRangeMap *descent_map, const ResourceAccessState *infill_state,
                              BarrierAction &barrier_action)
        : context_(context),
          address_type_(address_type),
          descent_map_(descent_map),
          infill_state_(infill_state),
          barrier_action_(barrier_action) {}
    ResolveAccessRangeFunctor() = delete;
    void operator()(const ResourceAccessRange &range) const {
        context_.ResolveAccessRange(address_type_, range, barrier_action_, descent_map_, infill_state_);
    }

  private:
    const AccessContext &context_;
    const AccessContext::AddressType address_type_;
    ResourceAccessRangeMap *const descent_map_;
    const ResourceAccessState *infill_state_;
    BarrierAction &barrier_action_;
};

template <typename BarrierAction>
void AccessContext::ResolveAccessRange(const IMAGE_STATE &image_state, const VkImageSubresourceRange &subresource_range,
                                       BarrierAction &barrier_action, AddressType address_type, ResourceAccessRangeMap *descent_map,
                                       const ResourceAccessState *infill_state) const {
    const ResolveAccessRangeFunctor<BarrierAction> action(*this, address_type, descent_map, infill_state, barrier_action);
    ApplyOverImageRange(image_state, subresource_range, action);
}

// Layout transitions are handled as if the were occuring in the beginning of the next subpass
bool AccessContext::ValidateLayoutTransitions(const SyncValidator &sync_state, const RENDER_PASS_STATE &rp_state,
                                              const VkRect2D &render_area, uint32_t subpass,
                                              const std::vector<const IMAGE_VIEW_STATE *> &attachment_views,
                                              const char *func_name) const {
    bool skip = false;
    // As validation methods are const and precede the record/update phase, for any tranistions from the immediately
    // previous subpass, we have to validate them against a copy of the AccessContext, with resolve operations applied, as
    // those affects have not been recorded yet.
    //
    // Note: we could be more efficient by tracking whether or not we actually *have* any changes (e.g. attachment resolve)
    // to apply and only copy then, if this proves a hot spot.
    std::unique_ptr<AccessContext> proxy_for_prev;
    TrackBack proxy_track_back;

    const auto &transitions = rp_state.subpass_transitions[subpass];
    for (const auto &transition : transitions) {
        const bool prev_needs_proxy = transition.prev_pass != VK_SUBPASS_EXTERNAL && (transition.prev_pass + 1 == subpass);

        const auto *track_back = GetTrackBackFromSubpass(transition.prev_pass);
        if (prev_needs_proxy) {
            if (!proxy_for_prev) {
                proxy_for_prev.reset(CreateStoreResolveProxyContext(*track_back->context, rp_state, transition.prev_pass,
                                                                    render_area, attachment_views));
                proxy_track_back = *track_back;
                proxy_track_back.context = proxy_for_prev.get();
            }
            track_back = &proxy_track_back;
        }
        auto hazard = DetectSubpassTransitionHazard(*track_back, attachment_views[transition.attachment]);
        if (hazard.hazard) {
            skip |= sync_state.LogError(rp_state.renderPass, string_SyncHazardVUID(hazard.hazard),
                                        "%s: Hazard %s in subpass %" PRIu32 " for attachment %" PRIu32
                                        " image layout transition (old_layout: %s, new_layout: %s). Access info %s.",
                                        func_name, string_SyncHazard(hazard.hazard), subpass, transition.attachment,
                                        string_VkImageLayout(transition.old_layout), string_VkImageLayout(transition.new_layout),
                                        string_UsageTag(hazard).c_str());
        }
    }
    return skip;
}

bool AccessContext::ValidateLoadOperation(const SyncValidator &sync_state, const RENDER_PASS_STATE &rp_state,
                                          const VkRect2D &render_area, uint32_t subpass,
                                          const std::vector<const IMAGE_VIEW_STATE *> &attachment_views,
                                          const char *func_name) const {
    bool skip = false;
    const auto *attachment_ci = rp_state.createInfo.pAttachments;
    VkExtent3D extent = CastTo3D(render_area.extent);
    VkOffset3D offset = CastTo3D(render_area.offset);

    for (uint32_t i = 0; i < rp_state.createInfo.attachmentCount; i++) {
        if (subpass == rp_state.attachment_first_subpass[i]) {
            if (attachment_views[i] == nullptr) continue;
            const IMAGE_VIEW_STATE &view = *attachment_views[i];
            const IMAGE_STATE *image = view.image_state.get();
            if (image == nullptr) continue;
            const auto &ci = attachment_ci[i];

            // Need check in the following way
            // 1) if the usage bit isn't in the dest_access_scope, and there is layout traniition for initial use, report hazard
            //    vs. transition
            // 2) if there isn't a layout transition, we need to look at the  external context with a "detect hazard" operation
            //    for each aspect loaded.

            const bool has_depth = FormatHasDepth(ci.format);
            const bool has_stencil = FormatHasStencil(ci.format);
            const bool is_color = !(has_depth || has_stencil);

            const SyncStageAccessIndex load_index = has_depth ? DepthStencilLoadUsage(ci.loadOp) : ColorLoadUsage(ci.loadOp);
            const SyncStageAccessIndex stencil_load_index = has_stencil ? DepthStencilLoadUsage(ci.stencilLoadOp) : load_index;

            HazardResult hazard;
            const char *aspect = nullptr;

            auto hazard_range = view.normalized_subresource_range;
            bool checked_stencil = false;
            if (is_color) {
                hazard = DetectHazard(*image, load_index, view.normalized_subresource_range, kColorAttachmentRasterOrder, offset,
                                      extent);
                aspect = "color";
            } else {
                if (has_depth) {
                    hazard_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                    hazard = DetectHazard(*image, load_index, hazard_range, kDepthStencilAttachmentRasterOrder, offset, extent);
                    aspect = "depth";
                }
                if (!hazard.hazard && has_stencil) {
                    hazard_range.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
                    hazard =
                        DetectHazard(*image, stencil_load_index, hazard_range, kDepthStencilAttachmentRasterOrder, offset, extent);
                    aspect = "stencil";
                    checked_stencil = true;
                }
            }

            if (hazard.hazard) {
                auto load_op_string = string_VkAttachmentLoadOp(checked_stencil ? ci.stencilLoadOp : ci.loadOp);
                if (hazard.tag == kCurrentCommandTag) {
                    // Hazard vs. ILT
                    skip |= sync_state.LogError(rp_state.renderPass, string_SyncHazardVUID(hazard.hazard),
                                                "%s: Hazard %s vs. layout transition in subpass %" PRIu32 " for attachment %" PRIu32
                                                " aspect %s during load with loadOp %s.",
                                                func_name, string_SyncHazard(hazard.hazard), subpass, i, aspect, load_op_string);
                } else {
                    skip |= sync_state.LogError(rp_state.renderPass, string_SyncHazardVUID(hazard.hazard),
                                                "%s: Hazard %s in subpass %" PRIu32 " for attachment %" PRIu32
                                                " aspect %s during load with loadOp %s. Access info %s.",
                                                func_name, string_SyncHazard(hazard.hazard), subpass, i, aspect, load_op_string,
                                                string_UsageTag(hazard).c_str());
                }
            }
        }
    }
    return skip;
}

// Store operation validation can ignore resolve (before it) and layout tranistions after it.  The first is ignored
// because of the ordering guarantees w.r.t. sample access and that the resolve validation hasn't altered the state, because
// store is part of the same Next/End operation.
// The latter is handled in layout transistion validation directly
bool AccessContext::ValidateStoreOperation(const SyncValidator &sync_state, const RENDER_PASS_STATE &rp_state,
                                           const VkRect2D &render_area, uint32_t subpass,
                                           const std::vector<const IMAGE_VIEW_STATE *> &attachment_views,
                                           const char *func_name) const {
    bool skip = false;
    const auto *attachment_ci = rp_state.createInfo.pAttachments;
    VkExtent3D extent = CastTo3D(render_area.extent);
    VkOffset3D offset = CastTo3D(render_area.offset);

    for (uint32_t i = 0; i < rp_state.createInfo.attachmentCount; i++) {
        if (subpass == rp_state.attachment_last_subpass[i]) {
            if (attachment_views[i] == nullptr) continue;
            const IMAGE_VIEW_STATE &view = *attachment_views[i];
            const IMAGE_STATE *image = view.image_state.get();
            if (image == nullptr) continue;
            const auto &ci = attachment_ci[i];

            // The spec states that "don't care" is an operation with VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            // so we assume that an implementation is *free* to write in that case, meaning that for correctness
            // sake, we treat DONT_CARE as writing.
            const bool has_depth = FormatHasDepth(ci.format);
            const bool has_stencil = FormatHasStencil(ci.format);
            const bool is_color = !(has_depth || has_stencil);
            const bool store_op_stores = ci.storeOp != VK_ATTACHMENT_STORE_OP_NONE_QCOM;
            if (!has_stencil && !store_op_stores) continue;

            HazardResult hazard;
            const char *aspect = nullptr;
            bool checked_stencil = false;
            if (is_color) {
                hazard = DetectHazard(*image, SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE,
                                      view.normalized_subresource_range, kAttachmentRasterOrder, offset, extent);
                aspect = "color";
            } else {
                const bool stencil_op_stores = ci.stencilStoreOp != VK_ATTACHMENT_STORE_OP_NONE_QCOM;
                auto hazard_range = view.normalized_subresource_range;
                if (has_depth && store_op_stores) {
                    hazard_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                    hazard = DetectHazard(*image, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, hazard_range,
                                          kAttachmentRasterOrder, offset, extent);
                    aspect = "depth";
                }
                if (!hazard.hazard && has_stencil && stencil_op_stores) {
                    hazard_range.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
                    hazard = DetectHazard(*image, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, hazard_range,
                                          kAttachmentRasterOrder, offset, extent);
                    aspect = "stencil";
                    checked_stencil = true;
                }
            }

            if (hazard.hazard) {
                const char *const op_type_string = checked_stencil ? "stencilStoreOp" : "storeOp";
                const char *const store_op_string = string_VkAttachmentStoreOp(checked_stencil ? ci.stencilStoreOp : ci.storeOp);
                skip |= sync_state.LogError(rp_state.renderPass, string_SyncHazardVUID(hazard.hazard),
                                            "%s: Hazard %s in subpass %" PRIu32 " for attachment %" PRIu32
                                            " %s aspect during store with %s %s. Access info %s",
                                            func_name, string_SyncHazard(hazard.hazard), subpass, i, aspect, op_type_string,
                                            store_op_string, string_UsageTag(hazard).c_str());
            }
        }
    }
    return skip;
}

bool AccessContext::ValidateResolveOperations(const SyncValidator &sync_state, const RENDER_PASS_STATE &rp_state,
                                              const VkRect2D &render_area,
                                              const std::vector<const IMAGE_VIEW_STATE *> &attachment_views, const char *func_name,
                                              uint32_t subpass) const {
    ValidateResolveAction validate_action(rp_state.renderPass, subpass, *this, sync_state, func_name);
    ResolveOperation(validate_action, rp_state, render_area, attachment_views, subpass);
    return validate_action.GetSkip();
}

class HazardDetector {
    SyncStageAccessIndex usage_index_;

  public:
    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos) const { return pos->second.DetectHazard(usage_index_); }
    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos) const {
        return pos->second.DetectAsyncHazard(usage_index_);
    }
    HazardDetector(SyncStageAccessIndex usage) : usage_index_(usage) {}
};

class HazardDetectorWithOrdering {
    const SyncStageAccessIndex usage_index_;
    const SyncOrderingBarrier &ordering_;

  public:
    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos) const {
        return pos->second.DetectHazard(usage_index_, ordering_);
    }
    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos) const {
        return pos->second.DetectAsyncHazard(usage_index_);
    }
    HazardDetectorWithOrdering(SyncStageAccessIndex usage, const SyncOrderingBarrier &ordering)
        : usage_index_(usage), ordering_(ordering) {}
};

HazardResult AccessContext::DetectHazard(AddressType type, SyncStageAccessIndex usage_index,
                                         const ResourceAccessRange &range) const {
    HazardDetector detector(usage_index);
    return DetectHazard(type, detector, range, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const BUFFER_STATE &buffer, SyncStageAccessIndex usage_index,
                                         const ResourceAccessRange &range) const {
    if (!SimpleBinding(buffer)) return HazardResult();
    return DetectHazard(AddressType::kLinearAddress, usage_index, range + ResourceBaseAddress(buffer));
}

template <typename Detector>
HazardResult AccessContext::DetectHazard(Detector &detector, const IMAGE_STATE &image,
                                         const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset,
                                         const VkExtent3D &extent, DetectOptions options) const {
    if (!SimpleBinding(image)) return HazardResult();
    subresource_adapter::ImageRangeGenerator range_gen(*image.fragment_encoder.get(), subresource_range, offset, extent);
    const auto address_type = ImageAddressType(image);
    const auto base_address = ResourceBaseAddress(image);
    for (; range_gen->non_empty(); ++range_gen) {
        HazardResult hazard = DetectHazard(address_type, detector, (*range_gen + base_address), options);
        if (hazard.hazard) return hazard;
    }
    return HazardResult();
}

HazardResult AccessContext::DetectHazard(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                                         const VkImageSubresourceLayers &subresource, const VkOffset3D &offset,
                                         const VkExtent3D &extent) const {
    VkImageSubresourceRange subresource_range = {subresource.aspectMask, subresource.mipLevel, 1, subresource.baseArrayLayer,
                                                 subresource.layerCount};
    return DetectHazard(image, current_usage, subresource_range, offset, extent);
}

HazardResult AccessContext::DetectHazard(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                                         const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset,
                                         const VkExtent3D &extent) const {
    HazardDetector detector(current_usage);
    return DetectHazard(detector, image, subresource_range, offset, extent, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                                         const VkImageSubresourceRange &subresource_range, const SyncOrderingBarrier &ordering,
                                         const VkOffset3D &offset, const VkExtent3D &extent) const {
    HazardDetectorWithOrdering detector(current_usage, ordering);
    return DetectHazard(detector, image, subresource_range, offset, extent, DetectOptions::kDetectAll);
}

// Some common code for looking at attachments, if there's anything wrong, we return no hazard, core validation
// should have reported the issue regarding an invalid attachment entry
HazardResult AccessContext::DetectHazard(const IMAGE_VIEW_STATE *view, SyncStageAccessIndex current_usage,
                                         const SyncOrderingBarrier &ordering, const VkOffset3D &offset, const VkExtent3D &extent,
                                         VkImageAspectFlags aspect_mask) const {
    if (view != nullptr) {
        const IMAGE_STATE *image = view->image_state.get();
        if (image != nullptr) {
            auto *detect_range = &view->normalized_subresource_range;
            VkImageSubresourceRange masked_range;
            if (aspect_mask) {  // If present and non-zero, restrict the normalized range to aspects present in aspect_mask
                masked_range = view->normalized_subresource_range;
                masked_range.aspectMask = aspect_mask & masked_range.aspectMask;
                detect_range = &masked_range;
            }

            // NOTE: The range encoding code is not robust to invalid ranges, so we protect it from our change
            if (detect_range->aspectMask) {
                return DetectHazard(*image, current_usage, *detect_range, ordering, offset, extent);
            }
        }
    }
    return HazardResult();
}
class BarrierHazardDetector {
  public:
    BarrierHazardDetector(SyncStageAccessIndex usage_index, VkPipelineStageFlags src_exec_scope,
                          SyncStageAccessFlags src_access_scope)
        : usage_index_(usage_index), src_exec_scope_(src_exec_scope), src_access_scope_(src_access_scope) {}

    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos) const {
        return pos->second.DetectBarrierHazard(usage_index_, src_exec_scope_, src_access_scope_);
    }
    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos) const {
        // Async barrier hazard detection can use the same path as the usage index is not IsRead, but is IsWrite
        return pos->second.DetectAsyncHazard(usage_index_);
    }

  private:
    SyncStageAccessIndex usage_index_;
    VkPipelineStageFlags src_exec_scope_;
    SyncStageAccessFlags src_access_scope_;
};

HazardResult AccessContext::DetectBarrierHazard(AddressType type, SyncStageAccessIndex current_usage,
                                                VkPipelineStageFlags src_exec_scope, SyncStageAccessFlags src_access_scope,
                                                const ResourceAccessRange &range, DetectOptions options) const {
    BarrierHazardDetector detector(current_usage, src_exec_scope, src_access_scope);
    return DetectHazard(type, detector, range, options);
}

HazardResult AccessContext::DetectImageBarrierHazard(const IMAGE_STATE &image, VkPipelineStageFlags src_exec_scope,
                                                     SyncStageAccessFlags src_access_scope,
                                                     const VkImageSubresourceRange &subresource_range,
                                                     DetectOptions options) const {
    BarrierHazardDetector detector(SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION, src_exec_scope, src_access_scope);
    VkOffset3D zero_offset = {0, 0, 0};
    return DetectHazard(detector, image, subresource_range, zero_offset, image.createInfo.extent, options);
}

HazardResult AccessContext::DetectImageBarrierHazard(const IMAGE_STATE &image, VkPipelineStageFlags src_exec_scope,
                                                     SyncStageAccessFlags src_stage_accesses,
                                                     const VkImageMemoryBarrier &barrier) const {
    auto subresource_range = NormalizeSubresourceRange(image.createInfo, barrier.subresourceRange);
    const auto src_access_scope = SyncStageAccess::AccessScope(src_stage_accesses, barrier.srcAccessMask);
    return DetectImageBarrierHazard(image, src_exec_scope, src_access_scope, subresource_range, kDetectAll);
}

template <typename Flags, typename Map>
SyncStageAccessFlags AccessScopeImpl(Flags flag_mask, const Map &map) {
    SyncStageAccessFlags scope = 0;
    for (const auto &bit_scope : map) {
        if (flag_mask < bit_scope.first) break;

        if (flag_mask & bit_scope.first) {
            scope |= bit_scope.second;
        }
    }
    return scope;
}

SyncStageAccessFlags SyncStageAccess::AccessScopeByStage(VkPipelineStageFlags stages) {
    return AccessScopeImpl(stages, syncStageAccessMaskByStageBit);
}

SyncStageAccessFlags SyncStageAccess::AccessScopeByAccess(VkAccessFlags accesses) {
    return AccessScopeImpl(accesses, syncStageAccessMaskByAccessBit);
}

// Getting from stage mask and access mask to stage/acess masks is something we need to be good at...
SyncStageAccessFlags SyncStageAccess::AccessScope(VkPipelineStageFlags stages, VkAccessFlags accesses) {
    // The access scope is the intersection of all stage/access types possible for the enabled stages and the enables
    // accesses (after doing a couple factoring of common terms the union of stage/access intersections is the intersections
    // of the union of all stage/access types for all the stages and the same unions for the access mask...
    return AccessScopeByStage(stages) & AccessScopeByAccess(accesses);
}

template <typename Action>
void UpdateMemoryAccessState(ResourceAccessRangeMap *accesses, const ResourceAccessRange &range, const Action &action) {
    // TODO: Optimization for operations that do a pure overwrite (i.e. WRITE usages which rewrite the state, vs READ usages
    //       that do incrementalupdates
    auto pos = accesses->lower_bound(range);
    if (pos == accesses->end() || !pos->first.intersects(range)) {
        // The range is empty, fill it with a default value.
        pos = action.Infill(accesses, pos, range);
    } else if (range.begin < pos->first.begin) {
        // Leading empty space, infill
        pos = action.Infill(accesses, pos, ResourceAccessRange(range.begin, pos->first.begin));
    } else if (pos->first.begin < range.begin) {
        // Trim the beginning if needed
        pos = accesses->split(pos, range.begin, sparse_container::split_op_keep_both());
        ++pos;
    }

    const auto the_end = accesses->end();
    while ((pos != the_end) && pos->first.intersects(range)) {
        if (pos->first.end > range.end) {
            pos = accesses->split(pos, range.end, sparse_container::split_op_keep_both());
        }

        pos = action(accesses, pos);
        if (pos == the_end) break;

        auto next = pos;
        ++next;
        if ((pos->first.end < range.end) && (next != the_end) && !next->first.is_subsequent_to(pos->first)) {
            // Need to infill if next is disjoint
            VkDeviceSize limit = (next == the_end) ? range.end : std::min(range.end, next->first.begin);
            ResourceAccessRange new_range(pos->first.end, limit);
            next = action.Infill(accesses, next, new_range);
        }
        pos = next;
    }
}

struct UpdateMemoryAccessStateFunctor {
    using Iterator = ResourceAccessRangeMap::iterator;
    Iterator Infill(ResourceAccessRangeMap *accesses, Iterator pos, ResourceAccessRange range) const {
        // this is only called on gaps, and never returns a gap.
        ResourceAccessState default_state;
        context.ResolvePreviousAccess(type, range, accesses, &default_state);
        return accesses->lower_bound(range);
    }

    Iterator operator()(ResourceAccessRangeMap *accesses, Iterator pos) const {
        auto &access_state = pos->second;
        access_state.Update(usage, tag);
        return pos;
    }

    UpdateMemoryAccessStateFunctor(AccessContext::AddressType type_, const AccessContext &context_, SyncStageAccessIndex usage_,
                                   const ResourceUsageTag &tag_)
        : type(type_), context(context_), usage(usage_), tag(tag_) {}
    const AccessContext::AddressType type;
    const AccessContext &context;
    const SyncStageAccessIndex usage;
    const ResourceUsageTag &tag;
};

// This functor applies a single barrier, updating the "pending state" in each touched memory range, but does not
// resolve the pendinging state. Suitable for processing Image and Buffer barriers from PipelineBarriers or Events
class ApplyBarrierFunctor {
  public:
    using Iterator = ResourceAccessRangeMap::iterator;
    inline Iterator Infill(ResourceAccessRangeMap *accesses, Iterator pos, ResourceAccessRange range) const { return pos; }

    Iterator operator()(ResourceAccessRangeMap *accesses, Iterator pos) const {
        auto &access_state = pos->second;
        access_state.ApplyBarrier(barrier_, layout_transition_);
        return pos;
    }

    ApplyBarrierFunctor(const SyncBarrier &barrier, bool layout_transition)
        : barrier_(barrier), layout_transition_(layout_transition) {}

  private:
    const SyncBarrier barrier_;
    const bool layout_transition_;
};

// This functor applies a collection of barriers, updating the "pending state" in each touched memory range, and optionally
// resolves the pending state. Suitable for processing Global memory barriers, or Subpass Barriers when the "final" barrier
// of a collection is known/present.
class ApplyBarrierOpsFunctor {
  public:
    using Iterator = ResourceAccessRangeMap::iterator;
    inline Iterator Infill(ResourceAccessRangeMap *accesses, Iterator pos, ResourceAccessRange range) const { return pos; }

    struct BarrierOp {
        SyncBarrier barrier;
        bool layout_transition;
        BarrierOp(const SyncBarrier &barrier_, bool layout_transition_)
            : barrier(barrier_), layout_transition(layout_transition_) {}
        BarrierOp() = default;
    };
    Iterator operator()(ResourceAccessRangeMap *accesses, Iterator pos) const {
        auto &access_state = pos->second;
        for (const auto op : barrier_ops_) {
            access_state.ApplyBarrier(op.barrier, op.layout_transition);
        }

        if (resolve_) {
            // If this is the last (or only) batch, we can do the pending resolve as the last step in this operation to avoid
            // another walk
            access_state.ApplyPendingBarriers(tag_);
        }
        return pos;
    }

    // A valid tag is required IFF any of the barriers ops are a layout transition, as transitions are write ops
    ApplyBarrierOpsFunctor(bool resolve, size_t size_hint, const ResourceUsageTag &tag)
        : resolve_(resolve), barrier_ops_(), tag_(tag) {
        if (size_hint) {
            barrier_ops_.reserve(size_hint);
        }
    };

    // A valid tag is required IFF layout_transition is true, as transitions are write ops
    ApplyBarrierOpsFunctor(bool resolve, const std::vector<SyncBarrier> &barriers, bool layout_transition,
                           const ResourceUsageTag &tag)
        : resolve_(resolve), barrier_ops_(), tag_(tag) {
        barrier_ops_.reserve(barriers.size());
        for (const auto &barrier : barriers) {
            barrier_ops_.emplace_back(barrier, layout_transition);
        }
    }

    void PushBack(const SyncBarrier &barrier, bool layout_transition) { barrier_ops_.emplace_back(barrier, layout_transition); }

    void PushBack(const std::vector<SyncBarrier> &barriers, bool layout_transition) {
        barrier_ops_.reserve(barrier_ops_.size() + barriers.size());
        for (const auto &barrier : barriers) {
            barrier_ops_.emplace_back(barrier, layout_transition);
        }
    }

  private:
    bool resolve_;
    std::vector<BarrierOp> barrier_ops_;
    const ResourceUsageTag &tag_;
};

void AccessContext::UpdateAccessState(AddressType type, SyncStageAccessIndex current_usage, const ResourceAccessRange &range,
                                      const ResourceUsageTag &tag) {
    UpdateMemoryAccessStateFunctor action(type, *this, current_usage, tag);
    UpdateMemoryAccessState(&GetAccessStateMap(type), range, action);
}

void AccessContext::UpdateAccessState(const BUFFER_STATE &buffer, SyncStageAccessIndex current_usage,
                                      const ResourceAccessRange &range, const ResourceUsageTag &tag) {
    if (!SimpleBinding(buffer)) return;
    const auto base_address = ResourceBaseAddress(buffer);
    UpdateAccessState(AddressType::kLinearAddress, current_usage, range + base_address, tag);
}

void AccessContext::UpdateAccessState(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                                      const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset,
                                      const VkExtent3D &extent, const ResourceUsageTag &tag) {
    if (!SimpleBinding(image)) return;
    subresource_adapter::ImageRangeGenerator range_gen(*image.fragment_encoder.get(), subresource_range, offset, extent);
    const auto address_type = ImageAddressType(image);
    const auto base_address = ResourceBaseAddress(image);
    UpdateMemoryAccessStateFunctor action(address_type, *this, current_usage, tag);
    for (; range_gen->non_empty(); ++range_gen) {
        UpdateMemoryAccessState(&GetAccessStateMap(address_type), (*range_gen + base_address), action);
    }
}
void AccessContext::UpdateAccessState(const IMAGE_VIEW_STATE *view, SyncStageAccessIndex current_usage, const VkOffset3D &offset,
                                      const VkExtent3D &extent, VkImageAspectFlags aspect_mask, const ResourceUsageTag &tag) {
    if (view != nullptr) {
        const IMAGE_STATE *image = view->image_state.get();
        if (image != nullptr) {
            auto *update_range = &view->normalized_subresource_range;
            VkImageSubresourceRange masked_range;
            if (aspect_mask) {  // If present and non-zero, restrict the normalized range to aspects present in aspect_mask
                masked_range = view->normalized_subresource_range;
                masked_range.aspectMask = aspect_mask & masked_range.aspectMask;
                update_range = &masked_range;
            }
            UpdateAccessState(*image, current_usage, *update_range, offset, extent, tag);
        }
    }
}

void AccessContext::UpdateAccessState(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                                      const VkImageSubresourceLayers &subresource, const VkOffset3D &offset,
                                      const VkExtent3D &extent, const ResourceUsageTag &tag) {
    VkImageSubresourceRange subresource_range = {subresource.aspectMask, subresource.mipLevel, 1, subresource.baseArrayLayer,
                                                 subresource.layerCount};
    UpdateAccessState(image, current_usage, subresource_range, offset, extent, tag);
}

template <typename Action>
void AccessContext::UpdateResourceAccess(const BUFFER_STATE &buffer, const ResourceAccessRange &range, const Action action) {
    if (!SimpleBinding(buffer)) return;
    const auto base_address = ResourceBaseAddress(buffer);
    UpdateMemoryAccessState(&GetAccessStateMap(AddressType::kLinearAddress), (range + base_address), action);
}

template <typename Action>
void AccessContext::UpdateResourceAccess(const IMAGE_STATE &image, const VkImageSubresourceRange &subresource_range,
                                         const Action action) {
    if (!SimpleBinding(image)) return;
    const auto address_type = ImageAddressType(image);
    auto *accesses = &GetAccessStateMap(address_type);

    subresource_adapter::ImageRangeGenerator range_gen(*image.fragment_encoder.get(), subresource_range, {0, 0, 0},
                                                       image.createInfo.extent);

    const auto base_address = ResourceBaseAddress(image);
    for (; range_gen->non_empty(); ++range_gen) {
        UpdateMemoryAccessState(accesses, (*range_gen + base_address), action);
    }
}

void AccessContext::UpdateAttachmentResolveAccess(const RENDER_PASS_STATE &rp_state, const VkRect2D &render_area,
                                                  const std::vector<const IMAGE_VIEW_STATE *> &attachment_views, uint32_t subpass,
                                                  const ResourceUsageTag &tag) {
    UpdateStateResolveAction update(*this, tag);
    ResolveOperation(update, rp_state, render_area, attachment_views, subpass);
}

void AccessContext::UpdateAttachmentStoreAccess(const RENDER_PASS_STATE &rp_state, const VkRect2D &render_area,
                                                const std::vector<const IMAGE_VIEW_STATE *> &attachment_views, uint32_t subpass,
                                                const ResourceUsageTag &tag) {
    const auto *attachment_ci = rp_state.createInfo.pAttachments;
    VkExtent3D extent = CastTo3D(render_area.extent);
    VkOffset3D offset = CastTo3D(render_area.offset);

    for (uint32_t i = 0; i < rp_state.createInfo.attachmentCount; i++) {
        if (rp_state.attachment_last_subpass[i] == subpass) {
            if (attachment_views[i] == nullptr) continue;  // UNUSED
            const auto &view = *attachment_views[i];
            const IMAGE_STATE *image = view.image_state.get();
            if (image == nullptr) continue;

            const auto &ci = attachment_ci[i];
            const bool has_depth = FormatHasDepth(ci.format);
            const bool has_stencil = FormatHasStencil(ci.format);
            const bool is_color = !(has_depth || has_stencil);
            const bool store_op_stores = ci.storeOp != VK_ATTACHMENT_STORE_OP_NONE_QCOM;

            if (is_color && store_op_stores) {
                UpdateAccessState(*image, SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, view.normalized_subresource_range,
                                  offset, extent, tag);
            } else {
                auto update_range = view.normalized_subresource_range;
                if (has_depth && store_op_stores) {
                    update_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                    UpdateAccessState(*image, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, update_range, offset, extent,
                                      tag);
                }
                const bool stencil_op_stores = ci.stencilStoreOp != VK_ATTACHMENT_STORE_OP_NONE_QCOM;
                if (has_stencil && stencil_op_stores) {
                    update_range.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
                    UpdateAccessState(*image, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, update_range, offset, extent,
                                      tag);
                }
            }
        }
    }
}

template <typename Action>
void AccessContext::ApplyGlobalBarriers(const Action &barrier_action) {
    // Note: Barriers do *not* cross context boundaries, applying to accessess within.... (at least for renderpass subpasses)
    for (const auto address_type : kAddressTypes) {
        UpdateMemoryAccessState(&GetAccessStateMap(address_type), full_range, barrier_action);
    }
}

void AccessContext::ResolveChildContexts(const std::vector<AccessContext> &contexts) {
    for (uint32_t subpass_index = 0; subpass_index < contexts.size(); subpass_index++) {
        auto &context = contexts[subpass_index];
        ApplyTrackbackBarriersAction barrier_action(context.GetDstExternalTrackBack().barriers);
        for (const auto address_type : kAddressTypes) {
            context.ResolveAccessRange(address_type, full_range, barrier_action, &GetAccessStateMap(address_type), nullptr, false);
        }
    }
}

// Suitable only for *subpass* access contexts
HazardResult AccessContext::DetectSubpassTransitionHazard(const TrackBack &track_back, const IMAGE_VIEW_STATE *attach_view) const {
    if (!attach_view) return HazardResult();
    const auto image_state = attach_view->image_state.get();
    if (!image_state) return HazardResult();

    // We should never ask for a transition from a context we don't have
    assert(track_back.context);

    // Do the detection against the specific prior context independent of other contexts.  (Synchronous only)
    // Hazard detection for the transition can be against the merged of the barriers (it only uses src_...)
    const auto merged_barrier = MergeBarriers(track_back.barriers);
    HazardResult hazard =
        track_back.context->DetectImageBarrierHazard(*image_state, merged_barrier.src_exec_scope, merged_barrier.src_access_scope,
                                                     attach_view->normalized_subresource_range, kDetectPrevious);
    if (!hazard.hazard) {
        // The Async hazard check is against the current context's async set.
        hazard = DetectImageBarrierHazard(*image_state, merged_barrier.src_exec_scope, merged_barrier.src_access_scope,
                                          attach_view->normalized_subresource_range, kDetectAsync);
    }

    return hazard;
}

void AccessContext::RecordLayoutTransitions(const RENDER_PASS_STATE &rp_state, uint32_t subpass,
                                            const std::vector<const IMAGE_VIEW_STATE *> &attachment_views,
                                            const ResourceUsageTag &tag) {
    const auto &transitions = rp_state.subpass_transitions[subpass];
    const ResourceAccessState empty_infill;
    for (const auto &transition : transitions) {
        const auto prev_pass = transition.prev_pass;
        const auto attachment_view = attachment_views[transition.attachment];
        if (!attachment_view) continue;
        const auto *image = attachment_view->image_state.get();
        if (!image) continue;
        if (!SimpleBinding(*image)) continue;

        const auto *trackback = GetTrackBackFromSubpass(prev_pass);
        assert(trackback);

        // Import the attachments into the current context
        const auto *prev_context = trackback->context;
        assert(prev_context);
        const auto address_type = ImageAddressType(*image);
        auto &target_map = GetAccessStateMap(address_type);
        ApplySubpassTransitionBarriersAction barrier_action(trackback->barriers);
        prev_context->ResolveAccessRange(*image, attachment_view->normalized_subresource_range, barrier_action, address_type,
                                         &target_map, &empty_infill);
    }

    // If there were no transitions skip this global map walk
    if (transitions.size()) {
        ApplyBarrierOpsFunctor apply_pending_action(true /* resolve */, 0, tag);
        ApplyGlobalBarriers(apply_pending_action);
    }
}

// Class CommandBufferAccessContext: Keep track of resource access state information for a specific command buffer
bool CommandBufferAccessContext::ValidateBeginRenderPass(const RENDER_PASS_STATE &rp_state,

                                                         const VkRenderPassBeginInfo *pRenderPassBegin,
                                                         const VkSubpassBeginInfoKHR *pSubpassBeginInfo,
                                                         const char *func_name) const {
    // Check if any of the layout transitions are hazardous.... but we don't have the renderpass context to work with, so we
    bool skip = false;

    assert(pRenderPassBegin);
    if (nullptr == pRenderPassBegin) return skip;

    const uint32_t subpass = 0;

    // Construct the state we can use to validate against... (since validation is const and RecordCmdBeginRenderPass
    // hasn't happened yet)
    const std::vector<AccessContext> empty_context_vector;
    AccessContext temp_context(subpass, queue_flags_, rp_state.subpass_dependencies, empty_context_vector,
                               const_cast<AccessContext *>(&cb_access_context_));

    // Create a view list
    const auto fb_state = sync_state_->Get<FRAMEBUFFER_STATE>(pRenderPassBegin->framebuffer);
    assert(fb_state);
    if (nullptr == fb_state) return skip;
    // NOTE: Must not use COMMAND_BUFFER_STATE variant of this as RecordCmdBeginRenderPass hasn't run and thus
    //       the activeRenderPass.* fields haven't been set.
    const auto views = sync_state_->GetAttachmentViews(*pRenderPassBegin, *fb_state);

    // Validate transitions
    skip |= temp_context.ValidateLayoutTransitions(*sync_state_, rp_state, pRenderPassBegin->renderArea, subpass, views, func_name);

    // Validate load operations if there were no layout transition hazards
    if (!skip) {
        temp_context.RecordLayoutTransitions(rp_state, subpass, views, kCurrentCommandTag);
        skip |= temp_context.ValidateLoadOperation(*sync_state_, rp_state, pRenderPassBegin->renderArea, subpass, views, func_name);
    }

    return skip;
}

bool CommandBufferAccessContext::ValidateDispatchDrawDescriptorSet(VkPipelineBindPoint pipelineBindPoint,
                                                                   const char *func_name) const {
    bool skip = false;
    const PIPELINE_STATE *pPipe = nullptr;
    const std::vector<LAST_BOUND_STATE::PER_SET> *per_sets = nullptr;
    GetCurrentPipelineAndDesriptorSetsFromCommandBuffer(*cb_state_.get(), pipelineBindPoint, &pPipe, &per_sets);
    if (!pPipe || !per_sets) {
        return skip;
    }

    using DescriptorClass = cvdescriptorset::DescriptorClass;
    using BufferDescriptor = cvdescriptorset::BufferDescriptor;
    using ImageDescriptor = cvdescriptorset::ImageDescriptor;
    using ImageSamplerDescriptor = cvdescriptorset::ImageSamplerDescriptor;
    using TexelDescriptor = cvdescriptorset::TexelDescriptor;

    for (const auto &stage_state : pPipe->stage_state) {
        if (stage_state.stage_flag == VK_SHADER_STAGE_FRAGMENT_BIT && pPipe->graphicsPipelineCI.pRasterizationState &&
            pPipe->graphicsPipelineCI.pRasterizationState->rasterizerDiscardEnable)
            continue;
        for (const auto &set_binding : stage_state.descriptor_uses) {
            cvdescriptorset::DescriptorSet *descriptor_set = (*per_sets)[set_binding.first.first].bound_descriptor_set;
            cvdescriptorset::DescriptorSetLayout::ConstBindingIterator binding_it(descriptor_set->GetLayout().get(),
                                                                                  set_binding.first.second);
            const auto descriptor_type = binding_it.GetType();
            cvdescriptorset::IndexRange index_range = binding_it.GetGlobalIndexRange();
            auto array_idx = 0;

            if (binding_it.IsVariableDescriptorCount()) {
                index_range.end = index_range.start + descriptor_set->GetVariableDescriptorCount();
            }
            SyncStageAccessIndex sync_index =
                GetSyncStageAccessIndexsByDescriptorSet(descriptor_type, set_binding.second, stage_state.stage_flag);

            for (uint32_t i = index_range.start; i < index_range.end; ++i, ++array_idx) {
                uint32_t index = i - index_range.start;
                const auto *descriptor = descriptor_set->GetDescriptorFromGlobalIndex(i);
                switch (descriptor->GetClass()) {
                    case DescriptorClass::ImageSampler:
                    case DescriptorClass::Image: {
                        const IMAGE_VIEW_STATE *img_view_state = nullptr;
                        VkImageLayout image_layout;
                        if (descriptor->GetClass() == DescriptorClass::ImageSampler) {
                            const auto image_sampler_descriptor = static_cast<const ImageSamplerDescriptor *>(descriptor);
                            img_view_state = image_sampler_descriptor->GetImageViewState();
                            image_layout = image_sampler_descriptor->GetImageLayout();
                        } else {
                            const auto image_descriptor = static_cast<const ImageDescriptor *>(descriptor);
                            img_view_state = image_descriptor->GetImageViewState();
                            image_layout = image_descriptor->GetImageLayout();
                        }
                        if (!img_view_state) continue;
                        const IMAGE_STATE *img_state = img_view_state->image_state.get();
                        VkExtent3D extent = {};
                        VkOffset3D offset = {};
                        if (sync_index == SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ) {
                            extent = CastTo3D(cb_state_->activeRenderPassBeginInfo.renderArea.extent);
                            offset = CastTo3D(cb_state_->activeRenderPassBeginInfo.renderArea.offset);
                        } else {
                            extent = img_state->createInfo.extent;
                        }
                        HazardResult hazard;
                        const auto &subresource_range = img_view_state->normalized_subresource_range;
                        if (descriptor_type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
                            // Input attachments are subject to raster ordering rules
                            hazard = current_context_->DetectHazard(*img_state, sync_index, subresource_range,
                                                                    kAttachmentRasterOrder, offset, extent);
                        } else {
                            hazard = current_context_->DetectHazard(*img_state, sync_index, subresource_range, offset, extent);
                        }
                        if (hazard.hazard && !sync_state_->SupressedBoundDescriptorWAW(hazard)) {
                            skip |= sync_state_->LogError(
                                img_view_state->image_view, string_SyncHazardVUID(hazard.hazard),
                                "%s: Hazard %s for %s, in %s, and %s, %s, type: %s, imageLayout: %s, binding #%" PRIu32
                                ", index %" PRIu32 ". Access info %s.",
                                func_name, string_SyncHazard(hazard.hazard),
                                sync_state_->report_data->FormatHandle(img_view_state->image_view).c_str(),
                                sync_state_->report_data->FormatHandle(cb_state_->commandBuffer).c_str(),
                                sync_state_->report_data->FormatHandle(pPipe->pipeline).c_str(),
                                sync_state_->report_data->FormatHandle(descriptor_set->GetSet()).c_str(),
                                string_VkDescriptorType(descriptor_type), string_VkImageLayout(image_layout),
                                set_binding.first.second, index, string_UsageTag(hazard).c_str());
                        }
                        break;
                    }
                    case DescriptorClass::TexelBuffer: {
                        auto buf_view_state = static_cast<const TexelDescriptor *>(descriptor)->GetBufferViewState();
                        if (!buf_view_state) continue;
                        const BUFFER_STATE *buf_state = buf_view_state->buffer_state.get();
                        const ResourceAccessRange range = MakeRange(*buf_view_state);
                        auto hazard = current_context_->DetectHazard(*buf_state, sync_index, range);
                        if (hazard.hazard && !sync_state_->SupressedBoundDescriptorWAW(hazard)) {
                            skip |= sync_state_->LogError(
                                buf_view_state->buffer_view, string_SyncHazardVUID(hazard.hazard),
                                "%s: Hazard %s for %s in %s, %s, and %s, type: %s, binding #%d index %d. Access info %s.",
                                func_name, string_SyncHazard(hazard.hazard),
                                sync_state_->report_data->FormatHandle(buf_view_state->buffer_view).c_str(),
                                sync_state_->report_data->FormatHandle(cb_state_->commandBuffer).c_str(),
                                sync_state_->report_data->FormatHandle(pPipe->pipeline).c_str(),
                                sync_state_->report_data->FormatHandle(descriptor_set->GetSet()).c_str(),
                                string_VkDescriptorType(descriptor_type), set_binding.first.second, index,
                                string_UsageTag(hazard).c_str());
                        }
                        break;
                    }
                    case DescriptorClass::GeneralBuffer: {
                        const auto *buffer_descriptor = static_cast<const BufferDescriptor *>(descriptor);
                        auto buf_state = buffer_descriptor->GetBufferState();
                        if (!buf_state) continue;
                        const ResourceAccessRange range =
                            MakeRange(*buf_state, buffer_descriptor->GetOffset(), buffer_descriptor->GetRange());
                        auto hazard = current_context_->DetectHazard(*buf_state, sync_index, range);
                        if (hazard.hazard && !sync_state_->SupressedBoundDescriptorWAW(hazard)) {
                            skip |= sync_state_->LogError(
                                buf_state->buffer, string_SyncHazardVUID(hazard.hazard),
                                "%s: Hazard %s for %s in %s, %s, and %s, type: %s, binding #%d index %d. Access info %s.",
                                func_name, string_SyncHazard(hazard.hazard),
                                sync_state_->report_data->FormatHandle(buf_state->buffer).c_str(),
                                sync_state_->report_data->FormatHandle(cb_state_->commandBuffer).c_str(),
                                sync_state_->report_data->FormatHandle(pPipe->pipeline).c_str(),
                                sync_state_->report_data->FormatHandle(descriptor_set->GetSet()).c_str(),
                                string_VkDescriptorType(descriptor_type), set_binding.first.second, index,
                                string_UsageTag(hazard).c_str());
                        }
                        break;
                    }
                    // TODO: INLINE_UNIFORM_BLOCK_EXT, ACCELERATION_STRUCTURE_KHR
                    default:
                        break;
                }
            }
        }
    }
    return skip;
}

void CommandBufferAccessContext::RecordDispatchDrawDescriptorSet(VkPipelineBindPoint pipelineBindPoint,
                                                                 const ResourceUsageTag &tag) {
    const PIPELINE_STATE *pPipe = nullptr;
    const std::vector<LAST_BOUND_STATE::PER_SET> *per_sets = nullptr;
    GetCurrentPipelineAndDesriptorSetsFromCommandBuffer(*cb_state_.get(), pipelineBindPoint, &pPipe, &per_sets);
    if (!pPipe || !per_sets) {
        return;
    }

    using DescriptorClass = cvdescriptorset::DescriptorClass;
    using BufferDescriptor = cvdescriptorset::BufferDescriptor;
    using ImageDescriptor = cvdescriptorset::ImageDescriptor;
    using ImageSamplerDescriptor = cvdescriptorset::ImageSamplerDescriptor;
    using TexelDescriptor = cvdescriptorset::TexelDescriptor;

    for (const auto &stage_state : pPipe->stage_state) {
        if (stage_state.stage_flag == VK_SHADER_STAGE_FRAGMENT_BIT && pPipe->graphicsPipelineCI.pRasterizationState &&
            pPipe->graphicsPipelineCI.pRasterizationState->rasterizerDiscardEnable)
            continue;
        for (const auto &set_binding : stage_state.descriptor_uses) {
            cvdescriptorset::DescriptorSet *descriptor_set = (*per_sets)[set_binding.first.first].bound_descriptor_set;
            cvdescriptorset::DescriptorSetLayout::ConstBindingIterator binding_it(descriptor_set->GetLayout().get(),
                                                                                  set_binding.first.second);
            const auto descriptor_type = binding_it.GetType();
            cvdescriptorset::IndexRange index_range = binding_it.GetGlobalIndexRange();
            auto array_idx = 0;

            if (binding_it.IsVariableDescriptorCount()) {
                index_range.end = index_range.start + descriptor_set->GetVariableDescriptorCount();
            }
            SyncStageAccessIndex sync_index =
                GetSyncStageAccessIndexsByDescriptorSet(descriptor_type, set_binding.second, stage_state.stage_flag);

            for (uint32_t i = index_range.start; i < index_range.end; ++i, ++array_idx) {
                const auto *descriptor = descriptor_set->GetDescriptorFromGlobalIndex(i);
                switch (descriptor->GetClass()) {
                    case DescriptorClass::ImageSampler:
                    case DescriptorClass::Image: {
                        const IMAGE_VIEW_STATE *img_view_state = nullptr;
                        if (descriptor->GetClass() == DescriptorClass::ImageSampler) {
                            img_view_state = static_cast<const ImageSamplerDescriptor *>(descriptor)->GetImageViewState();
                        } else {
                            img_view_state = static_cast<const ImageDescriptor *>(descriptor)->GetImageViewState();
                        }
                        if (!img_view_state) continue;
                        const IMAGE_STATE *img_state = img_view_state->image_state.get();
                        VkExtent3D extent = {};
                        VkOffset3D offset = {};
                        if (sync_index == SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ) {
                            extent = CastTo3D(cb_state_->activeRenderPassBeginInfo.renderArea.extent);
                            offset = CastTo3D(cb_state_->activeRenderPassBeginInfo.renderArea.offset);
                        } else {
                            extent = img_state->createInfo.extent;
                        }
                        current_context_->UpdateAccessState(*img_state, sync_index, img_view_state->normalized_subresource_range,
                                                            offset, extent, tag);
                        break;
                    }
                    case DescriptorClass::TexelBuffer: {
                        auto buf_view_state = static_cast<const TexelDescriptor *>(descriptor)->GetBufferViewState();
                        if (!buf_view_state) continue;
                        const BUFFER_STATE *buf_state = buf_view_state->buffer_state.get();
                        const ResourceAccessRange range = MakeRange(*buf_view_state);
                        current_context_->UpdateAccessState(*buf_state, sync_index, range, tag);
                        break;
                    }
                    case DescriptorClass::GeneralBuffer: {
                        const auto *buffer_descriptor = static_cast<const BufferDescriptor *>(descriptor);
                        auto buf_state = buffer_descriptor->GetBufferState();
                        if (!buf_state) continue;
                        const ResourceAccessRange range =
                            MakeRange(*buf_state, buffer_descriptor->GetOffset(), buffer_descriptor->GetRange());
                        current_context_->UpdateAccessState(*buf_state, sync_index, range, tag);
                        break;
                    }
                    // TODO: INLINE_UNIFORM_BLOCK_EXT, ACCELERATION_STRUCTURE_KHR
                    default:
                        break;
                }
            }
        }
    }
}

bool CommandBufferAccessContext::ValidateDrawVertex(uint32_t vertexCount, uint32_t firstVertex, const char *func_name) const {
    bool skip = false;
    const auto *pPipe = GetCurrentPipelineFromCommandBuffer(*cb_state_.get(), VK_PIPELINE_BIND_POINT_GRAPHICS);
    if (!pPipe) {
        return skip;
    }

    const auto &binding_buffers = cb_state_->current_vertex_buffer_binding_info.vertex_buffer_bindings;
    const auto &binding_buffers_size = binding_buffers.size();
    const auto &binding_descriptions_size = pPipe->vertex_binding_descriptions_.size();

    for (size_t i = 0; i < binding_descriptions_size; ++i) {
        const auto &binding_description = pPipe->vertex_binding_descriptions_[i];
        if (binding_description.binding < binding_buffers_size) {
            const auto &binding_buffer = binding_buffers[binding_description.binding];
            if (binding_buffer.buffer == VK_NULL_HANDLE) continue;

            auto *buf_state = sync_state_->Get<BUFFER_STATE>(binding_buffer.buffer);
            const ResourceAccessRange range = GetBufferRange(binding_buffer.offset, buf_state->createInfo.size, firstVertex,
                                                             vertexCount, binding_description.stride);
            auto hazard = current_context_->DetectHazard(*buf_state, SYNC_VERTEX_INPUT_VERTEX_ATTRIBUTE_READ, range);
            if (hazard.hazard) {
                skip |= sync_state_->LogError(
                    buf_state->buffer, string_SyncHazardVUID(hazard.hazard), "%s: Hazard %s for vertex %s in %s. Access info %s.",
                    func_name, string_SyncHazard(hazard.hazard), sync_state_->report_data->FormatHandle(buf_state->buffer).c_str(),
                    sync_state_->report_data->FormatHandle(cb_state_->commandBuffer).c_str(), string_UsageTag(hazard).c_str());
            }
        }
    }
    return skip;
}

void CommandBufferAccessContext::RecordDrawVertex(uint32_t vertexCount, uint32_t firstVertex, const ResourceUsageTag &tag) {
    const auto *pPipe = GetCurrentPipelineFromCommandBuffer(*cb_state_.get(), VK_PIPELINE_BIND_POINT_GRAPHICS);
    if (!pPipe) {
        return;
    }
    const auto &binding_buffers = cb_state_->current_vertex_buffer_binding_info.vertex_buffer_bindings;
    const auto &binding_buffers_size = binding_buffers.size();
    const auto &binding_descriptions_size = pPipe->vertex_binding_descriptions_.size();

    for (size_t i = 0; i < binding_descriptions_size; ++i) {
        const auto &binding_description = pPipe->vertex_binding_descriptions_[i];
        if (binding_description.binding < binding_buffers_size) {
            const auto &binding_buffer = binding_buffers[binding_description.binding];
            if (binding_buffer.buffer == VK_NULL_HANDLE) continue;

            auto *buf_state = sync_state_->Get<BUFFER_STATE>(binding_buffer.buffer);
            const ResourceAccessRange range = GetBufferRange(binding_buffer.offset, buf_state->createInfo.size, firstVertex,
                                                             vertexCount, binding_description.stride);
            current_context_->UpdateAccessState(*buf_state, SYNC_VERTEX_INPUT_VERTEX_ATTRIBUTE_READ, range, tag);
        }
    }
}

bool CommandBufferAccessContext::ValidateDrawVertexIndex(uint32_t indexCount, uint32_t firstIndex, const char *func_name) const {
    bool skip = false;
    if (cb_state_->index_buffer_binding.buffer == VK_NULL_HANDLE) return skip;

    auto *index_buf_state = sync_state_->Get<BUFFER_STATE>(cb_state_->index_buffer_binding.buffer);
    const auto index_size = GetIndexAlignment(cb_state_->index_buffer_binding.index_type);
    const ResourceAccessRange range = GetBufferRange(cb_state_->index_buffer_binding.offset, index_buf_state->createInfo.size,
                                                     firstIndex, indexCount, index_size);
    auto hazard = current_context_->DetectHazard(*index_buf_state, SYNC_VERTEX_INPUT_INDEX_READ, range);
    if (hazard.hazard) {
        skip |= sync_state_->LogError(
            index_buf_state->buffer, string_SyncHazardVUID(hazard.hazard), "%s: Hazard %s for index %s in %s. Access info %s.",
            func_name, string_SyncHazard(hazard.hazard), sync_state_->report_data->FormatHandle(index_buf_state->buffer).c_str(),
            sync_state_->report_data->FormatHandle(cb_state_->commandBuffer).c_str(), string_UsageTag(hazard).c_str());
    }

    // TODO: For now, we detect the whole vertex buffer. Index buffer could be changed until SubmitQueue.
    //       We will detect more accurate range in the future.
    skip |= ValidateDrawVertex(UINT32_MAX, 0, func_name);
    return skip;
}

void CommandBufferAccessContext::RecordDrawVertexIndex(uint32_t indexCount, uint32_t firstIndex, const ResourceUsageTag &tag) {
    if (cb_state_->index_buffer_binding.buffer == VK_NULL_HANDLE) return;

    auto *index_buf_state = sync_state_->Get<BUFFER_STATE>(cb_state_->index_buffer_binding.buffer);
    const auto index_size = GetIndexAlignment(cb_state_->index_buffer_binding.index_type);
    const ResourceAccessRange range = GetBufferRange(cb_state_->index_buffer_binding.offset, index_buf_state->createInfo.size,
                                                     firstIndex, indexCount, index_size);
    current_context_->UpdateAccessState(*index_buf_state, SYNC_VERTEX_INPUT_INDEX_READ, range, tag);

    // TODO: For now, we detect the whole vertex buffer. Index buffer could be changed until SubmitQueue.
    //       We will detect more accurate range in the future.
    RecordDrawVertex(UINT32_MAX, 0, tag);
}

bool CommandBufferAccessContext::ValidateDrawSubpassAttachment(const char *func_name) const {
    bool skip = false;
    if (!current_renderpass_context_) return skip;
    skip |= current_renderpass_context_->ValidateDrawSubpassAttachment(*sync_state_, *cb_state_.get(),
                                                                       cb_state_->activeRenderPassBeginInfo.renderArea, func_name);
    return skip;
}

void CommandBufferAccessContext::RecordDrawSubpassAttachment(const ResourceUsageTag &tag) {
    if (current_renderpass_context_)
        current_renderpass_context_->RecordDrawSubpassAttachment(*cb_state_.get(), cb_state_->activeRenderPassBeginInfo.renderArea,
                                                                 tag);
}

bool CommandBufferAccessContext::ValidateNextSubpass(const char *func_name) const {
    bool skip = false;
    if (!current_renderpass_context_) return skip;
    skip |=
        current_renderpass_context_->ValidateNextSubpass(*sync_state_, cb_state_->activeRenderPassBeginInfo.renderArea, func_name);

    return skip;
}

bool CommandBufferAccessContext::ValidateEndRenderpass(const char *func_name) const {
    // TODO: Things to add here.
    // Validate Preserve attachments
    bool skip = false;
    if (!current_renderpass_context_) return skip;
    skip |= current_renderpass_context_->ValidateEndRenderPass(*sync_state_, cb_state_->activeRenderPassBeginInfo.renderArea,
                                                               func_name);

    return skip;
}

void CommandBufferAccessContext::RecordBeginRenderPass(const ResourceUsageTag &tag) {
    assert(sync_state_);
    if (!cb_state_) return;

    // Create an access context the current renderpass.
    render_pass_contexts_.emplace_back();
    current_renderpass_context_ = &render_pass_contexts_.back();
    current_renderpass_context_->RecordBeginRenderPass(*sync_state_, *cb_state_, &cb_access_context_, queue_flags_, tag);
    current_context_ = &current_renderpass_context_->CurrentContext();
}

void CommandBufferAccessContext::RecordNextSubpass(const RENDER_PASS_STATE &rp_state, const ResourceUsageTag &tag) {
    assert(current_renderpass_context_);
    current_renderpass_context_->RecordNextSubpass(cb_state_->activeRenderPassBeginInfo.renderArea, tag);
    current_context_ = &current_renderpass_context_->CurrentContext();
}

void CommandBufferAccessContext::RecordEndRenderPass(const RENDER_PASS_STATE &render_pass, const ResourceUsageTag &tag) {
    assert(current_renderpass_context_);
    if (!current_renderpass_context_) return;

    current_renderpass_context_->RecordEndRenderPass(&cb_access_context_, cb_state_->activeRenderPassBeginInfo.renderArea, tag);
    current_context_ = &cb_access_context_;
    current_renderpass_context_ = nullptr;
}

bool RenderPassAccessContext::ValidateDrawSubpassAttachment(const SyncValidator &sync_state, const CMD_BUFFER_STATE &cmd,
                                                            const VkRect2D &render_area, const char *func_name) const {
    bool skip = false;
    const auto *pPipe = GetCurrentPipelineFromCommandBuffer(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS);
    if (!pPipe ||
        (pPipe->graphicsPipelineCI.pRasterizationState && pPipe->graphicsPipelineCI.pRasterizationState->rasterizerDiscardEnable)) {
        return skip;
    }
    const auto &list = pPipe->fragmentShader_writable_output_location_list;
    const auto &subpass = rp_state_->createInfo.pSubpasses[current_subpass_];
    VkExtent3D extent = CastTo3D(render_area.extent);
    VkOffset3D offset = CastTo3D(render_area.offset);

    const auto &current_context = CurrentContext();
    // Subpass's inputAttachment has been done in ValidateDispatchDrawDescriptorSet
    if (subpass.pColorAttachments && subpass.colorAttachmentCount && !list.empty()) {
        for (const auto location : list) {
            if (location >= subpass.colorAttachmentCount || subpass.pColorAttachments[location].attachment == VK_ATTACHMENT_UNUSED)
                continue;
            const IMAGE_VIEW_STATE *img_view_state = attachment_views_[subpass.pColorAttachments[location].attachment];
            HazardResult hazard = current_context.DetectHazard(img_view_state, SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE,
                                                               kColorAttachmentRasterOrder, offset, extent);
            if (hazard.hazard) {
                skip |= sync_state.LogError(img_view_state->image_view, string_SyncHazardVUID(hazard.hazard),
                                            "%s: Hazard %s for %s in %s, Subpass #%d, and pColorAttachments #%d. Access info %s.",
                                            func_name, string_SyncHazard(hazard.hazard),
                                            sync_state.report_data->FormatHandle(img_view_state->image_view).c_str(),
                                            sync_state.report_data->FormatHandle(cmd.commandBuffer).c_str(), cmd.activeSubpass,
                                            location, string_UsageTag(hazard).c_str());
            }
        }
    }

    // PHASE1 TODO: Add layout based read/vs. write selection.
    // PHASE1 TODO: Read operations for both depth and stencil are possible in the future.
    if (pPipe->graphicsPipelineCI.pDepthStencilState && subpass.pDepthStencilAttachment &&
        subpass.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
        const IMAGE_VIEW_STATE *img_view_state = attachment_views_[subpass.pDepthStencilAttachment->attachment];
        bool depth_write = false, stencil_write = false;

        // PHASE1 TODO: These validation should be in core_checks.
        if (!FormatIsStencilOnly(img_view_state->create_info.format) &&
            pPipe->graphicsPipelineCI.pDepthStencilState->depthTestEnable &&
            pPipe->graphicsPipelineCI.pDepthStencilState->depthWriteEnable &&
            IsImageLayoutDepthWritable(subpass.pDepthStencilAttachment->layout)) {
            depth_write = true;
        }
        // PHASE1 TODO: It needs to check if stencil is writable.
        //              If failOp, passOp, or depthFailOp are not KEEP, and writeMask isn't 0, it's writable.
        //              If depth test is disable, it's considered depth test passes, and then depthFailOp doesn't run.
        // PHASE1 TODO: These validation should be in core_checks.
        if (!FormatIsDepthOnly(img_view_state->create_info.format) &&
            pPipe->graphicsPipelineCI.pDepthStencilState->stencilTestEnable &&
            IsImageLayoutStencilWritable(subpass.pDepthStencilAttachment->layout)) {
            stencil_write = true;
        }

        // PHASE1 TODO: Add EARLY stage detection based on ExecutionMode.
        if (depth_write) {
            HazardResult hazard =
                current_context.DetectHazard(img_view_state, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                             kDepthStencilAttachmentRasterOrder, offset, extent, VK_IMAGE_ASPECT_DEPTH_BIT);
            if (hazard.hazard) {
                skip |= sync_state.LogError(
                    img_view_state->image_view, string_SyncHazardVUID(hazard.hazard),
                    "%s: Hazard %s for %s in %s, Subpass #%d, and depth part of pDepthStencilAttachment. Access info %s.",
                    func_name, string_SyncHazard(hazard.hazard),
                    sync_state.report_data->FormatHandle(img_view_state->image_view).c_str(),
                    sync_state.report_data->FormatHandle(cmd.commandBuffer).c_str(), cmd.activeSubpass,
                    string_UsageTag(hazard).c_str());
            }
        }
        if (stencil_write) {
            HazardResult hazard =
                current_context.DetectHazard(img_view_state, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                             kDepthStencilAttachmentRasterOrder, offset, extent, VK_IMAGE_ASPECT_STENCIL_BIT);
            if (hazard.hazard) {
                skip |= sync_state.LogError(
                    img_view_state->image_view, string_SyncHazardVUID(hazard.hazard),
                    "%s: Hazard %s for %s in %s, Subpass #%d, and stencil part of pDepthStencilAttachment. Access info %s.",
                    func_name, string_SyncHazard(hazard.hazard),
                    sync_state.report_data->FormatHandle(img_view_state->image_view).c_str(),
                    sync_state.report_data->FormatHandle(cmd.commandBuffer).c_str(), cmd.activeSubpass,
                    string_UsageTag(hazard).c_str());
            }
        }
    }
    return skip;
}

void RenderPassAccessContext::RecordDrawSubpassAttachment(const CMD_BUFFER_STATE &cmd, const VkRect2D &render_area,
                                                          const ResourceUsageTag &tag) {
    const auto *pPipe = GetCurrentPipelineFromCommandBuffer(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS);
    if (!pPipe ||
        (pPipe->graphicsPipelineCI.pRasterizationState && pPipe->graphicsPipelineCI.pRasterizationState->rasterizerDiscardEnable)) {
        return;
    }
    const auto &list = pPipe->fragmentShader_writable_output_location_list;
    const auto &subpass = rp_state_->createInfo.pSubpasses[current_subpass_];
    VkExtent3D extent = CastTo3D(render_area.extent);
    VkOffset3D offset = CastTo3D(render_area.offset);

    auto &current_context = CurrentContext();
    // Subpass's inputAttachment has been done in RecordDispatchDrawDescriptorSet
    if (subpass.pColorAttachments && subpass.colorAttachmentCount && !list.empty()) {
        for (const auto location : list) {
            if (location >= subpass.colorAttachmentCount || subpass.pColorAttachments[location].attachment == VK_ATTACHMENT_UNUSED)
                continue;
            const IMAGE_VIEW_STATE *img_view_state = attachment_views_[subpass.pColorAttachments[location].attachment];
            current_context.UpdateAccessState(img_view_state, SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, offset, extent,
                                              0, tag);
        }
    }

    // PHASE1 TODO: Add layout based read/vs. write selection.
    // PHASE1 TODO: Read operations for both depth and stencil are possible in the future.
    if (pPipe->graphicsPipelineCI.pDepthStencilState && subpass.pDepthStencilAttachment &&
        subpass.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED) {
        const IMAGE_VIEW_STATE *img_view_state = attachment_views_[subpass.pDepthStencilAttachment->attachment];
        bool depth_write = false, stencil_write = false;

        // PHASE1 TODO: These validation should be in core_checks.
        if (!FormatIsStencilOnly(img_view_state->create_info.format) &&
            pPipe->graphicsPipelineCI.pDepthStencilState->depthTestEnable &&
            pPipe->graphicsPipelineCI.pDepthStencilState->depthWriteEnable &&
            IsImageLayoutDepthWritable(subpass.pDepthStencilAttachment->layout)) {
            depth_write = true;
        }
        // PHASE1 TODO: It needs to check if stencil is writable.
        //              If failOp, passOp, or depthFailOp are not KEEP, and writeMask isn't 0, it's writable.
        //              If depth test is disable, it's considered depth test passes, and then depthFailOp doesn't run.
        // PHASE1 TODO: These validation should be in core_checks.
        if (!FormatIsDepthOnly(img_view_state->create_info.format) &&
            pPipe->graphicsPipelineCI.pDepthStencilState->stencilTestEnable &&
            IsImageLayoutStencilWritable(subpass.pDepthStencilAttachment->layout)) {
            stencil_write = true;
        }

        // PHASE1 TODO: Add EARLY stage detection based on ExecutionMode.
        if (depth_write) {
            current_context.UpdateAccessState(img_view_state, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, offset,
                                              extent, VK_IMAGE_ASPECT_DEPTH_BIT, tag);
        }
        if (stencil_write) {
            current_context.UpdateAccessState(img_view_state, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, offset,
                                              extent, VK_IMAGE_ASPECT_STENCIL_BIT, tag);
        }
    }
}

bool RenderPassAccessContext::ValidateNextSubpass(const SyncValidator &sync_state, const VkRect2D &render_area,
                                                  const char *func_name) const {
    // PHASE1 TODO: Add Validate Preserve attachments
    bool skip = false;
    skip |= CurrentContext().ValidateResolveOperations(sync_state, *rp_state_, render_area, attachment_views_, func_name,
                                                       current_subpass_);
    skip |= CurrentContext().ValidateStoreOperation(sync_state, *rp_state_, render_area, current_subpass_, attachment_views_,
                                                    func_name);

    const auto next_subpass = current_subpass_ + 1;
    const auto &next_context = subpass_contexts_[next_subpass];
    skip |= next_context.ValidateLayoutTransitions(sync_state, *rp_state_, render_area, next_subpass, attachment_views_, func_name);
    if (!skip) {
        // To avoid complex (and buggy) duplication of the affect of layout transitions on load operations, we'll record them
        // on a copy of the (empty) next context.
        // Note: The resource access map should be empty so hopefully this copy isn't too horrible from a perf POV.
        AccessContext temp_context(next_context);
        temp_context.RecordLayoutTransitions(*rp_state_, next_subpass, attachment_views_, kCurrentCommandTag);
        skip |= temp_context.ValidateLoadOperation(sync_state, *rp_state_, render_area, next_subpass, attachment_views_, func_name);
    }
    return skip;
}
bool RenderPassAccessContext::ValidateEndRenderPass(const SyncValidator &sync_state, const VkRect2D &render_area,
                                                    const char *func_name) const {
    // PHASE1 TODO: Validate Preserve
    bool skip = false;
    skip |= CurrentContext().ValidateResolveOperations(sync_state, *rp_state_, render_area, attachment_views_, func_name,
                                                       current_subpass_);
    skip |= CurrentContext().ValidateStoreOperation(sync_state, *rp_state_, render_area, current_subpass_, attachment_views_,
                                                    func_name);
    skip |= ValidateFinalSubpassLayoutTransitions(sync_state, render_area, func_name);
    return skip;
}

AccessContext *RenderPassAccessContext::CreateStoreResolveProxy(const VkRect2D &render_area) const {
    return CreateStoreResolveProxyContext(CurrentContext(), *rp_state_, current_subpass_, render_area, attachment_views_);
}

bool RenderPassAccessContext::ValidateFinalSubpassLayoutTransitions(const SyncValidator &sync_state, const VkRect2D &render_area,
                                                                    const char *func_name) const {
    bool skip = false;

    // As validation methods are const and precede the record/update phase, for any tranistions from the current (last)
    // subpass, we have to validate them against a copy of the current AccessContext, with resolve operations applied.
    // Note: we could be more efficient by tracking whether or not we actually *have* any changes (e.g. attachment resolve)
    // to apply and only copy then, if this proves a hot spot.
    std::unique_ptr<AccessContext> proxy_for_current;

    // Validate the "finalLayout" transitions to external
    // Get them from where there we're hidding in the extra entry.
    const auto &final_transitions = rp_state_->subpass_transitions.back();
    for (const auto &transition : final_transitions) {
        const auto &attach_view = attachment_views_[transition.attachment];
        const auto &trackback = subpass_contexts_[transition.prev_pass].GetDstExternalTrackBack();
        assert(trackback.context);  // Transitions are given implicit transitions if the StateTracker is working correctly
        auto *context = trackback.context;

        if (transition.prev_pass == current_subpass_) {
            if (!proxy_for_current) {
                // We haven't recorded resolve ofor the current_subpass, so we need to copy current and update it *as if*
                proxy_for_current.reset(CreateStoreResolveProxy(render_area));
            }
            context = proxy_for_current.get();
        }

        // Use the merged barrier for the hazard check (safe since it just considers the src (first) scope.
        const auto merged_barrier = MergeBarriers(trackback.barriers);
        auto hazard = context->DetectImageBarrierHazard(*attach_view->image_state, merged_barrier.src_exec_scope,
                                                        merged_barrier.src_access_scope, attach_view->normalized_subresource_range,
                                                        AccessContext::DetectOptions::kDetectPrevious);
        if (hazard.hazard) {
            skip |= sync_state.LogError(rp_state_->renderPass, string_SyncHazardVUID(hazard.hazard),
                                        "%s: Hazard %s with last use subpass %" PRIu32 " for attachment %" PRIu32
                                        " final image layout transition (old_layout: %s, new_layout: %s). Access info %s.",
                                        func_name, string_SyncHazard(hazard.hazard), transition.prev_pass, transition.attachment,
                                        string_VkImageLayout(transition.old_layout), string_VkImageLayout(transition.new_layout),
                                        string_UsageTag(hazard).c_str());
        }
    }
    return skip;
}

void RenderPassAccessContext::RecordLayoutTransitions(const ResourceUsageTag &tag) {
    // Add layout transitions...
    subpass_contexts_[current_subpass_].RecordLayoutTransitions(*rp_state_, current_subpass_, attachment_views_, tag);
}

void RenderPassAccessContext::RecordLoadOperations(const VkRect2D &render_area, const ResourceUsageTag &tag) {
    const auto *attachment_ci = rp_state_->createInfo.pAttachments;
    auto &subpass_context = subpass_contexts_[current_subpass_];
    VkExtent3D extent = CastTo3D(render_area.extent);
    VkOffset3D offset = CastTo3D(render_area.offset);

    for (uint32_t i = 0; i < rp_state_->createInfo.attachmentCount; i++) {
        if (rp_state_->attachment_first_subpass[i] == current_subpass_) {
            if (attachment_views_[i] == nullptr) continue;  // UNUSED
            const auto &view = *attachment_views_[i];
            const IMAGE_STATE *image = view.image_state.get();
            if (image == nullptr) continue;

            const auto &ci = attachment_ci[i];
            const bool has_depth = FormatHasDepth(ci.format);
            const bool has_stencil = FormatHasStencil(ci.format);
            const bool is_color = !(has_depth || has_stencil);

            if (is_color) {
                subpass_context.UpdateAccessState(*image, ColorLoadUsage(ci.loadOp), view.normalized_subresource_range, offset,
                                                  extent, tag);
            } else {
                auto update_range = view.normalized_subresource_range;
                if (has_depth) {
                    update_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                    subpass_context.UpdateAccessState(*image, DepthStencilLoadUsage(ci.loadOp), update_range, offset, extent, tag);
                }
                if (has_stencil) {
                    update_range.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
                    subpass_context.UpdateAccessState(*image, DepthStencilLoadUsage(ci.stencilLoadOp), update_range, offset, extent,
                                                      tag);
                }
            }
        }
    }
}

void RenderPassAccessContext::RecordBeginRenderPass(const SyncValidator &state, const CMD_BUFFER_STATE &cb_state,
                                                    const AccessContext *external_context, VkQueueFlags queue_flags,
                                                    const ResourceUsageTag &tag) {
    current_subpass_ = 0;
    rp_state_ = cb_state.activeRenderPass.get();
    subpass_contexts_.reserve(rp_state_->createInfo.subpassCount);
    // Add this for all subpasses here so that they exsist during next subpass validation
    for (uint32_t pass = 0; pass < rp_state_->createInfo.subpassCount; pass++) {
        subpass_contexts_.emplace_back(pass, queue_flags, rp_state_->subpass_dependencies, subpass_contexts_, external_context);
    }
    attachment_views_ = state.GetCurrentAttachmentViews(cb_state);

    RecordLayoutTransitions(tag);
    RecordLoadOperations(cb_state.activeRenderPassBeginInfo.renderArea, tag);
}

void RenderPassAccessContext::RecordNextSubpass(const VkRect2D &render_area, const ResourceUsageTag &tag) {
    // Resolves are against *prior* subpass context and thus *before* the subpass increment
    CurrentContext().UpdateAttachmentResolveAccess(*rp_state_, render_area, attachment_views_, current_subpass_, tag);
    CurrentContext().UpdateAttachmentStoreAccess(*rp_state_, render_area, attachment_views_, current_subpass_, tag);

    current_subpass_++;
    assert(current_subpass_ < subpass_contexts_.size());
    RecordLayoutTransitions(tag);
    RecordLoadOperations(render_area, tag);
}

void RenderPassAccessContext::RecordEndRenderPass(AccessContext *external_context, const VkRect2D &render_area,
                                                  const ResourceUsageTag &tag) {
    // Add the resolve and store accesses
    CurrentContext().UpdateAttachmentResolveAccess(*rp_state_, render_area, attachment_views_, current_subpass_, tag);
    CurrentContext().UpdateAttachmentStoreAccess(*rp_state_, render_area, attachment_views_, current_subpass_, tag);

    // Export the accesses from the renderpass...
    external_context->ResolveChildContexts(subpass_contexts_);

    // Add the "finalLayout" transitions to external
    // Get them from where there we're hidding in the extra entry.
    // Not that since *final* always comes from *one* subpass per view, we don't have to accumulate the barriers
    // TODO Aliasing we may need to reconsider barrier accumulation... though I don't know that it would be valid for aliasing
    //      that had mulitple final layout transistions from mulitple final subpasses.
    const auto &final_transitions = rp_state_->subpass_transitions.back();
    for (const auto &transition : final_transitions) {
        const auto &attachment = attachment_views_[transition.attachment];
        const auto &last_trackback = subpass_contexts_[transition.prev_pass].GetDstExternalTrackBack();
        assert(&subpass_contexts_[transition.prev_pass] == last_trackback.context);
        ApplyBarrierOpsFunctor barrier_ops(true /* resolve */, last_trackback.barriers, true /* layout transition */, tag);
        external_context->UpdateResourceAccess(*attachment->image_state, attachment->normalized_subresource_range, barrier_ops);
    }
}

SyncBarrier::SyncBarrier(VkQueueFlags queue_flags, const VkSubpassDependency2 &barrier) {
    const auto src_stage_mask = ExpandPipelineStages(queue_flags, barrier.srcStageMask);
    src_exec_scope = WithEarlierPipelineStages(src_stage_mask);
    src_access_scope = SyncStageAccess::AccessScope(src_stage_mask, barrier.srcAccessMask);
    const auto dst_stage_mask = ExpandPipelineStages(queue_flags, barrier.dstStageMask);
    dst_exec_scope = WithLaterPipelineStages(dst_stage_mask);
    dst_access_scope = SyncStageAccess::AccessScope(dst_stage_mask, barrier.dstAccessMask);
}

// Apply a list of barriers, without resolving pending state, useful for subpass layout transitions
void ResourceAccessState::ApplyBarriers(const std::vector<SyncBarrier> &barriers, bool layout_transition) {
    for (const auto &barrier : barriers) {
        ApplyBarrier(barrier, layout_transition);
    }
}

// ApplyBarriers is design for *fully* inclusive barrier lists without layout tranistions.  Designed use was for
// inter-subpass barriers for lazy-evaluation of parent context memory ranges.  Subpass layout transistions are *not* done
// lazily, s.t. no previous access reports should need layout transitions.
void ResourceAccessState::ApplyBarriers(const std::vector<SyncBarrier> &barriers, const ResourceUsageTag &tag) {
    assert(!pending_layout_transition);  // This should never be call in the middle of another barrier application
    assert(!pending_write_barriers);
    assert(!pending_write_dep_chain);
    for (const auto &barrier : barriers) {
        ApplyBarrier(barrier, false);
    }
    ApplyPendingBarriers(tag);
}
HazardResult ResourceAccessState::DetectHazard(SyncStageAccessIndex usage_index) const {
    HazardResult hazard;
    auto usage = FlagBit(usage_index);
    const auto usage_stage = PipelineStageBit(usage_index);
    if (IsRead(usage)) {
        if (IsRAWHazard(usage_stage, usage)) {
            hazard.Set(this, usage_index, READ_AFTER_WRITE, last_write, write_tag);
        }
    } else {
        // Write operation:
        // Check for read operations more recent than last_write (as setting last_write clears reads, that would be *any*
        // If reads exists -- test only against them because either:
        //     * the reads were hazards, and we've reported the hazard, so just test the current write vs. the read operations
        //     * the read weren't hazards, and thus if the write is safe w.r.t. the reads, no hazard vs. last_write is possible if
        //       the current write happens after the reads, so just test the write against the reades
        // Otherwise test against last_write
        //
        // Look for casus belli for WAR
        if (last_read_count) {
            for (uint32_t read_index = 0; read_index < last_read_count; read_index++) {
                const auto &read_access = last_reads[read_index];
                if (IsReadHazard(usage_stage, read_access)) {
                    hazard.Set(this, usage_index, WRITE_AFTER_READ, read_access.access, read_access.tag);
                    break;
                }
            }
        } else if (last_write && IsWriteHazard(usage)) {
            // Write-After-Write check -- if we have a previous write to test against
            hazard.Set(this, usage_index, WRITE_AFTER_WRITE, last_write, write_tag);
        }
    }
    return hazard;
}

HazardResult ResourceAccessState::DetectHazard(SyncStageAccessIndex usage_index, const SyncOrderingBarrier &ordering) const {
    // The ordering guarantees act as barriers to the last accesses, independent of synchronization operations
    HazardResult hazard;
    const auto usage_bit = FlagBit(usage_index);
    const auto usage_stage = PipelineStageBit(usage_index);
    const bool input_attachment_ordering = 0 != (ordering.access_scope & SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ_BIT);
    const bool last_write_is_ordered = 0 != (last_write & ordering.access_scope);
    if (IsRead(usage_bit)) {
        // Exclude RAW if no write, or write not most "most recent" operation w.r.t. usage;
        bool is_raw_hazard = IsRAWHazard(usage_stage, usage_bit);
        if (is_raw_hazard) {
            // NOTE: we know last_write is non-zero
            // See if the ordering rules save us from the simple RAW check above
            // First check to see if the current usage is covered by the ordering rules
            const bool usage_is_input_attachment = (usage_index == SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ);
            const bool usage_is_ordered =
                (input_attachment_ordering && usage_is_input_attachment) || (0 != (usage_stage & ordering.exec_scope));
            if (usage_is_ordered) {
                // Now see of the most recent write (or a subsequent read) are ordered
                const bool most_recent_is_ordered = last_write_is_ordered || (0 != GetOrderedStages(ordering));
                is_raw_hazard = !most_recent_is_ordered;
            }
        }
        if (is_raw_hazard) {
            hazard.Set(this, usage_index, READ_AFTER_WRITE, last_write, write_tag);
        }
    } else {
        // Only check for WAW if there are no reads since last_write
        bool usage_write_is_ordered = 0 != (usage_bit & ordering.access_scope);
        if (last_read_count) {
            // Look for any WAR hazards outside the ordered set of stages
            VkPipelineStageFlags ordered_stages = 0;
            if (usage_write_is_ordered) {
                // If the usage is ordered, we can ignore all ordered read stages w.r.t. WAR)
                ordered_stages = GetOrderedStages(ordering);
            }
            // If we're tracking any reads that aren't ordered against the current write, got to check 'em all.
            if ((ordered_stages & last_read_stages) != last_read_stages) {
                for (uint32_t read_index = 0; read_index < last_read_count; read_index++) {
                    const auto &read_access = last_reads[read_index];
                    if (read_access.stage & ordered_stages) continue;  // but we can skip the ordered ones
                    if (IsReadHazard(usage_stage, read_access)) {
                        hazard.Set(this, usage_index, WRITE_AFTER_READ, read_access.access, read_access.tag);
                        break;
                    }
                }
            }
        } else if (!(last_write_is_ordered && usage_write_is_ordered)) {
            if (last_write && IsWriteHazard(usage_bit)) {
                hazard.Set(this, usage_index, WRITE_AFTER_WRITE, last_write, write_tag);
            }
        }
    }
    return hazard;
}

// Asynchronous Hazards occur between subpasses with no connection through the DAG
HazardResult ResourceAccessState::DetectAsyncHazard(SyncStageAccessIndex usage_index) const {
    HazardResult hazard;
    auto usage = FlagBit(usage_index);
    if (IsRead(usage)) {
        if (last_write != 0) {
            hazard.Set(this, usage_index, READ_RACING_WRITE, last_write, write_tag);
        }
    } else {
        if (last_write != 0) {
            hazard.Set(this, usage_index, WRITE_RACING_WRITE, last_write, write_tag);
        } else if (last_read_count > 0) {
            // Any read could be reported, so we'll just pick the first one arbitrarily
            hazard.Set(this, usage_index, WRITE_RACING_READ, last_reads[0].access, last_reads[0].tag);
        }
    }
    return hazard;
}

HazardResult ResourceAccessState::DetectBarrierHazard(SyncStageAccessIndex usage_index, VkPipelineStageFlags src_exec_scope,
                                                      SyncStageAccessFlags src_access_scope) const {
    // Only supporting image layout transitions for now
    assert(usage_index == SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION);
    HazardResult hazard;
    // only test for WAW if there no intervening read operations.
    // See DetectHazard(SyncStagetAccessIndex) above for more details.
    if (last_read_count) {
        // Look at the reads if any
        for (uint32_t read_index = 0; read_index < last_read_count; read_index++) {
            const auto &read_access = last_reads[read_index];
            // If the read stage is not in the src sync sync
            // *AND* not execution chained with an existing sync barrier (that's the or)
            // then the barrier access is unsafe (R/W after R)
            if ((src_exec_scope & (read_access.stage | read_access.barriers)) == 0) {
                hazard.Set(this, usage_index, WRITE_AFTER_READ, read_access.access, read_access.tag);
                break;
            }
        }
    } else if (last_write) {
        // If the previous write is *not* in the 1st access scope
        // *AND* the current barrier is not in the dependency chain
        // *AND* the there is no prior memory barrier for the previous write in the dependency chain
        // then the barrier access is unsafe (R/W after W)
        if (((last_write & src_access_scope) == 0) && ((src_exec_scope & write_dependency_chain) == 0) && (write_barriers == 0)) {
            // TODO: Do we need a difference hazard name for this?
            hazard.Set(this, usage_index, WRITE_AFTER_WRITE, last_write, write_tag);
        }
    }

    return hazard;
}

// The logic behind resolves is the same as update, we assume that earlier hazards have be reported, and that no
// tranistive hazard can exists with a hazard between the earlier operations.  Yes, an early hazard can mask that another
// exists, but if you fix *that* hazard it either fixes or unmasks the subsequent ones.
void ResourceAccessState::Resolve(const ResourceAccessState &other) {
    if (write_tag.IsBefore(other.write_tag)) {
        // If this is a later write, we've reported any exsiting hazard, and we can just overwrite as the more recent
        // operation
        *this = other;
    } else if (!other.write_tag.IsBefore(write_tag)) {
        // This is the *equals* case for write operations, we merged the write barriers and the read state (but without the
        // dependency chaining logic or any stage expansion)
        write_barriers |= other.write_barriers;
        pending_write_barriers |= other.pending_write_barriers;
        pending_layout_transition |= other.pending_layout_transition;
        pending_write_dep_chain |= other.pending_write_dep_chain;

        // Merge the read states
        const auto pre_merge_count = last_read_count;
        const auto pre_merge_stages = last_read_stages;
        for (uint32_t other_read_index = 0; other_read_index < other.last_read_count; other_read_index++) {
            auto &other_read = other.last_reads[other_read_index];
            if (pre_merge_stages & other_read.stage) {
                // Merge in the barriers for read stages that exist in *both* this and other
                // TODO: This is N^2 with stages... perhaps the ReadStates should be sorted by stage index.
                //       but we should wait on profiling data for that.
                for (uint32_t my_read_index = 0; my_read_index < pre_merge_count; my_read_index++) {
                    auto &my_read = last_reads[my_read_index];
                    if (other_read.stage == my_read.stage) {
                        if (my_read.tag.IsBefore(other_read.tag)) {
                            // Other is more recent, copy in the state
                            my_read.access = other_read.access;
                            my_read.tag = other_read.tag;
                            my_read.pending_dep_chain = other_read.pending_dep_chain;
                            // TODO: Phase 2 -- review the state merge logic to avoid false positive from overwriting the barriers
                            //                  May require tracking more than one access per stage.
                            my_read.barriers = other_read.barriers;
                            if (my_read.stage == VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT) {
                                // Since I'm overwriting the fragement stage read, also update the input attachment info
                                // as this is the only stage that affects it.
                                input_attachment_read = other.input_attachment_read;
                            }
                        } else if (other_read.tag.IsBefore(my_read.tag)) {
                            // The read tags match so merge the barriers
                            my_read.barriers |= other_read.barriers;
                            my_read.pending_dep_chain |= other_read.pending_dep_chain;
                        }

                        break;
                    }
                }
            } else {
                // The other read stage doesn't exist in this, so add it.
                last_reads[last_read_count] = other_read;
                last_read_count++;
                last_read_stages |= other_read.stage;
                if (other_read.stage == VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT) {
                    input_attachment_read = other.input_attachment_read;
                }
            }
        }
        read_execution_barriers |= other.read_execution_barriers;
    }  // the else clause would be that other write is before this write... in which case we supercede the other state and
       // ignore it.
}

void ResourceAccessState::Update(SyncStageAccessIndex usage_index, const ResourceUsageTag &tag) {
    // Move this logic in the ResourceStateTracker as methods, thereof (or we'll repeat it for every flavor of resource...
    const auto usage_bit = FlagBit(usage_index);
    if (IsRead(usage_index)) {
        // Mulitple outstanding reads may be of interest and do dependency chains independently
        // However, for purposes of barrier tracking, only one read per pipeline stage matters
        const auto usage_stage = PipelineStageBit(usage_index);
        uint32_t update_index = kStageCount;
        if (usage_stage & last_read_stages) {
            for (uint32_t read_index = 0; read_index < last_read_count; read_index++) {
                if (last_reads[read_index].stage == usage_stage) {
                    update_index = read_index;
                    break;
                }
            }
            assert(update_index < last_read_count);
        } else {
            assert(last_read_count < last_reads.size());
            update_index = last_read_count++;
            last_read_stages |= usage_stage;
        }
        last_reads[update_index].Set(usage_stage, usage_bit, 0, tag);

        // Fragment shader reads come in two flavors, and we need to track if the one we're tracking is the special one.
        if (usage_stage == VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT) {
            // TODO Revisit re: multiple reads for a given stage
            input_attachment_read = (usage_bit == SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ_BIT);
        }
    } else {
        // Assume write
        // TODO determine what to do with READ-WRITE operations if any
        SetWrite(usage_bit, tag);
    }
}

// Clobber last read and all barriers... because all we have is DANGER, DANGER, WILL ROBINSON!!!
// if the last_reads/last_write were unsafe, we've reported them, in either case the prior access is irrelevant.
// We can overwrite them as *this* write is now after them.
//
// Note: intentionally ignore pending barriers and chains (i.e. don't apply or clear them), let ApplyPendingBarriers handle them.
void ResourceAccessState::SetWrite(SyncStageAccessFlagBits usage_bit, const ResourceUsageTag &tag) {
    last_read_count = 0;
    last_read_stages = 0;
    read_execution_barriers = 0;
    input_attachment_read = false;  // Denotes no outstanding input attachment read after the last write.

    write_barriers = 0;
    write_dependency_chain = 0;
    write_tag = tag;
    last_write = usage_bit;
}

// Apply the memory barrier without updating the existing barriers.  The execution barrier
// changes the "chaining" state, but to keep barriers independent, we defer this until all barriers
// of the batch have been processed. Also, depending on whether layout transition happens, we'll either
// replace the current write barriers or add to them, so accumulate to pending as well.
void ResourceAccessState::ApplyBarrier(const SyncBarrier &barrier, bool layout_transition) {
    // For independent barriers we need to track what the new barriers and dependency chain *will* be when we're done
    // applying the memory barriers
    // NOTE: We update the write barrier if the write is in the first access scope or if there is a layout
    //       transistion, under the theory of "most recent access".  If the read/write *isn't* safe
    //       vs. this layout transition DetectBarrierHazard should report it.  We treat the layout
    //       transistion *as* a write and in scope with the barrier (it's before visibility).
    if (layout_transition || InSourceScopeOrChain(barrier.src_exec_scope, barrier.src_access_scope)) {
        pending_write_barriers |= barrier.dst_access_scope;
        pending_write_dep_chain |= barrier.dst_exec_scope;
    }
    // Track layout transistion as pending as we can't modify last_write until all barriers processed
    pending_layout_transition |= layout_transition;

    if (!pending_layout_transition) {
        // Once we're dealing with a layout transition (which is modelled as a *write*) then the last reads/writes/chains
        // don't need to be tracked as we're just going to zero them.
        for (uint32_t read_index = 0; read_index < last_read_count; read_index++) {
            ReadState &access = last_reads[read_index];
            // The | implements the "dependency chain" logic for this access, as the barriers field stores the second sync scope
            if (barrier.src_exec_scope & (access.stage | access.barriers)) {
                access.pending_dep_chain |= barrier.dst_exec_scope;
            }
        }
    }
}

void ResourceAccessState::ApplyPendingBarriers(const ResourceUsageTag &tag) {
    if (pending_layout_transition) {
        // SetWrite clobbers the read count, and thus we don't have to clear the read_state out.
        SetWrite(SYNC_IMAGE_LAYOUT_TRANSITION_BIT, tag);  // Side effect notes below
        pending_layout_transition = false;
    }

    // Apply the accumulate execution barriers (and thus update chaining information)
    // for layout transition, read count is zeroed by SetWrite, so this will be skipped.
    for (uint32_t read_index = 0; read_index < last_read_count; read_index++) {
        ReadState &access = last_reads[read_index];
        access.barriers |= access.pending_dep_chain;
        read_execution_barriers |= access.barriers;
        access.pending_dep_chain = 0;
    }

    // We OR in the accumulated write chain and barriers even in the case of a layout transition as SetWrite zeros them.
    write_dependency_chain |= pending_write_dep_chain;
    write_barriers |= pending_write_barriers;
    pending_write_dep_chain = 0;
    pending_write_barriers = 0;
}

// This should be just Bits or Index, but we don't have an invalid state for Index
VkPipelineStageFlags ResourceAccessState::GetReadBarriers(SyncStageAccessFlags usage_bit) const {
    VkPipelineStageFlags barriers = 0U;

    for (uint32_t read_index = 0; read_index < last_read_count; read_index++) {
        const auto &read_access = last_reads[read_index];
        if (read_access.access & usage_bit) {
            barriers = read_access.barriers;
            break;
        }
    }

    return barriers;
}

inline bool ResourceAccessState::IsRAWHazard(VkPipelineStageFlagBits usage_stage, SyncStageAccessFlagBits usage) const {
    assert(IsRead(usage));
    // Only RAW vs. last_write if it doesn't happen-after any other read because either:
    //    * the previous reads are not hazards, and thus last_write must be visible and available to
    //      any reads that happen after.
    //    * the previous reads *are* hazards to last_write, have been reported, and if that hazard is fixed
    //      the current read will be also not be a hazard, thus reporting a hazard here adds no needed information.
    return (0 != last_write) && (0 == (read_execution_barriers & usage_stage)) && IsWriteHazard(usage);
}

VkPipelineStageFlags ResourceAccessState::GetOrderedStages(const SyncOrderingBarrier &ordering) const {
    // Whether the stage are in the ordering scope only matters if the current write is ordered
    VkPipelineStageFlags ordered_stages = last_read_stages & ordering.exec_scope;
    // Special input attachment handling as always (not encoded in exec_scop)
    const bool input_attachment_ordering = 0 != (ordering.access_scope & SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ_BIT);
    if (input_attachment_ordering && input_attachment_read) {
        // If we have an input attachment in last_reads and input attachments are ordered we all that stage
        ordered_stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    return ordered_stages;
}

inline ResourceAccessState::ReadState *ResourceAccessState::GetReadStateForStage(VkPipelineStageFlagBits stage,
                                                                                 uint32_t search_limit) {
    ReadState *read_state = nullptr;
    search_limit = std::min(search_limit, last_read_count);
    for (uint32_t i = 0; i < search_limit; i++) {
        if (last_reads[i].stage == stage) {
            read_state = &last_reads[i];
            break;
        }
    }
    return read_state;
}

void SyncValidator::ResetCommandBufferCallback(VkCommandBuffer command_buffer) {
    auto *access_context = GetAccessContextNoInsert(command_buffer);
    if (access_context) {
        access_context->Reset();
    }
}

void SyncValidator::FreeCommandBufferCallback(VkCommandBuffer command_buffer) {
    auto access_found = cb_access_state.find(command_buffer);
    if (access_found != cb_access_state.end()) {
        access_found->second->Reset();
        cb_access_state.erase(access_found);
    }
}

void SyncValidator::ApplyGlobalBarriers(AccessContext *context, VkPipelineStageFlags src_exec_scope,
                                        VkPipelineStageFlags dst_exec_scope, SyncStageAccessFlags src_access_scope,
                                        SyncStageAccessFlags dst_access_scope, uint32_t memory_barrier_count,
                                        const VkMemoryBarrier *pMemoryBarriers, const ResourceUsageTag &tag) {
    ApplyBarrierOpsFunctor barriers_functor(true /* resolve */, std::min<uint32_t>(1, memory_barrier_count), tag);
    for (uint32_t barrier_index = 0; barrier_index < memory_barrier_count; barrier_index++) {
        const auto &barrier = pMemoryBarriers[barrier_index];
        SyncBarrier sync_barrier(src_exec_scope, SyncStageAccess::AccessScope(src_access_scope, barrier.srcAccessMask),
                                 dst_exec_scope, SyncStageAccess::AccessScope(dst_access_scope, barrier.dstAccessMask));
        barriers_functor.PushBack(sync_barrier, false);
    }
    if (0 == memory_barrier_count) {
        // If there are no global memory barriers, force an exec barrier
        barriers_functor.PushBack(SyncBarrier(src_exec_scope, 0, dst_exec_scope, 0), false);
    }
    context->ApplyGlobalBarriers(barriers_functor);
}

void SyncValidator::ApplyBufferBarriers(AccessContext *context, VkPipelineStageFlags src_exec_scope,
                                        SyncStageAccessFlags src_stage_accesses, VkPipelineStageFlags dst_exec_scope,
                                        SyncStageAccessFlags dst_stage_accesses, uint32_t barrier_count,
                                        const VkBufferMemoryBarrier *barriers) {
    for (uint32_t index = 0; index < barrier_count; index++) {
        auto barrier = barriers[index];  // barrier is a copy
        const auto *buffer = Get<BUFFER_STATE>(barrier.buffer);
        if (!buffer) continue;
        barrier.size = GetBufferWholeSize(*buffer, barrier.offset, barrier.size);
        const ResourceAccessRange range = MakeRange(barrier);
        const auto src_access_scope = AccessScope(src_stage_accesses, barrier.srcAccessMask);
        const auto dst_access_scope = AccessScope(dst_stage_accesses, barrier.dstAccessMask);
        const SyncBarrier sync_barrier(src_exec_scope, src_access_scope, dst_exec_scope, dst_access_scope);
        const ApplyBarrierFunctor update_action(sync_barrier, false /* layout_transition */);
        context->UpdateResourceAccess(*buffer, range, update_action);
    }
}

void SyncValidator::ApplyImageBarriers(AccessContext *context, VkPipelineStageFlags src_exec_scope,
                                       SyncStageAccessFlags src_stage_accesses, VkPipelineStageFlags dst_exec_scope,
                                       SyncStageAccessFlags dst_stage_accesses, uint32_t barrier_count,
                                       const VkImageMemoryBarrier *barriers, const ResourceUsageTag &tag) {
    for (uint32_t index = 0; index < barrier_count; index++) {
        const auto &barrier = barriers[index];
        const auto *image = Get<IMAGE_STATE>(barrier.image);
        if (!image) continue;
        auto subresource_range = NormalizeSubresourceRange(image->createInfo, barrier.subresourceRange);
        bool layout_transition = barrier.oldLayout != barrier.newLayout;
        const auto src_access_scope = AccessScope(src_stage_accesses, barrier.srcAccessMask);
        const auto dst_access_scope = AccessScope(dst_stage_accesses, barrier.dstAccessMask);
        const SyncBarrier sync_barrier(src_exec_scope, src_access_scope, dst_exec_scope, dst_access_scope);
        const ApplyBarrierFunctor barrier_action(sync_barrier, layout_transition);
        context->UpdateResourceAccess(*image, subresource_range, barrier_action);
    }
}

bool SyncValidator::PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                                 uint32_t regionCount, const VkBufferCopy *pRegions) const {
    bool skip = false;
    const auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return skip;
    const auto *context = cb_context->GetCurrentAccessContext();

    // If we have no previous accesses, we have no hazards
    const auto *src_buffer = Get<BUFFER_STATE>(srcBuffer);
    const auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_buffer) {
            const ResourceAccessRange src_range = MakeRange(*src_buffer, copy_region.srcOffset, copy_region.size);
            auto hazard = context->DetectHazard(*src_buffer, SYNC_TRANSFER_TRANSFER_READ, src_range);
            if (hazard.hazard) {
                // TODO -- add tag information to log msg when useful.
                skip |= LogError(srcBuffer, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdCopyBuffer: Hazard %s for srcBuffer %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(srcBuffer).c_str(), region,
                                 string_UsageTag(hazard).c_str());
            }
        }
        if (dst_buffer && !skip) {
            const ResourceAccessRange dst_range = MakeRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            auto hazard = context->DetectHazard(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, dst_range);
            if (hazard.hazard) {
                skip |= LogError(dstBuffer, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdCopyBuffer: Hazard %s for dstBuffer %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(dstBuffer).c_str(), region,
                                 string_UsageTag(hazard).c_str());
            }
        }
        if (skip) break;
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                               uint32_t regionCount, const VkBufferCopy *pRegions) {
    auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    const auto tag = cb_context->NextCommandTag(CMD_COPYBUFFER);
    auto *context = cb_context->GetCurrentAccessContext();

    const auto *src_buffer = Get<BUFFER_STATE>(srcBuffer);
    const auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_buffer) {
            const ResourceAccessRange src_range = MakeRange(*src_buffer, copy_region.srcOffset, copy_region.size);
            context->UpdateAccessState(*src_buffer, SYNC_TRANSFER_TRANSFER_READ, src_range, tag);
        }
        if (dst_buffer) {
            const ResourceAccessRange dst_range = MakeRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            context->UpdateAccessState(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, dst_range, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer,
                                                     const VkCopyBufferInfo2KHR *pCopyBufferInfos) const {
    bool skip = false;
    const auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return skip;
    const auto *context = cb_context->GetCurrentAccessContext();

    // If we have no previous accesses, we have no hazards
    const auto *src_buffer = Get<BUFFER_STATE>(pCopyBufferInfos->srcBuffer);
    const auto *dst_buffer = Get<BUFFER_STATE>(pCopyBufferInfos->dstBuffer);

    for (uint32_t region = 0; region < pCopyBufferInfos->regionCount; region++) {
        const auto &copy_region = pCopyBufferInfos->pRegions[region];
        if (src_buffer) {
            const ResourceAccessRange src_range = MakeRange(*src_buffer, copy_region.srcOffset, copy_region.size);
            auto hazard = context->DetectHazard(*src_buffer, SYNC_TRANSFER_TRANSFER_READ, src_range);
            if (hazard.hazard) {
                // TODO -- add tag information to log msg when useful.
                skip |= LogError(pCopyBufferInfos->srcBuffer, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdCopyBuffer2KHR(): Hazard %s for srcBuffer %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(pCopyBufferInfos->srcBuffer).c_str(),
                                 region, string_UsageTag(hazard).c_str());
            }
        }
        if (dst_buffer && !skip) {
            const ResourceAccessRange dst_range = MakeRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            auto hazard = context->DetectHazard(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, dst_range);
            if (hazard.hazard) {
                skip |= LogError(pCopyBufferInfos->dstBuffer, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdCopyBuffer2KHR(): Hazard %s for dstBuffer %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(pCopyBufferInfos->dstBuffer).c_str(),
                                 region, string_UsageTag(hazard).c_str());
            }
        }
        if (skip) break;
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfos) {
    auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    const auto tag = cb_context->NextCommandTag(CMD_COPYBUFFER2KHR);
    auto *context = cb_context->GetCurrentAccessContext();

    const auto *src_buffer = Get<BUFFER_STATE>(pCopyBufferInfos->srcBuffer);
    const auto *dst_buffer = Get<BUFFER_STATE>(pCopyBufferInfos->dstBuffer);

    for (uint32_t region = 0; region < pCopyBufferInfos->regionCount; region++) {
        const auto &copy_region = pCopyBufferInfos->pRegions[region];
        if (src_buffer) {
            const ResourceAccessRange src_range = MakeRange(*src_buffer, copy_region.srcOffset, copy_region.size);
            context->UpdateAccessState(*src_buffer, SYNC_TRANSFER_TRANSFER_READ, src_range, tag);
        }
        if (dst_buffer) {
            const ResourceAccessRange dst_range = MakeRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            context->UpdateAccessState(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, dst_range, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkImageCopy *pRegions) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *src_image = Get<IMAGE_STATE>(srcImage);
    const auto *dst_image = Get<IMAGE_STATE>(dstImage);
    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, SYNC_TRANSFER_TRANSFER_READ, copy_region.srcSubresource,
                                                copy_region.srcOffset, copy_region.extent);
            if (hazard.hazard) {
                skip |= LogError(srcImage, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdCopyImage: Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(srcImage).c_str(), region,
                                 string_UsageTag(hazard).c_str());
            }
        }

        if (dst_image) {
            VkExtent3D dst_copy_extent =
                GetAdjustedDestImageExtent(src_image->createInfo.format, dst_image->createInfo.format, copy_region.extent);
            auto hazard = context->DetectHazard(*dst_image, SYNC_TRANSFER_TRANSFER_WRITE, copy_region.dstSubresource,
                                                copy_region.dstOffset, dst_copy_extent);
            if (hazard.hazard) {
                skip |= LogError(dstImage, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdCopyImage: Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(dstImage).c_str(), region,
                                 string_UsageTag(hazard).c_str());
            }
            if (skip) break;
        }
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                              const VkImageCopy *pRegions) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_COPYIMAGE);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto *src_image = Get<IMAGE_STATE>(srcImage);
    auto *dst_image = Get<IMAGE_STATE>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_TRANSFER_TRANSFER_READ, copy_region.srcSubresource, copy_region.srcOffset,
                                       copy_region.extent, tag);
        }
        if (dst_image) {
            VkExtent3D dst_copy_extent =
                GetAdjustedDestImageExtent(src_image->createInfo.format, dst_image->createInfo.format, copy_region.extent);
            context->UpdateAccessState(*dst_image, SYNC_TRANSFER_TRANSFER_WRITE, copy_region.dstSubresource, copy_region.dstOffset,
                                       dst_copy_extent, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer,
                                                    const VkCopyImageInfo2KHR *pCopyImageInfo) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *src_image = Get<IMAGE_STATE>(pCopyImageInfo->srcImage);
    const auto *dst_image = Get<IMAGE_STATE>(pCopyImageInfo->dstImage);
    for (uint32_t region = 0; region < pCopyImageInfo->regionCount; region++) {
        const auto &copy_region = pCopyImageInfo->pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, SYNC_TRANSFER_TRANSFER_READ, copy_region.srcSubresource,
                                                copy_region.srcOffset, copy_region.extent);
            if (hazard.hazard) {
                skip |= LogError(pCopyImageInfo->srcImage, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdCopyImage2KHR: Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(pCopyImageInfo->srcImage).c_str(),
                                 region, string_UsageTag(hazard).c_str());
            }
        }

        if (dst_image) {
            VkExtent3D dst_copy_extent =
                GetAdjustedDestImageExtent(src_image->createInfo.format, dst_image->createInfo.format, copy_region.extent);
            auto hazard = context->DetectHazard(*dst_image, SYNC_TRANSFER_TRANSFER_WRITE, copy_region.dstSubresource,
                                                copy_region.dstOffset, dst_copy_extent);
            if (hazard.hazard) {
                skip |= LogError(pCopyImageInfo->dstImage, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdCopyImage2KHR: Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(pCopyImageInfo->dstImage).c_str(),
                                 region, string_UsageTag(hazard).c_str());
            }
            if (skip) break;
        }
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_COPYIMAGE2KHR);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto *src_image = Get<IMAGE_STATE>(pCopyImageInfo->srcImage);
    auto *dst_image = Get<IMAGE_STATE>(pCopyImageInfo->dstImage);

    for (uint32_t region = 0; region < pCopyImageInfo->regionCount; region++) {
        const auto &copy_region = pCopyImageInfo->pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_TRANSFER_TRANSFER_READ, copy_region.srcSubresource, copy_region.srcOffset,
                                       copy_region.extent, tag);
        }
        if (dst_image) {
            VkExtent3D dst_copy_extent =
                GetAdjustedDestImageExtent(src_image->createInfo.format, dst_image->createInfo.format, copy_region.extent);
            context->UpdateAccessState(*dst_image, SYNC_TRANSFER_TRANSFER_WRITE, copy_region.dstSubresource, copy_region.dstOffset,
                                       dst_copy_extent, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                      VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                      uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                      uint32_t bufferMemoryBarrierCount,
                                                      const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                      uint32_t imageMemoryBarrierCount,
                                                      const VkImageMemoryBarrier *pImageMemoryBarriers) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto src_stage_mask = ExpandPipelineStages(cb_access_context->GetQueueFlags(), srcStageMask);
    const auto src_exec_scope = WithEarlierPipelineStages(src_stage_mask);
    auto src_stage_accesses = AccessScopeByStage(src_stage_mask);
    // Validate Image Layout transitions
    for (uint32_t index = 0; index < imageMemoryBarrierCount; index++) {
        const auto &barrier = pImageMemoryBarriers[index];
        if (barrier.newLayout == barrier.oldLayout) continue;  // Only interested in layout transitions at this point.
        const auto *image_state = Get<IMAGE_STATE>(barrier.image);
        if (!image_state) continue;
        const auto hazard = context->DetectImageBarrierHazard(*image_state, src_exec_scope, src_stage_accesses, barrier);
        if (hazard.hazard) {
            // PHASE1 TODO -- add tag information to log msg when useful.
            skip |= LogError(barrier.image, string_SyncHazardVUID(hazard.hazard),
                             "vkCmdPipelineBarrier: Hazard %s for image barrier %" PRIu32 " %s. Access info %s.",
                             string_SyncHazard(hazard.hazard), index, report_data->FormatHandle(barrier.image).c_str(),
                             string_UsageTag(hazard).c_str());
        }
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                    VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                    uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                    uint32_t bufferMemoryBarrierCount,
                                                    const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                    uint32_t imageMemoryBarrierCount,
                                                    const VkImageMemoryBarrier *pImageMemoryBarriers) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return;
    const auto tag = cb_access_context->NextCommandTag(CMD_PIPELINEBARRIER);
    auto access_context = cb_access_context->GetCurrentAccessContext();
    assert(access_context);
    if (!access_context) return;

    const auto src_stage_mask = ExpandPipelineStages(cb_access_context->GetQueueFlags(), srcStageMask);
    auto src_stage_accesses = AccessScopeByStage(src_stage_mask);
    const auto dst_stage_mask = ExpandPipelineStages(cb_access_context->GetQueueFlags(), dstStageMask);
    auto dst_stage_accesses = AccessScopeByStage(dst_stage_mask);
    const auto src_exec_scope = WithEarlierPipelineStages(src_stage_mask);
    const auto dst_exec_scope = WithLaterPipelineStages(dst_stage_mask);

    // These two apply barriers one at a time as the are restricted to the resource ranges specified per each barrier,
    // but do not update the dependency chain information (but set the "pending" state) // s.t. the order independence
    // of the barriers is maintained.
    ApplyBufferBarriers(access_context, src_exec_scope, src_stage_accesses, dst_exec_scope, dst_stage_accesses,
                        bufferMemoryBarrierCount, pBufferMemoryBarriers);
    ApplyImageBarriers(access_context, src_exec_scope, src_stage_accesses, dst_exec_scope, dst_stage_accesses,
                       imageMemoryBarrierCount, pImageMemoryBarriers, tag);

    // Apply the global barriers last as is it walks all memory, it can also clean up the "pending" state without requiring an
    // additional pass, updating the dependency chains *last* as it goes along.
    // This is needed to guarantee order independence of the three lists.
    ApplyGlobalBarriers(access_context, src_exec_scope, dst_exec_scope, src_stage_accesses, dst_stage_accesses, memoryBarrierCount,
                        pMemoryBarriers, tag);
}

void SyncValidator::PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkDevice *pDevice, VkResult result) {
    // The state tracker sets up the device state
    StateTracker::PostCallRecordCreateDevice(gpu, pCreateInfo, pAllocator, pDevice, result);

    // Add the callback hooks for the functions that are either broadly or deeply used and that the ValidationStateTracker
    // refactor would be messier without.
    // TODO: Find a good way to do this hooklessly.
    ValidationObject *device_object = GetLayerDataPtr(get_dispatch_key(*pDevice), layer_data_map);
    ValidationObject *validation_data = GetValidationObject(device_object->object_dispatch, LayerObjectTypeSyncValidation);
    SyncValidator *sync_device_state = static_cast<SyncValidator *>(validation_data);

    sync_device_state->SetCommandBufferResetCallback([sync_device_state](VkCommandBuffer command_buffer) -> void {
        sync_device_state->ResetCommandBufferCallback(command_buffer);
    });
    sync_device_state->SetCommandBufferFreeCallback([sync_device_state](VkCommandBuffer command_buffer) -> void {
        sync_device_state->FreeCommandBufferCallback(command_buffer);
    });
}

bool SyncValidator::ValidateBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                            const VkSubpassBeginInfoKHR *pSubpassBeginInfo, const char *func_name) const {
    bool skip = false;
    const auto rp_state = Get<RENDER_PASS_STATE>(pRenderPassBegin->renderPass);
    auto cb_context = GetAccessContext(commandBuffer);

    if (rp_state && cb_context) {
        skip |= cb_context->ValidateBeginRenderPass(*rp_state, pRenderPassBegin, pSubpassBeginInfo, func_name);
    }

    return skip;
}

bool SyncValidator::PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                      VkSubpassContents contents) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    auto subpass_begin_info = lvl_init_struct<VkSubpassBeginInfo>();
    subpass_begin_info.contents = contents;
    skip |= ValidateBeginRenderPass(commandBuffer, pRenderPassBegin, &subpass_begin_info, "vkCmdBeginRenderPass");
    return skip;
}

bool SyncValidator::PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                       const VkSubpassBeginInfoKHR *pSubpassBeginInfo) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    skip |= ValidateBeginRenderPass(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, "vkCmdBeginRenderPass2");
    return skip;
}

bool SyncValidator::PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                          const VkRenderPassBeginInfo *pRenderPassBegin,
                                                          const VkSubpassBeginInfoKHR *pSubpassBeginInfo) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    skip |= ValidateBeginRenderPass(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, "vkCmdBeginRenderPass2KHR");
    return skip;
}

void SyncValidator::PostCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo,
                                                     VkResult result) {
    // The state tracker sets up the command buffer state
    StateTracker::PostCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo, result);

    // Create/initialize the structure that trackers accesses at the command buffer scope.
    auto cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    cb_access_context->Reset();
}

void SyncValidator::RecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                             const VkSubpassBeginInfo *pSubpassBeginInfo, CMD_TYPE command) {
    auto cb_context = GetAccessContext(commandBuffer);
    if (cb_context) {
        cb_context->RecordBeginRenderPass(cb_context->NextCommandTag(command));
    }
}

void SyncValidator::PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                     VkSubpassContents contents) {
    StateTracker::PostCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    auto subpass_begin_info = lvl_init_struct<VkSubpassBeginInfo>();
    subpass_begin_info.contents = contents;
    RecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, &subpass_begin_info, CMD_BEGINRENDERPASS);
}

void SyncValidator::PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                      const VkSubpassBeginInfo *pSubpassBeginInfo) {
    StateTracker::PostCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    RecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, CMD_BEGINRENDERPASS2);
}

void SyncValidator::PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                         const VkRenderPassBeginInfo *pRenderPassBegin,
                                                         const VkSubpassBeginInfo *pSubpassBeginInfo) {
    StateTracker::PostCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    RecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, CMD_BEGINRENDERPASS2);
}

bool SyncValidator::ValidateCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR *pSubpassBeginInfo,
                                           const VkSubpassEndInfoKHR *pSubpassEndInfo, const char *func_name) const {
    bool skip = false;

    auto cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    auto cb_state = cb_context->GetCommandBufferState();
    if (!cb_state) return skip;

    auto rp_state = cb_state->activeRenderPass;
    if (!rp_state) return skip;

    skip |= cb_context->ValidateNextSubpass(func_name);

    return skip;
}

bool SyncValidator::PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) const {
    bool skip = StateTracker::PreCallValidateCmdNextSubpass(commandBuffer, contents);
    auto subpass_begin_info = lvl_init_struct<VkSubpassBeginInfo>();
    subpass_begin_info.contents = contents;
    skip |= ValidateCmdNextSubpass(commandBuffer, &subpass_begin_info, nullptr, "vkCmdNextSubpass");
    return skip;
}

bool SyncValidator::PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfoKHR *pSubpassBeginInfo,
                                                      const VkSubpassEndInfoKHR *pSubpassEndInfo) const {
    bool skip = StateTracker::PreCallValidateCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    skip |= ValidateCmdNextSubpass(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, "vkCmdNextSubpass2KHR");
    return skip;
}

bool SyncValidator::PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                   const VkSubpassEndInfo *pSubpassEndInfo) const {
    bool skip = StateTracker::PreCallValidateCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    skip |= ValidateCmdNextSubpass(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, "vkCmdNextSubpass2");
    return skip;
}

void SyncValidator::RecordCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                         const VkSubpassEndInfo *pSubpassEndInfo, CMD_TYPE command) {
    auto cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    auto cb_state = cb_context->GetCommandBufferState();
    if (!cb_state) return;

    auto rp_state = cb_state->activeRenderPass;
    if (!rp_state) return;

    cb_context->RecordNextSubpass(*rp_state, cb_context->NextCommandTag(command));
}

void SyncValidator::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    StateTracker::PostCallRecordCmdNextSubpass(commandBuffer, contents);
    auto subpass_begin_info = lvl_init_struct<VkSubpassBeginInfo>();
    subpass_begin_info.contents = contents;
    RecordCmdNextSubpass(commandBuffer, &subpass_begin_info, nullptr, CMD_NEXTSUBPASS);
}

void SyncValidator::PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                  const VkSubpassEndInfo *pSubpassEndInfo) {
    StateTracker::PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    RecordCmdNextSubpass(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, CMD_NEXTSUBPASS2);
}

void SyncValidator::PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                     const VkSubpassEndInfo *pSubpassEndInfo) {
    StateTracker::PostCallRecordCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    RecordCmdNextSubpass(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, CMD_NEXTSUBPASS2);
}

bool SyncValidator::ValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfoKHR *pSubpassEndInfo,
                                             const char *func_name) const {
    bool skip = false;

    auto cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    auto cb_state = cb_context->GetCommandBufferState();
    if (!cb_state) return skip;

    auto rp_state = cb_state->activeRenderPass;
    if (!rp_state) return skip;

    skip |= cb_context->ValidateEndRenderpass(func_name);
    return skip;
}

bool SyncValidator::PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer) const {
    bool skip = StateTracker::PreCallValidateCmdEndRenderPass(commandBuffer);
    skip |= ValidateCmdEndRenderPass(commandBuffer, nullptr, "vkEndRenderPass");
    return skip;
}

bool SyncValidator::PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer,
                                                     const VkSubpassEndInfoKHR *pSubpassEndInfo) const {
    bool skip = StateTracker::PreCallValidateCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
    skip |= ValidateCmdEndRenderPass(commandBuffer, pSubpassEndInfo, "vkEndRenderPass2");
    return skip;
}

bool SyncValidator::PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                        const VkSubpassEndInfoKHR *pSubpassEndInfo) const {
    bool skip = StateTracker::PreCallValidateCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
    skip |= ValidateCmdEndRenderPass(commandBuffer, pSubpassEndInfo, "vkEndRenderPass2KHR");
    return skip;
}

void SyncValidator::RecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                           CMD_TYPE command) {
    // Resolve the all subpass contexts to the command buffer contexts
    auto cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    auto cb_state = cb_context->GetCommandBufferState();
    if (!cb_state) return;

    const auto *rp_state = cb_state->activeRenderPass.get();
    if (!rp_state) return;

    cb_context->RecordEndRenderPass(*rp_state, cb_context->NextCommandTag(command));
}

// Simple heuristic rule to detect WAW operations representing algorithmically safe or increment
// updates to a resource which do not conflict at the byte level.
// TODO: Revisit this rule to see if it needs to be tighter or looser
// TODO: Add programatic control over suppression heuristics
bool SyncValidator::SupressedBoundDescriptorWAW(const HazardResult &hazard) const {
    return (hazard.hazard == WRITE_AFTER_WRITE) && (FlagBit(hazard.usage_index) == hazard.prior_access);
}

void SyncValidator::PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) {
    RecordCmdEndRenderPass(commandBuffer, nullptr, CMD_ENDRENDERPASS);
    StateTracker::PostCallRecordCmdEndRenderPass(commandBuffer);
}

void SyncValidator::PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo) {
    RecordCmdEndRenderPass(commandBuffer, pSubpassEndInfo, CMD_ENDRENDERPASS2);
    StateTracker::PostCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
}

void SyncValidator::PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo) {
    RecordCmdEndRenderPass(commandBuffer, pSubpassEndInfo, CMD_ENDRENDERPASS2);
    StateTracker::PostCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
}

template <typename BufferImageCopyRegionType>
bool SyncValidator::ValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                 VkImageLayout dstImageLayout, uint32_t regionCount,
                                                 const BufferImageCopyRegionType *pRegions, CopyCommandVersion version) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const bool is_2khr = (version == COPY_COMMAND_VERSION_2);
    const char *func_name = is_2khr ? "vkCmdCopyBufferToImage2KHR()" : "vkCmdCopyBufferToImage()";

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *src_buffer = Get<BUFFER_STATE>(srcBuffer);
    const auto *dst_image = Get<IMAGE_STATE>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_buffer) {
            ResourceAccessRange src_range =
                MakeRange(copy_region.bufferOffset, GetBufferSizeFromCopyImage(copy_region, dst_image->createInfo.format));
            auto hazard = context->DetectHazard(*src_buffer, SYNC_TRANSFER_TRANSFER_READ, src_range);
            if (hazard.hazard) {
                // PHASE1 TODO -- add tag information to log msg when useful.
                skip |= LogError(srcBuffer, string_SyncHazardVUID(hazard.hazard),
                                 "%s: Hazard %s for srcBuffer %s, region %" PRIu32 ". Access info %s.", func_name,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(srcBuffer).c_str(), region,
                                 string_UsageTag(hazard).c_str());
            }
        }
        if (dst_image) {
            auto hazard = context->DetectHazard(*dst_image, SYNC_TRANSFER_TRANSFER_WRITE, copy_region.imageSubresource,
                                                copy_region.imageOffset, copy_region.imageExtent);
            if (hazard.hazard) {
                skip |= LogError(dstImage, string_SyncHazardVUID(hazard.hazard),
                                 "%s: Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.", func_name,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(dstImage).c_str(), region,
                                 string_UsageTag(hazard).c_str());
            }
            if (skip) break;
        }
        if (skip) break;
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                        VkImageLayout dstImageLayout, uint32_t regionCount,
                                                        const VkBufferImageCopy *pRegions) const {
    return ValidateCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions,
                                        COPY_COMMAND_VERSION_1);
}

bool SyncValidator::PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                            const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo) const {
    return ValidateCmdCopyBufferToImage(commandBuffer, pCopyBufferToImageInfo->srcBuffer, pCopyBufferToImageInfo->dstImage,
                                        pCopyBufferToImageInfo->dstImageLayout, pCopyBufferToImageInfo->regionCount,
                                        pCopyBufferToImageInfo->pRegions, COPY_COMMAND_VERSION_2);
}

template <typename BufferImageCopyRegionType>
void SyncValidator::RecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                               VkImageLayout dstImageLayout, uint32_t regionCount,
                                               const BufferImageCopyRegionType *pRegions, CopyCommandVersion version) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);

    const bool is_2khr = (version == COPY_COMMAND_VERSION_2);
    const CMD_TYPE cmd_type = is_2khr ? CMD_COPYBUFFERTOIMAGE2KHR : CMD_COPYBUFFERTOIMAGE;

    const auto tag = cb_access_context->NextCommandTag(cmd_type);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    const auto *src_buffer = Get<BUFFER_STATE>(srcBuffer);
    const auto *dst_image = Get<IMAGE_STATE>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_buffer) {
            ResourceAccessRange src_range =
                MakeRange(copy_region.bufferOffset, GetBufferSizeFromCopyImage(copy_region, dst_image->createInfo.format));
            context->UpdateAccessState(*src_buffer, SYNC_TRANSFER_TRANSFER_READ, src_range, tag);
        }
        if (dst_image) {
            context->UpdateAccessState(*dst_image, SYNC_TRANSFER_TRANSFER_WRITE, copy_region.imageSubresource,
                                       copy_region.imageOffset, copy_region.imageExtent, tag);
        }
    }
}

void SyncValidator::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                      VkImageLayout dstImageLayout, uint32_t regionCount,
                                                      const VkBufferImageCopy *pRegions) {
    StateTracker::PreCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    RecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions, COPY_COMMAND_VERSION_1);
}

void SyncValidator::PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                          const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo) {
    StateTracker::PreCallRecordCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
    RecordCmdCopyBufferToImage(commandBuffer, pCopyBufferToImageInfo->srcBuffer, pCopyBufferToImageInfo->dstImage,
                               pCopyBufferToImageInfo->dstImageLayout, pCopyBufferToImageInfo->regionCount,
                               pCopyBufferToImageInfo->pRegions, COPY_COMMAND_VERSION_2);
}

template <typename BufferImageCopyRegionType>
bool SyncValidator::ValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                 VkBuffer dstBuffer, uint32_t regionCount,
                                                 const BufferImageCopyRegionType *pRegions, CopyCommandVersion version) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const bool is_2khr = (version == COPY_COMMAND_VERSION_2);
    const char *func_name = is_2khr ? "vkCmdCopyImageToBuffer2KHR()" : "vkCmdCopyImageToBuffer()";

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *src_image = Get<IMAGE_STATE>(srcImage);
    const auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);
    const auto dst_mem = (dst_buffer && !dst_buffer->sparse) ? dst_buffer->binding.mem_state->mem : VK_NULL_HANDLE;
    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, SYNC_TRANSFER_TRANSFER_READ, copy_region.imageSubresource,
                                                copy_region.imageOffset, copy_region.imageExtent);
            if (hazard.hazard) {
                skip |= LogError(srcImage, string_SyncHazardVUID(hazard.hazard),
                                 "%s: Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.", func_name,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(srcImage).c_str(), region,
                                 string_UsageTag(hazard).c_str());
            }
        }
        if (dst_mem) {
            ResourceAccessRange dst_range =
                MakeRange(copy_region.bufferOffset, GetBufferSizeFromCopyImage(copy_region, src_image->createInfo.format));
            auto hazard = context->DetectHazard(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, dst_range);
            if (hazard.hazard) {
                skip |= LogError(dstBuffer, string_SyncHazardVUID(hazard.hazard),
                                 "%s: Hazard %s for dstBuffer %s, region %" PRIu32 ". Access info %s.", func_name,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(dstBuffer).c_str(), region,
                                 string_UsageTag(hazard).c_str());
            }
        }
        if (skip) break;
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                        VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount,
                                                        const VkBufferImageCopy *pRegions) const {
    return ValidateCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions,
                                        COPY_COMMAND_VERSION_1);
}

bool SyncValidator::PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                            const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo) const {
    return ValidateCmdCopyImageToBuffer(commandBuffer, pCopyImageToBufferInfo->srcImage, pCopyImageToBufferInfo->srcImageLayout,
                                        pCopyImageToBufferInfo->dstBuffer, pCopyImageToBufferInfo->regionCount,
                                        pCopyImageToBufferInfo->pRegions, COPY_COMMAND_VERSION_2);
}

template <typename BufferImageCopyRegionType>
void SyncValidator::RecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                               VkBuffer dstBuffer, uint32_t regionCount, const BufferImageCopyRegionType *pRegions,
                                               CopyCommandVersion version) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);

    const bool is_2khr = (version == COPY_COMMAND_VERSION_2);
    const CMD_TYPE cmd_type = is_2khr ? CMD_COPYIMAGETOBUFFER2KHR : CMD_COPYIMAGETOBUFFER;

    const auto tag = cb_access_context->NextCommandTag(cmd_type);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    const auto *src_image = Get<IMAGE_STATE>(srcImage);
    auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);
    const auto dst_mem = (dst_buffer && !dst_buffer->sparse) ? dst_buffer->binding.mem_state->mem : VK_NULL_HANDLE;
    const VulkanTypedHandle dst_handle(dst_mem, kVulkanObjectTypeDeviceMemory);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_TRANSFER_TRANSFER_READ, copy_region.imageSubresource,
                                       copy_region.imageOffset, copy_region.imageExtent, tag);
        }
        if (dst_buffer) {
            ResourceAccessRange dst_range =
                MakeRange(copy_region.bufferOffset, GetBufferSizeFromCopyImage(copy_region, src_image->createInfo.format));
            context->UpdateAccessState(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, dst_range, tag);
        }
    }
}

void SyncValidator::PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                      VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions) {
    StateTracker::PreCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    RecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions, COPY_COMMAND_VERSION_1);
}

void SyncValidator::PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                          const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo) {
    StateTracker::PreCallRecordCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
    RecordCmdCopyImageToBuffer(commandBuffer, pCopyImageToBufferInfo->srcImage, pCopyImageToBufferInfo->srcImageLayout,
                               pCopyImageToBufferInfo->dstBuffer, pCopyImageToBufferInfo->regionCount,
                               pCopyImageToBufferInfo->pRegions, COPY_COMMAND_VERSION_2);
}

template <typename RegionType>
bool SyncValidator::ValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                         VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                         const RegionType *pRegions, VkFilter filter, const char *apiName) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *src_image = Get<IMAGE_STATE>(srcImage);
    const auto *dst_image = Get<IMAGE_STATE>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &blit_region = pRegions[region];
        if (src_image) {
            VkOffset3D offset = {std::min(blit_region.srcOffsets[0].x, blit_region.srcOffsets[1].x),
                                 std::min(blit_region.srcOffsets[0].y, blit_region.srcOffsets[1].y),
                                 std::min(blit_region.srcOffsets[0].z, blit_region.srcOffsets[1].z)};
            VkExtent3D extent = {static_cast<uint32_t>(abs(blit_region.srcOffsets[1].x - blit_region.srcOffsets[0].x)),
                                 static_cast<uint32_t>(abs(blit_region.srcOffsets[1].y - blit_region.srcOffsets[0].y)),
                                 static_cast<uint32_t>(abs(blit_region.srcOffsets[1].z - blit_region.srcOffsets[0].z))};
            auto hazard =
                context->DetectHazard(*src_image, SYNC_TRANSFER_TRANSFER_READ, blit_region.srcSubresource, offset, extent);
            if (hazard.hazard) {
                skip |= LogError(srcImage, string_SyncHazardVUID(hazard.hazard),
                                 "%s: Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.", apiName,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(srcImage).c_str(), region,
                                 string_UsageTag(hazard).c_str());
            }
        }

        if (dst_image) {
            VkOffset3D offset = {std::min(blit_region.dstOffsets[0].x, blit_region.dstOffsets[1].x),
                                 std::min(blit_region.dstOffsets[0].y, blit_region.dstOffsets[1].y),
                                 std::min(blit_region.dstOffsets[0].z, blit_region.dstOffsets[1].z)};
            VkExtent3D extent = {static_cast<uint32_t>(abs(blit_region.dstOffsets[1].x - blit_region.dstOffsets[0].x)),
                                 static_cast<uint32_t>(abs(blit_region.dstOffsets[1].y - blit_region.dstOffsets[0].y)),
                                 static_cast<uint32_t>(abs(blit_region.dstOffsets[1].z - blit_region.dstOffsets[0].z))};
            auto hazard =
                context->DetectHazard(*dst_image, SYNC_TRANSFER_TRANSFER_WRITE, blit_region.dstSubresource, offset, extent);
            if (hazard.hazard) {
                skip |= LogError(dstImage, string_SyncHazardVUID(hazard.hazard),
                                 "%s: Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.", apiName,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(dstImage).c_str(), region,
                                 string_UsageTag(hazard).c_str());
            }
            if (skip) break;
        }
    }

    return skip;
}

bool SyncValidator::PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkImageBlit *pRegions, VkFilter filter) const {
    return ValidateCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter,
                                "vkCmdBlitImage");
}

bool SyncValidator::PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer,
                                                    const VkBlitImageInfo2KHR *pBlitImageInfo) const {
    return ValidateCmdBlitImage(commandBuffer, pBlitImageInfo->srcImage, pBlitImageInfo->srcImageLayout, pBlitImageInfo->dstImage,
                                pBlitImageInfo->dstImageLayout, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
                                pBlitImageInfo->filter, "vkCmdBlitImage2KHR");
}

template <typename RegionType>
void SyncValidator::RecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                       VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                       const RegionType *pRegions, VkFilter filter, ResourceUsageTag tag) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto *src_image = Get<IMAGE_STATE>(srcImage);
    auto *dst_image = Get<IMAGE_STATE>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &blit_region = pRegions[region];
        if (src_image) {
            VkOffset3D offset = {std::min(blit_region.srcOffsets[0].x, blit_region.srcOffsets[1].x),
                                 std::min(blit_region.srcOffsets[0].y, blit_region.srcOffsets[1].y),
                                 std::min(blit_region.srcOffsets[0].z, blit_region.srcOffsets[1].z)};
            VkExtent3D extent = {static_cast<uint32_t>(abs(blit_region.srcOffsets[1].x - blit_region.srcOffsets[0].x)),
                                 static_cast<uint32_t>(abs(blit_region.srcOffsets[1].y - blit_region.srcOffsets[0].y)),
                                 static_cast<uint32_t>(abs(blit_region.srcOffsets[1].z - blit_region.srcOffsets[0].z))};
            context->UpdateAccessState(*src_image, SYNC_TRANSFER_TRANSFER_READ, blit_region.srcSubresource, offset, extent, tag);
        }
        if (dst_image) {
            VkOffset3D offset = {std::min(blit_region.dstOffsets[0].x, blit_region.dstOffsets[1].x),
                                 std::min(blit_region.dstOffsets[0].y, blit_region.dstOffsets[1].y),
                                 std::min(blit_region.dstOffsets[0].z, blit_region.dstOffsets[1].z)};
            VkExtent3D extent = {static_cast<uint32_t>(abs(blit_region.dstOffsets[1].x - blit_region.dstOffsets[0].x)),
                                 static_cast<uint32_t>(abs(blit_region.dstOffsets[1].y - blit_region.dstOffsets[0].y)),
                                 static_cast<uint32_t>(abs(blit_region.dstOffsets[1].z - blit_region.dstOffsets[0].z))};
            context->UpdateAccessState(*dst_image, SYNC_TRANSFER_TRANSFER_WRITE, blit_region.dstSubresource, offset, extent, tag);
        }
    }
}

void SyncValidator::PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                              const VkImageBlit *pRegions, VkFilter filter) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_BLITIMAGE);
    StateTracker::PreCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                            pRegions, filter);
    RecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter, tag);
}

void SyncValidator::PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo) {
    StateTracker::PreCallRecordCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_BLITIMAGE2KHR);
    RecordCmdBlitImage(commandBuffer, pBlitImageInfo->srcImage, pBlitImageInfo->srcImageLayout, pBlitImageInfo->dstImage,
                       pBlitImageInfo->dstImageLayout, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
                       pBlitImageInfo->filter, tag);
}

bool SyncValidator::ValidateIndirectBuffer(const AccessContext &context, VkCommandBuffer commandBuffer,
                                           const VkDeviceSize struct_size, const VkBuffer buffer, const VkDeviceSize offset,
                                           const uint32_t drawCount, const uint32_t stride, const char *function) const {
    bool skip = false;
    if (drawCount == 0) return skip;

    const auto *buf_state = Get<BUFFER_STATE>(buffer);
    VkDeviceSize size = struct_size;
    if (drawCount == 1 || stride == size) {
        if (drawCount > 1) size *= drawCount;
        const ResourceAccessRange range = MakeRange(offset, size);
        auto hazard = context.DetectHazard(*buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, range);
        if (hazard.hazard) {
            skip |= LogError(buf_state->buffer, string_SyncHazardVUID(hazard.hazard),
                             "%s: Hazard %s for indirect %s in %s. Access info %s.", function, string_SyncHazard(hazard.hazard),
                             report_data->FormatHandle(buffer).c_str(), report_data->FormatHandle(commandBuffer).c_str(),
                             string_UsageTag(hazard).c_str());
        }
    } else {
        for (uint32_t i = 0; i < drawCount; ++i) {
            const ResourceAccessRange range = MakeRange(offset + i * stride, size);
            auto hazard = context.DetectHazard(*buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, range);
            if (hazard.hazard) {
                skip |= LogError(buf_state->buffer, string_SyncHazardVUID(hazard.hazard),
                                 "%s: Hazard %s for indirect %s in %s. Access info %s.", function, string_SyncHazard(hazard.hazard),
                                 report_data->FormatHandle(buffer).c_str(), report_data->FormatHandle(commandBuffer).c_str(),
                                 string_UsageTag(hazard).c_str());
                break;
            }
        }
    }
    return skip;
}

void SyncValidator::RecordIndirectBuffer(AccessContext &context, const ResourceUsageTag &tag, const VkDeviceSize struct_size,
                                         const VkBuffer buffer, const VkDeviceSize offset, const uint32_t drawCount,
                                         uint32_t stride) {
    const auto *buf_state = Get<BUFFER_STATE>(buffer);
    VkDeviceSize size = struct_size;
    if (drawCount == 1 || stride == size) {
        if (drawCount > 1) size *= drawCount;
        const ResourceAccessRange range = MakeRange(offset, size);
        context.UpdateAccessState(*buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, range, tag);
    } else {
        for (uint32_t i = 0; i < drawCount; ++i) {
            const ResourceAccessRange range = MakeRange(offset + i * stride, size);
            context.UpdateAccessState(*buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, range, tag);
        }
    }
}

bool SyncValidator::ValidateCountBuffer(const AccessContext &context, VkCommandBuffer commandBuffer, VkBuffer buffer,
                                        VkDeviceSize offset, const char *function) const {
    bool skip = false;

    const auto *count_buf_state = Get<BUFFER_STATE>(buffer);
    const ResourceAccessRange range = MakeRange(offset, 4);
    auto hazard = context.DetectHazard(*count_buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, range);
    if (hazard.hazard) {
        skip |= LogError(count_buf_state->buffer, string_SyncHazardVUID(hazard.hazard),
                         "%s: Hazard %s for countBuffer %s in %s. Access info %s.", function, string_SyncHazard(hazard.hazard),
                         report_data->FormatHandle(buffer).c_str(), report_data->FormatHandle(commandBuffer).c_str(),
                         string_UsageTag(hazard).c_str());
    }
    return skip;
}

void SyncValidator::RecordCountBuffer(AccessContext &context, const ResourceUsageTag &tag, VkBuffer buffer, VkDeviceSize offset) {
    const auto *count_buf_state = Get<BUFFER_STATE>(buffer);
    const ResourceAccessRange range = MakeRange(offset, 4);
    context.UpdateAccessState(*count_buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, range, tag);
}

bool SyncValidator::PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, "vkCmdDispatch");
    return skip;
}

void SyncValidator::PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) {
    StateTracker::PreCallRecordCmdDispatch(commandBuffer, x, y, z);
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_DISPATCH);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, tag);
}

bool SyncValidator::PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, "vkCmdDispatchIndirect");
    skip |= ValidateIndirectBuffer(*context, commandBuffer, sizeof(VkDispatchIndirectCommand), buffer, offset, 1,
                                   sizeof(VkDispatchIndirectCommand), "vkCmdDispatchIndirect");
    return skip;
}

void SyncValidator::PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    StateTracker::PreCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset);
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_DISPATCHINDIRECT);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, tag);
    RecordIndirectBuffer(*context, tag, sizeof(VkDispatchIndirectCommand), buffer, offset, 1, sizeof(VkDispatchIndirectCommand));
}

bool SyncValidator::PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                           uint32_t firstVertex, uint32_t firstInstance) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, "vkCmdDraw");
    skip |= cb_access_context->ValidateDrawVertex(vertexCount, firstVertex, "vkCmdDraw");
    skip |= cb_access_context->ValidateDrawSubpassAttachment("vkCmdDraw");
    return skip;
}

void SyncValidator::PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                         uint32_t firstVertex, uint32_t firstInstance) {
    StateTracker::PreCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_DRAW);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawVertex(vertexCount, firstVertex, tag);
    cb_access_context->RecordDrawSubpassAttachment(tag);
}

bool SyncValidator::PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                  uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, "vkCmdDrawIndexed");
    skip |= cb_access_context->ValidateDrawVertexIndex(indexCount, firstIndex, "vkCmdDrawIndexed");
    skip |= cb_access_context->ValidateDrawSubpassAttachment("vkCmdDrawIndexed");
    return skip;
}

void SyncValidator::PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    StateTracker::PreCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_DRAWINDEXED);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawVertexIndex(indexCount, firstIndex, tag);
    cb_access_context->RecordDrawSubpassAttachment(tag);
}

bool SyncValidator::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   uint32_t drawCount, uint32_t stride) const {
    bool skip = false;
    if (drawCount == 0) return skip;

    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, "vkCmdDrawIndirect");
    skip |= cb_access_context->ValidateDrawSubpassAttachment("vkCmdDrawIndirect");
    skip |= ValidateIndirectBuffer(*context, commandBuffer, sizeof(VkDrawIndirectCommand), buffer, offset, drawCount, stride,
                                   "vkCmdDrawIndirect");

    // TODO: For now, we validate the whole vertex buffer. It might cause some false positive.
    //       VkDrawIndirectCommand buffer could be changed until SubmitQueue.
    //       We will validate the vertex buffer in SubmitQueue in the future.
    skip |= cb_access_context->ValidateDrawVertex(UINT32_MAX, 0, "vkCmdDrawIndirect");
    return skip;
}

void SyncValidator::PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride) {
    StateTracker::PreCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    if (drawCount == 0) return;
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_DRAWINDIRECT);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawSubpassAttachment(tag);
    RecordIndirectBuffer(*context, tag, sizeof(VkDrawIndirectCommand), buffer, offset, drawCount, stride);

    // TODO: For now, we record the whole vertex buffer. It might cause some false positive.
    //       VkDrawIndirectCommand buffer could be changed until SubmitQueue.
    //       We will record the vertex buffer in SubmitQueue in the future.
    cb_access_context->RecordDrawVertex(UINT32_MAX, 0, tag);
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          uint32_t drawCount, uint32_t stride) const {
    bool skip = false;
    if (drawCount == 0) return skip;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, "vkCmdDrawIndexedIndirect");
    skip |= cb_access_context->ValidateDrawSubpassAttachment("vkCmdDrawIndexedIndirect");
    skip |= ValidateIndirectBuffer(*context, commandBuffer, sizeof(VkDrawIndexedIndirectCommand), buffer, offset, drawCount, stride,
                                   "vkCmdDrawIndexedIndirect");

    // TODO: For now, we validate the whole index and vertex buffer. It might cause some false positive.
    //       VkDrawIndexedIndirectCommand buffer could be changed until SubmitQueue.
    //       We will validate the index and vertex buffer in SubmitQueue in the future.
    skip |= cb_access_context->ValidateDrawVertexIndex(UINT32_MAX, 0, "vkCmdDrawIndexedIndirect");
    return skip;
}

void SyncValidator::PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        uint32_t drawCount, uint32_t stride) {
    StateTracker::PreCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_DRAWINDEXEDINDIRECT);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawSubpassAttachment(tag);
    RecordIndirectBuffer(*context, tag, sizeof(VkDrawIndexedIndirectCommand), buffer, offset, drawCount, stride);

    // TODO: For now, we record the whole index and vertex buffer. It might cause some false positive.
    //       VkDrawIndexedIndirectCommand buffer could be changed until SubmitQueue.
    //       We will record the index and vertex buffer in SubmitQueue in the future.
    cb_access_context->RecordDrawVertexIndex(UINT32_MAX, 0, tag);
}

bool SyncValidator::ValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                 uint32_t stride, const char *function) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, function);
    skip |= cb_access_context->ValidateDrawSubpassAttachment(function);
    skip |= ValidateIndirectBuffer(*context, commandBuffer, sizeof(VkDrawIndirectCommand), buffer, offset, maxDrawCount, stride,
                                   function);
    skip |= ValidateCountBuffer(*context, commandBuffer, countBuffer, countBufferOffset, function);

    // TODO: For now, we validate the whole vertex buffer. It might cause some false positive.
    //       VkDrawIndirectCommand buffer could be changed until SubmitQueue.
    //       We will validate the vertex buffer in SubmitQueue in the future.
    skip |= cb_access_context->ValidateDrawVertex(UINT32_MAX, 0, function);
    return skip;
}

bool SyncValidator::PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride) const {
    return ValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                        "vkCmdDrawIndirectCount");
}

void SyncValidator::PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride) {
    StateTracker::PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                    stride);
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_DRAWINDIRECTCOUNT);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawSubpassAttachment(tag);
    RecordIndirectBuffer(*context, tag, sizeof(VkDrawIndirectCommand), buffer, offset, 1, stride);
    RecordCountBuffer(*context, tag, countBuffer, countBufferOffset);

    // TODO: For now, we record the whole vertex buffer. It might cause some false positive.
    //       VkDrawIndirectCommand buffer could be changed until SubmitQueue.
    //       We will record the vertex buffer in SubmitQueue in the future.
    cb_access_context->RecordDrawVertex(UINT32_MAX, 0, tag);
}

bool SyncValidator::PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride) const {
    return ValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                        "vkCmdDrawIndirectCountKHR");
}

void SyncValidator::PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride) {
    StateTracker::PreCallRecordCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                       stride);
    PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

bool SyncValidator::PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride) const {
    return ValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                        "vkCmdDrawIndirectCountAMD");
}

void SyncValidator::PreCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride) {
    StateTracker::PreCallRecordCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                       stride);
    PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

bool SyncValidator::ValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride, const char *function) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, function);
    skip |= cb_access_context->ValidateDrawSubpassAttachment(function);
    skip |= ValidateIndirectBuffer(*context, commandBuffer, sizeof(VkDrawIndexedIndirectCommand), buffer, offset, maxDrawCount,
                                   stride, function);
    skip |= ValidateCountBuffer(*context, commandBuffer, countBuffer, countBufferOffset, function);

    // TODO: For now, we validate the whole index and vertex buffer. It might cause some false positive.
    //       VkDrawIndexedIndirectCommand buffer could be changed until SubmitQueue.
    //       We will validate the index and vertex buffer in SubmitQueue in the future.
    skip |= cb_access_context->ValidateDrawVertexIndex(UINT32_MAX, 0, function);
    return skip;
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride) const {
    return ValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                               "vkCmdDrawIndexedIndirectCount");
}

void SyncValidator::PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                             uint32_t maxDrawCount, uint32_t stride) {
    StateTracker::PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                           maxDrawCount, stride);
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_DRAWINDEXEDINDIRECTCOUNT);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawSubpassAttachment(tag);
    RecordIndirectBuffer(*context, tag, sizeof(VkDrawIndexedIndirectCommand), buffer, offset, 1, stride);
    RecordCountBuffer(*context, tag, countBuffer, countBufferOffset);

    // TODO: For now, we record the whole index and vertex buffer. It might cause some false positive.
    //       VkDrawIndexedIndirectCommand buffer could be changed until SubmitQueue.
    //       We will update the index and vertex buffer in SubmitQueue in the future.
    cb_access_context->RecordDrawVertexIndex(UINT32_MAX, 0, tag);
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride) const {
    return ValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                               "vkCmdDrawIndexedIndirectCountKHR");
}

void SyncValidator::PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride) {
    StateTracker::PreCallRecordCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                              maxDrawCount, stride);
    PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride) const {
    return ValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                               "vkCmdDrawIndexedIndirectCountAMD");
}

void SyncValidator::PreCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride) {
    StateTracker::PreCallRecordCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                              maxDrawCount, stride);
    PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

bool SyncValidator::PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                      const VkClearColorValue *pColor, uint32_t rangeCount,
                                                      const VkImageSubresourceRange *pRanges) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *image_state = Get<IMAGE_STATE>(image);

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            auto hazard =
                context->DetectHazard(*image_state, SYNC_TRANSFER_TRANSFER_WRITE, range, {0, 0, 0}, image_state->createInfo.extent);
            if (hazard.hazard) {
                skip |= LogError(image, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdClearColorImage: Hazard %s for %s, range index %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(image).c_str(), index,
                                 string_UsageTag(hazard).c_str());
            }
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                    const VkClearColorValue *pColor, uint32_t rangeCount,
                                                    const VkImageSubresourceRange *pRanges) {
    StateTracker::PreCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_CLEARCOLORIMAGE);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    const auto *image_state = Get<IMAGE_STATE>(image);

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            context->UpdateAccessState(*image_state, SYNC_TRANSFER_TRANSFER_WRITE, range, {0, 0, 0}, image_state->createInfo.extent,
                                       tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image,
                                                             VkImageLayout imageLayout,
                                                             const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                             const VkImageSubresourceRange *pRanges) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *image_state = Get<IMAGE_STATE>(image);

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            auto hazard =
                context->DetectHazard(*image_state, SYNC_TRANSFER_TRANSFER_WRITE, range, {0, 0, 0}, image_state->createInfo.extent);
            if (hazard.hazard) {
                skip |= LogError(image, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdClearDepthStencilImage: Hazard %s for %s, range index %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(image).c_str(), index,
                                 string_UsageTag(hazard).c_str());
            }
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                           const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                           const VkImageSubresourceRange *pRanges) {
    StateTracker::PreCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_CLEARDEPTHSTENCILIMAGE);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    const auto *image_state = Get<IMAGE_STATE>(image);

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            context->UpdateAccessState(*image_state, SYNC_TRANSFER_TRANSFER_WRITE, range, {0, 0, 0}, image_state->createInfo.extent,
                                       tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                           uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer,
                                                           VkDeviceSize dstOffset, VkDeviceSize stride,
                                                           VkQueryResultFlags flags) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, stride * queryCount);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, range);
        if (hazard.hazard) {
            skip |=
                LogError(dstBuffer, string_SyncHazardVUID(hazard.hazard),
                         "vkCmdCopyQueryPoolResults: Hazard %s for dstBuffer %s. Access info %s.", string_SyncHazard(hazard.hazard),
                         report_data->FormatHandle(dstBuffer).c_str(), string_UsageTag(hazard).c_str());
        }
    }

    // TODO:Track VkQueryPool
    return skip;
}

void SyncValidator::PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                         uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                         VkDeviceSize stride, VkQueryResultFlags flags) {
    StateTracker::PreCallRecordCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset,
                                                       stride, flags);
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_COPYQUERYPOOLRESULTS);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    const auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, stride * queryCount);
        context->UpdateAccessState(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, range, tag);
    }

    // TODO:Track VkQueryPool
}

bool SyncValidator::PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                 VkDeviceSize size, uint32_t data) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(*dst_buffer, dstOffset, size);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, range);
        if (hazard.hazard) {
            skip |= LogError(dstBuffer, string_SyncHazardVUID(hazard.hazard),
                             "vkCmdFillBuffer: Hazard %s for dstBuffer %s. Access info %s.", string_SyncHazard(hazard.hazard),
                             report_data->FormatHandle(dstBuffer).c_str(), string_UsageTag(hazard).c_str());
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                               VkDeviceSize size, uint32_t data) {
    StateTracker::PreCallRecordCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_FILLBUFFER);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    const auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(*dst_buffer, dstOffset, size);
        context->UpdateAccessState(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, range, tag);
    }
}

bool SyncValidator::PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                   VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                   const VkImageResolve *pRegions) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *src_image = Get<IMAGE_STATE>(srcImage);
    const auto *dst_image = Get<IMAGE_STATE>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &resolve_region = pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, SYNC_TRANSFER_TRANSFER_READ, resolve_region.srcSubresource,
                                                resolve_region.srcOffset, resolve_region.extent);
            if (hazard.hazard) {
                skip |= LogError(srcImage, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdResolveImage: Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(srcImage).c_str(), region,
                                 string_UsageTag(hazard).c_str());
            }
        }

        if (dst_image) {
            auto hazard = context->DetectHazard(*dst_image, SYNC_TRANSFER_TRANSFER_WRITE, resolve_region.dstSubresource,
                                                resolve_region.dstOffset, resolve_region.extent);
            if (hazard.hazard) {
                skip |= LogError(dstImage, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdResolveImage: Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(dstImage).c_str(), region,
                                 string_UsageTag(hazard).c_str());
            }
            if (skip) break;
        }
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                 VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                 const VkImageResolve *pRegions) {
    StateTracker::PreCallRecordCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                               pRegions);
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_RESOLVEIMAGE);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto *src_image = Get<IMAGE_STATE>(srcImage);
    auto *dst_image = Get<IMAGE_STATE>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &resolve_region = pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_TRANSFER_TRANSFER_READ, resolve_region.srcSubresource,
                                       resolve_region.srcOffset, resolve_region.extent, tag);
        }
        if (dst_image) {
            context->UpdateAccessState(*dst_image, SYNC_TRANSFER_TRANSFER_WRITE, resolve_region.dstSubresource,
                                       resolve_region.dstOffset, resolve_region.extent, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                                       const VkResolveImageInfo2KHR *pResolveImageInfo) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *src_image = Get<IMAGE_STATE>(pResolveImageInfo->srcImage);
    const auto *dst_image = Get<IMAGE_STATE>(pResolveImageInfo->dstImage);

    for (uint32_t region = 0; region < pResolveImageInfo->regionCount; region++) {
        const auto &resolve_region = pResolveImageInfo->pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, SYNC_TRANSFER_TRANSFER_READ, resolve_region.srcSubresource,
                                                resolve_region.srcOffset, resolve_region.extent);
            if (hazard.hazard) {
                skip |= LogError(pResolveImageInfo->srcImage, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdResolveImage2KHR: Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(pResolveImageInfo->srcImage).c_str(),
                                 region, string_UsageTag(hazard).c_str());
            }
        }

        if (dst_image) {
            auto hazard = context->DetectHazard(*dst_image, SYNC_TRANSFER_TRANSFER_WRITE, resolve_region.dstSubresource,
                                                resolve_region.dstOffset, resolve_region.extent);
            if (hazard.hazard) {
                skip |= LogError(pResolveImageInfo->dstImage, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdResolveImage2KHR: Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(pResolveImageInfo->dstImage).c_str(),
                                 region, string_UsageTag(hazard).c_str());
            }
            if (skip) break;
        }
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                                     const VkResolveImageInfo2KHR *pResolveImageInfo) {
    StateTracker::PreCallRecordCmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_RESOLVEIMAGE2KHR);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto *src_image = Get<IMAGE_STATE>(pResolveImageInfo->srcImage);
    auto *dst_image = Get<IMAGE_STATE>(pResolveImageInfo->dstImage);

    for (uint32_t region = 0; region < pResolveImageInfo->regionCount; region++) {
        const auto &resolve_region = pResolveImageInfo->pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_TRANSFER_TRANSFER_READ, resolve_region.srcSubresource,
                                       resolve_region.srcOffset, resolve_region.extent, tag);
        }
        if (dst_image) {
            context->UpdateAccessState(*dst_image, SYNC_TRANSFER_TRANSFER_WRITE, resolve_region.dstSubresource,
                                       resolve_region.dstOffset, resolve_region.extent, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                   VkDeviceSize dataSize, const void *pData) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        // VK_WHOLE_SIZE not allowed
        const ResourceAccessRange range = MakeRange(dstOffset, dataSize);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, range);
        if (hazard.hazard) {
            skip |= LogError(dstBuffer, string_SyncHazardVUID(hazard.hazard),
                             "vkCmdUpdateBuffer: Hazard %s for dstBuffer %s. Access info %s.", string_SyncHazard(hazard.hazard),
                             report_data->FormatHandle(dstBuffer).c_str(), string_UsageTag(hazard).c_str());
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                 VkDeviceSize dataSize, const void *pData) {
    StateTracker::PreCallRecordCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_UPDATEBUFFER);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    const auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        // VK_WHOLE_SIZE not allowed
        const ResourceAccessRange range = MakeRange(dstOffset, dataSize);
        context->UpdateAccessState(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, range, tag);
    }
}

bool SyncValidator::PreCallValidateCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                           VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, 4);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, range);
        if (hazard.hazard) {
            skip |=
                LogError(dstBuffer, string_SyncHazardVUID(hazard.hazard),
                         "vkCmdWriteBufferMarkerAMD: Hazard %s for dstBuffer %s. Access info %s.", string_SyncHazard(hazard.hazard),
                         report_data->FormatHandle(dstBuffer).c_str(), string_UsageTag(hazard).c_str());
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                         VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) {
    StateTracker::PreCallRecordCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_WRITEBUFFERMARKERAMD);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    const auto *dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, 4);
        context->UpdateAccessState(*dst_buffer, SYNC_TRANSFER_TRANSFER_WRITE, range, tag);
    }
}
