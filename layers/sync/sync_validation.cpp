/* Copyright (c) 2019-2023 The Khronos Group Inc.
 * Copyright (c) 2019-2023 Valve Corporation
 * Copyright (c) 2019-2023 LunarG, Inc.
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
#include <limits>
#include <memory>
#include <vector>

#include "sync/sync_validation.h"
#include "sync/sync_utils.h"

// Utilities to DRY up Get... calls
template <typename Map, typename Key = typename Map::key_type, typename RetVal = std::optional<typename Map::mapped_type>>
RetVal GetMappedOptional(const Map &map, const Key &key) {
    RetVal ret_val;
    auto it = map.find(key);
    if (it != map.cend()) {
        ret_val.emplace(it->second);
    }
    return ret_val;
}
template <typename Map, typename Fn>
typename Map::mapped_type GetMapped(const Map &map, const typename Map::key_type &key, Fn &&default_factory) {
    auto value = GetMappedOptional(map, key);
    return (value) ? *value : default_factory();
}

template <typename Map, typename Key = typename Map::key_type, typename Mapped = typename Map::mapped_type,
          typename Value = typename Mapped::element_type>
Value *GetMappedPlainFromShared(const Map &map, const Key &key) {
    auto value = GetMappedOptional<Map, Key>(map, key);
    if (value) return value->get();
    return nullptr;
}

static bool SimpleBinding(const BINDABLE &bindable) { return !bindable.sparse && bindable.Binding(); }

static const ResourceAccessRange kFullRange(std::numeric_limits<VkDeviceSize>::min(), std::numeric_limits<VkDeviceSize>::max());

static const char *string_SyncHazardVUID(SyncHazard hazard) {
    switch (hazard) {
        case SyncHazard::NONE:
            return "SYNC-HAZARD-NONE";
            break;
        case SyncHazard::READ_AFTER_WRITE:
            return "SYNC-HAZARD-READ-AFTER-WRITE";
            break;
        case SyncHazard::WRITE_AFTER_READ:
            return "SYNC-HAZARD-WRITE-AFTER-READ";
            break;
        case SyncHazard::WRITE_AFTER_WRITE:
            return "SYNC-HAZARD-WRITE-AFTER-WRITE";
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
        case SyncHazard::READ_AFTER_PRESENT:
            return "SYNC-HAZARD-READ-AFTER-PRESENT";
            break;
        case SyncHazard::WRITE_AFTER_PRESENT:
            return "SYNC-HAZARD-WRITE-AFTER-PRESENT";
            break;
        case SyncHazard::PRESENT_AFTER_WRITE:
            return "SYNC-HAZARD-PRESENT-AFTER-WRITE";
            break;
        case SyncHazard::PRESENT_AFTER_READ:
            return "SYNC-HAZARD-PRESENT-AFTER-READ";
            break;
        default:
            assert(0);
    }
    return "SYNC-HAZARD-INVALID";
}

static bool IsHazardVsRead(SyncHazard hazard) {
    bool vs_read = false;
    switch (hazard) {
        case SyncHazard::WRITE_AFTER_READ:
            vs_read = true;
            break;
        case SyncHazard::WRITE_RACING_READ:
            vs_read = true;
            break;
        case SyncHazard::PRESENT_AFTER_READ:
            vs_read = true;
            break;
        default:
            break;
    }
    return vs_read;
}

static const char *string_SyncHazard(SyncHazard hazard) {
    switch (hazard) {
        case SyncHazard::NONE:
            return "NONE";
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
        case SyncHazard::READ_AFTER_PRESENT:
            return "READ_AFTER_PRESENT";
            break;
        case SyncHazard::WRITE_AFTER_PRESENT:
            return "WRITE_AFTER_PRESENT";
            break;
        case SyncHazard::PRESENT_AFTER_WRITE:
            return "PRESENT_AFTER_WRITE";
            break;
        case SyncHazard::PRESENT_AFTER_READ:
            return "PRESENT_AFTER_READ";
            break;
        default:
            assert(0);
    }
    return "INVALID HAZARD";
}

static const SyncStageAccessInfoType *SyncStageAccessInfoFromMask(SyncStageAccessFlags flags) {
    // Return the info for the first bit found
    const SyncStageAccessInfoType *info = nullptr;
    for (size_t i = 0; i < flags.size(); i++) {
        if (flags.test(i)) {
            info = &syncStageAccessInfoByStageAccessIndex()[i];
            break;
        }
    }
    return info;
}

static std::string string_SyncStageAccessFlags(const SyncStageAccessFlags &flags, const char *sep = "|") {
    std::string out_str;
    if (flags.none()) {
        out_str = "0";
    } else {
        for (size_t i = 0; i < syncStageAccessInfoByStageAccessIndex().size(); i++) {
            const auto &info = syncStageAccessInfoByStageAccessIndex()[i];
            if ((flags & info.stage_access_bit).any()) {
                if (!out_str.empty()) {
                    out_str.append(sep);
                }
                out_str.append(info.name);
            }
        }
        if (out_str.length() == 0) {
            out_str.append("Unhandled SyncStageAccess");
        }
    }
    return out_str;
}

struct SyncNodeFormatter {
    const debug_report_data *report_data;
    const BASE_NODE *node;
    const char *label;

    SyncNodeFormatter(const SyncValidator &sync_state, const CMD_BUFFER_STATE *cb_state)
        : report_data(sync_state.report_data), node(cb_state), label("command_buffer") {}
    SyncNodeFormatter(const SyncValidator &sync_state, const IMAGE_STATE *image)
        : report_data(sync_state.report_data), node(image), label("image") {}
    SyncNodeFormatter(const SyncValidator &sync_state, const QUEUE_STATE *q_state)
        : report_data(sync_state.report_data), node(q_state), label("queue") {}
    SyncNodeFormatter(const SyncValidator &sync_state, const BASE_NODE *base_node, const char *label_ = nullptr)
        : report_data(sync_state.report_data), node(base_node), label(label_) {}
};

std::ostream &operator<<(std::ostream &out, const SyncNodeFormatter &formatter) {
    if (formatter.label) {
        out << formatter.label << ": ";
    }
    if (formatter.node) {
        out << formatter.report_data->FormatHandle(*formatter.node).c_str();
        if (formatter.node->Destroyed()) {
            out << " (destroyed)";
        }
    } else {
        out << "null handle";
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, const NamedHandle::FormatterState &formatter) {
    const NamedHandle &handle = formatter.that;
    bool labeled = false;
    if (!handle.name.empty()) {
        out << handle.name;
        labeled = true;
    }
    if (handle.IsIndexed()) {
        out << "[" << handle.index << "]";
        labeled = true;
    }
    if (labeled) {
        out << ": ";
    }
    out << formatter.state.FormatHandle(handle.handle);
    return out;
}

std::ostream &operator<<(std::ostream &out, const ResourceUsageRecord::FormatterState &formatter) {
    const ResourceUsageRecord &record = formatter.record;
    if (record.alt_usage) {
        out << record.alt_usage.Formatter(formatter.sync_state);
    } else {
        out << "command: " << vvl::String(record.command);
        out << ", seq_no: " << record.seq_num;
        if (record.sub_command != 0) {
            out << ", subcmd: " << record.sub_command;
        }
        // Note: ex_cb_state set to null forces output of record.cb_state
        if (!formatter.ex_cb_state || (formatter.ex_cb_state != record.cb_state)) {
            out << ", " << SyncNodeFormatter(formatter.sync_state, record.cb_state);
        }
        for (const auto &named_handle : record.handles) {
            out << "," << named_handle.Formatter(formatter.sync_state);
        }
        out << ", reset_no: " << std::to_string(record.reset_count);
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, const HazardResult::HazardState &hazard) {
    assert(hazard.usage_index < static_cast<SyncStageAccessIndex>(syncStageAccessInfoByStageAccessIndex().size()));
    const auto &usage_info = syncStageAccessInfoByStageAccessIndex()[hazard.usage_index];
    const auto *info = SyncStageAccessInfoFromMask(hazard.prior_access);
    const char *stage_access_name = info ? info->name : "INVALID_STAGE_ACCESS";
    out << "(";
    if (!hazard.recorded_access.get()) {
        // if we have a recorded usage the usage is reported from the recorded contexts point of view
        out << "usage: " << usage_info.name << ", ";
    }
    out << "prior_usage: " << stage_access_name;
    if (IsHazardVsRead(hazard.hazard)) {
        const auto barriers = hazard.access_state->GetReadBarriers(hazard.prior_access);
        out << ", read_barriers: " << string_VkPipelineStageFlags2(barriers);
    } else {
        SyncStageAccessFlags write_barrier = hazard.access_state->GetWriteBarriers();
        out << ", write_barriers: " << string_SyncStageAccessFlags(write_barrier);
    }
    return out;
}

struct NoopBarrierAction {
    explicit NoopBarrierAction() {}
    void operator()(ResourceAccessState *access) const {}
    const bool layout_transition = false;
};

static void InitSubpassContexts(VkQueueFlags queue_flags, const RENDER_PASS_STATE &rp_state, const AccessContext *external_context,
                                std::vector<AccessContext> &subpass_contexts) {
    const auto &create_info = rp_state.createInfo;
    // Add this for all subpasses here so that they exsist during next subpass validation
    subpass_contexts.clear();
    subpass_contexts.reserve(create_info.subpassCount);
    for (uint32_t pass = 0; pass < create_info.subpassCount; pass++) {
        subpass_contexts.emplace_back(pass, queue_flags, rp_state.subpass_dependencies, subpass_contexts, external_context);
    }
}

// NOTE: Make sure the proxy doesn't outlive from, as the proxy is pointing directly to access contexts owned by from.
CommandBufferAccessContext::CommandBufferAccessContext(const CommandBufferAccessContext &from, AsProxyContext dummy)
    : CommandBufferAccessContext(from.sync_state_) {
    // Copy only the needed fields out of from for a temporary, proxy command buffer context
    cb_state_ = from.cb_state_;
    access_log_ = std::make_shared<AccessLog>(*from.access_log_);  // potentially large, but no choice given tagging lookup.
    command_number_ = from.command_number_;
    subcommand_number_ = from.subcommand_number_;
    reset_count_ = from.reset_count_;

    const auto *from_context = from.GetCurrentAccessContext();
    assert(from_context);

    // Construct a fully resolved single access context out of from
    const NoopBarrierAction noop_barrier;
    from_context->ResolveAccessRange(kFullRange, noop_barrier, &cb_access_context_.GetAccessStateMap(), nullptr);
    // The proxy has flatten the current render pass context (if any), but the async contexts are needed for hazard detection
    cb_access_context_.ImportAsyncContexts(*from_context);

    events_context_ = from.events_context_;

    // We don't want to copy the full render_pass_context_ history just for the proxy.
}

std::string CommandBufferAccessContext::FormatUsage(const ResourceUsageTag tag) const {
    if (tag >= access_log_->size()) return std::string();

    std::stringstream out;
    assert(tag < access_log_->size());
    const auto &record = (*access_log_)[tag];
    out << record.Formatter(*sync_state_, cb_state_);
    return out.str();
}

std::string CommandBufferAccessContext::FormatUsage(const ResourceFirstAccess &access) const {
    std::stringstream out;
    assert(access.usage_info);
    out << "(recorded_usage: " << access.usage_info->name;
    out << ", " << FormatUsage(access.tag) << ")";
    return out.str();
}

std::string CommandExecutionContext::FormatHazard(const HazardResult &hazard) const {
    std::stringstream out;
    assert(hazard.IsHazard());
    out << hazard.State();
    out << ", " << FormatUsage(hazard.Tag()) << ")";
    return out.str();
}

bool CommandExecutionContext::ValidForSyncOps() const {
    const bool valid = GetCurrentEventsContext() && GetCurrentAccessContext();
    assert(valid);
    return valid;
}

// NOTE: the attachement read flag is put *only* in the access scope and not in the exect scope, since the ordering
//       rules apply only to this specific access for this stage, and not the stage as a whole. The ordering detection
//       also reflects this special case for read hazard detection (using access instead of exec scope)
static constexpr VkPipelineStageFlags2KHR kColorAttachmentExecScope = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;
static const SyncStageAccessFlags kColorAttachmentAccessScope =
    SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ_BIT |
    SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT |
    SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE_BIT |
    SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ_BIT;  // Note: this is intentionally not in the exec scope
static constexpr VkPipelineStageFlags2KHR kDepthStencilAttachmentExecScope =
    VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT_KHR | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT_KHR;
static const SyncStageAccessFlags kDepthStencilAttachmentAccessScope =
    SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
    SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
    SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ_BIT;  // Note: this is intentionally not in the exec scope
static constexpr VkPipelineStageFlags2KHR kRasterAttachmentExecScope = kDepthStencilAttachmentExecScope | kColorAttachmentExecScope;
static const SyncStageAccessFlags kRasterAttachmentAccessScope = kDepthStencilAttachmentAccessScope | kColorAttachmentAccessScope;

ResourceAccessState::OrderingBarriers ResourceAccessState::kOrderingRules = {
    {{VK_PIPELINE_STAGE_2_NONE_KHR, SyncStageAccessFlags()},
     {kColorAttachmentExecScope, kColorAttachmentAccessScope},
     {kDepthStencilAttachmentExecScope, kDepthStencilAttachmentAccessScope},
     {kRasterAttachmentExecScope, kRasterAttachmentAccessScope}}};

// Sometimes we have an internal access conflict, and we using the kInvalidTag to set and detect in temporary/proxy contexts
static const ResourceUsageTag kInvalidTag(ResourceUsageRecord::kMaxIndex);

static VkDeviceSize ResourceBaseAddress(const BUFFER_STATE &buffer) { return buffer.GetFakeBaseAddress(); }

template <typename T>
static ResourceAccessRange MakeRange(const T &has_offset_and_size) {
    return ResourceAccessRange(has_offset_and_size.offset, (has_offset_and_size.offset + has_offset_and_size.size));
}

static ResourceAccessRange MakeRange(VkDeviceSize start, VkDeviceSize size) { return ResourceAccessRange(start, (start + size)); }

static ResourceAccessRange MakeRange(const BUFFER_STATE &buffer, VkDeviceSize offset, VkDeviceSize size) {
    return MakeRange(offset, buffer.ComputeSize(offset, size));
}

static ResourceAccessRange MakeRange(const BUFFER_VIEW_STATE &buf_view_state) {
    return MakeRange(*buf_view_state.buffer_state.get(), buf_view_state.create_info.offset, buf_view_state.create_info.range);
}

static ResourceAccessRange MakeRange(VkDeviceSize offset, uint32_t first_index, uint32_t count, uint32_t stride) {
    const VkDeviceSize range_start = offset + (first_index * stride);
    const VkDeviceSize range_size = count * stride;
    return MakeRange(range_start, range_size);
}

static ResourceAccessRange MakeRange(const BufferBinding &binding, uint32_t first_index, const std::optional<uint32_t> &count,
                                     uint32_t stride) {
    if (count) {
        return MakeRange(binding.offset, first_index, count.value(), stride);
    }
    return MakeRange(binding);
}

// Range generators for to allow event scope filtration to be limited to the top of the resource access traversal pipeline
//
// Note: there is no "begin/end" or reset facility.  These are each written as "one time through" generators.
//
// Usage:
//  Constructor() -- initializes the generator to point to the begin of the space declared.
//  *  -- the current range of the generator empty signfies end
//  ++ -- advance to the next non-empty range (or end)

// A wrapper for a single range with the same semantics as the actual generators below
template <typename KeyType>
class SingleRangeGenerator {
  public:
    SingleRangeGenerator(const KeyType &range) : current_(range) {}
    const KeyType &operator*() const { return current_; }
    const KeyType *operator->() const { return &current_; }
    SingleRangeGenerator &operator++() {
        current_ = KeyType();  // just one real range
        return *this;
    }

    bool operator==(const SingleRangeGenerator &other) const { return current_ == other.current_; }

  private:
    SingleRangeGenerator() = default;
    const KeyType range_;
    KeyType current_;
};

// Generate the ranges that are the intersection of range and the entries in the RangeMap
template <typename RangeMap, typename KeyType = typename RangeMap::key_type>
class MapRangesRangeGenerator {
  public:
    // Default constructed is safe to dereference for "empty" test, but for no other operation.
    MapRangesRangeGenerator() : range_(), map_(nullptr), map_pos_(), current_() {
        // Default construction for KeyType *must* be empty range
        assert(current_.empty());
    }
    MapRangesRangeGenerator(const RangeMap &filter, const KeyType &range) : range_(range), map_(&filter), map_pos_(), current_() {
        SeekBegin();
    }
    MapRangesRangeGenerator(const MapRangesRangeGenerator &from) = default;

    const KeyType &operator*() const { return current_; }
    const KeyType *operator->() const { return &current_; }
    MapRangesRangeGenerator &operator++() {
        ++map_pos_;
        UpdateCurrent();
        return *this;
    }

    bool operator==(const MapRangesRangeGenerator &other) const { return current_ == other.current_; }

  protected:
    void UpdateCurrent() {
        if (map_pos_ != map_->cend()) {
            current_ = range_ & map_pos_->first;
        } else {
            current_ = KeyType();
        }
    }
    void SeekBegin() {
        map_pos_ = map_->lower_bound(range_);
        UpdateCurrent();
    }

    // Adding this functionality here, to avoid gratuitous Base:: qualifiers in the derived class
    // Note: Not exposed in this classes public interface to encourage using a consistent ++/empty generator semantic
    template <typename Pred>
    MapRangesRangeGenerator &PredicatedIncrement(Pred &pred) {
        do {
            ++map_pos_;
        } while (map_pos_ != map_->cend() && map_pos_->first.intersects(range_) && !pred(map_pos_));
        UpdateCurrent();
        return *this;
    }

    const KeyType range_;
    const RangeMap *map_;
    typename RangeMap::const_iterator map_pos_;
    KeyType current_;
};
using EventSimpleRangeGenerator = MapRangesRangeGenerator<SyncEventState::ScopeMap>;

// Generate the ranges that are the intersection of the RangeGen ranges and the entries in the FilterMap
// Templated to allow for different Range generators or map sources...
template <typename RangeMap, typename RangeGen, typename KeyType = typename RangeMap::key_type>
class FilteredGeneratorGenerator {
  public:
    // Default constructed is safe to dereference for "empty" test, but for no other operation.
    FilteredGeneratorGenerator() : filter_(nullptr), gen_(), filter_pos_(), current_() {
        // Default construction for KeyType *must* be empty range
        assert(current_.empty());
    }
    FilteredGeneratorGenerator(const RangeMap &filter, RangeGen &gen) : filter_(&filter), gen_(gen), filter_pos_(), current_() {
        SeekBegin();
    }
    FilteredGeneratorGenerator(const FilteredGeneratorGenerator &from) = default;
    const KeyType &operator*() const { return current_; }
    const KeyType *operator->() const { return &current_; }
    FilteredGeneratorGenerator &operator++() {
        KeyType gen_range = GenRange();
        KeyType filter_range = FilterRange();
        current_ = KeyType();
        while (gen_range.non_empty() && filter_range.non_empty() && current_.empty()) {
            if (gen_range.end > filter_range.end) {
                // if the generated range is beyond the filter_range, advance the filter range
                filter_range = AdvanceFilter();
            } else {
                gen_range = AdvanceGen();
            }
            current_ = gen_range & filter_range;
        }
        return *this;
    }

    bool operator==(const FilteredGeneratorGenerator &other) const { return current_ == other.current_; }

  private:
    KeyType AdvanceFilter() {
        ++filter_pos_;
        auto filter_range = FilterRange();
        if (filter_range.valid()) {
            FastForwardGen(filter_range);
        }
        return filter_range;
    }
    KeyType AdvanceGen() {
        ++gen_;
        auto gen_range = GenRange();
        if (gen_range.valid()) {
            FastForwardFilter(gen_range);
        }
        return gen_range;
    }

    KeyType FilterRange() const { return (filter_pos_ != filter_->cend()) ? filter_pos_->first : KeyType(); }
    KeyType GenRange() const { return *gen_; }

    KeyType FastForwardFilter(const KeyType &range) {
        auto filter_range = FilterRange();
        int retry_count = 0;
        const static int kRetryLimit = 2;  // TODO -- determine whether this limit is optimal
        while (!filter_range.empty() && (filter_range.end <= range.begin)) {
            if (retry_count < kRetryLimit) {
                ++filter_pos_;
                filter_range = FilterRange();
                retry_count++;
            } else {
                // Okay we've tried walking, do a seek.
                filter_pos_ = filter_->lower_bound(range);
                break;
            }
        }
        return FilterRange();
    }

    // TODO: Consider adding "seek" (or an absolute bound "get" to range generators to make this walk
    // faster.
    KeyType FastForwardGen(const KeyType &range) {
        auto gen_range = GenRange();
        while (!gen_range.empty() && (gen_range.end <= range.begin)) {
            ++gen_;
            gen_range = GenRange();
        }
        return gen_range;
    }

    void SeekBegin() {
        auto gen_range = GenRange();
        if (gen_range.empty()) {
            current_ = KeyType();
            filter_pos_ = filter_->cend();
        } else {
            filter_pos_ = filter_->lower_bound(gen_range);
            current_ = gen_range & FilterRange();
        }
    }

    const RangeMap *filter_;
    RangeGen gen_;
    typename RangeMap::const_iterator filter_pos_;
    KeyType current_;
};

using EventImageRangeGenerator = FilteredGeneratorGenerator<SyncEventState::ScopeMap, subresource_adapter::ImageRangeGenerator>;

SyncStageAccessIndex GetSyncStageAccessIndexsByDescriptorSet(VkDescriptorType descriptor_type,
                                                             const ResourceInterfaceVariable &variable,
                                                             VkShaderStageFlagBits stage_flag) {
    if (descriptor_type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
        assert(stage_flag == VK_SHADER_STAGE_FRAGMENT_BIT);
        return SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ;
    }
    const auto stage_accesses = sync_utils::GetShaderStageAccesses(stage_flag);

    if (descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
        return stage_accesses.uniform_read;
    }

    // If the desriptorSet is writable, we don't need to care SHADER_READ. SHADER_WRITE is enough.
    // Because if write hazard happens, read hazard might or might not happen.
    // But if write hazard doesn't happen, read hazard is impossible to happen.
    if (variable.is_written_to) {
        return stage_accesses.storage_write;
    } else if (descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
               descriptor_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
               descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) {
        return stage_accesses.sampled_read;
    } else {
        return stage_accesses.storage_read;
    }
}

bool IsImageLayoutDepthWritable(VkImageLayout image_layout) {
    return (image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
            image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
            image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
}

bool IsImageLayoutStencilWritable(VkImageLayout image_layout) {
    return (image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
            image_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL ||
            image_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL);
}

// Tranverse the attachment resolves for this a specific subpass, and do action() to them.
// Used by both validation and record operations
//
// The signature for Action() reflect the needs of both uses.
template <typename Action>
void ResolveOperation(Action &action, const RENDER_PASS_STATE &rp_state, const AttachmentViewGenVector &attachment_views,
                      uint32_t subpass) {
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
                       AttachmentViewGen::Gen::kRenderArea, SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ,
                       SyncOrdering::kColorAttachment);
                action("color", "resolve write", color_attach, resolve_attach, attachment_views[resolve_attach],
                       AttachmentViewGen::Gen::kRenderArea, SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE,
                       SyncOrdering::kColorAttachment);
            }
        }
    }

    // Depth stencil resolve only if the extension is present
    const auto ds_resolve = vku::FindStructInPNextChain<VkSubpassDescriptionDepthStencilResolve>(subpass_ci.pNext);
    if (ds_resolve && ds_resolve->pDepthStencilResolveAttachment &&
        (ds_resolve->pDepthStencilResolveAttachment->attachment != VK_ATTACHMENT_UNUSED) && subpass_ci.pDepthStencilAttachment &&
        (subpass_ci.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED)) {
        const auto src_at = subpass_ci.pDepthStencilAttachment->attachment;
        const auto src_ci = attachment_ci[src_at];
        // The formats are required to match so we can pick either
        const bool resolve_depth = (ds_resolve->depthResolveMode != VK_RESOLVE_MODE_NONE) && vkuFormatHasDepth(src_ci.format);
        const bool resolve_stencil = (ds_resolve->stencilResolveMode != VK_RESOLVE_MODE_NONE) && vkuFormatHasStencil(src_ci.format);
        const auto dst_at = ds_resolve->pDepthStencilResolveAttachment->attachment;

        // Figure out which aspects are actually touched during resolve operations
        const char *aspect_string = nullptr;
        AttachmentViewGen::Gen gen_type = AttachmentViewGen::Gen::kRenderArea;
        if (resolve_depth && resolve_stencil) {
            aspect_string = "depth/stencil";
        } else if (resolve_depth) {
            // Validate depth only
            gen_type = AttachmentViewGen::Gen::kDepthOnlyRenderArea;
            aspect_string = "depth";
        } else if (resolve_stencil) {
            // Validate all stencil only
            gen_type = AttachmentViewGen::Gen::kStencilOnlyRenderArea;
            aspect_string = "stencil";
        }

        if (aspect_string) {
            action(aspect_string, "resolve read", src_at, dst_at, attachment_views[src_at], gen_type,
                   SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ, SyncOrdering::kRaster);
            action(aspect_string, "resolve write", src_at, dst_at, attachment_views[dst_at], gen_type,
                   SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, SyncOrdering::kRaster);
        }
    }
}

// Action for validating resolve operations
class ValidateResolveAction {
  public:
    ValidateResolveAction(VkRenderPass render_pass, uint32_t subpass, const AccessContext &context,
                          const CommandExecutionContext &exec_context, vvl::Func command)
        : render_pass_(render_pass),
          subpass_(subpass),
          context_(context),
          exec_context_(exec_context),
          command_(command),
          skip_(false) {}
    void operator()(const char *aspect_name, const char *attachment_name, uint32_t src_at, uint32_t dst_at,
                    const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type, SyncStageAccessIndex current_usage,
                    SyncOrdering ordering_rule) {
        HazardResult hazard;
        hazard = context_.DetectHazard(view_gen, gen_type, current_usage, ordering_rule);
        if (hazard.IsHazard()) {
            skip_ |= exec_context_.GetSyncState().LogError(
                render_pass_, string_SyncHazardVUID(hazard.Hazard()),
                "%s: Hazard %s in subpass %" PRIu32 "during %s %s, from attachment %" PRIu32 " to resolve attachment %" PRIu32
                ". Access info %s.",
                vvl::String(command_), string_SyncHazard(hazard.Hazard()), subpass_, aspect_name, attachment_name, src_at, dst_at,
                exec_context_.FormatHazard(hazard).c_str());
        }
    }
    // Providing a mechanism for the constructing caller to get the result of the validation
    bool GetSkip() const { return skip_; }

  private:
    VkRenderPass render_pass_;
    const uint32_t subpass_;
    const AccessContext &context_;
    const CommandExecutionContext &exec_context_;
    vvl::Func command_;
    bool skip_;
};

// Update action for resolve operations
class UpdateStateResolveAction {
  public:
    UpdateStateResolveAction(AccessContext &context, ResourceUsageTag tag) : context_(context), tag_(tag) {}
    void operator()(const char *, const char *, uint32_t, uint32_t, const AttachmentViewGen &view_gen,
                    AttachmentViewGen::Gen gen_type, SyncStageAccessIndex current_usage, SyncOrdering ordering_rule) {
        // Ignores validation only arguments...
        context_.UpdateAccessState(view_gen, gen_type, current_usage, ordering_rule, tag_);
    }

  private:
    AccessContext &context_;
    const ResourceUsageTag tag_;
};

void HazardResult::Set(const ResourceAccessState *access_state_, const SyncStageAccessInfoType &usage_info_, SyncHazard hazard_,
                       const ResourceAccessWriteState &prior_write) {
    state_.emplace(access_state_, usage_info_, hazard_, prior_write.Access().stage_access_bit, prior_write.Tag());
}

void HazardResult::Set(const ResourceAccessState *access_state_, const SyncStageAccessInfoType &usage_info_, SyncHazard hazard_,
                       const SyncStageAccessFlags &prior_, ResourceUsageTag tag_) {
    state_.emplace(access_state_, usage_info_, hazard_, prior_, tag_);
}

void HazardResult::AddRecordedAccess(const ResourceFirstAccess &first_access) {
    assert(state_.has_value());
    state_->recorded_access = std::make_unique<const ResourceFirstAccess>(first_access);
}
bool HazardResult::IsWAWHazard() const {
    assert(state_.has_value());
    return (state_->hazard == WRITE_AFTER_WRITE) && (state_->prior_access[state_->usage_index]);
}

AccessContext::AccessContext(uint32_t subpass, VkQueueFlags queue_flags,
                             const std::vector<SubpassDependencyGraphNode> &dependencies,
                             const std::vector<AccessContext> &contexts, const AccessContext *external_context) {
    Reset();
    const auto &subpass_dep = dependencies[subpass];
    const bool has_barrier_from_external = subpass_dep.barrier_from_external.size() > 0U;
    prev_.reserve(subpass_dep.prev.size() + (has_barrier_from_external ? 1U : 0U));
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
        // Start tags are not known at creation time (as it's done at BeginRenderpass)
        async_.emplace_back(contexts[async_subpass], kInvalidTag);
    }

    if (has_barrier_from_external) {
        // Store the barrier from external with the reat, but save pointer for "by subpass" lookups.
        prev_.emplace_back(external_context, queue_flags, subpass_dep.barrier_from_external);
        src_external_ = &prev_.back();
    }
    if (subpass_dep.barrier_to_external.size()) {
        dst_external_ = TrackBack(this, queue_flags, subpass_dep.barrier_to_external);
    }
}

void AccessContext::Trim() {
    auto normalize = [](ResourceAccessRangeMap::value_type &access) { access.second.Normalize(); };
    ForAll(normalize);
    // Consolidate map after normalization, combines directly adjacent ranges with common values.
    sparse_container::consolidate(access_state_map_);
}

void AccessContext::AddReferencedTags(ResourceUsageTagSet &used) const {
    auto gather = [&used](const ResourceAccessRangeMap::value_type &access) { access.second.GatherReferencedTags(used); };
    ConstForAll(gather);
}

template <typename Detector>
HazardResult AccessContext::DetectPreviousHazard(Detector &detector, const ResourceAccessRange &range) const {
    ResourceAccessRangeMap descent_map;
    ResolvePreviousAccess(range, &descent_map, nullptr);

    HazardResult hazard;
    for (auto prev = descent_map.begin(); prev != descent_map.end() && !hazard.IsHazard(); ++prev) {
        hazard = detector.Detect(prev);
    }
    return hazard;
}

template <typename Action>
void AccessContext::ForAll(Action &&action) {
    for (auto &access : access_state_map_) {
        action(access);
    }
}

template <typename Action>
void AccessContext::ConstForAll(Action &&action) const {
    for (auto &access : access_state_map_) {
        action(access);
    }
}

template <typename Predicate>
void AccessContext::EraseIf(Predicate &&pred) {
    // Note: Don't forward, we don't want r-values moved, since we're going to make multiple calls.
    vvl::EraseIf(access_state_map_, pred);
}

template <typename Detector, typename RangeGen>
HazardResult AccessContext::DetectHazard(Detector &detector, RangeGen &range_gen, DetectOptions options) const {
    for (; range_gen->non_empty(); ++range_gen) {
        HazardResult hazard = DetectHazard(detector, *range_gen, options);
        if (hazard.IsHazard()) return hazard;
    }
    return HazardResult();
}

// A recursive range walker for hazard detection, first for the current context and the (DetectHazardRecur) to walk
// the DAG of the contexts (for example subpasses)
template <typename Detector>
HazardResult AccessContext::DetectHazard(Detector &detector, const ResourceAccessRange &range, DetectOptions options) const {
    HazardResult hazard;

    if (static_cast<uint32_t>(options) & DetectOptions::kDetectAsync) {
        // Async checks don't require recursive lookups, as the async lists are exhaustive for the top-level context
        // so we'll check these first
        for (const auto &async_ref : async_) {
            hazard = async_ref.Context().DetectAsyncHazard(detector, range, async_ref.StartTag());
            if (hazard.IsHazard()) return hazard;
        }
    }

    const bool detect_prev = (static_cast<uint32_t>(options) & DetectOptions::kDetectPrevious) != 0;

    const auto the_end = access_state_map_.cend();  // End is not invalidated
    auto pos = access_state_map_.lower_bound(range);
    ResourceAccessRange gap = {range.begin, range.begin};

    while (pos != the_end && pos->first.begin < range.end) {
        // Cover any leading gap, or gap between entries
        if (detect_prev) {
            // TODO: After profiling we may want to change the descent logic such that we don't recur per gap...
            // Cover any leading gap, or gap between entries
            gap.end = pos->first.begin;  // We know this begin is < range.end
            if (gap.non_empty()) {
                // Recur on all gaps
                hazard = DetectPreviousHazard(detector, gap);
                if (hazard.IsHazard()) return hazard;
            }
            // Set up for the next gap.  If pos..end is >= range.end, loop will exit, and trailing gap will be empty
            gap.begin = pos->first.end;
        }

        hazard = detector.Detect(pos);
        if (hazard.IsHazard()) return hazard;
        ++pos;
    }

    if (detect_prev) {
        // Detect in the trailing empty as needed
        gap.end = range.end;
        if (gap.non_empty()) {
            hazard = DetectPreviousHazard(detector, gap);
        }
    }

    return hazard;
}

// A non recursive range walker for the asynchronous contexts (those we have no barriers with)
template <typename Detector>
HazardResult AccessContext::DetectAsyncHazard(const Detector &detector, const ResourceAccessRange &range,
                                              ResourceUsageTag async_tag) const {
    auto pos = access_state_map_.lower_bound(range);
    const auto the_end = access_state_map_.end();

    HazardResult hazard;
    while (pos != the_end && pos->first.begin < range.end) {
        hazard = detector.DetectAsync(pos, async_tag);
        if (hazard.IsHazard()) break;
        ++pos;
    }

    return hazard;
}

struct ApplySubpassTransitionBarriersAction {
    explicit ApplySubpassTransitionBarriersAction(const std::vector<SyncBarrier> &barriers_) : barriers(barriers_) {}
    void operator()(ResourceAccessState *access) const {
        assert(access);
        access->ApplyBarriers(barriers, true);
    }
    const std::vector<SyncBarrier> &barriers;
};

struct QueueTagOffsetBarrierAction {
    QueueTagOffsetBarrierAction(QueueId qid, ResourceUsageTag offset) : queue_id(qid), tag_offset(offset) {}
    void operator()(ResourceAccessState *access) const {
        access->OffsetTag(tag_offset);
        access->SetQueueId(queue_id);
    };
    QueueId queue_id;
    ResourceUsageTag tag_offset;
};

struct ApplyTrackbackStackAction {
    explicit ApplyTrackbackStackAction(const std::vector<SyncBarrier> &barriers_,
                                       const ResourceAccessStateFunction *previous_barrier_ = nullptr)
        : barriers(barriers_), previous_barrier(previous_barrier_) {}
    void operator()(ResourceAccessState *access) const {
        assert(access);
        assert(!access->HasPendingState());
        access->ApplyBarriers(barriers, false);
        // NOTE: We can use invalid tag, as these barriers do no include layout transitions (would assert in SetWrite)
        access->ApplyPendingBarriers(kInvalidTag);
        if (previous_barrier) {
            assert(bool(*previous_barrier));
            (*previous_barrier)(access);
        }
    }
    const std::vector<SyncBarrier> &barriers;
    const ResourceAccessStateFunction *previous_barrier;
};

static SyncBarrier MergeBarriers(const std::vector<SyncBarrier> &barriers) {
    SyncBarrier merged = {};
    for (const auto &barrier : barriers) {
        merged.Merge(barrier);
    }
    return merged;
}
template <typename BarrierAction>
void AccessContext::ResolveAccessRange(const ResourceAccessRange &range, BarrierAction &barrier_action,
                                       ResourceAccessRangeMap *resolve_map, const ResourceAccessState *infill_state,
                                       bool recur_to_infill) const {
    if (!range.non_empty()) return;

    ResourceRangeMergeIterator current(*resolve_map, access_state_map_, range.begin);
    while (current->range.non_empty() && range.includes(current->range.begin)) {
        const auto current_range = current->range & range;
        if (current->pos_B->valid) {
            const auto &src_pos = current->pos_B->lower_bound;
            ResourceAccessState access(src_pos->second);  // intentional copy
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
                ResourceAccessRange recurrence_range = current_range;
                // The current context is empty for the current range, so recur to fill the gap.
                // Since we will be recurring back up the DAG, expand the gap descent to cover the full range for which B
                // is not valid, to minimize that recurrence
                if (current->pos_B.at_end()) {
                    // Do the remainder here....
                    recurrence_range.end = range.end;
                } else {
                    // Recur only over the range until B becomes valid (within the limits of range).
                    recurrence_range.end = std::min(range.end, current->pos_B->lower_bound->first.begin);
                }
                ResolvePreviousAccessStack(recurrence_range, resolve_map, infill_state, barrier_action);

                // Given that there could be gaps we need to seek carefully to not repeatedly search the same gaps in the next
                // iterator of the outer while.

                // Set the parallel iterator to the end of this range s.t. ++ will move us to the next range whether or
                // not the end of the range is a gap.  For the seek to work, first we need to warn the parallel iterator
                // we stepped on the dest map
                const auto seek_to = recurrence_range.end - 1;  // The subtraction is safe as range can't be empty (loop condition)
                current.invalidate_A();                         // Changes current->range
                current.seek(seek_to);
            } else if (!current->pos_A->valid && infill_state) {
                // If we didn't find anything in the current range, and we aren't reccuring... we infill if required
                auto inserted = resolve_map->insert(current->pos_A->lower_bound, std::make_pair(current->range, *infill_state));
                current.invalidate_A(inserted);  // Update the parallel iterator to point at the correct segment after insert
            }
        }
        if (current->range.non_empty()) {
            ++current;
        }
    }

    // Infill if range goes passed both the current and resolve map prior contents
    if (recur_to_infill && (current->range.end < range.end)) {
        ResourceAccessRange trailing_fill_range = {current->range.end, range.end};
        ResolvePreviousAccessStack<BarrierAction>(trailing_fill_range, resolve_map, infill_state, barrier_action);
    }
}

template <typename BarrierAction>
void AccessContext::ResolvePreviousAccessStack(const ResourceAccessRange &range, ResourceAccessRangeMap *descent_map,
                                               const ResourceAccessState *infill_state,
                                               const BarrierAction &previous_barrier) const {
    ResourceAccessStateFunction stacked_barrier(std::ref(previous_barrier));
    ResolvePreviousAccess(range, descent_map, infill_state, &stacked_barrier);
}

void AccessContext::ResolvePreviousAccess(const ResourceAccessRange &range, ResourceAccessRangeMap *descent_map,
                                          const ResourceAccessState *infill_state,
                                          const ResourceAccessStateFunction *previous_barrier) const {
    if (prev_.size() == 0) {
        if (range.non_empty() && infill_state) {
            // Fill the empty poritions of descent_map with the default_state with the barrier function applied (iff present)
            ResourceAccessState state_copy;
            if (previous_barrier) {
                assert(bool(*previous_barrier));
                state_copy = *infill_state;
                (*previous_barrier)(&state_copy);
                infill_state = &state_copy;
            }
            sparse_container::update_range_value(*descent_map, range, *infill_state,
                                                 sparse_container::value_precedence::prefer_dest);
        }
    } else {
        // Look for something to fill the gap further along.
        for (const auto &prev_dep : prev_) {
            const ApplyTrackbackStackAction barrier_action(prev_dep.barriers, previous_barrier);
            prev_dep.source_subpass->ResolveAccessRange(range, barrier_action, descent_map, infill_state);
        }
    }
}

// Non-lazy import of all accesses, WaitEvents needs this.
void AccessContext::ResolvePreviousAccesses() {
    ResourceAccessState default_state;
    if (!prev_.size()) return;  // If no previous contexts, nothing to do

    ResolvePreviousAccess(kFullRange, &access_state_map_, &default_state);
}

static SyncStageAccessIndex ColorLoadUsage(VkAttachmentLoadOp load_op) {
    const auto stage_access = (load_op == VK_ATTACHMENT_LOAD_OP_NONE_EXT)
                                  ? SYNC_ACCESS_INDEX_NONE
                                  : ((load_op == VK_ATTACHMENT_LOAD_OP_LOAD) ? SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ
                                                                             : SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE);
    return stage_access;
}
static SyncStageAccessIndex DepthStencilLoadUsage(VkAttachmentLoadOp load_op) {
    const auto stage_access =
        (load_op == VK_ATTACHMENT_LOAD_OP_NONE_EXT)
            ? SYNC_ACCESS_INDEX_NONE
            : ((load_op == VK_ATTACHMENT_LOAD_OP_LOAD) ? SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_READ
                                                       : SYNC_EARLY_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE);
    return stage_access;
}

// Caller must manage returned pointer
static AccessContext *CreateStoreResolveProxyContext(const AccessContext &context, const RENDER_PASS_STATE &rp_state,
                                                     uint32_t subpass, const AttachmentViewGenVector &attachment_views) {
    auto *proxy = new AccessContext(context);
    proxy->UpdateAttachmentResolveAccess(rp_state, attachment_views, subpass, kInvalidTag);
    proxy->UpdateAttachmentStoreAccess(rp_state, attachment_views, subpass, kInvalidTag);
    return proxy;
}

template <typename BarrierAction>
void AccessContext::ResolveAccessRange(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type,
                                       BarrierAction &barrier_action, ResourceAccessRangeMap *descent_map,
                                       const ResourceAccessState *infill_state) const {
    const std::optional<ImageRangeGen> &attachment_gen = view_gen.GetRangeGen(gen_type);
    if (!attachment_gen) return;

    subresource_adapter::ImageRangeGenerator range_gen(*attachment_gen);
    for (; range_gen->non_empty(); ++range_gen) {
        ResolveAccessRange(*range_gen, barrier_action, descent_map, infill_state);
    }
}

template <typename ResolveOp>
void AccessContext::ResolveFromContext(ResolveOp &&resolve_op, const AccessContext &from_context,
                                       const ResourceAccessState *infill_state, bool recur_to_infill) {
    from_context.ResolveAccessRange(kFullRange, resolve_op, &access_state_map_, infill_state, recur_to_infill);
}

template <typename ResolveOp, typename RangeGenerator>
void AccessContext::ResolveFromContext(ResolveOp &&resolve_op, const AccessContext &from_context, RangeGenerator range_gen,
                                       const ResourceAccessState *infill_state, bool recur_to_infill) {
    for (; range_gen->non_empty(); ++range_gen) {
        from_context.ResolveAccessRange(*range_gen, resolve_op, &access_state_map_, infill_state, recur_to_infill);
    }
}

// Layout transitions are handled as if the were occuring in the beginning of the next subpass
bool AccessContext::ValidateLayoutTransitions(const CommandExecutionContext &exec_context, const RENDER_PASS_STATE &rp_state,
                                              const VkRect2D &render_area, uint32_t subpass,
                                              const AttachmentViewGenVector &attachment_views, vvl::Func command) const {
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
        assert(track_back);
        if (prev_needs_proxy) {
            if (!proxy_for_prev) {
                proxy_for_prev.reset(
                    CreateStoreResolveProxyContext(*track_back->source_subpass, rp_state, transition.prev_pass, attachment_views));
                proxy_track_back = *track_back;
                proxy_track_back.source_subpass = proxy_for_prev.get();
            }
            track_back = &proxy_track_back;
        }
        auto hazard = DetectSubpassTransitionHazard(*track_back, attachment_views[transition.attachment]);
        if (hazard.IsHazard()) {
            if (hazard.Tag() == kInvalidTag) {
                skip |= exec_context.GetSyncState().LogError(
                    rp_state.renderPass(), string_SyncHazardVUID(hazard.Hazard()),
                    "%s: Hazard %s in subpass %" PRIu32 " for attachment %" PRIu32
                    " image layout transition (old_layout: %s, new_layout: %s) after store/resolve operation in subpass %" PRIu32,
                    vvl::String(command), string_SyncHazard(hazard.Hazard()), subpass, transition.attachment,
                    string_VkImageLayout(transition.old_layout), string_VkImageLayout(transition.new_layout), transition.prev_pass);
            } else {
                skip |= exec_context.GetSyncState().LogError(
                    rp_state.renderPass(), string_SyncHazardVUID(hazard.Hazard()),
                    "%s: Hazard %s in subpass %" PRIu32 " for attachment %" PRIu32
                    " image layout transition (old_layout: %s, new_layout: %s). Access info %s.",
                    vvl::String(command), string_SyncHazard(hazard.Hazard()), subpass, transition.attachment,
                    string_VkImageLayout(transition.old_layout), string_VkImageLayout(transition.new_layout),
                    exec_context.FormatHazard(hazard).c_str());
            }
        }
    }
    return skip;
}

bool AccessContext::ValidateLoadOperation(const CommandExecutionContext &exec_context, const RENDER_PASS_STATE &rp_state,
                                          const VkRect2D &render_area, uint32_t subpass,
                                          const AttachmentViewGenVector &attachment_views, vvl::Func command) const {
    bool skip = false;
    const auto *attachment_ci = rp_state.createInfo.pAttachments;

    for (uint32_t i = 0; i < rp_state.createInfo.attachmentCount; i++) {
        if (subpass == rp_state.attachment_first_subpass[i]) {
            const auto &view_gen = attachment_views[i];
            if (!view_gen.IsValid()) continue;
            const auto &ci = attachment_ci[i];

            // Need check in the following way
            // 1) if the usage bit isn't in the dest_access_scope, and there is layout traniition for initial use, report hazard
            //    vs. transition
            // 2) if there isn't a layout transition, we need to look at the  external context with a "detect hazard" operation
            //    for each aspect loaded.

            const bool has_depth = vkuFormatHasDepth(ci.format);
            const bool has_stencil = vkuFormatHasStencil(ci.format);
            const bool is_color = !(has_depth || has_stencil);

            const SyncStageAccessIndex load_index = has_depth ? DepthStencilLoadUsage(ci.loadOp) : ColorLoadUsage(ci.loadOp);
            const SyncStageAccessIndex stencil_load_index = has_stencil ? DepthStencilLoadUsage(ci.stencilLoadOp) : load_index;

            HazardResult hazard;
            const char *aspect = nullptr;

            bool checked_stencil = false;
            if (is_color && (load_index != SYNC_ACCESS_INDEX_NONE)) {
                hazard = DetectHazard(view_gen, AttachmentViewGen::Gen::kRenderArea, load_index, SyncOrdering::kColorAttachment);
                aspect = "color";
            } else {
                if (has_depth && (load_index != SYNC_ACCESS_INDEX_NONE)) {
                    hazard = DetectHazard(view_gen, AttachmentViewGen::Gen::kDepthOnlyRenderArea, load_index,
                                          SyncOrdering::kDepthStencilAttachment);
                    aspect = "depth";
                }
                if (!hazard.IsHazard() && has_stencil && (stencil_load_index != SYNC_ACCESS_INDEX_NONE)) {
                    hazard = DetectHazard(view_gen, AttachmentViewGen::Gen::kStencilOnlyRenderArea, stencil_load_index,
                                          SyncOrdering::kDepthStencilAttachment);
                    aspect = "stencil";
                    checked_stencil = true;
                }
            }

            if (hazard.IsHazard()) {
                auto load_op_string = string_VkAttachmentLoadOp(checked_stencil ? ci.stencilLoadOp : ci.loadOp);
                const auto &sync_state = exec_context.GetSyncState();
                if (hazard.Tag() == kInvalidTag) {
                    // Hazard vs. ILT
                    skip |= sync_state.LogError(rp_state.renderPass(), string_SyncHazardVUID(hazard.Hazard()),
                                                "%s: Hazard %s vs. layout transition in subpass %" PRIu32 " for attachment %" PRIu32
                                                " aspect %s during load with loadOp %s.",
                                                vvl::String(command), string_SyncHazard(hazard.Hazard()), subpass, i, aspect,
                                                load_op_string);
                } else {
                    skip |= sync_state.LogError(rp_state.renderPass(), string_SyncHazardVUID(hazard.Hazard()),
                                                "%s: Hazard %s in subpass %" PRIu32 " for attachment %" PRIu32
                                                " aspect %s during load with loadOp %s. Access info %s.",
                                                vvl::String(command), string_SyncHazard(hazard.Hazard()), subpass, i, aspect,
                                                load_op_string, exec_context.FormatHazard(hazard).c_str());
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
bool AccessContext::ValidateStoreOperation(const CommandExecutionContext &exec_context, const RENDER_PASS_STATE &rp_state,
                                           const VkRect2D &render_area, uint32_t subpass,
                                           const AttachmentViewGenVector &attachment_views, vvl::Func command) const {
    bool skip = false;
    const auto *attachment_ci = rp_state.createInfo.pAttachments;

    for (uint32_t i = 0; i < rp_state.createInfo.attachmentCount; i++) {
        if (subpass == rp_state.attachment_last_subpass[i]) {
            const AttachmentViewGen &view_gen = attachment_views[i];
            if (!view_gen.IsValid()) continue;
            const auto &ci = attachment_ci[i];

            // The spec states that "don't care" is an operation with VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            // so we assume that an implementation is *free* to write in that case, meaning that for correctness
            // sake, we treat DONT_CARE as writing.
            const bool has_depth = vkuFormatHasDepth(ci.format);
            const bool has_stencil = vkuFormatHasStencil(ci.format);
            const bool is_color = !(has_depth || has_stencil);
            const bool store_op_stores = ci.storeOp != VK_ATTACHMENT_STORE_OP_NONE_EXT;
            if (!has_stencil && !store_op_stores) continue;

            HazardResult hazard;
            const char *aspect = nullptr;
            bool checked_stencil = false;
            if (is_color) {
                hazard = DetectHazard(view_gen, AttachmentViewGen::Gen::kRenderArea,
                                      SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, SyncOrdering::kRaster);
                aspect = "color";
            } else {
                const bool stencil_op_stores = ci.stencilStoreOp != VK_ATTACHMENT_STORE_OP_NONE_EXT;
                if (has_depth && store_op_stores) {
                    hazard = DetectHazard(view_gen, AttachmentViewGen::Gen::kDepthOnlyRenderArea,
                                          SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, SyncOrdering::kRaster);
                    aspect = "depth";
                }
                if (!hazard.IsHazard() && has_stencil && stencil_op_stores) {
                    hazard = DetectHazard(view_gen, AttachmentViewGen::Gen::kStencilOnlyRenderArea,
                                          SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, SyncOrdering::kRaster);
                    aspect = "stencil";
                    checked_stencil = true;
                }
            }

            if (hazard.IsHazard()) {
                const char *const op_type_string = checked_stencil ? "stencilStoreOp" : "storeOp";
                const char *const store_op_string = string_VkAttachmentStoreOp(checked_stencil ? ci.stencilStoreOp : ci.storeOp);
                skip |= exec_context.GetSyncState().LogError(rp_state.renderPass(), string_SyncHazardVUID(hazard.Hazard()),
                                                             "%s: Hazard %s in subpass %" PRIu32 " for attachment %" PRIu32
                                                             " %s aspect during store with %s %s. Access info %s",
                                                             vvl::String(command), string_SyncHazard(hazard.Hazard()), subpass, i,
                                                             aspect, op_type_string, store_op_string,
                                                             exec_context.FormatHazard(hazard).c_str());
            }
        }
    }
    return skip;
}

bool AccessContext::ValidateResolveOperations(const CommandExecutionContext &exec_context, const RENDER_PASS_STATE &rp_state,
                                              const VkRect2D &render_area, const AttachmentViewGenVector &attachment_views,
                                              vvl::Func command, uint32_t subpass) const {
    ValidateResolveAction validate_action(rp_state.renderPass(), subpass, *this, exec_context, command);
    ResolveOperation(validate_action, rp_state, attachment_views, subpass);
    return validate_action.GetSkip();
}

void AccessContext::AddAsyncContext(const AccessContext *context, ResourceUsageTag tag) {
    if (context) {
        async_.emplace_back(*context, tag);
    }
}

class HazardDetector {
    const SyncStageAccessInfoType &usage_info_;

  public:
    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos) const { return pos->second.DetectHazard(usage_info_); }
    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos, ResourceUsageTag start_tag) const {
        return pos->second.DetectAsyncHazard(usage_info_, start_tag);
    }
    explicit HazardDetector(SyncStageAccessIndex usage_index) : usage_info_(SyncStageAccess::UsageInfo(usage_index)) {}
};

class HazardDetectorWithOrdering {
    const SyncStageAccessInfoType &usage_info_;
    const SyncOrdering ordering_rule_;

  public:
    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos) const {
        return pos->second.DetectHazard(usage_info_, ordering_rule_, QueueSyncState::kQueueIdInvalid);
    }
    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos, ResourceUsageTag start_tag) const {
        return pos->second.DetectAsyncHazard(usage_info_, start_tag);
    }
    HazardDetectorWithOrdering(SyncStageAccessIndex usage_index, SyncOrdering ordering)
        : usage_info_(SyncStageAccess::UsageInfo(usage_index)), ordering_rule_(ordering) {}
};

HazardResult AccessContext::DetectHazard(const BUFFER_STATE &buffer, SyncStageAccessIndex usage_index,
                                         const ResourceAccessRange &range) const {
    if (!SimpleBinding(buffer)) return HazardResult();
    const auto base_address = ResourceBaseAddress(buffer);
    HazardDetector detector(usage_index);
    return DetectHazard(detector, (range + base_address), DetectOptions::kDetectAll);
}

template <typename Detector>
HazardResult AccessContext::DetectHazard(Detector &detector, const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type,
                                         DetectOptions options) const {
    const std::optional<ImageRangeGen> &attachment_gen = view_gen.GetRangeGen(gen_type);
    if (!attachment_gen) return HazardResult();

    subresource_adapter::ImageRangeGenerator range_gen(*attachment_gen);
    return DetectHazard(detector, range_gen, options);
}

template <typename Detector>
HazardResult AccessContext::DetectHazard(Detector &detector, const ImageState &image,
                                         const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset,
                                         const VkExtent3D &extent, bool is_depth_sliced, DetectOptions options) const {
    // range_gen is non-temporary to avoid additional copy
    ImageRangeGen range_gen = image.MakeImageRangeGen(subresource_range, offset, extent, is_depth_sliced);
    return DetectHazard(detector, range_gen, options);
}

template <typename Detector>
HazardResult AccessContext::DetectHazard(Detector &detector, const ImageState &image,
                                         const VkImageSubresourceRange &subresource_range, bool is_depth_sliced,
                                         DetectOptions options) const {
    // range_gen is non-temporary to avoid additional copy
    ImageRangeGen range_gen = image.MakeImageRangeGen(subresource_range, is_depth_sliced);
    return DetectHazard(detector, range_gen, options);
}

HazardResult AccessContext::DetectHazard(const ImageState &image, SyncStageAccessIndex current_usage,
                                         const VkImageSubresourceLayers &subresource, const VkOffset3D &offset,
                                         const VkExtent3D &extent, bool is_depth_sliced) const {
    VkImageSubresourceRange subresource_range = {subresource.aspectMask, subresource.mipLevel, 1, subresource.baseArrayLayer,
                                                 subresource.layerCount};
    HazardDetector detector(current_usage);
    return DetectHazard(detector, image, subresource_range, offset, extent, is_depth_sliced, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const ImageState &image, SyncStageAccessIndex current_usage,
                                         const VkImageSubresourceRange &subresource_range, bool is_depth_sliced) const {
    HazardDetector detector(current_usage);
    return DetectHazard(detector, image, subresource_range, is_depth_sliced, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const ImageViewState &image_view, SyncStageAccessIndex current_usage) const {
    // Get is const, but callee will copy
    HazardDetector detector(current_usage);
    return DetectHazard(detector, image_view.GetFullViewImageRangeGen(), DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const ImageViewState &image_view, SyncStageAccessIndex current_usage,
                                         SyncOrdering ordering_rule, const VkOffset3D &offset, const VkExtent3D &extent) const {
    // range_gen is non-temporary to avoid an additional copy
    ImageRangeGen range_gen(image_view.MakeImageRangeGen(offset, extent));
    HazardDetectorWithOrdering detector(current_usage, ordering_rule);
    return DetectHazard(detector, range_gen, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type,
                                         SyncStageAccessIndex current_usage, SyncOrdering ordering_rule) const {
    HazardDetectorWithOrdering detector(current_usage, ordering_rule);
    return DetectHazard(detector, view_gen, gen_type, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const ImageState &image, SyncStageAccessIndex current_usage,
                                         const VkImageSubresourceRange &subresource_range, SyncOrdering ordering_rule,
                                         const VkOffset3D &offset, const VkExtent3D &extent, bool is_depth_sliced) const {
    HazardDetectorWithOrdering detector(current_usage, ordering_rule);
    return DetectHazard(detector, image, subresource_range, offset, extent, is_depth_sliced, DetectOptions::kDetectAll);
}

class BarrierHazardDetector {
  public:
    BarrierHazardDetector(SyncStageAccessIndex usage_index, VkPipelineStageFlags2KHR src_exec_scope,
                          SyncStageAccessFlags src_access_scope)
        : usage_info_(SyncStageAccess::UsageInfo(usage_index)),
          src_exec_scope_(src_exec_scope),
          src_access_scope_(src_access_scope) {}

    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos) const {
        return pos->second.DetectBarrierHazard(usage_info_, QueueSyncState::kQueueIdInvalid, src_exec_scope_, src_access_scope_);
    }
    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos, ResourceUsageTag start_tag) const {
        // Async barrier hazard detection can use the same path as the usage index is not IsRead, but is IsWrite
        return pos->second.DetectAsyncHazard(usage_info_, start_tag);
    }

  private:
    const SyncStageAccessInfoType &usage_info_;
    VkPipelineStageFlags2KHR src_exec_scope_;
    SyncStageAccessFlags src_access_scope_;
};

class EventBarrierHazardDetector {
  public:
    EventBarrierHazardDetector(SyncStageAccessIndex usage_index, VkPipelineStageFlags2KHR src_exec_scope,
                               SyncStageAccessFlags src_access_scope, const SyncEventState::ScopeMap &event_scope, QueueId queue_id,
                               ResourceUsageTag scope_tag)
        : usage_info_(SyncStageAccess::UsageInfo(usage_index)),
          src_exec_scope_(src_exec_scope),
          src_access_scope_(src_access_scope),
          event_scope_(event_scope),
          scope_queue_id_(queue_id),
          scope_tag_(scope_tag),
          scope_pos_(event_scope.cbegin()),
          scope_end_(event_scope.cend()) {}

    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos) {
        // Need to piece together coverage of pos->first range:
        // Copy the range as we'll be chopping it up as needed
        ResourceAccessRange range = pos->first;
        const ResourceAccessState &access = pos->second;
        HazardResult hazard;

        bool in_scope = AdvanceScope(range);
        bool unscoped_tested = false;
        while (in_scope && !hazard.IsHazard()) {
            if (range.begin < ScopeBegin()) {
                if (!unscoped_tested) {
                    unscoped_tested = true;
                    hazard = access.DetectHazard(usage_info_);
                }
                // Note: don't need to check for in_scope as AdvanceScope true means range and ScopeRange intersect.
                // Thus a [ ScopeBegin, range.end ) will be non-empty.
                range.begin = ScopeBegin();
            } else {  // in_scope implied that ScopeRange and range intersect
                hazard = access.DetectBarrierHazard(usage_info_, ScopeState(), src_exec_scope_, src_access_scope_, scope_queue_id_,
                                                    scope_tag_);
                if (!hazard.IsHazard()) {
                    range.begin = ScopeEnd();
                    in_scope = AdvanceScope(range);  // contains a non_empty check
                }
            }
        }
        if (range.non_empty() && !hazard.IsHazard() && !unscoped_tested) {
            hazard = access.DetectHazard(usage_info_);
        }
        return hazard;
    }

    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos, ResourceUsageTag start_tag) const {
        // Async barrier hazard detection can use the same path as the usage index is not IsRead, but is IsWrite
        return pos->second.DetectAsyncHazard(usage_info_, start_tag);
    }

  private:
    bool ScopeInvalid() const { return scope_pos_ == scope_end_; }
    bool ScopeValid() const { return !ScopeInvalid(); }
    void ScopeSeek(const ResourceAccessRange &range) { scope_pos_ = event_scope_.lower_bound(range); }

    // Hiding away the std::pair grunge...
    ResourceAddress ScopeBegin() const { return scope_pos_->first.begin; }
    ResourceAddress ScopeEnd() const { return scope_pos_->first.end; }
    const ResourceAccessRange &ScopeRange() const { return scope_pos_->first; }
    const ResourceAccessState &ScopeState() const { return scope_pos_->second; }

    bool AdvanceScope(const ResourceAccessRange &range) {
        // Note: non_empty is (valid && !empty), so don't change !non_empty to empty...
        if (!range.non_empty()) return false;
        if (ScopeInvalid()) return false;

        if (ScopeRange().strictly_less(range)) {
            ScopeSeek(range);
        }

        return ScopeValid() && ScopeRange().intersects(range);
    }

    const SyncStageAccessInfoType usage_info_;
    VkPipelineStageFlags2KHR src_exec_scope_;
    SyncStageAccessFlags src_access_scope_;
    const SyncEventState::ScopeMap &event_scope_;
    QueueId scope_queue_id_;
    const ResourceUsageTag scope_tag_;
    SyncEventState::ScopeMap::const_iterator scope_pos_;
    SyncEventState::ScopeMap::const_iterator scope_end_;
};

HazardResult AccessContext::DetectImageBarrierHazard(const ImageState &image, const VkImageSubresourceRange &subresource_range,
                                                     VkPipelineStageFlags2KHR src_exec_scope,
                                                     const SyncStageAccessFlags &src_access_scope, QueueId queue_id,
                                                     const SyncEventState &sync_event, AccessContext::DetectOptions options) const {
    // It's not particularly DRY to get the address type in this function as well as lower down, but we have to select the
    // first access scope map to use, and there's no easy way to plumb it in below.
    const auto &event_scope = sync_event.FirstScope();

    EventBarrierHazardDetector detector(SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION, src_exec_scope, src_access_scope,
                                        event_scope, queue_id, sync_event.first_scope_tag);
    return DetectHazard(detector, image, subresource_range, false, options);
}

HazardResult AccessContext::DetectImageBarrierHazard(const AttachmentViewGen &view_gen, const SyncBarrier &barrier,
                                                     DetectOptions options) const {
    BarrierHazardDetector detector(SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION, barrier.src_exec_scope.exec_scope,
                                   barrier.src_access_scope);
    return DetectHazard(detector, view_gen, AttachmentViewGen::Gen::kViewSubresource, options);
}

HazardResult AccessContext::DetectImageBarrierHazard(const ImageState &image, VkPipelineStageFlags2KHR src_exec_scope,
                                                     const SyncStageAccessFlags &src_access_scope,
                                                     const VkImageSubresourceRange &subresource_range,
                                                     const DetectOptions options) const {
    BarrierHazardDetector detector(SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION, src_exec_scope, src_access_scope);
    return DetectHazard(detector, image, subresource_range, false, options);
}

HazardResult AccessContext::DetectImageBarrierHazard(const SyncImageMemoryBarrier &image_barrier) const {
    return DetectImageBarrierHazard(*image_barrier.image.get(), image_barrier.barrier.src_exec_scope.exec_scope,
                                    image_barrier.barrier.src_access_scope, image_barrier.range, kDetectAll);
}

template <typename Flags, typename Map>
SyncStageAccessFlags AccessScopeImpl(Flags flag_mask, const Map &map) {
    SyncStageAccessFlags scope;
    for (const auto &bit_scope : map) {
        if (flag_mask < bit_scope.first) break;

        if (flag_mask & bit_scope.first) {
            scope |= bit_scope.second;
        }
    }
    return scope;
}

SyncStageAccessFlags SyncStageAccess::AccessScopeByStage(VkPipelineStageFlags2KHR stages) {
    return AccessScopeImpl(stages, syncStageAccessMaskByStageBit());
}

SyncStageAccessFlags SyncStageAccess::AccessScopeByAccess(VkAccessFlags2KHR accesses) {
    return AccessScopeImpl(sync_utils::ExpandAccessFlags(accesses), syncStageAccessMaskByAccessBit());
}

// Getting from stage mask and access mask to stage/access masks is something we need to be good at...
SyncStageAccessFlags SyncStageAccess::AccessScope(VkPipelineStageFlags2KHR stages, VkAccessFlags2KHR accesses) {
    // The access scope is the intersection of all stage/access types possible for the enabled stages and the enables
    // accesses (after doing a couple factoring of common terms the union of stage/access intersections is the intersections
    // of the union of all stage/access types for all the stages and the same unions for the access mask...
    return AccessScopeByStage(stages) & AccessScopeByAccess(accesses);
}

// The semantics of the InfillUpdateOps of infill_update_range are slightly different than for the UpdateMemoryAccessState Action
// operations, as this simplifies the generic traversal.  So we wrap them in a semantics Adapter to get the same effect.
template <typename Action>
struct ActionToOpsAdapter {
    using Map = ResourceAccessRangeMap;
    using Range = typename Map::key_type;
    using Iterator = typename Map::iterator;
    using IndexType = typename Map::index_type;

    void infill(Map &accesses, const Iterator &pos, const Range &infill_range) const {
        // Combine Infill and update operations to make the generic implementation simpler
        Iterator infill = action.Infill(&accesses, pos, infill_range);
        if (infill == accesses.end()) return;  // Allow action to 'pass' on filling in the blanks

        // Need to apply the action to the Infill.  'infill_update_range' expect ops.infill to be completely done with
        // the infill_range, where as Action::Infill assumes the caller will apply the action() logic to the infill_range
        for (; infill != pos; ++infill) {
            assert(infill != accesses.end());
            action(infill);
        }
    }
    void update(const Iterator &pos) const { action(pos); }
    const Action &action;
};
template <typename Action, typename RangeGen>
void UpdateMemoryAccessState(ResourceAccessRangeMap &accesses, const Action &action, RangeGen &range_gen) {
    ActionToOpsAdapter<Action> ops{action};
    for (; range_gen->non_empty(); ++range_gen) {
        infill_update_range(accesses, *range_gen, ops);
    }
}

template <typename Action>
void UpdateMemoryAccessRangeState(ResourceAccessRangeMap &accesses, Action &action, const ResourceAccessRange &range) {
    ActionToOpsAdapter<Action> ops{action};
    infill_update_range(accesses, range, ops);
}

struct UpdateMemoryAccessStateFunctor {
    using Iterator = ResourceAccessRangeMap::iterator;
    Iterator Infill(ResourceAccessRangeMap *accesses, const Iterator &pos, const ResourceAccessRange &range) const {
        // this is only called on gaps, and never returns a gap.
        ResourceAccessState default_state;
        context.ResolvePreviousAccess(range, accesses, &default_state);
        return accesses->lower_bound(range);
    }

    void operator()(const Iterator &pos) const {
        auto &access_state = pos->second;
        access_state.Update(usage_info, ordering_rule, tag);
    }

    UpdateMemoryAccessStateFunctor(const AccessContext &context_, SyncStageAccessIndex usage_, SyncOrdering ordering_rule_,
                                   ResourceUsageTag tag_)
        : context(context_), usage_info(SyncStageAccess::UsageInfo(usage_)), ordering_rule(ordering_rule_), tag(tag_) {}
    const AccessContext &context;
    const SyncStageAccessInfoType &usage_info;
    const SyncOrdering ordering_rule;
    const ResourceUsageTag tag;
};

// The barrier operation for pipeline and subpass dependencies`
struct PipelineBarrierOp {
    SyncBarrier barrier;
    bool layout_transition;
    ResourceAccessState::QueueScopeOps scope;
    PipelineBarrierOp(QueueId queue_id, const SyncBarrier &barrier_, bool layout_transition_)
        : barrier(barrier_), layout_transition(layout_transition_), scope(queue_id) {
        if (queue_id != QueueSyncState::kQueueIdInvalid) {
            // This is a submit time application... supress layout transitions to not taint the QueueBatchContext write state
            layout_transition = false;
        }
    }

    PipelineBarrierOp(const PipelineBarrierOp &rhs)
        : barrier(rhs.barrier), layout_transition(rhs.layout_transition), scope(rhs.scope) {}

    void operator()(ResourceAccessState *access_state) const { access_state->ApplyBarrier(scope, barrier, layout_transition); }
};

// Batch barrier ops don't modify in place, and thus don't need to hold pending state, and also are *never* layout transitions.
struct BatchBarrierOp : public PipelineBarrierOp {
    void operator()(ResourceAccessState *access_state) const {
        access_state->ApplyBarrier(scope, barrier, layout_transition);
        access_state->ApplyPendingBarriers(kInvalidTag);  // There can't be any need for this tag
    }
    BatchBarrierOp(QueueId queue_id, const SyncBarrier &barrier_) : PipelineBarrierOp(queue_id, barrier_, false) {}
};

// The barrier operation for wait events
struct WaitEventBarrierOp {
    ResourceAccessState::EventScopeOps scope_ops;
    SyncBarrier barrier;
    bool layout_transition;

    WaitEventBarrierOp(const QueueId scope_queue_, const ResourceUsageTag scope_tag_, const SyncBarrier &barrier_,
                       bool layout_transition_)
        : scope_ops(scope_queue_, scope_tag_), barrier(barrier_), layout_transition(layout_transition_) {
        if (scope_queue_ != QueueSyncState::kQueueIdInvalid) {
            // This is a submit time application... supress layout transitions to not taint the QueueBatchContext write state
            layout_transition = false;
        }
    }
    void operator()(ResourceAccessState *access_state) const { access_state->ApplyBarrier(scope_ops, barrier, layout_transition); }
};

// This functor applies a collection of barriers, updating the "pending state" in each touched memory range, and optionally
// resolves the pending state. Suitable for processing Global memory barriers, or Subpass Barriers when the "final" barrier
// of a collection is known/present.
template <typename BarrierOp, typename OpVector = std::vector<BarrierOp>>
class ApplyBarrierOpsFunctor {
  public:
    using Iterator = ResourceAccessRangeMap::iterator;
    // Only called with a gap, and pos at the lower_bound(range)
    inline Iterator Infill(ResourceAccessRangeMap *accesses, const Iterator &pos, const ResourceAccessRange &range) const {
        if (!infill_default_) {
            return pos;
        }
        ResourceAccessState default_state;
        auto inserted = accesses->insert(pos, std::make_pair(range, default_state));
        return inserted;
    }

    void operator()(const Iterator &pos) const {
        auto &access_state = pos->second;
        for (const auto &op : barrier_ops_) {
            op(&access_state);
        }

        if (resolve_) {
            // If this is the last (or only) batch, we can do the pending resolve as the last step in this operation to avoid
            // another walk
            access_state.ApplyPendingBarriers(tag_);
        }
    }

    // A valid tag is required IFF layout_transition is true, as transitions are write ops
    ApplyBarrierOpsFunctor(bool resolve, typename OpVector::size_type size_hint, ResourceUsageTag tag)
        : resolve_(resolve), infill_default_(false), barrier_ops_(), tag_(tag) {
        barrier_ops_.reserve(size_hint);
    }
    void EmplaceBack(const BarrierOp &op) {
        barrier_ops_.emplace_back(op);
        infill_default_ |= op.layout_transition;
    }

  private:
    bool resolve_;
    bool infill_default_;
    OpVector barrier_ops_;
    const ResourceUsageTag tag_;
};

// This functor applies a single barrier, updating the "pending state" in each touched memory range, but does not
// resolve the pendinging state. Suitable for processing Image and Buffer barriers from PipelineBarriers or Events
template <typename BarrierOp>
class ApplyBarrierFunctor : public ApplyBarrierOpsFunctor<BarrierOp, small_vector<BarrierOp, 1>> {
    using Base = ApplyBarrierOpsFunctor<BarrierOp, small_vector<BarrierOp, 1>>;

  public:
    ApplyBarrierFunctor(const BarrierOp &barrier_op) : Base(false, 1, kInvalidTag) { Base::EmplaceBack(barrier_op); }
};

// This functor resolves the pendinging state.
class ResolvePendingBarrierFunctor : public ApplyBarrierOpsFunctor<NoopBarrierAction, small_vector<NoopBarrierAction, 1>> {
    using Base = ApplyBarrierOpsFunctor<NoopBarrierAction, small_vector<NoopBarrierAction, 1>>;

  public:
    ResolvePendingBarrierFunctor(ResourceUsageTag tag) : Base(true, 0, tag) {}
};

void AccessContext::UpdateAccessState(const BUFFER_STATE &buffer, SyncStageAccessIndex current_usage, SyncOrdering ordering_rule,
                                      const ResourceAccessRange &range, const ResourceUsageTag tag) {
    if (!SimpleBinding(buffer)) return;
    const auto base_address = ResourceBaseAddress(buffer);
    UpdateMemoryAccessStateFunctor action(*this, current_usage, ordering_rule, tag);
    UpdateMemoryAccessRangeState(access_state_map_, action, range + base_address);
}

void AccessContext::UpdateAccessState(const ImageState &image, SyncStageAccessIndex current_usage, SyncOrdering ordering_rule,
                                      const VkImageSubresourceRange &subresource_range, const ResourceUsageTag &tag) {
    // range_gen is non-temporary to avoid an additional copy
    ImageRangeGen range_gen = image.MakeImageRangeGen(subresource_range, false);
    UpdateAccessState(range_gen, current_usage, ordering_rule, tag);
}
void AccessContext::UpdateAccessState(const ImageState &image, SyncStageAccessIndex current_usage, SyncOrdering ordering_rule,
                                      const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset,
                                      const VkExtent3D &extent, const ResourceUsageTag tag) {
    // range_gen is non-temporary to avoid an additional copy
    ImageRangeGen range_gen = image.MakeImageRangeGen(subresource_range, offset, extent, false);
    UpdateAccessState(range_gen, current_usage, ordering_rule, tag);
}

void AccessContext::UpdateAccessState(const ImageViewState &image_view, SyncStageAccessIndex current_usage,
                                      SyncOrdering ordering_rule, const VkOffset3D &offset, const VkExtent3D &extent,
                                      const ResourceUsageTag tag) {
    // range_gen is non-temporary to avoid an additional copy
    ImageRangeGen range_gen(image_view.MakeImageRangeGen(offset, extent));
    UpdateAccessState(range_gen, current_usage, ordering_rule, tag);
}

void AccessContext::UpdateAccessState(const ImageViewState &image_view, SyncStageAccessIndex current_usage,
                                      SyncOrdering ordering_rule, ResourceUsageTag tag) {
    // Get is const, and will be copied in callee
    UpdateAccessState(image_view.GetFullViewImageRangeGen(), current_usage, ordering_rule, tag);
}

void AccessContext::UpdateAccessState(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type,
                                      SyncStageAccessIndex current_usage, SyncOrdering ordering_rule, const ResourceUsageTag tag) {
    const std::optional<ImageRangeGen> &attachment_gen = view_gen.GetRangeGen(gen_type);
    if (attachment_gen) {
        // Value of const optional is const, and will be copied in callee
        UpdateAccessState(*attachment_gen, current_usage, ordering_rule, tag);
    }
}

void AccessContext::UpdateAccessState(const ImageState &image, SyncStageAccessIndex current_usage, SyncOrdering ordering_rule,
                                      const VkImageSubresourceLayers &subresource, const VkOffset3D &offset,
                                      const VkExtent3D &extent, const ResourceUsageTag tag) {
    VkImageSubresourceRange subresource_range = {subresource.aspectMask, subresource.mipLevel, 1, subresource.baseArrayLayer,
                                                 subresource.layerCount};
    UpdateAccessState(image, current_usage, ordering_rule, subresource_range, offset, extent, tag);
}

void AccessContext::UpdateAccessState(ImageRangeGen &range_gen, SyncStageAccessIndex current_usage, SyncOrdering ordering_rule,
                                      ResourceUsageTag tag) {
    UpdateMemoryAccessStateFunctor action(*this, current_usage, ordering_rule, tag);
    UpdateMemoryAccessState(access_state_map_, action, range_gen);
}

void AccessContext::UpdateAccessState(const ImageRangeGen &range_gen, SyncStageAccessIndex current_usage,
                                      SyncOrdering ordering_rule, ResourceUsageTag tag) {
    // range_gen is non-temporary to avoid infinite call recursion
    ImageRangeGen mutable_range_gen(range_gen);
    UpdateAccessState(mutable_range_gen, current_usage, ordering_rule, tag);
}

template <typename Action>
void AccessContext::ApplyUpdateAction(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type, const Action &action) {
    const std::optional<ImageRangeGen> &ref_range_gen = view_gen.GetRangeGen(gen_type);
    if (ref_range_gen) {
        ImageRangeGen range_gen(*ref_range_gen);
        UpdateMemoryAccessState(access_state_map_, action, range_gen);
    }
}

void AccessContext::UpdateAttachmentResolveAccess(const RENDER_PASS_STATE &rp_state,
                                                  const AttachmentViewGenVector &attachment_views, uint32_t subpass,
                                                  const ResourceUsageTag tag) {
    UpdateStateResolveAction update(*this, tag);
    ResolveOperation(update, rp_state, attachment_views, subpass);
}

void AccessContext::UpdateAttachmentStoreAccess(const RENDER_PASS_STATE &rp_state, const AttachmentViewGenVector &attachment_views,
                                                uint32_t subpass, const ResourceUsageTag tag) {
    const auto *attachment_ci = rp_state.createInfo.pAttachments;

    for (uint32_t i = 0; i < rp_state.createInfo.attachmentCount; i++) {
        if (rp_state.attachment_last_subpass[i] == subpass) {
            const auto &view_gen = attachment_views[i];
            if (!view_gen.IsValid()) continue;  // UNUSED

            const auto &ci = attachment_ci[i];
            const bool has_depth = vkuFormatHasDepth(ci.format);
            const bool has_stencil = vkuFormatHasStencil(ci.format);
            const bool is_color = !(has_depth || has_stencil);
            const bool store_op_stores = ci.storeOp != VK_ATTACHMENT_STORE_OP_NONE_EXT;

            if (is_color && store_op_stores) {
                UpdateAccessState(view_gen, AttachmentViewGen::Gen::kRenderArea,
                                  SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, SyncOrdering::kRaster, tag);
            } else {
                if (has_depth && store_op_stores) {
                    UpdateAccessState(view_gen, AttachmentViewGen::Gen::kDepthOnlyRenderArea,
                                      SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, SyncOrdering::kRaster, tag);
                }
                const bool stencil_op_stores = ci.stencilStoreOp != VK_ATTACHMENT_STORE_OP_NONE_EXT;
                if (has_stencil && stencil_op_stores) {
                    UpdateAccessState(view_gen, AttachmentViewGen::Gen::kStencilOnlyRenderArea,
                                      SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, SyncOrdering::kRaster, tag);
                }
            }
        }
    }
}

template <typename Action>
void AccessContext::ApplyToContext(const Action &barrier_action) {
    // Note: Barriers do *not* cross context boundaries, applying to accessess within.... (at least for renderpass subpasses)
    UpdateMemoryAccessRangeState(access_state_map_, barrier_action, kFullRange);
}

void AccessContext::ResolveChildContexts(const std::vector<AccessContext> &contexts) {
    for (uint32_t subpass_index = 0; subpass_index < contexts.size(); subpass_index++) {
        auto &context = contexts[subpass_index];
        ApplyTrackbackStackAction barrier_action(context.GetDstExternalTrackBack().barriers);
        context.ResolveAccessRange(kFullRange, barrier_action, &access_state_map_, nullptr, false);
    }
}

// Caller must ensure that lifespan of this is less than the lifespan of from
void AccessContext::ImportAsyncContexts(const AccessContext &from) {
    async_.insert(async_.end(), from.async_.begin(), from.async_.end());
}

// Suitable only for *subpass* access contexts
HazardResult AccessContext::DetectSubpassTransitionHazard(const TrackBack &track_back, const AttachmentViewGen &attach_view) const {
    if (!attach_view.IsValid()) return HazardResult();

    // We should never ask for a transition from a context we don't have
    assert(track_back.source_subpass);

    // Do the detection against the specific prior context independent of other contexts.  (Synchronous only)
    // Hazard detection for the transition can be against the merged of the barriers (it only uses src_...)
    const auto merged_barrier = MergeBarriers(track_back.barriers);
    HazardResult hazard = track_back.source_subpass->DetectImageBarrierHazard(attach_view, merged_barrier, kDetectPrevious);
    if (!hazard.IsHazard()) {
        // The Async hazard check is against the current context's async set.
        SyncBarrier null_barrier = {};
        hazard = DetectImageBarrierHazard(attach_view, null_barrier, kDetectAsync);
    }

    return hazard;
}

void AccessContext::RecordLayoutTransitions(const RENDER_PASS_STATE &rp_state, uint32_t subpass,
                                            const AttachmentViewGenVector &attachment_views, const ResourceUsageTag tag) {
    const auto &transitions = rp_state.subpass_transitions[subpass];
    const ResourceAccessState empty_infill;
    for (const auto &transition : transitions) {
        const auto prev_pass = transition.prev_pass;
        const auto &view_gen = attachment_views[transition.attachment];
        if (!view_gen.IsValid()) continue;

        const auto *trackback = GetTrackBackFromSubpass(prev_pass);
        assert(trackback);

        // Import the attachments into the current context
        const auto *prev_context = trackback->source_subpass;
        assert(prev_context);
        ApplySubpassTransitionBarriersAction barrier_action(trackback->barriers);
        prev_context->ResolveAccessRange(view_gen, AttachmentViewGen::Gen::kViewSubresource, barrier_action, &access_state_map_,
                                         &empty_infill);
    }

    // If there were no transitions skip this global map walk
    if (transitions.size()) {
        ResolvePendingBarrierFunctor apply_pending_action(tag);
        ApplyToContext(apply_pending_action);
    }
}

bool CommandBufferAccessContext::ValidateDispatchDrawDescriptorSet(VkPipelineBindPoint pipelineBindPoint,
                                                                   const Location &loc) const {
    bool skip = false;
    const PIPELINE_STATE *pipe = nullptr;
    const std::vector<LAST_BOUND_STATE::PER_SET> *per_sets = nullptr;
    cb_state_->GetCurrentPipelineAndDesriptorSets(pipelineBindPoint, &pipe, &per_sets);
    if (!pipe || !per_sets) {
        return skip;
    }

    using DescriptorClass = cvdescriptorset::DescriptorClass;
    using BufferDescriptor = cvdescriptorset::BufferDescriptor;
    using ImageDescriptor = cvdescriptorset::ImageDescriptor;
    using TexelDescriptor = cvdescriptorset::TexelDescriptor;

    for (const auto &stage_state : pipe->stage_states) {
        const auto raster_state = pipe->RasterizationState();
        if (stage_state.GetStage() == VK_SHADER_STAGE_FRAGMENT_BIT && raster_state && raster_state->rasterizerDiscardEnable) {
            continue;
        } else if (!stage_state.entrypoint) {
            continue;
        }
        for (const auto &variable : stage_state.entrypoint->resource_interface_variables) {
            if (variable.decorations.set >= per_sets->size()) {
                // This should be caught by Core validation, but if core checks are disabled SyncVal should not crash.
                continue;
            }
            const auto *descriptor_set = (*per_sets)[variable.decorations.set].bound_descriptor_set.get();
            if (!descriptor_set) continue;
            auto binding = descriptor_set->GetBinding(variable.decorations.binding);
            const auto descriptor_type = binding->type;
            SyncStageAccessIndex sync_index =
                GetSyncStageAccessIndexsByDescriptorSet(descriptor_type, variable, stage_state.GetStage());

            // Currently, validation of memory accesses based on declared descriptors can produce false-positives.
            // The shader can decide not to do such accesses, it can perform accesses with more narrow scope
            // (e.g. read access, when both reads and writes are allowed) or for an array of descriptors, not all
            // elements are accessed in the general case.
            //
            // This workaround disables validation for the descriptor array case.
            if (binding->count > 1) {
                continue;
            }

            for (uint32_t index = 0; index < binding->count; index++) {
                const auto *descriptor = binding->GetDescriptor(index);
                switch (descriptor->GetClass()) {
                    case DescriptorClass::ImageSampler:
                    case DescriptorClass::Image: {
                        if (descriptor->Invalid()) {
                            continue;
                        }

                        // NOTE: ImageSamplerDescriptor inherits from ImageDescriptor, so this cast works for both types.
                        const auto *image_descriptor = static_cast<const ImageDescriptor *>(descriptor);
                        const auto *img_view_state =
                            static_cast<const syncval_state::ImageViewState *>(image_descriptor->GetImageViewState());
                        VkImageLayout image_layout = image_descriptor->GetImageLayout();

                        if (img_view_state->IsDepthSliced()) {
                            // NOTE: 2D ImageViews of VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT Images are not allowed in
                            // Descriptors, unless VK_EXT_image_2d_view_of_3d is supported, which it isn't at the moment.
                            // See: VUID 00343
                            continue;
                        }

                        HazardResult hazard;

                        if (sync_index == SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ) {
                            const VkExtent3D extent = CastTo3D(cb_state_->active_render_pass_begin_info.renderArea.extent);
                            const VkOffset3D offset = CastTo3D(cb_state_->active_render_pass_begin_info.renderArea.offset);
                            // Input attachments are subject to raster ordering rules
                            hazard =
                                current_context_->DetectHazard(*img_view_state, sync_index, SyncOrdering::kRaster, offset, extent);
                        } else {
                            hazard = current_context_->DetectHazard(*img_view_state, sync_index);
                        }

                        if (hazard.IsHazard() && !sync_state_->SupressedBoundDescriptorWAW(hazard)) {
                            skip |= sync_state_->LogError(
                                string_SyncHazardVUID(hazard.Hazard()), img_view_state->image_view(), loc,
                                "Hazard %s for %s, in %s, and %s, %s, type: %s, imageLayout: %s, binding #%" PRIu32
                                ", index %" PRIu32 ". Access info %s.",
                                string_SyncHazard(hazard.Hazard()), sync_state_->FormatHandle(img_view_state->image_view()).c_str(),
                                sync_state_->FormatHandle(cb_state_->commandBuffer()).c_str(),
                                sync_state_->FormatHandle(pipe->pipeline()).c_str(),
                                sync_state_->FormatHandle(descriptor_set->GetSet()).c_str(),
                                string_VkDescriptorType(descriptor_type), string_VkImageLayout(image_layout),
                                variable.decorations.binding, index, FormatHazard(hazard).c_str());
                        }
                        break;
                    }
                    case DescriptorClass::TexelBuffer: {
                        const auto *texel_descriptor = static_cast<const TexelDescriptor *>(descriptor);
                        if (texel_descriptor->Invalid()) {
                            continue;
                        }
                        const auto *buf_view_state = texel_descriptor->GetBufferViewState();
                        const auto *buf_state = buf_view_state->buffer_state.get();
                        const ResourceAccessRange range = MakeRange(*buf_view_state);
                        auto hazard = current_context_->DetectHazard(*buf_state, sync_index, range);
                        if (hazard.IsHazard() && !sync_state_->SupressedBoundDescriptorWAW(hazard)) {
                            skip |= sync_state_->LogError(
                                string_SyncHazardVUID(hazard.Hazard()), buf_view_state->buffer_view(), loc,
                                "Hazard %s for %s in %s, %s, and %s, type: %s, binding #%d index %d. Access info %s.",
                                string_SyncHazard(hazard.Hazard()),
                                sync_state_->FormatHandle(buf_view_state->buffer_view()).c_str(),
                                sync_state_->FormatHandle(cb_state_->commandBuffer()).c_str(),
                                sync_state_->FormatHandle(pipe->pipeline()).c_str(),
                                sync_state_->FormatHandle(descriptor_set->GetSet()).c_str(),
                                string_VkDescriptorType(descriptor_type), variable.decorations.binding, index,
                                FormatHazard(hazard).c_str());
                        }
                        break;
                    }
                    case DescriptorClass::GeneralBuffer: {
                        const auto *buffer_descriptor = static_cast<const BufferDescriptor *>(descriptor);
                        if (buffer_descriptor->Invalid()) {
                            continue;
                        }
                        const auto *buf_state = buffer_descriptor->GetBufferState();
                        const ResourceAccessRange range =
                            MakeRange(*buf_state, buffer_descriptor->GetOffset(), buffer_descriptor->GetRange());
                        auto hazard = current_context_->DetectHazard(*buf_state, sync_index, range);
                        if (hazard.IsHazard() && !sync_state_->SupressedBoundDescriptorWAW(hazard)) {
                            skip |= sync_state_->LogError(
                                string_SyncHazardVUID(hazard.Hazard()), buf_state->buffer(), loc,
                                "Hazard %s for %s in %s, %s, and %s, type: %s, binding #%d index %d. Access info %s.",
                                string_SyncHazard(hazard.Hazard()), sync_state_->FormatHandle(buf_state->buffer()).c_str(),
                                sync_state_->FormatHandle(cb_state_->commandBuffer()).c_str(),
                                sync_state_->FormatHandle(pipe->pipeline()).c_str(),
                                sync_state_->FormatHandle(descriptor_set->GetSet()).c_str(),
                                string_VkDescriptorType(descriptor_type), variable.decorations.binding, index,
                                FormatHazard(hazard).c_str());
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
                                                                 const ResourceUsageTag tag) {
    const PIPELINE_STATE *pipe = nullptr;
    const std::vector<LAST_BOUND_STATE::PER_SET> *per_sets = nullptr;
    cb_state_->GetCurrentPipelineAndDesriptorSets(pipelineBindPoint, &pipe, &per_sets);
    if (!pipe || !per_sets) {
        return;
    }

    using DescriptorClass = cvdescriptorset::DescriptorClass;
    using BufferDescriptor = cvdescriptorset::BufferDescriptor;
    using ImageDescriptor = cvdescriptorset::ImageDescriptor;
    using TexelDescriptor = cvdescriptorset::TexelDescriptor;

    for (const auto &stage_state : pipe->stage_states) {
        const auto raster_state = pipe->RasterizationState();
        if (stage_state.GetStage() == VK_SHADER_STAGE_FRAGMENT_BIT && raster_state && raster_state->rasterizerDiscardEnable) {
            continue;
        } else if (!stage_state.entrypoint) {
            continue;
        }
        for (const auto &variable : stage_state.entrypoint->resource_interface_variables) {
            if (variable.decorations.set >= per_sets->size()) {
                // This should be caught by Core validation, but if core checks are disabled SyncVal should not crash.
                continue;
            }
            const auto *descriptor_set = (*per_sets)[variable.decorations.set].bound_descriptor_set.get();
            if (!descriptor_set) continue;
            auto binding = descriptor_set->GetBinding(variable.decorations.binding);
            const auto descriptor_type = binding->type;
            SyncStageAccessIndex sync_index =
                GetSyncStageAccessIndexsByDescriptorSet(descriptor_type, variable, stage_state.GetStage());

            // Do not update state for descriptor array (the same as in Validate function).
            if (binding->count > 1) {
                continue;
            }

            for (uint32_t i = 0; i < binding->count; i++) {
                const auto *descriptor = binding->GetDescriptor(i);
                switch (descriptor->GetClass()) {
                    case DescriptorClass::ImageSampler:
                    case DescriptorClass::Image: {
                        // NOTE: ImageSamplerDescriptor inherits from ImageDescriptor, so this cast works for both types.
                        const auto *image_descriptor = static_cast<const ImageDescriptor *>(descriptor);
                        if (image_descriptor->Invalid()) {
                            continue;
                        }
                        const auto *img_view_state =
                            static_cast<const syncval_state::ImageViewState *>(image_descriptor->GetImageViewState());
                        if (img_view_state->IsDepthSliced()) {
                            // NOTE: 2D ImageViews of VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT Images are not allowed in
                            // Descriptors, unless VK_EXT_image_2d_view_of_3d is supported, which it isn't at the moment.
                            // See: VUID 00343
                            continue;
                        }
                        if (sync_index == SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ) {
                            const VkExtent3D extent = CastTo3D(cb_state_->active_render_pass_begin_info.renderArea.extent);
                            const VkOffset3D offset = CastTo3D(cb_state_->active_render_pass_begin_info.renderArea.offset);
                            current_context_->UpdateAccessState(*img_view_state, sync_index, SyncOrdering::kRaster, offset, extent,
                                                                tag);
                        } else {
                            current_context_->UpdateAccessState(*img_view_state, sync_index, SyncOrdering::kNonAttachment, tag);
                        }
                        break;
                    }
                    case DescriptorClass::TexelBuffer: {
                        const auto *texel_descriptor = static_cast<const TexelDescriptor *>(descriptor);
                        if (texel_descriptor->Invalid()) {
                            continue;
                        }
                        const auto *buf_view_state = texel_descriptor->GetBufferViewState();
                        const auto *buf_state = buf_view_state->buffer_state.get();
                        const ResourceAccessRange range = MakeRange(*buf_view_state);
                        current_context_->UpdateAccessState(*buf_state, sync_index, SyncOrdering::kNonAttachment, range, tag);
                        break;
                    }
                    case DescriptorClass::GeneralBuffer: {
                        const auto *buffer_descriptor = static_cast<const BufferDescriptor *>(descriptor);
                        if (buffer_descriptor->Invalid()) {
                            continue;
                        }
                        const auto *buf_state = buffer_descriptor->GetBufferState();
                        const ResourceAccessRange range =
                            MakeRange(*buf_state, buffer_descriptor->GetOffset(), buffer_descriptor->GetRange());
                        current_context_->UpdateAccessState(*buf_state, sync_index, SyncOrdering::kNonAttachment, range, tag);
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

bool CommandBufferAccessContext::ValidateDrawVertex(const std::optional<uint32_t> &vertexCount, uint32_t firstVertex,
                                                    const Location &loc) const {
    bool skip = false;
    const auto *pipe = cb_state_->GetCurrentPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS);
    if (!pipe) {
        return skip;
    }

    const auto &binding_buffers = cb_state_->current_vertex_buffer_binding_info.vertex_buffer_bindings;
    const auto &binding_buffers_size = binding_buffers.size();
    const auto &binding_descriptions_size = pipe->vertex_input_state->binding_descriptions.size();

    for (size_t i = 0; i < binding_descriptions_size; ++i) {
        const auto &binding_description = pipe->vertex_input_state->binding_descriptions[i];
        if (binding_description.binding < binding_buffers_size) {
            const auto &binding_buffer = binding_buffers[binding_description.binding];
            if (!binding_buffer.bound()) continue;

            auto *buf_state = binding_buffer.buffer_state.get();
            const ResourceAccessRange range = MakeRange(binding_buffer, firstVertex, vertexCount, binding_description.stride);
            auto hazard = current_context_->DetectHazard(*buf_state, SYNC_VERTEX_ATTRIBUTE_INPUT_VERTEX_ATTRIBUTE_READ, range);
            if (hazard.IsHazard()) {
                skip |= sync_state_->LogError(string_SyncHazardVUID(hazard.Hazard()), buf_state->buffer(), loc,
                                              "Hazard %s for vertex %s in %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                                              sync_state_->FormatHandle(buf_state->buffer()).c_str(),
                                              sync_state_->FormatHandle(cb_state_->commandBuffer()).c_str(),
                                              FormatHazard(hazard).c_str());
            }
        }
    }
    return skip;
}

void CommandBufferAccessContext::RecordDrawVertex(const std::optional<uint32_t> &vertexCount, uint32_t firstVertex,
                                                  const ResourceUsageTag tag) {
    const auto *pipe = cb_state_->GetCurrentPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS);
    if (!pipe) {
        return;
    }
    const auto &binding_buffers = cb_state_->current_vertex_buffer_binding_info.vertex_buffer_bindings;
    const auto &binding_buffers_size = binding_buffers.size();
    const auto &binding_descriptions_size = pipe->vertex_input_state->binding_descriptions.size();

    for (size_t i = 0; i < binding_descriptions_size; ++i) {
        const auto &binding_description = pipe->vertex_input_state->binding_descriptions[i];
        if (binding_description.binding < binding_buffers_size) {
            const auto &binding_buffer = binding_buffers[binding_description.binding];
            if (!binding_buffer.bound()) continue;

            auto *buf_state = binding_buffer.buffer_state.get();
            const ResourceAccessRange range = MakeRange(binding_buffer, firstVertex, vertexCount, binding_description.stride);
            current_context_->UpdateAccessState(*buf_state, SYNC_VERTEX_ATTRIBUTE_INPUT_VERTEX_ATTRIBUTE_READ,
                                                SyncOrdering::kNonAttachment, range, tag);
        }
    }
}

bool CommandBufferAccessContext::ValidateDrawVertexIndex(const std::optional<uint32_t> &index_count, uint32_t firstIndex,
                                                         const Location &loc) const {
    bool skip = false;
    if (!cb_state_->index_buffer_binding.bound()) {
        return skip;
    }

    const auto &index_binding = cb_state_->index_buffer_binding;
    auto *index_buf_state = index_binding.buffer_state.get();
    const auto index_size = GetIndexAlignment(index_binding.index_type);
    const ResourceAccessRange range = MakeRange(index_binding, firstIndex, index_count, index_size);

    auto hazard = current_context_->DetectHazard(*index_buf_state, SYNC_INDEX_INPUT_INDEX_READ, range);
    if (hazard.IsHazard()) {
        skip |= sync_state_->LogError(string_SyncHazardVUID(hazard.Hazard()), index_buf_state->buffer(), loc,
                                      "Hazard %s for index %s in %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                                      sync_state_->FormatHandle(index_buf_state->buffer()).c_str(),
                                      sync_state_->FormatHandle(cb_state_->commandBuffer()).c_str(), FormatHazard(hazard).c_str());
    }

    // TODO: For now, we detect the whole vertex buffer. Index buffer could be changed until SubmitQueue.
    //       We will detect more accurate range in the future.
    skip |= ValidateDrawVertex(std::optional<uint32_t>(), 0, loc);
    return skip;
}

void CommandBufferAccessContext::RecordDrawVertexIndex(const std::optional<uint32_t> &indexCount, uint32_t firstIndex,
                                                       const ResourceUsageTag tag) {
    if (!cb_state_->index_buffer_binding.bound()) return;

    const auto &index_binding = cb_state_->index_buffer_binding;
    auto *index_buf_state = index_binding.buffer_state.get();
    const auto index_size = GetIndexAlignment(index_binding.index_type);
    const ResourceAccessRange range = MakeRange(index_binding, firstIndex, indexCount, index_size);
    current_context_->UpdateAccessState(*index_buf_state, SYNC_INDEX_INPUT_INDEX_READ, SyncOrdering::kNonAttachment, range, tag);

    // TODO: For now, we detect the whole vertex buffer. Index buffer could be changed until SubmitQueue.
    //       We will detect more accurate range in the future.
    RecordDrawVertex(std::optional<uint32_t>(), 0, tag);
}

bool CommandBufferAccessContext::ValidateDrawSubpassAttachment(const Location &loc) const {
    bool skip = false;
    if (!current_renderpass_context_) return skip;
    skip |= current_renderpass_context_->ValidateDrawSubpassAttachment(GetExecutionContext(), *cb_state_, loc.function);
    return skip;
}

void CommandBufferAccessContext::RecordDrawSubpassAttachment(const ResourceUsageTag tag) {
    if (current_renderpass_context_) {
        current_renderpass_context_->RecordDrawSubpassAttachment(*cb_state_, tag);
    }
}

QueueId CommandBufferAccessContext::GetQueueId() const { return QueueSyncState::kQueueIdInvalid; }

ResourceUsageTag CommandBufferAccessContext::RecordBeginRenderPass(
    vvl::Func command, const RENDER_PASS_STATE &rp_state, const VkRect2D &render_area,
    const std::vector<const syncval_state::ImageViewState *> &attachment_views) {
    // Create an access context the current renderpass.
    const auto barrier_tag = NextCommandTag(command, NamedHandle("renderpass", rp_state.Handle()),
                                            ResourceUsageRecord::SubcommandType::kSubpassTransition);
    const auto load_tag = NextSubcommandTag(command, ResourceUsageRecord::SubcommandType::kLoadOp);
    render_pass_contexts_.emplace_back(
        std::make_unique<RenderPassAccessContext>(rp_state, render_area, GetQueueFlags(), attachment_views, &cb_access_context_));
    current_renderpass_context_ = render_pass_contexts_.back().get();
    current_renderpass_context_->RecordBeginRenderPass(barrier_tag, load_tag);
    current_context_ = &current_renderpass_context_->CurrentContext();
    return barrier_tag;
}

ResourceUsageTag CommandBufferAccessContext::RecordNextSubpass(vvl::Func command) {
    assert(current_renderpass_context_);
    if (!current_renderpass_context_) return NextCommandTag(command);

    auto store_tag = NextCommandTag(command, NamedHandle("renderpass", current_renderpass_context_->GetRenderPassState()->Handle()),
                                    ResourceUsageRecord::SubcommandType::kStoreOp);
    auto barrier_tag = NextSubcommandTag(command, ResourceUsageRecord::SubcommandType::kSubpassTransition);
    auto load_tag = NextSubcommandTag(command, ResourceUsageRecord::SubcommandType::kLoadOp);

    current_renderpass_context_->RecordNextSubpass(store_tag, barrier_tag, load_tag);
    current_context_ = &current_renderpass_context_->CurrentContext();
    return barrier_tag;
}

ResourceUsageTag CommandBufferAccessContext::RecordEndRenderPass(vvl::Func command) {
    assert(current_renderpass_context_);
    if (!current_renderpass_context_) return NextCommandTag(command);

    auto store_tag = NextCommandTag(command, NamedHandle("renderpass", current_renderpass_context_->GetRenderPassState()->Handle()),
                                    ResourceUsageRecord::SubcommandType::kStoreOp);
    auto barrier_tag = NextSubcommandTag(command, ResourceUsageRecord::SubcommandType::kSubpassTransition);

    current_renderpass_context_->RecordEndRenderPass(&cb_access_context_, store_tag, barrier_tag);
    current_context_ = &cb_access_context_;
    current_renderpass_context_ = nullptr;
    return barrier_tag;
}

void CommandBufferAccessContext::RecordDestroyEvent(EVENT_STATE *event_state) { GetCurrentEventsContext()->Destroy(event_state); }

bool ReplayState::DetectFirstUseHazard(const ResourceUsageRange &first_use_range) const {
    bool skip = false;
    if (first_use_range.non_empty()) {
        HazardResult hazard;
        // We're allowing for the Replay(Validate|Record) to modify the exec_context (e.g. for Renderpass operations), so
        // we need to fetch the current access context each time
        hazard = GetRecordedAccessContext()->DetectFirstUseHazard(exec_context_.GetQueueId(), first_use_range,
                                                                  *exec_context_.GetCurrentAccessContext());

        if (hazard.IsHazard()) {
            const SyncValidator &sync_state = exec_context_.GetSyncState();
            const auto handle = exec_context_.Handle();
            const auto recorded_handle = recorded_context_.GetCBState().commandBuffer();
            skip = sync_state.LogError(string_SyncHazardVUID(hazard.Hazard()), handle, error_obj_.location,
                                       "Hazard %s for entry %" PRIu32 ", %s, Recorded access info %s. Access info %s.",
                                       string_SyncHazard(hazard.Hazard()), index_, sync_state.FormatHandle(recorded_handle).c_str(),
                                       recorded_context_.FormatUsage(*hazard.RecordedAccess()).c_str(),
                                       recorded_context_.FormatHazard(hazard).c_str());
        }
    }
    return skip;
}

bool ReplayState::ValidateFirstUse() {
    if (!exec_context_.ValidForSyncOps()) return false;

    bool skip = false;
    ResourceUsageRange first_use_range = {0, 0};

    for (const auto &sync_op : recorded_context_.GetSyncOps()) {
        // Set the range to cover all accesses until the next sync_op, and validate
        first_use_range.end = sync_op.tag;
        skip |= DetectFirstUseHazard(first_use_range);

        // Call to replay validate support for syncop with non-trivial replay
        skip |= sync_op.sync_op->ReplayValidate(*this, sync_op.tag);

        // Record the barrier into the proxy context.
        sync_op.sync_op->ReplayRecord(exec_context_, base_tag_ + sync_op.tag);
        first_use_range.begin = sync_op.tag + 1;
    }

    // and anything after the last syncop
    first_use_range.end = ResourceUsageRecord::kMaxIndex;
    skip |= DetectFirstUseHazard(first_use_range);

    return skip;
}

void CommandBufferAccessContext::RecordExecutedCommandBuffer(const CommandBufferAccessContext &recorded_cb_context) {
    const AccessContext *recorded_context = recorded_cb_context.GetCurrentAccessContext();
    assert(recorded_context);

    // Just run through the barriers ignoring the usage from the recorded context, as Resolve will overwrite outdated state
    const ResourceUsageTag base_tag = GetTagLimit();
    for (const auto &sync_op : recorded_cb_context.GetSyncOps()) {
        // we update the range to any include layout transition first use writes,
        // as they are stored along with the source scope (as effective barrier) when recorded
        sync_op.sync_op->ReplayRecord(*this, base_tag + sync_op.tag);
    }

    ResourceUsageRange tag_range = ImportRecordedAccessLog(recorded_cb_context);
    assert(base_tag == tag_range.begin);  // to ensure the to offset calculation agree
    ResolveExecutedCommandBuffer(*recorded_context, tag_range.begin);
}

void CommandBufferAccessContext::ResolveExecutedCommandBuffer(const AccessContext &recorded_context, ResourceUsageTag offset) {
    auto tag_offset = [offset](ResourceAccessState *access) { access->OffsetTag(offset); };
    GetCurrentAccessContext()->ResolveFromContext(tag_offset, recorded_context);
}

ResourceUsageRange CommandExecutionContext::ImportRecordedAccessLog(const CommandBufferAccessContext &recorded_context) {
    // The execution references ensure lifespan for the referenced child CB's...
    ResourceUsageRange tag_range(GetTagLimit(), 0);
    InsertRecordedAccessLogEntries(recorded_context);
    tag_range.end = GetTagLimit();
    return tag_range;
}

void CommandBufferAccessContext::InsertRecordedAccessLogEntries(const CommandBufferAccessContext &recorded_context) {
    cbs_referenced_->emplace(recorded_context.GetCBStateShared());
    access_log_->insert(access_log_->end(), recorded_context.access_log_->cbegin(), recorded_context.access_log_->cend());
}

ResourceUsageTag CommandBufferAccessContext::NextSubcommandTag(vvl::Func command, ResourceUsageRecord::SubcommandType subcommand) {
    return NextSubcommandTag(command, NamedHandle(), subcommand);
}
ResourceUsageTag CommandBufferAccessContext::NextSubcommandTag(vvl::Func command, NamedHandle &&handle,
                                                               ResourceUsageRecord::SubcommandType subcommand) {
    ResourceUsageTag next = access_log_->size();
    access_log_->emplace_back(command, command_number_, subcommand, ++subcommand_number_, cb_state_, reset_count_);
    if (command_handles_.size()) {
        // This is a duplication, but it keeps tags->log information flat (i.e not depending on some "command tag" entry
        access_log_->back().handles = command_handles_;
    }
    if (handle) {
        access_log_->back().AddHandle(std::move(handle));
    }
    return next;
}

ResourceUsageTag CommandBufferAccessContext::NextCommandTag(vvl::Func command, ResourceUsageRecord::SubcommandType subcommand) {
    return NextCommandTag(command, NamedHandle(), subcommand);
}

ResourceUsageTag CommandBufferAccessContext::NextCommandTag(vvl::Func command, NamedHandle &&handle,
                                                            ResourceUsageRecord::SubcommandType subcommand) {
    command_number_++;
    command_handles_.clear();
    subcommand_number_ = 0;
    ResourceUsageTag next = access_log_->size();
    access_log_->emplace_back(command, command_number_, subcommand, subcommand_number_, cb_state_, reset_count_);
    if (handle) {
        access_log_->back().AddHandle(handle);
        command_handles_.emplace_back(std::move(handle));
    }
    return next;
}

ResourceUsageTag CommandBufferAccessContext::NextIndexedCommandTag(vvl::Func command, uint32_t index) {
    if (index == 0) {
        return NextCommandTag(command, ResourceUsageRecord::SubcommandType::kIndex);
    }
    return NextSubcommandTag(command, ResourceUsageRecord::SubcommandType::kIndex);
}

void CommandBufferAccessContext::RecordSyncOp(SyncOpPointer &&sync_op) {
    auto tag = sync_op->Record(this);
    // As renderpass operations can have side effects on the command buffer access context,
    // update the sync operation to record these if any.
    sync_ops_.emplace_back(tag, std::move(sync_op));
}

class HazardDetectFirstUse {
  public:
    HazardDetectFirstUse(const ResourceAccessState &recorded_use, QueueId queue_id, const ResourceUsageRange &tag_range)
        : recorded_use_(recorded_use), queue_id_(queue_id), tag_range_(tag_range) {}
    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos) const {
        return pos->second.DetectHazard(recorded_use_, queue_id_, tag_range_);
    }
    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos, ResourceUsageTag start_tag) const {
        return pos->second.DetectAsyncHazard(recorded_use_, tag_range_, start_tag);
    }

  private:
    const ResourceAccessState &recorded_use_;
    const QueueId queue_id_;
    const ResourceUsageRange &tag_range_;
};

// This is called with the *recorded* command buffers access context, with the *active* access context pass in, againsts which
// hazards will be detected
HazardResult AccessContext::DetectFirstUseHazard(QueueId queue_id, const ResourceUsageRange &tag_range,
                                                 const AccessContext &access_context) const {
    HazardResult hazard;
    for (const auto &recorded_access : access_state_map_) {
        // Cull any entries not in the current tag range
        if (!recorded_access.second.FirstAccessInTagRange(tag_range)) continue;
        HazardDetectFirstUse detector(recorded_access.second, queue_id, tag_range);
        hazard = access_context.DetectHazard(detector, recorded_access.first, DetectOptions::kDetectAll);
        if (hazard.IsHazard()) break;
    }

    return hazard;
}

bool RenderPassAccessContext::ValidateDrawSubpassAttachment(const CommandExecutionContext &exec_context,
                                                            const CMD_BUFFER_STATE &cmd_buffer, vvl::Func command) const {
    bool skip = false;
    const auto &sync_state = exec_context.GetSyncState();
    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    const auto last_bound_state = cmd_buffer.lastBound[lv_bind_point];
    const auto *pipe = last_bound_state.pipeline_state;
    if (!pipe) {
        return skip;
    }

    const auto raster_state = pipe->RasterizationState();
    if (raster_state && raster_state->rasterizerDiscardEnable) {
        return skip;
    }
    const char *caller_name = vvl::String(command);
    const auto &list = pipe->fragmentShader_writable_output_location_list;
    const auto &subpass = rp_state_->createInfo.pSubpasses[current_subpass_];

    const auto &current_context = CurrentContext();
    // Subpass's inputAttachment has been done in ValidateDispatchDrawDescriptorSet
    if (subpass.pColorAttachments && subpass.colorAttachmentCount && !list.empty()) {
        for (const auto location : list) {
            if (location >= subpass.colorAttachmentCount ||
                subpass.pColorAttachments[location].attachment == VK_ATTACHMENT_UNUSED) {
                continue;
            }
            const AttachmentViewGen &view_gen = attachment_views_[subpass.pColorAttachments[location].attachment];
            if (!view_gen.IsValid()) continue;
            HazardResult hazard =
                current_context.DetectHazard(view_gen, AttachmentViewGen::Gen::kRenderArea,
                                             SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, SyncOrdering::kColorAttachment);
            if (hazard.IsHazard()) {
                const VkImageView view_handle = view_gen.GetViewState()->image_view();
                skip |=
                    sync_state.LogError(view_handle, string_SyncHazardVUID(hazard.Hazard()),
                                        "%s: Hazard %s for %s in %s, Subpass #%d, and pColorAttachments #%d. Access info %s.",
                                        caller_name, string_SyncHazard(hazard.Hazard()),
                                        sync_state.FormatHandle(view_handle).c_str(), sync_state.FormatHandle(cmd_buffer).c_str(),
                                        cmd_buffer.GetActiveSubpass(), location, exec_context.FormatHazard(hazard).c_str());
            }
        }
    }

    // PHASE1 TODO: Add layout based read/vs. write selection.
    // PHASE1 TODO: Read operations for both depth and stencil are possible in the future.
    const auto ds_state = pipe->DepthStencilState();
    const uint32_t depth_stencil_attachment = GetSubpassDepthStencilAttachmentIndex(ds_state, subpass.pDepthStencilAttachment);

    if ((depth_stencil_attachment != VK_ATTACHMENT_UNUSED) && attachment_views_[depth_stencil_attachment].IsValid()) {
        const AttachmentViewGen &view_gen = attachment_views_[depth_stencil_attachment];
        const IMAGE_VIEW_STATE &view_state = *view_gen.GetViewState();
        bool depth_write = false, stencil_write = false;

        const bool depth_write_enable = last_bound_state.IsDepthWriteEnable();  // implicitly means DepthTestEnable is set
        const bool stencil_test_enable = last_bound_state.IsStencilTestEnable();

        // PHASE1 TODO: These validation should be in core_checks.
        if (!vkuFormatIsStencilOnly(view_state.create_info.format) && depth_write_enable &&
            IsImageLayoutDepthWritable(subpass.pDepthStencilAttachment->layout)) {
            depth_write = true;
        }
        // PHASE1 TODO: It needs to check if stencil is writable.
        //              If failOp, passOp, or depthFailOp are not KEEP, and writeMask isn't 0, it's writable.
        //              If depth test is disable, it's considered depth test passes, and then depthFailOp doesn't run.
        // PHASE1 TODO: These validation should be in core_checks.
        if (!vkuFormatIsDepthOnly(view_state.create_info.format) && stencil_test_enable &&
            IsImageLayoutStencilWritable(subpass.pDepthStencilAttachment->layout)) {
            stencil_write = true;
        }

        // PHASE1 TODO: Add EARLY stage detection based on ExecutionMode.
        if (depth_write) {
            HazardResult hazard = current_context.DetectHazard(view_gen, AttachmentViewGen::Gen::kDepthOnlyRenderArea,
                                                               SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                                               SyncOrdering::kDepthStencilAttachment);
            if (hazard.IsHazard()) {
                skip |= sync_state.LogError(
                    view_state.image_view(), string_SyncHazardVUID(hazard.Hazard()),
                    "%s: Hazard %s for %s in %s, Subpass #%d, and depth part of pDepthStencilAttachment. Access info %s.",
                    caller_name, string_SyncHazard(hazard.Hazard()), sync_state.FormatHandle(view_state).c_str(),
                    sync_state.FormatHandle(cmd_buffer).c_str(), cmd_buffer.GetActiveSubpass(),
                    exec_context.FormatHazard(hazard).c_str());
            }
        }
        if (stencil_write) {
            HazardResult hazard = current_context.DetectHazard(view_gen, AttachmentViewGen::Gen::kStencilOnlyRenderArea,
                                                               SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                                               SyncOrdering::kDepthStencilAttachment);
            if (hazard.IsHazard()) {
                skip |= sync_state.LogError(
                    view_state.image_view(), string_SyncHazardVUID(hazard.Hazard()),
                    "%s: Hazard %s for %s in %s, Subpass #%d, and stencil part of pDepthStencilAttachment. Access info %s.",
                    caller_name, string_SyncHazard(hazard.Hazard()), sync_state.FormatHandle(view_state).c_str(),
                    sync_state.FormatHandle(cmd_buffer).c_str(), cmd_buffer.GetActiveSubpass(),
                    exec_context.FormatHazard(hazard).c_str());
            }
        }
    }
    return skip;
}

void RenderPassAccessContext::RecordDrawSubpassAttachment(const CMD_BUFFER_STATE &cmd_buffer, const ResourceUsageTag tag) {
    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    const auto last_bound_state = cmd_buffer.lastBound[lv_bind_point];
    const auto *pipe = last_bound_state.pipeline_state;
    if (!pipe) {
        return;
    }

    const auto *raster_state = pipe->RasterizationState();
    if (raster_state && raster_state->rasterizerDiscardEnable) {
        return;
    }
    const auto &list = pipe->fragmentShader_writable_output_location_list;
    const auto &subpass = rp_state_->createInfo.pSubpasses[current_subpass_];

    auto &current_context = CurrentContext();
    // Subpass's inputAttachment has been done in RecordDispatchDrawDescriptorSet
    if (subpass.pColorAttachments && subpass.colorAttachmentCount && !list.empty()) {
        for (const auto location : list) {
            if (location >= subpass.colorAttachmentCount ||
                subpass.pColorAttachments[location].attachment == VK_ATTACHMENT_UNUSED) {
                continue;
            }
            const AttachmentViewGen &view_gen = attachment_views_[subpass.pColorAttachments[location].attachment];
            current_context.UpdateAccessState(view_gen, AttachmentViewGen::Gen::kRenderArea,
                                              SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, SyncOrdering::kColorAttachment,
                                              tag);
        }
    }

    // PHASE1 TODO: Add layout based read/vs. write selection.
    // PHASE1 TODO: Read operations for both depth and stencil are possible in the future.
    const auto *ds_state = pipe->DepthStencilState();
    const uint32_t depth_stencil_attachment = GetSubpassDepthStencilAttachmentIndex(ds_state, subpass.pDepthStencilAttachment);
    if ((depth_stencil_attachment != VK_ATTACHMENT_UNUSED) && attachment_views_[depth_stencil_attachment].IsValid()) {
        const AttachmentViewGen &view_gen = attachment_views_[depth_stencil_attachment];
        const IMAGE_VIEW_STATE &view_state = *view_gen.GetViewState();
        bool depth_write = false, stencil_write = false;
        const bool has_depth = 0 != (view_state.normalized_subresource_range.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT);
        const bool has_stencil = 0 != (view_state.normalized_subresource_range.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT);

        const bool depth_write_enable = last_bound_state.IsDepthWriteEnable();  // implicitly means DepthTestEnable is set
        const bool stencil_test_enable = last_bound_state.IsStencilTestEnable();

        // PHASE1 TODO: These validation should be in core_checks.
        if (has_depth && !vkuFormatIsStencilOnly(view_state.create_info.format) && depth_write_enable &&
            IsImageLayoutDepthWritable(subpass.pDepthStencilAttachment->layout)) {
            depth_write = true;
        }
        // PHASE1 TODO: It needs to check if stencil is writable.
        //              If failOp, passOp, or depthFailOp are not KEEP, and writeMask isn't 0, it's writable.
        //              If depth test is disable, it's considered depth test passes, and then depthFailOp doesn't run.
        // PHASE1 TODO: These validation should be in core_checks.
        if (has_stencil && !vkuFormatIsDepthOnly(view_state.create_info.format) && stencil_test_enable &&
            IsImageLayoutStencilWritable(subpass.pDepthStencilAttachment->layout)) {
            stencil_write = true;
        }

        if (depth_write || stencil_write) {
            const auto ds_gentype = view_gen.GetDepthStencilRenderAreaGenType(depth_write, stencil_write);
            // PHASE1 TODO: Add EARLY stage detection based on ExecutionMode.
            current_context.UpdateAccessState(view_gen, ds_gentype, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                              SyncOrdering::kDepthStencilAttachment, tag);
        }
    }
}

static constexpr VkImageAspectFlags kColorAspects =
    VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT;
static constexpr VkImageAspectFlags kDepthStencilAspects = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

static std::optional<uint32_t> GetAttachmentIndex(const RenderPassAccessContext &rp_context,
                                                  const VkClearAttachment &clear_attachment) {
    const auto &rpci = rp_context.GetRenderPassState()->createInfo;
    const auto &subpass = rpci.pSubpasses[rp_context.GetCurrentSubpass()];
    uint32_t attachment_index = VK_ATTACHMENT_UNUSED;

    if (clear_attachment.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
        if (clear_attachment.colorAttachment < subpass.colorAttachmentCount) {
            attachment_index = subpass.pColorAttachments[clear_attachment.colorAttachment].attachment;
        }
    } else if (clear_attachment.aspectMask & kDepthStencilAspects) {
        if (subpass.pDepthStencilAttachment) {
            attachment_index = subpass.pDepthStencilAttachment->attachment;
        }
    }
    const bool invalid_index = (attachment_index == VK_ATTACHMENT_UNUSED || attachment_index >= rpci.attachmentCount);
    return invalid_index ? std::optional<uint32_t>() : attachment_index;
}

static VkImageAspectFlags GetAspectsToClear(VkImageAspectFlags clear_aspect_mask, VkImageAspectFlags view_aspect_mask) {
    // Check if clear request is valid.
    const bool clear_color = (clear_aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) != 0;
    const bool clear_depth = (clear_aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0;
    const bool clear_stencil = (clear_aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) != 0;
    if (!clear_color && !clear_depth && !clear_stencil) {
        return 0;  // nothing to clear
    }
    if (clear_color && (clear_depth || clear_stencil)) {
        return 0;  // according to spec it's not allowed
    }

    // Collect aspects that should be cleared.
    VkImageAspectFlags aspects_to_clear = VK_IMAGE_ASPECT_NONE;
    if (clear_color && (view_aspect_mask & kColorAspects) != 0) {
        assert(GetBitSetCount(view_aspect_mask) == 1);
        aspects_to_clear |= view_aspect_mask;
    }
    if (clear_depth && (view_aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0) {
        aspects_to_clear |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    if (clear_stencil && (view_aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) != 0) {
        aspects_to_clear |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    return aspects_to_clear;
}

static std::optional<VkImageSubresourceRange> RestrictSubresourceRange(const VkImageSubresourceRange &normalized_subresource_range,
                                                                       const VkClearRect &clear_rect) {
    assert(normalized_subresource_range.layerCount != VK_REMAINING_ARRAY_LAYERS);  // contract of this function
    assert(clear_rect.layerCount != VK_REMAINING_ARRAY_LAYERS);                    // according to spec
    const uint32_t first = std::max(normalized_subresource_range.baseArrayLayer, clear_rect.baseArrayLayer);
    const uint32_t last_range = normalized_subresource_range.baseArrayLayer + normalized_subresource_range.layerCount;
    const uint32_t last_clear = clear_rect.baseArrayLayer + clear_rect.layerCount;
    const uint32_t last = std::min(last_range, last_clear);
    std::optional<VkImageSubresourceRange> result;
    if (first < last) {
        result = normalized_subresource_range;
        result->baseArrayLayer = first;
        result->layerCount = last - first;
    }
    return result;
}

std::optional<RenderPassAccessContext::ClearAttachmentInfo> RenderPassAccessContext::GetClearAttachmentInfo(
    const VkClearAttachment &clear_attachment, const VkClearRect &rect) const {
    const auto attachment_index = GetAttachmentIndex(*this, clear_attachment);
    if (!attachment_index) {
        return {};
    }
    const auto &view_subresource_range = attachment_views_[attachment_index.value()].GetViewState()->normalized_subresource_range;
    const auto aspects = GetAspectsToClear(clear_attachment.aspectMask, view_subresource_range.aspectMask);
    if (!aspects) {
        return {};
    }
    const auto subresource_range = RestrictSubresourceRange(view_subresource_range, rect);
    if (!subresource_range) {
        return {};
    }
    return ClearAttachmentInfo{attachment_index.value(), aspects, subresource_range.value()};
}

bool RenderPassAccessContext::ValidateClearAttachment(const CommandExecutionContext &exec_context,
                                                      const CMD_BUFFER_STATE &cmd_buffer, const Location &loc,
                                                      const VkClearAttachment &clear_attachment, const VkClearRect &rect,
                                                      uint32_t rect_index) const {
    const auto info = GetClearAttachmentInfo(clear_attachment, rect);
    if (!info) {
        return false;
    }
    const auto &view_state = *attachment_views_[info->attachment_index].GetViewState();
    const VkOffset3D offset = CastTo3D(rect.rect.offset);
    const VkExtent3D extent = CastTo3D(rect.rect.extent);
    auto subresource_range = info->subresource_range;
    bool skip = false;

    if (info->aspects_to_clear & kColorAspects) {
        assert(GetBitSetCount(info->aspects_to_clear) == 1);
        subresource_range.aspectMask = info->aspects_to_clear;

        HazardResult hazard = CurrentContext().DetectHazard(
            *view_state.GetImageState(), SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, subresource_range,
            SyncOrdering::kColorAttachment, offset, extent, view_state.IsDepthSliced());
        if (hazard.IsHazard()) {
            const LogObjectList objlist(cmd_buffer.commandBuffer(), view_state.image_view());
            skip |= exec_context.GetSyncState().LogError(string_SyncHazardVUID(hazard.Hazard()), objlist, loc,
                                                         "Hazard %s when clearing pRects[%" PRIu32
                                                         "] region of color attachment %" PRIu32 " in subpass %" PRIu32
                                                         ". Access info %s.",
                                                         string_SyncHazard(hazard.Hazard()), rect_index, info->attachment_index,
                                                         cmd_buffer.GetActiveSubpass(), exec_context.FormatHazard(hazard).c_str());
        }
    }

    constexpr VkImageAspectFlagBits depth_stencil_aspects[2] = {VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_ASPECT_STENCIL_BIT};
    for (const auto aspect : depth_stencil_aspects) {
        if (info->aspects_to_clear & aspect) {
            // Original aspect mask can contain both stencil and depth but here we track each aspect separately
            subresource_range.aspectMask = aspect;

            // vkCmdClearAttachments depth/stencil writes are executed by the EARLY_FRAGMENT_TESTS_BIT and LATE_FRAGMENT_TESTS_BIT
            // stages. The implementation tracks the most recent access, which happens in the LATE_FRAGMENT_TESTS_BIT stage.
            HazardResult hazard = CurrentContext().DetectHazard(
                *view_state.GetImageState(), SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, subresource_range,
                SyncOrdering::kDepthStencilAttachment, offset, extent, view_state.IsDepthSliced());

            if (hazard.IsHazard()) {
                const LogObjectList objlist(cmd_buffer.commandBuffer(), view_state.image_view());
                skip |= exec_context.GetSyncState().LogError(
                    string_SyncHazardVUID(hazard.Hazard()), objlist, loc,
                    "Hazard %s when clearing pRects[%" PRIu32 "] region of %s aspect of depth-stencil attachment %" PRIu32
                    " in subpass %" PRIu32 ". Access info %s.",
                    string_SyncHazard(hazard.Hazard()), rect_index, string_VkImageAspectFlagBits(aspect), info->attachment_index,
                    cmd_buffer.GetActiveSubpass(), exec_context.FormatHazard(hazard).c_str());
            }
        }
    }
    return skip;
}

void RenderPassAccessContext::RecordClearAttachment(const CMD_BUFFER_STATE &cmd_buffer, ResourceUsageTag tag,
                                                    const VkClearAttachment &clear_attachment, const VkClearRect &rect) {
    const auto info = GetClearAttachmentInfo(clear_attachment, rect);
    if (!info) {
        return;
    }
    const auto &view_state = *attachment_views_[info->attachment_index].GetViewState();
    const VkOffset3D offset = CastTo3D(rect.rect.offset);
    const VkExtent3D extent = CastTo3D(rect.rect.extent);
    auto subresource_range = info->subresource_range;

    // Original subresource range can include aspects that are not cleared, they should not be tracked
    subresource_range.aspectMask = info->aspects_to_clear;

    if (info->aspects_to_clear & kColorAspects) {
        assert((info->aspects_to_clear & kDepthStencilAspects) == 0);
        CurrentContext().UpdateAccessState(*view_state.GetImageState(), SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE,
                                           SyncOrdering::kColorAttachment, subresource_range, offset, extent, tag);
    } else {
        assert((info->aspects_to_clear & kColorAspects) == 0);
        CurrentContext().UpdateAccessState(*view_state.GetImageState(), SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                           SyncOrdering::kDepthStencilAttachment, subresource_range, offset, extent, tag);
    }
}

bool RenderPassAccessContext::ValidateNextSubpass(const CommandExecutionContext &exec_context, vvl::Func command) const {
    // PHASE1 TODO: Add Validate Preserve attachments
    bool skip = false;
    skip |= CurrentContext().ValidateResolveOperations(exec_context, *rp_state_, render_area_, attachment_views_, command,
                                                       current_subpass_);
    skip |= CurrentContext().ValidateStoreOperation(exec_context, *rp_state_, render_area_, current_subpass_, attachment_views_,
                                                    command);

    const auto next_subpass = current_subpass_ + 1;
    if (next_subpass >= subpass_contexts_.size()) {
        return skip;
    }
    const auto &next_context = subpass_contexts_[next_subpass];
    skip |=
        next_context.ValidateLayoutTransitions(exec_context, *rp_state_, render_area_, next_subpass, attachment_views_, command);
    if (!skip) {
        // To avoid complex (and buggy) duplication of the affect of layout transitions on load operations, we'll record them
        // on a copy of the (empty) next context.
        // Note: The resource access map should be empty so hopefully this copy isn't too horrible from a perf POV.
        AccessContext temp_context(next_context);
        temp_context.RecordLayoutTransitions(*rp_state_, next_subpass, attachment_views_, kInvalidTag);
        skip |=
            temp_context.ValidateLoadOperation(exec_context, *rp_state_, render_area_, next_subpass, attachment_views_, command);
    }
    return skip;
}
bool RenderPassAccessContext::ValidateEndRenderPass(const CommandExecutionContext &exec_context, vvl::Func command) const {
    // PHASE1 TODO: Validate Preserve
    bool skip = false;
    skip |= CurrentContext().ValidateResolveOperations(exec_context, *rp_state_, render_area_, attachment_views_, command,
                                                       current_subpass_);
    skip |= CurrentContext().ValidateStoreOperation(exec_context, *rp_state_, render_area_, current_subpass_, attachment_views_,
                                                    command);
    skip |= ValidateFinalSubpassLayoutTransitions(exec_context, command);
    return skip;
}

AccessContext *RenderPassAccessContext::CreateStoreResolveProxy() const {
    return CreateStoreResolveProxyContext(CurrentContext(), *rp_state_, current_subpass_, attachment_views_);
}

bool RenderPassAccessContext::ValidateFinalSubpassLayoutTransitions(const CommandExecutionContext &exec_context,
                                                                    vvl::Func command) const {
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
        const auto &view_gen = attachment_views_[transition.attachment];
        const auto &trackback = subpass_contexts_[transition.prev_pass].GetDstExternalTrackBack();
        assert(trackback.source_subpass);  // Transitions are given implicit transitions if the StateTracker is working correctly
        auto *context = trackback.source_subpass;

        if (transition.prev_pass == current_subpass_) {
            if (!proxy_for_current) {
                // We haven't recorded resolve ofor the current_subpass, so we need to copy current and update it *as if*
                proxy_for_current.reset(CreateStoreResolveProxy());
            }
            context = proxy_for_current.get();
        }

        // Use the merged barrier for the hazard check (safe since it just considers the src (first) scope.
        const auto merged_barrier = MergeBarriers(trackback.barriers);
        auto hazard = context->DetectImageBarrierHazard(view_gen, merged_barrier, AccessContext::DetectOptions::kDetectPrevious);
        if (hazard.IsHazard()) {
            if (hazard.Tag() == kInvalidTag) {
                // Hazard vs. ILT
                skip |= exec_context.GetSyncState().LogError(
                    rp_state_->renderPass(), string_SyncHazardVUID(hazard.Hazard()),
                    "%s: Hazard %s vs. store/resolve operations in subpass %" PRIu32 " for attachment %" PRIu32
                    " final image layout transition (old_layout: %s, new_layout: %s).",
                    vvl::String(command), string_SyncHazard(hazard.Hazard()), transition.prev_pass, transition.attachment,
                    string_VkImageLayout(transition.old_layout), string_VkImageLayout(transition.new_layout));
            } else {
                skip |= exec_context.GetSyncState().LogError(
                    rp_state_->renderPass(), string_SyncHazardVUID(hazard.Hazard()),
                    "%s: Hazard %s with last use subpass %" PRIu32 " for attachment %" PRIu32
                    " final image layout transition (old_layout: %s, new_layout: %s). Access info %s.",
                    vvl::String(command), string_SyncHazard(hazard.Hazard()), transition.prev_pass, transition.attachment,
                    string_VkImageLayout(transition.old_layout), string_VkImageLayout(transition.new_layout),
                    exec_context.FormatHazard(hazard).c_str());
            }
        }
    }
    return skip;
}

void RenderPassAccessContext::RecordLayoutTransitions(const ResourceUsageTag tag) {
    // Add layout transitions...
    subpass_contexts_[current_subpass_].RecordLayoutTransitions(*rp_state_, current_subpass_, attachment_views_, tag);
}

void RenderPassAccessContext::RecordLoadOperations(const ResourceUsageTag tag) {
    const auto *attachment_ci = rp_state_->createInfo.pAttachments;
    auto &subpass_context = subpass_contexts_[current_subpass_];

    for (uint32_t i = 0; i < rp_state_->createInfo.attachmentCount; i++) {
        if (rp_state_->attachment_first_subpass[i] == current_subpass_) {
            const AttachmentViewGen &view_gen = attachment_views_[i];
            if (!view_gen.IsValid()) continue;  // UNUSED

            const auto &ci = attachment_ci[i];
            const bool has_depth = vkuFormatHasDepth(ci.format);
            const bool has_stencil = vkuFormatHasStencil(ci.format);
            const bool is_color = !(has_depth || has_stencil);

            if (is_color) {
                const SyncStageAccessIndex load_op = ColorLoadUsage(ci.loadOp);
                if (load_op != SYNC_ACCESS_INDEX_NONE) {
                    subpass_context.UpdateAccessState(view_gen, AttachmentViewGen::Gen::kRenderArea, load_op,
                                                      SyncOrdering::kColorAttachment, tag);
                }
            } else {
                if (has_depth) {
                    const SyncStageAccessIndex load_op = DepthStencilLoadUsage(ci.loadOp);
                    if (load_op != SYNC_ACCESS_INDEX_NONE) {
                        subpass_context.UpdateAccessState(view_gen, AttachmentViewGen::Gen::kDepthOnlyRenderArea, load_op,
                                                          SyncOrdering::kDepthStencilAttachment, tag);
                    }
                }
                if (has_stencil) {
                    const SyncStageAccessIndex load_op = DepthStencilLoadUsage(ci.stencilLoadOp);
                    if (load_op != SYNC_ACCESS_INDEX_NONE) {
                        subpass_context.UpdateAccessState(view_gen, AttachmentViewGen::Gen::kStencilOnlyRenderArea, load_op,
                                                          SyncOrdering::kDepthStencilAttachment, tag);
                    }
                }
            }
        }
    }
}
AttachmentViewGenVector RenderPassAccessContext::CreateAttachmentViewGen(
    const VkRect2D &render_area, const std::vector<const syncval_state::ImageViewState *> &attachment_views) {
    AttachmentViewGenVector view_gens;
    VkExtent3D extent = CastTo3D(render_area.extent);
    VkOffset3D offset = CastTo3D(render_area.offset);
    view_gens.reserve(attachment_views.size());
    for (const auto *view : attachment_views) {
        view_gens.emplace_back(view, offset, extent);
    }
    return view_gens;
}
RenderPassAccessContext::RenderPassAccessContext(const RENDER_PASS_STATE &rp_state, const VkRect2D &render_area,
                                                 VkQueueFlags queue_flags,
                                                 const std::vector<const syncval_state::ImageViewState *> &attachment_views,
                                                 const AccessContext *external_context)
    : rp_state_(&rp_state), render_area_(render_area), current_subpass_(0U), attachment_views_() {
    // Add this for all subpasses here so that they exist during next subpass validation
    InitSubpassContexts(queue_flags, rp_state, external_context, subpass_contexts_);
    attachment_views_ = CreateAttachmentViewGen(render_area, attachment_views);
}
void RenderPassAccessContext::RecordBeginRenderPass(const ResourceUsageTag barrier_tag, const ResourceUsageTag load_tag) {
    assert(0 == current_subpass_);
    AccessContext &current_context = subpass_contexts_[current_subpass_];
    current_context.SetStartTag(barrier_tag);

    RecordLayoutTransitions(barrier_tag);
    RecordLoadOperations(load_tag);
}

void RenderPassAccessContext::RecordNextSubpass(const ResourceUsageTag store_tag, const ResourceUsageTag barrier_tag,
                                                const ResourceUsageTag load_tag) {
    // Resolves are against *prior* subpass context and thus *before* the subpass increment
    CurrentContext().UpdateAttachmentResolveAccess(*rp_state_, attachment_views_, current_subpass_, store_tag);
    CurrentContext().UpdateAttachmentStoreAccess(*rp_state_, attachment_views_, current_subpass_, store_tag);

    if (current_subpass_ + 1 >= subpass_contexts_.size()) {
        return;
    }
    // Move to the next sub-command for the new subpass. The resolve and store are logically part of the previous
    // subpass, so their tag needs to be different from the layout and load operations below.
    current_subpass_++;
    AccessContext &current_context = subpass_contexts_[current_subpass_];
    current_context.SetStartTag(barrier_tag);

    RecordLayoutTransitions(barrier_tag);
    RecordLoadOperations(load_tag);
}

void RenderPassAccessContext::RecordEndRenderPass(AccessContext *external_context, const ResourceUsageTag store_tag,
                                                  const ResourceUsageTag barrier_tag) {
    // Add the resolve and store accesses
    CurrentContext().UpdateAttachmentResolveAccess(*rp_state_, attachment_views_, current_subpass_, store_tag);
    CurrentContext().UpdateAttachmentStoreAccess(*rp_state_, attachment_views_, current_subpass_, store_tag);

    // Export the accesses from the renderpass...
    external_context->ResolveChildContexts(subpass_contexts_);

    // Add the "finalLayout" transitions to external
    // Get them from where there we're hidding in the extra entry.
    // Not that since *final* always comes from *one* subpass per view, we don't have to accumulate the barriers
    // TODO Aliasing we may need to reconsider barrier accumulation... though I don't know that it would be valid for aliasing
    //      that had mulitple final layout transistions from mulitple final subpasses.
    const auto &final_transitions = rp_state_->subpass_transitions.back();
    for (const auto &transition : final_transitions) {
        const AttachmentViewGen &view_gen = attachment_views_[transition.attachment];
        const auto &last_trackback = subpass_contexts_[transition.prev_pass].GetDstExternalTrackBack();
        assert(&subpass_contexts_[transition.prev_pass] == last_trackback.source_subpass);
        ApplyBarrierOpsFunctor<PipelineBarrierOp> barrier_action(true /* resolve */, last_trackback.barriers.size(), barrier_tag);
        for (const auto &barrier : last_trackback.barriers) {
            barrier_action.EmplaceBack(PipelineBarrierOp(QueueSyncState::kQueueIdInvalid, barrier, true));
        }
        external_context->ApplyUpdateAction(view_gen, AttachmentViewGen::Gen::kViewSubresource, barrier_action);
    }
}

SyncExecScope SyncExecScope::MakeSrc(VkQueueFlags queue_flags, VkPipelineStageFlags2KHR mask_param,
                                     const VkPipelineStageFlags2KHR disabled_feature_mask) {
    SyncExecScope result;
    result.mask_param = mask_param;
    result.expanded_mask = sync_utils::ExpandPipelineStages(mask_param, queue_flags, disabled_feature_mask);
    result.exec_scope = sync_utils::WithEarlierPipelineStages(result.expanded_mask);
    result.valid_accesses = SyncStageAccess::AccessScopeByStage(result.expanded_mask);
    return result;
}

SyncExecScope SyncExecScope::MakeDst(VkQueueFlags queue_flags, VkPipelineStageFlags2KHR mask_param) {
    SyncExecScope result;
    result.mask_param = mask_param;
    result.expanded_mask = sync_utils::ExpandPipelineStages(mask_param, queue_flags);
    result.exec_scope = sync_utils::WithLaterPipelineStages(result.expanded_mask);
    result.valid_accesses = SyncStageAccess::AccessScopeByStage(result.expanded_mask);
    return result;
}

SyncBarrier::SyncBarrier(const SyncExecScope &src, const SyncExecScope &dst)
    : src_exec_scope(src), src_access_scope(0), dst_exec_scope(dst), dst_access_scope(0) {}

SyncBarrier::SyncBarrier(const SyncExecScope &src, const SyncExecScope &dst, const SyncBarrier::AllAccess &)
    : src_exec_scope(src), src_access_scope(src.valid_accesses), dst_exec_scope(dst), dst_access_scope(dst.valid_accesses) {}

template <typename Barrier>
SyncBarrier::SyncBarrier(const Barrier &barrier, const SyncExecScope &src, const SyncExecScope &dst)
    : src_exec_scope(src),
      src_access_scope(SyncStageAccess::AccessScope(src.valid_accesses, barrier.srcAccessMask)),
      dst_exec_scope(dst),
      dst_access_scope(SyncStageAccess::AccessScope(dst.valid_accesses, barrier.dstAccessMask)) {}

SyncBarrier::SyncBarrier(VkQueueFlags queue_flags, const VkSubpassDependency2 &subpass) {
    const auto barrier = vku::FindStructInPNextChain<VkMemoryBarrier2KHR>(subpass.pNext);
    if (barrier) {
        auto src = SyncExecScope::MakeSrc(queue_flags, barrier->srcStageMask);
        src_exec_scope = src;
        src_access_scope = SyncStageAccess::AccessScope(src.valid_accesses, barrier->srcAccessMask);

        auto dst = SyncExecScope::MakeDst(queue_flags, barrier->dstStageMask);
        dst_exec_scope = dst;
        dst_access_scope = SyncStageAccess::AccessScope(dst.valid_accesses, barrier->dstAccessMask);

    } else {
        auto src = SyncExecScope::MakeSrc(queue_flags, subpass.srcStageMask);
        src_exec_scope = src;
        src_access_scope = SyncStageAccess::AccessScope(src.valid_accesses, subpass.srcAccessMask);

        auto dst = SyncExecScope::MakeDst(queue_flags, subpass.dstStageMask);
        dst_exec_scope = dst;
        dst_access_scope = SyncStageAccess::AccessScope(dst.valid_accesses, subpass.dstAccessMask);
    }
}

template <typename Barrier>
SyncBarrier::SyncBarrier(VkQueueFlags queue_flags, const Barrier &barrier) {
    auto src = SyncExecScope::MakeSrc(queue_flags, barrier.srcStageMask);
    src_exec_scope = src.exec_scope;
    src_access_scope = SyncStageAccess::AccessScope(src.valid_accesses, barrier.srcAccessMask);

    auto dst = SyncExecScope::MakeDst(queue_flags, barrier.dstStageMask);
    dst_exec_scope = dst.exec_scope;
    dst_access_scope = SyncStageAccess::AccessScope(dst.valid_accesses, barrier.dstAccessMask);
}

// Apply a list of barriers, without resolving pending state, useful for subpass layout transitions
void ResourceAccessState::ApplyBarriers(const std::vector<SyncBarrier> &barriers, bool layout_transition) {
    const UntaggedScopeOps scope;
    for (const auto &barrier : barriers) {
        ApplyBarrier(scope, barrier, layout_transition);
    }
}

// ApplyBarriers is design for *fully* inclusive barrier lists without layout tranistions.  Designed use was for
// inter-subpass barriers for lazy-evaluation of parent context memory ranges.  Subpass layout transistions are *not* done
// lazily, s.t. no previous access reports should need layout transitions.
void ResourceAccessState::ApplyBarriersImmediate(const std::vector<SyncBarrier> &barriers) {
    assert(!HasPendingState());  // This should never be call in the middle of another barrier application
    const UntaggedScopeOps scope;
    for (const auto &barrier : barriers) {
        ApplyBarrier(scope, barrier, false);
    }
    ApplyPendingBarriers(kInvalidTag);  // There can't be any need for this tag
}
HazardResult ResourceAccessState::DetectHazard(const SyncStageAccessInfoType &usage_info) const {
    HazardResult hazard;
    const auto &usage_stage = usage_info.stage_mask;
    if (IsRead(usage_info)) {
        if (IsRAWHazard(usage_info)) {
            hazard.Set(this, usage_info, READ_AFTER_WRITE, *last_write);
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
        if (last_reads.size()) {
            for (const auto &read_access : last_reads) {
                if (IsReadHazard(usage_stage, read_access)) {
                    hazard.Set(this, usage_info, WRITE_AFTER_READ, read_access.access, read_access.tag);
                    break;
                }
            }
        } else if (last_write.has_value() && last_write->IsWriteHazard(usage_info)) {
            // Write-After-Write check -- if we have a previous write to test against
            hazard.Set(this, usage_info, WRITE_AFTER_WRITE, *last_write);
        }
    }
    return hazard;
}

HazardResult ResourceAccessState::DetectHazard(const SyncStageAccessInfoType &usage_info, const SyncOrdering ordering_rule,
                                               QueueId queue_id) const {
    const auto &ordering = GetOrderingRules(ordering_rule);
    return DetectHazard(usage_info, ordering, queue_id);
}

HazardResult ResourceAccessState::DetectHazard(const SyncStageAccessInfoType &usage_info, const OrderingBarrier &ordering,
                                               QueueId queue_id) const {
    // The ordering guarantees act as barriers to the last accesses, independent of synchronization operations
    HazardResult hazard;
    const auto &usage_bit = usage_info.stage_access_bit;
    const auto &usage_stage = usage_info.stage_mask;
    const auto &usage_index = usage_info.stage_access_index;
    const bool input_attachment_ordering = ordering.access_scope[SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ];

    if (IsRead(usage_info)) {
        // Exclude RAW if no write, or write not most "most recent" operation w.r.t. usage;
        bool is_raw_hazard = IsRAWHazard(usage_info);
        if (is_raw_hazard) {
            // NOTE: we know last_write is non-zero
            // See if the ordering rules save us from the simple RAW check above
            // First check to see if the current usage is covered by the ordering rules
            const bool usage_is_input_attachment = (usage_index == SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ);
            const bool usage_is_ordered =
                (input_attachment_ordering && usage_is_input_attachment) || (0 != (usage_stage & ordering.exec_scope));
            if (usage_is_ordered) {
                // Now see of the most recent write (or a subsequent read) are ordered
                const bool most_recent_is_ordered =
                    last_write->IsOrdered(ordering, queue_id) || (0 != GetOrderedStages(queue_id, ordering));
                is_raw_hazard = !most_recent_is_ordered;
            }
        }
        if (is_raw_hazard) {
            hazard.Set(this, usage_info, READ_AFTER_WRITE, *last_write);
        }
    } else if (usage_index == SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION) {
        // For Image layout transitions, the barrier represents the first synchronization/access scope of the layout transition
        return DetectBarrierHazard(usage_info, queue_id, ordering.exec_scope, ordering.access_scope);
    } else {
        // Only check for WAW if there are no reads since last_write
        const bool usage_write_is_ordered = (usage_bit & ordering.access_scope).any();
        if (last_reads.size()) {
            // Look for any WAR hazards outside the ordered set of stages
            VkPipelineStageFlags2KHR ordered_stages = VK_PIPELINE_STAGE_2_NONE;
            if (usage_write_is_ordered) {
                // If the usage is ordered, we can ignore all ordered read stages w.r.t. WAR)
                ordered_stages = GetOrderedStages(queue_id, ordering);
            }
            // If we're tracking any reads that aren't ordered against the current write, got to check 'em all.
            if ((ordered_stages & last_read_stages) != last_read_stages) {
                for (const auto &read_access : last_reads) {
                    if (read_access.stage & ordered_stages) continue;  // but we can skip the ordered ones
                    if (IsReadHazard(usage_stage, read_access)) {
                        hazard.Set(this, usage_info, WRITE_AFTER_READ, read_access.access, read_access.tag);
                        break;
                    }
                }
            }
        } else if (last_write.has_value() && !(last_write->IsOrdered(ordering, queue_id) && usage_write_is_ordered)) {
            bool ilt_ilt_hazard = false;
            if ((usage_index == SYNC_IMAGE_LAYOUT_TRANSITION) && (last_write->IsIndex(SYNC_IMAGE_LAYOUT_TRANSITION))) {
                // ILT after ILT is a special case where we check the 2nd access scope of the first ILT against the first access
                // scope of the second ILT, which has been passed (smuggled?) in the ordering barrier
                ilt_ilt_hazard = !(last_write->Barriers() & ordering.access_scope).any();
            }
            if (ilt_ilt_hazard || last_write->IsWriteHazard(usage_info)) {
                hazard.Set(this, usage_info, WRITE_AFTER_WRITE, *last_write);
            }
        }
    }
    return hazard;
}

HazardResult ResourceAccessState::DetectHazard(const ResourceAccessState &recorded_use, QueueId queue_id,
                                               const ResourceUsageRange &tag_range) const {
    HazardResult hazard;
    using Size = FirstAccesses::size_type;
    const auto &recorded_accesses = recorded_use.first_accesses_;
    Size count = recorded_accesses.size();
    if (count) {
        // First access is only closed if the last is a write
        bool do_write_last = recorded_use.first_access_closed_;
        if (do_write_last) {
            // Note: We know count > 0 so this is alway safe.
            --count;
        }

        for (Size i = 0; i < count; ++count) {
            const auto &first = recorded_accesses[i];
            // Skip and quit logic
            if (first.tag < tag_range.begin) continue;
            if (first.tag >= tag_range.end) {
                do_write_last = false;  // ignore last since we know it can't be in tag_range
                break;
            }

            hazard = DetectHazard(*first.usage_info, first.ordering_rule, queue_id);
            if (hazard.IsHazard()) {
                hazard.AddRecordedAccess(first);
                break;
            }
        }

        if (do_write_last) {
            // Writes are a bit special... both for the "most recent" access logic, and layout transition specific logic
            const auto &last_access = recorded_accesses.back();
            if (tag_range.includes(last_access.tag)) {
                OrderingBarrier barrier = GetOrderingRules(last_access.ordering_rule);
                if (last_access.usage_info->stage_access_index == SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION) {
                    // Or in the layout first access scope as a barrier... IFF the usage is an ILT
                    // this was saved off in the "apply barriers" logic to simplify ILT access checks as they straddle
                    // the barrier that applies them
                    barrier |= recorded_use.first_write_layout_ordering_;
                }
                // Any read stages present in the recorded context (this) are most recent to the write, and thus mask those stages
                // in the active context
                if (recorded_use.first_read_stages_) {
                    // we need to ignore the first use read stage in the active context (so we add them to the ordering rule),
                    // reads in the active context are not "most recent" as all recorded context operations are *after* them
                    // This supresses only RAW checks for stages present in the recorded context, but not those only present in the
                    // active context.
                    barrier.exec_scope |= recorded_use.first_read_stages_;
                    // if there are any first use reads, we suppress WAW by injecting the active context write in the ordering rule
                    barrier.access_scope |= last_access.usage_info->stage_access_bit;
                }
                hazard = DetectHazard(*last_access.usage_info, barrier, queue_id);
                if (hazard.IsHazard()) {
                    hazard.AddRecordedAccess(last_access);
                }
            }
        }
    }
    return hazard;
}

// Asynchronous Hazards occur between subpasses with no connection through the DAG
HazardResult ResourceAccessState::DetectAsyncHazard(const SyncStageAccessInfoType &usage_info,
                                                    const ResourceUsageTag start_tag) const {
    HazardResult hazard;
    // Async checks need to not go back further than the start of the subpass, as we only want to find hazards between the async
    // subpasses.  Anything older than that should have been checked at the start of each subpass, taking into account all of
    // the raster ordering rules.
    if (IsRead(usage_info)) {
        if (last_write.has_value() && (last_write->tag_ >= start_tag)) {
            hazard.Set(this, usage_info, READ_RACING_WRITE, *last_write);
        }
    } else {
        if (last_write.has_value() && (last_write->tag_ >= start_tag)) {
            hazard.Set(this, usage_info, WRITE_RACING_WRITE, *last_write);
        } else if (last_reads.size() > 0) {
            // Any reads during the other subpass will conflict with this write, so we need to check them all.
            for (const auto &read_access : last_reads) {
                if (read_access.tag >= start_tag) {
                    hazard.Set(this, usage_info, WRITE_RACING_READ, read_access.access, read_access.tag);
                    break;
                }
            }
        }
    }
    return hazard;
}

HazardResult ResourceAccessState::DetectAsyncHazard(const ResourceAccessState &recorded_use, const ResourceUsageRange &tag_range,
                                                    ResourceUsageTag start_tag) const {
    HazardResult hazard;
    for (const auto &first : recorded_use.first_accesses_) {
        // Skip and quit logic
        if (first.tag < tag_range.begin) continue;
        if (first.tag >= tag_range.end) break;

        hazard = DetectAsyncHazard(*first.usage_info, start_tag);
        if (hazard.IsHazard()) {
            hazard.AddRecordedAccess(first);
            break;
        }
    }
    return hazard;
}

HazardResult ResourceAccessState::DetectBarrierHazard(const SyncStageAccessInfoType &usage_info, QueueId queue_id,
                                                      VkPipelineStageFlags2KHR src_exec_scope,
                                                      const SyncStageAccessFlags &src_access_scope) const {
    // Only supporting image layout transitions for now
    assert(usage_info.stage_access_index == SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION);
    HazardResult hazard;
    // only test for WAW if there no intervening read operations.
    // See DetectHazard(SyncStagetAccessIndex) above for more details.
    if (last_reads.size()) {
        // Look at the reads if any
        for (const auto &read_access : last_reads) {
            if (read_access.IsReadBarrierHazard(queue_id, src_exec_scope)) {
                hazard.Set(this, usage_info, WRITE_AFTER_READ, read_access.access, read_access.tag);
                break;
            }
        }
    } else if (last_write.has_value() && IsWriteBarrierHazard(queue_id, src_exec_scope, src_access_scope)) {
        hazard.Set(this, usage_info, WRITE_AFTER_WRITE, *last_write);
    }

    return hazard;
}

HazardResult ResourceAccessState::DetectBarrierHazard(const SyncStageAccessInfoType &usage_info,
                                                      const ResourceAccessState &scope_state,
                                                      VkPipelineStageFlags2KHR src_exec_scope,
                                                      const SyncStageAccessFlags &src_access_scope, QueueId event_queue,
                                                      ResourceUsageTag event_tag) const {
    // Only supporting image layout transitions for now
    assert(usage_info.stage_access_index == SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION);
    HazardResult hazard;

    if (last_write.has_value() && (last_write->tag_ >= event_tag)) {
        // Any write after the event precludes the possibility of being in the first access scope for the layout transition
        hazard.Set(this, usage_info, WRITE_AFTER_WRITE, *last_write);
    } else {
        // only test for WAW if there no intervening read operations.
        // See DetectHazard(SyncStagetAccessIndex) above for more details.
        if (last_reads.size()) {
            // Look at the reads if any... if reads exist, they are either the reason the access is in the event
            // first scope, or they are a hazard.
            const ReadStates &scope_reads = scope_state.last_reads;
            const ReadStates::size_type scope_read_count = scope_reads.size();
            // Since the hasn't been a write:
            //  * The current read state is a superset of the scoped one
            //  * The stage order is the same.
            assert(last_reads.size() >= scope_read_count);
            for (ReadStates::size_type read_idx = 0; read_idx < scope_read_count; ++read_idx) {
                const ReadState &scope_read = scope_reads[read_idx];
                const ReadState &current_read = last_reads[read_idx];
                assert(scope_read.stage == current_read.stage);
                if (current_read.tag > event_tag) {
                    // The read is more recent than the set event scope, thus no barrier from the wait/ILT.
                    hazard.Set(this, usage_info, WRITE_AFTER_READ, current_read.access, current_read.tag);
                } else {
                    // The read is in the events first synchronization scope, so we use a barrier hazard check
                    // If the read stage is not in the src sync scope
                    // *AND* not execution chained with an existing sync barrier (that's the or)
                    // then the barrier access is unsafe (R/W after R)
                    if (scope_read.IsReadBarrierHazard(event_queue, src_exec_scope)) {
                        hazard.Set(this, usage_info, WRITE_AFTER_READ, scope_read.access, scope_read.tag);
                        break;
                    }
                }
            }
            if (!hazard.IsHazard() && (last_reads.size() > scope_read_count)) {
                const ReadState &current_read = last_reads[scope_read_count];
                hazard.Set(this, usage_info, WRITE_AFTER_READ, current_read.access, current_read.tag);
            }
        } else if (last_write.has_value()) {
            // if there are no reads, the write is either the reason the access is in the event scope... they are a hazard
            // The write is in the first sync scope of the event (sync their aren't any reads to be the reason)
            // So do a normal barrier hazard check
            if (scope_state.IsWriteBarrierHazard(event_queue, src_exec_scope, src_access_scope)) {
                hazard.Set(&scope_state, usage_info, WRITE_AFTER_WRITE, *scope_state.last_write);
            }
        }
    }

    return hazard;
}
void ResourceAccessState::MergePending(const ResourceAccessState &other) {
    pending_layout_transition |= other.pending_layout_transition;
}

void ResourceAccessState::MergeReads(const ResourceAccessState &other) {
    // Merge the read states
    const auto pre_merge_count = last_reads.size();
    const auto pre_merge_stages = last_read_stages;
    for (uint32_t other_read_index = 0; other_read_index < other.last_reads.size(); other_read_index++) {
        auto &other_read = other.last_reads[other_read_index];
        if (pre_merge_stages & other_read.stage) {
            // Merge in the barriers for read stages that exist in *both* this and other
            // TODO: This is N^2 with stages... perhaps the ReadStates should be sorted by stage index.
            //       but we should wait on profiling data for that.
            for (uint32_t my_read_index = 0; my_read_index < pre_merge_count; my_read_index++) {
                auto &my_read = last_reads[my_read_index];
                if (other_read.stage == my_read.stage) {
                    if (my_read.tag < other_read.tag) {
                        // Other is more recent, copy in the state
                        my_read.access = other_read.access;
                        my_read.tag = other_read.tag;
                        my_read.queue = other_read.queue;
                        my_read.pending_dep_chain = other_read.pending_dep_chain;
                        // TODO: Phase 2 -- review the state merge logic to avoid false positive from overwriting the barriers
                        //                  May require tracking more than one access per stage.
                        my_read.barriers = other_read.barriers;
                        my_read.sync_stages = other_read.sync_stages;
                        if (my_read.stage == VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR) {
                            // Since I'm overwriting the fragement stage read, also update the input attachment info
                            // as this is the only stage that affects it.
                            input_attachment_read = other.input_attachment_read;
                        }
                    } else if (other_read.tag == my_read.tag) {
                        // The read tags match so merge the barriers
                        my_read.barriers |= other_read.barriers;
                        my_read.sync_stages |= other_read.sync_stages;
                        my_read.pending_dep_chain |= other_read.pending_dep_chain;
                    }

                    break;
                }
            }
        } else {
            // The other read stage doesn't exist in this, so add it.
            last_reads.emplace_back(other_read);
            last_read_stages |= other_read.stage;
            if (other_read.stage == VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR) {
                input_attachment_read = other.input_attachment_read;
            }
        }
    }
    read_execution_barriers |= other.read_execution_barriers;
}

// The logic behind resolves is the same as update, we assume that earlier hazards have be reported, and that no
// tranistive hazard can exists with a hazard between the earlier operations.  Yes, an early hazard can mask that another
// exists, but if you fix *that* hazard it either fixes or unmasks the subsequent ones.
void ResourceAccessState::Resolve(const ResourceAccessState &other) {
    bool skip_first = false;
    if (last_write.has_value()) {
        if (other.last_write.has_value()) {
            if (last_write->Tag() < other.last_write->Tag()) {
                // NOTE: Both last and other have writes, and thus first access is "closed". We are selecting other's
                //       first_access state, but it and this can only differ if there are async hazards
                //       error state.
                //
                // If this is a later write, we've reported any exsiting hazard, and we can just overwrite as the more recent
                // operation
                *this = other;
                skip_first = true;
            } else if (last_write->Tag() == other.last_write->Tag()) {
                // In the *equals* case for write operations, we merged the write barriers and the read state (but without the
                // dependency chaining logic or any stage expansion)
                last_write->MergeBarriers(*other.last_write);
                MergePending(other);
                MergeReads(other);
            } else {
                // other write is before this write... in which case we keep this instead of other
                // and can skip the "first_access" merge, since first_access has been closed since other write tag or before
                skip_first = true;
            }
        } else {
            // this has a write and other doesn't -- at best async read in other, which have been reported, and will be dropped
            // Since this has a write first access is closed and shouldn't be updated by other
            skip_first = true;
        }
    } else if (other.last_write.has_value()) {  // && not this->last_write
        // Other has write and this doesn't, thus keep it, See first access NOTE above
        *this = other;
        skip_first = true;
    } else {  // not this->last_write OR other.last_write
        // Neither state has a write, just merge the reads
        MergePending(other);
        MergeReads(other);
    }

    // Merge first access information by making a copy of this first_access and reconstructing with a shuffle
    // of the copy and other into this using the update first logic.
    // NOTE: All sorts of additional cleverness could be put into short circuts.  (for example back is write and is before front
    //       of the other first_accesses... )
    if (!skip_first && !(first_accesses_ == other.first_accesses_) && !other.first_accesses_.empty()) {
        FirstAccesses firsts(std::move(first_accesses_));
        ClearFirstUse();
        auto a = firsts.begin();
        auto a_end = firsts.end();
        for (auto &b : other.first_accesses_) {
            // TODO: Determine whether some tag offset will be needed for PHASE II
            while ((a != a_end) && (a->tag < b.tag)) {
                UpdateFirst(a->tag, *a->usage_info, a->ordering_rule);
                ++a;
            }
            UpdateFirst(b.tag, *b.usage_info, b.ordering_rule);
        }
        for (; a != a_end; ++a) {
            UpdateFirst(a->tag, *a->usage_info, a->ordering_rule);
        }
    }
}

void ResourceAccessState::Update(const SyncStageAccessInfoType &usage_info, SyncOrdering ordering_rule,
                                 const ResourceUsageTag tag) {
    // Move this logic in the ResourceStateTracker as methods, thereof (or we'll repeat it for every flavor of resource...
    const auto &usage_bit = usage_info.stage_access_bit;
    const auto &usage_stage = usage_info.stage_mask;
    if (IsRead(usage_info)) {
        // Mulitple outstanding reads may be of interest and do dependency chains independently
        // However, for purposes of barrier tracking, only one read per pipeline stage matters
        if (usage_stage & last_read_stages) {
            const auto not_usage_stage = ~usage_stage;
            for (auto &read_access : last_reads) {
                if (read_access.stage == usage_stage) {
                    read_access.Set(usage_stage, usage_bit, 0, tag);
                } else if (read_access.barriers & usage_stage) {
                    // If the current access is barriered to this stage, mark it as "known to happen after"
                    read_access.sync_stages |= usage_stage;
                } else {
                    // If the current access is *NOT* barriered to this stage it needs to be cleared.
                    // Note: this is possible because semaphores can *clear* effective barriers, so the assumption
                    //       that sync_stages is a subset of barriers may not apply.
                    read_access.sync_stages &= not_usage_stage;
                }
            }
        } else {
            for (auto &read_access : last_reads) {
                if (read_access.barriers & usage_stage) {
                    read_access.sync_stages |= usage_stage;
                }
            }
            last_reads.emplace_back(usage_stage, usage_bit, 0, tag);
            last_read_stages |= usage_stage;
        }

        // Fragment shader reads come in two flavors, and we need to track if the one we're tracking is the special one.
        if (usage_stage == VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR) {
            // TODO Revisit re: multiple reads for a given stage
            input_attachment_read = (usage_bit == SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ_BIT);
        }
    } else {
        // Assume write
        // TODO determine what to do with READ-WRITE operations if any
        SetWrite(usage_info, tag);
    }
    UpdateFirst(tag, usage_info, ordering_rule);
}

// Clobber last read and all barriers... because all we have is DANGER, DANGER, WILL ROBINSON!!!
// if the last_reads/last_write were unsafe, we've reported them, in either case the prior access is irrelevant.
// We can overwrite them as *this* write is now after them.
//
// Note: intentionally ignore pending barriers and chains (i.e. don't apply or clear them), let ApplyPendingBarriers handle them.
void ResourceAccessState::SetWrite(const SyncStageAccessInfoType &usage_info, const ResourceUsageTag tag) {
    ClearRead();
    if (last_write.has_value()) {
        last_write->Set(usage_info, tag);
    } else {
        last_write.emplace(usage_info, tag);
    }
}

void ResourceAccessState::ClearWrite() { last_write.reset(); }

void ResourceAccessState::ClearRead() {
    last_reads.clear();
    last_read_stages = VK_PIPELINE_STAGE_2_NONE;
    read_execution_barriers = VK_PIPELINE_STAGE_2_NONE;
    input_attachment_read = false;  // Denotes no outstanding input attachment read after the last write.
}

void ResourceAccessState::ClearPending() {
    pending_layout_transition = false;
    if (last_write.has_value()) last_write->ClearPending();
}

void ResourceAccessState::ClearFirstUse() {
    first_accesses_.clear();
    first_read_stages_ = VK_PIPELINE_STAGE_2_NONE;
    first_write_layout_ordering_ = OrderingBarrier();
    first_access_closed_ = false;
}

// Apply the memory barrier without updating the existing barriers.  The execution barrier
// changes the "chaining" state, but to keep barriers independent, we defer this until all barriers
// of the batch have been processed. Also, depending on whether layout transition happens, we'll either
// replace the current write barriers or add to them, so accumulate to pending as well.
template <typename ScopeOps>
void ResourceAccessState::ApplyBarrier(ScopeOps &&scope, const SyncBarrier &barrier, bool layout_transition) {
    // For independent barriers we need to track what the new barriers and dependency chain *will* be when we're done
    // applying the memory barriers
    // NOTE: We update the write barrier if the write is in the first access scope or if there is a layout
    //       transistion, under the theory of "most recent access".  If the resource acces  *isn't* safe
    //       vs. this layout transition DetectBarrierHazard should report it.  We treat the layout
    //       transistion *as* a write and in scope with the barrier (it's before visibility).
    if (layout_transition) {
        if (!last_write.has_value()) {
            last_write.emplace(UsageInfo(SYNC_ACCESS_INDEX_NONE), 0U);
        }
        last_write->UpdatePendingBarriers(barrier);
        last_write->UpdatePendingLayoutOrdering(barrier);
        pending_layout_transition = true;
    } else {
        if (scope.WriteInScope(barrier, *this)) {
            last_write->UpdatePendingBarriers(barrier);
        }

        if (!pending_layout_transition) {
            // Once we're dealing with a layout transition (which is modelled as a *write*) then the last reads/chains
            // don't need to be tracked as we're just going to clear them.
            VkPipelineStageFlags2 stages_in_scope = VK_PIPELINE_STAGE_2_NONE;

            for (auto &read_access : last_reads) {
                // The | implements the "dependency chain" logic for this access, as the barriers field stores the second sync
                // scope
                if (scope.ReadInScope(barrier, read_access)) {
                    // We'll apply the barrier in the next loop, because it's DRY'r to do it one place.
                    stages_in_scope |= read_access.stage;
                }
            }

            for (auto &read_access : last_reads) {
                if (0 != ((read_access.stage | read_access.sync_stages) & stages_in_scope)) {
                    // If this stage, or any stage known to be synchronized after it are in scope, apply the barrier to this
                    // read NOTE: Forwarding barriers to known prior stages changes the sync_stages from shallow to deep,
                    // because the
                    //       barriers used to determine sync_stages have been propagated to all known earlier stages
                    read_access.ApplyReadBarrier(barrier.dst_exec_scope.exec_scope);
                }
            }
        }
    }
}

void ResourceAccessState::ApplyPendingBarriers(const ResourceUsageTag tag) {
    if (pending_layout_transition) {
        // SetWrite clobbers the last_reads array, and thus we don't have to clear the read_state out.
        const SyncStageAccessInfoType &layout_usage_info = UsageInfo(SYNC_IMAGE_LAYOUT_TRANSITION);
        SetWrite(layout_usage_info, tag);  // Side effect notes below
        UpdateFirst(tag, layout_usage_info, SyncOrdering::kNonAttachment);
        TouchupFirstForLayoutTransition(tag, last_write->GetPendingLayoutOrdering());

        last_write->ApplyPendingBarriers();
        last_write->ClearPending();
        pending_layout_transition = false;
    } else {
        // Apply the accumulate execution barriers (and thus update chaining information)
        // for layout transition, last_reads is reset by SetWrite, so this will be skipped.
        for (auto &read_access : last_reads) {
            read_execution_barriers |= read_access.ApplyPendingBarriers();
        }

        // We OR in the accumulated write chain and barriers even in the case of a layout transition as SetWrite zeros them.
        if (last_write.has_value()) {
            last_write->ApplyPendingBarriers();
            last_write->ClearPending();
        }
    }
}

// Assumes signal queue != wait queue
void ResourceAccessState::ApplySemaphore(const SemaphoreScope &signal, const SemaphoreScope wait) {
    // Semaphores only guarantee the first scope of the signal is before the second scope of the wait.
    // If any access isn't in the first scope, there are no guarantees, thus those barriers are cleared
    assert(signal.queue != wait.queue);
    for (auto &read_access : last_reads) {
        if (read_access.ReadInQueueScopeOrChain(signal.queue, signal.exec_scope)) {
            // Deflects WAR on wait queue
            read_access.barriers = wait.exec_scope;
        } else {
            // Leave sync stages alone. Update method will clear unsynchronized stages on subsequent reads as needed.
            read_access.barriers = VK_PIPELINE_STAGE_2_NONE;
        }
    }
    if (WriteInQueueSourceScopeOrChain(signal.queue, signal.exec_scope, signal.valid_accesses)) {
        assert(last_write.has_value());
        // Will deflect RAW wait queue, WAW needs a chained barrier on wait queue
        read_execution_barriers = wait.exec_scope;
        last_write->barriers_ = wait.valid_accesses;
    } else {
        read_execution_barriers = VK_PIPELINE_STAGE_2_NONE;
        if (last_write.has_value()) last_write->barriers_.reset();
    }
    if (last_write.has_value()) last_write->dependency_chain_ = read_execution_barriers;
}

// Read access predicate for queue wait
bool ResourceAccessState::WaitQueueTagPredicate::operator()(const ResourceAccessState::ReadState &read_access) const {
    return (read_access.queue == queue) && (read_access.tag <= tag) &&
           (read_access.stage != VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL);
}
bool ResourceAccessState::WaitQueueTagPredicate::operator()(const ResourceAccessState &access) const {
    if (!access.last_write.has_value()) return false;
    const auto &write_state = *access.last_write;
    return write_state.IsQueue(queue) && (write_state.Tag() <= tag) &&
           !write_state.IsIndex(SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL);
}

// Read access predicate for queue wait
bool ResourceAccessState::WaitTagPredicate::operator()(const ResourceAccessState::ReadState &read_access) const {
    return (read_access.tag <= tag) && (read_access.stage != VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL);
}
bool ResourceAccessState::WaitTagPredicate::operator()(const ResourceAccessState &access) const {
    if (!access.last_write.has_value()) return false;
    const auto &write_state = *access.last_write;
    return (write_state.Tag() <= tag) && !write_state.IsIndex(SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL);
}

// Present operations only matching only the *exactly* tagged present and acquire operations
bool ResourceAccessState::WaitAcquirePredicate::operator()(const ResourceAccessState::ReadState &read_access) const {
    return (read_access.tag == acquire_tag) && (read_access.stage == VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL);
}
bool ResourceAccessState::WaitAcquirePredicate::operator()(const ResourceAccessState &access) const {
    if (!access.last_write.has_value()) return false;
    const auto &write_state = *access.last_write;
    return (write_state.Tag() == present_tag) && write_state.IsIndex(SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL);
}

// Return if the resulting state is "empty"
template <typename Predicate>
bool ResourceAccessState::ApplyPredicatedWait(Predicate &predicate) {
    VkPipelineStageFlags2KHR sync_reads = VK_PIPELINE_STAGE_2_NONE;

    // Use the predicate to build a mask of the read stages we are synchronizing
    // Use the sync_stages to also detect reads known to be before any synchronized reads (first pass)
    for (auto &read_access : last_reads) {
        if (predicate(read_access)) {
            // If we know this stage is before any stage we syncing, or if the predicate tells us that we are waited for..
            sync_reads |= read_access.stage;
        }
    }

    // Now that we know the reads directly in scopejust need to go over the list again to pick up the "known earlier" stages.
    // NOTE: sync_stages is "deep" catching all stages synchronized after it because we forward barriers
    uint32_t unsync_count = 0;
    for (auto &read_access : last_reads) {
        if (0 != ((read_access.stage | read_access.sync_stages) & sync_reads)) {
            // This is redundant in the "stage" case, but avoids a second branch to get an accurate count
            sync_reads |= read_access.stage;
        } else {
            ++unsync_count;
        }
    }

    if (unsync_count) {
        if (sync_reads) {
            // When have some remaining unsynchronized reads, we have to rewrite the last_reads array.
            ReadStates unsync_reads;
            unsync_reads.reserve(unsync_count);
            VkPipelineStageFlags2KHR unsync_read_stages = VK_PIPELINE_STAGE_2_NONE;
            for (auto &read_access : last_reads) {
                if (0 == (read_access.stage & sync_reads)) {
                    unsync_reads.emplace_back(read_access);
                    unsync_read_stages |= read_access.stage;
                }
            }
            last_read_stages = unsync_read_stages;
            last_reads = std::move(unsync_reads);
        }
    } else {
        // Nothing remains (or it was empty to begin with)
        ClearRead();
    }

    bool all_clear = last_reads.size() == 0;
    if (last_write.has_value()) {
        if (predicate(*this) || sync_reads) {
            // Clear any predicated write, or any the write from any any access with synchronized reads.
            // This could drop RAW detection, but only if the synchronized reads were RAW hazards, and given
            // MRR approach to reporting, this is consistent with other drops, especially since fixing the
            // RAW wit the sync_reads stages would preclude a subsequent RAW.
            ClearWrite();
        } else {
            all_clear = false;
        }
    }
    return all_clear;
}

bool ResourceAccessState::FirstAccessInTagRange(const ResourceUsageRange &tag_range) const {
    if (!first_accesses_.size()) return false;
    const ResourceUsageRange first_access_range = {first_accesses_.front().tag, first_accesses_.back().tag + 1};
    return tag_range.intersects(first_access_range);
}

void ResourceAccessState::OffsetTag(ResourceUsageTag offset) {
    if (last_write.has_value()) last_write->OffsetTag(offset);
    for (auto &read_access : last_reads) {
        read_access.tag += offset;
    }
    for (auto &first : first_accesses_) {
        first.tag += offset;
    }
}

static const SyncStageAccessFlags kAllSyncStageAccessBits = ~SyncStageAccessFlags(0);
ResourceAccessState::ResourceAccessState()
    : last_write(),
      last_read_stages(0),
      read_execution_barriers(VK_PIPELINE_STAGE_2_NONE),
      last_reads(),
      input_attachment_read(false),
      pending_layout_transition(false),
      first_accesses_(),
      first_read_stages_(VK_PIPELINE_STAGE_2_NONE),
      first_write_layout_ordering_(),
      first_access_closed_(false) {}

// This should be just Bits or Index, but we don't have an invalid state for Index
VkPipelineStageFlags2KHR ResourceAccessState::GetReadBarriers(const SyncStageAccessFlags &usage_bit) const {
    VkPipelineStageFlags2KHR barriers = VK_PIPELINE_STAGE_2_NONE;

    for (const auto &read_access : last_reads) {
        if ((read_access.access & usage_bit).any()) {
            barriers = read_access.barriers;
            break;
        }
    }

    return barriers;
}

void ResourceAccessState::SetQueueId(QueueId id) {
    for (auto &read_access : last_reads) {
        if (read_access.queue == QueueSyncState::kQueueIdInvalid) {
            read_access.queue = id;
        }
    }
    if (last_write.has_value()) last_write->SetQueueId(id);
}

bool ResourceAccessState::IsWriteBarrierHazard(QueueId queue_id, VkPipelineStageFlags2KHR src_exec_scope,
                                               const SyncStageAccessFlags &src_access_scope) const {
    return last_write.has_value() && last_write->IsWriteBarrierHazard(queue_id, src_exec_scope, src_access_scope);
}

bool ResourceAccessState::WriteInSourceScopeOrChain(VkPipelineStageFlags2KHR src_exec_scope,
                                                    SyncStageAccessFlags src_access_scope) const {
    return last_write.has_value() && last_write->WriteInSourceScopeOrChain(src_exec_scope, src_access_scope);
}

bool ResourceAccessState::WriteInQueueSourceScopeOrChain(QueueId queue, VkPipelineStageFlags2KHR src_exec_scope,
                                                         const SyncStageAccessFlags &src_access_scope) const {
    return last_write.has_value() && last_write->WriteInQueueSourceScopeOrChain(queue, src_exec_scope, src_access_scope);
}

bool ResourceAccessState::WriteInEventScope(VkPipelineStageFlags2KHR src_exec_scope, const SyncStageAccessFlags &src_access_scope,
                                            QueueId scope_queue, ResourceUsageTag scope_tag) const {
    return last_write.has_value() && last_write->WriteInEventScope(src_exec_scope, src_access_scope, scope_queue, scope_tag);
}

// As ReadStates must be unique by stage, this is as good a sort as needed
bool operator<(const ResourceAccessState::ReadState &lhs, const ResourceAccessState::ReadState &rhs) {
    return lhs.stage < rhs.stage;
}

void ResourceAccessState::Normalize() {
    if (!last_reads.size()) {
        ClearRead();
    } else {
        // Sort the reads in stage order for consistent comparisons
        std::sort(last_reads.begin(), last_reads.end());
        for (auto &read_access : last_reads) {
            read_access.Normalize();
        }
    }

    ClearPending();
    ClearFirstUse();
}

void ResourceAccessState::GatherReferencedTags(ResourceUsageTagSet &used) const {
    if (last_write.has_value()) {
        used.CachedInsert(last_write->Tag());
    }

    for (const auto &read_access : last_reads) {
        used.CachedInsert(read_access.tag);
    }
}

bool ResourceAccessState::IsRAWHazard(const SyncStageAccessInfoType &usage_info) const {
    assert(IsRead(usage_info));
    // Only RAW vs. last_write if it doesn't happen-after any other read because either:
    //    * the previous reads are not hazards, and thus last_write must be visible and available to
    //      any reads that happen after.
    //    * the previous reads *are* hazards to last_write, have been reported, and if that hazard is fixed
    //      the current read will be also not be a hazard, thus reporting a hazard here adds no needed information.
    return last_write.has_value() && (0 == (read_execution_barriers & usage_info.stage_mask)) &&
           last_write->IsWriteHazard(usage_info);
}

VkPipelineStageFlags2 ResourceAccessState::GetOrderedStages(QueueId queue_id, const OrderingBarrier &ordering) const {
    // At apply queue submission order limits on the effect of ordering
    VkPipelineStageFlags2 non_qso_stages = VK_PIPELINE_STAGE_2_NONE;
    if (queue_id != QueueSyncState::kQueueIdInvalid) {
        for (const auto &read_access : last_reads) {
            if (read_access.queue != queue_id) {
                non_qso_stages |= read_access.stage;
            }
        }
    }
    // Whether the stage are in the ordering scope only matters if the current write is ordered
    const VkPipelineStageFlags2 read_stages_in_qso = last_read_stages & ~non_qso_stages;
    VkPipelineStageFlags2 ordered_stages = read_stages_in_qso & ordering.exec_scope;
    // Special input attachment handling as always (not encoded in exec_scop)
    const bool input_attachment_ordering = ordering.access_scope[SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ];
    if (input_attachment_ordering && input_attachment_read) {
        // If we have an input attachment in last_reads and input attachments are ordered we all that stage
        ordered_stages |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR;
    }

    return ordered_stages;
}

void ResourceAccessState::UpdateFirst(const ResourceUsageTag tag, const SyncStageAccessInfoType &usage_info,
                                      SyncOrdering ordering_rule) {
    // Only record until we record a write.
    if (!first_access_closed_) {
        const bool is_read = IsRead(usage_info);
        const VkPipelineStageFlags2KHR usage_stage = is_read ? usage_info.stage_mask : 0U;
        if (0 == (usage_stage & first_read_stages_)) {
            // If this is a read we haven't seen or a write, record.
            // We always need to know what stages were found prior to write
            first_read_stages_ |= usage_stage;
            if (0 == (read_execution_barriers & usage_stage)) {
                // If this stage isn't masked then we add it (since writes map to usage_stage 0, this also records writes)
                first_accesses_.emplace_back(tag, usage_info, ordering_rule);
                first_access_closed_ = !is_read;
            }
        }
    }
}

void ResourceAccessState::TouchupFirstForLayoutTransition(ResourceUsageTag tag, const OrderingBarrier &layout_ordering) {
    // Only call this after recording an image layout transition
    assert(first_accesses_.size());
    if (first_accesses_.back().tag == tag) {
        // If this layout transition is the the first write, add the additional ordering rules that guard the ILT
        assert(first_accesses_.back().usage_info->stage_access_index == SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION);
        first_write_layout_ordering_ = layout_ordering;
    }
}

ResourceAccessState::ReadState::ReadState(VkPipelineStageFlags2KHR stage_, SyncStageAccessFlags access_,
                                          VkPipelineStageFlags2KHR barriers_, ResourceUsageTag tag_)
    : stage(stage_),
      access(access_),
      barriers(barriers_),
      sync_stages(VK_PIPELINE_STAGE_2_NONE),
      tag(tag_),
      queue(QueueSyncState::kQueueIdInvalid),
      pending_dep_chain(VK_PIPELINE_STAGE_2_NONE) {}

void ResourceAccessState::ReadState::Set(VkPipelineStageFlags2KHR stage_, const SyncStageAccessFlags &access_,
                                         VkPipelineStageFlags2KHR barriers_, ResourceUsageTag tag_) {
    stage = stage_;
    access = access_;
    barriers = barriers_;
    sync_stages = VK_PIPELINE_STAGE_2_NONE;
    tag = tag_;
    pending_dep_chain = VK_PIPELINE_STAGE_2_NONE;  // If this is a new read, we aren't applying a barrier set.
}

// Scope test including "queue submission order" effects.  Specifically, accesses from a different queue are not
// considered to be in "queue submission order" with barriers, events, or semaphore signalling, but any barriers
// that have bee applied (via semaphore) to those accesses can be chained off of.
bool ResourceAccessState::ReadState::ReadInQueueScopeOrChain(QueueId scope_queue, VkPipelineStageFlags2 exec_scope) const {
    VkPipelineStageFlags2 effective_stages = barriers | ((scope_queue == queue) ? stage : VK_PIPELINE_STAGE_2_NONE);
    return (exec_scope & effective_stages) != 0;
}

VkPipelineStageFlags2 ResourceAccessState::ReadState::ApplyPendingBarriers() {
    barriers |= pending_dep_chain;
    pending_dep_chain = VK_PIPELINE_STAGE_2_NONE;
    return barriers;
}

ResourceUsageRange SyncValidator::ReserveGlobalTagRange(size_t tag_count) const {
    ResourceUsageRange reserve;
    reserve.begin = tag_limit_.fetch_add(tag_count);
    reserve.end = reserve.begin + tag_count;
    return reserve;
}

void SyncValidator::ApplyTaggedWait(QueueId queue_id, ResourceUsageTag tag) {
    auto tagged_wait_op = [queue_id, tag](const std::shared_ptr<QueueBatchContext> &batch) {
        batch->ApplyTaggedWait(queue_id, tag);
        batch->Trim();
    };
    ForAllQueueBatchContexts(tagged_wait_op);
}

void SyncValidator::ApplyAcquireWait(const AcquiredImage &acquired) {
    auto acq_wait_op = [&acquired](const std::shared_ptr<QueueBatchContext> &batch) {
        batch->ApplyAcquireWait(acquired);
        batch->Trim();
    };
    ForAllQueueBatchContexts(acq_wait_op);
}

template <typename BatchOp>
void SyncValidator::ForAllQueueBatchContexts(BatchOp &&op) {
    // Often we need to go through every queue batch context and apply synchronization operations
    // As usual -- two groups, the "last batch" and the signaled semaphores
    QueueBatchContext::BatchSet queue_batch_contexts = GetQueueBatchSnapshot();

    // Note: The const is to force the reference to const be on all platforms.
    //
    // It's not obivious (nor cross platform consitent), that the batch reference should be const
    // but since it's pointing to the actual *key* for the set it must be. This doesn't make the
    // object the shared pointer is referencing constant however.
    for (const auto &batch : queue_batch_contexts) {
        op(batch);
    }
}

void SyncValidator::UpdateFenceWaitInfo(VkFence fence, QueueId queue_id, ResourceUsageTag tag) {
    std::shared_ptr<const FENCE_STATE> fence_state = Get<FENCE_STATE>(fence);
    UpdateFenceWaitInfo(fence_state, FenceSyncState(fence_state, queue_id, tag));
}
void SyncValidator::UpdateFenceWaitInfo(VkFence fence, const PresentedImage &image, ResourceUsageTag tag) {
    std::shared_ptr<const FENCE_STATE> fence_state = Get<FENCE_STATE>(fence);
    UpdateFenceWaitInfo(fence_state, FenceSyncState(fence_state, image, tag));
}

void SyncValidator::UpdateFenceWaitInfo(std::shared_ptr<const FENCE_STATE> &fence_state, FenceSyncState &&wait_info) {
    if (BASE_NODE::Invalid(fence_state)) return;
    waitable_fences_[fence_state->fence()] = std::move(wait_info);
}

void SyncValidator::WaitForFence(VkFence fence) {
    auto fence_it = waitable_fences_.find(fence);
    if (fence_it != waitable_fences_.end()) {
        // The fence may no longer be waitable for several valid reasons.
        FenceSyncState &wait_for = fence_it->second;
        if (wait_for.acquired.Invalid()) {
            // This is just a normal fence wait
            ApplyTaggedWait(wait_for.queue_id, wait_for.tag);
        } else {
            // This a fence wait for a present operation
            ApplyAcquireWait(wait_for.acquired);
        }
        waitable_fences_.erase(fence_it);
    }
}

void SyncValidator::UpdateSyncImageMemoryBindState(uint32_t count, const VkBindImageMemoryInfo *infos) {
    for (const auto &info : vvl::make_span(infos, count)) {
        if (VK_NULL_HANDLE == info.image) continue;
        auto image_state = Get<ImageState>(info.image);
        if (image_state->IsTiled()) {
            image_state->SetOpaqueBaseAddress(*this);
        }
    }
}

const QueueSyncState *SyncValidator::GetQueueSyncState(VkQueue queue) const {
    return GetMappedPlainFromShared(queue_sync_states_, queue);
}

QueueSyncState *SyncValidator::GetQueueSyncState(VkQueue queue) { return GetMappedPlainFromShared(queue_sync_states_, queue); }

std::shared_ptr<const QueueSyncState> SyncValidator::GetQueueSyncStateShared(VkQueue queue) const {
    return GetMapped(queue_sync_states_, queue, []() { return std::shared_ptr<QueueSyncState>(); });
}

std::shared_ptr<QueueSyncState> SyncValidator::GetQueueSyncStateShared(VkQueue queue) {
    return GetMapped(queue_sync_states_, queue, []() { return std::shared_ptr<QueueSyncState>(); });
}

template <typename T>
struct GetBatchTraits {};
template <>
struct GetBatchTraits<std::shared_ptr<QueueSyncState>> {
    using Batch = std::shared_ptr<QueueBatchContext>;
    static Batch Get(const std::shared_ptr<QueueSyncState> &qss) { return qss ? qss->LastBatch() : Batch(); }
};

template <>
struct GetBatchTraits<std::shared_ptr<SignaledSemaphores::Signal>> {
    using Batch = std::shared_ptr<QueueBatchContext>;
    static Batch Get(const std::shared_ptr<SignaledSemaphores::Signal> &sig) { return sig ? sig->batch : Batch(); }
};

template <typename BatchSet, typename Map, typename Predicate>
static BatchSet GetQueueBatchSnapshotImpl(const Map &map, Predicate &&pred) {
    BatchSet snapshot;
    for (auto &entry : map) {
        // Intentional copy
        auto batch = GetBatchTraits<typename Map::mapped_type>::Get(entry.second);
        if (batch && pred(batch)) snapshot.emplace(std::move(batch));
    }
    return snapshot;
}

template <typename Predicate>
QueueBatchContext::ConstBatchSet SyncValidator::GetQueueLastBatchSnapshot(Predicate &&pred) const {
    return GetQueueBatchSnapshotImpl<QueueBatchContext::ConstBatchSet>(queue_sync_states_, std::forward<Predicate>(pred));
}

template <typename Predicate>
QueueBatchContext::BatchSet SyncValidator::GetQueueLastBatchSnapshot(Predicate &&pred) {
    return GetQueueBatchSnapshotImpl<QueueBatchContext::BatchSet>(queue_sync_states_, std::forward<Predicate>(pred));
}

QueueBatchContext::BatchSet SyncValidator::GetQueueBatchSnapshot() {
    QueueBatchContext::BatchSet snapshot = GetQueueLastBatchSnapshot();
    auto append = [&snapshot](const std::shared_ptr<QueueBatchContext> &batch) {
        if (batch && !vvl::Contains(snapshot, batch)) {
            snapshot.emplace(batch);
        }
        return false;
    };
    GetQueueBatchSnapshotImpl<QueueBatchContext::BatchSet>(signaled_semaphores_, append);
    return snapshot;
}

struct QueueSubmitCmdState {
    std::shared_ptr<const QueueSyncState> queue;
    std::shared_ptr<QueueBatchContext> last_batch;
    const ErrorObject &error_obj;
    SignaledSemaphores signaled;
    QueueSubmitCmdState(const ErrorObject &error_obj, const SignaledSemaphores &parent_semaphores)
        : error_obj(error_obj), signaled(parent_semaphores) {}
};

bool QueueBatchContext::DoQueueSubmitValidate(const SyncValidator &sync_state, QueueSubmitCmdState &cmd_state,
                                              const VkSubmitInfo2 &batch_info) {
    bool skip = false;

    //  For each submit in the batch...
    for (const auto &cb : command_buffers_) {
        const auto &cb_access_context = cb.cb->access_context;
        if (cb_access_context.GetTagLimit() == 0) {
            batch_.cb_index++;
            continue;  // Skip empty CB's but also skip the unused index for correct reporting
        }
        skip |= ReplayState(*this, cb_access_context, cmd_state.error_obj, cb.index).ValidateFirstUse();

        // The barriers have already been applied in ValidatFirstUse
        ResourceUsageRange tag_range = ImportRecordedAccessLog(cb_access_context);
        ResolveSubmittedCommandBuffer(*cb_access_context.GetCurrentAccessContext(), tag_range.begin);
    }
    return skip;
}

bool SignaledSemaphores::SignalSemaphore(const std::shared_ptr<const SEMAPHORE_STATE> &sem_state,
                                         const std::shared_ptr<QueueBatchContext> &batch,
                                         const VkSemaphoreSubmitInfo &signal_info) {
    assert(batch);
    const SyncExecScope exec_scope =
        SyncExecScope::MakeSrc(batch->GetQueueFlags(), signal_info.stageMask, VK_PIPELINE_STAGE_2_HOST_BIT);
    std::shared_ptr<Signal> signal = std::make_shared<Signal>(sem_state, batch, exec_scope);
    return Insert(sem_state, std::move(signal));
}

bool SignaledSemaphores::Insert(const std::shared_ptr<const SEMAPHORE_STATE> &sem_state, std::shared_ptr<Signal> &&signal) {
    const VkSemaphore sem = sem_state->semaphore();
    auto signal_it = signaled_.find(sem);
    std::shared_ptr<Signal> insert_signal;
    if (signal_it == signaled_.end()) {
        if (prev_) {
            auto prev_sig = GetMapped(prev_->signaled_, sem_state->semaphore(), []() { return std::shared_ptr<Signal>(); });
            if (prev_sig) {
                // The is an invalid signal, as this semaphore is already signaled... copy the prev state (as prev_ is const)
                insert_signal = std::make_shared<Signal>(*prev_sig);
            }
        }
        auto insert_pair = signaled_.emplace(sem, std::move(insert_signal));
        signal_it = insert_pair.first;
    }

    bool success = false;
    if (!signal_it->second) {
        signal_it->second = std::move(signal);
        success = true;
    }

    return success;
}

bool SignaledSemaphores::SignalSemaphore(const std::shared_ptr<const SEMAPHORE_STATE> &sem_state, const PresentedImage &presented,
                                         ResourceUsageTag acq_tag) {
    // Ignore any signal we haven't waited... CoreChecks should have reported this
    std::shared_ptr<Signal> signal = std::make_shared<Signal>(sem_state, presented, acq_tag);
    return Insert(sem_state, std::move(signal));
}

std::shared_ptr<const SignaledSemaphores::Signal> SignaledSemaphores::Unsignal(VkSemaphore sem) {
    std::shared_ptr<const Signal> unsignaled;
    const auto found_it = signaled_.find(sem);
    if (found_it != signaled_.end()) {
        // Move the unsignaled singal out from the signaled list, but keep the shared_ptr as the caller needs the contents for
        // a bit.
        unsignaled = std::move(found_it->second);
        if (!prev_) {
            // No parent, not need to keep the entry
            // IFF (prev_)  leave the entry in the leaf table as we use it to export unsignal to prev_ during record phase
            signaled_.erase(found_it);
        }
    } else if (prev_) {
        // We can't unsignal prev_ because it's const * by design.
        // We put in an empty placeholder
        signaled_.emplace(sem, std::shared_ptr<Signal>());
        unsignaled = GetPrev(sem);
    }
    // NOTE: No else clause. Because if we didn't find it, and there's no previous, this indicates an error,
    // but CoreChecks should have reported it

    // If unsignaled is null, there was a missing pending semaphore, and that's also issue CoreChecks reports
    return unsignaled;
}

void SignaledSemaphores::Resolve(SignaledSemaphores &parent, std::shared_ptr<QueueBatchContext> &last_batch) {
    // Must only be called on child objects, with the non-const reference of the parent/previous object passed in
    assert(prev_ == &parent);

    // The global  the semaphores we applied to the cmd_state QueueBatchContexts
    // NOTE: All conserved QueueBatchContext's need to have there access logs reset to use the global logger and the only conserved
    //       QBC's are those referenced by unwaited signals and the last batch.
    for (auto &sig_sem : signaled_) {
        if (sig_sem.second && sig_sem.second->batch) {
            auto &sig_batch = sig_sem.second->batch;
            // Batches retained for signalled semaphore don't need to retain event data, unless it's the last batch in the submit
            if (sig_batch != last_batch) {
                sig_batch->ResetEventsContext();
                // Make sure that retained batches are minimal, and trim after the events contexts has been cleared.
                sig_batch->Trim();
            }
        }
        // Import clears in the parent any signal waited in the
        parent.Import(sig_sem.first, std::move(sig_sem.second));
    }
    Reset();
}

void SignaledSemaphores::Import(VkSemaphore sem, std::shared_ptr<Signal> &&from) {
    // Overwrite the s  tate with the last state from this
    if (from) {
        assert(sem == from->sem_state->semaphore());
        signaled_[sem] = std::move(from);
    } else {
        signaled_.erase(sem);
    }
}

void SignaledSemaphores::Reset() {
    signaled_.clear();
    prev_ = nullptr;
}
syncval_state::CommandBuffer::CommandBuffer(SyncValidator *dev, VkCommandBuffer cb, const VkCommandBufferAllocateInfo *pCreateInfo,
                                            const COMMAND_POOL_STATE *pool)
    : CMD_BUFFER_STATE(dev, cb, pCreateInfo, pool), access_context(*dev, this) {}

void syncval_state::CommandBuffer::Destroy() {
    access_context.Destroy();  // must be first to clean up self references correctly.
    CMD_BUFFER_STATE::Destroy();
}

void syncval_state::CommandBuffer::Reset() {
    CMD_BUFFER_STATE::Reset();
    access_context.Reset();
}

void syncval_state::CommandBuffer::NotifyInvalidate(const BASE_NODE::NodeList &invalid_nodes, bool unlink) {
    for (auto &obj : invalid_nodes) {
        switch (obj->Type()) {
            case kVulkanObjectTypeEvent:
                access_context.RecordDestroyEvent(static_cast<EVENT_STATE *>(obj.get()));
                break;
            default:
                break;
        }
        CMD_BUFFER_STATE::NotifyInvalidate(invalid_nodes, unlink);
    }
}

std::shared_ptr<CMD_BUFFER_STATE> SyncValidator::CreateCmdBufferState(VkCommandBuffer cb,
                                                                      const VkCommandBufferAllocateInfo *pCreateInfo,
                                                                      const COMMAND_POOL_STATE *cmd_pool) {
    auto cb_state = std::make_shared<syncval_state::CommandBuffer>(this, cb, pCreateInfo, cmd_pool);
    if (cb_state) {
        cb_state->access_context.SetSelfReference();
    }
    return std::static_pointer_cast<CMD_BUFFER_STATE>(cb_state);
}

std::shared_ptr<SWAPCHAIN_NODE> SyncValidator::CreateSwapchainState(const VkSwapchainCreateInfoKHR *create_info,
                                                                    VkSwapchainKHR swapchain) {
    return std::static_pointer_cast<SWAPCHAIN_NODE>(std::make_shared<syncval_state::Swapchain>(this, create_info, swapchain));
}

std::shared_ptr<IMAGE_STATE> SyncValidator::CreateImageState(VkImage img, const VkImageCreateInfo *pCreateInfo,
                                                             VkFormatFeatureFlags2KHR features) {
    return std::make_shared<ImageState>(this, img, pCreateInfo, features);
}

std::shared_ptr<IMAGE_STATE> SyncValidator::CreateImageState(VkImage img, const VkImageCreateInfo *pCreateInfo,
                                                             VkSwapchainKHR swapchain, uint32_t swapchain_index,
                                                             VkFormatFeatureFlags2KHR features) {
    return std::make_shared<ImageState>(this, img, pCreateInfo, swapchain, swapchain_index, features);
}
std::shared_ptr<IMAGE_VIEW_STATE> SyncValidator::CreateImageViewState(
    const std::shared_ptr<IMAGE_STATE> &image_state, VkImageView iv, const VkImageViewCreateInfo *ci, VkFormatFeatureFlags2KHR ff,
    const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props) {
    return std::make_shared<ImageViewState>(image_state, iv, ci, ff, cubic_props);
}

bool SyncValidator::PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                                 uint32_t regionCount, const VkBufferCopy *pRegions,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;
    const auto *context = cb_context->GetCurrentAccessContext();

    // If we have no previous accesses, we have no hazards
    auto src_buffer = Get<BUFFER_STATE>(srcBuffer);
    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_buffer) {
            const ResourceAccessRange src_range = MakeRange(*src_buffer, copy_region.srcOffset, copy_region.size);
            auto hazard = context->DetectHazard(*src_buffer, SYNC_COPY_TRANSFER_READ, src_range);
            if (hazard.IsHazard()) {
                skip |= LogError(srcBuffer, string_SyncHazardVUID(hazard.Hazard()),
                                 "vkCmdCopyBuffer: Hazard %s for srcBuffer %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.Hazard()), FormatHandle(srcBuffer).c_str(), region,
                                 cb_context->FormatHazard(hazard).c_str());
            }
        }
        if (dst_buffer && !skip) {
            const ResourceAccessRange dst_range = MakeRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, dst_range);
            if (hazard.IsHazard()) {
                skip |= LogError(dstBuffer, string_SyncHazardVUID(hazard.Hazard()),
                                 "vkCmdCopyBuffer: Hazard %s for dstBuffer %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.Hazard()), FormatHandle(dstBuffer).c_str(), region,
                                 cb_context->FormatHazard(hazard).c_str());
            }
        }
        if (skip) break;
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                               uint32_t regionCount, const VkBufferCopy *pRegions) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;
    const auto tag = cb_context->NextCommandTag(Func::vkCmdCopyBuffer);
    auto *context = cb_context->GetCurrentAccessContext();

    auto src_buffer = Get<BUFFER_STATE>(srcBuffer);
    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_buffer) {
            const ResourceAccessRange src_range = MakeRange(*src_buffer, copy_region.srcOffset, copy_region.size);
            context->UpdateAccessState(*src_buffer, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment, src_range, tag);
        }
        if (dst_buffer) {
            const ResourceAccessRange dst_range = MakeRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, dst_range, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;
    const auto *context = cb_context->GetCurrentAccessContext();

    // If we have no previous accesses, we have no hazards
    auto src_buffer = Get<BUFFER_STATE>(pCopyBufferInfo->srcBuffer);
    auto dst_buffer = Get<BUFFER_STATE>(pCopyBufferInfo->dstBuffer);

    for (uint32_t region = 0; region < pCopyBufferInfo->regionCount; region++) {
        const auto &copy_region = pCopyBufferInfo->pRegions[region];
        if (src_buffer) {
            const ResourceAccessRange src_range = MakeRange(*src_buffer, copy_region.srcOffset, copy_region.size);
            auto hazard = context->DetectHazard(*src_buffer, SYNC_COPY_TRANSFER_READ, src_range);
            if (hazard.IsHazard()) {
                // TODO -- add tag information to log msg when useful.
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), pCopyBufferInfo->srcBuffer, error_obj.location,
                             "Hazard %s for srcBuffer %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(pCopyBufferInfo->srcBuffer).c_str(), region, cb_context->FormatHazard(hazard).c_str());
            }
        }
        if (dst_buffer && !skip) {
            const ResourceAccessRange dst_range = MakeRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, dst_range);
            if (hazard.IsHazard()) {
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), pCopyBufferInfo->dstBuffer, error_obj.location,
                             "Hazard %s for dstBuffer %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(pCopyBufferInfo->dstBuffer).c_str(), region, cb_context->FormatHazard(hazard).c_str());
            }
        }
        if (skip) break;
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo,
                                                     const ErrorObject &error_obj) const {
    return PreCallValidateCmdCopyBuffer2(commandBuffer, pCopyBufferInfo, error_obj);
}

void SyncValidator::RecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;
    const auto tag = cb_context->NextCommandTag(command);
    auto *context = cb_context->GetCurrentAccessContext();

    auto src_buffer = Get<BUFFER_STATE>(pCopyBufferInfo->srcBuffer);
    auto dst_buffer = Get<BUFFER_STATE>(pCopyBufferInfo->dstBuffer);

    for (uint32_t region = 0; region < pCopyBufferInfo->regionCount; region++) {
        const auto &copy_region = pCopyBufferInfo->pRegions[region];
        if (src_buffer) {
            const ResourceAccessRange src_range = MakeRange(*src_buffer, copy_region.srcOffset, copy_region.size);
            context->UpdateAccessState(*src_buffer, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment, src_range, tag);
        }
        if (dst_buffer) {
            const ResourceAccessRange dst_range = MakeRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, dst_range, tag);
        }
    }
}

void SyncValidator::PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo) {
    RecordCmdCopyBuffer2(commandBuffer, pCopyBufferInfo, Func::vkCmdCopyBuffer2KHR);
}

void SyncValidator::PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo) {
    RecordCmdCopyBuffer2(commandBuffer, pCopyBufferInfo, Func::vkCmdCopyBuffer2);
}

bool SyncValidator::PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkImageCopy *pRegions, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_image = Get<ImageState>(srcImage);
    auto dst_image = Get<ImageState>(dstImage);
    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, SYNC_COPY_TRANSFER_READ, copy_region.srcSubresource,
                                                copy_region.srcOffset, copy_region.extent, false);
            if (hazard.IsHazard()) {
                skip |= LogError(srcImage, string_SyncHazardVUID(hazard.Hazard()),
                                 "vkCmdCopyImage: Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.Hazard()), FormatHandle(srcImage).c_str(), region,
                                 cb_access_context->FormatHazard(hazard).c_str());
            }
        }

        if (dst_image) {
            auto hazard = context->DetectHazard(*dst_image, SYNC_COPY_TRANSFER_WRITE, copy_region.dstSubresource,
                                                copy_region.dstOffset, copy_region.extent, false);
            if (hazard.IsHazard()) {
                skip |= LogError(dstImage, string_SyncHazardVUID(hazard.Hazard()),
                                 "vkCmdCopyImage: Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.Hazard()), FormatHandle(dstImage).c_str(), region,
                                 cb_access_context->FormatHazard(hazard).c_str());
            }
            if (skip) break;
        }
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                              const VkImageCopy *pRegions) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(Func::vkCmdCopyImage);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<ImageState>(srcImage);
    auto dst_image = Get<ImageState>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                       copy_region.srcSubresource, copy_region.srcOffset, copy_region.extent, tag);
        }
        if (dst_image) {
            context->UpdateAccessState(*dst_image, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       copy_region.dstSubresource, copy_region.dstOffset, copy_region.extent, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_image = Get<ImageState>(pCopyImageInfo->srcImage);
    auto dst_image = Get<ImageState>(pCopyImageInfo->dstImage);

    for (uint32_t region = 0; region < pCopyImageInfo->regionCount; region++) {
        const auto &copy_region = pCopyImageInfo->pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, SYNC_COPY_TRANSFER_READ, copy_region.srcSubresource,
                                                copy_region.srcOffset, copy_region.extent, false);
            if (hazard.IsHazard()) {
                skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), pCopyImageInfo->srcImage, error_obj.location,
                                 "Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.Hazard()), FormatHandle(pCopyImageInfo->srcImage).c_str(), region,
                                 cb_access_context->FormatHazard(hazard).c_str());
            }
        }

        if (dst_image) {
            auto hazard = context->DetectHazard(*dst_image, SYNC_COPY_TRANSFER_WRITE, copy_region.dstSubresource,
                                                copy_region.dstOffset, copy_region.extent, false);
            if (hazard.IsHazard()) {
                skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), pCopyImageInfo->dstImage, error_obj.location,
                                 "Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.Hazard()), FormatHandle(pCopyImageInfo->dstImage).c_str(), region,
                                 cb_access_context->FormatHazard(hazard).c_str());
            }
            if (skip) break;
        }
    }

    return skip;
}

bool SyncValidator::PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo,
                                                    const ErrorObject &error_obj) const {
    return PreCallValidateCmdCopyImage2(commandBuffer, pCopyImageInfo, error_obj);
}

void SyncValidator::RecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(command);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<ImageState>(pCopyImageInfo->srcImage);
    auto dst_image = Get<ImageState>(pCopyImageInfo->dstImage);

    for (uint32_t region = 0; region < pCopyImageInfo->regionCount; region++) {
        const auto &copy_region = pCopyImageInfo->pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                       copy_region.srcSubresource, copy_region.srcOffset, copy_region.extent, tag);
        }
        if (dst_image) {
            context->UpdateAccessState(*dst_image, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       copy_region.dstSubresource, copy_region.dstOffset, copy_region.extent, tag);
        }
    }
}

void SyncValidator::PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo) {
    RecordCmdCopyImage2(commandBuffer, pCopyImageInfo, Func::vkCmdCopyImage2KHR);
}

void SyncValidator::PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo) {
    RecordCmdCopyImage2(commandBuffer, pCopyImageInfo, Func::vkCmdCopyImage2);
}

bool SyncValidator::PreCallValidateCmdPipelineBarrier(
    VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier *pImageMemoryBarriers, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    SyncOpPipelineBarrier pipeline_barrier(error_obj.location.function, *this, cb_access_context->GetQueueFlags(), srcStageMask,
                                           dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers,
                                           bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
                                           pImageMemoryBarriers);
    skip = pipeline_barrier.Validate(*cb_access_context);
    return skip;
}

void SyncValidator::PreCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                    VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                    uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                    uint32_t bufferMemoryBarrierCount,
                                                    const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                    uint32_t imageMemoryBarrierCount,
                                                    const VkImageMemoryBarrier *pImageMemoryBarriers) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;

    cb_access_context->RecordSyncOp<SyncOpPipelineBarrier>(Func::vkCmdPipelineBarrier, *this, cb_access_context->GetQueueFlags(),
                                                           srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount,
                                                           pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                                                           imageMemoryBarrierCount, pImageMemoryBarriers);
}

bool SyncValidator::PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo,
                                                          const ErrorObject &error_obj) const {
    return PreCallValidateCmdPipelineBarrier2(commandBuffer, pDependencyInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo,
                                                       const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    SyncOpPipelineBarrier pipeline_barrier(error_obj.location.function, *this, cb_access_context->GetQueueFlags(),
                                           *pDependencyInfo);
    skip = pipeline_barrier.Validate(*cb_access_context);
    return skip;
}

void SyncValidator::PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;

    cb_access_context->RecordSyncOp<SyncOpPipelineBarrier>(Func::vkCmdPipelineBarrier2KHR, *this,
                                                           cb_access_context->GetQueueFlags(), *pDependencyInfo);
}

void SyncValidator::PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;

    cb_access_context->RecordSyncOp<SyncOpPipelineBarrier>(Func::vkCmdPipelineBarrier2, *this, cb_access_context->GetQueueFlags(),
                                                           *pDependencyInfo);
}

void SyncValidator::CreateDevice(const VkDeviceCreateInfo *pCreateInfo) {
    // The state tracker sets up the device state
    StateTracker::CreateDevice(pCreateInfo);

    ForEachShared<QUEUE_STATE>([this](const std::shared_ptr<QUEUE_STATE> &queue_state) {
        auto queue_flags = physical_device_state->queue_family_properties[queue_state->queueFamilyIndex].queueFlags;
        std::shared_ptr<QueueSyncState> queue_sync_state =
            std::make_shared<QueueSyncState>(queue_state, queue_flags, queue_id_limit_++);
        queue_sync_states_.emplace(std::make_pair(queue_state->Queue(), std::move(queue_sync_state)));
    });
}

bool SyncValidator::ValidateBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                            const VkSubpassBeginInfo *pSubpassBeginInfo, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    if (cb_state) {
        SyncOpBeginRenderPass sync_op(error_obj.location.function, *this, pRenderPassBegin, pSubpassBeginInfo);
        skip = sync_op.Validate(cb_state->access_context);
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                      VkSubpassContents contents, const ErrorObject &error_obj) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents, error_obj);
    VkSubpassBeginInfo subpass_begin_info = vku::InitStructHelper();
    subpass_begin_info.contents = contents;
    skip |= ValidateBeginRenderPass(commandBuffer, pRenderPassBegin, &subpass_begin_info, error_obj);
    return skip;
}

bool SyncValidator::PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                       const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                       const ErrorObject &error_obj) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, error_obj);
    skip |= ValidateBeginRenderPass(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, error_obj);
    return skip;
}

bool SyncValidator::PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                          const VkRenderPassBeginInfo *pRenderPassBegin,
                                                          const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                          const ErrorObject &error_obj) const {
    return PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, error_obj);
}

void SyncValidator::PostCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo,
                                                     const RecordObject &record_obj) {
    // The state tracker sets up the command buffer state
    StateTracker::PostCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo, record_obj);

    // Create/initialize the structure that trackers accesses at the command buffer scope.
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    cb_state->access_context.Reset();
}

void SyncValidator::RecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                             const VkSubpassBeginInfo *pSubpassBeginInfo, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    if (cb_state) {
        cb_state->access_context.RecordSyncOp<SyncOpBeginRenderPass>(command, *this, pRenderPassBegin, pSubpassBeginInfo);
    }
}

void SyncValidator::PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                     VkSubpassContents contents, const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents, record_obj);
    VkSubpassBeginInfo subpass_begin_info = vku::InitStructHelper();
    subpass_begin_info.contents = contents;
    RecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, &subpass_begin_info, record_obj.location.function);
}

void SyncValidator::PostCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                      const VkSubpassBeginInfo *pSubpassBeginInfo, const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, record_obj);
    RecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, record_obj.location.function);
}

void SyncValidator::PostCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                         const VkRenderPassBeginInfo *pRenderPassBegin,
                                                         const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                         const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, record_obj);
    RecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, record_obj.location.function);
}

bool SyncValidator::ValidateCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                           const VkSubpassEndInfo *pSubpassEndInfo, const ErrorObject &error_obj) const {
    bool skip = false;

    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;
    SyncOpNextSubpass sync_op(error_obj.location.function, *this, pSubpassBeginInfo, pSubpassEndInfo);
    return sync_op.Validate(*cb_context);
}

bool SyncValidator::PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                                  const ErrorObject &error_obj) const {
    bool skip = StateTracker::PreCallValidateCmdNextSubpass(commandBuffer, contents, error_obj);
    // Convert to a NextSubpass2
    VkSubpassBeginInfo subpass_begin_info = vku::InitStructHelper();
    subpass_begin_info.contents = contents;
    VkSubpassEndInfo subpass_end_info = vku::InitStructHelper();
    skip |= ValidateCmdNextSubpass(commandBuffer, &subpass_begin_info, &subpass_end_info, error_obj);
    return skip;
}

bool SyncValidator::PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                      const VkSubpassEndInfo *pSubpassEndInfo, const ErrorObject &error_obj) const {
    return PreCallValidateCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                   const VkSubpassEndInfo *pSubpassEndInfo, const ErrorObject &error_obj) const {
    bool skip = StateTracker::PreCallValidateCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, error_obj);
    skip |= ValidateCmdNextSubpass(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, error_obj);
    return skip;
}

void SyncValidator::RecordCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                         const VkSubpassEndInfo *pSubpassEndInfo, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

    cb_context->RecordSyncOp<SyncOpNextSubpass>(command, *this, pSubpassBeginInfo, pSubpassEndInfo);
}

void SyncValidator::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                                 const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdNextSubpass(commandBuffer, contents, record_obj);
    VkSubpassBeginInfo subpass_begin_info = vku::InitStructHelper();
    subpass_begin_info.contents = contents;
    RecordCmdNextSubpass(commandBuffer, &subpass_begin_info, nullptr, record_obj.location.function);
}

void SyncValidator::PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                  const VkSubpassEndInfo *pSubpassEndInfo, const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, record_obj);
    RecordCmdNextSubpass(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, record_obj.location.function);
}

void SyncValidator::PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                     const VkSubpassEndInfo *pSubpassEndInfo, const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, record_obj);
    RecordCmdNextSubpass(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, record_obj.location.function);
}

bool SyncValidator::ValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                             const ErrorObject &error_obj) const {
    bool skip = false;

    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    auto *cb_context = &cb_state->access_context;

    SyncOpEndRenderPass sync_op(error_obj.location.function, *this, pSubpassEndInfo);
    skip |= sync_op.Validate(*cb_context);
    return skip;
}

bool SyncValidator::PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const {
    bool skip = StateTracker::PreCallValidateCmdEndRenderPass(commandBuffer, error_obj);
    skip |= ValidateCmdEndRenderPass(commandBuffer, nullptr, error_obj);
    return skip;
}

bool SyncValidator::PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                     const ErrorObject &error_obj) const {
    bool skip = StateTracker::PreCallValidateCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, error_obj);
    skip |= ValidateCmdEndRenderPass(commandBuffer, pSubpassEndInfo, error_obj);
    return skip;
}

bool SyncValidator::PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                        const ErrorObject &error_obj) const {
    return PreCallValidateCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, error_obj);
}

void SyncValidator::RecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo, Func command) {
    // Resolve the all subpass contexts to the command buffer contexts
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

    cb_context->RecordSyncOp<SyncOpEndRenderPass>(command, *this, pSubpassEndInfo);
}

// Simple heuristic rule to detect WAW operations representing algorithmically safe or increment
// updates to a resource which do not conflict at the byte level.
// TODO: Revisit this rule to see if it needs to be tighter or looser
// TODO: Add programatic control over suppression heuristics
bool SyncValidator::SupressedBoundDescriptorWAW(const HazardResult &hazard) const {
    assert(hazard.IsHazard());
    return hazard.IsWAWHazard();
}

void SyncValidator::PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    RecordCmdEndRenderPass(commandBuffer, nullptr, record_obj.location.function);
    StateTracker::PostCallRecordCmdEndRenderPass(commandBuffer, record_obj);
}

void SyncValidator::PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                    const RecordObject &record_obj) {
    RecordCmdEndRenderPass(commandBuffer, pSubpassEndInfo, record_obj.location.function);
    StateTracker::PostCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, record_obj);
}

void SyncValidator::PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                       const RecordObject &record_obj) {
    RecordCmdEndRenderPass(commandBuffer, pSubpassEndInfo, record_obj.location.function);
    StateTracker::PostCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo, record_obj);
}

template <typename RegionType>
bool SyncValidator::ValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                 VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions,
                                                 const Location &loc) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_buffer = Get<BUFFER_STATE>(srcBuffer);
    auto dst_image = Get<ImageState>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        HazardResult hazard;
        if (dst_image) {
            if (src_buffer) {
                ResourceAccessRange src_range = MakeRange(
                    copy_region.bufferOffset,
                    GetBufferSizeFromCopyImage(copy_region, dst_image->createInfo.format, dst_image->createInfo.arrayLayers));
                hazard = context->DetectHazard(*src_buffer, SYNC_COPY_TRANSFER_READ, src_range);
                if (hazard.IsHazard()) {
                    // PHASE1 TODO -- add tag information to log msg when useful.
                    skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), srcBuffer, loc,
                                     "Hazard %s for srcBuffer %s, region %" PRIu32 ". Access info %s.",
                                     string_SyncHazard(hazard.Hazard()), FormatHandle(srcBuffer).c_str(), region,
                                     cb_access_context->FormatHazard(hazard).c_str());
                }
            }

            hazard = context->DetectHazard(*dst_image, SYNC_COPY_TRANSFER_WRITE, copy_region.imageSubresource,
                                           copy_region.imageOffset, copy_region.imageExtent, false);
            if (hazard.IsHazard()) {
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), dstImage, loc,
                             "Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(dstImage).c_str(), region, cb_access_context->FormatHazard(hazard).c_str());
            }
            if (skip) break;
        }
        if (skip) break;
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                        VkImageLayout dstImageLayout, uint32_t regionCount,
                                                        const VkBufferImageCopy *pRegions, const ErrorObject &error_obj) const {
    return ValidateCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions,
                                        error_obj.location);
}

bool SyncValidator::PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                            const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo,
                                                            const ErrorObject &error_obj) const {
    return PreCallValidateCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                         const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo,
                                                         const ErrorObject &error_obj) const {
    return ValidateCmdCopyBufferToImage(commandBuffer, pCopyBufferToImageInfo->srcBuffer, pCopyBufferToImageInfo->dstImage,
                                        pCopyBufferToImageInfo->dstImageLayout, pCopyBufferToImageInfo->regionCount,
                                        pCopyBufferToImageInfo->pRegions, error_obj.location.dot(Field::pCopyBufferToImageInfo));
}

template <typename RegionType>
void SyncValidator::RecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                               VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions,
                                               Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;

    const auto tag = cb_access_context->NextCommandTag(command);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_buffer = Get<BUFFER_STATE>(srcBuffer);
    auto dst_image = Get<ImageState>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (dst_image) {
            if (src_buffer) {
                ResourceAccessRange src_range = MakeRange(
                    copy_region.bufferOffset,
                    GetBufferSizeFromCopyImage(copy_region, dst_image->createInfo.format, dst_image->createInfo.arrayLayers));
                context->UpdateAccessState(*src_buffer, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment, src_range, tag);
            }
            context->UpdateAccessState(*dst_image, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       copy_region.imageSubresource, copy_region.imageOffset, copy_region.imageExtent, tag);
        }
    }
}

void SyncValidator::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                      VkImageLayout dstImageLayout, uint32_t regionCount,
                                                      const VkBufferImageCopy *pRegions) {
    StateTracker::PreCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    RecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions,
                               Func::vkCmdCopyBufferToImage);
}

void SyncValidator::PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                          const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo) {
    StateTracker::PreCallRecordCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
    RecordCmdCopyBufferToImage(commandBuffer, pCopyBufferToImageInfo->srcBuffer, pCopyBufferToImageInfo->dstImage,
                               pCopyBufferToImageInfo->dstImageLayout, pCopyBufferToImageInfo->regionCount,
                               pCopyBufferToImageInfo->pRegions, Func::vkCmdCopyBufferToImage2KHR);
}

void SyncValidator::PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                       const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo) {
    StateTracker::PreCallRecordCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo);
    RecordCmdCopyBufferToImage(commandBuffer, pCopyBufferToImageInfo->srcBuffer, pCopyBufferToImageInfo->dstImage,
                               pCopyBufferToImageInfo->dstImageLayout, pCopyBufferToImageInfo->regionCount,
                               pCopyBufferToImageInfo->pRegions, Func::vkCmdCopyBufferToImage2);
}

template <typename RegionType>
bool SyncValidator::ValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                 VkBuffer dstBuffer, uint32_t regionCount, const RegionType *pRegions,
                                                 const Location &loc) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_image = Get<ImageState>(srcImage);
    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);
    const auto dst_mem = (dst_buffer && !dst_buffer->sparse) ? dst_buffer->MemState()->deviceMemory() : VK_NULL_HANDLE;
    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, SYNC_COPY_TRANSFER_READ, copy_region.imageSubresource,
                                                copy_region.imageOffset, copy_region.imageExtent, false);
            if (hazard.IsHazard()) {
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), srcImage, loc,
                             "Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(srcImage).c_str(), region, cb_access_context->FormatHazard(hazard).c_str());
            }
            if (dst_mem) {
                ResourceAccessRange dst_range = MakeRange(
                    copy_region.bufferOffset,
                    GetBufferSizeFromCopyImage(copy_region, src_image->createInfo.format, src_image->createInfo.arrayLayers));
                hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, dst_range);
                if (hazard.IsHazard()) {
                    skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), dstBuffer, loc,
                                     "Hazard %s for dstBuffer %s, region %" PRIu32 ". Access info %s.",
                                     string_SyncHazard(hazard.Hazard()), FormatHandle(dstBuffer).c_str(), region,
                                     cb_access_context->FormatHazard(hazard).c_str());
                }
            }
        }
        if (skip) break;
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                        VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount,
                                                        const VkBufferImageCopy *pRegions, const ErrorObject &error_obj) const {
    return ValidateCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions,
                                        error_obj.location);
}

bool SyncValidator::PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                            const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo,
                                                            const ErrorObject &error_obj) const {
    return PreCallValidateCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                         const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo,
                                                         const ErrorObject &error_obj) const {
    return ValidateCmdCopyImageToBuffer(commandBuffer, pCopyImageToBufferInfo->srcImage, pCopyImageToBufferInfo->srcImageLayout,
                                        pCopyImageToBufferInfo->dstBuffer, pCopyImageToBufferInfo->regionCount,
                                        pCopyImageToBufferInfo->pRegions, error_obj.location.dot(Field::pCopyImageToBufferInfo));
}

template <typename RegionType>
void SyncValidator::RecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                               VkBuffer dstBuffer, uint32_t regionCount, const RegionType *pRegions, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;

    const auto tag = cb_access_context->NextCommandTag(command);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<ImageState>(srcImage);
    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);
    const auto dst_mem = (dst_buffer && !dst_buffer->sparse) ? dst_buffer->MemState()->deviceMemory() : VK_NULL_HANDLE;
    const VulkanTypedHandle dst_handle(dst_mem, kVulkanObjectTypeDeviceMemory);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                       copy_region.imageSubresource, copy_region.imageOffset, copy_region.imageExtent, tag);
            if (dst_buffer) {
                ResourceAccessRange dst_range = MakeRange(
                    copy_region.bufferOffset,
                    GetBufferSizeFromCopyImage(copy_region, src_image->createInfo.format, src_image->createInfo.arrayLayers));
                context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, dst_range, tag);
            }
        }
    }
}

void SyncValidator::PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                      VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions) {
    StateTracker::PreCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    RecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions,
                               Func::vkCmdCopyImageToBuffer);
}

void SyncValidator::PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                          const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo) {
    StateTracker::PreCallRecordCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
    RecordCmdCopyImageToBuffer(commandBuffer, pCopyImageToBufferInfo->srcImage, pCopyImageToBufferInfo->srcImageLayout,
                               pCopyImageToBufferInfo->dstBuffer, pCopyImageToBufferInfo->regionCount,
                               pCopyImageToBufferInfo->pRegions, Func::vkCmdCopyImageToBuffer2KHR);
}

void SyncValidator::PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                       const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo) {
    StateTracker::PreCallRecordCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo);
    RecordCmdCopyImageToBuffer(commandBuffer, pCopyImageToBufferInfo->srcImage, pCopyImageToBufferInfo->srcImageLayout,
                               pCopyImageToBufferInfo->dstBuffer, pCopyImageToBufferInfo->regionCount,
                               pCopyImageToBufferInfo->pRegions, Func::vkCmdCopyImageToBuffer2);
}

template <typename RegionType>
bool SyncValidator::ValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                         VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                         const RegionType *pRegions, VkFilter filter, const Location &loc) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_image = Get<ImageState>(srcImage);
    auto dst_image = Get<ImageState>(dstImage);

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
                context->DetectHazard(*src_image, SYNC_BLIT_TRANSFER_READ, blit_region.srcSubresource, offset, extent, false);
            if (hazard.IsHazard()) {
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), srcImage, loc,
                             "Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(srcImage).c_str(), region, cb_access_context->FormatHazard(hazard).c_str());
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
                context->DetectHazard(*dst_image, SYNC_BLIT_TRANSFER_WRITE, blit_region.dstSubresource, offset, extent, false);
            if (hazard.IsHazard()) {
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), dstImage, loc,
                             "Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(dstImage).c_str(), region, cb_access_context->FormatHazard(hazard).c_str());
            }
            if (skip) break;
        }
    }

    return skip;
}

bool SyncValidator::PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkImageBlit *pRegions, VkFilter filter, const ErrorObject &error_obj) const {
    return ValidateCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter,
                                error_obj.location);
}

bool SyncValidator::PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo,
                                                    const ErrorObject &error_obj) const {
    return PreCallValidateCmdBlitImage2(commandBuffer, pBlitImageInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo,
                                                 const ErrorObject &error_obj) const {
    return ValidateCmdBlitImage(commandBuffer, pBlitImageInfo->srcImage, pBlitImageInfo->srcImageLayout, pBlitImageInfo->dstImage,
                                pBlitImageInfo->dstImageLayout, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
                                pBlitImageInfo->filter, error_obj.location.dot(Field::pBlitImageInfo));
}

template <typename RegionType>
void SyncValidator::RecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                       VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                       const RegionType *pRegions, VkFilter filter, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    const auto tag = cb_state->access_context.NextCommandTag(command);
    auto *context = cb_state->access_context.GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<ImageState>(srcImage);
    auto dst_image = Get<ImageState>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &blit_region = pRegions[region];
        if (src_image) {
            VkOffset3D offset = {std::min(blit_region.srcOffsets[0].x, blit_region.srcOffsets[1].x),
                                 std::min(blit_region.srcOffsets[0].y, blit_region.srcOffsets[1].y),
                                 std::min(blit_region.srcOffsets[0].z, blit_region.srcOffsets[1].z)};
            VkExtent3D extent = {static_cast<uint32_t>(abs(blit_region.srcOffsets[1].x - blit_region.srcOffsets[0].x)),
                                 static_cast<uint32_t>(abs(blit_region.srcOffsets[1].y - blit_region.srcOffsets[0].y)),
                                 static_cast<uint32_t>(abs(blit_region.srcOffsets[1].z - blit_region.srcOffsets[0].z))};
            context->UpdateAccessState(*src_image, SYNC_BLIT_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                       blit_region.srcSubresource, offset, extent, tag);
        }
        if (dst_image) {
            VkOffset3D offset = {std::min(blit_region.dstOffsets[0].x, blit_region.dstOffsets[1].x),
                                 std::min(blit_region.dstOffsets[0].y, blit_region.dstOffsets[1].y),
                                 std::min(blit_region.dstOffsets[0].z, blit_region.dstOffsets[1].z)};
            VkExtent3D extent = {static_cast<uint32_t>(abs(blit_region.dstOffsets[1].x - blit_region.dstOffsets[0].x)),
                                 static_cast<uint32_t>(abs(blit_region.dstOffsets[1].y - blit_region.dstOffsets[0].y)),
                                 static_cast<uint32_t>(abs(blit_region.dstOffsets[1].z - blit_region.dstOffsets[0].z))};
            context->UpdateAccessState(*dst_image, SYNC_BLIT_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       blit_region.dstSubresource, offset, extent, tag);
        }
    }
}

void SyncValidator::PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                              const VkImageBlit *pRegions, VkFilter filter) {
    StateTracker::PreCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                            pRegions, filter);
    RecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter,
                       Func::vkCmdBlitImage);
}

void SyncValidator::PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo) {
    StateTracker::PreCallRecordCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
    RecordCmdBlitImage(commandBuffer, pBlitImageInfo->srcImage, pBlitImageInfo->srcImageLayout, pBlitImageInfo->dstImage,
                       pBlitImageInfo->dstImageLayout, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
                       pBlitImageInfo->filter, Func::vkCmdBlitImage2KHR);
}

void SyncValidator::PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo) {
    StateTracker::PreCallRecordCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
    RecordCmdBlitImage(commandBuffer, pBlitImageInfo->srcImage, pBlitImageInfo->srcImageLayout, pBlitImageInfo->dstImage,
                       pBlitImageInfo->dstImageLayout, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
                       pBlitImageInfo->filter, Func::vkCmdBlitImage2);
}

bool SyncValidator::ValidateIndirectBuffer(const CommandBufferAccessContext &cb_context, const AccessContext &context,
                                           VkCommandBuffer commandBuffer, const VkDeviceSize struct_size, const VkBuffer buffer,
                                           const VkDeviceSize offset, const uint32_t drawCount, const uint32_t stride,
                                           const Location &loc) const {
    bool skip = false;
    if (drawCount == 0) return skip;

    auto buf_state = Get<BUFFER_STATE>(buffer);
    VkDeviceSize size = struct_size;
    if (drawCount == 1 || stride == size) {
        if (drawCount > 1) size *= drawCount;
        const ResourceAccessRange range = MakeRange(offset, size);
        auto hazard = context.DetectHazard(*buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, range);
        if (hazard.IsHazard()) {
            skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), buf_state->buffer(), loc,
                             "Hazard %s for indirect %s in %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(buffer).c_str(), FormatHandle(commandBuffer).c_str(),
                             cb_context.FormatHazard(hazard).c_str());
        }
    } else {
        for (uint32_t i = 0; i < drawCount; ++i) {
            const ResourceAccessRange range = MakeRange(offset + i * stride, size);
            auto hazard = context.DetectHazard(*buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, range);
            if (hazard.IsHazard()) {
                skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), buf_state->buffer(), loc,
                                 "Hazard %s for indirect %s in %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                                 FormatHandle(buffer).c_str(), FormatHandle(commandBuffer).c_str(),
                                 cb_context.FormatHazard(hazard).c_str());
                break;
            }
        }
    }
    return skip;
}

void SyncValidator::RecordIndirectBuffer(AccessContext &context, const ResourceUsageTag tag, const VkDeviceSize struct_size,
                                         const VkBuffer buffer, const VkDeviceSize offset, const uint32_t drawCount,
                                         uint32_t stride) {
    auto buf_state = Get<BUFFER_STATE>(buffer);
    VkDeviceSize size = struct_size;
    if (drawCount == 1 || stride == size) {
        if (drawCount > 1) size *= drawCount;
        const ResourceAccessRange range = MakeRange(offset, size);
        context.UpdateAccessState(*buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, SyncOrdering::kNonAttachment, range, tag);
    } else {
        for (uint32_t i = 0; i < drawCount; ++i) {
            const ResourceAccessRange range = MakeRange(offset + i * stride, size);
            context.UpdateAccessState(*buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, SyncOrdering::kNonAttachment, range,
                                      tag);
        }
    }
}

bool SyncValidator::ValidateCountBuffer(const CommandBufferAccessContext &cb_context, const AccessContext &context,
                                        VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                        const Location &loc) const {
    bool skip = false;

    auto count_buf_state = Get<BUFFER_STATE>(buffer);
    const ResourceAccessRange range = MakeRange(offset, 4);
    auto hazard = context.DetectHazard(*count_buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, range);
    if (hazard.IsHazard()) {
        skip |=
            LogError(string_SyncHazardVUID(hazard.Hazard()), count_buf_state->buffer(), loc,
                     "Hazard %s for countBuffer %s in %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                     FormatHandle(buffer).c_str(), FormatHandle(commandBuffer).c_str(), cb_context.FormatHazard(hazard).c_str());
    }
    return skip;
}

void SyncValidator::RecordCountBuffer(AccessContext &context, const ResourceUsageTag tag, VkBuffer buffer, VkDeviceSize offset) {
    auto count_buf_state = Get<BUFFER_STATE>(buffer);
    const ResourceAccessRange range = MakeRange(offset, 4);
    context.UpdateAccessState(*count_buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, SyncOrdering::kNonAttachment, range, tag);
}

bool SyncValidator::PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                               const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;

    skip |= cb_state->access_context.ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, error_obj.location);
    return skip;
}

void SyncValidator::PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) {
    StateTracker::PreCallRecordCmdDispatch(commandBuffer, x, y, z);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(Func::vkCmdDispatch);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, tag);
}

bool SyncValidator::PreCallValidateCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                       const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;

    const auto *context = cb_state->access_context.GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_state->access_context.ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, error_obj.location);
    skip |= ValidateIndirectBuffer(cb_state->access_context, *context, commandBuffer, sizeof(VkDispatchIndirectCommand), buffer,
                                   offset, 1, sizeof(VkDispatchIndirectCommand), error_obj.location);
    return skip;
}

void SyncValidator::PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    StateTracker::PreCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(Func::vkCmdDispatchIndirect);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, tag);
    RecordIndirectBuffer(*context, tag, sizeof(VkDispatchIndirectCommand), buffer, offset, 1, sizeof(VkDispatchIndirectCommand));
}

bool SyncValidator::PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                           uint32_t firstVertex, uint32_t firstInstance, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawVertex(vertexCount, firstVertex, error_obj.location);
    skip |= cb_access_context->ValidateDrawSubpassAttachment(error_obj.location);
    return skip;
}

void SyncValidator::PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                         uint32_t firstVertex, uint32_t firstInstance) {
    StateTracker::PreCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(Func::vkCmdDraw);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawVertex(vertexCount, firstVertex, tag);
    cb_access_context->RecordDrawSubpassAttachment(tag);
}

bool SyncValidator::PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                  uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawVertexIndex(indexCount, firstIndex, error_obj.location);
    skip |= cb_access_context->ValidateDrawSubpassAttachment(error_obj.location);
    return skip;
}

void SyncValidator::PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    StateTracker::PreCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(Func::vkCmdDrawIndexed);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawVertexIndex(indexCount, firstIndex, tag);
    cb_access_context->RecordDrawSubpassAttachment(tag);
}

bool SyncValidator::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   uint32_t drawCount, uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    if (drawCount == 0) return skip;

    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawSubpassAttachment(error_obj.location);
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, commandBuffer, sizeof(VkDrawIndirectCommand), buffer, offset,
                                   drawCount, stride, error_obj.location);

    // TODO: For now, we validate the whole vertex buffer. It might cause some false positive.
    //       VkDrawIndirectCommand buffer could be changed until SubmitQueue.
    //       We will validate the vertex buffer in SubmitQueue in the future.
    skip |= cb_access_context->ValidateDrawVertex(std::optional<uint32_t>(), 0, error_obj.location);
    return skip;
}

void SyncValidator::PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride) {
    StateTracker::PreCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    if (drawCount == 0) return;
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(Func::vkCmdDrawIndirect);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawSubpassAttachment(tag);
    RecordIndirectBuffer(*context, tag, sizeof(VkDrawIndirectCommand), buffer, offset, drawCount, stride);

    // TODO: For now, we record the whole vertex buffer. It might cause some false positive.
    //       VkDrawIndirectCommand buffer could be changed until SubmitQueue.
    //       We will record the vertex buffer in SubmitQueue in the future.
    cb_access_context->RecordDrawVertex(std::optional<uint32_t>(), 0, tag);
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          uint32_t drawCount, uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    if (drawCount == 0) return skip;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawSubpassAttachment(error_obj.location);
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, commandBuffer, sizeof(VkDrawIndexedIndirectCommand), buffer,
                                   offset, drawCount, stride, error_obj.location);

    // TODO: For now, we validate the whole index and vertex buffer. It might cause some false positive.
    //       VkDrawIndexedIndirectCommand buffer could be changed until SubmitQueue.
    //       We will validate the index and vertex buffer in SubmitQueue in the future.
    skip |= cb_access_context->ValidateDrawVertexIndex(std::optional<uint32_t>(), 0, error_obj.location);
    return skip;
}

void SyncValidator::PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        uint32_t drawCount, uint32_t stride) {
    StateTracker::PreCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(Func::vkCmdDrawIndexedIndirect);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawSubpassAttachment(tag);
    RecordIndirectBuffer(*context, tag, sizeof(VkDrawIndexedIndirectCommand), buffer, offset, drawCount, stride);

    // TODO: For now, we record the whole index and vertex buffer. It might cause some false positive.
    //       VkDrawIndexedIndirectCommand buffer could be changed until SubmitQueue.
    //       We will record the index and vertex buffer in SubmitQueue in the future.
    cb_access_context->RecordDrawVertexIndex(std::optional<uint32_t>(), 0, tag);
}

bool SyncValidator::PreCallValidateCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawSubpassAttachment(error_obj.location);
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, commandBuffer, sizeof(VkDrawIndirectCommand), buffer, offset,
                                   maxDrawCount, stride, error_obj.location);
    skip |= ValidateCountBuffer(*cb_access_context, *context, commandBuffer, countBuffer, countBufferOffset, error_obj.location);

    // TODO: For now, we validate the whole vertex buffer. It might cause some false positive.
    //       VkDrawIndirectCommand buffer could be changed until SubmitQueue.
    //       We will validate the vertex buffer in SubmitQueue in the future.
    skip |= cb_access_context->ValidateDrawVertex(std::optional<uint32_t>(), 0, error_obj.location);
    return skip;
}

void SyncValidator::RecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                               uint32_t stride, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(command);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawSubpassAttachment(tag);
    RecordIndirectBuffer(*context, tag, sizeof(VkDrawIndirectCommand), buffer, offset, 1, stride);
    RecordCountBuffer(*context, tag, countBuffer, countBufferOffset);

    // TODO: For now, we record the whole vertex buffer. It might cause some false positive.
    //       VkDrawIndirectCommand buffer could be changed until SubmitQueue.
    //       We will record the vertex buffer in SubmitQueue in the future.
    cb_access_context->RecordDrawVertex(std::optional<uint32_t>(), 0, tag);
}

void SyncValidator::PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride) {
    StateTracker::PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                    stride);
    RecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                               Func::vkCmdDrawIndirectCount);
}
bool SyncValidator::PreCallValidateCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride,
                                                           const ErrorObject &error_obj) const {
    return PreCallValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                               error_obj);
}

void SyncValidator::PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride) {
    StateTracker::PreCallRecordCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                       stride);
    RecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                               Func::vkCmdDrawIndirectCountKHR);
}

bool SyncValidator::PreCallValidateCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                           uint32_t maxDrawCount, uint32_t stride,
                                                           const ErrorObject &error_obj) const {
    return PreCallValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                               error_obj);
}

void SyncValidator::PreCallRecordCmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                         VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                         uint32_t maxDrawCount, uint32_t stride) {
    StateTracker::PreCallRecordCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                       stride);
    RecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                               Func::vkCmdDrawIndirectCountAMD);
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride,
                                                               const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    skip |= cb_access_context->ValidateDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, error_obj.location);
    skip |= cb_access_context->ValidateDrawSubpassAttachment(error_obj.location);
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, commandBuffer, sizeof(VkDrawIndexedIndirectCommand), buffer,
                                   offset, maxDrawCount, stride, error_obj.location);
    skip |= ValidateCountBuffer(*cb_access_context, *context, commandBuffer, countBuffer, countBufferOffset, error_obj.location);

    // TODO: For now, we validate the whole index and vertex buffer. It might cause some false positive.
    //       VkDrawIndexedIndirectCommand buffer could be changed until SubmitQueue.
    //       We will validate the index and vertex buffer in SubmitQueue in the future.
    skip |= cb_access_context->ValidateDrawVertexIndex(std::optional<uint32_t>(), 0, error_obj.location);
    return skip;
}

void SyncValidator::RecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, Func command) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(command);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawSubpassAttachment(tag);
    RecordIndirectBuffer(*context, tag, sizeof(VkDrawIndexedIndirectCommand), buffer, offset, 1, stride);
    RecordCountBuffer(*context, tag, countBuffer, countBufferOffset);

    // TODO: For now, we record the whole index and vertex buffer. It might cause some false positive.
    //       VkDrawIndexedIndirectCommand buffer could be changed until SubmitQueue.
    //       We will update the index and vertex buffer in SubmitQueue in the future.
    cb_access_context->RecordDrawVertexIndex(std::optional<uint32_t>(), 0, tag);
}

void SyncValidator::PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                             uint32_t maxDrawCount, uint32_t stride) {
    StateTracker::PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                           maxDrawCount, stride);
    RecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      Func::vkCmdDrawIndexedIndirectCount);
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride, const ErrorObject &error_obj) const {
    return PreCallValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                      stride, error_obj);
}

void SyncValidator::PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride) {
    StateTracker::PreCallRecordCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                              maxDrawCount, stride);
    RecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      Func::vkCmdDrawIndexedIndirectCountKHR);
}

bool SyncValidator::PreCallValidateCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride, const ErrorObject &error_obj) const {
    return PreCallValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                      stride, error_obj);
}

void SyncValidator::PreCallRecordCmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                                VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                                uint32_t maxDrawCount, uint32_t stride) {
    StateTracker::PreCallRecordCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                              maxDrawCount, stride);
    RecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      Func::vkCmdDrawIndexedIndirectCountAMD);
}

bool SyncValidator::PreCallValidateCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                      const VkClearColorValue *pColor, uint32_t rangeCount,
                                                      const VkImageSubresourceRange *pRanges, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto image_state = Get<ImageState>(image);

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            auto hazard = context->DetectHazard(*image_state, SYNC_CLEAR_TRANSFER_WRITE, range, false);
            if (hazard.IsHazard()) {
                skip |= LogError(image, string_SyncHazardVUID(hazard.Hazard()),
                                 "vkCmdClearColorImage: Hazard %s for %s, range index %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.Hazard()), FormatHandle(image).c_str(), index,
                                 cb_access_context->FormatHazard(hazard).c_str());
            }
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                    const VkClearColorValue *pColor, uint32_t rangeCount,
                                                    const VkImageSubresourceRange *pRanges) {
    StateTracker::PreCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(Func::vkCmdClearColorImage);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto image_state = Get<ImageState>(image);

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            context->UpdateAccessState(*image_state, SYNC_CLEAR_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image,
                                                             VkImageLayout imageLayout,
                                                             const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                             const VkImageSubresourceRange *pRanges,
                                                             const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto image_state = Get<ImageState>(image);

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            auto hazard = context->DetectHazard(*image_state, SYNC_CLEAR_TRANSFER_WRITE, range, false);
            if (hazard.IsHazard()) {
                skip |= LogError(image, string_SyncHazardVUID(hazard.Hazard()),
                                 "vkCmdClearDepthStencilImage: Hazard %s for %s, range index %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.Hazard()), FormatHandle(image).c_str(), index,
                                 cb_access_context->FormatHazard(hazard).c_str());
            }
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                           const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                           const VkImageSubresourceRange *pRanges) {
    StateTracker::PreCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(Func::vkCmdClearDepthStencilImage);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto image_state = Get<ImageState>(image);

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            context->UpdateAccessState(*image_state, SYNC_CLEAR_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                       const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                       const VkClearRect *pRects, const ErrorObject &error_obj) const {
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    const auto cb_access_context = &cb_state->access_context;
    const auto rp_access_context = cb_access_context->GetCurrentRenderPassContext();
    if (!rp_access_context) return false;

    bool skip = false;
    for (const auto &attachment : vvl::make_span(pAttachments, attachmentCount)) {
        for (const auto &rect : vvl::make_span(pRects, rectCount)) {
            const auto rect_index = static_cast<uint32_t>(&rect - pRects);
            skip |= rp_access_context->ValidateClearAttachment(cb_access_context->GetExecutionContext(), *cb_state,
                                                               error_obj.location, attachment, rect, rect_index);
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                     const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                     const VkClearRect *pRects) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    auto cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(Func::vkCmdClearAttachments);
    const auto rp_access_context = cb_access_context->GetCurrentRenderPassContext();
    if (!rp_access_context) return;

    for (const auto &attachment : vvl::make_span(pAttachments, attachmentCount)) {
        for (const auto &rect : vvl::make_span(pRects, rectCount)) {
            rp_access_context->RecordClearAttachment(*cb_state, tag, attachment, rect);
        }
    }
}

bool SyncValidator::PreCallValidateCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                           uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer,
                                                           VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags,
                                                           const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, stride * queryCount);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, range);
        if (hazard.IsHazard()) {
            skip |= LogError(dstBuffer, string_SyncHazardVUID(hazard.Hazard()),
                             "vkCmdCopyQueryPoolResults: Hazard %s for dstBuffer %s. Access info %s.",
                             string_SyncHazard(hazard.Hazard()), FormatHandle(dstBuffer).c_str(),
                             cb_access_context->FormatHazard(hazard).c_str());
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
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(Func::vkCmdCopyQueryPoolResults);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, stride * queryCount);
        context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
    }

    // TODO:Track VkQueryPool
}

bool SyncValidator::PreCallValidateCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                 VkDeviceSize size, uint32_t data, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(*dst_buffer, dstOffset, size);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, range);
        if (hazard.IsHazard()) {
            skip |= LogError(dstBuffer, string_SyncHazardVUID(hazard.Hazard()),
                             "vkCmdFillBuffer: Hazard %s for dstBuffer %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(dstBuffer).c_str(), cb_access_context->FormatHazard(hazard).c_str());
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                               VkDeviceSize size, uint32_t data) {
    StateTracker::PreCallRecordCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(Func::vkCmdFillBuffer);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(*dst_buffer, dstOffset, size);
        context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
    }
}

bool SyncValidator::PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                   VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                   const VkImageResolve *pRegions, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_image = Get<ImageState>(srcImage);
    auto dst_image = Get<ImageState>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &resolve_region = pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, SYNC_RESOLVE_TRANSFER_READ, resolve_region.srcSubresource,
                                                resolve_region.srcOffset, resolve_region.extent, false);
            if (hazard.IsHazard()) {
                skip |= LogError(srcImage, string_SyncHazardVUID(hazard.Hazard()),
                                 "vkCmdResolveImage: Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.Hazard()), FormatHandle(srcImage).c_str(), region,
                                 cb_access_context->FormatHazard(hazard).c_str());
            }
        }

        if (dst_image) {
            auto hazard = context->DetectHazard(*dst_image, SYNC_RESOLVE_TRANSFER_WRITE, resolve_region.dstSubresource,
                                                resolve_region.dstOffset, resolve_region.extent, false);
            if (hazard.IsHazard()) {
                skip |= LogError(dstImage, string_SyncHazardVUID(hazard.Hazard()),
                                 "vkCmdResolveImage: Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.Hazard()), FormatHandle(dstImage).c_str(), region,
                                 cb_access_context->FormatHazard(hazard).c_str());
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
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(Func::vkCmdResolveImage);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<ImageState>(srcImage);
    auto dst_image = Get<ImageState>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &resolve_region = pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_RESOLVE_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                       resolve_region.srcSubresource, resolve_region.srcOffset, resolve_region.extent, tag);
        }
        if (dst_image) {
            context->UpdateAccessState(*dst_image, SYNC_RESOLVE_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       resolve_region.dstSubresource, resolve_region.dstOffset, resolve_region.extent, tag);
        }
    }
}

bool SyncValidator::PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo,
                                                    const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const Location image_info_loc = error_obj.location.dot(Field::pResolveImageInfo);
    auto src_image = Get<ImageState>(pResolveImageInfo->srcImage);
    auto dst_image = Get<ImageState>(pResolveImageInfo->dstImage);

    for (uint32_t region = 0; region < pResolveImageInfo->regionCount; region++) {
        const Location region_loc = image_info_loc.dot(Field::pRegions, region);
        const auto &resolve_region = pResolveImageInfo->pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, SYNC_RESOLVE_TRANSFER_READ, resolve_region.srcSubresource,
                                                resolve_region.srcOffset, resolve_region.extent, false);
            if (hazard.IsHazard()) {
                skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), pResolveImageInfo->srcImage, region_loc,
                                 "Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.Hazard()), FormatHandle(pResolveImageInfo->srcImage).c_str(), region,
                                 cb_access_context->FormatHazard(hazard).c_str());
            }
        }

        if (dst_image) {
            auto hazard = context->DetectHazard(*dst_image, SYNC_RESOLVE_TRANSFER_WRITE, resolve_region.dstSubresource,
                                                resolve_region.dstOffset, resolve_region.extent, false);
            if (hazard.IsHazard()) {
                skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), pResolveImageInfo->dstImage, region_loc,
                                 "Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.Hazard()), FormatHandle(pResolveImageInfo->dstImage).c_str(), region,
                                 cb_access_context->FormatHazard(hazard).c_str());
            }
            if (skip) break;
        }
    }

    return skip;
}

bool SyncValidator::PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                                       const VkResolveImageInfo2KHR *pResolveImageInfo,
                                                       const ErrorObject &error_obj) const {
    return PreCallValidateCmdResolveImage2(commandBuffer, pResolveImageInfo, error_obj);
}

void SyncValidator::RecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo,
                                           Func command) {
    StateTracker::PreCallRecordCmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(command);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<ImageState>(pResolveImageInfo->srcImage);
    auto dst_image = Get<ImageState>(pResolveImageInfo->dstImage);

    for (uint32_t region = 0; region < pResolveImageInfo->regionCount; region++) {
        const auto &resolve_region = pResolveImageInfo->pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_RESOLVE_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                       resolve_region.srcSubresource, resolve_region.srcOffset, resolve_region.extent, tag);
        }
        if (dst_image) {
            context->UpdateAccessState(*dst_image, SYNC_RESOLVE_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       resolve_region.dstSubresource, resolve_region.dstOffset, resolve_region.extent, tag);
        }
    }
}

void SyncValidator::PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                                     const VkResolveImageInfo2KHR *pResolveImageInfo) {
    RecordCmdResolveImage2(commandBuffer, pResolveImageInfo, Func::vkCmdResolveImage2KHR);
}

void SyncValidator::PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo) {
    RecordCmdResolveImage2(commandBuffer, pResolveImageInfo, Func::vkCmdResolveImage2);
}

bool SyncValidator::PreCallValidateCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                   VkDeviceSize dataSize, const void *pData, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        // VK_WHOLE_SIZE not allowed
        const ResourceAccessRange range = MakeRange(dstOffset, dataSize);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, range);
        if (hazard.IsHazard()) {
            skip |= LogError(dstBuffer, string_SyncHazardVUID(hazard.Hazard()),
                             "vkCmdUpdateBuffer: Hazard %s for dstBuffer %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(dstBuffer).c_str(), cb_access_context->FormatHazard(hazard).c_str());
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                 VkDeviceSize dataSize, const void *pData) {
    StateTracker::PreCallRecordCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(Func::vkCmdUpdateBuffer);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        // VK_WHOLE_SIZE not allowed
        const ResourceAccessRange range = MakeRange(dstOffset, dataSize);
        context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
    }
}

bool SyncValidator::PreCallValidateCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                           VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                           const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, 4);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, range);
        if (hazard.IsHazard()) {
            skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), dstBuffer, error_obj.location,
                             "Hazard %s for dstBuffer %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(dstBuffer).c_str(), cb_access_context->FormatHazard(hazard).c_str());
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                         VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) {
    StateTracker::PreCallRecordCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(Func::vkCmdWriteBufferMarkerAMD);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, 4);
        context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
    }
}

bool SyncValidator::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                               const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;
    const auto *access_context = cb_context->GetCurrentAccessContext();
    assert(access_context);
    if (!access_context) return skip;

    SyncOpSetEvent set_event_op(error_obj.location.function, *this, cb_context->GetQueueFlags(), event, stageMask, nullptr);
    return set_event_op.Validate(*cb_context);
}

void SyncValidator::PostCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                              const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdSetEvent(commandBuffer, event, stageMask, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

    cb_context->RecordSyncOp<SyncOpSetEvent>(record_obj.location.function, *this, cb_context->GetQueueFlags(), event, stageMask,
                                             cb_context->GetCurrentAccessContext());
}

bool SyncValidator::PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                   const VkDependencyInfoKHR *pDependencyInfo, const ErrorObject &error_obj) const {
    return PreCallValidateCmdSetEvent2(commandBuffer, event, pDependencyInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                                const VkDependencyInfo *pDependencyInfo, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;
    if (!pDependencyInfo) return skip;

    const auto *access_context = cb_context->GetCurrentAccessContext();
    assert(access_context);
    if (!access_context) return skip;

    SyncOpSetEvent set_event_op(error_obj.location.function, *this, cb_context->GetQueueFlags(), event, *pDependencyInfo, nullptr);
    return set_event_op.Validate(*cb_context);
}

void SyncValidator::PostCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                  const VkDependencyInfoKHR *pDependencyInfo, const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdSetEvent2KHR(commandBuffer, event, pDependencyInfo, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;
    if (!pDependencyInfo) return;

    cb_context->RecordSyncOp<SyncOpSetEvent>(record_obj.location.function, *this, cb_context->GetQueueFlags(), event,
                                             *pDependencyInfo, cb_context->GetCurrentAccessContext());
}

void SyncValidator::PostCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                               const VkDependencyInfo *pDependencyInfo, const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdSetEvent2(commandBuffer, event, pDependencyInfo, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;
    if (!pDependencyInfo) return;

    cb_context->RecordSyncOp<SyncOpSetEvent>(record_obj.location.function, *this, cb_context->GetQueueFlags(), event,
                                             *pDependencyInfo, cb_context->GetCurrentAccessContext());
}

bool SyncValidator::PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;

    SyncOpResetEvent reset_event_op(error_obj.location.function, *this, cb_context->GetQueueFlags(), event, stageMask);
    return reset_event_op.Validate(*cb_context);
}

void SyncValidator::PostCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask,
                                                const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdResetEvent(commandBuffer, event, stageMask, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

    cb_context->RecordSyncOp<SyncOpResetEvent>(record_obj.location.function, *this, cb_context->GetQueueFlags(), event, stageMask);
}

bool SyncValidator::PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;

    SyncOpResetEvent reset_event_op(error_obj.location.function, *this, cb_context->GetQueueFlags(), event, stageMask);
    return reset_event_op.Validate(*cb_context);
    return PreCallValidateCmdResetEvent2(commandBuffer, event, stageMask, error_obj);
}

bool SyncValidator::PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                     VkPipelineStageFlags2KHR stageMask, const ErrorObject &error_obj) const {
    return PreCallValidateCmdResetEvent2(commandBuffer, event, stageMask, error_obj);
}

void SyncValidator::PostCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                    VkPipelineStageFlags2KHR stageMask, const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdResetEvent2KHR(commandBuffer, event, stageMask, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

    cb_context->RecordSyncOp<SyncOpResetEvent>(record_obj.location.function, *this, cb_context->GetQueueFlags(), event, stageMask);
}

void SyncValidator::PostCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask,
                                                 const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdResetEvent2(commandBuffer, event, stageMask, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

    cb_context->RecordSyncOp<SyncOpResetEvent>(record_obj.location.function, *this, cb_context->GetQueueFlags(), event, stageMask);
}

bool SyncValidator::PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                 VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                 uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                 uint32_t bufferMemoryBarrierCount,
                                                 const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                 uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;

    SyncOpWaitEvents wait_events_op(error_obj.location.function, *this, cb_context->GetQueueFlags(), eventCount, pEvents,
                                    srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
                                    pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    return wait_events_op.Validate(*cb_context);
}

void SyncValidator::PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                uint32_t bufferMemoryBarrierCount,
                                                const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                                const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount,
                                              pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                                              imageMemoryBarrierCount, pImageMemoryBarriers, record_obj);

    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

    cb_context->RecordSyncOp<SyncOpWaitEvents>(record_obj.location.function, *this, cb_context->GetQueueFlags(), eventCount,
                                               pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers,
                                               bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
                                               pImageMemoryBarriers);
}

bool SyncValidator::PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                     const VkDependencyInfoKHR *pDependencyInfos,
                                                     const ErrorObject &error_obj) const {
    return PreCallValidateCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, error_obj);
}

void SyncValidator::PostCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                    const VkDependencyInfoKHR *pDependencyInfos, const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos, record_obj);

    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

    cb_context->RecordSyncOp<SyncOpWaitEvents>(record_obj.location.function, *this, cb_context->GetQueueFlags(), eventCount,
                                               pEvents, pDependencyInfos);
}

bool SyncValidator::PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                  const VkDependencyInfo *pDependencyInfos, const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;

    SyncOpWaitEvents wait_events_op(error_obj.location.function, *this, cb_context->GetQueueFlags(), eventCount, pEvents,
                                    pDependencyInfos);
    skip |= wait_events_op.Validate(*cb_context);
    return skip;
}

void SyncValidator::PostCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                 const VkDependencyInfo *pDependencyInfos, const RecordObject &record_obj) {
    StateTracker::PostCallRecordCmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos, record_obj);

    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;

    cb_context->RecordSyncOp<SyncOpWaitEvents>(record_obj.location.function, *this, cb_context->GetQueueFlags(), eventCount,
                                               pEvents, pDependencyInfos);
}

void SyncEventState::ResetFirstScope() {
    first_scope.reset();
    scope = SyncExecScope();
    first_scope_tag = 0;
}

// Keep the "ignore this event" logic in same place for ValidateWait and RecordWait to use
SyncEventState::IgnoreReason SyncEventState::IsIgnoredByWait(vvl::Func command, VkPipelineStageFlags2KHR srcStageMask) const {
    IgnoreReason reason = NotIgnored;

    if ((vvl::Func::vkCmdWaitEvents2KHR == command || vvl::Func::vkCmdWaitEvents2 == command) &&
        (vvl::Func::vkCmdSetEvent == last_command)) {
        reason = SetVsWait2;
    } else if ((last_command == vvl::Func::vkCmdResetEvent || last_command == vvl::Func::vkCmdResetEvent2KHR) &&
               !HasBarrier(0U, 0U)) {
        reason = (last_command == vvl::Func::vkCmdResetEvent) ? ResetWaitRace : Reset2WaitRace;
    } else if (unsynchronized_set != vvl::Func::Empty) {
        reason = SetRace;
    } else if (first_scope) {
        const VkPipelineStageFlags2KHR missing_bits = scope.mask_param & ~srcStageMask;
        // Note it is the "not missing bits" path that is the only "NotIgnored" path
        if (missing_bits) reason = MissingStageBits;
    } else {
        reason = MissingSetEvent;
    }

    return reason;
}

bool SyncEventState::HasBarrier(VkPipelineStageFlags2KHR stageMask, VkPipelineStageFlags2KHR exec_scope_arg) const {
    return (last_command == vvl::Func::Empty) || (stageMask & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) || (barriers & exec_scope_arg) ||
           (barriers & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
}

void SyncEventState::AddReferencedTags(ResourceUsageTagSet &referenced) const {
    if (first_scope) {
        first_scope->AddReferencedTags(referenced);
    }
}

SyncOpBarriers::SyncOpBarriers(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags,
                               VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                               VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount,
                               const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                               const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                               const VkImageMemoryBarrier *pImageMemoryBarriers)
    : SyncOpBase(command), barriers_(1) {
    auto &barrier_set = barriers_[0];
    barrier_set.dependency_flags = dependencyFlags;
    barrier_set.src_exec_scope = SyncExecScope::MakeSrc(queue_flags, srcStageMask);
    barrier_set.dst_exec_scope = SyncExecScope::MakeDst(queue_flags, dstStageMask);
    // Translate the API parameters into structures SyncVal understands directly, and dehandle for safer/faster replay.
    barrier_set.MakeMemoryBarriers(barrier_set.src_exec_scope, barrier_set.dst_exec_scope, dependencyFlags, memoryBarrierCount,
                                   pMemoryBarriers);
    barrier_set.MakeBufferMemoryBarriers(sync_state, barrier_set.src_exec_scope, barrier_set.dst_exec_scope, dependencyFlags,
                                         bufferMemoryBarrierCount, pBufferMemoryBarriers);
    barrier_set.MakeImageMemoryBarriers(sync_state, barrier_set.src_exec_scope, barrier_set.dst_exec_scope, dependencyFlags,
                                        imageMemoryBarrierCount, pImageMemoryBarriers);
}

SyncOpBarriers::SyncOpBarriers(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags, uint32_t event_count,
                               const VkDependencyInfoKHR *dep_infos)
    : SyncOpBase(command), barriers_(event_count) {
    for (uint32_t i = 0; i < event_count; i++) {
        const auto &dep_info = dep_infos[i];
        auto &barrier_set = barriers_[i];
        barrier_set.dependency_flags = dep_info.dependencyFlags;
        auto stage_masks = sync_utils::GetGlobalStageMasks(dep_info);
        barrier_set.src_exec_scope = SyncExecScope::MakeSrc(queue_flags, stage_masks.src);
        barrier_set.dst_exec_scope = SyncExecScope::MakeDst(queue_flags, stage_masks.dst);
        // Translate the API parameters into structures SyncVal understands directly, and dehandle for safer/faster replay.
        barrier_set.MakeMemoryBarriers(queue_flags, dep_info.dependencyFlags, dep_info.memoryBarrierCount,
                                       dep_info.pMemoryBarriers);
        barrier_set.MakeBufferMemoryBarriers(sync_state, queue_flags, dep_info.dependencyFlags, dep_info.bufferMemoryBarrierCount,
                                             dep_info.pBufferMemoryBarriers);
        barrier_set.MakeImageMemoryBarriers(sync_state, queue_flags, dep_info.dependencyFlags, dep_info.imageMemoryBarrierCount,
                                            dep_info.pImageMemoryBarriers);
    }
}

SyncOpPipelineBarrier::SyncOpPipelineBarrier(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags,
                                             VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                             VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount,
                                             const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                                             const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                                             const VkImageMemoryBarrier *pImageMemoryBarriers)
    : SyncOpBarriers(command, sync_state, queue_flags, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount,
                     pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
                     pImageMemoryBarriers) {}

SyncOpPipelineBarrier::SyncOpPipelineBarrier(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags,
                                             const VkDependencyInfoKHR &dep_info)
    : SyncOpBarriers(command, sync_state, queue_flags, 1, &dep_info) {}

bool SyncOpPipelineBarrier::Validate(const CommandBufferAccessContext &cb_context) const {
    bool skip = false;
    const auto *context = cb_context.GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;
    assert(barriers_.size() == 1);  // PipelineBarriers only support a single barrier set.

    // Validate Image Layout transitions
    const auto &barrier_set = barriers_[0];
    for (const auto &image_barrier : barrier_set.image_memory_barriers) {
        if (image_barrier.new_layout == image_barrier.old_layout) continue;  // Only interested in layout transitions at this point.
        const auto *image_state = image_barrier.image.get();
        if (!image_state) continue;
        const auto hazard = context->DetectImageBarrierHazard(image_barrier);
        if (hazard.IsHazard()) {
            // PHASE1 TODO -- add tag information to log msg when useful.
            const auto &sync_state = cb_context.GetSyncState();
            const auto image_handle = image_state->image();
            skip |= sync_state.LogError(image_handle, string_SyncHazardVUID(hazard.Hazard()),
                                        "%s: Hazard %s for image barrier %" PRIu32 " %s. Access info %s.", CmdName(),
                                        string_SyncHazard(hazard.Hazard()), image_barrier.index,
                                        sync_state.FormatHandle(image_handle).c_str(), cb_context.FormatHazard(hazard).c_str());
        }
    }
    return skip;
}

struct SyncOpPipelineBarrierFunctorFactory {
    using BarrierOpFunctor = PipelineBarrierOp;
    using ApplyFunctor = ApplyBarrierFunctor<BarrierOpFunctor>;
    using GlobalBarrierOpFunctor = PipelineBarrierOp;
    using GlobalApplyFunctor = ApplyBarrierOpsFunctor<GlobalBarrierOpFunctor>;
    using BufferRange = SingleRangeGenerator<ResourceAccessRange>;
    using ImageRange = subresource_adapter::ImageRangeGenerator;
    using GlobalRange = SingleRangeGenerator<ResourceAccessRange>;
    using ImageState = syncval_state::ImageState;

    ApplyFunctor MakeApplyFunctor(QueueId queue_id, const SyncBarrier &barrier, bool layout_transition) const {
        return ApplyFunctor(BarrierOpFunctor(queue_id, barrier, layout_transition));
    }
    GlobalApplyFunctor MakeGlobalApplyFunctor(size_t size_hint, ResourceUsageTag tag) const {
        return GlobalApplyFunctor(true /* resolve */, size_hint, tag);
    }
    GlobalBarrierOpFunctor MakeGlobalBarrierOpFunctor(QueueId queue_id, const SyncBarrier &barrier) const {
        return GlobalBarrierOpFunctor(queue_id, barrier, false);
    }

    BufferRange MakeRangeGen(const BUFFER_STATE &buffer, const ResourceAccessRange &range) const {
        if (!SimpleBinding(buffer)) return ResourceAccessRange();
        const auto base_address = ResourceBaseAddress(buffer);
        return (range + base_address);
    }
    ImageRange MakeRangeGen(const ImageState &image, const VkImageSubresourceRange &subresource_range) const {
        return image.MakeImageRangeGen(subresource_range, false);
    }
    GlobalRange MakeGlobalRangeGen() const { return kFullRange; }
};

template <typename Barriers, typename FunctorFactory>
void SyncOpBarriers::ApplyBarriers(const Barriers &barriers, const FunctorFactory &factory, const QueueId queue_id,
                                   const ResourceUsageTag tag, AccessContext *context) {
    for (const auto &barrier : barriers) {
        const auto *state = barrier.GetState();
        if (state) {
            auto update_action = factory.MakeApplyFunctor(queue_id, barrier.barrier, barrier.IsLayoutTransition());
            auto range_gen = factory.MakeRangeGen(*state, barrier.Range());
            UpdateMemoryAccessState(context->GetAccessStateMap(), update_action, range_gen);
        }
    }
}

template <typename Barriers, typename FunctorFactory>
void SyncOpBarriers::ApplyGlobalBarriers(const Barriers &barriers, const FunctorFactory &factory, const QueueId queue_id,
                                         const ResourceUsageTag tag, AccessContext *access_context) {
    auto barriers_functor = factory.MakeGlobalApplyFunctor(barriers.size(), tag);
    for (const auto &barrier : barriers) {
        barriers_functor.EmplaceBack(factory.MakeGlobalBarrierOpFunctor(queue_id, barrier));
    }
    auto range_gen = factory.MakeGlobalRangeGen();
    UpdateMemoryAccessState(access_context->GetAccessStateMap(), barriers_functor, range_gen);
}

ResourceUsageTag SyncOpPipelineBarrier::Record(CommandBufferAccessContext *cb_context) {
    const auto tag = cb_context->NextCommandTag(command_);
    ReplayRecord(*cb_context, tag);
    return tag;
}

void SyncOpPipelineBarrier::ReplayRecord(CommandExecutionContext &exec_context, const ResourceUsageTag exec_tag) const {
    SyncOpPipelineBarrierFunctorFactory factory;
    // Pipeline barriers only have a single barrier set, unlike WaitEvents2
    assert(barriers_.size() == 1);
    const auto &barrier_set = barriers_[0];
    if (!exec_context.ValidForSyncOps()) return;

    SyncEventsContext *events_context = exec_context.GetCurrentEventsContext();
    AccessContext *access_context = exec_context.GetCurrentAccessContext();
    const auto queue_id = exec_context.GetQueueId();
    ApplyBarriers(barrier_set.buffer_memory_barriers, factory, queue_id, exec_tag, access_context);
    ApplyBarriers(barrier_set.image_memory_barriers, factory, queue_id, exec_tag, access_context);
    ApplyGlobalBarriers(barrier_set.memory_barriers, factory, queue_id, exec_tag, access_context);
    if (barrier_set.single_exec_scope) {
        events_context->ApplyBarrier(barrier_set.src_exec_scope, barrier_set.dst_exec_scope, exec_tag);
    } else {
        for (const auto &barrier : barrier_set.memory_barriers) {
            events_context->ApplyBarrier(barrier.src_exec_scope, barrier.dst_exec_scope, exec_tag);
        }
    }
}

bool SyncOpPipelineBarrier::ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const {
    // The layout transitions happen at the replay tag
    ResourceUsageRange first_use_range = {recorded_tag, recorded_tag + 1};
    return replay.DetectFirstUseHazard(first_use_range);
}

void SyncOpBarriers::BarrierSet::MakeMemoryBarriers(const SyncExecScope &src, const SyncExecScope &dst,
                                                    VkDependencyFlags dependency_flags, uint32_t memory_barrier_count,
                                                    const VkMemoryBarrier *barriers) {
    memory_barriers.reserve(std::max<uint32_t>(1, memory_barrier_count));
    for (uint32_t barrier_index = 0; barrier_index < memory_barrier_count; barrier_index++) {
        const auto &barrier = barriers[barrier_index];
        SyncBarrier sync_barrier(barrier, src, dst);
        memory_barriers.emplace_back(sync_barrier);
    }
    if (0 == memory_barrier_count) {
        // If there are no global memory barriers, force an exec barrier
        memory_barriers.emplace_back(SyncBarrier(src, dst));
    }
    single_exec_scope = true;
}

void SyncOpBarriers::BarrierSet::MakeBufferMemoryBarriers(const SyncValidator &sync_state, const SyncExecScope &src,
                                                          const SyncExecScope &dst, VkDependencyFlags dependencyFlags,
                                                          uint32_t barrier_count, const VkBufferMemoryBarrier *barriers) {
    buffer_memory_barriers.reserve(barrier_count);
    for (uint32_t index = 0; index < barrier_count; index++) {
        const auto &barrier = barriers[index];
        auto buffer = sync_state.Get<BUFFER_STATE>(barrier.buffer);
        if (buffer) {
            const auto range = MakeRange(*buffer, barrier.offset, barrier.size);
            const SyncBarrier sync_barrier(barrier, src, dst);
            buffer_memory_barriers.emplace_back(buffer, sync_barrier, range);
        } else {
            buffer_memory_barriers.emplace_back();
        }
    }
}

void SyncOpBarriers::BarrierSet::MakeMemoryBarriers(VkQueueFlags queue_flags, VkDependencyFlags dependency_flags,
                                                    uint32_t memory_barrier_count, const VkMemoryBarrier2 *barriers) {
    memory_barriers.reserve(memory_barrier_count);
    for (uint32_t barrier_index = 0; barrier_index < memory_barrier_count; barrier_index++) {
        const auto &barrier = barriers[barrier_index];
        auto src = SyncExecScope::MakeSrc(queue_flags, barrier.srcStageMask);
        auto dst = SyncExecScope::MakeDst(queue_flags, barrier.dstStageMask);
        SyncBarrier sync_barrier(barrier, src, dst);
        memory_barriers.emplace_back(sync_barrier);
    }
    single_exec_scope = false;
}

void SyncOpBarriers::BarrierSet::MakeBufferMemoryBarriers(const SyncValidator &sync_state, VkQueueFlags queue_flags,
                                                          VkDependencyFlags dependencyFlags, uint32_t barrier_count,
                                                          const VkBufferMemoryBarrier2 *barriers) {
    buffer_memory_barriers.reserve(barrier_count);
    for (uint32_t index = 0; index < barrier_count; index++) {
        const auto &barrier = barriers[index];
        auto src = SyncExecScope::MakeSrc(queue_flags, barrier.srcStageMask);
        auto dst = SyncExecScope::MakeDst(queue_flags, barrier.dstStageMask);
        auto buffer = sync_state.Get<BUFFER_STATE>(barrier.buffer);
        if (buffer) {
            const auto range = MakeRange(*buffer, barrier.offset, barrier.size);
            const SyncBarrier sync_barrier(barrier, src, dst);
            buffer_memory_barriers.emplace_back(buffer, sync_barrier, range);
        } else {
            buffer_memory_barriers.emplace_back();
        }
    }
}

void SyncOpBarriers::BarrierSet::MakeImageMemoryBarriers(const SyncValidator &sync_state, const SyncExecScope &src,
                                                         const SyncExecScope &dst, VkDependencyFlags dependencyFlags,
                                                         uint32_t barrier_count, const VkImageMemoryBarrier *barriers) {
    image_memory_barriers.reserve(barrier_count);
    for (uint32_t index = 0; index < barrier_count; index++) {
        const auto &barrier = barriers[index];
        auto image = sync_state.Get<ImageState>(barrier.image);
        if (image) {
            auto subresource_range = NormalizeSubresourceRange(image->createInfo, barrier.subresourceRange);
            const SyncBarrier sync_barrier(barrier, src, dst);
            image_memory_barriers.emplace_back(image, index, sync_barrier, barrier.oldLayout, barrier.newLayout, subresource_range);
        } else {
            image_memory_barriers.emplace_back();
            image_memory_barriers.back().index = index;  // Just in case we're interested in the ones we skipped.
        }
    }
}

void SyncOpBarriers::BarrierSet::MakeImageMemoryBarriers(const SyncValidator &sync_state, VkQueueFlags queue_flags,
                                                         VkDependencyFlags dependencyFlags, uint32_t barrier_count,
                                                         const VkImageMemoryBarrier2 *barriers) {
    image_memory_barriers.reserve(barrier_count);
    for (uint32_t index = 0; index < barrier_count; index++) {
        const auto &barrier = barriers[index];
        auto src = SyncExecScope::MakeSrc(queue_flags, barrier.srcStageMask);
        auto dst = SyncExecScope::MakeDst(queue_flags, barrier.dstStageMask);
        auto image = sync_state.Get<ImageState>(barrier.image);
        if (image) {
            auto subresource_range = NormalizeSubresourceRange(image->createInfo, barrier.subresourceRange);
            const SyncBarrier sync_barrier(barrier, src, dst);
            image_memory_barriers.emplace_back(image, index, sync_barrier, barrier.oldLayout, barrier.newLayout, subresource_range);
        } else {
            image_memory_barriers.emplace_back();
            image_memory_barriers.back().index = index;  // Just in case we're interested in the ones we skipped.
        }
    }
}

SyncOpWaitEvents::SyncOpWaitEvents(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags,
                                   uint32_t eventCount, const VkEvent *pEvents, VkPipelineStageFlags srcStageMask,
                                   VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount,
                                   const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                                   const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                                   const VkImageMemoryBarrier *pImageMemoryBarriers)
    : SyncOpBarriers(command, sync_state, queue_flags, srcStageMask, dstStageMask, VkDependencyFlags(0U), memoryBarrierCount,
                     pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
                     pImageMemoryBarriers) {
    MakeEventsList(sync_state, eventCount, pEvents);
}

SyncOpWaitEvents::SyncOpWaitEvents(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags,
                                   uint32_t eventCount, const VkEvent *pEvents, const VkDependencyInfoKHR *pDependencyInfo)
    : SyncOpBarriers(command, sync_state, queue_flags, eventCount, pDependencyInfo) {
    MakeEventsList(sync_state, eventCount, pEvents);
    assert(events_.size() == barriers_.size());  // Just so nobody gets clever and decides to cull the event or barrier arrays
}

const char *const SyncOpWaitEvents::kIgnored = "Wait operation is ignored for this event.";

bool SyncOpWaitEvents::Validate(const CommandBufferAccessContext &cb_context) const {
    bool skip = false;
    const auto &sync_state = cb_context.GetSyncState();
    const auto command_buffer_handle = cb_context.GetCBState().commandBuffer();

    // This is only interesting at record and not replay (Execute/Submit) time.
    for (size_t barrier_set_index = 0; barrier_set_index < barriers_.size(); barrier_set_index++) {
        const auto &barrier_set = barriers_[barrier_set_index];
        if (barrier_set.single_exec_scope) {
            const Location loc(Cmd());
            if (barrier_set.src_exec_scope.mask_param & VK_PIPELINE_STAGE_HOST_BIT) {
                const std::string vuid = std::string("SYNC-") + std::string(CmdName()) + std::string("-hostevent-unsupported");
                sync_state.LogInfo(vuid, command_buffer_handle, loc,
                                   "srcStageMask includes %s, unsupported by synchronization validation.",
                                   string_VkPipelineStageFlagBits(VK_PIPELINE_STAGE_HOST_BIT));
            } else {
                const auto &barriers = barrier_set.memory_barriers;
                for (size_t barrier_index = 0; barrier_index < barriers.size(); barrier_index++) {
                    const auto &barrier = barriers[barrier_index];
                    if (barrier.src_exec_scope.mask_param & VK_PIPELINE_STAGE_HOST_BIT) {
                        const std::string vuid =
                            std::string("SYNC-") + std::string(CmdName()) + std::string("-hostevent-unsupported");

                        sync_state.LogInfo(vuid, command_buffer_handle, loc,
                                           "srcStageMask %s of %s %zu, %s %zu, unsupported by synchronization validation.",
                                           string_VkPipelineStageFlagBits(VK_PIPELINE_STAGE_HOST_BIT), "pDependencyInfo",
                                           barrier_set_index, "pMemoryBarriers", barrier_index);
                    }
                }
            }
        }
    }

    // The rest is common to record time and replay time.
    skip |= DoValidate(cb_context, ResourceUsageRecord::kMaxIndex);
    return skip;
}

bool SyncOpWaitEvents::DoValidate(const CommandExecutionContext &exec_context, const ResourceUsageTag base_tag) const {
    bool skip = false;
    const auto &sync_state = exec_context.GetSyncState();
    const QueueId queue_id = exec_context.GetQueueId();

    VkPipelineStageFlags2KHR event_stage_masks = 0U;
    VkPipelineStageFlags2KHR barrier_mask_params = 0U;
    bool events_not_found = false;
    const auto *events_context = exec_context.GetCurrentEventsContext();
    assert(events_context);
    size_t barrier_set_index = 0;
    size_t barrier_set_incr = (barriers_.size() == 1) ? 0 : 1;
    for (const auto &event : events_) {
        const auto *sync_event = events_context->Get(event.get());
        const auto &barrier_set = barriers_[barrier_set_index];
        if (!sync_event) {
            // NOTE PHASE2: This is where we'll need queue submit time validation to come back and check the srcStageMask bits
            //              or solve this with replay creating the SyncEventState in the queue context... also this will be a
            //              new validation error... wait without previously submitted set event...
            events_not_found = true;  // Demote "extra_stage_bits" error to warning, to avoid false positives at *record time*
            barrier_set_index += barrier_set_incr;
            continue;  // Core, Lifetimes, or Param check needs to catch invalid events.
        }

        // For replay calls, don't revalidate "same command buffer" events
        if (sync_event->last_command_tag >= base_tag) continue;

        const auto event_handle = sync_event->event->event();
        // TODO add "destroyed" checks

        if (sync_event->first_scope) {
            // Only accumulate barrier and event stages if there is a pending set in the current context
            barrier_mask_params |= barrier_set.src_exec_scope.mask_param;
            event_stage_masks |= sync_event->scope.mask_param;
        }

        const auto &src_exec_scope = barrier_set.src_exec_scope;

        const auto ignore_reason = sync_event->IsIgnoredByWait(command_, src_exec_scope.mask_param);
        if (ignore_reason) {
            switch (ignore_reason) {
                case SyncEventState::ResetWaitRace:
                case SyncEventState::Reset2WaitRace: {
                    // Four permuations of Reset and Wait calls...
                    const char *vuid = (command_ == vvl::Func::vkCmdWaitEvents) ? "VUID-vkCmdResetEvent-event-03834"
                                                                                : "VUID-vkCmdResetEvent-event-03835";
                    if (ignore_reason == SyncEventState::Reset2WaitRace) {
                        vuid = (command_ == vvl::Func::vkCmdWaitEvents) ? "VUID-vkCmdResetEvent2-event-03831"
                                                                        : "VUID-vkCmdResetEvent2-event-03832";
                    }
                    const char *const message =
                        "%s: %s %s operation following %s without intervening execution barrier, may cause race condition. %s";
                    skip |=
                        sync_state.LogError(event_handle, vuid, message, CmdName(), sync_state.FormatHandle(event_handle).c_str(),
                                            CmdName(), vvl::String(sync_event->last_command), kIgnored);
                    break;
                }
                case SyncEventState::SetRace: {
                    // Issue error message that Wait is waiting on an signal subject to race condition, and is thus ignored for
                    // this event
                    const char *const vuid = "SYNC-vkCmdWaitEvents-unsynchronized-setops";
                    const char *const message =
                        "%s: %s Unsychronized %s calls result in race conditions w.r.t. event signalling, %s %s";
                    const char *const reason = "First synchronization scope is undefined.";
                    skip |=
                        sync_state.LogError(event_handle, vuid, message, CmdName(), sync_state.FormatHandle(event_handle).c_str(),
                                            vvl::String(sync_event->last_command), reason, kIgnored);
                    break;
                }
                case SyncEventState::MissingStageBits: {
                    const auto missing_bits = sync_event->scope.mask_param & ~src_exec_scope.mask_param;
                    // Issue error message that event waited for is not in wait events scope
                    const char *const vuid = "VUID-vkCmdWaitEvents-srcStageMask-01158";
                    const char *const message = "%s: %s stageMask %" PRIx64 " includes bits not present in srcStageMask 0x%" PRIx64
                                                ". Bits missing from srcStageMask %s. %s";
                    skip |=
                        sync_state.LogError(event_handle, vuid, message, CmdName(), sync_state.FormatHandle(event_handle).c_str(),
                                            sync_event->scope.mask_param, src_exec_scope.mask_param,
                                            sync_utils::StringPipelineStageFlags(missing_bits).c_str(), kIgnored);
                    break;
                }
                case SyncEventState::SetVsWait2: {
                    skip |= sync_state.LogError(
                        event_handle, "VUID-vkCmdWaitEvents2-pEvents-03837", "%s: Follows set of %s by %s. Disallowed.", CmdName(),
                        sync_state.FormatHandle(event_handle).c_str(), vvl::String(sync_event->last_command));
                    break;
                }
                case SyncEventState::MissingSetEvent: {
                    // TODO: There are conditions at queue submit time where we can definitively say that
                    // a missing set event is an error.  Add those if not captured in CoreChecks
                    break;
                }
                default:
                    assert(ignore_reason == SyncEventState::NotIgnored);
            }
        } else if (barrier_set.image_memory_barriers.size()) {
            const auto &image_memory_barriers = barrier_set.image_memory_barriers;
            const auto *context = exec_context.GetCurrentAccessContext();
            assert(context);
            for (const auto &image_memory_barrier : image_memory_barriers) {
                if (image_memory_barrier.old_layout == image_memory_barrier.new_layout) continue;
                const auto *image_state = image_memory_barrier.image.get();
                if (!image_state) continue;
                const auto &subresource_range = image_memory_barrier.range;
                const auto &src_access_scope = image_memory_barrier.barrier.src_access_scope;
                const auto hazard = context->DetectImageBarrierHazard(*image_state, subresource_range, sync_event->scope.exec_scope,
                                                                      src_access_scope, queue_id, *sync_event,
                                                                      AccessContext::DetectOptions::kDetectAll);
                if (hazard.IsHazard()) {
                    skip |= sync_state.LogError(image_state->image(), string_SyncHazardVUID(hazard.Hazard()),
                                                "%s: Hazard %s for image barrier %" PRIu32 " %s. Access info %s.", CmdName(),
                                                string_SyncHazard(hazard.Hazard()), image_memory_barrier.index,
                                                sync_state.FormatHandle(image_state->image()).c_str(),
                                                exec_context.FormatHazard(hazard).c_str());
                    break;
                }
            }
        }
        // TODO:  Add infrastructure for checking pDependencyInfo's vs. CmdSetEvent2 VUID - vkCmdWaitEvents2KHR - pEvents -
        // 03839
        barrier_set_index += barrier_set_incr;
    }

    // Note that we can't check for HOST in pEvents as we don't track that set event type
    const auto extra_stage_bits = (barrier_mask_params & ~VK_PIPELINE_STAGE_2_HOST_BIT_KHR) & ~event_stage_masks;
    if (extra_stage_bits) {
        // Issue error message that event waited for is not in wait events scope
        // NOTE: This isn't exactly the right VUID for WaitEvents2, but it's as close as we currently have support for
        const char *const vuid = (vvl::Func::vkCmdWaitEvents == command_) ? "VUID-vkCmdWaitEvents-srcStageMask-01158"
                                                                          : "VUID-vkCmdWaitEvents2-pEvents-03838";
        const char *const message =
            "srcStageMask 0x%" PRIx64 " contains stages not present in pEvents stageMask. Extra stages are %s.%s";
        const auto handle = exec_context.Handle();
        const Location loc(Cmd());
        if (events_not_found) {
            sync_state.LogInfo(vuid, handle, loc, message, barrier_mask_params,
                               sync_utils::StringPipelineStageFlags(extra_stage_bits).c_str(),
                               " vkCmdSetEvent may be in previously submitted command buffer.");
        } else {
            skip |= sync_state.LogError(vuid, handle, loc, message, barrier_mask_params,
                                        sync_utils::StringPipelineStageFlags(extra_stage_bits).c_str(), "");
        }
    }
    return skip;
}

struct SyncOpWaitEventsFunctorFactory {
    using BarrierOpFunctor = WaitEventBarrierOp;
    using ApplyFunctor = ApplyBarrierFunctor<BarrierOpFunctor>;
    using GlobalBarrierOpFunctor = WaitEventBarrierOp;
    using GlobalApplyFunctor = ApplyBarrierOpsFunctor<GlobalBarrierOpFunctor>;
    using BufferRange = EventSimpleRangeGenerator;
    using ImageRange = EventImageRangeGenerator;
    using GlobalRange = EventSimpleRangeGenerator;
    using ImageState = syncval_state::ImageState;

    // Need to restrict to only valid exec and access scope for this event
    // Pass by value is intentional to get a copy we can change without modifying the passed barrier
    SyncBarrier RestrictToEvent(SyncBarrier barrier) const {
        barrier.src_exec_scope.exec_scope = sync_event->scope.exec_scope & barrier.src_exec_scope.exec_scope;
        barrier.src_access_scope = sync_event->scope.valid_accesses & barrier.src_access_scope;
        return barrier;
    }
    ApplyFunctor MakeApplyFunctor(QueueId queue_id, const SyncBarrier &barrier_arg, bool layout_transition) const {
        auto barrier = RestrictToEvent(barrier_arg);
        return ApplyFunctor(BarrierOpFunctor(queue_id, sync_event->first_scope_tag, barrier, layout_transition));
    }
    GlobalApplyFunctor MakeGlobalApplyFunctor(size_t size_hint, ResourceUsageTag tag) const {
        return GlobalApplyFunctor(false /* don't resolve */, size_hint, tag);
    }
    GlobalBarrierOpFunctor MakeGlobalBarrierOpFunctor(const QueueId queue_id, const SyncBarrier &barrier_arg) const {
        auto barrier = RestrictToEvent(barrier_arg);
        return GlobalBarrierOpFunctor(queue_id, sync_event->first_scope_tag, barrier, false);
    }

    BufferRange MakeRangeGen(const BUFFER_STATE &buffer, const ResourceAccessRange &range_arg) const {
        const auto base_address = ResourceBaseAddress(buffer);
        ResourceAccessRange range = SimpleBinding(buffer) ? (range_arg + base_address) : ResourceAccessRange();
        EventSimpleRangeGenerator filtered_range_gen(sync_event->FirstScope(), range);
        return filtered_range_gen;
    }
    ImageRange MakeRangeGen(const ImageState &image, const VkImageSubresourceRange &subresource_range) const {
        ImageRangeGen image_range_gen = image.MakeImageRangeGen(subresource_range, false);
        EventImageRangeGenerator filtered_range_gen(sync_event->FirstScope(), image_range_gen);

        return filtered_range_gen;
    }
    GlobalRange MakeGlobalRangeGen() const { return EventSimpleRangeGenerator(sync_event->FirstScope(), kFullRange); }
    SyncOpWaitEventsFunctorFactory(SyncEventState *sync_event_) : sync_event(sync_event_) { assert(sync_event); }
    SyncEventState *sync_event;
};

ResourceUsageTag SyncOpWaitEvents::Record(CommandBufferAccessContext *cb_context) {
    const auto tag = cb_context->NextCommandTag(command_);

    ReplayRecord(*cb_context, tag);
    return tag;
}

void SyncOpWaitEvents::ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const {
    // Unlike PipelineBarrier, WaitEvent is *not* limited to accesses within the current subpass (if any) and thus needs to import
    // all accesses. Can instead import for all first_scopes, or a union of them, if this becomes a performance/memory issue,
    // but with no idea of the performance of the union, nor of whether it even matters... take the simplest approach here,
    if (!exec_context.ValidForSyncOps()) return;
    AccessContext *access_context = exec_context.GetCurrentAccessContext();
    SyncEventsContext *events_context = exec_context.GetCurrentEventsContext();
    const QueueId queue_id = exec_context.GetQueueId();

    access_context->ResolvePreviousAccesses();

    size_t barrier_set_index = 0;
    size_t barrier_set_incr = (barriers_.size() == 1) ? 0 : 1;
    assert(barriers_.size() == 1 || (barriers_.size() == events_.size()));
    for (auto &event_shared : events_) {
        if (!event_shared.get()) continue;
        auto *sync_event = events_context->GetFromShared(event_shared);

        sync_event->last_command = command_;
        sync_event->last_command_tag = exec_tag;

        const auto &barrier_set = barriers_[barrier_set_index];
        const auto &dst = barrier_set.dst_exec_scope;
        if (!sync_event->IsIgnoredByWait(command_, barrier_set.src_exec_scope.mask_param)) {
            // These apply barriers one at a time as the are restricted to the resource ranges specified per each barrier,
            // but do not update the dependency chain information (but set the "pending" state) // s.t. the order independence
            // of the barriers is maintained.
            SyncOpWaitEventsFunctorFactory factory(sync_event);
            ApplyBarriers(barrier_set.buffer_memory_barriers, factory, queue_id, exec_tag, access_context);
            ApplyBarriers(barrier_set.image_memory_barriers, factory, queue_id, exec_tag, access_context);
            ApplyGlobalBarriers(barrier_set.memory_barriers, factory, queue_id, exec_tag, access_context);

            // Apply the global barrier to the event itself (for race condition tracking)
            // Events don't happen at a stage, so we need to store the unexpanded ALL_COMMANDS if set for inter-event-calls
            sync_event->barriers = dst.mask_param & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            sync_event->barriers |= dst.exec_scope;
        } else {
            // We ignored this wait, so we don't have any effective synchronization barriers for it.
            sync_event->barriers = 0U;
        }
        barrier_set_index += barrier_set_incr;
    }

    // Apply the pending barriers
    ResolvePendingBarrierFunctor apply_pending_action(exec_tag);
    access_context->ApplyToContext(apply_pending_action);
}

bool SyncOpWaitEvents::ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const {
    return DoValidate(replay.GetExecutionContext(), replay.GetBaseTag() + recorded_tag);
}

bool SyncValidator::PreCallValidateCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR pipelineStage,
                                                            VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                            const ErrorObject &error_obj) const {
    bool skip = false;
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_access_context = &cb_state->access_context;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, 4);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, range);
        if (hazard.IsHazard()) {
            skip |= LogError(dstBuffer, string_SyncHazardVUID(hazard.Hazard()),
                             "vkCmdWriteBufferMarkerAMD2: Hazard %s for dstBuffer %s. Access info %s.",
                             string_SyncHazard(hazard.Hazard()), FormatHandle(dstBuffer).c_str(),
                             cb_access_context->FormatHazard(hazard).c_str());
        }
    }
    return skip;
}

void SyncOpWaitEvents::MakeEventsList(const SyncValidator &sync_state, uint32_t event_count, const VkEvent *events) {
    events_.reserve(event_count);
    for (uint32_t event_index = 0; event_index < event_count; event_index++) {
        events_.emplace_back(sync_state.Get<EVENT_STATE>(events[event_index]));
    }
}

SyncOpResetEvent::SyncOpResetEvent(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkEvent event,
                                   VkPipelineStageFlags2KHR stageMask)
    : SyncOpBase(command),
      event_(sync_state.Get<EVENT_STATE>(event)),
      exec_scope_(SyncExecScope::MakeSrc(queue_flags, stageMask)) {}

bool SyncOpResetEvent::Validate(const CommandBufferAccessContext &cb_context) const {
    return DoValidate(cb_context, ResourceUsageRecord::kMaxIndex);
}

bool SyncOpResetEvent::DoValidate(const CommandExecutionContext &exec_context, const ResourceUsageTag base_tag) const {
    auto *events_context = exec_context.GetCurrentEventsContext();
    assert(events_context);
    bool skip = false;
    if (!events_context) return skip;

    const auto &sync_state = exec_context.GetSyncState();
    const auto *sync_event = events_context->Get(event_);
    if (!sync_event) return skip;  // Core, Lifetimes, or Param check needs to catch invalid events.

    if (sync_event->last_command_tag > base_tag) return skip;  // if we validated this in recording of the secondary, don't repeat

    const char *const set_wait =
        "%s: %s %s operation following %s without intervening execution barrier, is a race condition and may result in data "
        "hazards.";
    const char *message = set_wait;  // Only one message this call.
    if (!sync_event->HasBarrier(exec_scope_.mask_param, exec_scope_.exec_scope)) {
        const char *vuid = nullptr;
        switch (sync_event->last_command) {
            case vvl::Func::vkCmdSetEvent:
            case vvl::Func::vkCmdSetEvent2KHR:
            case vvl::Func::vkCmdSetEvent2:
                // Needs a barrier between set and reset
                vuid = "SYNC-vkCmdResetEvent-missingbarrier-set";
                break;
            case vvl::Func::vkCmdWaitEvents:
            case vvl::Func::vkCmdWaitEvents2KHR:
            case vvl::Func::vkCmdWaitEvents2: {
                // Needs to be in the barriers chain (either because of a barrier, or because of dstStageMask
                vuid = "SYNC-vkCmdResetEvent-missingbarrier-wait";
                break;
            }
            default:
                // The only other valid last command that wasn't one.
                assert((sync_event->last_command == vvl::Func::Empty) || (sync_event->last_command == vvl::Func::vkCmdResetEvent) ||
                       (sync_event->last_command == vvl::Func::vkCmdResetEvent2KHR));
                break;
        }
        if (vuid) {
            skip |= sync_state.LogError(event_->event(), vuid, message, CmdName(), sync_state.FormatHandle(event_->event()).c_str(),
                                        CmdName(), vvl::String(sync_event->last_command));
        }
    }
    return skip;
}

ResourceUsageTag SyncOpResetEvent::Record(CommandBufferAccessContext *cb_context) {
    const auto tag = cb_context->NextCommandTag(command_);
    ReplayRecord(*cb_context, tag);
    return tag;
}

bool SyncOpResetEvent::ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const {
    return DoValidate(replay.GetExecutionContext(), replay.GetBaseTag() + recorded_tag);
}

void SyncOpResetEvent::ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const {
    if (!exec_context.ValidForSyncOps()) return;
    SyncEventsContext *events_context = exec_context.GetCurrentEventsContext();

    auto *sync_event = events_context->GetFromShared(event_);
    if (!sync_event) return;  // Core, Lifetimes, or Param check needs to catch invalid events.

    // Update the event state
    sync_event->last_command = command_;
    sync_event->last_command_tag = exec_tag;
    sync_event->unsynchronized_set = vvl::Func::Empty;
    sync_event->ResetFirstScope();
    sync_event->barriers = 0U;
}

SyncOpSetEvent::SyncOpSetEvent(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkEvent event,
                               VkPipelineStageFlags2KHR stageMask, const AccessContext *access_context)
    : SyncOpBase(command),
      event_(sync_state.Get<EVENT_STATE>(event)),
      recorded_context_(),
      src_exec_scope_(SyncExecScope::MakeSrc(queue_flags, stageMask)),
      dep_info_() {
    // Snapshot the current access_context for later inspection at wait time.
    // NOTE: This appears brute force, but given that we only save a "first-last" model of access history, the current
    //       access context (include barrier state for chaining) won't necessarily contain the needed information at Wait
    //       or Submit time reference.
    if (access_context) {
        recorded_context_ = std::make_shared<const AccessContext>(*access_context);
    }
}

SyncOpSetEvent::SyncOpSetEvent(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkEvent event,
                               const VkDependencyInfoKHR &dep_info, const AccessContext *access_context)
    : SyncOpBase(command),
      event_(sync_state.Get<EVENT_STATE>(event)),
      recorded_context_(),
      src_exec_scope_(SyncExecScope::MakeSrc(queue_flags, sync_utils::GetGlobalStageMasks(dep_info).src)),
      dep_info_(new safe_VkDependencyInfo(&dep_info)) {
    if (access_context) {
        recorded_context_ = std::make_shared<const AccessContext>(*access_context);
    }
}

bool SyncOpSetEvent::Validate(const CommandBufferAccessContext &cb_context) const {
    return DoValidate(cb_context, ResourceUsageRecord::kMaxIndex);
}
bool SyncOpSetEvent::ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const {
    return DoValidate(replay.GetExecutionContext(), replay.GetBaseTag() + recorded_tag);
}

bool SyncOpSetEvent::DoValidate(const CommandExecutionContext &exec_context, const ResourceUsageTag base_tag) const {
    bool skip = false;

    const auto &sync_state = exec_context.GetSyncState();
    auto *events_context = exec_context.GetCurrentEventsContext();
    assert(events_context);
    if (!events_context) return skip;

    const auto *sync_event = events_context->Get(event_);
    if (!sync_event) return skip;  // Core, Lifetimes, or Param check needs to catch invalid events.

    if (sync_event->last_command_tag >= base_tag) return skip;  // for replay we don't want to revalidate internal "last commmand"

    const char *const reset_set =
        "%s: %s %s operation following %s without intervening execution barrier, is a race condition and may result in data "
        "hazards.";
    const char *const wait =
        "%s: %s %s operation following %s without intervening vkCmdResetEvent, may result in data hazard and is ignored.";

    if (!sync_event->HasBarrier(src_exec_scope_.mask_param, src_exec_scope_.exec_scope)) {
        const char *vuid_stem = nullptr;
        const char *message = nullptr;
        switch (sync_event->last_command) {
            case vvl::Func::vkCmdResetEvent:
            case vvl::Func::vkCmdResetEvent2KHR:
            case vvl::Func::vkCmdResetEvent2:
                // Needs a barrier between reset and set
                vuid_stem = "-missingbarrier-reset";
                message = reset_set;
                break;
            case vvl::Func::vkCmdSetEvent:
            case vvl::Func::vkCmdSetEvent2KHR:
            case vvl::Func::vkCmdSetEvent2:
                // Needs a barrier between set and set
                vuid_stem = "-missingbarrier-set";
                message = reset_set;
                break;
            case vvl::Func::vkCmdWaitEvents:
            case vvl::Func::vkCmdWaitEvents2KHR:
            case vvl::Func::vkCmdWaitEvents2:
                // Needs a barrier or is in second execution scope
                vuid_stem = "-missingbarrier-wait";
                message = wait;
                break;
            default:
                // The only other valid last command that wasn't one.
                assert(sync_event->last_command == vvl::Func::Empty);
                break;
        }
        if (vuid_stem) {
            assert(nullptr != message);
            std::string vuid("SYNC-");
            vuid.append(CmdName()).append(vuid_stem);
            skip |= sync_state.LogError(event_->event(), vuid.c_str(), message, CmdName(),
                                        sync_state.FormatHandle(event_->event()).c_str(), CmdName(),
                                        vvl::String(sync_event->last_command));
        }
    }

    return skip;
}

ResourceUsageTag SyncOpSetEvent::Record(CommandBufferAccessContext *cb_context) {
    const auto tag = cb_context->NextCommandTag(command_);
    auto *events_context = cb_context->GetCurrentEventsContext();
    const QueueId queue_id = cb_context->GetQueueId();
    assert(recorded_context_);
    if (recorded_context_ && events_context) {
        DoRecord(queue_id, tag, recorded_context_, events_context);
    }
    return tag;
}

void SyncOpSetEvent::ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const {
    // Create a copy of the current context, and merge in the state snapshot at record set event time
    // Note: we mustn't change the recorded context copy, as a given CB could be submitted more than once (in generaL)
    if (!exec_context.ValidForSyncOps()) return;
    SyncEventsContext *events_context = exec_context.GetCurrentEventsContext();
    AccessContext *access_context = exec_context.GetCurrentAccessContext();
    const QueueId queue_id = exec_context.GetQueueId();

    // Note: merged_context is a copy of the access_context, combined with the recorded context
    auto merged_context = std::make_shared<AccessContext>(*access_context);
    merged_context->ResolveFromContext(QueueTagOffsetBarrierAction(queue_id, exec_tag), *recorded_context_);
    merged_context->Trim();  // Ensure the copy is minimal and normalized
    DoRecord(queue_id, exec_tag, merged_context, events_context);
}

void SyncOpSetEvent::DoRecord(QueueId queue_id, ResourceUsageTag tag, const std::shared_ptr<const AccessContext> &access_context,
                              SyncEventsContext *events_context) const {
    auto *sync_event = events_context->GetFromShared(event_);
    if (!sync_event) return;  // Core, Lifetimes, or Param check needs to catch invalid events.

    // NOTE: We're going to simply record the sync scope here, as anything else would be implementation defined/undefined
    //       and we're issuing errors re: missing barriers between event commands, which if the user fixes would fix
    //       any issues caused by naive scope setting here.

    // What happens with two SetEvent is that one cannot know what group of operations will be waited for.
    // Given:
    //     Stuff1; SetEvent; Stuff2; SetEvent; WaitEvents;
    // WaitEvents cannot know which of Stuff1, Stuff2, or both has completed execution.

    if (!sync_event->HasBarrier(src_exec_scope_.mask_param, src_exec_scope_.exec_scope)) {
        sync_event->unsynchronized_set = sync_event->last_command;
        sync_event->ResetFirstScope();
    } else if (!sync_event->first_scope) {
        // We only set the scope if there isn't one
        sync_event->scope = src_exec_scope_;

        // Save the shared_ptr to copy of the access_context present at set time (sent us by the caller)
        sync_event->first_scope = access_context;
        sync_event->unsynchronized_set = vvl::Func::Empty;
        sync_event->first_scope_tag = tag;
    }
    // TODO: Store dep_info_ shared ptr in sync_state for WaitEvents2 validation
    sync_event->last_command = command_;
    sync_event->last_command_tag = tag;
    sync_event->barriers = 0U;
}

SyncOpBeginRenderPass::SyncOpBeginRenderPass(vvl::Func command, const SyncValidator &sync_state,
                                             const VkRenderPassBeginInfo *pRenderPassBegin,
                                             const VkSubpassBeginInfo *pSubpassBeginInfo)
    : SyncOpBase(command), rp_context_(nullptr) {
    if (pRenderPassBegin) {
        rp_state_ = sync_state.Get<RENDER_PASS_STATE>(pRenderPassBegin->renderPass);
        renderpass_begin_info_ = safe_VkRenderPassBeginInfo(pRenderPassBegin);
        auto fb_state = sync_state.Get<FRAMEBUFFER_STATE>(pRenderPassBegin->framebuffer);
        if (fb_state) {
            shared_attachments_ = sync_state.GetAttachmentViews(*renderpass_begin_info_.ptr(), *fb_state);
            // TODO: Revisit this when all attachment validation is through SyncOps to see if we can discard the plain pointer copy
            // Note that this a safe to presist as long as shared_attachments is not cleared
            attachments_.reserve(shared_attachments_.size());
            for (const auto &attachment : shared_attachments_) {
                attachments_.emplace_back(static_cast<const syncval_state::ImageViewState *>(attachment.get()));
            }
        }
        if (pSubpassBeginInfo) {
            subpass_begin_info_ = safe_VkSubpassBeginInfo(pSubpassBeginInfo);
        }
    }
}

bool SyncOpBeginRenderPass::Validate(const CommandBufferAccessContext &cb_context) const {
    // Check if any of the layout transitions are hazardous.... but we don't have the renderpass context to work with, so we
    bool skip = false;

    assert(rp_state_.get());
    if (nullptr == rp_state_.get()) return skip;
    auto &rp_state = *rp_state_.get();

    const uint32_t subpass = 0;

    // Construct the state we can use to validate against... (since validation is const and RecordCmdBeginRenderPass
    // hasn't happened yet)
    const std::vector<AccessContext> empty_context_vector;
    AccessContext temp_context(subpass, cb_context.GetQueueFlags(), rp_state.subpass_dependencies, empty_context_vector,
                               cb_context.GetCurrentAccessContext());

    // Validate attachment operations
    if (attachments_.size() == 0) return skip;
    const auto &render_area = renderpass_begin_info_.renderArea;

    // Since the isn't a valid RenderPassAccessContext until Record, needs to create the view/generator list... we could limit this
    // by predicating on whether subpass 0 uses the attachment if it is too expensive to create the full list redundantly here.
    // More broadly we could look at thread specific state shared between Validate and Record as is done for other heavyweight
    // operations (though it's currently a messy approach)
    AttachmentViewGenVector view_gens = RenderPassAccessContext::CreateAttachmentViewGen(render_area, attachments_);
    skip |= temp_context.ValidateLayoutTransitions(cb_context, rp_state, render_area, subpass, view_gens, command_);

    // Validate load operations if there were no layout transition hazards
    if (!skip) {
        temp_context.RecordLayoutTransitions(rp_state, subpass, view_gens, kInvalidTag);
        skip |= temp_context.ValidateLoadOperation(cb_context, rp_state, render_area, subpass, view_gens, command_);
    }

    return skip;
}

ResourceUsageTag SyncOpBeginRenderPass::Record(CommandBufferAccessContext *cb_context) {
    assert(rp_state_.get());
    if (nullptr == rp_state_.get()) return cb_context->NextCommandTag(command_);
    const ResourceUsageTag begin_tag =
        cb_context->RecordBeginRenderPass(command_, *rp_state_.get(), renderpass_begin_info_.renderArea, attachments_);

    // Note: this state update must be after RecordBeginRenderPass as there is no current render pass until that function runs
    rp_context_ = cb_context->GetCurrentRenderPassContext();

    return begin_tag;
}

bool SyncOpBeginRenderPass::ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const {
    // Need to update the exec_contexts state (which for RenderPass operations *must* be a QueueBatchContext, as
    // render pass operations are not allowed in secondary command buffers.
    replay.BeginRenderPassReplaySetup(*this);

    // Only the layout transitions happen at the replay tag, loadOp's happen at a subsequent tag
    ResourceUsageRange first_use_range = {recorded_tag, recorded_tag + 1};
    return replay.DetectFirstUseHazard(first_use_range);
}

void SyncOpBeginRenderPass::ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const {
    // All the needed replay state changes (for the layout transition, and context update) have to happen in ReplayValidate
}

SyncOpNextSubpass::SyncOpNextSubpass(vvl::Func command, const SyncValidator &sync_state,
                                     const VkSubpassBeginInfo *pSubpassBeginInfo, const VkSubpassEndInfo *pSubpassEndInfo)
    : SyncOpBase(command) {
    if (pSubpassBeginInfo) {
        subpass_begin_info_.initialize(pSubpassBeginInfo);
    }
    if (pSubpassEndInfo) {
        subpass_end_info_.initialize(pSubpassEndInfo);
    }
}

bool SyncOpNextSubpass::Validate(const CommandBufferAccessContext &cb_context) const {
    bool skip = false;
    const auto *renderpass_context = cb_context.GetCurrentRenderPassContext();
    if (!renderpass_context) return skip;

    skip |= renderpass_context->ValidateNextSubpass(cb_context.GetExecutionContext(), command_);
    return skip;
}

ResourceUsageTag SyncOpNextSubpass::Record(CommandBufferAccessContext *cb_context) {
    return cb_context->RecordNextSubpass(command_);
}

bool SyncOpNextSubpass::ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const {
    // Any store/resolve operations happen before the NextSubpass tag so we can advance to the next subpass state
    replay.NextSubpassReplaySetup();

    // Only the layout transitions happen at the replay tag, loadOp's happen at a subsequent tag
    ResourceUsageRange first_use_range = {recorded_tag, recorded_tag + 1};
    return replay.DetectFirstUseHazard(first_use_range);
}

void SyncOpNextSubpass::ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const {
    // All the needed replay state changes (for the layout transition, and context update) have to happen in ReplayValidate
}
SyncOpEndRenderPass::SyncOpEndRenderPass(vvl::Func command, const SyncValidator &sync_state,
                                         const VkSubpassEndInfo *pSubpassEndInfo)
    : SyncOpBase(command) {
    if (pSubpassEndInfo) {
        subpass_end_info_.initialize(pSubpassEndInfo);
    }
}

bool SyncOpEndRenderPass::Validate(const CommandBufferAccessContext &cb_context) const {
    bool skip = false;
    const auto *renderpass_context = cb_context.GetCurrentRenderPassContext();

    if (!renderpass_context) return skip;
    skip |= renderpass_context->ValidateEndRenderPass(cb_context.GetExecutionContext(), command_);
    return skip;
}

ResourceUsageTag SyncOpEndRenderPass::Record(CommandBufferAccessContext *cb_context) {
    return cb_context->RecordEndRenderPass(command_);
}

bool SyncOpEndRenderPass::ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const {
    // Any store/resolve operations happen before the EndRenderPass tag so we can ignore them
    // Only the layout transitions happen at the replay tag
    ResourceUsageRange first_use_range = {recorded_tag, recorded_tag + 1};
    bool skip = replay.DetectFirstUseHazard(first_use_range);

    // We can cleanup here as the recorded tag represents the final layout transition (which is the last operation or the RP
    replay.EndRenderPassReplayCleanup();

    return skip;
}

void SyncOpEndRenderPass::ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const {
}

void SyncValidator::PreCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR pipelineStage,
                                                          VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) {
    StateTracker::PreCallRecordCmdWriteBufferMarker2AMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(Func::vkCmdWriteBufferMarker2AMD);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, 4);
        context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
    }
}

bool SyncValidator::PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                                      const VkCommandBuffer *pCommandBuffers, const ErrorObject &error_obj) const {
    bool skip = StateTracker::PreCallValidateCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers, error_obj);
    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;
    const auto *cb_context = &cb_state->access_context;

    // Heavyweight, but we need a proxy copy of the active command buffer access context
    CommandBufferAccessContext proxy_cb_context(*cb_context, CommandBufferAccessContext::AsProxyContext());

    // Make working copies of the access and events contexts
    for (uint32_t cb_index = 0; cb_index < commandBufferCount; ++cb_index) {
        proxy_cb_context.NextIndexedCommandTag(error_obj.location.function, cb_index);

        const auto recorded_cb = Get<syncval_state::CommandBuffer>(pCommandBuffers[cb_index]);
        if (!recorded_cb) continue;
        const auto *recorded_cb_context = &recorded_cb->access_context;
        assert(recorded_cb_context);

        skip |= ReplayState(proxy_cb_context, *recorded_cb_context, error_obj, cb_index).ValidateFirstUse();

        // The barriers have already been applied in ValidatFirstUse
        ResourceUsageRange tag_range = proxy_cb_context.ImportRecordedAccessLog(*recorded_cb_context);
        proxy_cb_context.ResolveExecutedCommandBuffer(*recorded_cb_context->GetCurrentAccessContext(), tag_range.begin);
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                                    const VkCommandBuffer *pCommandBuffers) {
    StateTracker::PreCallRecordCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;
    for (uint32_t cb_index = 0; cb_index < commandBufferCount; ++cb_index) {
        const ResourceUsageTag cb_tag = cb_context->NextIndexedCommandTag(vvl::Func::vkCmdExecuteCommands, cb_index);
        const auto recorded_cb = Get<syncval_state::CommandBuffer>(pCommandBuffers[cb_index]);
        if (!recorded_cb) continue;
        cb_context->AddHandle(cb_tag, "pCommandBuffers", recorded_cb->Handle(), cb_index);
        const auto *recorded_cb_context = &recorded_cb->access_context;
        cb_context->RecordExecutedCommandBuffer(*recorded_cb_context);
    }
}

void SyncValidator::PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memoryOffset,
                                                  const RecordObject &record_obj) {
    StateTracker::PostCallRecordBindImageMemory(device, image, mem, memoryOffset, record_obj);
    if (VK_SUCCESS != record_obj.result) return;
    const VkBindImageMemoryInfo bind_info = ConvertImageMemoryInfo(device, image, mem, memoryOffset);
    UpdateSyncImageMemoryBindState(1, &bind_info);
}

void SyncValidator::PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos,
                                                   const RecordObject &record_obj) {
    StateTracker::PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, record_obj);
    if (VK_SUCCESS != record_obj.result) return;
    UpdateSyncImageMemoryBindState(bindInfoCount, pBindInfos);
}

void SyncValidator::PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                      const VkBindImageMemoryInfo *pBindInfos, const RecordObject &record_obj) {
    StateTracker::PostCallRecordBindImageMemory2KHR(device, bindInfoCount, pBindInfos, record_obj);
    if (VK_SUCCESS != record_obj.result) return;
    UpdateSyncImageMemoryBindState(bindInfoCount, pBindInfos);
}

void SyncValidator::PostCallRecordQueueWaitIdle(VkQueue queue, const RecordObject &record_obj) {
    StateTracker::PostCallRecordQueueWaitIdle(queue, record_obj);
    if ((record_obj.result != VK_SUCCESS) || (!enabled[sync_validation_queue_submit]) || (queue == VK_NULL_HANDLE)) return;

    const auto queue_state = GetQueueSyncStateShared(queue);
    if (!queue_state) return;  // Invalid queue
    QueueId waited_queue = queue_state->GetQueueId();
    ApplyTaggedWait(waited_queue, ResourceUsageRecord::kMaxIndex);

    // Eliminate waitable fences from the current queue.
    vvl::EraseIf(waitable_fences_, [waited_queue](const SignaledFence &sf) { return sf.second.queue_id == waited_queue; });
}

void SyncValidator::PostCallRecordDeviceWaitIdle(VkDevice device, const RecordObject &record_obj) {
    StateTracker::PostCallRecordDeviceWaitIdle(device, record_obj);

    // We need to treat this a fence waits for all queues... noting that present engine ops will be preserved.
    ForAllQueueBatchContexts([](const std::shared_ptr<QueueBatchContext> &batch) {
        batch->ApplyTaggedWait(QueueSyncState::kQueueAny, ResourceUsageRecord::kMaxIndex);
    });

    // As we we've waited for everything on device, any waits are mooted. (except for acquires)
    vvl::EraseIf(waitable_fences_, [](SignaledFences::value_type &waitable) { return waitable.second.acquired.Invalid(); });
}

struct QueuePresentCmdState {
    std::shared_ptr<const QueueSyncState> queue;
    std::shared_ptr<QueueBatchContext> present_batch;
    SignaledSemaphores signaled;
    PresentedImages presented_images;
    QueuePresentCmdState(const SignaledSemaphores &parent_semaphores) : signaled(parent_semaphores) {}
};

bool SyncValidator::PreCallValidateQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo,
                                                   const ErrorObject &error_obj) const {
    bool skip = false;

    // Since this early return is above the TlsGuard, the Record phase must also be.
    if (!enabled[sync_validation_queue_submit]) return skip;

    vvl::TlsGuard<QueuePresentCmdState> cmd_state(&skip, signaled_semaphores_);
    cmd_state->queue = GetQueueSyncStateShared(queue);
    if (!cmd_state->queue) return skip;  // Invalid Queue

    // The submit id is a mutable automic which is not recoverable on a skip == true condition
    uint64_t submit_id = cmd_state->queue->ReserveSubmitId();

    std::shared_ptr<const QueueBatchContext> last_batch = cmd_state->queue->LastBatch();
    std::shared_ptr<QueueBatchContext> batch(std::make_shared<QueueBatchContext>(*this, *cmd_state->queue, submit_id, 0));

    ResourceUsageRange tag_range = SetupPresentInfo(*pPresentInfo, batch, cmd_state->presented_images);
    batch->SetupAccessContext(last_batch, *pPresentInfo, cmd_state->presented_images, cmd_state->signaled);
    batch->SetupBatchTags(tag_range);
    // Update the present tags
    for (auto &presented : cmd_state->presented_images) {
        presented.tag += batch->GetTagRange().begin;
    }

    skip |= batch->DoQueuePresentValidate(error_obj.location, cmd_state->presented_images);
    batch->DoPresentOperations(cmd_state->presented_images);
    batch->LogPresentOperations(cmd_state->presented_images);
    batch->Cleanup();

    if (!skip) {
        cmd_state->present_batch = std::move(batch);
    }
    return skip;
}

ResourceUsageRange SyncValidator::SetupPresentInfo(const VkPresentInfoKHR &present_info, std::shared_ptr<QueueBatchContext> &batch,
                                                   PresentedImages &presented_images) const {
    const VkSwapchainKHR *const swapchains = present_info.pSwapchains;
    const uint32_t *const image_indices = present_info.pImageIndices;
    const uint32_t swap_count = present_info.swapchainCount;

    // Create the working list of presented images
    presented_images.reserve(swap_count);
    for (uint32_t present_index = 0; present_index < swap_count; present_index++) {
        // Note: Given the "EraseIf" implementation for acquire fence waits, each presentation needs a unique tag.
        const ResourceUsageTag tag = presented_images.size();
        presented_images.emplace_back(*this, batch, swapchains[present_index], image_indices[present_index], present_index, tag);
        if (presented_images.back().Invalid()) {
            presented_images.pop_back();
        }
    }

    // Present is tagged for each swap.
    return ResourceUsageRange(0, presented_images.size());
}

void SyncValidator::PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo,
                                                  const RecordObject &record_obj) {
    StateTracker::PostCallRecordQueuePresentKHR(queue, pPresentInfo, record_obj);
    if (!enabled[sync_validation_queue_submit]) return;

    // The earliest return (when enabled), must be *after* the TlsGuard, as it is the TlsGuard that cleans up the cmd_state
    // static payload
    vvl::TlsGuard<QueuePresentCmdState> cmd_state;

    // See ValidationStateTracker::PostCallRecordQueuePresentKHR for spec excerpt supporting
    if (record_obj.result == VK_ERROR_OUT_OF_HOST_MEMORY || record_obj.result == VK_ERROR_OUT_OF_DEVICE_MEMORY ||
        record_obj.result == VK_ERROR_DEVICE_LOST) {
        return;
    }

    // Update the state with the data from the validate phase
    cmd_state->signaled.Resolve(signaled_semaphores_, cmd_state->present_batch);
    std::shared_ptr<QueueSyncState> queue_state = std::const_pointer_cast<QueueSyncState>(std::move(cmd_state->queue));
    for (auto &presented : cmd_state->presented_images) {
        presented.ExportToSwapchain(*this);
    }
    queue_state->UpdateLastBatch(std::move(cmd_state->present_batch));
}

void SyncValidator::PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                      VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex,
                                                      const RecordObject &record_obj) {
    StateTracker::PostCallRecordAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex, record_obj);
    if (!enabled[sync_validation_queue_submit]) return;
    RecordAcquireNextImageState(device, swapchain, timeout, semaphore, fence, pImageIndex, record_obj);
}

void SyncValidator::PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                       uint32_t *pImageIndex, const RecordObject &record_obj) {
    StateTracker::PostCallRecordAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex, record_obj);
    if (!enabled[sync_validation_queue_submit]) return;
    RecordAcquireNextImageState(device, pAcquireInfo->swapchain, pAcquireInfo->timeout, pAcquireInfo->semaphore,
                                pAcquireInfo->fence, pImageIndex, record_obj);
}

void SyncValidator::RecordAcquireNextImageState(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore,
                                                VkFence fence, uint32_t *pImageIndex, const RecordObject &record_obj) {
    if ((VK_SUCCESS != record_obj.result) && (VK_SUBOPTIMAL_KHR != record_obj.result)) return;

    // Get the image out of the presented list and create apppropriate fences/semaphores.
    auto swapchain_state = Get<syncval_state::Swapchain>(swapchain);
    if (BASE_NODE::Invalid(swapchain_state)) return;  // Invalid acquire calls to be caught in CoreCheck/Parameter validation

    PresentedImage presented = swapchain_state->MovePresentedImage(*pImageIndex);
    if (presented.Invalid()) return;

    // No way to make access safe, so nothing to record
    if ((semaphore == VK_NULL_HANDLE) && (fence == VK_NULL_HANDLE)) return;

    // We create a queue-less QBC for the Semaphore and fences to wait on

    // Note: this is a heavyweight way to deal with the fact that all operation logs live in the QueueBatchContext... and
    // acquire doesn't happen on a queue, but we need a place to put the acquire operation access record.
    auto batch = std::make_shared<QueueBatchContext>(*this);
    batch->SetupAccessContext(presented);
    ResourceUsageRange acquire_tag_range(0, 1);
    batch->SetupBatchTags(ResourceUsageRange(0, 1));
    const ResourceUsageTag acquire_tag = batch->GetTagRange().begin;
    batch->DoAcquireOperation(presented);
    batch->LogAcquireOperation(presented, record_obj.location.function);

    // Now swap out the present queue batch with the acquired one.
    // Note that fence and signal will read the acquire batch from presented, so this needs to be done before
    // setting up the synchronization
    presented.batch = std::move(batch);

    if (semaphore != VK_NULL_HANDLE) {
        std::shared_ptr<const SEMAPHORE_STATE> sem_state = Get<SEMAPHORE_STATE>(semaphore);

        if (bool(sem_state)) {
            signaled_semaphores_.SignalSemaphore(sem_state, presented, acquire_tag);
        }
    }
    if (fence != VK_NULL_HANDLE) {
        UpdateFenceWaitInfo(fence, presented, acquire_tag);
    }
}

bool SyncValidator::PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                               const ErrorObject &error_obj) const {
    auto queue_state = GetQueueSyncStateShared(queue);
    if (!bool(queue_state)) return false;
    SubmitInfoConverter submit_info(submitCount, pSubmits, queue_state->GetQueueFlags());
    return ValidateQueueSubmit(queue, submitCount, submit_info.info2s.data(), fence, error_obj);
}

bool SyncValidator::ValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits, VkFence fence,
                                        const ErrorObject &error_obj) const {
    bool skip = false;

    // Since this early return is above the TlsGuard, the Record phase must also be.
    if (!enabled[sync_validation_queue_submit]) return skip;

    vvl::TlsGuard<QueueSubmitCmdState> cmd_state(&skip, error_obj, signaled_semaphores_);
    cmd_state->queue = GetQueueSyncStateShared(queue);
    if (!cmd_state->queue) return skip;  // Invalid Queue

    // The submit id is a mutable automic which is not recoverable on a skip == true condition
    uint64_t submit_id = cmd_state->queue->ReserveSubmitId();

    // verify each submit batch
    // Since the last batch from the queue state is const, we need to track the last_batch separately from the
    // most recently created batch
    std::shared_ptr<const QueueBatchContext> last_batch = cmd_state->queue->LastBatch();
    std::shared_ptr<QueueBatchContext> batch;
    for (uint32_t batch_idx = 0; batch_idx < submitCount; batch_idx++) {
        const VkSubmitInfo2 &submit = pSubmits[batch_idx];
        batch = std::make_shared<QueueBatchContext>(*this, *cmd_state->queue, submit_id, batch_idx);
        batch->SetupCommandBufferInfo(submit);
        batch->SetupAccessContext(last_batch, submit, cmd_state->signaled);

        // Skip import and validation of empty batches
        if (batch->GetTagRange().size()) {
            batch->SetupBatchTags();
            skip |= batch->DoQueueSubmitValidate(*this, *cmd_state, submit);
        }

        // Empty batches could have semaphores, though.
        for (uint32_t sem_idx = 0; sem_idx < submit.signalSemaphoreInfoCount; ++sem_idx) {
            const VkSemaphoreSubmitInfo &semaphore_info = submit.pSignalSemaphoreInfos[sem_idx];
            // Make a copy of the state, signal the copy and pend it...
            auto sem_state = Get<SEMAPHORE_STATE>(semaphore_info.semaphore);
            if (!sem_state) continue;
            cmd_state->signaled.SignalSemaphore(sem_state, batch, semaphore_info);
        }
        // Unless the previous batch was referenced by a signal, the QueueBatchContext will self destruct, but as
        // we ResolvePrevious as we can let any contexts we've fully referenced go.
        batch->Cleanup();  // Clear the temporaries that the batch holds.
        last_batch = batch;
    }
    // The most recently created batch will become the queue's "last batch" in the record phase
    if (batch) {
        cmd_state->last_batch = std::move(batch);
    }

    // Note that if we skip, guard cleans up for us, but cannot release the reserved tag range
    return skip;
}

void SyncValidator::PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence,
                                              const RecordObject &record_obj) {
    StateTracker::PostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, record_obj);

    RecordQueueSubmit(queue, fence, record_obj);
}

void SyncValidator::RecordQueueSubmit(VkQueue queue, VkFence fence, const RecordObject &record_obj) {
    // If this return is above the TlsGuard, then the Validate phase return must also be.
    if (!enabled[sync_validation_queue_submit]) return;  // Queue submit validation must be affirmatively enabled

    // The earliest return (when enabled), must be *after* the TlsGuard, as it is the TlsGuard that cleans up the cmd_state
    // static payload
    vvl::TlsGuard<QueueSubmitCmdState> cmd_state;

    if (VK_SUCCESS != record_obj.result) return;  // dispatched QueueSubmit failed
    if (!cmd_state->queue) return;     // Validation couldn't find a valid queue object

    // Don't need to look up the queue state again, but we need a non-const version
    std::shared_ptr<QueueSyncState> queue_state = std::const_pointer_cast<QueueSyncState>(std::move(cmd_state->queue));

    cmd_state->signaled.Resolve(signaled_semaphores_, cmd_state->last_batch);
    queue_state->UpdateLastBatch(std::move(cmd_state->last_batch));

    ResourceUsageRange fence_tag_range = ReserveGlobalTagRange(1U);
    UpdateFenceWaitInfo(fence, queue_state->GetQueueId(), fence_tag_range.begin);
}

bool SyncValidator::PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits,
                                                   VkFence fence, const ErrorObject &error_obj) const {
    return PreCallValidateQueueSubmit2(queue, submitCount, pSubmits, fence, error_obj);
}

bool SyncValidator::PreCallValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits,
                                                VkFence fence, const ErrorObject &error_obj) const {
    return ValidateQueueSubmit(queue, submitCount, pSubmits, fence, error_obj);
}

void SyncValidator::PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits,
                                                  VkFence fence, const RecordObject &record_obj) {
    StateTracker::PostCallRecordQueueSubmit2KHR(queue, submitCount, pSubmits, fence, record_obj);
    RecordQueueSubmit(queue, fence, record_obj);
}
void SyncValidator::PostCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits, VkFence fence,
                                               const RecordObject &record_obj) {
    StateTracker::PostCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence, record_obj);
    RecordQueueSubmit(queue, fence, record_obj);
}

void SyncValidator::PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, const RecordObject &record_obj) {
    StateTracker::PostCallRecordGetFenceStatus(device, fence, record_obj);
    if (!enabled[sync_validation_queue_submit]) return;
    if (record_obj.result == VK_SUCCESS) {
        // fence is signalled, mark it as waited for
        WaitForFence(fence);
    }
}

void SyncValidator::PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences, VkBool32 waitAll,
                                                uint64_t timeout, const RecordObject &record_obj) {
    StateTracker::PostCallRecordWaitForFences(device, fenceCount, pFences, waitAll, timeout, record_obj);
    if (!enabled[sync_validation_queue_submit]) return;
    if ((record_obj.result == VK_SUCCESS) && ((VK_TRUE == waitAll) || (1 == fenceCount))) {
        // We can only know the pFences have signal if we waited for all of them, or there was only one of them
        for (uint32_t i = 0; i < fenceCount; i++) {
            WaitForFence(pFences[i]);
        }
    }
}

void SyncValidator::PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount,
                                                        VkImage *pSwapchainImages, const RecordObject &record_obj) {
    StateTracker::PostCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages, record_obj);
    if ((record_obj.result != VK_SUCCESS) && (record_obj.result != VK_INCOMPLETE)) return;
    auto swapchain_state = Get<SWAPCHAIN_NODE>(swapchain);

    if (pSwapchainImages) {
        for (uint32_t i = 0; i < *pSwapchainImageCount; ++i) {
            SWAPCHAIN_IMAGE &swapchain_image = swapchain_state->images[i];
            if (swapchain_image.image_state) {
                auto *sync_image = static_cast<ImageState *>(swapchain_image.image_state);
                assert(sync_image->IsTiled());  // This is the assumption from the spec, and the implementation relies on it
                sync_image->SetOpaqueBaseAddress(*this);
            }
        }
    }
}

AttachmentViewGen::AttachmentViewGen(const syncval_state::ImageViewState *image_view, const VkOffset3D &offset,
                                     const VkExtent3D &extent)
    : view_(image_view), view_mask_(image_view->normalized_subresource_range.aspectMask), gen_store_() {
    gen_store_[Gen::kViewSubresource].emplace(image_view->GetFullViewImageRangeGen());
    gen_store_[Gen::kRenderArea].emplace(image_view->MakeImageRangeGen(offset, extent));

    const auto depth = view_mask_ & VK_IMAGE_ASPECT_DEPTH_BIT;
    if (depth && (depth != view_mask_)) {
        gen_store_[Gen::kDepthOnlyRenderArea].emplace(image_view->MakeImageRangeGen(offset, extent, depth));
    }
    const auto stencil = view_mask_ & VK_IMAGE_ASPECT_STENCIL_BIT;
    if (stencil && (stencil != view_mask_)) {
        gen_store_[Gen::kStencilOnlyRenderArea].emplace(image_view->MakeImageRangeGen(offset, extent, stencil));
    }
}

const std::optional<ImageRangeGen> &AttachmentViewGen::GetRangeGen(AttachmentViewGen::Gen type) const {
    static_assert(Gen::kGenSize == 4, "Function written with this assumption");
    // If the view is a depth only view, then the depth only portion of the render area is simply the render area.
    // If the view is a depth stencil view, then the depth only portion of the render area will be a subset,
    // and thus needs the generator function that will produce the address ranges of that subset
    const bool depth_only = (type == kDepthOnlyRenderArea) && (view_mask_ == VK_IMAGE_ASPECT_DEPTH_BIT);
    const bool stencil_only = (type == kStencilOnlyRenderArea) && (view_mask_ == VK_IMAGE_ASPECT_STENCIL_BIT);
    if (depth_only || stencil_only) {
        type = Gen::kRenderArea;
    }
    return gen_store_[type];
}

AttachmentViewGen::Gen AttachmentViewGen::GetDepthStencilRenderAreaGenType(bool depth_op, bool stencil_op) const {
    assert(IsValid());
    assert(view_mask_ & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT));
    if (depth_op) {
        assert(view_mask_ & VK_IMAGE_ASPECT_DEPTH_BIT);
        if (stencil_op) {
            assert(view_mask_ & VK_IMAGE_ASPECT_STENCIL_BIT);
            return kRenderArea;
        }
        return kDepthOnlyRenderArea;
    }
    if (stencil_op) {
        assert(view_mask_ & VK_IMAGE_ASPECT_STENCIL_BIT);
        return kStencilOnlyRenderArea;
    }

    assert(depth_op || stencil_op);
    return kRenderArea;
}


void SyncEventsContext::ApplyBarrier(const SyncExecScope &src, const SyncExecScope &dst, ResourceUsageTag tag) {
    const bool all_commands_bit = 0 != (src.mask_param & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    for (auto &event_pair : map_) {
        assert(event_pair.second);  // Shouldn't be storing empty
        auto &sync_event = *event_pair.second;
        // Events don't happen at a stage, so we need to check and store the unexpanded ALL_COMMANDS if set for inter-event-calls
        // But only if occuring before the tag
        if (((sync_event.barriers & src.exec_scope) || all_commands_bit) && (sync_event.last_command_tag <= tag)) {
            sync_event.barriers |= dst.exec_scope;
            sync_event.barriers |= dst.mask_param & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        }
    }
}

void SyncEventsContext::ApplyTaggedWait(VkQueueFlags queue_flags, ResourceUsageTag tag) {
    const SyncExecScope src_scope =
        SyncExecScope::MakeSrc(queue_flags, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_2_HOST_BIT);
    const SyncExecScope dst_scope = SyncExecScope::MakeDst(queue_flags, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
    ApplyBarrier(src_scope, dst_scope, tag);
}

SyncEventsContext &SyncEventsContext::DeepCopy(const SyncEventsContext &from) {
    // We need a deep copy of the const context to update during validation phase
    for (const auto &event : from.map_) {
        map_.emplace(event.first, std::make_shared<SyncEventState>(*event.second));
    }
    return *this;
}

void SyncEventsContext::AddReferencedTags(ResourceUsageTagSet &referenced) const {
    for (const auto &event : map_) {
        const std::shared_ptr<const SyncEventState> &event_state = event.second;
        if (event_state) {
            event_state->AddReferencedTags(referenced);
        }
    }
}

QueueBatchContext::QueueBatchContext(const SyncValidator &sync_state, const QueueSyncState &queue_state, uint64_t submit_index,
                                     uint32_t batch_index)
    : CommandExecutionContext(&sync_state),
      queue_state_(&queue_state),
      tag_range_(0, 0),
      current_access_context_(&access_context_),
      batch_log_(),
      queue_sync_tag_(sync_state.GetQueueIdLimit(), ResourceUsageTag(0)),
      batch_(queue_state, submit_index, batch_index) {}

QueueBatchContext::QueueBatchContext(const SyncValidator &sync_state)
    : CommandExecutionContext(&sync_state),
      queue_state_(),
      tag_range_(0, 0),
      current_access_context_(&access_context_),
      batch_log_(),
      queue_sync_tag_(sync_state.GetQueueIdLimit(), ResourceUsageTag(0)),
      batch_() {}

void QueueBatchContext::Trim() {
    // Clean up unneeded access context contents and log information
    access_context_.Trim();

    ResourceUsageTagSet used_tags;
    access_context_.AddReferencedTags(used_tags);

    // Note: AccessContexts in the SyncEventsState are trimmed when created.
    events_context_.AddReferencedTags(used_tags);

    // Only conserve AccessLog references that are referenced by used_tags
    batch_log_.Trim(used_tags);
}

void QueueBatchContext::ResolveSubmittedCommandBuffer(const AccessContext &recorded_context, ResourceUsageTag offset) {
    GetCurrentAccessContext()->ResolveFromContext(QueueTagOffsetBarrierAction(GetQueueId(), offset), recorded_context);
}

VulkanTypedHandle QueueBatchContext::Handle() const { return queue_state_->Handle(); }

template <typename Predicate>
void QueueBatchContext::ApplyPredicatedWait(Predicate &predicate) {
    access_context_.EraseIf([&predicate](ResourceAccessRangeMap::value_type &access) {
        // Apply..Wait returns true if the waited access is empty...
        return access.second.ApplyPredicatedWait<Predicate>(predicate);
    });
}

void QueueBatchContext::ApplyTaggedWait(QueueId queue_id, ResourceUsageTag tag) {
    const bool any_queue = (queue_id == QueueSyncState::kQueueAny);

    if (any_queue) {
        // This isn't just avoid an unneeded test, but to allow *all* queues to to be waited in a single pass
        // (and it does avoid doing the same test for every access, as well as avoiding the need for the predicate
        // to grok Queue/Device/Wait differences.
        ResourceAccessState::WaitTagPredicate predicate{tag};
        ApplyPredicatedWait(predicate);
    } else {
        ResourceAccessState::WaitQueueTagPredicate predicate{queue_id, tag};
        ApplyPredicatedWait(predicate);
    }

    // SwapChain acquire QBC's have no queue, but also, events are always empty.
    if (queue_state_ && (queue_id == GetQueueId() || any_queue)) {
        events_context_.ApplyTaggedWait(GetQueueFlags(), tag);
    }
}

void QueueBatchContext::ApplyAcquireWait(const AcquiredImage &acquired) {
    ResourceAccessState::WaitAcquirePredicate predicate{acquired.present_tag, acquired.acquire_tag};
    ApplyPredicatedWait(predicate);
}

void QueueBatchContext::BeginRenderPassReplaySetup(ReplayState &replay, const SyncOpBeginRenderPass &begin_op) {
    current_access_context_ = replay.ReplayStateRenderPassBegin(GetQueueFlags(), begin_op, access_context_);
}

void QueueBatchContext::NextSubpassReplaySetup(ReplayState &replay) {
    current_access_context_ = replay.ReplayStateRenderPassNext();
}

void QueueBatchContext::EndRenderPassReplayCleanup(ReplayState &replay) {
    replay.ReplayStateRenderPassEnd(access_context_);
    current_access_context_ = &access_context_;
}

void QueueBatchContext::Cleanup() {
    // Clear these after validation and import, not valid after.
    batch_ = BatchAccessLog::BatchRecord();
    command_buffers_.clear();
    async_batches_.clear();
}

AccessContext *ReplayState::RenderPassReplayState::Begin(VkQueueFlags queue_flags, const SyncOpBeginRenderPass &begin_op_,
                                                         const AccessContext &external_context) {
    Reset();

    begin_op = &begin_op_;
    subpass = 0;

    const RenderPassAccessContext *rp_context = begin_op->GetRenderPassAccessContext();
    assert(rp_context);
    replay_context = &rp_context->GetContexts()[0];

    InitSubpassContexts(queue_flags, *rp_context->GetRenderPassState(), &external_context, subpass_contexts);

    // Replace the Async contexts with the the async context of the "external" context
    // For replay we don't care about async subpasses, just async queue batches
    for (auto &context : subpass_contexts) {
        context.ClearAsyncContexts();
        context.ImportAsyncContexts(external_context);
    }

    return &subpass_contexts[0];
}

AccessContext *ReplayState::RenderPassReplayState::Next() {
    subpass++;

    const RenderPassAccessContext *rp_context = begin_op->GetRenderPassAccessContext();

    replay_context = &rp_context->GetContexts()[subpass];
    return &subpass_contexts[subpass];
}

void ReplayState::RenderPassReplayState::End(AccessContext &external_context) {
    external_context.ResolveChildContexts(subpass_contexts);
    Reset();
}

class ApplySemaphoreBarrierAction {
  public:
    ApplySemaphoreBarrierAction(const SemaphoreScope &signal, const SemaphoreScope &wait) : signal_(signal), wait_(wait) {}
    void operator()(ResourceAccessState *access) const { access->ApplySemaphore(signal_, wait_); }

  private:
    const SemaphoreScope &signal_;
    const SemaphoreScope wait_;
};

class ApplyAcquireNextSemaphoreAction {
  public:
    ApplyAcquireNextSemaphoreAction(const SyncExecScope &wait_scope, ResourceUsageTag acquire_tag)
        : barrier_(1, SyncBarrier(getPresentSrcScope(), getPresentValidAccesses(), wait_scope, SyncStageAccessFlags())),
          acq_tag_(acquire_tag) {}
    void operator()(ResourceAccessState *access) const {
        // Note that the present operations may or may not be present, given that the fence wait may have cleared them out.
        // Also, if a subsequent present has happened, we *don't* want to protect that...
        if (access->LastWriteTag() <= acq_tag_) {
            access->ApplyBarriersImmediate(barrier_);
        }
    }

  private:
    // kPresentSrcScope/kPresentValidAccesses cannot be regular global variables, because they use global
    // variables from another compilation unit (through syncStageAccessMaskByStageBit() call) for initialization,
    // and initialization of globals between compilation units is undefined. Instead they get initialized
    // on the first use (it's important to ensure this first use is also not initialization of some global!).
    const SyncExecScope &getPresentSrcScope() const {
        static const SyncExecScope kPresentSrcScope =
            SyncExecScope(VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL,  // mask_param (unused)
                          VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL,  // expanded_mask
                          VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL,  // exec_scope
                          getPresentValidAccesses());                      // valid_accesses
        return kPresentSrcScope;
    }
    const SyncStageAccessFlags &getPresentValidAccesses() const {
        static const SyncStageAccessFlags kPresentValidAccesses =
            SyncStageAccessFlags(SyncStageAccess::AccessScopeByStage(VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL));
        return kPresentValidAccesses;
    }

  private:
    std::vector<SyncBarrier> barrier_;
    ResourceUsageTag acq_tag_;
};

// Overload for QueuePresent semaphore waiting.  Not applicable to QueueSubmit semaphores
std::shared_ptr<QueueBatchContext> QueueBatchContext::ResolveOneWaitSemaphore(VkSemaphore sem,
                                                                              const PresentedImages &presented_images,
                                                                              SignaledSemaphores &signaled) {
    auto sem_state = sync_state_->Get<SEMAPHORE_STATE>(sem);
    if (!sem_state) return nullptr;  // Semaphore validity is handled by CoreChecks

    // When signal_state goes out of scope, the signal information will be dropped, as Unsignal has released ownership.
    auto signal_state = signaled.Unsignal(sem);
    if (!signal_state) return nullptr;  // Invalid signal, skip it.

    assert(signal_state->batch);

    const AccessContext &from_context = signal_state->batch->access_context_;
    const SemaphoreScope &signal_scope = signal_state->first_scope;
    const QueueId queue_id = GetQueueId();
    const auto queue_flags = queue_state_->GetQueueFlags();
    SemaphoreScope wait_scope{queue_id, SyncExecScope::MakeDst(queue_flags, VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL)};

    // If signal queue == wait queue, signal is treated as a memory barrier with an access scope equal to the present accesses
    SyncBarrier sem_barrier(signal_scope, wait_scope, SyncBarrier::AllAccess());
    const BatchBarrierOp sem_same_queue_op(wait_scope.queue, sem_barrier);

    // Need to import the rest of the same queue contents without modification
    SyncBarrier noop_barrier;
    const BatchBarrierOp noop_barrier_op(wait_scope.queue, noop_barrier);

    // Otherwise apply semaphore rules apply
    const ApplySemaphoreBarrierAction sem_not_same_queue_op(signal_scope, wait_scope);
    const SemaphoreScope noop_semaphore_scope(queue_id, noop_barrier.dst_exec_scope);
    const ApplySemaphoreBarrierAction noop_sem_op(signal_scope, noop_semaphore_scope);

    // For each presented image
    for (const auto &presented : presented_images) {
        // Need a copy that can be used as the pseudo-iterator...
        subresource_adapter::ImageRangeGenerator range_gen(presented.range_gen);
        if (signal_scope.queue == wait_scope.queue) {
            // If signal queue == wait queue, signal is treated as a memory barrier with an access scope equal to the
            // valid accesses for the sync scope.
            access_context_.ResolveFromContext(sem_same_queue_op, from_context, range_gen);
            access_context_.ResolveFromContext(noop_barrier_op, from_context);
        } else {
            access_context_.ResolveFromContext(sem_not_same_queue_op, from_context, range_gen);
            access_context_.ResolveFromContext(noop_sem_op, from_context);
        }
    }

    return signal_state->batch;
}

std::shared_ptr<QueueBatchContext> QueueBatchContext::ResolveOneWaitSemaphore(VkSemaphore sem, VkPipelineStageFlags2 wait_mask,
                                                                              SignaledSemaphores &signaled) {
    auto sem_state = sync_state_->Get<SEMAPHORE_STATE>(sem);
    if (!sem_state) return nullptr;  // Semaphore validity is handled by CoreChecks

    // When signal state goes out of scope, the signal information will be dropped, as Unsignal has released ownership.
    auto signal_state = signaled.Unsignal(sem);
    if (!signal_state) return nullptr;  // Invalid signal, skip it.

    assert(signal_state->batch);

    const SemaphoreScope &signal_scope = signal_state->first_scope;
    const auto queue_flags = queue_state_->GetQueueFlags();
    SemaphoreScope wait_scope{GetQueueId(), SyncExecScope::MakeDst(queue_flags, wait_mask)};

    const AccessContext &from_context = signal_state->batch->access_context_;
    if (signal_state->acquired.image) {
        // Import the *presenting* batch, but replacing presenting with acquired.
        ApplyAcquireNextSemaphoreAction apply_acq(wait_scope, signal_state->acquired.acquire_tag);
        access_context_.ResolveFromContext(apply_acq, from_context, signal_state->acquired.generator);

        // Grab the reset of the presenting QBC, with no effective barrier, won't overwrite the acquire, as the tag is newer
        SyncBarrier noop_barrier;
        const BatchBarrierOp noop_barrier_op(wait_scope.queue, noop_barrier);
        access_context_.ResolveFromContext(noop_barrier_op, from_context);
    } else {
        if (signal_scope.queue == wait_scope.queue) {
            // If signal queue == wait queue, signal is treated as a memory barrier with an access scope equal to the
            // valid accesses for the sync scope.
            SyncBarrier sem_barrier(signal_scope, wait_scope, SyncBarrier::AllAccess());
            const BatchBarrierOp sem_barrier_op(wait_scope.queue, sem_barrier);
            access_context_.ResolveFromContext(sem_barrier_op, from_context);
            events_context_.ApplyBarrier(sem_barrier.src_exec_scope, sem_barrier.dst_exec_scope, ResourceUsageRecord::kMaxIndex);
        } else {
            ApplySemaphoreBarrierAction sem_op(signal_scope, wait_scope);
            access_context_.ResolveFromContext(sem_op, signal_state->batch->access_context_);
        }
    }
    // Cannot move from the signal state because it could be from the const global state, and C++ doesn't
    // enforce deep constness.
    return signal_state->batch;
}

void QueueBatchContext::ImportSyncTags(const QueueBatchContext &from) {
    // NOTE: Assumes that from has set it's tag limit in it's own queue_id slot.
    size_t q_limit = queue_sync_tag_.size();
    assert(q_limit == from.queue_sync_tag_.size());
    for (size_t q = 0; q < q_limit; q++) {
        queue_sync_tag_[q] = std::max(queue_sync_tag_[q], from.queue_sync_tag_[q]);
    }
}

void QueueBatchContext::SetupAccessContext(const std::shared_ptr<const QueueBatchContext> &prev,
                                           const VkPresentInfoKHR &present_info, const PresentedImages &presented_images,
                                           SignaledSemaphores &signaled) {
    ConstBatchSet batches_resolved;
    for (VkSemaphore sem : vvl::make_span(present_info.pWaitSemaphores, present_info.waitSemaphoreCount)) {
        std::shared_ptr<QueueBatchContext> resolved = ResolveOneWaitSemaphore(sem, presented_images, signaled);
        if (resolved) {
            batches_resolved.emplace(std::move(resolved));
        }
    }
    CommonSetupAccessContext(prev, batches_resolved);
}

bool QueueBatchContext::DoQueuePresentValidate(const Location &loc, const PresentedImages &presented_images) {
    bool skip = false;

    HazardDetector detector(SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL);
    // Tag the presented images so record doesn't have to know the tagging scheme
    for (size_t index = 0; index < presented_images.size(); ++index) {
        const PresentedImage &presented = presented_images[index];

        // Need a copy that can be used as the pseudo-iterator...
        HazardResult hazard = access_context_.DetectHazard(detector, presented.range_gen, AccessContext::DetectOptions::kDetectAll);
        if (hazard.IsHazard()) {
            const auto queue_handle = queue_state_->Handle();
            const auto swap_handle = BASE_NODE::Handle(presented.swapchain_state.lock());
            const auto image_handle = BASE_NODE::Handle(presented.image);
            skip = sync_state_->LogError(
                string_SyncHazardVUID(hazard.Hazard()), queue_handle, loc,
                "Hazard %s for present pSwapchains[%" PRIu32 "] , swapchain %s, image index %" PRIu32 " %s, Access info %s.",
                string_SyncHazard(hazard.Hazard()), presented.present_index, sync_state_->FormatHandle(swap_handle).c_str(),
                presented.image_index, sync_state_->FormatHandle(image_handle).c_str(), FormatHazard(hazard).c_str());
            if (skip) break;
        }
    }
    return skip;
}

void QueueBatchContext::DoPresentOperations(const PresentedImages &presented_images) {
    // For present, tagging is internal to the presented image record.
    for (const auto &presented : presented_images) {
        // Update memory state
        presented.UpdateMemoryAccess(SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL, presented.tag, access_context_);
    }
}

void QueueBatchContext::LogPresentOperations(const PresentedImages &presented_images) {
    if (tag_range_.size()) {
        auto access_log = std::make_shared<AccessLog>();
        batch_log_.Insert(batch_, tag_range_, access_log);
        access_log->reserve(tag_range_.size());
        assert(tag_range_.size() == presented_images.size());
        for (const auto &presented : presented_images) {
            access_log->emplace_back(PresentResourceRecord(static_cast<const PresentedImageRecord>(presented)));
        }
    }
}

void QueueBatchContext::DoAcquireOperation(const PresentedImage &presented) {
    // Only one tag for acquire.  The tag in presented is the present tag
    presented.UpdateMemoryAccess(SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_ACQUIRE_READ_SYNCVAL, tag_range_.begin, access_context_);
}

void QueueBatchContext::LogAcquireOperation(const PresentedImage &presented, vvl::Func command) {
    auto access_log = std::make_shared<AccessLog>();
    batch_log_.Insert(batch_, tag_range_, access_log);
    access_log->emplace_back(AcquireResourceRecord(presented, tag_range_.begin, command));
}

void QueueBatchContext::SetupAccessContext(const std::shared_ptr<const QueueBatchContext> &prev, const VkSubmitInfo2 &submit_info,
                                           SignaledSemaphores &signaled) {
    // Import (resolve) the batches that are waited on, with the semaphore's effective barriers applied
    ConstBatchSet batches_resolved;
    const uint32_t wait_count = submit_info.waitSemaphoreInfoCount;
    const VkSemaphoreSubmitInfo *wait_infos = submit_info.pWaitSemaphoreInfos;
    for (const auto &wait_info : vvl::make_span(wait_infos, wait_count)) {
        std::shared_ptr<QueueBatchContext> resolved = ResolveOneWaitSemaphore(wait_info.semaphore, wait_info.stageMask, signaled);
        if (resolved) {
            batches_resolved.emplace(std::move(resolved));
        }
    }
    CommonSetupAccessContext(prev, batches_resolved);
}

void QueueBatchContext::SetupAccessContext(const PresentedImage &presented) {
    if (presented.batch) {
        access_context_.ResolveFromContext(NoopBarrierAction(), presented.batch->access_context_);
        batch_log_.Import(presented.batch->batch_log_);
        ImportSyncTags(*presented.batch);
    }
}

void QueueBatchContext::CommonSetupAccessContext(const std::shared_ptr<const QueueBatchContext> &prev,
                                                 QueueBatchContext::ConstBatchSet &batches_resolved) {
    // Import the previous batch information
    if (prev) {
        // Copy in the event state from the previous batch (on this queue)
        events_context_.DeepCopy(prev->events_context_);
        if (!vvl::Contains(batches_resolved, prev)) {
            // If there are no semaphores to the previous batch, make sure a "submit order" non-barriered import is done
            access_context_.ResolveFromContext(NoopBarrierAction(), prev->access_context_);
            batches_resolved.emplace(prev);
        }
    }

    // Get all the log and tag sync information for the resolved contexts
    for (const auto &batch : batches_resolved) {
        batch_log_.Import(batch->batch_log_);
        ImportSyncTags(*batch);
    }

    // Gather async context information for hazard checks and conserve the QBC's for the async batches
    async_batches_ =
        sync_state_->GetQueueLastBatchSnapshot([&batches_resolved](const std::shared_ptr<const QueueBatchContext> &batch) {
            return !vvl::Contains(batches_resolved, batch);
        });
    for (const auto &async_batch : async_batches_) {
        const QueueId async_queue = async_batch->GetQueueId();
        ResourceUsageTag sync_tag;
        if (async_queue < queue_sync_tag_.size()) {
            sync_tag = queue_sync_tag_[async_queue];
        } else {
            // If this isn't from a tracked queue, just check the batch itself
            sync_tag = async_batch->GetTagRange().begin;
        }

        // The start of the asynchronous access range for a given queue is one more than the highest tagged reference
        access_context_.AddAsyncContext(async_batch->GetCurrentAccessContext(), sync_tag);
        // We need to snapshot the async log information for async hazard reporting
        batch_log_.Import(async_batch->batch_log_);
    }
}

void QueueBatchContext::SetupCommandBufferInfo(const VkSubmitInfo2 &submit_info) {
    // Create the list of command buffers to submit
    const uint32_t cb_count = submit_info.commandBufferInfoCount;
    const VkCommandBufferSubmitInfo *const cb_infos = submit_info.pCommandBufferInfos;
    command_buffers_.reserve(cb_count);

    for (const auto &cb_info : vvl::make_span(cb_infos, cb_count)) {
        auto cb_state = sync_state_->Get<syncval_state::CommandBuffer>(cb_info.commandBuffer);
        if (cb_state) {
            tag_range_.end += cb_state->access_context.GetTagLimit();
            command_buffers_.emplace_back(static_cast<uint32_t>(&cb_info - cb_infos), std::move(cb_state));
        }
    }
}

// Look up the usage informaiton from the local or global logger
std::string QueueBatchContext::FormatUsage(ResourceUsageTag tag) const {
    std::stringstream out;
    BatchAccessLog::AccessRecord access = batch_log_[tag];
    if (access.IsValid()) {
        const BatchAccessLog::BatchRecord &batch = *access.batch;
        const ResourceUsageRecord &record = *access.record;
        if (batch.queue) {
            // Queue and Batch information (for enqueued operations)
            out << SyncNodeFormatter(*sync_state_, batch.queue->GetQueueState());
            out << ", submit: " << batch.submit_index << ", batch: " << batch.batch_index;
        }
        out << ", batch_tag: " << batch.bias;

        // Commandbuffer Usages Information
        out << ", " << record.Formatter(*sync_state_, nullptr);
    }
    return out.str();
}

VkQueueFlags QueueBatchContext::GetQueueFlags() const { return queue_state_->GetQueueFlags(); }

QueueId QueueBatchContext::GetQueueId() const {
    QueueId id = queue_state_ ? queue_state_->GetQueueId() : QueueSyncState::kQueueIdInvalid;
    return id;
}

// For QueuePresent, the tag range is defined externally and must be passed in
void QueueBatchContext::SetupBatchTags(const ResourceUsageRange &tag_range) {
    tag_range_ = tag_range;
    SetupBatchTags();
}

// For QueueSubmit, the tag range is defined by the CommandBuffer setup.
// For QueuePresent, this is called when the tag_range is specified
void QueueBatchContext::SetupBatchTags() {
    // Need new global tags for all accesses... the Reserve updates a mutable atomic
    ResourceUsageRange global_tags = sync_state_->ReserveGlobalTagRange(GetTagRange().size());
    SetTagBias(global_tags.begin);
}

void QueueBatchContext::InsertRecordedAccessLogEntries(const CommandBufferAccessContext &submitted_cb) {
    const ResourceUsageTag end_tag = batch_log_.Import(batch_, submitted_cb);
    batch_.bias = end_tag;
    batch_.cb_index++;
}

void QueueBatchContext::SetTagBias(ResourceUsageTag bias) {
    const auto size = tag_range_.size();
    tag_range_.begin = bias;
    tag_range_.end = bias + size;
    access_context_.SetStartTag(bias);
    batch_.bias = bias;

    // Needed for ImportSyncTags to pick up the "from" own sync tag.
    const QueueId this_q = GetQueueId();
    if (this_q < queue_sync_tag_.size()) {
        // If this is a non-queued operation we'll get a "special" value like invalid
        queue_sync_tag_[this_q] = tag_range_.end;
    }
}

// Since we're updating the QueueSync state, this is Record phase and the access log needs to point to the global one
// Batch Contexts saved during signalling have their AccessLog reset when the pending signals are signalled.
// NOTE: By design, QueueBatchContexts that are neither last, nor referenced by a signal are abandoned as unowned, since
//       the contexts Resolve all history from previous all contexts when created
void QueueSyncState::UpdateLastBatch(std::shared_ptr<QueueBatchContext> &&new_last) {
    // Update the queue to point to the last batch from the submit
    if (new_last) {
        // Clean up the events data in the previous last batch on queue, as only the subsequent batches have valid use for them
        // and the QueueBatchContext::Setup calls have be copying them along from batch to batch during submit.
        if (last_batch_) {
            last_batch_->ResetEventsContext();
        }
        new_last->Trim();
        last_batch_ = std::move(new_last);
    }
}

// Note that function is const, but updates mutable submit_index to allow Validate to create correct tagging for command invocation
// scope state.
// Given that queue submits are supposed to be externally synchronized for the same queue, this should safe without being
// atomic... but as the ops are per submit, the performance cost is negible for the peace of mind.
uint64_t QueueSyncState::ReserveSubmitId() const { return submit_index_.fetch_add(1); }

// This is a const method, force the returned value to be const
std::shared_ptr<const SignaledSemaphores::Signal> SignaledSemaphores::GetPrev(VkSemaphore sem) const {
    std::shared_ptr<Signal> prev_state;
    if (prev_) {
        prev_state = GetMapped(prev_->signaled_, sem, [&prev_state]() { return prev_state; });
    }
    return prev_state;
}

SignaledSemaphores::Signal::Signal(const std::shared_ptr<const SEMAPHORE_STATE> &sem_state_,
                                   const std::shared_ptr<QueueBatchContext> &batch_, const SyncExecScope &exec_scope_)
    : sem_state(sem_state_), batch(batch_), first_scope({batch->GetQueueId(), exec_scope_}) {
    // Illegal to create a signal from no batch or an invalid semaphore... caller must assure validity
    assert(batch);
    assert(sem_state);
}

SignaledSemaphores::Signal::Signal(const std::shared_ptr<const SEMAPHORE_STATE> &sem_state_, const PresentedImage &presented,
                                   ResourceUsageTag acq_tag)
    : sem_state(sem_state_), batch(presented.batch), first_scope(), acquired(presented, acq_tag) {
    // Illegal to create a signal from no batch or an invalid semaphore... caller must assure validity
    assert(batch);
    assert(sem_state);
}

FenceSyncState::FenceSyncState() : fence(), tag(kInvalidTag), queue_id(QueueSyncState::kQueueIdInvalid) {}

VkSemaphoreSubmitInfo SubmitInfoConverter::BatchStore::WaitSemaphore(const VkSubmitInfo &info, uint32_t index) {
    VkSemaphoreSubmitInfo semaphore_info = vku::InitStructHelper();
    semaphore_info.semaphore = info.pWaitSemaphores[index];
    semaphore_info.stageMask = info.pWaitDstStageMask[index];
    return semaphore_info;
}
VkCommandBufferSubmitInfo SubmitInfoConverter::BatchStore::CommandBuffer(const VkSubmitInfo &info, uint32_t index) {
    VkCommandBufferSubmitInfo cb_info = vku::InitStructHelper();
    cb_info.commandBuffer = info.pCommandBuffers[index];
    return cb_info;
}

VkSemaphoreSubmitInfo SubmitInfoConverter::BatchStore::SignalSemaphore(const VkSubmitInfo &info, uint32_t index,
                                                                       VkQueueFlags queue_flags) {
    VkSemaphoreSubmitInfo semaphore_info = vku::InitStructHelper();
    semaphore_info.semaphore = info.pSignalSemaphores[index];
    // Can't just use BOTTOM, because of how access expansion is done
    semaphore_info.stageMask =
        sync_utils::ExpandPipelineStages(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, queue_flags, VK_PIPELINE_STAGE_2_HOST_BIT);
    return semaphore_info;
}

SubmitInfoConverter::BatchStore::BatchStore(const VkSubmitInfo &info, VkQueueFlags queue_flags) {
    info2 = vku::InitStructHelper();

    info2.waitSemaphoreInfoCount = info.waitSemaphoreCount;
    waits.reserve(info2.waitSemaphoreInfoCount);
    for (uint32_t i = 0; i < info2.waitSemaphoreInfoCount; ++i) {
        waits.emplace_back(WaitSemaphore(info, i));
    }
    info2.pWaitSemaphoreInfos = waits.data();

    info2.commandBufferInfoCount = info.commandBufferCount;
    cbs.reserve(info2.commandBufferInfoCount);
    for (uint32_t i = 0; i < info2.commandBufferInfoCount; ++i) {
        cbs.emplace_back(CommandBuffer(info, i));
    }
    info2.pCommandBufferInfos = cbs.data();

    info2.signalSemaphoreInfoCount = info.signalSemaphoreCount;
    signals.reserve(info2.signalSemaphoreInfoCount);
    for (uint32_t i = 0; i < info2.signalSemaphoreInfoCount; ++i) {
        signals.emplace_back(SignalSemaphore(info, i, queue_flags));
    }
    info2.pSignalSemaphoreInfos = signals.data();
}

SubmitInfoConverter::SubmitInfoConverter(uint32_t count, const VkSubmitInfo *infos, VkQueueFlags queue_flags) {
    info_store.reserve(count);
    info2s.reserve(count);
    for (uint32_t batch = 0; batch < count; ++batch) {
        info_store.emplace_back(infos[batch], queue_flags);
        info2s.emplace_back(info_store.back().info2);
    }
}

ResourceUsageTag BatchAccessLog::Import(const BatchRecord &batch, const CommandBufferAccessContext &cb_access) {
    ResourceUsageTag bias = batch.bias;
    ResourceUsageTag tag_limit = bias + cb_access.GetTagLimit();
    ResourceUsageRange import_range = {bias, tag_limit};
    log_map_.insert(std::make_pair(import_range, CBSubmitLog(batch, cb_access)));
    return tag_limit;
}

void BatchAccessLog::Import(const BatchAccessLog &other) {
    for (const auto &entry : other.log_map_) {
        log_map_.insert(entry);
    }
}

void BatchAccessLog::Insert(const BatchRecord &batch, const ResourceUsageRange &range,
                            std::shared_ptr<const CommandExecutionContext::AccessLog> log) {
    log_map_.insert(std::make_pair(range, CBSubmitLog(batch, nullptr, std::move(log))));
}

// Trim: Remove any unreferenced AccessLog ranges from a BatchAccessLog
//
// In order to contain memory growth in the AccessLog information regarding prior submitted command buffers,
// the Trim call removes any AccessLog references that do not correspond to any tags in use. The set of referenced tag, used_tags,
// is generated by scanning the AccessContext and EventContext of the containing QueueBatchContext.
//
// Upon return the BatchAccessLog should only contain references to the AccessLog information needed by the
// containing parent QueueBatchContext.
//
// The algorithm used is another example of the "parallel iteration" pattern common within SyncVal.  In this case we are
// traversing the ordered range_map containing the AccessLog references and the ordered set of tags in use.
//
// To efficiently perform the parallel iteration, optimizations within this function include:
//  * when ranges are detected that have no tags referenced, all ranges between the last tag and the current tag are erased
//  * when used tags prior to the current range are found, all tags up to the current range are skipped
//  * when a tag is found within the current range, that range is skipped (and thus kept in the map), and further used tags
//    within the range are skipped.
//
// Note that for each subcase, any "next steps" logic is designed to be handled within the subsequent iteration -- meaning that
// each subcase simply handles the specifics of the current update/skip/erase action needed, and leaves the iterators in a sensible
// state for the top of loop... intentionally eliding special case handling.
void BatchAccessLog::Trim(const ResourceUsageTagSet &used_tags) {
    auto current_tag = used_tags.cbegin();
    const auto end_tag = used_tags.cend();
    auto current_map_range = log_map_.begin();
    const auto end_map = log_map_.end();

    while (current_map_range != end_map) {
        if (current_tag == end_tag) {
            // We're out of tags, the rest of the map isn't referenced, so erase it
            current_map_range = log_map_.erase(current_map_range, end_map);
        } else {
            auto &range = current_map_range->first;
            const ResourceUsageTag tag = *current_tag;
            if (tag < range.begin) {
                // Skip to the next tag potentially in range
                // if this is end_tag, we'll handle that next iteration
                current_tag = used_tags.lower_bound(range.begin);
            } else if (tag >= range.end) {
                // This tag is beyond the current range, delete all ranges between current_map_range,
                // and the next that includes the tag.  Next is not erased.
                auto next_used = log_map_.lower_bound(ResourceUsageRange(tag, tag + 1));
                current_map_range = log_map_.erase(current_map_range, next_used);
            } else {
                // Skip the rest of the tags in this range
                // If this is end, the next iteration will handle
                current_tag = used_tags.lower_bound(range.end);

                // This is a range we will keep, advance to the next. Next iteration handles end condition
                ++current_map_range;
            }
        }
    }
}

BatchAccessLog::AccessRecord BatchAccessLog::operator[](ResourceUsageTag tag) const {
    auto found_log = log_map_.find(tag);
    if (found_log != log_map_.cend()) {
        return found_log->second[tag];
    }
    // tag not found
    assert(false);
    return AccessRecord();
}

BatchAccessLog::AccessRecord BatchAccessLog::CBSubmitLog::operator[](ResourceUsageTag tag) const {
    assert(tag >= batch_.bias);
    const size_t index = tag - batch_.bias;
    assert(log_);
    assert(index < log_->size());
    return AccessRecord{&batch_, &(*log_)[index]};
}

PresentedImage::PresentedImage(const SyncValidator &sync_state, const std::shared_ptr<QueueBatchContext> batch_,
                               VkSwapchainKHR swapchain, uint32_t image_index_, uint32_t present_index_, ResourceUsageTag tag_)
    : PresentedImageRecord{tag_, image_index_, present_index_, sync_state.Get<syncval_state::Swapchain>(swapchain), {}},
      batch(std::move(batch_)) {
    SetImage(image_index_);
}

PresentedImage::PresentedImage(std::shared_ptr<const syncval_state::Swapchain> swapchain, uint32_t at_index) : PresentedImage() {
    swapchain_state = std::move(swapchain);
    tag = kInvalidTag;
    SetImage(at_index);
}

// Export uses move semantics...
void PresentedImage::ExportToSwapchain(SyncValidator &) {  // Include this argument to prove the const cast is safe
    // If the swapchain is dead just ignore the present
    auto swap_lock = swapchain_state.lock();
    if (BASE_NODE::Invalid(swap_lock)) return;
    auto swap = std::const_pointer_cast<syncval_state::Swapchain>(swap_lock);
    swap->RecordPresentedImage(std::move(*this));
}

void PresentedImage::SetImage(uint32_t at_index) {
    image_index = at_index;

    auto swap_lock = swapchain_state.lock();
    if (BASE_NODE::Invalid(swap_lock)) return;
    image = std::static_pointer_cast<const syncval_state::ImageState>(swap_lock->GetSwapChainImageShared(image_index));
    if (Invalid()) {
        range_gen = ImageRangeGen();
    } else {
        // For valid images create the type/range_gen to used to scope the semaphore operations
        range_gen = image->MakeImageRangeGen(image->full_range, false);
    }
}

void PresentedImage::UpdateMemoryAccess(SyncStageAccessIndex usage, ResourceUsageTag tag, AccessContext &access_context) const {
    // Intentional copy. The range_gen argument is not copied by the Update... call below
    subresource_adapter::ImageRangeGenerator generator = range_gen;
    UpdateMemoryAccessStateFunctor action(access_context, usage, SyncOrdering::kNonAttachment, tag);
    UpdateMemoryAccessState(access_context.GetAccessStateMap(), action, generator);
}

QueueBatchContext::PresentResourceRecord::Base_::Record QueueBatchContext::PresentResourceRecord::MakeRecord() const {
    return std::make_unique<PresentResourceRecord>(presented_);
}

std::ostream &QueueBatchContext::PresentResourceRecord::Format(std::ostream &out, const SyncValidator &sync_state) const {
    out << "vkQueuePresentKHR ";
    out << "present_tag:" << presented_.tag;
    out << ", pSwapchains[" << presented_.present_index << "]";
    out << ": " << SyncNodeFormatter(sync_state, presented_.swapchain_state.lock().get());
    out << ", image_index: " << presented_.image_index;
    out << SyncNodeFormatter(sync_state, presented_.image.get());

    return out;
}

QueueBatchContext::AcquireResourceRecord::Base_::Record QueueBatchContext::AcquireResourceRecord::MakeRecord() const {
    return std::make_unique<AcquireResourceRecord>(presented_, acquire_tag_, command_);
}

std::ostream &QueueBatchContext::AcquireResourceRecord::Format(std::ostream &out, const SyncValidator &sync_state) const {
    out << vvl::String(command_) << " ";
    out << "aquire_tag:" << acquire_tag_;
    out << ": " << SyncNodeFormatter(sync_state, presented_.swapchain_state.lock().get());
    out << ", image_index: " << presented_.image_index;
    out << SyncNodeFormatter(sync_state, presented_.image.get());

    return out;
}

syncval_state::Swapchain::Swapchain(ValidationStateTracker *dev_data, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                    VkSwapchainKHR swapchain)
    : SWAPCHAIN_NODE(dev_data, pCreateInfo, swapchain) {}

void syncval_state::Swapchain::RecordPresentedImage(PresentedImage &&presented_image) {
    // All presented images are stored within the swapchain until the are reaquired.
    const uint32_t image_index = presented_image.image_index;
    if (image_index >= presented.size()) presented.resize(image_index + 1);

    // Use move semantics to avoid atomic operations on the contained shared_ptrs
    presented[image_index] = std::move(presented_image);
}

// We move from the presented images array 1) so we don't copy shared_ptr, and 2) to mark it acquired
PresentedImage syncval_state::Swapchain::MovePresentedImage(uint32_t image_index) {
    if (presented.size() <= image_index) presented.resize(image_index + 1);
    PresentedImage ret_val = std::move(presented[image_index]);
    if (ret_val.Invalid()) {
        // If this is the first time the image has been acquired, then it's valid to have no present record, so we create one
        // Note: It's also possible this is an invalid acquire... but that's CoreChecks/Parameter validation's job to report
        ret_val = PresentedImage(static_cast<const syncval_state::Swapchain *>(this)->shared_from_this(), image_index);
    }
    return ret_val;
}

AcquiredImage::AcquiredImage(const PresentedImage &presented, ResourceUsageTag acq_tag)
    : image(presented.image),
      generator(presented.range_gen),
      present_tag(presented.tag),
      acquire_tag(acq_tag) {}

FenceSyncState::FenceSyncState(const std::shared_ptr<const FENCE_STATE> &fence_, QueueId queue_id_, ResourceUsageTag tag_)
    : fence(fence_), tag(tag_), queue_id(queue_id_) {}
FenceSyncState::FenceSyncState(const std::shared_ptr<const FENCE_STATE> &fence_, const PresentedImage &image, ResourceUsageTag tag_)
    : fence(fence_), tag(tag_), queue_id(QueueSyncState::kQueueIdInvalid), acquired(image, tag) {}

// For RenderPass time validation this is "start tag", for QueueSubmit, this is the earliest
// unsynchronized tag for the Queue being tested against (max synchrononous + 1, perhaps)

ResourceUsageTag AccessContext::AsyncReference::StartTag() const { return (tag_ == kInvalidTag) ? context_->StartTag() : tag_; }

bool syncval_state::ImageState::IsSimplyBound() const {
    bool simple = SimpleBinding(static_cast<const BINDABLE &>(*this)) || IsSwapchainImage() || bind_swapchain;

    // If it's not simple we must have an encoder.
    assert(!simple || fragment_encoder.get());

    return simple;
}

void syncval_state::ImageState::SetOpaqueBaseAddress(ValidationStateTracker &dev_data) {
    // This is safe to call if already called to simplify caller logic
    // NOTE: Not asserting IsTiled, as there could in future be other reasons for opaque representations
    if (opaque_base_address_) return;

    VkDeviceSize opaque_base = 0U;  // Fakespace Allocator starts > 0
    auto get_opaque_base = [&opaque_base](const IMAGE_STATE &other) {
        const ImageState &other_sync = static_cast<const ImageState &>(other);
        opaque_base = other_sync.opaque_base_address_;
        return true;
    };
    if (IsSwapchainImage()) {
        AnyAliasBindingOf(bind_swapchain->ObjectBindings(), get_opaque_base);
    } else {
        AnyImageAliasOf(get_opaque_base);
    }
    if (!opaque_base) {
        // The size of the opaque range is based on the SyncVal *internal* representation of the tiled resource, unrelated
        // to the acutal size of the the resource in device memory. If differing representations become possible, the allocated
        // size would need to be changed to those representation's size requirements.
        opaque_base = dev_data.AllocFakeMemory(fragment_encoder->TotalSize());
    }
    opaque_base_address_ = opaque_base;
}

VkDeviceSize syncval_state::ImageState::GetResourceBaseAddress() const {
    if (HasOpaqueMapping()) {
        return GetOpaqueBaseAddress();
    }
    return GetFakeBaseAddress();
}

ImageRangeGen syncval_state::ImageState::MakeImageRangeGen(const VkImageSubresourceRange &subresource_range,
                                                           bool is_depth_sliced) const {
    if (!fragment_encoder || !IsSimplyBound()) {
        return ImageRangeGen();  // default range generators have an empty position (generator "end")
    }

    const auto base_address = GetResourceBaseAddress();
    ImageRangeGen range_gen(*fragment_encoder.get(), subresource_range, base_address, is_depth_sliced);
    return range_gen;
}

ImageRangeGen syncval_state::ImageState::MakeImageRangeGen(const VkImageSubresourceRange &subresource_range,
                                                           const VkOffset3D &offset, const VkExtent3D &extent,
                                                           bool is_depth_sliced) const {
    if (!fragment_encoder || !IsSimplyBound()) {
        return ImageRangeGen();  // default range generators have an empty position (generator "end")
    }

    const auto base_address = GetResourceBaseAddress();
    subresource_adapter::ImageRangeGenerator range_gen(*fragment_encoder.get(), subresource_range, offset, extent, base_address,
                                                       is_depth_sliced);
    return range_gen;
}

ReplayState::ReplayState(CommandExecutionContext &exec_context, const CommandBufferAccessContext &recorded_context,
                         const ErrorObject &error_obj, uint32_t index)
    : exec_context_(exec_context),
      recorded_context_(recorded_context),
      error_obj_(error_obj),
      index_(index),
      base_tag_(exec_context.GetTagLimit()) {}

const AccessContext *ReplayState::GetRecordedAccessContext() const {
    if (rp_replay_) {
        return rp_replay_.replay_context;
    }
    return recorded_context_.GetCurrentAccessContext();
}

// intiially zero, but accumulating the dstStages of barriers if they chain.

ResourceAccessWriteState::ResourceAccessWriteState(const SyncStageAccessInfoType &usage_info, ResourceUsageTag tag)
    : access_(&usage_info),
      barriers_(),
      tag_(tag),
      queue_(QueueSyncState::kQueueIdInvalid),
      dependency_chain_(VK_PIPELINE_STAGE_2_NONE_KHR),
      pending_layout_ordering_(),
      pending_dep_chain_(VK_PIPELINE_STAGE_2_NONE),
      pending_barriers_() {}

bool ResourceAccessWriteState::IsWriteHazard(const SyncStageAccessInfoType &usage_info) const {
    return !barriers_[usage_info.stage_access_index];
}

bool ResourceAccessWriteState::IsOrdered(const OrderingBarrier &ordering, QueueId queue_id) const {
    assert(access_);
    return (queue_ == queue_id) && ordering.access_scope[access_->stage_access_index];
}

bool ResourceAccessWriteState::IsWriteBarrierHazard(QueueId queue_id, VkPipelineStageFlags2KHR src_exec_scope,
                                                    const SyncStageAccessFlags &src_access_scope) const {
    // Special rules for sequential ILT's
    if (IsIndex(SYNC_IMAGE_LAYOUT_TRANSITION)) {
        if (queue_id == queue_) {
            // In queue, they are implicitly ordered
            return false;
        } else {
            // In dep chain means that the ILT is *available*
            return !WriteInChain(src_exec_scope);
        }
    }
    // In dep chain means that the write is *available*.
    // Available writes are automatically made visible and can't cause hazards during transition.
    if (WriteInChain(src_exec_scope)) {
        return false;
    }
    // The write is not in chain (previous call), so need only to check if the write is in access scope.
    return !WriteInScope(src_access_scope);
}

void ResourceAccessWriteState::Set(const SyncStageAccessInfoType &usage_info, ResourceUsageTag tag) {
    access_ = &usage_info;
    barriers_.reset();
    dependency_chain_ = VK_PIPELINE_STAGE_2_NONE;
    tag_ = tag;
    queue_ = QueueSyncState::kQueueIdInvalid;
}

void ResourceAccessWriteState::MergeBarriers(const ResourceAccessWriteState &other) {
    barriers_ |= other.barriers_;
    dependency_chain_ |= other.dependency_chain_;

    pending_barriers_ |= other.pending_barriers_;
    pending_dep_chain_ |= other.pending_dep_chain_;
    pending_layout_ordering_ |= other.pending_layout_ordering_;
}

void ResourceAccessWriteState::UpdatePendingBarriers(const SyncBarrier &barrier) {
    pending_barriers_ |= barrier.dst_access_scope;
    pending_dep_chain_ |= barrier.dst_exec_scope.exec_scope;
}

void ResourceAccessWriteState::ApplyPendingBarriers() {
    dependency_chain_ |= pending_dep_chain_;
    barriers_ |= pending_barriers_;
}

void ResourceAccessWriteState::UpdatePendingLayoutOrdering(const SyncBarrier &barrier) {
    pending_layout_ordering_ |= OrderingBarrier(barrier.src_exec_scope.exec_scope, barrier.src_access_scope);
}

void ResourceAccessWriteState::ClearPending() {
    pending_dep_chain_ = VK_PIPELINE_STAGE_2_NONE;
    pending_barriers_.reset();
    pending_layout_ordering_ = OrderingBarrier();
}

void ResourceAccessWriteState::SetQueueId(QueueId id) {
    if (queue_ == QueueSyncState::kQueueIdInvalid) {
        queue_ = id;
    }
}

bool ResourceAccessWriteState::WriteInChain(VkPipelineStageFlags2KHR src_exec_scope) const {
    return 0 != (dependency_chain_ & src_exec_scope);
}

bool ResourceAccessWriteState::WriteInScope(const SyncStageAccessFlags &src_access_scope) const {
    assert(access_);
    return src_access_scope[access_->stage_access_index];
}

bool ResourceAccessWriteState::WriteInSourceScopeOrChain(VkPipelineStageFlags2KHR src_exec_scope,
                                                         SyncStageAccessFlags src_access_scope) const {
    assert(access_);
    return WriteInChain(src_exec_scope) || WriteInScope(src_access_scope);
}

bool ResourceAccessWriteState::WriteInQueueSourceScopeOrChain(QueueId queue, VkPipelineStageFlags2KHR src_exec_scope,
                                                              const SyncStageAccessFlags &src_access_scope) const {
    assert(access_);
    return WriteInChain(src_exec_scope) || ((queue == queue_) && WriteInScope(src_access_scope));
}

bool ResourceAccessWriteState::WriteInEventScope(VkPipelineStageFlags2KHR src_exec_scope,
                                                 const SyncStageAccessFlags &src_access_scope, QueueId scope_queue,
                                                 ResourceUsageTag scope_tag) const {
    // The scope logic for events is, if we're asking, the resource usage was flagged as "in the first execution scope" at
    // the time of the SetEvent, thus all we need check is whether the access is the same one (i.e. before the scope tag
    // in order to know if it's in the excecution scope
    assert(access_);
    return (tag_ < scope_tag) && WriteInQueueSourceScopeOrChain(scope_queue, src_exec_scope, src_access_scope);
}

HazardResult::HazardState::HazardState(const ResourceAccessState *access_state_, const SyncStageAccessInfoType &usage_info_,
                                       SyncHazard hazard_, const SyncStageAccessFlags &prior_, ResourceUsageTag tag_)
    : access_state(std::make_unique<const ResourceAccessState>(*access_state_)),
      recorded_access(),
      usage_index(usage_info_.stage_access_index),
      prior_access(prior_),
      tag(tag_),
      hazard(hazard_) {
    // Touchup the hazard to reflect "present as release" semantics
    // NOTE: For implementing QFO release/acquire semantics... touch up here as well
    if (access_state->IsLastWriteOp(SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL)) {
        if (hazard == SyncHazard::READ_AFTER_WRITE) {
            hazard = SyncHazard::READ_AFTER_PRESENT;
        } else if (hazard == SyncHazard::WRITE_AFTER_WRITE) {
            hazard = SyncHazard::WRITE_AFTER_PRESENT;
        }
    } else if (usage_index == SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL) {
        if (hazard == SyncHazard::WRITE_AFTER_READ) {
            hazard = SyncHazard::PRESENT_AFTER_READ;
        } else if (hazard == SyncHazard::WRITE_AFTER_WRITE) {
            hazard = SyncHazard::PRESENT_AFTER_WRITE;
        }
    }
}

syncval_state::ImageViewState::ImageViewState(const std::shared_ptr<IMAGE_STATE> &image_state, VkImageView iv,
                                              const VkImageViewCreateInfo *ci, VkFormatFeatureFlags2KHR ff,
                                              const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props)
    : IMAGE_VIEW_STATE(image_state, iv, ci, ff, cubic_props), view_range_gen(MakeImageRangeGen()) {}

ImageRangeGen syncval_state::ImageViewState::MakeImageRangeGen() const {
    return GetImageState()->MakeImageRangeGen(normalized_subresource_range, IsDepthSliced());
}

ImageRangeGen syncval_state::ImageViewState::MakeImageRangeGen(const VkOffset3D &offset, const VkExtent3D &extent,
                                                               const VkImageAspectFlags aspect_mask) const {
    if (Invalid()) ImageRangeGen();

    // Intentional copy
    VkImageSubresourceRange subresource_range = normalized_subresource_range;
    if (aspect_mask) {
        subresource_range.aspectMask &= aspect_mask;
        assert(subresource_range.aspectMask);
    }

    return GetImageState()->MakeImageRangeGen(subresource_range, offset, extent, IsDepthSliced());
}
