// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See helper_file_generator.py for modifications


/***************************************************************************
 *
 * Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (c) 2015-2021 Google Inc.
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
 *
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisforbes@google.com>
 * Author: John Zulauf<jzulauf@lunarg.com>
 *
 ****************************************************************************/

#pragma once
#include <vulkan/vulkan_sc.h>
// These empty generic templates are specialized for each type with sType
// members and for each sType -- providing a two way map between structure
// types and sTypes

template <VkStructureType id> struct LvlSTypeMap {};
template <typename T> struct LvlTypeMap {};

// Map type VkBufferMemoryBarrier to id VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER
template <> struct LvlTypeMap<VkBufferMemoryBarrier> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER> {
    typedef VkBufferMemoryBarrier Type;
};

// Map type VkImageMemoryBarrier to id VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER
template <> struct LvlTypeMap<VkImageMemoryBarrier> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER> {
    typedef VkImageMemoryBarrier Type;
};

// Map type VkMemoryBarrier to id VK_STRUCTURE_TYPE_MEMORY_BARRIER
template <> struct LvlTypeMap<VkMemoryBarrier> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_BARRIER> {
    typedef VkMemoryBarrier Type;
};

// Map type VkApplicationInfo to id VK_STRUCTURE_TYPE_APPLICATION_INFO
template <> struct LvlTypeMap<VkApplicationInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_APPLICATION_INFO> {
    typedef VkApplicationInfo Type;
};

// Map type VkInstanceCreateInfo to id VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO
template <> struct LvlTypeMap<VkInstanceCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO> {
    typedef VkInstanceCreateInfo Type;
};

// Map type VkDeviceQueueCreateInfo to id VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO
template <> struct LvlTypeMap<VkDeviceQueueCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO> {
    typedef VkDeviceQueueCreateInfo Type;
};

// Map type VkDeviceCreateInfo to id VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO
template <> struct LvlTypeMap<VkDeviceCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO> {
    typedef VkDeviceCreateInfo Type;
};

// Map type VkSubmitInfo to id VK_STRUCTURE_TYPE_SUBMIT_INFO
template <> struct LvlTypeMap<VkSubmitInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SUBMIT_INFO> {
    typedef VkSubmitInfo Type;
};

// Map type VkMappedMemoryRange to id VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE
template <> struct LvlTypeMap<VkMappedMemoryRange> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE> {
    typedef VkMappedMemoryRange Type;
};

// Map type VkMemoryAllocateInfo to id VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO
template <> struct LvlTypeMap<VkMemoryAllocateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO> {
    typedef VkMemoryAllocateInfo Type;
};

// Map type VkFenceCreateInfo to id VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
template <> struct LvlTypeMap<VkFenceCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FENCE_CREATE_INFO> {
    typedef VkFenceCreateInfo Type;
};

// Map type VkSemaphoreCreateInfo to id VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
template <> struct LvlTypeMap<VkSemaphoreCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO> {
    typedef VkSemaphoreCreateInfo Type;
};

// Map type VkEventCreateInfo to id VK_STRUCTURE_TYPE_EVENT_CREATE_INFO
template <> struct LvlTypeMap<VkEventCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EVENT_CREATE_INFO> {
    typedef VkEventCreateInfo Type;
};

// Map type VkQueryPoolCreateInfo to id VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO
template <> struct LvlTypeMap<VkQueryPoolCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO> {
    typedef VkQueryPoolCreateInfo Type;
};

// Map type VkBufferCreateInfo to id VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO
template <> struct LvlTypeMap<VkBufferCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO> {
    typedef VkBufferCreateInfo Type;
};

// Map type VkBufferViewCreateInfo to id VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO
template <> struct LvlTypeMap<VkBufferViewCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO> {
    typedef VkBufferViewCreateInfo Type;
};

// Map type VkImageCreateInfo to id VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO
template <> struct LvlTypeMap<VkImageCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO> {
    typedef VkImageCreateInfo Type;
};

// Map type VkImageViewCreateInfo to id VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO
template <> struct LvlTypeMap<VkImageViewCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO> {
    typedef VkImageViewCreateInfo Type;
};

// Map type VkPipelineCacheCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineCacheCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO> {
    typedef VkPipelineCacheCreateInfo Type;
};

// Map type VkPipelineShaderStageCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineShaderStageCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO> {
    typedef VkPipelineShaderStageCreateInfo Type;
};

// Map type VkComputePipelineCreateInfo to id VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO
template <> struct LvlTypeMap<VkComputePipelineCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO> {
    typedef VkComputePipelineCreateInfo Type;
};

// Map type VkPipelineVertexInputStateCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineVertexInputStateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO> {
    typedef VkPipelineVertexInputStateCreateInfo Type;
};

// Map type VkPipelineInputAssemblyStateCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineInputAssemblyStateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO> {
    typedef VkPipelineInputAssemblyStateCreateInfo Type;
};

// Map type VkPipelineTessellationStateCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineTessellationStateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO> {
    typedef VkPipelineTessellationStateCreateInfo Type;
};

// Map type VkPipelineViewportStateCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineViewportStateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO> {
    typedef VkPipelineViewportStateCreateInfo Type;
};

// Map type VkPipelineRasterizationStateCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineRasterizationStateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO> {
    typedef VkPipelineRasterizationStateCreateInfo Type;
};

// Map type VkPipelineMultisampleStateCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineMultisampleStateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO> {
    typedef VkPipelineMultisampleStateCreateInfo Type;
};

// Map type VkPipelineDepthStencilStateCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineDepthStencilStateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO> {
    typedef VkPipelineDepthStencilStateCreateInfo Type;
};

// Map type VkPipelineColorBlendStateCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineColorBlendStateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO> {
    typedef VkPipelineColorBlendStateCreateInfo Type;
};

// Map type VkPipelineDynamicStateCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineDynamicStateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO> {
    typedef VkPipelineDynamicStateCreateInfo Type;
};

// Map type VkGraphicsPipelineCreateInfo to id VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO
template <> struct LvlTypeMap<VkGraphicsPipelineCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO> {
    typedef VkGraphicsPipelineCreateInfo Type;
};

// Map type VkPipelineLayoutCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineLayoutCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO> {
    typedef VkPipelineLayoutCreateInfo Type;
};

// Map type VkSamplerCreateInfo to id VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO
template <> struct LvlTypeMap<VkSamplerCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO> {
    typedef VkSamplerCreateInfo Type;
};

// Map type VkCopyDescriptorSet to id VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET
template <> struct LvlTypeMap<VkCopyDescriptorSet> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET> {
    typedef VkCopyDescriptorSet Type;
};

// Map type VkDescriptorPoolCreateInfo to id VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO
template <> struct LvlTypeMap<VkDescriptorPoolCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO> {
    typedef VkDescriptorPoolCreateInfo Type;
};

// Map type VkDescriptorSetAllocateInfo to id VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO
template <> struct LvlTypeMap<VkDescriptorSetAllocateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO> {
    typedef VkDescriptorSetAllocateInfo Type;
};

// Map type VkDescriptorSetLayoutCreateInfo to id VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
template <> struct LvlTypeMap<VkDescriptorSetLayoutCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO> {
    typedef VkDescriptorSetLayoutCreateInfo Type;
};

// Map type VkWriteDescriptorSet to id VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET
template <> struct LvlTypeMap<VkWriteDescriptorSet> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET> {
    typedef VkWriteDescriptorSet Type;
};

// Map type VkFramebufferCreateInfo to id VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO
template <> struct LvlTypeMap<VkFramebufferCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO> {
    typedef VkFramebufferCreateInfo Type;
};

// Map type VkRenderPassCreateInfo to id VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO
template <> struct LvlTypeMap<VkRenderPassCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO> {
    typedef VkRenderPassCreateInfo Type;
};

// Map type VkCommandPoolCreateInfo to id VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO
template <> struct LvlTypeMap<VkCommandPoolCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO> {
    typedef VkCommandPoolCreateInfo Type;
};

// Map type VkCommandBufferAllocateInfo to id VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO
template <> struct LvlTypeMap<VkCommandBufferAllocateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO> {
    typedef VkCommandBufferAllocateInfo Type;
};

// Map type VkCommandBufferInheritanceInfo to id VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO
template <> struct LvlTypeMap<VkCommandBufferInheritanceInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO> {
    typedef VkCommandBufferInheritanceInfo Type;
};

// Map type VkCommandBufferBeginInfo to id VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
template <> struct LvlTypeMap<VkCommandBufferBeginInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO> {
    typedef VkCommandBufferBeginInfo Type;
};

// Map type VkRenderPassBeginInfo to id VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO
template <> struct LvlTypeMap<VkRenderPassBeginInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO> {
    typedef VkRenderPassBeginInfo Type;
};

// Map type VkPhysicalDeviceSubgroupProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceSubgroupProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES> {
    typedef VkPhysicalDeviceSubgroupProperties Type;
};

// Map type VkBindBufferMemoryInfo to id VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO
template <> struct LvlTypeMap<VkBindBufferMemoryInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO> {
    typedef VkBindBufferMemoryInfo Type;
};

// Map type VkBindImageMemoryInfo to id VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO
template <> struct LvlTypeMap<VkBindImageMemoryInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO> {
    typedef VkBindImageMemoryInfo Type;
};

// Map type VkPhysicalDevice16BitStorageFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES
template <> struct LvlTypeMap<VkPhysicalDevice16BitStorageFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES> {
    typedef VkPhysicalDevice16BitStorageFeatures Type;
};

// Map type VkMemoryDedicatedRequirements to id VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS
template <> struct LvlTypeMap<VkMemoryDedicatedRequirements> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS> {
    typedef VkMemoryDedicatedRequirements Type;
};

// Map type VkMemoryDedicatedAllocateInfo to id VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO
template <> struct LvlTypeMap<VkMemoryDedicatedAllocateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO> {
    typedef VkMemoryDedicatedAllocateInfo Type;
};

// Map type VkMemoryAllocateFlagsInfo to id VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO
template <> struct LvlTypeMap<VkMemoryAllocateFlagsInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO> {
    typedef VkMemoryAllocateFlagsInfo Type;
};

// Map type VkDeviceGroupRenderPassBeginInfo to id VK_STRUCTURE_TYPE_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO
template <> struct LvlTypeMap<VkDeviceGroupRenderPassBeginInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO> {
    typedef VkDeviceGroupRenderPassBeginInfo Type;
};

// Map type VkDeviceGroupCommandBufferBeginInfo to id VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO
template <> struct LvlTypeMap<VkDeviceGroupCommandBufferBeginInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO> {
    typedef VkDeviceGroupCommandBufferBeginInfo Type;
};

// Map type VkDeviceGroupSubmitInfo to id VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO
template <> struct LvlTypeMap<VkDeviceGroupSubmitInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO> {
    typedef VkDeviceGroupSubmitInfo Type;
};

// Map type VkBindBufferMemoryDeviceGroupInfo to id VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO
template <> struct LvlTypeMap<VkBindBufferMemoryDeviceGroupInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO> {
    typedef VkBindBufferMemoryDeviceGroupInfo Type;
};

// Map type VkBindImageMemoryDeviceGroupInfo to id VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO
template <> struct LvlTypeMap<VkBindImageMemoryDeviceGroupInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO> {
    typedef VkBindImageMemoryDeviceGroupInfo Type;
};

// Map type VkPhysicalDeviceGroupProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceGroupProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES> {
    typedef VkPhysicalDeviceGroupProperties Type;
};

// Map type VkDeviceGroupDeviceCreateInfo to id VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO
template <> struct LvlTypeMap<VkDeviceGroupDeviceCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO> {
    typedef VkDeviceGroupDeviceCreateInfo Type;
};

// Map type VkBufferMemoryRequirementsInfo2 to id VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2
template <> struct LvlTypeMap<VkBufferMemoryRequirementsInfo2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2> {
    typedef VkBufferMemoryRequirementsInfo2 Type;
};

// Map type VkImageMemoryRequirementsInfo2 to id VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2
template <> struct LvlTypeMap<VkImageMemoryRequirementsInfo2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2> {
    typedef VkImageMemoryRequirementsInfo2 Type;
};

// Map type VkMemoryRequirements2 to id VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2
template <> struct LvlTypeMap<VkMemoryRequirements2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2> {
    typedef VkMemoryRequirements2 Type;
};

// Map type VkPhysicalDeviceFeatures2 to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2
template <> struct LvlTypeMap<VkPhysicalDeviceFeatures2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2> {
    typedef VkPhysicalDeviceFeatures2 Type;
};

// Map type VkPhysicalDeviceProperties2 to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2
template <> struct LvlTypeMap<VkPhysicalDeviceProperties2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2> {
    typedef VkPhysicalDeviceProperties2 Type;
};

// Map type VkFormatProperties2 to id VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2
template <> struct LvlTypeMap<VkFormatProperties2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2> {
    typedef VkFormatProperties2 Type;
};

// Map type VkImageFormatProperties2 to id VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2
template <> struct LvlTypeMap<VkImageFormatProperties2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2> {
    typedef VkImageFormatProperties2 Type;
};

// Map type VkPhysicalDeviceImageFormatInfo2 to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2
template <> struct LvlTypeMap<VkPhysicalDeviceImageFormatInfo2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2> {
    typedef VkPhysicalDeviceImageFormatInfo2 Type;
};

// Map type VkQueueFamilyProperties2 to id VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2
template <> struct LvlTypeMap<VkQueueFamilyProperties2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2> {
    typedef VkQueueFamilyProperties2 Type;
};

// Map type VkPhysicalDeviceMemoryProperties2 to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2
template <> struct LvlTypeMap<VkPhysicalDeviceMemoryProperties2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2> {
    typedef VkPhysicalDeviceMemoryProperties2 Type;
};

// Map type VkPhysicalDevicePointClippingProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDevicePointClippingProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES> {
    typedef VkPhysicalDevicePointClippingProperties Type;
};

// Map type VkRenderPassInputAttachmentAspectCreateInfo to id VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO
template <> struct LvlTypeMap<VkRenderPassInputAttachmentAspectCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO> {
    typedef VkRenderPassInputAttachmentAspectCreateInfo Type;
};

// Map type VkImageViewUsageCreateInfo to id VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO
template <> struct LvlTypeMap<VkImageViewUsageCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO> {
    typedef VkImageViewUsageCreateInfo Type;
};

// Map type VkPipelineTessellationDomainOriginStateCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineTessellationDomainOriginStateCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO> {
    typedef VkPipelineTessellationDomainOriginStateCreateInfo Type;
};

// Map type VkRenderPassMultiviewCreateInfo to id VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO
template <> struct LvlTypeMap<VkRenderPassMultiviewCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO> {
    typedef VkRenderPassMultiviewCreateInfo Type;
};

// Map type VkPhysicalDeviceMultiviewFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceMultiviewFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES> {
    typedef VkPhysicalDeviceMultiviewFeatures Type;
};

// Map type VkPhysicalDeviceMultiviewProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceMultiviewProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES> {
    typedef VkPhysicalDeviceMultiviewProperties Type;
};

// Map type VkPhysicalDeviceVariablePointersFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceVariablePointersFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES> {
    typedef VkPhysicalDeviceVariablePointersFeatures Type;
};

// Map type VkPhysicalDeviceProtectedMemoryFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceProtectedMemoryFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES> {
    typedef VkPhysicalDeviceProtectedMemoryFeatures Type;
};

// Map type VkPhysicalDeviceProtectedMemoryProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceProtectedMemoryProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES> {
    typedef VkPhysicalDeviceProtectedMemoryProperties Type;
};

// Map type VkDeviceQueueInfo2 to id VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2
template <> struct LvlTypeMap<VkDeviceQueueInfo2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2> {
    typedef VkDeviceQueueInfo2 Type;
};

// Map type VkProtectedSubmitInfo to id VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO
template <> struct LvlTypeMap<VkProtectedSubmitInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO> {
    typedef VkProtectedSubmitInfo Type;
};

// Map type VkSamplerYcbcrConversionCreateInfo to id VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO
template <> struct LvlTypeMap<VkSamplerYcbcrConversionCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO> {
    typedef VkSamplerYcbcrConversionCreateInfo Type;
};

// Map type VkSamplerYcbcrConversionInfo to id VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO
template <> struct LvlTypeMap<VkSamplerYcbcrConversionInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO> {
    typedef VkSamplerYcbcrConversionInfo Type;
};

// Map type VkBindImagePlaneMemoryInfo to id VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO
template <> struct LvlTypeMap<VkBindImagePlaneMemoryInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO> {
    typedef VkBindImagePlaneMemoryInfo Type;
};

// Map type VkImagePlaneMemoryRequirementsInfo to id VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO
template <> struct LvlTypeMap<VkImagePlaneMemoryRequirementsInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO> {
    typedef VkImagePlaneMemoryRequirementsInfo Type;
};

// Map type VkPhysicalDeviceSamplerYcbcrConversionFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceSamplerYcbcrConversionFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES> {
    typedef VkPhysicalDeviceSamplerYcbcrConversionFeatures Type;
};

// Map type VkSamplerYcbcrConversionImageFormatProperties to id VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES
template <> struct LvlTypeMap<VkSamplerYcbcrConversionImageFormatProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES> {
    typedef VkSamplerYcbcrConversionImageFormatProperties Type;
};

// Map type VkPhysicalDeviceExternalImageFormatInfo to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO
template <> struct LvlTypeMap<VkPhysicalDeviceExternalImageFormatInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO> {
    typedef VkPhysicalDeviceExternalImageFormatInfo Type;
};

// Map type VkExternalImageFormatProperties to id VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES
template <> struct LvlTypeMap<VkExternalImageFormatProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES> {
    typedef VkExternalImageFormatProperties Type;
};

// Map type VkPhysicalDeviceExternalBufferInfo to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO
template <> struct LvlTypeMap<VkPhysicalDeviceExternalBufferInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO> {
    typedef VkPhysicalDeviceExternalBufferInfo Type;
};

// Map type VkExternalBufferProperties to id VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES
template <> struct LvlTypeMap<VkExternalBufferProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES> {
    typedef VkExternalBufferProperties Type;
};

// Map type VkPhysicalDeviceIDProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceIDProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES> {
    typedef VkPhysicalDeviceIDProperties Type;
};

// Map type VkExternalMemoryImageCreateInfo to id VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO
template <> struct LvlTypeMap<VkExternalMemoryImageCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO> {
    typedef VkExternalMemoryImageCreateInfo Type;
};

// Map type VkExternalMemoryBufferCreateInfo to id VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO
template <> struct LvlTypeMap<VkExternalMemoryBufferCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO> {
    typedef VkExternalMemoryBufferCreateInfo Type;
};

// Map type VkExportMemoryAllocateInfo to id VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO
template <> struct LvlTypeMap<VkExportMemoryAllocateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO> {
    typedef VkExportMemoryAllocateInfo Type;
};

// Map type VkPhysicalDeviceExternalFenceInfo to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO
template <> struct LvlTypeMap<VkPhysicalDeviceExternalFenceInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO> {
    typedef VkPhysicalDeviceExternalFenceInfo Type;
};

// Map type VkExternalFenceProperties to id VK_STRUCTURE_TYPE_EXTERNAL_FENCE_PROPERTIES
template <> struct LvlTypeMap<VkExternalFenceProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXTERNAL_FENCE_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXTERNAL_FENCE_PROPERTIES> {
    typedef VkExternalFenceProperties Type;
};

// Map type VkExportFenceCreateInfo to id VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO
template <> struct LvlTypeMap<VkExportFenceCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO> {
    typedef VkExportFenceCreateInfo Type;
};

// Map type VkExportSemaphoreCreateInfo to id VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO
template <> struct LvlTypeMap<VkExportSemaphoreCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO> {
    typedef VkExportSemaphoreCreateInfo Type;
};

// Map type VkPhysicalDeviceExternalSemaphoreInfo to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO
template <> struct LvlTypeMap<VkPhysicalDeviceExternalSemaphoreInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO> {
    typedef VkPhysicalDeviceExternalSemaphoreInfo Type;
};

// Map type VkExternalSemaphoreProperties to id VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES
template <> struct LvlTypeMap<VkExternalSemaphoreProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES> {
    typedef VkExternalSemaphoreProperties Type;
};

// Map type VkPhysicalDeviceMaintenance3Properties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceMaintenance3Properties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES> {
    typedef VkPhysicalDeviceMaintenance3Properties Type;
};

// Map type VkDescriptorSetLayoutSupport to id VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_SUPPORT
template <> struct LvlTypeMap<VkDescriptorSetLayoutSupport> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_SUPPORT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_SUPPORT> {
    typedef VkDescriptorSetLayoutSupport Type;
};

// Map type VkPhysicalDeviceShaderDrawParametersFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceShaderDrawParametersFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES> {
    typedef VkPhysicalDeviceShaderDrawParametersFeatures Type;
};

// Map type VkPhysicalDeviceVulkan11Features to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceVulkan11Features> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES> {
    typedef VkPhysicalDeviceVulkan11Features Type;
};

// Map type VkPhysicalDeviceVulkan11Properties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceVulkan11Properties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES> {
    typedef VkPhysicalDeviceVulkan11Properties Type;
};

// Map type VkPhysicalDeviceVulkan12Features to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceVulkan12Features> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES> {
    typedef VkPhysicalDeviceVulkan12Features Type;
};

// Map type VkPhysicalDeviceVulkan12Properties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceVulkan12Properties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES> {
    typedef VkPhysicalDeviceVulkan12Properties Type;
};

// Map type VkImageFormatListCreateInfo to id VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO
template <> struct LvlTypeMap<VkImageFormatListCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO> {
    typedef VkImageFormatListCreateInfo Type;
};

// Map type VkAttachmentDescription2 to id VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2
template <> struct LvlTypeMap<VkAttachmentDescription2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2> {
    typedef VkAttachmentDescription2 Type;
};

// Map type VkAttachmentReference2 to id VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2
template <> struct LvlTypeMap<VkAttachmentReference2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2> {
    typedef VkAttachmentReference2 Type;
};

// Map type VkSubpassDescription2 to id VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2
template <> struct LvlTypeMap<VkSubpassDescription2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2> {
    typedef VkSubpassDescription2 Type;
};

// Map type VkSubpassDependency2 to id VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2
template <> struct LvlTypeMap<VkSubpassDependency2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2> {
    typedef VkSubpassDependency2 Type;
};

// Map type VkRenderPassCreateInfo2 to id VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2
template <> struct LvlTypeMap<VkRenderPassCreateInfo2> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2> {
    typedef VkRenderPassCreateInfo2 Type;
};

// Map type VkSubpassBeginInfo to id VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO
template <> struct LvlTypeMap<VkSubpassBeginInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO> {
    typedef VkSubpassBeginInfo Type;
};

// Map type VkSubpassEndInfo to id VK_STRUCTURE_TYPE_SUBPASS_END_INFO
template <> struct LvlTypeMap<VkSubpassEndInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SUBPASS_END_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SUBPASS_END_INFO> {
    typedef VkSubpassEndInfo Type;
};

// Map type VkPhysicalDevice8BitStorageFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES
template <> struct LvlTypeMap<VkPhysicalDevice8BitStorageFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES> {
    typedef VkPhysicalDevice8BitStorageFeatures Type;
};

// Map type VkPhysicalDeviceDriverProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceDriverProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES> {
    typedef VkPhysicalDeviceDriverProperties Type;
};

// Map type VkPhysicalDeviceShaderAtomicInt64Features to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceShaderAtomicInt64Features> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES> {
    typedef VkPhysicalDeviceShaderAtomicInt64Features Type;
};

// Map type VkPhysicalDeviceShaderFloat16Int8Features to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceShaderFloat16Int8Features> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES> {
    typedef VkPhysicalDeviceShaderFloat16Int8Features Type;
};

// Map type VkPhysicalDeviceFloatControlsProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceFloatControlsProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES> {
    typedef VkPhysicalDeviceFloatControlsProperties Type;
};

// Map type VkDescriptorSetLayoutBindingFlagsCreateInfo to id VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO
template <> struct LvlTypeMap<VkDescriptorSetLayoutBindingFlagsCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO> {
    typedef VkDescriptorSetLayoutBindingFlagsCreateInfo Type;
};

// Map type VkPhysicalDeviceDescriptorIndexingFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceDescriptorIndexingFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES> {
    typedef VkPhysicalDeviceDescriptorIndexingFeatures Type;
};

// Map type VkPhysicalDeviceDescriptorIndexingProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceDescriptorIndexingProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES> {
    typedef VkPhysicalDeviceDescriptorIndexingProperties Type;
};

// Map type VkDescriptorSetVariableDescriptorCountAllocateInfo to id VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO
template <> struct LvlTypeMap<VkDescriptorSetVariableDescriptorCountAllocateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO> {
    typedef VkDescriptorSetVariableDescriptorCountAllocateInfo Type;
};

// Map type VkDescriptorSetVariableDescriptorCountLayoutSupport to id VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT
template <> struct LvlTypeMap<VkDescriptorSetVariableDescriptorCountLayoutSupport> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT> {
    typedef VkDescriptorSetVariableDescriptorCountLayoutSupport Type;
};

// Map type VkSubpassDescriptionDepthStencilResolve to id VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE
template <> struct LvlTypeMap<VkSubpassDescriptionDepthStencilResolve> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE> {
    typedef VkSubpassDescriptionDepthStencilResolve Type;
};

// Map type VkPhysicalDeviceDepthStencilResolveProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceDepthStencilResolveProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES> {
    typedef VkPhysicalDeviceDepthStencilResolveProperties Type;
};

// Map type VkPhysicalDeviceScalarBlockLayoutFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceScalarBlockLayoutFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES> {
    typedef VkPhysicalDeviceScalarBlockLayoutFeatures Type;
};

// Map type VkImageStencilUsageCreateInfo to id VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO
template <> struct LvlTypeMap<VkImageStencilUsageCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO> {
    typedef VkImageStencilUsageCreateInfo Type;
};

// Map type VkSamplerReductionModeCreateInfo to id VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO
template <> struct LvlTypeMap<VkSamplerReductionModeCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO> {
    typedef VkSamplerReductionModeCreateInfo Type;
};

// Map type VkPhysicalDeviceSamplerFilterMinmaxProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceSamplerFilterMinmaxProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES> {
    typedef VkPhysicalDeviceSamplerFilterMinmaxProperties Type;
};

// Map type VkPhysicalDeviceVulkanMemoryModelFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceVulkanMemoryModelFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES> {
    typedef VkPhysicalDeviceVulkanMemoryModelFeatures Type;
};

// Map type VkPhysicalDeviceImagelessFramebufferFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceImagelessFramebufferFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES> {
    typedef VkPhysicalDeviceImagelessFramebufferFeatures Type;
};

// Map type VkFramebufferAttachmentImageInfo to id VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO
template <> struct LvlTypeMap<VkFramebufferAttachmentImageInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO> {
    typedef VkFramebufferAttachmentImageInfo Type;
};

// Map type VkFramebufferAttachmentsCreateInfo to id VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO
template <> struct LvlTypeMap<VkFramebufferAttachmentsCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO> {
    typedef VkFramebufferAttachmentsCreateInfo Type;
};

// Map type VkRenderPassAttachmentBeginInfo to id VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO
template <> struct LvlTypeMap<VkRenderPassAttachmentBeginInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO> {
    typedef VkRenderPassAttachmentBeginInfo Type;
};

// Map type VkPhysicalDeviceUniformBufferStandardLayoutFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceUniformBufferStandardLayoutFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES> {
    typedef VkPhysicalDeviceUniformBufferStandardLayoutFeatures Type;
};

// Map type VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES> {
    typedef VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures Type;
};

// Map type VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES> {
    typedef VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures Type;
};

// Map type VkAttachmentReferenceStencilLayout to id VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_STENCIL_LAYOUT
template <> struct LvlTypeMap<VkAttachmentReferenceStencilLayout> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_STENCIL_LAYOUT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_STENCIL_LAYOUT> {
    typedef VkAttachmentReferenceStencilLayout Type;
};

// Map type VkAttachmentDescriptionStencilLayout to id VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT
template <> struct LvlTypeMap<VkAttachmentDescriptionStencilLayout> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT> {
    typedef VkAttachmentDescriptionStencilLayout Type;
};

// Map type VkPhysicalDeviceHostQueryResetFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceHostQueryResetFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES> {
    typedef VkPhysicalDeviceHostQueryResetFeatures Type;
};

// Map type VkPhysicalDeviceTimelineSemaphoreFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceTimelineSemaphoreFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES> {
    typedef VkPhysicalDeviceTimelineSemaphoreFeatures Type;
};

// Map type VkPhysicalDeviceTimelineSemaphoreProperties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceTimelineSemaphoreProperties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES> {
    typedef VkPhysicalDeviceTimelineSemaphoreProperties Type;
};

// Map type VkSemaphoreTypeCreateInfo to id VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO
template <> struct LvlTypeMap<VkSemaphoreTypeCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO> {
    typedef VkSemaphoreTypeCreateInfo Type;
};

// Map type VkTimelineSemaphoreSubmitInfo to id VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO
template <> struct LvlTypeMap<VkTimelineSemaphoreSubmitInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO> {
    typedef VkTimelineSemaphoreSubmitInfo Type;
};

// Map type VkSemaphoreWaitInfo to id VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO
template <> struct LvlTypeMap<VkSemaphoreWaitInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO> {
    typedef VkSemaphoreWaitInfo Type;
};

// Map type VkSemaphoreSignalInfo to id VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO
template <> struct LvlTypeMap<VkSemaphoreSignalInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO> {
    typedef VkSemaphoreSignalInfo Type;
};

// Map type VkPhysicalDeviceBufferDeviceAddressFeatures to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceBufferDeviceAddressFeatures> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES> {
    typedef VkPhysicalDeviceBufferDeviceAddressFeatures Type;
};

// Map type VkBufferDeviceAddressInfo to id VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO
template <> struct LvlTypeMap<VkBufferDeviceAddressInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO> {
    typedef VkBufferDeviceAddressInfo Type;
};

// Map type VkBufferOpaqueCaptureAddressCreateInfo to id VK_STRUCTURE_TYPE_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO
template <> struct LvlTypeMap<VkBufferOpaqueCaptureAddressCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO> {
    typedef VkBufferOpaqueCaptureAddressCreateInfo Type;
};

// Map type VkMemoryOpaqueCaptureAddressAllocateInfo to id VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO
template <> struct LvlTypeMap<VkMemoryOpaqueCaptureAddressAllocateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO> {
    typedef VkMemoryOpaqueCaptureAddressAllocateInfo Type;
};

// Map type VkDeviceMemoryOpaqueCaptureAddressInfo to id VK_STRUCTURE_TYPE_DEVICE_MEMORY_OPAQUE_CAPTURE_ADDRESS_INFO
template <> struct LvlTypeMap<VkDeviceMemoryOpaqueCaptureAddressInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_MEMORY_OPAQUE_CAPTURE_ADDRESS_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_MEMORY_OPAQUE_CAPTURE_ADDRESS_INFO> {
    typedef VkDeviceMemoryOpaqueCaptureAddressInfo Type;
};

// Map type VkPhysicalDeviceVulkanSC10Features to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_SC_1_0_FEATURES
template <> struct LvlTypeMap<VkPhysicalDeviceVulkanSC10Features> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_SC_1_0_FEATURES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_SC_1_0_FEATURES> {
    typedef VkPhysicalDeviceVulkanSC10Features Type;
};

// Map type VkPhysicalDeviceVulkanSC10Properties to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_SC_1_0_PROPERTIES
template <> struct LvlTypeMap<VkPhysicalDeviceVulkanSC10Properties> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_SC_1_0_PROPERTIES;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_SC_1_0_PROPERTIES> {
    typedef VkPhysicalDeviceVulkanSC10Properties Type;
};

// Map type VkPipelinePoolSize to id VK_STRUCTURE_TYPE_PIPELINE_POOL_SIZE
template <> struct LvlTypeMap<VkPipelinePoolSize> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_POOL_SIZE;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_POOL_SIZE> {
    typedef VkPipelinePoolSize Type;
};

// Map type VkDeviceObjectReservationCreateInfo to id VK_STRUCTURE_TYPE_DEVICE_OBJECT_RESERVATION_CREATE_INFO
template <> struct LvlTypeMap<VkDeviceObjectReservationCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_OBJECT_RESERVATION_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_OBJECT_RESERVATION_CREATE_INFO> {
    typedef VkDeviceObjectReservationCreateInfo Type;
};

// Map type VkCommandPoolMemoryReservationCreateInfo to id VK_STRUCTURE_TYPE_COMMAND_POOL_MEMORY_RESERVATION_CREATE_INFO
template <> struct LvlTypeMap<VkCommandPoolMemoryReservationCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COMMAND_POOL_MEMORY_RESERVATION_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COMMAND_POOL_MEMORY_RESERVATION_CREATE_INFO> {
    typedef VkCommandPoolMemoryReservationCreateInfo Type;
};

// Map type VkCommandPoolMemoryConsumption to id VK_STRUCTURE_TYPE_COMMAND_POOL_MEMORY_CONSUMPTION
template <> struct LvlTypeMap<VkCommandPoolMemoryConsumption> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COMMAND_POOL_MEMORY_CONSUMPTION;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COMMAND_POOL_MEMORY_CONSUMPTION> {
    typedef VkCommandPoolMemoryConsumption Type;
};

// Map type VkFaultData to id VK_STRUCTURE_TYPE_FAULT_DATA
template <> struct LvlTypeMap<VkFaultData> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FAULT_DATA;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FAULT_DATA> {
    typedef VkFaultData Type;
};

// Map type VkFaultCallbackInfo to id VK_STRUCTURE_TYPE_FAULT_CALLBACK_INFO
template <> struct LvlTypeMap<VkFaultCallbackInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FAULT_CALLBACK_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FAULT_CALLBACK_INFO> {
    typedef VkFaultCallbackInfo Type;
};

// Map type VkPipelineOfflineCreateInfo to id VK_STRUCTURE_TYPE_PIPELINE_OFFLINE_CREATE_INFO
template <> struct LvlTypeMap<VkPipelineOfflineCreateInfo> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_OFFLINE_CREATE_INFO;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_OFFLINE_CREATE_INFO> {
    typedef VkPipelineOfflineCreateInfo Type;
};

// Map type VkSwapchainCreateInfoKHR to id VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkSwapchainCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR> {
    typedef VkSwapchainCreateInfoKHR Type;
};

// Map type VkPresentInfoKHR to id VK_STRUCTURE_TYPE_PRESENT_INFO_KHR
template <> struct LvlTypeMap<VkPresentInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PRESENT_INFO_KHR> {
    typedef VkPresentInfoKHR Type;
};

// Map type VkImageSwapchainCreateInfoKHR to id VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkImageSwapchainCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR> {
    typedef VkImageSwapchainCreateInfoKHR Type;
};

// Map type VkBindImageMemorySwapchainInfoKHR to id VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR
template <> struct LvlTypeMap<VkBindImageMemorySwapchainInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR> {
    typedef VkBindImageMemorySwapchainInfoKHR Type;
};

// Map type VkAcquireNextImageInfoKHR to id VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR
template <> struct LvlTypeMap<VkAcquireNextImageInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR> {
    typedef VkAcquireNextImageInfoKHR Type;
};

// Map type VkDeviceGroupPresentCapabilitiesKHR to id VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_CAPABILITIES_KHR
template <> struct LvlTypeMap<VkDeviceGroupPresentCapabilitiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_CAPABILITIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_CAPABILITIES_KHR> {
    typedef VkDeviceGroupPresentCapabilitiesKHR Type;
};

// Map type VkDeviceGroupPresentInfoKHR to id VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_INFO_KHR
template <> struct LvlTypeMap<VkDeviceGroupPresentInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_INFO_KHR> {
    typedef VkDeviceGroupPresentInfoKHR Type;
};

// Map type VkDeviceGroupSwapchainCreateInfoKHR to id VK_STRUCTURE_TYPE_DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkDeviceGroupSwapchainCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR> {
    typedef VkDeviceGroupSwapchainCreateInfoKHR Type;
};

// Map type VkDisplayModeCreateInfoKHR to id VK_STRUCTURE_TYPE_DISPLAY_MODE_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkDisplayModeCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_MODE_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_MODE_CREATE_INFO_KHR> {
    typedef VkDisplayModeCreateInfoKHR Type;
};

// Map type VkDisplaySurfaceCreateInfoKHR to id VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkDisplaySurfaceCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR> {
    typedef VkDisplaySurfaceCreateInfoKHR Type;
};

// Map type VkDisplayPresentInfoKHR to id VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR
template <> struct LvlTypeMap<VkDisplayPresentInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR> {
    typedef VkDisplayPresentInfoKHR Type;
};

// Map type VkImportMemoryFdInfoKHR to id VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR
template <> struct LvlTypeMap<VkImportMemoryFdInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR> {
    typedef VkImportMemoryFdInfoKHR Type;
};

// Map type VkMemoryFdPropertiesKHR to id VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR
template <> struct LvlTypeMap<VkMemoryFdPropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR> {
    typedef VkMemoryFdPropertiesKHR Type;
};

// Map type VkMemoryGetFdInfoKHR to id VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR
template <> struct LvlTypeMap<VkMemoryGetFdInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR> {
    typedef VkMemoryGetFdInfoKHR Type;
};

// Map type VkImportSemaphoreFdInfoKHR to id VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR
template <> struct LvlTypeMap<VkImportSemaphoreFdInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR> {
    typedef VkImportSemaphoreFdInfoKHR Type;
};

// Map type VkSemaphoreGetFdInfoKHR to id VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR
template <> struct LvlTypeMap<VkSemaphoreGetFdInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR> {
    typedef VkSemaphoreGetFdInfoKHR Type;
};

// Map type VkPresentRegionsKHR to id VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR
template <> struct LvlTypeMap<VkPresentRegionsKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR> {
    typedef VkPresentRegionsKHR Type;
};

// Map type VkSharedPresentSurfaceCapabilitiesKHR to id VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR
template <> struct LvlTypeMap<VkSharedPresentSurfaceCapabilitiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR> {
    typedef VkSharedPresentSurfaceCapabilitiesKHR Type;
};

// Map type VkImportFenceFdInfoKHR to id VK_STRUCTURE_TYPE_IMPORT_FENCE_FD_INFO_KHR
template <> struct LvlTypeMap<VkImportFenceFdInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_FENCE_FD_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_FENCE_FD_INFO_KHR> {
    typedef VkImportFenceFdInfoKHR Type;
};

// Map type VkFenceGetFdInfoKHR to id VK_STRUCTURE_TYPE_FENCE_GET_FD_INFO_KHR
template <> struct LvlTypeMap<VkFenceGetFdInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FENCE_GET_FD_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FENCE_GET_FD_INFO_KHR> {
    typedef VkFenceGetFdInfoKHR Type;
};

// Map type VkPhysicalDevicePerformanceQueryFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDevicePerformanceQueryFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR> {
    typedef VkPhysicalDevicePerformanceQueryFeaturesKHR Type;
};

// Map type VkPhysicalDevicePerformanceQueryPropertiesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_PROPERTIES_KHR
template <> struct LvlTypeMap<VkPhysicalDevicePerformanceQueryPropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_PROPERTIES_KHR> {
    typedef VkPhysicalDevicePerformanceQueryPropertiesKHR Type;
};

// Map type VkPerformanceCounterKHR to id VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_KHR
template <> struct LvlTypeMap<VkPerformanceCounterKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_KHR> {
    typedef VkPerformanceCounterKHR Type;
};

// Map type VkPerformanceCounterDescriptionKHR to id VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_DESCRIPTION_KHR
template <> struct LvlTypeMap<VkPerformanceCounterDescriptionKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_DESCRIPTION_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_DESCRIPTION_KHR> {
    typedef VkPerformanceCounterDescriptionKHR Type;
};

// Map type VkQueryPoolPerformanceCreateInfoKHR to id VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkQueryPoolPerformanceCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR> {
    typedef VkQueryPoolPerformanceCreateInfoKHR Type;
};

// Map type VkAcquireProfilingLockInfoKHR to id VK_STRUCTURE_TYPE_ACQUIRE_PROFILING_LOCK_INFO_KHR
template <> struct LvlTypeMap<VkAcquireProfilingLockInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_ACQUIRE_PROFILING_LOCK_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_ACQUIRE_PROFILING_LOCK_INFO_KHR> {
    typedef VkAcquireProfilingLockInfoKHR Type;
};

// Map type VkPerformanceQuerySubmitInfoKHR to id VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_SUBMIT_INFO_KHR
template <> struct LvlTypeMap<VkPerformanceQuerySubmitInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_SUBMIT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_SUBMIT_INFO_KHR> {
    typedef VkPerformanceQuerySubmitInfoKHR Type;
};

// Map type VkPerformanceQueryReservationInfoKHR to id VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_RESERVATION_INFO_KHR
template <> struct LvlTypeMap<VkPerformanceQueryReservationInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_RESERVATION_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_RESERVATION_INFO_KHR> {
    typedef VkPerformanceQueryReservationInfoKHR Type;
};

// Map type VkPhysicalDeviceSurfaceInfo2KHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceSurfaceInfo2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR> {
    typedef VkPhysicalDeviceSurfaceInfo2KHR Type;
};

// Map type VkSurfaceCapabilities2KHR to id VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR
template <> struct LvlTypeMap<VkSurfaceCapabilities2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR> {
    typedef VkSurfaceCapabilities2KHR Type;
};

// Map type VkSurfaceFormat2KHR to id VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR
template <> struct LvlTypeMap<VkSurfaceFormat2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR> {
    typedef VkSurfaceFormat2KHR Type;
};

// Map type VkDisplayProperties2KHR to id VK_STRUCTURE_TYPE_DISPLAY_PROPERTIES_2_KHR
template <> struct LvlTypeMap<VkDisplayProperties2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_PROPERTIES_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_PROPERTIES_2_KHR> {
    typedef VkDisplayProperties2KHR Type;
};

// Map type VkDisplayPlaneProperties2KHR to id VK_STRUCTURE_TYPE_DISPLAY_PLANE_PROPERTIES_2_KHR
template <> struct LvlTypeMap<VkDisplayPlaneProperties2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_PLANE_PROPERTIES_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_PLANE_PROPERTIES_2_KHR> {
    typedef VkDisplayPlaneProperties2KHR Type;
};

// Map type VkDisplayModeProperties2KHR to id VK_STRUCTURE_TYPE_DISPLAY_MODE_PROPERTIES_2_KHR
template <> struct LvlTypeMap<VkDisplayModeProperties2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_MODE_PROPERTIES_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_MODE_PROPERTIES_2_KHR> {
    typedef VkDisplayModeProperties2KHR Type;
};

// Map type VkDisplayPlaneInfo2KHR to id VK_STRUCTURE_TYPE_DISPLAY_PLANE_INFO_2_KHR
template <> struct LvlTypeMap<VkDisplayPlaneInfo2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_PLANE_INFO_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_PLANE_INFO_2_KHR> {
    typedef VkDisplayPlaneInfo2KHR Type;
};

// Map type VkDisplayPlaneCapabilities2KHR to id VK_STRUCTURE_TYPE_DISPLAY_PLANE_CAPABILITIES_2_KHR
template <> struct LvlTypeMap<VkDisplayPlaneCapabilities2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_PLANE_CAPABILITIES_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_PLANE_CAPABILITIES_2_KHR> {
    typedef VkDisplayPlaneCapabilities2KHR Type;
};

// Map type VkPhysicalDeviceShaderClockFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceShaderClockFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR> {
    typedef VkPhysicalDeviceShaderClockFeaturesKHR Type;
};

// Map type VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES_KHR> {
    typedef VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR Type;
};

// Map type VkFragmentShadingRateAttachmentInfoKHR to id VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR
template <> struct LvlTypeMap<VkFragmentShadingRateAttachmentInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR> {
    typedef VkFragmentShadingRateAttachmentInfoKHR Type;
};

// Map type VkPipelineFragmentShadingRateStateCreateInfoKHR to id VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_STATE_CREATE_INFO_KHR
template <> struct LvlTypeMap<VkPipelineFragmentShadingRateStateCreateInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_STATE_CREATE_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_STATE_CREATE_INFO_KHR> {
    typedef VkPipelineFragmentShadingRateStateCreateInfoKHR Type;
};

// Map type VkPhysicalDeviceFragmentShadingRateFeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceFragmentShadingRateFeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR> {
    typedef VkPhysicalDeviceFragmentShadingRateFeaturesKHR Type;
};

// Map type VkPhysicalDeviceFragmentShadingRatePropertiesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceFragmentShadingRatePropertiesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR> {
    typedef VkPhysicalDeviceFragmentShadingRatePropertiesKHR Type;
};

// Map type VkPhysicalDeviceFragmentShadingRateKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceFragmentShadingRateKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_KHR> {
    typedef VkPhysicalDeviceFragmentShadingRateKHR Type;
};

// Map type VkRefreshObjectListKHR to id VK_STRUCTURE_TYPE_REFRESH_OBJECT_LIST_KHR
template <> struct LvlTypeMap<VkRefreshObjectListKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_REFRESH_OBJECT_LIST_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_REFRESH_OBJECT_LIST_KHR> {
    typedef VkRefreshObjectListKHR Type;
};

// Map type VkMemoryBarrier2KHR to id VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR
template <> struct LvlTypeMap<VkMemoryBarrier2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR> {
    typedef VkMemoryBarrier2KHR Type;
};

// Map type VkBufferMemoryBarrier2KHR to id VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2_KHR
template <> struct LvlTypeMap<VkBufferMemoryBarrier2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2_KHR> {
    typedef VkBufferMemoryBarrier2KHR Type;
};

// Map type VkImageMemoryBarrier2KHR to id VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2_KHR
template <> struct LvlTypeMap<VkImageMemoryBarrier2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2_KHR> {
    typedef VkImageMemoryBarrier2KHR Type;
};

// Map type VkDependencyInfoKHR to id VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR
template <> struct LvlTypeMap<VkDependencyInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR> {
    typedef VkDependencyInfoKHR Type;
};

// Map type VkSemaphoreSubmitInfoKHR to id VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR
template <> struct LvlTypeMap<VkSemaphoreSubmitInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR> {
    typedef VkSemaphoreSubmitInfoKHR Type;
};

// Map type VkCommandBufferSubmitInfoKHR to id VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR
template <> struct LvlTypeMap<VkCommandBufferSubmitInfoKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR> {
    typedef VkCommandBufferSubmitInfoKHR Type;
};

// Map type VkSubmitInfo2KHR to id VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR
template <> struct LvlTypeMap<VkSubmitInfo2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR> {
    typedef VkSubmitInfo2KHR Type;
};

// Map type VkPhysicalDeviceSynchronization2FeaturesKHR to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR
template <> struct LvlTypeMap<VkPhysicalDeviceSynchronization2FeaturesKHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR> {
    typedef VkPhysicalDeviceSynchronization2FeaturesKHR Type;
};

// Map type VkQueueFamilyCheckpointProperties2NV to id VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_2_NV
template <> struct LvlTypeMap<VkQueueFamilyCheckpointProperties2NV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_2_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_2_NV> {
    typedef VkQueueFamilyCheckpointProperties2NV Type;
};

// Map type VkCheckpointData2NV to id VK_STRUCTURE_TYPE_CHECKPOINT_DATA_2_NV
template <> struct LvlTypeMap<VkCheckpointData2NV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_CHECKPOINT_DATA_2_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_CHECKPOINT_DATA_2_NV> {
    typedef VkCheckpointData2NV Type;
};

// Map type VkBufferCopy2KHR to id VK_STRUCTURE_TYPE_BUFFER_COPY_2_KHR
template <> struct LvlTypeMap<VkBufferCopy2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_COPY_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_COPY_2_KHR> {
    typedef VkBufferCopy2KHR Type;
};

// Map type VkCopyBufferInfo2KHR to id VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2_KHR
template <> struct LvlTypeMap<VkCopyBufferInfo2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2_KHR> {
    typedef VkCopyBufferInfo2KHR Type;
};

// Map type VkImageCopy2KHR to id VK_STRUCTURE_TYPE_IMAGE_COPY_2_KHR
template <> struct LvlTypeMap<VkImageCopy2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_COPY_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_COPY_2_KHR> {
    typedef VkImageCopy2KHR Type;
};

// Map type VkCopyImageInfo2KHR to id VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2_KHR
template <> struct LvlTypeMap<VkCopyImageInfo2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2_KHR> {
    typedef VkCopyImageInfo2KHR Type;
};

// Map type VkBufferImageCopy2KHR to id VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2_KHR
template <> struct LvlTypeMap<VkBufferImageCopy2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2_KHR> {
    typedef VkBufferImageCopy2KHR Type;
};

// Map type VkCopyBufferToImageInfo2KHR to id VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2_KHR
template <> struct LvlTypeMap<VkCopyBufferToImageInfo2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2_KHR> {
    typedef VkCopyBufferToImageInfo2KHR Type;
};

// Map type VkCopyImageToBufferInfo2KHR to id VK_STRUCTURE_TYPE_COPY_IMAGE_TO_BUFFER_INFO_2_KHR
template <> struct LvlTypeMap<VkCopyImageToBufferInfo2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_COPY_IMAGE_TO_BUFFER_INFO_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_COPY_IMAGE_TO_BUFFER_INFO_2_KHR> {
    typedef VkCopyImageToBufferInfo2KHR Type;
};

// Map type VkImageBlit2KHR to id VK_STRUCTURE_TYPE_IMAGE_BLIT_2_KHR
template <> struct LvlTypeMap<VkImageBlit2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_BLIT_2_KHR> {
    typedef VkImageBlit2KHR Type;
};

// Map type VkBlitImageInfo2KHR to id VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2_KHR
template <> struct LvlTypeMap<VkBlitImageInfo2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2_KHR> {
    typedef VkBlitImageInfo2KHR Type;
};

// Map type VkImageResolve2KHR to id VK_STRUCTURE_TYPE_IMAGE_RESOLVE_2_KHR
template <> struct LvlTypeMap<VkImageResolve2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_RESOLVE_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_RESOLVE_2_KHR> {
    typedef VkImageResolve2KHR Type;
};

// Map type VkResolveImageInfo2KHR to id VK_STRUCTURE_TYPE_RESOLVE_IMAGE_INFO_2_KHR
template <> struct LvlTypeMap<VkResolveImageInfo2KHR> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RESOLVE_IMAGE_INFO_2_KHR;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RESOLVE_IMAGE_INFO_2_KHR> {
    typedef VkResolveImageInfo2KHR Type;
};

// Map type VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES_EXT> {
    typedef VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT Type;
};

// Map type VkImageViewASTCDecodeModeEXT to id VK_STRUCTURE_TYPE_IMAGE_VIEW_ASTC_DECODE_MODE_EXT
template <> struct LvlTypeMap<VkImageViewASTCDecodeModeEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_VIEW_ASTC_DECODE_MODE_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_VIEW_ASTC_DECODE_MODE_EXT> {
    typedef VkImageViewASTCDecodeModeEXT Type;
};

// Map type VkPhysicalDeviceASTCDecodeFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceASTCDecodeFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT> {
    typedef VkPhysicalDeviceASTCDecodeFeaturesEXT Type;
};

// Map type VkSurfaceCapabilities2EXT to id VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_EXT
template <> struct LvlTypeMap<VkSurfaceCapabilities2EXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_EXT> {
    typedef VkSurfaceCapabilities2EXT Type;
};

// Map type VkDisplayPowerInfoEXT to id VK_STRUCTURE_TYPE_DISPLAY_POWER_INFO_EXT
template <> struct LvlTypeMap<VkDisplayPowerInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_POWER_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_POWER_INFO_EXT> {
    typedef VkDisplayPowerInfoEXT Type;
};

// Map type VkDeviceEventInfoEXT to id VK_STRUCTURE_TYPE_DEVICE_EVENT_INFO_EXT
template <> struct LvlTypeMap<VkDeviceEventInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_EVENT_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_EVENT_INFO_EXT> {
    typedef VkDeviceEventInfoEXT Type;
};

// Map type VkDisplayEventInfoEXT to id VK_STRUCTURE_TYPE_DISPLAY_EVENT_INFO_EXT
template <> struct LvlTypeMap<VkDisplayEventInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DISPLAY_EVENT_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DISPLAY_EVENT_INFO_EXT> {
    typedef VkDisplayEventInfoEXT Type;
};

// Map type VkSwapchainCounterCreateInfoEXT to id VK_STRUCTURE_TYPE_SWAPCHAIN_COUNTER_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkSwapchainCounterCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SWAPCHAIN_COUNTER_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SWAPCHAIN_COUNTER_CREATE_INFO_EXT> {
    typedef VkSwapchainCounterCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceDiscardRectanglePropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceDiscardRectanglePropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceDiscardRectanglePropertiesEXT Type;
};

// Map type VkPipelineDiscardRectangleStateCreateInfoEXT to id VK_STRUCTURE_TYPE_PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkPipelineDiscardRectangleStateCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT> {
    typedef VkPipelineDiscardRectangleStateCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceConservativeRasterizationPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceConservativeRasterizationPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceConservativeRasterizationPropertiesEXT Type;
};

// Map type VkPipelineRasterizationConservativeStateCreateInfoEXT to id VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkPipelineRasterizationConservativeStateCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT> {
    typedef VkPipelineRasterizationConservativeStateCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceDepthClipEnableFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceDepthClipEnableFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT> {
    typedef VkPhysicalDeviceDepthClipEnableFeaturesEXT Type;
};

// Map type VkPipelineRasterizationDepthClipStateCreateInfoEXT to id VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkPipelineRasterizationDepthClipStateCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT> {
    typedef VkPipelineRasterizationDepthClipStateCreateInfoEXT Type;
};

// Map type VkHdrMetadataEXT to id VK_STRUCTURE_TYPE_HDR_METADATA_EXT
template <> struct LvlTypeMap<VkHdrMetadataEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_HDR_METADATA_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_HDR_METADATA_EXT> {
    typedef VkHdrMetadataEXT Type;
};

// Map type VkDebugUtilsLabelEXT to id VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT
template <> struct LvlTypeMap<VkDebugUtilsLabelEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT> {
    typedef VkDebugUtilsLabelEXT Type;
};

// Map type VkDebugUtilsObjectNameInfoEXT to id VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT
template <> struct LvlTypeMap<VkDebugUtilsObjectNameInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT> {
    typedef VkDebugUtilsObjectNameInfoEXT Type;
};

// Map type VkDebugUtilsMessengerCallbackDataEXT to id VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT
template <> struct LvlTypeMap<VkDebugUtilsMessengerCallbackDataEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT> {
    typedef VkDebugUtilsMessengerCallbackDataEXT Type;
};

// Map type VkDebugUtilsMessengerCreateInfoEXT to id VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkDebugUtilsMessengerCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT> {
    typedef VkDebugUtilsMessengerCreateInfoEXT Type;
};

// Map type VkDebugUtilsObjectTagInfoEXT to id VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT
template <> struct LvlTypeMap<VkDebugUtilsObjectTagInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT> {
    typedef VkDebugUtilsObjectTagInfoEXT Type;
};

// Map type VkSampleLocationsInfoEXT to id VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT
template <> struct LvlTypeMap<VkSampleLocationsInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT> {
    typedef VkSampleLocationsInfoEXT Type;
};

// Map type VkRenderPassSampleLocationsBeginInfoEXT to id VK_STRUCTURE_TYPE_RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT
template <> struct LvlTypeMap<VkRenderPassSampleLocationsBeginInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT> {
    typedef VkRenderPassSampleLocationsBeginInfoEXT Type;
};

// Map type VkPipelineSampleLocationsStateCreateInfoEXT to id VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkPipelineSampleLocationsStateCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT> {
    typedef VkPipelineSampleLocationsStateCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceSampleLocationsPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceSampleLocationsPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceSampleLocationsPropertiesEXT Type;
};

// Map type VkMultisamplePropertiesEXT to id VK_STRUCTURE_TYPE_MULTISAMPLE_PROPERTIES_EXT
template <> struct LvlTypeMap<VkMultisamplePropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MULTISAMPLE_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MULTISAMPLE_PROPERTIES_EXT> {
    typedef VkMultisamplePropertiesEXT Type;
};

// Map type VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT> {
    typedef VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT Type;
};

// Map type VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT Type;
};

// Map type VkPipelineColorBlendAdvancedStateCreateInfoEXT to id VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkPipelineColorBlendAdvancedStateCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT> {
    typedef VkPipelineColorBlendAdvancedStateCreateInfoEXT Type;
};

// Map type VkDrmFormatModifierPropertiesListEXT to id VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT
template <> struct LvlTypeMap<VkDrmFormatModifierPropertiesListEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT> {
    typedef VkDrmFormatModifierPropertiesListEXT Type;
};

// Map type VkPhysicalDeviceImageDrmFormatModifierInfoEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceImageDrmFormatModifierInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT> {
    typedef VkPhysicalDeviceImageDrmFormatModifierInfoEXT Type;
};

// Map type VkImageDrmFormatModifierListCreateInfoEXT to id VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkImageDrmFormatModifierListCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT> {
    typedef VkImageDrmFormatModifierListCreateInfoEXT Type;
};

// Map type VkImageDrmFormatModifierExplicitCreateInfoEXT to id VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkImageDrmFormatModifierExplicitCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT> {
    typedef VkImageDrmFormatModifierExplicitCreateInfoEXT Type;
};

// Map type VkImageDrmFormatModifierPropertiesEXT to id VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_PROPERTIES_EXT
template <> struct LvlTypeMap<VkImageDrmFormatModifierPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_PROPERTIES_EXT> {
    typedef VkImageDrmFormatModifierPropertiesEXT Type;
};

// Map type VkDrmFormatModifierPropertiesList2EXT to id VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_2_EXT
template <> struct LvlTypeMap<VkDrmFormatModifierPropertiesList2EXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_2_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_2_EXT> {
    typedef VkDrmFormatModifierPropertiesList2EXT Type;
};

// Map type VkPhysicalDeviceImageViewImageFormatInfoEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_IMAGE_FORMAT_INFO_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceImageViewImageFormatInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_IMAGE_FORMAT_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_IMAGE_FORMAT_INFO_EXT> {
    typedef VkPhysicalDeviceImageViewImageFormatInfoEXT Type;
};

// Map type VkFilterCubicImageViewImageFormatPropertiesEXT to id VK_STRUCTURE_TYPE_FILTER_CUBIC_IMAGE_VIEW_IMAGE_FORMAT_PROPERTIES_EXT
template <> struct LvlTypeMap<VkFilterCubicImageViewImageFormatPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FILTER_CUBIC_IMAGE_VIEW_IMAGE_FORMAT_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FILTER_CUBIC_IMAGE_VIEW_IMAGE_FORMAT_PROPERTIES_EXT> {
    typedef VkFilterCubicImageViewImageFormatPropertiesEXT Type;
};

// Map type VkDeviceQueueGlobalPriorityCreateInfoEXT to id VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkDeviceQueueGlobalPriorityCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_EXT> {
    typedef VkDeviceQueueGlobalPriorityCreateInfoEXT Type;
};

// Map type VkImportMemoryHostPointerInfoEXT to id VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT
template <> struct LvlTypeMap<VkImportMemoryHostPointerInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT> {
    typedef VkImportMemoryHostPointerInfoEXT Type;
};

// Map type VkMemoryHostPointerPropertiesEXT to id VK_STRUCTURE_TYPE_MEMORY_HOST_POINTER_PROPERTIES_EXT
template <> struct LvlTypeMap<VkMemoryHostPointerPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_HOST_POINTER_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_HOST_POINTER_PROPERTIES_EXT> {
    typedef VkMemoryHostPointerPropertiesEXT Type;
};

// Map type VkPhysicalDeviceExternalMemoryHostPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceExternalMemoryHostPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceExternalMemoryHostPropertiesEXT Type;
};

// Map type VkCalibratedTimestampInfoEXT to id VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_EXT
template <> struct LvlTypeMap<VkCalibratedTimestampInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_EXT> {
    typedef VkCalibratedTimestampInfoEXT Type;
};

// Map type VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT Type;
};

// Map type VkPipelineVertexInputDivisorStateCreateInfoEXT to id VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkPipelineVertexInputDivisorStateCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT> {
    typedef VkPipelineVertexInputDivisorStateCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT> {
    typedef VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT Type;
};

// Map type VkPhysicalDevicePCIBusInfoPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDevicePCIBusInfoPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT> {
    typedef VkPhysicalDevicePCIBusInfoPropertiesEXT Type;
};

// Map type VkPhysicalDeviceSubgroupSizeControlFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceSubgroupSizeControlFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES_EXT> {
    typedef VkPhysicalDeviceSubgroupSizeControlFeaturesEXT Type;
};

// Map type VkPhysicalDeviceSubgroupSizeControlPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceSubgroupSizeControlPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceSubgroupSizeControlPropertiesEXT Type;
};

// Map type VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT to id VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO_EXT> {
    typedef VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT> {
    typedef VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT Type;
};

// Map type VkPhysicalDeviceMemoryBudgetPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceMemoryBudgetPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceMemoryBudgetPropertiesEXT Type;
};

// Map type VkValidationFeaturesEXT to id VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT
template <> struct LvlTypeMap<VkValidationFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT> {
    typedef VkValidationFeaturesEXT Type;
};

// Map type VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT> {
    typedef VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT Type;
};

// Map type VkPhysicalDeviceYcbcrImageArraysFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceYcbcrImageArraysFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT> {
    typedef VkPhysicalDeviceYcbcrImageArraysFeaturesEXT Type;
};

// Map type VkHeadlessSurfaceCreateInfoEXT to id VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkHeadlessSurfaceCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT> {
    typedef VkHeadlessSurfaceCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceLineRasterizationFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceLineRasterizationFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT> {
    typedef VkPhysicalDeviceLineRasterizationFeaturesEXT Type;
};

// Map type VkPhysicalDeviceLineRasterizationPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceLineRasterizationPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceLineRasterizationPropertiesEXT Type;
};

// Map type VkPipelineRasterizationLineStateCreateInfoEXT to id VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkPipelineRasterizationLineStateCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT> {
    typedef VkPipelineRasterizationLineStateCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceShaderAtomicFloatFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT> {
    typedef VkPhysicalDeviceShaderAtomicFloatFeaturesEXT Type;
};

// Map type VkPhysicalDeviceIndexTypeUint8FeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceIndexTypeUint8FeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_EXT> {
    typedef VkPhysicalDeviceIndexTypeUint8FeaturesEXT Type;
};

// Map type VkPhysicalDeviceExtendedDynamicStateFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT> {
    typedef VkPhysicalDeviceExtendedDynamicStateFeaturesEXT Type;
};

// Map type VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES_EXT> {
    typedef VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT Type;
};

// Map type VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT> {
    typedef VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT Type;
};

// Map type VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT Type;
};

// Map type VkPhysicalDeviceRobustness2FeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceRobustness2FeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT> {
    typedef VkPhysicalDeviceRobustness2FeaturesEXT Type;
};

// Map type VkPhysicalDeviceRobustness2PropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceRobustness2PropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceRobustness2PropertiesEXT Type;
};

// Map type VkSamplerCustomBorderColorCreateInfoEXT to id VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkSamplerCustomBorderColorCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT> {
    typedef VkSamplerCustomBorderColorCreateInfoEXT Type;
};

// Map type VkPhysicalDeviceCustomBorderColorPropertiesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_PROPERTIES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceCustomBorderColorPropertiesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_PROPERTIES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_PROPERTIES_EXT> {
    typedef VkPhysicalDeviceCustomBorderColorPropertiesEXT Type;
};

// Map type VkPhysicalDeviceCustomBorderColorFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceCustomBorderColorFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT> {
    typedef VkPhysicalDeviceCustomBorderColorFeaturesEXT Type;
};

// Map type VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_2_PLANE_444_FORMATS_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_2_PLANE_444_FORMATS_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_2_PLANE_444_FORMATS_FEATURES_EXT> {
    typedef VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT Type;
};

// Map type VkPhysicalDeviceImageRobustnessFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceImageRobustnessFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES_EXT> {
    typedef VkPhysicalDeviceImageRobustnessFeaturesEXT Type;
};

// Map type VkPhysicalDevice4444FormatsFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDevice4444FormatsFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT> {
    typedef VkPhysicalDevice4444FormatsFeaturesEXT Type;
};

// Map type VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT> {
    typedef VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT Type;
};

// Map type VkVertexInputBindingDescription2EXT to id VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT
template <> struct LvlTypeMap<VkVertexInputBindingDescription2EXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT> {
    typedef VkVertexInputBindingDescription2EXT Type;
};

// Map type VkVertexInputAttributeDescription2EXT to id VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT
template <> struct LvlTypeMap<VkVertexInputAttributeDescription2EXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT> {
    typedef VkVertexInputAttributeDescription2EXT Type;
};

#ifdef VK_USE_PLATFORM_SCI
// Map type VkExportFenceSciSyncInfoNV to id VK_STRUCTURE_TYPE_EXPORT_FENCE_SCI_SYNC_INFO_NV
template <> struct LvlTypeMap<VkExportFenceSciSyncInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXPORT_FENCE_SCI_SYNC_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXPORT_FENCE_SCI_SYNC_INFO_NV> {
    typedef VkExportFenceSciSyncInfoNV Type;
};

#endif // VK_USE_PLATFORM_SCI
#ifdef VK_USE_PLATFORM_SCI
// Map type VkImportFenceSciSyncInfoNV to id VK_STRUCTURE_TYPE_IMPORT_FENCE_SCI_SYNC_INFO_NV
template <> struct LvlTypeMap<VkImportFenceSciSyncInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_FENCE_SCI_SYNC_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_FENCE_SCI_SYNC_INFO_NV> {
    typedef VkImportFenceSciSyncInfoNV Type;
};

#endif // VK_USE_PLATFORM_SCI
#ifdef VK_USE_PLATFORM_SCI
// Map type VkFenceGetSciSyncInfoNV to id VK_STRUCTURE_TYPE_FENCE_GET_SCI_SYNC_INFO_NV
template <> struct LvlTypeMap<VkFenceGetSciSyncInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_FENCE_GET_SCI_SYNC_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_FENCE_GET_SCI_SYNC_INFO_NV> {
    typedef VkFenceGetSciSyncInfoNV Type;
};

#endif // VK_USE_PLATFORM_SCI
#ifdef VK_USE_PLATFORM_SCI
// Map type VkSciSyncAttributesInfoNV to id VK_STRUCTURE_TYPE_SCI_SYNC_ATTRIBUTES_INFO_NV
template <> struct LvlTypeMap<VkSciSyncAttributesInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SCI_SYNC_ATTRIBUTES_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SCI_SYNC_ATTRIBUTES_INFO_NV> {
    typedef VkSciSyncAttributesInfoNV Type;
};

#endif // VK_USE_PLATFORM_SCI
#ifdef VK_USE_PLATFORM_SCI
// Map type VkExportSemaphoreSciSyncInfoNV to id VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_SCI_SYNC_INFO_NV
template <> struct LvlTypeMap<VkExportSemaphoreSciSyncInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_SCI_SYNC_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_SCI_SYNC_INFO_NV> {
    typedef VkExportSemaphoreSciSyncInfoNV Type;
};

#endif // VK_USE_PLATFORM_SCI
#ifdef VK_USE_PLATFORM_SCI
// Map type VkImportSemaphoreSciSyncInfoNV to id VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_SCI_SYNC_INFO_NV
template <> struct LvlTypeMap<VkImportSemaphoreSciSyncInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_SCI_SYNC_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_SCI_SYNC_INFO_NV> {
    typedef VkImportSemaphoreSciSyncInfoNV Type;
};

#endif // VK_USE_PLATFORM_SCI
#ifdef VK_USE_PLATFORM_SCI
// Map type VkSemaphoreGetSciSyncInfoNV to id VK_STRUCTURE_TYPE_SEMAPHORE_GET_SCI_SYNC_INFO_NV
template <> struct LvlTypeMap<VkSemaphoreGetSciSyncInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_SCI_SYNC_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_SEMAPHORE_GET_SCI_SYNC_INFO_NV> {
    typedef VkSemaphoreGetSciSyncInfoNV Type;
};

#endif // VK_USE_PLATFORM_SCI
#ifdef VK_USE_PLATFORM_SCI
// Map type VkPhysicalDeviceExternalSciSyncFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SCI_SYNC_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceExternalSciSyncFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SCI_SYNC_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SCI_SYNC_FEATURES_NV> {
    typedef VkPhysicalDeviceExternalSciSyncFeaturesNV Type;
};

#endif // VK_USE_PLATFORM_SCI
#ifdef VK_USE_PLATFORM_SCI
// Map type VkExportMemorySciBufInfoNV to id VK_STRUCTURE_TYPE_EXPORT_MEMORY_SCI_BUF_INFO_NV
template <> struct LvlTypeMap<VkExportMemorySciBufInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_SCI_BUF_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_EXPORT_MEMORY_SCI_BUF_INFO_NV> {
    typedef VkExportMemorySciBufInfoNV Type;
};

#endif // VK_USE_PLATFORM_SCI
#ifdef VK_USE_PLATFORM_SCI
// Map type VkImportMemorySciBufInfoNV to id VK_STRUCTURE_TYPE_IMPORT_MEMORY_SCI_BUF_INFO_NV
template <> struct LvlTypeMap<VkImportMemorySciBufInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_SCI_BUF_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_IMPORT_MEMORY_SCI_BUF_INFO_NV> {
    typedef VkImportMemorySciBufInfoNV Type;
};

#endif // VK_USE_PLATFORM_SCI
#ifdef VK_USE_PLATFORM_SCI
// Map type VkMemoryGetSciBufInfoNV to id VK_STRUCTURE_TYPE_MEMORY_GET_SCI_BUF_INFO_NV
template <> struct LvlTypeMap<VkMemoryGetSciBufInfoNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_GET_SCI_BUF_INFO_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_GET_SCI_BUF_INFO_NV> {
    typedef VkMemoryGetSciBufInfoNV Type;
};

#endif // VK_USE_PLATFORM_SCI
#ifdef VK_USE_PLATFORM_SCI
// Map type VkMemorySciBufPropertiesNV to id VK_STRUCTURE_TYPE_MEMORY_SCI_BUF_PROPERTIES_NV
template <> struct LvlTypeMap<VkMemorySciBufPropertiesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_MEMORY_SCI_BUF_PROPERTIES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_MEMORY_SCI_BUF_PROPERTIES_NV> {
    typedef VkMemorySciBufPropertiesNV Type;
};

#endif // VK_USE_PLATFORM_SCI
#ifdef VK_USE_PLATFORM_SCI
// Map type VkPhysicalDeviceExternalSciBufFeaturesNV to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SCI_BUF_FEATURES_NV
template <> struct LvlTypeMap<VkPhysicalDeviceExternalSciBufFeaturesNV> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SCI_BUF_FEATURES_NV;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SCI_BUF_FEATURES_NV> {
    typedef VkPhysicalDeviceExternalSciBufFeaturesNV Type;
};

#endif // VK_USE_PLATFORM_SCI
// Map type VkPhysicalDeviceExtendedDynamicState2FeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT> {
    typedef VkPhysicalDeviceExtendedDynamicState2FeaturesEXT Type;
};

// Map type VkPhysicalDeviceColorWriteEnableFeaturesEXT to id VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT
template <> struct LvlTypeMap<VkPhysicalDeviceColorWriteEnableFeaturesEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT> {
    typedef VkPhysicalDeviceColorWriteEnableFeaturesEXT Type;
};

// Map type VkPipelineColorWriteCreateInfoEXT to id VK_STRUCTURE_TYPE_PIPELINE_COLOR_WRITE_CREATE_INFO_EXT
template <> struct LvlTypeMap<VkPipelineColorWriteCreateInfoEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_WRITE_CREATE_INFO_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_PIPELINE_COLOR_WRITE_CREATE_INFO_EXT> {
    typedef VkPipelineColorWriteCreateInfoEXT Type;
};

// Map type VkApplicationParametersEXT to id VK_STRUCTURE_TYPE_APPLICATION_PARAMETERS_EXT
template <> struct LvlTypeMap<VkApplicationParametersEXT> {
    static const VkStructureType kSType = VK_STRUCTURE_TYPE_APPLICATION_PARAMETERS_EXT;
};

template <> struct LvlSTypeMap<VK_STRUCTURE_TYPE_APPLICATION_PARAMETERS_EXT> {
    typedef VkApplicationParametersEXT Type;
};

// Find an entry of the given type in the pNext chain
template <typename T> const T *LvlFindInChain(const void *next) {
    const VkBaseOutStructure *current = reinterpret_cast<const VkBaseOutStructure *>(next);
    const T *found = nullptr;
    while (current) {
        if (LvlTypeMap<T>::kSType == current->sType) {
            found = reinterpret_cast<const T*>(current);
            current = nullptr;
        } else {
            current = current->pNext;
        }
    }
    return found;
}

// Init the header of an sType struct with pNext
template <typename T> T LvlInitStruct(void *p_next) {
    T out = {};
    out.sType = LvlTypeMap<T>::kSType;
    out.pNext = p_next;
    return out;
}

// Init the header of an sType struct
template <typename T> T LvlInitStruct() {
    T out = {};
    out.sType = LvlTypeMap<T>::kSType;
    out.pNext = nullptr;
    return out;
}


// Find an entry of the given type in the pNext chain
template <typename T> const T *lvl_find_in_chain(const void *next) {
    const VkBaseOutStructure *current = reinterpret_cast<const VkBaseOutStructure *>(next);
    const T *found = nullptr;
    while (current) {
        if (LvlTypeMap<T>::kSType == current->sType) {
            found = reinterpret_cast<const T*>(current);
            current = nullptr;
        } else {
            current = current->pNext;
        }
    }
    return found;
}

// Init the header of an sType struct with pNext
template <typename T> T lvl_init_struct(void *p_next) {
    T out = {};
    out.sType = LvlTypeMap<T>::kSType;
    out.pNext = p_next;
    return out;
}

// Init the header of an sType struct
template <typename T> T lvl_init_struct() {
    T out = {};
    out.sType = LvlTypeMap<T>::kSType;
    out.pNext = nullptr;
    return out;
}

