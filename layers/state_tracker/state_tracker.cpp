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
#include <cmath>

#include "vk_enum_string_helper.h"
#include "vk_format_utils.h"
#include "vk_layer_data.h"
#include "vk_layer_utils.h"
#include "vk_layer_logging.h"
#include "vk_typemap_helper.h"

#include "chassis.h"
#include "state_tracker/state_tracker.h"
#include "core_checks/shader_validation.h"
#include "sync/sync_utils.h"
#include "state_tracker/cmd_buffer_state.h"
#include "state_tracker/render_pass_state.h"

// NOTE:  Beware the lifespan of the rp_begin when holding  the return.  If the rp_begin isn't a "safe" copy, "IMAGELESS"
//        attachments won't persist past the API entry point exit.
static std::pair<uint32_t, const VkImageView *> GetFramebufferAttachments(const VkRenderPassBeginInfo &rp_begin,
                                                                          const FRAMEBUFFER_STATE &fb_state) {
    const VkImageView *attachments = fb_state.createInfo.pAttachments;
    uint32_t count = fb_state.createInfo.attachmentCount;
    if (fb_state.createInfo.flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) {
        const auto *framebuffer_attachments = LvlFindInChain<VkRenderPassAttachmentBeginInfo>(rp_begin.pNext);
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
    VkFormatFeatureFlags format_features = 0;
    const VkExternalFormatANDROID *ext_fmt_android = LvlFindInChain<VkExternalFormatANDROID>(create_info->pNext);
    if (ext_fmt_android && (0 != ext_fmt_android->externalFormat)) {
        // VUID 01894 will catch if not found in map
        auto it = ahb_ext_formats_map.find(ext_fmt_android->externalFormat);
        if (it != ahb_ext_formats_map.end()) {
            format_features = it->second;
        }
    }
    return format_features;
}

void ValidationStateTracker::PostCallRecordGetAndroidHardwareBufferPropertiesANDROID(
    VkDevice device, const struct AHardwareBuffer *buffer, VkAndroidHardwareBufferPropertiesANDROID *pProperties, VkResult result) {
    if (VK_SUCCESS != result) return;
    auto ahb_format_props2 = LvlFindInChain<VkAndroidHardwareBufferFormatProperties2ANDROID>(pProperties->pNext);
    if (ahb_format_props2) {
        ahb_ext_formats_map.insert(ahb_format_props2->externalFormat, ahb_format_props2->formatFeatures);
    } else {
        auto ahb_format_props = LvlFindInChain<VkAndroidHardwareBufferFormatPropertiesANDROID>(pProperties->pNext);
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
        auto fmt_drm_props = LvlInitStruct<VkDrmFormatModifierPropertiesList2EXT>();
        auto fmt_props_3 = LvlInitStruct<VkFormatProperties3KHR>(has_drm_modifiers ? &fmt_drm_props : nullptr);
        auto fmt_props_2 = LvlInitStruct<VkFormatProperties2>(&fmt_props_3);

        DispatchGetPhysicalDeviceFormatProperties2(physical_device, format, &fmt_props_2);

        if (tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
            VkImageDrmFormatModifierPropertiesEXT drm_format_props = LvlInitStruct<VkImageDrmFormatModifierPropertiesEXT>();

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
        VkImageDrmFormatModifierPropertiesEXT drm_format_properties = LvlInitStruct<VkImageDrmFormatModifierPropertiesEXT>();
        DispatchGetImageDrmFormatModifierPropertiesEXT(device, image, &drm_format_properties);

        VkFormatProperties2 format_properties_2 = LvlInitStruct<VkFormatProperties2>();
        VkDrmFormatModifierPropertiesListEXT drm_properties_list = LvlInitStruct<VkDrmFormatModifierPropertiesListEXT>();
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
    std::shared_ptr<IMAGE_STATE> state;

    if (pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) {
        if (pCreateInfo->flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) {
            state = std::make_shared<IMAGE_STATE_SPARSE<true>>(this, img, pCreateInfo, features);
        } else {
            state = std::make_shared<IMAGE_STATE_SPARSE<false>>(this, img, pCreateInfo, features);
        }
    } else if (pCreateInfo->flags & VK_IMAGE_CREATE_DISJOINT_BIT) {
        uint32_t plane_count = FormatPlaneCount(pCreateInfo->format);
        switch (plane_count) {
            case 3:
                state = std::make_shared<IMAGE_STATE_MULTIPLANAR<3>>(this, img, pCreateInfo, features);
                break;
            case 2:
                state = std::make_shared<IMAGE_STATE_MULTIPLANAR<2>>(this, img, pCreateInfo, features);
                break;
            case 1:
                state = std::make_shared<IMAGE_STATE_MULTIPLANAR<1>>(this, img, pCreateInfo, features);
                break;
            default:
                // Not supported
                assert(false);
        }
    } else {
        state = std::make_shared<IMAGE_STATE_LINEAR>(this, img, pCreateInfo, features);
    }

    return state;
}

std::shared_ptr<IMAGE_STATE> ValidationStateTracker::CreateImageState(VkImage img, const VkImageCreateInfo *pCreateInfo,
                                                                      VkSwapchainKHR swapchain, uint32_t swapchain_index,
                                                                      VkFormatFeatureFlags2KHR features) {
    return std::make_shared<IMAGE_STATE_NO_BINDING>(this, img, pCreateInfo, swapchain, swapchain_index, features);
}

void ValidationStateTracker::PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkImage *pImage, VkResult result) {
    if (VK_SUCCESS != result) return;
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
        cb_state->RecordTransferCmd(CMD_CLEARCOLORIMAGE, Get<IMAGE_STATE>(image));
    }
}

void ValidationStateTracker::PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image,
                                                                    VkImageLayout imageLayout,
                                                                    const VkClearDepthStencilValue *pDepthStencil,
                                                                    uint32_t rangeCount, const VkImageSubresourceRange *pRanges) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_state) {
        cb_state->RecordTransferCmd(CMD_CLEARDEPTHSTENCILIMAGE, Get<IMAGE_STATE>(image));
    }
}

void ValidationStateTracker::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                       VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout,
                                                       uint32_t regionCount, const VkImageCopy *pRegions) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(CMD_COPYIMAGE, Get<IMAGE_STATE>(srcImage), Get<IMAGE_STATE>(dstImage));
}

void ValidationStateTracker::PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer,
                                                           const VkCopyImageInfo2KHR *pCopyImageInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(CMD_COPYIMAGE2KHR, Get<IMAGE_STATE>(pCopyImageInfo->srcImage),
                                Get<IMAGE_STATE>(pCopyImageInfo->dstImage));
}

void ValidationStateTracker::PreCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(CMD_COPYIMAGE2, Get<IMAGE_STATE>(pCopyImageInfo->srcImage),
                                Get<IMAGE_STATE>(pCopyImageInfo->dstImage));
}

void ValidationStateTracker::PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                          VkImageLayout srcImageLayout, VkImage dstImage,
                                                          VkImageLayout dstImageLayout, uint32_t regionCount,
                                                          const VkImageResolve *pRegions) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(CMD_RESOLVEIMAGE, Get<IMAGE_STATE>(srcImage), Get<IMAGE_STATE>(dstImage));
}

void ValidationStateTracker::PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                                              const VkResolveImageInfo2KHR *pResolveImageInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(CMD_RESOLVEIMAGE2KHR, Get<IMAGE_STATE>(pResolveImageInfo->srcImage),
                                Get<IMAGE_STATE>(pResolveImageInfo->dstImage));
}

void ValidationStateTracker::PreCallRecordCmdResolveImage2(VkCommandBuffer commandBuffer,
                                                           const VkResolveImageInfo2 *pResolveImageInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(CMD_RESOLVEIMAGE2, Get<IMAGE_STATE>(pResolveImageInfo->srcImage),
                                Get<IMAGE_STATE>(pResolveImageInfo->dstImage));
}

void ValidationStateTracker::PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                       VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout,
                                                       uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(CMD_BLITIMAGE, Get<IMAGE_STATE>(srcImage), Get<IMAGE_STATE>(dstImage));
}

void ValidationStateTracker::PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer,
                                                           const VkBlitImageInfo2KHR *pBlitImageInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(CMD_BLITIMAGE2KHR, Get<IMAGE_STATE>(pBlitImageInfo->srcImage),
                                Get<IMAGE_STATE>(pBlitImageInfo->dstImage));
}

void ValidationStateTracker::PreCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(CMD_BLITIMAGE2, Get<IMAGE_STATE>(pBlitImageInfo->srcImage),
                                Get<IMAGE_STATE>(pBlitImageInfo->dstImage));
}

void ValidationStateTracker::PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                                        const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer,
                                                        VkResult result) {
    if (result != VK_SUCCESS) return;

    std::shared_ptr<BUFFER_STATE> buffer_state;
    if (pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) {
        if (pCreateInfo->flags & VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT) {
            buffer_state = std::make_shared<BUFFER_STATE_SPARSE<true>>(this, *pBuffer, pCreateInfo);
        } else {
            buffer_state = std::make_shared<BUFFER_STATE_SPARSE<false>>(this, *pBuffer, pCreateInfo);
        }
    } else {
        buffer_state = std::make_shared<BUFFER_STATE_LINEAR>(this, *pBuffer, pCreateInfo);
    }

    if (pCreateInfo) {
        const auto *opaque_capture_address = LvlFindInChain<VkBufferOpaqueCaptureAddressCreateInfo>(pCreateInfo->pNext);
        if (opaque_capture_address && (opaque_capture_address->opaqueCaptureAddress != 0)) {
            WriteLockGuard guard(buffer_address_lock_);
            // address is used for GPU-AV and ray tracing buffer validation
            buffer_state->deviceAddress = opaque_capture_address->opaqueCaptureAddress;
            const auto address_range = buffer_state->DeviceAddressRange();

            buffer_address_map_.split_and_merge_insert(
                {address_range, {buffer_state}}, [](auto &current_buffer_list, const auto &new_buffer) {
                    assert(!current_buffer_list.empty());
                    const auto buffer_found_it = std::find(current_buffer_list.begin(), current_buffer_list.end(), new_buffer[0]);
                    if (buffer_found_it == current_buffer_list.end()) {
                        current_buffer_list.emplace_back(new_buffer[0]);
                    }
                });
        }

        const VkBufferUsageFlags descriptor_buffer_usages =
            VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

        if ((pCreateInfo->usage & descriptor_buffer_usages) != 0) {
            descriptorBufferAddressSpaceSize += pCreateInfo->size;

            if ((pCreateInfo->usage & VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT) != 0)
                resourceDescriptorBufferAddressSpaceSize += pCreateInfo->size;

            if ((pCreateInfo->usage & VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT) != 0)
                samplerDescriptorBufferAddressSpaceSize += pCreateInfo->size;
        }
    }
    Add(std::move(buffer_state));
}

void ValidationStateTracker::PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator, VkBufferView *pView,
                                                            VkResult result) {
    if (result != VK_SUCCESS) return;

    auto buffer_state = Get<BUFFER_STATE>(pCreateInfo->buffer);

    VkFormatFeatureFlags2KHR buffer_features;
    if (has_format_feature2) {
        auto fmt_props_3 = LvlInitStruct<VkFormatProperties3KHR>();
        auto fmt_props_2 = LvlInitStruct<VkFormatProperties2>(&fmt_props_3);
        DispatchGetPhysicalDeviceFormatProperties2(physical_device, pCreateInfo->format, &fmt_props_2);
        buffer_features = fmt_props_3.bufferFeatures;
    } else {
        VkFormatProperties format_properties;
        DispatchGetPhysicalDeviceFormatProperties(physical_device, pCreateInfo->format, &format_properties);
        buffer_features = format_properties.bufferFeatures;
    }

    Add(std::make_shared<BUFFER_VIEW_STATE>(buffer_state, *pView, pCreateInfo, buffer_features));
}

void ValidationStateTracker::PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                                           const VkAllocationCallbacks *pAllocator, VkImageView *pView,
                                                           VkResult result) {
    if (result != VK_SUCCESS) return;
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
    auto filter_cubic_props = LvlInitStruct<VkFilterCubicImageViewImageFormatPropertiesEXT>();
    if (IsExtEnabled(device_extensions.vk_ext_filter_cubic)) {
        auto imageview_format_info = LvlInitStruct<VkPhysicalDeviceImageViewImageFormatInfoEXT>();
        imageview_format_info.imageViewType = pCreateInfo->viewType;
        auto image_format_info = LvlInitStruct<VkPhysicalDeviceImageFormatInfo2>(&imageview_format_info);
        image_format_info.type = image_state->createInfo.imageType;
        image_format_info.format = image_state->createInfo.format;
        image_format_info.tiling = image_state->createInfo.tiling;
        auto usage_create_info = LvlFindInChain<VkImageViewUsageCreateInfo>(pCreateInfo->pNext);
        image_format_info.usage = usage_create_info ? usage_create_info->usage : image_state->createInfo.usage;
        image_format_info.flags = image_state->createInfo.flags;

        auto image_format_properties = LvlInitStruct<VkImageFormatProperties2>(&filter_cubic_props);

        DispatchGetPhysicalDeviceImageFormatProperties2(physical_device, &image_format_info, &image_format_properties);
    }

    Add(std::make_shared<IMAGE_VIEW_STATE>(image_state, *pView, pCreateInfo, format_features, filter_cubic_props));
}

void ValidationStateTracker::PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                                        uint32_t regionCount, const VkBufferCopy *pRegions) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(CMD_COPYBUFFER, Get<BUFFER_STATE>(srcBuffer), Get<BUFFER_STATE>(dstBuffer));
}

void ValidationStateTracker::PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer,
                                                            const VkCopyBufferInfo2KHR *pCopyBufferInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(CMD_COPYBUFFER2KHR, Get<BUFFER_STATE>(pCopyBufferInfo->srcBuffer),
                                Get<BUFFER_STATE>(pCopyBufferInfo->dstBuffer));
}

void ValidationStateTracker::PreCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(CMD_COPYBUFFER2, Get<BUFFER_STATE>(pCopyBufferInfo->srcBuffer),
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

        if ((buffer_state->createInfo.usage & descriptor_buffer_usages) != 0) {
            descriptorBufferAddressSpaceSize -= buffer_state->createInfo.size;

            if (buffer_state->createInfo.usage & VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT)
                resourceDescriptorBufferAddressSpaceSize -= buffer_state->createInfo.size;

            if (buffer_state->createInfo.usage & VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT)
                samplerDescriptorBufferAddressSpaceSize -= buffer_state->createInfo.size;
        }

        if (buffer_state->deviceAddress != 0) {
            const auto address_range = buffer_state->DeviceAddressRange();

            buffer_address_map_.erase_range_or_touch(address_range, [&buffer_state](auto &buffers) {
                assert(!buffers.empty());
                const auto buffer_found_it = std::find(buffers.begin(), buffers.end(), buffer_state);
                assert(buffer_found_it != buffers.end());

                // If buffer list only has one element, remove range map entry.
                // Else, remove target buffer from buffer list.
                if (buffer_found_it != buffers.end()) {
                    if (buffers.size() == 1) {
                        return true;
                    } else {
                        assert(!buffers.empty());
                        size_t i = std::distance(buffers.begin(), buffer_found_it);
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
    cb_state->RecordTransferCmd(CMD_FILLBUFFER, Get<BUFFER_STATE>(dstBuffer));
}

void ValidationStateTracker::PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                               VkImageLayout srcImageLayout, VkBuffer dstBuffer,
                                                               uint32_t regionCount, const VkBufferImageCopy *pRegions) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->RecordTransferCmd(CMD_COPYIMAGETOBUFFER, Get<IMAGE_STATE>(srcImage), Get<BUFFER_STATE>(dstBuffer));
}

void ValidationStateTracker::PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                                   const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(CMD_COPYIMAGETOBUFFER2KHR, Get<IMAGE_STATE>(pCopyImageToBufferInfo->srcImage),
                                Get<BUFFER_STATE>(pCopyImageToBufferInfo->dstBuffer));
}

void ValidationStateTracker::PreCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                                const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(CMD_COPYIMAGETOBUFFER2, Get<IMAGE_STATE>(pCopyImageToBufferInfo->srcImage),
                                Get<BUFFER_STATE>(pCopyImageToBufferInfo->dstBuffer));
}

void ValidationStateTracker::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                               VkImageLayout dstImageLayout, uint32_t regionCount,
                                                               const VkBufferImageCopy *pRegions) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(CMD_COPYBUFFERTOIMAGE, Get<BUFFER_STATE>(srcBuffer), Get<IMAGE_STATE>(dstImage));
}

void ValidationStateTracker::PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                                   const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(CMD_COPYBUFFERTOIMAGE2KHR, Get<BUFFER_STATE>(pCopyBufferToImageInfo->srcBuffer),
                                Get<IMAGE_STATE>(pCopyBufferToImageInfo->dstImage));
}

void ValidationStateTracker::PreCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                                const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(CMD_COPYBUFFERTOIMAGE2, Get<BUFFER_STATE>(pCopyBufferToImageInfo->srcBuffer),
                                Get<IMAGE_STATE>(pCopyBufferToImageInfo->dstImage));
}

// Gets union of all features defined by Potential Format Features
// except, does not handle the external format case for AHB as that only can be used for sampled images
VkFormatFeatureFlags2KHR ValidationStateTracker::GetPotentialFormatFeatures(VkFormat format) const {
    VkFormatFeatureFlags2KHR format_features = 0;

    if (format != VK_FORMAT_UNDEFINED) {
        if (has_format_feature2) {
            auto fmt_drm_props = LvlInitStruct<VkDrmFormatModifierPropertiesList2EXT>();
            auto fmt_props_3 = LvlInitStruct<VkFormatProperties3KHR>(
                IsExtEnabled(device_extensions.vk_ext_image_drm_format_modifier) ? &fmt_drm_props : nullptr);
            auto fmt_props_2 = LvlInitStruct<VkFormatProperties2>(&fmt_props_3);

            DispatchGetPhysicalDeviceFormatProperties2(physical_device, format, &fmt_props_2);

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
                auto fmt_drm_props = LvlInitStruct<VkDrmFormatModifierPropertiesListEXT>();
                auto fmt_props_2 = LvlInitStruct<VkFormatProperties2>(&fmt_drm_props);

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
                                                        VkResult result) {
    if (VK_SUCCESS != result) return;

    // The current object represents the VkInstance, look up / create the object for the device.
    ValidationObject *device_object = GetLayerDataPtr(get_dispatch_key(*pDevice), layer_data_map);
    ValidationObject *validation_data = GetValidationObject(device_object->object_dispatch, this->container_type);
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
    const VkPhysicalDeviceFeatures *enabled_features_found = pCreateInfo->pEnabledFeatures;
    if (nullptr == enabled_features_found) {
        const auto *features2 = LvlFindInChain<VkPhysicalDeviceFeatures2>(pCreateInfo->pNext);
        if (features2) {
            enabled_features_found = &(features2->features);
        }
    }

    if (nullptr == enabled_features_found) {
        enabled_features.core = {};
    } else {
        enabled_features.core = *enabled_features_found;
    }

    const auto *vulkan_13_features = LvlFindInChain<VkPhysicalDeviceVulkan13Features>(pCreateInfo->pNext);
    if (vulkan_13_features) {
        enabled_features.core13 = *vulkan_13_features;
    } else {
        enabled_features.core13 = {};
        const auto *image_robustness_features = LvlFindInChain<VkPhysicalDeviceImageRobustnessFeatures>(pCreateInfo->pNext);
        if (image_robustness_features) {
            enabled_features.core13.robustImageAccess = image_robustness_features->robustImageAccess;
        }

        const auto *inline_uniform_block_features = LvlFindInChain<VkPhysicalDeviceInlineUniformBlockFeatures>(pCreateInfo->pNext);
        if (inline_uniform_block_features) {
            enabled_features.core13.inlineUniformBlock = inline_uniform_block_features->inlineUniformBlock;
            enabled_features.core13.descriptorBindingInlineUniformBlockUpdateAfterBind =
                inline_uniform_block_features->descriptorBindingInlineUniformBlockUpdateAfterBind;
        }

        const auto *pipeline_creation_cache_control_features =
            LvlFindInChain<VkPhysicalDevicePipelineCreationCacheControlFeatures>(pCreateInfo->pNext);
        if (pipeline_creation_cache_control_features) {
            enabled_features.core13.pipelineCreationCacheControl =
                pipeline_creation_cache_control_features->pipelineCreationCacheControl;
        }

        const auto *private_data_features = LvlFindInChain<VkPhysicalDevicePrivateDataFeatures>(pCreateInfo->pNext);
        if (private_data_features) {
            enabled_features.core13.privateData = private_data_features->privateData;
        }

        const auto *demote_to_helper_invocation_features =
            LvlFindInChain<VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures>(pCreateInfo->pNext);
        if (demote_to_helper_invocation_features) {
            enabled_features.core13.shaderDemoteToHelperInvocation =
                demote_to_helper_invocation_features->shaderDemoteToHelperInvocation;
        }

        const auto *terminate_invocation_features =
            LvlFindInChain<VkPhysicalDeviceShaderTerminateInvocationFeatures>(pCreateInfo->pNext);
        if (terminate_invocation_features) {
            enabled_features.core13.shaderTerminateInvocation = terminate_invocation_features->shaderTerminateInvocation;
        }

        const auto *subgroup_size_control_features =
            LvlFindInChain<VkPhysicalDeviceSubgroupSizeControlFeatures>(pCreateInfo->pNext);
        if (subgroup_size_control_features) {
            enabled_features.core13.subgroupSizeControl = subgroup_size_control_features->subgroupSizeControl;
            enabled_features.core13.computeFullSubgroups = subgroup_size_control_features->computeFullSubgroups;
        }

        const auto *synchronization2_features = LvlFindInChain<VkPhysicalDeviceSynchronization2Features>(pCreateInfo->pNext);
        if (synchronization2_features) {
            enabled_features.core13.synchronization2 = synchronization2_features->synchronization2;
        }

        const auto *texture_compression_astchdr_features =
            LvlFindInChain<VkPhysicalDeviceTextureCompressionASTCHDRFeatures>(pCreateInfo->pNext);
        if (texture_compression_astchdr_features) {
            enabled_features.core13.textureCompressionASTC_HDR = texture_compression_astchdr_features->textureCompressionASTC_HDR;
        }

        const auto *initialize_workgroup_memory_features =
            LvlFindInChain<VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures>(pCreateInfo->pNext);
        if (initialize_workgroup_memory_features) {
            enabled_features.core13.shaderZeroInitializeWorkgroupMemory =
                initialize_workgroup_memory_features->shaderZeroInitializeWorkgroupMemory;
        }

        const auto *dynamic_rendering_features = LvlFindInChain<VkPhysicalDeviceDynamicRenderingFeatures>(pCreateInfo->pNext);
        if (dynamic_rendering_features) {
            enabled_features.core13.dynamicRendering = dynamic_rendering_features->dynamicRendering;
        }

        const auto *shader_integer_dot_product_features =
            LvlFindInChain<VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR>(pCreateInfo->pNext);
        if (shader_integer_dot_product_features) {
            enabled_features.core13.shaderIntegerDotProduct = shader_integer_dot_product_features->shaderIntegerDotProduct;
        }

        const auto *maintenance4_features = LvlFindInChain<VkPhysicalDeviceMaintenance4FeaturesKHR>(pCreateInfo->pNext);
        if (maintenance4_features) {
            enabled_features.core13.maintenance4 = maintenance4_features->maintenance4;
        }
    }

    const auto *vulkan_12_features = LvlFindInChain<VkPhysicalDeviceVulkan12Features>(pCreateInfo->pNext);
    if (vulkan_12_features) {
        enabled_features.core12 = *vulkan_12_features;
    } else {
        // Set Extension Feature Aliases to false as there is no struct to check
        enabled_features.core12.drawIndirectCount = VK_FALSE;
        enabled_features.core12.samplerMirrorClampToEdge = VK_FALSE;
        enabled_features.core12.descriptorIndexing = VK_FALSE;
        enabled_features.core12.samplerFilterMinmax = VK_FALSE;
        enabled_features.core12.shaderOutputLayer = VK_FALSE;
        enabled_features.core12.shaderOutputViewportIndex = VK_FALSE;
        enabled_features.core12.subgroupBroadcastDynamicId = VK_FALSE;

        // These structs are only allowed in pNext chain if there is no VkPhysicalDeviceVulkan12Features

        const auto *eight_bit_storage_features = LvlFindInChain<VkPhysicalDevice8BitStorageFeatures>(pCreateInfo->pNext);
        if (eight_bit_storage_features) {
            enabled_features.core12.storageBuffer8BitAccess = eight_bit_storage_features->storageBuffer8BitAccess;
            enabled_features.core12.uniformAndStorageBuffer8BitAccess =
                eight_bit_storage_features->uniformAndStorageBuffer8BitAccess;
            enabled_features.core12.storagePushConstant8 = eight_bit_storage_features->storagePushConstant8;
        }

        const auto *float16_int8_features = LvlFindInChain<VkPhysicalDeviceShaderFloat16Int8Features>(pCreateInfo->pNext);
        if (float16_int8_features) {
            enabled_features.core12.shaderFloat16 = float16_int8_features->shaderFloat16;
            enabled_features.core12.shaderInt8 = float16_int8_features->shaderInt8;
        }

        const auto *descriptor_indexing_features = LvlFindInChain<VkPhysicalDeviceDescriptorIndexingFeatures>(pCreateInfo->pNext);
        if (descriptor_indexing_features) {
            enabled_features.core12.shaderInputAttachmentArrayDynamicIndexing =
                descriptor_indexing_features->shaderInputAttachmentArrayDynamicIndexing;
            enabled_features.core12.shaderUniformTexelBufferArrayDynamicIndexing =
                descriptor_indexing_features->shaderUniformTexelBufferArrayDynamicIndexing;
            enabled_features.core12.shaderStorageTexelBufferArrayDynamicIndexing =
                descriptor_indexing_features->shaderStorageTexelBufferArrayDynamicIndexing;
            enabled_features.core12.shaderUniformBufferArrayNonUniformIndexing =
                descriptor_indexing_features->shaderUniformBufferArrayNonUniformIndexing;
            enabled_features.core12.shaderSampledImageArrayNonUniformIndexing =
                descriptor_indexing_features->shaderSampledImageArrayNonUniformIndexing;
            enabled_features.core12.shaderStorageBufferArrayNonUniformIndexing =
                descriptor_indexing_features->shaderStorageBufferArrayNonUniformIndexing;
            enabled_features.core12.shaderStorageImageArrayNonUniformIndexing =
                descriptor_indexing_features->shaderStorageImageArrayNonUniformIndexing;
            enabled_features.core12.shaderInputAttachmentArrayNonUniformIndexing =
                descriptor_indexing_features->shaderInputAttachmentArrayNonUniformIndexing;
            enabled_features.core12.shaderUniformTexelBufferArrayNonUniformIndexing =
                descriptor_indexing_features->shaderUniformTexelBufferArrayNonUniformIndexing;
            enabled_features.core12.shaderStorageTexelBufferArrayNonUniformIndexing =
                descriptor_indexing_features->shaderStorageTexelBufferArrayNonUniformIndexing;
            enabled_features.core12.descriptorBindingUniformBufferUpdateAfterBind =
                descriptor_indexing_features->descriptorBindingUniformBufferUpdateAfterBind;
            enabled_features.core12.descriptorBindingSampledImageUpdateAfterBind =
                descriptor_indexing_features->descriptorBindingSampledImageUpdateAfterBind;
            enabled_features.core12.descriptorBindingStorageImageUpdateAfterBind =
                descriptor_indexing_features->descriptorBindingStorageImageUpdateAfterBind;
            enabled_features.core12.descriptorBindingStorageBufferUpdateAfterBind =
                descriptor_indexing_features->descriptorBindingStorageBufferUpdateAfterBind;
            enabled_features.core12.descriptorBindingUniformTexelBufferUpdateAfterBind =
                descriptor_indexing_features->descriptorBindingUniformTexelBufferUpdateAfterBind;
            enabled_features.core12.descriptorBindingStorageTexelBufferUpdateAfterBind =
                descriptor_indexing_features->descriptorBindingStorageTexelBufferUpdateAfterBind;
            enabled_features.core12.descriptorBindingUpdateUnusedWhilePending =
                descriptor_indexing_features->descriptorBindingUpdateUnusedWhilePending;
            enabled_features.core12.descriptorBindingPartiallyBound = descriptor_indexing_features->descriptorBindingPartiallyBound;
            enabled_features.core12.descriptorBindingVariableDescriptorCount =
                descriptor_indexing_features->descriptorBindingVariableDescriptorCount;
            enabled_features.core12.runtimeDescriptorArray = descriptor_indexing_features->runtimeDescriptorArray;
        }

        const auto *scalar_block_layout_features = LvlFindInChain<VkPhysicalDeviceScalarBlockLayoutFeatures>(pCreateInfo->pNext);
        if (scalar_block_layout_features) {
            enabled_features.core12.scalarBlockLayout = scalar_block_layout_features->scalarBlockLayout;
        }

        const auto *imageless_framebuffer_features =
            LvlFindInChain<VkPhysicalDeviceImagelessFramebufferFeatures>(pCreateInfo->pNext);
        if (imageless_framebuffer_features) {
            enabled_features.core12.imagelessFramebuffer = imageless_framebuffer_features->imagelessFramebuffer;
        }

        const auto *uniform_buffer_standard_layout_features =
            LvlFindInChain<VkPhysicalDeviceUniformBufferStandardLayoutFeatures>(pCreateInfo->pNext);
        if (uniform_buffer_standard_layout_features) {
            enabled_features.core12.uniformBufferStandardLayout =
                uniform_buffer_standard_layout_features->uniformBufferStandardLayout;
        }

        const auto *subgroup_extended_types_features =
            LvlFindInChain<VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures>(pCreateInfo->pNext);
        if (subgroup_extended_types_features) {
            enabled_features.core12.shaderSubgroupExtendedTypes = subgroup_extended_types_features->shaderSubgroupExtendedTypes;
        }

        const auto *separate_depth_stencil_layouts_features =
            LvlFindInChain<VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures>(pCreateInfo->pNext);
        if (separate_depth_stencil_layouts_features) {
            enabled_features.core12.separateDepthStencilLayouts =
                separate_depth_stencil_layouts_features->separateDepthStencilLayouts;
        }

        const auto *host_query_reset_features = LvlFindInChain<VkPhysicalDeviceHostQueryResetFeatures>(pCreateInfo->pNext);
        if (host_query_reset_features) {
            enabled_features.core12.hostQueryReset = host_query_reset_features->hostQueryReset;
        }

        const auto *timeline_semaphore_features = LvlFindInChain<VkPhysicalDeviceTimelineSemaphoreFeatures>(pCreateInfo->pNext);
        if (timeline_semaphore_features) {
            enabled_features.core12.timelineSemaphore = timeline_semaphore_features->timelineSemaphore;
        }

        const auto *buffer_device_address = LvlFindInChain<VkPhysicalDeviceBufferDeviceAddressFeatures>(pCreateInfo->pNext);
        if (buffer_device_address) {
            enabled_features.core12.bufferDeviceAddress = buffer_device_address->bufferDeviceAddress;
            enabled_features.core12.bufferDeviceAddressCaptureReplay = buffer_device_address->bufferDeviceAddressCaptureReplay;
            enabled_features.core12.bufferDeviceAddressMultiDevice = buffer_device_address->bufferDeviceAddressMultiDevice;
        }

        const auto *atomic_int64_features = LvlFindInChain<VkPhysicalDeviceShaderAtomicInt64Features>(pCreateInfo->pNext);
        if (atomic_int64_features) {
            enabled_features.core12.shaderBufferInt64Atomics = atomic_int64_features->shaderBufferInt64Atomics;
            enabled_features.core12.shaderSharedInt64Atomics = atomic_int64_features->shaderSharedInt64Atomics;
        }

        const auto *memory_model_features = LvlFindInChain<VkPhysicalDeviceVulkanMemoryModelFeatures>(pCreateInfo->pNext);
        if (memory_model_features) {
            enabled_features.core12.vulkanMemoryModel = memory_model_features->vulkanMemoryModel;
            enabled_features.core12.vulkanMemoryModelDeviceScope = memory_model_features->vulkanMemoryModelDeviceScope;
            enabled_features.core12.vulkanMemoryModelAvailabilityVisibilityChains =
                memory_model_features->vulkanMemoryModelAvailabilityVisibilityChains;
        }
    }

    const auto *vulkan_11_features = LvlFindInChain<VkPhysicalDeviceVulkan11Features>(pCreateInfo->pNext);
    if (vulkan_11_features) {
        enabled_features.core11 = *vulkan_11_features;
    } else {
        // These structs are only allowed in pNext chain if there is no vkPhysicalDeviceVulkan11Features

        const auto *sixteen_bit_storage_features = LvlFindInChain<VkPhysicalDevice16BitStorageFeatures>(pCreateInfo->pNext);
        if (sixteen_bit_storage_features) {
            enabled_features.core11.storageBuffer16BitAccess = sixteen_bit_storage_features->storageBuffer16BitAccess;
            enabled_features.core11.uniformAndStorageBuffer16BitAccess =
                sixteen_bit_storage_features->uniformAndStorageBuffer16BitAccess;
            enabled_features.core11.storagePushConstant16 = sixteen_bit_storage_features->storagePushConstant16;
            enabled_features.core11.storageInputOutput16 = sixteen_bit_storage_features->storageInputOutput16;
        }

        const auto *multiview_features = LvlFindInChain<VkPhysicalDeviceMultiviewFeatures>(pCreateInfo->pNext);
        if (multiview_features) {
            enabled_features.core11.multiview = multiview_features->multiview;
            enabled_features.core11.multiviewGeometryShader = multiview_features->multiviewGeometryShader;
            enabled_features.core11.multiviewTessellationShader = multiview_features->multiviewTessellationShader;
        }

        const auto *variable_pointers_features = LvlFindInChain<VkPhysicalDeviceVariablePointersFeatures>(pCreateInfo->pNext);
        if (variable_pointers_features) {
            enabled_features.core11.variablePointersStorageBuffer = variable_pointers_features->variablePointersStorageBuffer;
            enabled_features.core11.variablePointers = variable_pointers_features->variablePointers;
        }

        const auto *protected_memory_features = LvlFindInChain<VkPhysicalDeviceProtectedMemoryFeatures>(pCreateInfo->pNext);
        if (protected_memory_features) {
            enabled_features.core11.protectedMemory = protected_memory_features->protectedMemory;
        }

        const auto *ycbcr_conversion_features = LvlFindInChain<VkPhysicalDeviceSamplerYcbcrConversionFeatures>(pCreateInfo->pNext);
        if (ycbcr_conversion_features) {
            enabled_features.core11.samplerYcbcrConversion = ycbcr_conversion_features->samplerYcbcrConversion;
        }

        const auto *shader_draw_parameters_features =
            LvlFindInChain<VkPhysicalDeviceShaderDrawParametersFeatures>(pCreateInfo->pNext);
        if (shader_draw_parameters_features) {
            enabled_features.core11.shaderDrawParameters = shader_draw_parameters_features->shaderDrawParameters;
        }
    }

    const auto *device_group_ci = LvlFindInChain<VkDeviceGroupDeviceCreateInfo>(pCreateInfo->pNext);
    if (device_group_ci) {
        physical_device_count = device_group_ci->physicalDeviceCount;
        if (physical_device_count == 0) {
            physical_device_count =
                1;  // see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDeviceGroupDeviceCreateInfo.html
        }
        device_group_create_info = *device_group_ci;
    } else {
        device_group_create_info = LvlInitStruct<VkDeviceGroupDeviceCreateInfo>();
        device_group_create_info.physicalDeviceCount = 1;  // see previous VkDeviceGroupDeviceCreateInfo link
        device_group_create_info.pPhysicalDevices = &physical_device;
        physical_device_count = 1;
    }

    // Features from other extensions passesd in create info
    {
        const auto *exclusive_scissor_features = LvlFindInChain<VkPhysicalDeviceExclusiveScissorFeaturesNV>(pCreateInfo->pNext);
        if (exclusive_scissor_features) {
            enabled_features.exclusive_scissor_features = *exclusive_scissor_features;
        }

        const auto *shading_rate_image_features = LvlFindInChain<VkPhysicalDeviceShadingRateImageFeaturesNV>(pCreateInfo->pNext);
        if (shading_rate_image_features) {
            enabled_features.shading_rate_image_features = *shading_rate_image_features;
        }

        const auto *mesh_shader_features_NV = LvlFindInChain<VkPhysicalDeviceMeshShaderFeaturesNV>(pCreateInfo->pNext);
        if (mesh_shader_features_NV) {
            enabled_features.mesh_shader_features.sType = mesh_shader_features_NV->sType;
            enabled_features.mesh_shader_features.pNext = mesh_shader_features_NV->pNext;
            enabled_features.mesh_shader_features.meshShader = mesh_shader_features_NV->meshShader;
            enabled_features.mesh_shader_features.taskShader = mesh_shader_features_NV->taskShader;
        }

        const auto *mesh_shader_features = LvlFindInChain<VkPhysicalDeviceMeshShaderFeaturesEXT>(pCreateInfo->pNext);
        if (mesh_shader_features) {
            enabled_features.mesh_shader_features = *mesh_shader_features;
        }

        const auto *descriptor_buffer_features = LvlFindInChain<VkPhysicalDeviceDescriptorBufferFeaturesEXT>(pCreateInfo->pNext);
        if (descriptor_buffer_features) {
            enabled_features.descriptor_buffer_features = *descriptor_buffer_features;
        }

        const auto *transform_feedback_features = LvlFindInChain<VkPhysicalDeviceTransformFeedbackFeaturesEXT>(pCreateInfo->pNext);
        if (transform_feedback_features) {
            enabled_features.transform_feedback_features = *transform_feedback_features;
        }

        const auto *vtx_attrib_div_features = LvlFindInChain<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT>(pCreateInfo->pNext);
        if (vtx_attrib_div_features) {
            enabled_features.vtx_attrib_divisor_features = *vtx_attrib_div_features;
        }

        const auto *buffer_device_address_ext_features =
            LvlFindInChain<VkPhysicalDeviceBufferDeviceAddressFeaturesEXT>(pCreateInfo->pNext);
        if (buffer_device_address_ext_features) {
            enabled_features.buffer_device_address_ext_features = *buffer_device_address_ext_features;
        }

        const auto *cooperative_matrix_features = LvlFindInChain<VkPhysicalDeviceCooperativeMatrixFeaturesNV>(pCreateInfo->pNext);
        if (cooperative_matrix_features) {
            enabled_features.cooperative_matrix_features = *cooperative_matrix_features;
        }

        const auto *compute_shader_derivatives_features =
            LvlFindInChain<VkPhysicalDeviceComputeShaderDerivativesFeaturesNV>(pCreateInfo->pNext);
        if (compute_shader_derivatives_features) {
            enabled_features.compute_shader_derivatives_features = *compute_shader_derivatives_features;
        }

        const auto *fragment_shader_barycentric_features =
            LvlFindInChain<VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV>(pCreateInfo->pNext);
        if (fragment_shader_barycentric_features) {
            enabled_features.fragment_shader_barycentric_features = *fragment_shader_barycentric_features;
        }

        const auto *shader_image_footprint_features =
            LvlFindInChain<VkPhysicalDeviceShaderImageFootprintFeaturesNV>(pCreateInfo->pNext);
        if (shader_image_footprint_features) {
            enabled_features.shader_image_footprint_features = *shader_image_footprint_features;
        }

        const auto *fragment_shader_interlock_features =
            LvlFindInChain<VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT>(pCreateInfo->pNext);
        if (fragment_shader_interlock_features) {
            enabled_features.fragment_shader_interlock_features = *fragment_shader_interlock_features;
        }

        const auto *texel_buffer_alignment_features =
            LvlFindInChain<VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT>(pCreateInfo->pNext);
        if (texel_buffer_alignment_features) {
            enabled_features.texel_buffer_alignment_features = *texel_buffer_alignment_features;
        }
        // texelBufferAlignment was not promoted to VkPhysicalDeviceVulkan13Features
        // but the feature is automatically enabled.
        // Setting the feature explicitly to 'false' doesn't change that
        if (api_version >= VK_API_VERSION_1_3) {
            enabled_features.texel_buffer_alignment_features.texelBufferAlignment = true;
        }

        const auto *pipeline_exe_props_features =
            LvlFindInChain<VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR>(pCreateInfo->pNext);
        if (pipeline_exe_props_features) {
            enabled_features.pipeline_exe_props_features = *pipeline_exe_props_features;
        }

        const auto *dedicated_allocation_image_aliasing_features =
            LvlFindInChain<VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV>(pCreateInfo->pNext);
        if (dedicated_allocation_image_aliasing_features) {
            enabled_features.dedicated_allocation_image_aliasing_features = *dedicated_allocation_image_aliasing_features;
        }

        const auto *performance_query_features = LvlFindInChain<VkPhysicalDevicePerformanceQueryFeaturesKHR>(pCreateInfo->pNext);
        if (performance_query_features) {
            enabled_features.performance_query_features = *performance_query_features;
        }

        const auto *device_coherent_memory_features = LvlFindInChain<VkPhysicalDeviceCoherentMemoryFeaturesAMD>(pCreateInfo->pNext);
        if (device_coherent_memory_features) {
            enabled_features.device_coherent_memory_features = *device_coherent_memory_features;
        }

        const auto *ycbcr_image_array_features = LvlFindInChain<VkPhysicalDeviceYcbcrImageArraysFeaturesEXT>(pCreateInfo->pNext);
        if (ycbcr_image_array_features) {
            enabled_features.ycbcr_image_array_features = *ycbcr_image_array_features;
        }

        const auto *ray_query_features = LvlFindInChain<VkPhysicalDeviceRayQueryFeaturesKHR>(pCreateInfo->pNext);
        if (ray_query_features) {
            enabled_features.ray_query_features = *ray_query_features;
        }

        const auto *ray_tracing_pipeline_features =
            LvlFindInChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(pCreateInfo->pNext);
        if (ray_tracing_pipeline_features) {
            enabled_features.ray_tracing_pipeline_features = *ray_tracing_pipeline_features;
        }

        const auto *ray_tracing_acceleration_structure_features =
            LvlFindInChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(pCreateInfo->pNext);
        if (ray_tracing_acceleration_structure_features) {
            enabled_features.ray_tracing_acceleration_structure_features = *ray_tracing_acceleration_structure_features;
        }

        const auto *robustness2_features = LvlFindInChain<VkPhysicalDeviceRobustness2FeaturesEXT>(pCreateInfo->pNext);
        if (robustness2_features) {
            enabled_features.robustness2_features = *robustness2_features;
        }

        const auto *fragment_density_map_features =
            LvlFindInChain<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>(pCreateInfo->pNext);
        if (fragment_density_map_features) {
            enabled_features.fragment_density_map_features = *fragment_density_map_features;
        }

        const auto *fragment_density_map_features2 =
            LvlFindInChain<VkPhysicalDeviceFragmentDensityMap2FeaturesEXT>(pCreateInfo->pNext);
        if (fragment_density_map_features2) {
            enabled_features.fragment_density_map2_features = *fragment_density_map_features2;
        }

        const auto *fragment_density_map_offset_features =
            LvlFindInChain<VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM>(pCreateInfo->pNext);
        if (fragment_density_map_offset_features) {
            enabled_features.fragment_density_map_offset_features = *fragment_density_map_offset_features;
        }

        const auto *astc_decode_features = LvlFindInChain<VkPhysicalDeviceASTCDecodeFeaturesEXT>(pCreateInfo->pNext);
        if (astc_decode_features) {
            enabled_features.astc_decode_features = *astc_decode_features;
        }

        const auto *custom_border_color_features = LvlFindInChain<VkPhysicalDeviceCustomBorderColorFeaturesEXT>(pCreateInfo->pNext);
        if (custom_border_color_features) {
            enabled_features.custom_border_color_features = *custom_border_color_features;
        }

        const auto *fragment_shading_rate_features =
            LvlFindInChain<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>(pCreateInfo->pNext);
        if (fragment_shading_rate_features) {
            enabled_features.fragment_shading_rate_features = *fragment_shading_rate_features;
        }

        const auto *fragment_shading_rate_enums_features =
            LvlFindInChain<VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV>(pCreateInfo->pNext);
        if (fragment_shading_rate_enums_features) {
            enabled_features.fragment_shading_rate_enums_features = *fragment_shading_rate_enums_features;
        }

        const auto *extended_dynamic_state_features =
            LvlFindInChain<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT>(pCreateInfo->pNext);
        if (extended_dynamic_state_features) {
            enabled_features.extended_dynamic_state_features = *extended_dynamic_state_features;
        }

        const auto *extended_dynamic_state2_features =
            LvlFindInChain<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>(pCreateInfo->pNext);
        if (extended_dynamic_state2_features) {
            enabled_features.extended_dynamic_state2_features = *extended_dynamic_state2_features;
        }

        const auto *extended_dynamic_state3_features =
            LvlFindInChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(pCreateInfo->pNext);
        if (extended_dynamic_state3_features) {
            enabled_features.extended_dynamic_state3_features = *extended_dynamic_state3_features;
        }

        const auto *depth_clip_enable_features = LvlFindInChain<VkPhysicalDeviceDepthClipEnableFeaturesEXT>(pCreateInfo->pNext);
        if (depth_clip_enable_features) {
            enabled_features.depth_clip_enable_features = *depth_clip_enable_features;
        }

        const auto *depth_clip_control_features = LvlFindInChain<VkPhysicalDeviceDepthClipControlFeaturesEXT>(pCreateInfo->pNext);
        if (depth_clip_control_features) {
            enabled_features.depth_clip_control_features = *depth_clip_control_features;
        }

        const auto *line_rasterization_features = LvlFindInChain<VkPhysicalDeviceLineRasterizationFeaturesEXT>(pCreateInfo->pNext);
        if (line_rasterization_features) {
            enabled_features.line_rasterization_features = *line_rasterization_features;
        }

        const auto *multiview_features = LvlFindInChain<VkPhysicalDeviceMultiviewFeatures>(pCreateInfo->pNext);
        if (multiview_features) {
            enabled_features.multiview_features = *multiview_features;
        }

        const auto *portability_features = LvlFindInChain<VkPhysicalDevicePortabilitySubsetFeaturesKHR>(pCreateInfo->pNext);
        if (portability_features) {
            enabled_features.portability_subset_features = *portability_features;
        }

        const auto *shader_integer_functions2_features =
            LvlFindInChain<VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL>(pCreateInfo->pNext);
        if (shader_integer_functions2_features) {
            enabled_features.shader_integer_functions2_features = *shader_integer_functions2_features;
        }

        const auto *shader_sm_builtins_features = LvlFindInChain<VkPhysicalDeviceShaderSMBuiltinsFeaturesNV>(pCreateInfo->pNext);
        if (shader_sm_builtins_features) {
            enabled_features.shader_sm_builtins_features = *shader_sm_builtins_features;
        }

        const auto *shader_atomic_float_features = LvlFindInChain<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>(pCreateInfo->pNext);
        if (shader_atomic_float_features) {
            enabled_features.shader_atomic_float_features = *shader_atomic_float_features;
        }

        const auto *shader_image_atomic_int64_features =
            LvlFindInChain<VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT>(pCreateInfo->pNext);
        if (shader_image_atomic_int64_features) {
            enabled_features.shader_image_atomic_int64_features = *shader_image_atomic_int64_features;
        }

        const auto *shader_clock_features = LvlFindInChain<VkPhysicalDeviceShaderClockFeaturesKHR>(pCreateInfo->pNext);
        if (shader_clock_features) {
            enabled_features.shader_clock_features = *shader_clock_features;
        }

        const auto *conditional_rendering_features =
            LvlFindInChain<VkPhysicalDeviceConditionalRenderingFeaturesEXT>(pCreateInfo->pNext);
        if (conditional_rendering_features) {
            enabled_features.conditional_rendering_features = *conditional_rendering_features;
        }

        const auto *workgroup_memory_explicit_layout_features =
            LvlFindInChain<VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR>(pCreateInfo->pNext);
        if (workgroup_memory_explicit_layout_features) {
            enabled_features.workgroup_memory_explicit_layout_features = *workgroup_memory_explicit_layout_features;
        }

        const auto *provoking_vertex_features = LvlFindInChain<VkPhysicalDeviceProvokingVertexFeaturesEXT>(pCreateInfo->pNext);
        if (provoking_vertex_features) {
            enabled_features.provoking_vertex_features = *provoking_vertex_features;
        }

        const auto *vertex_input_dynamic_state_features =
            LvlFindInChain<VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT>(pCreateInfo->pNext);
        if (vertex_input_dynamic_state_features) {
            enabled_features.vertex_input_dynamic_state_features = *vertex_input_dynamic_state_features;
        }

        const auto *inherited_viewport_scissor_features =
            LvlFindInChain<VkPhysicalDeviceInheritedViewportScissorFeaturesNV>(pCreateInfo->pNext);
        if (inherited_viewport_scissor_features) {
            enabled_features.inherited_viewport_scissor_features = *inherited_viewport_scissor_features;
        }

        const auto *multi_draw_features = LvlFindInChain<VkPhysicalDeviceMultiDrawFeaturesEXT>(pCreateInfo->pNext);
        if (multi_draw_features) {
            enabled_features.multi_draw_features = *multi_draw_features;
        }

        const auto *color_write_features = LvlFindInChain<VkPhysicalDeviceColorWriteEnableFeaturesEXT>(pCreateInfo->pNext);
        if (color_write_features) {
            enabled_features.color_write_features = *color_write_features;
        }

        const auto *shader_atomic_float2_features =
            LvlFindInChain<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT>(pCreateInfo->pNext);
        if (shader_atomic_float2_features) {
            enabled_features.shader_atomic_float2_features = *shader_atomic_float2_features;
        }

        const auto *present_id_features = LvlFindInChain<VkPhysicalDevicePresentIdFeaturesKHR>(pCreateInfo->pNext);
        if (present_id_features) {
            enabled_features.present_id_features = *present_id_features;
        }

        const auto *present_wait_features = LvlFindInChain<VkPhysicalDevicePresentWaitFeaturesKHR>(pCreateInfo->pNext);
        if (present_wait_features) {
            enabled_features.present_wait_features = *present_wait_features;
        }

        const auto *ray_tracing_motion_blur_features =
            LvlFindInChain<VkPhysicalDeviceRayTracingMotionBlurFeaturesNV>(pCreateInfo->pNext);
        if (ray_tracing_motion_blur_features) {
            enabled_features.ray_tracing_motion_blur_features = *ray_tracing_motion_blur_features;
        }

        const auto *primitive_topology_list_restart_features =
            LvlFindInChain<VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT>(pCreateInfo->pNext);
        if (primitive_topology_list_restart_features) {
            enabled_features.primitive_topology_list_restart_features = *primitive_topology_list_restart_features;
        }

        const auto *rgba10x6_formats_features = LvlFindInChain<VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT>(pCreateInfo->pNext);
        if (rgba10x6_formats_features) {
            enabled_features.rgba10x6_formats_features = *rgba10x6_formats_features;
        }

        const auto *image_view_min_lod_features = LvlFindInChain<VkPhysicalDeviceImageViewMinLodFeaturesEXT>(pCreateInfo->pNext);
        if (image_view_min_lod_features) {
            enabled_features.image_view_min_lod_features = *image_view_min_lod_features;
        }

        const auto *primitives_generated_query_features =
            LvlFindInChain<VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT>(pCreateInfo->pNext);
        if (primitives_generated_query_features) {
            enabled_features.primitives_generated_query_features = *primitives_generated_query_features;
        }

        const auto image_2d_view_of_3d_features = LvlFindInChain<VkPhysicalDeviceImage2DViewOf3DFeaturesEXT>(pCreateInfo->pNext);
        if (image_2d_view_of_3d_features) {
            enabled_features.image_2d_view_of_3d_features = *image_2d_view_of_3d_features;
        }

        const auto graphics_pipeline_library_features =
            LvlFindInChain<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>(pCreateInfo->pNext);
        if (graphics_pipeline_library_features) {
            enabled_features.graphics_pipeline_library_features = *graphics_pipeline_library_features;
        }

        const auto shader_subgroup_uniform_control_flow_features =
            LvlFindInChain<VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR>(pCreateInfo->pNext);
        if (shader_subgroup_uniform_control_flow_features) {
            enabled_features.shader_subgroup_uniform_control_flow_features = *shader_subgroup_uniform_control_flow_features;
        }

        const auto ray_tracing_maintenance1_features =
            LvlFindInChain<VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR>(pCreateInfo->pNext);
        if (ray_tracing_maintenance1_features) {
            enabled_features.ray_tracing_maintenance1_features = *ray_tracing_maintenance1_features;
        }

        const auto non_seamless_cube_map_features =
            LvlFindInChain<VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT>(pCreateInfo->pNext);
        if (non_seamless_cube_map_features) {
            enabled_features.non_seamless_cube_map_features = *non_seamless_cube_map_features;
        }

        const auto multisampled_render_to_single_sampled_features =
            LvlFindInChain<VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT>(pCreateInfo->pNext);
        if (multisampled_render_to_single_sampled_features) {
            enabled_features.multisampled_render_to_single_sampled_features = *multisampled_render_to_single_sampled_features;
        }

        const auto shader_module_identifier_features =
            LvlFindInChain<VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT>(pCreateInfo->pNext);
        if (shader_module_identifier_features) {
            enabled_features.shader_module_identifier_features = *shader_module_identifier_features;
        }

        const auto attachment_feedback_loop_layout =
            LvlFindInChain<VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT>(pCreateInfo->pNext);
        if (attachment_feedback_loop_layout) {
            enabled_features.attachment_feedback_loop_layout_features = *attachment_feedback_loop_layout;
        }

        const auto pipeline_protected_access_features =
            LvlFindInChain<VkPhysicalDevicePipelineProtectedAccessFeaturesEXT>(pCreateInfo->pNext);
        if (pipeline_protected_access_features) {
            enabled_features.pipeline_protected_access_features = *pipeline_protected_access_features;
        }

        const auto linear_color_attachment_features =
            LvlFindInChain<VkPhysicalDeviceLinearColorAttachmentFeaturesNV>(pCreateInfo->pNext);
        if (linear_color_attachment_features) {
            enabled_features.linear_color_attachment_features = *linear_color_attachment_features;
        }

        const auto shader_core_builtins_features =
            LvlFindInChain<VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM>(pCreateInfo->pNext);
        if (shader_core_builtins_features) {
            enabled_features.shader_core_builtins_features = *shader_core_builtins_features;
        }

        const auto pipeline_library_group_handles_features =
            LvlFindInChain<VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT>(pCreateInfo->pNext);
        if (pipeline_library_group_handles_features) {
            enabled_features.pipeline_library_group_handles_features = *pipeline_library_group_handles_features;
        }
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
            auto multiview_props = LvlInitStruct<VkPhysicalDeviceMultiviewProperties>();
            GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_khr_multiview, &multiview_props);
            phys_dev_props_core11.maxMultiviewViewCount = multiview_props.maxMultiviewViewCount;
            phys_dev_props_core11.maxMultiviewInstanceIndex = multiview_props.maxMultiviewInstanceIndex;
        }

        if (dev_ext.vk_khr_maintenance3) {
            auto maintenance3_props = LvlInitStruct<VkPhysicalDeviceMaintenance3Properties>();
            GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_khr_maintenance3, &maintenance3_props);
            phys_dev_props_core11.maxPerSetDescriptors = maintenance3_props.maxPerSetDescriptors;
            phys_dev_props_core11.maxMemoryAllocationSize = maintenance3_props.maxMemoryAllocationSize;
        }

        // Some 1.1 properties were added to core without previous extensions
        if (api_version >= VK_API_VERSION_1_1) {
            auto subgroup_prop = LvlInitStruct<VkPhysicalDeviceSubgroupProperties>();
            auto protected_memory_prop = LvlInitStruct<VkPhysicalDeviceProtectedMemoryProperties>(&subgroup_prop);
            auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&protected_memory_prop);
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
            auto descriptor_indexing_prop = LvlInitStruct<VkPhysicalDeviceDescriptorIndexingProperties>();
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
            auto depth_stencil_resolve_props = LvlInitStruct<VkPhysicalDeviceDepthStencilResolveProperties>();
            GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_khr_depth_stencil_resolve, &depth_stencil_resolve_props);
            phys_dev_props_core12.supportedDepthResolveModes = depth_stencil_resolve_props.supportedDepthResolveModes;
            phys_dev_props_core12.supportedStencilResolveModes = depth_stencil_resolve_props.supportedStencilResolveModes;
            phys_dev_props_core12.independentResolveNone = depth_stencil_resolve_props.independentResolveNone;
            phys_dev_props_core12.independentResolve = depth_stencil_resolve_props.independentResolve;
        }

        if (dev_ext.vk_khr_timeline_semaphore) {
            auto timeline_semaphore_props = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreProperties>();
            GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_khr_timeline_semaphore, &timeline_semaphore_props);
            phys_dev_props_core12.maxTimelineSemaphoreValueDifference =
                timeline_semaphore_props.maxTimelineSemaphoreValueDifference;
        }

        if (dev_ext.vk_ext_sampler_filter_minmax) {
            auto sampler_filter_minmax_props = LvlInitStruct<VkPhysicalDeviceSamplerFilterMinmaxProperties>();
            GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_sampler_filter_minmax, &sampler_filter_minmax_props);
            phys_dev_props_core12.filterMinmaxSingleComponentFormats =
                sampler_filter_minmax_props.filterMinmaxSingleComponentFormats;
            phys_dev_props_core12.filterMinmaxImageComponentMapping = sampler_filter_minmax_props.filterMinmaxImageComponentMapping;
        }

        if (dev_ext.vk_khr_shader_float_controls) {
            auto float_controls_props = LvlInitStruct<VkPhysicalDeviceFloatControlsProperties>();
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
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_descriptor_buffer, &phys_dev_props->descriptor_buffer_props);
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_descriptor_buffer_density,
                                   &phys_dev_props->descriptor_buffer_density_props);
    if (api_version >= VK_API_VERSION_1_1) {
        GetPhysicalDeviceExtProperties(physical_device, &phys_dev_props->subgroup_props);
    }
    GetPhysicalDeviceExtProperties(physical_device, dev_ext.vk_ext_extended_dynamic_state3,
                                   &phys_dev_props->extended_dynamic_state3_props);

    if (IsExtEnabled(dev_ext.vk_nv_cooperative_matrix)) {
        // Get the needed cooperative_matrix properties
        auto cooperative_matrix_props = LvlInitStruct<VkPhysicalDeviceCooperativeMatrixPropertiesNV>();
        auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&cooperative_matrix_props);
        instance_dispatch_table.GetPhysicalDeviceProperties2KHR(physical_device, &prop2);
        phys_dev_ext_props.cooperative_matrix_props = cooperative_matrix_props;

        uint32_t num_cooperative_matrix_properties = 0;
        instance_dispatch_table.GetPhysicalDeviceCooperativeMatrixPropertiesNV(physical_device, &num_cooperative_matrix_properties,
                                                                               NULL);
        cooperative_matrix_properties.resize(num_cooperative_matrix_properties, LvlInitStruct<VkCooperativeMatrixPropertiesNV>());

        instance_dispatch_table.GetPhysicalDeviceCooperativeMatrixPropertiesNV(physical_device, &num_cooperative_matrix_properties,
                                                                               cooperative_matrix_properties.data());
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
                    auto get_info = LvlInitStruct<VkDeviceQueueInfo2>();
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

        std::vector<VkQueueFamilyProperties2> props(queue_family_count, LvlInitStruct<VkQueueFamilyProperties2>());

        if (dev_ext.vk_khr_video_queue) {
            for (uint32_t i = 0; i < queue_family_count; ++i) {
                ext_props[i].query_result_status_props = LvlInitStruct<VkQueueFamilyQueryResultStatusPropertiesKHR>();
                ext_props[i].video_props = LvlInitStruct<VkQueueFamilyVideoPropertiesKHR>(&ext_props[i].query_result_status_props);
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
        auto *timeline_semaphore_submit = LvlFindInChain<VkTimelineSemaphoreSubmitInfo>(submit->pNext);
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

        const auto perf_submit = LvlFindInChain<VkPerformanceQuerySubmitInfoKHR>(submit->pNext);
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
        const auto perf_submit = LvlFindInChain<VkPerformanceQuerySubmitInfoKHR>(submit->pNext);
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
                                                          VkResult result) {
    if (VK_SUCCESS != result) {
        return;
    }
    const auto &memory_type = phys_dev_mem_props.memoryTypes[pAllocateInfo->memoryTypeIndex];
    const auto &memory_heap = phys_dev_mem_props.memoryHeaps[memory_type.heapIndex];
    auto fake_address = fake_memory.Alloc(pAllocateInfo->allocationSize);

    std::optional<DedicatedBinding> dedicated_binding;

    auto dedicated = LvlFindInChain<VkMemoryDedicatedAllocateInfo>(pAllocateInfo->pNext);
    if (dedicated) {
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
        auto timeline_info = LvlFindInChain<VkTimelineSemaphoreSubmitInfo>(bind_info.pNext);
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
                                                           VkResult result) {
    if (VK_SUCCESS != result) return;
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
                                                           VkResult result) {
    if (result != VK_SUCCESS) return;

    auto semaphore_state = Get<SEMAPHORE_STATE>(pSignalInfo->semaphore);
    if (semaphore_state) {
        semaphore_state->Retire(nullptr, pSignalInfo->value);
    }
}

void ValidationStateTracker::PostCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo,
                                                              VkResult result) {
    PostCallRecordSignalSemaphore(device, pSignalInfo, result);
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
                                                         VkBool32 waitAll, uint64_t timeout, VkResult result) {
    if (VK_SUCCESS != result) return;

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
                                                      VkResult result) {
    if (VK_SUCCESS != result) return;

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
                                                          VkResult result) {
    PostRecordWaitSemaphores(device, pWaitInfo, timeout, result);
}

void ValidationStateTracker::PostCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo,
                                                             uint64_t timeout, VkResult result) {
    PostRecordWaitSemaphores(device, pWaitInfo, timeout, result);
}

void ValidationStateTracker::RecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t *pValue,
                                                            VkResult result) {
    if (VK_SUCCESS != result) return;

    auto semaphore_state = Get<SEMAPHORE_STATE>(semaphore);
    if (semaphore_state) {
        semaphore_state->NotifyAndWait(*pValue);
    }
}

void ValidationStateTracker::PostCallRecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t *pValue,
                                                                    VkResult result) {
    RecordGetSemaphoreCounterValue(device, semaphore, pValue, result);
}

void ValidationStateTracker::PostCallRecordGetSemaphoreCounterValueKHR(VkDevice device, VkSemaphore semaphore, uint64_t *pValue,
                                                                       VkResult result) {
    RecordGetSemaphoreCounterValue(device, semaphore, pValue, result);
}

void ValidationStateTracker::PostCallRecordGetFenceStatus(VkDevice device, VkFence fence, VkResult result) {
    if (VK_SUCCESS != result) return;
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
                                                          VkQueue *pQueue) {
    RecordGetDeviceQueueState(queueFamilyIndex, {}, *pQueue);
}

void ValidationStateTracker::PostCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue) {
    RecordGetDeviceQueueState(pQueueInfo->queueFamilyIndex, pQueueInfo->flags, *pQueue);
}

void ValidationStateTracker::PostCallRecordQueueWaitIdle(VkQueue queue, VkResult result) {
    if (VK_SUCCESS != result) return;
    auto queue_state = Get<QUEUE_STATE>(queue);
    if (queue_state) {
        queue_state->NotifyAndWait();
    }
}

void ValidationStateTracker::PostCallRecordDeviceWaitIdle(VkDevice device, VkResult result) {
    if (VK_SUCCESS != result) return;
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
                                                            VkDeviceSize memoryOffset, VkResult result) {
    if (VK_SUCCESS != result) return;
    UpdateBindBufferMemoryState(buffer, mem, memoryOffset);
}

void ValidationStateTracker::PostCallRecordBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                             const VkBindBufferMemoryInfo *pBindInfos, VkResult result) {
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        UpdateBindBufferMemoryState(pBindInfos[i].buffer, pBindInfos[i].memory, pBindInfos[i].memoryOffset);
    }
}

void ValidationStateTracker::PostCallRecordBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                                const VkBindBufferMemoryInfo *pBindInfos, VkResult result) {
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        UpdateBindBufferMemoryState(pBindInfos[i].buffer, pBindInfos[i].memory, pBindInfos[i].memoryOffset);
    }
}

void ValidationStateTracker::RecordGetBufferMemoryRequirementsState(VkBuffer buffer) {
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    if (buffer_state) {
        buffer_state->memory_requirements_checked = true;
    }
}

void ValidationStateTracker::PostCallRecordGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer,
                                                                       VkMemoryRequirements *pMemoryRequirements) {
    RecordGetBufferMemoryRequirementsState(buffer);
}

void ValidationStateTracker::PostCallRecordGetBufferMemoryRequirements2(VkDevice device,
                                                                        const VkBufferMemoryRequirementsInfo2 *pInfo,
                                                                        VkMemoryRequirements2 *pMemoryRequirements) {
    RecordGetBufferMemoryRequirementsState(pInfo->buffer);
}

void ValidationStateTracker::PostCallRecordGetBufferMemoryRequirements2KHR(VkDevice device,
                                                                           const VkBufferMemoryRequirementsInfo2 *pInfo,
                                                                           VkMemoryRequirements2 *pMemoryRequirements) {
    RecordGetBufferMemoryRequirementsState(pInfo->buffer);
}

void ValidationStateTracker::RecordGetImageMemoryRequirementsState(VkImage image, const VkImageMemoryRequirementsInfo2 *pInfo) {
    const VkImagePlaneMemoryRequirementsInfo *plane_info =
        (pInfo == nullptr) ? nullptr : LvlFindInChain<VkImagePlaneMemoryRequirementsInfo>(pInfo->pNext);
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
                                                                      VkMemoryRequirements *pMemoryRequirements) {
    RecordGetImageMemoryRequirementsState(image, nullptr);
}

void ValidationStateTracker::PostCallRecordGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                                                       VkMemoryRequirements2 *pMemoryRequirements) {
    RecordGetImageMemoryRequirementsState(pInfo->image, pInfo);
}

void ValidationStateTracker::PostCallRecordGetImageMemoryRequirements2KHR(VkDevice device,
                                                                          const VkImageMemoryRequirementsInfo2 *pInfo,
                                                                          VkMemoryRequirements2 *pMemoryRequirements) {
    RecordGetImageMemoryRequirementsState(pInfo->image, pInfo);
}

void ValidationStateTracker::PostCallRecordGetImageSparseMemoryRequirements(
    VkDevice device, VkImage image, uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements *pSparseMemoryRequirements) {
    auto image_state = Get<IMAGE_STATE>(image);
    image_state->get_sparse_reqs_called = true;
}

void ValidationStateTracker::PostCallRecordGetImageSparseMemoryRequirements2(
    VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo, uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements) {
    auto image_state = Get<IMAGE_STATE>(pInfo->image);
    image_state->get_sparse_reqs_called = true;
}

void ValidationStateTracker::PostCallRecordGetImageSparseMemoryRequirements2KHR(
    VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo, uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements) {
    auto image_state = Get<IMAGE_STATE>(pInfo->image);
    image_state->get_sparse_reqs_called = true;
}

void ValidationStateTracker::PreCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                                              const VkAllocationCallbacks *pAllocator) {
    Destroy<SHADER_MODULE_STATE>(shaderModule);
}

void ValidationStateTracker::PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline,
                                                          const VkAllocationCallbacks *pAllocator) {
    Destroy<PIPELINE_STATE>(pipeline);
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

void ValidationStateTracker::PostCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkCommandPool *pCommandPool,
                                                             VkResult result) {
    if (VK_SUCCESS != result) return;
    auto queue_flags = physical_device_state->queue_family_properties[pCreateInfo->queueFamilyIndex].queueFlags;
    Add(std::make_shared<COMMAND_POOL_STATE>(this, *pCommandPool, pCreateInfo, queue_flags));
}

void ValidationStateTracker::PostCallRecordCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                                           const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool,
                                                           VkResult result) {
    if (VK_SUCCESS != result) return;

    uint32_t index_count = 0, n_perf_pass = 0;
    bool has_cb = false, has_rb = false;
    if (pCreateInfo->queryType == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) {
        const auto *perf = LvlFindInChain<VkQueryPoolPerformanceCreateInfoKHR>(pCreateInfo->pNext);
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
        video_profile_cache_.Get(this, LvlFindInChain<VkVideoProfileInfoKHR>(pCreateInfo->pNext))));
}

void ValidationStateTracker::PreCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                                             const VkAllocationCallbacks *pAllocator) {
    Destroy<COMMAND_POOL_STATE>(commandPool);
}

void ValidationStateTracker::PostCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool,
                                                            VkCommandPoolResetFlags flags, VkResult result) {
    if (VK_SUCCESS != result) return;
    // Reset all of the CBs allocated from this pool
    auto pool = Get<COMMAND_POOL_STATE>(commandPool);
    if (pool) {
        pool->Reset();
    }
}

void ValidationStateTracker::PostCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences,
                                                       VkResult result) {
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
                                                       const VkAllocationCallbacks *pAllocator, VkFence *pFence, VkResult result) {
    if (VK_SUCCESS != result) return;
    Add(std::make_shared<FENCE_STATE>(*this, *pFence, pCreateInfo));
}

std::shared_ptr<PIPELINE_STATE> ValidationStateTracker::CreateGraphicsPipelineState(
    const VkGraphicsPipelineCreateInfo *pCreateInfo, uint32_t create_index, std::shared_ptr<const RENDER_PASS_STATE> &&render_pass,
    std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout, CreateShaderModuleStates *csm_states) const {
    return std::make_shared<PIPELINE_STATE>(this, pCreateInfo, create_index, std::move(render_pass), std::move(layout), csm_states);
}

bool ValidationStateTracker::PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                    const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                                    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                    void *cgpl_state_data) const {
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
        } else if (enabled_features.core13.dynamicRendering) {
            auto dynamic_rendering = LvlFindInChain<VkPipelineRenderingCreateInfo>(create_info.pNext);
            render_pass = std::make_shared<RENDER_PASS_STATE>(dynamic_rendering);
        } else {
            const bool is_graphics_lib = GetGraphicsLibType(create_info) != static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0);
            const bool has_link_info = LvlFindInChain<VkPipelineLibraryCreateInfoKHR>(create_info.pNext) != nullptr;
            if (!is_graphics_lib && !has_link_info) {
                skip = true;
            }
        }
        auto csm_states = (cgpl_state->shader_states.size() > i) ? &cgpl_state->shader_states[i] : nullptr;
        cgpl_state->pipe_state.push_back(
            CreateGraphicsPipelineState(&create_info, i, std::move(render_pass), std::move(layout_state), csm_states));
    }
    return skip;
}

void ValidationStateTracker::PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                   const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                                   const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                   VkResult result, void *cgpl_state_data) {
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
    const VkComputePipelineCreateInfo *pCreateInfo, uint32_t create_index,
    std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout) const {
    return std::make_shared<PIPELINE_STATE>(this, pCreateInfo, create_index, std::move(layout));
}

bool ValidationStateTracker::PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                   const VkComputePipelineCreateInfo *pCreateInfos,
                                                                   const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                   void *ccpl_state_data) const {
    auto *ccpl_state = reinterpret_cast<create_compute_pipeline_api_state *>(ccpl_state_data);
    ccpl_state->pCreateInfos = pCreateInfos;  // GPU validation can alter this, so we have to set a default value for the Chassis
    ccpl_state->pipe_state.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        // Create and initialize internal tracking data structure
        ccpl_state->pipe_state.push_back(
            CreateComputePipelineState(&pCreateInfos[i], i, Get<PIPELINE_LAYOUT_STATE>(pCreateInfos[i].layout)));
    }
    return false;
}

void ValidationStateTracker::PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                  const VkComputePipelineCreateInfo *pCreateInfos,
                                                                  const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                  VkResult result, void *ccpl_state_data) {
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
    const VkRayTracingPipelineCreateInfoNV *pCreateInfo, uint32_t create_index,
    std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout) const {
    return std::make_shared<PIPELINE_STATE>(this, pCreateInfo, create_index, std::move(layout));
}

bool ValidationStateTracker::PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache,
                                                                        uint32_t count,
                                                                        const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
                                                                        const VkAllocationCallbacks *pAllocator,
                                                                        VkPipeline *pPipelines, void *crtpl_state_data) const {
    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_api_state *>(crtpl_state_data);
    crtpl_state->pipe_state.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        // Create and initialize internal tracking data structure
        crtpl_state->pipe_state.push_back(
            CreateRayTracingPipelineState(&pCreateInfos[i], i, Get<PIPELINE_LAYOUT_STATE>(pCreateInfos[i].layout)));
    }
    return false;
}

void ValidationStateTracker::PostCallRecordCreateRayTracingPipelinesNV(
    VkDevice device, VkPipelineCache pipelineCache, uint32_t count, const VkRayTracingPipelineCreateInfoNV *pCreateInfos,
    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines, VkResult result, void *crtpl_state_data) {
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
    const VkRayTracingPipelineCreateInfoKHR *pCreateInfo, uint32_t create_index,
    std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&layout) const {
    return std::make_shared<PIPELINE_STATE>(this, pCreateInfo, create_index, std::move(layout));
}

bool ValidationStateTracker::PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                         VkPipelineCache pipelineCache, uint32_t count,
                                                                         const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                                         const VkAllocationCallbacks *pAllocator,
                                                                         VkPipeline *pPipelines, void *crtpl_state_data) const {
    auto crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_khr_api_state *>(crtpl_state_data);
    crtpl_state->pipe_state.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        // Create and initialize internal tracking data structure
        crtpl_state->pipe_state.push_back(
            CreateRayTracingPipelineState(&pCreateInfos[i], i, Get<PIPELINE_LAYOUT_STATE>(pCreateInfos[i].layout)));
    }
    return false;
}

void ValidationStateTracker::PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                        VkPipelineCache pipelineCache, uint32_t count,
                                                                        const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                                        const VkAllocationCallbacks *pAllocator,
                                                                        VkPipeline *pPipelines, VkResult result,
                                                                        void *crtpl_state_data) {
    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_khr_api_state *>(crtpl_state_data);
    const bool operation_is_deferred = (deferredOperation != VK_NULL_HANDLE && result == VK_OPERATION_DEFERRED_KHR);
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

void ValidationStateTracker::PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator, VkSampler *pSampler,
                                                         VkResult result) {
    Add(std::make_shared<SAMPLER_STATE>(pSampler, pCreateInfo));
    if (pCreateInfo->borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT ||
        pCreateInfo->borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT) {
        custom_border_color_sampler_count++;
    }
}

void ValidationStateTracker::PostCallRecordCreateDescriptorSetLayout(VkDevice device,
                                                                     const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkDescriptorSetLayout *pSetLayout, VkResult result) {
    if (VK_SUCCESS != result) return;
    Add(std::make_shared<cvdescriptorset::DescriptorSetLayout>(pCreateInfo, *pSetLayout));
}

void ValidationStateTracker::PostCallRecordGetDescriptorSetLayoutSizeEXT(VkDevice device, VkDescriptorSetLayout layout,
                                                                         VkDeviceSize *pLayoutSizeInBytes) {
    auto descriptor_set_layout = Get<cvdescriptorset::DescriptorSetLayout>(layout);

    descriptor_set_layout->SetLayoutSizeInBytes(pLayoutSizeInBytes);
}

void ValidationStateTracker::PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                VkPipelineLayout *pPipelineLayout, VkResult result) {
    if (VK_SUCCESS != result) return;
    Add(std::make_shared<PIPELINE_LAYOUT_STATE>(this, *pPipelineLayout, pCreateInfo));
}

std::shared_ptr<DESCRIPTOR_POOL_STATE> ValidationStateTracker::CreateDescriptorPoolState(
    VkDescriptorPool pool, const VkDescriptorPoolCreateInfo *pCreateInfo) {
    return std::make_shared<DESCRIPTOR_POOL_STATE>(this, pool, pCreateInfo);
}

void ValidationStateTracker::PostCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                VkDescriptorPool *pDescriptorPool, VkResult result) {
    if (VK_SUCCESS != result) return;
    Add(CreateDescriptorPoolState(*pDescriptorPool, pCreateInfo));
}

void ValidationStateTracker::PostCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                               VkDescriptorPoolResetFlags flags, VkResult result) {
    if (VK_SUCCESS != result) return;
    auto pool = Get<DESCRIPTOR_POOL_STATE>(descriptorPool);
    if (pool) {
        pool->Reset();
    }
}

bool ValidationStateTracker::PreCallValidateAllocateDescriptorSets(VkDevice device,
                                                                   const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                                   VkDescriptorSet *pDescriptorSets, void *ads_state_data) const {
    // Always update common data
    cvdescriptorset::AllocateDescriptorSetsData *ads_state =
        reinterpret_cast<cvdescriptorset::AllocateDescriptorSetsData *>(ads_state_data);
    UpdateAllocateDescriptorSetsData(pAllocateInfo, ads_state);

    return false;
}

// Allocation state was good and call down chain was made so update state based on allocating descriptor sets
void ValidationStateTracker::PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                                  VkDescriptorSet *pDescriptorSets, VkResult result,
                                                                  void *ads_state_data) {
    if (VK_SUCCESS != result) return;
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

void ValidationStateTracker::PreCallRecordUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                               const VkWriteDescriptorSet *pDescriptorWrites,
                                                               uint32_t descriptorCopyCount,
                                                               const VkCopyDescriptorSet *pDescriptorCopies) {
    cvdescriptorset::PerformUpdateDescriptorSets(this, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount,
                                                 pDescriptorCopies);
}

void ValidationStateTracker::PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pCreateInfo,
                                                                  VkCommandBuffer *pCommandBuffers, VkResult result) {
    if (VK_SUCCESS != result) return;
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

void ValidationStateTracker::PostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, VkResult result) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state) return;

    cb_state->End(result);
}

void ValidationStateTracker::PostCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                                              VkResult result) {
    if (VK_SUCCESS == result) {
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
    cb_state->RecordCmd(CMD_BINDPIPELINE);

    auto pipe_state = Get<PIPELINE_STATE>(pipeline);
    if (VK_PIPELINE_BIND_POINT_GRAPHICS == pipelineBindPoint) {
        const auto *raster_state = pipe_state->RasterizationState();
        const bool rasterization_enabled = raster_state && !raster_state->rasterizerDiscardEnable;
        const auto *viewport_state = pipe_state->ViewportState();
        const auto *dynamic_state = pipe_state->DynamicState();
        cb_state->status &= ~cb_state->static_status;
        cb_state->static_status = MakeStaticStateMask(dynamic_state ? dynamic_state->ptr() : nullptr);
        cb_state->status |= cb_state->static_status;
        cb_state->dynamic_status = ~CBDynamicFlags(0);
        cb_state->dynamic_status &= ~cb_state->static_status;

        // Used to calculate CMD_BUFFER_STATE::usedViewportScissorCount upon draw command with this graphics pipeline.
        // If rasterization disabled (no viewport/scissors used), or the actual number of viewports/scissors is dynamic (unknown at
        // this time), then these are set to 0 to disable this checking.
        auto has_dynamic_viewport_count = cb_state->dynamic_status[CB_DYNAMIC_VIEWPORT_WITH_COUNT_SET];
        auto has_dynamic_scissor_count = cb_state->dynamic_status[CB_DYNAMIC_SCISSOR_WITH_COUNT_SET];
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
            if (rasterization_enabled && (cb_state->static_status[CB_DYNAMIC_VIEWPORT_SET])) {
                cb_state->trashedViewportMask |= (uint32_t(1) << viewport_state->viewportCount) - 1u;
                // should become = ~uint32_t(0) if the other interpretation is correct.
            }
        }
        if (!has_dynamic_scissor_count) {
            cb_state->trashedScissorCount = true;
            if (rasterization_enabled && (cb_state->static_status[CB_DYNAMIC_SCISSOR_SET])) {
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

void ValidationStateTracker::PostCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                          uint32_t viewportCount, const VkViewport *pViewports) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETVIEWPORT, CB_DYNAMIC_VIEWPORT_SET);
    uint32_t bits = ((1u << viewportCount) - 1u) << firstViewport;
    cb_state->viewportMask |= bits;
    cb_state->trashedViewportMask &= ~bits;

    cb_state->dynamicViewports.resize(std::max(size_t(firstViewport + viewportCount), cb_state->dynamicViewports.size()));
    for (size_t i = 0; i < viewportCount; ++i) {
        cb_state->dynamicViewports[firstViewport + i] = pViewports[i];
    }
}

void ValidationStateTracker::PostCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                                    uint32_t exclusiveScissorCount,
                                                                    const VkRect2D *pExclusiveScissors) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETEXCLUSIVESCISSORNV, CB_DYNAMIC_EXCLUSIVE_SCISSOR_NV_SET);
    // TODO: We don't have VUIDs for validating that all exclusive scissors have been set.
    // cb_state->exclusiveScissorMask |= ((1u << exclusiveScissorCount) - 1u) << firstExclusiveScissor;
}

void ValidationStateTracker::PreCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                                    VkImageLayout imageLayout) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_BINDSHADINGRATEIMAGENV);

    if (imageView != VK_NULL_HANDLE) {
        auto view_state = Get<IMAGE_VIEW_STATE>(imageView);
        cb_state->AddChild(view_state);
    }
}

void ValidationStateTracker::PostCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                              uint32_t viewportCount,
                                                                              const VkShadingRatePaletteNV *pShadingRatePalettes) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETVIEWPORTSHADINGRATEPALETTENV, CB_DYNAMIC_VIEWPORT_SHADING_RATE_PALETTE_NV_SET);
    // TODO: We don't have VUIDs for validating that all shading rate palettes have been set.
    // cb_state->shadingRatePaletteMask |= ((1u << viewportCount) - 1u) << firstViewport;
}

void ValidationStateTracker::PostCallRecordCreateAccelerationStructureNV(VkDevice device,
                                                                         const VkAccelerationStructureCreateInfoNV *pCreateInfo,
                                                                         const VkAllocationCallbacks *pAllocator,
                                                                         VkAccelerationStructureNV *pAccelerationStructure,
                                                                         VkResult result) {
    if (VK_SUCCESS != result) return;
    std::shared_ptr<ACCELERATION_STRUCTURE_STATE> state =
        std::make_shared<ACCELERATION_STRUCTURE_STATE_LINEAR>(device, *pAccelerationStructure, pCreateInfo);
    Add(std::move(state));
}

void ValidationStateTracker::PostCallRecordCreateAccelerationStructureKHR(VkDevice device,
                                                                          const VkAccelerationStructureCreateInfoKHR *pCreateInfo,
                                                                          const VkAllocationCallbacks *pAllocator,
                                                                          VkAccelerationStructureKHR *pAccelerationStructure,
                                                                          VkResult result) {
    if (VK_SUCCESS != result) return;
    auto buffer_state = Get<BUFFER_STATE>(pCreateInfo->buffer);
    Add(std::make_shared<ACCELERATION_STRUCTURE_STATE_KHR>(*pAccelerationStructure, pCreateInfo, std::move(buffer_state)));
}

void ValidationStateTracker::PostCallRecordBuildAccelerationStructuresKHR(
    VkDevice device, VkDeferredOperationKHR deferredOperation, uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos, VkResult result) {
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
    auto scratch_buffers = GetBuffersByAddress(info.scratchData.deviceAddress);
    if (!scratch_buffers.empty()) {
        cb_state.AddChildren(scratch_buffers);
    }

    for (uint32_t i = 0; i < info.geometryCount; i++) {
        // only one of pGeometries and ppGeometries can be non-null
        const auto &geom = info.pGeometries ? info.pGeometries[i] : *info.ppGeometries[i];
        switch (geom.geometryType) {
            case VK_GEOMETRY_TYPE_TRIANGLES_KHR: {
                auto vertex_buffers = GetBuffersByAddress(geom.geometry.triangles.vertexData.deviceAddress);
                if (!vertex_buffers.empty()) {
                    cb_state.AddChildren(vertex_buffers);
                }
                auto index_buffers = GetBuffersByAddress(geom.geometry.triangles.indexData.deviceAddress);
                if (!index_buffers.empty()) {
                    cb_state.AddChildren(index_buffers);
                }
                auto transform_buffers = GetBuffersByAddress(geom.geometry.triangles.transformData.deviceAddress);
                if (!transform_buffers.empty()) {
                    cb_state.AddChildren(transform_buffers);
                }
                const auto *motion_data = LvlFindInChain<VkAccelerationStructureGeometryMotionTrianglesDataNV>(info.pNext);
                if (motion_data) {
                    auto motion_buffers = GetBuffersByAddress(motion_data->vertexData.deviceAddress);
                    if (!motion_buffers.empty()) {
                        cb_state.AddChildren(motion_buffers);
                    }
                }
            } break;
            case VK_GEOMETRY_TYPE_AABBS_KHR: {
                auto data_buffers = GetBuffersByAddress(geom.geometry.aabbs.data.deviceAddress);
                if (!data_buffers.empty()) {
                    cb_state.AddChildren(data_buffers);
                }
            } break;
            case VK_GEOMETRY_TYPE_INSTANCES_KHR: {
                // NOTE: if arrayOfPointers is true, we don't track the pointers in the array. That would
                // require that data buffer be mapped to the CPU so that we could walk through it. We can't
                // easily ensure that's true.
                auto data_buffers = GetBuffersByAddress(geom.geometry.instances.data.deviceAddress);
                if (!data_buffers.empty()) {
                    cb_state.AddChildren(data_buffers);
                }
            } break;
            default:
                break;
        }
    }
}

void ValidationStateTracker::PostCallRecordCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state) {
        return;
    }
    cb_state->RecordCmd(CMD_BUILDACCELERATIONSTRUCTURESKHR);
    for (uint32_t i = 0; i < infoCount; i++) {
        RecordDeviceAccelerationStructureBuildInfo(*cb_state, pInfos[i]);
    }
    cb_state->has_build_as_cmd = true;
}

void ValidationStateTracker::PostCallRecordCmdBuildAccelerationStructuresIndirectKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkDeviceAddress *pIndirectDeviceAddresses, const uint32_t *pIndirectStrides,
    const uint32_t *const *ppMaxPrimitiveCounts) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state) {
        return;
    }
    cb_state->RecordCmd(CMD_BUILDACCELERATIONSTRUCTURESINDIRECTKHR);
    for (uint32_t i = 0; i < infoCount; i++) {
        RecordDeviceAccelerationStructureBuildInfo(*cb_state, pInfos[i]);
        if (!disabled[command_buffer_state]) {
            auto indirect_buffer = GetBuffersByAddress(pIndirectDeviceAddresses[i]);
            if (!indirect_buffer.empty()) {
                cb_state->AddChildren(indirect_buffer);
            }
        }
    }
    cb_state->has_build_as_cmd = true;
}

void ValidationStateTracker::PostCallRecordGetAccelerationStructureMemoryRequirementsNV(
    VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV *pInfo, VkMemoryRequirements2 *pMemoryRequirements) {
    auto as_state = Get<ACCELERATION_STRUCTURE_STATE>(pInfo->accelerationStructure);
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
    VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoNV *pBindInfos, VkResult result) {
    if (VK_SUCCESS != result) return;
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        const VkBindAccelerationStructureMemoryInfoNV &info = pBindInfos[i];

        auto as_state = Get<ACCELERATION_STRUCTURE_STATE>(info.accelerationStructure);
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
    VkBool32 update, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch, VkDeviceSize scratchOffset) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state) {
        return;
    }
    cb_state->RecordCmd(CMD_BUILDACCELERATIONSTRUCTURENV);

    auto dst_as_state = Get<ACCELERATION_STRUCTURE_STATE>(dst);
    if (dst_as_state) {
        dst_as_state->Build(pInfo);
        if (!disabled[command_buffer_state]) {
            cb_state->AddChild(dst_as_state);
        }
    }
    if (!disabled[command_buffer_state]) {
        auto src_as_state = Get<ACCELERATION_STRUCTURE_STATE>(src);
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
                                                                          VkCopyAccelerationStructureModeNV mode) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_state) {
        auto src_as_state = Get<ACCELERATION_STRUCTURE_STATE>(src);
        auto dst_as_state = Get<ACCELERATION_STRUCTURE_STATE>(dst);
        if (!disabled[command_buffer_state]) {
            cb_state->RecordTransferCmd(CMD_COPYACCELERATIONSTRUCTURENV, src_as_state, dst_as_state);
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
    Destroy<ACCELERATION_STRUCTURE_STATE>(accelerationStructure);
}

void ValidationStateTracker::PostCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                    uint32_t viewportCount,
                                                                    const VkViewportWScalingNV *pViewportWScalings) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETVIEWPORTWSCALINGNV, CB_DYNAMIC_VIEWPORT_W_SCALING_NV_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETLINEWIDTH, CB_DYNAMIC_LINE_WIDTH_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                                uint16_t lineStipplePattern) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETLINESTIPPLEEXT, CB_DYNAMIC_LINE_STIPPLE_EXT_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor,
                                                           float depthBiasClamp, float depthBiasSlopeFactor) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETDEPTHBIAS, CB_DYNAMIC_DEPTH_BIAS_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor,
                                                         uint32_t scissorCount, const VkRect2D *pScissors) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETSCISSOR, CB_DYNAMIC_SCISSOR_SET);
    uint32_t bits = ((1u << scissorCount) - 1u) << firstScissor;
    cb_state->scissorMask |= bits;
    cb_state->trashedScissorMask &= ~bits;
}

void ValidationStateTracker::PostCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETBLENDCONSTANTS, CB_DYNAMIC_BLEND_CONSTANTS_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds,
                                                             float maxDepthBounds) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETDEPTHBOUNDS, CB_DYNAMIC_DEPTH_BOUNDS_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                                    uint32_t compareMask) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETSTENCILCOMPAREMASK, CB_DYNAMIC_STENCIL_COMPARE_MASK_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                                  uint32_t writeMask) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETSTENCILWRITEMASK, CB_DYNAMIC_STENCIL_WRITE_MASK_SET);
    if (faceMask == VK_STENCIL_FACE_FRONT_BIT || faceMask == VK_STENCIL_FACE_FRONT_AND_BACK) {
        cb_state->dynamic_state_value.write_mask_front = writeMask;
    }
    if (faceMask == VK_STENCIL_FACE_BACK_BIT || faceMask == VK_STENCIL_FACE_FRONT_AND_BACK) {
        cb_state->dynamic_state_value.write_mask_back = writeMask;
    }
}

void ValidationStateTracker::PostCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                                  uint32_t reference) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETSTENCILREFERENCE, CB_DYNAMIC_STENCIL_REFERENCE_SET);
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
    cb_state->RecordCmd(CMD_BINDDESCRIPTORSETS);

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
                                                            const void *pValues) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_state) {
        cb_state->RecordCmd(CMD_PUSHCONSTANTS);
        auto layout_state = Get<PIPELINE_LAYOUT_STATE>(layout);
        cb_state->ResetPushConstantDataIfIncompatible(layout_state.get());

        auto &push_constant_data = cb_state->push_constant_data;
        assert((offset + size) <= static_cast<uint32_t>(push_constant_data.size()));
        std::memcpy(push_constant_data.data() + offset, pValues, static_cast<std::size_t>(size));
        cb_state->push_constant_pipeline_layout_set = layout;

        auto flags = stageFlags;
        uint32_t bit_shift = 0;
        while (flags) {
            if (flags & 1) {
                VkShaderStageFlagBits flag = static_cast<VkShaderStageFlagBits>(1 << bit_shift);
                const auto it = cb_state->push_constant_data_update.find(flag);

                if (it != cb_state->push_constant_data_update.end()) {
                    std::memset(it->second.data() + offset, PC_Byte_Updated, static_cast<std::size_t>(size));
                }
            }
            flags = flags >> 1;
            ++bit_shift;
        }
    }
}

void ValidationStateTracker::PreCallRecordCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                             VkIndexType indexType) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->index_buffer_binding.buffer_state = Get<BUFFER_STATE>(buffer);
    cb_state->index_buffer_binding.size = cb_state->index_buffer_binding.buffer_state->createInfo.size;
    cb_state->index_buffer_binding.offset = offset;
    cb_state->index_buffer_binding.index_type = indexType;
    // Add binding for this index buffer to this commandbuffer
    if (!disabled[command_buffer_state]) {
        cb_state->AddChild(cb_state->index_buffer_binding.buffer_state);
    }
}

void ValidationStateTracker::PreCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                               uint32_t bindingCount, const VkBuffer *pBuffers,
                                                               const VkDeviceSize *pOffsets) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_BINDVERTEXBUFFERS);

    uint32_t end = firstBinding + bindingCount;
    if (cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings.size() < end) {
        cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings.resize(end);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        auto &vertex_buffer_binding = cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings[i + firstBinding];
        vertex_buffer_binding.buffer_state = Get<BUFFER_STATE>(pBuffers[i]);
        vertex_buffer_binding.offset = pOffsets[i];
        vertex_buffer_binding.size = VK_WHOLE_SIZE;
        vertex_buffer_binding.stride = 0;
        // Add binding for this vertex buffer to this commandbuffer
        if (pBuffers[i] && !disabled[command_buffer_state]) {
            cb_state->AddChild(vertex_buffer_binding.buffer_state);
        }
    }
}

void ValidationStateTracker::PostCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                                                           VkDeviceSize dstOffset, VkDeviceSize dataSize, const void *pData) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(CMD_UPDATEBUFFER, Get<BUFFER_STATE>(dstBuffer));
}

void ValidationStateTracker::PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                                      VkPipelineStageFlags stageMask) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordSetEvent(CMD_SETEVENT, event, stageMask);
}

void ValidationStateTracker::PreCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                          const VkDependencyInfoKHR *pDependencyInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto stage_masks = sync_utils::GetGlobalStageMasks(*pDependencyInfo);

    cb_state->RecordSetEvent(CMD_SETEVENT2KHR, event, stage_masks.src);
    cb_state->RecordBarriers(*pDependencyInfo);
}

void ValidationStateTracker::PreCallRecordCmdSetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                                       const VkDependencyInfo *pDependencyInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto stage_masks = sync_utils::GetGlobalStageMasks(*pDependencyInfo);

    cb_state->RecordSetEvent(CMD_SETEVENT2, event, stage_masks.src);
    cb_state->RecordBarriers(*pDependencyInfo);
}

void ValidationStateTracker::PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                                        VkPipelineStageFlags stageMask) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordResetEvent(CMD_RESETEVENT, event, stageMask);
}

void ValidationStateTracker::PreCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                            VkPipelineStageFlags2KHR stageMask) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordResetEvent(CMD_RESETEVENT2KHR, event, stageMask);
}

void ValidationStateTracker::PreCallRecordCmdResetEvent2(VkCommandBuffer commandBuffer, VkEvent event,
                                                         VkPipelineStageFlags2 stageMask) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordResetEvent(CMD_RESETEVENT2, event, stageMask);
}

void ValidationStateTracker::PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                        VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                                        uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                        uint32_t bufferMemoryBarrierCount,
                                                        const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                        uint32_t imageMemoryBarrierCount,
                                                        const VkImageMemoryBarrier *pImageMemoryBarriers) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordWaitEvents(CMD_WAITEVENTS, eventCount, pEvents, sourceStageMask);
    cb_state->RecordBarriers(memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                             imageMemoryBarrierCount, pImageMemoryBarriers);
}

void ValidationStateTracker::PreCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount,
                                                            const VkEvent *pEvents, const VkDependencyInfoKHR *pDependencyInfos) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    for (uint32_t i = 0; i < eventCount; i++) {
        const auto &dep_info = pDependencyInfos[i];
        auto stage_masks = sync_utils::GetGlobalStageMasks(dep_info);
        cb_state->RecordWaitEvents(CMD_WAITEVENTS2KHR, 1, &pEvents[i], stage_masks.src);
        cb_state->RecordBarriers(dep_info);
    }
}

void ValidationStateTracker::PreCallRecordCmdWaitEvents2(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                         const VkDependencyInfo *pDependencyInfos) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    for (uint32_t i = 0; i < eventCount; i++) {
        const auto &dep_info = pDependencyInfos[i];
        auto stage_masks = sync_utils::GetGlobalStageMasks(dep_info);
        cb_state->RecordWaitEvents(CMD_WAITEVENTS2, 1, &pEvents[i], stage_masks.src);
        cb_state->RecordBarriers(dep_info);
    }
}

void ValidationStateTracker::PostCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                              VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                              uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                              uint32_t bufferMemoryBarrierCount,
                                                              const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                              uint32_t imageMemoryBarrierCount,
                                                              const VkImageMemoryBarrier *pImageMemoryBarriers) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_PIPELINEBARRIER);
    cb_state->RecordBarriers(memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                             imageMemoryBarrierCount, pImageMemoryBarriers);
}

void ValidationStateTracker::PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer,
                                                                 const VkDependencyInfoKHR *pDependencyInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_PIPELINEBARRIER2KHR);
    cb_state->RecordBarriers(*pDependencyInfo);
}

void ValidationStateTracker::PreCallRecordCmdPipelineBarrier2(VkCommandBuffer commandBuffer,
                                                              const VkDependencyInfo *pDependencyInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_PIPELINEBARRIER2);
    cb_state->RecordBarriers(*pDependencyInfo);
}

void ValidationStateTracker::PostCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                                         VkFlags flags) {
    if (disabled[query_validation]) return;
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);

    uint32_t num_queries = 1;
    // If render pass instance has multiview enabled, query uses N consecutive query indices
    if (cb_state->activeRenderPass) {
        uint32_t bits = cb_state->activeRenderPass->GetViewMaskBits(cb_state->activeSubpass);
        num_queries = std::max(num_queries, bits);
    }
    for (uint32_t i = 0; i < num_queries; ++i) {
        QueryObject query = {queryPool, slot};
        cb_state->RecordCmd(CMD_BEGINQUERY);
        if (!disabled[query_validation]) {
            cb_state->BeginQuery(query);
        }
        if (!disabled[command_buffer_state]) {
            auto pool_state = Get<QUERY_POOL_STATE>(query.pool);
            cb_state->AddChild(pool_state);
        }
    }
}

void ValidationStateTracker::PostCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot) {
    if (disabled[query_validation]) return;
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    uint32_t num_queries = 1;
    // If render pass instance has multiview enabled, query uses N consecutive query indices
    if (cb_state->activeRenderPass) {
        uint32_t bits = cb_state->activeRenderPass->GetViewMaskBits(cb_state->activeSubpass);
        num_queries = std::max(num_queries, bits);
    }

    for (uint32_t i = 0; i < num_queries; ++i) {
        QueryObject query_obj = {queryPool, slot + i};
        cb_state->RecordCmd(CMD_ENDQUERY);
        if (!disabled[query_validation]) {
            cb_state->EndQuery(query_obj);
        }
        if (!disabled[command_buffer_state]) {
            auto pool_state = Get<QUERY_POOL_STATE>(query_obj.pool);
            cb_state->AddChild(pool_state);
        }
    }
}

void ValidationStateTracker::PostCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                             uint32_t firstQuery, uint32_t queryCount) {
    if (disabled[query_validation]) return;
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->RecordCmd(CMD_RESETQUERYPOOL);
    cb_state->ResetQueryPool(queryPool, firstQuery, queryCount);

    if (!disabled[command_buffer_state]) {
        auto pool_state = Get<QUERY_POOL_STATE>(queryPool);
        cb_state->AddChild(pool_state);
    }
}

void ValidationStateTracker::PostCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                                   uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer,
                                                                   VkDeviceSize dstOffset, VkDeviceSize stride,
                                                                   VkQueryResultFlags flags) {
    if (disabled[query_validation] || disabled[command_buffer_state]) return;

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_COPYQUERYPOOLRESULTS);
    auto dst_buff_state = Get<BUFFER_STATE>(dstBuffer);
    cb_state->AddChild(dst_buff_state);
    auto pool_state = Get<QUERY_POOL_STATE>(queryPool);
    cb_state->AddChild(pool_state);
}

void ValidationStateTracker::PostCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                             VkQueryPool queryPool, uint32_t slot) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordWriteTimestamp(CMD_WRITETIMESTAMP, pipelineStage, queryPool, slot);
}

void ValidationStateTracker::PostCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer,
                                                                 VkPipelineStageFlags2KHR pipelineStage, VkQueryPool queryPool,
                                                                 uint32_t slot) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordWriteTimestamp(CMD_WRITETIMESTAMP2KHR, pipelineStage, queryPool, slot);
}

void ValidationStateTracker::PostCallRecordCmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 pipelineStage,
                                                              VkQueryPool queryPool, uint32_t slot) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordWriteTimestamp(CMD_WRITETIMESTAMP2, pipelineStage, queryPool, slot);
}

void ValidationStateTracker::PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) {
    if (disabled[query_validation]) return;
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_WRITEACCELERATIONSTRUCTURESPROPERTIESKHR);
    if (!disabled[command_buffer_state]) {
        auto pool_state = Get<QUERY_POOL_STATE>(queryPool);
        cb_state->AddChild(pool_state);
    }
    cb_state->EndQueries(queryPool, firstQuery, accelerationStructureCount);
}

void ValidationStateTracker::PostCallRecordCreateVideoSessionKHR(VkDevice device, const VkVideoSessionCreateInfoKHR *pCreateInfo,
                                                                 const VkAllocationCallbacks *pAllocator,
                                                                 VkVideoSessionKHR *pVideoSession, VkResult result) {
    if (VK_SUCCESS != result) return;

    auto profile_desc = video_profile_cache_.Get(this, pCreateInfo->pVideoProfile);
    Add(std::make_shared<VIDEO_SESSION_STATE>(this, *pVideoSession, pCreateInfo, std::move(profile_desc)));
}

void ValidationStateTracker::PostCallRecordGetVideoSessionMemoryRequirementsKHR(
    VkDevice device, VkVideoSessionKHR videoSession, uint32_t *pMemoryRequirementsCount,
    VkVideoSessionMemoryRequirementsKHR *pMemoryRequirements, VkResult result) {
    if (VK_SUCCESS != result) return;

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
                                                                     VkResult result) {
    if (VK_SUCCESS != result) return;

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
                                                                           VkResult result) {
    if (VK_SUCCESS != result) return;

    Add(std::make_shared<VIDEO_SESSION_PARAMETERS_STATE>(
        *pVideoSessionParameters, pCreateInfo, Get<VIDEO_SESSION_STATE>(pCreateInfo->videoSession),
        Get<VIDEO_SESSION_PARAMETERS_STATE>(pCreateInfo->videoSessionParametersTemplate)));
}

void ValidationStateTracker::PostCallRecordUpdateVideoSessionParametersKHR(VkDevice device,
                                                                           VkVideoSessionParametersKHR videoSessionParameters,
                                                                           const VkVideoSessionParametersUpdateInfoKHR *pUpdateInfo,
                                                                           VkResult result) {
    if (VK_SUCCESS != result) return;

    Get<VIDEO_SESSION_PARAMETERS_STATE>(videoSessionParameters)->Update(pUpdateInfo);
}

void ValidationStateTracker::PreCallRecordDestroyVideoSessionParametersKHR(VkDevice device,
                                                                           VkVideoSessionParametersKHR videoSessionParameters,
                                                                           const VkAllocationCallbacks *pAllocator) {
    Destroy<VIDEO_SESSION_PARAMETERS_STATE>(videoSessionParameters);
}

void ValidationStateTracker::PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer,
                                                             VkResult result) {
    if (VK_SUCCESS != result) return;

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
                                                            VkResult result) {
    if (VK_SUCCESS != result) return;
    Add(std::make_shared<RENDER_PASS_STATE>(*pRenderPass, pCreateInfo));
}

void ValidationStateTracker::PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                                VkResult result) {
    if (VK_SUCCESS != result) return;

    Add(std::make_shared<RENDER_PASS_STATE>(*pRenderPass, pCreateInfo));
}

void ValidationStateTracker::PostCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                             VkResult result) {
    if (VK_SUCCESS != result) return;

    Add(std::make_shared<RENDER_PASS_STATE>(*pRenderPass, pCreateInfo));
}

void ValidationStateTracker::PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer,
                                                             const VkRenderPassBeginInfo *pRenderPassBegin,
                                                             VkSubpassContents contents) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->BeginRenderPass(CMD_BEGINRENDERPASS, pRenderPassBegin, contents);
}

void ValidationStateTracker::PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                                 const VkRenderPassBeginInfo *pRenderPassBegin,
                                                                 const VkSubpassBeginInfo *pSubpassBeginInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->BeginRenderPass(CMD_BEGINRENDERPASS2KHR, pRenderPassBegin, pSubpassBeginInfo->contents);
}

void ValidationStateTracker::PreCallRecordCmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                                 const VkVideoBeginCodingInfoKHR *pBeginInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->BeginVideoCoding(pBeginInfo);
}

void ValidationStateTracker::PostCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                                        uint32_t counterBufferCount,
                                                                        const VkBuffer *pCounterBuffers,
                                                                        const VkDeviceSize *pCounterBufferOffsets) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->RecordCmd(CMD_BEGINTRANSFORMFEEDBACKEXT);
    cb_state->transform_feedback_active = true;
}

void ValidationStateTracker::PostCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                                      uint32_t counterBufferCount, const VkBuffer *pCounterBuffers,
                                                                      const VkDeviceSize *pCounterBufferOffsets) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->RecordCmd(CMD_ENDTRANSFORMFEEDBACKEXT);
    cb_state->transform_feedback_active = false;
}

void ValidationStateTracker::PostCallRecordCmdBeginConditionalRenderingEXT(
    VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT *pConditionalRenderingBegin) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->RecordCmd(CMD_BEGINCONDITIONALRENDERINGEXT);
    cb_state->conditional_rendering_active = true;
    cb_state->conditional_rendering_inside_render_pass = cb_state->activeRenderPass != nullptr;
    cb_state->conditional_rendering_subpass = cb_state->activeSubpass;
}

void ValidationStateTracker::PostCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->RecordCmd(CMD_ENDCONDITIONALRENDERINGEXT);
    cb_state->conditional_rendering_active = false;
    cb_state->conditional_rendering_inside_render_pass = false;
    cb_state->conditional_rendering_subpass = 0;
}

void ValidationStateTracker::RecordCmdEndRenderingRenderPassState(VkCommandBuffer commandBuffer) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->activeRenderPass = nullptr;
    cb_state->active_color_attachments_index.clear();
}

void ValidationStateTracker::PreCallRecordCmdBeginRenderingKHR(VkCommandBuffer commandBuffer,
                                                               const VkRenderingInfoKHR *pRenderingInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->BeginRendering(CMD_BEGINRENDERINGKHR, pRenderingInfo);
}

void ValidationStateTracker::PreCallRecordCmdBeginRendering(VkCommandBuffer commandBuffer, const VkRenderingInfo *pRenderingInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->BeginRendering(CMD_BEGINRENDERING, pRenderingInfo);
}

void ValidationStateTracker::PreCallRecordCmdEndRenderingKHR(VkCommandBuffer commandBuffer) {
    RecordCmdEndRenderingRenderPassState(commandBuffer);
}

void ValidationStateTracker::PreCallRecordCmdEndRendering(VkCommandBuffer commandBuffer) {
    RecordCmdEndRenderingRenderPassState(commandBuffer);
}

void ValidationStateTracker::PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer,
                                                              const VkRenderPassBeginInfo *pRenderPassBegin,
                                                              const VkSubpassBeginInfo *pSubpassBeginInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->BeginRenderPass(CMD_BEGINRENDERPASS2, pRenderPassBegin, pSubpassBeginInfo->contents);
}

void ValidationStateTracker::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->NextSubpass(CMD_NEXTSUBPASS, contents);
}

void ValidationStateTracker::PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer,
                                                              const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                              const VkSubpassEndInfo *pSubpassEndInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->NextSubpass(CMD_NEXTSUBPASS2KHR, pSubpassBeginInfo->contents);
}

void ValidationStateTracker::PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer,
                                                           const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                           const VkSubpassEndInfo *pSubpassEndInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->NextSubpass(CMD_NEXTSUBPASS2, pSubpassBeginInfo->contents);
}

void ValidationStateTracker::PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->EndRenderPass(CMD_ENDRENDERPASS);
}

void ValidationStateTracker::PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                                const VkSubpassEndInfo *pSubpassEndInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->EndRenderPass(CMD_ENDRENDERPASS2KHR);
}

void ValidationStateTracker::PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer,
                                                             const VkSubpassEndInfo *pSubpassEndInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->EndRenderPass(CMD_ENDRENDERPASS2);
}

void ValidationStateTracker::PostCallRecordCmdEndVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                                const VkVideoEndCodingInfoKHR *pEndCodingInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->EndVideoCoding(pEndCodingInfo);
}

void ValidationStateTracker::PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount,
                                                             const VkCommandBuffer *pCommandBuffers) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->ExecuteCommands({pCommandBuffers, commandBuffersCount});
}

void ValidationStateTracker::PostCallRecordMapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size,
                                                     VkFlags flags, void **ppData, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordMappedMemory(mem, offset, size, ppData);
}

void ValidationStateTracker::PreCallRecordUnmapMemory(VkDevice device, VkDeviceMemory mem) {
    auto mem_info = Get<DEVICE_MEMORY_STATE>(mem);
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
        const auto swapchain_info = LvlFindInChain<VkBindImageMemorySwapchainInfoKHR>(bindInfo.pNext);
        if (swapchain_info) {
            auto swapchain = Get<SWAPCHAIN_NODE>(swapchain_info->swapchain);
            if (swapchain) {
                SWAPCHAIN_IMAGE &swapchain_image = swapchain->images[swapchain_info->imageIndex];

                if (!swapchain_image.fake_base_address) {
                    auto size = image_state->fragment_encoder->TotalSize();
                    swapchain_image.fake_base_address = fake_memory.Alloc(size);
                }
                // All images bound to this swapchain and index are aliases
                image_state->SetSwapchain(swapchain, swapchain_info->imageIndex);
            }
        } else {
            // Track bound memory range information
            auto mem_info = Get<DEVICE_MEMORY_STATE>(bindInfo.memory);
            if (mem_info) {
                VkDeviceSize plane_index = 0u;
                if (image_state->disjoint && image_state->IsExternalAHB() == false) {
                    auto plane_info = LvlFindInChain<VkBindImagePlaneMemoryInfo>(bindInfo.pNext);
                    const VkImageAspectFlagBits aspect = plane_info->planeAspect;
                    switch (aspect) {
                        case VK_IMAGE_ASPECT_PLANE_0_BIT:
                            plane_index = 0;
                            break;
                        case VK_IMAGE_ASPECT_PLANE_1_BIT:
                            plane_index = 1;
                            break;
                        case VK_IMAGE_ASPECT_PLANE_2_BIT:
                            plane_index = 2;
                            break;
                        default:
                            assert(false);  // parameter validation should have caught this
                            break;
                    }
                }
                image_state->BindMemory(
                    image_state.get(), mem_info, bindInfo.memoryOffset, plane_index,
                    image_state->requirements[static_cast<decltype(image_state->requirements)::size_type>(plane_index)].size);
            }
        }
    }
}

void ValidationStateTracker::PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem,
                                                           VkDeviceSize memoryOffset, VkResult result) {
    if (VK_SUCCESS != result) return;
    auto bind_info = LvlInitStruct<VkBindImageMemoryInfo>();
    bind_info.image = image;
    bind_info.memory = mem;
    bind_info.memoryOffset = memoryOffset;
    UpdateBindImageMemoryState(bind_info);
}

void ValidationStateTracker::PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                                            const VkBindImageMemoryInfo *pBindInfos, VkResult result) {
    if (VK_SUCCESS != result) return;
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        UpdateBindImageMemoryState(pBindInfos[i]);
    }
}

void ValidationStateTracker::PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                               const VkBindImageMemoryInfo *pBindInfos, VkResult result) {
    if (VK_SUCCESS != result) return;
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
                                                                VkResult result) {
    if (VK_SUCCESS != result) return;
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
    VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR *pImportSemaphoreWin32HandleInfo, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordImportSemaphoreState(pImportSemaphoreWin32HandleInfo->semaphore, pImportSemaphoreWin32HandleInfo->handleType,
                               pImportSemaphoreWin32HandleInfo->flags);
}

void ValidationStateTracker::PostCallRecordGetSemaphoreWin32HandleKHR(VkDevice device,
                                                                      const VkSemaphoreGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                                      HANDLE *pHandle, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordGetExternalSemaphoreState(pGetWin32HandleInfo->semaphore, pGetWin32HandleInfo->handleType);
}

void ValidationStateTracker::PostCallRecordImportFenceWin32HandleKHR(
    VkDevice device, const VkImportFenceWin32HandleInfoKHR *pImportFenceWin32HandleInfo, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordImportFenceState(pImportFenceWin32HandleInfo->fence, pImportFenceWin32HandleInfo->handleType,
                           pImportFenceWin32HandleInfo->flags);
}

void ValidationStateTracker::PostCallRecordGetFenceWin32HandleKHR(VkDevice device,
                                                                  const VkFenceGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                                  HANDLE *pHandle, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordGetExternalFenceState(pGetWin32HandleInfo->fence, pGetWin32HandleInfo->handleType);
}
#endif

void ValidationStateTracker::PostCallRecordGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR *pGetFdInfo, int *pFd,
                                                             VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordGetExternalSemaphoreState(pGetFdInfo->semaphore, pGetFdInfo->handleType);
}

void ValidationStateTracker::RecordImportFenceState(VkFence fence, VkExternalFenceHandleTypeFlagBits handle_type,
                                                    VkFenceImportFlags flags) {
    auto fence_node = Get<FENCE_STATE>(fence);

    if (fence_node) {
        fence_node->Import(handle_type, flags);
    }
}

void ValidationStateTracker::PostCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *pImportFenceFdInfo,
                                                            VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordImportFenceState(pImportFenceFdInfo->fence, pImportFenceFdInfo->handleType, pImportFenceFdInfo->flags);
}

void ValidationStateTracker::RecordGetExternalFenceState(VkFence fence, VkExternalFenceHandleTypeFlagBits handle_type) {
    auto fence_state = Get<FENCE_STATE>(fence);
    if (fence_state) {
        fence_state->Export(handle_type);
    }
}

void ValidationStateTracker::PostCallRecordGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *pGetFdInfo, int *pFd,
                                                         VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordGetExternalFenceState(pGetFdInfo->fence, pGetFdInfo->handleType);
}

void ValidationStateTracker::PostCallRecordCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkEvent *pEvent, VkResult result) {
    if (VK_SUCCESS != result) return;
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
        auto swapchain_present_modes_ci = LvlFindInChain<VkSwapchainPresentModesCreateInfoEXT>(pCreateInfo->pNext);
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
                                                              VkResult result) {
    auto surface_state = Get<SURFACE_STATE>(pCreateInfo->surface);
    auto old_swapchain_state = Get<SWAPCHAIN_NODE>(pCreateInfo->oldSwapchain);
    RecordCreateSwapchainState(result, pCreateInfo, pSwapchain, std::move(surface_state), old_swapchain_state.get());
}

void ValidationStateTracker::PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                              const VkAllocationCallbacks *pAllocator) {
    Destroy<SWAPCHAIN_NODE>(swapchain);
}

void ValidationStateTracker::PostCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                                const VkDisplayModeCreateInfoKHR *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator, VkDisplayModeKHR *pMode,
                                                                VkResult result) {
    if (VK_SUCCESS != result) return;
    if (!pMode) return;
    Add(std::make_shared<DISPLAY_MODE_STATE>(*pMode, physicalDevice));
}

void ValidationStateTracker::PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo, VkResult result) {
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
    if (result == VK_ERROR_OUT_OF_HOST_MEMORY || result == VK_ERROR_OUT_OF_DEVICE_MEMORY || result == VK_ERROR_DEVICE_LOST) {
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

    const auto *present_id_info = LvlFindInChain<VkPresentIdKHR>(pPresentInfo->pNext);
    for (uint32_t i = 0; i < pPresentInfo->swapchainCount; ++i) {
        // Note: this is imperfect, in that we can get confused about what did or didn't succeed-- but if the app does that, it's
        // confused itself just as much.
        auto local_result = pPresentInfo->pResults ? pPresentInfo->pResults[i] : result;
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
                                                                     VkSwapchainKHR *pSwapchains, VkResult result) {
    if (pCreateInfos) {
        for (uint32_t i = 0; i < swapchainCount; i++) {
            auto surface_state = Get<SURFACE_STATE>(pCreateInfos[i].surface);
            auto old_swapchain_state = Get<SWAPCHAIN_NODE>(pCreateInfos[i].oldSwapchain);
            RecordCreateSwapchainState(result, &pCreateInfos[i], &pSwapchains[i], std::move(surface_state),
                                       old_swapchain_state.get());
        }
    }
}

void ValidationStateTracker::RecordAcquireNextImageState(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                         VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex) {
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
        semaphore_state->EnqueueAcquire();
    }

    // Mark the image as acquired.
    auto swapchain_data = Get<SWAPCHAIN_NODE>(swapchain);
    if (swapchain_data) {
        swapchain_data->AcquireImage(*pImageIndex);
    }
}

void ValidationStateTracker::PostCallRecordAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                               VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex,
                                                               VkResult result) {
    if ((VK_SUCCESS != result) && (VK_SUBOPTIMAL_KHR != result)) return;
    RecordAcquireNextImageState(device, swapchain, timeout, semaphore, fence, pImageIndex);
}

void ValidationStateTracker::PostCallRecordAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR *pAcquireInfo,
                                                                uint32_t *pImageIndex, VkResult result) {
    if ((VK_SUCCESS != result) && (VK_SUBOPTIMAL_KHR != result)) return;
    RecordAcquireNextImageState(device, pAcquireInfo->swapchain, pAcquireInfo->timeout, pAcquireInfo->semaphore,
                                pAcquireInfo->fence, pImageIndex);
}

std::shared_ptr<PHYSICAL_DEVICE_STATE> ValidationStateTracker::CreatePhysicalDeviceState(VkPhysicalDevice phys_dev) {
    return std::make_shared<PHYSICAL_DEVICE_STATE>(phys_dev);
}

void ValidationStateTracker::PostCallRecordCreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                                                          const VkAllocationCallbacks *pAllocator, VkInstance *pInstance,
                                                          VkResult result) {
    if (result != VK_SUCCESS) {
        return;
    }
    instance_state = this;
    uint32_t count = 0;
    // this can fail if the allocator fails
    result = DispatchEnumeratePhysicalDevices(*pInstance, &count, nullptr);
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
    auto export_metal_object_info = LvlFindInChain<VkExportMetalObjectCreateInfoEXT>(pCreateInfo->pNext);
    while (export_metal_object_info) {
        export_metal_flags.push_back(export_metal_object_info->exportObjectType);
        export_metal_object_info = LvlFindInChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
#endif  // VK_USE_PLATFORM_METAL_EXT
}

// Common function to update state for GetPhysicalDeviceQueueFamilyProperties & 2KHR version
static void StateUpdateCommonGetPhysicalDeviceQueueFamilyProperties(PHYSICAL_DEVICE_STATE *pd_state, uint32_t count) {
    pd_state->queue_family_known_count = std::max(pd_state->queue_family_known_count, count);
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                                  uint32_t *pQueueFamilyPropertyCount,
                                                                                  VkQueueFamilyProperties *pQueueFamilyProperties) {
    auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
    assert(pd_state);
    StateUpdateCommonGetPhysicalDeviceQueueFamilyProperties(pd_state.get(), *pQueueFamilyPropertyCount);
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(
    VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties2 *pQueueFamilyProperties) {
    auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
    assert(pd_state);
    StateUpdateCommonGetPhysicalDeviceQueueFamilyProperties(pd_state.get(), *pQueueFamilyPropertyCount);
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties2 *pQueueFamilyProperties) {
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
                                                                        VkSurfaceKHR *pSurface, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
void ValidationStateTracker::PostCallRecordCreateAndroidSurfaceKHR(VkInstance instance,
                                                                   const VkAndroidSurfaceCreateInfoKHR *pCreateInfo,
                                                                   const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                                   VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_FUCHSIA
void ValidationStateTracker::PostCallRecordCreateImagePipeSurfaceFUCHSIA(VkInstance instance,
                                                                         const VkImagePipeSurfaceCreateInfoFUCHSIA *pCreateInfo,
                                                                         const VkAllocationCallbacks *pAllocator,
                                                                         VkSurfaceKHR *pSurface, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_IOS_MVK
void ValidationStateTracker::PostCallRecordCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                               VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_IOS_MVK

#ifdef VK_USE_PLATFORM_MACOS_MVK
void ValidationStateTracker::PostCallRecordCreateMacOSSurfaceMVK(VkInstance instance,
                                                                 const VkMacOSSurfaceCreateInfoMVK *pCreateInfo,
                                                                 const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                                 VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_MACOS_MVK

#ifdef VK_USE_PLATFORM_METAL_EXT
void ValidationStateTracker::PostCallRecordCreateMetalSurfaceEXT(VkInstance instance,
                                                                 const VkMetalSurfaceCreateInfoEXT *pCreateInfo,
                                                                 const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                                 VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_METAL_EXT

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
void ValidationStateTracker::PostCallRecordCreateWaylandSurfaceKHR(VkInstance instance,
                                                                   const VkWaylandSurfaceCreateInfoKHR *pCreateInfo,
                                                                   const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                                   VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR
void ValidationStateTracker::PostCallRecordCreateWin32SurfaceKHR(VkInstance instance,
                                                                 const VkWin32SurfaceCreateInfoKHR *pCreateInfo,
                                                                 const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                                 VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR
void ValidationStateTracker::PostCallRecordCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                               VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_XLIB_KHR
void ValidationStateTracker::PostCallRecordCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                                VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}
#endif  // VK_USE_PLATFORM_XLIB_KHR

void ValidationStateTracker::PostCallRecordCreateHeadlessSurfaceEXT(VkInstance instance,
                                                                    const VkHeadlessSurfaceCreateInfoEXT *pCreateInfo,
                                                                    const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface,
                                                                    VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordVulkanSurface(pSurface);
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice,
                                                                                   VkSurfaceKHR surface,
                                                                                   VkSurfaceCapabilitiesKHR *pSurfaceCapabilities,
                                                                                   VkResult result) {
    if (VK_SUCCESS != result) return;
    auto surface_state = Get<SURFACE_STATE>(surface);
    surface_state->SetCapabilities(physicalDevice, *pSurfaceCapabilities);
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
    VkSurfaceCapabilities2KHR *pSurfaceCapabilities, VkResult result) {
    if (VK_SUCCESS != result) return;

    if (pSurfaceInfo->surface) {
        auto surface_state = Get<SURFACE_STATE>(pSurfaceInfo->surface);

        const VkSurfacePresentModeEXT *surface_present_mode = LvlFindInChain<VkSurfacePresentModeEXT>(pSurfaceInfo->pNext);
        if ((!IsExtEnabled(device_extensions.vk_ext_surface_maintenance1)) || (!surface_present_mode)) {
            surface_state->SetCapabilities(physicalDevice, pSurfaceCapabilities->surfaceCapabilities);
        } else {
            const VkSurfacePresentScalingCapabilitiesEXT *present_scaling_caps =
                LvlFindInChain<VkSurfacePresentScalingCapabilitiesEXT>(pSurfaceCapabilities->pNext);
            const VkSurfacePresentModeCompatibilityEXT *compatible_modes =
                LvlFindInChain<VkSurfacePresentModeCompatibilityEXT>(pSurfaceCapabilities->pNext);

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
               LvlFindInChain<VkSurfaceProtectedCapabilitiesKHR>(pSurfaceCapabilities->pNext)) {
        auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
        assert(pd_state);
        pd_state->surfaceless_query_state.capabilities = pSurfaceCapabilities->surfaceCapabilities;
    }
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice,
                                                                                    VkSurfaceKHR surface,
                                                                                    VkSurfaceCapabilities2EXT *pSurfaceCapabilities,
                                                                                    VkResult result) {
    auto surface_state = Get<SURFACE_STATE>(surface);
    VkSurfaceCapabilitiesKHR caps{
        pSurfaceCapabilities->minImageCount,           pSurfaceCapabilities->maxImageCount,
        pSurfaceCapabilities->currentExtent,           pSurfaceCapabilities->minImageExtent,
        pSurfaceCapabilities->maxImageExtent,          pSurfaceCapabilities->maxImageArrayLayers,
        pSurfaceCapabilities->supportedTransforms,     pSurfaceCapabilities->currentTransform,
        pSurfaceCapabilities->supportedCompositeAlpha, pSurfaceCapabilities->supportedUsageFlags,
    };
    surface_state->SetCapabilities(physicalDevice, caps);
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice,
                                                                              uint32_t queueFamilyIndex, VkSurfaceKHR surface,
                                                                              VkBool32 *pSupported, VkResult result) {
    if (VK_SUCCESS != result) return;
    auto surface_state = Get<SURFACE_STATE>(surface);
    surface_state->SetQueueSupport(physicalDevice, queueFamilyIndex, (*pSupported == VK_TRUE));
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice,
                                                                                   VkSurfaceKHR surface,
                                                                                   uint32_t *pPresentModeCount,
                                                                                   VkPresentModeKHR *pPresentModes,
                                                                                   VkResult result) {
    if ((VK_SUCCESS != result) && (VK_INCOMPLETE != result)) return;

    if (pPresentModes) {
        if (surface) {
            auto surface_state = Get<SURFACE_STATE>(surface);
            surface_state->SetPresentModes(physicalDevice,
                                           vvl::span<const VkPresentModeKHR>(pPresentModes, *pPresentModeCount));
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
                                                                              VkResult result) {
    if ((VK_SUCCESS != result) && (VK_INCOMPLETE != result)) return;

    if (pSurfaceFormats) {
        if (surface) {
            auto surface_state = Get<SURFACE_STATE>(surface);
            surface_state->SetFormats(physicalDevice,
                                      std::vector<VkSurfaceFormatKHR>(pSurfaceFormats, pSurfaceFormats + *pSurfaceFormatCount));
        } else if (IsExtEnabled(instance_extensions.vk_google_surfaceless_query)) {
            auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
            assert(pd_state);
            pd_state->surfaceless_query_state.formats =
                std::vector<VkSurfaceFormatKHR>(pSurfaceFormats, pSurfaceFormats + *pSurfaceFormatCount);
        }
    }
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice,
                                                                               const VkPhysicalDeviceSurfaceInfo2KHR *pSurfaceInfo,
                                                                               uint32_t *pSurfaceFormatCount,
                                                                               VkSurfaceFormat2KHR *pSurfaceFormats,
                                                                               VkResult result) {
    if ((VK_SUCCESS != result) && (VK_INCOMPLETE != result)) return;

    if (pSurfaceFormats) {
        std::vector<VkSurfaceFormatKHR> fmts(*pSurfaceFormatCount);
        for (uint32_t i = 0; i < *pSurfaceFormatCount; i++) {
            fmts[i] = pSurfaceFormats[i].surfaceFormat;
        }
        if (pSurfaceInfo->surface) {
            auto surface_state = Get<SURFACE_STATE>(pSurfaceInfo->surface);
            surface_state->SetFormats(physicalDevice, std::move(fmts));
        } else if (IsExtEnabled(instance_extensions.vk_google_surfaceless_query)) {
            auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
            assert(pd_state);
            pd_state->surfaceless_query_state.formats = std::move(fmts);
        }
    }
}

void ValidationStateTracker::PreCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                                                                     const VkDebugUtilsLabelEXT *pLabelInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_BEGINDEBUGUTILSLABELEXT);
    BeginCmdDebugUtilsLabel(report_data, commandBuffer, pLabelInfo);
}

void ValidationStateTracker::PostCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_ENDDEBUGUTILSLABELEXT);
    EndCmdDebugUtilsLabel(report_data, commandBuffer);
}

void ValidationStateTracker::PreCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                                                                      const VkDebugUtilsLabelEXT *pLabelInfo) {
    InsertCmdDebugUtilsLabel(report_data, commandBuffer, pLabelInfo);

    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_INSERTDEBUGUTILSLABELEXT);
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
    VkPerformanceCounterDescriptionKHR *pCounterDescriptions, VkResult result) {
    if ((VK_SUCCESS != result) && (VK_INCOMPLETE != result)) return;
    RecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCounters(physicalDevice, queueFamilyIndex, pCounterCount, pCounters);
}

void ValidationStateTracker::PostCallRecordAcquireProfilingLockKHR(VkDevice device, const VkAcquireProfilingLockInfoKHR *pInfo,
                                                                   VkResult result) {
    if (result == VK_SUCCESS) performance_lock_acquired = true;
}

void ValidationStateTracker::PostCallRecordReleaseProfilingLockKHR(VkDevice device) {
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
                                                                          VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordCreateDescriptorUpdateTemplateState(pCreateInfo, pDescriptorUpdateTemplate);
}

void ValidationStateTracker::PostCallRecordCreateDescriptorUpdateTemplateKHR(
    VkDevice device, const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
    VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate, VkResult result) {
    if (VK_SUCCESS != result) return;
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

    cb_state->RecordCmd(CMD_PUSHDESCRIPTORSETWITHTEMPLATEKHR);
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
                                                                                      VkResult result) {
    if ((VK_SUCCESS != result) && (VK_INCOMPLETE != result)) return;
    RecordGetPhysicalDeviceDisplayPlanePropertiesState(physicalDevice, pPropertyCount, pProperties);
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                                       uint32_t *pPropertyCount,
                                                                                       VkDisplayPlaneProperties2KHR *pProperties,
                                                                                       VkResult result) {
    if ((VK_SUCCESS != result) && (VK_INCOMPLETE != result)) return;
    RecordGetPhysicalDeviceDisplayPlanePropertiesState(physicalDevice, pPropertyCount, pProperties);
}

void ValidationStateTracker::PostCallRecordCmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                                   uint32_t query, VkQueryControlFlags flags, uint32_t index) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    uint32_t num_queries = 1;
    // If render pass instance has multiview enabled, query uses N consecutive query indices
    if (cb_state->activeRenderPass) {
        uint32_t bits = cb_state->activeRenderPass->GetViewMaskBits(cb_state->activeSubpass);
        num_queries = std::max(num_queries, bits);
    }

    for (uint32_t i = 0; i < num_queries; ++i) {
        QueryObject query_obj = {queryPool, query, index + i};
        cb_state->RecordCmd(CMD_BEGINQUERYINDEXEDEXT);
        cb_state->BeginQuery(query_obj);
    }
}

void ValidationStateTracker::PostCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                                 uint32_t query, uint32_t index) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    uint32_t num_queries = 1;
    // If render pass instance has multiview enabled, query uses N consecutive query indices
    if (cb_state->activeRenderPass) {
        uint32_t bits = cb_state->activeRenderPass->GetViewMaskBits(cb_state->activeSubpass);
        num_queries = std::max(num_queries, bits);
    }

    for (uint32_t i = 0; i < num_queries; ++i) {
        QueryObject query_obj = {queryPool, query, index + i};
        cb_state->RecordCmd(CMD_ENDQUERYINDEXEDEXT);
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
                                                                        VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordCreateSamplerYcbcrConversionState(pCreateInfo, *pYcbcrConversion);
}

void ValidationStateTracker::PostCallRecordCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                                           const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                                           const VkAllocationCallbacks *pAllocator,
                                                                           VkSamplerYcbcrConversion *pYcbcrConversion,
                                                                           VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordCreateSamplerYcbcrConversionState(pCreateInfo, *pYcbcrConversion);
}

void ValidationStateTracker::PostCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                                         const VkAllocationCallbacks *pAllocator) {
    Destroy<SAMPLER_YCBCR_CONVERSION_STATE>(ycbcrConversion);
}

void ValidationStateTracker::PostCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device,
                                                                            VkSamplerYcbcrConversion ycbcrConversion,
                                                                            const VkAllocationCallbacks *pAllocator) {
    Destroy<SAMPLER_YCBCR_CONVERSION_STATE>(ycbcrConversion);
}

void ValidationStateTracker::RecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                  uint32_t queryCount) {
    // Do nothing if the feature is not enabled.
    if (!enabled_features.core12.hostQueryReset) return;

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
                                                             uint32_t queryCount) {
    RecordResetQueryPool(device, queryPool, firstQuery, queryCount);
}

void ValidationStateTracker::PostCallRecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                          uint32_t queryCount) {
    RecordResetQueryPool(device, queryPool, firstQuery, queryCount);
}

void ValidationStateTracker::PerformUpdateDescriptorSetsWithTemplateKHR(VkDescriptorSet descriptorSet,
                                                                        const UPDATE_TEMPLATE_STATE *template_state,
                                                                        const void *pData) {
    // Translate the templated update into a normal update for validation...
    cvdescriptorset::DecodedTemplateUpdate decoded_update(this, descriptorSet, template_state, pData);
    cvdescriptorset::PerformUpdateDescriptorSets(this, static_cast<uint32_t>(decoded_update.desc_writes.size()),
                                                 decoded_update.desc_writes.data(), 0, NULL);
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
                                                   uint32_t firstVertex, uint32_t firstInstance) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDrawCmd(CMD_DRAW);
}

void ValidationStateTracker::PostCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                           const VkMultiDrawInfoEXT *pVertexInfo, uint32_t instanceCount,
                                                           uint32_t firstInstance, uint32_t stride) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDrawCmd(CMD_DRAWMULTIEXT);
}

void ValidationStateTracker::PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount,
                                                          uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                                                          uint32_t firstInstance) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDrawCmd(CMD_DRAWINDEXED);
}

void ValidationStateTracker::PostCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                                  const VkMultiDrawIndexedInfoEXT *pIndexInfo,
                                                                  uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                                                  const int32_t *pVertexOffset) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDrawCmd(CMD_DRAWMULTIINDEXEDEXT);
}

void ValidationStateTracker::PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           uint32_t count, uint32_t stride) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    cb_state->UpdateDrawCmd(CMD_DRAWINDIRECT);
    if (!disabled[command_buffer_state]) {
        cb_state->AddChild(buffer_state);
    }
}

void ValidationStateTracker::PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, uint32_t count, uint32_t stride) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    cb_state->UpdateDrawCmd(CMD_DRAWINDEXEDINDIRECT);
    if (!disabled[command_buffer_state]) {
        cb_state->AddChild(buffer_state);
    }
}

void ValidationStateTracker::PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDispatchCmd(CMD_DISPATCH);
}

void ValidationStateTracker::PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                               VkDeviceSize offset) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDispatchCmd(CMD_DISPATCHINDIRECT);
    if (!disabled[command_buffer_state]) {
        auto buffer_state = Get<BUFFER_STATE>(buffer);
        cb_state->AddChild(buffer_state);
    }
}

void ValidationStateTracker::PostCallRecordCmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t, uint32_t, uint32_t, uint32_t,
                                                              uint32_t, uint32_t) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDispatchCmd(CMD_DISPATCHBASEKHR);
}

void ValidationStateTracker::PostCallRecordCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t, uint32_t, uint32_t, uint32_t,
                                                           uint32_t, uint32_t) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDispatchCmd(CMD_DISPATCHBASE);
}

void ValidationStateTracker::RecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride, CMD_TYPE cmd_type) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDrawCmd(cmd_type);
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
                               CMD_DRAWINDIRECTCOUNTKHR);
}

void ValidationStateTracker::PreCallRecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride) {
    RecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                               CMD_DRAWINDIRECTCOUNT);
}

void ValidationStateTracker::RecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                               VkBuffer countBuffer, VkDeviceSize countBufferOffset,
                                                               uint32_t maxDrawCount, uint32_t stride, CMD_TYPE cmd_type) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDrawCmd(cmd_type);
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
                                      CMD_DRAWINDEXEDINDIRECTCOUNTKHR);
}

void ValidationStateTracker::PreCallRecordCmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                      VkDeviceSize offset, VkBuffer countBuffer,
                                                                      VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                      uint32_t stride) {
    RecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride,
                                      CMD_DRAWINDEXEDINDIRECTCOUNT);
}

void ValidationStateTracker::PreCallRecordCmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount,
                                                             uint32_t firstTask) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDrawCmd(CMD_DRAWMESHTASKSNV);
}

void ValidationStateTracker::PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                     VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDrawCmd(CMD_DRAWMESHTASKSINDIRECTNV);
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
    cb_state->UpdateDrawCmd(CMD_DRAWMESHTASKSINDIRECTCOUNTNV);
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
    cb_state->UpdateDrawCmd(CMD_DRAWMESHTASKSEXT);
}

void ValidationStateTracker::PreCallRecordCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                      VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateDrawCmd(CMD_DRAWMESHTASKSINDIRECTEXT);
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
    cb_state->UpdateDrawCmd(CMD_DRAWMESHTASKSINDIRECTCOUNTEXT);
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
    uint32_t width, uint32_t height, uint32_t depth) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateTraceRayCmd(CMD_TRACERAYSNV);
}

void ValidationStateTracker::PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                                           const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                           const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                           const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                           const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                           uint32_t width, uint32_t height, uint32_t depth) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateTraceRayCmd(CMD_TRACERAYSKHR);
}

void ValidationStateTracker::PostCallRecordCmdTraceRaysIndirectKHR(
    VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
    const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, VkDeviceAddress indirectDeviceAddress) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateTraceRayCmd(CMD_TRACERAYSINDIRECTKHR);
}

std::shared_ptr<SHADER_MODULE_STATE> ValidationStateTracker::CreateShaderModuleState(const VkShaderModuleCreateInfo &create_info,
                                                                                     uint32_t unique_shader_id,
                                                                                     VkShaderModule handle) const {
    spv_target_env spirv_environment = PickSpirvEnv(api_version, IsExtEnabled(device_extensions.vk_khr_spirv_1_4));
    const bool is_spirv = (create_info.pCode[0] == spv::MagicNumber);
    return is_spirv ? std::make_shared<SHADER_MODULE_STATE>(create_info, handle, spirv_environment, unique_shader_id)
                    : std::make_shared<SHADER_MODULE_STATE>();
}

void ValidationStateTracker::PostCallRecordCmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer,
                                                                    VkDeviceAddress indirectDeviceAddress) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateTraceRayCmd(CMD_TRACERAYSINDIRECT2KHR);
}

void ValidationStateTracker::PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkShaderModule *pShaderModule, VkResult result,
                                                              void *csm_state_data) {
    if (VK_SUCCESS != result) return;
    create_shader_module_api_state *csm_state = reinterpret_cast<create_shader_module_api_state *>(csm_state_data);

    Add(CreateShaderModuleState(*pCreateInfo, csm_state->unique_shader_id, *pShaderModule));
}

void ValidationStateTracker::PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                                 uint32_t *pSwapchainImageCount, VkImage *pSwapchainImages,
                                                                 VkResult result) {
    if ((result != VK_SUCCESS) && (result != VK_INCOMPLETE)) return;
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
            if (!swapchain_image.fake_base_address) {
                auto size = image_state->fragment_encoder->TotalSize();
                swapchain_image.fake_base_address = fake_memory.Alloc(size);
            }

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
                                                                        VkResult result) {
    auto src_as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfo->src);
    auto dst_as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfo->dst);
    if (dst_as_state != nullptr && src_as_state != nullptr) {
        dst_as_state->built = true;
        dst_as_state->build_info_khr = src_as_state->build_info_khr;
    }
}

void ValidationStateTracker::PostCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                           const VkCopyAccelerationStructureInfoKHR *pInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_state) {
        cb_state->RecordCmd(CMD_COPYACCELERATIONSTRUCTUREKHR);
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
    VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR *pInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_state) {
        cb_state->RecordCmd(CMD_COPYACCELERATIONSTRUCTURETOMEMORYKHR);
        auto src_as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfo->src);
        if (!disabled[command_buffer_state]) {
            cb_state->AddChild(src_as_state);
        }
        auto dst_buffers = GetBuffersByAddress(pInfo->dst.deviceAddress);
        if (!dst_buffers.empty()) {
            cb_state->AddChildren(dst_buffers);
        }
    }
}

void ValidationStateTracker::PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR *pInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_state) {
        cb_state->RecordCmd(CMD_COPYMEMORYTOACCELERATIONSTRUCTUREKHR);
        if (!disabled[command_buffer_state]) {
            auto src_buffers = GetBuffersByAddress(pInfo->src.deviceAddress);
            if (!src_buffers.empty()) {
                cb_state->AddChildren(src_buffers);
            }
            auto dst_as_state = Get<ACCELERATION_STRUCTURE_STATE_KHR>(pInfo->dst);
            cb_state->AddChild(dst_as_state);
        }
    }
}

void ValidationStateTracker::RecordCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode, CMD_TYPE cmd_type) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(cmd_type, CB_DYNAMIC_CULL_MODE_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {
    RecordCmdSetCullMode(commandBuffer, cullMode, CMD_SETCULLMODEEXT);
}

void ValidationStateTracker::PostCallRecordCmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {
    RecordCmdSetCullMode(commandBuffer, cullMode, CMD_SETCULLMODE);
}

void ValidationStateTracker::RecordCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace, CMD_TYPE cmd_type) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(cmd_type, CB_DYNAMIC_FRONT_FACE_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {
    RecordCmdSetFrontFace(commandBuffer, frontFace, CMD_SETFRONTFACEEXT);
}

void ValidationStateTracker::PostCallRecordCmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {
    RecordCmdSetFrontFace(commandBuffer, frontFace, CMD_SETFRONTFACE);
}

void ValidationStateTracker::RecordCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology,
                                                           CMD_TYPE cmd_type) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(cmd_type, CB_DYNAMIC_PRIMITIVE_TOPOLOGY_SET);
    cb_state->dynamic_state_value.primitive_topology = primitiveTopology;
}

void ValidationStateTracker::PostCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer,
                                                                      VkPrimitiveTopology primitiveTopology) {
    RecordCmdSetPrimitiveTopology(commandBuffer, primitiveTopology, CMD_SETPRIMITIVETOPOLOGYEXT);
}

void ValidationStateTracker::PostCallRecordCmdSetPrimitiveTopology(VkCommandBuffer commandBuffer,
                                                                   VkPrimitiveTopology primitiveTopology) {
    RecordCmdSetPrimitiveTopology(commandBuffer, primitiveTopology, CMD_SETPRIMITIVETOPOLOGY);
}

void ValidationStateTracker::RecordCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                           const VkViewport *pViewports, CMD_TYPE cmd_type) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(cmd_type, CB_DYNAMIC_VIEWPORT_WITH_COUNT_SET);
    uint32_t bits = (1u << viewportCount) - 1u;
    cb_state->viewportWithCountMask |= bits;
    cb_state->trashedViewportMask &= ~bits;
    cb_state->viewportWithCountCount = viewportCount;
    cb_state->trashedViewportCount = false;

    cb_state->dynamicViewports.resize(std::max(size_t(viewportCount), cb_state->dynamicViewports.size()));
    for (size_t i = 0; i < viewportCount; ++i) {
        cb_state->dynamicViewports[i] = pViewports[i];
    }
}

void ValidationStateTracker::PostCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                                      const VkViewport *pViewports) {
    RecordCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports, CMD_SETVIEWPORTWITHCOUNTEXT);
}

void ValidationStateTracker::PostCallRecordCmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                                   const VkViewport *pViewports) {
    RecordCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports, CMD_SETVIEWPORTWITHCOUNT);
}

void ValidationStateTracker::RecordCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                          const VkRect2D *pScissors, CMD_TYPE cmd_type) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(cmd_type, CB_DYNAMIC_SCISSOR_WITH_COUNT_SET);
    uint32_t bits = (1u << scissorCount) - 1u;
    cb_state->scissorWithCountMask |= bits;
    cb_state->trashedScissorMask &= ~bits;
    cb_state->scissorWithCountCount = scissorCount;
    cb_state->trashedScissorCount = false;
}

void ValidationStateTracker::PostCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                                     const VkRect2D *pScissors) {
    RecordCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors, CMD_SETSCISSORWITHCOUNTEXT);
}

void ValidationStateTracker::PostCallRecordCmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                                  const VkRect2D *pScissors) {
    RecordCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors, CMD_SETSCISSORWITHCOUNT);
}

void ValidationStateTracker::RecordCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                         uint32_t bindingCount, const VkBuffer *pBuffers,
                                                         const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                         const VkDeviceSize *pStrides, CMD_TYPE cmd_type) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    if (pStrides) {
        cb_state->RecordStateCmd(cmd_type, CB_DYNAMIC_VERTEX_INPUT_BINDING_STRIDE_SET);
    }

    uint32_t end = firstBinding + bindingCount;
    if (cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings.size() < end) {
        cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings.resize(end);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        auto &vertex_buffer_binding = cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings[i + firstBinding];
        vertex_buffer_binding.buffer_state = Get<BUFFER_STATE>(pBuffers[i]);
        vertex_buffer_binding.offset = pOffsets[i];
        vertex_buffer_binding.size = (pSizes) ? pSizes[i] : VK_WHOLE_SIZE;
        vertex_buffer_binding.stride = (pStrides) ? pStrides[i] : 0;
        // Add binding for this vertex buffer to this commandbuffer
        if (!disabled[command_buffer_state] && pBuffers[i]) {
            cb_state->AddChild(vertex_buffer_binding.buffer_state);
        }
    }
}

void ValidationStateTracker::PostCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                    uint32_t bindingCount, const VkBuffer *pBuffers,
                                                                    const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                                    const VkDeviceSize *pStrides) {
    RecordCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides,
                                CMD_BINDVERTEXBUFFERS2EXT);
}

void ValidationStateTracker::PostCallRecordCmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                 uint32_t bindingCount, const VkBuffer *pBuffers,
                                                                 const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                                 const VkDeviceSize *pStrides) {
    RecordCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides,
                                CMD_BINDVERTEXBUFFERS2);
}

void ValidationStateTracker::RecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable,
                                                         CMD_TYPE cmd_type) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(cmd_type, CB_DYNAMIC_DEPTH_TEST_ENABLE_SET);
    cb_state->dynamic_state_value.depth_test_enable = depthTestEnable;
}

void ValidationStateTracker::PostCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    RecordCmdSetDepthTestEnable(commandBuffer, depthTestEnable, CMD_SETDEPTHTESTENABLEEXT);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    RecordCmdSetDepthTestEnable(commandBuffer, depthTestEnable, CMD_SETDEPTHTESTENABLE);
}

void ValidationStateTracker::RecordCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable,
                                                          CMD_TYPE cmd_type) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(cmd_type, CB_DYNAMIC_DEPTH_WRITE_ENABLE_SET);
    cb_state->dynamic_state_value.depth_write_enable = depthWriteEnable;
}

void ValidationStateTracker::PostCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {
    RecordCmdSetDepthWriteEnable(commandBuffer, depthWriteEnable, CMD_SETDEPTHWRITEENABLEEXT);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {
    RecordCmdSetDepthWriteEnable(commandBuffer, depthWriteEnable, CMD_SETDEPTHWRITEENABLE);
}

void ValidationStateTracker::RecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp,
                                                        CMD_TYPE cmd_type) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETDEPTHCOMPAREOP, CB_DYNAMIC_DEPTH_COMPARE_OP_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    RecordCmdSetDepthCompareOp(commandBuffer, depthCompareOp, CMD_SETDEPTHCOMPAREOPEXT);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    RecordCmdSetDepthCompareOp(commandBuffer, depthCompareOp, CMD_SETDEPTHCOMPAREOP);
}

void ValidationStateTracker::RecordCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable,
                                                               CMD_TYPE cmd_type) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(cmd_type, CB_DYNAMIC_DEPTH_BOUNDS_TEST_ENABLE_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer,
                                                                          VkBool32 depthBoundsTestEnable) {
    RecordCmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable, CMD_SETDEPTHBOUNDSTESTENABLEEXT);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer,
                                                                       VkBool32 depthBoundsTestEnable) {
    RecordCmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable, CMD_SETDEPTHBOUNDSTESTENABLE);
}

void ValidationStateTracker::RecordCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable,
                                                           CMD_TYPE cmd_type) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(cmd_type, CB_DYNAMIC_STENCIL_TEST_ENABLE_SET);
    cb_state->dynamic_state_value.stencil_test_enable = stencilTestEnable;
}

void ValidationStateTracker::PostCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {
    RecordCmdSetStencilTestEnable(commandBuffer, stencilTestEnable, CMD_SETSTENCILTESTENABLEEXT);
}

void ValidationStateTracker::PostCallRecordCmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {
    RecordCmdSetStencilTestEnable(commandBuffer, stencilTestEnable, CMD_SETSTENCILTESTENABLE);
}

void ValidationStateTracker::RecordCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp,
                                                   VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp,
                                                   CMD_TYPE cmd_type) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(cmd_type, CB_DYNAMIC_STENCIL_OP_SET);
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
                                                              VkCompareOp compareOp) {
    RecordCmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp, CMD_SETSTENCILOPEXT);
}

void ValidationStateTracker::PostCallRecordCmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                           VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp,
                                                           VkCompareOp compareOp) {
    RecordCmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp, CMD_SETSTENCILOP);
}

void ValidationStateTracker::PostCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                                     uint32_t discardRectangleCount,
                                                                     const VkRect2D *pDiscardRectangles) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETDISCARDRECTANGLEEXT, CB_DYNAMIC_DISCARD_RECTANGLE_EXT_SET);
    for (uint32_t i = 0; i < discardRectangleCount; i++) {
        cb_state->dynamic_state_value.discard_rectangles.set(firstDiscardRectangle + i);
    }
}

void ValidationStateTracker::PostCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                                    const VkSampleLocationsInfoEXT *pSampleLocationsInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETSAMPLELOCATIONSEXT, CB_DYNAMIC_SAMPLE_LOCATIONS_EXT_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer,
                                                                     VkCoarseSampleOrderTypeNV sampleOrderType,
                                                                     uint32_t customSampleOrderCount,
                                                                     const VkCoarseSampleOrderCustomNV *pCustomSampleOrders) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETCOARSESAMPLEORDERNV, CB_DYNAMIC_VIEWPORT_COARSE_SAMPLE_ORDER_NV_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETPATCHCONTROLPOINTSEXT, CB_DYNAMIC_PATCH_CONTROL_POINTS_EXT_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETLOGICOPEXT, CB_DYNAMIC_LOGIC_OP_EXT_SET);
}

void ValidationStateTracker::RecordCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable,
                                                                 CMD_TYPE cmd_type) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(cmd_type, CB_DYNAMIC_RASTERIZER_DISCARD_ENABLE_SET);
    cb_state->rasterization_disabled = rasterizerDiscardEnable == VK_TRUE;
}

void ValidationStateTracker::PostCallRecordCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer,
                                                                            VkBool32 rasterizerDiscardEnable) {
    RecordCmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable, CMD_SETRASTERIZERDISCARDENABLEEXT);
}

void ValidationStateTracker::PostCallRecordCmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer,
                                                                         VkBool32 rasterizerDiscardEnable) {
    RecordCmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable, CMD_SETRASTERIZERDISCARDENABLE);
}

void ValidationStateTracker::RecordCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable,
                                                         CMD_TYPE cmd_type) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(cmd_type, CB_DYNAMIC_DEPTH_BIAS_ENABLE_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) {
    RecordCmdSetDepthBiasEnable(commandBuffer, depthBiasEnable, CMD_SETDEPTHBIASENABLEEXT);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) {
    RecordCmdSetDepthBiasEnable(commandBuffer, depthBiasEnable, CMD_SETDEPTHBIASENABLE);
}

void ValidationStateTracker::RecordCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable,
                                                                CMD_TYPE cmd_type) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(cmd_type, CB_DYNAMIC_PRIMITIVE_RESTART_ENABLE_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer,
                                                                           VkBool32 primitiveRestartEnable) {
    RecordCmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable, CMD_SETPRIMITIVERESTARTENABLEEXT);
}

void ValidationStateTracker::PostCallRecordCmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer,
                                                                        VkBool32 primitiveRestartEnable) {
    RecordCmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable, CMD_SETPRIMITIVERESTARTENABLE);
}

void ValidationStateTracker::PostCallRecordCmdSetVertexInputEXT(
    VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
    const VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount,
    const VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    CBDynamicFlags status_flags;
    status_flags.set(CB_DYNAMIC_VERTEX_INPUT_EXT_SET);

    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    const auto pipeline_state = cb_state->lastBound[lv_bind_point].pipeline_state;
    if (pipeline_state) {
        const auto *dynamic_state = pipeline_state->DynamicState();
        if (dynamic_state) {
            for (uint32_t i = 0; i < dynamic_state->dynamicStateCount; ++i) {
                if (dynamic_state->pDynamicStates[i] == VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT) {
                    status_flags.set(CB_DYNAMIC_VERTEX_INPUT_BINDING_STRIDE_SET);
                    break;
                }
            }
        }
    }
    cb_state->RecordStateCmd(CMD_SETVERTEXINPUTEXT, status_flags);
}

void ValidationStateTracker::PostCallRecordCmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
                                                                     const VkBool32 *pColorWriteEnables) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordColorWriteEnableStateCmd(CMD_SETCOLORWRITEENABLEEXT, CB_DYNAMIC_COLOR_WRITE_ENABLE_EXT_SET, attachmentCount);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
void ValidationStateTracker::PostCallRecordAcquireFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                                             VkResult result) {
    if (result != VK_SUCCESS) return;

    auto swapchain_state = Get<SWAPCHAIN_NODE>(swapchain);
    swapchain_state->exclusive_full_screen_access = true;
}

void ValidationStateTracker::PostCallRecordReleaseFullScreenExclusiveModeEXT(VkDevice device, VkSwapchainKHR swapchain,
                                                                             VkResult result) {
    if (result != VK_SUCCESS) return;

    auto swapchain_state = Get<SWAPCHAIN_NODE>(swapchain);
    swapchain_state->exclusive_full_screen_access = false;
}
#endif

void ValidationStateTracker::PostCallRecordCmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer,
                                                                             VkTessellationDomainOrigin domainOrigin) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETTESSELLATIONDOMAINORIGINEXT, CB_DYNAMIC_TESSELLATION_DOMAIN_ORIGIN_EXT_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClampEnable) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETDEPTHCLAMPENABLEEXT, CB_DYNAMIC_DEPTH_CLAMP_ENABLE_EXT_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetPolygonModeEXT(VkCommandBuffer commandBuffer, VkPolygonMode polygonMode) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETPOLYGONMODEEXT, CB_DYNAMIC_POLYGON_MODE_EXT_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer,
                                                                         VkSampleCountFlagBits rasterizationSamples) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETRASTERIZATIONSAMPLESEXT, CB_DYNAMIC_RASTERIZATION_SAMPLES_EXT_SET);
    cb_state->dynamic_state_value.rasterization_samples = rasterizationSamples;
}

void ValidationStateTracker::PostCallRecordCmdSetSampleMaskEXT(VkCommandBuffer commandBuffer, VkSampleCountFlagBits samples,
                                                               const VkSampleMask *pSampleMask) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETSAMPLEMASKEXT, CB_DYNAMIC_SAMPLE_MASK_EXT_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer,
                                                                          VkBool32 alphaToCoverageEnable) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETALPHATOCOVERAGEENABLEEXT, CB_DYNAMIC_ALPHA_TO_COVERAGE_ENABLE_EXT_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer, VkBool32 alphaToOneEnable) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETALPHATOONEENABLEEXT, CB_DYNAMIC_ALPHA_TO_ONE_ENABLE_EXT_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer, VkBool32 logicOpEnable) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETLOGICOPENABLEEXT, CB_DYNAMIC_LOGIC_OP_ENABLE_EXT_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                                     uint32_t attachmentCount, const VkBool32 *pColorBlendEnables) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETCOLORBLENDENABLEEXT, CB_DYNAMIC_COLOR_BLEND_ENABLE_EXT_SET);
    for (uint32_t i = 0; i < attachmentCount; i++) {
        cb_state->dynamic_state_value.color_blend_enable_attachments.set(firstAttachment + i);
    }
}

void ValidationStateTracker::PostCallRecordCmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                                       uint32_t attachmentCount,
                                                                       const VkColorBlendEquationEXT *pColorBlendEquations) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETCOLORBLENDEQUATIONEXT, CB_DYNAMIC_COLOR_BLEND_EQUATION_EXT_SET);
    for (uint32_t i = 0; i < attachmentCount; i++) {
        cb_state->dynamic_state_value.color_blend_equation_attachments.set(firstAttachment + i);
    }
}

void ValidationStateTracker::PostCallRecordCmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                                   uint32_t attachmentCount,
                                                                   const VkColorComponentFlags *pColorWriteMasks) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETCOLORWRITEMASKEXT, CB_DYNAMIC_COLOR_WRITE_MASK_EXT_SET);
    for (uint32_t i = 0; i < attachmentCount; i++) {
        cb_state->dynamic_state_value.color_write_mask_attachments.set(firstAttachment + i);
    }
}

void ValidationStateTracker::PostCallRecordCmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer,
                                                                        uint32_t rasterizationStream) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETRASTERIZATIONSTREAMEXT, CB_DYNAMIC_RASTERIZATION_STREAM_EXT_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetConservativeRasterizationModeEXT(
    VkCommandBuffer commandBuffer, VkConservativeRasterizationModeEXT conservativeRasterizationMode) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETCONSERVATIVERASTERIZATIONMODEEXT, CB_DYNAMIC_CONSERVATIVE_RASTERIZATION_MODE_EXT_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetExtraPrimitiveOverestimationSizeEXT(VkCommandBuffer commandBuffer,
                                                                                     float extraPrimitiveOverestimationSize) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETEXTRAPRIMITIVEOVERESTIMATIONSIZEEXT, CB_DYNAMIC_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthClipEnable) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETDEPTHCLIPENABLEEXT, CB_DYNAMIC_DEPTH_CLIP_ENABLE_EXT_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer,
                                                                          VkBool32 sampleLocationsEnable) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETSAMPLELOCATIONSENABLEEXT, CB_DYNAMIC_SAMPLE_LOCATIONS_ENABLE_EXT_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer, uint32_t firstAttachment,
                                                                       uint32_t attachmentCount,
                                                                       const VkColorBlendAdvancedEXT *pColorBlendAdvanced) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETCOLORBLENDADVANCEDEXT, CB_DYNAMIC_COLOR_BLEND_ADVANCED_EXT_SET);
    for (uint32_t i = 0; i < attachmentCount; i++) {
        cb_state->dynamic_state_value.color_blend_advanced_attachments.set(firstAttachment + i);
    }
}

void ValidationStateTracker::PostCallRecordCmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer,
                                                                        VkProvokingVertexModeEXT provokingVertexMode) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETPROVOKINGVERTEXMODEEXT, CB_DYNAMIC_PROVOKING_VERTEX_MODE_EXT_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                                                          VkLineRasterizationModeEXT lineRasterizationMode) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETLINERASTERIZATIONMODEEXT, CB_DYNAMIC_LINE_RASTERIZATION_MODE_EXT_SET);
    cb_state->dynamic_state_value.line_rasterization_mode = lineRasterizationMode;
}

void ValidationStateTracker::PostCallRecordCmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stippledLineEnable) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETLINESTIPPLEENABLEEXT, CB_DYNAMIC_LINE_STIPPLE_ENABLE_EXT_SET);
    cb_state->dynamic_state_value.stippled_line_enable = stippledLineEnable;
}

void ValidationStateTracker::PostCallRecordCmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer,
                                                                              VkBool32 negativeOneToOne) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETDEPTHCLIPNEGATIVEONETOONEEXT, CB_DYNAMIC_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetViewportWScalingEnableNV(VkCommandBuffer commandBuffer,
                                                                          VkBool32 viewportWScalingEnable) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETVIEWPORTWSCALINGENABLENV, CB_DYNAMIC_VIEWPORT_W_SCALING_ENABLE_NV_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetViewportSwizzleNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                   uint32_t viewportCount,
                                                                   const VkViewportSwizzleNV *pViewportSwizzles) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETVIEWPORTSWIZZLENV, CB_DYNAMIC_VIEWPORT_SWIZZLE_NV_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetCoverageToColorEnableNV(VkCommandBuffer commandBuffer,
                                                                         VkBool32 coverageToColorEnable) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETCOVERAGETOCOLORENABLENV, CB_DYNAMIC_COVERAGE_TO_COLOR_ENABLE_NV_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetCoverageToColorLocationNV(VkCommandBuffer commandBuffer,
                                                                           uint32_t coverageToColorLocation) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETCOVERAGETOCOLORLOCATIONNV, CB_DYNAMIC_COVERAGE_TO_COLOR_LOCATION_NV_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetCoverageModulationModeNV(VkCommandBuffer commandBuffer,
                                                                          VkCoverageModulationModeNV coverageModulationMode) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETCOVERAGEMODULATIONMODENV, CB_DYNAMIC_COVERAGE_MODULATION_MODE_NV_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetCoverageModulationTableEnableNV(VkCommandBuffer commandBuffer,
                                                                                 VkBool32 coverageModulationTableEnable) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETCOVERAGEMODULATIONTABLEENABLENV, CB_DYNAMIC_COVERAGE_MODULATION_TABLE_ENABLE_NV_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetCoverageModulationTableNV(VkCommandBuffer commandBuffer,
                                                                           uint32_t coverageModulationTableCount,
                                                                           const float *pCoverageModulationTable) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETCOVERAGEMODULATIONTABLENV, CB_DYNAMIC_COVERAGE_MODULATION_TABLE_NV_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetShadingRateImageEnableNV(VkCommandBuffer commandBuffer,
                                                                          VkBool32 shadingRateImageEnable) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETSHADINGRATEIMAGEENABLENV, CB_DYNAMIC_SHADING_RATE_IMAGE_ENABLE_NV_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetRepresentativeFragmentTestEnableNV(VkCommandBuffer commandBuffer,
                                                                                    VkBool32 representativeFragmentTestEnable) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETREPRESENTATIVEFRAGMENTTESTENABLENV, CB_DYNAMIC_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV_SET);
}

void ValidationStateTracker::PostCallRecordCmdSetCoverageReductionModeNV(VkCommandBuffer commandBuffer,
                                                                         VkCoverageReductionModeNV coverageReductionMode) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETCOVERAGEREDUCTIONMODENV, CB_DYNAMIC_COVERAGE_REDUCTION_MODE_NV_SET);
}

void ValidationStateTracker::PostCallRecordCmdControlVideoCodingKHR(VkCommandBuffer commandBuffer,
                                                                    const VkVideoCodingControlInfoKHR *pCodingControlInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->ControlVideoCoding(pCodingControlInfo);
}

void ValidationStateTracker::PostCallRecordCmdDecodeVideoKHR(VkCommandBuffer commandBuffer,
                                                             const VkVideoDecodeInfoKHR *pDecodeInfo) {
    auto cb_state = GetWrite<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->DecodeVideo(pDecodeInfo);
}

void ValidationStateTracker::RecordGetBufferDeviceAddress(const VkBufferDeviceAddressInfo *pInfo, VkDeviceAddress address) {
    auto buffer_state = Get<BUFFER_STATE>(pInfo->buffer);
    if (buffer_state && address != 0) {
        WriteLockGuard guard(buffer_address_lock_);
        // address is used for GPU-AV and ray tracing buffer validation
        buffer_state->deviceAddress = address;
        const auto address_range = buffer_state->DeviceAddressRange();

        buffer_address_map_.split_and_merge_insert(
            {address_range, {buffer_state}}, [](auto &current_buffer_list, const auto &new_buffer) {
                assert(!current_buffer_list.empty());
                const auto buffer_found_it = std::find(current_buffer_list.begin(), current_buffer_list.end(), new_buffer[0]);
                if (buffer_found_it == current_buffer_list.end()) {
                    current_buffer_list.emplace_back(new_buffer[0]);
                }
            });
    }
}

void ValidationStateTracker::PostCallRecordGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo *pInfo,
                                                                  VkDeviceAddress address) {
    RecordGetBufferDeviceAddress(pInfo, address);
}

void ValidationStateTracker::PostCallRecordGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo *pInfo,
                                                                     VkDeviceAddress address) {
    RecordGetBufferDeviceAddress(pInfo, address);
}

void ValidationStateTracker::PostCallRecordGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo *pInfo,
                                                                     VkDeviceAddress address) {
    RecordGetBufferDeviceAddress(pInfo, address);
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
