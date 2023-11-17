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
#include "sync/sync_access_context.h"
#include "sync/sync_sync_op.h"
#include "sync/sync_utils.h"
#include "sync/sync_renderpass.h"

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
    SyncNodeFormatter(const SyncValidator &sync_state, const vvl::Queue *q_state)
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

CommandBufferAccessContext::CommandBufferAccessContext(const SyncValidator *sync_validator)
    : CommandExecutionContext(sync_validator),
      cb_state_(),
      access_log_(std::make_shared<AccessLog>()),
      cbs_referenced_(std::make_shared<CommandBufferSet>()),
      command_number_(0),
      subcommand_number_(0),
      reset_count_(0),
      cb_access_context_(),
      current_context_(&cb_access_context_),
      events_context_(),
      render_pass_contexts_(),
      current_renderpass_context_(),
      sync_ops_() {}

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
    cb_access_context_.ResolveFromContext(*from_context);
    // The proxy has flatten the current render pass context (if any), but the async contexts are needed for hazard detection
    cb_access_context_.ImportAsyncContexts(*from_context);

    events_context_ = from.events_context_;

    // We don't want to copy the full render_pass_context_ history just for the proxy.
}

void CommandBufferAccessContext::Reset() {
    access_log_ = std::make_shared<AccessLog>();
    cbs_referenced_ = std::make_shared<CommandBufferSet>();
    if (cb_state_) {
        cbs_referenced_->insert(cb_state_->shared_from_this());
    }
    sync_ops_.clear();
    command_number_ = 0;
    subcommand_number_ = 0;
    reset_count_++;
    command_handles_.clear();
    cb_access_context_.Reset();
    render_pass_contexts_.clear();
    current_context_ = &cb_access_context_;
    current_renderpass_context_ = nullptr;
    events_context_.Clear();
    dynamic_rendering_info_.reset();
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

std::string SyncValidationInfo::FormatHazard(const HazardResult &hazard) const {
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

// Range generators for to allow event scope filtration to be limited to the top of the resource access traversal pipeline
//
// Note: there is no "begin/end" or reset facility.  These are each written as "one time through" generators.
//
// Usage:
//  Constructor() -- initializes the generator to point to the begin of the space declared.
//  *  -- the current range of the generator empty signfies end
//  ++ -- advance to the next non-empty range (or end)

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
using EventSimpleRangeGenerator = MapRangesRangeGenerator<AccessContext::ScopeMap>;

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

using EventImageRangeGenerator = FilteredGeneratorGenerator<AccessContext::ScopeMap, subresource_adapter::ImageRangeGenerator>;

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

// Store operation validation can ignore resolve (before it) and layout tranistions after it.  The first is ignored
// because of the ordering guarantees w.r.t. sample access and that the resolve validation hasn't altered the state, because
// store is part of the same Next/End operation.
// The latter is handled in layout transistion validation directly
bool CommandBufferAccessContext::ValidateBeginRendering(const ErrorObject &error_obj,
                                                        syncval_state::BeginRenderingCmdState &cmd_state) const {
    bool skip = false;
    const syncval_state::DynamicRenderingInfo &info = cmd_state.GetRenderingInfo();

    // Load operations do not happen when resuming
    if (info.info.flags & VK_RENDERING_RESUMING_BIT) return skip;

    HazardResult hazard;

    // Need to hazard detect load operations vs. the attachment views
    const uint32_t attachment_count = static_cast<uint32_t>(info.attachments.size());
    for (uint32_t i = 0; i < attachment_count; i++) {
        const auto &attachment = info.attachments[i];
        const SyncStageAccessIndex load_index = attachment.GetLoadUsage();
        if (load_index == SYNC_ACCESS_INDEX_NONE) continue;

        hazard = GetCurrentAccessContext()->DetectHazard(attachment.view_gen, load_index, attachment.GetOrdering());
        if (hazard.IsHazard()) {
            LogObjectList obj_list(cb_state_->Handle(), attachment.view->Handle());
            Location loc = attachment.GetLocation(error_obj.location, i);
            skip |= sync_state_->LogError(string_SyncHazardVUID(hazard.Hazard()), obj_list, loc.dot(vvl::Field::imageView),
                                          "(%s), with loadOp %s. Access info %s.",
                                          sync_state_->FormatHandle(attachment.view->Handle()).c_str(),
                                          string_VkAttachmentLoadOp(attachment.info.loadOp), FormatHazard(hazard).c_str());
            if (skip) break;
        }
    }
    return skip;
}

void CommandBufferAccessContext::RecordBeginRendering(syncval_state::BeginRenderingCmdState &cmd_state,
                                                      const RecordObject &record_obj) {
    using Attachment = syncval_state::DynamicRenderingInfo::Attachment;
    const syncval_state::DynamicRenderingInfo &info = cmd_state.GetRenderingInfo();
    const auto tag = NextCommandTag(record_obj.location.function);

    // Only load if not resuming
    if (0 == (info.info.flags & VK_RENDERING_RESUMING_BIT)) {
        const uint32_t attachment_count = static_cast<uint32_t>(info.attachments.size());
        for (uint32_t i = 0; i < attachment_count; i++) {
            const Attachment &attachment = info.attachments[i];
            const SyncStageAccessIndex load_index = attachment.GetLoadUsage();
            if (load_index == SYNC_ACCESS_INDEX_NONE) continue;

            GetCurrentAccessContext()->UpdateAccessState(attachment.view_gen, load_index, attachment.GetOrdering(), tag);
        }
    }

    dynamic_rendering_info_ = std::move(cmd_state.info);
}
bool CommandBufferAccessContext::ValidateEndRendering(const ErrorObject &error_obj) const {
    bool skip = false;
    if (dynamic_rendering_info_ && (0 == (dynamic_rendering_info_->info.flags & VK_RENDERING_SUSPENDING_BIT))) {
        // Only validate resolve and store if not suspending (as specified by BeginRendering)
        const syncval_state::DynamicRenderingInfo &info = *dynamic_rendering_info_;
        const uint32_t attachment_count = static_cast<uint32_t>(info.attachments.size());
        const AccessContext *access_context = GetCurrentAccessContext();
        assert(access_context);
        auto report_resolve_hazard = [this](const HazardResult &hazard, const Location &loc, const VulkanTypedHandle image_handle,
                                            const VkResolveModeFlagBits resolve_mode) {
            LogObjectList obj_list(cb_state_->Handle(), image_handle);
            return sync_state_->LogError(string_SyncHazardVUID(hazard.Hazard()), obj_list, loc,
                                         "(%s), during resolve with resolveMode %s. Access info %s.",
                                         sync_state_->FormatHandle(image_handle).c_str(),
                                         string_VkResolveModeFlagBits(resolve_mode), FormatHazard(hazard).c_str());
        };

        for (uint32_t i = 0; i < attachment_count && !skip; i++) {
            const auto &attachment = info.attachments[i];
            if (attachment.resolve_gen) {
                // The logic about whether to resolve is embedded in the Attachment constructor
                assert(attachment.view);
                HazardResult hazard = access_context->DetectHazard(attachment.view_gen, kResolveRead, kResolveOrder);

                if (hazard.IsHazard()) {
                    Location loc = attachment.GetLocation(error_obj.location, i);
                    skip |= report_resolve_hazard(hazard, loc.dot(vvl::Field::imageView), attachment.view->Handle(),
                                                  attachment.info.resolveMode);
                }
                if (!skip) {
                    hazard = access_context->DetectHazard(*attachment.resolve_gen, kResolveWrite, kResolveOrder);
                    if (hazard.IsHazard()) {
                        Location loc = attachment.GetLocation(error_obj.location, i);
                        skip |= report_resolve_hazard(hazard, loc.dot(vvl::Field::resolveImageView),
                                                      attachment.resolve_view->Handle(), attachment.info.resolveMode);
                    }
                }
            }

            const auto store_usage = attachment.GetStoreUsage();
            if (store_usage != SYNC_ACCESS_INDEX_NONE) {
                HazardResult hazard = access_context->DetectHazard(attachment.view_gen, store_usage, kStoreOrder);
                if (hazard.IsHazard()) {
                    const VulkanTypedHandle image_handle = attachment.view->Handle();
                    LogObjectList obj_list(cb_state_->Handle(), image_handle);
                    Location loc = attachment.GetLocation(error_obj.location, i);
                    skip |= sync_state_->LogError(
                        string_SyncHazardVUID(hazard.Hazard()), obj_list, loc.dot(vvl::Field::imageView),
                        "(%s), during store with storeOp %s. Access info %s.", sync_state_->FormatHandle(image_handle).c_str(),
                        string_VkAttachmentStoreOp(attachment.info.storeOp), FormatHazard(hazard).c_str());
                }
            }
        }
    }
    return skip;
}

void CommandBufferAccessContext::RecordEndRendering(const RecordObject &record_obj) {
    if (dynamic_rendering_info_ && (0 == (dynamic_rendering_info_->info.flags & VK_RENDERING_SUSPENDING_BIT))) {
        auto store_tag = NextCommandTag(record_obj.location.function, ResourceUsageRecord::SubcommandType::kStoreOp);

        const syncval_state::DynamicRenderingInfo &info = *dynamic_rendering_info_;
        const uint32_t attachment_count = static_cast<uint32_t>(info.attachments.size());
        AccessContext *access_context = GetCurrentAccessContext();
        for (uint32_t i = 0; i < attachment_count; i++) {
            const auto &attachment = info.attachments[i];
            if (attachment.resolve_gen) {
                access_context->UpdateAccessState(attachment.view_gen, kResolveRead, kResolveOrder, store_tag);
                access_context->UpdateAccessState(*attachment.resolve_gen, kResolveWrite, kResolveOrder, store_tag);
            }

            const SyncStageAccessIndex store_index = attachment.GetStoreUsage();
            if (store_index == SYNC_ACCESS_INDEX_NONE) continue;
            access_context->UpdateAccessState(attachment.view_gen, store_index, kStoreOrder, store_tag);
        }
    }

    dynamic_rendering_info_.reset();
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

    using DescriptorClass = vvl::DescriptorClass;
    using BufferDescriptor = vvl::BufferDescriptor;
    using ImageDescriptor = vvl::ImageDescriptor;
    using TexelDescriptor = vvl::TexelDescriptor;

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
                                current_context_->DetectHazard(*img_view_state, offset, extent, sync_index, SyncOrdering::kRaster);
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
                                sync_state_->FormatHandle(descriptor_set->Handle()).c_str(),
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
                                sync_state_->FormatHandle(descriptor_set->Handle()).c_str(),
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
                                sync_state_->FormatHandle(descriptor_set->Handle()).c_str(),
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

    using DescriptorClass = vvl::DescriptorClass;
    using BufferDescriptor = vvl::BufferDescriptor;
    using ImageDescriptor = vvl::ImageDescriptor;
    using TexelDescriptor = vvl::TexelDescriptor;

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

bool CommandBufferAccessContext::ValidateDrawAttachment(const Location &loc) const {
    bool skip = false;
    if (current_renderpass_context_) {
        skip |= current_renderpass_context_->ValidateDrawSubpassAttachment(GetExecutionContext(), *cb_state_, loc.function);
    } else if (dynamic_rendering_info_) {
        skip |= ValidateDrawDynamicRenderingAttachment(loc);
    }
    return skip;
}

bool CommandBufferAccessContext::ValidateDrawDynamicRenderingAttachment(const Location &location) const {
    bool skip = false;
    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    const auto &last_bound_state = cb_state_->lastBound[lv_bind_point];
    const auto *pipe = last_bound_state.pipeline_state;
    if (!pipe) {
        return skip;
    }

    const auto raster_state = pipe->RasterizationState();
    if (raster_state && raster_state->rasterizerDiscardEnable) {
        return skip;
    }

    const auto &list = pipe->fragmentShader_writable_output_location_list;
    const auto &access_context = *GetCurrentAccessContext();

    const syncval_state::DynamicRenderingInfo &info = *dynamic_rendering_info_;
    for (const auto output_location : list) {
        if (output_location >= info.info.colorAttachmentCount) continue;
        const auto &attachment = info.attachments[output_location];
        if (!attachment.IsWriteable(last_bound_state)) continue;

        HazardResult hazard = access_context.DetectHazard(attachment.view_gen, SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE,
                                                          SyncOrdering::kColorAttachment);
        if (hazard.IsHazard()) {
            LogObjectList obj_list(cb_state_->Handle(), attachment.view->Handle());
            Location loc = attachment.GetLocation(location, output_location);
            skip |= sync_state_->LogError(string_SyncHazardVUID(hazard.Hazard()), obj_list, loc.dot(vvl::Field::imageView),
                                          "(%s). Access info %s.", sync_state_->FormatHandle(attachment.view->Handle()).c_str(),
                                          FormatHazard(hazard).c_str());
        }
    }

    // TODO -- fixup this and Subpass attachment to correct map the various depth stencil enables/reads vs. writes
    // PHASE1 TODO: Add layout based read/vs. write selection.
    // PHASE1 TODO: Read operations for both depth and stencil are possible in the future.
    // PHASE1 TODO: Add EARLY stage detection based on ExecutionMode.

    const uint32_t attachment_count = static_cast<uint32_t>(info.attachments.size());
    for (uint32_t i = info.info.colorAttachmentCount; i < attachment_count; i++) {
        const auto &attachment = info.attachments[i];
        bool writeable = attachment.IsWriteable(last_bound_state);

        if (writeable) {
            HazardResult hazard =
                access_context.DetectHazard(attachment.view_gen, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                            SyncOrdering::kDepthStencilAttachment);
            // Depth stencil Hazard check
            if (hazard.IsHazard()) {
                LogObjectList obj_list(cb_state_->Handle(), attachment.view->Handle());
                Location loc = attachment.GetLocation(location);
                skip |= sync_state_->LogError(string_SyncHazardVUID(hazard.Hazard()), obj_list, loc.dot(vvl::Field::imageView),
                                              "(%s). Access info %s.", sync_state_->FormatHandle(attachment.view->Handle()).c_str(),
                                              FormatHazard(hazard).c_str());
            }
        }
    }

    return skip;
}

void CommandBufferAccessContext::RecordDrawAttachment(const ResourceUsageTag tag) {
    if (current_renderpass_context_) {
        current_renderpass_context_->RecordDrawSubpassAttachment(*cb_state_, tag);
    } else if (dynamic_rendering_info_) {
        RecordDrawDynamicRenderingAttachment(tag);
    }
}

void CommandBufferAccessContext::RecordDrawDynamicRenderingAttachment(ResourceUsageTag tag) {
    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    const auto &last_bound_state = cb_state_->lastBound[lv_bind_point];
    const auto *pipe = last_bound_state.pipeline_state;
    if (!pipe) return;

    const auto raster_state = pipe->RasterizationState();
    if (raster_state && raster_state->rasterizerDiscardEnable) return;

    const auto &list = pipe->fragmentShader_writable_output_location_list;
    auto &access_context = *GetCurrentAccessContext();

    const syncval_state::DynamicRenderingInfo &info = *dynamic_rendering_info_;
    for (const auto output_location : list) {
        if (output_location >= info.info.colorAttachmentCount) continue;
        const auto &attachment = info.attachments[output_location];
        if (!attachment.IsWriteable(last_bound_state)) continue;

        access_context.UpdateAccessState(attachment.view_gen, SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE,
                                         SyncOrdering::kColorAttachment, tag);
    }

    // TODO -- fixup this and Subpass attachment to correct map the various depth stencil enables/reads vs. writes
    // PHASE1 TODO: Add layout based read/vs. write selection.
    // PHASE1 TODO: Read operations for both depth and stencil are possible in the future.
    // PHASE1 TODO: Add EARLY stage detection based on ExecutionMode.

    const uint32_t attachment_count = static_cast<uint32_t>(info.attachments.size());
    for (uint32_t i = info.info.colorAttachmentCount; i < attachment_count; i++) {
        const auto &attachment = info.attachments[i];
        bool writeable = attachment.IsWriteable(last_bound_state);

        if (writeable) {
            access_context.UpdateAccessState(attachment.view_gen, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                             SyncOrdering::kDepthStencilAttachment, tag);
        }
    }
}

ClearAttachmentInfo CommandBufferAccessContext::GetClearAttachmentInfo(const VkClearAttachment &clear_attachment,
                                                                       const VkClearRect &rect) const {
    // This is a NOOP if there's no renderpass nor dynamic rendering
    // Caller must used "IsValid" to determine if clear_info contains meaningful information.
    ClearAttachmentInfo clear_info;
    if (current_renderpass_context_) {
        clear_info = current_renderpass_context_->GetClearAttachmentInfo(clear_attachment, rect);
    } else if (dynamic_rendering_info_) {
        clear_info = dynamic_rendering_info_->GetClearAttachmentInfo(clear_attachment, rect);
    }

    return clear_info;
}
bool CommandBufferAccessContext::ValidateClearAttachment(const Location &loc, const VkClearAttachment &clear_attachment,
                                                         const VkClearRect &rect) const {
    bool skip = false;

    ClearAttachmentInfo clear_info = GetClearAttachmentInfo(clear_attachment, rect);
    if (clear_info.IsValid()) {
        skip = ValidateClearAttachment(loc, clear_info);
    }

    return skip;
}

void CommandBufferAccessContext::RecordClearAttachment(ResourceUsageTag tag, const VkClearAttachment &clear_attachment,
                                                       const VkClearRect &rect) {
    ClearAttachmentInfo clear_info = GetClearAttachmentInfo(clear_attachment, rect);
    if (clear_info.IsValid()) {
        RecordClearAttachment(tag, clear_info);
    }
}

QueueId CommandBufferAccessContext::GetQueueId() const { return kQueueIdInvalid; }

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

bool CommandBufferAccessContext::ValidateClearAttachment(const Location &loc, const ClearAttachmentInfo &info) const {
    bool skip = false;
    VkImageSubresourceRange subresource_range = info.subresource_range;
    const AccessContext *access_context = GetCurrentAccessContext();
    assert(access_context);
    if (info.aspects_to_clear & kColorAspects) {
        assert(GetBitSetCount(info.aspects_to_clear) == 1);
        subresource_range.aspectMask = info.aspects_to_clear;

        HazardResult hazard = access_context->DetectHazard(
            *info.view->GetImageState(), subresource_range, info.offset, info.extent, info.view->IsDepthSliced(),
            SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, SyncOrdering::kColorAttachment);
        if (hazard.IsHazard()) {
            const LogObjectList objlist(cb_state_->Handle(), info.view->Handle());
            skip |= sync_state_->LogError(string_SyncHazardVUID(hazard.Hazard()), objlist, loc,
                                          "Hazard %s while clearing color attachment%s. Access info %s.",
                                          string_SyncHazard(hazard.Hazard()), info.GetSubpassAttachmentText().c_str(),
                                          FormatHazard(hazard).c_str());
        }
    }

    constexpr VkImageAspectFlagBits depth_stencil_aspects[2] = {VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_ASPECT_STENCIL_BIT};
    for (const auto aspect : depth_stencil_aspects) {
        if (info.aspects_to_clear & aspect) {
            // Original aspect mask can contain both stencil and depth but here we track each aspect separately
            subresource_range.aspectMask = aspect;

            // vkCmdClearAttachments depth/stencil writes are executed by the EARLY_FRAGMENT_TESTS_BIT and LATE_FRAGMENT_TESTS_BIT
            // stages. The implementation tracks the most recent access, which happens in the LATE_FRAGMENT_TESTS_BIT stage.
            HazardResult hazard = access_context->DetectHazard(
                *info.view->GetImageState(), info.subresource_range, info.offset, info.extent, info.view->IsDepthSliced(),
                SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, SyncOrdering::kDepthStencilAttachment);

            if (hazard.IsHazard()) {
                const LogObjectList objlist(cb_state_->Handle(), info.view->Handle());
                skip |= sync_state_->LogError(string_SyncHazardVUID(hazard.Hazard()), objlist, loc,
                                              "Hazard %s when clearing %s aspect of depth-stencil attachment%s. Access info %s.",
                                              string_SyncHazard(hazard.Hazard()), string_VkImageAspectFlagBits(aspect),
                                              info.GetSubpassAttachmentText().c_str(), FormatHazard(hazard).c_str());
            }
        }
    }
    return skip;
}

void CommandBufferAccessContext::RecordClearAttachment(ResourceUsageTag tag, const ClearAttachmentInfo &clear_info) {
    auto subresource_range = clear_info.subresource_range;

    // Original subresource range can include aspects that are not cleared, they should not be tracked
    subresource_range.aspectMask = clear_info.aspects_to_clear;
    AccessContext *access_context = GetCurrentAccessContext();

    if (clear_info.aspects_to_clear & kColorAspects) {
        assert((clear_info.aspects_to_clear & kDepthStencilAspects) == 0);
        access_context->UpdateAccessState(*clear_info.view->GetImageState(), SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE,
                                          SyncOrdering::kColorAttachment, subresource_range, clear_info.offset, clear_info.extent,
                                          tag);
    } else {
        assert((clear_info.aspects_to_clear & kColorAspects) == 0);
        access_context->UpdateAccessState(
            *clear_info.view->GetImageState(), SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
            SyncOrdering::kDepthStencilAttachment, subresource_range, clear_info.offset, clear_info.extent, tag);
    }
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
    std::shared_ptr<const vvl::Fence> fence_state = Get<vvl::Fence>(fence);
    UpdateFenceWaitInfo(fence_state, FenceSyncState(fence_state, queue_id, tag));
}
void SyncValidator::UpdateFenceWaitInfo(VkFence fence, const PresentedImage &image, ResourceUsageTag tag) {
    std::shared_ptr<const vvl::Fence> fence_state = Get<vvl::Fence>(fence);
    UpdateFenceWaitInfo(fence_state, FenceSyncState(fence_state, image, tag));
}

void SyncValidator::UpdateFenceWaitInfo(std::shared_ptr<const vvl::Fence> &fence_state, FenceSyncState &&wait_info) {
    if (BASE_NODE::Invalid(fence_state)) return;
    waitable_fences_[fence_state->VkHandle()] = std::move(wait_info);
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

bool SignaledSemaphores::SignalSemaphore(const std::shared_ptr<const vvl::Semaphore> &sem_state,
                                         const std::shared_ptr<QueueBatchContext> &batch,
                                         const VkSemaphoreSubmitInfo &signal_info) {
    assert(batch);
    const SyncExecScope exec_scope =
        SyncExecScope::MakeSrc(batch->GetQueueFlags(), signal_info.stageMask, VK_PIPELINE_STAGE_2_HOST_BIT);
    std::shared_ptr<Signal> signal = std::make_shared<Signal>(sem_state, batch, exec_scope);
    return Insert(sem_state, std::move(signal));
}

bool SignaledSemaphores::Insert(const std::shared_ptr<const vvl::Semaphore> &sem_state, std::shared_ptr<Signal> &&signal) {
    const VkSemaphore sem = sem_state->VkHandle();
    auto signal_it = signaled_.find(sem);
    std::shared_ptr<Signal> insert_signal;
    if (signal_it == signaled_.end()) {
        if (prev_) {
            auto prev_sig = GetMapped(prev_->signaled_, sem, []() { return std::shared_ptr<Signal>(); });
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

bool SignaledSemaphores::SignalSemaphore(const std::shared_ptr<const vvl::Semaphore> &sem_state, const PresentedImage &presented,
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
        assert(sem == from->sem_state->VkHandle());
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
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), srcBuffer, error_obj.location,
                             "Hazard %s for srcBuffer %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(srcBuffer).c_str(), region, cb_context->FormatHazard(hazard).c_str());
            }
        }
        if (dst_buffer && !skip) {
            const ResourceAccessRange dst_range = MakeRange(*dst_buffer, copy_region.dstOffset, copy_region.size);
            auto hazard = context->DetectHazard(*dst_buffer, SYNC_COPY_TRANSFER_WRITE, dst_range);
            if (hazard.IsHazard()) {
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), dstBuffer, error_obj.location,
                             "Hazard %s for dstBuffer %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(dstBuffer).c_str(), region, cb_context->FormatHazard(hazard).c_str());
            }
        }
        if (skip) break;
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                               uint32_t regionCount, const VkBufferCopy *pRegions, const RecordObject &record_obj) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;
    const auto tag = cb_context->NextCommandTag(record_obj.location.function);
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

void SyncValidator::PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo,
                                                   const RecordObject &record_obj) {
    RecordCmdCopyBuffer2(commandBuffer, pCopyBufferInfo, record_obj.location.function);
}

void SyncValidator::PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo,
                                                const RecordObject &record_obj) {
    RecordCmdCopyBuffer2(commandBuffer, pCopyBufferInfo, record_obj.location.function);
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
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(copy_region.srcSubresource), copy_region.srcOffset,
                                                copy_region.extent, false, SYNC_COPY_TRANSFER_READ);
            if (hazard.IsHazard()) {
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), srcImage, error_obj.location,
                             "Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(srcImage).c_str(), region, cb_access_context->FormatHazard(hazard).c_str());
            }
        }

        if (dst_image) {
            auto hazard = context->DetectHazard(*dst_image, RangeFromLayers(copy_region.dstSubresource), copy_region.dstOffset,
                                                copy_region.extent, false, SYNC_COPY_TRANSFER_WRITE);
            if (hazard.IsHazard()) {
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), dstImage, error_obj.location,
                             "Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(dstImage).c_str(), region, cb_access_context->FormatHazard(hazard).c_str());
            }
            if (skip) break;
        }
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                              const VkImageCopy *pRegions, const RecordObject &record_obj) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<ImageState>(srcImage);
    auto dst_image = Get<ImageState>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &copy_region = pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_COPY_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(copy_region.srcSubresource), copy_region.srcOffset, copy_region.extent, tag);
        }
        if (dst_image) {
            context->UpdateAccessState(*dst_image, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(copy_region.dstSubresource), copy_region.dstOffset, copy_region.extent, tag);
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
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(copy_region.srcSubresource), copy_region.srcOffset,
                                                copy_region.extent, false, SYNC_COPY_TRANSFER_READ);
            if (hazard.IsHazard()) {
                skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), pCopyImageInfo->srcImage, error_obj.location,
                                 "Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.Hazard()), FormatHandle(pCopyImageInfo->srcImage).c_str(), region,
                                 cb_access_context->FormatHazard(hazard).c_str());
            }
        }

        if (dst_image) {
            auto hazard = context->DetectHazard(*dst_image, RangeFromLayers(copy_region.dstSubresource), copy_region.dstOffset,
                                                copy_region.extent, false, SYNC_COPY_TRANSFER_WRITE);
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
                                       RangeFromLayers(copy_region.srcSubresource), copy_region.srcOffset, copy_region.extent, tag);
        }
        if (dst_image) {
            context->UpdateAccessState(*dst_image, SYNC_COPY_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(copy_region.dstSubresource), copy_region.dstOffset, copy_region.extent, tag);
        }
    }
}

void SyncValidator::PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo,
                                                  const RecordObject &record_obj) {
    RecordCmdCopyImage2(commandBuffer, pCopyImageInfo, record_obj.location.function);
}

void SyncValidator::PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo,
                                               const RecordObject &record_obj) {
    RecordCmdCopyImage2(commandBuffer, pCopyImageInfo, record_obj.location.function);
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

void SyncValidator::PreCallRecordCmdPipelineBarrier(
    VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier *pImageMemoryBarriers, const RecordObject &record_obj) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;

    cb_access_context->RecordSyncOp<SyncOpPipelineBarrier>(record_obj.location.function, *this, cb_access_context->GetQueueFlags(),
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

void SyncValidator::PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer, const VkDependencyInfoKHR *pDependencyInfo,
                                                        const RecordObject &record_obj) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;

    cb_access_context->RecordSyncOp<SyncOpPipelineBarrier>(record_obj.location.function, *this, cb_access_context->GetQueueFlags(),
                                                           *pDependencyInfo);
}

void SyncValidator::PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer, const VkDependencyInfo *pDependencyInfo,
                                                     const RecordObject &record_obj) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;

    cb_access_context->RecordSyncOp<SyncOpPipelineBarrier>(record_obj.location.function, *this, cb_access_context->GetQueueFlags(),
                                                           *pDependencyInfo);
}

void SyncValidator::CreateDevice(const VkDeviceCreateInfo *pCreateInfo) {
    // The state tracker sets up the device state
    StateTracker::CreateDevice(pCreateInfo);

    ForEachShared<vvl::Queue>([this](const std::shared_ptr<vvl::Queue> &queue_state) {
        auto queue_flags = physical_device_state->queue_family_properties[queue_state->queueFamilyIndex].queueFlags;
        std::shared_ptr<QueueSyncState> queue_sync_state =
            std::make_shared<QueueSyncState>(queue_state, queue_flags, queue_id_limit_++);
        queue_sync_states_.emplace(std::make_pair(queue_state->VkHandle(), std::move(queue_sync_state)));
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
    PostCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo, record_obj);
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
    PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo, record_obj);
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
    PostCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo, record_obj);
}

bool SyncValidator::PreCallValidateCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfoKHR *pRenderingInfo,
                                                        const ErrorObject &error_obj) const {
    return PreCallValidateCmdBeginRendering(commandBuffer, pRenderingInfo, error_obj);
}

bool SyncValidator::PreCallValidateCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo,
                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state || !pRenderingInfo) return skip;

    vvl::TlsGuard<syncval_state::BeginRenderingCmdState> cmd_state(&skip, std::move(cb_state));
    cmd_state->AddRenderingInfo(*this, *pRenderingInfo);

    // We need to set skip, because the TlsGuard destructor is looking at the skip value for RAII cleanup.
    skip = cmd_state->cb_state->access_context.ValidateBeginRendering(error_obj, *cmd_state);
    return skip;
}

void SyncValidator::PreCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer, const VkRenderingInfoKHR *pRenderingInfo,
                                                      const RecordObject &record_obj) {
    PreCallRecordCmdBeginRendering(commandBuffer, pRenderingInfo, record_obj);
}

void SyncValidator::PreCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo,
                                                   const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdBeginRendering(commandBuffer, pRenderingInfo, record_obj);
    vvl::TlsGuard<syncval_state::BeginRenderingCmdState> cmd_state;

    assert(cmd_state && cmd_state->cb_state && (cmd_state->cb_state->commandBuffer() == commandBuffer));
    // Note: for fine grain locking need to to something other than cast.
    auto cb_state = std::const_pointer_cast<syncval_state::CommandBuffer>(cmd_state->cb_state);
    cb_state->access_context.RecordBeginRendering(*cmd_state, record_obj);
}

bool SyncValidator::PreCallValidateCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const {
    return PreCallValidateCmdEndRendering(commandBuffer, error_obj);
}

bool SyncValidator::PreCallValidateCmdEndRendering(VkCommandBuffer commandBuffer, const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;

    skip = cb_state->access_context.ValidateEndRendering(error_obj);
    return skip;
}

void SyncValidator::PreCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    PreCallRecordCmdEndRendering(commandBuffer, record_obj);
}

void SyncValidator::PreCallRecordCmdEndRendering(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdEndRendering(commandBuffer, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;

    cb_state->access_context.RecordEndRendering(record_obj);
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

            hazard = context->DetectHazard(*dst_image, RangeFromLayers(copy_region.imageSubresource), copy_region.imageOffset,
                                           copy_region.imageExtent, false, SYNC_COPY_TRANSFER_WRITE);
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
                                       RangeFromLayers(copy_region.imageSubresource), copy_region.imageOffset,
                                       copy_region.imageExtent, tag);
        }
    }
}

void SyncValidator::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                      VkImageLayout dstImageLayout, uint32_t regionCount,
                                                      const VkBufferImageCopy *pRegions, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions,
                                                    record_obj);
    RecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions,
                               record_obj.location.function);
}

void SyncValidator::PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                          const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo,
                                                          const RecordObject &record_obj) {
    PreCallRecordCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo, record_obj);
}

void SyncValidator::PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                       const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo,
                                                       const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo, record_obj);
    RecordCmdCopyBufferToImage(commandBuffer, pCopyBufferToImageInfo->srcBuffer, pCopyBufferToImageInfo->dstImage,
                               pCopyBufferToImageInfo->dstImageLayout, pCopyBufferToImageInfo->regionCount,
                               pCopyBufferToImageInfo->pRegions, record_obj.location.function);
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
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(copy_region.imageSubresource), copy_region.imageOffset,
                                                copy_region.imageExtent, false, SYNC_COPY_TRANSFER_READ);
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
                                       RangeFromLayers(copy_region.imageSubresource), copy_region.imageOffset,
                                       copy_region.imageExtent, tag);
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
                                                      VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions,
                                                      const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions,
                                                    record_obj);
    RecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions,
                               record_obj.location.function);
}

void SyncValidator::PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                          const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo,
                                                          const RecordObject &record_obj) {
    PreCallRecordCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo, record_obj);
}

void SyncValidator::PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                       const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo,
                                                       const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo, record_obj);
    RecordCmdCopyImageToBuffer(commandBuffer, pCopyImageToBufferInfo->srcImage, pCopyImageToBufferInfo->srcImageLayout,
                               pCopyImageToBufferInfo->dstBuffer, pCopyImageToBufferInfo->regionCount,
                               pCopyImageToBufferInfo->pRegions, record_obj.location.function);
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
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(blit_region.srcSubresource), offset, extent, false,
                                                SYNC_BLIT_TRANSFER_READ);
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
            auto hazard = context->DetectHazard(*dst_image, RangeFromLayers(blit_region.dstSubresource), offset, extent, false,
                                                SYNC_BLIT_TRANSFER_WRITE);
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
                                       RangeFromLayers(blit_region.srcSubresource), offset, extent, tag);
        }
        if (dst_image) {
            VkOffset3D offset = {std::min(blit_region.dstOffsets[0].x, blit_region.dstOffsets[1].x),
                                 std::min(blit_region.dstOffsets[0].y, blit_region.dstOffsets[1].y),
                                 std::min(blit_region.dstOffsets[0].z, blit_region.dstOffsets[1].z)};
            VkExtent3D extent = {static_cast<uint32_t>(abs(blit_region.dstOffsets[1].x - blit_region.dstOffsets[0].x)),
                                 static_cast<uint32_t>(abs(blit_region.dstOffsets[1].y - blit_region.dstOffsets[0].y)),
                                 static_cast<uint32_t>(abs(blit_region.dstOffsets[1].z - blit_region.dstOffsets[0].z))};
            context->UpdateAccessState(*dst_image, SYNC_BLIT_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(blit_region.dstSubresource), offset, extent, tag);
        }
    }
}

void SyncValidator::PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                              const VkImageBlit *pRegions, VkFilter filter, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                            pRegions, filter, record_obj);
    RecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter,
                       record_obj.location.function);
}

void SyncValidator::PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo,
                                                  const RecordObject &record_obj) {
    PreCallRecordCmdBlitImage2(commandBuffer, pBlitImageInfo, record_obj);
}

void SyncValidator::PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo,
                                               const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdBlitImage2(commandBuffer, pBlitImageInfo, record_obj);
    RecordCmdBlitImage(commandBuffer, pBlitImageInfo->srcImage, pBlitImageInfo->srcImageLayout, pBlitImageInfo->dstImage,
                       pBlitImageInfo->dstImageLayout, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
                       pBlitImageInfo->filter, record_obj.location.function);
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

void SyncValidator::PreCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                             const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdDispatch(commandBuffer, x, y, z, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);

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

void SyncValidator::PreCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                     const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
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
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
    return skip;
}

void SyncValidator::PreCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                         uint32_t firstVertex, uint32_t firstInstance, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawVertex(vertexCount, firstVertex, tag);
    cb_access_context->RecordDrawAttachment(tag);
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
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
    return skip;
}

void SyncValidator::PreCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance,
                                                const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance,
                                              record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawVertexIndex(indexCount, firstIndex, tag);
    cb_access_context->RecordDrawAttachment(tag);
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
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, commandBuffer, sizeof(VkDrawIndirectCommand), buffer, offset,
                                   drawCount, stride, error_obj.location);

    // TODO: For now, we validate the whole vertex buffer. It might cause some false positive.
    //       VkDrawIndirectCommand buffer could be changed until SubmitQueue.
    //       We will validate the vertex buffer in SubmitQueue in the future.
    skip |= cb_access_context->ValidateDrawVertex(std::optional<uint32_t>(), 0, error_obj.location);
    return skip;
}

void SyncValidator::PreCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                 uint32_t drawCount, uint32_t stride, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride, record_obj);
    if (drawCount == 0) return;
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawAttachment(tag);
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
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
    skip |= ValidateIndirectBuffer(*cb_access_context, *context, commandBuffer, sizeof(VkDrawIndexedIndirectCommand), buffer,
                                   offset, drawCount, stride, error_obj.location);

    // TODO: For now, we validate the whole index and vertex buffer. It might cause some false positive.
    //       VkDrawIndexedIndirectCommand buffer could be changed until SubmitQueue.
    //       We will validate the index and vertex buffer in SubmitQueue in the future.
    skip |= cb_access_context->ValidateDrawVertexIndex(std::optional<uint32_t>(), 0, error_obj.location);
    return skip;
}

void SyncValidator::PreCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        uint32_t drawCount, uint32_t stride, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    cb_access_context->RecordDispatchDrawDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, tag);
    cb_access_context->RecordDrawAttachment(tag);
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
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
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
    cb_access_context->RecordDrawAttachment(tag);
    RecordIndirectBuffer(*context, tag, sizeof(VkDrawIndirectCommand), buffer, offset, 1, stride);
    RecordCountBuffer(*context, tag, countBuffer, countBufferOffset);

    // TODO: For now, we record the whole vertex buffer. It might cause some false positive.
    //       VkDrawIndirectCommand buffer could be changed until SubmitQueue.
    //       We will record the vertex buffer in SubmitQueue in the future.
    cb_access_context->RecordDrawVertex(std::optional<uint32_t>(), 0, tag);
}

void SyncValidator::PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                      VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                      uint32_t stride, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount,
                                                    stride, record_obj);
    RecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                               record_obj.location.function);
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
                                                         uint32_t maxDrawCount, uint32_t stride, const RecordObject &record_obj) {
    PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      record_obj);
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
                                                         uint32_t maxDrawCount, uint32_t stride, const RecordObject &record_obj) {
    PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      record_obj);
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
    skip |= cb_access_context->ValidateDrawAttachment(error_obj.location);
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
    cb_access_context->RecordDrawAttachment(tag);
    RecordIndirectBuffer(*context, tag, sizeof(VkDrawIndexedIndirectCommand), buffer, offset, 1, stride);
    RecordCountBuffer(*context, tag, countBuffer, countBufferOffset);

    // TODO: For now, we record the whole index and vertex buffer. It might cause some false positive.
    //       VkDrawIndexedIndirectCommand buffer could be changed until SubmitQueue.
    //       We will update the index and vertex buffer in SubmitQueue in the future.
    cb_access_context->RecordDrawVertexIndex(std::optional<uint32_t>(), 0, tag);
}

void SyncValidator::PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                             uint32_t maxDrawCount, uint32_t stride,
                                                             const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset,
                                                           maxDrawCount, stride, record_obj);
    RecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      record_obj.location.function);
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
                                                                uint32_t maxDrawCount, uint32_t stride,
                                                                const RecordObject &record_obj) {
    PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                             record_obj);
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
                                                                uint32_t maxDrawCount, uint32_t stride,
                                                                const RecordObject &record_obj) {
    PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                             record_obj);
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
                skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), image, error_obj.location,
                                 "Hazard %s for %s, range index %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                                 FormatHandle(image).c_str(), index, cb_access_context->FormatHazard(hazard).c_str());
            }
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                    const VkClearColorValue *pColor, uint32_t rangeCount,
                                                    const VkImageSubresourceRange *pRanges, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
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
                skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), image, error_obj.location,
                                 "Hazard %s for %s, range index %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                                 FormatHandle(image).c_str(), index, cb_access_context->FormatHazard(hazard).c_str());
            }
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
                                                           const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount,
                                                           const VkImageSubresourceRange *pRanges, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges,
                                                         record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
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
    bool skip = false;

    const auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return skip;

    for (const auto [attachment_index, attachment] : vvl::enumerate(pAttachments, attachmentCount)) {
        Location attachment_loc = error_obj.location.dot(Field::pAttachments, attachment_index);
        for (const auto [rect_index, rect] : vvl::enumerate(pRects, rectCount)) {
            Location rect_loc = attachment_loc.dot(Field::pRects, rect_index);
            skip |= cb_state->access_context.ValidateClearAttachment(rect_loc, *attachment, *rect);
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                     const VkClearAttachment *pAttachments, uint32_t rectCount,
                                                     const VkClearRect *pRects, const RecordObject &record_obj) {
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    auto cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);

    for (const auto &attachment : vvl::make_span(pAttachments, attachmentCount)) {
        for (const auto &rect : vvl::make_span(pRects, rectCount)) {
            cb_access_context->RecordClearAttachment(tag, attachment, rect);
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
            skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), dstBuffer, error_obj.location,
                             "Hazard %s for dstBuffer %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(dstBuffer).c_str(), cb_access_context->FormatHazard(hazard).c_str());
        }
    }

    // TODO:Track VkQueryPool
    return skip;
}

void SyncValidator::PreCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                                                         uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                         VkDeviceSize stride, VkQueryResultFlags flags,
                                                         const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset,
                                                       stride, flags, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
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
            skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), dstBuffer, error_obj.location,
                             "Hazard %s for dstBuffer %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(dstBuffer).c_str(), cb_access_context->FormatHazard(hazard).c_str());
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                               VkDeviceSize size, uint32_t data, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
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
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(resolve_region.srcSubresource),
                                                resolve_region.srcOffset, resolve_region.extent, false, SYNC_RESOLVE_TRANSFER_READ);
            if (hazard.IsHazard()) {
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), srcImage, error_obj.location,
                             "Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(srcImage).c_str(), region, cb_access_context->FormatHazard(hazard).c_str());
            }
        }

        if (dst_image) {
            auto hazard =
                context->DetectHazard(*dst_image, RangeFromLayers(resolve_region.dstSubresource), resolve_region.dstOffset,
                                      resolve_region.extent, false, SYNC_RESOLVE_TRANSFER_WRITE);
            if (hazard.IsHazard()) {
                skip |=
                    LogError(string_SyncHazardVUID(hazard.Hazard()), dstImage, error_obj.location,
                             "Hazard %s for dstImage %s, region %" PRIu32 ". Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(dstImage).c_str(), region, cb_access_context->FormatHazard(hazard).c_str());
            }
            if (skip) break;
        }
    }

    return skip;
}

void SyncValidator::PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                 VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                 const VkImageResolve *pRegions, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                               pRegions, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<ImageState>(srcImage);
    auto dst_image = Get<ImageState>(dstImage);

    for (uint32_t region = 0; region < regionCount; region++) {
        const auto &resolve_region = pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_RESOLVE_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(resolve_region.srcSubresource), resolve_region.srcOffset,
                                       resolve_region.extent, tag);
        }
        if (dst_image) {
            context->UpdateAccessState(*dst_image, SYNC_RESOLVE_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(resolve_region.dstSubresource), resolve_region.dstOffset,
                                       resolve_region.extent, tag);
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
            auto hazard = context->DetectHazard(*src_image, RangeFromLayers(resolve_region.srcSubresource),
                                                resolve_region.srcOffset, resolve_region.extent, false, SYNC_RESOLVE_TRANSFER_READ);
            if (hazard.IsHazard()) {
                skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), pResolveImageInfo->srcImage, region_loc,
                                 "Hazard %s for srcImage %s, region %" PRIu32 ". Access info %s.",
                                 string_SyncHazard(hazard.Hazard()), FormatHandle(pResolveImageInfo->srcImage).c_str(), region,
                                 cb_access_context->FormatHazard(hazard).c_str());
            }
        }

        if (dst_image) {
            auto hazard =
                context->DetectHazard(*dst_image, RangeFromLayers(resolve_region.dstSubresource), resolve_region.dstOffset,
                                      resolve_region.extent, false, SYNC_RESOLVE_TRANSFER_WRITE);
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

void SyncValidator::PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo,
                                                  const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdResolveImage2(commandBuffer, pResolveImageInfo, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
    auto *context = cb_access_context->GetCurrentAccessContext();
    assert(context);

    auto src_image = Get<ImageState>(pResolveImageInfo->srcImage);
    auto dst_image = Get<ImageState>(pResolveImageInfo->dstImage);

    for (uint32_t region = 0; region < pResolveImageInfo->regionCount; region++) {
        const auto &resolve_region = pResolveImageInfo->pRegions[region];
        if (src_image) {
            context->UpdateAccessState(*src_image, SYNC_RESOLVE_TRANSFER_READ, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(resolve_region.srcSubresource), resolve_region.srcOffset,
                                       resolve_region.extent, tag);
        }
        if (dst_image) {
            context->UpdateAccessState(*dst_image, SYNC_RESOLVE_TRANSFER_WRITE, SyncOrdering::kNonAttachment,
                                       RangeFromLayers(resolve_region.dstSubresource), resolve_region.dstOffset,
                                       resolve_region.extent, tag);
        }
    }
}

void SyncValidator::PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo,
                                                     const RecordObject &record_obj) {
    PreCallRecordCmdResolveImage2(commandBuffer, pResolveImageInfo, record_obj);
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
            skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), dstBuffer, error_obj.location,
                             "Hazard %s for dstBuffer %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(dstBuffer).c_str(), cb_access_context->FormatHazard(hazard).c_str());
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                 VkDeviceSize dataSize, const void *pData, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
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
                                                         VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                         const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
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
    PostCallRecordCmdSetEvent2(commandBuffer, event, pDependencyInfo, record_obj);
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
    PostCallRecordCmdResetEvent2(commandBuffer, event, stageMask, record_obj);
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
    PostCallRecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, record_obj);
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
    StateTracker::PostCallRecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos, record_obj);

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
            skip |= LogError(string_SyncHazardVUID(hazard.Hazard()), dstBuffer, error_obj.location,
                             "Hazard %s for dstBuffer %s. Access info %s.", string_SyncHazard(hazard.Hazard()),
                             FormatHandle(dstBuffer).c_str(), cb_access_context->FormatHazard(hazard).c_str());
        }
    }
    return skip;
}

void SyncValidator::PreCallRecordCmdWriteBufferMarker2AMD(VkCommandBuffer commandBuffer, VkPipelineStageFlags2KHR pipelineStage,
                                                          VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker,
                                                          const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdWriteBufferMarker2AMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_access_context = &cb_state->access_context;
    const auto tag = cb_access_context->NextCommandTag(record_obj.location.function);
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
                                                    const VkCommandBuffer *pCommandBuffers, const RecordObject &record_obj) {
    StateTracker::PreCallRecordCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers, record_obj);
    auto cb_state = Get<syncval_state::CommandBuffer>(commandBuffer);
    assert(cb_state);
    if (!cb_state) return;
    auto *cb_context = &cb_state->access_context;
    for (uint32_t cb_index = 0; cb_index < commandBufferCount; ++cb_index) {
        const ResourceUsageTag cb_tag = cb_context->NextIndexedCommandTag(record_obj.location.function, cb_index);
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
    PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, record_obj);
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
    ForAllQueueBatchContexts(
        [](const std::shared_ptr<QueueBatchContext> &batch) { batch->ApplyTaggedWait(kQueueAny, ResourceUsageRecord::kMaxIndex); });

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
        std::shared_ptr<const vvl::Semaphore> sem_state = Get<vvl::Semaphore>(semaphore);

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
            auto sem_state = Get<vvl::Semaphore>(semaphore_info.semaphore);
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
    PostCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence, record_obj);
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
    access_context_.TrimAndClearFirstAccess();

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
    const bool any_queue = (queue_id == kQueueAny);

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
    auto sem_state = sync_state_->Get<vvl::Semaphore>(sem);
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
    auto sem_state = sync_state_->Get<vvl::Semaphore>(sem);
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

    // Tag the presented images so record doesn't have to know the tagging scheme
    for (size_t index = 0; index < presented_images.size(); ++index) {
        const PresentedImage &presented = presented_images[index];

        // Need a copy that can be used as the pseudo-iterator...
        HazardResult hazard =
            access_context_.DetectHazard(presented.range_gen, SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL);
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
        access_context_.ResolveFromContext(presented.batch->access_context_);
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
            access_context_.ResolveFromContext(prev->access_context_);
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
    QueueId id = queue_state_ ? queue_state_->GetQueueId() : kQueueIdInvalid;
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

SignaledSemaphores::Signal::Signal(const std::shared_ptr<const vvl::Semaphore> &sem_state_,
                                   const std::shared_ptr<QueueBatchContext> &batch_, const SyncExecScope &exec_scope_)
    : sem_state(sem_state_), batch(batch_), first_scope({batch->GetQueueId(), exec_scope_}) {
    // Illegal to create a signal from no batch or an invalid semaphore... caller must assure validity
    assert(batch);
    assert(sem_state);
}

SignaledSemaphores::Signal::Signal(const std::shared_ptr<const vvl::Semaphore> &sem_state_, const PresentedImage &presented,
                                   ResourceUsageTag acq_tag)
    : sem_state(sem_state_), batch(presented.batch), first_scope(), acquired(presented, acq_tag) {
    // Illegal to create a signal from no batch or an invalid semaphore... caller must assure validity
    assert(batch);
    assert(sem_state);
}

FenceSyncState::FenceSyncState() : fence(), tag(kInvalidTag), queue_id(kQueueIdInvalid) {}

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
    access_context.UpdateAccessState(range_gen, usage, SyncOrdering::kNonAttachment, tag);
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

FenceSyncState::FenceSyncState(const std::shared_ptr<const vvl::Fence> &fence_, QueueId queue_id_, ResourceUsageTag tag_)
    : fence(fence_), tag(tag_), queue_id(queue_id_) {}
FenceSyncState::FenceSyncState(const std::shared_ptr<const vvl::Fence> &fence_, const PresentedImage &image, ResourceUsageTag tag_)
    : fence(fence_), tag(tag_), queue_id(kQueueIdInvalid), acquired(image, tag) {}


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

void syncval_state::BeginRenderingCmdState::AddRenderingInfo(const SyncValidator &state, const VkRenderingInfo &rendering_info) {
    info = std::make_unique<DynamicRenderingInfo>(state, rendering_info);
}

const syncval_state::DynamicRenderingInfo &syncval_state::BeginRenderingCmdState::GetRenderingInfo() const {
    assert(info);
    return *info;
}

