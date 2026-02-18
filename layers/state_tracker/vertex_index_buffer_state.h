/* Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (C) 2015-2026 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include <vulkan/vulkan_core.h>

namespace vvl {
class Buffer;
class DeviceState;

// Data for a given binding, that can be updated by calls like vkCmdBindVertexBuffers and vkCmdSetVertexInputEXT
struct VertexBufferBinding {
    // buffer might be VK_NULL_HANDLE because it was set or by default, we need to actually know if the command buffer binds or not
    bool bound;

    VkBuffer buffer;  // VK_NULL_HANDLE is valid if using nullDescriptor
    // Binding valid size: 0 if buffer is not tracked, actual size if VK_WHOLE_SIZE was specified,
    // clamped up to 0 if specified size is greater than the size actually left in the buffer
    VkDeviceSize effective_size;
    VkDeviceSize offset;
    VkDeviceSize stride;

    VertexBufferBinding() : bound(false), buffer(VK_NULL_HANDLE), effective_size(0), offset(0), stride(0) {}

    void reset() { *this = VertexBufferBinding(); }
};

class IndexBufferBinding {
  public:
    IndexBufferBinding() = default;  // reset state

    IndexBufferBinding(VkBuffer buffer_, VkDeviceSize size_, VkDeviceSize offset_, VkIndexType index_type_)
        : bound(true), size(size_), index_type(index_type_), buffer(buffer_), offset(offset_), address(0) {}
    IndexBufferBinding(VkDeviceAddress address_, VkDeviceSize size_, VkIndexType index_type_)
        : bound(true), size(size_), index_type(index_type_), buffer(VK_NULL_HANDLE), offset(0), address(address_) {}

    // buffer might be VK_NULL_HANDLE because it was set or by default, we need to actually know if the command buffer binds or not
    bool bound = false;

    VkDeviceSize size{0};

    VkIndexType index_type = static_cast<VkIndexType>(0);

    // Will look both in VkDeviceAddress and VkBuffer
    VkBuffer Handle(const vvl::DeviceState& device) const;

    bool HasNonNullBuffer() const { return buffer != VK_NULL_HANDLE || address != 0; }

    VkBuffer Buffer() const { return buffer; }
    VkDeviceSize BufferOffset() const { return offset; }

  private:
    // Only |buffer| or |address| can be non-null
    // Both can be null if using nullDescriptor
    VkBuffer buffer{VK_NULL_HANDLE};  // VK_NULL_HANDLE is valid if using maintenance6
    VkDeviceSize offset{0};           // only tied to the VkBuffer case

    VkDeviceAddress address{0};
};

}  // namespace vvl