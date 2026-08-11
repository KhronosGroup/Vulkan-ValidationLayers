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

#include "sync_barrier.h"
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

bool ValidateCmdSetEvent(const SyncEnvironment& env, const std::shared_ptr<const vvl::Event>& event,
                         const SyncExecScope& src_exec_scope, ResourceUsageTag base_tag, const Location& loc);

bool ValidateCmdResetEvent(const SyncEnvironment& env, const std::shared_ptr<const vvl::Event>& event,
                           const SyncExecScope& exec_scope, ResourceUsageTag base_tag, const Location& loc);

bool ValidateCmdWaitEvents(const SyncEnvironment& env, const AccessContext& access_context,
                           const std::vector<std::shared_ptr<const vvl::Event>>& events,
                           const vvl::span<const BarrierSet>& barrier_sets, const ResourceUsageTag base_tag, const Location& loc);

// Main functionality of the correspodning Record methods, which perform additional setup
void ApplyCmdSetEvent(SyncEnvironment& env, const std::shared_ptr<const vvl::Event>& event, const SyncExecScope& src_exec_scope,
                      const std::shared_ptr<const AccessContext>& src_access_context, ResourceUsageTag tag, vvl::Func command);

void ApplyCmdResetEvent(SyncEnvironment& env, const std::shared_ptr<const vvl::Event>& event, ResourceUsageTag tag,
                        vvl::Func command);

void ApplyCmdWaitEvents(SyncEnvironment& env, AccessContext& access_context,
                        const std::vector<std::shared_ptr<const vvl::Event>>& events, vvl::span<const BarrierSet> barrier_sets,
                        ResourceUsageTag tag, vvl::Func command);

}  // namespace syncval
