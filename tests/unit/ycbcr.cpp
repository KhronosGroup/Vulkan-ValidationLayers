/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/cast_utils.h"
#include "generated/enum_flag_bits.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/descriptor_helper.h"
#include "../framework/pipeline_helper.h"
#include "utils/vk_layer_utils.h"

TEST_F(NegativeYcbcr, Sampler) {
    TEST_DESCRIPTION("Verify YCbCr sampler creation.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitBasicYcbcr());

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkSamplerYcbcrConversion ycbcr_conv = VK_NULL_HANDLE;
    VkSamplerYcbcrConversionCreateInfo sycci = vku::InitStructHelper();
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    sycci.forceExplicitReconstruction = VK_FALSE;
    sycci.chromaFilter = VK_FILTER_NEAREST;
    sycci.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    sycci.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;

    // test non external conversion with a VK_FORMAT_UNDEFINED
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-format-04061");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, nullptr, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // test for non unorm
    sycci.format = VK_FORMAT_R8G8B8A8_SNORM;
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-format-04061");
    m_errorMonitor->SetUnexpectedError("VUID-VkSamplerYcbcrConversionCreateInfo-format-01650");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, nullptr, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // Force the multi-planar format support desired format features
    VkFormat mp_format = VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM;
    VkFormatProperties formatProps;
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), mp_format, &formatProps);
    formatProps.linearTilingFeatures = 0;
    formatProps.optimalTilingFeatures = 0;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), mp_format, formatProps);

    // Check that errors are caught when format feature don't exist
    sycci.format = mp_format;

    // No Chroma Sampler Bit set
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-format-01650");
    // 01651 set off twice for both xChromaOffset and yChromaOffset
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01651");
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01651");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, nullptr, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // Cosited feature supported, but midpoint samples set
    formatProps.linearTilingFeatures = 0;
    formatProps.optimalTilingFeatures = VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), mp_format, formatProps);
    sycci.xChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
    sycci.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01652");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, nullptr, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // Moving support to Linear to test that it checks either linear or optimal
    formatProps.linearTilingFeatures = VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT;
    formatProps.optimalTilingFeatures = 0;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), mp_format, formatProps);
    sycci.xChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
    sycci.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01652");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, nullptr, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // Using forceExplicitReconstruction without feature bit
    sycci.forceExplicitReconstruction = VK_TRUE;
    sycci.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    sycci.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-forceExplicitReconstruction-01656");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, nullptr, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // Linear chroma filtering without feature bit
    sycci.forceExplicitReconstruction = VK_FALSE;
    sycci.chromaFilter = VK_FILTER_LINEAR;
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-chromaFilter-01657");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, nullptr, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // Add linear feature bit so can create valid SamplerYcbcrConversion
    formatProps.linearTilingFeatures = VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT;
    formatProps.optimalTilingFeatures = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), mp_format, formatProps);
    vkt::SamplerYcbcrConversion conversion(*m_device, sycci);

    // Try to create a Sampler with non-matching filters without feature bit set
    VkSamplerYcbcrConversionInfo sampler_ycbcr_info = vku::InitStructHelper();
    sampler_ycbcr_info.conversion = conversion.handle();
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.minFilter = VK_FILTER_NEAREST;  // Different than chromaFilter
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.pNext = (void *)&sampler_ycbcr_info;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-minFilter-01645");

    sampler_info.magFilter = VK_FILTER_NEAREST;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-minFilter-01645");
}

TEST_F(NegativeYcbcr, Swizzle) {
    TEST_DESCRIPTION("Verify Invalid use of siwizzle components when dealing with YCbCr.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitBasicYcbcr());

    const VkFormat mp_format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;

    // Make sure components doesn't affect _444 formats
    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(gpu(), mp_format, &format_props);
    if ((format_props.optimalTilingFeatures &
         (VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT | VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT)) == 0) {
        GTEST_SKIP() << "Device does not support chroma sampling of 3plane 420 format";
    }

    const VkComponentMapping identity = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                         VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};

    VkSamplerYcbcrConversion ycbcr_conv = VK_NULL_HANDLE;
    VkSamplerYcbcrConversionCreateInfo sycci = vku::InitStructHelper();
    sycci.format = mp_format;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    sycci.forceExplicitReconstruction = VK_FALSE;
    sycci.chromaFilter = VK_FILTER_NEAREST;
    sycci.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    sycci.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;

    // test components.g
    // This test is also serves as positive form of VU 01655 since SWIZZLE_A is considered only valid with this format because
    // ycbcrModel RGB_IDENTITY
    sycci.components = identity;
    sycci.components.g = VK_COMPONENT_SWIZZLE_A;
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-components-02581");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // test components.a
    sycci.components = identity;
    sycci.components.a = VK_COMPONENT_SWIZZLE_R;
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-components-02582");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // make sure zero and one are allowed for components.a
    {
        sycci.components.a = VK_COMPONENT_SWIZZLE_ONE;
        vkt::SamplerYcbcrConversion conversion(*m_device, sycci);
    }
    {
        sycci.components.a = VK_COMPONENT_SWIZZLE_ZERO;
        vkt::SamplerYcbcrConversion conversion(*m_device, sycci);
    }

    // test components.r
    sycci.components = identity;
    sycci.components.r = VK_COMPONENT_SWIZZLE_G;
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-components-02583");
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-components-02585");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // test components.b
    sycci.components = identity;
    sycci.components.b = VK_COMPONENT_SWIZZLE_G;
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-components-02584");
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-components-02585");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // test components.r and components.b together
    sycci.components = identity;
    sycci.components.r = VK_COMPONENT_SWIZZLE_B;
    sycci.components.b = VK_COMPONENT_SWIZZLE_B;
    m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-components-02585");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // make sure components.r and components.b can be swapped
    {
        sycci.components = identity;
        sycci.components.r = VK_COMPONENT_SWIZZLE_B;
        sycci.components.b = VK_COMPONENT_SWIZZLE_R;
        vkt::SamplerYcbcrConversion conversion(*m_device, sycci);
    }

    // Non RGB Identity model
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_IDENTITY;
    {
        // Non RGB Identity can't have a explicit zero swizzle
        sycci.components = identity;
        sycci.components.g = VK_COMPONENT_SWIZZLE_ZERO;
        m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-ycbcrModel-01655");
        m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-components-02581");
        vk::CreateSamplerYcbcrConversion(device(), &sycci, nullptr, &ycbcr_conv);
        m_errorMonitor->VerifyFound();

        // Non RGB Identity can't have a explicit one swizzle
        sycci.components = identity;
        sycci.components.g = VK_COMPONENT_SWIZZLE_ONE;
        m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-ycbcrModel-01655");
        m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-components-02581");
        vk::CreateSamplerYcbcrConversion(device(), &sycci, nullptr, &ycbcr_conv);
        m_errorMonitor->VerifyFound();

        // VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM has 3 component so RGBA conversion has implicit A as one
        sycci.components = identity;
        sycci.components.g = VK_COMPONENT_SWIZZLE_A;
        m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-ycbcrModel-01655");
        m_errorMonitor->SetDesiredError("VUID-VkSamplerYcbcrConversionCreateInfo-components-02581");
        vk::CreateSamplerYcbcrConversion(device(), &sycci, nullptr, &ycbcr_conv);
        m_errorMonitor->VerifyFound();
    }
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;  // reset

    // Make sure components doesn't affect _444 formats
    vk::GetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM, &format_props);
    if ((format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT) != 0) {
        sycci.format = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM;
        sycci.components = identity;
        sycci.components.g = VK_COMPONENT_SWIZZLE_R;
        vkt::SamplerYcbcrConversion conversion(*m_device, sycci);
    }

    // Create a valid conversion with guaranteed support
    sycci.format = mp_format;
    sycci.components = identity;
    vkt::SamplerYcbcrConversion conversion(*m_device, sycci);

    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

    VkSamplerYcbcrConversionInfo conversion_info = vku::InitStructHelper();
    conversion_info.conversion = conversion.handle();

    VkImageViewCreateInfo image_view_create_info = vku::InitStructHelper(&conversion_info);
    image_view_create_info.image = image.handle();
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_create_info.components = identity;
    image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_B;
    CreateImageViewTest(*this, &image_view_create_info, "VUID-VkImageViewCreateInfo-pNext-01970");
}

TEST_F(NegativeYcbcr, Formats) {
    TEST_DESCRIPTION("Creating images with Ycbcr Formats.");
    RETURN_IF_SKIP(InitBasicYcbcr());
    InitRenderTarget();

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    if (!FormatIsSupported(gpu(), VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM)) {
        GTEST_SKIP() << "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM is unsupported";
    }

    // Set format features as needed for tests
    VkFormatProperties formatProps;
    const VkFormat mp_format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), mp_format, &formatProps);
    formatProps.optimalTilingFeatures |= VK_FORMAT_FEATURE_TRANSFER_SRC_BIT;
    formatProps.optimalTilingFeatures = formatProps.optimalTilingFeatures & ~VK_FORMAT_FEATURE_DISJOINT_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), mp_format, formatProps);

    // Create ycbcr image with all valid values
    // Each test changes needed values and returns them back after
    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = mp_format;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.arrayLayers = 1;
    VkImageCreateInfo reset_create_info = image_create_info;

    VkImageFormatProperties img_limits;
    ASSERT_EQ(VK_SUCCESS, GPDIFPHelper(gpu(), &image_create_info, &img_limits));

    // invalid mipLevels
    if (img_limits.maxMipLevels == 1) {
        printf("Multiplane image maxMipLevels is already 1.  Skipping test.\n");
    } else {
        // needs to be 2
        // if more then 2 the VU since its larger the (depth^2 + 1)
        // if up the depth the VU for IMAGE_TYPE_2D and depth != 1 hits
        image_create_info.mipLevels = 2;
        CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-06410");
        image_create_info = reset_create_info;
    }

    // invalid samples count
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    // Might need to add extra validation because implementation probably doesn't support YUV
    VkImageFormatProperties image_format_props;
    vk::GetPhysicalDeviceImageFormatProperties(gpu(), mp_format, image_create_info.imageType, image_create_info.tiling,
                                               image_create_info.usage, image_create_info.flags, &image_format_props);
    if ((image_format_props.sampleCounts & VK_SAMPLE_COUNT_4_BIT) == 0) {
        m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-samples-02258");
    }
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-06411");
    image_create_info = reset_create_info;

    // invalid width
    image_create_info.extent.width = 31;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-04712");
    image_create_info = reset_create_info;

    // invalid height (since 420 format)
    image_create_info.extent.height = 31;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-04713");
    image_create_info = reset_create_info;

    // invalid imageType
    image_create_info.imageType = VK_IMAGE_TYPE_1D;
    // Check that image format is valid
    if (vk::GetPhysicalDeviceImageFormatProperties(gpu(), mp_format, image_create_info.imageType, image_create_info.tiling,
                                                   image_create_info.usage, image_create_info.flags,
                                                   &image_format_props) == VK_SUCCESS) {
        // Can't just set height to 1 as stateless validation will hit 04713 first
        m_errorMonitor->SetUnexpectedError("VUID-VkImageCreateInfo-imageType-00956");
        m_errorMonitor->SetUnexpectedError("VUID-VkImageCreateInfo-extent-02253");
        CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-06412");
    }
    image_create_info = reset_create_info;

    // Test using a format that doesn't support disjoint
    image_create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-imageCreateFormatFeatures-02260");
    image_create_info = reset_create_info;
}

TEST_F(NegativeYcbcr, ImageViewFormat) {
    TEST_DESCRIPTION("Creating image view with invalid formats, but use image format list to get a valid VkImage.");
    RETURN_IF_SKIP(InitBasicYcbcr());
    InitRenderTarget();

    if (!FormatIsSupported(gpu(), VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM)) {
        GTEST_SKIP() << "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM is unsupported";
    }
    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    image_create_info.extent.width = 31;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.arrayLayers = 1;

    // The only way around this is to use VK_KHR_image_format_list, but currently could not find anyone who supports it for YCbCr
    // formats, but could in the future
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-format-04712");
    vkt::Image image(*m_device, image_create_info);
    if (!image.initialized()) {
        GTEST_SKIP() << "image couldn't be created";
    }

    vkt::SamplerYcbcrConversion conversion(*m_device, VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM);
    auto conversion_info = conversion.ConversionInfo();

    auto ivci = vku::InitStruct<VkImageViewCreateInfo>(&conversion_info);
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    m_errorMonitor->SetDesiredError("VUID-VkImageViewCreateInfo-format-04714");
    vkt::ImageView view(*m_device, ivci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeYcbcr, CopyImageSinglePlane422Alignment) {
    // Image copy tests on single-plane _422 formats with block alignment errors
    RETURN_IF_SKIP(InitBasicYcbcr());

    // Select a _422 format and verify support
    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = VK_FORMAT_G8B8G8R8_422_UNORM_KHR;
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
    bool supported = ImageFormatIsSupported(instance(), gpu(), ci, features);
    if (!supported) {
        // Assume there's low ROI on searching for different mp formats
        GTEST_SKIP() << "Single-plane _422 image format not supported";
    }

    // Create images
    ci.extent = {64, 64, 1};
    vkt::Image image_422(*m_device, ci, vkt::set_layout);

    ci.extent = {64, 64, 1};
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    vkt::Image image_ucmp(*m_device, ci, vkt::set_layout);

    m_commandBuffer->begin();

    VkImageCopy copy_region;
    copy_region.extent = {48, 48, 1};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource = copy_region.srcSubresource;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    // Src offsets must be multiples of compressed block sizes
    copy_region.srcOffset = {3, 4, 0};  // source offset x
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-pRegions-07278");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcOffset-01783");
    vk::CmdCopyImage(m_commandBuffer->handle(), image_422.handle(), VK_IMAGE_LAYOUT_GENERAL, image_ucmp.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.srcOffset = {0, 0, 0};

    // Dst offsets must be multiples of compressed block sizes
    copy_region.dstOffset = {1, 0, 0};
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-pRegions-07281");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstOffset-01784");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstOffset-00150");
    vk::CmdCopyImage(m_commandBuffer->handle(), image_ucmp.handle(), VK_IMAGE_LAYOUT_GENERAL, image_422.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();
    copy_region.dstOffset = {0, 0, 0};

    // Copy extent must be multiples of compressed block sizes if not full width/height
    copy_region.extent = {31, 60, 1};  // 422 source, extent.x
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01728");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcOffset-01783");
    vk::CmdCopyImage(m_commandBuffer->handle(), image_422.handle(), VK_IMAGE_LAYOUT_GENERAL, image_ucmp.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    // 422 dest
    vk::CmdCopyImage(m_commandBuffer->handle(), image_ucmp.handle(), VK_IMAGE_LAYOUT_GENERAL, image_422.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    copy_region.dstOffset = {0, 0, 0};

    m_commandBuffer->end();
}

TEST_F(NegativeYcbcr, CopyImageMultiplaneAspectBits) {
    // Image copy tests on multiplane images with aspect errors
    RETURN_IF_SKIP(InitBasicYcbcr());

    // Select multi-plane formats and verify support
    VkFormat mp3_format = VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR;
    VkFormat mp2_format = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR;

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = mp2_format;
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
    bool supported = ImageFormatIsSupported(instance(), gpu(), ci, features);
    ci.format = VK_FORMAT_D24_UNORM_S8_UINT;
    supported = supported && ImageFormatIsSupported(instance(), gpu(), ci, features);
    ci.format = mp3_format;
    supported = supported && ImageFormatIsSupported(instance(), gpu(), ci, features);
    if (!supported) {
        // Assume there's low ROI on searching for different mp formats
        GTEST_SKIP() << "Multiplane image formats or optimally tiled depth-stencil buffers not supported";
    }

    // Create images
    vkt::Image mp3_image(*m_device, ci, vkt::set_layout);

    ci.format = mp2_format;
    vkt::Image mp2_image(*m_device, ci, vkt::set_layout);

    ci.format = VK_FORMAT_D24_UNORM_S8_UINT;
    vkt::Image sp_image(*m_device, ci, vkt::set_layout);

    m_commandBuffer->begin();

    VkImageCopy copy_region;
    copy_region.extent = {128, 128, 1};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT_KHR;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT_KHR;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.dstSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.dstSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource.layerCount = 1;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-08713");
    vk::CmdCopyImage(m_commandBuffer->handle(), mp2_image.handle(), VK_IMAGE_LAYOUT_GENERAL, mp3_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT_KHR;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-08713");
    vk::CmdCopyImage(m_commandBuffer->handle(), mp3_image.handle(), VK_IMAGE_LAYOUT_GENERAL, mp2_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT_KHR;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT_KHR;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstImage-08714");
    vk::CmdCopyImage(m_commandBuffer->handle(), mp3_image.handle(), VK_IMAGE_LAYOUT_GENERAL, mp2_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstImage-08714");
    vk::CmdCopyImage(m_commandBuffer->handle(), mp2_image.handle(), VK_IMAGE_LAYOUT_GENERAL, mp3_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-srcImage-01556");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-None-01549");  // since also non-compatiable
    vk::CmdCopyImage(m_commandBuffer->handle(), mp2_image.handle(), VK_IMAGE_LAYOUT_GENERAL, sp_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT_KHR;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-dstImage-01557");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImage-None-01549");  // since also non-compatiable
    vk::CmdCopyImage(m_commandBuffer->handle(), sp_image.handle(), VK_IMAGE_LAYOUT_GENERAL, mp3_image.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &copy_region);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeYcbcr, SamplerYcbcrConversionEnable) {
    TEST_DESCRIPTION("Checks samplerYcbcrConversion is enabled before calling vkCreateSamplerYcbcrConversion");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());
    // Explictly not enable Ycbcr Conversion Features
    VkPhysicalDeviceSamplerYcbcrConversionFeatures ycbcr_features = vku::InitStructHelper();
    ycbcr_features.samplerYcbcrConversion = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &ycbcr_features));

    // Create Ycbcr conversion
    VkSamplerYcbcrConversion conversions;
    VkSamplerYcbcrConversionCreateInfo ycbcr_create_info = {VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO,
                                                            NULL,
                                                            VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR,
                                                            VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY,
                                                            VK_SAMPLER_YCBCR_RANGE_ITU_FULL,
                                                            {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                                             VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
                                                            VK_CHROMA_LOCATION_COSITED_EVEN,
                                                            VK_CHROMA_LOCATION_COSITED_EVEN,
                                                            VK_FILTER_NEAREST,
                                                            false};

    m_errorMonitor->SetDesiredError("VUID-vkCreateSamplerYcbcrConversion-None-01648");
    vk::CreateSamplerYcbcrConversionKHR(m_device->handle(), &ycbcr_create_info, nullptr, &conversions);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeYcbcr, ClearColorImageFormat) {
    TEST_DESCRIPTION("Record clear color with an invalid image formats");
    RETURN_IF_SKIP(InitBasicYcbcr());
    InitRenderTarget();

    VkFormat mp_format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = mp_format;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_create_info.arrayLayers = 1;

    bool supported = ImageFormatIsSupported(instance(), gpu(), image_create_info, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT);
    if (supported == false) {
        GTEST_SKIP() << "Multiplane image format not supported";
    }

    vkt::Image mp_image(*m_device, image_create_info, vkt::set_layout);
    m_commandBuffer->begin();

    VkClearColorValue color_clear_value = {};
    VkImageSubresourceRange clear_range;
    clear_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clear_range.baseMipLevel = 0;
    clear_range.baseArrayLayer = 0;
    clear_range.layerCount = 1;
    clear_range.levelCount = 1;

    m_errorMonitor->SetDesiredError("VUID-vkCmdClearColorImage-image-01545");
    vk::CmdClearColorImage(m_commandBuffer->handle(), mp_image.handle(), VK_IMAGE_LAYOUT_GENERAL, &color_clear_value, 1,
                           &clear_range);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeYcbcr, WriteDescriptorSet) {
    TEST_DESCRIPTION("Attempt to use VkSamplerYcbcrConversion ImageView to update descriptors that are not allowed.");
    RETURN_IF_SKIP(InitBasicYcbcr());

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    // Create Ycbcr conversion
    VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;  // guaranteed sampling support
    auto ycbcr_create_info = vku::InitStruct<VkSamplerYcbcrConversionCreateInfo>(
        nullptr, mp_format, VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY, VK_SAMPLER_YCBCR_RANGE_ITU_FULL,
        VkComponentMapping{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                           VK_COMPONENT_SWIZZLE_IDENTITY},
        VK_CHROMA_LOCATION_COSITED_EVEN, VK_CHROMA_LOCATION_COSITED_EVEN, VK_FILTER_NEAREST, VK_FALSE);
    vkt::SamplerYcbcrConversion conversion(*m_device, ycbcr_create_info);

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    auto image_ci = vku::InitStruct<VkImageCreateInfo>(
        nullptr, VkImageCreateFlags{VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT},  // need for multi-planar
        VK_IMAGE_TYPE_2D, mp_format, VkExtent3D{64, 64u, 1u}, 1u, 1u, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
        VkImageUsageFlags{VK_IMAGE_USAGE_SAMPLED_BIT}, VK_SHARING_MODE_EXCLUSIVE, 0u, nullptr, VK_IMAGE_LAYOUT_UNDEFINED);
    vkt::Image image_obj(*m_device, image_ci, vkt::set_layout);

    VkSamplerYcbcrConversionInfo ycbcr_info = vku::InitStructHelper();
    ycbcr_info.conversion = conversion.handle();

    VkImageViewCreateInfo image_view_create_info = vku::InitStructHelper(&ycbcr_info);
    image_view_create_info.image = image_obj.handle();
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = mp_format;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkt::ImageView image_view(*m_device, image_view_create_info);

    descriptor_set.WriteDescriptorImageInfo(0, image_view.handle(), VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

    m_errorMonitor->SetDesiredError("VUID-VkWriteDescriptorSet-descriptorType-01946");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeYcbcr, MultiplaneImageLayoutAspectFlags) {
    TEST_DESCRIPTION("Query layout of a multiplane image using illegal aspect flag masks");
    RETURN_IF_SKIP(InitBasicYcbcr());

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR;
    ci.extent = {128, 128, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_LINEAR;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Verify formats
    bool supported = ImageFormatIsSupported(instance(), gpu(), ci, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT);
    ci.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR;
    supported = supported && ImageFormatIsSupported(instance(), gpu(), ci, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT);
    if (!supported) {
        // Assume there's low ROI on searching for different mp formats
        GTEST_SKIP() << "Multiplane image format not supported";
    }

    VkImage image_2plane, image_3plane;
    ci.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR;
    VkResult err = vk::CreateImage(device(), &ci, NULL, &image_2plane);
    ASSERT_EQ(VK_SUCCESS, err);

    ci.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR;
    err = vk::CreateImage(device(), &ci, NULL, &image_3plane);
    ASSERT_EQ(VK_SUCCESS, err);

    // Query layout of 3rd plane, for a 2-plane image
    VkImageSubresource subres = {};
    subres.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT_KHR;
    subres.mipLevel = 0;
    subres.arrayLayer = 0;
    VkSubresourceLayout layout = {};

    m_errorMonitor->SetDesiredError("VUID-vkGetImageSubresourceLayout-tiling-08717");
    vk::GetImageSubresourceLayout(device(), image_2plane, &subres, &layout);
    m_errorMonitor->VerifyFound();

    // Query layout using color aspect, for a 3-plane image
    subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredError("VUID-vkGetImageSubresourceLayout-tiling-08717");
    vk::GetImageSubresourceLayout(device(), image_3plane, &subres, &layout);
    m_errorMonitor->VerifyFound();

    // Clean up
    vk::DestroyImage(device(), image_2plane, NULL);
    vk::DestroyImage(device(), image_3plane, NULL);
}

TEST_F(NegativeYcbcr, BindMemory) {
    RETURN_IF_SKIP(InitBasicYcbcr());

    // Try to bind an image created with Disjoint bit
    VkFormatProperties format_properties;
    VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), mp_format, &format_properties);
    // Need to make sure disjoint is supported for format
    // Also need to support an arbitrary image usage feature
    constexpr VkFormatFeatureFlags disjoint_sampled = VK_FORMAT_FEATURE_DISJOINT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
    if (disjoint_sampled != (format_properties.optimalTilingFeatures & disjoint_sampled)) {
        printf("test requires disjoint and sampled feature bit on format.  Skipping.\n");
    } else {
        VkImageCreateInfo image_create_info = vku::InitStructHelper();
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = mp_format;
        image_create_info.extent.width = 64;
        image_create_info.extent.height = 64;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        image_create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;

        VkImage image;
        ASSERT_EQ(VK_SUCCESS, vk::CreateImage(device(), &image_create_info, NULL, &image));

        VkImagePlaneMemoryRequirementsInfo image_plane_req = vku::InitStructHelper();
        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;

        VkImageMemoryRequirementsInfo2 mem_req_info2 = vku::InitStructHelper(&image_plane_req);
        mem_req_info2.image = image;
        VkMemoryRequirements2 mem_req2 = vku::InitStructHelper();
        vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mem_req2);

        // Find a valid memory type index to memory to be allocated from
        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
        alloc_info.allocationSize = mem_req2.memoryRequirements.size;
        ASSERT_TRUE(m_device->phy().set_memory_type(mem_req2.memoryRequirements.memoryTypeBits, &alloc_info, 0));

        VkDeviceMemory image_memory;
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &alloc_info, NULL, &image_memory));

        // Bind disjoint with BindImageMemory instead of BindImageMemory2
        m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-image-01608");
        m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-image-07736");
        vk::BindImageMemory(device(), image, image_memory, 0);
        m_errorMonitor->VerifyFound();

        VkBindImagePlaneMemoryInfo plane_memory_info = vku::InitStructHelper();
        ASSERT_TRUE(vkuFormatPlaneCount(mp_format) == 2);
        plane_memory_info.planeAspect = VK_IMAGE_ASPECT_PLANE_2_BIT;

        VkBindImageMemoryInfo bind_image_info = vku::InitStructHelper(&plane_memory_info);
        bind_image_info.image = image;
        bind_image_info.memory = image_memory;
        bind_image_info.memoryOffset = 0;

        // Set invalid planeAspect
        m_errorMonitor->SetDesiredError("VUID-VkBindImagePlaneMemoryInfo-planeAspect-02283");
        // Error is thrown from not having both planes bound
        m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory2-pBindInfos-02858");
        m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory2-pBindInfos-02858");
        // Might happen as plane2 wasn't queried for its memroy type
        m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-pNext-01619");
        m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-pNext-01621");
        vk::BindImageMemory2KHR(device(), 1, &bind_image_info);
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), image_memory, NULL);
        vk::DestroyImage(device(), image, nullptr);
    }

    // Bind image with VkBindImagePlaneMemoryInfo without disjoint bit in image
    // Need to support an arbitrary image usage feature for multi-planar format
    if (0 == (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        printf("test requires sampled feature bit on multi-planar format.  Skipping.\n");
    } else {
        VkImageCreateInfo image_create_info = vku::InitStructHelper();
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = mp_format;
        image_create_info.extent.width = 64;
        image_create_info.extent.height = 64;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        image_create_info.flags = 0;  // no disjoint bit set

        VkImage image;
        ASSERT_EQ(VK_SUCCESS, vk::CreateImage(device(), &image_create_info, NULL, &image));

        VkImageMemoryRequirementsInfo2 mem_req_info2 = vku::InitStructHelper();
        mem_req_info2.image = image;
        VkMemoryRequirements2 mem_req2 = vku::InitStructHelper();
        vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mem_req2);

        // Find a valid memory type index to memory to be allocated from
        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
        alloc_info.allocationSize = mem_req2.memoryRequirements.size;
        ASSERT_TRUE(m_device->phy().set_memory_type(mem_req2.memoryRequirements.memoryTypeBits, &alloc_info, 0));

        VkDeviceMemory image_memory;
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &alloc_info, NULL, &image_memory));

        VkBindImagePlaneMemoryInfo plane_memory_info = vku::InitStructHelper();
        plane_memory_info.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
        VkBindImageMemoryInfo bind_image_info = vku::InitStructHelper(&plane_memory_info);
        bind_image_info.image = image;
        bind_image_info.memory = image_memory;
        bind_image_info.memoryOffset = 0;

        m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-pNext-01618");
        vk::BindImageMemory2KHR(device(), 1, &bind_image_info);
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), image_memory, NULL);
        vk::DestroyImage(device(), image, nullptr);
    }
}

TEST_F(NegativeYcbcr, BindMemory2Disjoint) {
    TEST_DESCRIPTION("These tests deal with VK_KHR_bind_memory_2 and disjoint memory being bound");

    // Enable KHR YCbCr req'd extensions for Disjoint Bit
    AddRequiredExtensions(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const bool mp_supported = IsExtensionsEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    const VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    const VkFormat tex_format = VK_FORMAT_R8G8B8A8_UNORM;

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = tex_format;
    image_create_info.extent.width = 256;
    image_create_info.extent.height = 256;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = 0;

    // Only gets used in MP tests
    VkImageCreateInfo mp_image_create_info = image_create_info;
    mp_image_create_info.format = mp_format;
    mp_image_create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;

    // Check for support of format used by all multi-planar tests
    // Need seperate boolean as its valid to do tests that support YCbCr but not disjoint
    bool mp_disjoint_support = false;
    if (mp_supported) {
        VkFormatProperties mp_format_properties;
        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), mp_format, &mp_format_properties);
        if ((mp_format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT) &&
            (mp_format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
            mp_disjoint_support = true;
        }
    }

    // Try to bind memory to an object with an invalid memoryOffset
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    VkMemoryRequirements image_mem_reqs = {};
    vk::GetImageMemoryRequirements(device(), image.handle(), &image_mem_reqs);
    VkMemoryAllocateInfo image_alloc_info = vku::InitStructHelper();
    // Leave some extra space for alignment wiggle room
    image_alloc_info.allocationSize = image_mem_reqs.size + image_mem_reqs.alignment;
    m_device->phy().set_memory_type(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0);
    vkt::DeviceMemory image_mem(*m_device, image_alloc_info);

    // Keep values outside scope so multiple tests cases can reuse
    vkt::Image mp_image;
    vkt::DeviceMemory mp_image_mem[2];
    VkMemoryRequirements2 mp_image_mem_reqs2[2];
    VkMemoryAllocateInfo mp_image_alloc_info[2];
    if (mp_disjoint_support) {
        mp_image.init_no_mem(*m_device, mp_image_create_info);

        VkImagePlaneMemoryRequirementsInfo image_plane_req = vku::InitStructHelper();
        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;

        VkImageMemoryRequirementsInfo2 mem_req_info2 = vku::InitStructHelper(&image_plane_req);
        mem_req_info2.image = mp_image.handle();
        mp_image_mem_reqs2[0] = vku::InitStructHelper();
        vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mp_image_mem_reqs2[0]);

        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;
        mp_image_mem_reqs2[1] = vku::InitStructHelper();
        vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mp_image_mem_reqs2[1]);

        mp_image_alloc_info[0] = vku::InitStructHelper();
        mp_image_alloc_info[1] = vku::InitStructHelper();
        // Leave some extra space for alignment wiggle room
        // plane 0
        mp_image_alloc_info[0].allocationSize =
            mp_image_mem_reqs2[0].memoryRequirements.size + mp_image_mem_reqs2[0].memoryRequirements.alignment;
        m_device->phy().set_memory_type(mp_image_mem_reqs2[0].memoryRequirements.memoryTypeBits, &mp_image_alloc_info[0], 0);
        // Exact size as VU will always be for plane 1
        // plane 1
        mp_image_alloc_info[1].allocationSize = mp_image_mem_reqs2[1].memoryRequirements.size;
        m_device->phy().set_memory_type(mp_image_mem_reqs2[1].memoryRequirements.memoryTypeBits, &mp_image_alloc_info[1], 0);

        mp_image_mem[0].init(*m_device, mp_image_alloc_info[0]);
        mp_image_mem[1].init(*m_device, mp_image_alloc_info[1]);
    }

    // All planes must be bound at once the same here
    VkBindImagePlaneMemoryInfo plane_memory_info[2];
    plane_memory_info[0] = vku::InitStructHelper();
    plane_memory_info[0].planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
    plane_memory_info[1] = vku::InitStructHelper();
    plane_memory_info[1].planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;

    // Test unaligned memory offset

    // single-plane image
    VkBindImageMemoryInfo bind_image_info = vku::InitStructHelper();
    bind_image_info.image = image.handle();
    bind_image_info.memory = image_mem.handle();
    bind_image_info.memoryOffset = 1;  // off alignment

    if (mp_disjoint_support == true) {
        VkImageMemoryRequirementsInfo2 mem_req_info2 = vku::InitStructHelper();
        mem_req_info2.image = image.handle();
        VkMemoryRequirements2 mem_req2 = vku::InitStructHelper();
        vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mem_req2);

        if (mem_req2.memoryRequirements.alignment > 1) {
            m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-pNext-01616");
            vk::BindImageMemory2KHR(device(), 1, &bind_image_info);
            m_errorMonitor->VerifyFound();
        }
    } else {
        // Same as 01048 but with bindImageMemory2 call
        if (image_mem_reqs.alignment > 1) {
            m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-pNext-01616");
            vk::BindImageMemory2KHR(device(), 1, &bind_image_info);
            m_errorMonitor->VerifyFound();
        }
    }

    // Multi-plane image
    if (mp_disjoint_support == true) {
        if (mp_image_mem_reqs2[0].memoryRequirements.alignment > 1) {
            VkBindImageMemoryInfo bind_image_infos[2];
            bind_image_infos[0] = vku::InitStructHelper(&plane_memory_info[0]);
            bind_image_infos[0].image = mp_image.handle();
            bind_image_infos[0].memory = mp_image_mem[0].handle();
            bind_image_infos[0].memoryOffset = 1;  // off alignment
            bind_image_infos[1] = vku::InitStructHelper(&plane_memory_info[1]);
            bind_image_infos[1].image = mp_image.handle();
            bind_image_infos[1].memory = mp_image_mem[1].handle();
            bind_image_infos[1].memoryOffset = 0;

            m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-pNext-01620");
            vk::BindImageMemory2KHR(device(), 2, bind_image_infos);
            m_errorMonitor->VerifyFound();
        }
    }

    // Test memory offsets within the memory allocation, but which leave too little memory for
    // the resource.
    // single-plane image
    bind_image_info = vku::InitStructHelper();
    bind_image_info.image = image.handle();
    bind_image_info.memory = image_mem.handle();

    if (mp_disjoint_support == true) {
        VkImageMemoryRequirementsInfo2 mem_req_info2 = vku::InitStructHelper();
        mem_req_info2.image = image.handle();
        VkMemoryRequirements2 mem_req2 = vku::InitStructHelper();
        vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mem_req2);

        VkDeviceSize image2_offset = (mem_req2.memoryRequirements.size - 1) & ~(mem_req2.memoryRequirements.alignment - 1);
        if ((image2_offset > 0) &&
            (mem_req2.memoryRequirements.size < (image_alloc_info.allocationSize - mem_req2.memoryRequirements.alignment))) {
            bind_image_info.memoryOffset = image2_offset;
            m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-pNext-01617");
            vk::BindImageMemory2KHR(device(), 1, &bind_image_info);
            m_errorMonitor->VerifyFound();
        }
    } else {
        // Same as 01049 but with bindImageMemory2 call
        VkDeviceSize image_offset = (image_mem_reqs.size - 1) & ~(image_mem_reqs.alignment - 1);
        if ((image_offset > 0) && (image_mem_reqs.size < (image_alloc_info.allocationSize - image_mem_reqs.alignment))) {
            bind_image_info.memoryOffset = image_offset;
            m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-pNext-01617");
            vk::BindImageMemory2KHR(device(), 1, &bind_image_info);
            m_errorMonitor->VerifyFound();
        }
    }

    // Multi-plane image
    if (mp_disjoint_support == true) {
        VkDeviceSize mp_image_offset =
            (mp_image_mem_reqs2[0].memoryRequirements.size - 1) & ~(mp_image_mem_reqs2[0].memoryRequirements.alignment - 1);
        if ((mp_image_offset > 0) &&
            (mp_image_mem_reqs2[0].memoryRequirements.size <
             (mp_image_alloc_info[0].allocationSize - mp_image_mem_reqs2[0].memoryRequirements.alignment))) {
            VkBindImageMemoryInfo bind_image_infos[2];
            bind_image_infos[0] = vku::InitStructHelper(&plane_memory_info[0]);
            bind_image_infos[0].image = mp_image.handle();
            bind_image_infos[0].memory = mp_image_mem[0].handle();
            bind_image_infos[0].memoryOffset = mp_image_offset;  // mis-offset
            bind_image_infos[1] = vku::InitStructHelper(&plane_memory_info[1]);
            bind_image_infos[1].image = mp_image.handle();
            bind_image_infos[1].memory = mp_image_mem[1].handle();
            bind_image_infos[1].memoryOffset = 0;

            m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-pNext-01621");
            vk::BindImageMemory2KHR(device(), 2, bind_image_infos);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(NegativeYcbcr, BindMemory2DisjointUnsupported) {
    TEST_DESCRIPTION("These tests deal with VK_KHR_bind_memory_2 and disjoint memory being bound");

    // Enable KHR YCbCr req'd extensions for Disjoint Bit
    AddRequiredExtensions(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const bool mp_supported = IsExtensionsEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    const VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    const VkFormat tex_format = VK_FORMAT_R8G8B8A8_UNORM;

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = tex_format;
    image_create_info.extent.width = 256;
    image_create_info.extent.height = 256;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = 0;

    // Only gets used in MP tests
    VkImageCreateInfo mp_image_create_info = image_create_info;
    mp_image_create_info.format = mp_format;
    mp_image_create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;

    // Check for support of format used by all multi-planar tests
    // Need seperate boolean as its valid to do tests that support YCbCr but not disjoint
    bool mp_disjoint_support = false;
    if (mp_supported) {
        VkFormatProperties mp_format_properties;
        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), mp_format, &mp_format_properties);
        if ((mp_format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT) &&
            (mp_format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
            mp_disjoint_support = true;
        }
    }

    // Try to bind memory to an object with an invalid memoryOffset
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    VkMemoryRequirements image_mem_reqs = {};
    vk::GetImageMemoryRequirements(device(), image.handle(), &image_mem_reqs);
    VkMemoryAllocateInfo image_alloc_info = vku::InitStructHelper();
    // Leave some extra space for alignment wiggle room
    image_alloc_info.allocationSize = image_mem_reqs.size + image_mem_reqs.alignment;
    m_device->phy().set_memory_type(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0);
    vkt::DeviceMemory image_mem(*m_device, image_alloc_info);

    // Keep values outside scope so multiple tests cases can reuse
    vkt::Image mp_image;
    vkt::DeviceMemory mp_image_mem[2];
    VkMemoryRequirements2 mp_image_mem_reqs2[2];
    VkMemoryAllocateInfo mp_image_alloc_info[2];
    if (mp_disjoint_support) {
        mp_image.init_no_mem(*m_device, mp_image_create_info);

        VkImagePlaneMemoryRequirementsInfo image_plane_req = vku::InitStructHelper();
        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;

        VkImageMemoryRequirementsInfo2 mem_req_info2 = vku::InitStructHelper(&image_plane_req);
        mem_req_info2.image = mp_image.handle();
        mp_image_mem_reqs2[0] = vku::InitStructHelper();
        vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mp_image_mem_reqs2[0]);

        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;
        mp_image_mem_reqs2[1] = vku::InitStructHelper();
        vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mp_image_mem_reqs2[1]);

        mp_image_alloc_info[0] = vku::InitStructHelper();
        mp_image_alloc_info[1] = vku::InitStructHelper();
        // Leave some extra space for alignment wiggle room
        // plane 0
        mp_image_alloc_info[0].allocationSize =
            mp_image_mem_reqs2[0].memoryRequirements.size + mp_image_mem_reqs2[0].memoryRequirements.alignment;
        m_device->phy().set_memory_type(mp_image_mem_reqs2[0].memoryRequirements.memoryTypeBits, &mp_image_alloc_info[0], 0);
        // Exact size as VU will always be for plane 1
        // plane 1
        mp_image_alloc_info[1].allocationSize = mp_image_mem_reqs2[1].memoryRequirements.size;
        m_device->phy().set_memory_type(mp_image_mem_reqs2[1].memoryRequirements.memoryTypeBits, &mp_image_alloc_info[1], 0);

        mp_image_mem[0].init(*m_device, mp_image_alloc_info[0]);
        mp_image_mem[1].init(*m_device, mp_image_alloc_info[1]);
    }

    // All planes must be bound at once the same here
    VkBindImagePlaneMemoryInfo plane_memory_info[2];
    plane_memory_info[0] = vku::InitStructHelper();
    plane_memory_info[0].planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
    plane_memory_info[1] = vku::InitStructHelper();
    plane_memory_info[1].planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;

    // Try to bind memory to an object with an invalid memory type

    // Create a mask of available memory types *not* supported by these resources, and try to use one of them.
    VkPhysicalDeviceMemoryProperties memory_properties = {};
    vk::GetPhysicalDeviceMemoryProperties(m_device->phy().handle(), &memory_properties);

    // single-plane image
    VkBindImageMemoryInfo bind_image_info = vku::InitStructHelper();
    bind_image_info.image = image.handle();
    bind_image_info.memoryOffset = 0;

    if (mp_disjoint_support == true) {
        VkImageMemoryRequirementsInfo2 mem_req_info2 = vku::InitStructHelper();
        mem_req_info2.image = image.handle();
        VkMemoryRequirements2 mem_req2 = vku::InitStructHelper();
        vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mem_req2);

        uint32_t image2_unsupported_mem_type_bits =
            ((1 << memory_properties.memoryTypeCount) - 1) & ~mem_req2.memoryRequirements.memoryTypeBits;
        bool found_type =
            m_device->phy().set_memory_type(image2_unsupported_mem_type_bits, &image_alloc_info, 0,
                                            VK_MEMORY_PROPERTY_PROTECTED_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD);
        if (image2_unsupported_mem_type_bits != 0 && found_type) {
            vkt::DeviceMemory image_mem_tmp(*m_device, image_alloc_info);
            bind_image_info.memory = image_mem_tmp.handle();
            m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-pNext-01615");
            vk::BindImageMemory2KHR(device(), 1, &bind_image_info);
            m_errorMonitor->VerifyFound();
        }
    } else {
        // Same as 01047 but with bindImageMemory2 call
        uint32_t image_unsupported_mem_type_bits = ((1 << memory_properties.memoryTypeCount) - 1) & ~image_mem_reqs.memoryTypeBits;
        bool found_type =
            m_device->phy().set_memory_type(image_unsupported_mem_type_bits, &image_alloc_info, 0,
                                            VK_MEMORY_PROPERTY_PROTECTED_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD);
        if (image_unsupported_mem_type_bits != 0 && found_type) {
            vkt::DeviceMemory image_mem_tmp(*m_device, image_alloc_info);
            bind_image_info.memory = image_mem_tmp.handle();
            m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-pNext-01615");
            vk::BindImageMemory2KHR(device(), 1, &bind_image_info);
            m_errorMonitor->VerifyFound();
        }
    }

    // Multi-plane image
    if (mp_disjoint_support == true) {
        // Get plane 0 memory requirements
        VkImagePlaneMemoryRequirementsInfo image_plane_req = vku::InitStructHelper();
        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;

        VkImageMemoryRequirementsInfo2 mem_req_info2 = vku::InitStructHelper(&image_plane_req);
        mem_req_info2.image = mp_image.handle();
        vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mp_image_mem_reqs2[0]);

        uint32_t mp_image_unsupported_mem_type_bits =
            ((1 << memory_properties.memoryTypeCount) - 1) & ~mp_image_mem_reqs2[0].memoryRequirements.memoryTypeBits;
        bool found_type =
            m_device->phy().set_memory_type(mp_image_unsupported_mem_type_bits, &mp_image_alloc_info[0], 0,
                                            VK_MEMORY_PROPERTY_PROTECTED_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD);
        if (mp_image_unsupported_mem_type_bits != 0 && found_type) {
            mp_image_alloc_info[0].allocationSize = mp_image_mem_reqs2[0].memoryRequirements.size;
            vkt::DeviceMemory mp_image_mem_tmp(*m_device, mp_image_alloc_info[0]);

            VkBindImageMemoryInfo bind_image_infos[2];
            bind_image_infos[0] = vku::InitStructHelper(&plane_memory_info[0]);
            bind_image_infos[0].image = mp_image.handle();
            bind_image_infos[0].memory = mp_image_mem_tmp.handle();
            bind_image_infos[0].memoryOffset = 0;
            bind_image_infos[1] = vku::InitStructHelper(&plane_memory_info[1]);
            bind_image_infos[1].image = mp_image.handle();
            bind_image_infos[1].memory = mp_image_mem[1].handle();
            bind_image_infos[1].memoryOffset = 0;

            m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-pNext-01619");
            vk::BindImageMemory2KHR(device(), 2, bind_image_infos);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(NegativeYcbcr, MismatchedImageViewAndSamplerFormat) {
    TEST_DESCRIPTION("Create image view with a different format that SamplerYcbcr was created with.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitBasicYcbcr());

    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

    VkSamplerYcbcrConversionCreateInfo sampler_conversion_ci = vku::InitStructHelper();
    sampler_conversion_ci.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    sampler_conversion_ci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    sampler_conversion_ci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    sampler_conversion_ci.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                        VK_COMPONENT_SWIZZLE_IDENTITY};
    sampler_conversion_ci.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    sampler_conversion_ci.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    sampler_conversion_ci.chromaFilter = VK_FILTER_NEAREST;
    sampler_conversion_ci.forceExplicitReconstruction = false;

    vkt::SamplerYcbcrConversion sampler_conversion(*m_device, sampler_conversion_ci);

    VkSamplerYcbcrConversionInfo sampler_ycbcr_conversion_info = vku::InitStructHelper();
    sampler_ycbcr_conversion_info.conversion = sampler_conversion.handle();

    VkImageViewCreateInfo view_info = vku::InitStructHelper(&sampler_ycbcr_conversion_info);
    view_info.flags = 0;
    view_info.image = image.handle();
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    view_info.subresourceRange.layerCount = 1;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    CreateImageViewTest(*this, &view_info, "VUID-VkImageViewCreateInfo-pNext-06658");
}

TEST_F(NegativeYcbcr, MultiplaneIncompatibleViewFormat) {
    TEST_DESCRIPTION("Postive/negative tests of multiplane imageview format compatibility");

    // Use 1.1 to get VK_KHR_sampler_ycbcr_conversion easier
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitBasicYcbcr());

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    VkSamplerYcbcrConversionCreateInfo ycbcr_create_info = vku::InitStructHelper();
    ycbcr_create_info.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    ycbcr_create_info.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    ycbcr_create_info.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    ycbcr_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                    VK_COMPONENT_SWIZZLE_IDENTITY};
    ycbcr_create_info.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.chromaFilter = VK_FILTER_NEAREST;
    ycbcr_create_info.forceExplicitReconstruction = false;

    vkt::SamplerYcbcrConversion conversion(*m_device, ycbcr_create_info);

    VkSamplerYcbcrConversionInfo ycbcr_info = vku::InitStructHelper();
    ycbcr_info.conversion = conversion;

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    ci.extent = {128, 128, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    const VkFormatFeatureFlags features = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
    bool supported = ImageFormatIsSupported(instance(), gpu(), ci, features);
    // Verify format 3 Plane format
    if (!supported) {
        printf("Multiplane image format not supported.  Skipping test.\n");
    } else {
        vkt::Image image_obj(*m_device, ci, vkt::set_layout);

        VkImageViewCreateInfo ivci = vku::InitStructHelper();
        ivci.image = image_obj.handle();
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.format = VK_FORMAT_R8G8_UNORM;  // Compat is VK_FORMAT_R8_UNORM
        ivci.subresourceRange.layerCount = 1;
        ivci.subresourceRange.baseMipLevel = 0;
        ivci.subresourceRange.levelCount = 1;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;

        // Incompatible format error
        CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-01586");

        // Correct format succeeds
        ivci.format = VK_FORMAT_R8_UNORM;
        CreateImageViewTest(*this, &ivci);

        // R8_SNORM is compatible with R8_UNORM
        ivci.format = VK_FORMAT_R8_SNORM;
        CreateImageViewTest(*this, &ivci);

        // Try a multiplane imageview
        ivci.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-format-06415");

        // If using multiplane format, need a matching VkSamplerYcbcrConversion
        ivci.pNext = &ycbcr_info;
        CreateImageViewTest(*this, &ivci);
    }

    ci.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    supported = ImageFormatIsSupported(instance(), gpu(), ci, features);
    // Verify format 2 Plane format
    if (!supported) {
        printf("Multiplane image format not supported.  Skipping test.\n");
    } else {
        vkt::Image image_obj(*m_device, ci, vkt::set_layout);

        VkImageViewCreateInfo ivci = vku::InitStructHelper();
        ivci.image = image_obj.handle();
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.subresourceRange.layerCount = 1;
        ivci.subresourceRange.baseMipLevel = 0;
        ivci.subresourceRange.levelCount = 1;

        // Plane 0 is compatible with VK_FORMAT_R8_UNORM
        // Plane 1 is compatible with VK_FORMAT_R8G8_UNORM

        // Correct format succeeds
        ivci.format = VK_FORMAT_R8_UNORM;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
        CreateImageViewTest(*this, &ivci);

        ivci.format = VK_FORMAT_R8G8_UNORM;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
        CreateImageViewTest(*this, &ivci);

        // Incompatible format error
        ivci.format = VK_FORMAT_R8_UNORM;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
        CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-01586");

        ivci.format = VK_FORMAT_R8G8_UNORM;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
        CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-01586");
    }
}

TEST_F(NegativeYcbcr, MultiplaneImageViewAspectMasks) {
    TEST_DESCRIPTION("Create a VkImageView with multiple planar aspect masks");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitBasicYcbcr());

    if (IsExtensionsEnabled(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_portability_subset enabled, can hit issues with imageViewFormatReinterpretation";
    }

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    ci.extent = {128, 128, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;

    // Different formats between VkImage and VkImageView
    {
        ci.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
        vkt::Image image(*m_device, ci, vkt::set_layout);

        ivci.image = image.handle();
        ivci.format = VK_FORMAT_R8_UNORM;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT;

        CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-subresourceRange-07818");
    }

    // Without Mutable format
    {
        ci.flags = 0;
        vkt::Image image(*m_device, ci, vkt::set_layout);

        VkSamplerYcbcrConversionCreateInfo ycbcr_create_info = vku::InitStructHelper();
        ycbcr_create_info.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
        ycbcr_create_info.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
        ycbcr_create_info.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
        ycbcr_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                        VK_COMPONENT_SWIZZLE_IDENTITY};
        ycbcr_create_info.xChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
        ycbcr_create_info.yChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
        ycbcr_create_info.chromaFilter = VK_FILTER_NEAREST;
        ycbcr_create_info.forceExplicitReconstruction = false;

        vkt::SamplerYcbcrConversion conversion(*m_device, ycbcr_create_info);
        VkSamplerYcbcrConversionInfo ycbcr_info = vku::InitStructHelper();
        ycbcr_info.conversion = conversion;

        ivci.image = image.handle();
        ivci.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT;
        ivci.pNext = &ycbcr_info;
        CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-subresourceRange-07818");
    }
}

TEST_F(NegativeYcbcr, MultiplaneAspectBits) {
    TEST_DESCRIPTION("Attempt to update descriptor sets for images that do not have correct aspect bits sets.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitBasicYcbcr());

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;  // commonly supported multi-planar format
    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), mp_format, &format_props);
    if (!vkt::Image::IsCompatible(*m_device, VK_IMAGE_USAGE_SAMPLED_BIT, format_props.optimalTilingFeatures)) {
        GTEST_SKIP() << "multi-planar format cannot be sampled for optimalTiling.";
    }

    auto image_ci = vku::InitStruct<VkImageCreateInfo>(
        nullptr, VkImageCreateFlags{VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT},  // need for multi-planar
        VK_IMAGE_TYPE_2D, mp_format, VkExtent3D{64, 64, 1}, 1u, 1u, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
        VkImageUsageFlags{VK_IMAGE_USAGE_SAMPLED_BIT}, VK_SHARING_MODE_EXCLUSIVE, 0u, nullptr, VK_IMAGE_LAYOUT_UNDEFINED);
    vkt::Image image_obj(*m_device, image_ci, vkt::set_layout);

    VkSamplerYcbcrConversionCreateInfo ycbcr_create_info = vku::InitStructHelper();
    ycbcr_create_info.format = mp_format;
    ycbcr_create_info.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    ycbcr_create_info.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    ycbcr_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                    VK_COMPONENT_SWIZZLE_IDENTITY};
    ycbcr_create_info.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.chromaFilter = VK_FILTER_NEAREST;
    ycbcr_create_info.forceExplicitReconstruction = false;

    vkt::SamplerYcbcrConversion conversion(*m_device, ycbcr_create_info);

    VkSamplerYcbcrConversionInfo ycbcr_info = vku::InitStructHelper();
    ycbcr_info.conversion = conversion.handle();

    auto image_view_ci = image_obj.BasicViewCreatInfo();
    image_view_ci.pNext = &ycbcr_info;
    const auto image_view = vkt::ImageView(*m_device, image_view_ci);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.pNext = &ycbcr_info;
    vkt::Sampler sampler(*m_device, sampler_ci);
    ASSERT_TRUE(sampler.initialized());

    OneOffDescriptorSet descriptor_set(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, &sampler.handle()},
                  });

    if (descriptor_set.set_ == VK_NULL_HANDLE) {
        GTEST_SKIP() << "Couldn't create descriptor set";
    }

    // TODO - 01564 appears to be impossible to hit due to the following check in descriptor_validation.cpp:
    // if (sampler && !desc->IsImmutableSampler() && vkuFormatIsMultiplane(image_state->createInfo.format)) ...
    //   - !desc->IsImmutableSampler() will cause 02738; IOW, multi-plane conversion _requires_ an immutable sampler
    //   - !desc->IsImmutableSampler() must be removed for 01564 to get hit, but it's not clear whether or not this is
    //   correct based on the comments in the code
    // m_errorMonitor->SetDesiredError("VUID-VkDescriptorImageInfo-sampler-01564");
    descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();
    // m_errorMonitor->VerifyFound();
}

TEST_F(NegativeYcbcr, DisjointImageWithDrmFormatModifier) {
    TEST_DESCRIPTION("Create image with VK_IMAGE_CREATE_DISJOINT_BIT and VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicYcbcr());

    VkFormat format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;

    std::vector<uint64_t> mods;
    VkDrmFormatModifierPropertiesListEXT mod_props = vku::InitStructHelper();
    VkFormatProperties2 format_props = vku::InitStructHelper(&mod_props);
    vk::GetPhysicalDeviceFormatProperties2(gpu(), format, &format_props);
    if (mod_props.drmFormatModifierCount == 0) {
        GTEST_SKIP() << "drmFormatModifierCount is 0.";
    }

    std::vector<VkDrmFormatModifierPropertiesEXT> mod_props_length(mod_props.drmFormatModifierCount);
    mod_props.pDrmFormatModifierProperties = mod_props_length.data();
    vk::GetPhysicalDeviceFormatProperties2(gpu(), format, &format_props);

    for (uint32_t i = 0; i < mod_props.drmFormatModifierCount; ++i) {
        auto &mod = mod_props.pDrmFormatModifierProperties[i];
        if (((mod.drmFormatModifierTilingFeatures &
              (VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT | VK_FORMAT_FEATURE_DISJOINT_BIT)) ==
             (VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT | VK_FORMAT_FEATURE_DISJOINT_BIT))) {
            mods.push_back(mod.drmFormatModifier);
        }
    }

    if (mods.empty()) {
        GTEST_SKIP() << "Required format features not supported.";
    }

    VkImageDrmFormatModifierListCreateInfoEXT mod_list = vku::InitStructHelper();
    mod_list.pDrmFormatModifiers = mods.data();
    mod_list.drmFormatModifierCount = mods.size();

    VkImageCreateInfo image_create_info = vku::InitStructHelper(&mod_list);
    image_create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = format;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    VkImageMemoryRequirementsInfo2 mem_req_info2 = vku::InitStructHelper();
    mem_req_info2.image = image;
    VkMemoryRequirements2 mem_req2 = vku::InitStructHelper();
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryRequirementsInfo2-image-01589");
    m_errorMonitor->SetDesiredError("VUID-VkImageMemoryRequirementsInfo2-image-02279");
    vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mem_req2);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeYcbcr, DrawFetch) {
    TEST_DESCRIPTION("Do OpImageFetch on a Ycbcr COMBINED_IMAGE_SAMPLER.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitBasicYcbcr());
    InitRenderTarget();
    const VkFormat format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR;

    auto ci = vku::InitStruct<VkImageCreateInfo>();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = format;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    ci.extent = {256, 256, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    if (!ImageFormatIsSupported(instance(), gpu(), ci, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        // Assume there's low ROI on searching for different mp formats
        GTEST_SKIP() << "Multiplane image format not supported";
    }
    vkt::Image image(*m_device, ci, vkt::set_layout);

    vkt::SamplerYcbcrConversion conversion(*m_device, format);
    auto conversion_info = conversion.ConversionInfo();
    auto ivci = vku::InitStruct<VkImageViewCreateInfo>(&conversion_info);
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = format;
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkt::ImageView view(*m_device, ivci);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.pNext = &conversion_info;
    vkt::Sampler sampler(*m_device, sampler_ci);

    OneOffDescriptorSet descriptor_set(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, &sampler.handle()},
                  });
    if (!descriptor_set.set_) {
        GTEST_SKIP() << "Can't allocate descriptor with immutable sampler";
    }

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorImageInfo(0, view.handle(), sampler.handle());
    descriptor_set.UpdateDescriptorSets();

    const char fsSource[] = R"glsl(
        #version 450
        layout (set = 0, binding = 0) uniform sampler2D ycbcr;
        layout(location=0) out vec4 out_color;
        void main() {
            out_color = texelFetch(ycbcr, ivec2(0), 0);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-06550");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeYcbcr, DrawConstOffset) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitBasicYcbcr());
    InitRenderTarget();
    const VkFormat format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR;

    auto ci = vku::InitStruct<VkImageCreateInfo>();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = format;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    ci.extent = {256, 256, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    if (!ImageFormatIsSupported(instance(), gpu(), ci, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        // Assume there's low ROI on searching for different mp formats
        GTEST_SKIP() << "Multiplane image format not supported";
    }
    vkt::Image image(*m_device, ci, vkt::set_layout);

    vkt::SamplerYcbcrConversion conversion(*m_device, format);
    auto conversion_info = conversion.ConversionInfo();
    auto ivci = vku::InitStruct<VkImageViewCreateInfo>(&conversion_info);
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = format;
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkt::ImageView view(*m_device, ivci);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.pNext = &conversion_info;
    vkt::Sampler sampler(*m_device, sampler_ci);

    OneOffDescriptorSet descriptor_set(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, &sampler.handle()},
                  });
    if (!descriptor_set.set_) {
        GTEST_SKIP() << "Can't allocate descriptor with immutable sampler";
    }

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorImageInfo(0, view.handle(), sampler.handle());
    descriptor_set.UpdateDescriptorSets();

    const char fsSource[] = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %out_color
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %out_color "out_color"
               OpName %ycbcr "ycbcr"
               OpDecorate %out_color Location 0
               OpDecorate %ycbcr DescriptorSet 0
               OpDecorate %ycbcr Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %out_color = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
      %ycbcr = OpVariable %_ptr_UniformConstant_11 UniformConstant
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %17 = OpConstantComposite %v2float %float_0 %float_0
      %v2int = OpTypeVector %int 2
      %int_0 = OpConstant %int 0
   %offset_0 = OpConstantComposite %v2int %int_0 %int_0
       %main = OpFunction %void None %3
          %5 = OpLabel
         %14 = OpLoad %11 %ycbcr
         %18 = OpImageSampleImplicitLod %v4float %14 %17 ConstOffset %offset_0
               OpStore %out_color %18
               OpReturn
               OpFunctionEnd
    )";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-ConstOffset-06551");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}
