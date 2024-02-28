
/* Copyright (c) 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2020-2024 Valve Corporation
 * Copyright (c) 2020-2024 LunarG, Inc.
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
struct GpuAVSettings {
    bool validate_descriptors = true;
    bool validate_indirect_buffer = true;
    bool validate_copies = true;
    bool validate_ray_query = true;
    bool vma_linear_output = true;
    bool warn_on_robust_oob = true;
    bool cache_instrumented_shaders = true;
    bool select_instrumented_shaders = false;
    uint32_t gpuav_max_buffer_device_addresses = 10000;

    bool gpuav_debug_validate_instrumented_shaders = false;
    bool gpuav_debug_dump_instrumented_shaders = false;
};