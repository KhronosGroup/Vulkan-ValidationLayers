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
    InitBasicImageDrm();
    if (::testing::Test::IsSkipped()) return;

    std::vector<uint64_t> mods = GetFormatModifier(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    if (mods.empty()) {
        GTEST_SKIP() << "No valid Format Modifier found";
    }

    auto image_info = vku::InitStruct<VkImageCreateInfo>();
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.arrayLayers = 1;
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.mipLevels = 1;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    auto image_format_prop = vku::InitStruct<VkImageFormatProperties2>();
    auto image_format_info = vku::InitStruct<VkPhysicalDeviceImageFormatInfo2>();
    image_format_info.format = image_info.format;
    image_format_info.tiling = image_info.tiling;
    image_format_info.type = image_info.imageType;
    image_format_info.usage = image_info.usage;
    auto drm_format_mod_info = vku::InitStruct<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>();
    drm_format_mod_info.drmFormatModifier = mods[0];
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

    VkSubresourceLayout fake_plane_layout = {0, 0, 0, 0, 0};

    auto drm_format_mod_list = vku::InitStruct<VkImageDrmFormatModifierListCreateInfoEXT>();
    drm_format_mod_list.drmFormatModifierCount = mods.size();
    drm_format_mod_list.pDrmFormatModifiers = mods.data();

    auto drm_format_mod_explicit = vku::InitStruct<VkImageDrmFormatModifierExplicitCreateInfoEXT>();
    drm_format_mod_explicit.drmFormatModifierPlaneCount = 1;
    drm_format_mod_explicit.pPlaneLayouts = &fake_plane_layout;

    // No pNext
    CreateImageTest(*this, &image_info, "VUID-VkImageCreateInfo-tiling-02261");

    // Having wrong size, arrayPitch and depthPitch in VkSubresourceLayout
    fake_plane_layout.size = 1;
    fake_plane_layout.arrayPitch = 1;
    fake_plane_layout.depthPitch = 1;

    image_info.pNext = (void *)&drm_format_mod_explicit;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-size-02267");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-arrayPitch-02268");
    CreateImageTest(*this, &image_info, "VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-depthPitch-02269");

    // reset dummy plane layout
    memset(&fake_plane_layout, 0, sizeof(fake_plane_layout));

    auto drm_format_modifier = vku::InitStruct<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>();
    drm_format_modifier.drmFormatModifier = mods[1];
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
    InitBasicImageDrm();
    if (::testing::Test::IsSkipped()) return;

    auto image_drm_format_modifier = vku::InitStruct<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>();

    auto image_format_info = vku::InitStruct<VkPhysicalDeviceImageFormatInfo2>(&image_drm_format_modifier);
    image_format_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_format_info.type = VK_IMAGE_TYPE_2D;
    image_format_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_format_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_format_info.flags = 0;

    auto image_format_properties = vku::InitStruct<VkImageFormatProperties2>();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceImageFormatInfo2-tiling-02249");
    vk::GetPhysicalDeviceImageFormatProperties2KHR(gpu(), &image_format_info, &image_format_properties);
    m_errorMonitor->VerifyFound();

    image_format_info.pNext = nullptr;
    image_format_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceImageFormatInfo2-tiling-02249");
    vk::GetPhysicalDeviceImageFormatProperties2KHR(gpu(), &image_format_info, &image_format_properties);
    m_errorMonitor->VerifyFound();

    auto format_list = vku::InitStruct<VkImageFormatListCreateInfo>(&image_drm_format_modifier);
    format_list.viewFormatCount = 0;  // Invalid
    image_format_info.pNext = &format_list;
    image_format_info.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceImageFormatInfo2-tiling-02313");
    vk::GetPhysicalDeviceImageFormatProperties2KHR(gpu(), &image_format_info, &image_format_properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageDrm, GetImageSubresourceLayoutPlane) {
    TEST_DESCRIPTION("Try to get image subresource layout for drm image plane 3 when it only has 2");
    InitBasicImageDrm();
    if (::testing::Test::IsSkipped()) return;

    VkFormat format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    std::vector<uint64_t> mods = GetFormatModifier(format, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT, 2);
    if (mods.empty()) {
        GTEST_SKIP() << "No valid Format Modifier found";
    }

    auto list_create_info = vku::InitStruct<VkImageDrmFormatModifierListCreateInfoEXT>();
    list_create_info.drmFormatModifierCount = mods.size();
    list_create_info.pDrmFormatModifiers = mods.data();
    auto create_info = vku::InitStruct<VkImageCreateInfo>(&list_create_info);
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

    for (uint64_t mod : mods) {
        auto drm_format_modifier = vku::InitStruct<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>();
        drm_format_modifier.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        drm_format_modifier.drmFormatModifier = mod;
        auto image_info = vku::InitStruct<VkPhysicalDeviceImageFormatInfo2>(&drm_format_modifier);
        image_info.format = format;
        image_info.type = create_info.imageType;
        image_info.tiling = create_info.tiling;
        image_info.usage = create_info.usage;
        image_info.flags = create_info.flags;
        auto image_properties = vku::InitStruct<VkImageFormatProperties2>();
        if (vk::GetPhysicalDeviceImageFormatProperties2(gpu(), &image_info, &image_properties) != VK_SUCCESS) {
            // Works with Mesa, Pixel 7 doesn't support this combo
            GTEST_SKIP() << "Required formats/features not supported";
        }
    }

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
    InitBasicImageDrm();
    if (::testing::Test::IsSkipped()) return;

    VkSubresourceLayout planeLayout = {0, 0, 0, 0, 0};
    auto drm_format_modifier_create_info = vku::InitStruct<VkImageDrmFormatModifierExplicitCreateInfoEXT>();
    drm_format_modifier_create_info.drmFormatModifierPlaneCount = 1;
    drm_format_modifier_create_info.pPlaneLayouts = &planeLayout;

    auto image_create_info = vku::InitStruct<VkImageCreateInfo>(&drm_format_modifier_create_info);
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.arrayLayers = 1;

    auto device_image_memory_requirements = vku::InitStruct<VkDeviceImageMemoryRequirementsKHR>();
    device_image_memory_requirements.pCreateInfo = &image_create_info;
    device_image_memory_requirements.planeAspect = VK_IMAGE_ASPECT_COLOR_BIT;

    auto memory_requirements = vku::InitStruct<VkMemoryRequirements2>();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06776");
    vk::GetDeviceImageMemoryRequirementsKHR(device(), &device_image_memory_requirements, &memory_requirements);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageDrm, ImageSubresourceRangeAspectMask) {
    TEST_DESCRIPTION("Test creating Image with invalid VkImageSubresourceRange aspectMask.");
    InitBasicImageDrm();
    if (::testing::Test::IsSkipped()) return;

    VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    if (!ImageFormatAndFeaturesSupported(gpu(), mp_format, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    VkImageObj image(m_device);
    image.Init(32, 32, 1, mp_format, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL);

    vk_testing::SamplerYcbcrConversion conversion(*m_device, mp_format);
    auto conversion_info = conversion.ConversionInfo();
    VkImageViewCreateInfo ivci = vku::InitStructHelper(&conversion_info);
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = mp_format;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT;

    m_errorMonitor->SetUnexpectedError("UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
    CreateImageViewTest(*this, &ivci, "VUID-VkImageSubresourceRange-aspectMask-02278");
}

TEST_F(NegativeImageDrm, MutableFormat) {
    TEST_DESCRIPTION("use VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT with no VkImageFormatListCreateInfo.");
    InitBasicImageDrm();
    if (::testing::Test::IsSkipped()) return;

    std::vector<uint64_t> mods = GetFormatModifier(VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
    if (mods.empty()) {
        GTEST_SKIP() << "No valid Format Modifier found";
    }

    auto mod_list = vku::InitStruct<VkImageDrmFormatModifierListCreateInfoEXT>();
    mod_list.pDrmFormatModifiers = mods.data();
    mod_list.drmFormatModifierCount = mods.size();

    auto image_info = vku::InitStruct<VkImageCreateInfo>(&mod_list);
    image_info.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.arrayLayers = 1;
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.mipLevels = 1;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    CreateImageTest(*this, &image_info, "VUID-VkImageCreateInfo-tiling-02353");

    auto format_list = vku::InitStruct<VkImageFormatListCreateInfo>();
    format_list.viewFormatCount = 0;
    mod_list.pNext = &format_list;
    CreateImageTest(*this, &image_info, "VUID-VkImageCreateInfo-tiling-02353");
}

TEST_F(NegativeImageDrm, CompressionControl) {
    TEST_DESCRIPTION("mix VK_EXT_image_compression_control with DRM.");
    AddRequiredExtensions(VK_EXT_IMAGE_COMPRESSION_CONTROL_EXTENSION_NAME);
    InitBasicImageDrm();
    if (::testing::Test::IsSkipped()) return;

    auto compression_control = vku::InitStruct<VkImageCompressionControlEXT>();
    compression_control.flags = VK_IMAGE_COMPRESSION_DEFAULT_EXT;
    compression_control.compressionControlPlaneCount = 1;
    compression_control.pFixedRateFlags = nullptr;

    VkSubresourceLayout fake_plane_layout = {0, 0, 0, 0, 0};
    auto drm_format_mod_explicit = vku::InitStruct<VkImageDrmFormatModifierExplicitCreateInfoEXT>(&compression_control);
    drm_format_mod_explicit.drmFormatModifierPlaneCount = 1;
    drm_format_mod_explicit.pPlaneLayouts = &fake_plane_layout;

    auto image_info = vku::InitStruct<VkImageCreateInfo>(&drm_format_mod_explicit);
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.arrayLayers = 1;
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.mipLevels = 1;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    CreateImageTest(*this, &image_info, "VUID-VkImageCreateInfo-pNext-06746");
}

TEST_F(NegativeImageDrm, GetImageDrmFormatModifierProperties) {
    TEST_DESCRIPTION("Use vkGetImageDrmFormatModifierPropertiesEXT");
    InitBasicImageDrm();
    if (::testing::Test::IsSkipped()) return;

    auto image_info = vku::InitStruct<VkImageCreateInfo>();
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.extent = {128, 128, 1};
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;  // not DRM tiling
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vk_testing::Image image(*m_device, image_info);

    auto props = vku::InitStruct<VkImageDrmFormatModifierPropertiesEXT>();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageDrmFormatModifierPropertiesEXT-image-02272");
    vk::GetImageDrmFormatModifierPropertiesEXT(device(), image.handle(), &props);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageDrmFormatModifierPropertiesEXT-image-parameter");
    VkImage bad_image = CastFromUint64<VkImage>(0xFFFFEEEE);
    vk::GetImageDrmFormatModifierPropertiesEXT(device(), bad_image, &props);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageDrm, PhysicalDeviceImageDrmFormatModifierInfo) {
    TEST_DESCRIPTION("Use vkPhysicalDeviceImageDrmFormatModifierInfo with VK_SHARING_MODE_CONCURRENT");
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    InitBasicImageDrm();
    if (::testing::Test::IsSkipped()) return;

    auto drm_format_modifier = vku::InitStruct<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>();
    drm_format_modifier.sharingMode = VK_SHARING_MODE_CONCURRENT;
    drm_format_modifier.queueFamilyIndexCount = 0;

    auto external_image_info = vku::InitStruct<VkPhysicalDeviceExternalImageFormatInfo>(&drm_format_modifier);
    external_image_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    auto image_info = vku::InitStruct<VkPhysicalDeviceImageFormatInfo2>(&external_image_info);
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.type = VK_IMAGE_TYPE_2D;
    image_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_info.flags = 0;

    auto image_properties = vku::InitStruct<VkImageFormatProperties2>();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceImageDrmFormatModifierInfoEXT-sharingMode-02315");
    vk::GetPhysicalDeviceImageFormatProperties2(gpu(), &image_info, &image_properties);
    m_errorMonitor->VerifyFound();

    drm_format_modifier.queueFamilyIndexCount = 2;
    drm_format_modifier.pQueueFamilyIndices = nullptr;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceImageDrmFormatModifierInfoEXT-sharingMode-02314");
    vk::GetPhysicalDeviceImageFormatProperties2(gpu(), &image_info, &image_properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeImageDrm, PhysicalDeviceImageDrmFormatModifierInfoQuery) {
    TEST_DESCRIPTION("Use vkPhysicalDeviceImageDrmFormatModifierInfo with VK_SHARING_MODE_CONCURRENT");
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    InitBasicImageDrm();
    if (::testing::Test::IsSkipped()) return;

    uint32_t queue_family_property_count = 0;
    vk::GetPhysicalDeviceQueueFamilyProperties2(gpu(), &queue_family_property_count, nullptr);
    if (queue_family_property_count < 2) {
        GTEST_SKIP() << "pQueueFamilyPropertyCount is not 2 or more";
    }
    uint32_t queue_family_indices[2] = {0, 1};

    auto drm_format_modifier = vku::InitStruct<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>();
    drm_format_modifier.sharingMode = VK_SHARING_MODE_CONCURRENT;
    drm_format_modifier.queueFamilyIndexCount = 2;
    drm_format_modifier.pQueueFamilyIndices = queue_family_indices;

    auto external_image_info = vku::InitStruct<VkPhysicalDeviceExternalImageFormatInfo>(&drm_format_modifier);
    external_image_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    auto image_info = vku::InitStruct<VkPhysicalDeviceImageFormatInfo2>(&external_image_info);
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.type = VK_IMAGE_TYPE_2D;
    image_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_info.flags = 0;

    auto image_properties = vku::InitStruct<VkImageFormatProperties2>();

    // Count too large
    queue_family_indices[0] = queue_family_property_count + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceImageDrmFormatModifierInfoEXT-sharingMode-02316");
    vk::GetPhysicalDeviceImageFormatProperties2(gpu(), &image_info, &image_properties);
    m_errorMonitor->VerifyFound();

    // Not unique indices
    queue_family_indices[0] = 0;
    queue_family_indices[1] = 0;
    drm_format_modifier.queueFamilyIndexCount = queue_family_property_count;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceImageDrmFormatModifierInfoEXT-sharingMode-02316");
    vk::GetPhysicalDeviceImageFormatProperties2(gpu(), &image_info, &image_properties);
    m_errorMonitor->VerifyFound();
}
