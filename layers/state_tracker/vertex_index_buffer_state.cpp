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

#include "state_tracker/vertex_index_buffer_state.h"
#include "state_tracker/buffer_state.h"

BufferBinding::BufferBinding(const std::shared_ptr<vvl::Buffer> &buffer_state_, VkDeviceSize size_, VkDeviceSize offset_, VkDeviceSize stride_)
        : buffer_state(buffer_state_), size(size_), offset(offset_), stride(stride_) {}
BufferBinding::BufferBinding(const std::shared_ptr<vvl::Buffer> &buffer_state_, VkDeviceSize offset_)
        : BufferBinding(buffer_state_, vvl::Buffer::ComputeSize(buffer_state_, offset_, VK_WHOLE_SIZE), offset_, 0U) {}

bool BufferBinding::bound() const { return buffer_state && !buffer_state->Destroyed(); }

IndexBufferBinding::IndexBufferBinding(const std::shared_ptr<vvl::Buffer> &buffer_state_, VkDeviceSize offset_, VkIndexType index_type_)
        : BufferBinding(buffer_state_, offset_), index_type(index_type_) {}

IndexBufferBinding::IndexBufferBinding(const std::shared_ptr<vvl::Buffer> &buffer_state_, VkDeviceSize size_, VkDeviceSize offset_,
                       VkIndexType index_type_) : BufferBinding(buffer_state_, vvl::Buffer::ComputeSize(buffer_state_, offset_, size_), offset_, 0U),
          index_type(index_type_) {}