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
#include "error_message/error_strings.h"

static const char *string_SyncHazard(SyncHazard hazard) {
    switch (hazard) {
        case SyncHazard::NONE:
            return "NONE";
        case SyncHazard::READ_AFTER_WRITE:
            return "READ_AFTER_WRITE";
        case SyncHazard::WRITE_AFTER_READ:
            return "WRITE_AFTER_READ";
        case SyncHazard::WRITE_AFTER_WRITE:
            return "WRITE_AFTER_WRITE";
        case SyncHazard::READ_RACING_WRITE:
            return "READ_RACING_WRITE";
        case SyncHazard::WRITE_RACING_WRITE:
            return "WRITE_RACING_WRITE";
        case SyncHazard::WRITE_RACING_READ:
            return "WRITE_RACING_READ";
        case SyncHazard::READ_AFTER_PRESENT:
            return "READ_AFTER_PRESENT";
        case SyncHazard::WRITE_AFTER_PRESENT:
            return "WRITE_AFTER_PRESENT";
        case SyncHazard::PRESENT_AFTER_WRITE:
            return "PRESENT_AFTER_WRITE";
        case SyncHazard::PRESENT_AFTER_READ:
            return "PRESENT_AFTER_READ";
        default:
            assert(0);
            return "INVALID HAZARD";
    }
}

static bool IsHazardVsRead(SyncHazard hazard) {
    switch (hazard) {
        case SyncHazard::WRITE_AFTER_READ:
        case SyncHazard::WRITE_RACING_READ:
        case SyncHazard::PRESENT_AFTER_READ:
            return true;
        default:
            return false;
    }
}

static auto SortKeyValues(const std::vector<ReportProperties::NameValue> &name_values) {
    const std::vector<std::string> std_properties = {
        kPropertyMessageType,   kPropertyHazardType, kPropertyAccess,       kPropertyPriorAccess, kPropertyReadBarriers,
        kPropertyWriteBarriers, kPropertyCommand,    kPropertyPriorCommand, kPropertyDebugRegion, kPropertyPriorDebugRegion};
    const uint32_t other_properties_order = uint32_t(std_properties.size());
    const uint32_t debug_properties_order = other_properties_order + 1;

    auto get_sort_order = [&](const std::string &key) -> uint32_t {
        // at first put standard properties
        auto std_it = std::find(std_properties.begin(), std_properties.end(), key);
        if (std_it != std_properties.end()) {
            const uint32_t std_order = uint32_t(&*std_it - std_properties.data());
            return std_order;
        }
        // debug properties are at the end
        const char *debug_properties[] = {kPropertySeqNo, kPropertyResetNo, kPropertyBatchTag};
        if (IsValueIn(key, debug_properties)) {
            return debug_properties_order;
        }
        return other_properties_order;
    };
    auto sorted = name_values;
    std::stable_sort(sorted.begin(), sorted.end(), [&get_sort_order](const auto &a, const auto &b) {
        const uint32_t a_order = get_sort_order(a.name);
        const uint32_t b_order = get_sort_order(b.name);
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

static std::string FormatAccessProperty(const SyncAccessInfo &access) {
    constexpr std::array special_accesses = {SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_ACQUIRE_READ_SYNCVAL,
                                             SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL, SYNC_IMAGE_LAYOUT_TRANSITION,
                                             SYNC_QUEUE_FAMILY_OWNERSHIP_TRANSFER};
    if (IsValueIn(access.access_index, special_accesses)) {
        // Print internal name for accesses that don't have corresponding Vulkan constants
        return access.name;
    }
    std::stringstream ss;
    ss << string_VkPipelineStageFlagBits2(access.stage_mask);
    ss << "(";
    ss << string_VkAccessFlagBits2(access.access_mask);
    ss << ")";
    return ss.str();
}

static void GetAccessProperties(const HazardResult &hazard_result, const SyncValidator &device, VkQueueFlags allowed_queue_flags,
                                ReportProperties &properties) {
    const HazardResult::HazardState &hazard = hazard_result.State();
    const SyncAccessInfo &access_info = GetSyncAccessInfos()[hazard.access_index];
    const SyncAccessInfo &prior_access_info = GetSyncAccessInfos()[hazard.prior_access_index];

    if (!hazard.recorded_access.get()) {
        properties.Add(kPropertyAccess, FormatAccessProperty(access_info));
    }
    properties.Add(kPropertyPriorAccess, FormatAccessProperty(prior_access_info));

    if (IsHazardVsRead(hazard.hazard)) {
        const VkPipelineStageFlags2 barriers = hazard.access_state->GetReadBarriers(hazard.prior_access_index);
        const std::string barriers_str = string_VkPipelineStageFlags2(barriers);
        properties.Add(kPropertyReadBarriers, barriers ? barriers_str : "0");
    } else {
        const SyncAccessFlags barriers = hazard.access_state->GetWriteBarriers();
        const std::string property_barriers_str = FormatSyncAccesses(barriers, device, allowed_queue_flags, true);
        properties.Add(kPropertyWriteBarriers, property_barriers_str);
    }
}

static void GetPriorUsageProperties(const ResourceUsageInfo &prior_usage_info, ReportProperties &properties) {
    properties.Add(kPropertyPriorCommand, vvl::String(prior_usage_info.command));

    if (!prior_usage_info.debug_region_name.empty()) {
        properties.Add(kPropertyPriorDebugRegion, prior_usage_info.debug_region_name);
    }

    // These commands are not recorded/submitted, so the rest of the properties are not applicable.
    // TODO: we can track command seq number.
    if (IsValueIn(prior_usage_info.command,
                  {vvl::Func::vkQueuePresentKHR, vvl::Func::vkAcquireNextImageKHR, vvl::Func::vkAcquireNextImage2KHR})) {
        return;
    }
    properties.Add(kPropertySeqNo, prior_usage_info.command_seq);
    properties.Add(kPropertyResetNo, prior_usage_info.command_buffer_reset_count);
    if (prior_usage_info.queue) {
        properties.Add(kPropertyBatchTag, prior_usage_info.batch_base_tag);
        properties.Add(kPropertySubmitIndex, prior_usage_info.submit_index);
        properties.Add(kPropertyBatchIndex, prior_usage_info.batch_index);
    }
}

static VkPipelineStageFlags2 GetAllowedStages(VkQueueFlags queue_flags, VkPipelineStageFlagBits2 disabled_stages) {
    VkPipelineStageFlags2 allowed_stages = 0;
    for (const auto &[queue_flag, stages] : syncAllCommandStagesByQueueFlags()) {
        if (queue_flag & queue_flags) {
            allowed_stages |= (stages & ~disabled_stages);
        }
    }
    return allowed_stages;
}

static SyncAccessFlags FilterSyncAccessesByAllowedVkStages(const SyncAccessFlags &accesses, VkPipelineStageFlags2 allowed_stages,
                                                           VkAccessFlags2 disabled_accesses) {
    SyncAccessFlags filtered_accesses = accesses;
    const auto &access_infos = GetSyncAccessInfos();
    for (size_t i = 0; i < access_infos.size(); i++) {
        const SyncAccessInfo &access_info = access_infos[i];
        const bool is_stage_allowed = (access_info.stage_mask & allowed_stages) != 0;
        const bool is_access_allowed = (access_info.access_mask & disabled_accesses) == 0;
        if (!is_stage_allowed || !is_access_allowed) {
            filtered_accesses.reset(i);
        }
    }
    return filtered_accesses;
}

static SyncAccessFlags FilterSyncAccessesByAllowedVkAccesses(const SyncAccessFlags &accesses, VkAccessFlags2 allowed_vk_accesses) {
    SyncAccessFlags filtered_accesses = accesses;
    const auto &access_infos = GetSyncAccessInfos();
    for (size_t i = 0; i < access_infos.size(); i++) {
        const SyncAccessInfo &access_info = access_infos[i];
        if (filtered_accesses[i]) {
            const bool is_access_allowed = (access_info.access_mask & allowed_vk_accesses) != 0;
            if (!is_access_allowed) {
                filtered_accesses.reset(i);
            }
        }
    }
    return filtered_accesses;
}

static std::vector<std::pair<VkPipelineStageFlags2, VkAccessFlags2>> ConvertSyncAccessesToCompactVkForm(
    const SyncAccessFlags &sync_accesses, const SyncValidator &device, VkQueueFlags allowed_queue_flags) {
    if (sync_accesses.none()) {
        return {};
    }

    const VkPipelineStageFlags2 disabled_stages = sync_utils::DisabledPipelineStages(device.enabled_features, device.extensions);
    const VkPipelineStageFlags2 all_transfer_expand_bits = kAllTransferExpandBits & ~disabled_stages;

    const VkAccessFlags2 disabled_accesses = sync_utils::DisabledAccesses(device.extensions);
    const VkAccessFlags2 all_shader_read_bits = kShaderReadExpandBits & ~disabled_accesses;

    // Build stage -> accesses mapping. OR-merge accesses that happen on the same stage.
    // Also handle ALL_COMMANDS accesses.
    vvl::unordered_map<VkPipelineStageFlagBits2, VkAccessFlags2> stage_to_accesses;
    {
        const VkPipelineStageFlags2 allowed_stages = GetAllowedStages(allowed_queue_flags, disabled_stages);
        const SyncAccessFlags filtered_accesses =
            FilterSyncAccessesByAllowedVkStages(sync_accesses, allowed_stages, disabled_accesses);

        const SyncAccessFlags all_reads =
            FilterSyncAccessesByAllowedVkStages(syncAccessReadMask, allowed_stages, disabled_accesses);
        const SyncAccessFlags all_shader_reads = FilterSyncAccessesByAllowedVkAccesses(all_reads, kShaderReadExpandBits);

        const SyncAccessFlags all_writes =
            FilterSyncAccessesByAllowedVkStages(syncAccessWriteMask, allowed_stages, disabled_accesses);
        const SyncAccessFlags all_shader_writes = FilterSyncAccessesByAllowedVkAccesses(all_writes, kShaderWriteExpandBits);

        if (filtered_accesses == all_reads) {
            stage_to_accesses[VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT] = VK_ACCESS_2_MEMORY_READ_BIT;
        } else if (filtered_accesses == all_shader_reads) {
            stage_to_accesses[VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT] = VK_ACCESS_2_SHADER_READ_BIT;
        } else if (filtered_accesses == all_shader_writes) {
            stage_to_accesses[VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT] = VK_ACCESS_2_SHADER_WRITE_BIT;
        } else {
            for (size_t i = 0; i < filtered_accesses.size(); i++) {
                if (filtered_accesses[i]) {
                    const SyncAccessInfo &info = GetSyncAccessInfos()[i];
                    stage_to_accesses[info.stage_mask] |= info.access_mask;
                }
            }
        }
    }

    // Build accesses -> stages mapping. OR-merge stages that share the same accesses
    vvl::unordered_map<VkAccessFlags2, VkPipelineStageFlags2> accesses_to_stages;
    for (const auto [stage, accesses] : stage_to_accesses) {
        accesses_to_stages[accesses] |= stage;
    }

    // Replace sequences of stages/accesses with more compact equivalent meta values where possible
    std::vector<std::pair<VkPipelineStageFlags2, VkAccessFlags2>> result;
    VkPipelineStageFlags2 stages_with_all_supported_accesses = 0;
    VkAccessFlags2 all_accesses = 0;  // accesses for the above stages

    for (const auto &entry : accesses_to_stages) {
        VkAccessFlags2 accesses = entry.first;
        VkPipelineStageFlags2 stages = entry.second;

        VkAccessFlags2 all_accesses_supported_by_stages = sync_utils::CompatibleAccessMask(stages);
        {
            // Remove meta stages.
            // TODO: revisit CompatibleAccessMask helper. SyncVal works with expanded representation.
            // Meta stages are needed for core checks in this case, update function so serve both purposes well.
            all_accesses_supported_by_stages &= ~VK_ACCESS_2_SHADER_READ_BIT;
            all_accesses_supported_by_stages &= ~VK_ACCESS_2_SHADER_WRITE_BIT;
            // Remove unsupported accesses, otherwise the access mask won't be detected as the one that covers ALL accesses
            // TODO: ideally this should be integrated into utilities logic (need to revisit all use cases)
            if (!IsExtEnabled(device.extensions.vk_ext_blend_operation_advanced)) {
                all_accesses_supported_by_stages &= ~VK_ACCESS_2_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT;
            }
        }

        // Check if ALL supported accesses for the given stage are used.
        // This is an opportunity to use a compact message form.
        if (accesses == all_accesses_supported_by_stages) {
            stages_with_all_supported_accesses |= stages;
            all_accesses |= all_accesses_supported_by_stages;
            continue;
        }

        sync_utils::ReplaceExpandBitsWithMetaMask(stages, all_transfer_expand_bits, VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT);

        const VkAccessFlags2 all_shader_reads_supported_by_stages = all_shader_read_bits & all_accesses_supported_by_stages;
        sync_utils::ReplaceExpandBitsWithMetaMask(accesses, all_shader_reads_supported_by_stages, VK_ACCESS_2_SHADER_READ_BIT);
        result.emplace_back(stages, accesses);
    }
    if (stages_with_all_supported_accesses) {
        if (IsSingleBitSet(stages_with_all_supported_accesses) && GetBitSetCount(all_accesses) <= 2) {
            // For simple configurations (1 stage and at most 2 accesses) don't use ALL accesses shortcut
            result.emplace_back(stages_with_all_supported_accesses, all_accesses);
        } else {
            sync_utils::ReplaceExpandBitsWithMetaMask(stages_with_all_supported_accesses, all_transfer_expand_bits,
                                                      VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT);
            result.emplace_back(stages_with_all_supported_accesses, sync_utils::kAllAccesses);
        }
    }
    return result;
}

// Given that access is hazardous, we check if at least stage or access part of it is covered
// by the synchronization. If applied synchronization covers at least stage or access component
// then we can provide more precise message by focusing on the other component.
static std::pair<bool, bool> GetPartialProtectedInfo(const SyncAccessInfo &access, const SyncAccessFlags &write_barriers,
                                                     const CommandExecutionContext &context) {
    const auto protected_stage_access_pairs =
        ConvertSyncAccessesToCompactVkForm(write_barriers, context.GetSyncState(), context.GetQueueFlags());
    bool is_stage_protected = false;
    bool is_access_protected = false;
    for (const auto &protected_stage_access : protected_stage_access_pairs) {
        if (protected_stage_access.first & access.stage_mask) {
            is_stage_protected = true;
        }
        if (protected_stage_access.second & access.access_mask) {
            is_access_protected = true;
        }
    }
    return std::make_pair(is_stage_protected, is_access_protected);
}

void ReportProperties::Add(std::string_view property_name, std::string_view value) {
    name_values.emplace_back(NameValue{std::string(property_name), std::string(value)});
}

void ReportProperties::Add(std::string_view property_name, uint64_t value) {
    name_values.emplace_back(NameValue{std::string(property_name), std::to_string(value)});
}

std::string ReportProperties::FormatExtraPropertiesSection() const {
    if (name_values.empty()) {
        return {};
    }
    const auto sorted = SortKeyValues(name_values);
    std::stringstream ss;
    ss << "[Extra properties]\n";
    bool first = true;
    for (const NameValue &property : sorted) {
        if (!first) {
            ss << "\n";
        }
        first = false;
        ss << property.name << " = " << property.value;
    }
    return ss.str();
}

ReportProperties GetErrorMessageProperties(const HazardResult &hazard, const CommandExecutionContext &context, vvl::Func command,
                                           const char *message_type, const AdditionalMessageInfo &additional_info) {
    ReportProperties properties;
    properties.Add(kPropertyMessageType, message_type);
    properties.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
    properties.Add(kPropertyCommand, vvl::String(command));

    GetAccessProperties(hazard, context.GetSyncState(), context.GetQueueFlags(), properties);

    if (hazard.Tag() != kInvalidTag) {
        ResourceUsageInfo prior_usage_info = context.GetResourceUsageInfo(hazard.TagEx());
        GetPriorUsageProperties(prior_usage_info, properties);
    }
    for (const auto &property : additional_info.properties.name_values) {
        properties.Add(property.name, property.value);
    }
    return properties;
}

std::string FormatErrorMessage(const HazardResult &hazard, const CommandExecutionContext &context, vvl::Func command,
                               const std::string &resouce_description, const AdditionalMessageInfo &additional_info) {
    const SyncHazard hazard_type = hazard.Hazard();
    const SyncHazardInfo hazard_info = GetSyncHazardInfo(hazard_type);

    const SyncAccessInfo &access = GetSyncAccessInfos()[hazard.State().access_index];
    const SyncAccessInfo &prior_access = GetSyncAccessInfos()[hazard.State().prior_access_index];

    const SyncAccessFlags write_barriers = hazard.State().access_state->GetWriteBarriers();
    const VkPipelineStageFlags2 read_barriers = hazard.State().access_state->GetReadBarriers(hazard.State().prior_access_index);

    // TODO: BOTTOM_OF_PIPE part will go away when syncval switches internally to use NONE/ALL for everything
    const bool missing_synchronization = (hazard_info.IsPriorWrite() && write_barriers.none()) ||
                                         (hazard_info.IsPriorRead() && (read_barriers == VK_PIPELINE_STAGE_2_NONE ||
                                                                        read_barriers == VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT));

    std::stringstream ss;

    // Brief description of what happened
    ss << string_SyncHazard(hazard_type) << " hazard detected";
    if (!additional_info.hazard_overview.empty()) {
        ss << ": " << additional_info.hazard_overview;
    }
    ss << ". ";
    ss << (additional_info.access_initiator.empty() ? vvl::String(command) : additional_info.access_initiator);
    ss << " ";
    if (!additional_info.access_action.empty()) {
        ss << additional_info.access_action;
    } else {
        ss << (hazard_info.IsWrite() ? "writes to" : "reads");
    }
    ss << " " << resouce_description << ", which was previously ";
    if (hazard_info.IsPriorWrite()) {
        if (prior_access.access_index == SYNC_IMAGE_LAYOUT_TRANSITION) {
            ss << "written during an image layout transition initiated by ";
        } else {
            ss << "written by ";
        }
    } else {
        ss << "read by ";
    }
    if (hazard.Tag() == kInvalidTag) {
        // Invalid tag for prior access means the same command performed ILT before loadOp access
        ss << "the same command";
    } else {
        const ResourceUsageInfo prior_usage_info = context.GetResourceUsageInfo(hazard.TagEx());
        if (prior_usage_info.command == command) {
            ss << "another ";
        }
        ss << vvl::String(prior_usage_info.command);
        if (!prior_usage_info.debug_region_name.empty()) {
            ss << "[" << prior_usage_info.debug_region_name << "]";
        }
        if (prior_usage_info.command == command) {
            ss << " command";
        }
    }
    if (!additional_info.brief_description_end_text.empty()) {
        ss << " " << additional_info.brief_description_end_text;
    }
    ss << ". ";

    // Additional information before synchronization section.
    if (!additional_info.pre_synchronization_text.empty()) {
        ss << additional_info.pre_synchronization_text;
    }

    // Synchronization information
    ss << "\n";
    if (missing_synchronization) {
        const char *access_type = hazard_info.IsWrite() ? "write" : "read";
        const char *prior_access_type = hazard_info.IsPriorWrite() ? "write" : "read";

        auto get_special_access_name = [](SyncAccessIndex access) -> const char * {
            if (access == SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_ACQUIRE_READ_SYNCVAL) {
                return "swapchain image acquire operation";
            } else if (access == SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL) {
                return "swapchain present operation";
            } else if (access == SYNC_IMAGE_LAYOUT_TRANSITION) {
                return "layout transition";
            } else if (access == SYNC_QUEUE_FAMILY_OWNERSHIP_TRANSFER) {
                return "ownership transfer";
            }
            return nullptr;
        };

        ss << "No sufficient synchronization is present to ensure that a ";
        if (const char *special_access_name = get_special_access_name(access.access_index)) {
            ss << special_access_name;
        } else {
            assert(access.access_mask != VK_ACCESS_2_NONE);
            assert(access.stage_mask != VK_PIPELINE_STAGE_2_NONE);
            ss << access_type << " (" << string_VkAccessFlagBits2(access.access_mask) << ") at ";
            ss << string_VkPipelineStageFlagBits2(access.stage_mask);
        }

        ss << " does not conflict with a prior ";
        if (const char *special_access_name = get_special_access_name(prior_access.access_index)) {
            ss << special_access_name;
        } else {
            assert(prior_access.access_mask != VK_ACCESS_2_NONE);
            assert(prior_access.stage_mask != VK_PIPELINE_STAGE_2_NONE);
            ss << prior_access_type;
            if (prior_access.access_mask != access.access_mask) {
                ss << " (" << string_VkAccessFlags2(prior_access.access_mask) << ")";
            } else {
                ss << " of the same type";
            }
            ss << " at ";
            if (prior_access.stage_mask == access.stage_mask) {
                ss << "the same stage";
            } else {
                ss << string_VkPipelineStageFlagBits2(prior_access.stage_mask);
            }
        }
        ss << ".";
    } else if (hazard_info.IsPriorWrite()) {  // RAW/WAW hazards
        ss << "The current synchronization allows ";
        ss << FormatSyncAccesses(write_barriers, context.GetSyncState(), context.GetQueueFlags(), false);
        ss << ", but to prevent this hazard, ";
        auto [is_stage_protected, is_access_protected] = GetPartialProtectedInfo(access, write_barriers, context);
        if (is_access_protected) {
            ss << "it must allow these accesses at ";
            ss << string_VkPipelineStageFlagBits2(access.stage_mask) << ".";
        } else if (access.access_mask != VK_ACCESS_2_NONE) {
            ss << "it must allow ";
            ss << string_VkAccessFlagBits2(access.access_mask) << " accesses at ";
            ss << string_VkPipelineStageFlagBits2(access.stage_mask) << ".";
        } else {
            // TODO: analyse exact form of synchronization is needed or specific options to use
            ss << "it must protect layout transition accesses.";
        }
    } else {  // WAR hazard
        ss << "The current synchronization defines the destination stage mask as ";
        ss << string_VkPipelineStageFlags2(read_barriers);
        ss << ", but to prevent this hazard, it must include ";
        ss << string_VkPipelineStageFlagBits2(access.stage_mask) << ".";
    }

    // Give a hint for WAR hazard
    if (IsValueIn(hazard_type, {WRITE_AFTER_READ, WRITE_RACING_READ, PRESENT_AFTER_READ})) {
        ss << "\nVulkan insight: an execution dependency is sufficient to prevent this hazard.";
    }

    if (!additional_info.message_end_text.empty()) {
        ss << additional_info.message_end_text;
    }
    return ss.str();
}

std::string FormatSyncAccesses(const SyncAccessFlags &sync_accesses, const SyncValidator &device, VkQueueFlags allowed_queue_flags,
                               bool format_as_extra_property) {
    const auto report_accesses = ConvertSyncAccessesToCompactVkForm(sync_accesses, device, allowed_queue_flags);
    if (report_accesses.empty()) {
        return "0";
    }
    std::stringstream out;
    bool first = true;
    for (const auto &[stages, accesses] : report_accesses) {
        if (!first) {
            out << (format_as_extra_property ? ":" : ", ");
        }
        if (format_as_extra_property) {
            if (accesses == sync_utils::kAllAccesses) {
                out << string_VkPipelineStageFlags2(stages) << "(ALL_ACCESSES)";
            } else {
                out << string_VkPipelineStageFlags2(stages) << "(" << string_VkAccessFlags2(accesses) << ")";
            }
        } else {
            if (accesses == sync_utils::kAllAccesses) {
                out << "all accesses at " << string_VkPipelineStageFlags2(stages);
            } else {
                out << string_VkAccessFlags2(accesses) << " accesses at " << string_VkPipelineStageFlags2(stages);
            }
        }
        first = false;
    }
    return out.str();
}

void FormatVideoPictureResouce(const Logger &logger, const VkVideoPictureResourceInfoKHR &video_picture, std::stringstream &ss) {
    ss << "{";
    ss << logger.FormatHandle(video_picture.imageViewBinding);
    ss << ", codedOffset (" << string_VkOffset2D(video_picture.codedOffset) << ")";
    ss << ", codedExtent (" << string_VkExtent2D(video_picture.codedExtent) << ")";
    ss << ", baseArrayLayer = " << video_picture.baseArrayLayer;
    ss << "}";
}

void FormatVideoQuantizationMap(const Logger &logger, const VkVideoEncodeQuantizationMapInfoKHR &quantization_map,
                                std::stringstream &ss) {
    ss << "{";
    ss << logger.FormatHandle(quantization_map.quantizationMap);
    ss << ", quantizationMapExtent (" << string_VkExtent2D(quantization_map.quantizationMapExtent) << ")";
    ss << "}";
}

static ResourceUsageInfo GetResourceUsageInfoFromRecord(ResourceUsageTagEx tag_ex, const ResourceUsageRecord &record,
                                                        const DebugNameProvider *debug_name_provider) {
    ResourceUsageInfo info;
    if (record.alt_usage) {
        info.command = record.alt_usage.GetCommand();
    } else {
        info.command = record.command;
        info.command_seq = record.seq_num;
        info.command_buffer_reset_count = record.reset_count;

        // Associated resource
        if (tag_ex.handle_index != vvl::kNoIndex32) {
            auto &cb_context = syncval_state::SubState(*record.cb_state);
            const auto &handle_records = cb_context.access_context.GetHandleRecords();

            // Command buffer can be in inconsistent state due to unhandled core validation error (core validation is disabled).
            // In this case the goal is not to crash, no guarantees that reported information (handle index) makes sense.
            const bool valid_handle_index = tag_ex.handle_index < handle_records.size();
            if (valid_handle_index) {
                info.resource_handle = handle_records[tag_ex.handle_index].TypedHandle();
                // TODO: also extract optional index or get rid of index
            }
        }
        // Debug region name. Empty name means that we are not inside any debug region.
        if (debug_name_provider) {
            info.debug_region_name = debug_name_provider->GetDebugRegionName(record);
        }
    }
    info.cb = record.cb_state;
    return info;
}

ResourceUsageInfo CommandBufferAccessContext::GetResourceUsageInfo(ResourceUsageTagEx tag_ex) const {
    const ResourceUsageRecord &record = (*access_log_)[tag_ex.tag];
    const auto debug_name_provider = (record.label_command_index == vvl::kU32Max) ? nullptr : this;
    return GetResourceUsageInfoFromRecord(tag_ex, record, debug_name_provider);
}

ResourceUsageInfo QueueBatchContext::GetResourceUsageInfo(ResourceUsageTagEx tag_ex) const {
    BatchAccessLog::AccessRecord access = batch_log_.GetAccessRecord(tag_ex.tag);
    if (!access.IsValid()) {
        return {};
    }
    const ResourceUsageRecord &record = *access.record;
    ResourceUsageInfo info = GetResourceUsageInfoFromRecord(tag_ex, record, access.debug_name_provider);

    const BatchAccessLog::BatchRecord &batch = *access.batch;
    if (batch.queue) {
        info.queue = batch.queue->GetQueueState();
        info.submit_index = batch.submit_index;
        info.batch_index = batch.batch_index;
        info.batch_base_tag = batch.base_tag;
    }
    return info;
}
