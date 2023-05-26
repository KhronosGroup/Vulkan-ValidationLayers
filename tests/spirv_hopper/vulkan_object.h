/*
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once
#include <vulkan/vulkan_core.h>

// Holds all Vulkan objects for the entire run
struct VulkanInstance {
  public:
    VulkanInstance();
    ~VulkanInstance();

    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice gpu = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

    VkDebugUtilsMessengerEXT debug_utils_messenger = VK_NULL_HANDLE;

    // Any Validation Errors caught
    bool is_valid = true;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceMemoryProperties memory_properties;

    // Require possible changes, but only need to define once
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                                                                nullptr, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE};
    VkPipelineColorBlendStateCreateInfo color_blend_state{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                                                          nullptr,
                                                          0,
                                                          VK_FALSE,
                                                          VK_LOGIC_OP_CLEAR,
                                                          0,
                                                          nullptr,
                                                          {0.0f, 0.0f, 0.0f, 0.0f}};
    VkPipelineVertexInputStateCreateInfo vertex_input_state{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0, 1, &vertex_input_binding, 0, nullptr};
    VkAttachmentDescription basic_attachment_description = {0,
                                                            VK_FORMAT_R8G8B8A8_UNORM,
                                                            VK_SAMPLE_COUNT_1_BIT,
                                                            VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                            VK_ATTACHMENT_STORE_OP_STORE,
                                                            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                            VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                            VK_IMAGE_LAYOUT_UNDEFINED,
                                                            VK_IMAGE_LAYOUT_GENERAL};

    // Objects that are the same as they don't effect the shader run
    constexpr static VkViewport viewport{0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
    constexpr static VkRect2D scissor{{0, 0}, {1, 1}};
    constexpr static VkPipelineViewportStateCreateInfo viewport_input_state{
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, nullptr, 0, 1, &viewport, 1, &scissor};
    constexpr static VkPipelineRasterizationStateCreateInfo rasterization_state{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_FALSE,
        VK_FALSE,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_NONE,
        VK_FRONT_FACE_CLOCKWISE,
        VK_FALSE,
        1.0f,
        0.0f,
        0.0f,
        1.0f};
    constexpr static VkPipelineMultisampleStateCreateInfo multisample_state{
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FALSE,
        0.0f,
        nullptr,
        VK_FALSE,
        VK_FALSE};
    constexpr static VkPipelineTessellationStateCreateInfo tessellation_state{
        VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, nullptr, 0, 1};
    constexpr static VkVertexInputBindingDescription vertex_input_binding = {0, 16, VK_VERTEX_INPUT_RATE_VERTEX};
};