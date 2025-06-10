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

class PositiveVideoDecode : public VkVideoLayerTest {};

TEST_F(PositiveVideoDecode, ProfileIndependentResources) {
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

TEST_F(PositiveVideoDecode, DecodeImageLayouts) {
    TEST_DESCRIPTION("vkCmdDecodeVideoKHR with VK_KHR_unified_image_layouts");

    AddRequiredExtensions(VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::unifiedImageLayoutsVideo);
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

    vk::CmdPipelineBarrier2KHR(cb, context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR));
    vk::CmdPipelineBarrier2KHR(cb, context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR));

    cb.DecodeVideo(context.DecodeFrame(0));

    // Decode output must be in DECODE_DST layout if it is distinct from reconstructed
    if (config.SupportsDecodeOutputDistinct()) {
        vk::CmdPipelineBarrier2KHR(cb, context.DecodeOutput()->LayoutTransition(VK_IMAGE_LAYOUT_GENERAL));
        cb.DecodeVideo(context.DecodeFrame(0));
        m_errorMonitor->VerifyFound();
        vk::CmdPipelineBarrier2KHR(cb, context.Dpb()->LayoutTransition(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, 0, 1));
    }
}