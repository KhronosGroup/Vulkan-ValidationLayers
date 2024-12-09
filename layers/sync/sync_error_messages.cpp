/* Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
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
#include "state_tracker/buffer_state.h"
#include "state_tracker/descriptor_sets.h"

#include <cassert>
#include <cinttypes>
#include <cstdarg>
#include <sstream>

// TODO: update algorith in logging.cpp to be similar to this version.
// logging.cpp's vsnprintf usage in some places assumes that std::string
// storage reserves space for null terminator but it is not guaranteed
// (although it is in practise). vsnprintf there writes 0 terminator
// in the char indexed as str[std::string::size()] and it should not be
// accesses in this way by client code, only by std::string itself.

static std::string Format(const char* format, ...) {
    const int initial_max_symbol_count = 1024;
    std::vector<char> buffer(initial_max_symbol_count + 1 /*null terminator*/);

    va_list argptr;
    va_start(argptr, format);

    // The va_list will be destroyed by the call to vsnprintf(), so use a copy in case we need to try again.
    va_list argptr2;
    va_copy(argptr2, argptr);
    const int symbol_count = vsnprintf(buffer.data(), buffer.size(), format, argptr2);
    va_end(argptr2);

    if (symbol_count < 0) {
        assert(false && "Synchronization validation error formatting error");
        va_end(argptr);
        return {};
    }
    if (symbol_count > initial_max_symbol_count) {
        buffer.resize(symbol_count + 1 /*null terminator*/);
        vsnprintf(buffer.data(), buffer.size(), format, argptr);
    }
    va_end(argptr);
    const std::string message(buffer.data());
    return message;
}

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

namespace syncval {

void ErrorMessages::AddCbContextExtraProperties(const CommandBufferAccessContext& cb_context, ResourceUsageTag tag,
                                                std::string& message) const {
    if (validator_.syncval_settings.message_extra_properties) {
        ReportKeyValues key_values;
        cb_context.AddUsageRecordExtraProperties(tag, key_values);
        message += key_values.GetExtraPropertiesSection();
    }
}

std::string ErrorMessages::Error(const HazardResult& hazard, const char* description,
                                 const CommandBufferAccessContext& cb_context) const {
    const auto format = "Hazard %s for %s. Access info %s.";
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), description, cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::BufferError(const HazardResult& hazard, VkBuffer buffer, const char* buffer_description,
                                       const CommandBufferAccessContext& cb_context) const {
    const auto format = "Hazard %s for %s %s. Access info %s.";
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), buffer_description,
                                 validator_.FormatHandle(buffer).c_str(), cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::BufferRegionError(const HazardResult& hazard, VkBuffer buffer, bool is_src_buffer, uint32_t region_index,
                                             const CommandBufferAccessContext& cb_context) const {
    const auto format = "Hazard %s for %s %s, region %" PRIu32 ". Access info %s.";
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), is_src_buffer ? "srcBuffer" : "dstBuffer",
                                 validator_.FormatHandle(buffer).c_str(), region_index, cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::ImageRegionError(const HazardResult& hazard, VkImage image, bool is_src_image, uint32_t region_index,
                                            const CommandBufferAccessContext& cb_context) const {
    const auto format = "Hazard %s for %s %s, region %" PRIu32 ". Access info %s.";
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), is_src_image ? "srcImage" : "dstImage",
                                 validator_.FormatHandle(image).c_str(), region_index, cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::ImageSubresourceRangeError(const HazardResult& hazard, VkImage image, uint32_t subresource_range_index,
                                                      const CommandBufferAccessContext& cb_context) const {
    const auto format = "Hazard %s for %s, range index %" PRIu32 ". Access info %s.";
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), validator_.FormatHandle(image).c_str(),
                                 subresource_range_index, cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::BeginRenderingError(const HazardResult& hazard,
                                               const syncval_state::DynamicRenderingInfo::Attachment& attachment,
                                               const CommandBufferAccessContext& cb_context) const {
    const auto format = "(%s), with loadOp %s. Access info %s.";
    std::string message = Format(format, validator_.FormatHandle(attachment.view->Handle()).c_str(),
                                 string_VkAttachmentLoadOp(attachment.info.loadOp), cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::EndRenderingResolveError(const HazardResult& hazard, const VulkanTypedHandle& image_view_handle,
                                                    VkResolveModeFlagBits resolve_mode,
                                                    const CommandBufferAccessContext& cb_context) const {
    const auto format = "(%s), during resolve with resolveMode %s. Access info %s.";
    std::string message = Format(format, validator_.FormatHandle(image_view_handle).c_str(),
                                 string_VkResolveModeFlagBits(resolve_mode), cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::EndRenderingStoreError(const HazardResult& hazard, const VulkanTypedHandle& image_view_handle,
                                                  VkAttachmentStoreOp store_op,
                                                  const CommandBufferAccessContext& cb_context) const {
    const auto format = "(%s), during store with storeOp %s. Access info %s.";
    std::string message = Format(format, validator_.FormatHandle(image_view_handle).c_str(), string_VkAttachmentStoreOp(store_op),
                                 cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::DrawDispatchImageError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                  const vvl::ImageView& image_view, const vvl::Pipeline& pipeline,
                                                  const vvl::DescriptorSet& descriptor_set, VkDescriptorType descriptor_type,
                                                  VkImageLayout image_layout, uint32_t descriptor_binding,
                                                  uint32_t binding_index) const {
    const auto format =
        "Hazard %s for %s, in %s, and %s, %s, type: %s, imageLayout: %s, binding #%" PRIu32 ", index %" PRIu32 ". Access info %s.";
    std::string message =
        Format(format, string_SyncHazard(hazard.Hazard()), validator_.FormatHandle(image_view.Handle()).c_str(),
               validator_.FormatHandle(cb_context.Handle()).c_str(), validator_.FormatHandle(pipeline.Handle()).c_str(),
               validator_.FormatHandle(descriptor_set.Handle()).c_str(), string_VkDescriptorType(descriptor_type),
               string_VkImageLayout(image_layout), descriptor_binding, binding_index, cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::DrawDispatchTexelBufferError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                        const vvl::BufferView& buffer_view, const vvl::Pipeline& pipeline,
                                                        const vvl::DescriptorSet& descriptor_set, VkDescriptorType descriptor_type,
                                                        uint32_t descriptor_binding, uint32_t binding_index) const {
    const auto format = "Hazard %s for %s in %s, %s, and %s, type: %s, binding #%" PRIu32 " index %" PRIu32 ". Access info %s.";
    std::string message =
        Format(format, string_SyncHazard(hazard.Hazard()), validator_.FormatHandle(buffer_view.Handle()).c_str(),
               validator_.FormatHandle(cb_context.Handle()).c_str(), validator_.FormatHandle(pipeline.Handle()).c_str(),
               validator_.FormatHandle(descriptor_set.Handle()).c_str(), string_VkDescriptorType(descriptor_type),
               descriptor_binding, binding_index, cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::DrawDispatchBufferError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                   const vvl::Buffer& buffer, const vvl::Pipeline& pipeline,
                                                   const vvl::DescriptorSet& descriptor_set, VkDescriptorType descriptor_type,
                                                   uint32_t descriptor_binding, uint32_t binding_index) const {
    const auto format = "Hazard %s for %s in %s, %s, and %s, type: %s, binding #%" PRIu32 " index %" PRIu32 ". Access info %s.";
    std::string message =
        Format(format, string_SyncHazard(hazard.Hazard()), validator_.FormatHandle(buffer.Handle()).c_str(),
               validator_.FormatHandle(cb_context.Handle()).c_str(), validator_.FormatHandle(pipeline.Handle()).c_str(),
               validator_.FormatHandle(descriptor_set.Handle()).c_str(), string_VkDescriptorType(descriptor_type),
               descriptor_binding, binding_index, cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::DrawVertexBufferError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                 const vvl::Buffer& vertex_buffer) const {
    const auto format = "Hazard %s for vertex %s in %s. Access info %s.";
    std::string message =
        Format(format, string_SyncHazard(hazard.Hazard()), validator_.FormatHandle(vertex_buffer.Handle()).c_str(),
               validator_.FormatHandle(cb_context.Handle()).c_str(), cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::DrawIndexBufferError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                const vvl::Buffer& index_buffer) const {
    const auto format = "Hazard %s for index %s in %s. Access info %s.";
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), validator_.FormatHandle(index_buffer.Handle()).c_str(),
                                 validator_.FormatHandle(cb_context.Handle()).c_str(), cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::DrawAttachmentError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                               const vvl::ImageView& attachment_view) const {
    const auto format = "(%s). Access info %s.";
    std::string message =
        Format(format, validator_.FormatHandle(attachment_view.Handle()).c_str(), cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::ClearColorAttachmentError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                     const std::string& subpass_attachment_info) const {
    const auto format = "Hazard %s while clearing color attachment%s. Access info %s.";
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), subpass_attachment_info.c_str(),
                                 cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::ClearDepthStencilAttachmentError(const HazardResult& hazard,
                                                            const CommandBufferAccessContext& cb_context,
                                                            const std::string& subpass_attachment_info,
                                                            VkImageAspectFlagBits aspect) const {
    const auto format = "Hazard %s when clearing %s aspect of depth-stencil attachment%s. Access info %s.";
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), string_VkImageAspectFlagBits(aspect),
                                 subpass_attachment_info.c_str(), cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::PipelineBarrierError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                uint32_t image_barrier_index, const vvl::Image& image) const {
    const auto format = "Hazard %s for image barrier %" PRIu32 " %s. Access info %s.";
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), image_barrier_index,
                                 validator_.FormatHandle(image.Handle()).c_str(), cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::WaitEventsError(const HazardResult& hazard, const CommandExecutionContext& exec_context,
                                           uint32_t image_barrier_index, const vvl::Image& image) const {
    const auto format = "Hazard %s for image barrier %" PRIu32 " %s. Access info %s.";
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), image_barrier_index,
                                 validator_.FormatHandle(image.Handle()).c_str(), exec_context.FormatHazard(hazard).c_str());
    return message;
}

std::string ErrorMessages::FirstUseError(const HazardResult& hazard, const CommandExecutionContext& exec_context,
                                         const CommandBufferAccessContext& recorded_context, uint32_t command_buffer_index,
                                         VkCommandBuffer recorded_handle) const {
    const auto format = "Hazard %s for entry %" PRIu32 ", %s, %s access info %s. Access info %s.";
    std::string message =
        Format(format, string_SyncHazard(hazard.Hazard()), command_buffer_index, validator_.FormatHandle(recorded_handle).c_str(),
               exec_context.ExecutionTypeString(),
               recorded_context.FormatUsage(exec_context.ExecutionUsageString(), *hazard.RecordedAccess()).c_str(),
               exec_context.FormatHazard(hazard).c_str());
    if (validator_.syncval_settings.message_extra_properties) {
        ReportKeyValues key_values;
        exec_context.AddUsageRecordExtraProperties(hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection();
    }
    return message;
}

std::string ErrorMessages::RenderPassResolveError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                  uint32_t subpass, const char* aspect_name, const char* attachment_name,
                                                  uint32_t src_attachment, uint32_t dst_attachment) const {
    const auto format = "Hazard %s in subpass %" PRIu32 "during %s %s, from attachment %" PRIu32 " to resolve attachment %" PRIu32
                        ". Access info %s.";
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), subpass, aspect_name, attachment_name, src_attachment,
                                 dst_attachment, cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::RenderPassLayoutTransitionVsStoreResolveError(const HazardResult& hazard, uint32_t subpass,
                                                                         uint32_t attachment, VkImageLayout old_layout,
                                                                         VkImageLayout new_layout,
                                                                         uint32_t store_resolve_subpass) const {
    const auto format =
        "Hazard %s in subpass %" PRIu32 " for attachment %" PRIu32
        " image layout transition (old_layout: %s, new_layout: %s) after store/resolve operation in subpass %" PRIu32;
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), subpass, attachment, string_VkImageLayout(old_layout),
                                 string_VkImageLayout(new_layout), store_resolve_subpass);
    return message;
}

std::string ErrorMessages::RenderPassLayoutTransitionError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                           uint32_t subpass, uint32_t attachment, VkImageLayout old_layout,
                                                           VkImageLayout new_layout) const {
    const auto format = "Hazard %s in subpass %" PRIu32 " for attachment %" PRIu32
                        " image layout transition (old_layout: %s, new_layout: %s). Access info %s.";
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), subpass, attachment, string_VkImageLayout(old_layout),
                                 string_VkImageLayout(new_layout), cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::RenderPassLoadOpVsLayoutTransitionError(const HazardResult& hazard, uint32_t subpass,
                                                                   uint32_t attachment, const char* aspect_name,
                                                                   const char* load_op_name) const {
    const auto format =
        "Hazard %s vs. layout transition in subpass %" PRIu32 " for attachment %" PRIu32 " aspect %s during load with loadOp %s.";
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), subpass, attachment, aspect_name, load_op_name);
    return message;
}

std::string ErrorMessages::RenderPassLoadOpError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                 uint32_t subpass, uint32_t attachment, const char* aspect_name,
                                                 const char* load_op_name) const {
    const auto format =
        "Hazard %s in subpass %" PRIu32 " for attachment %" PRIu32 " aspect %s during load with loadOp %s. Access info %s.";
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), subpass, attachment, aspect_name, load_op_name,
                                 cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::RenderPassStoreOpError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                  uint32_t subpass, uint32_t attachment, const char* aspect_name,
                                                  const char* store_op_type_name, const char* store_op_name) const {
    const auto format =
        "Hazard %s in subpass %" PRIu32 " for attachment %" PRIu32 " %s aspect during store with %s %s. Access info %s";
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), subpass, attachment, aspect_name, store_op_type_name,
                                 store_op_name, cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::RenderPassColorAttachmentError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                          const vvl::ImageView& view, uint32_t attachment) const {
    const auto format = "Hazard %s for %s in %s, Subpass #%d, and pColorAttachments #%d. Access info %s.";
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), validator_.FormatHandle(view.Handle()).c_str(),
                                 validator_.FormatHandle(cb_context.GetCBState().Handle()).c_str(),
                                 cb_context.GetCBState().GetActiveSubpass(), attachment, cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::RenderPassDepthStencilAttachmentError(const HazardResult& hazard,
                                                                 const CommandBufferAccessContext& cb_context,
                                                                 const vvl::ImageView& view, bool is_depth) const {
    const auto format = "Hazard %s for %s in %s, Subpass #%d, and %s part of pDepthStencilAttachment. Access info %s.";

    std::string message =
        Format(format, string_SyncHazard(hazard.Hazard()), validator_.FormatHandle(view.Handle()).c_str(),
               validator_.FormatHandle(cb_context.GetCBState().Handle()).c_str(), cb_context.GetCBState().GetActiveSubpass(),
               is_depth ? "depth" : "stencil", cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::RenderPassFinalLayoutTransitionVsStoreResolveError(const HazardResult& hazard,
                                                                              const CommandBufferAccessContext& cb_context,
                                                                              uint32_t subpass, uint32_t attachment,
                                                                              VkImageLayout old_layout,
                                                                              VkImageLayout new_layout) const {
    const auto format = "Hazard %s vs. store/resolve operations in subpass %" PRIu32 " for attachment %" PRIu32
                        " final image layout transition (old_layout: %s, new_layout: %s).";
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), subpass, attachment, string_VkImageLayout(old_layout),
                                 string_VkImageLayout(new_layout));
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::RenderPassFinalLayoutTransitionError(const HazardResult& hazard,
                                                                const CommandBufferAccessContext& cb_context, uint32_t subpass,
                                                                uint32_t attachment, VkImageLayout old_layout,
                                                                VkImageLayout new_layout) const {
    const auto format = "Hazard %s with last use subpass %" PRIu32 " for attachment %" PRIu32
                        " final image layout transition (old_layout: %s, new_layout: %s). Access info %s.";
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), subpass, attachment, string_VkImageLayout(old_layout),
                                 string_VkImageLayout(new_layout), cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

std::string ErrorMessages::PresentError(const HazardResult& hazard, const QueueBatchContext& batch_context, uint32_t present_index,
                                        const VulkanTypedHandle& swapchain_handle, uint32_t image_index,
                                        const VulkanTypedHandle& image_handle) const {
    const auto format =
        "Hazard %s for present pSwapchains[%" PRIu32 "] , swapchain %s, image index %" PRIu32 " %s, Access info %s.";
    std::string message =
        Format(format, string_SyncHazard(hazard.Hazard()), present_index, validator_.FormatHandle(swapchain_handle).c_str(),
               image_index, validator_.FormatHandle(image_handle).c_str(), batch_context.FormatHazard(hazard).c_str());
    return message;
}

std::string ErrorMessages::VideoReferencePictureError(const HazardResult& hazard, uint32_t reference_picture_index,
                                                      const CommandBufferAccessContext& cb_context) const {
    const auto format = "Hazard %s for reference picture #%u. Access info %s.";
    std::string message =
        Format(format, string_SyncHazard(hazard.Hazard()), reference_picture_index, cb_context.FormatHazard(hazard).c_str());
    AddCbContextExtraProperties(cb_context, hazard.Tag(), message);
    return message;
}

}  // namespace syncval
