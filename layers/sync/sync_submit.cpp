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

#include "sync/sync_submit.h"
#include "sync/sync_validation.h"
#include "sync/sync_image.h"

AcquiredImage::AcquiredImage(const PresentedImage& presented, ResourceUsageTag acq_tag)
    : image(presented.image), generator(presented.range_gen), present_tag(presented.tag), acquire_tag(acq_tag) {}

bool AcquiredImage::Invalid() const { return vvl::StateObject::Invalid(image); }

SignalInfo::SignalInfo(const QueueBatchContext::Ptr& batch, const SyncExecScope& exec_scope)
    : batch(batch), first_scope({batch->GetQueueId(), exec_scope}) {}

SignalInfo::SignalInfo(const PresentedImage& presented, ResourceUsageTag acquire_tag)
    : batch(presented.batch), first_scope(), acquired_image(std::make_shared<AcquiredImage>(presented, acquire_tag)) {}

void SignaledSemaphoresUpdate::OnSignal(const QueueBatchContext::Ptr& batch, const VkSemaphoreSubmitInfo& signal_info) {
    auto sem_state = sync_validator_.Get<vvl::Semaphore>(signal_info.semaphore);
    if (!sem_state) {
        return;
    }
    const VkSemaphore semaphore = sem_state->VkHandle();
    // Signal can't be registered in both lists at the same time.
    assert(!vvl::Contains(signals_to_add, semaphore) || !vvl::Contains(signals_to_remove, semaphore));

    const bool emplace_signal =
        // Add signal if it was previously in the remove list. It's a scenario when the semaphore
        // is unsignaled and then signaled by the same queue submit command.
        (signals_to_remove.erase(semaphore) == 1) ||
        // Or if the semaphore is not in the removal list, then add it only if it is not registered in
        // the global signaling list, because duplicated signal is an error (reported by core validation)
        // and the state should not be updated in this case.
        !vvl::Contains(sync_validator_.signaled_semaphores_, semaphore);

    if (emplace_signal) {
        const VkQueueFlags queue_flags = batch->GetQueueFlags();
        const SyncExecScope exec_scope = SyncExecScope::MakeSrc(queue_flags, signal_info.stageMask, VK_PIPELINE_STAGE_2_HOST_BIT);
        // If the semaphore is already in this list (duplicated binary signal error)
        // then emplace does not update the map, and this is the behavior we need.
        signals_to_add.emplace(semaphore, SignalInfo(batch, exec_scope));
    }
}

std::optional<SignalInfo> SignaledSemaphoresUpdate::OnUnsignal(VkSemaphore semaphore) {
    // Signal can't be registered in both lists at the same time.
    assert(!vvl::Contains(signals_to_add, semaphore) || !vvl::Contains(signals_to_remove, semaphore));
    std::optional<SignalInfo> unsignaled;

    if (auto add_it = signals_to_add.find(semaphore); add_it != signals_to_add.end()) {
        unsignaled.emplace(std::move(add_it->second));
        signals_to_add.erase(add_it);
    } else if (auto* p_global_info = vvl::Find(sync_validator_.signaled_semaphores_, semaphore)) {
        unsignaled.emplace(*p_global_info);
    }
    signals_to_remove.emplace(semaphore);

    // If unsignaled is null, there was a missing pending semaphore.
    // The caller returns early in this case. Error is reported by core validation.
    return unsignaled;
}

FenceSyncState::FenceSyncState() : fence(), tag(kInvalidTag), queue_id(kQueueIdInvalid) {}

FenceSyncState::FenceSyncState(const std::shared_ptr<const vvl::Fence>& fence_, QueueId queue_id_, ResourceUsageTag tag_)
    : fence(fence_), tag(tag_), queue_id(queue_id_) {}
FenceSyncState::FenceSyncState(const std::shared_ptr<const vvl::Fence>& fence_, const PresentedImage& image, ResourceUsageTag tag_)
    : fence(fence_), tag(tag_), queue_id(kQueueIdInvalid), acquired(image, tag) {}

syncval_state::Swapchain::Swapchain(ValidationStateTracker& dev_data, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                    VkSwapchainKHR handle)
    : vvl::Swapchain(dev_data, pCreateInfo, handle) {}

void syncval_state::Swapchain::RecordPresentedImage(PresentedImage&& presented_image) {
    // All presented images are stored within the swapchain until the are reaquired.
    const uint32_t image_index = presented_image.image_index;
    if (image_index >= presented.size()) presented.resize(image_index + 1);

    // Use move semantics to avoid atomic operations on the contained shared_ptrs
    presented[image_index] = std::move(presented_image);
}

// We move from the presented images array 1) so we don't copy shared_ptr, and 2) to mark it acquired
PresentedImage syncval_state::Swapchain::MovePresentedImage(uint32_t image_index) {
    if (presented.size() <= image_index) presented.resize(image_index + 1);
    PresentedImage ret_val = std::move(presented[image_index]);
    if (ret_val.Invalid()) {
        // If this is the first time the image has been acquired, then it's valid to have no present record, so we create one
        // Note: It's also possible this is an invalid acquire... but that's CoreChecks/Parameter validation's job to report
        ret_val = PresentedImage(static_cast<const syncval_state::Swapchain*>(this)->shared_from_this(), image_index);
    }
    return ret_val;
}

class ApplySemaphoreBarrierAction {
  public:
    ApplySemaphoreBarrierAction(const SemaphoreScope& signal, const SemaphoreScope& wait) : signal_(signal), wait_(wait) {}
    void operator()(ResourceAccessState* access) const { access->ApplySemaphore(signal_, wait_); }

  private:
    const SemaphoreScope& signal_;
    const SemaphoreScope wait_;
};

class ApplyAcquireNextSemaphoreAction {
  public:
    ApplyAcquireNextSemaphoreAction(const SyncExecScope& wait_scope, ResourceUsageTag acquire_tag)
        : barrier_(1, SyncBarrier(getPresentSrcScope(), getPresentValidAccesses(), wait_scope, SyncStageAccessFlags())),
          acq_tag_(acquire_tag) {}
    void operator()(ResourceAccessState* access) const {
        // Note that the present operations may or may not be present, given that the fence wait may have cleared them out.
        // Also, if a subsequent present has happened, we *don't* want to protect that...
        if (access->LastWriteTag() <= acq_tag_) {
            access->ApplyBarriersImmediate(barrier_);
        }
    }

  private:
    // kPresentSrcScope/kPresentValidAccesses cannot be regular global variables, because they use global
    // variables from another compilation unit (through syncStageAccessMaskByStageBit() call) for initialization,
    // and initialization of globals between compilation units is undefined. Instead they get initialized
    // on the first use (it's important to ensure this first use is also not initialization of some global!).
    const SyncExecScope& getPresentSrcScope() const {
        static const SyncExecScope kPresentSrcScope =
            SyncExecScope(VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL,  // mask_param (unused)
                          VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL,  // expanded_mask
                          VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL,  // exec_scope
                          getPresentValidAccesses());                      // valid_accesses
        return kPresentSrcScope;
    }
    const SyncStageAccessFlags& getPresentValidAccesses() const {
        static const SyncStageAccessFlags kPresentValidAccesses =
            SyncStageAccessFlags(SyncStageAccess::AccessScopeByStage(VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL));
        return kPresentValidAccesses;
    }

  private:
    std::vector<SyncBarrier> barrier_;
    ResourceUsageTag acq_tag_;
};

QueueBatchContext::QueueBatchContext(const SyncValidator& sync_state, const QueueSyncState& queue_state)
    : CommandExecutionContext(&sync_state),
      queue_state_(&queue_state),
      tag_range_(0, 0),
      current_access_context_(&access_context_),
      batch_log_(),
      queue_sync_tag_(sync_state.GetQueueIdLimit(), ResourceUsageTag(0)) {}

QueueBatchContext::QueueBatchContext(const SyncValidator& sync_state)
    : CommandExecutionContext(&sync_state),
      queue_state_(),
      tag_range_(0, 0),
      current_access_context_(&access_context_),
      batch_log_(),
      queue_sync_tag_(sync_state.GetQueueIdLimit(), ResourceUsageTag(0)) {}

void QueueBatchContext::Trim() {
    // Clean up unneeded access context contents and log information
    access_context_.TrimAndClearFirstAccess();

    ResourceUsageTagSet used_tags;
    access_context_.AddReferencedTags(used_tags);

    // Note: AccessContexts in the SyncEventsState are trimmed when created.
    events_context_.AddReferencedTags(used_tags);

    // Only conserve AccessLog references that are referenced by used_tags
    batch_log_.Trim(used_tags);
}

void QueueBatchContext::ResolveSubmittedCommandBuffer(const AccessContext& recorded_context, ResourceUsageTag offset) {
    GetCurrentAccessContext()->ResolveFromContext(QueueTagOffsetBarrierAction(GetQueueId(), offset), recorded_context);
}

VulkanTypedHandle QueueBatchContext::Handle() const { return queue_state_->Handle(); }

template <typename Predicate>
void QueueBatchContext::ApplyPredicatedWait(Predicate& predicate) {
    access_context_.EraseIf([&predicate](ResourceAccessRangeMap::value_type& access) {
        // Apply..Wait returns true if the waited access is empty...
        return access.second.ApplyPredicatedWait<Predicate>(predicate);
    });
}

void QueueBatchContext::ApplyTaggedWait(QueueId queue_id, ResourceUsageTag tag) {
    const bool any_queue = (queue_id == kQueueAny);

    if (any_queue) {
        // This isn't just avoid an unneeded test, but to allow *all* queues to to be waited in a single pass
        // (and it does avoid doing the same test for every access, as well as avoiding the need for the predicate
        // to grok Queue/Device/Wait differences.
        ResourceAccessState::WaitTagPredicate predicate{tag};
        ApplyPredicatedWait(predicate);
    } else {
        ResourceAccessState::WaitQueueTagPredicate predicate{queue_id, tag};
        ApplyPredicatedWait(predicate);
    }

    // SwapChain acquire QBC's have no queue, but also, events are always empty.
    if (queue_state_ && (queue_id == GetQueueId() || any_queue)) {
        events_context_.ApplyTaggedWait(GetQueueFlags(), tag);
    }
}

void QueueBatchContext::ApplyAcquireWait(const AcquiredImage& acquired) {
    ResourceAccessState::WaitAcquirePredicate predicate{acquired.present_tag, acquired.acquire_tag};
    ApplyPredicatedWait(predicate);
}

void QueueBatchContext::BeginRenderPassReplaySetup(ReplayState& replay, const SyncOpBeginRenderPass& begin_op) {
    current_access_context_ = replay.ReplayStateRenderPassBegin(GetQueueFlags(), begin_op, access_context_);
}

void QueueBatchContext::NextSubpassReplaySetup(ReplayState& replay) {
    current_access_context_ = replay.ReplayStateRenderPassNext();
}

void QueueBatchContext::EndRenderPassReplayCleanup(ReplayState& replay) {
    replay.ReplayStateRenderPassEnd(access_context_);
    current_access_context_ = &access_context_;
}

void QueueBatchContext::ResolvePresentSemaphoreWait(const SignalInfo& signal_info, const PresentedImages& presented_images) {
    assert(signal_info.batch);

    const AccessContext& from_context = signal_info.batch->access_context_;
    const SemaphoreScope& signal_scope = signal_info.first_scope;
    const QueueId queue_id = GetQueueId();
    const auto queue_flags = queue_state_->GetQueueFlags();
    SemaphoreScope wait_scope{queue_id, SyncExecScope::MakeDst(queue_flags, VK_PIPELINE_STAGE_2_PRESENT_ENGINE_BIT_SYNCVAL)};

    // If signal queue == wait queue, signal is treated as a memory barrier with an access scope equal to the present accesses
    SyncBarrier sem_barrier(signal_scope, wait_scope, SyncBarrier::AllAccess());
    const BatchBarrierOp sem_same_queue_op(wait_scope.queue, sem_barrier);

    // Need to import the rest of the same queue contents without modification
    SyncBarrier noop_barrier;
    const BatchBarrierOp noop_barrier_op(wait_scope.queue, noop_barrier);

    // Otherwise apply semaphore rules apply
    const ApplySemaphoreBarrierAction sem_not_same_queue_op(signal_scope, wait_scope);
    const SemaphoreScope noop_semaphore_scope(queue_id, noop_barrier.dst_exec_scope);
    const ApplySemaphoreBarrierAction noop_sem_op(signal_scope, noop_semaphore_scope);

    // For each presented image
    for (const auto& presented : presented_images) {
        // Need a copy that can be used as the pseudo-iterator...
        subresource_adapter::ImageRangeGenerator range_gen(presented.range_gen);
        if (signal_scope.queue == wait_scope.queue) {
            // If signal queue == wait queue, signal is treated as a memory barrier with an access scope equal to the
            // valid accesses for the sync scope.
            access_context_.ResolveFromContext(sem_same_queue_op, from_context, range_gen);
            access_context_.ResolveFromContext(noop_barrier_op, from_context);
        } else {
            access_context_.ResolveFromContext(sem_not_same_queue_op, from_context, range_gen);
            access_context_.ResolveFromContext(noop_sem_op, from_context);
        }
    }
}

void QueueBatchContext::ResolveSubmitSemaphoreWait(const SignalInfo& signal_info, VkPipelineStageFlags2 wait_mask) {
    assert(signal_info.batch);

    const SemaphoreScope& signal_scope = signal_info.first_scope;
    const auto queue_flags = queue_state_->GetQueueFlags();
    SemaphoreScope wait_scope{GetQueueId(), SyncExecScope::MakeDst(queue_flags, wait_mask)};

    const AccessContext& from_context = signal_info.batch->access_context_;
    if (signal_info.acquired_image) {
        // Import the *presenting* batch, but replacing presenting with acquired.
        ApplyAcquireNextSemaphoreAction apply_acq(wait_scope, signal_info.acquired_image->acquire_tag);
        access_context_.ResolveFromContext(apply_acq, from_context, signal_info.acquired_image->generator);

        // Grab the reset of the presenting QBC, with no effective barrier, won't overwrite the acquire, as the tag is newer
        SyncBarrier noop_barrier;
        const BatchBarrierOp noop_barrier_op(wait_scope.queue, noop_barrier);
        access_context_.ResolveFromContext(noop_barrier_op, from_context);
    } else {
        if (signal_scope.queue == wait_scope.queue) {
            // If signal queue == wait queue, signal is treated as a memory barrier with an access scope equal to the
            // valid accesses for the sync scope.
            SyncBarrier sem_barrier(signal_scope, wait_scope, SyncBarrier::AllAccess());
            const BatchBarrierOp sem_barrier_op(wait_scope.queue, sem_barrier);
            access_context_.ResolveFromContext(sem_barrier_op, from_context);
            events_context_.ApplyBarrier(sem_barrier.src_exec_scope, sem_barrier.dst_exec_scope, ResourceUsageRecord::kMaxIndex);
        } else {
            ApplySemaphoreBarrierAction sem_op(signal_scope, wait_scope);
            access_context_.ResolveFromContext(sem_op, signal_info.batch->access_context_);
        }
    }
}

void QueueBatchContext::ImportTags(const QueueBatchContext& from) {
    batch_log_.Import(from.batch_log_);

    // NOTE: Assumes that "from" has set its tag limit in its own queue_id slot
    size_t q_limit = queue_sync_tag_.size();
    assert(q_limit == from.queue_sync_tag_.size());
    for (size_t q = 0; q < q_limit; q++) {
        queue_sync_tag_[q] = std::max(queue_sync_tag_[q], from.queue_sync_tag_[q]);
    }
}

std::vector<QueueBatchContext::ConstPtr> QueueBatchContext::ResolvePresentDependencies(
    vvl::span<const VkSemaphore> wait_semaphores, const ConstPtr& last_batch, const PresentedImages& presented_images,
    SignaledSemaphoresUpdate& signaled_semaphores_update) {
    std::vector<ConstPtr> batches_resolved;
    for (VkSemaphore semaphore : wait_semaphores) {
        auto signal_info = signaled_semaphores_update.OnUnsignal(semaphore);
        if (!signal_info) {
            continue;  // Binary signal not found. This is handled be the core validation if enabled.
        }
        ResolvePresentSemaphoreWait(*signal_info, presented_images);
        ImportTags(*signal_info->batch);
        batches_resolved.emplace_back(std::move(signal_info->batch));
    }
    // Import the previous batch information
    if (last_batch) {
        // Copy in the event state from the previous batch (on this queue)
        events_context_.DeepCopy(last_batch->events_context_);
        if (!vvl::Contains(batches_resolved, last_batch)) {
            // If there are no semaphores to the previous batch, make sure a "submit order" non-barriered import is done
            access_context_.ResolveFromContext(last_batch->access_context_);
            ImportTags(*last_batch);
            batches_resolved.emplace_back(last_batch);
        }
    }
    return batches_resolved;
}

bool QueueBatchContext::DoQueuePresentValidate(const Location& loc, const PresentedImages& presented_images) {
    bool skip = false;

    // Tag the presented images so record doesn't have to know the tagging scheme
    for (size_t index = 0; index < presented_images.size(); ++index) {
        const PresentedImage& presented = presented_images[index];

        // Need a copy that can be used as the pseudo-iterator...
        HazardResult hazard =
            access_context_.DetectHazard(presented.range_gen, SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL);
        if (hazard.IsHazard()) {
            const auto queue_handle = queue_state_->Handle();
            const auto swap_handle = vvl::StateObject::Handle(presented.swapchain_state.lock());
            const auto image_handle = vvl::StateObject::Handle(presented.image);
            skip |= sync_state_->LogError(
                string_SyncHazardVUID(hazard.Hazard()), queue_handle, loc,
                "Hazard %s for present pSwapchains[%" PRIu32 "] , swapchain %s, image index %" PRIu32 " %s, Access info %s.",
                string_SyncHazard(hazard.Hazard()), presented.present_index, sync_state_->FormatHandle(swap_handle).c_str(),
                presented.image_index, sync_state_->FormatHandle(image_handle).c_str(), FormatHazard(hazard).c_str());
            if (skip) break;
        }
    }
    return skip;
}

void QueueBatchContext::DoPresentOperations(const PresentedImages& presented_images) {
    // For present, tagging is internal to the presented image record.
    for (const auto& presented : presented_images) {
        // Update memory state
        presented.UpdateMemoryAccess(SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL, presented.tag, access_context_);
    }
}

void QueueBatchContext::LogPresentOperations(const PresentedImages& presented_images, uint64_t submit_index) {
    if (tag_range_.size()) {
        auto access_log = std::make_shared<AccessLog>();
        BatchAccessLog::BatchRecord batch{queue_state_};
        batch.submit_index = submit_index;
        batch.base_tag = tag_range_.begin;
        batch_log_.Insert(batch, tag_range_, access_log);
        access_log->reserve(tag_range_.size());
        assert(tag_range_.size() == presented_images.size());
        for (const auto& presented : presented_images) {
            access_log->emplace_back(PresentResourceRecord(static_cast<const PresentedImageRecord>(presented)));
        }
    }
}

void QueueBatchContext::DoAcquireOperation(const PresentedImage& presented) {
    // Only one tag for acquire.  The tag in presented is the present tag
    presented.UpdateMemoryAccess(SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_ACQUIRE_READ_SYNCVAL, tag_range_.begin, access_context_);
}

void QueueBatchContext::LogAcquireOperation(const PresentedImage& presented, vvl::Func command) {
    auto access_log = std::make_shared<AccessLog>();
    BatchAccessLog::BatchRecord batch{queue_state_};
    batch.base_tag = tag_range_.begin;
    batch_log_.Insert(batch, tag_range_, access_log);
    access_log->emplace_back(AcquireResourceRecord(presented, tag_range_.begin, command));
}

void QueueBatchContext::SetupAccessContext(const PresentedImage& presented) {
    if (presented.batch) {
        access_context_.ResolveFromContext(presented.batch->access_context_);
        ImportTags(*presented.batch);
    }
}

std::vector<QueueBatchContext::ConstPtr> QueueBatchContext::RegisterAsyncContexts(const std::vector<ConstPtr>& batches_resolved) {
    // Gather async context information for hazard checks and conserve the QBC's for the async batches
    auto skip_resolved_filter = [&batches_resolved](auto& batch) { return !vvl::Contains(batches_resolved, batch); };
    std::vector<ConstPtr> async_batches = sync_state_->GetLastBatches(skip_resolved_filter);
    for (const auto& async_batch : async_batches) {
        const QueueId async_queue = async_batch->GetQueueId();
        ResourceUsageTag sync_tag;
        if (async_queue < queue_sync_tag_.size()) {
            sync_tag = queue_sync_tag_[async_queue];
        } else {
            // If this isn't from a tracked queue, just check the batch itself
            sync_tag = async_batch->tag_range_.begin;
        }

        // The start of the asynchronous access range for a given queue is one more than the highest tagged reference
        access_context_.AddAsyncContext(async_batch->GetCurrentAccessContext(), sync_tag, async_batch->GetQueueId());
        // We need to snapshot the async log information for async hazard reporting
        batch_log_.Import(async_batch->batch_log_);
    }
    return async_batches;
}

std::vector<QueueBatchContext::CommandBufferInfo> QueueBatchContext::GetCommandBuffers(const VkSubmitInfo2& submit_info) {
    std::vector<CommandBufferInfo> command_buffers;
    command_buffers.reserve(submit_info.commandBufferInfoCount);
    for (uint32_t i = 0; i < submit_info.commandBufferInfoCount; i++) {
        VkCommandBuffer cbuf = submit_info.pCommandBufferInfos[i].commandBuffer;
        if (auto cb_state = sync_state_->Get<syncval_state::CommandBuffer>(cbuf)) {
            command_buffers.emplace_back(i, std::move(cb_state));
        }
    }
    return command_buffers;
}

// Look up the usage informaiton from the local or global logger
std::string QueueBatchContext::FormatUsage(ResourceUsageTagEx tag_ex) const {
    std::stringstream out;
    BatchAccessLog::AccessRecord access = batch_log_.GetAccessRecord(tag_ex.tag);
    if (access.IsValid()) {
        const BatchAccessLog::BatchRecord& batch = *access.batch;
        const ResourceUsageRecord& record = *access.record;
        if (batch.queue) {
            // Queue and Batch information (for enqueued operations)
            out << SyncNodeFormatter(*sync_state_, batch.queue->GetQueueState());
            out << ", submit: " << batch.submit_index << ", batch: " << batch.batch_index;
        }
        out << ", batch_tag: " << batch.base_tag;

        // Commandbuffer Usages Information
        out << ", " << record.Formatter(*sync_state_, nullptr, access.debug_name_provider, tag_ex.handle_index);
    }
    return out.str();
}

VkQueueFlags QueueBatchContext::GetQueueFlags() const { return queue_state_->GetQueueFlags(); }

QueueId QueueBatchContext::GetQueueId() const {
    QueueId id = queue_state_ ? queue_state_->GetQueueId() : kQueueIdInvalid;
    return id;
}

ResourceUsageTag QueueBatchContext::SetupBatchTags(uint32_t tag_count) {
    tag_range_ = sync_state_->ReserveGlobalTagRange(tag_count);
    access_context_.SetStartTag(tag_range_.begin);

    // Needed for ImportSyncTags to pick up the "from" own sync tag.
    const QueueId this_q = GetQueueId();
    if (this_q < queue_sync_tag_.size()) {
        // If this is a non-queued operation we'll get a "special" value like invalid
        queue_sync_tag_[this_q] = tag_range_.end;
    }
    return tag_range_.begin;
}

std::vector<QueueBatchContext::ConstPtr> QueueBatchContext::ResolveSubmitDependencies(
    vvl::span<const VkSemaphoreSubmitInfo> wait_infos, const QueueBatchContext::ConstPtr& last_batch,
    SignaledSemaphoresUpdate& signaled_semaphores_update) {
    // Import (resolve) the batches that are waited on, with the semaphore's effective barriers applied
    std::vector<ConstPtr> batches_resolved;
    for (const auto& wait_info : wait_infos) {
        auto signal_info = signaled_semaphores_update.OnUnsignal(wait_info.semaphore);
        if (!signal_info) {
            continue;  // Binary signal not found. This is handled be the core validation if enabled.
        }
        ResolveSubmitSemaphoreWait(*signal_info, wait_info.stageMask);
        ImportTags(*signal_info->batch);
        batches_resolved.emplace_back(std::move(signal_info->batch));
    }
    // Import the previous batch information
    if (last_batch) {
        // Copy in the event state from the previous batch (on this queue)
        events_context_.DeepCopy(last_batch->events_context_);
        if (!vvl::Contains(batches_resolved, last_batch)) {
            // If there are no semaphores to the previous batch, make sure a "submit order" non-barriered import is done
            access_context_.ResolveFromContext(last_batch->access_context_);
            ImportTags(*last_batch);
            batches_resolved.emplace_back(last_batch);
        }
    }
    return batches_resolved;
}

bool QueueBatchContext::ValidateSubmit(const VkSubmitInfo2& submit, uint64_t submit_index, uint32_t batch_index,
                                       std::vector<std::string>& current_label_stack, const ErrorObject& error_obj) {
    bool skip = false;
    const std::vector<CommandBufferInfo> command_buffers = GetCommandBuffers(submit);

    BatchAccessLog::BatchRecord batch{queue_state_, submit_index, batch_index};
    uint32_t tag_count = 0;
    for (const auto& cb : command_buffers) {
        tag_count += static_cast<uint32_t>(cb.cb_state->access_context.GetTagCount());
    }
    if (tag_count) {
        batch.base_tag = SetupBatchTags(tag_count);
    }
    for (const auto& cb : command_buffers) {
        // Validate and resolve command buffers that has tagged commands
        const CommandBufferAccessContext& access_context = cb.cb_state->access_context;
        if (access_context.GetTagCount() > 0) {
            skip |= ReplayState(*this, access_context, error_obj, cb.index, batch.base_tag).ValidateFirstUse();
            // The barriers have already been applied in ValidatFirstUse
            batch_log_.Import(batch, access_context, current_label_stack);
            ResolveSubmittedCommandBuffer(*access_context.GetCurrentAccessContext(), batch.base_tag);
            batch.base_tag += access_context.GetTagCount();
        }
        // Apply debug label commands
        vvl::CommandBuffer::ReplayLabelCommands(cb.cb_state->GetLabelCommands(), current_label_stack);
        batch.cb_index++;
    }
    return skip;
}

QueueBatchContext::PresentResourceRecord::Base_::Record QueueBatchContext::PresentResourceRecord::MakeRecord() const {
    return std::make_unique<PresentResourceRecord>(presented_);
}
std::ostream& QueueBatchContext::PresentResourceRecord::Format(std::ostream& out, const SyncValidator& sync_state) const {
    out << "vkQueuePresentKHR ";
    out << "present_tag:" << presented_.tag;
    out << ", pSwapchains[" << presented_.present_index << "]";
    out << ": " << SyncNodeFormatter(sync_state, presented_.swapchain_state.lock().get());
    out << ", image_index: " << presented_.image_index;
    out << SyncNodeFormatter(sync_state, presented_.image.get());

    return out;
}

QueueBatchContext::AcquireResourceRecord::Base_::Record QueueBatchContext::AcquireResourceRecord::MakeRecord() const {
    return std::make_unique<AcquireResourceRecord>(presented_, acquire_tag_, command_);
}

std::ostream& QueueBatchContext::AcquireResourceRecord::Format(std::ostream& out, const SyncValidator& sync_state) const {
    out << vvl::String(command_) << " ";
    out << "aquire_tag:" << acquire_tag_;
    out << ": " << SyncNodeFormatter(sync_state, presented_.swapchain_state.lock().get());
    out << ", image_index: " << presented_.image_index;
    out << SyncNodeFormatter(sync_state, presented_.image.get());

    return out;
}

// Since we're updating the QueueSync state, this is Record phase and the access log needs to point to the global one
// Batch Contexts saved during signalling have their AccessLog reset when the pending signals are signalled.
// NOTE: By design, QueueBatchContexts that are neither last, nor referenced by a signal are abandoned as unowned, since
//       the contexts Resolve all history from previous all contexts when created
void QueueSyncState::UpdateLastBatch() {
    // Update the queue to point to the last batch from the submit
    if (pending_last_batch_) {
        // Clean up the events data in the previous last batch on queue, as only the subsequent batches have valid use for them
        // and the QueueBatchContext::Setup calls have be copying them along from batch to batch during submit.
        if (last_batch_) {
            last_batch_->ResetEventsContext();
        }
        pending_last_batch_->Trim();
        last_batch_ = std::move(pending_last_batch_);
    }
}

std::vector<QueueBatchContext::ConstPtr> SyncValidator::GetLastBatches(
    std::function<bool(const QueueBatchContext::ConstPtr&)> filter) const {
    std::vector<QueueBatchContext::ConstPtr> snapshot;
    for (const auto& queue_sync_state : queue_sync_states_) {
        auto batch = queue_sync_state->LastBatch();
        if (batch && filter(batch)) {
            snapshot.emplace_back(std::move(batch));
        }
    }
    return snapshot;
}

std::vector<QueueBatchContext::Ptr> SyncValidator::GetLastBatches(std::function<bool(const QueueBatchContext::ConstPtr&)> filter) {
    std::vector<QueueBatchContext::Ptr> snapshot;
    for (const auto& queue_sync_state : queue_sync_states_) {
        auto batch = queue_sync_state->LastBatch();
        if (batch && filter(batch)) {
            snapshot.emplace_back(std::move(batch));
        }
    }
    return snapshot;
}

// Note that function is const, but updates mutable submit_index to allow Validate to create correct tagging for command invocation
// scope state.
// Given that queue submits are supposed to be externally synchronized for the same queue, this should safe without being
// atomic... but as the ops are per submit, the performance cost is negible for the peace of mind.
uint64_t QueueSyncState::ReserveSubmitId() const { return submit_index_.fetch_add(1); }

void QueueSyncState::SetPendingLastBatch(QueueBatchContext::Ptr&& last) const { pending_last_batch_ = std::move(last); }

void BatchAccessLog::Import(const BatchRecord& batch, const CommandBufferAccessContext& cb_access,
                            const std::vector<std::string>& initial_label_stack) {
    ResourceUsageRange import_range = {batch.base_tag, batch.base_tag + cb_access.GetTagCount()};
    log_map_.insert(std::make_pair(import_range, CBSubmitLog(batch, cb_access, initial_label_stack)));
}

void BatchAccessLog::Import(const BatchAccessLog& other) {
    for (const auto& entry : other.log_map_) {
        log_map_.insert(entry);
    }
}

void BatchAccessLog::Insert(const BatchRecord& batch, const ResourceUsageRange& range,
                            std::shared_ptr<const CommandExecutionContext::AccessLog> log) {
    log_map_.insert(std::make_pair(range, CBSubmitLog(batch, nullptr, std::move(log))));
}

// Trim: Remove any unreferenced AccessLog ranges from a BatchAccessLog
//
// In order to contain memory growth in the AccessLog information regarding prior submitted command buffers,
// the Trim call removes any AccessLog references that do not correspond to any tags in use. The set of referenced tag, used_tags,
// is generated by scanning the AccessContext and EventContext of the containing QueueBatchContext.
//
// Upon return the BatchAccessLog should only contain references to the AccessLog information needed by the
// containing parent QueueBatchContext.
//
// The algorithm used is another example of the "parallel iteration" pattern common within SyncVal.  In this case we are
// traversing the ordered range_map containing the AccessLog references and the ordered set of tags in use.
//
// To efficiently perform the parallel iteration, optimizations within this function include:
//  * when ranges are detected that have no tags referenced, all ranges between the last tag and the current tag are erased
//  * when used tags prior to the current range are found, all tags up to the current range are skipped
//  * when a tag is found within the current range, that range is skipped (and thus kept in the map), and further used tags
//    within the range are skipped.
//
// Note that for each subcase, any "next steps" logic is designed to be handled within the subsequent iteration -- meaning that
// each subcase simply handles the specifics of the current update/skip/erase action needed, and leaves the iterators in a sensible
// state for the top of loop... intentionally eliding special case handling.
void BatchAccessLog::Trim(const ResourceUsageTagSet& used_tags) {
    auto current_tag = used_tags.cbegin();
    const auto end_tag = used_tags.cend();
    auto current_map_range = log_map_.begin();
    const auto end_map = log_map_.end();

    while (current_map_range != end_map) {
        if (current_tag == end_tag) {
            // We're out of tags, the rest of the map isn't referenced, so erase it
            current_map_range = log_map_.erase(current_map_range, end_map);
        } else {
            auto& range = current_map_range->first;
            const ResourceUsageTag tag = *current_tag;
            if (tag < range.begin) {
                // Skip to the next tag potentially in range
                // if this is end_tag, we'll handle that next iteration
                current_tag = used_tags.lower_bound(range.begin);
            } else if (tag >= range.end) {
                // This tag is beyond the current range, delete all ranges between current_map_range,
                // and the next that includes the tag.  Next is not erased.
                auto next_used = log_map_.lower_bound(ResourceUsageRange(tag, tag + 1));
                current_map_range = log_map_.erase(current_map_range, next_used);
            } else {
                // Skip the rest of the tags in this range
                // If this is end, the next iteration will handle
                current_tag = used_tags.lower_bound(range.end);

                // This is a range we will keep, advance to the next. Next iteration handles end condition
                ++current_map_range;
            }
        }
    }
}

BatchAccessLog::AccessRecord BatchAccessLog::GetAccessRecord(ResourceUsageTag tag) const {
    auto found_log = log_map_.find(tag);
    if (found_log != log_map_.cend()) {
        return found_log->second.GetAccessRecord(tag);
    }
    // tag not found
    assert(false);
    return AccessRecord();
}

std::string BatchAccessLog::CBSubmitLog::GetDebugRegionName(const ResourceUsageRecord& record) const {
    // const auto& label_commands = (*cbs_)[0]->GetLabelCommands();
    const auto& label_commands = label_commands_;  // TODO: use the above line when timelines are supported
    return vvl::CommandBuffer::GetDebugRegionName(label_commands, record.label_command_index, initial_label_stack_);
}

BatchAccessLog::AccessRecord BatchAccessLog::CBSubmitLog::GetAccessRecord(ResourceUsageTag tag) const {
    assert(tag >= batch_.base_tag);
    const size_t index = tag - batch_.base_tag;
    assert(log_);
    assert(index < log_->size());
    const ResourceUsageRecord* record = &(*log_)[index];
    const auto debug_name_provider = (record->label_command_index == vvl::kU32Max) ? nullptr : this;
    return AccessRecord{&batch_, record, debug_name_provider};
}

BatchAccessLog::CBSubmitLog::CBSubmitLog(const BatchRecord& batch,
                                         std::shared_ptr<const CommandExecutionContext::CommandBufferSet> cbs,
                                         std::shared_ptr<const CommandExecutionContext::AccessLog> log)
    : batch_(batch), cbs_(cbs), log_(log) {}

BatchAccessLog::CBSubmitLog::CBSubmitLog(const BatchRecord& batch, const CommandBufferAccessContext& cb,
                                         const std::vector<std::string>& initial_label_stack)
    : batch_(batch), cbs_(cb.GetCBReferencesShared()), log_(cb.GetAccessLogShared()), initial_label_stack_(initial_label_stack) {
    label_commands_ = (*cbs_)[0]->GetLabelCommands();  // TODO: when timelines are supported use cbs directly
}

PresentedImage::PresentedImage(const SyncValidator& sync_state, QueueBatchContext::Ptr batch_, VkSwapchainKHR swapchain,
                               uint32_t image_index_, uint32_t present_index_, ResourceUsageTag tag_)
    : PresentedImageRecord{tag_, image_index_, present_index_, sync_state.Get<syncval_state::Swapchain>(swapchain), {}},
      batch(std::move(batch_)) {
    SetImage(image_index_);
}

PresentedImage::PresentedImage(std::shared_ptr<const syncval_state::Swapchain> swapchain, uint32_t at_index) : PresentedImage() {
    swapchain_state = std::move(swapchain);
    tag = kInvalidTag;
    SetImage(at_index);
}

bool PresentedImage::Invalid() const { return vvl::StateObject::Invalid(image); }

// Export uses move semantics...
void PresentedImage::ExportToSwapchain(SyncValidator&) {  // Include this argument to prove the const cast is safe
    // If the swapchain is dead just ignore the present
    auto swap_lock = swapchain_state.lock();
    if (vvl::StateObject::Invalid(swap_lock)) return;
    auto swap = std::const_pointer_cast<syncval_state::Swapchain>(swap_lock);
    swap->RecordPresentedImage(std::move(*this));
}

void PresentedImage::SetImage(uint32_t at_index) {
    image_index = at_index;

    auto swap_lock = swapchain_state.lock();
    if (vvl::StateObject::Invalid(swap_lock)) return;
    image = std::static_pointer_cast<const syncval_state::ImageState>(swap_lock->GetSwapChainImageShared(image_index));
    if (Invalid()) {
        range_gen = ImageRangeGen();
    } else {
        // For valid images create the type/range_gen to used to scope the semaphore operations
        range_gen = image->MakeImageRangeGen(image->full_range, false);
    }
}

void PresentedImage::UpdateMemoryAccess(SyncStageAccessIndex usage, ResourceUsageTag tag, AccessContext& access_context) const {
    // Intentional copy. The range_gen argument is not copied by the Update... call below
    access_context.UpdateAccessState(range_gen, usage, SyncOrdering::kNonAttachment, tag);
}
