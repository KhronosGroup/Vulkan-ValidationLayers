/* Copyright (c) 2019-2026 The Khronos Group Inc.
 * Copyright (c) 2019-2026 Valve Corporation
 * Copyright (c) 2019-2026 LunarG, Inc.
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

#include "sync/sync_common.h"
#include "sync/sync_access_context.h"
#include "error_message/error_location.h"
#include <vulkan/utility/vk_safe_struct.hpp>
#include <vulkan/vulkan.h>
#include <optional>

struct LastBound;

namespace vvl {
class CommandBuffer;
class RenderPass;
}  // namespace vvl

namespace syncval {

class CommandBufferContext;

enum class AttachmentType { kColor, kDepth, kStencil };

struct DynamicRenderingInfo {
    struct Attachment {
        const vku::safe_VkRenderingAttachmentInfo& info;
        std::shared_ptr<const vvl::ImageView> view;
        std::shared_ptr<const vvl::ImageView> resolve_view;
        ImageRangeGen view_gen;
        std::optional<ImageRangeGen> resolve_gen;
        AttachmentType type;

        Attachment(const SyncValidator& state, const vku::safe_VkRenderingAttachmentInfo& info, const AttachmentType type_,
                   const VkOffset3D& offset, const VkExtent3D& extent);

        ImageRangeGen GetRangeGen(uint32_t view_mask = 0) const;

        SyncAccessIndex GetLoadUsage() const;
        SyncAccessIndex GetStoreUsage() const;
        SyncOrdering GetOrdering() const;
        Location GetLocation(const Location& loc, uint32_t index = 0) const;
        bool IsWriteable(const LastBound& last_bound_state) const;
        bool IsValid() const { return view.get(); }
    };

    // attachments store references to this info, so make sure this doesn't get moved around.
    DynamicRenderingInfo(const DynamicRenderingInfo&) = delete;
    DynamicRenderingInfo(DynamicRenderingInfo&&) = delete;
    DynamicRenderingInfo& operator=(const DynamicRenderingInfo&) = delete;
    DynamicRenderingInfo& operator=(DynamicRenderingInfo&&) = delete;

    DynamicRenderingInfo(const SyncValidator& state, const VkRenderingInfo& rendering_info);

    const vvl::ImageView* GetClearAttachmentView(const VkClearAttachment& clear_attachment) const;

    vku::safe_VkRenderingInfo info;
    std::vector<Attachment> attachments;  // All attachments (with internal typing)
};

struct BeginRenderingCmdState {
    BeginRenderingCmdState(std::shared_ptr<const vvl::CommandBuffer>&& cb_state_) : cb_state(std::move(cb_state_)) {}
    void AddRenderingInfo(const SyncValidator& state, const VkRenderingInfo& rendering_info);
    const DynamicRenderingInfo& GetRenderingInfo() const;
    std::shared_ptr<const vvl::CommandBuffer> cb_state;
    std::unique_ptr<DynamicRenderingInfo> info;
};

std::unique_ptr<AccessContext[]> InitSubpassContexts(VkQueueFlags queue_flags, const vvl::RenderPass& rp_state,
                                                     const AccessContext& external_context);

using AttachmentViewGenVector = std::vector<AttachmentViewGen>;

class RenderPassAccessContext {
  public:
    static AttachmentViewGenVector CreateAttachmentViewGen(const VkRect2D& render_area,
                                                           vvl::span<const std::shared_ptr<const vvl::ImageView>> attachment_views);
    RenderPassAccessContext(const vvl::RenderPass& rp_state, const VkRect2D& render_area, VkQueueFlags queue_flags,
                            vvl::span<const std::shared_ptr<const vvl::ImageView>> attachment_views,
                            const AccessContext& external_context, uint32_t render_pass_instance_id);

    static bool ValidateLayoutTransitions(const SyncEnvironment& env, const AccessContext& access_context,
                                          const vvl::RenderPass& rp_state, uint32_t render_pass_instance_id, uint32_t subpass,
                                          uint32_t view_mask, const AttachmentViewGenVector& attachment_views,
                                          const CommandBufferContext& cb_context, ResourceUsageTag replay_tag, const Location& loc);

    static bool ValidateLoadOperation(const SyncEnvironment& env, const AccessContext& access_context,
                                      const vvl::RenderPass& rp_state, uint32_t render_pass_instance_id, uint32_t subpass,
                                      uint32_t view_mask, const AttachmentViewGenVector& attachment_views,
                                      const CommandBufferContext& cb_context, ResourceUsageTag replay_tag, const Location& loc);

    bool ValidateStoreOperation(const SyncEnvironment& env, const CommandBufferContext& cb_context, ResourceUsageTag replay_tag,
                                const Location& loc) const;
    bool ValidateResolveOperations(const SyncEnvironment& env, const CommandBufferContext& cb_context, ResourceUsageTag replay_tag,
                                   const Location& loc) const;

    static void UpdateAttachmentResolveAccess(const vvl::RenderPass& rp_state, const AttachmentViewGenVector& attachment_views,
                                              uint32_t render_pass_instance_id, uint32_t subpass, uint32_t view_mask,
                                              const ResourceUsageTag tag, AccessContext& access_context, QueueId queue_id);

    static void UpdateAttachmentStoreAccess(const vvl::RenderPass& rp_state, const AttachmentViewGenVector& attachment_views,
                                            uint32_t render_pass_instance_id, uint32_t subpass, uint32_t view_mask,
                                            const ResourceUsageTag tag, AccessContext& access_context, QueueId queue_id);

    static void RecordLayoutTransitions(const vvl::RenderPass& rp_state, uint32_t subpass,
                                        const AttachmentViewGenVector& attachment_views, const ResourceUsageTag tag,
                                        AccessContext& access_context, QueueId queue_id);

    bool ValidateDrawSubpassAttachment(const CommandBufferContext& cb_context, vvl::Func command) const;
    void RecordDrawSubpassAttachment(const vvl::CommandBuffer& cmd_buffer, ResourceUsageTag tag);

    const vvl::ImageView* GetClearAttachmentView(const VkClearAttachment& clear_attachment) const;

    bool ValidateNextSubpass(const SyncEnvironment& env, const CommandBufferContext& cb_context, ResourceUsageTag replay_tag,
                             const Location& loc) const;
    bool ValidateEndRenderPass(const SyncEnvironment& env, const CommandBufferContext& cb_context, ResourceUsageTag replay_tag,
                               const Location& loc) const;
    bool ValidateFinalSubpassLayoutTransitions(const SyncEnvironment& env, const CommandBufferContext& cb_context,
                                               ResourceUsageTag replay_tag, const Location& loc) const;

    void RecordLayoutTransitions(ResourceUsageTag tag, QueueId queue_id);
    void RecordLoadOperations(ResourceUsageTag tag, QueueId queue_id);
    void RecordBeginRenderPass(ResourceUsageTag transition_tag, ResourceUsageTag load_op_tag, QueueId queue_id);
    bool AdvanceSubpass();
    void RecordNextSubpass(ResourceUsageTag resolve_tag, ResourceUsageTag store_tag, ResourceUsageTag transition_tag,
                           ResourceUsageTag load_tag, QueueId queue_id);
    void RecordEndRenderPass(AccessContext& external_context, ResourceUsageTag store_tag, ResourceUsageTag transition_tag,
                             QueueId queue_id);

    uint32_t GetCurrentSubpass() const { return current_subpass_; }
    AccessContext& CurrentContext();
    const AccessContext& CurrentContext() const;
    vvl::span<const AccessContext> GetSubpassContexts() const;
    vvl::span<AccessContext> GetSubpassContexts();
    const vvl::RenderPass* GetRenderPassState() const { return rp_state_; }
    AccessContext* CreateStoreResolveProxy(QueueId queue_id) const;

  private:
    AttachmentAccess GetAttachmentAccess(SyncOrdering ordering, AttachmentAccessType type = AttachmentAccessType::Access) const;

  private:
    const vvl::RenderPass* rp_state_;
    const AttachmentViewGenVector attachment_views_;
    const AccessContext* external_context_;
    const std::unique_ptr<AccessContext[]> subpass_contexts_;
    const uint32_t render_pass_instance_id_;
    uint32_t current_subpass_;
};

}  // namespace syncval
