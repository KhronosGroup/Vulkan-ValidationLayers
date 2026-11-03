/*
 * Copyright (c) 2026 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "video_objects.h"

class NegativeVideoEncodeRgbConversion : public VkVideoLayerTest {
  public:
    void InitRgbConversion(bool enable_feature = true) {
        AddRequiredExtensions(VK_VALVE_VIDEO_ENCODE_RGB_CONVERSION_EXTENSION_NAME);
        if (enable_feature) {
            AddRequiredFeature(vkt::Feature::videoEncodeRgbConversion);
        } else {
            ForceDisableFeature(vkt::Feature::videoEncodeRgbConversion);
        }
        RETURN_IF_SKIP(Init());
    }

    static VkVideoSessionCreateInfoKHR GetSessionCreateInfo(const VideoConfig& config) {
        VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
        create_info.pVideoProfile = config.Profile();
        create_info.pStdHeaderVersion = config.StdVersion();
        return create_info;
    }

    // Returns the first of the candidate bits that is not set in mask, or 0 if all of them are supported
    template <typename FlagBits>
    static FlagBits FirstUnsupported(uint32_t mask, std::initializer_list<FlagBits> candidates) {
        for (FlagBits bit : candidates) {
            if ((mask & bit) == 0) return bit;
        }
        return static_cast<FlagBits>(0);
    }
};

TEST_F(NegativeVideoEncodeRgbConversion, FeatureNotEnabled) {
    TEST_DESCRIPTION("Using a profile with VkVideoEncodeProfileRgbConversionInfoVALVE requires videoEncodeRgbConversion");

    RETURN_IF_SKIP(InitRgbConversion(false));

    VideoConfig config = GetConfigEncodeRgbConversion();
    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile that supports RGB conversion";
    }

    VkVideoProfileListInfoKHR video_profiles = vku::InitStructHelper();
    video_profiles.profileCount = 1;
    video_profiles.pProfiles = config.Profile();

    {
        auto encode_feedback_info = vku::InitStruct<VkQueryPoolVideoEncodeFeedbackCreateInfoKHR>(config.Profile());
        encode_feedback_info.encodeFeedbackFlags = VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BYTES_WRITTEN_BIT_KHR;

        VkQueryPool query_pool = VK_NULL_HANDLE;
        auto create_info = vku::InitStruct<VkQueryPoolCreateInfo>(&encode_feedback_info);
        create_info.queryType = VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR;
        create_info.queryCount = 1;

        // The registry does not list VkQueryPoolCreateInfo in structextends of VkVideoEncodeProfileRgbConversionInfoVALVE
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkQueryPoolCreateInfo-pNext-pNext");
        m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-pNext-10918");
        vk::CreateQueryPool(device(), &create_info, nullptr, &query_pool);
        m_errorMonitor->VerifyFound();
    }

    {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkBufferCreateInfo buffer_ci = vku::InitStructHelper(&video_profiles);
        buffer_ci.usage = VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR;
        buffer_ci.size = 2048;

        m_errorMonitor->SetDesiredError("VUID-VkBufferCreateInfo-pNext-10919");
        vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
        m_errorMonitor->VerifyFound();
    }

    {
        const VkVideoFormatPropertiesKHR& format_props = *config.PictureFormatProps();
        VkImage image = VK_NULL_HANDLE;
        VkImageCreateInfo image_ci = vku::InitStructHelper(&video_profiles);
        image_ci.flags = format_props.imageCreateFlags;
        image_ci.imageType = format_props.imageType;
        image_ci.format = format_props.format;
        image_ci.extent = {config.MaxCodedExtent().width, config.MaxCodedExtent().height, 1};
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.tiling = format_props.imageTiling;
        image_ci.usage = VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-pNext-10920");
        vk::CreateImage(device(), &image_ci, nullptr, &image);
        m_errorMonitor->VerifyFound();
    }

    {
        VkVideoSessionKHR session = VK_NULL_HANDLE;
        m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-10923");
        VkVideoSessionCreateInfoKHR create_info = GetSessionCreateInfo(config);
        vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeVideoEncodeRgbConversion, CreateSessionMissingRgbConversionInfo) {
    TEST_DESCRIPTION(
        "vkCreateVideoSessionKHR - performEncodeRgbConversion requires VkVideoEncodeSessionRgbConversionCreateInfoVALVE");

    RETURN_IF_SKIP(InitRgbConversion());

    VideoConfig config = GetConfigEncodeRgbConversion();
    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile that supports RGB conversion";
    }

    VkVideoSessionCreateInfoKHR create_info = GetSessionCreateInfo(config);
    create_info.pNext = nullptr;

    VkVideoSessionKHR session = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pNext-10924");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeRgbConversion, CreateSessionUnsupportedRgbConversion) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - RGB conversion parameters must be supported by the video profile");

    RETURN_IF_SKIP(InitRgbConversion());

    // Real implementations may support every value of a given field, so only test the ones with an unsupported value
    const std::initializer_list<VkVideoEncodeRgbChromaOffsetFlagBitsVALVE> chroma_offsets = {
        VK_VIDEO_ENCODE_RGB_CHROMA_OFFSET_COSITED_EVEN_BIT_VALVE, VK_VIDEO_ENCODE_RGB_CHROMA_OFFSET_MIDPOINT_BIT_VALVE};

    bool tested = false;
    for (const auto& config : GetConfigsEncodeRgbConversion()) {
        VideoConfig rgb_config = config;
        const auto* caps = rgb_config.EncodeRgbConversionCaps();
        VkVideoEncodeSessionRgbConversionCreateInfoVALVE* rgb_info = rgb_config.EncodeRgbConversionSessionInfo();
        VkVideoSessionKHR session = VK_NULL_HANDLE;
        const VkVideoSessionCreateInfoKHR create_info = GetSessionCreateInfo(rgb_config);

        const auto unsupported_model = FirstUnsupported<VkVideoEncodeRgbModelConversionFlagBitsVALVE>(
            caps->rgbModels,
            {VK_VIDEO_ENCODE_RGB_MODEL_CONVERSION_RGB_IDENTITY_BIT_VALVE,
             VK_VIDEO_ENCODE_RGB_MODEL_CONVERSION_YCBCR_IDENTITY_BIT_VALVE,
             VK_VIDEO_ENCODE_RGB_MODEL_CONVERSION_YCBCR_709_BIT_VALVE, VK_VIDEO_ENCODE_RGB_MODEL_CONVERSION_YCBCR_601_BIT_VALVE,
             VK_VIDEO_ENCODE_RGB_MODEL_CONVERSION_YCBCR_2020_BIT_VALVE});
        if (unsupported_model) {
            const auto supported = rgb_info->rgbModel;
            rgb_info->rgbModel = unsupported_model;
            m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeSessionRgbConversionCreateInfoVALVE-rgbModel-10930");
            vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
            m_errorMonitor->VerifyFound();
            rgb_info->rgbModel = supported;
            tested = true;
        }

        const auto unsupported_range = FirstUnsupported<VkVideoEncodeRgbRangeCompressionFlagBitsVALVE>(
            caps->rgbRanges, {VK_VIDEO_ENCODE_RGB_RANGE_COMPRESSION_FULL_RANGE_BIT_VALVE,
                              VK_VIDEO_ENCODE_RGB_RANGE_COMPRESSION_NARROW_RANGE_BIT_VALVE});
        if (unsupported_range) {
            const auto supported = rgb_info->rgbRange;
            rgb_info->rgbRange = unsupported_range;
            m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeSessionRgbConversionCreateInfoVALVE-rgbRange-10931");
            vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
            m_errorMonitor->VerifyFound();
            rgb_info->rgbRange = supported;
            tested = true;
        }

        const auto unsupported_x_offset = FirstUnsupported(caps->xChromaOffsets, chroma_offsets);
        if (unsupported_x_offset) {
            const auto supported = rgb_info->xChromaOffset;
            rgb_info->xChromaOffset = unsupported_x_offset;
            m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeSessionRgbConversionCreateInfoVALVE-xChromaOffset-10932");
            vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
            m_errorMonitor->VerifyFound();
            rgb_info->xChromaOffset = supported;
            tested = true;
        }

        const auto unsupported_y_offset = FirstUnsupported(caps->yChromaOffsets, chroma_offsets);
        if (unsupported_y_offset) {
            const auto supported = rgb_info->yChromaOffset;
            rgb_info->yChromaOffset = unsupported_y_offset;
            m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeSessionRgbConversionCreateInfoVALVE-yChromaOffset-10933");
            vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
            m_errorMonitor->VerifyFound();
            rgb_info->yChromaOffset = supported;
            tested = true;
        }

        if (tested) break;
    }

    if (!tested) {
        GTEST_SKIP() << "Test requires a video encode profile with an unsupported RGB conversion value";
    }
}
