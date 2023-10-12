/*
 * Copyright (c) 2022-2023 The Khronos Group Inc.
 * Copyright (c) 2022-2023 RasterGrid Kft.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once

#include "layer_validation_tests.h"
#include "generated/vk_extension_helper.h"
#include <vk_video/vulkan_video_codecs_common.h>
#include <vk_video/vulkan_video_codec_h264std.h>
#include <vk_video/vulkan_video_codec_h264std_decode.h>
#include <vk_video/vulkan_video_codec_h265std.h>
#include <vk_video/vulkan_video_codec_h265std_decode.h>

#include <memory>
#include <vector>
#include <tuple>
#include <functional>
#include <math.h>

class VideoConfig {
  public:
    VideoConfig() { session_create_info_.queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; }

    operator bool() const { return profile_.videoCodecOperation != VK_VIDEO_CODEC_OPERATION_NONE_KHR; }

    uint32_t QueueFamilyIndex() const { return session_create_info_.queueFamilyIndex; }
    const VkVideoProfileInfoKHR* Profile() const { return profile_.ptr(); }
    VkVideoProfileInfoKHR* Profile() { return profile_.ptr(); }
    const VkExtensionProperties* StdVersion() const { return &caps_.stdHeaderVersion; }
    const VkVideoSessionCreateInfoKHR* SessionCreateInfo() const { return session_create_info_.ptr(); }
    VkVideoSessionCreateInfoKHR* SessionCreateInfo() { return session_create_info_.ptr(); }
    const VkVideoSessionParametersCreateInfoKHR* SessionParamsCreateInfo() const { return session_params_create_info_.ptr(); }
    VkVideoSessionParametersCreateInfoKHR* SessionParamsCreateInfo() { return session_params_create_info_.ptr(); }
    const VkVideoFormatPropertiesKHR* PictureFormatProps() const { return picture_format_props_.data(); }
    const VkVideoFormatPropertiesKHR* DpbFormatProps() const { return dpb_format_props_.data(); }
    const std::vector<VkVideoFormatPropertiesKHR>& SupportedPictureFormatProps() const { return picture_format_props_; }
    const std::vector<VkVideoFormatPropertiesKHR>& SupportedDpbFormatProps() const { return dpb_format_props_; }

    uint32_t DpbSlotCount() const { return session_create_info_.maxDpbSlots; }
    VkExtent2D MaxCodedExtent() const { return session_create_info_.maxCodedExtent; }

    void SetQueueFamilyIndex(uint32_t queue_family_index) { session_create_info_.queueFamilyIndex = queue_family_index; }

    void SetCodecProfile(void* profile) {
        auto codec_specific = reinterpret_cast<VkBaseInStructure*>(profile);
        auto pNext = reinterpret_cast<const VkBaseInStructure*>(profile_.pNext);
        profile_.pNext = codec_specific;
        codec_specific->pNext = pNext;
    }

    VkVideoCapabilitiesKHR* Caps() { return caps_.ptr(); }
    const VkVideoCapabilitiesKHR* Caps() const { return caps_.ptr(); }
    const VkVideoDecodeCapabilitiesKHR* DecodeCaps() const { return vku::FindStructInPNextChain<VkVideoDecodeCapabilitiesKHR>(caps_.pNext); }
    const VkVideoDecodeH264CapabilitiesKHR* DecodeCapsH264() const {
        return vku::FindStructInPNextChain<VkVideoDecodeH264CapabilitiesKHR>(caps_.pNext);
    }
    const VkVideoDecodeH265CapabilitiesKHR* DecodeCapsH265() const {
        return vku::FindStructInPNextChain<VkVideoDecodeH265CapabilitiesKHR>(caps_.pNext);
    }

    bool SupportsDecodeOutputDistinct() const {
        return DecodeCaps()->flags & VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR;
    }

    bool SupportsDecodeOutputCoincide() const {
        return DecodeCaps()->flags & VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR;
    }

    bool IsDecode() const { return is_decode_; }

    void SetDecode() { is_decode_ = true; }

    void SetCodecCapsChain(void* codecCapsChain) { caps_.pNext = codecCapsChain; }

    bool NeedsSessionParams() const { return session_params_create_info_.pNext != nullptr; }

    void SetCodecSessionParamsInfo(const void* paramsInfo) { session_params_create_info_.pNext = paramsInfo; }

    void SetFormatProps(const std::vector<VkVideoFormatPropertiesKHR>& picture_format_props,
                        const std::vector<VkVideoFormatPropertiesKHR>& dpb_format_props) {
        picture_format_props_ = picture_format_props;
        dpb_format_props_ = dpb_format_props;

        session_create_info_.pictureFormat = picture_format_props[0].format;
        session_create_info_.referencePictureFormat = dpb_format_props[0].format;
    }

  private:
    bool is_decode_{};
    safe_VkVideoProfileInfoKHR profile_{};
    safe_VkVideoCapabilitiesKHR caps_{};
    safe_VkVideoSessionCreateInfoKHR session_create_info_{};
    safe_VkVideoSessionParametersCreateInfoKHR session_params_create_info_{};
    std::vector<VkVideoFormatPropertiesKHR> picture_format_props_{};
    std::vector<VkVideoFormatPropertiesKHR> dpb_format_props_{};
};

class BitstreamBuffer {
  public:
    BitstreamBuffer(vkt::Device* device, const VideoConfig& config, VkDeviceSize size, bool is_protected = false)
        : device_(device), size_(size), memory_(VK_NULL_HANDLE), buffer_(VK_NULL_HANDLE) {
        VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
        profile_list.profileCount = 1;
        profile_list.pProfiles = config.Profile();

        Init(profile_list, size, config.Profile()->videoCodecOperation, is_protected);
    }

    ~BitstreamBuffer() { Destroy(); }

    VkDeviceSize Size() const { return size_; }
    VkBuffer Buffer() const { return buffer_; }

  private:
    void Init(const VkVideoProfileListInfoKHR& profile_list, VkDeviceSize size, VkVideoCodecOperationFlagBitsKHR codec_op,
              bool is_protected) {
        VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        switch (codec_op) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
            case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
                usage |= VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR;
                break;

            default:
                break;
        }

        {
            VkBufferCreateInfo create_info = vku::InitStructHelper();
            create_info.flags = is_protected ? VK_BUFFER_CREATE_PROTECTED_BIT : 0;
            create_info.pNext = &profile_list;
            create_info.size = size;
            create_info.usage = usage;
            create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            ASSERT_EQ(VK_SUCCESS, vk::CreateBuffer(device_->device(), &create_info, nullptr, &buffer_));
        }

        {
            VkMemoryRequirements mem_req;
            vk::GetBufferMemoryRequirements(device_->device(), buffer_, &mem_req);

            VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
            ASSERT_TRUE(device_->phy().set_memory_type(mem_req.memoryTypeBits, &alloc_info, 0));
            alloc_info.allocationSize = mem_req.size;

            ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device_->device(), &alloc_info, nullptr, &memory_));
            ASSERT_EQ(VK_SUCCESS, vk::BindBufferMemory(device_->device(), buffer_, memory_, 0));
        }
    }

    void Destroy() {
        vk::DestroyBuffer(device_->device(), buffer_, nullptr);
        vk::FreeMemory(device_->device(), memory_, nullptr);
    }

    vkt::Device* device_{};
    VkDeviceSize size_{};
    VkDeviceMemory memory_{};
    VkBuffer buffer_{};
};

class VideoPictureResource {
  public:
    ~VideoPictureResource() { Destroy(); }

    VkImage Image() const { return image_; }
    VkImageView ImageView() const { return image_view_; }

    const VkDependencyInfo* LayoutTransition(VkImageLayout new_layout, uint32_t first_layer = 0,
                                             uint32_t layer_count = VK_REMAINING_ARRAY_LAYERS) {
        barrier_.newLayout = new_layout;
        barrier_.subresourceRange.baseArrayLayer = first_layer;
        barrier_.subresourceRange.layerCount = layer_count;
        return &dep_info_;
    }

  protected:
    VideoPictureResource(vkt::Device* device)
        : device_(device),
          memory_(VK_NULL_HANDLE),
          image_(VK_NULL_HANDLE),
          image_view_(VK_NULL_HANDLE),
          dep_info_(vku::InitStruct<VkDependencyInfo>()),
          barrier_(vku::InitStruct<VkImageMemoryBarrier2>()) {}

    void Init(const VkVideoProfileListInfoKHR& profile_list, VkExtent2D extent, uint32_t layers,
              const VkVideoFormatPropertiesKHR& format_props, bool is_protected) {
        {
            VkImageCreateInfo create_info = vku::InitStructHelper();
            create_info.flags = is_protected ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;
            create_info.pNext = &profile_list;
            create_info.imageType = format_props.imageType;
            create_info.format = format_props.format;
            create_info.extent = {extent.width, extent.height, 1};
            create_info.mipLevels = 1;
            create_info.arrayLayers = layers;
            create_info.samples = VK_SAMPLE_COUNT_1_BIT;
            create_info.tiling = format_props.imageTiling;
            create_info.usage = format_props.imageUsageFlags;
            create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            ASSERT_EQ(VK_SUCCESS, vk::CreateImage(device_->device(), &create_info, nullptr, &image_));
        }

        {
            VkMemoryRequirements mem_req;
            vk::GetImageMemoryRequirements(device_->device(), image_, &mem_req);

            VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
            ASSERT_TRUE(device_->phy().set_memory_type(mem_req.memoryTypeBits, &alloc_info, 0));
            alloc_info.allocationSize = mem_req.size;

            ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device_->device(), &alloc_info, nullptr, &memory_));
            ASSERT_EQ(VK_SUCCESS, vk::BindImageMemory(device_->device(), image_, memory_, 0));
        }

        {
            VkImageViewCreateInfo create_info = vku::InitStructHelper();
            create_info.image = image_;
            create_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            create_info.format = format_props.format;
            create_info.components = format_props.componentMapping;
            create_info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layers};

            ASSERT_EQ(VK_SUCCESS, vk::CreateImageView(device_->device(), &create_info, nullptr, &image_view_));
        }

        {
            dep_info_.imageMemoryBarrierCount = 1;
            dep_info_.pImageMemoryBarriers = &barrier_;
            barrier_.srcStageMask = 0;
            barrier_.srcAccessMask = 0;
            barrier_.dstStageMask = 0;
            barrier_.dstAccessMask = 0;
            barrier_.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            if (format_props.imageUsageFlags &
                (VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR | VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR)) {
                barrier_.srcStageMask |= VK_PIPELINE_STAGE_2_VIDEO_DECODE_BIT_KHR;
                barrier_.srcAccessMask |= VK_ACCESS_2_VIDEO_DECODE_READ_BIT_KHR | VK_ACCESS_2_VIDEO_DECODE_WRITE_BIT_KHR;
                barrier_.dstStageMask |= VK_PIPELINE_STAGE_2_VIDEO_DECODE_BIT_KHR;
                barrier_.dstAccessMask |= VK_ACCESS_2_VIDEO_DECODE_READ_BIT_KHR | VK_ACCESS_2_VIDEO_DECODE_WRITE_BIT_KHR;
            }

            barrier_.image = Image();
            barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier_.subresourceRange.baseMipLevel = 0;
            barrier_.subresourceRange.levelCount = 1;
        }
    }

    void Destroy() {
        vk::DestroyImageView(device_->device(), image_view_, nullptr);
        vk::DestroyImage(device_->device(), image_, nullptr);
        vk::FreeMemory(device_->device(), memory_, nullptr);
    }

  private:
    vkt::Device* device_{};
    VkDeviceMemory memory_{};
    VkImage image_{};
    VkImageView image_view_{};
    VkDependencyInfo dep_info_{};
    VkImageMemoryBarrier2 barrier_{};
};

class VideoDecodeOutput : public VideoPictureResource {
  public:
    VideoDecodeOutput(vkt::Device* device, const VideoConfig& config, bool is_protected = false)
        : VideoPictureResource(device), picture_(vku::InitStruct<VkVideoPictureResourceInfoKHR>()) {
        VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
        profile_list.profileCount = 1;
        profile_list.pProfiles = config.Profile();

        Init(profile_list, config.MaxCodedExtent(), 1, *config.PictureFormatProps(), is_protected);

        picture_.codedOffset = {0, 0};
        picture_.codedExtent = config.MaxCodedExtent();
        picture_.baseArrayLayer = 0;
        picture_.imageViewBinding = ImageView();
    }

    const VkVideoPictureResourceInfoKHR& Picture() const { return picture_; }

  private:
    VkVideoPictureResourceInfoKHR picture_{};
};

class VideoDPB : public VideoPictureResource {
  public:
    VideoDPB(vkt::Device* device, const VideoConfig& config, bool is_protected = false) : VideoPictureResource(device) {
        VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
        profile_list.profileCount = 1;
        profile_list.pProfiles = config.Profile();

        Init(profile_list, config.MaxCodedExtent(), config.DpbSlotCount(), *config.DpbFormatProps(), is_protected);

        reference_pictures_.resize(config.DpbSlotCount());
        for (uint32_t i = 0; i < config.DpbSlotCount(); ++i) {
            reference_pictures_[i] = vku::InitStructHelper();
            reference_pictures_[i].codedOffset = {0, 0};
            reference_pictures_[i].codedExtent = config.MaxCodedExtent();
            reference_pictures_[i].baseArrayLayer = i;
            reference_pictures_[i].imageViewBinding = ImageView();
        }
    }

    size_t PictureCount() const { return reference_pictures_.size(); }
    const VkVideoPictureResourceInfoKHR& Picture(int32_t index) const { return reference_pictures_[index]; }

  private:
    std::vector<VkVideoPictureResourceInfoKHR> reference_pictures_{};
};

class VideoBeginCodingInfo {
  public:
    VideoBeginCodingInfo(VideoDPB* dpb, VkVideoSessionKHR session, VkVideoSessionParametersKHR session_params)
        : dpb_(dpb), info_(vku::InitStruct<VkVideoBeginCodingInfoKHR>()), slot_resources_() {
        info_.videoSession = session;
        info_.videoSessionParameters = session_params;
    }

    VideoBeginCodingInfo(VideoBeginCodingInfo const& other) { CopyData(other); }

    VideoBeginCodingInfo& operator=(VideoBeginCodingInfo const& other) {
        CopyData(other);
        return *this;
    }

    VideoBeginCodingInfo& AddResource(int32_t slot_index, const VkVideoPictureResourceInfoKHR& resource) {
        slot_resources_.push_back(vku::InitStruct<VkVideoReferenceSlotInfoKHR>());
        auto& res = slot_resources_[info_.referenceSlotCount++];

        res.slotIndex = slot_index;
        res.pPictureResource = &resource;

        info_.pReferenceSlots = slot_resources_.data();

        return *this;
    }

    VideoBeginCodingInfo& AddResource(int32_t slot_index, int32_t resource_index) {
        return AddResource(slot_index, dpb_->Picture(resource_index));
    }

    VideoBeginCodingInfo& InvalidateSlot(int32_t slot_index) {
        slot_resources_.push_back(vku::InitStruct<VkVideoReferenceSlotInfoKHR>());
        auto& res = slot_resources_[info_.referenceSlotCount++];

        res.slotIndex = slot_index;
        res.pPictureResource = nullptr;

        info_.pReferenceSlots = slot_resources_.data();

        return *this;
    }

    operator const VkVideoBeginCodingInfoKHR&() const { return info_; }
    VkVideoBeginCodingInfoKHR* operator->() { return &info_; }

  private:
    void CopyData(VideoBeginCodingInfo const& other) {
        dpb_ = other.dpb_;
        info_ = other.info_;
        slot_resources_ = other.slot_resources_;
        info_.pReferenceSlots = slot_resources_.data();
    }

    VideoDPB* dpb_{};
    VkVideoBeginCodingInfoKHR info_{};
    std::vector<VkVideoReferenceSlotInfoKHR> slot_resources_{};
};

class VideoCodingControlInfo {
  public:
    VideoCodingControlInfo() : info_(vku::InitStruct<VkVideoCodingControlInfoKHR>()) {}

    VideoCodingControlInfo& Reset() {
        info_.flags |= VK_VIDEO_CODING_CONTROL_RESET_BIT_KHR;
        return *this;
    }

    operator const VkVideoCodingControlInfoKHR&() const { return info_; }
    VkVideoCodingControlInfoKHR* operator->() { return &info_; }

  private:
    VkVideoCodingControlInfoKHR info_{};
};

class VideoEndCodingInfo {
  public:
    VideoEndCodingInfo() : info_(vku::InitStruct<VkVideoEndCodingInfoKHR>()) {}

    operator const VkVideoEndCodingInfoKHR&() const { return info_; }
    VkVideoEndCodingInfoKHR* operator->() { return &info_; }

  private:
    VkVideoEndCodingInfoKHR info_{};
};

class VideoDecodeInfo {
  public:
    VideoDecodeInfo(const VideoConfig& config, BitstreamBuffer& bitstream, VideoDPB* dpb, const VideoDecodeOutput* output)
        : output_distinct_(config.SupportsDecodeOutputDistinct()),
          codec_op_(config.Profile()->videoCodecOperation),
          dpb_(dpb),
          info_(vku::InitStruct<VkVideoDecodeInfoKHR>()),
          reconstructed_(vku::InitStruct<VkVideoReferenceSlotInfoKHR>()),
          references_(dpb ? dpb->PictureCount() : 0, vku::InitStruct<VkVideoReferenceSlotInfoKHR>()) {
        assert(output != nullptr);
        info_.srcBuffer = bitstream.Buffer();
        info_.srcBufferOffset = 0;
        info_.srcBufferRange = bitstream.Size();
        info_.dstPictureResource = output->Picture();

        switch (codec_op_) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
                info_.pNext = &codec_info_.decode_h264.picture_info;

                codec_info_.decode_h264.slice_offsets = {0};

                codec_info_.decode_h264.picture_info = vku::InitStructHelper();
                codec_info_.decode_h264.picture_info.pStdPictureInfo = &codec_info_.decode_h264.std_picture_info;
                codec_info_.decode_h264.picture_info.sliceCount = (uint32_t)codec_info_.decode_h264.slice_offsets.size();
                codec_info_.decode_h264.picture_info.pSliceOffsets = codec_info_.decode_h264.slice_offsets.data();

                codec_info_.decode_h264.std_picture_info = {};

                reconstructed_.pNext = &codec_info_.decode_h264.setup_slot_info;

                codec_info_.decode_h264.setup_slot_info = vku::InitStructHelper();
                codec_info_.decode_h264.setup_slot_info.pStdReferenceInfo = &codec_info_.decode_h264.std_setup_reference_info;

                codec_info_.decode_h264.std_setup_reference_info = {};

                codec_info_.decode_h264.dpb_slot_info.resize(references_.size(), vku::InitStruct<VkVideoDecodeH264DpbSlotInfoKHR>());
                codec_info_.decode_h264.std_reference_info.resize(references_.size(), {});

                for (size_t i = 0; i < references_.size(); ++i) {
                    references_[i].pNext = &codec_info_.decode_h264.dpb_slot_info[i];
                    codec_info_.decode_h264.dpb_slot_info[i].pStdReferenceInfo = &codec_info_.decode_h264.std_reference_info[i];
                }
                break;

            case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
                info_.pNext = &codec_info_.decode_h265.picture_info;

                codec_info_.decode_h265.slice_segment_offsets = {0};

                codec_info_.decode_h265.picture_info = vku::InitStructHelper();
                codec_info_.decode_h265.picture_info.pStdPictureInfo = &codec_info_.decode_h265.std_picture_info;
                codec_info_.decode_h265.picture_info.sliceSegmentCount =
                    (uint32_t)codec_info_.decode_h265.slice_segment_offsets.size();
                codec_info_.decode_h265.picture_info.pSliceSegmentOffsets = codec_info_.decode_h265.slice_segment_offsets.data();

                codec_info_.decode_h265.std_picture_info = {};

                reconstructed_.pNext = &codec_info_.decode_h265.setup_slot_info;

                codec_info_.decode_h265.setup_slot_info = vku::InitStructHelper();
                codec_info_.decode_h265.setup_slot_info.pStdReferenceInfo = &codec_info_.decode_h265.std_setup_reference_info;

                codec_info_.decode_h265.std_setup_reference_info = {};

                codec_info_.decode_h265.dpb_slot_info.resize(references_.size(), vku::InitStruct<VkVideoDecodeH265DpbSlotInfoKHR>());
                codec_info_.decode_h265.std_reference_info.resize(references_.size(), {});

                for (size_t i = 0; i < references_.size(); ++i) {
                    references_[i].pNext = &codec_info_.decode_h265.dpb_slot_info[i];
                    codec_info_.decode_h265.dpb_slot_info[i].pStdReferenceInfo = &codec_info_.decode_h265.std_reference_info[i];
                }
                break;

            default:
                break;
        }
    }

    VideoDecodeInfo(VideoDecodeInfo const& other) { CopyData(other); }

    VideoDecodeInfo& operator=(VideoDecodeInfo const& other) {
        CopyData(other);
        return *this;
    }

    VideoDecodeInfo& SetFrame() {
        switch (codec_op_) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
                codec_info_.decode_h264.std_picture_info.flags.field_pic_flag = 0;
                return *this;

            default:
                return *this;
        }
    }

    VideoDecodeInfo& SetTopField() {
        switch (codec_op_) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
                codec_info_.decode_h264.std_picture_info.flags.field_pic_flag = 1;
                codec_info_.decode_h264.std_picture_info.flags.bottom_field_flag = 0;
                return *this;

            default:
                return *this;
        }
    }

    VideoDecodeInfo& SetBottomField() {
        switch (codec_op_) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
                codec_info_.decode_h264.std_picture_info.flags.field_pic_flag = 1;
                codec_info_.decode_h264.std_picture_info.flags.bottom_field_flag = 1;
                return *this;

            default:
                return *this;
        }
    }

    VideoDecodeInfo& SetBitstream(const BitstreamBuffer& bitstream) {
        info_.srcBuffer = bitstream.Buffer();
        info_.srcBufferOffset = 0;
        info_.srcBufferRange = bitstream.Size();
        return *this;
    }

    VideoDecodeInfo& SetBitstreamBuffer(VkBuffer bitstream, VkDeviceSize offset, VkDeviceSize range) {
        info_.srcBuffer = bitstream;
        info_.srcBufferOffset = offset;
        info_.srcBufferRange = range;
        return *this;
    }

    VideoDecodeInfo& SetDecodeOutput(const VideoDecodeOutput* output) {
        assert(output != nullptr);
        info_.dstPictureResource = output->Picture();
        return *this;
    }

    VideoDecodeInfo& SetDecodeOutput(const VkVideoPictureResourceInfoKHR& resource) {
        info_.dstPictureResource = resource;
        return *this;
    }

    VideoDecodeInfo& SetupFrame(int32_t slot_index, const VkVideoPictureResourceInfoKHR* resource, bool force_coincide = false) {
        switch (codec_op_) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
                auto& info = codec_info_.decode_h264.std_setup_reference_info;
                info.flags.top_field_flag = 0;
                info.flags.bottom_field_flag = 0;
                return Setup(slot_index, resource, force_coincide);
            }

            default:
                return Setup(slot_index, resource, force_coincide);
        }
    }

    VideoDecodeInfo& SetupFrame(int32_t slot_index, int32_t resource_index, bool force_coincide = false) {
        return SetupFrame(slot_index, &dpb_->Picture(resource_index), force_coincide);
    }

    VideoDecodeInfo& SetupFrame(int32_t slot_index, bool force_coincide = false) {
        return SetupFrame(slot_index, slot_index, force_coincide);
    }

    VideoDecodeInfo& SetupTopField(int32_t slot_index, const VkVideoPictureResourceInfoKHR* resource, bool force_coincide = false) {
        switch (codec_op_) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
                auto& info = codec_info_.decode_h264.std_setup_reference_info;
                info.flags.top_field_flag = 1;
                info.flags.bottom_field_flag = 0;
                return Setup(slot_index, resource, force_coincide);
            }

            default:
                return Setup(slot_index, resource, force_coincide);
        }
    }

    VideoDecodeInfo& SetupTopField(int32_t slot_index, int32_t resource_index, bool force_coincide = false) {
        return SetupTopField(slot_index, &dpb_->Picture(resource_index), force_coincide);
    }

    VideoDecodeInfo& SetupTopField(int32_t slot_index, bool force_coincide = false) {
        return SetupTopField(slot_index, slot_index, force_coincide);
    }

    VideoDecodeInfo& SetupBottomField(int32_t slot_index, const VkVideoPictureResourceInfoKHR* resource,
                                      bool force_coincide = false) {
        switch (codec_op_) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
                auto& info = codec_info_.decode_h264.std_setup_reference_info;
                info.flags.top_field_flag = 0;
                info.flags.bottom_field_flag = 1;
                return Setup(slot_index, resource, force_coincide);
            }

            default:
                return Setup(slot_index, resource, force_coincide);
        }
    }

    VideoDecodeInfo& SetupBottomField(int32_t slot_index, int32_t resource_index, bool force_coincide = false) {
        return SetupBottomField(slot_index, &dpb_->Picture(resource_index), force_coincide);
    }

    VideoDecodeInfo& SetupBottomField(int32_t slot_index, bool force_coincide = false) {
        return SetupBottomField(slot_index, slot_index, force_coincide);
    }

    VideoDecodeInfo& AddReferenceFrame(int32_t slot_index, const VkVideoPictureResourceInfoKHR* resource) {
        assert(dpb_ != nullptr);
        size_t index = info_.referenceSlotCount++;

        references_[index].slotIndex = slot_index;
        references_[index].pPictureResource = resource;

        info_.pReferenceSlots = references_.data();

        switch (codec_op_) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
                auto& info = codec_info_.decode_h264.std_reference_info[index];
                info.flags.top_field_flag = 0;
                info.flags.bottom_field_flag = 0;
                return *this;
            }

            default:
                return *this;
        }
    }

    VideoDecodeInfo& AddReferenceFrame(int32_t slot_index, int32_t resource_index) {
        return AddReferenceFrame(slot_index, &dpb_->Picture(resource_index));
    }

    VideoDecodeInfo& AddReferenceFrame(int32_t slot_index) { return AddReferenceFrame(slot_index, slot_index); }

    VideoDecodeInfo& AddReferenceTopField(int32_t slot_index, const VkVideoPictureResourceInfoKHR* resource) {
        assert(dpb_ != nullptr);
        size_t index = info_.referenceSlotCount++;

        references_[index].slotIndex = slot_index;
        references_[index].pPictureResource = resource;

        info_.pReferenceSlots = references_.data();

        switch (codec_op_) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
                auto& info = codec_info_.decode_h264.std_reference_info[index];
                info.flags.top_field_flag = 1;
                info.flags.bottom_field_flag = 0;
                return *this;
            }

            default:
                return *this;
        }
    }

    VideoDecodeInfo& AddReferenceTopField(int32_t slot_index, int32_t resource_index) {
        return AddReferenceTopField(slot_index, &dpb_->Picture(resource_index));
    }

    VideoDecodeInfo& AddReferenceTopField(int32_t slot_index) { return AddReferenceTopField(slot_index, slot_index); }

    VideoDecodeInfo& AddReferenceBottomField(int32_t slot_index, const VkVideoPictureResourceInfoKHR* resource) {
        assert(dpb_ != nullptr);
        size_t index = info_.referenceSlotCount++;

        references_[index].slotIndex = slot_index;
        references_[index].pPictureResource = resource;

        info_.pReferenceSlots = references_.data();

        switch (codec_op_) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
                auto& info = codec_info_.decode_h264.std_reference_info[index];
                info.flags.top_field_flag = 0;
                info.flags.bottom_field_flag = 1;
                return *this;
            }

            default:
                return *this;
        }
    }

    VideoDecodeInfo& AddReferenceBottomField(int32_t slot_index, int32_t resource_index) {
        return AddReferenceBottomField(slot_index, &dpb_->Picture(resource_index));
    }

    VideoDecodeInfo& AddReferenceBottomField(int32_t slot_index) { return AddReferenceBottomField(slot_index, slot_index); }

    VideoDecodeInfo& AddReferenceBothFields(int32_t slot_index, const VkVideoPictureResourceInfoKHR* resource) {
        assert(dpb_ != nullptr);
        size_t index = info_.referenceSlotCount++;

        references_[index].slotIndex = slot_index;
        references_[index].pPictureResource = resource;

        info_.pReferenceSlots = references_.data();

        switch (codec_op_) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
                auto& info = codec_info_.decode_h264.std_reference_info[index];
                info.flags.top_field_flag = 1;
                info.flags.bottom_field_flag = 1;
                return *this;
            }

            default:
                return *this;
        }
    }

    VideoDecodeInfo& AddReferenceBothFields(int32_t slot_index, int32_t resource_index) {
        return AddReferenceBothFields(slot_index, &dpb_->Picture(resource_index));
    }

    VideoDecodeInfo& AddReferenceBothFields(int32_t slot_index) { return AddReferenceBothFields(slot_index, slot_index); }

    operator const VkVideoDecodeInfoKHR&() const { return info_; }
    VkVideoDecodeInfoKHR* operator->() { return &info_; }

  private:
    void CopyData(VideoDecodeInfo const& other) {
        output_distinct_ = other.output_distinct_;
        codec_op_ = other.codec_op_;
        dpb_ = other.dpb_;
        info_ = other.info_;
        reconstructed_ = other.reconstructed_;
        references_ = other.references_;

        if (other.info_.pSetupReferenceSlot != nullptr) {
            info_.pSetupReferenceSlot = &reconstructed_;
        }
        if (other.info_.pReferenceSlots != nullptr) {
            info_.pReferenceSlots = references_.data();
        }

        switch (codec_op_) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
                info_.pNext = &codec_info_.decode_h264.picture_info;

                codec_info_.decode_h264.slice_offsets = other.codec_info_.decode_h264.slice_offsets;

                codec_info_.decode_h264.picture_info = other.codec_info_.decode_h264.picture_info;
                codec_info_.decode_h264.picture_info.pStdPictureInfo = &codec_info_.decode_h264.std_picture_info;
                codec_info_.decode_h264.picture_info.sliceCount = (uint32_t)codec_info_.decode_h264.slice_offsets.size();
                codec_info_.decode_h264.picture_info.pSliceOffsets = codec_info_.decode_h264.slice_offsets.data();

                codec_info_.decode_h264.std_picture_info = other.codec_info_.decode_h264.std_picture_info;

                reconstructed_.pNext = &codec_info_.decode_h264.setup_slot_info;

                codec_info_.decode_h264.setup_slot_info = other.codec_info_.decode_h264.setup_slot_info;
                codec_info_.decode_h264.setup_slot_info.pStdReferenceInfo = &codec_info_.decode_h264.std_setup_reference_info;

                codec_info_.decode_h264.std_setup_reference_info = other.codec_info_.decode_h264.std_setup_reference_info;

                codec_info_.decode_h264.dpb_slot_info = other.codec_info_.decode_h264.dpb_slot_info;
                codec_info_.decode_h264.std_reference_info = other.codec_info_.decode_h264.std_reference_info;

                for (size_t i = 0; i < references_.size(); ++i) {
                    references_[i].pNext = &codec_info_.decode_h264.dpb_slot_info[i];
                    codec_info_.decode_h264.dpb_slot_info[i].pStdReferenceInfo = &codec_info_.decode_h264.std_reference_info[i];
                }
                break;

            case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
                info_.pNext = &codec_info_.decode_h265.picture_info;

                codec_info_.decode_h265.slice_segment_offsets = other.codec_info_.decode_h265.slice_segment_offsets;

                codec_info_.decode_h265.picture_info = other.codec_info_.decode_h265.picture_info;
                codec_info_.decode_h265.picture_info.pStdPictureInfo = &codec_info_.decode_h265.std_picture_info;
                codec_info_.decode_h265.picture_info.sliceSegmentCount =
                    (uint32_t)codec_info_.decode_h265.slice_segment_offsets.size();
                codec_info_.decode_h265.picture_info.pSliceSegmentOffsets = codec_info_.decode_h265.slice_segment_offsets.data();

                codec_info_.decode_h265.std_picture_info = other.codec_info_.decode_h265.std_picture_info;

                reconstructed_.pNext = &codec_info_.decode_h265.setup_slot_info;

                codec_info_.decode_h265.setup_slot_info = other.codec_info_.decode_h265.setup_slot_info;
                codec_info_.decode_h265.setup_slot_info.pStdReferenceInfo = &codec_info_.decode_h265.std_setup_reference_info;

                codec_info_.decode_h264.std_setup_reference_info = other.codec_info_.decode_h264.std_setup_reference_info;

                codec_info_.decode_h265.dpb_slot_info = other.codec_info_.decode_h265.dpb_slot_info;
                codec_info_.decode_h265.std_reference_info = other.codec_info_.decode_h265.std_reference_info;

                for (size_t i = 0; i < references_.size(); ++i) {
                    references_[i].pNext = &codec_info_.decode_h265.dpb_slot_info[i];
                    codec_info_.decode_h265.dpb_slot_info[i].pStdReferenceInfo = &codec_info_.decode_h265.std_reference_info[i];
                }
                break;

            default:
                break;
        }
    }

    VideoDecodeInfo& Setup(int32_t slot_index, const VkVideoPictureResourceInfoKHR* resource, bool force_coincide = false) {
        assert(dpb_ != nullptr);

        reconstructed_.slotIndex = slot_index;
        reconstructed_.pPictureResource = resource;

        if ((!output_distinct_ || force_coincide) && reconstructed_.pPictureResource != nullptr) {
            info_.dstPictureResource = *reconstructed_.pPictureResource;
        }

        info_.pSetupReferenceSlot = &reconstructed_;

        return *this;
    }

    bool output_distinct_{};
    VkVideoCodecOperationFlagBitsKHR codec_op_{};
    VideoDPB* dpb_{};
    VkVideoDecodeInfoKHR info_{};
    VkVideoReferenceSlotInfoKHR reconstructed_{};
    std::vector<VkVideoReferenceSlotInfoKHR> references_{};
    struct {
        struct {
            VkVideoDecodeH264PictureInfoKHR picture_info{};
            StdVideoDecodeH264PictureInfo std_picture_info{};
            std::vector<uint32_t> slice_offsets{};
            VkVideoDecodeH264DpbSlotInfoKHR setup_slot_info{};
            StdVideoDecodeH264ReferenceInfo std_setup_reference_info{};
            std::vector<VkVideoDecodeH264DpbSlotInfoKHR> dpb_slot_info{};
            std::vector<StdVideoDecodeH264ReferenceInfo> std_reference_info{};
        } decode_h264{};
        struct {
            VkVideoDecodeH265PictureInfoKHR picture_info{};
            StdVideoDecodeH265PictureInfo std_picture_info{};
            std::vector<uint32_t> slice_segment_offsets{};
            VkVideoDecodeH265DpbSlotInfoKHR setup_slot_info{};
            StdVideoDecodeH265ReferenceInfo std_setup_reference_info{};
            std::vector<VkVideoDecodeH265DpbSlotInfoKHR> dpb_slot_info{};
            std::vector<StdVideoDecodeH265ReferenceInfo> std_reference_info{};
        } decode_h265{};
    } codec_info_{};
};

class VideoContext {
  public:
    struct {
        PFN_vkGetVideoSessionMemoryRequirementsKHR GetVideoSessionMemoryRequirementsKHR;
        PFN_vkBindVideoSessionMemoryKHR BindVideoSessionMemoryKHR;
        PFN_vkCreateVideoSessionKHR CreateVideoSessionKHR;
        PFN_vkDestroyVideoSessionKHR DestroyVideoSessionKHR;
        PFN_vkCreateVideoSessionParametersKHR CreateVideoSessionParametersKHR;
        PFN_vkUpdateVideoSessionParametersKHR UpdateVideoSessionParametersKHR;
        PFN_vkDestroyVideoSessionParametersKHR DestroyVideoSessionParametersKHR;
    } vk{};

    explicit VideoContext(vkt::Device* device, const VideoConfig& config, bool protected_content = false)
        : vk(),
          config_(config),
          device_(device),
          queue_(GetQueue(device, config)),
          cmd_pool_(
              *device, queue_.get_family_index(),
              VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | (protected_content ? VK_COMMAND_POOL_CREATE_PROTECTED_BIT : 0)),
          cmd_buffer_(device, &cmd_pool_, VK_COMMAND_BUFFER_LEVEL_PRIMARY, &queue_),
          session_(VK_NULL_HANDLE),
          session_memory_(),
          session_params_(VK_NULL_HANDLE),
          status_query_pool_(VK_NULL_HANDLE),
          bitstream_(nullptr),
          dpb_(nullptr),
          decode_output_(nullptr) {
        Init(protected_content);
    }

    ~VideoContext() { Destroy(); }

    void CreateAndBindSessionMemory() {
        ASSERT_TRUE(session_ != VK_NULL_HANDLE);

        uint32_t mem_req_count = 0;
        ASSERT_EQ(VK_SUCCESS, vk.GetVideoSessionMemoryRequirementsKHR(device_->device(), session_, &mem_req_count, nullptr));
        if (mem_req_count == 0) return;

        std::vector<VkVideoSessionMemoryRequirementsKHR> mem_reqs(mem_req_count,
                                                                  vku::InitStruct<VkVideoSessionMemoryRequirementsKHR>());
        ASSERT_EQ(VK_SUCCESS, vk.GetVideoSessionMemoryRequirementsKHR(device_->device(), session_, &mem_req_count, mem_reqs.data()));

        std::vector<VkBindVideoSessionMemoryInfoKHR> bind_info(mem_req_count, vku::InitStruct<VkBindVideoSessionMemoryInfoKHR>());
        for (uint32_t i = 0; i < mem_req_count; ++i) {
            VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
            ASSERT_TRUE(device_->phy().set_memory_type(mem_reqs[i].memoryRequirements.memoryTypeBits, &alloc_info, 0));
            alloc_info.allocationSize = mem_reqs[i].memoryRequirements.size;

            VkDeviceMemory memory = VK_NULL_HANDLE;
            ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device_->device(), &alloc_info, nullptr, &memory));
            session_memory_.push_back(memory);

            bind_info[i].memoryBindIndex = mem_reqs[i].memoryBindIndex;
            bind_info[i].memory = memory;
            bind_info[i].memoryOffset = 0;
            bind_info[i].memorySize = mem_reqs[i].memoryRequirements.size;
        }

        ASSERT_EQ(VK_SUCCESS, vk.BindVideoSessionMemoryKHR(device_->device(), session_, (uint32_t)bind_info.size(), bind_info.data()));
    }

    void CreateResources(bool protected_bitstream = false, bool protected_dpb = false, bool protected_output = false) {
        VkDeviceSize buffer_size = std::max((VkDeviceSize)4096, config_.Caps()->minBitstreamBufferSizeAlignment * 2);
        bitstream_ = std::unique_ptr<BitstreamBuffer>(new BitstreamBuffer(device_, config_, buffer_size, protected_bitstream));

        if (config_.SessionCreateInfo()->maxDpbSlots > 0) {
            dpb_ = std::unique_ptr<VideoDPB>(new VideoDPB(device_, config_, protected_dpb));
        }

        decode_output_ = std::unique_ptr<VideoDecodeOutput>(new VideoDecodeOutput(device_, config_, protected_output));
    }

    void CreateStatusQueryPool(uint32_t query_count = 1) {
        VkQueryPoolCreateInfo create_info = vku::InitStructHelper();
        create_info.pNext = config_.Profile();
        create_info.queryType = VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR;
        create_info.queryCount = query_count;

        ASSERT_EQ(VK_SUCCESS, vk::CreateQueryPool(device_->device(), &create_info, nullptr, &status_query_pool_));
    }

    VkVideoSessionKHR Session() { return session_; }
    VkVideoSessionParametersKHR SessionParams() { return session_params_; }
    VkQueryPool StatusQueryPool() { return status_query_pool_; }
    vkt::Queue& Queue() { return queue_; }
    vkt::CommandBuffer& CmdBuffer() { return cmd_buffer_; }

    BitstreamBuffer& Bitstream() { return *bitstream_; }
    VideoDPB* Dpb() { return dpb_.get(); }
    VideoDecodeOutput* DecodeOutput() { return decode_output_.get(); }

    VideoBeginCodingInfo Begin() const { return VideoBeginCodingInfo(dpb_.get(), session_, session_params_); }
    VideoCodingControlInfo Control() const { return VideoCodingControlInfo(); }
    VideoEndCodingInfo End() const { return VideoEndCodingInfo(); }

    VideoDecodeInfo DecodeFrame() { return Decode(); }
    VideoDecodeInfo DecodeTopField() { return Decode().SetTopField(); }
    VideoDecodeInfo DecodeBottomField() { return Decode().SetBottomField(); }

  private:
    VideoDecodeInfo Decode() { return VideoDecodeInfo(config_, *bitstream_, dpb_.get(), decode_output_.get()); }

    vkt::Queue GetQueue(vkt::Device* device, const VideoConfig& config) const {
        VkQueue queue = VK_NULL_HANDLE;
        if (config.QueueFamilyIndex() != VK_QUEUE_FAMILY_IGNORED) {
            vk::GetDeviceQueue(device->device(), config.QueueFamilyIndex(), 0, &queue);
        }
        return vkt::Queue(queue, config.QueueFamilyIndex());
    }

    void Init(bool protected_content) {
        vk.GetVideoSessionMemoryRequirementsKHR = (PFN_vkGetVideoSessionMemoryRequirementsKHR)vk::GetDeviceProcAddr(
            device_->device(), "vkGetVideoSessionMemoryRequirementsKHR");
        ASSERT_NE(vk.GetVideoSessionMemoryRequirementsKHR, nullptr);

        vk.BindVideoSessionMemoryKHR =
            (PFN_vkBindVideoSessionMemoryKHR)vk::GetDeviceProcAddr(device_->device(), "vkBindVideoSessionMemoryKHR");
        ASSERT_NE(vk.BindVideoSessionMemoryKHR, nullptr);

        vk.CreateVideoSessionKHR = (PFN_vkCreateVideoSessionKHR)vk::GetDeviceProcAddr(device_->device(), "vkCreateVideoSessionKHR");
        ASSERT_NE(vk.CreateVideoSessionKHR, nullptr);

        vk.DestroyVideoSessionKHR =
            (PFN_vkDestroyVideoSessionKHR)vk::GetDeviceProcAddr(device_->device(), "vkDestroyVideoSessionKHR");
        ASSERT_NE(vk.DestroyVideoSessionKHR, nullptr);

        vk.CreateVideoSessionParametersKHR =
            (PFN_vkCreateVideoSessionParametersKHR)vk::GetDeviceProcAddr(device_->device(), "vkCreateVideoSessionParametersKHR");
        ASSERT_NE(vk.CreateVideoSessionParametersKHR, nullptr);

        vk.UpdateVideoSessionParametersKHR =
            (PFN_vkUpdateVideoSessionParametersKHR)vk::GetDeviceProcAddr(device_->device(), "vkUpdateVideoSessionParametersKHR");
        ASSERT_NE(vk.UpdateVideoSessionParametersKHR, nullptr);

        vk.DestroyVideoSessionParametersKHR =
            (PFN_vkDestroyVideoSessionParametersKHR)vk::GetDeviceProcAddr(device_->device(), "vkDestroyVideoSessionParametersKHR");
        ASSERT_NE(vk.DestroyVideoSessionParametersKHR, nullptr);

        ASSERT_TRUE(queue_.handle() != VK_NULL_HANDLE);
        ASSERT_TRUE(cmd_pool_.handle() != VK_NULL_HANDLE);
        ASSERT_TRUE(cmd_buffer_.handle() != VK_NULL_HANDLE);

        {
            VkVideoSessionCreateInfoKHR create_info = *config_.SessionCreateInfo();
            if (protected_content) {
                create_info.flags |= VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR;
            }
            create_info.pVideoProfile = config_.Profile();
            create_info.pStdHeaderVersion = config_.StdVersion();

            ASSERT_EQ(VK_SUCCESS, vk.CreateVideoSessionKHR(device_->device(), &create_info, nullptr, &session_));
        }

        if (config_.NeedsSessionParams()) {
            VkVideoSessionParametersCreateInfoKHR create_info = *config_.SessionParamsCreateInfo();
            create_info.videoSession = session_;

            ASSERT_EQ(VK_SUCCESS, vk.CreateVideoSessionParametersKHR(device_->device(), &create_info, nullptr, &session_params_));
        }
    }

    void Destroy() {
        vk.DestroyVideoSessionParametersKHR(device_->device(), session_params_, nullptr);
        vk.DestroyVideoSessionKHR(device_->device(), session_, nullptr);

        vk::DestroyQueryPool(device_->device(), status_query_pool_, nullptr);

        for (auto session_memory : session_memory_) {
            vk::FreeMemory(device_->device(), session_memory, nullptr);
        }
    }

    const VideoConfig config_{};

    vkt::Device* device_{};
    vkt::Queue queue_;
    vkt::CommandPool cmd_pool_{};
    vkt::CommandBuffer cmd_buffer_{};

    VkVideoSessionKHR session_{};
    std::vector<VkDeviceMemory> session_memory_{};
    VkVideoSessionParametersKHR session_params_{};

    VkQueryPool status_query_pool_{};

    std::unique_ptr<BitstreamBuffer> bitstream_{};
    std::unique_ptr<VideoDPB> dpb_{};
    std::unique_ptr<VideoDecodeOutput> decode_output_{};
};

class VkVideoLayerTest : public VkLayerTest {
  protected:
    struct {
        PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR GetPhysicalDeviceVideoCapabilitiesKHR;
        PFN_vkGetPhysicalDeviceVideoFormatPropertiesKHR GetPhysicalDeviceVideoFormatPropertiesKHR;
    } vk{};

    void Init(bool enable_protected_memory = false) {
        SetTargetApiVersion(VK_API_VERSION_1_1);

        AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
        AddRequiredExtensions(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME);
        AddRequiredExtensions(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME);

        // NOTE: this appears to be required for the format that is chosen in
        // VkVideoLayerTest.BeginCodingIncompatRefPicProfile
        AddRequiredExtensions(VK_EXT_YCBCR_2PLANE_444_FORMATS_EXTENSION_NAME);

        AddOptionalExtensions(VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME);
        AddOptionalExtensions(VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME);

        RETURN_IF_SKIP(InitFramework(instance_pnext_));

        VkPhysicalDeviceProtectedMemoryFeatures prot_mem_features = vku::InitStructHelper();
        VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = vku::InitStructHelper(&prot_mem_features);
        VkPhysicalDeviceFeatures2 features = vku::InitStructHelper(&sync2_features);
        vk::GetPhysicalDeviceFeatures2(gpu(), &features);

        if (!enable_protected_memory) {
            prot_mem_features.protectedMemory = VK_FALSE;
        }

        if (sync2_features.synchronization2 != VK_TRUE) {
            GTEST_SKIP() << "Test requires synchronization2.";
        }

        VkPhysicalDeviceProtectedMemoryProperties prot_mem_props = vku::InitStructHelper();
        VkPhysicalDeviceProperties2 props = vku::InitStructHelper(&prot_mem_props);
        vk::GetPhysicalDeviceProperties2(gpu(), &props);

        protected_memory_enabled_ = (prot_mem_features.protectedMemory == VK_TRUE);
        protected_no_fault_supported_ = (prot_mem_props.protectedNoFault == VK_TRUE);

        RETURN_IF_SKIP(InitState(nullptr, &features));

        vk.GetPhysicalDeviceVideoCapabilitiesKHR = (PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR)vk::GetInstanceProcAddr(
            instance(), "vkGetPhysicalDeviceVideoCapabilitiesKHR");
        ASSERT_NE(vk.GetPhysicalDeviceVideoCapabilitiesKHR, nullptr);

        vk.GetPhysicalDeviceVideoFormatPropertiesKHR = (PFN_vkGetPhysicalDeviceVideoFormatPropertiesKHR)vk::GetInstanceProcAddr(
            instance(), "vkGetPhysicalDeviceVideoFormatPropertiesKHR");
        ASSERT_NE(vk.GetPhysicalDeviceVideoFormatPropertiesKHR, nullptr);

        uint32_t qf_count;
        vk::GetPhysicalDeviceQueueFamilyProperties2(gpu(), &qf_count, nullptr);

        queue_family_props_.resize(qf_count, vku::InitStruct<VkQueueFamilyProperties2>());
        queue_family_video_props_.resize(qf_count, vku::InitStruct<VkQueueFamilyVideoPropertiesKHR>());
        queue_family_query_result_status_props_.resize(qf_count, vku::InitStruct<VkQueueFamilyQueryResultStatusPropertiesKHR>());
        for (uint32_t i = 0; i < qf_count; ++i) {
            queue_family_props_[i].pNext = &queue_family_video_props_[i];
            queue_family_video_props_[i].pNext = &queue_family_query_result_status_props_[i];
        }
        vk::GetPhysicalDeviceQueueFamilyProperties2(gpu(), &qf_count, queue_family_props_.data());

        InitConfigs();
    }

    const std::vector<VideoConfig>& GetConfigs() const { return configs_; }
    const VideoConfig& GetConfig() const { return GetConfig(configs_); }
    const std::vector<VideoConfig>& GetConfigsDecode() const { return configs_decode_; }
    const VideoConfig& GetConfigDecode() const { return GetConfig(configs_decode_); }
    const std::vector<VideoConfig>& GetConfigsDecodeH264() const { return configs_decode_h264_; }
    const VideoConfig& GetConfigDecodeH264() const { return GetConfig(configs_decode_h264_); }
    const std::vector<VideoConfig>& GetConfigsDecodeH264Interlaced() const { return configs_decode_h264_interlaced_; }
    const VideoConfig& GetConfigDecodeH264Interlaced() const { return GetConfig(configs_decode_h264_interlaced_); }
    const std::vector<VideoConfig>& GetConfigsDecodeH265() const { return configs_decode_h265_; }
    const VideoConfig& GetConfigDecodeH265() const { return GetConfig(configs_decode_h265_); }

    const VideoConfig& GetConfig(const std::vector<VideoConfig>& configs) const {
        return configs.empty() ? default_config_ : configs[0];
    }

    const std::vector<VideoConfig> FilterConfigs(const std::vector<VideoConfig>& configs,
                                                 std::function<bool(const VideoConfig&)> filter) const {
        std::vector<VideoConfig> filtered_configs;
        for (const auto& config : configs) {
            if (filter(config)) {
                filtered_configs.push_back(config);
            }
        }
        return filtered_configs;
    }

    const std::vector<VideoConfig> GetConfigsWithDpbSlots(const std::vector<VideoConfig>& configs, uint32_t count = 1) const {
        return FilterConfigs(configs, [count](const VideoConfig& config) { return config.Caps()->maxDpbSlots >= count; });
    }

    const std::vector<VideoConfig> GetConfigsWithReferences(const std::vector<VideoConfig>& configs, uint32_t count = 1) const {
        return FilterConfigs(configs,
                             [count](const VideoConfig& config) { return config.Caps()->maxActiveReferencePictures >= count; });
    }

    const VideoConfig& GetConfigWithParams(const std::vector<VideoConfig>& configs) const {
        for (const auto& config : configs) {
            if (config.NeedsSessionParams()) {
                return config;
            }
        }
        return default_config_;
    }

    const VideoConfig& GetConfigWithoutProtectedContent(const std::vector<VideoConfig>& configs) const {
        for (const auto& config : configs) {
            if ((config.Caps()->flags & VK_VIDEO_CAPABILITY_PROTECTED_CONTENT_BIT_KHR) == 0) {
                return config;
            }
        }
        return default_config_;
    }

    const VideoConfig& GetConfigWithProtectedContent(const std::vector<VideoConfig>& configs) const {
        for (const auto& config : configs) {
            if (config.Caps()->flags & VK_VIDEO_CAPABILITY_PROTECTED_CONTENT_BIT_KHR) {
                return config;
            }
        }
        return default_config_;
    }

    uint32_t QueueFamilyCount() const { return (uint32_t)queue_family_video_props_.size(); }

    VkQueueFlags QueueFamilyFlags(uint32_t qfi) const { return queue_family_props_[qfi].queueFamilyProperties.queueFlags; }

    VkVideoCodecOperationFlagsKHR QueueFamilyVideoCodecOps(uint32_t qfi) const {
        return queue_family_video_props_[qfi].videoCodecOperations;
    }

    bool QueueFamilySupportsResultStatusOnlyQueries(uint32_t qfi) const {
        return queue_family_query_result_status_props_[qfi].queryResultStatusSupport;
    }

    StdVideoH264SequenceParameterSet CreateH264SPS(uint8_t sps_id) const {
        StdVideoH264SequenceParameterSet sps{};
        sps.seq_parameter_set_id = sps_id;
        return sps;
    }

    StdVideoH264PictureParameterSet CreateH264PPS(uint8_t sps_id, uint8_t pps_id) const {
        StdVideoH264PictureParameterSet pps{};
        pps.seq_parameter_set_id = sps_id;
        pps.pic_parameter_set_id = pps_id;
        return pps;
    }

    StdVideoH265VideoParameterSet CreateH265VPS(uint8_t vps_id) const {
        StdVideoH265VideoParameterSet vps{};
        vps.vps_video_parameter_set_id = vps_id;
        return vps;
    }

    StdVideoH265SequenceParameterSet CreateH265SPS(uint8_t vps_id, uint8_t sps_id) const {
        StdVideoH265SequenceParameterSet sps{};
        sps.sps_video_parameter_set_id = vps_id;
        sps.sps_seq_parameter_set_id = sps_id;
        return sps;
    }

    StdVideoH265PictureParameterSet CreateH265PPS(uint8_t vps_id, uint8_t sps_id, uint8_t pps_id) const {
        StdVideoH265PictureParameterSet pps{};
        pps.sps_video_parameter_set_id = vps_id;
        pps.pps_seq_parameter_set_id = sps_id;
        pps.pps_pic_parameter_set_id = pps_id;
        return pps;
    }

    bool IsProtectedMemoryEnabled() const { return protected_memory_enabled_; }
    bool IsProtectedNoFaultSupported() const { return protected_no_fault_supported_; }

    void setInstancePNext(void* pNext) { instance_pnext_ = pNext; }

  private:
    uint32_t FindQueueFamilySupportingCodecOp(VkVideoCodecOperationFlagBitsKHR codec_op) {
        uint32_t qfi = VK_QUEUE_FAMILY_IGNORED;
        for (size_t i = 0; i < queue_family_video_props_.size(); ++i) {
            if (queue_family_video_props_[i].videoCodecOperations & codec_op) {
                qfi = i;
                break;
            }
        }
        return qfi;
    }

    bool GetCodecCapabilities(VideoConfig& config) {
        if (vk.GetPhysicalDeviceVideoCapabilitiesKHR(gpu(), config.Profile(), config.Caps()) != VK_SUCCESS) {
            return false;
        }

        config.SessionCreateInfo()->maxCodedExtent = config.Caps()->minCodedExtent;

        return true;
    }

    bool GetCodecFormats(VideoConfig& config) {
        VkVideoProfileListInfoKHR video_profiles = vku::InitStructHelper();
        video_profiles.profileCount = 1;
        video_profiles.pProfiles = config.Profile();

        VkPhysicalDeviceVideoFormatInfoKHR info = vku::InitStructHelper();
        info.pNext = &video_profiles;
        uint32_t count = 0;

        VkImageUsageFlags allowed_usages = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        if (config.IsDecode()) {
            allowed_usages |= VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR | VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR;

            auto decode_caps = config.DecodeCaps();
            if (decode_caps == nullptr) {
                return false;
            }

            if (decode_caps->flags & VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR) {
                info.imageUsage = VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR;

                VkResult result = vk.GetPhysicalDeviceVideoFormatPropertiesKHR(gpu(), &info, &count, nullptr);
                if (result != VK_SUCCESS || count == 0) {
                    return false;
                }

                std::vector<VkVideoFormatPropertiesKHR> pic_props(count, vku::InitStruct<VkVideoFormatPropertiesKHR>());
                result = vk.GetPhysicalDeviceVideoFormatPropertiesKHR(gpu(), &info, &count, pic_props.data());
                if (result != VK_SUCCESS) {
                    return false;
                }

                for (uint32_t i = 0; i < count; ++i) {
                    pic_props[i].imageUsageFlags &= allowed_usages;
                }

                info.imageUsage = VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR;

                result = vk.GetPhysicalDeviceVideoFormatPropertiesKHR(gpu(), &info, &count, nullptr);
                if (result != VK_SUCCESS || count == 0) {
                    return false;
                }
                std::vector<VkVideoFormatPropertiesKHR> dpb_props(count, vku::InitStruct<VkVideoFormatPropertiesKHR>());

                result = vk.GetPhysicalDeviceVideoFormatPropertiesKHR(gpu(), &info, &count, dpb_props.data());
                if (result != VK_SUCCESS) {
                    return false;
                }

                for (uint32_t i = 0; i < count; ++i) {
                    dpb_props[i].imageUsageFlags &= allowed_usages;
                }

                config.SetFormatProps(pic_props, dpb_props);
                return true;
            } else if (decode_caps->flags & VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR) {
                info.imageUsage = VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR | VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR;

                VkResult result = vk.GetPhysicalDeviceVideoFormatPropertiesKHR(gpu(), &info, &count, nullptr);
                if (result != VK_SUCCESS || count == 0) {
                    return false;
                }
                std::vector<VkVideoFormatPropertiesKHR> dpb_props(count, vku::InitStruct<VkVideoFormatPropertiesKHR>());

                result = vk.GetPhysicalDeviceVideoFormatPropertiesKHR(gpu(), &info, &count, dpb_props.data());
                if (result != VK_SUCCESS) {
                    return false;
                }

                for (uint32_t i = 0; i < count; ++i) {
                    dpb_props[i].imageUsageFlags &= allowed_usages;
                }

                config.SetFormatProps(dpb_props, dpb_props);
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    void CollectCodecProfileCapsFormats(VideoConfig& config, std::vector<VideoConfig>& config_list) {
        VkVideoChromaSubsamplingFlagsKHR subsamplings[] = {
            VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR, VK_VIDEO_CHROMA_SUBSAMPLING_422_BIT_KHR,
            VK_VIDEO_CHROMA_SUBSAMPLING_444_BIT_KHR, VK_VIDEO_CHROMA_SUBSAMPLING_MONOCHROME_BIT_KHR};
        VkVideoComponentBitDepthFlagsKHR bit_depths[] = {VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR,
                                                         VK_VIDEO_COMPONENT_BIT_DEPTH_10_BIT_KHR,
                                                         VK_VIDEO_COMPONENT_BIT_DEPTH_12_BIT_KHR};

        for (size_t bd_idx = 0; bd_idx < sizeof(bit_depths) / sizeof(bit_depths[0]); ++bd_idx) {
            for (size_t ss_idx = 0; ss_idx < sizeof(subsamplings) / sizeof(subsamplings[0]); ++ss_idx) {
                config.Profile()->chromaSubsampling = subsamplings[ss_idx];
                config.Profile()->lumaBitDepth = bit_depths[bd_idx];
                config.Profile()->chromaBitDepth = bit_depths[bd_idx];

                if (GetCodecCapabilities(config) && GetCodecFormats(config)) {
                    config_list.push_back(config);
                }
            }
        }
    }

    void InitDecodeH264Configs(uint32_t queueFamilyIndex) {
        const VkVideoDecodeH264PictureLayoutFlagBitsKHR picture_layouts[] = {
            VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_PROGRESSIVE_KHR,
            VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_INTERLACED_INTERLEAVED_LINES_BIT_KHR};

        for (size_t i = 0; i < sizeof(picture_layouts) / sizeof(picture_layouts[0]); ++i) {
            VideoConfig config;
            auto& configs = (picture_layouts[i] == VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_PROGRESSIVE_KHR)
                                ? configs_decode_h264_
                                : configs_decode_h264_interlaced_;

            config.SetDecode();
            config.SetQueueFamilyIndex(queueFamilyIndex);

            config.Profile()->videoCodecOperation = VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR;

            auto codec_profile = new safe_VkVideoDecodeH264ProfileInfoKHR();
            codec_profile->pictureLayout = picture_layouts[i];
            config.SetCodecProfile(codec_profile);

            auto decode_caps_h264 = new safe_VkVideoDecodeH264CapabilitiesKHR();
            auto decode_caps = new safe_VkVideoDecodeCapabilitiesKHR();
            decode_caps->pNext = decode_caps_h264;
            config.SetCodecCapsChain(decode_caps);

            auto sps = new StdVideoH264SequenceParameterSet[1]();
            auto pps = new StdVideoH264PictureParameterSet[1]();

            auto add_info = new safe_VkVideoDecodeH264SessionParametersAddInfoKHR();
            add_info->stdSPSCount = 1;
            add_info->pStdSPSs = sps;
            add_info->stdPPSCount = 1;
            add_info->pStdPPSs = pps;

            auto params_info = new safe_VkVideoDecodeH264SessionParametersCreateInfoKHR();
            params_info->maxStdSPSCount = 1;
            params_info->maxStdPPSCount = 1;
            params_info->pParametersAddInfo = add_info;

            config.SetCodecSessionParamsInfo(params_info);

            StdVideoH264ProfileIdc profile_idc_list[] = {
                STD_VIDEO_H264_PROFILE_IDC_BASELINE,
                STD_VIDEO_H264_PROFILE_IDC_MAIN,
                STD_VIDEO_H264_PROFILE_IDC_HIGH,
                STD_VIDEO_H264_PROFILE_IDC_HIGH_444_PREDICTIVE,
            };

            for (size_t j = 0; j < sizeof(profile_idc_list) / sizeof(profile_idc_list[0]); ++j) {
                codec_profile->stdProfileIdc = profile_idc_list[j];
                CollectCodecProfileCapsFormats(config, configs);
            }

            configs_decode_.insert(configs_decode_.end(), configs.begin(), configs.end());
            configs_.insert(configs_.end(), configs.begin(), configs.end());
        }
    }

    void InitDecodeH265Configs(uint32_t queueFamilyIndex) {
        VideoConfig config;

        config.SetDecode();
        config.SetQueueFamilyIndex(queueFamilyIndex);

        config.Profile()->videoCodecOperation = VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR;

        auto codec_profile = new safe_VkVideoDecodeH265ProfileInfoKHR();
        codec_profile->stdProfileIdc = STD_VIDEO_H265_PROFILE_IDC_MAIN;
        config.SetCodecProfile(codec_profile);

        auto decode_caps_h265 = new safe_VkVideoDecodeH265CapabilitiesKHR();
        auto decode_caps = new safe_VkVideoDecodeCapabilitiesKHR();
        decode_caps->pNext = decode_caps_h265;
        config.SetCodecCapsChain(decode_caps);

        auto vps = new StdVideoH265VideoParameterSet[1]();
        auto sps = new StdVideoH265SequenceParameterSet[1]();
        auto pps = new StdVideoH265PictureParameterSet[1]();

        auto add_info = new safe_VkVideoDecodeH265SessionParametersAddInfoKHR();
        add_info->stdVPSCount = 1;
        add_info->pStdVPSs = vps;
        add_info->stdSPSCount = 1;
        add_info->pStdSPSs = sps;
        add_info->stdPPSCount = 1;
        add_info->pStdPPSs = pps;

        auto params_info = new safe_VkVideoDecodeH265SessionParametersCreateInfoKHR();
        params_info->maxStdVPSCount = 1;
        params_info->maxStdSPSCount = 1;
        params_info->maxStdPPSCount = 1;
        params_info->pParametersAddInfo = add_info;

        config.SetCodecSessionParamsInfo(params_info);

        StdVideoH265ProfileIdc profile_idc_list[] = {
            STD_VIDEO_H265_PROFILE_IDC_MAIN,
            STD_VIDEO_H265_PROFILE_IDC_MAIN_10,
            STD_VIDEO_H265_PROFILE_IDC_MAIN_STILL_PICTURE,
            STD_VIDEO_H265_PROFILE_IDC_FORMAT_RANGE_EXTENSIONS,
            STD_VIDEO_H265_PROFILE_IDC_SCC_EXTENSIONS,
        };

        for (size_t i = 0; i < sizeof(profile_idc_list) / sizeof(profile_idc_list[0]); ++i) {
            codec_profile->stdProfileIdc = profile_idc_list[i];
            CollectCodecProfileCapsFormats(config, configs_decode_h265_);
        }

        configs_decode_.insert(configs_decode_.end(), configs_decode_h265_.begin(), configs_decode_h265_.end());
        configs_.insert(configs_.end(), configs_decode_h265_.begin(), configs_decode_h265_.end());
    }

    void InitConfigs() {
        default_config_ = VideoConfig();

        uint32_t qfi = VK_QUEUE_FAMILY_IGNORED;

        qfi = FindQueueFamilySupportingCodecOp(VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR);
        if (qfi != VK_QUEUE_FAMILY_IGNORED) {
            InitDecodeH264Configs(qfi);
        }

        qfi = FindQueueFamilySupportingCodecOp(VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR);
        if (qfi != VK_QUEUE_FAMILY_IGNORED) {
            InitDecodeH265Configs(qfi);
        }
    }

    void* instance_pnext_ = nullptr;

    std::vector<VkQueueFamilyProperties2> queue_family_props_{};
    std::vector<VkQueueFamilyVideoPropertiesKHR> queue_family_video_props_{};
    std::vector<VkQueueFamilyQueryResultStatusPropertiesKHR> queue_family_query_result_status_props_{};
    bool protected_memory_enabled_{};
    bool protected_no_fault_supported_{};

    VideoConfig default_config_{};
    std::vector<VideoConfig> configs_{};
    std::vector<VideoConfig> configs_decode_{};
    std::vector<VideoConfig> configs_decode_h264_{};
    std::vector<VideoConfig> configs_decode_h264_interlaced_{};
    std::vector<VideoConfig> configs_decode_h265_{};
};

class VkVideoBestPracticesLayerTest : public VkVideoLayerTest {
  public:
    VkVideoBestPracticesLayerTest() { setInstancePNext(&features_); }

  private:
    VkValidationFeatureEnableEXT enables_[1] = {VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT};
    VkValidationFeatureDisableEXT disables_[2] = {VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT,
                                                  VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};
    VkValidationFeaturesEXT features_ = {VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT, nullptr, 1, enables_, 2, disables_};
};
