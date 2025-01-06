/* Copyright (c) 2024-2025 The Khronos Group Inc.
 * Copyright (c) 2024-2025 Valve Corporation
 * Copyright (c) 2024-2025 LunarG, Inc.
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

#include "sync/sync_reporting.h"
#include "sync/sync_image.h"
#include "sync/sync_validation.h"

SyncNodeFormatter::SyncNodeFormatter(const SyncValidator &sync_state, const vvl::CommandBuffer *cb_state)
    : debug_report(sync_state.debug_report), node(cb_state), label("command_buffer") {}

SyncNodeFormatter::SyncNodeFormatter(const SyncValidator &sync_state, const vvl::Image *image)
    : debug_report(sync_state.debug_report), node(image), label("image") {}

SyncNodeFormatter::SyncNodeFormatter(const SyncValidator &sync_state, const vvl::Queue *q_state)
    : debug_report(sync_state.debug_report), node(q_state), label("queue") {}

SyncNodeFormatter::SyncNodeFormatter(const SyncValidator &sync_state, const vvl::StateObject *state_object, const char *label_)
    : debug_report(sync_state.debug_report), node(state_object), label(label_) {}

std::string FormatStateObject(const SyncNodeFormatter &formatter) {
    std::stringstream out;
    if (formatter.label) {
        out << formatter.label << ": ";
    }
    if (formatter.node) {
        out << formatter.debug_report->FormatHandle(*formatter.node).c_str();
        if (formatter.node->Destroyed()) {
            out << " (destroyed)";
        }
    } else {
        out << "null handle";
    }
    return out.str();
}

void ReportKeyValues::Add(std::string_view key, std::string_view value) {
    key_values.emplace_back(KeyValue{std::string(key), std::string(value)});
}

void ReportKeyValues::Add(std::string_view key, uint64_t value) {
    key_values.emplace_back(KeyValue{std::string(key), std::to_string(value)});
}

static auto SortKeyValues(const std::vector<ReportKeyValues::KeyValue> &key_values) {
    auto get_sort_order = [](const std::string &key) -> uint32_t {
        // message_type goes first
        if (key == kPropertyMessageType) {
            return 0;
        }
        // then some common properties
        const char *common_properties[] = {kPropertyAccess, kPropertyPriorAccess, kPropertyReadBarriers, kPropertyWriteBarriers};
        if (IsValueIn(key, common_properties)) {
            return 1;
        }
        // debug properties are at the end
        const char *debug_properties[] = {kPropertySeqNo, kPropertySubCmd, kPropertyResetNo, kPropertyBatchTag};
        if (IsValueIn(key, debug_properties)) {
            return 3;
        }
        // everything else
        return 2;
    };
    auto sorted = key_values;
    std::stable_sort(sorted.begin(), sorted.end(), [&get_sort_order](const auto &a, const auto &b) {
        const uint32_t a_order = get_sort_order(a.key);
        const uint32_t b_order = get_sort_order(b.key);
        // Sort ordering groups
        if (a_order != b_order) {
            return a_order < b_order;
        }
        // Do not rearrange elements within a group. By returning false we indicate neither element
        // in the group is less than the other one. Stable sort will keep the original order.
        return false;
    });
    return sorted;
}

std::string ReportKeyValues::GetExtraPropertiesSection(bool pretty_print) const {
    if (key_values.empty()) {
        return {};
    }
    const auto sorted = SortKeyValues(key_values);
    std::stringstream ss;
    ss << "\n[Extra properties]\n";
    bool first = true;
    for (const auto &kv : sorted) {
        if (!first) {
            ss << "\n";
        }
        first = false;

        const uint32_t pretty_print_alignment = 18;
        uint32_t extra_space_count = 0;
        if (pretty_print && kv.key.length() < pretty_print_alignment) {
            extra_space_count = pretty_print_alignment - (uint32_t)kv.key.length();
        }

        ss << kv.key << std::string(extra_space_count, ' ') << " = " << kv.value;
    }
    return ss.str();
}

static std::string FormatHandleRecord(const HandleRecord::FormatterState &formatter) {
    std::stringstream out;
    const HandleRecord &handle = formatter.that;
    bool labeled = false;

    // Hardcode possible options in order not to store string per HandleRecord object.
    // If more general solution is needed the preference should be to store const char*
    // literal (8 bytes on 64 bit) instead of std::string which, even if empty,
    // can occupy 40 bytes, as was observed in one implementation. HandleRecord is memory
    // sensitive object (there can be a lot of instances).
    if (handle.type == kVulkanObjectTypeRenderPass) {
        out << "renderpass";
        labeled = true;
    } else if (handle.type == kVulkanObjectTypeCommandBuffer && handle.IsIndexed()) {
        out << "pCommandBuffers";
        labeled = true;
    }

    if (handle.IsIndexed()) {
        out << "[" << handle.index << "]";
        labeled = true;
    }
    if (labeled) {
        out << ": ";
    }
    out << formatter.state.FormatHandle(handle.TypedHandle());
    return out.str();
}

std::string FormatResourceUsageRecord(const ResourceUsageRecord::FormatterState &formatter) {
    std::stringstream out;
    const ResourceUsageRecord &record = formatter.record;
    if (record.alt_usage) {
        out << record.alt_usage.Formatter(formatter.sync_state);
    } else {
        out << "command: " << vvl::String(record.command);
        // Note: ex_cb_state set to null forces output of record.cb_state
        if (!formatter.ex_cb_state || (formatter.ex_cb_state != record.cb_state)) {
            out << ", " << FormatStateObject(SyncNodeFormatter(formatter.sync_state, record.cb_state));
        }

        // Associated resource
        if (formatter.handle_index != vvl::kNoIndex32) {
            auto cb_context = static_cast<const syncval_state::CommandBuffer *>(record.cb_state);
            const auto handle_records = cb_context->access_context.GetHandleRecords();

            // Command buffer can be in inconsistent state due to unhandled core validation error (core validation is disabled).
            // In this case the goal is not to crash, no guarantees that reported information (handle index) makes sense.
            const bool valid_handle_index = formatter.handle_index < handle_records.size();

            if (valid_handle_index) {
                out << ", resource: " << FormatHandleRecord(handle_records[formatter.handle_index].Formatter(formatter.sync_state));
            }
        }
        // Report debug region name. Empty name means that we are not inside any debug region.
        if (formatter.debug_name_provider) {
            const std::string debug_region_name = formatter.debug_name_provider->GetDebugRegionName(record);
            if (!debug_region_name.empty()) {
                out << ", debug_region: " << debug_region_name;
            }
        }
    }
    return out.str();
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

static std::optional<std::string> GetCompactFormOfAccessFlags(const SyncAccessFlags &accesses, VkQueueFlags allowed_queue_flags) {
    assert(accesses.any());  // otherwise can report 0 as one of compact forms
    VkPipelineStageFlags2 allowed_stages = 0;
    for (const auto &[queue_flag, stages] : syncAllCommandStagesByQueueFlags()) {
        if (queue_flag & allowed_queue_flags) {
            allowed_stages |= stages;
        }
    }
    // Accesses filtered by allowed queue flags
    SyncAccessFlags all_read_accesses = syncAccessReadMask;
    SyncAccessFlags all_shader_read_accesses = syncAccessReadMask;
    SyncAccessFlags all_shader_write_accesses = syncAccessWriteMask;

    const auto &access_infos = syncAccessInfoByAccessIndex();
    for (size_t i = 0; i < access_infos.size(); i++) {
        const SyncAccessInfo &access_info = access_infos[i];
        const bool is_stage_allowed = (access_info.stage_mask & allowed_stages) != 0;
        if (!is_stage_allowed) {
            all_read_accesses.reset(i);
            all_shader_read_accesses.reset(i);
            all_shader_write_accesses.reset(i);
            continue;
        }
        if (all_shader_read_accesses[i]) {
            const bool is_shader_read = (access_info.access_mask & kShaderReadExpandBits) != 0;
            if (!is_shader_read) {
                all_shader_read_accesses.reset(i);
            }
        }
        if (all_shader_write_accesses[i]) {
            const bool is_shader_write = (access_info.access_mask & kShaderWriteExpandBits) != 0;
            if (!is_shader_write) {
                all_shader_write_accesses.reset(i);
            }
        }
    }
    if (accesses == all_read_accesses) {
        return "SYNC_ALL_COMMANDS_MEMORY_READ";
    } else if (accesses == all_shader_read_accesses) {
        return "SYNC_ALL_COMMANDS_SHADER_READ";
    } else if (accesses == all_shader_write_accesses) {
        return "SYNC_ALL_COMMANDS_SHADER_WRITE";
    }
    return {};
}

static std::string string_SyncStageAccessFlags(const SyncAccessFlags &accesses, VkQueueFlags allowed_queue_flags) {
    if (accesses.none()) {
        return "0";
    }
    const auto compact_form = GetCompactFormOfAccessFlags(accesses, allowed_queue_flags);
    if (compact_form.has_value()) {
        return *compact_form;
    }
    std::string accesses_str;
    for (const SyncAccessInfo &info : syncAccessInfoByAccessIndex()) {
        if ((accesses & info.access_bit).any()) {
            if (!accesses_str.empty()) {
                accesses_str.append("|");
            }
            accesses_str.append(info.name);
        }
    }
    return accesses_str;
}

static std::string FormatHazardState(const HazardResult::HazardState &hazard, VkQueueFlags queue_flags,
                                     ReportKeyValues &key_values) {
    std::stringstream out;
    assert(hazard.access_index < static_cast<SyncAccessIndex>(syncAccessInfoByAccessIndex().size()));
    assert(hazard.prior_access_index < static_cast<SyncAccessIndex>(syncAccessInfoByAccessIndex().size()));
    const auto &usage_info = syncAccessInfoByAccessIndex()[hazard.access_index];
    const auto &prior_usage_info = syncAccessInfoByAccessIndex()[hazard.prior_access_index];
    out << "(";
    if (!hazard.recorded_access.get()) {
        // if we have a recorded usage the usage is reported from the recorded contexts point of view
        out << "usage: " << usage_info.name << ", ";
        key_values.Add(kPropertyAccess, usage_info.name);
    }
    out << "prior_usage: " << prior_usage_info.name;
    key_values.Add(kPropertyPriorAccess, prior_usage_info.name);
    if (IsHazardVsRead(hazard.hazard)) {
        const VkPipelineStageFlags2 barriers = hazard.access_state->GetReadBarriers(hazard.prior_access_index);
        const std::string barriers_str = string_VkPipelineStageFlags2(barriers);
        out << ", read_barriers: " << barriers_str;
        key_values.Add(kPropertyReadBarriers, barriers_str);
    } else {
        const SyncAccessFlags barriers = hazard.access_state->GetWriteBarriers();
        const std::string barriers_str = string_SyncStageAccessFlags(barriers, queue_flags);
        out << ", write_barriers: " << barriers_str;
        key_values.Add(kPropertyWriteBarriers, barriers_str);
    }
    return out.str();
}

std::string CommandExecutionContext::FormatHazard(const HazardResult &hazard, ReportKeyValues &key_values) const {
    std::stringstream out;
    assert(hazard.IsHazard());
    out << FormatHazardState(hazard.State(), queue_flags_, key_values);
    out << ", " << FormatUsage(hazard.TagEx()) << ")";
    return out.str();
}

std::string CommandExecutionContext::FormatHazard(const HazardResult &hazard) const {
    ReportKeyValues key_values;
    return FormatHazard(hazard, key_values);
}

std::string CommandBufferAccessContext::FormatUsage(ResourceUsageTagEx tag_ex) const {
    if (tag_ex.tag >= access_log_->size()) return std::string();

    std::stringstream out;
    assert(tag_ex.tag < access_log_->size());
    const auto &record = (*access_log_)[tag_ex.tag];
    const auto debug_name_provider = (record.label_command_index == vvl::kU32Max) ? nullptr : this;
    out << FormatResourceUsageRecord(record.Formatter(sync_state_, cb_state_, debug_name_provider, tag_ex.handle_index));
    return out.str();
}

void CommandBufferAccessContext::AddUsageRecordExtraProperties(ResourceUsageTag tag, ReportKeyValues &extra_properties) const {
    if (tag >= access_log_->size()) return;
    const ResourceUsageRecord &record = (*access_log_)[tag];
    extra_properties.Add(kPropertySeqNo, record.seq_num);
    if (record.sub_command != 0) {
        extra_properties.Add(kPropertySubCmd, record.sub_command);
    }
    extra_properties.Add(kPropertyResetNo, record.reset_count);
}

std::string QueueBatchContext::FormatUsage(ResourceUsageTagEx tag_ex) const {
    std::stringstream out;
    BatchAccessLog::AccessRecord access = batch_log_.GetAccessRecord(tag_ex.tag);
    if (access.IsValid()) {
        const BatchAccessLog::BatchRecord &batch = *access.batch;
        const ResourceUsageRecord &record = *access.record;
        if (batch.queue) {
            // Queue and Batch information (for enqueued operations)
            out << FormatStateObject(SyncNodeFormatter(sync_state_, batch.queue->GetQueueState()));
            out << ", submit: " << batch.submit_index << ", batch: " << batch.batch_index << ", ";
        }
        out << FormatResourceUsageRecord(record.Formatter(sync_state_, nullptr, access.debug_name_provider, tag_ex.handle_index));
    }
    return out.str();
}

void QueueBatchContext::AddUsageRecordExtraProperties(ResourceUsageTag tag, ReportKeyValues &extra_properties) const {
    BatchAccessLog::AccessRecord access = batch_log_.GetAccessRecord(tag);
    if (access.IsValid()) {
        extra_properties.Add(kPropertyBatchTag, access.batch->base_tag);
    }
}
