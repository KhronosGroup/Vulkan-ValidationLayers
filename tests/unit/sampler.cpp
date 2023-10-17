/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
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
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"

TEST_F(NegativeSampler, MirrorClampToEdgeNotEnabled) {
    TEST_DESCRIPTION("Validation should catch using CLAMP_TO_EDGE addressing mode if the extension is not enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_0);
    RETURN_IF_SKIP(Init())

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    // Set the modes to cause the error
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    // Prior to 1.2 we get the implicit VU
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-addressModeU-parameter");
}

TEST_F(NegativeSampler, MirrorClampToEdgeNotEnabled12) {
    TEST_DESCRIPTION("Validation using CLAMP_TO_EDGE for Vulkan 1.2 without the samplerMirrorClampToEdge feature enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init())

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-addressModeU-01079");
}

TEST_F(NegativeSampler, AnisotropyFeatureDisabled) {
    TEST_DESCRIPTION("Validation should check anisotropy parameters are correct with samplerAnisotropy disabled.");

    // Determine if required device features are available
    VkPhysicalDeviceFeatures device_features = {};
    RETURN_IF_SKIP(InitFramework())
    GetPhysicalDeviceFeatures(&device_features);
    device_features.samplerAnisotropy = VK_FALSE;  // force anisotropy off
    RETURN_IF_SKIP(InitState(&device_features));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCreateInfo-anisotropyEnable-01070");
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    // With the samplerAnisotropy disable, the sampler must not enable it.
    sampler_info.anisotropyEnable = VK_TRUE;
    vkt::Sampler sampler(*m_device, sampler_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSampler, AnisotropyFeatureEnabled) {
    TEST_DESCRIPTION("Validation must check several conditions that apply only when Anisotropy is enabled.");

    AddOptionalExtensions(VK_IMG_FILTER_CUBIC_EXTENSION_NAME);
    VkPhysicalDeviceFeatures device_features = {};
    RETURN_IF_SKIP(InitFramework())
    const bool cubic_support = IsExtensionsEnabled(VK_IMG_FILTER_CUBIC_EXTENSION_NAME);
    GetPhysicalDeviceFeatures(&device_features);

    // These tests require that the device support anisotropic filtering
    if (VK_TRUE != device_features.samplerAnisotropy) {
        GTEST_SKIP() << "Test requires unsupported samplerAnisotropy feature";
    }

    VkSamplerCreateInfo sampler_info_ref = SafeSaneSamplerCreateInfo();
    sampler_info_ref.anisotropyEnable = VK_TRUE;
    VkSamplerCreateInfo sampler_info = sampler_info_ref;
    RETURN_IF_SKIP(InitState())

    // maxAnisotropy out-of-bounds low.
    sampler_info.maxAnisotropy = NearestSmaller(1.0F);
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-anisotropyEnable-01071");
    sampler_info.maxAnisotropy = sampler_info_ref.maxAnisotropy;

    // maxAnisotropy out-of-bounds high.
    sampler_info.maxAnisotropy = NearestGreater(m_device->phy().limits_.maxSamplerAnisotropy);
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-anisotropyEnable-01071");
    sampler_info.maxAnisotropy = sampler_info_ref.maxAnisotropy;

    // Both anisotropy and unnormalized coords enabled
    sampler_info.unnormalizedCoordinates = VK_TRUE;
    // If unnormalizedCoordinates is VK_TRUE, minLod and maxLod must be zero
    sampler_info.minLod = 0;
    sampler_info.maxLod = 0;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01076");
    sampler_info.unnormalizedCoordinates = sampler_info_ref.unnormalizedCoordinates;

    // Both anisotropy and cubic filtering enabled
    if (cubic_support) {
        sampler_info.minFilter = VK_FILTER_CUBIC_IMG;
        CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-magFilter-01081");
        sampler_info.minFilter = sampler_info_ref.minFilter;

        sampler_info.magFilter = VK_FILTER_CUBIC_IMG;
        CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-magFilter-01081");
        sampler_info.magFilter = sampler_info_ref.magFilter;
    } else {
        printf("Test requires unsupported extension \"VK_IMG_filter_cubic\". Skipped.\n");
    }
}

TEST_F(NegativeSampler, UnnormalizedCoordinatesEnabled) {
    TEST_DESCRIPTION("Validate restrictions on sampler parameters when unnormalizedCoordinates is true.");

    RETURN_IF_SKIP(InitFramework())
    VkSamplerCreateInfo sampler_info_ref = SafeSaneSamplerCreateInfo();
    sampler_info_ref.unnormalizedCoordinates = VK_TRUE;
    sampler_info_ref.minLod = 0.0f;
    sampler_info_ref.maxLod = 0.0f;
    VkSamplerCreateInfo sampler_info = sampler_info_ref;
    RETURN_IF_SKIP(InitState())

    // min and mag filters must be the same
    sampler_info.minFilter = VK_FILTER_NEAREST;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01072");
    std::swap(sampler_info.minFilter, sampler_info.magFilter);
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01072");
    sampler_info = sampler_info_ref;

    // mipmapMode must be NEAREST
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01073");
    sampler_info = sampler_info_ref;

    // minlod and maxlod must be zero
    sampler_info.maxLod = 3.14159f;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01074");
    sampler_info.minLod = 2.71828f;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01074");
    sampler_info = sampler_info_ref;

    // addressModeU and addressModeV must both be CLAMP_TO_EDGE or CLAMP_TO_BORDER
    // checks all 12 invalid combinations out of 16 total combinations
    const std::array<VkSamplerAddressMode, 4> kAddressModes = {{
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
    }};
    for (const auto umode : kAddressModes) {
        for (const auto vmode : kAddressModes) {
            if ((umode != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE && umode != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) ||
                (vmode != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE && vmode != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)) {
                sampler_info.addressModeU = umode;
                sampler_info.addressModeV = vmode;
                CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01075");
            }
        }
    }
    sampler_info = sampler_info_ref;

    // VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01076 is tested in AnisotropyFeatureEnabled above
    // Since it requires checking/enabling the anisotropic filtering feature, it's easier to do it
    // with the other anisotropic tests.

    // compareEnable must be VK_FALSE
    sampler_info.compareEnable = VK_TRUE;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01077");
    sampler_info = sampler_info_ref;
}

TEST_F(NegativeSampler, BasicUsage) {
    TEST_DESCRIPTION("Checks various cases where VkSamplerCreateInfo is invalid");
    RETURN_IF_SKIP(Init())

    // reference to reset values between test cases
    VkSamplerCreateInfo const sampler_info_ref = SafeSaneSamplerCreateInfo();
    VkSamplerCreateInfo sampler_info = sampler_info_ref;

    // Mix up Lod values
    sampler_info.minLod = 4.0f;
    sampler_info.maxLod = 1.0f;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-maxLod-01973");
    sampler_info.minLod = sampler_info_ref.minLod;
    sampler_info.maxLod = sampler_info_ref.maxLod;

    // Larger mipLodBias than max limit
    sampler_info.mipLodBias = NearestGreater(m_device->phy().limits_.maxSamplerLodBias);
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-mipLodBias-01069");
    sampler_info.mipLodBias = sampler_info_ref.mipLodBias;
}

TEST_F(NegativeSampler, AllocationCount) {
    VkResult err = VK_SUCCESS;
    const int max_samplers = 32;
    VkSampler samplers[max_samplers + 1];

    RETURN_IF_SKIP(InitFramework())

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }
    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    if (props.limits.maxSamplerAllocationCount > max_samplers) {
        props.limits.maxSamplerAllocationCount = max_samplers;
        fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    }
    RETURN_IF_SKIP(InitState())
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateSampler-maxSamplerAllocationCount-04110");

    VkSamplerCreateInfo sampler_create_info = SafeSaneSamplerCreateInfo();

    int i;
    for (i = 0; i <= max_samplers; i++) {
        err = vk::CreateSampler(m_device->device(), &sampler_create_info, NULL, &samplers[i]);
        if (err != VK_SUCCESS) {
            break;
        }
    }
    m_errorMonitor->VerifyFound();

    for (int j = 0; j < i; j++) {
        vk::DestroySampler(m_device->device(), samplers[j], NULL);
    }
}

TEST_F(NegativeSampler, ImageViewFormatUnsupportedFilter) {
    TEST_DESCRIPTION(
        "Create sampler with a filter and use with image view using a format that does not support the sampler filter.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddOptionalExtensions(VK_IMG_FILTER_CUBIC_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    const bool cubic_support = IsExtensionsEnabled(VK_IMG_FILTER_CUBIC_EXTENSION_NAME);

    enum FormatTypes { FLOAT, SINT, UINT };

    struct TestFilterType {
        VkFilter filter = VK_FILTER_LINEAR;
        VkFormatFeatureFlagBits required_format_feature = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
        VkImageTiling tiling = VK_IMAGE_TILING_LINEAR;
        VkFormat format = VK_FORMAT_UNDEFINED;
        FormatTypes format_type;
        std::string err_msg;
    };

    std::vector<std::pair<VkFormat, FormatTypes>> formats_to_check({{VK_FORMAT_R8_UNORM, FLOAT},
                                                                    {VK_FORMAT_R8_SNORM, FLOAT},
                                                                    {VK_FORMAT_R8_SRGB, FLOAT},
                                                                    {VK_FORMAT_R8G8_UNORM, FLOAT},
                                                                    {VK_FORMAT_R8G8_SNORM, FLOAT},
                                                                    {VK_FORMAT_R8G8_SRGB, FLOAT},
                                                                    {VK_FORMAT_R8G8B8_UNORM, FLOAT},
                                                                    {VK_FORMAT_R8G8B8_SNORM, FLOAT},
                                                                    {VK_FORMAT_R8G8B8_SRGB, FLOAT},
                                                                    {VK_FORMAT_R8G8B8A8_UNORM, FLOAT},
                                                                    {VK_FORMAT_R8G8B8A8_SNORM, FLOAT},
                                                                    {VK_FORMAT_R8G8B8A8_SRGB, FLOAT},
                                                                    {VK_FORMAT_B8G8R8A8_UNORM, FLOAT},
                                                                    {VK_FORMAT_B8G8R8A8_SNORM, FLOAT},
                                                                    {VK_FORMAT_B8G8R8A8_SRGB, FLOAT},
                                                                    {VK_FORMAT_R16_UNORM, FLOAT},
                                                                    {VK_FORMAT_R16_SNORM, FLOAT},
                                                                    {VK_FORMAT_R16_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R16G16_UNORM, FLOAT},
                                                                    {VK_FORMAT_R16G16_SNORM, FLOAT},
                                                                    {VK_FORMAT_R16G16_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R16G16B16_UNORM, FLOAT},
                                                                    {VK_FORMAT_R16G16B16_SNORM, FLOAT},
                                                                    {VK_FORMAT_R16G16B16_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R16G16B16A16_UNORM, FLOAT},
                                                                    {VK_FORMAT_R16G16B16A16_SNORM, FLOAT},
                                                                    {VK_FORMAT_R16G16B16A16_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R32_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R32G32_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R32G32B32_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R32G32B32A32_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R64_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R64G64_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R64G64B64_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R64G64B64A64_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R8_SINT, SINT},
                                                                    {VK_FORMAT_R8G8_SINT, SINT},
                                                                    {VK_FORMAT_R8G8B8_SINT, SINT},
                                                                    {VK_FORMAT_R8G8B8A8_SINT, SINT},
                                                                    {VK_FORMAT_B8G8R8A8_SINT, SINT},
                                                                    {VK_FORMAT_R16_SINT, SINT},
                                                                    {VK_FORMAT_R16G16_SINT, SINT},
                                                                    {VK_FORMAT_R16G16B16_SINT, SINT},
                                                                    {VK_FORMAT_R16G16B16A16_SINT, SINT},
                                                                    {VK_FORMAT_R32_SINT, SINT},
                                                                    {VK_FORMAT_R32G32_SINT, SINT},
                                                                    {VK_FORMAT_R32G32B32_SINT, SINT},
                                                                    {VK_FORMAT_R32G32B32A32_SINT, SINT},
                                                                    {VK_FORMAT_R64_SINT, SINT},
                                                                    {VK_FORMAT_R64G64_SINT, SINT},
                                                                    {VK_FORMAT_R64G64B64_SINT, SINT},
                                                                    {VK_FORMAT_R64G64B64A64_SINT, SINT},
                                                                    {VK_FORMAT_R8_UINT, UINT},
                                                                    {VK_FORMAT_R8G8_UINT, UINT},
                                                                    {VK_FORMAT_R8G8B8_UINT, UINT},
                                                                    {VK_FORMAT_R8G8B8A8_UINT, UINT},
                                                                    {VK_FORMAT_B8G8R8A8_UINT, UINT},
                                                                    {VK_FORMAT_R16_UINT, UINT},
                                                                    {VK_FORMAT_R16G16_UINT, UINT},
                                                                    {VK_FORMAT_R16G16B16_UINT, UINT},
                                                                    {VK_FORMAT_R16G16B16A16_UINT, UINT},
                                                                    {VK_FORMAT_R32_UINT, UINT},
                                                                    {VK_FORMAT_R32G32_UINT, UINT},
                                                                    {VK_FORMAT_R32G32B32_UINT, UINT},
                                                                    {VK_FORMAT_R32G32B32A32_UINT, UINT},
                                                                    {VK_FORMAT_R64_UINT, UINT},
                                                                    {VK_FORMAT_R64G64_UINT, UINT},
                                                                    {VK_FORMAT_R64G64B64_UINT, UINT},
                                                                    {VK_FORMAT_R64G64B64A64_UINT, UINT}});

    std::vector<struct TestFilterType> tests(2);
    tests[0].err_msg = "VUID-vkCmdDraw-magFilter-04553";

    tests[1].filter = VK_FILTER_CUBIC_IMG;
    tests[1].required_format_feature = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG;
    tests[1].err_msg = "VUID-vkCmdDraw-None-02692";

    for (auto &test_struct : tests) {
        for (std::pair<VkFormat, FormatTypes> cur_format_pair : formats_to_check) {
            VkFormatProperties props = {};
            vk::GetPhysicalDeviceFormatProperties(gpu(), cur_format_pair.first, &props);
            if (test_struct.format == VK_FORMAT_UNDEFINED && props.linearTilingFeatures != 0 &&
                (props.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) &&
                !(props.linearTilingFeatures & test_struct.required_format_feature)) {
                test_struct.format = cur_format_pair.first;
                test_struct.format_type = cur_format_pair.second;
            } else if (test_struct.format == VK_FORMAT_UNDEFINED && props.optimalTilingFeatures != 0 &&
                       (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) &&
                       !(props.optimalTilingFeatures & test_struct.required_format_feature)) {
                test_struct.format = cur_format_pair.first;
                test_struct.format_type = cur_format_pair.second;
                test_struct.tiling = VK_IMAGE_TILING_OPTIMAL;
            }

            if (test_struct.format != VK_FORMAT_UNDEFINED) {
                break;
            }
        }
    }

    const char bindStateFragiSamplerShaderText[] = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform isampler2D s;
        layout(location=0) out vec4 x;
        void main(){
           x = texture(s, vec2(1));
        }
    )glsl";

    const char bindStateFraguSamplerShaderText[] = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform usampler2D s;
        layout(location=0) out vec4 x;
        void main(){
           x = texture(s, vec2(1));
        }
    )glsl";

    InitRenderTarget();

    for (const auto &test_struct : tests) {
        if (test_struct.format == VK_FORMAT_UNDEFINED) {
            printf("Could not find a testable format for filter %d.  Skipping test for said filter.\n", test_struct.filter);
            continue;
        }

        VkSamplerCreateInfo sci = SafeSaneSamplerCreateInfo();

        sci.magFilter = test_struct.filter;
        sci.minFilter = test_struct.filter;
        sci.compareEnable = VK_FALSE;

        if (test_struct.filter == VK_FILTER_CUBIC_IMG) {
            if (cubic_support) {
                sci.anisotropyEnable = VK_FALSE;
            } else {
                printf("VK_FILTER_CUBIC_IMG not supported.  Skipping use of VK_FILTER_CUBIC_IMG this test.\n");
                continue;
            }
        }

        vkt::Sampler sampler(*m_device, sci);

        VkImageObj mpimage(m_device);
        mpimage.Init(128, 128, 1, test_struct.format, VK_IMAGE_USAGE_SAMPLED_BIT, test_struct.tiling);
        ASSERT_TRUE(mpimage.initialized());

        VkImageView view = mpimage.targetView(test_struct.format);

        CreatePipelineHelper pipe(*this);
        VkShaderObj *fs = nullptr;

        if (test_struct.format_type == FLOAT) {
            fs = new VkShaderObj(this, kFragmentSamplerGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
        } else if (test_struct.format_type == SINT) {
            fs = new VkShaderObj(this, bindStateFragiSamplerShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);
        } else if (test_struct.format_type == UINT) {
            fs = new VkShaderObj(this, bindStateFraguSamplerShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);
        }

        pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs->GetStageCreateInfo()};
        pipe.dsl_bindings_ = {
            {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
        };
        pipe.InitState();
        ASSERT_EQ(VK_SUCCESS, pipe.CreateGraphicsPipeline());

        pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        pipe.descriptor_set_->UpdateDescriptorSets();

        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                                  &pipe.descriptor_set_->set_, 0, nullptr);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test_struct.err_msg.c_str());
        vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();

        delete fs;
    }
}

TEST_F(NegativeSampler, AddressModeWithCornerSampledNV) {
    TEST_DESCRIPTION(
        "Create image with VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV flag and sample it with something other than "
        "VK_SAMPLER_ADDRESS_MODE_CLAMP_EDGE.");

    AddRequiredExtensions(VK_NV_CORNER_SAMPLED_IMAGE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState(nullptr, nullptr, 0));
    InitRenderTarget();

    VkImageObj test_image(m_device);
    VkImageCreateInfo image_info = VkImageObj::create_info();
    image_info.flags = VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    // If flags contains VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV,
    // imageType must be VK_IMAGE_TYPE_2D or VK_IMAGE_TYPE_3D
    image_info.imageType = VK_IMAGE_TYPE_2D;
    // If flags contains VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV and imageType is VK_IMAGE_TYPE_2D,
    // extent.width and extent.height must be greater than 1.
    image_info.extent = {2, 2, 1};
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    test_image.init(&image_info);
    ASSERT_TRUE(test_image.initialized());

    VkSamplerCreateInfo sci = SafeSaneSamplerCreateInfo();
    sci.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkt::Sampler sampler(*m_device, sci);

    VkImageView view = test_image.targetView(image_info.format);

    CreatePipelineHelper pipe(*this);
    VkShaderObj fs(this, kFragmentSamplerGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);

    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-flags-02696");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeSampler, MultiplaneImageSamplerConversionMismatch) {
    TEST_DESCRIPTION(
        "Create sampler with ycbcr conversion and use with an image created without ycrcb conversion or immutable sampler");

    // Use 1.1 to get VK_KHR_sampler_ycbcr_conversion easier
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceVulkan11Features features11 = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(features11);
    if (features11.samplerYcbcrConversion != VK_TRUE) {
        GTEST_SKIP() << "SamplerYcbcrConversion not supported";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    InitRenderTarget();

    const VkImageCreateInfo ci = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                  NULL,
                                  VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,  // need for multi-planar
                                  VK_IMAGE_TYPE_2D,
                                  VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR,
                                  {128, 128, 1},
                                  1,
                                  1,
                                  VK_SAMPLE_COUNT_1_BIT,
                                  VK_IMAGE_TILING_LINEAR,
                                  VK_IMAGE_USAGE_SAMPLED_BIT,
                                  VK_SHARING_MODE_EXCLUSIVE,
                                  VK_IMAGE_LAYOUT_UNDEFINED};

    // Verify formats
    bool supported = ImageFormatIsSupported(instance(), gpu(), ci, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
    if (!supported) {
        GTEST_SKIP() << "Multiplane image format not supported";
    }

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    // Create Ycbcr conversion
    VkSamplerYcbcrConversionCreateInfo ycbcr_create_info = vku::InitStructHelper();
    ycbcr_create_info.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR;
    ycbcr_create_info.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    ycbcr_create_info.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    ycbcr_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                    VK_COMPONENT_SWIZZLE_IDENTITY};
    ycbcr_create_info.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.chromaFilter = VK_FILTER_NEAREST;
    ycbcr_create_info.forceExplicitReconstruction = false;
    vkt::SamplerYcbcrConversion conversions[2];
    conversions[0].init(*m_device, ycbcr_create_info, false);
    ycbcr_create_info.components.a = VK_COMPONENT_SWIZZLE_ZERO;  // Just anything different than above
    conversions[1].init(*m_device, ycbcr_create_info, false);

    VkSamplerYcbcrConversionInfo ycbcr_info = vku::InitStructHelper();
    ycbcr_info.conversion = conversions[0].handle();

    // Create a sampler using conversion
    VkSamplerCreateInfo sci = SafeSaneSamplerCreateInfo();
    sci.pNext = &ycbcr_info;
    // Create two samplers with two different conversions, such that one will mismatch
    // It will make the second sampler fail to see if the log prints the second sampler or the first sampler.
    vkt::Sampler samplers[2];
    samplers[0].init(*m_device, sci);
    ycbcr_info.conversion = conversions[1].handle();  // Need two samplers with different conversions
    samplers[1].init(*m_device, sci);

    vkt::Sampler BadSampler;
    sci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCreateInfo-addressModeU-01646");
    BadSampler.init(*m_device, sci);
    m_errorMonitor->VerifyFound();

    sci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sci.unnormalizedCoordinates = VK_TRUE;
    sci.minLod = 0.0;
    sci.maxLod = 0.0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCreateInfo-addressModeU-01646");
    BadSampler.init(*m_device, sci);
    m_errorMonitor->VerifyFound();

    if (features2.features.samplerAnisotropy == VK_TRUE) {
        sci.unnormalizedCoordinates = VK_FALSE;
        sci.anisotropyEnable = VK_TRUE;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCreateInfo-addressModeU-01646");
        BadSampler.init(*m_device, sci);
        m_errorMonitor->VerifyFound();
    }

    // Create an image without a Ycbcr conversion
    VkImageObj mpimage(m_device);
    mpimage.init(&ci);

    ycbcr_info.conversion = conversions[0].handle();  // Need two samplers with different conversions
    VkImageViewCreateInfo ivci = vku::InitStructHelper(&ycbcr_info);
    ivci.image = mpimage.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;

    vkt::ImageView view(*m_device, ivci);

    VkSampler vksamplers[2] = {samplers[0].handle(), samplers[1].handle()};
    // Use the image and sampler together in a descriptor set
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, vksamplers},
                                       });

    if (!descriptor_set.set_) {
        GTEST_SKIP() << "Failed to allocate descriptor set, skipping test.";
    }

    // Use the same image view twice, using the same sampler, with the *second* mismatched with the *second* immutable sampler
    VkDescriptorImageInfo image_infos[2];
    image_infos[0] = {};
    image_infos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_infos[0].imageView = view.handle();
    image_infos[0].sampler = samplers[0].handle();
    image_infos[1] = image_infos[0];

    // Update the descriptor set expecting to get an error
    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 2;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = image_infos;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-01948");
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    // pImmutableSamplers = nullptr causes an error , VUID-VkWriteDescriptorSet-descriptorType-02738.
    // Because if pNext chains a VkSamplerYcbcrConversionInfo, the sampler has to be a immutable sampler.
    OneOffDescriptorSet descriptor_set_1947(m_device,
                                            {
                                                {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                            });
    descriptor_write.dstSet = descriptor_set_1947.set_;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pImageInfo = &image_infos[0];
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-02738");
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSampler, FilterMinmax) {
    TEST_DESCRIPTION("Invalid uses of VK_EXT_sampler_filter_minmax.");

    // Enable KHR multiplane req'd extensions
    AddRequiredExtensions(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    // Enable Ycbcr Conversion Features
    VkPhysicalDeviceSamplerYcbcrConversionFeatures ycbcr_features = vku::InitStructHelper();
    ycbcr_features.samplerYcbcrConversion = VK_TRUE;
    RETURN_IF_SKIP(InitState(nullptr, &ycbcr_features));

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    // Create Ycbcr conversion
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

    VkSamplerYcbcrConversion conversion;
    vk::CreateSamplerYcbcrConversionKHR(m_device->handle(), &ycbcr_create_info, nullptr, &conversion);

    VkSamplerYcbcrConversionInfo ycbcr_info = vku::InitStructHelper();
    ycbcr_info.conversion = conversion;

    VkSamplerReductionModeCreateInfo reduction_info = vku::InitStructHelper();
    reduction_info.reductionMode = VK_SAMPLER_REDUCTION_MODE_MIN;

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.pNext = &reduction_info;

    // Wrong mode with a YCbCr Conversion used
    reduction_info.pNext = &ycbcr_info;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-None-01647");

    // Wrong mode with compareEnable
    reduction_info.pNext = nullptr;
    sampler_info.compareEnable = VK_TRUE;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-compareEnable-01423");

    vk::DestroySamplerYcbcrConversionKHR(m_device->handle(), conversion, nullptr);
}

TEST_F(NegativeSampler, CustomBorderColor) {
    TEST_DESCRIPTION("Tests for VUs for VK_EXT_custom_border_color");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceCustomBorderColorFeaturesEXT border_color_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(border_color_features);
    if (border_color_features.customBorderColors != VK_TRUE) {
        GTEST_SKIP() << "customBorderColors feature not supported";
    }

    // Disable without format
    border_color_features.customBorderColorWithoutFormat = 0;

    RETURN_IF_SKIP(InitState(nullptr, &border_color_features));

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.borderColor = VK_BORDER_COLOR_INT_CUSTOM_EXT;
    // No SCBCCreateInfo in pNext
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-borderColor-04011");

    VkSamplerCustomBorderColorCreateInfoEXT custom_color_cinfo = vku::InitStructHelper();
    custom_color_cinfo.format = VK_FORMAT_R32_SFLOAT;
    sampler_info.pNext = &custom_color_cinfo;
    // Format mismatch
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCustomBorderColorCreateInfoEXT-format-07605");

    custom_color_cinfo.format = VK_FORMAT_UNDEFINED;
    // Format undefined with no customBorderColorWithoutFormat
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCustomBorderColorCreateInfoEXT-format-04014");

    custom_color_cinfo.format = VK_FORMAT_R8G8B8A8_UINT;
    vkt::Sampler sampler(*m_device, sampler_info);

    VkDescriptorSetLayoutBinding dsl_binding = {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, &sampler.handle()};
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, NULL, 0, 1, &dsl_binding};
    VkDescriptorSetLayout ds_layout;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutBinding-pImmutableSamplers-04009");
    vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();

    VkPhysicalDeviceCustomBorderColorPropertiesEXT custom_properties = vku::InitStructHelper();
    auto prop2 = GetPhysicalDeviceProperties2(custom_properties);

    if ((custom_properties.maxCustomBorderColorSamplers <= 0xFFFF) &&
        (prop2.properties.limits.maxSamplerAllocationCount >= custom_properties.maxCustomBorderColorSamplers)) {
        VkSampler samplers[0xFFFF];
        // Still have one custom border color sampler from above, so this should exceed max
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCreateInfo-None-04012");
        if (prop2.properties.limits.maxSamplerAllocationCount <= custom_properties.maxCustomBorderColorSamplers) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateSampler-maxSamplerAllocationCount-04110");
        }
        for (uint32_t i = 0; i < custom_properties.maxCustomBorderColorSamplers; i++) {
            vk::CreateSampler(m_device->device(), &sampler_info, NULL, &samplers[i]);
        }
        m_errorMonitor->VerifyFound();
        for (uint32_t i = 0; i < custom_properties.maxCustomBorderColorSamplers - 1; i++) {
            vk::DestroySampler(m_device->device(), samplers[i], nullptr);
        }
    }
}

TEST_F(NegativeSampler, CustomBorderColorFormatUndefined) {
    TEST_DESCRIPTION("Tests for VUID-VkSamplerCustomBorderColorCreateInfoEXT-format-04015");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceCustomBorderColorFeaturesEXT border_color_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(border_color_features);
    if (!border_color_features.customBorderColors || !border_color_features.customBorderColorWithoutFormat) {
        GTEST_SKIP() << "Custom border color feature not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &border_color_features));
    InitRenderTarget();

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.borderColor = VK_BORDER_COLOR_INT_CUSTOM_EXT;
    VkSamplerCustomBorderColorCreateInfoEXT custom_color_cinfo = vku::InitStructHelper();
    custom_color_cinfo.format = VK_FORMAT_UNDEFINED;
    sampler_info.pNext = &custom_color_cinfo;
    vkt::Sampler sampler(*m_device, sampler_info);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_B4G4R4A4_UNORM_PACK16, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    image.Layout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    auto image_view_create_info = image.BasicViewCreatInfo();
    vkt::ImageView view(*m_device, image_view_create_info);

    VkDescriptorImageInfo img_info = {};
    img_info.sampler = sampler.handle();
    img_info.imageView = view.handle();
    img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[0].pImageInfo = &img_info;

    vk::UpdateDescriptorSets(m_device->device(), 1, descriptor_writes, 0, NULL);
    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler2D s;
        layout(location=0) out vec4 x;
        void main(){
           x = texture(s, vec2(1));
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, NULL);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCustomBorderColorCreateInfoEXT-format-04015");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeSampler, UnnormalizedCoordinatesCombinedSampler) {
    TEST_DESCRIPTION(
        "If a samper is unnormalizedCoordinates, the imageview has to be some specific types. Uses COMBINED_IMAGE_SAMPLER");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // This generates OpImage*Dref* instruction on R8G8B8A8_UNORM format.
    // Verify that it is allowed on this implementation if
    // VK_KHR_format_feature_flags2 is available.
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        VkFormatProperties3KHR fmt_props_3 = vku::InitStructHelper();
        VkFormatProperties2 fmt_props = vku::InitStructHelper(&fmt_props_3);

        vk::GetPhysicalDeviceFormatProperties2(gpu(), VK_FORMAT_R8G8B8A8_UNORM, &fmt_props);

        if (!(fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT_KHR)) {
            GTEST_SKIP() << "R8G8B8A8_UNORM does not support OpImage*Dref* operations";
        }
    }

    VkShaderObj vs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_VERTEX_BIT);

    const char fsSource[] = R"glsl(
        #version 450
        layout (set = 0, binding = 0) uniform sampler3D image_view_3d;
        layout (set = 0, binding = 1) uniform sampler2D tex[2];
        layout (set = 0, binding = 2) uniform sampler2DShadow tex_dep[2];
        void main() {
            // VUID 08609
            // 3D Image View is used with unnormalized coordinates
            // Also is VUID 08610 but the invalid image view is reported first
            vec4 x = texture(image_view_3d, vec3(0));

            // VUID 08610
            // OpImageSampleDrefImplicitLod is used with unnormalized coordinates
            float f = texture(tex_dep[0], vec3(0));

            // VUID 08611
            // OpImageSampleExplicitLod instructions that incudes a offset with unnormalized coordinates
            x = textureLodOffset(tex[1], vec2(0), 0, ivec2(0));
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL_GRAPHICS, nullptr},
                            {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.InitState();
    ASSERT_EQ(VK_SUCCESS, g_pipe.CreateGraphicsPipeline());

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image.Init(image_ci);
    ASSERT_TRUE(image.initialized());
    VkImageView view_pass = image.targetView(format);

    VkImageObj image_3d(m_device);
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_3d.Init(image_ci);
    ASSERT_TRUE(image_3d.initialized());

    // If the sampler is unnormalizedCoordinates, the imageview type shouldn't be 3D, CUBE, 1D_ARRAY, 2D_ARRAY, CUBE_ARRAY.
    // This causes DesiredFailure.
    VkImageView view_fail = image_3d.targetView(format, VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0,
                                                VK_REMAINING_ARRAY_LAYERS, VK_IMAGE_VIEW_TYPE_3D);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.unnormalizedCoordinates = VK_TRUE;
    sampler_ci.maxLod = 0;
    vkt::Sampler sampler(*m_device, sampler_ci);

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, view_fail, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(1, view_pass, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(1, view_pass, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(2, view_pass, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(2, view_pass, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-08609");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-08610");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-08611");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeSampler, UnnormalizedCoordinatesSeparateSampler) {
    TEST_DESCRIPTION(
        "If a samper is unnormalizedCoordinates, the imageview has to be some specific types. Doesn't use COMBINED_IMAGE_SAMPLER");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    // This generates OpImage*Dref* instruction on R8G8B8A8_UNORM format.
    // Verify that it is allowed on this implementation if
    // VK_KHR_format_feature_flags2 is available.
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        VkFormatProperties3KHR fmt_props_3 = vku::InitStructHelper();
        VkFormatProperties2 fmt_props = vku::InitStructHelper(&fmt_props_3);

        vk::GetPhysicalDeviceFormatProperties2(gpu(), VK_FORMAT_R8G8B8A8_UNORM, &fmt_props);

        if (!(fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT_KHR)) {
            GTEST_SKIP() << "R8G8B8A8_UNORM does not support OpImage*Dref* operations";
        }
    }

    VkShaderObj vs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_VERTEX_BIT);

    const char fsSource[] = R"glsl(
        #version 450
        // VK_DESCRIPTOR_TYPE_SAMPLER
        layout(set = 0, binding = 0) uniform sampler s1;
        layout(set = 0, binding = 1) uniform sampler s2;
        // VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
        layout(set = 0, binding = 2) uniform texture2D si_good;
        layout(set = 0, binding = 3) uniform texture2D si_good_2;
        layout(set = 0, binding = 4) uniform texture3D si_bad[2]; // 3D image view

        void main() {
            // VUID 08609
            // 3D Image View is used with unnormalized coordinates
            // Also is VUID 08610 but the invalid image view is reported first
            vec4 x = texture(sampler3D(si_bad[1], s1), vec3(0));

            // VUID 08610
            // OpImageSampleImplicitLod is used with unnormalized coordinates
            x = texture(sampler2D(si_good, s1), vec2(0));

            // VUID 08611
            // OpImageSampleExplicitLod instructions that incudes a offset with unnormalized coordinates
            x = textureLodOffset(sampler2D(si_good_2, s2), vec2(0), 0, ivec2(0));
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {3, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {4, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.InitState();
    ASSERT_EQ(VK_SUCCESS, g_pipe.CreateGraphicsPipeline());

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image.Init(image_ci);
    ASSERT_TRUE(image.initialized());
    VkImageView view_pass_a = image.targetView(format);
    VkImageView view_pass_b = image.targetView(format);

    VkImageObj image_3d(m_device);
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_3d.Init(image_ci);
    ASSERT_TRUE(image_3d.initialized());

    // If the sampler is unnormalizedCoordinates, the imageview type shouldn't be 3D, CUBE, 1D_ARRAY, 2D_ARRAY, CUBE_ARRAY.
    // This causes DesiredFailure.
    VkImageView view_fail = image_3d.targetView(format, VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0,
                                                VK_REMAINING_ARRAY_LAYERS, VK_IMAGE_VIEW_TYPE_3D);

    // Need 2 samplers (and ImageView) because testing both VUID and it will tie both errors to the same sampler/imageView, but only
    // 08610 will be triggered since it's first in the validation code
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.unnormalizedCoordinates = VK_TRUE;
    sampler_ci.maxLod = 0;
    vkt::Sampler sampler_a(*m_device, sampler_ci);
    vkt::Sampler sampler_b(*m_device, sampler_ci);

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, VK_NULL_HANDLE, sampler_a.handle(), VK_DESCRIPTOR_TYPE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler_b.handle(), VK_DESCRIPTOR_TYPE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(2, view_pass_a, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(3, view_pass_b, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(4, view_fail, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(4, view_fail, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-08609");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-08610");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-08611");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeSampler, UnnormalizedCoordinatesSeparateSamplerSharedImage) {
    TEST_DESCRIPTION("Doesn't use COMBINED_IMAGE_SAMPLER, but multiple OpLoad share Image OpVariable");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkShaderObj vs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_VERTEX_BIT);

    // There are 2 OpLoad/OpAccessChain that point the same OpVariable
    const char fsSource[] = R"glsl(
        #version 450
        // VK_DESCRIPTOR_TYPE_SAMPLER
        layout(set = 0, binding = 0) uniform sampler s_good; // unnormalized
        layout(set = 0, binding = 1) uniform sampler s_bad; // unnormalized
        // VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
        layout(set = 0, binding = 2) uniform texture2D si_good;

        void main() {
            vec4 x = texture(sampler2D(si_good, s_good), vec2(0));
            vec4 y = texture(sampler2D(si_good, s_bad), vec2(0));
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.InitState();
    ASSERT_EQ(VK_SUCCESS, g_pipe.CreateGraphicsPipeline());

    const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image.Init(image_ci);
    ASSERT_TRUE(image.initialized());
    VkImageView image_view = image.targetView(format);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.unnormalizedCoordinates = VK_FALSE;
    sampler_ci.maxLod = 0;
    vkt::Sampler sampler_good(*m_device, sampler_ci);
    sampler_ci.unnormalizedCoordinates = VK_TRUE;
    vkt::Sampler sampler_bad(*m_device, sampler_ci);

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, VK_NULL_HANDLE, sampler_good.handle(), VK_DESCRIPTOR_TYPE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler_bad.handle(), VK_DESCRIPTOR_TYPE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(2, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08610");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeSampler, UnnormalizedCoordinatesSeparateSamplerSharedSampler) {
    TEST_DESCRIPTION("Doesn't use COMBINED_IMAGE_SAMPLER, but multiple OpLoad share Sampler OpVariable");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    InitRenderTarget();

    VkShaderObj vs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_VERTEX_BIT);

    // There are 2 OpLoad/OpAccessChain that point the same OpVariable
    const char fsSource[] = R"glsl(
        #version 450
        // VK_DESCRIPTOR_TYPE_SAMPLER
        layout(set = 0, binding = 0) uniform sampler s1;
        // VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
        layout(set = 0, binding = 1) uniform texture2D si_good;
        layout(set = 0, binding = 2) uniform texture3D si_bad[2]; // 3D image view

        void main() {
            vec4 x = texture(sampler2D(si_good, s1), vec2(0));
            vec4 y = texture(sampler3D(si_bad[1], s1), vec3(0));
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.InitState();
    ASSERT_EQ(VK_SUCCESS, g_pipe.CreateGraphicsPipeline());

    const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image.Init(image_ci);
    ASSERT_TRUE(image.initialized());
    VkImageView image_view = image.targetView(format);

    VkImageObj image_3d(m_device);
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_3d.Init(image_ci);
    ASSERT_TRUE(image_3d.initialized());
    VkImageView image_view_3d = image_3d.targetView(format, VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0,
                                                    VK_REMAINING_ARRAY_LAYERS, VK_IMAGE_VIEW_TYPE_3D);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.unnormalizedCoordinates = VK_TRUE;
    sampler_ci.maxLod = 0;
    vkt::Sampler sampler(*m_device, sampler_ci);

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, VK_NULL_HANDLE, sampler.handle(), VK_DESCRIPTOR_TYPE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(1, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(2, image_view_3d, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(2, image_view_3d, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08609");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-08610");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeSampler, UnnormalizedCoordinatesInBoundsAccess) {
    TEST_DESCRIPTION("If a samper is unnormalizedCoordinates, but using OpInBoundsAccessChain");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkShaderObj vs(this, kMinimalShaderGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    // layout (set = 0, binding = 0) uniform sampler2D tex[2];
    // void main() {
    //     vec4 x = textureLodOffset(tex[1], vec2(0), 0, ivec2(0));
    // }
    //
    // but with OpInBoundsAccessChain instead of normal generated OpAccessChain
    const char *fsSource = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpDecorate %tex DescriptorSet 0
               OpDecorate %tex Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%ptr_v4float = OpTypePointer Function %v4float
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
       %array = OpTypeArray %11 %uint_2
%ptr_uc_array = OpTypePointer UniformConstant %array
        %tex = OpVariable %ptr_uc_array UniformConstant
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
     %ptr_uc = OpTypePointer UniformConstant %11
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %24 = OpConstantComposite %v2float %float_0 %float_0
      %v2int = OpTypeVector %int 2
      %int_0 = OpConstant %int 0
         %27 = OpConstantComposite %v2int %int_0 %int_0
       %main = OpFunction %void None %3
          %5 = OpLabel
          %x = OpVariable %ptr_v4float Function
         %20 = OpInBoundsAccessChain %ptr_uc %tex %int_1
         %21 = OpLoad %11 %20
         %28 = OpImageSampleExplicitLod %v4float %21 %24 Lod|ConstOffset %float_0 %27
               OpStore %x %28
               OpReturn
               OpFunctionEnd
    )";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.InitState();
    ASSERT_EQ(VK_SUCCESS, g_pipe.CreateGraphicsPipeline());

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image.Init(image_ci);
    ASSERT_TRUE(image.initialized());
    VkImageView view_pass = image.targetView(format);

    VkImageObj image_3d(m_device);
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_3d.Init(image_ci);
    ASSERT_TRUE(image_3d.initialized());

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.unnormalizedCoordinates = VK_TRUE;
    sampler_ci.maxLod = 0;
    vkt::Sampler sampler(*m_device, sampler_ci);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, view_pass, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, view_pass, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-08611");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeSampler, ReductionModeFeature) {
    TEST_DESCRIPTION("Test using VkSamplerReductionModeCreateInfo without required feature.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(Init())

    VkSamplerReductionModeCreateInfo sampler_reduction_mode_ci = vku::InitStructHelper();
    sampler_reduction_mode_ci.reductionMode = VK_SAMPLER_REDUCTION_MODE_MIN;

    auto sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.pNext = &sampler_reduction_mode_ci;
    CreateSamplerTest(*this, &sampler_ci, "VUID-VkSamplerCreateInfo-pNext-06726");
}

// Not possible to hit 07911 without first hitting an early return in parameter validation.
TEST_F(NegativeSampler, DISABLED_ReductionMode) {
    TEST_DESCRIPTION("Create sampler with invalid combination of filter and reduction mode.");
    AddRequiredExtensions(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    VkSamplerReductionModeCreateInfo sampler_reduction_mode_ci = vku::InitStructHelper();
    sampler_reduction_mode_ci.reductionMode = VK_SAMPLER_REDUCTION_MODE_MAX;
    VkSamplerCreateInfo sampler_ci = vku::InitStructHelper(&sampler_reduction_mode_ci);
    sampler_ci.magFilter = VK_FILTER_CUBIC_EXT;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCreateInfo-magFilter-parameter");
    CreateSamplerTest(*this, &sampler_ci, "VUID-VkSamplerCreateInfo-magFilter-07911");
}

TEST_F(NegativeSampler, NonSeamlessCubeMapNotEnabled) {
    TEST_DESCRIPTION("Validation should catch using NON_SEAMLESS_CUBE_MAP if the feature is not enabled.");

    AddRequiredExtensions(VK_EXT_NON_SEAMLESS_CUBE_MAP_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT non_seamless_cube_map_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(non_seamless_cube_map_features);
    non_seamless_cube_map_features.nonSeamlessCubeMap = false;
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.flags = VK_SAMPLER_CREATE_NON_SEAMLESS_CUBE_MAP_BIT_EXT;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-nonSeamlessCubeMap-06788");
}

TEST_F(NegativeSampler, BorderColorSwizzle) {
    TEST_DESCRIPTION("Validate vkCreateSampler with VkSamplerBorderColorComponentMappingCreateInfoEXT");

    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkSamplerBorderColorComponentMappingCreateInfoEXT border_color_component_mapping =
        vku::InitStructHelper();
    border_color_component_mapping.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                                 VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};

    VkSamplerCreateInfo sampler_create_info = SafeSaneSamplerCreateInfo();
    sampler_create_info.pNext = &border_color_component_mapping;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkSamplerBorderColorComponentMappingCreateInfoEXT-borderColorSwizzle-06437");
    vkt::Sampler sampler(*m_device, sampler_create_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSampler, BorderColorValue) {
    TEST_DESCRIPTION("Using a bad VkBorderColor value.");
    RETURN_IF_SKIP(Init())
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sampler_info.borderColor = static_cast<VkBorderColor>(0xFFFFBAD0);
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-addressModeU-01078");
}

TEST_F(NegativeSampler, CompareOpValue) {
    TEST_DESCRIPTION("Using a bad VkCompareOp value.");
    RETURN_IF_SKIP(Init())
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.compareEnable = VK_TRUE;
    sampler_info.compareOp = static_cast<VkCompareOp>(0xFFFFBAD0);
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-compareEnable-01080");
}

TEST_F(NegativeSampler, CustomBorderColorsFeature) {
    TEST_DESCRIPTION("Don't turn on the customBorderColors feature");
    AddRequiredExtensions(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkSampler sampler = VK_NULL_HANDLE;
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_CUSTOM_EXT;

    VkSamplerCustomBorderColorCreateInfoEXT custom_color_cinfo = vku::InitStructHelper();
    custom_color_cinfo.format = VK_FORMAT_R32_SFLOAT;
    sampler_info.pNext = &custom_color_cinfo;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCreateInfo-customBorderColors-04085");
    vk::CreateSampler(m_device->device(), &sampler_info, NULL, &sampler);
    m_errorMonitor->VerifyFound();
}
