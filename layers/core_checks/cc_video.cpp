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
bool CoreChecks::InsideVideoCodingScope(const CMD_BUFFER_STATE &cb_state, const Location &loc, const char *vuid) const {
    bool inside = false;
    if (cb_state.bound_video_session) {
        inside = LogError(vuid, cb_state.commandBuffer(), loc, "It is invalid to issue this call inside a video coding block.");
    }
    return inside;
}

// Flags validation error if the associated call is made outside a video coding block.
// The apiName routine should ONLY be called inside a video coding block.
bool CoreChecks::OutsideVideoCodingScope(const CMD_BUFFER_STATE &cb_state, const Location &loc, const char *vuid) const {
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

bool CoreChecks::ValidateVideoPictureResource(const VideoPictureResource &picture_resource, VkCommandBuffer cmdbuf,
                                              const VIDEO_SESSION_STATE &vs_state, const char *api_name, const char *where,
                                              const char *coded_offset_vuid, const char *coded_extent_vuid) const {
    bool skip = false;

    const auto &profile_caps = vs_state.profile->GetCapabilities();

    if (coded_offset_vuid) {
        VkOffset2D offset_granularity{0, 0};
        if (vs_state.GetCodecOp() == VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR &&
            vs_state.GetH264PictureLayout() == VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_INTERLACED_SEPARATE_PLANES_BIT_KHR) {
            offset_granularity = profile_caps.decode_h264.fieldOffsetGranularity;
        }

        if (!IsIntegerMultipleOf(picture_resource.coded_offset, offset_granularity)) {
            LogObjectList objlist(cmdbuf);
            objlist.add(vs_state.videoSession());
            skip |= LogError(objlist, coded_offset_vuid,
                             "%s(): codedOffset (%u,%u) is not an integer multiple of the codedOffsetGranularity (%u,%u).%s",
                             api_name, picture_resource.coded_offset.x, picture_resource.coded_offset.y, offset_granularity.x,
                             offset_granularity.y, where);
        }
    }

    if (coded_extent_vuid &&
        !IsBetweenInclusive(picture_resource.coded_extent, profile_caps.base.minCodedExtent, vs_state.create_info.maxCodedExtent)) {
        LogObjectList objlist(cmdbuf);
        objlist.add(vs_state.videoSession());
        skip |= LogError(objlist, coded_extent_vuid,
                         "%s(): codedExtent (%u,%u) is outside of the range (%u,%u)-(%u,%u) supported by %s.%s", api_name,
                         picture_resource.coded_extent.width, picture_resource.coded_extent.height,
                         profile_caps.base.minCodedExtent.width, profile_caps.base.minCodedExtent.height,
                         vs_state.create_info.maxCodedExtent.width, vs_state.create_info.maxCodedExtent.height,
                         FormatHandle(vs_state).c_str(), where);
    }

    if (picture_resource.base_array_layer >= picture_resource.image_view_state->create_info.subresourceRange.layerCount) {
        LogObjectList objlist(cmdbuf);
        objlist.add(vs_state.videoSession());
        objlist.add(picture_resource.image_view_state->Handle());
        objlist.add(picture_resource.image_state->Handle());
        skip |= LogError(objlist, "VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175",
                         "%s(): baseArrayLayer (%u) is greater than or equal to the layerCount (%u) "
                         "the %s specified in imageViewBinding was created with.%s",
                         api_name, picture_resource.base_array_layer,
                         picture_resource.image_view_state->create_info.subresourceRange.layerCount,
                         FormatHandle(picture_resource.image_view_state->Handle()).c_str(), where);
    }

    return skip;
}

template bool CoreChecks::ValidateVideoProfileInfo<VkDevice>(const VkVideoProfileInfoKHR *profile, const VkDevice object,
                                                             const char *api_name, const char *where) const;
template bool CoreChecks::ValidateVideoProfileInfo<VkPhysicalDevice>(const VkVideoProfileInfoKHR *profile,
                                                                     const VkPhysicalDevice object, const char *api_name,
                                                                     const char *where) const;

template <typename T1>
bool CoreChecks::ValidateVideoProfileInfo(const VkVideoProfileInfoKHR *profile, const T1 object, const char *api_name,
                                          const char *where) const {
    bool skip = false;

    const char *profile_pnext_msg = "%s(): missing %s from the pNext chain of %s";

    if (GetBitSetCount(profile->chromaSubsampling) != 1) {
        skip |= LogError(object, "VUID-VkVideoProfileInfoKHR-chromaSubsampling-07013",
                         "%s(): chromaSubsampling in %s must have a single bit set", api_name, where);
    }

    if (GetBitSetCount(profile->lumaBitDepth) != 1) {
        skip |= LogError(object, "VUID-VkVideoProfileInfoKHR-lumaBitDepth-07014",
                         "%s(): lumaBitDepth in %s must have a single bit set", api_name, where);
    }

    if (profile->chromaSubsampling != VK_VIDEO_CHROMA_SUBSAMPLING_MONOCHROME_BIT_KHR) {
        if (GetBitSetCount(profile->chromaBitDepth) != 1) {
            skip |= LogError(object, "VUID-VkVideoProfileInfoKHR-chromaSubsampling-07015",
                             "%s(): chromaBitDepth in %s must have a single bit set", api_name, where);
        }
    }

    switch (profile->videoCodecOperation) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
            const auto decode_h264 = vku::FindStructInPNextChain<VkVideoDecodeH264ProfileInfoKHR>(profile->pNext);
            if (decode_h264 == nullptr) {
                skip |= LogError(object, "VUID-VkVideoProfileInfoKHR-videoCodecOperation-07179", profile_pnext_msg, api_name,
                                 "VkVideoDecodeH264ProfileInfoKHR", where);
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
            const auto decode_h265 = vku::FindStructInPNextChain<VkVideoDecodeH265ProfileInfoKHR>(profile->pNext);
            if (decode_h265 == nullptr) {
                skip |= LogError(object, "VUID-VkVideoProfileInfoKHR-videoCodecOperation-07180", profile_pnext_msg, api_name,
                                 "VkVideoDecodeH265ProfileInfoKHR", where);
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
    const VkVideoProfileListInfoKHR *profile_list, const VkDevice object, const char *api_name, bool expect_decode_profile,
    const char *missing_decode_profile_msg_code, bool expect_encode_profile, const char *missing_encode_profile_msg_code) const;
template bool CoreChecks::ValidateVideoProfileListInfo<VkPhysicalDevice>(
    const VkVideoProfileListInfoKHR *profile_list, const VkPhysicalDevice object, const char *api_name, bool expect_decode_profile,
    const char *missing_decode_profile_msg_code, bool expect_encode_profile, const char *missing_encode_profile_msg_code) const;

template <typename T1>
bool CoreChecks::ValidateVideoProfileListInfo(const VkVideoProfileListInfoKHR *profile_list, const T1 object, const char *api_name,
                                              bool expect_decode_profile, const char *missing_decode_profile_msg_code,
                                              bool expect_encode_profile, const char *missing_encode_profile_msg_code) const {
    bool skip = false;

    bool has_decode_profile = false;
    bool has_encode_profile = false;

    if (profile_list) {
        char where[64];
        for (uint32_t i = 0; i < profile_list->profileCount; ++i) {
            snprintf(where, sizeof(where), "VkVideoProfileListInfoKHR::pProfiles[%u]", i);
            skip |= ValidateVideoProfileInfo(&profile_list->pProfiles[i], object, api_name, where);

            switch (profile_list->pProfiles[i].videoCodecOperation) {
                case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
                case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
                    if (has_decode_profile) {
                        skip |= LogError(object, "VUID-VkVideoProfileListInfoKHR-pProfiles-06813",
                                         "%s(): the video profile list contains more than one profile with decode "
                                         "codec operation",
                                         api_name);
                    }
                    has_decode_profile = true;
                    break;

                case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_EXT:
                case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_EXT:
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
        skip |= LogError(device, missing_decode_profile_msg_code,
                         "%s(): the video profile list contains no profile with decode codec operation", api_name);
    }

    if (expect_encode_profile && !has_encode_profile) {
        skip |= LogError(device, missing_encode_profile_msg_code,
                         "%s(): the video profile list contains no profile with encode codec operation", api_name);
    }

    return skip;
}

bool CoreChecks::ValidateDecodeH264ParametersAddInfo(const VkVideoDecodeH264SessionParametersAddInfoKHR *add_info, VkDevice device,
                                                     const char *api_name, const char *where,
                                                     const VkVideoDecodeH264SessionParametersCreateInfoKHR *create_info,
                                                     const VIDEO_SESSION_PARAMETERS_STATE *template_state) const {
    bool skip = false;

    vvl::unordered_set<VIDEO_SESSION_PARAMETERS_STATE::ParameterKey> keys;
    auto template_data = template_state ? template_state->Lock() : VIDEO_SESSION_PARAMETERS_STATE::ReadOnlyAccessor();

    if (add_info) {
        // Verify SPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdSPSCount; ++i) {
            auto key = VIDEO_SESSION_PARAMETERS_STATE::GetH264SPSKey(add_info->pStdSPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError(device, "VUID-VkVideoDecodeH264SessionParametersAddInfoKHR-None-04825",
                                 "%s(): H.264 SPS keys are not unique in %s", api_name, where);
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
                             "VkVideoDecodeH264SessionParametersCreateInfoKHR::maxStdSPSCount (%u)",
                             api_name, keys.size(), create_info->maxStdSPSCount);
        }
    }

    keys.clear();

    if (add_info) {
        // Verify PPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdPPSCount; ++i) {
            auto key = VIDEO_SESSION_PARAMETERS_STATE::GetH264PPSKey(add_info->pStdPPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError(device, "VUID-VkVideoDecodeH264SessionParametersAddInfoKHR-None-04826",
                                 "%s(): H.264 PPS keys are not unique in %s", api_name, where);
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
                             "VkVideoDecodeH264SessionParametersCreateInfoKHR::maxStdPPSCount (%u)",
                             api_name, keys.size(), create_info->maxStdPPSCount);
        }
    }

    return skip;
}

bool CoreChecks::ValidateDecodeH265ParametersAddInfo(const VkVideoDecodeH265SessionParametersAddInfoKHR *add_info, VkDevice device,
                                                     const char *api_name, const char *where,
                                                     const VkVideoDecodeH265SessionParametersCreateInfoKHR *create_info,
                                                     const VIDEO_SESSION_PARAMETERS_STATE *template_state) const {
    bool skip = false;

    vvl::unordered_set<VIDEO_SESSION_PARAMETERS_STATE::ParameterKey> keys;
    auto template_data = template_state ? template_state->Lock() : VIDEO_SESSION_PARAMETERS_STATE::ReadOnlyAccessor();

    if (add_info) {
        // Verify VPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdVPSCount; ++i) {
            auto key = VIDEO_SESSION_PARAMETERS_STATE::GetH265VPSKey(add_info->pStdVPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError(device, "VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04833",
                                 "%s(): H.265 VPS keys are not unique in %s", api_name, where);
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
                             "VkVideoDecodeH265SessionParametersCreateInfoKHR::maxStdVPSCount (%u)",
                             api_name, keys.size(), create_info->maxStdVPSCount);
        }
    }

    keys.clear();

    if (add_info) {
        // Verify SPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdSPSCount; ++i) {
            auto key = VIDEO_SESSION_PARAMETERS_STATE::GetH265SPSKey(add_info->pStdSPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError(device, "VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04834",
                                 "%s(): H.265 SPS keys are not unique in %s", api_name, where);
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
                             "VkVideoDecodeH265SessionParametersCreateInfoKHR::maxStdSPSCount (%u)",
                             api_name, keys.size(), create_info->maxStdSPSCount);
        }
    }

    keys.clear();

    if (add_info) {
        // Verify PPS key uniqueness
        for (uint32_t i = 0; i < add_info->stdPPSCount; ++i) {
            auto key = VIDEO_SESSION_PARAMETERS_STATE::GetH265PPSKey(add_info->pStdPPSs[i]);
            if (!keys.emplace(key).second) {
                skip |= LogError(device, "VUID-VkVideoDecodeH265SessionParametersAddInfoKHR-None-04835",
                                 "%s(): H.265 PPS keys are not unique in %s", api_name, where);
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
                             "VkVideoDecodeH265SessionParametersCreateInfoKHR::maxStdPPSCount (%u)",
                             api_name, keys.size(), create_info->maxStdPPSCount);
        }
    }

    return skip;
}

bool CoreChecks::ValidateVideoDecodeInfoH264(const CMD_BUFFER_STATE &cb_state, const VkVideoDecodeInfoKHR &decode_info) const {
    bool skip = false;

    const char *pnext_msg = "%s(): missing %s from the pNext chain of %s";

    const auto &vs_state = *cb_state.bound_video_session;
    const auto &vsp_state = *cb_state.bound_video_session_parameters;
    const auto session_params = vsp_state.Lock();

    bool interlaced_frame_support =
        (vs_state.profile->GetH264PictureLayout() != VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_PROGRESSIVE_KHR);

    auto picture_info = vku::FindStructInPNextChain<VkVideoDecodeH264PictureInfoKHR>(decode_info.pNext);
    if (picture_info) {
        auto std_picture_info = picture_info->pStdPictureInfo;

        if (!interlaced_frame_support && std_picture_info->flags.field_pic_flag) {
            LogObjectList objlist(cb_state.commandBuffer());
            objlist.add(vs_state.videoSession());
            skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-None-07258",
                             "vkCmdDecodeVideoKHR(): decode output picture is a field but the bound video session "
                             "%s was not created with interlaced frame support",
                             FormatHandle(vs_state).c_str());
        }

        for (uint32_t i = 0; i < picture_info->sliceCount; ++i) {
            if (picture_info->pSliceOffsets[i] >= decode_info.srcBufferRange) {
                skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdDecodeVideoKHR-pSliceOffsets-07153",
                                 "vkCmdDecodeVideoKHR(): VkVideoDecodeH264PictureInfoKHR::pSliceOffsets[%u] "
                                 "(%u) is greater than or equal to pDecodeInfo->srcBufferRange (%" PRIu64 ")",
                                 i, picture_info->pSliceOffsets[i], decode_info.srcBufferRange);
            }
        }

        if (session_params.GetH264SPS(std_picture_info->seq_parameter_set_id) == nullptr) {
            LogObjectList objlist(cb_state.commandBuffer());
            objlist.add(vsp_state.videoSessionParameters());
            skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-StdVideoH264SequenceParameterSet-07154",
                             "vkCmdDecodeVideoKHR(): no H.264 SPS with seq_parameter_set_id = %u "
                             "exists in the bound video session parameters object %s",
                             std_picture_info->seq_parameter_set_id, FormatHandle(vsp_state).c_str());
        }

        if (session_params.GetH264PPS(std_picture_info->seq_parameter_set_id, std_picture_info->pic_parameter_set_id) == nullptr) {
            LogObjectList objlist(cb_state.commandBuffer());
            objlist.add(vsp_state.videoSessionParameters());
            skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-StdVideoH264PictureParameterSet-07155",
                             "vkCmdDecodeVideoKHR(): no H.264 PPS with seq_parameter_set_id = %u "
                             "and pic_parameter_set_id = %u exists in the bound video session parameters object %s",
                             std_picture_info->seq_parameter_set_id, std_picture_info->pic_parameter_set_id,
                             FormatHandle(vsp_state).c_str());
        }
    } else {
        skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdDecodeVideoKHR-pNext-07152", pnext_msg, "vkCmdDecodeVideoKHR()",
                         "VkVideoDecodeH264PictureInfoKHR", "pDecodeInfo");
    }

    if (decode_info.pSetupReferenceSlot) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoDecodeH264DpbSlotInfoKHR>(decode_info.pSetupReferenceSlot->pNext);
        if (dpb_slot_info) {
            VideoPictureID picture_id(*vs_state.profile, *decode_info.pSetupReferenceSlot);
            if (!interlaced_frame_support && !picture_id.IsFrame()) {
                LogObjectList objlist(cb_state.commandBuffer());
                objlist.add(vs_state.videoSession());
                skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07259",
                                 "vkCmdDecodeVideoKHR(): reconstructed picture is a field but the bound "
                                 "video session %s was not created with interlaced frame support",
                                 FormatHandle(vs_state).c_str());
            }

            if (picture_info) {
                bool dst_is_field = (picture_info->pStdPictureInfo->flags.field_pic_flag != 0);
                bool dst_is_bottom_field = (picture_info->pStdPictureInfo->flags.bottom_field_flag != 0);

                if (!dst_is_field && !picture_id.IsFrame()) {
                    skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07261",
                                     "vkCmdDecodeVideoKHR(): decode output picture is a frame but the "
                                     "reconstructed picture is not a frame");
                }

                if (dst_is_field && !dst_is_bottom_field && !picture_id.IsTopField()) {
                    skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07262",
                                     "vkCmdDecodeVideoKHR(): decode output picture is a top field but the "
                                     "reconstructed picture is not a top field");
                }

                if (dst_is_field && dst_is_bottom_field && !picture_id.IsBottomField()) {
                    skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07263",
                                     "vkCmdDecodeVideoKHR(): decode output picture is a bottom field but the "
                                     "reconstructed picture is not a bottom field");
                }
            }
        } else {
            skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07156", pnext_msg,
                             "vkCmdDecodeVideoKHR()", "VkVideoDecodeH264DpbSlotInfoKHR", "pDecodeInfo->pSetupReferenceSlot");
        }
    }

    for (uint32_t i = 0; i < decode_info.referenceSlotCount; ++i) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoDecodeH264DpbSlotInfoKHR>(decode_info.pReferenceSlots[i].pNext);
        if (dpb_slot_info) {
            VideoPictureID picture_id(*vs_state.profile, decode_info.pReferenceSlots[i]);
            if (!interlaced_frame_support && !picture_id.IsFrame()) {
                LogObjectList objlist(cb_state.commandBuffer());
                objlist.add(vs_state.videoSession());
                skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07260",
                                 "vkCmdDecodeVideoKHR(): reference picture specified in "
                                 "pDecodeInfo->pReferneceSlots[%u] is a field but the bound "
                                 "video session %s was not created with interlaced frame support",
                                 i, FormatHandle(vs_state).c_str());
            }
        } else {
            skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdDecodeVideoKHR-pNext-07157",
                             "vkCmdDecodeVideoKHR(): missing VkVideoDecodeH264DpbSlotInfoKHR from the "
                             "pNext chain of pDecodeInfo->pReferenceSlots[%u]",
                             i);
        }
    }

    return skip;
}

bool CoreChecks::ValidateVideoDecodeInfoH265(const CMD_BUFFER_STATE &cb_state, const VkVideoDecodeInfoKHR &decode_info) const {
    bool skip = false;

    const char *pnext_msg = "%s(): missing %s from the pNext chain of %s";

    const auto &vsp_state = *cb_state.bound_video_session_parameters;
    const auto session_params = vsp_state.Lock();

    auto picture_info = vku::FindStructInPNextChain<VkVideoDecodeH265PictureInfoKHR>(decode_info.pNext);
    if (picture_info) {
        auto std_picture_info = picture_info->pStdPictureInfo;

        for (uint32_t i = 0; i < picture_info->sliceSegmentCount; ++i) {
            if (picture_info->pSliceSegmentOffsets[i] >= decode_info.srcBufferRange) {
                skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdDecodeVideoKHR-pSliceSegmentOffsets-07159",
                                 "vkCmdDecodeVideoKHR(): VkVideoDecodeH265PictureInfoKHR::pSliceSegmentOffsets[%u] "
                                 "(%u) is greater than or equal to pDecodeInfo->srcBufferRange (%" PRIu64 ")",
                                 i, picture_info->pSliceSegmentOffsets[i], decode_info.srcBufferRange);
            }
        }

        if (session_params.GetH265VPS(std_picture_info->sps_video_parameter_set_id) == nullptr) {
            LogObjectList objlist(cb_state.commandBuffer());
            objlist.add(vsp_state.videoSessionParameters());
            skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-StdVideoH265VideoParameterSet-07160",
                             "vkCmdDecodeVideoKHR(): no H.265 VPS with sps_video_parameter_set_id = %u "
                             "exists in the bound video session parameters object %s",
                             std_picture_info->sps_video_parameter_set_id, FormatHandle(vsp_state).c_str());
        }

        if (session_params.GetH265SPS(std_picture_info->sps_video_parameter_set_id, std_picture_info->pps_seq_parameter_set_id) ==
            nullptr) {
            LogObjectList objlist(cb_state.commandBuffer());
            objlist.add(vsp_state.videoSessionParameters());
            skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-StdVideoH265SequenceParameterSet-07161",
                             "vkCmdDecodeVideoKHR(): no H.265 SPS with sps_video_parameter_set_id = %u "
                             "and pps_seq_parameter_set_id = %u exists in the bound video session "
                             "parameters object %s",
                             std_picture_info->sps_video_parameter_set_id, std_picture_info->pps_seq_parameter_set_id,
                             FormatHandle(vsp_state).c_str());
        }

        if (session_params.GetH265PPS(std_picture_info->sps_video_parameter_set_id, std_picture_info->pps_seq_parameter_set_id,
                                      std_picture_info->pps_pic_parameter_set_id) == nullptr) {
            LogObjectList objlist(cb_state.commandBuffer());
            objlist.add(vsp_state.videoSessionParameters());
            skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-StdVideoH265PictureParameterSet-07162",
                             "vkCmdDecodeVideoKHR(): no H.265 SPS with sps_video_parameter_set_id = %u, "
                             "pps_seq_parameter_set_id = %u, and pps_pic_parameter_set_id = %u exists in "
                             "the bound video session parameters object %s",
                             std_picture_info->sps_video_parameter_set_id, std_picture_info->pps_seq_parameter_set_id,
                             std_picture_info->pps_pic_parameter_set_id, FormatHandle(vsp_state).c_str());
        }
    } else {
        skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdDecodeVideoKHR-pNext-07158", pnext_msg, "vkCmdDecodeVideoKHR()",
                         "VkVideoDecodeH265PictureInfoKHR", "pDecodeInfo");
    }

    if (decode_info.pSetupReferenceSlot) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoDecodeH265DpbSlotInfoKHR>(decode_info.pSetupReferenceSlot->pNext);
        if (!dpb_slot_info) {
            skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07163", pnext_msg,
                             "vkCmdDecodeVideoKHR()", "VkVideoDecodeH265DpbSlotInfoKHR", "pDecodeInfo->pSetupReferenceSlot");
        }
    }

    for (uint32_t i = 0; i < decode_info.referenceSlotCount; ++i) {
        auto dpb_slot_info = vku::FindStructInPNextChain<VkVideoDecodeH265DpbSlotInfoKHR>(decode_info.pReferenceSlots[i].pNext);
        if (!dpb_slot_info) {
            skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdDecodeVideoKHR-pNext-07164",
                             "vkCmdDecodeVideoKHR(): missing VkVideoDecodeH265DpbSlotInfoKHR from the "
                             "pNext chain of pDecodeInfo->pReferenceSlots[%u]",
                             i);
        }
    }

    return skip;
}

bool CoreChecks::ValidateActiveReferencePictureCount(const CMD_BUFFER_STATE &cb_state,
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
        LogObjectList objlist(cb_state.commandBuffer());
        objlist.add(vs_state.videoSession());
        skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-activeReferencePictureCount-07150",
                         "vkCmdDecodeVideoKHR(): more active reference pictures (%u) were specified than "
                         "the maxActiveReferencePictures (%u) the bound video session %s was created with",
                         active_reference_picture_count, vs_state.create_info.maxActiveReferencePictures,
                         FormatHandle(vs_state).c_str());
    }

    return skip;
}

bool CoreChecks::ValidateReferencePictureUseCount(const CMD_BUFFER_STATE &cb_state, const VkVideoDecodeInfoKHR &decode_info) const {
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
                             "pDecodeInfo->pSetupReferenceSlot and the elements of pDecodeInfo->pReferenceSlots",
                             i);
        }
        if (interlaced_frame_support) {
            if (dpb_top_field_use_count[i] > 1) {
                skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdDecodeVideoKHR-dpbTopFieldUseCount-07177",
                                 "vkCmdDecodeVideoKHR(): top field in DPB slot %u is referred to multiple "
                                 "times across pDecodeInfo->pSetupReferenceSlot and the elements of "
                                 "pDecodeInfo->pReferenceSlots",
                                 i);
            }
            if (dpb_bottom_field_use_count[i] > 1) {
                skip |= LogError(cb_state.commandBuffer(), "VUID-vkCmdDecodeVideoKHR-dpbBottomFieldUseCount-07178",
                                 "vkCmdDecodeVideoKHR(): bottom field in DPB slot %u is referred to multiple "
                                 "times across pDecodeInfo->pSetupReferenceSlot and the elements of "
                                 "pDecodeInfo->pReferenceSlots",
                                 i);
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                                      const VkVideoProfileInfoKHR *pVideoProfile,
                                                                      VkVideoCapabilitiesKHR *pCapabilities,
                                                                      const ErrorObject &error_obj) const {
    bool skip = false;

    skip |= ValidateVideoProfileInfo(pVideoProfile, device, "vkGetPhysicalDeviceVideoCapabilitiesKHR", "pVideoProfile");

    const char *caps_pnext_msg = "vkGetPhysicalDeviceVideoCapabilitiesKHR(): Missing %s from the pNext chain of pCapabilities";

    bool is_decode = false;

    switch (pVideoProfile->videoCodecOperation) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
            is_decode = true;
            if (!vku::FindStructInPNextChain<VkVideoDecodeH264CapabilitiesKHR>(pCapabilities->pNext)) {
                skip |= LogError(physicalDevice, "VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07184", caps_pnext_msg,
                                 "VkVideoDecodeH264CapabilitiesKHR");
            }
            break;

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
            is_decode = true;
            if (!vku::FindStructInPNextChain<VkVideoDecodeH265CapabilitiesKHR>(pCapabilities->pNext)) {
                skip |= LogError(physicalDevice, "VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07185", caps_pnext_msg,
                                 "VkVideoDecodeH265CapabilitiesKHR");
            }
            break;

        default:
            break;
    }

    if (is_decode && !vku::FindStructInPNextChain<VkVideoDecodeCapabilitiesKHR>(pCapabilities->pNext)) {
        skip |= LogError(physicalDevice, "VUID-vkGetPhysicalDeviceVideoCapabilitiesKHR-pVideoProfile-07183", caps_pnext_msg,
                         "VkVideoDecodeCapabilitiesKHR");
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceVideoFormatPropertiesKHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoFormatInfoKHR *pVideoFormatInfo,
    uint32_t *pVideoFormatPropertyCount, VkVideoFormatPropertiesKHR *pVideoFormatProperties, const ErrorObject &error_obj) const {
    bool skip = false;

    const auto *video_profiles = vku::FindStructInPNextChain<VkVideoProfileListInfoKHR>(pVideoFormatInfo->pNext);
    if (video_profiles && video_profiles->profileCount != 0) {
        skip |= ValidateVideoProfileListInfo(video_profiles, physicalDevice, "vkGetPhysicalDeviceVideoFormatPropertiesKHR", false,
                                             nullptr, false, nullptr);
    } else {
        const char *msg = video_profiles ? "no VkVideoProfileListInfoKHR structure found in the pNext chain of pVideoFormatInfo"
                                         : "profileCount is zero in the VkVideoProfileListInfoKHR structure included in the "
                                           "pNext chain of pVideoFormatInfo";
        skip |= LogError(physicalDevice, "VUID-vkGetPhysicalDeviceVideoFormatPropertiesKHR-pNext-06812",
                         "vkGetPhysicalDeviceVideoFormatPropertiesKHR(): %s", msg);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR *pCreateInfo,
                                                      const VkAllocationCallbacks *pAllocator, VkVideoSessionKHR *pVideoSession,
                                                      const ErrorObject &error_obj) const {
    bool skip = false;

    skip |= ValidateVideoProfileInfo(pCreateInfo->pVideoProfile, device, "vkCreateVideoSessionKHR", "pCreateInfo->pVideoProfile");

    VideoProfileDesc profile_desc(this, pCreateInfo->pVideoProfile);
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
                skip |= LogError(device, "VUID-VkVideoSessionCreateInfoKHR-protectedMemory-07189",
                                 "vkCreateVideoSessionKHR(): VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR "
                                 "was specified but %s",
                                 error_msg);
            }
        }

        if (!IsBetweenInclusive(pCreateInfo->maxCodedExtent, profile_caps.base.minCodedExtent, profile_caps.base.maxCodedExtent)) {
            skip |= LogError(device, "VUID-VkVideoSessionCreateInfoKHR-maxCodedExtent-04851",
                             "vkCreateVideoSessionKHR(): pCreateInfo->maxCodedExtent (%u,%u) is outside of the "
                             "range (%u,%u)-(%u,%u) supported by the video profile",
                             pCreateInfo->maxCodedExtent.width, pCreateInfo->maxCodedExtent.height,
                             profile_caps.base.minCodedExtent.width, profile_caps.base.minCodedExtent.height,
                             profile_caps.base.maxCodedExtent.width, profile_caps.base.maxCodedExtent.height);
        }

        if (pCreateInfo->maxDpbSlots > profile_caps.base.maxDpbSlots) {
            skip |= LogError(device, "VUID-VkVideoSessionCreateInfoKHR-maxDpbSlots-04847",
                             "vkCreateVideoSessionKHR(): pCreateInfo->maxDpbSlots (%u) is greater than the "
                             "maxDpbSlots (%u) supported by the video profile",
                             pCreateInfo->maxDpbSlots, profile_caps.base.maxDpbSlots);
        }

        if (pCreateInfo->maxActiveReferencePictures > profile_caps.base.maxActiveReferencePictures) {
            skip |= LogError(device, "VUID-VkVideoSessionCreateInfoKHR-maxActiveReferencePictures-04849",
                             "vkCreateVideoSessionKHR(): pCreateInfo->maxActiveReferencePictures (%u) is greater "
                             "than the maxActiveReferencePictures (%u) supported by the video profile",
                             pCreateInfo->maxActiveReferencePictures, profile_caps.base.maxActiveReferencePictures);
        }

        if ((pCreateInfo->maxDpbSlots == 0 && pCreateInfo->maxActiveReferencePictures != 0) ||
            (pCreateInfo->maxDpbSlots != 0 && pCreateInfo->maxActiveReferencePictures == 0)) {
            skip |= LogError(device, "VUID-VkVideoSessionCreateInfoKHR-maxDpbSlots-04850",
                             "vkCreateVideoSessionKHR(): if either pCreateInfo->maxDpbSlots (%u) or "
                             "pCreateInfo->maxActiveReferencePictures (%u) is zero then both must be zero",
                             pCreateInfo->maxDpbSlots, pCreateInfo->maxActiveReferencePictures);
        }

        if (profile_desc.GetProfile().is_decode && pCreateInfo->maxActiveReferencePictures > 0 &&
            !IsVideoFormatSupported(pCreateInfo->referencePictureFormat, VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR,
                                    pCreateInfo->pVideoProfile)) {
            skip |= LogError(device, "VUID-VkVideoSessionCreateInfoKHR-referencePictureFormat-04852",
                             "vkCreateVideoSessionKHR(): pCreateInfo->referencePictureFormat (%s) is not a supported "
                             "decode DPB format for the video profile specified in pCreateInfo->pVideoProfile",
                             string_VkFormat(pCreateInfo->referencePictureFormat));
        }

        if (profile_desc.GetProfile().is_decode &&
            !IsVideoFormatSupported(pCreateInfo->pictureFormat, VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR,
                                    pCreateInfo->pVideoProfile)) {
            skip |= LogError(device, "VUID-VkVideoSessionCreateInfoKHR-pictureFormat-04853",
                             "vkCreateVideoSessionKHR(): pCreateInfo->pictureFormat (%s) is not a supported "
                             "decode output format for the video profile specified in pCreateInfo->pVideoProfile",
                             string_VkFormat(pCreateInfo->pictureFormat));
        }

        if (strncmp(pCreateInfo->pStdHeaderVersion->extensionName, profile_caps.base.stdHeaderVersion.extensionName,
                    VK_MAX_EXTENSION_NAME_SIZE)) {
            skip |= LogError(device, "VUID-VkVideoSessionCreateInfoKHR-pStdHeaderVersion-07190",
                             "vkCreateVideoSessionKHR(): unsupported Video Std header name '%.*s' specified in "
                             "pCreateInfo->pStdHeaderVersion->extensionName, expected '%.*s'",
                             VK_MAX_EXTENSION_NAME_SIZE, pCreateInfo->pStdHeaderVersion->extensionName, VK_MAX_EXTENSION_NAME_SIZE,
                             profile_caps.base.stdHeaderVersion.extensionName);
        }

        if (pCreateInfo->pStdHeaderVersion->specVersion > profile_caps.base.stdHeaderVersion.specVersion) {
            skip |= LogError(device, "VUID-VkVideoSessionCreateInfoKHR-pStdHeaderVersion-07191",
                             "vkCreateVideoSessionKHR(): Video Std header version (0x%08x) specified in "
                             "pCreateInfo->pStdHeaderVersion->specVersion is larger than the supported version (0x%08x)",
                             pCreateInfo->pStdHeaderVersion->specVersion, profile_caps.base.stdHeaderVersion.specVersion);
        }
    } else {
        skip |= LogError(device, "VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-04845",
                         "vkCreateVideoSessionKHR(): the video profile specified in pCreateInfo->pVideoProfile "
                         "is not supported");
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                       const VkAllocationCallbacks *pAllocator,
                                                       const ErrorObject &error_obj) const {
    auto video_session_state = Get<VIDEO_SESSION_STATE>(videoSession);
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

    auto vs_state = Get<VIDEO_SESSION_STATE>(videoSession);
    assert(vs_state);

    if (pBindSessionMemoryInfos) {
        {
            vvl::unordered_set<uint32_t> memory_bind_indices;
            for (uint32_t i = 0; i < bindSessionMemoryInfoCount; ++i) {
                uint32_t mem_bind_index = pBindSessionMemoryInfos[i].memoryBindIndex;
                if (memory_bind_indices.find(mem_bind_index) != memory_bind_indices.end()) {
                    skip |= LogError(videoSession, "VUID-vkBindVideoSessionMemoryKHR-memoryBindIndex-07196",
                                     "vkBindVideoSessionMemoryKHR(): memoryBindIndex values in pBindSessionMemoryInfos "
                                     "array are not unique");
                    break;
                }
                memory_bind_indices.emplace(mem_bind_index);
            }
        }

        for (uint32_t i = 0; i < bindSessionMemoryInfoCount; ++i) {
            const auto &bind_info = pBindSessionMemoryInfos[i];
            const auto &mem_binding_info = vs_state->GetMemoryBindingInfo(bind_info.memoryBindIndex);
            if (mem_binding_info != nullptr) {
                auto mem_state = Get<DEVICE_MEMORY_STATE>(bind_info.memory);
                if (mem_state) {
                    if (((1 << mem_state->alloc_info.memoryTypeIndex) & mem_binding_info->requirements.memoryTypeBits) == 0) {
                        LogObjectList objlist(videoSession);
                        objlist.add(mem_state->Handle());
                        skip |= LogError(objlist, "VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07198",
                                         "vkBindVideoSessionMemoryKHR(): memoryTypeBits (0x%x) for memory binding "
                                         "with index %u of %s are not compatible with the memory type index (%u) of "
                                         "%s specified in pBindSessionMemoryInfos[%u].memory",
                                         mem_binding_info->requirements.memoryTypeBits, bind_info.memoryBindIndex,
                                         FormatHandle(videoSession).c_str(), mem_state->alloc_info.memoryTypeIndex,
                                         FormatHandle(*mem_state).c_str(), i);
                    }

                    if (bind_info.memoryOffset >= mem_state->alloc_info.allocationSize) {
                        LogObjectList objlist(videoSession);
                        objlist.add(mem_state->Handle());
                        skip |= LogError(objlist, "VUID-VkBindVideoSessionMemoryInfoKHR-memoryOffset-07201",
                                         "vkBindVideoSessionMemoryKHR(): pBindSessionMemoryInfos[%u].memoryOffset (%" PRIuLEAST64
                                         ") must be less than the size (%" PRIuLEAST64 ") of %s",
                                         i, bind_info.memoryOffset, mem_state->alloc_info.allocationSize,
                                         FormatHandle(*mem_state).c_str());
                    } else if (bind_info.memoryOffset + bind_info.memorySize > mem_state->alloc_info.allocationSize) {
                        LogObjectList objlist(videoSession);
                        objlist.add(mem_state->Handle());
                        skip |= LogError(
                            objlist, "VUID-VkBindVideoSessionMemoryInfoKHR-memorySize-07202",
                            "vkBindVideoSessionMemoryKHR(): memoryOffset (%" PRIuLEAST64 ") + memory size (%" PRIuLEAST64
                            ") specified in pBindSessionMemoryInfos[%u] must be less than or equal to the size (%" PRIuLEAST64
                            ") of %s",
                            bind_info.memoryOffset, bind_info.memorySize, i, mem_state->alloc_info.allocationSize,
                            FormatHandle(*mem_state).c_str());
                    }
                }

                if (SafeModulo(bind_info.memoryOffset, mem_binding_info->requirements.alignment) != 0) {
                    skip |= LogError(videoSession, "VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07199",
                                     "vkBindVideoSessionMemoryKHR(): pBindSessionMemoryInfos[%u].memoryOffset is %" PRIuLEAST64
                                     " but must be an integer multiple of the alignment value %" PRIuLEAST64
                                     " for the memory binding index %u of %s",
                                     i, bind_info.memoryOffset, mem_binding_info->requirements.alignment, bind_info.memoryBindIndex,
                                     FormatHandle(videoSession).c_str());
                }

                if (bind_info.memorySize != mem_binding_info->requirements.size) {
                    skip |= LogError(videoSession, "VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07200",
                                     "vkBindVideoSessionMemoryKHR(): pBindSessionMemoryInfos[%u].memorySize (%" PRIuLEAST64
                                     ") does not equal the required size (%" PRIuLEAST64 ") for the memory binding index %u of %s",
                                     i, bind_info.memorySize, mem_binding_info->requirements.size, bind_info.memoryBindIndex,
                                     FormatHandle(videoSession).c_str());
                }

                if (mem_binding_info->bound) {
                    skip |= LogError(videoSession, "VUID-vkBindVideoSessionMemoryKHR-videoSession-07195",
                                     "vkBindVideoSessionMemoryKHR(): memory binding with index %u of %s is already "
                                     "bound but was specified in pBindSessionMemoryInfos[%u].memoryBindIndex",
                                     bind_info.memoryBindIndex, FormatHandle(videoSession).c_str(), i);
                }
            } else {
                skip |= LogError(videoSession, "VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07197",
                                 "vkBindVideoSessionMemoryKHR(): %s does not have a memory binding corresponding "
                                 "to the memoryBindIndex specified in pBindSessionMemoryInfos[%u]",
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

    std::shared_ptr<const VIDEO_SESSION_PARAMETERS_STATE> template_state;
    if (pCreateInfo->videoSessionParametersTemplate != VK_NULL_HANDLE) {
        template_state = Get<VIDEO_SESSION_PARAMETERS_STATE>(pCreateInfo->videoSessionParametersTemplate);
        if (template_state->vs_state->videoSession() != pCreateInfo->videoSession) {
            LogObjectList objlist(device);
            objlist.add(pCreateInfo->videoSessionParametersTemplate);
            objlist.add(pCreateInfo->videoSession);
            skip |= LogError(objlist, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-04855",
                             "vkCreateVideoSessionParametersKHR(): template %s was not created against the same %s",
                             FormatHandle(pCreateInfo->videoSessionParametersTemplate).c_str(),
                             FormatHandle(pCreateInfo->videoSession).c_str());
        }
    }

    auto vs_state = Get<VIDEO_SESSION_STATE>(pCreateInfo->videoSession);
    assert(vs_state);

    const char *pnext_chain_msg = "vkCreateVideoSessionParametersKHR(): missing %s from pCreateInfo pNext chain";
    switch (vs_state->GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
            auto codec_info = vku::FindStructInPNextChain<VkVideoDecodeH264SessionParametersCreateInfoKHR>(pCreateInfo->pNext);
            if (codec_info) {
                skip |= ValidateDecodeH264ParametersAddInfo(
                    codec_info->pParametersAddInfo, device, "vkCreateVideoSessionParametersKHR",
                    "VkVideoDecodeH264SessionParametersCreateInfoKHR::pParametersAddInfo", codec_info, template_state.get());
            } else {
                skip |= LogError(device, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07203", pnext_chain_msg,
                                 "VkVideoDecodeH264SessionParametersCreateInfoKHR");
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
            auto codec_info = vku::FindStructInPNextChain<VkVideoDecodeH265SessionParametersCreateInfoKHR>(pCreateInfo->pNext);
            if (codec_info) {
                skip |= ValidateDecodeH265ParametersAddInfo(
                    codec_info->pParametersAddInfo, device, "vkCreateVideoSessionParametersKHR",
                    "VkVideoDecodeH265SessionParametersCreateInfoKHR::pParametersAddInfo", codec_info, template_state.get());
            } else {
                skip |= LogError(device, "VUID-VkVideoSessionParametersCreateInfoKHR-videoSession-07206", pnext_chain_msg,
                                 "VkVideoDecodeH265SessionParametersCreateInfoKHR");
            }
            break;
        }

        default:
            break;
    }

    return skip;
}

bool CoreChecks::PreCallValidateUpdateVideoSessionParametersKHR(VkDevice device, VkVideoSessionParametersKHR videoSessionParameters,
                                                                const VkVideoSessionParametersUpdateInfoKHR *pUpdateInfo,
                                                                const ErrorObject &error_obj) const {
    bool skip = false;

    auto vsp_state = Get<VIDEO_SESSION_PARAMETERS_STATE>(videoSessionParameters);
    assert(vsp_state);

    auto vsp_data = vsp_state->Lock();

    if (pUpdateInfo->updateSequenceCount != vsp_data->update_sequence_counter + 1) {
        skip |= LogError(device, "VUID-vkUpdateVideoSessionParametersKHR-pUpdateInfo-07215",
                         "vkUpdateVideoSessionParametersKHR(): incorrect updateSequenceCount");
    }

    switch (vsp_state->GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
            auto add_info = vku::FindStructInPNextChain<VkVideoDecodeH264SessionParametersAddInfoKHR>(pUpdateInfo->pNext);
            if (add_info) {
                skip |= ValidateDecodeH264ParametersAddInfo(add_info, device, "vkUpdateVideoSessionParametersKHR",
                                                            "VkVideoDecodeH264SessionParametersAddInfoKHR");

                for (uint32_t i = 0; i < add_info->stdSPSCount; ++i) {
                    if (vsp_data.GetH264SPS(add_info->pStdSPSs[i].seq_parameter_set_id)) {
                        skip |=
                            LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07216",
                                     "vkUpdateVideoSessionParametersKHR(): H.264 SPS with key "
                                     "(SPS ID = %u) already exists in %s",
                                     add_info->pStdSPSs[i].seq_parameter_set_id, FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdSPSCount + vsp_data->h264.sps.size() > vsp_data->h264.spsCapacity) {
                    skip |= LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07217",
                                     "vkUpdateVideoSessionParametersKHR(): number of H.264 SPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.264 "
                                     "SPS capacity (%u) the %s was created with",
                                     add_info->stdSPSCount, vsp_data->h264.sps.size(), vsp_data->h264.spsCapacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }

                for (uint32_t i = 0; i < add_info->stdPPSCount; ++i) {
                    if (vsp_data.GetH264PPS(add_info->pStdPPSs[i].seq_parameter_set_id,
                                            add_info->pStdPPSs[i].pic_parameter_set_id)) {
                        skip |=
                            LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07218",
                                     "vkUpdateVideoSessionParametersKHR(): H.264 PPS with key "
                                     "(SPS ID = %u, PPS ID = %u) already exists in %s",
                                     add_info->pStdPPSs[i].seq_parameter_set_id, add_info->pStdPPSs[i].pic_parameter_set_id,
                                     FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdPPSCount + vsp_data->h264.pps.size() > vsp_data->h264.ppsCapacity) {
                    skip |= LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07219",
                                     "vkUpdateVideoSessionParametersKHR(): number of H.264 PPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.264 "
                                     "PPS capacity (%u) the %s was created with",
                                     add_info->stdPPSCount, vsp_data->h264.pps.size(), vsp_data->h264.ppsCapacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
            auto add_info = vku::FindStructInPNextChain<VkVideoDecodeH265SessionParametersAddInfoKHR>(pUpdateInfo->pNext);
            if (add_info) {
                skip |= ValidateDecodeH265ParametersAddInfo(add_info, device, "vkUpdateVideoSessionParametersKHR",
                                                            "VkVideoDecodeH265SessionParametersAddInfoKHR");

                for (uint32_t i = 0; i < add_info->stdVPSCount; ++i) {
                    if (vsp_data.GetH265VPS(add_info->pStdVPSs[i].vps_video_parameter_set_id)) {
                        skip |= LogError(
                            videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07220",
                            "vkUpdateVideoSessionParametersKHR(): H.265 VPS with key "
                            "(VPS ID = %u) already exists in %s",
                            add_info->pStdVPSs[i].vps_video_parameter_set_id, FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdVPSCount + vsp_data->h265.vps.size() > vsp_data->h265.vpsCapacity) {
                    skip |= LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07221",
                                     "vkUpdateVideoSessionParametersKHR(): number of H.265 VPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.265 "
                                     "VPS capacity (%u) the %s was created with",
                                     add_info->stdVPSCount, vsp_data->h265.vps.size(), vsp_data->h265.vpsCapacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }

                for (uint32_t i = 0; i < add_info->stdSPSCount; ++i) {
                    if (vsp_data.GetH265SPS(add_info->pStdSPSs[i].sps_video_parameter_set_id,
                                            add_info->pStdSPSs[i].sps_seq_parameter_set_id)) {
                        skip |=
                            LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07222",
                                     "vkUpdateVideoSessionParametersKHR(): H.265 SPS with key "
                                     "(VPS ID = %u, SPS ID = %u) already exists in %s",
                                     add_info->pStdSPSs[i].sps_video_parameter_set_id,
                                     add_info->pStdSPSs[i].sps_seq_parameter_set_id, FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdSPSCount + vsp_data->h265.sps.size() > vsp_data->h265.spsCapacity) {
                    skip |= LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07223",
                                     "vkUpdateVideoSessionParametersKHR(): number of H.265 SPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.265 "
                                     "SPS capacity (%u) the %s was created with",
                                     add_info->stdSPSCount, vsp_data->h265.sps.size(), vsp_data->h265.spsCapacity,
                                     FormatHandle(videoSessionParameters).c_str());
                }

                for (uint32_t i = 0; i < add_info->stdPPSCount; ++i) {
                    if (vsp_data.GetH265PPS(add_info->pStdPPSs[i].sps_video_parameter_set_id,
                                            add_info->pStdPPSs[i].pps_seq_parameter_set_id,
                                            add_info->pStdPPSs[i].pps_pic_parameter_set_id)) {
                        skip |= LogError(
                            videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07224",
                            "vkUpdateVideoSessionParametersKHR(): H.265 PPS with key "
                            "(VPS ID = %u, SPS ID = %u, PPS ID = %u) already exists in %s",
                            add_info->pStdPPSs[i].sps_video_parameter_set_id, add_info->pStdPPSs[i].pps_seq_parameter_set_id,
                            add_info->pStdPPSs[i].pps_pic_parameter_set_id, FormatHandle(videoSessionParameters).c_str());
                    }
                }

                if (add_info->stdPPSCount + vsp_data->h265.pps.size() > vsp_data->h265.ppsCapacity) {
                    skip |= LogError(videoSessionParameters, "VUID-vkUpdateVideoSessionParametersKHR-videoSessionParameters-07225",
                                     "vkUpdateVideoSessionParametersKHR(): number of H.265 PPS entries to add "
                                     "(%u) plus the already used capacity (%zu) is greater than the maximum H.265 "
                                     "PPS capacity (%u) the %s was created with",
                                     add_info->stdPPSCount, vsp_data->h265.pps.size(), vsp_data->h265.ppsCapacity,
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
    auto video_session_parameters_state = Get<VIDEO_SESSION_PARAMETERS_STATE>(videoSessionParameters);
    bool skip = false;
    if (video_session_parameters_state) {
        skip |= ValidateObjectNotInUse(video_session_parameters_state.get(), error_obj.location,
                                       "VUID-vkDestroyVideoSessionParametersKHR-videoSessionParameters-07212");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR *pBeginInfo,
                                                       const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state) return false;

    if (cb_state->activeQueries.size() > 0) {
        skip |= LogError(commandBuffer, "VUID-vkCmdBeginVideoCodingKHR-None-07232",
                         "vkCmdBeginVideoCodingKHR(): %s has active queries", FormatHandle(commandBuffer).c_str());
    }

    auto vs_state = Get<VIDEO_SESSION_STATE>(pBeginInfo->videoSession);
    assert(vs_state);

    auto vsp_state = Get<VIDEO_SESSION_PARAMETERS_STATE>(pBeginInfo->videoSessionParameters);

    auto qf_ext_props = queue_family_ext_props[cb_state->command_pool->queueFamilyIndex];

    if ((qf_ext_props.video_props.videoCodecOperations & vs_state->GetCodecOp()) == 0) {
        LogObjectList objlist(commandBuffer);
        objlist.add(pBeginInfo->videoSession);
        objlist.add(cb_state->command_pool->Handle());
        skip |= LogError(objlist, "VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07231",
                         "vkCmdBeginVideoCodingKHR(): %s does not support video codec operation %s "
                         "that %s specified in pBeginInfo->videoSession was created with",
                         FormatHandle(cb_state->command_pool->Handle()).c_str(),
                         string_VkVideoCodecOperationFlagBitsKHR(vs_state->GetCodecOp()),
                         FormatHandle(pBeginInfo->videoSession).c_str());
    }

    if (vs_state->GetUnboundMemoryBindingCount() > 0) {
        LogObjectList objlist(commandBuffer);
        objlist.add(pBeginInfo->videoSession);
        skip |= LogError(objlist, "VUID-VkVideoBeginCodingInfoKHR-videoSession-07237",
                         "vkCmdBeginVideoCodingKHR(): %s has %u unbound memory binding indices",
                         FormatHandle(pBeginInfo->videoSession).c_str(), vs_state->GetUnboundMemoryBindingCount());
    }

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state->unprotected == true) &&
        ((vs_state->create_info.flags & VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR) != 0)) {
        LogObjectList objlist(commandBuffer);
        objlist.add(pBeginInfo->videoSession);
        skip |= LogError(objlist, "VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07233",
                         "vkCmdBeginVideoCodingKHR(): %s is unprotected while %s was created with "
                         "VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR",
                         FormatHandle(commandBuffer).c_str(), FormatHandle(pBeginInfo->videoSession).c_str());
    }

    // if driver supports protectedNoFault the operation is valid, just has undefined values
    if ((!phys_dev_props_core11.protectedNoFault) && (cb_state->unprotected == false) &&
        ((vs_state->create_info.flags & VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR) == 0)) {
        LogObjectList objlist(commandBuffer);
        objlist.add(pBeginInfo->videoSession);
        skip |= LogError(objlist, "VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07234",
                         "vkCmdBeginVideoCodingKHR(): %s is protected while %s was created without "
                         "VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR",
                         FormatHandle(commandBuffer).c_str(), FormatHandle(pBeginInfo->videoSession).c_str());
    }

    if (pBeginInfo->pReferenceSlots) {
        VideoPictureResources unique_resources{};
        bool resources_unique = true;
        bool has_separate_images = false;
        const IMAGE_STATE *last_dpb_image = nullptr;
        char where[64];

        for (uint32_t i = 0; i < pBeginInfo->referenceSlotCount; ++i) {
            const auto &slot = pBeginInfo->pReferenceSlots[i];

            if (slot.slotIndex >= 0 && (uint32_t)slot.slotIndex >= vs_state->create_info.maxDpbSlots) {
                LogObjectList objlist(commandBuffer);
                objlist.add(pBeginInfo->videoSession);
                skip |= LogError(objlist, "VUID-VkVideoBeginCodingInfoKHR-slotIndex-04856",
                                 "vkCmdBeginVideoCodingKHR(): pBeginInfo->pReferenceSlots[%u].slotIndex (%d) "
                                 "is greater than the maxDpbSlots %s was created with",
                                 i, slot.slotIndex, FormatHandle(pBeginInfo->videoSession).c_str());
            }

            if (slot.pPictureResource != nullptr) {
                snprintf(where, sizeof(where), " Image referenced in pBeginInfo->pReferenceSlots[%u]", i);
                auto reference_resource = VideoPictureResource(this, *slot.pPictureResource);
                skip |= ValidateVideoPictureResource(reference_resource, commandBuffer, *vs_state, "vkCmdBeginVideoCodingKHR()",
                                                     where, "VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07242",
                                                     "VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07243");
                if (reference_resource) {
                    if (!unique_resources.emplace(reference_resource).second) {
                        resources_unique = false;
                    }

                    if (last_dpb_image != nullptr && last_dpb_image != reference_resource.image_state.get()) {
                        has_separate_images = true;
                    }

                    skip |= ValidateProtectedImage(*cb_state, *reference_resource.image_state, error_obj.location,
                                                   "VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07235", where);
                    skip |= ValidateUnprotectedImage(*cb_state, *reference_resource.image_state, error_obj.location,
                                                     "VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07236", where);

                    const auto &supported_profiles = reference_resource.image_state->supported_video_profiles;
                    if (supported_profiles.find(vs_state->profile) == supported_profiles.end()) {
                        LogObjectList objlist(commandBuffer);
                        objlist.add(pBeginInfo->videoSession);
                        objlist.add(reference_resource.image_view_state->image_view());
                        objlist.add(reference_resource.image_state->image());
                        skip |= LogError(objlist, "VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07240",
                                         "vkCmdBeginVideoCodingKHR(): pBeginInfo->pReferenceSlots[%u]."
                                         "imageViewBinding (%s created from %s) is not compatible with the "
                                         "video profile %s was created with",
                                         i, FormatHandle(reference_resource.image_view_state->image_view()).c_str(),
                                         FormatHandle(reference_resource.image_state->image()).c_str(),
                                         FormatHandle(pBeginInfo->videoSession).c_str());
                    }

                    if (reference_resource.image_view_state->create_info.format != vs_state->create_info.referencePictureFormat) {
                        LogObjectList objlist(commandBuffer);
                        objlist.add(pBeginInfo->videoSession);
                        objlist.add(reference_resource.image_view_state->image_view());
                        objlist.add(reference_resource.image_state->image());
                        skip |= LogError(objlist, "VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07241",
                                         "vkCmdBeginVideoCodingKHR(): pBeginInfo->pReferenceSlots[%u]."
                                         "imageViewBinding (%s created from %s) format (%s) does not match "
                                         "the referencePictureFormat (%s) %s was created with",
                                         i, FormatHandle(reference_resource.image_view_state->image_view()).c_str(),
                                         FormatHandle(reference_resource.image_state->image()).c_str(),
                                         string_VkFormat(reference_resource.image_view_state->create_info.format),
                                         string_VkFormat(vs_state->create_info.referencePictureFormat),
                                         FormatHandle(pBeginInfo->videoSession).c_str());
                    }

                    auto supported_usage = reference_resource.image_view_state->inherited_usage;
                    if ((supported_usage & VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR) == 0) {
                        LogObjectList objlist(commandBuffer);
                        objlist.add(pBeginInfo->videoSession);
                        objlist.add(reference_resource.image_view_state->image_view());
                        objlist.add(reference_resource.image_state->image());
                        skip |= LogError(objlist, "VUID-VkVideoBeginCodingInfoKHR-slotIndex-07245",
                                         "vkCmdBeginVideoCodingKHR(): pBeginInfo->pReferenceSlots[%u].imageViewBinding "
                                         "(%s created from %s) was not created with VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR "
                                         "thus it cannot be used as a reference picture with %s that was created with a "
                                         "decode operation",
                                         i, FormatHandle(reference_resource.image_view_state->image_view()).c_str(),
                                         FormatHandle(reference_resource.image_state->image()).c_str(),
                                         FormatHandle(pBeginInfo->videoSession).c_str());
                    }

                    last_dpb_image = reference_resource.image_state.get();
                }
            }
        }

        if (!resources_unique) {
            LogObjectList objlist(commandBuffer);
            objlist.add(pBeginInfo->videoSession);
            skip |= LogError(objlist, "VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07238",
                             "vkCmdBeginVideoCodingKHR(): more than one element of pBeginInfo->pReferenceSlots "
                             "refers to the same video picture resource");
        }

        auto supported_cap_flags = vs_state->profile->GetCapabilities().base.flags;
        if ((supported_cap_flags & VK_VIDEO_CAPABILITY_SEPARATE_REFERENCE_IMAGES_BIT_KHR) == 0 && has_separate_images) {
            LogObjectList objlist(commandBuffer);
            objlist.add(pBeginInfo->videoSession);
            skip |= LogError(objlist, "VUID-VkVideoBeginCodingInfoKHR-flags-07244",
                             "vkCmdBeginVideoCodingKHR(): not all elements of pBeginInfo->pReferenceSlots refer "
                             "to the same image and the video profile %s was created with does not support "
                             "VK_VIDEO_CAPABILITY_SEPARATE_REFERENCE_IMAGES_BIT_KHR",
                             FormatHandle(pBeginInfo->videoSession).c_str());
        }
    }

    if (vsp_state && vsp_state->vs_state->videoSession() != vs_state->videoSession()) {
        LogObjectList objlist(commandBuffer);
        objlist.add(pBeginInfo->videoSessionParameters);
        objlist.add(pBeginInfo->videoSession);
        skip |= LogError(objlist, "VUID-VkVideoBeginCodingInfoKHR-videoSessionParameters-04857",
                         "vkCmdBeginVideoCodingKHR(): %s was not created for %s",
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

        default:
            break;
    }

    if ((codec_op_requires_params_vuid != nullptr) && (vsp_state == nullptr)) {
        LogObjectList objlist(commandBuffer);
        objlist.add(pBeginInfo->videoSession);
        skip |= LogError(objlist, codec_op_requires_params_vuid,
                         "vkCmdBeginVideoCodingKHR(): %s was created with %s but no video session parameters object was "
                         "specified in pBeginInfo->videoSessionParameters",
                         FormatHandle(pBeginInfo->videoSession).c_str(),
                         string_VkVideoCodecOperationFlagBitsKHR(vs_state->GetCodecOp()));
    }

    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR *pEndCodingInfo,
                                                     const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state) return false;

    if (cb_state->activeQueries.size() > 0) {
        skip |= LogError(commandBuffer, "VUID-vkCmdEndVideoCodingKHR-None-07251", "vkCmdEndVideoCodingKHR(): %s has active queries",
                         FormatHandle(commandBuffer).c_str());
    }

    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                         const VkVideoCodingControlInfoKHR *pCodingControlInfo,
                                                         const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state) return false;

    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}

bool CoreChecks::PreCallValidateCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR *pDecodeInfo,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state) return false;

    const auto vs_state = cb_state->bound_video_session.get();
    if (!vs_state) return false;

    const auto &bound_resources = cb_state->bound_video_picture_resources;

    bool hit_error = false;

    const auto &profile_caps = vs_state->profile->GetCapabilities();

    auto buffer_state = Get<BUFFER_STATE>(pDecodeInfo->srcBuffer);
    if (buffer_state) {
        const char *where = " Buffer referenced in pDecodeInfo->srcBuffer";
        skip |= ValidateProtectedBuffer(*cb_state, *buffer_state, error_obj.location,
                                        "VUID-vkCmdDecodeVideoKHR-commandBuffer-07136", where);
        skip |= ValidateUnprotectedBuffer(*cb_state, *buffer_state, error_obj.location,
                                          "VUID-vkCmdDecodeVideoKHR-commandBuffer-07137", where);
    }

    if ((buffer_state->usage & VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR) == 0) {
        LogObjectList objlist(commandBuffer);
        objlist.add(vs_state->videoSession());
        objlist.add(pDecodeInfo->srcBuffer);
        skip |= LogError(objlist, "VUID-VkVideoDecodeInfoKHR-srcBuffer-07165",
                         "vkCmdDecodeVideoKHR(): pDecodeInfo->srcBuffer (%s) was not created with "
                         "VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR",
                         FormatHandle(pDecodeInfo->srcBuffer).c_str());
    }

    const auto &src_supported_profiles = buffer_state->supported_video_profiles;
    if (src_supported_profiles.find(vs_state->profile) == src_supported_profiles.end()) {
        LogObjectList objlist(commandBuffer);
        objlist.add(vs_state->videoSession());
        objlist.add(pDecodeInfo->srcBuffer);
        skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07135",
                         "vkCmdDecodeVideoKHR(): pDecodeInfo->srcBuffer (%s) is not compatible "
                         "with the video profile %s was created with",
                         FormatHandle(pDecodeInfo->srcBuffer).c_str(), FormatHandle(vs_state->videoSession()).c_str());
    }

    if (pDecodeInfo->srcBufferOffset >= buffer_state->createInfo.size) {
        LogObjectList objlist(commandBuffer);
        objlist.add(vs_state->videoSession());
        objlist.add(pDecodeInfo->srcBuffer);
        skip |= LogError(objlist, "VUID-VkVideoDecodeInfoKHR-srcBufferOffset-07166",
                         "vkCmdDecodeVideoKHR(): pDecodeInfo->srcBufferOffset (%" PRIu64 ") must be less than the size (%" PRIu64
                         ") of pDecodeInfo->srcBuffer (%s)",
                         pDecodeInfo->srcBufferOffset, buffer_state->createInfo.size, FormatHandle(pDecodeInfo->srcBuffer).c_str());
    }

    if (!IsIntegerMultipleOf(pDecodeInfo->srcBufferOffset, profile_caps.base.minBitstreamBufferOffsetAlignment)) {
        LogObjectList objlist(commandBuffer);
        objlist.add(vs_state->videoSession());
        skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07138",
                         "vkCmdDecodeVideoKHR(): pDecodeInfo->srcBufferOffset (%" PRIu64
                         ") is not an integer multiple of the minBitstreamBufferOffsetAlignment (%" PRIu64
                         ") required by the video profile %s was created with",
                         pDecodeInfo->srcBufferOffset, profile_caps.base.minBitstreamBufferOffsetAlignment,
                         FormatHandle(vs_state->videoSession()).c_str());
    }

    if (pDecodeInfo->srcBufferOffset + pDecodeInfo->srcBufferRange > buffer_state->createInfo.size) {
        LogObjectList objlist(commandBuffer);
        objlist.add(vs_state->videoSession());
        objlist.add(pDecodeInfo->srcBuffer);
        skip |=
            LogError(objlist, "VUID-VkVideoDecodeInfoKHR-srcBufferRange-07167",
                     "vkCmdDecodeVideoKHR(): pDecodeInfo->srcBufferOffset (%" PRIu64 ") plus pDecodeInfo->srcBufferRange (%" PRIu64
                     ") must be less than or equal to the size (%" PRIu64 ") of pDecodeInfo->srcBuffer (%s)",
                     pDecodeInfo->srcBufferOffset, pDecodeInfo->srcBufferRange, buffer_state->createInfo.size,
                     FormatHandle(pDecodeInfo->srcBuffer).c_str());
    }

    if (!IsIntegerMultipleOf(pDecodeInfo->srcBufferRange, profile_caps.base.minBitstreamBufferSizeAlignment)) {
        LogObjectList objlist(commandBuffer);
        objlist.add(vs_state->videoSession());
        skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07139",
                         "vkCmdDecodeVideoKHR(): pDecodeInfo->srcBufferRange (%" PRIu64
                         ") is not an integer multiple of the minBitstreamBufferSizeAlignment (%" PRIu64
                         ") required by the video profile %s was created with",
                         pDecodeInfo->srcBufferRange, profile_caps.base.minBitstreamBufferSizeAlignment,
                         FormatHandle(vs_state->videoSession()).c_str());
    }

    VideoPictureResource setup_resource;
    if (pDecodeInfo->pSetupReferenceSlot) {
        if (pDecodeInfo->pSetupReferenceSlot->slotIndex < 0) {
            skip |= LogError(commandBuffer, "VUID-VkVideoDecodeInfoKHR-pSetupReferenceSlot-07168",
                             "vkCmdDecodeVideoKHR(): pBeginInfo->pSetupReferenceSlot->slotIndex (%d) "
                             "must not be negative",
                             pDecodeInfo->pSetupReferenceSlot->slotIndex);
        } else if ((uint32_t)pDecodeInfo->pSetupReferenceSlot->slotIndex >= vs_state->create_info.maxDpbSlots) {
            skip |= LogError(commandBuffer, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07170",
                             "vkCmdDecodeVideoKHR(): pBeginInfo->pSetupReferenceSlot->slotIndex (%d) "
                             "must be smaller than the maxDpbSlots (%u) the bound video session %s "
                             "was created with",
                             pDecodeInfo->pSetupReferenceSlot->slotIndex, vs_state->create_info.maxDpbSlots,
                             FormatHandle(vs_state->videoSession()).c_str());
        }

        if (pDecodeInfo->pSetupReferenceSlot->pPictureResource != nullptr) {
            setup_resource = VideoPictureResource(this, *pDecodeInfo->pSetupReferenceSlot->pPictureResource);
            if (setup_resource) {
                skip |= ValidateVideoPictureResource(setup_resource, commandBuffer, *vs_state, "vkCmdDecodeVideoKHR()",
                                                     " Image referenced in pDecodeInfo->pSetupReferenceSlot",
                                                     "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07173");

                if (bound_resources.find(setup_resource) == bound_resources.end()) {
                    skip |= LogError(commandBuffer, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07149",
                                     "vkCmdDecodeVideoKHR(): the video picture resource specified in "
                                     "pDecodeInfo->pSetupReferenceSlot->pPictureResource is not one of the "
                                     "bound video picture resources");
                }

                skip |= VerifyImageLayout(*cb_state, *setup_resource.image_state, setup_resource.range,
                                          VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, error_obj.location,
                                          "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07254", &hit_error);
            }
        } else {
            skip |= LogError(commandBuffer, "VUID-VkVideoDecodeInfoKHR-pSetupReferenceSlot-07169",
                             "vkCmdDecodeVideoKHR(): pBeginInfo->pSetupReferenceSlot->pPictureResource "
                             "must not be NULL");
        }
    }

    auto dst_resource = VideoPictureResource(this, pDecodeInfo->dstPictureResource);
    skip |= ValidateVideoPictureResource(
        dst_resource, commandBuffer, *vs_state, "vkCmdDecodeVideoKHR()", " Image referenced in pDecodeInfo->dstPictureResource",
        "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07144", "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07145");
    if (dst_resource) {
        const char *where = " Image referenced in pDecodeInfo->dstPictureResource";
        skip |= ValidateProtectedImage(*cb_state, *dst_resource.image_state, error_obj.location,
                                       "VUID-vkCmdDecodeVideoKHR-commandBuffer-07147", where);
        skip |= ValidateUnprotectedImage(*cb_state, *dst_resource.image_state, error_obj.location,
                                         "VUID-vkCmdDecodeVideoKHR-commandBuffer-07148", where);

        const auto &dst_supported_profiles = dst_resource.image_state->supported_video_profiles;
        if (dst_supported_profiles.find(vs_state->profile) == dst_supported_profiles.end()) {
            LogObjectList objlist(commandBuffer);
            objlist.add(vs_state->videoSession());
            objlist.add(dst_resource.image_view_state->image_view());
            objlist.add(dst_resource.image_state->image());
            skip |=
                LogError(objlist, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07142",
                         "vkCmdDecodeVideoKHR(): pDecodeInfo->dstPictureResource.imageViewBinding "
                         "(%s created from %s) is not compatible with the video profile the bound "
                         "video session %s was created with",
                         FormatHandle(pDecodeInfo->dstPictureResource.imageViewBinding).c_str(),
                         FormatHandle(dst_resource.image_state->image()).c_str(), FormatHandle(vs_state->videoSession()).c_str());
        }

        if (dst_resource.image_view_state->create_info.format != vs_state->create_info.pictureFormat) {
            LogObjectList objlist(commandBuffer);
            objlist.add(vs_state->videoSession());
            objlist.add(dst_resource.image_view_state->image_view());
            objlist.add(dst_resource.image_state->image());
            skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07143",
                             "vkCmdDecodeVideoKHR(): pDecodeInfo->dstPictureResource.imageViewBinding "
                             "(%s created from %s) format (%s) does not match the pictureFormat (%s) "
                             "the bound video session %s was created with",
                             FormatHandle(dst_resource.image_view_state->image_view()).c_str(),
                             FormatHandle(dst_resource.image_state->image()).c_str(),
                             string_VkFormat(dst_resource.image_view_state->create_info.format),
                             string_VkFormat(vs_state->create_info.pictureFormat), FormatHandle(vs_state->videoSession()).c_str());
        }

        auto supported_usage = dst_resource.image_view_state->inherited_usage;
        if ((supported_usage & VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR) == 0) {
            LogObjectList objlist(commandBuffer);
            objlist.add(vs_state->videoSession());
            objlist.add(dst_resource.image_view_state->image_view());
            objlist.add(dst_resource.image_state->image());
            skip |=
                LogError(objlist, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07146",
                         "vkCmdDecodeVideoKHR(): pDecodeInfo->dstPictureResource.imageViewBinding "
                         "(%s created from %s) was not created with VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR "
                         "thus it cannot be used as a decode output picture with the bound video session %s "
                         "that was created with a decode operation",
                         FormatHandle(dst_resource.image_view_state->image_view()).c_str(),
                         FormatHandle(dst_resource.image_state->image()).c_str(), FormatHandle(vs_state->videoSession()).c_str());
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
                LogObjectList objlist(commandBuffer);
                objlist.add(vs_state->videoSession());
                skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07140",
                                 "vkCmdDecodeVideoKHR(): the video profile %s was created with does not support "
                                 "VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR but "
                                 "pDecodeInfo->dstPictureResource and "
                                 "pDecodeInfo->pSetupReferenceSlot->pPictureResource match",
                                 FormatHandle(vs_state->videoSession()).c_str());
            }

            if ((profile_caps.decode.flags & VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR) == 0 &&
                !dst_same_as_setup) {
                LogObjectList objlist(commandBuffer);
                objlist.add(vs_state->videoSession());
                skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07141",
                                 "vkCmdDecodeVideoKHR(): the video profile %s was created with does not support "
                                 "VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR but "
                                 "pDecodeInfo->dstPictureResource and pSetupReferenceSlot->pPictureResource "
                                 "do not match",
                                 FormatHandle(vs_state->videoSession()).c_str());
            }
        }
    }

    if (pDecodeInfo->pReferenceSlots) {
        VideoPictureResources unique_resources{};
        bool resources_unique = true;
        char where[64];

        skip |= ValidateActiveReferencePictureCount(*cb_state, *pDecodeInfo);
        skip |= ValidateReferencePictureUseCount(*cb_state, *pDecodeInfo);

        for (uint32_t i = 0; i < pDecodeInfo->referenceSlotCount; ++i) {
            if (pDecodeInfo->pReferenceSlots[i].slotIndex < 0) {
                skip |= LogError(commandBuffer, "VUID-VkVideoDecodeInfoKHR-slotIndex-07171",
                                 "vkCmdDecodeVideoKHR(): pDecodeInfo->pReferenceSlots[%u].slotIndex (%d) "
                                 "must not be negative",
                                 i, pDecodeInfo->pReferenceSlots[i].slotIndex);
            } else if ((uint32_t)pDecodeInfo->pReferenceSlots[i].slotIndex >= vs_state->create_info.maxDpbSlots) {
                LogObjectList objlist(commandBuffer);
                objlist.add(vs_state->videoSession());
                skip |= LogError(objlist, "VUID-vkCmdDecodeVideoKHR-slotIndex-07256",
                                 "vkCmdDecodeVideoKHR(): pDecodeInfo->pReferenceSlots[%u].slotIndex (%d) "
                                 "must be smaller than the maxDpbSlots (%u) the bound video session %s "
                                 "was created with",
                                 i, pDecodeInfo->pReferenceSlots[i].slotIndex, vs_state->create_info.maxDpbSlots,
                                 FormatHandle(vs_state->videoSession()).c_str());
            }

            if (pDecodeInfo->pReferenceSlots[i].pPictureResource != nullptr) {
                auto reference_resource = VideoPictureResource(this, *pDecodeInfo->pReferenceSlots[i].pPictureResource);
                if (reference_resource) {
                    if (!unique_resources.emplace(reference_resource).second) {
                        resources_unique = false;
                    }

                    const auto &it = bound_resources.find(reference_resource);
                    if (it == bound_resources.end()) {
                        skip |= LogError(commandBuffer, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151",
                                         "vkCmdDecodeVideoKHR(): the video picture resource specified in "
                                         "pDecodeInfo->pReferenceSlots[%u].pPictureResource is not one of the "
                                         "bound video picture resources",
                                         i);
                    } else if (pDecodeInfo->pReferenceSlots[i].slotIndex >= 0 &&
                               pDecodeInfo->pReferenceSlots[i].slotIndex != it->second) {
                        skip |= LogError(commandBuffer, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07151",
                                         "vkCmdDecodeVideoKHR(): the bound video picture resource specified in "
                                         "pDecodeInfo->pReferenceSlots[%u].pPictureResource is not currently "
                                         "associated with the DPB slot index specifed in "
                                         "pDecodeInfo->pReferenceSlots[%u].slotIndex (%d)",
                                         i, i, pDecodeInfo->pReferenceSlots[i].slotIndex);
                    }

                    snprintf(where, sizeof(where), " Image referenced in pDecodeInfo->pReferenceSlots[%u]", i);
                    skip |= ValidateVideoPictureResource(reference_resource, commandBuffer, *vs_state, "vkCmdDecodeVideoKHR()",
                                                         where, "VUID-vkCmdDecodeVideoKHR-codedOffset-07257");

                    skip |= VerifyImageLayout(*cb_state, *reference_resource.image_state, reference_resource.range,
                                              VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR, error_obj.location,
                                              "VUID-vkCmdDecodeVideoKHR-pPictureResource-07255", &hit_error);
                }
            } else {
                skip |= LogError(commandBuffer, "VUID-VkVideoDecodeInfoKHR-pPictureResource-07172",
                                 "vkCmdDecodeVideoKHR(): pDecodeInfo->pReferenceSlots[%u].pPictureResource "
                                 "must not be NULL",
                                 i);
            }
        }

        if (!resources_unique) {
            skip |= LogError(commandBuffer, "VUID-vkCmdDecodeVideoKHR-pDecodeInfo-07264",
                             "vkCmdDecodeVideoKHR(): more than one element of pDecodeInfo->pReferenceSlots "
                             "refers to the same video picture resource");
        }
    }

    for (const auto &query : cb_state->activeQueries) {
        uint32_t op_count = vs_state->GetVideoDecodeOperationCount(pDecodeInfo);
        if (query.active_query_index + op_count > query.last_activatable_query_index + 1) {
            auto query_pool_state = Get<QUERY_POOL_STATE>(query.pool);
            skip |= LogError(commandBuffer, "VUID-vkCmdDecodeVideoKHR-opCount-07134",
                             "vkCmdDecodeVideoKHR(): not enough activatable queries for query type %s "
                             "with opCount %u, active query index %u, and last activatable query index %u",
                             string_VkQueryType(query_pool_state->createInfo.queryType), op_count, query.active_query_index,
                             query.last_activatable_query_index);
        }
    }

    switch (vs_state->GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
            skip |= ValidateVideoDecodeInfoH264(*cb_state, *pDecodeInfo);
            break;

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
            skip |= ValidateVideoDecodeInfoH265(*cb_state, *pDecodeInfo);
            break;

        default:
            break;
    }

    skip |= ValidateCmd(*cb_state, error_obj.location);
    return skip;
}
