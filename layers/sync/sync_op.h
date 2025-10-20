/*
 * Copyright (c) 2019-2025 Valve Corporation
 * Copyright (c) 2019-2025 LunarG, Inc.
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

#include "sync/sync_access_context.h"
#include <vulkan/utility/vk_safe_struct.hpp>
#include "error_message/error_location.h"

struct DeviceExtensions;

namespace vvl {
class ImageView;
class RenderPass;
}  // namespace vvl

namespace syncval {

class CommandBufferAccessContext;
class CommandExecutionContext;
class RenderPassAccessContext;
class ReplayState;
class SyncValidator;

struct SyncEventState {
    enum IgnoreReason { NotIgnored = 0, ResetWaitRace, Reset2WaitRace, SetRace, MissingStageBits, SetVsWait2, MissingSetEvent };
    using EventPointer = std::shared_ptr<const vvl::Event>;
    EventPointer event;
    vvl::Func last_command;             // Only Event commands are valid here.
    ResourceUsageTag last_command_tag;  // Needed to filter replay validation
    vvl::Func unsynchronized_set;
    VkPipelineStageFlags2 barriers;
    SyncExecScope scope;
    ResourceUsageTag first_scope_tag;
    bool destroyed;
    std::shared_ptr<const AccessContext> first_scope;

    SyncEventState()
        : event(),
          last_command(vvl::Func::Empty),
          last_command_tag(0),
          unsynchronized_set(vvl::Func::Empty),
          barriers(0U),
          scope(),
          first_scope_tag(),
          destroyed(true) {}

    SyncEventState(const SyncEventState &) = default;
    SyncEventState(SyncEventState &&) = default;

    SyncEventState(const SyncEventState::EventPointer &event_state);

    void ResetFirstScope();
    const AccessContext::ScopeMap &FirstScope() const { return first_scope->GetAccessMap(); }
    IgnoreReason IsIgnoredByWait(vvl::Func command, VkPipelineStageFlags2 srcStageMask) const;
    bool HasBarrier(VkPipelineStageFlags2 stageMask, VkPipelineStageFlags2 exec_scope) const;
    void AddReferencedTags(ResourceUsageTagSet &referenced) const;
};

class SyncEventsContext {
  public:
    using Map = vvl::unordered_map<const vvl::Event *, std::shared_ptr<SyncEventState>>;
    using iterator = Map::iterator;
    using const_iterator = Map::const_iterator;

    SyncEventState *GetFromShared(const SyncEventState::EventPointer &event_state) {
        const auto find_it = map_.find(event_state.get());
        if (find_it == map_.end()) {
            if (!event_state.get()) return nullptr;

            const auto *event_plain_ptr = event_state.get();
            auto sync_state = std::make_shared<SyncEventState>(event_state);
            auto insert_pair = map_.emplace(event_plain_ptr, sync_state);
            return insert_pair.first->second.get();
        }
        return find_it->second.get();
    }

    const SyncEventState *Get(const vvl::Event *event_state) const {
        const auto find_it = map_.find(event_state);
        if (find_it == map_.end()) {
            return nullptr;
        }
        return find_it->second.get();
    }
    const SyncEventState *Get(const SyncEventState::EventPointer &event_state) const { return Get(event_state.get()); }

    void ApplyBarrier(const SyncExecScope &src, const SyncExecScope &dst, ResourceUsageTag tag);
    void ApplyTaggedWait(VkQueueFlags queue_flags, ResourceUsageTag tag);

    void Destroy(const vvl::Event *event_state) {
        auto sync_it = map_.find(event_state);
        if (sync_it != map_.end()) {
            sync_it->second->destroyed = true;
            map_.erase(sync_it);
        }
    }
    void Clear() { map_.clear(); }

    SyncEventsContext &DeepCopy(const SyncEventsContext &from);
    void AddReferencedTags(ResourceUsageTagSet &referenced) const;

  private:
    Map map_;
};

struct SyncBufferMemoryBarrier {
    std::shared_ptr<const vvl::Buffer> buffer;
    SyncBarrier barrier;
    AccessRange range;

	SyncBufferMemoryBarrier(const std::shared_ptr<const vvl::Buffer> &buffer, const SyncBarrier &barrier,
							const AccessRange &range)
		: buffer(buffer), barrier(barrier), range(range) {}
};

struct SyncImageMemoryBarrier {
    std::shared_ptr<const vvl::Image> image;
    SyncBarrier barrier;
    VkImageSubresourceRange subresource_range;
    bool layout_transition;
    uint32_t barrier_index;
    uint32_t handle_index = vvl::kNoIndex32;

    SyncImageMemoryBarrier(const std::shared_ptr<const vvl::Image> &image, const SyncBarrier &barrier,
                           const VkImageSubresourceRange &subresource_range, bool layout_transition, uint32_t barrier_index)
        : image(image),
          barrier(barrier),
          subresource_range(subresource_range),
          layout_transition(layout_transition),
          barrier_index(barrier_index) {}
};

struct BarrierSet {
    SyncExecScope src_exec_scope;
    SyncExecScope dst_exec_scope;
    std::vector<SyncBarrier> memory_barriers;
    std::vector<SyncBufferMemoryBarrier> buffer_memory_barriers;
    std::vector<SyncImageMemoryBarrier> image_memory_barriers;
    bool single_exec_scope;
    void MakeMemoryBarriers(const SyncExecScope &src, const SyncExecScope &dst, uint32_t memoryBarrierCount,
                            const VkMemoryBarrier *pMemoryBarriers);
    void MakeBufferMemoryBarriers(const SyncValidator &sync_state, const SyncExecScope &src, const SyncExecScope &dst,
                                  uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers);
    void MakeImageMemoryBarriers(const SyncValidator &sync_state, const SyncExecScope &src, const SyncExecScope &dst,
                                 uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers,
                                 const DeviceExtensions &extensions);
    void MakeMemoryBarriers(VkQueueFlags queue_flags, const VkDependencyInfo &dep_info);
    void MakeBufferMemoryBarriers(const SyncValidator &sync_state, VkQueueFlags queue_flags, uint32_t barrier_count,
                                  const VkBufferMemoryBarrier2 *barriers);
    void MakeImageMemoryBarriers(const SyncValidator &sync_state, VkQueueFlags queue_flags, uint32_t barrier_count,
                                 const VkImageMemoryBarrier2 *barriers, const DeviceExtensions &extensions);
};

class SyncOpBase {
  public:
    SyncOpBase() : command_(vvl::Func::Empty) {}
    SyncOpBase(vvl::Func command) : command_(command) {}
    virtual ~SyncOpBase() = default;

    const char *CmdName() const { return vvl::String(command_); }

    virtual bool Validate(const CommandBufferAccessContext &cb_context) const = 0;
    virtual ResourceUsageTag Record(CommandBufferAccessContext *cb_context) = 0;
    virtual bool ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const = 0;
    virtual void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const = 0;

  protected:
    vvl::Func command_;
};

class SyncOpPipelineBarrier : public SyncOpBase {
  public:
    SyncOpPipelineBarrier(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags,
                          VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount,
                          const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                          const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                          const VkImageMemoryBarrier *pImageMemoryBarriers);
    SyncOpPipelineBarrier(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags,
                          const VkDependencyInfo &pDependencyInfo);
    ~SyncOpPipelineBarrier() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) override;
    bool ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const override;

  private:
    // Single barrier can be applied more efficently since there is no need to support independent
    // barrier application and collect pending state.
    void ApplySingleBarrier(CommandExecutionContext &exec_context) const;

    void ApplyMultipleBarriers(CommandExecutionContext &exec_context, const ResourceUsageTag exec_tag) const;

  private:
    BarrierSet barrier_set_;
};

class SyncOpWaitEvents : public SyncOpBase {
  public:
    SyncOpWaitEvents(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags, uint32_t eventCount,
                     const VkEvent *pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                     uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                     const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                     const VkImageMemoryBarrier *pImageMemoryBarriers);

    SyncOpWaitEvents(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags, uint32_t eventCount,
                     const VkEvent *pEvents, const VkDependencyInfo *pDependencyInfo);
    ~SyncOpWaitEvents() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) override;
    bool ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const override;

  private:
    static const char *const kIgnored;
    bool DoValidate(const CommandExecutionContext &ex_context, const ResourceUsageTag base_tag) const;
    void DoRecord(CommandExecutionContext &ex_context, const ResourceUsageTag base_tag) const;
    void MakeEventsList(const SyncValidator &sync_state, uint32_t event_count, const VkEvent *events);

    // TODO PHASE2 This is the wrong thing to use for "replay".. as the event state will have moved on since the record
    // TODO PHASE2 May need to capture by value w.r.t. "first use" or build up in calling/enqueue context through replay.
    std::vector<std::shared_ptr<const vvl::Event>> events_;

    std::vector<BarrierSet> barrier_sets_;
};

class SyncOpResetEvent : public SyncOpBase {
  public:
    SyncOpResetEvent(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkEvent event,
                     VkPipelineStageFlags2 stageMask);
    ~SyncOpResetEvent() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) override;
    bool ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const override;

  private:
    bool DoValidate(const CommandExecutionContext &ex_context, const ResourceUsageTag base_tag) const;
    std::shared_ptr<const vvl::Event> event_;
    SyncExecScope exec_scope_;
};

class SyncOpSetEvent : public SyncOpBase {
  public:
    SyncOpSetEvent(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkEvent event,
                   VkPipelineStageFlags2 stageMask, const AccessContext *access_context);
    SyncOpSetEvent(vvl::Func command, const SyncValidator &sync_state, VkQueueFlags queue_flags, VkEvent event,
                   const VkDependencyInfo &dep_info, const AccessContext *access_context);
    ~SyncOpSetEvent() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) override;
    bool ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const override;

  private:
    bool DoValidate(const CommandExecutionContext &ex_context, const ResourceUsageTag base_tag) const;
    void DoRecord(QueueId queue_id, ResourceUsageTag recorded_tag, const std::shared_ptr<const AccessContext> &access_context,
                  SyncEventsContext *events_context) const;
    std::shared_ptr<const vvl::Event> event_;
    // The Access context of the command buffer at record set event time.
    std::shared_ptr<const AccessContext> recorded_context_;
    SyncExecScope src_exec_scope_;
    // Note that the dep info is *not* dehandled, but retained for comparison with a future WaitEvents2
    std::shared_ptr<vku::safe_VkDependencyInfo> dep_info_;
};

class SyncOpBeginRenderPass : public SyncOpBase {
  public:
    SyncOpBeginRenderPass(vvl::Func command, const SyncValidator &sync_state, const VkRenderPassBeginInfo *pRenderPassBegin,
                          const VkSubpassBeginInfo *pSubpassBeginInfo);
    ~SyncOpBeginRenderPass() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) override;
    bool ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const override;
    const RenderPassAccessContext *GetRenderPassAccessContext() const { return rp_context_; }

  protected:
    vku::safe_VkRenderPassBeginInfo renderpass_begin_info_;
    vku::safe_VkSubpassBeginInfo subpass_begin_info_;
    std::vector<std::shared_ptr<const vvl::ImageView>> shared_attachments_;
    std::vector<const vvl::ImageView *> attachments_;
    std::shared_ptr<const vvl::RenderPass> rp_state_;
    const RenderPassAccessContext *rp_context_;
};

class SyncOpNextSubpass : public SyncOpBase {
  public:
    SyncOpNextSubpass(vvl::Func command, const SyncValidator &sync_state, const VkSubpassBeginInfo *pSubpassBeginInfo,
                      const VkSubpassEndInfo *pSubpassEndInfo);
    ~SyncOpNextSubpass() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) override;
    bool ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const override;

  protected:
    vku::safe_VkSubpassBeginInfo subpass_begin_info_;
    vku::safe_VkSubpassEndInfo subpass_end_info_;
};

class SyncOpEndRenderPass : public SyncOpBase {
  public:
    SyncOpEndRenderPass(vvl::Func command, const SyncValidator &sync_state, const VkSubpassEndInfo *pSubpassEndInfo);
    ~SyncOpEndRenderPass() override = default;

    bool Validate(const CommandBufferAccessContext &cb_context) const override;
    ResourceUsageTag Record(CommandBufferAccessContext *cb_context) override;
    bool ReplayValidate(ReplayState &replay, ResourceUsageTag recorded_tag) const override;
    void ReplayRecord(CommandExecutionContext &exec_context, ResourceUsageTag exec_tag) const override;

  protected:
    vku::safe_VkSubpassEndInfo subpass_end_info_;
};

// Batch barrier ops don't modify in place, and thus don't need to hold pending state, and also are *never* layout transitions.
struct BatchBarrierOp {
    SyncBarrier barrier;
    BarrierScope barrier_scope;

    BatchBarrierOp(QueueId queue_id, const SyncBarrier &barrier) : barrier(barrier), barrier_scope(barrier, queue_id) {}

    void operator()(AccessState *access_state) const { access_state->ApplyBarrier(barrier_scope, barrier); }
};

// Allow keep track of the exec contexts replay state
class ReplayState {
  public:
    // A minimal subset of the functionality present in the RenderPassAccessContext. Since the accesses are recorded in the
    // first_use information of the recorded access contexts, s.t. all we need to support is the barrier/resolve operations
    struct RenderPassReplayState {
        AccessContext *Begin(VkQueueFlags queue_flags, const SyncOpBeginRenderPass &begin_op_,
                             const AccessContext &external_context);
        AccessContext *Next();
        void End(AccessContext &external_context);
        vvl::span<AccessContext> GetSubpassContexts();

        const SyncOpBeginRenderPass *begin_op = nullptr;
        const AccessContext *replay_context = nullptr;
        uint32_t subpass = VK_SUBPASS_EXTERNAL;
        std::unique_ptr<AccessContext[]> subpass_contexts;
    };

    bool ValidateFirstUse();
    bool DetectFirstUseHazard(const ResourceUsageRange &first_use_range) const;

    ReplayState(CommandExecutionContext &exec_context, const CommandBufferAccessContext &recorded_context,
                const ErrorObject &error_object, uint32_t index, ResourceUsageTag base_tag);

    CommandExecutionContext &GetExecutionContext() const { return exec_context_; }
    ResourceUsageTag GetBaseTag() const { return base_tag_; }

    AccessContext *ReplayStateRenderPassBegin(VkQueueFlags queue_flags, const SyncOpBeginRenderPass &begin_op,
                                              const AccessContext &external_context);
    AccessContext *ReplayStateRenderPassNext();
    void ReplayStateRenderPassEnd(AccessContext &external_context);

  protected:
    const AccessContext *GetRecordedAccessContext() const;

    CommandExecutionContext &exec_context_;
    const CommandBufferAccessContext &recorded_context_;
    const ErrorObject &error_obj_;
    const uint32_t index_;
    const ResourceUsageTag base_tag_;
    RenderPassReplayState rp_replay_;
};

}  // namespace syncval
