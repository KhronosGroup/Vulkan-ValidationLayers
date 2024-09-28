/*
 * Copyright (c) 2022-2024 The Khronos Group Inc.
 * Copyright (c) 2022-2024 RasterGrid Kft.
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
    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkEndCommandBuffer-None-06991");
    vk::EndCommandBuffer(cb.handle());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();

    // vkCmdEndVideoCoding not allowed outside video coding block
    cb.begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEndVideoCodingKHR-videocoding");
    cb.EndVideoCoding(context.End());
    m_errorMonitor->VerifyFound();

    cb.end();

    // vkCmdBeginVideoCoding not allowed inside video coding block
    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-videocoding");
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();

    // vkCmdControlVideoCoding not allowed outside video coding block
    cb.begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdControlVideoCodingKHR-videocoding");
    cb.ControlVideoCoding(context.Control().Reset());
    m_errorMonitor->VerifyFound();

    cb.end();
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
    vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), &profile, config.Caps());
    m_errorMonitor->VerifyFound();

    // Multiple bits in lumaBitDepth
    m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-lumaBitDepth-07014");
    profile = *config.Profile();
    profile.lumaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR | VK_VIDEO_COMPONENT_BIT_DEPTH_10_BIT_KHR;
    vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), &profile, config.Caps());
    m_errorMonitor->VerifyFound();

    // Multiple bits in chromaBitDepth
    m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-chromaSubsampling-07015");
    profile = *config.Profile();
    profile.chromaSubsampling = VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR;
    profile.chromaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR | VK_VIDEO_COMPONENT_BIT_DEPTH_10_BIT_KHR;
    vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), &profile, config.Caps());
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
        vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), &profile, config.Caps());
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigDecodeH265()) {
        VideoConfig config = GetConfigDecodeH265();

        profile = *config.Profile();
        profile.pNext = nullptr;

        // Missing codec-specific info for H.265 decode profile
        m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07180");
        vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), &profile, config.Caps());
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigDecodeAV1()) {
        VideoConfig config = GetConfigDecodeAV1();

        profile = *config.Profile();
        profile.pNext = nullptr;

        // Missing codec-specific info for AV1 decode profile
        m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-09256");
        vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), &profile, config.Caps());
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigEncodeH264()) {
        VideoConfig config = GetConfigEncodeH264();

        profile = *config.Profile();
        profile.pNext = nullptr;

        // Missing codec-specific info for H.264 encode profile
        m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07181");
        vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), &profile, config.Caps());
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigEncodeH265()) {
        VideoConfig config = GetConfigEncodeH265();

        profile = *config.Profile();
        profile.pNext = nullptr;

        // Missing codec-specific info for H.265 encode profile
        m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07182");
        vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), &profile, config.Caps());
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
        vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
        m_errorMonitor->VerifyFound();

        // Missing H.264 decode caps struct for H.264 decode profile
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07184");
        caps.pNext = &decode_caps;
        vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
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
        vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
        m_errorMonitor->VerifyFound();

        // Missing H.265 decode caps struct for H.265 decode profile
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07185");
        caps.pNext = &decode_caps;
        vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
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
        vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
        m_errorMonitor->VerifyFound();

        // Missing AV1 decode caps struct for AV1 decode profile
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-09257");
        caps.pNext = &decode_caps;
        vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
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
        vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
        m_errorMonitor->VerifyFound();

        // Missing H.264 encode caps struct for H.264 encode profile
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07187");
        caps.pNext = &encode_caps;
        vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
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
        vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
        m_errorMonitor->VerifyFound();

        // Missing H.265 encode caps struct for H.265 encode profile
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07188");
        caps.pNext = &encode_caps;
        vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
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
    vk.GetPhysicalDeviceVideoFormatPropertiesKHR(gpu(), &format_info, &format_count, nullptr);
    m_errorMonitor->VerifyFound();

    auto profile_list = vku::InitStruct<VkVideoProfileListInfoKHR>();
    format_info.pNext = &profile_list;

    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoFormatPropertiesKHR-pNext-06812");
    vk.GetPhysicalDeviceVideoFormatPropertiesKHR(gpu(), &format_info, &format_count, nullptr);
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
    vk.GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(gpu(), &quality_level_info, config.EncodeQualityLevelProps());
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
    vk.GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(gpu(), &quality_level_info, config.EncodeQualityLevelProps());
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
    vk.GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(gpu(), &quality_level_info, config.EncodeQualityLevelProps());
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
        vk.GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(gpu(), &quality_level_info, config.EncodeQualityLevelProps());
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigEncodeH265()) {
        VideoConfig config = GetConfigEncodeH265();

        profile = *config.Profile();
        profile.pNext = nullptr;

        // Missing codec-specific info for H.265 encode profile
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR-pVideoProfile-08259");
        m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07182");
        vk.GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(gpu(), &quality_level_info, config.EncodeQualityLevelProps());
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
        vk.GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(gpu(), &quality_level_info, &quality_level_props);
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigEncodeH265()) {
        VideoConfig config = GetConfigEncodeH265();
        quality_level_info.pVideoProfile = config.Profile();

        // Missing codec-specific output structure for H.265 encode profile
        m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR-pQualityLevelInfo-08258");
        vk.GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(gpu(), &quality_level_info, &quality_level_props);
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset());
    cb.EndVideoCoding(context.End());
    cb.end();

    context.Queue().Submit(cb);

    m_errorMonitor->SetDesiredError("VUID-vkDestroyVideoSessionKHR-videoSession-07192");
    context.vk.DestroyVideoSessionKHR(device(), context.Session(), nullptr);
    m_errorMonitor->VerifyFound();

    if (config.NeedsSessionParams()) {
        m_errorMonitor->SetDesiredError("VUID-vkDestroyVideoSessionParametersKHR-videoSessionParameters-07212");
        context.vk.DestroyVideoSessionParametersKHR(device(), context.SessionParams(), nullptr);
        m_errorMonitor->VerifyFound();
    }

    m_device->Wait();
}

TEST_F(NegativeVideo, CreateSessionVideoMaintenance1NotEnabled) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - cannot use inline queries without videoMaintenance1");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(m_device, config);

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.flags = VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoSessionCreateInfoKHR-flags-parameter");
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-flags-08371");
    context.vk.CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionProtectedMemoryNotEnabled) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - cannot enable protected content without protected memory");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(m_device, config);

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.flags = VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR;
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-protectedMemory-07189");
    context.vk.CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionProtectedContentUnsupported) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - cannot enable protected content if not supported");

    EnableProtectedMemory();
    RETURN_IF_SKIP(Init());
    if (!IsProtectedMemoryEnabled()) {
        GTEST_SKIP() << "Test requires protectedMemory support";
    }

    VideoConfig config = GetConfigWithoutProtectedContent(GetConfigs());
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with no protected content support";
    }

    VideoContext context(m_device, config);

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.flags = VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR;
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-protectedMemory-07189");
    context.vk.CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
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

    VideoContext context(m_device, config);

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();
    create_info.maxDpbSlots = config.Caps()->maxDpbSlots;
    create_info.maxActiveReferencePictures = config.Caps()->maxActiveReferencePictures;

    // maxDpbSlots too big
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxDpbSlots-04847");
    create_info.maxDpbSlots++;
    context.vk.CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    create_info.maxDpbSlots--;
    m_errorMonitor->VerifyFound();

    // maxActiveReferencePictures too big
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxActiveReferencePictures-04849");
    create_info.maxActiveReferencePictures++;
    context.vk.CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    create_info.maxActiveReferencePictures--;
    m_errorMonitor->VerifyFound();

    config = GetConfig(GetConfigsWithReferences(GetConfigs()));
    if (config) {
        // maxDpbSlots is 0, but maxActiveReferencePictures is not
        m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxDpbSlots-04850");
        create_info.maxDpbSlots = 0;
        create_info.maxActiveReferencePictures = 1;
        context.vk.CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
        m_errorMonitor->VerifyFound();

        // maxActiveReferencePictures is 0, but maxDpbSlots is not
        m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxDpbSlots-04850");
        create_info.maxDpbSlots = 1;
        create_info.maxActiveReferencePictures = 0;
        context.vk.CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
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

    VideoContext context(m_device, config);

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    // maxCodedExtent.width too small
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxCodedExtent-04851");
    create_info.maxCodedExtent = config.Caps()->minCodedExtent;
    --create_info.maxCodedExtent.width;
    context.vk.CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();

    // maxCodedExtent.height too small
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxCodedExtent-04851");
    create_info.maxCodedExtent = config.Caps()->minCodedExtent;
    --create_info.maxCodedExtent.height;
    context.vk.CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();

    // maxCodedExtent.width too big
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxCodedExtent-04851");
    create_info.maxCodedExtent = config.Caps()->maxCodedExtent;
    ++create_info.maxCodedExtent.width;
    context.vk.CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();

    // maxCodedExtent.height too big
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxCodedExtent-04851");
    create_info.maxCodedExtent = config.Caps()->maxCodedExtent;
    ++create_info.maxCodedExtent.height;
    context.vk.CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
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

    VideoContext context(m_device, config);

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-referencePictureFormat-04852");
    create_info.referencePictureFormat = VK_FORMAT_D16_UNORM;
    context.vk.CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
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

    VideoContext context(m_device, config);

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-referencePictureFormat-06814");
    create_info.referencePictureFormat = VK_FORMAT_D16_UNORM;
    context.vk.CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionInvalidDecodePictureFormat) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid decode picture format");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    VideoContext context(m_device, config);

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pictureFormat-04853");
    create_info.pictureFormat = VK_FORMAT_D16_UNORM;
    context.vk.CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionInvalidEncodePictureFormat) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid encode picture format");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pictureFormat-04854");
    create_info.pictureFormat = VK_FORMAT_D16_UNORM;
    context.vk.CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionInvalidStdHeaderVersion) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid Video Std header version");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(m_device, config);

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    VkExtensionProperties std_version = *config.StdVersion();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = &std_version;

    // Video Std header version not supported
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pStdHeaderVersion-07191");
    ++std_version.specVersion;
    context.vk.CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    --std_version.specVersion;
    m_errorMonitor->VerifyFound();

    // Video Std header name not supported
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pStdHeaderVersion-07190");
    strcpy(std_version.extensionName, "invalid_std_header_name");
    context.vk.CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionEncodeH264InvalidMaxLevel) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid H.264 maxLevelIdc for encode session");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    VideoContext context(m_device, config);

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
    context.vk.CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionEncodeH265InvalidMaxLevel) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid H.265 maxLevelIdc for encode session");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VideoContext context(m_device, config);

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
    context.vk.CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
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
    ASSERT_EQ(VK_SUCCESS, context.vk.GetVideoSessionMemoryRequirementsKHR(device(), context.Session(), &mem_req_count, nullptr));
    if (mem_req_count == 0) {
        GTEST_SKIP() << "Test can only run if video session needs memory bindings";
    }

    std::vector<VkVideoSessionMemoryRequirementsKHR> mem_reqs(mem_req_count, vku::InitStruct<VkVideoSessionMemoryRequirementsKHR>());
    ASSERT_EQ(VK_SUCCESS,
              context.vk.GetVideoSessionMemoryRequirementsKHR(device(), context.Session(), &mem_req_count, mem_reqs.data()));

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
        context.vk.BindVideoSessionMemoryKHR(device(), context.Session(), mem_req_count, bind_info.data());
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
        context.vk.BindVideoSessionMemoryKHR(device(), context.Session(), mem_req_count, bind_info.data());
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
        context.vk.BindVideoSessionMemoryKHR(device(), context.Session(), mem_req_count, bind_info.data());
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
        context.vk.BindVideoSessionMemoryKHR(device(), context.Session(), mem_req_count, bind_info.data());
        invalid = backup;
        m_errorMonitor->VerifyFound();
    }

    // Incorrect memorySize
    {
        m_errorMonitor->SetDesiredError("VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07200");
        auto& invalid = bind_info[mem_req_count / 2];
        auto backup = invalid;
        invalid.memorySize += 16;
        context.vk.BindVideoSessionMemoryKHR(device(), context.Session(), mem_req_count, bind_info.data());
        invalid = backup;
        m_errorMonitor->VerifyFound();
    }

    // Out-of-bounds memoryOffset
    {
        m_errorMonitor->SetDesiredError("VUID-VkBindVideoSessionMemoryInfoKHR-memoryOffset-07201");
        auto& invalid = bind_info[mem_req_count / 2];
        auto backup = invalid;
        invalid.memoryOffset = invalid.memorySize * 2;
        context.vk.BindVideoSessionMemoryKHR(device(), context.Session(), mem_req_count, bind_info.data());
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
        context.vk.BindVideoSessionMemoryKHR(device(), context.Session(), mem_req_count, bind_info.data());
        invalid = backup;
        m_errorMonitor->VerifyFound();
    }

    // Already bound
    {
        uint32_t first_bind_count = mem_req_count / 2;
        if (first_bind_count == 0) {
            first_bind_count = 1;
        }

        context.vk.BindVideoSessionMemoryKHR(device(), context.Session(), first_bind_count, bind_info.data());

        m_errorMonitor->SetDesiredError("VUID-vkBindVideoSessionMemoryKHR-videoSession-07195");
        context.vk.BindVideoSessionMemoryKHR(device(), context.Session(), 1, &bind_info[first_bind_count - 1]);
        m_errorMonitor->VerifyFound();
    }

    for (auto memory : session_memory) {
        vk::FreeMemory(device(), memory, nullptr);
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
    context1.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
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
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    // Expect to succeed to create parameters object with explicit encode quality level 0 against the same template
    quality_level_info.qualityLevel = 0;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    context.vk.DestroyVideoSessionParametersKHR(device(), params, nullptr);

    // Expect to succeed to create parameters object with highest encode quality level index but no template
    create_info.videoSessionParametersTemplate = VK_NULL_HANDLE;
    quality_level_info.qualityLevel = config.EncodeCaps()->maxQualityLevels - 1;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);

    // Expect to fail to create parameters object with encode quality level 0
    // with template using highest encode quality level index
    create_info.videoSessionParametersTemplate = params;
    quality_level_info.qualityLevel = 0;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-08310");
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    // Expect to fail the same with implicit encode quality level 0
    create_info.pNext = quality_level_info.pNext;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-08310");
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(device(), params, nullptr);
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
        context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
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
        context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
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
        context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
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
        context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
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
        context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
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
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
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
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07205");
    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 5;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 6;
    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    create_info.videoSessionParametersTemplate = params;
    sps_list[1].seq_parameter_set_id = 4;
    pps_list[1].seq_parameter_set_id = 4;
    pps_list[5].seq_parameter_set_id = 4;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07204");
    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 8;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07205");
    h264_ci.maxStdSPSCount = 4;
    h264_ci.maxStdPPSCount = 7;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    create_info.videoSessionParametersTemplate = params;
    h264_ci.pParametersAddInfo = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07204");
    h264_ci.maxStdSPSCount = 2;
    h264_ci.maxStdPPSCount = 8;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07205");
    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 5;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(device(), params, nullptr);
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
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07208");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 3;
    h265_ci.maxStdPPSCount = 9;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07209");
    h265_ci.maxStdVPSCount = 3;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 5;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 7;
    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    create_info.videoSessionParametersTemplate = params;
    vps_list[1].vps_video_parameter_set_id = 3;
    sps_list[1].sps_video_parameter_set_id = 3;
    pps_list[1].sps_video_parameter_set_id = 3;
    pps_list[5].sps_video_parameter_set_id = 3;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07207");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 10;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07208");
    h265_ci.maxStdVPSCount = 3;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 9;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07209");
    h265_ci.maxStdVPSCount = 3;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 8;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    create_info.videoSessionParametersTemplate = params;
    h265_ci.pParametersAddInfo = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07207");
    h265_ci.maxStdVPSCount = 1;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 7;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07208");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 3;
    h265_ci.maxStdPPSCount = 7;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07209");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 6;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(device(), params, nullptr);
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

    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);

    create_info.videoSessionParametersTemplate = params;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-09258");
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(device(), params, nullptr);
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
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04840");
    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 5;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 6;
    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    create_info.videoSessionParametersTemplate = params;
    sps_list[1].seq_parameter_set_id = 4;
    pps_list[1].seq_parameter_set_id = 4;
    pps_list[5].seq_parameter_set_id = 4;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04839");
    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 8;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04840");
    h264_ci.maxStdSPSCount = 4;
    h264_ci.maxStdPPSCount = 7;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    create_info.videoSessionParametersTemplate = params;
    h264_ci.pParametersAddInfo = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04839");
    h264_ci.maxStdSPSCount = 2;
    h264_ci.maxStdPPSCount = 8;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04840");
    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 5;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(device(), params, nullptr);
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
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04842");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 3;
    h265_ci.maxStdPPSCount = 9;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04843");
    h265_ci.maxStdVPSCount = 3;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 5;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 7;
    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    create_info.videoSessionParametersTemplate = params;
    vps_list[1].vps_video_parameter_set_id = 3;
    sps_list[1].sps_video_parameter_set_id = 3;
    pps_list[1].sps_video_parameter_set_id = 3;
    pps_list[5].sps_video_parameter_set_id = 3;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04841");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 10;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04842");
    h265_ci.maxStdVPSCount = 3;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 9;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04843");
    h265_ci.maxStdVPSCount = 3;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 8;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    create_info.videoSessionParametersTemplate = params;
    h265_ci.pParametersAddInfo = nullptr;

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04841");
    h265_ci.maxStdVPSCount = 1;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 7;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04842");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 3;
    h265_ci.maxStdPPSCount = 7;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04843");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 6;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(device(), params, nullptr);
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
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    // Then individual invalid ones one-by-one
    h265_ai.stdPPSCount = 1;

    h265_ai.pStdPPSs = &h265_pps_list[1];
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08319");
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    h265_ai.pStdPPSs = &h265_pps_list[4];
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08320");
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    h265_ai.pStdPPSs = &h265_pps_list[5];
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08319");
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08320");
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    // Successfully create object with an entry with the max limits
    h265_ai.pStdPPSs = &h265_pps_list[2];
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);

    // But then try to update with invalid ones
    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();
    update_info.pNext = &h265_ai;
    update_info.updateSequenceCount = 1;

    h265_ai.stdVPSCount = 0;
    h265_ai.stdSPSCount = 0;
    h265_ai.stdPPSCount = 1;

    h265_ai.pStdPPSs = &h265_pps_list[1];
    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-08321");
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    h265_ai.pStdPPSs = &h265_pps_list[4];
    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-08322");
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    h265_ai.pStdPPSs = &h265_pps_list[5];
    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-08321");
    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-08322");
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(device(), params, nullptr);
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
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    sps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH264SessionParametersAddInfoKHR-None-04826");
    pps_list[0].seq_parameter_set_id = 2;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    pps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    h264_ci.pParametersAddInfo = nullptr;
    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH264SessionParametersAddInfoKHR-None-04825");
    sps_list[0].seq_parameter_set_id = 3;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH264SessionParametersAddInfoKHR-None-04826");
    pps_list[0].seq_parameter_set_id = 2;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(device(), params, nullptr);
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
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    vps_list[0].vps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04834");
    sps_list[0].sps_video_parameter_set_id = 2;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    sps_list[0].sps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04835");
    pps_list[0].pps_seq_parameter_set_id = 2;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    pps_list[0].pps_seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    h265_ci.pParametersAddInfo = nullptr;
    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04833");
    vps_list[0].vps_video_parameter_set_id = 2;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    vps_list[0].vps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04834");
    sps_list[0].sps_video_parameter_set_id = 2;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list[0].sps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04835");
    pps_list[0].pps_seq_parameter_set_id = 2;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list[0].pps_seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(device(), params, nullptr);
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
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    sps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264SessionParametersAddInfoKHR-None-04838");
    pps_list[0].seq_parameter_set_id = 2;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    pps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    h264_ci.pParametersAddInfo = nullptr;
    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264SessionParametersAddInfoKHR-None-04837");
    sps_list[0].seq_parameter_set_id = 3;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264SessionParametersAddInfoKHR-None-04838");
    pps_list[0].seq_parameter_set_id = 2;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(device(), params, nullptr);
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
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    vps_list[0].vps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06439");
    sps_list[0].sps_video_parameter_set_id = 2;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    sps_list[0].sps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06440");
    pps_list[0].pps_seq_parameter_set_id = 2;
    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    pps_list[0].pps_seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    h265_ci.pParametersAddInfo = nullptr;
    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06438");
    vps_list[0].vps_video_parameter_set_id = 2;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    vps_list[0].vps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06439");
    sps_list[0].sps_video_parameter_set_id = 2;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list[0].sps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06440");
    pps_list[0].pps_seq_parameter_set_id = 2;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list[0].pps_seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(device(), params, nullptr);
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
    context.vk.UpdateVideoSessionParametersKHR(device(), context.SessionParams(), &update_info);
    m_errorMonitor->VerifyFound();

    update_info.updateSequenceCount = 1;
    ASSERT_EQ(VK_SUCCESS, context.vk.UpdateVideoSessionParametersKHR(device(), context.SessionParams(), &update_info));

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-pUpdateInfo-07215");
    update_info.updateSequenceCount = 1;
    context.vk.UpdateVideoSessionParametersKHR(device(), context.SessionParams(), &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-pUpdateInfo-07215");
    update_info.updateSequenceCount = 3;
    context.vk.UpdateVideoSessionParametersKHR(device(), context.SessionParams(), &update_info);
    m_errorMonitor->VerifyFound();

    update_info.updateSequenceCount = 2;
    ASSERT_EQ(VK_SUCCESS, context.vk.UpdateVideoSessionParametersKHR(device(), context.SessionParams(), &update_info));
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

    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    std::vector<StdVideoH264SequenceParameterSet> sps_list2{context.CreateH264SPS(4), context.CreateH264SPS(5)};

    std::vector<StdVideoH264PictureParameterSet> pps_list2{context.CreateH264PPS(1, 3), context.CreateH264PPS(3, 2),
                                                           context.CreateH264PPS(4, 1), context.CreateH264PPS(5, 2)};

    h264_ai.stdSPSCount = (uint32_t)sps_list2.size();
    h264_ai.pStdSPSs = sps_list2.data();
    h264_ai.stdPPSCount = (uint32_t)pps_list2.size();
    h264_ai.pStdPPSs = pps_list2.data();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07216");
    sps_list2[1].seq_parameter_set_id = 2;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list2[1].seq_parameter_set_id = 5;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07218");
    pps_list2[2].seq_parameter_set_id = 1;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list2[2].seq_parameter_set_id = 4;
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(device(), params, nullptr);
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

    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

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
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    vps_list2[0].vps_video_parameter_set_id = 3;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07222");
    sps_list2[0].sps_seq_parameter_set_id = 3;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list2[0].sps_seq_parameter_set_id = 2;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07224");
    pps_list2[1].pps_pic_parameter_set_id = 2;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list2[1].pps_pic_parameter_set_id = 4;
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(device(), params, nullptr);
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
    context.vk.UpdateVideoSessionParametersKHR(device(), context.SessionParams(), &update_info);
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

    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    std::vector<StdVideoH264SequenceParameterSet> sps_list2{context.CreateH264SPS(4), context.CreateH264SPS(5)};

    std::vector<StdVideoH264PictureParameterSet> pps_list2{context.CreateH264PPS(1, 3), context.CreateH264PPS(3, 2),
                                                           context.CreateH264PPS(4, 1), context.CreateH264PPS(5, 2)};

    h264_ai.stdSPSCount = (uint32_t)sps_list2.size();
    h264_ai.pStdSPSs = sps_list2.data();
    h264_ai.stdPPSCount = (uint32_t)pps_list2.size();
    h264_ai.pStdPPSs = pps_list2.data();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07226");
    sps_list2[1].seq_parameter_set_id = 2;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list2[1].seq_parameter_set_id = 5;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07227");
    pps_list2[2].seq_parameter_set_id = 1;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list2[2].seq_parameter_set_id = 4;
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(device(), params, nullptr);
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

    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

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
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    vps_list2[0].vps_video_parameter_set_id = 3;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07229");
    sps_list2[0].sps_seq_parameter_set_id = 3;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    sps_list2[0].sps_seq_parameter_set_id = 2;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07230");
    pps_list2[1].pps_pic_parameter_set_id = 2;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    pps_list2[1].pps_pic_parameter_set_id = 4;
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(device(), params, nullptr);
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

    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    std::vector<StdVideoH264SequenceParameterSet> sps_list2{context.CreateH264SPS(4), context.CreateH264SPS(5)};

    std::vector<StdVideoH264PictureParameterSet> pps_list2{context.CreateH264PPS(1, 3), context.CreateH264PPS(3, 2),
                                                           context.CreateH264PPS(4, 1), context.CreateH264PPS(5, 2)};

    h264_ai.pStdSPSs = sps_list2.data();
    h264_ai.pStdPPSs = pps_list2.data();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07217");
    h264_ai.stdSPSCount = 2;
    h264_ai.stdPPSCount = 2;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07219");
    h264_ai.stdSPSCount = 1;
    h264_ai.stdPPSCount = 4;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(device(), params, nullptr);
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

    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

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
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07223");
    h265_ai.stdVPSCount = 0;
    h265_ai.stdSPSCount = 2;
    h265_ai.stdPPSCount = 2;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07225");
    h265_ai.stdVPSCount = 0;
    h265_ai.stdSPSCount = 1;
    h265_ai.stdPPSCount = 3;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(device(), params, nullptr);
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

    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

    std::vector<StdVideoH264SequenceParameterSet> sps_list2{context.CreateH264SPS(4), context.CreateH264SPS(5)};

    std::vector<StdVideoH264PictureParameterSet> pps_list2{context.CreateH264PPS(1, 3), context.CreateH264PPS(3, 2),
                                                           context.CreateH264PPS(4, 1), context.CreateH264PPS(5, 2)};

    h264_ai.pStdSPSs = sps_list2.data();
    h264_ai.pStdPPSs = pps_list2.data();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06441");
    h264_ai.stdSPSCount = 2;
    h264_ai.stdPPSCount = 2;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06442");
    h264_ai.stdSPSCount = 1;
    h264_ai.stdPPSCount = 4;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(device(), params, nullptr);
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

    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params));

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
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06444");
    h265_ai.stdVPSCount = 0;
    h265_ai.stdSPSCount = 2;
    h265_ai.stdPPSCount = 2;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06445");
    h265_ai.stdVPSCount = 0;
    h265_ai.stdSPSCount = 1;
    h265_ai.stdPPSCount = 3;
    context.vk.UpdateVideoSessionParametersKHR(device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(device(), params, nullptr);
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
    encode_context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
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
        context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigEncodeH265()) {
        VideoContext context(m_device, GetConfigEncodeH265());

        auto get_info = vku::InitStruct<VkVideoEncodeSessionParametersGetInfoKHR>();
        get_info.videoSessionParameters = context.SessionParams();
        size_t data_size = 0;

        m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08265");
        context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
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
    context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request non-existent SPS
    h264_info = vku::InitStructHelper();
    h264_info.writeStdSPS = VK_TRUE;
    h264_info.stdSPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08263");
    context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request non-existent PPS
    h264_info = vku::InitStructHelper();
    h264_info.writeStdPPS = VK_TRUE;
    h264_info.stdPPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08264");
    context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    h264_info.stdSPSId = 1;
    h264_info.stdPPSId = 0;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08264");
    context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request both non-existent SPS and PPS
    h264_info = vku::InitStructHelper();
    h264_info.writeStdSPS = VK_TRUE;
    h264_info.writeStdPPS = VK_TRUE;
    h264_info.stdSPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08263");
    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08264");
    context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
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
    context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request non-existent VPS
    h265_info = vku::InitStructHelper();
    h265_info.writeStdVPS = VK_TRUE;
    h265_info.stdVPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08266");
    context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request non-existent SPS
    h265_info = vku::InitStructHelper();
    h265_info.writeStdSPS = VK_TRUE;
    h265_info.stdSPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08267");
    context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    h265_info.stdVPSId = 1;
    h265_info.stdSPSId = 0;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08267");
    context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request non-existent PPS
    h265_info = vku::InitStructHelper();
    h265_info.writeStdPPS = VK_TRUE;
    h265_info.stdPPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08268");
    context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    h265_info.stdSPSId = 1;
    h265_info.stdPPSId = 0;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08268");
    context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    h265_info.stdVPSId = 1;
    h265_info.stdSPSId = 0;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08268");
    context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
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
    context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
    m_errorMonitor->VerifyFound();

    // Trying to request only non-existent SPS and PPS
    h265_info.stdVPSId = 0;
    h265_info.stdSPSId = 1;

    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08267");
    m_errorMonitor->SetDesiredError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08268");
    context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);
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

    cb.begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07231");
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->VerifyFound();

    cb.end();
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

    cb.begin();
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 0, 0);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-None-07232");
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->VerifyFound();
    vk::CmdEndQuery(cb.handle(), context.StatusQueryPool(), 0);
    cb.end();
}

TEST_F(NegativeVideo, BeginCodingProtectedNoFaultSession) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - protectedNoFault tests for video session");

    EnableProtectedMemory();
    RETURN_IF_SKIP(Init());
    if (!IsProtectedMemoryEnabled() || IsProtectedNoFaultSupported()) {
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

    unprotected_cb.begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07233");
    unprotected_cb.BeginVideoCoding(protected_context.Begin());
    m_errorMonitor->VerifyFound();
    unprotected_cb.end();

    protected_cb.begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07234");
    protected_cb.BeginVideoCoding(unprotected_context.Begin());
    m_errorMonitor->VerifyFound();
    protected_cb.end();
}

TEST_F(NegativeVideo, BeginCodingProtectedNoFaultSlots) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - protectedNoFault tests for reference slots");

    EnableProtectedMemory();
    RETURN_IF_SKIP(Init());
    if (!IsProtectedMemoryEnabled() || IsProtectedNoFaultSupported()) {
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

    unprotected_cb.begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07235");
    unprotected_cb.BeginVideoCoding(unprotected_context.Begin().AddResource(-1, 0));
    m_errorMonitor->VerifyFound();
    unprotected_cb.end();

    protected_cb.begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07236");
    protected_cb.BeginVideoCoding(protected_context.Begin().AddResource(-1, 0));
    m_errorMonitor->VerifyFound();
    protected_cb.end();
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
    context.vk.GetVideoSessionMemoryRequirementsKHR(device(), context.Session(), &mem_req_count, nullptr);
    if (mem_req_count == 0) {
        GTEST_SKIP() << "Test requires video session to need memory bindings";
    }

    std::vector<VkVideoSessionMemoryRequirementsKHR> mem_reqs(mem_req_count,
                                                              vku::InitStruct<VkVideoSessionMemoryRequirementsKHR>());
    context.vk.GetVideoSessionMemoryRequirementsKHR(device(), context.Session(), &mem_req_count, mem_reqs.data());

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

        ASSERT_EQ(VK_SUCCESS, context.vk.BindVideoSessionMemoryKHR(device(), context.Session(), 1, &bind_info));
    }

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSession-07237");
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->VerifyFound();

    cb.end();

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

    cb.begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSessionParameters-04857");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();
    cb.end();
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

    cb.begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSession-07247");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();
    cb.end();
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

    cb.begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSession-07248");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();
    cb.end();
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

    cb.begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSession-09261");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();
    cb.end();
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

    cb.begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSession-07249");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();
    cb.end();
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

    cb.begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSession-07250");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();
    cb.end();
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

    cb.begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07240");
    cb.BeginVideoCoding(context1.Begin().AddResource(-1, context2.Dpb()->Picture(0)));
    m_errorMonitor->VerifyFound();
    cb.end();
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

    cb.begin();

    // Invalid baseArrayLayer in VkVideoBeginCodingInfoKHR::pReferenceSlots
    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);
    res.baseArrayLayer = 5;

    m_errorMonitor->SetDesiredError("VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.EndVideoCoding(context.End());
    cb.end();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
    cb.ControlVideoCoding(context.Control().Reset());
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, 0, 1));
    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().InvalidateSlot(0));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.EndVideoCoding(context.End());
    cb.end();

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

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
    cb.ControlVideoCoding(context.Control().Reset());
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, 0, 1));
    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    cb.EndVideoCoding(context.End());

    cb.BeginVideoCoding(context.Begin().AddResource(0, 1));
    cb.EndVideoCoding(context.End());
    cb.end();

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

    cb.begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-slotIndex-04856");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(3, 1).AddResource(-1, 2));
    m_errorMonitor->VerifyFound();

    cb.end();
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

    cb.begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07238");
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 0));
    m_errorMonitor->VerifyFound();

    cb.end();
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

    cb.begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07241");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, dpb.Picture(0)));
    m_errorMonitor->VerifyFound();

    cb.end();
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

    cb.begin();

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

    cb.end();
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

    cb.begin();

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

    cb.end();
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

    cb.begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-flags-07244");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, context.Dpb()->Picture(0)).AddResource(-1, separate_dpb.Picture(1)));
    m_errorMonitor->VerifyFound();

    cb.end();
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

    cb.begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-slotIndex-07245");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    cb.end();
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

    cb.begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-slotIndex-07246");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    cb.end();
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

    cb.begin();

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

    cb.end();
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

    cb.begin();

    // Test with VkVideoEncodeH264GopRemainingFrameInfoKHR missing from pNext chain
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

    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 0, 0);
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndVideoCodingKHR-None-07251");
    cb.EndVideoCoding(context.End());
    m_errorMonitor->VerifyFound();
    vk::CmdEndQuery(cb.handle(), context.StatusQueryPool(), 0);
    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(control_info);
    cb.EndVideoCoding(context.End());
    cb.end();

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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeQualityLevelInfoKHR-qualityLevel-08311");
    cb.ControlVideoCoding(control_info);
    m_errorMonitor->VerifyFound();
    cb.EndVideoCoding(context.End());
    cb.end();

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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    control_info.flags = VK_VIDEO_CODING_CONTROL_ENCODE_QUALITY_LEVEL_BIT_KHR;
    m_errorMonitor->SetDesiredError("VUID-vkCmdControlVideoCodingKHR-pCodingControlInfo-08243");
    cb.ControlVideoCoding(control_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->SetDesiredError("VUID-VkVideoCodingControlInfoKHR-flags-08349");
    cb.ControlVideoCoding(control_info);
    m_errorMonitor->VerifyFound();
    cb.EndVideoCoding(context.End());
    cb.end();
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

    context.vk.CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params_quality_level_1);

    // Submitting command buffer using mismatching parameters object encode quality level is fine as long as
    // there is no encode operation recorded
    cb.begin();
    cb.BeginVideoCoding(context.Begin().SetSessionParams(params_quality_level_1));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().Submit(cb);
    context.Queue().Wait();

    // Submitting command buffer with matching parameters object encode quality level is fine even if the
    // quality is modified afterwards.
    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    cb.EncodeVideo(context.EncodeFrame());
    cb.ControlVideoCoding(context.Control().EncodeQualityLevel(1));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().Submit(cb);
    context.Queue().Wait();

    // Submitting command buffer with matching parameters object encode quality level is fine, even if state
    // is not set here
    cb.begin();
    cb.BeginVideoCoding(context.Begin().SetSessionParams(params_quality_level_1));
    cb.EncodeVideo(context.EncodeFrame());
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().Submit(cb);
    context.Queue().Wait();

    // Submitting command buffer with mismatching parameters object is fine if the quality level is modified
    // to the right one before the encode operation
    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().EncodeQualityLevel(0));
    cb.EncodeVideo(context.EncodeFrame());
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().Submit(cb);
    context.Queue().Wait();

    // Submitting an encode operation with mismatching effective and parameters encode quality levels should fail
    cb.begin();
    cb.BeginVideoCoding(context.Begin().SetSessionParams(params_quality_level_1));
    cb.EncodeVideo(context.EncodeFrame());
    cb.EndVideoCoding(context.End());
    cb.end();
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-None-08318");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();

    // Same goes when encode quality level state is changed before/after
    // First test cases where command buffer recording time validation is possible
    cb.begin();
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
    cb.end();

    // Then test a case where only submit time validation is possible
    cb.begin();
    cb.BeginVideoCoding(context.Begin().SetSessionParams(params_quality_level_1));
    cb.EncodeVideo(context.EncodeFrame());
    cb.EndVideoCoding(context.End());
    cb.end();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-None-08318");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(device(), params_quality_level_1, nullptr);
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    control_info.flags = VK_VIDEO_CODING_CONTROL_ENCODE_RATE_CONTROL_BIT_KHR;
    m_errorMonitor->SetDesiredError("VUID-vkCmdControlVideoCodingKHR-pCodingControlInfo-08243");
    cb.ControlVideoCoding(control_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08244");
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08244");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-layerCount-08245");
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-layerCount-08245");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

            cb.begin();

            m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08248");
            cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
            m_errorMonitor->VerifyFound();

            cb.BeginVideoCoding(context.Begin());

            m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08248");
            cb.ControlVideoCoding(context.Control().RateControl(rc_info));
            m_errorMonitor->VerifyFound();

            cb.EndVideoCoding(context.End());
            cb.end();
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

            cb.begin();

            m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08275");
            cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
            m_errorMonitor->VerifyFound();

            cb.BeginVideoCoding(context.Begin());

            m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08275");
            cb.ControlVideoCoding(context.Control().RateControl(rc_info));
            m_errorMonitor->VerifyFound();

            cb.EndVideoCoding(context.End());
            cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    // averageBitrate must be less than or equal to maxBitrate
    rc_info.Layer(layer_index)->maxBitrate = rc_info.Layer(layer_index)->averageBitrate - 1;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08278");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    // virtualBufferSizeInMs must be greater than 0
    rc_info->virtualBufferSizeInMs = 0;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-layerCount-08357");
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-layerCount-08358");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    // initialVirtualBufferSizeInMs must be less than virtualBufferSizeInMs
    rc_info->virtualBufferSizeInMs = 1000;
    rc_info->initialVirtualBufferSizeInMs = 1000;
    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeRateControlInfoKHR-layerCount-08358");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();

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
    cb.end();
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

    cb.begin();

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
    cb.end();
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

    cb.begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264RateControlInfoKHR-flags-08280");
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    m_errorMonitor->VerifyFound();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264RateControlInfoKHR-flags-08280");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265RateControlInfoKHR-flags-08291");
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    m_errorMonitor->VerifyFound();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265RateControlInfoKHR-flags-08291");
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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
    TEST_DESCRIPTION("vkCmdBegin/ControlVideoCodingKHR - H.5 rate control layer QP out of bounds");

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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->SetDesiredError("VUID-VkVideoCodingControlInfoKHR-flags-07018");
    cb.ControlVideoCoding(control_info);
    m_errorMonitor->VerifyFound();
    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset().RateControl(rc_info));
    cb.EndVideoCoding(context.End());
    cb.end();

    context.Queue().Submit(cb);
    m_device->Wait();

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    cb.EndVideoCoding(context.End());
    cb.end();

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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset().RateControl(rc_info));
    cb.EndVideoCoding(context.End());
    cb.end();

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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset().RateControl(rc_info));
    cb.EndVideoCoding(context.End());
    cb.end();

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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset().RateControl(rc_info));
    cb.EndVideoCoding(context.End());
    cb.end();

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

    cb.begin();
    cb.BeginVideoCoding(encode_context.Begin());

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-commandBuffer-cmdpool");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-None-08249");
    cb.DecodeVideo(decode_context.DecodeFrame());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(encode_context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(decode_context.Begin());

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-commandBuffer-cmdpool");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-None-08250");
    cb.EncodeVideo(encode_context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(decode_context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    cb.DecodeVideo(context.DecodeFrame());
    cb.EndVideoCoding(context.End());
    cb.end();

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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    cb.EncodeVideo(context.EncodeFrame());
    cb.EndVideoCoding(context.End());
    cb.end();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-None-07012");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();
    m_device->Wait();
}

TEST_F(NegativeVideo, DecodeProtectedNoFaultBitstreamBuffer) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - protectedNoFault tests for bitstream buffer");

    EnableProtectedMemory();
    RETURN_IF_SKIP(Init());
    if (!IsProtectedMemoryEnabled() || IsProtectedNoFaultSupported()) {
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

    unprotected_cb.begin();
    unprotected_cb.BeginVideoCoding(unprotected_context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-commandBuffer-07136");
    unprotected_cb.DecodeVideo(unprotected_context.DecodeFrame());
    m_errorMonitor->VerifyFound();

    unprotected_cb.EndVideoCoding(unprotected_context.End());
    unprotected_cb.end();

    protected_cb.begin();
    protected_cb.BeginVideoCoding(protected_context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-commandBuffer-07137");
    protected_cb.DecodeVideo(protected_context.DecodeFrame());
    m_errorMonitor->VerifyFound();

    protected_cb.EndVideoCoding(protected_context.End());
    protected_cb.end();
}

TEST_F(NegativeVideo, EncodeProtectedNoFaultBitstreamBuffer) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - protectedNoFault tests for bitstream buffer");

    EnableProtectedMemory();
    RETURN_IF_SKIP(Init());
    if (!IsProtectedMemoryEnabled() || IsProtectedNoFaultSupported()) {
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

    unprotected_cb.begin();
    unprotected_cb.BeginVideoCoding(unprotected_context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-commandBuffer-08202");
    unprotected_cb.EncodeVideo(unprotected_context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    unprotected_cb.EndVideoCoding(unprotected_context.End());
    unprotected_cb.end();

    protected_cb.begin();
    protected_cb.BeginVideoCoding(protected_context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-commandBuffer-08203");
    protected_cb.EncodeVideo(protected_context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    protected_cb.EndVideoCoding(protected_context.End());
    protected_cb.end();
}

TEST_F(NegativeVideo, DecodeProtectedNoFaultDecodeOutput) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - protectedNoFault tests for decode output");

    EnableProtectedMemory();
    RETURN_IF_SKIP(Init());
    if (!IsProtectedMemoryEnabled() || IsProtectedNoFaultSupported()) {
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

    unprotected_cb.begin();
    unprotected_cb.BeginVideoCoding(unprotected_context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-commandBuffer-07147");
    unprotected_cb.DecodeVideo(unprotected_context.DecodeFrame());
    m_errorMonitor->VerifyFound();

    unprotected_cb.EndVideoCoding(unprotected_context.End());
    unprotected_cb.end();

    protected_cb.begin();
    protected_cb.BeginVideoCoding(protected_context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-commandBuffer-07148");
    protected_cb.DecodeVideo(protected_context.DecodeFrame());
    m_errorMonitor->VerifyFound();

    protected_cb.EndVideoCoding(protected_context.End());
    protected_cb.end();
}

TEST_F(NegativeVideo, EncodeProtectedNoFaultEncodeInput) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - protectedNoFault tests for encode input");

    EnableProtectedMemory();
    RETURN_IF_SKIP(Init());
    if (!IsProtectedMemoryEnabled() || IsProtectedNoFaultSupported()) {
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

    unprotected_cb.begin();
    unprotected_cb.BeginVideoCoding(unprotected_context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-commandBuffer-08211");
    unprotected_cb.EncodeVideo(unprotected_context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    unprotected_cb.EndVideoCoding(unprotected_context.End());
    unprotected_cb.end();

    protected_cb.begin();
    protected_cb.BeginVideoCoding(protected_context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-commandBuffer-08212");
    protected_cb.EncodeVideo(protected_context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    protected_cb.EndVideoCoding(protected_context.End());
    protected_cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 0, 0);
    cb.DecodeVideo(context.DecodeFrame());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-opCount-07134");
    cb.DecodeVideo(context.DecodeFrame());
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(cb.handle(), context.StatusQueryPool(), 0);
    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 0, 0);
    cb.EncodeVideo(context.EncodeFrame());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-opCount-07174");
    cb.EncodeVideo(context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(cb.handle(), context.StatusQueryPool(), 0);
    cb.EndVideoCoding(context.End());
    cb.end();

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    vk::CmdBeginQuery(cb.handle(), context.EncodeFeedbackQueryPool(), 0, 0);
    cb.EncodeVideo(context.EncodeFrame());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-opCount-07174");
    cb.EncodeVideo(context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(cb.handle(), context.EncodeFeedbackQueryPool(), 0);
    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07135");
    cb.DecodeVideo(context1.DecodeFrame().SetBitstream(context2.Bitstream()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08201");
    cb.EncodeVideo(context1.EncodeFrame().SetBitstream(context2.Bitstream()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeInfoKHR-srcBuffer-07165");
    cb.DecodeVideo(context.DecodeFrame().SetBitstreamBuffer(buffer.handle(), 0, create_info.size));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeInfoKHR-dstBuffer-08236");
    cb.EncodeVideo(context.EncodeFrame().SetBitstreamBuffer(buffer.handle(), 0, create_info.size));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    VideoDecodeInfo decode_info = context.DecodeFrame();
    ++decode_info->srcBufferOffset;
    decode_info->srcBufferRange -= config.Caps()->minBitstreamBufferSizeAlignment;

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07138");
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();
    ++encode_info->dstBufferOffset;
    encode_info->dstBufferRange -= config.Caps()->minBitstreamBufferSizeAlignment;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08204");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    VideoDecodeInfo decode_info = context.DecodeFrame();
    --decode_info->srcBufferRange;

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07139");
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();
    --encode_info->dstBufferRange;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08205");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07146");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07140");
    cb.DecodeVideo(context.DecodeFrame(0).SetDecodeOutput(context.Dpb()->Picture(0)));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07141");
    cb.DecodeVideo(context.DecodeFrame(0).SetDecodeOutput(context.DecodeOutput()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeInfoKHR-slotIndex-07171");
    cb.DecodeVideo(context.DecodeFrame(0).AddReferenceFrame(-1, 0));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-slotIndex-07256");
    cb.DecodeVideo(context.DecodeFrame(0).AddReferenceFrame(2, 0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeInfoKHR-slotIndex-08241");
    cb.EncodeVideo(context.EncodeFrame(0).AddReferenceFrame(-1, 0));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pPictureResource-08219");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-slotIndex-08217");
    cb.EncodeVideo(context.EncodeFrame(0).AddReferenceFrame(2, 0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    VideoDecodeInfo decode_info = context.DecodeFrame(0);
    decode_info->pSetupReferenceSlot = NULL;

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-08376");
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    VideoDecodeInfo decode_info = context.DecodeFrame(0);
    auto setup_slot = *decode_info->pSetupReferenceSlot;
    decode_info->pSetupReferenceSlot = &setup_slot;

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeInfoKHR-pSetupReferenceSlot-07169");
    setup_slot.pPictureResource = nullptr;
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    VideoEncodeInfo encode_info = context.EncodeFrame(0);
    encode_info->pSetupReferenceSlot = NULL;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08377");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    VideoEncodeInfo encode_info = context.EncodeFrame(0);
    auto setup_slot = *encode_info->pSetupReferenceSlot;
    encode_info->pSetupReferenceSlot = &setup_slot;

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeInfoKHR-pSetupReferenceSlot-08240");
    setup_slot.pPictureResource = nullptr;
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    m_errorMonitor->SetDesiredError("VUID-VkVideoDecodeInfoKHR-pPictureResource-07172");
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0, nullptr));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeInfoKHR-pPictureResource-08242");
    cb.EncodeVideo(context.EncodeFrame(1).AddReferenceFrame(0, nullptr));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07142");
    cb.DecodeVideo(context1.DecodeFrame().SetDecodeOutput(context2.DecodeOutput()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08206");
    cb.EncodeVideo(context1.EncodeFrame().SetEncodeInput(context2.EncodeInput()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07143");
    cb.DecodeVideo(context.DecodeFrame().SetDecodeOutput(output.Picture()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08207");
    cb.EncodeVideo(context.EncodeFrame().SetEncodeInput(output.Picture()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07146");
    cb.DecodeVideo(context.DecodeFrame().SetDecodeOutput(dst_res));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08210");
    cb.EncodeVideo(context.EncodeFrame().SetEncodeInput(src_res));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07149");
    cb.DecodeVideo(context.DecodeFrame(0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08215");
    cb.EncodeVideo(context.EncodeFrame(0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(-1, 2));

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-activeReferencePictureCount-07150");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0).AddReferenceFrame(1));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(-1, 2));

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-activeReferencePictureCount-08216");
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(0).AddReferenceFrame(1));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(-1, 2));

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-activeReferencePictureCount-07150");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0).AddReferenceBothFields(1));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-activeReferencePictureCount-07150");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceBothFields(1).AddReferenceFrame(0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07264");
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceTopField(0, 0).AddReferenceBottomField(0, 0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(0, 1).AddResource(-1, 2));

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-dpbFrameUseCount-07176");
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0, 0).AddReferenceFrame(0, 1));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(0, 1).AddResource(-1, 2));

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-dpbFrameUseCount-08221");
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(0, 0).AddReferenceFrame(0, 1));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
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
    cb.end();
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
    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    cb.DecodeVideo(context.DecodeFrame(0));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().Submit(cb);
    m_device->Wait();

    // Try to include the DPB slot expecting reference picture association at begin-time
    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.EndVideoCoding(context.End());
    cb.end();
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
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceFrame(0));
        cb.DecodeVideo(context.DecodeTopField(0));
        cb.EndVideoCoding(context.End());
        cb.end();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to include the DPB slot expecting reference picture association at begin-time
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.EndVideoCoding(context.End());
        cb.end();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup frame in DPB slot then use it as non-setup reconstructed picture for a bottom field
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceFrame(0));
        cb.DecodeVideo(context.DecodeTopField(0));
        cb.EndVideoCoding(context.End());
        cb.end();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to include the DPB slot expecting reference picture association at begin-time
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.EndVideoCoding(context.End());
        cb.end();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup top field in DPB slot then use it as non-setup reconstructed picture for a frame
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceTopField(0));
        cb.DecodeVideo(context.DecodeFrame(0));
        cb.EndVideoCoding(context.End());
        cb.end();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to include the DPB slot expecting reference picture association at begin-time
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.EndVideoCoding(context.End());
        cb.end();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup top field in DPB slot then use it as non-setup reconstructed picture for a top field
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceTopField(0));
        cb.DecodeVideo(context.DecodeTopField(0));
        cb.EndVideoCoding(context.End());
        cb.end();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to include the DPB slot expecting reference picture association at begin-time
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.EndVideoCoding(context.End());
        cb.end();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup bottom field in DPB slot then use it as non-setup reconstructed picture for a frame
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceBottomField(0));
        cb.DecodeVideo(context.DecodeFrame(0));
        cb.EndVideoCoding(context.End());
        cb.end();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to include the DPB slot expecting reference picture association at begin-time
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.EndVideoCoding(context.End());
        cb.end();
        m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup bottom field in DPB slot then use it as non-setup reconstructed picture for a bottom field
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceTopField(0));
        cb.DecodeVideo(context.DecodeTopField(0));
        cb.EndVideoCoding(context.End());
        cb.end();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to include the DPB slot expecting reference picture association at begin-time
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.EndVideoCoding(context.End());
        cb.end();
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
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceFrame(0));
        cb.EndVideoCoding(context.End());
        cb.end();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to reference DPB slot as top field
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceTopField(0));
        cb.EndVideoCoding(context.End());
        cb.end();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07267");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();

        // Try to reference DPB slot as bottom field
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceBottomField(0));
        cb.EndVideoCoding(context.End());
        cb.end();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07268");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup top field in DPB slot
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceTopField(0));
        cb.EndVideoCoding(context.End());
        cb.end();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to reference DPB slot as frame
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0));
        cb.EndVideoCoding(context.End());
        cb.end();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07266");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();

        // Try to reference DPB slot as bottom field
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceBottomField(0));
        cb.EndVideoCoding(context.End());
        cb.end();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07268");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup bottom field in DPB slot
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceBottomField(0));
        cb.EndVideoCoding(context.End());
        cb.end();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to reference DPB slot as frame
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0));
        cb.EndVideoCoding(context.End());
        cb.end();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07266");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();

        // Try to reference DPB slot as top field
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceTopField(0));
        cb.EndVideoCoding(context.End());
        cb.end();
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
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceTopField(0));
        cb.DecodeVideo(context.DecodeReferenceBottomField(0));
        cb.DecodeVideo(context.DecodeTopField(0));
        cb.EndVideoCoding(context.End());
        cb.end();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to reference DPB slot as top field
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceTopField(0));
        cb.EndVideoCoding(context.End());
        cb.end();
        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07267");
        context.Queue().Submit(cb);
        m_errorMonitor->VerifyFound();
        m_device->Wait();
    }

    {
        // Setup top and bottom field in DPB slot then use it as non-setup reconstructed picture for a bottom field
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
        cb.ControlVideoCoding(context.Control().Reset());
        cb.DecodeVideo(context.DecodeReferenceTopField(0));
        cb.DecodeVideo(context.DecodeReferenceBottomField(0));
        cb.DecodeVideo(context.DecodeBottomField(0));
        cb.EndVideoCoding(context.End());
        cb.end();
        context.Queue().Submit(cb);
        m_device->Wait();

        // Try to reference DPB slot as bottom field
        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));
        cb.DecodeVideo(context.DecodeFrame(1).AddReferenceBottomField(0));
        cb.EndVideoCoding(context.End());
        cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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

    // Missing reference slot for DPB slot index refered to by referenceNameSlotIndices
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
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07140");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07146");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-09249");
    cb.DecodeVideo(context.DecodeFrame(0).ApplyFilmGrain().SetDecodeOutput(context.Dpb()->Picture(0)));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(NegativeVideo, DecodeInlineQueryOpCount) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - inline query count does not match video codec operation count");

    EnableVideoMaintenance1();
    RETURN_IF_SKIP(Init());
    if (!IsVideoMaintenance1Enabled()) {
        GTEST_SKIP() << "Test requires videoMaintenance1";
    }

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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-08365");
    cb.DecodeVideo(context.DecodeFrame().InlineQuery(context.StatusQueryPool(), 0, 2));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(NegativeVideo, DecodeInlineQueryOutOfBounds) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - inline query firstQuery/queryCount out of bounds");

    EnableVideoMaintenance1();
    RETURN_IF_SKIP(Init());
    if (!IsVideoMaintenance1Enabled()) {
        GTEST_SKIP() << "Test requires videoMaintenance1";
    }

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

    cb.begin();
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
    cb.end();
}

TEST_F(NegativeVideo, DecodeInlineQueryUnavailable) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - inline queries must be unavailable");

    EnableVideoMaintenance1();
    RETURN_IF_SKIP(Init());
    if (!IsVideoMaintenance1Enabled()) {
        GTEST_SKIP() << "Test requires videoMaintenance1";
    }

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
    ;

    cb.begin(&begin_info);
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset());
    cb.DecodeVideo(context.DecodeFrame().InlineQuery(context.StatusQueryPool()));
    cb.EndVideoCoding(context.End());
    cb.end();

    m_command_buffer.begin(&begin_info);
    vk::CmdResetQueryPool(m_command_buffer.handle(), context.StatusQueryPool(), 0, 1);
    m_command_buffer.end();

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

    EnableVideoMaintenance1();
    RETURN_IF_SKIP(Init());
    if (!IsVideoMaintenance1Enabled()) {
        GTEST_SKIP() << "Test requires videoMaintenance1";
    }

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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-queryPool-08368");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-queryType-08367");
    cb.DecodeVideo(context.DecodeFrame().InlineQuery(query_pool.handle()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(NegativeVideo, DecodeInlineQueryProfileMismatch) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - inline query must have been created with the same profile");

    EnableVideoMaintenance1();
    RETURN_IF_SKIP(Init());
    if (!IsVideoMaintenance1Enabled()) {
        GTEST_SKIP() << "Test requires videoMaintenance1";
    }

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

    cb.begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-queryPool-08368");
    cb.DecodeVideo(context1.DecodeFrame().InlineQuery(context2.StatusQueryPool()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.end();
}

TEST_F(NegativeVideo, DecodeInlineQueryIncompatibleQueueFamily) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - result status queries require queue family support");

    EnableVideoMaintenance1();
    RETURN_IF_SKIP(Init());
    if (!IsVideoMaintenance1Enabled()) {
        GTEST_SKIP() << "Test requires videoMaintenance1";
    }

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

    cb.begin();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-cmdpool");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07231");
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-commandBuffer-cmdpool");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-queryType-08369");
    cb.DecodeVideo(context.DecodeFrame().InlineQuery(context.StatusQueryPool()));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEndVideoCodingKHR-commandBuffer-cmdpool");
    cb.EndVideoCoding(context.End());

    cb.end();
}

TEST_F(NegativeVideo, EncodeInlineQueryOpCount) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - inline query count does not match video codec operation count");

    EnableVideoMaintenance1();
    RETURN_IF_SKIP(Init());
    if (!IsVideoMaintenance1Enabled()) {
        GTEST_SKIP() << "Test requires videoMaintenance1";
    }

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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-08360");
    cb.EncodeVideo(context.EncodeFrame().InlineQuery(context.EncodeFeedbackQueryPool(), 0, 2));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(NegativeVideo, EncodeInlineQueryOutOfBounds) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - inline query firstQuery/queryCount out of bounds");

    EnableVideoMaintenance1();
    RETURN_IF_SKIP(Init());
    if (!IsVideoMaintenance1Enabled()) {
        GTEST_SKIP() << "Test requires videoMaintenance1";
    }

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

    cb.begin();
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
    cb.end();
}

TEST_F(NegativeVideo, EncodeInlineQueryUnavailable) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - inline queries must be unavailable");

    EnableVideoMaintenance1();
    RETURN_IF_SKIP(Init());
    if (!IsVideoMaintenance1Enabled()) {
        GTEST_SKIP() << "Test requires videoMaintenance1";
    }

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
    ;

    cb.begin(&begin_info);
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset());
    cb.EncodeVideo(context.EncodeFrame().InlineQuery(context.EncodeFeedbackQueryPool()));
    cb.EndVideoCoding(context.End());
    cb.end();

    m_command_buffer.begin(&begin_info);
    vk::CmdResetQueryPool(m_command_buffer.handle(), context.EncodeFeedbackQueryPool(), 0, 1);
    m_command_buffer.end();

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

    EnableVideoMaintenance1();
    RETURN_IF_SKIP(Init());
    if (!IsVideoMaintenance1Enabled()) {
        GTEST_SKIP() << "Test requires videoMaintenance1";
    }

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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-queryPool-08363");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-queryType-08362");
    cb.EncodeVideo(context.EncodeFrame().InlineQuery(query_pool.handle()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(NegativeVideo, EncodeInlineQueryProfileMismatch) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - inline query must have been created with the same profile");

    EnableVideoMaintenance1();
    RETURN_IF_SKIP(Init());
    if (!IsVideoMaintenance1Enabled()) {
        GTEST_SKIP() << "Test requires videoMaintenance1";
    }

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

    cb.begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-queryPool-08363");
    cb.EncodeVideo(context1.EncodeFrame().InlineQuery(context2.EncodeFeedbackQueryPool()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.end();
}

TEST_F(NegativeVideo, EncodeInlineQueryIncompatibleQueueFamily) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - result status queries require queue family support");

    EnableVideoMaintenance1();
    RETURN_IF_SKIP(Init());
    if (!IsVideoMaintenance1Enabled()) {
        GTEST_SKIP() << "Test requires videoMaintenance1";
    }

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

    cb.begin();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-cmdpool");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07231");
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-commandBuffer-cmdpool");
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-queryType-08364");
    cb.EncodeVideo(context.EncodeFrame().InlineQuery(context.StatusQueryPool()));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEndVideoCodingKHR-commandBuffer-cmdpool");
    cb.EndVideoCoding(context.End());

    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame();
    encode_info.CodecInfo().encode_h264.picture_info.generatePrefixNalu = VK_TRUE;

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264PictureInfoKHR-flags-08304");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08323");
    cb.EncodeVideo(context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddReferenceFrame(0);

        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264PictureInfoKHR-flags-08314");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.end();
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

        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(-1, 2));

        VideoEncodeInfo encode_info = context.EncodeFrame(2).AddReferenceFrame(0).AddBackReferenceFrame(1);

        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH264PictureInfoKHR-flags-08314");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.end();
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

        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddReferenceFrame(0);

        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08316");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.end();
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

        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(-1, 2));

        VideoEncodeInfo encode_info = context.EncodeFrame(2).AddReferenceFrame(0).AddBackReferenceFrame(1);
        encode_info.CodecInfo().encode_h265.std_picture_info.pic_type = STD_VIDEO_H265_PICTURE_TYPE_B;
        encode_info.CodecInfo().encode_h265.std_slice_segment_header.slice_type = STD_VIDEO_H265_SLICE_TYPE_B;

        m_errorMonitor->SetDesiredError("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08316");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.end();
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

        cb.begin();
        cb.BeginVideoCoding(context.Begin());

        VideoEncodeInfo encode_info = context.EncodeFrame();
        encode_info.CodecInfo().encode_h264.std_picture_info.primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_P;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxPPictureL0ReferenceCount-08340");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.end();
    }

    if (config_no_b) {
        VideoConfig config = config_no_b;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.begin();
        cb.BeginVideoCoding(context.Begin());

        VideoEncodeInfo encode_info = context.EncodeFrame();
        encode_info.CodecInfo().encode_h264.std_picture_info.primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_B;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxBPictureL0ReferenceCount-08341");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.end();
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

        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddReferenceFrame(0);
        encode_info.CodecInfo().encode_h264.std_picture_info.primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_P;
        encode_info.CodecInfo().encode_h264.std_reference_info[0].primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_P;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxPPictureL0ReferenceCount-08340");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxPPictureL0ReferenceCount-08340");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.end();
    }

    if (config_no_b) {
        VideoConfig config = config_no_b;

        config.SessionCreateInfo()->maxDpbSlots = 2;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.begin();
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
        cb.end();
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

        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddReferenceFrame(0);
        encode_info.CodecInfo().encode_h264.std_reference_info[0].primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_B;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-flags-08342");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.end();
    }

    if (config_no_b_in_l1) {
        VideoConfig config = config_no_b_in_l1;

        config.SessionCreateInfo()->maxDpbSlots = 2;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddBackReferenceFrame(0);
        encode_info.CodecInfo().encode_h264.std_reference_info[0].primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_B;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-flags-08343");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.end();
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

        cb.begin();
        cb.BeginVideoCoding(context.Begin());

        VideoEncodeInfo encode_info = context.EncodeFrame();
        encode_info.CodecInfo().encode_h265.std_picture_info.pic_type = STD_VIDEO_H265_PICTURE_TYPE_P;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxPPictureL0ReferenceCount-08345");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.end();
    }

    if (config_no_b) {
        VideoConfig config = config_no_b;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.begin();
        cb.BeginVideoCoding(context.Begin());

        VideoEncodeInfo encode_info = context.EncodeFrame();
        encode_info.CodecInfo().encode_h265.std_picture_info.pic_type = STD_VIDEO_H265_PICTURE_TYPE_B;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxBPictureL0ReferenceCount-08346");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.end();
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

        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddReferenceFrame(0);
        encode_info.CodecInfo().encode_h265.std_picture_info.pic_type = STD_VIDEO_H265_PICTURE_TYPE_P;
        encode_info.CodecInfo().encode_h265.std_reference_info[0].pic_type = STD_VIDEO_H265_PICTURE_TYPE_P;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxPPictureL0ReferenceCount-08345");
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-maxPPictureL0ReferenceCount-08345");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.end();
    }

    if (config_no_b) {
        VideoConfig config = config_no_b;

        config.SessionCreateInfo()->maxDpbSlots = 2;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.begin();
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
        cb.end();
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

        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddReferenceFrame(0);
        encode_info.CodecInfo().encode_h265.std_reference_info[0].pic_type = STD_VIDEO_H265_PICTURE_TYPE_B;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-flags-08347");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.end();
    }

    if (config_no_b_in_l1) {
        VideoConfig config = config_no_b_in_l1;

        config.SessionCreateInfo()->maxDpbSlots = 2;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

        VideoEncodeInfo encode_info = context.EncodeFrame(1).AddBackReferenceFrame(0);
        encode_info.CodecInfo().encode_h265.std_reference_info[0].pic_type = STD_VIDEO_H265_PICTURE_TYPE_B;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-flags-08348");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        cb.EndVideoCoding(context.End());
        cb.end();
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

    cb.begin();
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

    // Missing reference slot for DPB index refered to by the H.264 L0 or L1 list
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
    cb.end();
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

    cb.begin();
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

    // Missing reference slot for DPB index refered to by the H.265 L0 or L1 list
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
    cb.end();
}

TEST_F(NegativeVideo, CreateBufferInvalidProfileList) {
    TEST_DESCRIPTION("vkCreateBuffer - invalid/missing profile list");

    EnableVideoMaintenance1();
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

    EnableVideoMaintenance1();
    RETURN_IF_SKIP(Init());
    if (!IsVideoMaintenance1Enabled()) {
        GTEST_SKIP() << "Test requires videoMaintenance1";
    }

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

    EnableVideoMaintenance1();
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

    EnableVideoMaintenance1();
    RETURN_IF_SKIP(Init());
    if (!IsVideoMaintenance1Enabled()) {
        GTEST_SKIP() << "Test requires videoMaintenance1";
    }

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

    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-pNext-06811");
    image_ci.format = VK_FORMAT_D16_UNORM;
    vk::CreateImage(device(), &image_ci, nullptr, &image);
    m_errorMonitor->VerifyFound();
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

    EnableVideoMaintenance1();
    RETURN_IF_SKIP(Init());
    if (!IsVideoMaintenance1Enabled()) {
        GTEST_SKIP() << "Test requires videoMaintenance1";
    }

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

        auto image_usage_ci = vku::InitStruct<VkImageViewUsageCreateInfoKHR>();
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

    cb.begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-07126");
    vk::CmdBeginQuery(cb.handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->VerifyFound();

    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 0, 0);

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginQuery-queryPool-01922");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-None-07127");
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 1, 0);
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(cb.handle(), context.StatusQueryPool(), 0);
    cb.EndVideoCoding(context.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-07128");
    vk::CmdBeginQuery(cb.handle(), context2.StatusQueryPool(), 0, 0);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-07130");
    vk::CmdBeginQuery(cb.handle(), context2.EncodeFeedbackQueryPool(), 0, 0);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.end();
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

    cb.begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-07129");
    vk::CmdBeginQuery(cb.handle(), context.EncodeFeedbackQueryPool(), 0, 0);
    m_errorMonitor->VerifyFound();

    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginQuery-queryType-00803");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginQuery-queryType-07128");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-07131");
    vk::CmdBeginQuery(cb.handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(NegativeVideo, BeginQueryInlineQueries) {
    TEST_DESCRIPTION("vkCmdBeginQuery - bound video session was created with VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR");

    EnableVideoMaintenance1();
    RETURN_IF_SKIP(Init());
    if (!IsVideoMaintenance1Enabled()) {
        GTEST_SKIP() << "Test requires videoMaintenance1";
    }

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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-None-08370");
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 0, 0);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(NegativeVideo, GetQueryPoolResultsVideoQueryDataSize) {
    TEST_DESCRIPTION("vkGetQueryPoolResults - test expected data size for video query results");

    RETURN_IF_SKIP(Init());

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

    m_command_buffer.begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-queryType-09442");
    flags = 0;
    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0, 1, buffer.handle(), 0, sizeof(uint32_t), flags);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-flags-09443");
    flags = VK_QUERY_RESULT_WITH_STATUS_BIT_KHR | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT;
    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0, 1, buffer.handle(), 0, sizeof(uint32_t), flags);
    m_errorMonitor->VerifyFound();

    m_command_buffer.end();
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

    cb.begin();

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

    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset());

    cb.DecodeVideo(context.DecodeFrame());

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    cb.DecodeVideo(context.DecodeFrame());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(NegativeVideoSyncVal, DecodeReconstructedPicture) {
    TEST_DESCRIPTION("Test video decode reconstructred picture sync hazard");

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

    cb.begin();
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
    cb.end();
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

    cb.begin();
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
    cb.end();
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

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset());

    cb.EncodeVideo(context.EncodeFrame());

    m_errorMonitor->SetDesiredError("SYNC-HAZARD-WRITE-AFTER-WRITE");
    cb.EncodeVideo(context.EncodeFrame());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(NegativeVideoSyncVal, EncodeReconstructedPicture) {
    TEST_DESCRIPTION("Test video Encode reconstructred picture sync hazard");

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

    cb.begin();
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
    cb.end();
}

TEST_F(NegativeVideoSyncVal, EncodeReferencePicture) {
    TEST_DESCRIPTION("Test video encode reference picture sync hazard");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncode()), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video Encode support with references and 3 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 3;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
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
    cb.end();
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
    context.vk.GetVideoSessionMemoryRequirementsKHR(device(), context.Session(), &mem_req_count, &mem_req);
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
    context.vk.BindVideoSessionMemoryKHR(device(), context.Session(), 1, &bind_info);
    m_errorMonitor->VerifyFound();

    uint32_t mem_req_count = 0;
    context.vk.GetVideoSessionMemoryRequirementsKHR(device(), context.Session(), &mem_req_count, nullptr);

    if (mem_req_count > 0) {
        m_errorMonitor->SetDesiredWarning("BestPractices-vkBindVideoSessionMemoryKHR-requirements-not-all-retrieved");
        context.vk.BindVideoSessionMemoryKHR(device(), context.Session(), 1, &bind_info);
        m_errorMonitor->VerifyFound();

        if (mem_req_count > 1) {
            auto mem_req = vku::InitStruct<VkVideoSessionMemoryRequirementsKHR>();
            mem_req_count = 1;

            context.vk.GetVideoSessionMemoryRequirementsKHR(device(), context.Session(), &mem_req_count, &mem_req);

            m_errorMonitor->SetDesiredWarning("BestPractices-vkBindVideoSessionMemoryKHR-requirements-not-all-retrieved");
            context.vk.BindVideoSessionMemoryKHR(device(), context.Session(), 1, &bind_info);
            m_errorMonitor->VerifyFound();
        }
    }
}
