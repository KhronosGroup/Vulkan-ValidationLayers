/***************************************************************************
 *
 * Copyright (c) 2023-2023 The Khronos Group Inc.
 * Copyright (c) 2023-2023 RasterGrid Kft.
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
 ****************************************************************************/

#include "layer_validation_tests.h"

#include <type_traits>

// Test that using a custom framework enables us to define our own main
// function and that the custom framework class is added as the base class
// of VkLayerTest
int main(int argc, char **argv) {
    // VkLayerTest must be a subclass of VkCustomRenderFramework
    static_assert(std::is_base_of<VkCustomRenderFramework, VkLayerTest>::value);

    // VkCustomRenderFramework must be a subclass of VkRenderFramework
    static_assert(std::is_base_of<VkRenderFramework, VkCustomRenderFramework>::value);

    return 0;
}
