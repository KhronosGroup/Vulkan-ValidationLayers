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

#include <stdint.h>
#include "pass.h"

#include <vulkan/utility/vk_struct_helper.hpp>
#include "gpuav/resources/gpuav_vulkan_objects.h"

namespace gpuav {

// Vertex Attribute Fetch OOB checks are for Indexed draws, so when using another draw, we set the values of the small buffer to
// zero to indicate the instrumented vertex shader to skip validating the limits. We have a single global buffer that we can have
// all non-index draws point at.
struct VertexAttributeFetchOff {
    vko::Buffer buffer;
    bool valid = false;

    VertexAttributeFetchOff(Validator& gpuav) : buffer(gpuav) {
        VkBufferCreateInfo buffer_info = vku::InitStructHelper();
        buffer_info.size = 4 * sizeof(uint32_t);
        buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VmaAllocationCreateInfo alloc_info = {};
        alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        alloc_info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        const bool success = buffer.Create(&buffer_info, &alloc_info);
        if (!success) {
            valid = false;
            return;
        }
        valid = true;

        auto vertex_attribute_fetch_limits_buffer_ptr = (uint32_t *)buffer.GetMappedPtr();
        vertex_attribute_fetch_limits_buffer_ptr[0] = 0u;  // has_max_vbb_vertex_input_rate
        vertex_attribute_fetch_limits_buffer_ptr[2] = 0u;  // has_max_vbb_instance_input_rate
    }

    ~VertexAttributeFetchOff() { buffer.Destroy(); }
};

namespace spirv {

// Validating validating that gl_VertexID is an index within bound vertex buffers
class VertexAttributeFetchOob : public Pass {
  public:
    VertexAttributeFetchOob(Module& module);
    const char* Name() const final { return "VertexAttributeFetchOob"; }

    bool Instrument();
    void PrintDebugInfo() const final;

  private:
    uint32_t GetLinkFunctionId();

    bool instrumentation_performed = false;

    // Function IDs to link in
    uint32_t link_function_id_ = 0;
};

}  // namespace spirv
}  // namespace gpuav
