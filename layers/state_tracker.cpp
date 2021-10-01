/* Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (C) 2015-2021 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
 *
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Shannon McPherson <shannon@lunarg.com>
 * Author: Tobias Hector <tobias.hector@amd.com>
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
#include "state_tracker.h"
#include "shader_validation.h"
#include "sync_utils.h"
#include "cmd_buffer_state.h"
#include "render_pass_state.h"

extern template PIPELINE_STATE::PIPELINE_STATE(const ValidationStateTracker *, const VkRayTracingPipelineCreateInfoKHR *,
                                               std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&);
extern template PIPELINE_STATE::PIPELINE_STATE(const ValidationStateTracker *, const VkRayTracingPipelineCreateInfoNV *,
                                               std::shared_ptr<const PIPELINE_LAYOUT_STATE> &&);

void ValidationStateTracker::InitDeviceValidationObject(bool add_obj, ValidationObject *inst_obj, ValidationObject *dev_obj) {
    if (add_obj) {
        instance_state = reinterpret_cast<ValidationStateTracker *>(GetValidationObject(inst_obj->object_dispatch, container_type));
        // Call base class
        ValidationObject::InitDeviceValidationObject(add_obj, inst_obj, dev_obj);
    }
}

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

std::vector<std::shared_ptr<const IMAGE_VIEW_STATE>> ValidationStateTracker::GetSharedAttachmentViews(
    const VkRenderPassBeginInfo &rp_begin, const FRAMEBUFFER_STATE &fb_state) const {
    auto get_fn = [this](VkImageView handle) { return this->GetShared<IMAGE_VIEW_STATE>(handle); };
    return GetAttachmentViewsImpl<std::shared_ptr<const IMAGE_VIEW_STATE>>(rp_begin, fb_state, get_fn);
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR
// Android-specific validation that uses types defined only with VK_USE_PLATFORM_ANDROID_KHR
// This could also move into a seperate core_validation_android.cpp file... ?

template <typename CreateInfo>
VkFormatFeatureFlags ValidationStateTracker::GetExternalFormatFeaturesANDROID(const CreateInfo *create_info) const {
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
    auto ahb_format_props = LvlFindInChain<VkAndroidHardwareBufferFormatPropertiesANDROID>(pProperties->pNext);
    if (ahb_format_props) {
        ahb_ext_formats_map.emplace(ahb_format_props->externalFormat, ahb_format_props->formatFeatures);
    }
}

#else

template <typename CreateInfo>
VkFormatFeatureFlags ValidationStateTracker::GetExternalFormatFeaturesANDROID(const CreateInfo *create_info) const {
    return 0;
}

#endif  // VK_USE_PLATFORM_ANDROID_KHR

VkFormatFeatureFlags GetImageFormatFeatures(VkPhysicalDevice physical_device, VkDevice device, VkImage image, VkFormat format,
                                            VkImageTiling tiling) {
    VkFormatFeatureFlags format_features = 0;
    // Add feature support according to Image Format Features (vkspec.html#resources-image-format-features)
    // if format is AHB external format then the features are already set
    if (tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        VkImageDrmFormatModifierPropertiesEXT drm_format_properties = {VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_PROPERTIES_EXT,
                                                                       nullptr};
        DispatchGetImageDrmFormatModifierPropertiesEXT(device, image, &drm_format_properties);

        VkFormatProperties2 format_properties_2 = {VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2, nullptr};
        VkDrmFormatModifierPropertiesListEXT drm_properties_list = {VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT,
                                                                    nullptr};
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

void ValidationStateTracker::PostCallRecordCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkImage *pImage, VkResult result) {
    if (VK_SUCCESS != result) return;
    VkFormatFeatureFlags format_features = 0;
    if (IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)) {
        format_features = GetExternalFormatFeaturesANDROID(pCreateInfo);
    }
    if (format_features == 0) {
        format_features = GetImageFormatFeatures(physical_device, device, *pImage, pCreateInfo->format, pCreateInfo->tiling);
    }
    imageMap[*pImage] = std::make_shared<IMAGE_STATE>(this, *pImage, pCreateInfo, format_features);
}

void ValidationStateTracker::PreCallRecordDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator) {
    if (!image) return;
    IMAGE_STATE *image_state = GetImageState(image);
    if (!image_state) return;

    image_state->Destroy();
    imageMap.erase(image);
}

void ValidationStateTracker::PreCallRecordCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image,
                                                             VkImageLayout imageLayout, const VkClearColorValue *pColor,
                                                             uint32_t rangeCount, const VkImageSubresourceRange *pRanges) {

    if (disabled[command_buffer_state]) return;

    auto cb_node = Get<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_node) {
        cb_node->RecordTransferCmd(CMD_CLEARCOLORIMAGE, GetImageState(image));
    }
}

void ValidationStateTracker::PreCallRecordCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image,
                                                                    VkImageLayout imageLayout,
                                                                    const VkClearDepthStencilValue *pDepthStencil,
                                                                    uint32_t rangeCount, const VkImageSubresourceRange *pRanges) {
    if (disabled[command_buffer_state]) return;

    auto cb_node = Get<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_node) {
        cb_node->RecordTransferCmd(CMD_CLEARDEPTHSTENCILIMAGE, GetImageState(image));
    }
}

void ValidationStateTracker::PreCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                       VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout,
                                                       uint32_t regionCount, const VkImageCopy *pRegions) {
    if (disabled[command_buffer_state]) return;

    auto cb_node = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_node->RecordTransferCmd(CMD_COPYIMAGE, GetImageState(srcImage), GetImageState(dstImage));
}

void ValidationStateTracker::PreCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer,
                                                           const VkCopyImageInfo2KHR *pCopyImageInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_node = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_node->RecordTransferCmd(CMD_COPYIMAGE2KHR, GetImageState(pCopyImageInfo->srcImage), GetImageState(pCopyImageInfo->dstImage));
}

void ValidationStateTracker::PreCallRecordCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                          VkImageLayout srcImageLayout, VkImage dstImage,
                                                          VkImageLayout dstImageLayout, uint32_t regionCount,
                                                          const VkImageResolve *pRegions) {
    if (disabled[command_buffer_state]) return;

    auto cb_node = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_node->RecordTransferCmd(CMD_RESOLVEIMAGE, GetImageState(srcImage), GetImageState(dstImage));
}

void ValidationStateTracker::PreCallRecordCmdResolveImage2KHR(VkCommandBuffer commandBuffer,
                                                              const VkResolveImageInfo2KHR *pResolveImageInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_node = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_node->RecordTransferCmd(CMD_RESOLVEIMAGE2KHR, GetImageState(pResolveImageInfo->srcImage),
                               GetImageState(pResolveImageInfo->dstImage));
}

void ValidationStateTracker::PreCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                       VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout,
                                                       uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter) {
    if (disabled[command_buffer_state]) return;

    auto cb_node = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_node->RecordTransferCmd(CMD_BLITIMAGE, GetImageState(srcImage), GetImageState(dstImage));
}

void ValidationStateTracker::PreCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer,
                                                           const VkBlitImageInfo2KHR *pBlitImageInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_node = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_node->RecordTransferCmd(CMD_BLITIMAGE2KHR, GetImageState(pBlitImageInfo->srcImage), GetImageState(pBlitImageInfo->dstImage));
}

void ValidationStateTracker::PostCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
                                                        const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer,
                                                        VkResult result) {
    if (result != VK_SUCCESS) return;

    auto buffer_state = std::make_shared<BUFFER_STATE>(this, *pBuffer, pCreateInfo);

    if (pCreateInfo) {
        const auto *opaque_capture_address = LvlFindInChain<VkBufferOpaqueCaptureAddressCreateInfo>(pCreateInfo->pNext);
        if (opaque_capture_address) {
            // address is used for GPU-AV and ray tracing buffer validation
            buffer_state->deviceAddress = opaque_capture_address->opaqueCaptureAddress;
            buffer_address_map_.emplace(opaque_capture_address->opaqueCaptureAddress, buffer_state.get());
        }
    }
    bufferMap.emplace(*pBuffer, std::move(buffer_state));
}

void ValidationStateTracker::PostCallRecordCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator, VkBufferView *pView,
                                                            VkResult result) {
    if (result != VK_SUCCESS) return;

    auto buffer_state = GetBufferShared(pCreateInfo->buffer);

    VkFormatProperties format_properties;
    DispatchGetPhysicalDeviceFormatProperties(physical_device, pCreateInfo->format, &format_properties);

    bufferViewMap[*pView] =
        std::make_shared<BUFFER_VIEW_STATE>(buffer_state, *pView, pCreateInfo, format_properties.bufferFeatures);
}

void ValidationStateTracker::PostCallRecordCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                                           const VkAllocationCallbacks *pAllocator, VkImageView *pView,
                                                           VkResult result) {
    if (result != VK_SUCCESS) return;
    auto image_state = GetImageShared(pCreateInfo->image);

    VkFormatFeatureFlags format_features = 0;
    if (image_state->HasAHBFormat() == true) {
        // The ImageView uses same Image's format feature since they share same AHB
        format_features = image_state->format_features;
    } else {
        format_features = GetImageFormatFeatures(physical_device, device, image_state->image(), pCreateInfo->format,
                                                 image_state->createInfo.tiling);
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

    imageViewMap[*pView] =
        std::make_shared<IMAGE_VIEW_STATE>(image_state, *pView, pCreateInfo, format_features, filter_cubic_props);
}

void ValidationStateTracker::PreCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                                        uint32_t regionCount, const VkBufferCopy *pRegions) {
    if (disabled[command_buffer_state]) return;

    auto cb_node = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_node->RecordTransferCmd(CMD_COPYBUFFER, GetBufferState(srcBuffer), GetBufferState(dstBuffer));
}

void ValidationStateTracker::PreCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer,
                                                            const VkCopyBufferInfo2KHR *pCopyBufferInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_node = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_node->RecordTransferCmd(CMD_COPYBUFFER2KHR, GetBufferState(pCopyBufferInfo->srcBuffer),
                               GetBufferState(pCopyBufferInfo->dstBuffer));
}

void ValidationStateTracker::PreCallRecordDestroyImageView(VkDevice device, VkImageView imageView,
                                                           const VkAllocationCallbacks *pAllocator) {
    IMAGE_VIEW_STATE *image_view_state = GetImageViewState(imageView);
    if (!image_view_state) return;

    // Any bound cmd buffers are now invalid
    image_view_state->Destroy();
    imageViewMap.erase(imageView);
}

void ValidationStateTracker::PreCallRecordDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator) {
    if (!buffer) return;
    auto buffer_state = GetBufferState(buffer);

    buffer_state->Destroy();
    bufferMap.erase(buffer_state->buffer());
}

void ValidationStateTracker::PreCallRecordDestroyBufferView(VkDevice device, VkBufferView bufferView,
                                                            const VkAllocationCallbacks *pAllocator) {
    if (!bufferView) return;
    auto buffer_view_state = GetBufferViewState(bufferView);

    // Any bound cmd buffers are now invalid
    buffer_view_state->Destroy();
    bufferViewMap.erase(bufferView);
}

void ValidationStateTracker::PreCallRecordCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                                        VkDeviceSize size, uint32_t data) {
    if (disabled[command_buffer_state]) return;

    auto cb_node = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_node->RecordTransferCmd(CMD_FILLBUFFER, GetBufferState(dstBuffer));
}

void ValidationStateTracker::PreCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage,
                                                               VkImageLayout srcImageLayout, VkBuffer dstBuffer,
                                                               uint32_t regionCount, const VkBufferImageCopy *pRegions) {
    if (disabled[command_buffer_state]) return;

    auto cb_node = Get<CMD_BUFFER_STATE>(commandBuffer);

    cb_node->RecordTransferCmd(CMD_COPYIMAGETOBUFFER, GetImageState(srcImage), GetBufferState(dstBuffer));
}

void ValidationStateTracker::PreCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                                   const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo) {
    if (disabled[command_buffer_state]) return;

    auto cb_node = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_node->RecordTransferCmd(CMD_COPYIMAGETOBUFFER2KHR, GetImageState(pCopyImageToBufferInfo->srcImage),
                               GetBufferState(pCopyImageToBufferInfo->dstBuffer));
}

void ValidationStateTracker::PreCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                               VkImageLayout dstImageLayout, uint32_t regionCount,
                                                               const VkBufferImageCopy *pRegions) {
    if (disabled[command_buffer_state]) return;

    auto cb_node = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_node->RecordTransferCmd(CMD_COPYBUFFERTOIMAGE, GetBufferState(srcBuffer), GetImageState(dstImage));
}

void ValidationStateTracker::PreCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                                   const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo) {

    if (disabled[command_buffer_state]) return;

    auto cb_node = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_node->RecordTransferCmd(CMD_COPYBUFFERTOIMAGE2KHR, GetBufferState(pCopyBufferToImageInfo->srcBuffer),
                               GetImageState(pCopyBufferToImageInfo->dstImage));
}

// Return ptr to memory binding for given handle of specified type
template <typename State, typename Result>
static Result GetObjectMemBindingImpl(State state, const VulkanTypedHandle &typed_handle) {
    switch (typed_handle.type) {
        case kVulkanObjectTypeImage:
            return state->GetImageState(typed_handle.Cast<VkImage>());
        case kVulkanObjectTypeBuffer:
            return state->GetBufferState(typed_handle.Cast<VkBuffer>());
        case kVulkanObjectTypeAccelerationStructureNV:
            return state->GetAccelerationStructureStateNV(typed_handle.Cast<VkAccelerationStructureNV>());
        default:
            break;
    }
    return nullptr;
}

const BINDABLE *ValidationStateTracker::GetObjectMemBinding(const VulkanTypedHandle &typed_handle) const {
    return GetObjectMemBindingImpl<const ValidationStateTracker *, const BINDABLE *>(this, typed_handle);
}

BINDABLE *ValidationStateTracker::GetObjectMemBinding(const VulkanTypedHandle &typed_handle) {
    return GetObjectMemBindingImpl<ValidationStateTracker *, BINDABLE *>(this, typed_handle);
}

// Remove set from setMap and delete the set
void ValidationStateTracker::FreeDescriptorSet(cvdescriptorset::DescriptorSet *descriptor_set) {
    // Any bound cmd buffers are now invalid
    descriptor_set->Destroy();

    setMap.erase(descriptor_set->GetSet());
}

// Free all DS Pools including their Sets & related sub-structs
// NOTE : Calls to this function should be wrapped in mutex
void ValidationStateTracker::DeleteDescriptorSetPools() {
    for (auto ii = descriptorPoolMap.begin(); ii != descriptorPoolMap.end();) {
        // Remove this pools' sets from setMap and delete them
        for (auto *ds : ii->second->sets) {
            FreeDescriptorSet(ds);
        }
        ii->second->sets.clear();
        ii = descriptorPoolMap.erase(ii);
    }
}

// For given object struct return a ptr of BASE_NODE type for its wrapping struct
BASE_NODE *ValidationStateTracker::GetStateStructPtrFromObject(const VulkanTypedHandle &object_struct) {
    if (object_struct.node) {
#ifdef _DEBUG
        // assert that lookup would find the same object
        VulkanTypedHandle other = object_struct;
        other.node = nullptr;
        assert(object_struct.node == GetStateStructPtrFromObject(other));
#endif
        return object_struct.node;
    }
    BASE_NODE *base_ptr = nullptr;
    switch (object_struct.type) {
        case kVulkanObjectTypeDescriptorSet: {
            base_ptr = GetSetNode(object_struct.Cast<VkDescriptorSet>());
            break;
        }
        case kVulkanObjectTypeSampler: {
            base_ptr = GetSamplerState(object_struct.Cast<VkSampler>());
            break;
        }
        case kVulkanObjectTypeQueryPool: {
            base_ptr = GetQueryPoolState(object_struct.Cast<VkQueryPool>());
            break;
        }
        case kVulkanObjectTypePipeline: {
            base_ptr = GetPipelineState(object_struct.Cast<VkPipeline>());
            break;
        }
        case kVulkanObjectTypeBuffer: {
            base_ptr = GetBufferState(object_struct.Cast<VkBuffer>());
            break;
        }
        case kVulkanObjectTypeBufferView: {
            base_ptr = GetBufferViewState(object_struct.Cast<VkBufferView>());
            break;
        }
        case kVulkanObjectTypeImage: {
            base_ptr = GetImageState(object_struct.Cast<VkImage>());
            break;
        }
        case kVulkanObjectTypeImageView: {
            base_ptr = GetImageViewState(object_struct.Cast<VkImageView>());
            break;
        }
        case kVulkanObjectTypeEvent: {
            base_ptr = GetEventState(object_struct.Cast<VkEvent>());
            break;
        }
        case kVulkanObjectTypeDescriptorPool: {
            base_ptr = GetDescriptorPoolState(object_struct.Cast<VkDescriptorPool>());
            break;
        }
        case kVulkanObjectTypeCommandPool: {
            base_ptr = GetCommandPoolState(object_struct.Cast<VkCommandPool>());
            break;
        }
        case kVulkanObjectTypeFramebuffer: {
            base_ptr = GetFramebufferState(object_struct.Cast<VkFramebuffer>());
            break;
        }
        case kVulkanObjectTypeRenderPass: {
            base_ptr = GetRenderPassState(object_struct.Cast<VkRenderPass>());
            break;
        }
        case kVulkanObjectTypeDeviceMemory: {
            base_ptr = GetDevMemState(object_struct.Cast<VkDeviceMemory>());
            break;
        }
        case kVulkanObjectTypeAccelerationStructureNV: {
            base_ptr = GetAccelerationStructureStateNV(object_struct.Cast<VkAccelerationStructureNV>());
            break;
        }
        case kVulkanObjectTypeAccelerationStructureKHR: {
            base_ptr = GetAccelerationStructureStateKHR(object_struct.Cast<VkAccelerationStructureKHR>());
            break;
        }
        case kVulkanObjectTypeUnknown:
            // This can happen if an element of the object_bindings vector has been
            // zeroed out, after an object is destroyed.
            break;
        default:
            // TODO : Any other objects to be handled here?
            assert(0);
            break;
    }
    return base_ptr;
}

// Gets union of all features defined by Potential Format Features
// except, does not handle the external format case for AHB as that only can be used for sampled images
VkFormatFeatureFlags ValidationStateTracker::GetPotentialFormatFeatures(VkFormat format) const {
    VkFormatFeatureFlags format_features = 0;

    if (format != VK_FORMAT_UNDEFINED) {
        VkFormatProperties format_properties;
        DispatchGetPhysicalDeviceFormatProperties(physical_device, format, &format_properties);
        format_features |= format_properties.linearTilingFeatures;
        format_features |= format_properties.optimalTilingFeatures;
        if (IsExtEnabled(device_extensions.vk_ext_image_drm_format_modifier)) {
            // VK_KHR_get_physical_device_properties2 is required in this case
            VkFormatProperties2 format_properties_2 = {VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
            VkDrmFormatModifierPropertiesListEXT drm_properties_list = {VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT,
                                                                        nullptr};
            format_properties_2.pNext = (void *)&drm_properties_list;

            // First call is to get the number of modifiers compatible with the queried format
            DispatchGetPhysicalDeviceFormatProperties2(physical_device, format, &format_properties_2);

            std::vector<VkDrmFormatModifierPropertiesEXT> drm_properties;
            drm_properties.resize(drm_properties_list.drmFormatModifierCount);
            drm_properties_list.pDrmFormatModifierProperties = drm_properties.data();

            // Second call, now with an allocated array in pDrmFormatModifierProperties, is to get the modifiers
            // compatible with the queried format
            DispatchGetPhysicalDeviceFormatProperties2(physical_device, format, &format_properties_2);

            for (uint32_t i = 0; i < drm_properties_list.drmFormatModifierCount; i++) {
                format_features |= drm_properties_list.pDrmFormatModifierProperties[i].drmFormatModifierTilingFeatures;
            }
        }
    }

    return format_features;
}

void ValidationStateTracker::PostCallRecordCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                                        const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                                        VkResult result) {
    if (VK_SUCCESS != result) return;

    ValidationObject *device_object = GetLayerDataPtr(get_dispatch_key(*pDevice), layer_data_map);
    ValidationObject *validation_data = GetValidationObject(device_object->object_dispatch, this->container_type);
    ValidationStateTracker *state_tracker = static_cast<ValidationStateTracker *>(validation_data);

    const VkPhysicalDeviceFeatures *enabled_features_found = pCreateInfo->pEnabledFeatures;
    if (nullptr == enabled_features_found) {
        const auto *features2 = LvlFindInChain<VkPhysicalDeviceFeatures2>(pCreateInfo->pNext);
        if (features2) {
            enabled_features_found = &(features2->features);
        }
    }

    if (nullptr == enabled_features_found) {
        state_tracker->enabled_features.core = {};
    } else {
        state_tracker->enabled_features.core = *enabled_features_found;
    }

    // Save local link to this device's physical device state
    state_tracker->physical_device_state = Get<PHYSICAL_DEVICE_STATE>(gpu);

    const auto *vulkan_12_features = LvlFindInChain<VkPhysicalDeviceVulkan12Features>(pCreateInfo->pNext);
    if (vulkan_12_features) {
        state_tracker->enabled_features.core12 = *vulkan_12_features;
    } else {
        // Set Extension Feature Aliases to false as there is no struct to check
        state_tracker->enabled_features.core12.drawIndirectCount = VK_FALSE;
        state_tracker->enabled_features.core12.samplerMirrorClampToEdge = VK_FALSE;
        state_tracker->enabled_features.core12.descriptorIndexing = VK_FALSE;
        state_tracker->enabled_features.core12.samplerFilterMinmax = VK_FALSE;
        state_tracker->enabled_features.core12.shaderOutputLayer = VK_FALSE;
        state_tracker->enabled_features.core12.shaderOutputViewportIndex = VK_FALSE;
        state_tracker->enabled_features.core12.subgroupBroadcastDynamicId = VK_FALSE;

        // These structs are only allowed in pNext chain if there is no VkPhysicalDeviceVulkan12Features

        const auto *eight_bit_storage_features = LvlFindInChain<VkPhysicalDevice8BitStorageFeatures>(pCreateInfo->pNext);
        if (eight_bit_storage_features) {
            state_tracker->enabled_features.core12.storageBuffer8BitAccess = eight_bit_storage_features->storageBuffer8BitAccess;
            state_tracker->enabled_features.core12.uniformAndStorageBuffer8BitAccess =
                eight_bit_storage_features->uniformAndStorageBuffer8BitAccess;
            state_tracker->enabled_features.core12.storagePushConstant8 = eight_bit_storage_features->storagePushConstant8;
        }

        const auto *float16_int8_features = LvlFindInChain<VkPhysicalDeviceShaderFloat16Int8Features>(pCreateInfo->pNext);
        if (float16_int8_features) {
            state_tracker->enabled_features.core12.shaderFloat16 = float16_int8_features->shaderFloat16;
            state_tracker->enabled_features.core12.shaderInt8 = float16_int8_features->shaderInt8;
        }

        const auto *descriptor_indexing_features = LvlFindInChain<VkPhysicalDeviceDescriptorIndexingFeatures>(pCreateInfo->pNext);
        if (descriptor_indexing_features) {
            state_tracker->enabled_features.core12.shaderInputAttachmentArrayDynamicIndexing =
                descriptor_indexing_features->shaderInputAttachmentArrayDynamicIndexing;
            state_tracker->enabled_features.core12.shaderUniformTexelBufferArrayDynamicIndexing =
                descriptor_indexing_features->shaderUniformTexelBufferArrayDynamicIndexing;
            state_tracker->enabled_features.core12.shaderStorageTexelBufferArrayDynamicIndexing =
                descriptor_indexing_features->shaderStorageTexelBufferArrayDynamicIndexing;
            state_tracker->enabled_features.core12.shaderUniformBufferArrayNonUniformIndexing =
                descriptor_indexing_features->shaderUniformBufferArrayNonUniformIndexing;
            state_tracker->enabled_features.core12.shaderSampledImageArrayNonUniformIndexing =
                descriptor_indexing_features->shaderSampledImageArrayNonUniformIndexing;
            state_tracker->enabled_features.core12.shaderStorageBufferArrayNonUniformIndexing =
                descriptor_indexing_features->shaderStorageBufferArrayNonUniformIndexing;
            state_tracker->enabled_features.core12.shaderStorageImageArrayNonUniformIndexing =
                descriptor_indexing_features->shaderStorageImageArrayNonUniformIndexing;
            state_tracker->enabled_features.core12.shaderInputAttachmentArrayNonUniformIndexing =
                descriptor_indexing_features->shaderInputAttachmentArrayNonUniformIndexing;
            state_tracker->enabled_features.core12.shaderUniformTexelBufferArrayNonUniformIndexing =
                descriptor_indexing_features->shaderUniformTexelBufferArrayNonUniformIndexing;
            state_tracker->enabled_features.core12.shaderStorageTexelBufferArrayNonUniformIndexing =
                descriptor_indexing_features->shaderStorageTexelBufferArrayNonUniformIndexing;
            state_tracker->enabled_features.core12.descriptorBindingUniformBufferUpdateAfterBind =
                descriptor_indexing_features->descriptorBindingUniformBufferUpdateAfterBind;
            state_tracker->enabled_features.core12.descriptorBindingSampledImageUpdateAfterBind =
                descriptor_indexing_features->descriptorBindingSampledImageUpdateAfterBind;
            state_tracker->enabled_features.core12.descriptorBindingStorageImageUpdateAfterBind =
                descriptor_indexing_features->descriptorBindingStorageImageUpdateAfterBind;
            state_tracker->enabled_features.core12.descriptorBindingStorageBufferUpdateAfterBind =
                descriptor_indexing_features->descriptorBindingStorageBufferUpdateAfterBind;
            state_tracker->enabled_features.core12.descriptorBindingUniformTexelBufferUpdateAfterBind =
                descriptor_indexing_features->descriptorBindingUniformTexelBufferUpdateAfterBind;
            state_tracker->enabled_features.core12.descriptorBindingStorageTexelBufferUpdateAfterBind =
                descriptor_indexing_features->descriptorBindingStorageTexelBufferUpdateAfterBind;
            state_tracker->enabled_features.core12.descriptorBindingUpdateUnusedWhilePending =
                descriptor_indexing_features->descriptorBindingUpdateUnusedWhilePending;
            state_tracker->enabled_features.core12.descriptorBindingPartiallyBound =
                descriptor_indexing_features->descriptorBindingPartiallyBound;
            state_tracker->enabled_features.core12.descriptorBindingVariableDescriptorCount =
                descriptor_indexing_features->descriptorBindingVariableDescriptorCount;
            state_tracker->enabled_features.core12.runtimeDescriptorArray = descriptor_indexing_features->runtimeDescriptorArray;
        }

        const auto *scalar_block_layout_features = LvlFindInChain<VkPhysicalDeviceScalarBlockLayoutFeatures>(pCreateInfo->pNext);
        if (scalar_block_layout_features) {
            state_tracker->enabled_features.core12.scalarBlockLayout = scalar_block_layout_features->scalarBlockLayout;
        }

        const auto *imageless_framebuffer_features =
            LvlFindInChain<VkPhysicalDeviceImagelessFramebufferFeatures>(pCreateInfo->pNext);
        if (imageless_framebuffer_features) {
            state_tracker->enabled_features.core12.imagelessFramebuffer = imageless_framebuffer_features->imagelessFramebuffer;
        }

        const auto *uniform_buffer_standard_layout_features =
            LvlFindInChain<VkPhysicalDeviceUniformBufferStandardLayoutFeatures>(pCreateInfo->pNext);
        if (uniform_buffer_standard_layout_features) {
            state_tracker->enabled_features.core12.uniformBufferStandardLayout =
                uniform_buffer_standard_layout_features->uniformBufferStandardLayout;
        }

        const auto *subgroup_extended_types_features =
            LvlFindInChain<VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures>(pCreateInfo->pNext);
        if (subgroup_extended_types_features) {
            state_tracker->enabled_features.core12.shaderSubgroupExtendedTypes =
                subgroup_extended_types_features->shaderSubgroupExtendedTypes;
        }

        const auto *separate_depth_stencil_layouts_features =
            LvlFindInChain<VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures>(pCreateInfo->pNext);
        if (separate_depth_stencil_layouts_features) {
            state_tracker->enabled_features.core12.separateDepthStencilLayouts =
                separate_depth_stencil_layouts_features->separateDepthStencilLayouts;
        }

        const auto *host_query_reset_features = LvlFindInChain<VkPhysicalDeviceHostQueryResetFeatures>(pCreateInfo->pNext);
        if (host_query_reset_features) {
            state_tracker->enabled_features.core12.hostQueryReset = host_query_reset_features->hostQueryReset;
        }

        const auto *timeline_semaphore_features = LvlFindInChain<VkPhysicalDeviceTimelineSemaphoreFeatures>(pCreateInfo->pNext);
        if (timeline_semaphore_features) {
            state_tracker->enabled_features.core12.timelineSemaphore = timeline_semaphore_features->timelineSemaphore;
        }

        const auto *buffer_device_address = LvlFindInChain<VkPhysicalDeviceBufferDeviceAddressFeatures>(pCreateInfo->pNext);
        if (buffer_device_address) {
            state_tracker->enabled_features.core12.bufferDeviceAddress = buffer_device_address->bufferDeviceAddress;
            state_tracker->enabled_features.core12.bufferDeviceAddressCaptureReplay =
                buffer_device_address->bufferDeviceAddressCaptureReplay;
            state_tracker->enabled_features.core12.bufferDeviceAddressMultiDevice =
                buffer_device_address->bufferDeviceAddressMultiDevice;
        }

        const auto *atomic_int64_features = LvlFindInChain<VkPhysicalDeviceShaderAtomicInt64Features>(pCreateInfo->pNext);
        if (atomic_int64_features) {
            state_tracker->enabled_features.core12.shaderBufferInt64Atomics = atomic_int64_features->shaderBufferInt64Atomics;
            state_tracker->enabled_features.core12.shaderSharedInt64Atomics = atomic_int64_features->shaderSharedInt64Atomics;
        }

        const auto *memory_model_features = LvlFindInChain<VkPhysicalDeviceVulkanMemoryModelFeatures>(pCreateInfo->pNext);
        if (memory_model_features) {
            state_tracker->enabled_features.core12.vulkanMemoryModel = memory_model_features->vulkanMemoryModel;
            state_tracker->enabled_features.core12.vulkanMemoryModelDeviceScope =
                memory_model_features->vulkanMemoryModelDeviceScope;
            state_tracker->enabled_features.core12.vulkanMemoryModelAvailabilityVisibilityChains =
                memory_model_features->vulkanMemoryModelAvailabilityVisibilityChains;
        }
    }

    const auto *vulkan_11_features = LvlFindInChain<VkPhysicalDeviceVulkan11Features>(pCreateInfo->pNext);
    if (vulkan_11_features) {
        state_tracker->enabled_features.core11 = *vulkan_11_features;
    } else {
        // These structs are only allowed in pNext chain if there is no vkPhysicalDeviceVulkan11Features

        const auto *sixteen_bit_storage_features = LvlFindInChain<VkPhysicalDevice16BitStorageFeatures>(pCreateInfo->pNext);
        if (sixteen_bit_storage_features) {
            state_tracker->enabled_features.core11.storageBuffer16BitAccess =
                sixteen_bit_storage_features->storageBuffer16BitAccess;
            state_tracker->enabled_features.core11.uniformAndStorageBuffer16BitAccess =
                sixteen_bit_storage_features->uniformAndStorageBuffer16BitAccess;
            state_tracker->enabled_features.core11.storagePushConstant16 = sixteen_bit_storage_features->storagePushConstant16;
            state_tracker->enabled_features.core11.storageInputOutput16 = sixteen_bit_storage_features->storageInputOutput16;
        }

        const auto *multiview_features = LvlFindInChain<VkPhysicalDeviceMultiviewFeatures>(pCreateInfo->pNext);
        if (multiview_features) {
            state_tracker->enabled_features.core11.multiview = multiview_features->multiview;
            state_tracker->enabled_features.core11.multiviewGeometryShader = multiview_features->multiviewGeometryShader;
            state_tracker->enabled_features.core11.multiviewTessellationShader = multiview_features->multiviewTessellationShader;
        }

        const auto *variable_pointers_features = LvlFindInChain<VkPhysicalDeviceVariablePointersFeatures>(pCreateInfo->pNext);
        if (variable_pointers_features) {
            state_tracker->enabled_features.core11.variablePointersStorageBuffer =
                variable_pointers_features->variablePointersStorageBuffer;
            state_tracker->enabled_features.core11.variablePointers = variable_pointers_features->variablePointers;
        }

        const auto *protected_memory_features = LvlFindInChain<VkPhysicalDeviceProtectedMemoryFeatures>(pCreateInfo->pNext);
        if (protected_memory_features) {
            state_tracker->enabled_features.core11.protectedMemory = protected_memory_features->protectedMemory;
        }

        const auto *ycbcr_conversion_features = LvlFindInChain<VkPhysicalDeviceSamplerYcbcrConversionFeatures>(pCreateInfo->pNext);
        if (ycbcr_conversion_features) {
            state_tracker->enabled_features.core11.samplerYcbcrConversion = ycbcr_conversion_features->samplerYcbcrConversion;
        }

        const auto *shader_draw_parameters_features =
            LvlFindInChain<VkPhysicalDeviceShaderDrawParametersFeatures>(pCreateInfo->pNext);
        if (shader_draw_parameters_features) {
            state_tracker->enabled_features.core11.shaderDrawParameters = shader_draw_parameters_features->shaderDrawParameters;
        }
    }

    const auto *device_group_ci = LvlFindInChain<VkDeviceGroupDeviceCreateInfo>(pCreateInfo->pNext);
    if (device_group_ci) {
        state_tracker->physical_device_count = device_group_ci->physicalDeviceCount;
        state_tracker->device_group_create_info = *device_group_ci;
    } else {
        state_tracker->physical_device_count = 1;
    }

    // Features from other extensions passesd in create info
    {
        const auto *exclusive_scissor_features = LvlFindInChain<VkPhysicalDeviceExclusiveScissorFeaturesNV>(pCreateInfo->pNext);
        if (exclusive_scissor_features) {
            state_tracker->enabled_features.exclusive_scissor_features = *exclusive_scissor_features;
        }

        const auto *shading_rate_image_features = LvlFindInChain<VkPhysicalDeviceShadingRateImageFeaturesNV>(pCreateInfo->pNext);
        if (shading_rate_image_features) {
            state_tracker->enabled_features.shading_rate_image_features = *shading_rate_image_features;
        }

        const auto *mesh_shader_features = LvlFindInChain<VkPhysicalDeviceMeshShaderFeaturesNV>(pCreateInfo->pNext);
        if (mesh_shader_features) {
            state_tracker->enabled_features.mesh_shader_features = *mesh_shader_features;
        }

        const auto *inline_uniform_block_features =
            LvlFindInChain<VkPhysicalDeviceInlineUniformBlockFeaturesEXT>(pCreateInfo->pNext);
        if (inline_uniform_block_features) {
            state_tracker->enabled_features.inline_uniform_block_features = *inline_uniform_block_features;
        }

        const auto *transform_feedback_features = LvlFindInChain<VkPhysicalDeviceTransformFeedbackFeaturesEXT>(pCreateInfo->pNext);
        if (transform_feedback_features) {
            state_tracker->enabled_features.transform_feedback_features = *transform_feedback_features;
        }

        const auto *vtx_attrib_div_features = LvlFindInChain<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT>(pCreateInfo->pNext);
        if (vtx_attrib_div_features) {
            state_tracker->enabled_features.vtx_attrib_divisor_features = *vtx_attrib_div_features;
        }

        const auto *buffer_device_address_ext_features =
            LvlFindInChain<VkPhysicalDeviceBufferDeviceAddressFeaturesEXT>(pCreateInfo->pNext);
        if (buffer_device_address_ext_features) {
            state_tracker->enabled_features.buffer_device_address_ext_features = *buffer_device_address_ext_features;
        }

        const auto *cooperative_matrix_features = LvlFindInChain<VkPhysicalDeviceCooperativeMatrixFeaturesNV>(pCreateInfo->pNext);
        if (cooperative_matrix_features) {
            state_tracker->enabled_features.cooperative_matrix_features = *cooperative_matrix_features;
        }

        const auto *compute_shader_derivatives_features =
            LvlFindInChain<VkPhysicalDeviceComputeShaderDerivativesFeaturesNV>(pCreateInfo->pNext);
        if (compute_shader_derivatives_features) {
            state_tracker->enabled_features.compute_shader_derivatives_features = *compute_shader_derivatives_features;
        }

        const auto *fragment_shader_barycentric_features =
            LvlFindInChain<VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV>(pCreateInfo->pNext);
        if (fragment_shader_barycentric_features) {
            state_tracker->enabled_features.fragment_shader_barycentric_features = *fragment_shader_barycentric_features;
        }

        const auto *shader_image_footprint_features =
            LvlFindInChain<VkPhysicalDeviceShaderImageFootprintFeaturesNV>(pCreateInfo->pNext);
        if (shader_image_footprint_features) {
            state_tracker->enabled_features.shader_image_footprint_features = *shader_image_footprint_features;
        }

        const auto *fragment_shader_interlock_features =
            LvlFindInChain<VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT>(pCreateInfo->pNext);
        if (fragment_shader_interlock_features) {
            state_tracker->enabled_features.fragment_shader_interlock_features = *fragment_shader_interlock_features;
        }

        const auto *demote_to_helper_invocation_features =
            LvlFindInChain<VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT>(pCreateInfo->pNext);
        if (demote_to_helper_invocation_features) {
            state_tracker->enabled_features.demote_to_helper_invocation_features = *demote_to_helper_invocation_features;
        }

        const auto *texel_buffer_alignment_features =
            LvlFindInChain<VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT>(pCreateInfo->pNext);
        if (texel_buffer_alignment_features) {
            state_tracker->enabled_features.texel_buffer_alignment_features = *texel_buffer_alignment_features;
        }

        const auto *pipeline_exe_props_features =
            LvlFindInChain<VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR>(pCreateInfo->pNext);
        if (pipeline_exe_props_features) {
            state_tracker->enabled_features.pipeline_exe_props_features = *pipeline_exe_props_features;
        }

        const auto *dedicated_allocation_image_aliasing_features =
            LvlFindInChain<VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV>(pCreateInfo->pNext);
        if (dedicated_allocation_image_aliasing_features) {
            state_tracker->enabled_features.dedicated_allocation_image_aliasing_features =
                *dedicated_allocation_image_aliasing_features;
        }

        const auto *performance_query_features = LvlFindInChain<VkPhysicalDevicePerformanceQueryFeaturesKHR>(pCreateInfo->pNext);
        if (performance_query_features) {
            state_tracker->enabled_features.performance_query_features = *performance_query_features;
        }

        const auto *device_coherent_memory_features = LvlFindInChain<VkPhysicalDeviceCoherentMemoryFeaturesAMD>(pCreateInfo->pNext);
        if (device_coherent_memory_features) {
            state_tracker->enabled_features.device_coherent_memory_features = *device_coherent_memory_features;
        }

        const auto *ycbcr_image_array_features = LvlFindInChain<VkPhysicalDeviceYcbcrImageArraysFeaturesEXT>(pCreateInfo->pNext);
        if (ycbcr_image_array_features) {
            state_tracker->enabled_features.ycbcr_image_array_features = *ycbcr_image_array_features;
        }

        const auto *ray_query_features = LvlFindInChain<VkPhysicalDeviceRayQueryFeaturesKHR>(pCreateInfo->pNext);
        if (ray_query_features) {
            state_tracker->enabled_features.ray_query_features = *ray_query_features;
        }

        const auto *ray_tracing_pipeline_features =
            LvlFindInChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(pCreateInfo->pNext);
        if (ray_tracing_pipeline_features) {
            state_tracker->enabled_features.ray_tracing_pipeline_features = *ray_tracing_pipeline_features;
        }

        const auto *ray_tracing_acceleration_structure_features =
            LvlFindInChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(pCreateInfo->pNext);
        if (ray_tracing_acceleration_structure_features) {
            state_tracker->enabled_features.ray_tracing_acceleration_structure_features =
                *ray_tracing_acceleration_structure_features;
        }

        const auto *robustness2_features = LvlFindInChain<VkPhysicalDeviceRobustness2FeaturesEXT>(pCreateInfo->pNext);
        if (robustness2_features) {
            state_tracker->enabled_features.robustness2_features = *robustness2_features;
        }

        const auto *fragment_density_map_features =
            LvlFindInChain<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>(pCreateInfo->pNext);
        if (fragment_density_map_features) {
            state_tracker->enabled_features.fragment_density_map_features = *fragment_density_map_features;
        }

        const auto *fragment_density_map_features2 =
            LvlFindInChain<VkPhysicalDeviceFragmentDensityMap2FeaturesEXT>(pCreateInfo->pNext);
        if (fragment_density_map_features2) {
            state_tracker->enabled_features.fragment_density_map2_features = *fragment_density_map_features2;
        }

        const auto *astc_decode_features = LvlFindInChain<VkPhysicalDeviceASTCDecodeFeaturesEXT>(pCreateInfo->pNext);
        if (astc_decode_features) {
            state_tracker->enabled_features.astc_decode_features = *astc_decode_features;
        }

        const auto *custom_border_color_features = LvlFindInChain<VkPhysicalDeviceCustomBorderColorFeaturesEXT>(pCreateInfo->pNext);
        if (custom_border_color_features) {
            state_tracker->enabled_features.custom_border_color_features = *custom_border_color_features;
        }

        const auto *pipeline_creation_cache_control_features =
            LvlFindInChain<VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT>(pCreateInfo->pNext);
        if (pipeline_creation_cache_control_features) {
            state_tracker->enabled_features.pipeline_creation_cache_control_features = *pipeline_creation_cache_control_features;
        }

        const auto *fragment_shading_rate_features =
            LvlFindInChain<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>(pCreateInfo->pNext);
        if (fragment_shading_rate_features) {
            state_tracker->enabled_features.fragment_shading_rate_features = *fragment_shading_rate_features;
        }

        const auto *extended_dynamic_state_features =
            LvlFindInChain<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT>(pCreateInfo->pNext);
        if (extended_dynamic_state_features) {
            state_tracker->enabled_features.extended_dynamic_state_features = *extended_dynamic_state_features;
        }

        const auto *extended_dynamic_state2_features =
            LvlFindInChain<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>(pCreateInfo->pNext);
        if (extended_dynamic_state2_features) {
            state_tracker->enabled_features.extended_dynamic_state2_features = *extended_dynamic_state2_features;
        }

        const auto *multiview_features = LvlFindInChain<VkPhysicalDeviceMultiviewFeatures>(pCreateInfo->pNext);
        if (multiview_features) {
            state_tracker->enabled_features.multiview_features = *multiview_features;
        }

        const auto *portability_features = LvlFindInChain<VkPhysicalDevicePortabilitySubsetFeaturesKHR>(pCreateInfo->pNext);
        if (portability_features) {
            state_tracker->enabled_features.portability_subset_features = *portability_features;
        }

        const auto *shader_integer_functions2_features =
            LvlFindInChain<VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL>(pCreateInfo->pNext);
        if (shader_integer_functions2_features) {
            state_tracker->enabled_features.shader_integer_functions2_features = *shader_integer_functions2_features;
        }

        const auto *shader_sm_builtins_features = LvlFindInChain<VkPhysicalDeviceShaderSMBuiltinsFeaturesNV>(pCreateInfo->pNext);
        if (shader_sm_builtins_features) {
            state_tracker->enabled_features.shader_sm_builtins_features = *shader_sm_builtins_features;
        }

        const auto *shader_atomic_float_features = LvlFindInChain<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>(pCreateInfo->pNext);
        if (shader_atomic_float_features) {
            state_tracker->enabled_features.shader_atomic_float_features = *shader_atomic_float_features;
        }

        const auto *shader_image_atomic_int64_features =
            LvlFindInChain<VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT>(pCreateInfo->pNext);
        if (shader_image_atomic_int64_features) {
            state_tracker->enabled_features.shader_image_atomic_int64_features = *shader_image_atomic_int64_features;
        }

        const auto *shader_clock_features = LvlFindInChain<VkPhysicalDeviceShaderClockFeaturesKHR>(pCreateInfo->pNext);
        if (shader_clock_features) {
            state_tracker->enabled_features.shader_clock_features = *shader_clock_features;
        }

        const auto *conditional_rendering_features =
            LvlFindInChain<VkPhysicalDeviceConditionalRenderingFeaturesEXT>(pCreateInfo->pNext);
        if (conditional_rendering_features) {
            state_tracker->enabled_features.conditional_rendering_features = *conditional_rendering_features;
        }

        const auto *workgroup_memory_explicit_layout_features =
            LvlFindInChain<VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR>(pCreateInfo->pNext);
        if (workgroup_memory_explicit_layout_features) {
            state_tracker->enabled_features.workgroup_memory_explicit_layout_features = *workgroup_memory_explicit_layout_features;
        }

        const auto *synchronization2_features = LvlFindInChain<VkPhysicalDeviceSynchronization2FeaturesKHR>(pCreateInfo->pNext);
        if (synchronization2_features) {
            state_tracker->enabled_features.synchronization2_features = *synchronization2_features;
        }

        const auto *provoking_vertex_features = lvl_find_in_chain<VkPhysicalDeviceProvokingVertexFeaturesEXT>(pCreateInfo->pNext);
        if (provoking_vertex_features) {
            state_tracker->enabled_features.provoking_vertex_features = *provoking_vertex_features;
        }

        const auto *vertex_input_dynamic_state_features =
            LvlFindInChain<VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT>(pCreateInfo->pNext);
        if (vertex_input_dynamic_state_features) {
            state_tracker->enabled_features.vertex_input_dynamic_state_features = *vertex_input_dynamic_state_features;
        }

        const auto *inherited_viewport_scissor_features =
            LvlFindInChain<VkPhysicalDeviceInheritedViewportScissorFeaturesNV>(pCreateInfo->pNext);
        if (inherited_viewport_scissor_features) {
            state_tracker->enabled_features.inherited_viewport_scissor_features = *inherited_viewport_scissor_features;
        }

        const auto *multi_draw_features = LvlFindInChain<VkPhysicalDeviceMultiDrawFeaturesEXT>(pCreateInfo->pNext);
        if (multi_draw_features) {
            state_tracker->enabled_features.multi_draw_features = *multi_draw_features;
        }

        const auto *color_write_features = LvlFindInChain<VkPhysicalDeviceColorWriteEnableFeaturesEXT>(pCreateInfo->pNext);
        if (color_write_features) {
            state_tracker->enabled_features.color_write_features = *color_write_features;
        }

        const auto *shader_atomic_float2_features =
            LvlFindInChain<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT>(pCreateInfo->pNext);
        if (shader_atomic_float2_features) {
            state_tracker->enabled_features.shader_atomic_float2_features = *shader_atomic_float2_features;
        }

        const auto *present_id_features = LvlFindInChain<VkPhysicalDevicePresentIdFeaturesKHR>(pCreateInfo->pNext);
        if (present_id_features) {
            state_tracker->enabled_features.present_id_features = *present_id_features;
        }

        const auto *present_wait_features = LvlFindInChain<VkPhysicalDevicePresentWaitFeaturesKHR>(pCreateInfo->pNext);
        if (present_wait_features) {
            state_tracker->enabled_features.present_wait_features = *present_wait_features;
        }

        const auto *ray_tracing_motion_blur_features =
            LvlFindInChain<VkPhysicalDeviceRayTracingMotionBlurFeaturesNV>(pCreateInfo->pNext);
        if (ray_tracing_motion_blur_features) {
            state_tracker->enabled_features.ray_tracing_motion_blur_features = *ray_tracing_motion_blur_features;
        }

        const auto *shader_integer_dot_product_features =
            LvlFindInChain<VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR>(pCreateInfo->pNext);
        if (shader_integer_dot_product_features) {
            state_tracker->enabled_features.shader_integer_dot_product_features = *shader_integer_dot_product_features;
        }

        const auto *primitive_topology_list_restart_features =
            LvlFindInChain<VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT>(pCreateInfo->pNext);
        if (primitive_topology_list_restart_features) {
            state_tracker->enabled_features.primitive_topology_list_restart_features = *primitive_topology_list_restart_features;
        }
    }

    const auto *subgroup_size_control_features = LvlFindInChain<VkPhysicalDeviceSubgroupSizeControlFeaturesEXT>(pCreateInfo->pNext);
    if (subgroup_size_control_features) {
        state_tracker->enabled_features.subgroup_size_control_features = *subgroup_size_control_features;
    }

    // Store physical device properties and physical device mem limits into CoreChecks structs
    DispatchGetPhysicalDeviceMemoryProperties(gpu, &state_tracker->phys_dev_mem_props);
    DispatchGetPhysicalDeviceProperties(gpu, &state_tracker->phys_dev_props);

    const auto &dev_ext = state_tracker->device_extensions;
    auto *phys_dev_props = &state_tracker->phys_dev_ext_props;

    // Vulkan 1.2 can get properties from single struct, otherwise need to add to it per extension
    if (dev_ext.vk_feature_version_1_2) {
        GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_feature_version_1_2, &state_tracker->phys_dev_props_core11);
        GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_feature_version_1_2, &state_tracker->phys_dev_props_core12);
    } else {
        // VkPhysicalDeviceVulkan11Properties
        //
        // Can ingnore VkPhysicalDeviceIDProperties as it has no validation purpose

        if (dev_ext.vk_khr_multiview) {
            auto multiview_props = LvlInitStruct<VkPhysicalDeviceMultiviewProperties>();
            GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_khr_multiview, &multiview_props);
            state_tracker->phys_dev_props_core11.maxMultiviewViewCount = multiview_props.maxMultiviewViewCount;
            state_tracker->phys_dev_props_core11.maxMultiviewInstanceIndex = multiview_props.maxMultiviewInstanceIndex;
        }

        if (dev_ext.vk_khr_maintenance3) {
            auto maintenance3_props = LvlInitStruct<VkPhysicalDeviceMaintenance3Properties>();
            GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_khr_maintenance3, &maintenance3_props);
            state_tracker->phys_dev_props_core11.maxPerSetDescriptors = maintenance3_props.maxPerSetDescriptors;
            state_tracker->phys_dev_props_core11.maxMemoryAllocationSize = maintenance3_props.maxMemoryAllocationSize;
        }

        // Some 1.1 properties were added to core without previous extensions
        if (state_tracker->api_version >= VK_API_VERSION_1_1) {
            auto subgroup_prop = LvlInitStruct<VkPhysicalDeviceSubgroupProperties>();
            auto protected_memory_prop = LvlInitStruct<VkPhysicalDeviceProtectedMemoryProperties>(&subgroup_prop);
            auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&protected_memory_prop);
            instance_dispatch_table.GetPhysicalDeviceProperties2(gpu, &prop2);

            state_tracker->phys_dev_props_core11.subgroupSize = subgroup_prop.subgroupSize;
            state_tracker->phys_dev_props_core11.subgroupSupportedStages = subgroup_prop.supportedStages;
            state_tracker->phys_dev_props_core11.subgroupSupportedOperations = subgroup_prop.supportedOperations;
            state_tracker->phys_dev_props_core11.subgroupQuadOperationsInAllStages = subgroup_prop.quadOperationsInAllStages;

            state_tracker->phys_dev_props_core11.protectedNoFault = protected_memory_prop.protectedNoFault;
        }

        // VkPhysicalDeviceVulkan12Properties
        //
        // Can ingnore VkPhysicalDeviceDriverProperties as it has no validation purpose

        if (dev_ext.vk_ext_descriptor_indexing) {
            auto descriptor_indexing_prop = LvlInitStruct<VkPhysicalDeviceDescriptorIndexingProperties>();
            GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_ext_descriptor_indexing, &descriptor_indexing_prop);
            state_tracker->phys_dev_props_core12.maxUpdateAfterBindDescriptorsInAllPools =
                descriptor_indexing_prop.maxUpdateAfterBindDescriptorsInAllPools;
            state_tracker->phys_dev_props_core12.shaderUniformBufferArrayNonUniformIndexingNative =
                descriptor_indexing_prop.shaderUniformBufferArrayNonUniformIndexingNative;
            state_tracker->phys_dev_props_core12.shaderSampledImageArrayNonUniformIndexingNative =
                descriptor_indexing_prop.shaderSampledImageArrayNonUniformIndexingNative;
            state_tracker->phys_dev_props_core12.shaderStorageBufferArrayNonUniformIndexingNative =
                descriptor_indexing_prop.shaderStorageBufferArrayNonUniformIndexingNative;
            state_tracker->phys_dev_props_core12.shaderStorageImageArrayNonUniformIndexingNative =
                descriptor_indexing_prop.shaderStorageImageArrayNonUniformIndexingNative;
            state_tracker->phys_dev_props_core12.shaderInputAttachmentArrayNonUniformIndexingNative =
                descriptor_indexing_prop.shaderInputAttachmentArrayNonUniformIndexingNative;
            state_tracker->phys_dev_props_core12.robustBufferAccessUpdateAfterBind =
                descriptor_indexing_prop.robustBufferAccessUpdateAfterBind;
            state_tracker->phys_dev_props_core12.quadDivergentImplicitLod = descriptor_indexing_prop.quadDivergentImplicitLod;
            state_tracker->phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindSamplers =
                descriptor_indexing_prop.maxPerStageDescriptorUpdateAfterBindSamplers;
            state_tracker->phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindUniformBuffers =
                descriptor_indexing_prop.maxPerStageDescriptorUpdateAfterBindUniformBuffers;
            state_tracker->phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindStorageBuffers =
                descriptor_indexing_prop.maxPerStageDescriptorUpdateAfterBindStorageBuffers;
            state_tracker->phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindSampledImages =
                descriptor_indexing_prop.maxPerStageDescriptorUpdateAfterBindSampledImages;
            state_tracker->phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindStorageImages =
                descriptor_indexing_prop.maxPerStageDescriptorUpdateAfterBindStorageImages;
            state_tracker->phys_dev_props_core12.maxPerStageDescriptorUpdateAfterBindInputAttachments =
                descriptor_indexing_prop.maxPerStageDescriptorUpdateAfterBindInputAttachments;
            state_tracker->phys_dev_props_core12.maxPerStageUpdateAfterBindResources =
                descriptor_indexing_prop.maxPerStageUpdateAfterBindResources;
            state_tracker->phys_dev_props_core12.maxDescriptorSetUpdateAfterBindSamplers =
                descriptor_indexing_prop.maxDescriptorSetUpdateAfterBindSamplers;
            state_tracker->phys_dev_props_core12.maxDescriptorSetUpdateAfterBindUniformBuffers =
                descriptor_indexing_prop.maxDescriptorSetUpdateAfterBindUniformBuffers;
            state_tracker->phys_dev_props_core12.maxDescriptorSetUpdateAfterBindUniformBuffersDynamic =
                descriptor_indexing_prop.maxDescriptorSetUpdateAfterBindUniformBuffersDynamic;
            state_tracker->phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageBuffers =
                descriptor_indexing_prop.maxDescriptorSetUpdateAfterBindStorageBuffers;
            state_tracker->phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageBuffersDynamic =
                descriptor_indexing_prop.maxDescriptorSetUpdateAfterBindStorageBuffersDynamic;
            state_tracker->phys_dev_props_core12.maxDescriptorSetUpdateAfterBindSampledImages =
                descriptor_indexing_prop.maxDescriptorSetUpdateAfterBindSampledImages;
            state_tracker->phys_dev_props_core12.maxDescriptorSetUpdateAfterBindStorageImages =
                descriptor_indexing_prop.maxDescriptorSetUpdateAfterBindStorageImages;
            state_tracker->phys_dev_props_core12.maxDescriptorSetUpdateAfterBindInputAttachments =
                descriptor_indexing_prop.maxDescriptorSetUpdateAfterBindInputAttachments;
        }

        if (dev_ext.vk_khr_depth_stencil_resolve) {
            auto depth_stencil_resolve_props = LvlInitStruct<VkPhysicalDeviceDepthStencilResolveProperties>();
            GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_khr_depth_stencil_resolve, &depth_stencil_resolve_props);
            state_tracker->phys_dev_props_core12.supportedDepthResolveModes =
                depth_stencil_resolve_props.supportedDepthResolveModes;
            state_tracker->phys_dev_props_core12.supportedStencilResolveModes =
                depth_stencil_resolve_props.supportedStencilResolveModes;
            state_tracker->phys_dev_props_core12.independentResolveNone = depth_stencil_resolve_props.independentResolveNone;
            state_tracker->phys_dev_props_core12.independentResolve = depth_stencil_resolve_props.independentResolve;
        }

        if (dev_ext.vk_khr_timeline_semaphore) {
            auto timeline_semaphore_props = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreProperties>();
            GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_khr_timeline_semaphore, &timeline_semaphore_props);
            state_tracker->phys_dev_props_core12.maxTimelineSemaphoreValueDifference =
                timeline_semaphore_props.maxTimelineSemaphoreValueDifference;
        }

        if (dev_ext.vk_ext_sampler_filter_minmax) {
            auto sampler_filter_minmax_props = LvlInitStruct<VkPhysicalDeviceSamplerFilterMinmaxProperties>();
            GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_ext_sampler_filter_minmax, &sampler_filter_minmax_props);
            state_tracker->phys_dev_props_core12.filterMinmaxSingleComponentFormats =
                sampler_filter_minmax_props.filterMinmaxSingleComponentFormats;
            state_tracker->phys_dev_props_core12.filterMinmaxImageComponentMapping =
                sampler_filter_minmax_props.filterMinmaxImageComponentMapping;
        }

        if (dev_ext.vk_khr_shader_float_controls) {
            auto float_controls_props = LvlInitStruct<VkPhysicalDeviceFloatControlsProperties>();
            GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_khr_shader_float_controls, &float_controls_props);
            state_tracker->phys_dev_props_core12.denormBehaviorIndependence = float_controls_props.denormBehaviorIndependence;
            state_tracker->phys_dev_props_core12.roundingModeIndependence = float_controls_props.roundingModeIndependence;
            state_tracker->phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat16 =
                float_controls_props.shaderSignedZeroInfNanPreserveFloat16;
            state_tracker->phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat32 =
                float_controls_props.shaderSignedZeroInfNanPreserveFloat32;
            state_tracker->phys_dev_props_core12.shaderSignedZeroInfNanPreserveFloat64 =
                float_controls_props.shaderSignedZeroInfNanPreserveFloat64;
            state_tracker->phys_dev_props_core12.shaderDenormPreserveFloat16 = float_controls_props.shaderDenormPreserveFloat16;
            state_tracker->phys_dev_props_core12.shaderDenormPreserveFloat32 = float_controls_props.shaderDenormPreserveFloat32;
            state_tracker->phys_dev_props_core12.shaderDenormPreserveFloat64 = float_controls_props.shaderDenormPreserveFloat64;
            state_tracker->phys_dev_props_core12.shaderDenormFlushToZeroFloat16 =
                float_controls_props.shaderDenormFlushToZeroFloat16;
            state_tracker->phys_dev_props_core12.shaderDenormFlushToZeroFloat32 =
                float_controls_props.shaderDenormFlushToZeroFloat32;
            state_tracker->phys_dev_props_core12.shaderDenormFlushToZeroFloat64 =
                float_controls_props.shaderDenormFlushToZeroFloat64;
            state_tracker->phys_dev_props_core12.shaderRoundingModeRTEFloat16 = float_controls_props.shaderRoundingModeRTEFloat16;
            state_tracker->phys_dev_props_core12.shaderRoundingModeRTEFloat32 = float_controls_props.shaderRoundingModeRTEFloat32;
            state_tracker->phys_dev_props_core12.shaderRoundingModeRTEFloat64 = float_controls_props.shaderRoundingModeRTEFloat64;
            state_tracker->phys_dev_props_core12.shaderRoundingModeRTZFloat16 = float_controls_props.shaderRoundingModeRTZFloat16;
            state_tracker->phys_dev_props_core12.shaderRoundingModeRTZFloat32 = float_controls_props.shaderRoundingModeRTZFloat32;
            state_tracker->phys_dev_props_core12.shaderRoundingModeRTZFloat64 = float_controls_props.shaderRoundingModeRTZFloat64;
        }
    }

    // Extensions with properties to extract to DeviceExtensionProperties
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_khr_push_descriptor, &phys_dev_props->push_descriptor_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_nv_shading_rate_image, &phys_dev_props->shading_rate_image_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_nv_mesh_shader, &phys_dev_props->mesh_shader_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_ext_inline_uniform_block, &phys_dev_props->inline_uniform_block_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_ext_vertex_attribute_divisor, &phys_dev_props->vtx_attrib_divisor_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_ext_transform_feedback, &phys_dev_props->transform_feedback_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_nv_ray_tracing, &phys_dev_props->ray_tracing_propsNV);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_khr_ray_tracing_pipeline, &phys_dev_props->ray_tracing_propsKHR);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_khr_acceleration_structure, &phys_dev_props->acc_structure_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_ext_texel_buffer_alignment, &phys_dev_props->texel_buffer_alignment_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_ext_fragment_density_map, &phys_dev_props->fragment_density_map_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_ext_fragment_density_map2, &phys_dev_props->fragment_density_map2_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_khr_performance_query, &phys_dev_props->performance_query_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_ext_sample_locations, &phys_dev_props->sample_locations_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_ext_custom_border_color, &phys_dev_props->custom_border_color_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_khr_multiview, &phys_dev_props->multiview_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_khr_portability_subset, &phys_dev_props->portability_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_khr_fragment_shading_rate, &phys_dev_props->fragment_shading_rate_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_ext_provoking_vertex, &phys_dev_props->provoking_vertex_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_ext_multi_draw, &phys_dev_props->multi_draw_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_ext_discard_rectangles, &phys_dev_props->discard_rectangle_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_ext_blend_operation_advanced, &phys_dev_props->blend_operation_advanced_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_ext_conservative_rasterization, &phys_dev_props->conservative_rasterization_props);
    GetPhysicalDeviceExtProperties(gpu, dev_ext.vk_ext_subgroup_size_control, &phys_dev_props->subgroup_size_control_props);

    if (IsExtEnabled(dev_ext.vk_nv_cooperative_matrix)) {
        // Get the needed cooperative_matrix properties
        auto cooperative_matrix_props = LvlInitStruct<VkPhysicalDeviceCooperativeMatrixPropertiesNV>();
        auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&cooperative_matrix_props);
        instance_dispatch_table.GetPhysicalDeviceProperties2KHR(gpu, &prop2);
        state_tracker->phys_dev_ext_props.cooperative_matrix_props = cooperative_matrix_props;

        uint32_t num_cooperative_matrix_properties = 0;
        instance_dispatch_table.GetPhysicalDeviceCooperativeMatrixPropertiesNV(gpu, &num_cooperative_matrix_properties, NULL);
        state_tracker->cooperative_matrix_properties.resize(num_cooperative_matrix_properties,
                                                            LvlInitStruct<VkCooperativeMatrixPropertiesNV>());

        instance_dispatch_table.GetPhysicalDeviceCooperativeMatrixPropertiesNV(gpu, &num_cooperative_matrix_properties,
                                                                               state_tracker->cooperative_matrix_properties.data());
    }

    // Store queue family data
    if (pCreateInfo->pQueueCreateInfos != nullptr) {
        for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; ++i) {
            const VkDeviceQueueCreateInfo &queue_create_info = pCreateInfo->pQueueCreateInfos[i];
            state_tracker->queue_family_index_set.insert(queue_create_info.queueFamilyIndex);
            state_tracker->device_queue_info_list.push_back(
                {i, queue_create_info.queueFamilyIndex, queue_create_info.flags, queue_create_info.queueCount});
        }
    }
}

void ValidationStateTracker::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    if (!device) return;

    // Reset all command buffers before destroying them, to unlink object_bindings.
    for (auto &command_buffer : commandBufferMap) {
        command_buffer.second->Reset();
    }
    pipelineMap.clear();
    renderPassMap.clear();
    commandBufferMap.clear();

    // This will also delete all sets in the pool & remove them from setMap
    DeleteDescriptorSetPools();
    // All sets should be removed
    assert(setMap.empty());
    descriptorSetLayoutMap.clear();
    // Because swapchains are associated with Surfaces, which are at instance level,
    // they need to be explicitly destroyed here to avoid continued references to
    // the device we're destroying.
    for (auto &entry : swapchainMap) {
        entry.second->Destroy();
    }
    swapchainMap.clear();
    imageViewMap.clear();
    imageMap.clear();
    bufferViewMap.clear();
    bufferMap.clear();
    // Queues persist until device is destroyed
    queueMap.clear();
}

void ValidationStateTracker::RetireWorkOnQueue(QUEUE_STATE *pQueue, uint64_t seq) {
    layer_data::unordered_map<VkQueue, uint64_t> other_queue_seqs;
    layer_data::unordered_map<VkSemaphore, uint64_t> timeline_semaphore_counters;

    // Roll this queue forward, one submission at a time.
    while (pQueue->seq < seq) {
        auto &submission = pQueue->submissions.front();

        for (auto &wait : submission.waitSemaphores) {
            auto semaphore_state = GetSemaphoreState(wait.semaphore);
            if (semaphore_state) {
                semaphore_state->EndUse();
            }
            if (wait.type == VK_SEMAPHORE_TYPE_TIMELINE) {
                auto &last_counter = timeline_semaphore_counters[wait.semaphore];
                last_counter = std::max(last_counter, wait.payload);
            } else {
                auto &last_seq = other_queue_seqs[wait.queue];
                last_seq = std::max(last_seq, wait.seq);
            }
        }

        for (auto &signal : submission.signalSemaphores) {
            auto semaphore_state = GetSemaphoreState(signal.semaphore);
            if (semaphore_state) {
                semaphore_state->EndUse();
                if (semaphore_state->type == VK_SEMAPHORE_TYPE_TIMELINE && semaphore_state->payload < signal.payload) {
                    semaphore_state->payload = signal.payload;
                }
            }
        }

        for (auto &semaphore : submission.externalSemaphores) {
            auto semaphore_state = GetSemaphoreState(semaphore);
            if (semaphore_state) {
                semaphore_state->EndUse();
            }
        }

        for (auto cb : submission.cbs) {
            auto cb_node = Get<CMD_BUFFER_STATE>(cb);
            if (!cb_node) {
                continue;
            }
            // First perform decrement on general case bound objects
            for (auto event : cb_node->writeEventsBeforeWait) {
                auto event_node = eventMap.find(event);
                if (event_node != eventMap.end()) {
                    event_node->second->write_in_use--;
                }
            }
            QueryMap local_query_to_state_map;
            VkQueryPool first_pool = VK_NULL_HANDLE;
            for (auto &function : cb_node->queryUpdates) {
                function(nullptr, /*do_validate*/ false, first_pool, submission.perf_submit_pass, &local_query_to_state_map);
            }

            for (const auto &query_state_pair : local_query_to_state_map) {
                if (query_state_pair.second == QUERYSTATE_ENDED) {
                    queryToStateMap[query_state_pair.first] = QUERYSTATE_AVAILABLE;
                }
            }
            if (cb_node->createInfo.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
                cb_node->EndUse();
            }
        }

        auto fence_state = GetFenceState(submission.fence);
        if (fence_state && fence_state->scope == kSyncScopeInternal) {
            fence_state->state = FENCE_RETIRED;
        }

        pQueue->submissions.pop_front();
        pQueue->seq++;
    }

    // Roll other queues forward to the highest seq we saw a wait for
    for (const auto &qs : other_queue_seqs) {
        RetireWorkOnQueue(GetQueueState(qs.first), qs.second);
    }
    for (const auto &sc : timeline_semaphore_counters) {
        RetireTimelineSemaphore(sc.first, sc.second);
    }
}

// Submit a fence to a queue, delimiting previous fences and previous untracked
// work by it.
static void SubmitFence(QUEUE_STATE *pQueue, FENCE_STATE *pFence, uint64_t submitCount) {
    pFence->state = FENCE_INFLIGHT;
    pFence->signaler.first = pQueue->Queue();
    pFence->signaler.second = pQueue->seq + pQueue->submissions.size() + submitCount;
}

uint64_t ValidationStateTracker::RecordSubmitFence(QUEUE_STATE *queue_state, VkFence fence, uint32_t submit_count) {
    auto fence_state = GetFenceState(fence);
    uint64_t early_retire_seq = 0;
    if (fence_state) {
        if (fence_state->scope == kSyncScopeInternal) {
            // Mark fence in use
            SubmitFence(queue_state, fence_state, std::max(1u, submit_count));
            if (!submit_count) {
                // If no submissions, but just dropping a fence on the end of the queue,
                // record an empty submission with just the fence, so we can determine
                // its completion.
                CB_SUBMISSION submission;
                submission.fence = fence;
                queue_state->submissions.emplace_back(std::move(submission));
            }
        } else {
            // Retire work up until this fence early, we will not see the wait that corresponds to this signal
            early_retire_seq = queue_state->seq + queue_state->submissions.size();
        }
    }
    return early_retire_seq;
}

void ValidationStateTracker::RecordSubmitCommandBuffer(CB_SUBMISSION &submission, VkCommandBuffer command_buffer) {
    auto cb_node = Get<CMD_BUFFER_STATE>(command_buffer);
    if (cb_node) {
        submission.cbs.push_back(command_buffer);
        for (auto *secondary_cmd_buffer : cb_node->linkedCommandBuffers) {
            submission.cbs.push_back(secondary_cmd_buffer->commandBuffer());
            secondary_cmd_buffer->IncrementResources();
        }
        cb_node->IncrementResources();
        // increment use count for all bound objects including secondary cbs
        cb_node->BeginUse();

        VkQueryPool first_pool = VK_NULL_HANDLE;
        EventToStageMap local_event_to_stage_map;
        QueryMap local_query_to_state_map;
        for (auto &function : cb_node->queryUpdates) {
            function(nullptr, /*do_validate*/ false, first_pool, submission.perf_submit_pass, &local_query_to_state_map);
        }

        for (const auto &query_state_pair : local_query_to_state_map) {
            queryToStateMap[query_state_pair.first] = query_state_pair.second;
        }

        for (const auto &function : cb_node->eventUpdates) {
            function(nullptr, /*do_validate*/ false, &local_event_to_stage_map);
        }

        for (const auto &eventStagePair : local_event_to_stage_map) {
            eventMap[eventStagePair.first]->stageMask = eventStagePair.second;
        }
    }
}

void ValidationStateTracker::RecordSubmitWaitSemaphore(CB_SUBMISSION &submission, VkQueue queue, VkSemaphore semaphore,
                                                       uint64_t value, uint64_t next_seq) {
    auto semaphore_state = GetSemaphoreState(semaphore);
    if (semaphore_state) {
        if (semaphore_state->scope == kSyncScopeInternal) {
            SEMAPHORE_WAIT wait;
            wait.semaphore = semaphore;
            wait.type = semaphore_state->type;
            if (semaphore_state->type == VK_SEMAPHORE_TYPE_BINARY) {
                if (semaphore_state->signaler.first != VK_NULL_HANDLE) {
                    wait.queue = semaphore_state->signaler.first;
                    wait.seq = semaphore_state->signaler.second;
                    submission.waitSemaphores.emplace_back(std::move(wait));
                    semaphore_state->BeginUse();
                }
                semaphore_state->signaler.first = VK_NULL_HANDLE;
                semaphore_state->signaled = false;
            } else if (semaphore_state->payload < value) {
                wait.queue = queue;
                wait.seq = next_seq;
                wait.payload = value;
                submission.waitSemaphores.emplace_back(std::move(wait));
                semaphore_state->BeginUse();
            }
        } else {
            submission.externalSemaphores.push_back(semaphore);
            semaphore_state->BeginUse();
            if (semaphore_state->scope == kSyncScopeExternalTemporary) {
                semaphore_state->scope = kSyncScopeInternal;
            }
        }
    }
}

bool ValidationStateTracker::RecordSubmitSignalSemaphore(CB_SUBMISSION &submission, VkQueue queue, VkSemaphore semaphore,
                                                         uint64_t value, uint64_t next_seq) {
    bool retire_early = false;
    auto semaphore_state = GetSemaphoreState(semaphore);
    if (semaphore_state) {
        if (semaphore_state->scope == kSyncScopeInternal) {
            SEMAPHORE_SIGNAL signal;
            signal.semaphore = semaphore;
            signal.seq = next_seq;
            if (semaphore_state->type == VK_SEMAPHORE_TYPE_BINARY) {
                semaphore_state->signaler.first = queue;
                semaphore_state->signaler.second = next_seq;
                semaphore_state->signaled = true;
            } else {
                signal.payload = value;
            }
            semaphore_state->BeginUse();
            submission.signalSemaphores.emplace_back(std::move(signal));
        } else {
            // Retire work up until this submit early, we will not see the wait that corresponds to this signal
            retire_early = true;
        }
    }
    return retire_early;
}

void ValidationStateTracker::PostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits,
                                                       VkFence fence, VkResult result) {
    if (result != VK_SUCCESS) return;
    auto queue_state = GetQueueState(queue);

    uint64_t early_retire_seq = RecordSubmitFence(queue_state, fence, submitCount);

    // Now process each individual submit
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        CB_SUBMISSION submission;
        const VkSubmitInfo *submit = &pSubmits[submit_idx];
        const uint64_t next_seq = queue_state->seq + queue_state->submissions.size() + 1;
        auto *timeline_semaphore_submit = LvlFindInChain<VkTimelineSemaphoreSubmitInfo>(submit->pNext);
        for (uint32_t i = 0; i < submit->waitSemaphoreCount; ++i) {
            uint64_t value = 0;
            if (timeline_semaphore_submit && timeline_semaphore_submit->pWaitSemaphoreValues != nullptr &&
                (i < timeline_semaphore_submit->waitSemaphoreValueCount)) {
                value = timeline_semaphore_submit->pWaitSemaphoreValues[i];
            }
            RecordSubmitWaitSemaphore(submission, queue, submit->pWaitSemaphores[i], value, next_seq);
        }

        bool retire_early = false;
        for (uint32_t i = 0; i < submit->signalSemaphoreCount; ++i) {
            uint64_t value = 0;
            if (timeline_semaphore_submit && timeline_semaphore_submit->pSignalSemaphoreValues != nullptr &&
                (i < timeline_semaphore_submit->signalSemaphoreValueCount)) {
                value = timeline_semaphore_submit->pSignalSemaphoreValues[i];
            }
            retire_early |= RecordSubmitSignalSemaphore(submission, queue, submit->pSignalSemaphores[i], value, next_seq);
        }
        if (retire_early) {
            early_retire_seq = std::max(early_retire_seq, next_seq);
        }

        const auto perf_submit = LvlFindInChain<VkPerformanceQuerySubmitInfoKHR>(submit->pNext);
        submission.perf_submit_pass = perf_submit ? perf_submit->counterPassIndex : 0;

        for (uint32_t i = 0; i < submit->commandBufferCount; i++) {
            RecordSubmitCommandBuffer(submission, submit->pCommandBuffers[i]);
        }
        submission.fence = submit_idx == (submitCount - 1) ? fence : VK_NULL_HANDLE;
        queue_state->submissions.emplace_back(std::move(submission));
    }

    if (early_retire_seq) {
        RetireWorkOnQueue(queue_state, early_retire_seq);
    }
}

void ValidationStateTracker::PostCallRecordQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR *pSubmits,
                                                           VkFence fence, VkResult result) {
    if (result != VK_SUCCESS) return;
    auto queue_state = GetQueueState(queue);

    uint64_t early_retire_seq = RecordSubmitFence(queue_state, fence, submitCount);

    // Now process each individual submit
    for (uint32_t submit_idx = 0; submit_idx < submitCount; submit_idx++) {
        CB_SUBMISSION submission;
        const VkSubmitInfo2KHR *submit = &pSubmits[submit_idx];
        const uint64_t next_seq = queue_state->seq + queue_state->submissions.size() + 1;
        for (uint32_t i = 0; i < submit->waitSemaphoreInfoCount; ++i) {
            const auto &sem_info = submit->pWaitSemaphoreInfos[i];
            RecordSubmitWaitSemaphore(submission, queue, sem_info.semaphore, sem_info.value, next_seq);
        }
        bool retire_early = false;
        for (uint32_t i = 0; i < submit->signalSemaphoreInfoCount; ++i) {
            const auto &sem_info = submit->pSignalSemaphoreInfos[i];
            retire_early |= RecordSubmitSignalSemaphore(submission, queue, sem_info.semaphore, sem_info.value, next_seq);
        }
        if (retire_early) {
            early_retire_seq = std::max(early_retire_seq, next_seq);
        }
        const auto perf_submit = lvl_find_in_chain<VkPerformanceQuerySubmitInfoKHR>(submit->pNext);
        submission.perf_submit_pass = perf_submit ? perf_submit->counterPassIndex : 0;

        for (uint32_t i = 0; i < submit->commandBufferInfoCount; i++) {
            RecordSubmitCommandBuffer(submission, submit->pCommandBufferInfos[i].commandBuffer);
        }
        submission.fence = submit_idx == (submitCount - 1) ? fence : VK_NULL_HANDLE;
        queue_state->submissions.emplace_back(std::move(submission));
    }

    if (early_retire_seq) {
        RetireWorkOnQueue(queue_state, early_retire_seq);
    }
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

    layer_data::optional<DedicatedBinding> dedicated_binding;

    auto dedicated = LvlFindInChain<VkMemoryDedicatedAllocateInfo>(pAllocateInfo->pNext);
    if (dedicated) {
        if (dedicated->buffer) {
            const auto *buffer_state = GetBufferState(dedicated->buffer);
            assert(buffer_state);
            if (!buffer_state) {
                return;
            }
            dedicated_binding.emplace(dedicated->buffer, buffer_state->createInfo);
        } else if (dedicated->image) {
            const auto *image_state = GetImageState(dedicated->image);
            assert(image_state);
            if (!image_state) {
                return;
            }
            dedicated_binding.emplace(dedicated->image, image_state->createInfo);
        }
    }
    memObjMap[*pMemory] = std::make_shared<DEVICE_MEMORY_STATE>(*pMemory, pAllocateInfo, fake_address, memory_type, memory_heap,
                                                                std::move(dedicated_binding), physical_device_count);
    return;
}

void ValidationStateTracker::PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory mem, const VkAllocationCallbacks *pAllocator) {
    if (!mem) return;
    DEVICE_MEMORY_STATE *mem_info = GetDevMemState(mem);
    if (!mem_info) return;
    // Any bound cmd buffers are now invalid
    mem_info->Destroy();
    fake_memory.Free(mem_info->fake_base_address);
    memObjMap.erase(mem);
}

void ValidationStateTracker::PostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo *pBindInfo,
                                                           VkFence fence, VkResult result) {
    if (result != VK_SUCCESS) return;
    auto queue_state = GetQueueState(queue);

    uint64_t early_retire_seq = RecordSubmitFence(queue_state, fence, bindInfoCount);

    for (uint32_t bind_idx = 0; bind_idx < bindInfoCount; ++bind_idx) {
        const VkBindSparseInfo &bind_info = pBindInfo[bind_idx];
        // Track objects tied to memory
        for (uint32_t j = 0; j < bind_info.bufferBindCount; j++) {
            for (uint32_t k = 0; k < bind_info.pBufferBinds[j].bindCount; k++) {
                auto sparse_binding = bind_info.pBufferBinds[j].pBinds[k];
                auto buffer_state = GetBufferState(bind_info.pBufferBinds[j].buffer);
                auto mem_state = GetDevMemShared(sparse_binding.memory);
                if (buffer_state && mem_state) {
                    buffer_state->SetSparseMemBinding(mem_state, sparse_binding.memoryOffset, sparse_binding.size);
                }
            }
        }
        for (uint32_t j = 0; j < bind_info.imageOpaqueBindCount; j++) {
            for (uint32_t k = 0; k < bind_info.pImageOpaqueBinds[j].bindCount; k++) {
                auto sparse_binding = bind_info.pImageOpaqueBinds[j].pBinds[k];
                auto image_state = GetImageState(bind_info.pImageOpaqueBinds[j].image);
                auto mem_state = GetDevMemShared(sparse_binding.memory);
                if (image_state && mem_state) {
                    image_state->SetSparseMemBinding(mem_state, sparse_binding.memoryOffset, sparse_binding.size);
                }
            }
        }
        for (uint32_t j = 0; j < bind_info.imageBindCount; j++) {
            for (uint32_t k = 0; k < bind_info.pImageBinds[j].bindCount; k++) {
                auto sparse_binding = bind_info.pImageBinds[j].pBinds[k];
                // TODO: This size is broken for non-opaque bindings, need to update to comprehend full sparse binding data
                VkDeviceSize size = sparse_binding.extent.depth * sparse_binding.extent.height * sparse_binding.extent.width * 4;
                auto image_state = GetImageState(bind_info.pImageBinds[j].image);
                auto mem_state = GetDevMemShared(sparse_binding.memory);
                if (image_state && mem_state) {
                    image_state->SetSparseMemBinding(mem_state, sparse_binding.memoryOffset, size);
                }
            }
        }
        CB_SUBMISSION submission;
        const uint64_t next_seq = queue_state->seq + queue_state->submissions.size() + 1;
        for (uint32_t i = 0; i < bind_info.waitSemaphoreCount; ++i) {
            RecordSubmitWaitSemaphore(submission, queue, bind_info.pWaitSemaphores[i], 0, next_seq);
        }
        bool retire_early = false;
        for (uint32_t i = 0; i < bind_info.signalSemaphoreCount; ++i) {
            retire_early |= RecordSubmitSignalSemaphore(submission, queue, bind_info.pSignalSemaphores[i], 0, next_seq);
        }
        // Retire work up until this submit early, we will not see the wait that corresponds to this signal
        if (retire_early) {
            early_retire_seq = std::max(early_retire_seq, queue_state->seq + queue_state->submissions.size() + 1);
        }

        submission.fence = bind_idx == (bindInfoCount - 1) ? fence : VK_NULL_HANDLE;
        queue_state->submissions.emplace_back(std::move(submission));
    }

    if (early_retire_seq) {
        RetireWorkOnQueue(queue_state, early_retire_seq);
    }
}

void ValidationStateTracker::PostCallRecordCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
                                                           const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore,
                                                           VkResult result) {
    if (VK_SUCCESS != result) return;
    semaphoreMap[*pSemaphore] = std::make_shared<SEMAPHORE_STATE>(*pSemaphore, LvlFindInChain<VkSemaphoreTypeCreateInfo>(pCreateInfo->pNext));
}

void ValidationStateTracker::RecordImportSemaphoreState(VkSemaphore semaphore, VkExternalSemaphoreHandleTypeFlagBits handle_type,
                                                        VkSemaphoreImportFlags flags) {
    SEMAPHORE_STATE *sema_node = GetSemaphoreState(semaphore);
    if (sema_node && sema_node->scope != kSyncScopeExternalPermanent) {
        if ((handle_type == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT || flags & VK_SEMAPHORE_IMPORT_TEMPORARY_BIT) &&
            sema_node->scope == kSyncScopeInternal) {
            sema_node->scope = kSyncScopeExternalTemporary;
        } else {
            sema_node->scope = kSyncScopeExternalPermanent;
        }
    }
}

void ValidationStateTracker::PostCallRecordSignalSemaphoreKHR(VkDevice device, const VkSemaphoreSignalInfo *pSignalInfo,
                                                              VkResult result) {
    auto *semaphore_state = GetSemaphoreState(pSignalInfo->semaphore);
    semaphore_state->payload = pSignalInfo->value;
}

void ValidationStateTracker::RecordMappedMemory(VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, void **ppData) {
    auto mem_info = GetDevMemState(mem);
    if (mem_info) {
        mem_info->mapped_range.offset = offset;
        mem_info->mapped_range.size = size;
        mem_info->p_driver_data = *ppData;
    }
}

void ValidationStateTracker::RetireFence(VkFence fence) {
    auto fence_state = GetFenceState(fence);
    if (fence_state && fence_state->scope == kSyncScopeInternal) {
        if (fence_state->signaler.first != VK_NULL_HANDLE) {
            // Fence signaller is a queue -- use this as proof that prior operations on that queue have completed.
            RetireWorkOnQueue(GetQueueState(fence_state->signaler.first), fence_state->signaler.second);
        } else {
            // Fence signaller is the WSI. We're not tracking what the WSI op actually /was/ in CV yet, but we need to mark
            // the fence as retired.
            fence_state->state = FENCE_RETIRED;
        }
    }
}

void ValidationStateTracker::PostCallRecordWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences,
                                                         VkBool32 waitAll, uint64_t timeout, VkResult result) {
    if (VK_SUCCESS != result) return;

    // When we know that all fences are complete we can clean/remove their CBs
    if ((VK_TRUE == waitAll) || (1 == fenceCount)) {
        for (uint32_t i = 0; i < fenceCount; i++) {
            RetireFence(pFences[i]);
        }
    }
    // NOTE : Alternate case not handled here is when some fences have completed. In
    //  this case for app to guarantee which fences completed it will have to call
    //  vkGetFenceStatus() at which point we'll clean/remove their CBs if complete.
}

void ValidationStateTracker::RetireTimelineSemaphore(VkSemaphore semaphore, uint64_t until_payload) {
    auto semaphore_state = GetSemaphoreState(semaphore);
    if (semaphore_state) {
        for (const auto &pair : queueMap) {
            const auto &queue_state = pair.second;
            uint64_t max_seq = 0;
            for (const auto &submission : queue_state->submissions) {
                for (const auto &signal_semaphore : submission.signalSemaphores) {
                    if (signal_semaphore.semaphore == semaphore && signal_semaphore.payload <= until_payload) {
                        if (signal_semaphore.seq > max_seq) {
                            max_seq = signal_semaphore.seq;
                        }
                    }
                }
            }
            if (max_seq) {
                RetireWorkOnQueue(queue_state.get(), max_seq);
            }
        }
    }
}

void ValidationStateTracker::RecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout,
                                                  VkResult result) {
    if (VK_SUCCESS != result) return;

    for (uint32_t i = 0; i < pWaitInfo->semaphoreCount; i++) {
        RetireTimelineSemaphore(pWaitInfo->pSemaphores[i], pWaitInfo->pValues[i]);
    }
}

void ValidationStateTracker::PostCallRecordWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo, uint64_t timeout,
                                                          VkResult result) {
    RecordWaitSemaphores(device, pWaitInfo, timeout, result);
}

void ValidationStateTracker::PostCallRecordWaitSemaphoresKHR(VkDevice device, const VkSemaphoreWaitInfo *pWaitInfo,
                                                             uint64_t timeout, VkResult result) {
    RecordWaitSemaphores(device, pWaitInfo, timeout, result);
}

void ValidationStateTracker::RecordGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t *pValue,
                                                            VkResult result) {
    if (VK_SUCCESS != result) return;

    RetireTimelineSemaphore(semaphore, *pValue);
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
    RetireFence(fence);
}

void ValidationStateTracker::RecordGetDeviceQueueState(uint32_t queue_family_index, VkQueue queue) {
    queueMap.emplace(queue, std::make_shared<QUEUE_STATE>(queue, queue_family_index));
}

void ValidationStateTracker::PostCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex,
                                                          VkQueue *pQueue) {
    RecordGetDeviceQueueState(queueFamilyIndex, *pQueue);
}

void ValidationStateTracker::PostCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue) {
    RecordGetDeviceQueueState(pQueueInfo->queueFamilyIndex, *pQueue);
}

void ValidationStateTracker::PostCallRecordQueueWaitIdle(VkQueue queue, VkResult result) {
    if (VK_SUCCESS != result) return;
    QUEUE_STATE *queue_state = GetQueueState(queue);
    RetireWorkOnQueue(queue_state, queue_state->seq + queue_state->submissions.size());
}

void ValidationStateTracker::PostCallRecordDeviceWaitIdle(VkDevice device, VkResult result) {
    if (VK_SUCCESS != result) return;
    for (auto &queue : queueMap) {
        RetireWorkOnQueue(queue.second.get(), queue.second->seq + queue.second->submissions.size());
    }
}

void ValidationStateTracker::PreCallRecordDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks *pAllocator) {
    if (!fence) return;
    auto fence_state = GetFenceState(fence);
    fence_state->Destroy();
    fenceMap.erase(fence);
}

void ValidationStateTracker::PreCallRecordDestroySemaphore(VkDevice device, VkSemaphore semaphore,
                                                           const VkAllocationCallbacks *pAllocator) {
    if (!semaphore) return;
    auto semaphore_state = GetSemaphoreState(semaphore);
    semaphore_state->Destroy();
    semaphoreMap.erase(semaphore);
}

void ValidationStateTracker::PreCallRecordDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator) {
    if (!event) return;
    EVENT_STATE *event_state = Get<EVENT_STATE>(event);
    event_state->Destroy();
    eventMap.erase(event);
}

void ValidationStateTracker::PreCallRecordDestroyQueryPool(VkDevice device, VkQueryPool queryPool,
                                                           const VkAllocationCallbacks *pAllocator) {
    if (!queryPool) return;
    QUERY_POOL_STATE *qp_state = GetQueryPoolState(queryPool);
    qp_state->Destroy();
    queryPoolMap.erase(queryPool);
}

void ValidationStateTracker::UpdateBindBufferMemoryState(VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset) {
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    if (buffer_state) {
        // Track objects tied to memory
        auto mem_state = GetDevMemShared(mem);
        if (mem_state) {
            buffer_state->SetMemBinding(mem_state, memoryOffset);
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
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
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
    IMAGE_STATE *image_state = GetImageState(image);
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
    auto image_state = GetImageState(image);
    image_state->get_sparse_reqs_called = true;
}

void ValidationStateTracker::PostCallRecordGetImageSparseMemoryRequirements2(
    VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo, uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements) {
    auto image_state = GetImageState(pInfo->image);
    image_state->get_sparse_reqs_called = true;
}

void ValidationStateTracker::PostCallRecordGetImageSparseMemoryRequirements2KHR(
    VkDevice device, const VkImageSparseMemoryRequirementsInfo2 *pInfo, uint32_t *pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements) {
    auto image_state = GetImageState(pInfo->image);
    image_state->get_sparse_reqs_called = true;
}

void ValidationStateTracker::PreCallRecordDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                                                              const VkAllocationCallbacks *pAllocator) {
    if (!shaderModule) return;
    auto shader_module_state = GetShaderModuleState(shaderModule);
    shader_module_state->Destroy();
    shaderModuleMap.erase(shaderModule);
}

void ValidationStateTracker::PreCallRecordDestroyPipeline(VkDevice device, VkPipeline pipeline,
                                                          const VkAllocationCallbacks *pAllocator) {
    if (!pipeline) return;
    PIPELINE_STATE *pipeline_state = GetPipelineState(pipeline);
    // Any bound cmd buffers are now invalid
    pipeline_state->Destroy();
    pipelineMap.erase(pipeline);
}

void ValidationStateTracker::PreCallRecordDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                                                                const VkAllocationCallbacks *pAllocator) {
    if (!pipelineLayout) return;
    auto pipeline_layout_state = GetPipelineLayout(pipelineLayout);
    pipeline_layout_state->Destroy();
    pipelineLayoutMap.erase(pipelineLayout);
}

void ValidationStateTracker::PreCallRecordDestroySampler(VkDevice device, VkSampler sampler,
                                                         const VkAllocationCallbacks *pAllocator) {
    if (!sampler) return;
    SAMPLER_STATE *sampler_state = GetSamplerState(sampler);
    // Any bound cmd buffers are now invalid
    if (sampler_state) {
        if (sampler_state->createInfo.borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT ||
            sampler_state->createInfo.borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT) {
            custom_border_color_sampler_count--;
        }

        sampler_state->Destroy();
    }
    samplerMap.erase(sampler);
}

void ValidationStateTracker::PreCallRecordDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                                                     const VkAllocationCallbacks *pAllocator) {
    if (!descriptorSetLayout) return;
    auto layout_it = descriptorSetLayoutMap.find(descriptorSetLayout);
    if (layout_it != descriptorSetLayoutMap.end()) {
        layout_it->second.get()->Destroy();
        descriptorSetLayoutMap.erase(layout_it);
    }
}

void ValidationStateTracker::PreCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                                const VkAllocationCallbacks *pAllocator) {
    if (!descriptorPool) return;
    DESCRIPTOR_POOL_STATE *desc_pool_state = GetDescriptorPoolState(descriptorPool);
    if (desc_pool_state) {
        // Free sets that were in this pool
        for (auto *ds : desc_pool_state->sets) {
            FreeDescriptorSet(ds);
        }
        desc_pool_state->Destroy();
        descriptorPoolMap.erase(descriptorPool);
    }
}

// Free all command buffers in given list, removing all references/links to them using CMD_BUFFER_STATE::Reset
void ValidationStateTracker::FreeCommandBufferStates(COMMAND_POOL_STATE *pool_state, const uint32_t command_buffer_count,
                                                     const VkCommandBuffer *command_buffers) {
    for (uint32_t i = 0; i < command_buffer_count; i++) {
        auto cb_state = Get<CMD_BUFFER_STATE>(command_buffers[i]);
        // Remove references to command buffer's state and delete
        if (cb_state) {
            cb_state->Destroy();
        }
        // Remove CBState from CB map
        pool_state->commandBuffers.erase(command_buffers[i]);
        commandBufferMap.erase(command_buffers[i]);
    }
}

void ValidationStateTracker::PreCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool,
                                                             uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers) {
    auto pool = GetCommandPoolState(commandPool);
    FreeCommandBufferStates(pool, commandBufferCount, pCommandBuffers);
}

void ValidationStateTracker::PostCallRecordCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkCommandPool *pCommandPool,
                                                             VkResult result) {
    if (VK_SUCCESS != result) return;
    auto queue_flags = physical_device_state->queue_family_properties[pCreateInfo->queueFamilyIndex].queueFlags;
    commandPoolMap[*pCommandPool] = std::make_shared<COMMAND_POOL_STATE>(*pCommandPool, pCreateInfo, queue_flags);
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

    queryPoolMap[*pQueryPool] =
        std::make_shared<QUERY_POOL_STATE>(*pQueryPool, pCreateInfo, index_count, n_perf_pass, has_cb, has_rb);

    QueryObject query_obj{*pQueryPool, 0u};
    for (uint32_t i = 0; i < pCreateInfo->queryCount; ++i) {
        query_obj.query = i;
        queryToStateMap[query_obj] = QUERYSTATE_UNKNOWN;
    }
}

void ValidationStateTracker::PreCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                                             const VkAllocationCallbacks *pAllocator) {
    if (!commandPool) return;
    COMMAND_POOL_STATE *cp_state = GetCommandPoolState(commandPool);
    // Remove cmdpool from cmdpoolmap, after freeing layer data for the command buffers
    // "When a pool is destroyed, all command buffers allocated from the pool are freed."
    if (cp_state) {
        // Create a vector, as FreeCommandBufferStates deletes from cp_state->commandBuffers during iteration.
        std::vector<VkCommandBuffer> cb_vec{cp_state->commandBuffers.begin(), cp_state->commandBuffers.end()};
        FreeCommandBufferStates(cp_state, static_cast<uint32_t>(cb_vec.size()), cb_vec.data());
        cp_state->Destroy();
        commandPoolMap.erase(commandPool);
    }
}

void ValidationStateTracker::PostCallRecordResetCommandPool(VkDevice device, VkCommandPool commandPool,
                                                            VkCommandPoolResetFlags flags, VkResult result) {
    if (VK_SUCCESS != result) return;
    // Reset all of the CBs allocated from this pool
    auto command_pool_state = GetCommandPoolState(commandPool);
    for (auto cmd_buffer : command_pool_state->commandBuffers) {
        auto cb_state = Get<CMD_BUFFER_STATE>(cmd_buffer);
        cb_state->Reset();
    }
}

void ValidationStateTracker::PostCallRecordResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences,
                                                       VkResult result) {
    for (uint32_t i = 0; i < fenceCount; ++i) {
        auto fence_state = GetFenceState(pFences[i]);
        if (fence_state) {
            if (fence_state->scope == kSyncScopeInternal) {
                fence_state->state = FENCE_UNSIGNALED;
            } else if (fence_state->scope == kSyncScopeExternalTemporary) {
                fence_state->scope = kSyncScopeInternal;
            }
        }
    }
}

void ValidationStateTracker::PreCallRecordDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                                                             const VkAllocationCallbacks *pAllocator) {
    if (!framebuffer) return;
    FRAMEBUFFER_STATE *framebuffer_state = GetFramebufferState(framebuffer);
    framebuffer_state->Destroy();
    frameBufferMap.erase(framebuffer);
}

void ValidationStateTracker::PreCallRecordDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                                                            const VkAllocationCallbacks *pAllocator) {
    if (!renderPass) return;
    RENDER_PASS_STATE *rp_state = GetRenderPassState(renderPass);
    rp_state->Destroy();
    renderPassMap.erase(renderPass);
}

void ValidationStateTracker::PostCallRecordCreateFence(VkDevice device, const VkFenceCreateInfo *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkFence *pFence, VkResult result) {
    if (VK_SUCCESS != result) return;
    fenceMap[*pFence] = std::make_shared<FENCE_STATE>(*pFence, pCreateInfo);
}

bool ValidationStateTracker::PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t count,
                                                                    const VkGraphicsPipelineCreateInfo *pCreateInfos,
                                                                    const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines,
                                                                    void *cgpl_state_data) const {
    // Set up the state that CoreChecks, gpu_validation and later StateTracker Record will use.
    create_graphics_pipeline_api_state *cgpl_state = reinterpret_cast<create_graphics_pipeline_api_state *>(cgpl_state_data);
    cgpl_state->pCreateInfos = pCreateInfos;  // GPU validation can alter this, so we have to set a default value for the Chassis
    cgpl_state->pipe_state.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        cgpl_state->pipe_state.push_back(std::make_shared<PIPELINE_STATE>(this, &pCreateInfos[i],
                                                                          GetRenderPassShared(pCreateInfos[i].renderPass),
                                                                          GetPipelineLayoutShared(pCreateInfos[i].layout)));
    }
    return false;
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
            pipelineMap[pPipelines[i]] = std::move((cgpl_state->pipe_state)[i]);
        }
    }
    cgpl_state->pipe_state.clear();
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
            std::make_shared<PIPELINE_STATE>(this, &pCreateInfos[i], GetPipelineLayoutShared(pCreateInfos[i].layout)));
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
            pipelineMap[pPipelines[i]] = std::move((ccpl_state->pipe_state)[i]);
        }
    }
    ccpl_state->pipe_state.clear();
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
            std::make_shared<PIPELINE_STATE>(this, &pCreateInfos[i], GetPipelineLayoutShared(pCreateInfos[i].layout)));
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
            pipelineMap[pPipelines[i]] = std::move((crtpl_state->pipe_state)[i]);
        }
    }
    crtpl_state->pipe_state.clear();
}

bool ValidationStateTracker::PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                         VkPipelineCache pipelineCache, uint32_t count,
                                                                         const VkRayTracingPipelineCreateInfoKHR *pCreateInfos,
                                                                         const VkAllocationCallbacks *pAllocator,
                                                                         VkPipeline *pPipelines, void *crtpl_state_data) const {
    auto *crtpl_state = reinterpret_cast<create_ray_tracing_pipeline_khr_api_state *>(crtpl_state_data);
    crtpl_state->pipe_state.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        // Create and initialize internal tracking data structure
        crtpl_state->pipe_state.push_back(
            std::make_shared<PIPELINE_STATE>(this, &pCreateInfos[i], GetPipelineLayoutShared(pCreateInfos[i].layout)));
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
    // This API may create pipelines regardless of the return value
    for (uint32_t i = 0; i < count; i++) {
        if (pPipelines[i] != VK_NULL_HANDLE) {
            (crtpl_state->pipe_state)[i]->SetHandle(pPipelines[i]);
            pipelineMap[pPipelines[i]] = std::move((crtpl_state->pipe_state)[i]);
        }
    }
    crtpl_state->pipe_state.clear();
}

void ValidationStateTracker::PostCallRecordCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator, VkSampler *pSampler,
                                                         VkResult result) {
    samplerMap[*pSampler] = std::make_shared<SAMPLER_STATE>(pSampler, pCreateInfo);
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
    descriptorSetLayoutMap[*pSetLayout] = std::make_shared<cvdescriptorset::DescriptorSetLayout>(pCreateInfo, *pSetLayout);
}

void ValidationStateTracker::PostCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                VkPipelineLayout *pPipelineLayout, VkResult result) {
    if (VK_SUCCESS != result) return;

    pipelineLayoutMap[*pPipelineLayout] = std::make_shared<PIPELINE_LAYOUT_STATE>(this, *pPipelineLayout, pCreateInfo);
}

void ValidationStateTracker::PostCallRecordCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                VkDescriptorPool *pDescriptorPool, VkResult result) {
    if (VK_SUCCESS != result) return;
    descriptorPoolMap[*pDescriptorPool] = std::make_shared<DESCRIPTOR_POOL_STATE>(*pDescriptorPool, pCreateInfo);
}

void ValidationStateTracker::PostCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                               VkDescriptorPoolResetFlags flags, VkResult result) {
    if (VK_SUCCESS != result) return;
    DESCRIPTOR_POOL_STATE *pool = GetDescriptorPoolState(descriptorPool);
    // TODO: validate flags
    // For every set off of this pool, clear it, remove from setMap, and free cvdescriptorset::DescriptorSet
    for (auto *ds : pool->sets) {
        FreeDescriptorSet(ds);
    }
    pool->sets.clear();
    // Reset available count for each type and available sets for this pool
    for (auto it = pool->availableDescriptorTypeCount.begin(); it != pool->availableDescriptorTypeCount.end(); ++it) {
        pool->availableDescriptorTypeCount[it->first] = pool->maxDescriptorTypeCount[it->first];
    }
    pool->availableSets = pool->maxSets;
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
    PerformAllocateDescriptorSets(pAllocateInfo, pDescriptorSets, ads_state);
}

void ValidationStateTracker::PreCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count,
                                                             const VkDescriptorSet *pDescriptorSets) {
    DESCRIPTOR_POOL_STATE *pool_state = GetDescriptorPoolState(descriptorPool);
    // Update available descriptor sets in pool
    pool_state->availableSets += count;

    // For each freed descriptor add its resources back into the pool as available and remove from pool and setMap
    for (uint32_t i = 0; i < count; ++i) {
        if (pDescriptorSets[i] != VK_NULL_HANDLE) {
            auto descriptor_set = setMap[pDescriptorSets[i]].get();
            uint32_t type_index = 0, descriptor_count = 0;
            for (uint32_t j = 0; j < descriptor_set->GetBindingCount(); ++j) {
                type_index = static_cast<uint32_t>(descriptor_set->GetTypeFromIndex(j));
                descriptor_count = descriptor_set->GetDescriptorCountFromIndex(j);
                pool_state->availableDescriptorTypeCount[type_index] += descriptor_count;
            }
            FreeDescriptorSet(descriptor_set);
            pool_state->sets.erase(descriptor_set);
        }
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
                                                                  VkCommandBuffer *pCommandBuffer, VkResult result) {
    if (VK_SUCCESS != result) return;
    auto pool = GetCommandPoolShared(pCreateInfo->commandPool);
    if (pool) {
        for (uint32_t i = 0; i < pCreateInfo->commandBufferCount; i++) {
            // Add command buffer to its commandPool map
            pool->commandBuffers.insert(pCommandBuffer[i]);
            commandBufferMap[pCommandBuffer[i]] = CreateCmdBufferState(pCommandBuffer[i], pCreateInfo, pool);
        }
    }
}

void ValidationStateTracker::PreCallRecordBeginCommandBuffer(VkCommandBuffer commandBuffer,
                                                             const VkCommandBufferBeginInfo *pBeginInfo) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state) return;

    cb_state->Begin(pBeginInfo);
}

void ValidationStateTracker::PostCallRecordEndCommandBuffer(VkCommandBuffer commandBuffer, VkResult result) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state) return;

    cb_state->End(result);
}

void ValidationStateTracker::PostCallRecordResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags,
                                                              VkResult result) {
    if (VK_SUCCESS == result) {
        CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
        cb_state->Reset();
    }
}

CBStatusFlags MakeStaticStateMask(VkPipelineDynamicStateCreateInfo const *ds) {
    // initially assume everything is static state
    CBStatusFlags flags = CBSTATUS_ALL_STATE_SET;

    if (ds) {
        for (uint32_t i = 0; i < ds->dynamicStateCount; i++) {
            flags &= ~ConvertToCBStatusFlagBits(ds->pDynamicStates[i]);
        }
    }
    return flags;
}

// Validation cache:
// CV is the bottommost implementor of this extension. Don't pass calls down.

void ValidationStateTracker::PreCallRecordCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                          VkPipeline pipeline) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    assert(cb_state);
    cb_state->RecordCmd(CMD_BINDPIPELINE);

    auto pipe_state = GetPipelineState(pipeline);
    if (VK_PIPELINE_BIND_POINT_GRAPHICS == pipelineBindPoint) {
        const auto &create_info = pipe_state->create_info.graphics;
        bool rasterization_enabled = VK_FALSE == create_info.pRasterizationState->rasterizerDiscardEnable;
        const auto *viewport_state = create_info.pViewportState;
        const auto *dynamic_state = create_info.pDynamicState;
        cb_state->status &= ~cb_state->static_status;
        cb_state->static_status = MakeStaticStateMask(dynamic_state->ptr());
        cb_state->status |= cb_state->static_status;
        cb_state->dynamic_status = CBSTATUS_ALL_STATE_SET & (~cb_state->static_status);

        // Used to calculate CMD_BUFFER_STATE::usedViewportScissorCount upon draw command with this graphics pipeline.
        // If rasterization disabled (no viewport/scissors used), or the actual number of viewports/scissors is dynamic (unknown at
        // this time), then these are set to 0 to disable this checking.
        auto has_dynamic_viewport_count = cb_state->dynamic_status & CBSTATUS_VIEWPORT_WITH_COUNT_SET;
        auto has_dynamic_scissor_count = cb_state->dynamic_status & CBSTATUS_SCISSOR_WITH_COUNT_SET;
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
            if (rasterization_enabled && (cb_state->static_status & CBSTATUS_VIEWPORT_SET)) {
                cb_state->trashedViewportMask |= (uint32_t(1) << viewport_state->viewportCount) - 1u;
                // should become = ~uint32_t(0) if the other interpretation is correct.
            }
        }
        if (!has_dynamic_scissor_count) {
            cb_state->trashedScissorCount = true;
            if (rasterization_enabled && (cb_state->static_status & CBSTATUS_SCISSOR_SET)) {
                cb_state->trashedScissorMask |= (uint32_t(1) << viewport_state->scissorCount) - 1u;
                // should become = ~uint32_t(0) if the other interpretation is correct.
            }
        }
    }
    const auto lv_bind_point = ConvertToLvlBindPoint(pipelineBindPoint);
    cb_state->lastBound[lv_bind_point].pipeline_state = pipe_state;
    if (!disabled[command_buffer_state]) {
        cb_state->AddChild(pipe_state);
    }
    for (auto &slot : pipe_state->active_slots) {
        for (auto &req : slot.second) {
            for (auto &sampler : req.second.samplers_used_by_image) {
                for (auto &des : sampler) {
                    des.second = nullptr;
                }
            }
        }
    }
    cb_state->lastBound[lv_bind_point].UpdateSamplerDescriptorsUsedByImage();
}

void ValidationStateTracker::PreCallRecordCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                         uint32_t viewportCount, const VkViewport *pViewports) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETVIEWPORT, CBSTATUS_VIEWPORT_SET);
    uint32_t bits = ((1u << viewportCount) - 1u) << firstViewport;
    cb_state->viewportMask |= bits;
    cb_state->trashedViewportMask &= ~bits;

    cb_state->dynamicViewports.resize(std::max(size_t(firstViewport + viewportCount), cb_state->dynamicViewports.size()));
    for (size_t i = 0; i < viewportCount; ++i) {
        cb_state->dynamicViewports[firstViewport + i] = pViewports[i];
    }
}

void ValidationStateTracker::PreCallRecordCmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor,
                                                                   uint32_t exclusiveScissorCount,
                                                                   const VkRect2D *pExclusiveScissors) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETEXCLUSIVESCISSORNV, CBSTATUS_EXCLUSIVE_SCISSOR_SET);
    // TODO: We don't have VUIDs for validating that all exclusive scissors have been set.
    // cb_state->exclusiveScissorMask |= ((1u << exclusiveScissorCount) - 1u) << firstExclusiveScissor;
}

void ValidationStateTracker::PreCallRecordCmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView,
                                                                    VkImageLayout imageLayout) {
    if (disabled[command_buffer_state]) return;

    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_BINDSHADINGRATEIMAGENV);

    if (imageView != VK_NULL_HANDLE) {
        auto view_state = GetImageViewState(imageView);
        cb_state->AddChild(view_state);
    }
}

void ValidationStateTracker::PreCallRecordCmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                             uint32_t viewportCount,
                                                                             const VkShadingRatePaletteNV *pShadingRatePalettes) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETVIEWPORTSHADINGRATEPALETTENV, CBSTATUS_SHADING_RATE_PALETTE_SET);
    // TODO: We don't have VUIDs for validating that all shading rate palettes have been set.
    // cb_state->shadingRatePaletteMask |= ((1u << viewportCount) - 1u) << firstViewport;
}

void ValidationStateTracker::PostCallRecordCreateAccelerationStructureNV(VkDevice device,
                                                                         const VkAccelerationStructureCreateInfoNV *pCreateInfo,
                                                                         const VkAllocationCallbacks *pAllocator,
                                                                         VkAccelerationStructureNV *pAccelerationStructure,
                                                                         VkResult result) {
    if (VK_SUCCESS != result) return;
    auto as_state = std::make_shared<ACCELERATION_STRUCTURE_STATE>(*pAccelerationStructure, pCreateInfo);

    // Query the requirements in case the application doesn't (to avoid bind/validation time query)
    auto as_memory_requirements_info = LvlInitStruct<VkAccelerationStructureMemoryRequirementsInfoNV>();
    as_memory_requirements_info.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
    as_memory_requirements_info.accelerationStructure = as_state->acceleration_structure();
    DispatchGetAccelerationStructureMemoryRequirementsNV(device, &as_memory_requirements_info, &as_state->memory_requirements);

    auto scratch_memory_req_info = LvlInitStruct<VkAccelerationStructureMemoryRequirementsInfoNV>();
    scratch_memory_req_info.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
    scratch_memory_req_info.accelerationStructure = as_state->acceleration_structure();
    DispatchGetAccelerationStructureMemoryRequirementsNV(device, &scratch_memory_req_info,
                                                         &as_state->build_scratch_memory_requirements);

    auto update_memory_req_info = LvlInitStruct<VkAccelerationStructureMemoryRequirementsInfoNV>();
    update_memory_req_info.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV;
    update_memory_req_info.accelerationStructure = as_state->acceleration_structure();
    DispatchGetAccelerationStructureMemoryRequirementsNV(device, &update_memory_req_info,
                                                         &as_state->update_scratch_memory_requirements);
    as_state->allocator = pAllocator;
    accelerationStructureMap[*pAccelerationStructure] = std::move(as_state);
}

void ValidationStateTracker::PostCallRecordCreateAccelerationStructureKHR(VkDevice device,
                                                                          const VkAccelerationStructureCreateInfoKHR *pCreateInfo,
                                                                          const VkAllocationCallbacks *pAllocator,
                                                                          VkAccelerationStructureKHR *pAccelerationStructure,
                                                                          VkResult result) {
    if (VK_SUCCESS != result) return;
    auto as_state = std::make_shared<ACCELERATION_STRUCTURE_STATE_KHR>(*pAccelerationStructure, pCreateInfo);
    as_state->allocator = pAllocator;
    accelerationStructureMap_khr[*pAccelerationStructure] = std::move(as_state);
}

void ValidationStateTracker::PostCallRecordCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_state == nullptr) {
        return;
    }
    cb_state->RecordCmd(CMD_BUILDACCELERATIONSTRUCTURESKHR);
    for (uint32_t i = 0; i < infoCount; ++i) {
        auto *dst_as_state = GetAccelerationStructureStateKHR(pInfos[i].dstAccelerationStructure);
        if (dst_as_state != nullptr) {
            dst_as_state->built = true;
            dst_as_state->build_info_khr.initialize(&pInfos[i]);
            if (!disabled[command_buffer_state]) {
                cb_state->AddChild(dst_as_state);
            }
        }
        if (!disabled[command_buffer_state]) {
            auto *src_as_state = GetAccelerationStructureStateKHR(pInfos[i].srcAccelerationStructure);
            if (src_as_state != nullptr) {
                cb_state->AddChild(src_as_state);
            }
        }
    }
    cb_state->hasBuildAccelerationStructureCmd = true;
}

void ValidationStateTracker::PostCallRecordCmdBuildAccelerationStructuresIndirectKHR(
    VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
    const VkDeviceAddress *pIndirectDeviceAddresses, const uint32_t *pIndirectStrides,
    const uint32_t *const *ppMaxPrimitiveCounts) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_state == nullptr) {
        return;
    }
    cb_state->RecordCmd(CMD_BUILDACCELERATIONSTRUCTURESINDIRECTKHR);
    for (uint32_t i = 0; i < infoCount; ++i) {
        auto *dst_as_state = GetAccelerationStructureStateKHR(pInfos[i].dstAccelerationStructure);
        if (dst_as_state != nullptr) {
            dst_as_state->built = true;
            dst_as_state->build_info_khr.initialize(&pInfos[i]);
            if (!disabled[command_buffer_state]) {
                cb_state->AddChild(dst_as_state);
            }
        }
        if (!disabled[command_buffer_state]) {
            auto *src_as_state = GetAccelerationStructureStateKHR(pInfos[i].srcAccelerationStructure);
            if (src_as_state != nullptr) {
                cb_state->AddChild(src_as_state);
            }
        }
    }
    cb_state->hasBuildAccelerationStructureCmd = true;
}
void ValidationStateTracker::PostCallRecordGetAccelerationStructureMemoryRequirementsNV(
    VkDevice device, const VkAccelerationStructureMemoryRequirementsInfoNV *pInfo, VkMemoryRequirements2 *pMemoryRequirements) {
    ACCELERATION_STRUCTURE_STATE *as_state = GetAccelerationStructureStateNV(pInfo->accelerationStructure);
    if (as_state != nullptr) {
        if (pInfo->type == VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV) {
            as_state->memory_requirements = *pMemoryRequirements;
            as_state->memory_requirements_checked = true;
        } else if (pInfo->type == VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV) {
            as_state->build_scratch_memory_requirements = *pMemoryRequirements;
            as_state->build_scratch_memory_requirements_checked = true;
        } else if (pInfo->type == VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV) {
            as_state->update_scratch_memory_requirements = *pMemoryRequirements;
            as_state->update_scratch_memory_requirements_checked = true;
        }
    }
}

void ValidationStateTracker::PostCallRecordBindAccelerationStructureMemoryNV(
    VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoNV *pBindInfos, VkResult result) {
    if (VK_SUCCESS != result) return;
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        const VkBindAccelerationStructureMemoryInfoNV &info = pBindInfos[i];

        ACCELERATION_STRUCTURE_STATE *as_state = GetAccelerationStructureStateNV(info.accelerationStructure);
        if (as_state) {
            // Track objects tied to memory
            auto mem_state = GetDevMemShared(info.memory);
            if (mem_state) {
                as_state->SetMemBinding(mem_state, info.memoryOffset);
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
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_state == nullptr) {
        return;
    }
    cb_state->RecordCmd(CMD_BUILDACCELERATIONSTRUCTURENV);

    auto *dst_as_state = GetAccelerationStructureStateNV(dst);
    if (dst_as_state != nullptr) {
        dst_as_state->built = true;
        dst_as_state->build_info.initialize(pInfo);
        if (!disabled[command_buffer_state]) {
           cb_state->AddChild(dst_as_state);
        }
    }
    if (!disabled[command_buffer_state]) {
        auto *src_as_state = GetAccelerationStructureStateNV(src);
        if (src_as_state != nullptr) {
            cb_state->AddChild(src_as_state);
        }
    }
    cb_state->hasBuildAccelerationStructureCmd = true;
}

void ValidationStateTracker::PostCallRecordCmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer,
                                                                          VkAccelerationStructureNV dst,
                                                                          VkAccelerationStructureNV src,
                                                                          VkCopyAccelerationStructureModeNV mode) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_state) {
        ACCELERATION_STRUCTURE_STATE *src_as_state = GetAccelerationStructureStateNV(src);
        ACCELERATION_STRUCTURE_STATE *dst_as_state = GetAccelerationStructureStateNV(dst);
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
    if (!accelerationStructure) return;
    auto *as_state = GetAccelerationStructureStateKHR(accelerationStructure);
    if (as_state) {
        as_state->Destroy();
        accelerationStructureMap_khr.erase(accelerationStructure);
    }
}

void ValidationStateTracker::PreCallRecordDestroyAccelerationStructureNV(VkDevice device,
                                                                         VkAccelerationStructureNV accelerationStructure,
                                                                         const VkAllocationCallbacks *pAllocator) {
    if (!accelerationStructure) return;
    auto *as_state = GetAccelerationStructureStateNV(accelerationStructure);
    if (as_state) {
        as_state->Destroy();
        accelerationStructureMap.erase(accelerationStructure);
    }
}

void ValidationStateTracker::PreCallRecordCmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport,
                                                                   uint32_t viewportCount,
                                                                   const VkViewportWScalingNV *pViewportWScalings) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETVIEWPORTWSCALINGNV, CBSTATUS_VIEWPORT_W_SCALING_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETLINEWIDTH, CBSTATUS_LINE_WIDTH_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor,
                                                               uint16_t lineStipplePattern) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETLINESTIPPLEEXT, CBSTATUS_LINE_STIPPLE_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor,
                                                          float depthBiasClamp, float depthBiasSlopeFactor) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETDEPTHBIAS, CBSTATUS_DEPTH_BIAS_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
                                                        const VkRect2D *pScissors) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETSCISSOR, CBSTATUS_SCISSOR_SET);
    uint32_t bits = ((1u << scissorCount) - 1u) << firstScissor;
    cb_state->scissorMask |= bits;
    cb_state->trashedScissorMask &= ~bits;
}

void ValidationStateTracker::PreCallRecordCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETBLENDCONSTANTS, CBSTATUS_BLEND_CONSTANTS_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds,
                                                            float maxDepthBounds) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETDEPTHBOUNDS, CBSTATUS_DEPTH_BOUNDS_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                                   uint32_t compareMask) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETSTENCILCOMPAREMASK, CBSTATUS_STENCIL_READ_MASK_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                                 uint32_t writeMask) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETSTENCILWRITEMASK, CBSTATUS_STENCIL_WRITE_MASK_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                                 uint32_t reference) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETSTENCILREFERENCE, CBSTATUS_STENCIL_REFERENCE_SET);
}


// Update the bound state for the bind point, including the effects of incompatible pipeline layouts
void ValidationStateTracker::PreCallRecordCmdBindDescriptorSets(VkCommandBuffer commandBuffer,
                                                                VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                uint32_t firstSet, uint32_t setCount,
                                                                const VkDescriptorSet *pDescriptorSets, uint32_t dynamicOffsetCount,
                                                                const uint32_t *pDynamicOffsets) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_BINDDESCRIPTORSETS);
    auto pipeline_layout = GetPipelineLayout(layout);

    // Resize binding arrays
    uint32_t last_set_index = firstSet + setCount - 1;
    const auto lv_bind_point = ConvertToLvlBindPoint(pipelineBindPoint);
    if (last_set_index >= cb_state->lastBound[lv_bind_point].per_set.size()) {
        cb_state->lastBound[lv_bind_point].per_set.resize(last_set_index + 1);
    }

    cb_state->UpdateLastBoundDescriptorSets(pipelineBindPoint, pipeline_layout, firstSet, setCount, pDescriptorSets, nullptr,
                                            dynamicOffsetCount, pDynamicOffsets);
    cb_state->lastBound[lv_bind_point].pipeline_layout = layout;
    cb_state->lastBound[lv_bind_point].UpdateSamplerDescriptorsUsedByImage();
}

void ValidationStateTracker::PreCallRecordCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer,
                                                                  VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout,
                                                                  uint32_t set, uint32_t descriptorWriteCount,
                                                                  const VkWriteDescriptorSet *pDescriptorWrites) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    auto pipeline_layout = GetPipelineLayout(layout);
    cb_state->PushDescriptorSetState(pipelineBindPoint, pipeline_layout, set, descriptorWriteCount, pDescriptorWrites);
}

void ValidationStateTracker::PostCallRecordCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout,
                                                            VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size,
                                                            const void *pValues) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_state != nullptr) {
        cb_state->RecordCmd(CMD_PUSHCONSTANTS);
        cb_state->ResetPushConstantDataIfIncompatible(GetPipelineLayout(layout));

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
    auto cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->RecordStateCmd(CMD_BINDINDEXBUFFER, CBSTATUS_INDEX_BUFFER_BOUND);
    cb_state->index_buffer_binding.buffer_state = GetShared<BUFFER_STATE>(buffer);
    cb_state->index_buffer_binding.size = cb_state->index_buffer_binding.buffer_state->createInfo.size;
    cb_state->index_buffer_binding.offset = offset;
    cb_state->index_buffer_binding.index_type = indexType;
    // Add binding for this index buffer to this commandbuffer
    if (!disabled[command_buffer_state]) {
        cb_state->AddChild(cb_state->index_buffer_binding.buffer_state.get());
    }
}

void ValidationStateTracker::PreCallRecordCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                               uint32_t bindingCount, const VkBuffer *pBuffers,
                                                               const VkDeviceSize *pOffsets) {
    auto cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_BINDVERTEXBUFFERS);

    uint32_t end = firstBinding + bindingCount;
    if (cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings.size() < end) {
        cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings.resize(end);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        auto &vertex_buffer_binding = cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings[i + firstBinding];
        vertex_buffer_binding.buffer_state = GetShared<BUFFER_STATE>(pBuffers[i]);
        vertex_buffer_binding.offset = pOffsets[i];
        vertex_buffer_binding.size = VK_WHOLE_SIZE;
        vertex_buffer_binding.stride = 0;
        // Add binding for this vertex buffer to this commandbuffer
        if (pBuffers[i] && !disabled[command_buffer_state]) {
            cb_state->AddChild(vertex_buffer_binding.buffer_state.get());
        }
    }
}

void ValidationStateTracker::PostCallRecordCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                                                           VkDeviceSize dstOffset, VkDeviceSize dataSize, const void *pData) {
    if (disabled[command_buffer_state]) return;

    auto cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordTransferCmd(CMD_UPDATEBUFFER, GetBufferState(dstBuffer));
}

void ValidationStateTracker::PreCallRecordCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                                      VkPipelineStageFlags stageMask) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordSetEvent(CMD_SETEVENT, event, stageMask);
}

void ValidationStateTracker::PreCallRecordCmdSetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                          const VkDependencyInfoKHR *pDependencyInfo) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    auto stage_masks = sync_utils::GetGlobalStageMasks(*pDependencyInfo);

    cb_state->RecordSetEvent(CMD_SETEVENT2KHR, event, stage_masks.src);
    cb_state->RecordBarriers(*pDependencyInfo);
}

void ValidationStateTracker::PreCallRecordCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                                        VkPipelineStageFlags stageMask) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordResetEvent(CMD_RESETEVENT, event, stageMask);
}

void ValidationStateTracker::PreCallRecordCmdResetEvent2KHR(VkCommandBuffer commandBuffer, VkEvent event,
                                                            VkPipelineStageFlags2KHR stageMask) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordResetEvent(CMD_RESETEVENT2KHR, event, stageMask);
}

void ValidationStateTracker::PreCallRecordCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
                                                        VkPipelineStageFlags sourceStageMask, VkPipelineStageFlags dstStageMask,
                                                        uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                        uint32_t bufferMemoryBarrierCount,
                                                        const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                        uint32_t imageMemoryBarrierCount,
                                                        const VkImageMemoryBarrier *pImageMemoryBarriers) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordWaitEvents(CMD_WAITEVENTS, eventCount, pEvents);
    cb_state->RecordBarriers(memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                             imageMemoryBarrierCount, pImageMemoryBarriers);
}

void ValidationStateTracker::PreCallRecordCmdWaitEvents2KHR(VkCommandBuffer commandBuffer, uint32_t eventCount,
                                                            const VkEvent *pEvents, const VkDependencyInfoKHR *pDependencyInfos) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordWaitEvents(CMD_WAITEVENTS2KHR, eventCount, pEvents);
    for (uint32_t i = 0; i < eventCount; i++) {
        cb_state->RecordBarriers(pDependencyInfos[i]);
    }
}

void ValidationStateTracker::PostCallRecordCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                              VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                              uint32_t memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers,
                                                              uint32_t bufferMemoryBarrierCount,
                                                              const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                                                              uint32_t imageMemoryBarrierCount,
                                                              const VkImageMemoryBarrier *pImageMemoryBarriers) {
    auto cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_PIPELINEBARRIER);
    cb_state->RecordBarriers(memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
                             imageMemoryBarrierCount, pImageMemoryBarriers);
}

void ValidationStateTracker::PreCallRecordCmdPipelineBarrier2KHR(VkCommandBuffer commandBuffer,
                                                                 const VkDependencyInfoKHR *pDependencyInfo) {
    auto cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_PIPELINEBARRIER2KHR);
    cb_state->RecordBarriers(*pDependencyInfo);
}

QueryState ValidationStateTracker::GetQueryState(const QueryMap *localQueryToStateMap, VkQueryPool queryPool, uint32_t queryIndex,
                                                 uint32_t perfPass) const {
    QueryObject query = QueryObject(QueryObject(queryPool, queryIndex), perfPass);

    auto iter = localQueryToStateMap->find(query);
    if (iter != localQueryToStateMap->end()) return iter->second;

    return QUERYSTATE_UNKNOWN;
}

void ValidationStateTracker::PostCallRecordCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot,
                                                         VkFlags flags) {
    if (disabled[query_validation]) return;

    QueryObject query = {queryPool, slot};
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_BEGINQUERY);
    if (!disabled[query_validation]) {
        cb_state->BeginQuery(query);
    }
    if (!disabled[command_buffer_state]) {
        auto pool_state = GetQueryPoolState(query.pool);
        cb_state->AddChild(pool_state);
    }
}

void ValidationStateTracker::PostCallRecordCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot) {
    if (disabled[query_validation]) return;
    QueryObject query_obj = {queryPool, slot};
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_ENDQUERY);
    if (!disabled[query_validation]) {
        cb_state->EndQuery(query_obj);
    }
    if (!disabled[command_buffer_state]) {
        auto pool_state = GetQueryPoolState(query_obj.pool);
        cb_state->AddChild(pool_state);
    }
}

void ValidationStateTracker::PostCallRecordCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                             uint32_t firstQuery, uint32_t queryCount) {
    if (disabled[query_validation]) return;
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->RecordCmd(CMD_RESETQUERYPOOL);
    cb_state->ResetQueryPool(queryPool, firstQuery, queryCount);

    if (!disabled[command_buffer_state]) {
        auto pool_state = GetQueryPoolState(queryPool);
        cb_state->AddChild(pool_state);
    }
}

void ValidationStateTracker::PostCallRecordCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                                   uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer,
                                                                   VkDeviceSize dstOffset, VkDeviceSize stride,
                                                                   VkQueryResultFlags flags) {
    if (disabled[query_validation] || disabled[command_buffer_state]) return;

    auto cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_COPYQUERYPOOLRESULTS);
    auto dst_buff_state = GetBufferState(dstBuffer);
    cb_state->AddChild(dst_buff_state);
    auto pool_state = GetQueryPoolState(queryPool);
    cb_state->AddChild(pool_state);
}

void ValidationStateTracker::PostCallRecordCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                             VkQueryPool queryPool, uint32_t slot) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordWriteTimestamp(CMD_WRITETIMESTAMP, pipelineStage, queryPool, slot);
}

void ValidationStateTracker::PostCallRecordCmdWriteTimestamp2KHR(VkCommandBuffer commandBuffer,
                                                                 VkPipelineStageFlags2KHR pipelineStage, VkQueryPool queryPool,
                                                                 uint32_t slot) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordWriteTimestamp(CMD_WRITETIMESTAMP2KHR, pipelineStage, queryPool, slot);
}

void ValidationStateTracker::PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(
    VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR *pAccelerationStructures,
    VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) {
    if (disabled[query_validation]) return;
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_WRITEACCELERATIONSTRUCTURESPROPERTIESKHR);
    if (!disabled[command_buffer_state]) {
        auto pool_state = GetQueryPoolState(queryPool);
        cb_state->AddChild(pool_state);
    }
    cb_state->EndQueries(queryPool, firstQuery, accelerationStructureCount);
}

void ValidationStateTracker::PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer,
                                                             VkResult result) {
    if (VK_SUCCESS != result) return;

    std::vector<std::shared_ptr<IMAGE_VIEW_STATE>> views;
    if ((pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT) == 0) {
        views.resize(pCreateInfo->attachmentCount);

        for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
            views[i] = GetShared<IMAGE_VIEW_STATE>(pCreateInfo->pAttachments[i]);
        }
    }

    frameBufferMap[*pFramebuffer] = std::make_shared<FRAMEBUFFER_STATE>(
        *pFramebuffer, pCreateInfo, GetRenderPassShared(pCreateInfo->renderPass), std::move(views));
}

void ValidationStateTracker::PostCallRecordCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                            VkResult result) {
    if (VK_SUCCESS != result) return;
    renderPassMap[*pRenderPass] = std::make_shared<RENDER_PASS_STATE>(*pRenderPass, pCreateInfo);
}

void ValidationStateTracker::PostCallRecordCreateRenderPass2KHR(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                                VkResult result) {
    if (VK_SUCCESS != result) return;

    renderPassMap[*pRenderPass] = std::make_shared<RENDER_PASS_STATE>(*pRenderPass, pCreateInfo);
}

void ValidationStateTracker::PostCallRecordCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2 *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass,
                                                             VkResult result) {
    if (VK_SUCCESS != result) return;

    renderPassMap[*pRenderPass] = std::make_shared<RENDER_PASS_STATE>(*pRenderPass, pCreateInfo);
}

void ValidationStateTracker::PreCallRecordCmdBeginRenderPass(VkCommandBuffer commandBuffer,
                                                             const VkRenderPassBeginInfo *pRenderPassBegin,
                                                             VkSubpassContents contents) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->BeginRenderPass(CMD_BEGINRENDERPASS, pRenderPassBegin, contents);
}

void ValidationStateTracker::PreCallRecordCmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                                 const VkRenderPassBeginInfo *pRenderPassBegin,
                                                                 const VkSubpassBeginInfo *pSubpassBeginInfo) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->BeginRenderPass(CMD_BEGINRENDERPASS2KHR, pRenderPassBegin, pSubpassBeginInfo->contents);
}

void ValidationStateTracker::PostCallRecordCmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                                        uint32_t counterBufferCount,
                                                                        const VkBuffer *pCounterBuffers,
                                                                        const VkDeviceSize *pCounterBufferOffsets) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->RecordCmd(CMD_BEGINTRANSFORMFEEDBACKEXT);
    cb_state->transform_feedback_active = true;
}

void ValidationStateTracker::PostCallRecordCmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer,
                                                                      uint32_t counterBufferCount, const VkBuffer *pCounterBuffers,
                                                                      const VkDeviceSize *pCounterBufferOffsets) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->RecordCmd(CMD_ENDTRANSFORMFEEDBACKEXT);
    cb_state->transform_feedback_active = false;
}

void ValidationStateTracker::PostCallRecordCmdBeginConditionalRenderingEXT(
    VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT *pConditionalRenderingBegin) {
    auto *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->RecordCmd(CMD_BEGINCONDITIONALRENDERINGEXT);
    cb_state->conditional_rendering_active = true;
    cb_state->conditional_rendering_inside_render_pass = cb_state->activeRenderPass != nullptr;
    cb_state->conditional_rendering_subpass = cb_state->activeSubpass;
}

void ValidationStateTracker::PostCallRecordCmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) {
    auto *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->RecordCmd(CMD_ENDCONDITIONALRENDERINGEXT);
    cb_state->conditional_rendering_active = false;
    cb_state->conditional_rendering_inside_render_pass = false;
    cb_state->conditional_rendering_subpass = 0;
}

void ValidationStateTracker::PreCallRecordCmdBeginRenderPass2(VkCommandBuffer commandBuffer,
                                                              const VkRenderPassBeginInfo *pRenderPassBegin,
                                                              const VkSubpassBeginInfo *pSubpassBeginInfo) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->BeginRenderPass(CMD_BEGINRENDERPASS2, pRenderPassBegin, pSubpassBeginInfo->contents);
}


void ValidationStateTracker::PostCallRecordCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->NextSubpass(CMD_NEXTSUBPASS, contents);
}

void ValidationStateTracker::PostCallRecordCmdNextSubpass2KHR(VkCommandBuffer commandBuffer,
                                                              const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                              const VkSubpassEndInfo *pSubpassEndInfo) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->NextSubpass(CMD_NEXTSUBPASS2KHR, pSubpassBeginInfo->contents);
}

void ValidationStateTracker::PostCallRecordCmdNextSubpass2(VkCommandBuffer commandBuffer,
                                                           const VkSubpassBeginInfo *pSubpassBeginInfo,
                                                           const VkSubpassEndInfo *pSubpassEndInfo) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->NextSubpass(CMD_NEXTSUBPASS2, pSubpassBeginInfo->contents);
}

void ValidationStateTracker::PostCallRecordCmdEndRenderPass(VkCommandBuffer commandBuffer) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->EndRenderPass(CMD_ENDRENDERPASS);
}

void ValidationStateTracker::PostCallRecordCmdEndRenderPass2KHR(VkCommandBuffer commandBuffer,
                                                                const VkSubpassEndInfo *pSubpassEndInfo) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->EndRenderPass(CMD_ENDRENDERPASS2KHR);
}

void ValidationStateTracker::PostCallRecordCmdEndRenderPass2(VkCommandBuffer commandBuffer,
                                                             const VkSubpassEndInfo *pSubpassEndInfo) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->EndRenderPass(CMD_ENDRENDERPASS2);
}

void ValidationStateTracker::PreCallRecordCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount,
                                                             const VkCommandBuffer *pCommandBuffers) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->ExecuteCommands(commandBuffersCount, pCommandBuffers);
}

void ValidationStateTracker::PostCallRecordMapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size,
                                                     VkFlags flags, void **ppData, VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordMappedMemory(mem, offset, size, ppData);
}

void ValidationStateTracker::PreCallRecordUnmapMemory(VkDevice device, VkDeviceMemory mem) {
    auto mem_info = GetDevMemState(mem);
    if (mem_info) {
        mem_info->mapped_range = MemRange();
        mem_info->p_driver_data = nullptr;
    }
}

void ValidationStateTracker::UpdateBindImageMemoryState(const VkBindImageMemoryInfo &bindInfo) {
    auto image_state = GetShared<IMAGE_STATE>(bindInfo.image);
    if (image_state) {
        // An Android sepcial image cannot get VkSubresourceLayout until the image binds a memory.
        // See: VUID-vkGetImageSubresourceLayout-image-01895
        image_state->fragment_encoder =
            std::unique_ptr<const subresource_adapter::ImageRangeEncoder>(new subresource_adapter::ImageRangeEncoder(*image_state));
        const auto swapchain_info = LvlFindInChain<VkBindImageMemorySwapchainInfoKHR>(bindInfo.pNext);
        if (swapchain_info) {
            auto swapchain = GetShared<SWAPCHAIN_NODE>(swapchain_info->swapchain);
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
            auto mem_info = GetDevMemShared(bindInfo.memory);
            if (mem_info) {
                image_state->SetMemBinding(mem_info, bindInfo.memoryOffset);
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
    auto event_state = GetEventState(event);
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
    SEMAPHORE_STATE *semaphore_state = GetSemaphoreState(semaphore);
    if (semaphore_state && handle_type != VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT) {
        // Cannot track semaphore state once it is exported, except for Sync FD handle types which have copy transference
        semaphore_state->scope = kSyncScopeExternalPermanent;
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
    FENCE_STATE *fence_node = GetFenceState(fence);
    if (fence_node && fence_node->scope != kSyncScopeExternalPermanent) {
        if ((handle_type == VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT || flags & VK_FENCE_IMPORT_TEMPORARY_BIT) &&
            fence_node->scope == kSyncScopeInternal) {
            fence_node->scope = kSyncScopeExternalTemporary;
        } else {
            fence_node->scope = kSyncScopeExternalPermanent;
        }
    }
}

void ValidationStateTracker::PostCallRecordImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *pImportFenceFdInfo,
                                                            VkResult result) {
    if (VK_SUCCESS != result) return;
    RecordImportFenceState(pImportFenceFdInfo->fence, pImportFenceFdInfo->handleType, pImportFenceFdInfo->flags);
}

void ValidationStateTracker::RecordGetExternalFenceState(VkFence fence, VkExternalFenceHandleTypeFlagBits handle_type) {
    FENCE_STATE *fence_state = GetFenceState(fence);
    if (fence_state) {
        if (handle_type != VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT) {
            // Export with reference transference becomes external
            fence_state->scope = kSyncScopeExternalPermanent;
        } else if (fence_state->scope == kSyncScopeInternal) {
            // Export with copy transference has a side effect of resetting the fence
            fence_state->state = FENCE_UNSIGNALED;
        }
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
    const auto event = *pEvent;
    eventMap.emplace(event, std::make_shared<EVENT_STATE>(event, pCreateInfo->flags));
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
        swapchainMap[*pSwapchain] = std::move(swapchain);
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
    auto surface_state = GetShared<SURFACE_STATE>(pCreateInfo->surface);
    auto old_swapchain_state = GetSwapchainState(pCreateInfo->oldSwapchain);
    RecordCreateSwapchainState(result, pCreateInfo, pSwapchain, std::move(surface_state), old_swapchain_state);
}

void ValidationStateTracker::PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                              const VkAllocationCallbacks *pAllocator) {
    if (!swapchain) return;
    auto swapchain_data = GetSwapchainState(swapchain);
    if (!swapchain_data) return;

    swapchain_data->Destroy();
    swapchainMap.erase(swapchain);
}

void ValidationStateTracker::PostCallRecordCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                                const VkDisplayModeCreateInfoKHR *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator, VkDisplayModeKHR *pMode,
                                                                VkResult result) {
    if (VK_SUCCESS != result) return;
    if (!pMode) return;
    display_mode_map[*pMode] = std::make_shared<DISPLAY_MODE_STATE>(*pMode, physicalDevice);
}

void ValidationStateTracker::PostCallRecordQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *pPresentInfo, VkResult result) {
    // Semaphore waits occur before error generation, if the call reached the ICD. (Confirm?)
    for (uint32_t i = 0; i < pPresentInfo->waitSemaphoreCount; ++i) {
        auto semaphore_state = GetSemaphoreState(pPresentInfo->pWaitSemaphores[i]);
        if (semaphore_state) {
            semaphore_state->signaler.first = VK_NULL_HANDLE;
            semaphore_state->signaled = false;
        }
    }

    const auto *present_id_info = LvlFindInChain<VkPresentIdKHR>(pPresentInfo->pNext);
    for (uint32_t i = 0; i < pPresentInfo->swapchainCount; ++i) {
        // Note: this is imperfect, in that we can get confused about what did or didn't succeed-- but if the app does that, it's
        // confused itself just as much.
        auto local_result = pPresentInfo->pResults ? pPresentInfo->pResults[i] : result;
        if (local_result != VK_SUCCESS && local_result != VK_SUBOPTIMAL_KHR) continue;  // this present didn't actually happen.
        // Mark the image as having been released to the WSI
        auto swapchain_data = GetSwapchainState(pPresentInfo->pSwapchains[i]);
        if (swapchain_data) {
            swapchain_data->PresentImage(pPresentInfo->pImageIndices[i]);
            if (present_id_info) {
                if (i < present_id_info->swapchainCount && present_id_info->pPresentIds[i] > swapchain_data->max_present_id) {
                    swapchain_data->max_present_id = present_id_info->pPresentIds[i];
                }
            }
        }
    }
    // Note: even though presentation is directed to a queue, there is no direct ordering between QP and subsequent work, so QP (and
    // its semaphore waits) /never/ participate in any completion proof.
}

void ValidationStateTracker::PostCallRecordCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                                     const VkSwapchainCreateInfoKHR *pCreateInfos,
                                                                     const VkAllocationCallbacks *pAllocator,
                                                                     VkSwapchainKHR *pSwapchains, VkResult result) {
    if (pCreateInfos) {
        for (uint32_t i = 0; i < swapchainCount; i++) {
            auto surface_state = GetShared<SURFACE_STATE>(pCreateInfos[i].surface);
            auto old_swapchain_state = GetSwapchainState(pCreateInfos[i].oldSwapchain);
            RecordCreateSwapchainState(result, &pCreateInfos[i], &pSwapchains[i], std::move(surface_state), old_swapchain_state);
        }
    }
}

void ValidationStateTracker::RecordAcquireNextImageState(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout,
                                                         VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex) {
    auto fence_state = GetFenceState(fence);
    if (fence_state && fence_state->scope == kSyncScopeInternal) {
        // Treat as inflight since it is valid to wait on this fence, even in cases where it is technically a temporary
        // import
        fence_state->state = FENCE_INFLIGHT;
        fence_state->signaler.first = VK_NULL_HANDLE;  // ANI isn't on a queue, so this can't participate in a completion proof.
    }

    auto semaphore_state = GetSemaphoreState(semaphore);
    if (semaphore_state && semaphore_state->scope == kSyncScopeInternal) {
        // Treat as signaled since it is valid to wait on this semaphore, even in cases where it is technically a
        // temporary import
        semaphore_state->signaled = true;
        semaphore_state->signaler.first = VK_NULL_HANDLE;
    }

    // Mark the image as acquired.
    auto swapchain_data = GetSwapchainState(swapchain);
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

void ValidationStateTracker::PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount,
                                                                    VkPhysicalDevice *pPhysicalDevices, VkResult result) {
    if ((NULL != pPhysicalDevices) && ((result == VK_SUCCESS || result == VK_INCOMPLETE))) {
        for (uint32_t i = 0; i < *pPhysicalDeviceCount; i++) {
            physical_device_map.emplace(pPhysicalDevices[i], CreatePhysicalDeviceState(pPhysicalDevices[i]));
        }
    }
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
    StateUpdateCommonGetPhysicalDeviceQueueFamilyProperties(pd_state, *pQueueFamilyPropertyCount);
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(
    VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties2 *pQueueFamilyProperties) {
    auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
    assert(pd_state);
    StateUpdateCommonGetPhysicalDeviceQueueFamilyProperties(pd_state, *pQueueFamilyPropertyCount);
}

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties2 *pQueueFamilyProperties) {
    auto pd_state = Get<PHYSICAL_DEVICE_STATE>(physicalDevice);
    assert(pd_state);
    StateUpdateCommonGetPhysicalDeviceQueueFamilyProperties(pd_state, *pQueueFamilyPropertyCount);
}
void ValidationStateTracker::PreCallRecordDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                                            const VkAllocationCallbacks *pAllocator) {
    if (!surface) return;
    auto surface_state = GetSurfaceState(surface);
    surface_state->Destroy();
    surface_map.erase(surface);
}

void ValidationStateTracker::RecordVulkanSurface(VkSurfaceKHR *pSurface) {
    surface_map[*pSurface] = std::make_shared<SURFACE_STATE>(*pSurface);
}

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

void ValidationStateTracker::PostCallRecordGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice,
                                                                              uint32_t queueFamilyIndex, VkSurfaceKHR surface,
                                                                              VkBool32 *pSupported, VkResult result) {
    if (VK_SUCCESS != result) return;
    auto surface_state = GetSurfaceState(surface);
    surface_state->gpu_queue_support[{physicalDevice, queueFamilyIndex}] = (*pSupported == VK_TRUE);
}

void ValidationStateTracker::PreCallRecordCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                                                                     const VkDebugUtilsLabelEXT *pLabelInfo) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_BEGINDEBUGUTILSLABELEXT);
    BeginCmdDebugUtilsLabel(report_data, commandBuffer, pLabelInfo);
}

void ValidationStateTracker::PostCallRecordCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_ENDDEBUGUTILSLABELEXT);
    EndCmdDebugUtilsLabel(report_data, commandBuffer);
}

void ValidationStateTracker::PreCallRecordCmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer,
                                                                      const VkDebugUtilsLabelEXT *pLabelInfo) {
    InsertCmdDebugUtilsLabel(report_data, commandBuffer, pLabelInfo);

    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_INSERTDEBUGUTILSLABELEXT);
    // Squirrel away an easily accessible copy.
    cb_state->debug_label = LoggingLabel(pLabelInfo);
}

void ValidationStateTracker::RecordEnumeratePhysicalDeviceGroupsState(
    uint32_t *pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties) {
    if (NULL != pPhysicalDeviceGroupProperties) {
        for (uint32_t i = 0; i < *pPhysicalDeviceGroupCount; i++) {
            for (uint32_t j = 0; j < pPhysicalDeviceGroupProperties[i].physicalDeviceCount; j++) {
                VkPhysicalDevice cur_phys_dev = pPhysicalDeviceGroupProperties[i].physicalDevices[j];

                physical_device_map.emplace(cur_phys_dev, CreatePhysicalDeviceState(cur_phys_dev));
            }
        }
    }
}

void ValidationStateTracker::PostCallRecordEnumeratePhysicalDeviceGroups(
    VkInstance instance, uint32_t *pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties,
    VkResult result) {
    if ((VK_SUCCESS != result) && (VK_INCOMPLETE != result)) return;
    RecordEnumeratePhysicalDeviceGroupsState(pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
}

void ValidationStateTracker::PostCallRecordEnumeratePhysicalDeviceGroupsKHR(
    VkInstance instance, uint32_t *pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties *pPhysicalDeviceGroupProperties,
    VkResult result) {
    if ((VK_SUCCESS != result) && (VK_INCOMPLETE != result)) return;
    RecordEnumeratePhysicalDeviceGroupsState(pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
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
    for (auto &cmd_buffer : commandBufferMap) {
        cmd_buffer.second->performance_lock_released = true;
    }
}

void ValidationStateTracker::PreCallRecordDestroyDescriptorUpdateTemplate(VkDevice device,
                                                                          VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                          const VkAllocationCallbacks *pAllocator) {
    if (!descriptorUpdateTemplate) return;
    auto template_state = GetDescriptorTemplateState(descriptorUpdateTemplate);
    template_state->destroyed = true;
    desc_template_map.erase(descriptorUpdateTemplate);
}

void ValidationStateTracker::PreCallRecordDestroyDescriptorUpdateTemplateKHR(VkDevice device,
                                                                             VkDescriptorUpdateTemplate descriptorUpdateTemplate,
                                                                             const VkAllocationCallbacks *pAllocator) {
    if (!descriptorUpdateTemplate) return;
    auto template_state = GetDescriptorTemplateState(descriptorUpdateTemplate);
    template_state->destroyed = true;
    desc_template_map.erase(descriptorUpdateTemplate);
}

void ValidationStateTracker::RecordCreateDescriptorUpdateTemplateState(const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo,
                                                                       VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate) {
    safe_VkDescriptorUpdateTemplateCreateInfo local_create_info(pCreateInfo);
    auto template_state = std::make_shared<TEMPLATE_STATE>(*pDescriptorUpdateTemplate, &local_create_info);
    desc_template_map[*pDescriptorUpdateTemplate] = std::move(template_state);
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
    auto const template_map_entry = desc_template_map.find(descriptorUpdateTemplate);
    if ((template_map_entry == desc_template_map.end()) || (template_map_entry->second.get() == nullptr)) {
        assert(0);
    } else {
        const TEMPLATE_STATE *template_state = template_map_entry->second.get();
        // TODO: Record template push descriptor updates
        if (template_state->create_info.templateType == VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET) {
            PerformUpdateDescriptorSetsWithTemplateKHR(descriptorSet, template_state, pData);
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
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);

    cb_state->RecordCmd(CMD_PUSHDESCRIPTORSETWITHTEMPLATEKHR);
    const auto template_state = GetDescriptorTemplateState(descriptorUpdateTemplate);
    if (template_state) {
        auto layout_data = GetPipelineLayout(layout);
        auto dsl = layout_data ? layout_data->GetDsl(set) : nullptr;
        const auto &template_ci = template_state->create_info;
        // Decode the template into a set of write updates
        cvdescriptorset::DecodedTemplateUpdate decoded_template(this, VK_NULL_HANDLE, template_state, pData,
                                                                dsl->GetDescriptorSetLayout());
        cb_state->PushDescriptorSetState(template_ci.pipelineBindPoint, layout_data, set,
                                         static_cast<uint32_t>(decoded_template.desc_writes.size()),
                                         decoded_template.desc_writes.data());
    }
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
    QueryObject query_obj = {queryPool, query, index};
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_BEGINQUERYINDEXEDEXT);
    cb_state->BeginQuery(query_obj);
}

void ValidationStateTracker::PostCallRecordCmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool,
                                                                 uint32_t query, uint32_t index) {
    QueryObject query_obj = {queryPool, query, index};
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordCmd(CMD_ENDQUERYINDEXEDEXT);
    cb_state->EndQuery(query_obj);
}

void ValidationStateTracker::RecordCreateSamplerYcbcrConversionState(const VkSamplerYcbcrConversionCreateInfo *create_info,
                                                                     VkSamplerYcbcrConversion ycbcr_conversion) {
    VkFormatFeatureFlags format_features = 0;

    if (create_info->format != VK_FORMAT_UNDEFINED) {
        format_features = GetPotentialFormatFeatures(create_info->format);
    } else if (IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)) {
        // If format is VK_FORMAT_UNDEFINED, format_features will be set by external AHB features
        format_features = GetExternalFormatFeaturesANDROID(create_info);
    }

    samplerYcbcrConversionMap[ycbcr_conversion] =
        std::make_shared<SAMPLER_YCBCR_CONVERSION_STATE>(ycbcr_conversion, create_info, format_features);
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

void ValidationStateTracker::RecordDestroySamplerYcbcrConversionState(VkSamplerYcbcrConversion ycbcr_conversion) {
    auto ycbcr_state = GetSamplerYcbcrConversionState(ycbcr_conversion);
    ycbcr_state->Destroy();
    samplerYcbcrConversionMap.erase(ycbcr_conversion);
}

void ValidationStateTracker::PostCallRecordDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion,
                                                                         const VkAllocationCallbacks *pAllocator) {
    if (!ycbcrConversion) return;
    RecordDestroySamplerYcbcrConversionState(ycbcrConversion);
}

void ValidationStateTracker::PostCallRecordDestroySamplerYcbcrConversionKHR(VkDevice device,
                                                                            VkSamplerYcbcrConversion ycbcrConversion,
                                                                            const VkAllocationCallbacks *pAllocator) {
    if (!ycbcrConversion) return;
    RecordDestroySamplerYcbcrConversionState(ycbcrConversion);
}

void ValidationStateTracker::RecordResetQueryPool(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery,
                                                  uint32_t queryCount) {
    // Do nothing if the feature is not enabled.
    if (!enabled_features.core12.hostQueryReset) return;

    // Do nothing if the query pool has been destroyed.
    auto query_pool_state = GetQueryPoolState(queryPool);
    if (!query_pool_state) return;

    // Reset the state of existing entries.
    QueryObject query_obj{queryPool, 0};
    const uint32_t max_query_count = std::min(queryCount, query_pool_state->createInfo.queryCount - firstQuery);
    for (uint32_t i = 0; i < max_query_count; ++i) {
        query_obj.query = firstQuery + i;
        queryToStateMap[query_obj] = QUERYSTATE_RESET;
        if (query_pool_state->createInfo.queryType == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR) {
            for (uint32_t pass_index = 0; pass_index < query_pool_state->n_performance_passes; pass_index++) {
                query_obj.perf_pass = pass_index;
                queryToStateMap[query_obj] = QUERYSTATE_RESET;
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
                                                                        const TEMPLATE_STATE *template_state, const void *pData) {
    // Translate the templated update into a normal update for validation...
    cvdescriptorset::DecodedTemplateUpdate decoded_update(this, descriptorSet, template_state, pData);
    cvdescriptorset::PerformUpdateDescriptorSets(this, static_cast<uint32_t>(decoded_update.desc_writes.size()),
                                                 decoded_update.desc_writes.data(), 0, NULL);
}

// Update the common AllocateDescriptorSetsData
void ValidationStateTracker::UpdateAllocateDescriptorSetsData(const VkDescriptorSetAllocateInfo *p_alloc_info,
                                                              cvdescriptorset::AllocateDescriptorSetsData *ds_data) const {
    for (uint32_t i = 0; i < p_alloc_info->descriptorSetCount; i++) {
        auto layout = GetDescriptorSetLayoutShared(p_alloc_info->pSetLayouts[i]);
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

// Decrement allocated sets from the pool and insert new sets into set_map
void ValidationStateTracker::PerformAllocateDescriptorSets(const VkDescriptorSetAllocateInfo *p_alloc_info,
                                                           const VkDescriptorSet *descriptor_sets,
                                                           const cvdescriptorset::AllocateDescriptorSetsData *ds_data) {
    auto pool_state = descriptorPoolMap[p_alloc_info->descriptorPool].get();
    // Account for sets and individual descriptors allocated from pool
    pool_state->availableSets -= p_alloc_info->descriptorSetCount;
    for (auto it = ds_data->required_descriptors_by_type.begin(); it != ds_data->required_descriptors_by_type.end(); ++it) {
        pool_state->availableDescriptorTypeCount[it->first] -= ds_data->required_descriptors_by_type.at(it->first);
    }

    const auto *variable_count_info = LvlFindInChain<VkDescriptorSetVariableDescriptorCountAllocateInfo>(p_alloc_info->pNext);
    bool variable_count_valid = variable_count_info && variable_count_info->descriptorSetCount == p_alloc_info->descriptorSetCount;

    // Create tracking object for each descriptor set; insert into global map and the pool's set.
    for (uint32_t i = 0; i < p_alloc_info->descriptorSetCount; i++) {
        uint32_t variable_count = variable_count_valid ? variable_count_info->pDescriptorCounts[i] : 0;

        auto new_ds = std::make_shared<cvdescriptorset::DescriptorSet>(descriptor_sets[i], pool_state, ds_data->layout_nodes[i],
                                                                       variable_count, this);
        pool_state->sets.insert(new_ds.get());
        setMap[descriptor_sets[i]] = std::move(new_ds);
    }
}

void ValidationStateTracker::PostCallRecordCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                                   uint32_t firstVertex, uint32_t firstInstance) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateStateCmdDrawType(CMD_DRAW, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void ValidationStateTracker::PostCallRecordCmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                           const VkMultiDrawInfoEXT *pVertexInfo, uint32_t instanceCount,
                                                           uint32_t firstInstance, uint32_t stride) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateStateCmdDrawType(CMD_DRAWMULTIEXT, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void ValidationStateTracker::PostCallRecordCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount,
                                                          uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset,
                                                          uint32_t firstInstance) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateStateCmdDrawType(CMD_DRAWINDEXED, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void ValidationStateTracker::PostCallRecordCmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount,
                                                                  const VkMultiDrawIndexedInfoEXT *pIndexInfo,
                                                                  uint32_t instanceCount, uint32_t firstInstance, uint32_t stride,
                                                                  const int32_t *pVertexOffset) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateStateCmdDrawType(CMD_DRAWMULTIINDEXEDEXT, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void ValidationStateTracker::PostCallRecordCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                           uint32_t count, uint32_t stride) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    cb_state->UpdateStateCmdDrawType(CMD_DRAWINDIRECT, VK_PIPELINE_BIND_POINT_GRAPHICS);
    if (!disabled[command_buffer_state]) {
        cb_state->AddChild(buffer_state);
    }
}

void ValidationStateTracker::PostCallRecordCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                  VkDeviceSize offset, uint32_t count, uint32_t stride) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    cb_state->UpdateStateCmdDrawType(CMD_DRAWINDEXEDINDIRECT, VK_PIPELINE_BIND_POINT_GRAPHICS);
    if (!disabled[command_buffer_state]) {
        cb_state->AddChild(buffer_state);
    }
}

void ValidationStateTracker::PostCallRecordCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateStateCmdDrawDispatchType(CMD_DISPATCH, VK_PIPELINE_BIND_POINT_COMPUTE);
}

void ValidationStateTracker::PostCallRecordCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                               VkDeviceSize offset) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateStateCmdDrawDispatchType(CMD_DISPATCHINDIRECT, VK_PIPELINE_BIND_POINT_COMPUTE);
    if (!disabled[command_buffer_state]) {
        BUFFER_STATE *buffer_state = GetBufferState(buffer);
        cb_state->AddChild(buffer_state);
    }
}

void ValidationStateTracker::RecordCmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                        VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                        uint32_t stride, CMD_TYPE cmd_type) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateStateCmdDrawType(cmd_type, VK_PIPELINE_BIND_POINT_GRAPHICS);
    if (!disabled[command_buffer_state]) {
        BUFFER_STATE *buffer_state = GetBufferState(buffer);
        BUFFER_STATE *count_buffer_state = GetBufferState(countBuffer);
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
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateStateCmdDrawType(cmd_type, VK_PIPELINE_BIND_POINT_GRAPHICS);
    if (!disabled[command_buffer_state]) {
        BUFFER_STATE *buffer_state = GetBufferState(buffer);
        BUFFER_STATE *count_buffer_state = GetBufferState(countBuffer);
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
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateStateCmdDrawType(CMD_DRAWMESHTASKSNV, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void ValidationStateTracker::PreCallRecordCmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                     VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateStateCmdDrawType(CMD_DRAWMESHTASKSINDIRECTNV, VK_PIPELINE_BIND_POINT_GRAPHICS);
    BUFFER_STATE *buffer_state = GetBufferState(buffer);
    if (!disabled[command_buffer_state] && buffer_state) {
        cb_state->AddChild(buffer_state);
    }
}

void ValidationStateTracker::PreCallRecordCmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                                                          VkDeviceSize offset, VkBuffer countBuffer,
                                                                          VkDeviceSize countBufferOffset, uint32_t maxDrawCount,
                                                                          uint32_t stride) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateStateCmdDrawType(CMD_DRAWMESHTASKSINDIRECTCOUNTNV, VK_PIPELINE_BIND_POINT_GRAPHICS);
    if (!disabled[command_buffer_state]) {
        BUFFER_STATE *buffer_state = GetBufferState(buffer);
        BUFFER_STATE *count_buffer_state = GetBufferState(countBuffer);
        if (buffer_state) {
            cb_state->AddChild(buffer_state);
        }
        if (count_buffer_state) {
            cb_state->AddChild(count_buffer_state);
        }
    }
}

void ValidationStateTracker::PostCallRecordCmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer,
                                              VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer,
                                              VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride,
                                              VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset,
                                              VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer,
                                              VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride,
                                              uint32_t width, uint32_t height, uint32_t depth) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateStateCmdDrawDispatchType(CMD_TRACERAYSNV, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV);
    cb_state->hasTraceRaysCmd = true;
}


void ValidationStateTracker::PostCallRecordCmdTraceRaysKHR(VkCommandBuffer commandBuffer,
                                               const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                               const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable, uint32_t width,
                                               uint32_t height, uint32_t depth) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateStateCmdDrawDispatchType(CMD_TRACERAYSKHR, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
    cb_state->hasTraceRaysCmd = true;
}

void ValidationStateTracker::PostCallRecordCmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer,
                                                         const VkStridedDeviceAddressRegionKHR *pRaygenShaderBindingTable,
                                                         const VkStridedDeviceAddressRegionKHR *pMissShaderBindingTable,
                                                         const VkStridedDeviceAddressRegionKHR *pHitShaderBindingTable,
                                                         const VkStridedDeviceAddressRegionKHR *pCallableShaderBindingTable,
                                                         VkDeviceAddress indirectDeviceAddress) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->UpdateStateCmdDrawDispatchType(CMD_TRACERAYSINDIRECTKHR, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
    cb_state->hasTraceRaysCmd = true;
}

void ValidationStateTracker::PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkShaderModule *pShaderModule, VkResult result,
                                                              void *csm_state_data) {
    if (VK_SUCCESS != result) return;
    create_shader_module_api_state *csm_state = reinterpret_cast<create_shader_module_api_state *>(csm_state_data);

    spv_target_env spirv_environment = PickSpirvEnv(api_version, IsExtEnabled(device_extensions.vk_khr_spirv_1_4));
    bool is_spirv = (pCreateInfo->pCode[0] == spv::MagicNumber);
    auto new_shader_module = is_spirv ? std::make_shared<SHADER_MODULE_STATE>(pCreateInfo, *pShaderModule, spirv_environment,
                                                                              csm_state->unique_shader_id)
                                      : std::make_shared<SHADER_MODULE_STATE>();
    new_shader_module->SetPushConstantUsedInShader();
    shaderModuleMap[*pShaderModule] = std::move(new_shader_module);
}

void ValidationStateTracker::PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                                 uint32_t *pSwapchainImageCount, VkImage *pSwapchainImages,
                                                                 VkResult result) {
    if ((result != VK_SUCCESS) && (result != VK_INCOMPLETE)) return;
    auto swapchain_state = GetShared<SWAPCHAIN_NODE>(swapchain);

    if (*pSwapchainImageCount > swapchain_state->images.size()) swapchain_state->images.resize(*pSwapchainImageCount);

    if (pSwapchainImages) {
        for (uint32_t i = 0; i < *pSwapchainImageCount; ++i) {
            SWAPCHAIN_IMAGE &swapchain_image = swapchain_state->images[i];
            if (swapchain_image.image_state) continue;  // Already retrieved this.

            auto format_features =
                GetImageFormatFeatures(physical_device, device, pSwapchainImages[i], swapchain_state->image_create_info.format,
                                       swapchain_state->image_create_info.tiling);

            auto image_state = std::make_shared<IMAGE_STATE>(this, pSwapchainImages[i], swapchain_state->image_create_info.ptr(),
                                                             swapchain, i, format_features);
            if (!swapchain_image.fake_base_address) {
                auto size = image_state->fragment_encoder->TotalSize();
                swapchain_image.fake_base_address = fake_memory.Alloc(size);
            }

            image_state->SetSwapchain(swapchain_state, i);
            swapchain_image.image_state = image_state.get();
            imageMap[pSwapchainImages[i]] = std::move(image_state);
        }
    }

    if (*pSwapchainImageCount) {
        swapchain_state->get_swapchain_image_count = *pSwapchainImageCount;
    }
}

void ValidationStateTracker::PostCallRecordCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,
                                                                           const VkCopyAccelerationStructureInfoKHR *pInfo) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    if (cb_state) {
        cb_state->RecordCmd(CMD_COPYACCELERATIONSTRUCTUREKHR);
        ACCELERATION_STRUCTURE_STATE_KHR *src_as_state = GetAccelerationStructureStateKHR(pInfo->src);
        ACCELERATION_STRUCTURE_STATE_KHR *dst_as_state = GetAccelerationStructureStateKHR(pInfo->dst);
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

void ValidationStateTracker::PreCallRecordCmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETCULLMODEEXT, CBSTATUS_CULL_MODE_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETFRONTFACEEXT, CBSTATUS_FRONT_FACE_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer,
                                                                     VkPrimitiveTopology primitiveTopology) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETPRIMITIVETOPOLOGYEXT, CBSTATUS_PRIMITIVE_TOPOLOGY_SET);
    cb_state->primitiveTopology = primitiveTopology;
}

void ValidationStateTracker::PreCallRecordCmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount,
                                                                     const VkViewport *pViewports) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETVIEWPORTWITHCOUNTEXT, CBSTATUS_VIEWPORT_WITH_COUNT_SET);
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

void ValidationStateTracker::PreCallRecordCmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount,
                                                                    const VkRect2D *pScissors) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETSCISSORWITHCOUNTEXT, CBSTATUS_SCISSOR_WITH_COUNT_SET);
    uint32_t bits = (1u << scissorCount) - 1u;
    cb_state->scissorWithCountMask |= bits;
    cb_state->trashedScissorMask &= ~bits;
    cb_state->scissorWithCountCount = scissorCount;
    cb_state->trashedScissorCount = false;
}

void ValidationStateTracker::PreCallRecordCmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding,
                                                                   uint32_t bindingCount, const VkBuffer *pBuffers,
                                                                   const VkDeviceSize *pOffsets, const VkDeviceSize *pSizes,
                                                                   const VkDeviceSize *pStrides) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_BINDVERTEXBUFFERS2EXT, pStrides ? CBSTATUS_VERTEX_INPUT_BINDING_STRIDE_SET : CBSTATUS_NONE);

    uint32_t end = firstBinding + bindingCount;
    if (cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings.size() < end) {
        cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings.resize(end);
    }

    for (uint32_t i = 0; i < bindingCount; ++i) {
        auto &vertex_buffer_binding = cb_state->current_vertex_buffer_binding_info.vertex_buffer_bindings[i + firstBinding];
        vertex_buffer_binding.buffer_state = GetShared<BUFFER_STATE>(pBuffers[i]);
        vertex_buffer_binding.offset = pOffsets[i];
        vertex_buffer_binding.size = (pSizes) ? pSizes[i] : VK_WHOLE_SIZE;
        vertex_buffer_binding.stride = (pStrides) ? pStrides[i] : 0;
        // Add binding for this vertex buffer to this commandbuffer
        if (!disabled[command_buffer_state] && pBuffers[i]) {
            cb_state->AddChild(vertex_buffer_binding.buffer_state.get());
        }
    }
}

void ValidationStateTracker::PreCallRecordCmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETDEPTHTESTENABLEEXT, CBSTATUS_DEPTH_TEST_ENABLE_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETDEPTHWRITEENABLEEXT, CBSTATUS_DEPTH_WRITE_ENABLE_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETDEPTHCOMPAREOPEXT, CBSTATUS_DEPTH_COMPARE_OP_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer,
                                                                         VkBool32 depthBoundsTestEnable) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETDEPTHBOUNDSTESTENABLEEXT, CBSTATUS_DEPTH_BOUNDS_TEST_ENABLE_SET);
}
void ValidationStateTracker::PreCallRecordCmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETSTENCILTESTENABLEEXT, CBSTATUS_STENCIL_TEST_ENABLE_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask,
                                                             VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp,
                                                             VkCompareOp compareOp) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETSTENCILOPEXT, CBSTATUS_STENCIL_OP_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle,
                                                                    uint32_t discardRectangleCount,
                                                                    const VkRect2D *pDiscardRectangles) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETDISCARDRECTANGLEEXT, CBSTATUS_DISCARD_RECTANGLE_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                                                   const VkSampleLocationsInfoEXT *pSampleLocationsInfo) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETSAMPLELOCATIONSEXT, CBSTATUS_SAMPLE_LOCATIONS_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer,
                                                                    VkCoarseSampleOrderTypeNV sampleOrderType,
                                                                    uint32_t customSampleOrderCount,
                                                                    const VkCoarseSampleOrderCustomNV *pCustomSampleOrders) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETCOARSESAMPLEORDERNV, CBSTATUS_COARSE_SAMPLE_ORDER_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETPATCHCONTROLPOINTSEXT, CBSTATUS_PATCH_CONTROL_POINTS_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETLOGICOPEXT, CBSTATUS_LOGIC_OP_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer,
                                                                           VkBool32 rasterizerDiscardEnable) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETRASTERIZERDISCARDENABLEEXT, CBSTATUS_RASTERIZER_DISCARD_ENABLE_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETDEPTHBIASENABLEEXT, CBSTATUS_DEPTH_BIAS_ENABLE_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer,
                                                                          VkBool32 primitiveRestartEnable) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    cb_state->RecordStateCmd(CMD_SETPRIMITIVERESTARTENABLEEXT, CBSTATUS_PRIMITIVE_RESTART_ENABLE_SET);
}

void ValidationStateTracker::PreCallRecordCmdSetVertexInputEXT(
    VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount,
    const VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount,
    const VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions) {
    CMD_BUFFER_STATE *cb_state = Get<CMD_BUFFER_STATE>(commandBuffer);
    CBStatusFlags status_flags = CBSTATUS_VERTEX_INPUT_SET;

    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    const auto pipeline_state = cb_state->lastBound[lv_bind_point].pipeline_state;
    if (pipeline_state) {
        if (pipeline_state->create_info.graphics.pDynamicState) {
            for (uint32_t i = 0; i < pipeline_state->create_info.graphics.pDynamicState->dynamicStateCount; ++i) {
                if (pipeline_state->create_info.graphics.pDynamicState->pDynamicStates[i] ==
                    VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT) {
                    status_flags |= CBSTATUS_VERTEX_INPUT_BINDING_STRIDE_SET;
                    break;
                }
            }
        }
    }
    cb_state->RecordStateCmd(CMD_SETVERTEXINPUTEXT, status_flags);
}

void ValidationStateTracker::RecordGetBufferDeviceAddress(const VkBufferDeviceAddressInfo *pInfo, VkDeviceAddress address) {
    BUFFER_STATE *buffer_state = GetBufferState(pInfo->buffer);
    if (buffer_state) {
        // address is used for GPU-AV and ray tracing buffer validation
        buffer_state->deviceAddress = address;
        buffer_address_map_.emplace(address, buffer_state);
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
                                                                               std::shared_ptr<COMMAND_POOL_STATE> &pool) {
    return std::make_shared<CMD_BUFFER_STATE>(this, cb, create_info, pool);
}
