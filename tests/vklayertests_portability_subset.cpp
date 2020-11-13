/*
 * Copyright (c) 2020 The Khronos Group Inc.
 * Copyright (c) 2020 Valve Corporation
 * Copyright (c) 2020 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Nathaniel Cesario <nathaniel@lunarg.com>
 */

#include "cast_utils.h"
#include "layer_validation_tests.h"

class VkPortabilitySubsetTest : public VkLayerTest {
  public:
    void InitPortabilitySubsetFramework() {
        // VK_KHR_portability_subset extension dependencies
        instance_extensions_.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

        InitFramework(m_errorMonitor, nullptr);
    }
};

TEST_F(VkPortabilitySubsetTest, ValidatePortabilityCreateDevice) {
    TEST_DESCRIPTION("Portability: CreateDevice called and VK_KHR_portability_subset not enabled");

    ASSERT_NO_FATAL_FAILURE(InitPortabilitySubsetFramework());

    bool portability_supported = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    if (!portability_supported) {
        printf("%s Test requires VK_KHR_portability_subset, skipping\n", kSkipPrefix);
        return;
    }

    vk_testing::PhysicalDevice phys_device(gpu());

    // request all queues
    const std::vector<VkQueueFamilyProperties> queue_props = phys_device.queue_properties();
    vk_testing::QueueCreateInfoArray queue_info(phys_device.queue_properties());

    // Only request creation with queuefamilies that have at least one queue
    std::vector<VkDeviceQueueCreateInfo> create_queue_infos;
    auto qci = queue_info.data();
    for (uint32_t j = 0; j < queue_info.size(); ++j) {
        if (qci[j].queueCount) {
            create_queue_infos.push_back(qci[j]);
        }
    }

    VkDeviceCreateInfo dev_info = {};
    dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_info.pNext = nullptr;
    dev_info.queueCreateInfoCount = create_queue_infos.size();
    dev_info.pQueueCreateInfos = create_queue_infos.data();
    dev_info.enabledLayerCount = 0;
    dev_info.ppEnabledLayerNames = NULL;
    dev_info.enabledExtensionCount = 0;
    dev_info.ppEnabledExtensionNames =
        nullptr;  // VK_KHR_portability_subset not included in enabled extensions should trigger 04451

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-pProperties-04451");
    VkDevice device;
    vk::CreateDevice(gpu(), &dev_info, nullptr, &device);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkPortabilitySubsetTest, PortabilityCreateEvent) {
    TEST_DESCRIPTION("Portability: CreateEvent when not supported");

    ASSERT_NO_FATAL_FAILURE(InitPortabilitySubsetFramework());

    bool portability_supported = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    if (!portability_supported) {
        printf("%s Test requires VK_KHR_portability_subset, skipping\n", kSkipPrefix);
        return;
    }
    m_device_extension_names.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);

    auto portability_feature = lvl_init_struct<VkPhysicalDevicePortabilitySubsetFeaturesKHR>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&portability_feature);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    portability_feature.events = VK_FALSE;  // Make sure events are disabled

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateEvent-events-04468");
    VkEventCreateInfo eci = {VK_STRUCTURE_TYPE_EVENT_CREATE_INFO, nullptr, 0};
    VkEvent event;
    vk::CreateEvent(m_device->device(), &eci, nullptr, &event);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkPortabilitySubsetTest, CreateImage) {
    TEST_DESCRIPTION("Portability: CreateImage - VUIDs 04459, 04460");

    ASSERT_NO_FATAL_FAILURE(InitPortabilitySubsetFramework());

    bool portability_supported = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    if (!portability_supported) {
        printf("%s Test requires VK_KHR_portability_subset, skipping\n", kSkipPrefix);
        return;
    }
    m_device_extension_names.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);

    auto portability_feature = lvl_init_struct<VkPhysicalDevicePortabilitySubsetFeaturesKHR>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&portability_feature);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    // Make sure image features are disabled via portability extension
    portability_feature.imageView2DOn3DImage = VK_FALSE;
    portability_feature.multisampleArrayImage = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkImageCreateInfo ci;
    ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ci.pNext = NULL;
    ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent.width = 512;
    ci.extent.height = 64;
    ci.extent.depth = 1;
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = nullptr;
    ci.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    CreateImageTest(*this, &ci, "VUID-VkImageCreateInfo-imageView2DOn3DImage-04459");

    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.flags = 0;
    ci.samples = VK_SAMPLE_COUNT_2_BIT;
    ci.arrayLayers = 2;
    CreateImageTest(*this, &ci, "VUID-VkImageCreateInfo-multisampleArrayImage-04460");
}
