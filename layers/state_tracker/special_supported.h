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

// Validation requires "special" handling for some extensions/feature because the VU depends on if the feature is "supported" not
// "enabled". This struct/file is dedicatd to contain them in a single location, while also giving background for each.
struct SpecialSupported {
    // Adds flags that need to be queried and if the device supports the flags, we look for the app
    bool vk_khr_format_feature_flags2{false};  // VK_KHR_format_feature_flags2

    // VK_EXT_pipeline_robustness was designed to be a subset of robustness extensions
    // Enabling the other robustness features can reduce performance on GPU, so just the
    // support is needed to check
    bool robust_image_access{false};    // robustImageAccess (VK_EXT_image_robustness)
    bool robust_image_access2{false};   // robustImageAccess2 (VK_EXT_robustness2)
    bool robust_buffer_access2{false};  // robustBufferAccess2 (VK_EXT_robustness2)

    // There are various VK_EXT_descriptor_indexing limits that can be hit because when the feature is not supported, the property
    // limit will be zero see https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/7298
    bool descriptor_binding_sampled_image_uab{false};   // descriptorBindingSampledImageUpdateAfterBind
    bool descriptor_binding_uniform_buffer_uab{false};  // descriptorBindingUniformBufferUpdateAfterBind
    bool descriptor_binding_storage_buffer_uab{false};  // descriptorBindingStorageBufferUpdateAfterBind
    bool descriptor_binding_storage_image_uab{false};   // descriptorBindingStorageImageUpdateAfterBind
    // From VK_EXT_inline_uniform_block, but interacts with VK_EXT_descriptor_indexing
    bool descriptor_binding_inline_uniform_buffer_uab{false};  // descriptorBindingInlineUniformBlockUpdateAfterBind
};