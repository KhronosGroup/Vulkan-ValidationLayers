/* Copyright (c) 2018-2026 The Khronos Group Inc.
 * Copyright (c) 2018-2026 Valve Corporation
 * Copyright (c) 2018-2026 LunarG, Inc.
 * Copyright (c) 2025 Arm Limited.
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

#include "containers/custom_containers.h"
#include "gpuav/resources/gpuav_vulkan_objects.h"

#include <memory>

namespace vvl {
class DescriptorSet;
}

namespace gpuav {

namespace vko {
class GpuResourcesManager;
class StagingBuffer;
}  // namespace vko
struct QueueSubmission {
    vko::GpuResourcesManager* resources_manager = nullptr;
};

struct CommandBufferSubmitInfo {
    std::shared_ptr<vvl::unordered_map<std::shared_ptr<vvl::DescriptorSet>, vko::StagingBuffer>> bound_desc_sets_to_pp_buffer_map;
};
}  // namespace gpuav
