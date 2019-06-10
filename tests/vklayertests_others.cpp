/*
 * Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (c) 2015-2019 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Chia-I Wu <olvaffe@gmail.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Mike Stroyan <mike@LunarG.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: Cody Northrop <cnorthrop@google.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: Jeremy Kniager <jeremyk@lunarg.com>
 * Author: Shannon McPherson <shannon@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 */

#include "cast_utils.h"
#include "layer_validation_tests.h"

TEST_F(VkLayerTest, RequiredParameter) {
    TEST_DESCRIPTION("Specify VK_NULL_HANDLE, NULL, and 0 for required handle, pointer, array, and array count parameters");

    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "required parameter pFeatures specified as NULL");
    // Specify NULL for a pointer to a handle
    // Expected to trigger an error with
    // parameter_validation::validate_required_pointer
    vkGetPhysicalDeviceFeatures(gpu(), NULL);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "required parameter pQueueFamilyPropertyCount specified as NULL");
    // Specify NULL for pointer to array count
    // Expected to trigger an error with parameter_validation::validate_array
    vkGetPhysicalDeviceQueueFamilyProperties(gpu(), NULL, NULL);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdSetViewport-viewportCount-arraylength");
    // Specify 0 for a required array count
    // Expected to trigger an error with parameter_validation::validate_array
    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    m_commandBuffer->SetViewport(0, 0, &viewport);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCreateImage-pCreateInfo-parameter");
    // Specify a null pImageCreateInfo struct pointer
    VkImage test_image;
    vkCreateImage(device(), NULL, NULL, &test_image);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdSetViewport-pViewports-parameter");
    // Specify NULL for a required array
    // Expected to trigger an error with parameter_validation::validate_array
    m_commandBuffer->SetViewport(0, 1, NULL);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "required parameter memory specified as VK_NULL_HANDLE");
    // Specify VK_NULL_HANDLE for a required handle
    // Expected to trigger an error with
    // parameter_validation::validate_required_handle
    vkUnmapMemory(device(), VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "required parameter pFences[0] specified as VK_NULL_HANDLE");
    // Specify VK_NULL_HANDLE for a required handle array entry
    // Expected to trigger an error with
    // parameter_validation::validate_required_handle_array
    VkFence fence = VK_NULL_HANDLE;
    vkResetFences(device(), 1, &fence);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "required parameter pAllocateInfo specified as NULL");
    // Specify NULL for a required struct pointer
    // Expected to trigger an error with
    // parameter_validation::validate_struct_type
    VkDeviceMemory memory = VK_NULL_HANDLE;
    vkAllocateMemory(device(), NULL, NULL, &memory);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "value of faceMask must not be 0");
    // Specify 0 for a required VkFlags parameter
    // Expected to trigger an error with parameter_validation::validate_flags
    m_commandBuffer->SetStencilReference(0, 0);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "value of pSubmits[0].pWaitDstStageMask[0] must not be 0");
    // Specify 0 for a required VkFlags array entry
    // Expected to trigger an error with
    // parameter_validation::validate_flags_array
    VkSemaphore semaphore = VK_NULL_HANDLE;
    VkPipelineStageFlags stageFlags = 0;
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &semaphore;
    submitInfo.pWaitDstStageMask = &stageFlags;
    vkQueueSubmit(m_device->m_queue, 1, &submitInfo, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkSubmitInfo-sType-sType");
    stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    // Set a bogus sType and see what happens
    submitInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &semaphore;
    submitInfo.pWaitDstStageMask = &stageFlags;
    vkQueueSubmit(m_device->m_queue, 1, &submitInfo, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkSubmitInfo-pWaitSemaphores-parameter");
    stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    // Set a null pointer for pWaitSemaphores
    submitInfo.pWaitSemaphores = NULL;
    submitInfo.pWaitDstStageMask = &stageFlags;
    vkQueueSubmit(m_device->m_queue, 1, &submitInfo, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, PnextOnlyStructValidation) {
    TEST_DESCRIPTION("See if checks occur on structs ONLY used in pnext chains.");

    if (!(CheckDescriptorIndexingSupportAndInitFramework(this, m_instance_extension_names, m_device_extension_names, NULL,
                                                         m_errorMonitor))) {
        printf("Descriptor indexing or one of its dependencies not supported, skipping tests\n");
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device passing in a bad PdevFeatures2 value
    auto indexing_features = lvl_init_struct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&indexing_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    // Set one of the features values to an invalid boolean value
    indexing_features.descriptorBindingUniformBufferUpdateAfterBind = 800;

    uint32_t queue_node_count;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_node_count, NULL);
    VkQueueFamilyProperties *queue_props = new VkQueueFamilyProperties[queue_node_count];
    vkGetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_node_count, queue_props);
    float priorities[] = {1.0f};
    VkDeviceQueueCreateInfo queue_info{};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.pNext = NULL;
    queue_info.flags = 0;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priorities[0];
    VkDeviceCreateInfo dev_info = {};
    dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_info.pNext = NULL;
    dev_info.queueCreateInfoCount = 1;
    dev_info.pQueueCreateInfos = &queue_info;
    dev_info.enabledLayerCount = 0;
    dev_info.ppEnabledLayerNames = NULL;
    dev_info.enabledExtensionCount = m_device_extension_names.size();
    dev_info.ppEnabledExtensionNames = m_device_extension_names.data();
    dev_info.pNext = &features2;
    VkDevice dev;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_WARNING_BIT_EXT, "is neither VK_TRUE nor VK_FALSE");
    m_errorMonitor->SetUnexpectedError("Failed to create");
    vkCreateDevice(gpu(), &dev_info, NULL, &dev);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ReservedParameter) {
    TEST_DESCRIPTION("Specify a non-zero value for a reserved parameter");

    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, " must be 0");
    // Specify 0 for a reserved VkFlags parameter
    // Expected to trigger an error with
    // parameter_validation::validate_reserved_flags
    VkEvent event_handle = VK_NULL_HANDLE;
    VkEventCreateInfo event_info = {};
    event_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    event_info.flags = 1;
    vkCreateEvent(device(), &event_info, NULL, &event_handle);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DebugMarkerNameTest) {
    TEST_DESCRIPTION("Ensure debug marker object names are printed in debug report output");

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), "VK_LAYER_LUNARG_core_validation", VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    } else {
        printf("%s Debug Marker Extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkDebugMarkerSetObjectNameEXT fpvkDebugMarkerSetObjectNameEXT =
        (PFN_vkDebugMarkerSetObjectNameEXT)vkGetInstanceProcAddr(instance(), "vkDebugMarkerSetObjectNameEXT");
    if (!(fpvkDebugMarkerSetObjectNameEXT)) {
        printf("%s Can't find fpvkDebugMarkerSetObjectNameEXT; skipped.\n", kSkipPrefix);
        return;
    }

    if (DeviceSimulation()) {
        printf("%sSkipping object naming test.\n", kSkipPrefix);
        return;
    }

    VkBuffer buffer;
    VkDeviceMemory memory_1, memory_2;
    std::string memory_name = "memory_name";

    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_create_info.size = 1;

    vkCreateBuffer(device(), &buffer_create_info, nullptr, &buffer);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device(), buffer, &memRequirements);

    VkMemoryAllocateInfo memory_allocate_info = {};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memRequirements.size;
    memory_allocate_info.memoryTypeIndex = 0;

    vkAllocateMemory(device(), &memory_allocate_info, nullptr, &memory_1);
    vkAllocateMemory(device(), &memory_allocate_info, nullptr, &memory_2);

    VkDebugMarkerObjectNameInfoEXT name_info = {};
    name_info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
    name_info.pNext = nullptr;
    name_info.object = (uint64_t)memory_2;
    name_info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT;
    name_info.pObjectName = memory_name.c_str();
    fpvkDebugMarkerSetObjectNameEXT(device(), &name_info);

    vkBindBufferMemory(device(), buffer, memory_1, 0);

    // Test core_validation layer
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, memory_name);
    vkBindBufferMemory(device(), buffer, memory_2, 0);
    m_errorMonitor->VerifyFound();

    vkFreeMemory(device(), memory_1, nullptr);
    memory_1 = VK_NULL_HANDLE;
    vkFreeMemory(device(), memory_2, nullptr);
    memory_2 = VK_NULL_HANDLE;
    vkDestroyBuffer(device(), buffer, nullptr);
    buffer = VK_NULL_HANDLE;

    VkCommandBuffer commandBuffer;
    std::string commandBuffer_name = "command_buffer_name";
    VkCommandPool commandpool_1;
    VkCommandPool commandpool_2;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(device(), &pool_create_info, nullptr, &commandpool_1);
    vkCreateCommandPool(device(), &pool_create_info, nullptr, &commandpool_2);

    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = commandpool_1;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(device(), &command_buffer_allocate_info, &commandBuffer);

    name_info.object = (uint64_t)commandBuffer;
    name_info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT;
    name_info.pObjectName = commandBuffer_name.c_str();
    fpvkDebugMarkerSetObjectNameEXT(device(), &name_info);

    VkCommandBufferBeginInfo cb_begin_Info = {};
    cb_begin_Info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cb_begin_Info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &cb_begin_Info);

    const VkRect2D scissor = {{-1, 0}, {16, 16}};
    const VkRect2D scissors[] = {scissor, scissor};

    // Test parameter_validation layer
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, commandBuffer_name);
    vkCmdSetScissor(commandBuffer, 1, 1, scissors);
    m_errorMonitor->VerifyFound();

    // Test object_tracker layer
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, commandBuffer_name);
    vkFreeCommandBuffers(device(), commandpool_2, 1, &commandBuffer);
    m_errorMonitor->VerifyFound();

    vkDestroyCommandPool(device(), commandpool_1, NULL);
    vkDestroyCommandPool(device(), commandpool_2, NULL);
}

TEST_F(VkLayerTest, DebugUtilsNameTest) {
    TEST_DESCRIPTION("Ensure debug utils object names are printed in debug messenger output");

    // Skip test if extension not supported
    if (InstanceExtensionSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    } else {
        printf("%s Debug Utils Extension not supported, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkSetDebugUtilsObjectNameEXT fpvkSetDebugUtilsObjectNameEXT =
        (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance(), "vkSetDebugUtilsObjectNameEXT");
    ASSERT_TRUE(fpvkSetDebugUtilsObjectNameEXT);  // Must be extant if extension is enabled
    PFN_vkCreateDebugUtilsMessengerEXT fpvkCreateDebugUtilsMessengerEXT =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance(), "vkCreateDebugUtilsMessengerEXT");
    ASSERT_TRUE(fpvkCreateDebugUtilsMessengerEXT);  // Must be extant if extension is enabled
    PFN_vkDestroyDebugUtilsMessengerEXT fpvkDestroyDebugUtilsMessengerEXT =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance(), "vkDestroyDebugUtilsMessengerEXT");
    ASSERT_TRUE(fpvkDestroyDebugUtilsMessengerEXT);  // Must be extant if extension is enabled
    PFN_vkCmdInsertDebugUtilsLabelEXT fpvkCmdInsertDebugUtilsLabelEXT =
        (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance(), "vkCmdInsertDebugUtilsLabelEXT");
    ASSERT_TRUE(fpvkCmdInsertDebugUtilsLabelEXT);  // Must be extant if extension is enabled

    if (DeviceSimulation()) {
        printf("%sSkipping object naming test.\n", kSkipPrefix);
        return;
    }

    DebugUtilsLabelCheckData callback_data;
    auto empty_callback = [](const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, DebugUtilsLabelCheckData *data) {
        data->count++;
    };
    callback_data.count = 0;
    callback_data.callback = empty_callback;

    auto callback_create_info = lvl_init_struct<VkDebugUtilsMessengerCreateInfoEXT>();
    callback_create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    callback_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    callback_create_info.pfnUserCallback = DebugUtilsCallback;
    callback_create_info.pUserData = &callback_data;
    VkDebugUtilsMessengerEXT my_messenger = VK_NULL_HANDLE;
    fpvkCreateDebugUtilsMessengerEXT(instance(), &callback_create_info, nullptr, &my_messenger);

    VkBuffer buffer;
    VkDeviceMemory memory_1, memory_2;
    std::string memory_name = "memory_name";

    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_create_info.size = 1;

    vkCreateBuffer(device(), &buffer_create_info, nullptr, &buffer);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device(), buffer, &memRequirements);

    VkMemoryAllocateInfo memory_allocate_info = {};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memRequirements.size;
    memory_allocate_info.memoryTypeIndex = 0;

    vkAllocateMemory(device(), &memory_allocate_info, nullptr, &memory_1);
    vkAllocateMemory(device(), &memory_allocate_info, nullptr, &memory_2);

    VkDebugUtilsObjectNameInfoEXT name_info = {};
    name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    name_info.pNext = nullptr;
    name_info.objectHandle = (uint64_t)memory_2;
    name_info.objectType = VK_OBJECT_TYPE_DEVICE_MEMORY;
    name_info.pObjectName = memory_name.c_str();
    fpvkSetDebugUtilsObjectNameEXT(device(), &name_info);

    vkBindBufferMemory(device(), buffer, memory_1, 0);

    // Test core_validation layer
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, memory_name);
    vkBindBufferMemory(device(), buffer, memory_2, 0);
    m_errorMonitor->VerifyFound();

    vkFreeMemory(device(), memory_1, nullptr);
    memory_1 = VK_NULL_HANDLE;
    vkFreeMemory(device(), memory_2, nullptr);
    memory_2 = VK_NULL_HANDLE;
    vkDestroyBuffer(device(), buffer, nullptr);
    buffer = VK_NULL_HANDLE;

    VkCommandBuffer commandBuffer;
    std::string commandBuffer_name = "command_buffer_name";
    VkCommandPool commandpool_1;
    VkCommandPool commandpool_2;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(device(), &pool_create_info, nullptr, &commandpool_1);
    vkCreateCommandPool(device(), &pool_create_info, nullptr, &commandpool_2);

    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = commandpool_1;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(device(), &command_buffer_allocate_info, &commandBuffer);

    name_info.objectHandle = (uint64_t)commandBuffer;
    name_info.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
    name_info.pObjectName = commandBuffer_name.c_str();
    fpvkSetDebugUtilsObjectNameEXT(device(), &name_info);

    VkCommandBufferBeginInfo cb_begin_Info = {};
    cb_begin_Info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cb_begin_Info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &cb_begin_Info);

    const VkRect2D scissor = {{-1, 0}, {16, 16}};
    const VkRect2D scissors[] = {scissor, scissor};

    auto command_label = lvl_init_struct<VkDebugUtilsLabelEXT>();
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

    fpvkCmdInsertDebugUtilsLabelEXT(commandBuffer, &command_label);
    // Test parameter_validation layer
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, commandBuffer_name);
    vkCmdSetScissor(commandBuffer, 1, 1, scissors);
    m_errorMonitor->VerifyFound();

    // Check the label test
    if (!command_label_test) {
        ADD_FAILURE() << "Command label '" << command_label.pLabelName << "' not passed to callback.";
    }

    // Test object_tracker layer
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, commandBuffer_name);
    vkFreeCommandBuffers(device(), commandpool_2, 1, &commandBuffer);
    m_errorMonitor->VerifyFound();

    vkDestroyCommandPool(device(), commandpool_1, NULL);
    vkDestroyCommandPool(device(), commandpool_2, NULL);
    fpvkDestroyDebugUtilsMessengerEXT(instance(), my_messenger, nullptr);
}

TEST_F(VkLayerTest, InvalidStructSType) {
    TEST_DESCRIPTION("Specify an invalid VkStructureType for a Vulkan structure's sType field");

    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "parameter pAllocateInfo->sType must be");
    // Zero struct memory, effectively setting sType to
    // VK_STRUCTURE_TYPE_APPLICATION_INFO
    // Expected to trigger an error with
    // parameter_validation::validate_struct_type
    VkMemoryAllocateInfo alloc_info = {};
    VkDeviceMemory memory = VK_NULL_HANDLE;
    vkAllocateMemory(device(), &alloc_info, NULL, &memory);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "parameter pSubmits[0].sType must be");
    // Zero struct memory, effectively setting sType to
    // VK_STRUCTURE_TYPE_APPLICATION_INFO
    // Expected to trigger an error with
    // parameter_validation::validate_struct_type_array
    VkSubmitInfo submit_info = {};
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidStructPNext) {
    TEST_DESCRIPTION("Specify an invalid value for a Vulkan structure's pNext field");

    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_WARNING_BIT_EXT, "value of pCreateInfo->pNext must be NULL");
    // Set VkMemoryAllocateInfo::pNext to a non-NULL value, when pNext must be NULL.
    // Need to pick a function that has no allowed pNext structure types.
    // Expected to trigger an error with parameter_validation::validate_struct_pnext
    VkEvent event = VK_NULL_HANDLE;
    VkEventCreateInfo event_alloc_info = {};
    // Zero-initialization will provide the correct sType
    VkApplicationInfo app_info = {};
    event_alloc_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    event_alloc_info.pNext = &app_info;
    vkCreateEvent(device(), &event_alloc_info, NULL, &event);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_WARNING_BIT_EXT,
                                         " chain includes a structure with unexpected VkStructureType ");
    // Set VkMemoryAllocateInfo::pNext to a non-NULL value, but use
    // a function that has allowed pNext structure types and specify
    // a structure type that is not allowed.
    // Expected to trigger an error with parameter_validation::validate_struct_pnext
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkMemoryAllocateInfo memory_alloc_info = {};
    memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_alloc_info.pNext = &app_info;
    vkAllocateMemory(device(), &memory_alloc_info, NULL, &memory);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UnrecognizedValueOutOfRange) {
    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "does not fall within the begin..end range of the core VkFormat enumeration tokens");
    // Specify an invalid VkFormat value
    // Expected to trigger an error with
    // parameter_validation::validate_ranged_enum
    VkFormatProperties format_properties;
    vkGetPhysicalDeviceFormatProperties(gpu(), static_cast<VkFormat>(8000), &format_properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UnrecognizedValueBadMask) {
    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "contains flag bits that are not recognized members of");
    // Specify an invalid VkFlags bitmask value
    // Expected to trigger an error with parameter_validation::validate_flags
    VkImageFormatProperties image_format_properties;
    vkGetPhysicalDeviceImageFormatProperties(gpu(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                             static_cast<VkImageUsageFlags>(1 << 25), 0, &image_format_properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UnrecognizedValueBadFlag) {
    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "contains flag bits that are not recognized members of");
    // Specify an invalid VkFlags array entry
    // Expected to trigger an error with parameter_validation::validate_flags_array
    VkSemaphore semaphore;
    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore);
    // `stage_flags` is set to a value which, currently, is not a defined stage flag
    // `VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM` works well for this
    VkPipelineStageFlags stage_flags = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
    // `waitSemaphoreCount` *must* be greater than 0 to perform this check
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &semaphore;
    submit_info.pWaitDstStageMask = &stage_flags;
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkDestroySemaphore(m_device->device(), semaphore, nullptr);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UnrecognizedValueBadBool) {
    // Make sure using VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE doesn't trigger a false positive.
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
    } else {
        printf("%s VK_KHR_sampler_mirror_clamp_to_edge extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_WARNING_BIT_EXT, "is neither VK_TRUE nor VK_FALSE");
    // Specify an invalid VkBool32 value, expecting a warning with parameter_validation::validate_bool32
    VkSampler sampler = VK_NULL_HANDLE;
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;

    // Not VK_TRUE or VK_FALSE
    sampler_info.anisotropyEnable = 3;
    vkCreateSampler(m_device->device(), &sampler_info, NULL, &sampler);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UnrecognizedValueMaxEnum) {
    ASSERT_NO_FATAL_FAILURE(Init());

    // Specify MAX_ENUM
    VkFormatProperties format_properties;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "does not fall within the begin..end range");
    vkGetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_MAX_ENUM, &format_properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, SubmitSignaledFence) {
    vk_testing::Fence testFence;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "submitted in SIGNALED state.  Fences must be reset before being submitted");

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = NULL;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();
    m_commandBuffer->ClearAllBuffers(m_renderTargets, m_clear_color, nullptr, m_depth_clear_color, m_stencil_clear_color);
    m_commandBuffer->end();

    testFence.init(*m_device, fenceInfo);

    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = NULL;
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = NULL;
    submit_info.pWaitDstStageMask = NULL;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = NULL;

    vkQueueSubmit(m_device->m_queue, 1, &submit_info, testFence.handle());
    vkQueueWaitIdle(m_device->m_queue);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, LeakAnObject) {
    TEST_DESCRIPTION("Create a fence and destroy its device without first destroying the fence.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    // Workaround for overzealous layers checking even the guaranteed 0th queue family
    const auto q_props = vk_testing::PhysicalDevice(gpu()).queue_properties();
    ASSERT_TRUE(q_props.size() > 0);
    ASSERT_TRUE(q_props[0].queueCount > 0);

    const float q_priority[] = {1.0f};
    VkDeviceQueueCreateInfo queue_ci = {};
    queue_ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_ci.queueFamilyIndex = 0;
    queue_ci.queueCount = 1;
    queue_ci.pQueuePriorities = q_priority;

    VkDeviceCreateInfo device_ci = {};
    device_ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &queue_ci;

    VkDevice leaky_device;
    ASSERT_VK_SUCCESS(vkCreateDevice(gpu(), &device_ci, nullptr, &leaky_device));

    const VkFenceCreateInfo fence_ci = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    VkFence leaked_fence;
    ASSERT_VK_SUCCESS(vkCreateFence(leaky_device, &fence_ci, nullptr, &leaked_fence));

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkDestroyDevice-device-00378");
    vkDestroyDevice(leaky_device, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, UseObjectWithWrongDevice) {
    TEST_DESCRIPTION(
        "Try to destroy a render pass object using a device other than the one it was created on. This should generate a distinct "
        "error from the invalid handle error.");
    // Create first device and renderpass
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Create second device
    float priorities[] = {1.0f};
    VkDeviceQueueCreateInfo queue_info{};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.pNext = NULL;
    queue_info.flags = 0;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &priorities[0];

    VkDeviceCreateInfo device_create_info = {};
    auto features = m_device->phy().features();
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = NULL;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_info;
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = NULL;
    device_create_info.pEnabledFeatures = &features;

    VkDevice second_device;
    ASSERT_VK_SUCCESS(vkCreateDevice(gpu(), &device_create_info, NULL, &second_device));

    // Try to destroy the renderpass from the first device using the second device
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkDestroyRenderPass-renderPass-parent");
    vkDestroyRenderPass(second_device, m_renderPass, NULL);
    m_errorMonitor->VerifyFound();

    vkDestroyDevice(second_device, NULL);
}

TEST_F(VkLayerTest, MismatchedQueueFamiliesOnSubmit) {
    TEST_DESCRIPTION(
        "Submit command buffer created using one queue family and attempt to submit them on a queue created in a different queue "
        "family.");

    ASSERT_NO_FATAL_FAILURE(Init());  // assumes it initializes all queue families on vkCreateDevice

    // This test is meaningless unless we have multiple queue families
    auto queue_family_properties = m_device->phy().queue_properties();
    std::vector<uint32_t> queue_families;
    for (uint32_t i = 0; i < queue_family_properties.size(); ++i)
        if (queue_family_properties[i].queueCount > 0) queue_families.push_back(i);

    if (queue_families.size() < 2) {
        printf("%s Device only has one queue family; skipped.\n", kSkipPrefix);
        return;
    }

    const uint32_t queue_family = queue_families[0];

    const uint32_t other_queue_family = queue_families[1];
    VkQueue other_queue;
    vkGetDeviceQueue(m_device->device(), other_queue_family, 0, &other_queue);

    VkCommandPoolObj cmd_pool(m_device, queue_family);
    VkCommandBufferObj cmd_buff(m_device, &cmd_pool);

    cmd_buff.begin();
    cmd_buff.end();

    // Submit on the wrong queue
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd_buff.handle();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkQueueSubmit-pCommandBuffers-00074");
    vkQueueSubmit(other_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TemporaryExternalSemaphore) {
#ifdef _WIN32
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT_KHR;
#else
    const auto extension_name = VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
#endif
    // Check for external semaphore instance extensions
    if (InstanceExtensionSupported(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s External semaphore extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    // Check for external semaphore device extensions
    if (DeviceExtensionSupported(gpu(), nullptr, extension_name)) {
        m_device_extension_names.push_back(extension_name);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
    } else {
        printf("%s External semaphore extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Check for external semaphore import and export capability
    VkPhysicalDeviceExternalSemaphoreInfoKHR esi = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO_KHR, nullptr,
                                                    handle_type};
    VkExternalSemaphorePropertiesKHR esp = {VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES_KHR, nullptr};
    auto vkGetPhysicalDeviceExternalSemaphorePropertiesKHR =
        (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)vkGetInstanceProcAddr(
            instance(), "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
    vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(gpu(), &esi, &esp);

    if (!(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(esp.externalSemaphoreFeatures & VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT_KHR)) {
        printf("%s External semaphore does not support importing and exporting, skipping test\n", kSkipPrefix);
        return;
    }

    VkResult err;

    // Create a semaphore to export payload from
    VkExportSemaphoreCreateInfoKHR esci = {VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO_KHR, nullptr, handle_type};
    VkSemaphoreCreateInfo sci = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, &esci, 0};

    VkSemaphore export_semaphore;
    err = vkCreateSemaphore(m_device->device(), &sci, nullptr, &export_semaphore);
    ASSERT_VK_SUCCESS(err);

    // Create a semaphore to import payload into
    sci.pNext = nullptr;
    VkSemaphore import_semaphore;
    err = vkCreateSemaphore(m_device->device(), &sci, nullptr, &import_semaphore);
    ASSERT_VK_SUCCESS(err);

#ifdef _WIN32
    // Export semaphore payload to an opaque handle
    HANDLE handle = nullptr;
    VkSemaphoreGetWin32HandleInfoKHR ghi = {VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR, nullptr, export_semaphore,
                                            handle_type};
    auto vkGetSemaphoreWin32HandleKHR =
        (PFN_vkGetSemaphoreWin32HandleKHR)vkGetDeviceProcAddr(m_device->device(), "vkGetSemaphoreWin32HandleKHR");
    err = vkGetSemaphoreWin32HandleKHR(m_device->device(), &ghi, &handle);
    ASSERT_VK_SUCCESS(err);

    // Import opaque handle exported above *temporarily*
    VkImportSemaphoreWin32HandleInfoKHR ihi = {VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR,
                                               nullptr,
                                               import_semaphore,
                                               VK_SEMAPHORE_IMPORT_TEMPORARY_BIT_KHR,
                                               handle_type,
                                               handle,
                                               nullptr};
    auto vkImportSemaphoreWin32HandleKHR =
        (PFN_vkImportSemaphoreWin32HandleKHR)vkGetDeviceProcAddr(m_device->device(), "vkImportSemaphoreWin32HandleKHR");
    err = vkImportSemaphoreWin32HandleKHR(m_device->device(), &ihi);
    ASSERT_VK_SUCCESS(err);
#else
    // Export semaphore payload to an opaque handle
    int fd = 0;
    VkSemaphoreGetFdInfoKHR ghi = {VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR, nullptr, export_semaphore, handle_type};
    auto vkGetSemaphoreFdKHR = (PFN_vkGetSemaphoreFdKHR)vkGetDeviceProcAddr(m_device->device(), "vkGetSemaphoreFdKHR");
    err = vkGetSemaphoreFdKHR(m_device->device(), &ghi, &fd);
    ASSERT_VK_SUCCESS(err);

    // Import opaque handle exported above *temporarily*
    VkImportSemaphoreFdInfoKHR ihi = {VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR, nullptr,     import_semaphore,
                                      VK_SEMAPHORE_IMPORT_TEMPORARY_BIT_KHR,          handle_type, fd};
    auto vkImportSemaphoreFdKHR = (PFN_vkImportSemaphoreFdKHR)vkGetDeviceProcAddr(m_device->device(), "vkImportSemaphoreFdKHR");
    err = vkImportSemaphoreFdKHR(m_device->device(), &ihi);
    ASSERT_VK_SUCCESS(err);
#endif

    // Wait on the imported semaphore twice in vkQueueSubmit, the second wait should be an error
    VkPipelineStageFlags flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkSubmitInfo si[] = {
        {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, &flags, 0, nullptr, 1, &export_semaphore},
        {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 1, &import_semaphore, &flags, 0, nullptr, 0, nullptr},
        {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, &flags, 0, nullptr, 1, &export_semaphore},
        {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 1, &import_semaphore, &flags, 0, nullptr, 0, nullptr},
    };
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "has no way to be signaled");
    vkQueueSubmit(m_device->m_queue, 4, si, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Wait on the imported semaphore twice in vkQueueBindSparse, the second wait should be an error
    VkBindSparseInfo bi[] = {
        {VK_STRUCTURE_TYPE_BIND_SPARSE_INFO, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 1, &export_semaphore},
        {VK_STRUCTURE_TYPE_BIND_SPARSE_INFO, nullptr, 1, &import_semaphore, 0, nullptr, 0, nullptr, 0, nullptr, 0, nullptr},
        {VK_STRUCTURE_TYPE_BIND_SPARSE_INFO, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 1, &export_semaphore},
        {VK_STRUCTURE_TYPE_BIND_SPARSE_INFO, nullptr, 1, &import_semaphore, 0, nullptr, 0, nullptr, 0, nullptr, 0, nullptr},
    };
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "has no way to be signaled");
    vkQueueBindSparse(m_device->m_queue, 4, bi, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Cleanup
    err = vkQueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
    vkDestroySemaphore(m_device->device(), export_semaphore, nullptr);
    vkDestroySemaphore(m_device->device(), import_semaphore, nullptr);
}

TEST_F(VkLayerTest, TemporaryExternalFence) {
#ifdef _WIN32
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;
#else
    const auto extension_name = VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
#endif
    // Check for external fence instance extensions
    if (InstanceExtensionSupported(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s External fence extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    // Check for external fence device extensions
    if (DeviceExtensionSupported(gpu(), nullptr, extension_name)) {
        m_device_extension_names.push_back(extension_name);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME);
    } else {
        printf("%s External fence extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Check for external fence import and export capability
    VkPhysicalDeviceExternalFenceInfoKHR efi = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO_KHR, nullptr, handle_type};
    VkExternalFencePropertiesKHR efp = {VK_STRUCTURE_TYPE_EXTERNAL_FENCE_PROPERTIES_KHR, nullptr};
    auto vkGetPhysicalDeviceExternalFencePropertiesKHR = (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)vkGetInstanceProcAddr(
        instance(), "vkGetPhysicalDeviceExternalFencePropertiesKHR");
    vkGetPhysicalDeviceExternalFencePropertiesKHR(gpu(), &efi, &efp);

    if (!(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_EXPORTABLE_BIT_KHR) ||
        !(efp.externalFenceFeatures & VK_EXTERNAL_FENCE_FEATURE_IMPORTABLE_BIT_KHR)) {
        printf("%s External fence does not support importing and exporting, skipping test\n", kSkipPrefix);
        return;
    }

    VkResult err;

    // Create a fence to export payload from
    VkFence export_fence;
    {
        VkExportFenceCreateInfoKHR efci = {VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO_KHR, nullptr, handle_type};
        VkFenceCreateInfo fci = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, &efci, 0};
        err = vkCreateFence(m_device->device(), &fci, nullptr, &export_fence);
        ASSERT_VK_SUCCESS(err);
    }

    // Create a fence to import payload into
    VkFence import_fence;
    {
        VkFenceCreateInfo fci = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0};
        err = vkCreateFence(m_device->device(), &fci, nullptr, &import_fence);
        ASSERT_VK_SUCCESS(err);
    }

#ifdef _WIN32
    // Export fence payload to an opaque handle
    HANDLE handle = nullptr;
    {
        VkFenceGetWin32HandleInfoKHR ghi = {VK_STRUCTURE_TYPE_FENCE_GET_WIN32_HANDLE_INFO_KHR, nullptr, export_fence, handle_type};
        auto vkGetFenceWin32HandleKHR =
            (PFN_vkGetFenceWin32HandleKHR)vkGetDeviceProcAddr(m_device->device(), "vkGetFenceWin32HandleKHR");
        err = vkGetFenceWin32HandleKHR(m_device->device(), &ghi, &handle);
        ASSERT_VK_SUCCESS(err);
    }

    // Import opaque handle exported above
    {
        VkImportFenceWin32HandleInfoKHR ifi = {VK_STRUCTURE_TYPE_IMPORT_FENCE_WIN32_HANDLE_INFO_KHR,
                                               nullptr,
                                               import_fence,
                                               VK_FENCE_IMPORT_TEMPORARY_BIT_KHR,
                                               handle_type,
                                               handle,
                                               nullptr};
        auto vkImportFenceWin32HandleKHR =
            (PFN_vkImportFenceWin32HandleKHR)vkGetDeviceProcAddr(m_device->device(), "vkImportFenceWin32HandleKHR");
        err = vkImportFenceWin32HandleKHR(m_device->device(), &ifi);
        ASSERT_VK_SUCCESS(err);
    }
#else
    // Export fence payload to an opaque handle
    int fd = 0;
    {
        VkFenceGetFdInfoKHR gfi = {VK_STRUCTURE_TYPE_FENCE_GET_FD_INFO_KHR, nullptr, export_fence, handle_type};
        auto vkGetFenceFdKHR = (PFN_vkGetFenceFdKHR)vkGetDeviceProcAddr(m_device->device(), "vkGetFenceFdKHR");
        err = vkGetFenceFdKHR(m_device->device(), &gfi, &fd);
        ASSERT_VK_SUCCESS(err);
    }

    // Import opaque handle exported above
    {
        VkImportFenceFdInfoKHR ifi = {VK_STRUCTURE_TYPE_IMPORT_FENCE_FD_INFO_KHR, nullptr,     import_fence,
                                      VK_FENCE_IMPORT_TEMPORARY_BIT_KHR,          handle_type, fd};
        auto vkImportFenceFdKHR = (PFN_vkImportFenceFdKHR)vkGetDeviceProcAddr(m_device->device(), "vkImportFenceFdKHR");
        err = vkImportFenceFdKHR(m_device->device(), &ifi);
        ASSERT_VK_SUCCESS(err);
    }
#endif

    // Undo the temporary import
    vkResetFences(m_device->device(), 1, &import_fence);

    // Signal the previously imported fence twice, the second signal should produce a validation error
    vkQueueSubmit(m_device->m_queue, 0, nullptr, import_fence);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "is already in use by another submission.");
    vkQueueSubmit(m_device->m_queue, 0, nullptr, import_fence);
    m_errorMonitor->VerifyFound();

    // Cleanup
    err = vkQueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
    vkDestroyFence(m_device->device(), export_fence, nullptr);
    vkDestroyFence(m_device->device(), import_fence, nullptr);
}

TEST_F(VkLayerTest, InvalidCmdBufferEventDestroyed) {
    TEST_DESCRIPTION("Attempt to draw with a command buffer that is invalid due to an event dependency being destroyed.");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkEvent event;
    VkEventCreateInfo evci = {};
    evci.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    VkResult result = vkCreateEvent(m_device->device(), &evci, NULL, &event);
    ASSERT_VK_SUCCESS(result);

    m_commandBuffer->begin();
    vkCmdSetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, " that is invalid because bound Event ");
    // Destroy event dependency prior to submit to cause ERROR
    vkDestroyEvent(m_device->device(), event, NULL);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidCmdBufferQueryPoolDestroyed) {
    TEST_DESCRIPTION("Attempt to draw with a command buffer that is invalid due to a query pool dependency being destroyed.");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo qpci{};
    qpci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    qpci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    qpci.queryCount = 1;
    VkResult result = vkCreateQueryPool(m_device->device(), &qpci, nullptr, &query_pool);
    ASSERT_VK_SUCCESS(result);

    m_commandBuffer->begin();
    vkCmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, " that is invalid because bound QueryPool ");
    // Destroy query pool dependency prior to submit to cause ERROR
    vkDestroyQueryPool(m_device->device(), query_pool, NULL);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DeviceFeature2AndVertexAttributeDivisorExtensionUnenabled) {
    TEST_DESCRIPTION(
        "Test unenabled VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME & "
        "VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME.");

    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT vadf = {};
    vadf.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT;
    VkPhysicalDeviceFeatures2 pd_features2 = {};
    pd_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    pd_features2.pNext = &vadf;

    ASSERT_NO_FATAL_FAILURE(Init());
    vk_testing::QueueCreateInfoArray queue_info(m_device->queue_props);
    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = &pd_features2;
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    VkDevice testDevice;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VK_KHR_get_physical_device_properties2 must be enabled when it creates an instance");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VK_EXT_vertex_attribute_divisor must be enabled when it creates a device");
    m_errorMonitor->SetUnexpectedError("Failed to create device chain");
    vkCreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidDeviceMask) {
    TEST_DESCRIPTION("Invalid deviceMask.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    bool support_surface = false;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    if (InstanceExtensionSupported(VK_KHR_WIN32_SURFACE_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
        support_surface = true;
    } else {
        printf("%s VK_KHR_WIN32_SURFACE_EXTENSION_NAME extension not supported, skipping VkAcquireNextImageInfoKHR test\n",
               kSkipPrefix);
    }
#elif VK_USE_PLATFORM_XLIB_KHR
    if (InstanceExtensionSupported(VK_KHR_XLIB_SURFACE_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
        support_surface = true;
    } else {
        printf("%s VK_KHR_XLIB_SURFACE_EXTENSION_NAME extension not supported, skipping VkAcquireNextImageInfoKHR test\n",
               kSkipPrefix);
    }
#elif defined(VK_USE_PLATFORM_ANDROID_KHR) && defined(VALIDATION_APK)
    if (InstanceExtensionSupported(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
        support_surface = true;
    } else {
        printf("%s VK_KHR_ANDROID_SURFACE_EXTENSION_NAME extension not supported, skipping VkAcquireNextImageInfoKHR test\n",
               kSkipPrefix);
    }
#else
    printf("%s VkSurface not supported, skipping VkAcquireNextImageInfoKHR test\n", kSkipPrefix);
#endif

    if (support_surface) {
        if (InstanceExtensionSupported(VK_KHR_SURFACE_EXTENSION_NAME)) {
            m_instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        } else {
            printf("%s VK_KHR_SURFACE_EXTENSION_NAM extension not supported, skipping VkAcquireNextImageInfoKHR test\n",
                   kSkipPrefix);
            support_surface = false;
        }
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Device Groups requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    uint32_t physical_device_group_count = 0;
    vkEnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, nullptr);

    if (physical_device_group_count == 0) {
        printf("%s physical_device_group_count is 0, skipping test\n", kSkipPrefix);
        return;
    }

    if (support_surface) {
        if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
            m_device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        } else {
            printf("%s VK_KHR_SWAPCHAIN_EXTENSION_NAME extension not supported, skipping VkAcquireNextImageInfoKHR test\n",
                   kSkipPrefix);
            support_surface = false;
        }
    }

    std::vector<VkPhysicalDeviceGroupProperties> physical_device_group(physical_device_group_count,
                                                                       {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    vkEnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, physical_device_group.data());
    VkDeviceGroupDeviceCreateInfo create_device_pnext = {};
    create_device_pnext.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;
    create_device_pnext.physicalDeviceCount = physical_device_group[0].physicalDeviceCount;
    create_device_pnext.pPhysicalDevices = physical_device_group[0].physicalDevices;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &create_device_pnext, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Test VkMemoryAllocateFlagsInfo
    VkMemoryAllocateFlagsInfo alloc_flags_info = {};
    alloc_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    alloc_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_MASK_BIT;
    alloc_flags_info.deviceMask = 0xFFFFFFFF;
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = &alloc_flags_info;
    alloc_info.memoryTypeIndex = 0;
    alloc_info.allocationSize = 32;

    VkDeviceMemory mem;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkMemoryAllocateFlagsInfo-deviceMask-00675");
    vkAllocateMemory(m_device->device(), &alloc_info, NULL, &mem);
    m_errorMonitor->VerifyFound();

    alloc_flags_info.deviceMask = 0;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkMemoryAllocateFlagsInfo-deviceMask-00676");
    vkAllocateMemory(m_device->device(), &alloc_info, NULL, &mem);
    m_errorMonitor->VerifyFound();

    // Test VkDeviceGroupCommandBufferBeginInfo
    VkDeviceGroupCommandBufferBeginInfo dev_grp_cmd_buf_info = {};
    dev_grp_cmd_buf_info.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO;
    dev_grp_cmd_buf_info.deviceMask = 0xFFFFFFFF;
    VkCommandBufferBeginInfo cmd_buf_info = {};
    cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_buf_info.pNext = &dev_grp_cmd_buf_info;

    m_commandBuffer->reset();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-VkDeviceGroupCommandBufferBeginInfo-deviceMask-00106");
    vkBeginCommandBuffer(m_commandBuffer->handle(), &cmd_buf_info);
    m_errorMonitor->VerifyFound();

    dev_grp_cmd_buf_info.deviceMask = 0;
    m_commandBuffer->reset();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-VkDeviceGroupCommandBufferBeginInfo-deviceMask-00107");
    vkBeginCommandBuffer(m_commandBuffer->handle(), &cmd_buf_info);
    m_errorMonitor->VerifyFound();

    // Test VkDeviceGroupRenderPassBeginInfo
    dev_grp_cmd_buf_info.deviceMask = 0x00000001;
    m_commandBuffer->reset();
    vkBeginCommandBuffer(m_commandBuffer->handle(), &cmd_buf_info);

    VkDeviceGroupRenderPassBeginInfo dev_grp_rp_info = {};
    dev_grp_rp_info.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO;
    dev_grp_rp_info.deviceMask = 0xFFFFFFFF;
    m_renderPassBeginInfo.pNext = &dev_grp_rp_info;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkDeviceGroupRenderPassBeginInfo-deviceMask-00905");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkDeviceGroupRenderPassBeginInfo-deviceMask-00907");
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();

    dev_grp_rp_info.deviceMask = 0;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkDeviceGroupRenderPassBeginInfo-deviceMask-00906");
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();

    dev_grp_rp_info.deviceMask = 0x00000001;
    dev_grp_rp_info.deviceRenderAreaCount = physical_device_group[0].physicalDeviceCount + 1;
    std::vector<VkRect2D> device_render_areas(dev_grp_rp_info.deviceRenderAreaCount, m_renderPassBeginInfo.renderArea);
    dev_grp_rp_info.pDeviceRenderAreas = device_render_areas.data();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-VkDeviceGroupRenderPassBeginInfo-deviceRenderAreaCount-00908");
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();

    // Test vkCmdSetDeviceMask()
    vkCmdSetDeviceMask(m_commandBuffer->handle(), 0x00000001);

    dev_grp_rp_info.deviceRenderAreaCount = physical_device_group[0].physicalDeviceCount;
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdSetDeviceMask-deviceMask-00108");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdSetDeviceMask-deviceMask-00110");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdSetDeviceMask-deviceMask-00111");
    vkCmdSetDeviceMask(m_commandBuffer->handle(), 0xFFFFFFFF);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdSetDeviceMask-deviceMask-00109");
    vkCmdSetDeviceMask(m_commandBuffer->handle(), 0);
    m_errorMonitor->VerifyFound();

    VkSemaphoreCreateInfo semaphore_create_info = {};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vkCreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));
    VkSemaphore semaphore2;
    ASSERT_VK_SUCCESS(vkCreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore2));
    VkFenceCreateInfo fence_create_info = {};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkFence fence;
    ASSERT_VK_SUCCESS(vkCreateFence(m_device->device(), &fence_create_info, nullptr, &fence));

    if (support_surface) {
        // Test VkAcquireNextImageInfoKHR
        ASSERT_NO_FATAL_FAILURE(InitSwapchain());

        uint32_t imageIndex = 0;
        VkAcquireNextImageInfoKHR acquire_next_image_info = {};
        acquire_next_image_info.sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR;
        acquire_next_image_info.semaphore = semaphore;
        acquire_next_image_info.swapchain = m_swapchain;
        acquire_next_image_info.fence = fence;
        acquire_next_image_info.deviceMask = 0xFFFFFFFF;

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkAcquireNextImageInfoKHR-deviceMask-01290");
        vkAcquireNextImage2KHR(m_device->device(), &acquire_next_image_info, &imageIndex);
        m_errorMonitor->VerifyFound();

        vkWaitForFences(m_device->device(), 1, &fence, VK_TRUE, std::numeric_limits<int>::max());
        vkResetFences(m_device->device(), 1, &fence);

        acquire_next_image_info.semaphore = semaphore2;
        acquire_next_image_info.deviceMask = 0;

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkAcquireNextImageInfoKHR-deviceMask-01291");
        vkAcquireNextImage2KHR(m_device->device(), &acquire_next_image_info, &imageIndex);
        m_errorMonitor->VerifyFound();
        DestroySwapchain();
    }

    // Test VkDeviceGroupSubmitInfo
    VkDeviceGroupSubmitInfo device_group_submit_info = {};
    device_group_submit_info.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO;
    device_group_submit_info.commandBufferCount = 1;
    std::array<uint32_t, 1> command_buffer_device_masks = {0xFFFFFFFF};
    device_group_submit_info.pCommandBufferDeviceMasks = command_buffer_device_masks.data();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = &device_group_submit_info;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    m_commandBuffer->reset();
    vkBeginCommandBuffer(m_commandBuffer->handle(), &cmd_buf_info);
    vkEndCommandBuffer(m_commandBuffer->handle());
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-VkDeviceGroupSubmitInfo-pCommandBufferDeviceMasks-00086");
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    vkQueueWaitIdle(m_device->m_queue);

    vkWaitForFences(m_device->device(), 1, &fence, VK_TRUE, std::numeric_limits<int>::max());
    vkDestroyFence(m_device->device(), fence, nullptr);
    vkDestroySemaphore(m_device->device(), semaphore, nullptr);
    vkDestroySemaphore(m_device->device(), semaphore2, nullptr);
}

TEST_F(VkLayerTest, ValidationCacheTestBadMerge) {
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), "VK_LAYER_LUNARG_core_validation", VK_EXT_VALIDATION_CACHE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_VALIDATION_CACHE_EXTENSION_NAME);
    } else {
        printf("%s %s not supported, skipping test\n", kSkipPrefix, VK_EXT_VALIDATION_CACHE_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Load extension functions
    auto fpCreateValidationCache =
        (PFN_vkCreateValidationCacheEXT)vkGetDeviceProcAddr(m_device->device(), "vkCreateValidationCacheEXT");
    auto fpDestroyValidationCache =
        (PFN_vkDestroyValidationCacheEXT)vkGetDeviceProcAddr(m_device->device(), "vkDestroyValidationCacheEXT");
    auto fpMergeValidationCaches =
        (PFN_vkMergeValidationCachesEXT)vkGetDeviceProcAddr(m_device->device(), "vkMergeValidationCachesEXT");
    if (!fpCreateValidationCache || !fpDestroyValidationCache || !fpMergeValidationCaches) {
        printf("%s Failed to load function pointers for %s\n", kSkipPrefix, VK_EXT_VALIDATION_CACHE_EXTENSION_NAME);
        return;
    }

    VkValidationCacheCreateInfoEXT validationCacheCreateInfo;
    validationCacheCreateInfo.sType = VK_STRUCTURE_TYPE_VALIDATION_CACHE_CREATE_INFO_EXT;
    validationCacheCreateInfo.pNext = NULL;
    validationCacheCreateInfo.initialDataSize = 0;
    validationCacheCreateInfo.pInitialData = NULL;
    validationCacheCreateInfo.flags = 0;
    VkValidationCacheEXT validationCache = VK_NULL_HANDLE;
    VkResult res = fpCreateValidationCache(m_device->device(), &validationCacheCreateInfo, nullptr, &validationCache);
    ASSERT_VK_SUCCESS(res);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkMergeValidationCachesEXT-dstCache-01536");
    res = fpMergeValidationCaches(m_device->device(), validationCache, 1, &validationCache);
    m_errorMonitor->VerifyFound();

    fpDestroyValidationCache(m_device->device(), validationCache, nullptr);
}

TEST_F(VkLayerTest, InvalidQueueFamilyIndex) {
    // Miscellaneous queueFamilyIndex validation tests
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkBufferCreateInfo buffCI = {};
    buffCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffCI.size = 1024;
    buffCI.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffCI.queueFamilyIndexCount = 2;
    // Introduce failure by specifying invalid queue_family_index
    uint32_t qfi[2];
    qfi[0] = 777;
    qfi[1] = 0;

    buffCI.pQueueFamilyIndices = qfi;
    buffCI.sharingMode = VK_SHARING_MODE_CONCURRENT;  // qfi only matters in CONCURRENT mode

    VkBuffer ib;
    // Test for queue family index out of range
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkBufferCreateInfo-sharingMode-01419");
    vkCreateBuffer(m_device->device(), &buffCI, NULL, &ib);
    m_errorMonitor->VerifyFound();

    // Test for non-unique QFI in array
    qfi[0] = 0;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkBufferCreateInfo-sharingMode-01419");
    vkCreateBuffer(m_device->device(), &buffCI, NULL, &ib);
    m_errorMonitor->VerifyFound();

    if (m_device->queue_props.size() > 2) {
        VkBuffer ib2;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "which was not created allowing concurrent");

        // Create buffer shared to queue families 1 and 2, but submitted on queue family 0
        buffCI.queueFamilyIndexCount = 2;
        qfi[0] = 1;
        qfi[1] = 2;
        vkCreateBuffer(m_device->device(), &buffCI, NULL, &ib2);
        VkDeviceMemory mem;
        VkMemoryRequirements mem_reqs;
        vkGetBufferMemoryRequirements(m_device->device(), ib2, &mem_reqs);

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_reqs.size;
        bool pass = false;
        pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        if (!pass) {
            printf("%s Failed to allocate required memory.\n", kSkipPrefix);
            vkDestroyBuffer(m_device->device(), ib2, NULL);
            return;
        }
        vkAllocateMemory(m_device->device(), &alloc_info, NULL, &mem);
        vkBindBufferMemory(m_device->device(), ib2, mem, 0);

        m_commandBuffer->begin();
        vkCmdFillBuffer(m_commandBuffer->handle(), ib2, 0, 16, 5);
        m_commandBuffer->end();
        m_commandBuffer->QueueCommandBuffer(false);
        m_errorMonitor->VerifyFound();
        vkDestroyBuffer(m_device->device(), ib2, NULL);
        vkFreeMemory(m_device->device(), mem, NULL);
    }
}

TEST_F(VkLayerTest, InvalidQueryPoolCreate) {
    TEST_DESCRIPTION("Attempt to create a query pool for PIPELINE_STATISTICS without enabling pipeline stats for the device.");

    ASSERT_NO_FATAL_FAILURE(Init());

    vk_testing::QueueCreateInfoArray queue_info(m_device->queue_props);

    VkDevice local_device;
    VkDeviceCreateInfo device_create_info = {};
    auto features = m_device->phy().features();
    // Intentionally disable pipeline stats
    features.pipelineStatisticsQuery = VK_FALSE;
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = NULL;
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = NULL;
    device_create_info.pEnabledFeatures = &features;
    VkResult err = vkCreateDevice(gpu(), &device_create_info, nullptr, &local_device);
    ASSERT_VK_SUCCESS(err);

    VkQueryPoolCreateInfo qpci{};
    qpci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    qpci.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
    qpci.queryCount = 1;
    VkQueryPool query_pool;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkQueryPoolCreateInfo-queryType-00791");
    vkCreateQueryPool(local_device, &qpci, nullptr, &query_pool);
    m_errorMonitor->VerifyFound();

    vkDestroyDevice(local_device, nullptr);
}

TEST_F(VkLayerTest, UnclosedQuery) {
    TEST_DESCRIPTION("End a command buffer with a query still in progress.");

    const char *invalid_query = "Ending command buffer with in progress query: queryPool 0x";

    ASSERT_NO_FATAL_FAILURE(Init());

    VkEvent event;
    VkEventCreateInfo event_create_info{};
    event_create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    vkCreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 0, &queue);

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, invalid_query);

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = {};
    query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = 1;
    vkCreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    vkCmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0 /*startQuery*/, 1 /*queryCount*/);
    vkCmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, 0);

    vkEndCommandBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();

    vkDestroyQueryPool(m_device->device(), query_pool, nullptr);
    vkDestroyEvent(m_device->device(), event, nullptr);
}

TEST_F(VkLayerTest, QueryPreciseBit) {
    TEST_DESCRIPTION("Check for correct Query Precise Bit circumstances.");
    ASSERT_NO_FATAL_FAILURE(Init());

    // These tests require that the device support pipeline statistics query
    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));
    if (VK_TRUE != device_features.pipelineStatisticsQuery) {
        printf("%s Test requires unsupported pipelineStatisticsQuery feature. Skipped.\n", kSkipPrefix);
        return;
    }

    std::vector<const char *> device_extension_names;
    auto features = m_device->phy().features();

    // Test for precise bit when query type is not OCCLUSION
    if (features.occlusionQueryPrecise) {
        VkEvent event;
        VkEventCreateInfo event_create_info{};
        event_create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
        vkCreateEvent(m_device->handle(), &event_create_info, nullptr, &event);

        m_commandBuffer->begin();
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBeginQuery-queryType-00800");

        VkQueryPool query_pool;
        VkQueryPoolCreateInfo query_pool_create_info = {};
        query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        query_pool_create_info.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
        query_pool_create_info.queryCount = 1;
        vkCreateQueryPool(m_device->handle(), &query_pool_create_info, nullptr, &query_pool);

        vkCmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);
        vkCmdBeginQuery(m_commandBuffer->handle(), query_pool, 0, VK_QUERY_CONTROL_PRECISE_BIT);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->end();
        vkDestroyQueryPool(m_device->handle(), query_pool, nullptr);
        vkDestroyEvent(m_device->handle(), event, nullptr);
    }

    // Test for precise bit when precise feature is not available
    features.occlusionQueryPrecise = false;
    VkDeviceObj test_device(0, gpu(), device_extension_names, &features);

    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = test_device.graphics_queue_node_index_;

    VkCommandPool command_pool;
    vkCreateCommandPool(test_device.handle(), &pool_create_info, nullptr, &command_pool);

    VkCommandBufferAllocateInfo cmd = {};
    cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd.pNext = NULL;
    cmd.commandPool = command_pool;
    cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd.commandBufferCount = 1;

    VkCommandBuffer cmd_buffer;
    VkResult err = vkAllocateCommandBuffers(test_device.handle(), &cmd, &cmd_buffer);
    ASSERT_VK_SUCCESS(err);

    VkEvent event;
    VkEventCreateInfo event_create_info{};
    event_create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    vkCreateEvent(test_device.handle(), &event_create_info, nullptr, &event);

    VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
                                           VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};

    vkBeginCommandBuffer(cmd_buffer, &begin_info);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBeginQuery-queryType-00800");

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info = {};
    query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_create_info.queryCount = 1;
    vkCreateQueryPool(test_device.handle(), &query_pool_create_info, nullptr, &query_pool);

    vkCmdResetQueryPool(cmd_buffer, query_pool, 0, 1);
    vkCmdBeginQuery(cmd_buffer, query_pool, 0, VK_QUERY_CONTROL_PRECISE_BIT);
    m_errorMonitor->VerifyFound();

    vkEndCommandBuffer(cmd_buffer);
    vkDestroyQueryPool(test_device.handle(), query_pool, nullptr);
    vkDestroyEvent(test_device.handle(), event, nullptr);
    vkDestroyCommandPool(test_device.handle(), command_pool, nullptr);
}

TEST_F(VkLayerTest, StageMaskGsTsEnabled) {
    TEST_DESCRIPTION(
        "Attempt to use a stageMask w/ geometry shader and tesselation shader bits enabled when those features are disabled on the "
        "device.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    std::vector<const char *> device_extension_names;
    auto features = m_device->phy().features();
    // Make sure gs & ts are disabled
    features.geometryShader = false;
    features.tessellationShader = false;
    // The sacrificial device object
    VkDeviceObj test_device(0, gpu(), device_extension_names, &features);

    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = test_device.graphics_queue_node_index_;

    VkCommandPool command_pool;
    vkCreateCommandPool(test_device.handle(), &pool_create_info, nullptr, &command_pool);

    VkCommandBufferAllocateInfo cmd = {};
    cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd.pNext = NULL;
    cmd.commandPool = command_pool;
    cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd.commandBufferCount = 1;

    VkCommandBuffer cmd_buffer;
    VkResult err = vkAllocateCommandBuffers(test_device.handle(), &cmd, &cmd_buffer);
    ASSERT_VK_SUCCESS(err);

    VkEvent event;
    VkEventCreateInfo evci = {};
    evci.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    VkResult result = vkCreateEvent(test_device.handle(), &evci, NULL, &event);
    ASSERT_VK_SUCCESS(result);

    VkCommandBufferBeginInfo cbbi = {};
    cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(cmd_buffer, &cbbi);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdSetEvent-stageMask-01150");
    vkCmdSetEvent(cmd_buffer, event, VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdSetEvent-stageMask-01151");
    vkCmdSetEvent(cmd_buffer, event, VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT);
    m_errorMonitor->VerifyFound();

    vkDestroyEvent(test_device.handle(), event, NULL);
    vkDestroyCommandPool(test_device.handle(), command_pool, NULL);
}

TEST_F(VkLayerTest, DescriptorPoolInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete a DescriptorPool with a DescriptorSet that is in use.");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = {};
    ds_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ds_pool_ci.pNext = NULL;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    VkDescriptorPool ds_pool;
    VkResult err = vkCreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &ds_pool);
    ASSERT_VK_SUCCESS(err);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutObj ds_layout(m_device, {dsl_binding});

    VkDescriptorSet descriptor_set;
    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool;
    alloc_info.pSetLayouts = &ds_layout.handle();
    err = vkAllocateDescriptorSets(m_device->device(), &alloc_info, &descriptor_set);
    ASSERT_VK_SUCCESS(err);

    const VkPipelineLayoutObj pipeline_layout(m_device, {&ds_layout});

    // Create image to update the descriptor with
    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView view = image.targetView(VK_FORMAT_B8G8R8A8_UNORM);
    // Create Sampler
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    err = vkCreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);
    // Update descriptor with image and sampler
    VkDescriptorImageInfo img_info = {};
    img_info.sampler = sampler;
    img_info.imageView = view;
    img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet descriptor_write;
    memset(&descriptor_write, 0, sizeof(descriptor_write));
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = descriptor_set;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = &img_info;

    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    // Create PSO to be used for draw-time errors below
    char const *vsSource =
        "#version 450\n"
        "\n"
        "void main(){\n"
        "   gl_Position = vec4(1);\n"
        "}\n";
    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(set=0, binding=0) uniform sampler2D s;\n"
        "layout(location=0) out vec4 x;\n"
        "void main(){\n"
        "   x = texture(s, vec2(1));\n"
        "}\n";
    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                            &descriptor_set, 0, NULL);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vkCmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vkCmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Submit cmd buffer to put pool in-flight
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    // Destroy pool while in-flight, causing error
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkDestroyDescriptorPool-descriptorPool-00303");
    vkDestroyDescriptorPool(m_device->device(), ds_pool, NULL);
    m_errorMonitor->VerifyFound();
    vkQueueWaitIdle(m_device->m_queue);
    // Cleanup
    vkDestroySampler(m_device->device(), sampler, NULL);
    m_errorMonitor->SetUnexpectedError(
        "If descriptorPool is not VK_NULL_HANDLE, descriptorPool must be a valid VkDescriptorPool handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove DescriptorPool obj");
    vkDestroyDescriptorPool(m_device->device(), ds_pool, NULL);
    // TODO : It seems Validation layers think ds_pool was already destroyed, even though it wasn't?
}

TEST_F(VkLayerTest, FramebufferInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use framebuffer.");
    ASSERT_NO_FATAL_FAILURE(Init());
    VkFormatProperties format_properties;
    VkResult err = VK_SUCCESS;
    vkGetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_B8G8R8A8_UNORM, &format_properties);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj image(m_device);
    image.Init(256, 256, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());
    VkImageView view = image.targetView(VK_FORMAT_B8G8R8A8_UNORM);

    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, m_renderPass, 1, &view, 256, 256, 1};
    VkFramebuffer fb;
    err = vkCreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    // Just use default renderpass with our framebuffer
    m_renderPassBeginInfo.framebuffer = fb;
    // Create Null cmd buffer for submit
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Submit cmd buffer to put it in-flight
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    // Destroy framebuffer while in-flight
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkDestroyFramebuffer-framebuffer-00892");
    vkDestroyFramebuffer(m_device->device(), fb, NULL);
    m_errorMonitor->VerifyFound();
    // Wait for queue to complete so we can safely destroy everything
    vkQueueWaitIdle(m_device->m_queue);
    m_errorMonitor->SetUnexpectedError("If framebuffer is not VK_NULL_HANDLE, framebuffer must be a valid VkFramebuffer handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Framebuffer obj");
    vkDestroyFramebuffer(m_device->device(), fb, nullptr);
}

TEST_F(VkLayerTest, FramebufferImageInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use image that's child of framebuffer.");
    ASSERT_NO_FATAL_FAILURE(Init());
    VkFormatProperties format_properties;
    VkResult err = VK_SUCCESS;
    vkGetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_B8G8R8A8_UNORM, &format_properties);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_ci = {};
    image_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_ci.pNext = NULL;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_ci.extent.width = 256;
    image_ci.extent.height = 256;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.flags = 0;
    VkImage image;
    ASSERT_VK_SUCCESS(vkCreateImage(m_device->handle(), &image_ci, NULL, &image));

    VkMemoryRequirements memory_reqs;
    VkDeviceMemory image_memory;
    bool pass;
    VkMemoryAllocateInfo memory_info = {};
    memory_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_info.pNext = NULL;
    memory_info.allocationSize = 0;
    memory_info.memoryTypeIndex = 0;
    vkGetImageMemoryRequirements(m_device->device(), image, &memory_reqs);
    memory_info.allocationSize = memory_reqs.size;
    pass = m_device->phy().set_memory_type(memory_reqs.memoryTypeBits, &memory_info, 0);
    ASSERT_TRUE(pass);
    err = vkAllocateMemory(m_device->device(), &memory_info, NULL, &image_memory);
    ASSERT_VK_SUCCESS(err);
    err = vkBindImageMemory(m_device->device(), image, image_memory, 0);
    ASSERT_VK_SUCCESS(err);

    VkImageViewCreateInfo ivci = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
        0,
        image,
        VK_IMAGE_VIEW_TYPE_2D,
        VK_FORMAT_B8G8R8A8_UNORM,
        {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A},
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };
    VkImageView view;
    err = vkCreateImageView(m_device->device(), &ivci, nullptr, &view);
    ASSERT_VK_SUCCESS(err);

    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, m_renderPass, 1, &view, 256, 256, 1};
    VkFramebuffer fb;
    err = vkCreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    // Just use default renderpass with our framebuffer
    m_renderPassBeginInfo.framebuffer = fb;
    // Create Null cmd buffer for submit
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Submit cmd buffer to put it (and attached imageView) in-flight
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    // Submit cmd buffer to put framebuffer and children in-flight
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    // Destroy image attached to framebuffer while in-flight
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkDestroyImage-image-01000");
    vkDestroyImage(m_device->device(), image, NULL);
    m_errorMonitor->VerifyFound();
    // Wait for queue to complete so we can safely destroy image and other objects
    vkQueueWaitIdle(m_device->m_queue);
    m_errorMonitor->SetUnexpectedError("If image is not VK_NULL_HANDLE, image must be a valid VkImage handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Image obj");
    vkDestroyImage(m_device->device(), image, NULL);
    vkDestroyFramebuffer(m_device->device(), fb, nullptr);
    vkDestroyImageView(m_device->device(), view, nullptr);
    vkFreeMemory(m_device->device(), image_memory, nullptr);
}

TEST_F(VkLayerTest, EventInUseDestroyedSignaled) {
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();

    VkEvent event;
    VkEventCreateInfo event_create_info = {};
    event_create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    vkCreateEvent(m_device->device(), &event_create_info, nullptr, &event);
    vkCmdSetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

    m_commandBuffer->end();
    vkDestroyEvent(m_device->device(), event, nullptr);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "that is invalid because bound");
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InUseDestroyedSignaled) {
    TEST_DESCRIPTION(
        "Use vkCmdExecuteCommands with invalid state in primary and secondary command buffers. Delete objects that are in use. "
        "Call VkQueueSubmit with an event that has been deleted.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_errorMonitor->ExpectSuccess();

    VkSemaphoreCreateInfo semaphore_create_info = {};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vkCreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));
    VkFenceCreateInfo fence_create_info = {};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkFence fence;
    ASSERT_VK_SUCCESS(vkCreateFence(m_device->device(), &fence_create_info, nullptr, &fence));

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                     });

    VkBufferTest buffer_test(m_device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = buffer_test.GetBuffer();
    buffer_info.offset = 0;
    buffer_info.range = 1024;

    VkWriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set.dstSet = ds.set_;
    write_descriptor_set.descriptorCount = 1;
    write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_descriptor_set.pBufferInfo = &buffer_info;

    vkUpdateDescriptorSets(m_device->device(), 1, &write_descriptor_set, 0, nullptr);

    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    VkPipelineObj pipe(m_device);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);

    const VkPipelineLayoutObj pipeline_layout(m_device, {&ds.layout_});

    pipe.CreateVKPipeline(pipeline_layout.handle(), m_renderPass);

    VkEvent event;
    VkEventCreateInfo event_create_info = {};
    event_create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    vkCreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    m_commandBuffer->begin();

    vkCmdSetEvent(m_commandBuffer->handle(), event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1, &ds.set_, 0,
                            NULL);

    m_commandBuffer->end();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore;
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, fence);
    m_errorMonitor->Reset();  // resume logmsg processing

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkDestroyEvent-event-01145");
    vkDestroyEvent(m_device->device(), event, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkDestroySemaphore-semaphore-01137");
    vkDestroySemaphore(m_device->device(), semaphore, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "Fence 0x");
    vkDestroyFence(m_device->device(), fence, nullptr);
    m_errorMonitor->VerifyFound();

    vkQueueWaitIdle(m_device->m_queue);
    m_errorMonitor->SetUnexpectedError("If semaphore is not VK_NULL_HANDLE, semaphore must be a valid VkSemaphore handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Semaphore obj");
    vkDestroySemaphore(m_device->device(), semaphore, nullptr);
    m_errorMonitor->SetUnexpectedError("If fence is not VK_NULL_HANDLE, fence must be a valid VkFence handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Fence obj");
    vkDestroyFence(m_device->device(), fence, nullptr);
    m_errorMonitor->SetUnexpectedError("If event is not VK_NULL_HANDLE, event must be a valid VkEvent handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Event obj");
    vkDestroyEvent(m_device->device(), event, nullptr);
}

TEST_F(VkLayerTest, QueryPoolInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use query pool.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci{};
    query_pool_ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_ci.queryCount = 1;
    vkCreateQueryPool(m_device->device(), &query_pool_ci, nullptr, &query_pool);
    m_commandBuffer->begin();
    // Reset query pool to create binding with cmd buffer
    vkCmdResetQueryPool(m_commandBuffer->handle(), query_pool, 0, 1);

    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkGetQueryPoolResults-queryType-00818");
    uint32_t data_space[16];
    m_errorMonitor->SetUnexpectedError("Cannot get query results on queryPool");
    vkGetQueryPoolResults(m_device->handle(), query_pool, 0, 1, sizeof(data_space), &data_space, sizeof(uint32_t),
                          VK_QUERY_RESULT_PARTIAL_BIT);
    m_errorMonitor->VerifyFound();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    // Submit cmd buffer and then destroy query pool while in-flight
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkDestroyQueryPool-queryPool-00793");
    vkDestroyQueryPool(m_device->handle(), query_pool, NULL);
    m_errorMonitor->VerifyFound();

    vkQueueWaitIdle(m_device->m_queue);
    // Now that cmd buffer done we can safely destroy query_pool
    m_errorMonitor->SetUnexpectedError("If queryPool is not VK_NULL_HANDLE, queryPool must be a valid VkQueryPool handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove QueryPool obj");
    vkDestroyQueryPool(m_device->handle(), query_pool, NULL);
}

TEST_F(VkLayerTest, PipelineInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use pipeline.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkPipelineLayoutObj pipeline_layout(m_device);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkDestroyPipeline-pipeline-00765");
    // Create PSO to be used for draw-time errors below
    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);
    // Store pipeline handle so we can actually delete it before test finishes
    VkPipeline delete_this_pipeline;
    {  // Scope pipeline so it will be auto-deleted
        VkPipelineObj pipe(m_device);
        pipe.AddShader(&vs);
        pipe.AddShader(&fs);
        pipe.AddDefaultColorAttachment();
        pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());
        delete_this_pipeline = pipe.handle();

        m_commandBuffer->begin();
        // Bind pipeline to cmd buffer
        vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());

        m_commandBuffer->end();

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_commandBuffer->handle();
        // Submit cmd buffer and then pipeline destroyed while in-flight
        vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    }  // Pipeline deletion triggered here
    m_errorMonitor->VerifyFound();
    // Make sure queue finished and then actually delete pipeline
    vkQueueWaitIdle(m_device->m_queue);
    m_errorMonitor->SetUnexpectedError("If pipeline is not VK_NULL_HANDLE, pipeline must be a valid VkPipeline handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Pipeline obj");
    vkDestroyPipeline(m_device->handle(), delete_this_pipeline, nullptr);
}

TEST_F(VkLayerTest, ImageViewInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use imageView.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                     });

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;

    VkResult err;
    err = vkCreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    const VkPipelineLayoutObj pipeline_layout(m_device, {&ds.layout_});

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView view;
    VkImageViewCreateInfo ivci = {};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    err = vkCreateImageView(m_device->device(), &ivci, NULL, &view);
    ASSERT_VK_SUCCESS(err);

    VkDescriptorImageInfo image_info{};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView = view;
    image_info.sampler = sampler;

    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = &image_info;

    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    // Create PSO to use the sampler
    char const *vsSource =
        "#version 450\n"
        "\n"
        "void main(){\n"
        "   gl_Position = vec4(1);\n"
        "}\n";
    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(set=0, binding=0) uniform sampler2D s;\n"
        "layout(location=0) out vec4 x;\n"
        "void main(){\n"
        "   x = texture(s, vec2(1));\n"
        "}\n";
    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkDestroyImageView-imageView-01026");

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    // Bind pipeline to cmd buffer
    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1, &ds.set_, 0,
                            nullptr);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vkCmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vkCmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Submit cmd buffer then destroy sampler
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    // Submit cmd buffer and then destroy imageView while in-flight
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    vkDestroyImageView(m_device->device(), view, nullptr);
    m_errorMonitor->VerifyFound();
    vkQueueWaitIdle(m_device->m_queue);
    // Now we can actually destroy imageView
    m_errorMonitor->SetUnexpectedError("If imageView is not VK_NULL_HANDLE, imageView must be a valid VkImageView handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove ImageView obj");
    vkDestroyImageView(m_device->device(), view, NULL);
    vkDestroySampler(m_device->device(), sampler, nullptr);
}

TEST_F(VkLayerTest, BufferViewInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use bufferView.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                     });

    const VkPipelineLayoutObj pipeline_layout(m_device, {&ds.layout_});

    VkBuffer buffer;
    uint32_t queue_family_index = 0;
    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &queue_family_index;

    VkResult err = vkCreateBuffer(m_device->device(), &buffer_create_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkMemoryRequirements memory_reqs;
    VkDeviceMemory buffer_memory;

    VkMemoryAllocateInfo memory_info = {};
    memory_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_info.allocationSize = 0;
    memory_info.memoryTypeIndex = 0;

    vkGetBufferMemoryRequirements(m_device->device(), buffer, &memory_reqs);
    memory_info.allocationSize = memory_reqs.size;
    bool pass = m_device->phy().set_memory_type(memory_reqs.memoryTypeBits, &memory_info, 0);
    ASSERT_TRUE(pass);

    err = vkAllocateMemory(m_device->device(), &memory_info, NULL, &buffer_memory);
    ASSERT_VK_SUCCESS(err);
    err = vkBindBufferMemory(m_device->device(), buffer, buffer_memory, 0);
    ASSERT_VK_SUCCESS(err);

    VkBufferView view;
    VkBufferViewCreateInfo bvci = {};
    bvci.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    bvci.buffer = buffer;
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;

    err = vkCreateBufferView(m_device->device(), &bvci, NULL, &view);
    ASSERT_VK_SUCCESS(err);

    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    descriptor_write.pTexelBufferView = &view;

    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    char const *vsSource =
        "#version 450\n"
        "\n"
        "void main(){\n"
        "   gl_Position = vec4(1);\n"
        "}\n";
    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(set=0, binding=0, r32f) uniform readonly imageBuffer s;\n"
        "layout(location=0) out vec4 x;\n"
        "void main(){\n"
        "   x = imageLoad(s, 0);\n"
        "}\n";
    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkDestroyBufferView-bufferView-00936");

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vkCmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vkCmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    // Bind pipeline to cmd buffer
    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1, &ds.set_, 0,
                            nullptr);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    // Submit cmd buffer and then destroy bufferView while in-flight
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    vkDestroyBufferView(m_device->device(), view, nullptr);
    m_errorMonitor->VerifyFound();
    vkQueueWaitIdle(m_device->m_queue);
    // Now we can actually destroy bufferView
    m_errorMonitor->SetUnexpectedError("If bufferView is not VK_NULL_HANDLE, bufferView must be a valid VkBufferView handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove BufferView obj");
    vkDestroyBufferView(m_device->device(), view, NULL);
    vkDestroyBuffer(m_device->device(), buffer, NULL);
    vkFreeMemory(m_device->device(), buffer_memory, NULL);
}

TEST_F(VkLayerTest, SamplerInUseDestroyedSignaled) {
    TEST_DESCRIPTION("Delete in-use sampler.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet ds(m_device, {
                                         {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                     });

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;

    VkResult err;
    err = vkCreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    const VkPipelineLayoutObj pipeline_layout(m_device, {&ds.layout_});

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView view;
    VkImageViewCreateInfo ivci = {};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    err = vkCreateImageView(m_device->device(), &ivci, NULL, &view);
    ASSERT_VK_SUCCESS(err);

    VkDescriptorImageInfo image_info{};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView = view;
    image_info.sampler = sampler;

    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = ds.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = &image_info;

    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    // Create PSO to use the sampler
    char const *vsSource =
        "#version 450\n"
        "\n"
        "void main(){\n"
        "   gl_Position = vec4(1);\n"
        "}\n";
    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(set=0, binding=0) uniform sampler2D s;\n"
        "layout(location=0) out vec4 x;\n"
        "void main(){\n"
        "   x = texture(s, vec2(1));\n"
        "}\n";
    VkShaderObj vs(m_device, vsSource, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkDestroySampler-sampler-01082");

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    // Bind pipeline to cmd buffer
    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1, &ds.set_, 0,
                            nullptr);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vkCmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vkCmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Submit cmd buffer then destroy sampler
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    // Submit cmd buffer and then destroy sampler while in-flight
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    vkDestroySampler(m_device->device(), sampler, nullptr);  // Destroyed too soon
    m_errorMonitor->VerifyFound();
    vkQueueWaitIdle(m_device->m_queue);

    // Now we can actually destroy sampler
    m_errorMonitor->SetUnexpectedError("If sampler is not VK_NULL_HANDLE, sampler must be a valid VkSampler handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove Sampler obj");
    vkDestroySampler(m_device->device(), sampler, NULL);  // Destroyed for real
    vkDestroyImageView(m_device->device(), view, NULL);
}

TEST_F(VkLayerTest, QueueForwardProgressFenceWait) {
    TEST_DESCRIPTION(
        "Call VkQueueSubmit with a semaphore that is already signaled but not waited on by the queue. Wait on a fence that has not "
        "yet been submitted to a queue.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const char *queue_forward_progress_message = " that was previously signaled by queue 0x";
    const char *invalid_fence_wait_message = " which has not been submitted on a Queue or during acquire next image.";

    VkCommandBufferObj cb1(m_device, m_commandPool);
    cb1.begin();
    cb1.end();

    VkSemaphoreCreateInfo semaphore_create_info = {};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkSemaphore semaphore;
    ASSERT_VK_SUCCESS(vkCreateSemaphore(m_device->device(), &semaphore_create_info, nullptr, &semaphore));
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cb1.handle();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &semaphore;
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_commandBuffer->begin();
    m_commandBuffer->end();
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, queue_forward_progress_message);
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    VkFenceCreateInfo fence_create_info = {};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkFence fence;
    ASSERT_VK_SUCCESS(vkCreateFence(m_device->device(), &fence_create_info, nullptr, &fence));

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_WARNING_BIT_EXT, invalid_fence_wait_message);
    vkWaitForFences(m_device->device(), 1, &fence, VK_TRUE, UINT64_MAX);
    m_errorMonitor->VerifyFound();

    vkDeviceWaitIdle(m_device->device());
    vkDestroyFence(m_device->device(), fence, nullptr);
    vkDestroySemaphore(m_device->device(), semaphore, nullptr);
}

#if GTEST_IS_THREADSAFE
TEST_F(VkLayerTest, ThreadCommandBufferCollision) {
    test_platform_thread thread;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "THREADING ERROR");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Calls AllocateCommandBuffers
    VkCommandBufferObj commandBuffer(m_device, m_commandPool);

    commandBuffer.begin();

    VkEventCreateInfo event_info;
    VkEvent event;
    VkResult err;

    memset(&event_info, 0, sizeof(event_info));
    event_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;

    err = vkCreateEvent(device(), &event_info, NULL, &event);
    ASSERT_VK_SUCCESS(err);

    err = vkResetEvent(device(), event);
    ASSERT_VK_SUCCESS(err);

    struct thread_data_struct data;
    data.commandBuffer = commandBuffer.handle();
    data.event = event;
    data.bailout = false;
    m_errorMonitor->SetBailout(&data.bailout);

    // First do some correct operations using multiple threads.
    // Add many entries to command buffer from another thread.
    test_platform_thread_create(&thread, AddToCommandBuffer, (void *)&data);
    // Make non-conflicting calls from this thread at the same time.
    for (int i = 0; i < 80000; i++) {
        uint32_t count;
        vkEnumeratePhysicalDevices(instance(), &count, NULL);
    }
    test_platform_thread_join(thread, NULL);

    // Then do some incorrect operations using multiple threads.
    // Add many entries to command buffer from another thread.
    test_platform_thread_create(&thread, AddToCommandBuffer, (void *)&data);
    // Add many entries to command buffer from this thread at the same time.
    AddToCommandBuffer(&data);

    test_platform_thread_join(thread, NULL);
    commandBuffer.end();

    m_errorMonitor->SetBailout(NULL);

    m_errorMonitor->VerifyFound();

    vkDestroyEvent(device(), event, NULL);
}
#endif  // GTEST_IS_THREADSAFE

TEST_F(VkLayerTest, ExecuteUnrecordedPrimaryCB) {
    TEST_DESCRIPTION("Attempt vkQueueSubmit with a CB in the initial state");
    ASSERT_NO_FATAL_FAILURE(Init());
    // never record m_commandBuffer

    VkSubmitInfo si = {};
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &m_commandBuffer->handle();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkQueueSubmit-pCommandBuffers-00072");
    vkQueueSubmit(m_device->m_queue, 1, &si, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, Maintenance1AndNegativeViewport) {
    TEST_DESCRIPTION("Attempt to enable AMD_negative_viewport_height and Maintenance1_KHR extension simultaneously");

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    if (!((DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE1_EXTENSION_NAME)) &&
          (DeviceExtensionSupported(gpu(), nullptr, VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME)))) {
        printf("%s Maintenance1 and AMD_negative viewport height extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    vk_testing::QueueCreateInfoArray queue_info(m_device->queue_props);
    const char *extension_names[2] = {"VK_KHR_maintenance1", "VK_AMD_negative_viewport_height"};
    VkDevice testDevice;
    VkDeviceCreateInfo device_create_info = {};
    auto features = m_device->phy().features();
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = NULL;
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = NULL;
    device_create_info.enabledExtensionCount = 2;
    device_create_info.ppEnabledExtensionNames = (const char *const *)extension_names;
    device_create_info.pEnabledFeatures = &features;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkDeviceCreateInfo-ppEnabledExtensionNames-00374");
    // The following unexpected error is coming from the LunarG loader. Do not make it a desired message because platforms that do
    // not use the LunarG loader (e.g. Android) will not see the message and the test will fail.
    m_errorMonitor->SetUnexpectedError("Failed to create device chain.");
    vkCreateDevice(gpu(), &device_create_info, NULL, &testDevice);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, HostQueryResetNotEnabled) {
    TEST_DESCRIPTION("Use vkResetQueryPoolEXT without enabling the feature");

    if (!InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME)) {
        printf("%s Extension %s not supported by device; skipped.\n", kSkipPrefix, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
        return;
    }

    m_device_extension_names.push_back(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto fpvkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vkGetDeviceProcAddr(m_device->device(), "vkResetQueryPoolEXT");

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info{};
    query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vkCreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkResetQueryPoolEXT-None-02665");
    fpvkResetQueryPoolEXT(m_device->device(), query_pool, 0, 1);
    m_errorMonitor->VerifyFound();

    vkDestroyQueryPool(m_device->device(), query_pool, nullptr);
}

TEST_F(VkLayerTest, HostQueryResetBadFirstQuery) {
    TEST_DESCRIPTION("Bad firstQuery in vkResetQueryPoolEXT");

    if (!InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME)) {
        printf("%s Extension %s not supported by device; skipped.\n", kSkipPrefix, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
        return;
    }

    m_device_extension_names.push_back(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features{};
    host_query_reset_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES_EXT;
    host_query_reset_features.hostQueryReset = VK_TRUE;

    VkPhysicalDeviceFeatures2 pd_features2{};
    pd_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    pd_features2.pNext = &host_query_reset_features;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));

    auto fpvkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vkGetDeviceProcAddr(m_device->device(), "vkResetQueryPoolEXT");

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info{};
    query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vkCreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkResetQueryPoolEXT-firstQuery-02666");
    fpvkResetQueryPoolEXT(m_device->device(), query_pool, 1, 0);
    m_errorMonitor->VerifyFound();

    vkDestroyQueryPool(m_device->device(), query_pool, nullptr);
}

TEST_F(VkLayerTest, HostQueryResetBadRange) {
    TEST_DESCRIPTION("Bad range in vkResetQueryPoolEXT");

    if (!InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME)) {
        printf("%s Extension %s not supported by device; skipped.\n", kSkipPrefix, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
        return;
    }

    m_device_extension_names.push_back(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features{};
    host_query_reset_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES_EXT;
    host_query_reset_features.hostQueryReset = VK_TRUE;

    VkPhysicalDeviceFeatures2 pd_features2{};
    pd_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    pd_features2.pNext = &host_query_reset_features;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));

    auto fpvkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vkGetDeviceProcAddr(m_device->device(), "vkResetQueryPoolEXT");

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info{};
    query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vkCreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkResetQueryPoolEXT-firstQuery-02667");
    fpvkResetQueryPoolEXT(m_device->device(), query_pool, 0, 2);
    m_errorMonitor->VerifyFound();

    vkDestroyQueryPool(m_device->device(), query_pool, nullptr);
}

TEST_F(VkLayerTest, HostQueryResetInvalidQueryPool) {
    TEST_DESCRIPTION("Invalid queryPool in vkResetQueryPoolEXT");

    if (!InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME)) {
        printf("%s Extension %s not supported by device; skipped.\n", kSkipPrefix, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
        return;
    }

    m_device_extension_names.push_back(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features{};
    host_query_reset_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES_EXT;
    host_query_reset_features.hostQueryReset = VK_TRUE;

    VkPhysicalDeviceFeatures2 pd_features2{};
    pd_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    pd_features2.pNext = &host_query_reset_features;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));

    auto fpvkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vkGetDeviceProcAddr(m_device->device(), "vkResetQueryPoolEXT");

    // Create and destroy a query pool.
    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info{};
    query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vkCreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);
    vkDestroyQueryPool(m_device->device(), query_pool, nullptr);

    // Attempt to reuse the query pool handle.
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkResetQueryPoolEXT-queryPool-parameter");
    fpvkResetQueryPoolEXT(m_device->device(), query_pool, 0, 1);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, HostQueryResetWrongDevice) {
    TEST_DESCRIPTION("Device not matching queryPool in vkResetQueryPoolEXT");

    if (!InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME)) {
        printf("%s Extension %s not supported by device; skipped.\n", kSkipPrefix, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
        return;
    }

    m_device_extension_names.push_back(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);

    VkPhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_features{};
    host_query_reset_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES_EXT;
    host_query_reset_features.hostQueryReset = VK_TRUE;

    VkPhysicalDeviceFeatures2 pd_features2{};
    pd_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    pd_features2.pNext = &host_query_reset_features;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));

    auto fpvkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vkGetDeviceProcAddr(m_device->device(), "vkResetQueryPoolEXT");

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info{};
    query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vkCreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    // Create a second device with the feature enabled.
    vk_testing::QueueCreateInfoArray queue_info(m_device->queue_props);
    auto features = m_device->phy().features();

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = &host_query_reset_features;
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    device_create_info.pEnabledFeatures = &features;
    device_create_info.enabledExtensionCount = m_device_extension_names.size();
    device_create_info.ppEnabledExtensionNames = m_device_extension_names.data();

    VkDevice second_device;
    ASSERT_VK_SUCCESS(vkCreateDevice(gpu(), &device_create_info, nullptr, &second_device));

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkResetQueryPoolEXT-queryPool-parent");
    // Run vkResetQueryPoolExt on the wrong device.
    fpvkResetQueryPoolEXT(second_device, query_pool, 0, 1);
    m_errorMonitor->VerifyFound();

    vkDestroyQueryPool(m_device->device(), query_pool, nullptr);
    vkDestroyDevice(second_device, nullptr);
}

TEST_F(VkLayerTest, ResetEventThenSet) {
    TEST_DESCRIPTION("Reset an event then set it after the reset has been submitted.");

    ASSERT_NO_FATAL_FAILURE(Init());
    VkEvent event;
    VkEventCreateInfo event_create_info{};
    event_create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    vkCreateEvent(m_device->device(), &event_create_info, nullptr, &event);

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(m_device->device(), &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer;
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(m_device->device(), &command_buffer_allocate_info, &command_buffer);

    VkQueue queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(m_device->device(), m_device->graphics_queue_node_index_, 0, &queue);

    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(command_buffer, &begin_info);

        vkCmdResetEvent(command_buffer, event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        vkEndCommandBuffer(command_buffer);
    }
    {
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = nullptr;
        vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    }
    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "that is already in use by a command buffer.");
        vkSetEvent(m_device->device(), event);
        m_errorMonitor->VerifyFound();
    }

    vkQueueWaitIdle(queue);

    vkDestroyEvent(m_device->device(), event, nullptr);
    vkFreeCommandBuffers(m_device->device(), command_pool, 1, &command_buffer);
    vkDestroyCommandPool(m_device->device(), command_pool, NULL);
}

TEST_F(VkLayerTest, ShadingRateImageNV) {
    TEST_DESCRIPTION("Test VK_NV_shading_rate_image.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    std::array<const char *, 1> required_device_extensions = {{VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    if (DeviceIsMockICD() || DeviceSimulation()) {
        printf("%s Test not supported by MockICD, skipping tests\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device that enables shading_rate_image but disables multiViewport
    auto shading_rate_image_features = lvl_init_struct<VkPhysicalDeviceShadingRateImageFeaturesNV>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&shading_rate_image_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    features2.features.multiViewport = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Test shading rate image creation
    VkImage image = VK_NULL_HANDLE;
    VkResult result = VK_RESULT_MAX_ENUM;
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8_UINT;
    image_create_info.extent.width = 4;
    image_create_info.extent.height = 4;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

    // image type must be 2D
    image_create_info.imageType = VK_IMAGE_TYPE_3D;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageCreateInfo-imageType-02082");
    result = vkCreateImage(m_device->device(), &image_create_info, NULL, &image);
    m_errorMonitor->VerifyFound();
    if (VK_SUCCESS == result) {
        vkDestroyImage(m_device->device(), image, NULL);
        image = VK_NULL_HANDLE;
    }
    image_create_info.imageType = VK_IMAGE_TYPE_2D;

    // must be single sample
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageCreateInfo-samples-02083");
    result = vkCreateImage(m_device->device(), &image_create_info, NULL, &image);
    m_errorMonitor->VerifyFound();
    if (VK_SUCCESS == result) {
        vkDestroyImage(m_device->device(), image, NULL);
        image = VK_NULL_HANDLE;
    }
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;

    // tiling must be optimal
    image_create_info.tiling = VK_IMAGE_TILING_LINEAR;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageCreateInfo-tiling-02084");
    result = vkCreateImage(m_device->device(), &image_create_info, NULL, &image);
    m_errorMonitor->VerifyFound();
    if (VK_SUCCESS == result) {
        vkDestroyImage(m_device->device(), image, NULL);
        image = VK_NULL_HANDLE;
    }
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;

    // Should succeed.
    result = vkCreateImage(m_device->device(), &image_create_info, NULL, &image);
    m_errorMonitor->VerifyNotFound();

    // bind memory to the image
    VkMemoryRequirements memory_reqs;
    VkDeviceMemory image_memory;
    bool pass;
    VkMemoryAllocateInfo memory_info = {};
    memory_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_info.pNext = NULL;
    memory_info.allocationSize = 0;
    memory_info.memoryTypeIndex = 0;
    vkGetImageMemoryRequirements(m_device->device(), image, &memory_reqs);
    memory_info.allocationSize = memory_reqs.size;
    pass = m_device->phy().set_memory_type(memory_reqs.memoryTypeBits, &memory_info, 0);
    ASSERT_TRUE(pass);
    result = vkAllocateMemory(m_device->device(), &memory_info, NULL, &image_memory);
    ASSERT_VK_SUCCESS(result);
    result = vkBindImageMemory(m_device->device(), image, image_memory, 0);
    ASSERT_VK_SUCCESS(result);

    // Test image view creation
    VkImageView view;
    VkImageViewCreateInfo ivci = {};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.image = image;
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8_UINT;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // view type must be 2D or 2D_ARRAY
    ivci.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageViewCreateInfo-image-02086");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageViewCreateInfo-image-01003");
    result = vkCreateImageView(m_device->device(), &ivci, nullptr, &view);
    m_errorMonitor->VerifyFound();
    if (VK_SUCCESS == result) {
        vkDestroyImageView(m_device->device(), view, NULL);
        view = VK_NULL_HANDLE;
    }
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;

    // format must be R8_UINT
    ivci.format = VK_FORMAT_R8_UNORM;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageViewCreateInfo-image-02087");
    result = vkCreateImageView(m_device->device(), &ivci, nullptr, &view);
    m_errorMonitor->VerifyFound();
    if (VK_SUCCESS == result) {
        vkDestroyImageView(m_device->device(), view, NULL);
        view = VK_NULL_HANDLE;
    }
    ivci.format = VK_FORMAT_R8_UINT;

    vkCreateImageView(m_device->device(), &ivci, nullptr, &view);
    m_errorMonitor->VerifyNotFound();

    // Test pipeline creation
    VkPipelineViewportShadingRateImageStateCreateInfoNV vsrisci = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV};

    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    VkViewport viewports[20] = {viewport, viewport};
    VkRect2D scissor = {{0, 0}, {64, 64}};
    VkRect2D scissors[20] = {scissor, scissor};
    VkDynamicState dynPalette = VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV;
    VkPipelineDynamicStateCreateInfo dyn = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr, 0, 1, &dynPalette};

    // viewportCount must be 0 or 1 when multiViewport is disabled
    {
        const auto break_vp = [&](CreatePipelineHelper &helper) {
            helper.vp_state_ci_.viewportCount = 2;
            helper.vp_state_ci_.pViewports = viewports;
            helper.vp_state_ci_.scissorCount = 2;
            helper.vp_state_ci_.pScissors = scissors;
            helper.vp_state_ci_.pNext = &vsrisci;
            helper.dyn_state_ci_ = dyn;

            vsrisci.shadingRateImageEnable = VK_TRUE;
            vsrisci.viewportCount = 2;
        };
        CreatePipelineHelper::OneshotTest(
            *this, break_vp, VK_DEBUG_REPORT_ERROR_BIT_EXT,
            vector<std::string>({"VUID-VkPipelineViewportShadingRateImageStateCreateInfoNV-viewportCount-02054",
                                 "VUID-VkPipelineViewportStateCreateInfo-viewportCount-01216",
                                 "VUID-VkPipelineViewportStateCreateInfo-scissorCount-01217"}));
    }

    // viewportCounts must match
    {
        const auto break_vp = [&](CreatePipelineHelper &helper) {
            helper.vp_state_ci_.viewportCount = 1;
            helper.vp_state_ci_.pViewports = viewports;
            helper.vp_state_ci_.scissorCount = 1;
            helper.vp_state_ci_.pScissors = scissors;
            helper.vp_state_ci_.pNext = &vsrisci;
            helper.dyn_state_ci_ = dyn;

            vsrisci.shadingRateImageEnable = VK_TRUE;
            vsrisci.viewportCount = 0;
        };
        CreatePipelineHelper::OneshotTest(
            *this, break_vp, VK_DEBUG_REPORT_ERROR_BIT_EXT,
            vector<std::string>({"VUID-VkPipelineViewportShadingRateImageStateCreateInfoNV-shadingRateImageEnable-02056"}));
    }

    // pShadingRatePalettes must not be NULL.
    {
        const auto break_vp = [&](CreatePipelineHelper &helper) {
            helper.vp_state_ci_.viewportCount = 1;
            helper.vp_state_ci_.pViewports = viewports;
            helper.vp_state_ci_.scissorCount = 1;
            helper.vp_state_ci_.pScissors = scissors;
            helper.vp_state_ci_.pNext = &vsrisci;

            vsrisci.shadingRateImageEnable = VK_TRUE;
            vsrisci.viewportCount = 1;
        };
        CreatePipelineHelper::OneshotTest(
            *this, break_vp, VK_DEBUG_REPORT_ERROR_BIT_EXT,
            vector<std::string>({"VUID-VkPipelineViewportShadingRateImageStateCreateInfoNV-pDynamicStates-02057"}));
    }

    // Create an image without the SRI bit
    VkImageObj nonSRIimage(m_device);
    nonSRIimage.Init(256, 256, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(nonSRIimage.initialized());
    VkImageView nonSRIview = nonSRIimage.targetView(VK_FORMAT_B8G8R8A8_UNORM);

    // Test SRI layout on non-SRI image
    VkImageMemoryBarrier img_barrier = {};
    img_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    img_barrier.pNext = nullptr;
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = 0;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV;
    img_barrier.image = nonSRIimage.handle();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    m_commandBuffer->begin();

    // Error trying to convert it to SRI layout
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageMemoryBarrier-oldLayout-02088");
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &img_barrier);
    m_errorMonitor->VerifyFound();

    // succeed converting it to GENERAL
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &img_barrier);
    m_errorMonitor->VerifyNotFound();

    // Test vkCmdBindShadingRateImageNV errors
    auto vkCmdBindShadingRateImageNV =
        (PFN_vkCmdBindShadingRateImageNV)vkGetDeviceProcAddr(m_device->device(), "vkCmdBindShadingRateImageNV");

    // if the view is non-NULL, it must be R8_UINT, USAGE_SRI, image layout must match, layout must be valid
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBindShadingRateImageNV-imageView-02060");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBindShadingRateImageNV-imageView-02061");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBindShadingRateImageNV-imageView-02062");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBindShadingRateImageNV-imageLayout-02063");
    vkCmdBindShadingRateImageNV(m_commandBuffer->handle(), nonSRIview, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    m_errorMonitor->VerifyFound();

    // Test vkCmdSetViewportShadingRatePaletteNV errors
    auto vkCmdSetViewportShadingRatePaletteNV =
        (PFN_vkCmdSetViewportShadingRatePaletteNV)vkGetDeviceProcAddr(m_device->device(), "vkCmdSetViewportShadingRatePaletteNV");

    VkShadingRatePaletteEntryNV paletteEntries[100] = {};
    VkShadingRatePaletteNV palette = {100, paletteEntries};
    VkShadingRatePaletteNV palettes[] = {palette, palette};

    // errors on firstViewport/viewportCount
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdSetViewportShadingRatePaletteNV-firstViewport-02066");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdSetViewportShadingRatePaletteNV-firstViewport-02067");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdSetViewportShadingRatePaletteNV-firstViewport-02068");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdSetViewportShadingRatePaletteNV-viewportCount-02069");
    vkCmdSetViewportShadingRatePaletteNV(m_commandBuffer->handle(), 20, 2, palettes);
    m_errorMonitor->VerifyFound();

    // shadingRatePaletteEntryCount must be in range
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-VkShadingRatePaletteNV-shadingRatePaletteEntryCount-02071");
    vkCmdSetViewportShadingRatePaletteNV(m_commandBuffer->handle(), 0, 1, palettes);
    m_errorMonitor->VerifyFound();

    VkCoarseSampleLocationNV locations[100] = {
        {0, 0, 0},    {0, 0, 1}, {0, 1, 0}, {0, 1, 1}, {0, 1, 1},  // duplicate
        {1000, 0, 0},                                              // pixelX too large
        {0, 1000, 0},                                              // pixelY too large
        {0, 0, 1000},                                              // sample too large
    };

    // Test custom sample orders, both via pipeline state and via dynamic state
    {
        VkCoarseSampleOrderCustomNV sampOrdBadShadingRate = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_PIXEL_NV, 1, 1,
                                                             locations};
        VkCoarseSampleOrderCustomNV sampOrdBadSampleCount = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV, 3, 1,
                                                             locations};
        VkCoarseSampleOrderCustomNV sampOrdBadSampleLocationCount = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV,
                                                                     2, 2, locations};
        VkCoarseSampleOrderCustomNV sampOrdDuplicateLocations = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV, 2,
                                                                 1 * 2 * 2, &locations[1]};
        VkCoarseSampleOrderCustomNV sampOrdOutOfRangeLocations = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV, 2,
                                                                  1 * 2 * 2, &locations[4]};
        VkCoarseSampleOrderCustomNV sampOrdTooLargeSampleLocationCount = {
            VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X4_PIXELS_NV, 4, 64, &locations[8]};
        VkCoarseSampleOrderCustomNV sampOrdGood = {VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV, 2, 1 * 2 * 2,
                                                   &locations[0]};

        VkPipelineViewportCoarseSampleOrderStateCreateInfoNV csosci = {
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV};
        csosci.sampleOrderType = VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV;
        csosci.customSampleOrderCount = 1;

        using std::vector;
        struct TestCase {
            const VkCoarseSampleOrderCustomNV *order;
            vector<std::string> vuids;
        };

        vector<TestCase> test_cases = {
            {&sampOrdBadShadingRate, {"VUID-VkCoarseSampleOrderCustomNV-shadingRate-02073"}},
            {&sampOrdBadSampleCount,
             {"VUID-VkCoarseSampleOrderCustomNV-sampleCount-02074", "VUID-VkCoarseSampleOrderCustomNV-sampleLocationCount-02075"}},
            {&sampOrdBadSampleLocationCount, {"VUID-VkCoarseSampleOrderCustomNV-sampleLocationCount-02075"}},
            {&sampOrdDuplicateLocations, {"VUID-VkCoarseSampleOrderCustomNV-pSampleLocations-02077"}},
            {&sampOrdOutOfRangeLocations,
             {"VUID-VkCoarseSampleOrderCustomNV-pSampleLocations-02077", "VUID-VkCoarseSampleLocationNV-pixelX-02078",
              "VUID-VkCoarseSampleLocationNV-pixelY-02079", "VUID-VkCoarseSampleLocationNV-sample-02080"}},
            {&sampOrdTooLargeSampleLocationCount,
             {"VUID-VkCoarseSampleOrderCustomNV-sampleLocationCount-02076",
              "VUID-VkCoarseSampleOrderCustomNV-pSampleLocations-02077"}},
            {&sampOrdGood, {}},
        };

        for (const auto &test_case : test_cases) {
            const auto break_vp = [&](CreatePipelineHelper &helper) {
                helper.vp_state_ci_.pNext = &csosci;
                csosci.pCustomSampleOrders = test_case.order;
            };
            CreatePipelineHelper::OneshotTest(*this, break_vp, VK_DEBUG_REPORT_ERROR_BIT_EXT, test_case.vuids);
        }

        // Test vkCmdSetCoarseSampleOrderNV errors
        auto vkCmdSetCoarseSampleOrderNV =
            (PFN_vkCmdSetCoarseSampleOrderNV)vkGetDeviceProcAddr(m_device->device(), "vkCmdSetCoarseSampleOrderNV");

        for (const auto &test_case : test_cases) {
            for (uint32_t i = 0; i < test_case.vuids.size(); ++i) {
                m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, test_case.vuids[i]);
            }
            vkCmdSetCoarseSampleOrderNV(m_commandBuffer->handle(), VK_COARSE_SAMPLE_ORDER_TYPE_CUSTOM_NV, 1, test_case.order);
            if (test_case.vuids.size()) {
                m_errorMonitor->VerifyFound();
            } else {
                m_errorMonitor->VerifyNotFound();
            }
        }

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-vkCmdSetCoarseSampleOrderNV-sampleOrderType-02081");
        vkCmdSetCoarseSampleOrderNV(m_commandBuffer->handle(), VK_COARSE_SAMPLE_ORDER_TYPE_PIXEL_MAJOR_NV, 1, &sampOrdGood);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();

    vkDestroyImageView(m_device->device(), view, NULL);
    vkDestroyImage(m_device->device(), image, NULL);
    vkFreeMemory(m_device->device(), image_memory, NULL);
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
#include "android_ndk_types.h"

TEST_F(VkLayerTest, AndroidHardwareBufferImageCreate) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer image create info.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if ((DeviceExtensionSupported(gpu(), nullptr, VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)) &&
        // Also skip on devices that advertise AHB, but not the pre-requisite foreign_queue extension
        (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME))) {
        m_device_extension_names.push_back(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
    } else {
        printf("%s %s extension not supported, skipping tests\n", kSkipPrefix,
               VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    VkDevice dev = m_device->device();

    VkImage img = VK_NULL_HANDLE;
    auto reset_img = [&img, dev]() {
        if (VK_NULL_HANDLE != img) vkDestroyImage(dev, img, NULL);
        img = VK_NULL_HANDLE;
    };

    VkImageCreateInfo ici = {};
    ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ici.pNext = nullptr;
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_UNDEFINED;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    // undefined format
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageCreateInfo-pNext-01975");
    m_errorMonitor->SetUnexpectedError("VUID_Undefined");
    vkCreateImage(dev, &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    reset_img();

    // also undefined format
    VkExternalFormatANDROID efa = {};
    efa.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    efa.externalFormat = 0;
    ici.pNext = &efa;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageCreateInfo-pNext-01975");
    vkCreateImage(dev, &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    reset_img();

    // undefined format with an unknown external format
    efa.externalFormat = 0xBADC0DE;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkExternalFormatANDROID-externalFormat-01894");
    vkCreateImage(dev, &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    reset_img();

    AHardwareBuffer *ahb;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    // Allocate an AHardwareBuffer
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    // Retrieve it's properties to make it's external format 'known' (AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM)
    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = {};
    ahb_fmt_props.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = {};
    ahb_props.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    ahb_props.pNext = &ahb_fmt_props;
    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vkGetDeviceProcAddr(dev, "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);
    pfn_GetAHBProps(dev, ahb, &ahb_props);

    // a defined image format with a non-zero external format
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    efa.externalFormat = ahb_fmt_props.externalFormat;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageCreateInfo-pNext-01974");
    vkCreateImage(dev, &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    reset_img();
    ici.format = VK_FORMAT_UNDEFINED;

    // external format while MUTABLE
    ici.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageCreateInfo-pNext-02396");
    vkCreateImage(dev, &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    reset_img();
    ici.flags = 0;

    // external format while usage other than SAMPLED
    ici.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageCreateInfo-pNext-02397");
    vkCreateImage(dev, &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    reset_img();
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    // external format while tiline other than OPTIMAL
    ici.tiling = VK_IMAGE_TILING_LINEAR;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageCreateInfo-pNext-02398");
    vkCreateImage(dev, &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    reset_img();
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;

    // imageType
    VkExternalMemoryImageCreateInfo emici = {};
    emici.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
    emici.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    ici.pNext = &emici;  // remove efa from chain, insert emici
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.imageType = VK_IMAGE_TYPE_3D;
    ici.extent = {64, 64, 64};

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageCreateInfo-pNext-02393");
    vkCreateImage(dev, &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    reset_img();

    // wrong mipLevels
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.extent = {64, 64, 1};
    ici.mipLevels = 6;  // should be 7
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageCreateInfo-pNext-02394");
    vkCreateImage(dev, &ici, NULL, &img);
    m_errorMonitor->VerifyFound();
    reset_img();
}

TEST_F(VkLayerTest, AndroidHardwareBufferFetchUnboundImageInfo) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer retreive image properties while memory unbound.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if ((DeviceExtensionSupported(gpu(), nullptr, VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)) &&
        // Also skip on devices that advertise AHB, but not the pre-requisite foreign_queue extension
        (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME))) {
        m_device_extension_names.push_back(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
    } else {
        printf("%s %s extension not supported, skipping tests\n", kSkipPrefix,
               VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    VkDevice dev = m_device->device();

    VkImage img = VK_NULL_HANDLE;
    auto reset_img = [&img, dev]() {
        if (VK_NULL_HANDLE != img) vkDestroyImage(dev, img, NULL);
        img = VK_NULL_HANDLE;
    };

    VkImageCreateInfo ici = {};
    ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ici.pNext = nullptr;
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_LINEAR;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkExternalMemoryImageCreateInfo emici = {};
    emici.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
    emici.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    ici.pNext = &emici;

    m_errorMonitor->ExpectSuccess();
    vkCreateImage(dev, &ici, NULL, &img);
    m_errorMonitor->VerifyNotFound();

    // attempt to fetch layout from unbound image
    VkImageSubresource sub_rsrc = {};
    sub_rsrc.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    VkSubresourceLayout sub_layout = {};
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkGetImageSubresourceLayout-image-01895");
    vkGetImageSubresourceLayout(dev, img, &sub_rsrc, &sub_layout);
    m_errorMonitor->VerifyFound();

    // attempt to get memory reqs from unbound image
    VkImageMemoryRequirementsInfo2 imri = {};
    imri.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
    imri.image = img;
    VkMemoryRequirements2 mem_reqs = {};
    mem_reqs.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageMemoryRequirementsInfo2-image-01897");
    vkGetImageMemoryRequirements2(dev, &imri, &mem_reqs);
    m_errorMonitor->VerifyFound();

    reset_img();
}

TEST_F(VkLayerTest, AndroidHardwareBufferMemoryAllocation) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer memory allocation.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if ((DeviceExtensionSupported(gpu(), nullptr, VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)) &&
        // Also skip on devices that advertise AHB, but not the pre-requisite foreign_queue extension
        (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME))) {
        m_device_extension_names.push_back(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
    } else {
        printf("%s %s extension not supported, skipping tests\n", kSkipPrefix,
               VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    VkDevice dev = m_device->device();

    VkImage img = VK_NULL_HANDLE;
    auto reset_img = [&img, dev]() {
        if (VK_NULL_HANDLE != img) vkDestroyImage(dev, img, NULL);
        img = VK_NULL_HANDLE;
    };
    VkDeviceMemory mem_handle = VK_NULL_HANDLE;
    auto reset_mem = [&mem_handle, dev]() {
        if (VK_NULL_HANDLE != mem_handle) vkFreeMemory(dev, mem_handle, NULL);
        mem_handle = VK_NULL_HANDLE;
    };

    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vkGetDeviceProcAddr(dev, "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);

    // AHB structs
    AHardwareBuffer *ahb = nullptr;
    AHardwareBuffer_Desc ahb_desc = {};
    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = {};
    ahb_fmt_props.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = {};
    ahb_props.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    ahb_props.pNext = &ahb_fmt_props;
    VkImportAndroidHardwareBufferInfoANDROID iahbi = {};
    iahbi.sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;

    // destroy and re-acquire an AHB, and fetch it's properties
    auto recreate_ahb = [&ahb, &iahbi, &ahb_desc, &ahb_props, dev, pfn_GetAHBProps]() {
        if (ahb) AHardwareBuffer_release(ahb);
        ahb = nullptr;
        AHardwareBuffer_allocate(&ahb_desc, &ahb);
        if (ahb) {
            pfn_GetAHBProps(dev, ahb, &ahb_props);
            iahbi.buffer = ahb;
        }
    };

    // Allocate an AHardwareBuffer
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    recreate_ahb();

    // Create an image w/ external format
    VkExternalFormatANDROID efa = {};
    efa.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    efa.externalFormat = ahb_fmt_props.externalFormat;

    VkImageCreateInfo ici = {};
    ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ici.pNext = &efa;
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_UNDEFINED;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    VkResult res = vkCreateImage(dev, &ici, NULL, &img);
    ASSERT_VK_SUCCESS(res);

    VkMemoryAllocateInfo mai = {};
    mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mai.pNext = &iahbi;  // Chained import struct
    mai.allocationSize = ahb_props.allocationSize;
    mai.memoryTypeIndex = 32;
    // Set index to match one of the bits in ahb_props
    for (int i = 0; i < 32; i++) {
        if (ahb_props.memoryTypeBits & (1 << i)) {
            mai.memoryTypeIndex = i;
            break;
        }
    }
    ASSERT_NE(32, mai.memoryTypeIndex);

    // Import w/ non-dedicated memory allocation

    // Import requires format AHB_FMT_BLOB and usage AHB_USAGE_GPU_DATA_BUFFER
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkMemoryAllocateInfo-pNext-02384");
    vkAllocateMemory(dev, &mai, NULL, &mem_handle);
    m_errorMonitor->VerifyFound();
    reset_mem();

    // Allocation size mismatch
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_BLOB;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER;
    ahb_desc.height = 1;
    recreate_ahb();
    mai.allocationSize = ahb_props.allocationSize + 1;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkMemoryAllocateInfo-allocationSize-02383");
    vkAllocateMemory(dev, &mai, NULL, &mem_handle);
    m_errorMonitor->VerifyFound();
    mai.allocationSize = ahb_props.allocationSize;
    reset_mem();

    // memoryTypeIndex mismatch
    mai.memoryTypeIndex++;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkMemoryAllocateInfo-memoryTypeIndex-02385");
    vkAllocateMemory(dev, &mai, NULL, &mem_handle);
    m_errorMonitor->VerifyFound();
    mai.memoryTypeIndex--;
    reset_mem();

    // Insert dedicated image memory allocation to mai chain
    VkMemoryDedicatedAllocateInfo mdai = {};
    mdai.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
    mdai.image = img;
    mdai.buffer = VK_NULL_HANDLE;
    mdai.pNext = mai.pNext;
    mai.pNext = &mdai;

    // Dedicated allocation with unmatched usage bits
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT;
    ahb_desc.height = 64;
    recreate_ahb();
    mai.allocationSize = ahb_props.allocationSize;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkMemoryAllocateInfo-pNext-02390");
    vkAllocateMemory(dev, &mai, NULL, &mem_handle);
    m_errorMonitor->VerifyFound();
    reset_mem();

    // Dedicated allocation with incomplete mip chain
    reset_img();
    ici.mipLevels = 2;
    vkCreateImage(dev, &ici, NULL, &img);
    mdai.image = img;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE;
    recreate_ahb();

    if (ahb) {
        mai.allocationSize = ahb_props.allocationSize;
        for (int i = 0; i < 32; i++) {
            if (ahb_props.memoryTypeBits & (1 << i)) {
                mai.memoryTypeIndex = i;
                break;
            }
        }
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkMemoryAllocateInfo-pNext-02389");
        vkAllocateMemory(dev, &mai, NULL, &mem_handle);
        m_errorMonitor->VerifyFound();
        reset_mem();
    } else {
        // ERROR: AHardwareBuffer_allocate() with MIPMAP_COMPLETE fails. It returns -12, NO_MEMORY.
        // The problem seems to happen in Pixel 2, not Pixel 3.
        printf("%s AHARDWAREBUFFER_USAGE_GPU_MIPMAP_COMPLETE not supported, skipping tests\n", kSkipPrefix);
    }

    // Dedicated allocation with mis-matched dimension
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.height = 32;
    ahb_desc.width = 128;
    recreate_ahb();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkMemoryAllocateInfo-pNext-02388");
    vkAllocateMemory(dev, &mai, NULL, &mem_handle);
    m_errorMonitor->VerifyFound();
    reset_mem();

    // Dedicated allocation with mis-matched VkFormat
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.height = 64;
    ahb_desc.width = 64;
    recreate_ahb();
    ici.mipLevels = 1;
    ici.format = VK_FORMAT_B8G8R8A8_UNORM;
    ici.pNext = NULL;
    VkImage img2;
    vkCreateImage(dev, &ici, NULL, &img2);
    mdai.image = img2;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkMemoryAllocateInfo-pNext-02387");
    vkAllocateMemory(dev, &mai, NULL, &mem_handle);
    m_errorMonitor->VerifyFound();
    vkDestroyImage(dev, img2, NULL);
    mdai.image = img;
    reset_mem();

    // Missing required ahb usage
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkGetAndroidHardwareBufferPropertiesANDROID-buffer-01884");
    recreate_ahb();
    m_errorMonitor->VerifyFound();

    // Dedicated allocation with missing usage bits
    // Setting up this test also triggers a slew of others
    mai.allocationSize = ahb_props.allocationSize + 1;
    mai.memoryTypeIndex = 0;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkMemoryAllocateInfo-pNext-02390");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkMemoryAllocateInfo-memoryTypeIndex-02385");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkMemoryAllocateInfo-allocationSize-02383");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkMemoryAllocateInfo-pNext-02386");
    vkAllocateMemory(dev, &mai, NULL, &mem_handle);
    m_errorMonitor->VerifyFound();
    reset_mem();

    // Non-import allocation - replace import struct in chain with export struct
    VkExportMemoryAllocateInfo emai = {};
    emai.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
    emai.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    mai.pNext = &emai;
    emai.pNext = &mdai;  // still dedicated
    mdai.pNext = nullptr;

    // Export with allocation size non-zero
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    recreate_ahb();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkMemoryAllocateInfo-pNext-01874");
    vkAllocateMemory(dev, &mai, NULL, &mem_handle);
    m_errorMonitor->VerifyFound();
    reset_mem();

    AHardwareBuffer_release(ahb);
    reset_mem();
    reset_img();
}

TEST_F(VkLayerTest, AndroidHardwareBufferCreateYCbCrSampler) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer YCbCr sampler creation.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if ((DeviceExtensionSupported(gpu(), nullptr, VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)) &&
        // Also skip on devices that advertise AHB, but not the pre-requisite foreign_queue extension
        (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME))) {
        m_device_extension_names.push_back(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
    } else {
        printf("%s %s extension not supported, skipping tests\n", kSkipPrefix,
               VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    VkDevice dev = m_device->device();

    VkSamplerYcbcrConversion ycbcr_conv = VK_NULL_HANDLE;
    VkSamplerYcbcrConversionCreateInfo sycci = {};
    sycci.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkSamplerYcbcrConversionCreateInfo-format-01904");
    vkCreateSamplerYcbcrConversion(dev, &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    VkExternalFormatANDROID efa = {};
    efa.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    efa.externalFormat = AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM;
    sycci.format = VK_FORMAT_R8G8B8A8_UNORM;
    sycci.pNext = &efa;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkSamplerYcbcrConversionCreateInfo-format-01904");
    vkCreateSamplerYcbcrConversion(dev, &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, AndroidHardwareBufferPhysDevImageFormatProp2) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer GetPhysicalDeviceImageFormatProperties.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if ((DeviceExtensionSupported(gpu(), nullptr, VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)) &&
        // Also skip on devices that advertise AHB, but not the pre-requisite foreign_queue extension
        (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME))) {
        m_device_extension_names.push_back(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
    } else {
        printf("%s %s extension not supported, skipping test\n", kSkipPrefix,
               VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    if ((m_instance_api_version < VK_API_VERSION_1_1) &&
        !InstanceExtensionEnabled(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s %s extension not supported, skipping test\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    VkImageFormatProperties2 ifp = {};
    ifp.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
    VkPhysicalDeviceImageFormatInfo2 pdifi = {};
    pdifi.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
    pdifi.format = VK_FORMAT_R8G8B8A8_UNORM;
    pdifi.tiling = VK_IMAGE_TILING_OPTIMAL;
    pdifi.type = VK_IMAGE_TYPE_2D;
    pdifi.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkAndroidHardwareBufferUsageANDROID ahbu = {};
    ahbu.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_USAGE_ANDROID;
    ahbu.androidHardwareBufferUsage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ifp.pNext = &ahbu;

    // AHB_usage chained to input without a matching external image format struc chained to output
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkGetPhysicalDeviceImageFormatProperties2-pNext-01868");
    vkGetPhysicalDeviceImageFormatProperties2(m_device->phy().handle(), &pdifi, &ifp);
    m_errorMonitor->VerifyFound();

    // output struct chained, but does not include VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID usage
    VkPhysicalDeviceExternalImageFormatInfo pdeifi = {};
    pdeifi.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO;
    pdeifi.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;
    pdifi.pNext = &pdeifi;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkGetPhysicalDeviceImageFormatProperties2-pNext-01868");
    vkGetPhysicalDeviceImageFormatProperties2(m_device->phy().handle(), &pdifi, &ifp);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, AndroidHardwareBufferCreateImageView) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer image view creation.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if ((DeviceExtensionSupported(gpu(), nullptr, VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)) &&
        // Also skip on devices that advertise AHB, but not the pre-requisite foreign_queue extension
        (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME))) {
        m_device_extension_names.push_back(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
    } else {
        printf("%s %s extension not supported, skipping tests\n", kSkipPrefix,
               VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    VkDevice dev = m_device->device();

    // Allocate an AHB and fetch its properties
    AHardwareBuffer *ahb = nullptr;
    AHardwareBuffer_Desc ahb_desc = {};
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 64;
    ahb_desc.height = 64;
    ahb_desc.layers = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    // Retrieve AHB properties to make it's external format 'known'
    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props = {};
    ahb_fmt_props.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = {};
    ahb_props.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    ahb_props.pNext = &ahb_fmt_props;
    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vkGetDeviceProcAddr(dev, "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);
    pfn_GetAHBProps(dev, ahb, &ahb_props);
    AHardwareBuffer_release(ahb);

    // Give image an external format
    VkExternalFormatANDROID efa = {};
    efa.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    efa.externalFormat = ahb_fmt_props.externalFormat;

    ahb_desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    ahb_desc.width = 64;
    ahb_desc.height = 1;
    ahb_desc.layers = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);

    // Create another VkExternalFormatANDROID for test VUID-VkImageViewCreateInfo-image-02400
    VkAndroidHardwareBufferFormatPropertiesANDROID ahb_fmt_props_Ycbcr = {};
    ahb_fmt_props_Ycbcr.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
    VkAndroidHardwareBufferPropertiesANDROID ahb_props_Ycbcr = {};
    ahb_props_Ycbcr.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    ahb_props_Ycbcr.pNext = &ahb_fmt_props_Ycbcr;
    pfn_GetAHBProps(dev, ahb, &ahb_props_Ycbcr);
    AHardwareBuffer_release(ahb);

    VkExternalFormatANDROID efa_Ycbcr = {};
    efa_Ycbcr.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    efa_Ycbcr.externalFormat = ahb_fmt_props_Ycbcr.externalFormat;

    // Create the image
    VkImage img = VK_NULL_HANDLE;
    VkImageCreateInfo ici = {};
    ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ici.pNext = &efa;
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {64, 64, 1};
    ici.format = VK_FORMAT_UNDEFINED;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vkCreateImage(dev, &ici, NULL, &img);

    // Set up memory allocation
    VkDeviceMemory img_mem = VK_NULL_HANDLE;
    VkMemoryAllocateInfo mai = {};
    mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mai.allocationSize = 64 * 64 * 4;
    mai.memoryTypeIndex = 0;
    vkAllocateMemory(dev, &mai, NULL, &img_mem);

    // It shouldn't use vkGetImageMemoryRequirements for AndroidHardwareBuffer.
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "UNASSIGNED-CoreValidation-DrawState-InvalidImage");
    VkMemoryRequirements img_mem_reqs = {};
    vkGetImageMemoryRequirements(m_device->device(), img, &img_mem_reqs);
    vkBindImageMemory(dev, img, img_mem, 0);
    m_errorMonitor->VerifyFound();

    // Bind image to memory
    vkDestroyImage(dev, img, NULL);
    vkFreeMemory(dev, img_mem, NULL);
    vkCreateImage(dev, &ici, NULL, &img);
    vkAllocateMemory(dev, &mai, NULL, &img_mem);
    vkBindImageMemory(dev, img, img_mem, 0);

    // Create a YCbCr conversion, with different external format, chain to view
    VkSamplerYcbcrConversion ycbcr_conv = VK_NULL_HANDLE;
    VkSamplerYcbcrConversionCreateInfo sycci = {};
    sycci.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    sycci.pNext = &efa_Ycbcr;
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    vkCreateSamplerYcbcrConversion(dev, &sycci, NULL, &ycbcr_conv);
    VkSamplerYcbcrConversionInfo syci = {};
    syci.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO;
    syci.conversion = ycbcr_conv;

    // Create a view
    VkImageView image_view = VK_NULL_HANDLE;
    VkImageViewCreateInfo ivci = {};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.pNext = &syci;
    ivci.image = img;
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_UNDEFINED;
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    auto reset_view = [&image_view, dev]() {
        if (VK_NULL_HANDLE != image_view) vkDestroyImageView(dev, image_view, NULL);
        image_view = VK_NULL_HANDLE;
    };

    // Up to this point, no errors expected
    m_errorMonitor->VerifyNotFound();

    // Chained ycbcr conversion has different (external) format than image
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageViewCreateInfo-image-02400");
    // Also causes "unsupported format" - should be removed in future spec update
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageViewCreateInfo-None-02273");
    vkCreateImageView(dev, &ivci, NULL, &image_view);
    m_errorMonitor->VerifyFound();

    reset_view();
    vkDestroySamplerYcbcrConversion(dev, ycbcr_conv, NULL);
    sycci.pNext = &efa;
    vkCreateSamplerYcbcrConversion(dev, &sycci, NULL, &ycbcr_conv);
    syci.conversion = ycbcr_conv;

    // View component swizzle not IDENTITY
    ivci.components.r = VK_COMPONENT_SWIZZLE_B;
    ivci.components.b = VK_COMPONENT_SWIZZLE_R;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageViewCreateInfo-image-02401");
    // Also causes "unsupported format" - should be removed in future spec update
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageViewCreateInfo-None-02273");
    vkCreateImageView(dev, &ivci, NULL, &image_view);
    m_errorMonitor->VerifyFound();

    reset_view();
    ivci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    ivci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;

    // View with external format, when format is not UNDEFINED
    ivci.format = VK_FORMAT_R5G6B5_UNORM_PACK16;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageViewCreateInfo-image-02399");
    // Also causes "view format different from image format"
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageViewCreateInfo-image-01019");
    vkCreateImageView(dev, &ivci, NULL, &image_view);
    m_errorMonitor->VerifyFound();

    reset_view();
    vkDestroySamplerYcbcrConversion(dev, ycbcr_conv, NULL);
    vkDestroyImageView(dev, image_view, NULL);
    vkDestroyImage(dev, img, NULL);
    vkFreeMemory(dev, img_mem, NULL);
}

TEST_F(VkLayerTest, AndroidHardwareBufferImportBuffer) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer import as buffer.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if ((DeviceExtensionSupported(gpu(), nullptr, VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)) &&
        // Also skip on devices that advertise AHB, but not the pre-requisite foreign_queue extension
        (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME))) {
        m_device_extension_names.push_back(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
    } else {
        printf("%s %s extension not supported, skipping tests\n", kSkipPrefix,
               VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    VkDevice dev = m_device->device();

    VkDeviceMemory mem_handle = VK_NULL_HANDLE;
    auto reset_mem = [&mem_handle, dev]() {
        if (VK_NULL_HANDLE != mem_handle) vkFreeMemory(dev, mem_handle, NULL);
        mem_handle = VK_NULL_HANDLE;
    };

    PFN_vkGetAndroidHardwareBufferPropertiesANDROID pfn_GetAHBProps =
        (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vkGetDeviceProcAddr(dev, "vkGetAndroidHardwareBufferPropertiesANDROID");
    ASSERT_TRUE(pfn_GetAHBProps != nullptr);

    // AHB structs
    AHardwareBuffer *ahb = nullptr;
    AHardwareBuffer_Desc ahb_desc = {};
    VkAndroidHardwareBufferPropertiesANDROID ahb_props = {};
    ahb_props.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    VkImportAndroidHardwareBufferInfoANDROID iahbi = {};
    iahbi.sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;

    // Allocate an AHardwareBuffer
    ahb_desc.format = AHARDWAREBUFFER_FORMAT_BLOB;
    ahb_desc.usage = AHARDWAREBUFFER_USAGE_SENSOR_DIRECT_DATA;
    ahb_desc.width = 512;
    ahb_desc.height = 1;
    ahb_desc.layers = 1;
    AHardwareBuffer_allocate(&ahb_desc, &ahb);
    m_errorMonitor->SetUnexpectedError("VUID-vkGetAndroidHardwareBufferPropertiesANDROID-buffer-01884");
    pfn_GetAHBProps(dev, ahb, &ahb_props);
    iahbi.buffer = ahb;

    // Create export and import buffers
    VkExternalMemoryBufferCreateInfo ext_buf_info = {};
    ext_buf_info.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO_KHR;
    ext_buf_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;

    VkBufferCreateInfo bci = {};
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.pNext = &ext_buf_info;
    bci.size = ahb_props.allocationSize;
    bci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VkBuffer buf = VK_NULL_HANDLE;
    vkCreateBuffer(dev, &bci, NULL, &buf);
    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(dev, buf, &mem_reqs);

    // Allocation info
    VkMemoryAllocateInfo mai = vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, mem_reqs, 0);
    mai.pNext = &iahbi;  // Chained import struct
    VkPhysicalDeviceMemoryProperties memory_info;
    vkGetPhysicalDeviceMemoryProperties(gpu(), &memory_info);
    unsigned int i;
    for (i = 0; i < memory_info.memoryTypeCount; i++) {
        if ((ahb_props.memoryTypeBits & (1 << i))) {
            mai.memoryTypeIndex = i;
            break;
        }
    }
    if (i >= memory_info.memoryTypeCount) {
        printf("%s No invalid memory type index could be found; skipped.\n", kSkipPrefix);
        AHardwareBuffer_release(ahb);
        reset_mem();
        vkDestroyBuffer(dev, buf, NULL);
        return;
    }

    // Import as buffer requires format AHB_FMT_BLOB and usage AHB_USAGE_GPU_DATA_BUFFER
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-VkImportAndroidHardwareBufferInfoANDROID-buffer-01881");
    // Also causes "non-dedicated allocation format/usage" error
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkMemoryAllocateInfo-pNext-02384");
    vkAllocateMemory(dev, &mai, NULL, &mem_handle);
    m_errorMonitor->VerifyFound();

    AHardwareBuffer_release(ahb);
    reset_mem();
    vkDestroyBuffer(dev, buf, NULL);
}

TEST_F(VkLayerTest, AndroidHardwareBufferExporttBuffer) {
    TEST_DESCRIPTION("Verify AndroidHardwareBuffer export memory as AHB.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if ((DeviceExtensionSupported(gpu(), nullptr, VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)) &&
        // Also skip on devices that advertise AHB, but not the pre-requisite foreign_queue extension
        (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME))) {
        m_device_extension_names.push_back(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
    } else {
        printf("%s %s extension not supported, skipping tests\n", kSkipPrefix,
               VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    VkDevice dev = m_device->device();

    VkDeviceMemory mem_handle = VK_NULL_HANDLE;

    // Allocate device memory, no linked export struct indicating AHB handle type
    VkMemoryAllocateInfo mai = {};
    mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mai.allocationSize = 65536;
    mai.memoryTypeIndex = 0;
    vkAllocateMemory(dev, &mai, NULL, &mem_handle);

    PFN_vkGetMemoryAndroidHardwareBufferANDROID pfn_GetMemAHB =
        (PFN_vkGetMemoryAndroidHardwareBufferANDROID)vkGetDeviceProcAddr(dev, "vkGetMemoryAndroidHardwareBufferANDROID");
    ASSERT_TRUE(pfn_GetMemAHB != nullptr);

    VkMemoryGetAndroidHardwareBufferInfoANDROID mgahbi = {};
    mgahbi.sType = VK_STRUCTURE_TYPE_MEMORY_GET_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
    mgahbi.memory = mem_handle;
    AHardwareBuffer *ahb = nullptr;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-VkMemoryGetAndroidHardwareBufferInfoANDROID-handleTypes-01882");
    pfn_GetMemAHB(dev, &mgahbi, &ahb);
    m_errorMonitor->VerifyFound();

    if (ahb) AHardwareBuffer_release(ahb);
    ahb = nullptr;
    if (VK_NULL_HANDLE != mem_handle) vkFreeMemory(dev, mem_handle, NULL);
    mem_handle = VK_NULL_HANDLE;

    // Add an export struct with AHB handle type to allocation info
    VkExportMemoryAllocateInfo emai = {};
    emai.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
    emai.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    mai.pNext = &emai;

    // Create an image, do not bind memory
    VkImage img = VK_NULL_HANDLE;
    VkImageCreateInfo ici = {};
    ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.arrayLayers = 1;
    ici.extent = {128, 128, 1};
    ici.format = VK_FORMAT_R8G8B8A8_UNORM;
    ici.mipLevels = 1;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vkCreateImage(dev, &ici, NULL, &img);
    ASSERT_TRUE(VK_NULL_HANDLE != img);

    // Add image to allocation chain as dedicated info, re-allocate
    VkMemoryDedicatedAllocateInfo mdai = {VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO};
    mdai.image = img;
    emai.pNext = &mdai;
    mai.allocationSize = 0;
    vkAllocateMemory(dev, &mai, NULL, &mem_handle);
    mgahbi.memory = mem_handle;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-VkMemoryGetAndroidHardwareBufferInfoANDROID-pNext-01883");
    pfn_GetMemAHB(dev, &mgahbi, &ahb);
    m_errorMonitor->VerifyFound();

    if (ahb) AHardwareBuffer_release(ahb);
    if (VK_NULL_HANDLE != mem_handle) vkFreeMemory(dev, mem_handle, NULL);
    vkDestroyImage(dev, img, NULL);
}

#endif  // VK_USE_PLATFORM_ANDROID_KHR

TEST_F(VkLayerTest, ValidateStride) {
    TEST_DESCRIPTION("Validate Stride.");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_ci{};
    query_pool_ci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_ci.queryCount = 1;
    vkCreateQueryPool(m_device->device(), &query_pool_ci, nullptr, &query_pool);

    char data_space;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkGetQueryPoolResults-flags-00814");
    vkGetQueryPoolResults(m_device->handle(), query_pool, 0, 1, sizeof(data_space), &data_space, 1, VK_QUERY_RESULT_WAIT_BIT);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkGetQueryPoolResults-flags-00815");
    vkGetQueryPoolResults(m_device->handle(), query_pool, 0, 1, sizeof(data_space), &data_space, 1, VK_QUERY_RESULT_64_BIT);
    m_errorMonitor->VerifyFound();

    char data_space4[4];
    m_errorMonitor->ExpectSuccess();
    vkGetQueryPoolResults(m_device->handle(), query_pool, 0, 1, sizeof(data_space4), &data_space4, 4, VK_QUERY_RESULT_WAIT_BIT);
    m_errorMonitor->VerifyNotFound();

    char data_space8[8];
    m_errorMonitor->ExpectSuccess();
    vkGetQueryPoolResults(m_device->handle(), query_pool, 0, 1, sizeof(data_space8), &data_space8, 8, VK_QUERY_RESULT_64_BIT);
    m_errorMonitor->VerifyNotFound();

    uint32_t qfi = 0;
    VkBufferCreateInfo buff_create_info = {};
    buff_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buff_create_info.size = 128;
    buff_create_info.usage =
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    buff_create_info.queueFamilyIndexCount = 1;
    buff_create_info.pQueueFamilyIndices = &qfi;
    VkBufferObj buffer;
    buffer.init(*m_device, buff_create_info);

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdCopyQueryPoolResults-flags-00822");
    vkCmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool, 0, 1, buffer.handle(), 1, 1, VK_QUERY_RESULT_WAIT_BIT);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdCopyQueryPoolResults-flags-00823");
    vkCmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool, 0, 1, buffer.handle(), 1, 1, VK_QUERY_RESULT_64_BIT);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->ExpectSuccess();
    vkCmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool, 0, 1, buffer.handle(), 4, 4, VK_QUERY_RESULT_WAIT_BIT);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->ExpectSuccess();
    vkCmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool, 0, 1, buffer.handle(), 8, 8, VK_QUERY_RESULT_64_BIT);
    m_errorMonitor->VerifyNotFound();

    if (m_device->phy().features().multiDrawIndirect) {
        CreatePipelineHelper helper(*this);
        helper.InitInfo();
        helper.InitState();
        helper.CreateGraphicsPipeline();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, helper.pipeline_);

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDrawIndirect-drawCount-00476");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDrawIndirect-drawCount-00488");
        vkCmdDrawIndirect(m_commandBuffer->handle(), buffer.handle(), 0, 100, 2);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->ExpectSuccess();
        vkCmdDrawIndirect(m_commandBuffer->handle(), buffer.handle(), 0, 2, 24);
        m_errorMonitor->VerifyNotFound();

        vkCmdBindIndexBuffer(m_commandBuffer->handle(), buffer.handle(), 0, VK_INDEX_TYPE_UINT16);
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDrawIndexedIndirect-drawCount-00528");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDrawIndexedIndirect-drawCount-00540");
        vkCmdDrawIndexedIndirect(m_commandBuffer->handle(), buffer.handle(), 0, 100, 2);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->ExpectSuccess();
        vkCmdDrawIndexedIndirect(m_commandBuffer->handle(), buffer.handle(), 0, 2, 24);
        m_errorMonitor->VerifyNotFound();

        vkCmdEndRenderPass(m_commandBuffer->handle());
        m_commandBuffer->end();
    } else {
        printf("%s Test requires unsupported multiDrawIndirect feature. Skipped.\n", kSkipPrefix);
    }
    vkDestroyQueryPool(m_device->handle(), query_pool, NULL);
}
