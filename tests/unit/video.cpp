/*
 * Copyright (c) 2022-2023 The Khronos Group Inc.
 * Copyright (c) 2022-2023 RasterGrid Kft.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/video_objects.h"

TEST_F(VkVideoLayerTest, VideoCodingScope) {
    TEST_DESCRIPTION("Tests calling functions inside/outside video coding scope");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    // Video coding block must be ended before command buffer
    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkEndCommandBuffer-None-06991");
    vk::EndCommandBuffer(cb.handle());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();

    // vkCmdEndVideoCoding not allowed outside video coding block
    cb.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndVideoCodingKHR-videocoding");
    cb.EndVideoCoding(context.End());
    m_errorMonitor->VerifyFound();

    cb.end();

    // vkCmdBeginVideoCoding not allowed inside video coding block
    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginVideoCodingKHR-videocoding");
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();

    // vkCmdControlVideoCoding not allowed outside video coding block
    cb.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdControlVideoCodingKHR-videocoding");
    cb.ControlVideoCoding(context.Control().Reset());
    m_errorMonitor->VerifyFound();

    cb.end();
}

TEST_F(VkVideoLayerTest, VideoProfileInvalidLumaChromaSubsampling) {
    TEST_DESCRIPTION("Test single bit set in VkVideoProfileInfoKHR chromaSubsampling, lumaBitDepth, and chromaBitDepth");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VkVideoProfileInfoKHR profile;

    // Multiple bits in chromaSubsampling
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoProfileInfoKHR-chromaSubsampling-07013");
    profile = *config.Profile();
    profile.chromaSubsampling = VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR | VK_VIDEO_CHROMA_SUBSAMPLING_422_BIT_KHR;
    vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), &profile, config.Caps());
    m_errorMonitor->VerifyFound();

    // Multiple bits in lumaBitDepth
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoProfileInfoKHR-lumaBitDepth-07014");
    profile = *config.Profile();
    profile.lumaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR | VK_VIDEO_COMPONENT_BIT_DEPTH_10_BIT_KHR;
    vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), &profile, config.Caps());
    m_errorMonitor->VerifyFound();

    // Multiple bits in chromaBitDepth
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoProfileInfoKHR-chromaSubsampling-07015");
    profile = *config.Profile();
    profile.chromaSubsampling = VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR;
    profile.chromaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR | VK_VIDEO_COMPONENT_BIT_DEPTH_10_BIT_KHR;
    vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), &profile, config.Caps());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkVideoLayerTest, VideoProfileMissingCodecInfo) {
    TEST_DESCRIPTION("Test missing codec-specific structure in profile definition");

    RETURN_IF_SKIP(Init())

    VkVideoProfileInfoKHR profile;

    if (GetConfigDecodeH264()) {
        VideoConfig config = GetConfigDecodeH264();

        profile = *config.Profile();
        profile.pNext = nullptr;

        // Missing codec-specific info for H.264 decode profile
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoProfileInfoKHR-videoCodecOperation-07179");
        vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), &profile, config.Caps());
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigDecodeH265()) {
        VideoConfig config = GetConfigDecodeH265();

        profile = *config.Profile();
        profile.pNext = nullptr;

        // Missing codec-specific info for H.265 decode profile
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoProfileInfoKHR-videoCodecOperation-07180");
        vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), &profile, config.Caps());
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkVideoLayerTest, CapabilityQueryMissingChain) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoCapabilitiesKHR - missing return structures in chain");

    RETURN_IF_SKIP(Init())

    if (GetConfigDecodeH264()) {
        VideoConfig config = GetConfigDecodeH264();

        VkVideoCapabilitiesKHR caps = vku::InitStructHelper();
        VkVideoDecodeCapabilitiesKHR decode_caps = vku::InitStructHelper();
        VkVideoDecodeH264CapabilitiesKHR decode_h264_caps = vku::InitStructHelper();

        // Missing decode caps struct for decode profile
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07183");
        caps.pNext = &decode_h264_caps;
        vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
        m_errorMonitor->VerifyFound();

        // Missing H.264 decode caps struct for H.264 decode profile
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07184");
        caps.pNext = &decode_caps;
        vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigDecodeH265()) {
        VideoConfig config = GetConfigDecodeH265();

        VkVideoCapabilitiesKHR caps = vku::InitStructHelper();
        VkVideoDecodeCapabilitiesKHR decode_caps = vku::InitStructHelper();
        VkVideoDecodeH265CapabilitiesKHR decode_h265_caps = vku::InitStructHelper();

        // Missing decode caps struct for decode profile
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07183");
        caps.pNext = &decode_h265_caps;
        vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
        m_errorMonitor->VerifyFound();

        // Missing H.265 decode caps struct for H.265 decode profile
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07185");
        caps.pNext = &decode_caps;
        vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), &caps);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkVideoLayerTest, VideoFormatQueryMissingProfile) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoFormatPropertiesKHR - missing profile info");

    RETURN_IF_SKIP(Init())

    if (!GetConfig()) {
        GTEST_SKIP() << "Test requires video support";
    }

    VkPhysicalDeviceVideoFormatInfoKHR format_info = vku::InitStructHelper();
    format_info.imageUsage = VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR;
    uint32_t format_count = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceVideoFormatPropertiesKHR-pNext-06812");
    vk.GetPhysicalDeviceVideoFormatPropertiesKHR(gpu(), &format_info, &format_count, nullptr);
    m_errorMonitor->VerifyFound();

    VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
    format_info.pNext = &profile_list;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetPhysicalDeviceVideoFormatPropertiesKHR-pNext-06812");
    vk.GetPhysicalDeviceVideoFormatPropertiesKHR(gpu(), &format_info, &format_count, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkVideoLayerTest, InUseDestroyed) {
    TEST_DESCRIPTION("Test destroying objects while they are still in use");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfigWithParams(GetConfigs());
    if (!config) {
        config = GetConfig();
    }
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset());
    cb.EndVideoCoding(context.End());
    cb.end();

    context.Queue().submit(cb);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyVideoSessionKHR-videoSession-07192");
    context.vk.DestroyVideoSessionKHR(m_device->device(), context.Session(), nullptr);
    m_errorMonitor->VerifyFound();

    if (config.NeedsSessionParams()) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyVideoSessionParametersKHR-videoSessionParameters-07212");
        context.vk.DestroyVideoSessionParametersKHR(m_device->device(), context.SessionParams(), nullptr);
        m_errorMonitor->VerifyFound();
    }

    m_device->wait();
}

TEST_F(VkVideoLayerTest, CreateSessionProtectedMemoryNotEnabled) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - cannot enable protected content without protected memory");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(DeviceObj(), config);

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.flags = VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR;
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionCreateInfoKHR-protectedMemory-07189");
    context.vk.CreateVideoSessionKHR(m_device->device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkVideoLayerTest, CreateSessionProtectedContentUnsupported) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - cannot enable protected content if not supported");

    const bool enable_protected_memory = true;
    RETURN_IF_SKIP(Init(enable_protected_memory));
    if (!IsProtectedMemoryEnabled()) {
        GTEST_SKIP() << "Test requires protectedMemory support";
    }

    VideoConfig config = GetConfigWithoutProtectedContent(GetConfigs());
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with no protected content support";
    }

    VideoContext context(DeviceObj(), config);

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.flags = VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR;
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionCreateInfoKHR-protectedMemory-07189");
    context.vk.CreateVideoSessionKHR(m_device->device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkVideoLayerTest, CreateSessionUnsupportedProfile) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - unsupported profile");

    RETURN_IF_SKIP(Init())
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    VideoConfig config;
    VkVideoProfileInfoKHR profile;
    VkVideoDecodeH264ProfileInfoKHR profile_h264;
    VkVideoDecodeH265ProfileInfoKHR profile_h265;

    if (GetConfigDecodeH264()) {
        config = GetConfigDecodeH264();

        profile = *config.Profile();
        profile_h264 = *vku::FindStructInPNextChain<VkVideoDecodeH264ProfileInfoKHR>(profile.pNext);

        profile.pNext = &profile_h264;
        profile_h264.stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_INVALID;
    } else if (GetConfigDecodeH265()) {
        config = GetConfigDecodeH265();

        profile = *config.Profile();
        profile_h265 = *vku::FindStructInPNextChain<VkVideoDecodeH265ProfileInfoKHR>(profile.pNext);

        profile.pNext = &profile_h265;
        profile_h265.stdProfileIdc = STD_VIDEO_H265_PROFILE_IDC_INVALID;
    } else {
        GTEST_SKIP() << "Test requires support for H.264 or H.265 decode";
    }

    VideoContext context(DeviceObj(), config);

    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = &profile;
    create_info.pStdHeaderVersion = config.StdVersion();

    VkVideoSessionKHR session;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-04845");
    context.vk.CreateVideoSessionKHR(m_device->device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkVideoLayerTest, CreateSessionInvalidReferencePictureCounts) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid reference picture slot and active counts");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(DeviceObj(), config);

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();
    create_info.maxDpbSlots = config.Caps()->maxDpbSlots;
    create_info.maxActiveReferencePictures = config.Caps()->maxActiveReferencePictures;

    // maxDpbSlots too big
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionCreateInfoKHR-maxDpbSlots-04847");
    create_info.maxDpbSlots++;
    context.vk.CreateVideoSessionKHR(m_device->device(), &create_info, nullptr, &session);
    create_info.maxDpbSlots--;
    m_errorMonitor->VerifyFound();

    // maxActiveReferencePictures too big
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionCreateInfoKHR-maxActiveReferencePictures-04849");
    create_info.maxActiveReferencePictures++;
    context.vk.CreateVideoSessionKHR(m_device->device(), &create_info, nullptr, &session);
    create_info.maxActiveReferencePictures--;
    m_errorMonitor->VerifyFound();

    config = GetConfig(GetConfigsWithReferences(GetConfigs()));
    if (config) {
        // maxDpbSlots is 0, but maxActiveReferencePictures is not
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionCreateInfoKHR-maxDpbSlots-04850");
        create_info.maxDpbSlots = 0;
        create_info.maxActiveReferencePictures = 1;
        context.vk.CreateVideoSessionKHR(m_device->device(), &create_info, nullptr, &session);
        m_errorMonitor->VerifyFound();

        // maxActiveReferencePictures is 0, but maxDpbSlots is not
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionCreateInfoKHR-maxDpbSlots-04850");
        create_info.maxDpbSlots = 1;
        create_info.maxActiveReferencePictures = 0;
        context.vk.CreateVideoSessionKHR(m_device->device(), &create_info, nullptr, &session);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkVideoLayerTest, CreateSessionInvalidMaxCodedExtent) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - maxCodedExtent outside of supported range");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(DeviceObj(), config);

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    // maxCodedExtent.width too small
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionCreateInfoKHR-maxCodedExtent-04851");
    create_info.maxCodedExtent = config.Caps()->minCodedExtent;
    --create_info.maxCodedExtent.width;
    context.vk.CreateVideoSessionKHR(m_device->device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();

    // maxCodedExtent.height too small
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionCreateInfoKHR-maxCodedExtent-04851");
    create_info.maxCodedExtent = config.Caps()->minCodedExtent;
    --create_info.maxCodedExtent.height;
    context.vk.CreateVideoSessionKHR(m_device->device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();

    // maxCodedExtent.width too big
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionCreateInfoKHR-maxCodedExtent-04851");
    create_info.maxCodedExtent = config.Caps()->maxCodedExtent;
    ++create_info.maxCodedExtent.width;
    context.vk.CreateVideoSessionKHR(m_device->device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();

    // maxCodedExtent.height too big
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionCreateInfoKHR-maxCodedExtent-04851");
    create_info.maxCodedExtent = config.Caps()->maxCodedExtent;
    ++create_info.maxCodedExtent.height;
    context.vk.CreateVideoSessionKHR(m_device->device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkVideoLayerTest, CreateSessionInvalidDecodeReferencePictureFormat) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid decode reference picture format");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionCreateInfoKHR-referencePictureFormat-04852");
    create_info.referencePictureFormat = VK_FORMAT_D16_UNORM;
    context.vk.CreateVideoSessionKHR(m_device->device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkVideoLayerTest, CreateSessionInvalidDecodePictureFormat) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid decode picture format");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    VideoContext context(DeviceObj(), config);

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionCreateInfoKHR-pictureFormat-04853");
    create_info.pictureFormat = VK_FORMAT_D16_UNORM;
    context.vk.CreateVideoSessionKHR(m_device->device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkVideoLayerTest, CreateSessionInvalidStdHeaderVersion) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid Video Std header version");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(DeviceObj(), config);

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    VkExtensionProperties std_version = *config.StdVersion();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = &std_version;

    // Video Std header version not supported
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionCreateInfoKHR-pStdHeaderVersion-07191");
    ++std_version.specVersion;
    context.vk.CreateVideoSessionKHR(m_device->device(), &create_info, nullptr, &session);
    --std_version.specVersion;
    m_errorMonitor->VerifyFound();

    // Video Std header name not supported
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionCreateInfoKHR-pStdHeaderVersion-07190");
    strcpy(std_version.extensionName, "invalid_std_header_name");
    context.vk.CreateVideoSessionKHR(m_device->device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkVideoLayerTest, BindVideoSessionMemory) {
    TEST_DESCRIPTION("vkBindVideoSessionMemoryKHR - memory binding related invalid usages");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(DeviceObj(), config);

    uint32_t mem_req_count;
    ASSERT_EQ(VK_SUCCESS,
        context.vk.GetVideoSessionMemoryRequirementsKHR(m_device->device(), context.Session(), &mem_req_count, nullptr));
    if (mem_req_count == 0) {
        GTEST_SKIP() << "Test can only run if video session needs memory bindings";
    }

    std::vector<VkVideoSessionMemoryRequirementsKHR> mem_reqs(mem_req_count, vku::InitStruct<VkVideoSessionMemoryRequirementsKHR>());
    ASSERT_EQ(VK_SUCCESS,
        context.vk.GetVideoSessionMemoryRequirementsKHR(m_device->device(), context.Session(), &mem_req_count, mem_reqs.data()));

    std::vector<VkDeviceMemory> session_memory;
    std::vector<VkBindVideoSessionMemoryInfoKHR> bind_info(mem_req_count, vku::InitStruct<VkBindVideoSessionMemoryInfoKHR>());
    for (uint32_t i = 0; i < mem_req_count; ++i) {
        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
        ASSERT_TRUE(m_device->phy().set_memory_type(mem_reqs[i].memoryRequirements.memoryTypeBits, &alloc_info, 0));
        alloc_info.allocationSize = mem_reqs[i].memoryRequirements.size * 2;

        VkDeviceMemory memory = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(m_device->device(), &alloc_info, nullptr, &memory));
        session_memory.push_back(memory);

        bind_info[i].memoryBindIndex = mem_reqs[i].memoryBindIndex;
        bind_info[i].memory = memory;
        bind_info[i].memoryOffset = 0;
        bind_info[i].memorySize = mem_reqs[i].memoryRequirements.size;
    }

    // Duplicate memoryBindIndex
    if (mem_req_count > 1) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindVideoSessionMemoryKHR-memoryBindIndex-07196");
        auto& duplicate = bind_info[mem_req_count / 2];
        auto backup = duplicate;
        duplicate = bind_info[0];
        context.vk.BindVideoSessionMemoryKHR(m_device->device(), context.Session(), mem_req_count, bind_info.data());
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
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07197");
        auto& invalid = bind_info[mem_req_count / 2];
        auto backup = invalid;
        invalid.memoryBindIndex = invalid_bind_index;
        context.vk.BindVideoSessionMemoryKHR(m_device->device(), context.Session(), mem_req_count, bind_info.data());
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
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(m_device->device(), &alloc_info, nullptr, &memory));

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07198");
        auto& invalid = bind_info[invalid_mem_type_req_index];
        auto backup = invalid;
        invalid.memory = memory;
        context.vk.BindVideoSessionMemoryKHR(m_device->device(), context.Session(), mem_req_count, bind_info.data());
        invalid = backup;
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(m_device->device(), memory, nullptr);
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

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07199");
        auto& invalid = bind_info[invalid_offset_align_index];
        auto backup = invalid;
        invalid.memoryOffset = mem_req.alignment / 2;
        context.vk.BindVideoSessionMemoryKHR(m_device->device(), context.Session(), mem_req_count, bind_info.data());
        invalid = backup;
        m_errorMonitor->VerifyFound();
    }

    // Incorrect memorySize
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07200");
        auto& invalid = bind_info[mem_req_count / 2];
        auto backup = invalid;
        invalid.memorySize += 16;
        context.vk.BindVideoSessionMemoryKHR(m_device->device(), context.Session(), mem_req_count, bind_info.data());
        invalid = backup;
        m_errorMonitor->VerifyFound();
    }

    // Out-of-bounds memoryOffset
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindVideoSessionMemoryInfoKHR-memoryOffset-07201");
        auto& invalid = bind_info[mem_req_count / 2];
        auto backup = invalid;
        invalid.memoryOffset = invalid.memorySize * 2;
        context.vk.BindVideoSessionMemoryKHR(m_device->device(), context.Session(), mem_req_count, bind_info.data());
        invalid = backup;
        m_errorMonitor->VerifyFound();
    }

    // Out-of-bounds memoryOffset + memorySize
    {
        uint32_t index = mem_req_count / 2;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindVideoSessionMemoryInfoKHR-memorySize-07202");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07199");
        auto& invalid = bind_info[index];
        auto backup = invalid;
        invalid.memoryOffset = invalid.memorySize + 1;
        context.vk.BindVideoSessionMemoryKHR(m_device->device(), context.Session(), mem_req_count, bind_info.data());
        invalid = backup;
        m_errorMonitor->VerifyFound();
    }

    // Already bound
    {
        uint32_t first_bind_count = mem_req_count / 2;
        if (first_bind_count == 0) {
            first_bind_count = 1;
        }

        context.vk.BindVideoSessionMemoryKHR(m_device->device(), context.Session(), first_bind_count, bind_info.data());

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindVideoSessionMemoryKHR-videoSession-07195");
        context.vk.BindVideoSessionMemoryKHR(m_device->device(), context.Session(), 1, &bind_info[first_bind_count - 1]);
        m_errorMonitor->VerifyFound();
    }

    for (auto memory : session_memory) {
        vk::FreeMemory(m_device->device(), memory, nullptr);
    }
}

TEST_F(VkVideoLayerTest, CreateSessionParamsIncompatibleTemplate) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - incompatible template");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfigWithParams(GetConfigs());
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with session parameters";
    }

    VideoContext context1(DeviceObj(), config);
    VideoContext context2(DeviceObj(), config);

    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.videoSessionParametersTemplate = context1.SessionParams();
    create_info.videoSession = context2.Session();

    VkVideoSessionParametersKHR params;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-04855");
    context1.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkVideoLayerTest, CreateSessionParamsMissingCodecInfo) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - missing codec-specific chained structure");

    RETURN_IF_SKIP(Init())

    if (GetConfigDecodeH264()) {
        VideoConfig config = GetConfigDecodeH264();
        VideoContext context(DeviceObj(), config);

        VkVideoSessionParametersKHR params;
        VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
        VkVideoDecodeH265SessionParametersCreateInfoKHR other_codec_info = vku::InitStructHelper();
        other_codec_info.maxStdVPSCount = 1;
        other_codec_info.maxStdSPSCount = 1;
        other_codec_info.maxStdPPSCount = 1;
        create_info.pNext = &other_codec_info;
        create_info.videoSession = context.Session();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07203");
        context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params);
        m_errorMonitor->VerifyFound();
    }

    if (GetConfigDecodeH265()) {
        VideoConfig config = GetConfigDecodeH265();
        VideoContext context(DeviceObj(), config);

        VkVideoSessionParametersKHR params;
        VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
        VkVideoDecodeH264SessionParametersCreateInfoKHR other_codec_info = vku::InitStructHelper();
        other_codec_info.maxStdSPSCount = 1;
        other_codec_info.maxStdPPSCount = 1;
        create_info.pNext = &other_codec_info;
        create_info.videoSession = context.Session();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07206");
        context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkVideoLayerTest, CreateSessionParamsH264ExceededCapacity) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - H.264 parameter set capacity exceeded");

    RETURN_IF_SKIP(Init())
    VideoConfig config = GetConfigDecodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support";
    }

    VideoContext context(DeviceObj(), config);

    VkVideoDecodeH264SessionParametersCreateInfoKHR h264_ci = vku::InitStructHelper();
    VkVideoDecodeH264SessionParametersAddInfoKHR h264_ai = vku::InitStructHelper();

    VkVideoSessionParametersKHR params, params2;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h264_ci;
    create_info.videoSession = context.Session();

    h264_ci.pParametersAddInfo = &h264_ai;

    std::vector<StdVideoH264SequenceParameterSet> sps_list{CreateH264SPS(1), CreateH264SPS(2), CreateH264SPS(3)};

    std::vector<StdVideoH264PictureParameterSet> pps_list{
        CreateH264PPS(1, 1), CreateH264PPS(1, 4), CreateH264PPS(2, 1),
        CreateH264PPS(2, 2), CreateH264PPS(3, 1), CreateH264PPS(3, 3),
    };

    h264_ai.stdSPSCount = (uint32_t)sps_list.size();
    h264_ai.pStdSPSs = sps_list.data();
    h264_ai.stdPPSCount = (uint32_t)pps_list.size();
    h264_ai.pStdPPSs = pps_list.data();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07204");
    h264_ci.maxStdSPSCount = 2;
    h264_ci.maxStdPPSCount = 8;
    context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07205");
    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 5;
    context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 6;
    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params));

    create_info.videoSessionParametersTemplate = params;
    sps_list[1].seq_parameter_set_id = 4;
    pps_list[1].seq_parameter_set_id = 4;
    pps_list[5].seq_parameter_set_id = 4;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07204");
    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 8;
    context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07205");
    h264_ci.maxStdSPSCount = 4;
    h264_ci.maxStdPPSCount = 7;
    context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    create_info.videoSessionParametersTemplate = params;
    h264_ci.pParametersAddInfo = nullptr;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07204");
    h264_ci.maxStdSPSCount = 2;
    h264_ci.maxStdPPSCount = 8;
    context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07205");
    h264_ci.maxStdSPSCount = 3;
    h264_ci.maxStdPPSCount = 5;
    context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(m_device->device(), params, nullptr);
}

TEST_F(VkVideoLayerTest, CreateSessionParamsH265ExceededCapacity) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - H.265 parameter set capacity exceeded");

    RETURN_IF_SKIP(Init())
    VideoConfig config = GetConfigDecodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support";
    }

    VideoContext context(DeviceObj(), config);

    VkVideoDecodeH265SessionParametersCreateInfoKHR h265_ci = vku::InitStructHelper();
    VkVideoDecodeH265SessionParametersAddInfoKHR h265_ai = vku::InitStructHelper();

    VkVideoSessionParametersKHR params, params2;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h265_ci;
    create_info.videoSession = context.Session();

    h265_ci.pParametersAddInfo = &h265_ai;

    std::vector<StdVideoH265VideoParameterSet> vps_list{
        CreateH265VPS(1),
        CreateH265VPS(2),
    };

    std::vector<StdVideoH265SequenceParameterSet> sps_list{
        CreateH265SPS(1, 1),
        CreateH265SPS(1, 2),
        CreateH265SPS(2, 1),
        CreateH265SPS(2, 3),
    };

    std::vector<StdVideoH265PictureParameterSet> pps_list{
        CreateH265PPS(1, 1, 1), CreateH265PPS(1, 1, 2), CreateH265PPS(1, 2, 1), CreateH265PPS(2, 1, 3),
        CreateH265PPS(2, 3, 1), CreateH265PPS(2, 3, 2), CreateH265PPS(2, 3, 3),
    };

    h265_ai.stdVPSCount = (uint32_t)vps_list.size();
    h265_ai.pStdVPSs = vps_list.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list.size();
    h265_ai.pStdSPSs = sps_list.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list.size();
    h265_ai.pStdPPSs = pps_list.data();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07207");
    h265_ci.maxStdVPSCount = 1;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 8;
    context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07208");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 3;
    h265_ci.maxStdPPSCount = 9;
    context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07209");
    h265_ci.maxStdVPSCount = 3;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 5;
    context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();

    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 7;
    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params));

    create_info.videoSessionParametersTemplate = params;
    vps_list[1].vps_video_parameter_set_id = 3;
    sps_list[1].sps_video_parameter_set_id = 3;
    pps_list[1].sps_video_parameter_set_id = 3;
    pps_list[5].sps_video_parameter_set_id = 3;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07207");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 10;
    context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07208");
    h265_ci.maxStdVPSCount = 3;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 9;
    context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07209");
    h265_ci.maxStdVPSCount = 3;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 8;
    context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    create_info.videoSessionParametersTemplate = params;
    h265_ci.pParametersAddInfo = nullptr;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07207");
    h265_ci.maxStdVPSCount = 1;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 7;
    context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07208");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 3;
    h265_ci.maxStdPPSCount = 7;
    context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07209");
    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 4;
    h265_ci.maxStdPPSCount = 6;
    context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params2);
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(m_device->device(), params, nullptr);
}

TEST_F(VkVideoLayerTest, H264ParametersAddInfoUniqueness) {
    TEST_DESCRIPTION("VkVideoDecodeH264SessionParametersAddInfoKHR - parameter set uniqueness");

    RETURN_IF_SKIP(Init())
    VideoConfig config = GetConfigDecodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support";
    }

    VideoContext context(DeviceObj(), config);

    VkVideoDecodeH264SessionParametersCreateInfoKHR h264_ci = vku::InitStructHelper();
    VkVideoDecodeH264SessionParametersAddInfoKHR h264_ai = vku::InitStructHelper();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h264_ci;
    create_info.videoSession = context.Session();

    h264_ci.maxStdSPSCount = 10;
    h264_ci.maxStdPPSCount = 20;
    h264_ci.pParametersAddInfo = &h264_ai;

    VkVideoSessionParametersUpdateInfoKHR update_info = vku::InitStructHelper();
    update_info.pNext = &h264_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH264SequenceParameterSet> sps_list{CreateH264SPS(1), CreateH264SPS(2), CreateH264SPS(3)};

    std::vector<StdVideoH264PictureParameterSet> pps_list{
        CreateH264PPS(1, 1), CreateH264PPS(1, 4), CreateH264PPS(2, 1),
        CreateH264PPS(2, 2), CreateH264PPS(3, 1), CreateH264PPS(3, 3),
    };

    h264_ai.stdSPSCount = (uint32_t)sps_list.size();
    h264_ai.pStdSPSs = sps_list.data();
    h264_ai.stdPPSCount = (uint32_t)pps_list.size();
    h264_ai.pStdPPSs = pps_list.data();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoDecodeH264SessionParametersAddInfoKHR-None-04825");
    sps_list[0].seq_parameter_set_id = 3;
    context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params);
    sps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoDecodeH264SessionParametersAddInfoKHR-None-04826");
    pps_list[0].seq_parameter_set_id = 2;
    context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params);
    pps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    h264_ci.pParametersAddInfo = nullptr;
    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoDecodeH264SessionParametersAddInfoKHR-None-04825");
    sps_list[0].seq_parameter_set_id = 3;
    context.vk.UpdateVideoSessionParametersKHR(m_device->device(), params, &update_info);
    sps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoDecodeH264SessionParametersAddInfoKHR-None-04826");
    pps_list[0].seq_parameter_set_id = 2;
    context.vk.UpdateVideoSessionParametersKHR(m_device->device(), params, &update_info);
    pps_list[0].seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(m_device->device(), params, nullptr);
}

TEST_F(VkVideoLayerTest, H265ParametersAddInfoUniqueness) {
    TEST_DESCRIPTION("VkVideoDecodeH265SessionParametersAddInfoKHR - parameter set uniqueness");

    RETURN_IF_SKIP(Init())
    VideoConfig config = GetConfigDecodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();

    VkVideoDecodeH265SessionParametersCreateInfoKHR h265_ci = vku::InitStructHelper();
    VkVideoDecodeH265SessionParametersAddInfoKHR h265_ai = vku::InitStructHelper();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h265_ci;
    create_info.videoSession = context.Session();

    h265_ci.maxStdVPSCount = 10;
    h265_ci.maxStdSPSCount = 20;
    h265_ci.maxStdPPSCount = 30;
    h265_ci.pParametersAddInfo = &h265_ai;

    VkVideoSessionParametersUpdateInfoKHR update_info = vku::InitStructHelper();
    update_info.pNext = &h265_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH265VideoParameterSet> vps_list{
        CreateH265VPS(1),
        CreateH265VPS(2),
    };

    std::vector<StdVideoH265SequenceParameterSet> sps_list{
        CreateH265SPS(1, 1),
        CreateH265SPS(1, 2),
        CreateH265SPS(2, 1),
        CreateH265SPS(2, 3),
    };

    std::vector<StdVideoH265PictureParameterSet> pps_list{
        CreateH265PPS(1, 1, 1), CreateH265PPS(1, 1, 2), CreateH265PPS(1, 2, 1), CreateH265PPS(2, 1, 3),
        CreateH265PPS(2, 3, 1), CreateH265PPS(2, 3, 2), CreateH265PPS(2, 3, 3),
    };

    h265_ai.stdVPSCount = (uint32_t)vps_list.size();
    h265_ai.pStdVPSs = vps_list.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list.size();
    h265_ai.pStdSPSs = sps_list.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list.size();
    h265_ai.pStdPPSs = pps_list.data();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04833");
    vps_list[0].vps_video_parameter_set_id = 2;
    context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params);
    vps_list[0].vps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04834");
    sps_list[0].sps_video_parameter_set_id = 2;
    context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params);
    sps_list[0].sps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04835");
    pps_list[0].pps_seq_parameter_set_id = 2;
    context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params);
    pps_list[0].pps_seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    h265_ci.pParametersAddInfo = nullptr;
    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04833");
    vps_list[0].vps_video_parameter_set_id = 2;
    context.vk.UpdateVideoSessionParametersKHR(m_device->device(), params, &update_info);
    vps_list[0].vps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04834");
    sps_list[0].sps_video_parameter_set_id = 2;
    context.vk.UpdateVideoSessionParametersKHR(m_device->device(), params, &update_info);
    sps_list[0].sps_video_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04835");
    pps_list[0].pps_seq_parameter_set_id = 2;
    context.vk.UpdateVideoSessionParametersKHR(m_device->device(), params, &update_info);
    pps_list[0].pps_seq_parameter_set_id = 1;
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(m_device->device(), params, nullptr);
}

TEST_F(VkVideoLayerTest, UpdateSessionParamsIncorrectSequenceCount) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - invalid update sequence count");

    RETURN_IF_SKIP(Init())
    VideoConfig config = GetConfigWithParams(GetConfigs());
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with session parameters";
    }

    VideoContext context(DeviceObj(), config);

    VkVideoSessionParametersUpdateInfoKHR update_info = vku::InitStructHelper();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkUpdateVideoSessionParametersKHR-pUpdateInfo-07215");
    update_info.updateSequenceCount = 2;
    context.vk.UpdateVideoSessionParametersKHR(m_device->device(), context.SessionParams(), &update_info);
    m_errorMonitor->VerifyFound();

    update_info.updateSequenceCount = 1;
    ASSERT_EQ(VK_SUCCESS, context.vk.UpdateVideoSessionParametersKHR(m_device->device(), context.SessionParams(), &update_info));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkUpdateVideoSessionParametersKHR-pUpdateInfo-07215");
    update_info.updateSequenceCount = 1;
    context.vk.UpdateVideoSessionParametersKHR(m_device->device(), context.SessionParams(), &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkUpdateVideoSessionParametersKHR-pUpdateInfo-07215");
    update_info.updateSequenceCount = 3;
    context.vk.UpdateVideoSessionParametersKHR(m_device->device(), context.SessionParams(), &update_info);
    m_errorMonitor->VerifyFound();

    update_info.updateSequenceCount = 2;
    ASSERT_EQ(VK_SUCCESS, context.vk.UpdateVideoSessionParametersKHR(m_device->device(), context.SessionParams(), &update_info));
}

TEST_F(VkVideoLayerTest, UpdateSessionParamsH264ConflictingKeys) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - H.264 conflicting parameter set keys");

    RETURN_IF_SKIP(Init())
    VideoConfig config = GetConfigDecodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support";
    }

    VideoContext context(DeviceObj(), config);

    VkVideoDecodeH264SessionParametersCreateInfoKHR h264_ci = vku::InitStructHelper();
    VkVideoDecodeH264SessionParametersAddInfoKHR h264_ai = vku::InitStructHelper();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h264_ci;
    create_info.videoSession = context.Session();

    h264_ci.maxStdSPSCount = 10;
    h264_ci.maxStdPPSCount = 20;
    h264_ci.pParametersAddInfo = &h264_ai;

    VkVideoSessionParametersUpdateInfoKHR update_info = vku::InitStructHelper();
    update_info.pNext = &h264_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH264SequenceParameterSet> sps_list{CreateH264SPS(1), CreateH264SPS(2), CreateH264SPS(3)};

    std::vector<StdVideoH264PictureParameterSet> pps_list{
        CreateH264PPS(1, 1), CreateH264PPS(1, 4), CreateH264PPS(2, 1),
        CreateH264PPS(2, 2), CreateH264PPS(3, 1), CreateH264PPS(3, 3),
    };

    h264_ai.stdSPSCount = (uint32_t)sps_list.size();
    h264_ai.pStdSPSs = sps_list.data();
    h264_ai.stdPPSCount = (uint32_t)pps_list.size();
    h264_ai.pStdPPSs = pps_list.data();

    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params));

    std::vector<StdVideoH264SequenceParameterSet> sps_list2{CreateH264SPS(4), CreateH264SPS(5)};

    std::vector<StdVideoH264PictureParameterSet> pps_list2{CreateH264PPS(1, 3), CreateH264PPS(3, 2), CreateH264PPS(4, 1),
                                                           CreateH264PPS(5, 2)};

    h264_ai.stdSPSCount = (uint32_t)sps_list2.size();
    h264_ai.pStdSPSs = sps_list2.data();
    h264_ai.stdPPSCount = (uint32_t)pps_list2.size();
    h264_ai.pStdPPSs = pps_list2.data();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07216");
    sps_list2[1].seq_parameter_set_id = 2;
    context.vk.UpdateVideoSessionParametersKHR(m_device->device(), params, &update_info);
    sps_list2[1].seq_parameter_set_id = 5;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07218");
    pps_list2[2].seq_parameter_set_id = 1;
    context.vk.UpdateVideoSessionParametersKHR(m_device->device(), params, &update_info);
    pps_list2[2].seq_parameter_set_id = 4;
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(m_device->device(), params, nullptr);
}

TEST_F(VkVideoLayerTest, UpdateSessionParamsH265ConflictingKeys) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - H.265 conflicting parameter set keys");

    RETURN_IF_SKIP(Init())
    VideoConfig config = GetConfigDecodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();

    VkVideoDecodeH265SessionParametersCreateInfoKHR h265_ci = vku::InitStructHelper();
    VkVideoDecodeH265SessionParametersAddInfoKHR h265_ai = vku::InitStructHelper();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h265_ci;
    create_info.videoSession = context.Session();

    h265_ci.maxStdVPSCount = 10;
    h265_ci.maxStdSPSCount = 20;
    h265_ci.maxStdPPSCount = 30;
    h265_ci.pParametersAddInfo = &h265_ai;

    VkVideoSessionParametersUpdateInfoKHR update_info = vku::InitStructHelper();
    update_info.pNext = &h265_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH265VideoParameterSet> vps_list{
        CreateH265VPS(1),
        CreateH265VPS(2),
    };

    std::vector<StdVideoH265SequenceParameterSet> sps_list{
        CreateH265SPS(1, 1),
        CreateH265SPS(1, 2),
        CreateH265SPS(2, 1),
        CreateH265SPS(2, 3),
    };

    std::vector<StdVideoH265PictureParameterSet> pps_list{
        CreateH265PPS(1, 1, 1), CreateH265PPS(1, 1, 2), CreateH265PPS(1, 2, 1), CreateH265PPS(2, 1, 3),
        CreateH265PPS(2, 3, 1), CreateH265PPS(2, 3, 2), CreateH265PPS(2, 3, 3),
    };

    h265_ai.stdVPSCount = (uint32_t)vps_list.size();
    h265_ai.pStdVPSs = vps_list.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list.size();
    h265_ai.pStdSPSs = sps_list.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list.size();
    h265_ai.pStdPPSs = pps_list.data();

    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params));

    std::vector<StdVideoH265VideoParameterSet> vps_list2{CreateH265VPS(3)};

    std::vector<StdVideoH265SequenceParameterSet> sps_list2{CreateH265SPS(2, 2), CreateH265SPS(3, 1)};

    std::vector<StdVideoH265PictureParameterSet> pps_list2{CreateH265PPS(1, 2, 3), CreateH265PPS(2, 3, 4), CreateH265PPS(3, 1, 2)};

    h265_ai.stdVPSCount = (uint32_t)vps_list2.size();
    h265_ai.pStdVPSs = vps_list2.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list2.size();
    h265_ai.pStdSPSs = sps_list2.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list2.size();
    h265_ai.pStdPPSs = pps_list2.data();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07220");
    vps_list2[0].vps_video_parameter_set_id = 2;
    context.vk.UpdateVideoSessionParametersKHR(m_device->device(), params, &update_info);
    vps_list2[0].vps_video_parameter_set_id = 3;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07222");
    sps_list2[0].sps_seq_parameter_set_id = 3;
    context.vk.UpdateVideoSessionParametersKHR(m_device->device(), params, &update_info);
    sps_list2[0].sps_seq_parameter_set_id = 2;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07224");
    pps_list2[1].pps_pic_parameter_set_id = 2;
    context.vk.UpdateVideoSessionParametersKHR(m_device->device(), params, &update_info);
    pps_list2[1].pps_pic_parameter_set_id = 4;
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(m_device->device(), params, nullptr);
}

TEST_F(VkVideoLayerTest, UpdateSessionParamsH264ExceededCapacity) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - H.264 parameter set capacity exceeded");

    RETURN_IF_SKIP(Init())
    VideoConfig config = GetConfigDecodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support";
    }

    VideoContext context(DeviceObj(), config);

    VkVideoDecodeH264SessionParametersCreateInfoKHR h264_ci = vku::InitStructHelper();
    VkVideoDecodeH264SessionParametersAddInfoKHR h264_ai = vku::InitStructHelper();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h264_ci;
    create_info.videoSession = context.Session();

    h264_ci.maxStdSPSCount = 4;
    h264_ci.maxStdPPSCount = 9;
    h264_ci.pParametersAddInfo = &h264_ai;

    VkVideoSessionParametersUpdateInfoKHR update_info = vku::InitStructHelper();
    update_info.pNext = &h264_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH264SequenceParameterSet> sps_list{CreateH264SPS(1), CreateH264SPS(2), CreateH264SPS(3)};

    std::vector<StdVideoH264PictureParameterSet> pps_list{
        CreateH264PPS(1, 1), CreateH264PPS(1, 4), CreateH264PPS(2, 1),
        CreateH264PPS(2, 2), CreateH264PPS(3, 1), CreateH264PPS(3, 3),
    };

    h264_ai.stdSPSCount = (uint32_t)sps_list.size();
    h264_ai.pStdSPSs = sps_list.data();
    h264_ai.stdPPSCount = (uint32_t)pps_list.size();
    h264_ai.pStdPPSs = pps_list.data();

    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params));

    std::vector<StdVideoH264SequenceParameterSet> sps_list2{CreateH264SPS(4), CreateH264SPS(5)};

    std::vector<StdVideoH264PictureParameterSet> pps_list2{CreateH264PPS(1, 3), CreateH264PPS(3, 2), CreateH264PPS(4, 1),
                                                           CreateH264PPS(5, 2)};

    h264_ai.pStdSPSs = sps_list2.data();
    h264_ai.pStdPPSs = pps_list2.data();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07217");
    h264_ai.stdSPSCount = 2;
    h264_ai.stdPPSCount = 2;
    context.vk.UpdateVideoSessionParametersKHR(m_device->device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07219");
    h264_ai.stdSPSCount = 1;
    h264_ai.stdPPSCount = 4;
    context.vk.UpdateVideoSessionParametersKHR(m_device->device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(m_device->device(), params, nullptr);
}

TEST_F(VkVideoLayerTest, UpdateSessionParamsH265ExceededCapacity) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - H.265 parameter set capacity exceeded");

    RETURN_IF_SKIP(Init())
    VideoConfig config = GetConfigDecodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();

    VkVideoDecodeH265SessionParametersCreateInfoKHR h265_ci = vku::InitStructHelper();
    VkVideoDecodeH265SessionParametersAddInfoKHR h265_ai = vku::InitStructHelper();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.pNext = &h265_ci;
    create_info.videoSession = context.Session();

    h265_ci.maxStdVPSCount = 2;
    h265_ci.maxStdSPSCount = 5;
    h265_ci.maxStdPPSCount = 9;
    h265_ci.pParametersAddInfo = &h265_ai;

    VkVideoSessionParametersUpdateInfoKHR update_info = vku::InitStructHelper();
    update_info.pNext = &h265_ai;
    update_info.updateSequenceCount = 1;

    std::vector<StdVideoH265VideoParameterSet> vps_list{
        CreateH265VPS(1),
        CreateH265VPS(2),
    };

    std::vector<StdVideoH265SequenceParameterSet> sps_list{
        CreateH265SPS(1, 1),
        CreateH265SPS(1, 2),
        CreateH265SPS(2, 1),
        CreateH265SPS(2, 3),
    };

    std::vector<StdVideoH265PictureParameterSet> pps_list{
        CreateH265PPS(1, 1, 1), CreateH265PPS(1, 1, 2), CreateH265PPS(1, 2, 1), CreateH265PPS(2, 1, 3),
        CreateH265PPS(2, 3, 1), CreateH265PPS(2, 3, 2), CreateH265PPS(2, 3, 3),
    };

    h265_ai.stdVPSCount = (uint32_t)vps_list.size();
    h265_ai.pStdVPSs = vps_list.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list.size();
    h265_ai.pStdSPSs = sps_list.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list.size();
    h265_ai.pStdPPSs = pps_list.data();

    ASSERT_EQ(VK_SUCCESS, context.vk.CreateVideoSessionParametersKHR(m_device->device(), &create_info, nullptr, &params));

    std::vector<StdVideoH265VideoParameterSet> vps_list2{CreateH265VPS(3)};

    std::vector<StdVideoH265SequenceParameterSet> sps_list2{CreateH265SPS(2, 2), CreateH265SPS(3, 1)};

    std::vector<StdVideoH265PictureParameterSet> pps_list2{CreateH265PPS(1, 2, 3), CreateH265PPS(2, 3, 4), CreateH265PPS(3, 1, 2)};

    h265_ai.stdVPSCount = (uint32_t)vps_list2.size();
    h265_ai.pStdVPSs = vps_list2.data();
    h265_ai.stdSPSCount = (uint32_t)sps_list2.size();
    h265_ai.pStdSPSs = sps_list2.data();
    h265_ai.stdPPSCount = (uint32_t)pps_list2.size();
    h265_ai.pStdPPSs = pps_list2.data();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07221");
    h265_ai.stdVPSCount = 1;
    h265_ai.stdSPSCount = 1;
    h265_ai.stdPPSCount = 2;
    context.vk.UpdateVideoSessionParametersKHR(m_device->device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07223");
    h265_ai.stdVPSCount = 0;
    h265_ai.stdSPSCount = 2;
    h265_ai.stdPPSCount = 2;
    context.vk.UpdateVideoSessionParametersKHR(m_device->device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07225");
    h265_ai.stdVPSCount = 0;
    h265_ai.stdSPSCount = 1;
    h265_ai.stdPPSCount = 3;
    context.vk.UpdateVideoSessionParametersKHR(m_device->device(), params, &update_info);
    m_errorMonitor->VerifyFound();

    context.vk.DestroyVideoSessionParametersKHR(m_device->device(), params, nullptr);
}

TEST_F(VkVideoLayerTest, BeginCodingUnsupportedCodecOp) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - unsupported video codec operation");

    RETURN_IF_SKIP(Init())

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

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();

    vkt::CommandPool pool(*DeviceObj(), queue_family_index);
    vkt::CommandBuffer cb(DeviceObj(), &pool);

    cb.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07231");
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->VerifyFound();

    cb.end();
}

TEST_F(VkVideoLayerTest, BeginCodingActiveQueriesNotAllowed) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - there must be no active query");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateStatusQueryPool();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 0, 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginVideoCodingKHR-None-07232");
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->VerifyFound();
    vk::CmdEndQuery(cb.handle(), context.StatusQueryPool(), 0);
    cb.end();
}

TEST_F(VkVideoLayerTest, BeginCodingProtectedNoFaultSession) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - protectedNoFault tests for video session");

    const bool use_protected = true;
    RETURN_IF_SKIP(Init(use_protected));
    if (!IsProtectedMemoryEnabled() || IsProtectedNoFaultSupported()) {
        GTEST_SKIP() << "Test requires protectedMemory support without protectedNoFault support";
    }

    VideoConfig config = GetConfigWithProtectedContent(GetConfigs());
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with protected content support";
    }

    VideoContext unprotected_context(DeviceObj(), config);
    unprotected_context.CreateAndBindSessionMemory();
    unprotected_context.CreateResources();

    vkt::CommandBuffer& unprotected_cb = unprotected_context.CmdBuffer();

    VideoContext protected_context(DeviceObj(), config, use_protected);
    protected_context.CreateAndBindSessionMemory();
    protected_context.CreateResources(use_protected);

    vkt::CommandBuffer& protected_cb = protected_context.CmdBuffer();

    unprotected_cb.begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07233");
    unprotected_cb.BeginVideoCoding(protected_context.Begin());
    m_errorMonitor->VerifyFound();
    unprotected_cb.end();

    protected_cb.begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07234");
    protected_cb.BeginVideoCoding(unprotected_context.Begin());
    m_errorMonitor->VerifyFound();
    protected_cb.end();
}

TEST_F(VkVideoLayerTest, BeginCodingProtectedNoFaultSlots) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - protectedNoFault tests for reference slots");

    const bool use_protected = true;
    RETURN_IF_SKIP(Init(use_protected));
    if (!IsProtectedMemoryEnabled() || IsProtectedNoFaultSupported()) {
        GTEST_SKIP() << "Test requires protectedMemory support without protectedNoFault support";
    }

    VideoConfig config = GetConfigWithProtectedContent(GetConfigsWithReferences(GetConfigs()));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with protected content and reference picture support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext unprotected_context(DeviceObj(), config);
    unprotected_context.CreateAndBindSessionMemory();
    unprotected_context.CreateResources(use_protected);

    vkt::CommandBuffer& unprotected_cb = unprotected_context.CmdBuffer();

    VideoContext protected_context(DeviceObj(), config, use_protected);
    protected_context.CreateAndBindSessionMemory();
    protected_context.CreateResources();

    vkt::CommandBuffer& protected_cb = protected_context.CmdBuffer();

    unprotected_cb.begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07235");
    unprotected_cb.BeginVideoCoding(unprotected_context.Begin().AddResource(-1, 0));
    m_errorMonitor->VerifyFound();
    unprotected_cb.end();

    protected_cb.begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07236");
    protected_cb.BeginVideoCoding(protected_context.Begin().AddResource(-1, 0));
    m_errorMonitor->VerifyFound();
    protected_cb.end();
}

TEST_F(VkVideoLayerTest, BeginCodingSessionMemoryNotBound) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - session uninitialized");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(DeviceObj(), config);

    uint32_t mem_req_count;
    context.vk.GetVideoSessionMemoryRequirementsKHR(m_device->device(), context.Session(), &mem_req_count, nullptr);
    if (mem_req_count == 0) {
        GTEST_SKIP() << "Test requires video session to need memory bindings";
    }

    std::vector<VkVideoSessionMemoryRequirementsKHR> mem_reqs(mem_req_count, vku::InitStruct<VkVideoSessionMemoryRequirementsKHR>());
    context.vk.GetVideoSessionMemoryRequirementsKHR(m_device->device(), context.Session(), &mem_req_count, mem_reqs.data());

    std::vector<VkDeviceMemory> session_memory;
    for (uint32_t i = 0; i < mem_req_count; ++i) {
        // Skip binding one of the memory bindings
        if (i == mem_req_count / 2) continue;

        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
        m_device->phy().set_memory_type(mem_reqs[i].memoryRequirements.memoryTypeBits, &alloc_info, 0);
        alloc_info.allocationSize = mem_reqs[i].memoryRequirements.size;

        VkDeviceMemory memory = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(m_device->device(), &alloc_info, nullptr, &memory));
        session_memory.push_back(memory);

        VkBindVideoSessionMemoryInfoKHR bind_info = vku::InitStructHelper();
        bind_info.memoryBindIndex = mem_reqs[i].memoryBindIndex;
        bind_info.memory = memory;
        bind_info.memoryOffset = 0;
        bind_info.memorySize = mem_reqs[i].memoryRequirements.size;

        ASSERT_EQ(VK_SUCCESS, context.vk.BindVideoSessionMemoryKHR(m_device->device(), context.Session(), 1, &bind_info));
    }

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoBeginCodingInfoKHR-videoSession-07237");
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->VerifyFound();

    cb.end();

    for (auto memory : session_memory) {
        vk::FreeMemory(m_device->device(), memory, nullptr);
    }
}

TEST_F(VkVideoLayerTest, BeginCodingSessionUninitialized) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - session uninitialized");

    RETURN_IF_SKIP(Init())

    // NOTE: Once encode validation implementation is ready, this should be converted to
    // a test using an encode session, as, with the decode extensions only, there's no
    // flag that can be legally specified other than VK_VIDEO_CODING_CONTROL_RESET_BIT_KHR,
    // but that also means there's no other way to cover VU 07017 with tests before encode
    // is ready, hence the "illegal" approach of using an encode-specific flag is used here.
    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoCodingControlInfoKHR control_info = vku::InitStructHelper();
    control_info.flags = VK_VIDEO_CODING_CONTROL_ENCODE_RATE_CONTROL_BIT_KHR;

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(control_info);
    cb.EndVideoCoding(context.End());
    cb.end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdControlVideoCodingKHR-flags-07017");
    context.Queue().submit(cb, false);
    m_errorMonitor->VerifyFound();

    m_device->wait();
}

TEST_F(VkVideoLayerTest, BeginCodingInvalidSessionParams) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - invalid session parameters");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfigWithParams(GetConfigs());
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with session parameters";
    }

    VideoContext context1(DeviceObj(), config);
    VideoContext context2(DeviceObj(), config);
    vkt::CommandBuffer& cb = context1.CmdBuffer();

    context1.CreateAndBindSessionMemory();

    VkVideoBeginCodingInfoKHR beginInfo = context1.Begin();
    beginInfo.videoSessionParameters = context2.SessionParams();

    cb.begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoBeginCodingInfoKHR-videoSessionParameters-04857");
    cb.BeginVideoCoding(beginInfo);
    m_errorMonitor->VerifyFound();
    cb.end();
}

TEST_F(VkVideoLayerTest, BeginCodingDecodeH264RequiresParams) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - H.264 decode requires session parameters");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfigDecodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoBeginCodingInfoKHR beginInfo = context.Begin();
    beginInfo.videoSessionParameters = VK_NULL_HANDLE;

    cb.begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoBeginCodingInfoKHR-videoSession-07247");
    cb.BeginVideoCoding(beginInfo);
    m_errorMonitor->VerifyFound();
    cb.end();
}

TEST_F(VkVideoLayerTest, BeginCodingDecodeH265RequiresParams) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - H.265 decode requires session parameters");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfigDecodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoBeginCodingInfoKHR beginInfo = context.Begin();
    beginInfo.videoSessionParameters = VK_NULL_HANDLE;

    cb.begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoBeginCodingInfoKHR-videoSession-07248");
    cb.BeginVideoCoding(beginInfo);
    m_errorMonitor->VerifyFound();
    cb.end();
}

TEST_F(VkVideoLayerTest, BeginCodingIncompatRefPicProfile) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - reference picture must be compatible with the video profile");

    RETURN_IF_SKIP(Init())

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

    VideoContext context1(DeviceObj(), configs[0]);
    VideoContext context2(DeviceObj(), configs[1]);
    context1.CreateAndBindSessionMemory();
    context1.CreateResources();
    context2.CreateAndBindSessionMemory();
    context2.CreateResources();

    vkt::CommandBuffer& cb = context1.CmdBuffer();

    cb.begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07240");
    cb.BeginVideoCoding(context1.Begin().AddResource(-1, context2.Dpb()->Picture(0)));
    m_errorMonitor->VerifyFound();
    cb.end();
}

TEST_F(VkVideoLayerTest, BeginCodingInvalidResourceLayer) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - out-of-bounds layer index in VkVideoPictureResourceInfoKHR");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigs()));
    if (!config) {
        GTEST_SKIP() << "Test requires video support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();

    // Invalid baseArrayLayer in VkVideoBeginCodingInfoKHR::pReferenceSlots
    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);
    res.baseArrayLayer = 5;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    cb.end();
}

TEST_F(VkVideoLayerTest, BeginCodingDecodeSlotInactive) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - referenced DPB slot is inactive");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires a video decode profile with reference picture support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.EndVideoCoding(context.End());
    cb.end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
    context.Queue().submit(cb, false);
    m_errorMonitor->VerifyFound();
    m_device->wait();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
    cb.ControlVideoCoding(context.Control().Reset());
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, 0, 1));
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(0));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().submit(cb);
    m_device->wait();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().InvalidateSlot(0));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().submit(cb);
    m_device->wait();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.EndVideoCoding(context.End());
    cb.end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
    context.Queue().submit(cb, false);
    m_errorMonitor->VerifyFound();
    m_device->wait();
}

TEST_F(VkVideoLayerTest, BeginCodingDecodeInvalidSlotResourceAssociation) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - referenced DPB slot is not associated with the specified resource");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires a video decode profile with reference picture support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
    cb.ControlVideoCoding(context.Control().Reset());
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, 0, 1));
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(0));
    cb.EndVideoCoding(context.End());

    cb.BeginVideoCoding(context.Begin().AddResource(0, 1));
    cb.EndVideoCoding(context.End());
    cb.end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginVideoCodingKHR-pPictureResource-07265");
    context.Queue().submit(cb, false);
    m_errorMonitor->VerifyFound();
    m_device->wait();
}

TEST_F(VkVideoLayerTest, BeginCodingInvalidSlotIndex) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - referenced DPB slot index is invalid");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigs(), 4));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with support for 4 reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 3;
    config.SessionCreateInfo()->maxActiveReferencePictures = 3;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoBeginCodingInfoKHR-slotIndex-04856");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(3, 1).AddResource(-1, 2));
    m_errorMonitor->VerifyFound();

    cb.end();
}

TEST_F(VkVideoLayerTest, BeginCodingResourcesNotUnique) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - referenced video picture resources are not unique");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigs(), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with support for 2 reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07238");
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 0));
    m_errorMonitor->VerifyFound();

    cb.end();
}

TEST_F(VkVideoLayerTest, BeginCodingReferenceFormatMismatch) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - reference picture format mismatch");

    RETURN_IF_SKIP(Init())

    uint32_t alt_ref_format_index = 0;
    VideoConfig config =
        GetConfig(FilterConfigs(GetConfigsWithReferences(GetConfigs()), [&alt_ref_format_index](const VideoConfig& config) {
            const auto& format_props = config.SupportedDpbFormatProps();
            for (size_t i = 0; i < format_props.size(); ++i) {
                if (format_props[i].format != format_props[0].format) {
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

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoConfig config2 = config;
    config2.SetFormatProps(config.SupportedPictureFormatProps(), {config.SupportedDpbFormatProps()[alt_ref_format_index]});
    VideoDPB dpb(m_device, config2);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07241");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, dpb.Picture(0)));
    m_errorMonitor->VerifyFound();

    cb.end();
}

TEST_F(VkVideoLayerTest, BeginCodingInvalidCodedOffset) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - invalid coded offset");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigs()));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with reference picture support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);

    cb.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07242");
    res.codedOffset.x = 5;
    res.codedOffset.y = 0;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07242");
    res.codedOffset.x = 0;
    res.codedOffset.y = 4;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    cb.end();
}

TEST_F(VkVideoLayerTest, BeginCodingInvalidCodedExtent) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - invalid coded extent");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigs()));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with reference picture support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);

    cb.begin();

    res.codedExtent.width = config.Caps()->minCodedExtent.width;
    res.codedExtent.height = config.Caps()->minCodedExtent.height;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    cb.EndVideoCoding(context.End());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07243");
    res.codedExtent.width = config.Caps()->minCodedExtent.width - 1;
    res.codedExtent.height = config.Caps()->minCodedExtent.height;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07243");
    res.codedExtent.width = config.Caps()->minCodedExtent.width;
    res.codedExtent.height = config.Caps()->minCodedExtent.height - 1;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    res.codedExtent.width = config.SessionCreateInfo()->maxCodedExtent.width;
    res.codedExtent.height = config.SessionCreateInfo()->maxCodedExtent.height;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    cb.EndVideoCoding(context.End());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07243");
    res.codedExtent.width = config.SessionCreateInfo()->maxCodedExtent.width + 1;
    res.codedExtent.height = config.SessionCreateInfo()->maxCodedExtent.height;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07243");
    res.codedExtent.width = config.SessionCreateInfo()->maxCodedExtent.width;
    res.codedExtent.height = config.SessionCreateInfo()->maxCodedExtent.height + 1;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    cb.end();
}

TEST_F(VkVideoLayerTest, BeginCodingInvalidSeparateReferenceImages) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - unsupported use of separate reference images");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigs(), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with reference picture support";
    }

    if (config.Caps()->flags & VK_VIDEO_CAPABILITY_SEPARATE_REFERENCE_IMAGES_BIT_KHR) {
        GTEST_SKIP() << "This test can only run on implementations with no support for separate reference images";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoDPB separate_dpb(DeviceObj(), config);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoBeginCodingInfoKHR-flags-07244");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, context.Dpb()->Picture(0)).AddResource(-1, separate_dpb.Picture(1)));
    m_errorMonitor->VerifyFound();

    cb.end();
}

TEST_F(VkVideoLayerTest, BeginCodingMissingDecodeDpbUsage) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - reference picture resource missing VIDEO_DECODE_DPB usage");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecode(), 1));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with reference picture support";
    }

    if (config.DpbFormatProps()->imageUsageFlags == VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR) {
        GTEST_SKIP() << "Test requires reference format to support at least one more usage besides DECODE_DPB";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);

    VkImageViewUsageCreateInfo view_usage_ci = vku::InitStructHelper();
    view_usage_ci.usage = config.DpbFormatProps()->imageUsageFlags ^ VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR;

    VkImageViewCreateInfo image_view_ci = vku::InitStructHelper(&view_usage_ci);
    image_view_ci.image = context.Dpb()->Image();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    image_view_ci.format = config.DpbFormatProps()->format;
    image_view_ci.components = config.DpbFormatProps()->componentMapping;
    image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    vkt::ImageView image_view(*m_device, image_view_ci);

    res.imageViewBinding = image_view;

    cb.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoBeginCodingInfoKHR-slotIndex-07245");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    cb.end();
}

TEST_F(VkVideoLayerTest, EndCodingActiveQueriesNotAllowed) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - there must be no active query");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateStatusQueryPool();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 0, 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndVideoCodingKHR-None-07251");
    cb.EndVideoCoding(context.End());
    m_errorMonitor->VerifyFound();
    vk::CmdEndQuery(cb.handle(), context.StatusQueryPool(), 0);
    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeSessionUninitialized) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - session uninitialized");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    cb.DecodeVideo(context.DecodeFrame());
    cb.EndVideoCoding(context.End());
    cb.end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-None-07011");
    context.Queue().submit(cb, false);
    m_errorMonitor->VerifyFound();
    m_device->wait();
}

TEST_F(VkVideoLayerTest, DecodeProtectedNoFaultBitstreamBuffer) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - protectedNoFault tests for bitstream buffer");

    const bool use_protected = true;
    RETURN_IF_SKIP(Init(use_protected));
    if (!IsProtectedMemoryEnabled() || IsProtectedNoFaultSupported()) {
        GTEST_SKIP() << "Test requires protectedMemory support without protectedNoFault support";
    }

    VideoConfig config = GetConfigWithProtectedContent(GetConfigsDecode());
    if (!config) {
        GTEST_SKIP() << "Test requires a video decode profile with protected content support";
    }

    VideoContext unprotected_context(DeviceObj(), config);
    unprotected_context.CreateAndBindSessionMemory();
    unprotected_context.CreateResources();

    vkt::CommandBuffer& unprotected_cb = unprotected_context.CmdBuffer();

    VideoContext protected_context(DeviceObj(), config, use_protected);
    protected_context.CreateAndBindSessionMemory();
    protected_context.CreateResources(use_protected);

    vkt::CommandBuffer& protected_cb = protected_context.CmdBuffer();

    unprotected_cb.begin();
    unprotected_cb.BeginVideoCoding(unprotected_context.Begin());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-commandBuffer-07136");
    unprotected_cb.DecodeVideo(unprotected_context.DecodeFrame().SetBitstream(protected_context.Bitstream()));
    m_errorMonitor->VerifyFound();

    unprotected_cb.EndVideoCoding(unprotected_context.End());
    unprotected_cb.end();

    protected_cb.begin();
    protected_cb.BeginVideoCoding(protected_context.Begin());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-commandBuffer-07137");
    protected_cb.DecodeVideo(protected_context.DecodeFrame().SetBitstream(unprotected_context.Bitstream()));
    m_errorMonitor->VerifyFound();

    protected_cb.EndVideoCoding(protected_context.End());
    protected_cb.end();
}

TEST_F(VkVideoLayerTest, DecodeProtectedNoFaultDecodeOutput) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - protectedNoFault tests for bitstream buffer");

    const bool use_protected = true;
    RETURN_IF_SKIP(Init(use_protected));
    if (!IsProtectedMemoryEnabled() || IsProtectedNoFaultSupported()) {
        GTEST_SKIP() << "Test requires protectedMemory support without protectedNoFault support";
    }

    VideoConfig config = GetConfigWithProtectedContent(GetConfigsDecode());
    if (!config) {
        GTEST_SKIP() << "Test requires a video decode profile with protected content support";
    }

    VideoContext unprotected_context(DeviceObj(), config);
    unprotected_context.CreateAndBindSessionMemory();
    unprotected_context.CreateResources();

    vkt::CommandBuffer& unprotected_cb = unprotected_context.CmdBuffer();

    VideoContext protected_context(DeviceObj(), config, use_protected);
    protected_context.CreateAndBindSessionMemory();
    protected_context.CreateResources(use_protected);

    vkt::CommandBuffer& protected_cb = protected_context.CmdBuffer();

    unprotected_cb.begin();
    unprotected_cb.BeginVideoCoding(unprotected_context.Begin());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-commandBuffer-07147");
    unprotected_cb.DecodeVideo(unprotected_context.DecodeFrame().SetDecodeOutput(protected_context.DecodeOutput()));
    m_errorMonitor->VerifyFound();

    unprotected_cb.EndVideoCoding(unprotected_context.End());
    unprotected_cb.end();

    protected_cb.begin();
    protected_cb.BeginVideoCoding(protected_context.Begin());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-commandBuffer-07148");
    protected_cb.DecodeVideo(protected_context.DecodeFrame().SetDecodeOutput(unprotected_context.DecodeOutput()));
    m_errorMonitor->VerifyFound();

    protected_cb.EndVideoCoding(protected_context.End());
    protected_cb.end();
}

TEST_F(VkVideoLayerTest, DecodeImageLayouts) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - pictures should be in the expected layout");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires a video decode profile with reference picture support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));

    cb.DecodeVideo(context.DecodeFrame());

    // Decode output must be in DECODE_DST layout if there is no reconstructed
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07252");
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_GENERAL));
    cb.DecodeVideo(context.DecodeFrame());
    m_errorMonitor->VerifyFound();

    // Decode output must be in DECODE_DST layout if it is distinct from reconstructed
    if (config.SupportsDecodeOutputDistinct()) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07252");
        cb.DecodeVideo(context.DecodeFrame().SetupFrame(0));
        m_errorMonitor->VerifyFound();
        vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, 0, 1));
    }

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));

    // Decode output must be in DECODE_DPB layout if it coincides with reconstructed
    if (config.SupportsDecodeOutputCoincide()) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07254");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07253");
        vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR, 0, 1));
        cb.DecodeVideo(context.DecodeFrame().SetupFrame(0, true /* force coincide */));
        m_errorMonitor->VerifyFound();
    }

    // Reconstructed must be in DECODE_DPB layout
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07253");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07254");
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_GENERAL, 0, 1));
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(0));
    m_errorMonitor->VerifyFound();
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, 0, 1));

    // Reference must be in DECODE_DPB layout
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(0));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_GENERAL, 0, 1));
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pPictureResource-07255");
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeInvalidResourceLayer) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - out-of-bounds layer index in VkVideoPictureResourceInfoKHR");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoPictureResourceInfoKHR dpb_res = context.Dpb()->Picture(0);
    dpb_res.baseArrayLayer = 5;

    VkVideoPictureResourceInfoKHR dst_res = context.DecodeOutput()->Picture();
    dst_res.baseArrayLayer = 5;

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));

    // Invalid baseArrayLayer in VkVideoDecodeInfoKHR::dstPictureResource
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175");
    cb.DecodeVideo(context.DecodeFrame().SetDecodeOutput(dst_res));
    m_errorMonitor->VerifyFound();

    // Invalid baseArrayLayer in VkVideoDecodeInfoKHR::pSetupReferenceSlot
    if (config.SupportsDecodeOutputDistinct()) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07149");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175");
        cb.DecodeVideo(context.DecodeFrame().SetupFrame(0, &dpb_res));
        m_errorMonitor->VerifyFound();
    } else {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07149");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175");
        cb.DecodeVideo(context.DecodeFrame().SetDecodeOutput(dst_res).SetupFrame(0, &dpb_res));
        m_errorMonitor->VerifyFound();
    }

    // Invalid baseArrayLayer in VkVideoDecodeInfoKHR::pReferenceSlots
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-slotIndex-07256");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175");
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(0, &dpb_res));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeQueryTooManyOperations) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - no more queries available to store operation results");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video decode queue to support result status queries";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();
    context.CreateStatusQueryPool(2);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 0, 0);
    cb.DecodeVideo(context.DecodeFrame());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-opCount-07134");
    cb.DecodeVideo(context.DecodeFrame());
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(cb.handle(), context.StatusQueryPool(), 0);
    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeIncompatBufferProfile) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - bitstream buffer must be compatible with the video profile");

    RETURN_IF_SKIP(Init())

    auto configs = GetConfigsDecode();
    if (configs.size() < 2) {
        GTEST_SKIP() << "Test requires two video decode profiles";
    }

    VideoContext context1(DeviceObj(), configs[0]);
    VideoContext context2(DeviceObj(), configs[1]);
    context1.CreateAndBindSessionMemory();
    context1.CreateResources();
    context2.CreateAndBindSessionMemory();
    context2.CreateResources();

    vkt::CommandBuffer& cb = context1.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07135");
    cb.DecodeVideo(context1.DecodeFrame().SetBitstream(context2.Bitstream()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeBufferMissingDecodeSrcUsage) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - bitstream buffer missing DECODE_SRC usage");

    RETURN_IF_SKIP(Init())

    auto config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
    profile_list.profileCount = 1;
    profile_list.pProfiles = config.Profile();

    VkBufferCreateInfo create_info = vku::InitStructHelper();
    create_info.flags = 0;
    create_info.pNext = &profile_list;
    create_info.size = std::max((VkDeviceSize)4096, config.Caps()->minBitstreamBufferSizeAlignment);
    create_info.usage = VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer buffer = VK_NULL_HANDLE;
    ASSERT_EQ(VK_SUCCESS, vk::CreateBuffer(m_device->device(), &create_info, nullptr, &buffer));

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoDecodeInfoKHR-srcBuffer-07165");
    cb.DecodeVideo(context.DecodeFrame().SetBitstreamBuffer(buffer, 0, create_info.size));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();

    vk::DestroyBuffer(m_device->device(), buffer, nullptr);
}
TEST_F(VkVideoLayerTest, DecodeBufferOffsetOutOfBounds) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - bitstream buffer offset out of bounds");

    RETURN_IF_SKIP(Init())

    auto config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    VideoDecodeInfo decode_info = context.DecodeFrame();
    decode_info->srcBufferOffset = decode_info->srcBufferRange;
    decode_info->srcBufferOffset += config.Caps()->minBitstreamBufferOffsetAlignment - 1;
    decode_info->srcBufferOffset &= ~(config.Caps()->minBitstreamBufferOffsetAlignment - 1);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoDecodeInfoKHR-srcBufferOffset-07166");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoDecodeInfoKHR-srcBufferRange-07167");
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeBufferOffsetAlignment) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - bitstream buffer offset needs to be aligned");

    RETURN_IF_SKIP(Init())

    auto config = GetConfig(FilterConfigs(
        GetConfigsDecode(), [](const VideoConfig& config) { return config.Caps()->minBitstreamBufferOffsetAlignment > 1; }));
    if (!config) {
        GTEST_SKIP() << "Test requires a video decode profile with minBitstreamBufferOffsetAlignment > 1";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    VideoDecodeInfo decode_info = context.DecodeFrame();
    ++decode_info->srcBufferOffset;
    decode_info->srcBufferRange -= config.Caps()->minBitstreamBufferSizeAlignment;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07138");
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeBufferRangeOutOfBounds) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - bitstream buffer range out of bounds");

    RETURN_IF_SKIP(Init())

    auto config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    VideoDecodeInfo decode_info = context.DecodeFrame();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoDecodeInfoKHR-srcBufferRange-07167");
    decode_info->srcBufferOffset += config.Caps()->minBitstreamBufferOffsetAlignment;
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoDecodeInfoKHR-srcBufferRange-07167");
    decode_info->srcBufferOffset = 0;
    decode_info->srcBufferRange += config.Caps()->minBitstreamBufferSizeAlignment;
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeBufferRangeAlignment) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - bitstream buffer range needs to be aligned");

    RETURN_IF_SKIP(Init())

    auto config = GetConfig(FilterConfigs(
        GetConfigsDecode(), [](const VideoConfig& config) { return config.Caps()->minBitstreamBufferSizeAlignment > 1; }));
    if (!config) {
        GTEST_SKIP() << "Test requires a video decode profile with minBitstreamBufferSizeAlignment > 1";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    VideoDecodeInfo decode_info = context.DecodeFrame();
    --decode_info->srcBufferRange;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07139");
    cb.DecodeVideo(decode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeInvalidOutputAndSetupCoincide) {
    TEST_DESCRIPTION(
        "vkCmdDecodeVideoKHR - decode output and reconstructed pictures must not match "
        "if VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR is not supported");

    RETURN_IF_SKIP(Init())

    auto config = GetConfig(FilterConfigs(GetConfigsWithReferences(GetConfigsDecode()),
                                          [](const VideoConfig& config) { return !config.SupportsDecodeOutputCoincide(); }));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures and no support "
                        "for VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07146");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07140");
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(0).SetDecodeOutput(context.Dpb()->Picture(0)));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeInvalidOutputAndSetupDistinct) {
    TEST_DESCRIPTION(
        "vkCmdDecodeVideoKHR - decode output and reconstructed pictures must match "
        "if VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR is not supported");

    RETURN_IF_SKIP(Init())

    auto config = GetConfig(FilterConfigs(GetConfigsWithReferences(GetConfigsDecode()),
                                          [](const VideoConfig& config) { return !config.SupportsDecodeOutputDistinct(); }));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures and no support "
                        "for VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07141");
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(0).SetDecodeOutput(context.DecodeOutput()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeInvalidSetupSlotIndex) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - invalid slot index specified for reconstructed picture");

    RETURN_IF_SKIP(Init())

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsDecode(), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with 3 reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoDecodeInfoKHR-pSetupReferenceSlot-07168");
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(-1, 0));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07170");
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(2, 0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeInvalidRefSlotIndex) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - invalid slot index specified for reference picture");

    RETURN_IF_SKIP(Init())

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsDecode(), 3));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with 3 reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoDecodeInfoKHR-slotIndex-07171");
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(-1, 0));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-slotIndex-07256");
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(2, 0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeSetupResourceNull) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - reconstructed picture resource must not be NULL");

    RETURN_IF_SKIP(Init())

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoDecodeInfoKHR-pSetupReferenceSlot-07169");
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(0, nullptr));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeReferenceResourceNull) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - reference picture resource must not be NULL");

    RETURN_IF_SKIP(Init())

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkVideoDecodeInfoKHR-pPictureResource-07172");
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(0, nullptr));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeIncompatOutputPicProfile) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - decode output picture must be compatible with the video profile");

    RETURN_IF_SKIP(Init())

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

    VideoContext context1(DeviceObj(), configs[0]);
    VideoContext context2(DeviceObj(), configs[1]);
    context1.CreateAndBindSessionMemory();
    context1.CreateResources();
    context2.CreateAndBindSessionMemory();
    context2.CreateResources();

    vkt::CommandBuffer& cb = context1.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07142");
    cb.DecodeVideo(context1.DecodeFrame().SetDecodeOutput(context2.DecodeOutput()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeOutputFormatMismatch) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - decode output picture format mismatch");

    RETURN_IF_SKIP(Init())

    uint32_t alt_pic_format_index = 0;
    VideoConfig config = GetConfig(FilterConfigs(GetConfigsDecode(), [&alt_pic_format_index](const VideoConfig& config) {
        const auto& format_props = config.SupportedPictureFormatProps();
        for (size_t i = 0; i < format_props.size(); ++i) {
            if (format_props[i].format != format_props[0].format) {
                alt_pic_format_index = i;
                return true;
            }
        }
        return false;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with support for two output picture formats";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoConfig config2 = config;
    config2.SetFormatProps({config.SupportedPictureFormatProps()[alt_pic_format_index]}, config.SupportedDpbFormatProps());
    VideoDecodeOutput output(m_device, config2);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07143");
    cb.DecodeVideo(context.DecodeFrame().SetDecodeOutput(output.Picture()));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeOutputMissingDecodeDstUsage) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - decode output picture resource missing VIDEO_DECODE_DST usage");

    RETURN_IF_SKIP(Init())

    auto config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    if (config.PictureFormatProps()->imageUsageFlags == VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR) {
        GTEST_SKIP() << "Test requires output format to support at least one more usage besides DECODE_DST";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkImageViewUsageCreateInfo view_usage_ci = vku::InitStructHelper();
    view_usage_ci.usage = config.PictureFormatProps()->imageUsageFlags ^ VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR;

    VkImageViewCreateInfo image_view_ci = vku::InitStructHelper(&view_usage_ci);
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07146");
    cb.DecodeVideo(context.DecodeFrame().SetDecodeOutput(dst_res));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeOutputCodedOffsetExtent) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - invalid decode output picture coded offset/extent");

    RETURN_IF_SKIP(Init())

    auto config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    VkVideoPictureResourceInfoKHR dst_res = context.DecodeOutput()->Picture();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07144");
    ++dst_res.codedOffset.x;
    cb.DecodeVideo(context.DecodeFrame().SetDecodeOutput(dst_res));
    --dst_res.codedOffset.x;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07144");
    ++dst_res.codedOffset.y;
    cb.DecodeVideo(context.DecodeFrame().SetDecodeOutput(dst_res));
    --dst_res.codedOffset.y;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07145");
    ++dst_res.codedExtent.width;
    cb.DecodeVideo(context.DecodeFrame().SetDecodeOutput(dst_res));
    --dst_res.codedExtent.width;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07145");
    ++dst_res.codedExtent.height;
    cb.DecodeVideo(context.DecodeFrame().SetDecodeOutput(dst_res));
    --dst_res.codedExtent.height;
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeSetupAndRefCodedOffset) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - invalid reconstructed/reference picture coded offset");

    RETURN_IF_SKIP(Init())

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));

    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);

    if (!config.SupportsDecodeOutputDistinct()) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07144");
    }
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07149");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07173");
    ++res.codedOffset.x;
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(0, &res));
    --res.codedOffset.x;
    m_errorMonitor->VerifyFound();

    if (!config.SupportsDecodeOutputDistinct()) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07144");
    }
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07149");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07173");
    ++res.codedOffset.y;
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(0, &res));
    --res.codedOffset.y;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-codedOffset-07257");
    ++res.codedOffset.x;
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(0, &res));
    --res.codedOffset.x;
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-codedOffset-07257");
    ++res.codedOffset.y;
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(0, &res));
    --res.codedOffset.y;
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeSetupResourceNotBound) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - used reconstructed picture resource is not bound");

    RETURN_IF_SKIP(Init())

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07149");
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeRefResourceNotBoundToDPBSlot) {
    TEST_DESCRIPTION(
        "vkCmdDecodeVideoKHR - used reference picture resource is not bound as a resource "
        "currently associated with the corresponding DPB slot");

    RETURN_IF_SKIP(Init())

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = config.Caps()->maxDpbSlots;
    config.SessionCreateInfo()->maxActiveReferencePictures = config.Caps()->maxActiveReferencePictures;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));

    // Trying to refer to reference picture resource that is not bound at all
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(1));
    m_errorMonitor->VerifyFound();

    // Trying to refer to bound reference picture resource, but with incorrect slot index
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(1, 0));
    m_errorMonitor->VerifyFound();

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, 0, 1));
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(1, 0));
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(1, 0));

    // Trying to refer to bound reference picture resource, but with incorrect slot index after
    // the associated DPB slot index has been updated within the video coding scope
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(0, 0));
    m_errorMonitor->VerifyFound();

    cb.ControlVideoCoding(context.Control().Reset());

    // Trying to refer to bound reference picture resource after reset
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(1, 0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeTooManyReferences) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - reference picture count exceeds maxActiveReferencePictures");

    RETURN_IF_SKIP(Init())

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-activeReferencePictureCount-07150");
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(0).AddReferenceFrame(1));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeTooManyReferencesH264Interlaced) {
    TEST_DESCRIPTION(
        "vkCmdDecodeVideoKHR - reference picture count exceeds maxActiveReferencePictures"
        " (specific test for H.264 interlaced with both top and bottom field referenced)");

    RETURN_IF_SKIP(Init())

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecodeH264Interlaced(), 2), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with 2 reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-activeReferencePictureCount-07150");
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(0).AddReferenceBothFields(1));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-activeReferencePictureCount-07150");
    cb.DecodeVideo(context.DecodeFrame().AddReferenceBothFields(1).AddReferenceFrame(0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeDuplicateRefResource) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - same reference picture resource is used twice");

    RETURN_IF_SKIP(Init())

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07264");
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(0, &res).AddReferenceFrame(1, &res));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-dpbFrameUseCount-07176");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07264");
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(0, &res).AddReferenceFrame(0, &res));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeDuplicateRefResourceH264Interlaced) {
    TEST_DESCRIPTION(
        "vkCmdDecodeVideoKHR - same reference picture resource is used twice "
        "with one referring to the top field and another referring to the bottom field");

    RETURN_IF_SKIP(Init())

    auto config = GetConfig(GetConfigsWithReferences(GetConfigsDecodeH264Interlaced(), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with 2 reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07264");
    cb.DecodeVideo(context.DecodeFrame().AddReferenceTopField(0, 0).AddReferenceBottomField(0, 0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeDuplicateFrame) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - same DPB frame reference is used twice");

    RETURN_IF_SKIP(Init())

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(0, 1));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-dpbFrameUseCount-07176");
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(0, 0).AddReferenceFrame(0, 1));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeDuplicateFrameFieldH264Interlaced) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - same DPB frame, top field, or bottom field reference is used twice");

    RETURN_IF_SKIP(Init())

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecodeH264Interlaced(), 4), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with 4 reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 4;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(0, 1));

    // Same DPB frame is used twice
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-dpbFrameUseCount-07176");
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(0, 0).AddReferenceFrame(0, 1));
    m_errorMonitor->VerifyFound();

    // Same DPB top field is used twice
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-dpbTopFieldUseCount-07177");
    cb.DecodeVideo(context.DecodeFrame().AddReferenceTopField(0, 0).AddReferenceTopField(0, 1));
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-dpbTopFieldUseCount-07177");
    cb.DecodeVideo(context.DecodeFrame().AddReferenceTopField(0, 0).AddReferenceBothFields(0, 1));
    m_errorMonitor->VerifyFound();

    // Same DPB bottom field is used twice
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-dpbBottomFieldUseCount-07178");
    cb.DecodeVideo(context.DecodeFrame().AddReferenceBottomField(0, 0).AddReferenceBottomField(0, 1));
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-dpbBottomFieldUseCount-07178");
    cb.DecodeVideo(context.DecodeFrame().AddReferenceBottomField(0, 0).AddReferenceBothFields(0, 1));
    m_errorMonitor->VerifyFound();

    // Same DPB top & bottom field is used twice
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-dpbTopFieldUseCount-07177");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-dpbBottomFieldUseCount-07178");
    cb.DecodeVideo(context.DecodeFrame().AddReferenceBothFields(0, 0).AddReferenceBothFields(0, 1));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeRefPictureKindMismatch) {
    TEST_DESCRIPTION(
        "vkCmdDecodeVideoKHR - reference picture kind (frame, top field, bottom field) mismatch "
        "between actual DPB slot contents and specified reference pictures");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecodeH264Interlaced()));
    if (!config) {
        GTEST_SKIP() << "Test requires an H.264 interlaced decode profile with reference picture support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    // Setup frame in DPB slot
    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(0));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().submit(cb);
    m_device->wait();

    // Try to reference DPB slot as top field
    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.DecodeVideo(context.DecodeFrame().AddReferenceTopField(0));
    cb.EndVideoCoding(context.End());
    cb.end();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07267");
    context.Queue().submit(cb, false);
    m_errorMonitor->VerifyFound();
    m_device->wait();

    // Try to reference DPB slot as bottom field
    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.DecodeVideo(context.DecodeFrame().AddReferenceBottomField(0));
    cb.EndVideoCoding(context.End());
    cb.end();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07268");
    context.Queue().submit(cb, false);
    m_errorMonitor->VerifyFound();
    m_device->wait();

    // Setup top field in DPB slot
    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.DecodeVideo(context.DecodeTopField().SetupTopField(0));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().submit(cb);
    m_device->wait();

    // Try to reference DPB slot as frame
    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(0));
    cb.EndVideoCoding(context.End());
    cb.end();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07266");
    context.Queue().submit(cb, false);
    m_errorMonitor->VerifyFound();
    m_device->wait();

    // Try to reference DPB slot as bottom field
    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.DecodeVideo(context.DecodeFrame().AddReferenceBottomField(0));
    cb.EndVideoCoding(context.End());
    cb.end();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07268");
    context.Queue().submit(cb, false);
    m_errorMonitor->VerifyFound();
    m_device->wait();

    // Setup bottom field in DPB slot
    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.DecodeVideo(context.DecodeBottomField().SetupBottomField(0));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().submit(cb);
    m_device->wait();

    // Try to reference DPB slot as frame
    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(0));
    cb.EndVideoCoding(context.End());
    cb.end();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07266");
    context.Queue().submit(cb, false);
    m_errorMonitor->VerifyFound();
    m_device->wait();

    // Try to reference DPB slot as top field
    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.DecodeVideo(context.DecodeFrame().AddReferenceTopField(0));
    cb.EndVideoCoding(context.End());
    cb.end();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07267");
    context.Queue().submit(cb, false);
    m_errorMonitor->VerifyFound();
    m_device->wait();
}

TEST_F(VkVideoLayerTest, DecodeInvalidCodecInfoH264) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - invalid/missing H.264 codec-specific information");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecodeH264(), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VideoDecodeInfo decode_info = context.DecodeFrame();

    StdVideoDecodeH264PictureInfo std_picture_info{};
    VkVideoDecodeH264PictureInfoKHR picture_info = vku::InitStructHelper();
    uint32_t slice_offset = 0;
    picture_info.pStdPictureInfo = &std_picture_info;
    picture_info.sliceCount = 1;
    picture_info.pSliceOffsets = &slice_offset;

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));

    // Missing H.264 picture info
    {
        decode_info = context.DecodeFrame();
        decode_info->pNext = nullptr;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pNext-07152");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Decode output must be a frame if session does not support interlaced frames
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-None-07258");
        cb.DecodeVideo(context.DecodeTopField());
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-None-07258");
        cb.DecodeVideo(context.DecodeBottomField());
        m_errorMonitor->VerifyFound();
    }

    // Slice offsets must be within buffer range
    {
        decode_info = context.DecodeFrame();
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pSliceOffsets-07153");
        slice_offset = (uint32_t)decode_info->srcBufferRange;
        cb.DecodeVideo(decode_info);
        slice_offset = 0;
        m_errorMonitor->VerifyFound();
    }

    // No matching SPS/PPS
    {
        decode_info = context.DecodeFrame();
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-StdVideoH264SequenceParameterSet-07154");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-StdVideoH264PictureParameterSet-07155");
        std_picture_info.seq_parameter_set_id = 1;
        cb.DecodeVideo(decode_info);
        std_picture_info.seq_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-StdVideoH264PictureParameterSet-07155");
        std_picture_info.pic_parameter_set_id = 1;
        cb.DecodeVideo(decode_info);
        std_picture_info.pic_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();
    }

    // Missing H.264 setup reference info
    {
        VkVideoReferenceSlotInfoKHR slot = vku::InitStructHelper();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        decode_info = context.DecodeFrame();
        decode_info->pNext = &picture_info;
        decode_info->pSetupReferenceSlot = &slot;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07141");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07156");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Reconstructed picture must be a frame if session does not support interlaced frames
    {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07261");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07259");
        cb.DecodeVideo(context.DecodeFrame().SetupTopField(0));
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07261");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07259");
        cb.DecodeVideo(context.DecodeFrame().SetupBottomField(0));
        m_errorMonitor->VerifyFound();
    }

    // Missing H.264 reference info
    {
        VkVideoReferenceSlotInfoKHR slot = vku::InitStructHelper();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        decode_info = context.DecodeFrame();
        decode_info->pNext = &picture_info;
        decode_info->referenceSlotCount = 1;
        decode_info->pReferenceSlots = &slot;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pNext-07157");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Reference picture must be a frame if session does not support interlaced frames
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07260");
        cb.DecodeVideo(context.DecodeFrame().AddReferenceTopField(0));
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07260");
        cb.DecodeVideo(context.DecodeFrame().AddReferenceBottomField(0));
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07260");
        cb.DecodeVideo(context.DecodeFrame().AddReferenceBothFields(0));
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeFieldFrameMismatchH264) {
    TEST_DESCRIPTION(
        "vkCmdDecodeVideoKHR - H.264 interlaced field/frame mismatch between "
        "decode output picture and reconstructed picture");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecodeH264Interlaced()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 interlaced decode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    // Decode output is frame but reconstructed is top field
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07261");
    cb.DecodeVideo(context.DecodeFrame().SetupTopField(0));
    m_errorMonitor->VerifyFound();

    // Decode output is frame but reconstructed is bottom field
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07261");
    cb.DecodeVideo(context.DecodeFrame().SetupBottomField(0));
    m_errorMonitor->VerifyFound();

    // Decode output is top field but reconstructed is frame
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07262");
    cb.DecodeVideo(context.DecodeTopField().SetupFrame(0));
    m_errorMonitor->VerifyFound();

    // Decode output is top field but reconstructed is bottom field
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07262");
    cb.DecodeVideo(context.DecodeTopField().SetupBottomField(0));
    m_errorMonitor->VerifyFound();

    // Decode output is bottom field but reconstructed is frame
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07263");
    cb.DecodeVideo(context.DecodeBottomField().SetupFrame(0));
    m_errorMonitor->VerifyFound();

    // Decode output is bottom field but reconstructed is top field
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07263");
    cb.DecodeVideo(context.DecodeBottomField().SetupTopField(0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, DecodeInvalidCodecInfoH265) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - invalid/missing H.265 codec-specific information");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecodeH265()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VideoDecodeInfo decode_info = context.DecodeFrame();

    StdVideoDecodeH265PictureInfo std_picture_info{};
    VkVideoDecodeH265PictureInfoKHR picture_info = vku::InitStructHelper();
    uint32_t slice_segment_offset = 0;
    picture_info.pStdPictureInfo = &std_picture_info;
    picture_info.sliceSegmentCount = 1;
    picture_info.pSliceSegmentOffsets = &slice_segment_offset;

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));

    // Missing H.265 picture info
    {
        decode_info = context.DecodeFrame();
        decode_info->pNext = nullptr;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pNext-07158");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Slice offsets must be within buffer range
    {
        decode_info = context.DecodeFrame();
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pSliceSegmentOffsets-07159");
        slice_segment_offset = (uint32_t)decode_info->srcBufferRange;
        cb.DecodeVideo(decode_info);
        slice_segment_offset = 0;
        m_errorMonitor->VerifyFound();
    }

    // No matching VPS/SPS/PPS
    {
        decode_info = context.DecodeFrame();
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-StdVideoH265VideoParameterSet-07160");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-StdVideoH265SequenceParameterSet-07161");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-StdVideoH265PictureParameterSet-07162");
        std_picture_info.sps_video_parameter_set_id = 1;
        cb.DecodeVideo(decode_info);
        std_picture_info.sps_video_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-StdVideoH265SequenceParameterSet-07161");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-StdVideoH265PictureParameterSet-07162");
        std_picture_info.pps_seq_parameter_set_id = 1;
        cb.DecodeVideo(decode_info);
        std_picture_info.pps_seq_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-StdVideoH265PictureParameterSet-07162");
        std_picture_info.pps_pic_parameter_set_id = 1;
        cb.DecodeVideo(decode_info);
        std_picture_info.pps_pic_parameter_set_id = 0;
        m_errorMonitor->VerifyFound();
    }

    // Missing H.265 setup reference info
    {
        VkVideoReferenceSlotInfoKHR slot = vku::InitStructHelper();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        decode_info = context.DecodeFrame();
        decode_info->pNext = &picture_info;
        decode_info->pSetupReferenceSlot = &slot;

        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07141");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07163");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing H.265 reference info
    {
        VkVideoReferenceSlotInfoKHR slot = vku::InitStructHelper();
        slot.pNext = nullptr;
        slot.slotIndex = 0;
        slot.pPictureResource = &context.Dpb()->Picture(0);

        decode_info = context.DecodeFrame();
        decode_info->pNext = &picture_info;
        decode_info->referenceSlotCount = 1;
        decode_info->pReferenceSlots = &slot;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDecodeVideoKHR-pNext-07164");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, CreateBufferInvalidProfileList) {
    TEST_DESCRIPTION("vkCreateBuffer - invalid/missing profile list");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage = VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR;
    buffer_ci.size = 2048;
    CreateBufferTest(*this, &buffer_ci, "VUID-VkBufferCreateInfo-usage-04813");

    VkVideoProfileListInfoKHR video_profiles = vku::InitStructHelper();
    VkVideoProfileInfoKHR profiles[] = {*config.Profile(), *config.Profile()};
    video_profiles.profileCount = 2;
    video_profiles.pProfiles = profiles;
    buffer_ci.pNext = &video_profiles;
    CreateBufferTest(*this, &buffer_ci, "VUID-VkVideoProfileListInfoKHR-pProfiles-06813");

    video_profiles.profileCount = 1;
    video_profiles.pProfiles = config.Profile();

    VkBuffer buffer;
    vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
    vk::DestroyBuffer(device(), buffer, nullptr);

    buffer_ci.usage |= VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR;

    CreateBufferTest(*this, &buffer_ci, "VUID-VkBufferCreateInfo-usage-04814");
}

TEST_F(VkVideoLayerTest, CreateImageInvalidProfileList) {
    TEST_DESCRIPTION("vkCreateImage - invalid/missing profile list");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

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
    CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-usage-04815");

    VkVideoProfileListInfoKHR video_profiles = vku::InitStructHelper();
    VkVideoProfileInfoKHR profiles[] = {*config.Profile(), *config.Profile()};
    video_profiles.profileCount = 2;
    video_profiles.pProfiles = profiles;
    image_ci.pNext = &video_profiles;
    CreateImageTest(*this, &image_ci, "VUID-VkVideoProfileListInfoKHR-pProfiles-06813");
}

TEST_F(VkVideoLayerTest, CreateImageIncompatibleProfile) {
    TEST_DESCRIPTION("vkCreateImage - image parameters are incompatible with the profile");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(DeviceObj(), config);

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

    image_ci.format = VK_FORMAT_D16_UNORM;
    CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-pNext-06811");
}

TEST_F(VkVideoLayerTest, CreateImageViewInvalidViewType) {
    TEST_DESCRIPTION("vkCreateImageView - view type not compatible with video usage");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with reference picture support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;
    VideoContext context(DeviceObj(), config);

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

    vkt::Image image;
    image.init(*m_device, image_ci);

    VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
    image_view_ci.image = image.handle();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    image_view_ci.format = image_ci.format;
    image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_ci.subresourceRange.levelCount = 1;
    image_view_ci.subresourceRange.layerCount = 6;

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageViewCreateInfo-image-01003");
    CreateImageViewTest(*this, &image_view_ci, "VUID-VkImageViewCreateInfo-image-04817");
}

TEST_F(VkVideoLayerTest, BeginQueryIncompatibleQueueFamily) {
    TEST_DESCRIPTION("vkCmdBeginQuery - result status only queries require queue family support");

    RETURN_IF_SKIP(Init())

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

    VkQueryPoolCreateInfo create_info = vku::InitStructHelper();
    create_info.queryType = VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR;
    create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, create_info);

    vkt::CommandPool cmd_pool(*m_device, queue_family_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer cb(m_device, &cmd_pool);

    cb.begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-07126");
    vk::CmdBeginQuery(cb.handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->VerifyFound();

    cb.end();
}

TEST_F(VkVideoLayerTest, BeginQueryVideoCodingScopeQueryAlreadyActive) {
    TEST_DESCRIPTION("vkCmdBeginQuery - there must be no active query in video scope");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateStatusQueryPool(2);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 0, 0);

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginQuery-queryPool-01922");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-None-07127");
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 1, 0);
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(cb.handle(), context.StatusQueryPool(), 0);
    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, BeginQueryResultStatusProfileMismatch) {
    TEST_DESCRIPTION("vkCmdBeginQuery - result status query must have been created with the same profile");

    RETURN_IF_SKIP(Init())

    auto configs = GetConfigs();
    if (configs.size() < 2) {
        GTEST_SKIP() << "Test requires support for at least two video profiles";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(configs[0].QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    VideoContext context1(DeviceObj(), configs[0]);
    VideoContext context2(DeviceObj(), configs[1]);
    context1.CreateAndBindSessionMemory();
    context2.CreateStatusQueryPool();

    vkt::CommandBuffer& cb = context1.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-07128");
    vk::CmdBeginQuery(cb.handle(), context2.StatusQueryPool(), 0, 0);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, BeginQueryVideoCodingScopeIncompatibleQueryType) {
    TEST_DESCRIPTION("vkCmdBeginQuery - incompatible query type used in video coding scope");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();

    VkQueryPoolCreateInfo create_info = vku::InitStructHelper();
    create_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, create_info);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginQuery-queryType-00803");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginQuery-queryType-07128");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginQuery-queryType-07131");
    vk::CmdBeginQuery(cb.handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkVideoLayerTest, GetQueryPoolResultsStatusBit) {
    TEST_DESCRIPTION("vkGetQueryPoolResults - test invalid use of VK_QUERY_RESULT_WITH_STATUS_BIT_KHR");

    RETURN_IF_SKIP(Init())

    if (!GetConfig()) {
        GTEST_SKIP() << "Test requires video support";
    }

    VkQueryPoolCreateInfo create_info = vku::InitStructHelper();
    create_info.queryType = VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR;
    create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, create_info);

    uint32_t status;
    VkQueryResultFlags flags;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-queryType-04810");
    flags = 0;
    vk::GetQueryPoolResults(m_device->device(), query_pool.handle(), 0, 1, sizeof(status), &status, sizeof(status), flags);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetQueryPoolResults-flags-04811");
    flags = VK_QUERY_RESULT_WITH_STATUS_BIT_KHR | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT;
    vk::GetQueryPoolResults(m_device->device(), query_pool.handle(), 0, 1, sizeof(status), &status, sizeof(status), flags);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkVideoLayerTest, CopyQueryPoolResultsStatusBit) {
    TEST_DESCRIPTION("vkCmdCopyQueryPoolResults - test invalid use of VK_QUERY_RESULT_WITH_STATUS_BIT_KHR");

    RETURN_IF_SKIP(Init())

    if (!GetConfig()) {
        GTEST_SKIP() << "Test requires video support";
    }

    VkQueryPoolCreateInfo create_info = vku::InitStructHelper();
    create_info.queryType = VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR;
    create_info.queryCount = 1;
    vkt::QueryPool query_pool(*m_device, create_info);

    VkQueryResultFlags flags;

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = sizeof(uint32_t);
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vkt::Buffer buffer(*m_device, buffer_ci);

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-queryType-06901");
    flags = 0;
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool.handle(), 0, 1, buffer.handle(), 0, sizeof(uint32_t), flags);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyQueryPoolResults-flags-06902");
    flags = VK_QUERY_RESULT_WITH_STATUS_BIT_KHR | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT;
    vk::CmdCopyQueryPoolResults(m_commandBuffer->handle(), query_pool.handle(), 0, 1, buffer.handle(), 0, sizeof(uint32_t), flags);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkVideoLayerTest, ImageLayoutUsageMismatch) {
    TEST_DESCRIPTION("Image layout in image memory barrier is invalid for image usage");

    RETURN_IF_SKIP(Init())
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkImageMemoryBarrier image_barrier = vku::InitStructHelper();
    image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_barrier.subresourceRange.levelCount = 1;
    image_barrier.subresourceRange.layerCount = 1;

    VkImageMemoryBarrier2 image_barrier2 = vku::InitStructHelper();
    image_barrier2.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    image_barrier2.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    image_barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_barrier2.subresourceRange.levelCount = 1;
    image_barrier2.subresourceRange.layerCount = 1;

    VkDependencyInfo dep_info = vku::InitStructHelper();
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

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, params.vuid);
        image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_barrier.newLayout = params.invalid_layout;
        vk::CmdPipelineBarrier(cb.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                               0, nullptr, 1, &image_barrier);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, params.vuid);
        image_barrier.oldLayout = params.invalid_layout;
        image_barrier.newLayout = params.valid_layout;
        vk::CmdPipelineBarrier(cb.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
                               0, nullptr, 1, &image_barrier);
        m_errorMonitor->VerifyFound();

        image_barrier2.image = params.image;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, params.vuid2);
        image_barrier2.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_barrier2.newLayout = params.invalid_layout;
        vk::CmdPipelineBarrier2KHR(cb.handle(), &dep_info);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, params.vuid2);
        image_barrier2.oldLayout = params.invalid_layout;
        image_barrier2.newLayout = params.valid_layout;
        vk::CmdPipelineBarrier2KHR(cb.handle(), &dep_info);
        m_errorMonitor->VerifyFound();
    }

    cb.end();
}

TEST_F(VkVideoBestPracticesLayerTest, GetVideoSessionMemoryRequirements) {
    TEST_DESCRIPTION("vkGetVideoSessionMemoryRequirementsKHR - best practices");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(DeviceObj(), config);

    VkVideoSessionMemoryRequirementsKHR mem_req = vku::InitStructHelper();
    uint32_t mem_req_count = 1;

    m_errorMonitor->SetDesiredFailureMsg(kWarningBit,
                                         "UNASSIGNED-BestPractices-vkGetVideoSessionMemoryRequirementsKHR-count-not-retrieved");
    context.vk.GetVideoSessionMemoryRequirementsKHR(m_device->device(), context.Session(), &mem_req_count, &mem_req);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkVideoBestPracticesLayerTest, BindVideoSessionMemory) {
    TEST_DESCRIPTION("vkBindVideoSessionMemoryKHR - best practices");

    RETURN_IF_SKIP(Init())

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(DeviceObj(), config);

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
    ASSERT_TRUE(m_device->phy().set_memory_type(buf_mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    vkt::DeviceMemory memory(*m_device, alloc_info);

    // Set VkBindVideoSessionMemoryInfoKHR::memory to an allocation created before GetVideoSessionMemoryRequirementsKHR was called
    VkBindVideoSessionMemoryInfoKHR bind_info = vku::InitStructHelper();
    bind_info.memory = memory;
    bind_info.memoryOffset = 0;
    bind_info.memorySize = alloc_info.allocationSize;

    m_errorMonitor->SetDesiredFailureMsg(kWarningBit,
                                         "UNASSIGNED-BestPractices-vkBindVideoSessionMemoryKHR-requirements-count-not-retrieved");
    context.vk.BindVideoSessionMemoryKHR(m_device->device(), context.Session(), 1, &bind_info);
    m_errorMonitor->VerifyFound();

    uint32_t mem_req_count = 0;
    context.vk.GetVideoSessionMemoryRequirementsKHR(m_device->device(), context.Session(), &mem_req_count, nullptr);

    if (mem_req_count > 0) {
        m_errorMonitor->SetDesiredFailureMsg(kWarningBit,
                                             "UNASSIGNED-BestPractices-vkBindVideoSessionMemoryKHR-requirements-not-all-retrieved");
        context.vk.BindVideoSessionMemoryKHR(m_device->device(), context.Session(), 1, &bind_info);
        m_errorMonitor->VerifyFound();

        if (mem_req_count > 1) {
            VkVideoSessionMemoryRequirementsKHR mem_req = vku::InitStructHelper();
            mem_req_count = 1;

            context.vk.GetVideoSessionMemoryRequirementsKHR(m_device->device(), context.Session(), &mem_req_count, &mem_req);

            m_errorMonitor->SetDesiredFailureMsg(
                kWarningBit, "UNASSIGNED-BestPractices-vkBindVideoSessionMemoryKHR-requirements-not-all-retrieved");
            context.vk.BindVideoSessionMemoryKHR(m_device->device(), context.Session(), 1, &bind_info);
            m_errorMonitor->VerifyFound();
        }
    }
}
