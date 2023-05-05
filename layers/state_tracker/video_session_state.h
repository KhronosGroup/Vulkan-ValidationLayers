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
#pragma once

#include "state_tracker/base_node.h"
#include "utils/hash_util.h"
#include "generated/vk_safe_struct.h"
#include <memory>
#include <mutex>
#include <vector>
#include <functional>

class ValidationStateTracker;
class IMAGE_STATE;
class IMAGE_VIEW_STATE;

using SupportedVideoProfiles = vvl::unordered_set<std::shared_ptr<const class VideoProfileDesc>>;

// The VideoProfileDesc contains the entire video profile description, which includes all
// parameters specified in VkVideoProfileInfoKHR and its pNext chain. This includes any
// parameters specific to the coding operation type (e.g. decode/encode usage hints/modes)
// and the codec. Accordingly, hashing and comparison takes into consideration all the
// relevant parameters.
class VideoProfileDesc : public std::enable_shared_from_this<VideoProfileDesc> {
  public:
    struct Profile {
        bool valid;
        bool is_decode;
        VkVideoProfileInfoKHR base;
        union {
            VkVideoDecodeUsageInfoKHR decode_usage;
        };
        union {
            VkVideoDecodeH264ProfileInfoKHR decode_h264;
            VkVideoDecodeH265ProfileInfoKHR decode_h265;
        };
    };

    struct Capabilities {
        bool supported;
        VkVideoCapabilitiesKHR base;
        union {
            VkVideoDecodeCapabilitiesKHR decode;
        };
        union {
            VkVideoDecodeH264CapabilitiesKHR decode_h264;
            VkVideoDecodeH265CapabilitiesKHR decode_h265;
        };
    };

    VideoProfileDesc(const ValidationStateTracker *dev_data, VkVideoProfileInfoKHR const *profile);
    ~VideoProfileDesc();

    const Profile &GetProfile() const { return profile_; }
    const Capabilities &GetCapabilities() const { return capabilities_; }

    VkVideoCodecOperationFlagBitsKHR GetCodecOp() const { return profile_.base.videoCodecOperation; }
    VkVideoDecodeH264PictureLayoutFlagBitsKHR GetH264PictureLayout() const { return profile_.decode_h264.pictureLayout; }

    struct compare {
      public:
        bool operator()(VideoProfileDesc const *lhs, VideoProfileDesc const *rhs) const {
            bool match = lhs->profile_.base.videoCodecOperation == rhs->profile_.base.videoCodecOperation &&
                         lhs->profile_.base.chromaSubsampling == rhs->profile_.base.chromaSubsampling &&
                         lhs->profile_.base.lumaBitDepth == rhs->profile_.base.lumaBitDepth &&
                         lhs->profile_.base.chromaBitDepth == rhs->profile_.base.chromaBitDepth;

            if (match && lhs->profile_.is_decode) {
                match = match && lhs->profile_.decode_usage.videoUsageHints == rhs->profile_.decode_usage.videoUsageHints;
            }

            if (match) {
                switch (lhs->profile_.base.videoCodecOperation) {
                    case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
                        match = match && lhs->profile_.decode_h264.stdProfileIdc == rhs->profile_.decode_h264.stdProfileIdc &&
                                lhs->profile_.decode_h264.pictureLayout == rhs->profile_.decode_h264.pictureLayout;
                        break;

                    case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
                        match = match && lhs->profile_.decode_h265.stdProfileIdc == rhs->profile_.decode_h265.stdProfileIdc;
                        break;

                    default:
                        break;
                }
            }

            return match;
        }
    };

    struct hash {
      public:
        std::size_t operator()(VideoProfileDesc const *desc) const {
            hash_util::HashCombiner hc;
            hc << desc->profile_.base.videoCodecOperation << desc->profile_.base.chromaSubsampling
               << desc->profile_.base.lumaBitDepth << desc->profile_.base.chromaBitDepth;

            if (desc->profile_.is_decode) {
                hc << desc->profile_.decode_usage.videoUsageHints;
            }

            switch (desc->profile_.base.videoCodecOperation) {
                case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
                    hc << desc->profile_.decode_h264.stdProfileIdc << desc->profile_.decode_h264.pictureLayout;
                    break;
                }
                case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR: {
                    hc << desc->profile_.decode_h265.stdProfileIdc;
                    break;
                }
                default:
                    break;
            }
            return hc.Value();
        }
    };

    // The cache maintains non-owning references to all VideoProfileDesc objects that are referred to by shared
    // pointers of VideoProfileDesc owned by any object.
    // This ensured that every VideoProfileDesc object has only a unique instance acquired from the cache using the
    // Get() method and thus enable us to compare video profiles through a single pointer comparison instead of a
    // deep compare of the structure chains.
    // Once all shared pointers to a VideoProfileDesc go away the destructor will call Release() to remove the
    // non-owning reference to the to-be-deleted object.
    class Cache {
      public:
        std::shared_ptr<const VideoProfileDesc> Get(const ValidationStateTracker *dev_data, VkVideoProfileInfoKHR const *profile);
        SupportedVideoProfiles Get(const ValidationStateTracker *dev_data, VkVideoProfileListInfoKHR const *profile_list);
        void Release(VideoProfileDesc const *desc);

      private:
        std::mutex mutex_;
        vvl::unordered_set<VideoProfileDesc const *, VideoProfileDesc::hash, VideoProfileDesc::compare> set_;

        std::shared_ptr<const VideoProfileDesc> GetOrCreate(const ValidationStateTracker *dev_data,
                                                            VkVideoProfileInfoKHR const *profile);
    };

  private:
    Profile profile_;
    Capabilities capabilities_;
    Cache *cache_;

    bool InitProfile(VkVideoProfileInfoKHR const *profile);
    void InitCapabilities(const ValidationStateTracker *dev_data);
};

class VideoPictureResource {
  public:
    std::shared_ptr<const IMAGE_VIEW_STATE> image_view_state;
    std::shared_ptr<const IMAGE_STATE> image_state;
    uint32_t base_array_layer;
    VkImageSubresourceRange range;
    VkOffset2D coded_offset;
    VkExtent2D coded_extent;

    VideoPictureResource();
    VideoPictureResource(ValidationStateTracker const *dev_data, VkVideoPictureResourceInfoKHR const &res);

    operator bool() const { return image_view_state != nullptr; }

    bool operator==(VideoPictureResource const &rhs) const {
        return image_state == rhs.image_state && range.baseMipLevel == rhs.range.baseMipLevel &&
               range.baseArrayLayer == rhs.range.baseArrayLayer && coded_offset.x == rhs.coded_offset.x &&
               coded_offset.y == rhs.coded_offset.y && coded_extent.width == rhs.coded_extent.width &&
               coded_extent.height == rhs.coded_extent.height;
    }

    struct hash {
      public:
        std::size_t operator()(VideoPictureResource const &res) const {
            hash_util::HashCombiner hc;
            hc << res.image_state.get() << res.range.baseMipLevel << res.range.baseArrayLayer << res.coded_offset.x
               << res.coded_offset.y << res.coded_extent.width << res.coded_extent.height;
            return hc.Value();
        }
    };

  private:
    VkImageSubresourceRange GetImageSubresourceRange(IMAGE_VIEW_STATE const *image_view_state, uint32_t layer);
};

using VideoPictureResources = vvl::unordered_set<VideoPictureResource, VideoPictureResource::hash>;
using BoundVideoPictureResources = vvl::unordered_map<VideoPictureResource, int32_t, VideoPictureResource::hash>;

struct VideoPictureID {
    // Used by H.264 to indicate it's a top field picture
    bool top_field = false;
    // Used by H.264 to indicate it's a bottom field picture
    bool bottom_field = false;

    VideoPictureID() {}
    VideoPictureID(VideoProfileDesc const &profile, VkVideoReferenceSlotInfoKHR const &slot);

    static VideoPictureID Frame() {
        VideoPictureID id{};
        return id;
    }

    static VideoPictureID TopField() {
        VideoPictureID id{};
        id.top_field = true;
        return id;
    }

    static VideoPictureID BottomField() {
        VideoPictureID id{};
        id.bottom_field = true;
        return id;
    }

    bool IsFrame() const { return !top_field && !bottom_field; }
    bool IsTopField() const { return top_field && !bottom_field; }
    bool IsBottomField() const { return !top_field && bottom_field; }
    bool IsBothFields() const { return top_field && bottom_field; }
    bool ContainsTopField() const { return top_field; }
    bool ContainsBottomField() const { return bottom_field; }

    bool operator==(VideoPictureID const &rhs) const { return top_field == rhs.top_field && bottom_field == rhs.bottom_field; }

    struct hash {
      public:
        std::size_t operator()(VideoPictureID const &id) const {
            hash_util::HashCombiner hc;
            hc << id.top_field << id.bottom_field;
            return hc.Value();
        }
    };
};

struct VideoReferenceSlot {
    int32_t index;
    VideoPictureID picture_id;
    VideoPictureResource resource;

    VideoReferenceSlot() : index(-1), picture_id(), resource() {}

    VideoReferenceSlot(ValidationStateTracker const *dev_data, VideoProfileDesc const &profile,
                       VkVideoReferenceSlotInfoKHR const &slot, bool has_picture_id = true)
        : index(slot.slotIndex),
          picture_id(has_picture_id ? VideoPictureID(profile, slot) : VideoPictureID()),
          resource(slot.pPictureResource ? VideoPictureResource(dev_data, *slot.pPictureResource) : VideoPictureResource()) {}

    // The reference is only valid if it refers to a valid DPB index and resource
    operator bool() const { return index >= 0 && resource; }
};

class VideoSessionDeviceState {
  public:
    VideoSessionDeviceState(uint32_t reference_slot_count = 0)
        : initialized_(false),
          is_active_(reference_slot_count, false),
          all_pictures_(reference_slot_count),
          pictures_per_id_(reference_slot_count) {}

    bool IsInitialized() const { return initialized_; }
    bool IsSlotActive(int32_t slot_index) const { return is_active_[slot_index]; }

    bool IsSlotPicture(int32_t slot_index, const VideoPictureResource &res) const {
        return all_pictures_[slot_index].find(res) != all_pictures_[slot_index].end();
    }

    virtual bool IsSlotPicture(int32_t slot_index, const VideoPictureID &picture_id, const VideoPictureResource &res) const {
        auto it = pictures_per_id_[slot_index].find(picture_id);
        return it != pictures_per_id_[slot_index].end() && it->second == res;
    }

    void Reset();
    void Activate(int32_t slot_index, const VideoPictureID &picture_id, const VideoPictureResource &res);
    void Deactivate(int32_t slot_index);

  private:
    bool initialized_;
    std::vector<bool> is_active_;
    std::vector<VideoPictureResources> all_pictures_;
    std::vector<vvl::unordered_map<VideoPictureID, VideoPictureResource, VideoPictureID::hash>> pictures_per_id_;
};

class VIDEO_SESSION_STATE : public BASE_NODE {
  public:
    struct MemoryBindingInfo {
        VkMemoryRequirements requirements;
        bool bound;
    };
    using MemoryBindingMap = vvl::unordered_map<uint32_t, MemoryBindingInfo>;

    const safe_VkVideoSessionCreateInfoKHR create_info;
    std::shared_ptr<const VideoProfileDesc> profile;
    bool memory_binding_count_queried;
    uint32_t memory_bindings_queried;

    class DeviceStateWriter {
      public:
        DeviceStateWriter(VIDEO_SESSION_STATE *state) : lock_(state->device_state_mutex_), state_(state->device_state_) {}
        VideoSessionDeviceState &operator*() { return state_; }

      private:
        std::unique_lock<std::mutex> lock_;
        VideoSessionDeviceState &state_;
    };

    VIDEO_SESSION_STATE(ValidationStateTracker *dev_data, VkVideoSessionKHR vs, VkVideoSessionCreateInfoKHR const *pCreateInfo,
                        std::shared_ptr<const VideoProfileDesc> &&profile_desc);

    VkVideoSessionKHR videoSession() const { return handle_.Cast<VkVideoSessionKHR>(); }
    VkVideoCodecOperationFlagBitsKHR GetCodecOp() const { return profile->GetCodecOp(); }

    VkVideoDecodeH264PictureLayoutFlagBitsKHR GetH264PictureLayout() const { return profile->GetH264PictureLayout(); }

    VideoSessionDeviceState DeviceStateCopy() const {
        std::unique_lock<std::mutex> lock(device_state_mutex_);
        return device_state_;
    }

    DeviceStateWriter DeviceStateWrite() { return DeviceStateWriter(this); }

    const MemoryBindingInfo *GetMemoryBindingInfo(uint32_t index) const {
        auto it = memory_bindings_.find(index);
        if (it != memory_bindings_.end()) {
            return &it->second;
        } else {
            return nullptr;
        }
    }

    uint32_t GetMemoryBindingCount() const { return (uint32_t)memory_bindings_.size(); }
    uint32_t GetUnboundMemoryBindingCount() const { return unbound_memory_binding_count_; }

    void BindMemoryBindingIndex(uint32_t index) {
        auto it = memory_bindings_.find(index);
        if (it != memory_bindings_.end() && !it->second.bound) {
            it->second.bound = true;
            --unbound_memory_binding_count_;
        }
    }

    uint32_t GetVideoDecodeOperationCount(VkVideoDecodeInfoKHR const *) { return 1; }

  private:
    MemoryBindingMap GetMemoryBindings(ValidationStateTracker *dev_data, VkVideoSessionKHR vs);

    MemoryBindingMap memory_bindings_;
    uint32_t unbound_memory_binding_count_;

    mutable std::mutex device_state_mutex_;
    VideoSessionDeviceState device_state_;
};

class VIDEO_SESSION_PARAMETERS_STATE : public BASE_NODE {
  public:
    using H264SPSKey = uint8_t;
    using H264PPSKey = uint16_t;
    using H265VPSKey = uint8_t;
    using H265SPSKey = uint16_t;
    using H265PPSKey = uint32_t;
    using ParameterKey = uint32_t;

    struct H264Parameters {
        vvl::unordered_map<H264SPSKey, StdVideoH264SequenceParameterSet> sps;
        vvl::unordered_map<H264PPSKey, StdVideoH264PictureParameterSet> pps;
        uint32_t spsCapacity;
        uint32_t ppsCapacity;
    };

    struct H265Parameters {
        vvl::unordered_map<H265VPSKey, StdVideoH265VideoParameterSet> vps;
        vvl::unordered_map<H265SPSKey, StdVideoH265SequenceParameterSet> sps;
        vvl::unordered_map<H265PPSKey, StdVideoH265PictureParameterSet> pps;
        uint32_t vpsCapacity;
        uint32_t spsCapacity;
        uint32_t ppsCapacity;
    };

    struct Data {
        uint32_t update_sequence_counter;
        H264Parameters h264;
        H265Parameters h265;
    };

    static H264SPSKey GetH264SPSKey(const StdVideoH264SequenceParameterSet &sps) { return sps.seq_parameter_set_id; }

    static H264PPSKey GetH264PPSKey(const StdVideoH264PictureParameterSet &pps) {
        return GetKeyFor2xID8(pps.seq_parameter_set_id, pps.pic_parameter_set_id);
    }

    static H265VPSKey GetH265VPSKey(const StdVideoH265VideoParameterSet &vps) { return vps.vps_video_parameter_set_id; }

    static H265SPSKey GetH265SPSKey(const StdVideoH265SequenceParameterSet &sps) {
        return GetKeyFor2xID8(sps.sps_video_parameter_set_id, sps.sps_seq_parameter_set_id);
    }

    static H265PPSKey GetH265PPSKey(const StdVideoH265PictureParameterSet &pps) {
        return GetKeyFor3xID8(pps.sps_video_parameter_set_id, pps.pps_seq_parameter_set_id, pps.pps_pic_parameter_set_id);
    }

    class ReadOnlyAccessor {
      public:
        ReadOnlyAccessor() : lock_(), data_(nullptr) {}
        ReadOnlyAccessor(const VIDEO_SESSION_PARAMETERS_STATE *state) : lock_(state->mutex_), data_(&state->data_) {}
        operator bool() const { return data_ != nullptr; }
        const Data *operator->() const { return data_; }

        const StdVideoH264SequenceParameterSet *GetH264SPS(uint8_t sps_id) const {
            auto it = data_->h264.sps.find(sps_id);
            if (it != data_->h264.sps.end()) {
                return &it->second;
            } else {
                return nullptr;
            }
        }

        const StdVideoH264PictureParameterSet *GetH264PPS(uint8_t sps_id, uint8_t pps_id) const {
            auto it = data_->h264.pps.find(GetKeyFor2xID8(sps_id, pps_id));
            if (it != data_->h264.pps.end()) {
                return &it->second;
            } else {
                return nullptr;
            }
        }

        const StdVideoH265VideoParameterSet *GetH265VPS(uint8_t vps_id) const {
            auto it = data_->h265.vps.find(vps_id);
            if (it != data_->h265.vps.end()) {
                return &it->second;
            } else {
                return nullptr;
            }
        }

        const StdVideoH265SequenceParameterSet *GetH265SPS(uint8_t vps_id, uint8_t sps_id) const {
            auto it = data_->h265.sps.find(GetKeyFor2xID8(vps_id, sps_id));
            if (it != data_->h265.sps.end()) {
                return &it->second;
            } else {
                return nullptr;
            }
        }

        const StdVideoH265PictureParameterSet *GetH265PPS(uint8_t vps_id, uint8_t sps_id, uint8_t pps_id) const {
            auto it = data_->h265.pps.find(GetKeyFor3xID8(vps_id, sps_id, pps_id));
            if (it != data_->h265.pps.end()) {
                return &it->second;
            } else {
                return nullptr;
            }
        }

      private:
        std::unique_lock<std::mutex> lock_;
        const Data *data_;
    };

    const safe_VkVideoSessionParametersCreateInfoKHR createInfo;
    std::shared_ptr<const VIDEO_SESSION_STATE> vs_state;

    VIDEO_SESSION_PARAMETERS_STATE(VkVideoSessionParametersKHR vsp, VkVideoSessionParametersCreateInfoKHR const *pCreateInfo,
                                   std::shared_ptr<VIDEO_SESSION_STATE> &&vsstate,
                                   std::shared_ptr<VIDEO_SESSION_PARAMETERS_STATE> &&vsp_template);

    VkVideoSessionParametersKHR videoSessionParameters() const { return handle_.Cast<VkVideoSessionParametersKHR>(); }

    VkVideoCodecOperationFlagBitsKHR GetCodecOp() const { return vs_state->GetCodecOp(); }

    void Update(VkVideoSessionParametersUpdateInfoKHR const *info);

    ReadOnlyAccessor Lock() const { return ReadOnlyAccessor(this); }

  private:
    mutable std::mutex mutex_;
    Data data_;

    static uint16_t GetKeyFor2xID8(uint8_t id1, uint8_t id2) { return (id1 << 8) | id2; }
    static uint32_t GetKeyFor3xID8(uint8_t id1, uint8_t id2, uint8_t id3) { return (id1 << 16) | (id2 << 8) | id3; }

    void AddDecodeH264(VkVideoDecodeH264SessionParametersAddInfoKHR const *info);
    void AddDecodeH265(VkVideoDecodeH265SessionParametersAddInfoKHR const *info);
};

using VideoSessionUpdateList =
    std::vector<std::function<bool(const ValidationStateTracker *dev_data, const VIDEO_SESSION_STATE *vs_state,
                                   VideoSessionDeviceState &dev_state, bool do_validate)>>;
using VideoSessionUpdateMap = vvl::unordered_map<VkVideoSessionKHR, VideoSessionUpdateList>;
