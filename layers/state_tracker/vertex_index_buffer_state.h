/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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

#include "vulkan/vulkan.h"
#include <vector>
#include <memory>

namespace vvl {
class Buffer;
}  // namespace vvl

struct BufferBinding {
    std::shared_ptr<vvl::Buffer> buffer_state;
    VkDeviceSize size;
    VkDeviceSize offset;
    VkDeviceSize stride;

    BufferBinding() : buffer_state(), size(0), offset(0), stride(0) {}
    BufferBinding(const std::shared_ptr<vvl::Buffer> &buffer_state_, VkDeviceSize size_, VkDeviceSize offset_, VkDeviceSize stride_);
    BufferBinding(const std::shared_ptr<vvl::Buffer> &buffer_state_, VkDeviceSize offset_);
    virtual ~BufferBinding() {}

    virtual void reset() { *this = BufferBinding(); }
    bool bound() const;
};

struct IndexBufferBinding : BufferBinding {
    VkIndexType index_type;

    IndexBufferBinding() : BufferBinding(), index_type(static_cast<VkIndexType>(0)) {}
    IndexBufferBinding(const std::shared_ptr<vvl::Buffer> &buffer_state_, VkDeviceSize offset_, VkIndexType index_type_);
    // TODO - We could clean up the BufferBinding interface now we have 2 ways to bind both the Vertex and Index buffer
    IndexBufferBinding(const std::shared_ptr<vvl::Buffer> &buffer_state_, VkDeviceSize size_, VkDeviceSize offset_,
                       VkIndexType index_type_);
    virtual ~IndexBufferBinding() {}

    virtual void reset() override { *this = IndexBufferBinding(); }
};

struct CBVertexBufferBindingInfo {
    std::vector<BufferBinding> vertex_buffer_bindings;
};
