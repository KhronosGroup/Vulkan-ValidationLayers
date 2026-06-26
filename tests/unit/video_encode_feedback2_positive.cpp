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

#include "video_objects.h"
#include "generated/enum_flag_bits.h"

class PositiveVideoEncodeFeedback2 : public VkVideoLayerTest {};

TEST_F(PositiveVideoEncodeFeedback2, CreateQueryWithFeedback2Flag) {
    TEST_DESCRIPTION("vkCreateQueryPool - creating query pool with feedback 2 flags requiring videoEncodeFeedback2 feature");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_FEEDBACK_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeFeedback2);

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
    vk::CreateQueryPool(device(), &create_info, nullptr, &query_pool);
    vk::DestroyQueryPool(device(), query_pool, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(PositiveVideoEncodeFeedback2, CreateQueryWithPerPartitionFeedback) {
    TEST_DESCRIPTION("vkCreateQueryPool - creating query pool with partition feedback entries");

    AddRequiredExtensions(VK_KHR_VIDEO_ENCODE_FEEDBACK_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoEncodeFeedback2);

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(FilterConfigs(GetConfigsEncode(), [](const VideoConfig& config) {
        const auto supported_per_partition_encode_feedback_flags =
            config.EncodeFeedback2Caps()->supportedPerPartitionEncodeFeedbackFlags;
        return (config.EncodeFeedback2Caps()->maxPerPartitionFeedbackEntries != 0) &&
               (supported_per_partition_encode_feedback_flags != 0);
    }));

    if (!config) {
        GTEST_SKIP() << "Test requires a video encode profile that support non-zero maxPerPartitionFeedbackEntries";
    }

    auto per_partition_feedback_info = vku::InitStruct<VkQueryPoolVideoEncodePerPartitionFeedbackCreateInfoKHR>(config.Profile());
    per_partition_feedback_info.maxPerPartitionFeedbackEntries = config.EncodeFeedback2Caps()->maxPerPartitionFeedbackEntries;
    per_partition_feedback_info.perPartitionEncodeFeedbackFlags =
        config.EncodeFeedback2Caps()->supportedPerPartitionEncodeFeedbackFlags;

    auto encode_feedback_info = vku::InitStruct<VkQueryPoolVideoEncodeFeedbackCreateInfoKHR>(&per_partition_feedback_info);
    encode_feedback_info.encodeFeedbackFlags = config.EncodeCaps()->supportedEncodeFeedbackFlags;

    auto create_info = vku::InitStruct<VkQueryPoolCreateInfo>(&encode_feedback_info);
    create_info.queryType = VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR;
    create_info.queryCount = 1;

    VkQueryPool query_pool;
    vk::CreateQueryPool(device(), &create_info, nullptr, &query_pool);
    vk::DestroyQueryPool(device(), query_pool, nullptr);
    m_errorMonitor->VerifyFound();
}
