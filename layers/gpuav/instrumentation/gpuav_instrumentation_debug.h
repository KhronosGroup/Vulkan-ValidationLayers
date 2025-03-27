/* Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
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

#include <vulkan/vulkan.h>
#include <cstdint>
#include <filesystem>
#include <string>

namespace gpuav {

struct InstrumentedShaderDebugInfo {
    VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;

    // instrumenation counts
    uint32_t bda{};
    uint32_t dbgpf{};
    uint32_t dcgb{};
    uint32_t dctb{};
    uint32_t dioob{};
    uint32_t postp{};
    uint32_t rayq{};
    uint32_t vaoob{};

    void Clear() { *this = InstrumentedShaderDebugInfo(); }
    std::string ToStringFileSuffix() const;
};

extern thread_local InstrumentedShaderDebugInfo tl_instrumentation_debug_info;

void DumpSpirvToFile(const std::filesystem::path &file_path, const char *spirv_data, size_t spirv_size);

}  // namespace gpuav
