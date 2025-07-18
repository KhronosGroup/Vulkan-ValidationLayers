/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

class NegativeDebugExtensions : public VkLayerTest {};

TEST_F(NegativeDebugExtensions, DebugMarkerName) {
    TEST_DESCRIPTION("Ensure debug marker object names are printed in debug report output");
    AddRequiredExtensions(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Skipping object naming test with MockICD.";
    }

    std::string memory_name = "memory_name";

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_create_info.size = 1;
    vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);

    VkMemoryRequirements memRequirements;
    vk::GetBufferMemoryRequirements(device(), buffer, &memRequirements);

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper();
    memory_allocate_info.allocationSize = memRequirements.size;
    memory_allocate_info.memoryTypeIndex = 0;

    vkt::DeviceMemory memory_1(*m_device, memory_allocate_info);
    vkt::DeviceMemory memory_2(*m_device, memory_allocate_info);
    VkDebugMarkerObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.object = (uint64_t)memory_2.handle();
    name_info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT;
    name_info.pObjectName = memory_name.c_str();
    vk::DebugMarkerSetObjectNameEXT(device(), &name_info);

    vk::BindBufferMemory(device(), buffer, memory_1, 0);

    // Test core_validation layer
    m_errorMonitor->SetDesiredError(memory_name.c_str());
    vk::BindBufferMemory(device(), buffer, memory_2, 0);
    m_errorMonitor->VerifyFound();

    VkCommandBuffer commandBuffer;
    std::string commandBuffer_name = "command_buffer_name";
    VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkt::CommandPool command_pool_1(*m_device, pool_create_info);
    vkt::CommandPool command_pool_2(*m_device, pool_create_info);

    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = command_pool_1;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(device(), &command_buffer_allocate_info, &commandBuffer);

    name_info.object = (uint64_t)commandBuffer;
    name_info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT;
    name_info.pObjectName = commandBuffer_name.c_str();
    vk::DebugMarkerSetObjectNameEXT(device(), &name_info);

    VkCommandBufferBeginInfo cb_begin_Info = vku::InitStructHelper();
    cb_begin_Info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vk::BeginCommandBuffer(commandBuffer, &cb_begin_Info);

    const VkRect2D scissor = {{-1, 0}, {16, 16}};
    const VkRect2D scissors[] = {scissor, scissor};

    // Test parameter_validation layer
    m_errorMonitor->SetDesiredError(commandBuffer_name.c_str());
    vk::CmdSetScissor(commandBuffer, 0, 1, scissors);
    m_errorMonitor->VerifyFound();

    // Test object_tracker layer
    m_errorMonitor->SetDesiredError(commandBuffer_name.c_str());
    vk::FreeCommandBuffers(device(), command_pool_2, 1, &commandBuffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugExtensions, DebugMarkerSetObject) {
    AddRequiredExtensions(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Skipping object naming test with MockICD.";
    }

    std::string memory_name = "memory_name";

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper();
    memory_allocate_info.allocationSize = 64;
    memory_allocate_info.memoryTypeIndex = 0;
    vkt::DeviceMemory memory(*m_device, memory_allocate_info);

    VkDebugMarkerObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.object = (uint64_t)VK_NULL_HANDLE;
    name_info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT;
    name_info.pObjectName = memory_name.c_str();
    m_errorMonitor->SetDesiredError("VUID-VkDebugMarkerObjectNameInfoEXT-object-01491");
    vk::DebugMarkerSetObjectNameEXT(device(), &name_info);
    m_errorMonitor->VerifyFound();

    name_info.object = (uint64_t)memory.handle();
    name_info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkDebugMarkerObjectNameInfoEXT-objectType-01490");
    vk::DebugMarkerSetObjectNameEXT(device(), &name_info);
    m_errorMonitor->VerifyFound();

    name_info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkDebugMarkerObjectNameInfoEXT-object-01492");
    vk::DebugMarkerSetObjectNameEXT(device(), &name_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugExtensions, DebugUtilsName) {
    TEST_DESCRIPTION("Ensure debug utils object names are printed in debug messenger output");

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

    std::string memory_name = "memory_name";

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_create_info.size = 1;
    vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);

    VkMemoryRequirements memRequirements;
    vk::GetBufferMemoryRequirements(device(), buffer, &memRequirements);

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper();
    memory_allocate_info.allocationSize = memRequirements.size;
    memory_allocate_info.memoryTypeIndex = 0;

    vkt::DeviceMemory memory_1(*m_device, memory_allocate_info);
    vkt::DeviceMemory memory_2(*m_device, memory_allocate_info);

    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_DEVICE_MEMORY;
    name_info.pObjectName = memory_name.c_str();

    // Pass in bad handle make sure ObjectTracker catches it
    m_errorMonitor->SetDesiredError("VUID-VkDebugUtilsObjectNameInfoEXT-objectType-02590");
    name_info.objectHandle = (uint64_t)0xcadecade;
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);
    m_errorMonitor->VerifyFound();

    // Pass in null handle
    m_errorMonitor->SetDesiredError("VUID-vkSetDebugUtilsObjectNameEXT-pNameInfo-02588");
    name_info.objectHandle = 0;
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);
    m_errorMonitor->VerifyFound();

    // Pass in 'unknown' object type and see if parameter validation catches it
    m_errorMonitor->SetDesiredError("VUID-vkSetDebugUtilsObjectNameEXT-pNameInfo-02587");
    name_info.objectHandle = (uint64_t)memory_2.handle();
    name_info.objectType = VK_OBJECT_TYPE_UNKNOWN;
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);
    m_errorMonitor->VerifyFound();

    name_info.objectType = VK_OBJECT_TYPE_DEVICE_MEMORY;
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    vk::BindBufferMemory(device(), buffer, memory_1, 0);

    // Test core_validation layer
    m_errorMonitor->SetDesiredError(memory_name.c_str());
    vk::BindBufferMemory(device(), buffer, memory_2, 0);
    m_errorMonitor->VerifyFound();

    VkCommandBuffer commandBuffer;
    std::string commandBuffer_name = "command_buffer_name";
    VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkt::CommandPool command_pool_1(*m_device, pool_create_info);
    vkt::CommandPool command_pool_2(*m_device, pool_create_info);

    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = command_pool_1;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(device(), &command_buffer_allocate_info, &commandBuffer);

    name_info.objectHandle = (uint64_t)commandBuffer;
    name_info.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
    name_info.pObjectName = commandBuffer_name.c_str();
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    VkCommandBufferBeginInfo cb_begin_Info = vku::InitStructHelper();
    cb_begin_Info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vk::BeginCommandBuffer(commandBuffer, &cb_begin_Info);

    const VkRect2D scissor = {{-1, 0}, {16, 16}};
    const VkRect2D scissors[] = {scissor, scissor};

    VkDebugUtilsLabelEXT command_label = vku::InitStructHelper();
    command_label.pLabelName = "Command Label 0123";
    command_label.color[0] = 0.;
    command_label.color[1] = 1.;
    command_label.color[2] = 2.;
    command_label.color[3] = 3.0;
    bool command_label_test = false;
    auto command_label_callback = [command_label, &command_label_test](const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                                       DebugUtilsLabelCheckData *data) {
        data->count++;
        command_label_test = false;
        if (pCallbackData->cmdBufLabelCount == 1) {
            command_label_test = pCallbackData->pCmdBufLabels[0] == command_label;
        }
    };
    callback_data.callback = command_label_callback;

    vk::CmdInsertDebugUtilsLabelEXT(commandBuffer, &command_label);
    // Test parameter_validation layer
    m_errorMonitor->SetDesiredError(commandBuffer_name.c_str());
    vk::CmdSetScissor(commandBuffer, 0, 1, scissors);
    m_errorMonitor->VerifyFound();

    // Check the label test
    if (!command_label_test) {
        ADD_FAILURE() << "Command label '" << command_label.pLabelName << "' not passed to callback.";
    }

    // Test object_tracker layer
    m_errorMonitor->SetDesiredError(commandBuffer_name.c_str());
    vk::FreeCommandBuffers(device(), command_pool_2, 1, &commandBuffer);
    m_errorMonitor->VerifyFound();

    vk::DestroyDebugUtilsMessengerEXT(instance(), my_messenger, nullptr);
}

TEST_F(NegativeDebugExtensions, DebugMarkerSetUtils) {
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Skipping object naming test with MockICD.";
    }

    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper();
    memory_allocate_info.allocationSize = 64;
    memory_allocate_info.memoryTypeIndex = 0;
    vkt::DeviceMemory memory(*m_device, memory_allocate_info);

    int tags[3] = {1, 2, 3};
    VkDebugMarkerObjectTagInfoEXT name_info = vku::InitStructHelper();
    name_info.object = (uint64_t)VK_NULL_HANDLE;
    name_info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT;
    name_info.tagName = 1;
    name_info.tagSize = 4;
    name_info.pTag = tags;
    m_errorMonitor->SetDesiredError("VUID-VkDebugMarkerObjectTagInfoEXT-object-01494");
    vk::DebugMarkerSetObjectTagEXT(device(), &name_info);
    m_errorMonitor->VerifyFound();

    name_info.object = (uint64_t)memory.handle();
    name_info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkDebugMarkerObjectTagInfoEXT-objectType-01493");
    vk::DebugMarkerSetObjectTagEXT(device(), &name_info);
    m_errorMonitor->VerifyFound();

    name_info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT;
    m_errorMonitor->SetDesiredError("VUID-VkDebugMarkerObjectTagInfoEXT-object-01495");
    vk::DebugMarkerSetObjectTagEXT(device(), &name_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugExtensions, DebugUtilsParameterFlags) {
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    DebugUtilsLabelCheckData callback_data;
    auto empty_callback = [](const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, DebugUtilsLabelCheckData *data) {
        data->count++;
    };
    callback_data.count = 0;
    callback_data.callback = empty_callback;

    VkDebugUtilsMessengerCreateInfoEXT callback_create_info = vku::InitStructHelper();
    callback_create_info.messageSeverity = 0;
    callback_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    callback_create_info.pfnUserCallback = DebugUtilsCallback;
    callback_create_info.pUserData = &callback_data;
    VkDebugUtilsMessengerEXT my_messenger = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-VkDebugUtilsMessengerCreateInfoEXT-messageSeverity-requiredbitmask");
    vk::CreateDebugUtilsMessengerEXT(instance(), &callback_create_info, nullptr, &my_messenger);
    m_errorMonitor->VerifyFound();
}

struct LayerStatusCheckData {
    std::function<void(const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, LayerStatusCheckData *)> callback;
    ErrorMonitor *error_monitor;
};

TEST_F(NegativeDebugExtensions, LayerInfoMessages) {
    TEST_DESCRIPTION("Ensure layer prints startup status messages.");

    auto ici = GetInstanceCreateInfo();
    LayerStatusCheckData callback_data;
    auto local_callback = [](const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, LayerStatusCheckData *data) {
        std::string message(pCallbackData->pMessage);
        if ((data->error_monitor->GetMessageFlags() & kInformationBit) &&
            (message.find("UNASSIGNED-khronos-validation-createinstance-status-message") == std::string::npos)) {
            data->error_monitor->SetError("UNASSIGNED-Khronos-validation-createinstance-status-message-not-found");
        } else if ((data->error_monitor->GetMessageFlags() & kPerformanceWarningBit) &&
                   (message.find("UNASSIGNED-khronos-Validation-debug-build-warning-message") == std::string::npos)) {
            data->error_monitor->SetError("UNASSIGNED-khronos-validation-createinstance-debug-warning-message-not-found");
        }
    };
    callback_data.error_monitor = m_errorMonitor;
    callback_data.callback = local_callback;

    VkInstance local_instance;

    VkDebugUtilsMessengerCreateInfoEXT callback_create_info = vku::InitStructHelper();
    callback_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    callback_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    callback_create_info.pfnUserCallback = DebugUtilsCallback;
    callback_create_info.pUserData = &callback_data;
    ici.pNext = &callback_create_info;

    // Create an instance, error if layer status INFO message not found
    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&ici, nullptr, &local_instance));
    vk::DestroyInstance(local_instance, nullptr);

#ifndef NDEBUG
    // Create an instance, error if layer DEBUG_BUILD warning message not found
    callback_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    callback_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&ici, nullptr, &local_instance));
    vk::DestroyInstance(local_instance, nullptr);
#endif
}

TEST_F(NegativeDebugExtensions, SetDebugUtilsObjectSecondDevice) {
    TEST_DESCRIPTION("call vkSetDebugUtilsObjectNameEXT with a different VkDevice object");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Skipping object naming test with MockICD.";
    }

    auto features = m_device->Physical().Features();
    vkt::Device second_device(gpu_, m_device_extension_names, &features);

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

    const char *object_name = "device_object";

    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_DEVICE;
    name_info.objectHandle = (uint64_t)second_device.handle();
    name_info.pObjectName = object_name;
    m_errorMonitor->SetDesiredError("VUID-vkSetDebugUtilsObjectNameEXT-pNameInfo-07874");
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);
    m_errorMonitor->VerifyFound();

    vk::DestroyDebugUtilsMessengerEXT(instance(), my_messenger, nullptr);
}

TEST_F(NegativeDebugExtensions, SetDebugUtilsObjectDestroyedHandle) {
    TEST_DESCRIPTION("call vkSetDebugUtilsObjectNameEXT on a VkBuffer that is destroyed");
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

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    uint64_t bad_handle = (uint64_t)sampler.handle();
    sampler.Destroy();
    const char *object_name = "sampler_object";

    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_SAMPLER;
    name_info.objectHandle = bad_handle;
    name_info.pObjectName = object_name;
    m_errorMonitor->SetDesiredError("VUID-VkDebugUtilsObjectNameInfoEXT-objectType-02590");
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);
    m_errorMonitor->VerifyFound();

    vk::DestroyDebugUtilsMessengerEXT(instance(), my_messenger, nullptr);
}

TEST_F(NegativeDebugExtensions, DebugLabelPrimaryCommandBuffer) {
    TEST_DESCRIPTION("Test primary command buffer debug labels which are validated at submit time.");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    m_command_buffer.Begin();
    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEndDebugUtilsLabelEXT-commandBuffer-01912");
    m_default_queue->Submit(m_command_buffer);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeDebugExtensions, DebugLabelPrimaryCommandBuffer2) {
    TEST_DESCRIPTION("Test primary command buffer debug labels which are validated at submit time.");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkDebugUtilsLabelEXT label = vku::InitStructHelper();
    label.pLabelName = "regionA";
    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    cb0.Begin();
    vk::CmdBeginDebugUtilsLabelEXT(cb0, &label);
    cb0.End();
    m_default_queue->Submit(cb0);

    vkt::CommandBuffer cb1(*m_device, m_command_pool);
    cb1.Begin();
    vk::CmdEndDebugUtilsLabelEXT(cb1);
    vk::CmdEndDebugUtilsLabelEXT(cb1);
    cb1.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndDebugUtilsLabelEXT-commandBuffer-01912");
    m_default_queue->Submit(cb1);
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeDebugExtensions, DebugLabelPrimaryCommandBuffer3) {
    TEST_DESCRIPTION("Test primary command buffer debug labels which are validated at submit time.");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkDebugUtilsLabelEXT label = vku::InitStructHelper();
    label.pLabelName = "regionA";
    vkt::CommandBuffer cb0(*m_device, m_command_pool);
    cb0.Begin();
    vk::CmdBeginDebugUtilsLabelEXT(cb0, &label);
    vk::CmdEndDebugUtilsLabelEXT(cb0);
    cb0.End();

    vkt::CommandBuffer cb1(*m_device, m_command_pool);
    label.pLabelName = "regionB";
    cb1.Begin();
    vk::CmdBeginDebugUtilsLabelEXT(cb1, &label);
    vk::CmdEndDebugUtilsLabelEXT(cb1);
    vk::CmdEndDebugUtilsLabelEXT(cb1);
    cb1.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndDebugUtilsLabelEXT-commandBuffer-01912");
    m_default_queue->Submit({cb0, cb1});
    m_errorMonitor->VerifyFound();
    m_default_queue->Wait();
}

TEST_F(NegativeDebugExtensions, DebugLabelSecondaryCommandBuffer) {
    TEST_DESCRIPTION("Test secondary command buffer debug labels which are validated at recording time.");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    vkt::CommandBuffer cb(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndDebugUtilsLabelEXT-commandBuffer-01913");
    vk::CmdEndDebugUtilsLabelEXT(cb);
    m_errorMonitor->VerifyFound();
    cb.End();
}

TEST_F(NegativeDebugExtensions, SwapchainImagesDebugMarker) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7977");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    AddSurfaceExtension();
    RETURN_IF_SKIP(Init());
    RETURN_IF_SKIP(InitSurface());

    SurfaceInformation info = GetSwapchainInfo(m_surface.Handle());
    InitSwapchainInfo();

    VkSwapchainCreateInfoKHR swapchain_create_info = vku::InitStructHelper();
    swapchain_create_info.surface = m_surface.Handle();
    swapchain_create_info.minImageCount = info.surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = info.surface_formats[0].format;
    swapchain_create_info.imageColorSpace = info.surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = info.surface_capabilities.minImageExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = info.surface_composite_alpha;
    swapchain_create_info.presentMode = info.surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

    vkt::Swapchain swapchain(*m_device, swapchain_create_info);
    const auto images = swapchain.GetImages();

    {
        VkDebugMarkerObjectNameInfoEXT name_info = vku::InitStructHelper();
        name_info.object = (uint64_t)images[0];
        name_info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT;
        std::string image_name = "swapchain [0]";
        name_info.pObjectName = image_name.c_str();
        m_errorMonitor->SetDesiredError("VUID-VkDebugMarkerObjectNameInfoEXT-object-01492");
        vk::DebugMarkerSetObjectNameEXT(device(), &name_info);
        m_errorMonitor->VerifyFound();
    }

    {
        int tags[2] = {1, 2};
        VkDebugMarkerObjectTagInfoEXT name_info = vku::InitStructHelper();
        name_info.object = (uint64_t)images[0];
        name_info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT;
        name_info.tagName = 1;
        name_info.tagSize = 2;
        name_info.pTag = tags;
        m_errorMonitor->SetDesiredError("VUID-VkDebugMarkerObjectTagInfoEXT-object-01495");
        vk::DebugMarkerSetObjectTagEXT(device(), &name_info);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDebugExtensions, MultiObjectBindImage) {
    TEST_DESCRIPTION("Make sure both VkDeviceMemory and VkImage are displayed in error message");
    RETURN_IF_SKIP(Init());

    // Create an image, allocate memory, free it, and then try to bind it
    VkImageCreateInfo image_create_info =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);
    VkMemoryRequirements mem_reqs = image.MemoryRequirements();

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    // Introduce failure, do NOT set memProps to
    // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    mem_alloc.memoryTypeIndex = 1;
    mem_alloc.allocationSize = mem_reqs.size;
    ASSERT_TRUE(m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &mem_alloc, 0));

    vkt::DeviceMemory mem1(*m_device, mem_alloc);
    vkt::DeviceMemory mem2(*m_device, mem_alloc);

    // Bind first memory object to Image object
    vk::BindImageMemory(device(), image, mem1, 0);

    // Introduce validation failure, try to bind a different memory object to
    // the same image object
    m_errorMonitor->SetDesiredErrorRegex("VUID-vkBindImageMemory-image-07460", "Objects: 3");
    vk::BindImageMemory(device(), image, mem2, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugExtensions, SetDeviceHandle) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9780");
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Create second device
    float priorities[] = {1.0f};
    VkDeviceQueueCreateInfo queue_info = vku::InitStructHelper();
    queue_info.flags = 0;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priorities[0];

    VkDeviceCreateInfo device_create_info = vku::InitStructHelper();
    auto features = m_device->Physical().Features();
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_info;
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = nullptr;
    device_create_info.pEnabledFeatures = &features;

    VkDevice second_device;
    ASSERT_EQ(VK_SUCCESS, vk::CreateDevice(Gpu(), &device_create_info, nullptr, &second_device));

    const char *device_1_name = "device_1";
    const char *device_2_name = "device_2";
    VkDebugUtilsObjectNameInfoEXT name_info = vku::InitStructHelper();
    name_info.objectType = VK_OBJECT_TYPE_DEVICE;
    name_info.pObjectName = device_1_name;
    name_info.objectHandle = (uint64_t)device();
    vk::SetDebugUtilsObjectNameEXT(device(), &name_info);

    name_info.pObjectName = device_2_name;
    name_info.objectHandle = (uint64_t)second_device;
    vk::SetDebugUtilsObjectNameEXT(second_device, &name_info);

    VkBufferCreateInfo create_info = vkt::Buffer::CreateInfo(32, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    VkBuffer buffer = VK_NULL_HANDLE;
    vk::CreateBuffer(device(), &create_info, nullptr, &buffer);

    m_errorMonitor->SetDesiredErrorRegex("VUID-vkDestroyBuffer-buffer-parent",
                                         "was created, allocated or retrieved from VkDevice .*\\[device_1\\], but command is using "
                                         "\\(or its dispatchable parameter is associated with\\) VkDevice .*\\[device_2\\]");
    vk::DestroyBuffer(second_device, buffer, nullptr);
    m_errorMonitor->VerifyFound();

    vk::DestroyBuffer(device(), buffer, nullptr);
    vk::DestroyDevice(second_device, nullptr);
}

TEST_F(NegativeDebugExtensions, DebugMarkerRecording) {
    AddRequiredExtensions(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkDebugMarkerMarkerInfoEXT marker_info = vku::InitStructHelper();
    marker_info.pMarkerName = "test";
    m_errorMonitor->SetDesiredError("VUID-vkCmdDebugMarkerBeginEXT-commandBuffer-recording");
    vk::CmdDebugMarkerBeginEXT(m_command_buffer, &marker_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDebugMarkerEndEXT-commandBuffer-recording");
    vk::CmdDebugMarkerEndEXT(m_command_buffer);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDebugMarkerInsertEXT-commandBuffer-recording");
    vk::CmdDebugMarkerInsertEXT(m_command_buffer, &marker_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDebugExtensions, DebugUtilsRecording) {
    AddRequiredExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkDebugUtilsLabelEXT label = vku::InitStructHelper();
    label.pLabelName = "test";
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginDebugUtilsLabelEXT-commandBuffer-recording");
    vk::CmdBeginDebugUtilsLabelEXT(m_command_buffer, &label);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEndDebugUtilsLabelEXT-commandBuffer-recording");
    vk::CmdEndDebugUtilsLabelEXT(m_command_buffer);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdInsertDebugUtilsLabelEXT-commandBuffer-recording");
    vk::CmdInsertDebugUtilsLabelEXT(m_command_buffer, &label);
    m_errorMonitor->VerifyFound();
}
