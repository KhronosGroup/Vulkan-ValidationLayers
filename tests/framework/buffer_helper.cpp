/*
 * Copyright (c) 2015-2016, 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2016, 2020-2024 Valve Corporation
 * Copyright (c) 2015-2016, 2020-2024 LunarG, Inc.
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

#include "buffer_helper.h"

namespace vkt {

Buffer VertexBuffer(const Device &dev, const std::vector<float> &vertices) {
    vkt::Buffer vertex_buffer(dev, vertices.size() * sizeof(float), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    auto *vertex_buffer_ptr = static_cast<float *>(vertex_buffer.memory().map());
    std::copy(vertices.data(), vertices.data() + vertices.size(), vertex_buffer_ptr);
    vertex_buffer.memory().unmap();
    return vertex_buffer;
}
}  // namespace vkt
