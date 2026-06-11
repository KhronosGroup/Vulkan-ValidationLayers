/*
 * Copyright (c) 2026 The Khronos Group Inc.
 * Copyright (c) 2026 RasterGrid Kft.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/video_objects.h"
#include "generated/enum_flag_bits.h"

class NegativeVideoEncodeFeedback2 : public VkVideoLayerTest {};

TEST_F(NegativeVideoEncodeFeedback2, CreateQueryPoolFeedbackFlagsFeatureNotEnabled) {
    TEST_DESCRIPTION("vkCreateQueryPool - creating query pool with feedback 2 flags requires videoEncodeFeedback2 feature");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_FEEDBACK_2_EXTENSION_NAME);
    ForceDisableFeature(vkt::Feature::videoEncodeFeedback2);

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return config.EncodeCaps()->supportedEncodeFeedbackFlags & VK_VIDEO_ENCODE_FEEDBACK_AVERAGE_QUANTIZATION_BIT_KHR;
    }));

    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile that support video encode feedback2 flags";
    }

    auto encode_feedback_info = vku::InitStruct<VkQueryPoolVideoEncodeFeedbackCreateInfoKHR>(config.Profile());
    encode_feedback_info.encodeFeedbackFlags = VK_VIDEO_ENCODE_FEEDBACK_AVERAGE_QUANTIZATION_BIT_KHR;

    auto create_info = vku::InitStruct<VkQueryPoolCreateInfo>(&encode_feedback_info);
    create_info.queryType = VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR;
    create_info.queryCount = 1;

    VkQueryPool query_pool{};
    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-queryType-12437");
    vk::CreateQueryPool(device(), &create_info, nullptr, &query_pool);
    vk::DestroyQueryPool(device(), query_pool, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeFeedback2, CreateQueryPoolPerPartitionFeatureNotEnabled) {
    TEST_DESCRIPTION(
        "vkCreateQueryPool - creating query pool with per partition feedback entries requires videoEncodeFeedback2 feature");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_FEEDBACK_2_EXTENSION_NAME);
    ForceDisableFeature(vkt::Feature::videoEncodeFeedback2);

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return config.EncodeFeedback2Caps()->maxPerPartitionFeedbackEntries != 0;
    }));

    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile that support non-zero maxPerPartitionFeedbackEntries";
    }

    auto per_partition_feedback_info = vku::InitStruct<VkQueryPoolVideoEncodePerPartitionFeedbackCreateInfoKHR>(config.Profile());
    per_partition_feedback_info.maxPerPartitionFeedbackEntries = 4;
    per_partition_feedback_info.perPartitionEncodeFeedbackFlags = VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BUFFER_OFFSET_BIT_KHR;

    auto encode_feedback_info = vku::InitStruct<VkQueryPoolVideoEncodeFeedbackCreateInfoKHR>(&per_partition_feedback_info);
    encode_feedback_info.encodeFeedbackFlags = VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BUFFER_OFFSET_BIT_KHR;

    auto create_info = vku::InitStruct<VkQueryPoolCreateInfo>(&encode_feedback_info);
    create_info.queryType = VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR;
    create_info.queryCount = 1;

    VkQueryPool query_pool;
    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-queryType-12438");
    vk::CreateQueryPool(device(), &create_info, nullptr, &query_pool);
    vk::DestroyQueryPool(device(), query_pool, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeFeedback2, MissingPerPartitionFeedbackFlags) {
    TEST_DESCRIPTION("vkCreateQueryPool - creating query pool with invalid per partition feedback entry flags");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_FEEDBACK_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeFeedback2);

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.EncodeFeedback2Caps()->maxPerPartitionFeedbackEntries != 0) &&
               (config.EncodeFeedback2Caps()->supportedPerPartitionEncodeFeedbackFlags != 0);
    }));

    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile that support non-zero maxPerPartitionFeedbackEntries";
    }

    auto per_partition_feedback_info = vku::InitStruct<VkQueryPoolVideoEncodePerPartitionFeedbackCreateInfoKHR>(config.Profile());
    per_partition_feedback_info.maxPerPartitionFeedbackEntries = 4;
    per_partition_feedback_info.perPartitionEncodeFeedbackFlags = 0;

    auto encode_feedback_info = vku::InitStruct<VkQueryPoolVideoEncodeFeedbackCreateInfoKHR>(&per_partition_feedback_info);
    encode_feedback_info.encodeFeedbackFlags = VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BUFFER_OFFSET_BIT_KHR;

    auto create_info = vku::InitStruct<VkQueryPoolCreateInfo>(&encode_feedback_info);
    create_info.queryType = VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR;
    create_info.queryCount = 1;

    VkQueryPool query_pool;
    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-queryType-12439");
    vk::CreateQueryPool(device(), &create_info, nullptr, &query_pool);
    vk::DestroyQueryPool(device(), query_pool, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeFeedback2, TooManyPerPartitionFeedbackEntries) {
    TEST_DESCRIPTION("vkCreateQueryPool - creating query pool with too many per partition feedback entries");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_FEEDBACK_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeFeedback2);

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.EncodeFeedback2Caps()->maxPerPartitionFeedbackEntries != 0) &&
               (config.EncodeFeedback2Caps()->supportedPerPartitionEncodeFeedbackFlags != 0);
    }));

    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile that support non-zero maxPerPartitionFeedbackEntries";
    }

    auto per_partition_feedback_info = vku::InitStruct<VkQueryPoolVideoEncodePerPartitionFeedbackCreateInfoKHR>(config.Profile());
    per_partition_feedback_info.maxPerPartitionFeedbackEntries = config.EncodeFeedback2Caps()->maxPerPartitionFeedbackEntries + 1;
    per_partition_feedback_info.perPartitionEncodeFeedbackFlags =
        VK_VIDEO_ENCODE_PER_PARTITION_FEEDBACK_BITSTREAM_BUFFER_OFFSET_BIT_KHR;

    auto encode_feedback_info = vku::InitStruct<VkQueryPoolVideoEncodeFeedbackCreateInfoKHR>(&per_partition_feedback_info);
    encode_feedback_info.encodeFeedbackFlags = VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BUFFER_OFFSET_BIT_KHR;

    auto create_info = vku::InitStruct<VkQueryPoolCreateInfo>(&encode_feedback_info);
    create_info.queryType = VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR;
    create_info.queryCount = 1;

    VkQueryPool query_pool;
    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-queryType-12440");
    vk::CreateQueryPool(device(), &create_info, nullptr, &query_pool);
    vk::DestroyQueryPool(device(), query_pool, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeFeedback2, UnsupportedPerPartitionFeedbackFlags) {
    TEST_DESCRIPTION("vkCreateQueryPool - creating query pool with unsupported per partition feedback flags");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_FEEDBACK_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeFeedback2);

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        const auto supported_per_partition_encode_feedback_flags =
            config.EncodeFeedback2Caps()->supportedPerPartitionEncodeFeedbackFlags;
        return (config.EncodeFeedback2Caps()->maxPerPartitionFeedbackEntries != 0) &&
               (supported_per_partition_encode_feedback_flags != 0) &&
               (supported_per_partition_encode_feedback_flags != AllVkVideoEncodePerPartitionFeedbackFlagBitsKHR);
    }));

    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile that support non-zero maxPerPartitionFeedbackEntries, but not all "
                        "feedback flags";
    }

    auto per_partition_feedback_info = vku::InitStruct<VkQueryPoolVideoEncodePerPartitionFeedbackCreateInfoKHR>(config.Profile());
    per_partition_feedback_info.maxPerPartitionFeedbackEntries = 4;
    per_partition_feedback_info.perPartitionEncodeFeedbackFlags = AllVkVideoEncodePerPartitionFeedbackFlagBitsKHR;

    auto encode_feedback_info = vku::InitStruct<VkQueryPoolVideoEncodeFeedbackCreateInfoKHR>(&per_partition_feedback_info);
    encode_feedback_info.encodeFeedbackFlags = VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BUFFER_OFFSET_BIT_KHR;

    auto create_info = vku::InitStruct<VkQueryPoolCreateInfo>(&encode_feedback_info);
    create_info.queryType = VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR;
    create_info.queryCount = 1;

    VkQueryPool query_pool;
    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-queryType-12441");
    vk::CreateQueryPool(device(), &create_info, nullptr, &query_pool);
    vk::DestroyQueryPool(device(), query_pool, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoEncodeFeedback2, CreateQueryPoolUnsupportedEncodeFeedback2Flags) {
    TEST_DESCRIPTION("vkCreateQueryPool - missing VkQueryPoolVideoEncodeFeedbackCreateInfoKHR");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_FEEDBACK_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeFeedback2);

    RETURN_IF_SKIP(Init());

    const auto kAllVideoEncodeFeedback2Flags =
        VK_VIDEO_ENCODE_FEEDBACK_AVERAGE_QUANTIZATION_BIT_KHR | VK_VIDEO_ENCODE_FEEDBACK_MIN_QUANTIZATION_BIT_KHR |
        VK_VIDEO_ENCODE_FEEDBACK_MAX_QUANTIZATION_BIT_KHR | VK_VIDEO_ENCODE_FEEDBACK_INTRA_PIXELS_BIT_KHR |
        VK_VIDEO_ENCODE_FEEDBACK_INTER_PIXELS_BIT_KHR | VK_VIDEO_ENCODE_FEEDBACK_SKIPPED_PIXELS_BIT_KHR |
        VK_VIDEO_ENCODE_FEEDBACK_PICTURE_PARTITION_COUNT_BIT_KHR;

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        return (config.EncodeCaps()->supportedEncodeFeedbackFlags & kAllVideoEncodeFeedback2Flags) != kAllVideoEncodeFeedback2Flags;
    }));
    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile that does not support all encode feedback 2 flags";
    }

    auto encode_feedback_info = vku::InitStruct<VkQueryPoolVideoEncodeFeedbackCreateInfoKHR>(config.Profile());
    encode_feedback_info.encodeFeedbackFlags = kAllVideoEncodeFeedback2Flags;

    auto create_info = vku::InitStruct<VkQueryPoolCreateInfo>(&encode_feedback_info);
    create_info.queryType = VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR;
    create_info.queryCount = 1;

    VkQueryPool query_pool;
    m_errorMonitor->SetDesiredError("VUID-VkQueryPoolCreateInfo-queryType-07907");
    vk::CreateQueryPool(device(), &create_info, nullptr, &query_pool);
    m_errorMonitor->VerifyFound();
}
