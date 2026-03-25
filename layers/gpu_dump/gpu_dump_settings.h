/* Copyright (c) 2026 Valve Corporation
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

// Default values for those settings should match layers/VkLayer_khronos_validation.json.in

struct GpuDumpSettings {
    // Main settings
    bool descriptors = false;

    // Additional helpers
    bool to_stdout = false;

    // If any of the dump settings are turned on, we will enable the layer in the chassis
    bool EnableLayer() const { return descriptors; }
};
