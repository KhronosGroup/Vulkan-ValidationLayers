/* Copyright (c) 2018-2024 The Khronos Group Inc.
 * Copyright (c) 2018-2024 Valve Corporation
 * Copyright (c) 2018-2024 LunarG, Inc.
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

#include "gpu/core/gpuav.h"

#include "gpu/core/gpuav_constants.h"
#include "gpu/resources/gpuav_subclasses.h"

#include <cmath>
#include <fstream>

namespace gpuav {

bool Validator::AllocateErrorLogsBuffer(gpu::DeviceMemoryBlock &error_logs_mem, const Location &loc) {
    VkBufferCreateInfo buffer_info = vku::InitStructHelper();
    buffer_info.size = output_buffer_byte_size_;
    buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    alloc_info.pool = output_buffer_pool_;
    VkResult result =
        vmaCreateBuffer(vma_allocator_, &buffer_info, &alloc_info, &error_logs_mem.buffer, &error_logs_mem.allocation, nullptr);
    if (result != VK_SUCCESS) {
        InternalError(device, loc, "Unable to allocate device memory for error output buffer. Aborting GPU-AV.", true);
        return false;
    }

    uint32_t *output_buffer_ptr;
    result = vmaMapMemory(vma_allocator_, error_logs_mem.allocation, reinterpret_cast<void **>(&output_buffer_ptr));
    if (result == VK_SUCCESS) {
        memset(output_buffer_ptr, 0, output_buffer_byte_size_);
        if (gpuav_settings.validate_descriptors) {
            output_buffer_ptr[cst::stream_output_flags_offset] = cst::inst_buffer_oob_enabled;
        }
        vmaUnmapMemory(vma_allocator_, error_logs_mem.allocation);
    } else {
        InternalError(device, loc, "Unable to map device memory allocated for error output buffer. Aborting GPU-AV.", true);
        return false;
    }

    return true;
}

}  // namespace gpuav
