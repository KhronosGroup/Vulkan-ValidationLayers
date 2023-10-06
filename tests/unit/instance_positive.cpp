/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "generated/vk_extension_helper.h"

TEST_F(PositiveInstance, TwoInstances) {
    TEST_DESCRIPTION("Create two instances before destroy");

    RETURN_IF_SKIP(InitFramework())

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    VkInstance i1, i2, i3;

    VkInstanceCreateInfo ici = vku::InitStructHelper();
    ici.enabledLayerCount = instance_layers_.size();
    ici.ppEnabledLayerNames = instance_layers_.data();

    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&ici, nullptr, &i1));

    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&ici, nullptr, &i2));
    ASSERT_NO_FATAL_FAILURE(vk::DestroyInstance(i2, nullptr));

    ASSERT_EQ(VK_SUCCESS, vk::CreateInstance(&ici, nullptr, &i3));
    ASSERT_NO_FATAL_FAILURE(vk::DestroyInstance(i3, nullptr));

    ASSERT_NO_FATAL_FAILURE(vk::DestroyInstance(i1, nullptr));
}

TEST_F(PositiveInstance, NullFunctionPointer) {
    TEST_DESCRIPTION("On 1_0 instance , call GetDeviceProcAddr on promoted 1_1 device-level entrypoint");
    SetTargetApiVersion(VK_API_VERSION_1_0);

    AddRequiredExtensions(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())

    auto fpGetBufferMemoryRequirements =
        (PFN_vkGetBufferMemoryRequirements2)vk::GetDeviceProcAddr(m_device->device(), "vkGetBufferMemoryRequirements2");
    if (fpGetBufferMemoryRequirements) {
        m_errorMonitor->SetError("Null was expected!");
    }
}

TEST_F(PositiveInstance, ValidationInstanceExtensions) {
    RETURN_IF_SKIP(Init())

    std::string layer_name = "VK_LAYER_KHRONOS_validation";
    std::vector<std::string> extensions = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                                           VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME};
    uint32_t property_count;
    vk::EnumerateInstanceExtensionProperties(layer_name.c_str(), &property_count, NULL);
    std::vector<VkExtensionProperties> properties(property_count);
    vk::EnumerateInstanceExtensionProperties(layer_name.c_str(), &property_count, properties.data());
    for (size_t i = 0; i < extensions.size(); i++) {
        bool found = false;
        for (auto props : properties) {
            if (!strcmp(props.extensionName, extensions[i].c_str())) {
                found = true;
                break;
            }
        }
        if (!found) {
            FAIL() << "Validation layer is missing extension " << extensions[i].c_str();
        }
    }
}

TEST_F(PositiveInstance, ValidEnumBeforeLogicalDevice) {
    TEST_DESCRIPTION("Call a VkPhysicalDevice query API that uses an enum that is only valid with a promoted extension");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitFramework())
    // RETURN_IF_SKIP(InitState())

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR;
    ci.extent = {256, 256, 1};
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = NULL;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Verify formats
    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
    ImageFormatIsSupported(instance(), gpu(), ci, features);
}
