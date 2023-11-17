/*
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
#pragma once

#include "sync/sync_validation.h"

class CommandBufferAccessContext : public CommandExecutionContext {
  public:
    using SyncOpPointer = std::shared_ptr<SyncOpBase>;
    constexpr static SyncStageAccessIndex kResolveRead = SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_READ;
    constexpr static SyncStageAccessIndex kResolveWrite = SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE;
    constexpr static SyncOrdering kResolveOrder = SyncOrdering::kColorAttachment;
    constexpr static SyncOrdering kStoreOrder = SyncOrdering::kRaster;

    struct SyncOpEntry {
        ResourceUsageTag tag;
        SyncOpPointer sync_op;
        SyncOpEntry(ResourceUsageTag tag_, SyncOpPointer &&sync_op_) : tag(tag_), sync_op(std::move(sync_op_)) {}
        SyncOpEntry() = default;
        SyncOpEntry(const SyncOpEntry &other) = default;
    };

    CommandBufferAccessContext(const SyncValidator *sync_validator = nullptr);
    CommandBufferAccessContext(SyncValidator &sync_validator, CMD_BUFFER_STATE *cb_state)
        : CommandBufferAccessContext(&sync_validator) {
        cb_state_ = cb_state;
    }

    struct AsProxyContext {};
    CommandBufferAccessContext(const CommandBufferAccessContext &real_context, AsProxyContext dummy);

    // NOTE: because this class is encapsulated in syncval_state::CommandBuffer, it isn't safe
    // to use shared_from_this from the constructor.
    void SetSelfReference() { cbs_referenced_->insert(cb_state_->shared_from_this()); }

    ~CommandBufferAccessContext() override = default;
    const CommandExecutionContext &GetExecutionContext() const { return *this; }

    void Destroy() {
        // the cb self reference must be cleared or the command buffer reference count will never go to 0
        cbs_referenced_.reset();
        cb_state_ = nullptr;
    }

    void Reset();

    std::string FormatUsage(ResourceUsageTag tag) const override;
    std::string FormatUsage(const ResourceFirstAccess &access) const;  //  Only command buffers have "first usage"
    AccessContext *GetCurrentAccessContext() override { return current_context_; }
    SyncEventsContext *GetCurrentEventsContext() override { return &events_context_; }
    const AccessContext *GetCurrentAccessContext() const override { return current_context_; }
    const SyncEventsContext *GetCurrentEventsContext() const override { return &events_context_; }
    QueueId GetQueueId() const override;

    RenderPassAccessContext *GetCurrentRenderPassContext() { return current_renderpass_context_; }
    const RenderPassAccessContext *GetCurrentRenderPassContext() const { return current_renderpass_context_; }
    ResourceUsageTag RecordBeginRenderPass(vvl::Func command, const RENDER_PASS_STATE &rp_state, const VkRect2D &render_area,
                                           const std::vector<const syncval_state::ImageViewState *> &attachment_views);

    bool ValidateBeginRendering(const ErrorObject &error_obj, syncval_state::BeginRenderingCmdState &cmd_state) const;
    void RecordBeginRendering(syncval_state::BeginRenderingCmdState &cmd_state, const RecordObject &record_obj);
    bool ValidateEndRendering(const ErrorObject &error_obj) const;
    void RecordEndRendering(const RecordObject &record_obj);
    bool ValidateDispatchDrawDescriptorSet(VkPipelineBindPoint pipelineBindPoint, const Location &loc) const;
    void RecordDispatchDrawDescriptorSet(VkPipelineBindPoint pipelineBindPoint, ResourceUsageTag tag);
    bool ValidateDrawVertex(const std::optional<uint32_t> &vertexCount, uint32_t firstVertex, const Location &loc) const;
    void RecordDrawVertex(const std::optional<uint32_t> &vertexCount, uint32_t firstVertex, ResourceUsageTag tag);
    bool ValidateDrawVertexIndex(const std::optional<uint32_t> &indexCount, uint32_t firstIndex, const Location &loc) const;
    void RecordDrawVertexIndex(const std::optional<uint32_t> &indexCount, uint32_t firstIndex, ResourceUsageTag tag);
    bool ValidateDrawAttachment(const Location &loc) const;
    bool ValidateDrawDynamicRenderingAttachment(const Location &loc) const;
    void RecordDrawAttachment(ResourceUsageTag tag);
    void RecordDrawDynamicRenderingAttachment(ResourceUsageTag tag);
    ClearAttachmentInfo GetClearAttachmentInfo(const VkClearAttachment &clear_attachment, const VkClearRect &rect) const;
    bool ValidateClearAttachment(const Location &loc, const VkClearAttachment &clear_attachment, const VkClearRect &rect) const;
    void RecordClearAttachment(ResourceUsageTag tag, const VkClearAttachment &clear_attachment, const VkClearRect &rect);

    ResourceUsageTag RecordNextSubpass(vvl::Func command);
    ResourceUsageTag RecordEndRenderPass(vvl::Func command);
    void RecordDestroyEvent(EVENT_STATE *event_state);

    void RecordExecutedCommandBuffer(const CommandBufferAccessContext &recorded_context);
    void ResolveExecutedCommandBuffer(const AccessContext &recorded_context, ResourceUsageTag offset);

    VkQueueFlags GetQueueFlags() const { return cb_state_ ? cb_state_->GetQueueFlags() : 0; }

    ResourceUsageTag NextSubcommandTag(vvl::Func command, ResourceUsageRecord::SubcommandType subcommand);
    ResourceUsageTag NextSubcommandTag(vvl::Func command, NamedHandle &&handle, ResourceUsageRecord::SubcommandType subcommand);

    ResourceUsageTag GetTagLimit() const override { return access_log_->size(); }
    VulkanTypedHandle Handle() const override {
        if (cb_state_) {
            return cb_state_->Handle();
        }
        return VulkanTypedHandle(static_cast<VkCommandBuffer>(VK_NULL_HANDLE), kVulkanObjectTypeCommandBuffer);
    }

    ResourceUsageTag NextCommandTag(vvl::Func command, NamedHandle &&handle,
                                    ResourceUsageRecord::SubcommandType subcommand = ResourceUsageRecord::SubcommandType::kNone);

    ResourceUsageTag NextCommandTag(vvl::Func command,
                                    ResourceUsageRecord::SubcommandType subcommand = ResourceUsageRecord::SubcommandType::kNone);
    ResourceUsageTag NextIndexedCommandTag(vvl::Func command, uint32_t index);

    // NamedHandle must be constructable from args
    template <class... Args>
    void AddHandle(ResourceUsageTag tag, Args &&...args) {
        assert(tag < access_log_->size());
        if (tag < access_log_->size()) {
            (*access_log_)[tag].AddHandle(std::forward<Args>(args)...);
        }
    }

    std::shared_ptr<const CMD_BUFFER_STATE> GetCBStateShared() const { return cb_state_->shared_from_this(); }

    const CMD_BUFFER_STATE &GetCBState() const {
        assert(cb_state_);
        return *cb_state_;
    }

    template <class T, class... Args>
    void RecordSyncOp(Args &&...args) {
        // T must be as derived from SyncOpBase or the compiler will flag the next line as an error.
        SyncOpPointer sync_op(std::make_shared<T>(std::forward<Args>(args)...));
        RecordSyncOp(std::move(sync_op));  // Call the non-template version
    }
    std::shared_ptr<AccessLog> GetAccessLogShared() const { return access_log_; }
    std::shared_ptr<CommandBufferSet> GetCBReferencesShared() const { return cbs_referenced_; }
    void InsertRecordedAccessLogEntries(const CommandBufferAccessContext &cb_context) override;
    const std::vector<SyncOpEntry> &GetSyncOps() const { return sync_ops_; };

  private:
    // As this is passing around a shared pointer to record, move to avoid needless atomics.
    void RecordSyncOp(SyncOpPointer &&sync_op);

    bool ValidateClearAttachment(const Location &loc, const ClearAttachmentInfo &info) const;
    void RecordClearAttachment(ResourceUsageTag tag, const ClearAttachmentInfo &clear_info);

    // Note: since every CommandBufferAccessContext is encapsulated in its CommandBuffer object,
    // a reference count is not needed here.
    CMD_BUFFER_STATE *cb_state_;

    std::shared_ptr<AccessLog> access_log_;
    std::shared_ptr<CommandBufferSet> cbs_referenced_;
    uint32_t command_number_;
    uint32_t subcommand_number_;
    uint32_t reset_count_;
    small_vector<NamedHandle, 1> command_handles_;

    AccessContext cb_access_context_;
    AccessContext *current_context_;
    SyncEventsContext events_context_;

    // Don't need the following for an active proxy cb context
    std::vector<std::unique_ptr<RenderPassAccessContext>> render_pass_contexts_;
    RenderPassAccessContext *current_renderpass_context_;
    std::vector<SyncOpEntry> sync_ops_;

    // State during dynamic rendering (dynamic rendering rendering passes must be
    // contained within a single command buffer)
    std::unique_ptr<syncval_state::DynamicRenderingInfo> dynamic_rendering_info_;
};

namespace syncval_state {
class CommandBuffer : public CMD_BUFFER_STATE {
  public:
    CommandBufferAccessContext access_context;

    CommandBuffer(SyncValidator *dev, VkCommandBuffer cb, const VkCommandBufferAllocateInfo *pCreateInfo,
                  const COMMAND_POOL_STATE *pool);
    ~CommandBuffer() { Destroy(); }

    void NotifyInvalidate(const BASE_NODE::NodeList &invalid_nodes, bool unlink) override;

    void Destroy() override;
    void Reset() override;
};
}  // namespace syncval_state
