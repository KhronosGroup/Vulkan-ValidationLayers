/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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
    bool submit_time_validation = true;
    bool shader_accesses_heuristic = false;

    // This validation currently is controlled only by the settings and is disabled by default.
    // There is a discussion https://gitlab.khronos.org/vulkan/vulkan/-/issues/4513 to clarify
    // the spec and under which conditions this validation should be active.
    bool load_op_after_store_op_validation = false;

    bool message_extra_properties = false;
};
