/*
 * Copyright (c) 2022-2024 The Khronos Group Inc.
 * Copyright (c) 2022-2024 RasterGrid Kft.
 * Modifications Copyright (C) 2024 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/video_objects.h"
#include "generated/enum_flag_bits.h"
#include "utils/vk_layer_utils.h"

TEST_F(NegativeVideo, VideoCodingScope) {
    TEST_DESCRIPTION("Tests calling functions inside/outside video coding scope");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    // Video coding block must be ended before command buffer
    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkEndCommandBuffer-None-06991");
    vk::EndCommandBuffer(cb.handle());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();

    // vkCmdEndVideoCoding not allowed outside video coding block
    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEndVideoCodingKHR-videocoding");
    cb.EndVideoCoding(context.End());
    m_errorMonitor->VerifyFound();

    cb.End();

    // vkCmdBeginVideoCoding not allowed inside video coding block
    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-videocoding");
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();

    // vkCmdControlVideoCoding not allowed outside video coding block
    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdControlVideoCodingKHR-videocoding");
    cb.ControlVideoCoding(context.Control().Reset());
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, VideoProfileInvalidLumaChromaSubsampling) {
    TEST_DESCRIPTION("Test single bit set in VkVideoProfileInfoKHR chromaSubsampling, lumaBitDepth, and chromaBitDepth");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VkVideoProfileInfoKHR profile;

    // Multiple bits in chromaSubsampling
    m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-chromaSubsampling-07013");
    profile = *config.Profile();
    profile.chromaSubsampling = VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR | VK_VIDEO_CHROMA_SUBSAMPLING_422_BIT_KHR;
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), &profile, config.Caps());
    m_errorMonitor->VerifyFound();

    // Multiple bits in lumaBitDepth
    m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-lumaBitDepth-07014");
    profile = *config.Profile();
    profile.lumaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR | VK_VIDEO_COMPONENT_BIT_DEPTH_10_BIT_KHR;
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), &profile, config.Caps());
    m_errorMonitor->VerifyFound();

    // Multiple bits in chromaBitDepth
    m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-chromaSubsampling-07015");
    profile = *config.Profile();
    profile.chromaSubsampling = VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR;
    profile.chromaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR | VK_VIDEO_COMPONENT_BIT_DEPTH_10_BIT_KHR;
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), &profile, config.Caps());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, VideoProfileMissingCodecInfo) {
    TEST_DESCRIPTION("Test missing codec-specific structure in profile definition");

    RETURN_IF_SKIP(Init());

    VkVideoProfileInfoKHR profile;

    if (GetConfigDecodeH264()) {
        VideoConfig config = GetConfigDecodeH264();

        profile = *config.Profile();
        profile.pNext = nullptr;

        // Missing codec-specific info for H.264 decode profile
        m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07179");
        vk::GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), &profile, config.Caps());
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigDecodeH265()) {
        VideoConfig config = GetConfigDecodeH265();

        profile = *config.Profile();
        profile.pNext = nullptr;

        // Missing codec-specific info for H.265 decode profile
        m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07180");
        vk::GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), &profile, config.Caps());
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigDecodeAV1()) {
        VideoConfig config = GetConfigDecodeAV1();

        profile = *config.Profile();
        profile.pNext = nullptr;

        // Missing codec-specific info for AV1 decode profile
        m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-09256");
        vk::GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), &profile, config.Caps());
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigEncodeH264()) {
        VideoConfig config = GetConfigEncodeH264();

        profile = *config.Profile();
        profile.pNext = nullptr;

        // Missing codec-specific info for H.264 encode profile
        m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07181");
        vk::GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), &profile, config.Caps());
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigEncodeH265()) {
        VideoConfig config = GetConfigEncodeH265();

        profile = *config.Profile();
        profile.pNext = nullptr;

        // Missing codec-specific info for H.265 encode profile
        m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07182");
        vk::GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), &profile, config.Caps());
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigEncodeAV1()) {
        VideoConfig config = GetConfigEncodeAV1();

        profile = *config.Profile();
        profile.pNext = nullptr;

        // Missing codec-specific info for AV1 encode profile
        m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-10262");
        vk::GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), &profile, config.Caps());
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeVideo, CapabilityQueryMissingChain) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoCapabilitiesKHR - missing return structures in chain");

    RETURN_IF_SKIP(Init());

    if (GetConfigDecodeH264()) {
        VideoConfig config = GetConfigDecodeH264();

        auto caps = vku::InitStruct<VkVideoCapabilitiesKHR>();
        auto decode_caps = vku::InitStruct<VkVideoDecodeCapabilitiesKHR>();
        auto decode_h264_caps = vku::InitStruct<VkVideoDecodeH264CapabilitiesKHR>();

        // Missing decode caps struct for decode profile
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07183");
        caps.pNext = &decode_h264_caps;
        vk::GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
        m_errorMonitor->VerifyFound();

        // Missing H.264 decode caps struct for H.264 decode profile
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07184");
        caps.pNext = &decode_caps;
        vk::GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigDecodeH265()) {
        VideoConfig config = GetConfigDecodeH265();

        auto caps = vku::InitStruct<VkVideoCapabilitiesKHR>();
        auto decode_caps = vku::InitStruct<VkVideoDecodeCapabilitiesKHR>();
        auto decode_h265_caps = vku::InitStruct<VkVideoDecodeH265CapabilitiesKHR>();

        // Missing decode caps struct for decode profile
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07183");
        caps.pNext = &decode_h265_caps;
        vk::GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
        m_errorMonitor->VerifyFound();

        // Missing H.265 decode caps struct for H.265 decode profile
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07185");
        caps.pNext = &decode_caps;
        vk::GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigDecodeAV1()) {
        VideoConfig config = GetConfigDecodeAV1();

        auto caps = vku::InitStruct<VkVideoCapabilitiesKHR>();
        auto decode_caps = vku::InitStruct<VkVideoDecodeCapabilitiesKHR>();
        auto decode_av1_caps = vku::InitStruct<VkVideoDecodeAV1CapabilitiesKHR>();

        // Missing decode caps struct for decode profile
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07183");
        caps.pNext = &decode_av1_caps;
        vk::GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
        m_errorMonitor->VerifyFound();

        // Missing AV1 decode caps struct for AV1 decode profile
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-09257");
        caps.pNext = &decode_caps;
        vk::GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigEncodeH264()) {
        VideoConfig config = GetConfigEncodeH264();

        auto caps = vku::InitStruct<VkVideoCapabilitiesKHR>();
        auto encode_caps = vku::InitStruct<VkVideoEncodeCapabilitiesKHR>();
        auto encode_h264_caps = vku::InitStruct<VkVideoEncodeH264CapabilitiesKHR>();

        // Missing encode caps struct for encode profile
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07186");
        caps.pNext = &encode_h264_caps;
        vk::GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
        m_errorMonitor->VerifyFound();

        // Missing H.264 encode caps struct for H.264 encode profile
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07187");
        caps.pNext = &encode_caps;
        vk::GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigEncodeH265()) {
        VideoConfig config = GetConfigEncodeH265();

        auto caps = vku::InitStruct<VkVideoCapabilitiesKHR>();
        auto encode_caps = vku::InitStruct<VkVideoEncodeCapabilitiesKHR>();
        auto encode_h265_caps = vku::InitStruct<VkVideoEncodeH265CapabilitiesKHR>();

        // Missing encode caps struct for encode profile
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07186");
        caps.pNext = &encode_h265_caps;
        vk::GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
        m_errorMonitor->VerifyFound();

        // Missing H.265 encode caps struct for H.265 encode profile
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07188");
        caps.pNext = &encode_caps;
        vk::GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigEncodeAV1()) {
        VideoConfig config = GetConfigEncodeAV1();

        auto caps = vku::InitStruct<VkVideoCapabilitiesKHR>();
        auto encode_caps = vku::InitStruct<VkVideoEncodeCapabilitiesKHR>();
        auto encode_av1_caps = vku::InitStruct<VkVideoEncodeAV1CapabilitiesKHR>();

        // Missing encode caps struct for encode profile
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07186");
        caps.pNext = &encode_av1_caps;
        vk::GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
        m_errorMonitor->VerifyFound();

        // Missing AV1 encode caps struct for AV1 encode profile
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-10263");
        caps.pNext = &encode_caps;
        vk::GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeVideo, VideoFormatQueryMissingProfile) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoFormatPropertiesKHR - missing profile info");

    RETURN_IF_SKIP(Init());

    if (!GetConfig()) {
        GTEST_SKIP() << "Test requires video support";
    }

    auto format_info = vku::InitStruct<VkPhysicalDeviceVideoFormatInfoKHR>();
    format_info.imageUsage = VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR;
    uint32_t format_count = 0;

    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoFormatPropertiesKHR-pNext-06812");
    vk::GetPhysicalDeviceVideoFormatPropertiesKHR(gpu(), &format_info, &format_count, nullptr);
    m_errorMonitor->VerifyFound();

    auto profile_list = vku::InitStruct<VkVideoProfileListInfoKHR>();
    format_info.pNext = &profile_list;

    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoFormatPropertiesKHR-pNext-06812");
    vk::GetPhysicalDeviceVideoFormatPropertiesKHR(gpu(), &format_info, &format_count, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, EncodeQualityLevelPropsUnsupportedProfile) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR - unsupported profile");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigEncodeInvalid();
    if (!config) {
        GTEST_SKIP() << "Test requires encode support";
    }

    auto quality_level_info = vku::InitStruct<VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR>();
    quality_level_info.pVideoProfile = config.Profile();

    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR-pVideoProfile-08259");
    vk::GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(gpu(), &quality_level_info, config.EncodeQualityLevelProps());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, EncodeQualityLevelPropsProfileNotEncode) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR - profile is not an encode profile");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigDecode();
    if (!config || !GetConfigEncode()) {
        GTEST_SKIP() << "Test requires decode and encode support";
    }

    auto quality_level_info = vku::InitStruct<VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR>();
    quality_level_info.pVideoProfile = config.Profile();

    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR-pVideoProfile-08260");
    vk::GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(gpu(), &quality_level_info, config.EncodeQualityLevelProps());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, EncodeQualityLevelPropsInvalidQualityLevel) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR - invalid quality level");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires encode support";
    }

    auto quality_level_info = vku::InitStruct<VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR>();
    quality_level_info.pVideoProfile = config.Profile();
    quality_level_info.qualityLevel = config.EncodeCaps()->maxQualityLevels;

    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR-qualityLevel-08261");
    vk::GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(gpu(), &quality_level_info, config.EncodeQualityLevelProps());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, EncodeQualityLevelPropsMissingCodecInfo) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR - missing codec-specific profile info");

    RETURN_IF_SKIP(Init());

    VkVideoProfileInfoKHR profile;
    auto quality_level_info = vku::InitStruct<VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR>();
    quality_level_info.pVideoProfile = &profile;

    if (GetConfigEncodeH264()) {
        VideoConfig config = GetConfigEncodeH264();

        profile = *config.Profile();
        profile.pNext = nullptr;

        // Missing codec-specific info for H.264 encode profile
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR-pVideoProfile-08259");
        m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07181");
        vk::GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(gpu(), &quality_level_info, config.EncodeQualityLevelProps());
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigEncodeH265()) {
        VideoConfig config = GetConfigEncodeH265();

        profile = *config.Profile();
        profile.pNext = nullptr;

        // Missing codec-specific info for H.265 encode profile
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR-pVideoProfile-08259");
        m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07182");
        vk::GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(gpu(), &quality_level_info, config.EncodeQualityLevelProps());
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigEncodeAV1()) {
        VideoConfig config = GetConfigEncodeAV1();

        profile = *config.Profile();
        profile.pNext = nullptr;

        // Missing codec-specific info for AV1 encode profile
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR-pVideoProfile-08259");
        m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-10262");
        vk::GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(gpu(), &quality_level_info, config.EncodeQualityLevelProps());
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeVideo, EncodeQualityLevelPropsMissingChain) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR - missing return structures in chain");

    RETURN_IF_SKIP(Init());

    auto quality_level_info = vku::InitStruct<VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR>();
    auto quality_level_props = vku::InitStruct<VkVideoEncodeQualityLevelPropertiesKHR>();

    if (GetConfigEncodeH264()) {
        VideoConfig config = GetConfigEncodeH264();
        quality_level_info.pVideoProfile = config.Profile();

        // Missing codec-specific output structure for H.264 encode profile
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR-pQualityLevelInfo-08257");
        vk::GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(gpu(), &quality_level_info, &quality_level_props);
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigEncodeH265()) {
        VideoConfig config = GetConfigEncodeH265();
        quality_level_info.pVideoProfile = config.Profile();

        // Missing codec-specific output structure for H.265 encode profile
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR-pQualityLevelInfo-08258");
        vk::GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(gpu(), &quality_level_info, &quality_level_props);
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigEncodeAV1()) {
        VideoConfig config = GetConfigEncodeAV1();
        quality_level_info.pVideoProfile = config.Profile();

        // Missing codec-specific output structure for AV1 encode profile
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR-pQualityLevelInfo-10305");
        vk::GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(gpu(), &quality_level_info, &quality_level_props);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeVideo, InUseDestroyed) {
    TEST_DESCRIPTION("Test destroying objects while they are still in use");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigWithParams(GetConfigs());
    if (!config) {
        config = GetConfig();
    }
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset());
    cb.EndVideoCoding(context.End());
    cb.End();

    context.Queue().Submit(cb);

    m_errorMonitor->SetDesiredError("VUID-vkDestroyVideoSessionKHR-videoSession-07192");
    vk::DestroyVideoSessionKHR(device(), context.Session(), nullptr);
    m_errorMonitor->VerifyFound();

    if (config.NeedsSessionParams()) {
        m_errorMonitor->SetDesiredError("VUID-vkDestroyVideoSessionParametersKHR-videoSessionParameters-07212");
        vk::DestroyVideoSessionParametersKHR(device(), context.SessionParams(), nullptr);
        m_errorMonitor->VerifyFound();
    }

    m_device->Wait();
}

TEST_F(NegativeVideo, CreateSessionVideoQuantizationMapNotEnabled) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - quantization map flag is specified but videoEncodeQuantizationMap was not enabled");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    ForceDisableFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    for (VkVideoSessionCreateFlagBitsKHR flag : {VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                                 VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR}) {
        create_info.flags = flag;

        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoSessionCreateInfoKHR-flags-10267");
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoSessionCreateInfoKHR-flags-10268");
        m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-flags-10264");
        vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeVideo, CreateSessionQuantMapWithoutEncodeProfile) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - cannot use quantization map flags without encode profile");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    for (VkVideoSessionCreateFlagBitsKHR flag : {VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                                 VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR}) {
        create_info.flags = flag;

        m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-flags-10265");
        vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeVideo, CreateSessionQuantMapFlagsMutuallyExclusive) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - cannot allow both QUANTIZATION_DELTA and EMPHASIS maps at the same time");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR) &&
               (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR);
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with support for both QUANTIZATION_DELTA and EMPHASIS map";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();
    create_info.flags = VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR |
                        VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-flags-10266");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionQuantDeltaMapUnsupported) {
    TEST_DESCRIPTION(
        "vkCreateVideoSessionKHR - VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR "
        "requires VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires an encode profile without QUANTIZATION_DELTA_MAP support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.flags = VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR;
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-flags-10267");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionEmphasisMapUnsupported) {
    TEST_DESCRIPTION(
        "vkCreateVideoSessionKHR - VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR "
        "requires VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires an encode profile without EMPHASIS_MAP support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.flags = VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR;
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-flags-10268");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionVideoMaintenance1NotEnabled) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - cannot use inline queries without videoMaintenance1");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.flags = VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoSessionCreateInfoKHR-flags-parameter");
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-flags-08371");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionVideoEncodeAV1NotEnabled) {
    TEST_DESCRIPTION("vkCreateVideoSession - AV1 encode is specified but videoEncodeAV1 was not enabled");

    ForceDisableFeature(vkt::Feature::videoEncodeAV1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-10269");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionProtectedMemoryNotEnabled) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - cannot enable protected content without protected memory");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.flags = VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR;
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-protectedMemory-07189");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionProtectedContentUnsupported) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - cannot enable protected content if not supported");

    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigWithoutProtectedContent(GetConfigs());
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with no protected content support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.flags = VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR;
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-protectedMemory-07189");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionUnsupportedProfile) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - unsupported profile");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigInvalid();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    auto pfn_CreateVideoSessionKHR =
        (PFN_vkCreateVideoSessionKHR)vk::GetDeviceProcAddr(m_device->handle(), "vkCreateVideoSessionKHR");
    ASSERT_NE(pfn_CreateVideoSessionKHR, nullptr);

    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    VkVideoSessionKHR session;
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-04845");
    pfn_CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionInvalidReferencePictureCounts) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid reference picture slot and active counts");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();
    create_info.maxDpbSlots = config.Caps()->maxDpbSlots;
    create_info.maxActiveReferencePictures = config.Caps()->maxActiveReferencePictures;

    // maxDpbSlots too big
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxDpbSlots-04847");
    create_info.maxDpbSlots++;
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    create_info.maxDpbSlots--;
    m_errorMonitor->VerifyFound();

    // maxActiveReferencePictures too big
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxActiveReferencePictures-04849");
    create_info.maxActiveReferencePictures++;
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    create_info.maxActiveReferencePictures--;
    m_errorMonitor->VerifyFound();

    config = GetConfig(GetConfigsWithReferences(GetConfigs()));
    if (config) {
        // maxDpbSlots is 0, but maxActiveReferencePictures is not
        m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxDpbSlots-04850");
        create_info.maxDpbSlots = 0;
        create_info.maxActiveReferencePictures = 1;
        vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
        m_errorMonitor->VerifyFound();

        // maxActiveReferencePictures is 0, but maxDpbSlots is not
        m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxDpbSlots-04850");
        create_info.maxDpbSlots = 1;
        create_info.maxActiveReferencePictures = 0;
        vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeVideo, CreateSessionInvalidMaxCodedExtent) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - maxCodedExtent outside of supported range");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    // maxCodedExtent.width too small
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxCodedExtent-04851");
    create_info.maxCodedExtent = config.Caps()->minCodedExtent;
    --create_info.maxCodedExtent.width;
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();

    // maxCodedExtent.height too small
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxCodedExtent-04851");
    create_info.maxCodedExtent = config.Caps()->minCodedExtent;
    --create_info.maxCodedExtent.height;
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();

    // maxCodedExtent.width too big
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxCodedExtent-04851");
    create_info.maxCodedExtent = config.Caps()->maxCodedExtent;
    ++create_info.maxCodedExtent.width;
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();

    // maxCodedExtent.height too big
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxCodedExtent-04851");
    create_info.maxCodedExtent = config.Caps()->maxCodedExtent;
    ++create_info.maxCodedExtent.height;
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionInvalidDecodeReferencePictureFormat) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid decode reference picture format");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-referencePictureFormat-04852");
    create_info.referencePictureFormat = VK_FORMAT_D16_UNORM;
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionInvalidEncodeReferencePictureFormat) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid encode reference picture format");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-referencePictureFormat-06814");
    create_info.referencePictureFormat = VK_FORMAT_D16_UNORM;
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionInvalidDecodePictureFormat) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid decode picture format");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pictureFormat-04853");
    create_info.pictureFormat = VK_FORMAT_D16_UNORM;
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionInvalidEncodePictureFormat) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid encode picture format");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pictureFormat-04854");
    create_info.pictureFormat = VK_FORMAT_D16_UNORM;
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionInvalidStdHeaderVersion) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid Video Std header version");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    VkExtensionProperties std_version = *config.StdVersion();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = &std_version;

    // Video Std header version not supported
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pStdHeaderVersion-07191");
    ++std_version.specVersion;
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    --std_version.specVersion;
    m_errorMonitor->VerifyFound();

    // Video Std header name not supported
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pStdHeaderVersion-07190");
    strcpy(std_version.extensionName, "invalid_std_header_name");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionEncodeH264InvalidMaxLevel) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid H.264 maxLevelIdc for encode session");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    auto h264_create_info = vku::InitStruct<VkVideoEncodeH264SessionCreateInfoKHR>();
    h264_create_info.pNext = create_info.pNext;
    create_info.pNext = &h264_create_info;

    auto unsupported_level = static_cast<int32_t>(config.EncodeCapsH264()->maxLevelIdc) + 1;
    h264_create_info.maxLevelIdc = static_cast<StdVideoH264LevelIdc>(unsupported_level);

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-08251");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionEncodeH265InvalidMaxLevel) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid H.265 maxLevelIdc for encode session");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    auto h265_create_info = vku::InitStruct<VkVideoEncodeH265SessionCreateInfoKHR>();
    h265_create_info.pNext = create_info.pNext;
    create_info.pNext = &h265_create_info;

    auto unsupported_level = static_cast<int32_t>(config.EncodeCapsH265()->maxLevelIdc) + 1;
    h265_create_info.maxLevelIdc = static_cast<StdVideoH265LevelIdc>(unsupported_level);

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-08252");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionEncodeAV1InvalidMaxLevel) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid AV1 maxLevel for encode session");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    auto av1_create_info = vku::InitStruct<VkVideoEncodeAV1SessionCreateInfoKHR>();
    av1_create_info.pNext = create_info.pNext;
    create_info.pNext = &av1_create_info;

    auto unsupported_level = static_cast<int32_t>(config.EncodeCapsAV1()->maxLevel) + 1;
    av1_create_info.maxLevel = static_cast<StdVideoAV1Level>(unsupported_level);

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-10270");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, BindVideoSessionMemory) {
    TEST_DESCRIPTION("vkBindVideoSessionMemoryKHR - memory binding related invalid usages");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(m_device, config);

    uint32_t mem_req_count;
    ASSERT_EQ(VK_SUCCESS, vk::GetVideoSessionMemoryRequirementsKHR(device(), context.Session(), &mem_req_count, nullptr));
    if (mem_req_count == 0) {
        GTEST_SKIP() << "Test can only run if video session needs memory bindings";
    }

    std::vector<VkVideoSessionMemoryRequirementsKHR> mem_reqs(mem_req_count, vku::InitStruct<VkVideoSessionMemoryRequirementsKHR>());
    ASSERT_EQ(VK_SUCCESS, vk::GetVideoSessionMemoryRequirementsKHR(device(), context.Session(), &mem_req_count, mem_reqs.data()));

    std::vector<VkDeviceMemory> session_memory;
    std::vector<VkBindVideoSessionMemoryInfoKHR> bind_info(mem_req_count, vku::InitStruct<VkBindVideoSessionMemoryInfoKHR>());
    for (uint32_t i = 0; i < mem_req_count; ++i) {
        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
        ASSERT_TRUE(m_device->phy().SetMemoryType(mem_reqs[i].memoryRequirements.memoryTypeBits, &alloc_info, 0));
        alloc_info.allocationSize = mem_reqs[i].memoryRequirements.size * 2;

        VkDeviceMemory memory = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &alloc_info, nullptr, &memory));
        session_memory.push_back(memory);

        bind_info[i].memoryBindIndex = mem_reqs[i].memoryBindIndex;
        bind_info[i].memory = memory;
        bind_info[i].memoryOffset = 0;
        bind_info[i].memorySize = mem_reqs[i].memoryRequirements.size;
    }

    // Duplicate memoryBindIndex
    if (mem_req_count > 1) {
        m_errorMonitor->SetDesiredError("VUID-vkBindVideoSessionMemoryKHR-memoryBindIndex-07196");
        auto& duplicate = bind_info[mem_req_count / 2];
        auto backup = duplicate;
        duplicate = bind_info[0];
        vk::BindVideoSessionMemoryKHR(device(), context.Session(), mem_req_count, bind_info.data());
        duplicate = backup;
        m_errorMonitor->VerifyFound();
    }

    // Invalid memoryBindIndex
    uint32_t invalid_bind_index = vvl::kU32Max;
    for (uint32_t i = 0; i < mem_req_count; ++i) {
        if (mem_reqs[i].memoryBindIndex < vvl::kU32Max) {
            invalid_bind_index = mem_reqs[i].memoryBindIndex + 1;
        }
    }
    if (invalid_bind_index != vvl::kU32Max) {
        m_errorMonitor->SetDesiredError("VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07197");
        auto& invalid = bind_info[mem_req_count / 2];
        auto backup = invalid;
        invalid.memoryBindIndex = invalid_bind_index;
        vk::BindVideoSessionMemoryKHR(device(), context.Session(), mem_req_count, bind_info.data());
        invalid = backup;
        m_errorMonitor->VerifyFound();
    }

    // Incompatible memory type
    uint32_t invalid_mem_type_index = vvl::kU32Max;
    uint32_t invalid_mem_type_req_index = vvl::kU32Max;
    auto mem_props = m_device->phy().memory_properties_;
    for (uint32_t i = 0; i < mem_req_count; ++i) {
        uint32_t mem_type_bits = mem_reqs[i].memoryRequirements.memoryTypeBits;
        for (uint32_t mem_type_index = 0; mem_type_index < mem_props.memoryTypeCount; ++mem_type_index) {
            if ((mem_type_bits & (1 << mem_type_index)) == 0) {
                invalid_mem_type_index = mem_type_index;
                invalid_mem_type_req_index = i;
                break;
            }
        }
        if (invalid_mem_type_index != vvl::kU32Max) break;
    }
    if (invalid_mem_type_index != vvl::kU32Max) {
        auto& mem_req = mem_reqs[invalid_mem_type_req_index].memoryRequirements;

        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
        alloc_info.memoryTypeIndex = invalid_mem_type_index;
        alloc_info.allocationSize = mem_req.size * 2;

        VkDeviceMemory memory = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &alloc_info, nullptr, &memory));

        m_errorMonitor->SetDesiredError("VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07198");
        auto& invalid = bind_info[invalid_mem_type_req_index];
        auto backup = invalid;
        invalid.memory = memory;
        vk::BindVideoSessionMemoryKHR(device(), context.Session(), mem_req_count, bind_info.data());
        invalid = backup;
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), memory, nullptr);
    }

    // Incorrectly aligned memoryOffset
    uint32_t invalid_offset_align_index = vvl::kU32Max;
    for (uint32_t i = 0; i < mem_req_count; ++i) {
        if (mem_reqs[i].memoryRequirements.alignment > 1) {
            invalid_offset_align_index = i;
            break;
        }
    }
    if (invalid_offset_align_index != vvl::kU32Max) {
        auto& mem_req = mem_reqs[invalid_offset_align_index].memoryRequirements;

        m_errorMonitor->SetDesiredError("VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07199");
        auto& invalid = bind_info[invalid_offset_align_index];
        auto backup = invalid;
        invalid.memoryOffset = mem_req.alignment / 2;
        vk::BindVideoSessionMemoryKHR(device(), context.Session(), mem_req_count, bind_info.data());
        invalid = backup;
        m_errorMonitor->VerifyFound();
    }

    // Incorrect memorySize
    {
        m_errorMonitor->SetDesiredError("VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07200");
        auto& invalid = bind_info[mem_req_count / 2];
        auto backup = invalid;
        invalid.memorySize += 16;
        vk::BindVideoSessionMemoryKHR(device(), context.Session(), mem_req_count, bind_info.data());
        invalid = backup;
        m_errorMonitor->VerifyFound();
    }

    // Out-of-bounds memoryOffset
    {
        m_errorMonitor->SetDesiredError("VUID-VkBindVideoSessionMemoryInfoKHR-memoryOffset-07201");
        auto& invalid = bind_info[mem_req_count / 2];
        auto backup = invalid;
        invalid.memoryOffset = invalid.memorySize * 2;
        vk::BindVideoSessionMemoryKHR(device(), context.Session(), mem_req_count, bind_info.data());
        invalid = backup;
        m_errorMonitor->VerifyFound();
    }

    // Out-of-bounds memoryOffset + memorySize
    {
        uint32_t index = mem_req_count / 2;

        m_errorMonitor->SetDesiredError("VUID-VkBindVideoSessionMemoryInfoKHR-memorySize-07202");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07199");
        auto& invalid = bind_info[index];
        auto backup = invalid;
        invalid.memoryOffset = invalid.memorySize + 1;
        vk::BindVideoSessionMemoryKHR(device(), context.Session(), mem_req_count, bind_info.data());
        invalid = backup;
        m_errorMonitor->VerifyFound();
    }

    // Already bound
    {
        uint32_t first_bind_count = mem_req_count / 2;
        if (first_bind_count == 0) {
            first_bind_count = 1;
        }

        vk::BindVideoSessionMemoryKHR(device(), context.Session(), first_bind_count, bind_info.data());

        m_errorMonitor->SetDesiredError("VUID-vkBindVideoSessionMemoryKHR-videoSession-07195");
        vk::BindVideoSessionMemoryKHR(device(), context.Session(), 1, &bind_info[first_bind_count - 1]);
        m_errorMonitor->VerifyFound();
    }

    for (auto memory : session_memory) {
        vk::FreeMemory(device(), memory, nullptr);
    }
}

TEST_F(NegativeVideo, CreateSessionParamsQuantMapIncompatSession) {
    TEST_DESCRIPTION(
        "vkCreateVideoSessionParametersKHR - QUANTIZATION_MAP_COMPATIBLE_BIT requires session allowing quantization maps");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    ASSERT_TRUE(config) << "Support for videoEncodeQuantizationMap implies at least one supported encode profile";

    VideoContext context(m_device, config);

    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.flags = VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR;
    create_info.videoSession = context.Session();

    VkVideoSessionParametersKHR params;
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-flags-10271");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionParamsQuantMapTexelSize) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - missing or invalid quantizationMapTexelSize");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    struct TestConfig {
        VkVideoSessionCreateFlagBitsKHR flag;
        VideoConfig config;
        const std::vector<vku::safe_VkVideoFormatPropertiesKHR>& map_props;
        const char* vuid;
    };

    std::vector<TestConfig> tests;
    if (delta_config) {
        tests.push_back({VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR, delta_config,
                         delta_config.SupportedQuantDeltaMapProps(), "VUID-VkVideoSessionParametersCreateInfoKHR-flags-10273"});
    }
    if (emphasis_config) {
        tests.push_back({VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR, emphasis_config,
                         emphasis_config.SupportedQuantDeltaMapProps(), "VUID-VkVideoSessionParametersCreateInfoKHR-flags-10274"});
    }

    for (const auto& test : tests) {
        VideoConfig config = test.config;

        config.SessionCreateInfo()->flags |= test.flag;
        VideoContext context(m_device, config);

        // VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR missing from pNext chain
        {
            VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
            create_info.flags = VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR;
            create_info.videoSession = context.Session();

            VkVideoSessionParametersKHR params;
            m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-flags-10272");
            vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
            m_errorMonitor->VerifyFound();
        }

        {
            // Find an invalid quantizaitonMapTexelSize by using the maximum width + 1
            VkExtent2D invalid_texel_size = {0, 0};
            for (const auto& map_props : test.map_props) {
                auto texel_size = config.GetQuantMapTexelSize(map_props);
                if (invalid_texel_size.width < texel_size.width) {
                    invalid_texel_size.width = texel_size.width + 1;
                    invalid_texel_size.height = texel_size.height;
                }
            }

            VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
            create_info.flags = VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR;
            create_info.videoSession = context.Session();

            VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR map_create_info = vku::InitStructHelper();
            map_create_info.quantizationMapTexelSize = invalid_texel_size;
            map_create_info.pNext = create_info.pNext;
            create_info.pNext = &map_create_info;

            VkVideoSessionParametersKHR params;
            m_errorMonitor->SetDesiredError(test.vuid);
            vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
            m_errorMonitor->VerifyFound();
        }
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideo, CreateSessionParamsIncompatibleTemplateQuantMap) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - template must match in quantization map compatibility");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    std::vector<std::tuple<VideoConfig, VkVideoSessionCreateFlagBitsKHR, const VkVideoFormatPropertiesKHR*>> tests = {};
    if (delta_config) {
        tests.emplace_back(delta_config, VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                           delta_config.QuantDeltaMapProps());
    }
    if (emphasis_config) {
        tests.emplace_back(emphasis_config, VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR,
                           emphasis_config.EmphasisMapProps());
    }

    for (auto& [config, flag, map_props] : tests) {
        const auto texel_size = config.GetQuantMapTexelSize(*map_props);

        config.SessionCreateInfo()->flags |= flag;
        VideoContext context(m_device, config);

        // Template is not QUANIZATION_MAP_COMPATIBLE but QUANIZATION_MAP_COMPATIBLE is requested
        {
            auto template_params = context.CreateSessionParams();

            VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR map_create_info = vku::InitStructHelper();
            map_create_info.quantizationMapTexelSize = texel_size;
            VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
            map_create_info.pNext = create_info.pNext;
            create_info.pNext = &map_create_info;
            create_info.flags = VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR;
            create_info.videoSessionParametersTemplate = template_params;
            create_info.videoSession = context.Session();

            VkVideoSessionParametersKHR params2 = VK_NULL_HANDLE;
            m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-10275");
            vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
            m_errorMonitor->VerifyFound();
        }

        // Template is QUANIZATION_MAP_COMPATIBLE but QUANIZATION_MAP_COMPATIBLE is not requested
        {
            auto template_params = context.CreateSessionParamsWithQuantMapTexelSize(texel_size);

            VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
            create_info.videoSessionParametersTemplate = template_params;
            create_info.videoSession = context.Session();

            VkVideoSessionParametersKHR params2 = VK_NULL_HANDLE;
            m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-10277");
            vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
            m_errorMonitor->VerifyFound();
        }
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideo, CreateSessionParamsIncompatibleTemplateQuantMapTexelSize) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - quantizationMapTexelSize mismatches template");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    auto get_alt_texel_size_format = [](const std::vector<vku::safe_VkVideoFormatPropertiesKHR>& props) -> uint32_t {
        const auto texel_size = VideoConfig::GetQuantMapTexelSize(props[0]);
        for (uint32_t i = 1; i < props.size(); i++) {
            const auto alt_texel_size = VideoConfig::GetQuantMapTexelSize(props[i]);
            if ((alt_texel_size.width != texel_size.width || alt_texel_size.height != texel_size.height)) {
                return i;
            }
        }
        return 0;
    };

    uint32_t delta_alt_format_index = 0;
    VideoConfig delta_config = GetConfig(FilterConfigs(GetConfigsEncode(), [&](const auto& config) {
        if (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR) {
            const auto& props = config.SupportedQuantDeltaMapProps();
            if (delta_alt_format_index == 0) {
                delta_alt_format_index = get_alt_texel_size_format(props);
                if (delta_alt_format_index != 0) {
                    return true;
                }
            }
        }
        return false;
    }));

    uint32_t emphasis_alt_format_index = 0;
    VideoConfig emphasis_config = GetConfig(FilterConfigs(GetConfigsEncode(), [&](const auto& config) {
        if (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR) {
            const auto& props = config.SupportedEmphasisMapProps();
            if (emphasis_alt_format_index == 0) {
                emphasis_alt_format_index = get_alt_texel_size_format(props);
                if (emphasis_alt_format_index != 0) {
                    return true;
                }
            }
        }
        return false;
    }));

    if (!delta_config && !emphasis_config) {
        GTEST_SKIP() << "Test requires a video profile that has QUANTIZATION_DELTA or EMPHASIS support, with more than two "
                        "supported texel sizes";
    }

    struct TestConfig {
        VideoConfig config;
        VkVideoSessionCreateFlagBitsKHR flag;
        VkExtent2D texel_sizes[2];
    };

    std::vector<TestConfig> tests;
    if (delta_config) {
        const auto& map_props = delta_config.SupportedQuantDeltaMapProps();
        tests.emplace_back(TestConfig{delta_config,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      {
                                          delta_config.GetQuantMapTexelSize(map_props[0]),
                                          delta_config.GetQuantMapTexelSize(map_props[delta_alt_format_index]),
                                      }});
    }
    if (emphasis_config) {
        const auto& map_props = emphasis_config.SupportedEmphasisMapProps();
        tests.emplace_back(TestConfig{emphasis_config,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                      {
                                          delta_config.GetQuantMapTexelSize(map_props[0]),
                                          delta_config.GetQuantMapTexelSize(map_props[emphasis_alt_format_index]),
                                      }});
    }

    for (auto& [config, flag, texel_sizes] : tests) {
        config.SessionCreateInfo()->flags |= flag;
        VideoContext context(m_device, config);

        auto template_params = context.CreateSessionParamsWithQuantMapTexelSize(texel_sizes[0]);

        VkVideoEncodeQuantizationMapSessionParametersCreateInfoKHR map_create_info = vku::InitStructHelper();
        map_create_info.quantizationMapTexelSize = texel_sizes[1];
        VkVideoSessionParametersCreateInfoKHR create_info2 = *config.SessionParamsCreateInfo();
        map_create_info.pNext = create_info2.pNext;
        create_info2.pNext = &map_create_info;
        create_info2.flags = VK_VIDEO_SESSION_PARAMETERS_CREATE_QUANTIZATION_MAP_COMPATIBLE_BIT_KHR;
        create_info2.videoSessionParametersTemplate = template_params;
        create_info2.videoSession = context.Session();

        VkVideoSessionParametersKHR params2 = VK_NULL_HANDLE;
        m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-10276");
        vk::CreateVideoSessionParametersKHR(device(), &create_info2, nullptr, &params2);
        m_errorMonitor->VerifyFound();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideo, CreateSessionParamsIncompatibleTemplate) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - incompatible template");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigWithParams(GetConfigs());
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with session parameters";
    }

    VideoContext context1(m_device, config);
    VideoContext context2(m_device, config);

    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.videoSessionParametersTemplate = context1.SessionParams();
    create_info.videoSession = context2.Session();

    VkVideoSessionParametersKHR params;
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-04855");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionParamsIncompatibleTemplateEncodeQualityLevel) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - mismatch in encode quality level for template");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigWithMultiEncodeQualityLevelParams(GetConfigsEncode());
    if (!config) {
        GTEST_SKIP() << "Test requires an encode profile with support for parameters objects and at least two quality levels";
    }

    VideoContext context(m_device, config);

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    auto quality_level_info = vku::InitStruct<VkVideoEncodeQualityLevelInfoKHR>();
    quality_level_info.pNext = create_info.pNext;
    create_info.pNext = &quality_level_info;
    create_info.videoSession = context.Session();

    // Expect to fail to create parameters object with max encode quality level
    // with template using encode quality level 0
    create_info.videoSessionParametersTemplate = context.SessionParams();
    quality_level_info.qualityLevel = 1;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-08310");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    // Expect to succeed to create parameters object with explicit encode quality level 0 against the same template
    quality_level_info.qualityLevel = 0;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);

    // Expect to succeed to create parameters object with highest encode quality level index but no template
    create_info.videoSessionParametersTemplate = VK_NULL_HANDLE;
    quality_level_info.qualityLevel = config.EncodeCaps()->maxQualityLevels - 1;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);

    // Expect to fail to create parameters object with encode quality level 0
    // with template using highest encode quality level index
    create_info.videoSessionParametersTemplate = params;
    quality_level_info.qualityLevel = 0;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-08310");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    // Expect to fail the same with implicit encode quality level 0
    create_info.pNext = quality_level_info.pNext;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-08310");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideo, CreateSessionParamsMissingCodecInfo) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - missing codec-specific chained structure");

    RETURN_IF_SKIP(Init());

    if (GetConfigDecodeH264()) {
        VideoConfig config = GetConfigDecodeH264();
        VideoContext context(m_device, config);

        VkVideoSessionParametersKHR params;
        VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
        auto other_codec_info = vku::InitStruct<VkVideoDecodeH265SessionParametersCreateInfoKHR>();
        other_codec_info.maxStdVPSCount = 1;
        other_codec_info.maxStdSPSCount = 1;
        other_codec_info.maxStdPPSCount = 1;
        create_info.pNext = &other_codec_info;
        create_info.videoSession = context.Session();

        m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07203");
        vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigDecodeH265()) {
        VideoConfig config = GetConfigDecodeH265();
        VideoContext context(m_device, config);

        VkVideoSessionParametersKHR params;
        VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
        auto other_codec_info = vku::InitStruct<VkVideoDecodeH264SessionParametersCreateInfoKHR>();
        other_codec_info.maxStdSPSCount = 1;
        other_codec_info.maxStdPPSCount = 1;
        create_info.pNext = &other_codec_info;
        create_info.videoSession = context.Session();

        m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07206");
        vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigDecodeAV1()) {
        VideoConfig config = GetConfigDecodeAV1();
        VideoContext context(DeviceObj(), config);

        VkVideoSessionParametersKHR params;
        VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
        auto other_codec_info = vku::InitStruct<VkVideoDecodeH264SessionParametersCreateInfoKHR>();
        other_codec_info.maxStdSPSCount = 1;
        other_codec_info.maxStdPPSCount = 1;
        create_info.pNext = &other_codec_info;
        create_info.videoSession = context.Session();

        m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-09259");
        vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigEncodeH264()) {
        VideoConfig config = GetConfigEncodeH264();
        VideoContext context(m_device, config);

        VkVideoSessionParametersKHR params;
        VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
        auto other_codec_info = vku::InitStruct<VkVideoEncodeH265SessionParametersCreateInfoKHR>();
        other_codec_info.maxStdVPSCount = 1;
        other_codec_info.maxStdSPSCount = 1;
        other_codec_info.maxStdPPSCount = 1;
        create_info.pNext = &other_codec_info;
        create_info.videoSession = context.Session();

        m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07210");
        vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigEncodeH265()) {
        VideoConfig config = GetConfigEncodeH265();
        VideoContext context(m_device, config);

        VkVideoSessionParametersKHR params;
        VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
        auto other_codec_info = vku::InitStruct<VkVideoEncodeH264SessionParametersCreateInfoKHR>();
        other_codec_info.maxStdSPSCount = 1;
        other_codec_info.maxStdPPSCount = 1;
        create_info.pNext = &other_codec_info;
        create_info.videoSession = context.Session();

        m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07211");
        vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigEncodeAV1()) {
        VideoConfig config = GetConfigEncodeAV1();
        VideoContext context(m_device, config);

        VkVideoSessionParametersKHR params;
        VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
        auto other_codec_info = vku::InitStruct<VkVideoEncodeH264SessionParametersCreateInfoKHR>();
        other_codec_info.maxStdSPSCount = 1;
        other_codec_info.maxStdPPSCount = 1;
        create_info.pNext = &other_codec_info;
        create_info.videoSession = context.Session();

        m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-10279");
        vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeVideo, CreateSessionParamsInvalidEncodeQualityLevel) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - invalid encode quality level");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigWithParams(GetConfigsEncode());
    if (!config) {
        GTEST_SKIP() << "Test requires an encode profile with session parameters";
    }

    VideoContext context(m_device, config);

    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    auto quality_level_info = vku::InitStruct<VkVideoEncodeQualityLevelInfoKHR>();
    quality_level_info.qualityLevel = config.EncodeCaps()->maxQualityLevels;
    quality_level_info.pNext = create_info.pNext;
    create_info.pNext = &quality_level_info;
    create_info.videoSession = context.Session();

    VkVideoSessionParametersKHR params;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeQualityLevelInfoKHR-qualityLevel-08311");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionParamsDecodeH264ExceededCapacity) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - H.264 decode parameter set capacity exceeded");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigDecodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support";
    }

    VideoContext context(m_device, config);

    auto h264_ci = vku::InitStruct<VkVideoDecodeH264SessionParametersCreateInfoKHR>();
    auto h264_ai = vku::InitStruct<VkVideoDecodeH264SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params, params2;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h264_ci;
    create_info.videoSession = context.Session();

    h264_ci.pParametersAddInfo = &h264_ai;

    std::vector<StdVideoH264SequenceParameterSet> sps_list{context.CreateH264SPS(1), context.CreateH264SPS(2),
                                                           context.CreateH264SPS(3)};

    std::vector<StdVideoH264PictureParameterSet> pps_list{
        context.CreateH264PPS(1, 1), context.CreateH264PPS(1, 4), context.CreateH264PPS(2, 1),
        context.CreateH264PPS(2, 2), context.CreateH264PPS(3, 1), context.CreateH264PPS(3, 3),
    };

    h264_ai.stdSPSCount = (uint32_t)sps_list.size();
    h264_ai.pStdSPSs = sps_list.data();
    h264_ai.stdPPSCount = (uint32_t)pps_list.size();
    h264_ai.pStdPPSs = pps_list.data();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07204");
    h264_ci.maxStdSPSCount = 2;
    h264_ci.maxStdPPSCount = 8;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07205");
    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 5;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 6;
    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    create_info.videoSessionParametersTemplate = params;
    sps_list[1].seq_parameter_set_id = 4;
    pps_list[1].seq_parameter_set_id = 4;
    pps_list[5].seq_parameter_set_id = 4;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07204");
    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 8;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07205");
    h264_ci.maxStdSPSCount = 4;
    h264_ci.maxStdPPSCount = 7;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    create_info.videoSessionParametersTemplate = params;
    h264_ci.pParametersAddInfo = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07204");
    h264_ci.maxStdSPSCount = 2;
    h264_ci.maxStdPPSCount = 8;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07205");
    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 5;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideo, CreateSessionParamsDecodeH265ExceededCapacity) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - H.265 decode parameter set capacity exceeded");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigDecodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support";
    }

    VideoContext context(m_device, config);

    auto h265_ci = vku::InitStruct<VkVideoDecodeH265SessionParametersCreateInfoKHR>();
    auto h265_ai = vku::InitStruct<VkVideoDecodeH265SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params, params2;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h265_ci;
    create_info.videoSession = context.Session();

    h265_ci.pParametersAddInfo = &h265_ai;

    std::vector<StdVideoH265VideoParameterSet> vps_list{
        context.CreateH265VPS(1),
        context.CreateH265VPS(2),
    };

    std::vector<StdVideoH265SequenceParameterSet> sps_list{
        context.CreateH265SPS(1, 1),
        context.CreateH265SPS(1, 2),
        context.CreateH265SPS(2, 1),
        context.CreateH265SPS(2, 3),
    };

    std::vector<StdVideoH265PictureParameterSet> pps_list{
        context.CreateH265PPS(1, 1, 1), context.CreateH265PPS(1, 1, 2), context.CreateH265PPS(1, 2, 1),
        context.CreateH265PPS(2, 1, 3), context.CreateH265PPS(2, 3, 1), context.CreateH265PPS(2, 3, 2),
        context.CreateH265PPS(2, 3, 3),
    };

    h265_ai.stdVPSCount = (uint32_t)vps_list.size();
    h265_ai.pStdVPSs = vps_list.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list.size();
    h265_ai.pStdSPSs = sps_list.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list.size();
    h265_ai.pStdPPSs = pps_list.data();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07207");
    h265_ci.maxStdVPSCount = 1;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 8;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07208");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 3;
    h265_ci.maxStdPPSCount = 9;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07209");
    h265_ci.maxStdVPSCount = 3;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 5;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 7;
    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    create_info.videoSessionParametersTemplate = params;
    vps_list[1].vps_video_parameter_set_id = 3;
    sps_list[1].sps_video_parameter_set_id = 3;
    pps_list[1].sps_video_parameter_set_id = 3;
    pps_list[5].sps_video_parameter_set_id = 3;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07207");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 10;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07208");
    h265_ci.maxStdVPSCount = 3;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 9;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07209");
    h265_ci.maxStdVPSCount = 3;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 8;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    create_info.videoSessionParametersTemplate = params;
    h265_ci.pParametersAddInfo = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07207");
    h265_ci.maxStdVPSCount = 1;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 7;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07208");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 3;
    h265_ci.maxStdPPSCount = 7;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07209");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 6;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideo, CreateSessionParamsDecodeAV1TemplateNotAllowed) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - AV1 decode does not allow using parameters templates");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigDecodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 decode support";
    }

    VideoContext context(DeviceObj(), config);

    StdVideoAV1SequenceHeader seq_header{};
    auto av1_ci = vku::InitStruct<VkVideoDecodeAV1SessionParametersCreateInfoKHR>();
    av1_ci.pStdSequenceHeader = &seq_header;

    VkVideoSessionParametersKHR params, params2;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &av1_ci;
    create_info.videoSession = context.Session();

    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);

    create_info.videoSessionParametersTemplate = params;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-09258");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideo, CreateSessionParamsEncodeH264ExceededCapacity) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - H.264 encode parameter set capacity exceeded");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    VideoContext context(m_device, config);

    auto h264_ci = vku::InitStruct<VkVideoEncodeH264SessionParametersCreateInfoKHR>();
    auto h264_ai = vku::InitStruct<VkVideoEncodeH264SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params, params2;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h264_ci;
    create_info.videoSession = context.Session();

    h264_ci.pParametersAddInfo = &h264_ai;

    std::vector<StdVideoH264SequenceParameterSet> sps_list{context.CreateH264SPS(1), context.CreateH264SPS(2),
                                                           context.CreateH264SPS(3)};

    std::vector<StdVideoH264PictureParameterSet> pps_list{
        context.CreateH264PPS(1, 1), context.CreateH264PPS(1, 4), context.CreateH264PPS(2, 1),
        context.CreateH264PPS(2, 2), context.CreateH264PPS(3, 1), context.CreateH264PPS(3, 3),
    };

    h264_ai.stdSPSCount = (uint32_t)sps_list.size();
    h264_ai.pStdSPSs = sps_list.data();
    h264_ai.stdPPSCount = (uint32_t)pps_list.size();
    h264_ai.pStdPPSs = pps_list.data();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04839");
    h264_ci.maxStdSPSCount = 2;
    h264_ci.maxStdPPSCount = 8;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04840");
    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 5;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 6;
    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    create_info.videoSessionParametersTemplate = params;
    sps_list[1].seq_parameter_set_id = 4;
    pps_list[1].seq_parameter_set_id = 4;
    pps_list[5].seq_parameter_set_id = 4;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04839");
    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 8;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04840");
    h264_ci.maxStdSPSCount = 4;
    h264_ci.maxStdPPSCount = 7;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    create_info.videoSessionParametersTemplate = params;
    h264_ci.pParametersAddInfo = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04839");
    h264_ci.maxStdSPSCount = 2;
    h264_ci.maxStdPPSCount = 8;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04840");
    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 5;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideo, CreateSessionParamsEncodeH265ExceededCapacity) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - H.265 encode parameter set capacity exceeded");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VideoContext context(m_device, config);

    auto h265_ci = vku::InitStruct<VkVideoEncodeH265SessionParametersCreateInfoKHR>();
    auto h265_ai = vku::InitStruct<VkVideoEncodeH265SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params, params2;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h265_ci;
    create_info.videoSession = context.Session();

    h265_ci.pParametersAddInfo = &h265_ai;

    std::vector<StdVideoH265VideoParameterSet> vps_list{
        context.CreateH265VPS(1),
        context.CreateH265VPS(2),
    };

    std::vector<StdVideoH265SequenceParameterSet> sps_list{
        context.CreateH265SPS(1, 1),
        context.CreateH265SPS(1, 2),
        context.CreateH265SPS(2, 1),
        context.CreateH265SPS(2, 3),
    };

    std::vector<StdVideoH265PictureParameterSet> pps_list{
        context.CreateH265PPS(1, 1, 1), context.CreateH265PPS(1, 1, 2), context.CreateH265PPS(1, 2, 1),
        context.CreateH265PPS(2, 1, 3), context.CreateH265PPS(2, 3, 1), context.CreateH265PPS(2, 3, 2),
        context.CreateH265PPS(2, 3, 3),
    };

    h265_ai.stdVPSCount = (uint32_t)vps_list.size();
    h265_ai.pStdVPSs = vps_list.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list.size();
    h265_ai.pStdSPSs = sps_list.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list.size();
    h265_ai.pStdPPSs = pps_list.data();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04841");
    h265_ci.maxStdVPSCount = 1;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 8;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04842");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 3;
    h265_ci.maxStdPPSCount = 9;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04843");
    h265_ci.maxStdVPSCount = 3;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 5;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 7;
    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    create_info.videoSessionParametersTemplate = params;
    vps_list[1].vps_video_parameter_set_id = 3;
    sps_list[1].sps_video_parameter_set_id = 3;
    pps_list[1].sps_video_parameter_set_id = 3;
    pps_list[5].sps_video_parameter_set_id = 3;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04841");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 10;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04842");
    h265_ci.maxStdVPSCount = 3;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 9;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04843");
    h265_ci.maxStdVPSCount = 3;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 8;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    create_info.videoSessionParametersTemplate = params;
    h265_ci.pParametersAddInfo = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04841");
    h265_ci.maxStdVPSCount = 1;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 7;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04842");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 3;
    h265_ci.maxStdPPSCount = 7;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04843");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 6;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideo, CreateUpdateSessionParamsEncodeH265InvalidTileColumnsRows) {
    TEST_DESCRIPTION("vkCreate/UpdateVideoSessionParametersKHR - H.265 encode PPS contains invalid tile columns/rows");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VideoContext context(m_device, config);

    auto h265_ci = vku::InitStruct<VkVideoEncodeH265SessionParametersCreateInfoKHR>();
    auto h265_ai = vku::InitStruct<VkVideoEncodeH265SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h265_ci;
    create_info.videoSession = context.Session();

    h265_ci.pParametersAddInfo = &h265_ai;

    auto h265_vps = context.CreateH265VPS(1);
    auto h265_sps = context.CreateH265SPS(1, 1);

    std::vector<StdVideoH265PictureParameterSet> h265_pps_list{context.CreateH265PPS(1, 1, 1), context.CreateH265PPS(1, 1, 2),
                                                               context.CreateH265PPS(1, 1, 3), context.CreateH265PPS(1, 1, 4),
                                                               context.CreateH265PPS(1, 1, 5), context.CreateH265PPS(1, 1, 6)};

    h265_ci.maxStdVPSCount = 1;
    h265_ci.maxStdSPSCount = 1;
    h265_ci.maxStdPPSCount = (uint32_t)h265_pps_list.size();

    // Configure some of the PPS entries with various out-of-bounds tile row/column counts

    // Let PPS #2 have out-of-bounds num_tile_columns_minus1
    h265_pps_list[1].num_tile_columns_minus1 = (uint8_t)config.EncodeCapsH265()->maxTiles.width;
    h265_pps_list[1].num_tile_rows_minus1 = (uint8_t)config.EncodeCapsH265()->maxTiles.height / 2;

    // Let PPS #3 use the max limits
    h265_pps_list[2].num_tile_columns_minus1 = (uint8_t)config.EncodeCapsH265()->maxTiles.width - 1;
    h265_pps_list[2].num_tile_rows_minus1 = (uint8_t)config.EncodeCapsH265()->maxTiles.height - 1;

    // Let PPS #5 have out-of-bounds num_tile_rows_minus1
    h265_pps_list[4].num_tile_columns_minus1 = (uint8_t)config.EncodeCapsH265()->maxTiles.width / 2;
    h265_pps_list[4].num_tile_rows_minus1 = (uint8_t)config.EncodeCapsH265()->maxTiles.height;

    // Let PPS #6 have out-of-bounds num_tile_columns_minus1 and num_tile_rows_minus1
    h265_pps_list[5].num_tile_columns_minus1 = (uint8_t)config.EncodeCapsH265()->maxTiles.width;
    h265_pps_list[5].num_tile_rows_minus1 = (uint8_t)config.EncodeCapsH265()->maxTiles.height;

    h265_ai.stdVPSCount = 1;
    h265_ai.pStdVPSs = &h265_vps;
    h265_ai.stdSPSCount = 1;
    h265_ai.pStdSPSs = &h265_sps;
    h265_ai.stdPPSCount = (uint32_t)h265_pps_list.size();
    h265_ai.pStdPPSs = h265_pps_list.data();

    // Try first all of them together
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08319");
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08320");
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08319");
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08320");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    // Then individual invalid ones one-by-one
    h265_ai.stdPPSCount = 1;

    h265_ai.pStdPPSs = &h265_pps_list[1];
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08319");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    h265_ai.pStdPPSs = &h265_pps_list[4];
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08320");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    h265_ai.pStdPPSs = &h265_pps_list[5];
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08319");
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08320");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    // Successfully create object with an entry with the max limits
    h265_ai.pStdPPSs = &h265_pps_list[2];
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);

    // But then try to update with invalid ones
    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.pNext = &h265_ai;
    update_info.updateSequenceCount = 1;

    h265_ai.stdVPSCount = 0;
    h265_ai.stdSPSCount = 0;
    h265_ai.stdPPSCount = 1;

    h265_ai.pStdPPSs = &h265_pps_list[1];
    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-08321");
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    h265_ai.pStdPPSs = &h265_pps_list[4];
    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-08322");
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    h265_ai.pStdPPSs = &h265_pps_list[5];
    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-08321");
    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-08322");
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideo, CreateSessionParamsEncodeAV1TemplateNotAllowed) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - AV1 encode does not allow using parameters templates");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VideoContext context(DeviceObj(), config);

    StdVideoAV1SequenceHeader seq_header{};
    auto av1_ci = vku::InitStruct<VkVideoEncodeAV1SessionParametersCreateInfoKHR>();
    av1_ci.pStdSequenceHeader = &seq_header;

    VkVideoSessionParametersKHR params, params2;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &av1_ci;
    create_info.videoSession = context.Session();

    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);

    create_info.videoSessionParametersTemplate = params;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-10278");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideo, CreateSessionParamsEncodeAV1FilmGrain) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - encoding with film grain is not support");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VideoContext context(m_device, config);

    auto av1_seq_header = config.CreateAV1SequenceHeader();
    av1_seq_header.flags.film_grain_params_present = 1;

    auto av1_ci = vku::InitStruct<VkVideoEncodeAV1SessionParametersCreateInfoKHR>();
    av1_ci.pStdSequenceHeader = &av1_seq_header;

    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &av1_ci;
    create_info.videoSession = context.Session();

    VkVideoSessionParametersKHR params;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeAV1SessionParametersCreateInfoKHR-pStdSequenceHeader-10288");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionParamsEncodeAV1TooManyOperatingPoints) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - too many operating points");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VideoContext context(m_device, config);

    auto av1_seq_header = config.CreateAV1SequenceHeader();
    av1_seq_header.flags.reduced_still_picture_header = 0;
    std::vector<StdVideoEncodeAV1OperatingPointInfo> av1_op_points(config.EncodeCapsAV1()->maxOperatingPoints + 1);

    auto av1_ci = vku::InitStruct<VkVideoEncodeAV1SessionParametersCreateInfoKHR>();
    av1_ci.pStdSequenceHeader = &av1_seq_header;
    av1_ci.stdOperatingPointCount = static_cast<uint32_t>(av1_op_points.size());
    av1_ci.pStdOperatingPoints = av1_op_points.data();

    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &av1_ci;
    create_info.videoSession = context.Session();

    VkVideoSessionParametersKHR params;
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-10280");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, DecodeH264ParametersAddInfoUniqueness) {
    TEST_DESCRIPTION("VkVideoDecodeH264SessionParametersAddInfoKHR - parameter set uniqueness");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigDecodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support";
    }

    VideoContext context(m_device, config);

    auto h264_ci = vku::InitStruct<VkVideoDecodeH264SessionParametersCreateInfoKHR>();
    auto h264_ai = vku::InitStruct<VkVideoDecodeH264SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h264_ci;
    create_info.videoSession = context.Session();

    h264_ci.maxStdSPSCount = 10;
    h264_ci.maxStdPPSCount = 20;
    h264_ci.pParametersAddInfo = &h264_ai;

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.pNext = &h264_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH264SequenceParameterSet> sps_list{context.CreateH264SPS(1), context.CreateH264SPS(2),
                                                           context.CreateH264SPS(3)};

    std::vector<StdVideoH264PictureParameterSet> pps_list{
        context.CreateH264PPS(1, 1), context.CreateH264PPS(1, 4), context.CreateH264PPS(2, 1),
        context.CreateH264PPS(2, 2), context.CreateH264PPS(3, 1), context.CreateH264PPS(3, 3),
    };

    h264_ai.stdSPSCount = (uint32_t)sps_list.size();
    h264_ai.pStdSPSs = sps_list.data();
    h264_ai.stdPPSCount = (uint32_t)pps_list.size();
    h264_ai.pStdPPSs = pps_list.data();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH264SessionParametersAddInfoKHR-None-04825");
    sps_list[0].seq_parameter_set_id = 3;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    sps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH264SessionParametersAddInfoKHR-None-04826");
    pps_list[0].seq_parameter_set_id = 2;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    pps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    h264_ci.pParametersAddInfo = nullptr;
    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH264SessionParametersAddInfoKHR-None-04825");
    sps_list[0].seq_parameter_set_id = 3;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH264SessionParametersAddInfoKHR-None-04826");
    pps_list[0].seq_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideo, DecodeH265ParametersAddInfoUniqueness) {
    TEST_DESCRIPTION("VkVideoDecodeH265SessionParametersAddInfoKHR - parameter set uniqueness");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigDecodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    auto h265_ci = vku::InitStruct<VkVideoDecodeH265SessionParametersCreateInfoKHR>();
    auto h265_ai = vku::InitStruct<VkVideoDecodeH265SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h265_ci;
    create_info.videoSession = context.Session();

    h265_ci.maxStdVPSCount = 10;
    h265_ci.maxStdSPSCount = 20;
    h265_ci.maxStdPPSCount = 30;
    h265_ci.pParametersAddInfo = &h265_ai;

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.pNext = &h265_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH265VideoParameterSet> vps_list{
        context.CreateH265VPS(1),
        context.CreateH265VPS(2),
    };

    std::vector<StdVideoH265SequenceParameterSet> sps_list{
        context.CreateH265SPS(1, 1),
        context.CreateH265SPS(1, 2),
        context.CreateH265SPS(2, 1),
        context.CreateH265SPS(2, 3),
    };

    std::vector<StdVideoH265PictureParameterSet> pps_list{
        context.CreateH265PPS(1, 1, 1), context.CreateH265PPS(1, 1, 2), context.CreateH265PPS(1, 2, 1),
        context.CreateH265PPS(2, 1, 3), context.CreateH265PPS(2, 3, 1), context.CreateH265PPS(2, 3, 2),
        context.CreateH265PPS(2, 3, 3),
    };

    h265_ai.stdVPSCount = (uint32_t)vps_list.size();
    h265_ai.pStdVPSs = vps_list.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list.size();
    h265_ai.pStdSPSs = sps_list.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list.size();
    h265_ai.pStdPPSs = pps_list.data();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04833");
    vps_list[0].vps_video_parameter_set_id = 2;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    vps_list[0].vps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04834");
    sps_list[0].sps_video_parameter_set_id = 2;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    sps_list[0].sps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04835");
    pps_list[0].pps_seq_parameter_set_id = 2;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    pps_list[0].pps_seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    h265_ci.pParametersAddInfo = nullptr;
    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04833");
    vps_list[0].vps_video_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    vps_list[0].vps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04834");
    sps_list[0].sps_video_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list[0].sps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04835");
    pps_list[0].pps_seq_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list[0].pps_seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideo, EncodeH264ParametersAddInfoUniqueness) {
    TEST_DESCRIPTION("VkVideoEncodeH264SessionParametersAddInfoKHR - parameter set uniqueness");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    VideoContext context(m_device, config);

    auto h264_ci = vku::InitStruct<VkVideoEncodeH264SessionParametersCreateInfoKHR>();
    auto h264_ai = vku::InitStruct<VkVideoEncodeH264SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h264_ci;
    create_info.videoSession = context.Session();

    h264_ci.maxStdSPSCount = 10;
    h264_ci.maxStdPPSCount = 20;
    h264_ci.pParametersAddInfo = &h264_ai;

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.pNext = &h264_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH264SequenceParameterSet> sps_list{context.CreateH264SPS(1), context.CreateH264SPS(2),
                                                           context.CreateH264SPS(3)};

    std::vector<StdVideoH264PictureParameterSet> pps_list{
        context.CreateH264PPS(1, 1), context.CreateH264PPS(1, 4), context.CreateH264PPS(2, 1),
        context.CreateH264PPS(2, 2), context.CreateH264PPS(3, 1), context.CreateH264PPS(3, 3),
    };

    h264_ai.stdSPSCount = (uint32_t)sps_list.size();
    h264_ai.pStdSPSs = sps_list.data();
    h264_ai.stdPPSCount = (uint32_t)pps_list.size();
    h264_ai.pStdPPSs = pps_list.data();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264SessionParametersAddInfoKHR-None-04837");
    sps_list[0].seq_parameter_set_id = 3;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    sps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264SessionParametersAddInfoKHR-None-04838");
    pps_list[0].seq_parameter_set_id = 2;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    pps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    h264_ci.pParametersAddInfo = nullptr;
    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264SessionParametersAddInfoKHR-None-04837");
    sps_list[0].seq_parameter_set_id = 3;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264SessionParametersAddInfoKHR-None-04838");
    pps_list[0].seq_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideo, EncodeH265ParametersAddInfoUniqueness) {
    TEST_DESCRIPTION("VkVideoEncodeH265SessionParametersAddInfoKHR - parameter set uniqueness");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    auto h265_ci = vku::InitStruct<VkVideoEncodeH265SessionParametersCreateInfoKHR>();
    auto h265_ai = vku::InitStruct<VkVideoEncodeH265SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h265_ci;
    create_info.videoSession = context.Session();

    h265_ci.maxStdVPSCount = 10;
    h265_ci.maxStdSPSCount = 20;
    h265_ci.maxStdPPSCount = 30;
    h265_ci.pParametersAddInfo = &h265_ai;

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.pNext = &h265_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH265VideoParameterSet> vps_list{
        context.CreateH265VPS(1),
        context.CreateH265VPS(2),
    };

    std::vector<StdVideoH265SequenceParameterSet> sps_list{
        context.CreateH265SPS(1, 1),
        context.CreateH265SPS(1, 2),
        context.CreateH265SPS(2, 1),
        context.CreateH265SPS(2, 3),
    };

    std::vector<StdVideoH265PictureParameterSet> pps_list{
        context.CreateH265PPS(1, 1, 1), context.CreateH265PPS(1, 1, 2), context.CreateH265PPS(1, 2, 1),
        context.CreateH265PPS(2, 1, 3), context.CreateH265PPS(2, 3, 1), context.CreateH265PPS(2, 3, 2),
        context.CreateH265PPS(2, 3, 3),
    };

    h265_ai.stdVPSCount = (uint32_t)vps_list.size();
    h265_ai.pStdVPSs = vps_list.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list.size();
    h265_ai.pStdSPSs = sps_list.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list.size();
    h265_ai.pStdPPSs = pps_list.data();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06438");
    vps_list[0].vps_video_parameter_set_id = 2;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    vps_list[0].vps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06439");
    sps_list[0].sps_video_parameter_set_id = 2;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    sps_list[0].sps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06440");
    pps_list[0].pps_seq_parameter_set_id = 2;
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    pps_list[0].pps_seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    h265_ci.pParametersAddInfo = nullptr;
    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06438");
    vps_list[0].vps_video_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    vps_list[0].vps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06439");
    sps_list[0].sps_video_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list[0].sps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06440");
    pps_list[0].pps_seq_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list[0].pps_seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideo, UpdateSessionParamsIncorrectSequenceCount) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - invalid update sequence count");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigWithParams(GetConfigs());
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with session parameters";
    }

    VideoContext context(m_device, config);

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-pUpdateInfo-07215");
    update_info.updateSequenceCount = 2;
    vk::UpdateVideoSessionParametersKHR(device(), context.SessionParams(), &update_info);
    m_errorMonitor->VerifyFound();

    update_info.updateSequenceCount = 1;
    ASSERT_EQ(VK_SUCCESS, vk::UpdateVideoSessionParametersKHR(device(), context.SessionParams(), &update_info));

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-pUpdateInfo-07215");
    update_info.updateSequenceCount = 1;
    vk::UpdateVideoSessionParametersKHR(device(), context.SessionParams(), &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-pUpdateInfo-07215");
    update_info.updateSequenceCount = 3;
    vk::UpdateVideoSessionParametersKHR(device(), context.SessionParams(), &update_info);
    m_errorMonitor->VerifyFound();

    update_info.updateSequenceCount = 2;
    ASSERT_EQ(VK_SUCCESS, vk::UpdateVideoSessionParametersKHR(device(), context.SessionParams(), &update_info));
}

TEST_F(NegativeVideo, UpdateSessionParamsDecodeH264ConflictingKeys) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - H.264 decode conflicting parameter set keys");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigDecodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support";
    }

    VideoContext context(m_device, config);

    auto h264_ci = vku::InitStruct<VkVideoDecodeH264SessionParametersCreateInfoKHR>();
    auto h264_ai = vku::InitStruct<VkVideoDecodeH264SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h264_ci;
    create_info.videoSession = context.Session();

    h264_ci.maxStdSPSCount = 10;
    h264_ci.maxStdPPSCount = 20;
    h264_ci.pParametersAddInfo = &h264_ai;

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.pNext = &h264_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH264SequenceParameterSet> sps_list{context.CreateH264SPS(1), context.CreateH264SPS(2),
                                                           context.CreateH264SPS(3)};

    std::vector<StdVideoH264PictureParameterSet> pps_list{
        context.CreateH264PPS(1, 1), context.CreateH264PPS(1, 4), context.CreateH264PPS(2, 1),
        context.CreateH264PPS(2, 2), context.CreateH264PPS(3, 1), context.CreateH264PPS(3, 3),
    };

    h264_ai.stdSPSCount = (uint32_t)sps_list.size();
    h264_ai.pStdSPSs = sps_list.data();
    h264_ai.stdPPSCount = (uint32_t)pps_list.size();
    h264_ai.pStdPPSs = pps_list.data();

    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    std::vector<StdVideoH264SequenceParameterSet> sps_list2{context.CreateH264SPS(4), context.CreateH264SPS(5)};

    std::vector<StdVideoH264PictureParameterSet> pps_list2{context.CreateH264PPS(1, 3), context.CreateH264PPS(3, 2),
                                                           context.CreateH264PPS(4, 1), context.CreateH264PPS(5, 2)};

    h264_ai.stdSPSCount = (uint32_t)sps_list2.size();
    h264_ai.pStdSPSs = sps_list2.data();
    h264_ai.stdPPSCount = (uint32_t)pps_list2.size();
    h264_ai.pStdPPSs = pps_list2.data();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07216");
    sps_list2[1].seq_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list2[1].seq_parameter_set_id = 5;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07218");
    pps_list2[2].seq_parameter_set_id = 1;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list2[2].seq_parameter_set_id = 4;
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideo, UpdateSessionParamsDecodeH265ConflictingKeys) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - H.265 decode conflicting parameter set keys");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigDecodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    auto h265_ci = vku::InitStruct<VkVideoDecodeH265SessionParametersCreateInfoKHR>();
    auto h265_ai = vku::InitStruct<VkVideoDecodeH265SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h265_ci;
    create_info.videoSession = context.Session();

    h265_ci.maxStdVPSCount = 10;
    h265_ci.maxStdSPSCount = 20;
    h265_ci.maxStdPPSCount = 30;
    h265_ci.pParametersAddInfo = &h265_ai;

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.pNext = &h265_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH265VideoParameterSet> vps_list{
        context.CreateH265VPS(1),
        context.CreateH265VPS(2),
    };

    std::vector<StdVideoH265SequenceParameterSet> sps_list{
        context.CreateH265SPS(1, 1),
        context.CreateH265SPS(1, 2),
        context.CreateH265SPS(2, 1),
        context.CreateH265SPS(2, 3),
    };

    std::vector<StdVideoH265PictureParameterSet> pps_list{
        context.CreateH265PPS(1, 1, 1), context.CreateH265PPS(1, 1, 2), context.CreateH265PPS(1, 2, 1),
        context.CreateH265PPS(2, 1, 3), context.CreateH265PPS(2, 3, 1), context.CreateH265PPS(2, 3, 2),
        context.CreateH265PPS(2, 3, 3),
    };

    h265_ai.stdVPSCount = (uint32_t)vps_list.size();
    h265_ai.pStdVPSs = vps_list.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list.size();
    h265_ai.pStdSPSs = sps_list.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list.size();
    h265_ai.pStdPPSs = pps_list.data();

    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    std::vector<StdVideoH265VideoParameterSet> vps_list2{context.CreateH265VPS(3)};

    std::vector<StdVideoH265SequenceParameterSet> sps_list2{context.CreateH265SPS(2, 2), context.CreateH265SPS(3, 1)};

    std::vector<StdVideoH265PictureParameterSet> pps_list2{context.CreateH265PPS(1, 2, 3), context.CreateH265PPS(2, 3, 4),
                                                           context.CreateH265PPS(3, 1, 2)};

    h265_ai.stdVPSCount = (uint32_t)vps_list2.size();
    h265_ai.pStdVPSs = vps_list2.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list2.size();
    h265_ai.pStdSPSs = sps_list2.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list2.size();
    h265_ai.pStdPPSs = pps_list2.data();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07220");
    vps_list2[0].vps_video_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    vps_list2[0].vps_video_parameter_set_id = 3;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07222");
    sps_list2[0].sps_seq_parameter_set_id = 3;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list2[0].sps_seq_parameter_set_id = 2;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07224");
    pps_list2[1].pps_pic_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list2[1].pps_pic_parameter_set_id = 4;
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideo, UpdateSessionParamsDecodeAV1) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - not supported for AV1 decode");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigDecodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 decode support";
    }

    VideoContext context(DeviceObj(), config);

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.updateSequenceCount = 1;

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-09260");
    vk::UpdateVideoSessionParametersKHR(device(), context.SessionParams(), &update_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, UpdateSessionParamsEncodeH264ConflictingKeys) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - H.264 encode conflicting parameter set keys");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    VideoContext context(m_device, config);

    auto h264_ci = vku::InitStruct<VkVideoEncodeH264SessionParametersCreateInfoKHR>();
    auto h264_ai = vku::InitStruct<VkVideoEncodeH264SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h264_ci;
    create_info.videoSession = context.Session();

    h264_ci.maxStdSPSCount = 10;
    h264_ci.maxStdPPSCount = 20;
    h264_ci.pParametersAddInfo = &h264_ai;

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.pNext = &h264_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH264SequenceParameterSet> sps_list{context.CreateH264SPS(1), context.CreateH264SPS(2),
                                                           context.CreateH264SPS(3)};

    std::vector<StdVideoH264PictureParameterSet> pps_list{
        context.CreateH264PPS(1, 1), context.CreateH264PPS(1, 4), context.CreateH264PPS(2, 1),
        context.CreateH264PPS(2, 2), context.CreateH264PPS(3, 1), context.CreateH264PPS(3, 3),
    };

    h264_ai.stdSPSCount = (uint32_t)sps_list.size();
    h264_ai.pStdSPSs = sps_list.data();
    h264_ai.stdPPSCount = (uint32_t)pps_list.size();
    h264_ai.pStdPPSs = pps_list.data();

    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    std::vector<StdVideoH264SequenceParameterSet> sps_list2{context.CreateH264SPS(4), context.CreateH264SPS(5)};

    std::vector<StdVideoH264PictureParameterSet> pps_list2{context.CreateH264PPS(1, 3), context.CreateH264PPS(3, 2),
                                                           context.CreateH264PPS(4, 1), context.CreateH264PPS(5, 2)};

    h264_ai.stdSPSCount = (uint32_t)sps_list2.size();
    h264_ai.pStdSPSs = sps_list2.data();
    h264_ai.stdPPSCount = (uint32_t)pps_list2.size();
    h264_ai.pStdPPSs = pps_list2.data();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07226");
    sps_list2[1].seq_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list2[1].seq_parameter_set_id = 5;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07227");
    pps_list2[2].seq_parameter_set_id = 1;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list2[2].seq_parameter_set_id = 4;
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideo, UpdateSessionParamsEncodeH265ConflictingKeys) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - H.265 encode conflicting parameter set keys");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    auto h265_ci = vku::InitStruct<VkVideoEncodeH265SessionParametersCreateInfoKHR>();
    auto h265_ai = vku::InitStruct<VkVideoEncodeH265SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h265_ci;
    create_info.videoSession = context.Session();

    h265_ci.maxStdVPSCount = 10;
    h265_ci.maxStdSPSCount = 20;
    h265_ci.maxStdPPSCount = 30;
    h265_ci.pParametersAddInfo = &h265_ai;

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.pNext = &h265_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH265VideoParameterSet> vps_list{
        context.CreateH265VPS(1),
        context.CreateH265VPS(2),
    };

    std::vector<StdVideoH265SequenceParameterSet> sps_list{
        context.CreateH265SPS(1, 1),
        context.CreateH265SPS(1, 2),
        context.CreateH265SPS(2, 1),
        context.CreateH265SPS(2, 3),
    };

    std::vector<StdVideoH265PictureParameterSet> pps_list{
        context.CreateH265PPS(1, 1, 1), context.CreateH265PPS(1, 1, 2), context.CreateH265PPS(1, 2, 1),
        context.CreateH265PPS(2, 1, 3), context.CreateH265PPS(2, 3, 1), context.CreateH265PPS(2, 3, 2),
        context.CreateH265PPS(2, 3, 3),
    };

    h265_ai.stdVPSCount = (uint32_t)vps_list.size();
    h265_ai.pStdVPSs = vps_list.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list.size();
    h265_ai.pStdSPSs = sps_list.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list.size();
    h265_ai.pStdPPSs = pps_list.data();

    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    std::vector<StdVideoH265VideoParameterSet> vps_list2{context.CreateH265VPS(3)};

    std::vector<StdVideoH265SequenceParameterSet> sps_list2{context.CreateH265SPS(2, 2), context.CreateH265SPS(3, 1)};

    std::vector<StdVideoH265PictureParameterSet> pps_list2{context.CreateH265PPS(1, 2, 3), context.CreateH265PPS(2, 3, 4),
                                                           context.CreateH265PPS(3, 1, 2)};

    h265_ai.stdVPSCount = (uint32_t)vps_list2.size();
    h265_ai.pStdVPSs = vps_list2.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list2.size();
    h265_ai.pStdSPSs = sps_list2.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list2.size();
    h265_ai.pStdPPSs = pps_list2.data();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07228");
    vps_list2[0].vps_video_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    vps_list2[0].vps_video_parameter_set_id = 3;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07229");
    sps_list2[0].sps_seq_parameter_set_id = 3;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list2[0].sps_seq_parameter_set_id = 2;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07230");
    pps_list2[1].pps_pic_parameter_set_id = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list2[1].pps_pic_parameter_set_id = 4;
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideo, UpdateSessionParamsDecodeH264ExceededCapacity) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - H.264 decode parameter set capacity exceeded");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigDecodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support";
    }

    VideoContext context(m_device, config);

    auto h264_ci = vku::InitStruct<VkVideoDecodeH264SessionParametersCreateInfoKHR>();
    auto h264_ai = vku::InitStruct<VkVideoDecodeH264SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h264_ci;
    create_info.videoSession = context.Session();

    h264_ci.maxStdSPSCount = 4;
    h264_ci.maxStdPPSCount = 9;
    h264_ci.pParametersAddInfo = &h264_ai;

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.pNext = &h264_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH264SequenceParameterSet> sps_list{context.CreateH264SPS(1), context.CreateH264SPS(2),
                                                           context.CreateH264SPS(3)};

    std::vector<StdVideoH264PictureParameterSet> pps_list{
        context.CreateH264PPS(1, 1), context.CreateH264PPS(1, 4), context.CreateH264PPS(2, 1),
        context.CreateH264PPS(2, 2), context.CreateH264PPS(3, 1), context.CreateH264PPS(3, 3),
    };

    h264_ai.stdSPSCount = (uint32_t)sps_list.size();
    h264_ai.pStdSPSs = sps_list.data();
    h264_ai.stdPPSCount = (uint32_t)pps_list.size();
    h264_ai.pStdPPSs = pps_list.data();

    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    std::vector<StdVideoH264SequenceParameterSet> sps_list2{context.CreateH264SPS(4), context.CreateH264SPS(5)};

    std::vector<StdVideoH264PictureParameterSet> pps_list2{context.CreateH264PPS(1, 3), context.CreateH264PPS(3, 2),
                                                           context.CreateH264PPS(4, 1), context.CreateH264PPS(5, 2)};

    h264_ai.pStdSPSs = sps_list2.data();
    h264_ai.pStdPPSs = pps_list2.data();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07217");
    h264_ai.stdSPSCount = 2;
    h264_ai.stdPPSCount = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07219");
    h264_ai.stdSPSCount = 1;
    h264_ai.stdPPSCount = 4;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideo, UpdateSessionParamsDecodeH265ExceededCapacity) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - H.265 decode parameter set capacity exceeded");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigDecodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    auto h265_ci = vku::InitStruct<VkVideoDecodeH265SessionParametersCreateInfoKHR>();
    auto h265_ai = vku::InitStruct<VkVideoDecodeH265SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h265_ci;
    create_info.videoSession = context.Session();

    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 9;
    h265_ci.pParametersAddInfo = &h265_ai;

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.pNext = &h265_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH265VideoParameterSet> vps_list{
        context.CreateH265VPS(1),
        context.CreateH265VPS(2),
    };

    std::vector<StdVideoH265SequenceParameterSet> sps_list{
        context.CreateH265SPS(1, 1),
        context.CreateH265SPS(1, 2),
        context.CreateH265SPS(2, 1),
        context.CreateH265SPS(2, 3),
    };

    std::vector<StdVideoH265PictureParameterSet> pps_list{
        context.CreateH265PPS(1, 1, 1), context.CreateH265PPS(1, 1, 2), context.CreateH265PPS(1, 2, 1),
        context.CreateH265PPS(2, 1, 3), context.CreateH265PPS(2, 3, 1), context.CreateH265PPS(2, 3, 2),
        context.CreateH265PPS(2, 3, 3),
    };

    h265_ai.stdVPSCount = (uint32_t)vps_list.size();
    h265_ai.pStdVPSs = vps_list.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list.size();
    h265_ai.pStdSPSs = sps_list.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list.size();
    h265_ai.pStdPPSs = pps_list.data();

    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    std::vector<StdVideoH265VideoParameterSet> vps_list2{context.CreateH265VPS(3)};

    std::vector<StdVideoH265SequenceParameterSet> sps_list2{context.CreateH265SPS(2, 2), context.CreateH265SPS(3, 1)};

    std::vector<StdVideoH265PictureParameterSet> pps_list2{context.CreateH265PPS(1, 2, 3), context.CreateH265PPS(2, 3, 4),
                                                           context.CreateH265PPS(3, 1, 2)};

    h265_ai.stdVPSCount = (uint32_t)vps_list2.size();
    h265_ai.pStdVPSs = vps_list2.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list2.size();
    h265_ai.pStdSPSs = sps_list2.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list2.size();
    h265_ai.pStdPPSs = pps_list2.data();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07221");
    h265_ai.stdVPSCount = 1;
    h265_ai.stdSPSCount = 1;
    h265_ai.stdPPSCount = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07223");
    h265_ai.stdVPSCount = 0;
    h265_ai.stdSPSCount = 2;
    h265_ai.stdPPSCount = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07225");
    h265_ai.stdVPSCount = 0;
    h265_ai.stdSPSCount = 1;
    h265_ai.stdPPSCount = 3;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideo, UpdateSessionParamsEncodeH264ExceededCapacity) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - H.264 encode parameter set capacity exceeded");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    VideoContext context(m_device, config);

    auto h264_ci = vku::InitStruct<VkVideoEncodeH264SessionParametersCreateInfoKHR>();
    auto h264_ai = vku::InitStruct<VkVideoEncodeH264SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h264_ci;
    create_info.videoSession = context.Session();

    h264_ci.maxStdSPSCount = 4;
    h264_ci.maxStdPPSCount = 9;
    h264_ci.pParametersAddInfo = &h264_ai;

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.pNext = &h264_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH264SequenceParameterSet> sps_list{context.CreateH264SPS(1), context.CreateH264SPS(2),
                                                           context.CreateH264SPS(3)};

    std::vector<StdVideoH264PictureParameterSet> pps_list{
        context.CreateH264PPS(1, 1), context.CreateH264PPS(1, 4), context.CreateH264PPS(2, 1),
        context.CreateH264PPS(2, 2), context.CreateH264PPS(3, 1), context.CreateH264PPS(3, 3),
    };

    h264_ai.stdSPSCount = (uint32_t)sps_list.size();
    h264_ai.pStdSPSs = sps_list.data();
    h264_ai.stdPPSCount = (uint32_t)pps_list.size();
    h264_ai.pStdPPSs = pps_list.data();

    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    std::vector<StdVideoH264SequenceParameterSet> sps_list2{context.CreateH264SPS(4), context.CreateH264SPS(5)};

    std::vector<StdVideoH264PictureParameterSet> pps_list2{context.CreateH264PPS(1, 3), context.CreateH264PPS(3, 2),
                                                           context.CreateH264PPS(4, 1), context.CreateH264PPS(5, 2)};

    h264_ai.pStdSPSs = sps_list2.data();
    h264_ai.pStdPPSs = pps_list2.data();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06441");
    h264_ai.stdSPSCount = 2;
    h264_ai.stdPPSCount = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06442");
    h264_ai.stdSPSCount = 1;
    h264_ai.stdPPSCount = 4;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideo, UpdateSessionParamsEncodeH265ExceededCapacity) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - H.265 encode parameter set capacity exceeded");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    auto h265_ci = vku::InitStruct<VkVideoEncodeH265SessionParametersCreateInfoKHR>();
    auto h265_ai = vku::InitStruct<VkVideoEncodeH265SessionParametersAddInfoKHR>();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h265_ci;
    create_info.videoSession = context.Session();

    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 9;
    h265_ci.pParametersAddInfo = &h265_ai;

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.pNext = &h265_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH265VideoParameterSet> vps_list{
        context.CreateH265VPS(1),
        context.CreateH265VPS(2),
    };

    std::vector<StdVideoH265SequenceParameterSet> sps_list{
        context.CreateH265SPS(1, 1),
        context.CreateH265SPS(1, 2),
        context.CreateH265SPS(2, 1),
        context.CreateH265SPS(2, 3),
    };

    std::vector<StdVideoH265PictureParameterSet> pps_list{
        context.CreateH265PPS(1, 1, 1), context.CreateH265PPS(1, 1, 2), context.CreateH265PPS(1, 2, 1),
        context.CreateH265PPS(2, 1, 3), context.CreateH265PPS(2, 3, 1), context.CreateH265PPS(2, 3, 2),
        context.CreateH265PPS(2, 3, 3),
    };

    h265_ai.stdVPSCount = (uint32_t)vps_list.size();
    h265_ai.pStdVPSs = vps_list.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list.size();
    h265_ai.pStdSPSs = sps_list.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list.size();
    h265_ai.pStdPPSs = pps_list.data();

    ASSERT_EQ(VK_SUCCESS, vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    std::vector<StdVideoH265VideoParameterSet> vps_list2{context.CreateH265VPS(3)};

    std::vector<StdVideoH265SequenceParameterSet> sps_list2{context.CreateH265SPS(2, 2), context.CreateH265SPS(3, 1)};

    std::vector<StdVideoH265PictureParameterSet> pps_list2{context.CreateH265PPS(1, 2, 3), context.CreateH265PPS(2, 3, 4),
                                                           context.CreateH265PPS(3, 1, 2)};

    h265_ai.stdVPSCount = (uint32_t)vps_list2.size();
    h265_ai.pStdVPSs = vps_list2.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list2.size();
    h265_ai.pStdSPSs = sps_list2.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list2.size();
    h265_ai.pStdPPSs = pps_list2.data();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06443");
    h265_ai.stdVPSCount = 1;
    h265_ai.stdSPSCount = 1;
    h265_ai.stdPPSCount = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06444");
    h265_ai.stdVPSCount = 0;
    h265_ai.stdSPSCount = 2;
    h265_ai.stdPPSCount = 2;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06445");
    h265_ai.stdVPSCount = 0;
    h265_ai.stdSPSCount = 1;
    h265_ai.stdPPSCount = 3;
    vk::UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params, nullptr);
}

TEST_F(NegativeVideo, UpdateSessionParamsEncodeAV1) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - not supported for AV1 encode");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VideoContext context(DeviceObj(), config);

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.updateSequenceCount = 1;

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-10281");
    vk::UpdateVideoSessionParametersKHR(device(), context.SessionParams(), &update_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, GetEncodedSessionParamsRequiresEncodeProfile) {
    TEST_DESCRIPTION("vkGetEncodedVideoSessionParametersKHR - video session parameters object must use encode profile");

    RETURN_IF_SKIP(Init());
    VideoConfig decode_config = GetConfigWithParams(GetConfigsDecode());
    VideoConfig encode_config = GetConfigEncode();
    if (!decode_config || !encode_config) {
        GTEST_SKIP() << "Test requires video encode support and a decode profile with session parameters";
    }

    VideoContext decode_context(m_device, decode_config);
    VideoContext encode_context(m_device, encode_config);

    auto get_info = vku::InitStruct<VkVideoEncodeSessionParametersGetInfoKHR>();
    get_info.videoSessionParameters = decode_context.SessionParams();
    size_t data_size = 0;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08359");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, GetEncodedSessionParamsMissingCodecInfo) {
    TEST_DESCRIPTION("vkGetEncodedVideoSessionParametersKHR - missing codec-specific information");

    RETURN_IF_SKIP(Init());
    if (!GetConfigEncode()) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    if (GetConfigEncodeH264()) {
        VideoContext context(m_device, GetConfigEncodeH264());

        auto get_info = vku::InitStruct<VkVideoEncodeSessionParametersGetInfoKHR>();
        get_info.videoSessionParameters = context.SessionParams();
        size_t data_size = 0;

        m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08262");
        vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigEncodeH265()) {
        VideoContext context(m_device, GetConfigEncodeH265());

        auto get_info = vku::InitStruct<VkVideoEncodeSessionParametersGetInfoKHR>();
        get_info.videoSessionParameters = context.SessionParams();
        size_t data_size = 0;

        m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08265");
        vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeVideo, GetEncodedSessionParamsH264) {
    TEST_DESCRIPTION("vkGetEncodedVideoSessionParametersKHR - H.264 specific parameters are invalid");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    VideoContext context(m_device, config);

    auto h264_info = vku::InitStruct<VkVideoEncodeH264SessionParametersGetInfoKHR>();
    auto get_info = vku::InitStruct<VkVideoEncodeSessionParametersGetInfoKHR>(&h264_info);
    get_info.videoSessionParameters = context.SessionParams();
    size_t data_size = 0;

    // Need to request writing at least one parameter set
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264SessionParametersGetInfoKHR-writeStdSPS-08279");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request non-existent SPS
    h264_info = vku::InitStructHelper();
    h264_info.writeStdSPS = VK_TRUE;
    h264_info.stdSPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08263");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request non-existent PPS
    h264_info = vku::InitStructHelper();
    h264_info.writeStdPPS = VK_TRUE;
    h264_info.stdPPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08264");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    h264_info.stdSPSId = 1;
    h264_info.stdPPSId = 0;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08264");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request both non-existent SPS and PPS
    h264_info = vku::InitStructHelper();
    h264_info.writeStdSPS = VK_TRUE;
    h264_info.writeStdPPS = VK_TRUE;
    h264_info.stdSPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08263");
    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08264");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, GetEncodedSessionParamsH265) {
    TEST_DESCRIPTION("vkGetEncodedVideoSessionParametersKHR - H.265 specific parameters are invalid");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VideoContext context(m_device, config);

    auto h265_info = vku::InitStruct<VkVideoEncodeH265SessionParametersGetInfoKHR>();
    auto get_info = vku::InitStruct<VkVideoEncodeSessionParametersGetInfoKHR>(&h265_info);
    get_info.videoSessionParameters = context.SessionParams();
    size_t data_size = 0;

    // Need to request writing at least one parameter set
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265SessionParametersGetInfoKHR-writeStdVPS-08290");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request non-existent VPS
    h265_info = vku::InitStructHelper();
    h265_info.writeStdVPS = VK_TRUE;
    h265_info.stdVPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08266");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request non-existent SPS
    h265_info = vku::InitStructHelper();
    h265_info.writeStdSPS = VK_TRUE;
    h265_info.stdSPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08267");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    h265_info.stdVPSId = 1;
    h265_info.stdSPSId = 0;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08267");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request non-existent PPS
    h265_info = vku::InitStructHelper();
    h265_info.writeStdPPS = VK_TRUE;
    h265_info.stdPPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08268");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    h265_info.stdSPSId = 1;
    h265_info.stdPPSId = 0;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08268");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    h265_info.stdVPSId = 1;
    h265_info.stdSPSId = 0;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08268");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request non-existent VPS, SPS, and PPS
    h265_info = vku::InitStructHelper();
    h265_info.writeStdVPS = VK_TRUE;
    h265_info.writeStdSPS = VK_TRUE;
    h265_info.writeStdPPS = VK_TRUE;
    h265_info.stdVPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08266");
    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08267");
    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08268");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request only non-existent SPS and PPS
    h265_info.stdVPSId = 0;
    h265_info.stdSPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08267");
    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08268");
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, BeginCodingUnsupportedCodecOp) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - unsupported video codec operation");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    uint32_t queue_family_index = VK_QUEUE_FAMILY_IGNORED;
    for (uint32_t qfi = 0; qfi < QueueFamilyCount(); ++qfi) {
        if ((QueueFamilyFlags(qfi) & (VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR)) &&
            ((QueueFamilyVideoCodecOps(qfi) & config.Profile()->videoCodecOperation) == 0)) {
            queue_family_index = qfi;
            break;
        }
    }

    if (queue_family_index == VK_QUEUE_FAMILY_IGNORED) {
        GTEST_SKIP() << "Test requires a queue family that supports video but not the specific codec op";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    vkt::CommandPool pool(*m_device, queue_family_index);
    vkt::CommandBuffer cb(*m_device, pool);

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07231");
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, BeginCodingActiveQueriesNotAllowed) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - there must be no active query");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateStatusQueryPool();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 0, 0);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-None-07232");
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->VerifyFound();
    vk::CmdEndQuery(cb.handle(), context.StatusQueryPool(), 0);
    cb.End();
}

TEST_F(NegativeVideo, BeginCodingProtectedNoFaultSession) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - protectedNoFault tests for video session");

    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(Init());
    if (IsProtectedNoFaultSupported()) {
        GTEST_SKIP() << "Test requires protectedMemory support without protectedNoFault support";
    }

    VideoConfig config = GetConfigWithProtectedContent(GetConfigs());
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with protected content support";
    }

    const bool use_protected = true;

    VideoContext unprotected_context(m_device, config);
    unprotected_context.CreateAndBindSessionMemory();
    unprotected_context.CreateResources();

    vkt::CommandBuffer& unprotected_cb = unprotected_context.CmdBuffer();

    VideoContext protected_context(m_device, config, use_protected);
    protected_context.CreateAndBindSessionMemory();
    protected_context.CreateResources(use_protected /* bitstream */, use_protected /* DPB */, use_protected /* src/dst image */);

    vkt::CommandBuffer& protected_cb = protected_context.CmdBuffer();

    unprotected_cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07233");
    unprotected_cb.BeginVideoCoding(protected_context.Begin());
    m_errorMonitor->VerifyFound();
    unprotected_cb.End();

    protected_cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07234");
    protected_cb.BeginVideoCoding(unprotected_context.Begin());
    m_errorMonitor->VerifyFound();
    protected_cb.End();
}

TEST_F(NegativeVideo, BeginCodingProtectedNoFaultSlots) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - protectedNoFault tests for reference slots");

    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(Init());
    if (IsProtectedNoFaultSupported()) {
        GTEST_SKIP() << "Test requires protectedMemory support without protectedNoFault support";
    }

    VideoConfig config = GetConfigWithProtectedContent(GetConfigsWithReferences(GetConfigs()));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with protected content and reference picture support";
    }

    const bool use_protected = true;

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext unprotected_context(m_device, config);
    unprotected_context.CreateAndBindSessionMemory();
    unprotected_context.CreateResources(false /* bitstream */, use_protected /* DPB */, false /* src/dst image */);

    vkt::CommandBuffer& unprotected_cb = unprotected_context.CmdBuffer();

    VideoContext protected_context(m_device, config, use_protected);
    protected_context.CreateAndBindSessionMemory();
    protected_context.CreateResources(use_protected /* bitstream */, false /* DPB */, use_protected /* src/dst image */);

    vkt::CommandBuffer& protected_cb = protected_context.CmdBuffer();

    unprotected_cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07235");
    unprotected_cb.BeginVideoCoding(unprotected_context.Begin().AddResource(-1, 0));
    m_errorMonitor->VerifyFound();
    unprotected_cb.End();

    protected_cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07236");
    protected_cb.BeginVideoCoding(protected_context.Begin().AddResource(-1, 0));
    m_errorMonitor->VerifyFound();
    protected_cb.End();
}

TEST_F(NegativeVideo, BeginCodingSessionMemoryNotBound) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - session memory not bound");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(m_device, config);

    uint32_t mem_req_count;
    vk::GetVideoSessionMemoryRequirementsKHR(device(), context.Session(), &mem_req_count, nullptr);
    if (mem_req_count == 0) {
        GTEST_SKIP() << "Test requires video session to need memory bindings";
    }

    std::vector<VkVideoSessionMemoryRequirementsKHR> mem_reqs(mem_req_count,
                                                              vku::InitStruct<VkVideoSessionMemoryRequirementsKHR>());
    vk::GetVideoSessionMemoryRequirementsKHR(device(), context.Session(), &mem_req_count, mem_reqs.data());

    std::vector<VkDeviceMemory> session_memory;
    for (uint32_t i = 0; i < mem_req_count; ++i) {
        // Skip binding one of the memory bindings
        if (i == mem_req_count / 2) continue;

        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
        m_device->phy().SetMemoryType(mem_reqs[i].memoryRequirements.memoryTypeBits, &alloc_info, 0);
        alloc_info.allocationSize = mem_reqs[i].memoryRequirements.size;

        VkDeviceMemory memory = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &alloc_info, nullptr, &memory));
        session_memory.push_back(memory);

        VkBindVideoSessionMemoryInfoKHR bind_info = vku::InitStructHelper();
        bind_info.memoryBindIndex = mem_reqs[i].memoryBindIndex;
        bind_info.memory = memory;
        bind_info.memoryOffset = 0;
        bind_info.memorySize = mem_reqs[i].memoryRequirements.size;

        ASSERT_EQ(VK_SUCCESS, vk::BindVideoSessionMemoryKHR(device(), context.Session(), 1, &bind_info));
    }

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSession-07237");
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->VerifyFound();

    cb.End();

    for (auto memory : session_memory) {
        vk::FreeMemory(device(), memory, nullptr);
    }
}

TEST_F(NegativeVideo, BeginCodingInvalidSessionParams) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - invalid session parameters");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigWithParams(GetConfigs());
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with session parameters";
    }

    VideoContext context1(m_device, config);
    VideoContext context2(m_device, config);
    vkt::CommandBuffer& cb = context1.CmdBuffer();

    context1.CreateAndBindSessionMemory();

    VkVideoBeginCodingInfoKHR begin_info = context1.Begin();
    begin_info.videoSessionParameters = context2.SessionParams();

    cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSessionParameters-04857");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();
    cb.End();
}

TEST_F(NegativeVideo, BeginCodingDecodeH264RequiresParams) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - H.264 decode requires session parameters");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoBeginCodingInfoKHR begin_info = context.Begin();
    begin_info.videoSessionParameters = VK_NULL_HANDLE;

    cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSession-07247");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();
    cb.End();
}

TEST_F(NegativeVideo, BeginCodingDecodeH265RequiresParams) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - H.265 decode requires session parameters");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoBeginCodingInfoKHR begin_info = context.Begin();
    begin_info.videoSessionParameters = VK_NULL_HANDLE;

    cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSession-07248");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();
    cb.End();
}

TEST_F(NegativeVideo, BeginCodingDecodeAV1RequiresParams) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - AV1 decode requires session parameters");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 decode support";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoBeginCodingInfoKHR begin_info = context.Begin();
    begin_info.videoSessionParameters = VK_NULL_HANDLE;

    cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSession-09261");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();
    cb.End();
}

TEST_F(NegativeVideo, BeginCodingEncodeH264RequiresParams) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - H.264 encode requires session parameters");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoBeginCodingInfoKHR begin_info = context.Begin();
    begin_info.videoSessionParameters = VK_NULL_HANDLE;

    cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSession-07249");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();
    cb.End();
}

TEST_F(NegativeVideo, BeginCodingEncodeH265RequiresParams) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - H.265 encode requires session parameters");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoBeginCodingInfoKHR begin_info = context.Begin();
    begin_info.videoSessionParameters = VK_NULL_HANDLE;

    cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSession-07250");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();
    cb.End();
}

TEST_F(NegativeVideo, BeginCodingEncodeAV1RequiresParams) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - AV1 encode requires session parameters");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoBeginCodingInfoKHR begin_info = context.Begin();
    begin_info.videoSessionParameters = VK_NULL_HANDLE;

    cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSession-10283");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();
    cb.End();
}

TEST_F(NegativeVideo, BeginCodingIncompatRefPicProfile) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - reference picture must be compatible with the video profile");

    RETURN_IF_SKIP(Init());

    VideoConfig configs[2] = {};
    const auto& all_configs = GetConfigs();
    for (uint32_t i = 0; i < all_configs.size(); ++i) {
        for (uint32_t j = i + 1; j < all_configs.size(); ++j) {
            const auto& coded_extent1 = all_configs[i].SessionCreateInfo()->maxCodedExtent;
            const auto& coded_extent2 = all_configs[j].SessionCreateInfo()->maxCodedExtent;
            const auto& dpb_format1 = *all_configs[i].DpbFormatProps();
            const auto& dpb_format2 = *all_configs[j].DpbFormatProps();
            if ((coded_extent1.width == coded_extent2.width) && (coded_extent1.height == coded_extent2.height) &&
                (dpb_format1.imageType == dpb_format2.imageType) && (dpb_format1.imageTiling == dpb_format2.imageTiling) &&
                (dpb_format1.format == dpb_format2.format) && (dpb_format1.imageUsageFlags == dpb_format2.imageUsageFlags)) {
                configs[0] = all_configs[i];
                configs[1] = all_configs[j];
            }
        }
    }
    if (!configs[0]) {
        GTEST_SKIP() << "Test requires two video profiles with matching DPB format/size";
    }

    for (uint32_t i = 0; i < 2; ++i) {
        configs[i].SessionCreateInfo()->maxDpbSlots = 1;
        configs[i].SessionCreateInfo()->maxActiveReferencePictures = 1;
    }

    VideoContext context1(m_device, configs[0]);
    VideoContext context2(m_device, configs[1]);
    context1.CreateAndBindSessionMemory();
    context1.CreateResources();
    context2.CreateAndBindSessionMemory();
    context2.CreateResources();

    vkt::CommandBuffer& cb = context1.CmdBuffer();

    cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07240");
    cb.BeginVideoCoding(context1.Begin().AddResource(-1, context2.Dpb()->Picture(0)));
    m_errorMonitor->VerifyFound();
    cb.End();
}

TEST_F(NegativeVideo, BeginCodingInvalidResourceLayer) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - out-of-bounds layer index in VkVideoPictureResourceInfoKHR");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigs()));
    if (!config) {
        GTEST_SKIP() << "Test requires video support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();

    // Invalid baseArrayLayer in VkVideoBeginCodingInfoKHR::pReferenceSlots
    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);
    res.baseArrayLayer = 5;

    m_errorMonitor->SetDesiredError("VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, BeginCodingDecodeSlotInactive) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - referenced DPB slot is inactive");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires a video decode profile with reference picture support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.EndVideoCoding(context.End());
    cb.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
    cb.ControlVideoCoding(context.Control().Reset());
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, 0, 1));
    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().InvalidateSlot(0));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.EndVideoCoding(context.End());
    cb.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeVideo, BeginCodingDecodeInvalidSlotResourceAssociation) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - referenced DPB slot is not associated with the specified resource");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires a video decode profile with reference picture support and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
    cb.ControlVideoCoding(context.Control().Reset());
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, 0, 1));
    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    cb.EndVideoCoding(context.End());

    cb.BeginVideoCoding(context.Begin().AddResource(0, 1));
    cb.EndVideoCoding(context.End());
    cb.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-pPictureResource-07265");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeVideo, BeginCodingInvalidSlotIndex) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - referenced DPB slot index is invalid");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigs(), 4));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with support for 4 reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 3;
    config.SessionCreateInfo()->maxActiveReferencePictures = 3;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-slotIndex-04856");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(3, 1).AddResource(-1, 2));
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, BeginCodingResourcesNotUnique) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - referenced video picture resources are not unique");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigs(), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with support for 2 reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07238");
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 0));
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, BeginCodingReferenceFormatMismatch) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - reference picture format mismatch");

    RETURN_IF_SKIP(Init());

    uint32_t alt_ref_format_index = 0;
    VideoConfig config =
        GetConfig(FilterConfigs(GetConfigsWithReferences(GetConfigs()), [&alt_ref_format_index](const VideoConfig& config) {
            const auto& format_props = config.SupportedDpbFormatProps();
            for (size_t i = 0; i < format_props.size(); ++i) {
                if (format_props[i].format != format_props[0].format && alt_ref_format_index == 0) {
                    alt_ref_format_index = i;
                    return true;
                }
            }
            return false;
        }));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with support for two reference picture formats";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoConfig config2 = config;
    config2.SetFormatProps(config.SupportedPictureFormatProps(), {config.SupportedDpbFormatProps()[alt_ref_format_index]});
    VideoDPB dpb(m_device, config2);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07241");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, dpb.Picture(0)));
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, BeginCodingInvalidCodedOffset) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - invalid coded offset");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigs()));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with reference picture support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07242");
    res.codedOffset.x = 5;
    res.codedOffset.y = 0;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07242");
    res.codedOffset.x = 0;
    res.codedOffset.y = 4;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, BeginCodingInvalidCodedExtent) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - invalid coded extent");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigs()));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with reference picture support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);

    cb.Begin();

    res.codedExtent.width = config.Caps()->minCodedExtent.width;
    res.codedExtent.height = config.Caps()->minCodedExtent.height;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    cb.EndVideoCoding(context.End());

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07243");
    res.codedExtent.width = config.Caps()->minCodedExtent.width - 1;
    res.codedExtent.height = config.Caps()->minCodedExtent.height;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07243");
    res.codedExtent.width = config.Caps()->minCodedExtent.width;
    res.codedExtent.height = config.Caps()->minCodedExtent.height - 1;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    res.codedExtent.width = config.SessionCreateInfo()->maxCodedExtent.width;
    res.codedExtent.height = config.SessionCreateInfo()->maxCodedExtent.height;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    cb.EndVideoCoding(context.End());

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07243");
    res.codedExtent.width = config.SessionCreateInfo()->maxCodedExtent.width + 1;
    res.codedExtent.height = config.SessionCreateInfo()->maxCodedExtent.height;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07243");
    res.codedExtent.width = config.SessionCreateInfo()->maxCodedExtent.width;
    res.codedExtent.height = config.SessionCreateInfo()->maxCodedExtent.height + 1;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, BeginCodingInvalidSeparateReferenceImages) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - unsupported use of separate reference images");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigs(), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with reference picture support";
    }

    if (config.Caps()->flags & VK_VIDEO_CAPABILITY_SEPARATE_REFERENCE_IMAGES_BIT_KHR) {
        GTEST_SKIP() << "This test can only run on implementations with no support for separate reference images";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoDPB separate_dpb(m_device, config);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-flags-07244");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, context.Dpb()->Picture(0)).AddResource(-1, separate_dpb.Picture(1)));
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, BeginCodingMissingDecodeDpbUsage) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - reference picture resource missing VIDEO_DECODE_DPB usage");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecode(), 1));
    if (!config) {
        GTEST_SKIP() << "Test requires a decode profile with reference picture support";
    }

    if (config.DpbFormatProps()->imageUsageFlags == VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR) {
        GTEST_SKIP() << "Test requires reference format to support at least one more usage besides DECODE_DPB";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);

    auto view_usage_ci = vku::InitStruct<VkImageViewUsageCreateInfo>();
    view_usage_ci.usage = config.DpbFormatProps()->imageUsageFlags ^ VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR;

    auto image_view_ci = vku::InitStruct<VkImageViewCreateInfo>(&view_usage_ci);
    image_view_ci.image = context.Dpb()->Image();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    image_view_ci.format = config.DpbFormatProps()->format;
    image_view_ci.components = config.DpbFormatProps()->componentMapping;
    image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    vkt::ImageView image_view(*m_device, image_view_ci);

    res.imageViewBinding = image_view.handle();

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-slotIndex-07245");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, BeginCodingMissingEncodeDpbUsage) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - reference picture resource missing VIDEO_ENCODE_DPB usage");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsEncode(), 1));
    if (!config) {
        GTEST_SKIP() << "Test requires an encode profile with reference picture support";
    }

    if (config.DpbFormatProps()->imageUsageFlags == VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR) {
        GTEST_SKIP() << "Test requires reference format to support at least one more usage besides ENCODE_DPB";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);

    auto view_usage_ci = vku::InitStruct<VkImageViewUsageCreateInfo>();
    view_usage_ci.usage = config.DpbFormatProps()->imageUsageFlags ^ VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR;

    auto image_view_ci = vku::InitStruct<VkImageViewCreateInfo>(&view_usage_ci);
    image_view_ci.image = context.Dpb()->Image();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    image_view_ci.format = config.DpbFormatProps()->format;
    image_view_ci.components = config.DpbFormatProps()->componentMapping;
    image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    vkt::ImageView image_view(*m_device, image_view_ci);

    res.imageViewBinding = image_view.handle();

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-slotIndex-07246");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, BeginCodingEncodeH264MissingGopRemainingFrames) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - Encode H.264 useGopRemainingFrames missing when required");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeH264()), [](const VideoConfig& config) {
        return config.EncodeCapsH264()->requiresGopRemainingFrames == VK_TRUE;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires an H.264 encode profile with rate control and requiresGopRemainingFrames == VK_TRUE";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto gop_remaining_frames_info = vku::InitStruct<VkVideoEncodeH264GopRemainingFrameInfoKHR>();

    auto rc_layer = vku::InitStruct<VkVideoEncodeRateControlLayerInfoKHR>();
    rc_layer.averageBitrate = 32000;
    rc_layer.maxBitrate = 32000;
    rc_layer.frameRateNumerator = 15;
    rc_layer.frameRateDenominator = 1;

    auto rc_info_h264 = vku::InitStruct<VkVideoEncodeH264RateControlInfoKHR>();
    rc_info_h264.flags = VK_VIDEO_ENCODE_H264_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_info_h264.gopFrameCount = 15;

    auto rc_info = vku::InitStruct<VkVideoEncodeRateControlInfoKHR>(&rc_info_h264);
    rc_info.rateControlMode = config.GetAnySupportedRateControlMode();
    rc_info.layerCount = 1;
    rc_info.pLayers = &rc_layer;
    rc_info.virtualBufferSizeInMs = 1000;
    rc_info.initialVirtualBufferSizeInMs = 0;

    VkVideoBeginCodingInfoKHR begin_info = context.Begin();
    begin_info.pNext = &rc_info;

    cb.Begin();

    // Test with VkVideoEncodeH264GopRemainingFrameInfoKHR missing from pNext chain
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-pBeginInfo-08255");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();

    // Test with useGopRemainingFrames set to VK_FALSE
    gop_remaining_frames_info.useGopRemainingFrames = VK_FALSE;
    gop_remaining_frames_info.pNext = begin_info.pNext;
    begin_info.pNext = &gop_remaining_frames_info;
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-pBeginInfo-08255");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, BeginCodingEncodeH265MissingGopRemainingFrames) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - Encode H.265 useGopRemainingFrames missing when required");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeH265()), [](const VideoConfig& config) {
        return config.EncodeCapsH265()->requiresGopRemainingFrames == VK_TRUE;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires an H.265 encode profile with rate control and requiresGopRemainingFrames == VK_TRUE";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto gop_remaining_frames_info = vku::InitStruct<VkVideoEncodeH265GopRemainingFrameInfoKHR>();

    auto rc_layer = vku::InitStruct<VkVideoEncodeRateControlLayerInfoKHR>();
    rc_layer.averageBitrate = 32000;
    rc_layer.maxBitrate = 32000;
    rc_layer.frameRateNumerator = 15;
    rc_layer.frameRateDenominator = 1;

    auto rc_info_h265 = vku::InitStruct<VkVideoEncodeH265RateControlInfoKHR>();
    rc_info_h265.flags = VK_VIDEO_ENCODE_H264_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_info_h265.gopFrameCount = 15;

    auto rc_info = vku::InitStruct<VkVideoEncodeRateControlInfoKHR>(&rc_info_h265);
    rc_info.rateControlMode = config.GetAnySupportedRateControlMode();
    rc_info.layerCount = 1;
    rc_info.pLayers = &rc_layer;
    rc_info.virtualBufferSizeInMs = 1000;
    rc_info.initialVirtualBufferSizeInMs = 0;

    VkVideoBeginCodingInfoKHR begin_info = context.Begin();
    begin_info.pNext = &rc_info;

    cb.Begin();

    // Test with VkVideoEncodeH265GopRemainingFrameInfoKHR missing from pNext chain
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-pBeginInfo-08256");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();

    // Test with useGopRemainingFrames set to VK_FALSE
    gop_remaining_frames_info.useGopRemainingFrames = VK_FALSE;
    gop_remaining_frames_info.pNext = begin_info.pNext;
    begin_info.pNext = &gop_remaining_frames_info;
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-pBeginInfo-08256");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, BeginCodingEncodeAV1MissingGopRemainingFrames) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - Encode AV1 useGopRemainingFrames missing when required");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeAV1()), [](const VideoConfig& config) {
        return config.EncodeCapsAV1()->requiresGopRemainingFrames == VK_TRUE;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires an AV1 encode profile with rate control and requiresGopRemainingFrames == VK_TRUE";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto gop_remaining_frames_info = vku::InitStruct<VkVideoEncodeAV1GopRemainingFrameInfoKHR>();

    auto rc_layer = vku::InitStruct<VkVideoEncodeRateControlLayerInfoKHR>();
    rc_layer.averageBitrate = 32000;
    rc_layer.maxBitrate = 32000;
    rc_layer.frameRateNumerator = 15;
    rc_layer.frameRateDenominator = 1;

    auto rc_info_av1 = vku::InitStruct<VkVideoEncodeAV1RateControlInfoKHR>();
    rc_info_av1.flags = VK_VIDEO_ENCODE_H264_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_info_av1.gopFrameCount = 15;

    auto rc_info = vku::InitStruct<VkVideoEncodeRateControlInfoKHR>(&rc_info_av1);
    rc_info.rateControlMode = config.GetAnySupportedRateControlMode();
    rc_info.layerCount = 1;
    rc_info.pLayers = &rc_layer;
    rc_info.virtualBufferSizeInMs = 1000;
    rc_info.initialVirtualBufferSizeInMs = 0;

    VkVideoBeginCodingInfoKHR begin_info = context.Begin();
    begin_info.pNext = &rc_info;

    cb.Begin();

    // Test with VkVideoEncodeAV1GopRemainingFrameInfoKHR missing from pNext chain
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-pBeginInfo-10282");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();

    // Test with useGopRemainingFrames set to VK_FALSE
    gop_remaining_frames_info.useGopRemainingFrames = VK_FALSE;
    gop_remaining_frames_info.pNext = begin_info.pNext;
    begin_info.pNext = &gop_remaining_frames_info;
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-pBeginInfo-10282");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, EndCodingActiveQueriesNotAllowed) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - there must be no active query");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateStatusQueryPool();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 0, 0);
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndVideoCodingKHR-None-07251");
    cb.EndVideoCoding(context.End());
    m_errorMonitor->VerifyFound();
    vk::CmdEndQuery(cb.handle(), context.StatusQueryPool(), 0);
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, ControlSessionUninitialized) {
    TEST_DESCRIPTION("vkCmdControlVideoCodingKHR - session uninitialized");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = vku::InitStruct<VkVideoEncodeRateControlInfoKHR>();
    auto control_info = vku::InitStruct<VkVideoCodingControlInfoKHR>(&rc_info);
    control_info.flags = VK_VIDEO_CODING_CONTROL_ENCODE_RATE_CONTROL_BIT_KHR;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(control_info);
    cb.EndVideoCoding(context.End());
    cb.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdControlVideoCodingKHR-flags-07017");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();

    m_device->Wait();
}

TEST_F(NegativeVideo, EncodeQualityLevelControlInvalidQualityLevel) {
    TEST_DESCRIPTION("vkCmdControlVideoCodingKHR - invalid encode quality level");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto quality_level_info = vku::InitStruct<VkVideoEncodeQualityLevelInfoKHR>();
    quality_level_info.qualityLevel = config.EncodeCaps()->maxQualityLevels;
    auto control_info = vku::InitStruct<VkVideoCodingControlInfoKHR>(&quality_level_info);
    control_info.flags = VK_VIDEO_CODING_CONTROL_ENCODE_QUALITY_LEVEL_BIT_KHR;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeQualityLevelInfoKHR-qualityLevel-08311");
    cb.ControlVideoCoding(control_info);
    m_errorMonitor->VerifyFound();
    cb.EndVideoCoding(context.End());
    cb.End();

    m_device->Wait();
}

TEST_F(NegativeVideo, EncodeQualityLevelControlRequiresEncodeSession) {
    TEST_DESCRIPTION("vkCmdControlVideoCodingKHR - encode quality level control requires an encode session");

    RETURN_IF_SKIP(Init());

    VideoConfig encode_config = GetConfigEncode();
    VideoConfig decode_config = GetConfigDecode();
    if (!encode_config || !decode_config) {
        GTEST_SKIP() << "Test requires video decode and encode support";
    }

    VideoContext context(m_device, decode_config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto control_info = vku::InitStruct<VkVideoCodingControlInfoKHR>();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    control_info.flags = VK_VIDEO_CODING_CONTROL_ENCODE_QUALITY_LEVEL_BIT_KHR;
    m_errorMonitor->SetDesiredError("VUID-vkCmdControlVideoCodingKHR-pCodingControlInfo-08243");
    cb.ControlVideoCoding(control_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeQualityLevelControlMissingChain) {
    TEST_DESCRIPTION("vkCmdControlVideoCodingKHR - missing encode quality level info");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto control_info = vku::InitStruct<VkVideoCodingControlInfoKHR>();
    control_info.flags = VK_VIDEO_CODING_CONTROL_ENCODE_QUALITY_LEVEL_BIT_KHR;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->SetDesiredError("VUID-VkVideoCodingControlInfoKHR-flags-08349");
    cb.ControlVideoCoding(control_info);
    m_errorMonitor->VerifyFound();
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeParamsQualityLevelMismatch) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - quality level of bound parameters object does not match current quality level");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigWithMultiEncodeQualityLevelParams(GetConfigsEncode());
    if (!config) {
        GTEST_SKIP() << "Test requires an encode profile with support for parameters objects and at least two quality levels";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoSessionParametersKHR params_quality_level_1;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    auto quality_level_info = vku::InitStruct<VkVideoEncodeQualityLevelInfoKHR>();
    quality_level_info.pNext = create_info.pNext;
    quality_level_info.qualityLevel = 1;
    create_info.pNext = &quality_level_info;
    create_info.videoSession = context.Session();

    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params_quality_level_1);

    // Submitting command buffer using mismatching parameters object encode quality level is fine as long as
    // there is no encode operation recorded
    cb.Begin();
    cb.BeginVideoCoding(context.Begin().SetSessionParams(params_quality_level_1));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    context.Queue().Wait();

    // Submitting command buffer with matching parameters object encode quality level is fine even if the
    // quality is modified afterwards.
    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.EncodeVideo(context.EncodeFrame());
    cb.ControlVideoCoding(context.Control().EncodeQualityLevel(1));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    context.Queue().Wait();

    // Submitting command buffer with matching parameters object encode quality level is fine, even if state
    // is not set here
    cb.Begin();
    cb.BeginVideoCoding(context.Begin().SetSessionParams(params_quality_level_1));
    cb.EncodeVideo(context.EncodeFrame());
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    context.Queue().Wait();

    // Submitting command buffer with mismatching parameters object is fine if the quality level is modified
    // to the right one before the encode operation
    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().EncodeQualityLevel(0));
    cb.EncodeVideo(context.EncodeFrame());
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    context.Queue().Wait();

    // Submitting an encode operation with mismatching effective and parameters encode quality levels should fail
    cb.Begin();
    cb.BeginVideoCoding(context.Begin().SetSessionParams(params_quality_level_1));
    cb.EncodeVideo(context.EncodeFrame());
    cb.EndVideoCoding(context.End());
    cb.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-None-08318");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();

    // Same goes when encode quality level state is changed before/after
    // First test cases where command buffer recording time validation is possible
    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.EncodeVideo(context.EncodeFrame());
    cb.ControlVideoCoding(context.Control().EncodeQualityLevel(1));

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-None-08318");
    cb.EncodeVideo(context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    cb.ControlVideoCoding(context.Control().EncodeQualityLevel(0));
    cb.EncodeVideo(context.EncodeFrame());
    cb.EndVideoCoding(context.End());

    cb.BeginVideoCoding(context.Begin().SetSessionParams(params_quality_level_1));
    cb.EncodeVideo(context.EncodeFrame());  // This would only generate an error at submit time
    cb.ControlVideoCoding(context.Control().EncodeQualityLevel(1));
    cb.EncodeVideo(context.EncodeFrame());
    cb.ControlVideoCoding(context.Control().EncodeQualityLevel(0));

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-None-08318");
    cb.EncodeVideo(context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();

    // Then test a case where only submit time validation is possible
    cb.Begin();
    cb.BeginVideoCoding(context.Begin().SetSessionParams(params_quality_level_1));
    cb.EncodeVideo(context.EncodeFrame());
    cb.EndVideoCoding(context.End());
    cb.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-None-08318");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();

    vk::DestroyVideoSessionParametersKHR(device(), params_quality_level_1, nullptr);
}

TEST_F(NegativeVideo, EncodeQuantMapTypeMismatch) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - quantization map type specified at encode time does not match video session");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig base_config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR) &&
               (config.SupportedQuantDeltaMapProps().size() > 0) &&
               (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR) &&
               (config.SupportedEmphasisMapProps().size() > 0);
    }));
    if (!base_config) {
        GTEST_SKIP() << "Test requires an encode profile with support for both QUANTIZATION_DELTA and EMPHASIS maps";
    }

    struct TestConfig {
        VkVideoEncodeFlagBitsKHR encode_flag;
        VkVideoSessionCreateFlagBitsKHR session_create_flag;
        const VkVideoFormatPropertiesKHR* map_props;
        const char* vuid;
    };

    const std::vector<TestConfig> tests = {
        {
            VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
            VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
            base_config.EmphasisMapProps(),
            "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10311",
        },
        {VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR, VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR,
         base_config.QuantDeltaMapProps(), "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10312"},
    };

    for (const auto& test : tests) {
        VideoConfig config = base_config;
        config.SessionCreateInfo()->flags |= test.session_create_flag;
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        const auto texel_size = config.GetQuantMapTexelSize(*test.map_props);
        auto params = context.CreateSessionParamsWithQuantMapTexelSize(texel_size);

        VideoEncodeQuantizationMap quantization_map(m_device, config, *test.map_props);

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().SetSessionParams(params));

        if (test.encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        m_errorMonitor->SetDesiredError(test.vuid);
        cb.EncodeVideo(context.EncodeFrame().QuantizationMap(test.encode_flag, texel_size, quantization_map));
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }
}

TEST_F(NegativeVideo, EncodeMissingQuantMapUsage) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - quantization map image usage corresponding to encode flag is missing");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    if ((!delta_config ||
         delta_config.QuantDeltaMapProps()->imageUsageFlags == VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR) &&
        (!emphasis_config ||
         emphasis_config.EmphasisMapProps()->imageUsageFlags == VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR)) {
        GTEST_SKIP() << "Test requires quantization map format to support at least one more usage besides quantization map";
    }

    struct TestConfig {
        VideoConfig config;
        VkVideoEncodeFlagBitsKHR encode_flag;
        VkVideoSessionCreateFlagBitsKHR session_create_flag;
        VkImageUsageFlagBits image_usage;
        const VkVideoFormatPropertiesKHR* map_props;
        const char* vs_create_vuid;
    };

    std::vector<TestConfig> tests = {};
    if (delta_config &&
        delta_config.QuantDeltaMapProps()->imageUsageFlags != VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR) {
        tests.push_back({delta_config, VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
                         VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                         VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR, delta_config.QuantDeltaMapProps(),
                         "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10311"});
    }
    if (emphasis_config &&
        emphasis_config.EmphasisMapProps()->imageUsageFlags != VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR) {
        tests.push_back({delta_config, VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR,
                         VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR,
                         VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR, delta_config.EmphasisMapProps(),
                         "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10312"});
    }

    for (const auto& test : tests) {
        VideoConfig config = test.config;

        config.SessionCreateInfo()->flags |= test.session_create_flag;
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        const auto texel_size = config.GetQuantMapTexelSize(*test.map_props);
        auto params = context.CreateSessionParamsWithQuantMapTexelSize(texel_size);

        VideoEncodeQuantizationMap quantization_map(m_device, config, *test.map_props);

        auto view_usage_ci = vku::InitStruct<VkImageViewUsageCreateInfo>();
        view_usage_ci.usage = test.map_props->imageUsageFlags ^ test.image_usage;

        auto image_view_ci = vku::InitStruct<VkImageViewCreateInfo>(&view_usage_ci);
        image_view_ci.image = quantization_map.Image();
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        image_view_ci.format = test.map_props->format;
        image_view_ci.components = test.map_props->componentMapping;
        image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

        vkt::ImageView image_view(*m_device, image_view_ci);

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().SetSessionParams(params));

        if (test.encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        m_errorMonitor->SetDesiredError(test.vs_create_vuid);
        cb.EncodeVideo(context.EncodeFrame().QuantizationMap(test.encode_flag, texel_size, image_view));
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (!delta_config ||
        delta_config.QuantDeltaMapProps()->imageUsageFlags == VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR ||
        !emphasis_config ||
        emphasis_config.EmphasisMapProps()->imageUsageFlags == VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideo, EncodeIncompatQuantMapProfile) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - quantization map must be compatible with the video profile");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    auto delta_configs = FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR) &&
               (config.SupportedQuantDeltaMapProps().size() > 0);
    });
    auto emphasis_configs = FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR) &&
               (config.SupportedEmphasisMapProps().size() > 0);
    });

    if (delta_configs.size() < 2 && emphasis_configs.size() < 2) {
        GTEST_SKIP()
            << "Test requires at least two profiles that support QUANTIZATION_DELTA, or two profiles that support EMPHASIS";
    }

    struct TestConfig {
        VideoConfig configs[2];
        const VkVideoFormatPropertiesKHR* map_props[2];
        VkVideoEncodeFlagBitsKHR encode_flag;
        VkVideoSessionCreateFlagBitsKHR session_create_flag;
    };

    std::vector<TestConfig> tests;
    if (delta_configs.size() >= 2) {
        tests.emplace_back(TestConfig{{delta_configs[0], delta_configs[1]},
                                      {
                                          delta_configs[0].QuantDeltaMapProps(),
                                          delta_configs[1].QuantDeltaMapProps(),
                                      },
                                      VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR});
    }
    if (emphasis_configs.size() >= 2) {
        tests.emplace_back(TestConfig{{emphasis_configs[0], emphasis_configs[1]},
                                      {
                                          emphasis_configs[0].EmphasisMapProps(),
                                          emphasis_configs[1].EmphasisMapProps(),
                                      },
                                      VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR});
    }

    for (auto& [configs, map_props, encode_flag, session_create_flag] : tests) {
        configs[0].SessionCreateInfo()->flags |= session_create_flag;
        VideoContext context(m_device, configs[0]);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        const VkExtent2D texel_sizes[2] = {configs[0].GetQuantMapTexelSize(*map_props[0]),
                                           configs[1].GetQuantMapTexelSize(*map_props[1])};
        auto params = context.CreateSessionParamsWithQuantMapTexelSize(texel_sizes[0]);

        VideoEncodeQuantizationMap quantization_map(m_device, configs[1], *map_props[1]);

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().SetSessionParams(params));

        if (encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        if (texel_sizes[0].width != texel_sizes[1].width || texel_sizes[0].height != texel_sizes[1].height) {
            // If the two profile's texel sizes do not match, then we may have additional VUs triggered
            // related to the texel sizes and extents of the quantization map
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pNext-10316");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeQuantizationMapInfoKHR-quantizationMapExtent-10352");
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeQuantizationMapInfoKHR-quantizationMapExtent-10353");
        }

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10310");
        cb.EncodeVideo(context.EncodeFrame().QuantizationMap(encode_flag, texel_sizes[1], quantization_map));
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (delta_configs.size() < 2 || emphasis_configs.size() < 2) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideo, EncodeQuantMapNotAllowed) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - quantization map flag is passed against session that was not created to allow it");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    struct TestConfig {
        VideoConfig config;
        VkVideoEncodeFlagBitsKHR encode_flag;
        const char* vuid;
    };

    std::vector<TestConfig> tests;
    if (delta_config) {
        tests.emplace_back(TestConfig{delta_config, VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10306"});
    }
    if (emphasis_config) {
        tests.emplace_back(
            TestConfig{emphasis_config, VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR, "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10307"});
    }

    for (const auto& [config, encode_flag, vuid] : tests) {
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        auto params = context.CreateSessionParams();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        VideoEncodeInfo encode_info = context.EncodeFrame();
        encode_info->flags |= encode_flag;

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().SetSessionParams(params));

        if (encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        // VkVideoEncodeQuantizationMapInfoKHR is not chained so we allow related errors
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10309");
        m_errorMonitor->SetDesiredError(vuid);
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideo, EncodeQuantMapMissing) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - quantization map missing");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    struct TestConfig {
        VideoConfig config;
        VkVideoEncodeFlagBitsKHR encode_flag;
        VkVideoSessionCreateFlagBitsKHR session_create_flag;
        const VkVideoFormatPropertiesKHR* map_props;
    };

    std::vector<TestConfig> tests;
    if (delta_config) {
        tests.emplace_back(TestConfig{delta_config, VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      delta_config.QuantDeltaMapProps()});
    }
    if (emphasis_config) {
        tests.emplace_back(TestConfig{emphasis_config, VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                      emphasis_config.EmphasisMapProps()});
    }

    for (auto& [config, encode_flag, session_create_flag, map_props] : tests) {
        config.SessionCreateInfo()->flags |= session_create_flag;
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        const auto texel_size = config.GetQuantMapTexelSize(*map_props);
        auto params = context.CreateSessionParamsWithQuantMapTexelSize(texel_size);

        vkt::CommandBuffer& cb = context.CmdBuffer();

        VideoEncodeInfo encode_info = context.EncodeFrame();
        encode_info->flags |= encode_flag;

        // No VkVideoEncodeQuantizationMapInfoKHR in pNext
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().SetSessionParams(params));

        if (encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10309");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        VkVideoEncodeQuantizationMapInfoKHR quantization_map_info = vku::InitStructHelper();
        quantization_map_info.quantizationMapExtent.width =
            (encode_info->srcPictureResource.codedExtent.width + texel_size.width - 1) / texel_size.width;
        quantization_map_info.quantizationMapExtent.height =
            (encode_info->srcPictureResource.codedExtent.height + texel_size.height - 1) / texel_size.height;
        quantization_map_info.pNext = encode_info->pNext;
        encode_info->pNext = &quantization_map_info;

        // VkVideoEncodeQuantizationMapInfoKHR::quantizationMap = VK_NULL_HANDLE
        quantization_map_info.quantizationMap = VK_NULL_HANDLE;

        if (encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10309");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideo, EncodeParamsNotQuantMapCompatible) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - video session parameters object is not quantization map compatible");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    struct TestConfig {
        VideoConfig config;
        VkVideoEncodeFlagBitsKHR encode_flag;
        VkVideoSessionCreateFlagBitsKHR session_create_flag;
        const VkVideoFormatPropertiesKHR* map_props;
    };

    std::vector<TestConfig> tests;
    if (delta_config) {
        tests.emplace_back(TestConfig{delta_config, VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      delta_config.QuantDeltaMapProps()});
    }
    if (emphasis_config) {
        tests.emplace_back(TestConfig{emphasis_config, VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                      emphasis_config.EmphasisMapProps()});
    }

    for (auto& [config, encode_flag, session_create_flag, map_props] : tests) {
        config.SessionCreateInfo()->flags |= session_create_flag;
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        VideoEncodeQuantizationMap quantization_map(m_device, config, *map_props);

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin());

        if (encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-10315");
        cb.EncodeVideo(
            context.EncodeFrame().QuantizationMap(encode_flag, config.GetQuantMapTexelSize(*map_props), quantization_map));
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideo, EncodeQuantMapExtentCodedExtentMismatch) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - quantizationMapExtent inconsistent with the coded extent");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    struct TestConfig {
        VideoConfig config;
        VkVideoEncodeFlagBitsKHR encode_flag;
        VkVideoSessionCreateFlagBitsKHR session_create_flag;
        const VkVideoFormatPropertiesKHR* map_props;
    };

    std::vector<TestConfig> tests;
    if (delta_config) {
        tests.emplace_back(TestConfig{delta_config, VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      delta_config.QuantDeltaMapProps()});
    }
    if (emphasis_config) {
        tests.emplace_back(TestConfig{emphasis_config, VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                      emphasis_config.EmphasisMapProps()});
    }

    for (auto& [config, encode_flag, session_create_flag, map_props] : tests) {
        config.SessionCreateInfo()->flags |= session_create_flag;
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        const auto texel_size = config.GetQuantMapTexelSize(*map_props);
        auto params = context.CreateSessionParamsWithQuantMapTexelSize(texel_size);

        vkt::CommandBuffer& cb = context.CmdBuffer();

        VideoEncodeQuantizationMap quantization_map(m_device, config, *map_props);
        VideoEncodeInfo encode_info = context.EncodeFrame().QuantizationMap(encode_flag, texel_size, quantization_map);

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().SetSessionParams(params));

        if (encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        encode_info.QuantizationMapInfo().quantizationMapExtent.width -= 1;
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-10316");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
        encode_info.QuantizationMapInfo().quantizationMapExtent.width += 1;

        if (encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        encode_info.QuantizationMapInfo().quantizationMapExtent.height -= 1;
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-10316");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
        encode_info.QuantizationMapInfo().quantizationMapExtent.height += 1;

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideo, EncodeQuantMapExtentViewExtentMismatch) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - quantizationMapExtent inconsistent with the image view extent");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    struct TestConfig {
        VideoConfig config;
        VkVideoEncodeFlagBitsKHR encode_flag;
        VkVideoSessionCreateFlagBitsKHR session_create_flag;
        const VkVideoFormatPropertiesKHR* map_props;
    };

    std::vector<TestConfig> tests;
    if (delta_config) {
        tests.emplace_back(TestConfig{delta_config, VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      delta_config.QuantDeltaMapProps()});
    }
    if (emphasis_config) {
        tests.emplace_back(TestConfig{emphasis_config, VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                      emphasis_config.EmphasisMapProps()});
    }

    for (auto& [config, encode_flag, session_create_flag, map_props] : tests) {
        config.SessionCreateInfo()->flags |= session_create_flag;
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        const auto texel_size = config.GetQuantMapTexelSize(*map_props);
        auto params = context.CreateSessionParamsWithQuantMapTexelSize(texel_size);

        vkt::CommandBuffer& cb = context.CmdBuffer();

        VideoEncodeQuantizationMap quantization_map(m_device, config, *map_props);
        VideoEncodeInfo encode_info = context.EncodeFrame().QuantizationMap(encode_flag, texel_size, quantization_map);

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().SetSessionParams(params));

        // quantizationMapExtent is not consistent with the codedExtent
        if (encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        // This will certainly cause quantizationMapExtent to have a mismatch with codedExtent
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pNext-10316");

        encode_info.QuantizationMapInfo().quantizationMapExtent.width += 1;
        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeQuantizationMapInfoKHR-quantizationMapExtent-10352");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
        encode_info.QuantizationMapInfo().quantizationMapExtent.width -= 1;

        if (encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        // This will certainly cause quantizationMapExtent to have a mismatch with codedExtent
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pNext-10316");

        encode_info.QuantizationMapInfo().quantizationMapExtent.height += 1;
        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeQuantizationMapInfoKHR-quantizationMapExtent-10353");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
        encode_info.QuantizationMapInfo().quantizationMapExtent.height -= 1;

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideo, EncodeWithEmphasisMapIncompatibleRateControlMode) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - emphasis map used with incompatible rate control mode");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig config =
        GetConfigWithEmphasisMap(GetConfigsWithRateControl(GetConfigsEncode(), VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR));
    if (!config) {
        // If DISABLED is not supported, just use any encode config with emphasis map support available
        config = GetConfigWithEmphasisMap(GetConfigsEncode());
    }
    if (!config) {
        GTEST_SKIP() << "Test requires emphasis map support";
    }

    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR;
    VideoContext context(m_device, config);

    const auto texel_size = config.EmphasisMapTexelSize();
    auto params = context.CreateSessionParamsWithQuantMapTexelSize(texel_size);

    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoEncodeQuantizationMap quantization_map(m_device, config, *config.EmphasisMapProps());

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().SetSessionParams(params));

    // Test with DEFAULT rate control mode
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
    cb.EncodeVideo(context.EncodeFrame().QuantizationMap(VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR, texel_size, quantization_map));
    m_errorMonitor->VerifyFound();

    // If supported, also test with DISABLED rate control mode
    if (config.EncodeCaps()->rateControlModes & VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR) {
        auto rc_info = VideoEncodeRateControlInfo(config);
        rc_info->rateControlMode = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR;
        cb.ControlVideoCoding(context.Control().RateControl(rc_info));

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        cb.EncodeVideo(
            context.EncodeFrame().QuantizationMap(VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR, texel_size, quantization_map));
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeQuantMapImageLayout) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - quantization map is not in the correct image layout at execution time");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    struct TestConfig {
        VkVideoEncodeFlagBitsKHR encode_flag;
        VkVideoSessionCreateFlagBitsKHR session_create_flag;
        VideoConfig config;
        const VkVideoFormatPropertiesKHR* map_props;
    };

    std::vector<TestConfig> tests;
    if (delta_config) {
        tests.emplace_back(TestConfig{VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR, delta_config,
                                      delta_config.QuantDeltaMapProps()});
    }
    if (emphasis_config) {
        tests.emplace_back(TestConfig{VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR, emphasis_config,
                                      emphasis_config.EmphasisMapProps()});
    }

    for (const auto& test : tests) {
        VideoConfig config = test.config;

        config.SessionCreateInfo()->flags |= test.session_create_flag;
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        const auto texel_size = config.GetQuantMapTexelSize(*test.map_props);
        auto params = context.CreateSessionParamsWithQuantMapTexelSize(texel_size);

        VideoEncodeQuantizationMap quantization_map(m_device, config, *test.map_props);

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        // quantization map must be in VK_IMAGE_LAYOUT_ENCODE_QUANTIZATION_MAP_KHR
        vk::CmdPipelineBarrier2KHR(cb.handle(), quantization_map.LayoutTransition(VK_IMAGE_LAYOUT_GENERAL, 0, 1));
        cb.BeginVideoCoding(context.Begin().SetSessionParams(params));

        if (test.encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-10314");
        cb.EncodeVideo(context.EncodeFrame().QuantizationMap(test.encode_flag, texel_size, quantization_map));
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideo, EncodeRateControlRequiresEncodeSession) {
    TEST_DESCRIPTION("vkCmdControlVideoCodingKHR - rate control requires an encode session");

    RETURN_IF_SKIP(Init());

    VideoConfig encode_config = GetConfigEncode();
    VideoConfig decode_config = GetConfigDecode();
    if (!encode_config || !decode_config) {
        GTEST_SKIP() << "Test requires video decode and encode support";
    }

    VideoContext context(m_device, decode_config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto control_info = vku::InitStruct<VkVideoCodingControlInfoKHR>();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    control_info.flags = VK_VIDEO_CODING_CONTROL_ENCODE_RATE_CONTROL_BIT_KHR;
    m_errorMonitor->SetDesiredError("VUID-vkCmdControlVideoCodingKHR-pCodingControlInfo-08243");
    cb.ControlVideoCoding(control_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlUnsupportedMode) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - rate control mode is not supported");

    RETURN_IF_SKIP(Init());

    // Try to find a config that does not support all rate control modes
    VkVideoEncodeRateControlModeFlagsKHR all_rc_modes = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR |
                                                        VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR |
                                                        VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR;
    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [all_rc_modes](const VideoConfig& config) {
        return (config.EncodeCaps()->rateControlModes & all_rc_modes) < all_rc_modes;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires an encode profile that does not support all rate control modes";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto unsupported_rc_modes = (config.EncodeCaps()->rateControlModes & all_rc_modes) ^ all_rc_modes;

    auto rc_info = VideoEncodeRateControlInfo(config);
    rc_info->rateControlMode =
        static_cast<VkVideoEncodeRateControlModeFlagBitsKHR>(1 << static_cast<uint32_t>(log2(unsupported_rc_modes)));
    if (rc_info->rateControlMode != VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR) {
        rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    }

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08244");
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08244");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlTooManyLayers) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - rate control layer count exceeds maximum supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config).SetAnyMode();
    for (uint32_t i = 0; i <= config.EncodeCaps()->maxRateControlLayers; ++i) {
        rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    }

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-layerCount-08245");
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-layerCount-08245");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlNoLayers) {
    TEST_DESCRIPTION(
        "vkCmdBegin/ControlVideoCodingKHR - no rate control layers are allowed when "
        "rate control mode is DEFAULT or DISABLED");

    RETURN_IF_SKIP(Init());

    VideoConfig config =
        GetConfig(GetConfigsWithRateControl(GetConfigsEncode(), VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR));
    if (!config) {
        // If DISABLED is not supported, just use any encode config available
        config = GetConfigEncode();
    }
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config);
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));

    std::vector<VkVideoEncodeRateControlModeFlagBitsKHR> rate_control_modes = {VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DEFAULT_KHR,
                                                                               VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR};

    for (auto rate_control_mode : rate_control_modes) {
        if (config.SupportsRateControlMode(rate_control_mode)) {
            rc_info->rateControlMode = rate_control_mode;

            cb.Begin();

            m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08248");
            cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
            m_errorMonitor->VerifyFound();

            cb.BeginVideoCoding(context.Begin());

            m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08248");
            cb.ControlVideoCoding(context.Control().RateControl(rc_info));
            m_errorMonitor->VerifyFound();

            cb.EndVideoCoding(context.End());
            cb.End();
        }
    }
}

TEST_F(NegativeVideo, EncodeRateControlMissingLayers) {
    TEST_DESCRIPTION(
        "vkCmdBegin/ControlVideoCodingKHR - rate control layers are required when "
        "rate control mode is CBR or VBR");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config);

    std::vector<VkVideoEncodeRateControlModeFlagBitsKHR> rate_control_modes = {VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR,
                                                                               VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR};

    for (auto rate_control_mode : rate_control_modes) {
        if (config.SupportsRateControlMode(rate_control_mode)) {
            rc_info->rateControlMode = rate_control_mode;

            cb.Begin();

            m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08275");
            cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
            m_errorMonitor->VerifyFound();

            cb.BeginVideoCoding(context.Begin());

            m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08275");
            cb.ControlVideoCoding(context.Control().RateControl(rc_info));
            m_errorMonitor->VerifyFound();

            cb.EndVideoCoding(context.End());
            cb.End();
        }
    }
}

TEST_F(NegativeVideo, EncodeRateControlLayerBitrate) {
    TEST_DESCRIPTION("vkCmdControlVideoCodingKHR - test incorrect values for averageBitrate and maxBitrate");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    auto rc_info = VideoEncodeRateControlInfo(config).SetAnyMode();
    for (uint32_t i = 0; i < config.EncodeCaps()->maxRateControlLayers; ++i) {
        rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    }

    const uint32_t layer_index = config.EncodeCaps()->maxRateControlLayers / 2;

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    uint64_t orig_bitrate = rc_info.Layer(layer_index)->averageBitrate;

    // averageBitrate must be greater than 0
    rc_info.Layer(layer_index)->averageBitrate = 0;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-pLayers-08276");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    // averageBitrate must be less than or equal to maxBitrate cap
    rc_info.Layer(layer_index)->averageBitrate = config.EncodeCaps()->maxBitrate + 1;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08278");
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-pLayers-08276");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    rc_info.Layer(layer_index)->averageBitrate = orig_bitrate;
    orig_bitrate = rc_info.Layer(layer_index)->maxBitrate;

    // maxBitrate must be greater than 0
    rc_info.Layer(layer_index)->maxBitrate = 0;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08278");
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-pLayers-08277");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    // maxBitrate must be less than or equal to maxBitrate cap
    rc_info.Layer(layer_index)->maxBitrate = config.EncodeCaps()->maxBitrate + 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-pLayers-08277");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    rc_info.Layer(layer_index)->maxBitrate = orig_bitrate;

    if (config.EncodeCaps()->rateControlModes & VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR) {
        rc_info->rateControlMode = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR;

        // averageBitrate must be less than or equal to maxBitrate
        rc_info.Layer(layer_index)->maxBitrate = rc_info.Layer(layer_index)->averageBitrate - 1;
        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08278");
        cb.ControlVideoCoding(context.Control().RateControl(rc_info));
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlLayerBitrateCBR) {
    TEST_DESCRIPTION("vkCmdControlVideoCodingKHR - test incorrect values for averageBitrate and maxBitrate with CBR");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncode(), VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with CBR rate control mode";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    auto rc_info = VideoEncodeRateControlInfo(config);
    rc_info->rateControlMode = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR;
    for (uint32_t i = 0; i < config.EncodeCaps()->maxRateControlLayers; ++i) {
        rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    }

    const uint32_t layer_index = config.EncodeCaps()->maxRateControlLayers / 2;

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    // averageBitrate must equal maxBitrate for CBR
    rc_info.Layer(layer_index)->maxBitrate = rc_info.Layer(layer_index)->averageBitrate - 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08356");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    rc_info.Layer(layer_index)->maxBitrate = rc_info.Layer(layer_index)->averageBitrate;
    rc_info.Layer(layer_index)->averageBitrate -= 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08356");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlLayerBitrateVBR) {
    TEST_DESCRIPTION("vkCmdControlVideoCodingKHR - test incorrect values for averageBitrate and maxBitrate with VBR");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncode(), VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with VBR rate control mode";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    auto rc_info = VideoEncodeRateControlInfo(config);
    rc_info->rateControlMode = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR;
    for (uint32_t i = 0; i < config.EncodeCaps()->maxRateControlLayers; ++i) {
        rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    }

    const uint32_t layer_index = config.EncodeCaps()->maxRateControlLayers / 2;

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    // averageBitrate must be less than or equal to maxBitrate
    rc_info.Layer(layer_index)->maxBitrate = rc_info.Layer(layer_index)->averageBitrate - 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08278");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlLayerFrameRate) {
    TEST_DESCRIPTION("vkCmdControlVideoCodingKHR - test incorrect values for frameRateNumerator and frameRateDenominator");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    auto rc_info = VideoEncodeRateControlInfo(config).SetAnyMode();
    for (uint32_t i = 0; i < config.EncodeCaps()->maxRateControlLayers; ++i) {
        rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    }

    const uint32_t layer_index = config.EncodeCaps()->maxRateControlLayers / 2;

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    // frameRateNumerator must be greater than 0
    rc_info.Layer(layer_index)->frameRateNumerator = 0;
    rc_info.Layer(layer_index)->frameRateDenominator = 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlLayerInfoKHR-frameRateNumerator-08350");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    // frameRateDenominator must be greater than 0
    rc_info.Layer(layer_index)->frameRateNumerator = 30;
    rc_info.Layer(layer_index)->frameRateDenominator = 0;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlLayerInfoKHR-frameRateDenominator-08351");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlVirtualBufferSize) {
    TEST_DESCRIPTION(
        "vkCmdControlVideoCodingKHR - test incorrect values for virtualBufferSizeInMs and initialVirtualBufferSizeInMs");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    auto rc_info = VideoEncodeRateControlInfo(config).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    // virtualBufferSizeInMs must be greater than 0
    rc_info->virtualBufferSizeInMs = 0;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-layerCount-08357");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    // initialVirtualBufferSizeInMs must be less than or equal to virtualBufferSizeInMs
    rc_info->virtualBufferSizeInMs = 1000;
    rc_info->initialVirtualBufferSizeInMs = 1001;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-layerCount-08358");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlH264ConstantQpNonZero) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - constantQp must be zero for H.264 if rate control mode is not DISABLED");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeH264()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));

    VideoEncodeInfo encode_info = context.EncodeFrame();
    encode_info.CodecInfo().encode_h264.slice_info.constantQp = 1;

    if (config.EncodeCapsH264()->maxQp < 1) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-constantQp-08270");
    }
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-constantQp-08269");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlH264ConstantQpNotInCapRange) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - constantQp must be within H.264 minQp/maxQp caps");

    RETURN_IF_SKIP(Init());

    VideoConfig config =
        GetConfig(GetConfigsWithRateControl(GetConfigsEncodeH264(), VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with DISABLED rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config);
    rc_info->rateControlMode = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));

    VideoEncodeInfo encode_info = context.EncodeFrame();

    encode_info.CodecInfo().encode_h264.slice_info.constantQp = config.EncodeCapsH264()->minQp - 1;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-constantQp-08270");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    encode_info.CodecInfo().encode_h264.slice_info.constantQp = config.EncodeCapsH264()->maxQp + 1;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-constantQp-08270");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlH264ConstantQpPerSliceMismatch) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - constantQp must match across H.264 slices "
        "if VK_VIDEO_ENCODE_H264_CAPABILITY_PER_SLICE_CONSTANT_QP_BIT_KHR is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(
        GetConfigsWithRateControl(GetConfigsEncodeH264(), VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR),
        [](const VideoConfig& config) {
            return config.EncodeCapsH264()->maxSliceCount > 1 &&
                   (config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_PER_SLICE_CONSTANT_QP_BIT_KHR) == 0;
        }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with DISABLED rate control, "
                        "support for multiple slices but no support for per-slice constant QP";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config);
    rc_info->rateControlMode = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));

    VideoEncodeInfo encode_info = context.EncodeFrame();
    std::vector<VkVideoEncodeH264NaluSliceInfoKHR> slices(2, encode_info.CodecInfo().encode_h264.slice_info);
    encode_info.CodecInfo().encode_h264.picture_info.naluSliceEntryCount = 2;
    encode_info.CodecInfo().encode_h264.picture_info.pNaluSliceEntries = slices.data();

    slices[0].constantQp = config.EncodeCapsH264()->minQp;
    slices[1].constantQp = config.EncodeCapsH264()->maxQp;

    if (slices[0].constantQp == slices[1].constantQp) {
        slices[1].constantQp += 1;
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-constantQp-08270");
    }

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceEntryCount-08302");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceEntryCount-08312");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-constantQp-08271");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlH265ConstantQpNonZero) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - constantQp must be zero for H.265 if rate control mode is not DISABLED");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeH265()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));

    VideoEncodeInfo encode_info = context.EncodeFrame();
    encode_info.CodecInfo().encode_h265.slice_segment_info.constantQp = 1;

    if (config.EncodeCapsH265()->maxQp < 1) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-constantQp-08273");
    }
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-constantQp-08272");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlH265ConstantQpNotInCapRange) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - constantQp must be within H.265 minQp/maxQp caps");

    RETURN_IF_SKIP(Init());

    VideoConfig config =
        GetConfig(GetConfigsWithRateControl(GetConfigsEncodeH265(), VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with DISABLED rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config);
    rc_info->rateControlMode = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));

    VideoEncodeInfo encode_info = context.EncodeFrame();

    encode_info.CodecInfo().encode_h265.slice_segment_info.constantQp = config.EncodeCapsH265()->minQp - 1;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-constantQp-08273");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    encode_info.CodecInfo().encode_h265.slice_segment_info.constantQp = config.EncodeCapsH265()->maxQp + 1;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-constantQp-08273");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlH265ConstantQpPerSliceSegmentMismatch) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - constantQp must match across H.265 slice segments "
        "if VK_VIDEO_ENCODE_H265_CAPABILITY_PER_SLICE_SEGMENT_CONSTANT_QP_BIT_KHR is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(
        GetConfigsWithRateControl(GetConfigsEncodeH265(), VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR),
        [](const VideoConfig& config) {
            return config.EncodeCapsH265()->maxSliceSegmentCount > 1 &&
                   (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_PER_SLICE_SEGMENT_CONSTANT_QP_BIT_KHR) == 0;
        }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with DISABLED rate control, "
                        "support for multiple slice segments but no support for per-slice-segment constant QP";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config);
    rc_info->rateControlMode = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));

    VideoEncodeInfo encode_info = context.EncodeFrame();
    std::vector<VkVideoEncodeH265NaluSliceSegmentInfoKHR> slice_segments(2, encode_info.CodecInfo().encode_h265.slice_segment_info);
    encode_info.CodecInfo().encode_h265.picture_info.naluSliceSegmentEntryCount = 2;
    encode_info.CodecInfo().encode_h265.picture_info.pNaluSliceSegmentEntries = slice_segments.data();

    slice_segments[0].constantQp = config.EncodeCapsH265()->minQp;
    slice_segments[1].constantQp = config.EncodeCapsH265()->maxQp;

    if (slice_segments[0].constantQp == slice_segments[1].constantQp) {
        slice_segments[1].constantQp += 1;
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-constantQp-08273");
    }

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08324");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08307");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08313");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-constantQp-08274");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlAV1ConstantQIndexNonZero) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - constantQIndex must be zero for AV1 if rate control mode is not DISABLED");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeAV1()));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));

    VideoEncodeInfo encode_info = context.EncodeFrame();
    encode_info.CodecInfo().encode_av1.picture_info.constantQIndex = 1;

    if (config.EncodeCapsAV1()->maxQIndex < 1) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-constantQIndex-10321");
    }
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-constantQIndex-10320");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlAV1ConstantQIndexNotInCapRange) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - constantQIndex must be within AV1 minQIndex/maxQIndex caps");

    RETURN_IF_SKIP(Init());

    // Find an AV1 encode config with a non-zero minQIndex, if possible
    VideoConfig config = GetConfig(
        FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeAV1(), VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR),
                      [](const VideoConfig& config) { return config.EncodeCapsAV1()->minQIndex > 0; }));
    // If couldn't find any config with a non-zero minQIndex, settle with any config that supports DISABLED rate control
    if (!config) {
        config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeAV1(), VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR));
    }
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with DISABLED rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config);
    rc_info->rateControlMode = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));

    VideoEncodeInfo encode_info = context.EncodeFrame();

    if (config.EncodeCapsAV1()->minQIndex > 0) {
        encode_info.CodecInfo().encode_av1.picture_info.constantQIndex = config.EncodeCapsAV1()->minQIndex - 1;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-constantQIndex-10321");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    encode_info.CodecInfo().encode_av1.picture_info.constantQIndex = config.EncodeCapsAV1()->maxQIndex + 1;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-constantQIndex-10321");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlH264LayerCountMismatch) {
    TEST_DESCRIPTION(
        "vkCmdBegin/ControlVideoCodingKHR - when using more than one rate control layer "
        "the layer count must match the H.264 temporal layer count");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithMultiLayerRateControl(GetConfigsEncodeH264()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with multi-layer rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));

    cb.Begin();

    rc_info.CodecInfo().encode_h264.temporalLayerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-07022");
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    if (config.EncodeCapsH264()->maxTemporalLayerCount > 2) {
        rc_info.CodecInfo().encode_h264.temporalLayerCount = 3;
        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-07022");
        cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
        m_errorMonitor->VerifyFound();
    }

    cb.BeginVideoCoding(context.Begin());

    rc_info.CodecInfo().encode_h264.temporalLayerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-07022");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    if (config.EncodeCapsH264()->maxTemporalLayerCount > 2) {
        rc_info.CodecInfo().encode_h264.temporalLayerCount = 3;
        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-07022");
        cb.ControlVideoCoding(context.Control().RateControl(rc_info));
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlH265LayerCountMismatch) {
    TEST_DESCRIPTION(
        "vkCmdBegin/ControlVideoCodingKHR - when using more than one rate control layer "
        "the layer count must match the H.265 sub-layer count");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithMultiLayerRateControl(GetConfigsEncodeH265()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with multi-layer rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));

    cb.Begin();

    rc_info.CodecInfo().encode_h265.subLayerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-07025");
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    if (config.EncodeCapsH265()->maxSubLayerCount > 2) {
        rc_info.CodecInfo().encode_h265.subLayerCount = 3;
        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-07025");
        cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
        m_errorMonitor->VerifyFound();
    }

    cb.BeginVideoCoding(context.Begin());

    rc_info.CodecInfo().encode_h265.subLayerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-07025");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    if (config.EncodeCapsH265()->maxSubLayerCount > 2) {
        rc_info.CodecInfo().encode_h265.subLayerCount = 3;
        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-07025");
        cb.ControlVideoCoding(context.Control().RateControl(rc_info));
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlAV1LayerCountMismatch) {
    TEST_DESCRIPTION(
        "vkCmdBegin/ControlVideoCodingKHR - when using more than one rate control layer "
        "the layer count must match the AV1 temporal layer count");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithMultiLayerRateControl(GetConfigsEncodeAV1()));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with multi-layer rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));

    cb.Begin();

    rc_info.CodecInfo().encode_av1.temporalLayerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-10351");
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    if (config.EncodeCapsAV1()->maxTemporalLayerCount > 2) {
        rc_info.CodecInfo().encode_av1.temporalLayerCount = 3;
        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-10351");
        cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
        m_errorMonitor->VerifyFound();
    }

    cb.BeginVideoCoding(context.Begin());

    rc_info.CodecInfo().encode_av1.temporalLayerCount = 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-10351");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    if (config.EncodeCapsAV1()->maxTemporalLayerCount > 2) {
        rc_info.CodecInfo().encode_av1.temporalLayerCount = 3;
        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-10351");
        cb.ControlVideoCoding(context.Control().RateControl(rc_info));
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlAV1TemporalLayerCount) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - temporalLayerCount is greater than maxTemporalLayerCount");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeAV1()));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    rc_info.CodecInfo().encode_av1.temporalLayerCount = config.EncodeCapsAV1()->maxTemporalLayerCount + 1;

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeAV1RateControlInfoKHR-temporalLayerCount-10299");
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeAV1RateControlInfoKHR-temporalLayerCount-10299");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlH264HrdCompliance) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - H.264 HRD compliant rate control depends on capability");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeH264()), [](const VideoConfig& config) {
        return (config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_HRD_COMPLIANCE_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode without HRD compliance support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    rc_info.CodecInfo().encode_h264.flags = VK_VIDEO_ENCODE_H264_RATE_CONTROL_ATTEMPT_HRD_COMPLIANCE_BIT_KHR;

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264RateControlInfoKHR-flags-08280");
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    m_errorMonitor->VerifyFound();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264RateControlInfoKHR-flags-08280");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlH265HrdCompliance) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - H.265 HRD compliant rate control depends on capability");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeH265()), [](const VideoConfig& config) {
        return (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_HRD_COMPLIANCE_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode without HRD compliance support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    rc_info.CodecInfo().encode_h265.flags = VK_VIDEO_ENCODE_H265_RATE_CONTROL_ATTEMPT_HRD_COMPLIANCE_BIT_KHR;

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265RateControlInfoKHR-flags-08291");
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    m_errorMonitor->VerifyFound();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265RateControlInfoKHR-flags-08291");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlH264InvalidCodecInfo) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - invalid H.264 rate control info");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeH264()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    const char* vuid = nullptr;

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    // Reference pattern without regular GOP flag
    vuid = "VUID-VkVideoEncodeH264RateControlInfoKHR-flags-08281";

    rc_info.CodecInfo().encode_h264.gopFrameCount = 8;

    rc_info.CodecInfo().encode_h264.flags = VK_VIDEO_ENCODE_H264_RATE_CONTROL_REFERENCE_PATTERN_FLAT_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h264.flags |= VK_VIDEO_ENCODE_H264_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h264.flags = VK_VIDEO_ENCODE_H264_RATE_CONTROL_REFERENCE_PATTERN_DYADIC_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h264.flags |= VK_VIDEO_ENCODE_H264_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info);

    // Conflicting reference pattern flags
    vuid = "VUID-VkVideoEncodeH264RateControlInfoKHR-flags-08282";

    rc_info.CodecInfo().encode_h264.flags = VK_VIDEO_ENCODE_H264_RATE_CONTROL_REGULAR_GOP_BIT_KHR |
                                            VK_VIDEO_ENCODE_H264_RATE_CONTROL_REFERENCE_PATTERN_FLAT_BIT_KHR |
                                            VK_VIDEO_ENCODE_H264_RATE_CONTROL_REFERENCE_PATTERN_DYADIC_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    // Regular GOP flag requires non-zero GOP size
    vuid = "VUID-VkVideoEncodeH264RateControlInfoKHR-flags-08283";

    rc_info.CodecInfo().encode_h264.flags = VK_VIDEO_ENCODE_H264_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h264.gopFrameCount = 0;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    // Invalid IDR period
    vuid = "VUID-VkVideoEncodeH264RateControlInfoKHR-idrPeriod-08284";

    rc_info.CodecInfo().encode_h264.gopFrameCount = 8;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h264.idrPeriod = 4;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h264.idrPeriod = 8;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h264.gopFrameCount = 9;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h264.gopFrameCount = 1;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h264.idrPeriod = 0;
    rc_test_utils.TestRateControlInfo(rc_info);

    // Invalid consecutive B frame count
    vuid = "VUID-VkVideoEncodeH264RateControlInfoKHR-consecutiveBFrameCount-08285";

    rc_info.CodecInfo().encode_h264.gopFrameCount = 4;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h264.consecutiveBFrameCount = 4;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h264.consecutiveBFrameCount = 3;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h264.gopFrameCount = 1;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h264.consecutiveBFrameCount = 0;
    rc_test_utils.TestRateControlInfo(rc_info);
}

TEST_F(NegativeVideo, EncodeRateControlH265InvalidCodecInfo) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - invalid H.265 rate control info");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeH265()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    const char* vuid = nullptr;

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    // Reference pattern without regular GOP flag
    vuid = "VUID-VkVideoEncodeH265RateControlInfoKHR-flags-08292";

    rc_info.CodecInfo().encode_h265.gopFrameCount = 8;

    rc_info.CodecInfo().encode_h265.flags = VK_VIDEO_ENCODE_H265_RATE_CONTROL_REFERENCE_PATTERN_FLAT_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h265.flags |= VK_VIDEO_ENCODE_H265_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h265.flags = VK_VIDEO_ENCODE_H265_RATE_CONTROL_REFERENCE_PATTERN_DYADIC_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h265.flags |= VK_VIDEO_ENCODE_H265_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info);

    // Conflicting reference pattern flags
    vuid = "VUID-VkVideoEncodeH265RateControlInfoKHR-flags-08293";

    rc_info.CodecInfo().encode_h265.flags = VK_VIDEO_ENCODE_H265_RATE_CONTROL_REGULAR_GOP_BIT_KHR |
                                            VK_VIDEO_ENCODE_H265_RATE_CONTROL_REFERENCE_PATTERN_FLAT_BIT_KHR |
                                            VK_VIDEO_ENCODE_H265_RATE_CONTROL_REFERENCE_PATTERN_DYADIC_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    // Regular GOP flag requires non-zero GOP size
    vuid = "VUID-VkVideoEncodeH265RateControlInfoKHR-flags-08294";

    rc_info.CodecInfo().encode_h265.flags = VK_VIDEO_ENCODE_H265_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h265.gopFrameCount = 0;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    // Invalid IDR period
    vuid = "VUID-VkVideoEncodeH265RateControlInfoKHR-idrPeriod-08295";

    rc_info.CodecInfo().encode_h265.gopFrameCount = 8;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h265.idrPeriod = 4;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h265.idrPeriod = 8;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h265.gopFrameCount = 9;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h265.gopFrameCount = 1;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h265.idrPeriod = 0;
    rc_test_utils.TestRateControlInfo(rc_info);

    // Invalid consecutive B frame count
    vuid = "VUID-VkVideoEncodeH265RateControlInfoKHR-consecutiveBFrameCount-08296";

    rc_info.CodecInfo().encode_h265.gopFrameCount = 4;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h265.consecutiveBFrameCount = 4;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h265.consecutiveBFrameCount = 3;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_h265.gopFrameCount = 1;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_h265.consecutiveBFrameCount = 0;
    rc_test_utils.TestRateControlInfo(rc_info);
}

TEST_F(NegativeVideo, EncodeRateControlAV1InvalidCodecInfo) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - invalid AV1 rate control info");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeAV1()));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    const char* vuid = nullptr;

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    // Reference pattern without regular GOP flag
    vuid = "VUID-VkVideoEncodeAV1RateControlInfoKHR-flags-10294";

    rc_info.CodecInfo().encode_av1.gopFrameCount = 8;

    rc_info.CodecInfo().encode_av1.flags = VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REFERENCE_PATTERN_FLAT_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_av1.flags |= VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_av1.flags = VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REFERENCE_PATTERN_DYADIC_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_av1.flags |= VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info);

    // Conflicting reference pattern flags
    vuid = "VUID-VkVideoEncodeAV1RateControlInfoKHR-flags-10295";

    rc_info.CodecInfo().encode_av1.flags = VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REGULAR_GOP_BIT_KHR |
                                           VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REFERENCE_PATTERN_FLAT_BIT_KHR |
                                           VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REFERENCE_PATTERN_DYADIC_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    // Regular GOP flag requires non-zero GOP size
    vuid = "VUID-VkVideoEncodeAV1RateControlInfoKHR-flags-10296";

    rc_info.CodecInfo().encode_av1.flags = VK_VIDEO_ENCODE_AV1_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_av1.gopFrameCount = 0;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    // Invalid key frame period
    vuid = "VUID-VkVideoEncodeAV1RateControlInfoKHR-keyFramePeriod-10297";

    rc_info.CodecInfo().encode_av1.gopFrameCount = 8;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_av1.keyFramePeriod = 4;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_av1.keyFramePeriod = 8;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_av1.gopFrameCount = 9;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_av1.gopFrameCount = 1;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_av1.keyFramePeriod = 0;
    rc_test_utils.TestRateControlInfo(rc_info);

    // Invalid consecutive bipredictive frame count
    vuid = "VUID-VkVideoEncodeAV1RateControlInfoKHR-consecutiveBipredictiveFrameCount-10298";

    rc_info.CodecInfo().encode_av1.gopFrameCount = 4;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_av1.consecutiveBipredictiveFrameCount = 4;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_av1.consecutiveBipredictiveFrameCount = 3;
    rc_test_utils.TestRateControlInfo(rc_info);

    rc_info.CodecInfo().encode_av1.gopFrameCount = 1;
    rc_test_utils.TestRateControlInfo(rc_info, vuid);

    rc_info.CodecInfo().encode_av1.consecutiveBipredictiveFrameCount = 0;
    rc_test_utils.TestRateControlInfo(rc_info);
}

TEST_F(NegativeVideo, EncodeRateControlH264QpRange) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - H.264 rate control layer QP out of bounds");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeH264()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    const char* expected_min_qp_vuid = "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08286";
    const char* allowed_min_qp_vuid = "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08288";

    // minQp.qpI not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.minQp.qpI -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // minQp.qpP not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.minQp.qpP -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // minQp.qpB not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.minQp.qpB -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // out of bounds minQp should be ignored if useMinQp is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.useMinQp = VK_FALSE;
        rc_layer.CodecInfo().encode_h264.minQp.qpI -= 1;
        rc_layer.CodecInfo().encode_h264.minQp.qpP -= 1;
        rc_layer.CodecInfo().encode_h264.minQp.qpB -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }

    const char* expected_max_qp_vuid = "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08287";
    const char* allowed_max_qp_vuid = "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08289";

    // maxQp.qpI not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.maxQp.qpI += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // maxQp.qpP not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.maxQp.qpP += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // maxQp.qpB not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.maxQp.qpB += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // out of bounds maxQp should be ignored if useMaxQp is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.useMaxQp = VK_FALSE;
        rc_layer.CodecInfo().encode_h264.maxQp.qpI += 1;
        rc_layer.CodecInfo().encode_h264.maxQp.qpP += 1;
        rc_layer.CodecInfo().encode_h264.maxQp.qpB += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }
}

TEST_F(NegativeVideo, EncodeRateControlH265QpRange) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - H.265 rate control layer QP out of bounds");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeH265()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    const char* expected_min_qp_vuid = "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08297";
    const char* allowed_min_qp_vuid = "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08299";

    // minQp.qpI not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.minQp.qpI -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // minQp.qpP not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.minQp.qpP -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // minQp.qpB not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.minQp.qpB -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // out of bounds minQp should be ignored if useMinQp is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.useMinQp = VK_FALSE;
        rc_layer.CodecInfo().encode_h265.minQp.qpI -= 1;
        rc_layer.CodecInfo().encode_h265.minQp.qpP -= 1;
        rc_layer.CodecInfo().encode_h265.minQp.qpB -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }

    const char* expected_max_qp_vuid = "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08298";
    const char* allowed_max_qp_vuid = "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08300";

    // maxQp.qpI not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.maxQp.qpI += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // maxQp.qpP not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.maxQp.qpP += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // maxQp.qpB not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.maxQp.qpB += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // out of bounds maxQp should be ignored if useMaxQp is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.useMaxQp = VK_FALSE;
        rc_layer.CodecInfo().encode_h265.maxQp.qpI += 1;
        rc_layer.CodecInfo().encode_h265.maxQp.qpP += 1;
        rc_layer.CodecInfo().encode_h265.maxQp.qpB += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }
}

TEST_F(NegativeVideo, EncodeRateControlAV1QIndexRange) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - AV1 rate control layer quantizer index out of bounds");

    RETURN_IF_SKIP(Init());

    // Find an AV1 encode config with a non-zero minQIndex, if possible
    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeAV1()),
                                                 [](const VideoConfig& config) { return config.EncodeCapsAV1()->minQIndex > 0; }));
    // If couldn't find any config with a non-zero minQIndex, settle with any config that supports DISABLED rate control
    if (!config) {
        config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeAV1()));
    }
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    const char* expected_min_qi_vuid = "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMinQIndex-10300";
    const char* allowed_min_qi_vuid = "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMinQIndex-10301";

    if (config.EncodeCapsAV1()->minQIndex > 0) {
        // minQIndex.intraQIndex not in supported range
        {
            auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
            rc_layer.CodecInfo().encode_av1.minQIndex.intraQIndex -= 1;
            rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qi_vuid, {allowed_min_qi_vuid});
        }

        // minQIndex.predictiveQIndex not in supported range
        {
            auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
            rc_layer.CodecInfo().encode_av1.minQIndex.predictiveQIndex -= 1;
            rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qi_vuid, {allowed_min_qi_vuid});
        }

        // minQIndex.bipredictiveQIndex not in supported range
        {
            auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
            rc_layer.CodecInfo().encode_av1.minQIndex.bipredictiveQIndex -= 1;
            rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qi_vuid, {allowed_min_qi_vuid});
        }
    }

    // out of bounds minQIndex should be ignored if useMinQIndex is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.useMinQIndex = VK_FALSE;
        rc_layer.CodecInfo().encode_av1.minQIndex.intraQIndex -= 1;
        rc_layer.CodecInfo().encode_av1.minQIndex.predictiveQIndex -= 1;
        rc_layer.CodecInfo().encode_av1.minQIndex.bipredictiveQIndex -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }

    const char* expected_max_qi_vuid = "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMaxQIndex-10302";
    const char* allowed_max_qi_vuid = "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMaxQIndex-10303";

    // maxQIndex.intraQIndex not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.maxQIndex.intraQIndex += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qi_vuid, {allowed_max_qi_vuid});
    }

    // maxQIndex.predictiveQIndex not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.maxQIndex.predictiveQIndex += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qi_vuid, {allowed_max_qi_vuid});
    }

    // maxQIndex.bipredictiveQIndex not in supported range
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.maxQIndex.bipredictiveQIndex += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qi_vuid, {allowed_max_qi_vuid});
    }

    // out of bounds maxQIndex should be ignored if useMaxQIndex is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.useMaxQIndex = VK_FALSE;
        rc_layer.CodecInfo().encode_av1.maxQIndex.intraQIndex += 1;
        rc_layer.CodecInfo().encode_av1.maxQIndex.predictiveQIndex += 1;
        rc_layer.CodecInfo().encode_av1.maxQIndex.bipredictiveQIndex += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }
}

TEST_F(NegativeVideo, EncodeRateControlH264PerPicTypeQp) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - H.264 per picture type min/max QP depends on capability");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeH264()), [](const VideoConfig& config) {
        return (config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_PER_PICTURE_TYPE_MIN_MAX_QP_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode without per picture type min/max QP support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    const char* expected_min_qp_vuid = "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08288";
    const char* allowed_min_qp_vuid = "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08286";

    // minQp.qpI does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.minQp.qpI += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // minQp.qpP does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.minQp.qpP += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // minQp.qpB does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.minQp.qpB += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // non-matching QP values in minQp should be ignored if useMinQp is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.useMinQp = VK_FALSE;
        rc_layer.CodecInfo().encode_h264.minQp.qpP += 1;
        rc_layer.CodecInfo().encode_h264.minQp.qpB += 2;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }

    const char* expected_max_qp_vuid = "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08289";
    const char* allowed_max_qp_vuid = "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08287";

    // maxQp.qpI does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.maxQp.qpI -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // maxQp.qpP does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.maxQp.qpP -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // maxQp.qpB does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.maxQp.qpB -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // non-matching QP values in maxQp should be ignored if useMaxQp is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.useMaxQp = VK_FALSE;
        rc_layer.CodecInfo().encode_h264.maxQp.qpP -= 1;
        rc_layer.CodecInfo().encode_h264.maxQp.qpB -= 2;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }
}

TEST_F(NegativeVideo, EncodeRateControlH265PerPicTypeQp) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - H.265 per picture type min/max QP depends on capability");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeH265()), [](const VideoConfig& config) {
        return (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_PER_PICTURE_TYPE_MIN_MAX_QP_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode without per picture type min/max QP support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    const char* expected_min_qp_vuid = "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08299";
    const char* allowed_min_qp_vuid = "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08297";

    // minQp.qpI does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.minQp.qpI += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // minQp.qpP does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.minQp.qpP += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // minQp.qpB does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.minQp.qpB += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qp_vuid, {allowed_min_qp_vuid});
    }

    // non-matching QP values in minQp should be ignored if useMinQp is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.useMinQp = VK_FALSE;
        rc_layer.CodecInfo().encode_h265.minQp.qpP += 1;
        rc_layer.CodecInfo().encode_h265.minQp.qpB += 2;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }

    const char* expected_max_qp_vuid = "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08300";
    const char* allowed_max_qp_vuid = "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08298";

    // maxQp.qpI does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.maxQp.qpI -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // maxQp.qpP does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.maxQp.qpP -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // maxQp.qpB does not match the other QP values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.maxQp.qpB -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qp_vuid, {allowed_max_qp_vuid});
    }

    // non-matching QP values in maxQp should be ignored if useMaxQp is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.useMaxQp = VK_FALSE;
        rc_layer.CodecInfo().encode_h265.maxQp.qpP -= 1;
        rc_layer.CodecInfo().encode_h265.maxQp.qpB -= 2;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }
}

TEST_F(NegativeVideo, EncodeRateControlAV1PerRcGroupQIndex) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - AV1 per rate control group min/max quantizer index depends on capability");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeAV1()), [](const VideoConfig& config) {
        return (config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_PER_RATE_CONTROL_GROUP_MIN_MAX_Q_INDEX_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode without per rate control group min/max quantizer index support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    const char* expected_min_qi_vuid = "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMinQIndex-10301";
    const char* allowed_min_qi_vuid = "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMinQIndex-10300";

    // minQIndex.intraQIndex does not match the other quantizer index values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.minQIndex.intraQIndex += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qi_vuid, {allowed_min_qi_vuid});
    }

    // minQIndex.predictiveQIndex does not match the other quantizer index values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.minQIndex.predictiveQIndex += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qi_vuid, {allowed_min_qi_vuid});
    }

    // minQIndex.bipredictiveQIndex does not match the other quantizer index values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.minQIndex.bipredictiveQIndex += 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_min_qi_vuid, {allowed_min_qi_vuid});
    }

    // non-matching quantizer index values in minQIndex should be ignored if useMinQIndex is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.useMinQIndex = VK_FALSE;
        rc_layer.CodecInfo().encode_av1.minQIndex.predictiveQIndex += 1;
        rc_layer.CodecInfo().encode_av1.minQIndex.bipredictiveQIndex += 2;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }

    const char* expected_max_qi_vuid = "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMaxQIndex-10303";
    const char* allowed_max_qi_vuid = "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMaxQIndex-10302";

    // maxQIndex.intraQIndex does not match the other quantizer index values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.maxQIndex.intraQIndex -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qi_vuid, {allowed_max_qi_vuid});
    }

    // maxQIndex.predictiveQIndex does not match the other quantizer index values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.maxQIndex.predictiveQIndex -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qi_vuid, {allowed_max_qi_vuid});
    }

    // maxQIndex.bipredictiveQIndex does not match the other quantizer index values
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.maxQIndex.bipredictiveQIndex -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_max_qi_vuid, {allowed_max_qi_vuid});
    }

    // non-matching quantizer index values in maxQIndex should be ignored if useMaxQIndex is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.useMaxQIndex = VK_FALSE;
        rc_layer.CodecInfo().encode_av1.maxQIndex.predictiveQIndex -= 1;
        rc_layer.CodecInfo().encode_av1.maxQIndex.bipredictiveQIndex -= 2;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }
}

TEST_F(NegativeVideo, EncodeRateControlH264MinQpGreaterThanMaxQp) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - H.264 rate control layer minQp > maxQp");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeH264()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    const char* expected_vuid = "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08374";
    std::vector<const char*> allowed_vuids = {
        "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08287",
        "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08288",
        "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08289",
    };

    // minQp.qpI > maxQp.qpI
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.minQp.qpI = rc_layer.CodecInfo().encode_h264.maxQp.qpI;
        rc_layer.CodecInfo().encode_h264.maxQp.qpI -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_vuid, allowed_vuids);
    }

    // minQp.qpP > maxQp.qpP
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.minQp.qpP = rc_layer.CodecInfo().encode_h264.maxQp.qpP;
        rc_layer.CodecInfo().encode_h264.maxQp.qpP -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_vuid, allowed_vuids);
    }

    // minQp.qpB > maxQp.qpB
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.minQp.qpB = rc_layer.CodecInfo().encode_h264.maxQp.qpB;
        rc_layer.CodecInfo().encode_h264.maxQp.qpB -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_vuid, allowed_vuids);
    }

    // minQp can be equal to maxQp
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h264.minQp.qpI = rc_layer.CodecInfo().encode_h264.maxQp.qpI;
        rc_layer.CodecInfo().encode_h264.minQp.qpP = rc_layer.CodecInfo().encode_h264.maxQp.qpP;
        rc_layer.CodecInfo().encode_h264.minQp.qpB = rc_layer.CodecInfo().encode_h264.maxQp.qpB;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }

    // minQp can be larger than maxQp if useMinQp or useMaxQp is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        std::swap(rc_layer.CodecInfo().encode_h264.minQp, rc_layer.CodecInfo().encode_h264.maxQp);

        rc_layer.CodecInfo().encode_h264.useMinQp = VK_FALSE;
        rc_layer.CodecInfo().encode_h264.useMaxQp = VK_TRUE;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);

        rc_layer.CodecInfo().encode_h264.useMinQp = VK_TRUE;
        rc_layer.CodecInfo().encode_h264.useMaxQp = VK_FALSE;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }
}

TEST_F(NegativeVideo, EncodeRateControlH265MinQpGreaterThanMaxQp) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - H.265 rate control layer minQp > maxQp");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeH265()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    const char* expected_vuid = "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08375";
    std::vector<const char*> allowed_vuids = {
        "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08298",
        "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08299",
        "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08300",
    };

    // minQp.qpI > maxQp.qpI
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.minQp.qpI = rc_layer.CodecInfo().encode_h265.maxQp.qpI;
        rc_layer.CodecInfo().encode_h265.maxQp.qpI -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_vuid, allowed_vuids);
    }

    // minQp.qpP > maxQp.qpP
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.minQp.qpP = rc_layer.CodecInfo().encode_h265.maxQp.qpP;
        rc_layer.CodecInfo().encode_h265.maxQp.qpP -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_vuid, allowed_vuids);
    }

    // minQp.qpB > maxQp.qpB
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.minQp.qpB = rc_layer.CodecInfo().encode_h265.maxQp.qpB;
        rc_layer.CodecInfo().encode_h265.maxQp.qpB -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_vuid, allowed_vuids);
    }

    // minQp can be equal to maxQp
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        rc_layer.CodecInfo().encode_h265.minQp.qpI = rc_layer.CodecInfo().encode_h265.maxQp.qpI;
        rc_layer.CodecInfo().encode_h265.minQp.qpP = rc_layer.CodecInfo().encode_h265.maxQp.qpP;
        rc_layer.CodecInfo().encode_h265.minQp.qpB = rc_layer.CodecInfo().encode_h265.maxQp.qpB;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }

    // minQp can be larger than maxQp if useMinQp or useMaxQp is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQp();
        std::swap(rc_layer.CodecInfo().encode_h265.minQp, rc_layer.CodecInfo().encode_h265.maxQp);

        rc_layer.CodecInfo().encode_h265.useMinQp = VK_FALSE;
        rc_layer.CodecInfo().encode_h265.useMaxQp = VK_TRUE;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);

        rc_layer.CodecInfo().encode_h265.useMinQp = VK_TRUE;
        rc_layer.CodecInfo().encode_h265.useMaxQp = VK_FALSE;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }
}

TEST_F(NegativeVideo, EncodeRateControlAV1MinQIndexGreaterThanMaxQIndex) {
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - AV1 rate control layer minQIndex > maxQIndex");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncodeAV1()));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    const char* expected_vuid = "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMinQIndex-10304";
    std::vector<const char*> allowed_vuids = {
        "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMaxQIndex-10302",
        "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMinQIndex-10301",
        "VUID-VkVideoEncodeAV1RateControlLayerInfoKHR-useMaxQIndex-10303",
    };

    // minQIndex.intraQIndex > maxQIndex.intraQIndex
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.minQIndex.intraQIndex = rc_layer.CodecInfo().encode_av1.maxQIndex.intraQIndex;
        rc_layer.CodecInfo().encode_av1.maxQIndex.intraQIndex -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_vuid, allowed_vuids);
    }

    // minQIndex.predictiveQIndex > maxQIndex.predictiveQIndex
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.minQIndex.predictiveQIndex = rc_layer.CodecInfo().encode_av1.maxQIndex.predictiveQIndex;
        rc_layer.CodecInfo().encode_av1.maxQIndex.predictiveQIndex -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_vuid, allowed_vuids);
    }

    // minQIndex.bipredictiveQIndex > maxQIndex.bipredictiveQIndex
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.minQIndex.bipredictiveQIndex = rc_layer.CodecInfo().encode_av1.maxQIndex.bipredictiveQIndex;
        rc_layer.CodecInfo().encode_av1.maxQIndex.bipredictiveQIndex -= 1;
        rc_test_utils.TestRateControlLayerInfo(rc_layer, expected_vuid, allowed_vuids);
    }

    // minQIndex can be equal to maxQIndex
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        rc_layer.CodecInfo().encode_av1.minQIndex.intraQIndex = rc_layer.CodecInfo().encode_av1.maxQIndex.intraQIndex;
        rc_layer.CodecInfo().encode_av1.minQIndex.predictiveQIndex = rc_layer.CodecInfo().encode_av1.maxQIndex.predictiveQIndex;
        rc_layer.CodecInfo().encode_av1.minQIndex.bipredictiveQIndex = rc_layer.CodecInfo().encode_av1.maxQIndex.bipredictiveQIndex;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }

    // minQIndex can be larger than maxQIndex if useMinQIndex or useMaxQIndex is not set
    {
        auto rc_layer = rc_test_utils.CreateRateControlLayerWithMinMaxQIndex();
        std::swap(rc_layer.CodecInfo().encode_av1.minQIndex, rc_layer.CodecInfo().encode_av1.maxQIndex);

        rc_layer.CodecInfo().encode_av1.useMinQIndex = VK_FALSE;
        rc_layer.CodecInfo().encode_av1.useMaxQIndex = VK_TRUE;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);

        rc_layer.CodecInfo().encode_av1.useMinQIndex = VK_TRUE;
        rc_layer.CodecInfo().encode_av1.useMaxQIndex = VK_FALSE;
        rc_test_utils.TestRateControlLayerInfo(rc_layer);
    }
}

TEST_F(NegativeVideo, EncodeRateControlMissingChain) {
    TEST_DESCRIPTION("vkCmdControlVideoCodingKHR - missing rate control info");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto control_info = vku::InitStruct<VkVideoCodingControlInfoKHR>();
    control_info.flags = VK_VIDEO_CODING_CONTROL_ENCODE_RATE_CONTROL_BIT_KHR;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->SetDesiredError("VUID-VkVideoCodingControlInfoKHR-flags-07018");
    cb.ControlVideoCoding(control_info);
    m_errorMonitor->VerifyFound();
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRateControlStateMismatchNotDefault) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - rate control state not specified but rate control mode is not DEFAULT");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithRateControl(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with rate control";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset().RateControl(rc_info));
    cb.EndVideoCoding(context.End());
    cb.End();

    context.Queue().Submit(cb);
    m_device->Wait();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.EndVideoCoding(context.End());
    cb.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-pBeginInfo-08253");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeVideo, EncodeRateControlStateMismatch) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - rate control state specified does not match current configuration");

    RETURN_IF_SKIP(Init());

    // Try to find a config that supports both CBR and VBR
    VkVideoEncodeRateControlModeFlagsKHR rc_modes =
        VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR | VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR;
    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [rc_modes](const VideoConfig& config) {
        return (config.EncodeCaps()->rateControlModes & rc_modes) == rc_modes;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires an encode profile that supports both CBR and VBR rate control modes";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config);

    rc_info->rateControlMode = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR;
    rc_info->virtualBufferSizeInMs = 3000;
    rc_info->initialVirtualBufferSizeInMs = 1000;

    for (uint32_t i = 0; i < config.EncodeCaps()->maxRateControlLayers; ++i) {
        auto rc_layer = VideoEncodeRateControlLayerInfo(config);

        rc_layer->averageBitrate = 32000 / (i + 1);
        rc_layer->maxBitrate = 32000 / (i + 1);
        rc_layer->frameRateNumerator = 30;
        rc_layer->frameRateDenominator = i + 1;

        rc_info.AddLayer(rc_layer);
    }

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset().RateControl(rc_info));
    cb.EndVideoCoding(context.End());
    cb.End();

    context.Queue().Submit(cb);
    m_device->Wait();

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    // rateControlMode mismatch
    VkVideoEncodeRateControlModeFlagBitsKHR rate_control_mode = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR;
    std::swap(rc_info->rateControlMode, rate_control_mode);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info->rateControlMode, rate_control_mode);

    // layerCount mismatch
    if (rc_info->layerCount > 1) {
        uint32_t layer_count = rc_info->layerCount - 1;
        std::swap(rc_info->layerCount, layer_count);
        rc_test_utils.TestRateControlStateMismatch(rc_info);
        std::swap(rc_info->layerCount, layer_count);
    }

    // virtualBufferSizeInMs mismatch
    uint32_t vb_size = 2500;
    std::swap(rc_info->virtualBufferSizeInMs, vb_size);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info->virtualBufferSizeInMs, vb_size);

    // initialVirtualBufferSizeInMs mismatch
    uint32_t init_vb_size = 500;
    std::swap(rc_info->initialVirtualBufferSizeInMs, init_vb_size);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info->initialVirtualBufferSizeInMs, init_vb_size);

    // Layer averageBitrate/maxBitrate mismatch
    uint64_t max_bitrate = 23456;
    uint64_t avg_bitrate = 7891;
    std::swap(rc_info.Layer(0)->averageBitrate, avg_bitrate);
    std::swap(rc_info.Layer(0)->maxBitrate, max_bitrate);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.Layer(0)->averageBitrate, avg_bitrate);
    std::swap(rc_info.Layer(0)->maxBitrate, max_bitrate);

    // Layer frameRateNumerator/Denominator mismatch
    uint32_t fr_num = 30000;
    uint32_t fr_den = 1001;
    std::swap(rc_info.Layer(0)->frameRateNumerator, fr_num);
    std::swap(rc_info.Layer(0)->frameRateDenominator, fr_den);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.Layer(0)->frameRateNumerator, fr_num);
    std::swap(rc_info.Layer(0)->frameRateDenominator, fr_den);
}

TEST_F(NegativeVideo, EncodeRateControlStateMismatchH264) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - H.264 rate control state specified does not match current configuration");

    RETURN_IF_SKIP(Init());

    // Try to find a config that supports both CBR and VBR
    VkVideoEncodeRateControlModeFlagsKHR rc_modes =
        VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR | VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR;
    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeH264(), [rc_modes](const VideoConfig& config) {
        return (config.EncodeCaps()->rateControlModes & rc_modes) == rc_modes;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires an H.264 encode profile that supports both CBR and VBR rate control modes";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    const bool include_codec_info = true;

    auto rc_info = VideoEncodeRateControlInfo(config, include_codec_info);
    rc_info->rateControlMode = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR;
    rc_info->virtualBufferSizeInMs = 3000;
    rc_info->initialVirtualBufferSizeInMs = 1000;

    rc_info.CodecInfo().encode_h264.flags = VK_VIDEO_ENCODE_H264_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_info.CodecInfo().encode_h264.gopFrameCount = 15;
    rc_info.CodecInfo().encode_h264.idrPeriod = 60;
    rc_info.CodecInfo().encode_h264.consecutiveBFrameCount = 2;
    rc_info.CodecInfo().encode_h264.temporalLayerCount = config.EncodeCaps()->maxRateControlLayers;

    for (uint32_t i = 0; i < config.EncodeCaps()->maxRateControlLayers; ++i) {
        auto rc_layer = VideoEncodeRateControlLayerInfo(config, include_codec_info);

        rc_layer->averageBitrate = 32000 / (i + 1);
        rc_layer->maxBitrate = 48000 / (i + 1);
        rc_layer->frameRateNumerator = 30;
        rc_layer->frameRateDenominator = i + 1;

        rc_layer.CodecInfo().encode_h264.useMinQp = VK_TRUE;
        rc_layer.CodecInfo().encode_h264.minQp.qpI = config.ClampH264Qp(0 + i);
        rc_layer.CodecInfo().encode_h264.minQp.qpP = config.ClampH264Qp(5 + i);
        rc_layer.CodecInfo().encode_h264.minQp.qpB = config.ClampH264Qp(10 + i);

        if ((config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_PER_PICTURE_TYPE_MIN_MAX_QP_BIT_KHR) == 0) {
            rc_layer.CodecInfo().encode_h264.minQp.qpP = rc_layer.CodecInfo().encode_h264.minQp.qpI;
            rc_layer.CodecInfo().encode_h264.minQp.qpB = rc_layer.CodecInfo().encode_h264.minQp.qpI;
        }

        rc_layer.CodecInfo().encode_h264.useMaxFrameSize = VK_TRUE;
        rc_layer.CodecInfo().encode_h264.maxFrameSize.frameISize = 30000 / (i + 1);
        rc_layer.CodecInfo().encode_h264.maxFrameSize.framePSize = 20000 / (i + 1);
        rc_layer.CodecInfo().encode_h264.maxFrameSize.frameBSize = 10000 / (i + 1);

        rc_info.AddLayer(rc_layer);
    }

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset().RateControl(rc_info));
    cb.EndVideoCoding(context.End());
    cb.End();

    context.Queue().Submit(cb);
    m_device->Wait();

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    // flags mismatch
    VkVideoEncodeH264RateControlFlagsKHR flags = VK_VIDEO_ENCODE_H264_RATE_CONTROL_TEMPORAL_LAYER_PATTERN_DYADIC_BIT_KHR;
    std::swap(rc_info.CodecInfo().encode_h264.flags, flags);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.CodecInfo().encode_h264.flags, flags);

    // gopFrameCount mismatch
    uint32_t gop_frame_count = 12;
    std::swap(rc_info.CodecInfo().encode_h264.gopFrameCount, gop_frame_count);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.CodecInfo().encode_h264.gopFrameCount, gop_frame_count);

    // idrPeriod mismatch
    uint32_t idr_period = 30;
    std::swap(rc_info.CodecInfo().encode_h264.idrPeriod, idr_period);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.CodecInfo().encode_h264.idrPeriod, idr_period);

    // consecutiveBFrameCount mismatch
    uint32_t cons_b_frames = 4;
    std::swap(rc_info.CodecInfo().encode_h264.consecutiveBFrameCount, cons_b_frames);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.CodecInfo().encode_h264.consecutiveBFrameCount, cons_b_frames);

    // Layer useMinQp mismatch
    rc_info.Layer(0).CodecInfo().encode_h264.useMinQp = VK_FALSE;
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_h264.useMinQp = VK_TRUE;

    // Layer minQp.qpB mismatch
    rc_info.Layer(0).CodecInfo().encode_h264.minQp.qpB -= 1;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08286");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08288");
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_h264.minQp.qpB += 1;

    // Layer useMaxQp mismatch
    rc_info.Layer(0).CodecInfo().encode_h264.useMaxQp = VK_TRUE;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08287");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08289");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08374");
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_h264.useMaxQp = VK_FALSE;

    // Layer maxQp.qpP mismatch
    rc_info.Layer(0).CodecInfo().encode_h264.maxQp.qpP += 1;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08287");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08289");
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_h264.maxQp.qpP -= 1;

    // Layer useMaxFrameSize mismatch
    rc_info.Layer(0).CodecInfo().encode_h264.useMaxFrameSize = VK_FALSE;
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_h264.useMaxFrameSize = VK_TRUE;

    // Layer maxFrameSize.frameISize mismatch
    uint32_t max_frame_i_size = 12345;
    std::swap(rc_info.Layer(0).CodecInfo().encode_h264.maxFrameSize.frameISize, max_frame_i_size);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.Layer(0).CodecInfo().encode_h264.maxFrameSize.frameISize, max_frame_i_size);
}

TEST_F(NegativeVideo, EncodeRateControlStateMismatchH265) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - H.265 rate control state specified does not match current configuration");

    RETURN_IF_SKIP(Init());

    // Try to find a config that supports both CBR and VBR
    VkVideoEncodeRateControlModeFlagsKHR rc_modes =
        VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR | VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR;
    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [rc_modes](const VideoConfig& config) {
        return (config.EncodeCaps()->rateControlModes & rc_modes) == rc_modes;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires an H.265 encode profile that supports both CBR and VBR rate control modes";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    const bool include_codec_info = true;

    auto rc_info = VideoEncodeRateControlInfo(config, include_codec_info);
    rc_info->rateControlMode = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR;
    rc_info->virtualBufferSizeInMs = 3000;
    rc_info->initialVirtualBufferSizeInMs = 1000;

    rc_info.CodecInfo().encode_h265.flags = VK_VIDEO_ENCODE_H265_RATE_CONTROL_REGULAR_GOP_BIT_KHR;
    rc_info.CodecInfo().encode_h265.gopFrameCount = 15;
    rc_info.CodecInfo().encode_h265.idrPeriod = 60;
    rc_info.CodecInfo().encode_h265.consecutiveBFrameCount = 2;
    rc_info.CodecInfo().encode_h265.subLayerCount = config.EncodeCaps()->maxRateControlLayers;

    for (uint32_t i = 0; i < config.EncodeCaps()->maxRateControlLayers; ++i) {
        auto rc_layer = VideoEncodeRateControlLayerInfo(config, include_codec_info);

        rc_layer->averageBitrate = 32000 / (i + 1);
        rc_layer->maxBitrate = 48000 / (i + 1);
        rc_layer->frameRateNumerator = 30;
        rc_layer->frameRateDenominator = i + 1;

        rc_layer.CodecInfo().encode_h265.useMinQp = VK_TRUE;
        rc_layer.CodecInfo().encode_h265.minQp.qpI = config.ClampH265Qp(0 + i);
        rc_layer.CodecInfo().encode_h265.minQp.qpP = config.ClampH265Qp(5 + i);
        rc_layer.CodecInfo().encode_h265.minQp.qpB = config.ClampH265Qp(10 + i);

        if ((config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_PER_PICTURE_TYPE_MIN_MAX_QP_BIT_KHR) == 0) {
            rc_layer.CodecInfo().encode_h265.minQp.qpP = rc_layer.CodecInfo().encode_h265.minQp.qpI;
            rc_layer.CodecInfo().encode_h265.minQp.qpB = rc_layer.CodecInfo().encode_h265.minQp.qpI;
        }

        rc_layer.CodecInfo().encode_h265.useMaxFrameSize = VK_TRUE;
        rc_layer.CodecInfo().encode_h265.maxFrameSize.frameISize = 30000 / (i + 1);
        rc_layer.CodecInfo().encode_h265.maxFrameSize.framePSize = 20000 / (i + 1);
        rc_layer.CodecInfo().encode_h265.maxFrameSize.frameBSize = 10000 / (i + 1);

        rc_info.AddLayer(rc_layer);
    }

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset().RateControl(rc_info));
    cb.EndVideoCoding(context.End());
    cb.End();

    context.Queue().Submit(cb);
    m_device->Wait();

    VideoEncodeRateControlTestUtils rc_test_utils(this, context);

    // flags mismatch
    VkVideoEncodeH265RateControlFlagsKHR flags = VK_VIDEO_ENCODE_H265_RATE_CONTROL_TEMPORAL_SUB_LAYER_PATTERN_DYADIC_BIT_KHR;
    std::swap(rc_info.CodecInfo().encode_h265.flags, flags);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.CodecInfo().encode_h265.flags, flags);

    // gopFrameCount mismatch
    uint32_t gop_frame_count = 12;
    std::swap(rc_info.CodecInfo().encode_h265.gopFrameCount, gop_frame_count);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.CodecInfo().encode_h265.gopFrameCount, gop_frame_count);

    // idrPeriod mismatch
    uint32_t idr_period = 30;
    std::swap(rc_info.CodecInfo().encode_h265.idrPeriod, idr_period);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.CodecInfo().encode_h265.idrPeriod, idr_period);

    // consecutiveBFrameCount mismatch
    uint32_t cons_b_frames = 4;
    std::swap(rc_info.CodecInfo().encode_h265.consecutiveBFrameCount, cons_b_frames);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.CodecInfo().encode_h265.consecutiveBFrameCount, cons_b_frames);

    // Layer useMinQp mismatch
    rc_info.Layer(0).CodecInfo().encode_h265.useMinQp = VK_FALSE;
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_h265.useMinQp = VK_TRUE;

    // Layer minQp.qpB mismatch
    rc_info.Layer(0).CodecInfo().encode_h265.minQp.qpB -= 1;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08297");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08299");
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_h265.minQp.qpB += 1;

    // Layer useMaxQp mismatch
    rc_info.Layer(0).CodecInfo().encode_h265.useMaxQp = VK_TRUE;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08298");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08300");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08375");
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_h265.useMaxQp = VK_FALSE;

    // Layer maxQp.qpP mismatch
    rc_info.Layer(0).CodecInfo().encode_h265.maxQp.qpP += 1;
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08298");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08300");
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_h265.maxQp.qpP -= 1;

    // Layer useMaxFrameSize mismatch
    rc_info.Layer(0).CodecInfo().encode_h265.useMaxFrameSize = VK_FALSE;
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    rc_info.Layer(0).CodecInfo().encode_h265.useMaxFrameSize = VK_TRUE;

    // Layer maxFrameSize.frameISize mismatch
    uint32_t max_frame_i_size = 12345;
    std::swap(rc_info.Layer(0).CodecInfo().encode_h265.maxFrameSize.frameISize, max_frame_i_size);
    rc_test_utils.TestRateControlStateMismatch(rc_info);
    std::swap(rc_info.Layer(0).CodecInfo().encode_h265.maxFrameSize.frameISize, max_frame_i_size);
}

TEST_F(NegativeVideo, DecodeSessionNotDecode) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - bound video session was not created with decode operation");

    RETURN_IF_SKIP(Init());

    VideoConfig decode_config = GetConfigDecode();
    VideoConfig encode_config = GetConfigEncode();
    if (!decode_config || !encode_config) {
        GTEST_SKIP() << "Test requires both video decode and video encode support";
    }

    VideoContext decode_context(m_device, decode_config);
    decode_context.CreateAndBindSessionMemory();
    decode_context.CreateResources();

    VideoContext encode_context(m_device, encode_config);
    encode_context.CreateAndBindSessionMemory();
    encode_context.CreateResources();

    vkt::CommandBuffer& cb = encode_context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(encode_context.Begin());

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-commandBuffer-cmdpool");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-08249");
    cb.DecodeVideo(decode_context.DecodeFrame());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(encode_context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeSessionNotEncode) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - bound video session was not created with encode operation");

    RETURN_IF_SKIP(Init());

    VideoConfig decode_config = GetConfigDecode();
    VideoConfig encode_config = GetConfigEncode();
    if (!decode_config || !encode_config) {
        GTEST_SKIP() << "Test requires both video decode and video encode support";
    }

    VideoContext decode_context(m_device, decode_config);
    decode_context.CreateAndBindSessionMemory();
    decode_context.CreateResources();

    VideoContext encode_context(m_device, encode_config);
    encode_context.CreateAndBindSessionMemory();
    encode_context.CreateResources();

    vkt::CommandBuffer& cb = decode_context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(decode_context.Begin());

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-commandBuffer-cmdpool");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-None-08250");
    cb.EncodeVideo(encode_context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(decode_context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeSessionUninitialized) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - session uninitialized");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.DecodeVideo(context.DecodeFrame());
    cb.EndVideoCoding(context.End());
    cb.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-07011");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeVideo, EncodeSessionUninitialized) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - session uninitialized");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.EncodeVideo(context.EncodeFrame());
    cb.EndVideoCoding(context.End());
    cb.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-None-07012");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeVideo, DecodeProtectedNoFaultBitstreamBuffer) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - protectedNoFault tests for bitstream buffer");

    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(Init());
    if (IsProtectedNoFaultSupported()) {
        GTEST_SKIP() << "Test requires protectedMemory support without protectedNoFault support";
    }

    VideoConfig config = GetConfigWithProtectedContent(GetConfigsDecode());
    if (!config) {
        GTEST_SKIP() << "Test requires a video decode profile with protected content support";
    }

    const bool use_protected = true;

    VideoContext unprotected_context(m_device, config);
    unprotected_context.CreateAndBindSessionMemory();
    unprotected_context.CreateResources(use_protected /* bitstream */, false /* DPB */, false /* dst image */);

    vkt::CommandBuffer& unprotected_cb = unprotected_context.CmdBuffer();

    VideoContext protected_context(m_device, config, use_protected);
    protected_context.CreateAndBindSessionMemory();
    protected_context.CreateResources(false /* bitstream */, use_protected /* DPB */, use_protected /* dst image */);

    vkt::CommandBuffer& protected_cb = protected_context.CmdBuffer();

    unprotected_cb.Begin();
    unprotected_cb.BeginVideoCoding(unprotected_context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-commandBuffer-07136");
    unprotected_cb.DecodeVideo(unprotected_context.DecodeFrame());
    m_errorMonitor->VerifyFound();

    unprotected_cb.EndVideoCoding(unprotected_context.End());
    unprotected_cb.End();

    protected_cb.Begin();
    protected_cb.BeginVideoCoding(protected_context.Begin());

    protected_cb.DecodeVideo(protected_context.DecodeFrame());

    protected_cb.EndVideoCoding(protected_context.End());
    protected_cb.End();
}

TEST_F(NegativeVideo, EncodeProtectedNoFaultBitstreamBuffer) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - protectedNoFault tests for bitstream buffer");

    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(Init());
    if (IsProtectedNoFaultSupported()) {
        GTEST_SKIP() << "Test requires protectedMemory support without protectedNoFault support";
    }

    const bool use_protected = true;

    VideoConfig config = GetConfigWithProtectedContent(GetConfigsEncode());
    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile with protected content support";
    }

    VideoContext unprotected_context(m_device, config);
    unprotected_context.CreateAndBindSessionMemory();
    unprotected_context.CreateResources(use_protected /* bitstream */, false /* DPB */, false /* src image */);

    vkt::CommandBuffer& unprotected_cb = unprotected_context.CmdBuffer();

    VideoContext protected_context(m_device, config, use_protected);
    protected_context.CreateAndBindSessionMemory();
    protected_context.CreateResources(false /* bitstream */, use_protected /* DPB */, use_protected /* src image */);

    vkt::CommandBuffer& protected_cb = protected_context.CmdBuffer();

    unprotected_cb.Begin();
    unprotected_cb.BeginVideoCoding(unprotected_context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-commandBuffer-08202");
    unprotected_cb.EncodeVideo(unprotected_context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    unprotected_cb.EndVideoCoding(unprotected_context.End());
    unprotected_cb.End();

    protected_cb.Begin();
    protected_cb.BeginVideoCoding(protected_context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-commandBuffer-08203");
    protected_cb.EncodeVideo(protected_context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    protected_cb.EndVideoCoding(protected_context.End());
    protected_cb.End();
}

TEST_F(NegativeVideo, DecodeProtectedNoFaultDecodeOutput) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - protectedNoFault tests for decode output");

    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(Init());
    if (IsProtectedNoFaultSupported()) {
        GTEST_SKIP() << "Test requires protectedMemory support without protectedNoFault support";
    }

    const bool use_protected = true;

    VideoConfig config = GetConfigWithProtectedContent(GetConfigsDecode());
    if (!config) {
        GTEST_SKIP() << "Test requires a video decode profile with protected content support";
    }

    VideoContext unprotected_context(m_device, config);
    unprotected_context.CreateAndBindSessionMemory();
    unprotected_context.CreateResources(false /* bitstream */, false /* DPB */, use_protected /* dst image */);

    vkt::CommandBuffer& unprotected_cb = unprotected_context.CmdBuffer();

    VideoContext protected_context(m_device, config, use_protected);
    protected_context.CreateAndBindSessionMemory();
    protected_context.CreateResources(use_protected /* bitstream */, use_protected /* DPB */, false /* dst image */);

    vkt::CommandBuffer& protected_cb = protected_context.CmdBuffer();

    unprotected_cb.Begin();
    unprotected_cb.BeginVideoCoding(unprotected_context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-commandBuffer-07147");
    unprotected_cb.DecodeVideo(unprotected_context.DecodeFrame());
    m_errorMonitor->VerifyFound();

    unprotected_cb.EndVideoCoding(unprotected_context.End());
    unprotected_cb.End();

    protected_cb.Begin();
    protected_cb.BeginVideoCoding(protected_context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-commandBuffer-07148");
    protected_cb.DecodeVideo(protected_context.DecodeFrame());
    m_errorMonitor->VerifyFound();

    protected_cb.EndVideoCoding(protected_context.End());
    protected_cb.End();
}

TEST_F(NegativeVideo, EncodeProtectedNoFaultEncodeInput) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - protectedNoFault tests for encode input");

    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(Init());
    if (IsProtectedNoFaultSupported()) {
        GTEST_SKIP() << "Test requires protectedMemory support without protectedNoFault support";
    }

    const bool use_protected = true;

    VideoConfig config = GetConfigWithProtectedContent(GetConfigsEncode());
    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile with protected content support";
    }

    VideoContext unprotected_context(m_device, config);
    unprotected_context.CreateAndBindSessionMemory();
    unprotected_context.CreateResources(false /* bitstream */, false /* DPB */, use_protected /* src image */);

    vkt::CommandBuffer& unprotected_cb = unprotected_context.CmdBuffer();

    VideoContext protected_context(m_device, config, use_protected);
    protected_context.CreateAndBindSessionMemory();
    protected_context.CreateResources(use_protected /* bitstream */, use_protected /* DPB */, false /* src image */);

    vkt::CommandBuffer& protected_cb = protected_context.CmdBuffer();

    unprotected_cb.Begin();
    unprotected_cb.BeginVideoCoding(unprotected_context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-commandBuffer-08211");
    unprotected_cb.EncodeVideo(unprotected_context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    unprotected_cb.EndVideoCoding(unprotected_context.End());
    unprotected_cb.End();

    protected_cb.Begin();
    protected_cb.BeginVideoCoding(protected_context.Begin());

    protected_cb.EncodeVideo(protected_context.EncodeFrame());

    protected_cb.EndVideoCoding(protected_context.End());
    protected_cb.End();
}

TEST_F(NegativeVideo, EncodeProtectedNoFaultQuantizationMap) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - protectedNoFault tests for quantization map");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(Init());
    if (IsProtectedNoFaultSupported()) {
        GTEST_SKIP() << "Test requires protectedMemory support without protectedNoFault support";
    }

    const bool use_protected = true;

    VideoConfig delta_config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.Caps()->flags & VK_VIDEO_CAPABILITY_PROTECTED_CONTENT_BIT_KHR) &&
               (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR) &&
               (config.SupportedQuantDeltaMapProps().size() > 0);
    }));
    VideoConfig emphasis_config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.Caps()->flags & VK_VIDEO_CAPABILITY_PROTECTED_CONTENT_BIT_KHR) &&
               (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR) &&
               (config.SupportedEmphasisMapProps().size() > 0);
    }));
    if (!delta_config && !emphasis_config) {
        GTEST_SKIP() << "Test requires an encode profile that supports protected content and quantization maps";
    }

    struct TestConfig {
        VideoConfig config;
        VkVideoEncodeFlagBitsKHR encode_flag;
        VkVideoSessionCreateFlagBitsKHR session_create_flag;
        const VkVideoFormatPropertiesKHR* map_props;
    };

    std::vector<TestConfig> tests;
    if (delta_config) {
        tests.emplace_back(TestConfig{delta_config, VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      delta_config.QuantDeltaMapProps()});
    }
    if (emphasis_config) {
        tests.emplace_back(TestConfig{emphasis_config, VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                      emphasis_config.EmphasisMapProps()});
    }

    for (auto& [config, encode_flag, session_create_flag, map_props] : tests) {
        config.SessionCreateInfo()->flags |= session_create_flag;

        VideoContext unprotected_context(m_device, config);
        unprotected_context.CreateAndBindSessionMemory();
        unprotected_context.CreateResources(false /* bitstream */, false /* DPB */, false /* src_image */);

        vkt::CommandBuffer& unprotected_cb = unprotected_context.CmdBuffer();

        VideoContext protected_context(m_device, config, use_protected);
        protected_context.CreateAndBindSessionMemory();
        protected_context.CreateResources(use_protected /* bitstream */, use_protected /* DPB */, use_protected /* src_image */);

        vkt::CommandBuffer& protected_cb = protected_context.CmdBuffer();

        const auto texel_size = config.GetQuantMapTexelSize(*map_props);
        auto unprotected_params = unprotected_context.CreateSessionParamsWithQuantMapTexelSize(texel_size);
        auto protected_params = protected_context.CreateSessionParamsWithQuantMapTexelSize(texel_size);

        VideoEncodeQuantizationMap unprotected_quantization_map(m_device, config, *map_props, false);
        VideoEncodeQuantizationMap protected_quantization_map(m_device, config, *map_props, true);

        unprotected_cb.Begin();
        unprotected_cb.BeginVideoCoding(unprotected_context.Begin().SetSessionParams(unprotected_params));

        if (encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-10313");
        unprotected_cb.EncodeVideo(
            unprotected_context.EncodeFrame().QuantizationMap(encode_flag, texel_size, protected_quantization_map));
        m_errorMonitor->VerifyFound();

        unprotected_cb.EndVideoCoding(unprotected_context.End());
        unprotected_cb.End();

        protected_cb.Begin();
        protected_cb.BeginVideoCoding(protected_context.Begin().SetSessionParams(protected_params));

        if (encode_flag == VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR) {
            // In case of emphasis map usage we will get an error about using default rate control mode
            m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10308");
        }

        protected_cb.EncodeVideo(
            protected_context.EncodeFrame().QuantizationMap(encode_flag, texel_size, unprotected_quantization_map));
        m_errorMonitor->VerifyFound();

        protected_cb.EndVideoCoding(protected_context.End());
        protected_cb.End();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideo, DecodeImageLayouts) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - pictures should be in the expected layout");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires a video decode profile with reference picture support and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));

    cb.DecodeVideo(context.DecodeFrame(0));

    // Decode output must be in DECODE_DST layout if it is distinct from reconstructed
    if (config.SupportsDecodeOutputDistinct()) {
        vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_GENERAL));
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07252");
        cb.DecodeVideo(context.DecodeFrame(0));
        m_errorMonitor->VerifyFound();
        vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, 0, 1));
    }

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));

    // Decode output must be in DECODE_DPB layout if it coincides with reconstructed
    if (config.SupportsDecodeOutputCoincide()) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07254");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07253");
        vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, 0, 1));
        cb.DecodeVideo(context.DecodeFrame(0).SetDecodeOutput(context.Dpb()->Picture(0)));
        m_errorMonitor->VerifyFound();
    }

    // Reconstructed must be in DECODE_DPB layout
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07253");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07254");
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_GENERAL, 0, 1));
    cb.DecodeVideo(context.DecodeFrame(0));
    m_errorMonitor->VerifyFound();
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, 0, 1));

    // Reference must be in DECODE_DPB layout
    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_GENERAL, 0, 1));
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pPictureResource-07255");
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeImageLayouts) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - pictures should be in the expected layout");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile with reference picture support and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.EncodeInput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR));

    cb.EncodeVideo(context.EncodeFrame(0));

    // Encode input must be in ENCODE_SRC layout
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08222");
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.EncodeInput()->LayoutTransition(VK_IMAGE_LAYOUT_GENERAL));
    cb.EncodeVideo(context.EncodeFrame(0));
    m_errorMonitor->VerifyFound();
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.EncodeInput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR));

    // Reconstructed must be in ENCODE_DPB layout
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08223");
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_GENERAL, 0, 1));
    cb.EncodeVideo(context.EncodeFrame(0));
    m_errorMonitor->VerifyFound();
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR, 0, 1));

    // Reference must be in ENCODE_DPB layout
    cb.EncodeVideo(context.EncodeReferenceFrame(0));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_GENERAL, 0, 1));
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pPictureResource-08224");
    cb.EncodeVideo(context.EncodeFrame(1).AddReferenceFrame(0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeInvalidResourceLayer) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - out-of-bounds layer index in VkVideoPictureResourceInfoKHR");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoPictureResourceInfoKHR dpb_res = context.Dpb()->Picture(0);
    dpb_res.baseArrayLayer = 5;

    VkVideoPictureResourceInfoKHR dst_res = context.DecodeOutput()->Picture();
    dst_res.baseArrayLayer = 5;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    // Invalid baseArrayLayer in VkVideoDecodeInfoKHR::dstPictureResource
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07141");
    m_errorMonitor->SetDesiredError("VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175");
    cb.DecodeVideo(context.DecodeFrame(0).SetDecodeOutput(dst_res));
    m_errorMonitor->VerifyFound();

    // Invalid baseArrayLayer in VkVideoDecodeInfoKHR::pSetupReferenceSlot
    if (config.SupportsDecodeOutputDistinct()) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07149");
        m_errorMonitor->SetDesiredError("VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175");
        cb.DecodeVideo(context.DecodeFrame(0, &dpb_res));
        m_errorMonitor->VerifyFound();
    } else {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07141");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07149");
        m_errorMonitor->SetDesiredError("VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175");
        m_errorMonitor->SetDesiredError("VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175");
        cb.DecodeVideo(context.DecodeFrame(0, &dpb_res).SetDecodeOutput(dst_res));
        m_errorMonitor->VerifyFound();
    }

    // Invalid baseArrayLayer in VkVideoDecodeInfoKHR::pReferenceSlots
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-slotIndex-07256");
    m_errorMonitor->SetDesiredError("VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175");
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0, &dpb_res));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeQueryTooManyOperations) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - no more queries available to store operation results");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video decode queue to support result status queries";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();
    context.CreateStatusQueryPool(2);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 0, 0);
    cb.DecodeVideo(context.DecodeFrame());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-opCount-07134");
    cb.DecodeVideo(context.DecodeFrame());
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(cb.handle(), context.StatusQueryPool(), 0);
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeQueryTooManyOperations) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - no more queries available to store operation results");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video encode queue to support result status queries";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();
    context.CreateStatusQueryPool(2);
    context.CreateEncodeFeedbackQueryPool(2);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 0, 0);
    cb.EncodeVideo(context.EncodeFrame());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-opCount-07174");
    cb.EncodeVideo(context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(cb.handle(), context.StatusQueryPool(), 0);
    cb.EndVideoCoding(context.End());
    cb.End();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    vk::CmdBeginQuery(cb.handle(), context.EncodeFeedbackQueryPool(), 0, 0);
    cb.EncodeVideo(context.EncodeFrame());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-opCount-07174");
    cb.EncodeVideo(context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(cb.handle(), context.EncodeFeedbackQueryPool(), 0);
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeIncompatBufferProfile) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - bitstream buffer must be compatible with the video profile");

    RETURN_IF_SKIP(Init());

    auto configs = GetConfigsDecode();
    if (configs.size() < 2) {
        GTEST_SKIP() << "Test requires two video decode profiles";
    }

    VideoContext context1(m_device, configs[0]);
    VideoContext context2(m_device, configs[1]);
    context1.CreateAndBindSessionMemory();
    context1.CreateResources();
    context2.CreateAndBindSessionMemory();
    context2.CreateResources();

    vkt::CommandBuffer& cb = context1.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07135");
    cb.DecodeVideo(context1.DecodeFrame().SetBitstream(context2.Bitstream()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeIncompatBufferProfile) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - bitstream buffer must be compatible with the video profile");

    RETURN_IF_SKIP(Init());

    auto configs = GetConfigsEncode();
    if (configs.size() < 2) {
        GTEST_SKIP() << "Test requires two video encode profiles";
    }

    VideoContext context1(m_device, configs[0]);
    VideoContext context2(m_device, configs[1]);
    context1.CreateAndBindSessionMemory();
    context1.CreateResources();
    context2.CreateAndBindSessionMemory();
    context2.CreateResources();

    vkt::CommandBuffer& cb = context1.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08201");
    cb.EncodeVideo(context1.EncodeFrame().SetBitstream(context2.Bitstream()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeBufferMissingDecodeSrcUsage) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - bitstream buffer missing DECODE_SRC usage");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    auto profile_list = vku::InitStruct<VkVideoProfileListInfoKHR>();
    profile_list.profileCount = 1;
    profile_list.pProfiles = config.Profile();

    auto create_info = vku::InitStruct<VkBufferCreateInfo>(&profile_list);
    create_info.flags = 0;
    create_info.size = std::max((VkDeviceSize)4096, config.Caps()->minBitstreamBufferSizeAlignment);
    create_info.usage = VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkt::Buffer buffer(*m_device, create_info);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeInfoKHR-srcBuffer-07165");
    cb.DecodeVideo(context.DecodeFrame().SetBitstreamBuffer(buffer.handle(), 0, create_info.size));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeBufferMissingEncodeDstUsage) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - bitstream buffer missing ENCODE_DST usage");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    auto profile_list = vku::InitStruct<VkVideoProfileListInfoKHR>();
    profile_list.profileCount = 1;
    profile_list.pProfiles = config.Profile();

    auto create_info = vku::InitStruct<VkBufferCreateInfo>(&profile_list);
    create_info.flags = 0;
    create_info.size = std::max((VkDeviceSize)4096, config.Caps()->minBitstreamBufferSizeAlignment);
    create_info.usage = VK_BUFFER_USAGE_VIDEO_ENCODE_SRC_BIT_KHR;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkt::Buffer buffer(*m_device, create_info);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeInfoKHR-dstBuffer-08236");
    cb.EncodeVideo(context.EncodeFrame().SetBitstreamBuffer(buffer.handle(), 0, create_info.size));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeBufferOffsetOutOfBounds) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - bitstream buffer offset out of bounds");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoDecodeInfo decode_info = context.DecodeFrame();
    decode_info->srcBufferOffset = decode_info->srcBufferRange;
    decode_info->srcBufferOffset += config.Caps()->minBitstreamBufferOffsetAlignment - 1;
    decode_info->srcBufferOffset &= ~(config.Caps()->minBitstreamBufferOffsetAlignment - 1);

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeInfoKHR-srcBufferOffset-07166");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoDecodeInfoKHR-srcBufferRange-07167");
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeBufferOffsetOutOfBounds) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - bitstream buffer offset out of bounds");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();
    encode_info->dstBufferOffset = encode_info->dstBufferRange;
    encode_info->dstBufferOffset += config.Caps()->minBitstreamBufferOffsetAlignment - 1;
    encode_info->dstBufferOffset &= ~(config.Caps()->minBitstreamBufferOffsetAlignment - 1);

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeInfoKHR-dstBufferOffset-08237");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeInfoKHR-dstBufferRange-08238");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeBufferOffsetAlignment) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - bitstream buffer offset needs to be aligned");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(FilterConfigs(
        GetConfigsDecode(), [](const VideoConfig& config) { return config.Caps()->minBitstreamBufferOffsetAlignment > 1; }));
    if (!config) {
        GTEST_SKIP() << "Test requires a video decode profile with minBitstreamBufferOffsetAlignment > 1";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoDecodeInfo decode_info = context.DecodeFrame();
    ++decode_info->srcBufferOffset;
    decode_info->srcBufferRange -= config.Caps()->minBitstreamBufferSizeAlignment;

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07138");
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeBufferOffsetAlignment) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - bitstream buffer offset needs to be aligned");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(FilterConfigs(
        GetConfigsEncode(), [](const VideoConfig& config) { return config.Caps()->minBitstreamBufferOffsetAlignment > 1; }));
    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile with minBitstreamBufferOffsetAlignment > 1";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();
    ++encode_info->dstBufferOffset;
    encode_info->dstBufferRange -= config.Caps()->minBitstreamBufferSizeAlignment;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08204");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeBufferRangeOutOfBounds) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - bitstream buffer range out of bounds");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoDecodeInfo decode_info = context.DecodeFrame();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeInfoKHR-srcBufferRange-07167");
    decode_info->srcBufferOffset += config.Caps()->minBitstreamBufferOffsetAlignment;
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeInfoKHR-srcBufferRange-07167");
    decode_info->srcBufferOffset = 0;
    decode_info->srcBufferRange += config.Caps()->minBitstreamBufferSizeAlignment;
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeBufferRangeOutOfBounds) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - bitstream buffer range out of bounds");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeInfoKHR-dstBufferRange-08238");
    encode_info->dstBufferOffset += config.Caps()->minBitstreamBufferOffsetAlignment;
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeInfoKHR-dstBufferRange-08238");
    encode_info->dstBufferOffset = 0;
    encode_info->dstBufferRange += config.Caps()->minBitstreamBufferSizeAlignment;
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeBufferRangeAlignment) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - bitstream buffer range needs to be aligned");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(FilterConfigs(
        GetConfigsDecode(), [](const VideoConfig& config) { return config.Caps()->minBitstreamBufferSizeAlignment > 1; }));
    if (!config) {
        GTEST_SKIP() << "Test requires a video decode profile with minBitstreamBufferSizeAlignment > 1";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoDecodeInfo decode_info = context.DecodeFrame();
    --decode_info->srcBufferRange;

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07139");
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeBufferRangeAlignment) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - bitstream buffer range needs to be aligned");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(FilterConfigs(
        GetConfigsEncode(), [](const VideoConfig& config) { return config.Caps()->minBitstreamBufferSizeAlignment > 1; }));
    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile with minBitstreamBufferSizeAlignment > 1";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();
    --encode_info->dstBufferRange;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08205");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeInvalidOutputAndSetupCoincide) {
    TEST_DESCRIPTION(
        "vkCmdDecodeVideoKHR - decode output and reconstructed pictures must not match "
        "if VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR is not supported");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(FilterConfigs(GetConfigsWithReferences(GetConfigsDecode()),
                                          [](const VideoConfig& config) { return !config.SupportsDecodeOutputCoincide(); }));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures and no support "
                        "for VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    // It is possible that the DPB uses a different format, so ignore format mismatch errors
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07143");

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07146");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07140");
    cb.DecodeVideo(context.DecodeFrame(0).SetDecodeOutput(context.Dpb()->Picture(0)));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeInvalidOutputAndSetupDistinct) {
    TEST_DESCRIPTION(
        "vkCmdDecodeVideoKHR - decode output and reconstructed pictures must match "
        "if VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR is not supported");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(FilterConfigs(GetConfigsWithReferences(GetConfigsDecode()),
                                          [](const VideoConfig& config) { return !config.SupportsDecodeOutputDistinct(); }));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures and no support "
                        "for VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07141");
    cb.DecodeVideo(context.DecodeFrame(0).SetDecodeOutput(context.DecodeOutput()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeInvalidSetupSlotIndex) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - invalid slot index specified for reconstructed picture");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsDecode(), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with 3 reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    VideoDecodeInfo decode_info = context.DecodeFrame(0);
    auto setup_slot = *decode_info->pSetupReferenceSlot;
    decode_info->pSetupReferenceSlot = &setup_slot;

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeInfoKHR-pSetupReferenceSlot-07168");
    setup_slot.slotIndex = -1;
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07170");
    setup_slot.slotIndex = 2;
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeInvalidSetupSlotIndex) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - invalid slot index specified for reconstructed picture");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsEncode(), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with 3 reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    VideoEncodeInfo encode_info = context.EncodeFrame(0);
    auto setup_slot = *encode_info->pSetupReferenceSlot;
    encode_info->pSetupReferenceSlot = &setup_slot;

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeInfoKHR-pSetupReferenceSlot-08239");
    setup_slot.slotIndex = -1;
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08213");
    setup_slot.slotIndex = 2;
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeInvalidRefSlotIndex) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - invalid slot index specified for reference picture");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsDecode(), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with 3 reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeInfoKHR-slotIndex-07171");
    cb.DecodeVideo(context.DecodeFrame(0).AddReferenceFrame(-1, 0));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-slotIndex-07256");
    cb.DecodeVideo(context.DecodeFrame(0).AddReferenceFrame(2, 0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeInvalidRefSlotIndex) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - invalid slot index specified for reference picture");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsEncode(), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with 3 reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeInfoKHR-slotIndex-08241");
    cb.EncodeVideo(context.EncodeFrame(0).AddReferenceFrame(-1, 0));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pPictureResource-08219");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-slotIndex-08217");
    cb.EncodeVideo(context.EncodeFrame(0).AddReferenceFrame(2, 0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeSetupNull) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - reconstructed picture is required for sessions with DPB slots");

    RETURN_IF_SKIP(Init())

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    VideoDecodeInfo decode_info = context.DecodeFrame(0);
    decode_info->pSetupReferenceSlot = NULL;

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-08376");
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeSetupResourceNull) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - reconstructed picture resource must not be NULL");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    VideoDecodeInfo decode_info = context.DecodeFrame(0);
    auto setup_slot = *decode_info->pSetupReferenceSlot;
    decode_info->pSetupReferenceSlot = &setup_slot;

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeInfoKHR-pSetupReferenceSlot-07169");
    setup_slot.pPictureResource = nullptr;
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeSetupNull) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - reconstructed picture is required for sessions with DPB slots");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    VideoEncodeInfo encode_info = context.EncodeFrame(0);
    encode_info->pSetupReferenceSlot = NULL;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08377");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeSetupResourceNull) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - reconstructed picture resource must not be NULL");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    VideoEncodeInfo encode_info = context.EncodeFrame(0);
    auto setup_slot = *encode_info->pSetupReferenceSlot;
    encode_info->pSetupReferenceSlot = &setup_slot;

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeInfoKHR-pSetupReferenceSlot-08240");
    setup_slot.pPictureResource = nullptr;
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeReferenceResourceNull) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - reference picture resource must not be NULL");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures with 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeInfoKHR-pPictureResource-07172");
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0, nullptr));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeReferenceResourceNull) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - reference picture resource must not be NULL");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with reference pictures with 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeInfoKHR-pPictureResource-08242");
    cb.EncodeVideo(context.EncodeFrame(1).AddReferenceFrame(0, nullptr));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeIncompatOutputPicProfile) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - decode output picture must be compatible with the video profile");

    RETURN_IF_SKIP(Init());

    VideoConfig configs[2] = {};
    const auto& all_configs = GetConfigsDecode();
    for (uint32_t i = 0; i < all_configs.size(); ++i) {
        for (uint32_t j = i + 1; j < all_configs.size(); ++j) {
            const auto& coded_extent1 = all_configs[i].SessionCreateInfo()->maxCodedExtent;
            const auto& coded_extent2 = all_configs[j].SessionCreateInfo()->maxCodedExtent;
            const auto& pic_format1 = *all_configs[i].DpbFormatProps();
            const auto& pic_format2 = *all_configs[j].DpbFormatProps();
            if ((coded_extent1.width == coded_extent2.width) && (coded_extent1.height == coded_extent2.height) &&
                (pic_format1.imageType == pic_format2.imageType) && (pic_format1.imageTiling == pic_format2.imageTiling) &&
                (pic_format1.format == pic_format2.format) && (pic_format1.imageUsageFlags == pic_format2.imageUsageFlags)) {
                configs[0] = all_configs[i];
                configs[1] = all_configs[j];
            }
        }
    }
    if (!configs[0]) {
        GTEST_SKIP() << "Test requires two video profiles with matching decode output format/size";
    }

    VideoContext context1(m_device, configs[0]);
    VideoContext context2(m_device, configs[1]);
    context1.CreateAndBindSessionMemory();
    context1.CreateResources();
    context2.CreateAndBindSessionMemory();
    context2.CreateResources();

    vkt::CommandBuffer& cb = context1.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07142");
    cb.DecodeVideo(context1.DecodeFrame().SetDecodeOutput(context2.DecodeOutput()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeIncompatInputPicProfile) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - encode input picture must be compatible with the video profile");

    RETURN_IF_SKIP(Init());

    VideoConfig configs[2] = {};
    const auto& all_configs = GetConfigsEncode();
    for (uint32_t i = 0; i < all_configs.size(); ++i) {
        for (uint32_t j = i + 1; j < all_configs.size(); ++j) {
            const auto& coded_extent1 = all_configs[i].SessionCreateInfo()->maxCodedExtent;
            const auto& coded_extent2 = all_configs[j].SessionCreateInfo()->maxCodedExtent;
            const auto& pic_format1 = *all_configs[i].DpbFormatProps();
            const auto& pic_format2 = *all_configs[j].DpbFormatProps();
            if ((coded_extent1.width == coded_extent2.width) && (coded_extent1.height == coded_extent2.height) &&
                (pic_format1.imageType == pic_format2.imageType) && (pic_format1.imageTiling == pic_format2.imageTiling) &&
                (pic_format1.format == pic_format2.format) && (pic_format1.imageUsageFlags == pic_format2.imageUsageFlags)) {
                configs[0] = all_configs[i];
                configs[1] = all_configs[j];
            }
        }
    }
    if (!configs[0]) {
        GTEST_SKIP() << "Test requires two video profiles with matching encode input format/size";
    }

    VideoContext context1(m_device, configs[0]);
    VideoContext context2(m_device, configs[1]);
    context1.CreateAndBindSessionMemory();
    context1.CreateResources();
    context2.CreateAndBindSessionMemory();
    context2.CreateResources();

    vkt::CommandBuffer& cb = context1.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08206");
    cb.EncodeVideo(context1.EncodeFrame().SetEncodeInput(context2.EncodeInput()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeOutputFormatMismatch) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - decode output picture format mismatch");

    RETURN_IF_SKIP(Init());

    uint32_t alt_pic_format_index = 0;
    VideoConfig config = GetConfig(FilterConfigs(GetConfigsDecode(), [&alt_pic_format_index](const VideoConfig& config) {
        const auto& format_props = config.SupportedPictureFormatProps();
        for (size_t i = 0; i < format_props.size(); ++i) {
            if (format_props[i].format != format_props[0].format && alt_pic_format_index == 0) {
                alt_pic_format_index = i;
                return true;
            }
        }
        return false;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires a video decode profile with support for two output picture formats";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoConfig config2 = config;
    config2.SetFormatProps({config.SupportedPictureFormatProps()[alt_pic_format_index]}, config.SupportedDpbFormatProps());
    VideoDecodeOutput output(m_device, config2);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07143");
    cb.DecodeVideo(context.DecodeFrame().SetDecodeOutput(output.Picture()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeInputFormatMismatch) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - encode input picture format mismatch");

    RETURN_IF_SKIP(Init());

    uint32_t alt_pic_format_index = 0;
    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [&alt_pic_format_index](const VideoConfig& config) {
        const auto& format_props = config.SupportedPictureFormatProps();
        for (size_t i = 0; i < format_props.size(); ++i) {
            if (format_props[i].format != format_props[0].format && alt_pic_format_index == 0) {
                alt_pic_format_index = i;
                return true;
            }
        }
        return false;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile with support for two input picture formats";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoConfig config2 = config;
    config2.SetFormatProps({config.SupportedPictureFormatProps()[alt_pic_format_index]}, config.SupportedDpbFormatProps());
    VideoEncodeInput output(m_device, config2);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08207");
    cb.EncodeVideo(context.EncodeFrame().SetEncodeInput(output.Picture()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeOutputMissingDecodeDstUsage) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - decode output picture resource missing VIDEO_DECODE_DST usage");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    if (config.PictureFormatProps()->imageUsageFlags == VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR) {
        GTEST_SKIP() << "Test requires output format to support at least one more usage besides DECODE_DST";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto view_usage_ci = vku::InitStruct<VkImageViewUsageCreateInfo>();
    view_usage_ci.usage = config.PictureFormatProps()->imageUsageFlags ^ VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR;

    auto image_view_ci = vku::InitStruct<VkImageViewCreateInfo>(&view_usage_ci);
    image_view_ci.image = context.DecodeOutput()->Image();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    image_view_ci.format = config.PictureFormatProps()->format;
    image_view_ci.components = config.PictureFormatProps()->componentMapping;
    image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    vkt::ImageView image_view(*m_device, image_view_ci);

    VkVideoPictureResourceInfoKHR dst_res = context.DecodeOutput()->Picture();
    dst_res.imageViewBinding = image_view;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07146");
    cb.DecodeVideo(context.DecodeFrame().SetDecodeOutput(dst_res));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeInputMissingEncodeSrcUsage) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - encode input picture resource missing VIDEO_ENCODE_SRC usage");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    if (config.PictureFormatProps()->imageUsageFlags == VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR) {
        GTEST_SKIP() << "Test requires input format to support at least one more usage besides ENCODE_SRC";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto view_usage_ci = vku::InitStruct<VkImageViewUsageCreateInfo>();
    view_usage_ci.usage = config.PictureFormatProps()->imageUsageFlags ^ VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR;

    auto image_view_ci = vku::InitStruct<VkImageViewCreateInfo>(&view_usage_ci);
    image_view_ci.image = context.EncodeInput()->Image();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    image_view_ci.format = config.PictureFormatProps()->format;
    image_view_ci.components = config.PictureFormatProps()->componentMapping;
    image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    vkt::ImageView image_view(*m_device, image_view_ci);

    VkVideoPictureResourceInfoKHR src_res = context.EncodeInput()->Picture();
    src_res.imageViewBinding = image_view.handle();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08210");
    cb.EncodeVideo(context.EncodeFrame().SetEncodeInput(src_res));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeOutputCodedOffsetExtent) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - invalid decode output picture coded offset/extent");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VkVideoPictureResourceInfoKHR dst_res = context.DecodeOutput()->Picture();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07144");
    ++dst_res.codedOffset.x;
    cb.DecodeVideo(context.DecodeFrame().SetDecodeOutput(dst_res));
    --dst_res.codedOffset.x;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07144");
    ++dst_res.codedOffset.y;
    cb.DecodeVideo(context.DecodeFrame().SetDecodeOutput(dst_res));
    --dst_res.codedOffset.y;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07145");
    ++dst_res.codedExtent.width;
    cb.DecodeVideo(context.DecodeFrame().SetDecodeOutput(dst_res));
    --dst_res.codedExtent.width;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07145");
    ++dst_res.codedExtent.height;
    cb.DecodeVideo(context.DecodeFrame().SetDecodeOutput(dst_res));
    --dst_res.codedExtent.height;
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeInputCodedOffsetExtent) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - invalid encode input picture coded offset/extent");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VkVideoPictureResourceInfoKHR src_res = context.EncodeInput()->Picture();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08208");
    ++src_res.codedOffset.x;
    cb.EncodeVideo(context.EncodeFrame().SetEncodeInput(src_res));
    --src_res.codedOffset.x;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08208");
    ++src_res.codedOffset.y;
    cb.EncodeVideo(context.EncodeFrame().SetEncodeInput(src_res));
    --src_res.codedOffset.y;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08209");
    ++src_res.codedExtent.width;
    cb.EncodeVideo(context.EncodeFrame().SetEncodeInput(src_res));
    --src_res.codedExtent.width;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08209");
    ++src_res.codedExtent.height;
    cb.EncodeVideo(context.EncodeFrame().SetEncodeInput(src_res));
    --src_res.codedExtent.height;
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeSetupAndRefCodedOffset) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - invalid reconstructed/reference picture coded offset");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures with 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);

    if (!config.SupportsDecodeOutputDistinct()) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07144");
    }
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07149");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07173");
    ++res.codedOffset.x;
    cb.DecodeVideo(context.DecodeFrame(0, &res));
    --res.codedOffset.x;
    m_errorMonitor->VerifyFound();

    if (!config.SupportsDecodeOutputDistinct()) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07144");
    }
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07149");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07173");
    ++res.codedOffset.y;
    cb.DecodeVideo(context.DecodeFrame(0, &res));
    --res.codedOffset.y;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-codedOffset-07257");
    ++res.codedOffset.x;
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0, &res));
    --res.codedOffset.x;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-codedOffset-07257");
    ++res.codedOffset.y;
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0, &res));
    --res.codedOffset.y;
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeSetupAndRefCodedOffset) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - invalid reconstructed/reference picture coded offset");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with reference pictures with 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08215");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08214");
    ++res.codedOffset.x;
    cb.EncodeVideo(context.EncodeFrame(0, &res));
    --res.codedOffset.x;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08215");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08214");
    ++res.codedOffset.y;
    cb.EncodeVideo(context.EncodeFrame(0, &res));
    --res.codedOffset.y;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pPictureResource-08219");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-codedOffset-08218");
    ++res.codedOffset.x;
    cb.EncodeVideo(context.EncodeFrame(1).AddReferenceFrame(0, &res));
    --res.codedOffset.x;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pPictureResource-08219");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-codedOffset-08218");
    ++res.codedOffset.y;
    cb.EncodeVideo(context.EncodeFrame(1).AddReferenceFrame(0, &res));
    --res.codedOffset.y;
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeSetupResourceNotBound) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - used reconstructed picture resource is not bound");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07149");
    cb.DecodeVideo(context.DecodeFrame(0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeSetupResourceNotBound) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - used reconstructed picture resource is not bound");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08215");
    cb.EncodeVideo(context.EncodeFrame(0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeRefResourceNotBoundToDPBSlot) {
    TEST_DESCRIPTION(
        "vkCmdDecodeVideoKHR - used reference picture resource is not bound as a resource "
        "currently associated with the corresponding DPB slot");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures and 3 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = config.Caps()->maxDpbSlots;
    config.SessionCreateInfo()->maxActiveReferencePictures = config.Caps()->maxActiveReferencePictures;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 2));

    // Trying to refer to reference picture resource that is not bound at all
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(1));
    m_errorMonitor->VerifyFound();

    // Trying to refer to bound reference picture resource, but with incorrect slot index
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(1, 0));
    m_errorMonitor->VerifyFound();

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, 0, 1));
    cb.DecodeVideo(context.DecodeReferenceFrame(1, 0));
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(1, 0));

    // Trying to refer to bound reference picture resource, but with incorrect slot index after
    // the associated DPB slot index has been updated within the video coding scope
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0, 0));
    m_errorMonitor->VerifyFound();

    cb.ControlVideoCoding(context.Control().Reset());

    // Trying to refer to bound reference picture resource after reset
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(1, 0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeRefResourceNotBoundToDPBSlot) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - used reference picture resource is not bound as a resource "
        "currently associated with the corresponding DPB slot");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncode()), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with reference pictures and 3 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = config.Caps()->maxDpbSlots;
    config.SessionCreateInfo()->maxActiveReferencePictures = config.Caps()->maxActiveReferencePictures;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 2));

    // Trying to refer to reference picture resource that is not bound at all
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pPictureResource-08219");
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(1));
    m_errorMonitor->VerifyFound();

    // Trying to refer to bound reference picture resource, but with incorrect slot index
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pPictureResource-08219");
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(1, 0));
    m_errorMonitor->VerifyFound();

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.EncodeInput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR, 0, 1));
    cb.EncodeVideo(context.EncodeReferenceFrame(1, 0));
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(1, 0));

    // Trying to refer to bound reference picture resource, but with incorrect slot index after
    // the associated DPB slot index has been updated within the video coding scope
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pPictureResource-08219");
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(0, 0));
    m_errorMonitor->VerifyFound();

    cb.ControlVideoCoding(context.Control().Reset());

    // Trying to refer to bound reference picture resource after reset
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pPictureResource-08219");
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(1, 0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeTooManyReferences) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - reference picture count exceeds maxActiveReferencePictures");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures and 3 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 3;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(-1, 2));

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-activeReferencePictureCount-07150");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0).AddReferenceFrame(1));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeTooManyReferences) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - reference picture count exceeds maxActiveReferencePictures");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncode()), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with reference pictures and 3 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 3;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(-1, 2));

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-activeReferencePictureCount-08216");
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(0).AddReferenceFrame(1));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeTooManyReferencesH264Interlaced) {
    TEST_DESCRIPTION(
        "vkCmdDecodeVideoKHR - reference picture count exceeds maxActiveReferencePictures"
        " (specific test for H.264 interlaced with both top and bottom field referenced)");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecodeH264Interlaced(), 2), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with 2 reference pictures and 3 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 3;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(-1, 2));

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-activeReferencePictureCount-07150");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0).AddReferenceBothFields(1));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-activeReferencePictureCount-07150");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceBothFields(1).AddReferenceFrame(0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeDuplicateRefResource) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - same reference picture resource is used twice");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode(), 2), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with 2 reference pictures and 3 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 3;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 2));

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07264");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0, &res).AddReferenceFrame(1, &res));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-dpbFrameUseCount-07176");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07264");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0, &res).AddReferenceFrame(0, &res));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeDuplicateRefResourceH264Interlaced) {
    TEST_DESCRIPTION(
        "vkCmdDecodeVideoKHR - same reference picture resource is used twice "
        "with one referring to the top field and another referring to the bottom field");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsDecodeH264Interlaced(), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with 2 reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07264");
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceTopField(0, 0).AddReferenceBottomField(0, 0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeDuplicateRefResource) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - same reference picture resource is used twice");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncode(), 2), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with 2 reference pictures and 3 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 3;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 2));

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pPictureResource-08219");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pPictureResource-08220");
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(0, &res).AddReferenceFrame(1, &res));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-dpbFrameUseCount-08221");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pPictureResource-08220");
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(0, &res).AddReferenceFrame(0, &res));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeDuplicateFrame) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - same DPB frame reference is used twice");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode(), 2), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with 2 reference pictures and 3 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 3;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(0, 1).AddResource(-1, 2));

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-dpbFrameUseCount-07176");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0, 0).AddReferenceFrame(0, 1));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeDuplicateFrame) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - same DPB frame reference is used twice");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncode(), 2), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with 2 reference pictures and 3 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 3;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(0, 1).AddResource(-1, 2));

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-dpbFrameUseCount-08221");
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(0, 0).AddReferenceFrame(0, 1));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeDuplicateFrameFieldH264Interlaced) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - same DPB frame, top field, or bottom field reference is used twice");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecodeH264Interlaced(), 4), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with 4 reference pictures and 3 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 3;
    config.SessionCreateInfo()->maxActiveReferencePictures = 4;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(0, 1).AddResource(-1, 2));

    // Same DPB frame is used twice
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-dpbFrameUseCount-07176");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0, 0).AddReferenceFrame(0, 1));
    m_errorMonitor->VerifyFound();

    // Same DPB top field is used twice
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-dpbTopFieldUseCount-07177");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceTopField(0, 0).AddReferenceTopField(0, 1));
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-dpbTopFieldUseCount-07177");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceTopField(0, 0).AddReferenceBothFields(0, 1));
    m_errorMonitor->VerifyFound();

    // Same DPB bottom field is used twice
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-dpbBottomFieldUseCount-07178");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceBottomField(0, 0).AddReferenceBottomField(0, 1));
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-dpbBottomFieldUseCount-07178");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceBottomField(0, 0).AddReferenceBothFields(0, 1));
    m_errorMonitor->VerifyFound();

    // Same DPB top & bottom field is used twice
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-dpbTopFieldUseCount-07177");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-dpbBottomFieldUseCount-07178");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceBothFields(0, 0).AddReferenceBothFields(0, 1));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeImplicitDeactivation) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - test DPB slot deactivation caused by reference invalidation");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference picture support and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    // Setup frame in DPB slot then use it as non-setup reconstructed picture
    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    cb.DecodeVideo(context.DecodeFrame(0));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();

    // Try to include the DPB slot expecting reference picture association at begin-time
    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.EndVideoCoding(context.End());
    cb.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeVideo, DecodeImplicitDeactivationH264Interlaced) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - test DPB slot deactivation caused by H.264 interlaced reference invalidation");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecodeH264Interlaced()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires an H.264 interlaced decode profile with reference picture support and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    {
        // Setup frame in DPB slot then use it as non-setup reconstructed picture for a top field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceFrame(0));
        cb.DecodeVideo(context.DecodeTopField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to include the DPB slot expecting reference picture association at begin-time
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup frame in DPB slot then use it as non-setup reconstructed picture for a bottom field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceFrame(0));
        cb.DecodeVideo(context.DecodeTopField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to include the DPB slot expecting reference picture association at begin-time
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup top field in DPB slot then use it as non-setup reconstructed picture for a frame
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceTopField(0));
        cb.DecodeVideo(context.DecodeFrame(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to include the DPB slot expecting reference picture association at begin-time
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup top field in DPB slot then use it as non-setup reconstructed picture for a top field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceTopField(0));
        cb.DecodeVideo(context.DecodeTopField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to include the DPB slot expecting reference picture association at begin-time
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup bottom field in DPB slot then use it as non-setup reconstructed picture for a frame
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceBottomField(0));
        cb.DecodeVideo(context.DecodeFrame(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to include the DPB slot expecting reference picture association at begin-time
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup bottom field in DPB slot then use it as non-setup reconstructed picture for a bottom field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceTopField(0));
        cb.DecodeVideo(context.DecodeTopField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to include the DPB slot expecting reference picture association at begin-time
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }
}

TEST_F(NegativeVideo, DecodeRefPictureKindMismatchH264) {
    TEST_DESCRIPTION(
        "vkCmdDecodeVideoKHR - H.264 reference picture kind (frame, top field, bottom field) mismatch "
        "between actual DPB slot contents and specified reference pictures");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecodeH264Interlaced()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires an H.264 interlaced decode profile with reference picture support and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    {
        // Setup frame in DPB slot
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceFrame(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to reference DPB slot as top field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceTopField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07267");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();

        // Try to reference DPB slot as bottom field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceBottomField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07268");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup top field in DPB slot
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceTopField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to reference DPB slot as frame
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07266");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();

        // Try to reference DPB slot as bottom field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceBottomField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07268");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup bottom field in DPB slot
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceBottomField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to reference DPB slot as frame
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07266");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();

        // Try to reference DPB slot as top field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceTopField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07267");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }
}

TEST_F(NegativeVideo, DecodeInvalidationOnlyH264Interlaced) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - test H.264 interlaced reference invalidation without implicit DPB slot deactivation");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecodeH264Interlaced()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires an H.264 interlaced decode profile with reference picture support and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    {
        // Setup top and bottom field in DPB slot then use it as non-setup reconstructed picture for a top field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceTopField(0));
        cb.DecodeVideo(context.DecodeReferenceBottomField(0));
        cb.DecodeVideo(context.DecodeTopField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to reference DPB slot as top field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceTopField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07267");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup top and bottom field in DPB slot then use it as non-setup reconstructed picture for a bottom field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceTopField(0));
        cb.DecodeVideo(context.DecodeReferenceBottomField(0));
        cb.DecodeVideo(context.DecodeBottomField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to reference DPB slot as bottom field
        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceBottomField(0));
        cb.EndVideoCoding(context.End());
        cb.End();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07268");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }
}

TEST_F(NegativeVideo, DecodeInvalidCodecInfoH264) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - invalid/missing H.264 codec-specific information");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecodeH264(), 2), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support with 2 reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VideoDecodeInfo decode_info = context.DecodeFrame(0);

    StdVideoDecodeH264PictureInfo std_picture_info{};
    auto picture_info = vku::InitStruct<VkVideoDecodeH264PictureInfoKHR>();
    uint32_t slice_offset = 0;
    picture_info.pStdPictureInfo = &std_picture_info;
    picture_info.sliceCount = 1;
    picture_info.pSliceOffsets = &slice_offset;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    // Missing H.264 picture info
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = nullptr;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-07152");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Decode output must be a frame if session does not support interlaced frames
    {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07259");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-07258");
        cb.DecodeVideo(context.DecodeTopField(0));
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07259");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-07258");
        cb.DecodeVideo(context.DecodeBottomField(0));
        m_errorMonitor->VerifyFound();
    }

    // Slice offsets must be within buffer range
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pSliceOffsets-07153");
        slice_offset = (uint32_t)decode_info->srcBufferRange;
        cb.DecodeVideo(decode_info);
        slice_offset = 0;
        m_errorMonitor->VerifyFound();
    }

    // No matching SPS/PPS
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-StdVideoH264SequenceParameterSet-07154");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-StdVideoH264PictureParameterSet-07155");
        std_picture_info.seq_parameter_set_id = 1;
        cb.DecodeVideo(decode_info);
        std_picture_info.seq_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-StdVideoH264PictureParameterSet-07155");
        std_picture_info.pic_parameter_set_id = 1;
        cb.DecodeVideo(decode_info);
        std_picture_info.pic_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();
    }

    // Missing H.264 setup reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;
        decode_info->pSetupReferenceSlot = &slot;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07141");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07156");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Reconstructed picture must be a frame if session does not support interlaced frames
    {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07261");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07259");
        cb.DecodeVideo(context.DecodeReferenceTopField(0).SetFrame(true /* override decode output */));
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07261");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07259");
        cb.DecodeVideo(context.DecodeReferenceBottomField(0).SetFrame(true /* override decode output */));
        m_errorMonitor->VerifyFound();
    }

    // Missing H.264 reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        decode_info = context.DecodeFrame(1);
        decode_info->pNext = &picture_info;
        decode_info->referenceSlotCount = 1;
        decode_info->pReferenceSlots = &slot;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-07157");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Reference picture must be a frame if session does not support interlaced frames
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07260");
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceTopField(0));
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07260");
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceBottomField(0));
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07260");
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceBothFields(0));
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeFieldFrameMismatchH264) {
    TEST_DESCRIPTION(
        "vkCmdDecodeVideoKHR - H.264 interlaced field/frame mismatch between "
        "decode output picture and reconstructed picture");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecodeH264Interlaced()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 interlaced decode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    // Decode output is frame but reconstructed is top field
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07261");
    cb.DecodeVideo(context.DecodeReferenceTopField(0).SetFrame(true /* override decode output */));
    m_errorMonitor->VerifyFound();

    // Decode output is frame but reconstructed is bottom field
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07261");
    cb.DecodeVideo(context.DecodeReferenceBottomField(0).SetFrame(true /* override decode output */));
    m_errorMonitor->VerifyFound();

    // Decode output is top field but reconstructed is frame
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07262");
    cb.DecodeVideo(context.DecodeReferenceFrame(0).SetTopField(true /* override decode output */));
    m_errorMonitor->VerifyFound();

    // Decode output is top field but reconstructed is bottom field
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07262");
    cb.DecodeVideo(context.DecodeBottomField(0).SetTopField(true /* override decode output */));
    m_errorMonitor->VerifyFound();

    // Decode output is bottom field but reconstructed is frame
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07263");
    cb.DecodeVideo(context.DecodeReferenceFrame(0).SetBottomField(true /* override decode output */));
    m_errorMonitor->VerifyFound();

    // Decode output is bottom field but reconstructed is top field
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07263");
    cb.DecodeVideo(context.DecodeReferenceTopField(0).SetBottomField(true /* override decode output */));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeInvalidCodecInfoH265) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - invalid/missing H.265 codec-specific information");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecodeH265()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support with reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VideoDecodeInfo decode_info = context.DecodeFrame(0);

    StdVideoDecodeH265PictureInfo std_picture_info{};
    auto picture_info = vku::InitStruct<VkVideoDecodeH265PictureInfoKHR>();
    uint32_t slice_segment_offset = 0;
    picture_info.pStdPictureInfo = &std_picture_info;
    picture_info.sliceSegmentCount = 1;
    picture_info.pSliceSegmentOffsets = &slice_segment_offset;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    // Missing H.265 picture info
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = nullptr;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-07158");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Slice offsets must be within buffer range
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pSliceSegmentOffsets-07159");
        slice_segment_offset = (uint32_t)decode_info->srcBufferRange;
        cb.DecodeVideo(decode_info);
        slice_segment_offset = 0;
        m_errorMonitor->VerifyFound();
    }

    // No matching VPS/SPS/PPS
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-StdVideoH265VideoParameterSet-07160");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-StdVideoH265SequenceParameterSet-07161");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-StdVideoH265PictureParameterSet-07162");
        std_picture_info.sps_video_parameter_set_id = 1;
        cb.DecodeVideo(decode_info);
        std_picture_info.sps_video_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-StdVideoH265SequenceParameterSet-07161");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-StdVideoH265PictureParameterSet-07162");
        std_picture_info.pps_seq_parameter_set_id = 1;
        cb.DecodeVideo(decode_info);
        std_picture_info.pps_seq_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-StdVideoH265PictureParameterSet-07162");
        std_picture_info.pps_pic_parameter_set_id = 1;
        cb.DecodeVideo(decode_info);
        std_picture_info.pps_pic_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();
    }

    // Missing H.265 setup reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;
        decode_info->pSetupReferenceSlot = &slot;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07141");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07163");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing H.265 reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        decode_info = context.DecodeFrame(1);
        decode_info->pNext = &picture_info;
        decode_info->referenceSlotCount = 1;
        decode_info->pReferenceSlots = &slot;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-07164");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeInvalidCodecInfoAV1) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - invalid/missing AV1 codec-specific information");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecodeAV1(), 2), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 decode support with 2 reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VideoDecodeInfo decode_info = context.DecodeFrame(0);

    StdVideoDecodeAV1PictureInfo std_picture_info{};
    auto picture_info = vku::InitStruct<VkVideoDecodeAV1PictureInfoKHR>();
    uint32_t tile_offset = 0;
    uint32_t tile_size = (uint32_t)decode_info->srcBufferRange;
    picture_info.pStdPictureInfo = &std_picture_info;

    for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
        picture_info.referenceNameSlotIndices[i] = -1;
    }

    picture_info.tileCount = 1;
    picture_info.pTileOffsets = &tile_offset;
    picture_info.pTileSizes = &tile_size;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    // Missing AV1 picture info
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = nullptr;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-09250");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Frame header offset must be within buffer range
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-frameHeaderOffset-09251");
        picture_info.frameHeaderOffset = (uint32_t)decode_info->srcBufferRange;
        cb.DecodeVideo(decode_info);
        picture_info.frameHeaderOffset = 0;
        m_errorMonitor->VerifyFound();
    }

    // Tile offsets must be within buffer range
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pTileOffsets-09252");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pTileOffsets-09253");
        tile_offset = (uint32_t)decode_info->srcBufferRange;
        cb.DecodeVideo(decode_info);
        tile_offset = 0;
        m_errorMonitor->VerifyFound();
    }

    // Tile offset plus size must be within buffer range
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pTileOffsets-09252");
        tile_size = (uint32_t)decode_info->srcBufferRange + 1;
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pTileOffsets-09252");
        tile_offset = 1;
        tile_size = (uint32_t)decode_info->srcBufferRange;
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();

        tile_offset = 0;
        tile_size = (uint32_t)decode_info->srcBufferRange;
    }

    // Film grain cannot be used if the video profile did not enable support for it
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-09249");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-filmGrainSupport-09248");
        std_picture_info.flags.apply_grain = 1;
        cb.DecodeVideo(decode_info);
        std_picture_info.flags.apply_grain = 0;
        m_errorMonitor->VerifyFound();
    }

    // Missing AV1 setup reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        decode_info = context.DecodeFrame(1);
        decode_info->pNext = &picture_info;
        decode_info->pSetupReferenceSlot = &slot;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07141");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-09254");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing AV1 reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        decode_info = context.DecodeFrame(1);
        decode_info->pNext = &picture_info;
        decode_info->referenceSlotCount = 1;
        decode_info->pReferenceSlots = &slot;

        picture_info.referenceNameSlotIndices[3] = 0;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-09255");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();

        picture_info.referenceNameSlotIndices[3] = -1;
    }

    // Missing reference in referenceNameSlotIndices to reference slot
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        decode_info = context.DecodeFrame(1);
        decode_info->pNext = &picture_info;
        decode_info->referenceSlotCount = 1;
        decode_info->pReferenceSlots = &slot;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pNext-09255");
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-slotIndex-09263");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing reference slot for DPB slot index referred to by referenceNameSlotIndices
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;

        picture_info.referenceNameSlotIndices[3] = 0;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-referenceNameSlotIndices-09262");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();

        picture_info.referenceNameSlotIndices[3] = -1;
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeAV1FilmGrainRequiresDistinct) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - AV1 film grain requires distinct reconstructed picture");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecodeAV1FilmGrain()));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 decode support with reference pictures and film grain";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07140");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07146");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-09249");
    cb.DecodeVideo(context.DecodeFrame(0).ApplyFilmGrain().SetDecodeOutput(context.Dpb()->Picture(0)));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeInlineQueryOpCount) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - inline query count does not match video codec operation count");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();
    context.CreateStatusQueryPool(2);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-08365");
    cb.DecodeVideo(context.DecodeFrame().InlineQuery(context.StatusQueryPool(), 0, 2));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeInlineQueryOutOfBounds) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - inline query firstQuery/queryCount out of bounds");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();
    context.CreateStatusQueryPool(4);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoInlineQueryInfoKHR-queryPool-08372");
    m_errorMonitor->SetDesiredError("VUID-VkVideoInlineQueryInfoKHR-queryPool-08373");
    cb.DecodeVideo(context.DecodeFrame().InlineQuery(context.StatusQueryPool(), 4, 1));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pNext-08365");
    m_errorMonitor->SetDesiredError("VUID-VkVideoInlineQueryInfoKHR-queryPool-08373");
    cb.DecodeVideo(context.DecodeFrame().InlineQuery(context.StatusQueryPool(), 2, 3));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeInlineQueryUnavailable) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - inline queries must be unavailable");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();
    context.CreateStatusQueryPool();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    // Use custom begin info because the default uses VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();

    cb.Begin(&begin_info);
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset());
    cb.DecodeVideo(context.DecodeFrame().InlineQuery(context.StatusQueryPool()));
    cb.EndVideoCoding(context.End());
    cb.End();

    m_command_buffer.Begin(&begin_info);
    vk::CmdResetQueryPool(m_command_buffer.handle(), context.StatusQueryPool(), 0, 1);
    m_command_buffer.End();

    // Will fail as query pool has never been reset before
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-08366");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    // Will succeed this time as we reset the query
    context.Queue().Submit(cb);
    m_device->Wait();

    // Will fail again as we did not reset after use
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-08366");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    // Will succeed again after reset
    context.Queue().Submit(cb);
    m_device->Wait();
}

TEST_F(NegativeVideo, DecodeInlineQueryType) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - inline query type is invalid");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    auto create_info = vku::InitStruct<VkQueryPoolCreateInfo>();
    create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, create_info);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-queryPool-08368");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-queryType-08367");
    cb.DecodeVideo(context.DecodeFrame().InlineQuery(query_pool.handle()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeInlineQueryProfileMismatch) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - inline query must have been created with the same profile");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    auto configs = GetConfigsDecode();
    if (configs.size() < 2) {
        GTEST_SKIP() << "Test requires support for at least two video decode profiles";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(configs[0].QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    configs[0].SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context1(m_device, configs[0]);
    VideoContext context2(m_device, configs[1]);
    context1.CreateAndBindSessionMemory();
    context1.CreateResources();
    context2.CreateStatusQueryPool();

    vkt::CommandBuffer& cb = context1.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-queryPool-08368");
    cb.DecodeVideo(context1.DecodeFrame().InlineQuery(context2.StatusQueryPool()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.End();
}

TEST_F(NegativeVideo, DecodeInlineQueryIncompatibleQueueFamily) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - result status queries require queue family support");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    uint32_t queue_family_index = VK_QUEUE_FAMILY_IGNORED;
    for (uint32_t qfi = 0; qfi < QueueFamilyCount(); ++qfi) {
        if (!QueueFamilySupportsResultStatusOnlyQueries(qfi)) {
            queue_family_index = qfi;
            break;
        }
    }

    if (queue_family_index == VK_QUEUE_FAMILY_IGNORED) {
        GTEST_SKIP() << "Test requires a queue family with no support for result status queries";
    }

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();
    context.CreateStatusQueryPool();

    vkt::CommandPool cmd_pool(*m_device, queue_family_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer cb(*m_device, cmd_pool);

    cb.Begin();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-cmdpool");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07231");
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-commandBuffer-cmdpool");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-queryType-08369");
    cb.DecodeVideo(context.DecodeFrame().InlineQuery(context.StatusQueryPool()));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEndVideoCodingKHR-commandBuffer-cmdpool");
    cb.EndVideoCoding(context.End());

    cb.End();
}

TEST_F(NegativeVideo, EncodeInlineQueryOpCount) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - inline query count does not match video codec operation count");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();
    context.CreateEncodeFeedbackQueryPool(2);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08360");
    cb.EncodeVideo(context.EncodeFrame().InlineQuery(context.EncodeFeedbackQueryPool(), 0, 2));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeInlineQueryOutOfBounds) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - inline query firstQuery/queryCount out of bounds");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();
    context.CreateEncodeFeedbackQueryPool(4);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoInlineQueryInfoKHR-queryPool-08372");
    m_errorMonitor->SetDesiredError("VUID-VkVideoInlineQueryInfoKHR-queryPool-08373");
    cb.EncodeVideo(context.EncodeFrame().InlineQuery(context.EncodeFeedbackQueryPool(), 4, 1));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pNext-08360");
    m_errorMonitor->SetDesiredError("VUID-VkVideoInlineQueryInfoKHR-queryPool-08373");
    cb.EncodeVideo(context.EncodeFrame().InlineQuery(context.EncodeFeedbackQueryPool(), 2, 3));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeInlineQueryUnavailable) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - inline queries must be unavailable");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();
    context.CreateEncodeFeedbackQueryPool();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    // Use custom begin info because the default uses VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();

    cb.Begin(&begin_info);
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset());
    cb.EncodeVideo(context.EncodeFrame().InlineQuery(context.EncodeFeedbackQueryPool()));
    cb.EndVideoCoding(context.End());
    cb.End();

    m_command_buffer.Begin(&begin_info);
    vk::CmdResetQueryPool(m_command_buffer.handle(), context.EncodeFeedbackQueryPool(), 0, 1);
    m_command_buffer.End();

    // Will fail as query pool has never been reset before
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08361");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();

    m_default_queue->Submit(m_command_buffer);
    m_device->Wait();

    // Will succeed this time as we reset the query
    context.Queue().Submit(cb);
    m_device->Wait();

    // Will fail again as we did not reset after use
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08361");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    // Will succeed again after reset
    context.Queue().Submit(cb);
    m_device->Wait();
}

TEST_F(NegativeVideo, EncodeInlineQueryType) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - inline query type is invalid");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    auto create_info = vku::InitStruct<VkQueryPoolCreateInfo>();
    create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, create_info);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-queryPool-08363");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-queryType-08362");
    cb.EncodeVideo(context.EncodeFrame().InlineQuery(query_pool.handle()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeInlineQueryProfileMismatch) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - inline query must have been created with the same profile");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    auto configs = GetConfigsEncode();
    if (configs.size() < 2) {
        GTEST_SKIP() << "Test requires support for at least two video encode profiles";
    }

    configs[0].SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context1(m_device, configs[0]);
    VideoContext context2(m_device, configs[1]);
    context1.CreateAndBindSessionMemory();
    context1.CreateResources();
    context2.CreateEncodeFeedbackQueryPool();

    vkt::CommandBuffer& cb = context1.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-queryPool-08363");
    cb.EncodeVideo(context1.EncodeFrame().InlineQuery(context2.EncodeFeedbackQueryPool()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeInlineQueryIncompatibleQueueFamily) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - result status queries require queue family support");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    uint32_t queue_family_index = VK_QUEUE_FAMILY_IGNORED;
    for (uint32_t qfi = 0; qfi < QueueFamilyCount(); ++qfi) {
        if (!QueueFamilySupportsResultStatusOnlyQueries(qfi)) {
            queue_family_index = qfi;
            break;
        }
    }

    if (queue_family_index == VK_QUEUE_FAMILY_IGNORED) {
        GTEST_SKIP() << "Test requires a queue family with no support for result status queries";
    }

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();
    context.CreateStatusQueryPool();

    vkt::CommandPool cmd_pool(*m_device, queue_family_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer cb(*m_device, cmd_pool);

    cb.Begin();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-cmdpool");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07231");
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-commandBuffer-cmdpool");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-queryType-08364");
    cb.EncodeVideo(context.EncodeFrame().InlineQuery(context.StatusQueryPool()));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEndVideoCodingKHR-commandBuffer-cmdpool");
    cb.EndVideoCoding(context.End());

    cb.End();
}

TEST_F(NegativeVideo, EncodeCapsH264GenPrefixNalu) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - generating H.264 prefix NALU is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeH264(), [](const VideoConfig& config) {
        return (config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_GENERATE_PREFIX_NALU_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support without prefix NALU generation support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();
    encode_info.CodecInfo().encode_h264.picture_info.generatePrefixNalu = VK_TRUE;

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264PictureInfoKHR-flags-08304");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeCapsH264MaxSliceCount) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - test H.264 maxSliceCount capability");

    RETURN_IF_SKIP(Init());

    // Find the H.264 config with the highest slice count supported
    // so that we don't just check a config with only a single slice supported if possible
    auto configs = GetConfigsEncodeH264();
    VideoConfig config;
    uint32_t max_slice_count = 0;
    for (auto& cfg : configs) {
        if (cfg.EncodeCapsH264()->maxSliceCount > max_slice_count) {
            config = cfg;
            max_slice_count = cfg.EncodeCapsH264()->maxSliceCount;
        }
    }
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();

    const uint32_t slice_count = max_slice_count + 1;
    std::vector<VkVideoEncodeH264NaluSliceInfoKHR> slices(slice_count, encode_info.CodecInfo().encode_h264.slice_info);
    encode_info.CodecInfo().encode_h264.picture_info.naluSliceEntryCount = slice_count;
    encode_info.CodecInfo().encode_h264.picture_info.pNaluSliceEntries = slices.data();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceEntryCount-08302");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceEntryCount-08312");
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264PictureInfoKHR-naluSliceEntryCount-08301");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeCapsH264MoreSlicesThanMBs) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - more H.264 slices are requested than MBs "
        "when VK_VIDEO_ENCODE_H264_CAPABILITY_ROW_UNALIGNED_SLICE_BIT_KHR is supported");

    RETURN_IF_SKIP(Init());

    // Find the H.264 config with the highest slice count supported
    // so that we don't just check a config with only a single slice supported if possible
    auto configs = GetConfigsEncodeH264();
    VideoConfig config;
    uint32_t max_slice_count = 0;
    for (auto& cfg : configs) {
        if ((cfg.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_ROW_UNALIGNED_SLICE_BIT_KHR) != 0 &&
            cfg.EncodeCapsH264()->maxSliceCount > max_slice_count) {
            config = cfg;
            max_slice_count = cfg.EncodeCapsH264()->maxSliceCount;
        }
    }
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode with row unaligned slice supported";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();

    const uint32_t slice_count = config.MaxEncodeH264MBCount() + 1;
    std::vector<VkVideoEncodeH264NaluSliceInfoKHR> slices(slice_count, encode_info.CodecInfo().encode_h264.slice_info);
    encode_info.CodecInfo().encode_h264.picture_info.naluSliceEntryCount = slice_count;
    encode_info.CodecInfo().encode_h264.picture_info.pNaluSliceEntries = slices.data();

    if (slice_count > config.EncodeCapsH264()->maxSliceCount) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH264PictureInfoKHR-naluSliceEntryCount-08301");
    }
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-naluSliceEntryCount-08302");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeCapsH264MoreSlicesThanMBRows) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - more H.264 slices are requested than MB rows "
        "when VK_VIDEO_ENCODE_H264_CAPABILITY_ROW_UNALIGNED_SLICE_BIT_KHR is not supported");

    RETURN_IF_SKIP(Init());

    // Find the H.264 config with the highest slice count supported
    // so that we don't just check a config with only a single slice supported if possible
    auto configs = GetConfigsEncodeH264();
    VideoConfig config;
    uint32_t max_slice_count = 0;
    for (auto& cfg : configs) {
        if ((cfg.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_ROW_UNALIGNED_SLICE_BIT_KHR) == 0 &&
            cfg.EncodeCapsH264()->maxSliceCount > max_slice_count) {
            config = cfg;
            max_slice_count = cfg.EncodeCapsH264()->maxSliceCount;
        }
    }
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode without row unaligned slice supported";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();

    const uint32_t slice_count = config.MaxEncodeH264MBRowCount() + 1;
    std::vector<VkVideoEncodeH264NaluSliceInfoKHR> slices(slice_count, encode_info.CodecInfo().encode_h264.slice_info);
    encode_info.CodecInfo().encode_h264.picture_info.naluSliceEntryCount = slice_count;
    encode_info.CodecInfo().encode_h264.picture_info.pNaluSliceEntries = slices.data();

    if (slice_count > config.EncodeCapsH264()->maxSliceCount) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH264PictureInfoKHR-naluSliceEntryCount-08301");
    }
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-naluSliceEntryCount-08312");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeCapsH264DifferentSliceTypes) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - using different H.264 slice types is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeH264(), [](const VideoConfig& config) {
        return (config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_DIFFERENT_SLICE_TYPE_BIT_KHR) == 0 &&
               config.EncodeCapsH264()->maxSliceCount > 1;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with multiple slice support but no different slice types";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();

    const uint32_t slice_count = 2;
    std::vector<VkVideoEncodeH264NaluSliceInfoKHR> slices(slice_count, encode_info.CodecInfo().encode_h264.slice_info);
    std::vector<StdVideoEncodeH264SliceHeader> slice_headers(slice_count, encode_info.CodecInfo().encode_h264.std_slice_header);
    encode_info.CodecInfo().encode_h264.picture_info.naluSliceEntryCount = slice_count;
    encode_info.CodecInfo().encode_h264.picture_info.pNaluSliceEntries = slices.data();

    slices[0].pStdSliceHeader = &slice_headers[0];
    slices[1].pStdSliceHeader = &slice_headers[1];

    slice_headers[0].slice_type = STD_VIDEO_H264_SLICE_TYPE_I;
    slice_headers[1].slice_type = STD_VIDEO_H264_SLICE_TYPE_P;

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264PictureInfoKHR-flags-08315");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeCapsH265DifferentSliceSegmentTypes) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - using different H.265 slice segment types is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [](const VideoConfig& config) {
        return (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_DIFFERENT_SLICE_SEGMENT_TYPE_BIT_KHR) == 0 &&
               config.EncodeCapsH265()->maxSliceSegmentCount > 1;
    }));
    if (!config) {
        GTEST_SKIP()
            << "Test requires H.265 encode support with multiple slice segment support but no different slice segment types";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();

    const uint32_t slice_segment_count = 2;
    std::vector<VkVideoEncodeH265NaluSliceSegmentInfoKHR> slice_segments(slice_segment_count,
                                                                         encode_info.CodecInfo().encode_h265.slice_segment_info);
    std::vector<StdVideoEncodeH265SliceSegmentHeader> slice_segment_headers(
        slice_segment_count, encode_info.CodecInfo().encode_h265.std_slice_segment_header);
    encode_info.CodecInfo().encode_h265.picture_info.naluSliceSegmentEntryCount = slice_segment_count;
    encode_info.CodecInfo().encode_h265.picture_info.pNaluSliceSegmentEntries = slice_segments.data();

    slice_segments[0].pStdSliceSegmentHeader = &slice_segment_headers[0];
    slice_segments[1].pStdSliceSegmentHeader = &slice_segment_headers[1];

    slice_segment_headers[0].slice_type = STD_VIDEO_H265_SLICE_TYPE_I;
    slice_segment_headers[1].slice_type = STD_VIDEO_H265_SLICE_TYPE_P;

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08324");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08307");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08313");
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08317");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeCapsH265MultipleTilesPerSliceSegment) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - encoding multiple H.265 tiles per slice segment is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [](const VideoConfig& config) {
        return (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_MULTIPLE_TILES_PER_SLICE_SEGMENT_BIT_KHR) == 0 &&
               (config.EncodeCapsH265()->maxTiles.width > 1 || config.EncodeCapsH265()->maxTiles.height > 1);
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with multiple tile but no multiple tiles per slice segment support";
    }

    auto pps = config.EncodeH265PPS();
    pps->num_tile_columns_minus1 = (uint8_t)config.EncodeCapsH265()->maxTiles.width - 1;
    pps->num_tile_rows_minus1 = (uint8_t)config.EncodeCapsH265()->maxTiles.height - 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08323");
    cb.EncodeVideo(context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeCapsH265MultipleSliceSegmentsPerTile) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - encoding multiple H.265 slcie segments per tile is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [](const VideoConfig& config) {
        return (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_MULTIPLE_SLICE_SEGMENTS_PER_TILE_BIT_KHR) == 0 &&
               config.EncodeCapsH265()->maxSliceSegmentCount > 1;
    }));
    if (!config) {
        GTEST_SKIP()
            << "Test requires H.265 encode support with multiple slice segment but no multiple slice segments per tile support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();

    const uint32_t slice_segment_count = 2;
    std::vector<VkVideoEncodeH265NaluSliceSegmentInfoKHR> slice_segments(slice_segment_count,
                                                                         encode_info.CodecInfo().encode_h265.slice_segment_info);
    encode_info.CodecInfo().encode_h265.picture_info.naluSliceSegmentEntryCount = slice_segment_count;
    encode_info.CodecInfo().encode_h265.picture_info.pNaluSliceSegmentEntries = slice_segments.data();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08307");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08313");
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08324");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeCapsH265MaxSliceSegmentCount) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - test H.265 maxSliceSegmentCount capability");

    RETURN_IF_SKIP(Init());

    // Find the H.265 config with the highest slice segment count supported
    // so that we don't just check a config with only a single slice segment supported if possible
    auto configs = GetConfigsEncodeH265();
    VideoConfig config;
    uint32_t max_slice_segment_count = 0;
    for (auto& cfg : configs) {
        if (cfg.EncodeCapsH265()->maxSliceSegmentCount > max_slice_segment_count) {
            config = cfg;
            max_slice_segment_count = cfg.EncodeCapsH265()->maxSliceSegmentCount;
        }
    }
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();

    const uint32_t slice_segment_count = max_slice_segment_count + 1;
    std::vector<VkVideoEncodeH265NaluSliceSegmentInfoKHR> slice_segments(slice_segment_count,
                                                                         encode_info.CodecInfo().encode_h265.slice_segment_info);
    encode_info.CodecInfo().encode_h265.picture_info.naluSliceSegmentEntryCount = slice_segment_count;
    encode_info.CodecInfo().encode_h265.picture_info.pNaluSliceSegmentEntries = slice_segments.data();

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08324");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08307");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08313");
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265PictureInfoKHR-naluSliceSegmentEntryCount-08306");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeCapsH265MoreSliceSegmentsThanCTBs) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - more H.265 slice segments are requested than CTBs "
        "when VK_VIDEO_ENCODE_H265_CAPABILITY_ROW_UNALIGNED_SLICE_SEGMENT_BIT_KHR is supported");

    RETURN_IF_SKIP(Init());

    // Find the H.265 config with the highest slice segment count supported
    // so that we don't just check a config with only a single slice segment supported if possible
    auto configs = GetConfigsEncodeH265();
    VideoConfig config;
    uint32_t max_slice_segment_count = 0;
    for (auto& cfg : configs) {
        if ((cfg.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_ROW_UNALIGNED_SLICE_SEGMENT_BIT_KHR) != 0 &&
            cfg.EncodeCapsH265()->maxSliceSegmentCount > max_slice_segment_count) {
            config = cfg;
            max_slice_segment_count = cfg.EncodeCapsH265()->maxSliceSegmentCount;
        }
    }
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode with row unaligned slice segment supported";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();

    const uint32_t slice_segment_count = config.MaxEncodeH265CTBCount() + 1;
    std::vector<VkVideoEncodeH265NaluSliceSegmentInfoKHR> slice_segments(slice_segment_count,
                                                                         encode_info.CodecInfo().encode_h265.slice_segment_info);
    encode_info.CodecInfo().encode_h265.picture_info.naluSliceSegmentEntryCount = slice_segment_count;
    encode_info.CodecInfo().encode_h265.picture_info.pNaluSliceSegmentEntries = slice_segments.data();

    if (slice_segment_count > config.EncodeCapsH265()->maxSliceSegmentCount) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265PictureInfoKHR-naluSliceSegmentEntryCount-08306");
    }
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08324");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08307");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeCapsH265MoreSliceSegmentsThanCTBRows) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - more H.265 slice segments are requested than CTB rows "
        "when VK_VIDEO_ENCODE_H265_CAPABILITY_ROW_UNALIGNED_SLICE_SEGMENT_BIT_KHR is not supported");

    RETURN_IF_SKIP(Init());

    // Find the H.265 config with the highest slice segment count supported
    // so that we don't just check a config with only a single slice segment supported if possible
    auto configs = GetConfigsEncodeH265();
    VideoConfig config;
    uint32_t max_slice_segment_count = 0;
    for (auto& cfg : configs) {
        if ((cfg.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_ROW_UNALIGNED_SLICE_SEGMENT_BIT_KHR) == 0 &&
            cfg.EncodeCapsH265()->maxSliceSegmentCount > max_slice_segment_count) {
            config = cfg;
            max_slice_segment_count = cfg.EncodeCapsH265()->maxSliceSegmentCount;
        }
    }
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode without row unaligned slice segment supported";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();

    const uint32_t slice_segment_count = config.MaxEncodeH265CTBRowCount() + 1;
    std::vector<VkVideoEncodeH265NaluSliceSegmentInfoKHR> slice_segments(slice_segment_count,
                                                                         encode_info.CodecInfo().encode_h265.slice_segment_info);
    encode_info.CodecInfo().encode_h265.picture_info.naluSliceSegmentEntryCount = slice_segment_count;
    encode_info.CodecInfo().encode_h265.picture_info.pNaluSliceSegmentEntries = slice_segments.data();

    if (slice_segment_count > config.EncodeCapsH265()->maxSliceSegmentCount) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265PictureInfoKHR-naluSliceSegmentEntryCount-08306");
    }
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08324");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08313");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeCapsH264WeightTable) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - H.264 weight table is required for explicit sample prediction if "
        "VK_VIDEO_ENCODE_H264_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config_p = GetConfig(FilterConfigs(GetConfigsEncodeH264(), [](const VideoConfig& config) {
        return (config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR) == 0 &&
               config.EncodeCapsH264()->maxPPictureL0ReferenceCount > 0;
    }));
    VideoConfig config_b = GetConfig(FilterConfigs(GetConfigsEncodeH264(), [](const VideoConfig& config) {
        return (config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR) == 0 &&
               config.EncodeCapsH264()->maxBPictureL0ReferenceCount > 0 && config.EncodeCapsH264()->maxL1ReferenceCount > 0;
    }));
    if (!config_p && !config_b) {
        GTEST_SKIP() << "Test requires an H.264 encode profile without generated weight table support";
    }

    if (config_p) {
        VideoConfig config = config_p;

        // Enable explicit weighted sample prediction for P pictures
        config.EncodeH264PPS()->flags.weighted_pred_flag = 1;

        config.SessionCreateInfo()->maxDpbSlots = 2;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddReferenceFrame(0);

        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264PictureInfoKHR-flags-08314");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (config_b) {
        VideoConfig config = config_b;

        // Enable explicit weighted sample prediction for B pictures
        config.EncodeH264PPS()->weighted_bipred_idc = STD_VIDEO_H264_WEIGHTED_BIPRED_IDC_EXPLICIT;

        config.SessionCreateInfo()->maxDpbSlots = 3;
        config.SessionCreateInfo()->maxActiveReferencePictures = 2;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(-1, 2));

        VideoEncodeInfo encode_info = context.EncodeFrame(2).AddReferenceFrame(0).AddBackReferenceFrame(1);

        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264PictureInfoKHR-flags-08314");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }
}

TEST_F(NegativeVideo, EncodeCapsH265WeightTable) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - H.265 weight table is required for explicit sample prediction if "
        "VK_VIDEO_ENCODE_H265_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config_p = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [](const VideoConfig& config) {
        return (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR) == 0 &&
               config.EncodeCapsH265()->maxPPictureL0ReferenceCount > 0;
    }));
    VideoConfig config_b = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [](const VideoConfig& config) {
        return (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR) == 0 &&
               config.EncodeCapsH265()->maxBPictureL0ReferenceCount > 0 && config.EncodeCapsH265()->maxL1ReferenceCount > 0;
    }));
    if (!config_p && !config_b) {
        GTEST_SKIP() << "Test requires an H.265 encode profile without generated weight table support";
    }

    if (config_p) {
        VideoConfig config = config_p;

        // Enable explicit weighted sample prediction for P pictures
        config.EncodeH265PPS()->flags.weighted_pred_flag = 1;

        config.SessionCreateInfo()->maxDpbSlots = 2;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddReferenceFrame(0);

        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08316");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (config_b) {
        VideoConfig config = config_b;

        // Enable explicit weighted sample prediction for B pictures
        config.EncodeH265PPS()->flags.weighted_bipred_flag = 1;

        config.SessionCreateInfo()->maxDpbSlots = 3;
        config.SessionCreateInfo()->maxActiveReferencePictures = 2;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(-1, 2));

        VideoEncodeInfo encode_info = context.EncodeFrame(2).AddReferenceFrame(0).AddBackReferenceFrame(1);
        encode_info.CodecInfo().encode_h265.std_picture_info.pic_type = STD_VIDEO_H265_PICTURE_TYPE_B;
        encode_info.CodecInfo().encode_h265.std_slice_segment_header.slice_type = STD_VIDEO_H265_SLICE_TYPE_B;

        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08316");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }
}

TEST_F(NegativeVideo, EncodeCapsH264PicType) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Cannot encode H.264 P or B pictures without capability prerequisites");

    RETURN_IF_SKIP(Init());

    VideoConfig config_no_p = GetConfig(FilterConfigs(GetConfigsEncodeH264(), [](const VideoConfig& config) {
        return config.EncodeCapsH264()->maxPPictureL0ReferenceCount == 0;
    }));
    VideoConfig config_no_b = GetConfig(FilterConfigs(GetConfigsEncodeH264(), [](const VideoConfig& config) {
        return config.EncodeCapsH264()->maxBPictureL0ReferenceCount == 0 && config.EncodeCapsH264()->maxL1ReferenceCount == 0;
    }));
    if (!config_no_p && !config_no_b) {
        GTEST_SKIP() << "Test requires an H.264 encode profile without P or B frame support";
    }

    if (config_no_p) {
        VideoConfig config = config_no_p;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin());

        VideoEncodeInfo encode_info = context.EncodeFrame();
        encode_info.CodecInfo().encode_h264.std_picture_info.primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_P;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxPPictureL0ReferenceCount-08340");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (config_no_b) {
        VideoConfig config = config_no_b;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin());

        VideoEncodeInfo encode_info = context.EncodeFrame();
        encode_info.CodecInfo().encode_h264.std_picture_info.primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_B;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxBPictureL0ReferenceCount-08341");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }
}

TEST_F(NegativeVideo, EncodeCapsH264RefPicType) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Cannot reference H.264 P or B pictures without capability prerequisites");

    RETURN_IF_SKIP(Init());

    VideoConfig config_no_p = GetConfig(FilterConfigs(GetConfigsEncodeH264(), [](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > 1 && config.Caps()->maxActiveReferencePictures > 0 &&
               config.EncodeCapsH264()->maxPPictureL0ReferenceCount == 0;
    }));
    VideoConfig config_no_b = GetConfig(FilterConfigs(GetConfigsEncodeH264(), [](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > 1 && config.Caps()->maxActiveReferencePictures > 0 &&
               config.EncodeCapsH264()->maxBPictureL0ReferenceCount == 0 && config.EncodeCapsH264()->maxL1ReferenceCount == 0;
    }));
    if (!config_no_p && !config_no_b) {
        GTEST_SKIP() << "Test requires an H.264 encode profile without P or B frame support";
    }

    if (config_no_p) {
        VideoConfig config = config_no_p;

        config.SessionCreateInfo()->maxDpbSlots = 2;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddReferenceFrame(0);
        encode_info.CodecInfo().encode_h264.std_picture_info.primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_P;
        encode_info.CodecInfo().encode_h264.std_reference_info[0].primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_P;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxPPictureL0ReferenceCount-08340");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxPPictureL0ReferenceCount-08340");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (config_no_b) {
        VideoConfig config = config_no_b;

        config.SessionCreateInfo()->maxDpbSlots = 2;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddReferenceFrame(0);
        encode_info.CodecInfo().encode_h264.std_picture_info.primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_B;
        encode_info.CodecInfo().encode_h264.std_reference_info[0].primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_B;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-flags-08342");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxBPictureL0ReferenceCount-08341");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxBPictureL0ReferenceCount-08341");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }
}

TEST_F(NegativeVideo, EncodeCapsH264BPicInRefList) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Cannot reference H.264 B pictures in L0/L1 without capability prerequisites");

    RETURN_IF_SKIP(Init());

    VideoConfig config_no_b_in_l0 = GetConfig(FilterConfigs(GetConfigsEncodeH264(), [](const VideoConfig& config) {
        return (config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_B_FRAME_IN_L0_LIST_BIT_KHR) == 0 &&
               config.EncodeCapsH264()->maxBPictureL0ReferenceCount > 0;
    }));
    VideoConfig config_no_b_in_l1 = GetConfig(FilterConfigs(GetConfigsEncodeH264(), [](const VideoConfig& config) {
        return (config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_B_FRAME_IN_L1_LIST_BIT_KHR) == 0 &&
               config.EncodeCapsH264()->maxL1ReferenceCount > 0;
    }));
    if (!config_no_b_in_l0 && !config_no_b_in_l1) {
        GTEST_SKIP() << "Test requires an H.264 encode profile without B frame support in L0 or L1";
    }

    if (config_no_b_in_l0) {
        VideoConfig config = config_no_b_in_l0;

        config.SessionCreateInfo()->maxDpbSlots = 2;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddReferenceFrame(0);
        encode_info.CodecInfo().encode_h264.std_reference_info[0].primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_B;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-flags-08342");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (config_no_b_in_l1) {
        VideoConfig config = config_no_b_in_l1;

        config.SessionCreateInfo()->maxDpbSlots = 2;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddBackReferenceFrame(0);
        encode_info.CodecInfo().encode_h264.std_reference_info[0].primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_B;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-flags-08343");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }
}

TEST_F(NegativeVideo, EncodeCapsH265PicType) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Cannot encode H.265 P or B pictures without capability prerequisites");

    RETURN_IF_SKIP(Init());

    VideoConfig config_no_p = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [](const VideoConfig& config) {
        return config.EncodeCapsH265()->maxPPictureL0ReferenceCount == 0;
    }));
    VideoConfig config_no_b = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [](const VideoConfig& config) {
        return config.EncodeCapsH265()->maxBPictureL0ReferenceCount == 0 && config.EncodeCapsH265()->maxL1ReferenceCount == 0;
    }));
    if (!config_no_p && !config_no_b) {
        GTEST_SKIP() << "Test requires an H.265 encode profile without P or B frame support";
    }

    if (config_no_p) {
        VideoConfig config = config_no_p;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin());

        VideoEncodeInfo encode_info = context.EncodeFrame();
        encode_info.CodecInfo().encode_h265.std_picture_info.pic_type = STD_VIDEO_H265_PICTURE_TYPE_P;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxPPictureL0ReferenceCount-08345");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (config_no_b) {
        VideoConfig config = config_no_b;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin());

        VideoEncodeInfo encode_info = context.EncodeFrame();
        encode_info.CodecInfo().encode_h265.std_picture_info.pic_type = STD_VIDEO_H265_PICTURE_TYPE_B;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxBPictureL0ReferenceCount-08346");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }
}

TEST_F(NegativeVideo, EncodeCapsH265RefPicType) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Cannot reference H.265 P or B pictures without capability prerequisites");

    RETURN_IF_SKIP(Init());

    VideoConfig config_no_p = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > 1 && config.Caps()->maxActiveReferencePictures > 0 &&
               config.EncodeCapsH265()->maxPPictureL0ReferenceCount == 0;
    }));
    VideoConfig config_no_b = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > 1 && config.Caps()->maxActiveReferencePictures > 0 &&
               config.EncodeCapsH265()->maxBPictureL0ReferenceCount == 0 && config.EncodeCapsH265()->maxL1ReferenceCount == 0;
    }));
    if (!config_no_p && !config_no_b) {
        GTEST_SKIP() << "Test requires an H.265 encode profile without P or B frame support";
    }

    if (config_no_p) {
        VideoConfig config = config_no_p;

        config.SessionCreateInfo()->maxDpbSlots = 2;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddReferenceFrame(0);
        encode_info.CodecInfo().encode_h265.std_picture_info.pic_type = STD_VIDEO_H265_PICTURE_TYPE_P;
        encode_info.CodecInfo().encode_h265.std_reference_info[0].pic_type = STD_VIDEO_H265_PICTURE_TYPE_P;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxPPictureL0ReferenceCount-08345");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxPPictureL0ReferenceCount-08345");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (config_no_b) {
        VideoConfig config = config_no_b;

        config.SessionCreateInfo()->maxDpbSlots = 2;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddReferenceFrame(0);
        encode_info.CodecInfo().encode_h265.std_picture_info.pic_type = STD_VIDEO_H265_PICTURE_TYPE_B;
        encode_info.CodecInfo().encode_h265.std_reference_info[0].pic_type = STD_VIDEO_H265_PICTURE_TYPE_B;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-flags-08347");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxBPictureL0ReferenceCount-08346");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxBPictureL0ReferenceCount-08346");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }
}

TEST_F(NegativeVideo, EncodeCapsH265BPicInRefList) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Cannot reference H.265 B pictures in L0/L1 without capability prerequisites");

    RETURN_IF_SKIP(Init());

    VideoConfig config_no_b_in_l0 = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [](const VideoConfig& config) {
        return (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_B_FRAME_IN_L0_LIST_BIT_KHR) == 0 &&
               config.EncodeCapsH265()->maxBPictureL0ReferenceCount > 0;
    }));
    VideoConfig config_no_b_in_l1 = GetConfig(FilterConfigs(GetConfigsEncodeH265(), [](const VideoConfig& config) {
        return (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_B_FRAME_IN_L1_LIST_BIT_KHR) == 0 &&
               config.EncodeCapsH265()->maxL1ReferenceCount > 0;
    }));
    if (!config_no_b_in_l0 && !config_no_b_in_l1) {
        GTEST_SKIP() << "Test requires an H.265 encode profile without B frame support in L0 or L1";
    }

    if (config_no_b_in_l0) {
        VideoConfig config = config_no_b_in_l0;

        config.SessionCreateInfo()->maxDpbSlots = 2;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddReferenceFrame(0);
        encode_info.CodecInfo().encode_h265.std_reference_info[0].pic_type = STD_VIDEO_H265_PICTURE_TYPE_B;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-flags-08347");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }

    if (config_no_b_in_l1) {
        VideoConfig config = config_no_b_in_l1;

        config.SessionCreateInfo()->maxDpbSlots = 2;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddBackReferenceFrame(0);
        encode_info.CodecInfo().encode_h265.std_reference_info[0].pic_type = STD_VIDEO_H265_PICTURE_TYPE_B;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-flags-08348");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    }
}

TEST_F(NegativeVideo, EncodeCapsAV1PrimaryRefCdfOnly) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - using AV1 primary reference for CDF-only is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(
        GetConfigsWithReferences(FilterConfigs(GetConfigsEncodeAV1(),
                                               [](const VideoConfig& config) {
                                                   return (config.EncodeCapsAV1()->flags &
                                                           VK_VIDEO_ENCODE_AV1_CAPABILITY_PRIMARY_REFERENCE_CDF_ONLY_BIT_KHR) == 0;
                                               })),
        2));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support without CDF-only primary reference support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1));

    VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
    encode_info.CodecInfo().encode_av1.picture_info.primaryReferenceCdfOnly = VK_TRUE;

    // This may trigger other, prediction mode specific cap VUs, because we do not respect the need
    // to use one or two of the supported reference names corresponding to the used prediction mode
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-predictionMode-10329");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-predictionMode-10331");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-predictionMode-10333");

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeAV1PictureInfoKHR-flags-10289");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeCapsAV1GenObuExtHeader) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - generating AV1 OBU extension header is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [](const VideoConfig& config) {
        return (config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_GENERATE_OBU_EXTENSION_HEADER_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support without OBU extension header generation support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    StdVideoEncodeAV1ExtensionHeader av1_obu_ext_header{};
    VideoEncodeInfo encode_info = context.EncodeFrame();
    encode_info.CodecInfo().encode_av1.picture_info.generateObuExtensionHeader = VK_TRUE;
    encode_info.CodecInfo().encode_av1.std_picture_info.pExtensionHeader = &av1_obu_ext_header;

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeAV1PictureInfoKHR-flags-10292");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeCapsAV1FrameSizeOverride) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 frame size override is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [](const VideoConfig& config) {
        return ((config.Caps()->minCodedExtent.width < config.Caps()->maxCodedExtent.width) ||
                (config.Caps()->minCodedExtent.height < config.Caps()->maxCodedExtent.height)) &&
               (config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_FRAME_SIZE_OVERRIDE_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with no frame size override support";
    }

    config.UpdateMaxCodedExtent(config.Caps()->maxCodedExtent);

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    // Cannot set frame_size_override_flag
    auto encode_info = context.EncodeFrame();
    encode_info.CodecInfo().encode_av1.std_picture_info.flags.frame_size_override_flag = 1;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-flags-10322");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    encode_info.CodecInfo().encode_av1.std_picture_info.flags.frame_size_override_flag = 0;

    // Cannot use smaller coded width
    encode_info->srcPictureResource.codedExtent.width -= 1;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-flags-10323");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    encode_info->srcPictureResource.codedExtent.width += 1;

    // Cannot use smaller coded height
    encode_info->srcPictureResource.codedExtent.height -= 1;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-flags-10324");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    encode_info->srcPictureResource.codedExtent.height += 1;

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeCapsAV1MotionVectorScaling) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 motion vector scaling is not supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(
        GetConfigsWithReferences(FilterConfigs(
            GetConfigsEncodeAV1(),
            [](const VideoConfig& config) {
                return ((config.Caps()->minCodedExtent.width < config.Caps()->maxCodedExtent.width) ||
                        (config.Caps()->minCodedExtent.height < config.Caps()->maxCodedExtent.height)) &&
                       (config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_MOTION_VECTOR_SCALING_BIT_KHR) == 0;
            })),
        2));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with references but no motion vector scaling support";
    }

    config.UpdateMaxCodedExtent(config.Caps()->maxCodedExtent);

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    // We will use a setup where the encoded picture has an extent of maxCodedExtent
    // but the reference frame has an extent of minCodedExtent
    auto patched_resource = context.Dpb()->Picture(1);
    patched_resource.codedExtent = config.Caps()->minCodedExtent;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, patched_resource));

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-flags-10325");
    cb.EncodeVideo(context.EncodeFrame(0).AddReferenceFrame(1, &patched_resource));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeCapsAV1MaxTiles) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - TileCols and TileRows must be between (0,0) and maxTiles");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    StdVideoAV1TileInfo tile_info{};
    VideoEncodeInfo encode_info = context.EncodeFrame();
    encode_info.CodecInfo().encode_av1.std_picture_info.pTileInfo = &tile_info;

    // TileCols > 0 is not satisfied
    {
        tile_info.TileCols = 0;
        tile_info.TileRows = 1;

        // We may violate tile size limits here but that's okay
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10347");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10348");

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pTileInfo-10343");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // TileRows > 0 is not satisfied
    {
        tile_info.TileCols = 1;
        tile_info.TileRows = 0;

        // We may violate tile size limits here but that's okay
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10347");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10348");

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pTileInfo-10344");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // TileCols <= maxTiles.width is not satisfied
    {
        tile_info.TileCols = static_cast<uint8_t>(config.EncodeCapsAV1()->maxTiles.width + 1);
        tile_info.TileRows = 1;

        // We may violate tile size limits here but that's okay
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10347");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10348");

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pTileInfo-10345");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // TileRows <= maxTiles.height is not satisfied
    {
        tile_info.TileCols = 1;
        tile_info.TileRows = static_cast<uint8_t>(config.EncodeCapsAV1()->maxTiles.height + 1);

        // We may violate tile size limits here but that's okay
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10347");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10348");

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pTileInfo-10346");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeCapsAV1MinMaxTileSize) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - ceil(codedExtent / (TileCols,TileRows)) must be between minTileSize and maxTileSize");

    RETURN_IF_SKIP(Init());

    auto configs = GetConfigsEncodeAV1();
    if (configs.size() == 0) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    StdVideoAV1TileInfo tile_info{};

    auto test_encode = [&](const VideoConfig& config, const char* expected_tile_size_vuid) {
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin());

        VideoEncodeInfo encode_info = context.EncodeFrame();
        encode_info.CodecInfo().encode_av1.std_picture_info.pTileInfo = &tile_info;

        m_errorMonitor->SetDesiredError(expected_tile_size_vuid);
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();
    };

    // Test a case that triggers ceil(codedExtent.width / TileCols) < minTileSize.width
    {
        // Find the config with the largest supported minimum tile width
        uint32_t max_min_tile_width = 0;
        VideoConfig config;
        for (auto& cfg : configs) {
            if (cfg.EncodeCapsAV1()->minTileSize.width > max_min_tile_width) {
                max_min_tile_width = cfg.EncodeCapsAV1()->minTileSize.width;
                config = cfg;
            }
        }

        // Use the smallest possible coded extent
        const VkExtent2D coded_extent = config.Caps()->minCodedExtent;
        config.UpdateMaxCodedExtent(coded_extent);

        // Use more tile columns than what would be the maximum to have a tile width >= minTileSize.width
        tile_info.TileCols = static_cast<uint8_t>(coded_extent.width / config.EncodeCapsAV1()->minTileSize.width + 1);
        tile_info.TileRows = static_cast<uint8_t>(coded_extent.height / config.EncodeCapsAV1()->minTileSize.height);

        // We may violate some tile count VUIDs here but that is fine
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10345");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10346");

        test_encode(config, "VUID-vkCmdEncodeVideoKHR-pTileInfo-10347");
    }

    // Test a case that triggers ceil(codedExtent.height / TileRows) < minTileSize.height
    {
        // Find the config with the largest supported minimum tile height
        uint32_t max_min_tile_height = 0;
        VideoConfig config;
        for (auto& cfg : configs) {
            if (cfg.EncodeCapsAV1()->minTileSize.height > max_min_tile_height) {
                max_min_tile_height = cfg.EncodeCapsAV1()->minTileSize.height;
                config = cfg;
            }
        }

        // Use the smallest possible coded extent
        const VkExtent2D coded_extent = config.Caps()->minCodedExtent;
        config.UpdateMaxCodedExtent(coded_extent);

        // Use more tile rows than what would be the maximum to have a tile height >= minTileSize.height
        tile_info.TileCols = static_cast<uint8_t>(coded_extent.width / config.EncodeCapsAV1()->minTileSize.width);
        tile_info.TileRows = static_cast<uint8_t>(coded_extent.height / config.EncodeCapsAV1()->minTileSize.height + 1);

        // We may violate some tile count VUIDs here but that is fine
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10345");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pTileInfo-10346");

        test_encode(config, "VUID-vkCmdEncodeVideoKHR-pTileInfo-10348");
    }

    // Test a case that triggers ceil(codedExtent.width / TileCols) > maxTileSize.width
    {
        // Find the config with the smallest supported maximum tile width
        uint32_t min_max_tile_width = UINT32_MAX;
        VideoConfig config;
        for (auto& cfg : configs) {
            if (cfg.EncodeCapsAV1()->maxTileSize.width < min_max_tile_width) {
                min_max_tile_width = cfg.EncodeCapsAV1()->maxTileSize.width;
                config = cfg;
            }
        }

        // Use the largest possible coded extent
        const VkExtent2D coded_extent = config.Caps()->maxCodedExtent;
        config.UpdateMaxCodedExtent(coded_extent);

        // Use less tile columns than what would be the minimum to have a tile width <= maxTileSize.width
        tile_info.TileCols = static_cast<uint8_t>(
            (coded_extent.width + config.EncodeCapsAV1()->maxTileSize.width - 1) / config.EncodeCapsAV1()->maxTileSize.width - 1);
        tile_info.TileRows = static_cast<uint8_t>((coded_extent.height + config.EncodeCapsAV1()->maxTileSize.height - 1) /
                                                  config.EncodeCapsAV1()->maxTileSize.height);

        // We can only test this if the determined TileCols is not zero
        if (tile_info.TileCols > 0) {
            test_encode(config, "VUID-vkCmdEncodeVideoKHR-pTileInfo-10347");
        }
    }

    // Test a case that triggers ceil(codedExtent.height / TileRows) > maxTileSize.height
    {
        // Find the config with the smallest supported maximum tile height
        uint32_t min_max_tile_height = UINT32_MAX;
        VideoConfig config;
        for (auto& cfg : configs) {
            if (cfg.EncodeCapsAV1()->maxTileSize.height < min_max_tile_height) {
                min_max_tile_height = cfg.EncodeCapsAV1()->maxTileSize.height;
                config = cfg;
            }
        }

        // Use the largest possible coded extent
        const VkExtent2D coded_extent = config.Caps()->maxCodedExtent;
        config.UpdateMaxCodedExtent(coded_extent);

        // Use less tile rows than what would be the minimum to have a tile height <= maxTileSize.height
        tile_info.TileCols = static_cast<uint8_t>((coded_extent.width + config.EncodeCapsAV1()->maxTileSize.width - 1) /
                                                  config.EncodeCapsAV1()->maxTileSize.width);
        tile_info.TileRows = static_cast<uint8_t>((coded_extent.height + config.EncodeCapsAV1()->maxTileSize.height - 1) /
                                                      config.EncodeCapsAV1()->maxTileSize.height -
                                                  1);

        // We can only test this if the determined TileRows is not zero
        if (tile_info.TileRows > 0) {
            test_encode(config, "VUID-vkCmdEncodeVideoKHR-pTileInfo-10348");
        }
    }
}

TEST_F(NegativeVideo, EncodeInvalidCodecInfoH264) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - invalid/missing H.264 codec-specific information");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncodeH264()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VideoEncodeInfo encode_info = context.EncodeFrame(0);

    StdVideoEncodeH264PictureInfo std_picture_info{};
    StdVideoEncodeH264ReferenceListsInfo std_ref_lists{};
    StdVideoEncodeH264SliceHeader std_slice_header{};
    auto slice_info = vku::InitStruct<VkVideoEncodeH264NaluSliceInfoKHR>();
    auto picture_info = vku::InitStruct<VkVideoEncodeH264PictureInfoKHR>();
    std_picture_info.pRefLists = &std_ref_lists;
    picture_info.pStdPictureInfo = &std_picture_info;
    picture_info.naluSliceEntryCount = 1;
    picture_info.pNaluSliceEntries = &slice_info;
    slice_info.pStdSliceHeader = &std_slice_header;

    for (uint32_t i = 0; i < STD_VIDEO_H264_MAX_NUM_LIST_REF; ++i) {
        std_ref_lists.RefPicList0[i] = STD_VIDEO_H264_NO_REFERENCE_PICTURE;
        std_ref_lists.RefPicList1[i] = STD_VIDEO_H264_NO_REFERENCE_PICTURE;
    }

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    // Missing H.264 picture info
    {
        encode_info = context.EncodeFrame(0);
        encode_info->pNext = nullptr;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08225");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // No matching SPS/PPS
    {
        encode_info = context.EncodeFrame(0);
        encode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-StdVideoH264SequenceParameterSet-08226");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-StdVideoH264PictureParameterSet-08227");
        std_picture_info.seq_parameter_set_id = 1;
        cb.EncodeVideo(encode_info);
        std_picture_info.seq_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-StdVideoH264PictureParameterSet-08227");
        std_picture_info.pic_parameter_set_id = 1;
        cb.EncodeVideo(encode_info);
        std_picture_info.pic_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();
    }

    // Missing H.264 setup reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        encode_info = context.EncodeFrame(0);
        encode_info->pNext = &picture_info;
        encode_info->pSetupReferenceSlot = &slot;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08228");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing H.264 reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        encode_info = context.EncodeFrame(1);
        encode_info->pNext = &picture_info;
        encode_info->referenceSlotCount = 1;
        encode_info->pReferenceSlots = &slot;

        std_ref_lists.RefPicList0[0] = 0;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08229");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        std_ref_lists.RefPicList0[0] = STD_VIDEO_H264_NO_REFERENCE_PICTURE;
    }

    // Missing H.264 reference list info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        encode_info = context.EncodeFrame(1);
        encode_info->pNext = &picture_info;
        encode_info->referenceSlotCount = 1;
        encode_info->pReferenceSlots = &slot;

        std_picture_info.pRefLists = nullptr;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pNext-08229");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08352");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        std_picture_info.pRefLists = &std_ref_lists;
    }

    // Missing H.264 L0 or L1 list reference to reference slot
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        encode_info = context.EncodeFrame(1);
        encode_info->pNext = &picture_info;
        encode_info->referenceSlotCount = 1;
        encode_info->pReferenceSlots = &slot;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pNext-08229");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08353");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing reference slot for DPB index referred to by the H.264 L0 or L1 list
    {
        encode_info = context.EncodeFrame(0);
        encode_info->pNext = &picture_info;

        std_ref_lists.RefPicList0[0] = 0;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08339");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        std_ref_lists.RefPicList0[0] = STD_VIDEO_H264_NO_REFERENCE_PICTURE;

        std_ref_lists.RefPicList1[0] = 0;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08339");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        std_ref_lists.RefPicList1[0] = STD_VIDEO_H264_NO_REFERENCE_PICTURE;
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeInvalidCodecInfoH265) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - invalid/missing H.265 codec-specific information");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncodeH265()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VideoEncodeInfo encode_info = context.EncodeFrame(0);

    StdVideoEncodeH265PictureInfo std_picture_info{};
    StdVideoEncodeH265ReferenceListsInfo std_ref_lists{};
    StdVideoEncodeH265SliceSegmentHeader std_slice_segment_header{};
    auto slice_segment_info = vku::InitStruct<VkVideoEncodeH265NaluSliceSegmentInfoKHR>();
    auto picture_info = vku::InitStruct<VkVideoEncodeH265PictureInfoKHR>();
    std_picture_info.pRefLists = &std_ref_lists;
    picture_info.pStdPictureInfo = &std_picture_info;
    picture_info.naluSliceSegmentEntryCount = 1;
    picture_info.pNaluSliceSegmentEntries = &slice_segment_info;
    slice_segment_info.pStdSliceSegmentHeader = &std_slice_segment_header;

    for (uint32_t i = 0; i < STD_VIDEO_H265_MAX_NUM_LIST_REF; ++i) {
        std_ref_lists.RefPicList0[i] = STD_VIDEO_H265_NO_REFERENCE_PICTURE;
        std_ref_lists.RefPicList1[i] = STD_VIDEO_H265_NO_REFERENCE_PICTURE;
    }

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    // Missing H.265 picture info
    {
        encode_info = context.EncodeFrame(0);
        encode_info->pNext = nullptr;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08230");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // No matching VPS/SPS/PPS
    {
        encode_info = context.EncodeFrame(0);
        encode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-StdVideoH265VideoParameterSet-08231");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-StdVideoH265SequenceParameterSet-08232");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-StdVideoH265PictureParameterSet-08233");
        std_picture_info.sps_video_parameter_set_id = 1;
        cb.EncodeVideo(encode_info);
        std_picture_info.sps_video_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-StdVideoH265SequenceParameterSet-08232");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-StdVideoH265PictureParameterSet-08233");
        std_picture_info.pps_seq_parameter_set_id = 1;
        cb.EncodeVideo(encode_info);
        std_picture_info.pps_seq_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-StdVideoH265PictureParameterSet-08233");
        std_picture_info.pps_pic_parameter_set_id = 1;
        cb.EncodeVideo(encode_info);
        std_picture_info.pps_pic_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();
    }

    // Missing H.265 setup reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        encode_info = context.EncodeFrame(0);
        encode_info->pNext = &picture_info;
        encode_info->pSetupReferenceSlot = &slot;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08234");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing H.265 reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        encode_info = context.EncodeFrame(1);
        encode_info->pNext = &picture_info;
        encode_info->referenceSlotCount = 1;
        encode_info->pReferenceSlots = &slot;

        std_ref_lists.RefPicList0[0] = 0;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08235");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        std_ref_lists.RefPicList0[0] = STD_VIDEO_H265_NO_REFERENCE_PICTURE;
    }

    // Missing H.265 reference list info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        encode_info = context.EncodeFrame(1);
        encode_info->pNext = &picture_info;
        encode_info->referenceSlotCount = 1;
        encode_info->pReferenceSlots = &slot;

        std_picture_info.pRefLists = nullptr;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pNext-08235");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08354");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        std_picture_info.pRefLists = &std_ref_lists;
    }

    // Missing H.265 L0 or L1 list reference to reference slot
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        encode_info = context.EncodeFrame(1);
        encode_info->pNext = &picture_info;
        encode_info->referenceSlotCount = 1;
        encode_info->pReferenceSlots = &slot;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pNext-08235");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08355");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing reference slot for DPB index referred to by the H.265 L0 or L1 list
    {
        encode_info = context.EncodeFrame(0);
        encode_info->pNext = &picture_info;

        std_ref_lists.RefPicList0[0] = 0;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08344");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        std_ref_lists.RefPicList0[0] = STD_VIDEO_H264_NO_REFERENCE_PICTURE;

        std_ref_lists.RefPicList1[0] = 0;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08344");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        std_ref_lists.RefPicList1[0] = STD_VIDEO_H264_NO_REFERENCE_PICTURE;
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeInvalidCodecInfoAV1) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - invalid/missing AV1 codec-specific information");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncodeAV1()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    // Missing AV1 picture info
    {
        auto encode_info = context.EncodeFrame(0);
        encode_info->pNext = nullptr;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-10317");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // INTRA_ONLY prediction mode used with AV1 frame type other than KEY_FRAME or INTRA_ONLY_FRAME
    {
        auto encode_info = context.EncodeFrame(1).AddReferenceFrame(0);

        encode_info.CodecInfo().encode_av1.picture_info.predictionMode = VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_INTRA_ONLY_KHR;
        encode_info.CodecInfo().encode_av1.std_picture_info.frame_type = STD_VIDEO_AV1_FRAME_TYPE_INTER;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10326");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Non-INTRA_ONLY prediction mode used with AV1 frame type KEY_FRAME or INTRA_ONLY_FRAME
    {
        auto encode_info = context.EncodeFrame(1).AddReferenceFrame(0);

        encode_info.CodecInfo().encode_av1.std_picture_info.frame_type = STD_VIDEO_AV1_FRAME_TYPE_INTRA_ONLY;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pStdPictureInfo-10327");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        encode_info.CodecInfo().encode_av1.std_picture_info.frame_type = STD_VIDEO_AV1_FRAME_TYPE_KEY;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pStdPictureInfo-10327");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing AV1 setup reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        auto encode_info = context.EncodeFrame(0);
        encode_info->pSetupReferenceSlot = &slot;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10318");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing AV1 reference info
    {
        auto slot = vku::InitStruct<VkVideoReferenceSlotInfoKHR>();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        auto encode_info = context.EncodeFrame(1).AddReferenceFrame(0);
        encode_info->referenceSlotCount = 1;
        encode_info->pReferenceSlots = &slot;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-10319");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing reference in referenceNameSlotIndices to reference slot
    {
        auto encode_info = context.EncodeFrame(1).AddReferenceFrame(0);

        for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
            encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
        }

        // This will also trigger the VU related to primary_ref_frame DPB slot index
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeAV1PictureInfoKHR-pStdPictureInfo-10291");

        // This may trigger other, prediction mode specific cap VUs, because we do not respect the need
        // to use one or two of the supported reference names corresponding to the used prediction mode
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-predictionMode-10329");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-predictionMode-10331");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-predictionMode-10333");

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-slotIndex-10335");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing reference slot for DPB slot index referred to by referenceNameSlotIndices
    {
        auto encode_info = context.EncodeFrame(1).AddReferenceFrame(0);
        encode_info->referenceSlotCount = 0;
        encode_info->pReferenceSlots = nullptr;

        for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
            m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-referenceNameSlotIndices-10334");
        }

        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // AV1 segmentation is not supported but segmentation_enabled is not zero
    {
        auto encode_info = context.EncodeFrame(0);
        encode_info.CodecInfo().encode_av1.std_picture_info.flags.segmentation_enabled = 1;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pStdPictureInfo-10349");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // AV1 segmentation is not supported but pSegmentation is not NULL
    {
        StdVideoAV1Segmentation segmentation_info{};
        auto encode_info = context.EncodeFrame(0);
        encode_info.CodecInfo().encode_av1.std_picture_info.pSegmentation = &segmentation_info;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pStdPictureInfo-10350");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeInvalidCodecInfoAV1PrimaryRefCdfOnly) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - invalid/missing AV1 codec-specific information "
        "when primary reference is used only as CDF reference");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(
        GetConfigsWithReferences(FilterConfigs(GetConfigsEncodeAV1(),
                                               [](const VideoConfig& config) {
                                                   return (config.EncodeCapsAV1()->flags &
                                                           VK_VIDEO_ENCODE_AV1_CAPABILITY_PRIMARY_REFERENCE_CDF_ONLY_BIT_KHR) != 0;
                                               })),
        2));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with reference pictures, 2 DPB slots, "
                        "and CDF-only primary reference support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1));

    // Invalid primary_ref_frame
    {
        VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
        encode_info.CodecInfo().encode_av1.picture_info.primaryReferenceCdfOnly = VK_TRUE;
        encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame = VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR;

        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeAV1PictureInfoKHR-primaryReferenceCdfOnly-10290");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Invalid primary reference DPB slot index
    {
        VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
        encode_info.CodecInfo().encode_av1.picture_info.primaryReferenceCdfOnly = VK_TRUE;
        const uint8_t primary_ref_frame = encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame;
        encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[primary_ref_frame] = -1;

        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeAV1PictureInfoKHR-pStdPictureInfo-10291");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeAV1InvalidPrimaryRefFrameDpbSlotIndex) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - AV1 primary_ref_frame specifies a valid reference name with no associated DPB slot index");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncodeAV1()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1));

    VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
    const uint8_t primary_ref_frame = encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame;
    encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[primary_ref_frame] = -1;

    // This may trigger other, prediction mode specific cap VUs, because we do not respect the need
    // to use one or two of the supported reference names corresponding to the used prediction mode
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-predictionMode-10329");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-predictionMode-10331");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-predictionMode-10333");

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeAV1PictureInfoKHR-pStdPictureInfo-10291");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeInvalidCodecInfoAV1GenObuExtHeader) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - invalid/missing AV1 codec-specific information "
        "when OBU extension header generation is requested");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [](const VideoConfig& config) {
        return (config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_GENERATE_OBU_EXTENSION_HEADER_BIT_KHR) != 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with OBU extension header generation support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();
    encode_info.CodecInfo().encode_av1.picture_info.generateObuExtensionHeader = VK_TRUE;
    encode_info.CodecInfo().encode_av1.std_picture_info.pExtensionHeader = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeAV1PictureInfoKHR-generateObuExtensionHeader-10293");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeAV1SingleReferenceNotSupported) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 single reference prediction mode not supported");

    RETURN_IF_SKIP(Init());

    // Single reference prediction requires at least one active reference picture
    const uint32_t min_ref_count = 1;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [&](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > min_ref_count && config.Caps()->maxActiveReferencePictures >= min_ref_count &&
               config.EncodeCapsAV1()->maxSingleReferenceCount == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with no single reference prediction mode support";
    }

    config.SessionCreateInfo()->maxDpbSlots = min_ref_count + 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = min_ref_count;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(1, 1));

    VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
    encode_info.CodecInfo().encode_av1.picture_info.predictionMode = VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_SINGLE_REFERENCE_KHR;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxSingleReferenceCount-10328");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeAV1InvalidSingleReference) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Unsupported AV1 single reference name");

    RETURN_IF_SKIP(Init());

    // Single reference prediction requires at least one active reference picture
    const uint32_t min_ref_count = 1;

    // We need to find an unsupported AV1 reference name
    const uint32_t ref_name_mask = 0x7F;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [&](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > min_ref_count && config.Caps()->maxActiveReferencePictures >= min_ref_count &&
               config.EncodeCapsAV1()->maxSingleReferenceCount > 0 &&
               (config.EncodeCapsAV1()->singleReferenceNameMask & ref_name_mask) != ref_name_mask;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with single reference prediction mode support";
    }

    config.SessionCreateInfo()->maxDpbSlots = min_ref_count + 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = min_ref_count;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(1, 1));

    VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
    encode_info.CodecInfo().encode_av1.picture_info.predictionMode = VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_SINGLE_REFERENCE_KHR;

    // Clear all supported reference name indices, leaving only the unsupported ones on
    for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
        if ((config.EncodeCapsAV1()->singleReferenceNameMask & (1 << i)) != 0) {
            encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
        }
    }

    // This may trigger the VU related to primary_ref_frame DPB slot index
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeAV1PictureInfoKHR-pStdPictureInfo-10291");

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10329");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeAV1InvalidSingleReferenceCdfOnly) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Unsupported CDF-only AV1 single reference name");

    RETURN_IF_SKIP(Init());

    // Single reference prediction requires at least one active reference picture
    // The CDF-only reference can also refer to the same reference slot, only the reference name has to differ
    const uint32_t min_ref_count = 1;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [&](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > min_ref_count && config.Caps()->maxActiveReferencePictures >= min_ref_count &&
               config.EncodeCapsAV1()->maxSingleReferenceCount > 0 &&
               (config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_PRIMARY_REFERENCE_CDF_ONLY_BIT_KHR) != 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with single reference prediction mode "
                        "and CDF-only primary reference support";
    }

    config.SessionCreateInfo()->maxDpbSlots = min_ref_count + 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = min_ref_count;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(1, 1));

    VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
    encode_info.CodecInfo().encode_av1.picture_info.predictionMode = VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_SINGLE_REFERENCE_KHR;

    // Clear all supported reference name indices, leaving only the unsupported ones on
    uint8_t supported_ref_idx = 0;
    for (uint8_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
        if ((config.EncodeCapsAV1()->singleReferenceNameMask & (1 << i)) != 0) {
            encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
        } else {
            // Remember one of the supported reference names as we will use that as CDF-only reference
            supported_ref_idx = i;
        }
    }

    // Use the supported reference name as CDF-only reference
    encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[supported_ref_idx] = 1;
    encode_info.CodecInfo().encode_av1.picture_info.primaryReferenceCdfOnly = VK_TRUE;
    encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame = supported_ref_idx;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10329");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeAV1UnidirectionalCompoundNotSupported) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 unidirectional compound prediction mode not supported");

    RETURN_IF_SKIP(Init());

    // Unidirectional compound prediction requires at least one active reference picture
    // No need for two pictures as both reference names can point to the same picture
    const uint32_t min_ref_count = 1;

    // We need to find an unsupported AV1 reference name
    // NOTE: Unidirectional compound prediction does not support ALTREF2_FRAME in any combination
    const uint32_t ref_name_mask = 0x5F;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [&](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > min_ref_count && config.Caps()->maxActiveReferencePictures >= min_ref_count &&
               config.EncodeCapsAV1()->maxUnidirectionalCompoundReferenceCount == 0 &&
               (config.EncodeCapsAV1()->unidirectionalCompoundReferenceNameMask & ref_name_mask) != ref_name_mask;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with no unidirectional compound prediction mode support";
    }

    config.SessionCreateInfo()->maxDpbSlots = min_ref_count + 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = min_ref_count;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(1, 1));

    VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
    encode_info.CodecInfo().encode_av1.picture_info.predictionMode =
        VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_UNIDIRECTIONAL_COMPOUND_KHR;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxUnidirectionalCompoundReferenceCount-10330");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeAV1InvalidUnidirectionalCompound) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Unsupported AV1 unidirectional compound reference names");

    RETURN_IF_SKIP(Init());

    // Unidirectional compound prediction requires at least one active reference picture
    // No need for two pictures as both reference names can point to the same picture
    const uint32_t min_ref_count = 1;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [&](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > min_ref_count && config.Caps()->maxActiveReferencePictures >= min_ref_count &&
               config.EncodeCapsAV1()->maxUnidirectionalCompoundReferenceCount > 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with unidirectional compound prediction mode support";
    }

    config.SessionCreateInfo()->maxDpbSlots = min_ref_count + 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = min_ref_count;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(1, 1));

    VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
    encode_info.CodecInfo().encode_av1.picture_info.predictionMode =
        VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_UNIDIRECTIONAL_COMPOUND_KHR;

    // Unidirectional compound supports the following combinations
    //   (0,1) - LAST_FRAME + LAST2_FRAME
    //   (0,2) - LAST_FRAME + LAST3_FRAME
    //   (0,3) - LAST_FRAME + GOLDEN_FRAME
    //   (4,6) - BWDREF_FRAME + ALTREF_FRAME
    // Test with cases that fail these conditions in some way
    std::vector<uint32_t> clear_masks = {
        0x11,  // No LAST_FRAME or BWDREF_FRAME
        0x41,  // No LAST_FRAME or ALTREF_FRAME
        0x1E,  // No LAST2_FRAME, LAST3_FRAME, GOLDEN_FRAME, or BWDREF_FRAME
        0x4E,  // No LAST2_FRAME, LAST3_FRAME, GOLDEN_FRAME, or ALTREF_FRAME
    };
    for (auto clear_mask : clear_masks) {
        for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
            if ((clear_mask & (1 << i)) != 0) {
                encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
            }
        }

        // This may trigger the VU related to primary_ref_frame DPB slot index
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeAV1PictureInfoKHR-pStdPictureInfo-10291");

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10331");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeAV1InvalidUnidirectionalCompoundCdfOnly) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Unsupported CDF-only AV1 unidirectional compound reference names");

    RETURN_IF_SKIP(Init());

    // Unidirectional compound prediction requires at least one active reference picture
    // No need for two pictures as both reference names can point to the same picture
    // The CDF-only reference can also refer to the same reference slot, only the reference name has to differ
    const uint32_t min_ref_count = 1;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [&](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > min_ref_count && config.Caps()->maxActiveReferencePictures >= min_ref_count &&
               config.EncodeCapsAV1()->maxUnidirectionalCompoundReferenceCount > 0 &&
               (config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_PRIMARY_REFERENCE_CDF_ONLY_BIT_KHR) != 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with unidirectional compound prediction mode "
                        "and CDF-only primary reference support";
    }

    config.SessionCreateInfo()->maxDpbSlots = min_ref_count + 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = min_ref_count;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(1, 1));

    // Unidirectional compound supports the following combinations
    //   (0,1) - LAST_FRAME + LAST2_FRAME
    //   (0,2) - LAST_FRAME + LAST3_FRAME
    //   (0,3) - LAST_FRAME + GOLDEN_FRAME
    //   (4,6) - BWDREF_FRAME + ALTREF_FRAME

    const uint8_t last_frame_idx = STD_VIDEO_AV1_REFERENCE_NAME_LAST_FRAME - 1;
    const std::array<uint8_t, 3> last_frame_pair_indices = {STD_VIDEO_AV1_REFERENCE_NAME_LAST2_FRAME - 1,
                                                            STD_VIDEO_AV1_REFERENCE_NAME_LAST3_FRAME - 1,
                                                            STD_VIDEO_AV1_REFERENCE_NAME_GOLDEN_FRAME - 1};
    const uint8_t bwdref_frame_idx = STD_VIDEO_AV1_REFERENCE_NAME_BWDREF_FRAME - 1;
    const uint8_t altref_frame_idx = STD_VIDEO_AV1_REFERENCE_NAME_ALTREF_FRAME - 1;

    if ((config.EncodeCapsAV1()->unidirectionalCompoundReferenceNameMask & (1 << last_frame_idx)) != 0) {
        // If LAST_FRAME is supported, we clear all group 2 reference names and set it as CDF-only reference,
        VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
        encode_info.CodecInfo().encode_av1.picture_info.predictionMode =
            VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_UNIDIRECTIONAL_COMPOUND_KHR;

        for (uint8_t i = bwdref_frame_idx; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
            encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
        }
        encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[last_frame_idx] = 1;
        encode_info.CodecInfo().encode_av1.picture_info.primaryReferenceCdfOnly = VK_TRUE;
        encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame = last_frame_idx;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10331");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        // then we clear all except LAST_FRAME and enable one of its pairs (if supported) as CDF-only reference
        for (uint8_t i = last_frame_idx + 1; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
            encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
        }

        for (auto other_frame_idx : last_frame_pair_indices) {
            if ((config.EncodeCapsAV1()->unidirectionalCompoundReferenceNameMask & (1 << other_frame_idx)) != 0) {
                encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[other_frame_idx] = 1;
                encode_info.CodecInfo().encode_av1.picture_info.primaryReferenceCdfOnly = VK_TRUE;
                encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame = other_frame_idx;

                m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10331");
                cb.EncodeVideo(encode_info);
                m_errorMonitor->VerifyFound();

                encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[other_frame_idx] = -1;
            }
        }
    }

    if ((config.EncodeCapsAV1()->unidirectionalCompoundReferenceNameMask & (1 << bwdref_frame_idx)) != 0) {
        // If BWDREF_FRAME is supported, we clear all group 1 reference names and set it as CDF-only reference
        VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
        encode_info.CodecInfo().encode_av1.picture_info.predictionMode =
            VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_UNIDIRECTIONAL_COMPOUND_KHR;

        for (uint8_t i = last_frame_idx; i < bwdref_frame_idx; ++i) {
            encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
        }
        encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[bwdref_frame_idx] = 1;
        encode_info.CodecInfo().encode_av1.picture_info.primaryReferenceCdfOnly = VK_TRUE;
        encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame = bwdref_frame_idx;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10331");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    if ((config.EncodeCapsAV1()->unidirectionalCompoundReferenceNameMask & (1 << altref_frame_idx)) != 0) {
        // If ALTREF_FRAME is supported, we clear all group 1 reference names and set it as CDF-only reference
        VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
        encode_info.CodecInfo().encode_av1.picture_info.predictionMode =
            VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_UNIDIRECTIONAL_COMPOUND_KHR;

        for (uint8_t i = last_frame_idx; i < bwdref_frame_idx; ++i) {
            encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
        }
        encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[altref_frame_idx] = 1;
        encode_info.CodecInfo().encode_av1.picture_info.primaryReferenceCdfOnly = VK_TRUE;
        encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame = altref_frame_idx;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10331");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeAV1BidirectionalCompoundNotSupported) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 bidirectional compound prediction mode not supported");

    RETURN_IF_SKIP(Init());

    // Bidirectional compound prediction requires at least one active reference picture
    // No need for two pictures as both reference names can point to the same picture
    const uint32_t min_ref_count = 1;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [&](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > min_ref_count && config.Caps()->maxActiveReferencePictures >= min_ref_count &&
               config.EncodeCapsAV1()->maxBidirectionalCompoundReferenceCount == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with no bidirectional compound prediction mode support";
    }

    config.SessionCreateInfo()->maxDpbSlots = min_ref_count + 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = min_ref_count;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(1, 1));

    VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
    encode_info.CodecInfo().encode_av1.picture_info.predictionMode = VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_BIDIRECTIONAL_COMPOUND_KHR;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxBidirectionalCompoundReferenceCount-10332");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeAV1InvalidBidirectionalCompound) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Unsupported AV1 bidirectional compound reference names");

    RETURN_IF_SKIP(Init());

    // Bidirectional compound prediction requires at least one active reference picture
    // No need for two pictures as both reference names can point to the same picture
    const uint32_t min_ref_count = 1;

    // We need to find an unsupported AV1 reference name
    const uint32_t ref_name_mask = 0x7F;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [&](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > min_ref_count && config.Caps()->maxActiveReferencePictures >= min_ref_count &&
               config.EncodeCapsAV1()->maxBidirectionalCompoundReferenceCount > 0 &&
               (config.EncodeCapsAV1()->bidirectionalCompoundReferenceNameMask & ref_name_mask) != ref_name_mask;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with bidirectional compound prediction mode support";
    }

    config.SessionCreateInfo()->maxDpbSlots = min_ref_count + 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = min_ref_count;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(1, 1));

    const uint32_t bwdref_frame_idx = STD_VIDEO_AV1_REFERENCE_NAME_BWDREF_FRAME - 1;

    // Clear all supported reference name indices from group 1
    {
        VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
        encode_info.CodecInfo().encode_av1.picture_info.predictionMode =
            VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_BIDIRECTIONAL_COMPOUND_KHR;

        for (uint32_t i = 0; i < bwdref_frame_idx; ++i) {
            if ((config.EncodeCapsAV1()->bidirectionalCompoundReferenceNameMask & (1 << i)) != 0) {
                encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
            }
        }

        // This may trigger the VU related to primary_ref_frame DPB slot index
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeAV1PictureInfoKHR-pStdPictureInfo-10291");

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10333");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Clear all supported reference name indices from group 2
    {
        VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
        encode_info.CodecInfo().encode_av1.picture_info.predictionMode =
            VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_BIDIRECTIONAL_COMPOUND_KHR;

        for (uint32_t i = bwdref_frame_idx; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
            if ((config.EncodeCapsAV1()->bidirectionalCompoundReferenceNameMask & (1 << i)) != 0) {
                encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
            }
        }

        // This may trigger the VU related to primary_ref_frame DPB slot index
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeAV1PictureInfoKHR-pStdPictureInfo-10291");

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10333");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeAV1InvalidBidirectionalCompoundCdfOnly) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - Unsupported CDF-only AV1 bidirectional compound reference names");

    RETURN_IF_SKIP(Init());

    // Bidirectional compound prediction requires at least one active reference picture
    // No need for two pictures as both reference names can point to the same picture
    // The CDF-only reference can also refer to the same reference slot, only the reference name has to differ
    const uint32_t min_ref_count = 1;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [&](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > min_ref_count && config.Caps()->maxActiveReferencePictures >= min_ref_count &&
               config.EncodeCapsAV1()->maxBidirectionalCompoundReferenceCount > 0 &&
               (config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_PRIMARY_REFERENCE_CDF_ONLY_BIT_KHR) != 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with bidirectional compound prediction mode "
                        "and CDF-only primary reference support";
    }

    config.SessionCreateInfo()->maxDpbSlots = min_ref_count + 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = min_ref_count;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(1, 1));

    const uint8_t bwdref_frame_idx = STD_VIDEO_AV1_REFERENCE_NAME_BWDREF_FRAME - 1;

    // Clear all supported reference name indices from group 1
    {
        VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
        encode_info.CodecInfo().encode_av1.picture_info.predictionMode =
            VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_BIDIRECTIONAL_COMPOUND_KHR;

        uint8_t supported_group1_ref_idx = 0;
        for (uint8_t i = 0; i < bwdref_frame_idx; ++i) {
            if ((config.EncodeCapsAV1()->bidirectionalCompoundReferenceNameMask & (1 << i)) != 0) {
                encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
            } else {
                // Remember one of the supported group 1 reference names as we will use that as CDF-only reference
                supported_group1_ref_idx = i;
            }
        }

        // Use the supported group 1 reference name as CDF-only reference
        encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[supported_group1_ref_idx] = 1;
        encode_info.CodecInfo().encode_av1.picture_info.primaryReferenceCdfOnly = VK_TRUE;
        encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame = supported_group1_ref_idx;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10333");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Clear all supported reference name indices from group 2
    {
        VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
        encode_info.CodecInfo().encode_av1.picture_info.predictionMode =
            VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_BIDIRECTIONAL_COMPOUND_KHR;

        uint8_t supported_group2_ref_idx = 0;
        for (uint8_t i = bwdref_frame_idx; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
            if ((config.EncodeCapsAV1()->bidirectionalCompoundReferenceNameMask & (1 << i)) != 0) {
                encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
            } else {
                // Remember one of the supported group 2 reference names as we will use that as CDF-only reference
                supported_group2_ref_idx = i;
            }
        }

        // Use the supported group 2 reference name as CDF-only reference
        encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[supported_group2_ref_idx] = 1;
        encode_info.CodecInfo().encode_av1.picture_info.primaryReferenceCdfOnly = VK_TRUE;
        encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame = supported_group2_ref_idx;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10333");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeAV1ObuExtHeaderPictureSetupMismatch) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - AV1 OBU extension header must be specified for both or neither of picture and setup info");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncodeAV1())));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));

    // Picture info has AV1 OBU extension header info but the setup reference info does not
    {
        StdVideoEncodeAV1ExtensionHeader av1_obu_ext_header{};
        auto encode_info = context.EncodeFrame(0);
        encode_info.CodecInfo().encode_av1.std_picture_info.pExtensionHeader = &av1_obu_ext_header;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10338");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // Setup reference info has AV1 OBU extension header info but the picture info does not
    {
        StdVideoEncodeAV1ExtensionHeader av1_obu_ext_header{};
        auto encode_info = context.EncodeFrame(0);
        encode_info.CodecInfo().encode_av1.std_setup_reference_info.pExtensionHeader = &av1_obu_ext_header;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10338");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeAV1InvalidTemporalId) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 temporal ID exceeds maxTemporalLayerCount");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncodeAV1()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    // Picture temporal_id >= maxTemporalLayerCount
    {
        StdVideoEncodeAV1ExtensionHeader av1_obu_ext_header{};
        auto encode_info = context.EncodeFrame(0);
        encode_info.CodecInfo().encode_av1.std_picture_info.pExtensionHeader = &av1_obu_ext_header;
        encode_info.CodecInfo().encode_av1.std_setup_reference_info.pExtensionHeader = &av1_obu_ext_header;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pExtensionHeader-10336");
        av1_obu_ext_header.temporal_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxTemporalLayerCount + 1);
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pExtensionHeader-10336");
        av1_obu_ext_header.temporal_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxTemporalLayerCount);
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        av1_obu_ext_header.temporal_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxTemporalLayerCount - 1);
        cb.EncodeVideo(encode_info);
    }

    // Reference temporal_id >= maxTemporalLayerCount
    {
        StdVideoEncodeAV1ExtensionHeader av1_obu_ext_header{};
        auto encode_info = context.EncodeFrame(1).AddReferenceFrame(0);
        encode_info.CodecInfo().encode_av1.std_reference_info[0].pExtensionHeader = &av1_obu_ext_header;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pExtensionHeader-10341");
        av1_obu_ext_header.temporal_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxTemporalLayerCount + 1);
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pExtensionHeader-10341");
        av1_obu_ext_header.temporal_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxTemporalLayerCount);
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        av1_obu_ext_header.temporal_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxTemporalLayerCount - 1);
        cb.EncodeVideo(encode_info);
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeAV1PictureSetupTemporalIdMismatch) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - temporal_id mismatch between picture and setup info");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(FilterConfigs(
        GetConfigsEncodeAV1(), [](const VideoConfig& config) { return config.EncodeCapsAV1()->maxTemporalLayerCount > 1; }))));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with DPB slots and multiple temporal layers";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));

    StdVideoEncodeAV1ExtensionHeader picture_av1_obu_ext_header{};
    StdVideoEncodeAV1ExtensionHeader setup_av1_obu_ext_header{};
    auto encode_info = context.EncodeFrame(0);
    encode_info.CodecInfo().encode_av1.std_picture_info.pExtensionHeader = &picture_av1_obu_ext_header;
    encode_info.CodecInfo().encode_av1.std_setup_reference_info.pExtensionHeader = &setup_av1_obu_ext_header;

    picture_av1_obu_ext_header.temporal_id = 0;
    setup_av1_obu_ext_header.temporal_id = 1;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10339");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeAV1InvalidSpatialId) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 spatial ID exceeds maxSpatialLayerCount");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncodeAV1()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    // Picture spatial_id >= maxSpatialLayerCount
    {
        StdVideoEncodeAV1ExtensionHeader av1_obu_ext_header{};
        auto encode_info = context.EncodeFrame(0);
        encode_info.CodecInfo().encode_av1.std_picture_info.pExtensionHeader = &av1_obu_ext_header;
        encode_info.CodecInfo().encode_av1.std_setup_reference_info.pExtensionHeader = &av1_obu_ext_header;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pExtensionHeader-10337");
        av1_obu_ext_header.spatial_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxSpatialLayerCount + 1);
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pExtensionHeader-10337");
        av1_obu_ext_header.spatial_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxSpatialLayerCount);
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        av1_obu_ext_header.spatial_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxSpatialLayerCount - 1);
        cb.EncodeVideo(encode_info);
    }

    // Reference spatial_id >= maxSpatialLayerCount
    {
        StdVideoEncodeAV1ExtensionHeader av1_obu_ext_header{};
        auto encode_info = context.EncodeFrame(1).AddReferenceFrame(0);
        encode_info.CodecInfo().encode_av1.std_reference_info[0].pExtensionHeader = &av1_obu_ext_header;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pExtensionHeader-10342");
        av1_obu_ext_header.spatial_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxSpatialLayerCount + 1);
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pExtensionHeader-10342");
        av1_obu_ext_header.spatial_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxSpatialLayerCount);
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        av1_obu_ext_header.spatial_id = static_cast<uint8_t>(config.EncodeCapsAV1()->maxSpatialLayerCount - 1);
        cb.EncodeVideo(encode_info);
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, EncodeAV1PictureSetupSpatialIdMismatch) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - spatial_id mismatch between picture and setup info");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(FilterConfigs(
        GetConfigsEncodeAV1(), [](const VideoConfig& config) { return config.EncodeCapsAV1()->maxSpatialLayerCount > 1; }))));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with DPB slots and multiple spatial layers";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));

    StdVideoEncodeAV1ExtensionHeader picture_av1_obu_ext_header{};
    StdVideoEncodeAV1ExtensionHeader setup_av1_obu_ext_header{};
    auto encode_info = context.EncodeFrame(0);
    encode_info.CodecInfo().encode_av1.std_picture_info.pExtensionHeader = &picture_av1_obu_ext_header;
    encode_info.CodecInfo().encode_av1.std_setup_reference_info.pExtensionHeader = &setup_av1_obu_ext_header;

    picture_av1_obu_ext_header.spatial_id = 0;
    setup_av1_obu_ext_header.spatial_id = 1;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10340");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, CreateBufferInvalidProfileList) {
    TEST_DESCRIPTION("vkCreateBuffer - invalid/missing profile list");

    AddOptionalExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddOptionalFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig decode_config = GetConfigDecode();
    VideoConfig encode_config = GetConfigEncode();
    if (!decode_config && !encode_config) {
        GTEST_SKIP() << "Test requires video decode or encode support";
    }

    VkBuffer buffer = VK_NULL_HANDLE;
    VkVideoProfileListInfoKHR video_profiles = vku::InitStructHelper();
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 2048;

    if (decode_config) {
        buffer_ci.usage = VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR;

        m_errorMonitor->SetDesiredError("VUID-VkBufferCreateInfo-usage-04813");
        vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
        m_errorMonitor->VerifyFound();

        VkVideoProfileInfoKHR profiles[] = {*decode_config.Profile(), *decode_config.Profile()};
        video_profiles.profileCount = 2;
        video_profiles.pProfiles = profiles;
        buffer_ci.pNext = &video_profiles;

        m_errorMonitor->SetDesiredError("VUID-VkVideoProfileListInfoKHR-pProfiles-06813");
        vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
        m_errorMonitor->VerifyFound();

        video_profiles.profileCount = 1;
        video_profiles.pProfiles = decode_config.Profile();

        vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
        vk::DestroyBuffer(device(), buffer, nullptr);
    }

    if (decode_config && encode_config) {
        buffer_ci.usage |= VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR;

        m_errorMonitor->SetDesiredError("VUID-VkBufferCreateInfo-usage-04814");
        vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
        m_errorMonitor->VerifyFound();
    }

    if (encode_config) {
        buffer_ci.pNext = nullptr;
        buffer_ci.usage = VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR;

        m_errorMonitor->SetDesiredError("VUID-VkBufferCreateInfo-usage-04814");
        vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeVideo, CreateBufferVideoEncodeAV1NotEnabled) {
    TEST_DESCRIPTION("vkCreateBuffer - AV1 encode is specified but videoEncodeAV1 was not enabled");

    ForceDisableFeature(vkt::Feature::videoEncodeAV1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VkBuffer buffer = VK_NULL_HANDLE;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage = VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR;
    buffer_ci.size = 2048;

    VkVideoProfileListInfoKHR video_profiles = vku::InitStructHelper();
    video_profiles.profileCount = 1;
    video_profiles.pProfiles = config.Profile();
    buffer_ci.pNext = &video_profiles;

    m_errorMonitor->SetDesiredError("VUID-VkBufferCreateInfo-pNext-10249");
    vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateBufferProfileIndependentNotSupported) {
    TEST_DESCRIPTION("vkCreateBuffer - profile independent buffer creation requires videoMaintenance1");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VkBuffer buffer = VK_NULL_HANDLE;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.flags = VK_BUFFER_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR;
    buffer_ci.size = 2048;

    if (GetConfigDecode()) {
        buffer_ci.usage |= VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR;
    }

    if (GetConfigEncode()) {
        buffer_ci.usage |= VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR;
    }

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkBufferCreateInfo-flags-parameter");
    m_errorMonitor->SetDesiredError("VUID-VkBufferCreateInfo-flags-08325");
    vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateBufferProfileIndependent) {
    TEST_DESCRIPTION("vkCreateBuffer - profile independent buffer creation with invalid parameters");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig decode_config = GetConfigDecode();
    VideoConfig encode_config = GetConfigEncode();
    if (!decode_config && !encode_config) {
        GTEST_SKIP() << "Test requires video decode or encode support";
    }

    VkBuffer buffer = VK_NULL_HANDLE;
    VkVideoProfileListInfoKHR video_profiles = vku::InitStructHelper();
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.flags = VK_BUFFER_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR;
    buffer_ci.size = 2048;

    if (decode_config) {
        buffer_ci.usage = VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR;

        // Creating profile independent buffers without a profile list is allowed
        vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
        vk::DestroyBuffer(device(), buffer, nullptr);

        // An invalid profile list, however, should still cause a validation failure
        VkVideoProfileInfoKHR profiles[] = {*decode_config.Profile(), *decode_config.Profile()};
        video_profiles.profileCount = 2;
        video_profiles.pProfiles = profiles;
        buffer_ci.pNext = &video_profiles;

        m_errorMonitor->SetDesiredError("VUID-VkVideoProfileListInfoKHR-pProfiles-06813");
        vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
        m_errorMonitor->VerifyFound();

        // But a valid profile list should not
        video_profiles.profileCount = 1;
        video_profiles.pProfiles = decode_config.Profile();

        vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
        vk::DestroyBuffer(device(), buffer, nullptr);
    }

    if (encode_config) {
        // Creating profile independent buffers without a profile list is allowed
        buffer_ci.pNext = nullptr;
        buffer_ci.usage = VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR;

        vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
        vk::DestroyBuffer(device(), buffer, nullptr);
    }
}

TEST_F(NegativeVideo, CreateImageInvalidProfileList) {
    TEST_DESCRIPTION("vkCreateImage - invalid/missing profile list");

    AddOptionalExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddOptionalFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig decode_config = GetConfigDecode();
    VideoConfig encode_config = GetConfigEncode();
    if (!decode_config && !encode_config) {
        GTEST_SKIP() << "Test requires video decode or encode support";
    }

    for (auto config : {decode_config, encode_config}) {
        if (!config) continue;

        VkImage image = VK_NULL_HANDLE;
        VkImageCreateInfo image_ci = vku::InitStructHelper();
        image_ci.imageType = config.PictureFormatProps()->imageType;
        image_ci.format = config.PictureFormatProps()->format;
        image_ci.extent = {config.MaxCodedExtent().width, config.MaxCodedExtent().height, 1};
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.tiling = config.PictureFormatProps()->imageTiling;
        image_ci.usage = config.PictureFormatProps()->imageUsageFlags;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (config.IsDecode()) {
            m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-04815");
        }
        if (config.IsEncode()) {
            m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-04816");
        }
        vk::CreateImage(device(), &image_ci, nullptr, &image);
        m_errorMonitor->VerifyFound();

        if (config.IsDecode()) {
            VkVideoProfileListInfoKHR video_profiles = vku::InitStructHelper();
            VkVideoProfileInfoKHR profiles[] = {*config.Profile(), *config.Profile()};
            video_profiles.profileCount = 2;
            video_profiles.pProfiles = profiles;
            image_ci.pNext = &video_profiles;

            m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-pNext-06811");
            m_errorMonitor->SetDesiredError("VUID-VkVideoProfileListInfoKHR-pProfiles-06813");
            vk::CreateImage(device(), &image_ci, nullptr, &image);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(NegativeVideo, CreateImageVideoEncodeAV1NotEnabled) {
    TEST_DESCRIPTION("vkCreateImage - AV1 encode is specified but videoEncodeAV1 was not enabled");

    ForceDisableFeature(vkt::Feature::videoEncodeAV1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VkImage image = VK_NULL_HANDLE;
    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = config.PictureFormatProps()->imageType;
    image_ci.format = config.PictureFormatProps()->format;
    image_ci.extent = {config.MaxCodedExtent().width, config.MaxCodedExtent().height, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = config.PictureFormatProps()->imageTiling;
    image_ci.usage = config.PictureFormatProps()->imageUsageFlags;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkVideoProfileListInfoKHR video_profiles = vku::InitStructHelper();
    video_profiles.profileCount = 1;
    video_profiles.pProfiles = config.Profile();
    image_ci.pNext = &video_profiles;

    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-pNext-10250");
    vk::CreateImage(device(), &image_ci, nullptr, &image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateImageProfileIndependentNotSupported) {
    TEST_DESCRIPTION("vkCreateImage - profile independent image creation requires videoMaintenance1");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VkImage image = VK_NULL_HANDLE;
    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = VK_IMAGE_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR;
    image_ci.imageType = config.PictureFormatProps()->imageType;
    image_ci.format = config.PictureFormatProps()->format;
    image_ci.extent = {config.MaxCodedExtent().width, config.MaxCodedExtent().height, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = config.PictureFormatProps()->imageTiling;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (GetConfigDecode()) {
        image_ci.usage |= VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR;
    }

    if (GetConfigEncode()) {
        image_ci.usage |= VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR;
    }

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-flags-parameter");
    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-flags-08328");
    vk::CreateImage(device(), &image_ci, nullptr, &image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateImageProfileIndependent) {
    TEST_DESCRIPTION("vkCreateImage - profile independent image creation with invalid parameters");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig decode_config = GetConfigDecode();
    VideoConfig encode_config = GetConfigEncode();
    if (!decode_config && !encode_config) {
        GTEST_SKIP() << "Test requires video decode or encode support";
    }

    for (auto config : {decode_config, encode_config}) {
        if (!config) continue;

        VkImage image = VK_NULL_HANDLE;
        VkImageCreateInfo image_ci = vku::InitStructHelper();
        image_ci.flags = VK_IMAGE_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR;
        image_ci.imageType = config.PictureFormatProps()->imageType;
        image_ci.format = config.PictureFormatProps()->format;
        image_ci.extent = {config.MaxCodedExtent().width, config.MaxCodedExtent().height, 1};
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.tiling = config.PictureFormatProps()->imageTiling;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (config.IsDecode()) {
            // Video profile independent DECODE_DPB usage is not allowed
            image_ci.usage = VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR;
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
            m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-flags-08329");
            vk::CreateImage(device(), &image_ci, nullptr, &image);
            m_errorMonitor->VerifyFound();

            // Except when DECODE_DST usage is also requested
            image_ci.usage |= VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR;
            vk::CreateImage(device(), &image_ci, nullptr, &image);
            vk::DestroyImage(device(), image, nullptr);

            // Profile list is still validated though
            VkVideoProfileListInfoKHR video_profiles = vku::InitStructHelper();
            VkVideoProfileInfoKHR profiles[] = {*config.Profile(), *config.Profile()};
            video_profiles.profileCount = 2;
            video_profiles.pProfiles = profiles;
            image_ci.pNext = &video_profiles;

            m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-pNext-06811");
            m_errorMonitor->SetDesiredError("VUID-VkVideoProfileListInfoKHR-pProfiles-06813");
            vk::CreateImage(device(), &image_ci, nullptr, &image);
            m_errorMonitor->VerifyFound();
        }

        if (config.IsEncode()) {
            // Video profile independent ENCODE_DPB usage is not allowed
            image_ci.usage = VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR;
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
            m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-flags-08331");
            vk::CreateImage(device(), &image_ci, nullptr, &image);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(NegativeVideo, CreateImageIncompatibleProfile) {
    TEST_DESCRIPTION("vkCreateImage - image parameters are incompatible with the video profile");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(m_device, config);

    VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
    profile_list.profileCount = 1;
    profile_list.pProfiles = config.Profile();

    const VkVideoFormatPropertiesKHR* format_props = config.DpbFormatProps();
    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    image_ci.pNext = &profile_list;
    image_ci.imageType = format_props->imageType;
    image_ci.format = format_props->format;
    image_ci.extent = {1024, 1024, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 6;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = format_props->imageTiling;
    image_ci.usage = format_props->imageUsageFlags;
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImage image = VK_NULL_HANDLE;

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-pNext-06811");
    image_ci.format = VK_FORMAT_D16_UNORM;
    vk::CreateImage(device(), &image_ci, nullptr, &image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateImageVideoEncodeQuantizationMapNotEnabled) {
    TEST_DESCRIPTION("vkCreateImage - quantization map usage is specified but videoEncodeQuantizationMap was not enabled");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    ForceDisableFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    std::vector<std::tuple<VideoConfig, VkImageUsageFlagBits, const VkVideoFormatPropertiesKHR*>> quantization_map_usages;
    if (delta_config) {
        quantization_map_usages.emplace_back(delta_config, VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                             delta_config.QuantDeltaMapProps());
    }
    if (emphasis_config) {
        quantization_map_usages.emplace_back(emphasis_config, VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                             emphasis_config.EmphasisMapProps());
    }

    for (const auto& [config, usage, props] : quantization_map_usages) {
        VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
        profile_list.profileCount = 1;
        profile_list.pProfiles = config.Profile();

        VkImageCreateInfo image_ci = vku::InitStructHelper();
        image_ci.flags = 0;
        image_ci.pNext = &profile_list;
        image_ci.extent = {
            config.EncodeQuantizationMapCaps()->maxQuantizationMapExtent.width,
            config.EncodeQuantizationMapCaps()->maxQuantizationMapExtent.height,
            1,
        };
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_ci.usage = usage;
        image_ci.imageType = props->imageType;
        image_ci.format = props->format;
        image_ci.tiling = props->imageTiling;

        VkImage image = VK_NULL_HANDLE;
        m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10251");
        vk::CreateImage(device(), &image_ci, nullptr, &image);
        m_errorMonitor->VerifyFound();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideo, CreateImageQuantDeltaMapUnsupportedProfile) {
    TEST_DESCRIPTION("vkCreateImage - quantization delta map image created against profile without cap");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR) == 0;
    }));
    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    if (!config || !delta_config) {
        GTEST_SKIP() << "Test requires an encode profile without quantization delta map support";
    }

    const VkVideoFormatPropertiesKHR* format_props = delta_config.QuantDeltaMapProps();

    VideoContext context(m_device, config);

    VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
    profile_list.profileCount = 1;
    profile_list.pProfiles = config.Profile();

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = 0;
    image_ci.pNext = &profile_list;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = format_props->format;
    image_ci.extent = {1, 1, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = format_props->imageTiling;
    image_ci.usage = VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR;
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImage image = VK_NULL_HANDLE;

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10255");
    vk::CreateImage(device(), &image_ci, nullptr, &image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateImageEmphasisMapUnsupportedProfile) {
    TEST_DESCRIPTION("vkCreateImage - emphasis map image created against profile without cap");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR) == 0;
    }));
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    if (!config || !emphasis_config) {
        GTEST_SKIP() << "Test requires an encode profile without emphasis map support";
    }

    const VkVideoFormatPropertiesKHR* format_props = emphasis_config.EmphasisMapProps();

    VideoContext context(m_device, config);

    VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
    profile_list.profileCount = 1;
    profile_list.pProfiles = config.Profile();

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = 0;
    image_ci.pNext = &profile_list;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = format_props->format;
    image_ci.extent = {1, 1, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = format_props->imageTiling;
    image_ci.usage = VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR;
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImage image = VK_NULL_HANDLE;

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10256");
    vk::CreateImage(device(), &image_ci, nullptr, &image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateImageQuantMapProfileIndependent) {
    TEST_DESCRIPTION("vkCreateImage - quantization maps cannot be profile independent");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    std::vector<std::tuple<VideoConfig, VkImageUsageFlagBits, const VkVideoFormatPropertiesKHR*>> quantization_map_usages;
    if (delta_config) {
        quantization_map_usages.emplace_back(delta_config, VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                             delta_config.QuantDeltaMapProps());
    }
    if (emphasis_config) {
        quantization_map_usages.emplace_back(emphasis_config, VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                             emphasis_config.EmphasisMapProps());
    }

    for (const auto& [config, usage, props] : quantization_map_usages) {
        VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
        profile_list.profileCount = 1;
        profile_list.pProfiles = config.Profile();

        VkImageCreateInfo image_ci = vku::InitStructHelper();
        image_ci.flags = VK_IMAGE_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR;
        image_ci.pNext = &profile_list;
        image_ci.extent = {
            config.EncodeQuantizationMapCaps()->maxQuantizationMapExtent.width,
            config.EncodeQuantizationMapCaps()->maxQuantizationMapExtent.height,
            1,
        };
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_ci.usage = usage;
        image_ci.imageType = props->imageType;
        image_ci.format = props->format;
        image_ci.tiling = props->imageTiling;

        VkImage image = VK_NULL_HANDLE;
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
        m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-flags-08331");
        vk::CreateImage(device(), &image_ci, nullptr, &image);
        m_errorMonitor->VerifyFound();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideo, CreateImageQuantMapInvalidParameters) {
    TEST_DESCRIPTION("vkCreateImage - image parameters are incompatible with quantization maps");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    VideoConfig decode_config = GetConfigDecode();

    std::vector<std::tuple<VideoConfig, VkImageUsageFlagBits, const VkVideoFormatPropertiesKHR*>> quantization_map_usages;
    if (delta_config) {
        quantization_map_usages.emplace_back(delta_config, VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                             delta_config.QuantDeltaMapProps());
    }
    if (emphasis_config) {
        quantization_map_usages.emplace_back(emphasis_config, VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                             emphasis_config.EmphasisMapProps());
    }

    for (const auto& [config, usage, props] : quantization_map_usages) {
        VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
        VkVideoProfileInfoKHR profiles[] = {*config.Profile(), *config.Profile()};
        profile_list.profileCount = 1;
        profile_list.pProfiles = profiles;

        VkImageCreateInfo image_ci = vku::InitStructHelper();
        image_ci.pNext = &profile_list;
        image_ci.flags = 0;
        image_ci.imageType = VK_IMAGE_TYPE_2D;
        image_ci.format = props->format;
        image_ci.extent = {1, 1, 1};
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.tiling = props->imageTiling;
        image_ci.usage = usage;
        image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VkImage image = VK_NULL_HANDLE;

        image_ci.imageType = VK_IMAGE_TYPE_1D;
        // Implementations are not expected to support non-2D quantization maps
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
        m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10252");
        vk::CreateImage(device(), &image_ci, nullptr, &image);
        m_errorMonitor->VerifyFound();

        image_ci.imageType = VK_IMAGE_TYPE_2D;
        image_ci.samples = VK_SAMPLE_COUNT_2_BIT;
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-samples-02257");
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-samples-02258");
        m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10253");
        vk::CreateImage(device(), &image_ci, nullptr, &image);
        m_errorMonitor->VerifyFound();

        // Try no profile list
        image_ci.pNext = nullptr;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10254");
        vk::CreateImage(device(), &image_ci, nullptr, &image);
        m_errorMonitor->VerifyFound();

        // Try profileCount != 1
        image_ci.pNext = &profile_list;
        profile_list.profileCount = 0;
        m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10254");
        vk::CreateImage(device(), &image_ci, nullptr, &image);
        m_errorMonitor->VerifyFound();

        profile_list.profileCount = 2;
        m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10254");
        vk::CreateImage(device(), &image_ci, nullptr, &image);
        m_errorMonitor->VerifyFound();

        // Try decode profile, if available
        profile_list.profileCount = 1;
        if (decode_config) {
            profiles[0] = *decode_config.Profile();
            // Implementations probably don't support quantization map formats with decode profile
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
            m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10254");
            vk::CreateImage(device(), &image_ci, nullptr, &image);
            m_errorMonitor->VerifyFound();
        }

        // Try extent > maxQuantizationMapExtent
        profiles[0] = *config.Profile();
        image_ci.extent.width = config.EncodeQuantizationMapCaps()->maxQuantizationMapExtent.width + 1;
        m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10257");
        vk::CreateImage(device(), &image_ci, nullptr, &image);
        m_errorMonitor->VerifyFound();

        image_ci.extent.width = 1;
        image_ci.extent.height = config.EncodeQuantizationMapCaps()->maxQuantizationMapExtent.height + 1;
        m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-10258");
        vk::CreateImage(device(), &image_ci, nullptr, &image);
        m_errorMonitor->VerifyFound();
    };

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideo, CreateImageViewQuantMapInvalidViewType) {
    TEST_DESCRIPTION("vkCreateImageView - view type not compatible with quantization maps");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    std::vector<std::tuple<VideoConfig, VkImageUsageFlagBits, const VkVideoFormatPropertiesKHR*>> quantization_map_usages;
    if (delta_config) {
        quantization_map_usages.emplace_back(delta_config, VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                             delta_config.QuantDeltaMapProps());
    }
    if (emphasis_config) {
        quantization_map_usages.emplace_back(emphasis_config, VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                             emphasis_config.EmphasisMapProps());
    }

    for (const auto& [config, usage, props] : quantization_map_usages) {
        VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
        profile_list.profileCount = 1;
        profile_list.pProfiles = config.Profile();

        VkImageCreateInfo image_ci = vku::InitStructHelper();
        image_ci.pNext = &profile_list;
        image_ci.imageType = props->imageType;
        image_ci.format = props->format;
        image_ci.extent = {1, 1, 1};
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.tiling = props->imageTiling;
        image_ci.usage = props->imageUsageFlags;
        image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        vkt::Image image(*m_device, image_ci);
        VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
        image_view_ci.image = image.handle();
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_1D;
        image_view_ci.format = image_ci.format;
        image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_ci.subresourceRange.levelCount = 1;
        image_view_ci.subresourceRange.layerCount = 1;

        m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageViewCreateInfo-subResourceRange-01021");
        m_errorMonitor->SetDesiredError("VUID-VkImageViewCreateInfo-image-10261");
        VkImageView image_view = VK_NULL_HANDLE;
        vk::CreateImageView(device(), &image_view_ci, nullptr, &image_view);
        m_errorMonitor->VerifyFound();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideo, CreateImageViewQuantMapUnsupportedFormatFeatures) {
    TEST_DESCRIPTION("vkCreateImageView - quantization map view format features unsupported");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());
    ASSERT_TRUE(delta_config || emphasis_config)
        << "Support for videoEncodeQuantizationMap implies at least one encode profile should support some quantization map type";

    std::vector<std::tuple<VideoConfig, VkImageUsageFlagBits, const VkVideoFormatPropertiesKHR*, const char*>>
        quantization_map_usages;
    if (delta_config) {
        quantization_map_usages.emplace_back(delta_config, VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                             delta_config.QuantDeltaMapProps(), "VUID-VkImageViewCreateInfo-usage-10259");
    }
    if (emphasis_config) {
        quantization_map_usages.emplace_back(emphasis_config, VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                             emphasis_config.EmphasisMapProps(), "VUID-VkImageViewCreateInfo-usage-10260");
    }

    for (auto& [config, usage, props, vuid] : quantization_map_usages) {
        VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
        profile_list.profileCount = 1;
        profile_list.pProfiles = config.Profile();

        VkImageCreateInfo image_ci = vku::InitStructHelper();
        image_ci.pNext = &profile_list;
        image_ci.flags = 0;
        image_ci.imageType = props->imageType;
        image_ci.format = props->format;
        image_ci.extent = {1, 1, 1};
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.tiling = props->imageTiling;
        image_ci.usage = props->imageUsageFlags;
        image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        vkt::Image image(*m_device, image_ci);
        VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
        image_view_ci.image = image.handle();
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        // We will just use a definitely not supported view format here because we anyway
        // did not create the image with MUTABLE, so no need to worry about video format compatibility
        image_view_ci.format = VK_FORMAT_D32_SFLOAT;
        image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_ci.subresourceRange.levelCount = 1;
        image_view_ci.subresourceRange.layerCount = 1;

        // The image is not created with MUTABLE, so we need to allow this VU.
        // This should also bypass view format compatibility checks
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageViewCreateInfo-image-01762");
        m_errorMonitor->SetDesiredError(vuid);
        VkImageView image_view = VK_NULL_HANDLE;
        vk::CreateImageView(device(), &image_view_ci, nullptr, &image_view);
        m_errorMonitor->VerifyFound();
    }

    if (!delta_config || !emphasis_config) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideo, CreateImageViewInvalidViewType) {
    TEST_DESCRIPTION("vkCreateImageView - view type not compatible with video usage");

    RETURN_IF_SKIP(Init());

    VideoConfig decode_config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    VideoConfig encode_config = GetConfig(GetConfigsWithReferences(GetConfigsEncode()));
    if (!decode_config && !encode_config) {
        GTEST_SKIP() << "Test requires a video profile with reference picture support";
    }

    for (auto config : {decode_config, encode_config}) {
        if (!config) continue;

        config.SessionCreateInfo()->maxDpbSlots = 1;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
        profile_list.profileCount = 1;
        profile_list.pProfiles = config.Profile();

        const VkVideoFormatPropertiesKHR* format_props = config.DpbFormatProps();
        VkImageCreateInfo image_ci = vku::InitStructHelper();
        image_ci.pNext = &profile_list;
        image_ci.imageType = format_props->imageType;
        image_ci.format = format_props->format;
        image_ci.extent = {1024, 1024, 1};
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 6;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.tiling = format_props->imageTiling;
        image_ci.usage = format_props->imageUsageFlags;
        image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        vkt::Image image(*m_device, image_ci);

        VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
        image_view_ci.image = image.handle();
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        image_view_ci.format = image_ci.format;
        image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_ci.subresourceRange.levelCount = 1;
        image_view_ci.subresourceRange.layerCount = 6;

        m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageViewCreateInfo-image-01003");
        if (config.IsDecode()) {
            m_errorMonitor->SetDesiredError("VUID-VkImageViewCreateInfo-image-04817");
        }
        if (config.IsEncode()) {
            m_errorMonitor->SetDesiredError("VUID-VkImageViewCreateInfo-image-04818");
        }
        VkImageView image_view = VK_NULL_HANDLE;
        vk::CreateImageView(device(), &image_view_ci, nullptr, &image_view);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeVideo, CreateImageViewProfileIndependent) {
    TEST_DESCRIPTION("vkCreateImageView - video usage not supported in view created from video profile independent image");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig decode_config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    VideoConfig encode_config = GetConfig(GetConfigsWithReferences(GetConfigsEncode()));
    if (!decode_config && !encode_config) {
        GTEST_SKIP() << "Test requires a video profile with reference picture support";
    }

    struct TestCase {
        VideoConfig& config;
        VkImageUsageFlags usage;
        const char* vuid;
    };

    std::vector<TestCase> test_cases = {
        {decode_config, VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR, "VUID-VkImageViewCreateInfo-image-08333"},
        {decode_config, VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR, "VUID-VkImageViewCreateInfo-image-08334"},
        {decode_config, VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR, "VUID-VkImageViewCreateInfo-image-08335"},
        {encode_config, VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR, "VUID-VkImageViewCreateInfo-image-08336"},
        {encode_config, VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR, "VUID-VkImageViewCreateInfo-image-08337"},
        {encode_config, VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR, "VUID-VkImageViewCreateInfo-image-08338"},
    };

    // We choose a format that is not expected to support video usage
    VkFormat format = VK_FORMAT_R8G8_SNORM;
    VkFormatFeatureFlags video_format_flags =
        VK_FORMAT_FEATURE_VIDEO_DECODE_OUTPUT_BIT_KHR | VK_FORMAT_FEATURE_VIDEO_DECODE_DPB_BIT_KHR |
        VK_FORMAT_FEATURE_VIDEO_ENCODE_INPUT_BIT_KHR | VK_FORMAT_FEATURE_VIDEO_ENCODE_DPB_BIT_KHR;
    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(gpu(), format, &format_props);
    if ((format_props.optimalTilingFeatures & video_format_flags) != 0) {
        GTEST_SKIP() << "Test expects R8G8_SNORM format to not support video usage";
    }

    for (const auto& test_case : test_cases) {
        if (!test_case.config) {
            continue;
        }

        auto image_ci = vku::InitStruct<VkImageCreateInfo>();
        image_ci.flags = VK_IMAGE_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT;
        image_ci.imageType = VK_IMAGE_TYPE_2D;
        image_ci.format = format;
        image_ci.extent = {1024, 1024, 1};
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        vkt::Image image(*m_device, image_ci);

        auto image_usage_ci = vku::InitStruct<VkImageViewUsageCreateInfo>();
        image_usage_ci.usage = test_case.usage;
        auto image_view_ci = vku::InitStruct<VkImageViewCreateInfo>(&image_usage_ci);
        image_view_ci.image = image.handle();
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_ci.format = format;
        image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_ci.subresourceRange.levelCount = 1;
        image_view_ci.subresourceRange.layerCount = 1;

        VkImageView image_view = VK_NULL_HANDLE;

        m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageViewCreateInfo-pNext-02662");
        m_errorMonitor->SetDesiredError(test_case.vuid);
        vk::CreateImageView(device(), &image_view_ci, nullptr, &image_view);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeVideo, CreateQueryPoolVideoEncodeAV1NotEnabled) {
    TEST_DESCRIPTION("vkCreateQueryPool - AV1 encode is specified but videoEncodeAV1 was not enabled");

    ForceDisableFeature(vkt::Feature::videoEncodeAV1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    auto encode_feedback_info = vku::InitStruct<VkQueryPoolVideoEncodeFeedbackCreateInfoKHR>(config.Profile());
    encode_feedback_info.encodeFeedbackFlags = VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BYTES_WRITTEN_BIT_KHR;

    VkQueryPool query_pool = VK_NULL_HANDLE;
    auto create_info = vku::InitStruct<VkQueryPoolCreateInfo>(&encode_feedback_info);
    create_info.queryType = VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR;
    create_info.queryCount = 1;

    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-pNext-10248");
    vk::CreateQueryPool(device(), &create_info, nullptr, &query_pool);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateQueryPoolMissingEncodeFeedbackInfo) {
    TEST_DESCRIPTION("vkCreateQueryPool - missing VkQueryPoolVideoEncodeFeedbackCreateInfoKHR");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    auto create_info = vku::InitStruct<VkQueryPoolCreateInfo>(config.Profile());
    create_info.queryType = VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR;
    create_info.queryCount = 1;

    VkQueryPool query_pool;
    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-queryType-07906");
    vk::CreateQueryPool(device(), &create_info, nullptr, &query_pool);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateQueryPoolEncodeFeedbackProfile) {
    TEST_DESCRIPTION("vkCreateQueryPool - require encode profile for ENCODE_FEEDBACK query");

    RETURN_IF_SKIP(Init());

    VideoConfig decode_config = GetConfigDecode();
    VideoConfig encode_config = GetConfigEncode();
    if (!decode_config || !encode_config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    auto encode_feedback_info = vku::InitStruct<VkQueryPoolVideoEncodeFeedbackCreateInfoKHR>(decode_config.Profile());
    encode_feedback_info.encodeFeedbackFlags = VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BYTES_WRITTEN_BIT_KHR;

    auto create_info = vku::InitStruct<VkQueryPoolCreateInfo>(&encode_feedback_info);
    create_info.queryType = VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR;
    create_info.queryCount = 1;

    VkQueryPool query_pool;
    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-queryType-07133");
    vk::CreateQueryPool(device(), &create_info, nullptr, &query_pool);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateQueryPoolUnsupportedEncodeFeedback) {
    TEST_DESCRIPTION("vkCreateQueryPool - missing VkQueryPoolVideoEncodeFeedbackCreateInfoKHR");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return config.EncodeCaps()->supportedEncodeFeedbackFlags != AllVkVideoEncodeFeedbackFlagBitsKHR;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile that does not support all encode feedback flags";
    }

    auto encode_feedback_info = vku::InitStruct<VkQueryPoolVideoEncodeFeedbackCreateInfoKHR>(config.Profile());
    encode_feedback_info.encodeFeedbackFlags = AllVkVideoEncodeFeedbackFlagBitsKHR;

    auto create_info = vku::InitStruct<VkQueryPoolCreateInfo>(&encode_feedback_info);
    create_info.queryType = VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR;
    create_info.queryCount = 1;

    VkQueryPool query_pool;
    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-queryType-07907");
    vk::CreateQueryPool(device(), &create_info, nullptr, &query_pool);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, BeginQueryIncompatibleQueueFamily) {
    TEST_DESCRIPTION("vkCmdBeginQuery - result status only queries require queue family support");

    RETURN_IF_SKIP(Init());

    uint32_t queue_family_index = VK_QUEUE_FAMILY_IGNORED;
    for (uint32_t qfi = 0; qfi < QueueFamilyCount(); ++qfi) {
        if (!QueueFamilySupportsResultStatusOnlyQueries(qfi)) {
            queue_family_index = qfi;
            break;
        }
    }

    if (queue_family_index == VK_QUEUE_FAMILY_IGNORED) {
        GTEST_SKIP() << "Test requires a queue family with no support for result status queries";
    }

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR, 1);

    vkt::CommandPool cmd_pool(*m_device, queue_family_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer cb(*m_device, cmd_pool);

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-07126");
    vk::CmdBeginQuery(cb.handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, BeginQueryVideoCodingScopeQueryAlreadyActive) {
    TEST_DESCRIPTION("vkCmdBeginQuery - there must be no active query in video scope");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateStatusQueryPool(2);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 0, 0);

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginQuery-queryPool-01922");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-None-07127");
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 1, 0);
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(cb.handle(), context.StatusQueryPool(), 0);
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, BeginQueryResultStatusProfileMismatch) {
    TEST_DESCRIPTION("vkCmdBeginQuery - result status query must have been created with the same profile");

    RETURN_IF_SKIP(Init());

    auto configs = GetConfigs();
    if (configs.size() < 2) {
        GTEST_SKIP() << "Test requires support for at least two video profiles";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(configs[0].QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    VideoContext context1(m_device, configs[0]);
    VideoContext context2(m_device, configs[1]);
    context1.CreateAndBindSessionMemory();
    context2.CreateStatusQueryPool();

    vkt::CommandBuffer& cb = context1.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-07128");
    vk::CmdBeginQuery(cb.handle(), context2.StatusQueryPool(), 0, 0);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.End();
}

TEST_F(NegativeVideo, BeginQueryEncodeFeedbackProfileMismatch) {
    TEST_DESCRIPTION("vkCmdBeginQuery - encode feedback query must have been created with the same profile");

    RETURN_IF_SKIP(Init());

    auto configs = GetConfigsEncode();
    if (configs.size() < 2) {
        GTEST_SKIP() << "Test requires support for at least two video encode profiles";
    }

    VideoContext context1(m_device, configs[0]);
    VideoContext context2(m_device, configs[1]);
    context1.CreateAndBindSessionMemory();
    context2.CreateEncodeFeedbackQueryPool();

    vkt::CommandBuffer& cb = context1.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-07130");
    vk::CmdBeginQuery(cb.handle(), context2.EncodeFeedbackQueryPool(), 0, 0);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.End();
}

TEST_F(NegativeVideo, BeginQueryEncodeFeedbackNoBoundVideoSession) {
    TEST_DESCRIPTION("vkCmdBeginQuery - encode feedback query requires bound video session");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateEncodeFeedbackQueryPool();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-07129");
    vk::CmdBeginQuery(cb.handle(), context.EncodeFeedbackQueryPool(), 0, 0);
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, BeginQueryVideoCodingScopeIncompatibleQueryType) {
    TEST_DESCRIPTION("vkCmdBeginQuery - incompatible query type used in video coding scope");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginQuery-queryType-00803");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginQuery-queryType-07128");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-07131");
    vk::CmdBeginQuery(cb.handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, BeginQueryInlineQueries) {
    TEST_DESCRIPTION("vkCmdBeginQuery - bound video session was created with VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }
    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateStatusQueryPool();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-None-08370");
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 0, 0);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, GetQueryPoolResultsVideoQueryDataSize) {
    TEST_DESCRIPTION("vkGetQueryPoolResults - test expected data size for video query results");
    RETURN_IF_SKIP(InitFramework(&kDisableMessageLimit));
    RETURN_IF_SKIP(InitState());

    auto config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    auto feedback_flags = config.EncodeCaps()->supportedEncodeFeedbackFlags;
    auto feedback_flag_count = GetBitSetCount(feedback_flags);
    uint32_t total_query_count = 4;

    VideoContext context(m_device, config);
    context.CreateEncodeFeedbackQueryPool(total_query_count, feedback_flags);

    auto query_pool = context.EncodeFeedbackQueryPool();
    std::vector<uint64_t> results(feedback_flag_count + 1);

    for (uint32_t query_count = 1; query_count <= total_query_count; query_count++) {
        size_t total_feedback_count = feedback_flag_count * query_count;

        // Test 32-bit no availability/status
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkGetQueryPoolResults-None-09401");
        m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-dataSize-00817");
        vk::GetQueryPoolResults(device(), query_pool, 0, query_count, sizeof(uint32_t) * (total_feedback_count - 1), results.data(),
                                sizeof(uint32_t) * (total_feedback_count - 1), 0);
        m_errorMonitor->VerifyFound();

        // Test 32-bit with availability
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkGetQueryPoolResults-None-09401");
        m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-dataSize-00817");
        vk::GetQueryPoolResults(device(), query_pool, 0, query_count, sizeof(uint32_t) * (total_feedback_count + query_count - 1),
                                results.data(), sizeof(uint32_t) * (total_feedback_count + query_count - 1),
                                VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);
        m_errorMonitor->VerifyFound();

        // Test 32-bit with status
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkGetQueryPoolResults-None-09401");
        m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-dataSize-00817");
        vk::GetQueryPoolResults(device(), query_pool, 0, query_count, sizeof(uint32_t) * (total_feedback_count + query_count - 1),
                                results.data(), sizeof(uint32_t) * (total_feedback_count + query_count - 1),
                                VK_QUERY_RESULT_WITH_STATUS_BIT_KHR);
        m_errorMonitor->VerifyFound();

        // Test 64-bit no availability/status
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkGetQueryPoolResults-None-09401");
        m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-dataSize-00817");
        vk::GetQueryPoolResults(device(), query_pool, 0, query_count, sizeof(uint64_t) * (total_feedback_count - 1), results.data(),
                                sizeof(uint64_t) * (total_feedback_count - 1), VK_QUERY_RESULT_64_BIT);
        m_errorMonitor->VerifyFound();

        // Test 64-bit with availability
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkGetQueryPoolResults-None-09401");
        m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-dataSize-00817");
        vk::GetQueryPoolResults(device(), query_pool, 0, query_count, sizeof(uint64_t) * (total_feedback_count + query_count - 1),
                                results.data(), sizeof(uint64_t) * (total_feedback_count + query_count - 1),
                                VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);
        m_errorMonitor->VerifyFound();

        // Test 64-bit with status
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkGetQueryPoolResults-None-09401");
        m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-dataSize-00817");
        vk::GetQueryPoolResults(device(), query_pool, 0, query_count, sizeof(uint64_t) * (total_feedback_count + query_count - 1),
                                results.data(), sizeof(uint64_t) * (total_feedback_count + query_count - 1),
                                VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_STATUS_BIT_KHR);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeVideo, GetQueryPoolResultsStatusBit) {
    TEST_DESCRIPTION("vkGetQueryPoolResults - test invalid use of VK_QUERY_RESULT_WITH_STATUS_BIT_KHR");

    RETURN_IF_SKIP(Init());

    if (!GetConfig()) {
        GTEST_SKIP() << "Test requires video support";
    }

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR, 1);

    uint32_t status;
    VkQueryResultFlags flags;

    m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-queryType-09442");
    flags = 0;
    vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 1, sizeof(status), &status, sizeof(status), flags);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-flags-09443");
    flags = VK_QUERY_RESULT_WITH_STATUS_BIT_KHR | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT;
    vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 1, sizeof(status), &status, sizeof(status), flags);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CopyQueryPoolResultsStatusBit) {
    TEST_DESCRIPTION("vkCmdCopyQueryPoolResults - test invalid use of VK_QUERY_RESULT_WITH_STATUS_BIT_KHR");

    RETURN_IF_SKIP(Init());

    if (!GetConfig()) {
        GTEST_SKIP() << "Test requires video support";
    }

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR, 1);

    VkQueryResultFlags flags;

    vkt::Buffer buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-queryType-09442");
    flags = 0;
    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0, 1, buffer.handle(), 0, sizeof(uint32_t), flags);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-flags-09443");
    flags = VK_QUERY_RESULT_WITH_STATUS_BIT_KHR | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT;
    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0, 1, buffer.handle(), 0, sizeof(uint32_t), flags);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeVideo, ImageLayoutUsageMismatch) {
    TEST_DESCRIPTION("Image layout in image memory barrier is invalid for image usage");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto image_barrier = vku::InitStruct<VkImageMemoryBarrier>();
    image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_barrier.subresourceRange.levelCount = 1;
    image_barrier.subresourceRange.layerCount = 1;

    auto image_barrier2 = vku::InitStruct<VkImageMemoryBarrier2>();
    image_barrier2.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    image_barrier2.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    image_barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_barrier2.subresourceRange.levelCount = 1;
    image_barrier2.subresourceRange.layerCount = 1;

    auto dep_info = vku::InitStruct<VkDependencyInfo>();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &image_barrier2;

    struct TestParams {
        VkImage image;
        VkImageLayout invalid_layout;
        VkImageLayout valid_layout;
        const char* vuid;
        const char* vuid2;
    };

    std::vector<TestParams> test_params = {
        {context.DecodeOutput()->Image(), VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR,
         "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07120", "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07120"},
        {context.Dpb()->Image(), VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR, VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR,
         "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07120", "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07120"}};

    if (config.SupportsDecodeOutputDistinct()) {
        test_params.emplace_back(TestParams{
            context.Dpb()->Image(), VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR,
            "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07121", "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07121"});
        test_params.emplace_back(TestParams{
            context.DecodeOutput()->Image(), VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR,
            "VUID-VkImageMemoryBarrier-srcQueueFamilyIndex-07122", "VUID-VkImageMemoryBarrier2-srcQueueFamilyIndex-07122"});
    }

    cb.Begin();

    for (const auto& params : test_params) {
        image_barrier.image = params.image;

        m_errorMonitor->SetDesiredError(params.vuid);
        image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_barrier.newLayout = params.invalid_layout;
        vk::CmdPipelineBarrier(cb.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                               0, nullptr, 1, &image_barrier);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError(params.vuid);
        image_barrier.oldLayout = params.invalid_layout;
        image_barrier.newLayout = params.valid_layout;
        vk::CmdPipelineBarrier(cb.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                               0, nullptr, 1, &image_barrier);
        m_errorMonitor->VerifyFound();

        image_barrier2.image = params.image;

        m_errorMonitor->SetDesiredError(params.vuid2);
        image_barrier2.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_barrier2.newLayout = params.invalid_layout;
        vk::CmdPipelineBarrier2KHR(cb.handle(), &dep_info);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredError(params.vuid2);
        image_barrier2.oldLayout = params.invalid_layout;
        image_barrier2.newLayout = params.valid_layout;
        vk::CmdPipelineBarrier2KHR(cb.handle(), &dep_info);
        m_errorMonitor->VerifyFound();
    }

    cb.End();
}

TEST_F(NegativeVideoSyncVal, DecodeOutputPicture) {
    TEST_DESCRIPTION("Test video decode output picture sync hazard");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires decode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset());

    cb.DecodeVideo(context.DecodeFrame());

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    cb.DecodeVideo(context.DecodeFrame());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoSyncVal, DecodeReconstructedPicture) {
    TEST_DESCRIPTION("Test video decode reconstructed picture sync hazard");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(FilterConfigs(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 2),
                                          [](const VideoConfig& config) { return config.SupportsDecodeOutputDistinct(); }));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with references, 2 DPB slots, and distinct mode support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1));
    cb.ControlVideoCoding(context.Control().Reset());

    cb.DecodeVideo(context.DecodeFrame(0));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->MemoryBarrier());

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    cb.DecodeVideo(context.DecodeFrame(0));
    m_errorMonitor->VerifyFound();

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(0, 1));
    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(0, 1));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->MemoryBarrier());

    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->MemoryBarrier());

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    cb.DecodeVideo(context.DecodeFrame(0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoSyncVal, DecodeReferencePicture) {
    TEST_DESCRIPTION("Test video decode reference picture sync hazard");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with references and 3 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 3;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(2, 2));
    cb.ControlVideoCoding(context.Control().Reset());

    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->MemoryBarrier());

    cb.DecodeVideo(context.DecodeReferenceFrame(1));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->MemoryBarrier());

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0));
    m_errorMonitor->VerifyFound();

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(0, 1));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(2, 1));
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->MemoryBarrier());
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(2, 1));

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(1));
    m_errorMonitor->VerifyFound();

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(1, 2));
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(1));

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoSyncVal, EncodeBitstream) {
    TEST_DESCRIPTION("Test video encode bitstream sync hazard");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires Encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset());

    cb.EncodeVideo(context.EncodeFrame());

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    cb.EncodeVideo(context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoSyncVal, EncodeReconstructedPicture) {
    TEST_DESCRIPTION("Test video encode reconstructed picture sync hazard");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with references and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1));
    cb.ControlVideoCoding(context.Control().Reset());

    cb.EncodeVideo(context.EncodeFrame(0));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Bitstream().MemoryBarrier());

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    cb.EncodeVideo(context.EncodeReferenceFrame(0));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    cb.EncodeVideo(context.EncodeFrame(0));
    m_errorMonitor->VerifyFound();

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(0, 1));
    cb.EncodeVideo(context.EncodeReferenceFrame(0));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(0, 1));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Bitstream().MemoryBarrier());

    cb.EncodeVideo(context.EncodeFrame(1).AddReferenceFrame(0));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Bitstream().MemoryBarrier());

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    cb.EncodeVideo(context.EncodeReferenceFrame(0));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
    cb.EncodeVideo(context.EncodeFrame(0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoSyncVal, EncodeReferencePicture) {
    TEST_DESCRIPTION("Test video encode reference picture sync hazard");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncode()), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with references and 3 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 3;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(2, 2));
    cb.ControlVideoCoding(context.Control().Reset());

    cb.EncodeVideo(context.EncodeReferenceFrame(0));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Bitstream().MemoryBarrier());

    cb.EncodeVideo(context.EncodeReferenceFrame(1));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Bitstream().MemoryBarrier());

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(0));
    m_errorMonitor->VerifyFound();

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(0, 1));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(2, 1));
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(0));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Bitstream().MemoryBarrier());
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(2, 1));

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(1));
    m_errorMonitor->VerifyFound();

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(1, 2));
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(1));

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoSyncVal, EncodeQuantizationMap) {
    TEST_DESCRIPTION("Test video encode quantization map sync hazard");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    VideoConfig delta_config = GetConfigWithQuantDeltaMap(GetConfigsEncode());
    VideoConfig emphasis_config = GetConfigWithEmphasisMap(GetConfigsEncode());

    if ((!delta_config || (QueueFamilyFlags(delta_config.QueueFamilyIndex()) & VK_QUEUE_TRANSFER_BIT) == 0) &&
        (!emphasis_config || (QueueFamilyFlags(emphasis_config.QueueFamilyIndex()) & VK_QUEUE_TRANSFER_BIT) == 0)) {
        GTEST_SKIP() << "Test case requires video encode queue to also support transfer";
    }

    struct TestConfig {
        VideoConfig config;
        VkVideoEncodeFlagBitsKHR encode_flag;
        VkVideoSessionCreateFlagBitsKHR session_create_flag;
        const VkVideoFormatPropertiesKHR* map_props;
    };

    std::vector<TestConfig> tests;
    if (delta_config) {
        tests.emplace_back(TestConfig{delta_config, VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                      delta_config.QuantDeltaMapProps()});
    }
    if (emphasis_config) {
        tests.emplace_back(TestConfig{emphasis_config, VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR,
                                      VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR,
                                      emphasis_config.EmphasisMapProps()});
    }

    for (auto& [config, encode_flag, session_create_flag, map_props] : tests) {
        config.SessionCreateInfo()->flags |= session_create_flag;
        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        const auto texel_size = config.GetQuantMapTexelSize(*map_props);
        auto params = context.CreateSessionParamsWithQuantMapTexelSize(texel_size);

        vkt::CommandBuffer& cb = context.CmdBuffer();

        VideoEncodeQuantizationMap quantization_map(m_device, config, *map_props);

        cb.Begin();

        VkClearColorValue clear_value{};
        VkImageSubresourceRange subres_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        vk::CmdClearColorImage(cb.handle(), quantization_map.Image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_value, 1,
                               &subres_range);

        cb.BeginVideoCoding(context.Begin().SetSessionParams(params));
        cb.ControlVideoCoding(context.Control().Reset());

        m_errorMonitor->SetDesiredError("SYNC-HAZARD-READ-AFTER-WRITE");
        cb.EncodeVideo(context.EncodeFrame().QuantizationMap(encode_flag, texel_size, quantization_map));
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.End();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().SetSessionParams(params));
        cb.ControlVideoCoding(context.Control().Reset());

        cb.EncodeVideo(context.EncodeFrame().QuantizationMap(encode_flag, texel_size, quantization_map));
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());

        m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-READ");
        vk::CmdClearColorImage(cb.handle(), quantization_map.Image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_value, 1,
                               &subres_range);
        m_errorMonitor->VerifyFound();

        cb.End();
    }

    if (!delta_config || (QueueFamilyFlags(delta_config.QueueFamilyIndex()) & VK_QUEUE_TRANSFER_BIT) == 0 || !emphasis_config ||
        (QueueFamilyFlags(emphasis_config.QueueFamilyIndex()) & VK_QUEUE_TRANSFER_BIT) == 0) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}

TEST_F(NegativeVideoBestPractices, GetVideoSessionMemoryRequirements) {
    TEST_DESCRIPTION("vkGetVideoSessionMemoryRequirementsKHR - best practices");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(m_device, config);

    auto mem_req = vku::InitStruct<VkVideoSessionMemoryRequirementsKHR>();
    uint32_t mem_req_count = 1;

    m_errorMonitor->SetDesiredWarning("BestPractices-vkGetVideoSessionMemoryRequirementsKHR-count-not-retrieved");
    vk::GetVideoSessionMemoryRequirementsKHR(device(), context.Session(), &mem_req_count, &mem_req);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoBestPractices, BindVideoSessionMemory) {
    TEST_DESCRIPTION("vkBindVideoSessionMemoryKHR - best practices");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(m_device, config);

    // Create a buffer to get non-video-related memory requirements
    VkBufferCreateInfo buffer_create_info =
        vku::InitStruct<VkBufferCreateInfo>(nullptr, static_cast<VkBufferCreateFlags>(0), static_cast<VkDeviceSize>(4096),
                                          static_cast<VkBufferUsageFlags>(VK_BUFFER_USAGE_TRANSFER_SRC_BIT));
    vkt::Buffer buffer(*m_device, buffer_create_info);
    VkMemoryRequirements buf_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer, &buf_mem_reqs);

    // Create non-video-related DeviceMemory
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = buf_mem_reqs.size;
    ASSERT_TRUE(m_device->phy().SetMemoryType(buf_mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    vkt::DeviceMemory memory(*m_device, alloc_info);

    // Set VkBindVideoSessionMemoryInfoKHR::memory to an allocation created before GetVideoSessionMemoryRequirementsKHR was called
    auto bind_info = vku::InitStruct<VkBindVideoSessionMemoryInfoKHR>();
    bind_info.memory = memory;
    bind_info.memoryOffset = 0;
    bind_info.memorySize = alloc_info.allocationSize;

    m_errorMonitor->SetDesiredWarning("BestPractices-vkBindVideoSessionMemoryKHR-requirements-count-not-retrieved");
    vk::BindVideoSessionMemoryKHR(device(), context.Session(), 1, &bind_info);
    m_errorMonitor->VerifyFound();

    uint32_t mem_req_count = 0;
    vk::GetVideoSessionMemoryRequirementsKHR(device(), context.Session(), &mem_req_count, nullptr);

    if (mem_req_count > 0) {
        m_errorMonitor->SetDesiredWarning("BestPractices-vkBindVideoSessionMemoryKHR-requirements-not-all-retrieved");
        vk::BindVideoSessionMemoryKHR(device(), context.Session(), 1, &bind_info);
        m_errorMonitor->VerifyFound();

        if (mem_req_count > 1) {
            auto mem_req = vku::InitStruct<VkVideoSessionMemoryRequirementsKHR>();
            mem_req_count = 1;

            vk::GetVideoSessionMemoryRequirementsKHR(device(), context.Session(), &mem_req_count, &mem_req);

            m_errorMonitor->SetDesiredWarning("BestPractices-vkBindVideoSessionMemoryKHR-requirements-not-all-retrieved");
            vk::BindVideoSessionMemoryKHR(device(), context.Session(), 1, &bind_info);
            m_errorMonitor->VerifyFound();
        }
    }
}
