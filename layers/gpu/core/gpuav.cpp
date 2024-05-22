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
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
#include <unistd.h>
#endif

namespace gpuav {

VkDeviceAddress Validator::GetBufferDeviceAddress(VkBuffer buffer, const Location &loc) const {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8001
    // Setting enabled_features.bufferDeviceAddress to true in GpuShaderInstrumentor::PreCallRecordCreateDevice
    // when adding missing features will modify another validator object, one associated to VkInstance,
    // and "this" validator is associated to a device. enabled_features is not inherited, and besides
    // would be reset in GetEnabledDeviceFeatures.
    // The switch from the instance validator object to the device one happens in
    // `state_tracker.cpp`, `ValidationStateTracker::PostCallRecordCreateDevice`
    // TL;DR is the following type of sanity check is currently invalid, but it would be nice to have
    // assert(enabled_features.bufferDeviceAddress);

    VkBufferDeviceAddressInfo address_info = vku::InitStructHelper();
    address_info.buffer = buffer;
    if (api_version >= VK_API_VERSION_1_2) {
        return DispatchGetBufferDeviceAddress(device, &address_info);
    }
    if (IsExtEnabled(device_extensions.vk_ext_buffer_device_address)) {
        return DispatchGetBufferDeviceAddressEXT(device, &address_info);
    }
    if (IsExtEnabled(device_extensions.vk_khr_buffer_device_address)) {
        return DispatchGetBufferDeviceAddressKHR(device, &address_info);
    }
    return 0;
}

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
        InternalError(device, loc, "Unable to allocate device memory for error output buffer. Device could become unstable.", true);
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
        InternalError(device, loc, "Unable to map device memory allocated for error output buffer. Device could become unstable.",
                      true);
        return false;
    }

    return true;
}

void Validator::StoreCommandResources(const VkCommandBuffer cmd_buffer, std::unique_ptr<CommandResources> command_resources,
                                      const Location &loc) {
    if (!command_resources) return;

    auto cb_node = GetWrite<CommandBuffer>(cmd_buffer);
    if (!cb_node) {
        InternalError(cmd_buffer, loc, "Unrecognized command buffer");
        return;
    }

    cb_node->per_command_resources.emplace_back(std::move(command_resources));
}

}  // namespace gpuav
