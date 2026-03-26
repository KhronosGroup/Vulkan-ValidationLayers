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

#include <string>

namespace vvl {
class Buffer;
class DeviceState;

// Data for a given binding, that can be updated by calls like vkCmdBindVertexBuffers and vkCmdSetVertexInputEXT
class VertexBufferBinding {
  public:
    VertexBufferBinding() = default;  // reset state

    // We have these "Set" calls instead of a nice constructor because the API was messed up and the "stride" was in the pipeline,
    // but moved to vkCmdBindVertexBuffer2 as well in a "last one sets in win" fashion
    //
    // So if an app goes
    //   vkCmdBindPipeline (stride set here)
    //   vkCmdBindVertexBuffer (no stride)
    // so there is no "safe" default value to set here for a constructor
    void Set(VkBuffer buffer_, VkDeviceSize effective_size_, VkDeviceSize offset_, const VkDeviceSize* stride_) {
        bound = true;
        effective_size = effective_size_;
        if (stride_) {
            stride = *stride_;
        }
        buffer = buffer_;
        offset = offset_;
        address = 0;
    }
    void Set(VkDeviceAddress address_, VkDeviceSize effective_size_, const VkDeviceSize* stride_) {
        bound = true;
        effective_size = effective_size_;
        if (stride_) {
            stride = *stride_;
        }
        buffer = VK_NULL_HANDLE;
        offset = 0;
        address = address_;
    }

    // buffer might be VK_NULL_HANDLE because it was set or by default, we need to actually know if the command buffer binds or not
    bool bound{false};

    // Binding valid size: 0 if buffer is not tracked, actual size if VK_WHOLE_SIZE was specified,
    // clamped up to 0 if specified size is greater than the size actually left in the buffer
    VkDeviceSize effective_size{0};
    VkDeviceSize stride{0};

    // Will look both in VkDeviceAddress and VkBuffer
    VkBuffer Handle(const vvl::DeviceState& device) const;

    bool HasNonNullBuffer() const { return buffer != VK_NULL_HANDLE || address != 0; }

    VkBuffer Buffer() const { return buffer; }
    VkDeviceSize BufferOffset() const { return offset; }
    std::string String(const vvl::DeviceState& device) const;

  private:
    // Only |buffer| or |address| can be non-null
    // Both can be null if using nullDescriptor
    VkBuffer buffer{VK_NULL_HANDLE};  // VK_NULL_HANDLE is valid if using nullDescriptor
    VkDeviceSize offset{0};           // only tied to the VkBuffer case

    VkDeviceAddress address{0};
};

class IndexBufferBinding {
  public:
    IndexBufferBinding() = default;  // reset state

    IndexBufferBinding(VkBuffer buffer_, VkDeviceSize size_, VkDeviceSize offset_, VkIndexType index_type_)
        : bound(true), size(size_), index_type(index_type_), buffer(buffer_), offset(offset_), address(0) {}
    IndexBufferBinding(VkDeviceAddress address_, VkDeviceSize size_, VkIndexType index_type_)
        : bound(true), size(size_), index_type(index_type_), buffer(VK_NULL_HANDLE), offset(0), address(address_) {}

    // buffer might be VK_NULL_HANDLE because it was set or by default, we need to actually know if the command buffer binds or not
    bool bound{false};

    VkDeviceSize size{0};

    VkIndexType index_type = static_cast<VkIndexType>(0);

    // Will look both in VkDeviceAddress and VkBuffer
    VkBuffer Handle(const vvl::DeviceState& device) const;

    bool HasNonNullBuffer() const { return buffer != VK_NULL_HANDLE || address != 0; }

    VkBuffer Buffer() const { return buffer; }
    VkDeviceSize BufferOffset() const { return offset; }
    std::string String(const vvl::DeviceState& device) const;

  private:
    // Only |buffer| or |address| can be non-null
    // Both can be null if using nullDescriptor
    VkBuffer buffer{VK_NULL_HANDLE};  // VK_NULL_HANDLE is valid if using maintenance6
    VkDeviceSize offset{0};           // only tied to the VkBuffer case

    VkDeviceAddress address{0};
};

}  // namespace vvl