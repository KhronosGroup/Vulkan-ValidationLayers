/*
 * Copyright (c) 2023-2024 LunarG, Inc.
 * Copyright (c) 2023-2024 Valve Corporation
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
// clang-format off

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
    "VUID-VkMemoryToImageCopyEXT-pHostPointer-09061", "VUID-VkImageToMemoryCopyEXT-pHostPointer-09066"

    // these are already taken care in spirv-val for 08737
    "VUID-VkShaderModuleCreateInfo-pCode-08736", "VUID-VkShaderCreateInfoEXT-pCode-08736",
    "VUID-VkShaderModuleCreateInfo-pCode-08738", "VUID-VkShaderCreateInfoEXT-pCode-08738",

    // These implicit VUs ask to check for a valid structure that has no sType,
    // there is nothing that can actually be validated
    //
    // VkImageSubresourceLayers
    "VUID-VkImageBlit-dstSubresource-parameter",
    "VUID-VkImageBlit-srcSubresource-parameter",
    "VUID-VkImageBlit2-dstSubresource-parameter",
    "VUID-VkImageBlit2-srcSubresource-parameter",
    "VUID-VkImageCopy-dstSubresource-parameter",
    "VUID-VkImageCopy-srcSubresource-parameter",
    "VUID-VkImageCopy2-dstSubresource-parameter",
    "VUID-VkImageCopy2-srcSubresource-parameter",
    "VUID-VkImageResolve-dstSubresource-parameter",
    "VUID-VkImageResolve-srcSubresource-parameter",
    "VUID-VkImageResolve2-dstSubresource-parameter",
    "VUID-VkImageResolve2-srcSubresource-parameter",
    "VUID-VkBufferImageCopy-imageSubresource-parameter",
    "VUID-VkBufferImageCopy2-imageSubresource-parameter",
    "VUID-VkMemoryToImageCopyEXT-imageSubresource-parameter",
    "VUID-VkImageToMemoryCopyEXT-imageSubresource-parameter",
    "VUID-VkCopyMemoryToImageIndirectCommandNV-imageSubresource-parameter",
    // VkImageSubresourceRange
    "VUID-VkImageMemoryBarrier-subresourceRange-parameter",
    "VUID-VkImageMemoryBarrier2-subresourceRange-parameter",
    "VUID-VkHostImageLayoutTransitionInfoEXT-subresourceRange-parameter",
    "VUID-VkImageViewCreateInfo-subresourceRange-parameter",
    // VkImageSubresource
    "VUID-VkImageSubresource2KHR-imageSubresource-parameter",
    "VUID-VkSparseImageMemoryBind-subresource-parameter",
    // VkStencilOpState
    "VUID-VkPipelineDepthStencilStateCreateInfo-front-parameter",
    "VUID-VkPipelineDepthStencilStateCreateInfo-back-parameter",
    // VkClearValue
    "VUID-VkRenderingAttachmentInfo-clearValue-parameter",

    // When:
    //   Struct A has a pointer field to Struct B
    //   Struct B has a non-pointer field to Struct C
    // you get a situation where Struct B has a VU that is not hit because we validate it in Struct C
    "VUID-VkAttachmentSampleLocationsEXT-sampleLocationsInfo-parameter",              // VUID-VkSampleLocationsInfoEXT-sType-sType
    "VUID-VkSubpassSampleLocationsEXT-sampleLocationsInfo-parameter",                 // VUID-VkSampleLocationsInfoEXT-sType-sType
    "VUID-VkPipelineSampleLocationsStateCreateInfoEXT-sampleLocationsInfo-parameter", // VUID-VkSampleLocationsInfoEXT-sType-sType
    "VUID-VkComputePipelineCreateInfo-stage-parameter", // VUID-VkPipelineShaderStageCreateInfo-sType-sType

    // These VUs have "is not NULL it must be a pointer to a valid pointer to valid structure" language
    // There is no actual way to validate thsese
    // https://gitlab.khronos.org/vulkan/vulkan/-/issues/3718
    "VUID-VkDescriptorGetInfoEXT-pUniformTexelBuffer-parameter",
    "VUID-VkDescriptorGetInfoEXT-pStorageTexelBuffer-parameter",
    "VUID-VkDescriptorGetInfoEXT-pUniformBuffer-parameter",
    "VUID-VkDescriptorGetInfoEXT-pStorageBuffer-parameter",
};

// clang-format on