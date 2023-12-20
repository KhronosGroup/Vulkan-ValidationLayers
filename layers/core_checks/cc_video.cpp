/* Copyright (c) 2022-2023 The Khronos Group Inc.
 * Copyright (c) 2022-2023 RasterGrid Kft.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <assert.h>
#include <vector>

#include <vulkan/vk_enum_string_helper.h>
#include "generated/chassis.h"
#include "core_validation.h"

// Flags validation error if the associated call is made inside a video coding block.
// The apiName routine should ONLY be called outside a video coding block.
bool CoreChecks::InsideVideoCodingScope(const vvl::CommandBuffer &cb_state, const Location &loc, const char *vuid) const {
    bool inside = false;
    if (cb_state.bound_video_session) {
        inside = LogError(vuid, cb_state.commandBuffer(), loc, "It is invalid to issue this call inside a video coding block.");
    }
    return inside;
}

// Flags validation error if the associated call is made outside a video coding block.
// The apiName routine should ONLY be called inside a video coding block.
bool CoreChecks::OutsideVideoCodingScope(const vvl::CommandBuffer &cb_state, const Location &loc, const char *vuid) const {
    bool outside = false;
    if (!cb_state.bound_video_session) {
        outside = LogError(vuid, cb_state.commandBuffer(), loc, "This call must be issued inside a video coding block.");
    }
    return outside;
}

std::vector<VkVideoFormatPropertiesKHR> CoreChecks::GetVideoFormatProperties(VkImageUsageFlags image_usage,
                                                                             const VkVideoProfileListInfoKHR *profile_list) const {
    VkPhysicalDeviceVideoFormatInfoKHR format_info = vku::InitStructHelper();
    format_info.imageUsage = image_usage;
    format_info.pNext = profile_list;

    uint32_t format_count = 0;
    DispatchGetPhysicalDeviceVideoFormatPropertiesKHR(physical_device, &format_info, &format_count, nullptr);
    std::vector<VkVideoFormatPropertiesKHR> format_props(format_count, vku::InitStruct<VkVideoFormatPropertiesKHR>());
    DispatchGetPhysicalDeviceVideoFormatPropertiesKHR(physical_device, &format_info, &format_count, format_props.data());

    return format_props;
}

std::vector<VkVideoFormatPropertiesKHR> CoreChecks::GetVideoFormatProperties(VkImageUsageFlags image_usage,
                                                                             const VkVideoProfileInfoKHR *profile) const {
    VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
    profile_list.profileCount = 1;
    profile_list.pProfiles = profile;

    return GetVideoFormatProperties(image_usage, &profile_list);
}

bool CoreChecks::IsVideoFormatSupported(VkFormat format, VkImageUsageFlags image_usage,
                                        const VkVideoProfileInfoKHR *profile) const {
    auto format_props_list = GetVideoFormatProperties(image_usage, profile);
    for (const auto &format_props : format_props_list) {
        if (format_props.format == format) return true;
    }
    return false;
}

bool CoreChecks::IsBufferCompatibleWithVideoProfile(const vvl::Buffer &buffer_state,
                                                    const std::shared_ptr<const vvl::VideoProfileDesc> &video_profile) const {
    return (buffer_state.createInfo.flags & VK_BUFFER_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR) ||
           buffer_state.supported_video_profiles.find(video_profile) != buffer_state.supported_video_profiles.end();
}

bool CoreChecks::IsImageCompatibleWithVideoProfile(const vvl::Image &image_state,
                                                   const std::shared_ptr<const vvl::VideoProfileDesc> &video_profile) const {
    return (image_state.createInfo.flags & VK_IMAGE_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR) ||
           image_state.supported_video_profiles.find(video_profile) != image_state.supported_video_profiles.end();
}

void CoreChecks::EnqueueVerifyVideoSessionInitialized(vvl::CommandBuffer &cb_state, vvl::VideoSession &vs_state, const char *vuid) {
    cb_state.video_session_updates[vs_state.videoSession()].emplace_back(
        [vuid](const ValidationStateTracker *dev_data, const vvl::VideoSession *vs_state, vvl::VideoSessionDeviceState &dev_state,
               bool do_validate) {
            bool skip = false;
            if (!dev_state.IsInitialized()) {
                skip |= dev_data->LogError(vs_state->Handle(), vuid, "Bound video session %s is uninitialized.",
                                           dev_data->FormatHandle(*vs_state).c_str());
            }
            return skip;
        });
}

void CoreChecks::EnqueueVerifyVideoInlineQueryUnavailable(vvl::CommandBuffer &cb_state, const VkVideoInlineQueryInfoKHR &query_info,
                                                          Func command) {
    if (disabled[query_validation]) return;
    cb_state.queryUpdates.emplace_back([query_info, command](vvl::CommandBuffer &cb_state_arg, bool do_validate,
                                                             VkQueryPool &firstPerfQueryPool, uint32_t perfPass,
                                                             QueryMap *localQueryToStateMap) {
        if (!do_validate) return false;
        bool skip = false;
        for (uint32_t i = 0; i < query_info.queryCount; i++) {
            QueryObject query_obj = {query_info.queryPool, query_info.firstQuery + i, perfPass};
            skip |= VerifyQueryIsReset(cb_state_arg, query_obj, command, firstPerfQueryPool, perfPass, localQueryToStateMap);
        }
        return skip;
    });
}

bool CoreChecks::ValidateVideoInlineQueryInfo(const vvl::QueryPool &query_pool_state, const VkVideoInlineQueryInfoKHR &query_info,
                                              const Location &loc) const {
    bool skip = false;

    if (query_info.firstQuery >= query_pool_state.createInfo.queryCount) {
        skip |= LogError("VUID-VkVideoInlineQueryInfoKHR-queryPool-08372", query_pool_state.pool(), loc.dot(Field::firstQuery),
                         "(%u) is greater than or equal to the number of queries (%u) in %s.", query_info.firstQuery,
                         query_pool_state.createInfo.queryCount, FormatHandle(query_pool_state).c_str());
    }

    if (query_info.firstQuery + query_info.queryCount > query_pool_state.createInfo.queryCount) {
        skip |= LogError("VUID-VkVideoInlineQueryInfoKHR-queryPool-08373", query_pool_state.pool(), loc.dot(Field::firstQuery),
                         "(%u) plus queryCount (%u) is greater than the number of queries (%u) in %s.", query_info.firstQuery,
                         query_info.queryCount, query_pool_state.createInfo.queryCount, FormatHandle(query_pool_state).c_str());
    }

    return skip;
}

bool CoreChecks::ValidateVideoEncodeRateControlInfo(const VkVideoEncodeRateControlInfoKHR &rc_info, const void *pNext,
                                                    VkCommandBuffer cmdbuf, const vvl::VideoSession &vs_state,
                                                    const Location &loc) const {
    bool skip = false;

    const Location rc_info_loc = loc.pNext(Struct::VkVideoEncodeRateControlInfoKHR);

    const auto &profile_caps = vs_state.profile->GetCapabilities();

    if (rc_info.layerCount > profile_caps.encode.maxRateControlLayers) {
        const LogObjectList objlist(cmdbuf, vs_state.videoSession());
        skip |= LogError("VUID-VkVideoEncodeRateControlInfoKHR-layerCount-08245", objlist, rc_info_loc.dot(Field::layerCount),
                         "(%u) is greater than the maxRateControlLayers (%u) "
                         "supported by the video profile %s was created with.",
                         rc_info.layerCount, profile_caps.encode.maxRateControlLayers, FormatHandle(vs_state).c_str());
    }

    if ((rc_info.rateControlMode == VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DEFAULT_KHR ||
         rc_info.rateControlMode == VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR) &&
        rc_info.layerCount != 0) {
        skip |=
            LogError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08248", cmdbuf, rc_info_loc.dot(Field::rateControlMode),
                     "is %s, but %s (%u) is not zero.", string_VkVideoEncodeRateControlModeFlagBitsKHR(rc_info.rateControlMode),
                     rc_info_loc.dot(Field::layerCount).Fields().c_str(), rc_info.layerCount);
    }

    if (rc_info.rateControlMode == VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR ||
        rc_info.rateControlMode == VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR) {
        if (rc_info.layerCount != 0) {
            for (uint32_t layer_idx = 0; layer_idx < rc_info.layerCount; ++layer_idx) {
                skip |= ValidateVideoEncodeRateControlLayerInfo(layer_idx, rc_info, pNext, cmdbuf, vs_state, rc_info_loc);
            }
        } else {
            skip |= LogError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08275", cmdbuf,
                             rc_info_loc.dot(Field::rateControlMode), "is %s, but %s is zero.",
                             string_VkVideoEncodeRateControlModeFlagBitsKHR(rc_info.rateControlMode),
                             rc_info_loc.dot(Field::layerCount).Fields().c_str());
        }
    }

    if (rc_info.rateControlMode != VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DEFAULT_KHR) {
        switch (vs_state.GetCodecOp()) {
            case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
                skip |= ValidateVideoEncodeRateControlInfoH264(rc_info, pNext, cmdbuf, vs_state, loc);
                break;

            case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
                skip |= ValidateVideoEncodeRateControlInfoH265(rc_info, pNext, cmdbuf, vs_state, loc);
                break;

            default:
                assert(false);
                break;
        }
        if ((profile_caps.encode.rateControlModes & rc_info.rateControlMode) == 0) {
            const LogObjectList objlist(cmdbuf, vs_state.videoSession());
            skip |=
                LogError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08244", objlist,
                         rc_info_loc.dot(Field::rateControlMode), "(%s) is not supported by the video profile %s was created with.",
                         string_VkVideoEncodeRateControlModeFlagBitsKHR(rc_info.rateControlMode), FormatHandle(vs_state).c_str());
        }
    }

    if (rc_info.layerCount != 0) {
        if (rc_info.virtualBufferSizeInMs == 0) {
            skip |= LogError("VUID-VkVideoEncodeRateControlInfoKHR-layerCount-08357", cmdbuf,
                             rc_info_loc.dot(Field::virtualBufferSizeInMs), "must not be zero if %s (%u) is not zero.",
                             rc_info_loc.dot(Field::layerCount).Fields().c_str(), rc_info.layerCount);
        }
        if (rc_info.initialVirtualBufferSizeInMs >= rc_info.virtualBufferSizeInMs) {
            skip |= LogError("VUID-VkVideoEncodeRateControlInfoKHR-layerCount-08358", cmdbuf,
                             rc_info_loc.dot(Field::initialVirtualBufferSizeInMs),
                             "(%u) must be less than virtualBufferSizeInMs (%u) if %s (%u) is not zero.",
                             rc_info.initialVirtualBufferSizeInMs, rc_info.virtualBufferSizeInMs,
                             rc_info_loc.dot(Field::layerCount).Fields().c_str(), rc_info.layerCount);
        }
    }

    return skip;
}

bool CoreChecks::ValidateVideoEncodeRateControlInfoH264(const VkVideoEncodeRateControlInfoKHR &rc_info, const void *pNext,
                                                        VkCommandBuffer cmdbuf, const vvl::VideoSession &vs_state,
                                                        const Location &loc) const {
    bool skip = false;

    const auto rc_info_h264 = vku::FindStructInPNextChain<VkVideoEncodeH264RateControlInfoKHR>(pNext);
    if (rc_info_h264 == nullptr) return false;

    const auto rc_info_h264_loc = loc.pNext(Struct::VkVideoEncodeH264RateControlInfoKHR);

    const auto &profile_caps = vs_state.profile->GetCapabilities();

    if (rc_info_h264->flags & VK_VIDEO_ENCODE_H264_RATE_CONTROL_ATTEMPT_HRD_COMPLIANCE_BIT_KHR &&
        (profile_caps.encode_h264.flags & VK_VIDEO_ENCODE_H264_CAPABILITY_HRD_COMPLIANCE_BIT_KHR) == 0) {
        const LogObjectList objlist(cmdbuf, vs_state.videoSession());
        skip |= LogError("VUID-VkVideoEncodeH264RateControlInfoKHR-flags-08280", objlist, rc_info_h264_loc.dot(Field::flags),
                         "includes VK_VIDEO_ENCODE_H264_RATE_CONTROL_ATTEMPT_HRD_COMPLIANCE_BIT_KHR but HRD compliance "
                         "is not supported by the H.264 encode profile %s was created with.",
                         FormatHandle(vs_state).c_str());
    }

    if ((rc_info_h264->flags & VK_VIDEO_ENCODE_H264_RATE_CONTROL_REFERENCE_PATTERN_FLAT_BIT_KHR ||
         rc_info_h264->flags & VK_VIDEO_ENCODE_H264_RATE_CONTROL_REFERENCE_PATTERN_DYADIC_BIT_KHR) &&
        (rc_info_h264->flags & VK_VIDEO_ENCODE_H264_RATE_CONTROL_REGULAR_GOP_BIT_KHR) == 0) {
        skip |= LogError("VUID-VkVideoEncodeH264RateControlInfoKHR-flags-08281", cmdbuf, rc_info_h264_loc.dot(Field::flags),
                         "(%s) specifies a reference pattern but does not indicate the use of a regular GOP structure.",
                         string_VkVideoEncodeH264RateControlFlagsKHR(rc_info_h264->flags).c_str());
    }

    if (rc_info_h264->flags & VK_VIDEO_ENCODE_H264_RATE_CONTROL_REFERENCE_PATTERN_FLAT_BIT_KHR &&
        rc_info_h264->flags & VK_VIDEO_ENCODE_H264_RATE_CONTROL_REFERENCE_PATTERN_DYADIC_BIT_KHR) {
        skip |= LogError("VUID-VkVideoEncodeH264RateControlInfoKHR-flags-08282", cmdbuf, rc_info_h264_loc.dot(Field::flags),
                         "(%s) indicates conflicting reference patterns.",
                         string_VkVideoEncodeH264RateControlFlagsKHR(rc_info_h264->flags).c_str());
    }

    if (rc_info_h264->flags & VK_VIDEO_ENCODE_H264_RATE_CONTROL_REGULAR_GOP_BIT_KHR && rc_info_h264->gopFrameCount == 0) {
        skip |= LogError("VUID-VkVideoEncodeH264RateControlInfoKHR-flags-08283", cmdbuf, rc_info_h264_loc.dot(Field::flags),
                         "includes VK_VIDEO_ENCODE_H264_RATE_CONTROL_REGULAR_GOP_BIT_KHR but the GOP size "
                         "specified in %s is zero.",
                         rc_info_h264_loc.dot(Field::gopFrameCount).Fields().c_str());
    }

    if (rc_info_h264->idrPeriod != 0 && rc_info_h264->idrPeriod < rc_info_h264->gopFrameCount) {
        skip |= LogError("VUID-VkVideoEncodeH264RateControlInfoKHR-idrPeriod-08284", cmdbuf, rc_info_h264_loc.dot(Field::idrPeriod),
                         "(%u) is not zero and the specified IDR period smaller than the GOP size specified in %s (%u).",
                         rc_info_h264->idrPeriod, rc_info_h264_loc.dot(Field::gopFrameCount).Fields().c_str(),
                         rc_info_h264->gopFrameCount);
    }

    if (rc_info_h264->consecutiveBFrameCount != 0 && rc_info_h264->consecutiveBFrameCount >= rc_info_h264->gopFrameCount) {
        skip |=
            LogError("VUID-VkVideoEncodeH264RateControlInfoKHR-consecutiveBFrameCount-08285", cmdbuf,
                     rc_info_h264_loc.dot(Field::consecutiveBFrameCount),
                     "(%u) is greater than or equal to the GOP size specified in %s (%u).", rc_info_h264->consecutiveBFrameCount,
                     rc_info_h264_loc.dot(Field::gopFrameCount).Fields().c_str(), rc_info_h264->gopFrameCount);
    }

    if (rc_info.layerCount > 1 && rc_info.layerCount != rc_info_h264->temporalLayerCount) {
        skip |= LogError(
            "VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-07022", cmdbuf,
            rc_info_h264_loc.dot(Field::temporalLayerCount), "(%u) does not match %s (%u).", rc_info_h264->temporalLayerCount,
            loc.pNext(Struct::VkVideoEncodeRateControlInfoKHR, Field::layerCount).Fields().c_str(), rc_info.layerCount);
    }

    return skip;
}

bool CoreChecks::ValidateVideoEncodeRateControlInfoH265(const VkVideoEncodeRateControlInfoKHR &rc_info, const void *pNext,
                                                        VkCommandBuffer cmdbuf, const vvl::VideoSession &vs_state,
                                                        const Location &loc) const {
    bool skip = false;

    const auto rc_info_h265 = vku::FindStructInPNextChain<VkVideoEncodeH265RateControlInfoKHR>(pNext);
    if (rc_info_h265 == nullptr) return false;

    const auto rc_info_h265_loc = loc.pNext(Struct::VkVideoEncodeH264RateControlInfoKHR);

    const auto &profile_caps = vs_state.profile->GetCapabilities();

    if (rc_info_h265->flags & VK_VIDEO_ENCODE_H265_RATE_CONTROL_ATTEMPT_HRD_COMPLIANCE_BIT_KHR &&
        (profile_caps.encode_h265.flags & VK_VIDEO_ENCODE_H265_CAPABILITY_HRD_COMPLIANCE_BIT_KHR) == 0) {
        const LogObjectList objlist(cmdbuf, vs_state.videoSession());
        skip |= LogError("VUID-VkVideoEncodeH265RateControlInfoKHR-flags-08291", objlist, rc_info_h265_loc.dot(Field::flags),
                         "includes VK_VIDEO_ENCODE_H265_RATE_CONTROL_ATTEMPT_HRD_COMPLIANCE_BIT_KHR but HRD compliance "
                         "is not supported by the H.265 encode profile %s was created with.",
                         FormatHandle(vs_state).c_str());
    }

    if ((rc_info_h265->flags & VK_VIDEO_ENCODE_H265_RATE_CONTROL_REFERENCE_PATTERN_FLAT_BIT_KHR ||
         rc_info_h265->flags & VK_VIDEO_ENCODE_H265_RATE_CONTROL_REFERENCE_PATTERN_DYADIC_BIT_KHR) &&
        (rc_info_h265->flags & VK_VIDEO_ENCODE_H265_RATE_CONTROL_REGULAR_GOP_BIT_KHR) == 0) {
        skip |= LogError("VUID-VkVideoEncodeH265RateControlInfoKHR-flags-08292", cmdbuf, rc_info_h265_loc.dot(Field::flags),
                         "(%s) specifies a reference pattern but does not indicate the use of a regular GOP structure.",
                         string_VkVideoEncodeH265RateControlFlagsKHR(rc_info_h265->flags).c_str());
    }

    if (rc_info_h265->flags & VK_VIDEO_ENCODE_H265_RATE_CONTROL_REFERENCE_PATTERN_FLAT_BIT_KHR &&
        rc_info_h265->flags & VK_VIDEO_ENCODE_H265_RATE_CONTROL_REFERENCE_PATTERN_DYADIC_BIT_KHR) {
        skip |= LogError("VUID-VkVideoEncodeH265RateControlInfoKHR-flags-08293", cmdbuf, rc_info_h265_loc.dot(Field::flags),
                         "(%s) indicates conflicting reference patterns.",
                         string_VkVideoEncodeH265RateControlFlagsKHR(rc_info_h265->flags).c_str());
    }

    if (rc_info_h265->flags & VK_VIDEO_ENCODE_H265_RATE_CONTROL_REGULAR_GOP_BIT_KHR && rc_info_h265->gopFrameCount == 0) {
        skip |= LogError("VUID-VkVideoEncodeH265RateControlInfoKHR-flags-08294", cmdbuf, rc_info_h265_loc.dot(Field::flags),
                         "includes VK_VIDEO_ENCODE_H265_RATE_CONTROL_REGULAR_GOP_BIT_KHR but the GOP size "
                         "specified in %s is zero.",
                         rc_info_h265_loc.dot(Field::gopFrameCount).Fields().c_str());
    }

    if (rc_info_h265->idrPeriod != 0 && rc_info_h265->idrPeriod < rc_info_h265->gopFrameCount) {
        skip |= LogError("VUID-VkVideoEncodeH265RateControlInfoKHR-idrPeriod-08295", cmdbuf, rc_info_h265_loc.dot(Field::idrPeriod),
                         "(%u) is not zero and the specified IDR period smaller than the GOP size specified in %s (%u).",
                         rc_info_h265->idrPeriod, rc_info_h265_loc.dot(Field::gopFrameCount).Fields().c_str(),
                         rc_info_h265->gopFrameCount);
    }

    if (rc_info_h265->consecutiveBFrameCount != 0 && rc_info_h265->consecutiveBFrameCount >= rc_info_h265->gopFrameCount) {
        skip |=
            LogError("VUID-VkVideoEncodeH265RateControlInfoKHR-consecutiveBFrameCount-08296", cmdbuf,
                     rc_info_h265_loc.dot(Field::consecutiveBFrameCount),
                     "(%u) is greater than or equal to the GOP size specified in %s (%u).", rc_info_h265->consecutiveBFrameCount,
                     rc_info_h265_loc.dot(Field::gopFrameCount).Fields().c_str(), rc_info_h265->gopFrameCount);
    }

    if (rc_info.layerCount > 1 && rc_info.layerCount != rc_info_h265->subLayerCount) {
        skip |=
            LogError("VUID-VkVideoEncodeRateControlInfoKHR-videoCodecOperation-07025", cmdbuf,
                     rc_info_h265_loc.dot(Field::subLayerCount), "(%u) does not match %s (%u).", rc_info_h265->subLayerCount,
                     loc.pNext(Struct::VkVideoEncodeRateControlInfoKHR, Field::layerCount).Fields().c_str(), rc_info.layerCount);
    }

    return skip;
}

bool CoreChecks::ValidateVideoEncodeRateControlLayerInfo(uint32_t layer_index, const VkVideoEncodeRateControlInfoKHR &rc_info,
                                                         const void *pNext, VkCommandBuffer cmdbuf,
                                                         const vvl::VideoSession &vs_state, const Location &rc_info_loc) const {
    bool skip = false;

    const auto &rc_layer_info = rc_info.pLayers[layer_index];
    const auto &profile_caps = vs_state.profile->GetCapabilities();

    const Location rc_layer_info_loc = rc_info_loc.dot(Field::pLayers, layer_index);

    if (rc_layer_info.averageBitrate < 1 || rc_layer_info.averageBitrate > profile_caps.encode.maxBitrate) {
        const LogObjectList objlist(cmdbuf, vs_state.videoSession());
        skip |=
            LogError("VUID-VkVideoEncodeRateControlInfoKHR-pLayers-08276", objlist, rc_layer_info_loc.dot(Field::averageBitrate),
                     "(%" PRIu64 ") must be between 1 and VkVideoEncodeCapabilitiesKHR::maxBitrate (%" PRIu64
                     ") limit supported by the video profile %s was created with.",
                     rc_layer_info.averageBitrate, profile_caps.encode.maxBitrate, FormatHandle(vs_state).c_str());
    }

    if (rc_layer_info.maxBitrate < 1 || rc_layer_info.maxBitrate > profile_caps.encode.maxBitrate) {
        const LogObjectList objlist(cmdbuf, vs_state.videoSession());
        skip |= LogError("VUID-VkVideoEncodeRateControlInfoKHR-pLayers-08277", objlist, rc_layer_info_loc.dot(Field::maxBitrate),
                         "(%" PRIu64 ") must be between 1 and VkVideoEncodeCapabilitiesKHR::maxBitrate (%" PRIu64
                         ") limit supported by the video profile %s was created with.",
                         rc_layer_info.maxBitrate, profile_caps.encode.maxBitrate, FormatHandle(vs_state).c_str());
    }

    if (rc_info.rateControlMode == VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR &&
        rc_layer_info.averageBitrate != rc_layer_info.maxBitrate) {
        skip |=
            LogError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08356", cmdbuf, rc_info_loc.dot(Field::rateControlMode),
                     "is VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR but maxBitrate (%" PRIu64
                     ") is not equal to averageBitrate (%" PRIu64 ") in %s.",
                     rc_layer_info.maxBitrate, rc_layer_info.averageBitrate, rc_layer_info_loc.Fields().c_str());
    }

    if (rc_info.rateControlMode == VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR &&
        rc_layer_info.averageBitrate > rc_layer_info.maxBitrate) {
        skip |=
            LogError("VUID-VkVideoEncodeRateControlInfoKHR-rateControlMode-08278", cmdbuf, rc_info_loc.dot(Field::rateControlMode),
                     "is VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR but averageBitrate (%" PRIu64
                     ") is greater than maxBitrate (%" PRIu64 ") in %s.",
                     rc_layer_info.averageBitrate, rc_layer_info.maxBitrate, rc_layer_info_loc.Fields().c_str());
    }

    if (rc_layer_info.frameRateNumerator == 0) {
        skip |= LogError("VUID-VkVideoEncodeRateControlLayerInfoKHR-frameRateNumerator-08350", cmdbuf,
                         rc_layer_info_loc.dot(Field::frameRateNumerator), "is zero.");
    }

    if (rc_layer_info.frameRateDenominator == 0) {
        skip |= LogError("VUID-VkVideoEncodeRateControlLayerInfoKHR-frameRateDenominator-08351", cmdbuf,
                         rc_layer_info_loc.dot(Field::frameRateDenominator), "is zero.");
    }

    switch (vs_state.GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
            skip |= ValidateVideoEncodeRateControlLayerInfoH264(layer_index, rc_info, pNext, cmdbuf, vs_state, rc_layer_info_loc);
            break;

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
            skip |= ValidateVideoEncodeRateControlLayerInfoH265(layer_index, rc_info, pNext, cmdbuf, vs_state, rc_layer_info_loc);
            break;

        default:
            assert(false);
            break;
    }

    return skip;
}

template <typename RateControlLayerInfo>
bool CoreChecks::ValidateVideoEncodeRateControlH26xQp(VkCommandBuffer cmdbuf, const vvl::VideoSession &vs_state,
                                                      const RateControlLayerInfo &rc_layer_info, const char *min_qp_range_vuid,
                                                      const char *max_qp_range_vuid, int32_t min_qp, int32_t max_qp,
                                                      const char *min_qp_per_pic_type_vuid, const char *max_qp_per_pic_type_vuid,
                                                      bool qp_per_picture_type, const char *min_max_qp_compare_vuid,
                                                      const Location &loc) const {
    bool skip = false;

    auto qp_range_error = [&](const char *vuid, const Location &field_loc, int32_t value) {
        const LogObjectList objlist(cmdbuf, vs_state.videoSession());
        return LogError(vuid, objlist, field_loc,
                        "(%d) is outside of the range [%d, %d] supported by the video profile %s was created with.", value, min_qp,
                        max_qp, FormatHandle(vs_state).c_str());
    };

    auto qp_per_pic_type_error = [&](const char *vuid, const Location &struct_loc, int32_t qp_i, int32_t qp_p, int32_t qp_b) {
        const LogObjectList objlist(cmdbuf, vs_state.videoSession());
        return LogError(vuid, objlist, struct_loc,
                        "contains non-matching QP values (qpI = %d, qpP = %d, qpB = %d) but different QP values per "
                        "picture type are not supported by the video profile %s was created with.",
                        qp_i, qp_p, qp_b, FormatHandle(vs_state).c_str());
    };

    auto min_max_qp_compare_error = [&](const char *which, int32_t min_value, int32_t max_value) {
        return LogError(min_max_qp_compare_vuid, cmdbuf, loc, "minQp.%s (%d) is greater than maxQp.%s (%d).", which, min_value,
                        which, max_value);
    };

    if (rc_layer_info.useMinQp) {
        if (rc_layer_info.minQp.qpI < min_qp || rc_layer_info.minQp.qpI > max_qp) {
            skip |= qp_range_error(min_qp_range_vuid, loc.dot(Field::minQp).dot(Field::qpI), rc_layer_info.minQp.qpI);
        }

        if (rc_layer_info.minQp.qpP < min_qp || rc_layer_info.minQp.qpP > max_qp) {
            skip |= qp_range_error(min_qp_range_vuid, loc.dot(Field::minQp).dot(Field::qpP), rc_layer_info.minQp.qpP);
        }

        if (rc_layer_info.minQp.qpB < min_qp || rc_layer_info.minQp.qpB > max_qp) {
            skip |= qp_range_error(min_qp_range_vuid, loc.dot(Field::minQp).dot(Field::qpB), rc_layer_info.minQp.qpB);
        }

        if ((rc_layer_info.minQp.qpI != rc_layer_info.minQp.qpP || rc_layer_info.minQp.qpI != rc_layer_info.minQp.qpB) &&
            !qp_per_picture_type) {
            skip |= qp_per_pic_type_error(min_qp_per_pic_type_vuid, loc.dot(Field::minQp), rc_layer_info.minQp.qpI,
                                          rc_layer_info.minQp.qpP, rc_layer_info.minQp.qpB);
        }
    }

    if (rc_layer_info.useMaxQp) {
        if (rc_layer_info.maxQp.qpI < min_qp || rc_layer_info.maxQp.qpI > max_qp) {
            skip |= qp_range_error(max_qp_range_vuid, loc.dot(Field::maxQp).dot(Field::qpI), rc_layer_info.maxQp.qpI);
        }

        if (rc_layer_info.maxQp.qpP < min_qp || rc_layer_info.maxQp.qpP > max_qp) {
            skip |= qp_range_error(max_qp_range_vuid, loc.dot(Field::maxQp).dot(Field::qpP), rc_layer_info.maxQp.qpP);
        }

        if (rc_layer_info.maxQp.qpB < min_qp || rc_layer_info.maxQp.qpB > max_qp) {
            skip |= qp_range_error(max_qp_range_vuid, loc.dot(Field::maxQp).dot(Field::qpB), rc_layer_info.maxQp.qpB);
        }

        if ((rc_layer_info.maxQp.qpI != rc_layer_info.maxQp.qpP || rc_layer_info.maxQp.qpI != rc_layer_info.maxQp.qpB) &&
            !qp_per_picture_type) {
            skip |= qp_per_pic_type_error(max_qp_per_pic_type_vuid, loc.dot(Field::maxQp), rc_layer_info.maxQp.qpI,
                                          rc_layer_info.maxQp.qpP, rc_layer_info.maxQp.qpB);
        }
    }

    if (rc_layer_info.useMinQp && rc_layer_info.useMaxQp) {
        if (rc_layer_info.minQp.qpI > rc_layer_info.maxQp.qpI) {
            skip |= min_max_qp_compare_error("qpI", rc_layer_info.minQp.qpI, rc_layer_info.maxQp.qpI);
        }

        if (rc_layer_info.minQp.qpP > rc_layer_info.maxQp.qpP) {
            skip |= min_max_qp_compare_error("qpP", rc_layer_info.minQp.qpP, rc_layer_info.maxQp.qpP);
        }

        if (rc_layer_info.minQp.qpB > rc_layer_info.maxQp.qpB) {
            skip |= min_max_qp_compare_error("qpB", rc_layer_info.minQp.qpB, rc_layer_info.maxQp.qpB);
        }
    }

    return skip;
}

bool CoreChecks::ValidateVideoEncodeRateControlLayerInfoH264(uint32_t layer_index, const VkVideoEncodeRateControlInfoKHR &rc_info,
                                                             const void *pNext, VkCommandBuffer cmdbuf,
                                                             const vvl::VideoSession &vs_state,
                                                             const Location &rc_layer_info_loc) const {
    bool skip = false;

    const auto rc_layer_info_h264 =
        vku::FindStructInPNextChain<VkVideoEncodeH264RateControlLayerInfoKHR>(rc_info.pLayers[layer_index].pNext);
    if (rc_layer_info_h264 == nullptr) return false;

    const Location rc_layer_info_h264_loc = rc_layer_info_loc.pNext(Struct::VkVideoEncodeH264RateControlLayerInfoKHR);

    const auto &profile_caps = vs_state.profile->GetCapabilities();

    skip |= ValidateVideoEncodeRateControlH26xQp(
        cmdbuf, vs_state, *rc_layer_info_h264, "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08286",
        "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08287", profile_caps.encode_h264.minQp,
        profile_caps.encode_h264.maxQp, "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08288",
        "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMaxQp-08289",
        profile_caps.encode_h264.flags & VK_VIDEO_ENCODE_H264_CAPABILITY_PER_PICTURE_TYPE_MIN_MAX_QP_BIT_KHR,
        "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-useMinQp-08374", rc_layer_info_h264_loc);

    return skip;
}

bool CoreChecks::ValidateVideoEncodeRateControlLayerInfoH265(uint32_t layer_index, const VkVideoEncodeRateControlInfoKHR &rc_info,
                                                             const void *pNext, VkCommandBuffer cmdbuf,
                                                             const vvl::VideoSession &vs_state,
                                                             const Location &rc_layer_info_loc) const {
    bool skip = false;

    const auto rc_layer_info_h265 =
        vku::FindStructInPNextChain<VkVideoEncodeH265RateControlLayerInfoKHR>(rc_info.pLayers[layer_index].pNext);
    if (rc_layer_info_h265 == nullptr) return false;

    const Location rc_layer_info_h265_loc = rc_layer_info_loc.pNext(Struct::VkVideoEncodeH265RateControlLayerInfoKHR);

    const auto &profile_caps = vs_state.profile->GetCapabilities();

    skip |= ValidateVideoEncodeRateControlH26xQp(
        cmdbuf, vs_state, *rc_layer_info_h265, "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08297",
        "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08298", profile_caps.encode_h265.minQp,
        profile_caps.encode_h265.maxQp, "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08299",
        "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMaxQp-08300",
        profile_caps.encode_h265.flags & VK_VIDEO_ENCODE_H265_CAPABILITY_PER_PICTURE_TYPE_MIN_MAX_QP_BIT_KHR,
        "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-useMinQp-08375", rc_layer_info_h265_loc);

    return skip;
}

bool CoreChecks::ValidateVideoPictureResource(const vvl::VideoPictureResource &picture_resource, VkCommandBuffer cmdbuf,
                                              const vvl::VideoSession &vs_state, const Location &loc, const char *coded_offset_vuid,
                                              const char *coded_extent_vuid) const {
    bool skip = false;

    const auto &profile_caps = vs_state.profile->GetCapabilities();

    if (coded_offset_vuid) {
        VkOffset2D offset_granularity{0, 0};
        if (vs_state.GetCodecOp() == VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR &&
            vs_state.GetH264PictureLayout() == VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_INTERLACED_SEPARATE_PLANES_BIT_KHR) {
            offset_granularity = profile_caps.decode_h264.fieldOffsetGranularity;
        }

        if (!IsIntegerMultipleOf(picture_resource.coded_offset, offset_granularity)) {
            const LogObjectList objlist(cmdbuf, vs_state.videoSession());
            skip |= LogError(coded_offset_vuid, objlist, loc.dot(Field::codedExtent),
                             "(%u,%u) is not an integer multiple of the codedOffsetGranularity (%u,%u).",
                             picture_resource.coded_offset.x, picture_resource.coded_offset.y, offset_granularity.x,
                             offset_granularity.y);
        }
    }

    if (coded_extent_vuid &&
        !IsBetweenInclusive(picture_resource.coded_extent, profile_caps.base.minCodedExtent, vs_state.create_info.maxCodedExtent)) {
        const LogObjectList objlist(cmdbuf, vs_state.videoSession());
        skip |= LogError(coded_extent_vuid, objlist, loc.dot(Field::codedExtent),
                         "(%u,%u) is outside of the range (%u,%u)-(%u,%u) supported by %s.", picture_resource.coded_extent.width,
                         picture_resource.coded_extent.height, profile_caps.base.minCodedExtent.width,
                         profile_caps.base.minCodedExtent.height, vs_state.create_info.maxCodedExtent.width,
                         vs_state.create_info.maxCodedExtent.height, FormatHandle(vs_state).c_str());
    }

    if (picture_resource.base_array_layer >= picture_resource.image_view_state->create_info.subresourceRange.layerCount) {
        const LogObjectList objlist(cmdbuf, vs_state.videoSession(), picture_resource.image_view_state->Handle(),
                                    picture_resource.image_state->Handle());
        skip |=
            LogError("VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175", objlist, loc.dot(Field::baseArrayLayer),
                     "(%u) is greater than or equal to the layerCount (%u) "
                     "the %s specified in imageViewBinding was created with.",
                     picture_resource.base_array_layer, picture_resource.image_view_state->create_info.subresourceRange.layerCount,
                     FormatHandle(picture_resource.image_view_state->Handle()).c_str());
    }

    return skip;
}

template bool CoreChecks::ValidateVideoProfileInfo<VkDevice>(const VkVideoProfileInfoKHR *profile, const VkDevice object,
                                                             const Location &loc) const;
template bool CoreChecks::ValidateVideoProfileInfo<VkPhysicalDevice>(const VkVideoProfileInfoKHR *profile,
                                                                     const VkPhysicalDevice object, const Location &loc) const;

template <typename HandleT>
bool CoreChecks::ValidateVideoProfileInfo(const VkVideoProfileInfoKHR *profile, const HandleT object, const Location &loc) const {
    bool skip = false;

    const char *profile_pnext_msg = "chain does not contain a %s structure.";

    if (GetBitSetCount(profile->chromaSubsampling) != 1) {
        skip |= LogError("VUID-VkVideoProfileInfoKHR-chromaSubsampling-07013", object, loc.dot(Field::chromaSubsampling),
                         "must have a single bit set.");
    }

    if (GetBitSetCount(profile->lumaBitDepth) != 1) {
        skip |= LogError("VUID-VkVideoProfileInfoKHR-lumaBitDepth-07014", object, loc.dot(Field::lumaBitDepth),
                         "must have a single bit set.");
    }

    if (profile->chromaSubsampling != VK_VIDEO_CHROMA_SUBSAMPLING_MONOCHROME_BIT_KHR) {
        if (GetBitSetCount(profile->chromaBitDepth) != 1) {
            skip |= LogError("VUID-VkVideoProfileInfoKHR-chromaSubsampling-07015", object, loc.dot(Field::chromaBitDepth),
                             "must have a single bit set.");
        }
    }

    switch (profile->videoCodecOperation) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
            const auto decode_h264 = vku::FindStructInPNextChain<VkVideoDecodeH264ProfileInfoKHR>(profile->pNext);
            if (decode_h264 == nullptr) {
                skip |= LogError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07179", object, loc.dot(Field::pNext),
                                 profile_pnext_msg, "VkVideoDecodeH264ProfileInfoKHR");
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
            const auto decode_h265 = vku::FindStructInPNextChain<VkVideoDecodeH265ProfileInfoKHR>(profile->pNext);
            if (decode_h265 == nullptr) {
                skip |= LogError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07180", object, loc.dot(Field::pNext),
                                 profile_pnext_msg, "VkVideoDecodeH265ProfileInfoKHR");
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
            const auto encode_h264 = vku::FindStructInPNextChain<VkVideoEncodeH264ProfileInfoKHR>(profile->pNext);
            if (encode_h264 == nullptr) {
                skip |= LogError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07181", object, loc.dot(Field::pNext),
                                 profile_pnext_msg, "VkVideoEncodeH264ProfileInfoKHR");
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
            const auto encode_h265 = vku::FindStructInPNextChain<VkVideoEncodeH265ProfileInfoKHR>(profile->pNext);
            if (encode_h265 == nullptr) {
                skip |= LogError("VUID-VkVideoProfileInfoKHR-videoCodecOperation-07182", object, loc.dot(Field::pNext),
                                 profile_pnext_msg, "VkVideoEncodeH265ProfileInfoKHR");
            }
            break;
        }

        default:
            assert(false);
            skip = true;
            break;
    }

    return skip;
}

template bool CoreChecks::ValidateVideoProfileListInfo<VkDevice>(
    const VkVideoProfileListInfoKHR *profile_list, const VkDevice object, const Location &loc, bool expect_decode_profile,
    const char *missing_decode_profile_msg_code, bool expect_encode_profile, const char *missing_encode_profile_msg_code) const;
template bool CoreChecks::ValidateVideoProfileListInfo<VkPhysicalDevice>(
    const VkVideoProfileListInfoKHR *profile_list, const VkPhysicalDevice object, const Location &loc, bool expect_decode_profile,
    const char *missing_decode_profile_msg_code, bool expect_encode_profile, const char *missing_encode_profile_msg_code) const;

template <typename HandleT>
bool CoreChecks::ValidateVideoProfileListInfo(const VkVideoProfileListInfoKHR *profile_list, const HandleT object,
                                              const Location &loc, bool expect_decode_profile,
                                              const char *missing_decode_profile_msg_code, bool expect_encode_profile,
                                              const char *missing_encode_profile_msg_code) const {
    bool skip = false;

    bool has_decode_profile = false;
    bool has_encode_profile = false;

    if (profile_list) {
        for (uint32_t i = 0; i < profile_list->profileCount; ++i) {
            skip |= ValidateVideoProfileInfo(&profile_list->pProfiles[i], object, loc.dot(Field::pProfiles, i));

            switch (profile_list->pProfiles[i].videoCodecOperation) {
                case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
                case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
                    if (has_decode_profile) {
                        skip |= LogError("VUID-VkVideoProfileListInfoKHR-pProfiles-06813", object, loc,
                                         "contains more than one profile with decode codec operation.");
                    }
                    has_decode_profile = true;
                    break;

                case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
                case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
                    has_encode_profile = true;
                    break;

                default:
                    assert(false);
                    skip = true;
                    break;
            }
        }
    }

    if (expect_decode_profile && !has_decode_profile) {
        skip |= LogError(missing_decode_profile_msg_code, object, loc.dot(Field::pProfiles),
                         "contains no video profile specifying a video decode operation.");
    }

    if (expect_encode_profile && !has_encode_profile) {
        skip |= LogError(missing_encode_profile_msg_code, object, loc.dot(Field::pProfiles),
                         "contains no video profile specifying a video encode operation.");
    }

    return skip;
}

bool CoreChecks::ValidateDecodeH264ParametersAddInfo(const vvl::VideoSession &vs_state,
                                                     const VkVideoDecodeH264SessionParametersAddInfoKHR *add_info, VkDevice device,
                                                     const Location &loc,
                                                     const VkVideoDecodeH264SessionParametersCreateInfoKHR *create_info,
                                                     const vvl::VideoSessionParameters *template_state) const {
    bool skip = false;

    vvl::unordered_set<vvl::VideoSessionParameters::ParameterKey> keys;
    auto template_data = template_state ? template_state->Lock() : vvl::VideoSessionParameters::ReadOnlyAccessor();

    if (add_info) {
        // Verify SPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdSPSCount; ++i) {
            auto key = vvl::VideoSessionParameters::GetH264SPSKey(add_info->pStdSPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError("VUID-VkVideoDecodeH264SessionParametersAddInfoKHR-None-04825", device, loc.dot(Field::pStdSPSs),
                                 "keys are not unique.");
                break;
            }
        }
    }

    if (create_info) {
        // Verify SPS capacity
        if (template_data) {
            for (const auto &it : template_data->h264.sps) {
                keys.emplace(it.first);
            }
        }
        if (keys.size() > create_info->maxStdSPSCount) {
            skip |= LogError(device, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07204",
                             "%s(): number of H.264 SPS entries to add (%zu) is larger than "
                             "VkVideoDecodeH264SessionParametersCreateInfoKHR::maxStdSPSCount (%u).",
                             loc.StringFunc(), keys.size(), create_info->maxStdSPSCount);
        }
    }

    keys.clear();

    if (add_info) {
        // Verify PPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdPPSCount; ++i) {
            auto key = vvl::VideoSessionParameters::GetH264PPSKey(add_info->pStdPPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError("VUID-VkVideoDecodeH264SessionParametersAddInfoKHR-None-04826", device, loc.dot(Field::pStdPPSs),
                                 "keys are not unique.");
                break;
            }
        }
    }

    if (create_info) {
        // Verify PPS capacity
        if (template_data) {
            for (const auto &it : template_data->h264.pps) {
                keys.emplace(it.first);
            }
        }
        if (keys.size() > create_info->maxStdPPSCount) {
            skip |= LogError(device, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07205",
                             "%s(): number of H.264 PPS entries to add (%zu) is larger than "
                             "VkVideoDecodeH264SessionParametersCreateInfoKHR::maxStdPPSCount (%u).",
                             loc.StringFunc(), keys.size(), create_info->maxStdPPSCount);
        }
    }

    return skip;
}

bool CoreChecks::ValidateDecodeH265ParametersAddInfo(const vvl::VideoSession &vs_state,
                                                     const VkVideoDecodeH265SessionParametersAddInfoKHR *add_info, VkDevice device,
                                                     const Location &loc,
                                                     const VkVideoDecodeH265SessionParametersCreateInfoKHR *create_info,
                                                     const vvl::VideoSessionParameters *template_state) const {
    bool skip = false;

    vvl::unordered_set<vvl::VideoSessionParameters::ParameterKey> keys;
    auto template_data = template_state ? template_state->Lock() : vvl::VideoSessionParameters::ReadOnlyAccessor();

    if (add_info) {
        // Verify VPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdVPSCount; ++i) {
            auto key = vvl::VideoSessionParameters::GetH265VPSKey(add_info->pStdVPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04833", device, loc.dot(Field::pStdVPSs),
                                 "keys are not unique.");
                break;
            }
        }
    }

    if (create_info) {
        // Verify VPS capacity
        if (template_data) {
            for (const auto &it : template_data->h265.vps) {
                keys.emplace(it.first);
            }
        }
        if (keys.size() > create_info->maxStdVPSCount) {
            skip |= LogError(device, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07207",
                             "%s(): number of H.265 VPS entries to add (%zu) is larger than "
                             "VkVideoDecodeH265SessionParametersCreateInfoKHR::maxStdVPSCount (%u).",
                             loc.StringFunc(), keys.size(), create_info->maxStdVPSCount);
        }
    }

    keys.clear();

    if (add_info) {
        // Verify SPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdSPSCount; ++i) {
            auto key = vvl::VideoSessionParameters::GetH265SPSKey(add_info->pStdSPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04834", device, loc.dot(Field::pStdSPSs),
                                 "keys are not unique.");
                break;
            }
        }
    }

    if (create_info) {
        // Verify SPS capacity
        if (template_data) {
            for (const auto &it : template_data->h265.sps) {
                keys.emplace(it.first);
            }
        }
        if (keys.size() > create_info->maxStdSPSCount) {
            skip |= LogError(device, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07208",
                             "%s(): number of H.265 SPS entries to add (%zu) is larger than "
                             "VkVideoDecodeH265SessionParametersCreateInfoKHR::maxStdSPSCount (%u).",
                             loc.StringFunc(), keys.size(), create_info->maxStdSPSCount);
        }
    }

    keys.clear();

    if (add_info) {
        // Verify PPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdPPSCount; ++i) {
            auto key = vvl::VideoSessionParameters::GetH265PPSKey(add_info->pStdPPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError("VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04835", device, loc.dot(Field::pStdPPSs),
                                 "keys are not unique.");
                break;
            }
        }
    }

    if (create_info) {
        // Verify PPS capacity
        if (template_data) {
            for (const auto &it : template_data->h265.pps) {
                keys.emplace(it.first);
            }
        }
        if (keys.size() > create_info->maxStdPPSCount) {
            skip |= LogError(device, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07209",
                             "%s(): number of H.265 PPS entries to add (%zu) is larger than "
                             "VkVideoDecodeH265SessionParametersCreateInfoKHR::maxStdPPSCount (%u).",
                             loc.StringFunc(), keys.size(), create_info->maxStdPPSCount);
        }
    }

    return skip;
}

bool CoreChecks::ValidateEncodeH264ParametersAddInfo(const vvl::VideoSession &vs_state,
                                                     const VkVideoEncodeH264SessionParametersAddInfoKHR *add_info, VkDevice device,
                                                     const Location &loc,
                                                     const VkVideoEncodeH264SessionParametersCreateInfoKHR *create_info,
                                                     const vvl::VideoSessionParameters *template_state) const {
    bool skip = false;

    vvl::unordered_set<vvl::VideoSessionParameters::ParameterKey> keys;
    auto template_data = template_state ? template_state->Lock() : vvl::VideoSessionParameters::ReadOnlyAccessor();

    if (add_info) {
        // Verify SPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdSPSCount; ++i) {
            auto key = vvl::VideoSessionParameters::GetH264SPSKey(add_info->pStdSPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError("VUID-VkVideoEncodeH264SessionParametersAddInfoKHR-None-04837", device, loc.dot(Field::pStdSPSs),
                                 "keys are not unique.");
                break;
            }
        }
    }

    if (create_info) {
        // Verify SPS capacity
        if (template_data) {
            for (const auto &it : template_data->h264.sps) {
                keys.emplace(it.first);
            }
        }
        if (keys.size() > create_info->maxStdSPSCount) {
            skip |= LogError(device, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04839",
                             "%s(): number of H.264 SPS entries to add (%zu) is larger than "
                             "VkVideoEncodeH264SessionParametersCreateInfoKHR::maxStdSPSCount (%u).",
                             loc.StringFunc(), keys.size(), create_info->maxStdSPSCount);
        }
    }

    keys.clear();

    if (add_info) {
        // Verify PPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdPPSCount; ++i) {
            auto key = vvl::VideoSessionParameters::GetH264PPSKey(add_info->pStdPPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError("VUID-VkVideoEncodeH264SessionParametersAddInfoKHR-None-04838", device, loc.dot(Field::pStdPPSs),
                                 "keys are not unique.");
                break;
            }
        }
    }

    if (create_info) {
        // Verify PPS capacity
        if (template_data) {
            for (const auto &it : template_data->h264.pps) {
                keys.emplace(it.first);
            }
        }
        if (keys.size() > create_info->maxStdPPSCount) {
            skip |= LogError(device, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04840",
                             "%s(): number of H.264 PPS entries to add (%zu) is larger than "
                             "VkVideoEncodeH264SessionParametersCreateInfoKHR::maxStdPPSCount (%u).",
                             loc.StringFunc(), keys.size(), create_info->maxStdPPSCount);
        }
    }

    return skip;
}

bool CoreChecks::ValidateEncodeH265ParametersAddInfo(const vvl::VideoSession &vs_state,
                                                     const VkVideoEncodeH265SessionParametersAddInfoKHR *add_info, VkDevice device,
                                                     const Location &loc,
                                                     const VkVideoEncodeH265SessionParametersCreateInfoKHR *create_info,
                                                     const vvl::VideoSessionParameters *template_state) const {
    bool skip = false;

    vvl::unordered_set<vvl::VideoSessionParameters::ParameterKey> keys;
    auto template_data = template_state ? template_state->Lock() : vvl::VideoSessionParameters::ReadOnlyAccessor();

    if (add_info) {
        // Verify VPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdVPSCount; ++i) {
            auto key = vvl::VideoSessionParameters::GetH265VPSKey(add_info->pStdVPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06438", device, loc.dot(Field::pStdVPSs),
                                 "keys are not unique.");
                break;
            }
        }
    }

    if (create_info) {
        // Verify VPS capacity
        if (template_data) {
            for (const auto &it : template_data->h265.vps) {
                keys.emplace(it.first);
            }
        }
        if (keys.size() > create_info->maxStdVPSCount) {
            skip |= LogError(device, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04841",
                             "%s(): number of H.265 VPS entries to add (%zu) is larger than "
                             "VkVideoEncodeH265SessionParametersCreateInfoKHR::maxStdVPSCount (%u).",
                             loc.StringFunc(), keys.size(), create_info->maxStdVPSCount);
        }
    }

    keys.clear();

    if (add_info) {
        // Verify SPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdSPSCount; ++i) {
            auto key = vvl::VideoSessionParameters::GetH265SPSKey(add_info->pStdSPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06439", device, loc.dot(Field::pStdSPSs),
                                 "keys are not unique.");
                break;
            }
        }
    }

    if (create_info) {
        // Verify SPS capacity
        if (template_data) {
            for (const auto &it : template_data->h265.sps) {
                keys.emplace(it.first);
            }
        }
        if (keys.size() > create_info->maxStdSPSCount) {
            skip |= LogError(device, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04842",
                             "%s(): number of H.265 SPS entries to add (%zu) is larger than "
                             "VkVideoEncodeH265SessionParametersCreateInfoKHR::maxStdSPSCount (%u).",
                             loc.StringFunc(), keys.size(), create_info->maxStdSPSCount);
        }
    }

    keys.clear();

    if (add_info) {
        // Verify PPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdPPSCount; ++i) {
            auto key = vvl::VideoSessionParameters::GetH265PPSKey(add_info->pStdPPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError("VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-None-06440", device, loc.dot(Field::pStdPPSs),
                                 "keys are not unique.");
                break;
            }
        }
    }

    if (create_info) {
        // Verify PPS capacity
        if (template_data) {
            for (const auto &it : template_data->h265.pps) {
                keys.emplace(it.first);
            }
        }
        if (keys.size() > create_info->maxStdPPSCount) {
            skip |= LogError(device, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-04843",
                             "%s(): number of H.265 PPS entries to add (%zu) is larger than "
                             "VkVideoEncodeH265SessionParametersCreateInfoKHR::maxStdPPSCount (%u).",
                             loc.StringFunc(), keys.size(), create_info->maxStdPPSCount);
        }
    }

    if (add_info) {
        const auto &profile_caps = vs_state.profile->GetCapabilities();

        // Verify PPS contents
        for (uint32_t i = 0; i < add_info->stdPPSCount; ++i) {
            if (add_info->pStdPPSs[i].num_tile_columns_minus1 >= profile_caps.encode_h265.maxTiles.width) {
                const char *vuid = nullptr;
                if (create_info) {
                    assert(loc.function == Func::vkCreateVideoSessionParametersKHR);
                    vuid = "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08319";
                } else {
                    assert(loc.function == Func::vkUpdateVideoSessionParametersKHR);
                    vuid = "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-08321";
                }
                skip |= LogError(device, vuid,
                                 "%s(): %s.num_tile_columns_minus1 (%u) exceeds the maxTiles.width (%u) "
                                 "supported by the H.265 encode profile %s was created with.",
                                 loc.StringFunc(), loc.dot(Field::pStdPPSs, i).Fields().c_str(),
                                 add_info->pStdPPSs[i].num_tile_columns_minus1, profile_caps.encode_h265.maxTiles.width,
                                 FormatHandle(vs_state).c_str());
            }
            if (add_info->pStdPPSs[i].num_tile_rows_minus1 >= profile_caps.encode_h265.maxTiles.height) {
                const char *vuid = nullptr;
                if (create_info) {
                    assert(loc.function == Func::vkCreateVideoSessionParametersKHR);
                    vuid = "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-08320";
                } else {
                    assert(loc.function == Func::vkUpdateVideoSessionParametersKHR);
                    vuid = "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-08322";
                }
                skip |= LogError(device, vuid,
                                 "%s(): %s.num_tile_rows_minus1 (%u) exceeds the maxTiles.height (%u) "
                                 "supported by the H.265 encode profile %s was created with.",
                                 loc.StringFunc(), loc.dot(Field::pStdPPSs, i).Fields().c_str(),
                                 add_info->pStdPPSs[i].num_tile_rows_minus1, profile_caps.encode_h265.maxTiles.height,
                                 FormatHandle(vs_state).c_str());
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateVideoDecodeInfoH264(const vvl::CommandBuffer &cb_state, const VkVideoDecodeInfoKHR &decode_info,
                                             const Location &loc) const {
    bool skip = false;

    const char *pnext_msg = "chain does not contain a %s structure.";

    const auto &vs_state = *cb_state.bound_video_session;
    const auto &vsp_state = *cb_state.bound_video_session_parameters;
    const auto session_params = vsp_state.Lock();

    bool interlaced_frame_support =
        (vs_state.profile->GetH264PictureLayout() != VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_PROGRESSIVE_KHR);

    auto picture_info = vku::FindStructInPNextChain<VkVideoDecodeH264PictureInfoKHR>(decode_info.pNext);
    if (picture_info) {
        auto std_picture_info = picture_info->pStdPictureInfo;

        if (!interlaced_frame_support && std_picture_info->flags.field_pic_flag) {
            const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
            skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-None-07258",
                             "vkCmdDecodeVideoKHR(): decode output picture is a field but the bound video session "
                             "%s was not created with interlaced frame support.",
                             FormatHandle(vs_state).c_str());
        }

        for (uint32_t i = 0; i < picture_info->sliceCount; ++i) {
            if (picture_info->pSliceOffsets[i] >= decode_info.srcBufferRange) {
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-pSliceOffsets-07153", cb_state.commandBuffer(),
                                 loc.pNext(Struct::VkVideoDecodeH264PictureInfoKHR, Field::pSliceOffsets, i),
                                 "(%u) is greater than or equal to pDecodeInfo->srcBufferRange (%" PRIu64 ").",
                                 picture_info->pSliceOffsets[i], decode_info.srcBufferRange);
            }
        }

        if (session_params.GetH264SPS(std_picture_info->seq_parameter_set_id) == nullptr) {
            const LogObjectList objlist(cb_state.commandBuffer(), vsp_state.videoSessionParameters());
            skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-StdVideoH264SequenceParameterSet-07154",
                             "vkCmdDecodeVideoKHR(): no H.264 SPS with seq_parameter_set_id = %u "
                             "exists in the bound video session parameters object %s.",
                             std_picture_info->seq_parameter_set_id, FormatHandle(vsp_state).c_str());
        }

        if (session_params.GetH264PPS(std_picture_info->seq_parameter_set_id, std_picture_info->pic_parameter_set_id) == nullptr) {
            const LogObjectList objlist(cb_state.commandBuffer(), vsp_state.videoSessionParameters());
            skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-StdVideoH264PictureParameterSet-07155",
                             "vkCmdDecodeVideoKHR(): no H.264 PPS with seq_parameter_set_id = %u "
                             "and pic_parameter_set_id = %u exists in the bound video session parameters object %s.",
                             std_picture_info->seq_parameter_set_id, std_picture_info->pic_parameter_set_id,
                             FormatHandle(vsp_state).c_str());
        }
    } else {
        skip |= LogError("VUID-vkCmdDecodeVideoKHR-pNext-07152", cb_state.commandBuffer(), loc.dot(Field::pNext), pnext_msg,
                         "VkVideoDecodeH264PictureInfoKHR");
    }

    if (decode_info.pSetupReferenceSlot) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoDecodeH264DpbSlotInfoKHR>(decode_info.pSetupReferenceSlot->pNext);
        if (dpb_slot_info) {
            vvl::VideoPictureID picture_id(*vs_state.profile, *decode_info.pSetupReferenceSlot);
            if (!interlaced_frame_support && !picture_id.IsFrame()) {
                const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
                skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07259",
                                 "vkCmdDecodeVideoKHR(): reconstructed picture is a field but the bound "
                                 "video session %s was not created with interlaced frame support.",
                                 FormatHandle(vs_state).c_str());
            }

            if (picture_info) {
                bool dst_is_field = (picture_info->pStdPictureInfo->flags.field_pic_flag != 0);
                bool dst_is_bottom_field = (picture_info->pStdPictureInfo->flags.bottom_field_flag != 0);

                if (!dst_is_field && !picture_id.IsFrame()) {
                    skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07261",
                                     "vkCmdDecodeVideoKHR(): decode output picture is a frame but the "
                                     "reconstructed picture is not a frame.");
                }

                if (dst_is_field && !dst_is_bottom_field && !picture_id.IsTopField()) {
                    skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07262",
                                     "vkCmdDecodeVideoKHR(): decode output picture is a top field but the "
                                     "reconstructed picture is not a top field.");
                }

                if (dst_is_field && dst_is_bottom_field && !picture_id.IsBottomField()) {
                    skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07263",
                                     "vkCmdDecodeVideoKHR(): decode output picture is a bottom field but the "
                                     "reconstructed picture is not a bottom field.");
                }
            }
        } else {
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07156", cb_state.commandBuffer(),
                             loc.dot(Field::pSetupReferenceSlot).dot(Field::pNext), pnext_msg, "VkVideoDecodeH264DpbSlotInfoKHR");
        }
    }

    for (uint32_t i = 0; i < decode_info.referenceSlotCount; ++i) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoDecodeH264DpbSlotInfoKHR>(decode_info.pReferenceSlots[i].pNext);
        if (dpb_slot_info) {
            vvl::VideoPictureID picture_id(*vs_state.profile, decode_info.pReferenceSlots[i]);
            if (!interlaced_frame_support && !picture_id.IsFrame()) {
                const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
                skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07260",
                                 "vkCmdDecodeVideoKHR(): reference picture specified in "
                                 "pDecodeInfo->pReferenceSlots[%u] is a field but the bound "
                                 "video session %s was not created with interlaced frame support.",
                                 i, FormatHandle(vs_state).c_str());
            }
        } else {
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-pNext-07157", cb_state.commandBuffer(),
                             loc.dot(Field::pReferenceSlots, i).dot(Field::pNext), pnext_msg, "VkVideoDecodeH264DpbSlotInfoKHR");
        }
    }

    return skip;
}

bool CoreChecks::ValidateVideoDecodeInfoH265(const vvl::CommandBuffer &cb_state, const VkVideoDecodeInfoKHR &decode_info,
                                             const Location &loc) const {
    bool skip = false;

    const char *pnext_msg = "chain does not contain a %s structure.";

    const auto &vsp_state = *cb_state.bound_video_session_parameters;
    const auto session_params = vsp_state.Lock();

    auto picture_info = vku::FindStructInPNextChain<VkVideoDecodeH265PictureInfoKHR>(decode_info.pNext);
    if (picture_info) {
        auto std_picture_info = picture_info->pStdPictureInfo;

        for (uint32_t i = 0; i < picture_info->sliceSegmentCount; ++i) {
            if (picture_info->pSliceSegmentOffsets[i] >= decode_info.srcBufferRange) {
                skip |= LogError("VUID-vkCmdDecodeVideoKHR-pSliceSegmentOffsets-07159", cb_state.commandBuffer(),
                                 loc.pNext(Struct::VkVideoDecodeH265PictureInfoKHR, Field::pSliceSegmentOffsets, i),
                                 "(%u) is greater than or equal to pDecodeInfo->srcBufferRange (%" PRIu64 ").",
                                 picture_info->pSliceSegmentOffsets[i], decode_info.srcBufferRange);
            }
        }

        if (session_params.GetH265VPS(std_picture_info->sps_video_parameter_set_id) == nullptr) {
            const LogObjectList objlist(cb_state.commandBuffer(), vsp_state.videoSessionParameters());
            skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-StdVideoH265VideoParameterSet-07160",
                             "vkCmdDecodeVideoKHR(): no H.265 VPS with sps_video_parameter_set_id = %u "
                             "exists in the bound video session parameters object %s.",
                             std_picture_info->sps_video_parameter_set_id, FormatHandle(vsp_state).c_str());
        }

        if (session_params.GetH265SPS(std_picture_info->sps_video_parameter_set_id, std_picture_info->pps_seq_parameter_set_id) ==
            nullptr) {
            const LogObjectList objlist(cb_state.commandBuffer(), vsp_state.videoSessionParameters());
            skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-StdVideoH265SequenceParameterSet-07161",
                             "vkCmdDecodeVideoKHR(): no H.265 SPS with sps_video_parameter_set_id = %u "
                             "and pps_seq_parameter_set_id = %u exists in the bound video session "
                             "parameters object %s.",
                             std_picture_info->sps_video_parameter_set_id, std_picture_info->pps_seq_parameter_set_id,
                             FormatHandle(vsp_state).c_str());
        }

        if (session_params.GetH265PPS(std_picture_info->sps_video_parameter_set_id, std_picture_info->pps_seq_parameter_set_id,
                                      std_picture_info->pps_pic_parameter_set_id) == nullptr) {
            const LogObjectList objlist(cb_state.commandBuffer(), vsp_state.videoSessionParameters());
            skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-StdVideoH265PictureParameterSet-07162",
                             "vkCmdDecodeVideoKHR(): no H.265 SPS with sps_video_parameter_set_id = %u, "
                             "pps_seq_parameter_set_id = %u, and pps_pic_parameter_set_id = %u exists in "
                             "the bound video session parameters object %s.",
                             std_picture_info->sps_video_parameter_set_id, std_picture_info->pps_seq_parameter_set_id,
                             std_picture_info->pps_pic_parameter_set_id, FormatHandle(vsp_state).c_str());
        }
    } else {
        skip |= LogError("VUID-vkCmdDecodeVideoKHR-pNext-07158", cb_state.commandBuffer(), loc.dot(Field::pNext), pnext_msg,
                         "VkVideoDecodeH265PictureInfoKHR");
    }

    if (decode_info.pSetupReferenceSlot) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoDecodeH265DpbSlotInfoKHR>(decode_info.pSetupReferenceSlot->pNext);
        if (!dpb_slot_info) {
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07163", cb_state.commandBuffer(),
                             loc.dot(Field::pSetupReferenceSlot).dot(Field::pNext), pnext_msg, "VkVideoDecodeH265DpbSlotInfoKHR");
        }
    }

    for (uint32_t i = 0; i < decode_info.referenceSlotCount; ++i) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoDecodeH265DpbSlotInfoKHR>(decode_info.pReferenceSlots[i].pNext);
        if (!dpb_slot_info) {
            skip |= LogError("VUID-vkCmdDecodeVideoKHR-pNext-07164", cb_state.commandBuffer(),
                             loc.dot(Field::pReferenceSlots, i).dot(Field::pNext), pnext_msg, "VkVideoDecodeH265DpbSlotInfoKHR");
        }
    }

    return skip;
}

bool CoreChecks::ValidateVideoEncodeH264PicType(const vvl::VideoSession &vs_state, StdVideoH264PictureType pic_type,
                                                const char *where) const {
    bool skip = false;

    const auto &profile_caps = vs_state.profile->GetCapabilities();

    if (profile_caps.encode_h264.maxPPictureL0ReferenceCount == 0 && pic_type == STD_VIDEO_H264_PICTURE_TYPE_P) {
        skip |= LogError(vs_state.videoSession(), "VUID-vkCmdEncodeVideoKHR-maxPPictureL0ReferenceCount-08340",
                         "vkCmdEncodeVideoKHR(): %s is STD_VIDEO_H264_PICTURE_TYPE_P but P pictures "
                         "are not supported by the H.264 encode profile %s was created with.",
                         where, FormatHandle(vs_state).c_str());
    }

    if (profile_caps.encode_h264.maxBPictureL0ReferenceCount == 0 && profile_caps.encode_h264.maxL1ReferenceCount == 0 &&
        pic_type == STD_VIDEO_H264_PICTURE_TYPE_B) {
        skip |= LogError(vs_state.videoSession(), "VUID-vkCmdEncodeVideoKHR-maxBPictureL0ReferenceCount-08341",
                         "vkCmdEncodeVideoKHR(): %s is STD_VIDEO_H264_PICTURE_TYPE_B but B pictures "
                         "are not supported by the H.264 encode profile %s was created with.",
                         where, FormatHandle(vs_state).c_str());
    }

    return skip;
}

bool CoreChecks::ValidateVideoEncodeInfoH264(const vvl::CommandBuffer &cb_state, const VkVideoEncodeInfoKHR &encode_info,
                                             const Location &loc) const {
    bool skip = false;

    const char *pnext_msg = "chain does not contain a %s structure.";

    const auto &vs_state = *cb_state.bound_video_session;
    const auto &profile_caps = vs_state.profile->GetCapabilities();

    const auto &rc_state = cb_state.video_encode_rate_control_state;

    const auto &vsp_state = *cb_state.bound_video_session_parameters;
    const auto session_params = vsp_state.Lock();

    if (encode_info.pSetupReferenceSlot) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoEncodeH264DpbSlotInfoKHR>(encode_info.pSetupReferenceSlot->pNext);
        if (!dpb_slot_info) {
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08228", cb_state.commandBuffer(),
                             loc.dot(Field::pSetupReferenceSlot).dot(Field::pNext), pnext_msg, "VkVideoEncodeH264DpbSlotInfoKHR");
        }
    }

    vvl::unordered_map<int32_t, const VkVideoEncodeH264DpbSlotInfoKHR *> reference_slots{};
    for (uint32_t i = 0; i < encode_info.referenceSlotCount; ++i) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoEncodeH264DpbSlotInfoKHR>(encode_info.pReferenceSlots[i].pNext);
        if (!dpb_slot_info) {
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pNext-08229", cb_state.commandBuffer(),
                             loc.dot(Field::pReferenceSlots, i).dot(Field::pNext), pnext_msg, "VkVideoEncodeH264DpbSlotInfoKHR");
        }

        reference_slots.insert({encode_info.pReferenceSlots[i].slotIndex, dpb_slot_info});
    }

    auto picture_info = vku::FindStructInPNextChain<VkVideoEncodeH264PictureInfoKHR>(encode_info.pNext);
    if (picture_info) {
        auto std_picture_info = picture_info->pStdPictureInfo;
        auto std_sps = session_params.GetH264SPS(std_picture_info->seq_parameter_set_id);
        auto std_pps = session_params.GetH264PPS(std_picture_info->seq_parameter_set_id, std_picture_info->pic_parameter_set_id);

        const Location slice_count_loc = loc.pNext(Struct::VkVideoEncodeH264PictureInfoKHR, Field::naluSliceEntryCount);
        const Location slice_list_loc = loc.pNext(Struct::VkVideoEncodeH264PictureInfoKHR, Field::pNaluSliceEntries);

        if (std_sps == nullptr) {
            const LogObjectList objlist(cb_state.commandBuffer(), vsp_state.videoSessionParameters());
            skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-StdVideoH264SequenceParameterSet-08226",
                             "vkCmdEncodeVideoKHR(): no H.264 SPS with seq_parameter_set_id = %u "
                             "exists in the bound video session parameters object %s.",
                             std_picture_info->seq_parameter_set_id, FormatHandle(vsp_state).c_str());
        }

        if (std_pps == nullptr) {
            const LogObjectList objlist(cb_state.commandBuffer(), vsp_state.videoSessionParameters());
            skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-StdVideoH264PictureParameterSet-08227",
                             "vkCmdEncodeVideoKHR(): no H.264 PPS with seq_parameter_set_id = %u "
                             "and pic_parameter_set_id = %u exists in the bound video session parameters object %s.",
                             std_picture_info->seq_parameter_set_id, std_picture_info->pic_parameter_set_id,
                             FormatHandle(vsp_state).c_str());
        }

        if (picture_info->naluSliceEntryCount > profile_caps.encode_h264.maxSliceCount) {
            const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
            skip |=
                LogError("VUID-VkVideoEncodeH264PictureInfoKHR-naluSliceEntryCount-08301", objlist, slice_count_loc,
                         "(%u) exceeds the VkVideoEncodeH264CapabilitiesKHR::maxSliceCount (%u) limit "
                         "supported by the H.264 encode profile %s was created with.",
                         picture_info->naluSliceEntryCount, profile_caps.encode_h264.maxSliceCount, FormatHandle(vs_state).c_str());
        }

        VkExtent2D max_coding_block_size = vs_state.profile->GetMaxCodingBlockSize();
        VkExtent2D min_coding_block_extent = {
            encode_info.srcPictureResource.codedExtent.width / max_coding_block_size.width,
            encode_info.srcPictureResource.codedExtent.height / max_coding_block_size.height,
        };
        if (profile_caps.encode_h264.flags & VK_VIDEO_ENCODE_H264_CAPABILITY_ROW_UNALIGNED_SLICE_BIT_KHR) {
            if (picture_info->naluSliceEntryCount > min_coding_block_extent.width * min_coding_block_extent.height) {
                skip |=
                    LogError("VUID-vkCmdEncodeVideoKHR-naluSliceEntryCount-08302", cb_state.commandBuffer(), slice_count_loc,
                             "(%u) is greater than the number of MBs (minCodingBlockExtent = {%u, %u}) "
                             "that can be coded for the encode input picture specified in "
                             "pEncodeInfo->srcPictureResource (codedExtent = {%u, %u}).",
                             picture_info->naluSliceEntryCount, min_coding_block_extent.width, min_coding_block_extent.height,
                             encode_info.srcPictureResource.codedExtent.width, encode_info.srcPictureResource.codedExtent.height);
            }
        } else {
            if (picture_info->naluSliceEntryCount > min_coding_block_extent.height) {
                skip |=
                    LogError("VUID-vkCmdEncodeVideoKHR-naluSliceEntryCount-08312", cb_state.commandBuffer(), slice_count_loc,
                             "(%u) is greater than the number of MB rows (minCodingBlockExtent.height = %u) "
                             "that can be coded for the encode input picture specified in "
                             "pEncodeInfo->srcPictureResource (codedExtent = {%u, %u}).",
                             picture_info->naluSliceEntryCount, min_coding_block_extent.height,
                             encode_info.srcPictureResource.codedExtent.width, encode_info.srcPictureResource.codedExtent.height);
            }
        }

        bool different_slice_types = false;
        bool different_constant_qp_per_slice = false;
        for (uint32_t slice_idx = 0; slice_idx < picture_info->naluSliceEntryCount; ++slice_idx) {
            const auto &slice_info = picture_info->pNaluSliceEntries[slice_idx];
            const auto *std_slice_header = slice_info.pStdSliceHeader;
            const Location slice_info_loc = loc.pNext(Struct::VkVideoEncodeH264PictureInfoKHR, Field::pNaluSliceEntries, slice_idx);

            if (std_slice_header->slice_type != picture_info->pNaluSliceEntries[0].pStdSliceHeader->slice_type) {
                different_slice_types = true;
            }

            if (rc_state.base.rateControlMode == VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR) {
                if (slice_info.constantQp < profile_caps.encode_h264.minQp ||
                    slice_info.constantQp > profile_caps.encode_h264.maxQp) {
                    const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-constantQp-08270", objlist, slice_info_loc.dot(Field::constantQp),
                                     "(%d) is outside of the range [%d, %d] supported by the video "
                                     "profile %s was created with.",
                                     slice_info.constantQp, profile_caps.encode_h264.minQp, profile_caps.encode_h264.maxQp,
                                     FormatHandle(vs_state).c_str());
                }

                if (slice_info.constantQp != picture_info->pNaluSliceEntries[0].constantQp) {
                    different_constant_qp_per_slice = true;
                }
            } else {
                if (slice_info.constantQp != 0) {
                    const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
                    skip |= LogError("VUID-vkCmdEncodeVideoKHR-constantQp-08269", objlist, slice_info_loc.dot(Field::constantQp),
                                     "(%d) is not zero but the currently set video encode rate control mode for %s "
                                     "was specified to be %s when beginning the video coding scope.",
                                     slice_info.constantQp, FormatHandle(vs_state).c_str(),
                                     string_VkVideoEncodeRateControlModeFlagBitsKHR(rc_state.base.rateControlMode));
                }
            }

            if (std_pps != nullptr &&
                (profile_caps.encode_h264.flags & VK_VIDEO_ENCODE_H264_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR) == 0) {
                const char *weighted_pred_error_msg = nullptr;

                if (std_slice_header->slice_type == STD_VIDEO_H264_SLICE_TYPE_P && std_pps->flags.weighted_pred_flag) {
                    weighted_pred_error_msg =
                        "weighted_pred_flag is set in the active H.264 PPS, slice_type is STD_VIDEO_H264_SLICE_TYPE_P";
                } else if (std_slice_header->slice_type == STD_VIDEO_H264_SLICE_TYPE_B &&
                           std_pps->weighted_bipred_idc == STD_VIDEO_H264_WEIGHTED_BIPRED_IDC_EXPLICIT) {
                    weighted_pred_error_msg =
                        "weighted_bipred_idc is set to STD_VIDEO_H264_WEIGHTED_BIPRED_IDC_EXPLICIT "
                        "in the active H.264 PPS, slice_type is STD_VIDEO_H264_SLICE_TYPE_B";
                }

                if (std_slice_header->pWeightTable == nullptr && weighted_pred_error_msg != nullptr) {
                    const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
                    skip |= LogError(objlist, "VUID-VkVideoEncodeH264PictureInfoKHR-flags-08314",
                                     "vkCmdEncodeVideoKHR(): %s, and pWeightTable is NULL in %s but "
                                     "VK_VIDEO_ENCODE_H264_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR "
                                     "is not supported by the H.264 encode profile %s was created with.",
                                     weighted_pred_error_msg, slice_info_loc.dot(Field::pStdSliceHeader).Fields().c_str(),
                                     FormatHandle(vs_state).c_str());
                }
            }
        }

        if ((profile_caps.encode_h264.flags & VK_VIDEO_ENCODE_H264_CAPABILITY_DIFFERENT_SLICE_TYPE_BIT_KHR) == 0 &&
            different_slice_types) {
            const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
            skip |= LogError(objlist, "VUID-VkVideoEncodeH264PictureInfoKHR-flags-08315",
                             "VkCmdEncodeVideoKHR(): pStdSliceHeader->slice_type does not match across the elements "
                             "of %s but different slice types in a picture are not supported by the H.264 encode "
                             "profile %s was created with.",
                             slice_list_loc.Fields().c_str(), FormatHandle(vs_state).c_str());
        }

        if ((profile_caps.encode_h264.flags & VK_VIDEO_ENCODE_H264_CAPABILITY_PER_SLICE_CONSTANT_QP_BIT_KHR) == 0 &&
            different_constant_qp_per_slice) {
            const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
            skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-constantQp-08271",
                             "VkCmdEncodeVideoKHR(): constantQp does not match across the elements of %s"
                             "but per-slice constant QP values are not supported by the H.264 encode "
                             "profile %s was created with.",
                             slice_list_loc.Fields().c_str(), FormatHandle(vs_state).c_str());
        }

        skip |= ValidateVideoEncodeH264PicType(vs_state, std_picture_info->primary_pic_type,
                                               "VkVideoEncodeH264PictureInfoKHR::pStdPictureInfo->primary_pic_type");

        if (std_picture_info->pRefLists != nullptr) {
            vvl::unordered_set<uint8_t> ref_list_entries{};

            for (uint8_t i = 0; i < STD_VIDEO_H264_MAX_NUM_LIST_REF; ++i) {
                uint8_t ref_list_entry = std_picture_info->pRefLists->RefPicList0[i];
                if (ref_list_entry == STD_VIDEO_H264_NO_REFERENCE_PICTURE) {
                    continue;
                }

                const auto &ref_slot = reference_slots.find((int32_t)ref_list_entry);
                if (ref_slot != reference_slots.end()) {
                    if (ref_slot->second != nullptr) {
                        auto std_reference_info = ref_slot->second->pStdReferenceInfo;

                        skip |= ValidateVideoEncodeH264PicType(vs_state, std_reference_info->primary_pic_type,
                                                               "primary_pic_type for L0 reference");

                        if (std_reference_info->primary_pic_type == STD_VIDEO_H264_PICTURE_TYPE_B &&
                            (profile_caps.encode_h264.flags & VK_VIDEO_ENCODE_H264_CAPABILITY_B_FRAME_IN_L0_LIST_BIT_KHR) == 0) {
                            const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
                            skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-flags-08342",
                                             "vkCmdEncodeVideoKHR(): primary_pic_type for L0 reference is "
                                             "STD_VIDEO_H264_PICTURE_TYPE_B but B pictures are not supported in the "
                                             "L0 reference list by the H.264 encode profile %s was created with.",
                                             FormatHandle(vs_state).c_str());
                        }
                    }
                } else {
                    skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdEncodeVideoKHR-pNext-08339",
                                     "vkCmdEncodeVideoKHR(): %s->pRefLists->RefPicList0[%u] (%u) does not match "
                                     "the slotIndex member of any element of pEncodeInfo->pReferenceSlots.",
                                     loc.pNext(Struct::VkVideoEncodeH264PictureInfoKHR, Field::pStdPictureInfo).Fields().c_str(), i,
                                     ref_list_entry);
                }
                ref_list_entries.insert(ref_list_entry);
            }

            for (uint8_t i = 0; i < STD_VIDEO_H264_MAX_NUM_LIST_REF; ++i) {
                uint8_t ref_list_entry = std_picture_info->pRefLists->RefPicList1[i];
                if (ref_list_entry == STD_VIDEO_H264_NO_REFERENCE_PICTURE) {
                    continue;
                }

                const auto &ref_slot = reference_slots.find((int32_t)ref_list_entry);
                if (ref_slot != reference_slots.end()) {
                    if (ref_slot->second != nullptr) {
                        auto std_reference_info = ref_slot->second->pStdReferenceInfo;

                        skip |= ValidateVideoEncodeH264PicType(vs_state, std_reference_info->primary_pic_type,
                                                               "primary_pic_type for L1 reference");

                        if (std_reference_info->primary_pic_type == STD_VIDEO_H264_PICTURE_TYPE_B &&
                            (profile_caps.encode_h264.flags & VK_VIDEO_ENCODE_H264_CAPABILITY_B_FRAME_IN_L1_LIST_BIT_KHR) == 0) {
                            const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
                            skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-flags-08343",
                                             "vkCmdEncodeVideoKHR(): primary_pic_type for L1 reference is "
                                             "STD_VIDEO_H264_PICTURE_TYPE_B but B pictures are not supported in the "
                                             "L1 reference list by the H.264 encode profile %s was created with.",
                                             FormatHandle(vs_state).c_str());
                        }
                    }
                } else {
                    skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdEncodeVideoKHR-pNext-08339",
                                     "vkCmdEncodeVideoKHR(): %s->pRefLists->RefPicList1[%u] (%u) does not match "
                                     "the slotIndex member of any element of pEncodeInfo->pReferenceSlots.",
                                     loc.pNext(Struct::VkVideoEncodeH264PictureInfoKHR, Field::pStdPictureInfo).Fields().c_str(), i,
                                     ref_list_entry);
                }
                ref_list_entries.insert(ref_list_entry);
            }

            for (uint32_t i = 0; i < encode_info.referenceSlotCount; ++i) {
                int32_t slot_index = encode_info.pReferenceSlots[i].slotIndex;
                if (slot_index >= 0 && (uint32_t)slot_index < vs_state.create_info.maxDpbSlots &&
                    ref_list_entries.find((uint8_t)slot_index) == ref_list_entries.end()) {
                    skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdEncodeVideoKHR-pNext-08353",
                                     "vkCmdEncodeVideoKHR(): pEncodeInfo->pReferenceSlots[%u].slotIndex (%d) does not match "
                                     "any of the elements of RefPicList0 or RefPicList1 in %s->pRefLists.",
                                     i, slot_index,
                                     loc.pNext(Struct::VkVideoEncodeH264PictureInfoKHR, Field::pStdPictureInfo).Fields().c_str());
                }
            }
        } else if (encode_info.referenceSlotCount > 0) {
            skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdEncodeVideoKHR-pNext-08352",
                             "vkCmdEncodeVideoKHR(): %s->pRefLists is NULL but pEncodeInfo->referenceSlotCount (%u) is not zero.",
                             loc.pNext(Struct::VkVideoEncodeH264PictureInfoKHR, Field::pStdPictureInfo).Fields().c_str(),
                             encode_info.referenceSlotCount);
        }

        if (picture_info->generatePrefixNalu &&
            (profile_caps.encode_h264.flags & VK_VIDEO_ENCODE_H264_CAPABILITY_GENERATE_PREFIX_NALU_BIT_KHR) == 0) {
            const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
            skip |= LogError("VUID-VkVideoEncodeH264PictureInfoKHR-flags-08304", objlist,
                             loc.pNext(Struct::VkVideoEncodeH264PictureInfoKHR, Field::generatePrefixNalu),
                             "is VK_TRUE but VK_VIDEO_ENCODE_H264_CAPABILITY_GENERATE_PREFIX_NALU_BIT_KHR "
                             "is not supported by the H.264 encode profile %s was created with.",
                             FormatHandle(vs_state).c_str());
        }
    } else {
        skip |= LogError("VUID-vkCmdEncodeVideoKHR-pNext-08225", cb_state.commandBuffer(), loc.dot(Field::pNext), pnext_msg,
                         "VkVideoEncodeH264PictureInfoKHR");
    }

    return skip;
}

bool CoreChecks::ValidateVideoEncodeH265PicType(const vvl::VideoSession &vs_state, StdVideoH265PictureType pic_type,
                                                const char *where) const {
    bool skip = false;

    const auto &profile_caps = vs_state.profile->GetCapabilities();

    if (profile_caps.encode_h265.maxPPictureL0ReferenceCount == 0 && pic_type == STD_VIDEO_H265_PICTURE_TYPE_P) {
        skip |= LogError(vs_state.videoSession(), "VUID-vkCmdEncodeVideoKHR-maxPPictureL0ReferenceCount-08345",
                         "vkCmdEncodeVideoKHR(): %s is STD_VIDEO_H265_PICTURE_TYPE_P but P pictures "
                         "are not supported by the H.265 encode profile %s was created with.",
                         where, FormatHandle(vs_state).c_str());
    }

    if (profile_caps.encode_h265.maxBPictureL0ReferenceCount == 0 && profile_caps.encode_h265.maxL1ReferenceCount == 0 &&
        pic_type == STD_VIDEO_H265_PICTURE_TYPE_B) {
        skip |= LogError(vs_state.videoSession(), "VUID-vkCmdEncodeVideoKHR-maxBPictureL0ReferenceCount-08346",
                         "vkCmdEncodeVideoKHR(): %s is STD_VIDEO_H265_PICTURE_TYPE_B but B pictures "
                         "are not supported by the H.265 profile %s was created with.",
                         where, FormatHandle(vs_state).c_str());
    }

    return skip;
}

bool CoreChecks::ValidateVideoEncodeInfoH265(const vvl::CommandBuffer &cb_state, const VkVideoEncodeInfoKHR &encode_info,
                                             const Location &loc) const {
    bool skip = false;

    const char *pnext_msg = "chain does not contain a %s structure.";

    const auto &vs_state = *cb_state.bound_video_session;
    const auto &profile_caps = vs_state.profile->GetCapabilities();

    const auto &rc_state = cb_state.video_encode_rate_control_state;

    const auto &vsp_state = *cb_state.bound_video_session_parameters;
    const auto session_params = vsp_state.Lock();

    if (encode_info.pSetupReferenceSlot) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoEncodeH265DpbSlotInfoKHR>(encode_info.pSetupReferenceSlot->pNext);
        if (!dpb_slot_info) {
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08234", cb_state.commandBuffer(),
                             loc.dot(Field::pSetupReferenceSlot).dot(Field::pNext), pnext_msg, "VkVideoEncodeH265DpbSlotInfoKHR");
        }
    }

    vvl::unordered_map<int32_t, const VkVideoEncodeH265DpbSlotInfoKHR *> reference_slots{};
    for (uint32_t i = 0; i < encode_info.referenceSlotCount; ++i) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoEncodeH265DpbSlotInfoKHR>(encode_info.pReferenceSlots[i].pNext);
        if (!dpb_slot_info) {
            skip |= LogError("VUID-vkCmdEncodeVideoKHR-pNext-08235", cb_state.commandBuffer(),
                             loc.dot(Field::pReferenceSlots, i).dot(Field::pNext), pnext_msg, "VkVideoEncodeH265DpbSlotInfoKHR");
        }

        reference_slots.insert({encode_info.pReferenceSlots[i].slotIndex, dpb_slot_info});
    }

    auto picture_info = vku::FindStructInPNextChain<VkVideoEncodeH265PictureInfoKHR>(encode_info.pNext);
    if (picture_info) {
        auto std_picture_info = picture_info->pStdPictureInfo;
        auto std_vps = session_params.GetH265VPS(std_picture_info->sps_video_parameter_set_id);
        auto std_sps =
            session_params.GetH265SPS(std_picture_info->sps_video_parameter_set_id, std_picture_info->pps_seq_parameter_set_id);
        auto std_pps =
            session_params.GetH265PPS(std_picture_info->sps_video_parameter_set_id, std_picture_info->pps_seq_parameter_set_id,
                                      std_picture_info->pps_pic_parameter_set_id);

        const Location slice_seg_count_loc = loc.pNext(Struct::VkVideoEncodeH265PictureInfoKHR, Field::naluSliceSegmentEntryCount);
        const Location slice_seg_list_loc = loc.pNext(Struct::VkVideoEncodeH265PictureInfoKHR, Field::pNaluSliceSegmentEntries);

        if (std_vps == nullptr) {
            const LogObjectList objlist(cb_state.commandBuffer(), vsp_state.videoSessionParameters());
            skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-StdVideoH265VideoParameterSet-08231",
                             "vkCmdEncodeVideoKHR(): no H.265 VPS with sps_video_parameter_set_id = %u "
                             "exists in the bound video session parameters object %s.",
                             std_picture_info->sps_video_parameter_set_id, FormatHandle(vsp_state).c_str());
        }

        if (std_sps == nullptr) {
            const LogObjectList objlist(cb_state.commandBuffer(), vsp_state.videoSessionParameters());
            skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-StdVideoH265SequenceParameterSet-08232",
                             "vkCmdEncodeVideoKHR(): no H.265 SPS with sps_video_parameter_set_id = %u "
                             "and pps_seq_parameter_set_id = %u exists in the bound video session "
                             "parameters object %s.",
                             std_picture_info->sps_video_parameter_set_id, std_picture_info->pps_seq_parameter_set_id,
                             FormatHandle(vsp_state).c_str());
        }

        if (std_pps == nullptr) {
            const LogObjectList objlist(cb_state.commandBuffer(), vsp_state.videoSessionParameters());
            skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-StdVideoH265PictureParameterSet-08233",
                             "vkCmdEncodeVideoKHR(): no H.265 SPS with sps_video_parameter_set_id = %u, "
                             "pps_seq_parameter_set_id = %u, and pps_pic_parameter_set_id = %u exists in "
                             "the bound video session parameters object %s.",
                             std_picture_info->sps_video_parameter_set_id, std_picture_info->pps_seq_parameter_set_id,
                             std_picture_info->pps_pic_parameter_set_id, FormatHandle(vsp_state).c_str());
        }

        if (picture_info->naluSliceSegmentEntryCount > profile_caps.encode_h265.maxSliceSegmentCount) {
            const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
            skip |= LogError("VUID-VkVideoEncodeH265PictureInfoKHR-naluSliceSegmentEntryCount-08306", objlist, slice_seg_count_loc,
                             "(%u) exceeds the VkVideoEncodeH265CapabilitiesKHR::maxSliceSegmentCount (%u) limit "
                             "supported by the H.265 encode profile %s was created with.",
                             picture_info->naluSliceSegmentEntryCount, profile_caps.encode_h265.maxSliceSegmentCount,
                             FormatHandle(vs_state).c_str());
        }

        VkExtent2D max_coding_block_size = vs_state.profile->GetMaxCodingBlockSize();
        VkExtent2D min_coding_block_extent = {
            encode_info.srcPictureResource.codedExtent.width / max_coding_block_size.width,
            encode_info.srcPictureResource.codedExtent.height / max_coding_block_size.height,
        };
        if (profile_caps.encode_h265.flags & VK_VIDEO_ENCODE_H265_CAPABILITY_ROW_UNALIGNED_SLICE_SEGMENT_BIT_KHR) {
            if (picture_info->naluSliceSegmentEntryCount > min_coding_block_extent.width * min_coding_block_extent.height) {
                const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
                skip |= LogError("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08307", objlist, slice_seg_count_loc,
                                 "(%u) is greater than the number of CTBs (minCodingBlockExtent = {%u, %u}) that can "
                                 "be coded for the encode input picture specified in pEncodeInfo->srcPictureResource "
                                 "(codedExtent = {%u, %u}) assuming the maximum CTB size (%ux%u) supported by the "
                                 "H.265 encode profile %s was created with.",
                                 picture_info->naluSliceSegmentEntryCount, min_coding_block_extent.width,
                                 min_coding_block_extent.height, encode_info.srcPictureResource.codedExtent.width,
                                 encode_info.srcPictureResource.codedExtent.height, max_coding_block_size.width,
                                 max_coding_block_size.height, FormatHandle(vs_state).c_str());
            }
        } else {
            if (picture_info->naluSliceSegmentEntryCount > min_coding_block_extent.height) {
                const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
                skip |=
                    LogError("VUID-vkCmdEncodeVideoKHR-naluSliceSegmentEntryCount-08313", objlist, slice_seg_count_loc,
                             "(%u) is greater than the number of CTB rows (minCodingBlockExtent.height = %u) that can "
                             "be coded for the encode input picture specified in pEncodeInfo->srcPictureResource "
                             "(codedExtent = {%u, %u}) assuming the maximum CTB size (%ux%u) supported by the "
                             "H.265 encode profile %s was created with.",
                             picture_info->naluSliceSegmentEntryCount, min_coding_block_extent.height,
                             encode_info.srcPictureResource.codedExtent.width, encode_info.srcPictureResource.codedExtent.height,
                             max_coding_block_size.width, max_coding_block_size.height, FormatHandle(vs_state).c_str());
            }
        }

        if (std_pps != nullptr) {
            uint32_t num_tiles = (std_pps->num_tile_columns_minus1 + 1) * (std_pps->num_tile_rows_minus1 + 1);

            if ((profile_caps.encode_h265.flags & VK_VIDEO_ENCODE_H265_CAPABILITY_MULTIPLE_TILES_PER_SLICE_SEGMENT_BIT_KHR) == 0 &&
                picture_info->naluSliceSegmentEntryCount < num_tiles) {
                const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
                skip |= LogError("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08323", objlist, slice_seg_count_loc,
                                 "(%u) is less than the number of H.265 tiles (%u) in the encoded picture "
                                 "(num_tile_columns_minus1 = %u and num_tile_rows_minus1 = %u in the active H.265 PPS) "
                                 "but multiple tiles per slice segment are not supported by the H.265 encode profile "
                                 "%s was created with.",
                                 picture_info->naluSliceSegmentEntryCount, num_tiles, std_pps->num_tile_columns_minus1,
                                 std_pps->num_tile_rows_minus1, FormatHandle(vs_state).c_str());
            }

            if ((profile_caps.encode_h265.flags & VK_VIDEO_ENCODE_H265_CAPABILITY_MULTIPLE_SLICE_SEGMENTS_PER_TILE_BIT_KHR) == 0 &&
                picture_info->naluSliceSegmentEntryCount > num_tiles) {
                const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
                skip |= LogError("VUID-VkVideoEncodeH265PictureInfoKHR-flags-08324", objlist, slice_seg_count_loc,
                                 "(%u) is greater than the number of H.265 tiles (%u) in the encoded picture "
                                 "(num_tile_columns_minus1 = %u and num_tile_rows_minus1 = %u in the active H.265 PPS) "
                                 "but multiple slice segments per tile are not supported by the H.265 encode profile "
                                 "%s was created with.",
                                 picture_info->naluSliceSegmentEntryCount, num_tiles, std_pps->num_tile_columns_minus1,
                                 std_pps->num_tile_rows_minus1, FormatHandle(vs_state).c_str());
            }
        }

        bool different_slice_segment_types = false;
        bool different_constant_qp_per_slice_segment = false;
        for (uint32_t slice_seg_idx = 0; slice_seg_idx < picture_info->naluSliceSegmentEntryCount; ++slice_seg_idx) {
            const auto &slice_segment_info = picture_info->pNaluSliceSegmentEntries[slice_seg_idx];
            const auto *std_slice_segment_header = slice_segment_info.pStdSliceSegmentHeader;
            const Location slice_seg_info_loc =
                loc.pNext(Struct::VkVideoEncodeH265PictureInfoKHR, Field::pNaluSliceSegmentEntries, slice_seg_idx);

            if (std_slice_segment_header->slice_type !=
                picture_info->pNaluSliceSegmentEntries[0].pStdSliceSegmentHeader->slice_type) {
                different_slice_segment_types = true;
            }

            if (rc_state.base.rateControlMode == VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR) {
                if (slice_segment_info.constantQp < profile_caps.encode_h265.minQp ||
                    slice_segment_info.constantQp > profile_caps.encode_h265.maxQp) {
                    const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
                    skip |=
                        LogError("VUID-vkCmdEncodeVideoKHR-constantQp-08273", objlist, slice_seg_info_loc.dot(Field::constantQp),
                                 "(%d) is outside of the range [%d, %d] supported by the video "
                                 "profile %s was created with.",
                                 slice_segment_info.constantQp, profile_caps.encode_h265.minQp, profile_caps.encode_h265.maxQp,
                                 FormatHandle(vs_state).c_str());
                }

                if (slice_segment_info.constantQp != picture_info->pNaluSliceSegmentEntries[0].constantQp) {
                    different_constant_qp_per_slice_segment = true;
                }
            } else {
                if (slice_segment_info.constantQp != 0) {
                    const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
                    skip |=
                        LogError("VUID-vkCmdEncodeVideoKHR-constantQp-08272", objlist, slice_seg_info_loc.dot(Field::constantQp),
                                 "(%d) is not zero but the currently set video encode rate control mode for %s "
                                 "was specified to be %s when beginning the video coding scope.",
                                 slice_segment_info.constantQp, FormatHandle(vs_state).c_str(),
                                 string_VkVideoEncodeRateControlModeFlagBitsKHR(rc_state.base.rateControlMode));
                }
            }

            if (std_pps != nullptr &&
                (profile_caps.encode_h265.flags & VK_VIDEO_ENCODE_H265_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR) == 0) {
                const char *weighted_pred_error_msg = nullptr;

                if (std_slice_segment_header->slice_type == STD_VIDEO_H265_SLICE_TYPE_P && std_pps->flags.weighted_pred_flag) {
                    weighted_pred_error_msg =
                        "weighted_pred_flag is set in the active H.265 PPS, slice_type is STD_VIDEO_H265_SLICE_TYPE_P";
                } else if (std_slice_segment_header->slice_type == STD_VIDEO_H265_SLICE_TYPE_B &&
                           std_pps->flags.weighted_bipred_flag) {
                    weighted_pred_error_msg =
                        "weighted_bipred_flag is set in the active H.265 PPS, slice_type is STD_VIDEO_H265_SLICE_TYPE_B";
                }

                if (std_slice_segment_header->pWeightTable == nullptr && weighted_pred_error_msg != nullptr) {
                    const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
                    skip |=
                        LogError(objlist, "VUID-VkVideoEncodeH265PictureInfoKHR-flags-08316",
                                 "vkCmdEncodeVideoKHR(): %s, and pWeightTable is NULL in %s but "
                                 "VK_VIDEO_ENCODE_H265_CAPABILITY_PREDICTION_WEIGHT_TABLE_GENERATED_BIT_KHR "
                                 "is not supported by the H.265 encode profile %s was created with.",
                                 weighted_pred_error_msg, slice_seg_info_loc.dot(Field::pStdSliceSegmentHeader).Fields().c_str(),
                                 FormatHandle(vs_state).c_str());
                }
            }
        }

        if ((profile_caps.encode_h265.flags & VK_VIDEO_ENCODE_H265_CAPABILITY_DIFFERENT_SLICE_SEGMENT_TYPE_BIT_KHR) == 0 &&
            different_slice_segment_types) {
            const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
            skip |= LogError(objlist, "VUID-VkVideoEncodeH265PictureInfoKHR-flags-08317",
                             "VkCmdEncodeVideoKHR(): pStdSliceSegmentHeader->slice_type does not match across the elements "
                             "of %s but different slice segment types in a picture are not supported by the H.265 encode "
                             "profile %s was created with.",
                             slice_seg_list_loc.Fields().c_str(), FormatHandle(vs_state).c_str());
        }

        if ((profile_caps.encode_h265.flags & VK_VIDEO_ENCODE_H265_CAPABILITY_PER_SLICE_SEGMENT_CONSTANT_QP_BIT_KHR) == 0 &&
            different_constant_qp_per_slice_segment) {
            const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
            skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-constantQp-08274",
                             "VkCmdEncodeVideoKHR(): constantQp does not match across the elements of %s "
                             "but per-slice-segment constant QP values are not supported by the H.265 encode "
                             "profile %s was created with.",
                             slice_seg_list_loc.Fields().c_str(), FormatHandle(vs_state).c_str());
        }

        skip |= ValidateVideoEncodeH265PicType(vs_state, std_picture_info->pic_type,
                                               "VkVideoEncodeH265PictureInfoKHR::pStdPictureInfo->pic_type");

        if (std_picture_info->pRefLists != nullptr) {
            vvl::unordered_set<uint8_t> ref_list_entries{};

            for (uint8_t i = 0; i < STD_VIDEO_H265_MAX_NUM_LIST_REF; ++i) {
                uint8_t ref_list_entry = std_picture_info->pRefLists->RefPicList0[i];
                if (ref_list_entry == STD_VIDEO_H265_NO_REFERENCE_PICTURE) {
                    continue;
                }

                const auto &ref_slot = reference_slots.find((int32_t)ref_list_entry);
                if (ref_slot != reference_slots.end()) {
                    if (ref_slot->second != nullptr) {
                        auto std_reference_info = ref_slot->second->pStdReferenceInfo;

                        skip |= ValidateVideoEncodeH265PicType(vs_state, std_reference_info->pic_type, "pic_type for L0 reference");

                        if (std_reference_info->pic_type == STD_VIDEO_H265_PICTURE_TYPE_B &&
                            (profile_caps.encode_h265.flags & VK_VIDEO_ENCODE_H265_CAPABILITY_B_FRAME_IN_L0_LIST_BIT_KHR) == 0) {
                            const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
                            skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-flags-08347",
                                             "vkCmdEncodeVideoKHR(): pic_type for L0 reference is "
                                             "STD_VIDEO_H265_PICTURE_TYPE_B but B pictures are not supported in the "
                                             "L0 reference list by the H.265 encode profile %s was created with.",
                                             FormatHandle(vs_state).c_str());
                        }
                    }
                } else {
                    skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdEncodeVideoKHR-pNext-08344",
                                     "vkCmdEncodeVideoKHR(): %s->pRefLists->RefPicList0[%u] (%u) does not match "
                                     "the slotIndex member of any element of pEncodeInfo->pReferenceSlots.",
                                     loc.pNext(Struct::VkVideoEncodeH265PictureInfoKHR, Field::pStdPictureInfo).Fields().c_str(), i,
                                     ref_list_entry);
                }
                ref_list_entries.insert(ref_list_entry);
            }

            for (uint8_t i = 0; i < STD_VIDEO_H265_MAX_NUM_LIST_REF; ++i) {
                uint8_t ref_list_entry = std_picture_info->pRefLists->RefPicList1[i];
                if (ref_list_entry == STD_VIDEO_H265_NO_REFERENCE_PICTURE) {
                    continue;
                }

                const auto &ref_slot = reference_slots.find((int32_t)ref_list_entry);
                if (ref_slot != reference_slots.end()) {
                    if (ref_slot->second != nullptr) {
                        auto std_reference_info = ref_slot->second->pStdReferenceInfo;

                        skip |= ValidateVideoEncodeH265PicType(vs_state, std_reference_info->pic_type, "pic_type for L1 reference");

                        if (std_reference_info->pic_type == STD_VIDEO_H265_PICTURE_TYPE_B &&
                            (profile_caps.encode_h265.flags & VK_VIDEO_ENCODE_H265_CAPABILITY_B_FRAME_IN_L1_LIST_BIT_KHR) == 0) {
                            const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
                            skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-flags-08348",
                                             "vkCmdEncodeVideoKHR(): pic_type for L1 reference is "
                                             "STD_VIDEO_H265_PICTURE_TYPE_B but B pictures are not supported in the "
                                             "L1 reference list by the H.265 encode profile %s was created with.",
                                             FormatHandle(vs_state).c_str());
                        }
                    }
                } else {
                    skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdEncodeVideoKHR-pNext-08344",
                                     "vkCmdEncodeVideoKHR(): %s->pRefLists->RefPicList1[%u] (%u) does not match "
                                     "the slotIndex member of any element of pEncodeInfo->pReferenceSlots.",
                                     loc.pNext(Struct::VkVideoEncodeH265PictureInfoKHR, Field::pStdPictureInfo).Fields().c_str(), i,
                                     ref_list_entry);
                }
                ref_list_entries.insert(ref_list_entry);
            }

            for (uint32_t i = 0; i < encode_info.referenceSlotCount; ++i) {
                int32_t slot_index = encode_info.pReferenceSlots[i].slotIndex;
                if (slot_index >= 0 && (uint32_t)slot_index < vs_state.create_info.maxDpbSlots &&
                    ref_list_entries.find((uint8_t)slot_index) == ref_list_entries.end()) {
                    skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdEncodeVideoKHR-pNext-08355",
                                     "vkCmdEncodeVideoKHR(): pEncodeInfo->pReferenceSlots[%u].slotIndex (%d) does not match "
                                     "any of the elements of RefPicList0 or RefPicList1 in %s->pRefLists.",
                                     i, slot_index,
                                     loc.pNext(Struct::VkVideoEncodeH265PictureInfoKHR, Field::pStdPictureInfo).Fields().c_str());
                }
            }
        } else if (encode_info.referenceSlotCount > 0) {
            skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdEncodeVideoKHR-pNext-08354",
                             "vkCmdEncodeVideoKHR(): %s->pRefLists is NULL but pEncodeInfo->referenceSlotCount (%u) is not zero.",
                             loc.pNext(Struct::VkVideoEncodeH265PictureInfoKHR, Field::pStdPictureInfo).Fields().c_str(),
                             encode_info.referenceSlotCount);
        }
    } else {
        skip |= LogError("VUID-vkCmdEncodeVideoKHR-pNext-08230", cb_state.commandBuffer(), loc.dot(Field::pNext), pnext_msg,
                         "VkVideoEncodeH265PictureInfoKHR");
    }

    return skip;
}

bool CoreChecks::ValidateActiveReferencePictureCount(const vvl::CommandBuffer &cb_state,
                                                     const VkVideoDecodeInfoKHR &decode_info) const {
    bool skip = false;

    const auto &vs_state = *cb_state.bound_video_session;

    uint32_t active_reference_picture_count = decode_info.referenceSlotCount;

    if (vs_state.GetCodecOp() == VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR) {
        for (uint32_t i = 0; i < decode_info.referenceSlotCount; ++i) {
            auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoDecodeH264DpbSlotInfoKHR>(decode_info.pReferenceSlots[i].pNext);
            if (!dpb_slot_info) continue;

            auto std_reference_info = dpb_slot_info->pStdReferenceInfo;
            if (!std_reference_info) continue;

            if (std_reference_info->flags.top_field_flag && std_reference_info->flags.bottom_field_flag) {
                ++active_reference_picture_count;
            }
        }
    }

    if (active_reference_picture_count > vs_state.create_info.maxActiveReferencePictures) {
        const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
        skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-activeReferencePictureCount-07150",
                         "vkCmdDecodeVideoKHR(): more active reference pictures (%u) were specified than "
                         "the maxActiveReferencePictures (%u) the bound video session %s was created with.",
                         active_reference_picture_count, vs_state.create_info.maxActiveReferencePictures,
                         FormatHandle(vs_state).c_str());
    }

    return skip;
}

bool CoreChecks::ValidateActiveReferencePictureCount(const vvl::CommandBuffer &cb_state,
                                                     const VkVideoEncodeInfoKHR &encode_info) const {
    bool skip = false;

    const auto &vs_state = *cb_state.bound_video_session;

    uint32_t active_reference_picture_count = encode_info.referenceSlotCount;

    if (active_reference_picture_count > vs_state.create_info.maxActiveReferencePictures) {
        const LogObjectList objlist(cb_state.commandBuffer(), vs_state.videoSession());
        skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-activeReferencePictureCount-08216",
                         "vkCmdEncodeVideoKHR(): more active reference pictures (%u) were specified than "
                         "the maxActiveReferencePictures (%u) the bound video session %s was created with.",
                         active_reference_picture_count, vs_state.create_info.maxActiveReferencePictures,
                         FormatHandle(vs_state).c_str());
    }

    return skip;
}

bool CoreChecks::ValidateReferencePictureUseCount(const vvl::CommandBuffer &cb_state,
                                                  const VkVideoDecodeInfoKHR &decode_info) const {
    bool skip = false;

    const auto &vs_state = *cb_state.bound_video_session;

    std::vector<uint32_t> dpb_frame_use_count(vs_state.create_info.maxDpbSlots, 0);

    bool interlaced_frame_support = false;
    std::vector<uint32_t> dpb_top_field_use_count;
    std::vector<uint32_t> dpb_bottom_field_use_count;

    if (vs_state.GetCodecOp() == VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR) {
        if (vs_state.profile->GetH264PictureLayout() != VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_PROGRESSIVE_KHR) {
            interlaced_frame_support = true;
            dpb_top_field_use_count.resize(vs_state.create_info.maxDpbSlots, 0);
            dpb_bottom_field_use_count.resize(vs_state.create_info.maxDpbSlots, 0);
        }
    }

    // Collect use count for each DPB across the elements pReferenceSlots and pSetupReferenceSlot
    for (uint32_t i = 0; i <= decode_info.referenceSlotCount; ++i) {
        const VkVideoReferenceSlotInfoKHR *slot =
            (i == decode_info.referenceSlotCount) ? decode_info.pSetupReferenceSlot : &decode_info.pReferenceSlots[i];

        if (slot == nullptr) continue;
        if (slot->slotIndex < 0 || (uint32_t)slot->slotIndex >= vs_state.create_info.maxDpbSlots) continue;

        ++dpb_frame_use_count[slot->slotIndex];

        if (!interlaced_frame_support) continue;

        if (vs_state.GetCodecOp() == VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR) {
            auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoDecodeH264DpbSlotInfoKHR>(slot->pNext);
            if (!dpb_slot_info) continue;

            auto std_reference_info = dpb_slot_info->pStdReferenceInfo;
            if (!std_reference_info) continue;

            if (std_reference_info->flags.top_field_flag || std_reference_info->flags.bottom_field_flag) {
                --dpb_frame_use_count[slot->slotIndex];
            }
            if (std_reference_info->flags.top_field_flag) {
                ++dpb_top_field_use_count[slot->slotIndex];
            }
            if (std_reference_info->flags.bottom_field_flag) {
                ++dpb_bottom_field_use_count[slot->slotIndex];
            }
        }
    }

    for (uint32_t i = 0; i < vs_state.create_info.maxDpbSlots; ++i) {
        if (dpb_frame_use_count[i] > 1) {
            skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdDecodeVideoKHR-dpbFrameUseCount-07176",
                             "vkCmdDecodeVideoKHR(): frame in DPB slot %u is referred to multiple times across "
                             "pDecodeInfo->pSetupReferenceSlot and the elements of pDecodeInfo->pReferenceSlots.",
                             i);
        }
        if (interlaced_frame_support) {
            if (dpb_top_field_use_count[i] > 1) {
                skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdDecodeVideoKHR-dpbTopFieldUseCount-07177",
                                 "vkCmdDecodeVideoKHR(): top field in DPB slot %u is referred to multiple "
                                 "times across pDecodeInfo->pSetupReferenceSlot and the elements of "
                                 "pDecodeInfo->pReferenceSlots.",
                                 i);
            }
            if (dpb_bottom_field_use_count[i] > 1) {
                skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdDecodeVideoKHR-dpbBottomFieldUseCount-07178",
                                 "vkCmdDecodeVideoKHR(): bottom field in DPB slot %u is referred to multiple "
                                 "times across pDecodeInfo->pSetupReferenceSlot and the elements of "
                                 "pDecodeInfo->pReferenceSlots.",
                                 i);
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateReferencePictureUseCount(const vvl::CommandBuffer &cb_state,
                                                  const VkVideoEncodeInfoKHR &encode_info) const {
    bool skip = false;

    const auto &vs_state = *cb_state.bound_video_session;

    std::vector<uint32_t> dpb_frame_use_count(vs_state.create_info.maxDpbSlots, 0);

    // Collect use count for each DPB across the elements pReferenceSlots and pSetupReferenceSlot
    for (uint32_t i = 0; i <= encode_info.referenceSlotCount; ++i) {
        const VkVideoReferenceSlotInfoKHR *slot =
            (i == encode_info.referenceSlotCount) ? encode_info.pSetupReferenceSlot : &encode_info.pReferenceSlots[i];

        if (slot == nullptr) continue;
        if (slot->slotIndex < 0 || (uint32_t)slot->slotIndex >= vs_state.create_info.maxDpbSlots) continue;

        ++dpb_frame_use_count[slot->slotIndex];
    }

    for (uint32_t i = 0; i < vs_state.create_info.maxDpbSlots; ++i) {
        if (dpb_frame_use_count[i] > 1) {
            skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdEncodeVideoKHR-dpbFrameUseCount-08221",
                             "vkCmdEncodeVideoKHR(): frame in DPB slot %u is referred to multiple times across "
                             "pEncodeInfo->pSetupReferenceSlot and the elements of pEncodeInfo->pReferenceSlots.",
                             i);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                                      const VkVideoProfileInfoKHR *pVideoProfile,
                                                                      VkVideoCapabilitiesKHR *pCapabilities,
                                                                      const ErrorObject &error_obj) const {
    bool skip = false;

    skip |= ValidateVideoProfileInfo(pVideoProfile, device, error_obj.location.dot(Field::pVideoProfile));

    const char *caps_pnext_msg = "chain does not contain a %s structure.";

    const Location caps_loc = error_obj.location.dot(Field::pCapabilities);

    bool is_decode = false;
    bool is_encode = false;

    switch (pVideoProfile->videoCodecOperation) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
            is_decode = true;
            if (!vku::FindStructInPNextChain<VkVideoDecodeH264CapabilitiesKHR>(pCapabilities->pNext)) {
                skip |= LogError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07184", physicalDevice, caps_loc,
                                 caps_pnext_msg, "VkVideoDecodeH264CapabilitiesKHR");
            }
            break;

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
            is_decode = true;
            if (!vku::FindStructInPNextChain<VkVideoDecodeH265CapabilitiesKHR>(pCapabilities->pNext)) {
                skip |= LogError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07185", physicalDevice, caps_loc,
                                 caps_pnext_msg, "VkVideoDecodeH265CapabilitiesKHR");
            }
            break;

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
            is_encode = true;
            if (!vku::FindStructInPNextChain<VkVideoEncodeH264CapabilitiesKHR>(pCapabilities->pNext)) {
                skip |= LogError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07187", physicalDevice, caps_loc,
                                 caps_pnext_msg, "VkVideoEncodeH264CapabilitiesKHR");
            }
            break;

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
            is_encode = true;
            if (!vku::FindStructInPNextChain<VkVideoEncodeH265CapabilitiesKHR>(pCapabilities->pNext)) {
                skip |= LogError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07188", physicalDevice, caps_loc,
                                 caps_pnext_msg, "VkVideoEncodeH265CapabilitiesKHR");
            }
            break;

        default:
            break;
    }

    if (is_decode && !vku::FindStructInPNextChain<VkVideoDecodeCapabilitiesKHR>(pCapabilities->pNext)) {
        skip |= LogError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07183", physicalDevice, caps_loc,
                         caps_pnext_msg, "VkVideoDecodeCapabilitiesKHR");
    }

    if (is_encode && !vku::FindStructInPNextChain<VkVideoEncodeCapabilitiesKHR>(pCapabilities->pNext)) {
        skip |= LogError("VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07186", physicalDevice, caps_loc,
                         caps_pnext_msg, "VkVideoEncodeCapabilitiesKHR");
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceVideoFormatPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoFormatInfoKHR *pVideoFormatInfo,
    uint32_t *pVideoFormatPropertyCount, VkVideoFormatPropertiesKHR *pVideoFormatProperties, const ErrorObject &error_obj) const {
    bool skip = false;

    const auto *video_profiles = vku::FindStructInPNextChain<VkVideoProfileListInfoKHR>(pVideoFormatInfo->pNext);
    if (video_profiles && video_profiles->profileCount != 0) {
        skip |=
            ValidateVideoProfileListInfo(video_profiles, physicalDevice,
                                         error_obj.location.dot(Field::pVideoFormatInfo).pNext(Struct::VkVideoProfileListInfoKHR),
                                         false, nullptr, false, nullptr);
    } else {
        const char *msg = video_profiles ? "no VkVideoProfileListInfoKHR structure found in the pNext chain of pVideoFormatInfo."
                                         : "profileCount is zero in the VkVideoProfileListInfoKHR structure included in the "
                                           "pNext chain of pVideoFormatInfo.";
        skip |= LogError(physicalDevice, "VUID-vkGetPhysicalDeviceVideoFormatPropertiesKHR-pNext-06812",
                         "vkGetPhysicalDeviceVideoFormatPropertiesKHR(): %s", msg);
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR *pQualityLevelInfo,
    VkVideoEncodeQualityLevelPropertiesKHR *pQualityLevelProperties, const ErrorObject &error_obj) const {
    bool skip = false;

    const Location quality_level_info_loc = error_obj.location.dot(Field::pQualityLevelInfo);
    const Location quality_level_props_loc = error_obj.location.dot(Field::pQualityLevelProperties);

    const char *props_pnext_msg = "chain does not contain a %s structure.";

    skip |= ValidateVideoProfileInfo(pQualityLevelInfo->pVideoProfile, device, quality_level_info_loc.dot(Field::pVideoProfile));

    vvl::VideoProfileDesc profile_desc(physicalDevice, pQualityLevelInfo->pVideoProfile);
    const auto &profile_caps = profile_desc.GetCapabilities();

    if (!profile_desc.IsEncode()) {
        skip |= LogError("VUID-VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR-pVideoProfile-08260", physicalDevice,
                         quality_level_info_loc.dot(Field::pVideoProfile), "does not specify an encode profile.");
    }

    if (profile_caps.supported) {
        if (profile_desc.IsEncode() && pQualityLevelInfo->qualityLevel >= profile_caps.encode.maxQualityLevels) {
            skip |= LogError("VUID-VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR-qualityLevel-08261", physicalDevice,
                             quality_level_info_loc.dot(Field::qualityLevel),
                             "(%u) must be smaller than the VkVideoEncodeCapabilitiesKHR::maxQualityLevels (%u) limit "
                             "supported by the specified video profile.",
                             pQualityLevelInfo->qualityLevel, profile_caps.encode.maxQualityLevels);
        }
    } else {
        skip |= LogError(physicalDevice, "VUID-VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR-pVideoProfile-08259",
                         "vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(): the video profile specified "
                         "pQualityLevelInfo->pVideoProfile is not supported.");
    }

    switch (pQualityLevelInfo->pVideoProfile->videoCodecOperation) {
        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
            if (!vku::FindStructInPNextChain<VkVideoEncodeH264QualityLevelPropertiesKHR>(pQualityLevelProperties->pNext)) {
                skip |=
                    LogError("VUID-vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR-pQualityLevelInfo-08257", physicalDevice,
                             quality_level_props_loc, props_pnext_msg, "VkVideoEncodeH264QualityLevelPropertiesKHR");
            }
            break;

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
            if (!vku::FindStructInPNextChain<VkVideoEncodeH265QualityLevelPropertiesKHR>(pQualityLevelProperties->pNext)) {
                skip |=
                    LogError("VUID-vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR-pQualityLevelInfo-08258", physicalDevice,
                             quality_level_props_loc, props_pnext_msg, "VkVideoEncodeH264QualityLevelPropertiesKHR");
            }
            break;

        default:
            break;
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR *pCreateInfo,
                                                      const VkAllocationCallbacks *pAllocator, VkVideoSessionKHR *pVideoSession,
                                                      const ErrorObject &error_obj) const {
    bool skip = false;

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);

    skip |= ValidateVideoProfileInfo(pCreateInfo->pVideoProfile, device, create_info_loc.dot(Field::pVideoProfile));

    vvl::VideoProfileDesc profile_desc(physical_device, pCreateInfo->pVideoProfile);
    const auto &profile_caps = profile_desc.GetCapabilities();

    if (profile_caps.supported) {
        if (pCreateInfo->flags & VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR) {
            const char *error_msg = nullptr;
            if (enabled_features.protectedMemory == VK_FALSE) {
                error_msg = "the protectedMemory feature is not enabled";
            } else if ((profile_caps.base.flags & VK_VIDEO_CAPABILITY_PROTECTED_CONTENT_BIT_KHR) == 0) {
                error_msg = "protected content is not supported for the video profile";
            }
            if (error_msg != nullptr) {
                skip |=
                    LogError("VUID-VkVideoSessionCreateInfoKHR-protectedMemory-07189", device, create_info_loc.dot(Field::flags),
                             "has VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR set but %s.", error_msg);
            }
        }

        if (pCreateInfo->flags & VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR && !enabled_features.videoMaintenance1) {
            skip |= LogError("VUID-VkVideoSessionCreateInfoKHR-flags-08371", device, create_info_loc.dot(Field::flags),
                             "has VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR set but "
                             "the videoMaintenance1 device feature is not enabled.");
        }

        if (!IsBetweenInclusive(pCreateInfo->maxCodedExtent, profile_caps.base.minCodedExtent, profile_caps.base.maxCodedExtent)) {
            skip |= LogError(device, "VUID-VkVideoSessionCreateInfoKHR-maxCodedExtent-04851",
                             "vkCreateVideoSessionKHR(): pCreateInfo->maxCodedExtent (%u,%u) is outside of the "
                             "range (%u,%u)-(%u,%u) supported by the video profile.",
                             pCreateInfo->maxCodedExtent.width, pCreateInfo->maxCodedExtent.height,
                             profile_caps.base.minCodedExtent.width, profile_caps.base.minCodedExtent.height,
                             profile_caps.base.maxCodedExtent.width, profile_caps.base.maxCodedExtent.height);
        }

        if (pCreateInfo->maxDpbSlots > profile_caps.base.maxDpbSlots) {
            skip |= LogError(device, "VUID-VkVideoSessionCreateInfoKHR-maxDpbSlots-04847",
                             "vkCreateVideoSessionKHR(): pCreateInfo->maxDpbSlots (%u) is greater than the "
                             "maxDpbSlots (%u) supported by the video profile.",
                             pCreateInfo->maxDpbSlots, profile_caps.base.maxDpbSlots);
        }

        if (pCreateInfo->maxActiveReferencePictures > profile_caps.base.maxActiveReferencePictures) {
            skip |= LogError(device, "VUID-VkVideoSessionCreateInfoKHR-maxActiveReferencePictures-04849",
                             "vkCreateVideoSessionKHR(): pCreateInfo->maxActiveReferencePictures (%u) is greater "
                             "than the maxActiveReferencePictures (%u) supported by the video profile.",
                             pCreateInfo->maxActiveReferencePictures, profile_caps.base.maxActiveReferencePictures);
        }

        if ((pCreateInfo->maxDpbSlots == 0 && pCreateInfo->maxActiveReferencePictures != 0) ||
            (pCreateInfo->maxDpbSlots != 0 && pCreateInfo->maxActiveReferencePictures == 0)) {
            skip |= LogError(device, "VUID-VkVideoSessionCreateInfoKHR-maxDpbSlots-04850",
                             "vkCreateVideoSessionKHR(): if either pCreateInfo->maxDpbSlots (%u) or "
                             "pCreateInfo->maxActiveReferencePictures (%u) is zero then both must be zero.",
                             pCreateInfo->maxDpbSlots, pCreateInfo->maxActiveReferencePictures);
        }

        if (profile_desc.IsDecode() && pCreateInfo->maxActiveReferencePictures > 0 &&
            !IsVideoFormatSupported(pCreateInfo->referencePictureFormat, VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR,
                                    pCreateInfo->pVideoProfile)) {
            skip |= LogError(device, "VUID-VkVideoSessionCreateInfoKHR-referencePictureFormat-04852",
                             "vkCreateVideoSessionKHR(): pCreateInfo->referencePictureFormat (%s) is not a supported "
                             "decode DPB format for the video profile specified in pCreateInfo->pVideoProfile.",
                             string_VkFormat(pCreateInfo->referencePictureFormat));
        }

        if (profile_desc.IsEncode() && pCreateInfo->maxActiveReferencePictures > 0 &&
            !IsVideoFormatSupported(pCreateInfo->referencePictureFormat, VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR,
                                    pCreateInfo->pVideoProfile)) {
            skip |= LogError(device, "VUID-VkVideoSessionCreateInfoKHR-referencePictureFormat-06814",
                             "vkCreateVideoSessionKHR(): pCreateInfo->referencePictureFormat (%s) is not a supported "
                             "encode DPB format for the video profile specified in pCreateInfo->pVideoProfile.",
                             string_VkFormat(pCreateInfo->referencePictureFormat));
        }

        if (profile_desc.IsDecode() && !IsVideoFormatSupported(pCreateInfo->pictureFormat, VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR,
                                                               pCreateInfo->pVideoProfile)) {
            skip |= LogError(device, "VUID-VkVideoSessionCreateInfoKHR-pictureFormat-04853",
                             "vkCreateVideoSessionKHR(): pCreateInfo->pictureFormat (%s) is not a supported "
                             "decode output format for the video profile specified in pCreateInfo->pVideoProfile.",
                             string_VkFormat(pCreateInfo->pictureFormat));
        }

        if (profile_desc.IsEncode() && !IsVideoFormatSupported(pCreateInfo->pictureFormat, VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR,
                                                               pCreateInfo->pVideoProfile)) {
            skip |= LogError(device, "VUID-VkVideoSessionCreateInfoKHR-pictureFormat-04854",
                             "vkCreateVideoSessionKHR(): pCreateInfo->pictureFormat (%s) is not a supported "
                             "encode input format for the video profile specified in pCreateInfo->pVideoProfile.",
                             string_VkFormat(pCreateInfo->pictureFormat));
        }

        if (strncmp(pCreateInfo->pStdHeaderVersion->extensionName, profile_caps.base.stdHeaderVersion.extensionName,
                    VK_MAX_EXTENSION_NAME_SIZE)) {
            skip |= LogError(device, "VUID-VkVideoSessionCreateInfoKHR-pStdHeaderVersion-07190",
                             "vkCreateVideoSessionKHR(): unsupported Video Std header name '%.*s' specified in "
                             "pCreateInfo->pStdHeaderVersion->extensionName, expected '%.*s'.",
                             VK_MAX_EXTENSION_NAME_SIZE, pCreateInfo->pStdHeaderVersion->extensionName, VK_MAX_EXTENSION_NAME_SIZE,
                             profile_caps.base.stdHeaderVersion.extensionName);
        }

        if (pCreateInfo->pStdHeaderVersion->specVersion > profile_caps.base.stdHeaderVersion.specVersion) {
            skip |= LogError(device, "VUID-VkVideoSessionCreateInfoKHR-pStdHeaderVersion-07191",
                             "vkCreateVideoSessionKHR(): Video Std header version (0x%08x) specified in "
                             "pCreateInfo->pStdHeaderVersion->specVersion is larger than the supported version (0x%08x).",
                             pCreateInfo->pStdHeaderVersion->specVersion, profile_caps.base.stdHeaderVersion.specVersion);
        }
    } else {
        skip |= LogError(device, "VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-04845",
                         "vkCreateVideoSessionKHR(): the video profile specified in pCreateInfo->pVideoProfile "
                         "is not supported.");
    }

    switch (pCreateInfo->pVideoProfile->videoCodecOperation) {
        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
            auto h264_create_info = vku::FindStructInPNextChain<VkVideoEncodeH264SessionCreateInfoKHR>(pCreateInfo);
            if (h264_create_info != nullptr && h264_create_info->maxLevelIdc > profile_caps.encode_h264.maxLevelIdc) {
                skip |= LogError("VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-08251", device,
                                 create_info_loc.pNext(Struct::VkVideoEncodeH264SessionCreateInfoKHR, Field::maxLevelIdc),
                                 "(%u) exceeds the maxLevelIdc (%u) supported by the specified H.264 encode profile.",
                                 h264_create_info->maxLevelIdc, profile_caps.encode_h264.maxLevelIdc);
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
            auto h265_create_info = vku::FindStructInPNextChain<VkVideoEncodeH265SessionCreateInfoKHR>(pCreateInfo);
            if (h265_create_info != nullptr && h265_create_info->maxLevelIdc > profile_caps.encode_h265.maxLevelIdc) {
                skip |= LogError("VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-08252", device,
                                 create_info_loc.pNext(Struct::VkVideoEncodeH265SessionCreateInfoKHR, Field::maxLevelIdc),
                                 "(%u) exceeds the maxLevelIdc (%u) supported by the specified H.265 encode profile.",
                                 h265_create_info->maxLevelIdc, profile_caps.encode_h265.maxLevelIdc);
            }
            break;
        }

        default:
            break;
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                       const VkAllocationCallbacks *pAllocator,
                                                       const ErrorObject &error_obj) const {
    auto video_session_state = Get<vvl::VideoSession>(videoSession);
    bool skip = false;
    if (video_session_state) {
        skip |= ValidateObjectNotInUse(video_session_state.get(), error_obj.location,
                                       "VUID-vkDestroyVideoSessionKHR-videoSession-07192");
    }
    return skip;
}

bool CoreChecks::PreCallValidateBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                          uint32_t bindSessionMemoryInfoCount,
                                                          const VkBindVideoSessionMemoryInfoKHR *pBindSessionMemoryInfos,
                                                          const ErrorObject &error_obj) const {
    bool skip = false;

    auto vs_state = Get<vvl::VideoSession>(videoSession);
    if (!vs_state) return false;

    if (pBindSessionMemoryInfos) {
        {
            vvl::unordered_set<uint32_t> memory_bind_indices;
            for (uint32_t i = 0; i < bindSessionMemoryInfoCount; ++i) {
                uint32_t mem_bind_index = pBindSessionMemoryInfos[i].memoryBindIndex;
                if (memory_bind_indices.find(mem_bind_index) != memory_bind_indices.end()) {
                    skip |= LogError(videoSession, "VUID-vkBindVideoSessionMemoryKHR-memoryBindIndex-07196",
                                     "vkBindVideoSessionMemoryKHR(): memoryBindIndex values in pBindSessionMemoryInfos "
                                     "array are not unique.");
                    break;
                }
                memory_bind_indices.emplace(mem_bind_index);
            }
        }

        for (uint32_t i = 0; i < bindSessionMemoryInfoCount; ++i) {
            const auto &bind_info = pBindSessionMemoryInfos[i];
            const auto &mem_binding_info = vs_state->GetMemoryBindingInfo(bind_info.memoryBindIndex);
            if (mem_binding_info != nullptr) {
                auto mem_state = Get<vvl::DeviceMemory>(bind_info.memory);
                if (mem_state) {
                    if (((1 << mem_state->alloc_info.memoryTypeIndex) & mem_binding_info->requirements.memoryTypeBits) == 0) {
                        const LogObjectList objlist(videoSession, mem_state->Handle());
                        skip |= LogError(objlist, "VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07198",
                                         "vkBindVideoSessionMemoryKHR(): memoryTypeBits (0x%x) for memory binding "
                                         "with index %u of %s are not compatible with the memory type index (%u) of "
                                         "%s specified in pBindSessionMemoryInfos[%u].memory.",
                                         mem_binding_info->requirements.memoryTypeBits, bind_info.memoryBindIndex,
                                         FormatHandle(videoSession).c_str(), mem_state->alloc_info.memoryTypeIndex,
                                         FormatHandle(*mem_state).c_str(), i);
                    }

                    if (bind_info.memoryOffset >= mem_state->alloc_info.allocationSize) {
                        const LogObjectList objlist(videoSession, mem_state->Handle());
                        skip |= LogError(objlist, "VUID-VkBindVideoSessionMemoryInfoKHR-memoryOffset-07201",
                                         "vkBindVideoSessionMemoryKHR(): pBindSessionMemoryInfos[%u].memoryOffset (%" PRIuLEAST64
                                         ") must be less than the size (%" PRIuLEAST64 ") of %s.",
                                         i, bind_info.memoryOffset, mem_state->alloc_info.allocationSize,
                                         FormatHandle(*mem_state).c_str());
                    } else if (bind_info.memoryOffset + bind_info.memorySize > mem_state->alloc_info.allocationSize) {
                        const LogObjectList objlist(videoSession, mem_state->Handle());
                        skip |= LogError(
                            objlist, "VUID-VkBindVideoSessionMemoryInfoKHR-memorySize-07202",
                            "vkBindVideoSessionMemoryKHR(): memoryOffset (%" PRIuLEAST64 ") + memory size (%" PRIuLEAST64
                            ") specified in pBindSessionMemoryInfos[%u] must be less than or equal to the size (%" PRIuLEAST64
                            ") of %s.",
                            bind_info.memoryOffset, bind_info.memorySize, i, mem_state->alloc_info.allocationSize,
                            FormatHandle(*mem_state).c_str());
                    }
                }

                if (SafeModulo(bind_info.memoryOffset, mem_binding_info->requirements.alignment) != 0) {
                    skip |= LogError(videoSession, "VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07199",
                                     "vkBindVideoSessionMemoryKHR(): pBindSessionMemoryInfos[%u].memoryOffset is %" PRIuLEAST64
                                     " but must be an integer multiple of the alignment value %" PRIuLEAST64
                                     " for the memory binding index %u of %s.",
                                     i, bind_info.memoryOffset, mem_binding_info->requirements.alignment, bind_info.memoryBindIndex,
                                     FormatHandle(videoSession).c_str());
                }

                if (bind_info.memorySize != mem_binding_info->requirements.size) {
                    skip |= LogError(videoSession, "VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07200",
                                     "vkBindVideoSessionMemoryKHR(): pBindSessionMemoryInfos[%u].memorySize (%" PRIuLEAST64
                                     ") does not equal the required size (%" PRIuLEAST64 ") for the memory binding index %u of %s.",
                                     i, bind_info.memorySize, mem_binding_info->requirements.size, bind_info.memoryBindIndex,
                                     FormatHandle(videoSession).c_str());
                }

                if (mem_binding_info->bound) {
                    skip |= LogError(videoSession, "VUID-vkBindVideoSessionMemoryKHR-videoSession-07195",
                                     "vkBindVideoSessionMemoryKHR(): memory binding with index %u of %s is already "
                                     "bound but was specified in pBindSessionMemoryInfos[%u].memoryBindIndex.",
                                     bind_info.memoryBindIndex, FormatHandle(videoSession).c_str(), i);
                }
            } else {
                skip |= LogError(videoSession, "VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07197",
                                 "vkBindVideoSessionMemoryKHR(): %s does not have a memory binding corresponding "
                                 "to the memoryBindIndex specified in pBindSessionMemoryInfos[%u].",
                                 FormatHandle(videoSession).c_str(), i);
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateVideoSessionParametersKHR(VkDevice device,
                                                                const VkVideoSessionParametersCreateInfoKHR *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                VkVideoSessionParametersKHR *pVideoSessionParameters,
                                                                const ErrorObject &error_obj) const {
    bool skip = false;

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);

    std::shared_ptr<const vvl::VideoSessionParameters> template_state;
    if (pCreateInfo->videoSessionParametersTemplate != VK_NULL_HANDLE) {
        template_state = Get<vvl::VideoSessionParameters>(pCreateInfo->videoSessionParametersTemplate);
        if (template_state->vs_state->videoSession() != pCreateInfo->videoSession) {
            template_state = nullptr;
            const LogObjectList objlist(device, pCreateInfo->videoSessionParametersTemplate, pCreateInfo->videoSession);
            skip |= LogError(objlist, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-04855",
                             "vkCreateVideoSessionParametersKHR(): template %s was not created against the same %s.",
                             FormatHandle(pCreateInfo->videoSessionParametersTemplate).c_str(),
                             FormatHandle(pCreateInfo->videoSession).c_str());
        }
    }

    auto vs_state = Get<vvl::VideoSession>(pCreateInfo->videoSession);
    if (!vs_state) return false;

    const char *pnext_chain_msg = "does not contain a %s structure.";
    switch (vs_state->GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
            auto codec_info = vku::FindStructInPNextChain<VkVideoDecodeH264SessionParametersCreateInfoKHR>(pCreateInfo->pNext);
            if (codec_info) {
                skip |= ValidateDecodeH264ParametersAddInfo(
                    *vs_state, codec_info->pParametersAddInfo, device,
                    create_info_loc.pNext(Struct::VkVideoDecodeH264SessionParametersCreateInfoKHR, Field::pParametersAddInfo),
                    codec_info, template_state.get());
            } else {
                skip |=
                    LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07203", device,
                             create_info_loc.dot(Field::pNext), pnext_chain_msg, "VkVideoDecodeH264SessionParametersCreateInfoKHR");
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
            auto codec_info = vku::FindStructInPNextChain<VkVideoDecodeH265SessionParametersCreateInfoKHR>(pCreateInfo->pNext);
            if (codec_info) {
                skip |= ValidateDecodeH265ParametersAddInfo(
                    *vs_state, codec_info->pParametersAddInfo, device,
                    create_info_loc.pNext(Struct::VkVideoDecodeH265SessionParametersCreateInfoKHR, Field::pParametersAddInfo),
                    codec_info, template_state.get());
            } else {
                skip |=
                    LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07206", device,
                             create_info_loc.dot(Field::pNext), pnext_chain_msg, "VkVideoDecodeH265SessionParametersCreateInfoKHR");
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
            auto codec_info = vku::FindStructInPNextChain<VkVideoEncodeH264SessionParametersCreateInfoKHR>(pCreateInfo->pNext);
            if (codec_info) {
                skip |= ValidateEncodeH264ParametersAddInfo(
                    *vs_state, codec_info->pParametersAddInfo, device,
                    create_info_loc.pNext(Struct::VkVideoEncodeH264SessionParametersCreateInfoKHR, Field::pParametersAddInfo),
                    codec_info, template_state.get());
            } else {
                skip |=
                    LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07210", device,
                             create_info_loc.dot(Field::pNext), pnext_chain_msg, "VkVideoEncodeH264SessionParametersCreateInfoKHR");
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
            auto codec_info = vku::FindStructInPNextChain<VkVideoEncodeH265SessionParametersCreateInfoKHR>(pCreateInfo->pNext);
            if (codec_info) {
                skip |= ValidateEncodeH265ParametersAddInfo(
                    *vs_state, codec_info->pParametersAddInfo, device,
                    create_info_loc.pNext(Struct::VkVideoEncodeH265SessionParametersCreateInfoKHR, Field::pParametersAddInfo),
                    codec_info, template_state.get());
            } else {
                skip |=
                    LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07211", device,
                             create_info_loc.dot(Field::pNext), pnext_chain_msg, "VkVideoEncodeH265SessionParametersCreateInfoKHR");
            }
            break;
        }

        default:
            break;
    }

    if (vs_state->IsEncode()) {
        const uint32_t max_quality_levels = vs_state->profile->GetCapabilities().encode.maxQualityLevels;
        auto quality_level_info = vku::FindStructInPNextChain<VkVideoEncodeQualityLevelInfoKHR>(pCreateInfo->pNext);
        uint32_t encode_quality_level = 0;

        if (quality_level_info != nullptr) {
            encode_quality_level = quality_level_info->qualityLevel;
            if (encode_quality_level >= max_quality_levels) {
                skip |= LogError("VUID-VkVideoEncodeQualityLevelInfoKHR-qualityLevel-08311", pCreateInfo->videoSession,
                                 create_info_loc.pNext(Struct::VkVideoEncodeQualityLevelInfoKHR, Field::qualityLevel),
                                 "(%u) must be smaller than the maxQualityLevels (%u) supported by the video "
                                 "profile %s was created with.",
                                 encode_quality_level, max_quality_levels, FormatHandle(pCreateInfo->videoSession).c_str());
            }
        }

        if (template_state != nullptr && encode_quality_level != template_state->GetEncodeQualityLevel()) {
            const LogObjectList objlist(device, pCreateInfo->videoSessionParametersTemplate, pCreateInfo->videoSession);
            skip |= LogError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-08310)", objlist,
                             create_info_loc.pNext(Struct::VkVideoEncodeQualityLevelInfoKHR, Field::qualityLevel),
                             "(%u) does not match the video encode quality level (%u) template %s was created with.",
                             encode_quality_level, template_state->GetEncodeQualityLevel(),
                             FormatHandle(pCreateInfo->videoSessionParametersTemplate).c_str());
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                                const VkVideoSessionParametersUpdateInfoKHR *pUpdateInfo,
                                                                const ErrorObject &error_obj) const {
    bool skip = false;

    auto vsp_state = Get<vvl::VideoSessionParameters>(videoSessionParameters);
    if (!vsp_state) return false;

    const Location update_info_loc = error_obj.location.dot(Field::pUpdateInfo);

    auto vsp_data = vsp_state->Lock();

    if (pUpdateInfo->updateSequenceCount != vsp_data->update_sequence_counter + 1) {
        skip |= LogError(device, "VUID-vkUpdateVideoSessionParametersKHR-pUpdateInfo-07215",
                         "vkUpdateVideoSessionParametersKHR(): incorrect updateSequenceCount.");
    }

    switch (vsp_state->GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
            auto add_info = vku::FindStructInPNextChain<VkVideoDecodeH264SessionParametersAddInfoKHR>(pUpdateInfo->pNext);
            if (add_info) {
                skip |= ValidateDecodeH264ParametersAddInfo(
                    *vsp_state->vs_state, add_info, device,
                    update_info_loc.pNext(Struct::VkVideoDecodeH264SessionParametersAddInfoKHR));

                for (uint32_t i = 0; i < add_info->stdSPSCount; ++i) {
                    if (vsp_data.GetH264SPS(add_info->pStdSPSs[i].seq_parameter_set_id)) {
                        skip |=
                            LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07216",
                                     "vkUpdateVideoSessionParametersKHR(): H.264 SPS with key "
                                     "(SPS ID = %u) already exists in %s.",
                                     add_info->pStdSPSs[i].seq_parameter_set_id, FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdSPSCount + vsp_data->h264.sps.size() > vsp_data->h264.sps_capacity) {
                    skip |= LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07217",
                                     "vkUpdateVideoSessionParametersKHR(): number of H.264 SPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.264 "
                                     "SPS capacity (%u) the %s was created with.",
                                     add_info->stdSPSCount, vsp_data->h264.sps.size(), vsp_data->h264.sps_capacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }

                for (uint32_t i = 0; i < add_info->stdPPSCount; ++i) {
                    if (vsp_data.GetH264PPS(add_info->pStdPPSs[i].seq_parameter_set_id,
                                            add_info->pStdPPSs[i].pic_parameter_set_id)) {
                        skip |=
                            LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07218",
                                     "vkUpdateVideoSessionParametersKHR(): H.264 PPS with key "
                                     "(SPS ID = %u, PPS ID = %u) already exists in %s.",
                                     add_info->pStdPPSs[i].seq_parameter_set_id, add_info->pStdPPSs[i].pic_parameter_set_id,
                                     FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdPPSCount + vsp_data->h264.pps.size() > vsp_data->h264.pps_capacity) {
                    skip |= LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07219",
                                     "vkUpdateVideoSessionParametersKHR(): number of H.264 PPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.264 "
                                     "PPS capacity (%u) the %s was created with.",
                                     add_info->stdPPSCount, vsp_data->h264.pps.size(), vsp_data->h264.pps_capacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
            auto add_info = vku::FindStructInPNextChain<VkVideoDecodeH265SessionParametersAddInfoKHR>(pUpdateInfo->pNext);
            if (add_info) {
                skip |= ValidateDecodeH265ParametersAddInfo(
                    *vsp_state->vs_state, add_info, device,
                    update_info_loc.pNext(Struct::VkVideoDecodeH265SessionParametersAddInfoKHR));

                for (uint32_t i = 0; i < add_info->stdVPSCount; ++i) {
                    if (vsp_data.GetH265VPS(add_info->pStdVPSs[i].vps_video_parameter_set_id)) {
                        skip |= LogError(
                            videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07220",
                            "vkUpdateVideoSessionParametersKHR(): H.265 VPS with key "
                            "(VPS ID = %u) already exists in %s.",
                            add_info->pStdVPSs[i].vps_video_parameter_set_id, FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdVPSCount + vsp_data->h265.vps.size() > vsp_data->h265.vps_capacity) {
                    skip |= LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07221",
                                     "vkUpdateVideoSessionParametersKHR(): number of H.265 VPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.265 "
                                     "VPS capacity (%u) the %s was created with.",
                                     add_info->stdVPSCount, vsp_data->h265.vps.size(), vsp_data->h265.vps_capacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }

                for (uint32_t i = 0; i < add_info->stdSPSCount; ++i) {
                    if (vsp_data.GetH265SPS(add_info->pStdSPSs[i].sps_video_parameter_set_id,
                                            add_info->pStdSPSs[i].sps_seq_parameter_set_id)) {
                        skip |=
                            LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07222",
                                     "vkUpdateVideoSessionParametersKHR(): H.265 SPS with key "
                                     "(VPS ID = %u, SPS ID = %u) already exists in %s.",
                                     add_info->pStdSPSs[i].sps_video_parameter_set_id,
                                     add_info->pStdSPSs[i].sps_seq_parameter_set_id, FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdSPSCount + vsp_data->h265.sps.size() > vsp_data->h265.sps_capacity) {
                    skip |= LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07223",
                                     "vkUpdateVideoSessionParametersKHR(): number of H.265 SPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.265 "
                                     "SPS capacity (%u) the %s was created with.",
                                     add_info->stdSPSCount, vsp_data->h265.sps.size(), vsp_data->h265.sps_capacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }

                for (uint32_t i = 0; i < add_info->stdPPSCount; ++i) {
                    if (vsp_data.GetH265PPS(add_info->pStdPPSs[i].sps_video_parameter_set_id,
                                            add_info->pStdPPSs[i].pps_seq_parameter_set_id,
                                            add_info->pStdPPSs[i].pps_pic_parameter_set_id)) {
                        skip |= LogError(
                            videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07224",
                            "vkUpdateVideoSessionParametersKHR(): H.265 PPS with key "
                            "(VPS ID = %u, SPS ID = %u, PPS ID = %u) already exists in %s.",
                            add_info->pStdPPSs[i].sps_video_parameter_set_id, add_info->pStdPPSs[i].pps_seq_parameter_set_id,
                            add_info->pStdPPSs[i].pps_pic_parameter_set_id, FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdPPSCount + vsp_data->h265.pps.size() > vsp_data->h265.pps_capacity) {
                    skip |= LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07225",
                                     "vkUpdateVideoSessionParametersKHR(): number of H.265 PPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.265 "
                                     "PPS capacity (%u) the %s was created with.",
                                     add_info->stdPPSCount, vsp_data->h265.pps.size(), vsp_data->h265.pps_capacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
            auto add_info = vku::FindStructInPNextChain<VkVideoEncodeH264SessionParametersAddInfoKHR>(pUpdateInfo->pNext);
            if (add_info) {
                skip |= ValidateEncodeH264ParametersAddInfo(
                    *vsp_state->vs_state, add_info, device,
                    update_info_loc.pNext(Struct::VkVideoEncodeH264SessionParametersAddInfoKHR));

                for (uint32_t i = 0; i < add_info->stdSPSCount; ++i) {
                    if (vsp_data.GetH264SPS(add_info->pStdSPSs[i].seq_parameter_set_id)) {
                        skip |=
                            LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07226",
                                     "vkUpdateVideoSessionParametersKHR(): H.264 SPS with key "
                                     "(SPS ID = %u) already exists in %s.",
                                     add_info->pStdSPSs[i].seq_parameter_set_id, FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdSPSCount + vsp_data->h264.sps.size() > vsp_data->h264.sps_capacity) {
                    skip |= LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06441",
                                     "vkUpdateVideoSessionParametersKHR(): number of H.264 SPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.264 "
                                     "SPS capacity (%u) the %s was created with.",
                                     add_info->stdSPSCount, vsp_data->h264.sps.size(), vsp_data->h264.sps_capacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }

                for (uint32_t i = 0; i < add_info->stdPPSCount; ++i) {
                    if (vsp_data.GetH264PPS(add_info->pStdPPSs[i].seq_parameter_set_id,
                                            add_info->pStdPPSs[i].pic_parameter_set_id)) {
                        skip |=
                            LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07227",
                                     "vkUpdateVideoSessionParametersKHR(): H.264 PPS with key "
                                     "(SPS ID = %u, PPS ID = %u) already exists in %s.",
                                     add_info->pStdPPSs[i].seq_parameter_set_id, add_info->pStdPPSs[i].pic_parameter_set_id,
                                     FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdPPSCount + vsp_data->h264.pps.size() > vsp_data->h264.pps_capacity) {
                    skip |= LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06442",
                                     "vkUpdateVideoSessionParametersKHR(): number of H.264 PPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.264 "
                                     "PPS capacity (%u) the %s was created with.",
                                     add_info->stdPPSCount, vsp_data->h264.pps.size(), vsp_data->h264.pps_capacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
            auto add_info = vku::FindStructInPNextChain<VkVideoEncodeH265SessionParametersAddInfoKHR>(pUpdateInfo->pNext);
            if (add_info) {
                skip |= ValidateEncodeH265ParametersAddInfo(
                    *vsp_state->vs_state, add_info, device,
                    update_info_loc.pNext(Struct::VkVideoEncodeH265SessionParametersAddInfoKHR));

                for (uint32_t i = 0; i < add_info->stdVPSCount; ++i) {
                    if (vsp_data.GetH265VPS(add_info->pStdVPSs[i].vps_video_parameter_set_id)) {
                        skip |= LogError(
                            videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07228",
                            "vkUpdateVideoSessionParametersKHR(): H.265 VPS with key "
                            "(VPS ID = %u) already exists in %s.",
                            add_info->pStdVPSs[i].vps_video_parameter_set_id, FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdVPSCount + vsp_data->h265.vps.size() > vsp_data->h265.vps_capacity) {
                    skip |= LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06443",
                                     "vkUpdateVideoSessionParametersKHR(): number of H.265 VPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.265 "
                                     "VPS capacity (%u) the %s was created with.",
                                     add_info->stdVPSCount, vsp_data->h265.vps.size(), vsp_data->h265.vps_capacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }

                for (uint32_t i = 0; i < add_info->stdSPSCount; ++i) {
                    if (vsp_data.GetH265SPS(add_info->pStdSPSs[i].sps_video_parameter_set_id,
                                            add_info->pStdSPSs[i].sps_seq_parameter_set_id)) {
                        skip |=
                            LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07229",
                                     "vkUpdateVideoSessionParametersKHR(): H.265 SPS with key "
                                     "(VPS ID = %u, SPS ID = %u) already exists in %s.",
                                     add_info->pStdSPSs[i].sps_video_parameter_set_id,
                                     add_info->pStdSPSs[i].sps_seq_parameter_set_id, FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdSPSCount + vsp_data->h265.sps.size() > vsp_data->h265.sps_capacity) {
                    skip |= LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06444",
                                     "vkUpdateVideoSessionParametersKHR(): number of H.265 SPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.265 "
                                     "SPS capacity (%u) the %s was created with.",
                                     add_info->stdSPSCount, vsp_data->h265.sps.size(), vsp_data->h265.sps_capacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }

                for (uint32_t i = 0; i < add_info->stdPPSCount; ++i) {
                    if (vsp_data.GetH265PPS(add_info->pStdPPSs[i].sps_video_parameter_set_id,
                                            add_info->pStdPPSs[i].pps_seq_parameter_set_id,
                                            add_info->pStdPPSs[i].pps_pic_parameter_set_id)) {
                        skip |= LogError(
                            videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07230",
                            "vkUpdateVideoSessionParametersKHR(): H.265 PPS with key "
                            "(VPS ID = %u, SPS ID = %u, PPS ID = %u) already exists in %s.",
                            add_info->pStdPPSs[i].sps_video_parameter_set_id, add_info->pStdPPSs[i].pps_seq_parameter_set_id,
                            add_info->pStdPPSs[i].pps_pic_parameter_set_id, FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdPPSCount + vsp_data->h265.pps.size() > vsp_data->h265.pps_capacity) {
                    skip |= LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-06445",
                                     "vkUpdateVideoSessionParametersKHR(): number of H.265 PPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.265 "
                                     "PPS capacity (%u) the %s was created with.",
                                     add_info->stdPPSCount, vsp_data->h265.pps.size(), vsp_data->h265.pps_capacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }
            }
            break;
        }

        default:
            break;
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyVideoSessionParametersKHR(VkDevice device,
                                                                 VkVideoSessionParametersKHR videoSessionParameters,
                                                                 const VkAllocationCallbacks *pAllocator,
                                                                 const ErrorObject &error_obj) const {
    auto video_session_parameters_state = Get<vvl::VideoSessionParameters>(videoSessionParameters);
    bool skip = false;
    if (video_session_parameters_state) {
        skip |= ValidateObjectNotInUse(video_session_parameters_state.get(), error_obj.location,
                                       "VUID-vkDestroyVideoSessionParametersKHR-videoSessionParameters-07212");
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetEncodedVideoSessionParametersKHR(
    VkDevice device, const VkVideoEncodeSessionParametersGetInfoKHR *pVideoSessionParametersInfo,
    VkVideoEncodeSessionParametersFeedbackInfoKHR *pFeedbackInfo, size_t *pDataSize, void *pData,
    const ErrorObject &error_obj) const {
    bool skip = false;

    const auto vsp_state = Get<vvl::VideoSessionParameters>(pVideoSessionParametersInfo->videoSessionParameters);
    if (!vsp_state) return false;

    const Location params_info_loc = error_obj.location.dot(Field::pVideoSessionParametersInfo);

    auto vsp_data = vsp_state->Lock();

    const char *pnext_msg = "chain does not contain a %s structure.";

    if (vsp_state->IsEncode()) {
        switch (vsp_state->GetCodecOp()) {
            case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
                auto h264_info =
                    vku::FindStructInPNextChain<VkVideoEncodeH264SessionParametersGetInfoKHR>(pVideoSessionParametersInfo->pNext);
                if (h264_info) {
                    const Location h264_info_loc = params_info_loc.pNext(Struct::VkVideoEncodeH264SessionParametersGetInfoKHR);

                    if (!h264_info->writeStdSPS && !h264_info->writeStdPPS) {
                        skip |= LogError("VUID-VkVideoEncodeH264SessionParametersGetInfoKHR-writeStdSPS-08279",
                                         pVideoSessionParametersInfo->videoSessionParameters, h264_info_loc,
                                         "must have at least one of writeStdSPS and writeStdPPS set to VK_TRUE.");
                    }

                    if (h264_info->writeStdSPS && vsp_data.GetH264SPS(h264_info->stdSPSId) == nullptr) {
                        skip |= LogError(pVideoSessionParametersInfo->videoSessionParameters,
                                         "VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08263",
                                         "vkGetEncodedVideoSessionParametersKHR(): %s does not contain an H.264 SPS "
                                         "matching the stdSPSId (%u) specified in %s.",
                                         FormatHandle(*vsp_state).c_str(), h264_info->stdSPSId, h264_info_loc.Fields().c_str());
                    }

                    if (h264_info->writeStdPPS && vsp_data.GetH264PPS(h264_info->stdSPSId, h264_info->stdPPSId) == nullptr) {
                        skip |= LogError(pVideoSessionParametersInfo->videoSessionParameters,
                                         "VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08264",
                                         "vkGetEncodedVideoSessionParametersKHR(): %s does not contain an H.264 PPS "
                                         "matching the stdSPSId (%u) and stdPPSId (%u) specified in %s.",
                                         FormatHandle(*vsp_state).c_str(), h264_info->stdSPSId, h264_info->stdPPSId,
                                         h264_info_loc.Fields().c_str());
                    }
                } else {
                    skip |= LogError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08262",
                                     pVideoSessionParametersInfo->videoSessionParameters, params_info_loc.dot(Field::pNext),
                                     pnext_msg, "VkVideoEncodeH264SessionParametersGetInfoKHR");
                }
                break;
            }

            case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
                auto h265_info =
                    vku::FindStructInPNextChain<VkVideoEncodeH265SessionParametersGetInfoKHR>(pVideoSessionParametersInfo->pNext);
                if (h265_info) {
                    const Location h265_info_loc = params_info_loc.pNext(Struct::VkVideoEncodeH264SessionParametersGetInfoKHR);

                    if (!h265_info->writeStdVPS && !h265_info->writeStdSPS && !h265_info->writeStdPPS) {
                        skip |= LogError("VUID-VkVideoEncodeH265SessionParametersGetInfoKHR-writeStdVPS-08290",
                                         pVideoSessionParametersInfo->videoSessionParameters, h265_info_loc,
                                         "must have at least one of writeStdVPS, writeStdSPS, and writeStdPPS set to VK_TRUE.");
                    }

                    if (h265_info->writeStdVPS && vsp_data.GetH265VPS(h265_info->stdVPSId) == nullptr) {
                        skip |= LogError(pVideoSessionParametersInfo->videoSessionParameters,
                                         "VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08266",
                                         "vkGetEncodedVideoSessionParametersKHR(): %s does not contain an H.265 VPS "
                                         "matching the stdVPSId (%u) specified in %s.",
                                         FormatHandle(*vsp_state).c_str(), h265_info->stdVPSId, h265_info_loc.Fields().c_str());
                    }

                    if (h265_info->writeStdSPS && vsp_data.GetH265SPS(h265_info->stdVPSId, h265_info->stdSPSId) == nullptr) {
                        skip |= LogError(pVideoSessionParametersInfo->videoSessionParameters,
                                         "VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08267",
                                         "vkGetEncodedVideoSessionParametersKHR(): %s does not contain an H.265 SPS "
                                         "matching the stdVPSId (%u) and stdSPSId (%u) specified in %s.",
                                         FormatHandle(*vsp_state).c_str(), h265_info->stdVPSId, h265_info->stdSPSId,
                                         h265_info_loc.Fields().c_str());
                    }

                    if (h265_info->writeStdPPS &&
                        vsp_data.GetH265PPS(h265_info->stdVPSId, h265_info->stdSPSId, h265_info->stdPPSId) == nullptr) {
                        skip |= LogError(pVideoSessionParametersInfo->videoSessionParameters,
                                         "VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08268",
                                         "vkGetEncodedVideoSessionParametersKHR(): %s does not contain an H.265 PPS "
                                         "matching the stdVPSId(%u), stdSPSId (%u), and stdPPSId (%u) specified in %s.",
                                         FormatHandle(*vsp_state).c_str(), h265_info->stdVPSId, h265_info->stdSPSId,
                                         h265_info->stdPPSId, h265_info_loc.Fields().c_str());
                    }
                } else {
                    skip |= LogError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08265",
                                     pVideoSessionParametersInfo->videoSessionParameters, params_info_loc.dot(Field::pNext),
                                     pnext_msg, "VkVideoEncodeH265SessionParametersGetInfoKHR");
                }
                break;
            }

            default:
                assert(false);
                break;
        }
    } else {
        skip |= LogError("VUID-vkGetEncodedVideoSessionParametersKHR-pVideoSessionParametersInfo-08359",
                         pVideoSessionParametersInfo->videoSessionParameters, params_info_loc.dot(Field::videoSessionParameters),
                         "(%s) was not created with an encode operation.",
                         FormatHandle(pVideoSessionParametersInfo->videoSessionParameters).c_str());
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR *pBeginInfo,
                                                       const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) return false;

    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (cb_state->activeQueries.size() > 0) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBeginVideoCodingKHR-None-07232",
                         "vkCmdBeginVideoCodingKHR(): %s has active queries.", FormatHandle(commandBuffer).c_str());
    }

    auto vs_state = Get<vvl::VideoSession>(pBeginInfo->videoSession);
    if (!vs_state) return false;

    const Location begin_info_loc = error_obj.location.dot(Field::pBeginInfo);

    auto vsp_state = Get<vvl::VideoSessionParameters>(pBeginInfo->videoSessionParameters);

    auto qf_ext_props = queue_family_ext_props[cb_state->command_pool->queueFamilyIndex];

    if ((qf_ext_props.video_props.videoCodecOperations & vs_state->GetCodecOp()) == 0) {
        const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession, cb_state->command_pool->Handle());
        skip |= LogError(objlist, "VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07231",
                         "vkCmdBeginVideoCodingKHR(): %s does not support video codec operation %s "
                         "that %s specified in pBeginInfo->videoSession was created with.",
                         FormatHandle(cb_state->command_pool->Handle()).c_str(),
                         string_VkVideoCodecOperationFlagBitsKHR(vs_state->GetCodecOp()),
                         FormatHandle(pBeginInfo->videoSession).c_str());
    }

    if (vs_state->GetUnboundMemoryBindingCount() > 0) {
        const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession);
        skip |= LogError(objlist, "VUID-VkVideoBeginCodingInfoKHR-videoSession-07237",
                         "vkCmdBeginVideoCodingKHR(): %s has %u unbound memory binding indices.",
                         FormatHandle(pBeginInfo->videoSession).c_str(), vs_state->GetUnboundMemoryBindingCount());
    }

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state->unprotected == true) &&
        ((vs_state->create_info.flags & VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR) != 0)) {
        const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession);
        skip |= LogError(objlist, "VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07233",
                         "vkCmdBeginVideoCodingKHR(): %s is unprotected while %s was created with "
                         "VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR.",
                         FormatHandle(commandBuffer).c_str(), FormatHandle(pBeginInfo->videoSession).c_str());
    }

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state->unprotected == false) &&
        ((vs_state->create_info.flags & VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR) == 0)) {
        const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession);
        skip |= LogError(objlist, "VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07234",
                         "vkCmdBeginVideoCodingKHR(): %s is protected while %s was created without "
                         "VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR.",
                         FormatHandle(commandBuffer).c_str(), FormatHandle(pBeginInfo->videoSession).c_str());
    }

    if (pBeginInfo->pReferenceSlots) {
        vvl::VideoPictureResources unique_resources{};
        bool resources_unique = true;
        bool has_separate_images = false;
        const vvl::Image *last_dpb_image = nullptr;
        char where[64];

        for (uint32_t i = 0; i < pBeginInfo->referenceSlotCount; ++i) {
            const auto &slot = pBeginInfo->pReferenceSlots[i];

            if (slot.slotIndex >= 0 && (uint32_t)slot.slotIndex >= vs_state->create_info.maxDpbSlots) {
                const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession);
                skip |= LogError(objlist, "VUID-VkVideoBeginCodingInfoKHR-slotIndex-04856",
                                 "vkCmdBeginVideoCodingKHR(): pBeginInfo->pReferenceSlots[%u].slotIndex (%d) "
                                 "is greater than the maxDpbSlots %s was created with.",
                                 i, slot.slotIndex, FormatHandle(pBeginInfo->videoSession).c_str());
            }

            if (slot.pPictureResource != nullptr) {
                auto reference_resource = vvl::VideoPictureResource(this, *slot.pPictureResource);
                skip |= ValidateVideoPictureResource(reference_resource, commandBuffer, *vs_state,
                                                     begin_info_loc.dot(Field::pReferenceSlots, i).dot(Field::pPictureResource),
                                                     "VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07242",
                                                     "VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07243");
                if (reference_resource) {
                    if (!unique_resources.emplace(reference_resource).second) {
                        resources_unique = false;
                    }

                    if (last_dpb_image != nullptr && last_dpb_image != reference_resource.image_state.get()) {
                        has_separate_images = true;
                    }

                    snprintf(where, sizeof(where), " Image referenced in pBeginInfo->pReferenceSlots[%u].", i);
                    skip |= ValidateProtectedImage(*cb_state, *reference_resource.image_state, error_obj.location,
                                                   "VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07235", where);
                    skip |= ValidateUnprotectedImage(*cb_state, *reference_resource.image_state, error_obj.location,
                                                     "VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07236", where);

                    if (!IsImageCompatibleWithVideoProfile(*reference_resource.image_state, vs_state->profile)) {
                        const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession,
                                                    reference_resource.image_view_state->image_view(),
                                                    reference_resource.image_state->image());
                        skip |= LogError(objlist, "VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07240",
                                         "vkCmdBeginVideoCodingKHR(): pBeginInfo->pReferenceSlots[%u]."
                                         "pPictureResource->imageViewBinding (%s created from %s) is not "
                                         "compatible with the video profile %s was created with.",
                                         i, FormatHandle(reference_resource.image_view_state->Handle()).c_str(),
                                         FormatHandle(reference_resource.image_state->Handle()).c_str(),
                                         FormatHandle(pBeginInfo->videoSession).c_str());
                    }

                    if (reference_resource.image_view_state->create_info.format != vs_state->create_info.referencePictureFormat) {
                        const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession,
                                                    reference_resource.image_view_state->image_view(),
                                                    reference_resource.image_state->image());
                        skip |= LogError(objlist, "VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07241",
                                         "vkCmdBeginVideoCodingKHR(): pBeginInfo->pReferenceSlots[%u]."
                                         "pPictureResource->imageViewBinding (%s created from %s) format (%s) "
                                         "does not match the referencePictureFormat (%s) %s was created with.",
                                         i, FormatHandle(reference_resource.image_view_state->Handle()).c_str(),
                                         FormatHandle(reference_resource.image_state->Handle()).c_str(),
                                         string_VkFormat(reference_resource.image_view_state->create_info.format),
                                         string_VkFormat(vs_state->create_info.referencePictureFormat),
                                         FormatHandle(pBeginInfo->videoSession).c_str());
                    }

                    auto supported_usage = reference_resource.image_view_state->inherited_usage;

                    if (vs_state->IsDecode() && (supported_usage & VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR) == 0) {
                        const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession,
                                                    reference_resource.image_view_state->image_view(),
                                                    reference_resource.image_state->image());
                        skip |= LogError(objlist, "VUID-VkVideoBeginCodingInfoKHR-slotIndex-07245",
                                         "vkCmdBeginVideoCodingKHR(): pBeginInfo->pReferenceSlots[%u]."
                                         "pPictureResource->imageViewBinding (%s created from %s) was not created "
                                         "with VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR thus it cannot be used as "
                                         "a reference picture with %s that was created with a decode operation.",
                                         i, FormatHandle(reference_resource.image_view_state->Handle()).c_str(),
                                         FormatHandle(reference_resource.image_state->Handle()).c_str(),
                                         FormatHandle(pBeginInfo->videoSession).c_str());
                    }

                    if (vs_state->IsEncode() && (supported_usage & VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR) == 0) {
                        const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession,
                                                    reference_resource.image_view_state->image_view(),
                                                    reference_resource.image_state->image());
                        skip |= LogError(objlist, "VUID-VkVideoBeginCodingInfoKHR-slotIndex-07246",
                                         "vkCmdBeginVideoCodingKHR(): pBeginInfo->pReferenceSlots[%u]."
                                         "pPictureResource->imageViewBinding (%s created from %s) was not created "
                                         "with VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR thus it cannot be used as "
                                         "a reference picture with %s that was created with an encode operation.",
                                         i, FormatHandle(reference_resource.image_view_state->Handle()).c_str(),
                                         FormatHandle(reference_resource.image_state->Handle()).c_str(),
                                         FormatHandle(pBeginInfo->videoSession).c_str());
                    }

                    last_dpb_image = reference_resource.image_state.get();
                }
            }
        }

        if (!resources_unique) {
            const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession);
            skip |= LogError(objlist, "VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07238",
                             "vkCmdBeginVideoCodingKHR(): more than one element of pBeginInfo->pReferenceSlots "
                             "refers to the same video picture resource.");
        }

        auto supported_cap_flags = vs_state->profile->GetCapabilities().base.flags;
        if ((supported_cap_flags & VK_VIDEO_CAPABILITY_SEPARATE_REFERENCE_IMAGES_BIT_KHR) == 0 && has_separate_images) {
            const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession);
            skip |= LogError(objlist, "VUID-VkVideoBeginCodingInfoKHR-flags-07244",
                             "vkCmdBeginVideoCodingKHR(): not all elements of pBeginInfo->pReferenceSlots refer "
                             "to the same image and the video profile %s was created with does not support "
                             "VK_VIDEO_CAPABILITY_SEPARATE_REFERENCE_IMAGES_BIT_KHR.",
                             FormatHandle(pBeginInfo->videoSession).c_str());
        }
    }

    if (vsp_state && vsp_state->vs_state->videoSession() != vs_state->videoSession()) {
        const LogObjectList objlist(commandBuffer, pBeginInfo->videoSessionParameters, pBeginInfo->videoSession);
        skip |= LogError(objlist, "VUID-VkVideoBeginCodingInfoKHR-videoSessionParameters-04857",
                         "vkCmdBeginVideoCodingKHR(): %s was not created for %s.",
                         FormatHandle(pBeginInfo->videoSessionParameters).c_str(), FormatHandle(pBeginInfo->videoSession).c_str());
    }

    const char *codec_op_requires_params_vuid = nullptr;
    switch (vs_state->GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
            codec_op_requires_params_vuid = "VUID-VkVideoBeginCodingInfoKHR-videoSession-07247";
            break;

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
            codec_op_requires_params_vuid = "VUID-VkVideoBeginCodingInfoKHR-videoSession-07248";
            break;

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
            codec_op_requires_params_vuid = "VUID-VkVideoBeginCodingInfoKHR-videoSession-07249";
            break;

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
            codec_op_requires_params_vuid = "VUID-VkVideoBeginCodingInfoKHR-videoSession-07250";
            break;

        default:
            break;
    }

    if ((codec_op_requires_params_vuid != nullptr) && (vsp_state == nullptr)) {
        const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession);
        skip |= LogError(objlist, codec_op_requires_params_vuid,
                         "vkCmdBeginVideoCodingKHR(): %s was created with %s but no video session parameters object was "
                         "specified in pBeginInfo->videoSessionParameters.",
                         FormatHandle(pBeginInfo->videoSession).c_str(),
                         string_VkVideoCodecOperationFlagBitsKHR(vs_state->GetCodecOp()));
    }

    if (vs_state->IsEncode()) {
        const auto rc_info = vku::FindStructInPNextChain<VkVideoEncodeRateControlInfoKHR>(pBeginInfo->pNext);
        if (rc_info != nullptr) {
            skip |= ValidateVideoEncodeRateControlInfo(*rc_info, pBeginInfo->pNext, commandBuffer, *vs_state, begin_info_loc);

            if (rc_info->rateControlMode != VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DEFAULT_KHR &&
                rc_info->rateControlMode != VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR) {
                switch (vs_state->GetCodecOp()) {
                    case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR: {
                        auto gop_info_h264 =
                            vku::FindStructInPNextChain<VkVideoEncodeH264GopRemainingFrameInfoKHR>(pBeginInfo->pNext);
                        if (vs_state->profile->GetCapabilities().encode_h264.requiresGopRemainingFrames &&
                            (gop_info_h264 == nullptr || gop_info_h264->useGopRemainingFrames == VK_FALSE)) {
                            const char *why = gop_info_h264 == nullptr
                                                  ? "there is no VkVideoEncodeH264GopRemainingFrameInfoKHR structure"
                                                  : "VkVideoEncodeH264GopRemainingFrameInfoKHR::useGopRemainingFrames is VK_FALSE";
                            const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession);
                            skip |= LogError(objlist, "VUID-vkCmdBeginVideoCodingKHR-pBeginInfo-08255",
                                             "vkCmdBeginVideoCodingKHR(): requiresGopRemainingFrames is VK_TRUE for "
                                             "the H.264 encode profile %s was created with, but %s in the pNext "
                                             "chain of pBeginInfo.",
                                             FormatHandle(pBeginInfo->videoSession).c_str(), why);
                        }
                        break;
                    }

                    case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR: {
                        auto gop_info_h265 =
                            vku::FindStructInPNextChain<VkVideoEncodeH265GopRemainingFrameInfoKHR>(pBeginInfo->pNext);
                        if (vs_state->profile->GetCapabilities().encode_h265.requiresGopRemainingFrames &&
                            (gop_info_h265 == nullptr || gop_info_h265->useGopRemainingFrames == VK_FALSE)) {
                            const char *why = gop_info_h265 == nullptr
                                                  ? "there is no VkVideoEncodeH265GopRemainingFrameInfoKHR structure"
                                                  : "VkVideoEncodeH265GopRemainingFrameInfoKHR::useGopRemainingFrames is VK_FALSE";
                            const LogObjectList objlist(commandBuffer, pBeginInfo->videoSession);
                            skip |= LogError(objlist, "VUID-vkCmdBeginVideoCodingKHR-pBeginInfo-08256",
                                             "vkCmdBeginVideoCodingKHR(): requiresGopRemainingFrames is VK_TRUE for "
                                             "the H.265 encode profile %s was created with, but %s in the pNext "
                                             "chain of pBeginInfo.",
                                             FormatHandle(pBeginInfo->videoSession).c_str(), why);
                        }
                        break;
                    }

                    default:
                        break;
                }
            }
        }
    }

    return skip;
}

void CoreChecks::PreCallRecordCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR *pBeginInfo,
                                                     const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) return;

    auto vs_state = Get<vvl::VideoSession>(pBeginInfo->videoSession);
    if (!vs_state) return;

    if (pBeginInfo->referenceSlotCount > 0) {
        std::vector<vvl::VideoReferenceSlot> expected_slots{};
        expected_slots.reserve(pBeginInfo->referenceSlotCount);

        for (uint32_t i = 0; i < pBeginInfo->referenceSlotCount; ++i) {
            if (pBeginInfo->pReferenceSlots[i].slotIndex >= 0) {
                expected_slots.emplace_back(this, *vs_state->profile, pBeginInfo->pReferenceSlots[i], false);
            }
        }

        // Enqueue submission time validation of DPB slots
        cb_state->video_session_updates[vs_state->videoSession()].emplace_back(
            [expected_slots](const ValidationStateTracker *dev_data, const vvl::VideoSession *vs_state,
                             vvl::VideoSessionDeviceState &dev_state, bool do_validate) {
                if (!do_validate) return false;
                bool skip = false;
                for (const auto &slot : expected_slots) {
                    if (!dev_state.IsSlotActive(slot.index)) {
                        skip |= dev_data->LogError(vs_state->Handle(), "VUID-vkCmdBeginVideoCodingKHR-slotIndex-07239",
                                                   "DPB slot index %d is not active in %s.", slot.index,
                                                   dev_data->FormatHandle(*vs_state).c_str());
                    } else if (slot.resource && !dev_state.IsSlotPicture(slot.index, slot.resource)) {
                        skip |= dev_data->LogError(vs_state->Handle(), "VUID-vkCmdBeginVideoCodingKHR-pPictureResource-07265",
                                                   "DPB slot index %d of %s is not currently associated with the specified "
                                                   "video picture resource: %s, layer %u, offset (%u,%u), extent (%u,%u).",
                                                   slot.index, dev_data->FormatHandle(*vs_state).c_str(),
                                                   dev_data->FormatHandle(slot.resource.image_state->Handle()).c_str(),
                                                   slot.resource.range.baseArrayLayer, slot.resource.coded_offset.x,
                                                   slot.resource.coded_offset.y, slot.resource.coded_extent.width,
                                                   slot.resource.coded_extent.height);
                    }
                }
                return skip;
            });
    }

    if (vs_state->IsEncode()) {
        safe_VkVideoBeginCodingInfoKHR begin_info(pBeginInfo);

        // Enqueue submission time validation of rate control state
        cb_state->video_session_updates[vs_state->videoSession()].emplace_back(
            [begin_info](const ValidationStateTracker *dev_data, const vvl::VideoSession *vs_state,
                         vvl::VideoSessionDeviceState &dev_state, bool do_validate) {
                if (!do_validate) return false;
                return dev_state.ValidateRateControlState(dev_data, vs_state, begin_info);
            });
    }
}

bool CoreChecks::PreCallValidateCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR *pEndCodingInfo,
                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) return false;

    skip |= ValidateCmd(*cb_state, error_obj.location);

    if (cb_state->activeQueries.size() > 0) {
        skip |= LogError(commandBuffer, "VUID-vkCmdEndVideoCodingKHR-None-07251",
                         "vkCmdEndVideoCodingKHR(): %s has active queries.", FormatHandle(commandBuffer).c_str());
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                         const VkVideoCodingControlInfoKHR *pCodingControlInfo,
                                                         const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) return false;

    skip |= ValidateCmd(*cb_state, error_obj.location);

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return false;

    const Location control_info_loc = error_obj.location.dot(Field::pCodingControlInfo);

    const auto &profile_caps = vs_state->profile->GetCapabilities();

    const char *flags_pnext_msg = "has %s set but missing %s from the pNext chain of pCodingControlInfo.";
    const char *flags_require_encode_msg = "has %s set but %s is not a video encode session.";

    if (pCodingControlInfo->flags & VK_VIDEO_CODING_CONTROL_ENCODE_RATE_CONTROL_BIT_KHR) {
        if (vs_state->IsEncode()) {
            const auto rc_info = vku::FindStructInPNextChain<VkVideoEncodeRateControlInfoKHR>(pCodingControlInfo->pNext);
            if (rc_info != nullptr) {
                skip |= ValidateVideoEncodeRateControlInfo(*rc_info, pCodingControlInfo->pNext, commandBuffer, *vs_state,
                                                           control_info_loc);
            } else {
                skip |= LogError("VUID-VkVideoCodingControlInfoKHR-flags-07018", commandBuffer, control_info_loc.dot(Field::flags),
                                 flags_pnext_msg, "VK_VIDEO_CODING_CONTROL_ENCODE_RATE_CONTROL_BIT_KHR",
                                 "VkVideoEncodeRateControlInfoKHR");
            }
        } else {
            const LogObjectList objlist(commandBuffer, vs_state->videoSession());
            skip |= LogError("VUID-vkCmdControlVideoCodingKHR-pCodingControlInfo-08243", objlist,
                             control_info_loc.dot(Field::flags), flags_require_encode_msg,
                             "VK_VIDEO_CODING_CONTROL_ENCODE_RATE_CONTROL_BIT_KHR", FormatHandle(*vs_state).c_str());
        }
    }

    if (pCodingControlInfo->flags & VK_VIDEO_CODING_CONTROL_ENCODE_QUALITY_LEVEL_BIT_KHR) {
        if (vs_state->IsEncode()) {
            const auto quality_level_info =
                vku::FindStructInPNextChain<VkVideoEncodeQualityLevelInfoKHR>(pCodingControlInfo->pNext);
            if (quality_level_info != nullptr) {
                if (quality_level_info->qualityLevel >= profile_caps.encode.maxQualityLevels) {
                    const LogObjectList objlist(commandBuffer, vs_state->videoSession());
                    skip |= LogError("VUID-VkVideoEncodeQualityLevelInfoKHR-qualityLevel-08311", objlist,
                                     control_info_loc.pNext(Struct::VkVideoEncodeQualityLevelInfoKHR, Field::qualityLevel),
                                     "(%u) must be smaller than the maxQualityLevels (%u) supported by the video "
                                     "profile %s was created with.",
                                     quality_level_info->qualityLevel, profile_caps.encode.maxQualityLevels,
                                     FormatHandle(*vs_state).c_str());
                }
            } else {
                skip |= LogError("VUID-VkVideoCodingControlInfoKHR-flags-08349", commandBuffer, control_info_loc.dot(Field::flags),
                                 flags_pnext_msg, "VK_VIDEO_CODING_CONTROL_ENCODE_QUALITY_LEVEL_BIT_KHR",
                                 "VkVideoEncodeQualityLevelInfoKHR");
            }
        } else {
            const LogObjectList objlist(commandBuffer, vs_state->videoSession());
            skip |= LogError("VUID-vkCmdControlVideoCodingKHR-pCodingControlInfo-08243", objlist,
                             control_info_loc.dot(Field::flags), flags_require_encode_msg,
                             "VK_VIDEO_CODING_CONTROL_ENCODE_QUALITY_LEVEL_BIT_KHR", FormatHandle(*vs_state).c_str());
        }
    }

    return skip;
}

void CoreChecks::PreCallRecordCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                       const VkVideoCodingControlInfoKHR *pCodingControlInfo,
                                                       const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) return;

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return;

    if ((pCodingControlInfo->flags & VK_VIDEO_CODING_CONTROL_RESET_BIT_KHR) == 0) {
        EnqueueVerifyVideoSessionInitialized(*cb_state, *vs_state, "VUID-vkCmdControlVideoCodingKHR-flags-07017");
    }
}

bool CoreChecks::PreCallValidateCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR *pDecodeInfo,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) return false;

    skip |= ValidateCmd(*cb_state, error_obj.location);

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return false;

    const Location decode_info_loc = error_obj.location.dot(Field::pDecodeInfo);

    if (!vs_state->IsDecode()) {
        const LogObjectList objlist(commandBuffer, vs_state->videoSession());
        skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-None-08249",
                         "vkCmdDecodeVideoKHR(): the video codec operation (%s) the bound video session %s "
                         "was created with is not a decode operation.",
                         string_VkVideoCodecOperationFlagBitsKHR(vs_state->GetCodecOp()), FormatHandle(*vs_state).c_str());
        return skip;
    }

    const auto &bound_resources = cb_state->bound_video_picture_resources;

    bool hit_error = false;

    const auto &profile_caps = vs_state->profile->GetCapabilities();

    auto buffer_state = Get<vvl::Buffer>(pDecodeInfo->srcBuffer);
    if (buffer_state) {
        const char *where = " Buffer referenced in pDecodeInfo->srcBuffer.";
        skip |= ValidateProtectedBuffer(*cb_state, *buffer_state, error_obj.location,
                                        "VUID-vkCmdDecodeVideoKHR-commandBuffer-07136", where);
        skip |= ValidateUnprotectedBuffer(*cb_state, *buffer_state, error_obj.location,
                                          "VUID-vkCmdDecodeVideoKHR-commandBuffer-07137", where);
    }

    if ((buffer_state->usage & VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR) == 0) {
        const LogObjectList objlist(commandBuffer, vs_state->videoSession(), pDecodeInfo->srcBuffer);
        skip |= LogError(objlist, "VUID-VkVideoDecodeInfoKHR-srcBuffer-07165",
                         "vkCmdDecodeVideoKHR(): pDecodeInfo->srcBuffer (%s) was not created with "
                         "VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR.",
                         FormatHandle(pDecodeInfo->srcBuffer).c_str());
    }

    if (!IsBufferCompatibleWithVideoProfile(*buffer_state, vs_state->profile)) {
        const LogObjectList objlist(commandBuffer, vs_state->videoSession(), pDecodeInfo->srcBuffer);
        skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07135",
                         "vkCmdDecodeVideoKHR(): pDecodeInfo->srcBuffer (%s) is not compatible "
                         "with the video profile %s was created with.",
                         FormatHandle(pDecodeInfo->srcBuffer).c_str(), FormatHandle(*vs_state).c_str());
    }

    if (pDecodeInfo->srcBufferOffset >= buffer_state->createInfo.size) {
        const LogObjectList objlist(commandBuffer, vs_state->videoSession(), pDecodeInfo->srcBuffer);
        skip |= LogError(objlist, "VUID-VkVideoDecodeInfoKHR-srcBufferOffset-07166",
                         "vkCmdDecodeVideoKHR(): pDecodeInfo->srcBufferOffset (%" PRIu64 ") must be less than the size (%" PRIu64
                         ") of pDecodeInfo->srcBuffer (%s).",
                         pDecodeInfo->srcBufferOffset, buffer_state->createInfo.size, FormatHandle(pDecodeInfo->srcBuffer).c_str());
    }

    if (!IsIntegerMultipleOf(pDecodeInfo->srcBufferOffset, profile_caps.base.minBitstreamBufferOffsetAlignment)) {
        const LogObjectList objlist(commandBuffer, vs_state->videoSession());
        skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07138",
                         "vkCmdDecodeVideoKHR(): pDecodeInfo->srcBufferOffset (%" PRIu64
                         ") is not an integer multiple of the minBitstreamBufferOffsetAlignment (%" PRIu64
                         ") required by the video profile %s was created with.",
                         pDecodeInfo->srcBufferOffset, profile_caps.base.minBitstreamBufferOffsetAlignment,
                         FormatHandle(*vs_state).c_str());
    }

    if (pDecodeInfo->srcBufferOffset + pDecodeInfo->srcBufferRange > buffer_state->createInfo.size) {
        const LogObjectList objlist(commandBuffer, vs_state->videoSession(), pDecodeInfo->srcBuffer);
        skip |=
            LogError(objlist, "VUID-VkVideoDecodeInfoKHR-srcBufferRange-07167",
                     "vkCmdDecodeVideoKHR(): pDecodeInfo->srcBufferOffset (%" PRIu64 ") plus pDecodeInfo->srcBufferRange (%" PRIu64
                     ") must be less than or equal to the size (%" PRIu64 ") of pDecodeInfo->srcBuffer (%s).",
                     pDecodeInfo->srcBufferOffset, pDecodeInfo->srcBufferRange, buffer_state->createInfo.size,
                     FormatHandle(pDecodeInfo->srcBuffer).c_str());
    }

    if (!IsIntegerMultipleOf(pDecodeInfo->srcBufferRange, profile_caps.base.minBitstreamBufferSizeAlignment)) {
        const LogObjectList objlist(commandBuffer, vs_state->videoSession());
        skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07139",
                         "vkCmdDecodeVideoKHR(): pDecodeInfo->srcBufferRange (%" PRIu64
                         ") is not an integer multiple of the minBitstreamBufferSizeAlignment (%" PRIu64
                         ") required by the video profile %s was created with.",
                         pDecodeInfo->srcBufferRange, profile_caps.base.minBitstreamBufferSizeAlignment,
                         FormatHandle(*vs_state).c_str());
    }

    if (vs_state->create_info.maxDpbSlots > 0 && pDecodeInfo->pSetupReferenceSlot == nullptr) {
        const LogObjectList objlist(commandBuffer, vs_state->videoSession());
        skip |= LogError("VUID-vkCmdDecodeVideoKHR-pDecodeInfo-08376", objlist, decode_info_loc.dot(Field::pSetupReferenceSlot),
                         "is NULL but the bound video session %s was created with maxDpbSlot %u.", FormatHandle(*vs_state).c_str(),
                         vs_state->create_info.maxDpbSlots);
    }

    vvl::VideoPictureResource setup_resource;
    if (pDecodeInfo->pSetupReferenceSlot) {
        if (pDecodeInfo->pSetupReferenceSlot->slotIndex < 0) {
            skip |= LogError(commandBuffer, "VUID-VkVideoDecodeInfoKHR-pSetupReferenceSlot-07168",
                             "vkCmdDecodeVideoKHR(): pDecodeInfo->pSetupReferenceSlot->slotIndex (%d) "
                             "must not be negative.",
                             pDecodeInfo->pSetupReferenceSlot->slotIndex);
        } else if ((uint32_t)pDecodeInfo->pSetupReferenceSlot->slotIndex >= vs_state->create_info.maxDpbSlots) {
            skip |= LogError(commandBuffer, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07170",
                             "vkCmdDecodeVideoKHR(): pDecodeInfo->pSetupReferenceSlot->slotIndex (%d) "
                             "must be smaller than the maxDpbSlots (%u) the bound video session %s "
                             "was created with.",
                             pDecodeInfo->pSetupReferenceSlot->slotIndex, vs_state->create_info.maxDpbSlots,
                             FormatHandle(*vs_state).c_str());
        }

        if (pDecodeInfo->pSetupReferenceSlot->pPictureResource != nullptr) {
            setup_resource = vvl::VideoPictureResource(this, *pDecodeInfo->pSetupReferenceSlot->pPictureResource);
            if (setup_resource) {
                skip |= ValidateVideoPictureResource(setup_resource, commandBuffer, *vs_state,
                                                     decode_info_loc.dot(Field::pSetupReferenceSlot).dot(Field::pPictureResource),
                                                     "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07173");

                if (bound_resources.find(setup_resource) == bound_resources.end()) {
                    skip |= LogError(commandBuffer, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07149",
                                     "vkCmdDecodeVideoKHR(): the video picture resource specified in "
                                     "pDecodeInfo->pSetupReferenceSlot->pPictureResource is not one of the "
                                     "bound video picture resources.");
                }

                skip |= VerifyImageLayout(*cb_state, *setup_resource.image_state, setup_resource.range,
                                          VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, error_obj.location,
                                          "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07254", &hit_error);
            }
        } else {
            skip |= LogError(commandBuffer, "VUID-VkVideoDecodeInfoKHR-pSetupReferenceSlot-07169",
                             "vkCmdDecodeVideoKHR(): pDecodeInfo->pSetupReferenceSlot->pPictureResource "
                             "must not be NULL");
        }
    }

    auto dst_resource = vvl::VideoPictureResource(this, pDecodeInfo->dstPictureResource);
    skip |=
        ValidateVideoPictureResource(dst_resource, commandBuffer, *vs_state, decode_info_loc.dot(Field::dstPictureResource),
                                     "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07144", "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07145");
    if (dst_resource) {
        const char *where = " Image referenced in pDecodeInfo->dstPictureResource.";
        skip |= ValidateProtectedImage(*cb_state, *dst_resource.image_state, error_obj.location,
                                       "VUID-vkCmdDecodeVideoKHR-commandBuffer-07147", where);
        skip |= ValidateUnprotectedImage(*cb_state, *dst_resource.image_state, error_obj.location,
                                         "VUID-vkCmdDecodeVideoKHR-commandBuffer-07148", where);

        if (!IsImageCompatibleWithVideoProfile(*dst_resource.image_state, vs_state->profile)) {
            const LogObjectList objlist(commandBuffer, vs_state->videoSession(), dst_resource.image_view_state->image_view(),
                                        dst_resource.image_state->image());
            skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07142",
                             "vkCmdDecodeVideoKHR(): pDecodeInfo->dstPictureResource.imageViewBinding "
                             "(%s created from %s) is not compatible with the video profile the bound "
                             "video session %s was created with.",
                             FormatHandle(pDecodeInfo->dstPictureResource.imageViewBinding).c_str(),
                             FormatHandle(dst_resource.image_state->Handle()).c_str(), FormatHandle(*vs_state).c_str());
        }

        if (dst_resource.image_view_state->create_info.format != vs_state->create_info.pictureFormat) {
            const LogObjectList objlist(commandBuffer, vs_state->videoSession(), dst_resource.image_view_state->image_view(),
                                        dst_resource.image_state->image());
            skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07143",
                             "vkCmdDecodeVideoKHR(): pDecodeInfo->dstPictureResource.imageViewBinding "
                             "(%s created from %s) format (%s) does not match the pictureFormat (%s) "
                             "the bound video session %s was created with.",
                             FormatHandle(dst_resource.image_view_state->Handle()).c_str(),
                             FormatHandle(dst_resource.image_state->Handle()).c_str(),
                             string_VkFormat(dst_resource.image_view_state->create_info.format),
                             string_VkFormat(vs_state->create_info.pictureFormat), FormatHandle(vs_state->videoSession()).c_str());
        }

        auto supported_usage = dst_resource.image_view_state->inherited_usage;
        if ((supported_usage & VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR) == 0) {
            const LogObjectList objlist(commandBuffer, vs_state->videoSession(), dst_resource.image_view_state->image_view(),
                                        dst_resource.image_state->image());
            skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07146",
                             "vkCmdDecodeVideoKHR(): pDecodeInfo->dstPictureResource.imageViewBinding "
                             "(%s created from %s) was not created with VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR "
                             "thus it cannot be used as a decode output picture with the bound video session %s "
                             "that was created with a decode operation.",
                             FormatHandle(dst_resource.image_view_state->Handle()).c_str(),
                             FormatHandle(dst_resource.image_state->Handle()).c_str(), FormatHandle(*vs_state).c_str());
        }

        bool dst_same_as_setup = (setup_resource == dst_resource);

        VkImageLayout expected_layout =
            dst_same_as_setup ? VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR : VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR;

        const char *vuid =
            dst_same_as_setup ? "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07253" : "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07252";

        skip |= VerifyImageLayout(*cb_state, *dst_resource.image_state, dst_resource.range, expected_layout, error_obj.location,
                                  vuid, &hit_error);

        if (setup_resource) {
            if ((profile_caps.decode.flags & VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR) == 0 &&
                dst_same_as_setup) {
                const LogObjectList objlist(commandBuffer, vs_state->videoSession());
                skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07140",
                                 "vkCmdDecodeVideoKHR(): the video profile %s was created with does not support "
                                 "VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR but "
                                 "pDecodeInfo->dstPictureResource and "
                                 "pDecodeInfo->pSetupReferenceSlot->pPictureResource match.",
                                 FormatHandle(*vs_state).c_str());
            }

            if ((profile_caps.decode.flags & VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR) == 0 &&
                !dst_same_as_setup) {
                const LogObjectList objlist(commandBuffer, vs_state->videoSession());
                skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07141",
                                 "vkCmdDecodeVideoKHR(): the video profile %s was created with does not support "
                                 "VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR but "
                                 "pDecodeInfo->dstPictureResource and pSetupReferenceSlot->pPictureResource "
                                 "do not match.",
                                 FormatHandle(*vs_state).c_str());
            }
        }
    }

    if (pDecodeInfo->pReferenceSlots) {
        vvl::VideoPictureResources unique_resources{};
        bool resources_unique = true;

        skip |= ValidateActiveReferencePictureCount(*cb_state, *pDecodeInfo);
        skip |= ValidateReferencePictureUseCount(*cb_state, *pDecodeInfo);

        for (uint32_t i = 0; i < pDecodeInfo->referenceSlotCount; ++i) {
            if (pDecodeInfo->pReferenceSlots[i].slotIndex < 0) {
                skip |= LogError(commandBuffer, "VUID-VkVideoDecodeInfoKHR-slotIndex-07171",
                                 "vkCmdDecodeVideoKHR(): pDecodeInfo->pReferenceSlots[%u].slotIndex (%d) "
                                 "must not be negative.",
                                 i, pDecodeInfo->pReferenceSlots[i].slotIndex);
            } else if ((uint32_t)pDecodeInfo->pReferenceSlots[i].slotIndex >= vs_state->create_info.maxDpbSlots) {
                const LogObjectList objlist(commandBuffer, vs_state->videoSession());
                skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-slotIndex-07256",
                                 "vkCmdDecodeVideoKHR(): pDecodeInfo->pReferenceSlots[%u].slotIndex (%d) "
                                 "must be smaller than the maxDpbSlots (%u) the bound video session %s "
                                 "was created with.",
                                 i, pDecodeInfo->pReferenceSlots[i].slotIndex, vs_state->create_info.maxDpbSlots,
                                 FormatHandle(*vs_state).c_str());
            }

            if (pDecodeInfo->pReferenceSlots[i].pPictureResource != nullptr) {
                auto reference_resource = vvl::VideoPictureResource(this, *pDecodeInfo->pReferenceSlots[i].pPictureResource);
                if (reference_resource) {
                    if (!unique_resources.emplace(reference_resource).second) {
                        resources_unique = false;
                    }

                    const auto &it = bound_resources.find(reference_resource);
                    if (it == bound_resources.end()) {
                        skip |= LogError(commandBuffer, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151",
                                         "vkCmdDecodeVideoKHR(): the video picture resource specified in "
                                         "pDecodeInfo->pReferenceSlots[%u].pPictureResource is not one of the "
                                         "bound video picture resources.",
                                         i);
                    } else if (pDecodeInfo->pReferenceSlots[i].slotIndex >= 0 &&
                               pDecodeInfo->pReferenceSlots[i].slotIndex != it->second) {
                        skip |= LogError(commandBuffer, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151",
                                         "vkCmdDecodeVideoKHR(): the bound video picture resource specified in "
                                         "pDecodeInfo->pReferenceSlots[%u].pPictureResource is not currently "
                                         "associated with the DPB slot index specifed in "
                                         "pDecodeInfo->pReferenceSlots[%u].slotIndex (%d).",
                                         i, i, pDecodeInfo->pReferenceSlots[i].slotIndex);
                    }

                    skip |=
                        ValidateVideoPictureResource(reference_resource, commandBuffer, *vs_state,
                                                     decode_info_loc.dot(Field::pReferenceSlots, i).dot(Field::pPictureResource),
                                                     "VUID-vkCmdDecodeVideoKHR-codedOffset-07257");

                    skip |= VerifyImageLayout(*cb_state, *reference_resource.image_state, reference_resource.range,
                                              VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, error_obj.location,
                                              "VUID-vkCmdDecodeVideoKHR-pPictureResource-07255", &hit_error);
                }
            } else {
                skip |= LogError(commandBuffer, "VUID-VkVideoDecodeInfoKHR-pPictureResource-07172",
                                 "vkCmdDecodeVideoKHR(): pDecodeInfo->pReferenceSlots[%u].pPictureResource "
                                 "must not be NULL.",
                                 i);
            }
        }

        if (!resources_unique) {
            skip |= LogError(commandBuffer, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07264",
                             "vkCmdDecodeVideoKHR(): more than one element of pDecodeInfo->pReferenceSlots "
                             "refers to the same video picture resource.");
        }
    }

    uint32_t op_count = vs_state->GetVideoDecodeOperationCount(pDecodeInfo);
    for (const auto &query : cb_state->activeQueries) {
        if (query.active_query_index + op_count > query.last_activatable_query_index + 1) {
            auto query_pool_state = Get<vvl::QueryPool>(query.pool);
            skip |= LogError(commandBuffer, "VUID-vkCmdDecodeVideoKHR-opCount-07134",
                             "vkCmdDecodeVideoKHR(): not enough activatable queries for query type %s "
                             "with opCount %u, active query index %u, and last activatable query index %u.",
                             string_VkQueryType(query_pool_state->createInfo.queryType), op_count, query.active_query_index,
                             query.last_activatable_query_index);
        }
    }

    if (vs_state->create_info.flags & VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR) {
        const auto inline_query_info = vku::FindStructInPNextChain<VkVideoInlineQueryInfoKHR>(pDecodeInfo->pNext);
        if (inline_query_info != nullptr) {
            auto query_pool_state = Get<vvl::QueryPool>(inline_query_info->queryPool);
            if (query_pool_state) {
                skip |= ValidateVideoInlineQueryInfo(*query_pool_state, *inline_query_info,
                                                     decode_info_loc.pNext(Struct::VkVideoInlineQueryInfoKHR));

                if (inline_query_info->queryCount != op_count) {
                    const LogObjectList objlist(commandBuffer, vs_state->videoSession());
                    skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-pNext-08365",
                                     "vkCmdDecodeVideoKHR(): VkVideoInlineQueryInfoKHR::queryCount (%u) "
                                     "is not equal to opCount (%u).",
                                     inline_query_info->queryCount, op_count);
                }

                if (query_pool_state->createInfo.queryType != VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR) {
                    const LogObjectList objlist(commandBuffer, inline_query_info->queryPool);
                    skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-queryType-08367",
                                     "vkCmdDecodeVideoKHR(): the query type (%s) of %s specified in "
                                     "VkVideoInlineQueryInfoKHR::queryPool is not "
                                     "VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR.",
                                     string_VkQueryType(query_pool_state->createInfo.queryType),
                                     FormatHandle(*query_pool_state).c_str());
                }

                if (vs_state->profile != query_pool_state->supported_video_profile) {
                    const LogObjectList objlist(commandBuffer, inline_query_info->queryPool, vs_state->videoSession());
                    skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-queryPool-08368",
                                     "vkCmdDecodeVideoKHR: the video profile %s was created with does not "
                                     "match the video profile of the bound video session %s.",
                                     FormatHandle(*query_pool_state).c_str(), FormatHandle(*vs_state).c_str());
                }

                const auto &qf_ext_props = queue_family_ext_props[cb_state->command_pool->queueFamilyIndex];
                if (!qf_ext_props.query_result_status_props.queryResultStatusSupport) {
                    const LogObjectList objlist(commandBuffer, inline_query_info->queryPool);
                    skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-queryType-08369",
                                     "vkCmdDecodeVideoKHR(): the command pool's queue family (index %u) the command "
                                     "buffer %s was allocated from does not support result status queries.",
                                     cb_state->command_pool->queueFamilyIndex, FormatHandle(*cb_state).c_str());
                }
            }
        }
    }

    switch (vs_state->GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
            skip |= ValidateVideoDecodeInfoH264(*cb_state, *pDecodeInfo, decode_info_loc);
            break;

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
            skip |= ValidateVideoDecodeInfoH265(*cb_state, *pDecodeInfo, decode_info_loc);
            break;

        default:
            break;
    }

    return skip;
}

void CoreChecks::PreCallRecordCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR *pDecodeInfo,
                                                const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) return;

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return;

    EnqueueVerifyVideoSessionInitialized(*cb_state, *vs_state, "VUID-vkCmdDecodeVideoKHR-None-07011");

    if (vs_state->GetCodecOp() == VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR) {
        std::vector<vvl::VideoReferenceSlot> reference_slots{};
        reference_slots.reserve(pDecodeInfo->referenceSlotCount);
        for (uint32_t i = 0; i < pDecodeInfo->referenceSlotCount; ++i) {
            reference_slots.emplace_back(this, *vs_state->profile, pDecodeInfo->pReferenceSlots[i]);
        }

        // Enqueue submission time validation of picture kind (frame, top field, bottom field) for H.264
        cb_state->video_session_updates[vs_state->videoSession()].emplace_back(
            [reference_slots](const ValidationStateTracker *dev_data, const vvl::VideoSession *vs_state,
                              vvl::VideoSessionDeviceState &dev_state, bool do_validate) {
                if (!do_validate) return false;
                bool skip = false;
                const auto log_picture_kind_error = [&](const vvl::VideoReferenceSlot &slot, const char *vuid,
                                                        const char *picture_kind) -> bool {
                    return dev_data->LogError(vs_state->Handle(), vuid,
                                              "DPB slot index %d of %s does not currently contain a %s with the specified "
                                              "video picture resource: %s, layer %u, offset (%u,%u), extent (%u,%u).",
                                              slot.index, dev_data->FormatHandle(*vs_state).c_str(), picture_kind,
                                              dev_data->FormatHandle(slot.resource.image_state->Handle()).c_str(),
                                              slot.resource.range.baseArrayLayer, slot.resource.coded_offset.x,
                                              slot.resource.coded_offset.y, slot.resource.coded_extent.width,
                                              slot.resource.coded_extent.height);
                };
                for (const auto &slot : reference_slots) {
                    if (slot.picture_id.IsFrame() &&
                        !dev_state.IsSlotPicture(slot.index, vvl::VideoPictureID::Frame(), slot.resource)) {
                        skip |= log_picture_kind_error(slot, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07266", "frame");
                    }
                    if (slot.picture_id.ContainsTopField() &&
                        !dev_state.IsSlotPicture(slot.index, vvl::VideoPictureID::TopField(), slot.resource)) {
                        skip |= log_picture_kind_error(slot, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07267", "top field");
                    }
                    if (slot.picture_id.ContainsBottomField() &&
                        !dev_state.IsSlotPicture(slot.index, vvl::VideoPictureID::BottomField(), slot.resource)) {
                        skip |= log_picture_kind_error(slot, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07268", "bottom field");
                    }
                }
                return skip;
            });
    }

    if (vs_state->create_info.flags & VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR) {
        const auto inline_query_info = vku::FindStructInPNextChain<VkVideoInlineQueryInfoKHR>(pDecodeInfo->pNext);
        if (inline_query_info != nullptr && inline_query_info->queryPool != VK_NULL_HANDLE) {
            EnqueueVerifyVideoInlineQueryUnavailable(*cb_state, *inline_query_info, Func::vkCmdDecodeVideoKHR);
        }
    }
}

bool CoreChecks::PreCallValidateCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR *pEncodeInfo,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) return false;

    skip |= ValidateCmd(*cb_state, error_obj.location);

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return false;

    const Location encode_info_loc = error_obj.location.dot(Field::pEncodeInfo);

    if (!vs_state->IsEncode()) {
        const LogObjectList objlist(commandBuffer, vs_state->videoSession());
        skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-None-08250",
                         "vkCmdEncodeVideoKHR(): the video codec operation (%s) the bound video session %s "
                         "was created with is not an encode operation.",
                         string_VkVideoCodecOperationFlagBitsKHR(vs_state->GetCodecOp()), FormatHandle(*vs_state).c_str());
        return skip;
    }

    const auto vsp_state = cb_state->bound_video_session_parameters.get();
    if (vs_state->IsEncode() && vsp_state && cb_state->video_encode_quality_level.has_value()) {
        // If we already know the current encode quality level already at command buffer recording
        // time, because it was set in this command buffer, then we do command buffer recording time
        // validation for matching parameters object encode quality level
        if (vsp_state->GetEncodeQualityLevel() != cb_state->video_encode_quality_level.value()) {
            const LogObjectList objlist(vs_state->videoSession(), vsp_state->videoSessionParameters());
            skip = LogError(objlist, "VUID-vkCmdEncodeVideoKHR-None-08318",
                            "vkCmdEncodeVideoKHR(): the currently configured encode quality level (%u) for %s "
                            "does not match the encode quality level (%u) %s was created with.",
                            cb_state->video_encode_quality_level.value(), FormatHandle(*vs_state).c_str(),
                            vsp_state->GetEncodeQualityLevel(), FormatHandle(*vsp_state).c_str());
        }
    }

    const auto &bound_resources = cb_state->bound_video_picture_resources;

    bool hit_error = false;

    const auto &profile_caps = vs_state->profile->GetCapabilities();

    auto buffer_state = Get<vvl::Buffer>(pEncodeInfo->dstBuffer);
    if (buffer_state) {
        const char *where = " Buffer referenced in pEncodeInfo->dstBuffer.";
        skip |= ValidateProtectedBuffer(*cb_state, *buffer_state, error_obj.location,
                                        "VUID-vkCmdEncodeVideoKHR-commandBuffer-08202", where);
        skip |= ValidateUnprotectedBuffer(*cb_state, *buffer_state, error_obj.location,
                                          "VUID-vkCmdEncodeVideoKHR-commandBuffer-08203", where);
    }

    if ((buffer_state->createInfo.usage & VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR) == 0) {
        const LogObjectList objlist(commandBuffer, vs_state->videoSession(), pEncodeInfo->dstBuffer);
        skip |= LogError(objlist, "VUID-VkVideoEncodeInfoKHR-dstBuffer-08236",
                         "vkCmdEncodeVideoKHR(): pEncodeInfo->dstBuffer (%s) was not created with "
                         "VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR.",
                         FormatHandle(pEncodeInfo->dstBuffer).c_str());
    }

    if (!IsBufferCompatibleWithVideoProfile(*buffer_state, vs_state->profile)) {
        const LogObjectList objlist(commandBuffer, vs_state->videoSession(), pEncodeInfo->dstBuffer);
        skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08201",
                         "vkCmdEncodeVideoKHR(): pEncodeInfo->dstBuffer (%s) is not compatible "
                         "with the video profile %s was created with.",
                         FormatHandle(pEncodeInfo->dstBuffer).c_str(), FormatHandle(*vs_state).c_str());
    }

    if (pEncodeInfo->dstBufferOffset >= buffer_state->createInfo.size) {
        const LogObjectList objlist(commandBuffer, vs_state->videoSession(), pEncodeInfo->dstBuffer);
        skip |= LogError(objlist, "VUID-VkVideoEncodeInfoKHR-dstBufferOffset-08237",
                         "vkCmdEncodeVideoKHR(): pEncodeInfo->dstBufferOffset (%" PRIu64 ") must be less than the size (%" PRIu64
                         ") of pEncodeInfo->dstBuffer (%s).",
                         pEncodeInfo->dstBufferOffset, buffer_state->createInfo.size, FormatHandle(pEncodeInfo->dstBuffer).c_str());
    }

    if (!IsIntegerMultipleOf(pEncodeInfo->dstBufferOffset, profile_caps.base.minBitstreamBufferOffsetAlignment)) {
        const LogObjectList objlist(commandBuffer, vs_state->videoSession());
        skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08204",
                         "vkCmdEncodeVideoKHR(): pEncodeInfo->dstBufferOffset (%" PRIu64
                         ") is not an integer multiple of the minBitstreamBufferOffsetAlignment (%" PRIu64
                         ") required by the video profile %s was created with.",
                         pEncodeInfo->dstBufferOffset, profile_caps.base.minBitstreamBufferOffsetAlignment,
                         FormatHandle(*vs_state).c_str());
    }

    if (pEncodeInfo->dstBufferOffset + pEncodeInfo->dstBufferRange > buffer_state->createInfo.size) {
        const LogObjectList objlist(commandBuffer, vs_state->videoSession(), pEncodeInfo->dstBuffer);
        skip |=
            LogError(objlist, "VUID-VkVideoEncodeInfoKHR-dstBufferRange-08238",
                     "vkCmdEncodeVideoKHR(): pEncodeInfo->dstBufferOffset (%" PRIu64 ") plus pEncodeInfo->dstBufferRange (%" PRIu64
                     ") must be less than or equal to the size (%" PRIu64 ") of pEncodeInfo->dstBuffer (%s).",
                     pEncodeInfo->dstBufferOffset, pEncodeInfo->dstBufferRange, buffer_state->createInfo.size,
                     FormatHandle(pEncodeInfo->dstBuffer).c_str());
    }

    if (!IsIntegerMultipleOf(pEncodeInfo->dstBufferRange, profile_caps.base.minBitstreamBufferSizeAlignment)) {
        const LogObjectList objlist(commandBuffer, vs_state->videoSession());
        skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08205",
                         "vkCmdEncodeVideoKHR(): pEncodeInfo->dstBufferRange (%" PRIu64
                         ") is not an integer multiple of the minBitstreamBufferSizeAlignment (%" PRIu64
                         ") required by the video profile %s was created with.",
                         pEncodeInfo->dstBufferRange, profile_caps.base.minBitstreamBufferSizeAlignment,
                         FormatHandle(*vs_state).c_str());
    }

    if (vs_state->create_info.maxDpbSlots > 0 && pEncodeInfo->pSetupReferenceSlot == nullptr) {
        const LogObjectList objlist(commandBuffer, vs_state->videoSession());
        skip |= LogError("VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08377", objlist, encode_info_loc.dot(Field::pSetupReferenceSlot),
                         "is NULL but the bound video session %s was created with maxDpbSlot %u.", FormatHandle(*vs_state).c_str(),
                         vs_state->create_info.maxDpbSlots);
    }

    vvl::VideoPictureResource setup_resource;
    if (pEncodeInfo->pSetupReferenceSlot) {
        if (pEncodeInfo->pSetupReferenceSlot->slotIndex < 0) {
            skip |= LogError(commandBuffer, "VUID-VkVideoEncodeInfoKHR-pSetupReferenceSlot-08239",
                             "vkCmdEncodeVideoKHR(): pEncodeInfo->pSetupReferenceSlot->slotIndex (%d) "
                             "must not be negative.",
                             pEncodeInfo->pSetupReferenceSlot->slotIndex);
        } else if ((uint32_t)pEncodeInfo->pSetupReferenceSlot->slotIndex >= vs_state->create_info.maxDpbSlots) {
            skip |= LogError(commandBuffer, "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08213",
                             "vkCmdEncodeVideoKHR(): pEncodeInfo->pSetupReferenceSlot->slotIndex (%d) "
                             "must be smaller than the maxDpbSlots (%u) the bound video session %s "
                             "was created with.",
                             pEncodeInfo->pSetupReferenceSlot->slotIndex, vs_state->create_info.maxDpbSlots,
                             FormatHandle(*vs_state).c_str());
        }

        if (pEncodeInfo->pSetupReferenceSlot->pPictureResource != nullptr) {
            setup_resource = vvl::VideoPictureResource(this, *pEncodeInfo->pSetupReferenceSlot->pPictureResource);
            if (setup_resource) {
                skip |= ValidateVideoPictureResource(setup_resource, commandBuffer, *vs_state,
                                                     encode_info_loc.dot(Field::pSetupReferenceSlot).dot(Field::pPictureResource),
                                                     "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08214");

                if (bound_resources.find(setup_resource) == bound_resources.end()) {
                    skip |= LogError(commandBuffer, "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08215",
                                     "vkCmdEncodeVideoKHR(): the video picture resource specified in "
                                     "pEncodeInfo->pSetupReferenceSlot->pPictureResource is not one of the "
                                     "bound video picture resources.");
                }

                skip |= VerifyImageLayout(*cb_state, *setup_resource.image_state, setup_resource.range,
                                          VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR, error_obj.location,
                                          "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08223", &hit_error);
            }
        } else {
            skip |= LogError(commandBuffer, "VUID-VkVideoEncodeInfoKHR-pSetupReferenceSlot-08240",
                             "vkCmdEncodeVideoKHR(): pEncodeInfo->pSetupReferenceSlot->pPictureResource "
                             "must not be NULL.");
        }
    }

    auto src_resource = vvl::VideoPictureResource(this, pEncodeInfo->srcPictureResource);
    skip |=
        ValidateVideoPictureResource(src_resource, commandBuffer, *vs_state, encode_info_loc.dot(Field::srcPictureResource),
                                     "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08208", "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08209");
    if (src_resource) {
        const char *where = " Image referenced in pEncodeInfo->srcPictureResource.";
        skip |= ValidateProtectedImage(*cb_state, *src_resource.image_state, error_obj.location,
                                       "VUID-vkCmdEncodeVideoKHR-commandBuffer-08211", where);
        skip |= ValidateUnprotectedImage(*cb_state, *src_resource.image_state, error_obj.location,
                                         "VUID-vkCmdEncodeVideoKHR-commandBuffer-08212", where);

        if (!IsImageCompatibleWithVideoProfile(*src_resource.image_state, vs_state->profile)) {
            const LogObjectList objlist(commandBuffer, vs_state->videoSession(), src_resource.image_view_state->image_view(),
                                        src_resource.image_state->image());
            skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08206",
                             "vkCmdEncodeVideoKHR(): pEncodeInfo->srcPictureResource.imageViewBinding "
                             "(%s created from %s) is not compatible with the video profile the bound "
                             "video session %s was created with.",
                             FormatHandle(pEncodeInfo->srcPictureResource.imageViewBinding).c_str(),
                             FormatHandle(src_resource.image_state->Handle()).c_str(), FormatHandle(*vs_state).c_str());
        }

        if (src_resource.image_view_state->create_info.format != vs_state->create_info.pictureFormat) {
            const LogObjectList objlist(commandBuffer, vs_state->videoSession(), src_resource.image_view_state->image_view(),
                                        src_resource.image_state->image());
            skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08207",
                             "vkCmdEncodeVideoKHR(): pEncodeInfo->srcPictureResource.imageViewBinding "
                             "(%s created from %s) format (%s) does not match the pictureFormat (%s) "
                             "the bound video session %s was created with.",
                             FormatHandle(src_resource.image_view_state->Handle()).c_str(),
                             FormatHandle(src_resource.image_state->Handle()).c_str(),
                             string_VkFormat(src_resource.image_view_state->create_info.format),
                             string_VkFormat(vs_state->create_info.pictureFormat), FormatHandle(*vs_state).c_str());
        }

        auto supported_usage = src_resource.image_view_state->inherited_usage;
        if ((supported_usage & VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR) == 0) {
            const LogObjectList objlist(commandBuffer, vs_state->videoSession(), src_resource.image_view_state->image_view(),
                                        src_resource.image_state->image());
            skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08210",
                             "vkCmdEncodeVideoKHR(): pEncodeInfo->srcPictureResource.imageViewBinding "
                             "(%s created from %s) was not created with VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR "
                             "thus it cannot be used as an encode input picture with the bound video session %s "
                             "that was created with an encode operation.",
                             FormatHandle(src_resource.image_view_state->Handle()).c_str(),
                             FormatHandle(src_resource.image_state->Handle()).c_str(), FormatHandle(*vs_state).c_str());
        }

        skip |= VerifyImageLayout(*cb_state, *src_resource.image_state, src_resource.range, VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR,
                                  error_obj.location, "VUID-vkCmdEncodeVideoKHR-pEncodeInfo-08222", &hit_error);
    }

    if (pEncodeInfo->pReferenceSlots) {
        vvl::VideoPictureResources unique_resources{};
        bool resources_unique = true;

        skip |= ValidateActiveReferencePictureCount(*cb_state, *pEncodeInfo);
        skip |= ValidateReferencePictureUseCount(*cb_state, *pEncodeInfo);

        for (uint32_t i = 0; i < pEncodeInfo->referenceSlotCount; ++i) {
            if (pEncodeInfo->pReferenceSlots[i].slotIndex < 0) {
                skip |= LogError(commandBuffer, "VUID-VkVideoEncodeInfoKHR-slotIndex-08241",
                                 "vkCmdEncodeVideoKHR(): pEncodeInfo->pReferenceSlots[%u].slotIndex (%d) "
                                 "must not be negative.",
                                 i, pEncodeInfo->pReferenceSlots[i].slotIndex);
            } else if ((uint32_t)pEncodeInfo->pReferenceSlots[i].slotIndex >= vs_state->create_info.maxDpbSlots) {
                const LogObjectList objlist(commandBuffer, vs_state->videoSession());
                skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-slotIndex-08217",
                                 "vkCmdEncodeVideoKHR(): pEncodeInfo->pReferenceSlots[%u].slotIndex (%d) "
                                 "must be smaller than the maxDpbSlots (%u) the bound video session %s "
                                 "was created with.",
                                 i, pEncodeInfo->pReferenceSlots[i].slotIndex, vs_state->create_info.maxDpbSlots,
                                 FormatHandle(*vs_state).c_str());
            }

            if (pEncodeInfo->pReferenceSlots[i].pPictureResource != nullptr) {
                auto reference_resource = vvl::VideoPictureResource(this, *pEncodeInfo->pReferenceSlots[i].pPictureResource);
                if (reference_resource) {
                    if (!unique_resources.emplace(reference_resource).second) {
                        resources_unique = false;
                    }

                    const auto &it = bound_resources.find(reference_resource);
                    if (it == bound_resources.end()) {
                        skip |= LogError(commandBuffer, "VUID-vkCmdEncodeVideoKHR-pPictureResource-08219",
                                         "vkCmdEncodeVideoKHR(): the video picture resource specified in "
                                         "pEncodeInfo->pReferenceSlots[%u].pPictureResource is not one of the "
                                         "bound video picture resources.",
                                         i);
                    } else if (pEncodeInfo->pReferenceSlots[i].slotIndex >= 0 &&
                               pEncodeInfo->pReferenceSlots[i].slotIndex != it->second) {
                        skip |= LogError(commandBuffer, "VUID-vkCmdEncodeVideoKHR-pPictureResource-08219",
                                         "vkCmdEncodeVideoKHR(): the bound video picture resource specified in "
                                         "pEncodeInfo->pReferenceSlots[%u].pPictureResource is not currently "
                                         "associated with the DPB slot index specifed in "
                                         "pEncodeInfo->pReferenceSlots[%u].slotIndex (%d).",
                                         i, i, pEncodeInfo->pReferenceSlots[i].slotIndex);
                    }

                    skip |=
                        ValidateVideoPictureResource(reference_resource, commandBuffer, *vs_state,
                                                     encode_info_loc.dot(Field::pReferenceSlots, i).dot(Field::pPictureResource),
                                                     "VUID-vkCmdEncodeVideoKHR-codedOffset-08218");

                    skip |= VerifyImageLayout(*cb_state, *reference_resource.image_state, reference_resource.range,
                                              VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR, error_obj.location,
                                              "VUID-vkCmdEncodeVideoKHR-pPictureResource-08224", &hit_error);
                }
            } else {
                skip |= LogError(commandBuffer, "VUID-VkVideoEncodeInfoKHR-pPictureResource-08242",
                                 "vkCmdEncodeVideoKHR(): pEncodeInfo->pReferenceSlots[%u].pPictureResource "
                                 "must not be NULL.",
                                 i);
            }
        }

        if (!resources_unique) {
            skip |= LogError(commandBuffer, "VUID-vkCmdEncodeVideoKHR-pPictureResource-08220",
                             "vkCmdEncodeVideoKHR(): more than one element of pEncodeInfo->pReferenceSlots "
                             "refers to the same video picture resource.");
        }
    }

    uint32_t op_count = vs_state->GetVideoEncodeOperationCount(pEncodeInfo);

    for (const auto &query : cb_state->activeQueries) {
        if (query.active_query_index + op_count > query.last_activatable_query_index + 1) {
            auto query_pool_state = Get<vvl::QueryPool>(query.pool);
            skip |= LogError(commandBuffer, "VUID-vkCmdEncodeVideoKHR-opCount-07174",
                             "vkCmdEncodeVideoKHR(): not enough activatable queries for query type %s "
                             "with opCount %u, active query index %u, and last activatable query index %u.",
                             string_VkQueryType(query_pool_state->createInfo.queryType), op_count, query.active_query_index,
                             query.last_activatable_query_index);
        }
    }

    if (vs_state->create_info.flags & VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR) {
        const auto inline_query_info = vku::FindStructInPNextChain<VkVideoInlineQueryInfoKHR>(pEncodeInfo->pNext);
        if (inline_query_info != nullptr) {
            auto query_pool_state = Get<vvl::QueryPool>(inline_query_info->queryPool);
            if (query_pool_state) {
                skip |= ValidateVideoInlineQueryInfo(*query_pool_state, *inline_query_info,
                                                     encode_info_loc.pNext(Struct::VkVideoInlineQueryInfoKHR));

                if (inline_query_info->queryCount != op_count) {
                    const LogObjectList objlist(commandBuffer, vs_state->videoSession());
                    skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-pNext-08360",
                                     "vkCmdEncodeVideoKHR(): VkVideoInlineQueryInfoKHR::queryCount (%u) "
                                     "is not equal to opCount (%u).",
                                     inline_query_info->queryCount, op_count);
                }

                if (query_pool_state->createInfo.queryType != VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR &&
                    query_pool_state->createInfo.queryType != VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR) {
                    const LogObjectList objlist(commandBuffer, inline_query_info->queryPool);
                    skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-queryType-08362",
                                     "vkCmdEncodeVideoKHR(): the query type (%s) of %s specified in "
                                     "VkVideoInlineQueryInfoKHR::queryPool is not "
                                     "VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR or VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR.",
                                     string_VkQueryType(query_pool_state->createInfo.queryType),
                                     FormatHandle(*query_pool_state).c_str());
                }

                if (vs_state->profile != query_pool_state->supported_video_profile) {
                    const LogObjectList objlist(commandBuffer, inline_query_info->queryPool, vs_state->videoSession());
                    skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-queryPool-08363",
                                     "vkCmdEncodeVideoKHR: the video profile %s was created with does not "
                                     "match the video profile of the bound video session %s.",
                                     FormatHandle(*query_pool_state).c_str(), FormatHandle(*vs_state).c_str());
                }

                const auto &qf_ext_props = queue_family_ext_props[cb_state->command_pool->queueFamilyIndex];
                if (!qf_ext_props.query_result_status_props.queryResultStatusSupport) {
                    const LogObjectList objlist(commandBuffer, inline_query_info->queryPool);
                    skip |= LogError(objlist, "VUID-vkCmdEncodeVideoKHR-queryType-08364",
                                     "vkCmdEncodeVideoKHR(): the command pool's queue family (index %u) the command "
                                     "buffer %s was allocated from does not support result status queries.",
                                     cb_state->command_pool->queueFamilyIndex, FormatHandle(*cb_state).c_str());
                }
            }
        }
    }

    switch (vs_state->GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
            skip |= ValidateVideoEncodeInfoH264(*cb_state, *pEncodeInfo, encode_info_loc);
            break;

        case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
            skip |= ValidateVideoEncodeInfoH265(*cb_state, *pEncodeInfo, encode_info_loc);
            break;

        default:
            break;
    }

    return skip;
}

void CoreChecks::PreCallRecordCmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR *pEncodeInfo,
                                                const RecordObject &record_obj) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    if (!cb_state) return;

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return;

    EnqueueVerifyVideoSessionInitialized(*cb_state, *vs_state, "VUID-vkCmdEncodeVideoKHR-None-07012");

    // For encode sessions also verify encode quality level match for the bound parameters object
    if (vs_state->IsEncode() && cb_state->bound_video_session_parameters) {
        if (!cb_state->video_encode_quality_level.has_value()) {
            // If we already know the current encode quality level already at command buffer recording
            // time, because it was set in this command buffer, then that was already checked outside
            // so we only have to do submit-time validation if that's not the case
            cb_state->video_session_updates[vs_state->videoSession()].emplace_back(
                [vsp_state = cb_state->bound_video_session_parameters](const ValidationStateTracker *dev_data,
                                                                       const vvl::VideoSession *vs_state,
                                                                       vvl::VideoSessionDeviceState &dev_state, bool do_validate) {
                    if (!do_validate) return false;
                    bool skip = false;
                    if (vsp_state->GetEncodeQualityLevel() != dev_state.GetEncodeQualityLevel()) {
                        const LogObjectList objlist(vs_state->videoSession(), vsp_state->videoSessionParameters());
                        skip |= dev_data->LogError(objlist, "VUID-vkCmdEncodeVideoKHR-None-08318",
                                                   "The currently configured encode quality level (%u) for %s "
                                                   "does not match the encode quality level (%u) %s was created with.",
                                                   dev_state.GetEncodeQualityLevel(), dev_data->FormatHandle(*vs_state).c_str(),
                                                   vsp_state->GetEncodeQualityLevel(), dev_data->FormatHandle(*vsp_state).c_str());
                    }
                    return skip;
                });
        }
    }

    if (vs_state->create_info.flags & VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR) {
        const auto inline_query_info = vku::FindStructInPNextChain<VkVideoInlineQueryInfoKHR>(pEncodeInfo->pNext);
        if (inline_query_info != nullptr && inline_query_info->queryPool != VK_NULL_HANDLE) {
            EnqueueVerifyVideoInlineQueryUnavailable(*cb_state, *inline_query_info, Func::vkCmdEncodeVideoKHR);
        }
    }
}
