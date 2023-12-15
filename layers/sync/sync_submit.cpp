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

#include "sync/sync_submit.h"
#include "sync/sync_validation.h"

AcquiredImage::AcquiredImage(const PresentedImage& presented, ResourceUsageTag acq_tag)
    : image(presented.image), generator(presented.range_gen), present_tag(presented.tag), acquire_tag(acq_tag) {}

// This is a const method, force the returned value to be const
std::shared_ptr<const SignaledSemaphores::Signal> SignaledSemaphores::GetPrev(VkSemaphore sem) const {
    std::shared_ptr<Signal> prev_state;
    if (prev_) {
        prev_state = syncval_state::GetMapped(prev_->signaled_, sem, [&prev_state]() { return prev_state; });
    }
    return prev_state;
}

SignaledSemaphores::Signal::Signal(const std::shared_ptr<const vvl::Semaphore>& sem_state_,
                                   const std::shared_ptr<QueueBatchContext>& batch_, const SyncExecScope& exec_scope_)
    : sem_state(sem_state_), batch(batch_), first_scope({batch->GetQueueId(), exec_scope_}) {
    // Illegal to create a signal from no batch or an invalid semaphore... caller must assure validity
    assert(batch);
    assert(sem_state);
}

SignaledSemaphores::Signal::Signal(const std::shared_ptr<const vvl::Semaphore>& sem_state_, const PresentedImage& presented,
                                   ResourceUsageTag acq_tag)
    : sem_state(sem_state_), batch(presented.batch), first_scope(), acquired(presented, acq_tag) {
    // Illegal to create a signal from no batch or an invalid semaphore... caller must assure validity
    assert(batch);
    assert(sem_state);
}
bool SignaledSemaphores::SignalSemaphore(const std::shared_ptr<const vvl::Semaphore>& sem_state,
                                         const std::shared_ptr<QueueBatchContext>& batch,
                                         const VkSemaphoreSubmitInfo& signal_info) {
    assert(batch);
    const SyncExecScope exec_scope =
        SyncExecScope::MakeSrc(batch->GetQueueFlags(), signal_info.stageMask, VK_PIPELINE_STAGE_2_HOST_BIT);
    std::shared_ptr<Signal> signal = std::make_shared<Signal>(sem_state, batch, exec_scope);
    return Insert(sem_state, std::move(signal));
}

bool SignaledSemaphores::Insert(const std::shared_ptr<const vvl::Semaphore>& sem_state, std::shared_ptr<Signal>&& signal) {
    const VkSemaphore sem = sem_state->VkHandle();
    auto signal_it = signaled_.find(sem);
    std::shared_ptr<Signal> insert_signal;
    if (signal_it == signaled_.end()) {
        if (prev_) {
            auto prev_sig =
                syncval_state::GetMapped(prev_->signaled_, sem_state->VkHandle(), []() { return std::shared_ptr<Signal>(); });
            if (prev_sig) {
                // The is an invalid signal, as this semaphore is already signaled... copy the prev state (as prev_ is const)
                insert_signal = std::make_shared<Signal>(*prev_sig);
            }
        }
        auto insert_pair = signaled_.emplace(sem, std::move(insert_signal));
        signal_it = insert_pair.first;
    }

    bool success = false;
    if (!signal_it->second) {
        signal_it->second = std::move(signal);
        success = true;
    }

    return success;
}

bool SignaledSemaphores::SignalSemaphore(const std::shared_ptr<const vvl::Semaphore>& sem_state, const PresentedImage& presented,
                                         ResourceUsageTag acq_tag) {
    // Ignore any signal we haven't waited... CoreChecks should have reported this
    std::shared_ptr<Signal> signal = std::make_shared<Signal>(sem_state, presented, acq_tag);
    return Insert(sem_state, std::move(signal));
}

std::shared_ptr<const SignaledSemaphores::Signal> SignaledSemaphores::Unsignal(VkSemaphore sem) {
    std::shared_ptr<const Signal> unsignaled;
    const auto found_it = signaled_.find(sem);
    if (found_it != signaled_.end()) {
        // Move the unsignaled singal out from the signaled list, but keep the shared_ptr as the caller needs the contents for
        // a bit.
        unsignaled = std::move(found_it->second);
        if (!prev_) {
            // No parent, not need to keep the entry
            // IFF (prev_)  leave the entry in the leaf table as we use it to export unsignal to prev_ during record phase
            signaled_.erase(found_it);
        }
    } else if (prev_) {
        // We can't unsignal prev_ because it's const * by design.
        // We put in an empty placeholder
        signaled_.emplace(sem, std::shared_ptr<Signal>());
        unsignaled = GetPrev(sem);
    }
    // NOTE: No else clause. Because if we didn't find it, and there's no previous, this indicates an error,
    // but CoreChecks should have reported it

    // If unsignaled is null, there was a missing pending semaphore, and that's also issue CoreChecks reports
    return unsignaled;
}

void SignaledSemaphores::Resolve(SignaledSemaphores& parent, std::shared_ptr<QueueBatchContext>& last_batch) {
    // Must only be called on child objects, with the non-const reference of the parent/previous object passed in
    assert(prev_ == &parent);

    // The global  the semaphores we applied to the cmd_state QueueBatchContexts
    // NOTE: All conserved QueueBatchContext's need to have there access logs reset to use the global logger and the only conserved
    //       QBC's are those referenced by unwaited signals and the last batch.
    for (auto& sig_sem : signaled_) {
        if (sig_sem.second && sig_sem.second->batch) {
            auto& sig_batch = sig_sem.second->batch;
            // Batches retained for signalled semaphore don't need to retain event data, unless it's the last batch in the submit
            if (sig_batch != last_batch) {
                sig_batch->ResetEventsContext();
                // Make sure that retained batches are minimal, and trim after the events contexts has been cleared.
                sig_batch->Trim();
            }
        }
        // Import clears in the parent any signal waited in the
        parent.Import(sig_sem.first, std::move(sig_sem.second));
    }
    Reset();
}

void SignaledSemaphores::Import(VkSemaphore sem, std::shared_ptr<Signal>&& from) {
    // Overwrite the s  tate with the last state from this
    if (from) {
        assert(sem == from->sem_state->VkHandle());
        signaled_[sem] = std::move(from);
    } else {
        signaled_.erase(sem);
    }
}

void SignaledSemaphores::Reset() {
    signaled_.clear();
    prev_ = nullptr;
}

FenceSyncState::FenceSyncState() : fence(), tag(kInvalidTag), queue_id(kQueueIdInvalid) {}

FenceSyncState::FenceSyncState(const std::shared_ptr<const vvl::Fence>& fence_, QueueId queue_id_, ResourceUsageTag tag_)
    : fence(fence_), tag(tag_), queue_id(queue_id_) {}
FenceSyncState::FenceSyncState(const std::shared_ptr<const vvl::Fence>& fence_, const PresentedImage& image, ResourceUsageTag tag_)
    : fence(fence_), tag(tag_), queue_id(kQueueIdInvalid), acquired(image, tag) {}

syncval_state::Swapchain::Swapchain(ValidationStateTracker* dev_data, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                    VkSwapchainKHR swapchain)
    : vvl::Swapchain(dev_data, pCreateInfo, swapchain) {}

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

QueueBatchContext::QueueBatchContext(const SyncValidator& sync_state, const QueueSyncState& queue_state, uint64_t submit_index,
                                     uint32_t batch_index)
    : CommandExecutionContext(&sync_state),
      queue_state_(&queue_state),
      tag_range_(0, 0),
      current_access_context_(&access_context_),
      batch_log_(),
      queue_sync_tag_(sync_state.GetQueueIdLimit(), ResourceUsageTag(0)),
      batch_(queue_state, submit_index, batch_index) {}

QueueBatchContext::QueueBatchContext(const SyncValidator& sync_state)
    : CommandExecutionContext(&sync_state),
      queue_state_(),
      tag_range_(0, 0),
      current_access_context_(&access_context_),
      batch_log_(),
      queue_sync_tag_(sync_state.GetQueueIdLimit(), ResourceUsageTag(0)),
      batch_() {}

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

void QueueBatchContext::Cleanup() {
    // Clear these after validation and import, not valid after.
    batch_ = BatchAccessLog::BatchRecord();
    command_buffers_.clear();
    async_batches_.clear();
}

// Overload for QueuePresent semaphore waiting.  Not applicable to QueueSubmit semaphores
std::shared_ptr<QueueBatchContext> QueueBatchContext::ResolveOneWaitSemaphore(VkSemaphore sem,
                                                                              const PresentedImages& presented_images,
                                                                              SignaledSemaphores& signaled) {
    auto sem_state = sync_state_->Get<vvl::Semaphore>(sem);
    if (!sem_state) return nullptr;  // Semaphore validity is handled by CoreChecks

    // When signal_state goes out of scope, the signal information will be dropped, as Unsignal has released ownership.
    auto signal_state = signaled.Unsignal(sem);
    if (!signal_state) return nullptr;  // Invalid signal, skip it.

    assert(signal_state->batch);

    const AccessContext& from_context = signal_state->batch->access_context_;
    const SemaphoreScope& signal_scope = signal_state->first_scope;
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

    return signal_state->batch;
}

std::shared_ptr<QueueBatchContext> QueueBatchContext::ResolveOneWaitSemaphore(VkSemaphore sem, VkPipelineStageFlags2 wait_mask,
                                                                              SignaledSemaphores& signaled) {
    auto sem_state = sync_state_->Get<vvl::Semaphore>(sem);
    if (!sem_state) return nullptr;  // Semaphore validity is handled by CoreChecks

    // When signal state goes out of scope, the signal information will be dropped, as Unsignal has released ownership.
    auto signal_state = signaled.Unsignal(sem);
    if (!signal_state) return nullptr;  // Invalid signal, skip it.

    assert(signal_state->batch);

    const SemaphoreScope& signal_scope = signal_state->first_scope;
    const auto queue_flags = queue_state_->GetQueueFlags();
    SemaphoreScope wait_scope{GetQueueId(), SyncExecScope::MakeDst(queue_flags, wait_mask)};

    const AccessContext& from_context = signal_state->batch->access_context_;
    if (signal_state->acquired.image) {
        // Import the *presenting* batch, but replacing presenting with acquired.
        ApplyAcquireNextSemaphoreAction apply_acq(wait_scope, signal_state->acquired.acquire_tag);
        access_context_.ResolveFromContext(apply_acq, from_context, signal_state->acquired.generator);

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
            access_context_.ResolveFromContext(sem_op, signal_state->batch->access_context_);
        }
    }
    // Cannot move from the signal state because it could be from the const global state, and C++ doesn't
    // enforce deep constness.
    return signal_state->batch;
}

void QueueBatchContext::ImportSyncTags(const QueueBatchContext& from) {
    // NOTE: Assumes that from has set it's tag limit in it's own queue_id slot.
    size_t q_limit = queue_sync_tag_.size();
    assert(q_limit == from.queue_sync_tag_.size());
    for (size_t q = 0; q < q_limit; q++) {
        queue_sync_tag_[q] = std::max(queue_sync_tag_[q], from.queue_sync_tag_[q]);
    }
}

void QueueBatchContext::SetupAccessContext(const std::shared_ptr<const QueueBatchContext>& prev,
                                           const VkPresentInfoKHR& present_info, const PresentedImages& presented_images,
                                           SignaledSemaphores& signaled) {
    ConstBatchSet batches_resolved;
    for (VkSemaphore sem : vvl::make_span(present_info.pWaitSemaphores, present_info.waitSemaphoreCount)) {
        std::shared_ptr<QueueBatchContext> resolved = ResolveOneWaitSemaphore(sem, presented_images, signaled);
        if (resolved) {
            batches_resolved.emplace(std::move(resolved));
        }
    }
    CommonSetupAccessContext(prev, batches_resolved);
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
            skip = sync_state_->LogError(
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

void QueueBatchContext::LogPresentOperations(const PresentedImages& presented_images) {
    if (tag_range_.size()) {
        auto access_log = std::make_shared<AccessLog>();
        batch_log_.Insert(batch_, tag_range_, access_log);
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
    batch_log_.Insert(batch_, tag_range_, access_log);
    access_log->emplace_back(AcquireResourceRecord(presented, tag_range_.begin, command));
}

void QueueBatchContext::SetupAccessContext(const std::shared_ptr<const QueueBatchContext>& prev, const VkSubmitInfo2& submit_info,
                                           SignaledSemaphores& signaled) {
    // Import (resolve) the batches that are waited on, with the semaphore's effective barriers applied
    ConstBatchSet batches_resolved;
    const uint32_t wait_count = submit_info.waitSemaphoreInfoCount;
    const VkSemaphoreSubmitInfo* wait_infos = submit_info.pWaitSemaphoreInfos;
    for (const auto& wait_info : vvl::make_span(wait_infos, wait_count)) {
        std::shared_ptr<QueueBatchContext> resolved = ResolveOneWaitSemaphore(wait_info.semaphore, wait_info.stageMask, signaled);
        if (resolved) {
            batches_resolved.emplace(std::move(resolved));
        }
    }
    CommonSetupAccessContext(prev, batches_resolved);
}

void QueueBatchContext::SetupAccessContext(const PresentedImage& presented) {
    if (presented.batch) {
        access_context_.ResolveFromContext(presented.batch->access_context_);
        batch_log_.Import(presented.batch->batch_log_);
        ImportSyncTags(*presented.batch);
    }
}

void QueueBatchContext::CommonSetupAccessContext(const std::shared_ptr<const QueueBatchContext>& prev,
                                                 QueueBatchContext::ConstBatchSet& batches_resolved) {
    // Import the previous batch information
    if (prev) {
        // Copy in the event state from the previous batch (on this queue)
        events_context_.DeepCopy(prev->events_context_);
        if (!vvl::Contains(batches_resolved, prev)) {
            // If there are no semaphores to the previous batch, make sure a "submit order" non-barriered import is done
            access_context_.ResolveFromContext(prev->access_context_);
            batches_resolved.emplace(prev);
        }
    }

    // Get all the log and tag sync information for the resolved contexts
    for (const auto& batch : batches_resolved) {
        batch_log_.Import(batch->batch_log_);
        ImportSyncTags(*batch);
    }

    // Gather async context information for hazard checks and conserve the QBC's for the async batches
    async_batches_ =
        sync_state_->GetQueueLastBatchSnapshot([&batches_resolved](const std::shared_ptr<const QueueBatchContext>& batch) {
            return !vvl::Contains(batches_resolved, batch);
        });
    for (const auto& async_batch : async_batches_) {
        const QueueId async_queue = async_batch->GetQueueId();
        ResourceUsageTag sync_tag;
        if (async_queue < queue_sync_tag_.size()) {
            sync_tag = queue_sync_tag_[async_queue];
        } else {
            // If this isn't from a tracked queue, just check the batch itself
            sync_tag = async_batch->GetTagRange().begin;
        }

        // The start of the asynchronous access range for a given queue is one more than the highest tagged reference
        access_context_.AddAsyncContext(async_batch->GetCurrentAccessContext(), sync_tag);
        // We need to snapshot the async log information for async hazard reporting
        batch_log_.Import(async_batch->batch_log_);
    }
}

void QueueBatchContext::SetupCommandBufferInfo(const VkSubmitInfo2& submit_info) {
    // Create the list of command buffers to submit
    const uint32_t cb_count = submit_info.commandBufferInfoCount;
    const VkCommandBufferSubmitInfo* const cb_infos = submit_info.pCommandBufferInfos;
    command_buffers_.reserve(cb_count);

    for (const auto& cb_info : vvl::make_span(cb_infos, cb_count)) {
        auto cb_state = sync_state_->Get<syncval_state::CommandBuffer>(cb_info.commandBuffer);
        if (cb_state) {
            tag_range_.end += cb_state->access_context.GetTagLimit();
            command_buffers_.emplace_back(static_cast<uint32_t>(&cb_info - cb_infos), std::move(cb_state));
        }
    }
}

// Look up the usage informaiton from the local or global logger
std::string QueueBatchContext::FormatUsage(ResourceUsageTag tag) const {
    std::stringstream out;
    BatchAccessLog::AccessRecord access = batch_log_[tag];
    if (access.IsValid()) {
        const BatchAccessLog::BatchRecord& batch = *access.batch;
        const ResourceUsageRecord& record = *access.record;
        if (batch.queue) {
            // Queue and Batch information (for enqueued operations)
            out << SyncNodeFormatter(*sync_state_, batch.queue->GetQueueState());
            out << ", submit: " << batch.submit_index << ", batch: " << batch.batch_index;
        }
        out << ", batch_tag: " << batch.bias;

        // Commandbuffer Usages Information
        out << ", " << record.Formatter(*sync_state_, nullptr);
    }
    return out.str();
}

VkQueueFlags QueueBatchContext::GetQueueFlags() const { return queue_state_->GetQueueFlags(); }

QueueId QueueBatchContext::GetQueueId() const {
    QueueId id = queue_state_ ? queue_state_->GetQueueId() : kQueueIdInvalid;
    return id;
}

// For QueuePresent, the tag range is defined externally and must be passed in
void QueueBatchContext::SetupBatchTags(const ResourceUsageRange& tag_range) {
    tag_range_ = tag_range;
    SetupBatchTags();
}

// For QueueSubmit, the tag range is defined by the CommandBuffer setup.
// For QueuePresent, this is called when the tag_range is specified
void QueueBatchContext::SetupBatchTags() {
    // Need new global tags for all accesses... the Reserve updates a mutable atomic
    ResourceUsageRange global_tags = sync_state_->ReserveGlobalTagRange(GetTagRange().size());
    SetTagBias(global_tags.begin);
}

void QueueBatchContext::InsertRecordedAccessLogEntries(const CommandBufferAccessContext& submitted_cb) {
    const ResourceUsageTag end_tag = batch_log_.Import(batch_, submitted_cb);
    batch_.bias = end_tag;
    batch_.cb_index++;
}

void QueueBatchContext::SetTagBias(ResourceUsageTag bias) {
    const auto size = tag_range_.size();
    tag_range_.begin = bias;
    tag_range_.end = bias + size;
    access_context_.SetStartTag(bias);
    batch_.bias = bias;

    // Needed for ImportSyncTags to pick up the "from" own sync tag.
    const QueueId this_q = GetQueueId();
    if (this_q < queue_sync_tag_.size()) {
        // If this is a non-queued operation we'll get a "special" value like invalid
        queue_sync_tag_[this_q] = tag_range_.end;
    }
}

bool QueueBatchContext::DoQueueSubmitValidate(const SyncValidator& sync_state, QueueSubmitCmdState& cmd_state,
                                              const VkSubmitInfo2& batch_info) {
    bool skip = false;

    //  For each submit in the batch...
    for (const auto& cb : command_buffers_) {
        const auto& cb_access_context = cb.cb->access_context;
        if (cb_access_context.GetTagLimit() == 0) {
            batch_.cb_index++;
            continue;  // Skip empty CB's but also skip the unused index for correct reporting
        }
        skip |= ReplayState(*this, cb_access_context, cmd_state.error_obj, cb.index).ValidateFirstUse();

        // The barriers have already been applied in ValidatFirstUse
        ResourceUsageRange tag_range = ImportRecordedAccessLog(cb_access_context);
        ResolveSubmittedCommandBuffer(*cb_access_context.GetCurrentAccessContext(), tag_range.begin);
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
void QueueSyncState::UpdateLastBatch(std::shared_ptr<QueueBatchContext>&& new_last) {
    // Update the queue to point to the last batch from the submit
    if (new_last) {
        // Clean up the events data in the previous last batch on queue, as only the subsequent batches have valid use for them
        // and the QueueBatchContext::Setup calls have be copying them along from batch to batch during submit.
        if (last_batch_) {
            last_batch_->ResetEventsContext();
        }
        new_last->Trim();
        last_batch_ = std::move(new_last);
    }
}

template <typename T>
struct GetBatchTraits {};
template <>
struct GetBatchTraits<std::shared_ptr<QueueSyncState>> {
    using Batch = std::shared_ptr<QueueBatchContext>;
    static Batch Get(const std::shared_ptr<QueueSyncState>& qss) { return qss ? qss->LastBatch() : Batch(); }
};

template <>
struct GetBatchTraits<std::shared_ptr<SignaledSemaphores::Signal>> {
    using Batch = std::shared_ptr<QueueBatchContext>;
    static Batch Get(const std::shared_ptr<SignaledSemaphores::Signal>& sig) { return sig ? sig->batch : Batch(); }
};

template <typename BatchSet, typename Map, typename Predicate>
static BatchSet GetQueueBatchSnapshotImpl(const Map& map, Predicate&& pred) {
    BatchSet snapshot;
    for (auto& entry : map) {
        // Intentional copy
        auto batch = GetBatchTraits<typename Map::mapped_type>::Get(entry.second);
        if (batch && pred(batch)) snapshot.emplace(std::move(batch));
    }
    return snapshot;
}

template <typename Predicate>
QueueBatchContext::ConstBatchSet SyncValidator::GetQueueLastBatchSnapshot(Predicate&& pred) const {
    return GetQueueBatchSnapshotImpl<QueueBatchContext::ConstBatchSet>(queue_sync_states_, std::forward<Predicate>(pred));
}

template <typename Predicate>
QueueBatchContext::BatchSet SyncValidator::GetQueueLastBatchSnapshot(Predicate&& pred) {
    return GetQueueBatchSnapshotImpl<QueueBatchContext::BatchSet>(queue_sync_states_, std::forward<Predicate>(pred));
}

QueueBatchContext::BatchSet SyncValidator::GetQueueBatchSnapshot() {
    QueueBatchContext::BatchSet snapshot = GetQueueLastBatchSnapshot();
    auto append = [&snapshot](const std::shared_ptr<QueueBatchContext>& batch) {
        if (batch && !vvl::Contains(snapshot, batch)) {
            snapshot.emplace(batch);
        }
        return false;
    };
    GetQueueBatchSnapshotImpl<QueueBatchContext::BatchSet>(signaled_semaphores_, append);
    return snapshot;
}

// Note that function is const, but updates mutable submit_index to allow Validate to create correct tagging for command invocation
// scope state.
// Given that queue submits are supposed to be externally synchronized for the same queue, this should safe without being
// atomic... but as the ops are per submit, the performance cost is negible for the peace of mind.
uint64_t QueueSyncState::ReserveSubmitId() const { return submit_index_.fetch_add(1); }

VkSemaphoreSubmitInfo SubmitInfoConverter::BatchStore::WaitSemaphore(const VkSubmitInfo& info, uint32_t index) {
    VkSemaphoreSubmitInfo semaphore_info = vku::InitStructHelper();
    semaphore_info.semaphore = info.pWaitSemaphores[index];
    semaphore_info.stageMask = info.pWaitDstStageMask[index];
    return semaphore_info;
}
VkCommandBufferSubmitInfo SubmitInfoConverter::BatchStore::CommandBuffer(const VkSubmitInfo& info, uint32_t index) {
    VkCommandBufferSubmitInfo cb_info = vku::InitStructHelper();
    cb_info.commandBuffer = info.pCommandBuffers[index];
    return cb_info;
}

VkSemaphoreSubmitInfo SubmitInfoConverter::BatchStore::SignalSemaphore(const VkSubmitInfo& info, uint32_t index,
                                                                       VkQueueFlags queue_flags) {
    VkSemaphoreSubmitInfo semaphore_info = vku::InitStructHelper();
    semaphore_info.semaphore = info.pSignalSemaphores[index];
    // Can't just use BOTTOM, because of how access expansion is done
    semaphore_info.stageMask =
        sync_utils::ExpandPipelineStages(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, queue_flags, VK_PIPELINE_STAGE_2_HOST_BIT);
    return semaphore_info;
}

SubmitInfoConverter::BatchStore::BatchStore(const VkSubmitInfo& info, VkQueueFlags queue_flags) {
    info2 = vku::InitStructHelper();

    info2.waitSemaphoreInfoCount = info.waitSemaphoreCount;
    waits.reserve(info2.waitSemaphoreInfoCount);
    for (uint32_t i = 0; i < info2.waitSemaphoreInfoCount; ++i) {
        waits.emplace_back(WaitSemaphore(info, i));
    }
    info2.pWaitSemaphoreInfos = waits.data();

    info2.commandBufferInfoCount = info.commandBufferCount;
    cbs.reserve(info2.commandBufferInfoCount);
    for (uint32_t i = 0; i < info2.commandBufferInfoCount; ++i) {
        cbs.emplace_back(CommandBuffer(info, i));
    }
    info2.pCommandBufferInfos = cbs.data();

    info2.signalSemaphoreInfoCount = info.signalSemaphoreCount;
    signals.reserve(info2.signalSemaphoreInfoCount);
    for (uint32_t i = 0; i < info2.signalSemaphoreInfoCount; ++i) {
        signals.emplace_back(SignalSemaphore(info, i, queue_flags));
    }
    info2.pSignalSemaphoreInfos = signals.data();
}

SubmitInfoConverter::SubmitInfoConverter(uint32_t count, const VkSubmitInfo* infos, VkQueueFlags queue_flags) {
    info_store.reserve(count);
    info2s.reserve(count);
    for (uint32_t batch = 0; batch < count; ++batch) {
        info_store.emplace_back(infos[batch], queue_flags);
        info2s.emplace_back(info_store.back().info2);
    }
}

ResourceUsageTag BatchAccessLog::Import(const BatchRecord& batch, const CommandBufferAccessContext& cb_access) {
    ResourceUsageTag bias = batch.bias;
    ResourceUsageTag tag_limit = bias + cb_access.GetTagLimit();
    ResourceUsageRange import_range = {bias, tag_limit};
    log_map_.insert(std::make_pair(import_range, CBSubmitLog(batch, cb_access)));
    return tag_limit;
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

BatchAccessLog::AccessRecord BatchAccessLog::operator[](ResourceUsageTag tag) const {
    auto found_log = log_map_.find(tag);
    if (found_log != log_map_.cend()) {
        return found_log->second[tag];
    }
    // tag not found
    assert(false);
    return AccessRecord();
}

BatchAccessLog::AccessRecord BatchAccessLog::CBSubmitLog::operator[](ResourceUsageTag tag) const {
    assert(tag >= batch_.bias);
    const size_t index = tag - batch_.bias;
    assert(log_);
    assert(index < log_->size());
    return AccessRecord{&batch_, &(*log_)[index]};
}

BatchAccessLog::CBSubmitLog::CBSubmitLog(const BatchRecord& batch, const CommandBufferAccessContext& cb)
    : CBSubmitLog(batch, cb.GetCBReferencesShared(), cb.GetAccessLogShared()) {}

PresentedImage::PresentedImage(const SyncValidator& sync_state, const std::shared_ptr<QueueBatchContext> batch_,
                               VkSwapchainKHR swapchain, uint32_t image_index_, uint32_t present_index_, ResourceUsageTag tag_)
    : PresentedImageRecord{tag_, image_index_, present_index_, sync_state.Get<syncval_state::Swapchain>(swapchain), {}},
      batch(std::move(batch_)) {
    SetImage(image_index_);
}

PresentedImage::PresentedImage(std::shared_ptr<const syncval_state::Swapchain> swapchain, uint32_t at_index) : PresentedImage() {
    swapchain_state = std::move(swapchain);
    tag = kInvalidTag;
    SetImage(at_index);
}

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
