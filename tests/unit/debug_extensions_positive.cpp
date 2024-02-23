/*
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

TEST_F(PositiveDebugExtensions, SetDebugUtilsObjectBuffer) {
    TEST_DESCRIPTION("call vkSetDebugUtilsObjectNameEXT on a VkBuffer");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Skipping object naming test with MockICD.";
    }

    DebugUtilsLabelCheckData callback_data;
    auto empty_callback = [](const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, DebugUtilsLabelCheckData *data) {
        data->count++;
    };
    callback_data.count = 0;
    callback_data.callback = empty_callback;

    VkDebugUtilsMessengerCreateInfoEXT callback_create_info = vku::InitStructHelper();
    callback_create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    callback_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    callback_create_info.pfnUserCallback = DebugUtilsCallback;
    callback_create_info.pUserData = &callback_data;
    VkDebugUtilsMessengerEXT my_messenger = VK_NULL_HANDLE;
    vk::CreateDebugUtilsMessengerEXT(instance(), &callback_create_info, nullptr, &my_messenger);

    vkt::Buffer buffer(*m_device, 64);
    const char* object_name = "buffer_object";

    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_BUFFER;
    name_info.objectHandle = (uint64_t)buffer.handle();
    name_info.pObjectName = object_name;
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    vk::DestroyDebugUtilsMessengerEXT(instance(), my_messenger, nullptr);
}

TEST_F(PositiveDebugExtensions, SetDebugUtilsObjectDevice) {
    TEST_DESCRIPTION("call vkSetDebugUtilsObjectNameEXT on a itself as a VkDevice object");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Skipping object naming test with MockICD.";
    }

    DebugUtilsLabelCheckData callback_data;
    auto empty_callback = [](const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, DebugUtilsLabelCheckData *data) {
        data->count++;
    };
    callback_data.count = 0;
    callback_data.callback = empty_callback;

    VkDebugUtilsMessengerCreateInfoEXT callback_create_info = vku::InitStructHelper();
    callback_create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    callback_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    callback_create_info.pfnUserCallback = DebugUtilsCallback;
    callback_create_info.pUserData = &callback_data;
    VkDebugUtilsMessengerEXT my_messenger = VK_NULL_HANDLE;
    vk::CreateDebugUtilsMessengerEXT(instance(), &callback_create_info, nullptr, &my_messenger);

    const char* object_name = "device_object";

    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_DEVICE;
    name_info.objectHandle = (uint64_t)device();
    name_info.pObjectName = object_name;
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    vk::DestroyDebugUtilsMessengerEXT(instance(), my_messenger, nullptr);
}

TEST_F(PositiveDebugExtensions, DebugLabelPrimaryCommandBuffer) {
    TEST_DESCRIPTION("Test primary command buffer debug labels");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    m_commandBuffer->begin();
    VkDebugUtilsLabelEXT label = vku::InitStructHelper();
    label.pLabelName = "test";
    vk::CmdBeginDebugUtilsLabelEXT(*m_commandBuffer, &label);
    vk::CmdEndDebugUtilsLabelEXT(*m_commandBuffer);
    m_commandBuffer->end();

    m_default_queue->submit(*m_commandBuffer);
    m_default_queue->wait();
}

TEST_F(PositiveDebugExtensions, DebugLabelPrimaryCommandBuffer2) {
    TEST_DESCRIPTION("Test primary command buffer debug labels with multiple submits");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkDebugUtilsLabelEXT label = vku::InitStructHelper();
    label.pLabelName = "test";
    vkt::CommandBuffer cb0(*m_device, m_commandPool);
    cb0.begin();
    vk::CmdBeginDebugUtilsLabelEXT(cb0, &label);
    cb0.end();
    m_default_queue->submit(cb0);

    vkt::CommandBuffer cb1(*m_device, m_commandPool);
    cb1.begin();
    vk::CmdEndDebugUtilsLabelEXT(cb1);
    cb1.end();
    m_default_queue->submit(cb1);

    m_default_queue->wait();
}

TEST_F(PositiveDebugExtensions, DebugLabelPrimaryCommandBuffer3) {
    TEST_DESCRIPTION("Test primary command buffer debug labels with multiple submits");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkDebugUtilsLabelEXT label = vku::InitStructHelper();
    label.pLabelName = "test";
    vkt::CommandBuffer cb0(*m_device, m_commandPool);
    cb0.begin();
    vk::CmdBeginDebugUtilsLabelEXT(cb0, &label);
    cb0.end();

    vkt::CommandBuffer cb1(*m_device, m_commandPool);
    cb1.begin();
    vk::CmdEndDebugUtilsLabelEXT(cb1);
    cb1.end();

    m_default_queue->submit({&cb0, &cb1}, vkt::Fence{});
    m_default_queue->wait();
}

TEST_F(PositiveDebugExtensions, DebugLabelSecondaryCommandBuffer) {
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    vkt::CommandBuffer cb(*m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    cb.begin();
    {
        VkDebugUtilsLabelEXT label = vku::InitStructHelper();
        label.pLabelName = "test";
        vk::CmdBeginDebugUtilsLabelEXT(cb, &label);
        vk::CmdEndDebugUtilsLabelEXT(cb);
    }
    cb.end();
}
