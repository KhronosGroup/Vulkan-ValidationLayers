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

namespace vvl {
class Pipeline;
class RenderPass;
}  // namespace vvl

namespace syncval {

class CommandBufferContext;

std::unique_ptr<AccessContext[]> InitSubpassContexts(VkQueueFlags queue_flags, const vvl::RenderPass& rp_state,
                                                     const AccessContext& external_context, QueueId queue_id);

using AttachmentViewGenVector = std::vector<AttachmentViewGen>;

class RenderPassAccessContext {
  public:
    static AttachmentViewGenVector CreateAttachmentViewGen(const VkRect2D& render_area,
                                                           vvl::span<const std::shared_ptr<const vvl::ImageView>> attachment_views);
    RenderPassAccessContext(const vvl::RenderPass& rp_state, const VkRect2D& render_area, VkQueueFlags queue_flags,
                            vvl::span<const std::shared_ptr<const vvl::ImageView>> attachment_views,
                            const AccessContext& external_context, uint32_t render_pass_instance_id, QueueId queue_id);

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
                                        AccessContext& access_context);

    bool ValidateDrawSubpassAttachment(const SyncEnvironment& env, const CommandBufferContext& cb_context,
                                       ResourceUsageTag replay_tag, const Location& loc, const vvl::Pipeline* pipeline,
                                       bool depth_write_enabled, bool stencil_write_enabled) const;
    void RecordDrawSubpassAttachment(const vvl::Pipeline* pipeline, bool depth_write_enabled, bool stencil_write_enabled,
                                     ResourceUsageTag tag, QueueId queue_id);

    const vvl::ImageView* GetClearAttachmentView(const VkClearAttachment& clear_attachment) const;

    bool ValidateNextSubpass(const SyncEnvironment& env, const CommandBufferContext& cb_context, ResourceUsageTag replay_tag,
                             const Location& loc) const;
    bool ValidateEndRenderPass(const SyncEnvironment& env, const CommandBufferContext& cb_context, ResourceUsageTag replay_tag,
                               const Location& loc) const;
    bool ValidateFinalSubpassLayoutTransitions(const SyncEnvironment& env, const CommandBufferContext& cb_context,
                                               ResourceUsageTag replay_tag, const Location& loc) const;

    void RecordLayoutTransitions(ResourceUsageTag tag);
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
