/* Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
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

#include "sync/sync_commandbuffer.h"

struct SyncNodeFormatter {
    const DebugReport *debug_report;
    const vvl::StateObject *node;
    const char *label;

    SyncNodeFormatter(const SyncValidator &sync_state, const vvl::CommandBuffer *cb_state);
    SyncNodeFormatter(const SyncValidator &sync_state, const vvl::Image *image);
    SyncNodeFormatter(const SyncValidator &sync_state, const vvl::Queue *q_state);
    SyncNodeFormatter(const SyncValidator &sync_state, const vvl::StateObject *state_object, const char *label_ = nullptr);
};
std::string FormatStateObject(const SyncNodeFormatter &formatter);
