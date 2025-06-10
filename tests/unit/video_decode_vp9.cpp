/*
 * Copyright (c) 2022-2025 The Khronos Group Inc.
 * Copyright (c) 2022-2025 RasterGrid Kft.
 * Modifications Copyright (C) 2024 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/video_objects.h"

class NegativeVideoDecodeVP9 : public VkVideoLayerTest {};

TEST_F(NegativeVideoDecodeVP9, ProfileMissingCodecInfo) {
    TEST_DESCRIPTION("Test missing codec-specific structure in profile definition");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeVP9();
    if (!config) {
        GTEST_SKIP() << "Test requires VP9 decode support";
    }

    VkVideoProfileInfoKHR profile = *config.Profile();
    profile.pNext = nullptr;

    // Missing codec-specific info for AV1 decode profile
    m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-10791");
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), &profile, config.Caps());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoDecodeVP9, CapabilityQueryMissingChain) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoCapabilitiesKHR - missing return structures in chain");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeVP9();
    if (!config) {
        GTEST_SKIP() << "Test requires VP9 decode support";
    }

    auto caps = vku::InitStruct<VkVideoCapabilitiesKHR>();
    auto decode_caps = vku::InitStruct<VkVideoDecodeCapabilitiesKHR>();
    auto decode_vp9_caps = vku::InitStruct<VkVideoDecodeVP9CapabilitiesKHR>();

    // Missing decode caps struct for decode profile
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07183");
    caps.pNext = &decode_vp9_caps;
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), config.Profile(), &caps);
    m_errorMonitor->VerifyFound();

    // Missing VP9 decode caps struct for VP9 decode profile
    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-10792");
    caps.pNext = &decode_caps;
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), config.Profile(), &caps);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoDecodeVP9, CreateSessionVideoDecodeVP9NotEnabled) {
    TEST_DESCRIPTION("vkCreateVideoSession - VP9 decode is specified but videoDecodeVP9 was not enabled");

    ForceDisableFeature(vkt::Feature::videoDecodeVP9);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeVP9();
    if (!config) {
        GTEST_SKIP() << "Test requires VP9 encode support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-10793");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoDecodeVP9, CreateBufferVideoDecodeVP9NotEnabled) {
    TEST_DESCRIPTION("vkCreateBuffer - VP9 decode is specified but videoDecodeVP9 was not enabled");

    ForceDisableFeature(vkt::Feature::videoDecodeVP9);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeVP9();
    if (!config) {
        GTEST_SKIP() << "Test requires VP9 encode support";
    }

    VkBuffer buffer = VK_NULL_HANDLE;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage = VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR;
    buffer_ci.size = 2048;

    VkVideoProfileListInfoKHR video_profiles = vku::InitStructHelper();
    video_profiles.profileCount = 1;
    video_profiles.pProfiles = config.Profile();
    buffer_ci.pNext = &video_profiles;

    m_errorMonitor->SetDesiredError("VUID-VkBufferCreateInfo-pNext-10783");
    vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoDecodeVP9, CreateImageVideoDecodeVP9NotEnabled) {
    TEST_DESCRIPTION("vkCreateImage - VP9 decode is specified but videoDecodeVP9 was not enabled");

    ForceDisableFeature(vkt::Feature::videoDecodeVP9);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeVP9();
    if (!config) {
        GTEST_SKIP() << "Test requires VP9 encode support";
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

    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-pNext-10784");
    vk::CreateImage(device(), &image_ci, nullptr, &image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoDecodeVP9, CreateQueryPoolVideoDecodeVP9NotEnabled) {
    TEST_DESCRIPTION("vkCreateQueryPool - VP9 decode is specified but videoDecodeVP9 was not enabled");

    ForceDisableFeature(vkt::Feature::videoDecodeVP9);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeVP9();
    if (!config) {
        GTEST_SKIP() << "Test requires VP9 encode support";
    }
    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video decode queue to support result status queries";
    }

    VkQueryPool query_pool = VK_NULL_HANDLE;
    auto create_info = vku::InitStruct<VkQueryPoolCreateInfo>(config.Profile());
    create_info.queryType = VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR;
    create_info.queryCount = 1;

    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-pNext-10779");
    vk::CreateQueryPool(device(), &create_info, nullptr, &query_pool);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoDecodeVP9, CreateVideoSessionParams) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - VP9 decode does not support creating session parameters.");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecodeVP9();
    if (!config) {
        GTEST_SKIP() << "Test requires VP9 encode support";
    }

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VkVideoSessionParametersKHR params;
    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.videoSession = context.Session();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-10794");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoDecodeVP9, DecodeInvalidCodecInfo) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - invalid/missing VP9 codec-specific information");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecodeVP9(), 2), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires VP9 decode support with 2 reference pictures and 2 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VideoDecodeInfo decode_info = context.DecodeFrame(0);

    StdVideoDecodeVP9PictureInfo std_picture_info{};
    auto picture_info = vku::InitStruct<VkVideoDecodeVP9PictureInfoKHR>();
    picture_info.pStdPictureInfo = &std_picture_info;
    for (uint32_t i = 0; i < VK_MAX_VIDEO_VP9_REFERENCES_PER_FRAME_KHR; ++i) {
        picture_info.referenceNameSlotIndices[i] = -1;
    }
    picture_info.uncompressedHeaderOffset = 0;
    picture_info.compressedHeaderOffset = 0;
    picture_info.tilesOffset = 0;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(-1, 1));

    // Missing VP9 picture info
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = nullptr;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-pNext-10805");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Uncompressed frame header offset must be within buffer range
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-uncompressedHeaderOffset-10806");
        picture_info.uncompressedHeaderOffset = (uint32_t)decode_info->srcBufferRange;
        cb.DecodeVideo(decode_info);
        picture_info.uncompressedHeaderOffset -= 1;
        m_errorMonitor->VerifyFound();
    }

    // Compressed frame header offset must be within buffer range
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-compressedHeaderOffset-10807");
        picture_info.compressedHeaderOffset = (uint32_t)decode_info->srcBufferRange;
        cb.DecodeVideo(decode_info);
        picture_info.compressedHeaderOffset -= 1;
        m_errorMonitor->VerifyFound();
    }

    // Tile offset must be within buffer range
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-tilesOffset-10808");
        picture_info.tilesOffset = (uint32_t)decode_info->srcBufferRange;
        cb.DecodeVideo(decode_info);
        picture_info.tilesOffset -= 1;
        m_errorMonitor->VerifyFound();
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

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-slotIndex-10810");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();
    }

    // Missing reference slot for DPB slot index referred to by referenceNameSlotIndices
    {
        decode_info = context.DecodeFrame(0);
        decode_info->pNext = &picture_info;

        picture_info.referenceNameSlotIndices[2] = 0;

        m_errorMonitor->SetDesiredError("VUID-vkCmdDecodeVideoKHR-referenceNameSlotIndices-10809");
        cb.DecodeVideo(decode_info);
        m_errorMonitor->VerifyFound();

        picture_info.referenceNameSlotIndices[2] = -1;
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}
