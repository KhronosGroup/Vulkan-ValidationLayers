/*
 * Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (c) 2015-2026 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <gtest/gtest.h>
#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"

class PositiveSampler : public VkLayerTest {};

TEST_F(PositiveSampler, SamplerMirrorClampToEdgeWithoutFeature) {
    TEST_DESCRIPTION("Use VK_KHR_sampler_mirror_clamp_to_edge in 1.1 before samplerMirrorClampToEdge feature was added");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();
    if (DeviceValidationVersion() != VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Test requires Vulkan 1.1 exactly";
    }

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    vkt::Sampler sampler(*m_device, sampler_info);
}

TEST_F(PositiveSampler, SamplerMirrorClampToEdgeWithoutFeature12) {
    TEST_DESCRIPTION("Use VK_KHR_sampler_mirror_clamp_to_edge in 1.2 using the extension");

    // We need to explicitly allow promoted extensions to be enabled as this test relies on this behavior
    AllowPromotedExtensions();

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    vkt::Sampler sampler(*m_device, sampler_info);
}

TEST_F(PositiveSampler, SamplerMirrorClampToEdgeWithFeature) {
    TEST_DESCRIPTION("Use VK_KHR_sampler_mirror_clamp_to_edge in 1.2 with feature bit enabled");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceVulkan12Features features12 = vku::InitStructHelper();
    features12.samplerMirrorClampToEdge = VK_TRUE;
    auto features2 = GetPhysicalDeviceFeatures2(features12);
    if (features12.samplerMirrorClampToEdge == VK_FALSE) {
        GTEST_SKIP() << "samplerMirrorClampToEdge not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    vkt::Sampler sampler(*m_device, sampler_info);
}

TEST_F(PositiveSampler, SamplerConversionDifferentHandle) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/10920");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_6_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance6);
    AddRequiredFeature(vkt::Feature::samplerYcbcrConversion);
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image_ci.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    image_ci.tiling = VK_IMAGE_TILING_LINEAR;

    if (!IsImageFormatSupported(Gpu(), image_ci, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        GTEST_SKIP() << "Multiplane image format not supported";
    } else if (!FormatFeaturesAreSupported(Gpu(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                           VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    VkSamplerYcbcrConversionCreateInfo ycbcr_create_info = vku::InitStructHelper();
    ycbcr_create_info.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    ycbcr_create_info.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    ycbcr_create_info.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    ycbcr_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                    VK_COMPONENT_SWIZZLE_IDENTITY};
    ycbcr_create_info.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.chromaFilter = VK_FILTER_NEAREST;
    ycbcr_create_info.forceExplicitReconstruction = false;
    vkt::SamplerYcbcrConversion conversions_0(*m_device, ycbcr_create_info);
    vkt::SamplerYcbcrConversion conversions_1(*m_device, ycbcr_create_info);

    VkSamplerYcbcrConversionInfo ycbcr_info = vku::InitStructHelper();
    ycbcr_info.conversion = conversions_0;
    VkSamplerCreateInfo sci = SafeSaneSamplerCreateInfo(&ycbcr_info);
    vkt::Sampler samplers_0(*m_device, sci);

    vkt::Image mpimage(*m_device, image_ci, vkt::set_layout);
    ycbcr_info.conversion = conversions_1;
    vkt::ImageView image_view = mpimage.CreateView(VK_IMAGE_ASPECT_PLANE_0_BIT, &ycbcr_info);

    VkDescriptorSetLayoutBinding bindings{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL,
                                          &samplers_0.handle()};
    vkt::DescriptorSetLayout layout(*m_device, bindings);

    VkPhysicalDeviceMaintenance6PropertiesKHR maintenance6_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(maintenance6_props);

    // Can't use OneOffDescriptorSet because need to account for YCbCr size
    VkDescriptorPoolSize pool_sizes{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                    maintenance6_props.maxCombinedImageSamplerDescriptorCount};
    VkDescriptorPoolCreateInfo pool_ci = vku::InitStructHelper();
    pool_ci.flags = 0;
    pool_ci.maxSets = 1;
    pool_ci.poolSizeCount = 1;
    pool_ci.pPoolSizes = &pool_sizes;
    vkt::DescriptorPool pool(*m_device, pool_ci);

    VkDescriptorSetAllocateInfo ds_alloc_info = vku::InitStructHelper();
    ds_alloc_info.descriptorPool = pool;
    ds_alloc_info.descriptorSetCount = 1;
    ds_alloc_info.pSetLayouts = &layout.handle();
    VkDescriptorSet descriptor_set;
    vk::AllocateDescriptorSets(*m_device, &ds_alloc_info, &descriptor_set);

    VkDescriptorImageInfo image_info = {VK_NULL_HANDLE, image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = &image_info;
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
}
