/*
 * Copyright (c) 2023 LunarG, Inc.
 * Copyright (c) 2023 Valve Corporation
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

// This file list all VUID that are not possible to validate.
// This file should never be included, but here for searchability and statistics

const char* unimplementable_validation[] = {
    // sparseAddressSpaceSize can't be tracked in a layer
    // https://gitlab.khronos.org/vulkan/vulkan/-/issues/2403
    "VUID-vkCreateBuffer-flags-00911",

    // Some of the early extensions were not created with a feature bit. This means if the extension is used, we considered it
    // "enabled". This becomes a problem as some coniditional VUIDs depend on the Extension to be enabled, this means we are left
    // with 2 variations of the VUIDs, but only one is not possible to ever get to.
    // The following are a list of these:
    "VUID-VkSubpassDescription2-multisampledRenderToSingleSampled-06869",  // VUID-VkSubpassDescription2-multisampledRenderToSingleSampled-06872

    // This VUID cannot be validated at vkCmdEndDebugUtilsLabelEXT time. Needs spec clarification.
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5671
    "VUID-vkCmdEndDebugUtilsLabelEXT-commandBuffer-01912",

    // These VUIDs cannot be validated beyond making sure the pointer is not null
    "VUID-VkMemoryToImageCopyEXT-pHostPointer-09061", "VUID-VkImageToMemoryCopyEXT-pHostPointer-09066"};
