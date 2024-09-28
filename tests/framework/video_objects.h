/*
 * Copyright (c) 2022-2024 The Khronos Group Inc.
 * Copyright (c) 2022-2024 RasterGrid Kft.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once

#include "layer_validation_tests.h"
#include <vulkan/utility/vk_safe_struct.hpp>
#include <vk_video/vulkan_video_codecs_common.h>
#include <vk_video/vulkan_video_codec_h264std.h>
#include <vk_video/vulkan_video_codec_h264std_decode.h>
#include <vk_video/vulkan_video_codec_h264std_encode.h>
#include <vk_video/vulkan_video_codec_h265std.h>
#include <vk_video/vulkan_video_codec_h265std_decode.h>
#include <vk_video/vulkan_video_codec_h265std_encode.h>

#include <memory>
#include <vector>
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
        assert(profile_.videoCodecOperation == VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR);
        return vku::FindStructInPNextChain<VkVideoDecodeH264CapabilitiesKHR>(caps_.pNext);
    }
    const VkVideoDecodeH265CapabilitiesKHR* DecodeCapsH265() const {
        assert(profile_.videoCodecOperation == VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR);
        return vku::FindStructInPNextChain<VkVideoDecodeH265CapabilitiesKHR>(caps_.pNext);
    }
    const VkVideoEncodeCapabilitiesKHR* EncodeCaps() const {
        return vku::FindStructInPNextChain<VkVideoEncodeCapabilitiesKHR>(caps_.pNext);
    }
    const VkVideoDecodeAV1CapabilitiesKHR* DecodeCapsAV1() const {
        assert(profile_.videoCodecOperation == VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR);
        return vku::FindStructInPNextChain<VkVideoDecodeAV1CapabilitiesKHR>(caps_.pNext);
    }
    const VkVideoEncodeH264CapabilitiesKHR* EncodeCapsH264() const {
        assert(profile_.videoCodecOperation == VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR);
        return vku::FindStructInPNextChain<VkVideoEncodeH264CapabilitiesKHR>(caps_.pNext);
    }
    const VkVideoEncodeH265CapabilitiesKHR* EncodeCapsH265() const {
        assert(profile_.videoCodecOperation == VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR);
        return vku::FindStructInPNextChain<VkVideoEncodeH265CapabilitiesKHR>(caps_.pNext);
    }

    VkVideoEncodeQualityLevelPropertiesKHR* EncodeQualityLevelProps() { return encode_quality_level_props_.ptr(); }

    bool SupportsDecodeOutputDistinct() const {
        return DecodeCaps()->flags & VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_DISTINCT_BIT_KHR;
    }

    bool SupportsDecodeOutputCoincide() const {
        return DecodeCaps()->flags & VK_VIDEO_DECODE_CAPABILITY_DPB_AND_OUTPUT_COINCIDE_BIT_KHR;
    }

    VkVideoEncodeRateControlModeFlagBitsKHR GetAnySupportedRateControlMode() const {
        // Return a "real" rate control mode here, excluding the "DISABLED" mode
        assert(EncodeCaps()->rateControlModes > VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR);
        return static_cast<VkVideoEncodeRateControlModeFlagBitsKHR>(1
                                                                    << static_cast<uint32_t>(log2(EncodeCaps()->rateControlModes)));
    }

    bool SupportsRateControlMode(VkVideoEncodeRateControlModeFlagBitsKHR rate_control_mode) const {
        return rate_control_mode == VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DEFAULT_KHR ||
               (EncodeCaps()->rateControlModes & rate_control_mode) != 0;
    }

    uint32_t MaxEncodeH264MBCount() const {
        assert(profile_.videoCodecOperation == VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR);
        const uint32_t mb_size = 16;
        return ((MaxCodedExtent().width + mb_size - 1) / mb_size) * ((MaxCodedExtent().height + mb_size - 1) / mb_size);
    }

    uint32_t MaxEncodeH264MBRowCount() const {
        assert(profile_.videoCodecOperation == VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR);
        const uint32_t mb_size = 16;
        return (MaxCodedExtent().height + mb_size - 1) / mb_size;
    }

    uint32_t MaxEncodeH265CTBSize() const {
        assert(profile_.videoCodecOperation == VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR);
        return 16 * static_cast<uint32_t>(log2(EncodeCapsH265()->ctbSizes));
    }

    uint32_t MaxEncodeH265CTBCount() const {
        assert(profile_.videoCodecOperation == VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR);
        const uint32_t max_ctb_size = MaxEncodeH265CTBSize();
        return ((MaxCodedExtent().width + max_ctb_size - 1) / max_ctb_size) *
               ((MaxCodedExtent().height + max_ctb_size - 1) / max_ctb_size);
    }

    uint32_t MaxEncodeH265CTBRowCount() const {
        assert(profile_.videoCodecOperation == VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR);
        const uint32_t max_ctb_size = MaxEncodeH265CTBSize();
        return (MaxCodedExtent().height + max_ctb_size - 1) / max_ctb_size;
    }

    int32_t ClampH264Qp(int32_t qp) const {
        auto caps = EncodeCapsH264();
        return std::clamp(qp, caps->minQp, caps->maxQp);
    }

    int32_t ClampH265Qp(int32_t qp) const {
        auto caps = EncodeCapsH265();
        return std::clamp(qp, caps->minQp, caps->maxQp);
    }

    bool IsDecode() const { return is_decode_; }
    bool IsEncode() const { return is_encode_; }

    void SetDecode() { is_decode_ = true; }
    void SetEncode() { is_encode_ = true; }

    void EnableProfileIndependentResources() { use_profile_independent_resources_ = true; }
    bool UseProfileIndependentResources() const { return use_profile_independent_resources_; }

    void SetCodecCapsChain(void* codec_chain) {
        assert(caps_.pNext == nullptr);
        caps_.pNext = codec_chain;
    }

    void SetCodecEncodeQualityLevelPropsChain(void* codec_chain) {
        assert(encode_quality_level_props_.pNext == nullptr);
        encode_quality_level_props_.pNext = codec_chain;
    }

    bool NeedsSessionParams() const { return session_params_create_info_.pNext != nullptr; }

    void SetCodecSessionParamsInfo(const void* paramsInfo) {
        assert(session_params_create_info_.pNext == nullptr);
        session_params_create_info_.pNext = paramsInfo;
    }

    void SetFormatProps(const std::vector<VkVideoFormatPropertiesKHR>& picture_format_props,
                        const std::vector<VkVideoFormatPropertiesKHR>& dpb_format_props) {
        picture_format_props_ = picture_format_props;
        dpb_format_props_ = dpb_format_props;

        session_create_info_.pictureFormat = picture_format_props[0].format;
        session_create_info_.referencePictureFormat = dpb_format_props[0].format;
    }

    StdVideoH264SequenceParameterSet CreateH264SPS(uint8_t sps_id) const {
        StdVideoH264SequenceParameterSet sps{};
        sps.seq_parameter_set_id = sps_id;

        if (is_decode_) {
            auto profile = vku::FindStructInPNextChain<VkVideoDecodeH264ProfileInfoKHR>(profile_.pNext);
            assert(profile != nullptr);

            sps.profile_idc = profile->stdProfileIdc;
        } else if (is_encode_) {
            auto profile = vku::FindStructInPNextChain<VkVideoEncodeH264ProfileInfoKHR>(profile_.pNext);
            assert(profile != nullptr);

            sps.profile_idc = profile->stdProfileIdc;
        }

        sps.flags.frame_mbs_only_flag = 1;
        sps.chroma_format_idc = static_cast<StdVideoH264ChromaFormatIdc>(log2(profile_.chromaSubsampling));
        sps.bit_depth_luma_minus8 = static_cast<uint8_t>(log2(profile_.lumaBitDepth));
        sps.bit_depth_chroma_minus8 = static_cast<uint8_t>(log2(profile_.chromaBitDepth));
        sps.pic_width_in_mbs_minus1 = (caps_.minCodedExtent.width + 15) / 16 - 1;
        sps.pic_height_in_map_units_minus1 = (caps_.minCodedExtent.height + 15) / 16 - 1;

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

    StdVideoH264SequenceParameterSet* EncodeH264SPS() {
        auto create_info =
            vku::FindStructInPNextChain<VkVideoEncodeH264SessionParametersCreateInfoKHR>(session_params_create_info_.pNext);
        return const_cast<StdVideoH264SequenceParameterSet*>(create_info->pParametersAddInfo->pStdSPSs);
    }

    StdVideoH264PictureParameterSet* EncodeH264PPS() {
        auto create_info =
            vku::FindStructInPNextChain<VkVideoEncodeH264SessionParametersCreateInfoKHR>(session_params_create_info_.pNext);
        return const_cast<StdVideoH264PictureParameterSet*>(create_info->pParametersAddInfo->pStdPPSs);
    }

    StdVideoH265VideoParameterSet* EncodeH265VPS() {
        auto create_info =
            vku::FindStructInPNextChain<VkVideoEncodeH265SessionParametersCreateInfoKHR>(session_params_create_info_.pNext);
        return const_cast<StdVideoH265VideoParameterSet*>(create_info->pParametersAddInfo->pStdVPSs);
    }

    StdVideoH265SequenceParameterSet* EncodeH265SPS() {
        auto create_info =
            vku::FindStructInPNextChain<VkVideoEncodeH265SessionParametersCreateInfoKHR>(session_params_create_info_.pNext);
        return const_cast<StdVideoH265SequenceParameterSet*>(create_info->pParametersAddInfo->pStdSPSs);
    }

    StdVideoH265PictureParameterSet* EncodeH265PPS() {
        auto create_info =
            vku::FindStructInPNextChain<VkVideoEncodeH265SessionParametersCreateInfoKHR>(session_params_create_info_.pNext);
        return const_cast<StdVideoH265PictureParameterSet*>(create_info->pParametersAddInfo->pStdPPSs);
    }

  private:
    bool is_decode_{false};
    bool is_encode_{false};
    bool use_profile_independent_resources_{false};
    vku::safe_VkVideoProfileInfoKHR profile_{};
    vku::safe_VkVideoCapabilitiesKHR caps_{};
    vku::safe_VkVideoEncodeQualityLevelPropertiesKHR encode_quality_level_props_{};
    vku::safe_VkVideoSessionCreateInfoKHR session_create_info_{};
    vku::safe_VkVideoSessionParametersCreateInfoKHR session_params_create_info_{};
    std::vector<VkVideoFormatPropertiesKHR> picture_format_props_{};
    std::vector<VkVideoFormatPropertiesKHR> dpb_format_props_{};
};

class BitstreamBuffer {
  public:
    BitstreamBuffer(vkt::Device* device, const VideoConfig& config, VkDeviceSize size, bool is_protected = false)
        : device_(device),
          size_(size),
          memory_(VK_NULL_HANDLE),
          buffer_(VK_NULL_HANDLE),
          dep_info_(vku::InitStruct<VkDependencyInfo>()),
          barrier_(vku::InitStruct<VkBufferMemoryBarrier2>()) {
        if (config.UseProfileIndependentResources()) {
            Init(nullptr, size, config.Profile()->videoCodecOperation, is_protected);
        } else {
            VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
            profile_list.profileCount = 1;
            profile_list.pProfiles = config.Profile();

            Init(&profile_list, size, config.Profile()->videoCodecOperation, is_protected);
        }
    }

    ~BitstreamBuffer() { Destroy(); }

    VkDeviceSize Size() const { return size_; }
    VkBuffer Buffer() const { return buffer_; }

    const VkDependencyInfo* MemoryBarrier() { return &dep_info_; }

  private:
    void Init(const VkVideoProfileListInfoKHR* profile_list, VkDeviceSize size, VkVideoCodecOperationFlagBitsKHR codec_op,
              bool is_protected) {
        VkBufferUsageFlags usage = 0;
        switch (codec_op) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
            case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
            case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR:
                usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR;
                break;

            case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
            case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
                usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR;
                break;

            default:
                break;
        }

        {
            VkBufferCreateInfo create_info = vku::InitStructHelper();
            create_info.flags = is_protected ? VK_BUFFER_CREATE_PROTECTED_BIT : 0;
            if (profile_list != nullptr) {
                create_info.pNext = profile_list;
            } else {
                create_info.flags |= VK_BUFFER_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR;
            }
            create_info.size = size;
            create_info.usage = usage;
            create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            ASSERT_EQ(VK_SUCCESS, vk::CreateBuffer(device_->handle(), &create_info, nullptr, &buffer_));
        }

        {
            VkMemoryRequirements mem_req;
            vk::GetBufferMemoryRequirements(device_->handle(), buffer_, &mem_req);

            VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
            VkMemoryPropertyFlags mem_props = is_protected ? VK_MEMORY_PROPERTY_PROTECTED_BIT : 0;
            ASSERT_TRUE(device_->phy().SetMemoryType(mem_req.memoryTypeBits, &alloc_info, mem_props));
            alloc_info.allocationSize = mem_req.size;

            ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device_->handle(), &alloc_info, nullptr, &memory_));
            ASSERT_EQ(VK_SUCCESS, vk::BindBufferMemory(device_->handle(), buffer_, memory_, 0));
        }

        {
            dep_info_.bufferMemoryBarrierCount = 1;
            dep_info_.pBufferMemoryBarriers = &barrier_;
            barrier_.srcStageMask = 0;
            barrier_.srcAccessMask = 0;
            barrier_.dstStageMask = 0;
            barrier_.dstAccessMask = 0;

            if (usage & VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR) {
                barrier_.srcStageMask |= VK_PIPELINE_STAGE_2_VIDEO_ENCODE_BIT_KHR;
                barrier_.srcAccessMask |= VK_ACCESS_2_VIDEO_ENCODE_WRITE_BIT_KHR;
                barrier_.dstStageMask |= VK_PIPELINE_STAGE_2_VIDEO_ENCODE_BIT_KHR;
                barrier_.dstAccessMask |= VK_ACCESS_2_VIDEO_ENCODE_WRITE_BIT_KHR;
            }

            barrier_.buffer = Buffer();
            barrier_.offset = 0;
            barrier_.size = Size();
        }
    }

    void Destroy() {
        vk::DestroyBuffer(device_->handle(), buffer_, nullptr);
        vk::FreeMemory(device_->handle(), memory_, nullptr);
    }

    vkt::Device* device_{};
    VkDeviceSize size_{};
    VkDeviceMemory memory_{};
    VkBuffer buffer_{};
    VkDependencyInfo dep_info_{};
    VkBufferMemoryBarrier2 barrier_{};
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

    const VkDependencyInfo* MemoryBarrier(uint32_t first_layer = 0, uint32_t layer_count = VK_REMAINING_ARRAY_LAYERS) {
        barrier_.newLayout = barrier_.oldLayout;
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

    void Init(const VkVideoProfileListInfoKHR* profile_list, VkExtent2D extent, uint32_t layers,
              const VkVideoFormatPropertiesKHR& format_props, bool is_protected) {
        {
            VkImageCreateInfo create_info = vku::InitStructHelper();
            create_info.flags = is_protected ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;
            if (profile_list != nullptr) {
                create_info.pNext = profile_list;
            } else {
                create_info.flags |= VK_IMAGE_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR;
            }
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

            ASSERT_EQ(VK_SUCCESS, vk::CreateImage(device_->handle(), &create_info, nullptr, &image_));
        }

        {
            VkMemoryRequirements mem_req;
            vk::GetImageMemoryRequirements(device_->handle(), image_, &mem_req);

            VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
            VkMemoryPropertyFlags mem_props = is_protected ? VK_MEMORY_PROPERTY_PROTECTED_BIT : 0;
            ASSERT_TRUE(device_->phy().SetMemoryType(mem_req.memoryTypeBits, &alloc_info, mem_props));
            alloc_info.allocationSize = mem_req.size;

            ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device_->handle(), &alloc_info, nullptr, &memory_));
            ASSERT_EQ(VK_SUCCESS, vk::BindImageMemory(device_->handle(), image_, memory_, 0));
        }

        {
            VkImageViewCreateInfo create_info = vku::InitStructHelper();
            create_info.image = image_;
            create_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            create_info.format = format_props.format;
            create_info.components = format_props.componentMapping;
            create_info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layers};

            ASSERT_EQ(VK_SUCCESS, vk::CreateImageView(device_->handle(), &create_info, nullptr, &image_view_));
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
            if (format_props.imageUsageFlags &
                (VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR | VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR)) {
                barrier_.srcStageMask |= VK_PIPELINE_STAGE_2_VIDEO_ENCODE_BIT_KHR;
                barrier_.srcAccessMask |= VK_ACCESS_2_VIDEO_ENCODE_READ_BIT_KHR | VK_ACCESS_2_VIDEO_ENCODE_WRITE_BIT_KHR;
                barrier_.dstStageMask |= VK_PIPELINE_STAGE_2_VIDEO_ENCODE_BIT_KHR;
                barrier_.dstAccessMask |= VK_ACCESS_2_VIDEO_ENCODE_READ_BIT_KHR | VK_ACCESS_2_VIDEO_ENCODE_WRITE_BIT_KHR;
            }

            barrier_.image = Image();
            barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier_.subresourceRange.baseMipLevel = 0;
            barrier_.subresourceRange.levelCount = 1;
        }
    }

    void Destroy() {
        vk::DestroyImageView(device_->handle(), image_view_, nullptr);
        vk::DestroyImage(device_->handle(), image_, nullptr);
        vk::FreeMemory(device_->handle(), memory_, nullptr);
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
        if (config.UseProfileIndependentResources()) {
            Init(nullptr, config.MaxCodedExtent(), 1, *config.PictureFormatProps(), is_protected);
        } else {
            VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
            profile_list.profileCount = 1;
            profile_list.pProfiles = config.Profile();

            Init(&profile_list, config.MaxCodedExtent(), 1, *config.PictureFormatProps(), is_protected);
        }

        picture_.codedOffset = {0, 0};
        picture_.codedExtent = config.MaxCodedExtent();
        picture_.baseArrayLayer = 0;
        picture_.imageViewBinding = ImageView();
    }

    const VkVideoPictureResourceInfoKHR& Picture() const { return picture_; }

  private:
    VkVideoPictureResourceInfoKHR picture_{};
};

class VideoEncodeInput : public VideoPictureResource {
  public:
    VideoEncodeInput(vkt::Device* device, const VideoConfig& config, bool is_protected = false)
        : VideoPictureResource(device), picture_(vku::InitStruct<VkVideoPictureResourceInfoKHR>()) {
        if (config.UseProfileIndependentResources()) {
            // For profile independent encode input pictures make sure to exclude DPB usage
            // even if the implementation supports the same format for both input and DPB
            auto format_props = *config.PictureFormatProps();
            format_props.imageUsageFlags &= ~VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR;
            Init(nullptr, config.MaxCodedExtent(), 1, format_props, is_protected);
        } else {
            VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
            profile_list.profileCount = 1;
            profile_list.pProfiles = config.Profile();

            Init(&profile_list, config.MaxCodedExtent(), 1, *config.PictureFormatProps(), is_protected);
        }

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

        Init(&profile_list, config.MaxCodedExtent(), config.DpbSlotCount(), *config.DpbFormatProps(), is_protected);

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

template <typename INFOTYPE>
class VideoOpParams {
  public:
    VideoOpParams(const VideoConfig& config) : config_(&config), info_(vku::InitStruct<INFOTYPE>()) {}

    VideoOpParams(VideoOpParams<INFOTYPE> const& other) : config_(other.config_) {
        info_ = other.info_;
        info_.pNext = nullptr;
    }

    VideoOpParams<INFOTYPE>& operator=(VideoOpParams<INFOTYPE> const& other) {
        config_ = other.config_;
        info_ = other.info_;
        info_.pNext = nullptr;
        return *this;
    }

    operator const INFOTYPE&() const { return info_; }
    INFOTYPE* operator->() { return &info_; }

    INFOTYPE& Ref() { return info_; }

  protected:
    template <typename T>
    void ChainInfo(T& info, bool condition = true) {
        assert(info.sType == vku::GetSType<T>());
        if (condition) {
            assert(vku::FindStructInPNextChain<T>(info_.pNext) == nullptr);
            auto p = reinterpret_cast<VkBaseOutStructure*>(&info_);
            while (p->pNext != nullptr) {
                p = p->pNext;
            }
            p->pNext = reinterpret_cast<VkBaseOutStructure*>(&info);
        }
    }

    const VideoConfig* config_{};
    INFOTYPE info_{};
};

class VideoEncodeRateControlLayerInfo : public VideoOpParams<VkVideoEncodeRateControlLayerInfoKHR> {
  public:
    struct CodecInfoType {
        VkVideoEncodeH264RateControlLayerInfoKHR encode_h264{};
        VkVideoEncodeH265RateControlLayerInfoKHR encode_h265{};
    };

    VideoEncodeRateControlLayerInfo(const VideoConfig& config, bool include_codec_info = false)
        : VideoOpParams<VkVideoEncodeRateControlLayerInfoKHR>(config), include_codec_info_(include_codec_info), codec_info_() {
        info_.averageBitrate = std::min((uint64_t)32000, config.EncodeCaps()->maxBitrate);
        info_.maxBitrate = std::min((uint64_t)32000, config.EncodeCaps()->maxBitrate);
        info_.frameRateNumerator = 15;
        info_.frameRateDenominator = 1;

        switch (config_->Profile()->videoCodecOperation) {
            case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
                codec_info_.encode_h264 = vku::InitStructHelper();
                break;

            case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
                codec_info_.encode_h265 = vku::InitStructHelper();
                break;

            default:
                break;
        }

        IncludeCodecInfo();
    }

    VideoEncodeRateControlLayerInfo(VideoEncodeRateControlLayerInfo const& other)
        : VideoOpParams<VkVideoEncodeRateControlLayerInfoKHR>(other) {
        CopyData(other);
    }
    VideoEncodeRateControlLayerInfo& operator=(VideoEncodeRateControlLayerInfo const& other) {
        VideoOpParams<VkVideoEncodeRateControlLayerInfoKHR>::operator=(other);
        CopyData(other);
        return *this;
    }

    CodecInfoType& CodecInfo() { return codec_info_; }

  private:
    void IncludeCodecInfo() {
        switch (config_->Profile()->videoCodecOperation) {
            case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
                ChainInfo(codec_info_.encode_h264, include_codec_info_);
                break;

            case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
                ChainInfo(codec_info_.encode_h265, include_codec_info_);
                break;

            default:
                break;
        }
    }

    void CopyData(VideoEncodeRateControlLayerInfo const& other) {
        include_codec_info_ = other.include_codec_info_;
        codec_info_ = other.codec_info_;

        IncludeCodecInfo();
    }

    bool include_codec_info_{};
    CodecInfoType codec_info_{};
};

class VideoEncodeRateControlInfo : public VideoOpParams<VkVideoEncodeRateControlInfoKHR> {
  public:
    struct CodecInfoType {
        VkVideoEncodeH264RateControlInfoKHR encode_h264{};
        VkVideoEncodeH265RateControlInfoKHR encode_h265{};
    };

    VideoEncodeRateControlInfo(const VideoConfig& config, bool include_codec_info = false)
        : VideoOpParams<VkVideoEncodeRateControlInfoKHR>(config),
          layers_(),
          layer_info_(),
          include_codec_info_(include_codec_info),
          codec_info_() {
        info_.virtualBufferSizeInMs = 1000;
        info_.initialVirtualBufferSizeInMs = 0;

        switch (config_->Profile()->videoCodecOperation) {
            case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
                codec_info_.encode_h264 = vku::InitStructHelper();
                break;

            case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
                codec_info_.encode_h265 = vku::InitStructHelper();
                break;

            default:
                break;
        }

        IncludeCodecInfo();
    }

    VideoEncodeRateControlInfo(VideoEncodeRateControlInfo const& other) : VideoOpParams<VkVideoEncodeRateControlInfoKHR>(other) {
        CopyData(other);
    }
    VideoEncodeRateControlInfo& operator=(VideoEncodeRateControlInfo const& other) {
        VideoOpParams<VkVideoEncodeRateControlInfoKHR>::operator=(other);
        CopyData(other);
        return *this;
    }

    VideoEncodeRateControlInfo& SetAnyMode() {
        info_.rateControlMode = config_->GetAnySupportedRateControlMode();
        return *this;
    }

    VideoEncodeRateControlInfo& AddLayer(const VideoEncodeRateControlLayerInfo& layer_info) {
        layer_info_.push_back(layer_info);
        layers_.push_back(layer_info_.back());

        // push_back may have caused a reallocation of vector storage, hence we need to update pNext pointers
        for (size_t i = 0; i < layer_info_.size(); ++i) {
            layers_[i].pNext = layer_info_[i]->pNext;
        }

        info_.layerCount = (uint32_t)layers_.size();
        info_.pLayers = layers_.data();
        return *this;
    }

    CodecInfoType& CodecInfo() { return codec_info_; }

    // This helper proxy is for allowing the individual configured layers to be modified and then propagated
    // from the VideoEncodeRateControlLayerInfo object stored in the layer_info_ vector to the flattened
    // array of VkVideoEncodeRateControlLayerInfoKHR structures stored in the layers_ vector.
    class LayerModifierProxy {
      public:
        LayerModifierProxy(VideoEncodeRateControlInfo& parent, uint32_t layer_index, VideoEncodeRateControlLayerInfo& info)
            : parent_(parent), layer_index_(layer_index), info_(info) {}

        LayerModifierProxy(const LayerModifierProxy&) = delete;
        LayerModifierProxy& operator=(const LayerModifierProxy&) = delete;

        ~LayerModifierProxy() { parent_.UpdateLayer(layer_index_); }

        VkVideoEncodeRateControlLayerInfoKHR* operator->() { return info_.operator->(); }

        VideoEncodeRateControlLayerInfo::CodecInfoType& CodecInfo() { return info_.CodecInfo(); }

      private:
        VideoEncodeRateControlInfo& parent_;
        uint32_t layer_index_;
        VideoEncodeRateControlLayerInfo& info_;
    };

    LayerModifierProxy Layer(uint32_t index) { return LayerModifierProxy(*this, index, layer_info_[index]); }

  protected:
    void UpdateLayer(uint32_t layer_index) {
        assert(layer_index < layers_.size());
        assert(layer_index < layer_info_.size());
        layers_[layer_index] = layer_info_[layer_index];
    }

  private:
    void IncludeCodecInfo() {
        switch (config_->Profile()->videoCodecOperation) {
            case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
                ChainInfo(codec_info_.encode_h264, include_codec_info_);
                break;

            case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
                ChainInfo(codec_info_.encode_h265, include_codec_info_);
                break;

            default:
                break;
        }
    }

    void CopyData(VideoEncodeRateControlInfo const& other) {
        layer_info_ = other.layer_info_;
        include_codec_info_ = other.include_codec_info_;
        codec_info_ = other.codec_info_;

        layers_.resize(layer_info_.size());
        for (size_t i = 0; i < layer_info_.size(); ++i) {
            layers_[i] = layer_info_[i];
            assert(layers_[i].pNext == layer_info_[i]->pNext);
        }
        info_.pLayers = layers_.data();

        IncludeCodecInfo();
    }

    std::vector<VkVideoEncodeRateControlLayerInfoKHR> layers_{};
    std::vector<VideoEncodeRateControlLayerInfo> layer_info_{};
    bool include_codec_info_{};
    CodecInfoType codec_info_{};
};

class VideoBeginCodingInfo : public VideoOpParams<VkVideoBeginCodingInfoKHR> {
  public:
    struct VideoEncodeGopRemainingFrameInfo {
        VkVideoEncodeH264GopRemainingFrameInfoKHR encode_h264{};
        VkVideoEncodeH265GopRemainingFrameInfoKHR encode_h265{};
    };

    VideoBeginCodingInfo(const VideoConfig& config, VideoDPB* dpb, VkVideoSessionKHR session,
                         VkVideoSessionParametersKHR session_params)
        : VideoOpParams<VkVideoBeginCodingInfoKHR>(config),
          dpb_(dpb),
          rc_info_(config),
          slot_resources_(),
          gop_remaining_frame_info_() {
        info_.videoSession = session;
        info_.videoSessionParameters = session_params;
    }

    VideoBeginCodingInfo(VideoBeginCodingInfo const& other)
        : VideoOpParams<VkVideoBeginCodingInfoKHR>(other), rc_info_(*other.config_) {
        CopyData(other);
    }
    VideoBeginCodingInfo& operator=(VideoBeginCodingInfo const& other) {
        VideoOpParams<VkVideoBeginCodingInfoKHR>::operator=(other);
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

    VideoBeginCodingInfo& RateControl(const VideoEncodeRateControlInfo& rc_info) {
        rc_info_ = rc_info;
        ChainGopRemainingFramesIfNeeded();
        ChainInfo(rc_info_.Ref());
        return *this;
    }

    VideoBeginCodingInfo& SetSessionParams(VkVideoSessionParametersKHR session_params) {
        info_.videoSessionParameters = session_params;
        return *this;
    }

  private:
    void ChainGopRemainingFramesIfNeeded() {
        if (rc_info_->rateControlMode != VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DEFAULT_KHR &&
            rc_info_->rateControlMode != VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DISABLED_BIT_KHR) {
            switch (config_->Profile()->videoCodecOperation) {
                case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
                    if (config_->EncodeCapsH264()->requiresGopRemainingFrames) {
                        gop_remaining_frame_info_.encode_h264 = vku::InitStructHelper();
                        gop_remaining_frame_info_.encode_h264.useGopRemainingFrames = VK_TRUE;
                        gop_remaining_frame_info_.encode_h264.gopRemainingI = 5;
                        ChainInfo(gop_remaining_frame_info_.encode_h264);
                    }
                    break;

                case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
                    if (config_->EncodeCapsH265()->requiresGopRemainingFrames) {
                        gop_remaining_frame_info_.encode_h265 = vku::InitStructHelper();
                        gop_remaining_frame_info_.encode_h265.useGopRemainingFrames = VK_TRUE;
                        gop_remaining_frame_info_.encode_h265.gopRemainingI = 5;
                        ChainInfo(gop_remaining_frame_info_.encode_h265);
                    }
                    break;

                default:
                    break;
            }
        }
    }

    void CopyData(VideoBeginCodingInfo const& other) {
        dpb_ = other.dpb_;
        rc_info_ = other.rc_info_;
        slot_resources_ = other.slot_resources_;
        gop_remaining_frame_info_ = other.gop_remaining_frame_info_;

        info_.pReferenceSlots = slot_resources_.data();

        if (rc_info_->rateControlMode != VK_VIDEO_ENCODE_RATE_CONTROL_MODE_DEFAULT_KHR) {
            ChainGopRemainingFramesIfNeeded();
            ChainInfo(rc_info_.Ref());
        }
    }

    VideoDPB* dpb_{};
    VideoEncodeRateControlInfo rc_info_;
    std::vector<VkVideoReferenceSlotInfoKHR> slot_resources_{};
    VideoEncodeGopRemainingFrameInfo gop_remaining_frame_info_{};
};

class VideoCodingControlInfo : public VideoOpParams<VkVideoCodingControlInfoKHR> {
  public:
    VideoCodingControlInfo(const VideoConfig& config)
        : VideoOpParams<VkVideoCodingControlInfoKHR>(config),
          encode_rate_control_info_(config),
          encode_quality_level_info_(vku::InitStruct<VkVideoEncodeQualityLevelInfoKHR>()) {}

    VideoCodingControlInfo(VideoCodingControlInfo const& other)
        : VideoOpParams<VkVideoCodingControlInfoKHR>(other), encode_rate_control_info_(*other.config_) {
        CopyData(other);
    }
    VideoCodingControlInfo& operator=(VideoCodingControlInfo const& other) {
        VideoOpParams<VkVideoCodingControlInfoKHR>::operator=(other);
        CopyData(other);
        return *this;
    }

    VideoCodingControlInfo& Reset() {
        info_.flags |= VK_VIDEO_CODING_CONTROL_RESET_BIT_KHR;
        return *this;
    }

    VideoCodingControlInfo& RateControl(const VideoEncodeRateControlInfo& rc_info) {
        info_.flags |= VK_VIDEO_CODING_CONTROL_ENCODE_RATE_CONTROL_BIT_KHR;
        encode_rate_control_info_ = rc_info;
        ChainInfo(encode_rate_control_info_.Ref());
        return *this;
    }

    VideoCodingControlInfo& EncodeQualityLevel(uint32_t quality_level) {
        info_.flags |= VK_VIDEO_CODING_CONTROL_ENCODE_QUALITY_LEVEL_BIT_KHR;
        encode_quality_level_info_.qualityLevel = quality_level;
        ChainInfo(encode_quality_level_info_);
        return *this;
    }

  private:
    void CopyData(VideoCodingControlInfo const& other) {
        encode_rate_control_info_ = other.encode_rate_control_info_;
        ChainInfo(encode_rate_control_info_.Ref(), info_.flags & VK_VIDEO_CODING_CONTROL_ENCODE_RATE_CONTROL_BIT_KHR);

        encode_quality_level_info_ = other.encode_quality_level_info_;
        ChainInfo(encode_quality_level_info_, info_.flags & VK_VIDEO_CODING_CONTROL_ENCODE_QUALITY_LEVEL_BIT_KHR);
    }

    VideoEncodeRateControlInfo encode_rate_control_info_;
    VkVideoEncodeQualityLevelInfoKHR encode_quality_level_info_{};
};

class VideoEndCodingInfo : public VideoOpParams<VkVideoEndCodingInfoKHR> {
  public:
    VideoEndCodingInfo(const VideoConfig& config) : VideoOpParams<VkVideoEndCodingInfoKHR>(config) {}
};

class VideoDecodeInfo : public VideoOpParams<VkVideoDecodeInfoKHR> {
  public:
    struct CodecInfoType {
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
        struct {
            VkVideoDecodeAV1PictureInfoKHR picture_info{};
            StdVideoDecodeAV1PictureInfo std_picture_info{};
            std::vector<uint32_t> tile_offsets{};
            std::vector<uint32_t> tile_sizes{};
            VkVideoDecodeAV1DpbSlotInfoKHR setup_slot_info{};
            StdVideoDecodeAV1ReferenceInfo std_setup_reference_info{};
            std::vector<VkVideoDecodeAV1DpbSlotInfoKHR> dpb_slot_info{};
            std::vector<StdVideoDecodeAV1ReferenceInfo> std_reference_info{};
        } decode_av1{};
    };

    VideoDecodeInfo(const VideoConfig& config, BitstreamBuffer& bitstream, VideoDPB* dpb, const VideoDecodeOutput* output,
                    int32_t slot_index, const VkVideoPictureResourceInfoKHR* resource, bool reference = false)
        : VideoOpParams<VkVideoDecodeInfoKHR>(config),
          output_distinct_(config.SupportsDecodeOutputDistinct()),
          distinct_output_picture_(output->Picture()),
          dpb_(dpb),
          reconstructed_(vku::InitStruct<VkVideoReferenceSlotInfoKHR>()),
          references_(dpb ? dpb->PictureCount() : 0, vku::InitStruct<VkVideoReferenceSlotInfoKHR>()),
          codec_info_(),
          inline_query_info_() {
        assert(config_->IsDecode());
        assert(output != nullptr);
        info_.srcBuffer = bitstream.Buffer();
        info_.srcBufferOffset = 0;
        info_.srcBufferRange = bitstream.Size();
        info_.dstPictureResource = output->Picture();

        assert(slot_index >= 0 || config.SessionCreateInfo()->maxDpbSlots == 0);
        if (slot_index >= 0) {
            if (resource == nullptr) {
                resource = &dpb->Picture(slot_index);
            }
            reconstructed_.slotIndex = slot_index;
            reconstructed_.pPictureResource = resource;
            info_.pSetupReferenceSlot = &reconstructed_;

            if (!output_distinct_) {
                info_.dstPictureResource = *reconstructed_.pPictureResource;
            }
        }

        switch (config_->Profile()->videoCodecOperation) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
                codec_info_.decode_h264.slice_offsets = {0};

                codec_info_.decode_h264.picture_info = vku::InitStructHelper();
                codec_info_.decode_h264.picture_info.pStdPictureInfo = &codec_info_.decode_h264.std_picture_info;
                codec_info_.decode_h264.picture_info.sliceCount = (uint32_t)codec_info_.decode_h264.slice_offsets.size();
                codec_info_.decode_h264.picture_info.pSliceOffsets = codec_info_.decode_h264.slice_offsets.data();

                codec_info_.decode_h264.std_picture_info = {};
                codec_info_.decode_h264.std_picture_info.flags.is_reference = reference ? 1 : 0;

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

                ChainInfo(codec_info_.decode_h264.picture_info);
                break;

            case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
                codec_info_.decode_h265.slice_segment_offsets = {0};

                codec_info_.decode_h265.picture_info = vku::InitStructHelper();
                codec_info_.decode_h265.picture_info.pStdPictureInfo = &codec_info_.decode_h265.std_picture_info;
                codec_info_.decode_h265.picture_info.sliceSegmentCount =
                    (uint32_t)codec_info_.decode_h265.slice_segment_offsets.size();
                codec_info_.decode_h265.picture_info.pSliceSegmentOffsets = codec_info_.decode_h265.slice_segment_offsets.data();

                codec_info_.decode_h265.std_picture_info = {};
                codec_info_.decode_h265.std_picture_info.flags.IsReference = reference ? 1 : 0;

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

                ChainInfo(codec_info_.decode_h265.picture_info);
                break;

            case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR:
                codec_info_.decode_av1.tile_offsets = {0};
                codec_info_.decode_av1.tile_sizes = {256};

                codec_info_.decode_av1.picture_info = vku::InitStructHelper();
                codec_info_.decode_av1.picture_info.pStdPictureInfo = &codec_info_.decode_av1.std_picture_info;

                for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
                    codec_info_.decode_av1.picture_info.referenceNameSlotIndices[i] = -1;
                }

                codec_info_.decode_av1.picture_info.frameHeaderOffset = 0;
                codec_info_.decode_av1.picture_info.tileCount = (uint32_t)codec_info_.decode_av1.tile_offsets.size();
                codec_info_.decode_av1.picture_info.pTileOffsets = codec_info_.decode_av1.tile_offsets.data();
                codec_info_.decode_av1.picture_info.pTileSizes = codec_info_.decode_av1.tile_sizes.data();

                codec_info_.decode_av1.std_picture_info = {};
                // We will simply use the DPB slot index as the VBI slot index for the purposes of testing
                codec_info_.decode_av1.std_picture_info.refresh_frame_flags = static_cast<uint8_t>(1 << slot_index);

                reconstructed_.pNext = &codec_info_.decode_av1.setup_slot_info;

                codec_info_.decode_av1.setup_slot_info = vku::InitStructHelper();
                codec_info_.decode_av1.setup_slot_info.pStdReferenceInfo = &codec_info_.decode_av1.std_setup_reference_info;

                codec_info_.decode_av1.std_setup_reference_info = {};

                codec_info_.decode_av1.dpb_slot_info.resize(references_.size(), vku::InitStruct<VkVideoDecodeAV1DpbSlotInfoKHR>());
                codec_info_.decode_av1.std_reference_info.resize(references_.size(), {});

                for (size_t i = 0; i < references_.size(); ++i) {
                    references_[i].pNext = &codec_info_.decode_av1.dpb_slot_info[i];
                    codec_info_.decode_av1.dpb_slot_info[i].pStdReferenceInfo = &codec_info_.decode_av1.std_reference_info[i];
                }

                ChainInfo(codec_info_.decode_av1.picture_info);
                break;

            default:
                break;
        }
    }

    VideoDecodeInfo(VideoDecodeInfo const& other) : VideoOpParams<VkVideoDecodeInfoKHR>(other) { CopyData(other); }
    VideoDecodeInfo& operator=(VideoDecodeInfo const& other) {
        VideoOpParams<VkVideoDecodeInfoKHR>::operator=(other);
        CopyData(other);
        return *this;
    }

    VideoDecodeInfo& SetFrame(bool picture_only = false) {
        switch (config_->Profile()->videoCodecOperation) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
                codec_info_.decode_h264.std_picture_info.flags.field_pic_flag = 0;
                if (!picture_only) {
                    auto& setup_info = codec_info_.decode_h264.std_setup_reference_info;
                    setup_info.flags.top_field_flag = 0;
                    setup_info.flags.bottom_field_flag = 0;
                }
                return *this;

            default:
                return *this;
        }
    }

    VideoDecodeInfo& SetTopField(bool picture_only = false) {
        switch (config_->Profile()->videoCodecOperation) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
                codec_info_.decode_h264.std_picture_info.flags.field_pic_flag = 1;
                codec_info_.decode_h264.std_picture_info.flags.bottom_field_flag = 0;
                if (!picture_only) {
                    auto& setup_info = codec_info_.decode_h264.std_setup_reference_info;
                    setup_info.flags.top_field_flag = 1;
                    setup_info.flags.bottom_field_flag = 0;
                }
                return *this;

            default:
                assert(false);
                return *this;
        }
    }

    VideoDecodeInfo& SetBottomField(bool picture_only = false) {
        switch (config_->Profile()->videoCodecOperation) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
                codec_info_.decode_h264.std_picture_info.flags.field_pic_flag = 1;
                codec_info_.decode_h264.std_picture_info.flags.bottom_field_flag = 1;
                if (!picture_only) {
                    auto& setup_info = codec_info_.decode_h264.std_setup_reference_info;
                    setup_info.flags.top_field_flag = 0;
                    setup_info.flags.bottom_field_flag = 1;
                }
                return *this;

            default:
                assert(false);
                return *this;
        }
    }

    VideoDecodeInfo& ApplyFilmGrain() {
        switch (config_->Profile()->videoCodecOperation) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR:
                codec_info_.decode_av1.std_picture_info.flags.apply_grain = 1;
                // We have to force using distinct output in this case
                info_.dstPictureResource = distinct_output_picture_;
                return *this;

            default:
                assert(false);
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

    VideoDecodeInfo& AddReferenceFrame(int32_t slot_index, const VkVideoPictureResourceInfoKHR* resource) {
        assert(dpb_ != nullptr);
        size_t index = info_.referenceSlotCount++;

        references_[index].slotIndex = slot_index;
        references_[index].pPictureResource = resource;

        info_.pReferenceSlots = references_.data();

        switch (config_->Profile()->videoCodecOperation) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
                auto& info = codec_info_.decode_h264.std_reference_info[index];
                info.flags.top_field_flag = 0;
                info.flags.bottom_field_flag = 0;
                return *this;
            }

            case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR: {
                auto& info = codec_info_.decode_av1.picture_info;
                const StdVideoAV1ReferenceName def_ref_name = STD_VIDEO_AV1_REFERENCE_NAME_GOLDEN_FRAME;
                if (info.referenceNameSlotIndices[def_ref_name - STD_VIDEO_AV1_REFERENCE_NAME_LAST_FRAME] < 0) {
                    info.referenceNameSlotIndices[def_ref_name - STD_VIDEO_AV1_REFERENCE_NAME_LAST_FRAME] = slot_index;
                } else {
                    for (uint32_t i = 0; i < VK_MAX_VIDEO_AV1_REFERENCES_PER_FRAME_KHR; ++i) {
                        if (info.referenceNameSlotIndices[i] < 0) {
                            info.referenceNameSlotIndices[i] = slot_index;
                        }
                    }
                }
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

        switch (config_->Profile()->videoCodecOperation) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
                auto& info = codec_info_.decode_h264.std_reference_info[index];
                info.flags.top_field_flag = 1;
                info.flags.bottom_field_flag = 0;
                return *this;
            }

            default:
                assert(false);
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

        switch (config_->Profile()->videoCodecOperation) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
                auto& info = codec_info_.decode_h264.std_reference_info[index];
                info.flags.top_field_flag = 0;
                info.flags.bottom_field_flag = 1;
                return *this;
            }

            default:
                assert(false);
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

        switch (config_->Profile()->videoCodecOperation) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR: {
                auto& info = codec_info_.decode_h264.std_reference_info[index];
                info.flags.top_field_flag = 1;
                info.flags.bottom_field_flag = 1;
                return *this;
            }

            default:
                assert(false);
                return *this;
        }
    }

    VideoDecodeInfo& AddReferenceBothFields(int32_t slot_index, int32_t resource_index) {
        return AddReferenceBothFields(slot_index, &dpb_->Picture(resource_index));
    }

    VideoDecodeInfo& AddReferenceBothFields(int32_t slot_index) { return AddReferenceBothFields(slot_index, slot_index); }

    VideoDecodeInfo& InlineQuery(VkQueryPool pool, uint32_t first = 0, uint32_t count = 1) {
        inline_query_info_ = vku::InitStructHelper();
        inline_query_info_.queryPool = pool;
        inline_query_info_.firstQuery = first;
        inline_query_info_.queryCount = count;
        ChainInfo(inline_query_info_);
        return *this;
    }

    CodecInfoType& CodecInfo() { return codec_info_; }

  private:
    void CopyData(VideoDecodeInfo const& other) {
        output_distinct_ = other.output_distinct_;
        distinct_output_picture_ = other.distinct_output_picture_;
        dpb_ = other.dpb_;
        reconstructed_ = other.reconstructed_;
        references_ = other.references_;

        if (other.info_.pSetupReferenceSlot != nullptr) {
            info_.pSetupReferenceSlot = &reconstructed_;
        }
        if (other.info_.pReferenceSlots != nullptr) {
            info_.pReferenceSlots = references_.data();
        }

        switch (config_->Profile()->videoCodecOperation) {
            case VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR:
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

                ChainInfo(codec_info_.decode_h264.picture_info);
                break;

            case VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR:
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

                codec_info_.decode_h265.std_setup_reference_info = other.codec_info_.decode_h265.std_setup_reference_info;

                codec_info_.decode_h265.dpb_slot_info = other.codec_info_.decode_h265.dpb_slot_info;
                codec_info_.decode_h265.std_reference_info = other.codec_info_.decode_h265.std_reference_info;

                for (size_t i = 0; i < references_.size(); ++i) {
                    references_[i].pNext = &codec_info_.decode_h265.dpb_slot_info[i];
                    codec_info_.decode_h265.dpb_slot_info[i].pStdReferenceInfo = &codec_info_.decode_h265.std_reference_info[i];
                }

                ChainInfo(codec_info_.decode_h265.picture_info);
                break;

            case VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR:
                codec_info_.decode_av1.tile_offsets = other.codec_info_.decode_av1.tile_offsets;
                codec_info_.decode_av1.tile_sizes = other.codec_info_.decode_av1.tile_sizes;

                codec_info_.decode_av1.picture_info = other.codec_info_.decode_av1.picture_info;
                codec_info_.decode_av1.picture_info.pStdPictureInfo = &codec_info_.decode_av1.std_picture_info;

                codec_info_.decode_av1.picture_info.frameHeaderOffset = other.codec_info_.decode_av1.picture_info.frameHeaderOffset;
                codec_info_.decode_av1.picture_info.tileCount = (uint32_t)codec_info_.decode_av1.tile_offsets.size();
                codec_info_.decode_av1.picture_info.pTileOffsets = codec_info_.decode_av1.tile_offsets.data();
                codec_info_.decode_av1.picture_info.pTileSizes = codec_info_.decode_av1.tile_sizes.data();

                codec_info_.decode_av1.std_picture_info = other.codec_info_.decode_av1.std_picture_info;

                reconstructed_.pNext = &codec_info_.decode_av1.setup_slot_info;

                codec_info_.decode_av1.setup_slot_info = other.codec_info_.decode_av1.setup_slot_info;
                codec_info_.decode_av1.setup_slot_info.pStdReferenceInfo = &codec_info_.decode_av1.std_setup_reference_info;

                codec_info_.decode_av1.std_setup_reference_info = other.codec_info_.decode_av1.std_setup_reference_info;

                codec_info_.decode_av1.dpb_slot_info = other.codec_info_.decode_av1.dpb_slot_info;
                codec_info_.decode_av1.std_reference_info = other.codec_info_.decode_av1.std_reference_info;

                for (size_t i = 0; i < references_.size(); ++i) {
                    references_[i].pNext = &codec_info_.decode_av1.dpb_slot_info[i];
                    codec_info_.decode_av1.dpb_slot_info[i].pStdReferenceInfo = &codec_info_.decode_av1.std_reference_info[i];
                }

                ChainInfo(codec_info_.decode_av1.picture_info);
                break;

            default:
                break;
        }

        if (other.inline_query_info_.sType == VK_STRUCTURE_TYPE_VIDEO_INLINE_QUERY_INFO_KHR) {
            inline_query_info_ = other.inline_query_info_;
            ChainInfo(inline_query_info_);
        }
    }

    bool output_distinct_{};
    VkVideoPictureResourceInfoKHR distinct_output_picture_{};
    VideoDPB* dpb_{};
    VkVideoReferenceSlotInfoKHR reconstructed_{};
    std::vector<VkVideoReferenceSlotInfoKHR> references_{};
    CodecInfoType codec_info_{};
    VkVideoInlineQueryInfoKHR inline_query_info_{};
};

class VideoEncodeInfo : public VideoOpParams<VkVideoEncodeInfoKHR> {
  public:
    struct CodecInfoType {
        struct {
            VkVideoEncodeH264PictureInfoKHR picture_info{};
            StdVideoEncodeH264PictureInfo std_picture_info{};
            StdVideoEncodeH264ReferenceListsInfo std_ref_lists_info{};
            VkVideoEncodeH264NaluSliceInfoKHR slice_info{};
            StdVideoEncodeH264SliceHeader std_slice_header{};
            VkVideoEncodeH264DpbSlotInfoKHR setup_slot_info{};
            StdVideoEncodeH264ReferenceInfo std_setup_reference_info{};
            std::vector<VkVideoEncodeH264DpbSlotInfoKHR> dpb_slot_info{};
            std::vector<StdVideoEncodeH264ReferenceInfo> std_reference_info{};
        } encode_h264{};
        struct {
            VkVideoEncodeH265PictureInfoKHR picture_info{};
            StdVideoEncodeH265PictureInfo std_picture_info{};
            StdVideoEncodeH265ReferenceListsInfo std_ref_lists_info{};
            VkVideoEncodeH265NaluSliceSegmentInfoKHR slice_segment_info{};
            StdVideoEncodeH265SliceSegmentHeader std_slice_segment_header{};
            VkVideoEncodeH265DpbSlotInfoKHR setup_slot_info{};
            StdVideoEncodeH265ReferenceInfo std_setup_reference_info{};
            std::vector<VkVideoEncodeH265DpbSlotInfoKHR> dpb_slot_info{};
            std::vector<StdVideoEncodeH265ReferenceInfo> std_reference_info{};
        } encode_h265{};
    };

    VideoEncodeInfo(const VideoConfig& config, BitstreamBuffer& bitstream, VideoDPB* dpb, const VideoEncodeInput* input,
                    int32_t slot_index, const VkVideoPictureResourceInfoKHR* resource, bool reference = false)
        : VideoOpParams<VkVideoEncodeInfoKHR>(config),
          dpb_(dpb),
          reconstructed_(vku::InitStruct<VkVideoReferenceSlotInfoKHR>()),
          references_(dpb ? dpb->PictureCount() : 0, vku::InitStruct<VkVideoReferenceSlotInfoKHR>()),
          codec_info_(),
          inline_query_info_() {
        assert(config_->IsEncode());
        assert(input != nullptr);
        info_.dstBuffer = bitstream.Buffer();
        info_.dstBufferOffset = 0;
        info_.dstBufferRange = bitstream.Size();
        info_.srcPictureResource = input->Picture();

        assert(slot_index >= 0 || config.SessionCreateInfo()->maxDpbSlots == 0);
        if (slot_index >= 0) {
            if (resource == nullptr) {
                resource = &dpb->Picture(slot_index);
            }
            reconstructed_.slotIndex = slot_index;
            reconstructed_.pPictureResource = resource;
            info_.pSetupReferenceSlot = &reconstructed_;
        }

        switch (config_->Profile()->videoCodecOperation) {
            case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
                codec_info_.encode_h264.slice_info = vku::InitStructHelper();
                codec_info_.encode_h264.slice_info.pStdSliceHeader = &codec_info_.encode_h264.std_slice_header;

                codec_info_.encode_h264.picture_info = vku::InitStructHelper();
                codec_info_.encode_h264.picture_info.naluSliceEntryCount = 1;
                codec_info_.encode_h264.picture_info.pNaluSliceEntries = &codec_info_.encode_h264.slice_info;
                codec_info_.encode_h264.picture_info.pStdPictureInfo = &codec_info_.encode_h264.std_picture_info;

                codec_info_.encode_h264.std_picture_info = {};
                codec_info_.encode_h264.std_picture_info.flags.is_reference = reference ? 1 : 0;
                codec_info_.encode_h264.std_picture_info.primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_I;
                codec_info_.encode_h264.std_picture_info.pRefLists = &codec_info_.encode_h264.std_ref_lists_info;

                codec_info_.encode_h264.std_ref_lists_info = {};
                for (uint32_t i = 0; i < STD_VIDEO_H264_MAX_NUM_LIST_REF; ++i) {
                    codec_info_.encode_h264.std_ref_lists_info.RefPicList0[i] = STD_VIDEO_H264_NO_REFERENCE_PICTURE;
                    codec_info_.encode_h264.std_ref_lists_info.RefPicList1[i] = STD_VIDEO_H264_NO_REFERENCE_PICTURE;
                }

                codec_info_.encode_h264.std_slice_header = {};
                codec_info_.encode_h264.std_slice_header.slice_type = STD_VIDEO_H264_SLICE_TYPE_I;

                reconstructed_.pNext = &codec_info_.encode_h264.setup_slot_info;

                codec_info_.encode_h264.setup_slot_info = vku::InitStructHelper();
                codec_info_.encode_h264.setup_slot_info.pStdReferenceInfo = &codec_info_.encode_h264.std_setup_reference_info;

                codec_info_.encode_h264.std_setup_reference_info = {};

                codec_info_.encode_h264.dpb_slot_info.resize(references_.size(),
                                                             vku::InitStruct<VkVideoEncodeH264DpbSlotInfoKHR>());
                codec_info_.encode_h264.std_reference_info.resize(references_.size(), {});

                for (size_t i = 0; i < references_.size(); ++i) {
                    references_[i].pNext = &codec_info_.encode_h264.dpb_slot_info[i];
                    codec_info_.encode_h264.dpb_slot_info[i].pStdReferenceInfo = &codec_info_.encode_h264.std_reference_info[i];
                    codec_info_.encode_h264.std_reference_info[i].primary_pic_type = STD_VIDEO_H264_PICTURE_TYPE_I;
                }

                ChainInfo(codec_info_.encode_h264.picture_info);
                break;

            case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
                codec_info_.encode_h265.slice_segment_info = vku::InitStructHelper();
                codec_info_.encode_h265.slice_segment_info.pStdSliceSegmentHeader =
                    &codec_info_.encode_h265.std_slice_segment_header;

                codec_info_.encode_h265.picture_info = vku::InitStructHelper();
                codec_info_.encode_h265.picture_info.naluSliceSegmentEntryCount = 1;
                codec_info_.encode_h265.picture_info.pNaluSliceSegmentEntries = &codec_info_.encode_h265.slice_segment_info;
                codec_info_.encode_h265.picture_info.pStdPictureInfo = &codec_info_.encode_h265.std_picture_info;

                codec_info_.encode_h265.std_picture_info = {};
                codec_info_.encode_h265.std_picture_info.flags.is_reference = reference ? 1 : 0;
                codec_info_.encode_h265.std_picture_info.pic_type = STD_VIDEO_H265_PICTURE_TYPE_I;
                codec_info_.encode_h265.std_picture_info.pRefLists = &codec_info_.encode_h265.std_ref_lists_info;

                codec_info_.encode_h265.std_ref_lists_info = {};
                for (uint32_t i = 0; i < STD_VIDEO_H265_MAX_NUM_LIST_REF; ++i) {
                    codec_info_.encode_h265.std_ref_lists_info.RefPicList0[i] = STD_VIDEO_H265_NO_REFERENCE_PICTURE;
                    codec_info_.encode_h265.std_ref_lists_info.RefPicList1[i] = STD_VIDEO_H265_NO_REFERENCE_PICTURE;
                }

                codec_info_.encode_h265.std_slice_segment_header = {};
                codec_info_.encode_h265.std_slice_segment_header.slice_type = STD_VIDEO_H265_SLICE_TYPE_I;

                reconstructed_.pNext = &codec_info_.encode_h265.setup_slot_info;

                codec_info_.encode_h265.setup_slot_info = vku::InitStructHelper();
                codec_info_.encode_h265.setup_slot_info.pStdReferenceInfo = &codec_info_.encode_h265.std_setup_reference_info;

                codec_info_.encode_h265.std_setup_reference_info = {};

                codec_info_.encode_h265.dpb_slot_info.resize(references_.size(),
                                                             vku::InitStruct<VkVideoEncodeH265DpbSlotInfoKHR>());
                codec_info_.encode_h265.std_reference_info.resize(references_.size(), {});

                for (size_t i = 0; i < references_.size(); ++i) {
                    references_[i].pNext = &codec_info_.encode_h265.dpb_slot_info[i];
                    codec_info_.encode_h265.dpb_slot_info[i].pStdReferenceInfo = &codec_info_.encode_h265.std_reference_info[i];
                    codec_info_.encode_h265.std_reference_info[i].pic_type = STD_VIDEO_H265_PICTURE_TYPE_I;
                }

                ChainInfo(codec_info_.encode_h265.picture_info);
                break;

            default:
                break;
        }
    }

    VideoEncodeInfo(VideoEncodeInfo const& other) : VideoOpParams<VkVideoEncodeInfoKHR>(other) { CopyData(other); }
    VideoEncodeInfo& operator=(VideoEncodeInfo const& other) {
        VideoOpParams<VkVideoEncodeInfoKHR>::operator=(other);
        CopyData(other);
        return *this;
    }

    VideoEncodeInfo& SetBitstream(const BitstreamBuffer& bitstream) {
        info_.dstBuffer = bitstream.Buffer();
        info_.dstBufferOffset = 0;
        info_.dstBufferRange = bitstream.Size();
        return *this;
    }

    VideoEncodeInfo& SetBitstreamBuffer(VkBuffer bitstream, VkDeviceSize offset, VkDeviceSize range) {
        info_.dstBuffer = bitstream;
        info_.dstBufferOffset = offset;
        info_.dstBufferRange = range;
        return *this;
    }

    VideoEncodeInfo& SetEncodeInput(const VideoEncodeInput* input) {
        assert(input != nullptr);
        info_.srcPictureResource = input->Picture();
        return *this;
    }

    VideoEncodeInfo& SetEncodeInput(const VkVideoPictureResourceInfoKHR& resource) {
        info_.srcPictureResource = resource;
        return *this;
    }

    VideoEncodeInfo& AddReferenceFrame(int32_t slot_index, const VkVideoPictureResourceInfoKHR* resource) {
        return AddReferenceFrameInternal(slot_index, resource, false);
    }

    VideoEncodeInfo& AddReferenceFrame(int32_t slot_index, int32_t resource_index) {
        return AddReferenceFrame(slot_index, &dpb_->Picture(resource_index));
    }

    VideoEncodeInfo& AddReferenceFrame(int32_t slot_index) { return AddReferenceFrame(slot_index, slot_index); }

    VideoEncodeInfo& AddBackReferenceFrame(int32_t slot_index, const VkVideoPictureResourceInfoKHR* resource) {
        return AddReferenceFrameInternal(slot_index, resource, true);
    }

    VideoEncodeInfo& AddBackReferenceFrame(int32_t slot_index, int32_t resource_index) {
        return AddBackReferenceFrame(slot_index, &dpb_->Picture(resource_index));
    }

    VideoEncodeInfo& AddBackReferenceFrame(int32_t slot_index) { return AddBackReferenceFrame(slot_index, slot_index); }

    VideoEncodeInfo& InlineQuery(VkQueryPool pool, uint32_t first = 0, uint32_t count = 1) {
        inline_query_info_ = vku::InitStructHelper();
        inline_query_info_.queryPool = pool;
        inline_query_info_.firstQuery = first;
        inline_query_info_.queryCount = count;
        ChainInfo(inline_query_info_);
        return *this;
    }

    CodecInfoType& CodecInfo() { return codec_info_; }

  private:
    void CopyData(VideoEncodeInfo const& other) {
        dpb_ = other.dpb_;
        reconstructed_ = other.reconstructed_;
        references_ = other.references_;

        if (other.info_.pSetupReferenceSlot != nullptr) {
            info_.pSetupReferenceSlot = &reconstructed_;
        }
        if (other.info_.pReferenceSlots != nullptr) {
            info_.pReferenceSlots = references_.data();
        }

        switch (config_->Profile()->videoCodecOperation) {
            case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
                codec_info_.encode_h264.slice_info = other.codec_info_.encode_h264.slice_info;
                codec_info_.encode_h264.slice_info.pStdSliceHeader = &codec_info_.encode_h264.std_slice_header;

                codec_info_.encode_h264.picture_info = other.codec_info_.encode_h264.picture_info;
                codec_info_.encode_h264.picture_info.pNaluSliceEntries = &codec_info_.encode_h264.slice_info;
                codec_info_.encode_h264.picture_info.pStdPictureInfo = &codec_info_.encode_h264.std_picture_info;

                codec_info_.encode_h264.std_picture_info = other.codec_info_.encode_h264.std_picture_info;
                codec_info_.encode_h264.std_picture_info.pRefLists = &codec_info_.encode_h264.std_ref_lists_info;

                codec_info_.encode_h264.std_ref_lists_info = other.codec_info_.encode_h264.std_ref_lists_info;
                codec_info_.encode_h264.std_slice_header = other.codec_info_.encode_h264.std_slice_header;

                reconstructed_.pNext = &codec_info_.encode_h264.setup_slot_info;

                codec_info_.encode_h264.setup_slot_info = other.codec_info_.encode_h264.setup_slot_info;
                codec_info_.encode_h264.setup_slot_info.pStdReferenceInfo = &codec_info_.encode_h264.std_setup_reference_info;

                codec_info_.encode_h264.std_setup_reference_info = other.codec_info_.encode_h264.std_setup_reference_info;

                codec_info_.encode_h264.dpb_slot_info = other.codec_info_.encode_h264.dpb_slot_info;
                codec_info_.encode_h264.std_reference_info = other.codec_info_.encode_h264.std_reference_info;

                for (size_t i = 0; i < references_.size(); ++i) {
                    references_[i].pNext = &codec_info_.encode_h264.dpb_slot_info[i];
                    codec_info_.encode_h264.dpb_slot_info[i].pStdReferenceInfo = &codec_info_.encode_h264.std_reference_info[i];
                }

                ChainInfo(codec_info_.encode_h264.picture_info);
                break;

            case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
                codec_info_.encode_h265.slice_segment_info = other.codec_info_.encode_h265.slice_segment_info;
                codec_info_.encode_h265.slice_segment_info.pStdSliceSegmentHeader =
                    &codec_info_.encode_h265.std_slice_segment_header;

                codec_info_.encode_h265.picture_info = other.codec_info_.encode_h265.picture_info;
                codec_info_.encode_h265.picture_info.pNaluSliceSegmentEntries = &codec_info_.encode_h265.slice_segment_info;
                codec_info_.encode_h265.picture_info.pStdPictureInfo = &codec_info_.encode_h265.std_picture_info;

                codec_info_.encode_h265.std_picture_info = other.codec_info_.encode_h265.std_picture_info;
                codec_info_.encode_h265.std_picture_info.pRefLists = &codec_info_.encode_h265.std_ref_lists_info;

                codec_info_.encode_h265.std_ref_lists_info = other.codec_info_.encode_h265.std_ref_lists_info;
                codec_info_.encode_h265.std_slice_segment_header = other.codec_info_.encode_h265.std_slice_segment_header;

                reconstructed_.pNext = &codec_info_.encode_h265.setup_slot_info;

                codec_info_.encode_h265.setup_slot_info = other.codec_info_.encode_h265.setup_slot_info;
                codec_info_.encode_h265.setup_slot_info.pStdReferenceInfo = &codec_info_.encode_h265.std_setup_reference_info;

                codec_info_.encode_h265.std_setup_reference_info = other.codec_info_.encode_h265.std_setup_reference_info;

                codec_info_.encode_h265.dpb_slot_info = other.codec_info_.encode_h265.dpb_slot_info;
                codec_info_.encode_h265.std_reference_info = other.codec_info_.encode_h265.std_reference_info;

                for (size_t i = 0; i < references_.size(); ++i) {
                    references_[i].pNext = &codec_info_.encode_h265.dpb_slot_info[i];
                    codec_info_.encode_h265.dpb_slot_info[i].pStdReferenceInfo = &codec_info_.encode_h265.std_reference_info[i];
                }

                ChainInfo(codec_info_.encode_h265.picture_info);
                break;

            default:
                break;
        }

        if (other.inline_query_info_.sType == VK_STRUCTURE_TYPE_VIDEO_INLINE_QUERY_INFO_KHR) {
            inline_query_info_ = other.inline_query_info_;
            ChainInfo(inline_query_info_);
        }
    }

    VideoEncodeInfo& AddReferenceFrameInternal(int32_t slot_index, const VkVideoPictureResourceInfoKHR* resource,
                                               bool back_reference) {
        assert(dpb_ != nullptr);
        size_t index = info_.referenceSlotCount++;

        references_[index].slotIndex = slot_index;
        references_[index].pPictureResource = resource;

        info_.pReferenceSlots = references_.data();

        switch (config_->Profile()->videoCodecOperation) {
            case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
                if (codec_info_.encode_h264.std_picture_info.primary_pic_type != STD_VIDEO_H264_PICTURE_TYPE_B) {
                    bool use_b_pic = back_reference || config_->EncodeCapsH264()->maxPPictureL0ReferenceCount == 0;
                    codec_info_.encode_h264.std_picture_info.primary_pic_type =
                        use_b_pic ? STD_VIDEO_H264_PICTURE_TYPE_B : STD_VIDEO_H264_PICTURE_TYPE_P;
                    codec_info_.encode_h264.std_slice_header.slice_type =
                        use_b_pic ? STD_VIDEO_H264_SLICE_TYPE_B : STD_VIDEO_H264_SLICE_TYPE_P;
                }
                AddReferenceInfoH264(slot_index, !back_reference);
                break;

            case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
                if (codec_info_.encode_h265.std_picture_info.pic_type != STD_VIDEO_H265_PICTURE_TYPE_B) {
                    bool use_b_pic = back_reference || config_->EncodeCapsH265()->maxPPictureL0ReferenceCount == 0;
                    codec_info_.encode_h265.std_picture_info.pic_type =
                        use_b_pic ? STD_VIDEO_H265_PICTURE_TYPE_B : STD_VIDEO_H265_PICTURE_TYPE_P;
                    codec_info_.encode_h265.std_slice_segment_header.slice_type =
                        use_b_pic ? STD_VIDEO_H265_SLICE_TYPE_B : STD_VIDEO_H265_SLICE_TYPE_P;
                }
                AddReferenceInfoH265(slot_index, !back_reference);
                break;

            default:
                break;
        }

        return *this;
    }

    void AddReferenceInfoH264(int32_t slot_index, bool L0 = true) {
        uint8_t& active_minus1 = L0 ? codec_info_.encode_h264.std_ref_lists_info.num_ref_idx_l0_active_minus1
                                    : codec_info_.encode_h264.std_ref_lists_info.num_ref_idx_l1_active_minus1;
        uint8_t* ref_pic_list = L0 ? &codec_info_.encode_h264.std_ref_lists_info.RefPicList0[0]
                                   : &codec_info_.encode_h264.std_ref_lists_info.RefPicList1[0];

        if (ref_pic_list[0] == STD_VIDEO_H264_NO_REFERENCE_PICTURE) {
            ref_pic_list[0] = (uint8_t)slot_index;
        } else {
            ref_pic_list[++active_minus1] = (uint8_t)slot_index;
        }
    }

    void AddReferenceInfoH265(int32_t slot_index, bool L0 = true) {
        uint8_t& active_minus1 = L0 ? codec_info_.encode_h265.std_ref_lists_info.num_ref_idx_l0_active_minus1
                                    : codec_info_.encode_h265.std_ref_lists_info.num_ref_idx_l1_active_minus1;
        uint8_t* ref_pic_list = L0 ? &codec_info_.encode_h265.std_ref_lists_info.RefPicList0[0]
                                   : &codec_info_.encode_h265.std_ref_lists_info.RefPicList1[0];

        if (ref_pic_list[0] == STD_VIDEO_H265_NO_REFERENCE_PICTURE) {
            ref_pic_list[0] = (uint8_t)slot_index;
        } else {
            ref_pic_list[++active_minus1] = (uint8_t)slot_index;
        }
    }

    VideoDPB* dpb_{};
    VkVideoReferenceSlotInfoKHR reconstructed_{};
    std::vector<VkVideoReferenceSlotInfoKHR> references_{};
    CodecInfoType codec_info_{};
    VkVideoInlineQueryInfoKHR inline_query_info_{};
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
        PFN_vkGetEncodedVideoSessionParametersKHR GetEncodedVideoSessionParametersKHR;
    } vk{};

    explicit VideoContext(vkt::Device* device, const VideoConfig& config, bool protected_content = false)
        : vk(),
          config_(config),
          device_(device),
          queue_(GetQueue(device, config)),
          cmd_pool_(
              *device, queue_.family_index,
              VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | (protected_content ? VK_COMMAND_POOL_CREATE_PROTECTED_BIT : 0)),
          cmd_buffer_(*device, cmd_pool_, VK_COMMAND_BUFFER_LEVEL_PRIMARY),
          session_(VK_NULL_HANDLE),
          session_memory_(),
          session_params_(VK_NULL_HANDLE),
          status_query_pool_(VK_NULL_HANDLE),
          encode_feedback_query_pool_(VK_NULL_HANDLE),
          bitstream_(nullptr),
          dpb_(nullptr),
          decode_output_(nullptr),
          encode_input_(nullptr),
          dep_info_(vku::InitStruct<VkDependencyInfo>()),
          barrier_(vku::InitStruct<VkMemoryBarrier2>()) {
        Init(protected_content);
    }

    ~VideoContext() { Destroy(); }

    const VideoConfig& Config() const { return config_; }

    void CreateAndBindSessionMemory() {
        ASSERT_TRUE(session_ != VK_NULL_HANDLE);

        uint32_t mem_req_count = 0;
        ASSERT_EQ(VK_SUCCESS, vk.GetVideoSessionMemoryRequirementsKHR(device_->handle(), session_, &mem_req_count, nullptr));
        if (mem_req_count == 0) return;

        std::vector<VkVideoSessionMemoryRequirementsKHR> mem_reqs(mem_req_count,
                                                                  vku::InitStruct<VkVideoSessionMemoryRequirementsKHR>());
        ASSERT_EQ(VK_SUCCESS,
                  vk.GetVideoSessionMemoryRequirementsKHR(device_->handle(), session_, &mem_req_count, mem_reqs.data()));

        std::vector<VkBindVideoSessionMemoryInfoKHR> bind_info(mem_req_count, vku::InitStruct<VkBindVideoSessionMemoryInfoKHR>());
        for (uint32_t i = 0; i < mem_req_count; ++i) {
            VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();

            ASSERT_TRUE(device_->phy().SetMemoryType(mem_reqs[i].memoryRequirements.memoryTypeBits, &alloc_info, 0));
            alloc_info.allocationSize = mem_reqs[i].memoryRequirements.size;

            VkDeviceMemory memory = VK_NULL_HANDLE;
            ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device_->handle(), &alloc_info, nullptr, &memory));
            session_memory_.push_back(memory);

            bind_info[i].memoryBindIndex = mem_reqs[i].memoryBindIndex;
            bind_info[i].memory = memory;
            bind_info[i].memoryOffset = 0;
            bind_info[i].memorySize = mem_reqs[i].memoryRequirements.size;
        }

        ASSERT_EQ(VK_SUCCESS,
                  vk.BindVideoSessionMemoryKHR(device_->handle(), session_, (uint32_t)bind_info.size(), bind_info.data()));
    }

    void CreateResources(bool protected_bitstream = false, bool protected_dpb = false, bool protected_image = false) {
        VkDeviceSize buffer_size = 0;

        if (config_.IsDecode()) {
            // A small placeholder buffer should be enough as the input bitstream buffer for decode
            buffer_size = std::max((VkDeviceSize)4096, config_.Caps()->minBitstreamBufferSizeAlignment * 2);
        }
        if (config_.IsEncode()) {
            // For encode we should be conservative and request a large bitstream buffer just in case
            buffer_size = 4 * 1024 * 1024;
        }

        bitstream_ = std::unique_ptr<BitstreamBuffer>(new BitstreamBuffer(device_, config_, buffer_size, protected_bitstream));

        if (config_.SessionCreateInfo()->maxDpbSlots > 0) {
            dpb_ = std::unique_ptr<VideoDPB>(new VideoDPB(device_, config_, protected_dpb));
        }

        if (config_.IsDecode()) {
            decode_output_ = std::unique_ptr<VideoDecodeOutput>(new VideoDecodeOutput(device_, config_, protected_image));
        }

        if (config_.IsEncode()) {
            encode_input_ = std::unique_ptr<VideoEncodeInput>(new VideoEncodeInput(device_, config_, protected_image));
        }
    }

    void CreateStatusQueryPool(uint32_t query_count = 1) {
        VkQueryPoolCreateInfo create_info = vku::InitStructHelper();
        create_info.pNext = config_.Profile();
        create_info.queryType = VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR;
        create_info.queryCount = query_count;

        ASSERT_EQ(VK_SUCCESS, vk::CreateQueryPool(device_->handle(), &create_info, nullptr, &status_query_pool_));
    }

    void CreateEncodeFeedbackQueryPool(uint32_t query_count = 1, VkVideoEncodeFeedbackFlagsKHR encode_feedback_flags =
                                                                     VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BUFFER_OFFSET_BIT_KHR |
                                                                     VK_VIDEO_ENCODE_FEEDBACK_BITSTREAM_BYTES_WRITTEN_BIT_KHR) {
        assert(config_.IsEncode());

        auto encode_feedback_info = vku::InitStruct<VkQueryPoolVideoEncodeFeedbackCreateInfoKHR>();
        encode_feedback_info.pNext = config_.Profile();
        encode_feedback_info.encodeFeedbackFlags = encode_feedback_flags;

        auto create_info = vku::InitStruct<VkQueryPoolCreateInfo>(&encode_feedback_info);
        create_info.queryType = VK_QUERY_TYPE_VIDEO_ENCODE_FEEDBACK_KHR;
        create_info.queryCount = query_count;

        ASSERT_EQ(VK_SUCCESS, vk::CreateQueryPool(device_->handle(), &create_info, nullptr, &encode_feedback_query_pool_));
    }

    StdVideoH264SequenceParameterSet CreateH264SPS(uint8_t sps_id) const { return config_.CreateH264SPS(sps_id); }

    StdVideoH264PictureParameterSet CreateH264PPS(uint8_t sps_id, uint8_t pps_id) const {
        return config_.CreateH264PPS(sps_id, pps_id);
    }

    StdVideoH265VideoParameterSet CreateH265VPS(uint8_t vps_id) const { return config_.CreateH265VPS(vps_id); }

    StdVideoH265SequenceParameterSet CreateH265SPS(uint8_t vps_id, uint8_t sps_id) const {
        return config_.CreateH265SPS(vps_id, sps_id);
    }

    StdVideoH265PictureParameterSet CreateH265PPS(uint8_t vps_id, uint8_t sps_id, uint8_t pps_id) const {
        return config_.CreateH265PPS(vps_id, sps_id, pps_id);
    }

    VkVideoSessionKHR Session() { return session_; }
    VkVideoSessionParametersKHR SessionParams() { return session_params_; }
    VkQueryPool StatusQueryPool() { return status_query_pool_; }
    VkQueryPool EncodeFeedbackQueryPool() { return encode_feedback_query_pool_; }
    vkt::Queue& Queue() { return queue_; }
    vkt::CommandBuffer& CmdBuffer() { return cmd_buffer_; }

    BitstreamBuffer& Bitstream() { return *bitstream_; }
    VideoDPB* Dpb() { return dpb_.get(); }
    VideoDecodeOutput* DecodeOutput() { return decode_output_.get(); }
    VideoEncodeInput* EncodeInput() { return encode_input_.get(); }

    VideoBeginCodingInfo Begin() const { return VideoBeginCodingInfo(config_, dpb_.get(), session_, session_params_); }
    VideoCodingControlInfo Control() const { return VideoCodingControlInfo(config_); }
    VideoEndCodingInfo End() const { return VideoEndCodingInfo(config_); }

    VideoDecodeInfo DecodeFrame(int32_t slot_index = -1, const VkVideoPictureResourceInfoKHR* resource = nullptr) {
        return Decode(slot_index, resource);
    }
    VideoDecodeInfo DecodeFrame(int32_t slot_index, int32_t resource_index) {
        return Decode(slot_index, &Dpb()->Picture(resource_index));
    }

    VideoDecodeInfo DecodeTopField(int32_t slot_index = -1, const VkVideoPictureResourceInfoKHR* resource = nullptr) {
        return Decode(slot_index, resource).SetTopField();
    }
    VideoDecodeInfo DecodeTopField(int32_t slot_index, int32_t resource_index) {
        return Decode(slot_index, &Dpb()->Picture(resource_index)).SetTopField();
    }

    VideoDecodeInfo DecodeBottomField(int32_t slot_index = -1, const VkVideoPictureResourceInfoKHR* resource = nullptr) {
        return Decode(slot_index, resource).SetBottomField();
    }
    VideoDecodeInfo DecodeBottomField(int32_t slot_index, int32_t resource_index) {
        return Decode(slot_index, &Dpb()->Picture(resource_index)).SetBottomField();
    }

    VideoDecodeInfo DecodeReferenceFrame(int32_t slot_index = -1, const VkVideoPictureResourceInfoKHR* resource = nullptr) {
        return Decode(slot_index, resource, true);
    }
    VideoDecodeInfo DecodeReferenceFrame(int32_t slot_index, int32_t resource_index) {
        return Decode(slot_index, &Dpb()->Picture(resource_index), true);
    }

    VideoDecodeInfo DecodeReferenceTopField(int32_t slot_index = -1, const VkVideoPictureResourceInfoKHR* resource = nullptr) {
        return Decode(slot_index, resource, true).SetTopField();
    }
    VideoDecodeInfo DecodeReferenceTopField(int32_t slot_index, int32_t resource_index) {
        return Decode(slot_index, &Dpb()->Picture(resource_index), true).SetTopField();
    }

    VideoDecodeInfo DecodeReferenceBottomField(int32_t slot_index = -1, const VkVideoPictureResourceInfoKHR* resource = nullptr) {
        return Decode(slot_index, resource, true).SetBottomField();
    }
    VideoDecodeInfo DecodeReferenceBottomField(int32_t slot_index, int32_t resource_index) {
        return Decode(slot_index, &Dpb()->Picture(resource_index), true).SetBottomField();
    }

    VideoEncodeInfo EncodeFrame(int32_t slot_index = -1, const VkVideoPictureResourceInfoKHR* resource = nullptr) {
        return Encode(slot_index, resource);
    }
    VideoEncodeInfo EncodeFrame(int32_t slot_index, int32_t resource_index) {
        return Encode(slot_index, &Dpb()->Picture(resource_index));
    }

    VideoEncodeInfo EncodeReferenceFrame(int32_t slot_index = -1, const VkVideoPictureResourceInfoKHR* resource = nullptr) {
        return Encode(slot_index, resource, true);
    }
    VideoEncodeInfo EncodeReferenceFrame(int32_t slot_index, int32_t resource_index) {
        return Encode(slot_index, &Dpb()->Picture(resource_index), true);
    }

    const VkDependencyInfo* MemoryBarrier() {
        if (config_.IsDecode()) {
            return MemoryBarrier(VK_PIPELINE_STAGE_2_VIDEO_DECODE_BIT_KHR,
                                 VK_ACCESS_2_VIDEO_DECODE_READ_BIT_KHR | VK_ACCESS_2_VIDEO_DECODE_WRITE_BIT_KHR,
                                 VK_PIPELINE_STAGE_2_VIDEO_DECODE_BIT_KHR,
                                 VK_ACCESS_2_VIDEO_DECODE_READ_BIT_KHR | VK_ACCESS_2_VIDEO_DECODE_WRITE_BIT_KHR);
        }
        if (config_.IsEncode()) {
            return MemoryBarrier(VK_PIPELINE_STAGE_2_VIDEO_ENCODE_BIT_KHR,
                                 VK_ACCESS_2_VIDEO_ENCODE_READ_BIT_KHR | VK_ACCESS_2_VIDEO_ENCODE_WRITE_BIT_KHR,
                                 VK_PIPELINE_STAGE_2_VIDEO_ENCODE_BIT_KHR,
                                 VK_ACCESS_2_VIDEO_ENCODE_READ_BIT_KHR | VK_ACCESS_2_VIDEO_ENCODE_WRITE_BIT_KHR);
        }
        assert(false);
        dep_info_ = vku::InitStructHelper();
        return &dep_info_;
    }

    const VkDependencyInfo* MemoryBarrier(VkPipelineStageFlags2 src_stages, VkAccessFlagBits2 src_access,
                                          VkPipelineStageFlags2 dst_stages, VkAccessFlagBits2 dst_access) {
        dep_info_ = vku::InitStructHelper();
        dep_info_.memoryBarrierCount = 1;
        dep_info_.pMemoryBarriers = &barrier_;
        barrier_ = vku::InitStructHelper();
        barrier_.srcStageMask = src_stages;
        barrier_.srcAccessMask = src_access;
        barrier_.dstStageMask = dst_stages;
        barrier_.dstAccessMask = dst_access;
        return &dep_info_;
    }

  private:
    VideoDecodeInfo Decode(int32_t slot_index, const VkVideoPictureResourceInfoKHR* resource, bool reference = false) {
        return VideoDecodeInfo(config_, *bitstream_, dpb_.get(), decode_output_.get(), slot_index, resource, reference);
    }
    VideoEncodeInfo Encode(int32_t slot_index, const VkVideoPictureResourceInfoKHR* resource, bool reference = false) {
        return VideoEncodeInfo(config_, *bitstream_, dpb_.get(), encode_input_.get(), slot_index, resource, reference);
    }

    vkt::Queue GetQueue(vkt::Device* device, const VideoConfig& config) const {
        VkQueue queue = VK_NULL_HANDLE;
        assert(config.QueueFamilyIndex() != VK_QUEUE_FAMILY_IGNORED);
        vk::GetDeviceQueue(device->handle(), config.QueueFamilyIndex(), 0, &queue);
        return vkt::Queue(queue, config.QueueFamilyIndex());
    }

    void Init(bool protected_content) {
        vk.GetVideoSessionMemoryRequirementsKHR = (PFN_vkGetVideoSessionMemoryRequirementsKHR)vk::GetDeviceProcAddr(
            device_->handle(), "vkGetVideoSessionMemoryRequirementsKHR");
        ASSERT_NE(vk.GetVideoSessionMemoryRequirementsKHR, nullptr);

        vk.BindVideoSessionMemoryKHR =
            (PFN_vkBindVideoSessionMemoryKHR)vk::GetDeviceProcAddr(device_->handle(), "vkBindVideoSessionMemoryKHR");
        ASSERT_NE(vk.BindVideoSessionMemoryKHR, nullptr);

        vk.CreateVideoSessionKHR = (PFN_vkCreateVideoSessionKHR)vk::GetDeviceProcAddr(device_->handle(), "vkCreateVideoSessionKHR");
        ASSERT_NE(vk.CreateVideoSessionKHR, nullptr);

        vk.DestroyVideoSessionKHR =
            (PFN_vkDestroyVideoSessionKHR)vk::GetDeviceProcAddr(device_->handle(), "vkDestroyVideoSessionKHR");
        ASSERT_NE(vk.DestroyVideoSessionKHR, nullptr);

        vk.CreateVideoSessionParametersKHR =
            (PFN_vkCreateVideoSessionParametersKHR)vk::GetDeviceProcAddr(device_->handle(), "vkCreateVideoSessionParametersKHR");
        ASSERT_NE(vk.CreateVideoSessionParametersKHR, nullptr);

        vk.UpdateVideoSessionParametersKHR =
            (PFN_vkUpdateVideoSessionParametersKHR)vk::GetDeviceProcAddr(device_->handle(), "vkUpdateVideoSessionParametersKHR");
        ASSERT_NE(vk.UpdateVideoSessionParametersKHR, nullptr);

        vk.DestroyVideoSessionParametersKHR =
            (PFN_vkDestroyVideoSessionParametersKHR)vk::GetDeviceProcAddr(device_->handle(), "vkDestroyVideoSessionParametersKHR");
        ASSERT_NE(vk.DestroyVideoSessionParametersKHR, nullptr);

        if (config_.IsEncode()) {
            vk.GetEncodedVideoSessionParametersKHR = (PFN_vkGetEncodedVideoSessionParametersKHR)vk::GetDeviceProcAddr(
                device_->handle(), "vkGetEncodedVideoSessionParametersKHR");
            ASSERT_NE(vk.GetEncodedVideoSessionParametersKHR, nullptr);
        }

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

            ASSERT_EQ(VK_SUCCESS, vk.CreateVideoSessionKHR(device_->handle(), &create_info, nullptr, &session_));
        }

        if (config_.NeedsSessionParams()) {
            VkVideoSessionParametersCreateInfoKHR create_info = *config_.SessionParamsCreateInfo();
            create_info.videoSession = session_;

            ASSERT_EQ(VK_SUCCESS, vk.CreateVideoSessionParametersKHR(device_->handle(), &create_info, nullptr, &session_params_));
        }
    }

    void Destroy() {
        vk.DestroyVideoSessionParametersKHR(device_->handle(), session_params_, nullptr);
        vk.DestroyVideoSessionKHR(device_->handle(), session_, nullptr);

        vk::DestroyQueryPool(device_->handle(), status_query_pool_, nullptr);
        vk::DestroyQueryPool(device_->handle(), encode_feedback_query_pool_, nullptr);

        for (auto session_memory : session_memory_) {
            vk::FreeMemory(device_->handle(), session_memory, nullptr);
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
    VkQueryPool encode_feedback_query_pool_{};

    std::unique_ptr<BitstreamBuffer> bitstream_{};
    std::unique_ptr<VideoDPB> dpb_{};
    std::unique_ptr<VideoDecodeOutput> decode_output_{};
    std::unique_ptr<VideoEncodeInput> encode_input_{};

    VkDependencyInfo dep_info_{};
    VkMemoryBarrier2 barrier_{};
};

class VideoEncodeRateControlTestUtils {
  public:
    VideoEncodeRateControlTestUtils(VkLayerTest* test, VideoContext& context) : test_(test), context_(context) {}

    VideoEncodeRateControlLayerInfo CreateRateControlLayerWithMinMaxQp() const {
        const auto& config = context_.Config();
        auto rc_layer = VideoEncodeRateControlLayerInfo(config, true);
        switch (config.Profile()->videoCodecOperation) {
            case VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR:
                rc_layer.CodecInfo().encode_h264.useMinQp = VK_TRUE;
                rc_layer.CodecInfo().encode_h264.minQp.qpI = config.EncodeCapsH264()->minQp;
                rc_layer.CodecInfo().encode_h264.minQp.qpP = rc_layer.CodecInfo().encode_h264.minQp.qpI;
                rc_layer.CodecInfo().encode_h264.minQp.qpB = rc_layer.CodecInfo().encode_h264.minQp.qpI;
                rc_layer.CodecInfo().encode_h264.useMaxQp = VK_TRUE;
                rc_layer.CodecInfo().encode_h264.maxQp.qpI = config.EncodeCapsH264()->maxQp;
                rc_layer.CodecInfo().encode_h264.maxQp.qpP = rc_layer.CodecInfo().encode_h264.maxQp.qpI;
                rc_layer.CodecInfo().encode_h264.maxQp.qpB = rc_layer.CodecInfo().encode_h264.maxQp.qpI;
                break;

            case VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR:
                rc_layer.CodecInfo().encode_h265.useMinQp = VK_TRUE;
                rc_layer.CodecInfo().encode_h265.minQp.qpI = config.EncodeCapsH265()->minQp;
                rc_layer.CodecInfo().encode_h265.minQp.qpP = rc_layer.CodecInfo().encode_h265.minQp.qpI;
                rc_layer.CodecInfo().encode_h265.minQp.qpB = rc_layer.CodecInfo().encode_h265.minQp.qpI;
                rc_layer.CodecInfo().encode_h265.useMaxQp = VK_TRUE;
                rc_layer.CodecInfo().encode_h265.maxQp.qpI = config.EncodeCapsH265()->maxQp;
                rc_layer.CodecInfo().encode_h265.maxQp.qpP = rc_layer.CodecInfo().encode_h265.maxQp.qpI;
                rc_layer.CodecInfo().encode_h265.maxQp.qpB = rc_layer.CodecInfo().encode_h265.maxQp.qpI;
                break;

            default:
                assert(false);
                break;
        }
        return rc_layer;
    }

    void TestRateControlInfo(const VideoEncodeRateControlInfo& rc_info, const char* expected_vuid = nullptr,
                             const std::vector<const char*>& allowed_vuids = {}) {
        vkt::CommandBuffer& cb = context_.CmdBuffer();
        cb.begin();

        if (expected_vuid != nullptr) {
            for (auto allowed_vuid : allowed_vuids) {
                test_->Monitor().SetAllowedFailureMsg(allowed_vuid);
            }
            test_->Monitor().SetDesiredError(expected_vuid);
            cb.BeginVideoCoding(context_.Begin().RateControl(rc_info));
            test_->Monitor().VerifyFound();
            cb.BeginVideoCoding(context_.Begin());
        } else {
            cb.BeginVideoCoding(context_.Begin().RateControl(rc_info));
        }

        if (expected_vuid != nullptr) {
            for (auto allowed_vuid : allowed_vuids) {
                test_->Monitor().SetAllowedFailureMsg(allowed_vuid);
            }
            test_->Monitor().SetDesiredError(expected_vuid);
            cb.ControlVideoCoding(context_.Control().RateControl(rc_info));
            test_->Monitor().VerifyFound();
        } else {
            cb.ControlVideoCoding(context_.Control().RateControl(rc_info));
        }

        cb.EndVideoCoding(context_.End());
        cb.end();
    };

    void TestRateControlLayerInfo(const VideoEncodeRateControlLayerInfo& rc_layer_info, const char* expected_vuid = nullptr,
                                  const std::vector<const char*>& allowed_vuids = {}) {
        auto rc_info = VideoEncodeRateControlInfo(context_.Config()).SetAnyMode();
        rc_info.AddLayer(rc_layer_info);
        TestRateControlInfo(rc_info, expected_vuid, allowed_vuids);
    }

    void TestRateControlStateMismatch(const VideoEncodeRateControlInfo& rc_info) {
        vkt::CommandBuffer& cb = context_.CmdBuffer();

        cb.begin();
        cb.BeginVideoCoding(context_.Begin().RateControl(rc_info));
        cb.EndVideoCoding(context_.End());
        cb.end();

        test_->Monitor().SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-pBeginInfo-08254");
        context_.Queue().Submit(cb);
        test_->Monitor().VerifyFound();
        test_->DeviceObj()->Wait();
    }

  private:
    VkLayerTest* test_;
    VideoContext& context_;
};

class VkVideoLayerTest : public VkLayerTest {
  protected:
    struct {
        PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR GetPhysicalDeviceVideoCapabilitiesKHR;
        PFN_vkGetPhysicalDeviceVideoFormatPropertiesKHR GetPhysicalDeviceVideoFormatPropertiesKHR;
        PFN_vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR;
    } vk{};

    void EnableVideoMaintenance1() {
        assert(!initialized_);
        video_maintenance1_enabled_ = true;
    }

    void EnableProtectedMemory() {
        assert(!initialized_);
        protected_memory_enabled_ = true;
    }

    void Init() {
        initialized_ = true;

        SetTargetApiVersion(VK_API_VERSION_1_1);

        AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
        AddRequiredExtensions(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME);

        AddOptionalExtensions(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME);
        AddOptionalExtensions(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME);

        AddOptionalExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);

        // NOTE: this appears to be required for the format that is chosen in
        // VkVideoLayerTest.BeginCodingIncompatRefPicProfile
        AddOptionalExtensions(VK_EXT_YCBCR_2PLANE_444_FORMATS_EXTENSION_NAME);

        AddOptionalExtensions(VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME);
        AddOptionalExtensions(VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME);
        AddOptionalExtensions(VK_KHR_VIDEO_DECODE_AV1_EXTENSION_NAME);

        AddOptionalExtensions(VK_KHR_VIDEO_ENCODE_H264_EXTENSION_NAME);
        AddOptionalExtensions(VK_KHR_VIDEO_ENCODE_H265_EXTENSION_NAME);

        RETURN_IF_SKIP(InitFramework(instance_pnext_));

        auto prot_mem_features = vku::InitStruct<VkPhysicalDeviceProtectedMemoryFeatures>();
        auto sync2_features = vku::InitStruct<VkPhysicalDeviceSynchronization2FeaturesKHR>(&prot_mem_features);
        auto features = vku::InitStruct<VkPhysicalDeviceFeatures2>(&sync2_features);
        auto video_maintenance1_features = vku::InitStruct<VkPhysicalDeviceVideoMaintenance1FeaturesKHR>();
        if (IsExtensionsEnabled(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME)) {
            prot_mem_features.pNext = &video_maintenance1_features;
        }
        vk::GetPhysicalDeviceFeatures2(gpu(), &features);

        if (!protected_memory_enabled_) {
            prot_mem_features.protectedMemory = VK_FALSE;
        }

        if (!video_maintenance1_enabled_) {
            video_maintenance1_features.videoMaintenance1 = VK_FALSE;
        }

        if (sync2_features.synchronization2 != VK_TRUE) {
            GTEST_SKIP() << "Test requires synchronization2.";
        }

        VkPhysicalDeviceProtectedMemoryProperties prot_mem_props = vku::InitStructHelper();
        VkPhysicalDeviceProperties2 props = vku::InitStructHelper(&prot_mem_props);
        vk::GetPhysicalDeviceProperties2(gpu(), &props);

        video_maintenance1_enabled_ = (video_maintenance1_features.videoMaintenance1 == VK_TRUE);
        protected_memory_enabled_ = (prot_mem_features.protectedMemory == VK_TRUE);
        protected_no_fault_supported_ = (prot_mem_props.protectedNoFault == VK_TRUE);

        RETURN_IF_SKIP(InitState(nullptr, &features));

        vk.GetPhysicalDeviceVideoCapabilitiesKHR = (PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR)vk::GetInstanceProcAddr(
            instance(), "vkGetPhysicalDeviceVideoCapabilitiesKHR");
        ASSERT_NE(vk.GetPhysicalDeviceVideoCapabilitiesKHR, nullptr);

        vk.GetPhysicalDeviceVideoFormatPropertiesKHR = (PFN_vkGetPhysicalDeviceVideoFormatPropertiesKHR)vk::GetInstanceProcAddr(
            instance(), "vkGetPhysicalDeviceVideoFormatPropertiesKHR");
        ASSERT_NE(vk.GetPhysicalDeviceVideoFormatPropertiesKHR, nullptr);

        if (IsExtensionsEnabled(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME)) {
            vk.GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR =
                (PFN_vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR)vk::GetInstanceProcAddr(
                    instance(), "vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR");
            ASSERT_NE(vk.GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR, nullptr);
        }

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
    const VideoConfig& GetConfigInvalid() const { return config_invalid_; }

    const std::vector<VideoConfig>& GetConfigsDecode() const { return configs_decode_; }
    const VideoConfig& GetConfigDecode() const { return GetConfig(configs_decode_); }
    const VideoConfig& GetConfigDecodeInvalid() const { return config_decode_invalid_; }

    const std::vector<VideoConfig>& GetConfigsDecodeH264() const { return configs_decode_h264_; }
    const VideoConfig& GetConfigDecodeH264() const { return GetConfig(configs_decode_h264_); }
    const std::vector<VideoConfig>& GetConfigsDecodeH264Interlaced() const { return configs_decode_h264_interlaced_; }
    const VideoConfig& GetConfigDecodeH264Interlaced() const { return GetConfig(configs_decode_h264_interlaced_); }
    const std::vector<VideoConfig>& GetConfigsDecodeH265() const { return configs_decode_h265_; }
    const VideoConfig& GetConfigDecodeH265() const { return GetConfig(configs_decode_h265_); }
    const std::vector<VideoConfig>& GetConfigsDecodeAV1() const { return configs_decode_av1_; }
    const VideoConfig& GetConfigDecodeAV1() const { return GetConfig(configs_decode_av1_); }
    const std::vector<VideoConfig>& GetConfigsDecodeAV1FilmGrain() const { return configs_decode_av1_film_grain_; }
    const VideoConfig& GetConfigDecodeAV1FilmGrain() const { return GetConfig(configs_decode_av1_film_grain_); }

    const std::vector<VideoConfig>& GetConfigsEncode() const { return configs_encode_; }
    const VideoConfig& GetConfigEncode() const { return GetConfig(configs_encode_); }
    const VideoConfig& GetConfigEncodeInvalid() const { return config_encode_invalid_; }

    const std::vector<VideoConfig>& GetConfigsEncodeH264() const { return configs_encode_h264_; }
    const VideoConfig& GetConfigEncodeH264() const { return GetConfig(configs_encode_h264_); }
    const std::vector<VideoConfig>& GetConfigsEncodeH265() const { return configs_encode_h265_; }
    const VideoConfig& GetConfigEncodeH265() const { return GetConfig(configs_encode_h265_); }

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

    const std::vector<VideoConfig> GetConfigsWithRateControl(
        const std::vector<VideoConfig>& configs,
        VkVideoEncodeRateControlModeFlagsKHR oneOfModes = VK_VIDEO_ENCODE_RATE_CONTROL_MODE_CBR_BIT_KHR |
                                                          VK_VIDEO_ENCODE_RATE_CONTROL_MODE_VBR_BIT_KHR) const {
        return FilterConfigs(
            configs, [oneOfModes](const VideoConfig& config) { return (config.EncodeCaps()->rateControlModes & oneOfModes) != 0; });
    }

    const std::vector<VideoConfig> GetConfigsWithMultiLayerRateControl(const std::vector<VideoConfig>& configs) const {
        return FilterConfigs(GetConfigsWithRateControl(configs),
                             [](const VideoConfig& config) { return config.EncodeCaps()->maxRateControlLayers > 1; });
    }

    const VideoConfig& GetConfigWithParams(const std::vector<VideoConfig>& configs) const {
        for (const auto& config : configs) {
            if (config.NeedsSessionParams()) {
                return config;
            }
        }
        return default_config_;
    }

    const VideoConfig& GetConfigWithMultiEncodeQualityLevelParams(const std::vector<VideoConfig>& configs) const {
        for (const auto& config : configs) {
            assert(config.IsEncode());
            if (config.NeedsSessionParams() && config.EncodeCaps()->maxQualityLevels > 1) {
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

    bool IsVideoMaintenance1Enabled() const { return video_maintenance1_enabled_; }
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
        } else if (config.IsEncode()) {
            allowed_usages |= VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR | VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR;

            info.imageUsage = VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR;

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

            info.imageUsage = VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR;

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

            auto codec_profile = new vku::safe_VkVideoDecodeH264ProfileInfoKHR();
            codec_profile->pictureLayout = picture_layouts[i];
            config.SetCodecProfile(codec_profile);

            auto decode_caps_h264 = new vku::safe_VkVideoDecodeH264CapabilitiesKHR();
            auto decode_caps = new vku::safe_VkVideoDecodeCapabilitiesKHR();
            decode_caps->pNext = decode_caps_h264;
            config.SetCodecCapsChain(decode_caps);

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

            for (auto& added_config : configs) {
                auto sps = new StdVideoH264SequenceParameterSet[1]();
                *sps = added_config.CreateH264SPS(0);
                auto pps = new StdVideoH264PictureParameterSet[1]();
                *pps = added_config.CreateH264PPS(0, 0);

                auto add_info = new vku::safe_VkVideoDecodeH264SessionParametersAddInfoKHR();
                add_info->stdSPSCount = 1;
                add_info->pStdSPSs = sps;
                add_info->stdPPSCount = 1;
                add_info->pStdPPSs = pps;

                auto params_info = new vku::safe_VkVideoDecodeH264SessionParametersCreateInfoKHR();
                params_info->maxStdSPSCount = 1;
                params_info->maxStdPPSCount = 1;
                params_info->pParametersAddInfo = add_info;

                added_config.SetCodecSessionParamsInfo(params_info);
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

        auto codec_profile = new vku::safe_VkVideoDecodeH265ProfileInfoKHR();
        codec_profile->stdProfileIdc = STD_VIDEO_H265_PROFILE_IDC_MAIN;
        config.SetCodecProfile(codec_profile);

        auto decode_caps_h265 = new vku::safe_VkVideoDecodeH265CapabilitiesKHR();
        auto decode_caps = new vku::safe_VkVideoDecodeCapabilitiesKHR();
        decode_caps->pNext = decode_caps_h265;
        config.SetCodecCapsChain(decode_caps);

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

        for (auto& added_config : configs_decode_h265_) {
            auto vps = new StdVideoH265VideoParameterSet[1]();
            *vps = added_config.CreateH265VPS(0);
            auto sps = new StdVideoH265SequenceParameterSet[1]();
            *sps = added_config.CreateH265SPS(0, 0);
            auto pps = new StdVideoH265PictureParameterSet[1]();
            *pps = added_config.CreateH265PPS(0, 0, 0);

            auto add_info = new vku::safe_VkVideoDecodeH265SessionParametersAddInfoKHR();
            add_info->stdVPSCount = 1;
            add_info->pStdVPSs = vps;
            add_info->stdSPSCount = 1;
            add_info->pStdSPSs = sps;
            add_info->stdPPSCount = 1;
            add_info->pStdPPSs = pps;

            auto params_info = new vku::safe_VkVideoDecodeH265SessionParametersCreateInfoKHR();
            params_info->maxStdVPSCount = 1;
            params_info->maxStdSPSCount = 1;
            params_info->maxStdPPSCount = 1;
            params_info->pParametersAddInfo = add_info;

            added_config.SetCodecSessionParamsInfo(params_info);
        }

        configs_decode_.insert(configs_decode_.end(), configs_decode_h265_.begin(), configs_decode_h265_.end());
        configs_.insert(configs_.end(), configs_decode_h265_.begin(), configs_decode_h265_.end());
    }

    void InitDecodeAV1Configs(uint32_t queueFamilyIndex) {
        for (size_t i = 0; i < 2; ++i) {
            VideoConfig config;
            auto& configs = (i == 0) ? configs_decode_av1_ : configs_decode_av1_film_grain_;

            config.SetDecode();
            config.SetQueueFamilyIndex(queueFamilyIndex);

            config.Profile()->videoCodecOperation = VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR;

            auto codec_profile = new vku::safe_VkVideoDecodeAV1ProfileInfoKHR();
            codec_profile->filmGrainSupport = (i == 0) ? VK_FALSE : VK_TRUE;
            config.SetCodecProfile(codec_profile);

            auto decode_caps_av1 = new vku::safe_VkVideoDecodeAV1CapabilitiesKHR();
            auto decode_caps = new vku::safe_VkVideoDecodeCapabilitiesKHR();
            decode_caps->pNext = decode_caps_av1;
            config.SetCodecCapsChain(decode_caps);

            StdVideoAV1Profile profile_list[] = {
                STD_VIDEO_AV1_PROFILE_MAIN,
                STD_VIDEO_AV1_PROFILE_HIGH,
                STD_VIDEO_AV1_PROFILE_PROFESSIONAL,
            };

            for (size_t j = 0; j < sizeof(profile_list) / sizeof(profile_list[0]); ++j) {
                codec_profile->stdProfile = profile_list[j];
                CollectCodecProfileCapsFormats(config, configs);
            }

            for (auto& added_config : configs) {
                auto seq_header = new StdVideoAV1SequenceHeader();

                auto params_info = new vku::safe_VkVideoDecodeAV1SessionParametersCreateInfoKHR();
                params_info->pStdSequenceHeader = seq_header;

                added_config.SetCodecSessionParamsInfo(params_info);
            }

            configs_decode_.insert(configs_decode_.end(), configs.begin(), configs.end());
            configs_.insert(configs_.end(), configs.begin(), configs.end());
        }
    }

    void InitEncodeH264Configs(uint32_t queueFamilyIndex) {
        VideoConfig config;

        config.SetEncode();
        config.SetQueueFamilyIndex(queueFamilyIndex);

        config.Profile()->videoCodecOperation = VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR;

        auto codec_profile = new vku::safe_VkVideoEncodeH264ProfileInfoKHR();
        codec_profile->stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_BASELINE;
        config.SetCodecProfile(codec_profile);

        auto encode_caps_h264 = new vku::safe_VkVideoEncodeH264CapabilitiesKHR();
        auto encode_caps = new vku::safe_VkVideoEncodeCapabilitiesKHR();
        encode_caps->pNext = encode_caps_h264;
        config.SetCodecCapsChain(encode_caps);
        config.SetCodecEncodeQualityLevelPropsChain(new vku::safe_VkVideoEncodeH264QualityLevelPropertiesKHR());

        StdVideoH264ProfileIdc profile_idc_list[] = {
            STD_VIDEO_H264_PROFILE_IDC_BASELINE,
            STD_VIDEO_H264_PROFILE_IDC_MAIN,
            STD_VIDEO_H264_PROFILE_IDC_HIGH,
            STD_VIDEO_H264_PROFILE_IDC_HIGH_444_PREDICTIVE,
        };

        for (size_t i = 0; i < sizeof(profile_idc_list) / sizeof(profile_idc_list[0]); ++i) {
            codec_profile->stdProfileIdc = profile_idc_list[i];
            CollectCodecProfileCapsFormats(config, configs_encode_h264_);
        }

        for (auto& added_config : configs_encode_h264_) {
            auto sps = new StdVideoH264SequenceParameterSet[1]();
            *sps = added_config.CreateH264SPS(0);
            auto pps = new StdVideoH264PictureParameterSet[1]();
            *pps = added_config.CreateH264PPS(0, 0);

            auto add_info = new vku::safe_VkVideoEncodeH264SessionParametersAddInfoKHR();
            add_info->stdSPSCount = 1;
            add_info->pStdSPSs = sps;
            add_info->stdPPSCount = 1;
            add_info->pStdPPSs = pps;

            auto params_info = new vku::safe_VkVideoEncodeH264SessionParametersCreateInfoKHR();
            params_info->maxStdSPSCount = 1;
            params_info->maxStdPPSCount = 1;
            params_info->pParametersAddInfo = add_info;

            added_config.SetCodecSessionParamsInfo(params_info);
        }

        configs_encode_.insert(configs_encode_.end(), configs_encode_h264_.begin(), configs_encode_h264_.end());
        configs_.insert(configs_.end(), configs_encode_h264_.begin(), configs_encode_h264_.end());
    }

    void InitEncodeH265Configs(uint32_t queueFamilyIndex) {
        VideoConfig config;

        config.SetEncode();
        config.SetQueueFamilyIndex(queueFamilyIndex);

        config.Profile()->videoCodecOperation = VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR;

        auto codec_profile = new vku::safe_VkVideoEncodeH265ProfileInfoKHR();
        codec_profile->stdProfileIdc = STD_VIDEO_H265_PROFILE_IDC_MAIN;
        config.SetCodecProfile(codec_profile);

        auto encode_caps_h265 = new vku::safe_VkVideoEncodeH265CapabilitiesKHR();
        auto encode_caps = new vku::safe_VkVideoEncodeCapabilitiesKHR();
        encode_caps->pNext = encode_caps_h265;
        config.SetCodecCapsChain(encode_caps);
        config.SetCodecEncodeQualityLevelPropsChain(new vku::safe_VkVideoEncodeH265QualityLevelPropertiesKHR());

        StdVideoH265ProfileIdc profile_idc_list[] = {
            STD_VIDEO_H265_PROFILE_IDC_MAIN,
            STD_VIDEO_H265_PROFILE_IDC_MAIN_10,
            STD_VIDEO_H265_PROFILE_IDC_MAIN_STILL_PICTURE,
            STD_VIDEO_H265_PROFILE_IDC_FORMAT_RANGE_EXTENSIONS,
            STD_VIDEO_H265_PROFILE_IDC_SCC_EXTENSIONS,
        };

        for (size_t i = 0; i < sizeof(profile_idc_list) / sizeof(profile_idc_list[0]); ++i) {
            codec_profile->stdProfileIdc = profile_idc_list[i];
            CollectCodecProfileCapsFormats(config, configs_encode_h265_);
        }

        for (auto& added_config : configs_encode_h265_) {
            auto vps = new StdVideoH265VideoParameterSet[1]();
            *vps = added_config.CreateH265VPS(0);
            auto sps = new StdVideoH265SequenceParameterSet[1]();
            *sps = added_config.CreateH265SPS(0, 0);
            auto pps = new StdVideoH265PictureParameterSet[1]();
            *pps = added_config.CreateH265PPS(0, 0, 0);

            auto add_info = new vku::safe_VkVideoEncodeH265SessionParametersAddInfoKHR();
            add_info->stdVPSCount = 1;
            add_info->pStdVPSs = vps;
            add_info->stdSPSCount = 1;
            add_info->pStdSPSs = sps;
            add_info->stdPPSCount = 1;
            add_info->pStdPPSs = pps;

            auto params_info = new vku::safe_VkVideoEncodeH265SessionParametersCreateInfoKHR();
            params_info->maxStdVPSCount = 1;
            params_info->maxStdSPSCount = 1;
            params_info->maxStdPPSCount = 1;
            params_info->pParametersAddInfo = add_info;

            added_config.SetCodecSessionParamsInfo(params_info);
        }

        configs_encode_.insert(configs_encode_.end(), configs_encode_h265_.begin(), configs_encode_h265_.end());
        configs_.insert(configs_.end(), configs_encode_h265_.begin(), configs_encode_h265_.end());
    }

    void InitInvalidConfigs() {
        if (IsExtensionsEnabled(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME)) {
            auto codec_profile = new vku::safe_VkVideoDecodeH264ProfileInfoKHR();
            codec_profile->stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_INVALID;

            config_decode_invalid_.SetDecode();
            config_decode_invalid_.SetQueueFamilyIndex(0);
            config_decode_invalid_.Profile()->videoCodecOperation = VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR;
            config_decode_invalid_.Profile()->chromaSubsampling = VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR;
            config_decode_invalid_.Profile()->lumaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR;
            config_decode_invalid_.Profile()->chromaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR;
            config_decode_invalid_.SetCodecProfile(codec_profile);

            config_invalid_ = config_decode_invalid_;
        }

        if (IsExtensionsEnabled(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME)) {
            auto codec_profile = new vku::safe_VkVideoEncodeH264ProfileInfoKHR();
            codec_profile->stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_INVALID;

            config_encode_invalid_.SetEncode();
            config_encode_invalid_.SetQueueFamilyIndex(0);
            config_encode_invalid_.Profile()->videoCodecOperation = VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR;
            config_encode_invalid_.Profile()->chromaSubsampling = VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR;
            config_encode_invalid_.Profile()->lumaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR;
            config_encode_invalid_.Profile()->chromaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR;
            config_encode_invalid_.SetCodecProfile(codec_profile);
            config_encode_invalid_.SetCodecEncodeQualityLevelPropsChain(new vku::safe_VkVideoEncodeH264QualityLevelPropertiesKHR());

            config_invalid_ = config_encode_invalid_;
        }
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

        qfi = FindQueueFamilySupportingCodecOp(VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR);
        if (qfi != VK_QUEUE_FAMILY_IGNORED) {
            InitDecodeAV1Configs(qfi);
        }

        qfi = FindQueueFamilySupportingCodecOp(VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR);
        if (qfi != VK_QUEUE_FAMILY_IGNORED) {
            InitEncodeH264Configs(qfi);
        }

        qfi = FindQueueFamilySupportingCodecOp(VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR);
        if (qfi != VK_QUEUE_FAMILY_IGNORED) {
            InitEncodeH265Configs(qfi);
        }

        InitInvalidConfigs();
    }

    void* instance_pnext_ = nullptr;

    std::vector<VkQueueFamilyProperties2> queue_family_props_{};
    std::vector<VkQueueFamilyVideoPropertiesKHR> queue_family_video_props_{};
    std::vector<VkQueueFamilyQueryResultStatusPropertiesKHR> queue_family_query_result_status_props_{};

    bool initialized_{false};
    bool video_maintenance1_enabled_{false};
    bool protected_memory_enabled_{false};
    bool protected_no_fault_supported_{false};

    VideoConfig default_config_{};
    VideoConfig config_invalid_{};
    VideoConfig config_decode_invalid_{};
    VideoConfig config_encode_invalid_{};

    std::vector<VideoConfig> configs_{};

    std::vector<VideoConfig> configs_decode_{};
    std::vector<VideoConfig> configs_decode_h264_{};
    std::vector<VideoConfig> configs_decode_h264_interlaced_{};
    std::vector<VideoConfig> configs_decode_h265_{};
    std::vector<VideoConfig> configs_decode_av1_{};
    std::vector<VideoConfig> configs_decode_av1_film_grain_{};

    std::vector<VideoConfig> configs_encode_{};
    std::vector<VideoConfig> configs_encode_h264_{};
    std::vector<VideoConfig> configs_encode_h265_{};
};

class NegativeVideo : public VkVideoLayerTest {};
class PositiveVideo : public VkVideoLayerTest {};

class VkVideoSyncLayerTest : public VkVideoLayerTest {
  public:
    VkVideoSyncLayerTest() { setInstancePNext(&features_); }

  private:
    const VkValidationFeatureEnableEXT enables_[1] = {VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT};
    const VkValidationFeatureDisableEXT disables_[4] = {
        VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT, VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT,
        VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT, VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};
    VkValidationFeaturesEXT features_ = {VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT, nullptr, 1u, enables_, 4, disables_};
};

class NegativeVideoSyncVal : public VkVideoSyncLayerTest {};
class PositiveVideoSyncVal : public VkVideoSyncLayerTest {};

class VkVideoBestPracticesLayerTest : public VkVideoLayerTest {
  public:
    VkVideoBestPracticesLayerTest() { setInstancePNext(&features_); }

  private:
    VkValidationFeatureEnableEXT enables_[1] = {VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT};
    VkValidationFeatureDisableEXT disables_[2] = {VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT,
                                                  VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};
    VkValidationFeaturesEXT features_ = {VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT, nullptr, 1, enables_, 2, disables_};
};

class NegativeVideoBestPractices : public VkVideoBestPracticesLayerTest {};
