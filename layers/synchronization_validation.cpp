/* Copyright (c) 2019-2022 The Khronos Group Inc.
 * Copyright (c) 2019-2022 Valve Corporation
 * Copyright (c) 2019-2022 LunarG, Inc.
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
 * Author: Locke Lin <locke@lunarg.com>
 * Author: Jeremy Gebben <jeremyg@lunarg.com>
 */

#include <limits>
#include <vector>
#include <memory>
#include <bitset>
#include "synchronization_validation.h"
#include "sync_utils.h"

// Utilities to DRY up Get... calls
template <typename Map, typename Key = typename Map::key_type, typename RetVal = layer_data::optional<typename Map::mapped_type>>
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

template <typename Map, typename Fn>
typename Map::mapped_type GetMappedInsert(Map &map, const typename Map::key_type &key, Fn &&emplace_factory) {
    auto value = GetMappedOptional(map, key);
    if (value) {
        return *value;
    }
    auto insert_it = map.emplace(std::make_pair(key, emplace_factory()));
    assert(insert_it.second);

    return insert_it.first->second;
}

template <typename Map, typename Key = typename Map::key_type, typename Mapped = typename Map::mapped_type,
          typename Value = typename Mapped::element_type>
Value *GetMappedPlainFromShared(const Map &map, const Key &key) {
    auto value = GetMappedOptional<Map, Key>(map, key);
    if (value) return value->get();
    return nullptr;
}

static bool SimpleBinding(const BINDABLE &bindable) { return !bindable.sparse && bindable.Binding(); }

static bool SimpleBinding(const IMAGE_STATE &image_state) {
    bool simple =
        SimpleBinding(static_cast<const BINDABLE &>(image_state)) || image_state.IsSwapchainImage() || image_state.bind_swapchain;

    // If it's not simple we must have an encoder.
    assert(!simple || image_state.fragment_encoder.get());
    return simple;
}

static const ResourceAccessRange kFullRange(std::numeric_limits<VkDeviceSize>::min(), std::numeric_limits<VkDeviceSize>::max());
static const std::array<AccessAddressType, static_cast<size_t>(AccessAddressType::kTypeCount)> kAddressTypes = {
    AccessAddressType::kLinear, AccessAddressType::kIdealized};

static constexpr AccessAddressType GetAccessAddressType(const BUFFER_STATE &) { return AccessAddressType::kLinear; };
static AccessAddressType GetAccessAddressType(const IMAGE_STATE &image) {
    return SimpleBinding(image) ? AccessContext::ImageAddressType(image) : AccessAddressType::kIdealized;
}

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
    for (size_t i = 0; i < flags.size(); i++) {
        if (flags.test(i)) {
            info = &syncStageAccessInfoByStageAccessIndex[i];
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
        for (size_t i = 0; i < syncStageAccessInfoByStageAccessIndex.size(); i++) {
            const auto &info = syncStageAccessInfoByStageAccessIndex[i];
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

std::ostream &operator<<(std::ostream &out, const ResourceUsageRecord &record) {
    out << "command: " << CommandTypeString(record.command);
    out << ", seq_no: " << record.seq_num;
    if (record.sub_command != 0) {
        out << ", subcmd: " << record.sub_command;
    }
    return out;
}

static std::string string_UsageIndex(SyncStageAccessIndex usage_index) {
    const char *stage_access_name = "INVALID_STAGE_ACCESS";
    if (usage_index < static_cast<SyncStageAccessIndex>(syncStageAccessInfoByStageAccessIndex.size())) {
        stage_access_name = syncStageAccessInfoByStageAccessIndex[usage_index].name;
    }
    return std::string(stage_access_name);
}

struct SyncNodeFormatter {
    const debug_report_data *report_data;
    const BASE_NODE *node;
    const char *label;

    SyncNodeFormatter(const SyncValidator &sync_state, const CMD_BUFFER_STATE *cb_state)
        : report_data(sync_state.report_data), node(cb_state), label("command_buffer") {}
    SyncNodeFormatter(const SyncValidator &sync_state, const QUEUE_STATE *q_state)
        : report_data(sync_state.report_data), node(q_state), label("queue") {}
};

std::ostream &operator<<(std::ostream &out, const SyncNodeFormatter &formater) {
    if (formater.node) {
        out << ", " << formater.label << ": " << formater.report_data->FormatHandle(formater.node->Handle()).c_str();
        if (formater.node->Destroyed()) {
            out << " (destroyed)";
        }
    } else {
        out << ", " << formater.label << ": null handle";
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, const HazardResult &hazard) {
    assert(hazard.usage_index < static_cast<SyncStageAccessIndex>(syncStageAccessInfoByStageAccessIndex.size()));
    const auto &usage_info = syncStageAccessInfoByStageAccessIndex[hazard.usage_index];
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
        out << ", read_barriers: " << string_VkPipelineStageFlags2KHR(barriers);
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

// NOTE: Make sure the proxy doesn't outlive from, as the proxy is pointing directly to access contexts owned by from.
CommandBufferAccessContext::CommandBufferAccessContext(const CommandBufferAccessContext &from, AsProxyContext dummy)
    : CommandBufferAccessContext(from.sync_state_) {
    // Copy only the needed fields out of from for a temporary, proxy command buffer context
    cb_state_ = from.cb_state_;
    queue_flags_ = from.queue_flags_;
    destroyed_ = from.destroyed_;
    access_log_ = from.access_log_;  // potentially large, but no choice given tagging lookup.
    command_number_ = from.command_number_;
    subcommand_number_ = from.subcommand_number_;
    reset_count_ = from.reset_count_;

    const auto *from_context = from.GetCurrentAccessContext();
    assert(from_context);

    // Construct a fully resolved single access context out of from
    const NoopBarrierAction noop_barrier;
    for (AccessAddressType address_type : kAddressTypes) {
        from_context->ResolveAccessRange(address_type, kFullRange, noop_barrier,
                                         &cb_access_context_.GetAccessStateMap(address_type), nullptr);
    }
    // The proxy has flatten the current render pass context (if any), but the async contexts are needed for hazard detection
    cb_access_context_.ImportAsyncContexts(*from_context);

    events_context_ = from.events_context_;

    // We don't want to copy the full render_pass_context_ history just for the proxy.
}

std::string CommandBufferAccessContext::FormatUsage(const ResourceUsageTag tag) const {
    if (tag >= access_log_.size()) return std::string();

    std::stringstream out;
    assert(tag < access_log_.size());
    const auto &record = access_log_[tag];
    out << record;
    if (cb_state_.get() != record.cb_state) {
        out << SyncNodeFormatter(*sync_state_, record.cb_state);
    }
    out << ", reset_no: " << std::to_string(record.reset_count);
    return out.str();
}

std::string CommandBufferAccessContext::FormatUsage(const ResourceFirstAccess &access) const {
    std::stringstream out;
    out << "(recorded_usage: " << string_UsageIndex(access.usage_index);
    out << ", " << FormatUsage(access.tag) << ")";
    return out.str();
}

std::string CommandExecutionContext::FormatHazard(const HazardResult &hazard) const {
    std::stringstream out;
    out << hazard;
    out << ", " << FormatUsage(hazard.tag) << ")";
    return out.str();
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

static VkDeviceSize ResourceBaseAddress(const BINDABLE &bindable) { return bindable.GetFakeBaseAddress(); }

VkDeviceSize GetRealWholeSize(VkDeviceSize offset, VkDeviceSize size, VkDeviceSize whole_size) {
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
using SingleAccessRangeGenerator = SingleRangeGenerator<ResourceAccessRange>;
using EventSimpleRangeGenerator = MapRangesRangeGenerator<SyncEventState::ScopeMap>;

// Generate the ranges for entries meeting the predicate that are the intersection of range and the entries in the RangeMap
template <typename RangeMap, typename Predicate, typename KeyType = typename RangeMap::key_type>
class PredicatedMapRangesRangeGenerator : public MapRangesRangeGenerator<RangeMap, KeyType> {
  public:
    using Base = MapRangesRangeGenerator<RangeMap, KeyType>;
    // Default constructed is safe to dereference for "empty" test, but for no other operation.
    PredicatedMapRangesRangeGenerator() : Base(), pred_() {}
    PredicatedMapRangesRangeGenerator(const RangeMap &filter, const KeyType &range, Predicate pred)
        : Base(filter, range), pred_(pred) {}
    PredicatedMapRangesRangeGenerator(const PredicatedMapRangesRangeGenerator &from) = default;

    PredicatedMapRangesRangeGenerator &operator++() {
        Base::PredicatedIncrement(pred_);
        return *this;
    }

  protected:
    Predicate pred_;
};

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
        return stage_access->second.storage_write;
    }
    // TODO: sampled_read
    return stage_access->second.storage_read;
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
template <typename Action>
static void ApplyOverImageRange(const IMAGE_STATE &image_state, const VkImageSubresourceRange &subresource_range_arg,
                                Action &action) {
    // At this point the "apply over range" logic only supports a single memory binding
    if (!SimpleBinding(image_state)) return;
    auto subresource_range = NormalizeSubresourceRange(image_state.createInfo, subresource_range_arg);
    const auto base_address = ResourceBaseAddress(image_state);
    subresource_adapter::ImageRangeGenerator range_gen(*image_state.fragment_encoder.get(), subresource_range, {0, 0, 0},
                                                       image_state.createInfo.extent, base_address);
    for (; range_gen->non_empty(); ++range_gen) {
        action(*range_gen);
    }
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
    const auto ds_resolve = LvlFindInChain<VkSubpassDescriptionDepthStencilResolve>(subpass_ci.pNext);
    if (ds_resolve && ds_resolve->pDepthStencilResolveAttachment &&
        (ds_resolve->pDepthStencilResolveAttachment->attachment != VK_ATTACHMENT_UNUSED) && subpass_ci.pDepthStencilAttachment &&
        (subpass_ci.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED)) {
        const auto src_at = subpass_ci.pDepthStencilAttachment->attachment;
        const auto src_ci = attachment_ci[src_at];
        // The formats are required to match so we can pick either
        const bool resolve_depth = (ds_resolve->depthResolveMode != VK_RESOLVE_MODE_NONE) && FormatHasDepth(src_ci.format);
        const bool resolve_stencil = (ds_resolve->stencilResolveMode != VK_RESOLVE_MODE_NONE) && FormatHasStencil(src_ci.format);
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
                          const CommandExecutionContext &exec_context, const char *func_name)
        : render_pass_(render_pass),
          subpass_(subpass),
          context_(context),
          exec_context_(exec_context),
          func_name_(func_name),
          skip_(false) {}
    void operator()(const char *aspect_name, const char *attachment_name, uint32_t src_at, uint32_t dst_at,
                    const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type, SyncStageAccessIndex current_usage,
                    SyncOrdering ordering_rule) {
        HazardResult hazard;
        hazard = context_.DetectHazard(view_gen, gen_type, current_usage, ordering_rule);
        if (hazard.hazard) {
            skip_ |=
                exec_context_.GetSyncState().LogError(render_pass_, string_SyncHazardVUID(hazard.hazard),
                                                      "%s: Hazard %s in subpass %" PRIu32 "during %s %s, from attachment %" PRIu32
                                                      " to resolve attachment %" PRIu32 ". Access info %s.",
                                                      func_name_, string_SyncHazard(hazard.hazard), subpass_, aspect_name,
                                                      attachment_name, src_at, dst_at, exec_context_.FormatHazard(hazard).c_str());
        }
    }
    // Providing a mechanism for the constructing caller to get the result of the validation
    bool GetSkip() const { return skip_; }

  private:
    VkRenderPass render_pass_;
    const uint32_t subpass_;
    const AccessContext &context_;
    const CommandExecutionContext &exec_context_;
    const char *func_name_;
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

void HazardResult::Set(const ResourceAccessState *access_state_, SyncStageAccessIndex usage_index_, SyncHazard hazard_,
                       const SyncStageAccessFlags &prior_, const ResourceUsageTag tag_) {
    access_state = layer_data::make_unique<const ResourceAccessState>(*access_state_);
    usage_index = usage_index_;
    hazard = hazard_;
    prior_access = prior_;
    tag = tag_;
}

void HazardResult::AddRecordedAccess(const ResourceFirstAccess &first_access) {
    recorded_access = layer_data::make_unique<const ResourceFirstAccess>(first_access);
}

void AccessContext::DeleteAccess(const AddressRange &address) { GetAccessStateMap(address.type).erase_range(address.range); }

AccessContext::AccessContext(uint32_t subpass, VkQueueFlags queue_flags,
                             const std::vector<SubpassDependencyGraphNode> &dependencies,
                             const std::vector<AccessContext> &contexts, const AccessContext *external_context) {
    Reset();
    const auto &subpass_dep = dependencies[subpass];
    bool has_barrier_from_external = subpass_dep.barrier_from_external.size() > 0U;
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
        async_.emplace_back(&contexts[async_subpass]);
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

template <typename Detector>
HazardResult AccessContext::DetectPreviousHazard(AccessAddressType type, const Detector &detector,
                                                 const ResourceAccessRange &range) const {
    ResourceAccessRangeMap descent_map;
    ResolvePreviousAccess(type, range, &descent_map, nullptr);

    HazardResult hazard;
    for (auto prev = descent_map.begin(); prev != descent_map.end() && !hazard.hazard; ++prev) {
        hazard = detector.Detect(prev);
    }
    return hazard;
}

template <typename Action>
void AccessContext::ForAll(Action &&action) {
    for (const auto address_type : kAddressTypes) {
        auto &accesses = GetAccessStateMap(address_type);
        for (auto &access : accesses) {
            action(address_type, access);
        }
    }
}

// A recursive range walker for hazard detection, first for the current context and the (DetectHazardRecur) to walk
// the DAG of the contexts (for example subpasses)
template <typename Detector>
HazardResult AccessContext::DetectHazard(AccessAddressType type, const Detector &detector, const ResourceAccessRange &range,
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
    const auto the_end = accesses.cend();  // End is not invalidated
    auto pos = accesses.lower_bound(range);
    ResourceAccessRange gap = {range.begin, range.begin};

    while (pos != the_end && pos->first.begin < range.end) {
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
        ++pos;
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
HazardResult AccessContext::DetectAsyncHazard(AccessAddressType type, const Detector &detector,
                                              const ResourceAccessRange &range) const {
    auto &accesses = GetAccessStateMap(type);
    auto pos = accesses.lower_bound(range);
    const auto the_end = accesses.end();

    HazardResult hazard;
    while (pos != the_end && pos->first.begin < range.end) {
        hazard = detector.DetectAsync(pos, start_tag_);
        if (hazard.hazard) break;
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
void AccessContext::ResolveAccessRange(AccessAddressType type, const ResourceAccessRange &range, BarrierAction &barrier_action,
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
                ResolvePreviousAccessStack(type, recurrence_range, resolve_map, infill_state, barrier_action);

                // Given that there could be gaps we need to seek carefully to not repeatedly search the same gaps in the next
                // iterator of the outer while.

                // Set the parallel iterator to the end of this range s.t. ++ will move us to the next range whether or
                // not the end of the range is a gap.  For the seek to work, first we need to warn the parallel iterator
                // we stepped on the dest map
                const auto seek_to = recurrence_range.end - 1;  // The subtraction is safe as range can't be empty (loop condition)
                current.invalidate_A();                      // Changes current->range
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
        ResolvePreviousAccessStack<BarrierAction>(type, trailing_fill_range, resolve_map, infill_state, barrier_action);
    }
}

template <typename BarrierAction>
void AccessContext::ResolvePreviousAccessStack(AccessAddressType type, const ResourceAccessRange &range,
                                               ResourceAccessRangeMap *descent_map, const ResourceAccessState *infill_state,
                                               const BarrierAction &previous_barrier) const {
    ResourceAccessStateFunction stacked_barrier(std::ref(previous_barrier));
    ResolvePreviousAccess(type, range, descent_map, infill_state, &stacked_barrier);
}

void AccessContext::ResolvePreviousAccess(AccessAddressType type, const ResourceAccessRange &range,
                                          ResourceAccessRangeMap *descent_map, const ResourceAccessState *infill_state,
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
            prev_dep.source_subpass->ResolveAccessRange(type, range, barrier_action, descent_map, infill_state);
        }
    }
}

// Non-lazy import of all accesses, WaitEvents needs this.
void AccessContext::ResolvePreviousAccesses() {
    ResourceAccessState default_state;
    if (!prev_.size()) return;  // If no previous contexts, nothing to do

    for (const auto address_type : kAddressTypes) {
        ResolvePreviousAccess(address_type, kFullRange, &GetAccessStateMap(address_type), &default_state);
    }
}

AccessAddressType AccessContext::ImageAddressType(const IMAGE_STATE &image) {
    return (image.fragment_encoder->IsLinearImage()) ? AccessAddressType::kLinear : AccessAddressType::kIdealized;
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
    const auto *attachment_gen = view_gen.GetRangeGen(gen_type);
    if (!attachment_gen) return;

    subresource_adapter::ImageRangeGenerator range_gen(*attachment_gen);
    const AccessAddressType address_type = view_gen.GetAddressType();
    for (; range_gen->non_empty(); ++range_gen) {
        ResolveAccessRange(address_type, *range_gen, barrier_action, descent_map, infill_state);
    }
}

template <typename ResolveOp>
void AccessContext::ResolveFromContext(ResolveOp &&resolve_op, const AccessContext &from_context,
                                       const ResourceAccessState *infill_state, bool recur_to_infill) {
    for (auto address_type : kAddressTypes) {
        from_context.ResolveAccessRange(address_type, kFullRange, resolve_op, &GetAccessStateMap(address_type), infill_state,
                                        recur_to_infill);
    }
}

// Layout transitions are handled as if the were occuring in the beginning of the next subpass
bool AccessContext::ValidateLayoutTransitions(const CommandExecutionContext &exec_context, const RENDER_PASS_STATE &rp_state,
                                              const VkRect2D &render_area, uint32_t subpass,
                                              const AttachmentViewGenVector &attachment_views, const char *func_name) const {
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
        if (hazard.hazard) {
            if (hazard.tag == kInvalidTag) {
                skip |= exec_context.GetSyncState().LogError(
                    rp_state.renderPass(), string_SyncHazardVUID(hazard.hazard),
                    "%s: Hazard %s in subpass %" PRIu32 " for attachment %" PRIu32
                    " image layout transition (old_layout: %s, new_layout: %s) after store/resolve operation in subpass %" PRIu32,
                    func_name, string_SyncHazard(hazard.hazard), subpass, transition.attachment,
                    string_VkImageLayout(transition.old_layout), string_VkImageLayout(transition.new_layout), transition.prev_pass);
            } else {
                skip |= exec_context.GetSyncState().LogError(
                    rp_state.renderPass(), string_SyncHazardVUID(hazard.hazard),
                    "%s: Hazard %s in subpass %" PRIu32 " for attachment %" PRIu32
                    " image layout transition (old_layout: %s, new_layout: %s). Access info %s.",
                    func_name, string_SyncHazard(hazard.hazard), subpass, transition.attachment,
                    string_VkImageLayout(transition.old_layout), string_VkImageLayout(transition.new_layout),
                    exec_context.FormatHazard(hazard).c_str());
            }
        }
    }
    return skip;
}

bool AccessContext::ValidateLoadOperation(const CommandExecutionContext &exec_context, const RENDER_PASS_STATE &rp_state,
                                          const VkRect2D &render_area, uint32_t subpass,
                                          const AttachmentViewGenVector &attachment_views, const char *func_name) const {
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

            const bool has_depth = FormatHasDepth(ci.format);
            const bool has_stencil = FormatHasStencil(ci.format);
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
                if (!hazard.hazard && has_stencil && (stencil_load_index != SYNC_ACCESS_INDEX_NONE)) {
                    hazard = DetectHazard(view_gen, AttachmentViewGen::Gen::kStencilOnlyRenderArea, stencil_load_index,
                                          SyncOrdering::kDepthStencilAttachment);
                    aspect = "stencil";
                    checked_stencil = true;
                }
            }

            if (hazard.hazard) {
                auto load_op_string = string_VkAttachmentLoadOp(checked_stencil ? ci.stencilLoadOp : ci.loadOp);
                const auto &sync_state = exec_context.GetSyncState();
                if (hazard.tag == kInvalidTag) {
                    // Hazard vs. ILT
                    skip |= sync_state.LogError(rp_state.renderPass(), string_SyncHazardVUID(hazard.hazard),
                                                "%s: Hazard %s vs. layout transition in subpass %" PRIu32 " for attachment %" PRIu32
                                                " aspect %s during load with loadOp %s.",
                                                func_name, string_SyncHazard(hazard.hazard), subpass, i, aspect, load_op_string);
                } else {
                    skip |= sync_state.LogError(rp_state.renderPass(), string_SyncHazardVUID(hazard.hazard),
                                                "%s: Hazard %s in subpass %" PRIu32 " for attachment %" PRIu32
                                                " aspect %s during load with loadOp %s. Access info %s.",
                                                func_name, string_SyncHazard(hazard.hazard), subpass, i, aspect, load_op_string,
                                                exec_context.FormatHazard(hazard).c_str());
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
                                           const AttachmentViewGenVector &attachment_views, const char *func_name) const {
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
            const bool has_depth = FormatHasDepth(ci.format);
            const bool has_stencil = FormatHasStencil(ci.format);
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
                if (!hazard.hazard && has_stencil && stencil_op_stores) {
                    hazard = DetectHazard(view_gen, AttachmentViewGen::Gen::kStencilOnlyRenderArea,
                                          SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, SyncOrdering::kRaster);
                    aspect = "stencil";
                    checked_stencil = true;
                }
            }

            if (hazard.hazard) {
                const char *const op_type_string = checked_stencil ? "stencilStoreOp" : "storeOp";
                const char *const store_op_string = string_VkAttachmentStoreOp(checked_stencil ? ci.stencilStoreOp : ci.storeOp);
                skip |= exec_context.GetSyncState().LogError(rp_state.renderPass(), string_SyncHazardVUID(hazard.hazard),
                                                             "%s: Hazard %s in subpass %" PRIu32 " for attachment %" PRIu32
                                                             " %s aspect during store with %s %s. Access info %s",
                                                             func_name, string_SyncHazard(hazard.hazard), subpass, i, aspect,
                                                             op_type_string, store_op_string,
                                                             exec_context.FormatHazard(hazard).c_str());
            }
        }
    }
    return skip;
}

bool AccessContext::ValidateResolveOperations(const CommandExecutionContext &exec_context, const RENDER_PASS_STATE &rp_state,
                                              const VkRect2D &render_area, const AttachmentViewGenVector &attachment_views,
                                              const char *func_name, uint32_t subpass) const {
    ValidateResolveAction validate_action(rp_state.renderPass(), subpass, *this, exec_context, func_name);
    ResolveOperation(validate_action, rp_state, attachment_views, subpass);
    return validate_action.GetSkip();
}

AccessContext::TrackBack *AccessContext::AddTrackBack(const AccessContext *context, const SyncBarrier &barrier) {
    prev_.emplace_back(context, barrier);
    return &prev_.back();
}

void AccessContext::AddAsyncContext(const AccessContext *context) { async_.emplace_back(context); }

class HazardDetector {
    SyncStageAccessIndex usage_index_;

  public:
    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos) const { return pos->second.DetectHazard(usage_index_); }
    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos, ResourceUsageTag start_tag) const {
        return pos->second.DetectAsyncHazard(usage_index_, start_tag);
    }
    explicit HazardDetector(SyncStageAccessIndex usage) : usage_index_(usage) {}
};

class HazardDetectorWithOrdering {
    const SyncStageAccessIndex usage_index_;
    const SyncOrdering ordering_rule_;

  public:
    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos) const {
        return pos->second.DetectHazard(usage_index_, ordering_rule_);
    }
    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos, ResourceUsageTag start_tag) const {
        return pos->second.DetectAsyncHazard(usage_index_, start_tag);
    }
    HazardDetectorWithOrdering(SyncStageAccessIndex usage, SyncOrdering ordering) : usage_index_(usage), ordering_rule_(ordering) {}
};

HazardResult AccessContext::DetectHazard(const BUFFER_STATE &buffer, SyncStageAccessIndex usage_index,
                                         const ResourceAccessRange &range) const {
    if (!SimpleBinding(buffer)) return HazardResult();
    const auto base_address = ResourceBaseAddress(buffer);
    HazardDetector detector(usage_index);
    return DetectHazard(AccessAddressType::kLinear, detector, (range + base_address), DetectOptions::kDetectAll);
}

template <typename Detector>
HazardResult AccessContext::DetectHazard(Detector &detector, const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type,
                                         DetectOptions options) const {
    const auto *attachment_gen = view_gen.GetRangeGen(gen_type);
    if (!attachment_gen) return HazardResult();

    subresource_adapter::ImageRangeGenerator range_gen(*attachment_gen);
    const auto address_type = view_gen.GetAddressType();
    for (; range_gen->non_empty(); ++range_gen) {
        HazardResult hazard = DetectHazard(address_type, detector, *range_gen, options);
        if (hazard.hazard) return hazard;
    }

    return HazardResult();
}

template <typename Detector>
HazardResult AccessContext::DetectHazard(Detector &detector, const IMAGE_STATE &image,
                                         const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset,
                                         const VkExtent3D &extent, DetectOptions options) const {
    if (!SimpleBinding(image)) return HazardResult();
    const auto base_address = ResourceBaseAddress(image);
    subresource_adapter::ImageRangeGenerator range_gen(*image.fragment_encoder.get(), subresource_range, offset, extent,
                                                       base_address);
    const auto address_type = ImageAddressType(image);
    for (; range_gen->non_empty(); ++range_gen) {
        HazardResult hazard = DetectHazard(address_type, detector, *range_gen, options);
        if (hazard.hazard) return hazard;
    }
    return HazardResult();
}
template <typename Detector>
HazardResult AccessContext::DetectHazard(Detector &detector, const IMAGE_STATE &image,
                                         const VkImageSubresourceRange &subresource_range, DetectOptions options) const {
    if (!SimpleBinding(image)) return HazardResult();
    const auto base_address = ResourceBaseAddress(image);
    subresource_adapter::ImageRangeGenerator range_gen(*image.fragment_encoder.get(), subresource_range, base_address);
    const auto address_type = ImageAddressType(image);
    for (; range_gen->non_empty(); ++range_gen) {
        HazardResult hazard = DetectHazard(address_type, detector, *range_gen, options);
        if (hazard.hazard) return hazard;
    }
    return HazardResult();
}

HazardResult AccessContext::DetectHazard(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                                         const VkImageSubresourceLayers &subresource, const VkOffset3D &offset,
                                         const VkExtent3D &extent) const {
    VkImageSubresourceRange subresource_range = {subresource.aspectMask, subresource.mipLevel, 1, subresource.baseArrayLayer,
                                                 subresource.layerCount};
    HazardDetector detector(current_usage);
    return DetectHazard(detector, image, subresource_range, offset, extent, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                                         const VkImageSubresourceRange &subresource_range) const {
    HazardDetector detector(current_usage);
    return DetectHazard(detector, image, subresource_range, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type,
                                         SyncStageAccessIndex current_usage, SyncOrdering ordering_rule) const {
    HazardDetectorWithOrdering detector(current_usage, ordering_rule);
    return DetectHazard(detector, view_gen, gen_type, DetectOptions::kDetectAll);
}

HazardResult AccessContext::DetectHazard(const IMAGE_STATE &image, SyncStageAccessIndex current_usage,
                                         const VkImageSubresourceRange &subresource_range, SyncOrdering ordering_rule,
                                         const VkOffset3D &offset, const VkExtent3D &extent) const {
    HazardDetectorWithOrdering detector(current_usage, ordering_rule);
    return DetectHazard(detector, image, subresource_range, offset, extent, DetectOptions::kDetectAll);
}

class BarrierHazardDetector {
  public:
    BarrierHazardDetector(SyncStageAccessIndex usage_index, VkPipelineStageFlags2KHR src_exec_scope,
                          SyncStageAccessFlags src_access_scope)
        : usage_index_(usage_index), src_exec_scope_(src_exec_scope), src_access_scope_(src_access_scope) {}

    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos) const {
        return pos->second.DetectBarrierHazard(usage_index_, src_exec_scope_, src_access_scope_);
    }
    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos, ResourceUsageTag start_tag) const {
        // Async barrier hazard detection can use the same path as the usage index is not IsRead, but is IsWrite
        return pos->second.DetectAsyncHazard(usage_index_, start_tag);
    }

  private:
    SyncStageAccessIndex usage_index_;
    VkPipelineStageFlags2KHR src_exec_scope_;
    SyncStageAccessFlags src_access_scope_;
};

class EventBarrierHazardDetector {
  public:
    EventBarrierHazardDetector(SyncStageAccessIndex usage_index, VkPipelineStageFlags2KHR src_exec_scope,
                               SyncStageAccessFlags src_access_scope, const SyncEventState::ScopeMap &event_scope,
                               ResourceUsageTag scope_tag)
        : usage_index_(usage_index),
          src_exec_scope_(src_exec_scope),
          src_access_scope_(src_access_scope),
          event_scope_(event_scope),
          scope_pos_(event_scope.cbegin()),
          scope_end_(event_scope.cend()),
          scope_tag_(scope_tag) {}

    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos) const {
        // TODO NOTE: This is almost the slowest way to do this... need to intelligently walk this...
        // Need to find a more efficient sync, since we know pos->first is strictly increasing call to call
        // NOTE: "cached_lower_bound_impl" with upgrades could do this.
        if (scope_pos_ == scope_end_) return HazardResult();
        if (!scope_pos_->first.intersects(pos->first)) {
            event_scope_.lower_bound(pos->first);
            if ((scope_pos_ == scope_end_) || !scope_pos_->first.intersects(pos->first)) return HazardResult();
        }

        // Some portion of this pos is in the event_scope, so check for a barrier hazard
        return pos->second.DetectBarrierHazard(usage_index_, src_exec_scope_, src_access_scope_, scope_tag_);
    }
    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos, ResourceUsageTag start_tag) const {
        // Async barrier hazard detection can use the same path as the usage index is not IsRead, but is IsWrite
        return pos->second.DetectAsyncHazard(usage_index_, start_tag);
    }

  private:
    SyncStageAccessIndex usage_index_;
    VkPipelineStageFlags2KHR src_exec_scope_;
    SyncStageAccessFlags src_access_scope_;
    const SyncEventState::ScopeMap &event_scope_;
    SyncEventState::ScopeMap::const_iterator scope_pos_;
    SyncEventState::ScopeMap::const_iterator scope_end_;
    const ResourceUsageTag scope_tag_;
};

HazardResult AccessContext::DetectImageBarrierHazard(const IMAGE_STATE &image, VkPipelineStageFlags2KHR src_exec_scope,
                                                     const SyncStageAccessFlags &src_access_scope,
                                                     const VkImageSubresourceRange &subresource_range,
                                                     const SyncEventState &sync_event, DetectOptions options) const {
    // It's not particularly DRY to get the address type in this function as well as lower down, but we have to select the
    // first access scope map to use, and there's no easy way to plumb it in below.
    const auto address_type = ImageAddressType(image);
    const auto &event_scope = sync_event.FirstScope(address_type);

    EventBarrierHazardDetector detector(SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION, src_exec_scope, src_access_scope,
                                        event_scope, sync_event.first_scope_tag);
    return DetectHazard(detector, image, subresource_range, options);
}

HazardResult AccessContext::DetectImageBarrierHazard(const AttachmentViewGen &view_gen, const SyncBarrier &barrier,
                                                     DetectOptions options) const {
    BarrierHazardDetector detector(SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION, barrier.src_exec_scope.exec_scope,
                                   barrier.src_access_scope);
    return DetectHazard(detector, view_gen, AttachmentViewGen::Gen::kViewSubresource, options);
}

HazardResult AccessContext::DetectImageBarrierHazard(const IMAGE_STATE &image, VkPipelineStageFlags2KHR src_exec_scope,
                                                     const SyncStageAccessFlags &src_access_scope,
                                                     const VkImageSubresourceRange &subresource_range,
                                                     const DetectOptions options) const {
    BarrierHazardDetector detector(SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION, src_exec_scope, src_access_scope);
    return DetectHazard(detector, image, subresource_range, options);
}

HazardResult AccessContext::DetectImageBarrierHazard(const IMAGE_STATE &image, VkPipelineStageFlags2KHR src_exec_scope,
                                                     const SyncStageAccessFlags &src_stage_accesses,
                                                     const VkImageMemoryBarrier &barrier) const {
    auto subresource_range = NormalizeSubresourceRange(image.createInfo, barrier.subresourceRange);
    const auto src_access_scope = SyncStageAccess::AccessScope(src_stage_accesses, barrier.srcAccessMask);
    return DetectImageBarrierHazard(image, src_exec_scope, src_access_scope, subresource_range, kDetectAll);
}
HazardResult AccessContext::DetectImageBarrierHazard(const SyncImageMemoryBarrier &image_barrier) const {
    return DetectImageBarrierHazard(*image_barrier.image.get(), image_barrier.barrier.src_exec_scope.exec_scope,
                                    image_barrier.barrier.src_access_scope, image_barrier.range, kDetectAll);
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

SyncStageAccessFlags SyncStageAccess::AccessScopeByStage(VkPipelineStageFlags2KHR stages) {
    return AccessScopeImpl(stages, syncStageAccessMaskByStageBit);
}

SyncStageAccessFlags SyncStageAccess::AccessScopeByAccess(VkAccessFlags2KHR accesses) {
    return AccessScopeImpl(sync_utils::ExpandAccessFlags(accesses), syncStageAccessMaskByAccessBit);
}

// Getting from stage mask and access mask to stage/access masks is something we need to be good at...
SyncStageAccessFlags SyncStageAccess::AccessScope(VkPipelineStageFlags2KHR stages, VkAccessFlags2KHR accesses) {
    // The access scope is the intersection of all stage/access types possible for the enabled stages and the enables
    // accesses (after doing a couple factoring of common terms the union of stage/access intersections is the intersections
    // of the union of all stage/access types for all the stages and the same unions for the access mask...
    return AccessScopeByStage(stages) & AccessScopeByAccess(accesses);
}

template <typename Action>
void UpdateMemoryAccessState(ResourceAccessRangeMap *accesses, const ResourceAccessRange &range, const Action &action) {
    // TODO: Optimization for operations that do a pure overwrite (i.e. WRITE usages which rewrite the state, vs READ usages
    //       that do incrementalupdates
    assert(accesses);
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

// Give a comparable interface for range generators and ranges
template <typename Action>
void UpdateMemoryAccessState(ResourceAccessRangeMap *accesses, const Action &action, ResourceAccessRange *range) {
    assert(range);
    UpdateMemoryAccessState(accesses, *range, action);
}

template <typename Action, typename RangeGen>
void UpdateMemoryAccessState(ResourceAccessRangeMap *accesses, const Action &action, RangeGen *range_gen_arg) {
    assert(range_gen_arg);
    RangeGen &range_gen = *range_gen_arg;  // Non-const references must be * by style requirement but deref-ing * iterator is a pain
    for (; range_gen->non_empty(); ++range_gen) {
        UpdateMemoryAccessState(accesses, *range_gen, action);
    }
}

template <typename Action, typename RangeGen>
void UpdateMemoryAccessState(ResourceAccessRangeMap *accesses, const Action &action, const RangeGen &range_gen_prebuilt) {
    RangeGen range_gen(range_gen_prebuilt);  // RangeGenerators can be expensive to create from scratch... initialize from built
    for (; range_gen->non_empty(); ++range_gen) {
        UpdateMemoryAccessState(accesses, *range_gen, action);
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
        access_state.Update(usage, ordering_rule, tag);
        return pos;
    }

    UpdateMemoryAccessStateFunctor(AccessAddressType type_, const AccessContext &context_, SyncStageAccessIndex usage_,
                                   SyncOrdering ordering_rule_, ResourceUsageTag tag_)
        : type(type_), context(context_), usage(usage_), ordering_rule(ordering_rule_), tag(tag_) {}
    const AccessAddressType type;
    const AccessContext &context;
    const SyncStageAccessIndex usage;
    const SyncOrdering ordering_rule;
    const ResourceUsageTag tag;
};

// The barrier operation for pipeline and subpass dependencies`
struct PipelineBarrierOp {
    SyncBarrier barrier;
    bool layout_transition;
    PipelineBarrierOp(const SyncBarrier &barrier_, bool layout_transition_)
        : barrier(barrier_), layout_transition(layout_transition_) {}
    PipelineBarrierOp() = default;
    PipelineBarrierOp(const PipelineBarrierOp &) = default;
    void operator()(ResourceAccessState *access_state) const { access_state->ApplyBarrier(barrier, layout_transition); }
};
// The barrier operation for wait events
struct WaitEventBarrierOp {
    ResourceUsageTag scope_tag;
    SyncBarrier barrier;
    bool layout_transition;
    WaitEventBarrierOp(const ResourceUsageTag scope_tag_, const SyncBarrier &barrier_, bool layout_transition_)
        : scope_tag(scope_tag_), barrier(barrier_), layout_transition(layout_transition_) {}
    WaitEventBarrierOp() = default;
    void operator()(ResourceAccessState *access_state) const { access_state->ApplyBarrier(scope_tag, barrier, layout_transition); }
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

    Iterator operator()(ResourceAccessRangeMap *accesses, const Iterator &pos) const {
        auto &access_state = pos->second;
        for (const auto &op : barrier_ops_) {
            op(&access_state);
        }

        if (resolve_) {
            // If this is the last (or only) batch, we can do the pending resolve as the last step in this operation to avoid
            // another walk
            access_state.ApplyPendingBarriers(tag_);
        }
        return pos;
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

void AccessContext::UpdateAccessState(AccessAddressType type, SyncStageAccessIndex current_usage, SyncOrdering ordering_rule,
                                      const ResourceAccessRange &range, const ResourceUsageTag tag) {
    UpdateMemoryAccessStateFunctor action(type, *this, current_usage, ordering_rule, tag);
    UpdateMemoryAccessState(&GetAccessStateMap(type), range, action);
}

void AccessContext::UpdateAccessState(const BUFFER_STATE &buffer, SyncStageAccessIndex current_usage, SyncOrdering ordering_rule,
                                      const ResourceAccessRange &range, const ResourceUsageTag tag) {
    if (!SimpleBinding(buffer)) return;
    const auto base_address = ResourceBaseAddress(buffer);
    UpdateAccessState(AccessAddressType::kLinear, current_usage, ordering_rule, range + base_address, tag);
}

void AccessContext::UpdateAccessState(const IMAGE_STATE &image, SyncStageAccessIndex current_usage, SyncOrdering ordering_rule,
                                      const VkImageSubresourceRange &subresource_range, const ResourceUsageTag &tag) {
    if (!SimpleBinding(image)) return;
    const auto base_address = ResourceBaseAddress(image);
    subresource_adapter::ImageRangeGenerator range_gen(*image.fragment_encoder.get(), subresource_range, base_address);
    const auto address_type = ImageAddressType(image);
    UpdateMemoryAccessStateFunctor action(address_type, *this, current_usage, ordering_rule, tag);
    UpdateMemoryAccessState(&GetAccessStateMap(address_type), action, &range_gen);
}
void AccessContext::UpdateAccessState(const IMAGE_STATE &image, SyncStageAccessIndex current_usage, SyncOrdering ordering_rule,
                                      const VkImageSubresourceRange &subresource_range, const VkOffset3D &offset,
                                      const VkExtent3D &extent, const ResourceUsageTag tag) {
    if (!SimpleBinding(image)) return;
    const auto base_address = ResourceBaseAddress(image);
    subresource_adapter::ImageRangeGenerator range_gen(*image.fragment_encoder.get(), subresource_range, offset, extent,
                                                       base_address);
    const auto address_type = ImageAddressType(image);
    UpdateMemoryAccessStateFunctor action(address_type, *this, current_usage, ordering_rule, tag);
    UpdateMemoryAccessState(&GetAccessStateMap(address_type), action, &range_gen);
}

void AccessContext::UpdateAccessState(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type,
                                      SyncStageAccessIndex current_usage, SyncOrdering ordering_rule, const ResourceUsageTag tag) {
    const ImageRangeGen *gen = view_gen.GetRangeGen(gen_type);
    if (!gen) return;
    subresource_adapter::ImageRangeGenerator range_gen(*gen);
    const auto address_type = view_gen.GetAddressType();
    UpdateMemoryAccessStateFunctor action(address_type, *this, current_usage, ordering_rule, tag);
    ApplyUpdateAction(address_type, action, &range_gen);
}

void AccessContext::UpdateAccessState(const IMAGE_STATE &image, SyncStageAccessIndex current_usage, SyncOrdering ordering_rule,
                                      const VkImageSubresourceLayers &subresource, const VkOffset3D &offset,
                                      const VkExtent3D &extent, const ResourceUsageTag tag) {
    VkImageSubresourceRange subresource_range = {subresource.aspectMask, subresource.mipLevel, 1, subresource.baseArrayLayer,
                                                 subresource.layerCount};
    UpdateAccessState(image, current_usage, ordering_rule, subresource_range, offset, extent, tag);
}

template <typename Action, typename RangeGen>
void AccessContext::ApplyUpdateAction(AccessAddressType address_type, const Action &action, RangeGen *range_gen_arg) {
    assert(range_gen_arg);  //  Old Google C++ styleguide require non-const object pass by * not &, but this isn't an optional arg.
    UpdateMemoryAccessState(&GetAccessStateMap(address_type), action, range_gen_arg);
}

template <typename Action>
void AccessContext::ApplyUpdateAction(const AttachmentViewGen &view_gen, AttachmentViewGen::Gen gen_type, const Action &action) {
    const ImageRangeGen *gen = view_gen.GetRangeGen(gen_type);
    if (!gen) return;
    UpdateMemoryAccessState(&GetAccessStateMap(view_gen.GetAddressType()), action, *gen);
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
            const bool has_depth = FormatHasDepth(ci.format);
            const bool has_stencil = FormatHasStencil(ci.format);
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
    for (const auto address_type : kAddressTypes) {
        UpdateMemoryAccessState(&GetAccessStateMap(address_type), kFullRange, barrier_action);
    }
}

void AccessContext::ResolveChildContexts(const std::vector<AccessContext> &contexts) {
    for (uint32_t subpass_index = 0; subpass_index < contexts.size(); subpass_index++) {
        auto &context = contexts[subpass_index];
        ApplyTrackbackStackAction barrier_action(context.GetDstExternalTrackBack().barriers);
        for (const auto address_type : kAddressTypes) {
            context.ResolveAccessRange(address_type, kFullRange, barrier_action, &GetAccessStateMap(address_type), nullptr, false);
        }
    }
}

// Caller must ensure that lifespan of this is less than from
void AccessContext::ImportAsyncContexts(const AccessContext &from) { async_ = from.async_; }

// Suitable only for *subpass* access contexts
HazardResult AccessContext::DetectSubpassTransitionHazard(const TrackBack &track_back, const AttachmentViewGen &attach_view) const {
    if (!attach_view.IsValid()) return HazardResult();

    // We should never ask for a transition from a context we don't have
    assert(track_back.source_subpass);

    // Do the detection against the specific prior context independent of other contexts.  (Synchronous only)
    // Hazard detection for the transition can be against the merged of the barriers (it only uses src_...)
    const auto merged_barrier = MergeBarriers(track_back.barriers);
    HazardResult hazard = track_back.source_subpass->DetectImageBarrierHazard(attach_view, merged_barrier, kDetectPrevious);
    if (!hazard.hazard) {
        // The Async hazard check is against the current context's async set.
        hazard = DetectImageBarrierHazard(attach_view, merged_barrier, kDetectAsync);
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
        const auto address_type = view_gen.GetAddressType();
        auto &target_map = GetAccessStateMap(address_type);
        ApplySubpassTransitionBarriersAction barrier_action(trackback->barriers);
        prev_context->ResolveAccessRange(view_gen, AttachmentViewGen::Gen::kViewSubresource, barrier_action, &target_map,
                                         &empty_infill);
    }

    // If there were no transitions skip this global map walk
    if (transitions.size()) {
        ResolvePendingBarrierFunctor apply_pending_action(tag);
        ApplyToContext(apply_pending_action);
    }
}

void CommandBufferAccessContext::ApplyGlobalBarriersToEvents(const SyncExecScope &src, const SyncExecScope &dst) {
    auto *events_context = GetCurrentEventsContext();
    assert(events_context);
    events_context->ApplyBarrier(src, dst);
}

bool CommandBufferAccessContext::ValidateDispatchDrawDescriptorSet(VkPipelineBindPoint pipelineBindPoint,
                                                                   const char *func_name) const {
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

    for (const auto &stage_state : pipe->stage_state) {
        const auto raster_state = pipe->RasterizationState();
        if (stage_state.stage_flag == VK_SHADER_STAGE_FRAGMENT_BIT && raster_state && raster_state->rasterizerDiscardEnable) {
            continue;
        }
        for (const auto &set_binding : stage_state.descriptor_uses) {
            const auto *descriptor_set = (*per_sets)[set_binding.first.set].bound_descriptor_set.get();
            cvdescriptorset::DescriptorSetLayout::ConstBindingIterator binding_it(descriptor_set->GetLayout().get(),
                                                                                  set_binding.first.binding);
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
                        if (descriptor->Invalid()) {
                            continue;
                        }

                        // NOTE: ImageSamplerDescriptor inherits from ImageDescriptor, so this cast works for both types.
                        const auto *image_descriptor = static_cast<const ImageDescriptor *>(descriptor);
                        const auto *img_view_state = image_descriptor->GetImageViewState();
                        VkImageLayout image_layout = image_descriptor->GetImageLayout();

                        HazardResult hazard;
                        // NOTE: 2D ImageViews of VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT Images are not allowed in
                        // Descriptors, so we do not have to worry about depth slicing here.
                        // See: VUID 00343
                        assert(!img_view_state->IsDepthSliced());
                        const IMAGE_STATE *img_state = img_view_state->image_state.get();
                        const auto &subresource_range = img_view_state->normalized_subresource_range;

                        if (sync_index == SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ) {
                            const VkExtent3D extent = CastTo3D(cb_state_->activeRenderPassBeginInfo.renderArea.extent);
                            const VkOffset3D offset = CastTo3D(cb_state_->activeRenderPassBeginInfo.renderArea.offset);
                            // Input attachments are subject to raster ordering rules
                            hazard = current_context_->DetectHazard(*img_state, sync_index, subresource_range,
                                                                    SyncOrdering::kRaster, offset, extent);
                        } else {
                            hazard = current_context_->DetectHazard(*img_state, sync_index, subresource_range);
                        }

                        if (hazard.hazard && !sync_state_->SupressedBoundDescriptorWAW(hazard)) {
                            skip |= sync_state_->LogError(
                                img_view_state->image_view(), string_SyncHazardVUID(hazard.hazard),
                                "%s: Hazard %s for %s, in %s, and %s, %s, type: %s, imageLayout: %s, binding #%" PRIu32
                                ", index %" PRIu32 ". Access info %s.",
                                func_name, string_SyncHazard(hazard.hazard),
                                sync_state_->report_data->FormatHandle(img_view_state->image_view()).c_str(),
                                sync_state_->report_data->FormatHandle(cb_state_->commandBuffer()).c_str(),
                                sync_state_->report_data->FormatHandle(pipe->pipeline()).c_str(),
                                sync_state_->report_data->FormatHandle(descriptor_set->GetSet()).c_str(),
                                string_VkDescriptorType(descriptor_type), string_VkImageLayout(image_layout),
                                set_binding.first.binding, index, FormatHazard(hazard).c_str());
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
                        if (hazard.hazard && !sync_state_->SupressedBoundDescriptorWAW(hazard)) {
                            skip |= sync_state_->LogError(
                                buf_view_state->buffer_view(), string_SyncHazardVUID(hazard.hazard),
                                "%s: Hazard %s for %s in %s, %s, and %s, type: %s, binding #%d index %d. Access info %s.",
                                func_name, string_SyncHazard(hazard.hazard),
                                sync_state_->report_data->FormatHandle(buf_view_state->buffer_view()).c_str(),
                                sync_state_->report_data->FormatHandle(cb_state_->commandBuffer()).c_str(),
                                sync_state_->report_data->FormatHandle(pipe->pipeline()).c_str(),
                                sync_state_->report_data->FormatHandle(descriptor_set->GetSet()).c_str(),
                                string_VkDescriptorType(descriptor_type), set_binding.first.binding, index,
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
                        if (hazard.hazard && !sync_state_->SupressedBoundDescriptorWAW(hazard)) {
                            skip |= sync_state_->LogError(
                                buf_state->buffer(), string_SyncHazardVUID(hazard.hazard),
                                "%s: Hazard %s for %s in %s, %s, and %s, type: %s, binding #%d index %d. Access info %s.",
                                func_name, string_SyncHazard(hazard.hazard),
                                sync_state_->report_data->FormatHandle(buf_state->buffer()).c_str(),
                                sync_state_->report_data->FormatHandle(cb_state_->commandBuffer()).c_str(),
                                sync_state_->report_data->FormatHandle(pipe->pipeline()).c_str(),
                                sync_state_->report_data->FormatHandle(descriptor_set->GetSet()).c_str(),
                                string_VkDescriptorType(descriptor_type), set_binding.first.binding, index,
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

    for (const auto &stage_state : pipe->stage_state) {
        const auto raster_state = pipe->RasterizationState();
        if (stage_state.stage_flag == VK_SHADER_STAGE_FRAGMENT_BIT && raster_state && raster_state->rasterizerDiscardEnable) {
            continue;
        }
        for (const auto &set_binding : stage_state.descriptor_uses) {
            const auto *descriptor_set = (*per_sets)[set_binding.first.set].bound_descriptor_set.get();
            cvdescriptorset::DescriptorSetLayout::ConstBindingIterator binding_it(descriptor_set->GetLayout().get(),
                                                                                  set_binding.first.binding);
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
                        // NOTE: ImageSamplerDescriptor inherits from ImageDescriptor, so this cast works for both types.
                        const auto *image_descriptor = static_cast<const ImageDescriptor *>(descriptor);
                        if (image_descriptor->Invalid()) {
                            continue;
                        }
                        const auto *img_view_state = image_descriptor->GetImageViewState();
                        // NOTE: 2D ImageViews of VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT Images are not allowed in
                        // Descriptors, so we do not have to worry about depth slicing here.
                        // See: VUID 00343
                        assert(!img_view_state->IsDepthSliced());
                        const IMAGE_STATE *img_state = img_view_state->image_state.get();
                        if (sync_index == SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ) {
                            const VkExtent3D extent = CastTo3D(cb_state_->activeRenderPassBeginInfo.renderArea.extent);
                            const VkOffset3D offset = CastTo3D(cb_state_->activeRenderPassBeginInfo.renderArea.offset);
                            current_context_->UpdateAccessState(*img_state, sync_index, SyncOrdering::kRaster,
                                                                img_view_state->normalized_subresource_range, offset, extent, tag);
                        } else {
                            current_context_->UpdateAccessState(*img_state, sync_index, SyncOrdering::kNonAttachment,
                                                                img_view_state->normalized_subresource_range, tag);
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

bool CommandBufferAccessContext::ValidateDrawVertex(uint32_t vertexCount, uint32_t firstVertex, const char *func_name) const {
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
            if (binding_buffer.buffer_state == nullptr || binding_buffer.buffer_state->Destroyed()) continue;

            auto *buf_state = binding_buffer.buffer_state.get();
            const ResourceAccessRange range = GetBufferRange(binding_buffer.offset, buf_state->createInfo.size, firstVertex,
                                                             vertexCount, binding_description.stride);
            auto hazard = current_context_->DetectHazard(*buf_state, SYNC_VERTEX_ATTRIBUTE_INPUT_VERTEX_ATTRIBUTE_READ, range);
            if (hazard.hazard) {
                skip |= sync_state_->LogError(
                    buf_state->buffer(), string_SyncHazardVUID(hazard.hazard), "%s: Hazard %s for vertex %s in %s. Access info %s.",
                    func_name, string_SyncHazard(hazard.hazard),
                    sync_state_->report_data->FormatHandle(buf_state->buffer()).c_str(),
                    sync_state_->report_data->FormatHandle(cb_state_->commandBuffer()).c_str(), FormatHazard(hazard).c_str());
            }
        }
    }
    return skip;
}

void CommandBufferAccessContext::RecordDrawVertex(uint32_t vertexCount, uint32_t firstVertex, const ResourceUsageTag tag) {
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
            if (binding_buffer.buffer_state == nullptr || binding_buffer.buffer_state->Destroyed()) continue;

            auto *buf_state = binding_buffer.buffer_state.get();
            const ResourceAccessRange range = GetBufferRange(binding_buffer.offset, buf_state->createInfo.size, firstVertex,
                                                             vertexCount, binding_description.stride);
            current_context_->UpdateAccessState(*buf_state, SYNC_VERTEX_ATTRIBUTE_INPUT_VERTEX_ATTRIBUTE_READ,
                                                SyncOrdering::kNonAttachment, range, tag);
        }
    }
}

bool CommandBufferAccessContext::ValidateDrawVertexIndex(uint32_t indexCount, uint32_t firstIndex, const char *func_name) const {
    bool skip = false;
    if (cb_state_->index_buffer_binding.buffer_state == nullptr || cb_state_->index_buffer_binding.buffer_state->Destroyed()) {
        return skip;
    }

    auto *index_buf_state = cb_state_->index_buffer_binding.buffer_state.get();
    const auto index_size = GetIndexAlignment(cb_state_->index_buffer_binding.index_type);
    const ResourceAccessRange range = GetBufferRange(cb_state_->index_buffer_binding.offset, index_buf_state->createInfo.size,
                                                     firstIndex, indexCount, index_size);
    auto hazard = current_context_->DetectHazard(*index_buf_state, SYNC_INDEX_INPUT_INDEX_READ, range);
    if (hazard.hazard) {
        skip |= sync_state_->LogError(
            index_buf_state->buffer(), string_SyncHazardVUID(hazard.hazard), "%s: Hazard %s for index %s in %s. Access info %s.",
            func_name, string_SyncHazard(hazard.hazard), sync_state_->report_data->FormatHandle(index_buf_state->buffer()).c_str(),
            sync_state_->report_data->FormatHandle(cb_state_->commandBuffer()).c_str(), FormatHazard(hazard).c_str());
    }

    // TODO: For now, we detect the whole vertex buffer. Index buffer could be changed until SubmitQueue.
    //       We will detect more accurate range in the future.
    skip |= ValidateDrawVertex(UINT32_MAX, 0, func_name);
    return skip;
}

void CommandBufferAccessContext::RecordDrawVertexIndex(uint32_t indexCount, uint32_t firstIndex, const ResourceUsageTag tag) {
    if (cb_state_->index_buffer_binding.buffer_state == nullptr || cb_state_->index_buffer_binding.buffer_state->Destroyed()) return;

    auto *index_buf_state = cb_state_->index_buffer_binding.buffer_state.get();
    const auto index_size = GetIndexAlignment(cb_state_->index_buffer_binding.index_type);
    const ResourceAccessRange range = GetBufferRange(cb_state_->index_buffer_binding.offset, index_buf_state->createInfo.size,
                                                     firstIndex, indexCount, index_size);
    current_context_->UpdateAccessState(*index_buf_state, SYNC_INDEX_INPUT_INDEX_READ, SyncOrdering::kNonAttachment, range, tag);

    // TODO: For now, we detect the whole vertex buffer. Index buffer could be changed until SubmitQueue.
    //       We will detect more accurate range in the future.
    RecordDrawVertex(UINT32_MAX, 0, tag);
}

bool CommandBufferAccessContext::ValidateDrawSubpassAttachment(const char *func_name) const {
    bool skip = false;
    if (!current_renderpass_context_) return skip;
    skip |= current_renderpass_context_->ValidateDrawSubpassAttachment(GetExecutionContext(), *cb_state_.get(), func_name);
    return skip;
}

void CommandBufferAccessContext::RecordDrawSubpassAttachment(const ResourceUsageTag tag) {
    if (current_renderpass_context_) {
        current_renderpass_context_->RecordDrawSubpassAttachment(*cb_state_.get(), tag);
    }
}

ResourceUsageTag CommandBufferAccessContext::RecordBeginRenderPass(CMD_TYPE cmd, const RENDER_PASS_STATE &rp_state,
                                                                   const VkRect2D &render_area,
                                                                   const std::vector<const IMAGE_VIEW_STATE *> &attachment_views) {
    // Create an access context the current renderpass.
    const auto barrier_tag = NextCommandTag(cmd, ResourceUsageRecord::SubcommandType::kSubpassTransition);
    const auto load_tag = NextSubcommandTag(cmd, ResourceUsageRecord::SubcommandType::kLoadOp);
    render_pass_contexts_.emplace_back(rp_state, render_area, GetQueueFlags(), attachment_views, &cb_access_context_);
    current_renderpass_context_ = &render_pass_contexts_.back();
    current_renderpass_context_->RecordBeginRenderPass(barrier_tag, load_tag);
    current_context_ = &current_renderpass_context_->CurrentContext();
    return barrier_tag;
}

ResourceUsageTag CommandBufferAccessContext::RecordNextSubpass(const CMD_TYPE cmd) {
    assert(current_renderpass_context_);
    if (!current_renderpass_context_) return NextCommandTag(cmd);

    auto store_tag = NextCommandTag(cmd, ResourceUsageRecord::SubcommandType::kStoreOp);
    auto barrier_tag = NextSubcommandTag(cmd, ResourceUsageRecord::SubcommandType::kSubpassTransition);
    auto load_tag = NextSubcommandTag(cmd, ResourceUsageRecord::SubcommandType::kLoadOp);

    current_renderpass_context_->RecordNextSubpass(store_tag, barrier_tag, load_tag);
    current_context_ = &current_renderpass_context_->CurrentContext();
    return barrier_tag;
}

ResourceUsageTag CommandBufferAccessContext::RecordEndRenderPass(const CMD_TYPE cmd) {
    assert(current_renderpass_context_);
    if (!current_renderpass_context_) return NextCommandTag(cmd);

    auto store_tag = NextCommandTag(cmd, ResourceUsageRecord::SubcommandType::kStoreOp);
    auto barrier_tag = NextSubcommandTag(cmd, ResourceUsageRecord::SubcommandType::kSubpassTransition);

    current_renderpass_context_->RecordEndRenderPass(&cb_access_context_, store_tag, barrier_tag);
    current_context_ = &cb_access_context_;
    current_renderpass_context_ = nullptr;
    return barrier_tag;
}

void CommandBufferAccessContext::RecordDestroyEvent(VkEvent event) {
    // Erase is okay with the key not being
    auto event_state = sync_state_->Get<EVENT_STATE>(event);
    if (event_state) {
        GetCurrentEventsContext()->Destroy(event_state.get());
    }
}

// The is the recorded cb context
bool CommandBufferAccessContext::ValidateFirstUse(CommandExecutionContext *proxy_context, const char *func_name,
                                                  uint32_t index) const {
    assert(proxy_context);
    auto *events_context = proxy_context->GetCurrentEventsContext();
    auto *access_context = proxy_context->GetCurrentAccessContext();
    const ResourceUsageTag base_tag = proxy_context->GetTagLimit();
    bool skip = false;
    ResourceUsageRange tag_range = {0, 0};
    const AccessContext *recorded_context = GetCurrentAccessContext();
    assert(recorded_context);
    HazardResult hazard;
    auto log_msg = [this](const HazardResult &hazard, const CommandExecutionContext &exec_context, const char *func_name,
                          uint32_t index) {
        const auto handle = exec_context.Handle();
        const auto recorded_handle = cb_state_->commandBuffer();
        const auto *report_data = sync_state_->report_data;
        return sync_state_->LogError(handle, string_SyncHazardVUID(hazard.hazard),
                                     "%s: Hazard %s for entry %" PRIu32 ", %s, Recorded access info %s. Access info %s.", func_name,
                                     string_SyncHazard(hazard.hazard), index, report_data->FormatHandle(recorded_handle).c_str(),
                                     FormatUsage(*hazard.recorded_access).c_str(), exec_context.FormatHazard(hazard).c_str());
    };
    const ReplayTrackbackBarriersAction *replay_context = nullptr;
    for (const auto &sync_op : sync_ops_) {
        // we update the range to any include layout transition first use writes,
        // as they are stored along with the source scope (as effective barrier) when recorded
        tag_range.end = sync_op.tag + 1;
        skip |= sync_op.sync_op->ReplayValidate(sync_op.tag, *this, base_tag, proxy_context);

        hazard = recorded_context->DetectFirstUseHazard(tag_range, *access_context, replay_context);
        if (hazard.hazard) {
            skip |= log_msg(hazard, *proxy_context, func_name, index);
        }
        // NOTE: Add call to replay validate here when we add support for syncop with non-trivial replay
        // Record the barrier into the proxy context.
        sync_op.sync_op->ReplayRecord(base_tag + sync_op.tag, access_context, events_context);
        replay_context = sync_op.sync_op->GetReplayTrackback();
        tag_range.begin = tag_range.end;
    }

    // Renderpasses may not cross command buffer boundaries
    assert(replay_context == nullptr);

    // and anything after the last syncop
    tag_range.end = ResourceUsageRecord::kMaxIndex;
    hazard = recorded_context->DetectFirstUseHazard(tag_range, *access_context, replay_context);
    if (hazard.hazard) {
        skip |= log_msg(hazard, *proxy_context, func_name, index);
    }

    return skip;
}

void CommandBufferAccessContext::RecordExecutedCommandBuffer(const CommandBufferAccessContext &recorded_cb_context, CMD_TYPE cmd) {
    auto *events_context = GetCurrentEventsContext();
    auto *access_context = GetCurrentAccessContext();
    const AccessContext *recorded_context = recorded_cb_context.GetCurrentAccessContext();
    assert(recorded_context);

    // Just run through the barriers ignoring the usage from the recorded context, as Resolve will overwrite outdated state
    const ResourceUsageTag base_tag = GetTagLimit();
    for (const auto &sync_op : recorded_cb_context.GetSyncOps()) {
        // we update the range to any include layout transition first use writes,
        // as they are stored along with the source scope (as effective barrier) when recorded
        sync_op.sync_op->ReplayRecord(base_tag + sync_op.tag, access_context, events_context);
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
    cbs_referenced_.emplace(recorded_context.GetCBStateShared());
    access_log_.insert(access_log_.end(), recorded_context.access_log_.cbegin(), recorded_context.access_log_.end());
}

ResourceUsageTag CommandBufferAccessContext::NextSubcommandTag(CMD_TYPE command, ResourceUsageRecord::SubcommandType subcommand) {
    ResourceUsageTag next = access_log_.size();
    access_log_.emplace_back(command, command_number_, subcommand, ++subcommand_number_, cb_state_.get(), reset_count_);
    return next;
}

ResourceUsageTag CommandBufferAccessContext::NextCommandTag(CMD_TYPE command, ResourceUsageRecord::SubcommandType subcommand) {
    command_number_++;
    subcommand_number_ = 0;
    ResourceUsageTag next = access_log_.size();
    access_log_.emplace_back(command, command_number_, subcommand, subcommand_number_, cb_state_.get(), reset_count_);
    return next;
}

ResourceUsageTag CommandBufferAccessContext::NextIndexedCommandTag(CMD_TYPE command, uint32_t index) {
    if (index == 0) {
        return NextCommandTag(command, ResourceUsageRecord::SubcommandType::kIndex);
    }
    return NextSubcommandTag(command, ResourceUsageRecord::SubcommandType::kIndex);
}

void CommandBufferAccessContext::RecordSyncOp(SyncOpPointer &&sync_op) {
    auto tag = sync_op->Record(this);
    // As renderpass operations can have side effects on the command buffer access context,
    // update the sync operation to record these if any.
    if (current_renderpass_context_) {
        const auto &rpc = *current_renderpass_context_;
        sync_op->SetReplayContext(rpc.GetCurrentSubpass(), rpc.GetReplayContext());
    }
    sync_ops_.emplace_back(tag, std::move(sync_op));
}

class HazardDetectFirstUse {
  public:
    HazardDetectFirstUse(const ResourceAccessState &recorded_use, const ResourceUsageRange &tag_range,
                         const ReplayTrackbackBarriersAction *replay_barrier)
        : recorded_use_(recorded_use), tag_range_(tag_range), replay_barrier_(replay_barrier) {}
    HazardResult Detect(const ResourceAccessRangeMap::const_iterator &pos) const {
        if (replay_barrier_) {
            // Intentional copy to apply the replay barrier
            auto access = pos->second;
            (*replay_barrier_)(&access);
            return access.DetectHazard(recorded_use_, tag_range_);
        }
        return pos->second.DetectHazard(recorded_use_, tag_range_);
    }
    HazardResult DetectAsync(const ResourceAccessRangeMap::const_iterator &pos, ResourceUsageTag start_tag) const {
        return pos->second.DetectAsyncHazard(recorded_use_, tag_range_, start_tag);
    }

  private:
    const ResourceAccessState &recorded_use_;
    const ResourceUsageRange &tag_range_;
    const ReplayTrackbackBarriersAction *replay_barrier_;
};

// This is called with the *recorded* command buffers access context, with the *active* access context pass in, againsts which
// hazards will be detected
HazardResult AccessContext::DetectFirstUseHazard(const ResourceUsageRange &tag_range, const AccessContext &access_context,
                                                 const ReplayTrackbackBarriersAction *replay_barrier) const {
    HazardResult hazard;
    for (const auto address_type : kAddressTypes) {
        const auto &recorded_access_map = GetAccessStateMap(address_type);
        for (const auto &recorded_access : recorded_access_map) {
            // Cull any entries not in the current tag range
            if (!recorded_access.second.FirstAccessInTagRange(tag_range)) continue;
            HazardDetectFirstUse detector(recorded_access.second, tag_range, replay_barrier);
            hazard = access_context.DetectHazard(address_type, detector, recorded_access.first, DetectOptions::kDetectAll);
            if (hazard.hazard) break;
        }
    }

    return hazard;
}

bool RenderPassAccessContext::ValidateDrawSubpassAttachment(const CommandExecutionContext &exec_context,
                                                            const CMD_BUFFER_STATE &cmd, const char *func_name) const {
    bool skip = false;
    const auto &sync_state = exec_context.GetSyncState();
    const auto *pipe = cmd.GetCurrentPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS);
    if (!pipe) {
        return skip;
    }

    const auto raster_state = pipe->RasterizationState();
    if (raster_state && raster_state->rasterizerDiscardEnable) {
        return skip;
    }
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
            if (hazard.hazard) {
                const VkImageView view_handle = view_gen.GetViewState()->image_view();
                skip |= sync_state.LogError(view_handle, string_SyncHazardVUID(hazard.hazard),
                                            "%s: Hazard %s for %s in %s, Subpass #%d, and pColorAttachments #%d. Access info %s.",
                                            func_name, string_SyncHazard(hazard.hazard),
                                            sync_state.report_data->FormatHandle(view_handle).c_str(),
                                            sync_state.report_data->FormatHandle(cmd.commandBuffer()).c_str(), cmd.activeSubpass,
                                            location, exec_context.FormatHazard(hazard).c_str());
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

        // PHASE1 TODO: These validation should be in core_checks.
        if (!FormatIsStencilOnly(view_state.create_info.format) && ds_state->depthTestEnable && ds_state->depthWriteEnable &&
            IsImageLayoutDepthWritable(subpass.pDepthStencilAttachment->layout)) {
            depth_write = true;
        }
        // PHASE1 TODO: It needs to check if stencil is writable.
        //              If failOp, passOp, or depthFailOp are not KEEP, and writeMask isn't 0, it's writable.
        //              If depth test is disable, it's considered depth test passes, and then depthFailOp doesn't run.
        // PHASE1 TODO: These validation should be in core_checks.
        if (!FormatIsDepthOnly(view_state.create_info.format) && ds_state->stencilTestEnable &&
            IsImageLayoutStencilWritable(subpass.pDepthStencilAttachment->layout)) {
            stencil_write = true;
        }

        // PHASE1 TODO: Add EARLY stage detection based on ExecutionMode.
        if (depth_write) {
            HazardResult hazard = current_context.DetectHazard(view_gen, AttachmentViewGen::Gen::kDepthOnlyRenderArea,
                                                               SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                                               SyncOrdering::kDepthStencilAttachment);
            if (hazard.hazard) {
                skip |= sync_state.LogError(
                    view_state.image_view(), string_SyncHazardVUID(hazard.hazard),
                    "%s: Hazard %s for %s in %s, Subpass #%d, and depth part of pDepthStencilAttachment. Access info %s.",
                    func_name, string_SyncHazard(hazard.hazard),
                    sync_state.report_data->FormatHandle(view_state.image_view()).c_str(),
                    sync_state.report_data->FormatHandle(cmd.commandBuffer()).c_str(), cmd.activeSubpass,
                    exec_context.FormatHazard(hazard).c_str());
            }
        }
        if (stencil_write) {
            HazardResult hazard = current_context.DetectHazard(view_gen, AttachmentViewGen::Gen::kStencilOnlyRenderArea,
                                                               SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                                               SyncOrdering::kDepthStencilAttachment);
            if (hazard.hazard) {
                skip |= sync_state.LogError(
                    view_state.image_view(), string_SyncHazardVUID(hazard.hazard),
                    "%s: Hazard %s for %s in %s, Subpass #%d, and stencil part of pDepthStencilAttachment. Access info %s.",
                    func_name, string_SyncHazard(hazard.hazard),
                    sync_state.report_data->FormatHandle(view_state.image_view()).c_str(),
                    sync_state.report_data->FormatHandle(cmd.commandBuffer()).c_str(), cmd.activeSubpass,
                    exec_context.FormatHazard(hazard).c_str());
            }
        }
    }
    return skip;
}

void RenderPassAccessContext::RecordDrawSubpassAttachment(const CMD_BUFFER_STATE &cmd, const ResourceUsageTag tag) {
    const auto *pipe = cmd.GetCurrentPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS);
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

        // PHASE1 TODO: These validation should be in core_checks.
        if (has_depth && !FormatIsStencilOnly(view_state.create_info.format) && ds_state->depthTestEnable &&
            ds_state->depthWriteEnable && IsImageLayoutDepthWritable(subpass.pDepthStencilAttachment->layout)) {
            depth_write = true;
        }
        // PHASE1 TODO: It needs to check if stencil is writable.
        //              If failOp, passOp, or depthFailOp are not KEEP, and writeMask isn't 0, it's writable.
        //              If depth test is disable, it's considered depth test passes, and then depthFailOp doesn't run.
        // PHASE1 TODO: These validation should be in core_checks.
        if (has_stencil && !FormatIsDepthOnly(view_state.create_info.format) && ds_state->stencilTestEnable &&
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

bool RenderPassAccessContext::ValidateNextSubpass(const CommandExecutionContext &exec_context, const char *func_name) const {
    // PHASE1 TODO: Add Validate Preserve attachments
    bool skip = false;
    skip |= CurrentContext().ValidateResolveOperations(exec_context, *rp_state_, render_area_, attachment_views_, func_name,
                                                       current_subpass_);
    skip |= CurrentContext().ValidateStoreOperation(exec_context, *rp_state_, render_area_, current_subpass_, attachment_views_,
                                                    func_name);

    const auto next_subpass = current_subpass_ + 1;
    if (next_subpass >= subpass_contexts_.size()) {
        return skip;
    }
    const auto &next_context = subpass_contexts_[next_subpass];
    skip |=
        next_context.ValidateLayoutTransitions(exec_context, *rp_state_, render_area_, next_subpass, attachment_views_, func_name);
    if (!skip) {
        // To avoid complex (and buggy) duplication of the affect of layout transitions on load operations, we'll record them
        // on a copy of the (empty) next context.
        // Note: The resource access map should be empty so hopefully this copy isn't too horrible from a perf POV.
        AccessContext temp_context(next_context);
        temp_context.RecordLayoutTransitions(*rp_state_, next_subpass, attachment_views_, kInvalidTag);
        skip |=
            temp_context.ValidateLoadOperation(exec_context, *rp_state_, render_area_, next_subpass, attachment_views_, func_name);
    }
    return skip;
}
bool RenderPassAccessContext::ValidateEndRenderPass(const CommandExecutionContext &exec_context, const char *func_name) const {
    // PHASE1 TODO: Validate Preserve
    bool skip = false;
    skip |= CurrentContext().ValidateResolveOperations(exec_context, *rp_state_, render_area_, attachment_views_, func_name,
                                                       current_subpass_);
    skip |= CurrentContext().ValidateStoreOperation(exec_context, *rp_state_, render_area_, current_subpass_,

                                                    attachment_views_, func_name);
    skip |= ValidateFinalSubpassLayoutTransitions(exec_context, func_name);
    return skip;
}

AccessContext *RenderPassAccessContext::CreateStoreResolveProxy() const {
    return CreateStoreResolveProxyContext(CurrentContext(), *rp_state_, current_subpass_, attachment_views_);
}

bool RenderPassAccessContext::ValidateFinalSubpassLayoutTransitions(const CommandExecutionContext &exec_context,
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
        if (hazard.hazard) {
            if (hazard.tag == kInvalidTag) {
                // Hazard vs. ILT
                skip |= exec_context.GetSyncState().LogError(
                    rp_state_->renderPass(), string_SyncHazardVUID(hazard.hazard),
                    "%s: Hazard %s vs. store/resolve operations in subpass %" PRIu32 " for attachment %" PRIu32
                    " final image layout transition (old_layout: %s, new_layout: %s).",
                    func_name, string_SyncHazard(hazard.hazard), transition.prev_pass, transition.attachment,
                    string_VkImageLayout(transition.old_layout), string_VkImageLayout(transition.new_layout));
            } else {
                skip |= exec_context.GetSyncState().LogError(
                    rp_state_->renderPass(), string_SyncHazardVUID(hazard.hazard),
                    "%s: Hazard %s with last use subpass %" PRIu32 " for attachment %" PRIu32
                    " final image layout transition (old_layout: %s, new_layout: %s). Access info %s.",
                    func_name, string_SyncHazard(hazard.hazard), transition.prev_pass, transition.attachment,
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
            const bool has_depth = FormatHasDepth(ci.format);
            const bool has_stencil = FormatHasStencil(ci.format);
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
    const VkRect2D &render_area, const std::vector<const IMAGE_VIEW_STATE *> &attachment_views) {
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
                                                 const std::vector<const IMAGE_VIEW_STATE *> &attachment_views,
                                                 const AccessContext *external_context)
    : rp_state_(&rp_state), render_area_(render_area), current_subpass_(0U), attachment_views_() {
    // Add this for all subpasses here so that they exsist during next subpass validation
    subpass_contexts_.reserve(rp_state_->createInfo.subpassCount);
    replay_context_ = std::make_shared<ReplayRenderpassContext>();
    auto &replay_subpass_contexts = replay_context_->subpass_contexts;
    replay_subpass_contexts.reserve(rp_state_->createInfo.subpassCount);
    for (uint32_t pass = 0; pass < rp_state_->createInfo.subpassCount; pass++) {
        subpass_contexts_.emplace_back(pass, queue_flags, rp_state_->subpass_dependencies, subpass_contexts_, external_context);
        replay_subpass_contexts.emplace_back(queue_flags, rp_state_->subpass_dependencies[pass], replay_subpass_contexts);
    }
    attachment_views_ = CreateAttachmentViewGen(render_area, attachment_views);
}
void RenderPassAccessContext::RecordBeginRenderPass(const ResourceUsageTag barrier_tag, const ResourceUsageTag load_tag) {
    assert(0 == current_subpass_);
    subpass_contexts_[current_subpass_].SetStartTag(barrier_tag);
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
    subpass_contexts_[current_subpass_].SetStartTag(barrier_tag);
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
            barrier_action.EmplaceBack(PipelineBarrierOp(barrier, true));
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
    result.valid_accesses = SyncStageAccess::AccessScopeByStage(result.exec_scope);
    return result;
}

SyncExecScope SyncExecScope::MakeDst(VkQueueFlags queue_flags, VkPipelineStageFlags2KHR mask_param) {
    SyncExecScope result;
    result.mask_param = mask_param;
    result.expanded_mask = sync_utils::ExpandPipelineStages(mask_param, queue_flags);
    result.exec_scope = sync_utils::WithLaterPipelineStages(result.expanded_mask);
    result.valid_accesses = SyncStageAccess::AccessScopeByStage(result.exec_scope);
    return result;
}

SyncBarrier::SyncBarrier(const SyncExecScope &src, const SyncExecScope &dst) {
    src_exec_scope = src;
    src_access_scope = 0;
    dst_exec_scope = dst;
    dst_access_scope = 0;
}

template <typename Barrier>
SyncBarrier::SyncBarrier(const Barrier &barrier, const SyncExecScope &src, const SyncExecScope &dst) {
    src_exec_scope = src;
    src_access_scope = SyncStageAccess::AccessScope(src.valid_accesses, barrier.srcAccessMask);
    dst_exec_scope = dst;
    dst_access_scope = SyncStageAccess::AccessScope(dst.valid_accesses, barrier.dstAccessMask);
}

SyncBarrier::SyncBarrier(VkQueueFlags queue_flags, const VkSubpassDependency2 &subpass) {
    const auto barrier = lvl_find_in_chain<VkMemoryBarrier2KHR>(subpass.pNext);
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
    for (const auto &barrier : barriers) {
        ApplyBarrier(barrier, layout_transition);
    }
}

// ApplyBarriers is design for *fully* inclusive barrier lists without layout tranistions.  Designed use was for
// inter-subpass barriers for lazy-evaluation of parent context memory ranges.  Subpass layout transistions are *not* done
// lazily, s.t. no previous access reports should need layout transitions.
void ResourceAccessState::ApplyBarriersImmediate(const std::vector<SyncBarrier> &barriers) {
    assert(!pending_layout_transition);  // This should never be call in the middle of another barrier application
    assert(pending_write_barriers.none());
    assert(!pending_write_dep_chain);
    for (const auto &barrier : barriers) {
        ApplyBarrier(barrier, false);
    }
    ApplyPendingBarriers(kInvalidTag);  // There can't be any need for this tag
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
        if (last_reads.size()) {
            for (const auto &read_access : last_reads) {
                if (IsReadHazard(usage_stage, read_access)) {
                    hazard.Set(this, usage_index, WRITE_AFTER_READ, read_access.access, read_access.tag);
                    break;
                }
            }
        } else if (last_write.any() && IsWriteHazard(usage)) {
            // Write-After-Write check -- if we have a previous write to test against
            hazard.Set(this, usage_index, WRITE_AFTER_WRITE, last_write, write_tag);
        }
    }
    return hazard;
}

HazardResult ResourceAccessState::DetectHazard(SyncStageAccessIndex usage_index, const SyncOrdering ordering_rule) const {
    const auto &ordering = GetOrderingRules(ordering_rule);
    return DetectHazard(usage_index, ordering);
}

HazardResult ResourceAccessState::DetectHazard(SyncStageAccessIndex usage_index, const OrderingBarrier &ordering) const {
    // The ordering guarantees act as barriers to the last accesses, independent of synchronization operations
    HazardResult hazard;
    const auto usage_bit = FlagBit(usage_index);
    const auto usage_stage = PipelineStageBit(usage_index);
    const bool input_attachment_ordering = (ordering.access_scope & SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ_BIT).any();
    const bool last_write_is_ordered = (last_write & ordering.access_scope).any();
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
    } else if (usage_index == SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION) {
        // For Image layout transitions, the barrier represents the first synchronization/access scope of the layout transition
        return DetectBarrierHazard(usage_index, ordering.exec_scope, ordering.access_scope);
    } else {
        // Only check for WAW if there are no reads since last_write
        bool usage_write_is_ordered = (usage_bit & ordering.access_scope).any();
        if (last_reads.size()) {
            // Look for any WAR hazards outside the ordered set of stages
            VkPipelineStageFlags2KHR ordered_stages = 0;
            if (usage_write_is_ordered) {
                // If the usage is ordered, we can ignore all ordered read stages w.r.t. WAR)
                ordered_stages = GetOrderedStages(ordering);
            }
            // If we're tracking any reads that aren't ordered against the current write, got to check 'em all.
            if ((ordered_stages & last_read_stages) != last_read_stages) {
                for (const auto &read_access : last_reads) {
                    if (read_access.stage & ordered_stages) continue;  // but we can skip the ordered ones
                    if (IsReadHazard(usage_stage, read_access)) {
                        hazard.Set(this, usage_index, WRITE_AFTER_READ, read_access.access, read_access.tag);
                        break;
                    }
                }
            }
        } else if (last_write.any() && !(last_write_is_ordered && usage_write_is_ordered)) {
            bool ilt_ilt_hazard = false;
            if ((usage_index == SYNC_IMAGE_LAYOUT_TRANSITION) && (usage_bit == last_write)) {
                // ILT after ILT is a special case where we check the 2nd access scope of the first ILT against the first access
                // scope of the second ILT, which has been passed (smuggled?) in the ordering barrier
                ilt_ilt_hazard = !(write_barriers & ordering.access_scope).any();
            }
            if (ilt_ilt_hazard || IsWriteHazard(usage_bit)) {
                hazard.Set(this, usage_index, WRITE_AFTER_WRITE, last_write, write_tag);
            }
        }
    }
    return hazard;
}

HazardResult ResourceAccessState::DetectHazard(const ResourceAccessState &recorded_use, const ResourceUsageRange &tag_range) const {
    HazardResult hazard;
    using Size = FirstAccesses::size_type;
    const auto &recorded_accesses = recorded_use.first_accesses_;
    Size count = recorded_accesses.size();
    if (count) {
        const auto &last_access = recorded_accesses.back();
        bool do_write_last = IsWrite(last_access.usage_index);
        if (do_write_last) --count;

        for (Size i = 0; i < count; ++count) {
            const auto &first = recorded_accesses[i];
            // Skip and quit logic
            if (first.tag < tag_range.begin) continue;
            if (first.tag >= tag_range.end) {
                do_write_last = false;  // ignore last since we know it can't be in tag_range
                break;
            }

            hazard = DetectHazard(first.usage_index, first.ordering_rule);
            if (hazard.hazard) {
                hazard.AddRecordedAccess(first);
                break;
            }
        }

        if (do_write_last && tag_range.includes(last_access.tag)) {
            // Writes are a bit special... both for the "most recent" access logic, and layout transition specific logic
            OrderingBarrier barrier = GetOrderingRules(last_access.ordering_rule);
            if (last_access.usage_index == SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION) {
                // Or in the layout first access scope as a barrier... IFF the usage is an ILT
                // this was saved off in the "apply barriers" logic to simplify ILT access checks as they straddle
                // the barrier that applies them
                barrier |= recorded_use.first_write_layout_ordering_;
            }
            // Any read stages present in the recorded context (this) are most recent to the write, and thus mask those stages in
            // the active context
            if (recorded_use.first_read_stages_) {
                // we need to ignore the first use read stage in the active context (so we add them to the ordering rule),
                // reads in the active context are not "most recent" as all recorded context operations are *after* them
                // This supresses only RAW checks for stages present in the recorded context, but not those only present in the
                // active context.
                barrier.exec_scope |= recorded_use.first_read_stages_;
                // if there are any first use reads, we suppress WAW by injecting the active context write in the ordering rule
                barrier.access_scope |= FlagBit(last_access.usage_index);
            }
            hazard = DetectHazard(last_access.usage_index, barrier);
            if (hazard.hazard) {
                hazard.AddRecordedAccess(last_access);
            }
        }
    }
    return hazard;
}

// Asynchronous Hazards occur between subpasses with no connection through the DAG
HazardResult ResourceAccessState::DetectAsyncHazard(SyncStageAccessIndex usage_index, const ResourceUsageTag start_tag) const {
    HazardResult hazard;
    auto usage = FlagBit(usage_index);
    // Async checks need to not go back further than the start of the subpass, as we only want to find hazards between the async
    // subpasses.  Anything older than that should have been checked at the start of each subpass, taking into account all of
    // the raster ordering rules.
    if (IsRead(usage)) {
        if (last_write.any() && (write_tag >= start_tag)) {
            hazard.Set(this, usage_index, READ_RACING_WRITE, last_write, write_tag);
        }
    } else {
        if (last_write.any() && (write_tag >= start_tag)) {
            hazard.Set(this, usage_index, WRITE_RACING_WRITE, last_write, write_tag);
        } else if (last_reads.size() > 0) {
            // Any reads during the other subpass will conflict with this write, so we need to check them all.
            for (const auto &read_access : last_reads) {
                if (read_access.tag >= start_tag) {
                    hazard.Set(this, usage_index, WRITE_RACING_READ, read_access.access, read_access.tag);
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

        hazard = DetectAsyncHazard(first.usage_index, start_tag);
        if (hazard.hazard) {
            hazard.AddRecordedAccess(first);
            break;
        }
    }
    return hazard;
}

HazardResult ResourceAccessState::DetectBarrierHazard(SyncStageAccessIndex usage_index, VkPipelineStageFlags2KHR src_exec_scope,
                                                      const SyncStageAccessFlags &src_access_scope) const {
    // Only supporting image layout transitions for now
    assert(usage_index == SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION);
    HazardResult hazard;
    // only test for WAW if there no intervening read operations.
    // See DetectHazard(SyncStagetAccessIndex) above for more details.
    if (last_reads.size()) {
        // Look at the reads if any
        for (const auto &read_access : last_reads) {
            if (read_access.IsReadBarrierHazard(src_exec_scope)) {
                hazard.Set(this, usage_index, WRITE_AFTER_READ, read_access.access, read_access.tag);
                break;
            }
        }
    } else if (last_write.any() && IsWriteBarrierHazard(src_exec_scope, src_access_scope)) {
        hazard.Set(this, usage_index, WRITE_AFTER_WRITE, last_write, write_tag);
    }

    return hazard;
}

HazardResult ResourceAccessState::DetectBarrierHazard(SyncStageAccessIndex usage_index, VkPipelineStageFlags2KHR src_exec_scope,
                                                      const SyncStageAccessFlags &src_access_scope,
                                                      const ResourceUsageTag event_tag) const {
    // Only supporting image layout transitions for now
    assert(usage_index == SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION);
    HazardResult hazard;
    // only test for WAW if there no intervening read operations.
    // See DetectHazard(SyncStagetAccessIndex) above for more details.

    if (last_reads.size()) {
        // Look at the reads if any... if reads exist, they are either the resaon the access is in the event
        // first scope, or they are a hazard.
        for (const auto &read_access : last_reads) {
            if (read_access.tag < event_tag) {
                // The read is in the events first synchronization scope, so we use a barrier hazard check
                // If the read stage is not in the src sync scope
                // *AND* not execution chained with an existing sync barrier (that's the or)
                // then the barrier access is unsafe (R/W after R)
                if (read_access.IsReadBarrierHazard(src_exec_scope)) {
                    hazard.Set(this, usage_index, WRITE_AFTER_READ, read_access.access, read_access.tag);
                    break;
                }
            } else {
                // The read not in the event first sync scope and so is a hazard vs. the layout transition
                hazard.Set(this, usage_index, WRITE_AFTER_READ, read_access.access, read_access.tag);
            }
        }
    } else if (last_write.any()) {
        // if there are no reads, the write is either the reason the access is in the event scope... they are a hazard
        if (write_tag < event_tag) {
            // The write is in the first sync scope of the event (sync their aren't any reads to be the reason)
            // So do a normal barrier hazard check
            if (IsWriteBarrierHazard(src_exec_scope, src_access_scope)) {
                hazard.Set(this, usage_index, WRITE_AFTER_WRITE, last_write, write_tag);
            }
        } else {
            // The write isn't in scope, and is thus a hazard to the layout transistion for wait
            hazard.Set(this, usage_index, WRITE_AFTER_WRITE, last_write, write_tag);
        }
    }

    return hazard;
}

// The logic behind resolves is the same as update, we assume that earlier hazards have be reported, and that no
// tranistive hazard can exists with a hazard between the earlier operations.  Yes, an early hazard can mask that another
// exists, but if you fix *that* hazard it either fixes or unmasks the subsequent ones.
void ResourceAccessState::Resolve(const ResourceAccessState &other) {
    if (write_tag < other.write_tag) {
        // If this is a later write, we've reported any exsiting hazard, and we can just overwrite as the more recent
        // operation
        *this = other;
    } else if (other.write_tag == write_tag) {
        // In the *equals* case for write operations, we merged the write barriers and the read state (but without the
        // dependency chaining logic or any stage expansion)
        write_barriers |= other.write_barriers;
        pending_write_barriers |= other.pending_write_barriers;
        pending_layout_transition |= other.pending_layout_transition;
        pending_write_dep_chain |= other.pending_write_dep_chain;
        pending_layout_ordering_ |= other.pending_layout_ordering_;

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
    }  // the else clause would be that other write is before this write... in which case we supercede the other state and
       // ignore it.

    // Merge first access information by making a copy of this first_access and reconstructing with a shuffle
    // of the copy and other into this using the update first logic.
    // NOTE: All sorts of additional cleverness could be put into short circuts.  (for example back is write and is before front
    //       of the other first_accesses... )
    if (!(first_accesses_ == other.first_accesses_) && !other.first_accesses_.empty()) {
        FirstAccesses firsts(std::move(first_accesses_));
        first_accesses_.clear();
        first_read_stages_ = 0U;
        auto a = firsts.begin();
        auto a_end = firsts.end();
        for (auto &b : other.first_accesses_) {
            // TODO: Determine whether some tag offset will be needed for PHASE II
            while ((a != a_end) && (a->tag < b.tag)) {
                UpdateFirst(a->tag, a->usage_index, a->ordering_rule);
                ++a;
            }
            UpdateFirst(b.tag, b.usage_index, b.ordering_rule);
        }
        for (; a != a_end; ++a) {
            UpdateFirst(a->tag, a->usage_index, a->ordering_rule);
        }
    }
}

void ResourceAccessState::Update(SyncStageAccessIndex usage_index, SyncOrdering ordering_rule, const ResourceUsageTag tag) {
    // Move this logic in the ResourceStateTracker as methods, thereof (or we'll repeat it for every flavor of resource...
    const auto usage_bit = FlagBit(usage_index);
    if (IsRead(usage_index)) {
        // Mulitple outstanding reads may be of interest and do dependency chains independently
        // However, for purposes of barrier tracking, only one read per pipeline stage matters
        const auto usage_stage = PipelineStageBit(usage_index);
        if (usage_stage & last_read_stages) {
            for (auto &read_access : last_reads) {
                if (read_access.stage == usage_stage) {
                    read_access.Set(usage_stage, usage_bit, 0, tag);
                } else if (read_access.barriers & usage_stage) {
                    read_access.sync_stages |= usage_stage;
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
        SetWrite(usage_bit, tag);
    }
    UpdateFirst(tag, usage_index, ordering_rule);
}

// Clobber last read and all barriers... because all we have is DANGER, DANGER, WILL ROBINSON!!!
// if the last_reads/last_write were unsafe, we've reported them, in either case the prior access is irrelevant.
// We can overwrite them as *this* write is now after them.
//
// Note: intentionally ignore pending barriers and chains (i.e. don't apply or clear them), let ApplyPendingBarriers handle them.
void ResourceAccessState::SetWrite(const SyncStageAccessFlags &usage_bit, const ResourceUsageTag tag) {
    ClearRead();
    ClearWrite();
    write_tag = tag;
    last_write = usage_bit;
}

void ResourceAccessState::ClearWrite() {
    read_execution_barriers = VK_PIPELINE_STAGE_2_NONE;
    input_attachment_read = false;  // Denotes no outstanding input attachment read after the last write.
    write_barriers.reset();
    write_dependency_chain = VK_PIPELINE_STAGE_2_NONE;
    last_write.reset();

    write_tag = 0;
    write_queue = QueueSyncState::kQueueIdInvalid;
}

void ResourceAccessState::ClearRead() {
    last_reads.clear();
    last_read_stages = VK_PIPELINE_STAGE_2_NONE;
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
    if (layout_transition || WriteInSourceScopeOrChain(barrier.src_exec_scope.exec_scope, barrier.src_access_scope)) {
        pending_write_barriers |= barrier.dst_access_scope;
        pending_write_dep_chain |= barrier.dst_exec_scope.exec_scope;
        if (layout_transition) {
            pending_layout_ordering_ |= OrderingBarrier(barrier.src_exec_scope.exec_scope, barrier.src_access_scope);
        }
    }
    // Track layout transistion as pending as we can't modify last_write until all barriers processed
    pending_layout_transition |= layout_transition;

    if (!pending_layout_transition) {
        // Once we're dealing with a layout transition (which is modelled as a *write*) then the last reads/writes/chains
        // don't need to be tracked as we're just going to zero them.
        for (auto &read_access : last_reads) {
            // The | implements the "dependency chain" logic for this access, as the barriers field stores the second sync scope
            if (barrier.src_exec_scope.exec_scope & (read_access.stage | read_access.barriers)) {
                read_access.pending_dep_chain |= barrier.dst_exec_scope.exec_scope;
            }
        }
    }
}

// Apply the tag scoped memory barrier without updating the existing barriers.  The execution barrier
// changes the "chaining" state, but to keep barriers independent. See discussion above.
void ResourceAccessState::ApplyBarrier(const ResourceUsageTag scope_tag, const SyncBarrier &barrier, bool layout_transition) {
    // The scope logic for events is, if we're here, the resource usage was flagged as "in the first execution scope" at
    // the time of the SetEvent, thus all we need check is whether the access is the same one (i.e. before the scope tag
    // in order to know if it's in the excecution scope
    // Notice that the layout transition sets the pending barriers *regardless*, as any lack of src_access_scope to
    // guard against the layout transition should be reported in the detect barrier hazard phase, and we only report
    // errors w.r.t. "most recent" accesses.
    if (layout_transition || ((write_tag < scope_tag) && (barrier.src_access_scope & last_write).any())) {
        pending_write_barriers |= barrier.dst_access_scope;
        pending_write_dep_chain |= barrier.dst_exec_scope.exec_scope;
        if (layout_transition) {
            pending_layout_ordering_ |= OrderingBarrier(barrier.src_exec_scope.exec_scope, barrier.src_access_scope);
        }
    }
    // Track layout transistion as pending as we can't modify last_write until all barriers processed
    pending_layout_transition |= layout_transition;

    if (!pending_layout_transition) {
        // Once we're dealing with a layout transition (which is modelled as a *write*) then the last reads/writes/chains
        // don't need to be tracked as we're just going to zero them.
        for (auto &read_access : last_reads) {
            // If this read is the same one we included in the set event and in scope, then apply the execution barrier...
            // NOTE: That's not really correct... this read stage might *not* have been included in the setevent, and the barriers
            // representing the chain might have changed since then (that would be an odd usage), so as a first approximation
            // we'll assume the barriers *haven't* been changed since (if the tag hasn't), and while this could be a false
            // positive in the case of Set; SomeBarrier; Wait; we'll live with it until we can add more state to the first scope
            // capture (the specific write and read stages that *were* in scope at the moment of SetEvents.
            // TODO: eliminate the false positive by including write/read-stages "in scope" information in SetEvents first_scope
            if ((read_access.tag < scope_tag) && (barrier.src_exec_scope.exec_scope & (read_access.stage | read_access.barriers))) {
                read_access.pending_dep_chain |= barrier.dst_exec_scope.exec_scope;
            }
        }
    }
}
void ResourceAccessState::ApplyPendingBarriers(const ResourceUsageTag tag) {
    if (pending_layout_transition) {
        // SetWrite clobbers the last_reads array, and thus we don't have to clear the read_state out.
        SetWrite(SYNC_IMAGE_LAYOUT_TRANSITION_BIT, tag);  // Side effect notes below
        UpdateFirst(tag, SYNC_IMAGE_LAYOUT_TRANSITION, SyncOrdering::kNonAttachment);
        TouchupFirstForLayoutTransition(tag, pending_layout_ordering_);
        pending_layout_ordering_ = OrderingBarrier();
        pending_layout_transition = false;
    }

    // Apply the accumulate execution barriers (and thus update chaining information)
    // for layout transition, last_reads is reset by SetWrite, so this will be skipped.
    for (auto &read_access : last_reads) {
        read_access.barriers |= read_access.pending_dep_chain;
        read_execution_barriers |= read_access.barriers;
        read_access.pending_dep_chain = 0;
    }

    // We OR in the accumulated write chain and barriers even in the case of a layout transition as SetWrite zeros them.
    write_dependency_chain |= pending_write_dep_chain;
    write_barriers |= pending_write_barriers;
    pending_write_dep_chain = 0;
    pending_write_barriers = 0;
}

bool ResourceAccessState::QueueTagPredicate::operator()(QueueId usage_queue, ResourceUsageTag usage_tag) {
    return (queue == usage_queue) && (tag <= usage_tag);
}

bool ResourceAccessState::QueuePredicate::operator()(QueueId usage_queue, ResourceUsageTag) { return queue == usage_queue; }

bool ResourceAccessState::TagPredicate::operator()(QueueId, ResourceUsageTag usage_tag) { return tag <= usage_tag; }

// Return if the resulting state is "empty"
template <typename Pred>
bool ResourceAccessState::ApplyQueueTagWait(Pred &&queue_tag_test) {
    VkPipelineStageFlags2KHR sync_reads = VK_PIPELINE_STAGE_2_NONE;

    // Use the predicate to build a mask of the read stages we are synchronizing
    // Use the sync_stages to also detect reads known to be before any synchronized reads (first pass)
    uint32_t unsync_count = 0;
    uint32_t unsync_sync_stages = 0;
    for (auto &read_access : last_reads) {
        if ((read_access.sync_stages | sync_reads) || queue_tag_test(read_access.queue, read_access.tag)) {
            // If we know this stage is before any stage we syncing, or if the predicate tells us that we are waited for..
            sync_reads |= read_access.stage;
        } else {
            ++unsync_count;
            unsync_sync_stages |= read_access.sync_stages;
        }
    }

    // This processing is not order independent, so it's possible an earlier entry in last_reads with a sync_stages
    // including a sync_reads stage occuring later in last_reads won't be caught.  Thus we re-rinse through the list until
    // until we catch all of them.
    //
    // NOTE: This could be n-squared with last_read.size(), but that tends to be very small (<3), should be okay
    while (unsync_sync_stages & sync_reads) {  // If we found unsync stages that were later shown to be sync'd
        unsync_count = 0;
        unsync_sync_stages = 0;
        for (auto &read_access : last_reads) {
            if (read_access.sync_stages | sync_reads) {
                sync_reads |= read_access.stage;
            } else {
                ++unsync_count;
                unsync_sync_stages |= read_access.sync_stages;
            }
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
    if (last_write.any()) {
        if (queue_tag_test(write_queue, write_tag) || sync_reads) {
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
    if (last_write.any()) write_tag += offset;
    for (auto &read_access : last_reads) {
        read_access.tag += offset;
    }
    for (auto &first : first_accesses_) {
        first.tag += offset;
    }
}

ResourceAccessState::ResourceAccessState()
    : write_barriers(~SyncStageAccessFlags(0)),
      write_dependency_chain(0),
      write_tag(),
      write_queue(QueueSyncState::kQueueIdInvalid),
      last_write(0),
      input_attachment_read(false),
      last_read_stages(0),
      read_execution_barriers(0),
      pending_write_dep_chain(0),
      pending_layout_transition(false),
      pending_write_barriers(0),
      pending_layout_ordering_(),
      first_accesses_(),
      first_read_stages_(0U),
      first_write_layout_ordering_() {}

// This should be just Bits or Index, but we don't have an invalid state for Index
VkPipelineStageFlags2KHR ResourceAccessState::GetReadBarriers(const SyncStageAccessFlags &usage_bit) const {
    VkPipelineStageFlags2KHR barriers = 0U;

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
    if (last_write.any() && (write_queue == QueueSyncState::kQueueIdInvalid)) {
        write_queue = id;
    }
}

bool ResourceAccessState::IsRAWHazard(VkPipelineStageFlags2KHR usage_stage, const SyncStageAccessFlags &usage) const {
    assert(IsRead(usage));
    // Only RAW vs. last_write if it doesn't happen-after any other read because either:
    //    * the previous reads are not hazards, and thus last_write must be visible and available to
    //      any reads that happen after.
    //    * the previous reads *are* hazards to last_write, have been reported, and if that hazard is fixed
    //      the current read will be also not be a hazard, thus reporting a hazard here adds no needed information.
    return last_write.any() && (0 == (read_execution_barriers & usage_stage)) && IsWriteHazard(usage);
}

VkPipelineStageFlags2KHR ResourceAccessState::GetOrderedStages(const OrderingBarrier &ordering) const {
    // Whether the stage are in the ordering scope only matters if the current write is ordered
    VkPipelineStageFlags2KHR ordered_stages = last_read_stages & ordering.exec_scope;
    // Special input attachment handling as always (not encoded in exec_scop)
    const bool input_attachment_ordering = (ordering.access_scope & SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ_BIT).any();
    if (input_attachment_ordering && input_attachment_read) {
        // If we have an input attachment in last_reads and input attachments are ordered we all that stage
        ordered_stages |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR;
    }

    return ordered_stages;
}

void ResourceAccessState::UpdateFirst(const ResourceUsageTag tag, SyncStageAccessIndex usage_index, SyncOrdering ordering_rule) {
    // Only record until we record a write.
    if (first_accesses_.empty() || IsRead(first_accesses_.back().usage_index)) {
        const VkPipelineStageFlags2KHR usage_stage = IsRead(usage_index) ? PipelineStageBit(usage_index) : 0U;
        if (0 == (usage_stage & first_read_stages_)) {
            // If this is a read we haven't seen or a write, record.
            // We always need to know what stages were found prior to write
            first_read_stages_ |= usage_stage;
            if (0 == (read_execution_barriers & usage_stage)) {
                // If this stage isn't masked then we add it (since writes map to usage_stage 0, this also records writes)
                first_accesses_.emplace_back(tag, usage_index, ordering_rule);
            }
        }
    }
}

void ResourceAccessState::TouchupFirstForLayoutTransition(ResourceUsageTag tag, const OrderingBarrier &layout_ordering) {
    // Only call this after recording an image layout transition
    assert(first_accesses_.size());
    if (first_accesses_.back().tag == tag) {
        // If this layout transition is the the first write, add the additional ordering rules that guard the ILT
        assert(first_accesses_.back().usage_index == SyncStageAccessIndex::SYNC_IMAGE_LAYOUT_TRANSITION);
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

ResourceUsageRange SyncValidator::ReserveGlobalTagRange(size_t tag_count) const {
    ResourceUsageRange reserve;
    reserve.begin = tag_limit_.fetch_add(tag_count);
    reserve.end = reserve.begin + tag_count;
    return reserve;
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

template <typename BatchSet, typename Predicate>
static BatchSet GetQueueLastBatchSnapshotImpl(const SyncValidator::QueueSyncStatesMap &queues, Predicate &&pred) {
    BatchSet snapshot;
    for (auto &queue : queues) {
        auto batch = queue.second->LastBatch();
        if (batch && pred(batch)) snapshot.emplace(std::move(batch));
    }
    return snapshot;
}

template <typename Predicate>
QueueBatchContext::ConstBatchSet SyncValidator::GetQueueLastBatchSnapshot(Predicate &&pred) const {
    return GetQueueLastBatchSnapshotImpl<QueueBatchContext::ConstBatchSet, Predicate>(queue_sync_states_,
                                                                                      std::forward<Predicate>(pred));
}

template <typename Predicate>
QueueBatchContext::BatchSet SyncValidator::GetQueueLastBatchSnapshot(Predicate &&pred) {
    return GetQueueLastBatchSnapshotImpl<QueueBatchContext::BatchSet, Predicate>(queue_sync_states_, std::forward<Predicate>(pred));
}

bool SignaledSemaphores::SignalSemaphore(const std::shared_ptr<const SEMAPHORE_STATE> &sem_state,
                                         const std::shared_ptr<QueueBatchContext> &batch,
                                         const VkSemaphoreSubmitInfo &signal_info) {
    const SyncExecScope exec_scope =
        SyncExecScope::MakeSrc(batch->GetQueueFlags(), signal_info.stageMask, VK_PIPELINE_STAGE_2_HOST_BIT);
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
        signal_it->second = std::make_shared<Signal>(sem_state, batch, exec_scope);
        success = true;
    }

    return success;
}

std::shared_ptr<SignaledSemaphores::Signal> SignaledSemaphores::Unsignal(VkSemaphore sem) {
    std::shared_ptr<Signal> unsignaled;
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

std::shared_ptr<CommandBufferAccessContext> SyncValidator::AccessContextFactory(VkCommandBuffer command_buffer) {
    // If we don't have one, make it.
    auto cb_state = Get<CMD_BUFFER_STATE>(command_buffer);
    assert(cb_state.get());
    auto queue_flags = cb_state->GetQueueFlags();
    return std::make_shared<CommandBufferAccessContext>(*this, cb_state, queue_flags);
}

std::shared_ptr<CommandBufferAccessContext> SyncValidator::GetAccessContextShared(VkCommandBuffer command_buffer) {
    return GetMappedInsert(cb_access_state, command_buffer,
                           [this, command_buffer]() { return AccessContextFactory(command_buffer); });
}

std::shared_ptr<const CommandBufferAccessContext> SyncValidator::GetAccessContextShared(VkCommandBuffer command_buffer) const {
    return GetMapped(cb_access_state, command_buffer, []() { return std::shared_ptr<CommandBufferAccessContext>(); });
}

const CommandBufferAccessContext *SyncValidator::GetAccessContext(VkCommandBuffer command_buffer) const {
    return GetMappedPlainFromShared(cb_access_state, command_buffer);
}

CommandBufferAccessContext *SyncValidator::GetAccessContext(VkCommandBuffer command_buffer) {
    return GetAccessContextShared(command_buffer).get();
}

CommandBufferAccessContext *SyncValidator::GetAccessContextNoInsert(VkCommandBuffer command_buffer) {
    return GetMappedPlainFromShared(cb_access_state, command_buffer);
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
        access_found->second->MarkDestroyed();
        cb_access_state.erase(access_found);
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
    auto src_buffer = Get<BUFFER_STATE>(srcBuffer);
    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_buffer) {
            const ResourceAccessRange src_range = MakeRange(*src_buffer, copy_region.srcOffset, copy_region.size);
            auto hazard = context->DetectHazard(*src_buffer, SYNC_COPY_TRANSFER_READ, src_range);
            if (hazard.hazard) {
                skip |= LogError(srcBuffer, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdCopyBuffer: Hazard %s for srcBuffer %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(srcBuffer).c_str(), region,
                                 cb_context->FormatHazard(hazard).c_str());
            }
        }
        if (dst_buffer && !skip) {
            const ResourceAccessRange dst_range = MakeRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, dst_range);
            if (hazard.hazard) {
                skip |= LogError(dstBuffer, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdCopyBuffer: Hazard %s for dstBuffer %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(dstBuffer).c_str(), region,
                                 cb_context->FormatHazard(hazard).c_str());
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

void SyncValidator::PreCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator) {
    // Clear out events from the command buffer contexts
    for (auto &cb_context : cb_access_state) {
        cb_context.second->RecordDestroyEvent(event);
    }
}

bool SyncValidator::ValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfos,
                                           CMD_TYPE cmd_type) const {
    bool skip = false;
    const auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return skip;
    const auto *context = cb_context->GetCurrentAccessContext();
    const char *func_name = CommandTypeString(cmd_type);

    // If we have no previous accesses, we have no hazards
    auto src_buffer = Get<BUFFER_STATE>(pCopyBufferInfos->srcBuffer);
    auto dst_buffer = Get<BUFFER_STATE>(pCopyBufferInfos->dstBuffer);

    for (uint32_t region = 0; region < pCopyBufferInfos->regionCount; region++) {
        const auto &copy_region = pCopyBufferInfos->pRegions[region];
        if (src_buffer) {
            const ResourceAccessRange src_range = MakeRange(*src_buffer, copy_region.srcOffset, copy_region.size);
            auto hazard = context->DetectHazard(*src_buffer, SYNC_COPY_TRANSFER_READ, src_range);
            if (hazard.hazard) {
                // TODO -- add tag information to log msg when useful.
                skip |= LogError(pCopyBufferInfos->srcBuffer, string_SyncHazardVUID(hazard.hazard),
                                 "%s(): Hazard %s for srcBuffer %s, region %" PRIu32 ". Access info %s.", func_name,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(pCopyBufferInfos->srcBuffer).c_str(),
                                 region, cb_context->FormatHazard(hazard).c_str());
            }
        }
        if (dst_buffer && !skip) {
            const ResourceAccessRange dst_range = MakeRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, dst_range);
            if (hazard.hazard) {
                skip |= LogError(pCopyBufferInfos->dstBuffer, string_SyncHazardVUID(hazard.hazard),
                                 "%s(): Hazard %s for dstBuffer %s, region %" PRIu32 ". Access info %s.", func_name,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(pCopyBufferInfos->dstBuffer).c_str(),
                                 region, cb_context->FormatHazard(hazard).c_str());
            }
        }
        if (skip) break;
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer,
                                                     const VkCopyBufferInfo2KHR *pCopyBufferInfos) const {
    return ValidateCmdCopyBuffer2(commandBuffer, pCopyBufferInfos, CMD_COPYBUFFER2KHR);
}

bool SyncValidator::PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfos) const {
    return ValidateCmdCopyBuffer2(commandBuffer, pCopyBufferInfos, CMD_COPYBUFFER2);
}

void SyncValidator::RecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfos, CMD_TYPE cmd_type) {
    auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    const auto tag = cb_context->NextCommandTag(cmd_type);
    auto *context = cb_context->GetCurrentAccessContext();

    auto src_buffer = Get<BUFFER_STATE>(pCopyBufferInfos->srcBuffer);
    auto dst_buffer = Get<BUFFER_STATE>(pCopyBufferInfos->dstBuffer);

    for (uint32_t region = 0; region < pCopyBufferInfos->regionCount; region++) {
        const auto &copy_region = pCopyBufferInfos->pRegions[region];
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

void SyncValidator::PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfos) {
    RecordCmdCopyBuffer2(commandBuffer, pCopyBufferInfos, CMD_COPYBUFFER2KHR);
}

void SyncValidator::PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfos) {
    RecordCmdCopyBuffer2(commandBuffer, pCopyBufferInfos, CMD_COPYBUFFER2);
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

    auto src_image = Get<IMAGE_STATE>(srcImage);
    auto dst_image = Get<IMAGE_STATE>(dstImage);
    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, SYNC_COPY_TRANSFER_READ, copy_region.srcSubresource,
                                                copy_region.srcOffset, copy_region.extent);
            if (hazard.hazard) {
                skip |= LogError(srcImage, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdCopyImage: Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(srcImage).c_str(), region,
                                 cb_access_context->FormatHazard(hazard).c_str());
            }
        }

        if (dst_image) {
            auto hazard = context->DetectHazard(*dst_image, SYNC_COPY_TRANSFER_WRITE, copy_region.dstSubresource,
                                                copy_region.dstOffset, copy_region.extent);
            if (hazard.hazard) {
                skip |= LogError(dstImage, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdCopyImage: Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(dstImage).c_str(), region,
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
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_COPYIMAGE);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<IMAGE_STATE>(srcImage);
    auto dst_image = Get<IMAGE_STATE>(dstImage);

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

bool SyncValidator::ValidateCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo,
                                          CMD_TYPE cmd_type) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const char *func_name = CommandTypeString(cmd_type);
    auto src_image = Get<IMAGE_STATE>(pCopyImageInfo->srcImage);
    auto dst_image = Get<IMAGE_STATE>(pCopyImageInfo->dstImage);

    for (uint32_t region = 0; region < pCopyImageInfo->regionCount; region++) {
        const auto &copy_region = pCopyImageInfo->pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, SYNC_COPY_TRANSFER_READ, copy_region.srcSubresource,
                                                copy_region.srcOffset, copy_region.extent);
            if (hazard.hazard) {
                skip |= LogError(pCopyImageInfo->srcImage, string_SyncHazardVUID(hazard.hazard),
                                 "%s: Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.", func_name,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(pCopyImageInfo->srcImage).c_str(),
                                 region, cb_access_context->FormatHazard(hazard).c_str());
            }
        }

        if (dst_image) {
            auto hazard = context->DetectHazard(*dst_image, SYNC_COPY_TRANSFER_WRITE, copy_region.dstSubresource,
                                                copy_region.dstOffset, copy_region.extent);
            if (hazard.hazard) {
                skip |= LogError(pCopyImageInfo->dstImage, string_SyncHazardVUID(hazard.hazard),
                                 "%s: Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.", func_name,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(pCopyImageInfo->dstImage).c_str(),
                                 region, cb_access_context->FormatHazard(hazard).c_str());
            }
            if (skip) break;
        }
    }

    return skip;
}

bool SyncValidator::PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer,
                                                    const VkCopyImageInfo2KHR *pCopyImageInfo) const {
    return ValidateCmdCopyImage2(commandBuffer, pCopyImageInfo, CMD_COPYIMAGE2KHR);
}

bool SyncValidator::PreCallValidateCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo) const {
    return ValidateCmdCopyImage2(commandBuffer, pCopyImageInfo, CMD_COPYIMAGE2);
}

void SyncValidator::RecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo, CMD_TYPE cmd_type) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(cmd_type);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<IMAGE_STATE>(pCopyImageInfo->srcImage);
    auto dst_image = Get<IMAGE_STATE>(pCopyImageInfo->dstImage);

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
    RecordCmdCopyImage2(commandBuffer, pCopyImageInfo, CMD_COPYIMAGE2KHR);
}

void SyncValidator::PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo) {
    RecordCmdCopyImage2(commandBuffer, pCopyImageInfo, CMD_COPYIMAGE2);
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

    SyncOpPipelineBarrier pipeline_barrier(CMD_PIPELINEBARRIER, *this, cb_access_context->GetQueueFlags(), srcStageMask,
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
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return;

    cb_access_context->RecordSyncOp<SyncOpPipelineBarrier>(CMD_PIPELINEBARRIER, *this, cb_access_context->GetQueueFlags(),
                                                           srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount,
                                                           pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                                                           imageMemoryBarrierCount, pImageMemoryBarriers);
}

bool SyncValidator::PreCallValidateCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer,
                                                          const VkDependencyInfoKHR *pDependencyInfo) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    SyncOpPipelineBarrier pipeline_barrier(CMD_PIPELINEBARRIER2KHR, *this, cb_access_context->GetQueueFlags(), *pDependencyInfo);
    skip = pipeline_barrier.Validate(*cb_access_context);
    return skip;
}

bool SyncValidator::PreCallValidateCmdPipelineBarrier2(VkCommandBuffer commandBuffer,
                                                       const VkDependencyInfo *pDependencyInfo) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    SyncOpPipelineBarrier pipeline_barrier(CMD_PIPELINEBARRIER2, *this, cb_access_context->GetQueueFlags(), *pDependencyInfo);
    skip = pipeline_barrier.Validate(*cb_access_context);
    return skip;
}

void SyncValidator::PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return;

    cb_access_context->RecordSyncOp<SyncOpPipelineBarrier>(CMD_PIPELINEBARRIER2KHR, *this, cb_access_context->GetQueueFlags(),
                                                           *pDependencyInfo);
}

void SyncValidator::PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return;

    cb_access_context->RecordSyncOp<SyncOpPipelineBarrier>(CMD_PIPELINEBARRIER2, *this, cb_access_context->GetQueueFlags(),
                                                           *pDependencyInfo);
}

void SyncValidator::CreateDevice(const VkDeviceCreateInfo *pCreateInfo) {
    // The state tracker sets up the device state
    StateTracker::CreateDevice(pCreateInfo);

    // Add the callback hooks for the functions that are either broadly or deeply used and that the ValidationStateTracker
    // refactor would be messier without.
    // TODO: Find a good way to do this hooklessly.
    SetCommandBufferResetCallback([this](VkCommandBuffer command_buffer) -> void { ResetCommandBufferCallback(command_buffer); });
    SetCommandBufferFreeCallback([this](VkCommandBuffer command_buffer) -> void { FreeCommandBufferCallback(command_buffer); });

    QueueId queue_id = QueueSyncState::kQueueIdBase;
    ForEachShared<QUEUE_STATE>([this, &queue_id](const std::shared_ptr<QUEUE_STATE> &queue_state) {
        auto queue_flags = physical_device_state->queue_family_properties[queue_state->queueFamilyIndex].queueFlags;
        std::shared_ptr<QueueSyncState> queue_sync_state = std::make_shared<QueueSyncState>(queue_state, queue_flags, queue_id++);
        queue_sync_states_.emplace(std::make_pair(queue_state->Queue(), std::move(queue_sync_state)));
    });
}

bool SyncValidator::ValidateBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                            const VkSubpassBeginInfo *pSubpassBeginInfo, CMD_TYPE cmd) const {
    bool skip = false;
    auto cb_context = GetAccessContext(commandBuffer);
    if (cb_context) {
        SyncOpBeginRenderPass sync_op(cmd, *this, pRenderPassBegin, pSubpassBeginInfo);
        skip = sync_op.Validate(*cb_context);
    }
    return skip;
}

bool SyncValidator::PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                      VkSubpassContents contents) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    auto subpass_begin_info = LvlInitStruct<VkSubpassBeginInfo>();
    subpass_begin_info.contents = contents;
    skip |= ValidateBeginRenderPass(commandBuffer, pRenderPassBegin, &subpass_begin_info, CMD_BEGINRENDERPASS);
    return skip;
}

bool SyncValidator::PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                       const VkSubpassBeginInfo *pSubpassBeginInfo) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    skip |= ValidateBeginRenderPass(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, CMD_BEGINRENDERPASS2);
    return skip;
}

bool SyncValidator::PreCallValidateCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                          const VkRenderPassBeginInfo *pRenderPassBegin,
                                                          const VkSubpassBeginInfo *pSubpassBeginInfo) const {
    bool skip = StateTracker::PreCallValidateCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    skip |= ValidateBeginRenderPass(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, CMD_BEGINRENDERPASS2KHR);
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
                                             const VkSubpassBeginInfo *pSubpassBeginInfo, CMD_TYPE cmd) {
    auto cb_context = GetAccessContext(commandBuffer);
    if (cb_context) {
        cb_context->RecordSyncOp<SyncOpBeginRenderPass>(cmd, *this, pRenderPassBegin, pSubpassBeginInfo);
    }
}

void SyncValidator::PostCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBegin,
                                                     VkSubpassContents contents) {
    StateTracker::PostCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    auto subpass_begin_info = LvlInitStruct<VkSubpassBeginInfo>();
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
    RecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, CMD_BEGINRENDERPASS2KHR);
}

bool SyncValidator::ValidateCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                           const VkSubpassEndInfo *pSubpassEndInfo, CMD_TYPE cmd) const {
    bool skip = false;

    auto cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return skip;
    SyncOpNextSubpass sync_op(cmd, *this, pSubpassBeginInfo, pSubpassEndInfo);
    return sync_op.Validate(*cb_context);
}

bool SyncValidator::PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) const {
    bool skip = StateTracker::PreCallValidateCmdNextSubpass(commandBuffer, contents);
    // Convert to a NextSubpass2
    auto subpass_begin_info = LvlInitStruct<VkSubpassBeginInfo>();
    subpass_begin_info.contents = contents;
    auto subpass_end_info = LvlInitStruct<VkSubpassEndInfo>();
    skip |= ValidateCmdNextSubpass(commandBuffer, &subpass_begin_info, &subpass_end_info, CMD_NEXTSUBPASS);
    return skip;
}

bool SyncValidator::PreCallValidateCmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                      const VkSubpassEndInfo *pSubpassEndInfo) const {
    bool skip = StateTracker::PreCallValidateCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    skip |= ValidateCmdNextSubpass(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, CMD_NEXTSUBPASS2KHR);
    return skip;
}

bool SyncValidator::PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                   const VkSubpassEndInfo *pSubpassEndInfo) const {
    bool skip = StateTracker::PreCallValidateCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    skip |= ValidateCmdNextSubpass(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, CMD_NEXTSUBPASS2);
    return skip;
}

void SyncValidator::RecordCmdNextSubpass(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                         const VkSubpassEndInfo *pSubpassEndInfo, CMD_TYPE cmd) {
    auto cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return;

    cb_context->RecordSyncOp<SyncOpNextSubpass>(cmd, *this, pSubpassBeginInfo, pSubpassEndInfo);
}

void SyncValidator::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    StateTracker::PostCallRecordCmdNextSubpass(commandBuffer, contents);
    auto subpass_begin_info = LvlInitStruct<VkSubpassBeginInfo>();
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
    RecordCmdNextSubpass(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, CMD_NEXTSUBPASS2KHR);
}

bool SyncValidator::ValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                             CMD_TYPE cmd) const {
    bool skip = false;

    auto cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return skip;

    SyncOpEndRenderPass sync_op(cmd, *this, pSubpassEndInfo);
    skip |= sync_op.Validate(*cb_context);
    return skip;
}

bool SyncValidator::PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer) const {
    bool skip = StateTracker::PreCallValidateCmdEndRenderPass(commandBuffer);
    skip |= ValidateCmdEndRenderPass(commandBuffer, nullptr, CMD_ENDRENDERPASS);
    return skip;
}

bool SyncValidator::PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo) const {
    bool skip = StateTracker::PreCallValidateCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
    skip |= ValidateCmdEndRenderPass(commandBuffer, pSubpassEndInfo, CMD_ENDRENDERPASS2);
    return skip;
}

bool SyncValidator::PreCallValidateCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                        const VkSubpassEndInfo *pSubpassEndInfo) const {
    bool skip = StateTracker::PreCallValidateCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
    skip |= ValidateCmdEndRenderPass(commandBuffer, pSubpassEndInfo, CMD_ENDRENDERPASS2KHR);
    return skip;
}

void SyncValidator::RecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo, CMD_TYPE cmd) {
    // Resolve the all subpass contexts to the command buffer contexts
    auto cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return;

    cb_context->RecordSyncOp<SyncOpEndRenderPass>(cmd, *this, pSubpassEndInfo);
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
    RecordCmdEndRenderPass(commandBuffer, pSubpassEndInfo, CMD_ENDRENDERPASS2KHR);
    StateTracker::PostCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
}

template <typename RegionType>
bool SyncValidator::ValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                 VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions,
                                                 CMD_TYPE cmd_type) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const char *func_name = CommandTypeString(cmd_type);

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_buffer = Get<BUFFER_STATE>(srcBuffer);
    auto dst_image = Get<IMAGE_STATE>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        HazardResult hazard;
        if (dst_image) {
            if (src_buffer) {
                ResourceAccessRange src_range =
                    MakeRange(copy_region.bufferOffset, GetBufferSizeFromCopyImage(copy_region, dst_image->createInfo.format));
                hazard = context->DetectHazard(*src_buffer, SYNC_COPY_TRANSFER_READ, src_range);
                if (hazard.hazard) {
                    // PHASE1 TODO -- add tag information to log msg when useful.
                    skip |= LogError(srcBuffer, string_SyncHazardVUID(hazard.hazard),
                                     "%s: Hazard %s for srcBuffer %s, region %" PRIu32 ". Access info %s.", func_name,
                                     string_SyncHazard(hazard.hazard), report_data->FormatHandle(srcBuffer).c_str(), region,
                                     cb_access_context->FormatHazard(hazard).c_str());
                }
            }

            hazard = context->DetectHazard(*dst_image, SYNC_COPY_TRANSFER_WRITE, copy_region.imageSubresource,
                                           copy_region.imageOffset, copy_region.imageExtent);
            if (hazard.hazard) {
                skip |= LogError(dstImage, string_SyncHazardVUID(hazard.hazard),
                                 "%s: Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.", func_name,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(dstImage).c_str(), region,
                                 cb_access_context->FormatHazard(hazard).c_str());
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
                                        CMD_COPYBUFFERTOIMAGE);
}

bool SyncValidator::PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                            const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo) const {
    return ValidateCmdCopyBufferToImage(commandBuffer, pCopyBufferToImageInfo->srcBuffer, pCopyBufferToImageInfo->dstImage,
                                        pCopyBufferToImageInfo->dstImageLayout, pCopyBufferToImageInfo->regionCount,
                                        pCopyBufferToImageInfo->pRegions, CMD_COPYBUFFERTOIMAGE2KHR);
}

bool SyncValidator::PreCallValidateCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                         const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo) const {
    return ValidateCmdCopyBufferToImage(commandBuffer, pCopyBufferToImageInfo->srcBuffer, pCopyBufferToImageInfo->dstImage,
                                        pCopyBufferToImageInfo->dstImageLayout, pCopyBufferToImageInfo->regionCount,
                                        pCopyBufferToImageInfo->pRegions, CMD_COPYBUFFERTOIMAGE2);
}

template <typename RegionType>
void SyncValidator::RecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                               VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions,
                                               CMD_TYPE cmd_type) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);

    const auto tag = cb_access_context->NextCommandTag(cmd_type);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_buffer = Get<BUFFER_STATE>(srcBuffer);
    auto dst_image = Get<IMAGE_STATE>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (dst_image) {
            if (src_buffer) {
                ResourceAccessRange src_range =
                    MakeRange(copy_region.bufferOffset, GetBufferSizeFromCopyImage(copy_region, dst_image->createInfo.format));
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
    RecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions, CMD_COPYBUFFERTOIMAGE);
}

void SyncValidator::PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                          const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo) {
    StateTracker::PreCallRecordCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
    RecordCmdCopyBufferToImage(commandBuffer, pCopyBufferToImageInfo->srcBuffer, pCopyBufferToImageInfo->dstImage,
                               pCopyBufferToImageInfo->dstImageLayout, pCopyBufferToImageInfo->regionCount,
                               pCopyBufferToImageInfo->pRegions, CMD_COPYBUFFERTOIMAGE2KHR);
}

void SyncValidator::PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                       const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo) {
    StateTracker::PreCallRecordCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo);
    RecordCmdCopyBufferToImage(commandBuffer, pCopyBufferToImageInfo->srcBuffer, pCopyBufferToImageInfo->dstImage,
                               pCopyBufferToImageInfo->dstImageLayout, pCopyBufferToImageInfo->regionCount,
                               pCopyBufferToImageInfo->pRegions, CMD_COPYBUFFERTOIMAGE2);
}

template <typename RegionType>
bool SyncValidator::ValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                 VkBuffer dstBuffer, uint32_t regionCount, const RegionType *pRegions,
                                                 CMD_TYPE cmd_type) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;
    const char *func_name = CommandTypeString(cmd_type);

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto src_image = Get<IMAGE_STATE>(srcImage);
    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);
    const auto dst_mem = (dst_buffer && !dst_buffer->sparse) ? dst_buffer->MemState()->mem() : VK_NULL_HANDLE;
    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, SYNC_COPY_TRANSFER_READ, copy_region.imageSubresource,
                                                copy_region.imageOffset, copy_region.imageExtent);
            if (hazard.hazard) {
                skip |= LogError(srcImage, string_SyncHazardVUID(hazard.hazard),
                                 "%s: Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.", func_name,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(srcImage).c_str(), region,
                                 cb_access_context->FormatHazard(hazard).c_str());
            }
            if (dst_mem) {
                ResourceAccessRange dst_range =
                    MakeRange(copy_region.bufferOffset, GetBufferSizeFromCopyImage(copy_region, src_image->createInfo.format));
                hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, dst_range);
                if (hazard.hazard) {
                    skip |= LogError(dstBuffer, string_SyncHazardVUID(hazard.hazard),
                                     "%s: Hazard %s for dstBuffer %s, region %" PRIu32 ". Access info %s.", func_name,
                                     string_SyncHazard(hazard.hazard), report_data->FormatHandle(dstBuffer).c_str(), region,
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
                                                        const VkBufferImageCopy *pRegions) const {
    return ValidateCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions,
                                        CMD_COPYIMAGETOBUFFER);
}

bool SyncValidator::PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                            const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo) const {
    return ValidateCmdCopyImageToBuffer(commandBuffer, pCopyImageToBufferInfo->srcImage, pCopyImageToBufferInfo->srcImageLayout,
                                        pCopyImageToBufferInfo->dstBuffer, pCopyImageToBufferInfo->regionCount,
                                        pCopyImageToBufferInfo->pRegions, CMD_COPYIMAGETOBUFFER2KHR);
}

bool SyncValidator::PreCallValidateCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                         const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo) const {
    return ValidateCmdCopyImageToBuffer(commandBuffer, pCopyImageToBufferInfo->srcImage, pCopyImageToBufferInfo->srcImageLayout,
                                        pCopyImageToBufferInfo->dstBuffer, pCopyImageToBufferInfo->regionCount,
                                        pCopyImageToBufferInfo->pRegions, CMD_COPYIMAGETOBUFFER2);
}

template <typename RegionType>
void SyncValidator::RecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                               VkBuffer dstBuffer, uint32_t regionCount, const RegionType *pRegions,
                                               CMD_TYPE cmd_type) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);

    const auto tag = cb_access_context->NextCommandTag(cmd_type);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<IMAGE_STATE>(srcImage);
    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);
    const auto dst_mem = (dst_buffer && !dst_buffer->sparse) ? dst_buffer->MemState()->mem() : VK_NULL_HANDLE;
    const VulkanTypedHandle dst_handle(dst_mem, kVulkanObjectTypeDeviceMemory);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                       copy_region.imageSubresource, copy_region.imageOffset, copy_region.imageExtent, tag);
            if (dst_buffer) {
                ResourceAccessRange dst_range =
                    MakeRange(copy_region.bufferOffset, GetBufferSizeFromCopyImage(copy_region, src_image->createInfo.format));
                context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, dst_range, tag);
            }
        }
    }
}

void SyncValidator::PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                      VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions) {
    StateTracker::PreCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    RecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions, CMD_COPYIMAGETOBUFFER);
}

void SyncValidator::PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                          const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo) {
    StateTracker::PreCallRecordCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
    RecordCmdCopyImageToBuffer(commandBuffer, pCopyImageToBufferInfo->srcImage, pCopyImageToBufferInfo->srcImageLayout,
                               pCopyImageToBufferInfo->dstBuffer, pCopyImageToBufferInfo->regionCount,
                               pCopyImageToBufferInfo->pRegions, CMD_COPYIMAGETOBUFFER2KHR);
}

void SyncValidator::PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                       const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo) {
    StateTracker::PreCallRecordCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo);
    RecordCmdCopyImageToBuffer(commandBuffer, pCopyImageToBufferInfo->srcImage, pCopyImageToBufferInfo->srcImageLayout,
                               pCopyImageToBufferInfo->dstBuffer, pCopyImageToBufferInfo->regionCount,
                               pCopyImageToBufferInfo->pRegions, CMD_COPYIMAGETOBUFFER2);
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

    auto src_image = Get<IMAGE_STATE>(srcImage);
    auto dst_image = Get<IMAGE_STATE>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &blit_region = pRegions[region];
        if (src_image) {
            VkOffset3D offset = {std::min(blit_region.srcOffsets[0].x, blit_region.srcOffsets[1].x),
                                 std::min(blit_region.srcOffsets[0].y, blit_region.srcOffsets[1].y),
                                 std::min(blit_region.srcOffsets[0].z, blit_region.srcOffsets[1].z)};
            VkExtent3D extent = {static_cast<uint32_t>(abs(blit_region.srcOffsets[1].x - blit_region.srcOffsets[0].x)),
                                 static_cast<uint32_t>(abs(blit_region.srcOffsets[1].y - blit_region.srcOffsets[0].y)),
                                 static_cast<uint32_t>(abs(blit_region.srcOffsets[1].z - blit_region.srcOffsets[0].z))};
            auto hazard = context->DetectHazard(*src_image, SYNC_BLIT_TRANSFER_READ, blit_region.srcSubresource, offset, extent);
            if (hazard.hazard) {
                skip |= LogError(srcImage, string_SyncHazardVUID(hazard.hazard),
                                 "%s: Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.", apiName,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(srcImage).c_str(), region,
                                 cb_access_context->FormatHazard(hazard).c_str());
            }
        }

        if (dst_image) {
            VkOffset3D offset = {std::min(blit_region.dstOffsets[0].x, blit_region.dstOffsets[1].x),
                                 std::min(blit_region.dstOffsets[0].y, blit_region.dstOffsets[1].y),
                                 std::min(blit_region.dstOffsets[0].z, blit_region.dstOffsets[1].z)};
            VkExtent3D extent = {static_cast<uint32_t>(abs(blit_region.dstOffsets[1].x - blit_region.dstOffsets[0].x)),
                                 static_cast<uint32_t>(abs(blit_region.dstOffsets[1].y - blit_region.dstOffsets[0].y)),
                                 static_cast<uint32_t>(abs(blit_region.dstOffsets[1].z - blit_region.dstOffsets[0].z))};
            auto hazard = context->DetectHazard(*dst_image, SYNC_BLIT_TRANSFER_WRITE, blit_region.dstSubresource, offset, extent);
            if (hazard.hazard) {
                skip |= LogError(dstImage, string_SyncHazardVUID(hazard.hazard),
                                 "%s: Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.", apiName,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(dstImage).c_str(), region,
                                 cb_access_context->FormatHazard(hazard).c_str());
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

bool SyncValidator::PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer,
    const VkBlitImageInfo2 *pBlitImageInfo) const {
    return ValidateCmdBlitImage(commandBuffer, pBlitImageInfo->srcImage, pBlitImageInfo->srcImageLayout, pBlitImageInfo->dstImage,
        pBlitImageInfo->dstImageLayout, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
        pBlitImageInfo->filter, "vkCmdBlitImage2");
}

template <typename RegionType>
void SyncValidator::RecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                       VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                       const RegionType *pRegions, VkFilter filter, ResourceUsageTag tag) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<IMAGE_STATE>(srcImage);
    auto dst_image = Get<IMAGE_STATE>(dstImage);

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

void SyncValidator::PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo) {
    StateTracker::PreCallRecordCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_BLITIMAGE2);
    RecordCmdBlitImage(commandBuffer, pBlitImageInfo->srcImage, pBlitImageInfo->srcImageLayout, pBlitImageInfo->dstImage,
        pBlitImageInfo->dstImageLayout, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
        pBlitImageInfo->filter, tag);
}

bool SyncValidator::ValidateIndirectBuffer(const CommandBufferAccessContext &cb_context, const AccessContext &context,
                                           VkCommandBuffer commandBuffer, const VkDeviceSize struct_size, const VkBuffer buffer,
                                           const VkDeviceSize offset, const uint32_t drawCount, const uint32_t stride,
                                           const char *function) const {
    bool skip = false;
    if (drawCount == 0) return skip;

    auto buf_state = Get<BUFFER_STATE>(buffer);
    VkDeviceSize size = struct_size;
    if (drawCount == 1 || stride == size) {
        if (drawCount > 1) size *= drawCount;
        const ResourceAccessRange range = MakeRange(offset, size);
        auto hazard = context.DetectHazard(*buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, range);
        if (hazard.hazard) {
            skip |= LogError(buf_state->buffer(), string_SyncHazardVUID(hazard.hazard),
                             "%s: Hazard %s for indirect %s in %s. Access info %s.", function, string_SyncHazard(hazard.hazard),
                             report_data->FormatHandle(buffer).c_str(), report_data->FormatHandle(commandBuffer).c_str(),
                             cb_context.FormatHazard(hazard).c_str());
        }
    } else {
        for (uint32_t i = 0; i < drawCount; ++i) {
            const ResourceAccessRange range = MakeRange(offset + i * stride, size);
            auto hazard = context.DetectHazard(*buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, range);
            if (hazard.hazard) {
                skip |= LogError(buf_state->buffer(), string_SyncHazardVUID(hazard.hazard),
                                 "%s: Hazard %s for indirect %s in %s. Access info %s.", function, string_SyncHazard(hazard.hazard),
                                 report_data->FormatHandle(buffer).c_str(), report_data->FormatHandle(commandBuffer).c_str(),
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
                                        const char *function) const {
    bool skip = false;

    auto count_buf_state = Get<BUFFER_STATE>(buffer);
    const ResourceAccessRange range = MakeRange(offset, 4);
    auto hazard = context.DetectHazard(*count_buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, range);
    if (hazard.hazard) {
        skip |= LogError(count_buf_state->buffer(), string_SyncHazardVUID(hazard.hazard),
                         "%s: Hazard %s for countBuffer %s in %s. Access info %s.", function, string_SyncHazard(hazard.hazard),
                         report_data->FormatHandle(buffer).c_str(), report_data->FormatHandle(commandBuffer).c_str(),
                         cb_context.FormatHazard(hazard).c_str());
    }
    return skip;
}

void SyncValidator::RecordCountBuffer(AccessContext &context, const ResourceUsageTag tag, VkBuffer buffer, VkDeviceSize offset) {
    auto count_buf_state = Get<BUFFER_STATE>(buffer);
    const ResourceAccessRange range = MakeRange(offset, 4);
    context.UpdateAccessState(*count_buf_state, SYNC_DRAW_INDIRECT_INDIRECT_COMMAND_READ, SyncOrdering::kNonAttachment, range, tag);
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
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, commandBuffer, sizeof(VkDispatchIndirectCommand), buffer, offset,
                                   1, sizeof(VkDispatchIndirectCommand), "vkCmdDispatchIndirect");
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
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, commandBuffer, sizeof(VkDrawIndirectCommand), buffer, offset,
                                   drawCount, stride, "vkCmdDrawIndirect");

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
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, commandBuffer, sizeof(VkDrawIndexedIndirectCommand), buffer,
                                   offset, drawCount, stride, "vkCmdDrawIndexedIndirect");

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
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, commandBuffer, sizeof(VkDrawIndirectCommand), buffer, offset,
                                   maxDrawCount, stride, function);
    skip |= ValidateCountBuffer(*cb_access_context, *context, commandBuffer, countBuffer, countBufferOffset, function);

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

void SyncValidator::RecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                               uint32_t stride, CMD_TYPE cmd_type) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(cmd_type);
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

void SyncValidator::PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride) {
    StateTracker::PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                    stride);
    RecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                               CMD_DRAWINDIRECTCOUNT);
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
    RecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                               CMD_DRAWINDIRECTCOUNTKHR);
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
    RecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                               CMD_DRAWINDIRECTCOUNTAMD);
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
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, commandBuffer, sizeof(VkDrawIndexedIndirectCommand), buffer,
                                   offset, maxDrawCount, stride, function);
    skip |= ValidateCountBuffer(*cb_access_context, *context, commandBuffer, countBuffer, countBufferOffset, function);

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

void SyncValidator::RecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, CMD_TYPE cmd_type) {
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(cmd_type);
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

void SyncValidator::PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                             uint32_t maxDrawCount, uint32_t stride) {
    StateTracker::PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                           maxDrawCount, stride);
    RecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      CMD_DRAWINDEXEDINDIRECTCOUNT);
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
    RecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      CMD_DRAWINDEXEDINDIRECTCOUNTKHR);
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
    RecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      CMD_DRAWINDEXEDINDIRECTCOUNTAMD);
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

    auto image_state = Get<IMAGE_STATE>(image);

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            auto hazard = context->DetectHazard(*image_state, SYNC_CLEAR_TRANSFER_WRITE, range);
            if (hazard.hazard) {
                skip |= LogError(image, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdClearColorImage: Hazard %s for %s, range index %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(image).c_str(), index,
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
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_CLEARCOLORIMAGE);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto image_state = Get<IMAGE_STATE>(image);

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
                                                             const VkImageSubresourceRange *pRanges) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto image_state = Get<IMAGE_STATE>(image);

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            auto hazard = context->DetectHazard(*image_state, SYNC_CLEAR_TRANSFER_WRITE, range);
            if (hazard.hazard) {
                skip |= LogError(image, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdClearDepthStencilImage: Hazard %s for %s, range index %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(image).c_str(), index,
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
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_CLEARDEPTHSTENCILIMAGE);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto image_state = Get<IMAGE_STATE>(image);

    for (uint32_t index = 0; index < rangeCount; index++) {
        const auto &range = pRanges[index];
        if (image_state) {
            context->UpdateAccessState(*image_state, SYNC_CLEAR_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
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

    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, stride * queryCount);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, range);
        if (hazard.hazard) {
            skip |=
                LogError(dstBuffer, string_SyncHazardVUID(hazard.hazard),
                         "vkCmdCopyQueryPoolResults: Hazard %s for dstBuffer %s. Access info %s.", string_SyncHazard(hazard.hazard),
                         report_data->FormatHandle(dstBuffer).c_str(), cb_access_context->FormatHazard(hazard).c_str());
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

    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, stride * queryCount);
        context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
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

    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(*dst_buffer, dstOffset, size);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, range);
        if (hazard.hazard) {
            skip |= LogError(dstBuffer, string_SyncHazardVUID(hazard.hazard),
                             "vkCmdFillBuffer: Hazard %s for dstBuffer %s. Access info %s.", string_SyncHazard(hazard.hazard),
                             report_data->FormatHandle(dstBuffer).c_str(), cb_access_context->FormatHazard(hazard).c_str());
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

    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(*dst_buffer, dstOffset, size);
        context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
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

    auto src_image = Get<IMAGE_STATE>(srcImage);
    auto dst_image = Get<IMAGE_STATE>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &resolve_region = pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, SYNC_RESOLVE_TRANSFER_READ, resolve_region.srcSubresource,
                                                resolve_region.srcOffset, resolve_region.extent);
            if (hazard.hazard) {
                skip |= LogError(srcImage, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdResolveImage: Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(srcImage).c_str(), region,
                                 cb_access_context->FormatHazard(hazard).c_str());
            }
        }

        if (dst_image) {
            auto hazard = context->DetectHazard(*dst_image, SYNC_RESOLVE_TRANSFER_WRITE, resolve_region.dstSubresource,
                                                resolve_region.dstOffset, resolve_region.extent);
            if (hazard.hazard) {
                skip |= LogError(dstImage, string_SyncHazardVUID(hazard.hazard),
                                 "vkCmdResolveImage: Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(dstImage).c_str(), region,
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
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_RESOLVEIMAGE);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<IMAGE_STATE>(srcImage);
    auto dst_image = Get<IMAGE_STATE>(dstImage);

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

bool SyncValidator::ValidateCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo,
                                             CMD_TYPE cmd_type) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    const char *func_name = CommandTypeString(cmd_type);
    auto src_image = Get<IMAGE_STATE>(pResolveImageInfo->srcImage);
    auto dst_image = Get<IMAGE_STATE>(pResolveImageInfo->dstImage);

    for (uint32_t region = 0; region < pResolveImageInfo->regionCount; region++) {
        const auto &resolve_region = pResolveImageInfo->pRegions[region];
        if (src_image) {
            auto hazard = context->DetectHazard(*src_image, SYNC_RESOLVE_TRANSFER_READ, resolve_region.srcSubresource,
                                                resolve_region.srcOffset, resolve_region.extent);
            if (hazard.hazard) {
                skip |= LogError(pResolveImageInfo->srcImage, string_SyncHazardVUID(hazard.hazard),
                                 "%s: Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.", func_name,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(pResolveImageInfo->srcImage).c_str(),
                                 region, cb_access_context->FormatHazard(hazard).c_str());
            }
        }

        if (dst_image) {
            auto hazard = context->DetectHazard(*dst_image, SYNC_RESOLVE_TRANSFER_WRITE, resolve_region.dstSubresource,
                                                resolve_region.dstOffset, resolve_region.extent);
            if (hazard.hazard) {
                skip |= LogError(pResolveImageInfo->dstImage, string_SyncHazardVUID(hazard.hazard),
                                 "%s: Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.", func_name,
                                 string_SyncHazard(hazard.hazard), report_data->FormatHandle(pResolveImageInfo->dstImage).c_str(),
                                 region, cb_access_context->FormatHazard(hazard).c_str());
            }
            if (skip) break;
        }
    }

    return skip;
}

bool SyncValidator::PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                                       const VkResolveImageInfo2KHR *pResolveImageInfo) const {
    return ValidateCmdResolveImage2(commandBuffer, pResolveImageInfo, CMD_RESOLVEIMAGE2KHR);
}

bool SyncValidator::PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer,
                                                    const VkResolveImageInfo2 *pResolveImageInfo) const {
    return ValidateCmdResolveImage2(commandBuffer, pResolveImageInfo, CMD_RESOLVEIMAGE2);
}

void SyncValidator::RecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo,
                                           CMD_TYPE cmd_type) {
    StateTracker::PreCallRecordCmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(cmd_type);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<IMAGE_STATE>(pResolveImageInfo->srcImage);
    auto dst_image = Get<IMAGE_STATE>(pResolveImageInfo->dstImage);

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
    RecordCmdResolveImage2(commandBuffer, pResolveImageInfo, CMD_RESOLVEIMAGE2KHR);
}

void SyncValidator::PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo) {
    RecordCmdResolveImage2(commandBuffer, pResolveImageInfo, CMD_RESOLVEIMAGE2);
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

    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        // VK_WHOLE_SIZE not allowed
        const ResourceAccessRange range = MakeRange(dstOffset, dataSize);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, range);
        if (hazard.hazard) {
            skip |= LogError(dstBuffer, string_SyncHazardVUID(hazard.hazard),
                             "vkCmdUpdateBuffer: Hazard %s for dstBuffer %s. Access info %s.", string_SyncHazard(hazard.hazard),
                             report_data->FormatHandle(dstBuffer).c_str(), cb_access_context->FormatHazard(hazard).c_str());
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

    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        // VK_WHOLE_SIZE not allowed
        const ResourceAccessRange range = MakeRange(dstOffset, dataSize);
        context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
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

    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, 4);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, range);
        if (hazard.hazard) {
            skip |=
                LogError(dstBuffer, string_SyncHazardVUID(hazard.hazard),
                         "vkCmdWriteBufferMarkerAMD: Hazard %s for dstBuffer %s. Access info %s.", string_SyncHazard(hazard.hazard),
                         report_data->FormatHandle(dstBuffer).c_str(), cb_access_context->FormatHazard(hazard).c_str());
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

    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, 4);
        context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
    }
}

bool SyncValidator::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const {
    bool skip = false;
    const auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return skip;

    SyncOpSetEvent set_event_op(CMD_SETEVENT, *this, cb_context->GetQueueFlags(), event, stageMask);
    return set_event_op.Validate(*cb_context);
}

void SyncValidator::PostCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    StateTracker::PostCallRecordCmdSetEvent(commandBuffer, event, stageMask);
    auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return;
    cb_context->RecordSyncOp<SyncOpSetEvent>(CMD_SETEVENT, *this, cb_context->GetQueueFlags(), event, stageMask);
}

bool SyncValidator::PreCallValidateCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                   const VkDependencyInfoKHR *pDependencyInfo) const {
    bool skip = false;
    const auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context || !pDependencyInfo) return skip;

    SyncOpSetEvent set_event_op(CMD_SETEVENT2KHR, *this, cb_context->GetQueueFlags(), event, *pDependencyInfo);
    return set_event_op.Validate(*cb_context);
}

bool SyncValidator::PreCallValidateCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                                const VkDependencyInfo *pDependencyInfo) const {
    bool skip = false;
    const auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context || !pDependencyInfo) return skip;

    SyncOpSetEvent set_event_op(CMD_SETEVENT2, *this, cb_context->GetQueueFlags(), event, *pDependencyInfo);
    return set_event_op.Validate(*cb_context);
}

void SyncValidator::PostCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                  const VkDependencyInfoKHR *pDependencyInfo) {
    StateTracker::PostCallRecordCmdSetEvent2KHR(commandBuffer, event, pDependencyInfo);
    auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context || !pDependencyInfo) return;

    cb_context->RecordSyncOp<SyncOpSetEvent>(CMD_SETEVENT2KHR, *this, cb_context->GetQueueFlags(), event, *pDependencyInfo);
}

void SyncValidator::PostCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                               const VkDependencyInfo *pDependencyInfo) {
    StateTracker::PostCallRecordCmdSetEvent2(commandBuffer, event, pDependencyInfo);
    auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context || !pDependencyInfo) return;

    cb_context->RecordSyncOp<SyncOpSetEvent>(CMD_SETEVENT2, *this, cb_context->GetQueueFlags(), event, *pDependencyInfo);
}

bool SyncValidator::PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                                 VkPipelineStageFlags stageMask) const {
    bool skip = false;
    const auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return skip;

    SyncOpResetEvent reset_event_op(CMD_RESETEVENT, *this, cb_context->GetQueueFlags(), event, stageMask);
    return reset_event_op.Validate(*cb_context);
}

void SyncValidator::PostCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    StateTracker::PostCallRecordCmdResetEvent(commandBuffer, event, stageMask);
    auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return;

    cb_context->RecordSyncOp<SyncOpResetEvent>(CMD_RESETEVENT, *this, cb_context->GetQueueFlags(), event, stageMask);
}

bool SyncValidator::PreCallValidateCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                     VkPipelineStageFlags2KHR stageMask) const {
    bool skip = false;
    const auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return skip;

    SyncOpResetEvent reset_event_op(CMD_RESETEVENT2KHR, *this, cb_context->GetQueueFlags(), event, stageMask);
    return reset_event_op.Validate(*cb_context);
}

bool SyncValidator::PreCallValidateCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                                  VkPipelineStageFlags2 stageMask) const {
    bool skip = false;
    const auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return skip;

    SyncOpResetEvent reset_event_op(CMD_RESETEVENT2, *this, cb_context->GetQueueFlags(), event, stageMask);
    return reset_event_op.Validate(*cb_context);
}

void SyncValidator::PostCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                    VkPipelineStageFlags2KHR stageMask) {
    StateTracker::PostCallRecordCmdResetEvent2KHR(commandBuffer, event, stageMask);
    auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return;

    cb_context->RecordSyncOp<SyncOpResetEvent>(CMD_RESETEVENT2KHR, *this, cb_context->GetQueueFlags(), event, stageMask);
}

void SyncValidator::PostCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags2 stageMask) {
    StateTracker::PostCallRecordCmdResetEvent2(commandBuffer, event, stageMask);
    auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return;

    cb_context->RecordSyncOp<SyncOpResetEvent>(CMD_RESETEVENT2, *this, cb_context->GetQueueFlags(), event, stageMask);
}

bool SyncValidator::PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                 VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                 uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                 uint32_t bufferMemoryBarrierCount,
                                                 const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                 uint32_t imageMemoryBarrierCount,
                                                 const VkImageMemoryBarrier *pImageMemoryBarriers) const {
    bool skip = false;
    const auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return skip;

    SyncOpWaitEvents wait_events_op(CMD_WAITEVENTS, *this, cb_context->GetQueueFlags(), eventCount, pEvents, srcStageMask,
                                    dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount,
                                    pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    return wait_events_op.Validate(*cb_context);
}

void SyncValidator::PostCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                uint32_t bufferMemoryBarrierCount,
                                                const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                uint32_t imageMemoryBarrierCount,
                                                const VkImageMemoryBarrier *pImageMemoryBarriers) {
    StateTracker::PostCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount,
                                              pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                                              imageMemoryBarrierCount, pImageMemoryBarriers);

    auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return;

    cb_context->RecordSyncOp<SyncOpWaitEvents>(
        CMD_WAITEVENTS, *this, cb_context->GetQueueFlags(), eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount,
        pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

bool SyncValidator::PreCallValidateCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                     const VkDependencyInfoKHR *pDependencyInfos) const {
    bool skip = false;
    const auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return skip;

    SyncOpWaitEvents wait_events_op(CMD_WAITEVENTS2KHR, *this, cb_context->GetQueueFlags(), eventCount, pEvents, pDependencyInfos);
    skip |= wait_events_op.Validate(*cb_context);
    return skip;
}

void SyncValidator::PostCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                    const VkDependencyInfoKHR *pDependencyInfos) {
    StateTracker::PostCallRecordCmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);

    auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return;

    cb_context->RecordSyncOp<SyncOpWaitEvents>(CMD_WAITEVENTS2KHR, *this, cb_context->GetQueueFlags(), eventCount, pEvents,
                                               pDependencyInfos);
}

bool SyncValidator::PreCallValidateCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                  const VkDependencyInfo *pDependencyInfos) const {
    bool skip = false;
    const auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return skip;

    SyncOpWaitEvents wait_events_op(CMD_WAITEVENTS2, *this, cb_context->GetQueueFlags(), eventCount, pEvents, pDependencyInfos);
    skip |= wait_events_op.Validate(*cb_context);
    return skip;
}

void SyncValidator::PostCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                 const VkDependencyInfo *pDependencyInfos) {
    StateTracker::PostCallRecordCmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);

    auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    if (!cb_context) return;

    cb_context->RecordSyncOp<SyncOpWaitEvents>(CMD_WAITEVENTS2, *this, cb_context->GetQueueFlags(), eventCount, pEvents,
                                               pDependencyInfos);
}

void SyncEventState::ResetFirstScope() {
    for (const auto address_type : kAddressTypes) {
        first_scope[static_cast<size_t>(address_type)].clear();
    }
    scope = SyncExecScope();
    first_scope_set = false;
    first_scope_tag = 0;
}

// Keep the "ignore this event" logic in same place for ValidateWait and RecordWait to use
SyncEventState::IgnoreReason SyncEventState::IsIgnoredByWait(CMD_TYPE cmd, VkPipelineStageFlags2KHR srcStageMask) const {
    IgnoreReason reason = NotIgnored;

    if ((CMD_WAITEVENTS2KHR == cmd || CMD_WAITEVENTS2 == cmd) && (CMD_SETEVENT == last_command)) {
        reason = SetVsWait2;
    } else if ((last_command == CMD_RESETEVENT || last_command == CMD_RESETEVENT2KHR) && !HasBarrier(0U, 0U)) {
        reason = (last_command == CMD_RESETEVENT) ? ResetWaitRace : Reset2WaitRace;
    } else if (unsynchronized_set) {
        reason = SetRace;
    } else if (first_scope_set) {
        const VkPipelineStageFlags2KHR missing_bits = scope.mask_param & ~srcStageMask;
        if (missing_bits) reason = MissingStageBits;
    }

    return reason;
}

bool SyncEventState::HasBarrier(VkPipelineStageFlags2KHR stageMask, VkPipelineStageFlags2KHR exec_scope_arg) const {
    bool has_barrier = (last_command == CMD_NONE) || (stageMask & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) ||
                       (barriers & exec_scope_arg) || (barriers & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    return has_barrier;
}

void SyncOpBase::SetReplayContext(uint32_t subpass, ReplayContextPtr &&replay) {
    subpass_ = subpass;
    replay_context_ = std::move(replay);
}

const ReplayTrackbackBarriersAction *SyncOpBase::GetReplayTrackback() const {
    if (replay_context_) {
        assert(subpass_ < replay_context_->subpass_contexts.size());
        return &replay_context_->subpass_contexts[subpass_];
    }
    return nullptr;
}

SyncOpBarriers::SyncOpBarriers(CMD_TYPE cmd, const SyncValidator &sync_state, VkQueueFlags queue_flags,
                               VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                               VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount,
                               const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                               const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                               const VkImageMemoryBarrier *pImageMemoryBarriers)
    : SyncOpBase(cmd), barriers_(1) {
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

SyncOpBarriers::SyncOpBarriers(CMD_TYPE cmd, const SyncValidator &sync_state, VkQueueFlags queue_flags, uint32_t event_count,
                               const VkDependencyInfoKHR *dep_infos)
    : SyncOpBase(cmd), barriers_(event_count) {
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

SyncOpPipelineBarrier::SyncOpPipelineBarrier(CMD_TYPE cmd, const SyncValidator &sync_state, VkQueueFlags queue_flags,
                                             VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                             VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount,
                                             const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                                             const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                                             const VkImageMemoryBarrier *pImageMemoryBarriers)
    : SyncOpBarriers(cmd, sync_state, queue_flags, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers,
                     bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers) {}

SyncOpPipelineBarrier::SyncOpPipelineBarrier(CMD_TYPE cmd, const SyncValidator &sync_state, VkQueueFlags queue_flags,
                                             const VkDependencyInfoKHR &dep_info)
    : SyncOpBarriers(cmd, sync_state, queue_flags, 1, &dep_info) {}

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
        if (hazard.hazard) {
            // PHASE1 TODO -- add tag information to log msg when useful.
            const auto &sync_state = cb_context.GetSyncState();
            const auto image_handle = image_state->image();
            skip |= sync_state.LogError(image_handle, string_SyncHazardVUID(hazard.hazard),
                                        "%s: Hazard %s for image barrier %" PRIu32 " %s. Access info %s.", CmdName(),
                                        string_SyncHazard(hazard.hazard), image_barrier.index,
                                        sync_state.report_data->FormatHandle(image_handle).c_str(),
                                        cb_context.FormatHazard(hazard).c_str());
        }
    }
    return skip;
}

struct SyncOpPipelineBarrierFunctorFactory {
    using BarrierOpFunctor = PipelineBarrierOp;
    using ApplyFunctor = ApplyBarrierFunctor<BarrierOpFunctor>;
    using GlobalBarrierOpFunctor = PipelineBarrierOp;
    using GlobalApplyFunctor = ApplyBarrierOpsFunctor<GlobalBarrierOpFunctor>;
    using BufferRange = ResourceAccessRange;
    using ImageRange = subresource_adapter::ImageRangeGenerator;
    using GlobalRange = ResourceAccessRange;

    ApplyFunctor MakeApplyFunctor(const SyncBarrier &barrier, bool layout_transition) const {
        return ApplyFunctor(BarrierOpFunctor(barrier, layout_transition));
    }
    GlobalApplyFunctor MakeGlobalApplyFunctor(size_t size_hint, ResourceUsageTag tag) const {
        return GlobalApplyFunctor(true /* resolve */, size_hint, tag);
    }
    GlobalBarrierOpFunctor MakeGlobalBarrierOpFunctor(const SyncBarrier &barrier) const {
        return GlobalBarrierOpFunctor(barrier, false);
    }

    BufferRange MakeRangeGen(const BUFFER_STATE &buffer, const ResourceAccessRange &range) const {
        if (!SimpleBinding(buffer)) return ResourceAccessRange();
        const auto base_address = ResourceBaseAddress(buffer);
        return (range + base_address);
    }
    ImageRange MakeRangeGen(const IMAGE_STATE &image, const VkImageSubresourceRange &subresource_range) const {
        if (!SimpleBinding(image)) return subresource_adapter::ImageRangeGenerator();

        const auto base_address = ResourceBaseAddress(image);
        subresource_adapter::ImageRangeGenerator range_gen(*image.fragment_encoder.get(), subresource_range, base_address);
        return range_gen;
    }
    GlobalRange MakeGlobalRangeGen(AccessAddressType) const { return kFullRange; }
};

template <typename Barriers, typename FunctorFactory>
void SyncOpBarriers::ApplyBarriers(const Barriers &barriers, const FunctorFactory &factory, const ResourceUsageTag tag,
                                   AccessContext *context) {
    for (const auto &barrier : barriers) {
        const auto *state = barrier.GetState();
        if (state) {
            auto *const accesses = &context->GetAccessStateMap(GetAccessAddressType(*state));
            auto update_action = factory.MakeApplyFunctor(barrier.barrier, barrier.IsLayoutTransition());
            auto range_gen = factory.MakeRangeGen(*state, barrier.Range());
            UpdateMemoryAccessState(accesses, update_action, &range_gen);
        }
    }
}

template <typename Barriers, typename FunctorFactory>
void SyncOpBarriers::ApplyGlobalBarriers(const Barriers &barriers, const FunctorFactory &factory, const ResourceUsageTag tag,
                                         AccessContext *access_context) {
    auto barriers_functor = factory.MakeGlobalApplyFunctor(barriers.size(), tag);
    for (const auto &barrier : barriers) {
        barriers_functor.EmplaceBack(factory.MakeGlobalBarrierOpFunctor(barrier));
    }
    for (const auto address_type : kAddressTypes) {
        auto range_gen = factory.MakeGlobalRangeGen(address_type);
        UpdateMemoryAccessState(&(access_context->GetAccessStateMap(address_type)), barriers_functor, &range_gen);
    }
}

ResourceUsageTag SyncOpPipelineBarrier::Record(CommandBufferAccessContext *cb_context) const {
    auto *access_context = cb_context->GetCurrentAccessContext();
    auto *events_context = cb_context->GetCurrentEventsContext();
    const auto tag = cb_context->NextCommandTag(cmd_);
    ReplayRecord(tag, access_context, events_context);
    return tag;
}

void SyncOpPipelineBarrier::ReplayRecord(const ResourceUsageTag tag, AccessContext *access_context,
                                         SyncEventsContext *events_context) const {
    SyncOpPipelineBarrierFunctorFactory factory;
    // Pipeline barriers only have a single barrier set, unlike WaitEvents2
    assert(barriers_.size() == 1);
    const auto &barrier_set = barriers_[0];
    ApplyBarriers(barrier_set.buffer_memory_barriers, factory, tag, access_context);
    ApplyBarriers(barrier_set.image_memory_barriers, factory, tag, access_context);
    ApplyGlobalBarriers(barrier_set.memory_barriers, factory, tag, access_context);
    if (barrier_set.single_exec_scope) {
        events_context->ApplyBarrier(barrier_set.src_exec_scope, barrier_set.dst_exec_scope);
    } else {
        for (const auto &barrier : barrier_set.memory_barriers) {
            events_context->ApplyBarrier(barrier.src_exec_scope, barrier.dst_exec_scope);
        }
    }
}

bool SyncOpPipelineBarrier::ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                                           ResourceUsageTag base_tag, CommandExecutionContext *exec_context) const {
    // No Validation for replay, as the layout transition accesses are checked directly, and the src*Mask ordering is captured
    // with first access information.
    return false;
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
            const auto barrier_size = GetBufferWholeSize(*buffer, barrier.offset, barrier.size);
            const auto range = MakeRange(barrier.offset, barrier_size);
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
            const auto barrier_size = GetBufferWholeSize(*buffer, barrier.offset, barrier.size);
            const auto range = MakeRange(barrier.offset, barrier_size);
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
        auto image = sync_state.Get<IMAGE_STATE>(barrier.image);
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
        auto image = sync_state.Get<IMAGE_STATE>(barrier.image);
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

SyncOpWaitEvents::SyncOpWaitEvents(CMD_TYPE cmd, const SyncValidator &sync_state, VkQueueFlags queue_flags, uint32_t eventCount,
                                   const VkEvent *pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                   uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                   uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                   uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers)
    : SyncOpBarriers(cmd, sync_state, queue_flags, srcStageMask, dstStageMask, VkDependencyFlags(0U), memoryBarrierCount,
                     pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
                     pImageMemoryBarriers) {
    MakeEventsList(sync_state, eventCount, pEvents);
}

SyncOpWaitEvents::SyncOpWaitEvents(CMD_TYPE cmd, const SyncValidator &sync_state, VkQueueFlags queue_flags, uint32_t eventCount,
                                   const VkEvent *pEvents, const VkDependencyInfoKHR *pDependencyInfo)
    : SyncOpBarriers(cmd, sync_state, queue_flags, eventCount, pDependencyInfo) {
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
            if (barrier_set.src_exec_scope.mask_param & VK_PIPELINE_STAGE_HOST_BIT) {
                const std::string vuid = std::string("SYNC-") + std::string(CmdName()) + std::string("-hostevent-unsupported");
                skip = sync_state.LogInfo(command_buffer_handle, vuid,
                                          "%s, srcStageMask includes %s, unsupported by synchronization validation.", CmdName(),
                                          string_VkPipelineStageFlagBits(VK_PIPELINE_STAGE_HOST_BIT));
            } else {
                const auto &barriers = barrier_set.memory_barriers;
                for (size_t barrier_index = 0; barrier_index < barriers.size(); barrier_index++) {
                    const auto &barrier = barriers[barrier_index];
                    if (barrier.src_exec_scope.mask_param & VK_PIPELINE_STAGE_HOST_BIT) {
                        const std::string vuid =
                            std::string("SYNC-") + std::string(CmdName()) + std::string("-hostevent-unsupported");
                        skip =
                            sync_state.LogInfo(command_buffer_handle, vuid,
                                               "%s, srcStageMask %s of %s %zu, %s %zu, unsupported by synchronization validation.",
                                               CmdName(), string_VkPipelineStageFlagBits(VK_PIPELINE_STAGE_HOST_BIT),
                                               "pDependencyInfo", barrier_set_index, "pMemoryBarriers", barrier_index);
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
        if (sync_event->last_command_tag > base_tag) continue;

        const auto event_handle = sync_event->event->event();
        // TODO add "destroyed" checks

        if (sync_event->first_scope_set) {
            // Only accumulate barrier and event stages if there is a pending set in the current context
            barrier_mask_params |= barrier_set.src_exec_scope.mask_param;
            event_stage_masks |= sync_event->scope.mask_param;
        }

        const auto &src_exec_scope = barrier_set.src_exec_scope;

        const auto ignore_reason = sync_event->IsIgnoredByWait(cmd_, src_exec_scope.mask_param);
        if (ignore_reason) {
            switch (ignore_reason) {
                case SyncEventState::ResetWaitRace:
                case SyncEventState::Reset2WaitRace: {
                    // Four permuations of Reset and Wait calls...
                    const char *vuid =
                        (cmd_ == CMD_WAITEVENTS) ? "VUID-vkCmdResetEvent-event-03834" : "VUID-vkCmdResetEvent-event-03835";
                    if (ignore_reason == SyncEventState::Reset2WaitRace) {
                        vuid = (cmd_ == CMD_WAITEVENTS) ? "VUID-vkCmdResetEvent2-event-03831"
                                                        : "VUID-vkCmdResetEvent2-event-03832";
                    }
                    const char *const message =
                        "%s: %s %s operation following %s without intervening execution barrier, may cause race condition. %s";
                    skip |= sync_state.LogError(event_handle, vuid, message, CmdName(),
                                                sync_state.report_data->FormatHandle(event_handle).c_str(), CmdName(),
                                                CommandTypeString(sync_event->last_command), kIgnored);
                    break;
                }
                case SyncEventState::SetRace: {
                    // Issue error message that Wait is waiting on an signal subject to race condition, and is thus ignored for
                    // this event
                    const char *const vuid = "SYNC-vkCmdWaitEvents-unsynchronized-setops";
                    const char *const message =
                        "%s: %s Unsychronized %s calls result in race conditions w.r.t. event signalling, %s %s";
                    const char *const reason = "First synchronization scope is undefined.";
                    skip |= sync_state.LogError(event_handle, vuid, message, CmdName(),
                                                sync_state.report_data->FormatHandle(event_handle).c_str(),
                                                CommandTypeString(sync_event->last_command), reason, kIgnored);
                    break;
                }
                case SyncEventState::MissingStageBits: {
                    const auto missing_bits = sync_event->scope.mask_param & ~src_exec_scope.mask_param;
                    // Issue error message that event waited for is not in wait events scope
                    const char *const vuid = "VUID-vkCmdWaitEvents-srcStageMask-01158";
                    const char *const message = "%s: %s stageMask %" PRIx64 " includes bits not present in srcStageMask 0x%" PRIx64
                                                ". Bits missing from srcStageMask %s. %s";
                    skip |= sync_state.LogError(event_handle, vuid, message, CmdName(),
                                                sync_state.report_data->FormatHandle(event_handle).c_str(),
                                                sync_event->scope.mask_param, src_exec_scope.mask_param,
                                                sync_utils::StringPipelineStageFlags(missing_bits).c_str(), kIgnored);
                    break;
                }
                case SyncEventState::SetVsWait2: {
                    skip |= sync_state.LogError(event_handle, "VUID-vkCmdWaitEvents2-pEvents-03837",
                                                "%s: Follows set of %s by %s. Disallowed.", CmdName(),
                                                sync_state.report_data->FormatHandle(event_handle).c_str(),
                                                CommandTypeString(sync_event->last_command));
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
                const auto hazard =
                    context->DetectImageBarrierHazard(*image_state, sync_event->scope.exec_scope, src_access_scope,
                                                      subresource_range, *sync_event, AccessContext::DetectOptions::kDetectAll);
                if (hazard.hazard) {
                    skip |= sync_state.LogError(image_state->image(), string_SyncHazardVUID(hazard.hazard),
                                                "%s: Hazard %s for image barrier %" PRIu32 " %s. Access info %s.", CmdName(),
                                                string_SyncHazard(hazard.hazard), image_memory_barrier.index,
                                                sync_state.report_data->FormatHandle(image_state->image()).c_str(),
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
        const char *const vuid =
            (CMD_WAITEVENTS == cmd_) ? "VUID-vkCmdWaitEvents-srcStageMask-01158" : "VUID-vkCmdWaitEvents2-pEvents-03838";
        const char *const message =
            "%s: srcStageMask 0x%" PRIx64 " contains stages not present in pEvents stageMask. Extra stages are %s.%s";
        const auto handle = exec_context.Handle();
        if (events_not_found) {
            skip |= sync_state.LogInfo(handle, vuid, message, CmdName(), barrier_mask_params,
                                       sync_utils::StringPipelineStageFlags(extra_stage_bits).c_str(),
                                       " vkCmdSetEvent may be in previously submitted command buffer.");
        } else {
            skip |= sync_state.LogError(handle, vuid, message, CmdName(), barrier_mask_params,
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

    // Need to restrict to only valid exec and access scope for this event
    // Pass by value is intentional to get a copy we can change without modifying the passed barrier
    SyncBarrier RestrictToEvent(SyncBarrier barrier) const {
        barrier.src_exec_scope.exec_scope = sync_event->scope.exec_scope & barrier.src_exec_scope.exec_scope;
        barrier.src_access_scope = sync_event->scope.valid_accesses & barrier.src_access_scope;
        return barrier;
    }
    ApplyFunctor MakeApplyFunctor(const SyncBarrier &barrier_arg, bool layout_transition) const {
        auto barrier = RestrictToEvent(barrier_arg);
        return ApplyFunctor(BarrierOpFunctor(sync_event->first_scope_tag, barrier, layout_transition));
    }
    GlobalApplyFunctor MakeGlobalApplyFunctor(size_t size_hint, ResourceUsageTag tag) const {
        return GlobalApplyFunctor(false /* don't resolve */, size_hint, tag);
    }
    GlobalBarrierOpFunctor MakeGlobalBarrierOpFunctor(const SyncBarrier &barrier_arg) const {
        auto barrier = RestrictToEvent(barrier_arg);
        return GlobalBarrierOpFunctor(sync_event->first_scope_tag, barrier, false);
    }

    BufferRange MakeRangeGen(const BUFFER_STATE &buffer, const ResourceAccessRange &range_arg) const {
        const AccessAddressType address_type = GetAccessAddressType(buffer);
        const auto base_address = ResourceBaseAddress(buffer);
        ResourceAccessRange range = SimpleBinding(buffer) ? (range_arg + base_address) : ResourceAccessRange();
        EventSimpleRangeGenerator filtered_range_gen(sync_event->FirstScope(address_type), range);
        return filtered_range_gen;
    }
    ImageRange MakeRangeGen(const IMAGE_STATE &image, const VkImageSubresourceRange &subresource_range) const {
        if (!SimpleBinding(image)) return ImageRange();
        const auto address_type = GetAccessAddressType(image);
        const auto base_address = ResourceBaseAddress(image);
        subresource_adapter::ImageRangeGenerator image_range_gen(*image.fragment_encoder.get(), subresource_range, base_address);
        EventImageRangeGenerator filtered_range_gen(sync_event->FirstScope(address_type), image_range_gen);

        return filtered_range_gen;
    }
    GlobalRange MakeGlobalRangeGen(AccessAddressType address_type) const {
        return EventSimpleRangeGenerator(sync_event->FirstScope(address_type), kFullRange);
    }
    SyncOpWaitEventsFunctorFactory(SyncEventState *sync_event_) : sync_event(sync_event_) { assert(sync_event); }
    SyncEventState *sync_event;
};

ResourceUsageTag SyncOpWaitEvents::Record(CommandBufferAccessContext *cb_context) const {
    const auto tag = cb_context->NextCommandTag(cmd_);
    auto *access_context = cb_context->GetCurrentAccessContext();
    assert(access_context);
    if (!access_context) return tag;
    auto *events_context = cb_context->GetCurrentEventsContext();
    assert(events_context);
    if (!events_context) return tag;

    ReplayRecord(tag, access_context, events_context);
    return tag;
}

void SyncOpWaitEvents::ReplayRecord(ResourceUsageTag tag, AccessContext *access_context, SyncEventsContext *events_context) const {
    // Unlike PipelineBarrier, WaitEvent is *not* limited to accesses within the current subpass (if any) and thus needs to import
    // all accesses. Can instead import for all first_scopes, or a union of them, if this becomes a performance/memory issue,
    // but with no idea of the performance of the union, nor of whether it even matters... take the simplest approach here,
    access_context->ResolvePreviousAccesses();

    size_t barrier_set_index = 0;
    size_t barrier_set_incr = (barriers_.size() == 1) ? 0 : 1;
    assert(barriers_.size() == 1 || (barriers_.size() == events_.size()));
    for (auto &event_shared : events_) {
        if (!event_shared.get()) continue;
        auto *sync_event = events_context->GetFromShared(event_shared);

        sync_event->last_command = cmd_;
        sync_event->last_command_tag = tag;

        const auto &barrier_set = barriers_[barrier_set_index];
        const auto &dst = barrier_set.dst_exec_scope;
        if (!sync_event->IsIgnoredByWait(cmd_, barrier_set.src_exec_scope.mask_param)) {
            // These apply barriers one at a time as the are restricted to the resource ranges specified per each barrier,
            // but do not update the dependency chain information (but set the "pending" state) // s.t. the order independence
            // of the barriers is maintained.
            SyncOpWaitEventsFunctorFactory factory(sync_event);
            ApplyBarriers(barrier_set.buffer_memory_barriers, factory, tag, access_context);
            ApplyBarriers(barrier_set.image_memory_barriers, factory, tag, access_context);
            ApplyGlobalBarriers(barrier_set.memory_barriers, factory, tag, access_context);

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
    ResolvePendingBarrierFunctor apply_pending_action(tag);
    access_context->ApplyToContext(apply_pending_action);
}

bool SyncOpWaitEvents::ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                                      ResourceUsageTag base_tag, CommandExecutionContext *exec_context) const {
    return DoValidate(*exec_context, base_tag);
}

bool SyncValidator::PreCallValidateCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR pipelineStage,
                                                            VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) const {
    bool skip = false;
    const auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    if (!cb_access_context) return skip;

    const auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);
    if (!context) return skip;

    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, 4);
        auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, range);
        if (hazard.hazard) {
            skip |= LogError(dstBuffer, string_SyncHazardVUID(hazard.hazard),
                             "vkCmdWriteBufferMarkerAMD2: Hazard %s for dstBuffer %s. Access info %s.",
                             string_SyncHazard(hazard.hazard), report_data->FormatHandle(dstBuffer).c_str(),
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

SyncOpResetEvent::SyncOpResetEvent(CMD_TYPE cmd, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkEvent event,
                                   VkPipelineStageFlags2KHR stageMask)
    : SyncOpBase(cmd), event_(sync_state.Get<EVENT_STATE>(event)), exec_scope_(SyncExecScope::MakeSrc(queue_flags, stageMask)) {}

bool SyncOpResetEvent::Validate(const CommandBufferAccessContext& cb_context) const {
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
            case CMD_SETEVENT:
            case CMD_SETEVENT2KHR:
            case CMD_SETEVENT2:
                // Needs a barrier between set and reset
                vuid = "SYNC-vkCmdResetEvent-missingbarrier-set";
                break;
            case CMD_WAITEVENTS:
            case CMD_WAITEVENTS2:
            case CMD_WAITEVENTS2KHR: {
                // Needs to be in the barriers chain (either because of a barrier, or because of dstStageMask
                vuid = "SYNC-vkCmdResetEvent-missingbarrier-wait";
                break;
            }
            default:
                // The only other valid last command that wasn't one.
                assert((sync_event->last_command == CMD_NONE) || (sync_event->last_command == CMD_RESETEVENT) ||
                       (sync_event->last_command == CMD_RESETEVENT2KHR));
                break;
        }
        if (vuid) {
            skip |= sync_state.LogError(event_->event(), vuid, message, CmdName(),
                                        sync_state.report_data->FormatHandle(event_->event()).c_str(), CmdName(),
                                        CommandTypeString(sync_event->last_command));
        }
    }
    return skip;
}

ResourceUsageTag SyncOpResetEvent::Record(CommandBufferAccessContext *cb_context) const {
    const auto tag = cb_context->NextCommandTag(cmd_);
    auto *events_context = cb_context->GetCurrentEventsContext();
    assert(events_context);
    if (!events_context) return tag;

    auto *sync_event = events_context->GetFromShared(event_);
    if (!sync_event) return tag;  // Core, Lifetimes, or Param check needs to catch invalid events.

    // Update the event state
    sync_event->last_command = cmd_;
    sync_event->last_command_tag = tag;
    sync_event->unsynchronized_set = CMD_NONE;
    sync_event->ResetFirstScope();
    sync_event->barriers = 0U;

    return tag;
}

bool SyncOpResetEvent::ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                                      ResourceUsageTag base_tag, CommandExecutionContext *exec_context) const {
    return DoValidate(*exec_context, base_tag);
}

void SyncOpResetEvent::ReplayRecord(ResourceUsageTag tag, AccessContext *access_context, SyncEventsContext *events_context) const {}

SyncOpSetEvent::SyncOpSetEvent(CMD_TYPE cmd, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkEvent event,
                               VkPipelineStageFlags2KHR stageMask)
    : SyncOpBase(cmd),
      event_(sync_state.Get<EVENT_STATE>(event)),
      src_exec_scope_(SyncExecScope::MakeSrc(queue_flags, stageMask)),
      dep_info_() {}

SyncOpSetEvent::SyncOpSetEvent(CMD_TYPE cmd, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkEvent event,
                               const VkDependencyInfoKHR &dep_info)
    : SyncOpBase(cmd),
      event_(sync_state.Get<EVENT_STATE>(event)),
      src_exec_scope_(SyncExecScope::MakeSrc(queue_flags, sync_utils::GetGlobalStageMasks(dep_info).src)),
      dep_info_(new safe_VkDependencyInfo(&dep_info)) {}

bool SyncOpSetEvent::Validate(const CommandBufferAccessContext &cb_context) const {
    return DoValidate(cb_context, ResourceUsageRecord::kMaxIndex);
}
bool SyncOpSetEvent::ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                                    ResourceUsageTag base_tag, CommandExecutionContext *exec_context) const {
    assert(exec_context);
    return DoValidate(*exec_context, base_tag);
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
            case CMD_RESETEVENT:
            case CMD_RESETEVENT2KHR:
            case CMD_RESETEVENT2:
                // Needs a barrier between reset and set
                vuid_stem = "-missingbarrier-reset";
                message = reset_set;
                break;
            case CMD_SETEVENT:
            case CMD_SETEVENT2KHR:
            case CMD_SETEVENT2:
                // Needs a barrier between set and set
                vuid_stem = "-missingbarrier-set";
                message = reset_set;
                break;
            case CMD_WAITEVENTS:
            case CMD_WAITEVENTS2:
            case CMD_WAITEVENTS2KHR:
                // Needs a barrier or is in second execution scope
                vuid_stem = "-missingbarrier-wait";
                message = wait;
                break;
            default:
                // The only other valid last command that wasn't one.
                assert(sync_event->last_command == CMD_NONE);
                break;
        }
        if (vuid_stem) {
            assert(nullptr != message);
            std::string vuid("SYNC-");
            vuid.append(CmdName()).append(vuid_stem);
            skip |= sync_state.LogError(event_->event(), vuid.c_str(), message, CmdName(),
                                        sync_state.report_data->FormatHandle(event_->event()).c_str(), CmdName(),
                                        CommandTypeString(sync_event->last_command));
        }
    }

    return skip;
}

ResourceUsageTag SyncOpSetEvent::Record(CommandBufferAccessContext *cb_context) const {
    const auto tag = cb_context->NextCommandTag(cmd_);
    auto *events_context = cb_context->GetCurrentEventsContext();
    auto *access_context = cb_context->GetCurrentAccessContext();
    assert(events_context);
    if (access_context && events_context) {
        ReplayRecord(tag, access_context, events_context);
    }
    return tag;
}

void SyncOpSetEvent::ReplayRecord(ResourceUsageTag tag, AccessContext *access_context, SyncEventsContext *events_context) const {
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
    } else if (!sync_event->first_scope_set) {
        // We only set the scope if there isn't one
        sync_event->scope = src_exec_scope_;

        auto set_scope = [&sync_event](AccessAddressType address_type, const ResourceAccessRangeMap::value_type &access) {
            auto &scope_map = sync_event->first_scope[static_cast<size_t>(address_type)];
            if (access.second.InSourceScopeOrChain(sync_event->scope.exec_scope, sync_event->scope.valid_accesses)) {
                scope_map.insert(scope_map.end(), std::make_pair(access.first, true));
            }
        };
        access_context->ForAll(set_scope);
        sync_event->unsynchronized_set = CMD_NONE;
        sync_event->first_scope_set = true;
        sync_event->first_scope_tag = tag;
    }
    // TODO: Store dep_info_ shared ptr in sync_state for WaitEvents2 validation
    sync_event->last_command = cmd_;
    sync_event->last_command_tag = tag;
    sync_event->barriers = 0U;
}

SyncOpBeginRenderPass::SyncOpBeginRenderPass(CMD_TYPE cmd, const SyncValidator &sync_state,
                                             const VkRenderPassBeginInfo *pRenderPassBegin,
                                             const VkSubpassBeginInfo *pSubpassBeginInfo)
    : SyncOpBase(cmd) {
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
                attachments_.emplace_back(attachment.get());
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
    skip |= temp_context.ValidateLayoutTransitions(cb_context, rp_state, render_area, subpass, view_gens, CmdName());

    // Validate load operations if there were no layout transition hazards
    if (!skip) {
        temp_context.RecordLayoutTransitions(rp_state, subpass, view_gens, kInvalidTag);
        skip |= temp_context.ValidateLoadOperation(cb_context, rp_state, render_area, subpass, view_gens, CmdName());
    }

    return skip;
}

ResourceUsageTag SyncOpBeginRenderPass::Record(CommandBufferAccessContext *cb_context) const {
    // TODO PHASE2 need to have a consistent way to record to either command buffer or queue contexts
    assert(rp_state_.get());
    if (nullptr == rp_state_.get()) return cb_context->NextCommandTag(cmd_);
    return cb_context->RecordBeginRenderPass(cmd_, *rp_state_.get(), renderpass_begin_info_.renderArea, attachments_);
}

bool SyncOpBeginRenderPass::ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                                           ResourceUsageTag base_tag, CommandExecutionContext *exec_context) const {
    return false;
}

void SyncOpBeginRenderPass::ReplayRecord(ResourceUsageTag tag, AccessContext *access_context,
                                         SyncEventsContext *events_context) const {}

SyncOpNextSubpass::SyncOpNextSubpass(CMD_TYPE cmd, const SyncValidator &sync_state, const VkSubpassBeginInfo *pSubpassBeginInfo,
                                     const VkSubpassEndInfo *pSubpassEndInfo)
    : SyncOpBase(cmd) {
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

    skip |= renderpass_context->ValidateNextSubpass(cb_context.GetExecutionContext(), CmdName());
    return skip;
}

ResourceUsageTag SyncOpNextSubpass::Record(CommandBufferAccessContext *cb_context) const {
    return cb_context->RecordNextSubpass(cmd_);
}

bool SyncOpNextSubpass::ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                                       ResourceUsageTag base_tag, CommandExecutionContext *exec_context) const {
    return false;
}

SyncOpEndRenderPass::SyncOpEndRenderPass(CMD_TYPE cmd, const SyncValidator &sync_state, const VkSubpassEndInfo *pSubpassEndInfo)
    : SyncOpBase(cmd) {
    if (pSubpassEndInfo) {
        subpass_end_info_.initialize(pSubpassEndInfo);
    }
}

void SyncOpNextSubpass::ReplayRecord(ResourceUsageTag tag, AccessContext *access_context, SyncEventsContext *events_context) const {
}

bool SyncOpEndRenderPass::Validate(const CommandBufferAccessContext &cb_context) const {
    bool skip = false;
    const auto *renderpass_context = cb_context.GetCurrentRenderPassContext();

    if (!renderpass_context) return skip;
    skip |= renderpass_context->ValidateEndRenderPass(cb_context.GetExecutionContext(), CmdName());
    return skip;
}

ResourceUsageTag SyncOpEndRenderPass::Record(CommandBufferAccessContext *cb_context) const {
    return cb_context->RecordEndRenderPass(cmd_);
}

bool SyncOpEndRenderPass::ReplayValidate(ResourceUsageTag recorded_tag, const CommandBufferAccessContext &recorded_context,
                                         ResourceUsageTag base_tag, CommandExecutionContext *exec_context) const {
    return false;
}

void SyncOpEndRenderPass::ReplayRecord(ResourceUsageTag tag, AccessContext *access_context,
                                       SyncEventsContext *events_context) const {}

void SyncValidator::PreCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR pipelineStage,
                                                          VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) {
    StateTracker::PreCallRecordCmdWriteBufferMarker2AMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    auto *cb_access_context = GetAccessContext(commandBuffer);
    assert(cb_access_context);
    const auto tag = cb_access_context->NextCommandTag(CMD_WRITEBUFFERMARKERAMD);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto dst_buffer = Get<BUFFER_STATE>(dstBuffer);

    if (dst_buffer) {
        const ResourceAccessRange range = MakeRange(dstOffset, 4);
        context->UpdateAccessState(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment, range, tag);
    }
}

bool SyncValidator::PreCallValidateCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                                      const VkCommandBuffer *pCommandBuffers) const {
    bool skip = StateTracker::PreCallValidateCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
    const char *func_name = "vkCmdExecuteCommands";
    const auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);

    // Heavyweight, but we need a proxy copy of the active command buffer access context
    CommandBufferAccessContext proxy_cb_context(*cb_context, CommandBufferAccessContext::AsProxyContext());

    // Make working copies of the access and events contexts
    for (uint32_t cb_index = 0; cb_index < commandBufferCount; ++cb_index) {
        proxy_cb_context.NextIndexedCommandTag(CMD_EXECUTECOMMANDS, cb_index);

        const auto *recorded_cb_context = GetAccessContext(pCommandBuffers[cb_index]);
        if (!recorded_cb_context) continue;

        const auto *recorded_context = recorded_cb_context->GetCurrentAccessContext();
        assert(recorded_context);
        skip |= recorded_cb_context->ValidateFirstUse(&proxy_cb_context, func_name, cb_index);

        // The barriers have already been applied in ValidatFirstUse
        ResourceUsageRange tag_range = proxy_cb_context.ImportRecordedAccessLog(*recorded_cb_context);
        proxy_cb_context.ResolveExecutedCommandBuffer(*recorded_context, tag_range.begin);
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
                                                    const VkCommandBuffer *pCommandBuffers) {
    StateTracker::PreCallRecordCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
    auto *cb_context = GetAccessContext(commandBuffer);
    assert(cb_context);
    for (uint32_t cb_index = 0; cb_index < commandBufferCount; ++cb_index) {
        cb_context->NextIndexedCommandTag(CMD_EXECUTECOMMANDS, cb_index);
        const auto *recorded_cb_context = GetAccessContext(pCommandBuffers[cb_index]);
        if (!recorded_cb_context) continue;
        cb_context->RecordExecutedCommandBuffer(*recorded_cb_context, CMD_EXECUTECOMMANDS);
    }
}

void SyncValidator::PostCallRecordQueueWaitIdle(VkQueue queue, VkResult result) {
    StateTracker::PostCallRecordQueueWaitIdle(queue, result);
    if ((result != VK_SUCCESS) || (!enabled[sync_validation_queue_submit]) || (queue == VK_NULL_HANDLE)) return;

    const auto queue_state = GetQueueSyncStateShared(queue);
    if (!queue_state) return;  // Invalid queue
    QueueId waited_queue = queue_state->GetQueueId();

    // We need to go through every queue batch context and clear all accesses this wait synchronizes
    // As usual -- two groups, the "last batch" and the signaled semaphores
    // NOTE: Since ApplyTaggedWait crawls through every usage in every ResourceAccessState in the AccessContext of *every*
    // QueueBatchContext, track which we've done to avoid duplicate traversals
    QueueBatchContext::BatchSet waited;
    for (auto &queue : queue_sync_states_) {
        auto batch = queue.second->LastBatch();
        if (batch) {
            batch->ApplyTaggedWait(waited_queue, ResourceUsageRecord::kMaxIndex);
            waited.emplace(batch);
        }
    }

    for (const auto &signaled : signaled_semaphores_) {
        auto &sem_sig = signaled.second;
        if (sem_sig && sem_sig->batch && (waited.find(sem_sig->batch) == waited.end())) {
            sem_sig->batch->ApplyTaggedWait(waited_queue, ResourceUsageRecord::kMaxIndex);
            waited.emplace(sem_sig->batch);
        }
    }
    // TODO: Update Events and Fences affected by Wait
}

void SyncValidator::PostCallRecordDeviceWaitIdle(VkDevice device, VkResult result) {
    StateTracker::PostCallRecordDeviceWaitIdle(device, result);
    for (auto &queue : queue_sync_states_) {
        auto batch = queue.second->LastBatch();
        if (batch) {
            batch->ApplyDeviceWait();
        }
    }

    auto wait_no_list = [](std::shared_ptr<QueueBatchContext> &batch) {
        batch->ApplyDeviceWait();
        return false;
    };

    GetQueueLastBatchSnapshot(wait_no_list);

    // TODO: Update Events and Fences affected by Wait
}

struct QueueSubmitCmdState {
    std::shared_ptr<const QueueSyncState> queue;
    std::shared_ptr<QueueBatchContext> last_batch;
    AccessLogger logger;
    SignaledSemaphores signaled;
    QueueSubmitCmdState(const AccessLogger &parent_log) : logger(&parent_log) {}
};

template <>
thread_local layer_data::optional<QueueSubmitCmdState> layer_data::TlsGuard<QueueSubmitCmdState>::payload_{};

bool SyncValidator::PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits,
                                               VkFence fence) const {
    bool skip = false;

    // Since this early return is above the TlsGuard, the Record phase must also be.
    if (!enabled[sync_validation_queue_submit]) return skip;

    layer_data::TlsGuard<QueueSubmitCmdState> cmd_state(&skip, global_access_log_);
    const auto fence_state = Get<FENCE_STATE>(fence);
    cmd_state->queue = GetQueueSyncStateShared(queue);
    if (!cmd_state->queue) return skip;  // Invalid Queue

    const char *func_name = "vkQueueSubmit";

    // The submit id is a mutable automic which is not recoverable on a skip == true condition
    uint64_t submit_id = cmd_state->queue->ReserveSubmitId();

    // verify each submit batch
    // Since the last batch from the queue state is const, we need to track the last_batch separately from the
    // most recently created batch
    std::shared_ptr<const QueueBatchContext> last_batch = cmd_state->queue->LastBatch();
    std::shared_ptr<QueueBatchContext> batch;
    for (uint32_t batch_idx = 0; batch_idx < submitCount; batch_idx++) {
        const VkSubmitInfo &submit = pSubmits[batch_idx];
        batch = std::make_shared<QueueBatchContext>(*this, *cmd_state->queue);
        batch->Setup(last_batch, submit, cmd_state->signaled);
        batch->SetBatchLog(cmd_state->logger, submit_id, batch_idx);

        //  For each submit in the batch...
        for (const auto &cb : *batch) {
            skip |= cb.cb->ValidateFirstUse(batch.get(), func_name, cb.index);

            // The barriers have already been applied in ValidatFirstUse
            ResourceUsageRange tag_range = batch->ImportRecordedAccessLog(*cb.cb);
            batch->ResolveSubmittedCommandBuffer(*cb.cb->GetCurrentAccessContext(), tag_range.begin);
        }

        for (auto &sem : layer_data::make_span(submit.pSignalSemaphores, submit.signalSemaphoreCount)) {
            // Make a copy of the state, signal the copy and pend it...
            auto sem_state = Get<SEMAPHORE_STATE>(sem);
            if (!sem_state) continue;
            auto semaphore_info = lvl_init_struct<VkSemaphoreSubmitInfo>();
            semaphore_info.stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
            cmd_state->signaled.SignalSemaphore(sem_state, batch, semaphore_info);
        }
        // Unless the previous batch was referenced by a signal, the QueueBatchContext will self destruct, but as
        // we ResolvePrevious as we can let any contexts we've fully referenced go.
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
                                              VkResult result) {
    StateTracker::PostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, result);

    // If this return is above the TlsGuard, then the Validate phase return must also be.
    if (!enabled[sync_validation_queue_submit]) return;  // Queue submit validation must be affirmatively enabled

    // The earliest return (when enabled), must be *after* the TlsGuard, as it is the TlsGuard that cleans up the cmd_state
    // static payload
    layer_data::TlsGuard<QueueSubmitCmdState> cmd_state;

    if (VK_SUCCESS != result) return;  // dispatched QueueSubmit failed
    if (!cmd_state->queue) return;  // Validation couldn't find a valid queue object

    // Don't need to look up the queue state again, but we need a non-const version
    std::shared_ptr<QueueSyncState> queue_state = std::const_pointer_cast<QueueSyncState>(std::move(cmd_state->queue));

    // The global  the semaphores we applied to the cmd_state QueueBatchContexts
    // NOTE: All conserved QueueBatchContext's need to have there access logs reset to use the global logger and the only conserved
    //       QBC's are those referenced by unwaited signals and the last batch.
    for (auto &sig_sem : cmd_state->signaled) {
        if (sig_sem.second && sig_sem.second->batch) {
            sig_sem.second->batch->ResetAccessLog();
        }
        signaled_semaphores_.Import(sig_sem.first, std::move(sig_sem.second));
    }
    cmd_state->signaled.Reset();

    // Update the queue to point to the last batch from the submit
    if (cmd_state->last_batch) {
        cmd_state->last_batch->ResetAccessLog();
        queue_state->SetLastBatch(std::move(cmd_state->last_batch));
    }

    // Update the global access log from the one built during validation
    global_access_log_.MergeMove(std::move(cmd_state->logger));


    // WIP: record information about fences
}

bool SyncValidator::PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits,
                                                   VkFence fence) const {
    bool skip = false;
    if (!enabled[sync_validation_queue_submit]) return skip;

    // WIP: Add Submit2 support
    return skip;
}

void SyncValidator::PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits,
                                                  VkFence fence, VkResult result) {
    StateTracker::PostCallRecordQueueSubmit2KHR(queue, submitCount, pSubmits, fence, result);
    if (VK_SUCCESS != result) return;  // dispatched QueueSubmit2 failed

    if (!enabled[sync_validation_queue_submit]) return;

    // WIP: Add Submit2 support
}

AttachmentViewGen::AttachmentViewGen(const IMAGE_VIEW_STATE *view, const VkOffset3D &offset, const VkExtent3D &extent)
    : view_(view), view_mask_(), gen_store_() {
    if (!view_ || !view_->image_state || !SimpleBinding(*view_->image_state)) return;
    const IMAGE_STATE &image_state = *view_->image_state.get();
    const auto base_address = ResourceBaseAddress(image_state);
    const auto *encoder = image_state.fragment_encoder.get();
    if (!encoder) return;
    // Get offset and extent for the view, accounting for possible depth slicing
    const VkOffset3D zero_offset = view->GetOffset();
    const VkExtent3D &image_extent = view->GetExtent();
    // Intentional copy
    VkImageSubresourceRange subres_range = view_->normalized_subresource_range;
    view_mask_ = subres_range.aspectMask;
    gen_store_[Gen::kViewSubresource].emplace(*encoder, subres_range, zero_offset, image_extent, base_address);
    gen_store_[Gen::kRenderArea].emplace(*encoder, subres_range, offset, extent, base_address);

    const auto depth = view_mask_ & VK_IMAGE_ASPECT_DEPTH_BIT;
    if (depth && (depth != view_mask_)) {
        subres_range.aspectMask = depth;
        gen_store_[Gen::kDepthOnlyRenderArea].emplace(*encoder, subres_range, offset, extent, base_address);
    }
    const auto stencil = view_mask_ & VK_IMAGE_ASPECT_STENCIL_BIT;
    if (stencil && (stencil != view_mask_)) {
        subres_range.aspectMask = stencil;
        gen_store_[Gen::kStencilOnlyRenderArea].emplace(*encoder, subres_range, offset, extent, base_address);
    }
}

const ImageRangeGen *AttachmentViewGen::GetRangeGen(AttachmentViewGen::Gen gen_type) const {
    const ImageRangeGen *got = nullptr;
    switch (gen_type) {
        case kViewSubresource:
            got = &gen_store_[kViewSubresource];
            break;
        case kRenderArea:
            got = &gen_store_[kRenderArea];
            break;
        case kDepthOnlyRenderArea:
            got =
                (view_mask_ == VK_IMAGE_ASPECT_DEPTH_BIT) ? &gen_store_[Gen::kRenderArea] : &gen_store_[Gen::kDepthOnlyRenderArea];
            break;
        case kStencilOnlyRenderArea:
            got = (view_mask_ == VK_IMAGE_ASPECT_STENCIL_BIT) ? &gen_store_[Gen::kRenderArea]
                                                              : &gen_store_[Gen::kStencilOnlyRenderArea];
            break;
        default:
            assert(got);
    }
    return got;
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

AccessAddressType AttachmentViewGen::GetAddressType() const { return AccessContext::ImageAddressType(*view_->image_state); }

void SyncEventsContext::ApplyBarrier(const SyncExecScope &src, const SyncExecScope &dst) {
    const bool all_commands_bit = 0 != (src.mask_param & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    for (auto &event_pair : map_) {
        assert(event_pair.second);  // Shouldn't be storing empty
        auto &sync_event = *event_pair.second;
        // Events don't happen at a stage, so we need to check and store the unexpanded ALL_COMMANDS if set for inter-event-calls
        if ((sync_event.barriers & src.exec_scope) || all_commands_bit) {
            sync_event.barriers |= dst.exec_scope;
            sync_event.barriers |= dst.mask_param & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        }
    }
}

ReplayTrackbackBarriersAction::ReplayTrackbackBarriersAction(VkQueueFlags queue_flags,
                                                             const SubpassDependencyGraphNode &subpass_dep,
                                                             const std::vector<ReplayTrackbackBarriersAction> &replay_contexts) {
    bool has_barrier_from_external = subpass_dep.barrier_from_external.size() > 0U;
    trackback_barriers.reserve(subpass_dep.prev.size() + (has_barrier_from_external ? 1U : 0U));
    for (const auto &prev_dep : subpass_dep.prev) {
        const auto prev_pass = prev_dep.first->pass;
        const auto &prev_barriers = prev_dep.second;
        trackback_barriers.emplace_back(&replay_contexts[prev_pass], queue_flags, prev_barriers);
    }
    if (has_barrier_from_external) {
        // Store the barrier from external with the reat, but save pointer for "by subpass" lookups.
        trackback_barriers.emplace_back(nullptr, queue_flags, subpass_dep.barrier_from_external);
    }
}

void ReplayTrackbackBarriersAction::operator()(ResourceAccessState *access) const {
    if (trackback_barriers.size() == 1) {
        trackback_barriers[0](access);
    } else {
        ResourceAccessState resolved;
        for (const auto &trackback : trackback_barriers) {
            ResourceAccessState access_copy = *access;
            trackback(&access_copy);
            resolved.Resolve(access_copy);
        }
        *access = resolved;
    }
}

ReplayTrackbackBarriersAction::TrackbackBarriers::TrackbackBarriers(
    const ReplayTrackbackBarriersAction *source_subpass_, VkQueueFlags queue_flags_,
    const std::vector<const VkSubpassDependency2 *> &subpass_dependencies_)
    : Base(source_subpass_, queue_flags_, subpass_dependencies_) {}

void ReplayTrackbackBarriersAction::TrackbackBarriers::operator()(ResourceAccessState *access) const {
    if (source_subpass) {
        (*source_subpass)(access);
    }
    access->ApplyBarriersImmediate(barriers);
}

QueueBatchContext::QueueBatchContext(const SyncValidator &sync_state, const QueueSyncState &queue_state)
    : CommandExecutionContext(&sync_state), queue_state_(&queue_state), tag_range_(0, 0), batch_log_(nullptr) {}

template <typename BatchInfo>
void QueueBatchContext::Setup(const std::shared_ptr<const QueueBatchContext> &prev_batch, const BatchInfo &batch_info,
                              SignaledSemaphores &signaled) {
    SetupCommandBufferInfo(batch_info);
    SetupAccessContext(prev_batch, batch_info, signaled);
}
void QueueBatchContext::ResolveSubmittedCommandBuffer(const AccessContext &recorded_context, ResourceUsageTag offset) {
    QueueId queue_id = queue_state_->GetQueueId();
    auto tag_queue_op = [offset, queue_id](ResourceAccessState *access) {
        access->OffsetTag(offset);
        access->SetQueueId(queue_id);
    };
    GetCurrentAccessContext()->ResolveFromContext(tag_queue_op, recorded_context);
}

VulkanTypedHandle QueueBatchContext::Handle() const { return queue_state_->Handle(); }

void QueueBatchContext::ApplyTaggedWait(QueueId queue_id, ResourceUsageTag tag) {
    QueueWaitWorm wait_worm(queue_id);
    access_context_.ForAll(wait_worm);
    if (wait_worm.erase_all) {
        access_context_.Reset();
    } else {
        // TODO: Profiling will tell us if we need a more efficient clean up.
        for (const auto &address : wait_worm.erase_list) {
            access_context_.DeleteAccess(address);
        }
    }
}

// Clear all accesses
void QueueBatchContext::ApplyDeviceWait() { access_context_.Reset(); }

void QueueBatchContext::WaitOneSemaphore(VkSemaphore sem, VkPipelineStageFlags2 wait_mask, WaitBatchMap &batch_trackbacks,
                                         SignaledSemaphores &signaled) {
    auto sem_state = sync_state_->Get<SEMAPHORE_STATE>(sem);
    if (!sem_state) return;  // Semaphore validity is handled by CoreChecks

    // When signal state goes out of scope, the signal information will be dropped, as Unsignal has released ownership.
    auto signal_state = signaled.Unsignal(sem);
    if (!signal_state) return;  // Invalid signal, skip it.

    const auto &sem_batch = signal_state->batch;
    assert(sem_batch);

    const AccessContext *sem_context = sem_batch->GetCurrentAccessContext();

    using TrackBackPtr = AccessContext::TrackBack *;
    const auto trackback_insert = batch_trackbacks.emplace(sem_batch.get(), TrackBackPtr());
    const bool inserted = trackback_insert.second;
    const auto trackback_it = trackback_insert.first;

    const SyncExecScope &sem_scope = signal_state->exec_scope;
    const auto queue_flags = queue_state_->GetQueueFlags();
    SyncExecScope dst_mask = SyncExecScope::MakeDst(queue_flags, wait_mask);
    const SyncBarrier sem_barrier(sem_scope, sem_scope.valid_accesses, dst_mask, SyncStageAccessFlags());
    if (inserted) {
        // If this is the first time we referenced this QueueBatchContext
        trackback_it->second = access_context_.AddTrackBack(sem_context, sem_barrier);
    }
    assert(trackback_it->second);
    trackback_it->second->barriers.emplace_back(sem_barrier);
}

// Accessor Traits to allow Submit and Submit2 constructors to call the same utilities
template <>
class QueueBatchContext::SubmitInfoAccessor<VkSubmitInfo> {
  public:
    SubmitInfoAccessor(const VkSubmitInfo &info) : info_(info) {}
    inline uint32_t WaitSemaphoreCount() const { return info_.waitSemaphoreCount; }
    inline VkSemaphore WaitSemaphore(uint32_t index) { return info_.pWaitSemaphores[index]; }
    inline VkPipelineStageFlags2 WaitDstMask(uint32_t index) { return info_.pWaitDstStageMask[index]; }
    inline uint32_t CommandBufferCount() const { return info_.commandBufferCount; }
    inline VkCommandBuffer CommandBuffer(uint32_t index) { return info_.pCommandBuffers[index]; }

  private:
    const VkSubmitInfo &info_;
};
template <typename BatchInfo, typename Fn>
void QueueBatchContext::ForEachWaitSemaphore(const BatchInfo &batch_info, Fn &&func) {
    using Accessor = QueueBatchContext::SubmitInfoAccessor<BatchInfo>;
    Accessor batch(batch_info);
    const uint32_t wait_count = batch.WaitSemaphoreCount();
    for (uint32_t i = 0; i < wait_count; ++i) {
        func(batch.WaitSemaphore(i), batch.WaitDstMask(i));
    }
}

template <typename BatchInfo>
void QueueBatchContext::SetupAccessContext(const std::shared_ptr<const QueueBatchContext> &prev, const BatchInfo &batch_info,
                                           SignaledSemaphores &signaled) {
    // Create trackbacks for the access context for this batch based on the semaphores and the previous batch in this
    // queue.
    WaitBatchMap batch_trackbacks;
    ForEachWaitSemaphore(batch_info, [this, &batch_trackbacks, &signaled](VkSemaphore sem, VkPipelineStageFlags2 wait_mask) {
        WaitOneSemaphore(sem, wait_mask, batch_trackbacks, signaled);
    });

    // If there are no semaphores to the previous batch, make sure a "submit order" empty trackback is added
    if (prev && (batch_trackbacks.find(prev.get()) == batch_trackbacks.end())) {
        access_context_.AddTrackBack(prev->GetCurrentAccessContext(), SyncBarrier());
    }

    // Flatten all previous contexts into the current one (for dependency chaining reasons)
    access_context_.ResolvePreviousAccesses();
    access_context_.ClearTrackBacks();

    // Gather async context information for hazard checks and conserve the QBC's for the async batches
    const auto end_it = batch_trackbacks.end();
    async_batches_ = sync_state_->GetQueueLastBatchSnapshot(
        [&batch_trackbacks, end_it, &prev](const std::shared_ptr<const QueueBatchContext> &batch) {
            const auto found_it = batch_trackbacks.find(batch.get());
            return found_it == end_it && (batch != prev);
        });
    for (const auto &async_batch : async_batches_) {
        access_context_.AddAsyncContext(async_batch->GetCurrentAccessContext());
    }
}

template <typename BatchInfo>
void QueueBatchContext::SetupCommandBufferInfo(const BatchInfo &batch_info) {
    using Accessor = QueueBatchContext::SubmitInfoAccessor<BatchInfo>;
    Accessor batch(batch_info);

    // Create the list of command buffers to submit
    const uint32_t cb_count = batch.CommandBufferCount();
    command_buffers_.reserve(cb_count);
    for (uint32_t index = 0; index < cb_count; ++index) {
        auto cb_context = sync_state_->GetAccessContextShared(batch.CommandBuffer(index));
        if (cb_context) {
            tag_range_.end += cb_context->GetTagLimit();
            command_buffers_.emplace_back(index, std::move(cb_context));
        }
    }
}

// Look up the usage informaiton from the local or global logger
std::string QueueBatchContext::FormatUsage(ResourceUsageTag tag) const {
    const AccessLogger &use_logger = (logger_) ? *logger_ : sync_state_->global_access_log_;
    std::stringstream out;
    AccessLogger::AccessRecord access = use_logger[tag];
    if (access.IsValid()) {
        const AccessLogger::BatchRecord &batch = *access.batch;
        const ResourceUsageRecord &record = *access.record;
        // Queue and Batch information
        out << SyncNodeFormatter(*sync_state_, batch.queue->GetQueueState());
        out << ", submit: " << batch.submit_index << ", batch: " << batch.batch_index;

        // Commandbuffer Usages Information
        out << record;
        out << SyncNodeFormatter(*sync_state_, record.cb_state);
        out << ", reset_no: " << std::to_string(record.reset_count);
    }
    return out.str();
}

VkQueueFlags QueueBatchContext::GetQueueFlags() const { return queue_state_->GetQueueFlags(); }

void QueueBatchContext::SetBatchLog(AccessLogger &logger, uint64_t submit_id, uint32_t batch_id) {
    // Need new global tags for all accesses... the Reserve updates a mutable atomic
    ResourceUsageRange global_tags = sync_state_->ReserveGlobalTagRange(GetTagRange().size());
    SetTagBias(global_tags.begin);
    // Add an access log for the batches range and point the batch at it.
    logger_ = &logger;
    batch_log_ = logger.AddBatch(queue_state_, submit_id, batch_id, global_tags);
}

void QueueBatchContext::InsertRecordedAccessLogEntries(const CommandBufferAccessContext &submitted_cb) {
    assert(batch_log_);  // Don't import command buffer contexts until you've set up the log for the batch context
    batch_log_->Append(submitted_cb.GetAccessLog());
}

void QueueBatchContext::SetTagBias(ResourceUsageTag bias) {
    const auto size = tag_range_.size();
    tag_range_.begin = bias;
    tag_range_.end = bias + size;
    access_context_.SetStartTag(bias);
}

QueueBatchContext::QueueWaitWorm::QueueWaitWorm(QueueId queue, ResourceUsageTag tag) : predicate(queue) {}

void QueueBatchContext::QueueWaitWorm::operator()(AccessAddressType address_type, ResourceAccessRangeMap::value_type &access) {
    bool erased = access.second.ApplyQueueTagWait(predicate);
    if (erased) {
        erase_list.emplace_back(address_type, access.first);
    } else {
        erase_all = false;
    }
}

AccessLogger::BatchLog *AccessLogger::AddBatch(const QueueSyncState *queue_state, uint64_t submit_id, uint32_t batch_id,
                                               const ResourceUsageRange &range) {
    const auto inserted = access_log_map_.insert(std::make_pair(range, BatchLog(BatchRecord(queue_state, submit_id, batch_id))));
    assert(inserted.second);
    return &inserted.first->second;
}

void AccessLogger::MergeMove(AccessLogger &&child) {
    for (auto &range : child.access_log_map_) {
        BatchLog &child_batch = range.second;
        auto insert_pair = access_log_map_.insert(std::make_pair(range.first, BatchLog()));
        insert_pair.first->second = std::move(child_batch);
        assert(insert_pair.second);
    }
    child.Reset();
}

void AccessLogger::Reset() {
    prev_ = nullptr;
    access_log_map_.clear();
}

// Since we're updating the QueueSync state, this is Record phase and the access log needs to point to the global one
// Batch Contexts saved during signalling have their AccessLog reset when the pending signals are signalled.
// NOTE: By design, QueueBatchContexts that are neither last, nor referenced by a signal are abandoned as unowned, since
//       the contexts Resolve all history from previous all contexts when created
void QueueSyncState::SetLastBatch(std::shared_ptr<QueueBatchContext> &&last) {
    last_batch_ = std::move(last);
    last_batch_->ResetAccessLog();
}

// Note that function is const, but updates mutable submit_index to allow Validate to create correct tagging for command invocation
// scope state.
// Given that queue submits are supposed to be externally synchronized for the same queue, this should safe without being
// atomic... but as the ops are per submit, the performance cost is negible for the peace of mind.
uint64_t QueueSyncState::ReserveSubmitId() const { return submit_index_.fetch_add(1); }

void AccessLogger::BatchLog::Append(const CommandExecutionContext::AccessLog &other) {
    log_.insert(log_.end(), other.cbegin(), other.cend());
    for (const auto &record : other) {
        assert(record.cb_state);
        cbs_referenced_.insert(record.cb_state->shared_from_this());
    }
}

AccessLogger::AccessRecord AccessLogger::BatchLog::operator[](size_t index) const {
    assert(index < log_.size());
    return AccessRecord{&batch_, &log_[index]};
}

AccessLogger::AccessRecord AccessLogger::operator[](ResourceUsageTag tag) const {
    AccessRecord access_record = {nullptr, nullptr};

    auto found_range = access_log_map_.find(tag);
    if (found_range != access_log_map_.cend()) {
        const ResourceUsageTag bias = found_range->first.begin;
        assert(tag >= bias);
        access_record = found_range->second[tag - bias];
    } else if (prev_) {
        access_record = (*prev_)[tag];
    }

    return access_record;
}

std::shared_ptr<SignaledSemaphores::Signal> SignaledSemaphores::GetPrev(VkSemaphore sem) const {
    std::shared_ptr<Signal> prev_state;
    if (prev_) {
        prev_state = GetMapped(prev_->signaled_, sem, [&prev_state]() { return prev_state; });
    }
    return prev_state;
}
