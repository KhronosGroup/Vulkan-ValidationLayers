/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 RasterGrid Kft.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/video_objects.h"
#include "generated/enum_flag_bits.h"

class NegativeVideoEncodeIntraRefresh : public VkVideoLayerTest {};

TEST_F(NegativeVideoEncodeIntraRefresh, CreateSessionVideoEncodeIntraRefreshNotEnabled) {
    TEST_DESCRIPTION("vkCreateVideoSession - creating session with intra refresh requires videoEncodeIntraRefresh feature");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    ForceDisableFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithIntraRefresh(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with intra refresh";
    }

    VkVideoSessionKHR session = VK_NULL_HANDLE;

    config.SetVideoEncodeIntraRefreshMode(VK_VIDEO_ENCODE_INTRA_REFRESH_MODE_NONE_KHR);
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    EXPECT_EQ(vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session), VK_SUCCESS);
    vk::DestroyVideoSessionKHR(device(), session, nullptr);

    config.SetVideoEncodeIntraRefreshMode(config.GetAnySupportedIntraRefreshMode());
    create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-10835");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeIntraRefresh, IntraRefreshUnsupportedMode) {
    TEST_DESCRIPTION("vkCreateVideoSession - intra refresh mode is not supported");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    // Try to find a config that does not support all intra refresh modes
    VkVideoEncodeIntraRefreshModeFlagsKHR all_ir_modes = AllVkVideoEncodeIntraRefreshModeFlagBitsKHR;
    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [all_ir_modes](const VideoConfig& config) {
        return (config.EncodeIntraRefreshCaps()->intraRefreshModes & all_ir_modes) < all_ir_modes;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires an encode profile that does not support all intra refresh modes";
    }

    auto unsupported_ir_modes = (config.EncodeIntraRefreshCaps()->intraRefreshModes & all_ir_modes) ^ all_ir_modes;

    VkVideoSessionKHR session = VK_NULL_HANDLE;

    config.SetVideoEncodeIntraRefreshMode(
        static_cast<VkVideoEncodeIntraRefreshModeFlagBitsKHR>(1 << static_cast<uint32_t>(log2(unsupported_ir_modes))));
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-10836");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeIntraRefresh, InvalidIntraRefreshMode) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - intra refresh mode is invalid");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithIntraRefresh(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with intra refresh";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10837");
    cb.EncodeVideo(context.EncodeFrame(0).IntraRefresh(2, 0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeIntraRefresh, TooManyIntraRefreshActiveReferencePictures) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - too many active reference pictures when using intra refresh");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithIntraRefresh(GetConfigsEncode()), [](const VideoConfig& config) {
        const uint32_t max_intra_refresh_refs = config.EncodeIntraRefreshCaps()->maxIntraRefreshActiveReferencePictures;
        return config.Caps()->maxDpbSlots > max_intra_refresh_refs + 1 &&
               config.Caps()->maxActiveReferencePictures > max_intra_refresh_refs;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with limited max intra refresh active references";
    }

    auto max_active_ref_plus1 = config.EncodeIntraRefreshCaps()->maxIntraRefreshActiveReferencePictures + 1;

    config.SetVideoEncodeIntraRefreshMode(config.GetAnySupportedIntraRefreshMode());
    config.SessionCreateInfo()->maxDpbSlots = max_active_ref_plus1 + 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = max_active_ref_plus1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto begin_info = context.Begin();
    for (unsigned int i = 0; i < max_active_ref_plus1; ++i) {
        begin_info.AddResource(i, i);
    }
    begin_info.AddResource(-1, max_active_ref_plus1);

    cb.Begin();
    cb.BeginVideoCoding(begin_info);

    auto encode_info = context.EncodeFrame(max_active_ref_plus1).IntraRefresh(2, 0);
    for (unsigned int i = 0; i < max_active_ref_plus1; ++i) {
        encode_info.AddReferenceFrame(i);
    }

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10838");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeIntraRefresh, BlockBasedInvalidIntraRefreshInfo) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - intra refresh info is invalid for block-based intra refresh");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithBlockBasedIntraRefresh(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with block-based intra refresh";
    }

    config.SetVideoEncodeIntraRefreshMode(VK_VIDEO_ENCODE_INTRA_REFRESH_MODE_BLOCK_BASED_BIT_KHR);

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    auto encode_info = context.EncodeFrame();
    encode_info->flags |= VK_VIDEO_ENCODE_INTRA_REFRESH_BIT_KHR;

    auto ir_info = vku::InitStruct<VkVideoEncodeIntraRefreshInfoKHR>();

    // VkVideoEncodeInfo::pNext missing VkVideoEncodeIntraRefreshInfoKHR
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10839");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    ir_info.pNext = encode_info->pNext;
    encode_info->pNext = &ir_info;

    // intraRefreshCycleDuration == 0
    {
        ir_info.intraRefreshCycleDuration = 0;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10840");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // intraRefreshIndex > intraRefreshCycleDuration
    {
        ir_info.intraRefreshCycleDuration = 2;
        ir_info.intraRefreshIndex = 3;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10841");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // intraRefreshCycleDuration out of supported range
    {
        ir_info.intraRefreshIndex = 0;

        ir_info.intraRefreshCycleDuration = 1;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-10844");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        ir_info.intraRefreshCycleDuration = config.EncodeIntraRefreshCaps()->maxIntraRefreshCycleDuration + 1;

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-10844");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeIntraRefresh, PerPicturePartitionInvalidIntraRefreshInfo) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - intra refresh info is invalid for per-picture-partition intra refresh");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithPerPartitionIntraRefresh(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with per-picture-partition intra refresh";
    }

    config.SetVideoEncodeIntraRefreshMode(VK_VIDEO_ENCODE_INTRA_REFRESH_MODE_PER_PICTURE_PARTITION_BIT_KHR);

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    auto encode_info = context.EncodeFrame();
    encode_info->flags |= VK_VIDEO_ENCODE_INTRA_REFRESH_BIT_KHR;

    // For per-picture-partition intra refresh mode we may indirectly violate other codec-specific VUs
    auto set_allowed_vuids = [&] {
        if (config.GetVideoEncodeIntraRefreshMode() == VK_VIDEO_ENCODE_INTRA_REFRESH_MODE_PER_PICTURE_PARTITION_BIT_KHR) {
            switch (config.Profile()->videoCodecOperation) {
                case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
                    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10846");
                    break;
                case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
                    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10852");
                    break;
                default:
                    break;
            }
        }
    };

    auto ir_info = vku::InitStruct<VkVideoEncodeIntraRefreshInfoKHR>();

    // VkVideoEncodeInfo::pNext missing VkVideoEncodeIntraRefreshInfoKHR
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10839");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    ir_info.pNext = encode_info->pNext;
    encode_info->pNext = &ir_info;

    // intraRefreshCycleDuration == 0
    {
        ir_info.intraRefreshCycleDuration = 0;

        set_allowed_vuids();
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10840");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // intraRefreshIndex > intraRefreshCycleDuration
    {
        ir_info.intraRefreshCycleDuration = 2;
        ir_info.intraRefreshIndex = 3;

        set_allowed_vuids();
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10841");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // intraRefreshCycleDuration out of supported range
    {
        ir_info.intraRefreshIndex = 0;

        ir_info.intraRefreshCycleDuration = 1;

        set_allowed_vuids();
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-10844");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();

        ir_info.intraRefreshCycleDuration = config.EncodeIntraRefreshCaps()->maxIntraRefreshCycleDuration + 1;

        set_allowed_vuids();
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-10844");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeIntraRefresh, InvalidDirtyRegions) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - intra refresh reference info contains invalid dirty intra refresh regions");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithIntraRefresh(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with intra refresh";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    config.SetVideoEncodeIntraRefreshMode(config.GetAnySupportedIntraRefreshMode());

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(1, 1));

    // dirtyIntraRefreshRegions is not zero but intra refresh is not enabled
    {
        auto encode_info = context.EncodeFrame(0).IntraRefresh(2, 1).AddReferenceFrameWithDirtyRegions(1, 1);
        encode_info->flags = encode_info->flags & (~VK_VIDEO_ENCODE_INTRA_REFRESH_BIT_KHR);

        // For per-picture-partition intra refresh mode we may indirectly violate other codec-specific VUs
        if (config.GetVideoEncodeIntraRefreshMode() == VK_VIDEO_ENCODE_INTRA_REFRESH_MODE_PER_PICTURE_PARTITION_BIT_KHR) {
            switch (config.Profile()->videoCodecOperation) {
                case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
                    if ((config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_DIFFERENT_SLICE_TYPE_BIT_KHR) == 0) {
                        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-flags-10850");
                    }
                    break;
                case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
                    if ((config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_DIFFERENT_SLICE_SEGMENT_TYPE_BIT_KHR) ==
                        0) {
                        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdEncodeVideoKHR-flags-10856");
                    }
                    break;
                default:
                    break;
            }
        }

        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-10842");
        cb.EncodeVideo(encode_info);
        m_errorMonitor->VerifyFound();
    }

    // dirtyIntraRefreshRegions != intraRefreshCycleDuration - intraRefreshIndex
    {
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-10843");
        cb.EncodeVideo(context.EncodeFrame(0).IntraRefresh(2, 1).AddReferenceFrameWithDirtyRegions(1, 2));
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeVideoEncodeIntraRefresh, BlockBasedSliceCountH264) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - cannot encode multiple H.264 slices with block-based intra refresh when partition independent intra "
        "refresh regions is not supported");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config =
        GetConfig(FilterConfigs(GetConfigsWithBlockBasedIntraRefresh(GetConfigsEncodeH264()), [](const VideoConfig& config) {
            return config.EncodeIntraRefreshCaps()->partitionIndependentIntraRefreshRegions == VK_FALSE &&
                   config.EncodeCapsH264()->maxSliceCount > 1;
        }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support without partition-independent intra refresh with block-based mode";
    }

    config.SetVideoEncodeIntraRefreshMode(VK_VIDEO_ENCODE_INTRA_REFRESH_MODE_BLOCK_BASED_BIT_KHR);

    // We use the largest coded extent to avoid hitting other limitations
    config.SessionCreateInfo()->maxCodedExtent = config.Caps()->maxCodedExtent;

    const uint32_t duration = config.EncodeIntraRefreshCaps()->maxIntraRefreshCycleDuration;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10845");
    cb.EncodeVideo(context.EncodeFrame().IntraRefresh(duration, 0).SetPartitionCount(2));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeIntraRefresh, PerPicturePartitionSliceCountH264) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - H.264 encode with per-picture-partition intra refresh requires matching slice count and duration");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithPerPartitionIntraRefresh(GetConfigsEncodeH264()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with per-picture-partition intra refresh";
    }

    config.SetVideoEncodeIntraRefreshMode(VK_VIDEO_ENCODE_INTRA_REFRESH_MODE_PER_PICTURE_PARTITION_BIT_KHR);

    // We use the largest coded extent to avoid hitting other limitations
    config.SessionCreateInfo()->maxCodedExtent = config.Caps()->maxCodedExtent;

    const uint32_t duration = std::min(config.EncodeIntraRefreshCaps()->maxIntraRefreshCycleDuration, 3u);
    const uint32_t slice_count =
        std::min(config.EncodeCapsH264()->maxSliceCount, config.EncodeIntraRefreshCaps()->maxIntraRefreshCycleDuration - 1);

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10846");
    cb.EncodeVideo(context.EncodeFrame().IntraRefresh(duration, 0).SetPartitionCount(slice_count));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeIntraRefresh, PerPicturePartitionSliceTypeH264) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - H.264 encode with per-picture-partition intra refresh requires I slice for refreshed slice");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config =
        GetConfig(FilterConfigs(GetConfigsWithPerPartitionIntraRefresh(GetConfigsEncodeH264()),
                                [](const VideoConfig& config) { return config.EncodeCapsH264()->maxSliceCount > 1; }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with per-picture-partition intra refresh";
    }

    config.SetVideoEncodeIntraRefreshMode(VK_VIDEO_ENCODE_INTRA_REFRESH_MODE_PER_PICTURE_PARTITION_BIT_KHR);

    // We use the largest coded extent to avoid hitting other limitations
    config.SessionCreateInfo()->maxCodedExtent = config.Caps()->maxCodedExtent;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    const uint32_t refreshed_slice_index = 1;
    auto encode_info = context.EncodeFrame().IntraRefresh(2, refreshed_slice_index);
    auto& slice_headers = encode_info.CodecInfo().encode_h264.std_slice_headers;

    slice_headers[refreshed_slice_index].slice_type = STD_VIDEO_H264_SLICE_TYPE_P;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-intraRefreshH264SliceIndex-10847");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeIntraRefresh, PerPicturePartitionNonRectTooManySlicesH264) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - H.264 per-picture-partition intra refresh with too many slices but no non-rectangular intra refresh "
        "regions");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    auto configs = GetConfigsWithPerPartitionIntraRefresh(GetConfigsEncodeH264());
    VideoConfig config;
    uint32_t max_slice_count = 0;
    for (auto& cfg : configs) {
        if (cfg.EncodeIntraRefreshCaps()->nonRectangularIntraRefreshRegions == VK_FALSE &&
            cfg.EncodeCapsH264()->maxSliceCount <= cfg.EncodeIntraRefreshCaps()->maxIntraRefreshCycleDuration &&
            cfg.EncodeCapsH264()->maxSliceCount > max_slice_count) {
            config = cfg;
            max_slice_count = cfg.EncodeCapsH264()->maxSliceCount;
        }
    }
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with per-picture-partition intra refresh";
    }

    config.SetVideoEncodeIntraRefreshMode(VK_VIDEO_ENCODE_INTRA_REFRESH_MODE_PER_PICTURE_PARTITION_BIT_KHR);

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    const uint32_t slice_count = config.MaxEncodeH264MBRowCount() + 1;

    if (slice_count > config.EncodeCapsH264()->maxSliceCount) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH264PictureInfoKHR-naluSliceEntryCount-08301");
    }
    if (slice_count > config.EncodeIntraRefreshCaps()->maxIntraRefreshCycleDuration) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-10844");
    }
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10848");
    cb.EncodeVideo(context.EncodeFrame().IntraRefresh(slice_count, 0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeIntraRefresh, UnsupportedBPicH264) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - H.264 B picture encode with intra refresh without support");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithIntraRefresh(GetConfigsEncodeH264()), [](const VideoConfig& config) {
        return (config.EncodeCapsH264()->maxBPictureL0ReferenceCount > 0) &&
               (config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_B_PICTURE_INTRA_REFRESH_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with no support for B picture encode with intra refresh";
    }

    config.SetVideoEncodeIntraRefreshMode(config.GetAnySupportedIntraRefreshMode());

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    auto encode_info = context.EncodeFrame().IntraRefresh(2, 0);
    encode_info.CodecInfo().encode_h264.std_picture_info.primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_B;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-h264PictureType-10849");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeIntraRefresh, BlockBasedSliceSegmentCountH265) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - cannot encode multiple H.265 slice segments with block-based intra refresh when partition "
        "independent intra "
        "refresh regions is not supported");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config =
        GetConfig(FilterConfigs(GetConfigsWithBlockBasedIntraRefresh(GetConfigsEncodeH265()), [](const VideoConfig& config) {
            return config.EncodeIntraRefreshCaps()->partitionIndependentIntraRefreshRegions == VK_FALSE &&
                   config.EncodeCapsH265()->maxSliceSegmentCount > 1;
        }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support without partition-independent intra refresh with block-based mode";
    }

    config.SetVideoEncodeIntraRefreshMode(VK_VIDEO_ENCODE_INTRA_REFRESH_MODE_BLOCK_BASED_BIT_KHR);

    // We use the largest coded extent to avoid hitting other limitations
    config.SessionCreateInfo()->maxCodedExtent = config.Caps()->maxCodedExtent;

    const uint32_t duration = config.EncodeIntraRefreshCaps()->maxIntraRefreshCycleDuration;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    // We want to test this VU, even in the absence of some capabilities
    if ((config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_MULTIPLE_SLICE_SEGMENTS_PER_TILE_BIT_KHR) == 0) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08324");
    }
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10851");
    cb.EncodeVideo(context.EncodeFrame().IntraRefresh(duration, 0).SetPartitionCount(2));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeIntraRefresh, PerPicturePartitionSliceCountH265) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - H.265 encode with per-picture-partition intra refresh requires matching slice segment count and "
        "duration");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithPerPartitionIntraRefresh(GetConfigsEncodeH265()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with per-picture-partition intra refresh";
    }

    config.SetVideoEncodeIntraRefreshMode(VK_VIDEO_ENCODE_INTRA_REFRESH_MODE_PER_PICTURE_PARTITION_BIT_KHR);

    // We use the largest coded extent to avoid hitting other limitations
    config.SessionCreateInfo()->maxCodedExtent = config.Caps()->maxCodedExtent;

    const uint32_t duration = std::min(config.EncodeIntraRefreshCaps()->maxIntraRefreshCycleDuration, 3u);
    const uint32_t slice_segment_count =
        std::min(config.EncodeCapsH265()->maxSliceSegmentCount, config.EncodeIntraRefreshCaps()->maxIntraRefreshCycleDuration - 1);

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10852");
    cb.EncodeVideo(context.EncodeFrame().IntraRefresh(duration, 0).SetPartitionCount(slice_segment_count));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeIntraRefresh, PerPicturePartitionSliceTypeH265) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - H.265 encode with per-picture-partition intra refresh requires I slice segment for refreshed slice");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config =
        GetConfig(FilterConfigs(GetConfigsWithPerPartitionIntraRefresh(GetConfigsEncodeH265()),
                                [](const VideoConfig& config) { return config.EncodeCapsH265()->maxSliceSegmentCount > 1; }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with per-picture-partition intra refresh";
    }

    config.SetVideoEncodeIntraRefreshMode(VK_VIDEO_ENCODE_INTRA_REFRESH_MODE_PER_PICTURE_PARTITION_BIT_KHR);

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    const uint32_t refreshed_slice_segment_index = 1;
    auto encode_info = context.EncodeFrame().IntraRefresh(2, refreshed_slice_segment_index);
    auto& slice_segment_headers = encode_info.CodecInfo().encode_h265.std_slice_segment_headers;

    slice_segment_headers[refreshed_slice_segment_index].slice_type = STD_VIDEO_H265_SLICE_TYPE_P;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-intraRefreshH265SliceSegmentIndex-10853");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeIntraRefresh, PerPicturePartitionNonRectTooManySliceSegmentsH265) {
    TEST_DESCRIPTION(
        "vkCmdEncodeVideoKHR - H.265 per-picture-partition intra refresh with too many slice segments but no non-rectangular intra "
        "refresh regions");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    auto configs = GetConfigsWithPerPartitionIntraRefresh(GetConfigsEncodeH265());
    VideoConfig config;
    uint32_t max_slice_count = 0;
    for (auto& cfg : configs) {
        if (cfg.EncodeIntraRefreshCaps()->nonRectangularIntraRefreshRegions == VK_FALSE &&
            cfg.EncodeCapsH265()->maxSliceSegmentCount <= cfg.EncodeIntraRefreshCaps()->maxIntraRefreshCycleDuration &&
            cfg.EncodeCapsH265()->maxSliceSegmentCount > max_slice_count) {
            config = cfg;
            max_slice_count = cfg.EncodeCapsH265()->maxSliceSegmentCount;
        }
    }
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with per-picture-partition intra refresh";
    }

    config.SetVideoEncodeIntraRefreshMode(VK_VIDEO_ENCODE_INTRA_REFRESH_MODE_PER_PICTURE_PARTITION_BIT_KHR);

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    const uint32_t slice_segment_count = config.MaxEncodeH265CTBRowCount() + 1;

    if (slice_segment_count > config.EncodeCapsH265()->maxSliceSegmentCount) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoEncodeH265PictureInfoKHR-naluSliceSegmentEntryCount-08306");
    }
    if (slice_segment_count > config.EncodeIntraRefreshCaps()->maxIntraRefreshCycleDuration) {
        m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pNext-10844");
    }
    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-10854");
    cb.EncodeVideo(context.EncodeFrame().IntraRefresh(slice_segment_count, 0));
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeIntraRefresh, UnsupportedBPicH265) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - H.265 B picture encode with intra refresh without support");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithIntraRefresh(GetConfigsEncodeH265()), [](const VideoConfig& config) {
        return (config.EncodeCapsH265()->maxBPictureL0ReferenceCount > 0) &&
               (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_B_PICTURE_INTRA_REFRESH_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with no support for B picture encode with intra refresh";
    }

    config.SetVideoEncodeIntraRefreshMode(config.GetAnySupportedIntraRefreshMode());

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    auto encode_info = context.EncodeFrame().IntraRefresh(2, 0);
    encode_info.CodecInfo().encode_h265.std_picture_info.pic_type = STD_VIDEO_H265_PICTURE_TYPE_B;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-h265PictureType-10855");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeIntraRefresh, UnsupportedUnidirectionCompoundAV1) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 unidirectional compound prediction with intra refresh without support");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithIntraRefresh(GetConfigsEncodeAV1()), [](const VideoConfig& config) {
        return (config.EncodeCapsAV1()->maxUnidirectionalCompoundReferenceCount > 0) &&
               (config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_COMPOUND_PREDICTION_INTRA_REFRESH_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with unidirectional compound prediction mode support "
                        "but no support for compound prediction with intra refresh";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    config.SetVideoEncodeIntraRefreshMode(config.GetAnySupportedIntraRefreshMode());

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(1, 1));

    auto encode_info = context.EncodeFrame(0).IntraRefresh(2, 0).AddReferenceFrame(1, 1);
    encode_info.CodecInfo().encode_av1.picture_info.predictionMode =
        VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_UNIDIRECTIONAL_COMPOUND_KHR;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10857");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideoEncodeIntraRefresh, UnsupportedBidirectionCompoundAV1) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 bidirectional compound prediction with intra refresh without support");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithIntraRefresh(GetConfigsEncodeAV1()), [](const VideoConfig& config) {
        return (config.EncodeCapsAV1()->maxBidirectionalCompoundReferenceCount > 0) &&
               (config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_COMPOUND_PREDICTION_INTRA_REFRESH_BIT_KHR) == 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with bidirectional compound prediction mode support "
                        "but no support for compound prediction with intra refresh";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    config.SetVideoEncodeIntraRefreshMode(config.GetAnySupportedIntraRefreshMode());

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(1, 1));

    auto encode_info = context.EncodeFrame(0).IntraRefresh(2, 0).AddReferenceFrame(1, 1);
    encode_info.CodecInfo().encode_av1.picture_info.predictionMode = VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_BIDIRECTIONAL_COMPOUND_KHR;

    m_errorMonitor->SetDesiredError("VUID-vkCmdEncodeVideoKHR-predictionMode-10857");
    cb.EncodeVideo(encode_info);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}
