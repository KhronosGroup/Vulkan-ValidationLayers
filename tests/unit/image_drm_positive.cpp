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

TEST_F(PositiveImageDrm, Basic) {
    // See https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/2610
    TEST_DESCRIPTION("Create image and imageView using VK_EXT_image_drm_format_modifier");

    SetTargetApiVersion(VK_API_VERSION_1_1);  // for extension dependencies
    AddRequiredExtensions(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD, skipping tests";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    // we just hope that one of these formats supports modifiers
    // for more detailed checking, we could also check multi-planar formats.
    auto format_list = {
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_B8G8R8A8_SRGB,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_R8G8B8A8_SRGB,
    };

    for (auto format : format_list) {
        std::vector<uint64_t> mods;

        // get general features and modifiers
        auto modp = LvlInitStruct<VkDrmFormatModifierPropertiesListEXT>();
        auto fmtp = LvlInitStruct<VkFormatProperties2>(&modp);

        vk::GetPhysicalDeviceFormatProperties2(gpu(), format, &fmtp);

        if (modp.drmFormatModifierCount > 0) {
            // the first call to vkGetPhysicalDeviceFormatProperties2 did only
            // retrieve the number of modifiers, we now have to retrieve
            // the modifiers
            std::vector<VkDrmFormatModifierPropertiesEXT> mod_props(modp.drmFormatModifierCount);
            modp.pDrmFormatModifierProperties = mod_props.data();

            vk::GetPhysicalDeviceFormatProperties2(gpu(), format, &fmtp);

            for (auto i = 0u; i < modp.drmFormatModifierCount; ++i) {
                auto &mod = modp.pDrmFormatModifierProperties[i];
                auto features = VK_FORMAT_FEATURE_TRANSFER_DST_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;

                if ((mod.drmFormatModifierTilingFeatures & features) != features) {
                    continue;
                }

                mods.push_back(mod.drmFormatModifier);
            }
        }

        if (mods.empty()) {
            continue;
        }

        // create image
        auto ci = LvlInitStruct<VkImageCreateInfo>();
        ci.flags = 0;
        ci.imageType = VK_IMAGE_TYPE_2D;
        ci.format = format;
        ci.extent = {128, 128, 1};
        ci.mipLevels = 1;
        ci.arrayLayers = 1;
        ci.samples = VK_SAMPLE_COUNT_1_BIT;
        ci.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
        ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        auto mod_list = LvlInitStruct<VkImageDrmFormatModifierListCreateInfoEXT>();
        mod_list.pDrmFormatModifiers = mods.data();
        mod_list.drmFormatModifierCount = mods.size();
        ci.pNext = &mod_list;

        VkImage image;
        VkResult err = vk::CreateImage(device(), &ci, nullptr, &image);
        ASSERT_VK_SUCCESS(err);

        // bind memory
        VkPhysicalDeviceMemoryProperties phys_mem_props;
        vk::GetPhysicalDeviceMemoryProperties(gpu(), &phys_mem_props);
        VkMemoryRequirements mem_reqs;
        vk::GetImageMemoryRequirements(device(), image, &mem_reqs);
        VkDeviceMemory mem_obj = VK_NULL_HANDLE;
        VkMemoryPropertyFlagBits mem_props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        for (uint32_t type = 0; type < phys_mem_props.memoryTypeCount; type++) {
            if ((mem_reqs.memoryTypeBits & (1 << type)) &&
                ((phys_mem_props.memoryTypes[type].propertyFlags & mem_props) == mem_props)) {
                auto alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
                alloc_info.allocationSize = mem_reqs.size;
                alloc_info.memoryTypeIndex = type;
                ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &alloc_info, nullptr, &mem_obj));
                break;
            }
        }

        ASSERT_NE((VkDeviceMemory)VK_NULL_HANDLE, mem_obj);
        ASSERT_VK_SUCCESS(vk::BindImageMemory(device(), image, mem_obj, 0));

        // create image view
        VkImageViewCreateInfo ivci = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            nullptr,
            0,
            image,
            VK_IMAGE_VIEW_TYPE_2D,
            format,
            {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
             VK_COMPONENT_SWIZZLE_IDENTITY},
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        };

        CreateImageViewTest(*this, &ivci);

        // for more detailed checking, we could export the image to dmabuf
        // and then import it again (using VkImageDrmFormatModifierExplicitCreateInfoEXT)

        vk::FreeMemory(device(), mem_obj, nullptr);
        vk::DestroyImage(device(), image, nullptr);
    }
}

TEST_F(PositiveImageDrm, ExternalMemory) {
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5649
    TEST_DESCRIPTION(
        "Create image with VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT and VkExternalMemoryImageCreateInfo in the pNext chain");

    SetTargetApiVersion(VK_API_VERSION_1_1);  // for extension dependencies
    AddRequiredExtensions(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD, skipping tests";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    std::vector<uint64_t> mods;

    const auto format = VK_FORMAT_R8G8B8A8_UNORM;

    // Get info needed to fill out VkImageDrmFormatModifierListCreateInfoEXT
    {
        auto modp = LvlInitStruct<VkDrmFormatModifierPropertiesListEXT>();
        auto fmtp = LvlInitStruct<VkFormatProperties2>(&modp);
        vk::GetPhysicalDeviceFormatProperties2(gpu(), format, &fmtp);
        if (modp.drmFormatModifierCount == 0) {
            GTEST_SKIP() << "drmFormatModifierCount equal to 0";
        }
        std::vector<VkDrmFormatModifierPropertiesEXT> mod_props(modp.drmFormatModifierCount);
        modp.pDrmFormatModifierProperties = mod_props.data();

        vk::GetPhysicalDeviceFormatProperties2(gpu(), format, &fmtp);

        for (uint32_t i = 0; i < modp.drmFormatModifierCount; ++i) {
            auto &mod = modp.pDrmFormatModifierProperties[i];
            auto features = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;

            if ((mod.drmFormatModifierTilingFeatures & features) != features) {
                continue;
            }

            mods.push_back(mod.drmFormatModifier);
        }
    }

    if (mods.empty()) {
        GTEST_SKIP() << "Skip test";
    }

    auto external_info = LvlInitStruct<VkExternalMemoryImageCreateInfo>();
    external_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    // handleTypes needs to be assigned to trigger the behavior we want
    assert(external_info.handleTypes);

    auto drm_info = LvlInitStruct<VkImageDrmFormatModifierListCreateInfoEXT>(&external_info);
    drm_info.drmFormatModifierCount = size32(mods);
    drm_info.pDrmFormatModifiers = mods.data();

    auto ci = LvlInitStruct<VkImageCreateInfo>(&drm_info);
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = format;
    ci.extent = {128, 128, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    {
        auto drm_format_modifier = LvlInitStruct<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>();
        drm_format_modifier.sharingMode = ci.sharingMode;
        drm_format_modifier.queueFamilyIndexCount = ci.queueFamilyIndexCount;
        drm_format_modifier.pQueueFamilyIndices = ci.pQueueFamilyIndices;
        auto external_image_info = LvlInitStruct<VkPhysicalDeviceExternalImageFormatInfo>(&drm_format_modifier);
        external_image_info.handleType = static_cast<VkExternalMemoryHandleTypeFlagBits>(external_info.handleTypes);
        auto image_info = LvlInitStruct<VkPhysicalDeviceImageFormatInfo2>(&external_image_info);
        image_info.format = ci.format;
        image_info.type = ci.imageType;
        image_info.tiling = ci.tiling;
        image_info.usage = ci.usage;
        image_info.flags = ci.flags;

        auto external_image_properties = LvlInitStruct<VkExternalImageFormatProperties>();
        auto image_properties = LvlInitStruct<VkImageFormatProperties2>(&external_image_properties);

        if (const auto result = vk::GetPhysicalDeviceImageFormatProperties2(gpu(), &image_info, &image_properties);
            result != VK_SUCCESS) {
            GTEST_SKIP() << "Unable to create image. VkResult = " << string_VkResult(result);
        }

        if ((external_image_properties.externalMemoryProperties.compatibleHandleTypes & external_info.handleTypes) !=
            external_info.handleTypes) {
            GTEST_SKIP() << "Unable to create image, VkExternalMemoryImageCreateInfo::handleTypes not supported";
        }
    }

    CreateImageTest(*this, &ci);
}
