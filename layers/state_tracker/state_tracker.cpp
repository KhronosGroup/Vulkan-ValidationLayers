/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2022 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include <algorithm>

#include <vulkan/utility/vk_format_utils.h>
#include <vulkan/utility/vk_struct_helper.hpp>

#include "containers/custom_containers.h"
#include "utils/vk_layer_utils.h"

#include "generated/chassis.h"
#include "generated/state_tracker_helper.h"
#include "state_tracker/state_tracker.h"
#include "utils/shader_utils.h"
#include "sync/sync_utils.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/shader_object_state.h"

// NOTE:  Beware the lifespan of the rp_begin when holding  the return.  If the rp_begin isn't a "safe" copy, "IMAGELESS"
//        attachments won't persist past the API entry point exit.
static std::pair<uint32_t, const VkImageView *> GetFramebufferAttachments(const VkRenderPassBeginInfo &rp_begin,
                                                                          const FRAMEBUFFER_STATE &fb_state) {
    const VkImageView *attachments = fb_state.createInfo.pAttachments;
    uint32_t count = fb_state.createInfo.attachmentCount;
    if (fb_state.createInfo.flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) {
        const auto *framebuffer_attachments = vku::FindStructInPNextChain<VkRenderPassAttachmentBeginInfo>(rp_begin.pNext);
        if (framebuffer_attachments) {
            attachments = framebuffer_attachments->pAttachments;
            count = framebuffer_attachments->attachmentCount;
        }
    }
    return std::make_pair(count, attachments);
}

template <typename ImageViewPointer, typename Get>
std::vector<ImageViewPointer> GetAttachmentViewsImpl(const VkRenderPassBeginInfo &rp_begin, const FRAMEBUFFER_STATE &fb_state,
                                                     const Get &get_fn) {
    std::vector<ImageViewPointer> views;

    const auto count_attachment = GetFramebufferAttachments(rp_begin, fb_state);
    const auto attachment_count = count_attachment.first;
    const auto *attachments = count_attachment.second;
    views.resize(attachment_count, nullptr);
    for (uint32_t i = 0; i < attachment_count; i++) {
        if (attachments[i] != VK_NULL_HANDLE) {
            views[i] = get_fn(attachments[i]);
        }
    }
    return views;
}

std::vector<std::shared_ptr<const IMAGE_VIEW_STATE>> ValidationStateTracker::GetAttachmentViews(
    const VkRenderPassBeginInfo &rp_begin, const FRAMEBUFFER_STATE &fb_state) const {
    auto get_fn = [this](VkImageView handle) { return this->Get<IMAGE_VIEW_STATE>(handle); };
    return GetAttachmentViewsImpl<std::shared_ptr<const IMAGE_VIEW_STATE>>(rp_begin, fb_state, get_fn);
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
// Android-specific validation that uses types defined only with VK_USE_PLATFORM_ANDROID_KHR
// This could also move into a seperate core_validation_android.cpp file... ?

template <typename CreateInfo>
VkFormatFeatureFlags2KHR ValidationStateTracker::GetExternalFormatFeaturesANDROID(const CreateInfo *create_info) const {
    VkFormatFeatureFlags2KHR format_features = 0;
    const uint64_t external_format = GetExternalFormat(create_info->pNext);
    if ((0 != external_format)) {
        // VUID 01894 will catch if not found in map
        auto it = ahb_ext_formats_map.find(external_format);
        if (it != ahb_ext_formats_map.end()) {
            format_features = it->second;
        }
    }
    return format_features;
}

void ValidationStateTracker::PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(
    VkDevice device, const struct AHardwareBuffer *buffer, VkAndroidHardwareBufferPropertiesANDROID *pProperties,
    const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    auto ahb_format_props2 = vku::FindStructInPNextChain<VkAndroidHardwareBufferFormatProperties2ANDROID>(pProperties->pNext);
    if (ahb_format_props2) {
        ahb_ext_formats_map.insert(ahb_format_props2->externalFormat, ahb_format_props2->formatFeatures);
    } else {
        auto ahb_format_props = vku::FindStructInPNextChain<VkAndroidHardwareBufferFormatPropertiesANDROID>(pProperties->pNext);
        if (ahb_format_props) {
            ahb_ext_formats_map.insert(ahb_format_props->externalFormat,
                                       static_cast<VkFormatFeatureFlags2KHR>(ahb_format_props->formatFeatures));
        }
    }
}

#else

template <typename CreateInfo>
VkFormatFeatureFlags2KHR ValidationStateTracker::GetExternalFormatFeaturesANDROID(const CreateInfo *create_info) const {
    return 0;
}

#endif  // VK_USE_PLATFORM_ANDROID_KHR

VkFormatFeatureFlags2KHR GetImageFormatFeatures(VkPhysicalDevice physical_device, bool has_format_feature2, bool has_drm_modifiers,
                                                VkDevice device, VkImage image, VkFormat format, VkImageTiling tiling) {
    VkFormatFeatureFlags2KHR format_features = 0;

    // Add feature support according to Image Format Features (vkspec.html#resources-image-format-features)
    // if format is AHB external format then the features are already set
    if (has_format_feature2) {
        VkDrmFormatModifierPropertiesList2EXT fmt_drm_props = vku::InitStructHelper();
        auto fmt_props_3 = vku::InitStruct<VkFormatProperties3KHR>(has_drm_modifiers ? &fmt_drm_props : nullptr);
        VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&fmt_props_3);

        DispatchGetPhysicalDeviceFormatProperties2(physical_device, format, &fmt_props_2);

        fmt_props_3.linearTilingFeatures |= fmt_props_2.formatProperties.linearTilingFeatures;
        fmt_props_3.optimalTilingFeatures |= fmt_props_2.formatProperties.optimalTilingFeatures;
        fmt_props_3.bufferFeatures |= fmt_props_2.formatProperties.bufferFeatures;

        if (tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
            VkImageDrmFormatModifierPropertiesEXT drm_format_props = vku::InitStructHelper();

            // Find the image modifier
            DispatchGetImageDrmFormatModifierPropertiesEXT(device, image, &drm_format_props);

            std::vector<VkDrmFormatModifierProperties2EXT> drm_mod_props;
            drm_mod_props.resize(fmt_drm_props.drmFormatModifierCount);
            fmt_drm_props.pDrmFormatModifierProperties = &drm_mod_props[0];

            // Second query to have all the modifiers filled
            DispatchGetPhysicalDeviceFormatProperties2(physical_device, format, &fmt_props_2);

            // Look for the image modifier in the list
            for (uint32_t i = 0; i < fmt_drm_props.drmFormatModifierCount; i++) {
                if (fmt_drm_props.pDrmFormatModifierProperties[i].drmFormatModifier == drm_format_props.drmFormatModifier) {
                    format_features = fmt_drm_props.pDrmFormatModifierProperties[i].drmFormatModifierTilingFeatures;
                    break;
                }
            }
        } else {
            format_features =
                (tiling == VK_IMAGE_TILING_LINEAR) ? fmt_props_3.linearTilingFeatures : fmt_props_3.optimalTilingFeatures;
        }
    } else if (tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        VkImageDrmFormatModifierPropertiesEXT drm_format_properties = vku::InitStructHelper();
        DispatchGetImageDrmFormatModifierPropertiesEXT(device, image, &drm_format_properties);

        VkFormatProperties2 format_properties_2 = vku::InitStructHelper();
        VkDrmFormatModifierPropertiesListEXT drm_properties_list = vku::InitStructHelper();
        format_properties_2.pNext = (void *)&drm_properties_list;
        DispatchGetPhysicalDeviceFormatProperties2(physical_device, format, &format_properties_2);
        std::vector<VkDrmFormatModifierPropertiesEXT> drm_properties;
        drm_properties.resize(drm_properties_list.drmFormatModifierCount);
        drm_properties_list.pDrmFormatModifierProperties = &drm_properties[0];
        DispatchGetPhysicalDeviceFormatProperties2(physical_device, format, &format_properties_2);

        for (uint32_t i = 0; i < drm_properties_list.drmFormatModifierCount; i++) {
            if (drm_properties_list.pDrmFormatModifierProperties[i].drmFormatModifier == drm_format_properties.drmFormatModifier) {
                format_features = drm_properties_list.pDrmFormatModifierProperties[i].drmFormatModifierTilingFeatures;
                break;
            }
        }
    } else {
        VkFormatProperties format_properties;
        DispatchGetPhysicalDeviceFormatProperties(physical_device, format, &format_properties);
        format_features =
            (tiling == VK_IMAGE_TILING_LINEAR) ? format_properties.linearTilingFeatures : format_properties.optimalTilingFeatures;
    }
    return format_features;
}

std::shared_ptr<IMAGE_STATE> ValidationStateTracker::CreateImageState(VkImage img, const VkImageCreateInfo *pCreateInfo,
                                                                      VkFormatFeatureFlags2KHR features) {
    return std::make_shared<IMAGE_STATE>(this, img, pCreateInfo, features);
}

std::shared_ptr<IMAGE_STATE> ValidationStateTracker::CreateImageState(VkImage img, const VkImageCreateInfo *pCreateInfo,
                                                                      VkSwapchainKHR swapchain, uint32_t swapchain_index,
                                                                      VkFormatFeatureFlags2KHR features) {
    return std::make_shared<IMAGE_STATE>(this, img, pCreateInfo, swapchain, swapchain_index, features);
}

void ValidationStateTracker::PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkImage *pImage,
                                                       const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    VkFormatFeatureFlags2KHR format_features = 0;
    if (IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)) {
        format_features = GetExternalFormatFeaturesANDROID(pCreateInfo);
    }
    if (format_features == 0) {
        format_features = GetImageFormatFeatures(physical_device, has_format_feature2,
                                                 IsExtEnabled(device_extensions.vk_ext_image_drm_format_modifier), device, *pImage,
                                                 pCreateInfo->format, pCreateInfo->tiling);
    }
    Add(CreateImageState(*pImage, pCreateInfo, format_features));
}

void ValidationStateTracker::PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator) {
    Destroy<IMAGE_STATE>(image);
}

void ValidationStateTracker::PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image,
                                                             VkImageLayout imageLayout, const VkClearColorValue *pColor,
                                                             uint32_t rangeCount, const VkImageSubresourceRange *pRanges) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_state) {
        cb_state->RecordTransferCmd(Func::vkCmdClearColorImage, Get<IMAGE_STATE>(image));
    }
}

void ValidationStateTracker::PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image,
                                                                    VkImageLayout imageLayout,
                                                                    const VkClearDepthStencilValue *pDepthStencil,
                                                                    uint32_t rangeCount, const VkImageSubresourceRange *pRanges) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_state) {
        cb_state->RecordTransferCmd(Func::vkCmdClearDepthStencilImage, Get<IMAGE_STATE>(image));
    }
}

void ValidationStateTracker::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                       VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout,
                                                       uint32_t regionCount, const VkImageCopy *pRegions) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(Func::vkCmdCopyImage, Get<IMAGE_STATE>(srcImage), Get<IMAGE_STATE>(dstImage));
}

void ValidationStateTracker::PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer,
                                                           const VkCopyImageInfo2KHR *pCopyImageInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(Func::vkCmdCopyImage2KHR, Get<IMAGE_STATE>(pCopyImageInfo->srcImage),
                                Get<IMAGE_STATE>(pCopyImageInfo->dstImage));
}

void ValidationStateTracker::PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(Func::vkCmdCopyImage2, Get<IMAGE_STATE>(pCopyImageInfo->srcImage),
                                Get<IMAGE_STATE>(pCopyImageInfo->dstImage));
}

void ValidationStateTracker::PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                          VkImageLayout srcImageLayout, VkImage dstImage,
                                                          VkImageLayout dstImageLayout, uint32_t regionCount,
                                                          const VkImageResolve *pRegions) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(Func::vkCmdResolveImage, Get<IMAGE_STATE>(srcImage), Get<IMAGE_STATE>(dstImage));
}

void ValidationStateTracker::PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                                              const VkResolveImageInfo2KHR *pResolveImageInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(Func::vkCmdResolveImage2KHR, Get<IMAGE_STATE>(pResolveImageInfo->srcImage),
                                Get<IMAGE_STATE>(pResolveImageInfo->dstImage));
}

void ValidationStateTracker::PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer,
                                                           const VkResolveImageInfo2 *pResolveImageInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(Func::vkCmdResolveImage2, Get<IMAGE_STATE>(pResolveImageInfo->srcImage),
                                Get<IMAGE_STATE>(pResolveImageInfo->dstImage));
}

void ValidationStateTracker::PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                       VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout,
                                                       uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(Func::vkCmdBlitImage, Get<IMAGE_STATE>(srcImage), Get<IMAGE_STATE>(dstImage));
}

void ValidationStateTracker::PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer,
                                                           const VkBlitImageInfo2KHR *pBlitImageInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(Func::vkCmdBlitImage2KHR, Get<IMAGE_STATE>(pBlitImageInfo->srcImage),
                                Get<IMAGE_STATE>(pBlitImageInfo->dstImage));
}

void ValidationStateTracker::PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(Func::vkCmdBlitImage2, Get<IMAGE_STATE>(pBlitImageInfo->srcImage),
                                Get<IMAGE_STATE>(pBlitImageInfo->dstImage));
}

struct BufferAddressInfillUpdateOps {
    using Map = typename ValidationStateTracker::BufferAddressRangeMap;
    using Iterator = typename Map::iterator;
    using Value = typename Map::value_type;
    using Mapped = typename Map::mapped_type;
    using Range = typename Map::key_type;
    void infill(Map &map, const Iterator &pos, const Range &infill_range) const {
        map.insert(pos, Value(infill_range, insert_value));
    }
    void update(const Iterator &pos) const {
        auto &current_buffer_list = pos->second;
        assert(!current_buffer_list.empty());
        const auto buffer_found_it = std::find(current_buffer_list.begin(), current_buffer_list.end(), insert_value[0]);
        if (buffer_found_it == current_buffer_list.end()) {
            if (current_buffer_list.capacity() <= (current_buffer_list.size() + 1)) {
                current_buffer_list.reserve(current_buffer_list.capacity() * 2);
            }
            current_buffer_list.emplace_back(insert_value[0]);
        }
    }
    const Mapped &insert_value;
};

std::shared_ptr<BUFFER_STATE> ValidationStateTracker::CreateBufferState(VkBuffer buf, const VkBufferCreateInfo* pCreateInfo) {
    return std::make_shared<BUFFER_STATE>(this, buf, pCreateInfo);
}

void ValidationStateTracker::PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                                        const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer,
                                                        const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) return;

    std::shared_ptr<BUFFER_STATE> buffer_state = CreateBufferState(*pBuffer, pCreateInfo);

    if (pCreateInfo) {
        const auto *opaque_capture_address = vku::FindStructInPNextChain<VkBufferOpaqueCaptureAddressCreateInfo>(pCreateInfo->pNext);
        if (opaque_capture_address && (opaque_capture_address->opaqueCaptureAddress != 0)) {
            WriteLockGuard guard(buffer_address_lock_);
            // address is used for GPU-AV and ray tracing buffer validation
            buffer_state->deviceAddress = opaque_capture_address->opaqueCaptureAddress;
            const auto address_range = buffer_state->DeviceAddressRange();

            BufferAddressInfillUpdateOps ops{{buffer_state.get()}};
            sparse_container::infill_update_range(buffer_address_map_, address_range, ops);
        }

        const VkBufferUsageFlags descriptor_buffer_usages =
            VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

        if ((buffer_state->usage & descriptor_buffer_usages) != 0) {
            descriptorBufferAddressSpaceSize += pCreateInfo->size;

            if ((buffer_state->usage & VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT) != 0)
                resourceDescriptorBufferAddressSpaceSize += pCreateInfo->size;

            if ((buffer_state->usage & VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT) != 0)
                samplerDescriptorBufferAddressSpaceSize += pCreateInfo->size;
        }
    }
    Add(std::move(buffer_state));
}

std::shared_ptr<BUFFER_VIEW_STATE> ValidationStateTracker::CreateBufferViewState(const std::shared_ptr<BUFFER_STATE> &bf,
                                                                                 VkBufferView bv,
                                                                                 const VkBufferViewCreateInfo *ci,
                                                                                 VkFormatFeatureFlags2KHR buf_ff) {
    return std::make_shared<BUFFER_VIEW_STATE>(bf, bv, ci, buf_ff);
}

void ValidationStateTracker::PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator, VkBufferView *pView,
                                                            const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) return;

    auto buffer_state = Get<BUFFER_STATE>(pCreateInfo->buffer);

    VkFormatFeatureFlags2KHR buffer_features;
    if (has_format_feature2) {
        VkFormatProperties3KHR fmt_props_3 = vku::InitStructHelper();
        VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&fmt_props_3);
        DispatchGetPhysicalDeviceFormatProperties2(physical_device, pCreateInfo->format, &fmt_props_2);
        buffer_features = fmt_props_3.bufferFeatures | fmt_props_2.formatProperties.bufferFeatures;
    } else {
        VkFormatProperties format_properties;
        DispatchGetPhysicalDeviceFormatProperties(physical_device, pCreateInfo->format, &format_properties);
        buffer_features = format_properties.bufferFeatures;
    }

    Add(CreateBufferViewState(buffer_state, *pView, pCreateInfo, buffer_features));
}

std::shared_ptr<IMAGE_VIEW_STATE> ValidationStateTracker::CreateImageViewState(
    const std::shared_ptr<IMAGE_STATE> &image_state, VkImageView iv, const VkImageViewCreateInfo *ci, VkFormatFeatureFlags2KHR ff,
    const VkFilterCubicImageViewImageFormatPropertiesEXT &cubic_props) {
    return std::make_shared<IMAGE_VIEW_STATE>(image_state, iv, ci, ff, cubic_props);
}

void ValidationStateTracker::PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                                           const VkAllocationCallbacks *pAllocator, VkImageView *pView,
                                                           const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) return;
    auto image_state = Get<IMAGE_STATE>(pCreateInfo->image);

    VkFormatFeatureFlags2KHR format_features = 0;
    if (image_state->HasAHBFormat() == true) {
        // The ImageView uses same Image's format feature since they share same AHB
        format_features = image_state->format_features;
    } else {
        format_features = GetImageFormatFeatures(physical_device, has_format_feature2,
                                                 IsExtEnabled(device_extensions.vk_ext_image_drm_format_modifier), device,
                                                 image_state->image(), pCreateInfo->format, image_state->createInfo.tiling);
    }

    // filter_cubic_props is used in CmdDraw validation. But it takes a lot of performance if it does in CmdDraw.
    VkFilterCubicImageViewImageFormatPropertiesEXT filter_cubic_props = vku::InitStructHelper();
    if (IsExtEnabled(device_extensions.vk_ext_filter_cubic)) {
        VkPhysicalDeviceImageViewImageFormatInfoEXT imageview_format_info = vku::InitStructHelper();
        imageview_format_info.imageViewType = pCreateInfo->viewType;
        VkPhysicalDeviceImageFormatInfo2 image_format_info = vku::InitStructHelper(&imageview_format_info);
        image_format_info.type = image_state->createInfo.imageType;
        image_format_info.format = image_state->createInfo.format;
        image_format_info.tiling = image_state->createInfo.tiling;
        auto usage_create_info = vku::FindStructInPNextChain<VkImageViewUsageCreateInfo>(pCreateInfo->pNext);
        image_format_info.usage = usage_create_info ? usage_create_info->usage : image_state->createInfo.usage;
        image_format_info.flags = image_state->createInfo.flags;

        VkImageFormatProperties2 image_format_properties = vku::InitStructHelper(&filter_cubic_props);

        DispatchGetPhysicalDeviceImageFormatProperties2(physical_device, &image_format_info, &image_format_properties);
    }

    Add(CreateImageViewState(image_state, *pView, pCreateInfo, format_features, filter_cubic_props));
}

void ValidationStateTracker::PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                                        uint32_t regionCount, const VkBufferCopy *pRegions) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(Func::vkCmdCopyBuffer, Get<BUFFER_STATE>(srcBuffer), Get<BUFFER_STATE>(dstBuffer));
}

void ValidationStateTracker::PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer,
                                                            const VkCopyBufferInfo2KHR *pCopyBufferInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(Func::vkCmdCopyBuffer2KHR, Get<BUFFER_STATE>(pCopyBufferInfo->srcBuffer),
                                Get<BUFFER_STATE>(pCopyBufferInfo->dstBuffer));
}

void ValidationStateTracker::PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(Func::vkCmdCopyBuffer2, Get<BUFFER_STATE>(pCopyBufferInfo->srcBuffer),
                                Get<BUFFER_STATE>(pCopyBufferInfo->dstBuffer));
}

void ValidationStateTracker::PreCallRecordDestroyImageView(VkDevice device, VkImageView imageView,
                                                           const VkAllocationCallbacks *pAllocator) {
    Destroy<IMAGE_VIEW_STATE>(imageView);
}

void ValidationStateTracker::PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator) {
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    if (buffer_state) {
        WriteLockGuard guard(buffer_address_lock_);

        const VkBufferUsageFlags descriptor_buffer_usages =
            VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

        if ((buffer_state->usage & descriptor_buffer_usages) != 0) {
            descriptorBufferAddressSpaceSize -= buffer_state->createInfo.size;

            if (buffer_state->usage & VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT)
                resourceDescriptorBufferAddressSpaceSize -= buffer_state->createInfo.size;

            if (buffer_state->usage & VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT)
                samplerDescriptorBufferAddressSpaceSize -= buffer_state->createInfo.size;
        }

        if (buffer_state->deviceAddress != 0) {
            const auto address_range = buffer_state->DeviceAddressRange();

            buffer_address_map_.erase_range_or_touch(address_range, [buffer_state_raw = buffer_state.get()](auto &buffers) {
                assert(!buffers.empty());
                const auto buffer_found_it = std::find(buffers.begin(), buffers.end(), buffer_state_raw);
                assert(buffer_found_it != buffers.end());

                // If buffer list only has one element, remove range map entry.
                // Else, remove target buffer from buffer list.
                if (buffer_found_it != buffers.end()) {
                    if (buffers.size() == 1) {
                        return true;
                    } else {
                        assert(!buffers.empty());
                        const size_t i = std::distance(buffers.begin(), buffer_found_it);
                        std::swap(buffers[i], buffers[buffers.size() - 1]);
                        buffers.resize(buffers.size() - 1);
                        return false;
                    }
                }

                return false;
            });
        }
    }
    Destroy<BUFFER_STATE>(buffer);
}

void ValidationStateTracker::PreCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView,
                                                            const VkAllocationCallbacks *pAllocator) {
    Destroy<BUFFER_VIEW_STATE>(bufferView);
}

void ValidationStateTracker::PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                        VkDeviceSize size, uint32_t data) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(Func::vkCmdFillBuffer, Get<BUFFER_STATE>(dstBuffer));
}

void ValidationStateTracker::PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                               VkImageLayout srcImageLayout, VkBuffer dstBuffer,
                                                               uint32_t regionCount, const VkBufferImageCopy *pRegions) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->RecordTransferCmd(Func::vkCmdCopyImageToBuffer, Get<IMAGE_STATE>(srcImage), Get<BUFFER_STATE>(dstBuffer));
}

void ValidationStateTracker::PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                                   const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(Func::vkCmdCopyImageToBuffer2KHR, Get<IMAGE_STATE>(pCopyImageToBufferInfo->srcImage),
                                Get<BUFFER_STATE>(pCopyImageToBufferInfo->dstBuffer));
}

void ValidationStateTracker::PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                                const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(Func::vkCmdCopyImageToBuffer2, Get<IMAGE_STATE>(pCopyImageToBufferInfo->srcImage),
                                Get<BUFFER_STATE>(pCopyImageToBufferInfo->dstBuffer));
}

void ValidationStateTracker::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                               VkImageLayout dstImageLayout, uint32_t regionCount,
                                                               const VkBufferImageCopy *pRegions) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(Func::vkCmdCopyBufferToImage, Get<BUFFER_STATE>(srcBuffer), Get<IMAGE_STATE>(dstImage));
}

void ValidationStateTracker::PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                                   const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(Func::vkCmdCopyBufferToImage2KHR, Get<BUFFER_STATE>(pCopyBufferToImageInfo->srcBuffer),
                                Get<IMAGE_STATE>(pCopyBufferToImageInfo->dstImage));
}

void ValidationStateTracker::PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                                const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(Func::vkCmdCopyBufferToImage2, Get<BUFFER_STATE>(pCopyBufferToImageInfo->srcBuffer),
                                Get<IMAGE_STATE>(pCopyBufferToImageInfo->dstImage));
}

// Gets union of all features defined by Potential Format Features
// except, does not handle the external format case for AHB as that only can be used for sampled images
VkFormatFeatureFlags2KHR ValidationStateTracker::GetPotentialFormatFeatures(VkFormat format) const {
    VkFormatFeatureFlags2KHR format_features = 0;

    if (format != VK_FORMAT_UNDEFINED) {
        if (has_format_feature2) {
            VkDrmFormatModifierPropertiesList2EXT fmt_drm_props = vku::InitStructHelper();
            auto fmt_props_3 = vku::InitStruct<VkFormatProperties3KHR>(
                IsExtEnabled(device_extensions.vk_ext_image_drm_format_modifier) ? &fmt_drm_props : nullptr);
            VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&fmt_props_3);

            DispatchGetPhysicalDeviceFormatProperties2(physical_device, format, &fmt_props_2);

            format_features |= fmt_props_2.formatProperties.linearTilingFeatures;
            format_features |= fmt_props_2.formatProperties.optimalTilingFeatures;

            format_features |= fmt_props_3.linearTilingFeatures;
            format_features |= fmt_props_3.optimalTilingFeatures;

            if (IsExtEnabled(device_extensions.vk_ext_image_drm_format_modifier)) {
                std::vector<VkDrmFormatModifierProperties2EXT> drm_properties;
                drm_properties.resize(fmt_drm_props.drmFormatModifierCount);
                fmt_drm_props.pDrmFormatModifierProperties = drm_properties.data();
                DispatchGetPhysicalDeviceFormatProperties2(physical_device, format, &fmt_props_2);

                for (uint32_t i = 0; i < fmt_drm_props.drmFormatModifierCount; i++) {
                    format_features |= fmt_drm_props.pDrmFormatModifierProperties[i].drmFormatModifierTilingFeatures;
                }
            }
        } else {
            VkFormatProperties format_properties;
            DispatchGetPhysicalDeviceFormatProperties(physical_device, format, &format_properties);
            format_features |= format_properties.linearTilingFeatures;
            format_features |= format_properties.optimalTilingFeatures;

            if (IsExtEnabled(device_extensions.vk_ext_image_drm_format_modifier)) {
                VkDrmFormatModifierPropertiesListEXT fmt_drm_props = vku::InitStructHelper();
                VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&fmt_drm_props);

                DispatchGetPhysicalDeviceFormatProperties2(physical_device, format, &fmt_props_2);

                std::vector<VkDrmFormatModifierPropertiesEXT> drm_properties;
                drm_properties.resize(fmt_drm_props.drmFormatModifierCount);
                fmt_drm_props.pDrmFormatModifierProperties = drm_properties.data();
                DispatchGetPhysicalDeviceFormatProperties2(physical_device, format, &fmt_props_2);

                for (uint32_t i = 0; i < fmt_drm_props.drmFormatModifierCount; i++) {
                    format_features |= fmt_drm_props.pDrmFormatModifierProperties[i].drmFormatModifierTilingFeatures;
                }
            }
        }
    }

    return format_features;
}

void ValidationStateTracker::PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                                        const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                                        const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    // The current object represents the VkInstance, look up / create the object for the device.
    ValidationObject *device_object = GetLayerDataPtr(get_dispatch_key(*pDevice), layer_data_map);
    ValidationObject *validation_data = device_object->GetValidationObject(this->container_type);
    ValidationStateTracker *device_state = static_cast<ValidationStateTracker *>(validation_data);

    device_state->instance_state = this;
    // Save local link to this device's physical device state
    device_state->physical_device_state = Get<PHYSICAL_DEVICE_STATE>(gpu).get();
    // finish setup in the object representing the device
    device_state->CreateDevice(pCreateInfo);
}

std::shared_ptr<QUEUE_STATE> ValidationStateTracker::CreateQueue(VkQueue q, uint32_t index, VkDeviceQueueCreateFlags flags,
                                                                 const VkQueueFamilyProperties &queueFamilyProperties) {
    return std::make_shared<QUEUE_STATE>(*this, q, index, flags, queueFamilyProperties);
}

void ValidationStateTracker::CreateDevice(const VkDeviceCreateInfo *pCreateInfo) {
    GetEnabledDeviceFeatures(pCreateInfo, &enabled_features, api_version);

    const auto *device_group_ci = vku::FindStructInPNextChain<VkDeviceGroupDeviceCreateInfo>(pCreateInfo->pNext);
    if (device_group_ci) {
        physical_device_count = device_group_ci->physicalDeviceCount;
        if (physical_device_count == 0) {
            physical_device_count =
                1;  // see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDeviceGroupDeviceCreateInfo.html
        }
        device_group_create_info = *device_group_ci;
    } else {
        device_group_create_info = vku::InitStructHelper();
        device_group_create_info.physicalDeviceCount = 1;  // see previous VkDeviceGroupDeviceCreateInfo link
        device_group_create_info.pPhysicalDevices = &physical_device;
        physical_device_count = 1;
    }

    // Store physical device properties and physical device mem limits into CoreChecks structs
    DispatchGetPhysicalDeviceMemoryProperties(physical_device, &phys_dev_mem_props);
    DispatchGetPhysicalDeviceProperties(physical_device, &phys_dev_props);

    {
        uint32_t n_props = 0;
        std::vector<VkExtensionProperties> props;
        instance_dispatch_table.EnumerateDeviceExtensionProperties(physical_device, NULL, &n_props, NULL);
        props.resize(n_props);
        instance_dispatch_table.EnumerateDeviceExtensionProperties(physical_device, NULL, &n_props, props.data());

        for (const auto &ext_prop : props) {
            phys_dev_extensions.insert(ext_prop.extensionName);
        }

        // Even if VK_KHR_format_feature_flags2 is available, we need to have
        // a path to grab that information from the physical device. This
        // requires to have VK_KHR_get_physical_device_properties2 enabled or
        // Vulkan 1.1 (which made this core).
        has_format_feature2 =
            (api_version >= VK_API_VERSION_1_1 || IsExtEnabled(instance_extensions.vk_khr_get_physical_device_properties2)) &&
            phys_dev_extensions.find(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME) != phys_dev_extensions.end();

        // feature is required if 1.3 or extension is supported
        has_robust_image_access =
            (api_version >= VK_API_VERSION_1_3 || IsExtEnabled(instance_extensions.vk_khr_get_physical_device_properties2)) &&
            phys_dev_extensions.find(VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME) != phys_dev_extensions.end();
    }

    const auto &dev_ext = device_extensions;
    auto *phys_dev_props = &phys_dev_ext_props;

    // Vulkan 1.2 / 1.3 can get properties from single struct, otherwise need to add to it per extension
    if (dev_ext.vk_feature_version_1_2 || dev_ext.vk_feature_version_1_3) {
        GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_feature_version_1_2, &phys_dev_props_core11);
        GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_feature_version_1_2, &phys_dev_props_core12);
        if (dev_ext.vk_feature_version_1_3)
            GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_feature_version_1_3, &phys_dev_props_core13);
    } else {
        // VkPhysicalDeviceVulkan11Properties
        //
        // Can ingnore VkPhysicalDeviceIDProperties as it has no validation purpose

        if (dev_ext.vk_khr_multiview) {
            VkPhysicalDeviceMultiviewProperties multiview_props = vku::InitStructHelper();
            GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_khr_multiview, &multiview_props);
            phys_dev_props_core11.maxMultiviewViewCount = multiview_props.maxMultiviewViewCount;
            phys_dev_props_core11.maxMultiviewInstanceIndex = multiview_props.maxMultiviewInstanceIndex;
        }

        if (dev_ext.vk_khr_maintenance3) {
            VkPhysicalDeviceMaintenance3Properties maintenance3_props = vku::InitStructHelper();
            GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_khr_maintenance3, &maintenance3_props);
            phys_dev_props_core11.maxPerSetDescriptors = maintenance3_props.maxPerSetDescriptors;
            phys_dev_props_core11.maxMemoryAllocationSize = maintenance3_props.maxMemoryAllocationSize;
        }

        // Some 1.1 properties were added to core without previous extensions
        if (api_version >= VK_API_VERSION_1_1) {
            VkPhysicalDeviceSubgroupProperties subgroup_prop = vku::InitStructHelper();
            VkPhysicalDeviceProtectedMemoryProperties protected_memory_prop = vku::InitStructHelper(&subgroup_prop);
            VkPhysicalDeviceProperties2 prop2 = vku::InitStructHelper(&protected_memory_prop);
            instance_dispatch_table.GetPhysicalDeviceProperties2(physical_device, &prop2);

            phys_dev_props_core11.subgroupSize = subgroup_prop.subgroupSize;
            phys_dev_props_core11.subgroupSupportedStages = subgroup_prop.supportedStages;
            phys_dev_props_core11.subgroupSupportedOperations = subgroup_prop.supportedOperations;
            phys_dev_props_core11.subgroupQuadOperationsInAllStages = subgroup_prop.quadOperationsInAllStages;

            phys_dev_props_core11.protectedNoFault = protected_memory_prop.protectedNoFault;
        }

        // VkPhysicalDeviceVulkan12Properties
        //
        // Can ingnore VkPhysicalDeviceDriverProperties as it has no validation purpose

        if (dev_ext.vk_ext_descriptor_indexing) {
            VkPhysicalDeviceDescriptorIndexingProperties descriptor_indexing_prop = vku::InitStructHelper();
            GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_descriptor_indexing, &descriptor_indexing_prop);
            phys_dev_props_core12.maxUpdateAfterBindDescriptorsInAllPools =
                descriptor_indexing_prop.maxUpdateAfterBindDescriptorsInAllPools;
            phys_dev_props_core12.shaderUniformBufferArrayNonUniformIndexingNative =
                descriptor_indexing_prop.shaderUniformBufferArrayNonUniformIndexingNative;
            phys_dev_props_core12.shaderSampledImageArrayNonUniformIndexingNative =
                descriptor_indexing_prop.shaderSampledImageArrayNonUniformIndexingNative;
            phys_dev_props_core12.shaderStorageBufferArrayNonUniformIndexingNative =
                descriptor_indexing_prop.shaderStorageBufferArrayNonUniformIndexingNative;
            phys_dev_props_core12.shaderStorageImageArrayNonUniformIndexingNative =
                descriptor_indexing_prop.shaderStorageImageArrayNonUniformIndexingNative;
            phys_dev_props_core12.shaderInputAttachmentArrayNonUniformIndexingNative =
                descriptor_indexing_prop.shaderInputAttachmentArrayNonUniformIndexingNative;
            phys_dev_props_core12.robustBufferAccessUpdateAfterBind = descriptor_indexing_prop.robustBufferAccessUpdateAfterBind;
            phys_dev_props_core12.quadDivergentImplicitLod = descriptor_indexing_prop.quadDivergentImplicitLod;
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindSamplers =
                descriptor_indexing_prop.maxPerStageDescriptorUpdateAfterBindSamplers;
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindUniformBuffers =
                descriptor_indexing_prop.maxPerStageDescriptorUpdateAfterBindUniformBuffers;
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindStorageBuffers =
                descriptor_indexing_prop.maxPerStageDescriptorUpdateAfterBindStorageBuffers;
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindSampledImages =
                descriptor_indexing_prop.maxPerStageDescriptorUpdateAfterBindSampledImages;
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindStorageImages =
                descriptor_indexing_prop.maxPerStageDescriptorUpdateAfterBindStorageImages;
            phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindInputAttachments =
                descriptor_indexing_prop.maxPerStageDescriptorUpdateAfterBindInputAttachments;
            phys_dev_props_core12.maxPerStageUpdateAfterBindResources =
                descriptor_indexing_prop.maxPerStageUpdateAfterBindResources;
            phys_dev_props_core12.maxDescriptorSetUpdateAfterBindSamplers =
                descriptor_indexing_prop.maxDescriptorSetUpdateAfterBindSamplers;
            phys_dev_props_core12.maxDescriptorSetUpdateAfterBindUniformBuffers =
                descriptor_indexing_prop.maxDescriptorSetUpdateAfterBindUniformBuffers;
            phys_dev_props_core12.maxDescriptorSetUpdateAfterBindUniformBuffersDynamic =
                descriptor_indexing_prop.maxDescriptorSetUpdateAfterBindUniformBuffersDynamic;
            phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageBuffers =
                descriptor_indexing_prop.maxDescriptorSetUpdateAfterBindStorageBuffers;
            phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageBuffersDynamic =
                descriptor_indexing_prop.maxDescriptorSetUpdateAfterBindStorageBuffersDynamic;
            phys_dev_props_core12.maxDescriptorSetUpdateAfterBindSampledImages =
                descriptor_indexing_prop.maxDescriptorSetUpdateAfterBindSampledImages;
            phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageImages =
                descriptor_indexing_prop.maxDescriptorSetUpdateAfterBindStorageImages;
            phys_dev_props_core12.maxDescriptorSetUpdateAfterBindInputAttachments =
                descriptor_indexing_prop.maxDescriptorSetUpdateAfterBindInputAttachments;
        }

        if (dev_ext.vk_khr_depth_stencil_resolve) {
            VkPhysicalDeviceDepthStencilResolveProperties depth_stencil_resolve_props = vku::InitStructHelper();
            GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_khr_depth_stencil_resolve, &depth_stencil_resolve_props);
            phys_dev_props_core12.supportedDepthResolveModes = depth_stencil_resolve_props.supportedDepthResolveModes;
            phys_dev_props_core12.supportedStencilResolveModes = depth_stencil_resolve_props.supportedStencilResolveModes;
            phys_dev_props_core12.independentResolveNone = depth_stencil_resolve_props.independentResolveNone;
            phys_dev_props_core12.independentResolve = depth_stencil_resolve_props.independentResolve;
        }

        if (dev_ext.vk_khr_timeline_semaphore) {
            VkPhysicalDeviceTimelineSemaphoreProperties timeline_semaphore_props = vku::InitStructHelper();
            GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_khr_timeline_semaphore, &timeline_semaphore_props);
            phys_dev_props_core12.maxTimelineSemaphoreValueDifference =
                timeline_semaphore_props.maxTimelineSemaphoreValueDifference;
        }

        if (dev_ext.vk_ext_sampler_filter_minmax) {
            VkPhysicalDeviceSamplerFilterMinmaxProperties sampler_filter_minmax_props = vku::InitStructHelper();
            GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_sampler_filter_minmax, &sampler_filter_minmax_props);
            phys_dev_props_core12.filterMinmaxSingleComponentFormats =
                sampler_filter_minmax_props.filterMinmaxSingleComponentFormats;
            phys_dev_props_core12.filterMinmaxImageComponentMapping = sampler_filter_minmax_props.filterMinmaxImageComponentMapping;
        }

        if (dev_ext.vk_khr_shader_float_controls) {
            VkPhysicalDeviceFloatControlsProperties float_controls_props = vku::InitStructHelper();
            GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_khr_shader_float_controls, &float_controls_props);
            phys_dev_props_core12.denormBehaviorIndependence = float_controls_props.denormBehaviorIndependence;
            phys_dev_props_core12.roundingModeIndependence = float_controls_props.roundingModeIndependence;
            phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat16 =
                float_controls_props.shaderSignedZeroInfNanPreserveFloat16;
            phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat32 =
                float_controls_props.shaderSignedZeroInfNanPreserveFloat32;
            phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat64 =
                float_controls_props.shaderSignedZeroInfNanPreserveFloat64;
            phys_dev_props_core12.shaderDenormPreserveFloat16 = float_controls_props.shaderDenormPreserveFloat16;
            phys_dev_props_core12.shaderDenormPreserveFloat32 = float_controls_props.shaderDenormPreserveFloat32;
            phys_dev_props_core12.shaderDenormPreserveFloat64 = float_controls_props.shaderDenormPreserveFloat64;
            phys_dev_props_core12.shaderDenormFlushToZeroFloat16 = float_controls_props.shaderDenormFlushToZeroFloat16;
            phys_dev_props_core12.shaderDenormFlushToZeroFloat32 = float_controls_props.shaderDenormFlushToZeroFloat32;
            phys_dev_props_core12.shaderDenormFlushToZeroFloat64 = float_controls_props.shaderDenormFlushToZeroFloat64;
            phys_dev_props_core12.shaderRoundingModeRTEFloat16 = float_controls_props.shaderRoundingModeRTEFloat16;
            phys_dev_props_core12.shaderRoundingModeRTEFloat32 = float_controls_props.shaderRoundingModeRTEFloat32;
            phys_dev_props_core12.shaderRoundingModeRTEFloat64 = float_controls_props.shaderRoundingModeRTEFloat64;
            phys_dev_props_core12.shaderRoundingModeRTZFloat16 = float_controls_props.shaderRoundingModeRTZFloat16;
            phys_dev_props_core12.shaderRoundingModeRTZFloat32 = float_controls_props.shaderRoundingModeRTZFloat32;
            phys_dev_props_core12.shaderRoundingModeRTZFloat64 = float_controls_props.shaderRoundingModeRTZFloat64;
        }
    }

    // Extensions with properties to extract to DeviceExtensionProperties
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_khr_push_descriptor, &phys_dev_props->push_descriptor_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_nv_shading_rate_image, &phys_dev_props->shading_rate_image_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_nv_mesh_shader, &phys_dev_props->mesh_shader_props_nv);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_mesh_shader, &phys_dev_props->mesh_shader_props_ext);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_inline_uniform_block,
                                   &phys_dev_props->inline_uniform_block_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_vertex_attribute_divisor,
                                   &phys_dev_props->vtx_attrib_divisor_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_transform_feedback, &phys_dev_props->transform_feedback_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_nv_ray_tracing, &phys_dev_props->ray_tracing_props_nv);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_khr_ray_tracing_pipeline, &phys_dev_props->ray_tracing_props_khr);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_khr_acceleration_structure, &phys_dev_props->acc_structure_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_texel_buffer_alignment,
                                   &phys_dev_props->texel_buffer_alignment_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_fragment_density_map,
                                   &phys_dev_props->fragment_density_map_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_fragment_density_map2,
                                   &phys_dev_props->fragment_density_map2_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_qcom_fragment_density_map_offset,
                                   &phys_dev_props->fragment_density_map_offset_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_khr_performance_query, &phys_dev_props->performance_query_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_sample_locations, &phys_dev_props->sample_locations_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_custom_border_color, &phys_dev_props->custom_border_color_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_khr_multiview, &phys_dev_props->multiview_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_khr_portability_subset, &phys_dev_props->portability_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_khr_fragment_shading_rate,
                                   &phys_dev_props->fragment_shading_rate_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_provoking_vertex, &phys_dev_props->provoking_vertex_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_multi_draw, &phys_dev_props->multi_draw_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_discard_rectangles, &phys_dev_props->discard_rectangle_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_blend_operation_advanced,
                                   &phys_dev_props->blend_operation_advanced_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_conservative_rasterization,
                                   &phys_dev_props->conservative_rasterization_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_subgroup_size_control,
                                   &phys_dev_props->subgroup_size_control_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_qcom_image_processing, &phys_dev_props->image_processing_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_descriptor_buffer, &phys_dev_props->descriptor_buffer_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_descriptor_buffer, &phys_dev_props->descriptor_buffer_density_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_host_image_copy, &phys_dev_props->host_image_copy_properties);
    if ((phys_dev_props->host_image_copy_properties.copySrcLayoutCount > 0) ||
        (phys_dev_props->host_image_copy_properties.copyDstLayoutCount > 0)) {
        // Have to allocate memory for the layout lists
        host_image_copy_src_layouts.resize(phys_dev_props->host_image_copy_properties.copySrcLayoutCount);
        host_image_copy_dst_layouts.resize(phys_dev_props->host_image_copy_properties.copyDstLayoutCount);
        if (phys_dev_props->host_image_copy_properties.copySrcLayoutCount > 0) {
            phys_dev_props->host_image_copy_properties.pCopySrcLayouts = host_image_copy_src_layouts.data();
        }
        if (phys_dev_props->host_image_copy_properties.copyDstLayoutCount > 0) {
            phys_dev_props->host_image_copy_properties.pCopyDstLayouts = host_image_copy_dst_layouts.data();
        }
        // Call again (without init) to fill in lists
        GetPhysicalDeviceExtProperties<false>(physical_device, dev_ext.vk_ext_host_image_copy,
                                              &phys_dev_props->host_image_copy_properties);
    }
    if (api_version >= VK_API_VERSION_1_1) {
        GetPhysicalDeviceExtProperties(physical_device, &phys_dev_props->subgroup_props);
    }
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_extended_dynamic_state3,
                                   &phys_dev_props->extended_dynamic_state3_props);

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    VkPhysicalDeviceExternalFormatResolvePropertiesANDROID android_format_resolve_props;
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_android_external_format_resolve, &android_format_resolve_props);
    android_external_format_resolve_null_color_attachment_prop =
        android_format_resolve_props.nullColorAttachmentWithExternalFormatResolve;
#endif

    if (IsExtEnabled(dev_ext.vk_nv_cooperative_matrix)) {
        // Get the needed cooperative_matrix properties
        VkPhysicalDeviceCooperativeMatrixPropertiesNV cooperative_matrix_props = vku::InitStructHelper();
        VkPhysicalDeviceProperties2 prop2 = vku::InitStructHelper(&cooperative_matrix_props);
        instance_dispatch_table.GetPhysicalDeviceProperties2KHR(physical_device, &prop2);
        phys_dev_ext_props.cooperative_matrix_props = cooperative_matrix_props;

        uint32_t num_cooperative_matrix_properties = 0;
        instance_dispatch_table.GetPhysicalDeviceCooperativeMatrixPropertiesNV(physical_device, &num_cooperative_matrix_properties,
                                                                               NULL);
        cooperative_matrix_properties.resize(num_cooperative_matrix_properties, vku::InitStruct<VkCooperativeMatrixPropertiesNV>());

        instance_dispatch_table.GetPhysicalDeviceCooperativeMatrixPropertiesNV(physical_device, &num_cooperative_matrix_properties,
                                                                               cooperative_matrix_properties.data());
    }

    if (IsExtEnabled(dev_ext.vk_khr_cooperative_matrix)) {
        // Get the needed KHR cooperative_matrix properties
        VkPhysicalDeviceCooperativeMatrixPropertiesKHR cooperative_matrix_props_khr = vku::InitStructHelper();
        VkPhysicalDeviceProperties2 prop2 = vku::InitStructHelper(&cooperative_matrix_props_khr);
        instance_dispatch_table.GetPhysicalDeviceProperties2KHR(physical_device, &prop2);
        phys_dev_ext_props.cooperative_matrix_props_khr = cooperative_matrix_props_khr;

        uint32_t num_cooperative_matrix_properties_khr = 0;
        instance_dispatch_table.GetPhysicalDeviceCooperativeMatrixPropertiesKHR(physical_device,
                                                                                &num_cooperative_matrix_properties_khr, NULL);
        cooperative_matrix_properties_khr.resize(num_cooperative_matrix_properties_khr,
                                                 vku::InitStruct<VkCooperativeMatrixPropertiesKHR>());

        instance_dispatch_table.GetPhysicalDeviceCooperativeMatrixPropertiesKHR(
            physical_device, &num_cooperative_matrix_properties_khr, cooperative_matrix_properties_khr.data());
    }

    // Store queue family data
    if (pCreateInfo->pQueueCreateInfos != nullptr) {
        uint32_t num_queue_families = 0;
        instance_dispatch_table.GetPhysicalDeviceQueueFamilyProperties(physical_device, &num_queue_families, nullptr);
        std::vector<VkQueueFamilyProperties> queue_family_properties_list(num_queue_families);
        instance_dispatch_table.GetPhysicalDeviceQueueFamilyProperties(physical_device, &num_queue_families,
                                                                       queue_family_properties_list.data());

        for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; ++i) {
            const VkDeviceQueueCreateInfo &queue_create_info = pCreateInfo->pQueueCreateInfos[i];
            queue_family_index_set.insert(queue_create_info.queueFamilyIndex);
            device_queue_info_list.push_back(
                {i, queue_create_info.queueFamilyIndex, queue_create_info.flags, queue_create_info.queueCount});
        }
        for (const auto &queue_info : device_queue_info_list) {
            for (uint32_t i = 0; i < queue_info.queue_count; i++) {
                VkQueue queue = VK_NULL_HANDLE;
                // vkGetDeviceQueue2() was added in vulkan 1.1, and there was never a KHR version of it.
                if (api_version >= VK_API_VERSION_1_1 && queue_info.flags != 0) {
                    VkDeviceQueueInfo2 get_info = vku::InitStructHelper();
                    get_info.flags = queue_info.flags;
                    get_info.queueFamilyIndex = queue_info.queue_family_index;
                    get_info.queueIndex = i;
                    DispatchGetDeviceQueue2(device, &get_info, &queue);
                } else {
                    DispatchGetDeviceQueue(device, queue_info.queue_family_index, i, &queue);
                }
                assert(queue != VK_NULL_HANDLE);
                Add(CreateQueue(queue, queue_info.queue_family_index, queue_info.flags,
                                queue_family_properties_list[queue_info.queue_family_index]));
            }
        }
    }

    // Query queue family extension properties
    {
        uint32_t queue_family_count = (uint32_t)physical_device_state->queue_family_properties.size();
        auto &ext_props = queue_family_ext_props;
        ext_props.resize(queue_family_count);

        std::vector<VkQueueFamilyProperties2> props(queue_family_count, vku::InitStruct<VkQueueFamilyProperties2>());

        if (dev_ext.vk_khr_video_queue) {
            for (uint32_t i = 0; i < queue_family_count; ++i) {
                ext_props[i].query_result_status_props = vku::InitStructHelper();
                ext_props[i].video_props = vku::InitStructHelper(&ext_props[i].query_result_status_props);
                props[i].pNext = &ext_props[i].video_props;
            }
        }

        if (api_version >= VK_API_VERSION_1_1) {
            DispatchGetPhysicalDeviceQueueFamilyProperties2(physical_device, &queue_family_count, props.data());
        } else if (IsExtEnabled(instance_extensions.vk_khr_get_physical_device_properties2)) {
            DispatchGetPhysicalDeviceQueueFamilyProperties2KHR(physical_device, &queue_family_count, props.data());
        }
    }
}

void ValidationStateTracker::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    if (!device) return;

    command_pool_map_.clear();
    assert(command_buffer_map_.empty());
    pipeline_map_.clear();
    shader_object_map_.clear();
    render_pass_map_.clear();

    // This will also delete all sets in the pool & remove them from setMap
    descriptor_pool_map_.clear();
    // All sets should be removed
    assert(descriptor_set_map_.empty());
    desc_template_map_.clear();
    descriptor_set_layout_map_.clear();
    // Because swapchains are associated with Surfaces, which are at instance level,
    // they need to be explicitly destroyed here to avoid continued references to
    // the device we're destroying.
    for (auto &entry : swapchain_map_.snapshot()) {
        entry.second->Destroy();
    }
    swapchain_map_.clear();
    image_view_map_.clear();
    image_map_.clear();
    buffer_view_map_.clear();
    buffer_map_.clear();
    // Queues persist until device is destroyed
    for (auto &entry : queue_map_.snapshot()) {
        entry.second->Destroy();
    }
    queue_map_.clear();
}

void ValidationStateTracker::PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits,
                                                      VkFence fence) {
    auto queue_state = Get<QUEUE_STATE>(queue);

    uint64_t early_retire_seq = 0;

    if (submitCount == 0) {
        CB_SUBMISSION submission;
        submission.AddFence(Get<FENCE_STATE>(fence));
        early_retire_seq = queue_state->Submit(std::move(submission));
    }

    // Now process each individual submit
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        CB_SUBMISSION submission;
        const VkSubmitInfo *submit = &pSubmits[submit_idx];
        auto *timeline_semaphore_submit = vku::FindStructInPNextChain<VkTimelineSemaphoreSubmitInfo>(submit->pNext);
        for (uint32_t i = 0; i < submit->waitSemaphoreCount; ++i) {
            uint64_t value{0};
            if (timeline_semaphore_submit && timeline_semaphore_submit->pWaitSemaphoreValues != nullptr &&
                (i < timeline_semaphore_submit->waitSemaphoreValueCount)) {
                value = timeline_semaphore_submit->pWaitSemaphoreValues[i];
            }
            submission.AddWaitSemaphore(Get<SEMAPHORE_STATE>(submit->pWaitSemaphores[i]), value);
        }

        for (uint32_t i = 0; i < submit->signalSemaphoreCount; ++i) {
            uint64_t value{0};
            if (timeline_semaphore_submit && timeline_semaphore_submit->pSignalSemaphoreValues != nullptr &&
                (i < timeline_semaphore_submit->signalSemaphoreValueCount)) {
                value = timeline_semaphore_submit->pSignalSemaphoreValues[i];
            }
            submission.AddSignalSemaphore(Get<SEMAPHORE_STATE>(submit->pSignalSemaphores[i]), value);
        }

        const auto perf_submit = vku::FindStructInPNextChain<VkPerformanceQuerySubmitInfoKHR>(submit->pNext);
        submission.perf_submit_pass = perf_submit ? perf_submit->counterPassIndex : 0;

        for (uint32_t i = 0; i < submit->commandBufferCount; i++) {
            auto cb_state = Get<CMD_BUFFER_STATE>(submit->pCommandBuffers[i]);
            if (cb_state) {
                submission.AddCommandBuffer(std::move(cb_state));
            }
        }
        if (submit_idx == (submitCount - 1) && fence != VK_NULL_HANDLE) {
            submission.AddFence(Get<FENCE_STATE>(fence));
        }
        auto submit_seq = queue_state->Submit(std::move(submission));
        early_retire_seq = std::max(early_retire_seq, submit_seq);
    }

    if (early_retire_seq) {
        queue_state->NotifyAndWait(early_retire_seq);
    }
}

void ValidationStateTracker::RecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits,
                                                VkFence fence) {
    auto queue_state = Get<QUEUE_STATE>(queue);
    uint64_t early_retire_seq = 0;
    if (submitCount == 0) {
        CB_SUBMISSION submission;
        submission.AddFence(Get<FENCE_STATE>(fence));
        early_retire_seq = queue_state->Submit(std::move(submission));
    }

    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        CB_SUBMISSION submission;
        const VkSubmitInfo2KHR *submit = &pSubmits[submit_idx];
        for (uint32_t i = 0; i < submit->waitSemaphoreInfoCount; ++i) {
            const auto &sem_info = submit->pWaitSemaphoreInfos[i];
            submission.AddWaitSemaphore(Get<SEMAPHORE_STATE>(sem_info.semaphore), sem_info.value);
        }
        for (uint32_t i = 0; i < submit->signalSemaphoreInfoCount; ++i) {
            const auto &sem_info = submit->pSignalSemaphoreInfos[i];
            submission.AddSignalSemaphore(Get<SEMAPHORE_STATE>(sem_info.semaphore), sem_info.value);
        }
        const auto perf_submit = vku::FindStructInPNextChain<VkPerformanceQuerySubmitInfoKHR>(submit->pNext);
        submission.perf_submit_pass = perf_submit ? perf_submit->counterPassIndex : 0;

        for (uint32_t i = 0; i < submit->commandBufferInfoCount; i++) {
            submission.AddCommandBuffer(GetWrite<CMD_BUFFER_STATE>(submit->pCommandBufferInfos[i].commandBuffer));
        }
        if (submit_idx == (submitCount - 1)) {
            submission.AddFence(Get<FENCE_STATE>(fence));
        }
        auto submit_seq = queue_state->Submit(std::move(submission));
        early_retire_seq = std::max(early_retire_seq, submit_seq);
    }
    if (early_retire_seq) {
        queue_state->NotifyAndWait(early_retire_seq);
    }
}

void ValidationStateTracker::PreCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits,
                                                          VkFence fence) {
    RecordQueueSubmit2(queue, submitCount, pSubmits, fence);
}

void ValidationStateTracker::PreCallRecordQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2 *pSubmits,
                                                       VkFence fence) {
    RecordQueueSubmit2(queue, submitCount, pSubmits, fence);
}

void ValidationStateTracker::PostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                                          const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory,
                                                          const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) {
        return;
    }
    const auto &memory_type = phys_dev_mem_props.memoryTypes[pAllocateInfo->memoryTypeIndex];
    const auto &memory_heap = phys_dev_mem_props.memoryHeaps[memory_type.heapIndex];
    auto fake_address = fake_memory.Alloc(pAllocateInfo->allocationSize);

    std::optional<DedicatedBinding> dedicated_binding;
    if (const auto dedicated = vku::FindStructInPNextChain<VkMemoryDedicatedAllocateInfo>(pAllocateInfo->pNext)) {
        if (dedicated->buffer) {
            auto buffer_state = Get<BUFFER_STATE>(dedicated->buffer);
            assert(buffer_state);
            if (!buffer_state) {
                return;
            }
            dedicated_binding.emplace(dedicated->buffer, buffer_state->createInfo);
        } else if (dedicated->image) {
            auto image_state = Get<IMAGE_STATE>(dedicated->image);
            assert(image_state);
            if (!image_state) {
                return;
            }
            dedicated_binding.emplace(dedicated->image, image_state->createInfo);
        }
    }
    if (const auto import_memory_fd_info = vku::FindStructInPNextChain<VkImportMemoryFdInfoKHR>(pAllocateInfo->pNext)) {
        // Successful import operation transfers POSIX handle ownership to the driver.
        // Stop tracking handle at this point. It can not be used for import operations anymore.
        // The map's erase is a no-op for externally created handles that are not tracked here.
        // NOTE: In contrast, the successful import does not transfer ownership of a Win32 handle.
        WriteLockGuard guard(fd_handle_map_lock_);
        fd_handle_map_.erase(import_memory_fd_info->fd);
    }
    Add(CreateDeviceMemoryState(*pMemory, pAllocateInfo, fake_address, memory_type, memory_heap, std::move(dedicated_binding),
                                physical_device_count));
    return;
}

void ValidationStateTracker::PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory mem, const VkAllocationCallbacks *pAllocator) {
    auto mem_info = Get<DEVICE_MEMORY_STATE>(mem);
    if (mem_info) {
        fake_memory.Free(mem_info->fake_base_address);
    }
    Destroy<DEVICE_MEMORY_STATE>(mem);
}

void ValidationStateTracker::PreCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo *pBindInfo,
                                                          VkFence fence) {
    auto queue_state = Get<QUEUE_STATE>(queue);

    uint64_t early_retire_seq = 0;

    for (uint32_t bind_idx = 0; bind_idx < bindInfoCount; ++bind_idx) {
        const VkBindSparseInfo &bind_info = pBindInfo[bind_idx];
        // Track objects tied to memory
        for (uint32_t j = 0; j < bind_info.bufferBindCount; j++) {
            for (uint32_t k = 0; k < bind_info.pBufferBinds[j].bindCount; k++) {
                auto sparse_binding = bind_info.pBufferBinds[j].pBinds[k];
                auto buffer_state = Get<BUFFER_STATE>(bind_info.pBufferBinds[j].buffer);
                auto mem_state = Get<DEVICE_MEMORY_STATE>(sparse_binding.memory);
                if (buffer_state) {
                    buffer_state->BindMemory(buffer_state.get(), mem_state, sparse_binding.memoryOffset,
                                             sparse_binding.resourceOffset, sparse_binding.size);
                }
            }
        }
        for (uint32_t j = 0; j < bind_info.imageOpaqueBindCount; j++) {
            for (uint32_t k = 0; k < bind_info.pImageOpaqueBinds[j].bindCount; k++) {
                auto sparse_binding = bind_info.pImageOpaqueBinds[j].pBinds[k];
                auto image_state = Get<IMAGE_STATE>(bind_info.pImageOpaqueBinds[j].image);
                auto mem_state = Get<DEVICE_MEMORY_STATE>(sparse_binding.memory);
                if (image_state) {
                    // An Android special image cannot get VkSubresourceLayout until the image binds a memory.
                    // See: VUID-vkGetImageSubresourceLayout-image-01895
                    if (!image_state->fragment_encoder) {
                        image_state->fragment_encoder =
                            std::make_unique<const subresource_adapter::ImageRangeEncoder>(*image_state);
                    }
                    image_state->BindMemory(image_state.get(), mem_state, sparse_binding.memoryOffset,
                                            sparse_binding.resourceOffset, sparse_binding.size);
                }
            }
        }
        for (uint32_t j = 0; j < bind_info.imageBindCount; j++) {
            for (uint32_t k = 0; k < bind_info.pImageBinds[j].bindCount; k++) {
                auto sparse_binding = bind_info.pImageBinds[j].pBinds[k];
                // TODO: This size is broken for non-opaque bindings, need to update to comprehend full sparse binding data
                VkDeviceSize size = sparse_binding.extent.depth * sparse_binding.extent.height * sparse_binding.extent.width * 4;
                VkDeviceSize offset = sparse_binding.offset.z * sparse_binding.offset.y * sparse_binding.offset.x * 4;
                auto image_state = Get<IMAGE_STATE>(bind_info.pImageBinds[j].image);
                auto mem_state = Get<DEVICE_MEMORY_STATE>(sparse_binding.memory);
                if (image_state) {
                    // An Android special image cannot get VkSubresourceLayout until the image binds a memory.
                    // See: VUID-vkGetImageSubresourceLayout-image-01895
                    if (!image_state->fragment_encoder) {
                        image_state->fragment_encoder =
                            std::make_unique<const subresource_adapter::ImageRangeEncoder>(*image_state);
                    }
                    image_state->BindMemory(image_state.get(), mem_state, sparse_binding.memoryOffset, offset, size);
                }
            }
        }
        auto timeline_info = vku::FindStructInPNextChain<VkTimelineSemaphoreSubmitInfo>(bind_info.pNext);
        CB_SUBMISSION submission;
        for (uint32_t i = 0; i < bind_info.waitSemaphoreCount; ++i) {
            uint64_t payload = 0;
            if (timeline_info && i < timeline_info->waitSemaphoreValueCount) {
                payload = timeline_info->pWaitSemaphoreValues[i];
            }
            submission.AddWaitSemaphore(Get<SEMAPHORE_STATE>(bind_info.pWaitSemaphores[i]), payload);
        }
        for (uint32_t i = 0; i < bind_info.signalSemaphoreCount; ++i) {
            uint64_t payload = 0;
            if (timeline_info && i < timeline_info->signalSemaphoreValueCount) {
                payload = timeline_info->pSignalSemaphoreValues[i];
            }
            submission.AddSignalSemaphore(Get<SEMAPHORE_STATE>(bind_info.pSignalSemaphores[i]), payload);
        }
        if (bind_idx == (bindInfoCount - 1)) {
            submission.AddFence(Get<FENCE_STATE>(fence));
        }
        auto submit_seq = queue_state->Submit(std::move(submission));
        early_retire_seq = std::max(early_retire_seq, submit_seq);
    }

    if (early_retire_seq) {
        queue_state->NotifyAndWait(early_retire_seq);
    }
}

void ValidationStateTracker::PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                                           const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore,
                                                           const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(std::make_shared<SEMAPHORE_STATE>(*this, *pSemaphore, pCreateInfo));
}

void ValidationStateTracker::RecordImportSemaphoreState(VkSemaphore semaphore, VkExternalSemaphoreHandleTypeFlagBits handle_type,
                                                        VkSemaphoreImportFlags flags) {
    auto semaphore_state = Get<SEMAPHORE_STATE>(semaphore);
    if (semaphore_state) {
        semaphore_state->Import(handle_type, flags);
    }
}

void ValidationStateTracker::PreCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo) {
    auto semaphore_state = Get<SEMAPHORE_STATE>(pSignalInfo->semaphore);
    if (semaphore_state) {
        auto value = pSignalInfo->value;  // const workaround
        semaphore_state->EnqueueSignal(nullptr, 0, value);
    }
}

void ValidationStateTracker::PreCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo) {
    PreCallRecordSignalSemaphore(device, pSignalInfo);
}

void ValidationStateTracker::PostCallRecordSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo,
                                                           const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) return;

    auto semaphore_state = Get<SEMAPHORE_STATE>(pSignalInfo->semaphore);
    if (semaphore_state) {
        semaphore_state->Retire(nullptr, pSignalInfo->value);
    }
}

void ValidationStateTracker::PostCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo,
                                                              const RecordObject &record_obj) {
    PostCallRecordSignalSemaphore(device, pSignalInfo, record_obj);
}

void ValidationStateTracker::RecordMappedMemory(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, void **ppData) {
    auto mem_info = Get<DEVICE_MEMORY_STATE>(mem);
    if (mem_info) {
        mem_info->mapped_range.offset = offset;
        mem_info->mapped_range.size = size;
        mem_info->p_driver_data = *ppData;
    }
}

void ValidationStateTracker::PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences,
                                                         VkBool32 waitAll, uint64_t timeout, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    // When we know that all fences are complete we can clean/remove their CBs
    if ((VK_TRUE == waitAll) || (1 == fenceCount)) {
        for (uint32_t i = 0; i < fenceCount; i++) {
            auto fence_state = Get<FENCE_STATE>(pFences[i]);
            if (fence_state) {
                fence_state->NotifyAndWait();
            }
        }
    }
    // NOTE : Alternate case not handled here is when some fences have completed. In
    //  this case for app to guarantee which fences completed it will have to call
    //  vkGetFenceStatus() at which point we'll clean/remove their CBs if complete.
}

void ValidationStateTracker::PreRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout) {
    for (uint32_t i = 0; i < pWaitInfo->semaphoreCount; i++) {
        auto semaphore_state = Get<SEMAPHORE_STATE>(pWaitInfo->pSemaphores[i]);
        if (semaphore_state) {
            auto value = pWaitInfo->pValues[i];  // const workaround
            semaphore_state->EnqueueWait(nullptr, 0, value);
        }
    }
}

void ValidationStateTracker::PreCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout) {
    PreRecordWaitSemaphores(device, pWaitInfo, timeout);
}

void ValidationStateTracker::PreCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo,
                                                            uint64_t timeout) {
    PreRecordWaitSemaphores(device, pWaitInfo, timeout);
}

void ValidationStateTracker::PostRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout,
                                                      const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    // Same logic as vkWaitForFences(). If some semaphores are not signaled, we will get their status when
    // the application calls vkGetSemaphoreCounterValue() on each of them.
    if ((pWaitInfo->flags & VK_SEMAPHORE_WAIT_ANY_BIT) == 0 || pWaitInfo->semaphoreCount == 1) {
        for (uint32_t i = 0; i < pWaitInfo->semaphoreCount; i++) {
            auto semaphore_state = Get<SEMAPHORE_STATE>(pWaitInfo->pSemaphores[i]);
            if (semaphore_state) {
                semaphore_state->NotifyAndWait(pWaitInfo->pValues[i]);
            }
        }
    }
}

void ValidationStateTracker::PostCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout,
                                                          const RecordObject &record_obj) {
    PostRecordWaitSemaphores(device, pWaitInfo, timeout, record_obj);
}

void ValidationStateTracker::PostCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo,
                                                             uint64_t timeout, const RecordObject &record_obj) {
    PostRecordWaitSemaphores(device, pWaitInfo, timeout, record_obj);
}

void ValidationStateTracker::RecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t *pValue,
                                                            const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    auto semaphore_state = Get<SEMAPHORE_STATE>(semaphore);
    if (semaphore_state) {
        semaphore_state->NotifyAndWait(*pValue);
    }
}

void ValidationStateTracker::PostCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t *pValue,
                                                                    const RecordObject &record_obj) {
    RecordGetSemaphoreCounterValue(device, semaphore, pValue, record_obj);
}

void ValidationStateTracker::PostCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t *pValue,
                                                                       const RecordObject &record_obj) {
    RecordGetSemaphoreCounterValue(device, semaphore, pValue, record_obj);
}

void ValidationStateTracker::PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    auto fence_state = Get<FENCE_STATE>(fence);
    if (fence_state) {
        fence_state->NotifyAndWait();
    }
}

void ValidationStateTracker::RecordGetDeviceQueueState(uint32_t queue_family_index, VkDeviceQueueCreateFlags flags, VkQueue queue) {
    if (Get<QUEUE_STATE>(queue) == nullptr) {
        uint32_t num_queue_families = 0;
        instance_dispatch_table.GetPhysicalDeviceQueueFamilyProperties(physical_device, &num_queue_families, nullptr);
        std::vector<VkQueueFamilyProperties> queue_family_properties_list(num_queue_families);
        instance_dispatch_table.GetPhysicalDeviceQueueFamilyProperties(physical_device, &num_queue_families,
                                                                       queue_family_properties_list.data());

        Add(CreateQueue(queue, queue_family_index, flags, queue_family_properties_list[queue_family_index]));
    }
}

void ValidationStateTracker::PostCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex,
                                                          VkQueue *pQueue, const RecordObject &record_obj) {
    RecordGetDeviceQueueState(queueFamilyIndex, {}, *pQueue);
}

void ValidationStateTracker::PostCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue,
                                                           const RecordObject &record_obj) {
    RecordGetDeviceQueueState(pQueueInfo->queueFamilyIndex, pQueueInfo->flags, *pQueue);
}

void ValidationStateTracker::PostCallRecordQueueWaitIdle(VkQueue queue, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    auto queue_state = Get<QUEUE_STATE>(queue);
    if (queue_state) {
        queue_state->NotifyAndWait();
    }
}

void ValidationStateTracker::PostCallRecordDeviceWaitIdle(VkDevice device, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    for (auto &queue : queue_map_.snapshot()) {
        queue.second->NotifyAndWait();
    }
}

void ValidationStateTracker::PreCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks *pAllocator) {
    Destroy<FENCE_STATE>(fence);
}

void ValidationStateTracker::PreCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore,
                                                           const VkAllocationCallbacks *pAllocator) {
    Destroy<SEMAPHORE_STATE>(semaphore);
}

void ValidationStateTracker::PreCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator) {
    Destroy<EVENT_STATE>(event);
}

void ValidationStateTracker::PreCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool,
                                                           const VkAllocationCallbacks *pAllocator) {
    Destroy<QUERY_POOL_STATE>(queryPool);
}

void ValidationStateTracker::UpdateBindBufferMemoryState(VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset) {
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    if (buffer_state) {
        // Track objects tied to memory
        auto mem_state = Get<DEVICE_MEMORY_STATE>(mem);
        if (mem_state) {
            buffer_state->BindMemory(buffer_state.get(), mem_state, memoryOffset, 0u, buffer_state->requirements.size);
        }
    }
}

void ValidationStateTracker::PostCallRecordBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem,
                                                            VkDeviceSize memoryOffset, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    UpdateBindBufferMemoryState(buffer, mem, memoryOffset);
}

void ValidationStateTracker::PostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                             const VkBindBufferMemoryInfo *pBindInfos,
                                                             const RecordObject &record_obj) {
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        UpdateBindBufferMemoryState(pBindInfos[i].buffer, pBindInfos[i].memory, pBindInfos[i].memoryOffset);
    }
}

void ValidationStateTracker::PostCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                                const VkBindBufferMemoryInfo *pBindInfos,
                                                                const RecordObject &record_obj) {
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        UpdateBindBufferMemoryState(pBindInfos[i].buffer, pBindInfos[i].memory, pBindInfos[i].memoryOffset);
    }
}

void ValidationStateTracker::RecordGetImageMemoryRequirementsState(VkImage image, const VkImageMemoryRequirementsInfo2 *pInfo) {
    const VkImagePlaneMemoryRequirementsInfo *plane_info =
        (pInfo == nullptr) ? nullptr : vku::FindStructInPNextChain<VkImagePlaneMemoryRequirementsInfo>(pInfo->pNext);
    auto image_state = Get<IMAGE_STATE>(image);
    if (image_state) {
        if (plane_info != nullptr) {
            // Multi-plane image
            if (plane_info->planeAspect == VK_IMAGE_ASPECT_PLANE_0_BIT) {
                image_state->memory_requirements_checked[0] = true;
            } else if (plane_info->planeAspect == VK_IMAGE_ASPECT_PLANE_1_BIT) {
                image_state->memory_requirements_checked[1] = true;
            } else if (plane_info->planeAspect == VK_IMAGE_ASPECT_PLANE_2_BIT) {
                image_state->memory_requirements_checked[2] = true;
            }
        } else if (!image_state->disjoint) {
            // Single Plane image
            image_state->memory_requirements_checked[0] = true;
        }
    }
}

void ValidationStateTracker::PostCallRecordGetImageMemoryRequirements(VkDevice device, VkImage image,
                                                                      VkMemoryRequirements *pMemoryRequirements,
                                                                      const RecordObject &record_obj) {
    RecordGetImageMemoryRequirementsState(image, nullptr);
}

void ValidationStateTracker::PostCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                                                       VkMemoryRequirements2 *pMemoryRequirements,
                                                                       const RecordObject &record_obj) {
    RecordGetImageMemoryRequirementsState(pInfo->image, pInfo);
}

void ValidationStateTracker::PostCallRecordGetImageMemoryRequirements2KHR(VkDevice device,
                                                                          const VkImageMemoryRequirementsInfo2 *pInfo,
                                                                          VkMemoryRequirements2 *pMemoryRequirements,
                                                                          const RecordObject &record_obj) {
    RecordGetImageMemoryRequirementsState(pInfo->image, pInfo);
}

void ValidationStateTracker::PostCallRecordGetImageSparseMemoryRequirements(
    VkDevice device, VkImage image, uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements *pSparseMemoryRequirements, const RecordObject &record_obj) {
    auto image_state = Get<IMAGE_STATE>(image);
    image_state->get_sparse_reqs_called = true;
}

void ValidationStateTracker::PostCallRecordGetImageSparseMemoryRequirements2(
    VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo, uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements, const RecordObject &record_obj) {
    auto image_state = Get<IMAGE_STATE>(pInfo->image);
    image_state->get_sparse_reqs_called = true;
}

void ValidationStateTracker::PostCallRecordGetImageSparseMemoryRequirements2KHR(
    VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo, uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements, const RecordObject &record_obj) {
    auto image_state = Get<IMAGE_STATE>(pInfo->image);
    image_state->get_sparse_reqs_called = true;
}

void ValidationStateTracker::PreCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                                              const VkAllocationCallbacks *pAllocator) {
    Destroy<SHADER_MODULE_STATE>(shaderModule);
}

void ValidationStateTracker::PreCallRecordDestroyShaderEXT(VkDevice device, VkShaderEXT shader,
                                                           const VkAllocationCallbacks *pAllocator) {
    Destroy<SHADER_OBJECT_STATE>(shader);
}

void ValidationStateTracker::PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline,
                                                          const VkAllocationCallbacks *pAllocator) {
    Destroy<PIPELINE_STATE>(pipeline);
}

void ValidationStateTracker::PostCallRecordCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount,
                                                             const VkShaderStageFlagBits *pStages, const VkShaderEXT *pShaders,
                                                             const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    for (uint32_t i = 0; i < stageCount; ++i) {
        SHADER_OBJECT_STATE *shader_object_state = nullptr;
        if (pShaders && pShaders[i] != VK_NULL_HANDLE) {
            shader_object_state = Get<SHADER_OBJECT_STATE>(pShaders[i]).get();
        }
        cb_state->BindShader(pStages[i], shader_object_state);
    }
}

void ValidationStateTracker::PreCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                                                const VkAllocationCallbacks *pAllocator) {
    Destroy<PIPELINE_LAYOUT_STATE>(pipelineLayout);
}

void ValidationStateTracker::PreCallRecordDestroySampler(VkDevice device, VkSampler sampler,
                                                         const VkAllocationCallbacks *pAllocator) {
    if (!sampler) return;
    auto sampler_state = Get<SAMPLER_STATE>(sampler);
    // Any bound cmd buffers are now invalid
    if (sampler_state) {
        if (sampler_state->createInfo.borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT ||
            sampler_state->createInfo.borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT) {
            custom_border_color_sampler_count--;
        }
    }
    Destroy<SAMPLER_STATE>(sampler);
}

void ValidationStateTracker::PreCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                                     const VkAllocationCallbacks *pAllocator) {
    Destroy<cvdescriptorset::DescriptorSetLayout>(descriptorSetLayout);
}

void ValidationStateTracker::PreCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                                const VkAllocationCallbacks *pAllocator) {
    Destroy<DESCRIPTOR_POOL_STATE>(descriptorPool);
}

void ValidationStateTracker::PreCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool,
                                                             uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers) {
    auto pool = Get<COMMAND_POOL_STATE>(commandPool);
    if (pool) {
        pool->Free(commandBufferCount, pCommandBuffers);
    }
}

std::shared_ptr<COMMAND_POOL_STATE> ValidationStateTracker::CreateCommandPoolState(VkCommandPool command_pool,
                                                                                   const VkCommandPoolCreateInfo *pCreateInfo) {
    auto queue_flags = physical_device_state->queue_family_properties[pCreateInfo->queueFamilyIndex].queueFlags;
    return std::make_shared<COMMAND_POOL_STATE>(this, command_pool, pCreateInfo, queue_flags);
}

void ValidationStateTracker::PostCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkCommandPool *pCommandPool,
                                                             const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(CreateCommandPoolState(*pCommandPool, pCreateInfo));
}

void ValidationStateTracker::PostCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                                           const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool,
                                                           const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    uint32_t index_count = 0, n_perf_pass = 0;
    bool has_cb = false, has_rb = false;
    if (pCreateInfo->queryType == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) {
        const auto *perf = vku::FindStructInPNextChain<VkQueryPoolPerformanceCreateInfoKHR>(pCreateInfo->pNext);
        index_count = perf->counterIndexCount;

        const QUEUE_FAMILY_PERF_COUNTERS &counters = *physical_device_state->perf_counters[perf->queueFamilyIndex];
        for (uint32_t i = 0; i < perf->counterIndexCount; i++) {
            const auto &counter = counters.counters[perf->pCounterIndices[i]];
            switch (counter.scope) {
                case VK_QUERY_SCOPE_COMMAND_BUFFER_KHR:
                    has_cb = true;
                    break;
                case VK_QUERY_SCOPE_RENDER_PASS_KHR:
                    has_rb = true;
                    break;
                default:
                    break;
            }
        }

        DispatchGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physical_device_state->PhysDev(), perf, &n_perf_pass);
    }

    Add(std::make_shared<QUERY_POOL_STATE>(
        *pQueryPool, pCreateInfo, index_count, n_perf_pass, has_cb, has_rb,
        video_profile_cache_.Get(this, vku::FindStructInPNextChain<VkVideoProfileInfoKHR>(pCreateInfo->pNext))));
}

void ValidationStateTracker::PreCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                                             const VkAllocationCallbacks *pAllocator) {
    Destroy<COMMAND_POOL_STATE>(commandPool);
}

void ValidationStateTracker::PostCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool,
                                                            VkCommandPoolResetFlags flags, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    // Reset all of the CBs allocated from this pool
    auto pool = Get<COMMAND_POOL_STATE>(commandPool);
    if (pool) {
        pool->Reset();
    }
}

void ValidationStateTracker::PostCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences,
                                                       const RecordObject &record_obj) {
    for (uint32_t i = 0; i < fenceCount; ++i) {
        auto fence_state = Get<FENCE_STATE>(pFences[i]);
        if (fence_state) {
            fence_state->Reset();
        }
    }
}

void ValidationStateTracker::PreCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                                                             const VkAllocationCallbacks *pAllocator) {
    Destroy<FRAMEBUFFER_STATE>(framebuffer);
}

void ValidationStateTracker::PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                                                            const VkAllocationCallbacks *pAllocator) {
    Destroy<RENDER_PASS_STATE>(renderPass);
}

void ValidationStateTracker::PostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkFence *pFence,
                                                       const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(std::make_shared<FENCE_STATE>(*this, *pFence, pCreateInfo));
}

std::shared_ptr<PIPELINE_CACHE_STATE> ValidationStateTracker::CreatePipelineCacheState(
    VkPipelineCache pipeline_cache, const VkPipelineCacheCreateInfo *pCreateInfo) const {
    return std::make_shared<PIPELINE_CACHE_STATE>(pipeline_cache, pCreateInfo);
}

void ValidationStateTracker::PostCallRecordCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkPipelineCache *pPipelineCache, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(CreatePipelineCacheState(*pPipelineCache, pCreateInfo));
}

void ValidationStateTracker::PreCallRecordDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache,
                                                               const VkAllocationCallbacks *pAllocator) {
    Destroy<PIPELINE_CACHE_STATE>(pipelineCache);
}

std::shared_ptr<PIPELINE_STATE> ValidationStateTracker::CreateGraphicsPipelineState(
    const VkGraphicsPipelineCreateInfo *pCreateInfo, std::shared_ptr<const RENDER_PASS_STATE> &&render_pass,
    std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout, CreateShaderModuleStates *csm_states) const {
    return std::make_shared<PIPELINE_STATE>(this, pCreateInfo, std::move(render_pass), std::move(layout), csm_states);
}

bool ValidationStateTracker::PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                    const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                                    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                    const ErrorObject &error_obj, void *cgpl_state_data) const {
    bool skip = false;
    // Set up the state that CoreChecks, gpu_validation and later StateTracker Record will use.
    create_graphics_pipeline_api_state *cgpl_state = reinterpret_cast<create_graphics_pipeline_api_state *>(cgpl_state_data);
    cgpl_state->pCreateInfos = pCreateInfos;  // GPU validation can alter this, so we have to set a default value for the Chassis
    cgpl_state->pipe_state.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        const auto &create_info = pCreateInfos[i];
        auto layout_state = Get<PIPELINE_LAYOUT_STATE>(create_info.layout);
        std::shared_ptr<const RENDER_PASS_STATE> render_pass;

        if (pCreateInfos[i].renderPass != VK_NULL_HANDLE) {
            render_pass = Get<RENDER_PASS_STATE>(create_info.renderPass);
        } else if (enabled_features.dynamicRendering) {
            auto dynamic_rendering = vku::FindStructInPNextChain<VkPipelineRenderingCreateInfo>(create_info.pNext);
            const bool rasterization_enabled = PIPELINE_STATE::EnablesRasterizationStates(*this, create_info);
            const bool has_fragment_output_state =
                PIPELINE_STATE::ContainsSubState(this, create_info, VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT);
            render_pass =
                std::make_shared<RENDER_PASS_STATE>(dynamic_rendering, rasterization_enabled && has_fragment_output_state);
        } else {
            const bool is_graphics_lib = GetGraphicsLibType(create_info) != static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0);
            const bool has_link_info = vku::FindStructInPNextChain<VkPipelineLibraryCreateInfoKHR>(create_info.pNext) != nullptr;
            if (!is_graphics_lib && !has_link_info) {
                skip = true;
            }
        }
        auto csm_states = (cgpl_state->shader_states.size() > i) ? &cgpl_state->shader_states[i] : nullptr;
        cgpl_state->pipe_state.push_back(
            CreateGraphicsPipelineState(&create_info, std::move(render_pass), std::move(layout_state), csm_states));
    }
    return skip;
}

void ValidationStateTracker::PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                   const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                                   const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                   const RecordObject &record_obj, void *cgpl_state_data) {
    create_graphics_pipeline_api_state *cgpl_state = reinterpret_cast<create_graphics_pipeline_api_state *>(cgpl_state_data);
    // This API may create pipelines regardless of the return value
    for (uint32_t i = 0; i < count; i++) {
        if (pPipelines[i] != VK_NULL_HANDLE) {
            (cgpl_state->pipe_state)[i]->SetHandle(pPipelines[i]);
            Add(std::move((cgpl_state->pipe_state)[i]));
        }
    }
    cgpl_state->pipe_state.clear();
}

std::shared_ptr<PIPELINE_STATE> ValidationStateTracker::CreateComputePipelineState(
    const VkComputePipelineCreateInfo *pCreateInfo, std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout) const {
    return std::make_shared<PIPELINE_STATE>(this, pCreateInfo, std::move(layout));
}

bool ValidationStateTracker::PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                   const VkComputePipelineCreateInfo *pCreateInfos,
                                                                   const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                   const ErrorObject &error_obj, void *ccpl_state_data) const {
    auto *ccpl_state = reinterpret_cast<create_compute_pipeline_api_state *>(ccpl_state_data);
    ccpl_state->pCreateInfos = pCreateInfos;  // GPU validation can alter this, so we have to set a default value for the Chassis
    ccpl_state->pipe_state.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        // Create and initialize internal tracking data structure
        ccpl_state->pipe_state.push_back(
            CreateComputePipelineState(&pCreateInfos[i], Get<PIPELINE_LAYOUT_STATE>(pCreateInfos[i].layout)));
    }
    return false;
}

void ValidationStateTracker::PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                  const VkComputePipelineCreateInfo *pCreateInfos,
                                                                  const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                  const RecordObject &record_obj, void *ccpl_state_data) {
    create_compute_pipeline_api_state *ccpl_state = reinterpret_cast<create_compute_pipeline_api_state *>(ccpl_state_data);

    // This API may create pipelines regardless of the return value
    for (uint32_t i = 0; i < count; i++) {
        if (pPipelines[i] != VK_NULL_HANDLE) {
            (ccpl_state->pipe_state)[i]->SetHandle(pPipelines[i]);
            Add(std::move((ccpl_state->pipe_state)[i]));
        }
    }
    ccpl_state->pipe_state.clear();
}

std::shared_ptr<PIPELINE_STATE> ValidationStateTracker::CreateRayTracingPipelineState(
    const VkRayTracingPipelineCreateInfoNV *pCreateInfo, std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout) const {
    return std::make_shared<PIPELINE_STATE>(this, pCreateInfo, std::move(layout));
}

bool ValidationStateTracker::PreCallValidateCreateRayTracingPipelinesNV(
    VkDevice device, VkPipelineCache pipelineCache, uint32_t count, const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines, const ErrorObject &error_obj, void *crtpl_state_data) const {
    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_api_state *>(crtpl_state_data);
    crtpl_state->pipe_state.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        // Create and initialize internal tracking data structure
        crtpl_state->pipe_state.push_back(
            CreateRayTracingPipelineState(&pCreateInfos[i], Get<PIPELINE_LAYOUT_STATE>(pCreateInfos[i].layout)));
    }
    return false;
}

void ValidationStateTracker::PostCallRecordCreateRayTracingPipelinesNV(
    VkDevice device, VkPipelineCache pipelineCache, uint32_t count, const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines, const RecordObject &record_obj, void *crtpl_state_data) {
    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_api_state *>(crtpl_state_data);
    // This API may create pipelines regardless of the return value
    for (uint32_t i = 0; i < count; i++) {
        if (pPipelines[i] != VK_NULL_HANDLE) {
            (crtpl_state->pipe_state)[i]->SetHandle(pPipelines[i]);
            Add(std::move((crtpl_state->pipe_state)[i]));
        }
    }
    crtpl_state->pipe_state.clear();
}

std::shared_ptr<PIPELINE_STATE> ValidationStateTracker::CreateRayTracingPipelineState(
    const VkRayTracingPipelineCreateInfoKHR *pCreateInfo, std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout) const {
    return std::make_shared<PIPELINE_STATE>(this, pCreateInfo, std::move(layout));
}

bool ValidationStateTracker::PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                         VkPipelineCache pipelineCache, uint32_t count,
                                                                         const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                                         const VkAllocationCallbacks *pAllocator,
                                                                         VkPipeline *pPipelines, const ErrorObject &error_obj,
                                                                         void *crtpl_state_data) const {
    auto crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_khr_api_state *>(crtpl_state_data);
    crtpl_state->pipe_state.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        // Create and initialize internal tracking data structure
        crtpl_state->pipe_state.push_back(
            CreateRayTracingPipelineState(&pCreateInfos[i], Get<PIPELINE_LAYOUT_STATE>(pCreateInfos[i].layout)));
    }
    return false;
}

void ValidationStateTracker::PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                        VkPipelineCache pipelineCache, uint32_t count,
                                                                        const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                                        const VkAllocationCallbacks *pAllocator,
                                                                        VkPipeline *pPipelines, const RecordObject &record_obj,
                                                                        void *crtpl_state_data) {
    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_khr_api_state *>(crtpl_state_data);
    const bool operation_is_deferred = (deferredOperation != VK_NULL_HANDLE && record_obj.result == VK_OPERATION_DEFERRED_KHR);
    // This API may create pipelines regardless of the return value

    if (!operation_is_deferred) {
        for (uint32_t i = 0; i < count; i++) {
            if (pPipelines[i] != VK_NULL_HANDLE) {
                (crtpl_state->pipe_state)[i]->SetHandle(pPipelines[i]);
                Add(std::move((crtpl_state->pipe_state)[i]));
            }
        }
    } else {
        auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
        if (wrap_handles) {
            deferredOperation = layer_data->Unwrap(deferredOperation);
        }
        std::vector<std::function<void(const std::vector<VkPipeline> &)>> cleanup_fn;
        auto find_res = layer_data->deferred_operation_post_check.pop(deferredOperation);
        if (find_res->first) {
            cleanup_fn = std::move(find_res->second);
        }
        auto &pipeline_states = crtpl_state->pipe_state;
        // Mutable lambda because we want to move the shared pointer contained in the copied vector
        cleanup_fn.emplace_back([this, pipeline_states](const std::vector<VkPipeline> &pipelines) mutable {
            for (size_t i = 0; i < pipeline_states.size(); ++i) {
                pipeline_states[i]->SetHandle(pipelines[i]);
                this->Add(std::move(pipeline_states[i]));
            }
        });
        layer_data->deferred_operation_post_check.insert(deferredOperation, cleanup_fn);
    }
    crtpl_state->pipe_state.clear();
}

std::shared_ptr<SAMPLER_STATE> ValidationStateTracker::CreateSamplerState(VkSampler s, const VkSamplerCreateInfo *ci) {
    return std::make_shared<SAMPLER_STATE>(s, ci);
}

void ValidationStateTracker::PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator, VkSampler *pSampler,
                                                         const RecordObject &record_obj) {
    Add(CreateSamplerState(*pSampler, pCreateInfo));
    if (pCreateInfo->borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT ||
        pCreateInfo->borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT) {
        custom_border_color_sampler_count++;
    }
}

void ValidationStateTracker::PostCallRecordCreateDescriptorSetLayout(VkDevice device,
                                                                     const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkDescriptorSetLayout *pSetLayout,
                                                                     const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(std::make_shared<cvdescriptorset::DescriptorSetLayout>(pCreateInfo, *pSetLayout));
}

void ValidationStateTracker::PostCallRecordGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                                         VkDeviceSize *pLayoutSizeInBytes,
                                                                         const RecordObject &record_obj) {
    auto descriptor_set_layout = Get<cvdescriptorset::DescriptorSetLayout>(layout);

    descriptor_set_layout->SetLayoutSizeInBytes(pLayoutSizeInBytes);
}

void ValidationStateTracker::PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                VkPipelineLayout *pPipelineLayout, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(std::make_shared<PIPELINE_LAYOUT_STATE>(this, *pPipelineLayout, pCreateInfo));
}

std::shared_ptr<DESCRIPTOR_POOL_STATE> ValidationStateTracker::CreateDescriptorPoolState(
    VkDescriptorPool pool, const VkDescriptorPoolCreateInfo *pCreateInfo) {
    return std::make_shared<DESCRIPTOR_POOL_STATE>(this, pool, pCreateInfo);
}

std::shared_ptr<cvdescriptorset::DescriptorSet> ValidationStateTracker::CreateDescriptorSet(
    VkDescriptorSet set, DESCRIPTOR_POOL_STATE *pool, const std::shared_ptr<cvdescriptorset::DescriptorSetLayout const> &layout,
    uint32_t variable_count) {
    return std::make_shared<cvdescriptorset::DescriptorSet>(set, pool, layout, variable_count, this);
}

void ValidationStateTracker::PostCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                VkDescriptorPool *pDescriptorPool, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(CreateDescriptorPoolState(*pDescriptorPool, pCreateInfo));
}

void ValidationStateTracker::PostCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                               VkDescriptorPoolResetFlags flags, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    auto pool = Get<DESCRIPTOR_POOL_STATE>(descriptorPool);
    if (pool) {
        pool->Reset();
    }
}

bool ValidationStateTracker::PreCallValidateAllocateDescriptorSets(VkDevice device,
                                                                   const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                                   VkDescriptorSet *pDescriptorSets, const ErrorObject &error_obj,
                                                                   void *ads_state_data) const {
    // Always update common data
    cvdescriptorset::AllocateDescriptorSetsData *ads_state =
        reinterpret_cast<cvdescriptorset::AllocateDescriptorSetsData *>(ads_state_data);
    UpdateAllocateDescriptorSetsData(pAllocateInfo, ads_state);

    return false;
}

// Allocation state was good and call down chain was made so update state based on allocating descriptor sets
void ValidationStateTracker::PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                                  VkDescriptorSet *pDescriptorSets, const RecordObject &record_obj,
                                                                  void *ads_state_data) {
    if (VK_SUCCESS != record_obj.result) return;
    // All the updates are contained in a single cvdescriptorset function
    cvdescriptorset::AllocateDescriptorSetsData *ads_state =
        reinterpret_cast<cvdescriptorset::AllocateDescriptorSetsData *>(ads_state_data);
    auto pool_state = Get<DESCRIPTOR_POOL_STATE>(pAllocateInfo->descriptorPool);
    if (pool_state) {
        pool_state->Allocate(pAllocateInfo, pDescriptorSets, ads_state);
    }
}

void ValidationStateTracker::PreCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count,
                                                             const VkDescriptorSet *pDescriptorSets) {
    auto pool_state = Get<DESCRIPTOR_POOL_STATE>(descriptorPool);
    if (pool_state) {
        pool_state->Free(count, pDescriptorSets);
    }
}

void ValidationStateTracker::PerformUpdateDescriptorSets(uint32_t write_count, const VkWriteDescriptorSet *p_wds,
                                                         uint32_t copy_count, const VkCopyDescriptorSet *p_cds) {
    // Write updates first
    uint32_t i = 0;
    for (i = 0; i < write_count; ++i) {
        auto dest_set = p_wds[i].dstSet;
        auto set_node = Get<cvdescriptorset::DescriptorSet>(dest_set);
        if (set_node) {
            set_node->PerformWriteUpdate(p_wds[i]);
        }
    }
    // Now copy updates
    for (i = 0; i < copy_count; ++i) {
        auto dst_set = p_cds[i].dstSet;
        auto src_set = p_cds[i].srcSet;
        auto src_node = Get<cvdescriptorset::DescriptorSet>(src_set);
        auto dst_node = Get<cvdescriptorset::DescriptorSet>(dst_set);
        if (src_node && dst_node) {
            dst_node->PerformCopyUpdate(p_cds[i], *src_node);
        }
    }
}

void ValidationStateTracker::PreCallRecordUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                               const VkWriteDescriptorSet *pDescriptorWrites,
                                                               uint32_t descriptorCopyCount,
                                                               const VkCopyDescriptorSet *pDescriptorCopies) {
    PerformUpdateDescriptorSets(descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
}

void ValidationStateTracker::PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pCreateInfo,
                                                                  VkCommandBuffer *pCommandBuffers,
                                                                  const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    auto pool = Get<COMMAND_POOL_STATE>(pCreateInfo->commandPool);
    if (pool) {
        pool->Allocate(pCreateInfo, pCommandBuffers);
    }
}

void ValidationStateTracker::PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                                             const VkCommandBufferBeginInfo *pBeginInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state) return;

    cb_state->Begin(pBeginInfo);
}

void ValidationStateTracker::PostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state) return;

    cb_state->End(record_obj.result);
}

void ValidationStateTracker::PostCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                                              const RecordObject &record_obj) {
    if (VK_SUCCESS == record_obj.result) {
        auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
        if (cb_state) {
            cb_state->Reset();
        }
    }
}

// Validation cache:
// CV is the bottommost implementor of this extension. Don't pass calls down.

void ValidationStateTracker::PreCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                          VkPipeline pipeline) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    cb_state->RecordCmd(Func::vkCmdBindPipeline);

    auto pipe_state = Get<PIPELINE_STATE>(pipeline);
    if (VK_PIPELINE_BIND_POINT_GRAPHICS == pipelineBindPoint) {
        const auto *raster_state = pipe_state->RasterizationState();
        const bool rasterization_enabled = raster_state && !raster_state->rasterizerDiscardEnable;
        const auto *viewport_state = pipe_state->ViewportState();

        cb_state->dynamic_state_status.pipeline.reset();

        // Used to calculate CMD_BUFFER_STATE::usedViewportScissorCount upon draw command with this graphics pipeline.
        // If rasterization disabled (no viewport/scissors used), or the actual number of viewports/scissors is dynamic (unknown at
        // this time), then these are set to 0 to disable this checking.
        const auto has_dynamic_viewport_count = pipe_state->IsDynamic(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
        const auto has_dynamic_scissor_count = pipe_state->IsDynamic(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
        cb_state->pipelineStaticViewportCount =
            has_dynamic_viewport_count || !rasterization_enabled ? 0 : viewport_state->viewportCount;
        cb_state->pipelineStaticScissorCount =
            has_dynamic_scissor_count || !rasterization_enabled ? 0 : viewport_state->scissorCount;

        // Trash dynamic viewport/scissor state if pipeline defines static state and enabled rasterization.
        // akeley98 NOTE: There's a bit of an ambiguity in the spec, whether binding such a pipeline overwrites
        // the entire viewport (scissor) array, or only the subsection defined by the viewport (scissor) count.
        // I am taking the latter interpretation based on the implementation details of NVIDIA's Vulkan driver.
        if (!has_dynamic_viewport_count) {
            cb_state->trashedViewportCount = true;
            if (rasterization_enabled && (!pipe_state->IsDynamic(VK_DYNAMIC_STATE_VIEWPORT))) {
                cb_state->trashedViewportMask |= (uint32_t(1) << viewport_state->viewportCount) - 1u;
                // should become = ~uint32_t(0) if the other interpretation is correct.
            }
        }
        if (!has_dynamic_scissor_count) {
            cb_state->trashedScissorCount = true;
            if (rasterization_enabled && (!pipe_state->IsDynamic(VK_DYNAMIC_STATE_SCISSOR))) {
                cb_state->trashedScissorMask |= (uint32_t(1) << viewport_state->scissorCount) - 1u;
                // should become = ~uint32_t(0) if the other interpretation is correct.
            }
        }
    }
    cb_state->BindPipeline(ConvertToLvlBindPoint(pipelineBindPoint), pipe_state.get());
    if (!disabled[command_buffer_state]) {
        cb_state->AddChild(pipe_state);
    }
}

void ValidationStateTracker::PostCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                           VkPipeline pipeline, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    auto pipe_state = Get<PIPELINE_STATE>(pipeline);

    if (enabled_features.variableMultisampleRate == VK_FALSE) {
        if (const auto *multisample_state = pipe_state->MultisampleState(); multisample_state) {
            if (const auto &render_pass = cb_state->activeRenderPass; render_pass) {
                const uint32_t subpass = cb_state->GetActiveSubpass();
                // if render pass uses no attachment, all bound pipelines in the same subpass must have the same
                // pMultisampleState->rasterizationSamples. To check that, record pMultisampleState->rasterizationSamples of the
                // first bound pipeline.
                if (!render_pass->UsesDynamicRendering() && !render_pass->UsesColorAttachment(subpass) &&
                    !render_pass->UsesDepthStencilAttachment(subpass)) {
                    if (std::optional<VkSampleCountFlagBits> subpass_rasterization_samples =
                            cb_state->GetActiveSubpassRasterizationSampleCount();
                        !subpass_rasterization_samples) {
                        cb_state->SetActiveSubpassRasterizationSampleCount(multisample_state->rasterizationSamples);
                    }
                }
            }
        }
    }
}

void ValidationStateTracker::PostCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                          uint32_t viewportCount, const VkViewport *pViewports,
                                                          const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_VIEWPORT);
    uint32_t bits = ((1u << viewportCount) - 1u) << firstViewport;
    cb_state->viewportMask |= bits;
    cb_state->trashedViewportMask &= ~bits;

    if (cb_state->dynamic_state_value.viewports.size() < firstViewport + viewportCount) {
        cb_state->dynamic_state_value.viewports.resize(firstViewport + viewportCount);
    }
    for (size_t i = 0; i < viewportCount; ++i) {
        cb_state->dynamic_state_value.viewports[firstViewport + i] = pViewports[i];
    }
}

void ValidationStateTracker::PostCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                                    uint32_t exclusiveScissorCount,
                                                                    const VkRect2D *pExclusiveScissors,
                                                                    const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV);
    // TODO: We don't have VUIDs for validating that all exclusive scissors have been set.
    // cb_state->exclusiveScissorMask |= ((1u << exclusiveScissorCount) - 1u) << firstExclusiveScissor;

    cb_state->dynamic_state_value.exclusive_scissor_first = firstExclusiveScissor;
    cb_state->dynamic_state_value.exclusive_scissor_count = exclusiveScissorCount;
    cb_state->dynamic_state_value.exclusive_scissors.resize(firstExclusiveScissor + exclusiveScissorCount);
    for (size_t i = 0; i < exclusiveScissorCount; ++i) {
        cb_state->dynamic_state_value.exclusive_scissors[firstExclusiveScissor + i] = pExclusiveScissors[i];
    }
}

void ValidationStateTracker::PostCallRecordCmdSetExclusiveScissorEnableNV(VkCommandBuffer commandBuffer,
                                                                          uint32_t firstExclusiveScissor,
                                                                          uint32_t exclusiveScissorCount,
                                                                          const VkBool32 *pExclusiveScissorEnables,
                                                                          const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV);

    cb_state->dynamic_state_value.exclusive_scissor_enable_first = firstExclusiveScissor;
    cb_state->dynamic_state_value.exclusive_scissor_enable_count = exclusiveScissorCount;
    cb_state->dynamic_state_value.exclusive_scissor_enables.resize(firstExclusiveScissor + exclusiveScissorCount);
    for (size_t i = 0; i < exclusiveScissorCount; ++i) {
        cb_state->dynamic_state_value.exclusive_scissor_enables[firstExclusiveScissor + i] = pExclusiveScissorEnables[i];
    }
}

void ValidationStateTracker::PreCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                                    VkImageLayout imageLayout) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(Func::vkCmdBindShadingRateImageNV);

    if (imageView != VK_NULL_HANDLE) {
        auto view_state = Get<IMAGE_VIEW_STATE>(imageView);
        cb_state->AddChild(view_state);
    }
}

void ValidationStateTracker::PostCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                              uint32_t viewportCount,
                                                                              const VkShadingRatePaletteNV *pShadingRatePalettes,
                                                                              const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV);
    // TODO: We don't have VUIDs for validating that all shading rate palettes have been set.
    // cb_state->shadingRatePaletteMask |= ((1u << viewportCount) - 1u) << firstViewport;
    cb_state->dynamic_state_value.shading_rate_palette_count = viewportCount;
}

std::shared_ptr<ACCELERATION_STRUCTURE_STATE_NV> ValidationStateTracker::CreateAccelerationStructureState(
    VkAccelerationStructureNV as, const VkAccelerationStructureCreateInfoNV *ci) {
    return std::make_shared<ACCELERATION_STRUCTURE_STATE_NV>(device, as, ci);
}

void ValidationStateTracker::PostCallRecordCreateAccelerationStructureNV(VkDevice device,
                                                                         const VkAccelerationStructureCreateInfoNV *pCreateInfo,
                                                                         const VkAllocationCallbacks *pAllocator,
                                                                         VkAccelerationStructureNV *pAccelerationStructure,
                                                                         const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(CreateAccelerationStructureState(*pAccelerationStructure, pCreateInfo));
}

std::shared_ptr<ACCELERATION_STRUCTURE_STATE_KHR> ValidationStateTracker::CreateAccelerationStructureState(
    VkAccelerationStructureKHR as, const VkAccelerationStructureCreateInfoKHR *ci, std::shared_ptr<BUFFER_STATE> &&buf_state,
    VkDeviceAddress address) {
    return std::make_shared<ACCELERATION_STRUCTURE_STATE_KHR>(as, ci, std::move(buf_state), address);
}

void ValidationStateTracker::PostCallRecordCreateAccelerationStructureKHR(VkDevice device,
                                                                          const VkAccelerationStructureCreateInfoKHR *pCreateInfo,
                                                                          const VkAllocationCallbacks *pAllocator,
                                                                          VkAccelerationStructureKHR *pAccelerationStructure,
                                                                          const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    auto buffer_state = Get<BUFFER_STATE>(pCreateInfo->buffer);
    VkAccelerationStructureDeviceAddressInfoKHR as_address_info = vku::InitStructHelper();
    as_address_info.accelerationStructure = *pAccelerationStructure;
    const VkDeviceAddress as_address = DispatchGetAccelerationStructureDeviceAddressKHR(device, &as_address_info);
    Add(CreateAccelerationStructureState(*pAccelerationStructure, pCreateInfo, std::move(buffer_state), as_address));
}

void ValidationStateTracker::PostCallRecordBuildAccelerationStructuresKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, const RecordObject &record_obj) {
    for (uint32_t i = 0; i < infoCount; ++i) {
        auto dst_as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfos[i].dstAccelerationStructure);
        if (dst_as_state != nullptr) {
            dst_as_state->Build(&pInfos[i], true, *ppBuildRangeInfos);
        }
    }
}

// helper method for device side acceleration structure builds
void ValidationStateTracker::RecordDeviceAccelerationStructureBuildInfo(CMD_BUFFER_STATE &cb_state,
                                                                        const VkAccelerationStructureBuildGeometryInfoKHR &info) {
    auto dst_as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(info.dstAccelerationStructure);
    if (dst_as_state) {
        dst_as_state->Build(&info, false, nullptr);
    }
    if (disabled[command_buffer_state]) {
        return;
    }
    if (dst_as_state) {
        cb_state.AddChild(dst_as_state);
    }
    auto src_as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(info.srcAccelerationStructure);
    if (src_as_state) {
        cb_state.AddChild(src_as_state);
    }

    // Issue https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6461
    // showed that it is incorrect to try to add buffers obtained through a call to GetBuffersByAddress as children to a command
    // buffer
}

void ValidationStateTracker::PostCallRecordCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state) {
        return;
    }
    cb_state->RecordCmd(record_obj.location.function);
    for (uint32_t i = 0; i < infoCount; i++) {
        RecordDeviceAccelerationStructureBuildInfo(*cb_state, pInfos[i]);
    }
    cb_state->has_build_as_cmd = true;
}

void ValidationStateTracker::PostCallRecordCmdBuildAccelerationStructuresIndirectKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkDeviceAddress *pIndirectDeviceAddresses, const uint32_t *pIndirectStrides, const uint32_t *const *ppMaxPrimitiveCounts,
    const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state) {
        return;
    }
    cb_state->RecordCmd(record_obj.location.function);
    for (uint32_t i = 0; i < infoCount; i++) {
        RecordDeviceAccelerationStructureBuildInfo(*cb_state, pInfos[i]);
        // Issue https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6461
        // showed that it is incorrect to try to add buffers obtained through a call to GetBuffersByAddress as children to a command
        // buffer
    }
    cb_state->has_build_as_cmd = true;
}

void ValidationStateTracker::PostCallRecordGetAccelerationStructureMemoryRequirementsNV(
    VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV *pInfo, VkMemoryRequirements2 *pMemoryRequirements,
    const RecordObject &record_obj) {
    auto as_state = Get<ACCELERATION_STRUCTURE_STATE_NV>(pInfo->accelerationStructure);
    if (as_state != nullptr) {
        if (pInfo->type == VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV) {
            as_state->memory_requirements_checked = true;
        } else if (pInfo->type == VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV) {
            as_state->build_scratch_memory_requirements_checked = true;
        } else if (pInfo->type == VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV) {
            as_state->update_scratch_memory_requirements_checked = true;
        }
    }
}

void ValidationStateTracker::PostCallRecordBindAccelerationStructureMemoryNV(
    VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoNV *pBindInfos,
    const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        const VkBindAccelerationStructureMemoryInfoNV &info = pBindInfos[i];

        auto as_state = Get<ACCELERATION_STRUCTURE_STATE_NV>(info.accelerationStructure);
        if (as_state) {
            // Track objects tied to memory
            auto mem_state = Get<DEVICE_MEMORY_STATE>(info.memory);
            if (mem_state) {
                as_state->BindMemory(as_state.get(), mem_state, info.memoryOffset, 0u, as_state->memory_requirements.size);
            }

            // GPU validation of top level acceleration structure building needs acceleration structure handles.
            // XXX TODO: Query device address for KHR extension
            if (enabled[gpu_validation]) {
                DispatchGetAccelerationStructureHandleNV(device, info.accelerationStructure, 8, &as_state->opaque_handle);
            }
        }
    }
}

void ValidationStateTracker::PostCallRecordCmdBuildAccelerationStructureNV(
    VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV *pInfo, VkBuffer instanceData, VkDeviceSize instanceOffset,
    VkBool32 update, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch, VkDeviceSize scratchOffset,
    const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state) {
        return;
    }
    cb_state->RecordCmd(record_obj.location.function);

    auto dst_as_state = Get<ACCELERATION_STRUCTURE_STATE_NV>(dst);
    if (dst_as_state) {
        dst_as_state->Build(pInfo);
        if (!disabled[command_buffer_state]) {
            cb_state->AddChild(dst_as_state);
        }
    }
    if (!disabled[command_buffer_state]) {
        auto src_as_state = Get<ACCELERATION_STRUCTURE_STATE_NV>(src);
        if (src_as_state) {
            cb_state->AddChild(src_as_state);
        }
        auto instance_buffer = Get<BUFFER_STATE>(instanceData);
        if (instance_buffer) {
            cb_state->AddChild(instance_buffer);
        }
        auto scratch_buffer = Get<BUFFER_STATE>(scratch);
        if (scratch_buffer) {
            cb_state->AddChild(scratch_buffer);
        }

        for (uint32_t i = 0; i < pInfo->geometryCount; i++) {
            const auto &geom = pInfo->pGeometries[i];

            auto vertex_buffer = Get<BUFFER_STATE>(geom.geometry.triangles.vertexData);
            if (vertex_buffer) {
                cb_state->AddChild(vertex_buffer);
            }
            auto index_buffer = Get<BUFFER_STATE>(geom.geometry.triangles.indexData);
            if (index_buffer) {
                cb_state->AddChild(index_buffer);
            }
            auto transform_buffer = Get<BUFFER_STATE>(geom.geometry.triangles.transformData);
            if (transform_buffer) {
                cb_state->AddChild(transform_buffer);
            }

            auto aabb_buffer = Get<BUFFER_STATE>(geom.geometry.aabbs.aabbData);
            if (aabb_buffer) {
                cb_state->AddChild(aabb_buffer);
            }
        }
    }
    cb_state->has_build_as_cmd = true;
}

void ValidationStateTracker::PostCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                                          VkAccelerationStructureNV dst,
                                                                          VkAccelerationStructureNV src,
                                                                          VkCopyAccelerationStructureModeNV mode,
                                                                          const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_state) {
        auto src_as_state = Get<ACCELERATION_STRUCTURE_STATE_NV>(src);
        auto dst_as_state = Get<ACCELERATION_STRUCTURE_STATE_NV>(dst);
        if (!disabled[command_buffer_state]) {
            cb_state->RecordTransferCmd(record_obj.location.function, src_as_state, dst_as_state);
        }
        if (dst_as_state != nullptr && src_as_state != nullptr) {
            dst_as_state->built = true;
            dst_as_state->build_info = src_as_state->build_info;
        }
    }
}

void ValidationStateTracker::PreCallRecordDestroyAccelerationStructureKHR(VkDevice device,
                                                                          VkAccelerationStructureKHR accelerationStructure,
                                                                          const VkAllocationCallbacks *pAllocator) {
    Destroy<ACCELERATION_STRUCTURE_STATE_KHR>(accelerationStructure);
}

void ValidationStateTracker::PreCallRecordDestroyAccelerationStructureNV(VkDevice device,
                                                                         VkAccelerationStructureNV accelerationStructure,
                                                                         const VkAllocationCallbacks *pAllocator) {
    Destroy<ACCELERATION_STRUCTURE_STATE_NV>(accelerationStructure);
}

void ValidationStateTracker::PostCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                    uint32_t viewportCount,
                                                                    const VkViewportWScalingNV *pViewportWScalings,
                                                                    const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV);
    cb_state->dynamic_state_value.viewport_w_scaling_first = firstViewport;
    cb_state->dynamic_state_value.viewport_w_scaling_count = viewportCount;
    cb_state->dynamic_state_value.viewport_w_scalings.resize(viewportCount);
    for (size_t i = 0; i < viewportCount; ++i) {
        cb_state->dynamic_state_value.viewport_w_scalings[i] = pViewportWScalings[i];
    }
}

void ValidationStateTracker::PostCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth,
                                                           const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_LINE_WIDTH);
}

void ValidationStateTracker::PostCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                                uint16_t lineStipplePattern, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_LINE_STIPPLE_EXT);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor,
                                                           float depthBiasClamp, float depthBiasSlopeFactor,
                                                           const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DEPTH_BIAS);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthBias2EXT(VkCommandBuffer commandBuffer,
                                                               const VkDepthBiasInfoEXT *pDepthBiasInfo,
                                                               const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DEPTH_BIAS);
}

void ValidationStateTracker::PostCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor,
                                                         uint32_t scissorCount, const VkRect2D *pScissors,
                                                         const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_SCISSOR);
    uint32_t bits = ((1u << scissorCount) - 1u) << firstScissor;
    cb_state->scissorMask |= bits;
    cb_state->trashedScissorMask &= ~bits;
}

void ValidationStateTracker::PostCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4],
                                                                const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_BLEND_CONSTANTS);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds,
                                                             float maxDepthBounds, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DEPTH_BOUNDS);
}

void ValidationStateTracker::PostCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                                    uint32_t compareMask, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
}

void ValidationStateTracker::PostCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                                  uint32_t writeMask, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_STENCIL_WRITE_MASK);
    if (faceMask == VK_STENCIL_FACE_FRONT_BIT || faceMask == VK_STENCIL_FACE_FRONT_AND_BACK) {
        cb_state->dynamic_state_value.write_mask_front = writeMask;
    }
    if (faceMask == VK_STENCIL_FACE_BACK_BIT || faceMask == VK_STENCIL_FACE_FRONT_AND_BACK) {
        cb_state->dynamic_state_value.write_mask_back = writeMask;
    }
}

void ValidationStateTracker::PostCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                                  uint32_t reference, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_STENCIL_REFERENCE);
}

// Update the bound state for the bind point, including the effects of incompatible pipeline layouts
void ValidationStateTracker::PreCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer,
                                                                VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                uint32_t firstSet, uint32_t setCount,
                                                                const VkDescriptorSet *pDescriptorSets, uint32_t dynamicOffsetCount,
                                                                const uint32_t *pDynamicOffsets) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto pipeline_layout = Get<PIPELINE_LAYOUT_STATE>(layout);
    if (!cb_state || !pipeline_layout) {
        return;
    }
    cb_state->RecordCmd(Func::vkCmdBindDescriptorSets);

    std::shared_ptr<cvdescriptorset::DescriptorSet> no_push_desc;

    cb_state->UpdateLastBoundDescriptorSets(pipelineBindPoint, *pipeline_layout, firstSet, setCount, pDescriptorSets, no_push_desc,
                                            dynamicOffsetCount, pDynamicOffsets);
}

void ValidationStateTracker::PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer,
                                                                  VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                  uint32_t set, uint32_t descriptorWriteCount,
                                                                  const VkWriteDescriptorSet *pDescriptorWrites) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto pipeline_layout = Get<PIPELINE_LAYOUT_STATE>(layout);
    cb_state->PushDescriptorSetState(pipelineBindPoint, *pipeline_layout, set, descriptorWriteCount, pDescriptorWrites);
}

void ValidationStateTracker::PreCallRecordCmdBindDescriptorBuffersEXT(VkCommandBuffer commandBuffer, uint32_t bufferCount,
                                                                      const VkDescriptorBufferBindingInfoEXT *pBindingInfos) {
    auto cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->descriptor_buffer_binding_info.resize(bufferCount);

    std::copy(pBindingInfos, pBindingInfos + bufferCount, cb_state->descriptor_buffer_binding_info.data());
}

void ValidationStateTracker::PreCallRecordCmdSetDescriptorBufferOffsetsEXT(VkCommandBuffer commandBuffer,
                                                                           VkPipelineBindPoint pipelineBindPoint,
                                                                           VkPipelineLayout layout, uint32_t firstSet,
                                                                           uint32_t setCount, const uint32_t *pBufferIndices,
                                                                           const VkDeviceSize *pOffsets) {
    auto cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    auto pipeline_layout = Get<PIPELINE_LAYOUT_STATE>(layout);

    cb_state->UpdateLastBoundDescriptorBuffers(pipelineBindPoint, *pipeline_layout, firstSet, setCount, pBufferIndices, pOffsets);
}

void ValidationStateTracker::PostCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                            VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                                                            const void *pValues, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_state) {
        cb_state->RecordCmd(record_obj.location.function);
        auto layout_state = Get<PIPELINE_LAYOUT_STATE>(layout);
        cb_state->ResetPushConstantDataIfIncompatible(layout_state.get());

        auto &push_constant_data = cb_state->push_constant_data;
        assert((offset + size) <= static_cast<uint32_t>(push_constant_data.size()));
        std::memcpy(push_constant_data.data() + offset, pValues, static_cast<std::size_t>(size));
    }
}

void ValidationStateTracker::PreCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             VkIndexType indexType) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->index_buffer_binding = IndexBufferBinding(Get<BUFFER_STATE>(buffer), offset, indexType);

    // Add binding for this index buffer to this commandbuffer
    if (!disabled[command_buffer_state]) {
        cb_state->AddChild(cb_state->index_buffer_binding.buffer_state);
    }
}

void ValidationStateTracker::PreCallRecordCmdBindIndexBuffer2KHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                 VkDeviceSize offset, VkDeviceSize size, VkIndexType indexType) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->index_buffer_binding = IndexBufferBinding(Get<BUFFER_STATE>(buffer), size, offset, indexType);

    // Add binding for this index buffer to this commandbuffer
    if (!disabled[command_buffer_state]) {
        cb_state->AddChild(cb_state->index_buffer_binding.buffer_state);
    }
}

void ValidationStateTracker::PreCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                               uint32_t bindingCount, const VkBuffer *pBuffers,
                                                               const VkDeviceSize *pOffsets) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(Func::vkCmdBindVertexBuffers);

    uint32_t end = firstBinding + bindingCount;
    if (cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings.size() < end) {
        cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings.resize(end);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        auto &vertex_buffer_binding = cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings[i + firstBinding];
        vertex_buffer_binding = BufferBinding(Get<BUFFER_STATE>(pBuffers[i]), pOffsets[i]);

        // Add binding for this vertex buffer to this commandbuffer
        if (pBuffers[i] && !disabled[command_buffer_state]) {
            cb_state->AddChild(vertex_buffer_binding.buffer_state);
        }
    }
}

void ValidationStateTracker::PostCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                                                           VkDeviceSize dstOffset, VkDeviceSize dataSize, const void *pData,
                                                           const RecordObject &record_obj) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(record_obj.location.function, Get<BUFFER_STATE>(dstBuffer));
}

void ValidationStateTracker::PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                                      VkPipelineStageFlags stageMask) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordSetEvent(Func::vkCmdSetEvent, event, stageMask);
}

void ValidationStateTracker::PreCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                          const VkDependencyInfoKHR *pDependencyInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto stage_masks = sync_utils::GetGlobalStageMasks(*pDependencyInfo);

    cb_state->RecordSetEvent(Func::vkCmdSetEvent2KHR, event, stage_masks.src);
    cb_state->RecordBarriers(*pDependencyInfo);
}

void ValidationStateTracker::PreCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                                       const VkDependencyInfo *pDependencyInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto stage_masks = sync_utils::GetGlobalStageMasks(*pDependencyInfo);

    cb_state->RecordSetEvent(Func::vkCmdSetEvent2, event, stage_masks.src);
    cb_state->RecordBarriers(*pDependencyInfo);
}

void ValidationStateTracker::PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                                        VkPipelineStageFlags stageMask) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordResetEvent(Func::vkCmdResetEvent, event, stageMask);
}

void ValidationStateTracker::PreCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                            VkPipelineStageFlags2KHR stageMask) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordResetEvent(Func::vkCmdResetEvent2KHR, event, stageMask);
}

void ValidationStateTracker::PreCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                                         VkPipelineStageFlags2 stageMask) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordResetEvent(Func::vkCmdResetEvent2, event, stageMask);
}

void ValidationStateTracker::PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                        VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                                        uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                        uint32_t bufferMemoryBarrierCount,
                                                        const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                        uint32_t imageMemoryBarrierCount,
                                                        const VkImageMemoryBarrier *pImageMemoryBarriers) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordWaitEvents(Func::vkCmdWaitEvents, eventCount, pEvents, sourceStageMask);
    cb_state->RecordBarriers(memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                             imageMemoryBarrierCount, pImageMemoryBarriers);
}

void ValidationStateTracker::PreCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount,
                                                            const VkEvent *pEvents, const VkDependencyInfoKHR *pDependencyInfos) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    for (uint32_t i = 0; i < eventCount; i++) {
        const auto &dep_info = pDependencyInfos[i];
        auto stage_masks = sync_utils::GetGlobalStageMasks(dep_info);
        cb_state->RecordWaitEvents(Func::vkCmdWaitEvents2KHR, 1, &pEvents[i], stage_masks.src);
        cb_state->RecordBarriers(dep_info);
    }
}

void ValidationStateTracker::PreCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                         const VkDependencyInfo *pDependencyInfos) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    for (uint32_t i = 0; i < eventCount; i++) {
        const auto &dep_info = pDependencyInfos[i];
        auto stage_masks = sync_utils::GetGlobalStageMasks(dep_info);
        cb_state->RecordWaitEvents(Func::vkCmdWaitEvents2, 1, &pEvents[i], stage_masks.src);
        cb_state->RecordBarriers(dep_info);
    }
}

void ValidationStateTracker::PostCallRecordCmdPipelineBarrier(
    VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier *pImageMemoryBarriers, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(record_obj.location.function);
    cb_state->RecordBarriers(memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                             imageMemoryBarrierCount, pImageMemoryBarriers);
}

void ValidationStateTracker::PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer,
                                                                 const VkDependencyInfoKHR *pDependencyInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(Func::vkCmdPipelineBarrier2KHR);
    cb_state->RecordBarriers(*pDependencyInfo);
}

void ValidationStateTracker::PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer,
                                                              const VkDependencyInfo *pDependencyInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(Func::vkCmdPipelineBarrier2);
    cb_state->RecordBarriers(*pDependencyInfo);
}

void ValidationStateTracker::PostCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                                         VkQueryControlFlags flags, const RecordObject &record_obj) {
    if (disabled[query_validation]) return;
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);

    uint32_t num_queries = 1;
    // If render pass instance has multiview enabled, query uses N consecutive query indices
    if (cb_state->activeRenderPass) {
        uint32_t bits = cb_state->activeRenderPass->GetViewMaskBits(cb_state->GetActiveSubpass());
        num_queries = std::max(num_queries, bits);
    }
    for (uint32_t i = 0; i < num_queries; ++i) {
        QueryObject query_obj = {queryPool, slot, flags};
        cb_state->RecordCmd(record_obj.location.function);
        if (!disabled[query_validation]) {
            cb_state->BeginQuery(query_obj);
        }
        if (!disabled[command_buffer_state]) {
            auto pool_state = Get<QUERY_POOL_STATE>(queryPool);
            cb_state->AddChild(pool_state);
        }
    }
}

void ValidationStateTracker::PostCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                                       const RecordObject &record_obj) {
    if (disabled[query_validation]) return;
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    uint32_t num_queries = 1;
    // If render pass instance has multiview enabled, query uses N consecutive query indices
    if (cb_state->activeRenderPass) {
        uint32_t bits = cb_state->activeRenderPass->GetViewMaskBits(cb_state->GetActiveSubpass());
        num_queries = std::max(num_queries, bits);
    }

    for (uint32_t i = 0; i < num_queries; ++i) {
        QueryObject query_obj = {queryPool, slot + i};
        cb_state->RecordCmd(record_obj.location.function);
        if (!disabled[query_validation]) {
            cb_state->EndQuery(query_obj);
        }
        if (!disabled[command_buffer_state]) {
            auto pool_state = Get<QUERY_POOL_STATE>(queryPool);
            cb_state->AddChild(pool_state);
        }
    }
}

void ValidationStateTracker::PostCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                             uint32_t firstQuery, uint32_t queryCount,
                                                             const RecordObject &record_obj) {
    if (disabled[query_validation]) return;
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->RecordCmd(record_obj.location.function);
    cb_state->ResetQueryPool(queryPool, firstQuery, queryCount);

    if (!disabled[command_buffer_state]) {
        auto pool_state = Get<QUERY_POOL_STATE>(queryPool);
        cb_state->AddChild(pool_state);
    }
}

void ValidationStateTracker::PostCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                                   uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer,
                                                                   VkDeviceSize dstOffset, VkDeviceSize stride,
                                                                   VkQueryResultFlags flags, const RecordObject &record_obj) {
    if (disabled[query_validation] || disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(record_obj.location.function);
    auto dst_buff_state = Get<BUFFER_STATE>(dstBuffer);
    cb_state->AddChild(dst_buff_state);
    auto pool_state = Get<QUERY_POOL_STATE>(queryPool);
    cb_state->AddChild(pool_state);
}

void ValidationStateTracker::PostCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                             VkQueryPool queryPool, uint32_t slot, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordWriteTimestamp(record_obj.location.function, pipelineStage, queryPool, slot);
}

void ValidationStateTracker::PostCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer,
                                                                 VkPipelineStageFlags2KHR pipelineStage, VkQueryPool queryPool,
                                                                 uint32_t slot, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordWriteTimestamp(record_obj.location.function, pipelineStage, queryPool, slot);
}

void ValidationStateTracker::PostCallRecordCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 pipelineStage,
                                                              VkQueryPool queryPool, uint32_t slot,
                                                              const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordWriteTimestamp(record_obj.location.function, pipelineStage, queryPool, slot);
}

void ValidationStateTracker::PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery, const RecordObject &record_obj) {
    if (disabled[query_validation]) return;
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(record_obj.location.function);
    if (!disabled[command_buffer_state]) {
        auto pool_state = Get<QUERY_POOL_STATE>(queryPool);
        cb_state->AddChild(pool_state);
    }
    cb_state->EndQueries(queryPool, firstQuery, accelerationStructureCount);
}

void ValidationStateTracker::PostCallRecordCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR *pCreateInfo,
                                                                 const VkAllocationCallbacks *pAllocator,
                                                                 VkVideoSessionKHR *pVideoSession, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    auto profile_desc = video_profile_cache_.Get(this, pCreateInfo->pVideoProfile);
    Add(std::make_shared<VIDEO_SESSION_STATE>(this, *pVideoSession, pCreateInfo, std::move(profile_desc)));
}

void ValidationStateTracker::PostCallRecordGetVideoSessionMemoryRequirementsKHR(
    VkDevice device, VkVideoSessionKHR videoSession, uint32_t *pMemoryRequirementsCount,
    VkVideoSessionMemoryRequirementsKHR *pMemoryRequirements, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    auto vs_state = Get<VIDEO_SESSION_STATE>(videoSession);
    assert(vs_state);

    if (pMemoryRequirements != nullptr) {
        if (*pMemoryRequirementsCount > vs_state->memory_bindings_queried) {
            vs_state->memory_bindings_queried = *pMemoryRequirementsCount;
        }
    } else {
        vs_state->memory_binding_count_queried = true;
    }
}

void ValidationStateTracker::PostCallRecordBindVideoSessionMemoryKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                                     uint32_t bindSessionMemoryInfoCount,
                                                                     const VkBindVideoSessionMemoryInfoKHR *pBindSessionMemoryInfos,
                                                                     const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    auto vs_state = Get<VIDEO_SESSION_STATE>(videoSession);
    assert(vs_state);

    for (uint32_t i = 0; i < bindSessionMemoryInfoCount; ++i) {
        vs_state->BindMemoryBindingIndex(pBindSessionMemoryInfos[i].memoryBindIndex);
    }
}

void ValidationStateTracker::PreCallRecordDestroyVideoSessionKHR(VkDevice device, VkVideoSessionKHR videoSession,
                                                                 const VkAllocationCallbacks *pAllocator) {
    Destroy<VIDEO_SESSION_STATE>(videoSession);
}

void ValidationStateTracker::PostCallRecordCreateVideoSessionParametersKHR(VkDevice device,
                                                                           const VkVideoSessionParametersCreateInfoKHR *pCreateInfo,
                                                                           const VkAllocationCallbacks *pAllocator,
                                                                           VkVideoSessionParametersKHR *pVideoSessionParameters,
                                                                           const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    Add(std::make_shared<VIDEO_SESSION_PARAMETERS_STATE>(
        *pVideoSessionParameters, pCreateInfo, Get<VIDEO_SESSION_STATE>(pCreateInfo->videoSession),
        Get<VIDEO_SESSION_PARAMETERS_STATE>(pCreateInfo->videoSessionParametersTemplate)));
}

void ValidationStateTracker::PostCallRecordUpdateVideoSessionParametersKHR(VkDevice device,
                                                                           VkVideoSessionParametersKHR videoSessionParameters,
                                                                           const VkVideoSessionParametersUpdateInfoKHR *pUpdateInfo,
                                                                           const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    Get<VIDEO_SESSION_PARAMETERS_STATE>(videoSessionParameters)->Update(pUpdateInfo);
}

void ValidationStateTracker::PreCallRecordDestroyVideoSessionParametersKHR(VkDevice device,
                                                                           VkVideoSessionParametersKHR videoSessionParameters,
                                                                           const VkAllocationCallbacks *pAllocator) {
    Destroy<VIDEO_SESSION_PARAMETERS_STATE>(videoSessionParameters);
}

void ValidationStateTracker::PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer,
                                                             const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    std::vector<std::shared_ptr<IMAGE_VIEW_STATE>> views;
    if ((pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) == 0) {
        views.resize(pCreateInfo->attachmentCount);

        for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
            views[i] = Get<IMAGE_VIEW_STATE>(pCreateInfo->pAttachments[i]);
        }
    }

    Add(std::make_shared<FRAMEBUFFER_STATE>(*pFramebuffer, pCreateInfo, Get<RENDER_PASS_STATE>(pCreateInfo->renderPass),
                                            std::move(views)));
}

void ValidationStateTracker::PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                            const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(std::make_shared<RENDER_PASS_STATE>(*pRenderPass, pCreateInfo));
}

void ValidationStateTracker::PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                                const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    Add(std::make_shared<RENDER_PASS_STATE>(*pRenderPass, pCreateInfo));
}

void ValidationStateTracker::PostCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                             const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    Add(std::make_shared<RENDER_PASS_STATE>(*pRenderPass, pCreateInfo));
}

void ValidationStateTracker::PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer,
                                                             const VkRenderPassBeginInfo *pRenderPassBegin,
                                                             VkSubpassContents contents) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->BeginRenderPass(Func::vkCmdBeginRenderPass, pRenderPassBegin, contents);
}

void ValidationStateTracker::PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                                 const VkRenderPassBeginInfo *pRenderPassBegin,
                                                                 const VkSubpassBeginInfo *pSubpassBeginInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->BeginRenderPass(Func::vkCmdBeginRenderPass2KHR, pRenderPassBegin, pSubpassBeginInfo->contents);
}

void ValidationStateTracker::PreCallRecordCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                                 const VkVideoBeginCodingInfoKHR *pBeginInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->BeginVideoCoding(pBeginInfo);
}

void ValidationStateTracker::PostCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                                        uint32_t counterBufferCount,
                                                                        const VkBuffer *pCounterBuffers,
                                                                        const VkDeviceSize *pCounterBufferOffsets,
                                                                        const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->RecordCmd(record_obj.location.function);
    cb_state->transform_feedback_active = true;
}

void ValidationStateTracker::PostCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                                      uint32_t counterBufferCount, const VkBuffer *pCounterBuffers,
                                                                      const VkDeviceSize *pCounterBufferOffsets,
                                                                      const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->RecordCmd(record_obj.location.function);
    cb_state->transform_feedback_active = false;
}

void ValidationStateTracker::PostCallRecordCmdBeginConditionalRenderingEXT(
    VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT *pConditionalRenderingBegin,
    const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->RecordCmd(record_obj.location.function);
    cb_state->conditional_rendering_active = true;
    cb_state->conditional_rendering_inside_render_pass = cb_state->activeRenderPass != nullptr;
    cb_state->conditional_rendering_subpass = cb_state->GetActiveSubpass();
}

void ValidationStateTracker::PostCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer,
                                                                         const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->RecordCmd(record_obj.location.function);
    cb_state->conditional_rendering_active = false;
    cb_state->conditional_rendering_inside_render_pass = false;
    cb_state->conditional_rendering_subpass = 0;
}

void ValidationStateTracker::PreCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer,
                                                               const VkRenderingInfoKHR *pRenderingInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->BeginRendering(Func::vkCmdBeginRenderingKHR, pRenderingInfo);
}

void ValidationStateTracker::PreCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->BeginRendering(Func::vkCmdBeginRendering, pRenderingInfo);
}

void ValidationStateTracker::PreCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->EndRendering(Func::vkCmdEndRenderingKHR);
}

void ValidationStateTracker::PreCallRecordCmdEndRendering(VkCommandBuffer commandBuffer) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->EndRendering(Func::vkCmdEndRendering);
}

void ValidationStateTracker::PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer,
                                                              const VkRenderPassBeginInfo *pRenderPassBegin,
                                                              const VkSubpassBeginInfo *pSubpassBeginInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->BeginRenderPass(Func::vkCmdBeginRenderPass2, pRenderPassBegin, pSubpassBeginInfo->contents);
}

void ValidationStateTracker::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                                          const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->NextSubpass(record_obj.location.function, contents);
}

void ValidationStateTracker::PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer,
                                                              const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                              const VkSubpassEndInfo *pSubpassEndInfo,
                                                              const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->NextSubpass(record_obj.location.function, pSubpassBeginInfo->contents);
}

void ValidationStateTracker::PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer,
                                                           const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                           const VkSubpassEndInfo *pSubpassEndInfo,
                                                           const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->NextSubpass(record_obj.location.function, pSubpassBeginInfo->contents);
}

void ValidationStateTracker::PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->EndRenderPass(record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                                const VkSubpassEndInfo *pSubpassEndInfo,
                                                                const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->EndRenderPass(record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo *pSubpassEndInfo,
                                                             const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->EndRenderPass(record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                                const VkVideoEndCodingInfoKHR *pEndCodingInfo,
                                                                const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->EndVideoCoding(pEndCodingInfo);
}

void ValidationStateTracker::PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount,
                                                             const VkCommandBuffer *pCommandBuffers) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->ExecuteCommands({pCommandBuffers, commandBuffersCount});
}

void ValidationStateTracker::PostCallRecordMapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size,
                                                     VkFlags flags, void **ppData, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordMappedMemory(mem, offset, size, ppData);
}

void ValidationStateTracker::PostCallRecordMapMemory2KHR(VkDevice device, const VkMemoryMapInfoKHR *pMemoryMapInfo, void **ppData,
                                                         const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordMappedMemory(pMemoryMapInfo->memory, pMemoryMapInfo->offset, pMemoryMapInfo->size, ppData);
}

void ValidationStateTracker::PreCallRecordUnmapMemory(VkDevice device, VkDeviceMemory mem) {
    auto mem_info = Get<DEVICE_MEMORY_STATE>(mem);
    if (mem_info) {
        mem_info->mapped_range = MemRange();
        mem_info->p_driver_data = nullptr;
    }
}

void ValidationStateTracker::PreCallRecordUnmapMemory2KHR(VkDevice device, const VkMemoryUnmapInfoKHR *pMemoryUnmapInfo) {
    auto mem_info = Get<DEVICE_MEMORY_STATE>(pMemoryUnmapInfo->memory);
    if (mem_info) {
        mem_info->mapped_range = MemRange();
        mem_info->p_driver_data = nullptr;
    }
}

void ValidationStateTracker::UpdateBindImageMemoryState(const VkBindImageMemoryInfo &bindInfo) {
    auto image_state = Get<IMAGE_STATE>(bindInfo.image);
    if (image_state) {
        // An Android sepcial image cannot get VkSubresourceLayout until the image binds a memory.
        // See: VUID-vkGetImageSubresourceLayout-image-01895
        image_state->fragment_encoder =
            std::unique_ptr<const subresource_adapter::ImageRangeEncoder>(new subresource_adapter::ImageRangeEncoder(*image_state));
        const auto swapchain_info = vku::FindStructInPNextChain<VkBindImageMemorySwapchainInfoKHR>(bindInfo.pNext);
        if (swapchain_info) {
            auto swapchain = Get<SWAPCHAIN_NODE>(swapchain_info->swapchain);
            if (swapchain) {
                // All images bound to this swapchain and index are aliases
                image_state->SetSwapchain(swapchain, swapchain_info->imageIndex);
            }
        } else {
            // Track bound memory range information
            auto mem_info = Get<DEVICE_MEMORY_STATE>(bindInfo.memory);
            if (mem_info) {
                VkDeviceSize plane_index = 0u;
                if (image_state->disjoint && image_state->IsExternalBuffer() == false) {
                    auto plane_info = vku::FindStructInPNextChain<VkBindImagePlaneMemoryInfo>(bindInfo.pNext);
                    plane_index = vkuGetPlaneIndex(plane_info->planeAspect);
                }
                image_state->BindMemory(
                    image_state.get(), mem_info, bindInfo.memoryOffset, plane_index,
                    image_state->requirements[static_cast<decltype(image_state->requirements)::size_type>(plane_index)].size);
            }
        }
    }
}

VkBindImageMemoryInfo ValidationStateTracker::ConvertImageMemoryInfo(VkDevice device, VkImage image, VkDeviceMemory mem,
                                                                     VkDeviceSize memoryOffset) {
    VkBindImageMemoryInfo bind_info = vku::InitStructHelper();
    bind_info.image = image;
    bind_info.memory = mem;
    bind_info.memoryOffset = memoryOffset;
    return bind_info;
}

void ValidationStateTracker::PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem,
                                                           VkDeviceSize memoryOffset, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    UpdateBindImageMemoryState(ConvertImageMemoryInfo(device, image, mem, memoryOffset));
}

void ValidationStateTracker::PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                                            const VkBindImageMemoryInfo *pBindInfos,
                                                            const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        UpdateBindImageMemoryState(pBindInfos[i]);
    }
}

void ValidationStateTracker::PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                               const VkBindImageMemoryInfo *pBindInfos,
                                                               const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        UpdateBindImageMemoryState(pBindInfos[i]);
    }
}

void ValidationStateTracker::PreCallRecordSetEvent(VkDevice device, VkEvent event) {
    auto event_state = Get<EVENT_STATE>(event);
    if (event_state) {
        event_state->stageMask = VK_PIPELINE_STAGE_HOST_BIT;
    }
}

void ValidationStateTracker::PostCallRecordImportSemaphoreFdKHR(VkDevice device,
                                                                const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo,
                                                                const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordImportSemaphoreState(pImportSemaphoreFdInfo->semaphore, pImportSemaphoreFdInfo->handleType,
                               pImportSemaphoreFdInfo->flags);
}

void ValidationStateTracker::RecordGetExternalSemaphoreState(VkSemaphore semaphore,
                                                             VkExternalSemaphoreHandleTypeFlagBits handle_type) {
    auto semaphore_state = Get<SEMAPHORE_STATE>(semaphore);
    if (semaphore_state) {
        semaphore_state->Export(handle_type);
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void ValidationStateTracker::PostCallRecordImportSemaphoreWin32HandleKHR(
    VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR *pImportSemaphoreWin32HandleInfo, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordImportSemaphoreState(pImportSemaphoreWin32HandleInfo->semaphore, pImportSemaphoreWin32HandleInfo->handleType,
                               pImportSemaphoreWin32HandleInfo->flags);
}

void ValidationStateTracker::PostCallRecordGetSemaphoreWin32HandleKHR(VkDevice device,
                                                                      const VkSemaphoreGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                                      HANDLE *pHandle, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordGetExternalSemaphoreState(pGetWin32HandleInfo->semaphore, pGetWin32HandleInfo->handleType);
}

void ValidationStateTracker::PostCallRecordImportFenceWin32HandleKHR(
    VkDevice device, const VkImportFenceWin32HandleInfoKHR *pImportFenceWin32HandleInfo, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordImportFenceState(pImportFenceWin32HandleInfo->fence, pImportFenceWin32HandleInfo->handleType,
                           pImportFenceWin32HandleInfo->flags);
}

void ValidationStateTracker::PostCallRecordGetFenceWin32HandleKHR(VkDevice device,
                                                                  const VkFenceGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                                  HANDLE *pHandle, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordGetExternalFenceState(pGetWin32HandleInfo->fence, pGetWin32HandleInfo->handleType);
}
#endif

void ValidationStateTracker::PostCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR *pGetFdInfo, int *pFd,
                                                             const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordGetExternalSemaphoreState(pGetFdInfo->semaphore, pGetFdInfo->handleType);
}

void ValidationStateTracker::RecordImportFenceState(VkFence fence, VkExternalFenceHandleTypeFlagBits handle_type,
                                                    VkFenceImportFlags flags) {
    auto fence_node = Get<FENCE_STATE>(fence);

    if (fence_node) {
        fence_node->Import(handle_type, flags);
    }
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void ValidationStateTracker::PostCallRecordGetMemoryWin32HandleKHR(VkDevice device,
                                                                   const VkMemoryGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                                   HANDLE *pHandle, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    if (const auto memory_state = Get<DEVICE_MEMORY_STATE>(pGetWin32HandleInfo->memory)) {
        // For validation purposes we need to keep allocation size and memory type index.
        // There is no need to keep pNext chain.
        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
        alloc_info.allocationSize = memory_state->alloc_info.allocationSize;
        alloc_info.memoryTypeIndex = memory_state->alloc_info.memoryTypeIndex;

        WriteLockGuard guard(win32_handle_map_lock_);
        // `insert_or_assign` ensures that information is updated when the system decides to re-use
        // closed handle value for a new handle. The validation layer does not track handle close operation
        // which is performed by 'CloseHandle' system call.
        win32_handle_map_.insert_or_assign(*pHandle, alloc_info);
    }
}
#endif

void ValidationStateTracker::PostCallRecordGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR *pGetFdInfo, int *pFd,
                                                          const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    if (const auto memory_state = Get<DEVICE_MEMORY_STATE>(pGetFdInfo->memory)) {
        // For validation purposes we need to keep allocation size and memory type index.
        // There is no need to keep pNext chain.
        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
        alloc_info.allocationSize = memory_state->alloc_info.allocationSize;
        alloc_info.memoryTypeIndex = memory_state->alloc_info.memoryTypeIndex;

        WriteLockGuard guard(fd_handle_map_lock_);
        // `insert_or_assign` ensures that information is updated when the system decides to re-use
        // closed handle value for a new handle. The fd handle created inside Vulkan _can_ be closed
        // using the 'close' system call, which is not tracked by the validation layer.
        fd_handle_map_.insert_or_assign(*pFd, alloc_info);
    }
}

void ValidationStateTracker::PostCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *pImportFenceFdInfo,
                                                            const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordImportFenceState(pImportFenceFdInfo->fence, pImportFenceFdInfo->handleType, pImportFenceFdInfo->flags);
}

void ValidationStateTracker::RecordGetExternalFenceState(VkFence fence, VkExternalFenceHandleTypeFlagBits handle_type) {
    auto fence_state = Get<FENCE_STATE>(fence);
    if (fence_state) {
        // We no longer can track inflight fence after the export - perform early retire.
        fence_state->NotifyAndWait();
        fence_state->Export(handle_type);
    }
}

void ValidationStateTracker::PostCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *pGetFdInfo, int *pFd,
                                                         const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordGetExternalFenceState(pGetFdInfo->fence, pGetFdInfo->handleType);
}

void ValidationStateTracker::PostCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkEvent *pEvent,
                                                       const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    Add(std::make_shared<EVENT_STATE>(*pEvent, pCreateInfo));
}

void ValidationStateTracker::RecordCreateSwapchainState(VkResult result, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                                        VkSwapchainKHR *pSwapchain, std::shared_ptr<SURFACE_STATE> &&surface_state,
                                                        SWAPCHAIN_NODE *old_swapchain_state) {
    if (VK_SUCCESS == result) {
        if (surface_state->swapchain) {
            surface_state->RemoveParent(surface_state->swapchain);
        }
        auto swapchain = CreateSwapchainState(pCreateInfo, *pSwapchain);
        surface_state->AddParent(swapchain.get());
        surface_state->swapchain = swapchain.get();
        swapchain->surface = std::move(surface_state);
        auto swapchain_present_modes_ci = vku::FindStructInPNextChain<VkSwapchainPresentModesCreateInfoEXT>(pCreateInfo->pNext);
        if (swapchain_present_modes_ci) {
            const uint32_t present_mode_count = swapchain_present_modes_ci->presentModeCount;
            swapchain->present_modes.reserve(present_mode_count);
            std::copy(swapchain_present_modes_ci->pPresentModes, swapchain_present_modes_ci->pPresentModes + present_mode_count,
                      std::back_inserter(swapchain->present_modes));
        }
        Add(std::move(swapchain));
    } else {
        surface_state->swapchain = nullptr;
    }
    // Spec requires that even if CreateSwapchainKHR fails, oldSwapchain is retired
    // Retired swapchains remain associated with the surface until they are destroyed.
    if (old_swapchain_state) {
        old_swapchain_state->retired = true;
    }
    return;
}

void ValidationStateTracker::PostCallRecordCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain,
                                                              const RecordObject &record_obj) {
    auto surface_state = Get<SURFACE_STATE>(pCreateInfo->surface);
    auto old_swapchain_state = Get<SWAPCHAIN_NODE>(pCreateInfo->oldSwapchain);
    RecordCreateSwapchainState(record_obj.result, pCreateInfo, pSwapchain, std::move(surface_state), old_swapchain_state.get());
}

void ValidationStateTracker::PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                              const VkAllocationCallbacks *pAllocator) {
    Destroy<SWAPCHAIN_NODE>(swapchain);
}

void ValidationStateTracker::PostCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                                const VkDisplayModeCreateInfoKHR *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator, VkDisplayModeKHR *pMode,
                                                                const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    if (!pMode) return;
    Add(std::make_shared<DISPLAY_MODE_STATE>(*pMode, physicalDevice));
}

void ValidationStateTracker::PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo,
                                                           const RecordObject &record_obj) {
    // spec: If vkQueuePresentKHR fails to enqueue the corresponding set of queue operations, it may return
    // VK_ERROR_OUT_OF_HOST_MEMORY or VK_ERROR_OUT_OF_DEVICE_MEMORY. If it does, the implementation must ensure that the state and
    // contents of any resources or synchronization primitives referenced is unaffected by the call or its failure.
    //
    // If vkQueuePresentKHR fails in such a way that the implementation is unable to make that guarantee, the implementation must
    // return VK_ERROR_DEVICE_LOST.
    //
    // However, if the presentation request is rejected by the presentation engine with an error VK_ERROR_OUT_OF_DATE_KHR,
    // VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT, or VK_ERROR_SURFACE_LOST_KHR, the set of queue operations are still considered
    // to be enqueued and thus any semaphore wait operation specified in VkPresentInfoKHR will execute when the corresponding queue
    // operation is complete.
    //
    // NOTE: This is the only queue submit-like call that has its state updated in PostCallRecord(). In part that is because of
    // these non-fatal error cases. Also we need a place to handle the swapchain image bookkeeping, which really should be happening
    // once all the wait semaphores have completed. Since most of the PostCall queue submit race conditions are related to timeline
    // semaphores, and acquire sempaphores are always binary, this seems ok-ish.
    if (record_obj.result == VK_ERROR_OUT_OF_HOST_MEMORY || record_obj.result == VK_ERROR_OUT_OF_DEVICE_MEMORY ||
        record_obj.result == VK_ERROR_DEVICE_LOST) {
        return;
    }
    auto queue_state = Get<QUEUE_STATE>(queue);
    CB_SUBMISSION submission;
    for (uint32_t i = 0; i < pPresentInfo->waitSemaphoreCount; ++i) {
        auto semaphore_state = Get<SEMAPHORE_STATE>(pPresentInfo->pWaitSemaphores[i]);
        if (semaphore_state) {
            submission.AddWaitSemaphore(std::move(semaphore_state), 0);
        }
    }

    const auto *present_id_info = vku::FindStructInPNextChain<VkPresentIdKHR>(pPresentInfo->pNext);
    for (uint32_t i = 0; i < pPresentInfo->swapchainCount; ++i) {
        // Note: this is imperfect, in that we can get confused about what did or didn't succeed-- but if the app does that, it's
        // confused itself just as much.
        auto local_result = pPresentInfo->pResults ? pPresentInfo->pResults[i] : record_obj.result;
        if (local_result != VK_SUCCESS && local_result != VK_SUBOPTIMAL_KHR) continue;  // this present didn't actually happen.
        // Mark the image as having been released to the WSI
        auto swapchain_data = Get<SWAPCHAIN_NODE>(pPresentInfo->pSwapchains[i]);
        if (swapchain_data) {
            uint64_t present_id = (present_id_info && i < present_id_info->swapchainCount) ? present_id_info->pPresentIds[i] : 0;
            swapchain_data->PresentImage(pPresentInfo->pImageIndices[i], present_id);
        }
    }

    auto early_retire_seq = queue_state->Submit(std::move(submission));
    if (early_retire_seq) {
        queue_state->NotifyAndWait(early_retire_seq);
    }
}

void ValidationStateTracker::PostCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                                     const VkSwapchainCreateInfoKHR *pCreateInfos,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkSwapchainKHR *pSwapchains, const RecordObject &record_obj) {
    if (pCreateInfos) {
        for (uint32_t i = 0; i < swapchainCount; i++) {
            auto surface_state = Get<SURFACE_STATE>(pCreateInfos[i].surface);
            auto old_swapchain_state = Get<SWAPCHAIN_NODE>(pCreateInfos[i].oldSwapchain);
            RecordCreateSwapchainState(record_obj.result, &pCreateInfos[i], &pSwapchains[i], std::move(surface_state),
                                       old_swapchain_state.get());
        }
    }
}

void ValidationStateTracker::RecordAcquireNextImageState(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                         VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex,
                                                         vvl::Func command) {
    auto fence_state = Get<FENCE_STATE>(fence);
    if (fence_state) {
        // Treat as inflight since it is valid to wait on this fence, even in cases where it is technically a temporary
        // import
        fence_state->EnqueueSignal(nullptr, 0);
    }

    auto semaphore_state = Get<SEMAPHORE_STATE>(semaphore);
    if (semaphore_state) {
        // Treat as signaled since it is valid to wait on this semaphore, even in cases where it is technically a
        // temporary import
        semaphore_state->EnqueueAcquire(command);
    }

    // Mark the image as acquired.
    auto swapchain_data = Get<SWAPCHAIN_NODE>(swapchain);
    if (swapchain_data) {
        swapchain_data->AcquireImage(*pImageIndex);
    }
}

void ValidationStateTracker::PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                               VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex,
                                                               const RecordObject &record_obj) {
    if ((VK_SUCCESS != record_obj.result) && (VK_SUBOPTIMAL_KHR != record_obj.result)) return;
    RecordAcquireNextImageState(device, swapchain, timeout, semaphore, fence, pImageIndex, record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                                uint32_t *pImageIndex, const RecordObject &record_obj) {
    if ((VK_SUCCESS != record_obj.result) && (VK_SUBOPTIMAL_KHR != record_obj.result)) return;
    RecordAcquireNextImageState(device, pAcquireInfo->swapchain, pAcquireInfo->timeout, pAcquireInfo->semaphore,
                                pAcquireInfo->fence, pImageIndex, record_obj.location.function);
}

std::shared_ptr<PHYSICAL_DEVICE_STATE> ValidationStateTracker::CreatePhysicalDeviceState(VkPhysicalDevice phys_dev) {
    return std::make_shared<PHYSICAL_DEVICE_STATE>(phys_dev);
}

void ValidationStateTracker::PostCallRecordCreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                                                          const VkAllocationCallbacks *pAllocator, VkInstance *pInstance,
                                                          const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) {
        return;
    }
    instance_state = this;
    uint32_t count = 0;
    // this can fail if the allocator fails
    VkResult result = DispatchEnumeratePhysicalDevices(*pInstance, &count, nullptr);
    if (result != VK_SUCCESS) {
        return;
    }
    std::vector<VkPhysicalDevice> physdev_handles(count);
    result = DispatchEnumeratePhysicalDevices(*pInstance, &count, physdev_handles.data());
    if (result != VK_SUCCESS) {
        return;
    }

    for (auto physdev : physdev_handles) {
        Add(CreatePhysicalDeviceState(physdev));
    }

#ifdef VK_USE_PLATFORM_METAL_EXT
    auto export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(pCreateInfo->pNext);
    while (export_metal_object_info) {
        export_metal_flags.push_back(export_metal_object_info->exportObjectType);
        export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
#endif  // VK_USE_PLATFORM_METAL_EXT
}

// Common function to update state for GetPhysicalDeviceQueueFamilyProperties & 2KHR version
static void StateUpdateCommonGetPhysicalDeviceQueueFamilyProperties(PHYSICAL_DEVICE_STATE *pd_state, uint32_t count) {
    pd_state->queue_family_known_count = std::max(pd_state->queue_family_known_count, count);
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                                  uint32_t *pQueueFamilyPropertyCount,
                                                                                  VkQueueFamilyProperties *pQueueFamilyProperties,
                                                                                  const RecordObject &record_obj) {
    auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
    assert(pd_state);
    StateUpdateCommonGetPhysicalDeviceQueueFamilyProperties(pd_state.get(), *pQueueFamilyPropertyCount);
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                                   uint32_t *pQueueFamilyPropertyCount,
                                                                                   VkQueueFamilyProperties2 *pQueueFamilyProperties,
                                                                                   const RecordObject &record_obj) {
    auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
    assert(pd_state);
    StateUpdateCommonGetPhysicalDeviceQueueFamilyProperties(pd_state.get(), *pQueueFamilyPropertyCount);
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties2 *pQueueFamilyProperties,
    const RecordObject &record_obj) {
    auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
    assert(pd_state);
    StateUpdateCommonGetPhysicalDeviceQueueFamilyProperties(pd_state.get(), *pQueueFamilyPropertyCount);
}
void ValidationStateTracker::PreCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                                            const VkAllocationCallbacks *pAllocator) {
    Destroy<SURFACE_STATE>(surface);
}

void ValidationStateTracker::RecordVulkanSurface(VkSurfaceKHR *pSurface) { Add(std::make_shared<SURFACE_STATE>(*pSurface)); }

void ValidationStateTracker::PostCallRecordCreateDisplayPlaneSurfaceKHR(VkInstance instance,
                                                                        const VkDisplaySurfaceCreateInfoKHR *pCreateInfo,
                                                                        const VkAllocationCallbacks *pAllocator,
                                                                        VkSurfaceKHR *pSurface, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
void ValidationStateTracker::PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance,
                                                                   const VkAndroidSurfaceCreateInfoKHR *pCreateInfo,
                                                                   const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                                   const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_FUCHSIA
void ValidationStateTracker::PostCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance,
                                                                         const VkImagePipeSurfaceCreateInfoFUCHSIA *pCreateInfo,
                                                                         const VkAllocationCallbacks *pAllocator,
                                                                         VkSurfaceKHR *pSurface, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_IOS_MVK
void ValidationStateTracker::PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                               const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_IOS_MVK

#ifdef VK_USE_PLATFORM_MACOS_MVK
void ValidationStateTracker::PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance,
                                                                 const VkMacOSSurfaceCreateInfoMVK *pCreateInfo,
                                                                 const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                                 const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_MACOS_MVK

#ifdef VK_USE_PLATFORM_METAL_EXT
void ValidationStateTracker::PostCallRecordCreateMetalSurfaceEXT(VkInstance instance,
                                                                 const VkMetalSurfaceCreateInfoEXT *pCreateInfo,
                                                                 const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                                 const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_METAL_EXT

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
void ValidationStateTracker::PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance,
                                                                   const VkWaylandSurfaceCreateInfoKHR *pCreateInfo,
                                                                   const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                                   const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
void ValidationStateTracker::PostCallRecordCreateWin32SurfaceKHR(VkInstance instance,
                                                                 const VkWin32SurfaceCreateInfoKHR *pCreateInfo,
                                                                 const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                                 const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR
void ValidationStateTracker::PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                               const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_XLIB_KHR
void ValidationStateTracker::PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                                const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_SCREEN_QNX
void ValidationStateTracker::PostCallRecordCreateScreenSurfaceQNX(VkInstance instance,
                                                                  const VkScreenSurfaceCreateInfoQNX *pCreateInfo,
                                                                  const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                                  const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX

void ValidationStateTracker::PostCallRecordCreateHeadlessSurfaceEXT(VkInstance instance,
                                                                    const VkHeadlessSurfaceCreateInfoEXT *pCreateInfo,
                                                                    const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                                    const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordVulkanSurface(pSurface);
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                                                   VkSurfaceKHR surface,
                                                                                   VkSurfaceCapabilitiesKHR *pSurfaceCapabilities,
                                                                                   const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    auto surface_state = Get<SURFACE_STATE>(surface);
    VkSurfaceCapabilities2KHR caps2 = vku::InitStructHelper();
    caps2.surfaceCapabilities = *pSurfaceCapabilities;
    surface_state->SetCapabilities(physicalDevice, safe_VkSurfaceCapabilities2KHR(&caps2));
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
    VkSurfaceCapabilities2KHR *pSurfaceCapabilities, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;

    if (pSurfaceInfo->surface) {
        auto surface_state = Get<SURFACE_STATE>(pSurfaceInfo->surface);

        const VkSurfacePresentModeEXT *surface_present_mode = vku::FindStructInPNextChain<VkSurfacePresentModeEXT>(pSurfaceInfo->pNext);
        if ((!IsExtEnabled(device_extensions.vk_ext_surface_maintenance1)) || (!surface_present_mode)) {
            surface_state->SetCapabilities(physicalDevice, pSurfaceCapabilities);
        } else {
            const VkSurfacePresentScalingCapabilitiesEXT *present_scaling_caps =
                vku::FindStructInPNextChain<VkSurfacePresentScalingCapabilitiesEXT>(pSurfaceCapabilities->pNext);
            const VkSurfacePresentModeCompatibilityEXT *compatible_modes =
                vku::FindStructInPNextChain<VkSurfacePresentModeCompatibilityEXT>(pSurfaceCapabilities->pNext);

            if (compatible_modes && compatible_modes->pPresentModes) {
                surface_state->SetCompatibleModes(
                    physicalDevice, surface_present_mode->presentMode,
                    vvl::span<const VkPresentModeKHR>(compatible_modes->pPresentModes, compatible_modes->presentModeCount));
            }
            if (present_scaling_caps) {
                surface_state->SetPresentModeCapabilities(physicalDevice, surface_present_mode->presentMode,
                                                          pSurfaceCapabilities->surfaceCapabilities, *present_scaling_caps);
            }
        }
    } else if (IsExtEnabled(instance_extensions.vk_google_surfaceless_query) &&
               vku::FindStructInPNextChain<VkSurfaceProtectedCapabilitiesKHR>(pSurfaceCapabilities->pNext)) {
        auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
        assert(pd_state);
        pd_state->surfaceless_query_state.capabilities = safe_VkSurfaceCapabilities2KHR(pSurfaceCapabilities);
    }
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice,
                                                                                    VkSurfaceKHR surface,
                                                                                    VkSurfaceCapabilities2EXT *pSurfaceCapabilities,
                                                                                    const RecordObject &record_obj) {
    auto surface_state = Get<SURFACE_STATE>(surface);
    const VkSurfaceCapabilitiesKHR caps{
        pSurfaceCapabilities->minImageCount,           pSurfaceCapabilities->maxImageCount,
        pSurfaceCapabilities->currentExtent,           pSurfaceCapabilities->minImageExtent,
        pSurfaceCapabilities->maxImageExtent,          pSurfaceCapabilities->maxImageArrayLayers,
        pSurfaceCapabilities->supportedTransforms,     pSurfaceCapabilities->currentTransform,
        pSurfaceCapabilities->supportedCompositeAlpha, pSurfaceCapabilities->supportedUsageFlags,
    };
    VkSurfaceCapabilities2KHR caps2 = vku::InitStructHelper();
    caps2.surfaceCapabilities = caps;
    surface_state->SetCapabilities(physicalDevice, safe_VkSurfaceCapabilities2KHR(&caps2));
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice,
                                                                              uint32_t queueFamilyIndex, VkSurfaceKHR surface,
                                                                              VkBool32 *pSupported,
                                                                              const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    auto surface_state = Get<SURFACE_STATE>(surface);
    surface_state->SetQueueSupport(physicalDevice, queueFamilyIndex, (*pSupported == VK_TRUE));
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice,
                                                                                   VkSurfaceKHR surface,
                                                                                   uint32_t *pPresentModeCount,
                                                                                   VkPresentModeKHR *pPresentModes,
                                                                                   const RecordObject &record_obj) {
    if ((VK_SUCCESS != record_obj.result) && (VK_INCOMPLETE != record_obj.result)) return;

    if (pPresentModes) {
        if (surface) {
            auto surface_state = Get<SURFACE_STATE>(surface);
            surface_state->SetPresentModes(physicalDevice, vvl::span<const VkPresentModeKHR>(pPresentModes, *pPresentModeCount));
        } else if (IsExtEnabled(instance_extensions.vk_google_surfaceless_query)) {
            auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
            assert(pd_state);
            pd_state->surfaceless_query_state.present_modes =
                std::vector<VkPresentModeKHR>(pPresentModes, pPresentModes + *pPresentModeCount);
        }
    }
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                              uint32_t *pSurfaceFormatCount,
                                                                              VkSurfaceFormatKHR *pSurfaceFormats,
                                                                              const RecordObject &record_obj) {
    if ((VK_SUCCESS != record_obj.result) && (VK_INCOMPLETE != record_obj.result)) return;

    if (pSurfaceFormats) {
        std::vector<safe_VkSurfaceFormat2KHR> formats2(*pSurfaceFormatCount);
        for (uint32_t surface_format_index = 0; surface_format_index < *pSurfaceFormatCount; surface_format_index++) {
            formats2[surface_format_index].surfaceFormat = pSurfaceFormats[surface_format_index];
        }
        if (surface) {
            auto surface_state = Get<SURFACE_STATE>(surface);
            surface_state->SetFormats(physicalDevice, std::move(formats2));
        } else if (IsExtEnabled(instance_extensions.vk_google_surfaceless_query)) {
            auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
            assert(pd_state);
            pd_state->surfaceless_query_state.formats = std::move(formats2);
        }
    }
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                               const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                               uint32_t *pSurfaceFormatCount,
                                                                               VkSurfaceFormat2KHR *pSurfaceFormats,
                                                                               const RecordObject &record_obj) {
    if ((VK_SUCCESS != record_obj.result) && (VK_INCOMPLETE != record_obj.result)) return;

    if (pSurfaceFormats) {
        if (pSurfaceInfo->surface) {
            auto surface_state = Get<SURFACE_STATE>(pSurfaceInfo->surface);
            std::vector<safe_VkSurfaceFormat2KHR> formats2(*pSurfaceFormatCount);
            for (uint32_t surface_format_index = 0; surface_format_index < *pSurfaceFormatCount; surface_format_index++) {
                formats2[surface_format_index].initialize(&pSurfaceFormats[surface_format_index]);
            }
            surface_state->SetFormats(physicalDevice, std::move(formats2));
        } else if (IsExtEnabled(instance_extensions.vk_google_surfaceless_query)) {
            auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
            assert(pd_state);
            pd_state->surfaceless_query_state.formats.clear();
            pd_state->surfaceless_query_state.formats.reserve(*pSurfaceFormatCount);
            for (uint32_t surface_format_index = 0; surface_format_index < *pSurfaceFormatCount; ++surface_format_index) {
                pd_state->surfaceless_query_state.formats.emplace_back(
                    safe_VkSurfaceFormat2KHR(&pSurfaceFormats[surface_format_index]));
            }
        }
    }
}

void ValidationStateTracker::PreCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                                                                     const VkDebugUtilsLabelEXT *pLabelInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(Func::vkCmdBeginDebugUtilsLabelEXT);
    BeginCmdDebugUtilsLabel(report_data, commandBuffer, pLabelInfo);
}

void ValidationStateTracker::PostCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(record_obj.location.function);
    EndCmdDebugUtilsLabel(report_data, commandBuffer);
}

void ValidationStateTracker::PreCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                                                                      const VkDebugUtilsLabelEXT *pLabelInfo) {
    InsertCmdDebugUtilsLabel(report_data, commandBuffer, pLabelInfo);

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(Func::vkCmdInsertDebugUtilsLabelEXT);
    // Squirrel away an easily accessible copy.
    cb_state->debug_label = LoggingLabel(pLabelInfo);
}

void ValidationStateTracker::RecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCounters(VkPhysicalDevice physicalDevice,
                                                                                              uint32_t queueFamilyIndex,
                                                                                              uint32_t *pCounterCount,
                                                                                              VkPerformanceCounterKHR *pCounters) {
    if (NULL == pCounters) return;

    auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
    assert(pd_state);

    std::unique_ptr<QUEUE_FAMILY_PERF_COUNTERS> queue_family_counters(new QUEUE_FAMILY_PERF_COUNTERS());
    queue_family_counters->counters.resize(*pCounterCount);
    for (uint32_t i = 0; i < *pCounterCount; i++) queue_family_counters->counters[i] = pCounters[i];

    pd_state->perf_counters[queueFamilyIndex] = std::move(queue_family_counters);
}

void ValidationStateTracker::PostCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t *pCounterCount, VkPerformanceCounterKHR *pCounters,
    VkPerformanceCounterDescriptionKHR *pCounterDescriptions, const RecordObject &record_obj) {
    if ((VK_SUCCESS != record_obj.result) && (VK_INCOMPLETE != record_obj.result)) return;
    RecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCounters(physicalDevice, queueFamilyIndex, pCounterCount, pCounters);
}

void ValidationStateTracker::PostCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR *pInfo,
                                                                   const RecordObject &record_obj) {
    if (record_obj.result == VK_SUCCESS) performance_lock_acquired = true;
}

void ValidationStateTracker::PostCallRecordReleaseProfilingLockKHR(VkDevice device, const RecordObject &record_obj) {
    performance_lock_acquired = false;
    for (auto &cmd_buffer : command_buffer_map_.snapshot()) {
        cmd_buffer.second->performance_lock_released = true;
    }
}

void ValidationStateTracker::PreCallRecordDestroyDescriptorUpdateTemplate(VkDevice device,
                                                                          VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                          const VkAllocationCallbacks *pAllocator) {
    Destroy<UPDATE_TEMPLATE_STATE>(descriptorUpdateTemplate);
}

void ValidationStateTracker::PreCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device,
                                                                             VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                             const VkAllocationCallbacks *pAllocator) {
    Destroy<UPDATE_TEMPLATE_STATE>(descriptorUpdateTemplate);
}

void ValidationStateTracker::RecordCreateDescriptorUpdateTemplateState(const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
                                                                       VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate) {
    Add(std::make_shared<UPDATE_TEMPLATE_STATE>(*pDescriptorUpdateTemplate, pCreateInfo));
}

void ValidationStateTracker::PostCallRecordCreateDescriptorUpdateTemplate(VkDevice device,
                                                                          const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
                                                                          const VkAllocationCallbacks *pAllocator,
                                                                          VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate,
                                                                          const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordCreateDescriptorUpdateTemplateState(pCreateInfo, pDescriptorUpdateTemplate);
}

void ValidationStateTracker::PostCallRecordCreateDescriptorUpdateTemplateKHR(
    VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
    VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate, const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordCreateDescriptorUpdateTemplateState(pCreateInfo, pDescriptorUpdateTemplate);
}

void ValidationStateTracker::RecordUpdateDescriptorSetWithTemplateState(VkDescriptorSet descriptorSet,
                                                                        VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                        const void *pData) {
    auto const template_state = Get<UPDATE_TEMPLATE_STATE>(descriptorUpdateTemplate);
    assert(template_state);
    if (template_state) {
        // TODO: Record template push descriptor updates
        if (template_state->create_info.templateType == VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET) {
            PerformUpdateDescriptorSetsWithTemplateKHR(descriptorSet, template_state.get(), pData);
        }
    }
}

void ValidationStateTracker::PreCallRecordUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet,
                                                                          VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                          const void *pData) {
    RecordUpdateDescriptorSetWithTemplateState(descriptorSet, descriptorUpdateTemplate, pData);
}

void ValidationStateTracker::PreCallRecordUpdateDescriptorSetWithTemplateKHR(VkDevice device, VkDescriptorSet descriptorSet,
                                                                             VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                             const void *pData) {
    RecordUpdateDescriptorSetWithTemplateState(descriptorSet, descriptorUpdateTemplate, pData);
}

void ValidationStateTracker::PreCallRecordCmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer,
                                                                              VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                              VkPipelineLayout layout, uint32_t set,
                                                                              const void *pData) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto template_state = Get<UPDATE_TEMPLATE_STATE>(descriptorUpdateTemplate);
    auto layout_data = Get<PIPELINE_LAYOUT_STATE>(layout);
    if (!cb_state || !template_state || !layout_data) {
        return;
    }

    cb_state->RecordCmd(Func::vkCmdPushDescriptorSetWithTemplateKHR);
    auto dsl = layout_data->GetDsl(set);
    const auto &template_ci = template_state->create_info;
    // Decode the template into a set of write updates
    cvdescriptorset::DecodedTemplateUpdate decoded_template(this, VK_NULL_HANDLE, template_state.get(), pData,
                                                            dsl->GetDescriptorSetLayout());
    cb_state->PushDescriptorSetState(template_ci.pipelineBindPoint, *layout_data, set,
                                     static_cast<uint32_t>(decoded_template.desc_writes.size()),
                                     decoded_template.desc_writes.data());
}

void ValidationStateTracker::RecordGetPhysicalDeviceDisplayPlanePropertiesState(VkPhysicalDevice physicalDevice,
                                                                                uint32_t *pPropertyCount, void *pProperties) {
    auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
    if (*pPropertyCount) {
        pd_state->display_plane_property_count = *pPropertyCount;
    }
    if (*pPropertyCount || pProperties) {
        pd_state->vkGetPhysicalDeviceDisplayPlanePropertiesKHR_called = true;
    }
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                                      uint32_t *pPropertyCount,
                                                                                      VkDisplayPlanePropertiesKHR *pProperties,
                                                                                      const RecordObject &record_obj) {
    if ((VK_SUCCESS != record_obj.result) && (VK_INCOMPLETE != record_obj.result)) return;
    RecordGetPhysicalDeviceDisplayPlanePropertiesState(physicalDevice, pPropertyCount, pProperties);
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                                       uint32_t *pPropertyCount,
                                                                                       VkDisplayPlaneProperties2KHR *pProperties,
                                                                                       const RecordObject &record_obj) {
    if ((VK_SUCCESS != record_obj.result) && (VK_INCOMPLETE != record_obj.result)) return;
    RecordGetPhysicalDeviceDisplayPlanePropertiesState(physicalDevice, pPropertyCount, pProperties);
}

void ValidationStateTracker::PostCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                                   uint32_t slot, VkQueryControlFlags flags, uint32_t index,
                                                                   const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    uint32_t num_queries = 1;
    // If render pass instance has multiview enabled, query uses N consecutive query indices
    if (cb_state->activeRenderPass) {
        uint32_t bits = cb_state->activeRenderPass->GetViewMaskBits(cb_state->GetActiveSubpass());
        num_queries = std::max(num_queries, bits);
    }

    for (uint32_t i = 0; i < num_queries; ++i) {
        QueryObject query_obj = {queryPool, slot, flags, 0, true, index + i};
        cb_state->RecordCmd(record_obj.location.function);
        cb_state->BeginQuery(query_obj);
    }
}

void ValidationStateTracker::PostCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                                 uint32_t slot, uint32_t index, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    uint32_t num_queries = 1;
    // If render pass instance has multiview enabled, query uses N consecutive query indices
    if (cb_state->activeRenderPass) {
        uint32_t bits = cb_state->activeRenderPass->GetViewMaskBits(cb_state->GetActiveSubpass());
        num_queries = std::max(num_queries, bits);
    }

    for (uint32_t i = 0; i < num_queries; ++i) {
        QueryObject query_obj = {queryPool, slot, 0, 0, true, index + i};
        cb_state->RecordCmd(record_obj.location.function);
        cb_state->EndQuery(query_obj);
    }
}

void ValidationStateTracker::RecordCreateSamplerYcbcrConversionState(const VkSamplerYcbcrConversionCreateInfo *create_info,
                                                                     VkSamplerYcbcrConversion ycbcr_conversion) {
    VkFormatFeatureFlags2KHR format_features = 0;

    if (create_info->format != VK_FORMAT_UNDEFINED) {
        format_features = GetPotentialFormatFeatures(create_info->format);
    } else if (IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)) {
        // If format is VK_FORMAT_UNDEFINED, format_features will be set by external AHB features
        format_features = GetExternalFormatFeaturesANDROID(create_info);
    }

    Add(std::make_shared<SAMPLER_YCBCR_CONVERSION_STATE>(ycbcr_conversion, create_info, format_features));
}

void ValidationStateTracker::PostCallRecordCreateSamplerYcbcrConversion(VkDevice device,
                                                                        const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                                        const VkAllocationCallbacks *pAllocator,
                                                                        VkSamplerYcbcrConversion *pYcbcrConversion,
                                                                        const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordCreateSamplerYcbcrConversionState(pCreateInfo, *pYcbcrConversion);
}

void ValidationStateTracker::PostCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                                           const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                                           const VkAllocationCallbacks *pAllocator,
                                                                           VkSamplerYcbcrConversion *pYcbcrConversion,
                                                                           const RecordObject &record_obj) {
    if (VK_SUCCESS != record_obj.result) return;
    RecordCreateSamplerYcbcrConversionState(pCreateInfo, *pYcbcrConversion);
}

void ValidationStateTracker::PostCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                                         const VkAllocationCallbacks *pAllocator,
                                                                         const RecordObject &record_obj) {
    Destroy<SAMPLER_YCBCR_CONVERSION_STATE>(ycbcrConversion);
}

void ValidationStateTracker::PostCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device,
                                                                            VkSamplerYcbcrConversion ycbcrConversion,
                                                                            const VkAllocationCallbacks *pAllocator,
                                                                            const RecordObject &record_obj) {
    Destroy<SAMPLER_YCBCR_CONVERSION_STATE>(ycbcrConversion);
}

void ValidationStateTracker::RecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                  uint32_t queryCount) {
    // Do nothing if the feature is not enabled.
    if (!enabled_features.hostQueryReset) return;

    // Do nothing if the query pool has been destroyed.
    auto query_pool_state = Get<QUERY_POOL_STATE>(queryPool);
    if (!query_pool_state) return;

    // Reset the state of existing entries.
    const uint32_t max_query_count = std::min(queryCount, query_pool_state->createInfo.queryCount - firstQuery);
    for (uint32_t i = 0; i < max_query_count; ++i) {
        auto query_index = firstQuery + i;
        query_pool_state->SetQueryState(query_index, 0, QUERYSTATE_RESET);
        if (query_pool_state->createInfo.queryType == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) {
            for (uint32_t pass_index = 0; pass_index < query_pool_state->n_performance_passes; pass_index++) {
                query_pool_state->SetQueryState(query_index, pass_index, QUERYSTATE_RESET);
            }
        }
    }
}

void ValidationStateTracker::PostCallRecordResetQueryPoolEXT(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                             uint32_t queryCount, const RecordObject &record_obj) {
    RecordResetQueryPool(device, queryPool, firstQuery, queryCount);
}

void ValidationStateTracker::PostCallRecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                          uint32_t queryCount, const RecordObject &record_obj) {
    RecordResetQueryPool(device, queryPool, firstQuery, queryCount);
}

void ValidationStateTracker::PerformUpdateDescriptorSetsWithTemplateKHR(VkDescriptorSet descriptorSet,
                                                                        const UPDATE_TEMPLATE_STATE *template_state,
                                                                        const void *pData) {
    // Translate the templated update into a normal update for validation...
    cvdescriptorset::DecodedTemplateUpdate decoded_update(this, descriptorSet, template_state, pData);
    PerformUpdateDescriptorSets(static_cast<uint32_t>(decoded_update.desc_writes.size()), decoded_update.desc_writes.data(), 0,
                                NULL);
}

// Update the common AllocateDescriptorSetsData
void ValidationStateTracker::UpdateAllocateDescriptorSetsData(const VkDescriptorSetAllocateInfo *p_alloc_info,
                                                              cvdescriptorset::AllocateDescriptorSetsData *ds_data) const {
    for (uint32_t i = 0; i < p_alloc_info->descriptorSetCount; i++) {
        auto layout = Get<cvdescriptorset::DescriptorSetLayout>(p_alloc_info->pSetLayouts[i]);
        if (layout) {
            ds_data->layout_nodes[i] = layout;
            // Count total descriptors required per type
            for (uint32_t j = 0; j < layout->GetBindingCount(); ++j) {
                const auto &binding_layout = layout->GetDescriptorSetLayoutBindingPtrFromIndex(j);
                uint32_t type_index = static_cast<uint32_t>(binding_layout->descriptorType);
                ds_data->required_descriptors_by_type[type_index] += binding_layout->descriptorCount;
            }
        }
        // Any unknown layouts will be flagged as errors during ValidateAllocateDescriptorSets() call
    }
}

void ValidationStateTracker::PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                                   uint32_t firstVertex, uint32_t firstInstance, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDrawCmd(record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                           const VkMultiDrawInfoEXT *pVertexInfo, uint32_t instanceCount,
                                                           uint32_t firstInstance, uint32_t stride,
                                                           const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDrawCmd(record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount,
                                                          uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                                                          uint32_t firstInstance, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDrawCmd(record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                                  const VkMultiDrawIndexedInfoEXT *pIndexInfo,
                                                                  uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                                                  const int32_t *pVertexOffset, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDrawCmd(record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           uint32_t count, uint32_t stride, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    cb_state->UpdateDrawCmd(record_obj.location.function);
    if (!disabled[command_buffer_state]) {
        cb_state->AddChild(buffer_state);
    }
}

void ValidationStateTracker::PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, uint32_t count, uint32_t stride,
                                                                  const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    cb_state->UpdateDrawCmd(record_obj.location.function);
    if (!disabled[command_buffer_state]) {
        cb_state->AddChild(buffer_state);
    }
}

void ValidationStateTracker::PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z,
                                                       const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDispatchCmd(record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDispatchCmd(record_obj.location.function);
    if (!disabled[command_buffer_state]) {
        auto buffer_state = Get<BUFFER_STATE>(buffer);
        cb_state->AddChild(buffer_state);
    }
}

void ValidationStateTracker::PostCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t, uint32_t, uint32_t, uint32_t,
                                                              uint32_t, uint32_t, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDispatchCmd(record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t, uint32_t, uint32_t, uint32_t,
                                                           uint32_t, uint32_t, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDispatchCmd(record_obj.location.function);
}

void ValidationStateTracker::RecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride, Func command) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDrawCmd(command);
    if (!disabled[command_buffer_state]) {
        auto buffer_state = Get<BUFFER_STATE>(buffer);
        auto count_buffer_state = Get<BUFFER_STATE>(countBuffer);
        cb_state->AddChild(buffer_state);
        cb_state->AddChild(count_buffer_state);
    }
}

void ValidationStateTracker::PreCallRecordCmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, VkBuffer countBuffer,
                                                                  VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                  uint32_t stride) {
    RecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                               Func::vkCmdDrawIndexedIndirectCountKHR);
}

void ValidationStateTracker::PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride) {
    RecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                               Func::vkCmdDrawIndexedIndirectCount);
}

void ValidationStateTracker::RecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride, Func command) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDrawCmd(command);
    if (!disabled[command_buffer_state]) {
        auto buffer_state = Get<BUFFER_STATE>(buffer);
        auto count_buffer_state = Get<BUFFER_STATE>(countBuffer);
        cb_state->AddChild(buffer_state);
        cb_state->AddChild(count_buffer_state);
    }
}

void ValidationStateTracker::PreCallRecordCmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                         VkDeviceSize offset, VkBuffer countBuffer,
                                                                         VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                         uint32_t stride) {
    RecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      Func::vkCmdDrawIndexedIndirectCountKHR);
}

void ValidationStateTracker::PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                      VkDeviceSize offset, VkBuffer countBuffer,
                                                                      VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                      uint32_t stride) {
    RecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      Func::vkCmdDrawIndexedIndirectCount);
}

void ValidationStateTracker::PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount,
                                                             uint32_t firstTask) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDrawCmd(Func::vkCmdDrawMeshTasksNV);
}

void ValidationStateTracker::PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                     VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDrawCmd(Func::vkCmdDrawMeshTasksIndirectNV);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    if (!disabled[command_buffer_state] && buffer_state) {
        cb_state->AddChild(buffer_state);
    }
}

void ValidationStateTracker::PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                          VkDeviceSize offset, VkBuffer countBuffer,
                                                                          VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                          uint32_t stride) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDrawCmd(Func::vkCmdDrawMeshTasksIndirectCountNV);
    if (!disabled[command_buffer_state]) {
        auto buffer_state = Get<BUFFER_STATE>(buffer);
        auto count_buffer_state = Get<BUFFER_STATE>(countBuffer);
        if (buffer_state) {
            cb_state->AddChild(buffer_state);
        }
        if (count_buffer_state) {
            cb_state->AddChild(count_buffer_state);
        }
    }
}

void ValidationStateTracker::PreCallRecordCmdDrawMeshTasksEXT(VkCommandBuffer commandBuffer, uint32_t groupCountX,
                                                              uint32_t groupCountY, uint32_t groupCountZ) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDrawCmd(Func::vkCmdDrawMeshTasksEXT);
}

void ValidationStateTracker::PreCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                      VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDrawCmd(Func::vkCmdDrawMeshTasksIndirectEXT);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    if (!disabled[command_buffer_state] && buffer_state) {
        cb_state->AddChild(buffer_state);
    }
}

void ValidationStateTracker::PreCallRecordCmdDrawMeshTasksIndirectCountEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                           VkDeviceSize offset, VkBuffer countBuffer,
                                                                           VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                           uint32_t stride) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDrawCmd(Func::vkCmdDrawMeshTasksIndirectCountEXT);
    if (!disabled[command_buffer_state]) {
        auto buffer_state = Get<BUFFER_STATE>(buffer);
        auto count_buffer_state = Get<BUFFER_STATE>(countBuffer);
        if (buffer_state) {
            cb_state->AddChild(buffer_state);
        }
        if (count_buffer_state) {
            cb_state->AddChild(count_buffer_state);
        }
    }
}

void ValidationStateTracker::PostCallRecordCmdTraceRaysNV(
    VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer, VkDeviceSize raygenShaderBindingOffset,
    VkBuffer missShaderBindingTableBuffer, VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
    VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset, VkDeviceSize hitShaderBindingStride,
    VkBuffer callableShaderBindingTableBuffer, VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
    uint32_t width, uint32_t height, uint32_t depth, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateTraceRayCmd(record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                           const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                           const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                           const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                           const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                           uint32_t width, uint32_t height, uint32_t depth,
                                                           const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateTraceRayCmd(record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdTraceRaysIndirectKHR(
    VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, VkDeviceAddress indirectDeviceAddress,
    const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateTraceRayCmd(record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer,
                                                                    VkDeviceAddress indirectDeviceAddress,
                                                                    const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateTraceRayCmd(record_obj.location.function);
}

void ValidationStateTracker::PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule,
                                                             void *csm_state_data) {
    create_shader_module_api_state *csm_state = static_cast<create_shader_module_api_state *>(csm_state_data);
    csm_state->module_state = std::make_shared<SPIRV_MODULE_STATE>(pCreateInfo->codeSize, pCreateInfo->pCode);
    if (csm_state->module_state && csm_state->module_state->static_data_.has_group_decoration) {
        spv_target_env spirv_environment = PickSpirvEnv(api_version, IsExtEnabled(device_extensions.vk_khr_spirv_1_4));
        spvtools::Optimizer optimizer(spirv_environment);
        optimizer.RegisterPass(spvtools::CreateFlattenDecorationPass());
        std::vector<uint32_t> optimized_binary;
        // Run optimizer to flatten decorations only, set skip_validation so as to not re-run validator
        auto result = optimizer.Run(csm_state->module_state->words_.data(), csm_state->module_state->words_.size(),
                                    &optimized_binary, spvtools::ValidatorOptions(), true);

        if (result) {
            // Easier to just re-create the ShaderModule as StaticData uses itself when building itself up
            // It is really rare this will get here as Group Decorations have been deprecated and before this was added no one ever
            // raised an issue for a bug that would crash the layers that was around for many releases
            csm_state->module_state =
                std::make_shared<SPIRV_MODULE_STATE>(optimized_binary.size() * sizeof(uint32_t), optimized_binary.data());
        }
    }
}

void ValidationStateTracker::PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                           const VkShaderCreateInfoEXT *pCreateInfos,
                                                           const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                                           void *csm_state_data) {
    create_shader_object_api_state *csm_state = static_cast<create_shader_object_api_state *>(csm_state_data);
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        // don't need to worry about GroupDecoration with VK_EXT_shader_object
        if (pCreateInfos[i].codeType == VK_SHADER_CODE_TYPE_SPIRV_EXT) {
            csm_state->module_states[i] = std::make_shared<SPIRV_MODULE_STATE>(
                pCreateInfos[i].codeSize, static_cast<const uint32_t *>(pCreateInfos[i].pCode));
        }
    }
}

void ValidationStateTracker::PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkShaderModule *pShaderModule, const RecordObject &record_obj,
                                                              void *csm_state_data) {
    if (VK_SUCCESS != record_obj.result) return;
    create_shader_module_api_state *csm_state = static_cast<create_shader_module_api_state *>(csm_state_data);
    Add(std::make_shared<SHADER_MODULE_STATE>(*pShaderModule, csm_state->module_state, csm_state->unique_shader_id));
}

void ValidationStateTracker::PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                            const VkShaderCreateInfoEXT *pCreateInfos,
                                                            const VkAllocationCallbacks *pAllocator, VkShaderEXT *pShaders,
                                                            const RecordObject &record_obj, void *csm_state_data) {
    if (VK_SUCCESS != record_obj.result) return;
    create_shader_object_api_state *csm_state = static_cast<create_shader_object_api_state *>(csm_state_data);
    for (uint32_t i = 0; i < createInfoCount; ++i) {
        if (pShaders[i] != VK_NULL_HANDLE) {
            Add(std::make_shared<SHADER_OBJECT_STATE>(this, pCreateInfos[i], pShaders[i], csm_state->module_states[i], createInfoCount, pShaders, csm_state->unique_shader_ids[i]));
        }
    }
}

void ValidationStateTracker::PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                                 uint32_t *pSwapchainImageCount, VkImage *pSwapchainImages,
                                                                 const RecordObject &record_obj) {
    if ((record_obj.result != VK_SUCCESS) && (record_obj.result != VK_INCOMPLETE)) return;
    auto swapchain_state = Get<SWAPCHAIN_NODE>(swapchain);

    if (*pSwapchainImageCount > swapchain_state->images.size()) swapchain_state->images.resize(*pSwapchainImageCount);

    if (pSwapchainImages) {
        for (uint32_t i = 0; i < *pSwapchainImageCount; ++i) {
            SWAPCHAIN_IMAGE &swapchain_image = swapchain_state->images[i];
            if (swapchain_image.image_state) continue;  // Already retrieved this.

            auto format_features = GetImageFormatFeatures(
                physical_device, has_format_feature2, IsExtEnabled(device_extensions.vk_ext_image_drm_format_modifier), device,
                pSwapchainImages[i], swapchain_state->image_create_info.format, swapchain_state->image_create_info.tiling);

            auto image_state =
                CreateImageState(pSwapchainImages[i], swapchain_state->image_create_info.ptr(), swapchain, i, format_features);

            image_state->SetSwapchain(swapchain_state, i);
            swapchain_image.image_state = image_state.get();
            Add(std::move(image_state));
        }
    }

    if (*pSwapchainImageCount) {
        swapchain_state->get_swapchain_image_count = *pSwapchainImageCount;
    }
}

void ValidationStateTracker::PostCallRecordCopyAccelerationStructureKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                        const VkCopyAccelerationStructureInfoKHR *pInfo,
                                                                        const RecordObject &record_obj) {
    auto src_as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfo->src);
    auto dst_as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfo->dst);
    if (dst_as_state != nullptr && src_as_state != nullptr) {
        dst_as_state->built = true;
        dst_as_state->build_info_khr = src_as_state->build_info_khr;
    }
}

void ValidationStateTracker::PostCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                           const VkCopyAccelerationStructureInfoKHR *pInfo,
                                                                           const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_state) {
        cb_state->RecordCmd(record_obj.location.function);
        auto src_as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfo->src);
        auto dst_as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfo->dst);
        if (dst_as_state != nullptr && src_as_state != nullptr) {
            dst_as_state->built = true;
            dst_as_state->build_info_khr = src_as_state->build_info_khr;
            if (!disabled[command_buffer_state]) {
                cb_state->AddChild(dst_as_state);
                cb_state->AddChild(src_as_state);
            }
        }
    }
}

void ValidationStateTracker::PostCallRecordCmdCopyAccelerationStructureToMemoryKHR(
    VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_state) {
        cb_state->RecordCmd(record_obj.location.function);
        auto src_as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfo->src);
        if (!disabled[command_buffer_state]) {
            cb_state->AddChild(src_as_state);
        }
        // Issue https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6461
        // showed that it is incorrect to try to add buffers obtained through a call to GetBuffersByAddress as children to a command
        // buffer
    }
}

void ValidationStateTracker::PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_state) {
        cb_state->RecordCmd(record_obj.location.function);
        if (!disabled[command_buffer_state]) {
            auto dst_as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfo->dst);
            cb_state->AddChild(dst_as_state);

            // Issue https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6461
            // showed that it is incorrect to try to add buffers obtained through a call to GetBuffersByAddress as children to a
            // command buffer
        }
    }
}

void ValidationStateTracker::RecordCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode, Func command) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(command, CB_DYNAMIC_STATE_CULL_MODE);
    cb_state->dynamic_state_value.cull_mode = cullMode;
}

void ValidationStateTracker::PostCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                                             const RecordObject &record_obj) {
    RecordCmdSetCullMode(commandBuffer, cullMode, record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode,
                                                          const RecordObject &record_obj) {
    RecordCmdSetCullMode(commandBuffer, cullMode, record_obj.location.function);
}

void ValidationStateTracker::RecordCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace, Func command) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(command, CB_DYNAMIC_STATE_FRONT_FACE);
}

void ValidationStateTracker::PostCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                                              const RecordObject &record_obj) {
    RecordCmdSetFrontFace(commandBuffer, frontFace, record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace,
                                                           const RecordObject &record_obj) {
    RecordCmdSetFrontFace(commandBuffer, frontFace, record_obj.location.function);
}

void ValidationStateTracker::RecordCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                           Func command) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(command, CB_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY);
    cb_state->dynamic_state_value.primitive_topology = primitiveTopology;
}

void ValidationStateTracker::PostCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer,
                                                                      VkPrimitiveTopology primitiveTopology,
                                                                      const RecordObject &record_obj) {
    RecordCmdSetPrimitiveTopology(commandBuffer, primitiveTopology, record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer,
                                                                   VkPrimitiveTopology primitiveTopology,
                                                                   const RecordObject &record_obj) {
    RecordCmdSetPrimitiveTopology(commandBuffer, primitiveTopology, record_obj.location.function);
}

void ValidationStateTracker::RecordCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                           const VkViewport *pViewports, Func command) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(command, CB_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
    uint32_t bits = (1u << viewportCount) - 1u;
    cb_state->viewportWithCountMask |= bits;
    cb_state->trashedViewportMask &= ~bits;
    cb_state->dynamic_state_value.viewport_count = viewportCount;
    cb_state->trashedViewportCount = false;

    cb_state->dynamic_state_value.viewports.resize(viewportCount);
    for (size_t i = 0; i < viewportCount; ++i) {
        cb_state->dynamic_state_value.viewports[i] = pViewports[i];
    }
}

void ValidationStateTracker::PostCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                                      const VkViewport *pViewports,
                                                                      const RecordObject &record_obj) {
    RecordCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports, record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                                   const VkViewport *pViewports, const RecordObject &record_obj) {
    RecordCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports, record_obj.location.function);
}

void ValidationStateTracker::RecordCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                          const VkRect2D *pScissors, Func command) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(command, CB_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
    uint32_t bits = (1u << scissorCount) - 1u;
    cb_state->scissorWithCountMask |= bits;
    cb_state->trashedScissorMask &= ~bits;
    cb_state->dynamic_state_value.scissor_count = scissorCount;
    cb_state->trashedScissorCount = false;
}

void ValidationStateTracker::PostCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                                     const VkRect2D *pScissors, const RecordObject &record_obj) {
    RecordCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors, record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                                  const VkRect2D *pScissors, const RecordObject &record_obj) {
    RecordCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors, record_obj.location.function);
}

void ValidationStateTracker::RecordCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                         uint32_t bindingCount, const VkBuffer *pBuffers,
                                                         const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                         const VkDeviceSize *pStrides, Func command) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (pStrides) {
        cb_state->RecordStateCmd(command, CB_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
    }

    uint32_t end = firstBinding + bindingCount;
    if (cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings.size() < end) {
        cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings.resize(end);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        auto &vertex_buffer_binding = cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings[i + firstBinding];
        const VkDeviceSize binding_size = (pSizes) ? pSizes[i] : VK_WHOLE_SIZE;
        const VkDeviceSize binding_stride = (pStrides) ? pStrides[i] : 0;
        vertex_buffer_binding = BufferBinding(Get<BUFFER_STATE>(pBuffers[i]), binding_size, pOffsets[i], binding_stride);

        // Add binding for this vertex buffer to this commandbuffer
        if (!disabled[command_buffer_state] && pBuffers[i]) {
            cb_state->AddChild(vertex_buffer_binding.buffer_state);
        }
    }
}

void ValidationStateTracker::PostCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                    uint32_t bindingCount, const VkBuffer *pBuffers,
                                                                    const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                                    const VkDeviceSize *pStrides, const RecordObject &record_obj) {
    RecordCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides,
                                record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                 uint32_t bindingCount, const VkBuffer *pBuffers,
                                                                 const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                                 const VkDeviceSize *pStrides, const RecordObject &record_obj) {
    RecordCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides,
                                record_obj.location.function);
}

void ValidationStateTracker::RecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable, Func command) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(command, CB_DYNAMIC_STATE_DEPTH_TEST_ENABLE);
    cb_state->dynamic_state_value.depth_test_enable = depthTestEnable;
}

void ValidationStateTracker::PostCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                                    const RecordObject &record_obj) {
    RecordCmdSetDepthTestEnable(commandBuffer, depthTestEnable, record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                                 const RecordObject &record_obj) {
    RecordCmdSetDepthTestEnable(commandBuffer, depthTestEnable, record_obj.location.function);
}

void ValidationStateTracker::RecordCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable, Func command) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(command, CB_DYNAMIC_STATE_DEPTH_WRITE_ENABLE);
    cb_state->dynamic_state_value.depth_write_enable = depthWriteEnable;
}

void ValidationStateTracker::PostCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                                     const RecordObject &record_obj) {
    RecordCmdSetDepthWriteEnable(commandBuffer, depthWriteEnable, record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                                  const RecordObject &record_obj) {
    RecordCmdSetDepthWriteEnable(commandBuffer, depthWriteEnable, record_obj.location.function);
}

void ValidationStateTracker::RecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp, Func command) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(command, CB_DYNAMIC_STATE_DEPTH_COMPARE_OP);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                                   const RecordObject &record_obj) {
    RecordCmdSetDepthCompareOp(commandBuffer, depthCompareOp, record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                                const RecordObject &record_obj) {
    RecordCmdSetDepthCompareOp(commandBuffer, depthCompareOp, record_obj.location.function);
}

void ValidationStateTracker::RecordCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                               Func command) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(command, CB_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE);
    cb_state->dynamic_state_value.depth_bounds_test_enable = depthBoundsTestEnable;
}

void ValidationStateTracker::PostCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer,
                                                                          VkBool32 depthBoundsTestEnable,
                                                                          const RecordObject &record_obj) {
    RecordCmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable, record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer,
                                                                       VkBool32 depthBoundsTestEnable,
                                                                       const RecordObject &record_obj) {
    RecordCmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable, record_obj.location.function);
}

void ValidationStateTracker::RecordCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                           Func command) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(command, CB_DYNAMIC_STATE_STENCIL_TEST_ENABLE);
    cb_state->dynamic_state_value.stencil_test_enable = stencilTestEnable;
}

void ValidationStateTracker::PostCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                                      const RecordObject &record_obj) {
    RecordCmdSetStencilTestEnable(commandBuffer, stencilTestEnable, record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                                   const RecordObject &record_obj) {
    RecordCmdSetStencilTestEnable(commandBuffer, stencilTestEnable, record_obj.location.function);
}

void ValidationStateTracker::RecordCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                                   VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                                   Func command) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(command, CB_DYNAMIC_STATE_STENCIL_OP);
    if (faceMask == VK_STENCIL_FACE_FRONT_BIT || faceMask == VK_STENCIL_FACE_FRONT_AND_BACK) {
        cb_state->dynamic_state_value.fail_op_front = failOp;
        cb_state->dynamic_state_value.pass_op_front = passOp;
        cb_state->dynamic_state_value.depth_fail_op_front = depthFailOp;
    }
    if (faceMask == VK_STENCIL_FACE_BACK_BIT || faceMask == VK_STENCIL_FACE_FRONT_AND_BACK) {
        cb_state->dynamic_state_value.fail_op_back = failOp;
        cb_state->dynamic_state_value.pass_op_back = passOp;
        cb_state->dynamic_state_value.depth_fail_op_back = depthFailOp;
    }
}

void ValidationStateTracker::PostCallRecordCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                              VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp,
                                                              VkCompareOp compareOp, const RecordObject &record_obj) {
    RecordCmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp, record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                           VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp,
                                                           VkCompareOp compareOp, const RecordObject &record_obj) {
    RecordCmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp, record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                                     uint32_t discardRectangleCount,
                                                                     const VkRect2D *pDiscardRectangles,
                                                                     const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT);
    for (uint32_t i = 0; i < discardRectangleCount; i++) {
        cb_state->dynamic_state_value.discard_rectangles.set(firstDiscardRectangle + i);
    }
}

void ValidationStateTracker::PostCallRecordCmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer,
                                                                           VkBool32 discardRectangleEnable,
                                                                           const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT);
    cb_state->dynamic_state_value.discard_rectangle_enable = discardRectangleEnable;
}

void ValidationStateTracker::PostCallRecordCmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer,
                                                                         VkDiscardRectangleModeEXT discardRectangleMode,
                                                                         const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT);
}

void ValidationStateTracker::PostCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                                    const VkSampleLocationsInfoEXT *pSampleLocationsInfo,
                                                                    const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT);
    cb_state->dynamic_state_value.sample_locations_info = *pSampleLocationsInfo;
}

void ValidationStateTracker::PostCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer,
                                                                     VkCoarseSampleOrderTypeNV sampleOrderType,
                                                                     uint32_t customSampleOrderCount,
                                                                     const VkCoarseSampleOrderCustomNV *pCustomSampleOrders,
                                                                     const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV);
}

void ValidationStateTracker::PostCallRecordCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints,
                                                                       const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT);
}

void ValidationStateTracker::PostCallRecordCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp,
                                                            const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_LOGIC_OP_EXT);
}

void ValidationStateTracker::RecordCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                                 Func command) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(command, CB_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE);
    cb_state->dynamic_state_value.rasterizer_discard_enable = (rasterizerDiscardEnable == VK_TRUE);
}

void ValidationStateTracker::PostCallRecordCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer,
                                                                            VkBool32 rasterizerDiscardEnable,
                                                                            const RecordObject &record_obj) {
    RecordCmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable, record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer,
                                                                         VkBool32 rasterizerDiscardEnable,
                                                                         const RecordObject &record_obj) {
    RecordCmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable, record_obj.location.function);
}

void ValidationStateTracker::RecordCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable, Func command) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(command, CB_DYNAMIC_STATE_DEPTH_BIAS_ENABLE);
    cb_state->dynamic_state_value.depth_bias_enable = depthBiasEnable;
}

void ValidationStateTracker::PostCallRecordCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                                    const RecordObject &record_obj) {
    RecordCmdSetDepthBiasEnable(commandBuffer, depthBiasEnable, record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                                 const RecordObject &record_obj) {
    RecordCmdSetDepthBiasEnable(commandBuffer, depthBiasEnable, record_obj.location.function);
}

void ValidationStateTracker::RecordCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                                Func command) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(command, CB_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE);
}

void ValidationStateTracker::PostCallRecordCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer,
                                                                           VkBool32 primitiveRestartEnable,
                                                                           const RecordObject &record_obj) {
    RecordCmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable, record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer,
                                                                        VkBool32 primitiveRestartEnable,
                                                                        const RecordObject &record_obj) {
    RecordCmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable, record_obj.location.function);
}

void ValidationStateTracker::PostCallRecordCmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer,
                                                                        const VkExtent2D *pFragmentSize,
                                                                        const VkFragmentShadingRateCombinerOpKHR combinerOps[2],
                                                                        const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR);
    cb_state->dynamic_state_value.fragment_size = *pFragmentSize;
}

void ValidationStateTracker::PostCallRecordCmdSetVertexInputEXT(
    VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
    const VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount,
    const VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    CBDynamicFlags status_flags;
    status_flags.set(CB_DYNAMIC_STATE_VERTEX_INPUT_EXT);

    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    const auto pipeline_state = cb_state->lastBound[lv_bind_point].pipeline_state;
    if (pipeline_state && pipeline_state->IsDynamic(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT)) {
        status_flags.set(CB_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
    }
    cb_state->RecordStateCmd(record_obj.location.function, status_flags);
    cb_state->dynamic_state_value.vertex_attribute_descriptions.resize(vertexAttributeDescriptionCount);
    for (uint32_t i = 0; i < vertexAttributeDescriptionCount; ++i) {
        cb_state->dynamic_state_value.vertex_attribute_descriptions[i] = pVertexAttributeDescriptions[i];
    }
}

void ValidationStateTracker::PostCallRecordCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                                     const VkBool32 *pColorWriteEnables,
                                                                     const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT);
    cb_state->dynamic_state_value.color_write_enable_attachment_count = attachmentCount;
    for (uint32_t i = 0; i < attachmentCount; ++i) {
        if (pColorWriteEnables[i]) {
            cb_state->dynamic_state_value.color_write_enabled.set(i);
        } else {
            cb_state->dynamic_state_value.color_write_enabled.reset(i);
        }
    }
}

void ValidationStateTracker::PostCallRecordCmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer,
                                                                                 VkImageAspectFlags aspectMask,
                                                                                 const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void ValidationStateTracker::PostCallRecordAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                                             const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) return;

    auto swapchain_state = Get<SWAPCHAIN_NODE>(swapchain);
    swapchain_state->exclusive_full_screen_access = true;
}

void ValidationStateTracker::PostCallRecordReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                                             const RecordObject &record_obj) {
    if (record_obj.result != VK_SUCCESS) return;

    auto swapchain_state = Get<SWAPCHAIN_NODE>(swapchain);
    swapchain_state->exclusive_full_screen_access = false;
}
#endif

void ValidationStateTracker::PostCallRecordCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer,
                                                                             VkTessellationDomainOrigin domainOrigin,
                                                                             const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable,
                                                                     const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT);
}

void ValidationStateTracker::PostCallRecordCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode,
                                                                const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_POLYGON_MODE_EXT);
    cb_state->dynamic_state_value.polygon_mode = polygonMode;
}

void ValidationStateTracker::PostCallRecordCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer,
                                                                         VkSampleCountFlagBits rasterizationSamples,
                                                                         const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT);
    cb_state->dynamic_state_value.rasterization_samples = rasterizationSamples;
}

void ValidationStateTracker::PostCallRecordCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                                               const VkSampleMask *pSampleMask, const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_SAMPLE_MASK_EXT);
}

void ValidationStateTracker::PostCallRecordCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer,
                                                                          VkBool32 alphaToCoverageEnable,
                                                                          const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT);
    cb_state->dynamic_state_value.alpha_to_coverage_enable = alphaToCoverageEnable;
}

void ValidationStateTracker::PostCallRecordCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable,
                                                                     const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT);
}

void ValidationStateTracker::PostCallRecordCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable,
                                                                  const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT);
    cb_state->dynamic_state_value.logic_op_enable = logicOpEnable;
}

void ValidationStateTracker::PostCallRecordCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                                     uint32_t attachmentCount, const VkBool32 *pColorBlendEnables,
                                                                     const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT);
    for (uint32_t i = 0; i < attachmentCount; i++) {
        cb_state->dynamic_state_value.color_blend_enable_attachments.set(firstAttachment + i);
        if (pColorBlendEnables[i]) {
            cb_state->dynamic_state_value.color_blend_enabled.set(firstAttachment + i);
        } else {
            cb_state->dynamic_state_value.color_blend_enabled.reset(firstAttachment + i);
        }
    }
}

void ValidationStateTracker::PostCallRecordCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                                       uint32_t attachmentCount,
                                                                       const VkColorBlendEquationEXT *pColorBlendEquations,
                                                                       const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT);
    if (cb_state->dynamic_state_value.color_blend_equations.size() < firstAttachment + attachmentCount) {
        cb_state->dynamic_state_value.color_blend_equations.resize(firstAttachment + attachmentCount);
    }
    for (uint32_t i = 0; i < attachmentCount; i++) {
        cb_state->dynamic_state_value.color_blend_equation_attachments.set(firstAttachment + i);
        cb_state->dynamic_state_value.color_blend_equations[firstAttachment + i] = pColorBlendEquations[i];
    }
}

void ValidationStateTracker::PostCallRecordCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                                   uint32_t attachmentCount,
                                                                   const VkColorComponentFlags *pColorWriteMasks,
                                                                   const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT);
    if (cb_state->dynamic_state_value.color_write_masks.size() < firstAttachment + attachmentCount) {
        cb_state->dynamic_state_value.color_write_masks.resize(firstAttachment + attachmentCount);
    }
    for (uint32_t i = 0; i < attachmentCount; i++) {
        cb_state->dynamic_state_value.color_write_mask_attachments.set(firstAttachment + i);
        cb_state->dynamic_state_value.color_write_masks[i] = pColorWriteMasks[i];
    }
}

void ValidationStateTracker::PostCallRecordCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer, uint32_t rasterizationStream,
                                                                        const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT);
}

void ValidationStateTracker::PostCallRecordCmdSetConservativeRasterizationModeEXT(
    VkCommandBuffer commandBuffer, VkConservativeRasterizationModeEXT conservativeRasterizationMode,
    const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT);
    cb_state->dynamic_state_value.conservative_rasterization_mode = conservativeRasterizationMode;
}

void ValidationStateTracker::PostCallRecordCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                                                     float extraPrimitiveOverestimationSize,
                                                                                     const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable,
                                                                    const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT);
}

void ValidationStateTracker::PostCallRecordCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer,
                                                                          VkBool32 sampleLocationsEnable,
                                                                          const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT);
    cb_state->dynamic_state_value.sample_locations_enable = sampleLocationsEnable;
}

void ValidationStateTracker::PostCallRecordCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                                       uint32_t attachmentCount,
                                                                       const VkColorBlendAdvancedEXT *pColorBlendAdvanced,
                                                                       const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT);
    for (uint32_t i = 0; i < attachmentCount; i++) {
        cb_state->dynamic_state_value.color_blend_advanced_attachments.set(firstAttachment + i);
    }
}

void ValidationStateTracker::PostCallRecordCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer,
                                                                        VkProvokingVertexModeEXT provokingVertexMode,
                                                                        const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT);
}

void ValidationStateTracker::PostCallRecordCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                                          VkLineRasterizationModeEXT lineRasterizationMode,
                                                                          const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT);
    cb_state->dynamic_state_value.line_rasterization_mode = lineRasterizationMode;
}

void ValidationStateTracker::PostCallRecordCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable,
                                                                      const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT);
    cb_state->dynamic_state_value.stippled_line_enable = stippledLineEnable;
}

void ValidationStateTracker::PostCallRecordCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer,
                                                                              VkBool32 negativeOneToOne,
                                                                              const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT);
}

void ValidationStateTracker::PostCallRecordCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer,
                                                                          VkBool32 viewportWScalingEnable,
                                                                          const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV);
    cb_state->dynamic_state_value.viewport_w_scaling_enable = viewportWScalingEnable;
}

void ValidationStateTracker::PostCallRecordCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                   uint32_t viewportCount,
                                                                   const VkViewportSwizzleNV *pViewportSwizzles,
                                                                   const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV);
}

void ValidationStateTracker::PostCallRecordCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer,
                                                                         VkBool32 coverageToColorEnable,
                                                                         const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV);
    cb_state->dynamic_state_value.coverage_to_color_enable = coverageToColorEnable;
}

void ValidationStateTracker::PostCallRecordCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer,
                                                                           uint32_t coverageToColorLocation,
                                                                           const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV);
}

void ValidationStateTracker::PostCallRecordCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                                          VkCoverageModulationModeNV coverageModulationMode,
                                                                          const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV);
    cb_state->dynamic_state_value.coverage_modulation_mode = coverageModulationMode;
}

void ValidationStateTracker::PostCallRecordCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer,
                                                                                 VkBool32 coverageModulationTableEnable,
                                                                                 const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV);
    cb_state->dynamic_state_value.coverage_modulation_table_enable = coverageModulationTableEnable;
}

void ValidationStateTracker::PostCallRecordCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer,
                                                                           uint32_t coverageModulationTableCount,
                                                                           const float *pCoverageModulationTable,
                                                                           const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV);
}

void ValidationStateTracker::PostCallRecordCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer,
                                                                          VkBool32 shadingRateImageEnable,
                                                                          const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV);
    cb_state->dynamic_state_value.shading_rate_image_enable = shadingRateImageEnable;
}

void ValidationStateTracker::PostCallRecordCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                                                    VkBool32 representativeFragmentTestEnable,
                                                                                    const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV);
}

void ValidationStateTracker::PostCallRecordCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
                                                                         VkCoverageReductionModeNV coverageReductionMode,
                                                                         const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(record_obj.location.function, CB_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV);
}

void ValidationStateTracker::PostCallRecordCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                                    const VkVideoCodingControlInfoKHR *pCodingControlInfo,
                                                                    const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->ControlVideoCoding(pCodingControlInfo);
}

void ValidationStateTracker::PostCallRecordCmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR *pDecodeInfo,
                                                             const RecordObject &record_obj) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->DecodeVideo(pDecodeInfo);
}

void ValidationStateTracker::RecordGetBufferDeviceAddress(const VkBufferDeviceAddressInfo *pInfo, const RecordObject &record_obj) {
    auto buffer_state = Get<BUFFER_STATE>(pInfo->buffer);
    if (buffer_state && record_obj.device_address != 0) {
        WriteLockGuard guard(buffer_address_lock_);
        // address is used for GPU-AV and ray tracing buffer validation
        buffer_state->deviceAddress = record_obj.device_address;
        const auto address_range = buffer_state->DeviceAddressRange();

        BufferAddressInfillUpdateOps ops{{buffer_state.get()}};
        sparse_container::infill_update_range(buffer_address_map_, address_range, ops);
        buffer_device_address_ranges_version++;
    }
}

void ValidationStateTracker::PostCallRecordGetShaderModuleIdentifierEXT(VkDevice, const VkShaderModule shaderModule,
                                                                        VkShaderModuleIdentifierEXT *pIdentifier,
                                                                        const RecordObject &record_obj) {
    if (const auto shader_state = Get<SHADER_MODULE_STATE>(shaderModule); shader_state) {
        WriteLockGuard guard(shader_identifier_map_lock_);
        shader_identifier_map_.emplace(*pIdentifier, std::move(shader_state));
    }
}

void ValidationStateTracker::PostCallRecordGetShaderModuleCreateInfoIdentifierEXT(VkDevice,
                                                                                  const VkShaderModuleCreateInfo *pCreateInfo,
                                                                                  VkShaderModuleIdentifierEXT *pIdentifier,
                                                                                  const RecordObject &record_obj) {
    WriteLockGuard guard(shader_identifier_map_lock_);
    shader_identifier_map_.emplace(*pIdentifier, std::make_shared<SHADER_MODULE_STATE>(0));
}

void ValidationStateTracker::PreCallRecordCmdBindShadersEXT(VkCommandBuffer commandBuffer, uint32_t stageCount,
                                                            const VkShaderStageFlagBits *pStages, const VkShaderEXT *pShaders) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (pStages) {
        for (uint32_t i = 0; i < stageCount; ++i) {
            cb_state->BindPipeline(ConvertToLvlBindPoint(pStages[i]), nullptr);
        }
    }
}

void ValidationStateTracker::PostCallRecordGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo *pInfo,
                                                                  const RecordObject &record_obj) {
    RecordGetBufferDeviceAddress(pInfo, record_obj);
}

void ValidationStateTracker::PostCallRecordGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo *pInfo,
                                                                     const RecordObject &record_obj) {
    RecordGetBufferDeviceAddress(pInfo, record_obj);
}

void ValidationStateTracker::PostCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo *pInfo,
                                                                     const RecordObject &record_obj) {
    RecordGetBufferDeviceAddress(pInfo, record_obj);
}

std::shared_ptr<SWAPCHAIN_NODE> ValidationStateTracker::CreateSwapchainState(const VkSwapchainCreateInfoKHR *create_info,
                                                                             VkSwapchainKHR swapchain) {
    return std::make_shared<SWAPCHAIN_NODE>(this, create_info, swapchain);
}

std::shared_ptr<CMD_BUFFER_STATE> ValidationStateTracker::CreateCmdBufferState(VkCommandBuffer cb,
                                                                               const VkCommandBufferAllocateInfo *create_info,
                                                                               const COMMAND_POOL_STATE *pool) {
    return std::make_shared<CMD_BUFFER_STATE>(this, cb, create_info, pool);
}

std::shared_ptr<DEVICE_MEMORY_STATE> ValidationStateTracker::CreateDeviceMemoryState(
    VkDeviceMemory mem, const VkMemoryAllocateInfo *p_alloc_info, uint64_t fake_address, const VkMemoryType &memory_type,
    const VkMemoryHeap &memory_heap, std::optional<DedicatedBinding> &&dedicated_binding, uint32_t physical_device_count) {
    return std::make_shared<DEVICE_MEMORY_STATE>(mem, p_alloc_info, fake_address, memory_type, memory_heap,
                                                 std::move(dedicated_binding), physical_device_count);
}
