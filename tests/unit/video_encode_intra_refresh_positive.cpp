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

class PositiveVideoEncodeIntraRefresh : public VkVideoLayerTest {
  public:
    void TestIntraRefreshCycle(VideoConfig& config, uint32_t duration, uint32_t partition_count = 1) {
        // Use maximum extent to not run into other limitations related to granularity
        config.UpdateMaxCodedExtent(config.Caps()->maxCodedExtent);

        config.SessionCreateInfo()->maxDpbSlots = 2;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VideoContext context(m_device, config);
        context.CreateAndBindSessionMemory();
        context.CreateResources();

        vkt::CommandBuffer& cb = context.CmdBuffer();

        cb.Begin();
        cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 1));

        uint32_t ref_slot = 1;
        uint32_t target_slot = 0;

        cb.EncodeVideo(context.EncodeReferenceFrame(target_slot));

        for (uint32_t index = 0; index < duration; ++index) {
            std::swap(target_slot, ref_slot);

            if (index == 0) {
                // We refer to a non-intra-refreshed frame
                cb.EncodeVideo(context.EncodeReferenceFrame(target_slot)
                                   .SetPartitionCount(partition_count)
                                   .IntraRefresh(duration, index)
                                   .AddReferenceFrame(ref_slot));
            } else {
                // We refer to an intra-refreshed frame with dirty regions
                const uint32_t dirty_regions = duration - index;
                cb.EncodeVideo(context.EncodeReferenceFrame(target_slot)
                                   .SetPartitionCount(partition_count)
                                   .IntraRefresh(duration, index)
                                   .AddReferenceFrameWithDirtyRegions(ref_slot, dirty_regions));
            }
        }

        cb.EncodeVideo(context.EncodeReferenceFrame(0));

        cb.EndVideoCoding(context.End());
        cb.End();
    }
};

TEST_F(PositiveVideoEncodeIntraRefresh, BlockBased) {
    TEST_DESCRIPTION("Test block-based partition intra refresh modes");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithBlockBasedIntraRefresh(GetConfigsEncode()));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with block-based intra refresh modes";
    }

    config.SetVideoEncodeIntraRefreshMode(VK_VIDEO_ENCODE_INTRA_REFRESH_MODE_BLOCK_BASED_BIT_KHR);
    TestIntraRefreshCycle(config, config.EncodeIntraRefreshCaps()->maxIntraRefreshCycleDuration);
}

TEST_F(PositiveVideoEncodeIntraRefresh, BlockRowBased) {
    TEST_DESCRIPTION("Test block-row-based partition intra refresh modes");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    const VkVideoEncodeIntraRefreshModeFlagBitsKHR mode = VK_VIDEO_ENCODE_INTRA_REFRESH_MODE_BLOCK_ROW_BASED_BIT_KHR;
    VideoConfig config = GetConfig(GetConfigsWithBlockBasedIntraRefresh(GetConfigsEncode(), mode));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with block-row-based intra refresh modes";
    }

    config.SetVideoEncodeIntraRefreshMode(mode);
    TestIntraRefreshCycle(config, config.EncodeIntraRefreshCaps()->maxIntraRefreshCycleDuration);
}

TEST_F(PositiveVideoEncodeIntraRefresh, BlockColumnBased) {
    TEST_DESCRIPTION("Test block-column-based partition intra refresh modes");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    const VkVideoEncodeIntraRefreshModeFlagBitsKHR mode = VK_VIDEO_ENCODE_INTRA_REFRESH_MODE_BLOCK_COLUMN_BASED_BIT_KHR;
    VideoConfig config = GetConfig(GetConfigsWithBlockBasedIntraRefresh(GetConfigsEncode(), mode));
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support with block-column-based intra refresh modes";
    }

    config.SetVideoEncodeIntraRefreshMode(mode);
    TestIntraRefreshCycle(config, config.EncodeIntraRefreshCaps()->maxIntraRefreshCycleDuration);
}

TEST_F(PositiveVideoEncodeIntraRefresh, BlockBasedPartitionIndependentH264) {
    TEST_DESCRIPTION("Test partition-independent block-based intra refresh with multiple H.264 slices");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config =
        GetConfig(FilterConfigs(GetConfigsWithBlockBasedIntraRefresh(GetConfigsEncodeH264()), [](const VideoConfig& config) {
            return config.EncodeIntraRefreshCaps()->partitionIndependentIntraRefreshRegions == VK_TRUE &&
                   config.EncodeCapsH264()->maxSliceCount > 1;
        }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with partition-independent block-based intra refresh";
    }

    const uint32_t duration = config.EncodeIntraRefreshCaps()->maxIntraRefreshCycleDuration;
    const uint32_t max_slice_count = config.EncodeCapsH264()->maxSliceCount;
    const uint32_t slice_count = max_slice_count - ((duration == max_slice_count) ? 1 : 0);
    config.SetVideoEncodeIntraRefreshMode(VK_VIDEO_ENCODE_INTRA_REFRESH_MODE_BLOCK_BASED_BIT_KHR);
    TestIntraRefreshCycle(config, duration, slice_count);
}

TEST_F(PositiveVideoEncodeIntraRefresh, BlockBasedPartitionIndependentH265) {
    TEST_DESCRIPTION("Test partition-independent block-based intra refresh with multiple H.265 slice segments");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config =
        GetConfig(FilterConfigs(GetConfigsWithBlockBasedIntraRefresh(GetConfigsEncodeH265()), [](const VideoConfig& config) {
            return config.EncodeIntraRefreshCaps()->partitionIndependentIntraRefreshRegions == VK_TRUE &&
                   config.EncodeCapsH265()->maxSliceSegmentCount > 1 &&
                   (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_MULTIPLE_SLICE_SEGMENTS_PER_TILE_BIT_KHR) != 0;
        }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with partition-independent block-based intra refresh";
    }

    const uint32_t duration = config.EncodeIntraRefreshCaps()->maxIntraRefreshCycleDuration;
    const uint32_t max_slice_segment_count = config.EncodeCapsH265()->maxSliceSegmentCount;
    const uint32_t slice_segment_count = max_slice_segment_count - ((duration == max_slice_segment_count) ? 1 : 0);
    config.SetVideoEncodeIntraRefreshMode(VK_VIDEO_ENCODE_INTRA_REFRESH_MODE_BLOCK_BASED_BIT_KHR);
    TestIntraRefreshCycle(config, duration, slice_segment_count);
}

TEST_F(PositiveVideoEncodeIntraRefresh, PerPicturePartitionH264) {
    TEST_DESCRIPTION("Test per-picture-partition intra refresh mode with H.264 encode profile");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithPerPartitionIntraRefresh(GetConfigsEncodeH264()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with per-picture-partition intra refresh mode";
    }

    const uint32_t duration =
        std::min(config.EncodeIntraRefreshCaps()->maxIntraRefreshCycleDuration, config.EncodeCapsH264()->maxSliceCount);
    const uint32_t slice_count = duration;
    config.SetVideoEncodeIntraRefreshMode(VK_VIDEO_ENCODE_INTRA_REFRESH_MODE_PER_PICTURE_PARTITION_BIT_KHR);
    TestIntraRefreshCycle(config, duration, slice_count);
}

TEST_F(PositiveVideoEncodeIntraRefresh, PerPicturePartitionH265) {
    TEST_DESCRIPTION("Test per-picture-partition intra refresh mode with H.265 encode profile");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithPerPartitionIntraRefresh(GetConfigsEncodeH265()));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with per-picture-partition intra refresh mode";
    }

    const uint32_t duration =
        std::min(config.EncodeIntraRefreshCaps()->maxIntraRefreshCycleDuration, config.EncodeCapsH265()->maxSliceSegmentCount);
    const uint32_t slice_segment_count = duration;
    config.SetVideoEncodeIntraRefreshMode(VK_VIDEO_ENCODE_INTRA_REFRESH_MODE_PER_PICTURE_PARTITION_BIT_KHR);
    TestIntraRefreshCycle(config, duration, slice_segment_count);
}

TEST_F(PositiveVideoEncodeIntraRefresh, EncodeCapsDifferentSliceTypesH264) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - using different H.264 slice types is not supported, but slice is intra refreshed");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config =
        GetConfig(FilterConfigs(GetConfigsWithPerPartitionIntraRefresh(GetConfigsEncodeH264()), [](const VideoConfig& config) {
            return (config.EncodeCapsH264()->flags & VK_VIDEO_ENCODE_H264_CAPABILITY_DIFFERENT_SLICE_TYPE_BIT_KHR) == 0 &&
                   config.EncodeCapsH264()->maxSliceCount > 1;
        }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.264 encode support with intra refresh but no different slice types";
    }

    config.SetVideoEncodeIntraRefreshMode(VK_VIDEO_ENCODE_INTRA_REFRESH_MODE_PER_PICTURE_PARTITION_BIT_KHR);

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame().IntraRefresh(2, 0);
    auto& slice_headers = encode_info.CodecInfo().encode_h264.std_slice_headers;

    slice_headers[0].slice_type = STD_VIDEO_H264_SLICE_TYPE_I;
    slice_headers[1].slice_type = STD_VIDEO_H264_SLICE_TYPE_P;

    cb.EncodeVideo(encode_info);

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(PositiveVideoEncodeIntraRefresh, EncodeCapsDifferentSliceTypesH265) {
    TEST_DESCRIPTION("vkCmdEncodeVideoKHR - using different H.265 slice types is not supported, but slice is intra refreshed");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_INTRA_REFRESH_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeIntraRefresh);
    RETURN_IF_SKIP(Init());

    VideoConfig config =
        GetConfig(FilterConfigs(GetConfigsWithPerPartitionIntraRefresh(GetConfigsEncodeH265()), [](const VideoConfig& config) {
            return (config.EncodeCapsH265()->flags & VK_VIDEO_ENCODE_H265_CAPABILITY_DIFFERENT_SLICE_SEGMENT_TYPE_BIT_KHR) == 0 &&
                   config.EncodeCapsH265()->maxSliceSegmentCount > 1;
        }));
    if (!config) {
        GTEST_SKIP() << "Test requires H.265 encode support with multiple slice support but no different slice types";
    }

    config.SetVideoEncodeIntraRefreshMode(VK_VIDEO_ENCODE_INTRA_REFRESH_MODE_PER_PICTURE_PARTITION_BIT_KHR);

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    VideoEncodeInfo encode_info = context.EncodeFrame().IntraRefresh(2, 0);
    auto& slice_segment_headers = encode_info.CodecInfo().encode_h265.std_slice_segment_headers;

    slice_segment_headers[0].slice_type = STD_VIDEO_H265_SLICE_TYPE_I;
    slice_segment_headers[1].slice_type = STD_VIDEO_H265_SLICE_TYPE_P;

    cb.EncodeVideo(encode_info);

    cb.EndVideoCoding(context.End());
    cb.End();
}
