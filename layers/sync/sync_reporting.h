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

#pragma once

#include "generated/error_location_helper.h"
#include "generated/sync_validation_types.h"
#include "generated/vk_object_types.h"

class CommandExecutionContext;
class HazardResult;
class Logger;
class SyncValidator;

// Collection of named values that describe key information associated with an error message.
// This can be useful to filter out messages or for quick inspection as a more structured (but lose)
// representation of the main error message text. The main error message might change relatively
// often due to wording improvements and minor fixes (e.g. punctuation fix), but the associated
// properties will change less frequently and are more suited for automatic processing.
struct ReportProperties {
    struct NameValue {
        std::string name;
        std::string value;
    };
    std::vector<NameValue> name_values;

    void Add(std::string_view property_name, std::string_view value);
    void Add(std::string_view property_name, uint64_t value);
    std::string FormatExtraPropertiesSection() const;
};

// Customization options to modify the standard form of the synchronization error message.
struct AdditionalMessageInfo {
    // These are message-specific properties. They are combined with common message properties.
    ReportProperties properties;

    // When we need something more complex than vvl::Func
    std::string access_initiator;

    // Replaces standard "writes to"/"reads" access wording.
    // For example, "clears" for a clear operation might be more specific than a write
    std::string access_action;

    std::string hazard_overview;
    std::string brief_description_end_text;
    std::string pre_synchronization_text;
    std::string message_end_text;
};

ReportProperties GetErrorMessageProperties(const HazardResult &hazard, const CommandExecutionContext &context, vvl::Func command,
                                           const char *message_type, const AdditionalMessageInfo &additional_info);

std::string FormatErrorMessage(const HazardResult &hazard, const CommandExecutionContext &context, vvl::Func command,
                               const std::string &resouce_description, const AdditionalMessageInfo &additional_info);

std::string FormatSyncAccesses(const SyncAccessFlags &sync_accesses, const SyncValidator &device, VkQueueFlags allowed_queue_flags,
                               bool format_as_extra_property);

void FormatVideoPictureResouce(const Logger &logger, const VkVideoPictureResourceInfoKHR &video_picture, std::stringstream &ss);
void FormatVideoQuantizationMap(const Logger &logger, const VkVideoEncodeQuantizationMapInfoKHR &quantization_map,
                                std::stringstream &ss);

// Common properties
inline constexpr const char *kPropertyMessageType = "message_type";
inline constexpr const char *kPropertyHazardType = "hazard_type";
inline constexpr const char *kPropertyCommand = "command";
inline constexpr const char *kPropertyPriorCommand = "prior_command";
inline constexpr const char *kPropertyDebugRegion = "debug_region";
inline constexpr const char *kPropertyPriorDebugRegion = "prior_debug_region";
inline constexpr const char *kPropertyAccess = "access";
inline constexpr const char *kPropertyPriorAccess = "prior_access";
inline constexpr const char *kPropertyReadBarriers = "read_barriers";
inline constexpr const char *kPropertyWriteBarriers = "write_barriers";

// Message-specific properties
inline constexpr const char *kPropertyRegionIndex = "region_index";
inline constexpr const char *kPropertyLoadOp = "load_op";
inline constexpr const char *kPropertyStoreOp = "store_op";
inline constexpr const char *kPropertyResolveMode = "resolve_mode";
inline constexpr const char *kPropertyOldLayout = "old_layout";
inline constexpr const char *kPropertyNewLayout = "new_layout";
inline constexpr const char *kPropertyDescriptorType = "descriptor_type";
inline constexpr const char *kPropertyDescriptorBinding = "descriptor_binding";
inline constexpr const char *kPropertyDescriptorArrayElement = "descriptor_array_element";
inline constexpr const char *kPropertyImageLayout = "image_layout";
inline constexpr const char *kPropertyImageAspect = "image_aspect";
inline constexpr const char *kPropertySubmitIndex = "submit_index";
inline constexpr const char *kPropertyBatchIndex = "batch_index";
inline constexpr const char *kPropertyCommandBufferIndex = "command_buffer_index";
inline constexpr const char *kPropertySwapchainIndex = "swapchain_index";

// Debug properties
inline constexpr const char *kPropertySeqNo = "seq_no";
inline constexpr const char *kPropertyResetNo = "reset_no";
inline constexpr const char *kPropertyBatchTag = "batch_tag";
