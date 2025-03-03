/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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

#include "sync/sync_error_messages.h"
#include "sync/sync_commandbuffer.h"
#include "sync/sync_image.h"
#include "sync/sync_reporting.h"
#include "sync/sync_validation.h"
#include "error_message/error_strings.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/descriptor_sets.h"
#include "utils/text_utils.h"
using text::Format;

#include <cassert>
#include <cinttypes>
#include <cstdarg>
#include <sstream>

static const char* string_SyncHazard(SyncHazard hazard) {
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

// Given that access is hazardous, we check if at least stage or access part of it is covered
// by the synchronization. If applied synchronization covers at least stage or access component
// then we can provide more precise message by focusing on the other component.
static std::pair<bool, bool> GetPartialProtectedInfo(const SyncAccessInfo& access, const SyncAccessFlags& write_barriers,
                                                     const CommandExecutionContext& context) {
    const auto protected_stage_access_pairs = ConvertSyncAccessesToCompactVkForm(
        write_barriers, context.GetQueueFlags(), context.GetSyncState().enabled_features, context.GetSyncState().extensions);
    bool is_stage_protected = false;
    bool is_access_protected = false;
    for (const auto& protected_stage_access : protected_stage_access_pairs) {
        if (protected_stage_access.first & access.stage_mask) {
            is_stage_protected = true;
        }
        if (protected_stage_access.second & access.access_mask) {
            is_access_protected = true;
        }
    }
    return std::make_pair(is_stage_protected, is_access_protected);
}

static void FormatCommonMessage(const HazardResult& hazard, const std::string& resouce_description, const vvl::Func command,
                                const ReportKeyValues& key_values, const CommandExecutionContext& context,
                                const syncval::AdditionalMessageInfo& additional_info, std::stringstream& ss) {
    const SyncHazard hazard_type = hazard.Hazard();
    const SyncHazardInfo hazard_info = GetSyncHazardInfo(hazard_type);

    const SyncAccessInfo& access = syncAccessInfoByAccessIndex()[hazard.State().access_index];
    const SyncAccessInfo& prior_access = syncAccessInfoByAccessIndex()[hazard.State().prior_access_index];

    const SyncAccessFlags write_barriers = hazard.State().access_state->GetWriteBarriers();
    const VkPipelineStageFlags2 read_barriers = hazard.State().access_state->GetReadBarriers(hazard.State().prior_access_index);

    // TODO: BOTTOM_OF_PIPE part will go away when syncval switches internally to use NONE/ALL for everything
    const bool missing_synchronization = (hazard_info.IsPriorWrite() && write_barriers.none()) ||
                                         (hazard_info.IsPriorRead() && (read_barriers == VK_PIPELINE_STAGE_2_NONE ||
                                                                        read_barriers == VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT));

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
        const ReportUsageInfo usage_info = context.GetReportUsageInfo(hazard.TagEx());
        if (usage_info.command == command) {
            ss << "another ";
        }
        ss << vvl::String(usage_info.command);
        if (const auto* debug_region = key_values.FindProperty(kPropertyPriorDebugRegion)) {
            ss << "[" << *debug_region << "]";
        }
        if (usage_info.command == command) {
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
        const char* access_type = hazard_info.IsWrite() ? "write" : "read";
        const char* prior_access_type = hazard_info.IsPriorWrite() ? "write" : "read";

        auto get_special_access_name = [](SyncAccessIndex access) -> const char* {
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
        if (const char* special_access_name = get_special_access_name(access.access_index)) {
            ss << special_access_name;
        } else {
            assert(access.access_mask != VK_ACCESS_2_NONE);
            assert(access.stage_mask != VK_PIPELINE_STAGE_2_NONE);
            ss << access_type << " (" << string_VkAccessFlagBits2(access.access_mask) << ") at ";
            ss << string_VkPipelineStageFlagBits2(access.stage_mask);
        }

        ss << " does not conflict with a prior ";
        if (const char* special_access_name = get_special_access_name(prior_access.access_index)) {
            ss << special_access_name;
        } else {
            assert(prior_access.access_mask != VK_ACCESS_2_NONE);
            assert(prior_access.stage_mask != VK_PIPELINE_STAGE_2_NONE);
            ss << prior_access_type;
            if (prior_access.access_mask != access.access_mask) {
                ss << " (" << string_VkAccessFlags2(prior_access.access_mask) << ")";
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
        ss << FormatSyncAccesses(write_barriers, context.GetQueueFlags(), context.GetSyncState().enabled_features,
                                 context.GetSyncState().extensions, false);
        auto [is_stage_protected, is_access_protected] = GetPartialProtectedInfo(access, write_barriers, context);
        if (is_access_protected) {
            ss << " but not at " << string_VkPipelineStageFlagBits2(access.stage_mask) << ".";
        } else {
            ss << ", but to prevent this hazard, ";

            if (access.access_mask != VK_ACCESS_2_NONE) {
                ss << "it must allow ";
                ss << string_VkAccessFlagBits2(access.access_mask) << " accesses at ";
                ss << string_VkPipelineStageFlagBits2(access.stage_mask) << ".";
            } else {
                // TODO: analyse exact form of synchronization is needed or specific options to use
                ss << "it must protect layout transition accesses.";
            }
        }
    } else {  // WAR hazard
        ss << "The current synchronization waits at ";
        ss << string_VkPipelineStageFlags2(read_barriers);
        ss << ", but to prevent this hazard, it must wait at ";
        ss << string_VkPipelineStageFlagBits2(access.stage_mask) << ".";
    }

    // Give a hint for WAR hazard
    if (IsValueIn(hazard_type, {WRITE_AFTER_READ, WRITE_RACING_READ, PRESENT_AFTER_READ})) {
        ss << "\nVulkan insight: an execution dependency is sufficient to prevent this hazard.";
    }
}

namespace syncval {

ErrorMessages::ErrorMessages(vvl::Device& validator)
    : validator_(validator),
      extra_properties_(validator_.syncval_settings.message_extra_properties),
      pretty_print_extra_(validator_.syncval_settings.message_extra_properties_pretty_print) {}

std::string ErrorMessages::Error(const HazardResult& hazard, const CommandExecutionContext& context, vvl::Func command,
                                 const std::string& resouce_description, const char* message_type,
                                 const AdditionalMessageInfo& additional_info) const {
    ReportKeyValues key_values;
    context.FormatHazard(hazard, key_values);
    key_values.Add(kPropertyMessageType, message_type);
    key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
    key_values.Add(kPropertyCommand, vvl::String(command));
    for (const auto& kv : additional_info.properties.key_values) {
        key_values.Add(kv.key, kv.value);
    }

    if (validator_.syncval_settings.message_extra_properties) {
        context.AddUsageRecordExtraProperties(hazard.Tag(), key_values);
    }

    std::stringstream ss;
    FormatCommonMessage(hazard, resouce_description, command, key_values, context, additional_info, ss);

    if (!additional_info.message_end_text.empty()) {
        ss << additional_info.message_end_text;
    }

    std::string message = ss.str();
    if (extra_properties_) {
        if (!message.empty() && message.back() != '\n') {
            message += '\n';
        }
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::BufferError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context, vvl::Func command,
                                       const std::string& resource_description, const ResourceAccessRange range,
                                       AdditionalMessageInfo additional_info) const {
    std::stringstream ss;
    ss << "\nBuffer access region: {\n";
    ss << "  offset = " << range.begin << "\n";
    ss << "  size = " << range.end - range.begin << "\n";
    ss << "}\n";
    additional_info.message_end_text += ss.str();

    return Error(hazard, cb_context, command, resource_description, "BufferError", additional_info);
}

std::string ErrorMessages::BufferCopyError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                           const vvl::Func command, const std::string& resource_description, uint32_t region_index,
                                           ResourceAccessRange range) const {
    AdditionalMessageInfo additional_info;
    additional_info.properties.Add(kPropertyRegionIndex, region_index);

    std::stringstream ss;
    ss << "\nBuffer copy region " << region_index << ": {\n";
    ss << "  offset = " << range.begin << ",\n";
    ss << "  size = " << range.end - range.begin << "\n";
    ss << "}\n";
    additional_info.message_end_text = ss.str();

    return Error(hazard, cb_context, command, resource_description, "BufferCopyError", additional_info);
}

std::string ErrorMessages::ImageCopyResolveBlitError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                     vvl::Func command, const std::string& resource_description,
                                                     uint32_t region_index, const VkOffset3D& offset, const VkExtent3D& extent,
                                                     const VkImageSubresourceLayers& subresource) const {
    const char* action = nullptr;
    const char* message_type = nullptr;
    if (IsValueIn(command, {vvl::Func::vkCmdBlitImage, vvl::Func::vkCmdBlitImage2, vvl::Func::vkCmdBlitImage2KHR})) {
        action = "blit";
        message_type = "ImageBlitError";
    } else if (IsValueIn(command,
                         {vvl::Func::vkCmdResolveImage, vvl::Func::vkCmdResolveImage2, vvl::Func::vkCmdResolveImage2KHR})) {
        action = "resolve";
        message_type = "ImageResolveError";
    } else {
        action = "copy";
        message_type = "ImageCopyError";
    }
    std::stringstream ss;
    ss << "\nImage " << action << " region " << region_index << ": {\n";
    ss << "  offset = {" << string_VkOffset3D(offset) << "},\n";
    ss << "  extent = {" << string_VkExtent3D(extent) << "},\n";
    ss << "  subresource = {" << string_VkImageSubresourceLayers(subresource) << "}\n";
    ss << "}\n";

    AdditionalMessageInfo additional_info;
    additional_info.message_end_text = ss.str();
    additional_info.properties.Add(kPropertyRegionIndex, region_index);

    return Error(hazard, cb_context, command, resource_description, message_type, additional_info);
}

std::string ErrorMessages::ImageSubresourceRangeError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                      vvl::Func command, const std::string& resource_description,
                                                      uint32_t subresource_range_index,
                                                      const VkImageSubresourceRange& subresource_range) const {
    std::stringstream ss;
    ss << "Image clear range: index = " << subresource_range_index;
    ss << ", subresource range = {" << string_VkImageSubresourceRange(subresource_range) << "}.";

    AdditionalMessageInfo additional_info;
    additional_info.message_end_text = ss.str();
    additional_info.properties.Add(kPropertyRegionIndex, subresource_range_index);

    return Error(hazard, cb_context, command, resource_description, "ImageSubresourceRangeError", additional_info);
}

std::string ErrorMessages::BufferDescriptorError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                 vvl::Func command, const std::string& resource_description,
                                                 const vvl::Pipeline& pipeline, const vvl::DescriptorSet& descriptor_set,
                                                 VkDescriptorType descriptor_type, uint32_t descriptor_binding,
                                                 uint32_t descriptor_array_element, VkShaderStageFlagBits shader_stage) const {
    const char* descriptor_type_str = string_VkDescriptorType(descriptor_type);

    AdditionalMessageInfo additional_info;
    additional_info.properties.Add(kPropertyDescriptorType, descriptor_type_str);
    additional_info.properties.Add(kPropertyDescriptorBinding, descriptor_binding);
    additional_info.properties.Add(kPropertyDescriptorArrayElement, descriptor_array_element);
    additional_info.access_initiator = std::string("Shader stage ") + string_VkShaderStageFlagBits(shader_stage);

    std::stringstream ss;
    ss << "\nThe buffer is referenced by a ";
    ss << descriptor_type_str << " descriptor in ";
    ss << validator_.FormatHandle(descriptor_set);
    ss << ", binding " << descriptor_binding;
    if (descriptor_set.GetDescriptorCountFromBinding(descriptor_binding) > 1) {
        ss << ", array element " << descriptor_array_element;
    }
    ss << ", " << validator_.FormatHandle(pipeline);
    ss << ".";
    additional_info.pre_synchronization_text = ss.str();

    return Error(hazard, cb_context, command, resource_description, "BufferDescriptorError", additional_info);
}

std::string ErrorMessages::ImageDescriptorError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                vvl::Func command, const std::string& resource_description,
                                                const vvl::Pipeline& pipeline, const vvl::DescriptorSet& descriptor_set,
                                                VkDescriptorType descriptor_type, uint32_t descriptor_binding,
                                                uint32_t descriptor_array_element, VkShaderStageFlagBits shader_stage,
                                                VkImageLayout image_layout) const {
    const char* descriptor_type_str = string_VkDescriptorType(descriptor_type);
    const char* image_layout_str = string_VkImageLayout(image_layout);

    AdditionalMessageInfo additional_info;
    additional_info.properties.Add(kPropertyDescriptorType, descriptor_type_str);
    additional_info.properties.Add(kPropertyDescriptorBinding, descriptor_binding);
    additional_info.properties.Add(kPropertyDescriptorArrayElement, descriptor_array_element);
    additional_info.access_initiator = std::string("Shader stage ") + string_VkShaderStageFlagBits(shader_stage);
    additional_info.properties.Add(kPropertyImageLayout, image_layout_str);

    std::stringstream ss;
    ss << "\nThe image is referenced by a ";
    ss << descriptor_type_str << " descriptor in ";
    ss << validator_.FormatHandle(descriptor_set);
    ss << ", binding " << descriptor_binding;
    if (descriptor_set.GetDescriptorCountFromBinding(descriptor_binding) > 1) {
        ss << ", array element " << descriptor_array_element;
    }
    ss << ", image layout " << string_VkImageLayout(image_layout);
    ss << ", " << validator_.FormatHandle(pipeline);
    ss << ".";
    additional_info.pre_synchronization_text = ss.str();

    return Error(hazard, cb_context, command, resource_description, "ImageDescriptorError", additional_info);
}

std::string ErrorMessages::ClearAttachmentError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                vvl::Func command, const std::string& resource_description,
                                                VkImageAspectFlagBits aspect, uint32_t clear_rect_index,
                                                const VkClearRect& clear_rect) const {
    std::stringstream ss;
    ss << "\nClear region: {\n";
    ss << "  region_index = " << clear_rect_index << ",\n";
    ss << "  rect = {" << string_VkRect2D(clear_rect.rect) << "},\n";
    ss << "  baseArrayLayer = " << clear_rect.baseArrayLayer << ",\n";
    ss << "  layerCount = " << clear_rect.layerCount << "\n";
    ss << "}\n";

    AdditionalMessageInfo additional_info;
    additional_info.properties.Add(kPropertyImageAspect, string_VkImageAspectFlagBits(aspect));
    additional_info.access_action = "clears";
    additional_info.message_end_text = ss.str();

    return Error(hazard, cb_context, command, resource_description, "ClearAttachmentError", additional_info);
}

std::string ErrorMessages::RenderPassAttachmentError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                     vvl::Func command, const std::string& resource_description) const {
    // TODO: revisit error message when this function is covered by the tests.
    return Error(hazard, cb_context, command, resource_description, "RenderPassAttachmentError");
}

static const char* GetLoadOpActionName(VkAttachmentLoadOp load_op) {
    if (load_op == VK_ATTACHMENT_LOAD_OP_LOAD) {
        return "reads";
    } else if (load_op == VK_ATTACHMENT_LOAD_OP_CLEAR) {
        return "clears";
    } else if (load_op == VK_ATTACHMENT_LOAD_OP_DONT_CARE) {
        return "potentially modifies";
    }
    // If custon action name is not specified then it will be derived from hazard type (read or write)
    return "";
}

static void CheckForLoadOpDontCareInsight(VkAttachmentLoadOp load_op, bool is_color, std::string& message_end_text) {
    if (load_op == VK_ATTACHMENT_LOAD_OP_DONT_CARE) {
        std::stringstream ss;
        ss << "\nVulkan insight: according to the specification VK_ATTACHMENT_LOAD_OP_DONT_CARE is a write access (";
        if (is_color) {
            ss << "VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT for color attachment";
        } else {
            ss << "VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT for depth/stencil attachment";
        }
        ss << ").";
        message_end_text += ss.str();
    }
}

std::string ErrorMessages::BeginRenderingError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                               vvl::Func command, const std::string& resource_description,
                                               VkAttachmentLoadOp load_op) const {
    AdditionalMessageInfo additional_info;
    const char* load_op_str = string_VkAttachmentLoadOp(load_op);
    additional_info.properties.Add(kPropertyLoadOp, load_op_str);
    additional_info.access_action = GetLoadOpActionName(load_op);
    return Error(hazard, cb_context, command, resource_description, "BeginRenderingError", additional_info);
}

std::string ErrorMessages::EndRenderingResolveError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                    vvl::Func command, const std::string& resource_description,
                                                    VkResolveModeFlagBits resolve_mode, bool resolve_write) const {
    AdditionalMessageInfo additional_info;
    const char* resolve_mode_str = string_VkResolveModeFlagBits(resolve_mode);
    additional_info.properties.Add(kPropertyResolveMode, resolve_mode_str);
    additional_info.access_action = resolve_write ? "writes to single sample resolve attachment" : "reads multisample attachment";
    return Error(hazard, cb_context, command, resource_description, "EndRenderingResolveError", additional_info);
}

std::string ErrorMessages::EndRenderingStoreError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                  vvl::Func command, const std::string& resource_description,
                                                  VkAttachmentStoreOp store_op) const {
    AdditionalMessageInfo additional_info;
    const char* store_op_str = string_VkAttachmentStoreOp(store_op);
    additional_info.properties.Add(kPropertyStoreOp, store_op_str);
    return Error(hazard, cb_context, command, resource_description, "EndRenderingStoreError", additional_info);
}

std::string ErrorMessages::RenderPassLoadOpError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                 vvl::Func command, const std::string& resource_description, uint32_t subpass,
                                                 uint32_t attachment, VkAttachmentLoadOp load_op, bool is_color) const {
    AdditionalMessageInfo additional_info;
    const char* load_op_str = string_VkAttachmentLoadOp(load_op);
    additional_info.properties.Add(kPropertyLoadOp, load_op_str);
    additional_info.access_action = GetLoadOpActionName(load_op);
    CheckForLoadOpDontCareInsight(load_op, is_color, additional_info.message_end_text);
    return Error(hazard, cb_context, command, resource_description, "RenderPassLoadOpError", additional_info);
}

std::string ErrorMessages::RenderPassLoadOpVsLayoutTransitionError(const HazardResult& hazard,
                                                                   const CommandBufferAccessContext& cb_context, vvl::Func command,
                                                                   const std::string& resource_description,
                                                                   VkAttachmentLoadOp load_op, bool is_color) const {
    AdditionalMessageInfo additional_info;
    const char* load_op_str = string_VkAttachmentLoadOp(load_op);
    additional_info.properties.Add(kPropertyLoadOp, load_op_str);
    additional_info.hazard_overview = "attachment loadOp access is not synchronized with the attachment layout transition";
    additional_info.access_action = GetLoadOpActionName(load_op);
    CheckForLoadOpDontCareInsight(load_op, is_color, additional_info.message_end_text);
    return Error(hazard, cb_context, command, resource_description, "RenderPassLoadOpVsLayoutTransitionError", additional_info);
}

std::string ErrorMessages::RenderPassResolveError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                  vvl::Func command, const std::string& resource_description) const {
    // TODO: rework error message and maybe refactor ValidateResolveAction helper when this function is covered by the tests.
    return Error(hazard, cb_context, command, resource_description, "RenderPassResolveError");
}

// TODO: this one also does not have tests!
std::string ErrorMessages::RenderPassStoreOpError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                  vvl::Func command, const std::string& resource_description,
                                                  VkAttachmentStoreOp store_op) const {
    AdditionalMessageInfo additional_info;
    const char* store_op_str = string_VkAttachmentStoreOp(store_op);
    additional_info.properties.Add(kPropertyStoreOp, store_op_str);
    return Error(hazard, cb_context, command, resource_description, "RenderPassStoreOpError", additional_info);
}

std::string ErrorMessages::RenderPassLayoutTransitionError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                           vvl::Func command, const std::string& resource_description,
                                                           VkImageLayout old_layout, VkImageLayout new_layout) const {
    const char* old_layout_str = string_VkImageLayout(old_layout);
    const char* new_layout_str = string_VkImageLayout(new_layout);

    AdditionalMessageInfo additional_info;
    additional_info.properties.Add(kPropertyOldLayout, old_layout_str);
    additional_info.properties.Add(kPropertyNewLayout, new_layout_str);
    additional_info.access_action = "performs image layout transition";
    return Error(hazard, cb_context, command, resource_description, "RenderPassLayoutTransitionError", additional_info);
}

std::string ErrorMessages::RenderPassLayoutTransitionVsStoreOrResolveError(const HazardResult& hazard,
                                                                           const CommandBufferAccessContext& cb_context,
                                                                           vvl::Func command,
                                                                           const std::string& resource_description,
                                                                           VkImageLayout old_layout, VkImageLayout new_layout,
                                                                           uint32_t store_resolve_subpass) const {
    const char* old_layout_str = string_VkImageLayout(old_layout);
    const char* new_layout_str = string_VkImageLayout(new_layout);

    AdditionalMessageInfo additional_info;
    additional_info.properties.Add(kPropertyOldLayout, old_layout_str);
    additional_info.properties.Add(kPropertyNewLayout, new_layout_str);
    additional_info.access_action = "performs image layout transition";
    additional_info.brief_description_end_text = "during store/resolve operation in subpass ";
    additional_info.brief_description_end_text += std::to_string(store_resolve_subpass);

    return Error(hazard, cb_context, command, resource_description, "RenderPassLayoutTransitionVsStoreOrResolveError",
                 additional_info);
}

std::string ErrorMessages::RenderPassFinalLayoutTransitionError(const HazardResult& hazard,
                                                                const CommandBufferAccessContext& cb_context, vvl::Func command,
                                                                const std::string& resource_description, VkImageLayout old_layout,
                                                                VkImageLayout new_layout) const {
    const char* old_layout_str = string_VkImageLayout(old_layout);
    const char* new_layout_str = string_VkImageLayout(new_layout);

    AdditionalMessageInfo additional_info;
    additional_info.properties.Add(kPropertyOldLayout, old_layout_str);
    additional_info.properties.Add(kPropertyNewLayout, new_layout_str);
    additional_info.access_action = "performs final image layout transition";
    return Error(hazard, cb_context, command, resource_description, "RenderPassFinalLayoutTransitionError", additional_info);
}

std::string ErrorMessages::RenderPassFinalLayoutTransitionVsStoreOrResolveError(const HazardResult& hazard,
                                                                                const CommandBufferAccessContext& cb_context,
                                                                                vvl::Func command,
                                                                                const std::string& resource_description,
                                                                                VkImageLayout old_layout, VkImageLayout new_layout,
                                                                                uint32_t store_resolve_subpass) const {
    const char* old_layout_str = string_VkImageLayout(old_layout);
    const char* new_layout_str = string_VkImageLayout(new_layout);

    AdditionalMessageInfo additional_info;
    additional_info.properties.Add(kPropertyOldLayout, old_layout_str);
    additional_info.properties.Add(kPropertyNewLayout, new_layout_str);
    additional_info.access_action = "performs final image layout transition";
    additional_info.brief_description_end_text = "during store/resolve operation in subpass ";
    additional_info.brief_description_end_text += std::to_string(store_resolve_subpass);

    return Error(hazard, cb_context, command, resource_description, "RenderPassFinalLayoutTransitionVsStoreOrResolveError",
                 additional_info);
}

std::string ErrorMessages::ImageBarrierError(const HazardResult& hazard, const CommandExecutionContext& context, vvl::Func command,
                                             const std::string& resource_description, const SyncImageMemoryBarrier& barrier) const {
    AdditionalMessageInfo additional_info;
    additional_info.access_action = "performs image layout transition on the";

    std::stringstream ss;
    ss << "\npImageMemoryBarriers[" << barrier.barrier_index << "]: {\n";
    ss << "  source accesses = "
       << FormatSyncAccesses(barrier.barrier.src_access_scope, context.GetQueueFlags(), context.GetSyncState().enabled_features,
                             context.GetSyncState().extensions, false)
       << ",\n";
    ss << "  destination accesses = "
       << FormatSyncAccesses(barrier.barrier.dst_access_scope, context.GetQueueFlags(), context.GetSyncState().enabled_features,
                             context.GetSyncState().extensions, false)
       << ",\n";
    ss << "  srcStageMask = " << string_VkPipelineStageFlags2(barrier.barrier.src_exec_scope.mask_param) << ",\n";
    ss << "  dstStageMask = " << string_VkPipelineStageFlags2(barrier.barrier.dst_exec_scope.mask_param) << ",\n";
    ss << "}\n";
    additional_info.message_end_text = ss.str();

    return Error(hazard, context, command, resource_description, "ImageBarrierError", additional_info);
}

std::string ErrorMessages::FirstUseError(const HazardResult& hazard, const CommandExecutionContext& exec_context,
                                         const CommandBufferAccessContext& recorded_context, uint32_t command_buffer_index) const {
    const ReportUsageInfo report_info = recorded_context.GetReportUsageInfo(hazard.RecordedAccess()->TagEx());
    const ReportUsageInfo exec_info = exec_context.GetReportUsageInfo(hazard.TagEx());

    AdditionalMessageInfo additional_info;
    additional_info.properties.Add(kPropertyCommandBufferIndex, command_buffer_index);

    std::stringstream ss;
    ss << vvl::String(report_info.command);
    if (!report_info.debug_region_name.empty()) {
        ss << "[" << report_info.debug_region_name << "]";
    }
    if (exec_info.queue) {
        ss << " (from " << validator_.FormatHandle(recorded_context.Handle());
        ss << " submitted on the current ";
        ss << validator_.FormatHandle(exec_info.queue->Handle()) << ")";
        additional_info.properties.Add(kPropertySubmitIndex, exec_info.submit_index);
        additional_info.properties.Add(kPropertyBatchIndex, exec_info.batch_index);
    } else {
        ss << " (from the secondary " << validator_.FormatHandle(recorded_context.Handle()) << ")";
    }
    additional_info.access_initiator = ss.str();

    std::stringstream ss2;
    if (exec_context.Handle().type == kVulkanObjectTypeQueue) {
        if (exec_info.cb) {
            ss2 << "(from " << validator_.FormatHandle(exec_info.cb->Handle());
            ss2 << " submitted on " << validator_.FormatHandle(exec_context.Handle()) << ")";
        } else {  // QueuePresent case (not recorded into command buffer)
            ss2 << "(submitted on " << validator_.FormatHandle(exec_context.Handle()) << ")";
        }
    } else {  // primary command buffer executes secondary one
        assert(exec_context.Handle().type == kVulkanObjectTypeCommandBuffer);
        // TODO: distinuish between "native" primary command buffer commands and
        // command recorded from the secondary command buffers.
        ss2 << "(from the primary " << validator_.FormatHandle(exec_context.Handle()) << ")";
    }
    additional_info.brief_description_end_text = ss2.str();

    if (!report_info.debug_region_name.empty()) {
        additional_info.properties.Add(kPropertyDebugRegion, report_info.debug_region_name);
    }

    // Use generic "resource" when resource handle is not specified for some reason (likely just a missing code).
    // TODO: specify resources in EndRenderPass (NegativeSyncVal.QSOBarrierHazard).
    const std::string resource_description =
        report_info.resource_handle ? validator_.FormatHandle(report_info.resource_handle) : "resource";
    return Error(hazard, exec_context, report_info.command, resource_description, "SubmitTimeError", additional_info);
}

std::string ErrorMessages::PresentError(const HazardResult& hazard, const QueueBatchContext& batch_context, vvl::Func command,
                                        const std::string& resource_description, uint32_t swapchain_index) const {
    AdditionalMessageInfo additional_info;
    additional_info.access_action = "presents";
    additional_info.properties.Add(kPropertySwapchainIndex, swapchain_index);
    return Error(hazard, batch_context, command, resource_description, "PresentError", additional_info);
}

std::string ErrorMessages::VideoError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context, vvl::Func command,
                                      const std::string& resource_description) const {
    return Error(hazard, cb_context, command, resource_description, "VideoError");
}

}  // namespace syncval
