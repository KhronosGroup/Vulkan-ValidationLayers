// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See dispatch_object_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
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
 ****************************************************************************/

// NOLINTBEGIN

#include "chassis/dispatch_object.h"
#include "thread_tracker/thread_safety_validation.h"
#include "stateless/stateless_validation.h"
#include "generated/legacy.h"
#include "object_tracker/object_lifetime_validation.h"
#include "state_tracker/state_tracker.h"
#include "core_checks/core_validation.h"
#include "best_practices/best_practices_validation.h"
#include "gpuav/core/gpuav.h"
#include "sync/sync_validation.h"
#include "gpu_dump/gpu_dump.h"

namespace vvl {

void DispatchInstance::InitValidationObjects() {
    // Note that this DEFINES THE ORDER IN WHICH THE LAYER VALIDATION OBJECTS ARE CALLED
    // Anyone using state tracking (LayerObjectTypeStateTracker) must be called after and enable it

    if (!settings.disabled[thread_safety]) {
        object_dispatch.emplace_back(new threadsafety::Instance(this));
    }
    if (!settings.disabled[stateless_checks]) {
        object_dispatch.emplace_back(new stateless::Instance(this));
    }
    if (settings.enabled[legacy_detection]) {
        object_dispatch.emplace_back(new legacy::Instance(this));
    }
    if (!settings.disabled[object_tracking]) {
        object_dispatch.emplace_back(new object_lifetimes::Instance(this));
    }
    if (!settings.disabled[core_checks] || settings.enabled[best_practices] || settings.enabled[gpu_validation] ||
        settings.enabled[debug_printf_validation] || settings.enabled[sync_validation] || settings.enabled[gpu_dump]) {
        object_dispatch.emplace_back(new vvl::InstanceState(this));
    }
    if (!settings.disabled[core_checks]) {
        object_dispatch.emplace_back(new core::Instance(this));
    }
    if (settings.enabled[best_practices]) {
        object_dispatch.emplace_back(new bp_state::Instance(this));
    }
    if (settings.enabled[gpu_validation] || settings.enabled[debug_printf_validation]) {
        object_dispatch.emplace_back(new gpuav::Instance(this));
    }
    if (settings.enabled[sync_validation]) {
        object_dispatch.emplace_back(new syncval::Instance(this));
    }
    if (settings.enabled[gpu_dump]) {
        object_dispatch.emplace_back(new gpudump::Instance(this));
    }
}

void DispatchDevice::InitValidationObjects() {
    // Note that this DEFINES THE ORDER IN WHICH THE LAYER VALIDATION OBJECTS ARE CALLED
    // Anyone using state tracking (LayerObjectTypeStateTracker) must be called after and enable it

    if (!settings.disabled[thread_safety]) {
        object_dispatch.emplace_back(new threadsafety::Device(
            this, static_cast<threadsafety::Instance*>(dispatch_instance->GetValidationObject(LayerObjectTypeThreading))));
    }
    if (!settings.disabled[stateless_checks]) {
        object_dispatch.emplace_back(new stateless::Device(
            this, static_cast<stateless::Instance*>(dispatch_instance->GetValidationObject(LayerObjectTypeParameterValidation))));
    }
    if (settings.enabled[legacy_detection]) {
        object_dispatch.emplace_back(new legacy::Device(
            this, static_cast<legacy::Instance*>(dispatch_instance->GetValidationObject(LayerObjectTypeLegacy))));
    }
    if (!settings.disabled[object_tracking]) {
        object_dispatch.emplace_back(new object_lifetimes::Device(
            this, static_cast<object_lifetimes::Instance*>(dispatch_instance->GetValidationObject(LayerObjectTypeObjectTracker))));
    }
    if (!settings.disabled[core_checks] || settings.enabled[best_practices] || settings.enabled[gpu_validation] ||
        settings.enabled[debug_printf_validation] || settings.enabled[sync_validation] || settings.enabled[gpu_dump]) {
        object_dispatch.emplace_back(new vvl::DeviceState(
            this, static_cast<vvl::InstanceState*>(dispatch_instance->GetValidationObject(LayerObjectTypeStateTracker))));
    }
    if (!settings.disabled[core_checks]) {
        object_dispatch.emplace_back(new CoreChecks(
            this, static_cast<core::Instance*>(dispatch_instance->GetValidationObject(LayerObjectTypeCoreValidation))));
    }
    if (settings.enabled[best_practices]) {
        object_dispatch.emplace_back(new BestPractices(
            this, static_cast<bp_state::Instance*>(dispatch_instance->GetValidationObject(LayerObjectTypeBestPractices))));
    }
    if (settings.enabled[gpu_validation] || settings.enabled[debug_printf_validation]) {
        object_dispatch.emplace_back(new gpuav::Validator(
            this, static_cast<gpuav::Instance*>(dispatch_instance->GetValidationObject(LayerObjectTypeGpuAssisted))));
    }
    if (settings.enabled[sync_validation]) {
        object_dispatch.emplace_back(new syncval::SyncValidator(
            this, static_cast<syncval::Instance*>(dispatch_instance->GetValidationObject(LayerObjectTypeSyncValidation))));
    }
    if (settings.enabled[gpu_dump]) {
        object_dispatch.emplace_back(new gpudump::GpuDump(
            this, static_cast<gpudump::Instance*>(dispatch_instance->GetValidationObject(LayerObjectTypeGpuDump))));
    }
}
}  // namespace vvl

// NOLINTEND
