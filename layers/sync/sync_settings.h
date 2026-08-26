/* Copyright (c) 2025-2026 The Khronos Group Inc.
 * Copyright (c) 2025-2026 Valve Corporation
 * Copyright (c) 2025-2026 LunarG, Inc.
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

struct SyncValSettings {
    bool full_validation = false;
    bool record_time_validation = true;
    bool legacy_submit_time_validation = true;  // TODO: remove after refactor
    bool shader_accesses_heuristic = false;

    // TODO: remove this and replace with direct record_time_validation access after refactor
    bool IsRecordTimeValidationEnabled() const { return record_time_validation || legacy_submit_time_validation; }

    bool IsSubmitTimeProcessingEnabled() const { return legacy_submit_time_validation || full_validation; }

    // This validation currently is controlled only by the settings and is disabled by default.
    // There is a discussion https://gitlab.khronos.org/vulkan/vulkan/-/issues/4513 to clarify
    // the spec and under which conditions this validation should be active.
    bool load_op_after_store_op_validation = false;

    bool message_extra_properties = false;
};
