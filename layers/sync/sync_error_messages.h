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

#include "sync/sync_renderpass.h"  // DynamicRenderingInfo::Attachment
#include "sync/sync_reporting.h"

#include <vulkan/vulkan.h>
#include <string>

class CommandBufferAccessContext;
class HazardResult;
struct ReportKeyValues;
class QueueBatchContext;

namespace vvl {
class DescriptorSet;
class Device;
class Pipeline;
}  // namespace vvl

namespace syncval {

struct AdditionalMessageInfo {
    ReportKeyValues properties;

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

class ErrorMessages {
  public:
    explicit ErrorMessages(vvl::Device& validator);

    std::string Error(const HazardResult& hazard, const CommandExecutionContext& context, vvl::Func command,
                      const std::string& resouce_description, const char* message_type,
                      const AdditionalMessageInfo& additional_info = {}) const;

    std::string BufferError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context, vvl::Func command,
                            const std::string& resource_description, const ResourceAccessRange range,
                            AdditionalMessageInfo additional_info = {}) const;

    std::string BufferCopyError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context, const vvl::Func command,
                                const std::string& resouce_description, uint32_t region_index, ResourceAccessRange range) const;

    std::string ImageCopyResolveBlitError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                          vvl::Func command, const std::string& resource_description, uint32_t region_index,
                                          const VkOffset3D& offset, const VkExtent3D& extent,
                                          const VkImageSubresourceLayers& subresource) const;

    std::string ImageSubresourceRangeError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                           vvl::Func command, const std::string& resource_description,
                                           uint32_t subresource_range_index,
                                           const VkImageSubresourceRange& subresource_range) const;

    std::string BufferDescriptorError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context, vvl::Func command,
                                      const std::string& resource_description, const vvl::Pipeline& pipeline,
                                      const vvl::DescriptorSet& descriptor_set, VkDescriptorType descriptor_type,
                                      uint32_t descriptor_binding, uint32_t descriptor_array_element,
                                      VkShaderStageFlagBits shader_stage) const;

    std::string ImageDescriptorError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context, vvl::Func command,
                                     const std::string& resource_description, const vvl::Pipeline& pipeline,
                                     const vvl::DescriptorSet& descriptor_set, VkDescriptorType descriptor_type,
                                     uint32_t descriptor_binding, uint32_t descriptor_array_element,
                                     VkShaderStageFlagBits shader_stage, VkImageLayout image_layout) const;

    std::string ClearAttachmentError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context, vvl::Func command,
                                     const std::string& resource_description, VkImageAspectFlagBits aspect,
                                     uint32_t clear_rect_index, const VkClearRect& clear_rect) const;

    std::string RenderPassAttachmentError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                          vvl::Func command, const std::string& resource_description) const;

    std::string BeginRenderingError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context, vvl::Func command,
                                    const std::string& resource_description, VkAttachmentLoadOp load_op) const;
    std::string EndRenderingResolveError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                         vvl::Func command, const std::string& resource_description,
                                         VkResolveModeFlagBits resolve_mode, bool resolve_write) const;
    std::string EndRenderingStoreError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context, vvl::Func command,
                                       const std::string& resource_description, VkAttachmentStoreOp store_op) const;

    std::string RenderPassLoadOpError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context, vvl::Func command,
                                      const std::string& resource_description, uint32_t subpass, uint32_t attachment,
                                      VkAttachmentLoadOp load_op, bool is_color) const;
    std::string RenderPassLoadOpVsLayoutTransitionError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                        vvl::Func command, const std::string& resource_description,
                                                        VkAttachmentLoadOp load_op, bool is_color) const;
    std::string RenderPassResolveError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context, vvl::Func command,
                                       const std::string& resource_description) const;
    std::string RenderPassStoreOpError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context, vvl::Func command,
                                       const std::string& resource_description, VkAttachmentStoreOp store_op) const;


    std::string RenderPassLayoutTransitionError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                vvl::Func command, const std::string& resource_description,
                                                VkImageLayout old_layout, VkImageLayout new_layout) const;
    std::string RenderPassLayoutTransitionVsStoreOrResolveError(const HazardResult& hazard,
                                                                const CommandBufferAccessContext& cb_context, vvl::Func command,
                                                                const std::string& resource_description, VkImageLayout old_layout,
                                                                VkImageLayout new_layout, uint32_t store_resolve_subpass) const;
    std::string RenderPassFinalLayoutTransitionError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                     vvl::Func command, const std::string& resource_description,
                                                     VkImageLayout old_layout, VkImageLayout new_layout) const;
    std::string RenderPassFinalLayoutTransitionVsStoreOrResolveError(const HazardResult& hazard,
                                                                     const CommandBufferAccessContext& cb_context,
                                                                     vvl::Func command, const std::string& resource_description,
                                                                     VkImageLayout old_layout, VkImageLayout new_layout,
                                                                     uint32_t store_resolve_subpass) const;

    std::string ImageBarrierError(const HazardResult& hazard, const CommandExecutionContext& context, vvl::Func command,
                                  const std::string& resource_description, const SyncImageMemoryBarrier& barrier) const;

    std::string FirstUseError(const HazardResult& hazard, const CommandExecutionContext& exec_context,
                              const CommandBufferAccessContext& recorded_context, uint32_t command_buffer_index) const;

    std::string PresentError(const HazardResult& hazard, const QueueBatchContext& batch_context, vvl::Func command,
                             const std::string& resource_description, uint32_t swapchain_index) const;

    std::string VideoError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context, vvl::Func command,
                           const std::string& resource_description) const;

  private:
    vvl::Device& validator_;
    const bool& extra_properties_;
    const bool& pretty_print_extra_;
};

}  // namespace syncval
