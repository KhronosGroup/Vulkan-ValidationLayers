/*
 * Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
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

#include "binding.h"

namespace vkt {

template <typename VertexT>
Buffer VertexBuffer(const Device &dev, const std::vector<float> &vertices) {
    vkt::Buffer vertex_buffer(dev, vertices.size() * sizeof(VertexT), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    auto *vertex_buffer_ptr = static_cast<VertexT *>(vertex_buffer.memory().map());
    std::copy(vertices.data(), vertices.data() + vertices.size(), vertex_buffer_ptr);
    vertex_buffer.memory().unmap();
    return vertex_buffer;
}

template <typename IndexT>
Buffer IndexBuffer(const Device &dev, const std::vector<IndexT> &indices) {
    vkt::Buffer index_buffer(dev, indices.size() * sizeof(IndexT), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    auto *index_buffer_ptr = static_cast<IndexT *>(index_buffer.memory().map());
    std::copy(indices.data(), indices.data() + indices.size(), index_buffer_ptr);
    index_buffer.memory().unmap();
    return index_buffer;
}

}  // namespace vkt