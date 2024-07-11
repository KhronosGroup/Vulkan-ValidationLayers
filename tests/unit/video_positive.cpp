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

TEST_F(PositiveVideo, VideoCodingScope) {
    TEST_DESCRIPTION("Tests calling functions inside/outside video coding scope");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(m_device, GetConfigDecode());
    context.CreateAndBindSessionMemory();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset());
    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(PositiveVideo, MultipleCmdBufs) {
    TEST_DESCRIPTION("Tests submit-time validation with multiple command buffers submitted at once");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandPool cmd_pool(*m_device, config.QueueFamilyIndex(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer cb1(*m_device, cmd_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    vkt::CommandBuffer cb2(*m_device, cmd_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    cb1.begin();
    cb1.BeginVideoCoding(context.Begin());
    cb1.ControlVideoCoding(context.Control().Reset());
    cb1.EndVideoCoding(context.End());
    cb1.end();

    cb2.begin();
    vk::CmdPipelineBarrier2KHR(cb2.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    cb2.BeginVideoCoding(context.Begin().AddResource(-1, 0));
    cb2.DecodeVideo(context.DecodeFrame(0));
    cb2.EndVideoCoding(context.End());
    cb2.end();

    std::array cbs = {&cb1, &cb2};
    context.Queue().Submit(cbs);
    m_device->Wait();
}

TEST_F(PositiveVideo, BeginCodingOutOfBoundsSlotIndex) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - referenced DPB slot index is invalid but it should not cause submit time crash");

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
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoBeginCodingInfoKHR-slotIndex-04856");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(3, 1).AddResource(-1, 2));
    cb.EndVideoCoding(context.End());
    cb.end();

    context.Queue().Submit(cb);
    m_device->Wait();
}

TEST_F(PositiveVideo, VideoDecodeProfileIndependentResources) {
    TEST_DESCRIPTION("Test video profile independent resources with decode");

    EnableVideoMaintenance1();
    RETURN_IF_SKIP(Init());
    if (!IsVideoMaintenance1Enabled()) {
        GTEST_SKIP() << "Test requires videoMaintenance1";
    }

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with references";
    }

    config.EnableProfileIndependentResources();
    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(PositiveVideo, VideoEncodeProfileIndependentResources) {
    TEST_DESCRIPTION("Test video profile independent resources with encode");

    EnableVideoMaintenance1();
    RETURN_IF_SKIP(Init());
    if (!IsVideoMaintenance1Enabled()) {
        GTEST_SKIP() << "Test requires videoMaintenance1";
    }

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with references";
    }

    config.EnableProfileIndependentResources();
    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.EncodeVideo(context.EncodeReferenceFrame(0));
    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(PositiveVideo, VideoDecodeH264) {
    TEST_DESCRIPTION("Tests basic H.264/AVC video decode use case for framework verification purposes");

    RETURN_IF_SKIP(Init());

    const uint32_t dpb_slots = 3;
    const uint32_t active_refs = 2;

    VideoConfig config =
        GetConfig(GetConfigsWithReferences(GetConfigsWithDpbSlots(GetConfigsDecodeH264(), dpb_slots), active_refs));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support with 3 DPB slots and 2 active references";
    }

    config.SessionCreateInfo()->maxDpbSlots = dpb_slots;
    config.SessionCreateInfo()->maxActiveReferencePictures = active_refs;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR));
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1).AddResource(-1, 2));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    cb.DecodeVideo(context.DecodeReferenceFrame(1).AddReferenceFrame(0));
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0));
    cb.DecodeVideo(context.DecodeReferenceFrame(2).AddReferenceFrame(0));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).InvalidateSlot(1).AddResource(-1, 1).AddResource(2, 2));
    cb.DecodeVideo(context.DecodeFrame(1));
    cb.DecodeVideo(context.DecodeReferenceFrame(1).AddReferenceFrame(0).AddReferenceFrame(2));
    cb.DecodeVideo(context.DecodeReferenceFrame(2).AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().Submit(cb);
    m_device->Wait();
}

TEST_F(PositiveVideo, VideoDecodeH264Interlaced) {
    TEST_DESCRIPTION("Tests basic H.264/AVC interlaced video decode use case for framework verification purposes");

    RETURN_IF_SKIP(Init());

    const uint32_t dpb_slots = 2;
    const uint32_t active_refs = 2;

    VideoConfig config =
        GetConfig(GetConfigsWithReferences(GetConfigsWithDpbSlots(GetConfigsDecodeH264Interlaced(), dpb_slots), active_refs));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 interlaced decode support with 2 DPB slots and 2 active references";
    }

    config.SessionCreateInfo()->maxDpbSlots = dpb_slots;
    config.SessionCreateInfo()->maxActiveReferencePictures = active_refs;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR));
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.DecodeVideo(context.DecodeReferenceTopField(0));
    cb.DecodeVideo(context.DecodeReferenceBottomField(0).AddReferenceTopField(0));
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceBottomField(0));
    cb.DecodeVideo(context.DecodeReferenceFrame(1).AddReferenceBothFields(0));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().InvalidateSlot(0).AddResource(-1, 0).AddResource(1, 1));
    cb.DecodeVideo(context.DecodeFrame(0));
    cb.DecodeVideo(context.DecodeReferenceFrame(0).AddReferenceFrame(1));
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().Submit(cb);
    m_device->Wait();
}

TEST_F(PositiveVideo, VideoDecodeH264InterlacedPartialInvalidation) {
    TEST_DESCRIPTION("Tests H.264/AVC interlaced video decode with partial DPB slot picture reference invalidation");

    RETURN_IF_SKIP(Init());

    const uint32_t dpb_slots = 3;
    const uint32_t active_refs = 2;

    VideoConfig config =
        GetConfig(GetConfigsWithReferences(GetConfigsWithDpbSlots(GetConfigsDecodeH264Interlaced(), dpb_slots), active_refs));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 interlaced decode support with 3 DPB slots and 2 active references";
    }

    config.SessionCreateInfo()->maxDpbSlots = dpb_slots;
    config.SessionCreateInfo()->maxActiveReferencePictures = active_refs;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR));
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.DecodeVideo(context.DecodeReferenceTopField(0));
    cb.DecodeVideo(context.DecodeBottomField(0).AddReferenceTopField(0));
    cb.DecodeVideo(context.DecodeReferenceFrame(1).AddReferenceTopField(0));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(-1, 2));
    cb.DecodeVideo(context.DecodeReferenceBottomField(2).AddReferenceTopField(0).AddReferenceFrame(1));
    cb.DecodeVideo(context.DecodeTopField(2).AddReferenceBottomField(2));
    cb.DecodeVideo(context.DecodeFrame(0).AddReferenceBottomField(2));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().Submit(cb);
    m_device->Wait();
}

TEST_F(PositiveVideo, VideoDecodeH265) {
    TEST_DESCRIPTION("Tests basic H.265/HEVC video decode use case for framework verification purposes");

    RETURN_IF_SKIP(Init());

    const uint32_t dpb_slots = 3;
    const uint32_t active_refs = 2;

    VideoConfig config =
        GetConfig(GetConfigsWithReferences(GetConfigsWithDpbSlots(GetConfigsDecodeH265(), dpb_slots), active_refs));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support with 3 DPB slots and 2 active references";
    }

    config.SessionCreateInfo()->maxDpbSlots = dpb_slots;
    config.SessionCreateInfo()->maxActiveReferencePictures = active_refs;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR));
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1).AddResource(-1, 2));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0));
    cb.DecodeVideo(context.DecodeReferenceFrame(1).AddReferenceFrame(0));
    cb.DecodeVideo(context.DecodeFrame(2));
    cb.DecodeVideo(context.DecodeReferenceFrame(2).AddReferenceFrame(0).AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).InvalidateSlot(1).AddResource(-1, 1).AddResource(2, 2));
    cb.DecodeVideo(context.DecodeFrame(1));
    cb.DecodeVideo(context.DecodeReferenceFrame(1).AddReferenceFrame(0).AddReferenceFrame(2));
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().Submit(cb);
    m_device->Wait();
}

TEST_F(PositiveVideo, VideoDecodeAV1) {
    TEST_DESCRIPTION("Tests basic AV1 video decode use case for framework verification purposes");

    RETURN_IF_SKIP(Init());

    const uint32_t dpb_slots = 3;
    const uint32_t active_refs = 2;

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigsWithDpbSlots(GetConfigsDecodeAV1(), dpb_slots), active_refs));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 decode support with at least 3 DPB slots and 2 active references";
    }

    config.SessionCreateInfo()->maxDpbSlots = dpb_slots;
    config.SessionCreateInfo()->maxActiveReferencePictures = active_refs;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR));
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1).AddResource(-1, 2));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0));
    cb.DecodeVideo(context.DecodeReferenceFrame(1).AddReferenceFrame(0));
    cb.DecodeVideo(context.DecodeFrame(2));
    cb.DecodeVideo(context.DecodeReferenceFrame(2).AddReferenceFrame(0).AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).InvalidateSlot(1).AddResource(-1, 1).AddResource(2, 2));
    cb.DecodeVideo(context.DecodeFrame(1));
    cb.DecodeVideo(context.DecodeReferenceFrame(1).AddReferenceFrame(0).AddReferenceFrame(2));
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().Submit(cb);
    m_device->Wait();
}

TEST_F(PositiveVideo, DecodeAV1DistinctWithFilmGrain) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - AV1 works with distinct reconstructed picture if using film grain");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithReferences(GetConfigsDecodeAV1FilmGrain()),
                                                 [](const VideoConfig& config) { return !config.SupportsDecodeOutputDistinct(); }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 decode support with reference pictures, film grain, and no distinct mode";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
    cb.DecodeVideo(context.DecodeReferenceFrame(0).ApplyFilmGrain());
    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(PositiveVideo, DecodeAV1CoincideWithoutFilmGrain) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR - AV1 only requires distinct reconstructed picture if using film grain");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithReferences(GetConfigsDecodeAV1FilmGrain()),
                                                 [](const VideoConfig& config) { return config.SupportsDecodeOutputCoincide(); }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 decode support with reference pictures, film grain, and coincide mode";
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
    cb.DecodeVideo(context.DecodeFrame(0).SetDecodeOutput(context.Dpb()->Picture(0)));

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(PositiveVideo, VideoEncodeH264) {
    TEST_DESCRIPTION("Tests basic H.264/AVC video encode use case for framework verification purposes");

    RETURN_IF_SKIP(Init());

    const uint32_t dpb_slots = 3;
    const uint32_t active_refs = 2;

    VideoConfig config = GetConfig(GetConfigsWithReferences(
        GetConfigsWithDpbSlots(GetConfigsWithRateControl(GetConfigsEncodeH264()), dpb_slots), active_refs));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with rate control and 3 DPB slots and 2 active references";
    }

    config.SessionCreateInfo()->maxDpbSlots = dpb_slots;
    config.SessionCreateInfo()->maxActiveReferencePictures = active_refs;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config).SetAnyMode();
    for (uint32_t i = 0; i < config.EncodeCaps()->maxRateControlLayers; ++i) {
        auto rc_layer = VideoEncodeRateControlLayerInfo(config);
        rc_layer->averageBitrate = 64000;
        rc_layer->maxBitrate = 64000;
        rc_layer->frameRateNumerator = 30;
        rc_layer->frameRateDenominator = 1;
        rc_info.AddLayer(rc_layer);
    }

    cb.begin();
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.EncodeInput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR));
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1).AddResource(-1, 2));
    cb.ControlVideoCoding(context.Control().Reset().RateControl(rc_info).EncodeQualityLevel(0));
    cb.EncodeVideo(context.EncodeReferenceFrame(0));
    cb.EncodeVideo(context.EncodeReferenceFrame(1).AddReferenceFrame(0));
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(0));
    cb.EncodeVideo(context.EncodeReferenceFrame(2).AddReferenceFrame(0));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.begin();
    cb.BeginVideoCoding(
        context.Begin().RateControl(rc_info).AddResource(0, 0).InvalidateSlot(1).AddResource(-1, 1).AddResource(2, 2));
    cb.EncodeVideo(context.EncodeFrame(1));
    cb.EncodeVideo(context.EncodeReferenceFrame(1).AddReferenceFrame(0).AddReferenceFrame(2));
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().Submit(cb);
    m_device->Wait();
}

TEST_F(PositiveVideo, VideoEncodeH265) {
    TEST_DESCRIPTION("Tests basic H.265/HEVC video encode use case for framework verification purposes");

    RETURN_IF_SKIP(Init());

    const uint32_t dpb_slots = 3;
    const uint32_t active_refs = 2;

    VideoConfig config = GetConfig(GetConfigsWithReferences(
        GetConfigsWithDpbSlots(GetConfigsWithRateControl(GetConfigsEncodeH265()), dpb_slots), active_refs));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with rate control and 3 DPB slots and 2 active references";
    }

    config.SessionCreateInfo()->maxDpbSlots = dpb_slots;
    config.SessionCreateInfo()->maxActiveReferencePictures = active_refs;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config).SetAnyMode();
    for (uint32_t i = 0; i < config.EncodeCaps()->maxRateControlLayers; ++i) {
        auto rc_layer = VideoEncodeRateControlLayerInfo(config);
        rc_layer->averageBitrate = 128000;
        rc_layer->maxBitrate = 128000;
        rc_layer->frameRateNumerator = 30;
        rc_layer->frameRateDenominator = 1;
        rc_info.AddLayer(rc_layer);
    }

    cb.begin();
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.EncodeInput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR));
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1).AddResource(-1, 2));
    cb.ControlVideoCoding(context.Control().Reset().RateControl(rc_info).EncodeQualityLevel(0));
    cb.EncodeVideo(context.EncodeReferenceFrame(0));
    cb.EncodeVideo(context.EncodeFrame(1).AddReferenceFrame(0));
    cb.EncodeVideo(context.EncodeReferenceFrame(1).AddReferenceFrame(0));
    cb.EncodeVideo(context.EncodeFrame(2));
    cb.EncodeVideo(context.EncodeReferenceFrame(2).AddReferenceFrame(0).AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.begin();
    cb.BeginVideoCoding(
        context.Begin().RateControl(rc_info).AddResource(0, 0).InvalidateSlot(1).AddResource(-1, 1).AddResource(2, 2));
    cb.EncodeVideo(context.EncodeFrame(1));
    cb.EncodeVideo(context.EncodeReferenceFrame(1).AddReferenceFrame(0).AddReferenceFrame(2));
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().Submit(cb);
    m_device->Wait();
}

TEST_F(PositiveVideo, EncodeRateControlH264LayerCount) {
    TEST_DESCRIPTION(
        "vkCmdBeginVideoCodingKHR / vkCmdControlVideoCodingKHR - H.264 temporal layer count must only match "
        "the layer count if the layer count is greater than 1");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeH264()), [](const VideoConfig& config) {
        return config.EncodeCapsH264()->maxTemporalLayerCount > 1;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with rate control and temporal layer support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    rc_info.CodecInfo().encode_h264.temporalLayerCount = 2;

    cb.begin();
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(PositiveVideo, EncodeRateControlH265LayerCount) {
    TEST_DESCRIPTION(
        "vkCmdBeginVideoCodingKHR / vkCmdControlVideoCodingKHR - H.265 sub-layer count must only match "
        "the layer count if the layer count is greater than 1");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeH265()), [](const VideoConfig& config) {
        return config.EncodeCapsH265()->maxSubLayerCount > 1;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with rate control and sub-layer support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    rc_info.CodecInfo().encode_h265.subLayerCount = 2;

    cb.begin();
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(PositiveVideo, GetEncodedSessionParamsH264) {
    TEST_DESCRIPTION("vkGetEncodedVideoSessionParametersKHR - test basic usage");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support";
    }

    VideoContext context(m_device, config);

    auto h264_info = vku::InitStruct<VkVideoEncodeH264SessionParametersGetInfoKHR>();
    h264_info.writeStdSPS = VK_TRUE;
    h264_info.writeStdPPS = VK_TRUE;

    auto get_info = vku::InitStruct<VkVideoEncodeSessionParametersGetInfoKHR>(&h264_info);
    get_info.videoSessionParameters = context.SessionParams();

    auto feedback_info = vku::InitStruct<VkVideoEncodeSessionParametersFeedbackInfoKHR>();
    size_t data_size = 0;

    // Calling without feedback info and data pointer is legal
    context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);

    std::vector<uint8_t> data_buffer(data_size);

    // Calling without feedback info but data pointer is legal
    context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, data_buffer.data());

    // Calling with feedback info not including codec-specific feedback info
    context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, &feedback_info, &data_size, nullptr);
}

TEST_F(PositiveVideo, GetEncodedSessionParamsH265) {
    TEST_DESCRIPTION("vkGetEncodedVideoSessionParametersKHR - test basic usage");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support";
    }

    VideoContext context(m_device, config);

    auto h265_info = vku::InitStruct<VkVideoEncodeH265SessionParametersGetInfoKHR>();
    h265_info.writeStdVPS = VK_TRUE;
    h265_info.writeStdSPS = VK_TRUE;
    h265_info.writeStdPPS = VK_TRUE;

    auto get_info = vku::InitStruct<VkVideoEncodeSessionParametersGetInfoKHR>(&h265_info);
    get_info.videoSessionParameters = context.SessionParams();

    auto feedback_info = vku::InitStruct<VkVideoEncodeSessionParametersFeedbackInfoKHR>();
    size_t data_size = 0;

    // Calling without feedback info and data pointer is legal
    context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);

    std::vector<uint8_t> data_buffer(data_size);

    // Calling without feedback info but data pointer is legal
    context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, data_buffer.data());

    // Calling with feedback info not including codec-specific feedback info
    context.vk.GetEncodedVideoSessionParametersKHR(device(), &get_info, &feedback_info, &data_size, nullptr);
}

TEST_F(PositiveVideoSyncVal, ImageRangeGenYcbcrSubsampling) {
    TEST_DESCRIPTION(
        "Test that subsampled YCbCr image planes are handled correctly "
        "by the image range generation utilities used by sync validation");

    RETURN_IF_SKIP(Init());

    // Test values that require the implementation to handle YCbCr subsampling correctly
    // across planes in order for this test to not hit any asserts
    const VkExtent2D max_coded_extent = {272, 272};
    const VkExtent2D coded_extent = {256, 256};

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsDecode(), [&](const VideoConfig& config) {
        return config.PictureFormatProps()->format == VK_FORMAT_G8_B8R8_2PLANE_420_UNORM &&
               config.Caps()->maxCodedExtent.width >= max_coded_extent.width &&
               config.Caps()->maxCodedExtent.height >= max_coded_extent.height;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires decode with NV12 decode picture format support";
    }

    config.SessionCreateInfo()->maxCodedExtent = max_coded_extent;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset());

    // Test with a subregion that would cross the half-extent boundaries of a 4:2:0 subsampled image
    auto decode_info = context.DecodeFrame();
    decode_info->dstPictureResource.codedExtent = coded_extent;
    cb.DecodeVideo(decode_info);

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->MemoryBarrier());

    // Also test with an offset (ignoring other validation violations)
    decode_info->dstPictureResource.codedOffset = {1, 1};
    cb.DecodeVideo(decode_info);

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(PositiveVideoSyncVal, DecodeCoincide) {
    TEST_DESCRIPTION("Test video decode in coincide mode without sync hazards");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(FilterConfigs(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 3),
                                          [](const VideoConfig& config) { return !config.SupportsDecodeOutputDistinct(); }));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with references, 3 DPB slots, and coincide mode support";
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
    cb.DecodeVideo(context.DecodeFrame(1));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(1, 1));
    cb.DecodeVideo(context.DecodeFrame(1));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(0, 1));
    cb.DecodeVideo(context.DecodeReferenceFrame(0));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(0, 1));
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(1, 1));
    cb.DecodeVideo(context.DecodeReferenceFrame(1));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(0, 2));
    cb.DecodeVideo(context.DecodeFrame(0).AddReferenceFrame(1));

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(PositiveVideoSyncVal, DecodeDistinct) {
    TEST_DESCRIPTION("Test video decode in distinct mode without sync hazards");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(FilterConfigs(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsDecode()), 4),
                                          [](const VideoConfig& config) { return config.SupportsDecodeOutputDistinct(); }));
    if (!config) {
        GTEST_SKIP() << "Test requires video decode support with references, 4 DPB slots, and distinct mode support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 4;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(2, 2).AddResource(3, 3));
    cb.ControlVideoCoding(context.Control().Reset());

    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->MemoryBarrier());
    cb.DecodeVideo(context.DecodeFrame(1));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->MemoryBarrier());
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(0, 1));
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->MemoryBarrier());
    cb.DecodeVideo(context.DecodeReferenceFrame(3));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->MemoryBarrier());
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(2, 2));
    cb.DecodeVideo(context.DecodeReferenceFrame(2).AddReferenceFrame(3));

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(PositiveVideoSyncVal, Encode) {
    TEST_DESCRIPTION("Test video without sync hazards");

    RETURN_IF_SKIP(Init());

    auto config = GetConfig(GetConfigsWithDpbSlots(GetConfigsWithReferences(GetConfigsEncode()), 4));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with references and 4 DPB slots";
    }

    config.SessionCreateInfo()->maxDpbSlots = 4;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(2, 2).AddResource(3, 3));
    cb.ControlVideoCoding(context.Control().Reset());

    cb.EncodeVideo(context.EncodeReferenceFrame(0));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Bitstream().MemoryBarrier());
    cb.EncodeVideo(context.EncodeFrame(1));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Bitstream().MemoryBarrier());
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(0, 1));
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(0));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Bitstream().MemoryBarrier());
    cb.EncodeVideo(context.EncodeReferenceFrame(3));

    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Bitstream().MemoryBarrier());
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->MemoryBarrier(2, 2));
    cb.EncodeVideo(context.EncodeReferenceFrame(2).AddReferenceFrame(3));

    cb.EndVideoCoding(context.End());
    cb.end();
}
