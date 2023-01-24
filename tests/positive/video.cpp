/*
 * Copyright (c) 2022-2023 The Khronos Group Inc.
 * Copyright (c) 2022-2023 RasterGrid Kft.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Daniel Rakos <daniel.rakos@rastergrid.com>
 */

#include "../vklayertests_video.h"
#include "vk_extension_helper.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>

#include "cast_utils.h"

class VkPositiveVideoLayerTest : public VkVideoLayerTest {};

TEST_F(VkPositiveVideoLayerTest, VideoCodingScope) {
    TEST_DESCRIPTION("Tests calling functions inside/outside video coding scope");

    ASSERT_NO_FATAL_FAILURE(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(DeviceObj(), GetConfigDecode());
    context.CreateAndBindSessionMemory();

    VkCommandBufferObj& cb = context.CmdBuffer();

    cb.begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset());
    cb.EndVideoCoding(context.End());
    cb.end();
}

TEST_F(VkPositiveVideoLayerTest, VideoDecodeH264) {
    TEST_DESCRIPTION("Tests basic H.264/AVC video decode use case for framework verification purposes");

    ASSERT_NO_FATAL_FAILURE(Init());

    VideoConfig config = GetConfigDecodeH264();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 decode support";
    }

    const uint32_t maxDpbSlots = 3;
    const uint32_t maxActiveRefs = 2;

    if (config.Caps()->maxDpbSlots < maxDpbSlots || config.Caps()->maxActiveReferencePictures < maxActiveRefs) {
        GTEST_SKIP() << "Test requires at least 3 DPB slots and 2 active references";
    }

    config.SessionCreateInfo()->maxDpbSlots = maxDpbSlots;
    config.SessionCreateInfo()->maxActiveReferencePictures = maxActiveRefs;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VkCommandBufferObj& cb = context.CmdBuffer();

    cb.begin();
    cb.PipelineBarrier2KHR(context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    cb.PipelineBarrier2KHR(context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR));
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1).AddResource(-1, 2));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(0));
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(1).AddReferenceFrame(0));
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(0));
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(2).AddReferenceFrame(0));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().submit(cb);
    m_device->wait();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).InvalidateSlot(1).AddResource(-1, 1).AddResource(2, 2));
    cb.DecodeVideo(context.DecodeFrame());
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(1).AddReferenceFrame(0).AddReferenceFrame(2));
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().submit(cb);
    m_device->wait();
}

TEST_F(VkPositiveVideoLayerTest, VideoDecodeH264Interlaced) {
    TEST_DESCRIPTION("Tests basic H.264/AVC interlaced video decode use case for framework verification purposes");

    ASSERT_NO_FATAL_FAILURE(Init());

    VideoConfig config = GetConfigDecodeH264Interlaced();
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 interlaced decode support";
    }

    const uint32_t maxDpbSlots = 3;
    const uint32_t maxActiveRefs = 2;

    if (config.Caps()->maxDpbSlots < maxDpbSlots || config.Caps()->maxActiveReferencePictures < maxActiveRefs) {
        GTEST_SKIP() << "Test requires at least 3 DPB slots and 2 active references";
    }

    config.SessionCreateInfo()->maxDpbSlots = maxDpbSlots;
    config.SessionCreateInfo()->maxActiveReferencePictures = maxActiveRefs;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VkCommandBufferObj& cb = context.CmdBuffer();

    cb.begin();
    cb.PipelineBarrier2KHR(context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    cb.PipelineBarrier2KHR(context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR));
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.DecodeVideo(context.DecodeTopField().SetupTopField(0));
    cb.DecodeVideo(context.DecodeBottomField().SetupBottomField(0).AddReferenceTopField(0));
    cb.DecodeVideo(context.DecodeFrame().AddReferenceBottomField(0));
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(1).AddReferenceBothFields(0));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().submit(cb);
    m_device->wait();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().InvalidateSlot(0).AddResource(-1, 0).AddResource(1, 1));
    cb.DecodeVideo(context.DecodeFrame());
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(0).AddReferenceFrame(1));
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(0));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().submit(cb);
    m_device->wait();
}

TEST_F(VkPositiveVideoLayerTest, VideoDecodeH265) {
    TEST_DESCRIPTION("Tests basic H.265/HEVC video decode use case for framework verification purposes");

    ASSERT_NO_FATAL_FAILURE(Init());

    VideoConfig config = GetConfigDecodeH265();
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 decode support";
    }

    const uint32_t maxDpbSlots = 3;
    const uint32_t maxActiveRefs = 2;

    if (config.Caps()->maxDpbSlots < maxDpbSlots || config.Caps()->maxActiveReferencePictures < maxActiveRefs) {
        GTEST_SKIP() << "Test requires at least 3 DPB slots and 2 active references";
    }

    config.SessionCreateInfo()->maxDpbSlots = maxDpbSlots;
    config.SessionCreateInfo()->maxActiveReferencePictures = maxActiveRefs;

    VideoContext context(DeviceObj(), config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VkCommandBufferObj& cb = context.CmdBuffer();

    cb.begin();
    cb.PipelineBarrier2KHR(context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));
    cb.PipelineBarrier2KHR(context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR));
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(-1, 1).AddResource(-1, 2));
    cb.ControlVideoCoding(context.Control().Reset());
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(0));
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(0));
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(1).AddReferenceFrame(0));
    cb.DecodeVideo(context.DecodeFrame());
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(2).AddReferenceFrame(0).AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().submit(cb);
    m_device->wait();

    cb.begin();
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).InvalidateSlot(1).AddResource(-1, 1).AddResource(2, 2));
    cb.DecodeVideo(context.DecodeFrame());
    cb.DecodeVideo(context.DecodeFrame().SetupFrame(1).AddReferenceFrame(0).AddReferenceFrame(2));
    cb.DecodeVideo(context.DecodeFrame().AddReferenceFrame(1));
    cb.EndVideoCoding(context.End());
    cb.end();
    context.Queue().submit(cb);
    m_device->wait();
}
