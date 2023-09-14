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

TEST_F(NegativeImageDrm, Basic) {
    TEST_DESCRIPTION("General testing of VK_EXT_image_drm_format_modifier");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires at least Vulkan 1.1";
    }
    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    constexpr std::array<uint64_t, 2> dummy_modifiers = {0, 1};

    auto image_info = LvlInitStruct<VkImageCreateInfo>();
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.arrayLayers = 1;
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.mipLevels = 1;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    auto image_format_prop = LvlInitStruct<VkImageFormatProperties2>();
    auto image_format_info = LvlInitStruct<VkPhysicalDeviceImageFormatInfo2>();
    image_format_info.format = image_info.format;
    image_format_info.tiling = image_info.tiling;
    image_format_info.type = image_info.imageType;
    image_format_info.usage = image_info.usage;
    auto drm_format_mod_info = LvlInitStruct<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>();
    drm_format_mod_info.drmFormatModifier = dummy_modifiers[0];
    drm_format_mod_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    drm_format_mod_info.queueFamilyIndexCount = 0;
    image_format_info.pNext = (void *)&drm_format_mod_info;
    vk::GetPhysicalDeviceImageFormatProperties2(m_device->phy().handle(), &image_format_info, &image_format_prop);

    {
        VkImageFormatProperties dummy_props;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceImageFormatProperties-tiling-02248");
        vk::GetPhysicalDeviceImageFormatProperties(m_device->phy().handle(), image_info.format, image_info.imageType,
                                                   image_info.tiling, image_info.usage, image_info.flags, &dummy_props);
        m_errorMonitor->VerifyFound();
    }

    VkSubresourceLayout dummyPlaneLayout = {0, 0, 0, 0, 0};

    auto drm_format_mod_list = LvlInitStruct<VkImageDrmFormatModifierListCreateInfoEXT>();
    drm_format_mod_list.drmFormatModifierCount = dummy_modifiers.size();
    drm_format_mod_list.pDrmFormatModifiers = dummy_modifiers.data();

    auto drm_format_mod_explicit = LvlInitStruct<VkImageDrmFormatModifierExplicitCreateInfoEXT>();
    drm_format_mod_explicit.drmFormatModifierPlaneCount = 1;
    drm_format_mod_explicit.pPlaneLayouts = &dummyPlaneLayout;

    // No pNext
    CreateImageTest(*this, &image_info, "VUID-VkImageCreateInfo-tiling-02261");

    // Having wrong size, arrayPitch and depthPitch in VkSubresourceLayout
    dummyPlaneLayout.size = 1;
    dummyPlaneLayout.arrayPitch = 1;
    dummyPlaneLayout.depthPitch = 1;

    image_info.pNext = (void *)&drm_format_mod_explicit;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-size-02267");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-arrayPitch-02268");
    CreateImageTest(*this, &image_info, "VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-depthPitch-02269");

    // reset dummy plane layout
    memset(&dummyPlaneLayout, 0, sizeof(dummyPlaneLayout));

    auto drm_format_modifier = LvlInitStruct<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>();
    drm_format_modifier.drmFormatModifier = dummy_modifiers[1];
    image_format_info.pNext = &drm_format_modifier;
    VkResult result = vk::GetPhysicalDeviceImageFormatProperties2(m_device->phy().handle(), &image_format_info, &image_format_prop);
    if (result == VK_ERROR_FORMAT_NOT_SUPPORTED) {
        printf("Format VK_FORMAT_R8G8B8A8_UNORM not supported with format modifiers, Skipping the remaining tests.\n");
        return;
    }
    VkImage image = VK_NULL_HANDLE;
    // Postive check if only 1
    image_info.pNext = (void *)&drm_format_mod_list;
    vk::CreateImage(device(), &image_info, nullptr, &image);
    vk::DestroyImage(device(), image, nullptr);

    image_info.pNext = (void *)&drm_format_mod_explicit;
    vk::CreateImage(device(), &image_info, nullptr, &image);
    vk::DestroyImage(device(), image, nullptr);

    // Having both in pNext
    drm_format_mod_explicit.pNext = (void *)&drm_format_mod_list;
    CreateImageTest(*this, &image_info, "VUID-VkImageCreateInfo-tiling-02261");

    // Only 1 pNext but wrong tiling
    image_info.pNext = (void *)&drm_format_mod_list;
    image_info.tiling = VK_IMAGE_TILING_LINEAR;
    CreateImageTest(*this, &image_info, "VUID-VkImageCreateInfo-pNext-02262");
}

TEST_F(NegativeImageDrm, ImageFormatInfo) {
    TEST_DESCRIPTION("Validate VkPhysicalDeviceImageFormatInfo2.");

    AddRequiredExtensions(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    auto image_drm_format_modifier = LvlInitStruct<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>();

    auto image_format_info = LvlInitStruct<VkPhysicalDeviceImageFormatInfo2>(&image_drm_format_modifier);
    image_format_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_format_info.type = VK_IMAGE_TYPE_2D;
    image_format_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_format_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_format_info.flags = 0;

    auto image_format_properties = LvlInitStruct<VkImageFormatProperties2>();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceImageFormatInfo2-tiling-02249");
    vk::GetPhysicalDeviceImageFormatProperties2KHR(gpu(), &image_format_info, &image_format_properties);
    m_errorMonitor->VerifyFound();

    image_format_info.pNext = nullptr;
    image_format_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceImageFormatInfo2-tiling-02249");
    vk::GetPhysicalDeviceImageFormatProperties2KHR(gpu(), &image_format_info, &image_format_properties);
    m_errorMonitor->VerifyFound();

    image_format_info.pNext = &image_drm_format_modifier;
    image_drm_format_modifier.sharingMode = VK_SHARING_MODE_CONCURRENT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceImageDrmFormatModifierInfoEXT-sharingMode-02315");
    vk::GetPhysicalDeviceImageFormatProperties2KHR(gpu(), &image_format_info, &image_format_properties);
    m_errorMonitor->VerifyFound();
    image_drm_format_modifier.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto format_list = LvlInitStruct<VkImageFormatListCreateInfo>(&image_drm_format_modifier);
    format_list.viewFormatCount = 0;  // Invalid
    image_format_info.pNext = &format_list;
    image_format_info.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceImageFormatInfo2-tiling-02313");
    vk::GetPhysicalDeviceImageFormatProperties2KHR(gpu(), &image_format_info, &image_format_properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageDrm, GetImageSubresourceLayoutPlane) {
    TEST_DESCRIPTION("Try to get image subresource layout for drm image plane 3 when it only has 2");

    // Try to enable 1.2 since all required extensions were promoted
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    // By this point we already know if all required device extensions are supported
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported().c_str() << " extensions not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkFormat format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    auto modifiers_list = LvlInitStruct<VkDrmFormatModifierPropertiesListEXT>();
    auto format_properties = LvlInitStruct<VkFormatProperties2>(&modifiers_list);
    vk::GetPhysicalDeviceFormatProperties2(m_device->phy().handle(), format, &format_properties);

    if (modifiers_list.drmFormatModifierCount == 0) {
        GTEST_SKIP() << "No drm format modifier found for image format VK_FORMAT_G8_B8R8_2PLANE_420_UNORM.";
    }

    std::vector<VkDrmFormatModifierPropertiesEXT> modifiers_properties(modifiers_list.drmFormatModifierCount);
    modifiers_list.pDrmFormatModifierProperties = modifiers_properties.data();
    vk::GetPhysicalDeviceFormatProperties2(m_device->phy().handle(), format, &format_properties);

    size_t modifier_index = 0u;
    for (; modifier_index < modifiers_properties.size(); ++modifier_index) {
        if (modifiers_properties[modifier_index].drmFormatModifierPlaneCount < 3) {
            break;
        }
    }

    if (modifier_index >= modifiers_properties.size()) {
        GTEST_SKIP() << "No drm modifier found with less than 3 planes needed for testing.";
    }

    uint64_t chosen_drm_modifier = modifiers_properties[modifier_index].drmFormatModifier;
    auto list_create_info = LvlInitStruct<VkImageDrmFormatModifierListCreateInfoEXT>();
    list_create_info.drmFormatModifierCount = 1u;
    list_create_info.pDrmFormatModifiers = &chosen_drm_modifier;
    auto create_info = LvlInitStruct<VkImageCreateInfo>(&list_create_info);
    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.format = format;
    create_info.extent.width = 64;
    create_info.extent.height = 64;
    create_info.extent.depth = 1;
    create_info.mipLevels = 1;
    create_info.arrayLayers = 1;
    create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    create_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageObj image{m_device};
    image.init_no_mem(*m_device, create_info);
    if (image.initialized() == false) {
        GTEST_SKIP() << "Failed to create image.";
    }

    // Try to get layout for plane 3 when we only have 2
    VkImageSubresource subresource{};
    subresource.aspectMask = VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT;
    VkSubresourceLayout layout{};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-tiling-02271");
    vk::GetImageSubresourceLayout(m_device->handle(), image.handle(), &subresource, &layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageDrm, DeviceImageMemoryRequirements) {
    TEST_DESCRIPTION("Validate usage of VkDeviceImageMemoryRequirementsKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkSubresourceLayout planeLayout = {0, 0, 0, 0, 0};
    auto drm_format_modifier_create_info = LvlInitStruct<VkImageDrmFormatModifierExplicitCreateInfoEXT>();
    drm_format_modifier_create_info.drmFormatModifierPlaneCount = 1;
    drm_format_modifier_create_info.pPlaneLayouts = &planeLayout;

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>(&drm_format_modifier_create_info);
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.arrayLayers = 1;

    auto device_image_memory_requirements = LvlInitStruct<VkDeviceImageMemoryRequirementsKHR>();
    device_image_memory_requirements.pCreateInfo = &image_create_info;
    device_image_memory_requirements.planeAspect = VK_IMAGE_ASPECT_COLOR_BIT;

    auto memory_requirements = LvlInitStruct<VkMemoryRequirements2>();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06776");
    vk::GetDeviceImageMemoryRequirementsKHR(device(), &device_image_memory_requirements, &memory_requirements);
    m_errorMonitor->VerifyFound();
}
