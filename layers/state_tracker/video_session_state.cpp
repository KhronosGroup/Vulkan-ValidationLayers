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
#include "state_tracker/video_session_state.h"
#include "state_tracker/image_state.h"
#include "state_tracker/state_tracker.h"
#include "layer_chassis_dispatch.h"

VideoProfileDesc::VideoProfileDesc(const ValidationStateTracker *dev_data, VkVideoProfileInfoKHR const *profile)
    : std::enable_shared_from_this<VideoProfileDesc>(), profile_(), capabilities_(), cache_(nullptr) {
    if (InitProfile(profile)) {
        InitCapabilities(dev_data);
    }
}

VideoProfileDesc::~VideoProfileDesc() {
    if (cache_) {
        cache_->Release(this);
    }
}

bool VideoProfileDesc::InitProfile(VkVideoProfileInfoKHR const *profile) {
    if (profile) {
        profile_.base = *profile;
        profile_.base.pNext = nullptr;
        if (profile_.base.chromaSubsampling == VK_VIDEO_CHROMA_SUBSAMPLING_MONOCHROME_BIT_KHR) {
            // If monochrome, then chromaBitDepth is ignored, so let's set it to INVALID
            // to avoid special-casing the comparison and hash functions.
            profile_.base.chromaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_INVALID_KHR;
        }

        switch (profile->videoCodecOperation) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
                auto decode_h264 = LvlFindInChain<VkVideoDecodeH264ProfileInfoKHR>(profile->pNext);
                if (decode_h264) {
                    profile_.valid = true;
                    profile_.decode_h264 = *decode_h264;
                    profile_.decode_h264.pNext = nullptr;
                } else {
                    profile_.valid = false;
                    profile_.decode_h264 = LvlInitStruct<VkVideoDecodeH264ProfileInfoKHR>();
                }
                profile_.is_decode = true;
                profile_.base.pNext = &profile_.decode_h264;
                break;
            }
            case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
                auto decode_h265 = LvlFindInChain<VkVideoDecodeH265ProfileInfoKHR>(profile->pNext);
                if (decode_h265) {
                    profile_.valid = true;
                    profile_.decode_h265 = *decode_h265;
                    profile_.decode_h265.pNext = nullptr;
                } else {
                    profile_.valid = false;
                    profile_.decode_h265 = LvlInitStruct<VkVideoDecodeH265ProfileInfoKHR>();
                }
                profile_.is_decode = true;
                profile_.base.pNext = &profile_.decode_h265;
                break;
            }
            default:
                profile_.valid = false;
                break;
        }

        if (profile_.is_decode) {
            auto usage = LvlFindInChain<VkVideoDecodeUsageInfoKHR>(profile->pNext);
            if (usage) {
                profile_.decode_usage = *usage;
                profile_.decode_usage.pNext = profile_.base.pNext;
                profile_.base.pNext = &profile_.decode_usage;
            } else {
                profile_.decode_usage = LvlInitStruct<VkVideoDecodeUsageInfoKHR>();
            }
        }
    } else {
        profile_.valid = false;
        profile_.base = LvlInitStruct<VkVideoProfileInfoKHR>();
    }

    return profile_.valid;
}

void VideoProfileDesc::InitCapabilities(const ValidationStateTracker *dev_data) {
    capabilities_.base = LvlInitStruct<VkVideoCapabilitiesKHR>();
    switch (profile_.base.videoCodecOperation) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
            capabilities_.base.pNext = &capabilities_.decode;
            capabilities_.decode = LvlInitStruct<VkVideoDecodeCapabilitiesKHR>();
            capabilities_.decode.pNext = &capabilities_.decode_h264;
            capabilities_.decode_h264 = LvlInitStruct<VkVideoDecodeH264CapabilitiesKHR>();
            break;

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
            capabilities_.base.pNext = &capabilities_.decode;
            capabilities_.decode = LvlInitStruct<VkVideoDecodeCapabilitiesKHR>();
            capabilities_.decode.pNext = &capabilities_.decode_h265;
            capabilities_.decode_h265 = LvlInitStruct<VkVideoDecodeH265CapabilitiesKHR>();
            break;

        default:
            return;
    }

    VkResult result = DispatchGetPhysicalDeviceVideoCapabilitiesKHR(dev_data->physical_device, &profile_.base, &capabilities_.base);
    if (result == VK_SUCCESS) {
        capabilities_.supported = true;
    }
}

std::shared_ptr<const VideoProfileDesc> VideoProfileDesc::Cache::GetOrCreate(const ValidationStateTracker *dev_data,
                                                                             VkVideoProfileInfoKHR const *profile) {
    VideoProfileDesc desc(dev_data, profile);
    if (desc.GetProfile().valid) {
        auto it = set_.find(&desc);
        if (it != set_.end()) {
            return (*it)->shared_from_this();
        } else {
            auto desc_ptr = std::make_shared<VideoProfileDesc>(desc);
            desc_ptr->cache_ = this;
            set_.emplace(desc_ptr.get());
            return desc_ptr;
        }
    } else {
        return nullptr;
    }
}

std::shared_ptr<const VideoProfileDesc> VideoProfileDesc::Cache::Get(const ValidationStateTracker *dev_data,
                                                                     VkVideoProfileInfoKHR const *profile) {
    if (profile) {
        std::unique_lock<std::mutex> lock(mutex_);
        return GetOrCreate(dev_data, profile);
    } else {
        return nullptr;
    }
}

SupportedVideoProfiles VideoProfileDesc::Cache::Get(const ValidationStateTracker *dev_data,
                                                    VkVideoProfileListInfoKHR const *profile_list) {
    SupportedVideoProfiles supported_profiles{};
    if (profile_list) {
        std::unique_lock<std::mutex> lock(mutex_);
        for (uint32_t i = 0; i < profile_list->profileCount; ++i) {
            auto profile_desc = GetOrCreate(dev_data, &profile_list->pProfiles[i]);
            if (profile_desc) {
                supported_profiles.insert(std::move(profile_desc));
            }
        }
    }
    return supported_profiles;
}

void VideoProfileDesc::Cache::Release(VideoProfileDesc const *desc) {
    std::unique_lock<std::mutex> lock(mutex_);
    set_.erase(desc);
}

VideoPictureResource::VideoPictureResource()
    : image_view_state(nullptr), image_state(nullptr), base_array_layer(0), range(), coded_offset(), coded_extent() {}

VideoPictureResource::VideoPictureResource(ValidationStateTracker const *dev_data, VkVideoPictureResourceInfoKHR const &res)
    : image_view_state(dev_data->Get<IMAGE_VIEW_STATE>(res.imageViewBinding)),
      image_state(image_view_state ? image_view_state->image_state : nullptr),
      base_array_layer(res.baseArrayLayer),
      range(GetImageSubresourceRange(image_view_state.get(), res.baseArrayLayer)),
      coded_offset(res.codedOffset),
      coded_extent(res.codedExtent) {}

VkImageSubresourceRange VideoPictureResource::GetImageSubresourceRange(IMAGE_VIEW_STATE const *image_view_state, uint32_t layer) {
    VkImageSubresourceRange range{};
    if (image_view_state) {
        range = image_view_state->normalized_subresource_range;
        range.baseArrayLayer += layer;
    }
    return range;
}

VideoPictureID::VideoPictureID(VideoProfileDesc const &profile, VkVideoReferenceSlotInfoKHR const &slot) {
    switch (profile.GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
            auto slot_info = LvlFindInChain<VkVideoDecodeH264DpbSlotInfoKHR>(slot.pNext);
            if (slot_info && slot_info->pStdReferenceInfo) {
                top_field = slot_info->pStdReferenceInfo->flags.top_field_flag;
                bottom_field = slot_info->pStdReferenceInfo->flags.bottom_field_flag;
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
            break;

        default:
            break;
    }
}

void VideoSessionDeviceState::Reset() {
    initialized_ = true;
    for (size_t i = 0; i < is_active_.size(); ++i) {
        is_active_[i] = false;
        all_pictures_[i].clear();
        pictures_per_id_[i].clear();
    }
}

void VideoSessionDeviceState::Activate(int32_t slot_index, const VideoPictureID &picture_id, const VideoPictureResource &res) {
    assert(!picture_id.IsBothFields());

    is_active_[slot_index] = true;

    if (picture_id.IsFrame()) {
        // If slot is activated with a frame then it overrides all previous pictures
        all_pictures_[slot_index].clear();
        pictures_per_id_[slot_index].clear();
    }

    auto prev_res_it = pictures_per_id_[slot_index].find(picture_id);
    if (prev_res_it != pictures_per_id_[slot_index].end()) {
        // If we replace an existing picture then remove it
        all_pictures_[slot_index].erase(prev_res_it->second);
    }

    all_pictures_[slot_index].insert(res);
    pictures_per_id_[slot_index][picture_id] = res;
}

void VideoSessionDeviceState::Deactivate(int32_t slot_index) {
    is_active_[slot_index] = false;
    all_pictures_[slot_index].clear();
    pictures_per_id_[slot_index].clear();
}

VIDEO_SESSION_STATE::VIDEO_SESSION_STATE(ValidationStateTracker *dev_data, VkVideoSessionKHR vs,
                                         VkVideoSessionCreateInfoKHR const *pCreateInfo,
                                         std::shared_ptr<const VideoProfileDesc> &&profile_desc)
    : BASE_NODE(vs, kVulkanObjectTypeVideoSessionKHR),
      create_info(pCreateInfo),
      profile(std::move(profile_desc)),
      memory_binding_count_queried(false),
      memory_bindings_queried(0),
      memory_bindings_(GetMemoryBindings(dev_data, vs)),
      unbound_memory_binding_count_(static_cast<uint32_t>(memory_bindings_.size())),
      device_state_mutex_(),
      device_state_(pCreateInfo->maxDpbSlots) {}

VIDEO_SESSION_STATE::MemoryBindingMap VIDEO_SESSION_STATE::GetMemoryBindings(ValidationStateTracker *dev_data,
                                                                             VkVideoSessionKHR vs) {
    uint32_t memory_requirement_count;
    DispatchGetVideoSessionMemoryRequirementsKHR(dev_data->device, vs, &memory_requirement_count, nullptr);

    std::vector<VkVideoSessionMemoryRequirementsKHR> memory_requirements(memory_requirement_count,
                                                                         LvlInitStruct<VkVideoSessionMemoryRequirementsKHR>());
    DispatchGetVideoSessionMemoryRequirementsKHR(dev_data->device, vs, &memory_requirement_count, memory_requirements.data());

    MemoryBindingMap memory_bindings;
    for (uint32_t i = 0; i < memory_requirement_count; ++i) {
        memory_bindings[memory_requirements[i].memoryBindIndex].requirements = memory_requirements[i].memoryRequirements;
    }

    return memory_bindings;
}

VIDEO_SESSION_PARAMETERS_STATE::VIDEO_SESSION_PARAMETERS_STATE(VkVideoSessionParametersKHR vsp,
                                                               VkVideoSessionParametersCreateInfoKHR const *pCreateInfo,
                                                               std::shared_ptr<VIDEO_SESSION_STATE> &&vsstate,
                                                               std::shared_ptr<VIDEO_SESSION_PARAMETERS_STATE> &&vsp_template)
    : BASE_NODE(vsp, kVulkanObjectTypeVideoSessionParametersKHR), createInfo(pCreateInfo), vs_state(vsstate), mutex_(), data_() {
    data_.update_sequence_counter = 0;

    switch (vs_state->GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
            const auto decode_h264 = LvlFindInChain<VkVideoDecodeH264SessionParametersCreateInfoKHR>(createInfo.pNext);
            if (vsp_template) {
                auto template_data = vsp_template->Lock();
                data_.h264.sps = template_data->h264.sps;
                data_.h264.pps = template_data->h264.pps;
            }
            if (decode_h264->pParametersAddInfo) {
                AddDecodeH264(decode_h264->pParametersAddInfo);
            }
            data_.h264.spsCapacity = decode_h264->maxStdSPSCount;
            data_.h264.ppsCapacity = decode_h264->maxStdPPSCount;
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
            const auto decode_h265 = LvlFindInChain<VkVideoDecodeH265SessionParametersCreateInfoKHR>(createInfo.pNext);
            if (vsp_template) {
                auto template_data = vsp_template->Lock();
                data_.h265.vps = template_data->h265.vps;
                data_.h265.sps = template_data->h265.sps;
                data_.h265.pps = template_data->h265.pps;
            }
            if (decode_h265->pParametersAddInfo) {
                AddDecodeH265(decode_h265->pParametersAddInfo);
            }
            data_.h265.vpsCapacity = decode_h265->maxStdVPSCount;
            data_.h265.spsCapacity = decode_h265->maxStdSPSCount;
            data_.h265.ppsCapacity = decode_h265->maxStdPPSCount;
            break;
        }

        default:
            break;
    }
}

void VIDEO_SESSION_PARAMETERS_STATE::Update(VkVideoSessionParametersUpdateInfoKHR const *info) {
    auto lock = Lock();

    data_.update_sequence_counter = info->updateSequenceCount;

    switch (vs_state->GetCodecOp()) {
        case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
            auto add_info = LvlFindInChain<VkVideoDecodeH264SessionParametersAddInfoKHR>(info->pNext);
            if (add_info) {
                AddDecodeH264(add_info);
            }
            break;
        }

        case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
            auto add_info = LvlFindInChain<VkVideoDecodeH265SessionParametersAddInfoKHR>(info->pNext);
            if (add_info) {
                AddDecodeH265(add_info);
            }
            break;
        }

        default:
            break;
    }
}

void VIDEO_SESSION_PARAMETERS_STATE::AddDecodeH264(VkVideoDecodeH264SessionParametersAddInfoKHR const *info) {
    for (uint32_t i = 0; i < info->stdSPSCount; ++i) {
        const auto &entry = info->pStdSPSs[i];
        data_.h264.sps[GetH264SPSKey(entry)] = entry;
    }
    for (uint32_t i = 0; i < info->stdPPSCount; ++i) {
        const auto &entry = info->pStdPPSs[i];
        data_.h264.pps[GetH264PPSKey(entry)] = entry;
    }
}

void VIDEO_SESSION_PARAMETERS_STATE::AddDecodeH265(VkVideoDecodeH265SessionParametersAddInfoKHR const *info) {
    for (uint32_t i = 0; i < info->stdVPSCount; ++i) {
        const auto &entry = info->pStdVPSs[i];
        data_.h265.vps[GetH265VPSKey(entry)] = entry;
    }
    for (uint32_t i = 0; i < info->stdSPSCount; ++i) {
        const auto &entry = info->pStdSPSs[i];
        data_.h265.sps[GetH265SPSKey(entry)] = entry;
    }
    for (uint32_t i = 0; i < info->stdPPSCount; ++i) {
        const auto &entry = info->pStdPPSs[i];
        data_.h265.pps[GetH265PPSKey(entry)] = entry;
    }
}
