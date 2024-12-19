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

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset());
    cb.EndVideoCoding(context.End());
    cb.End();
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

    cb1.Begin();
    cb1.BeginVideoCoding(context.Begin());
    cb1.ControlVideoCoding(context.Control().Reset());
    cb1.EndVideoCoding(context.End());
    cb1.End();

    cb2.Begin();
    vk::CmdPipelineBarrier2KHR(cb2.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    cb2.BeginVideoCoding(context.Begin().AddResource(-1, 0));
    cb2.DecodeVideo(context.DecodeFrame(0));
    cb2.EndVideoCoding(context.End());
    cb2.End();

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

    cb.Begin();
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkVideoBeginCodingInfoKHR-slotIndex-04856");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(3, 1).AddResource(-1, 2));
    cb.EndVideoCoding(context.End());
    cb.End();

    context.Queue().Submit(cb);
    m_device->Wait();
}

TEST_F(PositiveVideo, VideoDecodeProfileIndependentResources) {
    TEST_DESCRIPTION("Test video profile independent resources with decode");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

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

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveVideo, VideoEncodeProfileIndependentResources) {
    TEST_DESCRIPTION("Test video profile independent resources with encode");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

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

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0));
    cb.EncodeVideo(context.EncodeReferenceFrame(0));
    cb.EndVideoCoding(context.End());
    cb.End();
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

    cb.Begin();
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR));
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1).AddResource(-1, 2));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.DecodeVideo(context.DecodeReferenceFrame(0));
    cb.DecodeVideo(context.DecodeReferenceFrame(1).AddReferenceFrame(0));
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(0));
    cb.DecodeVideo(context.DecodeReferenceFrame(2).AddReferenceFrame(0));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).InvalidateSlot(1).AddResource(-1, 1).AddResource(2, 2));
    cb.DecodeVideo(context.DecodeFrame(1));
    cb.DecodeVideo(context.DecodeReferenceFrame(1).AddReferenceFrame(0).AddReferenceFrame(2));
    cb.DecodeVideo(context.DecodeReferenceFrame(2).AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.End();
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

    cb.Begin();
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR));
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.DecodeVideo(context.DecodeReferenceTopField(0));
    cb.DecodeVideo(context.DecodeReferenceBottomField(0).AddReferenceTopField(0));
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceBottomField(0));
    cb.DecodeVideo(context.DecodeReferenceFrame(1).AddReferenceBothFields(0));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().InvalidateSlot(0).AddResource(-1, 0).AddResource(1, 1));
    cb.DecodeVideo(context.DecodeFrame(0));
    cb.DecodeVideo(context.DecodeReferenceFrame(0).AddReferenceFrame(1));
    cb.DecodeVideo(context.DecodeFrame(1).AddReferenceFrame(0));
    cb.EndVideoCoding(context.End());
    cb.End();
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

    cb.Begin();
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR));
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.DecodeVideo(context.DecodeReferenceTopField(0));
    cb.DecodeVideo(context.DecodeBottomField(0).AddReferenceTopField(0));
    cb.DecodeVideo(context.DecodeReferenceFrame(1).AddReferenceTopField(0));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1).AddResource(-1, 2));
    cb.DecodeVideo(context.DecodeReferenceBottomField(2).AddReferenceTopField(0).AddReferenceFrame(1));
    cb.DecodeVideo(context.DecodeTopField(2).AddReferenceBottomField(2));
    cb.DecodeVideo(context.DecodeFrame(0).AddReferenceBottomField(2));
    cb.EndVideoCoding(context.End());
    cb.End();
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

    cb.Begin();
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
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).InvalidateSlot(1).AddResource(-1, 1).AddResource(2, 2));
    cb.DecodeVideo(context.DecodeFrame(1));
    cb.DecodeVideo(context.DecodeReferenceFrame(1).AddReferenceFrame(0).AddReferenceFrame(2));
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.End();
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

    cb.Begin();
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
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).InvalidateSlot(1).AddResource(-1, 1).AddResource(2, 2));
    cb.DecodeVideo(context.DecodeFrame(1));
    cb.DecodeVideo(context.DecodeReferenceFrame(1).AddReferenceFrame(0).AddReferenceFrame(2));
    cb.DecodeVideo(context.DecodeFrame(2).AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.End();
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

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));
    cb.DecodeVideo(context.DecodeReferenceFrame(0).ApplyFilmGrain());
    cb.EndVideoCoding(context.End());
    cb.End();
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

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0));

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07146");
    cb.DecodeVideo(context.DecodeFrame(0).SetDecodeOutput(context.Dpb()->Picture(0)));

    cb.EndVideoCoding(context.End());
    cb.End();
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

    cb.Begin();
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.EncodeInput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR));
    vk::CmdPipelineBarrier2KHR(cb.handle(), context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR));
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1).AddResource(-1, 2));
    cb.ControlVideoCoding(context.Control().Reset().RateControl(rc_info).EncodeQualityLevel(0));
    cb.EncodeVideo(context.EncodeReferenceFrame(0));
    cb.EncodeVideo(context.EncodeReferenceFrame(1).AddReferenceFrame(0));
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(0));
    cb.EncodeVideo(context.EncodeReferenceFrame(2).AddReferenceFrame(0));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.Begin();
    cb.BeginVideoCoding(
        context.Begin().RateControl(rc_info).AddResource(0, 0).InvalidateSlot(1).AddResource(-1, 1).AddResource(2, 2));
    cb.EncodeVideo(context.EncodeFrame(1));
    cb.EncodeVideo(context.EncodeReferenceFrame(1).AddReferenceFrame(0).AddReferenceFrame(2));
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.End();
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

    cb.Begin();
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
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.Begin();
    cb.BeginVideoCoding(
        context.Begin().RateControl(rc_info).AddResource(0, 0).InvalidateSlot(1).AddResource(-1, 1).AddResource(2, 2));
    cb.EncodeVideo(context.EncodeFrame(1));
    cb.EncodeVideo(context.EncodeReferenceFrame(1).AddReferenceFrame(0).AddReferenceFrame(2));
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();
}

TEST_F(PositiveVideo, EncodeRateControlVirtualBufferSize) {
    TEST_DESCRIPTION(
        "vkCmdControlVideoCodingKHR - test valid values for "
        "virtualBufferSizeInMs and initialVirtualBufferSizeInMs");

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

    // initialVirtualBufferSizeInMs can be less than virtualBufferSizeInMs
    rc_info->virtualBufferSizeInMs = 1000;
    rc_info->initialVirtualBufferSizeInMs = 0;
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));

    // initialVirtualBufferSizeInMs can be equal to virtualBufferSizeInMs
    rc_info->virtualBufferSizeInMs = 1000;
    rc_info->initialVirtualBufferSizeInMs = 1000;
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));

    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(PositiveVideo, VideoEncodeAV1) {
    TEST_DESCRIPTION("Tests basic AV1 video encode use case for framework verification purposes");

    RETURN_IF_SKIP(Init());

    const uint32_t dpb_slots = 3;
    const uint32_t active_refs = 2;

    VideoConfig config = GetConfig(
        GetConfigsWithReferences(GetConfigsWithDpbSlots(GetConfigsWithRateControl(GetConfigsEncodeAV1()), dpb_slots), active_refs));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with rate control and 3 DPB slots and 2 active references";
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

    cb.Begin();
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
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();

    cb.Begin();
    cb.BeginVideoCoding(
        context.Begin().RateControl(rc_info).AddResource(0, 0).InvalidateSlot(1).AddResource(-1, 1).AddResource(2, 2));
    cb.EncodeVideo(context.EncodeFrame(1));
    cb.EncodeVideo(context.EncodeReferenceFrame(1).AddReferenceFrame(0).AddReferenceFrame(2));
    cb.EncodeVideo(context.EncodeFrame(2).AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.End();
    context.Queue().Submit(cb);
    m_device->Wait();
}

TEST_F(PositiveVideo, VideoEncodeQuantDeltaMap) {
    TEST_DESCRIPTION("Tests video encode quantization delta maps");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    if (!GetConfigWithQuantDeltaMap(GetConfigsEncode())) {
        GTEST_SKIP() << "Test requires at least one encode profile with quantization delta map support";
    }

    // Test all encode configs that support quantization delta maps so that we cover all codecs
    // Furthermore, test all quantization delta map formats so that we cover different formats and texel sizes
    for (auto config : GetConfigsEncode()) {
        if (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_QUANTIZATION_DELTA_MAP_BIT_KHR) {
            config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR;

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
            m_device->Wait();

            for (const auto& map_props : config.SupportedQuantDeltaMapProps()) {
                const auto texel_size = config.GetQuantMapTexelSize(map_props);
                auto params = context.CreateSessionParamsWithQuantMapTexelSize(texel_size);
                VideoEncodeQuantizationMap quantization_map(m_device, config, *map_props.ptr());

                cb.Begin();
                cb.BeginVideoCoding(context.Begin().SetSessionParams(params));
                cb.EncodeVideo(context.EncodeFrame().QuantizationMap(VK_VIDEO_ENCODE_WITH_QUANTIZATION_DELTA_MAP_BIT_KHR,
                                                                     texel_size, quantization_map));
                cb.EndVideoCoding(context.End());
                cb.End();
                context.Queue().Submit(cb);
                m_device->Wait();
            }
        }
    }
}

TEST_F(PositiveVideo, VideoEncodeEmphasisMap) {
    TEST_DESCRIPTION("Tests video encode emphasis maps");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_QUANTIZATION_MAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeQuantizationMap);
    RETURN_IF_SKIP(Init());

    if (!GetConfigWithEmphasisMap(GetConfigsEncode())) {
        GTEST_SKIP() << "Test requires at least one encode profile with emphasis map support";
    }

    // Test all encode configs that support emphasis maps so that we cover all codecs
    // Furthermore, test all emphasis map formats so that we cover different formats and texel sizes
    for (auto config : GetConfigsEncode()) {
        if (config.EncodeCaps()->flags & VK_VIDEO_ENCODE_CAPABILITY_EMPHASIS_MAP_BIT_KHR) {
            config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_ALLOW_ENCODE_EMPHASIS_MAP_BIT_KHR;

            VideoContext context(m_device, config);
            context.CreateAndBindSessionMemory();
            context.CreateResources();

            vkt::CommandBuffer& cb = context.CmdBuffer();

            auto rc_info = VideoEncodeRateControlInfo(config).SetAnyMode();
            auto rc_layer = VideoEncodeRateControlLayerInfo(config);
            rc_layer->averageBitrate = 64000;
            rc_layer->maxBitrate = 64000;
            rc_layer->frameRateNumerator = 30;
            rc_layer->frameRateDenominator = 1;
            rc_info.AddLayer(rc_layer);

            cb.Begin();
            cb.BeginVideoCoding(context.Begin());
            cb.ControlVideoCoding(context.Control().Reset().RateControl(rc_info));
            cb.EndVideoCoding(context.End());
            cb.End();
            context.Queue().Submit(cb);
            m_device->Wait();

            for (const auto& map_props : config.SupportedEmphasisMapProps()) {
                const auto texel_size = config.GetQuantMapTexelSize(map_props);
                auto params = context.CreateSessionParamsWithQuantMapTexelSize(texel_size);
                VideoEncodeQuantizationMap quantization_map(m_device, config, *map_props.ptr());

                cb.Begin();
                cb.BeginVideoCoding(context.Begin().SetSessionParams(params).RateControl(rc_info));
                cb.EncodeVideo(
                    context.EncodeFrame().QuantizationMap(VK_VIDEO_ENCODE_WITH_EMPHASIS_MAP_BIT_KHR, texel_size, quantization_map));
                cb.EndVideoCoding(context.End());
                cb.End();
                context.Queue().Submit(cb);
                m_device->Wait();
            }
        }
    }
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

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    cb.EndVideoCoding(context.End());
    cb.End();
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

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveVideo, EncodeRateControlAV1LayerCount) {
    TEST_DESCRIPTION(
        "vkCmdBeginVideoCodingKHR / vkCmdControlVideoCodingKHR - AV1 temporal layer count must only match "
        "the layer count if the layer count is greater than 1");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsWithRateControl(GetConfigsEncodeAV1()), [](const VideoConfig& config) {
        return config.EncodeCapsAV1()->maxTemporalLayerCount > 1;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with rate control and temporal layer support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = VideoEncodeRateControlInfo(config, true).SetAnyMode();
    rc_info.AddLayer(VideoEncodeRateControlLayerInfo(config));
    rc_info.CodecInfo().encode_av1.temporalLayerCount = 2;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin().RateControl(rc_info));
    cb.ControlVideoCoding(context.Control().RateControl(rc_info));
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveVideo, EncodeAV1FrameSizeOverride) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 frame size override should be allowed when supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [](const VideoConfig& config) {
        return ((config.Caps()->minCodedExtent.width < config.Caps()->maxCodedExtent.width) ||
                (config.Caps()->minCodedExtent.height < config.Caps()->maxCodedExtent.height)) &&
               (config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_FRAME_SIZE_OVERRIDE_BIT_KHR) != 0;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with frame size override support";
    }

    config.UpdateMaxCodedExtent(config.Caps()->maxCodedExtent);

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    // We will use a smaller resolution than the max
    auto encode_info = context.EncodeFrame();
    encode_info.CodecInfo().encode_av1.std_picture_info.flags.frame_size_override_flag = 1;
    encode_info->srcPictureResource.codedExtent = config.Caps()->minCodedExtent;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.EncodeVideo(encode_info);
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveVideo, EncodeAV1MotionVectorScaling) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 motion vector scaling should be allowed when supported");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithDpbSlots(
        GetConfigsWithReferences(FilterConfigs(
            GetConfigsEncodeAV1(),
            [](const VideoConfig& config) {
                return ((config.Caps()->minCodedExtent.width < config.Caps()->maxCodedExtent.width) ||
                        (config.Caps()->minCodedExtent.height < config.Caps()->maxCodedExtent.height)) &&
                       (config.EncodeCapsAV1()->flags & VK_VIDEO_ENCODE_AV1_CAPABILITY_MOTION_VECTOR_SCALING_BIT_KHR) != 0;
            })),
        2));
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support with references and motion vector scaling support";
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
    cb.EncodeVideo(context.EncodeFrame(0).AddReferenceFrame(1, &patched_resource));
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveVideo, EncodeAV1SingleReference) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 single reference prediction");

    RETURN_IF_SKIP(Init());

    // Single reference prediction requires at least one active reference picture
    const uint32_t min_ref_count = 1;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [&](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > min_ref_count && config.Caps()->maxActiveReferencePictures >= min_ref_count &&
               config.EncodeCapsAV1()->maxSingleReferenceCount > 0;
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

    // Test all supported reference names
    for (uint8_t ref_name_idx = 0; ref_name_idx < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++ref_name_idx) {
        if ((config.EncodeCapsAV1()->singleReferenceNameMask & (1 << ref_name_idx)) != 0) {
            for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
                encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
            }
            encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[ref_name_idx] = 1;
            encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame = ref_name_idx;

            cb.EncodeVideo(encode_info);
        }
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveVideo, EncodeAV1UnidirectionalCompound) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 unidirectional compound prediction");

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
    std::vector<std::pair<uint8_t, uint8_t>> ref_name_pairs = {
        std::make_pair<uint8_t, uint8_t>(0, 1),  // LAST_FRAME + LAST2_FRAME
        std::make_pair<uint8_t, uint8_t>(0, 2),  // LAST_FRAME + LAST3_FRAME
        std::make_pair<uint8_t, uint8_t>(0, 3),  // LAST_FRAME + GOLDEN_FRAME
        std::make_pair<uint8_t, uint8_t>(4, 6),  // BWDREF_FRAME + ALTREF_FRAME
    };

    // Test all supported reference name combinations
    for (auto ref_name_pair : ref_name_pairs) {
        const uint32_t ref_name_mask = (1 << ref_name_pair.first) | (1 << ref_name_pair.second);
        if ((config.EncodeCapsAV1()->unidirectionalCompoundReferenceNameMask & ref_name_mask) == ref_name_mask) {
            for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
                encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
            }
            encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[ref_name_pair.first] = 1;
            encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[ref_name_pair.second] = 1;
            encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame = ref_name_pair.first;

            cb.EncodeVideo(encode_info);
        }
    }

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveVideo, EncodeAV1BidirectionalCompound) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - AV1 bidirectional compound prediction");

    RETURN_IF_SKIP(Init());

    // Bidirectional compound prediction requires at least one active reference picture
    // No need for two pictures as both reference names can point to the same picture
    const uint32_t min_ref_count = 1;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncodeAV1(), [&](const VideoConfig& config) {
        return config.Caps()->maxDpbSlots > min_ref_count && config.Caps()->maxActiveReferencePictures >= min_ref_count &&
               config.EncodeCapsAV1()->maxBidirectionalCompoundReferenceCount > 0;
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

    VideoEncodeInfo encode_info = context.EncodeFrame(0).AddReferenceFrame(1);
    encode_info.CodecInfo().encode_av1.picture_info.predictionMode = VK_VIDEO_ENCODE_AV1_PREDICTION_MODE_BIDIRECTIONAL_COMPOUND_KHR;

    // Test all supported reference name combinations (one from group 1 and one from group 2)
    const uint8_t bwdref_frame_idx = STD_VIDEO_AV1_REFERENCE_NAME_BWDREF_FRAME - 1;
    for (uint8_t ref_name_1 = 0; ref_name_1 < bwdref_frame_idx; ref_name_1++) {
        for (uint8_t ref_name_2 = bwdref_frame_idx; ref_name_2 < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ref_name_2++) {
            const uint32_t ref_name_mask = (1 << ref_name_1) | (1 << ref_name_2);
            if ((config.EncodeCapsAV1()->bidirectionalCompoundReferenceNameMask & ref_name_mask) == ref_name_mask) {
                for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
                    encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[i] = -1;
                }
                encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[ref_name_1] = 1;
                encode_info.CodecInfo().encode_av1.picture_info.referenceNameSlotIndices[ref_name_2] = 1;
                encode_info.CodecInfo().encode_av1.std_picture_info.primary_ref_frame = ref_name_1;

                cb.EncodeVideo(encode_info);
            }
        }
    }

    cb.EndVideoCoding(context.End());
    cb.End();
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
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);

    std::vector<uint8_t> data_buffer(data_size);

    // Calling without feedback info but data pointer is legal
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, data_buffer.data());

    // Calling with feedback info not including codec-specific feedback info
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, &feedback_info, &data_size, nullptr);
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
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);

    std::vector<uint8_t> data_buffer(data_size);

    // Calling without feedback info but data pointer is legal
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, data_buffer.data());

    // Calling with feedback info not including codec-specific feedback info
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, &feedback_info, &data_size, nullptr);
}

TEST_F(PositiveVideo, GetEncodedSessionParamsAV1) {
    TEST_DESCRIPTION("vkGetEncodedVideoSessionParametersKHR - test basic usage");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncodeAV1();
    if (!config) {
        GTEST_SKIP() << "Test requires AV1 encode support";
    }

    VideoContext context(m_device, config);

    auto get_info = vku::InitStruct<VkVideoEncodeSessionParametersGetInfoKHR>();
    get_info.videoSessionParameters = context.SessionParams();

    auto feedback_info = vku::InitStruct<VkVideoEncodeSessionParametersFeedbackInfoKHR>();
    size_t data_size = 0;

    // Calling without feedback info and data pointer is legal
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, nullptr);

    std::vector<uint8_t> data_buffer(data_size);

    // Calling without feedback info but data pointer is legal
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, nullptr, &data_size, data_buffer.data());

    // Calling with feedback info not including codec-specific feedback info
    vk::GetEncodedVideoSessionParametersKHR(device(), &get_info, &feedback_info, &data_size, nullptr);
}

TEST_F(PositiveSyncValVideo, ImageRangeGenYcbcrSubsampling) {
    TEST_DESCRIPTION(
        "Test that subsampled YCbCr image planes are handled correctly "
        "by the image range generation utilities used by sync validation");

    RETURN_IF_SKIP(Init());

    // Test values that require the implementation to handle YCbCr subsampling correctly
    // across planes in order for this test to not hit any asserts
    const VkExtent2D max_coded_extent = {272, 272};
    const VkExtent2D coded_extent = {256, 256};

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsDecode(), [&](const VideoConfig& config) {
        return (config.PictureFormatProps()->format == VK_FORMAT_G8_B8R8_2PLANE_420_UNORM ||
                config.PictureFormatProps()->format == VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM) &&
               config.Caps()->maxCodedExtent.width >= max_coded_extent.width &&
               config.Caps()->maxCodedExtent.height >= max_coded_extent.height;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires decode with 4:2:0 decode picture format support";
    }

    config.UpdateMaxCodedExtent(config.Caps()->maxCodedExtent);

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
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
    cb.End();
}

TEST_F(PositiveSyncValVideo, DecodeCoincide) {
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

    cb.Begin();
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
    cb.End();
}

TEST_F(PositiveSyncValVideo, DecodeDistinct) {
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

    cb.Begin();
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
    cb.End();
}

TEST_F(PositiveSyncValVideo, Encode) {
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

    cb.Begin();
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
    cb.End();
}

TEST_F(PositiveSyncValVideo, EncodeQuantizationMap) {
    TEST_DESCRIPTION("Test video encode quantization map without sync hazards");

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
        if (QueueFamilyFlags(config.QueueFamilyIndex()) & VK_QUEUE_TRANSFER_BIT) {
        }

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

        auto barrier_from_transfer = vku::InitStruct<VkImageMemoryBarrier2>();
        barrier_from_transfer.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier_from_transfer.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier_from_transfer.dstStageMask = VK_PIPELINE_STAGE_2_VIDEO_ENCODE_BIT_KHR;
        barrier_from_transfer.dstAccessMask = VK_ACCESS_2_VIDEO_ENCODE_READ_BIT_KHR;
        barrier_from_transfer.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier_from_transfer.newLayout = VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR;
        barrier_from_transfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier_from_transfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier_from_transfer.image = quantization_map.Image();
        barrier_from_transfer.subresourceRange = subres_range;
        auto dep_info_from_transfer = vku::InitStruct<VkDependencyInfo>();
        dep_info_from_transfer.imageMemoryBarrierCount = 1;
        dep_info_from_transfer.pImageMemoryBarriers = &barrier_from_transfer;
        vk::CmdPipelineBarrier2KHR(cb.handle(), &dep_info_from_transfer);

        cb.BeginVideoCoding(context.Begin().SetSessionParams(params));
        cb.ControlVideoCoding(context.Control().Reset());

        cb.EncodeVideo(context.EncodeFrame().QuantizationMap(encode_flag, texel_size, quantization_map));
        vk::CmdPipelineBarrier2KHR(cb.handle(), context.Bitstream().MemoryBarrier());
        cb.EncodeVideo(context.EncodeFrame().QuantizationMap(encode_flag, texel_size, quantization_map));

        auto barrier_to_transfer = vku::InitStruct<VkImageMemoryBarrier2>();
        barrier_to_transfer.srcStageMask = VK_PIPELINE_STAGE_2_VIDEO_ENCODE_BIT_KHR;
        barrier_to_transfer.srcAccessMask = VK_ACCESS_2_VIDEO_ENCODE_READ_BIT_KHR;
        barrier_to_transfer.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier_to_transfer.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier_to_transfer.oldLayout = VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR;
        barrier_to_transfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier_to_transfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier_to_transfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier_to_transfer.image = quantization_map.Image();
        barrier_to_transfer.subresourceRange = subres_range;
        auto dep_info_to_transfer = vku::InitStruct<VkDependencyInfo>();
        dep_info_to_transfer.imageMemoryBarrierCount = 1;
        dep_info_to_transfer.pImageMemoryBarriers = &barrier_to_transfer;
        vk::CmdPipelineBarrier2KHR(cb.handle(), &dep_info_to_transfer);

        cb.EndVideoCoding(context.End());

        vk::CmdClearColorImage(cb.handle(), quantization_map.Image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_value, 1,
                               &subres_range);

        cb.End();
    }

    if (!delta_config || (QueueFamilyFlags(delta_config.QueueFamilyIndex()) & VK_QUEUE_TRANSFER_BIT) == 0 || !emphasis_config ||
        (QueueFamilyFlags(emphasis_config.QueueFamilyIndex()) & VK_QUEUE_TRANSFER_BIT) == 0) {
        GTEST_SKIP() << "Not all quantization map types could be tested";
    }
}
