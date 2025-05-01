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
#include "sync/sync_validation.h"
#include "error_message/error_strings.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/pipeline_state.h"

#include <cassert>
#include <cinttypes>
#include <sstream>

namespace syncval {

ErrorMessages::ErrorMessages(SyncValidator& validator) : validator_(validator) {}

std::string ErrorMessages::Error(const HazardResult& hazard, const CommandExecutionContext& context, vvl::Func command,
                                 const std::string& resource_description, const char* message_type,
                                 const AdditionalMessageInfo& additional_info) const {
    std::string message = FormatErrorMessage(hazard, context, command, resource_description, additional_info);

    if (validator_.syncval_settings.message_extra_properties) {
        if (!message.empty() && message.back() != '\n') {
            message += '\n';
        }
        const ReportProperties properties = GetErrorMessageProperties(hazard, context, command, message_type, additional_info);
        message += properties.FormatExtraPropertiesSection();
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

std::string ErrorMessages::ImageClearError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                      vvl::Func command, const std::string& resource_description,
                                                      uint32_t subresource_range_index,
                                                      const VkImageSubresourceRange& subresource_range) const {
    std::stringstream ss;
    ss << "\nImage clear subresource range " << subresource_range_index << ": {\n";
    ss << "  " << string_VkImageSubresourceRange(subresource_range) << "\n";
    ss << "}\n";

    AdditionalMessageInfo additional_info;
    additional_info.message_end_text = ss.str();
    additional_info.properties.Add(kPropertyRegionIndex, subresource_range_index);

    return Error(hazard, cb_context, command, resource_description, "ImageSubresourceRangeError", additional_info);
}

static void PrepareCommonDescriptorMessage(Logger& logger, const vvl::Pipeline& pipeline, uint32_t descriptor_set_number,
                                           const vvl::DescriptorSet& descriptor_set, VkDescriptorType descriptor_type,
                                           uint32_t descriptor_binding, uint32_t descriptor_array_element,
                                           VkShaderStageFlagBits shader_stage, const char* resource_type,
                                           AdditionalMessageInfo& additional_info, std::stringstream& ss) {
    const char* descriptor_type_str = string_VkDescriptorType(descriptor_type);

    additional_info.properties.Add(kPropertyDescriptorType, descriptor_type_str);
    additional_info.properties.Add(kPropertyDescriptorBinding, descriptor_binding);
    additional_info.properties.Add(kPropertyDescriptorArrayElement, descriptor_array_element);
    additional_info.access_initiator = std::string("Shader stage ") + string_VkShaderStageFlagBits(shader_stage);

    ss << "\nThe " << resource_type << " is referenced by descriptor binding " << descriptor_binding;
    ss << " (" << descriptor_type_str << ")";
    if (descriptor_set.GetDescriptorCountFromBinding(descriptor_binding) > 1) {
        ss << ", array element " << descriptor_array_element;
    }
    ss << " from descriptor set " << descriptor_set_number << " (" << logger.FormatHandle(descriptor_set) << ")";
    ss << ", " << logger.FormatHandle(pipeline);
}

std::string ErrorMessages::BufferDescriptorError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                 vvl::Func command, const std::string& resource_description,
                                                 const vvl::Pipeline& pipeline, uint32_t set_number,
                                                 const vvl::DescriptorSet& descriptor_set, VkDescriptorType descriptor_type,
                                                 uint32_t descriptor_binding, uint32_t descriptor_array_element,
                                                 VkShaderStageFlagBits shader_stage) const {
    AdditionalMessageInfo additional_info;
    std::stringstream ss;
    PrepareCommonDescriptorMessage(validator_, pipeline, set_number, descriptor_set, descriptor_type, descriptor_binding,
                                   descriptor_array_element, shader_stage, "buffer", additional_info, ss);
    ss << ".";

    additional_info.pre_synchronization_text = ss.str();
    return Error(hazard, cb_context, command, resource_description, "BufferDescriptorError", additional_info);
}

std::string ErrorMessages::ImageDescriptorError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                vvl::Func command, const std::string& resource_description,
                                                const vvl::Pipeline& pipeline, uint32_t set_number,
                                                const vvl::DescriptorSet& descriptor_set, VkDescriptorType descriptor_type,
                                                uint32_t descriptor_binding, uint32_t descriptor_array_element,
                                                VkShaderStageFlagBits shader_stage, VkImageLayout image_layout) const {
    AdditionalMessageInfo additional_info;
    std::stringstream ss;
    PrepareCommonDescriptorMessage(validator_, pipeline, set_number, descriptor_set, descriptor_type, descriptor_binding,
                                   descriptor_array_element, shader_stage, "image", additional_info, ss);
    ss << ", image layout " << string_VkImageLayout(image_layout) << ".";

    additional_info.pre_synchronization_text = ss.str();
    additional_info.properties.Add(kPropertyImageLayout, string_VkImageLayout(image_layout));
    return Error(hazard, cb_context, command, resource_description, "ImageDescriptorError", additional_info);
}

std::string ErrorMessages::AccelerationStructureDescriptorError(
    const HazardResult& hazard, const CommandBufferAccessContext& cb_context, vvl::Func command,
    const std::string& resource_description, const vvl::Pipeline& pipeline, uint32_t set_number,
    const vvl::DescriptorSet& descriptor_set, VkDescriptorType descriptor_type, uint32_t descriptor_binding,
    uint32_t descriptor_array_element, VkShaderStageFlagBits shader_stage) const {
    AdditionalMessageInfo additional_info;
    additional_info.access_action = "traces rays against";

    std::stringstream ss;
    PrepareCommonDescriptorMessage(validator_, pipeline, set_number, descriptor_set, descriptor_type, descriptor_binding,
                                   descriptor_array_element, shader_stage, "acceleration structure", additional_info, ss);
    ss << ".";
    additional_info.pre_synchronization_text = ss.str();

    return Error(hazard, cb_context, command, resource_description, "AccelerationStructureDescriptorError", additional_info);
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
       << FormatSyncAccesses(barrier.barrier.src_access_scope, context.GetSyncState(), context.GetQueueFlags(), false) << ",\n";
    ss << "  destination accesses = "
       << FormatSyncAccesses(barrier.barrier.dst_access_scope, context.GetSyncState(), context.GetQueueFlags(), false) << ",\n";
    ss << "  srcStageMask = " << string_VkPipelineStageFlags2(barrier.barrier.src_exec_scope.mask_param) << ",\n";
    ss << "  dstStageMask = " << string_VkPipelineStageFlags2(barrier.barrier.dst_exec_scope.mask_param) << ",\n";
    ss << "}\n";
    additional_info.message_end_text = ss.str();

    return Error(hazard, context, command, resource_description, "ImageBarrierError", additional_info);
}

std::string ErrorMessages::FirstUseError(const HazardResult& hazard, const CommandExecutionContext& exec_context,
                                         const CommandBufferAccessContext& recorded_context, uint32_t command_buffer_index) const {
    const ResourceUsageInfo prior_usage_info = exec_context.GetResourceUsageInfo(hazard.TagEx());
    const ResourceUsageInfo recorded_usage_info = recorded_context.GetResourceUsageInfo(hazard.RecordedAccess()->TagEx());

    AdditionalMessageInfo additional_info;
    additional_info.properties.Add(kPropertyCommandBufferIndex, command_buffer_index);

    std::stringstream ss;
    ss << vvl::String(recorded_usage_info.command);
    if (!recorded_usage_info.debug_region_name.empty()) {
        ss << "[" << recorded_usage_info.debug_region_name << "]";
    }
    if (exec_context.Handle().type == kVulkanObjectTypeQueue) {
        ss << " (from " << validator_.FormatHandle(recorded_context.Handle());
        ss << " submitted on the current ";
        ss << validator_.FormatHandle(exec_context.Handle()) << ")";
    } else {  // primary command buffer executes secondary one
        assert(exec_context.Handle().type == kVulkanObjectTypeCommandBuffer);
        ss << " (from the secondary " << validator_.FormatHandle(recorded_context.Handle()) << ")";
    }
    additional_info.access_initiator = ss.str();

    std::stringstream ss2;
    if (prior_usage_info.queue) {
        if (prior_usage_info.cb) {
            ss2 << "(from " << validator_.FormatHandle(prior_usage_info.cb->Handle());
            ss2 << " submitted on " << validator_.FormatHandle(prior_usage_info.queue->Handle()) << ")";
        } else {  // QueuePresent case (not recorded into command buffer)
            ss2 << "(submitted on " << validator_.FormatHandle(prior_usage_info.queue->Handle()) << ")";
        }
    } else if (prior_usage_info.cb) {
        // TODO: distinuish between "native" primary command buffer commands and
        // command recorded from the secondary command buffers.
        ss2 << "(from the primary " << validator_.FormatHandle(prior_usage_info.cb->Handle()) << ")";
    }
    additional_info.brief_description_end_text = ss2.str();

    if (!recorded_usage_info.debug_region_name.empty()) {
        additional_info.properties.Add(kPropertyDebugRegion, recorded_usage_info.debug_region_name);
    }

    // Use generic "resource" when resource handle is not specified for some reason (likely just a missing code).
    // TODO: specify resources in EndRenderPass (NegativeSyncVal.QSOBarrierHazard).
    const std::string resource_description =
        recorded_usage_info.resource_handle ? validator_.FormatHandle(recorded_usage_info.resource_handle) : "resource";
    return Error(hazard, exec_context, recorded_usage_info.command, resource_description, "SubmitTimeError", additional_info);
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
