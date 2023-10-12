
/* Copyright (c) 2020-2023 The Khronos Group Inc.
 * Copyright (c) 2020-2023 Valve Corporation
 * Copyright (c) 2020-2023 LunarG, Inc.
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
typedef struct {
    bool validate_descriptors;
    bool validate_draw_indirect;
    bool validate_dispatch_indirect;
    bool vma_linear_output;
    bool warn_on_robust_oob;
    bool cache_instrumented_shaders;
    bool select_instrumented_shaders;
    uint32_t gpuav_max_buffer_device_addresses;
} GpuAVSettings;