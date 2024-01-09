/*
 * Copyright (c) 2019-2024 Valve Corporation
 * Copyright (c) 2019-2024 LunarG, Inc.
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

#include "sync/sync_renderpass.h"

class SyncValidator;

class AlternateResourceUsage {
  public:
    struct RecordBase;
    struct RecordBase {
        using Record = std::unique_ptr<RecordBase>;
        virtual Record MakeRecord() const = 0;
        virtual std::ostream &Format(std::ostream &out, const SyncValidator &sync_state) const = 0;
        virtual ~RecordBase() {}
    };

    struct FormatterState {
        FormatterState(const SyncValidator &sync_state_, const AlternateResourceUsage &usage_)
            : sync_state(sync_state_), usage(usage_) {}
        const SyncValidator &sync_state;
        const AlternateResourceUsage &usage;
    };

    FormatterState Formatter(const SyncValidator &sync_state) const { return FormatterState(sync_state, *this); };

    std::ostream &Format(std::ostream &out, const SyncValidator &sync_state) const { return record_->Format(out, sync_state); };
    AlternateResourceUsage() = default;
    AlternateResourceUsage(const RecordBase &record) : record_(record.MakeRecord()) {}
    AlternateResourceUsage(const AlternateResourceUsage &other) : record_() {
        if (bool(other.record_)) {
            record_ = other.record_->MakeRecord();
        }
    }
    AlternateResourceUsage &operator=(const AlternateResourceUsage &other) {
        if (bool(other.record_)) {
            record_ = other.record_->MakeRecord();
        } else {
            record_.reset();
        }
        return *this;
    }

    operator bool() const { return bool(record_); }

  private:
    RecordBase::Record record_;
};

inline std::ostream &operator<<(std::ostream &out, const AlternateResourceUsage::FormatterState &formatter) {
    formatter.usage.Format(out, formatter.sync_state);
    return out;
}

template <typename State, typename T>
struct FormatterImpl {
    using That = T;
    friend T;
    const State &state;
    const That &that;

  private:
    // Only intended to be invoke with from That method
    FormatterImpl(const State &state_, const That &that_) : state(state_), that(that_) {}
};

struct NamedHandle {
    const static size_t kInvalidIndex = std::numeric_limits<size_t>::max();
    std::string name;
    VulkanTypedHandle handle;
    size_t index = kInvalidIndex;

    using FormatterState = FormatterImpl<SyncValidator, NamedHandle>;
    // NOTE: CRTP could DRY this
    FormatterState Formatter(const SyncValidator &sync_state) const { return FormatterState(sync_state, *this); }

    NamedHandle() = default;
    NamedHandle(const NamedHandle &other) = default;
    NamedHandle(NamedHandle &&other) = default;
    NamedHandle(const std::string &name_, const VulkanTypedHandle &handle_, size_t index_ = kInvalidIndex)
        : name(name_), handle(handle_), index(index_) {}
    NamedHandle(const char *name_, const VulkanTypedHandle &handle_, size_t index_ = kInvalidIndex)
        : name(name_), handle(handle_), index(index_) {}
    NamedHandle(const VulkanTypedHandle &handle_) : name(), handle(handle_) {}
    NamedHandle &operator=(const NamedHandle &other) = default;
    NamedHandle &operator=(NamedHandle &&other) = default;

    operator bool() const { return (handle.handle != 0U) && (handle.type != VulkanObjectType::kVulkanObjectTypeUnknown); }
    bool IsIndexed() const { return index != kInvalidIndex; }
};

using NamedHandleVector = small_vector<NamedHandle, 1, uint32_t>;

struct ResourceCmdUsageRecord {
    using TagIndex = ResourceUsageTag;
    using Count = uint32_t;
    constexpr static TagIndex kMaxIndex = std::numeric_limits<TagIndex>::max();

    enum class SubcommandType { kNone, kSubpassTransition, kLoadOp, kStoreOp, kResolveOp, kIndex };

    ResourceCmdUsageRecord() = default;
    ResourceCmdUsageRecord(vvl::Func command_, Count seq_num_, SubcommandType sub_type_, Count sub_command_,
                           const vvl::CommandBuffer *cb_state_, Count reset_count_)
        : command(command_),
          seq_num(seq_num_),
          sub_command_type(sub_type_),
          sub_command(sub_command_),
          cb_state(cb_state_),
          reset_count(reset_count_) {}

    // NamedHandle must be constructable from args
    template <class... Args>
    void AddHandle(Args &&...args) {
        handles.emplace_back(std::forward<Args>(args)...);
    }

    vvl::Func command = vvl::Func::Empty;
    Count seq_num = 0U;
    SubcommandType sub_command_type = SubcommandType::kNone;
    Count sub_command = 0U;

    // This is somewhat repetitive, but it prevents the need for Exec/Submit time touchup, after which usage records can be
    // from different command buffers and resets.
    // plain pointer as a shared pointer is held by the context storing this record
    const vvl::CommandBuffer *cb_state = nullptr;
    Count reset_count;
    NamedHandleVector handles;

    // Indexes CommandBufferAccessContext::DebugRegions::commands.
    // Allows to derive fully qualified name (i.e. including parent scopes)
    // of the debug region where the current command is located.
    uint32_t debug_region_command_index = vvl::kU32Max;
};

struct ResourceUsageRecord : public ResourceCmdUsageRecord {
    struct FormatterState {
        FormatterState(const SyncValidator &sync_state_, const ResourceUsageRecord &record_, const vvl::CommandBuffer *cb_state_)
            : sync_state(sync_state_), record(record_), ex_cb_state(cb_state_) {}
        const SyncValidator &sync_state;
        const ResourceUsageRecord &record;
        const vvl::CommandBuffer *ex_cb_state;
    };
    FormatterState Formatter(const SyncValidator &sync_state, const vvl::CommandBuffer *ex_cb_state) const {
        return FormatterState(sync_state, *this, ex_cb_state);
    }

    AlternateResourceUsage alt_usage;

    ResourceUsageRecord() = default;
    ResourceUsageRecord(vvl::Func command_, Count seq_num_, SubcommandType sub_type_, Count sub_command_,
                        const vvl::CommandBuffer *cb_state_, Count reset_count_)
        : ResourceCmdUsageRecord(command_, seq_num_, sub_type_, sub_command_, cb_state_, reset_count_) {}

    ResourceUsageRecord(const AlternateResourceUsage &other) : ResourceCmdUsageRecord(), alt_usage(other) {}
    ResourceUsageRecord(const ResourceUsageRecord &other) : ResourceCmdUsageRecord(other), alt_usage(other.alt_usage) {}
    ResourceUsageRecord &operator=(const ResourceUsageRecord &other) = default;
};

// Command execution context is the base class for command buffer and queue contexts
// Preventing unintented leakage of subclass specific state, storing enough information
// for message logging.
// TODO: determine where to draw the design split for tag tracking (is there anything command to Queues and CB's)
class CommandExecutionContext : public SyncValidationInfo {
  public:
    using AccessLog = std::vector<ResourceUsageRecord>;
    using CommandBufferSet = vvl::unordered_set<std::shared_ptr<const vvl::CommandBuffer>>;
    CommandExecutionContext() : SyncValidationInfo(nullptr) {}
    CommandExecutionContext(const SyncValidator *sync_validator) : SyncValidationInfo(sync_validator) {}
    virtual ~CommandExecutionContext() = default;

    // Are imported command buffers Submitted (QueueBatchContext), or Executed (CommandBufferAccessContext)
    enum ExecutionType : int {
        kExecuted = 0,  // Recorded contexts are integrated into context during vkCmdExecuteCommands
        kSubmitted = 1  // Recorded contexts are integrated into context during vkQueueSubmit (etc.)
    };

    virtual ExecutionType Type() const = 0;

    const char *ExecutionTypeString() {
        const char *type_string[] = {"Executed", "Submitted"};
        return type_string[Type()];
    }
    const char *ExecutionUsageString() {
        const char *usage_string[] = {"executed_usage", "submitted_usage"};
        return usage_string[Type()];
    }

    virtual AccessContext *GetCurrentAccessContext() = 0;
    virtual SyncEventsContext *GetCurrentEventsContext() = 0;
    virtual const AccessContext *GetCurrentAccessContext() const = 0;
    virtual const SyncEventsContext *GetCurrentEventsContext() const = 0;
    virtual QueueId GetQueueId() const = 0;

    ResourceUsageRange ImportRecordedAccessLog(const CommandBufferAccessContext &recorded_context);

    virtual ResourceUsageTag GetTagLimit() const = 0;
    virtual VulkanTypedHandle Handle() const = 0;
    virtual void InsertRecordedAccessLogEntries(const CommandBufferAccessContext &cb_context) = 0;

    virtual void BeginRenderPassReplaySetup(ReplayState &replay, const SyncOpBeginRenderPass &begin_op) {
        // Must override if use by derived type is valid
        assert(false);
    }

    virtual void NextSubpassReplaySetup(ReplayState &replay) {
        // Must override if use by derived type is valid
        assert(false);
    }

    virtual void EndRenderPassReplayCleanup(ReplayState &replay) {
        // Must override if use by derived type is valid
        assert(false);
    }

    bool ValidForSyncOps() const;
};

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
    CommandBufferAccessContext(SyncValidator &sync_validator, vvl::CommandBuffer *cb_state)
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
    std::string FormatUsage(const char *usage_string,
                            const ResourceFirstAccess &access) const;  //  Only command buffers have "first usage"
    AccessContext *GetCurrentAccessContext() override { return current_context_; }
    SyncEventsContext *GetCurrentEventsContext() override { return &events_context_; }
    const AccessContext *GetCurrentAccessContext() const override { return current_context_; }
    const SyncEventsContext *GetCurrentEventsContext() const override { return &events_context_; }
    QueueId GetQueueId() const override;

    RenderPassAccessContext *GetCurrentRenderPassContext() { return current_renderpass_context_; }
    const RenderPassAccessContext *GetCurrentRenderPassContext() const { return current_renderpass_context_; }
    ResourceUsageTag RecordBeginRenderPass(vvl::Func command, const vvl::RenderPass &rp_state, const VkRect2D &render_area,
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
    void RecordDestroyEvent(vvl::Event *event_state);

    void RecordExecutedCommandBuffer(const CommandBufferAccessContext &recorded_context);
    void ResolveExecutedCommandBuffer(const AccessContext &recorded_context, ResourceUsageTag offset);

    VkQueueFlags GetQueueFlags() const { return cb_state_ ? cb_state_->GetQueueFlags() : 0; }

    ResourceUsageTag NextSubcommandTag(vvl::Func command, ResourceUsageRecord::SubcommandType subcommand);
    ResourceUsageTag NextSubcommandTag(vvl::Func command, NamedHandle &&handle, ResourceUsageRecord::SubcommandType subcommand);

    ExecutionType Type() const override { return kExecuted; }
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

    std::shared_ptr<const vvl::CommandBuffer> GetCBStateShared() const { return cb_state_->shared_from_this(); }

    const vvl::CommandBuffer &GetCBState() const {
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

    void PushDebugRegion(const char *region_name);
    void PopDebugRegion();
    std::string GetDebugRegionFullyQualifiedName(uint32_t debug_region_command_index) const;

  private:
    // As this is passing around a shared pointer to record, move to avoid needless atomics.
    void RecordSyncOp(SyncOpPointer &&sync_op);

    bool ValidateClearAttachment(const Location &loc, const ClearAttachmentInfo &info) const;
    void RecordClearAttachment(ResourceUsageTag tag, const ClearAttachmentInfo &clear_info);

    void CheckCommandTagDebugCheckpoint();

    // Note: since every CommandBufferAccessContext is encapsulated in its CommandBuffer object,
    // a reference count is not needed here.
    vvl::CommandBuffer *cb_state_;

    std::shared_ptr<AccessLog> access_log_;
    std::shared_ptr<CommandBufferSet> cbs_referenced_;
    uint32_t command_number_;
    uint32_t subcommand_number_;
    uint32_t reset_count_;
    NamedHandleVector command_handles_;

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

    // Encodes information about debug regions defined for the current command buffer
    // using vkCmdBeginDebugUtilsLabelEXT/vkCmdEndDebugUtilsLabelEXT
    struct DebugRegions {
        struct DebugRegionCommand {
            // true when starting debug frame. label_name_index refers to label name.
            // false when ending debug frame. label_name_index is not used.
            bool start_region = false;
            uint32_t label_name_index = vvl::kU32Max;
        };
        std::vector<std::string> label_names;
        std::vector<DebugRegionCommand> commands;

        std::string GetDebugRegionFullyQualifiedName(uint32_t debug_region_command_index) const;
    };
    DebugRegions debug_regions_;
};

namespace syncval_state {
class CommandBuffer : public vvl::CommandBuffer {
  public:
    CommandBufferAccessContext access_context;

    CommandBuffer(SyncValidator *dev, VkCommandBuffer cb, const VkCommandBufferAllocateInfo *pCreateInfo,
                  const vvl::CommandPool *pool);
    ~CommandBuffer() { Destroy(); }

    void NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) override;

    void Destroy() override;
    void Reset() override;
};
}  // namespace syncval_state

// Message Creation Helpers
struct SyncNodeFormatter {
    const debug_report_data *report_data;
    const vvl::StateObject *node;
    const char *label;

    SyncNodeFormatter(const SyncValidator &sync_state, const vvl::CommandBuffer *cb_state);
    SyncNodeFormatter(const SyncValidator &sync_state, const vvl::Image *image);
    SyncNodeFormatter(const SyncValidator &sync_state, const vvl::Queue *q_state);
    SyncNodeFormatter(const SyncValidator &sync_state, const vvl::StateObject *state_object, const char *label_ = nullptr);
};

std::ostream &operator<<(std::ostream &out, const SyncNodeFormatter &formatter);
std::ostream &operator<<(std::ostream &out, const NamedHandle::FormatterState &formatter);
std::ostream &operator<<(std::ostream &out, const ResourceUsageRecord::FormatterState &formatter);
std::ostream &operator<<(std::ostream &out, const HazardResult::HazardState &hazard);
