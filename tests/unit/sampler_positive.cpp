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
#include "layer_validation_tests.h"
#include "pipeline_helper.h"

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

TEST_F(PositiveSampler, FilterCubicRangeClampWithReducionMode) {
    TEST_DESCRIPTION("Sample an image view with VK_FILTER_CUBIC_EXT filter and sampler reductionMode "
                     "VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE_RANGECLAMP_QCOM while the cubicRangeClamp feature is enabled.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_FILTER_CUBIC_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
    AddRequiredExtensions(VK_QCOM_FILTER_CUBIC_CLAMP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::cubicRangeClamp);
    AddRequiredFeature(vkt::Feature::samplerFilterMinmax);
    RETURN_IF_SKIP(Init());

    constexpr VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkPhysicalDeviceImageViewImageFormatInfoEXT phy_image_view_info = vku::InitStructHelper();
    phy_image_view_info.imageViewType = VK_IMAGE_VIEW_TYPE_2D;
    VkPhysicalDeviceImageFormatInfo2 phy_image_format_info2 = vku::InitStructHelper(&phy_image_view_info);
    phy_image_format_info2.format = format;
    phy_image_format_info2.type = VK_IMAGE_TYPE_2D;
    phy_image_format_info2.tiling = VK_IMAGE_TILING_OPTIMAL;
    phy_image_format_info2.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFilterCubicImageViewImageFormatPropertiesEXT cubic_props = vku::InitStructHelper();
    VkImageFormatProperties2 image_format_props2 = vku::InitStructHelper(&cubic_props);
    vk::GetPhysicalDeviceImageFormatProperties2(Gpu(), &phy_image_format_info2, &image_format_props2);

    if (!cubic_props.filterCubic) {
        GTEST_SKIP() << "VK_FORMAT_R8G8B8A8_UNORM doesn't support filterCubic feature, skipping test.";
    }

    vkt::Image sampled_image{*m_device, 64, 64, format, VK_IMAGE_USAGE_SAMPLED_BIT};
    vkt::ImageView image_view = sampled_image.CreateView();

    VkSamplerReductionModeCreateInfo reduction_mode_ci = vku::InitStructHelper();
    reduction_mode_ci.reductionMode = VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE_RANGECLAMP_QCOM;
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo(&reduction_mode_ci);
    sampler_ci.magFilter = VK_FILTER_CUBIC_EXT;
    sampler_ci.minFilter = VK_FILTER_CUBIC_EXT;
    vkt::Sampler sampler{*m_device, sampler_ci};

    const char* cs_source = R"glsl(
        #version 450

        layout(set = 0, binding = 0) uniform sampler2D sampled_tex;
        layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

        void main() {
            vec4 color = texture(sampled_tex, vec2(0.5, 0.5));
        }
    )glsl";

    CreateComputePipelineHelper compute_pipe{*this};
    compute_pipe.cs_ = VkShaderObj{*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2};
    compute_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    compute_pipe.CreateComputePipeline();
    compute_pipe.descriptor_set_.WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    compute_pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe.pipeline_layout_, 0, 1,
                              &compute_pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
}

TEST_F(PositiveSampler, BlitImageWithFilterCubicWeights) {
    TEST_DESCRIPTION("Blit an image with VK_FILTER_CUBIC_EXT filter while the selectableCubicWeights feature is not enabled, "
                     "with cubicFilterWeights set to VK_CUBIC_FILTER_WEIGHTS_CATMULL_ROM_QCOM.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_FILTER_CUBIC_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
    AddRequiredExtensions(VK_QCOM_FILTER_CUBIC_WEIGHTS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::samplerFilterMinmax);
    RETURN_IF_SKIP(Init());

    constexpr VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    if (!FormatFeaturesAreSupported(Gpu(), format, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT |
                                    VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT)) {
        GTEST_SKIP() << "VK_FORMAT_R8G8B8A8_UNORM doesn't support blit or filterCubic feature, skipping test.";
    }

    vkt::Image src_image{*m_device, 64, 64, format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT};
    vkt::Image dst_image{*m_device, 64, 64, format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT};
    VkImageBlit2 image_blit2 = vku::InitStructHelper();
    image_blit2.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_blit2.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    image_blit2.srcOffsets[1] = {63, 63, 1};
    image_blit2.dstOffsets[1] = {63, 63, 1};
    VkBlitImageCubicWeightsInfoQCOM weights_info = vku::InitStructHelper();
    weights_info.cubicWeights = VK_CUBIC_FILTER_WEIGHTS_CATMULL_ROM_QCOM;
    VkBlitImageInfo2 blit_image_info2 = vku::InitStructHelper(&weights_info);
    blit_image_info2.srcImage = src_image;
    blit_image_info2.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    blit_image_info2.dstImage = dst_image;
    blit_image_info2.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    blit_image_info2.regionCount = 1;
    blit_image_info2.pRegions = &image_blit2;
    blit_image_info2.filter = VK_FILTER_CUBIC_EXT;

    m_command_buffer.Begin();
    vk::CmdBlitImage2(m_command_buffer, &blit_image_info2);
    m_command_buffer.End();
}

TEST_F(PositiveSampler, ImageSampledWithFilterCubicWeights) {
    TEST_DESCRIPTION("Sample an image view with VK_FILTER_CUBIC_EXT filter while the selectableCubicWeights feature is enabled "
                     "and cubicFilterWeights is set to VK_CUBIC_FILTER_WEIGHTS_MITCHELL_NETRAVALI_QCOM.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_FILTER_CUBIC_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
    AddRequiredExtensions(VK_QCOM_FILTER_CUBIC_WEIGHTS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::samplerFilterMinmax);
    AddRequiredFeature(vkt::Feature::selectableCubicWeights);
    RETURN_IF_SKIP(Init());

    constexpr VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkPhysicalDeviceImageViewImageFormatInfoEXT phy_image_view_info = vku::InitStructHelper();
    phy_image_view_info.imageViewType = VK_IMAGE_VIEW_TYPE_2D;
    VkPhysicalDeviceImageFormatInfo2 phy_image_format_info2 = vku::InitStructHelper(&phy_image_view_info);
    phy_image_format_info2.format = format;
    phy_image_format_info2.type = VK_IMAGE_TYPE_2D;
    phy_image_format_info2.tiling = VK_IMAGE_TILING_OPTIMAL;
    phy_image_format_info2.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFilterCubicImageViewImageFormatPropertiesEXT cubic_props = vku::InitStructHelper();
    VkImageFormatProperties2 image_format_props2 = vku::InitStructHelper(&cubic_props);
    vk::GetPhysicalDeviceImageFormatProperties2(Gpu(), &phy_image_format_info2, &image_format_props2);

    if (!cubic_props.filterCubic) {
        GTEST_SKIP() << "VK_FORMAT_R8G8B8A8_UNORM doesn't support filterCubic feature, skipping test.";
    }

    vkt::Image sampled_image{*m_device, 64, 64, format, VK_IMAGE_USAGE_SAMPLED_BIT};
    vkt::ImageView image_view = sampled_image.CreateView();

    VkSamplerCubicWeightsCreateInfoQCOM weights_ci = vku::InitStructHelper();
    weights_ci.cubicWeights = VK_CUBIC_FILTER_WEIGHTS_MITCHELL_NETRAVALI_QCOM;
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo(&weights_ci);
    sampler_ci.magFilter = VK_FILTER_CUBIC_EXT;
    sampler_ci.minFilter = VK_FILTER_CUBIC_EXT;
    vkt::Sampler sampler{*m_device, sampler_ci};

    const char* cs_source = R"glsl(
        #version 450

        layout(set = 0, binding = 0) uniform sampler2D sampled_tex;
        layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

        void main() {
            vec4 color = texture(sampled_tex, vec2(0.5, 0.5));
        }
    )glsl";

    CreateComputePipelineHelper compute_pipe{*this};
    compute_pipe.cs_ = VkShaderObj{*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2};
    compute_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    compute_pipe.CreateComputePipeline();
    compute_pipe.descriptor_set_.WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    compute_pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe.pipeline_layout_, 0, 1,
                              &compute_pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();
}
