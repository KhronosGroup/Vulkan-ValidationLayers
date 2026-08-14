/* Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 Valve Corporation
 * Copyright (c) 2026 LunarG, Inc.
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
#include "sync/sync_barrier.h"
#include "containers/span.h"
#include "generated/error_location_helper.h"
#include <memory>
#include <vector>

namespace vvl {
class Event;
}

struct Location;

namespace syncval {

class AccessContext;
struct SyncEnvironment;

struct SyncEventState {
    using EventPointer = std::shared_ptr<const vvl::Event>;
    EventPointer event;
    vvl::Func last_command;             // Only Event commands are valid here.
    ResourceUsageTag last_command_tag;  // Needed to filter replay validation
    vvl::Func unsynchronized_set;
    VkPipelineStageFlags2 barriers;
    SyncExecScope scope;
    ResourceUsageTag first_scope_tag;
    std::shared_ptr<const AccessContext> first_scope;

    SyncEventState()
        : event(),
          last_command(vvl::Func::Empty),
          last_command_tag(0),
          unsynchronized_set(vvl::Func::Empty),
          barriers(0U),
          scope(),
          first_scope_tag() {}

    SyncEventState(const SyncEventState&) = default;
    SyncEventState(SyncEventState&&) = default;

    SyncEventState(const SyncEventState::EventPointer& event_state);

    void ResetFirstScope();
    const AccessContext::ScopeMap& FirstScope() const { return first_scope->GetAccessMap(); }
    bool HasBarrier(VkPipelineStageFlags2 stageMask, VkPipelineStageFlags2 exec_scope) const;
    void AddReferencedTags(ResourceUsageTagSet& referenced) const;
};

class SyncEventsContext {
  public:
    using Map = vvl::unordered_map<const vvl::Event*, std::shared_ptr<SyncEventState>>;
    using iterator = Map::iterator;
    using const_iterator = Map::const_iterator;

    SyncEventState* GetFromShared(const SyncEventState::EventPointer& event_state) {
        const auto find_it = map_.find(event_state.get());
        if (find_it == map_.end()) {
            if (!event_state.get()) return nullptr;

            const auto* event_plain_ptr = event_state.get();
            auto sync_state = std::make_shared<SyncEventState>(event_state);
            auto insert_pair = map_.emplace(event_plain_ptr, sync_state);
            return insert_pair.first->second.get();
        }
        return find_it->second.get();
    }

    const SyncEventState* Get(const SyncEventState::EventPointer& event_state) const {
        const auto find_it = map_.find(event_state.get());
        if (find_it == map_.end()) {
            return nullptr;
        }
        return find_it->second.get();
    }

    void ApplyBarrier(const SyncExecScope& src, const SyncExecScope& dst, ResourceUsageTag tag);
    void ApplyTaggedWait(VkQueueFlags queue_flags, ResourceUsageTag tag);

    void Destroy(const vvl::Event* event_state) {
        auto sync_it = map_.find(event_state);
        if (sync_it != map_.end()) {
            map_.erase(sync_it);
        }
    }
    void Clear() { map_.clear(); }

    SyncEventsContext& DeepCopy(const SyncEventsContext& from);
    void AddReferencedTags(ResourceUsageTagSet& referenced) const;

  private:
    Map map_;
};

bool ValidateCmdSetEvent(const SyncEnvironment& env, const std::shared_ptr<const vvl::Event>& event,
                         const SyncExecScope& src_exec_scope, ResourceUsageTag base_tag, const Location& loc);

bool ValidateCmdResetEvent(const SyncEnvironment& env, const std::shared_ptr<const vvl::Event>& event,
                           const SyncExecScope& exec_scope, ResourceUsageTag base_tag, const Location& loc);

bool ValidateCmdWaitEvents(const SyncEnvironment& env, const std::vector<std::shared_ptr<const vvl::Event>>& events,
                           const ResourceUsageTag base_tag, const Location& loc);

bool DetectCmdWaitEventsImageBarrierHazard(const SyncEnvironment& env, const AccessContext& access_context,
                                           const std::vector<std::shared_ptr<const vvl::Event>>& events,
                                           const vvl::span<const BarrierSet>& barrier_sets, ResourceUsageTag base_tag,
                                           const Location& loc);

// Main functionality of the correspodning Record methods, which perform additional setup
void ApplyCmdSetEvent(SyncEnvironment& env, const std::shared_ptr<const vvl::Event>& event, const SyncExecScope& src_exec_scope,
                      const std::shared_ptr<const AccessContext>& src_access_context, ResourceUsageTag tag, vvl::Func command);

void ApplyCmdResetEvent(SyncEnvironment& env, const std::shared_ptr<const vvl::Event>& event, ResourceUsageTag tag,
                        vvl::Func command);

void ApplyCmdWaitEvents(SyncEnvironment& env, AccessContext& access_context,
                        const std::vector<std::shared_ptr<const vvl::Event>>& events, vvl::span<const BarrierSet> barrier_sets,
                        ResourceUsageTag tag, vvl::Func command);

}  // namespace syncval
